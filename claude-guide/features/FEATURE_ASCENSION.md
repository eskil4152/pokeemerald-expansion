# Feature: Ascension, reign and visiting champions

Branch: `feature/ascension` (from master). Status: design agreed, not started. Building waits for manual tests of the dependency branches and a `dev` integration branch that merges them.

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

1. Tier storage, stat bonus, cry pitch, star mark, editor field, `src/data/ascension.h`.
2. Shard item, shrine map, guardian ritual.
3. Rank questline and its trials.
4. Reign and visiting champions, post-champion scaling.
5. Dialogue, difficulty and economy tie-ins.

## State and save

- Tier and win counter: spare bits on each Pokémon.
- Player rank, reign day counter, reign state: unused vars (`0x40A1`, `0x40A8`, `0x40B8` are free across all branches).
- Shards: bag items.
- No save struct changes planned.

## Open decisions

- Where the shrine entrance is (proposed: an NPC at the Pokémon League after becoming Champion).
- Dethrone details: who holds the title after a loss and how to reclaim it.
- Rank table numbers: level floors per rank and NPC tier caps.
- Which canon champions beyond the six with existing sprites.
- Dialga and Palkia base or Origin Formes.
