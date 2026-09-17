# 04. Where things live

Lookup table: for each kind of thing, the constant, the data, the logic, the graphics, and the script access. Paths are relative to the repo root.

## Species

| What | Where |
|---|---|
| Id constant | `include/constants/species.h`. `enum` with a `SPECIES_CUSTOM_START` .. `SPECIES_CUSTOM_END` gap for your additions. `NUM_SPECIES` derives from it. Ids are stored in saves, never renumber |
| National dex number | `include/constants/pokedex.h`, and the `.natDexNum` field |
| All data | `src/data/pokemon/species_info/gen_N_families.h`, table `gSpeciesInfo[]`. Struct in `include/pokemon.h` (`struct SpeciesInfo`). Stats, types, catch rate, exp yield, EV yield, held items, gender, egg cycles, growth rate, egg groups, abilities (3 slots), body colour, name, cry id, dex text, height, weight, sprite pointers and sizes, animation ids, icon, footprint, flags (legendary, mythical, ...), learnset pointers, evolutions, form tables, overworld sprite |
| Level-up learnset | `src/data/pokemon/level_up_learnsets/gen_N.h`, `sXLevelUpLearnset[]` |
| Egg moves | `src/data/pokemon/egg_moves.h` |
| TM and tutor learnset | generated `src/data/pokemon/teachable_learnsets.h` from `src/data/pokemon/all_learnables.json`, TM list `include/constants/tms_hms.h`, and tutor scripts (see `docs/tutorials/teachable_learnsets.md`) |
| Evolutions | `.evolutions = EVOLUTION({EVO_LEVEL, 16, SPECIES_X})` in the species entry. Only a few base methods (`EVO_LEVEL`, `EVO_ITEM`, `EVO_TRADE`, `EVO_SCRIPT_TRIGGER`, `EVO_BATTLE_END`, ...) plus stackable `CONDITIONS({IF_MIN_FRIENDSHIP, n}, {IF_TIME, TIME_NIGHT}, {IF_HOLD_ITEM, ITEM_X}, {IF_KNOWS_MOVE, MOVE_X}, ...)`; all `IF_*` in `include/constants/pokemon.h`; logic `GetEvolutionTargetSpecies` in `src/pokemon.c` |
| Forms | `src/data/pokemon/form_species_tables.h` (which species are forms of each other), `form_change_tables.h` (triggers: item hold, mega, weather, HP, time). Types in `include/constants/form_change_types.h` |
| Sprites | `graphics/pokemon/<name>/front.png`, `back.png`, `icon.png`, `normal.pal`, `shiny.pal`, `footprint.png`, `anim_front.png`; declared in `src/data/graphics/pokemon.h`; overworld sprite `overworld.png` + tables in `src/data/object_events/object_event_graphics_info_followers.h` and `object_event_pic_tables_followers.h` |
| Cry | `sound/direct_sound_samples/cries/<name>.aif`, `sound/cry_tables.inc`, `include/constants/cries.h` |
| Runtime instance | `struct Pokemon` / `struct BoxPokemon` in `include/pokemon.h`. Read and write only via `GetMonData(mon, MON_DATA_X)` / `SetMonData(mon, MON_DATA_X, &value)`; field ids in `include/constants/pokemon.h` |
| Player party | `gPlayerParty[PARTY_SIZE]` (aliases `gSaveBlock1Ptr->playerParty`), count `gPlayerPartyCount`. Boxes: `gPokemonStoragePtr->boxes[box][slot]` |
| Creating one | `CreateMon`, `CreateMonWithIVs`, `CreateMonFromTemplate` in `src/pokemon.c`; from scripts `givemon`, `createmon`, `giveegg` (implemented in `src/script_pokemon_util.c`) |
| Enable/disable families | `include/config/species_enabled.h` |

## Moves

| What | Where |
|---|---|
| Id constant | `include/constants/moves.h`. Add custom moves before `MOVES_COUNT` (after the last Gen 9 move), because Z and Max moves follow |
| Data | `src/data/moves_info.h`, `gMovesInfo[]`. Name, description, `.effect`, power, type, accuracy, PP, target, priority, category, `.additionalEffects` (up to 3 secondary effects with chance), flags (`makesContact`, `soundMove`, ...), Z move, contest data |
| Effect behaviour | `.effect = EFFECT_X`. Effects are listed in `include/constants/battle_move_effects.h` and mapped to a battle script in `src/data/battle_move_effects.h`. The script is `BattleScript_EffectX::` in `data/battle_scripts_1.s` |
| Secondary effects | `MOVE_EFFECT_*` in `include/constants/battle.h`, applied by `SetMoveEffect` in `src/battle_set_effect.c` |
| Dynamic power/type/accuracy | `CalcMoveBasePowerAfterModifiers` in `src/battle_util.c`; `SetTypeBeforeUsingMove` in `src/battle_main.c`; accuracy in `src/battle_script_commands.c` |
| Damage formula | `CalculateMoveDamage` and `DoMoveDamageCalc` in `src/battle_util.c` (`struct DamageContext`) |
| Animation | `.battleAnimScript = gBattleAnimMove_X` field in the move's `gMovesInfo` entry, pointing at a script label in `data/battle_anim_scripts.s` (the old `gBattleAnims_Moves` table named in the move tutorial no longer exists). Particle graphics in `graphics/battle_anims/`, sprite templates and tasks in `src/battle_anim*.c` |
| Messages | `include/constants/battle_string_ids.h` + `src/battle_message.c` |
| Access helpers | `GetMovePower(move)`, `GetMoveType`, `GetMoveEffect`, ... in `include/move.h` / `src/move.c` |
| Tutorial | `docs/tutorials/how_to_new_move.md` |

## Abilities

| What | Where |
|---|---|
| Id constant | `include/constants/abilities.h`, `ABILITIES_COUNT` at the end |
| Name, description, AI rating, flags | `src/data/abilities.h`, `gAbilitiesInfo[]` (`struct AbilityInfo` in `include/pokemon.h`) |
| Battle logic | `AbilityBattleEffects(caseID, battler, ability, move, ...)` in `src/battle_util.c`. It is one huge switch on `caseID` (`ABILITYEFFECT_ON_SWITCHIN`, `ABILITYEFFECT_ENDTURN`, `ABILITYEFFECT_MOVE_END`, `ABILITYEFFECT_MOVE_END_ATTACKER`, `ABILITYEFFECT_IMMUNITY`, `ABILITYEFFECT_ON_WEATHER`, `ABILITYEFFECT_ON_TERRAIN`, ...) with an inner switch on the ability. Damage-modifying abilities are in the damage calc functions; speed, priority, and stat abilities are scattered by their effect point. `grep -n "case ABILITY_INTIMIDATE" src/` shows all hook points of an ability |
| Overworld effects | `src/ow_abilities.c` (e.g. Synchronize nature, encounter rate) |
| Popup message and scripts | `BattleScript_AbilityPopUp` and friends in `data/battle_scripts_1.s` |

## Items

| What | Where |
|---|---|
| Id constant | `include/constants/items.h`, `enum Item`, `ITEMS_COUNT` |
| Data | `src/data/items.h`, `gItemsInfo[]` (`struct ItemInfo` in `include/item.h`): `.name`, `.pluralName`, `.price`, `.description`, `.pocket`, `.sortType`, `.type` (`ITEM_USE_BAG_MENU` / `PARTY_MENU` / `FIELD` / ...), `.fieldUseFunc`, `.battleUsage`, `.effect` (for medicines, points to `item_effects.h` tables), `.holdEffect`, `.holdEffectParam`, `.secondaryId` (ball id, TM move), `.importance` (key item, unique), `.flingPower`, `.iconPic`, `.iconPalette`, `.shopCriteriaFunc` |
| Field use behaviour | `src/item_use.c`: one `ItemUseOutOfBattle_X(u8 taskId)` per behaviour. `ItemUseOutOfBattle_CannotUse` for items with no use |
| Party-menu item effects (medicine, candies) | `src/data/pokemon/item_effects.h` + `PokemonUseItemEffects` in `src/pokemon.c` |
| Hold effects in battle | `include/constants/hold_effects.h`; timing bits in `gHoldEffectsInfo[]` and dispatch `ItemBattleEffects` in `src/battle_hold_effects.c`; damage multipliers in the damage calc in `src/battle_util.c` |
| Icons | `graphics/items/icons/<name>.png` + `graphics/items/icon_palettes/<name>.pal`, declared in `src/data/graphics/items.h` |
| Bag storage | `gSaveBlock1Ptr->bag` (`struct Bag` in `global.h`), pocket sizes in `include/constants/global.h` (`BAG_ITEMS_COUNT 30`, key items 30, balls 16, TMs 64, berries 46). API `AddBagItem`, `RemoveBagItem`, `CheckBagHasItem`, `CountTotalItemQuantityInBag` in `src/item.c`. Item quantities are XOR encrypted with `gSaveBlock2Ptr->encryptionKey`, so never touch the array directly |
| PC storage | `gSaveBlock1Ptr->pcItems`, `AddPCItem` |
| Scripts | `giveitem ITEM_X, n` (with message), `additem`, `removeitem`, `checkitem` (result in `VAR_RESULT`) |
| Shops | `pokemart ItemList` in a script; list is `.2byte ITEM_X` lines ending with `pokemartlistend`; default list `Pokemart_DefaultItemList` in `data/scripts/mart_clerk.inc`. Criteria filtering `src/shop_criteria.c` (`docs/tutorials/how_to_dynamic_shop.md`). Sell price `GetItemSellPrice` in `src/item.c` (half price, or quarter with `I_SELL_VALUE_FRACTION GEN_9`) |
| Money | `gSaveBlock1Ptr->money`, encrypted; use `GetMoney`, `SetMoney`, `AddMoney`, `RemoveMoney`, `IsEnoughMoney` from `src/money.c`. `MAX_MONEY` 999999 in `include/money.h` (can be raised). Scripts `addmoney`, `removemoney`, `checkmoney`, `showmoneybox`. Initial money set in `NewGameInitData` (`src/new_game.c`), 3000. Coins: `src/coins.c` |

## Trainers and battles

| What | Where |
|---|---|
| Trainer id | `include/constants/opponents.h`, `TRAINER_X`. `TRAINERS_COUNT` 855, `MAX_TRAINERS_COUNT` 864 (trainer-defeated flags are allocated by this; raise it to add many trainers, which costs save space) |
| Trainer data | `src/data/trainers.party` in Showdown syntax (name, class, pic, gender, music, items, AI flags, battle type, mugshot, starting status, party with levels, IVs, EVs, items, moves, abilities, natures, balls, tera type). Compiled to `trainers.h` by `trainerproc`. Struct `struct Trainer` / `struct TrainerMon` in `include/data.h` |
| Difficulty variants | Same file, `Difficulty: Hard` sections, active when `B_VAR_DIFFICULTY` is assigned (`src/difficulty.c`) |
| Party pools | `docs/tutorials/how_to_trainer_party_pool.md`, rules in `src/data/battle_pool_rules.h` |
| Classes | `include/constants/trainers.h` (`TRAINER_CLASS_*`), names and money multipliers `gTrainerClasses[]` in `src/battle_main.c`. Prize money = last mon level × class money × 4 (see `Cmd_getmoneyreward` in `src/battle_script_commands.c`) |
| Pics | `TRAINER_PIC_*` in `include/constants/trainers.h`, tables in `src/data.c`, images `graphics/trainers/front_pics/`, `back_pics/`. Tutorials `docs/tutorials/how_to_trainer_front_pic.md`, `how_to_trainer_back_pic.md` |
| AI flags | `include/constants/battle_ai.h`, explained in `docs/tutorials/ai_flags.md`. `AI_FLAG_BASIC_TRAINER` and `AI_FLAG_SMART_TRAINER` are the composites. Logic in `src/battle_ai_main.c`, `battle_ai_switch.c`, `battle_ai_items.c` |
| Mid-battle trainer messages | `docs/tutorials/how_to_new_trainer_slide.md`, `src/trainer_slide.c` |
| Placing a trainer on a map | Object event in `map.json` with `trainer_type: TRAINER_TYPE_NORMAL`, `trainer_sight_or_berry_tree_id` = view range, and a script that uses `trainerbattle_single TRAINER_X, IntroText, DefeatText`. Once defeated the flag `TRAINER_FLAGS_START + id` is set. Snippets in `data/scripts/trainer_battle.inc`; the dynamic script builder is `BattleSetup_ConfigureTrainerBattle` in `src/battle_setup.c` (`docs/tutorials/how_to_dynamic_trainer_script.md`) |
| Rematches | `gRematchTable` in `src/battle_setup.c`, ids in `include/constants/rematches.h` |
| Wild encounters | `src/data/wild_encounters.json` (per map: land, water, rock smash, fishing, with rates and levels), generated to `wild_encounters.h`. Logic `src/wild_encounter.c`. Time of day tables: `docs/tutorials/how_to_time_of_day_encounters.md` |
| Scripted wild battle | `setwildbattle SPECIES_X, level, item` then `dowildbattle` (`BattleSetup_StartScriptedWildBattle`). Legendary variants `seteventmon`, `BattleSetup_StartLegendaryBattle` |
| Battle kinds | `BATTLE_TYPE_*` bit flags in `include/constants/battle.h`, set in `gBattleTypeFlags` before `CB2_InitBattle` |
| Battle Frontier facilities | `src/battle_*` per facility, data `src/data/battle_frontier/` |
| Starting statuses, terrains, environments | `struct StartingStatuses` in trainers; `src/data/battle_environment.h` (backgrounds per terrain) |
| Gimmicks (Mega, Z, Dynamax, Tera) | `src/data/gimmicks.h`, `src/battle_gimmick.c`, `battle_dynamax.c`, `battle_terastal.c`, `battle_z_move.c` |

## Maps and the overworld

| What | Where |
|---|---|
| Map registry | `data/maps/map_groups.json` (groups and order); generated ids in `include/constants/map_groups.h` as `MAP_NAME` |
| Map header and events | `data/maps/<Name>/map.json`: `layout`, `music`, `region_map_section`, `weather`, `map_type`, `allow_*`, `battle_scene`, `connections`, `object_events`, `warp_events`, `coord_events` (type `trigger` with var/value, or `weather`), `bg_events` (`sign`, `hidden_item`, `secret_base`) |
| Map scripts | `data/maps/<Name>/scripts.inc`, header `<Name>_MapScripts::` with `map_script MAP_SCRIPT_ON_TRANSITION, Label` entries. Types in `include/constants/map_scripts.h` with the order they run |
| Layout (tiles) | `data/layouts/layouts.json` entry → `data/layouts/<Name>/map.bin` + `border.bin`. Binary: use Porymap |
| Tilesets | `data/tilesets/primary/<name>/` and `secondary/<name>/`: `tiles.png`, `palettes/NN.pal`, `metatiles.bin`, `metatile_attributes.bin`, `anim/`. Headers `src/data/tilesets/headers.h`, includes `graphics.h`, `metatiles.h`. Animations `src/tileset_anims.c` |
| Metatile behaviours | `include/constants/metatile_behaviors.h` (`MB_TALL_GRASS`, `MB_PC`, `MB_WARP_DOOR`, ...), queries `src/metatile_behavior.c`. A behaviour is a number on each metatile; the engine asks "is this tile grass / water / a ledge" through these functions |
| NPC sprites | `include/constants/event_objects.h` (`OBJ_EVENT_GFX_*`), `src/data/object_events/object_event_graphics.h` (PNG includes), `object_event_pic_tables.h`, `object_event_graphics_info.h` (size, palette, anims), `object_event_graphics_info_pointers.h`, `graphics/object_events/pics/people/*.png`. Any species can be an NPC via `OBJ_EVENT_GFX_SPECIES(NAME)` |
| Movement types | `include/constants/event_object_movement.h` (`MOVEMENT_TYPE_WANDER_AROUND`, `FACE_DOWN`, ...), implemented in `src/event_object_movement.c` |
| Scripted movement | `applymovement LOCALID, Movement_Label` + `waitmovement 0`; movement scripts are lists of `walk_up`, `face_player`, `emote_exclamation_mark`, ... ending in `step_end`, macros in `asm/macros/movement.inc`, shared ones in `data/scripts/movement.inc` |
| Warps and respawn | `warp MAP_X, x, y`; heal locations `src/data/heal_locations.json`; `setrespawn` |
| Weather | `include/constants/weather.h`, `src/field_weather.c`, `field_weather_effect.c` (dynamic weather: `docs/tutorials/how_to_dynamic_weather.md`) |
| Region map | `src/data/region_map/`, `src/region_map.c`, sections in `region_map_sections.json` |
| Map name popups | `src/map_name_popup.c`, `graphics/map_popup/` |
| Day/night | `OW_ENABLE_DNS`, `src/field_weather.c` palette tinting, `docs/tutorials/dns.md` |
| Field moves | `src/field_move.c`, `src/fldeff_*.c`, `data/scripts/field_move_scripts.inc` |
| Deleting vanilla maps | `docs/tutorials/how_to_delete_vanilla_maps.md` |

## Scripts, text and specials

| What | Where |
|---|---|
| Command macros (the language reference) | `asm/macros/event.inc`. 401 macros. Each has a comment describing arguments |
| Command implementations | `src/scrcmd.c`, `ScrCmd_name`. Opcode table `data/script_cmd_table.inc` |
| Specials (C functions callable from scripts) | `data/specials.inc` (`def_special Name`), functions live anywhere in `src/` (most in `src/field_specials.c`). Call with `special Name` or `specialvar VAR_RESULT, Name` |
| Script-callable C with arguments | `callnative` with a `void Func(struct ScriptContext *ctx)` that reads its operands; see `ScrCmd_createmon` in `src/script_pokemon_util.c` and the `givemon` macro |
| Standard message scripts | `data/scripts/std_msgbox.inc`; used through `msgbox Text, MSGBOX_NPC` / `MSGBOX_SIGN` / `MSGBOX_DEFAULT` / `MSGBOX_YESNO` / `MSGBOX_AUTOCLOSE` |
| Obtain item flow | `data/scripts/obtain_item.inc` |
| Multichoice menus | `dynmultichoice` (`docs/tutorials/how_to_dynmultichoice.md`) or vanilla `multichoice` with tables in `src/data/script_menu.h` |
| Text in scripts | `.string "text$"` labels next to the script; placeholders and control codes from `charmap.txt`; `{PLAYER}`, `{STR_VAR_1..3}`, `{COLOR RED}`, `\n`, `\l`, `\p` |
| Text in C | `_("...")` constants, `COMPOUND_STRING(...)`, `gStringVar1`..`gStringVar4` buffers, `StringExpandPlaceholders`, `ConvertIntToDecimalStringN`, `StringCopy` (`include/string_util.h`). Shared strings `src/strings.c` / `include/strings.h` |
| Name box (speaker names) | `docs/tutorials/how_to_namebox.md`, `src/field_name_box.c` |
| Fonts | `graphics/fonts/`, `src/fonts.c`, `src/text.c` |

## UI building blocks

| What | Where |
|---|---|
| Windows (text boxes) | `struct WindowTemplate` (bg, tile position, size, palette, base block), `AddWindow`/`InitWindows`, `PutWindowTilemap`, `CopyWindowToVram`, `FillWindowPixelBuffer`, `ClearWindowTilemap`, `RemoveWindow` in `src/window.c`. Frames `DrawStdWindowFrame`, `DrawDialogueFrame` in `src/menu.c` |
| Printing text | `AddTextPrinterParameterized(windowId, font, str, x, y, speed, callback)` and variants `3`/`4`/`5` with colours in `src/menu.c`. `TEXT_SKIP_DRAW` for instant |
| Field message | `ShowFieldMessage`, `ShowFieldAutoScrollMessage` in `src/field_message_box.c` |
| Yes/No | `CreateYesNoMenu`, `DisplayYesNoMenuDefaultYes`, `Menu_ProcessInputNoWrapClearOnChoose` in `src/menu.c`; scripts `yesnobox` / `MSGBOX_YESNO` result in `VAR_RESULT` |
| Vertical option menus | `PrintMenuTable` + `InitMenuInUpperLeftCornerNormal` + `Menu_ProcessInput`, or `struct ListMenuTemplate` + `ListMenuInit` + `ListMenu_ProcessInput` in `src/list_menu.c` for scrolling lists. `src/start_menu.c` and `src/debug.c` are readable examples |
| Backgrounds | `struct BgTemplate`, `InitBgsFromTemplates`, `SetBgTilemapBuffer`, `ShowBg`, `LoadPalette`, `DecompressAndCopyTileDataToVram` in `src/bg.c`, `decompress.c`, `palette.c` |
| Fades | `BeginNormalPaletteFade(PALETTES_ALL, delay, from, to, colour)`, check `gPaletteFade.active` |
| Sprites | `struct SpriteTemplate`, `CreateSprite`, `DestroySprite`, `LoadCompressedSpriteSheet`, `LoadSpritePalette`, `StartSpriteAnim` in `src/sprite.c` |
| Money box | `DrawMoneyBox`, `HideMoneyBox` in `src/money.c` |
| Start menu entries | `src/start_menu.c`: `enum MENU_ACTION_*`, `sStartMenuItems[]`, `BuildNormalStartMenu` |
| Full-screen minimal example | `src/diploma.c` |
| Item icons in menus | `AddItemIconSprite` in `src/item_icon.c` |

## Sound

| What | Where |
|---|---|
| Ids | `include/constants/songs.h` (`MUS_*`, `SE_*`), matching the order of `sound/song_table.inc` |
| Music | `sound/songs/midi/<name>.mid`, options in `sound/songs/midi/midi.cfg`, voice group in `sound/voicegroups/` |
| Sound effects | Same table, programmable wave and direct sound samples in `sound/` |
| Cries | `sound/direct_sound_samples/cries/`, `sound/cry_tables.inc`, `include/constants/cries.h` |
| API | `PlaySE(SE_X)`, `PlayBGM`, `PlayFanfare`, `PlayCry_Normal` in `include/sound.h`. Scripts `playse`, `playbgm`, `playfanfare`, `waitfanfare`, `playmoncry` |
| Map music | `music` field in `map.json` |

## Time

| What | Where |
|---|---|
| Real time clock | `src/rtc.c`, `gLocalTime`, `RtcCalcLocalTime`, `GetTimeOfDay` (morning/day/evening/night by `OW_TIMES_OF_DAY`) |
| Fake clock (advances with play time) | `OW_USE_FAKE_RTC`, `src/fake_rtc.c`, stored in `SaveBlock3`; script commands `addtime`, `fwdtime`, `pausefakertc` |
| Daily and per-minute hooks | `DoTimeBasedEvents` → `UpdatePerDay` / `UpdatePerMinute` in `src/clock.c`; `gSaveBlock1Ptr->dailySeed` for deterministic daily randomness; daily flags (`DAILY_FLAGS_START` in `flags.h`) clear each day |
| Debug | Debug menu → Utilities → Time submenu changes time of day and weekday |

## Save data

| What | Where |
|---|---|
| Structs | `include/global.h`: `struct SaveBlock1`, `SaveBlock2`, `SaveBlock3`; `struct PokemonStorage` in `include/pokemon_storage_system.h` |
| Pointers | `gSaveBlock1Ptr`, `gSaveBlock2Ptr`, `gSaveBlock3Ptr`, `gPokemonStoragePtr` (`src/load_save.c`) |
| Limits and layout | `include/save.h` (`SECTOR_DATA_SIZE 3968`, 14 sectors per slot), `src/save.c` (`STATIC_ASSERT`s that fail the build if a struct is too big, sector layout comment at the top) |
| Free space toggles | `include/config/save.h` |
| New game defaults | `NewGameInitData` in `src/new_game.c`, then `EventScript_ResetAllMapFlags` in `data/scripts/new_game.inc` |
| Flags | `include/constants/flags.h`, `FlagSet/Clear/Get` in `src/event_data.c`. Ranges: temp `0x00`..`0x1F` (cleared on every map load), general `0x20`..`0x4FF` (375 unused), trainer `0x500`..`0x85F`, system `0x860`.., daily, special `0x4000`.. (RAM only, not saved) |
| Vars | `include/constants/vars.h`, `VarSet/VarGet`. `VAR_TEMP_0`..`F` cleared on map load, `VAR_UNUSED_*` (29) for you, `VAR_0x8000`..`0x800B` scratch for scripts, `VAR_RESULT` (0x800D) the conventional return value, `VAR_LAST_TALKED`, `VAR_FACING`. Special vars are RAM only |
| Game stats | `IncrementGameStat(GAME_STAT_X)`, `include/constants/game_stat.h`, 64 counters in the save |
