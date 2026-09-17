# Claude Guide to this pokeemerald-expansion sandbox

## Run

Build the ROM and open it in mGBA:

```bash
make -j$(sysctl -n hw.ncpu) && open -a mGBA pokeemerald.gba
```

If `arm-none-eabi-gcc` is not found, the shell did not load `~/.zshrc`. Run `export DEVKITPRO=/opt/devkitpro DEVKITARM=/opt/devkitpro/devkitARM` first.

Other useful invocations:

| Command | What it does |
|---|---|
| `make -j$(sysctl -n hw.ncpu)` | Incremental build. Only changed files recompile. Output: `pokeemerald.gba` |
| `make check -j$(sysctl -n hw.ncpu)` | Run the automated test suite headless in mGBA (needs `brew install coreutils`) |
| `make check TESTS="Stun Spore"` | Run only tests whose name starts with that prefix |
| `make debug -j$(sysctl -n hw.ncpu)` | Build with `-Og -g` for GDB debugging (separate build dir) |
| `make release -j$(sysctl -n hw.ncpu)` | Optimised build with debug menus, quickstart and printf stripped. Output: `pokeemerald-release.gba` |
| `make syms` | Write `pokeemerald.sym` symbol file for mGBA |
| `make tidy` | Delete build output but keep tools and generated assets |
| `make clean` | Delete everything including generated files. Next build is slow |

In game, developer shortcuts that are on by default (all disabled by `make release`):

| Shortcut | Where | What |
|---|---|---|
| Select | Title screen | Quickstart: new game with no intro, drops you in the truck |
| Hold R, press Start | Overworld | Debug menu: warp, give items/Pokémon/money, edit flags and vars, start battles, cheat start |
| Select | In battle | Battle debug menu: edit stats, statuses, weather, moves on the fly |
| Select | Summary screen | Pokémon sprite visualiser |

The overworld debug menu has "Cheat Start" which gives a full party with HM users, all badges, Pokédex, running shoes and bike, and unlocks Fly to every town. That is the fastest way to get a sandbox save.

## What this project is

Your fork of rh-hideout/pokeemerald-expansion, version 1.17.1. It is pret's pokeemerald decompilation of Pokémon Emerald plus the Rom Hacking Hideout expansion: Gen 1 to 9 Pokémon, moves, abilities and items, a modern battle engine, and many developer tools. It is plain C for the GBA, compiled with devkitARM GCC 16, plus a custom scripting language for overworld events and another for battle effects.

The goal of this project is a sandbox: a Pokémon game where anything can be added or changed, not only more of what Pokémon already has. The guide is written to support that. Files 01 and 05 explain the engine primitives everything is built from. File 07 shows how to compose them into mechanics the game does not have.

## Files in this guide

| File | Read it when |
|---|---|
| [01-mental-model.md](01-mental-model.md) | You want to understand how the engine works: main loop, callbacks, tasks, how source becomes a ROM, what runs where |
| [02-directory-map.md](02-directory-map.md) | You want to know what a directory or file is for |
| [03-config.md](03-config.md) | You want to toggle behaviour without writing code, or find the flag for a feature |
| [04-where-things-live.md](04-where-things-live.md) | You want to find the data for a species, move, item, trainer, map, script, text, graphic, sound or save field |
| [05-engine-systems.md](05-engine-systems.md) | You are about to write code: scripting, flags and vars, save data, menus, text, sprites, battle hooks, time, memory |
| [06-recipes-existing-content.md](06-recipes-existing-content.md) | You want to add a Pokémon, move, ability, item, trainer, battle, map, NPC, shop, or change prices and rewards |
| [07-recipes-new-mechanics.md](07-recipes-new-mechanics.md) | You want to build something Pokémon does not have: new resources, world state, screens, NPC systems, battle rules, god mode |
| [08-testing-and-debugging.md](08-testing-and-debugging.md) | Something is broken, or you want tests |
| [09-gotchas.md](09-gotchas.md) | Before you change save structs, constants, or anything that looks harmless |

## External tools worth installing

- **Porymap** (github.com/huderlem/porymap): the map editor. Edits `data/maps/*/map.json`, layouts, tilesets, events and wild encounters visually. Nearly everyone uses it. Not required, the JSON is editable by hand.
- **Poryscript** (github.com/huderlem/poryscript): optional higher-level language that compiles to the `.inc` script format with `if`/`while`/`switch`. Not set up in this repo. The raw script language is fine for most work.
- **mGBA**: installed at `/Applications/mGBA.app`. It has a GDB server (Tools menu), a log viewer for printf output, and memory viewers.
