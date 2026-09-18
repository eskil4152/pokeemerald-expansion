#include "global.h"
#include "quest.h"
#include "event_data.h"
#include "sound.h"
#include "constants/songs.h"

/*
 * Quest system.
 *
 * State: gSaveBlock1Ptr->questStages[MAX_QUESTS], one byte per quest:
 *   QUEST_STAGE_NONE, 1..stageCount (active), QUEST_STAGE_DONE.
 * Trigger: script macros startquest / advancequest / setqueststage /
 *   completequest / checkquest (asm/macros/event.inc) wrapping the specials
 *   below, callable from any NPC or map script.
 * Presentation: the Journal screen (src/quest_log.c) from the Start menu,
 *   which appears once the first quest has started (FLAG_SYS_JOURNAL_GET).
 * Data: src/data/quests.h.
 */

#include "data/quests.h"

STATIC_ASSERT(QUEST_COUNT <= MAX_QUESTS, TooManyQuestsForSaveArray);

void Quest_ResetSaveData(void)
{
    memset(gSaveBlock1Ptr->questStages, QUEST_STAGE_NONE, sizeof(gSaveBlock1Ptr->questStages));
}

u32 Quest_GetStage(enum QuestId id)
{
    if (id >= QUEST_COUNT)
        return QUEST_STAGE_NONE;
    return gSaveBlock1Ptr->questStages[id];
}

void Quest_SetStage(enum QuestId id, u32 stage)
{
    if (id >= QUEST_COUNT)
        return;
    if (stage != QUEST_STAGE_DONE && stage > gQuests[id].stageCount)
        stage = gQuests[id].stageCount;
    gSaveBlock1Ptr->questStages[id] = stage;
    if (stage != QUEST_STAGE_NONE)
        FlagSet(FLAG_SYS_JOURNAL_GET);
}

void Quest_Start(enum QuestId id)
{
    if (Quest_GetStage(id) == QUEST_STAGE_NONE)
        Quest_SetStage(id, 1);
}

// Moves an active quest to its next stage; past the last stage it completes.
void Quest_Advance(enum QuestId id)
{
    u32 stage = Quest_GetStage(id);

    if (!Quest_IsActive(id))
        return;
    if (stage >= gQuests[id].stageCount)
        Quest_SetStage(id, QUEST_STAGE_DONE);
    else
        Quest_SetStage(id, stage + 1);
}

void Quest_Complete(enum QuestId id)
{
    Quest_SetStage(id, QUEST_STAGE_DONE);
}

bool32 Quest_IsActive(enum QuestId id)
{
    u32 stage = Quest_GetStage(id);
    return stage != QUEST_STAGE_NONE && stage != QUEST_STAGE_DONE;
}

bool32 Quest_IsDone(enum QuestId id)
{
    return Quest_GetStage(id) == QUEST_STAGE_DONE;
}

bool32 Quest_IsLocked(enum QuestId id)
{
    if (id >= QUEST_COUNT || gQuests[id].unlockFlag == 0)
        return FALSE;
    return Quest_GetStage(id) == QUEST_STAGE_NONE && !FlagGet(gQuests[id].unlockFlag);
}

// Started quests always show. Unstarted ones show only if they advertise a lock.
bool32 Quest_IsVisibleInJournal(enum QuestId id)
{
    if (id >= QUEST_COUNT)
        return FALSE;
    return Quest_GetStage(id) != QUEST_STAGE_NONE || gQuests[id].unlockFlag != 0;
}

// ---- Specials ----

void Special_Quest_Start(void)
{
    Quest_Start(gSpecialVar_0x8004);
}

void Special_Quest_Advance(void)
{
    Quest_Advance(gSpecialVar_0x8004);
}

void Special_Quest_SetStage(void)
{
    Quest_SetStage(gSpecialVar_0x8004, gSpecialVar_0x8005);
}

void Special_Quest_Complete(void)
{
    Quest_Complete(gSpecialVar_0x8004);
}

// Out: VAR_RESULT = stage (QUEST_STAGE_NONE / 1..n / QUEST_STAGE_DONE).
void Special_Quest_GetStage(void)
{
    gSpecialVar_Result = Quest_GetStage(gSpecialVar_0x8004);
}
