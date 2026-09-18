# Feature: Pokémon editor

Branch: `feature/pokemon-editor` (from master). Status: implemented, builds, awaiting manual test.

## Goal

Edit any owned Pokémon in place, from the party menu and from PC boxes: species, level, nature, ability, moves, held item, IVs, EVs, shiny, gender, friendship, nickname. Kept in release builds.

## Player-facing behaviour

- Party menu → select a Pokémon → new option EDIT (next to Summary, Switch, Item).
- PC box → select a Pokémon → new option EDIT.
- Opens a list: each row shows the field and its current value. A opens a picker for that field (number spinner, or a searchable name list for species, moves, items, abilities). B returns. Changes apply immediately and stats are recalculated.

## Design

- The debug menu (`src/debug.c`) already has pickers for species, moves, items, abilities and numeric fields in its "Give Pokémon (complex)" flow, including name search. The editor reuses those selection helpers, pointed at an existing `struct Pokemon` instead of a new one, in a new `src/pokemon_editor.c`.
- Applies through `SetMonData` and `CalculateMonStats`; species change also resets the ability slot if invalid and re-checks form tables.
- Party menu hook: add `MENU_EDIT` to the action list in `src/party_menu.c` (see `sPartyMenuActions` and `CursorCb_*` functions). PC hook: `src/pokemon_storage_system.c` box mon menu.
- Editing a Pokémon in a PC box operates on `struct BoxPokemon`; use `BoxMonToMon` / `CopyMon` round-trip for simplicity.

## Implementation notes

- Own screen in `src/pokemon_editor.c` (own CB2, one background, three windows), not the debug menu's selection framework, which is tied to the overworld task context. Pickers are plain scrolling lists built on the heap from the name tables (species filtered by `IsSpeciesEnabled`, all items, all moves, natures, the species' ability slots, types). L/R page through long lists.
- Works on a private copy and writes back after every change (`*partyMon = mon` or `*boxMon = mon.box`), so leaving at any time keeps the edits.
- Level sets exp from `gExperienceTables`; nature uses the mint field (`MON_DATA_HIDDEN_NATURE`) so the personality is untouched; species change fixes an invalid ability slot and renames a default nickname; moves via `SetMonMoveSlot` so PP is refilled; shiny via `MON_DATA_IS_SHINY`.
- Nickname reuses the naming screen and returns to the editor.
- Gender is not editable (personality-derived); a later pass can regenerate the personality.
- Party hook: `MENU_EDIT` in `src/party_menu.c` (field action list, before CANCEL), table entry in `src/data/party_menu.h`, action array raised to 10. Returns through the summary screen's return path.
- PC hook: `MENU_EDIT` in `src/pokemon_storage_system.c` mon menu, `SCREEN_CHANGE_EDITOR`, `Task_ShowMonEditor`; hidden while a mon is being carried. Returns through `CB2_ReturnToPokeStorage` with the cursor on the edited mon.

## Files

`src/pokemon_editor.c`, `include/pokemon_editor.h`, `src/party_menu.c`, `src/data/party_menu.h`, `src/pokemon_storage_system.c`.

## How to test

1. Party menu → any Pokémon → EDIT. Change level, IVs, EVs, friendship with the spinner (Up/Down 1, Left/Right 10, A confirm, B cancel).
2. Species: pick another; the header and nickname update, stats recalc. Ability slots follow the new species.
3. Moves 1 to 4, held item, nature, tera type, shiny toggle.
4. Nickname: naming screen, then back in the editor.
5. Exit or B: back in the party menu on the same Pokémon; summary reflects changes.
6. PC → a boxed Pokémon → EDIT; same, returns to the box with the cursor on it.
7. Save and reload; edits persist.

## Save impact

None.

## Notes

- High effort: this is the largest UI piece and touches two big vanilla screens.
- Later ties into ascension: awakened stats and extra ability slot become editable fields gated by tier.
