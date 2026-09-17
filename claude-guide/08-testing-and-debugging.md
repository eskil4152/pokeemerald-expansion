# 08. Testing and debugging

## The fast loop

1. Edit.
2. `make -j$(sysctl -n hw.ncpu)` (seconds for a one-file change; a config header change rebuilds most of the tree, a few minutes).
3. In mGBA, File → Load ROM (recent list) or `open -a mGBA pokeemerald.gba` again. mGBA keeps the `.sav` next to the ROM, so your save persists across rebuilds unless you changed a save struct.
4. Use the debug menu (hold R, press Start) to jump to what you changed.

Save states (Shift+F1 to save, F1 to load in mGBA) survive rebuilds only if RAM layout did not change. After any C change treat them as suspect; after a save struct change delete the `.sav` too.

## Debug menu (overworld)

`src/debug.c`. Hold R and press Start. Submenus:

- **Utilities**: warp to any map, fly, cheat start (full party, badges, dex, bike), watch credits, time submenu (set time of day, weekday, rerun daily events), randomiser values, player name/gender/id, follower NPC create/destroy.
- **Scripts**: runs `Debug_EventScript_Script_1`..`8` from `data/scripts/debug.inc`. These are empty slots for your own test scripts. Put `givemon`, `setflag`, `warp`, or a call to your special there.
- **Flags & Vars**: set or clear any flag, edit any var, toggle Pokédex/National dex/PokéNav/match call, set or reset all dex flags.
- **Battle**: start a wild or trainer battle with chosen species, level, AI flags, terrain, weather, battle type.
- **Give**: item, all TMs, Pokémon (simple or with full stats/moves/shiny), max money, coins, battle points, day care egg, fill PC or bag.
- **Party**: heal, clear Pokérus, clear party, set a preset party.
- **Sound**: play any SE or music by id.
- **Berries**, **Trainers** (rematches, Vs Seeker), **Encounters** (mass outbreaks).

Adding a menu entry: the item lists are `sDebugMenu_Items_*[]` arrays of `{ COMPOUND_STRING("Name"), Handler, arg }` and the handlers are `DebugAction_*` functions. Copy a neighbour.

## Battle debug menu

Press Select during a battle (`DEBUG_BATTLE_MENU`). Edit any battler's HP, stats, stat stages, status, ability, held item, moves and PP, and the field's weather and terrain, mid-battle. Useful for reaching an edge case without setting it up.

## Printf debugging

`DebugPrintf("guild points %d, flag %d", VarGet(VAR_X), FlagGet(FLAG_Y));` from any C file (`include/gba/isagbprint.h`, enabled by `LOG_HANDLER_MGBA_PRINT` in `config/general.h`). Output appears in mGBA under Tools → View Logs. Encoded game strings print with `%S` when `PRETTY_PRINT_HANDLER` is `PRETTY_PRINT_MINI_PRINTF` (the default). `DebugPrintfLevel(MGBA_LOG_INFO, ...)` for other levels. Removed automatically in `make release`.

`assertf(condition, "message %d", x)` (`include/assertf.h`) halts with a message in the log and, in expansion, a crash screen naming the assertion. Use it liberally in new code; `StopScript` already asserts when a script exits mid-warp.

The script engine has `DebugPrintScriptStack` and `DebugPrintGlobalScriptStack` macros (`include/script.h`) for "why did my script jump here".

## GDB

`make debug -j$(sysctl -n hw.ncpu)` builds `build/emerald-debug/` and a `pokeemerald.elf` with `-Og -g`. In mGBA: Tools → Start GDB server (port 2345). Then:

```bash
/opt/devkitpro/devkitARM/bin/arm-none-eabi-gdb pokeemerald.elf -ex "target remote :2345"
```

Breakpoints on any function, `print gSaveBlock1Ptr->money ^ gSaveBlock2Ptr->encryptionKey`, `bt` for stack traces, `watch` on a global to find who writes it. `make syms` writes `pokeemerald.sym` which mGBA loads automatically for symbol names in its memory viewer.

## Automated tests

Runner: `make check -j$(sysctl -n hw.ncpu)` builds a test ELF and runs every test headless in `tools/mgba/mgba-rom-test-mac`, in parallel. First run needs `brew install coreutils` on macOS (the doc says optional; the parallel runner uses GNU tools). `make check TESTS="Prefix"` runs tests whose name starts with the prefix. `make pokeemerald-test.elf TESTS="Prefix"` builds an ELF you can open in mGBA to watch the test play out.

The full reference is `docs/tutorials/how_to_testing_system.md`. The shapes:

**Battle test** (`test/battle/...`):

```c
#include "global.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("My Ability raises Speed at end of turn")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_MYABILITY); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        ABILITY_POPUP(player, ABILITY_MYABILITY);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player);
        MESSAGE("Wobbuffet's Speed rose!");
    }
}
```

`GIVEN` builds parties (`Level`, `Item`, `Moves`, `HP`, `Status1`, `Speed`), `FLAG_SET(FLAG_X)` sets a flag, `WHEN` lists turns (`MOVE`, `SWITCH`, `USE_ITEM`, `SEND_OUT`), `SCENE` asserts what the player would see in order (`MESSAGE`, `ANIMATION`, `HP_BAR`, `STATUS_ICON`, `ABILITY_POPUP`, `NOT`, `ONE_OF`), `THEN` checks internal state, `PARAMETRIZE` runs variants, `PASSES_RANDOMLY(n, m, RNG_TAG)` for chance-based effects. `DOUBLE_BATTLE_TEST`, `AI_SINGLE_BATTLE_TEST` with `AI_FLAGS` and `EXPECT_MOVE` for AI behaviour. `ASSUMPTIONS { ASSUME(...); }` at the top documents what the test relies on and skips instead of fails if the assumption is false.

**Non-battle test** (`test/*.c`):

```c
#include "global.h"
#include "test/test.h"
#include "test/overworld_script.h"

TEST("Guild points cap at MAX_GUILD_POINTS")
{
    VarSet(VAR_GUILD_POINTS, MAX_GUILD_POINTS - 1);
    AddGuildPoints(10);
    EXPECT_EQ(VarGet(VAR_GUILD_POINTS), MAX_GUILD_POINTS);
}

TEST("Vendor script deducts points")
{
    VarSet(VAR_GUILD_POINTS, 20);
    RUN_OVERWORLD_SCRIPT(
        setvar VAR_0x8004, 10;
        special Special_TrySpendGuildPoints;
    );
    EXPECT_EQ(VarGet(VAR_GUILD_POINTS), 10);
    EXPECT_EQ(VarGet(VAR_RESULT), TRUE);
}
```

Tests have flags and vars but no map, so script commands that touch object events or the screen (`applymovement`, `msgbox`, `warp`) cannot run; `waitstate` is not supported in `RUN_OVERWORLD_SCRIPT`. Flags and vars used in tests are cleared between tests.

`KNOWN_FAILING;` marks a test that documents a bug you have not fixed.

## Common failure signatures

| Symptom | Likely cause |
|---|---|
| Build error `size of array 'SaveBlock1FreeSpace' is negative` | A save struct exceeded its sector budget. Free space via `config/save.h` or shrink your field |
| Build error about `.rodata` or region `ROM` overflowed | ROM full. Disable families in `species_enabled.h` or cries |
| `region IWRAM overflowed` | Too much `COMMON_DATA` / stack. Move globals to `EWRAM_DATA` |
| Link error `undefined reference to gText_X` | Script text label missing `::` (double colon makes it global) or the `.inc` is not included in `event_scripts.s` |
| Script runs but shows garbage text or freezes on a message | Missing `$` terminator, or `\n`/`\l` misuse making the line too long |
| NPC does nothing on A | `"script": "0x0"` or misspelled label; check `map.json` |
| Game freezes on entering a map | A warp or connection to a map that does not exist, or a script with `waitstate` that nothing re-enables |
| Debug menu warp works but the map is black | Layout has no tileset or a bad `map.bin` path in `layouts.json` |
| Save shows "corrupted" after rebuild | Save struct layout changed. Delete the `.sav` |
| Pokémon appears as a question mark | Species entry missing or `#if P_FAMILY_X` false; sprite pointers missing |
| Move name prints as blank in battle | Move added after `MOVES_COUNT` instead of before it |
| New C function "not found" from a script | Not registered in `data/specials.inc`, or declared `static` |
| Random crash after opening and closing a custom screen a few times | Heap leak: an `Alloc` without `Free`, or windows not freed |
| Colours wrong on a new sprite or tile | Palette index collision; check `paletteNum`/tag, and that the PNG has 16 colours with transparent at index 0 |
| `assertf` crash screen | Read the message; it names file and line |

## Reading upstream when stuck

`docs/changelogs/1.*.x/*.md` list every change per release with PR numbers. When code looks different from a tutorial, the changelog for the relevant version usually explains the rename. The RHH Discord (link in `README.md`) is the community; most questions about the expansion have been answered there.
