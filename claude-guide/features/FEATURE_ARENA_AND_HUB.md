# Feature: Arena and sandbox hub maps

Branch: `feature/arena-and-hub`. Contains `feature/wager-battles` and `feature/underground-economy` merged in, because the arena is built from both. Status: implemented, builds, awaiting manual test.

## Goal

1. **Sandbox hub**: a room to heal, buy anything and relearn moves, reachable from Littleroot.
2. **Underground arena**: a hidden gambling den with a five-table ladder of wager battles paid in UCOIN.

## Player-facing behaviour

### Hub

- A scientist (the guide) stands in Littleroot at (18, 12). Say yes and you are taken to the hub.
- Inside: the nurse heals, the clerk sells 57 items (every ball including Master Ball, all medicine, vitamins, Rare Candy, PP Up/Max, Ability Capsule/Patch, Bottle Caps, every evolution stone, competitive held items, Exp. Share), the hex maniac is the move relearner, and the room's PC works.
- The exit mat returns you beside the guide.

### Arena access

1. Do one deal with the fence at (1, 15) in Littleroot, buying or selling a Pokémon.
2. He then offers the way downstairs for a one-time 100 UCOIN buy-in. He asks again after later sales if you decline, and at every greeting until you pay.
3. After paying, his menu has "Go downstairs".
4. With heat at 80 or more he refuses to deal, arena included. The bookies inside also refuse at that heat.
5. The exit mat returns you to Littleroot beside him at (2, 15).

### Arena ladder

The host near the entrance explains the rules and shows tables beaten and your UCOIN.

| Table | Where | Trainer | Level | House stake |
|---|---|---|---|---|
| 1 | Slot machines, (4, 6) | SLICK, Team Aqua | 18 | 150 UCOIN |
| 2 | (16, 5) | KNUCKLES, Black Belt | 28–29 | 400 UCOIN |
| 3 | (12, 7) | MADAME, Hex Maniac | 38–39 | 900 UCOIN + Lv 30 Bagon |
| 4 | (20, 8) | ACE, Expert, smart AI | 48 | 2000 UCOIN |
| 5 | Behind the counter, (13, 2) | HOUSE, Gentleman, smart AI, 6 Pokémon | 60–61 | 5000 UCOIN + Lv 50 Garchomp |

- Each table only takes your bet once the one before it is beaten. Tables can be replayed.
- Your stake is UCOIN (50, 200, 500 or 2000), a party Pokémon, or both. "Nothing" means no fight.
- Every fight adds 5 heat.
- Losing never whites out: the party is healed and you keep playing.
- Beating table 5 plays the badge fanfare.

## Design decisions

- **No new tile art.** Both maps reference existing layouts directly: `LAYOUT_POKEMON_CENTER_1F` and `LAYOUT_MAUVILLE_CITY_GAME_CORNER`. No binary layout files were copied. The Game Corner's slot machines and prize counter are decoration because their events were not copied.
- **Return trips use the dynamic warp.** The guide and the fence store where to come back to with `setdynamicwarp`, and the exit mats target `MAP_DYNAMIC`. This is how vanilla link rooms return you. A later "Go to hub" row in the Sandbox menu can reuse it by storing the player's position instead.
- **Currency switch in the wager code.** `FLAG_WAGER_IN_UCOIN` makes the stake picker offer UCOIN amounts, checks and moves UCOIN instead of money, and formats every amount as "N UCOIN" or "¥N". Settlement clears the flag.
- **Ladder state** is one var, `VAR_ARENA_TIER`, the highest table beaten. Gating and recording are specials in `src/arena.c` rather than script arithmetic.
- New maps are in their own group, `gMapGroup_Sandbox`, appended last so no existing map id changes.

## Fixes made while building this

These were bugs in the dependency branches, fixed on those branches first and then merged in:

- Wager battles were flagged as the first-battle tutorial through an upstream bitwise check, which replaced the opponent's AI. Fixed in `src/battle_setup.c` on `feature/wager-battles`.
- The wager stake picker wiped the opponent's stake on entry, so wins paid nothing. Fixed on `feature/wager-battles`.
- The fence was placed inside Birch's lab building. Moved to (1, 15) on `feature/underground-economy`.

## Files

- New: `include/constants/arena.h`, `include/arena.h`, `src/arena.c`, `data/maps/SandboxHub/{map.json,scripts.inc}`, `data/maps/UndergroundArena/{map.json,scripts.inc}`.
- Changed: `include/constants/flags.h` (`FLAG_WAGER_IN_UCOIN`, `FLAG_ARENA_ACCESS`, `FLAG_UNDERGROUND_TRUSTED`), `include/constants/vars.h` (`VAR_ARENA_TIER`), `include/constants/opponents.h` (`TRAINER_ARENA_1`..`5`, 856–860, count 861), `src/data/trainers.party`, `src/wager.c`, `data/scripts/wager.inc`, `data/scripts/underground.inc`, `data/maps/map_groups.json`, `data/maps/LittlerootTown/map.json` (guide), `data/event_scripts.s`, `data/specials.inc`.

## Save impact

The economy's save struct is in this branch, so it is save-breaking relative to master. The arena itself only uses previously unused flags, one unused var, and trainer flag slots within the existing maximum.

## How to test

1. Delete the old `.sav`, new game, reach Littleroot.
2. Hub: talk to the scientist at (18, 12), heal, buy something, try the relearner and the PC, step on the exit mat. You land beside him.
3. Fence at (1, 15): sell a Pokémon. He offers the arena for 100 UCOIN. If short, sell another.
4. Pay, pick "Go downstairs". You arrive at the Game Corner door.
5. Talk to the host: rules and status.
6. Talk to table 2 first: refused. Beat table 1 with a UCOIN stake: payout and "next table" message.
7. Lose a table on purpose with a Pokémon staked: it is gone, the party is healed, no whiteout.
8. Step on the exit mat: back in Littleroot beside the fence.
9. Raise heat to 80 (debug menu or repeated fights): fence and bookies refuse.

## Follow-ups

- A "Go to hub" row in the Sandbox menu once that branch and this one meet.
- Real names for UCOIN, the arena and the bookies.
- A proper entrance in a building's back room once new maps exist.
