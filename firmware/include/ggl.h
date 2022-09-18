/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */
#ifndef _GGL_H
#define _GGL_H

#include "common-macros.h"
#include "newrpl_types.h"

#include <stdint.h>
#include <target.h>

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
#  error Cannot build GGL without BITS_PER_PIXEL
#endif

#if BITS_PER_PIXEL == 1
#  define PATTERN_WIDTH  8
#  define PATTERN_HEIGHT 8
#elif BITS_PER_PIXEL == 4
#  define PATTERN_WIDTH  4
#  define PATTERN_HEIGHT 4
#elif BITS_PER_PIXEL == 16
#  define PATTERN_WIDTH  2
#  define PATTERN_HEIGHT 2
#else
#  error Unknown value for BITS_PER_PIXEL
#endif

#define BITS_PER_PATTERN_ROW   (BITS_PER_PIXEL * PATTERN_WIDTH)
#define BYTES_PER_PATTERN_ROW  (BITS_PER_PATTERN_ROW / 8)
#define COLORS_PER_PATTERN_ROW (BYTES_PER_PATTERN_ROW / sizeof(color_t))
#define BITS_PER_PATTERN       (8 * sizeof(pattern_t))
#define BITS_PER_WORD          (8 * sizeof(pixword))


typedef int      coord;
typedef unsigned size;
typedef int      offset;
typedef unsigned pixword;
typedef unsigned palette_index;

// Generic definitions for both color and gray modes
typedef union color1
{
    struct bits
    {
        uint8_t bit0 : 1;
        uint8_t bit1 : 1;
        uint8_t bit2 : 1;
        uint8_t bit3 : 1;
        uint8_t bit4 : 1;
        uint8_t bit5 : 1;
        uint8_t bit6 : 1;
        uint8_t bit7 : 1;
    } __attribute__((packed)) bits;
    uint8_t value;
} color1_t;

typedef union color4_t
{
    struct nibbles
    {
        uint8_t low  : 4;
        uint8_t high : 4;
    } __attribute__((packed)) nibbles;
    uint8_t value;
} color4_t;

typedef union color16
{
    struct rgb16
    {
        uint8_t blue  : 5;
        uint8_t green : 6;
        uint8_t red   : 5;
    } __attribute__((packed)) rgb16;
    uint16_t value;
} color16_t;

#define color_t CAT(color, CAT(BITS_PER_PIXEL, _t))

typedef union pattern
{
    uint64_t  bits;
    uint64_t  color : BITS_PER_PIXEL;
    color1_t  plane1[8];
    color4_t  plane4[8];
    color16_t plane16[4];
} pattern_t;


static inline uint64_t ggl_rotate_pattern_bits(uint64_t bits, unsigned shift)
{
    return (bits << (shift % BITS_PER_PATTERN)) |
           (bits >> (BITS_PER_PATTERN - (shift % BITS_PER_PATTERN)));
}

static inline pattern_t ggl_rotate_pattern(pattern_t pat, unsigned pixels)
{
    pat.bits = ggl_rotate_pattern_bits(pat.bits, pixels * BITS_PER_PIXEL);
    return pat;
}

static inline pattern_t ggl_solid_pattern(color_t color)
{
    uint64_t bits = 0;
    for (unsigned shift = 0; shift < 64; shift += BITS_PER_PIXEL)
        bits |= color.value << shift;
    pattern_t pat = { .bits = bits };
    return pat;
}

static inline pattern_t ggl_pattern_2_colors_array(color_t colors[2])
{
    uint64_t bits = 0;
    for (unsigned shift = 0; shift < 64 / BITS_PER_PIXEL; shift++)
        bits |= colors[((shift + ((shift / PATTERN_WIDTH) % 2)) % 2)].value;
    pattern_t pat = { .bits = bits };
    return pat;
}

static inline pattern_t ggl_pattern_2_colors(color_t color1, color_t color2)
{
    color_t colors[] = { color1, color2 };
    return ggl_pattern_2_colors_array(colors);
}

static inline pattern_t ggl_pattern_4_colors_array(color_t colors[4])
{
    uint64_t bits = 0;
    for (unsigned shift = 0; shift < 64 / BITS_PER_PIXEL; shift++)
        bits |= colors[((shift + ((shift / PATTERN_WIDTH) % 4)) % 4)].value;
    pattern_t pat = { .bits = bits };
    return pat;
}

static inline pattern_t ggl_pattern_4_colors(color_t c1, color_t c2, color_t c3, color_t c4)
{
    color_t colors[] = { c1, c2, c3, c4 };
    return ggl_pattern_4_colors_array(colors);
}

// (From ggl.h) Generic definitions for both color and gray modes

// Convert from RGB (0-255) to RGB16(5-6-5)
static inline color16_t ggl_rgb16(uint8_t red, uint8_t green, uint8_t blue)
{
    color16_t result = {
        .rgb16 = {.red = red, .green = green, .blue = blue}
    };
    return result;
}

// Convert from RGB (0-255) to RGB16(5-6-5)
static inline color16_t ggl_rgb32_to_rgb16(uint8_t red, uint8_t green, uint8_t blue)
{
    return ggl_rgb16(red >> 3, green >> 2, blue >> 3);
}


static inline color_t ggl_rgb16_to_color(color16_t c16)
{
#if BITS_PER_PIXEL == 1
    uint8_t value = c16.value ? 1 : 0;
#elif BITS_PER_PIXEL == 4
    uint8_t value = (c16.rgb16.red + c16.rgb16.green + c16.rgb16.blue) / 8;
#elif BITS_PER_PIXEL == 16
    uint16_t value = c16.value;
#else
#  error Invalid BITS_PER_PIXEL
#endif //
    color_t color = { .value = value };
    return color;
}

static inline color16_t ggl_color_to_rgb16(color_t color)
{
#if BITS_PER_PIXEL == 1
    uint8_t value = color ? 0xFF : 0x00;
    return ggl_rgb32_to_rgb16(value, value, value);
#elif BITS_PER_PIXEL == 4
    uint8_t value = color.value | (color.value << 4);
    return ggl_rgb32_to_rgb16(value, value, value);
#elif BITS_PER_PIXEL == 16
    return color;
#else
#  error Invalid BITS_PER_PIXEL
#endif //
}

static inline uint8_t ggl_red(color16_t color)
{
    return (color.rgb16.red << 3) | (color.rgb16.red >> 2);
}

static inline uint8_t ggl_green(color16_t color)
{
    return (color.rgb16.green << 2) | (color.rgb16.green >> 3);
}

static inline uint8_t ggl_blue(color16_t color)
{
    return (color.rgb16.blue << 3) | (color.rgb16.blue >> 2);
}

#define RGB_TO_RGB16(red, green, blue) (ggl_rgb32_to_rgb16((red), (green), (blue)))

// Pack RGB16 components (red=0-31, green=0-63, blue=0-31)
#define PACK_RGB16(red, green, blue)   (ggl_rgb16((red), (green), (blue)))

// Extract RGB red component from RGB16 color (bit expand to 0-255 range)
#define RGBRED(color)                  ggl_red(ggl_color_to_rgb16(color))
// Extract RGB green component from RGB16 color (bit expand to 0-255 range)
#define RGBGREEN(color)                ggl_green(ggl_color_to_rgb16(color))
// Extract RGB blue component from RGB16 color (bit expand to 0-255 range)
#define RGBBLUE(color)                 ggl_blue(ggl_color_to_rgb16(color))

// Convert from RGB (0-255) to GRAY16(4-bit)
#define RGB16_TO_GRAY(red, green, blue) ((((red) + (green) + (green) + (blue)) >> 6) & 0xf)

// Theming engine definitions, include early to allow for target-specific overrides

// Default palette size, entries 0-15 are for grayscale conversion, entries above 16 are customizable Theme colors for
// different elements of the UI
#define PALETTE_SIZE 64
#define PALETTE_MASK 63

// Global palette, can be used for grayscale conversion or for themes
extern color_t ggl_palette[PALETTE_SIZE];

static inline color_t ggl_color(palette_index index)
{
    return ggl_palette[index & PALETTE_MASK];
}

static inline void ggl_set_color(palette_index index, color_t color)
{
    ggl_palette[index & PALETTE_MASK] = color;
}

static inline void ggl_set_color16(palette_index index, color16_t color)
{
    ggl_palette[index & PALETTE_MASK] = ggl_rgb16_to_color(color);
}


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
    pixword *pixels;        //! Word-aligned address of the surface buffer
    size     width;         //! Width (in pixels) of the buffer
    size     height;        //! Height (in pixels) of the buffer
    coord    x, y;          //! Offset coordinates within the buffer
    coord    left, right;   //! Horizontal clip area
    coord    top, bottom;   //! Vertical clip area
    int      active_buffer; //! Active buffer: 0 or 1
} gglsurface;

typedef pixword (*gglop)(pixword dst, pixword src, pixword arg);

typedef pixword (*gglfilter_fn)(pixword color, pixword param);

typedef pixword (*ggloperator_fn)(pixword dest, pixword source, pixword param);

static inline offset ggl_pixel_offset(gglsurface *s, coord x, coord y)
{
    return (s->width * y + x) * BITS_PER_PIXEL / BITS_PER_WORD;
}

static inline pixword *ggl_pixel_address(gglsurface *s, coord x, coord y)
{
    return s->pixels + ggl_pixel_offset(s, x, y);
}

// Operations for ggl_blit
static inline pixword ggl_set(pixword dst, pixword src, pixword arg)
{
    UNUSED(dst);
    UNUSED(src);
    return arg;
}

#if 0 // Seems to be unused at the moment
// transparency blend:
// weight is 0 for opaque, 1 bits for transparent
static inline pixword ggl_transparency(pixword dst, pixword src, pixword arg)
{
    // Weighted proportionality, where 'arg' is the weight.
    // Brighter colors in 'arg' make the source more transparent.
    pixword result = 0;
    pixword max = (1U << BITS_PER_PIXEL) - 1;
    pixword mask = max;
    for (unsigned shift = 0; shift < BITS_PER_WORD; shift += BITS_PER_PIXEL)
    {
        pixword srcpix = (src >> shift) & mask;
        pixword dstpix = (dst >> shift) & mask;
        // In all cases, a N x N multiplication uses 2N bits, so when
        // we shift by BITS_PER_PIXEL, we get back the original resolution.
        pixword pix = (srcpix * (max - arg) + dstpix * arg) >> BITS_PER_PIXEL;
        result |= (pix & mask) << shift;
    }
    return result;
}
#endif

// Color mask:
// Parts that contain zero in the source are transparent, replaced with arg
static inline pixword ggl_opmaskcol(pixword dst, pixword src, pixword arg)
{
    pixword result = 0;
    pixword mask = (1U << BITS_PER_PIXEL) - 1;
    for (unsigned shift = 0; shift < BITS_PER_WORD; shift += BITS_PER_PIXEL)
    {
        pixword srcpix = (src >> shift) & mask;
        pixword dstpix = (dst >> shift) & mask;
        pixword argpix = (arg >> shift) & mask;
        if (srcpix)
            result |= srcpix << shift;
        else if (dstpix)
            result |= dstpix << shift;
        else
            result |= argpix << shift;
    }
    return result;
}

typedef enum clip
{
    CLIP_NONE = 0,
    CLIP_SRC  = 1,
    CLIP_DST  = 2,
    CLIP_ALL  = 3
} clip_t;

// General-purpose blitting operation
// This transfers pixels from 'src' to 'dst' (which can be equal)
// targeting a rectangle defined by (left, right, top, bottom)
// fetching pixels from (x,y) in the source
// applying the given operation in 'op'
// Everything is inline so that the compiler can optimize away unnecessary code
static inline void ggl_blit(gglsurface *dst,
                            gglsurface *src,
                            coord       left,
                            coord       right,
                            coord       top,
                            coord       bottom,
                            coord       x,
                            coord       y,
                            gglop       op,
                            pattern_t   colors,
                            clip_t      clip)
{
    if (clip & CLIP_SRC)
    {
        if (x < src->left)
        {
            left += src->left - x;
            x = src->left;
        }
        if (x + right - left > src->right)
            right = src->right - x + left;
        if (y < src->top)
        {
            top += src->top - y;
            y = src->top;
        }
        if (y + bottom - top > src->bottom)
            bottom = src->bottom - y + top;
    }

    if (clip & CLIP_DST)
    {
        // Clipping based on target
        if (left < dst->left)
            left = dst->left;
        if (right > dst->right)
            right = dst->right;
        if (top < dst->top)
            top = dst->top;
        if (bottom > dst->bottom)
            bottom = dst->bottom;

        // Bail out if there is no effect
        if (left > dst->right || right < dst->left || top > dst->bottom ||
            bottom < dst->top)
            return;
    }

    // Check whether we need to go forward or backward along X or Y
    int      xback  = x < left;
    int      yback  = y < top;
    int      xdir   = xback ? -1 : 1;
    int      ydir   = yback ? -1 : 1;
    coord    dx1    = xback ? right : left;
    coord    dx2    = xback ? left : right;
    coord    dy1    = yback ? bottom : top;
    coord    sx1    = xback ? x + right - left : x;
    coord    sy1    = yback ? y + bottom - top : y;
    coord    ycount = bottom - top;

    // Offset in words for a displacement along Y
    offset   dyoff  = ggl_pixel_offset(dst, 0, ydir);
    offset   syoff  = ggl_pixel_offset(src, 0, ydir);

    // Pointers to word containing start and end pixel
    pixword *dp1    = ggl_pixel_address(dst, dx1, dy1);
    pixword *dp2    = ggl_pixel_address(dst, dx2, dy1);
    pixword *sp1    = ggl_pixel_address(src, sx1, sy1);

    // Left and right pixel shift
    unsigned dsh1   = dx1 * BITS_PER_PIXEL % BITS_PER_WORD;
    unsigned dsh2   = dx2 * BITS_PER_PIXEL % BITS_PER_WORD;
    unsigned ssh1   = (sx1 - dx1) * BITS_PER_PIXEL % BITS_PER_WORD;
    unsigned srt1   = BITS_PER_WORD - ssh1;

    // Left and right masks
    pixword  ones   = ~0U;
    pixword  dmsk1  = xback ? ones >> (BITS_PER_WORD - dsh1) : ones << dsh1;
    pixword  dmsk2  = xback ? ones << dsh2 : ones >> (BITS_PER_WORD - dsh2);

    // Adjust the color pattern based on starting point
    uint64_t data   = colors.bits;
    data = ggl_rotate_pattern_bits(data, (dx1 + dy1) * BITS_PER_PIXEL);

    // Check if we are writing a single word and need to combine masks
    if (dp1 == dp2)
    {
        pixword dmsk = dmsk1 & dmsk2;
        while (ycount-- >= 0)
        {
            pixword dword  = *dp1;
            pixword sword  = (sp1[0] << ssh1) | (sp1[xdir] >> srt1);
            pixword aword  = data;
            pixword result = op(dword, sword, aword);
            *dp1           = (result & dmsk) | (dword & ~dmsk);
            dp1 += dyoff;
            sp1 += syoff;
            data = ggl_rotate_pattern_bits(data, ydir * BITS_PER_PIXEL);
        }
    }
    else
    {
        // Need to mask beginning and end differently
        while (ycount-- >= 0)
        {
            pixword *dp     = dp1;
            pixword *sp     = sp1;
            pixword  dword  = *dp;
            pixword  sword  = (sp[0] << ssh1) | (sp[xdir] >> srt1);
            pixword  aword  = data;
            pixword  result = op(dword, sword, aword);
            *dp             = (result & dmsk1) | (dword & ~dmsk1);
            dp += xdir;
            sp += xdir;
            data = ggl_rotate_pattern_bits(data, BITS_PER_WORD);
            while (dp != dp2)
            {
                dword  = *dp;
                sword  = (sp[0] << ssh1) | (sp[xdir] >> srt1);
                aword  = data;
                result = op(dword, sword, aword);
                *dp    = result;
                dp += xdir;
                sp += xdir;
                data = ggl_rotate_pattern_bits(data, BITS_PER_WORD);
            }
            dword  = *dp;
            sword  = *sp;
            aword  = data;
            result = op(dword, sword, aword);
            *dp    = (result & dmsk2) | (dword & ~dmsk2);
            data   = ggl_rotate_pattern_bits(data, BITS_PER_WORD);

            dp1 += dyoff;
            dp2 += dyoff;
            sp1 += syoff;
            data = ggl_rotate_pattern_bits(data, ydir * BITS_PER_PIXEL);
        }
    }
}


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

// Read a pixel from a monochrome bitmap
static inline int ggl_getmonopix(byte_p buf, offset addr)
{
    byte_p ptr = buf + (addr >> 3);
    return (*ptr & (1 << (addr & 7))) ? 1 : 0;
}


int  ggl_getmonopix(byte_p buf, offset off);                      // peek a pixel in monochrome bitmap (off in pixels)

// general drawing primitives

// The low-level primitives take a pattern to describe colors because they work
// on words rather than individual pixels.
// The pattern_t argument is a 64-bit value containing a different color for
// each pixel, arranged in a grid PATTERN_WIDTH wide and PATTERN_HEIGHT high.
// The pattern repeats both horizontally and vertically, which makes it possible
// to implement efficient operations on all supported BITS_PER_PIXEL configurations.
// The ggl_solid will create a solid pattern from a palette index
// The ggl_solid_pattern will do the same from a color
// Note that the wrapping of data in a struct is zero-cost on any reasonably
// decent compiler.

void ggl_hline(gglsurface *srf, int y, int xl, int xr, pattern_t color); // fast low-level horizontal line
void ggl_cliphline(gglsurface *srf, int y, int xl, int xr, pattern_t color);
void ggl_vline(gglsurface *srf, int x, int yt, int yb, pattern_t color); // fast low-level vertical line
void ggl_clipvline(gglsurface *srf, int x, int yt, int yb, pattern_t color);
void ggl_rect(gglsurface *srf, int x1, int y1, int x2, int y2, pattern_t color);     // low-level rectangle
void ggl_cliprect(gglsurface *srf, int x1, int y1, int x2, int y2, pattern_t color); // low-level rectangle

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

void ggl_hblt(pixword *dest, int destoff, pixword *src, int srcoff, size npixels); // copy a row of pixels

// same behavior as hblt but specifying a transparent color
// every pixel in *src with the transparent color will not affect the
// corresponding pixel in *dest
void ggl_hbltmask(pixword *dest, int destoff, pixword *src, int srcoff, size npixels, int tcol); // copy a row of pixels w/mask

// rectangle blt
// note: see gglsurface above for complete understanding of the behavior of these routines
// ggl_bitblt loops from top to bottom
void ggl_bitblt(gglsurface *dest, gglsurface *src, size width, size height); // copy a rectangular region
// ggl_revblt loops from bottom to top, for overlapping zones
void ggl_revblt(gglsurface *dest, gglsurface *src, size width, size height); // copy a rectangular region, reverse loop
// ggl_ovlblt chooses to use normal/reverse loop based on the addresses
// use it when the direcction of movement is unknown
void ggl_ovlblt(gglsurface *dest, gglsurface *src, size width, size height); // copy overlapped regions
// ggl_bitbltmask behaves exactly as ggl_bitblt but using tcol as a transparent color

void     ggl_bitbltclip(gglsurface *dest,
                        gglsurface *src,
                        size        width,
                        size        height); // copy a rectangular region, clipped within dest

// rectangle scrolling routines
// dest contains the surface to scroll, and width and height define the rectangle
// the area that needs to be redrawn after the scroll is not erased or modified by these routines
void     ggl_scrollup(gglsurface *dest, size width, size height, size npixels); // scroll npixels up
void     ggl_scrolldn(gglsurface *dest, size width, size height, size npixels); // scroll npixels dn
void     ggl_scrolllf(gglsurface *dest, size width, size height, size npixels); // scroll npixels left
void     ggl_scrollrt(gglsurface *dest, size width, size height, size npixels); // scroll npixels right

// custom filters and operators

// low-level row filtering routine
void     ggl_hbltfilter(pixword *dest, int destoff, size npixels, int param, gglfilter_fn filterfunc);
// bitmap filtering routine
void     ggl_filter(gglsurface *dest, size width, size height, int param, gglfilter_fn filterfunc);

// low-level row operator routine
void     ggl_hbltoper(pixword *dest, int destoff, pixword *src, int srcoff, size npixels, int param, ggloperator_fn foperator);
// low-level row operator routine for monochrome bitmaps
void      ggl_monohbltoper(pixword       *dest,
                           int            destoff,
                           unsigned char *src,
                           int            srcoff,
                           int            npixels,
                           int            param,
                           ggloperator_fn foperator);
// bitblt operator routine
void     ggl_bitbltoper(gglsurface *dest, gglsurface *src, size width, size height, int param, ggloperator_fn fop);
// bitblt operator routine for monochrome bitmaps
void     ggl_monobitbltoper(gglsurface *dest, gglsurface *src, size width, size height, int param, ggloperator_fn fop);

// predefined filters and operators

// filters (unary operators)
// ligthens an image by subtracting param from all pixels
pixword ggl_fltlighten(pixword color, pixword param);
// darkens an image by adding param to all pixels
pixword ggl_fltdarken(pixword color, pixword param);
// invert the colors on all pixels
pixword ggl_fltinvert(pixword color, pixword param);
// replace a color with another
pixword ggl_fltreplace(pixword color, pixword param);



// ggl_color repeats the same color on every nibble
// ggl_color(2) will return 0x22222222
static inline pattern_t ggl_solid(palette_index index) { return ggl_solid_pattern(ggl_color(index)); }

// ggl_color32 creates virtual 32-colors by using 8x8 patterns
// col32 is a value from 0 to 30, being 30=black, 0=white
// note: the user is responsible to provide a valid int[8] buffer in the
// pattern argument
void ggl_color32(int col32, int *pattern); // 50% dither pattern generator for 31 colors

#endif
