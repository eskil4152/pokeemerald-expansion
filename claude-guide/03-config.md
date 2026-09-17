# 03. Configuration

All configuration is `#define`s in `include/config/*.h`. `global.h` includes `config/general.h` and `config/save.h`, and most others are included where they are used, so changing one usually triggers a large rebuild. There is no runtime config; it is compile time.

## How the values work

- **Generation switches**: most battle and Pokémon options are set to `GEN_LATEST`, which is `GEN_9` (defined in `config/general.h`). Setting a switch to `GEN_3`, `GEN_4`, ... picks that generation's behaviour. Setting `GEN_LATEST` itself to another value flips everything at once. `GEN_CHAMPIONS` is one above `GEN_9`.
- **Booleans**: `TRUE`/`FALSE`.
- **Flag and var hooks**: options like `B_FLAG_NO_WHITEOUT` or `OW_FLAG_NO_COLLISION` default to `0`, meaning off. To enable such a feature you assign it a flag id: pick an unused one from `include/constants/flags.h` (they are named `FLAG_UNUSED_0x...`, 375 of them), rename it to something meaningful, and set the config to that name. Then the feature is on whenever that flag is set in the save, which you control from scripts (`setflag`), C (`FlagSet`), or the debug menu. Same idea for `B_VAR_*` with `VAR_UNUSED_0x...` vars. This is the pattern to copy for your own toggleable mechanics.
- **`DISABLED_ON_RELEASE` / `ENABLED_ON_RELEASE`**: evaluate to `TRUE` in normal builds and flip in `make release`. Debug menus use this.

## The config files

| File | Controls | Notable for a sandbox |
|---|---|---|
| `general.h` | Printf handler, `BUGFIX`, `GEN_*` constants, `GEN_LATEST`, intro, units, keyboard | `EXPANSION_INTRO` to remove the RHH splash. `LOG_HANDLER_MGBA_PRINT` is on so `DebugPrintf` shows in mGBA's log viewer |
| `debug.h` | Debug menus | `DEBUG_OVERWORLD_MENU`, `DEBUG_OVERWORLD_HELD_KEYS` (R), `DEBUG_OVERWORLD_TRIGGER_EVENT` (Start), `DEBUG_OVERWORLD_IN_MENU` to put it in the Start menu instead, `DEBUG_BATTLE_MENU`, `DEBUG_POKEMON_SPRITE_VISUALIZER` |
| `quickstart.h` | Select on title screen starts a new game | Gender, HUD position |
| `battle.h` (457 lines) | Every battle mechanic: crit, exp, damage formulas, type chart, turn counts, move data generation, abilities, items, catching, UI speed, animations, trainer AI defaults, flags | `B_FLAG_NO_WHITEOUT` (never lose against trainers), `B_FLAG_INVERSE_BATTLE`, `B_FLAG_AI_VS_AI_BATTLE`, `B_FLAG_SLEEP_CLAUSE`, `B_VAR_DIFFICULTY`, `B_VAR_NO_BAG_USE`, `B_WHITEOUT_MONEY`, `B_EXP_CATCH`, `B_SPLIT_EXP`, `B_FAST_*` and `B_WAIT_TIME_MULTIPLIER` for faster battles, `B_RUN_TRAINER_BATTLE`, `B_SHOW_TYPES`, `B_SHOW_EFFECTIVENESS`, `B_TRAINERS_KNOCK_OFF_ITEMS`, `B_CRITICAL_CAPTURE`, `B_LAST_USED_BALL`, `B_CATCH_SWAP_INTO_PARTY` |
| `pokemon.h` | Species data updates per gen, learnset generation, evolution and breeding, sprites style, cries, shiny forcing, Pokédex gaps | `P_FLAG_FORCE_SHINY`, `P_FLAG_FORCE_NO_SHINY`, `P_EGG_HATCH_LEVEL`, `P_FRIENDSHIP_EVO_THRESHOLD`, `P_CAN_FORGET_HIDDEN_MOVE`, `P_CRIES_ENABLED` (25% of ROM), `P_LEARNSET_HELPER_TEACHABLE` |
| `species_enabled.h` (606 lines) | Which families, forms, megas, regional forms are compiled in | Disabling generations or families saves ROM space. Changing the latest enabled generation changes the save layout (dex flags) |
| `item.h` | Item behaviour per gen, TM reuse, Exp Share flag, repel menu, Vs Seeker, dowsing | `I_REUSABLE_TMS`, `I_EXP_SHARE_FLAG` (assign a flag for party-wide exp), `I_SELL_VALUE_FRACTION`, `I_PRICE`, `I_VS_SEEKER_CHARGING` |
| `caps.h` | Level caps and EV caps | `B_EXP_CAP_TYPE` hard/soft, `B_LEVEL_CAP_TYPE` by badge flags or by a var, `B_RARE_CANDY_CAP`. The badge-to-level table is in `src/caps.c` |
| `overworld.h` (169 lines) | Running indoors, poison, whiteout cutscene, field moves, PC behaviour, berries, overworld Pokémon and followers, out-of-battle abilities, time and fake RTC, day/night tint, shadows, map popups, ambient cries | `OW_FLAG_NO_COLLISION` (walk through walls), `OW_FLAG_NO_TRAINER_SEE`, `OW_FLAG_PAUSE_TIME`, `OW_FLAG_POKE_RIDER` (fly from the map), `OW_FOLLOWERS_ENABLED` (HGSS following Pokémon), `OW_USE_FAKE_RTC`, `OW_ENABLE_DNS` (day/night palette tint, on by default), `OW_TIME_OF_DAY_ENCOUNTERS`, `OW_POPUP_GENERATION`, `OW_WHITEOUT_CUTSCENE`, `OW_CHECK_FOR_TOTAL_EVS` |
| `wild_encounter.h` | Random encounters, double wilds, overworld visible encounters | `WE_FLAG_NO_ENCOUNTER`, `WE_FLAG_NO_CATCHING`, `WE_FLAG_NO_RUNNING`, `WE_FLAG_FORCE_DOUBLE_WILD`, `WE_SMART_WILD_AI_FLAG`, `WE_OW_ENCOUNTERS` (Pokémon visible on the map, PLA style) |
| `save.h` | Skip save confirmation; free unused vanilla save data | Each `FREE_*` removes a vanilla feature's bytes from the save structs, up to 3790 bytes total. Use these when you need room for new persistent data. Each one breaks existing saves |
| `ai.h` | AI switch probabilities and behaviour tuning | Percent chances the AI switches in various situations |
| `dexnav.h` | DexNav feature and its flags/vars | Off by default. Needs several flags and vars assigned |
| `summary_screen.h` | IV/EV display, nature colours, rename, move relearner from the summary | `P_SUMMARY_SCREEN_IV_EV_INFO`, `P_ENABLE_MOVE_RELEARNERS`, `P_ENABLE_ALL_LEVEL_UP_MOVES` |
| `text.h` | Text speed, auto scroll, instant text | `TEXT_SPEED_INSTANT`, `FLAG_TEXT_SPEED_INSTANT` |
| `follower_npc.h` | DPPt style NPC followers | `FNPC_ENABLE_NPC_FOLLOWERS` |
| `pokerus.h`, `fishing.h`, `contest.h`, `name_box.h`, `map_preview_screen.h`, `pokedex_plus_hgss.h` | Their named features | `POKEDEX_PLUS_HGSS` switches to the detailed Pokédex |
| `test.h` (1148 lines) | Test-only overrides | Do not edit |

## Sandbox presets worth setting early

These are the single-line changes that most affect a "do what I want" game. Each needs a rebuild and a new save when marked.

| Goal | Change |
|---|---|
| Never lose to trainers | `B_FLAG_NO_WHITEOUT` → an unused flag, then set it in a script or the debug menu |
| Walk through walls | `OW_FLAG_NO_COLLISION` → an unused flag |
| Trainers only battle when spoken to | `OW_FLAG_NO_TRAINER_SEE` → an unused flag |
| No random encounters | `WE_FLAG_NO_ENCOUNTER` → an unused flag |
| Whole party gains exp | `I_EXP_SHARE_FLAG` → an unused flag, or `I_EXP_SHARE_ITEM GEN_6` to make the Exp Share item toggle it |
| Instant text | `TEXT_SPEED_INSTANT TRUE` in `text.h` |
| Faster battles | `B_WAIT_TIME_MULTIPLIER` lower, `B_FAST_INTRO_NO_SLIDE TRUE` |
| Following Pokémon | `OW_FOLLOWERS_ENABLED TRUE` (new save; the tutorial `docs/tutorials/how_to_new_pokemon.md` section on followers explains scripting needs) |
| Visible wild Pokémon on the map | `WE_OW_ENCOUNTERS TRUE` and `OW_GFX_COMPRESS FALSE` |
| Difficulty modes for trainers | `B_VAR_DIFFICULTY` → an unused var. Then `trainers.party` can define `Difficulty: Hard` variants |
| Level cap tied to badges | `B_EXP_CAP_TYPE EXP_CAP_HARD`, `B_LEVEL_CAP_TYPE LEVEL_CAP_FLAG_LIST` |
| Smaller ROM | `P_GEN_x_POKEMON FALSE` for generations you do not want, or `P_CRIES_ENABLED FALSE` |
| Remove RHH intro | `EXPANSION_INTRO FALSE` |

## Where a config is actually read

`grep -rn CONFIG_NAME src/` finds the code. Most are read inline (`if (B_SOMETHING >= GEN_5)`) per the style guide, so the code path is easy to follow. `src/config_changes.c` builds a table of battle configs so tests can override them; you do not need it unless writing tests that vary configs.
