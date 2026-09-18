#include "global.h"
#include "arena.h"
#include "event_data.h"
#include "string_util.h"
#include "underground.h"
#include "wager.h"

/*
 * Underground arena ladder.
 *
 * State: VAR_ARENA_TIER = highest table beaten (0..ARENA_TIER_COUNT).
 *        FLAG_ARENA_ACCESS = buy-in paid to the fence.
 *        FLAG_UNDERGROUND_TRUSTED = at least one deal done with the fence.
 * Trigger: the fence in Littleroot (data/scripts/underground.inc) and the
 *   bookies in data/maps/UndergroundArena/scripts.inc.
 * Fights are wager battles (src/wager.c) with FLAG_WAGER_IN_UCOIN set, so
 *   stakes are paid in UCOIN.
 */

// In: VAR_0x8008 = table (1-based). Out: VAR_RESULT = TRUE if the player may play it.
void Special_Arena_IsTierOpen(void)
{
    gSpecialVar_Result = VarGet(VAR_ARENA_TIER) + 1 >= gSpecialVar_0x8008;
}

// In: VAR_0x8004 = wager result (WAGER_RESULT_*), VAR_0x8008 = table.
// Adds heat for the fight. Out: VAR_RESULT = TRUE if this win opened a new table.
void Special_Arena_RecordResult(void)
{
    u32 tier = gSpecialVar_0x8008;

    Underground_AddHeat(ARENA_HEAT_PER_FIGHT);
    gSpecialVar_Result = FALSE;
    if (gSpecialVar_0x8004 == WAGER_RESULT_WON && VarGet(VAR_ARENA_TIER) < tier)
    {
        VarSet(VAR_ARENA_TIER, tier);
        gSpecialVar_Result = TRUE;
    }
}

// Out: STR_VAR_1 = tables beaten, STR_VAR_2 = UCOIN balance, STR_VAR_3 = table count.
void Special_Arena_BufferStatus(void)
{
    ConvertIntToDecimalStringN(gStringVar1, VarGet(VAR_ARENA_TIER), STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, Underground_GetUcoins(), STR_CONV_MODE_LEFT_ALIGN, 7);
    ConvertIntToDecimalStringN(gStringVar3, ARENA_TIER_COUNT, STR_CONV_MODE_LEFT_ALIGN, 2);
}
