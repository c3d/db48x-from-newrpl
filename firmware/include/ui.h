/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef UI_H
#define UI_H

// Color to grayscale conversion helpers


// Palette color names

// First 16 are reserved for 16 grays (for grayscale compatibility)

#define PAL_GRAY0   0
#define PAL_GRAY1   1
#define PAL_GRAY2   2
#define PAL_GRAY3   3
#define PAL_GRAY4   4
#define PAL_GRAY5   5
#define PAL_GRAY6   6
#define PAL_GRAY7   7
#define PAL_GRAY8   8
#define PAL_GRAY9   9
#define PAL_GRAY10  10
#define PAL_GRAY11  11
#define PAL_GRAY12  12
#define PAL_GRAY13  13
#define PAL_GRAY14  14
#define PAL_GRAY15  15

// Theme colors for the stack
#define PAL_STKBACKGND 16
#define PAL_STKINDEX   17
#define PAL_STKVLINE   18
#define PAL_STKITEM1   19
#define PAL_STKITEMS   20
#define PAL_STKSELBKGND 21
#define PAL_STKSELITEM  22
#define PAL_STKCURSOR   23

// Theme colors for the command line
#define PAL_CMDBACKGND  24
#define PAL_CMDTEXT     25
#define PAL_CMDSELBACKGND 26
#define PAL_CMDSELTEXT  27
#define PAL_CMDCURSORBACKGND 28
#define PAL_CMDCURSOR   29
#define PAL_DIVLINE     30

// Theme colors for menu
#define PAL_MENUBACKGND 31
#define PAL_MENUINVBACKGND 32
#define PAL_MENUTEXT    33
#define PAL_MENUINVTEXT 34
#define PAL_MENUDIRMARK 35
#define PAL_MENUINVDIRMARK 36
#define PAL_MENUHLINE   37
#define PAL_MENUFOCUSHLINE 38
#define PAL_MENUPRESSBACKGND 39
#define PAL_MENUPRESSINVBACKGND 40


// Theme colors for status area
#define PAL_STABACKGND  41
#define PAL_STATEXT     42
#define PAL_STAANNPRESS 43
#define PAL_STAANN      44
#define PAL_STABAT      45
#define PAL_STAUFLAG0   46
#define PAL_STAUFLAG1   47
// Add additional theme colors here



// up to PALETTESIZE palette colors


// Default Palette values
// stick to 16 grays mode by default

#define THEME_GRAY0   RGB_TO_RGB16(255,255,255)
#define THEME_GRAY1   RGB_TO_RGB16(238,238,238)
#define THEME_GRAY2   RGB_TO_RGB16(221,221,221)
#define THEME_GRAY3   RGB_TO_RGB16(204,204,204)
#define THEME_GRAY4   RGB_TO_RGB16(187,187,187)
#define THEME_GRAY5   RGB_TO_RGB16(170,170,170)
#define THEME_GRAY6   RGB_TO_RGB16(153,153,153)
#define THEME_GRAY7   RGB_TO_RGB16(136,136,136)
#define THEME_GRAY8   RGB_TO_RGB16(119,119,119)
#define THEME_GRAY9   RGB_TO_RGB16(102,102,102)
#define THEME_GRAY10   RGB_TO_RGB16(85,85,85)
#define THEME_GRAY11   RGB_TO_RGB16(68,68,68)
#define THEME_GRAY12   RGB_TO_RGB16(51,51,51)
#define THEME_GRAY13   RGB_TO_RGB16(34,34,34)
#define THEME_GRAY14   RGB_TO_RGB16(17,17,17)
#define THEME_GRAY15   RGB_TO_RGB16(0,0,0)

#define THEME_STKBACKGND RGB_TO_RGB16(255,255,255)
#define THEME_STKINDEX   RGB_TO_RGB16(0,0,0)
#define THEME_STKVLINE   RGB_TO_RGB16(128,128,128)
#define THEME_STKITEM1   RGB_TO_RGB16(0,0,0)
#define THEME_STKITEMS   RGB_TO_RGB16(0,0,0)
#define THEME_STKSELBKGND RGB_TO_RGB16(192,192,192)
#define THEME_STKSELITEM  RGB_TO_RGB16(0,0,0)
#define THEME_STKCURSOR   RGB_TO_RGB16(0,0,0)

#define THEME_CMDBACKGND  RGB_TO_RGB16(255,255,255)
#define THEME_CMDTEXT     RGB_TO_RGB16(0,0,0)
#define THEME_CMDSELBACKGND RGB_TO_RGB16(192,192,192)
#define THEME_CMDSELTEXT  RGB_TO_RGB16(0,0,0)
#define THEME_CMDCURSORBACKGND RGB_TO_RGB16(0,0,0)
#define THEME_CMDCURSOR   RGB_TO_RGB16(255,255,255)
#define THEME_DIVLINE     RGB_TO_RGB16(128,128,128)

// Theme colors for menu
#define THEME_MENUBACKGND RGB_TO_RGB16(0,0,0)
#define THEME_MENUINVBACKGND RGB_TO_RGB16(220,220,220)
#define THEME_MENUTEXT    RGB_TO_RGB16(255,255,255)
#define THEME_MENUINVTEXT RGB_TO_RGB16(0,0,0)
#define THEME_MENUDIRMARK RGB_TO_RGB16(128,128,128)
#define THEME_MENUINVDIRMARK RGB_TO_RGB16(192,192,192)
#define THEME_MENUHLINE   RGB_TO_RGB16(128,128,128)
#define THEME_MENUFOCUSHLINE RGB_TO_RGB16(255,255,255)
#define THEME_MENUPRESSBACKGND RGB_TO_RGB16(255,0,0)
#define THEME_MENUPRESSINVBACKGND RGB_TO_RGB16(255,0,0)

// Theme colors for status area
#define THEME_STABACKGND  RGB_TO_RGB16(255,255,255)
#define THEME_STATEXT     RGB_TO_RGB16(0,0,0)
#define THEME_STAANNPRESS RGB_TO_RGB16(0,0,0)
#define THEME_STAANN      RGB_TO_RGB16(128,128,128)
#define THEME_STABAT      RGB_TO_RGB16(0,0,0)
#define THEME_STAUFLAG0   RGB_TO_RGB16(128,128,128)
#define THEME_STAUFLAG1   RGB_TO_RGB16(0,0,0)









#include <newrpl.h>
#include <utf8lib.h>
#include <hal_api.h>

#define UNUSED_ARGUMENT(a) (void)(a)

// HERE'S THE LANGUAGE OF THE ROM
// CHANGE THIS FOR OTHER LANGUAGES
// ONLY THE MATCHING LANGUAGE WILL BE COMPILED AND INCLUDED
#define UI_LANG_ENGLISH 1

// CONSTANTS THAT DEFINE THE LOOK OF THE USER INTERFACE
#ifndef STATUSAREA_X
#define STATUSAREA_X  (66*SCREEN_WIDTH)/131
#endif
// BASIC HEIGHT OF SCREEN AREAS IN PIXELS - THIS IS HARDWARE DEPENDENT SO WE NEED TO ALLOW TARGET HEADER TO DEFINE IF NEEDED
#ifndef MENU2_HEIGHT
#define MENU2_HEIGHT ((2*(1+(*halScreen.FontArray[FONT_MENU])->BitmapHeight))+2)
#endif
#ifndef MENU1_HEIGHT
#define MENU1_HEIGHT (((*halScreen.FontArray[FONT_MENU])->BitmapHeight)+3)
#endif

#define MENU_TAB_WIDTH (((SCREEN_WIDTH-5)/6)+1)
#define BITSPERPIXEL (32/PIXELS_PER_WORD)

// PREFERRED HEIGHT OF THE FONTS
#define DEF_FNTSTK_HEIGHT  (SCREEN_HEIGHT/10)
#define DEF_FNT1STK_HEIGHT DEF_FNTSTK_HEIGHT
#define DEF_FNTCMDL_HEIGHT DEF_FNTSTK_HEIGHT
#define DEF_FNTFORM_HEIGHT DEF_FNTSTK_HEIGHT
#define DEF_FNTPLOT_HEIGHT DEF_FNTSTK_HEIGHT
#define DEF_FNTMENU_HEIGHT (SCREEN_HEIGHT/13)
#define DEF_FNTSTAT_HEIGHT DEF_FNTMENU_HEIGHT

// COMMAND LINE

WORDPTR halSaveCmdLine();
BINT halRestoreCmdLine(WORDPTR data);
// INSERT TEXT, OPEN NEW CMD LINE IF NEEDED
void uiOpenAndInsertTextN(BYTEPTR start, BYTEPTR end);

extern BINT ui_visibleline, ui_nlines;
extern BINT ui_currentline, ui_prevline;
extern BINT ui_islinemodified;
extern BINT ui_cursorx, ui_cursoroffset;
extern BINT ui_visiblex;

void uiSetCmdLineState(BINT state);
BINT uiGetCmdLineState();

void uiEnsureCursorVisible();
void uiModifyLine(int dontaddnewline);
void uiExtractLine(BINT line);
BYTEPTR uiFindNumberStart(BYTEPTR * endofnum, BINT * flagsptr);
WORDPTR uiGetCmdLineText();
BINT uiSetCmdLineText(WORDPTR text);
void uiOpenCmdLine(BINT mode);
void uiCloseCmdLine();
void uiSetCurrentLine(BINT line);
BINT uiInsertCharacters(BYTEPTR string);
BINT uiInsertCharactersN(BYTEPTR string, BYTEPTR end);
void uiRemoveCharacters(BINT length);

void uiStretchCmdLine(BINT addition);
void uiAutocompleteUpdate();
void uiAutocompNext();
void uiAutocompPrev();
void uiAutocompInsert();

BYTEPTR uiAutocompStringStart();
BYTEPTR uiAutocompStringEnd();
BYTEPTR uiAutocompStringTokEnd();

void uiSetSelectionStart();
void uiSetSelectionEnd();
WORDPTR uiExtractSelection();
BINT uiDeleteSelection();

BINT uiGetIndentLevel(BINT * isemptyline);

void uiSeparateToken();

void uiMoveCursor(BINT offset);
void uiCursorLeft(BINT nchars);
void uiCursorRight(BINT nchars);
void uiCursorDown(BINT nlines);
void uiCursorUp(BINT nlines);
void uiCursorEndOfLine();
void uiCursorStartOfLine();
void uiCursorStartOfText();
void uiCursorEndOfText();
void uiCursorPageLeft();
void uiCursorPageUp();
void uiCursorPageDown();
void uiCursorPageRight();

// SOFT MENUS

BINT uiCountMenuItems(WORD MenuCode, WORDPTR menu);
WORDPTR uiGetLibMenu(BINT64 MenuCode);
WORDPTR uiGetMenuItem(BINT64 MenuCode, WORDPTR menu, BINT item);
WORDPTR uiGetMenuItemAction(WORDPTR item, BINT shift);
WORDPTR uiGetMenuItemHelp(WORDPTR item);
void uiDrawMenuItem(WORDPTR item, BINT color, DRAWSURFACE * scr);

WORDPTR uiGetLibHelp(WORDPTR Object);

WORDPTR uiGetLibMsg(WORD MsgCode);

// RPL CODE EXECUTION FROM UI
void uiCmdRun(WORD Opcode);
BINT uiCmdRunTransparent(WORD Opcode, BINT nargs, BINT nresults);

// FORMS

void uiUpdateForm(WORDPTR form);

#endif // UI_H
