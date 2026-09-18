#ifndef GUARD_CONSTANTS_QUESTS_H
#define GUARD_CONSTANTS_QUESTS_H

// Quest ids index gQuests (src/data/quests.h) and gSaveBlock1Ptr->questStages.
// Append new quests at the end; ids are stored in saves.
enum QuestId
{
    QUEST_MEET_THE_NEIGHBOURS,
    QUEST_ASCENSION_I,
    QUEST_ASCENSION_II,
    QUEST_ASCENSION_III,
    QUEST_ASCENSION_IV,
    QUEST_ASCENSION_V,
    QUEST_COUNT,
};

#define MAX_QUESTS          32      // size of the save array; QUEST_COUNT must stay below this

// Values of a quest's stage byte.
#define QUEST_STAGE_NONE    0       // not started
#define QUEST_STAGE_DONE    0xFF    // completed
// 1..stageCount: active, at that stage

#endif // GUARD_CONSTANTS_QUESTS_H
