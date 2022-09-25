/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

void ggl_init_screen(gglsurface *srf)
{
#if SCREEN_BUFFERS > 1
    int active = lcd_getactivebuffer();
    size_t offset = LCD_W * LCD_H / PIXELS_PER_WORD * active;
    srf->pixels = (pixword *) MEM_PHYS_SCREEN + offset;
#else
    srf->pixels = (pixword *) MEM_PHYS_SCREEN;
#endif
    srf->width  = LCD_SCANLINE;
    srf->height = LCD_H;
    srf->bpp    = BITS_PER_PIXEL;
    srf->left   = 0;
    srf->top    = 0;
    srf->right  = LCD_W - 1;
    srf->bottom = LCD_H - 1;
}

gglsurface ggl_bitmap(pixword *bits, size width, size height)
{
    gglsurface result = {
        .pixels = bits,
        .width  = width,
        .height = height,
        .bpp    = 1,
        .left   = 0,
        .top    = 0,
        .right  = width - 1,
        .bottom = height - 1,
    };
    return result;
}

gglsurface ggl_grob(word_p bmp)
{
    // REVISIT: Check if (!ISBITMAP(*bmp))
    size width = bmp[1];
    size height = bmp[2];
    static const size bpps[8] = {
        [BITMAP_RAWMONO]  =  1,
        [BITMAP_RAW16G]   =  4,
        [BITMAP_RAW256G]  =  8,
        [BITMAP_RAW64KC]  = 16,
        [BITMAP_RAWARGB]  = 32,
        [BITMAP_EXTERNAL] = 32,
    };
    unsigned bppindex = LIBNUM(*bmp) & 7;
    size bpp = bpps[bppindex];
    gglsurface result = {
        .pixels        = (pixword *) (bmp + 3),
        .width         = width,
        .height        = height,
        .bpp           = bpp,
        .left          = 0,
        .right         = width - 1,
        .top           = 0,
        .bottom        = height - 1,
        };
    return result;
}

// Global palette, can be used for grayscale conversion or for themes
color_t ggl_palette[PALETTE_SIZE] SYSTEM_GLOBAL;
