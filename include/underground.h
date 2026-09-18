#ifndef GUARD_UNDERGROUND_H
#define GUARD_UNDERGROUND_H

#include "constants/underground.h"

// Underground economy: second currency (UCOIN), fence, workers, heat.
// State lives in gSaveBlock1Ptr->underground (struct Underground, global.h).

void Underground_ResetSaveData(void);
void Underground_DoDailyEvents(u32 daysSince);

u32 Underground_GetUcoins(void);
void Underground_AddUcoins(u32 amount);
bool32 Underground_TrySpendUcoins(u32 amount);
void Underground_AddHeat(u32 amount);
u32 Underground_GetMonSellValue(struct Pokemon *mon);

// Specials (data/specials.inc). Inputs in VAR_0x8004.., outputs in VAR_RESULT and STR_VAR_n.
void Special_Underground_BufferUcoins(void);
void Special_Underground_AddUcoins(void);
void Special_Underground_TrySpendUcoins(void);
void Special_Underground_GetHeat(void);
void Special_Underground_AddHeat(void);
void Special_Underground_IsLockedOut(void);
void Special_Underground_GetMonSellValue(void);
void Special_Underground_SellPartyMon(void);
void Special_Underground_BuyMon(void);
void Special_Underground_BufferWorkerInfo(void);
void Special_Underground_BuyWorker(void);
void Special_Underground_BufferPendingPayout(void);
void Special_Underground_CollectPayout(void);

#endif // GUARD_UNDERGROUND_H
