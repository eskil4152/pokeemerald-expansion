#ifndef GUARD_COMPANION_H
#define GUARD_COMPANION_H

#include "constants/companion.h"

struct CompanionGift
{
    u16 item;
    u16 affection;
};

struct CompanionInfo
{
    const u8 *name;
    u16 graphicsId;
    u16 trainerId;                  // lounge battles
    u8 partnerId;                   // PARTNER_* when walking together as spouse
    u8 gender;                      // MALE / FEMALE; shown to players of the other gender
    u8 unlock;                      // COMPANION_UNLOCK_*
    u8 visitorId;                   // ASCENSION_VISITOR_* for COMPANION_UNLOCK_VISITOR
    const struct CompanionGift *likes;
    u8 likeCount;
    const u8 *lines[COMPANION_STAGE_COUNT][3];
    const u8 *outings[3];           // Lilycove, Mossdeep, Shrine confession
    const u8 *proposal;
    const u8 *whatCouldHaveBeen;    // said by others after you marry someone else
};

extern const struct CompanionInfo gCompanions[COMPANION_COUNT];

// Engine hooks.
void Companion_DoDailyEvents(u32 daysSince);
void Companion_OnRankUp(void);
void Companion_OnPartnerPartyCreated(u16 trainerId);
bool32 Companion_TryBufferRumour(void);
bool32 Companion_IsVisitorStaying(u32 visitorId);

// Specials (data/specials.inc).
void Special_Companion_SetupMap(void);
void Special_Companion_SelectSlot(void);
void Special_Companion_SelectSpouse(void);
void Special_Companion_SelectOuting(void);
void Special_Companion_Talk(void);
void Special_Companion_PrepareGift(void);
void Special_Companion_GiveGift(void);
void Special_Companion_CanBattle(void);
void Special_Companion_AfterBattle(void);
void Special_Companion_GetOutingState(void);
void Special_Companion_StartOuting(void);
void Special_Companion_BufferOutingScene(void);
void Special_Companion_FinishOuting(void);
void Special_Companion_CanPropose(void);
void Special_Companion_Propose(void);
void Special_Companion_StartFollowing(void);
void Special_Companion_StopFollowing(void);
void Special_Companion_OnVisitorBeaten(void);

#endif // GUARD_COMPANION_H
