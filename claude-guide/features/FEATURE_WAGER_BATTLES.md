# Feature: Wager battles

Branch: `feature/wager-battles` (from master). Status: implemented, builds, awaiting manual test.

## Goal

Before a trainer battle the player stakes money, a party Pokémon, or both, against a stake the NPC puts up. Win: take the NPC's money and Pokémon. Lose: the stake is gone. Reusable from any NPC script so the underground arena can be built on it later.

## Player-facing behaviour

1. NPC offers a wager and names their stake.
2. Menu: Money / Pokémon / Both / Nothing. Money is picked from preset amounts (500, 2000, 5000, 20000) with the money box shown. A Pokémon is picked from the party; eggs and the last Pokémon are refused.
3. Confirmation shows the full stake.
4. Battle. The player never whites out: on a loss the party is healed and the script continues.
5. Payout or loss messages. A won Pokémon goes to the party or PC.

Demo: a gentleman named LUCKY stands in Littleroot Town at (11, 14), stakes 3000 and a level 20 Absol, and can be challenged repeatedly.

## Design decisions

- **No-whiteout battle**: the engine's early-rival battle mode (`earlyRival` with `RIVAL_BATTLE_HEAL_AFTER`) already does exactly this: loss heals and returns to the script with `VAR_RESULT = TRUE`. The new `wagerbattle` macro in `asm/macros/event.inc` is a `trainerbattle` with those flags, `continueScript`, `facePlayer` and `skipFlagCheck` set, so the same trainer can be fought again. This avoids touching battle code.
- **Settlement after the battle** rather than escrow before it, because the battle can no longer end in a whiteout that would abandon the script.
- **State in five saved vars** (renamed from unused ones). Money stakes are capped at 65535 by the var width. Larger stakes would need two vars; not needed yet.
- **Opponent stake is data in vars** set by the calling script, not tied to the trainer's real party, so any trainer can be wagered against and the prize Pokémon is generated with `ScriptGiveMon`.
- **Player's staked Pokémon is removed with `ZeroMonData` + `CompactPartySlots`** on a loss. It is gone for good; a future feature could have the NPC keep it for buy-back.
- Five specials in `src/wager.c`, all text and flow in `data/scripts/wager.inc`.

## Files

- `include/constants/vars.h`: five var renames.
- `include/constants/wager.h`, `include/wager.h`, `src/wager.c`: new.
- `data/scripts/wager.inc`: new; included from `data/event_scripts.s`, which also includes the new constants header.
- `data/specials.inc`: five entries.
- `asm/macros/event.inc`: `wagerbattle` macro.
- `include/constants/opponents.h`: `TRAINER_WAGER_DEMO` (855), count 856.
- `src/data/trainers.party`: LUCKY.
- `data/maps/LittlerootTown/map.json`: bookie object event.

## Save impact

None. Five previously unused vars and one trainer flag slot (there were 9 spare).

## How to test

1. New game, get past the truck. LUCKY stands south of the lab area in Littleroot at (11, 14).
2. Stake 500 with a level 5 starter and lose: money drops by 500, party healed, script continues.
3. Stake a Pokémon and lose: it is removed from the party.
4. Give yourself a strong Pokémon (debug menu) and win: +3000 and an Absol in the party or PC.
5. Try to stake an egg or your only Pokémon: refused.
6. Pick Nothing or say no: no battle.

## Follow-ups

- Underground arena uses this with tiers of NPCs and the underground currency instead of money (add a currency selector to `Wager_Settle`).
- Losing a staked Pokémon could hand it to the NPC's pool for buy-back.
