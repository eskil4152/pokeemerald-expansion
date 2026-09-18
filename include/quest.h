#ifndef GUARD_QUEST_H
#define GUARD_QUEST_H

#include "constants/quests.h"

struct Quest
{
    const u8 *name;
    const u8 *description;
    const u8 *const *stageTexts;    // stageCount entries
    const u8 *lockedText;           // shown in the Journal while locked, may be NULL
    u8 stageCount;
    u16 unlockFlag;                 // 0 = no lock
};

extern const struct Quest gQuests[QUEST_COUNT];

void Quest_ResetSaveData(void);
u32 Quest_GetStage(enum QuestId id);
void Quest_SetStage(enum QuestId id, u32 stage);
void Quest_Start(enum QuestId id);
void Quest_Advance(enum QuestId id);
void Quest_Complete(enum QuestId id);
bool32 Quest_IsActive(enum QuestId id);
bool32 Quest_IsDone(enum QuestId id);
bool32 Quest_IsLocked(enum QuestId id);
bool32 Quest_IsVisibleInJournal(enum QuestId id);

// Specials (data/specials.inc). Quest id in VAR_0x8004, stage in VAR_0x8005.
void Special_Quest_Start(void);
void Special_Quest_Advance(void);
void Special_Quest_SetStage(void);
void Special_Quest_Complete(void);
void Special_Quest_GetStage(void);

#endif // GUARD_QUEST_H
