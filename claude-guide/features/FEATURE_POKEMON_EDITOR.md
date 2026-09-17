# Feature: Pokémon editor

Branch: `feature/pokemon-editor` (from master). Status: planned, not started.

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

## Files (planned)

`src/pokemon_editor.c`, `include/pokemon_editor.h`, `src/party_menu.c`, `src/pokemon_storage_system.c`, possibly a shared header extracting the pickers from `src/debug.c`.

## Save impact

None.

## Notes

- High effort: this is the largest UI piece and touches two big vanilla screens.
- Later ties into ascension: awakened stats and extra ability slot become editable fields gated by tier.
