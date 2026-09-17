# 01. Mental model of the engine

This file explains how the game runs, so that you can reason about where any new thing belongs. Everything else in the guide assumes this.

## The hardware you are targeting

- **CPU**: ARM7TDMI at 16.78 MHz, no floating point, no OS. Every frame is 1/60 s. All code is yours: there is no runtime under it.
- **Memory** (from `ld_script_modern.ld` and `include/gba/defines.h`):
  - EWRAM: 256 KB. All `EWRAM_DATA` globals and the heap live here. The save blocks live here while playing.
  - IWRAM: 32 KB, fast. `COMMON_DATA` and `IWRAM_DATA` globals and the stack.
  - ROM: 32 MB max. Code, data tables, graphics, sound. The current build uses about 26.7 MB (80%). The linker prints the usage at the end of every build.
  - VRAM: 96 KB for backgrounds and sprites, 1 KB palette RAM (512 colours as 32 palettes of 16), 1 KB OAM for up to 128 sprites.
- **Save**: 128 KB flash, split into 32 sectors of 4 KB. Two rotating save slots use 14 sectors each. See "Save data" in `05-engine-systems.md`.
- **Graphics**: tile based. Backgrounds are grids of 8x8 tiles from a tileset with 16-colour palettes. Sprites are also tiles. Text is drawn by rendering glyphs into a window's tiles. There is no framebuffer. This is why "add a UI" always means "define tiles, palettes, a window template, and print into it".

## From source to ROM

`make` runs the toolchain in `tools/` and devkitARM to produce `pokeemerald.gba`. The main transformations:

| Input | Tool | Output | Notes |
|---|---|---|---|
| `src/**/*.c` | cpp → `preproc` → cc1 → as | object files | `preproc` is a custom pass that encodes `_("text")` and `COMPOUND_STRING` using `charmap.txt`, and expands `INCBIN`/`INCGFX` |
| `src/data/*.h` | included into `.c` files | part of the object above | Most game data is C tables in headers, included once. They are not separately compiled |
| `data/*.s`, `data/**/*.inc` | `preproc` → cpp → as | object files | Overworld scripts, battle scripts, animation scripts, map data. Assembly syntax, but really a bytecode built from macros in `asm/macros/` |
| `data/maps/*/map.json`, `data/layouts/layouts.json`, `map_groups.json` | `mapjson` | `*.inc` files and `include/constants/map_groups.h`, `layouts.h`, `map_event_ids.h` | Generated files are git-ignored. Never edit them |
| `graphics/**/*.png`, `*.pal` | `gbagfx` | `.4bpp`, `.8bpp`, `.gbapal`, `.lz` | PNG must be indexed colour, 16 colours per palette |
| any graphic | `compresSmol` | `.smol`, `.smolTM` | The expansion's compression. `INCGFX_U32(path, ".4bpp.smol")` in C picks the pipeline |
| `sound/songs/midi/*.mid` + `midi.cfg` | `mid2agb` | `.s` | Music. Cries are `.wav` via `wav2agb` |
| `src/data/trainers.party` | `trainerproc` | `src/data/trainers.h` | Trainers written in Pokémon Showdown syntax |
| `src/data/wild_encounters.json` | `tools/wild_encounters/*.py` | `src/data/wild_encounters.h` | |
| `src/data/pokemon/all_learnables.json` + TMs + tutors | `tools/learnset_helpers/*.py` | `teachable_learnsets.h` | Regenerated when TMs or tutor scripts change |
| `data/script_cmd_table.inc` | `tools/misc/make_scr_cmd_constants.py` | `include/constants/script_commands.h` | Script opcode numbers |

Rules that follow from this:

- Any new `src/**/*.c` file is compiled automatically. The Makefile uses wildcards.
- Any new `.inc` script file must be pulled in with a `.include` line in `data/event_scripts.s`, or it is not in the ROM.
- Any new map is registered by `mapjson` from `map_groups.json`. Its `scripts.inc` still needs the `.include` in `data/event_scripts.s`.
- Any new graphic is included from C with `INCGFX_U32("graphics/x/y.png", ".4bpp.smol")` or `INCBIN_U16("graphics/x/y.gbapal")`. The Makefile builds the converted file on demand from the extension you ask for.
- Configuration is `#define` in `include/config/*.h`. Changing one triggers a large rebuild because `global.h` includes them.

## What runs every frame

`src/main.c`:

1. `AgbMain` initialises hardware, then loops forever in `AgbMainLoop`.
2. Each iteration: read the joypad into `gMain.newKeys` / `gMain.heldKeys`, call `gMain.callback1` if set, call `gMain.callback2`, update play time and music, then wait for VBlank.
3. The VBlank interrupt (`VBlankIntr`) runs `gMain.vblankCallback`, copies buffered GPU registers, processes DMA requests, runs the sound engine, and advances the RNG.

`gMain.callback2` is "the current screen". Every screen in the game is a function pointer installed with `SetMainCallback2`:

- `CB2_Overworld` while walking around
- `BattleMainCB2` in battle
- `CB2_BagMenuRun`, `CB2_PartyMenuMain`, `CB2_ShowDiploma`, and so on for each menu

`gMain.callback1` is used by the overworld (`CB1_Overworld`) for field input and by link play. `gMain.savedCallback` is the conventional "return here when done" pointer.

Switching screens is therefore: fade to black, save any state, `SetMainCallback2(NewScreenInit)`. The init function resets the GPU, loads graphics, creates tasks, and installs its own steady-state `CB2` that runs tasks each frame. Returning is `SetMainCallback2(CB2_ReturnToFieldFadeFromBlack)` or whatever was saved. `src/diploma.c` is the smallest complete example of this pattern (about 200 lines) and is worth reading once.

## Tasks

`src/task.c`: a fixed table of 16 tasks (`gTasks`). Each has a function pointer, a priority, and 16 `s16` data slots. `RunTasks()` calls every active task's function once per frame, in priority order. Tasks are how the game runs state machines over time without threads:

```c
static void Task_MyThing(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (data[0])
    {
    case 0: /* start a fade */ data[0]++; break;
    case 1: if (!gPaletteFade.active) data[0]++; break;
    case 2: DestroyTask(taskId); break;
    }
}
// somewhere: CreateTask(Task_MyThing, 0);
```

Conventions: `data[0]` is usually a state counter, `#define tState data[0]` style aliases are common, and `SetWordTaskArg`/`GetWordTaskArg` store 32-bit values across two slots. Tasks are per screen: `ResetTasks()` is called when a screen initialises.

## Sprites

`src/sprite.c`: up to 64 `struct Sprite` objects (`gSprites`), each with a callback run every frame by `AnimateSprites()`, its own `data[8]`, position, animation and OAM settings. Created with `CreateSprite(&template, x, y, subpriority)` from a `struct SpriteTemplate` that names a tile sheet, a palette, and animation tables. Sprite graphics are loaded into VRAM with `LoadCompressedSpriteSheet` / `LoadSpritePalette` and are found again by numeric tags.

## The three "languages"

1. **C** in `src/`. All logic.
2. **Overworld event scripts** in `data/scripts/*.inc` and `data/maps/*/scripts.inc`. Bytecode interpreted by `src/script.c`; commands implemented in `src/scrcmd.c`; macros that emit the bytes in `asm/macros/event.inc`. This is how NPCs talk, give items, start battles, warp, set flags. It runs cooperatively inside the overworld frame, one script at a time, and can yield with `waitstate` while C code does something.
3. **Battle scripts** in `data/battle_scripts_1.s` and `data/battle_scripts_2.s`. A separate bytecode interpreted by `RunBattleScriptCommands` in `src/battle_main.c`; commands in `src/battle_script_commands.c`; macros in `asm/macros/battle_script.inc`. Every move effect, ability activation and end-of-turn effect is a battle script that prints messages, plays animations and applies damage.

Both script languages can call arbitrary C: `special` and `callnative` in event scripts, `callnative` in battle scripts. So the rule for new features is: write the logic in C, expose a thin script command, and use the script for sequencing and text.

## The overworld loop

`src/overworld.c` and `src/field_control_avatar.c`:

- `CB2_LoadMap` loads a map: layout, tilesets, object events (NPCs), runs the map's `ON_LOAD` and `ON_TRANSITION` scripts, spawns sprites, fades in.
- Every frame `CB1_Overworld` builds a `struct FieldInput` from the joypad and calls `ProcessPlayerFieldInput`, which is an ordered list of checks: trainers who see you, `ON_FRAME` map scripts, step-based events (poison, egg hatch, repel, coord triggers, wild encounters), A-button interaction with the tile or NPC in front of you, doors, Start menu, Select registered item, R-button DexNav, the debug menu combo. The first check that returns TRUE wins for that frame.
- Interaction finds a script pointer (NPC's `script` field from the map JSON, sign script, PC, etc.) and runs it with `ScriptContext_SetupScript`. While a script runs, player input is locked.
- `CB2_Overworld` then runs tasks, animates sprites, updates the camera and weather, and draws.

So "the player does X near Y" is almost always either a map event in the JSON, or a new check inserted in `ProcessPlayerFieldInput`.

## The battle loop

`src/battle_main.c`:

1. Something calls `CB2_InitBattle` after setting `gBattleTypeFlags` (which kind of battle) and, for trainers, `TRAINER_BATTLE_PARAM`. `src/battle_setup.c` has the entry points: `BattleSetup_StartWildBattle`, `BattleSetup_StartTrainerBattle`, scripted variants.
2. `CB2_HandleStartBattle` builds `gBattleMons[]` from the party, sets up controllers (one per battler: player, opponent AI, partner, link), and installs `BattleMainCB2`.
3. `BattleMainCB2` each frame runs `gBattleMainFunc`, a state pointer that walks: `DoBattleIntro` → `TryDoEventsBeforeFirstTurn` → `HandleTurnActionSelectionState` (menus, AI decides) → `SetActionsAndBattlersTurnOrder` → `RunTurnActionsFunctions` (each action such as `HandleAction_UseMove` starts a battle script) → `BattleTurnPassed` → back to action selection, until `gBattleOutcome` is set. Then `sEndTurnFuncsTable` picks `HandleEndTurn_BattleWon` / `BattleLost` / `RanFromBattle`, then `HandleEndTurn_FinishBattle` writes results back to the party, and `ReturnFromBattleToOverworld` calls `gMain.savedCallback`.
4. Battle scripts run through `gBattlescriptCurrInstr`. Abilities and items hook in through `AbilityBattleEffects(caseId, ...)` in `src/battle_util.c` and `ItemBattleEffects(...)` in `src/battle_hold_effects.c`, which are called at fixed points (switch-in, end of turn, after a move hits, ...) and switch on the ability or hold effect.

Key globals: `gBattleTypeFlags`, `gBattleMons[4]`, `gBattleStruct` (large per-battle scratch state), `gBattlerAttacker`, `gBattlerTarget`, `gCurrentMove`, `gBattleMoveDamage`, `gBattleOutcome`.

## Persistence

Three save structs in `include/global.h`: `SaveBlock1` (world state: position, party, money, bag, flags, vars, NPC state, berry trees, day care, ...), `SaveBlock2` (player identity, options, Pokédex, Battle Frontier records), `SaveBlock3` (expansion additions, small). Plus `PokemonStorage` (boxes). In RAM they are reached through `gSaveBlock1Ptr`, `gSaveBlock2Ptr`, `gSaveBlock3Ptr`, `gPokemonStoragePtr`. Saving copies them to flash sector by sector. Anything not in these structs is lost on reset.

The generic persistent state you should reach for first: **flags** (2000+ one-bit booleans, `FlagSet`/`FlagGet`) and **vars** (256 sixteen-bit values, `VarSet`/`VarGet`). They are in `SaveBlock1`, usable from both C and scripts, editable in the debug menu, and hundreds are unused. Most new mechanics need nothing else. See `05-engine-systems.md`.

## Text

Strings are not ASCII in ROM. `charmap.txt` maps characters and control codes to bytes. In C you write `_("Hello")` for a string constant or `COMPOUND_STRING("...")` for a deduplicated one, and `preproc` encodes it. In scripts you write `.string "Hello$"` where `$` is the terminator. Placeholders like `{PLAYER}`, `{STR_VAR_1}` are expanded at print time by `StringExpandPlaceholders` into `gStringVar4`. `\n` is a new line, `\l` scrolls, `\p` waits for a button and clears.

## Where does new code go

- **New self-contained feature**: new `src/myfeature.c` + `include/myfeature.h`. It compiles automatically. Expose script commands through `data/specials.inc` (see file 05). This is also what the expansion's style guide asks for: minimally invasive, isolated in its own file.
- **New data table**: `src/data/myfeature.h`, included by your `.c` file.
- **New scripts and text**: `data/scripts/myfeature.inc`, plus `.include "data/scripts/myfeature.inc"` in `data/event_scripts.s`.
- **New persistent state**: flags and vars first. Only add fields to a save struct when you need more than a u16 per item.
- **Hooking into existing flow**: find the dispatch point (for example `ProcessPlayerFieldInput`, `AbilityBattleEffects`, `HandleEndTurn_BattleWon`, `NewGameInitData`, `DoTimeBasedEvents`) and add one call into your file.
