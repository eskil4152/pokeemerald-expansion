#ifndef GUARD_ASCENSION_H
#define GUARD_ASCENSION_H

#include "constants/ascension.h"

// One row per tier: what it takes to reach that tier, and what it grants.
// Row 0 is the unascended state.
struct AscensionStep
{
    u8 requiredRank;            // player rank needed to perform this step
    u8 minLevel;                // 0 = no level requirement
    u8 minFriendship;
    u8 requiredWins;            // trainer wins since the previous step
    u8 shardCost;
    u8 statBonusPercent;        // added to all six stats at this tier
    enum Species guardian;
    s8 guardianLevelOffset;     // guardian level = challenger level + offset
};

// One row per player rank.
struct AscensionRank
{
    u8 trainerLevelFloor;       // post-champion regular trainers are at least this level
    u8 npcMaxTier;              // highest tier a visiting champion may field
    u8 difficulty;              // enum DifficultyLevel for B_VAR_DIFFICULTY
};

struct AscensionVisitor
{
    u16 trainerId;
    u16 graphicsId;             // OBJ_EVENT_GFX_* shown in the League lobby
    u8 maxTier;                 // highest tier this champion fields, capped by player rank
};

extern const struct AscensionStep gAscensionSteps[ASCENSION_MAX_TIER + 1];
extern const struct AscensionRank gAscensionRanks[ASCENSION_MAX_RANK + 1];
extern const struct AscensionVisitor gAscensionVisitors[ASCENSION_VISITOR_COUNT];

u32 Ascension_GetMonTier(struct Pokemon *mon);
u32 Ascension_ApplyStatBonus(u32 tier, u32 stat);
u32 Ascension_GetCryMode(struct Pokemon *mon, u32 defaultMode);
void Ascension_AppendTierMark(struct Pokemon *mon, u8 *dest);
u32 Ascension_GetRank(void);

// Engine hooks.
bool32 Ascension_IsGuardianBattle(void);
void Ascension_OnTrainerPartyCreated(struct Pokemon *party, u16 trainerId);
void Ascension_OnBattleFinished(void);
void Ascension_DoDailyEvents(u32 daysSince);

// Specials (data/specials.inc).
void Special_Ascension_CheckRequirements(void);
void Special_Ascension_StartGuardianBattle(void);
void Special_Ascension_FinishGuardianBattle(void);
void Special_Ascension_GetTrialState(void);
void Special_Ascension_AdvanceRank(void);
void Special_Ascension_BufferStatus(void);
void Special_Ascension_SetupLeagueVisitor(void);
void Special_Ascension_BufferVisitor(void);
void Special_Ascension_OnVisitorBattle(void);
void Special_Ascension_TryWeeklyBounty(void);
void Special_Ascension_BufferRumour(void);

#endif // GUARD_ASCENSION_H
