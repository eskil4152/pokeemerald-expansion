# Feature: Quest system

Branch: `feature/quests` (from master). Status: implemented, builds, awaiting manual test.

## Goal

A foundation for quests and, later, the ascension tiers: a ROM table of quests, one saved stage var per quest, script commands to start, advance, complete and query quests, and a Journal screen from the Start menu that lists active, completed and locked quests with their current stage text.

## Player-facing behaviour

- Start → JOURNAL opens a scrolling list: Active, Completed, Locked sections. Selecting a quest shows its description and the text of its current stage.
- Quests are started and advanced by NPCs and events; the journal only reads.
- Locked quests show their unlock requirement (for ascension tiers).

## Design

- `struct Quest { const u8 *name; const u8 *description; const u8 *const *stageTexts; u8 stageCount; u16 stageVar; u16 unlockFlag; }` in `src/data/quests.h`, ids in `include/constants/quests.h`.
- Per quest one var: 0 = not started, 1..n = stage, `QUEST_STAGE_DONE` (0xFF) = completed. Vars come from the unused pool; the arena and economy branches must not reuse the same ones (allocation list kept in this file).
- Script commands via specials: `startquest`, `advancequest`, `setqueststage`, `completequest`, `checkquest` (stage into `VAR_RESULT`), each a small macro wrapping `special` with `VAR_0x8004` arguments in `asm/macros/event.inc`.
- Journal screen: full-screen list built like `src/diploma.c` with a `ListMenuTemplate`, in `src/quest_log.c`. Start menu entry `MENU_ACTION_JOURNAL` shown once any quest has started (`FLAG_SYS_JOURNAL_GET`).
- Optional later: reward on completion, town-map markers, counters ("beat 10 trainers") using a second var per quest.

## Files (planned)

`include/constants/quests.h`, `src/data/quests.h`, `src/quest.c`, `include/quest.h`, `src/quest_log.c`, `data/specials.inc`, `asm/macros/event.inc`, `src/start_menu.c`, `include/constants/vars.h` (var renames), `include/constants/flags.h` (one flag).

## Save impact

None beyond consuming unused vars. If more than ~20 quests are needed, a `u8 questStage[QUEST_COUNT]` array in SaveBlock1 replaces the vars (save-breaking).

## How to test (once built)

Start a demo quest from an NPC, check the journal, advance it, complete it, confirm the journal moves it to Completed.

## Implementation notes

- State: `u8 questStages[MAX_QUESTS]` (32 bytes) appended to `SaveBlock1` instead of vars, so quests scale. **Save-breaking**: delete the `.sav` when switching to this branch.
- `src/quest.c`: `Quest_Start/Advance/SetStage/Complete/GetStage`, lock and visibility rules, five specials. Starting any quest sets `FLAG_SYS_JOURNAL_GET` (renamed from `FLAG_UNUSED_0x03A`), which adds JOURNAL to the Start menu.
- Script macros in `asm/macros/event.inc`: `startquest`, `advancequest` (past the last stage completes), `setqueststage`, `completequest`, `checkquest` (stage into `VAR_RESULT`).
- `src/data/quests.h`: the quest table. A quest with an `unlockFlag` is listed as Locked with its `lockedText` until the flag is set; that is the hook for ascension tiers. `QUEST_ASCENSION_I` is included as a locked example gated on `FLAG_IS_CHAMPION`.
- `src/quest_log.c`: Journal screen, list with a status column (Active, Done, Locked, Available) and a detail box with the description and the current stage text. Returns to the field with the Start menu open.
- Demo quest "Meet the Neighbours" in Littleroot: the boy near the lab starts it, the man who talks about the PC advances it, the boy completes it and gives a Rare Candy.

## Files

`include/constants/quests.h`, `src/data/quests.h`, `include/quest.h`, `src/quest.c`, `include/quest_log.h`, `src/quest_log.c`, `include/global.h`, `src/new_game.c`, `src/start_menu.c`, `asm/macros/event.inc`, `data/specials.inc`, `data/event_scripts.s`, `include/constants/flags.h`, `data/maps/LittlerootTown/scripts.inc`.

## How to test

1. Delete the old `.sav`, new game, reach Littleroot.
2. Talk to the boy (wanders south of the lab): "The JOURNAL was updated!". Start menu now shows JOURNAL.
3. JOURNAL: "Meet the Neighbours" is Active; detail shows stage 1. "Ascension I" shows Locked with its hint.
4. Talk to the man near the houses: journal updates to stage 2.
5. Talk to the boy: quest completes, Rare Candy. Journal shows Done.
6. Debug menu → set `FLAG_IS_CHAMPION`: Ascension I becomes Available.
