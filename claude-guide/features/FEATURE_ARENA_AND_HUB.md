# Feature: Arena and sandbox hub maps

Branch: `feature/arena-and-hub` (from master). Status: planned, not started. Depends on `feature/wager-battles` and `feature/underground-economy` for content; the maps themselves can be built first.

## Goal

Two new maps built from existing tilesets:

1. **Sandbox hub**: a small building reachable from Littleroot with a PC, a healer, a vendor that sells everything, and a warp back. The place to test everything.
2. **Underground arena**: a hidden room entered through a disguised door (a specific NPC or tile that opens only after a password or a heat threshold). Tiered bookies using wager battles paid in the underground currency, a leaderboard var, and a boss.

## Design

- Maps by hand-edited JSON plus Porymap for layouts: `data/layouts/layouts.json` entries, `data/layouts/<Name>/map.bin` + `border.bin`, `data/maps/<Name>/map.json` + `scripts.inc`, `map_groups.json` (a new group `gMapGroup_Sandbox`), `.include` lines in `data/event_scripts.s`.
- Hub: copy an existing Pokémon Center interior layout; NPCs are script-only (heal script, `pokemart` with a full list, PC via the standard PC metatile).
- Arena: copy a basement or cave interior; entrance warp placed in a Littleroot house or Route 101 tile guarded by a script that checks a flag.
- Tiers: `TRAINER_ARENA_1`..`5` in `trainers.party` with escalating stakes set by the bookie script; a var records the highest tier beaten; the boss only appears at max.

## Files (planned)

`data/layouts/*`, `data/maps/SandboxHub/*`, `data/maps/UndergroundArena/*`, `data/maps/map_groups.json`, `data/event_scripts.s`, `src/data/trainers.party`, `include/constants/opponents.h`, one entrance edit in an existing map.

## Save impact

None from the maps. `MAX_TRAINERS_COUNT` may need raising for the tier trainers (save-breaking); coordinate with the economy branch.

## Notes

Install Porymap before starting: layouts are binary and painful by hand. Reusing tilesets keeps this to hours, not days.
