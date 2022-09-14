/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */
#ifndef _GGL_H
#define _GGL_H

#include <stdint.h>
#include <target.h>
#include "common-macros.h"

// GGL provides a unified interface for three forms of graphics with different bits per pixel (BPP):
// 1. 4BPP  gray-scale graphics on gray-scale platforms (HP50g).
// 2. 16BPP RGB16 color graphics on color platforms (Prime)
// 3. 1BPP  Black and white graphics on bitmap platforms (DM42)
//
// In order to accomodate common theming across the platforms, graphics functions
// accept two forms of color input: flat color, which uses the same color for
// every pixel, and color pattern, which is a 64-bit value that creates a
// pattern of different sizes depending on the value of BITS_PER_PIXEL
//
// In 1BPP mode, the 64-bits correspond to an 8x8 bitmap pattern
// In 4BPP mode, the 64-bits correspond to a 4x4 grayscale pattern
// In 16BPP mode, the 64 bits correspond to a 2x2 color pattern
//
// In combined operations, the pattern is always aligned with the destination

#ifndef BITS_PER_PIXEL
#error Cannot build GGL without BITS_PER_PIXEL
#endif

#if BITS_PER_PIXEL == 1
#define PATTERN_WIDTH       8
#define PATTERN_HEIGHT      8
#elif BITS_PER_PIXEL == 4
#define PATTERN_WIDTH       4
#define PATTERN_HEIGHT      4
#elif BITS_PER_PIXEL == 16
#define PATTERN_WIDTH       2
#define PATTERN_HEIGHT      2
#else
#error Unknown value for BITS_PER_PIXEL
#endif

// Generic definitions for both color and gray modes
typedef union color1
{
    struct bits
    {
        uint8_t bit0    : 1;
        uint8_t bit1    : 1;
        uint8_t bit2    : 1;
        uint8_t bit3    : 1;
        uint8_t bit4    : 1;
        uint8_t bit5    : 1;
        uint8_t bit6    : 1;
        uint8_t bit7    : 1;
    } __attribute__((packed)) bits;
    uint8_t     value;
} color1_t;

typedef union color4_t
{
    struct nibbles
    {
        uint8_t low         : 4;
        uint8_t high        : 4;
    } __attribute__((packed)) nibbles;
    uint8_t value;
} color4_t;

typedef union color16
{
    struct rgb16
    {
        uint8_t blue    : 5;
        uint8_t green   : 6;
        uint8_t red     : 5;
    } __attribute__((packed)) rgb16;
    uint16_t value;
} color16_t;

#define color_t CAT(color, CAT(BITS_PER_PIXEL,_t))

typedef union pattern
{
    uint64_t    bits;
    uint64_t    color   : BITS_PER_PIXEL;
    color1_t    plane1[8];
    color4_t    plane4[8];
    color16_t   plane16[4];
} pattern_t;


static inline pattern_t ggl_solid_pattern(color_t color)
{
    uint64_t bits = 0;
    for (unsigned shift = 0; shift < 64; shift += BITS_PER_PIXEL)
        bits |= color.value << shift;
    pattern_t pat = { .bits = bits };
    return pat;
}

static inline pattern_t ggl_pattern_2_colors(color_t colors[2])
{
    uint64_t bits = 0;
    for (unsigned shift = 0; shift < 64/BITS_PER_PIXEL; shift++)
        bits |= colors[((shift + ((shift / PATTERN_WIDTH) % 2)) % 2)].value;
    pattern_t pat = { .bits = bits };
    return pat;
}

static inline pattern_t ggl_pattern_4_colors(color_t colors[4])
{
    uint64_t bits = 0;
    for (unsigned shift = 0; shift < 64/BITS_PER_PIXEL; shift++)
        bits |= colors[((shift + ((shift / PATTERN_WIDTH) % 4)) % 4)].value;
    pattern_t pat = { .bits = bits };
    return pat;
}

// (From ggl.h) Generic definitions for both color and gray modes

// Convert from RGB (0-255) to RGB16(5-6-5)
#define RGB_TO_RGB16(red, green, blue) ((((red) &0xf8) << 8) | (((green) &0xfc) << 3) | (((blue) &0xf8) >> 3))

// Pack RGB16 components (red=0-31, green=0-63, blue=0-31)
#define PACK_RGB16(red, green, blue)   ((((red) &0x1f) << 11) | (((green) &0x3f) << 5) | (((blue) &0x1f)))

// Extract RGB red component from RGB16 color (bit expand to 0-255 range)
#define RGBRED(rgb16)                  (((rgb16) &0x8000) ? (((rgb16) >> 8) | 7) : ((rgb16) >> 8) & 0xf8)
// Extract RGB green component from RGB16 color (bit expand to 0-255 range)
#define RGBGREEN(rgb16)                (((rgb16) &0x400) ? ((((rgb16) >> 3) & 0xff) | 3) : ((rgb16) >> 3) & 0xfc)
// Extract RGB blue component from RGB16 color (bit expand to 0-255 range)
#define RGBBLUE(rgb16)                 (((rgb16) &0x10) ? ((((rgb16) << 3) & 0xff) | 7) : ((rgb16) << 3) & 0xfc)

// Extract RGB components from a 16-grays color value
#define G2RGBRED(gray)                 ((((gray) &0xf) << 4) | (((gray) &0x8) ? 0xf : 0))
#define G2RGBGREEN(gray)               ((((gray) &0xf) << 4) | (((gray) &0x8) ? 0xf : 0))
#define G2RGBBLUE(gray)                ((((gray) &0xf) << 4) | (((gray) &0x8) ? 0xf : 0))

#ifndef TARGET_PRIME1
#  define C2RGBRED(gray)   G2RGBRED(gray)
#  define C2RGBGREEN(gray) G2RGBGREEN(gray)
#  define C2RGBBLUE(gray)  G2RGBBLUE(gray)
#else // TARGET_PRIME1
#  define C2RGBRED(rgb16)   RGBRED(rgb16)
#  define C2RGBGREEN(rgb16) RGBGREEN(rgb16)
#  define C2RGBBLUE(rgb16)  RGBBLUE(rgb16)
#endif // TARGET_PRIME1

// Convert from RGB (0-255) to GRAY16(4-bit)
#define RGB_TO_GRAY16(red, green, blue) ((((red) + (green) + (green) + (blue)) >> 6) & 0xf)


#define REPEAT_NIBBLE(nib)              (((nib) &0xf) | (((nib) &0xf) << 4))
#define REPEAT_BYTE(byte)               (((byte) &0xff) | (((byte) &0xff) << 8))
#define REPEAT_HALFWORD(hword)          (((hword) &0xffff) | (((hword) &0xffff) << 16))

#define PATTERN_SOLID(gray)             REPEAT_HALFWORD(REPEAT_BYTE(REPEAT_NIBBLE(gray)))
#define PATTERN_2COL(dot1, dot2) REPEAT_HALFWORD(REPEAT_BYTE( ((dot1)&0xf) | (((dot2)&0xf)<<4) )))
#define PATTERN_4COL(dot1, dot2, dot3, dot4) \
  REPEAT_HALFWORD(((dot1) &0xf) | (((dot2) &0xf) << 4) | (((dot3) &0xf) << 8) | (((dot4) &0xf) << 12))

// Theming engine definitions, include early to allow for target-specific overrides

// Default palette size, entries 0-15 are for grayscale conversion, entries above 16 are customizable Theme colors for
// different elements of the UI
#define PALETTE_SIZE      64
#define PALETTE_MASK      63

#define IS_PALETTE_COLOR 0x10000

// Global palette, can be used for grayscale conversion or for themes
extern int ggl_palette[PALETTE_SIZE];

// internal buffer for hblt routines

#define HBLT_BUFFER 64 // default 64 words = 512 pixels

#ifndef TARGET_PRIME1
// The CGL library is a close drop-in replacement to replace the GGL (Gray Graphics Library)
// which was fixed to 4-bits per pixel
// CGL library has ggl_xxx functions that are fully compatible with GGL and will map the 16 grays
// to a proper color. Code designed to run on GGL should work correctly with the screen in color mode.
// ggl_xxx functions are equivalent but work with full color.

// Data structures and API are meant to be compatible with GGL, so they need to be very similar for
// ease of porting.

#endif /* ! TARGET_PRIME1 */
// data types/structures

// surface used for bitblt operations
// notes:
// a surface is infinite in both width and height, there are no memory limits
// .addr must be a word aligned address
// .width is used only to find a new scanline, and can be changed at will
//        the width is given in pixels and it can be arbitrary (no alignement needed)
// for normal drawing primitives, (0,0) is the word-aligned address pointed by .addr,
// disregarding of the values in .x and .y
// for bitblt operations, .x and .y give the origin (top-left corner) of the region to use
#ifndef TARGET_PRIME1
// the surface is PIXEL-aligned, so there's multiple pixels per word, and a scanline
// may start misaligned. Use a proper .width if each scanline needs to be word aligned
#else  /* TARGET_PRIME1 */
// the surface is nibble-aligned, so a 1 pixel wide surface will contain 8
// rows of pixels per word
#endif /* TARGET_PRIME1 */

typedef struct
{
    int *addr;  //! Word-aligned address of the surface buffer
    int  width; //! Width (in pixels) of the buffer
    int  x, y;  //! Offset coordinates within the buffer
    int  clipx, clipx2, clipy, clipy2;
#ifdef TARGET_PRIME1
    int actbuffer; //! Active buffer: 0 or 1
#endif             /* TARGET_PRIME1 */
} gglsurface;

typedef unsigned int (*gglfilter)(unsigned int pixels, int param);

typedef unsigned int (*ggloperator)(unsigned int dest, unsigned int source, int param);

// inline routines

// THIS IS PLATFORM INDEPENDENT ROTATION
// IN ARM, A<<B WITH B>=32 = ZERO
// IN X86, A<<B WITH B>=32 = A<<(B&31)

#define ROT_LEFT(a, b)    (((b) >= 32) ? 0 : (((unsigned) a) << (b)))
#define ROT_RIGHT(a, b)   (((b) >= 32) ? 0 : ((a) >> (b)))

#define ggl_leftmask(cx)  ((ROT_LEFT(1, (((cx) &7) << 2)) - 1))    // create mask
#define ggl_rightmask(cx) (ROT_LEFT((-1), ((((cx) &7) + 1) << 2))) // create mask

void ggl_initscr(gglsurface *surface);

// drawing primitives
// general pixel set/read routines

void ggl_pltnib(int *buff, int off, int color); // poke a pixel (off in nibbles)
int  ggl_getnib(int *buff, int off);            // peek a pixel (off in nibbles)
int  ggl_getmonopix(char *buf, int off);        // peek a pixel in monochrome bitmap (off in pixels)

// general drawing primitives

// note: the argument color is a 32-bit value containing a different
//       color for each pixel. For solid colors, set color to contain the same value
//       on every nibble (for color 8, color=0x88888888)
//       or call ggl_mkcolor for that purpose

void ggl_hline(gglsurface *srf, int y, int xl, int xr, int color); // fast low-level horizontal line
void ggl_cliphline(gglsurface *srf, int y, int xl, int xr, int color);
void ggl_vline(gglsurface *srf, int x, int yt, int yb, int color); // fast low-level vertical line
void ggl_clipvline(gglsurface *srf, int x, int yt, int yb, int color);
void ggl_rect(gglsurface *srf, int x1, int y1, int x2, int y2, int color);     // low-level rectangle
void ggl_cliprect(gglsurface *srf, int x1, int y1, int x2, int y2, int color); // low-level rectangle

void ggl_rectp(gglsurface *srf, int x1, int y1, int x2, int y2, int *color); // low-level rectangle with 8x8 pattern

// bit-blit functions

// LOW-LEVEL row copy functions
// ggl_ll_hblt is a general nibble-aligned memcpyb
// WARNING: npixels is limited to 512!!
//          if you need more than that, increase the constant HBLT_BUFFER above
//          and RE-COMPILE the ggl library
// dest and src must be word-aligned
// destoff and srcoff are offsets in nibbles from the word-aligned pointers
// npixels is the number of nibbles to copy
// note: hblt will behave well even if the zones overlap, no need for moveup/movedown

void ggl_hblt(int *dest, int destoff, int *src, int srcoff, int npixels); // copy a row of pixels

// same behavior as hblt but specifying a transparent color
// every pixel in *src with the transparent color will not affect the
// corresponding pixel in *dest
void ggl_hbltmask(int *dest, int destoff, int *src, int srcoff, int npixels, int tcol); // copy a row of pixels w/mask

// rectangle blt
// note: see gglsurface above for complete understanding of the behavior of these routines
// ggl_bitblt loops from top to bottom
void ggl_bitblt(gglsurface *dest, gglsurface *src, int width, int height); // copy a rectangular region
// ggl_revblt loops from bottom to top, for overlapping zones
void ggl_revblt(gglsurface *dest, gglsurface *src, int width, int height); // copy a rectangular region, reverse loop
// ggl_ovlblt chooses to use normal/reverse loop based on the addresses
// use it when the direcction of movement is unknown
void ggl_ovlblt(gglsurface *dest, gglsurface *src, int width, int height); // copy overlapped regions
// ggl_bitbltmask behaves exactly as ggl_bitblt but using tcol as a transparent color
#define ggl_bitbltmask(dest, src, width, height, tcol) \
  ggl_bitbltoper(dest, src, width, height, tcol, (ggloperator) &ggl_opmask)
#define ggl_monobitbltmask(dest, src, width, height, tcol) \
  ggl_monobitbltoper(dest, src, width, height, tcol, (ggloperator) &ggl_opmask)

void     ggl_bitbltclip(gglsurface *dest,
                        gglsurface *src,
                        int         width,
                        int         height); // copy a rectangular region, clipped within dest

// rectangle scrolling routines
// dest contains the surface to scroll, and width and height define the rectangle
// the area that needs to be redrawn after the scroll is not erased or modified by these routines
void     ggl_scrollup(gglsurface *dest, int width, int height, int npixels); // scroll npixels up
void     ggl_scrolldn(gglsurface *dest, int width, int height, int npixels); // scroll npixels dn
void     ggl_scrolllf(gglsurface *dest, int width, int height, int npixels); // scroll npixels left
void     ggl_scrollrt(gglsurface *dest, int width, int height, int npixels); // scroll npixels right

// custom filters and operators

// low-level row filtering routine
void     ggl_hbltfilter(int *dest, int destoff, int npixels, int param, gglfilter filterfunc);
// bitmap filtering routine
void     ggl_filter(gglsurface *dest, int width, int height, int param, gglfilter filterfunc);

// low-level row operator routine
void     ggl_hbltoper(int *dest, int destoff, int *src, int srcoff, int npixels, int param, ggloperator foperator);
// low-level row operator routine for monochrome bitmaps
void     ggl_monohbltoper(int           *dest,
                          int            destoff,
                          unsigned char *src,
                          int            srcoff,
                          int            npixels,
                          int            param,
                          ggloperator    foperator);
// bitblt operator routine
void     ggl_bitbltoper(gglsurface *dest, gglsurface *src, int width, int height, int param, ggloperator fop);
// bitblt operator routine for monochrome bitmaps
void     ggl_monobitbltoper(gglsurface *dest, gglsurface *src, int width, int height, int param, ggloperator fop);

// predefined filters and operators

// filters (unary operators)
// ligthens an image by subtracting param from all pixels
unsigned ggl_fltlighten(unsigned word, int param);
// darkens an image by adding param to all pixels
unsigned ggl_fltdarken(unsigned word, int param);
// invert the colors on all pixels
unsigned ggl_fltinvert(unsigned word, int param);
// replace a color with another
unsigned ggl_fltreplace(unsigned word, int param);

// operators (between two surfaces)
// standard mask, tcolor in src is considered transparent
unsigned ggl_opmask(unsigned dest, unsigned src, unsigned tcolor);
// transparency blend, weight is 0 = src is opaque, 16 = src is fully transparent
unsigned ggl_optransp(unsigned dest, unsigned src, int weight);
// standard mask, tcolor in src is considered transparent, white color in src is replaced with newcolor
unsigned ggl_opmaskcol(unsigned dest, unsigned src, unsigned tcolor, unsigned newcolor);

// ggl_mkcolor repeats the same color on every nibble
// ggl_mkcolor(2) will return 0x22222222
int      ggl_mksolid(int color); // solid color generator

// ggl_getcolor takes a system palette index color and expands to an actual RGB16 color
// for values 0-15 the system palette must match grayscale levels for compatibility
#define ggl_mkcolor(color) (ggl_palette[(color) &PALETTE_MASK])

// Set a palette index entry
#define ggl_setpalette(index, color)             \
  {                                              \
    ggl_palette[(index) &PALETTE_MASK] = (color); \
  }

// Return the actual color to draw, either the given color or a palette color
#define ggl_getcolor(color) (((color) &IS_PALETTE_COLOR) ? ggl_palette[(color) &PALETTE_MASK] : (color))

// ggl_mkcolor32 creates virtual 32-colors by using 8x8 patterns
// col32 is a value from 0 to 30, being 30=black, 0=white
// note: the user is responsible to provide a valid int[8] buffer in the
// pattern argument
void ggl_mkcolor32(int col32, int *pattern); // 50% dither pattern generator for 31 colors

#endif
