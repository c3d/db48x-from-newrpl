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

// ****************************************************************************
//
// GGL provides a unified interface for three forms of graphics with different
// bits per pixel (BPP):
// 1. 4BPP  gray-scale graphics on gray-scale platforms (HP50g).
// 2. 16BPP RGB16 color graphics on color platforms (Prime)
// 3. 1BPP  Black and white graphics on bitmap platforms (DM42)
//
// In order to accomodate common theming across the platforms, graphics
// functions accept two forms of color input: flat color, which uses the same
// color for every pixel, and color pattern, which is a 64-bit value that
// creates a pattern of different sizes depending on the value of BITS_PER_PIXEL
//
// In 1BPP mode, the 64-bits correspond to an 8x8 bitmap pattern
// In 4BPP mode, the 64-bits correspond to a 4x4 grayscale pattern
// In 16BPP mode, the 64 bits correspond to a 2x2 color pattern
//
// In combined operations, the pattern is always aligned with the destination
//
// ****************************************************************************


// ============================================================================
//
//     Configuration constants
//
// ============================================================================

#ifndef BITS_PER_PIXEL
#    error Cannot build GGL without BITS_PER_PIXEL
#endif

#if BITS_PER_PIXEL == 1
#    define PATTERN_WIDTH  8
#    define PATTERN_HEIGHT 8
#elif BITS_PER_PIXEL == 4
#    define PATTERN_WIDTH  4
#    define PATTERN_HEIGHT 4
#elif BITS_PER_PIXEL == 16
#    define PATTERN_WIDTH  2
#    define PATTERN_HEIGHT 2
#else
#    error Unknown value for BITS_PER_PIXEL
#endif

#define BITS_PER_PATTERN_ROW   (BITS_PER_PIXEL * PATTERN_WIDTH)
#define BYTES_PER_PATTERN_ROW  (BITS_PER_PATTERN_ROW / 8)
#define COLORS_PER_PATTERN_ROW (BYTES_PER_PATTERN_ROW / sizeof(color_t))
#define BITS_PER_PATTERN       (8 * sizeof(pattern_t))
#define BITS_PER_WORD          (8 * sizeof(pixword))


// ============================================================================
//
//   Data types
//
// ============================================================================

typedef int      coord;
typedef unsigned size;
typedef int      offset;
typedef unsigned pixword;
typedef unsigned palette_index;

typedef union color1
// ----------------------------------------------------------------------------
//  1-bit color representation (e.g. DM42)
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
//   4-bit color representation (e.g. grayscales on HP50 and the like)
// ----------------------------------------------------------------------------
{
    struct nibbles
    {
        uint8_t low  : 4;
        uint8_t high : 4;
    } __attribute__((packed)) nibbles;
    uint8_t value;
} color4_t;


typedef union color16
// ----------------------------------------------------------------------------
//   16-bit color representation, e.g. Prime1
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
//   The color type for the current platform
// ----------------------------------------------------------------------------


typedef union pattern
// ----------------------------------------------------------------------------
//   Pattern representation for static inline fills
// ----------------------------------------------------------------------------
{
    uint64_t  bits;
    uint64_t  color : BITS_PER_PIXEL;
    color1_t  plane1[8];
    color4_t  plane4[8];
    color16_t plane16[4];
} pattern_t;


typedef struct gglsurface
// ----------------------------------------------------------------------------
//   Structure representing a surface on screen
// ----------------------------------------------------------------------------
{
    pixword *pixels;        //! Word-aligned address of the surface buffer
    size     width;         //! Width (in pixels) of the buffer
    size     height;        //! Height (in pixels) of the buffer
    size     bpp;           //! Bits per pixel
    coord    left, right;   //! Horizontal clip area
    coord    top, bottom;   //! Vertical clip area
} gglsurface;


// ============================================================================
//
//   Helper routines (not supposed to be called directly)
//
// ============================================================================

static inline uint64_t ggl_rotate_pattern(uint64_t bits, unsigned shift)
// ----------------------------------------------------------------------------
//   Rotate a 64-bit pattern by an arbitrary amount
// ----------------------------------------------------------------------------
{
    shift %= BITS_PER_PATTERN;
    return ((bits >> shift) | (bits << (BITS_PER_PATTERN - shift)));
}


static inline uint64_t ggl_bpp_pattern_multiplier(size bpp)
// ----------------------------------------------------------------------------
//   A multiplier to quickly build a pattern from a color
// ----------------------------------------------------------------------------
{
    if (bpp == 1)
        return 0xFFFFFFFFFFFFFFFFull;
    if (bpp == 4)
        return 0x1111111111111111ull;
    if (bpp == 16)
        return 0x0001000100010001ull;
    uint64_t result = 0;
    for (unsigned shift = 0; shift < 64; shift += bpp)
        result |= 1ull << shift;
    return result;
}


static inline offset ggl_pixel_offset(gglsurface *s,
                                      size        sbpp,
                                      coord       x,
                                      coord       y)
// ----------------------------------------------------------------------------
//   Offset in words in a given surface for the given coordinates
// ----------------------------------------------------------------------------
{
    return ((offset) s->width * y + x) * (offset) sbpp / BITS_PER_WORD;
}


static inline offset ggl_pixel_shift(gglsurface *s, size sbpp, coord x, coord y)
// ----------------------------------------------------------------------------
//   Shift in bits in the word for the given coordinates
// ----------------------------------------------------------------------------
{
    return ((offset) s->width * y + x) * (offset) sbpp % BITS_PER_WORD;
}


static inline pixword *ggl_pixel_address(gglsurface *s,
                                         size        sbpp,
                                         coord       x,
                                         coord       y)
// ----------------------------------------------------------------------------
//   Get the address of a word representing the pixel in a surface
// ----------------------------------------------------------------------------
{
    return s->pixels + ggl_pixel_offset(s, sbpp, x, y);
}


static inline pixword shl(pixword value, unsigned shift)
// ----------------------------------------------------------------------------
//   Shift left that guarantees a zero for a large shift (even on x86)
// ----------------------------------------------------------------------------
{
    return shift < BITS_PER_WORD ? value << shift : 0;
}


static inline pixword shr(pixword value, unsigned shift)
// ----------------------------------------------------------------------------
//   A shift right that guarantees a zero for a large shift (even on x86)
// ----------------------------------------------------------------------------
{
    return shift < BITS_PER_WORD ? value >> shift : 0;
}


static inline pixword shlc(pixword value, unsigned shift)
// ----------------------------------------------------------------------------
//   Shift left by the complement of the given shift
// ----------------------------------------------------------------------------
{
    return shl(value, BITS_PER_WORD - shift);
}


static inline pixword shrc(pixword value, unsigned shift)
// ----------------------------------------------------------------------------
//   Shift right by the complement of the given shift
// ----------------------------------------------------------------------------
{
    return shr(value, BITS_PER_WORD - shift);
}


static inline pixword rol(pixword value, unsigned shift)
// ----------------------------------------------------------------------------
//   Rotate left
// ----------------------------------------------------------------------------
{
    return shl(value, shift) | shrc(value, shift);
}

static inline pixword ror(pixword value, unsigned shift)
// ----------------------------------------------------------------------------
//   Rotate right
// ----------------------------------------------------------------------------
{
    return shr(value, shift) | shlc(value, shift);
}


static inline pixword min(pixword a, pixword b)
// ----------------------------------------------------------------------------
//   Min value for pixwords
// ----------------------------------------------------------------------------
{
    return a < b ? a : b;
}


static inline pixword max(pixword a, pixword b)
// ----------------------------------------------------------------------------
//   Max value for pixwords
// ----------------------------------------------------------------------------
{
    return a > b ? a : b;
}


// ============================================================================
//
//   Pattern-building helpers
//
// ============================================================================

static inline pattern_t ggl_solid_pattern(color_t color)
// ----------------------------------------------------------------------------
//    Build a solid pattern from a single color
// ----------------------------------------------------------------------------
{
    uint64_t  multiplier = ggl_bpp_pattern_multiplier(BITS_PER_PIXEL);
    pattern_t pat        = { .bits = color.value * multiplier };
    return pat;
}

static inline pattern_t ggl_pattern_2_colors_array(color_t colors[2])
// ----------------------------------------------------------------------------
//   Build an alternating pattern from two colors given in an array
// ----------------------------------------------------------------------------
{
    uint64_t bits = 0;
    for (unsigned shift = 0; shift < 64 / BITS_PER_PIXEL; shift++)
    {
        unsigned index = (shift + ((shift / PATTERN_WIDTH) % 2)) % 2;
        bits |= (uint64_t) colors[index].value << (shift * BITS_PER_PIXEL);
    }
    pattern_t pat = { .bits = bits };
    return pat;
}


static inline pattern_t ggl_pattern_2_colors(color_t color1, color_t color2)
// ----------------------------------------------------------------------------
//   Build an alternating pattern from two colors given directly
// ----------------------------------------------------------------------------
{
    color_t colors[] = { color1, color2 };
    return ggl_pattern_2_colors_array(colors);
}


static inline pattern_t ggl_pattern_4_colors_array(color_t colors[4])
// ----------------------------------------------------------------------------
//   Build a 4-color pattern from an array
// ----------------------------------------------------------------------------
{
    uint64_t bits = 0;
    for (unsigned shift = 0; shift < 64 / BITS_PER_PIXEL; shift++)
    {
        unsigned index = (shift + ((shift / PATTERN_WIDTH) % 4)) % 4;
        bits |= (uint64_t) colors[index].value << (shift * BITS_PER_PIXEL);
    }
    pattern_t pat = { .bits = bits };
    return pat;
}


static inline pattern_t ggl_pattern_4_colors(color_t a,
                                             color_t b,
                                             color_t c,
                                             color_t d)
// ----------------------------------------------------------------------------
//  Build a 4-color pattern from 4 distinct colors
// ----------------------------------------------------------------------------
{
    color_t colors[] = { a, b, c, d };
    return ggl_pattern_4_colors_array(colors);
}


static inline color16_t ggl_rgb16(uint8_t red, uint8_t green, uint8_t blue)
// ----------------------------------------------------------------------------
//   Convert from RGB (0-255) to RGB16(5-6-5)
// ----------------------------------------------------------------------------
{
    color16_t result = { .rgb16 = {.red = red, .green = green, .blue = blue} };
    return result;
}


static inline color16_t ggl_rgb32_to_rgb16(uint8_t red,
                                           uint8_t green,
                                           uint8_t blue)
// ----------------------------------------------------------------------------
//   Convert from RGB (0-255) to RGB16(5-6-5)
// ----------------------------------------------------------------------------
{
    return ggl_rgb16(red >> 3, green >> 2, blue >> 3);
}


static inline pattern_t ggl_rgb_to_pattern(uint8_t red,
                                           uint8_t green,
                                           uint8_t blue)
// ----------------------------------------------------------------------------
//   Convert from RGB to current color mode
// ----------------------------------------------------------------------------
{
#if BITS_PER_PIXEL == 1
    uint16_t gray = (red + green + green + blue) / 4;
    color1_t a = { .value = gray > 192 };
    color1_t b = { .value = gray > 64  };
    color1_t c = { .value = gray > 128 };
    color1_t d = { .value = gray > 32  };
    return ggl_pattern_4_colors(a, b, c, d);
#elif BITS_PER_PIXEL == 4
    uint16_t gray = (red + green + green + blue) / 4;
    // On the HP48, 0xF is black, not white
    color_t color = { .value = (uint8_t) (0xF - gray / 16) };
    return ggl_solid_pattern(color);
#elif BITS_PER_PIXEL == 16
    color_t color = ggl_rgb32_to_rgb16(red, green, blue);
    return ggl_solid_pattern(color);
#else
#    error Invalid BITS_PER_PIXEL
#endif //
}


static inline color16_t ggl_color_to_rgb16(color_t color)
// ----------------------------------------------------------------------------
//   Convert ggl color to a standard rgb16
// ----------------------------------------------------------------------------
{
#if BITS_PER_PIXEL == 1
    uint8_t value = color.value ? 0xFF : 0x00;
    return ggl_rgb32_to_rgb16(value, value, value);
#elif BITS_PER_PIXEL == 4
    uint8_t value = color.value | (color.value << 4);
    return ggl_rgb32_to_rgb16(value, value, value);
#elif BITS_PER_PIXEL == 16
    return color;
#else
#    error Invalid BITS_PER_PIXEL
#endif //
}


static inline uint8_t ggl_red(color16_t color)
// ----------------------------------------------------------------------------
//   Convert the red level of a RGB16 to a 8-bit level
// ----------------------------------------------------------------------------
{
    return (color.rgb16.red << 3) | (color.rgb16.red >> 2);
}


static inline uint8_t ggl_green(color16_t color)
// ----------------------------------------------------------------------------
//   Convert the green level to a standard 8-bit level
// ----------------------------------------------------------------------------
{
    return (color.rgb16.green << 2) | (color.rgb16.green >> 3);
}


static inline uint8_t ggl_blue(color16_t color)
// ----------------------------------------------------------------------------
//   Convert the blue level to a standard 8-bit level
// ----------------------------------------------------------------------------
{
    return (color.rgb16.blue << 3) | (color.rgb16.blue >> 2);
}



// ============================================================================
//
//   Palette management
//
// ============================================================================

// Default palette size, entries 0-15 are for grayscale conversion, entries
// above 16 are customizable Theme colors for different elements of the UI
#define PALETTE_SIZE 64
#define PALETTE_MASK 63

// Global palette, can be used for grayscale conversion or for themes
extern pattern_t      ggl_palette[PALETTE_SIZE];

static inline pattern_t ggl_color(palette_index index)
// ----------------------------------------------------------------------------
//   Find a color from a palette entry
// ----------------------------------------------------------------------------
{
    return ggl_palette[index & PALETTE_MASK];
}


static inline void ggl_palette_set(palette_index index,
                                   uint8_t       red,
                                   uint8_t       green,
                                   uint8_t       blue)
// ----------------------------------------------------------------------------
//   Set a color from a standard RGB16 value
// ----------------------------------------------------------------------------
{
    ggl_palette[index & PALETTE_MASK] = ggl_rgb_to_pattern(red, green, blue);
}


// ============================================================================
//
//   Bit blitting
//
// ============================================================================

void       ggl_init_screen(gglsurface *surface);
gglsurface ggl_bitmap(pixword *bits, size width, size height);
gglsurface ggl_grob(word_p bmp);


typedef pixword (*gglop)(pixword dst, pixword src, pixword arg);


// ============================================================================
//
//   Operators for ggl_blit
//
// ============================================================================

static inline pixword ggl_op_set(pixword dst, pixword src, pixword arg)
// ----------------------------------------------------------------------------
//   This simly sets the color passed in arg
// ----------------------------------------------------------------------------
{
    UNUSED(dst);
    UNUSED(src);
    return arg;
}

static inline pixword ggl_op_source(pixword dst, pixword src, pixword arg)
// ----------------------------------------------------------------------------
//   This simly sets the color from the source
// ----------------------------------------------------------------------------
{
    UNUSED(dst);
    UNUSED(arg);
    return src;
}

static inline pixword ggl_op_mono_fg_1bpp(pixword dst, pixword src, pixword arg)
// ----------------------------------------------------------------------------
//   Bitmap foreground colorization (1bpp destination)
// ----------------------------------------------------------------------------
{
    // For 1bpp, 'arg' is simply a bit mask from the source
    return (dst & ~src) | (arg & src);
}


static inline pixword ggl_op_mono_bg_1bpp(pixword dst, pixword src, pixword arg)
// ----------------------------------------------------------------------------
//   Bitmap baground colorization (1bpp destination)
// ----------------------------------------------------------------------------
{
    return ggl_op_mono_fg_1bpp(dst, ~src, arg);
}


static inline pixword ggl_op_mono_fg_4bpp(pixword dst, pixword src, pixword arg)
// ----------------------------------------------------------------------------
//   Bitmap foreground colorization (4bpp destination)
// ----------------------------------------------------------------------------
{
    // Expand the 8 bits from the source into a 32-bit mask
    pixword mask = 0;
    for (unsigned shift = 0; shift < 8; shift++)
        if (src & (1 << shift))
            mask |= 0xF << (4 * shift);
    return (dst & ~mask) | (arg & mask);
}


static inline pixword ggl_op_mono_bg_4bpp(pixword dst, pixword src, pixword arg)
// ----------------------------------------------------------------------------
//   Bitmap background colorization (4bpp destination)
// ----------------------------------------------------------------------------
{
    return ggl_op_mono_fg_4bpp(dst, ~src, arg);
}


static inline pixword ggl_op_mono_fg_16bpp(pixword dst,
                                           pixword src,
                                           pixword arg)
// ----------------------------------------------------------------------------
//   Bitmap foreground colorization (16bpp destination)
// ----------------------------------------------------------------------------
{
    // Expand the low 2 bits from the source into a 32-bit mask
    pixword mask = 0;
    for (unsigned shift = 0; shift < 2; shift++)
        if (src & (1 << shift))
            mask |= 0xFFFF << (16 * shift);
    return (dst & ~mask) | (arg & mask);
}


static inline pixword ggl_op_mono_bg_16bpp(pixword dst,
                                           pixword src,
                                           pixword arg)
// ----------------------------------------------------------------------------
//   Bitmap background colorization (16bpp destination)
// ----------------------------------------------------------------------------
{
    return ggl_op_mono_fg_16bpp(dst, ~src, arg);
}


static inline pixword ggl_op_mono_fg(pixword dst, pixword src, pixword arg)
// ----------------------------------------------------------------------------
//   The foreground variant that is appropriate for the hardware display
// ----------------------------------------------------------------------------
{
    return CAT(CAT(ggl_op_mono_fg_, BITS_PER_PIXEL), bpp)(dst, src, arg);
}


static inline pixword ggl_op_mono_bg(pixword dst, pixword src, pixword arg)
// ----------------------------------------------------------------------------
//   The background variant that is appropriate for the hardware display
// ----------------------------------------------------------------------------
{
    return CAT(CAT(ggl_op_mono_bg_, BITS_PER_PIXEL), bpp)(dst, src, arg);
}


static inline pixword ggl_flt_lighten_1bpp(pixword dst,
                                           pixword src,
                                           pixword arg)
// ----------------------------------------------------------------------------
//   Lighten the destination - Monochrome version (0 = white, 1 = black)
// ----------------------------------------------------------------------------
{
    dst = src & arg;
    return dst;
}


static inline pixword ggl_flt_lighten_4bpp(pixword dst,
                                           pixword src,
                                           pixword arg)
// ----------------------------------------------------------------------------
//   Lighten the destination - Grayscale version (0xF = black, 0x0 = white)
// ----------------------------------------------------------------------------
{
    dst = 0;
    for (unsigned shift = 0; shift < BITS_PER_WORD; shift += 4)
    {
        int data = ((src >> shift) & 0xF) - ((arg >> shift) & 0xF);
        if (data < 0)
            data = 0;
        dst |= data << shift;
    }
    return dst;
}


static inline pixword ggl_flt_lighten_16bpp(pixword dst,
                                            pixword src,
                                            pixword arg)
// ----------------------------------------------------------------------------
//   Lighten - 16bpp version - Lighter value are higher
// ----------------------------------------------------------------------------
{
    dst = 0;
    for (unsigned shift = 0; shift < BITS_PER_WORD; shift += 16)
    {
        color16_t acol = { .value = (uint16_t) (arg >> shift) };
        color16_t scol = { .value = (uint16_t) (src >> shift) };
        color16_t dcol = {
            .rgb16 = {
                .blue  = (uint8_t) min(acol.rgb16.blue + scol.rgb16.blue, 31),
                .green = (uint8_t) min(acol.rgb16.green + scol.rgb16.green, 63),
                .red   = (uint8_t) min(acol.rgb16.red + scol.rgb16.red, 31),
            }
        };
        dst |= dcol.value << shift;
    }
    return dst;
}

static inline pixword ggl_flt_lighten(pixword dst, pixword src, pixword arg)
// ----------------------------------------------------------------------------
//   Lighten for default bits per pixel
// ----------------------------------------------------------------------------
{
    return CAT(CAT(ggl_flt_lighten_, BITS_PER_PIXEL), bpp)(dst, src, arg);
}


static inline pixword ggl_flt_darken_1bpp(pixword dst, pixword src, pixword arg)
// ----------------------------------------------------------------------------
//   Darken the destination - Monochrome version (0 = white, 1 = black)
// ----------------------------------------------------------------------------
{
    dst = src | arg;
    return dst;
}


static inline pixword ggl_flt_darken_4bpp(pixword dst, pixword src, pixword arg)
// ----------------------------------------------------------------------------
//   Darken the destination - Grayscale version (0xF = black, 0x0 = white)
// ----------------------------------------------------------------------------
{
    dst = 0;
    for (unsigned shift = 0; shift < BITS_PER_WORD; shift += 4)
    {
        int data = ((src >> shift) & 0xF) + ((arg >> shift) & 0xF);
        if (data > 0xF)
            data = 0xF;
        dst |= data << shift;
    }
    return dst;
}


static inline pixword ggl_flt_darken_16bpp(pixword dst,
                                           pixword src,
                                           pixword arg)
// ----------------------------------------------------------------------------
//   Darken - 16bpp version
// ----------------------------------------------------------------------------
{
    dst = 0;
    for (unsigned shift = 0; shift < BITS_PER_WORD; shift += 16)
    {
        color16_t acol = { .value = (uint16_t) (arg >> shift) };
        color16_t scol = { .value = (uint16_t) (src >> shift) };
        color16_t dcol = {
            .rgb16 = {
                .blue  = (uint8_t) max(acol.rgb16.blue  - scol.rgb16.blue, 0),
                .green = (uint8_t) max(acol.rgb16.green - scol.rgb16.green, 0),
                .red   = (uint8_t) max(acol.rgb16.red   - scol.rgb16.red, 0),
            }
        };
        dst |= dcol.value << shift;
    }
    return dst;
}

static inline pixword ggl_flt_darken(pixword dst, pixword src, pixword arg)
// ----------------------------------------------------------------------------
//   Darken for default bits per pixel
// ----------------------------------------------------------------------------
{
    return CAT(CAT(ggl_flt_darken_, BITS_PER_PIXEL), bpp)(dst, src, arg);
}


static inline pixword ggl_flt_invert(pixword dst, pixword src, pixword arg)
// ----------------------------------------------------------------------------
//   Inverting colors can always be achieved with a simple xor
// ----------------------------------------------------------------------------
{
    dst = src ^ arg;
    return dst;
}


// ============================================================================
//
//    Core blitting routine
//
// ============================================================================

typedef enum clip
{
    CLIP_NONE        = 0,
    CLIP_SRC         = 1,
    CLIP_DST         = 2,
    CLIP_ALL         = 3,
    CLIP_SKIP_SOURCE = 4,
    CLIP_SKIP_COLOR  = 8,
} clip_t;


static inline void ggl_mixblit(gglsurface *dst,    // Destination surface
                               gglsurface *src,    // Source surface
                               coord       left,   // Destination rectangle
                               coord       right,  // (0,0) is top/left
                               coord       top,    // Nothing happens if
                               coord       bottom, // left>right or top>bottom
                               coord       x,      // Source position
                               coord       y,      // (0,0) is top-left
                               gglop       op,     // Operation on pixels
                               pattern_t   colors, // Parameter to operation
                               unsigned    dbpp,   // Bits per pixel in dst
                               unsigned    sbpp,   // Bits per pixel in src
                               unsigned    cbpp,   // Bits per pixel in colors
                               clip_t      clip)        // Clipping operations
// ----------------------------------------------------------------------------
//   Generalized multi-bpp blitting routine
// ----------------------------------------------------------------------------
//   This transfers pixels from 'src' to 'dst' (which can be equal)
//   - targeting a rectangle defined by (left, right, top, bottom)
//   - fetching pixels from (x,y) in the source
//   - applying the given operation in 'op'
//
//   Everything is inline so that the compiler can optimize code away
//
//   The code selects the correct direction for copies within the same surface,
//   so it is safe to use for operations like scrolling.
//
//   We pass the bits-per-pixel as arguments to make it easier for the optimizer
//   to replace multiplications with shifts when we pasa a constant.
//   Additional flags can be used to statically disable sections of the code.
//
//   An arbitrary gglop is passed, which can be used to process each set of
//   pixels in turn. That operation is dependent on the respective bits per
//   pixesls, and can be used e.g. to do bit-plane conversions. See how this is
//   used in DrawText to colorize 1bpp bitplanes. The source and color pattern
//   are both aligned to match the destination before the operator is called.
//   So if the source is 1bpp and the destination is 4bpp, you might end up
//   with the 8 low bits of the source being the bit pattern that applies to the
//   8 nibbles in the destination word.
{
    int clip_src = clip & CLIP_SRC;
    int clip_dst = clip & CLIP_DST;
    int skip_src = clip & CLIP_SKIP_SOURCE;
    int skip_col = clip & CLIP_SKIP_COLOR;

    if (clip_src)
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

    if (clip_dst)
    {
        // Clipping based on target
        if (left < dst->left)
        {
            x += dst->left - left;
            left = dst->left;
        }
        if (right > dst->right)
            right = dst->right;
        if (top < dst->top)
        {
            y += dst->top - top;
            top = dst->top;
        }
        if (bottom > dst->bottom)
            bottom = dst->bottom;
    }

    // Bail out if there is no effect
    if (left > right || top > bottom)
        return;

    // Check whether we need to go forward or backward along X or Y
    int        xback  = x < left;
    int        yback  = y < top;
    int        xdir   = xback ? -1 : 1;
    int        ydir   = yback ? -1 : 1;
    coord      dx1    = xback ? right : left;
    coord      dx2    = xback ? left : right;
    coord      dy1    = yback ? bottom : top;
    coord      sl     = x;
    coord      sr     = sl + right - left;
    coord      st     = y;
    coord      sb     = st + bottom - top;
    coord      sx1    = xback ? sr : sl;
    coord      sy1    = yback ? sb : st;
    coord      ycount = bottom - top;

    // Offset in words for a displacement along Y
    offset     dyoff  = ggl_pixel_offset(dst, dbpp, 0, ydir);
    offset     syoff  = skip_src ? 0 : ggl_pixel_offset(src, sbpp, 0, ydir);
    size       swidth = src->width;
    unsigned   sslant = skip_src ? 0 : ggl_pixel_shift(src, sbpp, swidth, 0);
    size       dwidth = dst->width;
    unsigned   dslant = ggl_pixel_shift(dst, dbpp, dwidth, 0);

    // Pointers to word containing start and end pixel
    pixword   *dp1    = ggl_pixel_address(dst, dbpp, dx1, dy1);
    pixword   *dp2    = ggl_pixel_address(dst, dbpp, dx2, dy1);
    pixword   *sp     = skip_src ? dp1 : ggl_pixel_address(src, sbpp, sx1, sy1);

    // Left and right pixel shift
    unsigned   dls    = ggl_pixel_shift(dst, dbpp, left, dy1);
    unsigned   drs    = ggl_pixel_shift(dst, dbpp, right, dy1);
    unsigned   dws    = xback ? drs : dls;
    unsigned   sls    = ggl_pixel_shift(src, sbpp, sl, sy1);
    unsigned   srs    = ggl_pixel_shift(src, sbpp, sr, sy1);
    unsigned   sws    = xback ? srs : sls;
    const size bpw    = BITS_PER_WORD;
    unsigned   cshift = (cbpp == 16 ? 32 : cbpp == 4 ? 16 : cbpp == 1 ? 8 : 0);
    unsigned   cys    = ydir * (int) cshift;
    unsigned   cxs    = xdir * bpw * cbpp / dbpp;

    // Shift adjustment from source to destinaation
    unsigned   sadj   = (int) (sws * dbpp - dws * sbpp) / (int) dbpp;
    unsigned   sxadj  = xdir * (int) (sbpp * bpw / dbpp);

    // Left and right masks
    pixword    ones   = ~0U;
    pixword    lmask  = ones << dls;
    pixword    rmask  = shrc(ones, drs + dbpp);
    pixword    dmask1 = xback ? rmask : lmask;
    pixword    dmask2 = xback ? lmask : rmask;

    // Adjust the color pattern based on starting point
    uint64_t   cdata64 = colors.bits;
    if (!skip_col)
        cdata64 = ggl_rotate_pattern(cdata64, dx1 * cbpp + dy1 * cshift - dws);


    while (ycount-- >= 0)
    {
        uint64_t csave    = cdata64;
        unsigned sadjsave = sadj;
        pixword *ssave    = sp;
        pixword *dp       = dp1;
        pixword  dmask    = dmask1;
        int      xdone    = 0;
        pixword  smem     = sp[0];
        pixword  snew     = smem;
        pixword  sdata    = 0;
        pixword  cdata    = 0;

        if (xback)
            sadj -= sxadj;

        do
        {
            xdone = dp == dp2;
            if (xdone)
                dmask &= dmask2;

            if (!skip_src)
            {
                unsigned nextsadj = sadj + sxadj;

                // Check if we change source word
                int skip = nextsadj >= bpw;
                if (skip)
                {
                    sp += xdir;
                    smem = snew;
                    snew = sp[0];
                }

                nextsadj %= bpw;
                sadj %= bpw;
                if (sadj)
                    if (xback)
                        sdata = shlc(smem, nextsadj) | shr(snew, nextsadj);
                    else
                        sdata = shlc(snew, sadj) | shr(smem, sadj);
                else
                    sdata = snew;

                sadj = nextsadj;
            }
            if (!skip_col)
            {
                cdata   = cdata64;
                cdata64 = ggl_rotate_pattern(cdata64, cxs);
            }

            pixword ddata = dp[0];
            pixword tdata = op(ddata, sdata, cdata);
            *dp           = (tdata & dmask) | (ddata & ~dmask);

            dp += xdir;
            dmask = ~0U;
            smem = snew;
        } while (!xdone);

        // Move to next line
        if (dslant)
        {
            dy1 += ydir;
            dp1     = ggl_pixel_address(dst, dbpp, dx1, dy1);
            dp2     = ggl_pixel_address(dst, dbpp, dx2, dy1);
            dls     = ggl_pixel_shift(dst, dbpp, left, dy1);
            drs     = ggl_pixel_shift(dst, dbpp, right, dy1);
            dws     = xback ? drs : dls;
            lmask   = ones << dls;
            rmask   = shrc(ones, drs + dbpp);
            dmask1  = xback ? rmask : lmask;
            dmask2  = xback ? lmask : rmask;
            cdata64 = colors.bits;
            cdata64 =
                ggl_rotate_pattern(cdata64, dx1 * cbpp + dy1 * cshift - dws);
        }
        else
        {
            dp1 += dyoff;
            dp2 += dyoff;
            if (!skip_col)
                cdata64 = ggl_rotate_pattern(csave, cys);
        }

        // Check if we can directly move to next line
        if (sslant || dslant)
        {
            sy1 += ydir;
            sp   = ggl_pixel_address(src, sbpp, sx1, sy1);
            sls  = ggl_pixel_shift(src, sbpp, sl, sy1);
            srs  = ggl_pixel_shift(src, sbpp, sr, sy1);
            sws  = xback ? srs : sls;
            sadj = (int) (sws * dbpp - dws * sbpp) / (int) dbpp;
        }
        else
        {
            sp   = ssave + syoff;
            sadj = sadjsave;
        }
    }
}

static inline void ggl_blit(gglsurface *d, // Destination surface
                            gglsurface *s, // Source surface
                            coord       l, // Destination rectangle
                            coord       r, // (0,0) is top/left
                            coord       t, // Nothing happens if
                            coord       b, // left>right or top>bottom
                            coord       x, // Source position
                            coord       y, // (0,0) is top-left
                            gglop       o, // Operation on pixels
                            pattern_t   c, // Parameter to operation
                            clip_t      clip)
{
    const size bpp = BITS_PER_PIXEL;
    ggl_mixblit(d, s, l, r, t, b, x, y, o, c, bpp, bpp, bpp, clip);
}


// ============================================================================
//
// General drawing primitives based on ggl_blit
//
// ============================================================================

static inline void ggl_rect(gglsurface *srf,
                            int         x1,
                            int         y1,
                            int         x2,
                            int         y2,
                            pattern_t   color)
{
    ggl_blit(srf, srf, x1, x2, y1, y2, 0, 0, ggl_op_set, color, CLIP_NONE);
}

static inline void ggl_cliprect(gglsurface *srf,
                                int         x1,
                                int         y1,
                                int         x2,
                                int         y2,
                                pattern_t   c)
{
    ggl_blit(srf, srf, x1, x2, y1, y2, 0, 0, ggl_op_set, c, CLIP_DST);
}

static inline void ggl_clear(gglsurface *srf, pattern_t color)
{
    ggl_rect(srf, srf->left, srf->top, srf->right, srf->bottom, color);
}

static inline void ggl_hline(gglsurface *srf,
                             int         y,
                             int         xl,
                             int         xr,
                             pattern_t   color)
{
    ggl_blit(srf, srf, xl, xr, y, y, 0, 0, ggl_op_set, color, CLIP_NONE);
}

static inline void ggl_cliphline(gglsurface *srf,
                                 int         y,
                                 int         xl,
                                 int         xr,
                                 pattern_t   color)
{
    ggl_blit(srf, srf, xl, xr, y, y, 0, 0, ggl_op_set, color, CLIP_DST);
}

static inline void ggl_vline(gglsurface *srf,
                             int         x,
                             int         yt,
                             int         yb,
                             pattern_t   colors)
{
    ggl_blit(srf, srf, x, x, yt, yb, 0, 0, ggl_op_set, colors, CLIP_NONE);
}

static inline void ggl_clipvline(gglsurface *srf,
                                 int         x,
                                 int         yt,
                                 int         yb,
                                 pattern_t   colors)
{
    ggl_blit(srf, srf, x, x, yt, yb, 0, 0, ggl_op_set, colors, CLIP_DST);
}

static inline void ggl_filter(gglsurface *dst,
                              size        width,
                              size        height,
                              gglop       filter,
                              pattern_t   param)
{
    ggl_blit(dst,
             dst,
             0,
             width - 1,
             0,
             height - 1,
             0,
             0,
             filter,
             param,
             CLIP_DST);
}

static inline void ggl_copy_at(gglsurface *dst,
                               gglsurface *src,
                               coord       x,
                               coord       y,
                               size        width,
                               size        height)
{
    pattern_t clear = { .bits = 0 };
    ggl_blit(dst,
             src,
             x,
             x + width - 1,
             y,
             y + height - 1,
             0,
             0,
             ggl_op_source,
             clear,
             CLIP_ALL);
}

static inline void ggl_copy(gglsurface *dst,
                            gglsurface *src,
                            size        width,
                            size        height)
{
    ggl_copy_at(dst, src, 0, 0, width, height);
}

static inline void ggl_copy_from(gglsurface *dst,
                                 gglsurface *src,
                                 coord       x,
                                 coord       y,
                                 size        width,
                                 size        height)
{
    pattern_t clear = { .bits = 0 };
    ggl_blit(dst,
             src,
             0,
             width - 1,
             0,
             height - 1,
             x,
             y,
             ggl_op_source,
             clear,
             CLIP_ALL);
}



// LOW-LEVEL row copy functions
// ggl_ll_hblt is a general nibble-aligned memcpyb
// WARNING: npixels is limited to 512!!
//          if you need more than that, increase the constant HBLT_BUFFER above
//          and RE-COMPILE the ggl library
// dest and src must be word-aligned
// destoff and srcoff are offsets in nibbles from the word-aligned pointers
// npixels is the number of nibbles to copy
// note: hblt will behave well even if the zones overlap, no need for
// moveup/movedown

void ggl_hblt(pixword *dest,
              int      destoff,
              pixword *src,
              int      srcoff,
              size     npixels); // copy a row of pixels

// same behavior as hblt but specifying a transparent color
// every pixel in *src with the transparent color will not affect the
// corresponding pixel in *dest
void ggl_hbltmask(pixword *dest,
                  int      destoff,
                  pixword *src,
                  int      srcoff,
                  size     npixels,
                  int      tcol); // copy a row of pixels w/mask

// rectangle scrolling routines
// dest contains the surface to scroll, and width and height define the
// rectangle the area that needs to be redrawn after the scroll is not erased or
// modified by these routines
void ggl_scrolllf(gglsurface *dest,
                  size        width,
                  size        height,
                  size        npixels); // scroll npixels left
void ggl_scrollrt(gglsurface *dest,
                  size        width,
                  size        height,
                  size        npixels); // scroll npixels right

#endif
