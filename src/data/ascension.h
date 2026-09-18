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
// Not read yet; used by post-champion scaling and visiting champions (phase 4).
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
