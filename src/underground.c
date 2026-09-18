#include "global.h"
#include "underground.h"
#include "event_data.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "script_pokemon_util.h"
#include "string_util.h"
#include "constants/items.h"

/*
 * Underground economy.
 *
 * State: gSaveBlock1Ptr->underground (struct Underground in global.h):
 *   ucoins        the second currency (placeholder name UCOIN)
 *   pendingPayout worker income waiting to be collected from the broker
 *   heat          0..100, rises with illegal acts, decays daily
 *   workers[]     owned count per worker type
 * Trigger: NPC scripts in data/scripts/underground.inc; daily hook from
 *   DoDailyEvents (src/clock.c); reset from NewGameInitData.
 * Logic: below. Presentation: the scripts.
 */

struct WorkerInfo
{
    const u8 *name;
    u16 price;
    u16 dailyYield;
};

static const struct WorkerInfo sWorkerInfo[NUM_WORKER_TYPES] =
{
    [WORKER_PICKPOCKET] = { COMPOUND_STRING("Pickpocket"), 200,  20 },
    [WORKER_SMUGGLER]   = { COMPOUND_STRING("Smuggler"),   600,  70 },
    [WORKER_FENCE]      = { COMPOUND_STRING("Fence"),      1500, 200 },
};

#define UG (gSaveBlock1Ptr->underground)

void Underground_ResetSaveData(void)
{
    memset(&UG, 0, sizeof(UG));
}

u32 Underground_GetUcoins(void)
{
    return UG.ucoins;
}

void Underground_AddUcoins(u32 amount)
{
    u32 total = UG.ucoins + amount;

    if (total > UCOIN_MAX || total < UG.ucoins)
        total = UCOIN_MAX;
    UG.ucoins = total;
}

bool32 Underground_TrySpendUcoins(u32 amount)
{
    if (UG.ucoins < amount)
        return FALSE;
    UG.ucoins -= amount;
    return TRUE;
}

void Underground_AddHeat(u32 amount)
{
    u32 heat = UG.heat + amount;

    if (heat > HEAT_MAX)
        heat = HEAT_MAX;
    UG.heat = heat;
}

static bool32 IsLockedOut(void)
{
    return UG.heat >= HEAT_LOCKOUT_THRESHOLD;
}

static u32 GetDailyIncome(void)
{
    u32 i, income = 0;

    for (i = 0; i < NUM_WORKER_TYPES; i++)
        income += UG.workers[i] * sWorkerInfo[i].dailyYield;
    return income;
}

// Called once per day change with the number of days elapsed.
void Underground_DoDailyEvents(u32 daysSince)
{
    u32 income = GetDailyIncome() * daysSince;
    u32 pending = UG.pendingPayout + income;
    u32 decay = HEAT_DECAY_PER_DAY * daysSince;

    if (pending > UCOIN_MAX || pending < UG.pendingPayout)
        pending = UCOIN_MAX;
    UG.pendingPayout = pending;

    if (UG.heat > decay)
        UG.heat -= decay;
    else
        UG.heat = 0;
}

// Base stat total scaled by level; legendaries and mythicals pay triple.
u32 Underground_GetMonSellValue(struct Pokemon *mon)
{
    enum Species species = GetMonData(mon, MON_DATA_SPECIES);
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    u32 bst = info->baseHP + info->baseAttack + info->baseDefense
            + info->baseSpeed + info->baseSpAttack + info->baseSpDefense;
    u32 value = bst * GetMonData(mon, MON_DATA_LEVEL) / 10;

    if (info->isRestrictedLegendary || info->isSubLegendary || info->isMythical)
        value *= 3;
    if (GetMonData(mon, MON_DATA_IS_SHINY))
        value *= 2;
    if (value < 10)
        value = 10;
    return value;
}

static void BufferNumber(u8 *dest, u32 value)
{
    ConvertIntToDecimalStringN(dest, value, STR_CONV_MODE_LEFT_ALIGN, 6);
}

static u32 ClampToVar(u32 value)
{
    return value > UCOIN_VAR_CAP ? UCOIN_VAR_CAP : value;
}

// ---- Specials ----

// Out: STR_VAR_1 = balance.
void Special_Underground_BufferUcoins(void)
{
    BufferNumber(gStringVar1, UG.ucoins);
}

// In: VAR_0x8004 = amount.
void Special_Underground_AddUcoins(void)
{
    Underground_AddUcoins(gSpecialVar_0x8004);
}

// In: VAR_0x8004 = amount. Out: VAR_RESULT = TRUE if paid.
void Special_Underground_TrySpendUcoins(void)
{
    gSpecialVar_Result = Underground_TrySpendUcoins(gSpecialVar_0x8004);
}

// Out: VAR_RESULT = heat, STR_VAR_1 = heat.
void Special_Underground_GetHeat(void)
{
    gSpecialVar_Result = UG.heat;
    BufferNumber(gStringVar1, UG.heat);
}

// In: VAR_0x8004 = amount.
void Special_Underground_AddHeat(void)
{
    Underground_AddHeat(gSpecialVar_0x8004);
}

// Out: VAR_RESULT = TRUE if the underground refuses to deal right now.
void Special_Underground_IsLockedOut(void)
{
    gSpecialVar_Result = IsLockedOut();
}

// In: VAR_0x8004 = party slot. Out: VAR_RESULT = value (0 = cannot sell),
//     STR_VAR_1 = nickname, STR_VAR_2 = value.
void Special_Underground_GetMonSellValue(void)
{
    u32 slot = gSpecialVar_0x8004;
    struct Pokemon *mon;

    gSpecialVar_Result = 0;
    if (slot >= PARTY_SIZE)
        return;
    mon = &gParties[B_TRAINER_PLAYER][slot];
    if (CalculatePlayerPartyCount() <= 1 || GetMonData(mon, MON_DATA_IS_EGG))
        return;

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    BufferNumber(gStringVar2, Underground_GetMonSellValue(mon));
    gSpecialVar_Result = ClampToVar(Underground_GetMonSellValue(mon));
}

// In: VAR_0x8004 = party slot (validated by the special above). Removes the
// Pokémon, pays its value, raises heat. Out: STR_VAR_2 = value paid.
void Special_Underground_SellPartyMon(void)
{
    u32 slot = gSpecialVar_0x8004;
    struct Pokemon *mon;
    u32 value;

    if (slot >= PARTY_SIZE)
        return;
    mon = &gParties[B_TRAINER_PLAYER][slot];
    value = Underground_GetMonSellValue(mon);
    Underground_AddUcoins(value);
    Underground_AddHeat(HEAT_SELL_MON);
    BufferNumber(gStringVar2, value);
    ZeroMonData(mon);
    CompactPartySlots();
    CalculatePlayerPartyCount();
}

// In: VAR_0x8004 = species, VAR_0x8005 = level, VAR_0x8006 = price.
// Out: VAR_RESULT = MON_GIVEN_TO_PARTY / MON_GIVEN_TO_PC / MON_CANT_GIVE / UNDERGROUND_CANT_AFFORD.
//      STR_VAR_1 = species name.
void Special_Underground_BuyMon(void)
{
    enum Species species = gSpecialVar_0x8004;
    u32 level = gSpecialVar_0x8005;
    u32 price = gSpecialVar_0x8006;
    u32 result;

    StringCopy(gStringVar1, GetSpeciesName(species));
    if (UG.ucoins < price)
    {
        gSpecialVar_Result = UNDERGROUND_CANT_AFFORD;
        return;
    }
    result = ScriptGiveMon(species, level, ITEM_NONE);
    if (result != MON_CANT_GIVE)
    {
        Underground_TrySpendUcoins(price);
        Underground_AddHeat(HEAT_BUY_MON);
    }
    gSpecialVar_Result = result;
}

// In: VAR_0x8004 = worker type. Out: STR_VAR_1 = name, STR_VAR_2 = price,
//     STR_VAR_3 = owned count, VAR_RESULT = daily yield.
void Special_Underground_BufferWorkerInfo(void)
{
    u32 type = gSpecialVar_0x8004;

    if (type >= NUM_WORKER_TYPES)
    {
        gSpecialVar_Result = 0;
        return;
    }
    StringCopy(gStringVar1, sWorkerInfo[type].name);
    BufferNumber(gStringVar2, sWorkerInfo[type].price);
    BufferNumber(gStringVar3, UG.workers[type]);
    gSpecialVar_Result = sWorkerInfo[type].dailyYield;
}

// In: VAR_0x8004 = worker type. Out: VAR_RESULT = TRUE if bought.
void Special_Underground_BuyWorker(void)
{
    u32 type = gSpecialVar_0x8004;

    gSpecialVar_Result = FALSE;
    if (type >= NUM_WORKER_TYPES || UG.workers[type] >= MAX_WORKERS_PER_TYPE)
        return;
    if (!Underground_TrySpendUcoins(sWorkerInfo[type].price))
        return;
    UG.workers[type]++;
    Underground_AddHeat(HEAT_BUY_WORKER);
    gSpecialVar_Result = TRUE;
}

// Out: STR_VAR_1 = pending payout, STR_VAR_2 = income per day, VAR_RESULT = pending (capped).
void Special_Underground_BufferPendingPayout(void)
{
    BufferNumber(gStringVar1, UG.pendingPayout);
    BufferNumber(gStringVar2, GetDailyIncome());
    gSpecialVar_Result = ClampToVar(UG.pendingPayout);
}

// Moves the pending payout into the balance. Out: STR_VAR_1 = amount collected.
void Special_Underground_CollectPayout(void)
{
    u32 amount = UG.pendingPayout;

    BufferNumber(gStringVar1, amount);
    Underground_AddUcoins(amount);
    UG.pendingPayout = 0;
    if (amount > 0)
        Underground_AddHeat(HEAT_COLLECT_PAYOUT);
    gSpecialVar_Result = ClampToVar(amount);
}
