/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef UI_H
#define UI_H

#include <fontlist.h>
#include <hal_api.h>
#include <newrpl.h>
#include <utf8lib.h>


// Color to grayscale conversion helpers


// Palette color names

// First 16 are reserved for 16 grays (for grayscale compatibility)

#define PAL_GRAY0             0
#define PAL_GRAY1             1
#define PAL_GRAY2             2
#define PAL_GRAY3             3
#define PAL_GRAY4             4
#define PAL_GRAY5             5
#define PAL_GRAY6             6
#define PAL_GRAY7             7
#define PAL_GRAY8             8
#define PAL_GRAY9             9
#define PAL_GRAY10            10
#define PAL_GRAY11            11
#define PAL_GRAY12            12
#define PAL_GRAY13            13
#define PAL_GRAY14            14
#define PAL_GRAY15            15

// Theme colors for the stack
#define PAL_STK_BG            16
#define PAL_STK_INDEX         17
#define PAL_STK_VLINE         18
#define PAL_STK_IDX_BG        19
#define PAL_STK_ITEMS         20
#define PAL_STK_SEL_BG        21
#define PAL_STK_SEL_ITEM      22
#define PAL_STK_CURSOR        23

// Theme colors for the command line
#define PAL_CMD_BG            24
#define PAL_CMD_TEXT          25
#define PAL_CMD_SEL_BG        26
#define PAL_CMD_SELTEXT       27
#define PAL_CMD_CURSOR_BG     28
#define PAL_CMD_CURSOR        29
#define PAL_DIV_LINE          30

// Theme colors for menu
#define PAL_MENU_BG           31
#define PAL_MENU_INV_BG       32
#define PAL_MENU_TEXT         33
#define PAL_MENU_INV_TEXT     34
#define PAL_MENU_DIR_MARK     35
#define PAL_MENU_INV_DIR_MARK 36
#define PAL_MENU_HLINE        37
#define PAL_MENU_FOCUS_HLINE  38
#define PAL_MENU_PRESS_BG     39
#define PAL_MENU_PRESS_INV_BG 40


// Theme colors for status area
#define PAL_STA_BG            41
#define PAL_STA_TEXT          42
#define PAL_STA_ANNPRESS      43
#define PAL_STA_ANN           44
#define PAL_STA_BAT           45
#define PAL_STA_UFLAG0        46
#define PAL_STA_UFLAG1        47

#define PAL_HLP_BG            48
#define PAL_HLP_TEXT          49
#define PAL_HLP_LINES         50


#define PAL_FORM_BG           51
#define PAL_FORM_TEXT         52
#define PAL_FORM_SELTEXT      53
#define PAL_FORM_SEL_BG       54
#define PAL_FORM_CURSOR       55


// Add additional theme colors here


// up to PALETTE_SIZE palette colors


// Default Palette values
// stick to 16 grays mode by default

#define THEME_GRAY0           RGB_TO_RGB16(255, 255, 255)
#define THEME_GRAY1           RGB_TO_RGB16(238, 238, 238)
#define THEME_GRAY2           RGB_TO_RGB16(221, 221, 221)
#define THEME_GRAY3           RGB_TO_RGB16(204, 204, 204)
#define THEME_GRAY4           RGB_TO_RGB16(187, 187, 187)
#define THEME_GRAY5           RGB_TO_RGB16(170, 170, 170)
#define THEME_GRAY6           RGB_TO_RGB16(153, 153, 153)
#define THEME_GRAY7           RGB_TO_RGB16(136, 136, 136)
#define THEME_GRAY8           RGB_TO_RGB16(119, 119, 119)
#define THEME_GRAY9           RGB_TO_RGB16(102, 102, 102)
#define THEME_GRAY10          RGB_TO_RGB16(85, 85, 85)
#define THEME_GRAY11          RGB_TO_RGB16(68, 68, 68)
#define THEME_GRAY12          RGB_TO_RGB16(51, 51, 51)
#define THEME_GRAY13          RGB_TO_RGB16(34, 34, 34)
#define THEME_GRAY14          RGB_TO_RGB16(17, 17, 17)
#define THEME_GRAY15          RGB_TO_RGB16(0, 0, 0)


// Theme colors for stack
#ifndef NEWRPL_COLOR
// Default Palette values for GRAYSCALE hardware
// stick to 16 grays mode by default

// Theme colors for stack
#  define THEME_STK_BG            PATTERN_SOLID(PAL_GRAY0)
#  define THEME_STK_INDEX         PATTERN_SOLID(PAL_GRAY15)
#  define THEME_STK_VLINE         PATTERN_SOLID(PAL_GRAY8)
#  define THEME_STK_IDX_BG        PATTERN_SOLID(PAL_GRAY0)
#  define THEME_STK_ITEMS         PATTERN_SOLID(PAL_GRAY15)
#  define THEME_STK_SEL_BG        PATTERN_SOLID(PAL_GRAY6)
#  define THEME_STK_SEL_ITEM      PATTERN_SOLID(PAL_GRAY15)
#  define THEME_STK_CURSOR        PATTERN_SOLID(PAL_GRAY15)

// Theme colors for command line
#  define THEME_CMD_BG            PATTERN_SOLID(PAL_GRAY0)
#  define THEME_CMD_TEXT          PATTERN_SOLID(PAL_GRAY15)
#  define THEME_CMD_SEL_BG        PATTERN_SOLID(PAL_GRAY6)
#  define THEME_CMD_SELTEXT       PATTERN_SOLID(PAL_GRAY15)
#  define THEME_CMD_CURSOR_BG     PATTERN_SOLID(PAL_GRAY15)
#  define THEME_CMD_CURSOR        PATTERN_SOLID(PAL_GRAY0)
#  define THEME_DIV_LINE          PATTERN_SOLID(PAL_GRAY8)

// Theme colors for menu
#  define THEME_MENU_BG           PATTERN_SOLID(PAL_GRAY15)
#  define THEME_MENU_INV_BG       PATTERN_SOLID(PAL_GRAY0)
#  define THEME_MENU_TEXT         PATTERN_SOLID(PAL_GRAY0)
#  define THEME_MENU_INV_TEXT     PATTERN_SOLID(PAL_GRAY15)
#  define THEME_MENU_DIR_MARK     PATTERN_SOLID(PAL_GRAY8)
#  define THEME_MENU_INV_DIR_MARK PATTERN_SOLID(PAL_GRAY6)
#  define THEME_MENU_HLINE        PATTERN_SOLID(PAL_GRAY8)
#  define THEME_MENU_FOCUS_HLINE  PATTERN_SOLID(PAL_GRAY0)
#  define THEME_MENU_PRESS_BG     RGB_TO_RGB16(255, 0, 0)
#  define THEME_MENU_PRESS_INV_BG RGB_TO_RGB16(255, 0, 0)

// Theme colors for status area
#  define THEME_STA_BG            PATTERN_SOLID(PAL_GRAY0)
#  define THEME_STA_TEXT          PATTERN_SOLID(PAL_GRAY15)
#  define THEME_STA_ANNPRESS      PATTERN_SOLID(PAL_GRAY15)
#  define THEME_STA_ANN           PATTERN_SOLID(PAL_GRAY8)
#  define THEME_STA_BAT           PATTERN_SOLID(PAL_GRAY15)
#  define THEME_STA_UFLAG0        PATTERN_SOLID(PAL_GRAY8)
#  define THEME_STA_UFLAG1        PATTERN_SOLID(PAL_GRAY15)

// Theme colors for help and popup messages
#  define THEME_HLP_BG            PATTERN_SOLID(PAL_GRAY0)
#  define THEME_HLP_TEXT          PATTERN_SOLID(PAL_GRAY15)
#  define THEME_HLP_LINES         PATTERN_SOLID(PAL_GRAY8)

// Theme colors for Forms
#  define THEME_FORMBACKGROUND    PATTERN_SOLID(PAL_GRAY0)
#  define THEME_FORMTEXT          PATTERN_SOLID(PAL_GRAY15)
#  define THEME_FORMSELTEXT       PATTERN_SOLID(PAL_GRAY15)
#  define THEME_FORMSEL_BG        PATTERN_SOLID(PAL_GRAY6)
#  define THEME_FORMCURSOR        PATTERN_SOLID(PAL_GRAY15)

#else // NEWRPL_COLOR

#  define THEME_STK_BG            RGB_TO_RGB16(255, 255, 255)
#  define THEME_STK_INDEX         RGB_TO_RGB16(0, 0, 0)
#  define THEME_STK_VLINE         RGB_TO_RGB16(128, 128, 128)
#  define THEME_STK_IDX_BG        RGB_TO_RGB16(255, 255, 255)
#  define THEME_STK_ITEMS         RGB_TO_RGB16(0, 0, 0)
#  define THEME_STK_SEL_BG        RGB_TO_RGB16(192, 192, 192)
#  define THEME_STK_SEL_ITEM      RGB_TO_RGB16(0, 0, 0)
#  define THEME_STK_CURSOR        RGB_TO_RGB16(0, 0, 0)

// Theme colors for command line
#  define THEME_CMD_BG            RGB_TO_RGB16(255, 255, 255)
#  define THEME_CMD_TEXT          RGB_TO_RGB16(0, 0, 0)
#  define THEME_CMD_SEL_BG        RGB_TO_RGB16(192, 192, 192)
#  define THEME_CMD_SELTEXT       RGB_TO_RGB16(0, 0, 0)
#  define THEME_CMD_CURSOR_BG     RGB_TO_RGB16(0, 0, 0)
#  define THEME_CMD_CURSOR        RGB_TO_RGB16(255, 255, 255)
#  define THEME_DIV_LINE          RGB_TO_RGB16(128, 128, 128)

// Theme colors for menu
#  define THEME_MENU_BG           RGB_TO_RGB16(0, 0, 0)
#  define THEME_MENU_INV_BG       RGB_TO_RGB16(220, 220, 220)
#  define THEME_MENU_TEXT         RGB_TO_RGB16(255, 255, 255)
#  define THEME_MENU_INV_TEXT     RGB_TO_RGB16(0, 0, 0)
#  define THEME_MENU_DIR_MARK     RGB_TO_RGB16(128, 128, 128)
#  define THEME_MENU_INV_DIR_MARK RGB_TO_RGB16(192, 192, 192)
#  define THEME_MENU_HLINE        RGB_TO_RGB16(128, 128, 128)
#  define THEME_MENU_FOCUS_HLINE  RGB_TO_RGB16(255, 255, 255)
#  define THEME_MENU_PRESS_BG     RGB_TO_RGB16(255, 0, 0)
#  define THEME_MENU_PRESS_INV_BG RGB_TO_RGB16(255, 0, 0)


// Theme colors for status area
#  define THEME_STA_BG            RGB_TO_RGB16(255, 255, 255)
#  define THEME_STA_TEXT          RGB_TO_RGB16(0, 0, 0)
#  define THEME_STA_ANNPRESS      RGB_TO_RGB16(0, 0, 0)
#  define THEME_STA_ANN           RGB_TO_RGB16(128, 128, 128)
#  define THEME_STA_BAT           RGB_TO_RGB16(0, 0, 0)
#  define THEME_STA_UFLAG0        RGB_TO_RGB16(128, 128, 128)
#  define THEME_STA_UFLAG1        RGB_TO_RGB16(0, 0, 0)

// Theme colors for help and popup messages
#  define THEME_HLP_BG            RGB_TO_RGB16(255, 255, 255)
#  define THEME_HLP_TEXT          RGB_TO_RGB16(0, 0, 0)
#  define THEME_HLP_LINES         RGB_TO_RGB16(128, 128, 128)

// Theme colors for Forms
#  define THEME_FORMBACKGROUND    RGB_TO_RGB16(255, 255, 255)
#  define THEME_FORMTEXT          RGB_TO_RGB16(0, 0, 0)
#  define THEME_FORMSELTEXT       RGB_TO_RGB16(0, 0, 0)
#  define THEME_FORMSEL_BG        RGB_TO_RGB16(192, 192, 192)
#  define THEME_FORMCURSOR        RGB_TO_RGB16(0, 0, 0)
#endif // NEWRPL_COLOR

// HERE'S THE LANGUAGE OF THE ROM
// CHANGE THIS FOR OTHER LANGUAGES
// ONLY THE MATCHING LANGUAGE WILL BE COMPILED AND INCLUDED
#define UI_LANG_ENGLISH    1

// CONSTANTS THAT DEFINE THE LOOK OF THE USER INTERFACE
#ifndef STATUS_AREA_X
#  define STATUS_AREA_X (66 * LCD_W) / 131
#endif
// BASIC HEIGHT OF SCREEN AREAS IN PIXELS - THIS IS HARDWARE DEPENDENT SO WE NEED TO ALLOW TARGET HEADER TO DEFINE IF
// NEEDED
#ifndef MENU2_HEIGHT
#  define MENU2_HEIGHT (2 * (1 + FONT_HEIGHT(FONT_MENU)) + 2)
#endif
#ifndef MENU1_HEIGHT
#  define MENU1_HEIGHT (FONT_HEIGHT(FONT_MENU) + 3)
#endif
#define CMDLINE_HEIGHT      (FONT_HEIGHT(FONT_CMDLINE) + 2)

#define MENU_TAB_WIDTH      (((LCD_W - 5) / 6) + 1)

#define BITSPERPIXEL        (32 / PIXELS_PER_WORD)

// PREFERRED HEIGHT OF THE FONTS
#define DEF_FNT_STK_HEIGHT  (LCD_H / 10)
#define DEF_FNT_1STK_HEIGHT DEF_FNT_STK_HEIGHT
#define DEF_FNT_CMDL_HEIGHT DEF_FNT_STK_HEIGHT
#define DEF_FNT_FORM_HEIGHT DEF_FNT_STK_HEIGHT
#define DEF_FNT_PLOT_HEIGHT DEF_FNT_STK_HEIGHT
#define DEF_FNT_MENU_HEIGHT (LCD_H / 13)
#define DEF_FNT_STAT_HEIGHT DEF_FNT_MENU_HEIGHT
#define DEF_FNT_HELP_HEIGHT DEF_FNT_MENU_HEIGHT
#define DEF_FNT_HLPT_HEIGHT DEF_FNT_MENU_HEIGHT

// COMMAND LINE

word_p     halSaveCmdLine();
int32_t        halRestoreCmdLine(word_p data);
// INSERT TEXT, OPEN NEW CMD LINE IF NEEDED
void        uiOpenAndInsertTextN(byte_p start, byte_p end);

extern int32_t ui_visibleline, ui_nlines;
extern int32_t ui_currentline, ui_prevline;
extern int32_t ui_islinemodified;
extern int32_t ui_cursorx, ui_cursoroffset;
extern int32_t ui_visiblex;

void        uiSetCmdLineState(int32_t state);
int32_t        uiGetCmdLineState();

void        uiEnsureCursorVisible();
void        uiModifyLine(int dontaddnewline);
void        uiExtractLine(int32_t line);
byte_p     uiFindNumberStart(byte_p *endofnum, int32_t *flagsptr);
word_p     uiGetCmdLineText();
int32_t        uiSetCmdLineText(word_p text);
void        uiOpenCmdLine(int32_t mode);
void        uiCloseCmdLine();
void        uiSetCurrentLine(int32_t line);
int32_t        uiInsertCharacters(byte_p string);
int32_t        uiInsertCharactersN(byte_p string, byte_p end);
void        uiRemoveCharacters(int32_t length);

void        uiStretchCmdLine(int32_t addition);
void        uiAutocompleteUpdate();
void        uiAutocompNext();
void        uiAutocompPrev();
void        uiAutocompInsert();

byte_p     uiAutocompStringStart();
byte_p     uiAutocompStringEnd();
byte_p     uiAutocompStringTokEnd();

void        uiSetSelectionStart();
void        uiSetSelectionEnd();
word_p     uiExtractSelection();
int32_t        uiDeleteSelection();

int32_t        uiGetIndentLevel(int32_t *isemptyline);

void        uiSeparateToken();

void        uiMoveCursor(int32_t offset);
void        uiCursorLeft(int32_t nchars);
void        uiCursorRight(int32_t nchars);
void        uiCursorDown(int32_t nlines);
void        uiCursorUp(int32_t nlines);
void        uiCursorEndOfLine();
void        uiCursorStartOfLine();
void        uiCursorStartOfText();
void        uiCursorEndOfText();
void        uiCursorPageLeft();
void        uiCursorPageUp();
void        uiCursorPageDown();
void        uiCursorPageRight();

// SOFT MENUS

int32_t        uiCountMenuItems(WORD MenuCode, word_p menu);
word_p     uiGetLibMenu(int64_t MenuCode);
word_p     uiGetMenuItem(int64_t MenuCode, word_p menu, int32_t item);
word_p     uiGetMenuItemAction(word_p item, int32_t shift);
word_p     uiGetMenuItemHelp(word_p item);
void        uiDrawMenuItem(word_p item, int32_t palette_color, int32_t palette_bkcolor, gglsurface *scr);

word_p     uiGetLibHelp(word_p Object);

word_p     uiGetLibMsg(WORD MsgCode);

// RPL CODE EXECUTION FROM UI
void        uiCmdRun(WORD Opcode);
int32_t        uiCmdRunTransparent(WORD Opcode, int32_t nargs, int32_t nresults);

// FORMS
void        uiUpdateForm(word_p form);

#endif // UI_H
