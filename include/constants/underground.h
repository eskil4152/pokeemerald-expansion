#ifndef GUARD_CONSTANTS_UNDERGROUND_H
#define GUARD_CONSTANTS_UNDERGROUND_H

// Placeholder currency name: UCOIN. Rename in strings only; the code uses "ucoins".

#define UCOIN_MAX               999999
#define UCOIN_VAR_CAP           65535   // largest amount a script var can carry

// Heat: 0..100. Illegal actions raise it, it decays daily, high heat locks the underground.
#define HEAT_MAX                100
#define HEAT_DECAY_PER_DAY      10
#define HEAT_LOCKOUT_THRESHOLD  80
#define HEAT_SELL_MON           15
#define HEAT_BUY_MON            5
#define HEAT_BUY_WORKER         10
#define HEAT_COLLECT_PAYOUT     5

// Workers: owned NPC labour that produces UCOIN daily.
enum WorkerType
{
    WORKER_PICKPOCKET,
    WORKER_SMUGGLER,
    WORKER_FENCE,
    NUM_WORKER_TYPES,
};
#define MAX_WORKERS_PER_TYPE    99

// Results (VAR_RESULT) of Underground_BuyMon.
// MON_GIVEN_TO_PARTY / MON_GIVEN_TO_PC / MON_CANT_GIVE come from constants/pokemon.h.
#define UNDERGROUND_CANT_AFFORD 3

#endif // GUARD_CONSTANTS_UNDERGROUND_H
