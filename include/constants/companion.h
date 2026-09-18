#ifndef GUARD_CONSTANTS_COMPANION_H
#define GUARD_CONSTANTS_COMPANION_H

// Companions (src/companion.c). Candidates, gifts and lines live in src/data/companion.h.

// Candidates. Ids are stored in the save; append new ones before COMPANION_COUNT.
// Only candidates of the other gender than the player appear.
#define COMPANION_CYNTHIA           0
#define COMPANION_ROXANNE           1
#define COMPANION_FLANNERY          2
#define COMPANION_WINONA            3
#define COMPANION_PHOEBE            4
#define COMPANION_GLACIA            5
#define COMPANION_STEVEN            6
#define COMPANION_WALLACE           7
#define COMPANION_BRAWLY            8
#define COMPANION_SIDNEY            9
#define COMPANION_LANCE             10
#define COMPANION_COUNT             11
#define COMPANION_MAX               16      // size of the save arrays
#define COMPANION_NONE              0xFF

// Stage per candidate. Outings lead from one stage to the next; the ring leads
// from PARTNER to SPOUSE.
#define COMPANION_STAGE_NONE        0   // not met yet
#define COMPANION_STAGE_MET         1   // hangs out in the Champions' Lounge
#define COMPANION_STAGE_FRIEND      2   // after the Lilycove outing
#define COMPANION_STAGE_CLOSE       3   // after the Mossdeep outing
#define COMPANION_STAGE_PARTNER     4   // after the Shrine confession
#define COMPANION_STAGE_SPOUSE      5   // married; lives in the Residence
#define COMPANION_STAGE_ASCENDED    6   // ascended together at Rank V
#define COMPANION_STAGE_COUNT       7

// Outings. The candidate waits at the outing's map until talked to.
#define COMPANION_OUTING_NONE       0
#define COMPANION_OUTING_LILYCOVE   1
#define COMPANION_OUTING_MOSSDEEP   2
#define COMPANION_OUTING_CONFESSION 3
#define COMPANION_OUTING_CEREMONY   4

// How a candidate is unlocked.
#define COMPANION_UNLOCK_CHAMPION   0   // once the player is Champion
#define COMPANION_UNLOCK_VISITOR    1   // after beating them as a visiting champion

// Affection, 0..COMPANION_MAX_AFFECTION. Thresholds per stage in src/data/companion.h.
#define COMPANION_MAX_AFFECTION     1000
#define COMPANION_TALK_AFFECTION    8   // first talk each day
#define COMPANION_BATTLE_AFFECTION  15  // lounge battle, once a day, win or lose
#define COMPANION_RANK_AFFECTION    20  // everyone at PARTNER or above, per ascension rank gained
#define COMPANION_OUTING_AFFECTION  40  // each outing completed
#define COMPANION_NEUTRAL_GIFT      3   // any gift item they don't especially like
#define COMPANION_DECAY_GRACE_DAYS  3   // days without talking before affection drifts down
#define COMPANION_DECAY_PER_DAY     2   // never below the current stage's floor

#define COMPANION_LOUNGE_SLOTS      6

// Special_Companion_PrepareGift results (VAR_RESULT).
#define COMPANION_GIFT_OK           0
#define COMPANION_GIFT_ALREADY      1
#define COMPANION_GIFT_NOTHING      2

// Special_Companion_GetOutingState results (VAR_RESULT). VAR_0x8005 = outing.
#define COMPANION_OUTING_NOT_READY  0
#define COMPANION_OUTING_READY      1
#define COMPANION_OUTING_NEEDS_RANK 2   // ceremony: affection reached, Rank V missing
#define COMPANION_OUTING_NEEDS_RING 3   // PARTNER: next step is proposing
#define COMPANION_OUTING_DONE       4   // nothing left

// Special_Companion_CanPropose results (VAR_RESULT).
#define COMPANION_PROPOSE_OK            0
#define COMPANION_PROPOSE_NOT_PARTNER   1
#define COMPANION_PROPOSE_LOW_AFFECTION 2
#define COMPANION_PROPOSE_NO_RING       3
#define COMPANION_PROPOSE_TAKEN         4   // already married to someone
#define COMPANION_PROPOSE_ALREADY       5   // this one is the spouse

#endif // GUARD_CONSTANTS_COMPANION_H
