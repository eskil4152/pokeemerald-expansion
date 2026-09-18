#ifndef GUARD_CONSTANTS_ASCENSION_H
#define GUARD_CONSTANTS_ASCENSION_H

// Ascension (src/ascension.c). Tables with the tuning numbers live in src/data/ascension.h.

#define ASCENSION_MAX_TIER      5       // Pokémon tier 0..5, stored in 3 bits (max 7)
#define ASCENSION_MAX_RANK      5       // player rank 0..5
#define ASCENSION_MAX_WINS      63      // per-Pokémon win counter, stored in 6 bits

// Cry of an ascended Pokémon (CRY_MODE_ASCENDED in src/sound.c).
// Normal cries use pitch 15360, length 210, release 0, chorus 0.
#define ASCENSION_CRY_MIN_TIER  3       // tier from which the ascended cry plays
#define ASCENSION_CRY_PITCH     16000   // higher = higher pitch
#define ASCENSION_CRY_LENGTH    210
#define ASCENSION_CRY_RELEASE   30
#define ASCENSION_CRY_CHORUS    60      // shimmer; Dynamax uses 200

// Reign (src/ascension.c, Ascension_DoDailyEvents).
#define ASCENSION_TRIAL_I_DAYS          7   // days the title must be held for Trial I
#define ASCENSION_CHALLENGE_INTERVAL    7   // a visiting champion arrives every this many reign days
#define ASCENSION_CHALLENGE_DEADLINE    3   // days to answer a challenge before losing the title
#define ASCENSION_SHARDS_PER_RANK       2   // shards granted for each trial passed
#define ASCENSION_VISITOR_BOUNTY        1   // shards for beating a visiting champion
#define ASCENSION_WEEKLY_BOUNTY         1   // shards for the weekly arena boss win
#define ASCENSION_BOUNTY_INTERVAL       7   // days between weekly bounties
#define ASCENSION_RUMOUR_CHANCE         3   // nurses share a rumour one visit in this many

// Post-champion trainer scaling (Ascension_OnTrainerPartyCreated).
#define ASCENSION_SCALE_MARGIN          5   // regular trainers: party average minus this, at least the rank floor
#define ASCENSION_ELITE_BONUS           2   // Elite Four and Wallace: party average plus this
#define ASCENSION_VISITOR_BONUS         3   // visiting champions: party average plus this
#define ASCENSION_ASCENDED_BONUS        5   // the Ascended One: party average plus this

// Visiting champions, in rotation order. Stored in VAR_ASCENSION_VISITOR.
#define ASCENSION_VISITOR_NONE          0
#define ASCENSION_VISITOR_RED           1
#define ASCENSION_VISITOR_LEAF          2
#define ASCENSION_VISITOR_BLUE          3
#define ASCENSION_VISITOR_LANCE         4
#define ASCENSION_VISITOR_STEVEN        5
#define ASCENSION_VISITOR_WALLACE       6
#define ASCENSION_VISITOR_CYNTHIA       7
#define ASCENSION_VISITOR_COUNT         8

// Ascension_CheckRequirements results (VAR_RESULT).
#define ASC_REQ_OK                      0
#define ASC_REQ_EGG                     1
#define ASC_REQ_MAXED                   2
#define ASC_REQ_RANK                    3
#define ASC_REQ_LEVEL                   4
#define ASC_REQ_FRIENDSHIP              5
#define ASC_REQ_WINS                    6
#define ASC_REQ_SHARDS                  7

// Ascension_GetTrialState results (VAR_RESULT). VAR_0x8005 gets the current rank.
#define ASC_TRIAL_NOT_READY             0
#define ASC_TRIAL_READY                 1   // condition met, rank can advance now
#define ASC_TRIAL_BATTLE                2   // the trial is a fight the Oracle starts
#define ASC_TRIAL_ALL_DONE              3
#define ASC_TRIAL_DETHRONED             4

#endif // GUARD_CONSTANTS_ASCENSION_H
