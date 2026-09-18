# Feature: Champions' Lounge (companions, romance, marriage)

Branch `feature/lounge`, built on `feature/ascension`.

## Goal

After becoming Champion, befriend and date other Champions, Gym Leaders and Elite Four members; marry one. Affection rises and falls slowly and never fails. Cynthia has the deepest arc.

## Candidates

Only candidates of the other gender than the player appear.

- For a male player: Cynthia, Roxanne, Flannery, Winona, Phoebe, Glacia.
- For a female player: Steven, Wallace, Brawly, Sidney, Lance.

Cynthia and Lance join after you beat them as visiting champions. Everyone else joins once you are Champion. Cynthia now arrives first in the visitor rotation; visitors who stay stop arriving as challengers. Child characters (May, Brendan, Leaf, Red, Tate and Liza) are left out on purpose.

## Places

- **League lobby attendant** (gentleman, middle of the lobby): Lounge, Residence (once married), Promise Ring shop (₽30,000).
- **Champions' Lounge** (Battle Frontier lounge layout): up to six candidates.
- **Champions' Residence** (bedroom layout with PC): the spouse lives here. Rest together (heals), walk together, and the ceremony when ready. The stairs lead back to the League lobby.
- **Outings**: the candidate goes ahead and waits at Lilycove beach (east), Mossdeep beach (west), or the Ascension Shrine.

## Flow

| Stage | How you get there |
|---|---|
| Met | Unlock (Champion, or beat them as a visitor) |
| Friend | Lilycove outing at 150 affection |
| Close | Mossdeep outing at 350 |
| Partner | Shrine confession at 600 |
| Spouse | Promise Ring at 700 (one spouse only) |
| Ascended | Shrine ceremony at 850 and Rank V: they become your God or Goddess |

Affection:

- **Talk:** +8, once a day.
- **Gift:** once a day. Their liked items give 15–60, anything polite gives +3. The game offers the best item you have.
- **Lounge battle:** +15, once a day, win or lose.
- **Outing:** +40.
- **Rank up:** +20 for everyone at Partner or above.
- **Decay:** −2 a day after 3 days without talking, never below the stage you already reached.

You can date several at once; each tracks their own affection. After you marry, the others sometimes say their "what could have been" line.

Spouse perks:

- They live in the Residence.
- They walk behind you and join every trainer battle as your partner (NPC follower system).
- Their partner team scales to your party level; it is tier 5 once ascended, otherwise the rank's NPC tier, capped at 3.

## Code

- `src/companion.c`, `include/companion.h`, `include/constants/companion.h`.
- `src/data/companion.h`: every candidate (sprite, trainer, partner, likes, lines per stage, outing scenes, proposal, "what could have been"), the thresholds, and the rumours.
- `data/scripts/companion.inc`: attendant, lounge, gifts, battles, outings, proposal, residence, follower.
- Maps: `data/maps/ChampionsLounge`, `data/maps/ChampionsResidence`. Objects were added to the League lobby (attendant), Lilycove, Mossdeep and the Shrine.
- Partner teams: `PARTNER_CYNTHIA`..`PARTNER_LANCE` in `src/data/battle_partners.party`. Steven uses `PARTNER_STEVEN_COMPANION`, so the story partner is unchanged.
- Item: `ITEM_PROMISE_RING` (875), Pearl String icon for now.
- Quest: "Hearts" (`QUEST_COMPANION`).
- Hooks (one line each): `Companion_DoDailyEvents` in `clock.c`; `Companion_OnPartnerPartyCreated` in `FillPartnerParty`; `Companion_OnRankUp`, `Companion_TryBufferRumour` and `Companion_IsVisitorStaying` in `ascension.c`; `special Special_Companion_SetupMap` in the Lilycove and Mossdeep on-transition scripts; the visitor win path in `ascension.inc`.

## Save

- **Breaks saves once.** `FNPC_ENABLE_NPC_FOLLOWERS` is now TRUE, which inserts follower data into SaveBlock3. `struct CompanionSave` (about 150 bytes) is appended to SaveBlock1. Start a new game on this branch.
- Flags 0x33–0x39 and 0x3B–0x3D hide objects. No vars are used.

## Not done / placeholders

- Cynthia uses the Cooltrainer F battle picture and overworld sprite until real Gen 3-style sprites exist. Swap `graphicsId` in `src/data/companion.h`, the `Pic:` in both party files, and `gAscensionVisitors`.
- There is no bag picker for gifts; the best gift you carry is offered. Dislikes aren't modelled.
- No jealousy mechanics. There is no garden at the Residence (the bedroom layout has none).

## Testing shortcuts

- Debug menu: set `FLAG_IS_CHAMPION` (0x87F) and walk into the League lobby. The attendant opens the Lounge.
- Affection lives in the save struct, not in vars. To test stages quickly, give gifts (Comet Shard is +60 for Cynthia) and advance the clock.
- To meet Cynthia: set `VAR_ASCENSION_VISITOR` (0x40B8) to 7 and flag 0x2E, re-enter the lobby, and beat her.
