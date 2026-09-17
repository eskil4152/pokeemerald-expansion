# 02. Directory map

What each directory holds and why it matters. Sizes are rough. "Generated" means git-ignored output of the build that you never edit.

## Top level

| Path | What | Why it matters |
|---|---|---|
| `Makefile`, `config.mk`, `*_rules.mk`, `make_tools.mk` | Build system | `Makefile` has the C rules and targets. `*_rules.mk` cover graphics, maps, JSON data, audio, trainers. You rarely edit them; new C files and graphics are picked up automatically |
| `ld_script_modern.ld` | Linker script | Memory map. Defines EWRAM/IWRAM/ROM regions and section order. Do not touch unless adding a section |
| `charmap.txt` | Text encoding table | Every character and control code (`{PLAYER}`, `{COLOR}`, `\n`) is defined here. Add new glyph codes here |
| `rom.sha1` | Checksum of retail Emerald | Irrelevant for a hack. `make compare` only makes sense in vanilla |
| `sym_*.txt`, `common_syms/` | Not present in this fork | Vanilla uses them for matching builds |
| `CLAUDE.md` | Session context for Claude | Short. Points here |
| `claude-guide/` | This guide | |
| `docs/` | Upstream documentation | `docs/tutorials/*.md` are the official how-tos. `docs/changelogs/` explain what changed per version, useful when merging upstream |
| `src/` | All C source | See below |
| `include/` | All C headers, constants, configs | See below |
| `data/` | Scripts, maps, layouts, tilesets, battle scripts, sound data | See below |
| `graphics/` | PNG and palette sources | Organised by feature. See below |
| `sound/` | Music, sound effects, cries | See below |
| `asm/` | Macros for the script languages, and a few assembly files | `asm/macros/event.inc`, `battle_script.inc`, `movement.inc`, `battle_anim_script.inc` define every script command's byte layout |
| `test/` | Automated tests | `test/battle/` is the bulk. Run with `make check` |
| `tools/` | Build tools, compiled on first `make` | `gbagfx`, `preproc`, `mapjson`, `trainerproc`, `mgba` (headless test runner), Python helpers |
| `libagbsyscall/` | Thin wrappers for GBA BIOS calls | Never edit |
| `migration_scripts/`, `dev_scripts/` | Upstream maintenance scripts | For example `dev_scripts/delete_frlg_maps.py` |
| `build/` | Generated | Object files, per build type (`emerald`, `emerald-test`, `emerald-debug`, `emerald-release`) |
| `pokeemerald.gba`, `.elf`, `.map` | Generated | The ROM, the ELF with symbols for GDB, the linker map |

## `src/` (about 480 files)

The biggest files and groups, by what they own. Everything in `src/` is compiled.

| Path | Owns |
|---|---|
| `main.c` | Boot, main loop, interrupts, joypad reading |
| `task.c`, `sprite.c`, `bg.c`, `window.c`, `text.c`, `menu.c`, `list_menu.c`, `palette.c`, `gpu_regs.c`, `dma3_manager.c`, `malloc.c` | Engine primitives: tasks, sprites, backgrounds, windows, text printing, menus, palettes and fades, DMA, heap |
| `overworld.c` | Map loading, overworld callbacks, warps, link overworld |
| `field_control_avatar.c` | Per-frame field input dispatch (`ProcessPlayerFieldInput`) |
| `field_player_avatar.c`, `bike.c`, `field_effect*.c`, `field_weather*.c`, `field_camera.c`, `fieldmap.c`, `metatile_behavior.c`, `tileset_anims.c` | Player movement, surfing, biking, field effects (Cut, Flash, sparkle...), weather, camera, map grid access, tile behaviour queries, animated tiles |
| `event_object_movement.c` (very large) | NPC objects: spawning, movement types, movement actions, sprites, followers |
| `event_data.c` | Flags and vars |
| `script.c`, `scrcmd.c`, `script_movement.c`, `script_menu.c`, `script_pokemon_util.c` | Overworld script engine, all script commands, movement application, multichoice menus, `givemon`/`createmon` |
| `field_specials.c`, `field_message_box.c`, `field_tasks.c`, `field_screen_effect.c` | Grab bag of `special` functions, field text boxes, per-map step tasks, screen fades and warp effects |
| `start_menu.c`, `option_menu.c`, `main_menu.c`, `title_screen.c`, `naming_screen.c`, `new_game.c`, `quickstart.c` | Menus around the overworld and game start. `new_game.c` has `NewGameInitData` which sets initial money etc |
| `save.c`, `load_save.c`, `save_failed_screen.c` | Save to flash, save block pointers |
| `pokemon.c` (very large) | `struct Pokemon` access (`GetMonData`/`SetMonData`), creation, stats, evolution, learnsets, form changes |
| `pokemon_storage_system.c`, `party_menu.c`, `pokemon_summary_screen.c`, `pokedex.c`, `pokedex_plus_hgss.c`, `pokemon_icon.c`, `pokemon_animation.c`, `trainer_pokemon_sprites.c` | PC boxes, party menu, summary, Pokédex (vanilla and HGSS style), icons and sprite animation |
| `item.c`, `item_menu.c`, `item_use.c`, `item_icon.c`, `shop.c`, `shop_criteria.c`, `money.c`, `coins.c` | Bag storage, bag UI, item effects when used, item icons, marts, money |
| `battle_main.c`, `battle_setup.c`, `battle_util.c` (very large), `battle_script_commands.c` (very large), `battle_controllers.c` + `battle_controller_*.c`, `battle_interface.c`, `battle_message.c`, `battle_end_turn.c`, `battle_switch_in.c`, `battle_stat_change.c`, `battle_set_effect.c`, `battle_hold_effects.c`, `battle_move_resolution.c`, `battle_gimmick.c`, `battle_dynamax.c`, `battle_terastal.c`, `battle_z_move.c`, `battle_debug.c`, `battle_transition.c`, `battle_intro.c`, `battle_bg.c`, `battle_gfx_sfx_util.c` | The battle engine. `battle_setup.c` starts battles from the field. `battle_util.c` has damage calc, ability and item dispatch. `battle_script_commands.c` implements every battle script command. Controllers translate "battler wants to do X" into UI or AI |
| `battle_ai_*.c` | Trainer AI: move scoring, switching, item use |
| `battle_anim*.c` | Move animations, one file per type |
| `battle_frontier.c`, `battle_tower.c`, `battle_dome.c`, `battle_factory*.c`, `battle_pike.c`, `battle_pyramid*.c`, `battle_arena.c`, `battle_palace.c`, `battle_tent.c`, `frontier_*.c`, `apprentice.c`, `trainer_hill.c`, `trainer_tower.c` | Battle Frontier and similar facilities. Big and self-contained. Candidates to delete or ignore in a sandbox |
| `trainer.c`, `trainer_see.c`, `trainer_slide.c`, `trainer_pools.c`, `trainer_card.c`, `gym_leader_rematch.c`, `match_call.c`, `vs_seeker.c`, `difficulty.c` | Trainer data access, trainers spotting the player, mid-battle trainer messages, party pools, trainer card, rematches, difficulty levels |
| `wild_encounter.c`, `wild_encounter_ow.c`, `roamer.c`, `mass_outbreak.c`, `dexnav.c`, `fishing.c`, `pokemon_spots.c`, `random_mon_generation.c` | Wild Pokémon: encounter tables, overworld encounters, roamers, outbreaks, DexNav, fishing, generic spots, random generation |
| `daycare.c`, `egg_hatch.c`, `evolution_scene.c`, `move_relearner.c`, `pokeblock*.c`, `berry*.c`, `apricorn_tree.c`, `contest*.c`, `decoration*.c`, `secret_base.c`, `tv.c`, `mauville_old_man.c`, `lilycove_lady.c`, `dewford_trend.c`, `easy_chat.c`, `mail*.c`, `pokenav*.c`, `region_map.c`, `map_name_popup.c`, `map_preview_screen.c` | Vanilla side systems. Each is a self-contained example of a feature with its own save data and UI |
| `rtc.c`, `fake_rtc.c`, `datetime.c`, `clock.c`, `time_events.c`, `wallclock.c` | Real time clock, fake clock, time of day, daily events |
| `debug.c` (4800 lines), `pokemon_sprite_visualizer.c` | Overworld debug menu. Good reference for list menus and input handling |
| `link*.c`, `librfu*.c`, `union_room*.c`, `cable_club.c`, `record_mixing.c`, `mystery_*.c`, `ereader*.c`, `multiboot*.c` | Link cable, wireless, mystery gift. Ignore for a single-player sandbox |
| `m4a*.c`, `sound.c` | Sound engine and the `PlaySE`/`PlayBGM` API |
| `credits*.c`, `intro*.c`, `expansion_intro.c`, `hall_of_fame*.c` | Intro and ending sequences |
| `data.c`, `graphics.c`, `strings.c`, `text_input_strings.c`, `move.c`, `trainer.c` | Files that exist mainly to include big tables from `src/data/` and declare shared strings |
| `*_frlg.c`, `oak_speech.c`, `seagallop.c`, `fame_checker.c`, `ss_anne.c` | FireRed/LeafGreen support. The expansion can build FRLG (`make firered`). Compiled out in the Emerald build via `IS_FRLG` |

## `src/data/`

Data tables in C. Included by exactly one `.c` each.

| Path | Holds |
|---|---|
| `pokemon/species_info/gen_N_families.h` | `gSpeciesInfo` entries: stats, types, abilities, names, dex text, sprites, learnset pointers, evolutions, overworld sprite. One file per generation, one `#if P_FAMILY_X` block per family |
| `pokemon/level_up_learnsets/gen_N.h`, `pokemon/egg_moves.h`, `pokemon/teachable_learnsets.h` (generated), `pokemon/all_learnables.json` | Learnsets |
| `pokemon/form_change_tables.h`, `form_species_tables.h`, `form_change_table_pointers.h` | Form changes (Mega, weather forms, item holds) |
| `pokemon/experience_tables.h`, `egg_data.h`, `item_effects.h`, `pokedex_orders.h` | Exp curves, egg data, medicine effects, dex ordering |
| `moves_info.h` | `gMovesInfo`: every move's data and contest data |
| `battle_move_effects.h` | Maps `EFFECT_*` to the battle script that runs it |
| `abilities.h` | Ability names, descriptions, AI rating, flags. Logic is in `battle_util.c` |
| `items.h` | `gItemsInfo`: every item. Price, pocket, description, use functions, hold effect, icon, shop criteria |
| `graphics/*.h` | `INCGFX` declarations for item icons, Pokémon sprites, Poké Balls, etc |
| `trainers.party` → `trainers.h` | All trainers. Edit `.party`, never `.h` |
| `battle_partners.party`, `debug_trainers.party` | Multi-battle partners and the debug menu's trainers |
| `battle_pool_rules.h` | Rules for trainer party pools |
| `types_info.h` | Type names, colours, type chart |
| `hold_effects.h` | Hold effect descriptions |
| `gimmicks.h` | Mega, Z, Dynamax, Tera hooks |
| `wild_encounters.json` → `wild_encounters.h` | Encounter tables per map |
| `object_events/*.h` | NPC sprite graphics, animations, subsprite tables, movement function tables, follower Pokémon sprites |
| `tilesets/*.h` | Tileset headers, graphics includes, metatile tables |
| `region_map/*`, `heal_locations.json` | Town map and Pokémon Center respawn points |
| `text/*.h`, `script_menu.h`, `speaker_names.h` | Some text tables, multichoice menu definitions, name box speaker names |
| `battle_frontier/*.h`, `bard_music/`, `easy_chat/`, `decoration/`, `contest_*`, `lilycove_lady.h`, `trade.h`, `union_room.h`, `wallpapers.h`, `credits.h` | Vanilla side content |
| `map_group_count.h`, `heal_locations.h`, `wild_encounters.h`, `trainers.h`, `tutor_moves.h` | Generated |

## `include/`

| Path | Holds |
|---|---|
| `global.h` | Included by everything. Save structs, common macros (`ARRAY_COUNT`, `JOY_NEW`, `min`/`max`), base types |
| `gba/` | Hardware definitions: registers, memory addresses, `EWRAM_DATA`, DMA, BIOS calls, `isagbprint.h` for printf |
| `config/*.h` | All feature configuration. See `03-config.md` |
| `constants/*.h` | Enums and defines: `species.h`, `moves.h`, `items.h`, `abilities.h`, `flags.h`, `vars.h`, `songs.h`, `battle.h` (battle types, statuses), `event_objects.h` (NPC graphics ids), `metatile_behaviors.h`, `map_scripts.h`, `opponents.h` (trainer ids), `trainers.h` (classes, pics), `battle_ai.h`, `hold_effects.h`, `pokemon.h` (`MON_DATA_*`), `global.h` (`PARTY_SIZE`, bag sizes), plus generated `map_groups.h`, `layouts.h`, `map_event_ids.h`, `script_commands.h` |
| `*.h` matching `src/*.c` | The API of each source file. `battle.h` has `struct BattleStruct`, `struct BattlePokemon` and most battle globals. `pokemon.h` has `struct Pokemon`, `struct SpeciesInfo`. `item.h` has `struct ItemInfo`. `data.h` has `struct Trainer`. `event_scripts.h` declares script labels for C. `battle_scripts.h` declares battle script labels |
| `test/` | Test framework macros |

## `data/`

| Path | Holds |
|---|---|
| `event_scripts.s` | Root of all overworld script data. Includes every `data/scripts/*.inc`, every map's `scripts.inc`, `data/text/*.inc`, the command table and specials table. Add `.include` lines here |
| `scripts/*.inc` | Shared scripts by topic: `std_msgbox.inc` (message standards), `obtain_item.inc`, `trainer_battle.inc`, `pkmn_center_nurse.inc`, `mart_clerk.inc`, `pc.inc`, `day_care.inc`, `movement.inc` (common movement sequences), `debug.inc` (debug menu scripts, has empty `Script_1`..`Script_8` slots), `config.inc`, `new_game.inc` |
| `maps/<MapName>/map.json` | The map: layout ref, music, weather, type, connections, object events (NPCs), warps, coord triggers, signs and hidden items. 944 maps |
| `maps/<MapName>/scripts.inc` | That map's scripts and text. Map scripts table (`ON_TRANSITION` etc) at the top |
| `maps/map_groups.json` | Registry of all maps by group. Adding a map means adding it here |
| `layouts/layouts.json`, `layouts/<Name>/map.bin`, `border.bin` | Tile grids. Binary; edit with Porymap |
| `tilesets/primary/<name>/`, `tilesets/secondary/<name>/` | `tiles.png`, `palettes/*.pal`, `metatiles.bin`, `metatile_attributes.bin`, `anim/` |
| `battle_scripts_1.s`, `battle_scripts_2.s` | Every battle script: move effects, ability messages, status effects, end turn, catching |
| `battle_anim_scripts.s` | Move animations and `gBattleAnims_Moves` table |
| `field_effect_scripts.s` | Field effect animations |
| `script_cmd_table.inc` | Opcode → C function table for event scripts |
| `specials.inc` | `special` name → C function table. 622 entries. Add yours at the end |
| `text/*.inc` | Text used by shared scripts |
| `sound_data.s` | Includes sound tables |
| `contest_ai_scripts.s`, `mystery_*`, `multiboot_*`, `*.gba` | Vanilla oddities. Ignore |

## `graphics/`

One directory per feature. Every image is an indexed PNG. Palettes are `.pal` (JASC format) or come from the PNG itself.

Notable: `pokemon/<species>/` (front, back, icon, footprint, palettes, anim frames), `object_events/pics/people/` (NPC sprites), `items/icons/`, `trainers/front_pics/` and `back_pics/`, `battle_anims/`, `interface/`, `text_window/`, `fonts/`, `title_screen/`, `map_popup/`, `pokedex/`, `summary_screen/`, `bag/`, `shop/`.

## `sound/`

`songs/midi/*.mid` + `midi.cfg` (per-song mid2agb options), `song_table.inc` (the list; order matches `include/constants/songs.h`), `voicegroups/` and `voice_groups.inc` (instrument definitions), `direct_sound_samples/` (samples, `cries/` for Pokémon cries), `cry_tables.inc`.

## `test/`

`test/battle/` has hundreds of files grouped by `ability/`, `move_effect/`, `hold_effect/`, `item_effect/`, `ai/`, `gimmick/`, `form_change/`. Non-battle tests are top level (`pokemon.c`, `script.c`, `save.c`, `bag.c`, `daycare.c`, `random.c`...). `test_runner*.c` is the harness. See `08-testing-and-debugging.md`.

## `tools/`

Compiled on first build. The ones you may run by hand: `tools/mgba/mgba-rom-test-mac` (headless test emulator), `tools/learnset_helpers/*.py`, `tools/wild_encounters/wild_encounters_to_header.py`, `tools/misc/make_scr_cmd_constants.py`. `tools/find_func/` and `tools/patchelf/` are build internals.
