# 09. Gotchas and conventions

Things that bite. Read before changing constants, structs, or the build.

## Saves

- **Anything in `SaveBlock1/2/3` or `PokemonStorage` is layout-sensitive.** Adding, removing, reordering or resizing a field, changing `MAX_TRAINERS_COUNT`, `FLAGS_COUNT`, `VARS_COUNT`, `NUM_SPECIES` (dex flag arrays), `POKEMON_NAME_LENGTH`, or flipping a `FREE_*` or `OW_USE_FAKE_RTC` config shifts every later byte. Old saves are then garbage. There is no migration; delete the `.sav`.
- **Species, move, item, ability ids are stored in saves.** Append new ones; never insert or renumber. `SPECIES_CUSTOM_START..END` exists for this.
- **Encrypted fields**: money, coins, bag quantities, berry powder, game stats. Always use the accessors. Reading `gSaveBlock1Ptr->money` directly gives a random-looking number.
- **The save is flash with a write cycle limit** on real hardware. Do not add code that saves every step. Emulator does not care.
- **Sector budget**: SaveBlock2 has 3968 bytes and is nearly full; SaveBlock1 has 15872; SaveBlock3 1624. The `STATIC_ASSERT` in `src/save.c` tells you when you exceed one.

## Constants and enums

- **Flags above `SPECIAL_FLAGS_START` (0x4000) are not saved.** Do not use them for progress.
- **`VAR_TEMP_*` and `FLAG_TEMP_*` clear on every map load**, including menus that reload the map? No: only on map transitions and warps. A menu return keeps them. A battle keeps them. A warp does not.
- **`VAR_0x8000`-`0x800B` and `VAR_RESULT` are shared scratch.** Any `special` or `giveitem` may overwrite them. Read results immediately.
- **Trainer flags are `TRAINER_FLAGS_START + id`**, so raising `MAX_TRAINERS_COUNT` moves `SYSTEM_FLAGS` and everything after: a save-breaking change even though no struct changed.
- **`MOVES_COUNT`, `ITEMS_COUNT`, `ABILITIES_COUNT`, `NUM_SPECIES` size arrays all over the code.** After adding entries a full rebuild is automatic, but generated learnsets (`teachable_learnsets.h`) update only if their inputs changed; `make clean-teachables` forces it.
- **Adding a species changes the dex flag arrays** (`NUM_DEX_FLAG_BYTES`) only when it crosses a byte boundary, but treat it as save-breaking anyway.

## Scripts

- **`::` vs `:`**: double colon exports the label (needed for anything referenced from another file or from C). Single colon is file-local. Text labels referenced only within the same file can be single.
- **Every string ends with `$`.** Missing it reads into the next label.
- **`lock`/`release` pairs.** A script that `lock`s and ends without `release` leaves the player frozen. `MSGBOX_NPC` does both for you.
- **`waitstate` needs a matching `ScriptContext_Enable()`** from C, or the game hangs. Specials with `waitstate=1` in `specials.inc` insert it automatically.
- **Movement scripts must end in `step_end`.** `waitmovement 0` waits for the last `applymovement` target; pass a local id to wait for a specific one.
- **`applymovement` on the player** uses `LOCALID_PLAYER` (255), and the follower Pokémon returns to its ball unless `FLAG_SAFE_FOLLOWER_MOVEMENT` is set or the movement is in the "safe" region of `data/scripts/movement.inc`.
- **Numbers in scripts are 16-bit** unless the macro says `.4byte` (money, callnative args). `setvar VAR_X, 70000` truncates silently.
- **`goto_if_eq VAR, value, Label` is the combined form.** Older tutorials write `compare` then `goto_if_eq Label`; both work.
- **A map's `scripts.inc` must be included in `data/event_scripts.s`**, and the file must be listed in `map_groups.json`, and the layout in `layouts.json`. Missing any of the three gives confusing link errors.
- **Script effect analysis**: new commands and specials should call `Script_RequestEffects(...)`. Skipping it is safe but marks the function opaque.

## Battle

- **`gBattleMons[]` is a copy.** Changes to it do not reach the party until the battle ends and `HandleEndTurn_FinishBattle` writes back. Changing the party mid-battle from a menu requires updating both.
- **Battler indices are positions, not party slots.** `gBattlerPartyIndexes[battler]` maps to the party.
- **Battle scripts cannot contain C; C cannot block.** A C hook that needs a message must `BattleScriptCall` a script that prints it, then return; the script runs on later frames.
- **AI reads the same data as the player**, so a rule that changes damage must be reflected in `src/battle_ai_util.c` damage estimates or the AI will misjudge it (fine for a sandbox, just know why the AI looks dumb).
- **Tests rig RNG**: moves always hit, secondary effects always trigger. A test that passes may still see different behaviour in play.
- **Two free `BATTLE_TYPE_*` bits only** (`14` and `30`). Prefer flags for rules.

## Overworld

- **16 object events on screen at once**, including the player and followers. Extra NPCs in the JSON silently fail to spawn.
- **Metatile ids are per tileset pair.** `setmetatile` values depend on the map's primary and secondary tilesets. Labels in `metatile_labels.h` help.
- **Elevation matters.** An NPC at elevation 3 on a bridge at elevation 4 is under it. `0` means "any".
- **Object event coordinates in the JSON are map coordinates**; `SpawnSpecialObjectEventParameterized` subtracts `MAP_OFFSET` (7) itself, `setobjectxy` takes map coordinates.
- **Day/night tinting** (`OW_ENABLE_DNS`) alters map palettes at runtime; custom map graphics may look wrong at night if they use palette slots the tint targets. `docs/tutorials/dns.md` explains lights and exceptions.
- **Overworld sprite compression** (`OW_GFX_COMPRESS TRUE`) forbids 48x48 sprites and loads sprites into VRAM; if you add many custom NPC sprites and see corruption, set it `FALSE`.

## Graphics

- **Indexed PNG, 16 colours, colour 0 is transparent.** `gbagfx` errors are cryptic; "too many colours" and "not indexed" are the usual causes. Export from Aseprite or GIMP in indexed mode.
- **Sizes**: Pokémon sprites 64x64 per frame; icons 32x32 per frame stacked vertically; NPC sprites 16x32 or 32x32 frames in a strip; item icons 24x24; trainer front pics 64x64 (or 80x80 in expansion).
- **Palettes are the bottleneck**, not tiles. 16 background palettes, 16 sprite palettes, most in use in the overworld and battle.
- **INCBIN vs INCGFX**: `INCBIN` embeds the file as is; `INCGFX(path, ".ext")` asks the Makefile to convert the PNG to that extension first. Use the extension chain other entries in the same table use.

## Build

- **`make` after changing `include/config/*.h` or `include/global.h` recompiles almost everything.** Group config changes.
- **Generated files are git-ignored**: `trainers.h`, `wild_encounters.h`, `teachable_learnsets.h`, `map_groups.h`, `layouts.h`, `map_event_ids.h`, `script_commands.h`, all `.4bpp`/`.gbapal`/`.smol`. Edit the source (`.party`, `.json`, `.png`), never the output.
- **`make clean` deletes generated graphics**; the next build takes several minutes. `make tidy` is usually enough.
- **Warnings are errors** (`-Werror`) except unused-variable ones in non-RHH repos. Enum conversion warnings (`-Wenum-conversion`) are on; use the right enum types (`enum Species`, `enum Item`, `enum Move`) in new code.
- **`make release`** changes behaviour: `NDEBUG`, no debug menus, no quickstart, no printf, LTO. Test both before sharing a ROM.
- **Python 3 is required** at build time for the learnset and wild encounter scripts.
- **FRLG support** is compiled out in the Emerald build via `IS_FRLG`, but the files exist. Ignore `*_frlg*` files and `Frlg` map groups; do not delete them casually, upstream merges will bring them back.

## Style and upstream compatibility

The expansion's style guide (`docs/STYLEGUIDE.md`): 4 spaces, `PascalCase` functions and structs, `camelCase` variables, `g`/`s` prefixes for globals and statics, `CAPS` constants, braces on their own lines, `u32`/`s32` for locals, smallest types in saves, enums over magic numbers, config checks inline (`if (CONFIG)`) not `#ifdef` around whole functions. New features in their own files with minimal edits to existing ones. Following this keeps upstream merges small.

Merging upstream (`git fetch upstream && git merge upstream/master`): the changelog for each version lists renames and save-breaking changes. Conflicts concentrate in `species_enabled.h`, `items.h`, `moves_info.h`, `trainers.party`, and any battle file you touched. Keep your mechanics in their own files and hook with one-line calls so the conflicts are one line each.
