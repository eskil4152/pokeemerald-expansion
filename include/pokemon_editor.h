#ifndef GUARD_POKEMON_EDITOR_H
#define GUARD_POKEMON_EDITOR_H

#include "main.h"

// In-place Pokémon editor screen. Opened from the party menu (EDIT) and the
// PC storage mon menu (EDIT). Edits are written to the given Pokémon as they
// are made. returnCallback is installed with SetMainCallback2 on exit.
void ShowPokemonEditor(struct Pokemon *mon, MainCallback returnCallback);
void ShowPokemonEditorForBoxMon(struct BoxPokemon *boxMon, MainCallback returnCallback);

#endif // GUARD_POKEMON_EDITOR_H
