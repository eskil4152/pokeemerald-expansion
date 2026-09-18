#include "global.h"
#include "ascension.h"
#include "arena.h"
#include "battle.h"
#include "data.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "item.h"
#include "pokemon.h"
#include "quest.h"
#include "random.h"
#include "script_pokemon_util.h"
#include "string_util.h"
#include "constants/arena.h"
#include "constants/battle.h"
#include "constants/difficulty.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/opponents.h"
#include "constants/sound.h"
#include "constants/species.h"

/*
 * Ascension, phase 1: permanent Pokémon tiers.
 *
 * State: each Pokémon stores its tier (MON_DATA_ASCENSION_TIER, 3 bits) and a
 *   win counter (MON_DATA_ASCENSION_WINS, 6 bits) in previously unused bits of
 *   struct PokemonSubstruct0. Existing Pokémon read as tier 0, so saves stay valid.
 * Effects: a stat bonus applied in CalculateMonStatsCont (src/pokemon.c), an
 *   ascended cry from ASCENSION_CRY_MIN_TIER (src/sound.c, src/pokeball.c,
 *   src/pokemon_summary_screen.c), and a star mark on the summary screen.
 * Tuning: src/data/ascension.h.
 *
 * Phases 2-5: the Shrine, ranks, reign and the world reacting.
 * State: VAR_ASCENSION_RANK (0..5), VAR_ASCENSION_REIGN_DAYS,
 *   VAR_ASCENSION_VISITOR / _CHALLENGE_DAYS / _NEXT_VISITOR, VAR_ASCENSION_BOUNTY_DAY,
 *   FLAG_ASCENSION_* (include/constants/flags.h). ITEM_ASCENSION_SHARD in the bag.
 * Triggers: the mystic in the League lobby and the hub, the Oracle at the Shrine
 *   (data/scripts/ascension.inc, data/maps/AscensionShrine), the visitor object in
 *   the League lobby, and engine hooks:
 *   - Ascension_OnTrainerPartyCreated from CreateNPCTrainerParty (battle_setup.c):
 *     post-champion level scaling and tiers for visiting champions.
 *   - Ascension_OnBattleFinished from HandleEndTurn_FinishBattle (battle_main.c):
 *     per-Pokémon trainer win counter.
 *   - Ascension_IsGuardianBattle in CB2_EndScriptedWildBattle: no whiteout.
 *   - Ascension_DoDailyEvents from DoDailyEvents (clock.c): reign and challengers.
 */

#include "data/ascension.h"

static const u8 sText_TierStar[] = _("{STAR}");
static const u8 *const sRankNames[ASCENSION_MAX_RANK + 1] =
{
    COMPOUND_STRING("Champion"),
    COMPOUND_STRING("Rank I"),
    COMPOUND_STRING("Rank II"),
    COMPOUND_STRING("Rank III"),
    COMPOUND_STRING("Rank IV"),
    COMPOUND_STRING("Rank V"),
};

// The guardian battle is fought by one Pokémon alone; the rest of the party
// waits here until Special_Ascension_FinishGuardianBattle puts it back.
static EWRAM_DATA struct Pokemon sPartyBackup[PARTY_SIZE] = {0};
static EWRAM_DATA bool8 sInGuardianBattle = FALSE;
static EWRAM_DATA u8 sChallengerSlot = 0;

enum TrainerScaling
{
    SCALING_NONE,
    SCALING_REGULAR,
    SCALING_ELITE,
    SCALING_VISITOR,
    SCALING_ASCENDED,
};

u32 Ascension_GetMonTier(struct Pokemon *mon)
{
    u32 tier = GetMonData(mon, MON_DATA_ASCENSION_TIER);
    return tier > ASCENSION_MAX_TIER ? ASCENSION_MAX_TIER : tier;
}

u32 Ascension_ApplyStatBonus(u32 tier, u32 stat)
{
    if (tier == 0)
        return stat;
    if (tier > ASCENSION_MAX_TIER)
        tier = ASCENSION_MAX_TIER;
    return stat + (stat * gAscensionSteps[tier].statBonusPercent) / 100;
}

// Only the normal, healthy cry changes; weak, fainting and double-battle cries keep their mode.
u32 Ascension_GetCryMode(struct Pokemon *mon, u32 defaultMode)
{
    if (defaultMode == CRY_MODE_NORMAL && Ascension_GetMonTier(mon) >= ASCENSION_CRY_MIN_TIER)
        return CRY_MODE_ASCENDED;
    return defaultMode;
}

// Appends "★N" to dest if the Pokémon is ascended.
void Ascension_AppendTierMark(struct Pokemon *mon, u8 *dest)
{
    u32 tier = Ascension_GetMonTier(mon);
    u8 *end;

    if (tier == 0)
        return;
    end = StringAppend(dest, sText_TierStar);
    ConvertIntToDecimalStringN(end, tier, STR_CONV_MODE_LEFT_ALIGN, 1);
}

u32 Ascension_GetRank(void)
{
    u32 rank = VarGet(VAR_ASCENSION_RANK);
    return rank > ASCENSION_MAX_RANK ? ASCENSION_MAX_RANK : rank;
}

static void SetMonTier(struct Pokemon *mon, u32 tier)
{
    SetMonData(mon, MON_DATA_ASCENSION_TIER, &tier);
    CalculateMonStats(mon);
}

static void SetMonLevel(struct Pokemon *mon, u32 level)
{
    u32 species = GetMonData(mon, MON_DATA_SPECIES);
    u32 exp = gExperienceTables[gSpeciesInfo[species].growthRate][level];

    SetMonData(mon, MON_DATA_EXP, &exp);
    CalculateMonStats(mon);
}

static u32 GetPlayerAverageLevel(void)
{
    u32 i, total = 0, count = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        total += GetMonData(mon, MON_DATA_LEVEL);
        count++;
    }
    return count ? total / count : 1;
}

static u32 GetVisitorIdForTrainer(u16 trainerId)
{
    u32 i;

    for (i = ASCENSION_VISITOR_NONE + 1; i < ASCENSION_VISITOR_COUNT; i++)
    {
        if (gAscensionVisitors[i].trainerId == trainerId)
            return i;
    }
    return ASCENSION_VISITOR_NONE;
}

static enum TrainerScaling GetTrainerScaling(u16 trainerId)
{
    if (trainerId == TRAINER_ASCENDED_ONE)
        return SCALING_ASCENDED;
    if (GetVisitorIdForTrainer(trainerId) != ASCENSION_VISITOR_NONE)
        return SCALING_VISITOR;
    if (trainerId == TRAINER_NONE || trainerId >= TRAINERS_COUNT || trainerId == TRAINER_WAGER_DEMO
     || (trainerId >= TRAINER_ARENA_1 && trainerId <= TRAINER_ARENA_5))
        return SCALING_NONE;
    if (!FlagGet(FLAG_IS_CHAMPION))
        return SCALING_NONE;
    if ((trainerId >= TRAINER_SIDNEY && trainerId <= TRAINER_DRAKE) || trainerId == TRAINER_WALLACE)
        return SCALING_ELITE;
    return SCALING_REGULAR;
}

// Called at the end of CreateNPCTrainerParty. Raises levels after the player is
// Champion and awakens visiting champions, the Ascended One and the arena boss.
void Ascension_OnTrainerPartyCreated(struct Pokemon *party, u16 trainerId)
{
    u32 i, rank = Ascension_GetRank();
    u32 average, target, tier = 0;
    enum TrainerScaling scaling = GetTrainerScaling(trainerId);

    if (trainerId == TRAINER_ARENA_5 && FlagGet(FLAG_IS_CHAMPION))
        tier = gAscensionRanks[rank].npcMaxTier;
    if (scaling == SCALING_NONE && tier == 0)
        return;

    average = GetPlayerAverageLevel();
    switch (scaling)
    {
    case SCALING_REGULAR:
        target = average > ASCENSION_SCALE_MARGIN ? average - ASCENSION_SCALE_MARGIN : 1;
        break;
    case SCALING_ELITE:
        target = average + ASCENSION_ELITE_BONUS;
        break;
    case SCALING_VISITOR:
        target = average + ASCENSION_VISITOR_BONUS;
        tier = min(gAscensionVisitors[GetVisitorIdForTrainer(trainerId)].maxTier, gAscensionRanks[rank].npcMaxTier);
        break;
    case SCALING_ASCENDED:
        target = average + ASCENSION_ASCENDED_BONUS;
        tier = ASCENDED_ONE_TIER;
        break;
    default:
        target = 0;
        break;
    }
    if (scaling != SCALING_NONE && target < gAscensionRanks[rank].trainerLevelFloor)
        target = gAscensionRanks[rank].trainerLevelFloor;
    if (target > MAX_LEVEL)
        target = MAX_LEVEL;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &party[i];

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
            break;
        if (GetMonData(mon, MON_DATA_LEVEL) < target)
            SetMonLevel(mon, target);
        if (tier != 0)
            SetMonTier(mon, tier);
    }
}

// Called from HandleEndTurn_FinishBattle while battle state still exists.
// Every Pokémon the player sent out in a won trainer battle gains a win.
void Ascension_OnBattleFinished(void)
{
    u32 i;

    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER) || gBattleOutcome != B_OUTCOME_WON)
        return;
    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK | BATTLE_TYPE_RECORDED | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_TRAINER_HILL))
        return;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 wins;

        if (!gBattleStruct->partyState[B_TRAINER_PLAYER][i].sentOut)
            continue;
        wins = GetMonData(mon, MON_DATA_ASCENSION_WINS);
        if (wins < ASCENSION_MAX_WINS)
        {
            wins++;
            SetMonData(mon, MON_DATA_ASCENSION_WINS, &wins);
        }
    }
}

// Reign: counts days as Champion, sends a visiting champion every
// ASCENSION_CHALLENGE_INTERVAL days, and takes the title if a challenge goes
// unanswered for ASCENSION_CHALLENGE_DEADLINE days.
void Ascension_DoDailyEvents(u32 daysSince)
{
    u32 day;

    if (daysSince > ASCENSION_CHALLENGE_INTERVAL * 2)
        daysSince = ASCENSION_CHALLENGE_INTERVAL * 2;

    for (day = 0; day < daysSince; day++)
    {
        u32 reign;

        if (!FlagGet(FLAG_IS_CHAMPION) || FlagGet(FLAG_ASCENSION_DETHRONED))
            return;

        reign = VarGet(VAR_ASCENSION_REIGN_DAYS);
        if (reign < 0xFFFF)
            VarSet(VAR_ASCENSION_REIGN_DAYS, ++reign);

        if (FlagGet(FLAG_ASCENSION_CHALLENGE_PENDING))
        {
            u32 waited = VarGet(VAR_ASCENSION_CHALLENGE_DAYS) + 1;

            VarSet(VAR_ASCENSION_CHALLENGE_DAYS, waited);
            if (waited >= ASCENSION_CHALLENGE_DEADLINE)
            {
                FlagClear(FLAG_ASCENSION_CHALLENGE_PENDING);
                FlagSet(FLAG_ASCENSION_DETHRONED);
            }
        }
        else if (reign % ASCENSION_CHALLENGE_INTERVAL == 0)
        {
            u32 next = VarGet(VAR_ASCENSION_NEXT_VISITOR) % (ASCENSION_VISITOR_COUNT - 1);

            VarSet(VAR_ASCENSION_VISITOR, next + 1);
            VarSet(VAR_ASCENSION_NEXT_VISITOR, next + 1);
            VarSet(VAR_ASCENSION_CHALLENGE_DAYS, 0);
            FlagSet(FLAG_ASCENSION_CHALLENGE_PENDING);
        }
    }
}

bool32 Ascension_IsGuardianBattle(void)
{
    return sInGuardianBattle;
}

// In: VAR_0x8004 = party slot. Out: VAR_RESULT = ASC_REQ_*.
// Buffers: STR_VAR_1 = nickname, STR_VAR_2 = the missing amount or requirement,
// STR_VAR_3 = target tier. On ASC_REQ_OK, STR_VAR_2 = guardian and VAR_0x8005 = shard cost.
void Special_Ascension_CheckRequirements(void)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gSpecialVar_0x8004];
    u32 tier, target;
    const struct AscensionStep *step;

    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    if (GetMonData(mon, MON_DATA_IS_EGG))
    {
        gSpecialVar_Result = ASC_REQ_EGG;
        return;
    }
    tier = Ascension_GetMonTier(mon);
    if (tier >= ASCENSION_MAX_TIER)
    {
        gSpecialVar_Result = ASC_REQ_MAXED;
        return;
    }
    target = tier + 1;
    step = &gAscensionSteps[target];
    ConvertIntToDecimalStringN(gStringVar3, target, STR_CONV_MODE_LEFT_ALIGN, 1);

    if (Ascension_GetRank() < step->requiredRank)
    {
        StringCopy(gStringVar2, sRankNames[step->requiredRank]);
        gSpecialVar_Result = ASC_REQ_RANK;
    }
    else if (GetMonData(mon, MON_DATA_LEVEL) < step->minLevel)
    {
        ConvertIntToDecimalStringN(gStringVar2, step->minLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
        gSpecialVar_Result = ASC_REQ_LEVEL;
    }
    else if (GetMonData(mon, MON_DATA_FRIENDSHIP) < step->minFriendship)
    {
        gSpecialVar_Result = ASC_REQ_FRIENDSHIP;
    }
    else if (GetMonData(mon, MON_DATA_ASCENSION_WINS) < step->requiredWins)
    {
        u32 missing = step->requiredWins - GetMonData(mon, MON_DATA_ASCENSION_WINS);
        ConvertIntToDecimalStringN(gStringVar2, missing, STR_CONV_MODE_LEFT_ALIGN, 2);
        gSpecialVar_Result = ASC_REQ_WINS;
    }
    else if (!CheckBagHasItem(ITEM_ASCENSION_SHARD, step->shardCost))
    {
        ConvertIntToDecimalStringN(gStringVar2, step->shardCost, STR_CONV_MODE_LEFT_ALIGN, 1);
        gSpecialVar_Result = ASC_REQ_SHARDS;
    }
    else
    {
        StringCopy(gStringVar2, GetSpeciesName(step->guardian));
        gSpecialVar_0x8005 = step->shardCost;
        gSpecialVar_Result = ASC_REQ_OK;
    }
}

// In: VAR_0x8004 = party slot, requirements already checked.
// Leaves only the challenger in the party and replaces the wild mon that
// setwildbattle created with the guardian. The script then runs dowildbattle.
void Special_Ascension_StartGuardianBattle(void)
{
    struct Pokemon *guardian = &gParties[B_TRAINER_OPPONENT_A][0];
    struct Pokemon *challenger;
    u32 i, target, iv = MAX_PER_STAT_IVS;
    s32 level;
    const struct AscensionStep *step;

    sChallengerSlot = gSpecialVar_0x8004;
    for (i = 0; i < PARTY_SIZE; i++)
        sPartyBackup[i] = gParties[B_TRAINER_PLAYER][i];
    challenger = &sPartyBackup[sChallengerSlot];

    gParties[B_TRAINER_PLAYER][0] = *challenger;
    for (i = 1; i < PARTY_SIZE; i++)
        ZeroMonData(&gParties[B_TRAINER_PLAYER][i]);
    CalculatePlayerPartyCount();

    target = Ascension_GetMonTier(challenger) + 1;
    step = &gAscensionSteps[target];
    level = (s32)GetMonData(challenger, MON_DATA_LEVEL) + step->guardianLevelOffset;
    if (level < 1)
        level = 1;
    if (level > MAX_LEVEL)
        level = MAX_LEVEL;

    CreateScriptedWildMon(step->guardian, level, ITEM_NONE);
    for (i = 0; i < NUM_STATS; i++)
        SetMonData(guardian, MON_DATA_HP_IV + i, &iv);
    SetMonTier(guardian, target);
    i = GetMonData(guardian, MON_DATA_MAX_HP);
    SetMonData(guardian, MON_DATA_HP, &i);

    VarSet(VAR_ASCENSION_NO_BAG, NO_BAG_IN_BATTLE);
    sInGuardianBattle = TRUE;
}

// Puts the party back with the challenger in its old slot.
// Out: VAR_RESULT = TRUE if the guardian was defeated; then the shards are
// spent and the challenger ascends. STR_VAR_1 = nickname, STR_VAR_2 = new tier.
void Special_Ascension_FinishGuardianBattle(void)
{
    struct Pokemon *challenger;
    u32 i, tier, wins = 0;
    bool32 won = (gBattleOutcome == B_OUTCOME_WON);

    sPartyBackup[sChallengerSlot] = gParties[B_TRAINER_PLAYER][0];
    for (i = 0; i < PARTY_SIZE; i++)
        gParties[B_TRAINER_PLAYER][i] = sPartyBackup[i];
    CalculatePlayerPartyCount();
    VarSet(VAR_ASCENSION_NO_BAG, NO_BAG_RESTRICTION);
    sInGuardianBattle = FALSE;

    challenger = &gParties[B_TRAINER_PLAYER][sChallengerSlot];
    GetMonData(challenger, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    gSpecialVar_Result = won;
    if (won)
    {
        tier = Ascension_GetMonTier(challenger) + 1;
        RemoveBagItem(ITEM_ASCENSION_SHARD, gAscensionSteps[tier].shardCost);
        SetMonData(challenger, MON_DATA_ASCENSION_WINS, &wins);
        SetMonTier(challenger, tier);
        ConvertIntToDecimalStringN(gStringVar2, tier, STR_CONV_MODE_LEFT_ALIGN, 1);
    }
    UpdateFollowingPokemon();
}

// Out: VAR_RESULT = ASC_TRIAL_*, VAR_0x8005 = current rank.
void Special_Ascension_GetTrialState(void)
{
    u32 rank = Ascension_GetRank();

    gSpecialVar_0x8005 = rank;
    if (rank >= ASCENSION_MAX_RANK)
        gSpecialVar_Result = ASC_TRIAL_ALL_DONE;
    else if (FlagGet(FLAG_ASCENSION_DETHRONED))
        gSpecialVar_Result = ASC_TRIAL_DETHRONED;
    else if (rank == 0)
        gSpecialVar_Result = VarGet(VAR_ASCENSION_REIGN_DAYS) >= ASCENSION_TRIAL_I_DAYS ? ASC_TRIAL_READY : ASC_TRIAL_NOT_READY;
    else if (rank == 1)
        gSpecialVar_Result = VarGet(VAR_ARENA_TIER) >= ARENA_TIER_COUNT ? ASC_TRIAL_READY : ASC_TRIAL_NOT_READY;
    else if (rank == 2)
        gSpecialVar_Result = FlagGet(FLAG_ASCENSION_BEAT_VISITOR) ? ASC_TRIAL_READY : ASC_TRIAL_NOT_READY;
    else
        gSpecialVar_Result = ASC_TRIAL_BATTLE;
}

// Raises the rank by one and moves the ascension quests along.
// Out: STR_VAR_1 = new rank name. The script gives the shards.
void Special_Ascension_AdvanceRank(void)
{
    u32 rank = Ascension_GetRank();

    if (rank >= ASCENSION_MAX_RANK)
        return;
    Quest_Complete(QUEST_ASCENSION_I + rank);
    rank++;
    VarSet(VAR_ASCENSION_RANK, rank);
    if (rank < ASCENSION_MAX_RANK)
        Quest_Start(QUEST_ASCENSION_I + rank);
    StringCopy(gStringVar1, sRankNames[rank]);
}

// Out: STR_VAR_1 = rank name, STR_VAR_2 = days reigned, STR_VAR_3 = shards held.
void Special_Ascension_BufferStatus(void)
{
    StringCopy(gStringVar1, sRankNames[Ascension_GetRank()]);
    ConvertIntToDecimalStringN(gStringVar2, VarGet(VAR_ASCENSION_REIGN_DAYS), STR_CONV_MODE_LEFT_ALIGN, 5);
    ConvertIntToDecimalStringN(gStringVar3, CountTotalItemQuantityInBag(ITEM_ASCENSION_SHARD), STR_CONV_MODE_LEFT_ALIGN, 3);
}

// League lobby on-transition: shows the visiting champion while a challenge
// is pending or while they hold the title.
void Special_Ascension_SetupLeagueVisitor(void)
{
    u32 visitor = VarGet(VAR_ASCENSION_VISITOR);

    if (visitor >= ASCENSION_VISITOR_COUNT)
        visitor = ASCENSION_VISITOR_NONE;
    if (visitor != ASCENSION_VISITOR_NONE
     && (FlagGet(FLAG_ASCENSION_CHALLENGE_PENDING) || FlagGet(FLAG_ASCENSION_DETHRONED)))
    {
        VarSet(VAR_OBJ_GFX_ID_0, gAscensionVisitors[visitor].graphicsId);
        FlagClear(FLAG_HIDE_ASCENSION_VISITOR);
    }
    else
    {
        FlagSet(FLAG_HIDE_ASCENSION_VISITOR);
    }
}

// Out: VAR_RESULT = visitor (ASCENSION_VISITOR_*), STR_VAR_1 = their name.
void Special_Ascension_BufferVisitor(void)
{
    u32 visitor = VarGet(VAR_ASCENSION_VISITOR);

    if (visitor >= ASCENSION_VISITOR_COUNT)
        visitor = ASCENSION_VISITOR_NONE;
    gSpecialVar_Result = visitor;
    if (visitor != ASCENSION_VISITOR_NONE)
        StringCopy(gStringVar1, GetTrainerNameFromId(gAscensionVisitors[visitor].trainerId));
}

// In: VAR_0x8004 = TRUE if the player lost (VAR_RESULT from the battle).
// A loss takes the title; a win answers the challenge or reclaims the title.
void Special_Ascension_OnVisitorBattle(void)
{
    if (gSpecialVar_0x8004)
    {
        FlagClear(FLAG_ASCENSION_CHALLENGE_PENDING);
        FlagSet(FLAG_ASCENSION_DETHRONED);
        return;
    }
    FlagClear(FLAG_ASCENSION_CHALLENGE_PENDING);
    FlagClear(FLAG_ASCENSION_DETHRONED);
    FlagSet(FLAG_ASCENSION_BEAT_VISITOR);
    FlagSet(FLAG_HIDE_ASCENSION_VISITOR);
    VarSet(VAR_ASCENSION_VISITOR, ASCENSION_VISITOR_NONE);
    VarSet(VAR_ASCENSION_CHALLENGE_DAYS, 0);
}

// Out: VAR_RESULT = TRUE if a weekly bounty is due; marks it paid.
void Special_Ascension_TryWeeklyBounty(void)
{
    u32 today = VarGet(VAR_DAYS);
    u32 last = VarGet(VAR_ASCENSION_BOUNTY_DAY);

    gSpecialVar_Result = FALSE;
    if (!FlagGet(FLAG_IS_CHAMPION))
        return;
    if (last != 0 && today < last + ASCENSION_BOUNTY_INTERVAL)
        return;
    VarSet(VAR_ASCENSION_BOUNTY_DAY, today == 0 ? 1 : today);
    gSpecialVar_Result = TRUE;
}

// Out: VAR_RESULT = TRUE and gStringVar4 = a rumour about the player, one
// visit in ASCENSION_RUMOUR_CHANCE, once the player is Champion.
void Special_Ascension_BufferRumour(void)
{
    gSpecialVar_Result = FALSE;
    if (!FlagGet(FLAG_IS_CHAMPION) || Random() % ASCENSION_RUMOUR_CHANCE != 0)
        return;
    StringExpandPlaceholders(gStringVar4, sRumours[Ascension_GetRank()][Random() % 2]);
    gSpecialVar_Result = TRUE;
}
