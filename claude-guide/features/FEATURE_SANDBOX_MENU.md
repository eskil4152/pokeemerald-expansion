# Feature: Sandbox menu

Branch: `feature/sandbox-menu`. Status: implemented, builds, awaiting manual test.

## Goal

A Start menu entry named SANDBOX that opens a list of god-mode style toggles the player can switch on and off at any time in the overworld. Kept in release builds.

## Player-facing behaviour

Start → SANDBOX opens a scrolling list. Each row shows ON or OFF. A toggles the row, B or "Close" leaves. "All ON" and "All OFF" set every toggle at once.

| Row | Effect | How |
|---|---|---|
| No Whiteout | Losing to a trainer does not white out. Party is not auto-healed | `B_FLAG_NO_WHITEOUT` |
| Walk Through Walls | Ignore collision | `OW_FLAG_NO_COLLISION` |
| No Wild Encounters | Grass, water, caves give no random battles | `WE_FLAG_NO_ENCOUNTER` |
| Trainers Blind | Trainers only battle when spoken to | `OW_FLAG_NO_TRAINER_SEE` |
| Full Exp. Share | Whole party gains experience | `I_EXP_SHARE_FLAG` |
| Infinite Money | Money set to the cap when toggled on and again on every step | `src/sandbox.c` |
| Auto-Heal | Party fully healed when toggled on and on every step | `src/sandbox.c` |
| Instant Text | All text prints instantly | `FLAG_TEXT_SPEED_INSTANT` |
| Always Shiny | Every wild and gift Pokémon is shiny | `P_FLAG_FORCE_SHINY` |
| Fly From Map | Press R on a town in the PokéNav map or Town Map to fly there | `OW_FLAG_POKE_RIDER` |

## Design decisions

- One flag per toggle, `FLAG_SANDBOX_*`, renamed from `FLAG_UNUSED_0x020`..`0x029` in `include/constants/flags.h`. Flags are saved, so toggles persist across saves and are editable from the debug menu too.
- Where the expansion already had a config hook that reads a flag, the hook is pointed at the sandbox flag. No engine code was changed for those rows.
- The two rows without a hook (money, heal) are applied on every step through one call in `ProcessPlayerFieldInput`. Per-step is enough: a shop purchase leaves you short only until the next step.
- The menu is modelled on the debug menu's list (`src/debug.c`) but self-contained: static item table, an `itemPrintFunc` that prints ON/OFF, one input task.
- Not gated by `DISABLED_ON_RELEASE`, so it survives `make release`.

## Files

- `include/constants/flags.h`: ten flag renames.
- `include/config/battle.h`, `overworld.h`, `wild_encounter.h`, `item.h`, `text.h`, `pokemon.h`: hook assignments.
- `include/sandbox.h`, `src/sandbox.c`: new.
- `src/start_menu.c`: `MENU_ACTION_SANDBOX`, text, item, callback, added to the normal and debug menu builders.
- `src/field_control_avatar.c`: one call at the top of the took-a-step block.

## Save impact

None. Only previously unused flag bits are used.

## How to test

1. New game (Select on title). Walk outside. Start → SANDBOX.
2. Toggle Walk Through Walls, walk into a house wall.
3. Toggle Infinite Money, open the trainer card or a shop: money is 999,999.
4. Toggle No Wild Encounters, walk in Route 101 grass.
5. Toggle Trainers Blind, walk past a trainer on Route 102.
6. Toggle No Whiteout, lose a trainer battle on purpose.
7. Toggle Instant Text, talk to anyone.
8. Save, reset, reopen the menu: toggles are still set.

## Follow-ups

- The followers branch adds a "Followers" row to this menu.
- Later, this menu is planned to become the ascension power tree, where rows unlock instead of being free.
