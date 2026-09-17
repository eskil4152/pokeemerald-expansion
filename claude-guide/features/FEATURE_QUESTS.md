# Feature: Quest system

Branch: `feature/quests` (from master). Status: planned, not started.

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
