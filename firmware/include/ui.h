/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#ifndef UI_H
#define UI_H

#define UNUSED_ARGUMENT(a) (void)(a)

#ifndef NEWRPL_H
// DEFINE COMMON TYPES
#include <stdint.h>

typedef uint16_t HALFWORD;
typedef uint32_t WORD;
typedef uint8_t BYTE;
typedef WORD *WORDPTR;
typedef BYTE   *BYTEPTR;
typedef int32_t BINT;
typedef uint32_t UBINT;
typedef int64_t BINT64;
typedef uint64_t UBINT64;
#if defined(__LP64__) || defined(_WIN64)
typedef uint64_t PTR2NUMBER;
#else
typedef uint32_t PTR2NUMBER;
#endif


#endif



#ifndef _GGL_H
#include <ggl.h>
#endif
#ifndef UTF8LIB_H
#include <utf8lib.h>
#endif
#ifndef _HAL_API_H
#include <hal_api.h>
#endif

extern BINT ui_visibleline,ui_nlines;
extern BINT ui_currentline,ui_prevline;
extern BINT ui_islinemodified;
extern BINT ui_cursorx,ui_cursoroffset;
extern BINT ui_visiblex;


extern void uiSetCmdLineState(BINT state);
extern BINT uiGetCmdLineState();

extern void uiEnsureCursorVisible();
extern void uiModifyLine();
extern void uiExtractLine(BINT line);
extern BYTEPTR uiFindNumberStart();
extern WORDPTR uiGetCmdLineText();
extern void uiSetCmdLineText(WORDPTR text);
extern void uiOpenCmdLine(BINT mode);
extern void uiCloseCmdLine();
extern void uiInsertCharacters(BYTEPTR string);
extern void uiInsertCharactersN(BYTEPTR string,BYTEPTR end);
extern void uiRemoveCharacters(BINT length);

extern void uiSeparateToken();

extern void uiMoveCursor(BINT offset);
extern void uiCursorLeft(BINT nchars);
extern void uiCursorRight(BINT nchars);




#endif // UI_H
