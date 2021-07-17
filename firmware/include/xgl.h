#ifndef XGL_H
#define XGL_H

// Generic definitions for both color and gray modes

// Convert from RGB (0-255) to RGB16(5-6-5)
#define RGB_TO_RGB16(red,green,blue) ((((red)&0xf8)<<8)|(((green)&0xfc)<<3)|(((blue)&0xf8)>>3))

// Pack RGB16 components (red=0-31, green=0-63, blue=0-31)
#define PACK_RGB16(red,green,blue) ((((red)&0x1f)<<11)|(((green)&0x3f)<<5)|(((blue)&0x1f)))

// Extract RGB red component from RGB16 color (bit expand to 0-255 range)
#define RGBRED(rgb16) ( ((rgb16)&0x8000)? (((rgb16)>>8)|7) : ((rgb16)>>8)&0xf8)
// Extract RGB green component from RGB16 color (bit expand to 0-255 range)
#define RGBGREEN(rgb16) ( ((rgb16)&0x400)? ((((rgb16)>>3)&0xff)|3) : ((rgb16)>>3)&0xfc)
// Extract RGB blue component from RGB16 color (bit expand to 0-255 range)
#define RGBBLUE(rgb16) ( ((rgb16)&0x10)? ((((rgb16)<<3)&0xff)|7) : ((rgb16)<<3)&0xfc)

// Extract RGB components from a 16-grays color value
#define G2RGBRED(gray) ( (((gray)&0xf)<<4) | (((gray)&0x8)? 0xf:0) )
#define G2RGBGREEN(gray) ( (((gray)&0xf)<<4) | (((gray)&0x8)? 0xf:0) )
#define G2RGBBLUE(gray) ( (((gray)&0xf)<<4) | (((gray)&0x8)? 0xf:0) )

// Convert from RGB (0-255) to GRAY16(4-bit)
#define RGB_TO_GRAY16(red,green,blue) ((( (red)+(green)+(green)+(blue)) >> 6)&0xf)


#define REPEAT_NIBBLE(nib) (((nib)&0xf)|(((nib)&0xf)<<4))
#define REPEAT_BYTE(byte) (((byte)&0xff)|(((byte)&0xff)<<8))
#define REPEAT_HALFWORD(hword) (((hword)&0xffff)|(((hword)&0xffff)<<16))

#define PATTERN_SOLID(gray) REPEAT_HALFWORD(REPEAT_BYTE(REPEAT_NIBBLE(gray)))
#define PATTERN_2COL(dot1,dot2) REPEAT_HALFWORD(REPEAT_BYTE( ((dot1)&0xf) | (((dot2)&0xf)<<4) )))
#define PATTERN_4COL(dot1,dot2,dot3,dot4) REPEAT_HALFWORD( ((dot1)&0xf) | (((dot2)&0xf)<<4)| (((dot3)&0xf)<<8)| (((dot4)&0xf)<<12) )

// Theming engine definitions, include early to allow for target-specific overrides

// Default palette size, entries 0-15 are for grayscale conversion, entries above 16 are customizable Theme colors for different elements of the UI
#define PALETTESIZE     64
#define PALETTEMASK     63

#define IS_PALETTE_COLOR 0x10000

// Global palette, can be used for grayscale conversion or for themes
extern int cgl_palette[PALETTESIZE];




// Select the proper library according to the target (4-bit grayscale or full color)
#ifdef TARGET_PC
#ifdef TARGET_PC_PRIMEG1
#include "cgl.h"
#else


// Add some stub functions to map cgl to ggl
#define cgl_initscr ggl_initscr
#define cgl_rect ggl_rect
#define cgl_cliprect ggl_cliprect
#define cgl_cliphline ggl_cliphline
#define cgl_clipvline ggl_clipvline
#define cgl_bitbltclip ggl_bitbltclip

#define cgl_filter ggl_filter

#define cgl_fltreplace ggl_fltreplace

// Use a palette entry
#define cgl_mkcolor(color) (cgl_palette[(color)&PALETTEMASK])

// Set a palette index entry
#define cgl_setpalette(index,color) { cgl_palette[(index)&PALETTEMASK]=(color); }


#include "ggl.h"
#endif
#endif

#ifdef TARGET_PRIME1
#include "cgl.h"
//#include "ggl.h"
#endif

#ifdef TARGET_50G

// Add some stub functions to map cgl to ggl
#define cgl_initscr ggl_initscr
#define cgl_rect ggl_rect
#define cgl_cliprect ggl_cliprect
#define cgl_cliphline ggl_cliphline
#define cgl_clipvline ggl_clipvline
#define cgl_bitbltclip ggl_bitbltclip

#define cgl_filter ggl_filter

#define cgl_fltreplace ggl_fltreplace

// Use a palette entry
#define cgl_mkcolor(color) (cgl_palette[(color)&PALETTEMASK])

// Set a palette index entry
#define cgl_setpalette(index,color) { cgl_palette[(index)&PALETTEMASK]=(color); }

#include "ggl.h"
#endif

#ifdef TARGET_39GS

// Add some stub functions to map cgl to ggl
#define cgl_initscr ggl_initscr
#define cgl_rect ggl_rect
#define cgl_cliprect ggl_cliprect
#define cgl_cliphline ggl_cliphline
#define cgl_clipvline ggl_clipvline
#define cgl_bitbltclip ggl_bitbltclip

#define cgl_filter ggl_filter

#define cgl_fltreplace ggl_fltreplace

// Use a palette entry
#define cgl_mkcolor(color) (cgl_palette[(color)&PALETTEMASK])

// Set a palette index entry
#define cgl_setpalette(index,color) { cgl_palette[(index)&PALETTEMASK]=(color); }

#include "ggl.h"
#endif

#ifdef TARGET_40GS

// Add some stub functions to map cgl to ggl
#define cgl_initscr ggl_initscr
#define cgl_rect ggl_rect
#define cgl_cliprect ggl_cliprect
#define cgl_cliphline ggl_cliphline
#define cgl_clipvline ggl_clipvline
#define cgl_bitbltclip ggl_bitbltclip

#define cgl_filter ggl_filter

#define cgl_fltreplace ggl_fltreplace

// Use a palette entry
#define cgl_mkcolor(color) (cgl_palette[(color)&PALETTEMASK])

// Set a palette index entry
#define cgl_setpalette(index,color) { cgl_palette[(index)&PALETTEMASK]=(color); }

#include "ggl.h"
#endif

#ifdef TARGET_48GII

// Add some stub functions to map cgl to ggl
#define cgl_initscr ggl_initscr
#define cgl_rect ggl_rect
#define cgl_cliprect ggl_cliprect
#define cgl_cliphline ggl_cliphline
#define cgl_clipvline ggl_clipvline
#define cgl_bitbltclip ggl_bitbltclip

#define cgl_filter ggl_filter

#define cgl_fltreplace ggl_fltreplace

// Use a palette entry
#define cgl_mkcolor(color) (cgl_palette[(color)&PALETTEMASK])

// Set a palette index entry
#define cgl_setpalette(index,color) { cgl_palette[(index)&PALETTEMASK]=(color); }

#include "ggl.h"
#endif

#endif // XGL_H
