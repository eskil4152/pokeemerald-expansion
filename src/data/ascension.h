// Ascension tuning. Every number the ascension systems use lives here.
// Included by src/ascension.c. Change values freely; the code only reads them.
//
// Steps: row N is what it takes to reach tier N and what tier N grants.
// Guardian: fought one on one by the ascending Pokémon at the target tier.

const struct AscensionStep gAscensionSteps[ASCENSION_MAX_TIER + 1] =
{
    //     requiredRank  minLevel  minFriendship  requiredWins  shardCost  statBonus%  guardian                  levelOffset
    [0] = { 0,           0,        0,             0,            0,         0,          SPECIES_NONE,              0 },
    [1] = { 1,           0,        MAX_FRIENDSHIP, 0,           1,         8,          SPECIES_SHAYMIN,          -5 },
    [2] = { 2,           0,        MAX_FRIENDSHIP, 20,          1,         16,         SPECIES_DARKRAI,          -3 },
    [3] = { 3,           0,        MAX_FRIENDSHIP, 30,          2,         24,         SPECIES_DIALGA,           -3 },
    [4] = { 4,           0,        MAX_FRIENDSHIP, 40,          2,         32,         SPECIES_PALKIA,            0 },
    [5] = { 5,           0,        MAX_FRIENDSHIP, 50,          3,         50,         SPECIES_GIRATINA_ORIGIN,   2 },
};

// Ranks: row 0 is Champion without ascension, rows 1..5 are Ranks I..V.
// trainerLevelFloor: post-champion trainers are raised to at least this level.
// npcMaxTier: highest tier visiting champions and the arena boss may field.
// difficulty: not applied yet (no trainer has Hard parties); see FEATURE_ASCENSION.md.
const struct AscensionRank gAscensionRanks[ASCENSION_MAX_RANK + 1] =
{
    //     trainerLevelFloor  npcMaxTier  difficulty
    [0] = { 60,               0,          DIFFICULTY_NORMAL },
    [1] = { 65,               1,          DIFFICULTY_NORMAL },
    [2] = { 70,               2,          DIFFICULTY_NORMAL },
    [3] = { 75,               3,          DIFFICULTY_HARD },
    [4] = { 80,               4,          DIFFICULTY_HARD },
    [5] = { 90,               5,          DIFFICULTY_HARD },
};

// Visiting champions, in rotation order. maxTier is further capped by the
// player's rank (gAscensionRanks[rank].npcMaxTier).
const struct AscensionVisitor gAscensionVisitors[ASCENSION_VISITOR_COUNT] =
{
    //                              trainerId                graphicsId               maxTier
    [ASCENSION_VISITOR_NONE]    = { TRAINER_NONE,            OBJ_EVENT_GFX_BOY_1,     0 },
    [ASCENSION_VISITOR_RED]     = { TRAINER_RED,             OBJ_EVENT_GFX_RED,       3 },
    [ASCENSION_VISITOR_LEAF]    = { TRAINER_LEAF,            OBJ_EVENT_GFX_LEAF,      3 },
    [ASCENSION_VISITOR_BLUE]    = { TRAINER_VISITOR_BLUE,    OBJ_EVENT_GFX_BLUE,      3 },
    [ASCENSION_VISITOR_LANCE]   = { TRAINER_VISITOR_LANCE,   OBJ_EVENT_GFX_LANCE,     4 },
    [ASCENSION_VISITOR_STEVEN]  = { TRAINER_VISITOR_STEVEN,  OBJ_EVENT_GFX_STEVEN,    5 },
    [ASCENSION_VISITOR_WALLACE] = { TRAINER_VISITOR_WALLACE, OBJ_EVENT_GFX_WALLACE,   5 },
};

// Tier of every Pokémon the Ascended One fields (Trial V).
#define ASCENDED_ONE_TIER   5

// What people say about you once you are Champion, by rank (0..5).
// Nurses pick one at random. {PLAYER} expands to the player's name.
static const u8 *const sRumours[ASCENSION_MAX_RANK + 1][2] =
{
    [0] = { COMPOUND_STRING("People say the new Champion\nnever sleeps."),
            COMPOUND_STRING("A mystic waits in the League lobby.\nThey asked for you by name.") },
    [1] = { COMPOUND_STRING("They say {PLAYER}'s Pokémon glow\nfaintly at night."),
            COMPOUND_STRING("Trainers from other regions are\nasking where to find you.") },
    [2] = { COMPOUND_STRING("The Underground bows when you\nwalk past, I hear."),
            COMPOUND_STRING("Your Pokémon's cries sound\ndifferent now. Deeper.") },
    [3] = { COMPOUND_STRING("Even Champions of Kanto talk\nabout you now."),
            COMPOUND_STRING("Some say you went to the Shrine\nand came back changed.") },
    [4] = { COMPOUND_STRING("Children draw your face on\ntheir walls, {PLAYER}."),
            COMPOUND_STRING("Legends are said to hide when\nyou pass by.") },
    [5] = { COMPOUND_STRING("...Forgive me. I didn't know\nwhether to bow."),
            COMPOUND_STRING("They built a small shrine to\nyou on Route 101.") },
};
