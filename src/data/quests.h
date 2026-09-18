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
    COMPOUND_STRING("Hold the Champion title for 7 days, then present your trial to the Oracle."),
};

static const u8 *const sQuestStages_AscensionII[] =
{
    COMPOUND_STRING("Clear every table of the Underground Arena, then return to the Oracle."),
};

static const u8 *const sQuestStages_AscensionIII[] =
{
    COMPOUND_STRING("Defeat a visiting Champion in the League lobby, then return to the Oracle."),
};

static const u8 *const sQuestStages_AscensionIV[] =
{
    COMPOUND_STRING("Ask the Oracle for the gauntlet: five battles, no items, no healing."),
};

static const u8 *const sQuestStages_AscensionV[] =
{
    COMPOUND_STRING("Ask the Oracle to face the Ascended One."),
};

static const u8 *const sQuestStages_Companion[] =
{
    COMPOUND_STRING("Spend time with the Champions in the Lounge at the Pokémon League."),
    COMPOUND_STRING("Someone loves you back. A Promise Ring from the Lounge attendant could make it forever."),
    COMPOUND_STRING("Once you reach Rank V, ascend together at the Shrine."),
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
        .description = COMPOUND_STRING("A mystic in the League lobby speaks of a Shrine beyond the title."),
        .stageTexts = sQuestStages_AscensionI,
        .stageCount = ARRAY_COUNT(sQuestStages_AscensionI),
        .unlockFlag = FLAG_IS_CHAMPION,
        .lockedText = COMPOUND_STRING("Become Champion to unlock."),
    },
    [QUEST_ASCENSION_II] =
    {
        .name = COMPOUND_STRING("Ascension II"),
        .description = COMPOUND_STRING("Rule the Underground as well as the League."),
        .stageTexts = sQuestStages_AscensionII,
        .stageCount = ARRAY_COUNT(sQuestStages_AscensionII),
        .unlockFlag = 0,
        .lockedText = NULL,
    },
    [QUEST_ASCENSION_III] =
    {
        .name = COMPOUND_STRING("Ascension III"),
        .description = COMPOUND_STRING("Your dominance must be known."),
        .stageTexts = sQuestStages_AscensionIII,
        .stageCount = ARRAY_COUNT(sQuestStages_AscensionIII),
        .unlockFlag = 0,
        .lockedText = NULL,
    },
    [QUEST_ASCENSION_IV] =
    {
        .name = COMPOUND_STRING("Ascension IV"),
        .description = COMPOUND_STRING("Five shadows stand between you and the summit."),
        .stageTexts = sQuestStages_AscensionIV,
        .stageCount = ARRAY_COUNT(sQuestStages_AscensionIV),
        .unlockFlag = 0,
        .lockedText = NULL,
    },
    [QUEST_ASCENSION_V] =
    {
        .name = COMPOUND_STRING("Ascension V"),
        .description = COMPOUND_STRING("Someone climbed the Shrine before you."),
        .stageTexts = sQuestStages_AscensionV,
        .stageCount = ARRAY_COUNT(sQuestStages_AscensionV),
        .unlockFlag = 0,
        .lockedText = NULL,
    },
    [QUEST_COMPANION] =
    {
        .name = COMPOUND_STRING("Hearts"),
        .description = COMPOUND_STRING("Champions need company too."),
        .stageTexts = sQuestStages_Companion,
        .stageCount = ARRAY_COUNT(sQuestStages_Companion),
        .unlockFlag = 0,
        .lockedText = NULL,
    },
};
