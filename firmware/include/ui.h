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

#define PAL_GRAY0                0
#define PAL_GRAY1                1
#define PAL_GRAY2                2
#define PAL_GRAY3                3
#define PAL_GRAY4                4
#define PAL_GRAY5                5
#define PAL_GRAY6                6
#define PAL_GRAY7                7
#define PAL_GRAY8                8
#define PAL_GRAY9                9
#define PAL_GRAY10               10
#define PAL_GRAY11               11
#define PAL_GRAY12               12
#define PAL_GRAY13               13
#define PAL_GRAY14               14
#define PAL_GRAY15               15

// Theme colors for the stack
#define PAL_STK_BACKGND          16
#define PAL_STK_INDEX            17
#define PAL_STK_VLINE            18
#define PAL_STK_IDXBACKGND       19
#define PAL_STK_ITEMS            20
#define PAL_STK_SELBKGND         21
#define PAL_STK_SELITEM          22
#define PAL_STK_CURSOR           23

// Theme colors for the command line
#define PAL_CMD_BACKGND          24
#define PAL_CMD_TEXT             25
#define PAL_CMD_SELBACKGND       26
#define PAL_CMD_SELTEXT          27
#define PAL_CMD_CURSORBACKGND    28
#define PAL_CMD_CURSOR           29
#define PAL_DIVLINE              30

// Theme colors for menu
#define PAL_MENU_BACKGND         31
#define PAL_MENU_INVBACKGND      32
#define PAL_MENU_TEXT            33
#define PAL_MENU_INVTEXT         34
#define PAL_MENU_DIRMARK         35
#define PAL_MENU_INVDIRMARK      36
#define PAL_MENU_HLINE           37
#define PAL_MENU_FOCUSHLINE      38
#define PAL_MENU_PRESSBACKGND    39
#define PAL_MENU_PRESSINVBACKGND 40


// Theme colors for status area
#define PAL_STA_BACKGND          41
#define PAL_STA_TEXT             42
#define PAL_STA_ANNPRESS         43
#define PAL_STA_ANN              44
#define PAL_STA_BAT              45
#define PAL_STA_UFLAG0           46
#define PAL_STA_UFLAG1           47

#define PAL_HLP_BACKGND          48
#define PAL_HLP_TEXT             49
#define PAL_HLP_LINES            50


#define PAL_FORM_BACKGND         51
#define PAL_FORM_TEXT            52
#define PAL_FORM_SELTEXT         53
#define PAL_FORM_SELBACKGND      54
#define PAL_FORM_CURSOR          55


// Add additional theme colors here


// up to PALETTESIZE palette colors


// Default Palette values
// stick to 16 grays mode by default

#define THEME_GRAY0              RGB_TO_RGB16(255, 255, 255)
#define THEME_GRAY1              RGB_TO_RGB16(238, 238, 238)
#define THEME_GRAY2              RGB_TO_RGB16(221, 221, 221)
#define THEME_GRAY3              RGB_TO_RGB16(204, 204, 204)
#define THEME_GRAY4              RGB_TO_RGB16(187, 187, 187)
#define THEME_GRAY5              RGB_TO_RGB16(170, 170, 170)
#define THEME_GRAY6              RGB_TO_RGB16(153, 153, 153)
#define THEME_GRAY7              RGB_TO_RGB16(136, 136, 136)
#define THEME_GRAY8              RGB_TO_RGB16(119, 119, 119)
#define THEME_GRAY9              RGB_TO_RGB16(102, 102, 102)
#define THEME_GRAY10             RGB_TO_RGB16(85, 85, 85)
#define THEME_GRAY11             RGB_TO_RGB16(68, 68, 68)
#define THEME_GRAY12             RGB_TO_RGB16(51, 51, 51)
#define THEME_GRAY13             RGB_TO_RGB16(34, 34, 34)
#define THEME_GRAY14             RGB_TO_RGB16(17, 17, 17)
#define THEME_GRAY15             RGB_TO_RGB16(0, 0, 0)


// Theme colors for stack
#ifndef NEWRPL_COLOR
// Default Palette values for GRAYSCALE hardware
// stick to 16 grays mode by default

// Theme colors for stack
#  define THEME_STKBACKGND          PATTERN_SOLID(PAL_GRAY0)
#  define THEME_STKINDEX            PATTERN_SOLID(PAL_GRAY15)
#  define THEME_STKVLINE            PATTERN_SOLID(PAL_GRAY8)
#  define THEME_STKIDXBACKGND       PATTERN_SOLID(PAL_GRAY0)
#  define THEME_STKITEMS            PATTERN_SOLID(PAL_GRAY15)
#  define THEME_STKSELBKGND         PATTERN_SOLID(PAL_GRAY6)
#  define THEME_STKSELITEM          PATTERN_SOLID(PAL_GRAY15)
#  define THEME_STKCURSOR           PATTERN_SOLID(PAL_GRAY15)

// Theme colors for command line
#  define THEME_CMDBACKGND          PATTERN_SOLID(PAL_GRAY0)
#  define THEME_CMDTEXT             PATTERN_SOLID(PAL_GRAY15)
#  define THEME_CMDSELBACKGND       PATTERN_SOLID(PAL_GRAY6)
#  define THEME_CMDSELTEXT          PATTERN_SOLID(PAL_GRAY15)
#  define THEME_CMDCURSORBACKGND    PATTERN_SOLID(PAL_GRAY15)
#  define THEME_CMDCURSOR           PATTERN_SOLID(PAL_GRAY0)
#  define THEME_DIVLINE             PATTERN_SOLID(PAL_GRAY8)

// Theme colors for menu
#  define THEME_MENUBACKGND         PATTERN_SOLID(PAL_GRAY15)
#  define THEME_MENUINVBACKGND      PATTERN_SOLID(PAL_GRAY0)
#  define THEME_MENUTEXT            PATTERN_SOLID(PAL_GRAY0)
#  define THEME_MENUINVTEXT         PATTERN_SOLID(PAL_GRAY15)
#  define THEME_MENUDIRMARK         PATTERN_SOLID(PAL_GRAY8)
#  define THEME_MENUINVDIRMARK      PATTERN_SOLID(PAL_GRAY6)
#  define THEME_MENUHLINE           PATTERN_SOLID(PAL_GRAY8)
#  define THEME_MENUFOCUSHLINE      PATTERN_SOLID(PAL_GRAY0)
#  define THEME_MENUPRESSBACKGND    RGB_TO_RGB16(255, 0, 0)
#  define THEME_MENUPRESSINVBACKGND RGB_TO_RGB16(255, 0, 0)

// Theme colors for status area
#  define THEME_STABACKGND          PATTERN_SOLID(PAL_GRAY0)
#  define THEME_STATEXT             PATTERN_SOLID(PAL_GRAY15)
#  define THEME_STAANNPRESS         PATTERN_SOLID(PAL_GRAY15)
#  define THEME_STAANN              PATTERN_SOLID(PAL_GRAY8)
#  define THEME_STABAT              PATTERN_SOLID(PAL_GRAY15)
#  define THEME_STAUFLAG0           PATTERN_SOLID(PAL_GRAY8)
#  define THEME_STAUFLAG1           PATTERN_SOLID(PAL_GRAY15)

// Theme colors for help and popup messages
#  define THEME_HLPBACKGND          PATTERN_SOLID(PAL_GRAY0)
#  define THEME_HLPTEXT             PATTERN_SOLID(PAL_GRAY15)
#  define THEME_HLPLINES            PATTERN_SOLID(PAL_GRAY8)

// Theme colors for Forms
#  define THEME_FORMBACKGND         PATTERN_SOLID(PAL_GRAY0)
#  define THEME_FORMTEXT            PATTERN_SOLID(PAL_GRAY15)
#  define THEME_FORMSELTEXT         PATTERN_SOLID(PAL_GRAY15)
#  define THEME_FORMSELBACKGND      PATTERN_SOLID(PAL_GRAY6)
#  define THEME_FORMCURSOR          PATTERN_SOLID(PAL_GRAY15)

#else // NEWRPL_COLOR

#  define THEME_STKBACKGND          RGB_TO_RGB16(255, 255, 255)
#  define THEME_STKINDEX            RGB_TO_RGB16(0, 0, 0)
#  define THEME_STKVLINE            RGB_TO_RGB16(128, 128, 128)
#  define THEME_STKIDXBACKGND       RGB_TO_RGB16(255, 255, 255)
#  define THEME_STKITEMS            RGB_TO_RGB16(0, 0, 0)
#  define THEME_STKSELBKGND         RGB_TO_RGB16(192, 192, 192)
#  define THEME_STKSELITEM          RGB_TO_RGB16(0, 0, 0)
#  define THEME_STKCURSOR           RGB_TO_RGB16(0, 0, 0)

// Theme colors for command line
#  define THEME_CMDBACKGND          RGB_TO_RGB16(255, 255, 255)
#  define THEME_CMDTEXT             RGB_TO_RGB16(0, 0, 0)
#  define THEME_CMDSELBACKGND       RGB_TO_RGB16(192, 192, 192)
#  define THEME_CMDSELTEXT          RGB_TO_RGB16(0, 0, 0)
#  define THEME_CMDCURSORBACKGND    RGB_TO_RGB16(0, 0, 0)
#  define THEME_CMDCURSOR           RGB_TO_RGB16(255, 255, 255)
#  define THEME_DIVLINE             RGB_TO_RGB16(128, 128, 128)

// Theme colors for menu
#  define THEME_MENUBACKGND         RGB_TO_RGB16(0, 0, 0)
#  define THEME_MENUINVBACKGND      RGB_TO_RGB16(220, 220, 220)
#  define THEME_MENUTEXT            RGB_TO_RGB16(255, 255, 255)
#  define THEME_MENUINVTEXT         RGB_TO_RGB16(0, 0, 0)
#  define THEME_MENUDIRMARK         RGB_TO_RGB16(128, 128, 128)
#  define THEME_MENUINVDIRMARK      RGB_TO_RGB16(192, 192, 192)
#  define THEME_MENUHLINE           RGB_TO_RGB16(128, 128, 128)
#  define THEME_MENUFOCUSHLINE      RGB_TO_RGB16(255, 255, 255)
#  define THEME_MENUPRESSBACKGND    RGB_TO_RGB16(255, 0, 0)
#  define THEME_MENUPRESSINVBACKGND RGB_TO_RGB16(255, 0, 0)


// Theme colors for status area
#  define THEME_STABACKGND          RGB_TO_RGB16(255, 255, 255)
#  define THEME_STATEXT             RGB_TO_RGB16(0, 0, 0)
#  define THEME_STAANNPRESS         RGB_TO_RGB16(0, 0, 0)
#  define THEME_STAANN              RGB_TO_RGB16(128, 128, 128)
#  define THEME_STABAT              RGB_TO_RGB16(0, 0, 0)
#  define THEME_STAUFLAG0           RGB_TO_RGB16(128, 128, 128)
#  define THEME_STAUFLAG1           RGB_TO_RGB16(0, 0, 0)

// Theme colors for help and popup messages
#  define THEME_HLPBACKGND          RGB_TO_RGB16(255, 255, 255)
#  define THEME_HLPTEXT             RGB_TO_RGB16(0, 0, 0)
#  define THEME_HLPLINES            RGB_TO_RGB16(128, 128, 128)

// Theme colors for Forms
#  define THEME_FORMBACKGND         RGB_TO_RGB16(255, 255, 255)
#  define THEME_FORMTEXT            RGB_TO_RGB16(0, 0, 0)
#  define THEME_FORMSELTEXT         RGB_TO_RGB16(0, 0, 0)
#  define THEME_FORMSELBACKGND      RGB_TO_RGB16(192, 192, 192)
#  define THEME_FORMCURSOR          RGB_TO_RGB16(0, 0, 0)
#endif // NEWRPL_COLOR

#define UNUSED_ARGUMENT(a) (void) (a)

// HERE'S THE LANGUAGE OF THE ROM
// CHANGE THIS FOR OTHER LANGUAGES
// ONLY THE MATCHING LANGUAGE WILL BE COMPILED AND INCLUDED
#define UI_LANG_ENGLISH    1

// CONSTANTS THAT DEFINE THE LOOK OF THE USER INTERFACE
#ifndef STATUSAREA_X
#  define STATUSAREA_X (66 * SCREEN_WIDTH) / 131
#endif
// BASIC HEIGHT OF SCREEN AREAS IN PIXELS - THIS IS HARDWARE DEPENDENT SO WE NEED TO ALLOW TARGET HEADER TO DEFINE IF
// NEEDED
#ifndef MENU2_HEIGHT
#  define MENU2_HEIGHT (2 * (1 + FONT_HEIGHT(FONT_MENU)) + 2)
#endif
#ifndef MENU1_HEIGHT
#  define MENU1_HEIGHT (FONT_HEIGHT(FONT_MENU) + 3)
#endif
#define CMDLINE_HEIGHT (FONT_HEIGHT(FONT_CMDLINE) + 2)

#define MENU_TAB_WIDTH     (((SCREEN_WIDTH - 5) / 6) + 1)
#define BITSPERPIXEL       (32 / PIXELS_PER_WORD)

// PREFERRED HEIGHT OF THE FONTS
#define DEF_FNTSTK_HEIGHT  (SCREEN_HEIGHT / 10)
#define DEF_FNT1STK_HEIGHT DEF_FNTSTK_HEIGHT
#define DEF_FNTCMDL_HEIGHT DEF_FNTSTK_HEIGHT
#define DEF_FNTFORM_HEIGHT DEF_FNTSTK_HEIGHT
#define DEF_FNTPLOT_HEIGHT DEF_FNTSTK_HEIGHT
#define DEF_FNTMENU_HEIGHT (SCREEN_HEIGHT / 13)
#define DEF_FNTSTAT_HEIGHT DEF_FNTMENU_HEIGHT
#define DEF_FNTHELP_HEIGHT DEF_FNTMENU_HEIGHT
#define DEF_FNTHLPT_HEIGHT DEF_FNTMENU_HEIGHT

// COMMAND LINE

WORDPTR     halSaveCmdLine();
BINT        halRestoreCmdLine(WORDPTR data);
// INSERT TEXT, OPEN NEW CMD LINE IF NEEDED
void        uiOpenAndInsertTextN(BYTEPTR start, BYTEPTR end);

extern BINT ui_visibleline, ui_nlines;
extern BINT ui_currentline, ui_prevline;
extern BINT ui_islinemodified;
extern BINT ui_cursorx, ui_cursoroffset;
extern BINT ui_visiblex;

void        uiSetCmdLineState(BINT state);
BINT        uiGetCmdLineState();

void        uiEnsureCursorVisible();
void        uiModifyLine(int dontaddnewline);
void        uiExtractLine(BINT line);
BYTEPTR     uiFindNumberStart(BYTEPTR *endofnum, BINT *flagsptr);
WORDPTR     uiGetCmdLineText();
BINT        uiSetCmdLineText(WORDPTR text);
void        uiOpenCmdLine(BINT mode);
void        uiCloseCmdLine();
void        uiSetCurrentLine(BINT line);
BINT        uiInsertCharacters(BYTEPTR string);
BINT        uiInsertCharactersN(BYTEPTR string, BYTEPTR end);
void        uiRemoveCharacters(BINT length);

void        uiStretchCmdLine(BINT addition);
void        uiAutocompleteUpdate();
void        uiAutocompNext();
void        uiAutocompPrev();
void        uiAutocompInsert();

BYTEPTR     uiAutocompStringStart();
BYTEPTR     uiAutocompStringEnd();
BYTEPTR     uiAutocompStringTokEnd();

void        uiSetSelectionStart();
void        uiSetSelectionEnd();
WORDPTR     uiExtractSelection();
BINT        uiDeleteSelection();

BINT        uiGetIndentLevel(BINT *isemptyline);

void        uiSeparateToken();

void        uiMoveCursor(BINT offset);
void        uiCursorLeft(BINT nchars);
void        uiCursorRight(BINT nchars);
void        uiCursorDown(BINT nlines);
void        uiCursorUp(BINT nlines);
void        uiCursorEndOfLine();
void        uiCursorStartOfLine();
void        uiCursorStartOfText();
void        uiCursorEndOfText();
void        uiCursorPageLeft();
void        uiCursorPageUp();
void        uiCursorPageDown();
void        uiCursorPageRight();

// SOFT MENUS

BINT        uiCountMenuItems(WORD MenuCode, WORDPTR menu);
WORDPTR     uiGetLibMenu(BINT64 MenuCode);
WORDPTR     uiGetMenuItem(BINT64 MenuCode, WORDPTR menu, BINT item);
WORDPTR     uiGetMenuItemAction(WORDPTR item, BINT shift);
WORDPTR     uiGetMenuItemHelp(WORDPTR item);
void        uiDrawMenuItem(WORDPTR item, BINT palette_color, BINT palette_bkcolor, DRAWSURFACE *scr);

WORDPTR     uiGetLibHelp(WORDPTR Object);

WORDPTR     uiGetLibMsg(WORD MsgCode);

// RPL CODE EXECUTION FROM UI
void        uiCmdRun(WORD Opcode);
BINT        uiCmdRunTransparent(WORD Opcode, BINT nargs, BINT nresults);

// FORMS
void        uiUpdateForm(WORDPTR form);

#endif // UI_H
