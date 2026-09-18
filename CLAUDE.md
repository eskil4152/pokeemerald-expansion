# CLAUDE.md

Read this first in every session. Then read `claude-guide/README.md` and the guide file that matches the task.

## What this is

Eskil's personal fork of rh-hideout/pokeemerald-expansion (version 1.17.1), a GBA ROM hack base built on pret's Pokémon Emerald decompilation. The goal is a **sandbox Pokémon game**: any mechanic, system or content Eskil wants, not limited to what Pokémon has. There is no fixed design; features are decided as they come. Nothing has been built yet beyond the guide.

Eskil is a developer. No git or tooling handholding is needed. Pronouns: they/them unless told otherwise.

## Environment

- macOS, zsh. Repo at `/Users/eskil/Documents/Projects/pokemon-source-code/pokeemerald-expansion`.
- Toolchain: devkitARM 16.1.0 at `/opt/devkitpro`. `DEVKITPRO` and `DEVKITARM` are exported in `~/.zshrc`; if a shell lacks them, `export DEVKITPRO=/opt/devkitpro DEVKITARM=/opt/devkitpro/devkitARM`.
- Homebrew: libpng, pkg-config installed. `coreutils` is not installed (needed for `make check`).
- Emulator: mGBA at `/Applications/mGBA.app`. Default keys: arrows, X = A, Z = B, Return = Start, Backspace = Select, A = L, S = R.
- **macOS 27 toolchain workaround**: the selected developer dir is Xcode 26.6, whose linker cannot read the macOS 27 SDK, so host tools (e.g. `tools/mid2agb`) fail to link. Build with `DEVELOPER_DIR=/Library/Developer/CommandLineTools` exported (the Command Line Tools 27 link fine). Permanent alternatives: `sudo xcode-select -s /Library/Developer/CommandLineTools`, or updating Xcode to 27.
- Git: `origin` is eskil4152/pokeemerald-expansion, `upstream` is rh-hideout/pokeemerald-expansion. `master` tracks upstream and stays clean; never merge into it without Eskil saying so. Features live on `feature/*` branches; `dev` integrates them.

## Commands

```bash
make -j$(sysctl -n hw.ncpu) && open -a mGBA pokeemerald.gba
```

`make check -j$(sysctl -n hw.ncpu)` runs tests. `make release` strips debug features. `make tidy` cleans build output. Full list in `claude-guide/README.md`.

In game: Select on the title screen quickstarts a new game; hold R + Start in the overworld opens the debug menu (warp, give anything, edit flags and vars, cheat start); Select in battle opens the battle debug menu.

## Working rules

- Work on a branch. Do not commit to `master`. Commit when a piece of work is complete and builds.
- Every change must build (`make`) before it is reported done. If it touches battle logic, run the relevant tests with `make check TESTS="..."`.
- Put new mechanics in their own `src/<name>.c` + `include/<name>.h`, expose them to scripts via `data/specials.inc`, and hook into existing code with single-line calls. This is both the upstream style rule and what keeps upstream merges cheap.
- Prefer flags and vars for persistent state. Changing a save struct breaks existing saves; say so when you do it.
- Never edit generated files (`src/data/trainers.h`, `wild_encounters.h`, `teachable_learnsets.h`, `include/constants/map_groups.h`, `layouts.h`, `map_event_ids.h`, `script_commands.h`, converted graphics). Edit their sources.
- Append new species, moves, items, abilities and trainers at the end of their enums; never renumber.
- Style per `docs/STYLEGUIDE.md`: 4 spaces, PascalCase functions, camelCase variables, `g`/`s` prefixes, enums not magic numbers, inline config checks.
- Effort: medium is enough for content and scripting work; use high for battle engine or save layout changes.

## Guide index (`claude-guide/`)

- `README.md`: run commands, in-game shortcuts, file index, external tools.
- `01-mental-model.md`: how the engine runs (main loop, callbacks, tasks, script languages, overworld and battle loops, persistence), how source becomes a ROM, where new code goes.
- `02-directory-map.md`: every directory and important file.
- `03-config.md`: `include/config/*.h` explained, sandbox presets.
- `04-where-things-live.md`: lookup table for species, moves, abilities, items, trainers, maps, scripts, text, UI, sound, time, save.
- `05-engine-systems.md`: the primitives with real APIs: flags/vars, save blocks, event scripts, specials/callnative, triggers, text, menus and screens, sprites, battle hooks, time, memory, RNG.
- `06-recipes-existing-content.md`: step-by-step for adding Pokémon, moves, abilities, items, TMs, trainers, battles, maps, NPCs, shops, prices, start of game, encounters.
- `07-recipes-new-mechanics.md`: patterns for things Pokémon lacks: currencies, world state, global rules and god mode, NPC relationship systems, overworld systems, custom screens, battle rules, player creation, and a checklist.
- `08-testing-and-debugging.md`: debug menus, printf, GDB, test framework, failure signatures.
- `09-gotchas.md`: what breaks saves, script pitfalls, battle and overworld limits, graphics rules, build rules, upstream merging.

## Current state

- 2026-09-17: Fork cloned, toolchain installed, first build succeeded, mGBA installed. Guide written on branch `docs/claude-guide` (merged to master).
- 2026-09-18: Features built on separate branches, each with a plan file in `claude-guide/features/`: sandbox menu, followers, wager battles, Pokémon editor, underground economy, quests, arena and hub. All manually tested by Eskil. Ascension designed (`FEATURE_ASCENSION.md`), not built.
- 2026-09-18: `dev` branch merges every feature branch plus the ascension plan; EXIT removed from the Start menu. Master is untouched upstream plus the guide. Ascension work happens on top of `dev`.
- 2026-09-18: Ascension phase 1 on `feature/ascension`: per-Pokémon tier and win counter in spare struct bits, stat bonus, ascended cry, summary star, editor fields, tuning in `src/data/ascension.h`. Tested.
- 2026-09-18: Ascension phases 2–5 on `feature/ascension`: Ascension Shard, Shrine map, mystic/Oracle, guardian ritual, rank trials I–V with quests, reign and visiting champions, post-champion scaling, arena bounty, nurse rumours. Trainer id space is now full (864). Difficulty and economy tie-ins not done. Awaiting manual test.

Update this section when work lands, one line per milestone with the date.
