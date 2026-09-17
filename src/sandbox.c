#include "global.h"
#include "sandbox.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "list_menu.h"
#include "main.h"
#include "map_name_popup.h"
#include "menu.h"
#include "money.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"

/*
 * Sandbox menu.
 *
 * State: one flag per toggle, FLAG_SANDBOX_* in include/constants/flags.h.
 * Trigger: Start menu -> SANDBOX (src/start_menu.c), and Sandbox_OnPlayerStep
 *          from ProcessPlayerFieldInput for the per-step effects.
 * Logic: the engine's own config hooks read the flags:
 *   B_FLAG_NO_WHITEOUT, OW_FLAG_NO_COLLISION, OW_FLAG_NO_TRAINER_SEE,
 *   WE_FLAG_NO_ENCOUNTER, I_EXP_SHARE_FLAG, FLAG_TEXT_SPEED_INSTANT,
 *   P_FLAG_FORCE_SHINY, OW_FLAG_POKE_RIDER (see include/config/).
 *   Infinite money and auto-heal are applied here on every step.
 * Presentation: a scrolling list in the overworld showing ON/OFF per row.
 */

enum SandboxRow
{
    ROW_NO_WHITEOUT,
    ROW_NOCLIP,
    ROW_NO_ENCOUNTERS,
    ROW_TRAINERS_BLIND,
    ROW_EXP_SHARE,
    ROW_INFINITE_MONEY,
    ROW_AUTO_HEAL,
    ROW_INSTANT_TEXT,
    ROW_ALWAYS_SHINY,
    ROW_FLY_ANYWHERE,
    ROW_TOGGLE_COUNT,
    ROW_ALL_ON = ROW_TOGGLE_COUNT,
    ROW_ALL_OFF,
    ROW_CLOSE,
    ROW_COUNT,
};

static const struct ListMenuItem sSandboxListItems[ROW_COUNT] =
{
    [ROW_NO_WHITEOUT]    = { COMPOUND_STRING("No Whiteout"),      ROW_NO_WHITEOUT },
    [ROW_NOCLIP]         = { COMPOUND_STRING("Walk Through Walls"), ROW_NOCLIP },
    [ROW_NO_ENCOUNTERS]  = { COMPOUND_STRING("No Wild Encounters"), ROW_NO_ENCOUNTERS },
    [ROW_TRAINERS_BLIND] = { COMPOUND_STRING("Trainers Blind"),   ROW_TRAINERS_BLIND },
    [ROW_EXP_SHARE]      = { COMPOUND_STRING("Full Exp. Share"),  ROW_EXP_SHARE },
    [ROW_INFINITE_MONEY] = { COMPOUND_STRING("Infinite Money"),   ROW_INFINITE_MONEY },
    [ROW_AUTO_HEAL]      = { COMPOUND_STRING("Auto-Heal"),        ROW_AUTO_HEAL },
    [ROW_INSTANT_TEXT]   = { COMPOUND_STRING("Instant Text"),     ROW_INSTANT_TEXT },
    [ROW_ALWAYS_SHINY]   = { COMPOUND_STRING("Always Shiny"),     ROW_ALWAYS_SHINY },
    [ROW_FLY_ANYWHERE]   = { COMPOUND_STRING("Fly From Map"),     ROW_FLY_ANYWHERE },
    [ROW_ALL_ON]         = { COMPOUND_STRING("All ON"),           ROW_ALL_ON },
    [ROW_ALL_OFF]        = { COMPOUND_STRING("All OFF"),          ROW_ALL_OFF },
    [ROW_CLOSE]          = { COMPOUND_STRING("Close"),            ROW_CLOSE },
};

static const u16 sSandboxRowFlags[ROW_TOGGLE_COUNT] =
{
    [ROW_NO_WHITEOUT]    = FLAG_SANDBOX_NO_WHITEOUT,
    [ROW_NOCLIP]         = FLAG_SANDBOX_NOCLIP,
    [ROW_NO_ENCOUNTERS]  = FLAG_SANDBOX_NO_ENCOUNTERS,
    [ROW_TRAINERS_BLIND] = FLAG_SANDBOX_TRAINERS_BLIND,
    [ROW_EXP_SHARE]      = FLAG_SANDBOX_EXP_SHARE,
    [ROW_INFINITE_MONEY] = FLAG_SANDBOX_INFINITE_MONEY,
    [ROW_AUTO_HEAL]      = FLAG_SANDBOX_AUTO_HEAL,
    [ROW_INSTANT_TEXT]   = FLAG_SANDBOX_INSTANT_TEXT,
    [ROW_ALWAYS_SHINY]   = FLAG_SANDBOX_ALWAYS_SHINY,
    [ROW_FLY_ANYWHERE]   = FLAG_SANDBOX_FLY_ANYWHERE,
};

#define SANDBOX_MENU_WIDTH  20
#define SANDBOX_MENU_ROWS   8
#define SANDBOX_STATUS_X    (SANDBOX_MENU_WIDTH * 8 - 30)

static const struct WindowTemplate sSandboxWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = SANDBOX_MENU_WIDTH,
    .height = 2 * SANDBOX_MENU_ROWS,
    .paletteNum = 15,
    .baseBlock = 1,
};

static const u8 sText_On[] = _("ON");
static const u8 sText_Off[] = _("OFF");

#define tMenuTaskId data[0]
#define tWindowId   data[1]

static void Task_SandboxMenuInput(u8 taskId);
static void PrintRowStatus(u8 windowId, u32 row, u8 y);
static void CloseSandboxMenu(u8 taskId);
static void ApplyToggleSideEffects(u32 row);

void Sandbox_ShowMenu(void)
{
    struct ListMenuTemplate menuTemplate = {0};
    u8 windowId, menuTaskId, inputTaskId;

    HideMapNamePopUpWindow();
    LoadMessageBoxAndBorderGfx();
    windowId = AddWindow(&sSandboxWindowTemplate);
    DrawStdWindowFrame(windowId, FALSE);
    CopyWindowToVram(windowId, COPYWIN_GFX);

    menuTemplate.items = sSandboxListItems;
    menuTemplate.moveCursorFunc = ListMenuDefaultCursorMoveFunc;
    menuTemplate.itemPrintFunc = PrintRowStatus;
    menuTemplate.totalItems = ROW_COUNT;
    menuTemplate.maxShowed = SANDBOX_MENU_ROWS;
    menuTemplate.windowId = windowId;
    menuTemplate.header_X = 0;
    menuTemplate.item_X = 8;
    menuTemplate.cursor_X = 0;
    menuTemplate.upText_Y = 1;
    menuTemplate.cursorPal = 2;
    menuTemplate.fillValue = 1;
    menuTemplate.cursorShadowPal = 3;
    menuTemplate.lettersSpacing = 1;
    menuTemplate.itemVerticalPadding = 0;
    menuTemplate.scrollMultiple = LIST_NO_MULTIPLE_SCROLL;
    menuTemplate.fontId = FONT_NORMAL;
    menuTemplate.cursorKind = 0;
    menuTaskId = ListMenuInit(&menuTemplate, 0, 0);

    inputTaskId = CreateTask(Task_SandboxMenuInput, 3);
    gTasks[inputTaskId].tMenuTaskId = menuTaskId;
    gTasks[inputTaskId].tWindowId = windowId;

    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void PrintRowStatus(u8 windowId, u32 row, u8 y)
{
    const u8 *status;

    if (row >= ROW_TOGGLE_COUNT)
        return;

    status = FlagGet(sSandboxRowFlags[row]) ? sText_On : sText_Off;
    AddTextPrinterParameterized(windowId, FONT_NORMAL, status, SANDBOX_STATUS_X, y, TEXT_SKIP_DRAW, NULL);
}

static void Task_SandboxMenuInput(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(gTasks[taskId].tMenuTaskId);

    if (input == LIST_CANCEL)
    {
        PlaySE(SE_SELECT);
        CloseSandboxMenu(taskId);
        return;
    }
    if (input == LIST_NOTHING_CHOSEN)
        return;

    switch (input)
    {
    case ROW_CLOSE:
        PlaySE(SE_SELECT);
        CloseSandboxMenu(taskId);
        return;
    case ROW_ALL_ON:
        PlaySE(SE_PC_LOGIN);
        Sandbox_SetAll(TRUE);
        break;
    case ROW_ALL_OFF:
        PlaySE(SE_PC_OFF);
        Sandbox_SetAll(FALSE);
        break;
    default:
        FlagToggle(sSandboxRowFlags[input]);
        PlaySE(FlagGet(sSandboxRowFlags[input]) ? SE_PC_LOGIN : SE_PC_OFF);
        ApplyToggleSideEffects(input);
        break;
    }
    RedrawListMenu(gTasks[taskId].tMenuTaskId);
}

static void CloseSandboxMenu(u8 taskId)
{
    DestroyListMenuTask(gTasks[taskId].tMenuTaskId, NULL, NULL);
    ClearStdWindowAndFrame(gTasks[taskId].tWindowId, TRUE);
    RemoveWindow(gTasks[taskId].tWindowId);
    DestroyTask(taskId);
    ScriptContext_Enable();
    UnfreezeObjectEvents();
}

static void ApplyToggleSideEffects(u32 row)
{
    switch (row)
    {
    case ROW_INFINITE_MONEY:
        if (FlagGet(FLAG_SANDBOX_INFINITE_MONEY))
            SetMoney(&gSaveBlock1Ptr->money, MAX_MONEY);
        break;
    case ROW_AUTO_HEAL:
        if (FlagGet(FLAG_SANDBOX_AUTO_HEAL))
            HealPlayerParty();
        break;
    }
}

void Sandbox_SetAll(bool32 enabled)
{
    u32 i;

    for (i = 0; i < ROW_TOGGLE_COUNT; i++)
    {
        if (enabled)
            FlagSet(sSandboxRowFlags[i]);
        else
            FlagClear(sSandboxRowFlags[i]);
        ApplyToggleSideEffects(i);
    }
}

// Called from ProcessPlayerFieldInput whenever the player takes a step.
void Sandbox_OnPlayerStep(void)
{
    if (FlagGet(FLAG_SANDBOX_AUTO_HEAL))
        HealPlayerParty();
    if (FlagGet(FLAG_SANDBOX_INFINITE_MONEY))
        SetMoney(&gSaveBlock1Ptr->money, MAX_MONEY);
}

#undef tMenuTaskId
#undef tWindowId
