#include "global.h"
#include "companion.h"
#include "ascension.h"
#include "battle.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "follower_npc.h"
#include "item.h"
#include "pokemon.h"
#include "quest.h"
#include "random.h"
#include "string_util.h"
#include "constants/ascension.h"
#include "constants/battle_partner.h"
#include "constants/characters.h"
#include "constants/event_objects.h"
#include "constants/follower_npc.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/opponents.h"

/*
 * Companions: champions and gym leaders the player can befriend, date and
 * marry one of.
 *
 * State: gSaveBlock1Ptr->companions (struct CompanionSave): affection, stage
 *   and day stamps per candidate, the spouse, the outing in progress and
 *   whether the spouse walks with the player. FLAG_HIDE_LOUNGE_SLOT_1..6,
 *   FLAG_HIDE_OUTING_*, FLAG_HIDE_RESIDENCE_SPOUSE hide the map objects.
 * Candidates: gCompanions in src/data/companion.h. Only those of the other
 *   gender than the player appear. Most unlock with FLAG_IS_CHAMPION; Cynthia
 *   and Lance after being beaten as visiting champions (src/ascension.c).
 * Places: the Champions' Lounge and Residence (entered from the attendant in
 *   the League lobby), outings at Lilycove, Mossdeep and the Ascension Shrine.
 *   Each map's on-transition script calls Special_Companion_SetupMap.
 * Flow: stages MET -> FRIEND -> CLOSE -> PARTNER through outings unlocked by
 *   affection; PARTNER -> SPOUSE with a Promise Ring (one spouse); SPOUSE ->
 *   ASCENDED through the Shrine ceremony at Rank V. Affection drifts down
 *   slowly when ignored but never below the current stage's floor.
 * Hooks: Companion_DoDailyEvents (clock.c), Companion_OnRankUp and the visitor
 *   and rumour checks (ascension.c), Companion_OnPartnerPartyCreated
 *   (battle_partner.c).
 * Scripts: data/scripts/companion.inc.
 */

#include "data/companion.h"

extern const u8 Companion_EventScript_Follower[];

#define sSave (gSaveBlock1Ptr->companions)

// The candidate the current script is about. Set by the Select specials.
static EWRAM_DATA u8 sCurrent = 0;
static EWRAM_DATA u16 sGiftItem = ITEM_NONE;
static EWRAM_DATA u16 sGiftAffection = 0;

static u32 Today(void)
{
    return VarGet(VAR_DAYS) + 1;
}

static u32 GetStage(u32 id)
{
    return sSave.stage[id] < COMPANION_STAGE_COUNT ? sSave.stage[id] : COMPANION_STAGE_NONE;
}

static void SetStage(u32 id, u32 stage)
{
    sSave.stage[id] = stage;
}

static void AddAffection(u32 id, s32 delta)
{
    s32 value = (s32)sSave.affection[id] + delta;

    if (value < 0)
        value = 0;
    if (value > COMPANION_MAX_AFFECTION)
        value = COMPANION_MAX_AFFECTION;
    sSave.affection[id] = value;
}

static u32 GetSpouse(void)
{
    return (sSave.spouse != 0 && sSave.spouse <= COMPANION_COUNT) ? sSave.spouse - 1 : COMPANION_NONE;
}

static u32 GetOutingPartner(void)
{
    if (sSave.outing == COMPANION_OUTING_NONE || sSave.outingWith == 0 || sSave.outingWith > COMPANION_COUNT)
        return COMPANION_NONE;
    return sSave.outingWith - 1;
}

static bool32 IsAvailableGender(u32 id)
{
    return gCompanions[id].gender != gSaveBlock2Ptr->playerGender;
}

// Candidates unlocked by becoming Champion join the lounge the first time
// the player has the title.
static void RefreshUnlocks(void)
{
    u32 id;
    bool32 anyMet = FALSE;

    for (id = 0; id < COMPANION_COUNT; id++)
    {
        if (!IsAvailableGender(id))
            continue;
        if (GetStage(id) == COMPANION_STAGE_NONE && gCompanions[id].unlock == COMPANION_UNLOCK_CHAMPION && FlagGet(FLAG_IS_CHAMPION))
            SetStage(id, COMPANION_STAGE_MET);
        if (GetStage(id) != COMPANION_STAGE_NONE)
            anyMet = TRUE;
    }
    if (anyMet && Quest_GetStage(QUEST_COMPANION) == QUEST_STAGE_NONE)
        Quest_Start(QUEST_COMPANION);
}

// Not in the lounge: the spouse lives in the Residence, someone on an outing
// waits there, and the one walking with the player is with the player.
static bool32 IsAway(u32 id)
{
    return id == GetSpouse() || id == GetOutingPartner();
}

static u32 GetLoungeCandidates(u8 *out)
{
    u32 id, count = 0;

    for (id = 0; id < COMPANION_COUNT && count < COMPANION_LOUNGE_SLOTS; id++)
    {
        if (!IsAvailableGender(id) || GetStage(id) == COMPANION_STAGE_NONE || IsAway(id))
            continue;
        out[count++] = id;
    }
    return count;
}

static void ShowObject(u16 flag, u16 gfxVar, u32 id)
{
    VarSet(gfxVar, gCompanions[id].graphicsId);
    FlagClear(flag);
}

static bool32 IsCurrentMap(u32 mapGroup, u32 mapNum)
{
    return gSaveBlock1Ptr->location.mapGroup == mapGroup && gSaveBlock1Ptr->location.mapNum == mapNum;
}

// On-transition of every map with a companion object. Picks graphics and
// shows or hides the objects for the map being entered.
void Special_Companion_SetupMap(void)
{
    u32 partner = GetOutingPartner();

    if (sSave.following && !PlayerHasFollowerNPC())
        sSave.following = FALSE;

    if (IsCurrentMap(MAP_GROUP(MAP_CHAMPIONS_LOUNGE), MAP_NUM(MAP_CHAMPIONS_LOUNGE)))
    {
        u8 slots[COMPANION_LOUNGE_SLOTS];
        u32 i, count;

        RefreshUnlocks();
        count = GetLoungeCandidates(slots);
        for (i = 0; i < COMPANION_LOUNGE_SLOTS; i++)
        {
            if (i < count)
                ShowObject(FLAG_HIDE_LOUNGE_SLOT_1 + i, VAR_OBJ_GFX_ID_0 + i, slots[i]);
            else
                FlagSet(FLAG_HIDE_LOUNGE_SLOT_1 + i);
        }
    }
    else if (IsCurrentMap(MAP_GROUP(MAP_CHAMPIONS_RESIDENCE), MAP_NUM(MAP_CHAMPIONS_RESIDENCE)))
    {
        if (GetSpouse() != COMPANION_NONE && !sSave.following && partner != GetSpouse())
            ShowObject(FLAG_HIDE_RESIDENCE_SPOUSE, VAR_OBJ_GFX_ID_0, GetSpouse());
        else
            FlagSet(FLAG_HIDE_RESIDENCE_SPOUSE);
    }
    else if (IsCurrentMap(MAP_GROUP(MAP_LILYCOVE_CITY), MAP_NUM(MAP_LILYCOVE_CITY)))
    {
        if (sSave.outing == COMPANION_OUTING_LILYCOVE && partner != COMPANION_NONE)
            ShowObject(FLAG_HIDE_OUTING_LILYCOVE, VAR_OBJ_GFX_ID_F, partner);
        else
            FlagSet(FLAG_HIDE_OUTING_LILYCOVE);
    }
    else if (IsCurrentMap(MAP_GROUP(MAP_MOSSDEEP_CITY), MAP_NUM(MAP_MOSSDEEP_CITY)))
    {
        if (sSave.outing == COMPANION_OUTING_MOSSDEEP && partner != COMPANION_NONE)
            ShowObject(FLAG_HIDE_OUTING_MOSSDEEP, VAR_OBJ_GFX_ID_F, partner);
        else
            FlagSet(FLAG_HIDE_OUTING_MOSSDEEP);
    }
    else if (IsCurrentMap(MAP_GROUP(MAP_ASCENSION_SHRINE), MAP_NUM(MAP_ASCENSION_SHRINE)))
    {
        if ((sSave.outing == COMPANION_OUTING_CONFESSION || sSave.outing == COMPANION_OUTING_CEREMONY) && partner != COMPANION_NONE)
            ShowObject(FLAG_HIDE_OUTING_SHRINE, VAR_OBJ_GFX_ID_0, partner);
        else
            FlagSet(FLAG_HIDE_OUTING_SHRINE);
    }
}

static void SelectCandidate(u32 id)
{
    sCurrent = id;
    gSpecialVar_0x8008 = id;
    StringCopy(gStringVar1, gCompanions[id].name);
}

// In: VAR_0x8004 = lounge slot (0-based). Out: VAR_0x8008 = candidate, STR_VAR_1 = name.
void Special_Companion_SelectSlot(void)
{
    u8 slots[COMPANION_LOUNGE_SLOTS];
    u32 count = GetLoungeCandidates(slots);

    SelectCandidate(gSpecialVar_0x8004 < count ? slots[gSpecialVar_0x8004] : slots[0]);
}

// Out: VAR_RESULT = TRUE if married; selects the spouse.
void Special_Companion_SelectSpouse(void)
{
    u32 spouse = GetSpouse();

    gSpecialVar_Result = (spouse != COMPANION_NONE);
    if (spouse != COMPANION_NONE)
        SelectCandidate(spouse);
}

// Out: VAR_0x8005 = outing in progress; selects the candidate on it.
void Special_Companion_SelectOuting(void)
{
    u32 partner = GetOutingPartner();

    gSpecialVar_0x8005 = sSave.outing;
    if (partner != COMPANION_NONE)
        SelectCandidate(partner);
}

// Out: gStringVar4 = something they say. VAR_RESULT = TRUE if this was the
// first talk today, which raises affection.
void Special_Companion_Talk(void)
{
    const struct CompanionInfo *info = &gCompanions[sCurrent];
    u32 stage = GetStage(sCurrent);
    u32 spouse = GetSpouse();
    const u8 *line = NULL;
    u32 count = 0;

    gSpecialVar_Result = FALSE;
    if (sSave.talkDay[sCurrent] != Today())
    {
        sSave.talkDay[sCurrent] = Today();
        AddAffection(sCurrent, COMPANION_TALK_AFFECTION);
        gSpecialVar_Result = TRUE;
    }

    if (spouse != COMPANION_NONE && spouse != sCurrent && stage >= COMPANION_STAGE_CLOSE && (Random() % 2))
    {
        line = info->whatCouldHaveBeen;
    }
    else
    {
        while (count < 3 && info->lines[stage][count] != NULL)
            count++;
        if (count != 0)
            line = info->lines[stage][Random() % count];
    }
    if (line == NULL)
        line = info->lines[COMPANION_STAGE_MET][0];
    StringExpandPlaceholders(gStringVar4, line);
}

// Out: VAR_RESULT = COMPANION_GIFT_*. On OK, STR_VAR_2 = the item offered.
// Offers their favourite item the player has, else any polite gift.
void Special_Companion_PrepareGift(void)
{
    const struct CompanionInfo *info = &gCompanions[sCurrent];
    u32 i;

    sGiftItem = ITEM_NONE;
    if (sSave.giftDay[sCurrent] == Today())
    {
        gSpecialVar_Result = COMPANION_GIFT_ALREADY;
        return;
    }
    for (i = 0; i < info->likeCount && sGiftItem == ITEM_NONE; i++)
    {
        if (CheckBagHasItem(info->likes[i].item, 1))
        {
            sGiftItem = info->likes[i].item;
            sGiftAffection = info->likes[i].affection;
        }
    }
    for (i = 0; i < ARRAY_COUNT(sNeutralGifts) && sGiftItem == ITEM_NONE; i++)
    {
        if (CheckBagHasItem(sNeutralGifts[i], 1))
        {
            sGiftItem = sNeutralGifts[i];
            sGiftAffection = COMPANION_NEUTRAL_GIFT;
        }
    }
    if (sGiftItem == ITEM_NONE)
    {
        gSpecialVar_Result = COMPANION_GIFT_NOTHING;
        return;
    }
    CopyItemName(sGiftItem, gStringVar2);
    gSpecialVar_Result = COMPANION_GIFT_OK;
}

// Gives the prepared gift. Out: VAR_RESULT = 0 polite, 1 liked, 2 delighted.
void Special_Companion_GiveGift(void)
{
    RemoveBagItem(sGiftItem, 1);
    AddAffection(sCurrent, sGiftAffection);
    sSave.giftDay[sCurrent] = Today();
    if (sGiftAffection >= GIFT_DELIGHT_THRESHOLD)
        gSpecialVar_Result = 2;
    else if (sGiftAffection > COMPANION_NEUTRAL_GIFT)
        gSpecialVar_Result = 1;
    else
        gSpecialVar_Result = 0;
}

// Out: VAR_RESULT = TRUE if they will battle today.
void Special_Companion_CanBattle(void)
{
    gSpecialVar_Result = (sSave.battleDay[sCurrent] != Today());
}

void Special_Companion_AfterBattle(void)
{
    sSave.battleDay[sCurrent] = Today();
    AddAffection(sCurrent, COMPANION_BATTLE_AFFECTION);
}

// Out: VAR_RESULT = COMPANION_OUTING_* state, VAR_0x8005 = the outing.
void Special_Companion_GetOutingState(void)
{
    u32 stage = GetStage(sCurrent);
    u32 outing = sStageOuting[stage];

    gSpecialVar_0x8005 = outing;
    if (stage == COMPANION_STAGE_PARTNER)
        gSpecialVar_Result = COMPANION_OUTING_NEEDS_RING;
    else if (outing == COMPANION_OUTING_NONE)
        gSpecialVar_Result = COMPANION_OUTING_DONE;
    else if (sSave.affection[sCurrent] < sStageAffection[stage + 1] || sSave.outing != COMPANION_OUTING_NONE)
        gSpecialVar_Result = COMPANION_OUTING_NOT_READY;
    else if (outing == COMPANION_OUTING_CEREMONY && Ascension_GetRank() < ASCENSION_MAX_RANK)
        gSpecialVar_Result = COMPANION_OUTING_NEEDS_RANK;
    else
        gSpecialVar_Result = COMPANION_OUTING_READY;
}

// In: VAR_0x8005 = outing. The candidate goes ahead and waits there.
void Special_Companion_StartOuting(void)
{
    sSave.outing = gSpecialVar_0x8005;
    sSave.outingWith = sCurrent + 1;
}

// Out: gStringVar4 = the scene for the outing in progress.
void Special_Companion_BufferOutingScene(void)
{
    u32 outing = sSave.outing;

    if (outing >= COMPANION_OUTING_LILYCOVE && outing <= COMPANION_OUTING_CONFESSION)
        StringExpandPlaceholders(gStringVar4, gCompanions[sCurrent].outings[outing - COMPANION_OUTING_LILYCOVE]);
    else
        gStringVar4[0] = EOS;
}

// Ends the outing and moves to the next stage.
// Out: STR_VAR_2 = "Goddess" or "God" (for the ceremony).
void Special_Companion_FinishOuting(void)
{
    u32 stage = GetStage(sCurrent);

    if (sSave.outing == COMPANION_OUTING_CEREMONY)
    {
        SetStage(sCurrent, COMPANION_STAGE_ASCENDED);
        Quest_Complete(QUEST_COMPANION);
    }
    else if (stage < COMPANION_STAGE_PARTNER)
    {
        SetStage(sCurrent, stage + 1);
        if (stage + 1 == COMPANION_STAGE_PARTNER && Quest_GetStage(QUEST_COMPANION) < 2)
            Quest_SetStage(QUEST_COMPANION, 2);
    }
    AddAffection(sCurrent, COMPANION_OUTING_AFFECTION);
    sSave.outing = COMPANION_OUTING_NONE;
    sSave.outingWith = 0;
    StringCopy(gStringVar2, gCompanions[sCurrent].gender == FEMALE ? COMPOUND_STRING("Goddess") : COMPOUND_STRING("God"));
}

// Out: VAR_RESULT = COMPANION_PROPOSE_*.
void Special_Companion_CanPropose(void)
{
    u32 spouse = GetSpouse();

    if (spouse == sCurrent)
        gSpecialVar_Result = COMPANION_PROPOSE_ALREADY;
    else if (spouse != COMPANION_NONE)
        gSpecialVar_Result = COMPANION_PROPOSE_TAKEN;
    else if (GetStage(sCurrent) != COMPANION_STAGE_PARTNER)
        gSpecialVar_Result = COMPANION_PROPOSE_NOT_PARTNER;
    else if (sSave.affection[sCurrent] < sStageAffection[COMPANION_STAGE_SPOUSE])
        gSpecialVar_Result = COMPANION_PROPOSE_LOW_AFFECTION;
    else if (!CheckBagHasItem(ITEM_PROMISE_RING, 1))
        gSpecialVar_Result = COMPANION_PROPOSE_NO_RING;
    else
        gSpecialVar_Result = COMPANION_PROPOSE_OK;
}

// Uses the ring and marries the current candidate. Out: gStringVar4 = their answer.
void Special_Companion_Propose(void)
{
    RemoveBagItem(ITEM_PROMISE_RING, 1);
    sSave.spouse = sCurrent + 1;
    SetStage(sCurrent, COMPANION_STAGE_SPOUSE);
    Quest_SetStage(QUEST_COMPANION, 3);
    StringExpandPlaceholders(gStringVar4, gCompanions[sCurrent].proposal);
}

// The spouse walks behind the player and joins trainer battles as a partner.
void Special_Companion_StartFollowing(void)
{
    u32 spouse = GetSpouse();

    if (spouse == COMPANION_NONE || PlayerHasFollowerNPC())
        return;
    SetFollowerNPCData(FNPC_DATA_BATTLE_PARTNER, gCompanions[spouse].partnerId);
    CreateFollowerNPC(gCompanions[spouse].graphicsId, FNPC_ALL, Companion_EventScript_Follower);
    sSave.following = TRUE;
}

void Special_Companion_StopFollowing(void)
{
    if (PlayerHasFollowerNPC())
        DestroyFollowerNPC();
    sSave.following = FALSE;
}

// In: VAR_0x8009 = visitor beaten (ASCENSION_VISITOR_*).
// Out: VAR_RESULT = TRUE if that visitor now stays in the lounge; STR_VAR_1 = name.
void Special_Companion_OnVisitorBeaten(void)
{
    u32 id;

    gSpecialVar_Result = FALSE;
    for (id = 0; id < COMPANION_COUNT; id++)
    {
        if (gCompanions[id].unlock != COMPANION_UNLOCK_VISITOR || gCompanions[id].visitorId != gSpecialVar_0x8009)
            continue;
        if (!IsAvailableGender(id) || GetStage(id) != COMPANION_STAGE_NONE)
            continue;
        SetStage(id, COMPANION_STAGE_MET);
        StringCopy(gStringVar1, gCompanions[id].name);
        gSpecialVar_Result = TRUE;
    }
}

// Visitors who became companions stop arriving as challengers.
bool32 Companion_IsVisitorStaying(u32 visitorId)
{
    u32 id;

    for (id = 0; id < COMPANION_COUNT; id++)
    {
        if (gCompanions[id].unlock == COMPANION_UNLOCK_VISITOR && gCompanions[id].visitorId == visitorId
         && IsAvailableGender(id) && GetStage(id) != COMPANION_STAGE_NONE)
            return TRUE;
    }
    return FALSE;
}

// Affection drifts down after COMPANION_DECAY_GRACE_DAYS without talking,
// but never below the floor of the stage already reached.
void Companion_DoDailyEvents(u32 daysSince)
{
    u32 id, today = Today();

    for (id = 0; id < COMPANION_COUNT; id++)
    {
        u32 stage = GetStage(id);
        u32 floor = sStageAffection[stage];

        if (stage == COMPANION_STAGE_NONE || sSave.talkDay[id] == 0)
            continue;
        if (today < sSave.talkDay[id] + COMPANION_DECAY_GRACE_DAYS)
            continue;
        if (sSave.affection[id] > floor)
        {
            u32 loss = min(daysSince * COMPANION_DECAY_PER_DAY, sSave.affection[id] - floor);
            AddAffection(id, -(s32)loss);
        }
    }
}

void Companion_OnRankUp(void)
{
    u32 id;

    for (id = 0; id < COMPANION_COUNT; id++)
    {
        if (GetStage(id) >= COMPANION_STAGE_PARTNER)
            AddAffection(id, COMPANION_RANK_AFFECTION);
    }
}

// Called after FillPartnerParty builds the spouse's team for a partner battle.
// Levels follow the player; an ascended spouse fields tier 5.
void Companion_OnPartnerPartyCreated(u16 trainerId)
{
    u32 spouse = GetSpouse();
    u32 i, total = 0, count = 0, level, tier;

    if (spouse == COMPANION_NONE || trainerId != TRAINER_PARTNER(gCompanions[spouse].partnerId))
        return;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        total += GetMonData(mon, MON_DATA_LEVEL);
        count++;
    }
    level = count ? total / count : 50;
    tier = GetStage(spouse) == COMPANION_STAGE_ASCENDED ? ASCENSION_MAX_TIER : min(gAscensionRanks[Ascension_GetRank()].npcMaxTier, 3);

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PARTNER][i];
        u32 species = GetMonData(mon, MON_DATA_SPECIES);
        u32 exp;

        if (species == SPECIES_NONE)
            break;
        if (GetMonData(mon, MON_DATA_LEVEL) < level)
        {
            exp = gExperienceTables[gSpeciesInfo[species].growthRate][level];
            SetMonData(mon, MON_DATA_EXP, &exp);
        }
        SetMonData(mon, MON_DATA_ASCENSION_TIER, &tier);
        CalculateMonStats(mon);
    }
}

// Rumour about the closest companion, if anyone has noticed yet.
// Out: gStringVar4. Returns FALSE if there is nothing to say.
bool32 Companion_TryBufferRumour(void)
{
    u32 id, best = COMPANION_NONE, bestStage = COMPANION_STAGE_MET;

    for (id = 0; id < COMPANION_COUNT; id++)
    {
        if (GetStage(id) > bestStage)
        {
            best = id;
            bestStage = GetStage(id);
        }
    }
    if (best == COMPANION_NONE || sRumours[bestStage][0] == NULL)
        return FALSE;
    StringCopy(gStringVar1, gCompanions[best].name);
    StringExpandPlaceholders(gStringVar4, sRumours[bestStage][Random() % 2]);
    return TRUE;
}
