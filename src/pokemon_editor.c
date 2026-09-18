#include "global.h"
#include "pokemon_editor.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "item.h"
#include "list_menu.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "move.h"
#include "naming_screen.h"
#include "palette.h"
#include "pokemon.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "battle_main.h"
#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/species.h"

/*
 * Pokémon editor screen.
 *
 * Works on a private copy of the Pokémon (sEditor->mon) and writes the copy
 * back to the real party slot or box slot after every change, so leaving at
 * any point keeps what was edited. Opened from src/party_menu.c (EDIT) and
 * src/pokemon_storage_system.c (EDIT).
 *
 * Layout: one background, three windows. Header shows nickname, level and
 * species. The list shows every editable field with its value. Numeric
 * fields open a small spinner window; enum fields replace the list with a
 * picker list; the nickname opens the naming screen and returns here.
 */

enum EditorField
{
    FIELD_SPECIES,
    FIELD_LEVEL,
    FIELD_NATURE,
    FIELD_ABILITY,
    FIELD_ITEM,
    FIELD_MOVE1,
    FIELD_MOVE2,
    FIELD_MOVE3,
    FIELD_MOVE4,
    FIELD_IV_HP,
    FIELD_IV_ATK,
    FIELD_IV_DEF,
    FIELD_IV_SPEED,
    FIELD_IV_SPATK,
    FIELD_IV_SPDEF,
    FIELD_EV_HP,
    FIELD_EV_ATK,
    FIELD_EV_DEF,
    FIELD_EV_SPEED,
    FIELD_EV_SPATK,
    FIELD_EV_SPDEF,
    FIELD_SHINY,
    FIELD_FRIENDSHIP,
    FIELD_TERA_TYPE,
    FIELD_NICKNAME,
    FIELD_EXIT,
    FIELD_COUNT,
};

enum EditorMode
{
    MODE_FIELDS,
    MODE_NUMBER,
    MODE_PICKER,
};

enum PickerKind
{
    PICKER_SPECIES,
    PICKER_ITEM,
    PICKER_MOVE,
    PICKER_NATURE,
    PICKER_ABILITY,
    PICKER_TERA_TYPE,
};

struct PokemonEditor
{
    struct Pokemon mon;
    struct Pokemon *partyMon;
    struct BoxPokemon *boxMon;
    MainCallback returnCallback;
    struct ListMenuItem *pickerItems;
    u32 pickerCount;
    s32 number;
    s32 numberMin;
    s32 numberMax;
    u16 listScroll;
    u16 listRow;
    u8 taskId;
    u8 listTaskId;
    u8 headerWindowId;
    u8 listWindowId;
    u8 numberWindowId;
    u8 mode;
    u8 field;
    u8 pickerKind;
};

static EWRAM_DATA struct PokemonEditor *sEditor = NULL;

enum EditorWindow
{
    WIN_HEADER,
    WIN_LIST,
    WIN_NUMBER,
};

#define LIST_ROWS       7
#define VALUE_X         120
#define LIST_TEXT_COLOR sTextColors

static const u8 sTextColors[3] = { TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY };

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    },
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_HEADER] = { .bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 28, .height = 2,  .paletteNum = 15, .baseBlock = 1 },
    [WIN_LIST]   = { .bg = 0, .tilemapLeft = 1, .tilemapTop = 4, .width = 28, .height = 14, .paletteNum = 15, .baseBlock = 57 },
    DUMMY_WIN_TEMPLATE,
};

static const struct WindowTemplate sNumberWindowTemplate =
{
    .bg = 0, .tilemapLeft = 8, .tilemapTop = 7, .width = 14, .height = 4, .paletteNum = 15, .baseBlock = 449,
};

static const struct ListMenuItem sFieldItems[FIELD_COUNT] =
{
    [FIELD_SPECIES]    = { COMPOUND_STRING("Species"),    FIELD_SPECIES },
    [FIELD_LEVEL]      = { COMPOUND_STRING("Level"),      FIELD_LEVEL },
    [FIELD_NATURE]     = { COMPOUND_STRING("Nature"),     FIELD_NATURE },
    [FIELD_ABILITY]    = { COMPOUND_STRING("Ability"),    FIELD_ABILITY },
    [FIELD_ITEM]       = { COMPOUND_STRING("Held Item"),  FIELD_ITEM },
    [FIELD_MOVE1]      = { COMPOUND_STRING("Move 1"),     FIELD_MOVE1 },
    [FIELD_MOVE2]      = { COMPOUND_STRING("Move 2"),     FIELD_MOVE2 },
    [FIELD_MOVE3]      = { COMPOUND_STRING("Move 3"),     FIELD_MOVE3 },
    [FIELD_MOVE4]      = { COMPOUND_STRING("Move 4"),     FIELD_MOVE4 },
    [FIELD_IV_HP]      = { COMPOUND_STRING("IV HP"),      FIELD_IV_HP },
    [FIELD_IV_ATK]     = { COMPOUND_STRING("IV Attack"),  FIELD_IV_ATK },
    [FIELD_IV_DEF]     = { COMPOUND_STRING("IV Defense"), FIELD_IV_DEF },
    [FIELD_IV_SPEED]   = { COMPOUND_STRING("IV Speed"),   FIELD_IV_SPEED },
    [FIELD_IV_SPATK]   = { COMPOUND_STRING("IV Sp. Atk"), FIELD_IV_SPATK },
    [FIELD_IV_SPDEF]   = { COMPOUND_STRING("IV Sp. Def"), FIELD_IV_SPDEF },
    [FIELD_EV_HP]      = { COMPOUND_STRING("EV HP"),      FIELD_EV_HP },
    [FIELD_EV_ATK]     = { COMPOUND_STRING("EV Attack"),  FIELD_EV_ATK },
    [FIELD_EV_DEF]     = { COMPOUND_STRING("EV Defense"), FIELD_EV_DEF },
    [FIELD_EV_SPEED]   = { COMPOUND_STRING("EV Speed"),   FIELD_EV_SPEED },
    [FIELD_EV_SPATK]   = { COMPOUND_STRING("EV Sp. Atk"), FIELD_EV_SPATK },
    [FIELD_EV_SPDEF]   = { COMPOUND_STRING("EV Sp. Def"), FIELD_EV_SPDEF },
    [FIELD_SHINY]      = { COMPOUND_STRING("Shiny"),      FIELD_SHINY },
    [FIELD_FRIENDSHIP] = { COMPOUND_STRING("Friendship"), FIELD_FRIENDSHIP },
    [FIELD_TERA_TYPE]  = { COMPOUND_STRING("Tera Type"),  FIELD_TERA_TYPE },
    [FIELD_NICKNAME]   = { COMPOUND_STRING("Nickname"),   FIELD_NICKNAME },
    [FIELD_EXIT]       = { COMPOUND_STRING("Exit"),       FIELD_EXIT },
};

static const u8 sText_Yes[] = _("Yes");
static const u8 sText_No[] = _("No");
static const u8 sText_Lv[] = _(" Lv.");
static const u8 sText_Spaces[] = _("  ");
static const u8 sText_NumberHint[] = _("{DPAD_UPDOWN}1  {DPAD_LEFTRIGHT}10  {A_BUTTON}OK");

static void CB2_InitEditor(void);
static void CB2_EditorMain(void);
static void VBlankCB_Editor(void);
static void Task_EditorFadeIn(u8 taskId);
static void Task_EditorMain(u8 taskId);
static void Task_EditorExit(u8 taskId);
static void Task_EditorOpenNamingScreen(u8 taskId);
static void CB2_EditorReturnFromNaming(void);
static void DrawHeader(void);
static void CreateFieldList(void);
static void PrintFieldValue(u8 windowId, u32 field, u8 y);
static void SelectField(u32 field);
static void WriteBack(void);
static void RecalcStats(void);
static void OpenNumberEditor(u32 field, s32 min, s32 max, s32 current);
static void PrintNumberValue(void);
static void HandleNumberInput(void);
static void ApplyNumber(void);
static void CloseNumberEditor(void);
static void OpenPicker(u32 field, u8 kind);
static void HandlePickerInput(void);
static void ApplyPicker(s32 id);
static void ClosePicker(void);

void ShowPokemonEditor(struct Pokemon *mon, MainCallback returnCallback)
{
    sEditor = AllocZeroed(sizeof(*sEditor));
    sEditor->mon = *mon;
    sEditor->partyMon = mon;
    sEditor->boxMon = NULL;
    sEditor->returnCallback = returnCallback;
    SetMainCallback2(CB2_InitEditor);
}

void ShowPokemonEditorForBoxMon(struct BoxPokemon *boxMon, MainCallback returnCallback)
{
    sEditor = AllocZeroed(sizeof(*sEditor));
    BoxMonToMon(boxMon, &sEditor->mon);
    sEditor->partyMon = NULL;
    sEditor->boxMon = boxMon;
    sEditor->returnCallback = returnCallback;
    SetMainCallback2(CB2_InitEditor);
}

static void VBlankCB_Editor(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_EditorMain(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void CB2_InitEditor(void)
{
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
    FillPalette(RGB(4, 4, 10), BG_PLTT_ID(0), PLTT_SIZEOF(1));

    sEditor->headerWindowId = WIN_HEADER;
    sEditor->listWindowId = WIN_LIST;
    sEditor->numberWindowId = WINDOW_NONE;
    sEditor->mode = MODE_FIELDS;

    PutWindowTilemap(WIN_HEADER);
    PutWindowTilemap(WIN_LIST);
    DrawStdWindowFrame(WIN_HEADER, FALSE);
    DrawStdWindowFrame(WIN_LIST, FALSE);
    DrawHeader();
    CreateFieldList();

    BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    EnableInterrupts(1);
    SetVBlankCallback(VBlankCB_Editor);
    SetMainCallback2(CB2_EditorMain);
    sEditor->taskId = CreateTask(Task_EditorFadeIn, 0);
}

static void Task_EditorFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_EditorMain;
}

static void Task_EditorMain(u8 taskId)
{
    s32 input;

    switch (sEditor->mode)
    {
    case MODE_FIELDS:
        input = ListMenu_ProcessInput(sEditor->listTaskId);
        if (input == LIST_CANCEL)
        {
            PlaySE(SE_SELECT);
            SelectField(FIELD_EXIT);
        }
        else if (input != LIST_NOTHING_CHOSEN)
        {
            PlaySE(SE_SELECT);
            SelectField(input);
        }
        break;
    case MODE_NUMBER:
        HandleNumberInput();
        break;
    case MODE_PICKER:
        HandlePickerInput();
        break;
    }
}

// ---- Header and field list ----

static void DrawHeader(void)
{
    u8 nickname[POKEMON_NAME_BUFFER_SIZE];

    GetMonData(&sEditor->mon, MON_DATA_NICKNAME, nickname);
    StringGet_Nickname(nickname);
    StringCopy(gStringVar4, nickname);
    StringAppend(gStringVar4, sText_Lv);
    ConvertIntToDecimalStringN(gStringVar1, GetMonData(&sEditor->mon, MON_DATA_LEVEL), STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(gStringVar4, gStringVar1);
    StringAppend(gStringVar4, sText_Spaces);
    StringAppend(gStringVar4, GetSpeciesName(GetMonData(&sEditor->mon, MON_DATA_SPECIES)));

    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
    AddTextPrinterParameterized4(WIN_HEADER, FONT_NORMAL, 4, 0, 0, 0, sTextColors, TEXT_SKIP_DRAW, gStringVar4);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void CreateFieldList(void)
{
    struct ListMenuTemplate template = {0};

    template.items = sFieldItems;
    template.moveCursorFunc = ListMenuDefaultCursorMoveFunc;
    template.itemPrintFunc = PrintFieldValue;
    template.totalItems = FIELD_COUNT;
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
    sEditor->listTaskId = ListMenuInit(&template, sEditor->listScroll, sEditor->listRow);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
}

static void PrintFieldValue(u8 windowId, u32 field, u8 y)
{
    struct Pokemon *mon = &sEditor->mon;
    const u8 *str = NULL;
    u8 nickname[POKEMON_NAME_BUFFER_SIZE];

    switch (field)
    {
    case FIELD_SPECIES:
        str = GetSpeciesName(GetMonData(mon, MON_DATA_SPECIES));
        break;
    case FIELD_LEVEL:
        ConvertIntToDecimalStringN(gStringVar1, GetMonData(mon, MON_DATA_LEVEL), STR_CONV_MODE_LEFT_ALIGN, 3);
        str = gStringVar1;
        break;
    case FIELD_NATURE:
        str = gNaturesInfo[GetNature(mon)].name;
        break;
    case FIELD_ABILITY:
        str = gAbilitiesInfo[GetMonAbility(mon)].name;
        break;
    case FIELD_ITEM:
        str = GetItemName(GetMonData(mon, MON_DATA_HELD_ITEM));
        break;
    case FIELD_MOVE1:
    case FIELD_MOVE2:
    case FIELD_MOVE3:
    case FIELD_MOVE4:
        str = GetMoveName(GetMonData(mon, MON_DATA_MOVE1 + (field - FIELD_MOVE1)));
        break;
    case FIELD_IV_HP:
    case FIELD_IV_ATK:
    case FIELD_IV_DEF:
    case FIELD_IV_SPEED:
    case FIELD_IV_SPATK:
    case FIELD_IV_SPDEF:
        ConvertIntToDecimalStringN(gStringVar1, GetMonData(mon, MON_DATA_HP_IV + (field - FIELD_IV_HP)), STR_CONV_MODE_LEFT_ALIGN, 3);
        str = gStringVar1;
        break;
    case FIELD_EV_HP:
    case FIELD_EV_ATK:
    case FIELD_EV_DEF:
    case FIELD_EV_SPEED:
    case FIELD_EV_SPATK:
    case FIELD_EV_SPDEF:
        ConvertIntToDecimalStringN(gStringVar1, GetMonData(mon, MON_DATA_HP_EV + (field - FIELD_EV_HP)), STR_CONV_MODE_LEFT_ALIGN, 3);
        str = gStringVar1;
        break;
    case FIELD_SHINY:
        str = GetMonData(mon, MON_DATA_IS_SHINY) ? sText_Yes : sText_No;
        break;
    case FIELD_FRIENDSHIP:
        ConvertIntToDecimalStringN(gStringVar1, GetMonData(mon, MON_DATA_FRIENDSHIP), STR_CONV_MODE_LEFT_ALIGN, 3);
        str = gStringVar1;
        break;
    case FIELD_TERA_TYPE:
        str = gTypesInfo[GetMonData(mon, MON_DATA_TERA_TYPE)].name;
        break;
    case FIELD_NICKNAME:
        GetMonData(mon, MON_DATA_NICKNAME, nickname);
        StringGet_Nickname(nickname);
        StringCopy(gStringVar2, nickname);
        str = gStringVar2;
        break;
    default:
        return;
    }

    AddTextPrinterParameterized4(windowId, FONT_NORMAL, VALUE_X, y, 0, 0, sTextColors, TEXT_SKIP_DRAW, str);
}

static void RefreshFieldList(void)
{
    RedrawListMenu(sEditor->listTaskId);
    DrawHeader();
}

// ---- Applying changes ----

static void WriteBack(void)
{
    if (sEditor->partyMon != NULL)
        *sEditor->partyMon = sEditor->mon;
    else
        *sEditor->boxMon = sEditor->mon.box;
}

static void RecalcStats(void)
{
    CalculateMonStats(&sEditor->mon);
}

static void SelectField(u32 field)
{
    struct Pokemon *mon = &sEditor->mon;

    switch (field)
    {
    case FIELD_SPECIES:
        OpenPicker(field, PICKER_SPECIES);
        break;
    case FIELD_LEVEL:
        OpenNumberEditor(field, 1, MAX_LEVEL, GetMonData(mon, MON_DATA_LEVEL));
        break;
    case FIELD_NATURE:
        OpenPicker(field, PICKER_NATURE);
        break;
    case FIELD_ABILITY:
        OpenPicker(field, PICKER_ABILITY);
        break;
    case FIELD_ITEM:
        OpenPicker(field, PICKER_ITEM);
        break;
    case FIELD_MOVE1:
    case FIELD_MOVE2:
    case FIELD_MOVE3:
    case FIELD_MOVE4:
        OpenPicker(field, PICKER_MOVE);
        break;
    case FIELD_IV_HP:
    case FIELD_IV_ATK:
    case FIELD_IV_DEF:
    case FIELD_IV_SPEED:
    case FIELD_IV_SPATK:
    case FIELD_IV_SPDEF:
        OpenNumberEditor(field, 0, MAX_PER_STAT_IVS, GetMonData(mon, MON_DATA_HP_IV + (field - FIELD_IV_HP)));
        break;
    case FIELD_EV_HP:
    case FIELD_EV_ATK:
    case FIELD_EV_DEF:
    case FIELD_EV_SPEED:
    case FIELD_EV_SPATK:
    case FIELD_EV_SPDEF:
        OpenNumberEditor(field, 0, MAX_PER_STAT_EVS, GetMonData(mon, MON_DATA_HP_EV + (field - FIELD_EV_HP)));
        break;
    case FIELD_SHINY:
    {
        bool32 isShiny = !GetMonData(mon, MON_DATA_IS_SHINY);
        SetMonData(mon, MON_DATA_IS_SHINY, &isShiny);
        WriteBack();
        RefreshFieldList();
        break;
    }
    case FIELD_FRIENDSHIP:
        OpenNumberEditor(field, 0, MAX_FRIENDSHIP, GetMonData(mon, MON_DATA_FRIENDSHIP));
        break;
    case FIELD_TERA_TYPE:
        OpenPicker(field, PICKER_TERA_TYPE);
        break;
    case FIELD_NICKNAME:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[sEditor->taskId].func = Task_EditorOpenNamingScreen;
        break;
    case FIELD_EXIT:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[sEditor->taskId].func = Task_EditorExit;
        break;
    }
}

// ---- Number editor ----

static void OpenNumberEditor(u32 field, s32 min, s32 max, s32 current)
{
    sEditor->field = field;
    sEditor->numberMin = min;
    sEditor->numberMax = max;
    sEditor->number = current;
    sEditor->numberWindowId = AddWindow(&sNumberWindowTemplate);
    DrawStdWindowFrame(sEditor->numberWindowId, FALSE);
    PutWindowTilemap(sEditor->numberWindowId);
    PrintNumberValue();
    sEditor->mode = MODE_NUMBER;
}

static void PrintNumberValue(void)
{
    u8 windowId = sEditor->numberWindowId;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    AddTextPrinterParameterized4(windowId, FONT_NORMAL, 4, 0, 0, 0, sTextColors, TEXT_SKIP_DRAW, sFieldItems[sEditor->field].name);
    ConvertIntToDecimalStringN(gStringVar1, sEditor->number, STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized4(windowId, FONT_NORMAL, 80, 0, 0, 0, sTextColors, TEXT_SKIP_DRAW, gStringVar1);
    AddTextPrinterParameterized4(windowId, FONT_SMALL, 4, 16, 0, 0, sTextColors, TEXT_SKIP_DRAW, sText_NumberHint);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void HandleNumberInput(void)
{
    s32 delta = 0;

    if (JOY_REPEAT(DPAD_UP))
        delta = 1;
    else if (JOY_REPEAT(DPAD_DOWN))
        delta = -1;
    else if (JOY_REPEAT(DPAD_RIGHT))
        delta = 10;
    else if (JOY_REPEAT(DPAD_LEFT))
        delta = -10;

    if (delta != 0)
    {
        s32 value = sEditor->number + delta;
        if (value < sEditor->numberMin)
            value = sEditor->numberMin;
        if (value > sEditor->numberMax)
            value = sEditor->numberMax;
        if (value != sEditor->number)
        {
            sEditor->number = value;
            PlaySE(SE_SELECT);
            PrintNumberValue();
        }
        return;
    }

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        ApplyNumber();
        CloseNumberEditor();
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseNumberEditor();
    }
}

static void ApplyNumber(void)
{
    struct Pokemon *mon = &sEditor->mon;
    u32 field = sEditor->field;
    u32 value = sEditor->number;

    switch (field)
    {
    case FIELD_LEVEL:
    {
        enum Species species = GetMonData(mon, MON_DATA_SPECIES);
        u32 exp = gExperienceTables[gSpeciesInfo[species].growthRate][value];
        SetMonData(mon, MON_DATA_EXP, &exp);
        break;
    }
    case FIELD_FRIENDSHIP:
        SetMonData(mon, MON_DATA_FRIENDSHIP, &value);
        break;
    default:
        if (field >= FIELD_IV_HP && field <= FIELD_IV_SPDEF)
            SetMonData(mon, MON_DATA_HP_IV + (field - FIELD_IV_HP), &value);
        else if (field >= FIELD_EV_HP && field <= FIELD_EV_SPDEF)
            SetMonData(mon, MON_DATA_HP_EV + (field - FIELD_EV_HP), &value);
        break;
    }
    RecalcStats();
    WriteBack();
}

static void CloseNumberEditor(void)
{
    ClearStdWindowAndFrame(sEditor->numberWindowId, TRUE);
    RemoveWindow(sEditor->numberWindowId);
    sEditor->numberWindowId = WINDOW_NONE;
    sEditor->mode = MODE_FIELDS;
    // The number window covered part of the list; redraw it.
    DrawStdWindowFrame(WIN_LIST, FALSE);
    PutWindowTilemap(WIN_LIST);
    RefreshFieldList();
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
}

// ---- Pickers ----

static void AddPickerItem(const u8 *name, s32 id)
{
    sEditor->pickerItems[sEditor->pickerCount].name = name;
    sEditor->pickerItems[sEditor->pickerCount].id = id;
    sEditor->pickerCount++;
}

static s32 GetPickerCurrentId(u32 field, u8 kind)
{
    struct Pokemon *mon = &sEditor->mon;

    switch (kind)
    {
    case PICKER_SPECIES:   return GetMonData(mon, MON_DATA_SPECIES);
    case PICKER_ITEM:      return GetMonData(mon, MON_DATA_HELD_ITEM);
    case PICKER_MOVE:      return GetMonData(mon, MON_DATA_MOVE1 + (field - FIELD_MOVE1));
    case PICKER_NATURE:    return GetNature(mon);
    case PICKER_ABILITY:   return GetMonData(mon, MON_DATA_ABILITY_NUM);
    case PICKER_TERA_TYPE: return GetMonData(mon, MON_DATA_TERA_TYPE);
    }
    return 0;
}

static void OpenPicker(u32 field, u8 kind)
{
    struct ListMenuTemplate template = {0};
    u32 i, capacity, currentIndex = 0;
    s32 currentId = GetPickerCurrentId(field, kind);
    enum Species species = GetMonData(&sEditor->mon, MON_DATA_SPECIES);

    switch (kind)
    {
    case PICKER_SPECIES:   capacity = NUM_SPECIES; break;
    case PICKER_ITEM:      capacity = ITEMS_COUNT; break;
    case PICKER_MOVE:      capacity = MOVES_COUNT; break;
    case PICKER_NATURE:    capacity = NUM_NATURES; break;
    case PICKER_ABILITY:   capacity = NUM_ABILITY_SLOTS; break;
    default:               capacity = NUMBER_OF_MON_TYPES; break;
    }

    sEditor->field = field;
    sEditor->pickerKind = kind;
    sEditor->pickerCount = 0;
    sEditor->pickerItems = AllocZeroed(capacity * sizeof(struct ListMenuItem));

    switch (kind)
    {
    case PICKER_SPECIES:
        for (i = 1; i < NUM_SPECIES; i++)
            if (IsSpeciesEnabled(i))
                AddPickerItem(GetSpeciesName(i), i);
        break;
    case PICKER_ITEM:
        for (i = ITEM_NONE; i < ITEMS_COUNT; i++)
            AddPickerItem(GetItemName(i), i);
        break;
    case PICKER_MOVE:
        for (i = MOVE_NONE; i < MOVES_COUNT; i++)
            AddPickerItem(GetMoveName(i), i);
        break;
    case PICKER_NATURE:
        for (i = 0; i < NUM_NATURES; i++)
            AddPickerItem(gNaturesInfo[i].name, i);
        break;
    case PICKER_ABILITY:
        for (i = 0; i < NUM_ABILITY_SLOTS; i++)
        {
            enum Ability ability = GetAbilityBySpecies(species, i);
            if (ability != ABILITY_NONE)
                AddPickerItem(gAbilitiesInfo[ability].name, i);
        }
        break;
    case PICKER_TERA_TYPE:
        for (i = TYPE_NONE + 1; i < NUMBER_OF_MON_TYPES; i++)
            AddPickerItem(gTypesInfo[i].name, i);
        break;
    }

    if (sEditor->pickerCount == 0)
    {
        Free(sEditor->pickerItems);
        sEditor->pickerItems = NULL;
        return;
    }

    for (i = 0; i < sEditor->pickerCount; i++)
    {
        if (sEditor->pickerItems[i].id == currentId)
        {
            currentIndex = i;
            break;
        }
    }

    DestroyListMenuTask(sEditor->listTaskId, &sEditor->listScroll, &sEditor->listRow);

    template.items = sEditor->pickerItems;
    template.moveCursorFunc = ListMenuDefaultCursorMoveFunc;
    template.itemPrintFunc = NULL;
    template.totalItems = sEditor->pickerCount;
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

    {
        u16 row = (currentIndex < LIST_ROWS - 1) ? currentIndex : LIST_ROWS - 1;
        u16 scroll = currentIndex - row;
        if (scroll + LIST_ROWS > sEditor->pickerCount && sEditor->pickerCount >= LIST_ROWS)
        {
            scroll = sEditor->pickerCount - LIST_ROWS;
            row = currentIndex - scroll;
        }
        FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
        sEditor->listTaskId = ListMenuInit(&template, scroll, row);
        CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
    }
    sEditor->mode = MODE_PICKER;
}

static void HandlePickerInput(void)
{
    s32 input = ListMenu_ProcessInput(sEditor->listTaskId);

    if (input == LIST_CANCEL)
    {
        PlaySE(SE_SELECT);
        ClosePicker();
    }
    else if (input != LIST_NOTHING_CHOSEN)
    {
        PlaySE(SE_SELECT);
        ApplyPicker(input);
        ClosePicker();
    }
}

static void ApplyPicker(s32 id)
{
    struct Pokemon *mon = &sEditor->mon;
    u32 field = sEditor->field;

    switch (sEditor->pickerKind)
    {
    case PICKER_SPECIES:
    {
        enum Species oldSpecies = GetMonData(mon, MON_DATA_SPECIES);
        enum Species newSpecies = id;
        u8 nickname[POKEMON_NAME_BUFFER_SIZE];
        u32 abilityNum;

        GetMonData(mon, MON_DATA_NICKNAME, nickname);
        SetMonData(mon, MON_DATA_SPECIES, &newSpecies);
        // Keep the nickname in step if it was just the species name.
        if (StringCompare(nickname, GetSpeciesName(oldSpecies)) == 0)
            SetMonData(mon, MON_DATA_NICKNAME, GetSpeciesName(newSpecies));
        abilityNum = GetMonData(mon, MON_DATA_ABILITY_NUM);
        if (GetAbilityBySpecies(newSpecies, abilityNum) == ABILITY_NONE)
        {
            abilityNum = 0;
            SetMonData(mon, MON_DATA_ABILITY_NUM, &abilityNum);
        }
        break;
    }
    case PICKER_ITEM:
        SetMonData(mon, MON_DATA_HELD_ITEM, &id);
        break;
    case PICKER_MOVE:
        SetMonMoveSlot(mon, id, field - FIELD_MOVE1);
        break;
    case PICKER_NATURE:
        SetMonData(mon, MON_DATA_HIDDEN_NATURE, &id);
        break;
    case PICKER_ABILITY:
        SetMonData(mon, MON_DATA_ABILITY_NUM, &id);
        break;
    case PICKER_TERA_TYPE:
        SetMonData(mon, MON_DATA_TERA_TYPE, &id);
        break;
    }
    RecalcStats();
    WriteBack();
}

static void ClosePicker(void)
{
    DestroyListMenuTask(sEditor->listTaskId, NULL, NULL);
    Free(sEditor->pickerItems);
    sEditor->pickerItems = NULL;
    sEditor->pickerCount = 0;
    sEditor->mode = MODE_FIELDS;
    CreateFieldList();
    DrawHeader();
}

// ---- Nickname and exit ----

static void Task_EditorOpenNamingScreen(u8 taskId)
{
    if (gPaletteFade.active)
        return;

    ListMenuGetScrollAndRow(sEditor->listTaskId, &sEditor->listScroll, &sEditor->listRow);
    DestroyListMenuTask(sEditor->listTaskId, NULL, NULL);
    FreeAllWindowBuffers();
    DestroyTask(taskId);
    GetMonData(&sEditor->mon, MON_DATA_NICKNAME, gStringVar2);
    DoNamingScreen(NAMING_SCREEN_NICKNAME, gStringVar2,
                   GetMonData(&sEditor->mon, MON_DATA_SPECIES),
                   GetMonGender(&sEditor->mon),
                   GetMonData(&sEditor->mon, MON_DATA_PERSONALITY),
                   CB2_EditorReturnFromNaming);
}

static void CB2_EditorReturnFromNaming(void)
{
    SetMonData(&sEditor->mon, MON_DATA_NICKNAME, gStringVar2);
    WriteBack();
    CB2_InitEditor();
}

static void Task_EditorExit(u8 taskId)
{
    MainCallback callback;

    if (gPaletteFade.active)
        return;

    DestroyListMenuTask(sEditor->listTaskId, NULL, NULL);
    if (sEditor->pickerItems != NULL)
        Free(sEditor->pickerItems);
    FreeAllWindowBuffers();
    DestroyTask(taskId);
    callback = sEditor->returnCallback;
    Free(sEditor);
    sEditor = NULL;
    SetMainCallback2(callback);
}
