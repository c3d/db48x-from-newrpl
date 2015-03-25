#ifndef UI_H
#define UI_H

#ifndef NEWRPL_H
#define UNUSED_ARGUMENT(a) (void)(a)

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

#endif



#ifndef _GGL_H
#include <ggl.h>
#endif
#ifndef _HAL_API_H
#include <hal_api.h>
#endif

extern BINT ui_visibleline,ui_nlines;
extern BINT ui_currentline,ui_prevline;
extern BINT ui_islinemodified;
extern BINT ui_cursorx,ui_cursoroffset;
extern BINT ui_visiblex;

extern void uiEnsureCursorVisible();
extern void uiModifyLine();
extern void uiExtractLine();


#endif // UI_H
