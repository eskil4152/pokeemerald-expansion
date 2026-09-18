// Quest definitions. Included by src/quest.c.
//
// Each quest: a name, a one-line description, one text per stage (what to do
// now), an optional text shown while locked, and an optional unlock flag.
// A quest with an unlock flag is listed in the Journal as "Locked" until the
// flag is set, which is how ascension tiers advertise their requirements.

static const u8 *const sQuestStages_MeetTheNeighbours[] =
{
    COMPOUND_STRING("Find the man in Littleroot who knows about storing items."),
    COMPOUND_STRING("Report back to the boy near the lab."),
};

static const u8 *const sQuestStages_AscensionI[] =
{
    COMPOUND_STRING("Speak to the one who waits beyond the League."),
};

const struct Quest gQuests[QUEST_COUNT] =
{
    [QUEST_MEET_THE_NEIGHBOURS] =
    {
        .name = COMPOUND_STRING("Meet the Neighbours"),
        .description = COMPOUND_STRING("The boy wants you to meet everyone in town."),
        .stageTexts = sQuestStages_MeetTheNeighbours,
        .stageCount = ARRAY_COUNT(sQuestStages_MeetTheNeighbours),
        .unlockFlag = 0,
        .lockedText = NULL,
    },
    [QUEST_ASCENSION_I] =
    {
        .name = COMPOUND_STRING("Ascension I"),
        .description = COMPOUND_STRING("The first step beyond being Champion."),
        .stageTexts = sQuestStages_AscensionI,
        .stageCount = ARRAY_COUNT(sQuestStages_AscensionI),
        .unlockFlag = FLAG_IS_CHAMPION,
        .lockedText = COMPOUND_STRING("Become Champion to unlock."),
    },
};
