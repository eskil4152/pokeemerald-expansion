# 05. Engine systems: the building blocks

Every mechanic in this game, vanilla or new, is built from a handful of primitives. This file explains each one with the real API and the pattern used in the codebase. `07-recipes-new-mechanics.md` composes them.

Primitives, in the order you will need them:

1. Flags and vars (state that persists, readable from C and scripts)
2. Save blocks (larger persistent state)
3. Event scripts (sequencing NPC interactions, text, choices)
4. Calling C from scripts: specials, callnative, new commands
5. Triggers: where the engine lets you hook in (map events, per-step, per-frame, per-day, on battle end, on new game)
6. Text and message boxes
7. Menus and full screens
8. Sprites and graphics
9. Battle engine hooks
10. Time
11. Memory and allocation
12. Random numbers

## 1. Flags and vars

`src/event_data.c`, `include/constants/flags.h`, `include/constants/vars.h`.

**Flags** are single bits in `gSaveBlock1Ptr->flags`. Ids are `u16`. `FlagSet(id)`, `FlagClear(id)`, `FlagGet(id)`, `FlagToggle(id)`. In scripts: `setflag`, `clearflag`, `checkflag` then `goto_if_set` / `goto_if_unset`, or the shorthand `goto_if_set FLAG_X, Label`.

Ranges:

| Range | Ids | Behaviour |
|---|---|---|
| Temp | `FLAG_TEMP_1`..`FLAG_TEMP_1F` | Cleared every map load. For "already talked this visit" |
| General | `0x20`..`0x4FF` | Saved. 375 are `FLAG_UNUSED_0x...`; rename one for your feature |
| Trainer | `TRAINER_FLAGS_START + trainerId` | Set when that trainer is beaten |
| System | `FLAG_SYS_*` | Pokédex obtained, badges (`FLAG_BADGE01_GET`..), running shoes, etc. |
| Daily | `DAILY_FLAGS_START`.. | Cleared by `ClearDailyFlags` when the day changes |
| Special | `0x4000`+ | Not saved. Runtime toggles like `FLAG_HIDE_MAP_NAME_POPUP` |

**Vars** are `u16` in `gSaveBlock1Ptr->vars`. `VarSet(id, value)`, `VarGet(id)`, `GetVarPointer(id)` for `+=` style. In scripts: `setvar`, `addvar`, `subvar`, `copyvar`, `compare VAR, value` then `goto_if_eq/ne/lt/gt/le/ge`, `switch VAR` with `case n, Label`. Most script arguments accept a var id in place of a literal (`VarGet` on read: anything `>= 0x4000` is treated as a var).

| Range | Ids | Behaviour |
|---|---|---|
| Temp | `VAR_TEMP_0`..`VAR_TEMP_F` | Cleared every map load |
| General | `0x4020`..`0x40FF` | Saved. 29 `VAR_UNUSED_0x...` free, plus many vanilla ones you can repurpose (each map's `VAR_*_STATE` for example) |
| Object gfx | `VAR_OBJ_GFX_ID_0`..`F` | An NPC with graphics `OBJ_EVENT_GFX_VAR_n` uses the sprite id in this var. Dynamic NPC appearance |
| Special | `VAR_0x8000`..`VAR_0x800B` | Not saved. Script scratch and argument passing to specials |
| `VAR_RESULT` | `0x800D` | Return value convention for commands and specials |
| `VAR_LAST_TALKED`, `VAR_FACING` | | Local id of the NPC in the current script, player facing direction |

The map's object event `flag` field hides the NPC while that flag is set. `coord_events` of type `trigger` fire when `var == var_value` and the player steps on the tile. These two facts plus `VAR_*_STATE` counters are how the whole vanilla story is sequenced.

**When flags and vars are not enough**: you need more than 16 bits per item, or arrays per species, or structures. Then go to save blocks.

Debug menu → Flags & Vars lets you set any of them at runtime, which makes testing a new mechanic trivial.

## 2. Save blocks

Structs in `include/global.h`. Reached through `gSaveBlock1Ptr->field`, `gSaveBlock2Ptr->field`, `gSaveBlock3Ptr->field`.

Size limits (`include/save.h`, enforced by `STATIC_ASSERT` in `src/save.c`, the build fails if exceeded):

| Block | Limit | Holds |
|---|---|---|
| SaveBlock2 | 3968 bytes (1 sector) | Player name, gender, id, play time, options, Pokédex, Frontier records |
| SaveBlock1 | 15872 bytes (4 sectors) | Position, party, money, coins, bag, PC items, flags, vars, game stats, object events, berry trees, secret bases, decorations, TV, mail, day care, roamers, mystery gift, dex seen/caught |
| SaveBlock3 | 1624 bytes (116 spare bytes in each of 14 sectors) | Fake RTC, NPC follower, item flags, DexNav, apricorn trees |
| PokemonStorage | 35712 bytes (9 sectors) | Boxes |

Free space: `include/config/save.h` has `FREE_*` switches that remove vanilla data you do not use (mystery gift 876 bytes, hall records 1032, and so on, 3790 bytes total). The Battle Frontier struct inside SaveBlock2 is also large if you ever remove the Frontier.

**Adding persistent data**:

1. Add the field to `struct SaveBlock1` (or 2 or 3) in `include/global.h`. Prefer the end of the struct. Use the smallest type that fits, per the style guide.
2. Initialise it in `NewGameInitData` (`src/new_game.c`) if zero is not the right default. `ClearSav1`/`ClearSav2`/`ClearSav3` zero everything first.
3. Access it from C directly, and from scripts through a special or callnative (section 4).
4. Existing save files are now misaligned. Start a new game. There is no save migration in expansion; the project's own policy is to batch save-breaking changes. For a sandbox: just delete the `.sav` next to the ROM.

Encrypted fields: `money`, `coins`, bag quantities, and some others are XOR'd with `gSaveBlock2Ptr->encryptionKey`. Use the accessor functions (`GetMoney`, `AddBagItem`, ...). If you add an encrypted field, mirror what `money.c` does and register it in `ApplyNewEncryptionKeyToAllEncryptedData` (`src/load_save.c`). For new fields, do not encrypt; it only exists to defeat cheat devices.

## 3. Event scripts

Files: `data/scripts/*.inc`, `data/maps/*/scripts.inc`. Language reference: `asm/macros/event.inc` (every macro has a comment). Interpreter: `src/script.c`. Commands: `src/scrcmd.c`.

A script is a label with `::` (global) or `:` (file-local), commands one per line, ending in `end` or `return`. Text is a label followed by `.string "…$"`.

```
MyMap_EventScript_Guard::
	lock
	faceplayer
	goto_if_set FLAG_PAID_GUARD, MyMap_EventScript_Guard_Paid
	msgbox MyMap_Text_PayMe, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MyMap_EventScript_Guard_Refused
	checkmoney 500
	goto_if_eq VAR_RESULT, FALSE, MyMap_EventScript_Guard_NoMoney
	removemoney 500
	setflag FLAG_PAID_GUARD
	msgbox MyMap_Text_Thanks, MSGBOX_DEFAULT
	release
	end

MyMap_EventScript_Guard_Paid::
	msgbox MyMap_Text_GoAhead, MSGBOX_DEFAULT
	release
	end

MyMap_Text_PayMe:
	.string "500 to pass. Pay?$"
```

Core vocabulary:

| Need | Commands |
|---|---|
| Freeze the world for a cutscene | `lock` / `lockall` at the start, `release` / `releaseall` at the end. `faceplayer` turns the NPC |
| Show text | `msgbox Text, MSGBOX_DEFAULT` (wait for button), `MSGBOX_NPC` (adds lock/faceplayer/release), `MSGBOX_SIGN`, `MSGBOX_YESNO` (result in `VAR_RESULT`), `MSGBOX_AUTOCLOSE`. Lower level: `message`, `waitmessage`, `waitbuttonpress`, `closemessage` |
| Branch | `goto_if_set/unset FLAG`, `compare VAR, n` + `goto_if_eq/ne/lt/gt/le/ge Label` (or the combined `goto_if_eq VAR, n, Label`), `switch VAR` + `case n, Label`, `call`/`return` for subroutines, `goto_if_defeated TRAINER_X` |
| Items | `giveitem ITEM_X, n` (message + fanfare, `VAR_RESULT` FALSE if bag full), `additem`, `removeitem`, `checkitem` |
| Money | `addmoney n`, `removemoney n`, `checkmoney n`, `showmoneybox`, `hidemoneybox` |
| Pokémon | `givemon SPECIES_X, level, item, ball=, nature=, ...` (named args, `VAR_RESULT` = `MON_GIVEN_TO_PARTY` / `PC` / `CANT_GIVE`), `giveegg`, `setwildbattle` + `dowildbattle`, `checkpartysize` via `getpartysize`, `special HealPlayerParty` |
| Trainer battle | `trainerbattle_single TRAINER_X, IntroText, DefeatText[, AfterScript]`, `_double`, `_rematch`, `_no_intro`, `_two_trainers` |
| NPC movement | `applymovement LOCALID_X, Movement_Label` then `waitmovement 0`. Player is `LOCALID_PLAYER` (see `include/constants/event_objects.h`). `addobject`/`removeobject LOCALID` show and hide, `setobjectxy` moves, `setobjectxyperm` changes the saved template position |
| Warp | `warp MAP_X, x, y` (with fade), `warpsilent`, `warpdoor`, `setrespawn HEAL_LOCATION_X`, `setescapewarp` |
| Sound | `playse SE_X`, `playbgm MUS_X, TRUE`, `playfanfare MUS_X` + `waitfanfare`, `fadedefaultbgm` |
| Screen | `fadescreen FADE_TO_BLACK` / `FADE_FROM_BLACK`, `delay frames`, `setweather`/`doweather`, `setmetatile x, y, METATILE_X, collision` for changing the map |
| Menus | `yesnobox x, y`, `dynmultichoice ...` (see the tutorial), `multichoice` |
| Buffers for text | `bufferitemname STR_VAR_1, ITEM_X`, `bufferspeciesname`, `buffernumberstring STR_VAR_1, VAR_X`, `bufferstring`, `bufferpartymonnick` then use `{STR_VAR_1}` in text |
| Random | `random n` puts 0..n-1 in `VAR_RESULT` |
| Misc | `incrementgamestat`, `settrainerflag`, `setvar VAR_0x8004, x` to pass arguments to a `special` |

**Execution model**: one global script context. When a command returns `TRUE` the interpreter yields until next frame (e.g. `waitmessage`, `delay`). `waitstate` stops the context entirely until C code calls `ScriptContext_Enable()`, which is how specials that open a screen hand control back: the screen returns through `CB2_ReturnToFieldContinueScript` or a warp-exit field callback whose task ends by re-enabling the context. Scripts marked `waitstate=1` in `data/specials.inc` get the `waitstate` inserted automatically after `special Name`.

**Map scripts** (in `<Map>_MapScripts::`): `MAP_SCRIPT_ON_TRANSITION` for setting up state before the map is shown, `ON_LOAD` for `setmetatile` changes before drawing, `ON_FRAME_TABLE` (`map_script_2 VAR, value, Script` rows, run every frame until one matches, then it is your cutscene), `ON_WARP_INTO_MAP_TABLE` for positioning the player, `ON_RESUME` on entering and every return from a menu or battle, `ON_RETURN_TO_FIELD`. Order and meaning documented in `include/constants/map_scripts.h`.

**Registering a script file**: `.include "data/scripts/myfeature.inc"` in `data/event_scripts.s`. Map `scripts.inc` files are already included by the generated map section.

**Using script labels from C**: declare `extern const u8 MyScript[];` (`include/event_scripts.h` collects these) then `ScriptContext_SetupScript(MyScript)` to run it in the overworld, or `RunScriptImmediately(MyScript)` for scripts with no waits (used by `NewGameInitData`).

## 4. Calling C from scripts

Three mechanisms, choose by need:

**A. `special`** (no arguments, uses special vars). Add `def_special MyFunc` at the end of `data/specials.inc`. Write `void MyFunc(void)` in any `src/*.c`, non-static, declared in a header. Pass inputs through `VAR_0x8004` etc, return through `gSpecialVar_Result`, or make it `u16 MyFunc(void)` and call it with `specialvar VAR_RESULT, MyFunc`. Add `waitstate=1` if the function opens a screen or starts a task and will call `ScriptContext_Enable()` later. This is the mechanism vanilla uses 622 times. It is enough for almost everything.

```c
// src/myfeature.c
void GetGuildRank(void)
{
    gSpecialVar_Result = gSaveBlock1Ptr->vars[VAR_GUILD_RANK - VARS_START]; // or VarGet(VAR_GUILD_RANK)
}
```

**B. `callnative`** (typed arguments). Write `void MyCmd(struct ScriptContext *ctx)` that reads operands with `ScriptReadByte`, `ScriptReadHalfword`, `ScriptReadWord`, optionally through `VarGet` so callers may pass vars. Add a macro in `asm/macros/event.inc`:

```
	.macro paytribute amount:req, toWhom:req
	callnative ScrCmd_PayTribute
	.4byte \amount
	.2byte \toWhom
	.endm
```

Set `ctx->waitAfterCallNative = TRUE` if it needs to yield. `ScrCmd_createmon` in `src/script_pokemon_util.c` is the model.

**C. A new opcode**. Append a `script_cmd_table_entry SCR_OP_X ScrCmd_x` line to `data/script_cmd_table.inc`, implement `bool8 ScrCmd_x(struct ScriptContext *ctx)` in `src/scrcmd.c` (return `TRUE` to yield a frame), add the macro. The constants header regenerates automatically. Only worth it for commands used everywhere; A and B are simpler.

Every command and special should call `Script_RequestEffects(SCREFF_V1 | SCREFF_SAVE)` (or `SCREFF_V1` if it has no side effects) near the top. This is the expansion's static analysis for scripts; the `requests_effects=1` flag in the tables marks that the function does so. If you skip it the analysis just treats the function as opaque, which is safe.

## 5. Triggers: where the engine calls you

| You want code to run | Hook |
|---|---|
| When the player talks to an NPC or reads a sign | The `script` field of the object event / bg event in `map.json` |
| When the player steps on a tile | `coord_events` `trigger` in `map.json` (var-gated), or `MB_*` metatile behaviour checks in `TryStartStepBasedScript` / `ProcessPlayerFieldInput` (`src/field_control_avatar.c`) for something global |
| Every step, globally | `ProcessPlayerFieldInput` when `input->tookStep`. Vanilla does poison, egg hatching, repel here. Add `if (input->tookStep && MyStepHook()) return TRUE;` |
| Every frame in the overworld | `ON_FRAME_TABLE` map script (per map), or a task created on map load, or a check in `ProcessPlayerFieldInput` (runs only when no script is active) |
| A button press anywhere in the field | `ProcessPlayerFieldInput` has `pressedStartButton`, `pressedSelectButton`, `pressedRButton` examples |
| When a map loads | `ON_TRANSITION` / `ON_LOAD` / `ON_RESUME` map scripts; globally `CB2_LoadMap` in `src/overworld.c` or `RunOnLoadMapScript` in `src/script.c` |
| When a battle ends | `HandleEndTurn_BattleWon` / `BattleLost` / `RanFromBattle` in `src/battle_main.c` for battle-side effects; `CB2_EndWildBattle` / `CB2_EndTrainerBattle` in `src/battle_setup.c` for overworld-side effects; `gBattleOutcome` tells the result and `gSpecialVar_Result` receives it for trainer scripts |
| When a Pokémon is caught | `Cmd_givecaughtmon` in `src/battle_script_commands.c` (copies into the party or via `CopyMonToPC`), then `gBattleOutcome == B_OUTCOME_CAUGHT` at battle end |
| When a Pokémon levels up or evolves | `MonTryLearningNewMove`, `GetEvolutionTargetSpecies` in `src/pokemon.c`; evolution scene `src/evolution_scene.c` |
| When money changes | `AddMoney` / `RemoveMoney` in `src/money.c` (single choke point) |
| When an item is used | Its `fieldUseFunc` in `src/item_use.c` |
| When the day changes / every minute | `UpdatePerDay` / `UpdatePerMinute` in `src/clock.c` |
| When a new game starts | `NewGameInitData` in `src/new_game.c` and `EventScript_ResetAllMapFlags` in `data/scripts/new_game.inc` |
| When the game is saved or loaded | `src/save.c` `WriteSaveBlock*`, `src/load_save.c` `LoadPlayerParty` etc. Rarely needed |
| When a trainer sees the player | `CheckForTrainersWantingBattle` in `src/trainer_see.c`; the approach scripts are built in `BattleSetup_ConfigureApproachingTrainerBattle` (`src/battle_setup.c`) |
| When a wild encounter is about to start | `CheckStandardWildEncounter` → `TryGenerateWildMon` in `src/wild_encounter.c` |
| In the Start menu | `src/start_menu.c`: add a `MENU_ACTION_X`, entry in `sStartMenuItems`, callback, and an `AddStartMenuAction` line in `BuildNormalStartMenu` |
| Damage calculation | `CalculateMoveDamage` and its helpers in `src/battle_util.c` |
| Each turn start / end in battle | `TryDoEventsBeforeFirstTurn`, `BattleTurnPassed` in `src/battle_main.c`; end-turn effects table in `src/battle_end_turn.c` |

## 6. Text and message boxes

**Encoding**: strings are encoded at build time with `charmap.txt`. In C use `_("Text")` for a `const u8[]` literal and `COMPOUND_STRING("Text")` inside struct initialisers (it deduplicates identical strings). In `.inc` files, `.string "Text$"`. Special characters: `\n` newline, `\l` scroll one line, `\p` new page after button, `{PLAYER}`, `{RIVAL}`, `{STR_VAR_1}`, `{STR_VAR_2}`, `{STR_VAR_3}`, `{COLOR RED}`/`{COLOR BLUE}`/`{COLOR LIGHT_GRAY}` etc, `{PAUSE 30}`, `{PKMN}`, `{POKEBLOCK}`. Names in `charmap.txt` between the `FD`/`FC` sections.

**Buffers**: `gStringVar1`, `gStringVar2`, `gStringVar3` (256 bytes) are the placeholders `{STR_VAR_n}`; `gStringVar4` (1000 bytes) is the conventional output buffer. `StringExpandPlaceholders(gStringVar4, gText_Template)` expands `{...}`. `ConvertIntToDecimalStringN(gStringVar1, value, STR_CONV_MODE_LEFT_ALIGN, digits)` for numbers. `StringCopy`, `StringAppend`, `GetSpeciesName`, `CopyItemName`, `GetMonNickname`.

**In the field** (from C): `ShowFieldMessage(str)` shows the dialogue box; `IsFieldMessageBoxHidden()` to poll; `HideFieldMessageBox()`. From a script it is `msgbox`. The vanilla special `ShowFieldMessageStringVar4` shows whatever C put in `gStringVar4`, so the common C-to-script text path is: C fills `gStringVar4`, script does `special ShowFieldMessageStringVar4` + `waitmessage` + `waitbuttonpress`.

**In a custom screen**: a window template gives a rectangle of tiles; `AddWindow(&template)` returns an id; `FillWindowPixelBuffer(id, PIXEL_FILL(0))`; `AddTextPrinterParameterized(id, FONT_NORMAL, str, x, y, TEXT_SKIP_DRAW, NULL)` draws text; `PutWindowTilemap(id)` and `CopyWindowToVram(id, COPYWIN_FULL)` show it. `DrawStdWindowFrame(id, FALSE)` draws the border. Fonts: `FONT_NORMAL`, `FONT_SMALL`, `FONT_NARROW`, `FONT_SHORT`. Colours through `AddTextPrinterParameterized3` with a `{bg, fg, shadow}` triple indexing the window's palette.

**Name box** for speaker names above dialogue: `setspeaker` script command, `docs/tutorials/how_to_namebox.md`.

## 7. Menus and full screens

**Quick option menu inside the overworld** (like the Start menu): `PrintMenuTable(windowId, count, menuActions)` plus `InitMenuInUpperLeftCornerNormal(windowId, count, cursorPos)` then poll `Menu_ProcessInput()` each frame from a task; returns the index, `MENU_B_PRESSED`, or `MENU_NOTHING_CHOSEN`. `src/start_menu.c` is the model.

**Scrolling list**: `struct ListMenuTemplate` with an array of `struct ListMenuItem {name, id}`, `ListMenuInit(&template, scroll, row)` returns a task id, `ListMenu_ProcessInput(taskId)` returns the chosen id, `LIST_NOTHING_CHOSEN` or `LIST_CANCEL`, then `DestroyListMenuTask`. `src/debug.c` uses this for every submenu and is easy to copy from. `src/shop.c` and `src/item_menu.c` are the heavier versions with icons and descriptions.

**Yes/No**: `CreateYesNoMenu(&windowTemplate, baseTile, palette, initialCursor)` then `Menu_ProcessInputNoWrapClearOnChoose()` returns 0 yes, 1 no, `MENU_B_PRESSED`.

**From a script**: `dynmultichoice left, top, ignoreB, maxShown, initial, DYN_MULTICHOICE_CB_NONE, Text1, Text2, ...` puts the index in `VAR_RESULT`. Simplest way to offer choices with no C at all.

**A full screen** (new UI that replaces the overworld). Copy `src/diploma.c`. The skeleton:

1. Entry: `void CB2_ShowMyScreen(void)`. Reset GPU registers, clear VRAM/OAM/palettes, `ScanlineEffect_Stop`, `ResetTasks`, `ResetSpriteData`, `ResetPaletteFade`, `FreeAllSpritePalettes`.
2. Backgrounds: `InitBgsFromTemplates(0, templates, count)`, allocate a tilemap buffer with `Alloc`, `SetBgTilemapBuffer`, load tiles with `DecompressAndCopyTileDataToVram`, palettes with `LoadPalette`, `ShowBg(n)`.
3. Windows: `InitWindows(templates)`, `DeactivateAllTextPrinters`, load the menu palette `LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP)`, print.
4. `BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK)`, `SetVBlankCallback(VBlankCB)` (which does `LoadOam(); ProcessSpriteCopyRequests(); TransferPlttBuffer();`), `SetMainCallback2(MainCB2)` (which does `RunTasks(); AnimateSprites(); BuildOamBuffer(); UpdatePaletteFade();`), `CreateTask(Task_MyScreenInput, 0)`.
5. The task waits for `!gPaletteFade.active`, then reads `JOY_NEW(A_BUTTON)` etc. On exit: fade out, `Free` buffers, `FreeAllWindowBuffers`, `DestroyTask`, `SetMainCallback2(CB2_ReturnToFieldFadeFromBlack)` or `CB2_ReturnToFieldContinueScript` if a script is waiting.

To open it from the overworld: a special that does `SetMainCallback2(CB2_ShowMyScreen); LockPlayerFieldControls();` with `waitstate=1`, or a Start menu callback that fades out then switches (`StartMenuPokedexCallback` shows the fade-then-switch shape).

Party selection, Pokémon summary, bag, PC are their own screens with entry points (`ShowPokemonSummaryScreen`, `CB2_BagMenuFromStartMenu`, `ChooseMonForMoveRelearner`-style helpers in `src/party_menu.c`). Reuse them rather than rewriting.

## 8. Sprites and graphics

**Backgrounds**: 4 layers, each a tilemap of 8x8 tiles. Tiles come from a `.png` compiled to `.4bpp` (16 colours). A palette is 16 `u16` colours. `INCGFX_U32("graphics/mine/tiles.png", ".4bpp.smol")` in C gives compressed tiles; `INCBIN_U16("graphics/mine/tiles.gbapal")` gives a palette converted from a `.pal` file; `INCGFX_U32("graphics/mine/map.bin", ".smolTM")` a compressed tilemap. The image must be indexed PNG with at most 16 colours, first colour is transparent. Tilemaps can be authored as `.bin` (Porymap/Tilemap Studio) or built at runtime by writing to the buffer from `SetBgTilemapBuffer`.

**Sprites**: `struct SpriteTemplate` needs an `OamData` (size, shape, priority), a tile tag, a palette tag, an anim table, and a callback. Load resources by tag once: `LoadCompressedSpriteSheet(&sheet)` (`struct CompressedSpriteSheet {data, size, tag}`) and `LoadSpritePalette(&pal)`. Then `CreateSprite(&template, x, y, subpriority)` returns an index into `gSprites`. The callback runs each frame; `sprite->data[0..7]` are scratch. `DestroySprite` when done; `FreeSpriteTilesByTag` / `FreeSpritePaletteByTag` to release. `src/money.c` (the money label sprite) is a small complete example; `src/item_icon.c` shows drawing an item icon anywhere.

**NPC graphics on the map**: add a PNG under `graphics/object_events/pics/people/`, declare it in `src/data/object_events/object_event_graphics.h`, a frame table in `object_event_pic_tables.h`, a `struct ObjectEventGraphicsInfo` in `object_event_graphics_info.h`, a pointer in `object_event_graphics_info_pointers.h`, and an `OBJ_EVENT_GFX_X` constant in `include/constants/event_objects.h`. Standard sizes 16x32 (people) or 32x32; frames in the order down, up, left, down-walk-1, up-walk-1, left-walk-1, down-walk-2, up-walk-2, left-walk-2 (right is left mirrored). Any Pokémon already works as `OBJ_EVENT_GFX_SPECIES(NAME)`.

**Palettes** are the scarce resource: 16 background palettes and 16 sprite palettes, and the overworld already uses most. Day/night tinting (`OW_ENABLE_DNS`) reads the map palettes, so custom overworld sprites should use the shared NPC palettes where possible.

**Pokémon and trainer sprites** have dedicated pipelines; follow `docs/tutorials/how_to_new_pokemon.md` (Graphics section) and `how_to_trainer_front_pic.md`.

## 9. Battle engine hooks

Structures: `gBattleMons[battler]` (`struct BattlePokemon`: species, stats, stat stages, moves, PP, HP, status1, status2, types, ability, item), `gBattleStruct` (`struct BattleStruct` in `include/battle.h`, everything else per battle), `gBattleTypeFlags`, `gBattlerAttacker`, `gBattlerTarget`, `gCurrentMove`, `gBattleMoveDamage`, `gBattleOutcome`, `gBattleScripting`. Battler ids: 0 player left, 1 opponent left, 2 player right, 3 opponent right; `GetBattlerSide(battler)` gives `B_SIDE_PLAYER` / `B_SIDE_OPPONENT`.

Battle scripts: a script runs when something sets `gBattlescriptCurrInstr`. From C, `BattleScriptCall(BattleScript_X)` (push return, run X, come back), `BattleScriptExecute(BattleScript_X)` (replace), `BattleScriptPushCursorAndCallback`. Scripts print with `printstring STRINGID_X`, animate with `attackanimation`/`playanimation`, change HP with `datahpupdate`, change stats with `statbuffchange`, and end with `end` or `return`. Labels are declared for C in `include/battle_scripts.h`.

Where mechanics attach:

| Mechanic | Attach point |
|---|---|
| New move effect | `EFFECT_X` constant + `src/data/battle_move_effects.h` entry naming `BattleScript_EffectX` + the script in `data/battle_scripts_1.s`. For a damaging move with a twist, `.effect = EFFECT_HIT` plus `.additionalEffects` is often enough with no script |
| New secondary effect | `MOVE_EFFECT_X` in `include/constants/battle.h`, case in `SetMoveEffect` (`src/battle_set_effect.c`) |
| New ability | Constant, `gAbilitiesInfo` entry, then a `case ABILITY_X:` under the right `ABILITYEFFECT_*` in `AbilityBattleEffects` (`src/battle_util.c`): `ON_SWITCHIN` for Intimidate-likes, `ENDTURN` for Speed Boost-likes, `MOVE_END` for contact effects on the defender, `MOVE_END_ATTACKER` for the attacker, `IMMUNITY` for status immunities. Damage or stat multipliers go in the damage calc (`CalcAttackStat`, `CalcDefenseStat`, `CalcMoveBasePowerAfterModifiers`, type effectiveness functions). Speed: `GetBattlerTotalSpeedStat`. Tests: `test/battle/ability/` |
| New hold effect | `HOLD_EFFECT_X` constant, `.holdEffect` on the item, a `gHoldEffectsInfo[HOLD_EFFECT_X]` entry setting which timing bits it activates on (`onSwitchIn`, `leftovers` end-turn, `onTargetAfterHit`, `onHpThreshold`, ...), then a `case HOLD_EFFECT_X:` in `ItemBattleEffects` in `src/battle_hold_effects.c`. The engine calls `ItemBattleEffects(battler, ..., IsOnSwitchInActivation)` and similar predicates at each timing; the predicate reads the info bits |
| New battle-wide rule (a "mode") | A flag or var checked at the relevant points. Examples to copy: `B_FLAG_INVERSE_BATTLE` (type chart), `B_FLAG_SLEEP_CLAUSE`, `B_VAR_NO_BAG_USE` (menu), `B_FLAG_NO_WHITEOUT`. Or a new `BATTLE_TYPE_*` bit if it changes setup; there are two free bits (`BATTLE_TYPE_14`, `BATTLE_TYPE_30`) |
| Change what happens on win/loss | `HandleEndTurn_BattleWon` / `BattleLost` in `src/battle_main.c`. Money from trainers: `Cmd_getmoneyreward` in `src/battle_script_commands.c`. Money lost on whiteout: `B_WHITEOUT_MONEY` logic in the same file |
| Exp and EV gain | `Cmd_getexp` in `src/battle_script_commands.c`, caps in `src/caps.c` |
| Catch rate | `Cmd_handleballthrow` in `src/battle_script_commands.c` |
| Turn order | `SetActionsAndBattlersTurnOrder`, `GetWhichBattlerFaster` in `src/battle_main.c` |
| AI decisions | `src/battle_ai_main.c` scoring, gated by AI flags |
| Battle UI | `src/battle_interface.c` (health boxes), `src/battle_controller_player.c` (menus), `src/battle_message.c` (strings) |
| Starting a battle from anywhere | Set `gBattleTypeFlags`, set `gMain.savedCallback` to the function to return to, call `CreateBattleStartTask(transition, music)` as `DoStandardWildBattle` does; for trainers fill `TRAINER_BATTLE_PARAM` and use `BattleSetup_StartTrainerBattle`. From scripts, `setwildbattle`/`dowildbattle` and `trainerbattle_*` |

The `test/battle/` suite is the safety net for anything here. Battle tests are fast to write; see `08-testing-and-debugging.md`.

## 10. Time

`gLocalTime` (`struct Time`: days, hours, minutes, seconds) is refreshed by `RtcCalcLocalTime()`. `GetTimeOfDay()` returns `TIME_MORNING` / `TIME_DAY` / `TIME_EVENING` / `TIME_NIGHT` per `OW_TIMES_OF_DAY`. The real clock is the cartridge RTC, which mGBA emulates from the host clock. With `OW_USE_FAKE_RTC TRUE` time advances only while playing, at the rate `OW_ALTERED_TIME_RATIO`, and is stored in `SaveBlock3`; scripts can `addtime`, `fwdtime`, `pausefakertc`.

Per-day logic: `UpdatePerDay(days)` in `src/clock.c` is called with the number of days elapsed since the last check; berry growth, Shoal Cave tides and mirage island already hook there. `gSaveBlock1Ptr->dailySeed` gives deterministic "today's random" values (`LocalRandomSeed(dailySeed ^ SALT)`). Daily flags clear each day. Day of week: `GetDayOfWeek()`.

## 11. Memory and allocation

- `EWRAM_DATA static u8 sMyState;` for globals. Zero-initialised at boot, not saved. Keep them small; EWRAM is 256 KB total including the 116 KB heap and the save blocks.
- `Alloc(size)` / `AllocZeroed(size)` / `Free(ptr)` from `include/malloc.h` for screen-lifetime buffers (tilemaps, list data). Always free on exit, the heap is not garbage collected and leaks crash later screens.
- `COMMON_DATA` / `IWRAM_DATA` for hot small variables; IWRAM is 32 KB and shared with the stack.
- Large `const` tables belong in ROM: just make them `static const` at file scope, the linker puts `.rodata` in ROM.
- ROM budget: 32 MB, currently ~26.7 MB used. Disable Pokémon families or cries in `species_enabled.h` / `pokemon.h` if you need room.
- The linker prints `EWRAM`, `IWRAM`, `ROM` usage after every build; watch IWRAM in particular.

## 12. Random numbers

`Random()` returns `u16`, `Random32()` returns `u32`, `RandomUniform(tag, lo, hi)`, `RandomPercentage(tag, pct)`, `RandomElement(tag, array)` (`include/random.h`). The tag argument (`RNG_NONE` or an `RNG_*` enum) exists so tests can rig outcomes; use `RNG_NONE` outside battle. In scripts, `random n`. Seeded from the RTC and advanced every VBlank, so it is not reproducible across runs unless you seed explicitly.
