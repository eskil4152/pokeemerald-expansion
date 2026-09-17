# 07. Recipes: mechanics Pokémon does not have

The engine has no concept of "quest", "faction", "shop economy", "job", "ownership", "reputation" or whatever you want next. But every such thing decomposes into four parts, and the engine has primitives for each:

| Part | Primitive (see `05-engine-systems.md`) |
|---|---|
| **State**: what is remembered | flags and vars first; a field in a save block if bigger |
| **Trigger**: when it happens | an NPC script, a map trigger tile, a per-step or per-frame hook, a per-day hook, a battle-end hook, a menu entry, a button |
| **Logic**: what changes | C in a new `src/mything.c`, exposed through a `special` or `callnative` |
| **Presentation**: what the player sees | script text with buffers, a message box, a menu, a custom screen, sprites, sounds |

Pick the simplest option in each row that works. Most mechanics need no custom screen and no save struct change.

The worked patterns below go from trivial to large. Each says which primitives it composes, where the code goes, and how to test it.

## Pattern A: a new resource or currency

Example: "Guild Points" earned by beating trainers, spent at a vendor.

**State**: one var. Rename `VAR_UNUSED_0x404E` to `VAR_GUILD_POINTS` in `include/constants/vars.h`. A var holds 0..65535; if you need more, use two vars or a `u32` in `SaveBlock1`.

**Logic** (`src/guild.c`, `include/guild.h`):

```c
#include "global.h"
#include "event_data.h"
#include "guild.h"

void AddGuildPoints(u32 amount)
{
    u32 points = VarGet(VAR_GUILD_POINTS) + amount;
    if (points > MAX_GUILD_POINTS) points = MAX_GUILD_POINTS;
    VarSet(VAR_GUILD_POINTS, points);
}

bool32 TrySpendGuildPoints(u32 amount)
{
    if (VarGet(VAR_GUILD_POINTS) < amount) return FALSE;
    VarSet(VAR_GUILD_POINTS, VarGet(VAR_GUILD_POINTS) - amount);
    return TRUE;
}

// Specials for scripts. Argument in VAR_0x8004, result in VAR_RESULT.
void Special_TrySpendGuildPoints(void) { gSpecialVar_Result = TrySpendGuildPoints(gSpecialVar_0x8004); }
u16 Special_GetGuildPoints(void)      { return VarGet(VAR_GUILD_POINTS); }
```

Register in `data/specials.inc`: `def_special Special_TrySpendGuildPoints` and `def_special Special_GetGuildPoints`.

**Trigger**: earn on trainer win. In `HandleEndTurn_BattleWon` (`src/battle_main.c`) inside the `BATTLE_TYPE_TRAINER` branch add `AddGuildPoints(GetTrainerPartySizeFromId(TRAINER_BATTLE_PARAM.opponentA) * 2);`. Or purely in scripts: put `special` calls in the post-battle script of specific trainers.

**Presentation**: a vendor script:

```
Guild_EventScript_Vendor::
	lock
	faceplayer
	specialvar VAR_RESULT, Special_GetGuildPoints
	buffernumberstring STR_VAR_1, VAR_RESULT
	msgbox Guild_Text_YouHave, MSGBOX_DEFAULT        @ "You have {STR_VAR_1} points."
	dynmultichoice 0, 0, FALSE, 3, 0, DYN_MULTICHOICE_CB_NONE, Guild_Text_OptRareCandy, Guild_Text_OptNothing
	switch VAR_RESULT
	case 0, Guild_EventScript_BuyCandy
	release
	end

Guild_EventScript_BuyCandy:
	setvar VAR_0x8004, 10
	special Special_TrySpendGuildPoints
	goto_if_eq VAR_RESULT, FALSE, Guild_EventScript_TooPoor
	giveitem ITEM_RARE_CANDY
	release
	end
```

To show the balance permanently, a Start menu entry or a small window drawn from a task; `DrawMoneyBox` in `src/money.c` shows how the money window is drawn and can be copied for a second currency.

**Test**: Debug menu → Flags & Vars → set `VAR_GUILD_POINTS`, then talk to the vendor.

## Pattern B: a persistent world state that changes the map

Example: a town that grows through three stages as the player invests, with different NPCs and buildings each stage.

**State**: a var `VAR_TOWN_STAGE` (0, 1, 2).

**Trigger and presentation** (all in map data, no C):

- NPCs that exist only at a stage: give each object event a `flag`, and in the map's `ON_TRANSITION` script set or clear those flags from the var:

```
MyTown_OnTransition:
	call_if_eq VAR_TOWN_STAGE, 0, MyTown_EventScript_Stage0Flags
	call_if_eq VAR_TOWN_STAGE, 1, MyTown_EventScript_Stage1Flags
	end
MyTown_EventScript_Stage0Flags:
	setflag FLAG_HIDE_MYTOWN_SHOPKEEPER
	clearflag FLAG_HIDE_MYTOWN_BUILDER
	return
```

- Buildings that appear: `ON_LOAD` script with `setmetatile x, y, METATILE_Petalburg_Building..., TRUE` lines per stage (labels in `include/constants/metatile_labels.h`; Porymap shows metatile ids). Or use `setmaplayoutindex LAYOUT_MYTOWN_STAGE2` in `ON_TRANSITION` to swap the whole layout, which is cleaner for large changes: make one layout per stage.
- Warps into a building that only exists later: the warp tile is just a metatile with warp behaviour; place it in the stage-2 layout only.
- Dialogue that changes: `switch VAR_TOWN_STAGE` in the NPC scripts.

**Logic**: the "invest" NPC does `checkmoney`, `removemoney`, `addvar VAR_TOWN_STAGE, 1`, then `warp MAP_MY_TOWN, x, y` or `fadescreen` + `special` that calls `DrawWholeMapView()` after `setmetatile` to refresh in place.

This is exactly how vanilla does Trick House rooms, the Sootopolis story states and the Mossdeep space center. The Sootopolis `scripts.inc` is a full example.

## Pattern C: a global rule or modifier (god mode, hard mode, challenge modes)

Example: a "god mode" toggle: no whiteouts, walk through walls, no random encounters, trainers never spot you, always max money, party healed every step.

**State**: one flag `FLAG_GOD_MODE` (rename an unused one).

**Logic**: most of it already exists as config hooks. Set in `include/config/`:

```c
#define B_FLAG_NO_WHITEOUT        FLAG_GOD_MODE   // config/battle.h
#define OW_FLAG_NO_COLLISION      FLAG_GOD_MODE   // config/overworld.h
#define OW_FLAG_NO_TRAINER_SEE    FLAG_GOD_MODE   // config/overworld.h
#define WE_FLAG_NO_ENCOUNTER      FLAG_GOD_MODE   // config/wild_encounter.h
#define I_EXP_SHARE_FLAG          FLAG_GOD_MODE   // config/item.h
```

The same flag can drive several configs. For the parts with no config, add a per-step hook in `ProcessPlayerFieldInput` (`src/field_control_avatar.c`):

```c
if (input->tookStep && FlagGet(FLAG_GOD_MODE))
{
    HealPlayerParty();
    SetMoney(&gSaveBlock1Ptr->money, MAX_MONEY);
}
```

**Trigger**: a toggle. Cheapest: Debug menu → Flags. Player-facing: a key item (`ITEM_GOD_STONE`, `.type = ITEM_USE_FIELD`, `fieldUseFunc` that does `FlagToggle(FLAG_GOD_MODE)` and shows a message), registrable to Select. Or a Start menu entry. Or an options-menu row (`src/option_menu.c` has the table of rows; add one and store it in a flag).

Other rules follow the same shape. "Nuzlocke": on `HandleEndTurn_BattleWon`/`FinishBattle`, walk the party and release any Pokémon with 0 HP (`ZeroMonData` + `CompactPartySlots`), and set a per-map flag on first catch to block `Cmd_handleballthrow`. "Level scaling": before a trainer battle in `CreateNPCTrainerPartyFromTrainer` (`src/battle_setup.c`), overwrite `.lvl` with a function of the player's highest level. "Permadeath for the player": on `HandleEndTurn_BattleLost`, wipe the save with `ClearSaveData()` and `DoSoftReset()`.

## Pattern D: a new NPC interaction type (jobs, ownership, relationships, taxes, bribes)

Example: NPCs can be "hired": each hireable NPC has a wage, hired NPCs give a daily payout, and the player can fire them.

**State**: per-NPC data. If there are at most 16 hireable NPCs, one flag each (`FLAG_HIRED_NPC_0`..`15`) is enough. If you need a count or level per NPC, a var each, or an array in `SaveBlock1`:

```c
// include/global.h, inside struct SaveBlock1, at the end
u8 hiredNpcLevel[NUM_HIREABLE_NPCS];   // 0 = not hired
```

**Logic** (`src/hiring.c`): `HireNpc(id, wage)`, `FireNpc(id)`, `IsNpcHired(id)`, `PayDailyWages(void)`. Expose with specials that take the NPC id in `VAR_0x8004`.

**Trigger**: the NPC script asks and calls the special. The daily payout hooks into `UpdatePerDay` in `src/clock.c`: `PayDailyWages(days)` adds `days × sum of wages` to money. If you want the player to collect in person instead, store an accumulator var and have a bank NPC hand it out.

**Presentation**: the NPC's script with `MSGBOX_YESNO`; a "ledger" screen if you want, built like `src/diploma.c` with a list of names and wages printed from the save array.

**Generalisation**: any relationship between the player and a set of things (NPCs, houses, Pokémon, towns, factions) is "an array indexed by thing id in the save, plus a special that reads and writes it, plus scripts that call the special". The vanilla Secret Base and Berry Tree systems are exactly this shape (`gSaveBlock1Ptr->berryTrees[]` indexed by tree id, `src/berry.c`, `data/scripts/berry_tree.inc`) and are good templates to read.

## Pattern E: a new overworld system (something that happens on the map on its own)

Example: roaming merchants that move between towns each day, or weather that damages the party, or a stamina bar that drains as you run.

Three shapes:

1. **Per day**: `UpdatePerDay` in `src/clock.c`. Compute new positions into vars; the maps' `ON_TRANSITION` scripts show or hide the merchant object event by flag according to the var. Vanilla roamers (`src/roamer.c`) are this with the extra twist of moving on map change.
2. **Per step**: `ProcessPlayerFieldInput` under `input->tookStep`. Vanilla poison damage (`UpdatePoisonStepCounter`) and egg hatching are the models: keep a counter in a var, act every N steps, and run a script with `ScriptContext_SetupScript(EventScript_X)` if you need text or a fade.
3. **Per frame**: a task created when the map loads. `src/field_tasks.c` has the per-map step callback system (`SetUpFieldTasks`, used for Cycling Road and the Sootopolis ice puzzle) and is the sanctioned way to run per-frame map logic. A stamina bar that drains while running: a task that checks `TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_DASH)` each frame, decrements a var, and when it hits zero clears `FLAG_SYS_B_DASH` so running stops. Draw the bar with a small window or sprite updated by the same task.

Persistence for shapes 2 and 3 is the var they count in. Reset it in `NewGameInitData` if it must not start at zero.

## Pattern F: a new screen (journal, quest log, faction board, crafting)

Copy `src/diploma.c` into `src/questlog.c`. Change the graphics to a plain window frame, and in `DisplayDiplomaText`'s place print a list built from your save data:

```c
static void PrintQuests(void)
{
    u32 i, y = 1;
    for (i = 0; i < NUM_QUESTS; i++)
    {
        if (!FlagGet(sQuests[i].startedFlag)) continue;
        StringExpandPlaceholders(gStringVar4, sQuests[i].title);
        AddTextPrinterParameterized(0, FONT_NORMAL, gStringVar4, 4, y * 16, TEXT_SKIP_DRAW, NULL);
        if (FlagGet(sQuests[i].doneFlag))
            AddTextPrinterParameterized(0, FONT_SMALL, gText_Done, 160, y * 16, TEXT_SKIP_DRAW, NULL);
        y++;
    }
    PutWindowTilemap(0);
    CopyWindowToVram(0, COPYWIN_FULL);
}
```

Quest data is a `static const struct Quest sQuests[]` table in ROM with a title string and two flags per quest; quests are started and completed from any script with `setflag`. Open the screen from the Start menu (add `MENU_ACTION_QUESTS` to `src/start_menu.c`, callback that fades then `SetMainCallback2(CB2_ShowQuestLog)`) or from a key item. Use a `ListMenuTemplate` if the list should scroll; `src/debug.c` shows list menus with a description pane.

Crafting is the same screen plus a confirm step: choose a recipe row, `CheckBagHasItem` on each ingredient, `RemoveBagItem` them, `AddBagItem` the product, print the result.

## Pattern G: a new battle rule or format

Example: "wager battles" where the player bets money before a trainer battle and wins double or loses it; or "team size 3" battles; or a battle where the field damages everyone each turn.

- **Pre-battle choices**: script before `trainerbattle_*`: `checkmoney`, store the wager in a var, `removemoney`. After the battle the trainer's post-battle script (the `event_script` argument of `trainerbattle_single`) reads `VAR_RESULT` (battle outcome) and `addmoney` accordingly. No C needed.
- **Party restrictions**: `ReducePlayerPartyToSelectedMons` and the party selection UI used by the Battle Frontier (`special ChooseHalfPartyForBattle`, `src/party_menu.c` `PARTY_MENU_TYPE_CHOOSE_HALF`) let the player pick a subset; `SavePlayerParty` / `LoadPlayerParty` restore afterwards. See the Battle Tent scripts for the sequence.
- **New rule inside the battle**: a flag checked at the right point. For "everyone takes 1/16 damage each turn": add an entry to the end-turn effect list in `src/battle_end_turn.c` guarded by `FlagGet(FLAG_HAZARD_FIELD)`, calling `BattleScriptExecute(BattleScript_HazardDamage)` with a small script that prints a string and `datahpupdate`s each battler. Copy the sandstorm end-turn damage as the template.
- **Different win condition**: normally `Cmd_checkteamslost` (`src/battle_script_commands.c`), run from the faint-handling battle scripts, sets `gBattleOutcome` to `B_OUTCOME_WON` or `B_OUTCOME_LOST` when a side has no usable Pokémon. A "survive 5 turns" mode can set `gBattleOutcome |= B_OUTCOME_WON` from `BattleTurnPassed` when `gBattleResults.battleTurnCounter >= 5` and the flag is on; `BattleDebug_WonBattle` in `src/battle_main.c` shows the two lines that end a battle from C.
- **New battle type bit**: if setup itself must differ (different intro, no run option), use `BATTLE_TYPE_14` or `BATTLE_TYPE_30` and check it where `BATTLE_TYPE_SAFARI` is checked, since Safari is the vanilla example of a totally different battle format.
- **AI vs AI**: `B_FLAG_AI_VS_AI_BATTLE` already exists for spectated battles.

Battle tests (`08-testing-and-debugging.md`) can set flags with `FLAG_SET(FLAG_X)` in `GIVEN`, so every rule gets a regression test.

## Pattern H: player-driven creation (name your own NPC, build a house, breed custom Pokémon)

The naming screen is reusable: `DoNamingScreen(NAMING_SCREEN_X, dest, ...)` in `src/naming_screen.c` writes a string into a buffer you supply, and the vanilla player name and box names are stored in the save. Store player-authored strings in a save array (`u8 customNames[N][PLAYER_NAME_LENGTH + 1]`) and show them through `StringCopy(gStringVar1, ...)` + `{STR_VAR_1}`.

Decorations (`src/decoration.c`, `src/secret_base.c`) are the vanilla "place things on a map" system: a list of placed items with positions in the save, drawn by setting metatiles and spawning object events on map load. Extending it to a player house on any map is mostly data: new decoration entries in `src/data/decoration/` and a new "placeable area" map with the `SECRET_BASE` bg events.

## Pattern I: things the engine cannot do cheaply

Know these so you do not fight them:

- **Free-form text input** beyond 10 to 12 characters: the naming screen is the only keyboard.
- **More than 16 NPCs on screen**: `OBJECT_EVENTS_COUNT` is 16, including the player. Crowds need sprites without object events, or spawn/despawn tricks.
- **Real-time multiplayer**: link code exists but only for the vanilla link features.
- **Large data in the save**: the total is about 57 KB, of which ~52 KB is used. Free vanilla data with `config/save.h` or the Frontier before adding kilobytes.
- **Floating point**: none in hardware. Use fixed point (`Q_4_12` helpers in `include/fpmath.h`) or integer math.
- **Big new UI**: every screen is hand-built from tiles and windows. Budget a day for a polished one; a text-only list is an hour.

## Checklist for any new mechanic

1. Write the state as flags/vars/save fields and list them in a comment at the top of your `src/mything.c`.
2. Write the C logic as pure functions on that state.
3. Expose one or two specials.
4. Write the scripts and text.
5. Hook the triggers.
6. Add a Debug menu script slot (`data/scripts/debug.inc` has `Debug_EventScript_Script_1`..`8`, reachable from Debug → Scripts) that exercises it, so you can test without playing to it.
7. Build, run, test, then write a test if it is battle-side.
