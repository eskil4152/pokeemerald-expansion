#ifndef GUARD_CONSTANTS_WAGER_H
#define GUARD_CONSTANTS_WAGER_H

// Results of Wager_Settle, in VAR_RESULT.
#define WAGER_RESULT_WON    0
#define WAGER_RESULT_LOST   1

// Pokémon outcome of Wager_Settle, in VAR_0x8005.
// On a win the value is MON_GIVEN_TO_PARTY, MON_GIVEN_TO_PC or MON_CANT_GIVE
// (constants/pokemon.h) when the opponent staked a Pokémon.
#define WAGER_MON_NONE      3   // no Pokémon changed hands
#define WAGER_MON_TAKEN     4   // the player's staked Pokémon was taken

// Wager_TryStakePartyMon result in VAR_RESULT.
#define WAGER_STAKE_OK        1
#define WAGER_STAKE_INVALID   0
#define WAGER_STAKE_CANCELLED 2

// Money stakes are stored in a u16 var.
#define WAGER_MAX_MONEY_STAKE 65535

#endif // GUARD_CONSTANTS_WAGER_H
