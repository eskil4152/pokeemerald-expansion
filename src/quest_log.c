#include "global.h"
#include "quest_log.h"
#include "quest.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "list_menu.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

/*
 * Journal screen.
 *
 * Header, a scrolling list of visible quests with a status column, and a
 * detail box that follows the cursor showing the description and the current
 * stage text (or the unlock hint while locked). Built like src/diploma.c and
 * src/pokemon_editor.c: one background, windows, one task.
 */

enum JournalWindow
{
    WIN_HEADER,
    WIN_LIST,
    WIN_DETAIL,
};

#define LIST_ROWS   5
#define STATUS_X    160
#define EMPTY_ID    (-1)

struct QuestLog
{
    struct ListMenuItem *items;
    u32 itemCount;
    u8 listTaskId;
    u8 taskId;
};

static EWRAM_DATA struct QuestLog *sLog = NULL;

static const u8 sTextColors[3] = { TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY };

static const struct BgTemplate sBgTemplates[] =
{
    { .bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0 },
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_HEADER] = { .bg = 0, .tilemapLeft = 1, .tilemapTop = 1,  .width = 28, .height = 2,  .paletteNum = 15, .baseBlock = 1 },
    [WIN_LIST]   = { .bg = 0, .tilemapLeft = 1, .tilemapTop = 4,  .width = 28, .height = 10, .paletteNum = 15, .baseBlock = 57 },
    [WIN_DETAIL] = { .bg = 0, .tilemapLeft = 1, .tilemapTop = 15, .width = 28, .height = 4,  .paletteNum = 15, .baseBlock = 337 },
    DUMMY_WIN_TEMPLATE,
};

static const u8 sText_Journal[] = _("JOURNAL");
static const u8 sText_Active[] = _("Active");
static const u8 sText_Done[] = _("Done");
static const u8 sText_Locked[] = _("Locked");
static const u8 sText_Available[] = _("Available");
static const u8 sText_NoQuests[] = _("No quests yet.");
static const u8 sText_NoQuestsDetail[] = _("Talk to people. Someone always needs something.");

static void CB2_QuestLogMain(void);
static void VBlankCB_QuestLog(void);
static void Task_QuestLogFadeIn(u8 taskId);
static void Task_QuestLogInput(u8 taskId);
static void Task_QuestLogExit(u8 taskId);
static void BuildQuestList(void);
static void PrintQuestStatus(u8 windowId, u32 questId, u8 y);
static void OnCursorMove(s32 questId, bool8 onInit, struct ListMenu *list);
static void PrintDetail(s32 questId);

static void VBlankCB_QuestLog(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_QuestLogMain(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

void CB2_ShowQuestLog(void)
{
    struct ListMenuTemplate template = {0};

    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BG3CNT, 0);
    SetGpuReg(REG_OFFSET_BG2CNT, 0);
    SetGpuReg(REG_OFFSET_BG1CNT, 0);
    SetGpuReg(REG_OFFSET_BG0CNT, 0);
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);
    ScanlineEffect_Stop();
    ResetTasks();
    ResetSpriteData();
    ResetPaletteFade();
    FreeAllSpritePalettes();

    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);

    InitWindows(sWindowTemplates);
    DeactivateAllTextPrinters();
    LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    LoadMessageBoxAndBorderGfx();
    FillPalette(RGB(6, 4, 2), BG_PLTT_ID(0), PLTT_SIZEOF(1));

    sLog = AllocZeroed(sizeof(*sLog));
    BuildQuestList();

    PutWindowTilemap(WIN_HEADER);
    PutWindowTilemap(WIN_LIST);
    PutWindowTilemap(WIN_DETAIL);
    DrawStdWindowFrame(WIN_HEADER, FALSE);
    DrawStdWindowFrame(WIN_LIST, FALSE);
    DrawStdWindowFrame(WIN_DETAIL, FALSE);

    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
    AddTextPrinterParameterized4(WIN_HEADER, FONT_NORMAL, 4, 0, 0, 0, sTextColors, TEXT_SKIP_DRAW, sText_Journal);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);

    template.items = sLog->items;
    template.moveCursorFunc = OnCursorMove;
    template.itemPrintFunc = PrintQuestStatus;
    template.totalItems = sLog->itemCount;
    template.maxShowed = LIST_ROWS;
    template.windowId = WIN_LIST;
    template.header_X = 0;
    template.item_X = 8;
    template.cursor_X = 0;
    template.upText_Y = 1;
    template.cursorPal = 2;
    template.fillValue = 1;
    template.cursorShadowPal = 3;
    template.lettersSpacing = 1;
    template.itemVerticalPadding = 0;
    template.scrollMultiple = LIST_MULTIPLE_SCROLL_L_R;
    template.fontId = FONT_NORMAL;
    template.cursorKind = 0;
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    sLog->listTaskId = ListMenuInit(&template, 0, 0);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);

    BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    EnableInterrupts(1);
    SetVBlankCallback(VBlankCB_QuestLog);
    SetMainCallback2(CB2_QuestLogMain);
    sLog->taskId = CreateTask(Task_QuestLogFadeIn, 0);
}

static void BuildQuestList(void)
{
    u32 i;

    sLog->items = AllocZeroed((QUEST_COUNT + 1) * sizeof(struct ListMenuItem));
    sLog->itemCount = 0;
    for (i = 0; i < QUEST_COUNT; i++)
    {
        if (!Quest_IsVisibleInJournal(i))
            continue;
        sLog->items[sLog->itemCount].name = gQuests[i].name;
        sLog->items[sLog->itemCount].id = i;
        sLog->itemCount++;
    }
    if (sLog->itemCount == 0)
    {
        sLog->items[0].name = sText_NoQuests;
        sLog->items[0].id = EMPTY_ID;
        sLog->itemCount = 1;
    }
}

static void PrintQuestStatus(u8 windowId, u32 questId, u8 y)
{
    const u8 *status;

    if (questId == (u32)EMPTY_ID)
        return;
    if (Quest_IsDone(questId))
        status = sText_Done;
    else if (Quest_IsActive(questId))
        status = sText_Active;
    else if (Quest_IsLocked(questId))
        status = sText_Locked;
    else
        status = sText_Available;
    AddTextPrinterParameterized4(windowId, FONT_NORMAL, STATUS_X, y, 0, 0, sTextColors, TEXT_SKIP_DRAW, status);
}

static void OnCursorMove(s32 questId, bool8 onInit, struct ListMenu *list)
{
    if (!onInit)
        PlaySE(SE_SELECT);
    PrintDetail(questId);
}

static void PrintDetail(s32 questId)
{
    const u8 *line1;
    const u8 *line2 = NULL;

    if (questId == EMPTY_ID)
    {
        line1 = sText_NoQuestsDetail;
    }
    else
    {
        const struct Quest *quest = &gQuests[questId];
        u32 stage = Quest_GetStage(questId);

        line1 = quest->description;
        if (Quest_IsLocked(questId))
            line2 = quest->lockedText;
        else if (stage >= 1 && stage <= quest->stageCount)
            line2 = quest->stageTexts[stage - 1];
        else if (stage == QUEST_STAGE_DONE)
            line2 = sText_Done;
    }

    FillWindowPixelBuffer(WIN_DETAIL, PIXEL_FILL(1));
    AddTextPrinterParameterized4(WIN_DETAIL, FONT_SMALL, 2, 0, 0, 0, sTextColors, TEXT_SKIP_DRAW, line1);
    if (line2 != NULL)
        AddTextPrinterParameterized4(WIN_DETAIL, FONT_SMALL, 2, 16, 0, 0, sTextColors, TEXT_SKIP_DRAW, line2);
    CopyWindowToVram(WIN_DETAIL, COPYWIN_FULL);
}

static void Task_QuestLogFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_QuestLogInput;
}

static void Task_QuestLogInput(u8 taskId)
{
    s32 input = ListMenu_ProcessInput(sLog->listTaskId);

    if (input == LIST_CANCEL || input != LIST_NOTHING_CHOSEN)
    {
        PlaySE(SE_SELECT);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_QuestLogExit;
    }
}

static void Task_QuestLogExit(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    DestroyListMenuTask(sLog->listTaskId, NULL, NULL);
    FreeAllWindowBuffers();
    Free(sLog->items);
    Free(sLog);
    sLog = NULL;
    DestroyTask(taskId);
    SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
}
