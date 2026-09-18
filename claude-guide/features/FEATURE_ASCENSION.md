# Feature: Ascension, reign and visiting champions

Branch: `feature/ascension` (built on `dev`). Status: phase 1 implemented, builds, awaiting manual test. Phases 2–5 not started.

## Goal

Beating the Champion is the start, not the end. The player holds the title (reign), climbs five ascension ranks through a post-game questline, and permanently awakens Pokémon through guardian trials. The world reacts to each state: difficulty, NPC dialogue, quests and the underground all follow the player's rank. Hard, not impossible.

## Game states

Pre-champion → Champion → Rank I → II → III → IV → V. Every system below reads the current state from the player's rank var and the reign state.

## Player rank (0–5)

Earned through the main Ascension questline, which the Journal already supports as locked entries (`QUEST_ASCENSION_I` exists as a placeholder on `feature/quests`). Each rank-up grants shards and raises the ceiling for Pokémon tiers.

| Rank | Trial |
|---|---|
| I | Become Champion, then hold the title for 7 in-game days (see Reign) |
| II | Clear the underground arena (all five tables) |
| III | Defeat a visiting champion |
| IV | Win a gauntlet of five battles in a row at the shrine, no items |
| V | Defeat the Ascended One: a tier 5 team, the only fight ever above your rank |

## Pokémon tier (0–5)

Stored on each Pokémon in spare bits of `struct PokemonSubstruct0` (verified unused: `unused_04:3` for the tier, `unused_02:6` for the win counter). No struct resize, not save-breaking: existing Pokémon read as tier 0. Accessed through new `MON_DATA_ASCENSION_TIER` and `MON_DATA_ASCENSION_WINS` cases in `GetBoxMonData`/`SetBoxMonData`.

- **Stat bonus**: applied in stat calculation to all six stats including HP. 8, 16, 24, 32, 50 percent.
- **Cry**: raised pitch from tier 3, reusing the path that pitch-shifts Mega cries (`src/sound.c`).
- **Display**: a star mark with the tier in the summary screen; a Tier field in the Pokémon editor.
- **Win counter**: trainer battles won in which the Pokémon took part. Wild battles do not count. Caps at 63, resets on each ascension.

## Ascending a Pokémon

At the shrine, a new map on the Sky Pillar summit layout, opened after becoming Champion.

1. Choose the Pokémon. It must meet the step's requirements.
2. Pay the step's shards. They stay dormant until the guardian yields.
3. The guardian challenges that Pokémon alone. The rest of the party is set aside and restored afterwards, as the Battle Tents do (`SavePlayerParty` / `LoadPlayerParty`).
4. Held items allowed, bag disabled (via `B_VAR_NO_BAG_USE`). Guardians cannot be caught.
5. **Win**: shards consumed, tier raised, cutscene and fanfare. **Lose**: no whiteout, party healed, shards kept.

Guardian rules: always at the target tier, one above the challenger. Level is the challenger's level plus the step's offset, so the fight is about the tier gap, matchups and moves, not grinding levels.

## Tuning: one data file

`src/data/ascension.h` holds every number. The code only reads the tables. Starting values:

```c
//         rank  friendship  wins  shards  statBonus%  guardian                 levelOffset
[TIER_1] = { 1,       255,     0,      1,         8,  SPECIES_SHAYMIN,          -5 },
[TIER_2] = { 2,       255,    20,      1,        16,  SPECIES_DARKRAI,          -3 },
[TIER_3] = { 3,       255,    30,      2,        24,  SPECIES_DIALGA,           -3 },
[TIER_4] = { 4,       255,    40,      2,        32,  SPECIES_PALKIA,            0 },
[TIER_5] = { 5,       255,    50,      3,        50,  SPECIES_GIRATINA_ORIGIN,  +2 },
```

- No level requirement (a minimum level column exists, set to 0).
- Guardians are all unobtainable elsewhere in Emerald. Dialga and Palkia also have Origin Formes if wanted.
- A second table holds per rank: trainer level floor, the highest tier NPCs may field, and the difficulty setting.

## Shards

A new key item, Ascension Shard (icon reused from the existing shard items). Sources are quests only, no drops, no farming, no shop:

- The main ascension questline grants shards at each rank.
- Repeatable post-game bounties grant more: beat a visiting champion, clear the arena, win a gauntlet.

Costs above: 9 shards per Pokémon to tier 5, 54 for a full team. The main line starts you off; bounties are the long-term loop.

## Reign and visiting champions (one system)

- Becoming Champion starts the reign: a day counter of how long you have held the title.
- Canon champions visit on a weekly rotation and challenge the title. Sprites already exist for Red, Leaf, Blue, Lance, Steven and Wallace. Others (Cynthia, Leon, ...) need front sprites added (art, tutorial in `docs/tutorials/how_to_trainer_front_pic.md`).
- Their teams are awakened, capped at the player's rank. Their dialogue references your reign and rank: your dominance is known.
- Beating one keeps the title and grants a bounty. Losing dethrones you until you win it back.
- Rank I requires holding the title for 7 days, so the first challenge comes within that week.

## NPC rules

- Regular trainers after becoming Champion: level scaled, always tier 0.
- Scaling is mixed: each rank sets a level floor; if the party average is higher, trainers rise to it minus a small margin. Never below the floor.
- Visiting champions: awakened, capped at the player's rank.
- Arena boss: gains tiers as the player's rank rises.
- Only the Rank V trial exceeds the player's rank.

## World reaction

- **Difficulty**: the expansion's difficulty var (`B_VAR_DIFFICULTY`) is tied to rank, so trainers with hard variants switch automatically.
- **Dialogue**: key NPCs get lines per rank (mom, Birch, the rival, gym leaders, nurses). Random townsfolk draw from a rumour pool keyed on rank. Not every NPC in Hoenn.
- **Quests**: side quests unlock by rank through the Journal's lock mechanism.
- **Economy**: heat thresholds and underground prices shift with rank.

## Dependencies

`feature/quests` (Journal, questline), `feature/underground-economy` and `feature/arena-and-hub` (rank II trial, bounties, heat), `feature/pokemon-editor` (tier field). Build on a `dev` branch merging all tested features.

## Phases

1. **Done.** Tier storage, stat bonus, ascended cry, star mark, editor fields, `src/data/ascension.h`.
2. **Done.** Shard item, shrine map, guardian ritual.
3. **Done.** Rank questline and its trials.
4. **Done.** Reign and visiting champions, post-champion scaling, bounties.
5. **Partly done.** Nurse rumours. Difficulty and economy tie-ins not done (see below).

## State and save

- Tier and win counter: spare bits on each Pokémon.
- Vars (were unused): `VAR_ASCENSION_RANK` 0x40A1, `_REIGN_DAYS` 0x40A8, `_VISITOR` 0x40B8, `_CHALLENGE_DAYS` 0x40BB, `_BOUNTY_DAY` 0x40DB, `_NEXT_VISITOR` 0x40DC, `_NO_BAG` 0x40E5 (this is `B_VAR_NO_BAG_USE`).
- Flags (were unused): `FLAG_ASCENSION_CHALLENGE_PENDING` 0x2E, `_DETHRONED` 0x2F, `_BEAT_VISITOR` 0x30, `FLAG_HIDE_ASCENSION_VISITOR` 0x31, `FLAG_ASCENSION_MET_ORACLE` 0x32.
- Shards: `ITEM_ASCENSION_SHARD` (874, Items pocket, cannot be sold or tossed).
- Trainer ids: 851 `TRAINER_RED`, 852 `TRAINER_LEAF` (unused FRLG placeholders, now real teams), 853 `TRAINER_VISITOR_BLUE` and 854 `TRAINER_VISITOR_LANCE` (were the unused Brendan/May placeholders), 861 `TRAINER_VISITOR_STEVEN`, 862 `TRAINER_VISITOR_WALLACE`, 863 `TRAINER_ASCENDED_ONE`. The trainer id space (864) is now full; more trainers need `MAX_TRAINERS_COUNT` raised, which shifts system flags and breaks saves.
- No save struct changes. Old saves keep working.

## Phase 1 implementation notes

- `struct PokemonSubstruct0`: `unused_04:3` became `ascensionTier:3`, `unused_02:6` became `ascensionWins:6`. Accessed with `MON_DATA_ASCENSION_TIER` / `MON_DATA_ASCENSION_WINS`. Struct size unchanged; existing Pokémon read 0.
- If upstream ever widens `heldItem` (10 bits, next to the win counter) the merge will conflict loudly on that line; move the counter to `unused_0A:2` plus another spare field then.
- Stat bonus: `Ascension_ApplyStatBonus` called for each non-HP stat after nature and friendship, and for max HP, in `CalculateMonStatsCont` (`src/pokemon.c`). Shedinja keeps 1 HP. Everything that recalculates stats (level up, PC, editor, battle end) picks it up; battles copy stats from the party.
- Cry: new `CRY_MODE_ASCENDED` in `src/sound.c`, tuned by `ASCENSION_CRY_*` in `include/constants/ascension.h`. Used for the healthy single cry when sent out (`src/pokeball.c`) and in the summary screen, from tier 3. Weak, fainting and double-battle cries are unchanged. The Howl mode first planned was too short and shrill, so the ascended mode is full length with a slight pitch rise and chorus.
- Summary: the mark is drawn left of the level, e.g. "★3 Lv50 ♂" (the gender symbol sits right of the level).
- Editor: "Asc. Tier" (0–5) and "Asc. Wins" (0–63) fields.
- Tuning: `src/data/ascension.h` holds the step table (with guardians) and the rank table (not read until phase 4).
- No way to ascend in game yet except the editor; the shrine comes in phase 2.

## Phases 2–5 implementation notes

Code: `src/ascension.c` (logic), `src/data/ascension.h` (every number and table, plus rumours), `include/constants/ascension.h` (timers, bonuses, result codes), `data/scripts/ascension.inc` (mystic, Oracle, trials, visitors, bounty, rumours), `data/maps/AscensionShrine/`.

Engine hooks (one line each):
- `CreateNPCTrainerParty` (`src/battle_setup.c`) calls `Ascension_OnTrainerPartyCreated`: scaling and NPC tiers.
- `CB2_EndScriptedWildBattle` (`src/battle_setup.c`): no whiteout during a guardian battle.
- `HandleEndTurn_FinishBattle` (`src/battle_main.c`) calls `Ascension_OnBattleFinished`: win counter.
- `DoDailyEvents` (`src/clock.c`) calls `Ascension_DoDailyEvents`: reign.
- `Common_EventScript_PkmnCenterNurse`, `Arena_EventScript_AfterFight`, League lobby on-transition: one `call`/`special` each.

Getting there: after becoming Champion, a mystic stands in the Pokémon League lobby (left side) and in the Sandbox Hub (left of the mart). Before that they refuse. The first talk starts Ascension I. Yes warps to the Shrine (Sky Pillar summit layout, Oracle at the top). The stairs always lead back to the League lobby door, also when you came from the hub.

Ascending: Oracle → Ascend a Pokémon → pick one. `Special_Ascension_CheckRequirements` checks, in order: egg, max tier, rank, level, friendship, wins since last step, shards. On yes the party is stashed in EWRAM, the challenger fights the guardian alone (bag disabled through `VAR_ASCENSION_NO_BAG` = 2), the party is restored and healed. Win: shards spent, tier +1, win counter reset. Loss or running: nothing spent. Guardian level = challenger level + offset, perfect IVs, tier = the target tier.

Win counter: every Pokémon sent out in a won trainer battle gets +1 (max 63). Link, recorded, Frontier and Trainer Hill battles do not count.

Trials (Oracle → Present my trial). Each pass gives +1 rank and 2 shards, completes that quest and starts the next:
- I: hold the title 7 days (`VAR_ASCENSION_REIGN_DAYS` ≥ 7).
- II: `VAR_ARENA_TIER` ≥ 5 (all arena tables cleared).
- III: beat a visiting champion once (`FLAG_ASCENSION_BEAT_VISITOR`).
- IV: gauntlet at the Shrine: Sidney, Phoebe, Glacia, Drake, Wallace back to back, no items, no healing between; a loss ends it with no penalty.
- V: the Ascended One (Brendan sprite, Tyranitar/Salamence/Metagross/Latios/Garchomp/Rayquaza, all tier 5). Items allowed.
While dethroned the Oracle refuses trials until you reclaim the title.

Reign: each in-game day as Champion adds a reign day. Every 7th reign day a visiting champion arrives in the League lobby (rotation Red, Leaf, Blue, Lance, Steven, Wallace). Answer within 3 days or lose the title. Losing the fight also loses it; the visitor then holds the title in the lobby until beaten. Winning pays 1 shard. Ranks are never lost.

Scaling (only after `FLAG_IS_CHAMPION`): regular trainers are raised to max(rank floor, party average − 5); Elite Four and Wallace to max(floor, average + 2); visitors average + 3 and tier min(visitor cap, rank cap); the Ascended One average + 5. Levels are only raised, never lowered, and moves are kept. Arena trainers and the wager demo are not scaled; the arena boss gets the rank's NPC tier.

Bounty: beating the arena boss (table 5) pays 1 shard once per 7 days.

World: nurses share a rumour about you one visit in three once you are Champion; text pool by rank in `src/data/ascension.h`.

Shards available: 10 from trials, 1 per visitor win, 1 per week from the arena. One Pokémon to tier 5 costs 9.

Not done:
- Difficulty: `B_VAR_DIFFICULTY` is not assigned. `DIFFICULTY_EASY` is 0, so pointing it at a var would put existing saves on Easy, and no trainer has Hard parties yet, so the rank table's difficulty column has no effect. To use it: assign a var, set it with `Script_SetDifficulty` on rank up, and add `Difficulty: Hard` parties.
- Economy by rank (heat threshold, prices) and Mom/Birch lines.
- Guardians use the wild AI; `B_VAR_WILD_AI_FLAGS` could make them smarter.
- Scaled trainers keep their movesets and do not evolve.

Testing shortcuts (debug menu, R + Start): set `FLAG_IS_CHAMPION` (0x87F), `VAR_ASCENSION_RANK` (0x40A1), `VAR_ASCENSION_REIGN_DAYS` (0x40A8), give `ITEM_ASCENSION_SHARD`, and max friendship / wins with the Pokémon editor. To force a visitor: set `VAR_ASCENSION_VISITOR` (0x40B8) to 1–6 and `FLAG_ASCENSION_CHALLENGE_PENDING` (0x2E), then re-enter the League lobby.

## Decisions taken

- Shrine entrance: mystic in the League lobby plus a copy in the hub.
- Dethroning: loss, or 3 days unanswered. The visitor holds the title at the League until beaten. Ranks never drop.
- Rank table: floors 60/65/70/75/80/90, NPC tier caps 0–5. Placeholders; tune in `src/data/ascension.h`.
- Visiting champions: Red, Leaf, Blue, Lance, Steven, Wallace.
- Dialga and Palkia: base formes.
