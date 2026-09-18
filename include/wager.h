#ifndef GUARD_WAGER_H
#define GUARD_WAGER_H

#include "constants/wager.h"

// Wager battles: stake money and/or a party Pokémon against an NPC's stake.
// All functions are specials called from data/scripts/wager.inc.
void Wager_Clear(void);
void Wager_ClearPlayerStake(void);
void Wager_TrySetMoneyStake(void);
void Wager_TryStakePartyMon(void);
void Wager_BufferStake(void);
void Wager_Settle(void);

#endif // GUARD_WAGER_H
