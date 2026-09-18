#include "global.h"
#include "wager.h"
#include "event_data.h"
#include "money.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "script_pokemon_util.h"
#include "string_util.h"
#include "underground.h"
#include "constants/battle.h"

/*
 * Wager battles.
 *
 * State (saved vars, include/constants/vars.h):
 *   VAR_WAGER_MONEY        money the player staked
 *   VAR_WAGER_PLAYER_SLOT  staked party slot + 1, 0 = none
 *   VAR_WAGER_OPP_MONEY    money the opponent put up
 *   VAR_WAGER_OPP_SPECIES  Pokémon the opponent put up, SPECIES_NONE = none
 *   VAR_WAGER_OPP_LEVEL    its level
 * Trigger: any NPC script. The opponent stake is set with setvar, the player
 *   stake through Wager_EventScript_ChooseStake, the battle through the
 *   wagerbattle macro (a no-whiteout trainer battle that returns to the
 *   script with VAR_RESULT = TRUE if the player lost), then Wager_Settle.
 * Logic: below. Presentation: data/scripts/wager.inc.
 */

static const u8 sText_Nothing[] = _("nothing");
static const u8 sText_Yen[] = _("¥");
static const u8 sText_UcoinSuffix[] = _(" UCOIN");

// FLAG_WAGER_IN_UCOIN switches every stake and payout to the underground currency.
static bool32 IsUcoinWager(void)
{
    return FlagGet(FLAG_WAGER_IN_UCOIN);
}

// Writes "¥1234" or "1234 UCOIN" depending on the wager currency.
static void BufferCurrency(u8 *dest, u32 amount)
{
    u8 *end = dest;

    if (!IsUcoinWager())
        end = StringCopy(dest, sText_Yen);
    end = ConvertIntToDecimalStringN(end, amount, STR_CONV_MODE_LEFT_ALIGN, 7);
    if (IsUcoinWager())
        StringCopy(end, sText_UcoinSuffix);
}

static bool32 CanAffordStake(u32 amount)
{
    if (IsUcoinWager())
        return Underground_GetUcoins() >= amount;
    return IsEnoughMoney(&gSaveBlock1Ptr->money, amount);
}

// Clears only the player's side, so a stake the NPC set up beforehand survives.
void Wager_ClearPlayerStake(void)
{
    VarSet(VAR_WAGER_MONEY, 0);
    VarSet(VAR_WAGER_PLAYER_SLOT, 0);
}

void Wager_Clear(void)
{
    VarSet(VAR_WAGER_MONEY, 0);
    VarSet(VAR_WAGER_PLAYER_SLOT, 0);
    VarSet(VAR_WAGER_OPP_MONEY, 0);
    VarSet(VAR_WAGER_OPP_SPECIES, SPECIES_NONE);
    VarSet(VAR_WAGER_OPP_LEVEL, 0);
    FlagClear(FLAG_WAGER_IN_UCOIN);
}

// In: VAR_0x8004 = amount. Out: VAR_RESULT = TRUE if the player can afford it (stake recorded).
void Wager_TrySetMoneyStake(void)
{
    u32 amount = gSpecialVar_0x8004;

    if (amount > WAGER_MAX_MONEY_STAKE || !CanAffordStake(amount))
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    VarSet(VAR_WAGER_MONEY, amount);
    gSpecialVar_Result = TRUE;
}

// In: VAR_0x8004 = party slot from ChoosePartyMon (>= PARTY_SIZE means cancelled).
// Out: VAR_RESULT = WAGER_STAKE_OK / INVALID / CANCELLED. STR_VAR_1 = nickname on success.
void Wager_TryStakePartyMon(void)
{
    u32 slot = gSpecialVar_0x8004;

    if (slot >= PARTY_SIZE)
    {
        gSpecialVar_Result = WAGER_STAKE_CANCELLED;
        return;
    }
    if (CalculatePlayerPartyCount() <= 1 || GetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_IS_EGG))
    {
        gSpecialVar_Result = WAGER_STAKE_INVALID;
        return;
    }
    VarSet(VAR_WAGER_PLAYER_SLOT, slot + 1);
    GetMonData(&gParties[B_TRAINER_PLAYER][slot], MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    gSpecialVar_Result = WAGER_STAKE_OK;
}

// Out: STR_VAR_1 = money staked, STR_VAR_2 = staked Pokémon's nickname or "nothing".
void Wager_BufferStake(void)
{
    u32 slot = VarGet(VAR_WAGER_PLAYER_SLOT);

    BufferCurrency(gStringVar1, VarGet(VAR_WAGER_MONEY));
    if (slot != 0)
    {
        GetMonData(&gParties[B_TRAINER_PLAYER][slot - 1], MON_DATA_NICKNAME, gStringVar2);
        StringGet_Nickname(gStringVar2);
    }
    else
    {
        StringCopy(gStringVar2, sText_Nothing);
    }
}

// In: VAR_0x8004 = TRUE if the player was defeated (VAR_RESULT after wagerbattle).
// Out: VAR_RESULT = WAGER_RESULT_WON / LOST.
//      VAR_0x8005 = Pokémon outcome (see constants/wager.h).
//      VAR_0x8006 = money that changed hands (0 = none).
//      STR_VAR_1 = that money, STR_VAR_2 = the Pokémon involved.
void Wager_Settle(void)
{
    bool32 defeated = gSpecialVar_0x8004;
    u32 slot = VarGet(VAR_WAGER_PLAYER_SLOT);
    u32 money;

    gSpecialVar_0x8005 = WAGER_MON_NONE;

    if (!defeated)
    {
        enum Species species = VarGet(VAR_WAGER_OPP_SPECIES);

        money = VarGet(VAR_WAGER_OPP_MONEY);
        if (IsUcoinWager())
            Underground_AddUcoins(money);
        else
            AddMoney(&gSaveBlock1Ptr->money, money);
        if (species != SPECIES_NONE)
        {
            StringCopy(gStringVar2, GetSpeciesName(species));
            gSpecialVar_0x8005 = ScriptGiveMon(species, VarGet(VAR_WAGER_OPP_LEVEL), ITEM_NONE);
        }
        gSpecialVar_Result = WAGER_RESULT_WON;
    }
    else
    {
        money = VarGet(VAR_WAGER_MONEY);
        if (!IsUcoinWager())
            RemoveMoney(&gSaveBlock1Ptr->money, money);
        else if (!Underground_TrySpendUcoins(money))
            Underground_TrySpendUcoins(Underground_GetUcoins());
        if (slot != 0)
        {
            GetMonData(&gParties[B_TRAINER_PLAYER][slot - 1], MON_DATA_NICKNAME, gStringVar2);
            StringGet_Nickname(gStringVar2);
            ZeroMonData(&gParties[B_TRAINER_PLAYER][slot - 1]);
            CompactPartySlots();
            CalculatePlayerPartyCount();
            gSpecialVar_0x8005 = WAGER_MON_TAKEN;
        }
        gSpecialVar_Result = WAGER_RESULT_LOST;
    }

    gSpecialVar_0x8006 = money;
    BufferCurrency(gStringVar1, money);
    Wager_Clear();
}
