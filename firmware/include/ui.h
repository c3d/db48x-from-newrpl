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
#define PAL_GRAY0             ggl_color(0)
#define PAL_GRAY1             ggl_color(1)
#define PAL_GRAY2             ggl_color(2)
#define PAL_GRAY3             ggl_color(3)
#define PAL_GRAY4             ggl_color(4)
#define PAL_GRAY5             ggl_color(5)
#define PAL_GRAY6             ggl_color(6)
#define PAL_GRAY7             ggl_color(7)
#define PAL_GRAY8             ggl_color(8)
#define PAL_GRAY9             ggl_color(9)
#define PAL_GRAY10            ggl_color(10)
#define PAL_GRAY11            ggl_color(11)
#define PAL_GRAY12            ggl_color(12)
#define PAL_GRAY13            ggl_color(13)
#define PAL_GRAY14            ggl_color(14)
#define PAL_GRAY15            ggl_color(15)

#define PAL_GRAY_MASK         0xF

// Theme colors for the stack
#define PAL_STK_BG            ggl_color(16)
#define PAL_STK_INDEX         ggl_color(17)
#define PAL_STK_VLINE         ggl_color(18)
#define PAL_STK_IDX_BG        ggl_color(19)
#define PAL_STK_ITEMS         ggl_color(20)
#define PAL_STK_SEL_BG        ggl_color(21)
#define PAL_STK_SEL_ITEM      ggl_color(22)
#define PAL_STK_CURSOR        ggl_color(23)

// Theme colors for the command line
#define PAL_CMD_BG            ggl_color(24)
#define PAL_CMD_TEXT          ggl_color(25)
#define PAL_CMD_SEL_BG        ggl_color(26)
#define PAL_CMD_SELTEXT       ggl_color(27)
#define PAL_CMD_CURSOR_BG     ggl_color(28)
#define PAL_CMD_CURSOR        ggl_color(29)
#define PAL_DIV_LINE          ggl_color(30)

// Theme colors for menu
#define PAL_MENU_BG           ggl_color(31)
#define PAL_MENU_TEXT         ggl_color(33)
#define PAL_MENU_DIR          ggl_color(35)
#define PAL_MENU_DIR_BG       ggl_color(36)
#define PAL_MENU_HLINE        ggl_color(37)
#define PAL_MENU_FOCUS_HLINE  ggl_color(38)
#define PAL_MENU_PRESS_BG     ggl_color(39)

// Obsolete
#define PAL_MENU_INV_TEXT     ggl_color(34)
#define PAL_MENU_INV_BG       ggl_color(32)
#define PAL_MENU_PRESS_INV_BG ggl_color(40)


// Theme colors for status area
#define PAL_STA_BG            ggl_color(41)
#define PAL_STA_TEXT          ggl_color(42)
#define PAL_STA_ANNPRESS      ggl_color(43)
#define PAL_STA_ANN           ggl_color(44)
#define PAL_STA_BAT           ggl_color(45)
#define PAL_STA_UFLAG0        ggl_color(46)
#define PAL_STA_UFLAG1        ggl_color(47)

#define PAL_HLP_BG            ggl_color(48)
#define PAL_HLP_TEXT          ggl_color(49)
#define PAL_HLP_LINES         ggl_color(50)


#define PAL_FORM_BG           ggl_color(51)
#define PAL_FORM_TEXT         ggl_color(52)
#define PAL_FORM_SELTEXT      ggl_color(53)
#define PAL_FORM_SEL_BG       ggl_color(54)
#define PAL_FORM_CURSOR       ggl_color(55)


// Add additional theme colors here


// up to PALETTE_SIZE palette colors


// Default Palette values
// stick to 16 grays mode by default

#define THEME_GRAY0               255, 255, 255
#define THEME_GRAY1               238, 238, 238
#define THEME_GRAY2               221, 221, 221
#define THEME_GRAY3               204, 204, 204
#define THEME_GRAY4               187, 187, 187
#define THEME_GRAY5               170, 170, 170
#define THEME_GRAY6               153, 153, 153
#define THEME_GRAY7               136, 136, 136
#define THEME_GRAY8               119, 119, 119
#define THEME_GRAY9               102, 102, 102
#define THEME_GRAY10              85, 85, 85
#define THEME_GRAY11              68, 68, 68
#define THEME_GRAY12              51, 51, 51
#define THEME_GRAY13              34, 34, 34
#define THEME_GRAY14              17, 17, 17
#define THEME_GRAY15              0, 0, 0


// Theme colors for stack
// Default Palette values for GRAYSCALE hardware
// stick to 16 grays mode by default

// Theme colors for stack
#define THEME_STK_BG              THEME_GRAY0
#define THEME_STK_INDEX           THEME_GRAY15
#define THEME_STK_VLINE           THEME_GRAY8
#define THEME_STK_IDX_BG          THEME_GRAY0
#define THEME_STK_ITEMS           THEME_GRAY15
#define THEME_STK_SEL_BG          THEME_GRAY6
#define THEME_STK_SEL_ITEM        THEME_GRAY15
#define THEME_STK_CURSOR          THEME_GRAY15

// Theme colors for command line
#define THEME_CMD_BG              THEME_GRAY2
#define THEME_CMD_TEXT            THEME_GRAY15
#define THEME_CMD_SEL_BG          THEME_GRAY6
#define THEME_CMD_SELTEXT         THEME_GRAY15
#define THEME_CMD_CURSOR_BG       THEME_GRAY15
#define THEME_CMD_CURSOR          THEME_GRAY0
#define THEME_DIV_LINE            THEME_GRAY8

// Theme colors for menu
#define THEME_MENU_BG             THEME_GRAY15
#define THEME_MENU_INV_BG         THEME_GRAY0
#define THEME_MENU_TEXT           THEME_GRAY0
#define THEME_MENU_INV_TEXT       THEME_GRAY15
#define THEME_MENU_DIR            THEME_GRAY1
#define THEME_MENU_DIR_BG         THEME_GRAY14
#define THEME_MENU_HLINE          THEME_GRAY8
#define THEME_MENU_FOCUS_HLINE    THEME_GRAY0
#define THEME_MENU_PRESS_BG       255, 0, 0
#define THEME_MENU_PRESS_INV_BG   255, 0, 0

// Theme colors for status area
#define THEME_STA_BG              THEME_GRAY0
#define THEME_STA_TEXT            THEME_GRAY15
#define THEME_STA_ANNPRESS        THEME_GRAY8
#define THEME_STA_ANN             THEME_GRAY15
#define THEME_STA_BAT             THEME_GRAY15
#define THEME_STA_UFLAG0          THEME_GRAY8
#define THEME_STA_UFLAG1          THEME_GRAY15

// Theme colors for help and popup messages
#define THEME_HLP_BG              THEME_GRAY0
#define THEME_HLP_TEXT            THEME_GRAY15
#define THEME_HLP_LINES           THEME_GRAY8

// Theme colors for Forms
#define THEME_FORMBACKGROUND      THEME_GRAY0
#define THEME_FORMTEXT            THEME_GRAY15
#define THEME_FORMSELTEXT         THEME_GRAY15
#define THEME_FORMSEL_BG          THEME_GRAY6
#define THEME_FORMCURSOR          THEME_GRAY15

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
#if LCD_H > 150
#define DEF_FNT_1STK_HEIGHT (DEF_FNT_STK_HEIGHT + 8)
#else
#define DEF_FNT_1STK_HEIGHT (DEF_FNT_STK_HEIGHT + 4)
#endif
#define DEF_FNT_CMDL_HEIGHT DEF_FNT_1STK_HEIGHT
#define DEF_FNT_CURS_HEIGHT (DEF_FNT_STK_HEIGHT - 1)
#define DEF_FNT_FORM_HEIGHT DEF_FNT_STK_HEIGHT
#define DEF_FNT_PLOT_HEIGHT DEF_FNT_STK_HEIGHT
#if LCD_H > 150
#define DEF_FNT_MENU_HEIGHT (LCD_H / 22)
#else
#define DEF_FNT_MENU_HEIGHT (LCD_H / 13)
#endif
#define DEF_FNT_STAT_HEIGHT DEF_FNT_MENU_HEIGHT
#define DEF_FNT_HELP_HEIGHT DEF_FNT_MENU_HEIGHT
#define DEF_FNT_HLPT_HEIGHT DEF_FNT_MENU_HEIGHT

// Command line

word_p         halSaveCmdLine();
int32_t        halRestoreCmdLine(word_p data);

// Insert text, open new commandline if needed
void           uiOpenAndInsertTextN(byte_p start, byte_p end);

extern int32_t ui_visibleline, ui_nlines;
extern int32_t ui_currentline, ui_prevline;
extern int32_t ui_islinemodified;
extern int32_t ui_cursorx, ui_cursoroffset;
extern int32_t ui_visiblex;

void           uiSetCmdLineState(int32_t state);
int32_t        uiGetCmdLineState();

void           uiEnsureCursorVisible();
void           uiModifyLine(int dontaddnewline);
void           uiExtractLine(int32_t line);
byte_p         uiFindNumberStart(byte_p *endofnum, int32_t *flagsptr);
word_p         uiGetCmdLineText();
int32_t        uiSetCmdLineText(word_p text);
void           uiOpenCmdLine(int32_t mode);
void           uiCloseCmdLine();
void           uiSetCurrentLine(int32_t line);
int32_t        uiInsertCharacters(byte_p string);
int32_t        uiInsertCharactersN(byte_p string, byte_p end);
void           uiRemoveCharacters(int32_t length);

void           uiStretchCmdLine(int32_t addition);
void           uiAutocompleteUpdate();
void           uiAutocompNext();
void           uiAutocompPrev();
void           uiAutocompInsert();

byte_p         uiAutocompStringStart();
byte_p         uiAutocompStringEnd();
byte_p         uiAutocompStringTokEnd();

void           uiSetSelectionStart();
void           uiSetSelectionEnd();
word_p         uiExtractSelection();
int32_t        uiDeleteSelection();

int32_t        uiGetIndentLevel(int32_t *isemptyline);

void           uiSeparateToken();

void           uiMoveCursor(int32_t offset);
void           uiCursorLeft(int32_t nchars);
void           uiCursorRight(int32_t nchars);
void           uiCursorDown(int32_t nlines);
void           uiCursorUp(int32_t nlines);
void           uiCursorEndOfLine();
void           uiCursorStartOfLine();
void           uiCursorStartOfText();
void           uiCursorEndOfText();
void           uiCursorPageLeft();
void           uiCursorPageUp();
void           uiCursorPageDown();
void           uiCursorPageRight();

// Soft menus
int32_t        uiCountMenuItems(WORD MenuCode, word_p menu);
word_p         uiGetLibMenu(int64_t MenuCode);
word_p         uiGetMenuItem(int64_t MenuCode, word_p menu, int32_t item);
word_p         uiGetMenuItemAction(word_p item, int32_t shift);
word_p         uiGetMenuItemHelp(word_p item);
void           uiDrawMenuItem(gglsurface *scr, word_p item);
void           uiDrawHelpMenuItem(gglsurface *scr, word_p item);
word_p         uiGetLibHelp(word_p Object);

word_p         uiGetLibMsg(WORD MsgCode);

// RPL code execution from UI
void           uiCmdRun(WORD Opcode);
int32_t        uiCmdRunTransparent(WORD Opcode, int32_t nargs, int32_t nres);

// Forms
void           uiUpdateForm(word_p form);

#endif // UI_H
