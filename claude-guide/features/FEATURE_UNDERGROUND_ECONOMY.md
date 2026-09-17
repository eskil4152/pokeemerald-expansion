# Feature: Underground economy

Branch: `feature/underground-economy` (from master). Status: planned, not started. Currency and arena names are placeholders (`UCOIN`).

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
