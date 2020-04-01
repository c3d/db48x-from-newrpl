/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef UI_H
#define UI_H

#include <newrpl.h>
#include <utf8lib.h>
#include <hal_api.h>

#define UNUSED_ARGUMENT(a) (void)(a)

// HERE'S THE LANGUAGE OF THE ROM
// CHANGE THIS FOR OTHER LANGUAGES
// ONLY THE MATCHING LANGUAGE WILL BE COMPILED AND INCLUDED
#define UI_LANG_ENGLISH 1

// CONSTANTS THAT DEFINE THE LOOK OF THE USER INTERFACE
#define STATUSAREA_X  (66*SCREEN_WIDTH)/131
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
