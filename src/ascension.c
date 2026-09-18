#include "global.h"
#include "ascension.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/difficulty.h"
#include "constants/sound.h"
#include "constants/species.h"

/*
 * Ascension, phase 1: permanent Pokémon tiers.
 *
 * State: each Pokémon stores its tier (MON_DATA_ASCENSION_TIER, 3 bits) and a
 *   win counter (MON_DATA_ASCENSION_WINS, 6 bits) in previously unused bits of
 *   struct PokemonSubstruct0. Existing Pokémon read as tier 0, so saves stay valid.
 * Effects: a stat bonus applied in CalculateMonStatsCont (src/pokemon.c), an
 *   ascended cry from ASCENSION_CRY_MIN_TIER (src/sound.c, src/pokeball.c,
 *   src/pokemon_summary_screen.c), and a star mark on the summary screen.
 * Tuning: src/data/ascension.h.
 */

#include "data/ascension.h"

static const u8 sText_TierStar[] = _("{STAR}");

u32 Ascension_GetMonTier(struct Pokemon *mon)
{
    u32 tier = GetMonData(mon, MON_DATA_ASCENSION_TIER);
    return tier > ASCENSION_MAX_TIER ? ASCENSION_MAX_TIER : tier;
}

u32 Ascension_ApplyStatBonus(u32 tier, u32 stat)
{
    if (tier == 0)
        return stat;
    if (tier > ASCENSION_MAX_TIER)
        tier = ASCENSION_MAX_TIER;
    return stat + (stat * gAscensionSteps[tier].statBonusPercent) / 100;
}

// Only the normal, healthy cry changes; weak, fainting and double-battle cries keep their mode.
u32 Ascension_GetCryMode(struct Pokemon *mon, u32 defaultMode)
{
    if (defaultMode == CRY_MODE_NORMAL && Ascension_GetMonTier(mon) >= ASCENSION_CRY_MIN_TIER)
        return CRY_MODE_ASCENDED;
    return defaultMode;
}

// Appends "★N" to dest if the Pokémon is ascended.
void Ascension_AppendTierMark(struct Pokemon *mon, u8 *dest)
{
    u32 tier = Ascension_GetMonTier(mon);
    u8 *end;

    if (tier == 0)
        return;
    end = StringAppend(dest, sText_TierStar);
    ConvertIntToDecimalStringN(end, tier, STR_CONV_MODE_LEFT_ALIGN, 1);
}
