# Feature: Underground economy

Branch: `feature/underground-economy` (from master). Status: implemented, builds, awaiting manual test. Currency name is a placeholder (`UCOIN`), changed in text only.

## Goal

A second currency the legitimate economy never touches, earned only through illegal activity and spent only in the underground: buying Pokémon the normal game gates, buying workers who produce daily income, black-market items, bribes. A heat value that rises with illegal activity and has consequences.

## Player-facing behaviour

- UCOIN balance shown by underground NPCs and in the Sandbox/Journal screens.
- Earned by: arena wins, selling Pokémon to a fence, daily payouts from owned workers.
- Spent at: a fence (buy Pokémon), a worker broker (buy workers), a black-market shop (items), bribes in scripts.
- Heat rises with each illegal act, decays daily; at thresholds, scripted consequences (police NPCs appear, prices rise, arena closes for a day).

## Design

- State: `u32 ucoins`, `u8 heat`, `u8 workers[NUM_WORKER_TYPES]` as new fields at the end of `SaveBlock1` (save-breaking, one time). Initialised in `NewGameInitData`.
- `src/underground.c` with specials: `AddUcoins`, `TrySpendUcoins`, `BufferUcoins`, `SellPartyMonToFence` (value from base stats and level), `BuyWorker`, `CollectWorkerPayout`, `AddHeat`, `GetHeat`.
- Daily hook in `UpdatePerDay` (`src/clock.c`): accumulate worker payouts into a pending var, decay heat.
- Black-market shop: `pokemart` with `shopCriteriaFunc` gating is money-based; the UCOIN shop is a script `dynmultichoice` list backed by `TrySpendUcoins`, or a small currency parameter added to `src/shop.c` later.
- Wager battles gain a currency selector so the arena pays in UCOIN.

## Files (planned)

`include/global.h` (save fields), `src/new_game.c`, `src/underground.c`, `include/underground.h`, `data/specials.inc`, `data/scripts/underground.inc`, `src/clock.c`, later `src/wager.c`.

## Save impact

Yes: new SaveBlock1 fields. Do this on a branch that also lands any other save-breaking change so saves are reset once.

## Open decisions

Currency and arena names, worker types and their yields, heat thresholds and consequences, sell-price formula.

## Implementation notes

- State: `struct Underground { u32 ucoins; u32 pendingPayout; u8 heat; u8 workers[3]; }` appended to `SaveBlock1` (12 bytes). **Save-breaking**: delete the `.sav` when switching to this branch.
- `src/underground.c`: pure logic plus thirteen specials (`Special_Underground_*`). Sell value = base stat total × level / 10, ×3 for legendaries and mythicals, ×2 for shinies, minimum 10. Eggs and the last party member cannot be sold.
- Workers: Pickpocket 200u → 20/day, Smuggler 600u → 70/day, Fence 1500u → 200/day, up to 99 each. Income accrues into `pendingPayout` from `DoDailyEvents` (`src/clock.c`) and is collected from the broker.
- Heat: selling +15, buying a Pokémon +5, hiring +10, collecting +5; decays 10 per day; at 80 or more both NPCs refuse to deal.
- Demo NPCs in Littleroot Town: the fence (MAN_3 sprite) at (8, 14) with sell / buy (Dratini 500u, Beldum 800u, Larvitar 800u, level 20) / balance; the broker (OLD_MAN sprite) at (17, 14) with hire / collect / balance.

## Files

`include/global.h`, `include/constants/underground.h`, `include/underground.h`, `src/underground.c`, `data/scripts/underground.inc`, `data/event_scripts.s`, `data/specials.inc`, `src/new_game.c`, `src/clock.c`, `data/maps/LittlerootTown/map.json`.

## How to test

1. Delete the old `.sav`, new game, reach Littleroot.
2. Fence → Sell a Pokémon: pick one; balance rises by the quoted value. Try selling your last Pokémon or an egg: refused.
3. Fence → Buy: with under 500u, "can't cover that"; sell something first, then buy a Dratini.
4. Broker → Hire a Pickpocket; Check balance shows heat rising.
5. Change the day (debug menu → Utilities → Time, or set the clock forward) and Collect earnings.
6. Repeat sells until heat is 80 or more: both NPCs refuse; advance days and they deal again.
