# 06. Recipes: adding more of what Pokémon already has

Each recipe lists the files to touch in order, with the exact names as of this fork. Where an upstream tutorial exists it is referenced; those go deeper on graphics.

Before any recipe: work on a branch, build, test in mGBA, and use the debug menu to reach the content instead of playing to it.

## Add a Pokémon species

Full tutorial: `docs/tutorials/how_to_new_pokemon.md`. The short path:

1. **Constant**: `include/constants/species.h`. Add `SPECIES_MYMON,` between `SPECIES_CUSTOM_START` and `SPECIES_CUSTOM_END`. Never insert elsewhere: ids are in saves.
2. **National dex**: `include/constants/pokedex.h`, add `NATIONAL_DEX_MYMON` before `NATIONAL_DEX_COUNT` (and the Hoenn dex list if you want it in the regional dex; `src/data/pokemon/pokedex_orders.h` for ordering).
3. **Data**: add a `[SPECIES_MYMON] = { ... }` entry. The intended place is `src/data/pokemon/species_info.h`, after the `#include "species_info/gen_9_families.h"` line (there is a commented-out template near the end of the file). Copy a similar species from `gen_N_families.h` and change fields. Required: base stats, `.types`, `.catchRate`, `.expYield`, `.genderRatio`, `.eggCycles`, `.friendship`, `.growthRate`, `.eggGroups`, `.abilities`, `.bodyColor`, `.speciesName`, `.cryId`, `.natDexNum`, `.categoryName`, `.height`, `.weight`, `.description`, scale/offset fields, pic and palette pointers with sizes, `.iconSprite`, `.iconPalIndex`, `.levelUpLearnset`, `.teachableLearnset`. Optional: `.eggMoveLearnset`, `.evolutions`, `.formSpeciesIdTable`, `.formChangeTable`, `SHADOW`, `FOOTPRINT`, `OVERWORLD`.
4. **Learnsets**: a `static const struct LevelUpMove sMymonLevelUpLearnset[] = { LEVEL_UP_MOVE(1, MOVE_TACKLE), ... LEVEL_UP_END };` (put it in `src/data/pokemon/level_up_learnsets/gen_9.h` or a new file included next to it), and either `sNoneTeachableLearnset` or an entry in `all_learnables.json` so the generator produces `sMymonTeachableLearnset` (see `docs/tutorials/teachable_learnsets.md`).
5. **Graphics**: `graphics/pokemon/mymon/front.png` (64x64, two frames stacked for animation: 64x128), `back.png`, `icon.png` (32x64, two frames), `normal.pal`, `shiny.pal`, `footprint.png`. Declare in `src/data/graphics/pokemon.h` following any neighbour (`gMonFrontPic_Mymon`, `gMonBackPic_`, `gMonPalette_`, `gMonShinyPalette_`, `gMonIcon_`, `gMonFootprint_`). Use the sprite visualiser (Select in the summary screen) to tune `.frontPicYOffset` and elevation.
6. **Cry**: `sound/direct_sound_samples/cries/mymon.aif`, entry in `sound/cry_tables.inc`, `CRY_MYMON` in `include/constants/cries.h`. Or reuse an existing `.cryId`.
7. **Overworld sprite** (only if followers or NPC use): `graphics/pokemon/mymon/overworld.png` + `overworld_normal.pal`, `overworld_shiny.pal`, tables in `src/data/object_events/object_event_pic_tables_followers.h`, and the `OVERWORLD(...)` block in the species entry. Skip it and `OW_SUBSTITUTE_PLACEHOLDER` shows a stand-in.
8. **Make it obtainable**: `givemon SPECIES_MYMON, 20` in a script, or add to `src/data/wild_encounters.json`, or to a trainer. Debug menu → Give → Pokémon can spawn it immediately for testing.

Evolution methods are few (`EVO_LEVEL`, `EVO_ITEM`, `EVO_TRADE`, `EVO_SCRIPT_TRIGGER`, `EVO_BATTLE_END`, `EVO_LEVEL_BATTLE_ONLY`); everything else is a stacked `CONDITIONS(...)` list using `IF_*` constants from `include/constants/pokemon.h` (`IF_MIN_FRIENDSHIP`, `IF_TIME`, `IF_HOLD_ITEM`, `IF_KNOWS_MOVE`, `IF_GENDER`, `IF_IN_MAPSEC`, `IF_WEATHER`, `IF_MIN_OVERWORLD_STEPS`, `IF_BAG_ITEM_COUNT`, and about 40 more). Real examples from the data:

```c
.evolutions = EVOLUTION({EVO_LEVEL, 30, SPECIES_MYMON2},
                        {EVO_ITEM, ITEM_FIRE_STONE, SPECIES_MYMON3}),
// friendship: level 0 means "any level up"
.evolutions = EVOLUTION({EVO_LEVEL, 0, SPECIES_PIKACHU, CONDITIONS({IF_MIN_FRIENDSHIP, FRIENDSHIP_EVO_THRESHOLD})}),
// night + held item, two ways to trigger
.evolutions = EVOLUTION({EVO_LEVEL, 0, SPECIES_GLISCOR, CONDITIONS({IF_TIME, TIME_NIGHT}, {IF_HOLD_ITEM, ITEM_RAZOR_FANG})},
                        {EVO_ITEM, ITEM_RAZOR_FANG, SPECIES_GLISCOR, CONDITIONS({IF_TIME, TIME_NIGHT})}),
```

`EVO_SCRIPT_TRIGGER` lets a script (`tryspecialevo`) evolve a Pokémon on demand, which is the hook for any custom evolution mechanic.

## Add a move

Tutorial: `docs/tutorials/how_to_new_move.md` (note the animation table it mentions is now a field, see step 3).

1. `include/constants/moves.h`: add `MOVE_MYMOVE,` right before `MOVES_COUNT = MOVES_COUNT_GEN9` and bump nothing else (the enum counts itself). Z and Max moves must stay after.
2. `src/data/moves_info.h`: add `[MOVE_MYMOVE] = { .name = COMPOUND_STRING("My Move"), .description = COMPOUND_STRING("..."), .effect = EFFECT_HIT, .power = 80, .type = TYPE_FIRE, .accuracy = 100, .pp = 10, .target = TARGET_SELECTED, .priority = 0, .category = DAMAGE_CATEGORY_PHYSICAL, .makesContact = TRUE, .additionalEffects = ADDITIONAL_EFFECTS({ .moveEffect = MOVE_EFFECT_BURN, .chance = 30 }), .battleAnimScript = gBattleAnimMove_MyMove, .contestEffect = ..., .contestCategory = ... }`.
3. Animation: add `gBattleAnimMove_MyMove::` in `data/battle_anim_scripts.s`, or point `.battleAnimScript` at an existing move's animation (`gBattleAnimMove_Ember`). Declare new labels in `include/battle_anim_scripts.h` alongside the others.
4. If no existing `EFFECT_*` fits: add `EFFECT_MYEFFECT` to `include/constants/battle_move_effects.h`, an entry in `src/data/battle_move_effects.h` `{ .battleScript = BattleScript_EffectMyEffect, ... }`, the script in `data/battle_scripts_1.s`, and declare it in `include/battle_scripts.h`. Copy the closest existing script. For dynamic power or type, add a case in `CalcMoveBasePowerAfterModifiers` (`src/battle_util.c`) or `SetTypeBeforeUsingMove` (`src/battle_main.c`).
5. Make it learnable: a level-up learnset entry, or a TM (`include/constants/tms_hms.h` `FOREACH_TM` list plus the TM item), or a tutor script.
6. Test: `test/battle/move_effect/` has one file per effect. Add a `SINGLE_BATTLE_TEST` for yours.

## Add an ability

1. `include/constants/abilities.h`: `ABILITY_MYABILITY,` before `ABILITIES_COUNT`.
2. `src/data/abilities.h`: `[ABILITY_MYABILITY] = { .name = _("My Ability"), .description = COMPOUND_STRING("..."), .aiRating = 5, }` plus flags like `.breakable = TRUE` if Mold Breaker should ignore it.
3. Logic: find where the closest existing ability lives (`grep -n "case ABILITY_SPEED_BOOST" src/`) and add a `case ABILITY_MYABILITY:` next to it. The main dispatch is `AbilityBattleEffects` in `src/battle_util.c` grouped by `ABILITYEFFECT_*` timing; stat and damage modifiers are in the damage calc; overworld effects in `src/ow_abilities.c`.
4. Assign it in a species' `.abilities = { ABILITY_A, ABILITY_B, ABILITY_HIDDEN }`.
5. Test in `test/battle/ability/myability.c`.

## Add an item

1. `include/constants/items.h`: `ITEM_MYITEM,` before `ITEMS_COUNT`. Keep it inside the enum; order does not matter to saves (bag stores ids) but never renumber existing ones.
2. Icon: `graphics/items/icons/my_item.png` (24x24, 16 colours) and `graphics/items/icon_palettes/my_item.pal`; declare `gItemIcon_MyItem` / `gItemIconPalette_MyItem` in `src/data/graphics/items.h` like the neighbours.
3. `src/data/items.h`: entry with `.name = ITEM_NAME("My Item")`, `.price`, `.description`, `.pocket` (`POCKET_ITEMS`, `POCKET_KEY_ITEMS`, `POCKET_POKE_BALLS`, `POCKET_TM_HM`, `POCKET_BERRIES`), `.sortType`, `.type`, `.fieldUseFunc`, `.iconPic`, `.iconPalette`. For a key item add `.importance = 1`. For a held item set `.holdEffect` and `.holdEffectParam`. For a medicine set `.type = ITEM_USE_PARTY_MENU`, `.fieldUseFunc = ItemUseOutOfBattle_Medicine`, `.effect = gItemEffect_MyItem` with the effect bytes in `src/data/pokemon/item_effects.h`. For a ball, `.battleUsage = EFFECT_ITEM_THROW_BALL`, `.secondaryId = BALL_X` (new ball kinds need `src/pokeball.c` graphics and catch logic).
4. Custom field behaviour: write `void ItemUseOutOfBattle_MyItem(u8 taskId)` in `src/item_use.c`, declare it in `include/item_use.h`. Look at `ItemUseOutOfBattle_EscapeRope` (sets a field callback then closes the bag) or `ItemUseOutOfBattle_CoinCase` (shows a message) for the two common shapes. Items with `.type = ITEM_USE_FIELD` can be registered to Select.
5. Give it: `giveitem ITEM_MYITEM` in a script, add to a shop list, or debug menu.

## Add a TM

Add the item as above with `.pocket = POCKET_TM_HM`, `.secondaryId = MOVE_X`, `.fieldUseFunc = ItemUseOutOfBattle_TMHM`, and add the TM to `FOREACH_TM` in `include/constants/tms_hms.h`. The learnset generator then grants it to every species that can learn the move.

## Add a trainer

1. `include/constants/opponents.h`: `#define TRAINER_MYTRAINER 855` and raise `TRAINERS_COUNT_EMERALD` to 856. There is room for 9 more before `MAX_TRAINERS_COUNT_EMERALD` (864); raising that costs one flag per trainer.
2. `src/data/trainers.party`: append a block. Minimal:

```
=== TRAINER_MYTRAINER ===
Name: JANE
Class: Cooltrainer
Pic: Cooltrainer F
Gender: Female
Music: Female
AI: Basic Trainer

Gardevoir @ Choice Specs
Level: 45
Ability: Trace
Nature: Timid
EVs: 252 SpA / 252 Spe
- Psychic
- Moonblast
- Shadow Ball
- Calm Mind
```

`Items: Full Restore / Full Restore` for trainer items, `Double Battle: Yes`, `Difficulty: Hard` blocks for variants, `Party Size: n` smaller than the list for a random pool. The header comment of the file documents every field.

3. Place them: an object event in the map JSON with `"trainer_type": "TRAINER_TYPE_NORMAL"`, `"trainer_sight_or_berry_tree_id": "4"` (tiles of sight), `"script": "MyMap_EventScript_Jane"`, and in `scripts.inc`:

```
MyMap_EventScript_Jane::
	trainerbattle_single TRAINER_MYTRAINER, MyMap_Text_JaneIntro, MyMap_Text_JaneDefeat
	msgbox MyMap_Text_JanePostBattle, MSGBOX_AUTOCLOSE
	end
```

4. New class or pic: `include/constants/trainers.h` and `gTrainerClasses` in `src/battle_main.c` (name, prize money multiplier, ball); pics per `docs/tutorials/how_to_trainer_front_pic.md`.
5. Rematches: add to `gRematchTable` in `src/battle_setup.c` and `include/constants/rematches.h`, or use the Vs Seeker (`docs/tutorials/vs_seeker.md`).
6. Test quickly: Debug menu → Utilities → Trainers → Try Battle, or `debug_trainers.party` for scratch teams.

## Add a scripted battle

- Wild: `setwildbattle SPECIES_X, 50, ITEM_NONE` then `dowildbattle`; `special HealPlayerParty` before if you want. For two wild Pokémon add `species2`, `level2`. Result in `VAR_RESULT` (`B_OUTCOME_*`). Legendary style with music and no fleeing: look at `data/scripts/static_pokemon.inc` and `seteventmon`.
- Trainer from a script without an NPC: `trainerbattle_no_intro TRAINER_X, DefeatText`.
- Two trainers at once: `trainerbattle_two_trainers`. With a partner: `setmultitrainerbattle` (see `src/battle_setup.c`, `SetMultiTrainerBattle`) and the partner list in `battle_partners.party`.
- Player-controlled vs AI-controlled: set the `B_FLAG_AI_VS_AI_BATTLE` flag before the battle.
- Inverse, sky, or other rule flags: the `B_FLAG_*` configs.

## Add a map

Use Porymap for the layout, but everything is plain files:

1. `data/layouts/layouts.json`: add `{ "id": "LAYOUT_MY_PLACE", "name": "MyPlace_Layout", "width": 20, "height": 20, "primary_tileset": "gTileset_General", "secondary_tileset": "gTileset_Petalburg", "border_filepath": "data/layouts/MyPlace/border.bin", "blockdata_filepath": "data/layouts/MyPlace/map.bin", "layout_version": "emerald" }`. Copy an existing `map.bin` and `border.bin` as a start (each metatile is a u16: 10 bits id, 6 bits collision and elevation).
2. `data/maps/MyPlace/map.json`: copy `LittlerootTown_BrendansHouse_1F/map.json` for an indoor map or `LittlerootTown/map.json` for outdoors. Set `id`, `name`, `layout`, `music`, `region_map_section`, `weather`, `map_type`, and empty or new event arrays.
3. `data/maps/MyPlace/scripts.inc`: at least

```
MyPlace_MapScripts::
	.byte 0
```

4. `data/maps/map_groups.json`: add `"MyPlace"` to a group array (for example `gMapGroup_IndoorDynamic` or a new group listed in `group_order`).
5. `data/event_scripts.s`: add `.include "data/maps/MyPlace/scripts.inc"`.
6. Warps: a `warp_events` entry in the source map pointing at `MAP_MY_PLACE` warp `0`, and one in the new map pointing back. Warp ids are indexes into the destination's `warp_events` array.
7. `make`. `mapjson` generates `MAP_MY_PLACE` and `LAYOUT_MY_PLACE` constants. Debug menu → Utilities → Warp reaches it.

New tileset: directory under `data/tilesets/secondary/`, header in `src/data/tilesets/headers.h`, graphics in `graphics.h`, metatiles in `metatiles.h`. Porymap can create one.

## Add an NPC with dialogue

1. In `map.json` `object_events`, add `{ "graphics_id": "OBJ_EVENT_GFX_WOMAN_1", "x": 5, "y": 7, "elevation": 3, "movement_type": "MOVEMENT_TYPE_FACE_DOWN", "movement_range_x": 1, "movement_range_y": 1, "trainer_type": "TRAINER_TYPE_NONE", "trainer_sight_or_berry_tree_id": "0", "script": "MyPlace_EventScript_Woman", "flag": "0" }`. Optionally `"local_id": "LOCALID_MYPLACE_WOMAN"` so scripts can `applymovement` it by name (constants generated into `map_event_ids.h`).
2. In `scripts.inc`:

```
MyPlace_EventScript_Woman::
	msgbox MyPlace_Text_Woman, MSGBOX_NPC
	end

MyPlace_Text_Woman:
	.string "Lovely weather today,\n"
	.string "{PLAYER}.$"
```

3. To hide the NPC after an event, set its `flag` to a flag id and `setflag` it. To make it a Pokémon, `"graphics_id": "OBJ_EVENT_GFX_SPECIES(PIKACHU)"`. To make it walk during a cutscene, `applymovement LOCALID_MYPLACE_WOMAN, MyPlace_Movement_Walk` + `waitmovement 0` with the movement sequence defined as `walk_left`, `walk_left`, `step_end`.

Common NPC behaviours are one script each in `data/scripts/`: `pkmn_center_nurse.inc` (heal), `mart_clerk.inc` (shop), `move_tutors.inc`, `day_care.inc`, `pc.inc`. Copy the calling pattern from any map that uses them.

## Add a shop or change prices

- **Shop**: script with `pokemart MyPlace_Items` where

```
MyPlace_Items::
	.2byte ITEM_POKE_BALL
	.2byte ITEM_POTION
	.2byte ITEM_MYITEM
	pokemartlistend
```

wrapped in `lock`/`faceplayer`/`message gText_HowMayIServeYou`/`waitmessage` before and `msgbox gText_PleaseComeAgain`/`release`/`end` after (see `data/scripts/mart_clerk.inc`). Items appear only if their `shopCriteriaFunc` allows; unset means always.
- **Buy price**: `.price` in `src/data/items.h`. Many are gen-dependent expressions like `(I_PRICE >= GEN_7) ? 1000 : 550`; replace with a literal.
- **Sell price**: `GetItemSellPrice` in `src/item.c` (half of price, quarter with `I_SELL_VALUE_FRACTION` at `GEN_9`). Change the divisor or make it per item.
- **Trainer prize money**: `4 × last mon level × class money multiplier` in `GetTrainerMoneyToGive` (`src/battle_script_commands.c`); class multipliers in `gTrainerClasses` (`src/battle_main.c`). Doubles pay double.
- **Money lost on whiteout**: `B_WHITEOUT_MONEY` in `include/config/battle.h` picks the formula; the code is next to `Cmd_getmoneyreward`.
- **Starting money**: `SetMoney(&gSaveBlock1Ptr->money, 3000)` in `NewGameInitData`.
- **Money cap**: `MAX_MONEY` in `include/money.h`.
- **Pay Day**, Amulet Coin: `gBattleStruct->moneyMultiplier` and the Pay Day script.

## Change the starting game

- Starter choice: `src/starter_choose.c` (`sStarterMon[]`) and the Birch bag script in `data/maps/Route101/scripts.inc` / `data/scripts/prof_birch.inc`.
- Starting map and cutscene: `WarpToTruck` in `NewGameInitData` and `EventScript_ResetAllMapFlags` in `data/scripts/new_game.inc`. Replace `WarpToTruck` with `SetWarpDestination`-style code to start elsewhere, or keep quickstart for development.
- Starting inventory: `NewGameInitPCItems` in `src/player_pc.c` and `ClearBag`; add `AddBagItem(ITEM_X, n)` after `ClearBag()` in `NewGameInitData`.
- Skip the intro entirely: `main_menu.c` and `oak_speech.c`/`src/intro.c` control it; quickstart already skips it in dev builds.

## Change wild encounters

`src/data/wild_encounters.json`: find the map's block (`"map": "MAP_ROUTE101"`), edit `land_mons` (12 slots with fixed rates 20,20,10,10,10,10,5,5,4,4,1,1), `water_mons`, `rock_smash_mons`, `fishing_mons` (old/good/super rod groups). `encounter_rate` is the per-step chance. Porymap edits this visually. Time-of-day tables: `docs/tutorials/how_to_time_of_day_encounters.md`.
