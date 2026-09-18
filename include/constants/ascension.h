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

#endif // GUARD_CONSTANTS_ASCENSION_H
