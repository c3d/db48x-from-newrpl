/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

void ggl_initscr(gglsurface *srf)
{
    srf->pixels = (pixword *) MEM_PHYS_SCREEN;
    srf->width  = LCD_SCANLINE;
    srf->height = LCD_H;
    srf->bpp    = BITS_PER_PIXEL;
    srf->x      = 0;
    srf->y      = 0;
    srf->left   = 0;
    srf->top    = 0;
    srf->right  = LCD_W - 1;
    srf->bottom = LCD_H - 1;

#if SCREEN_BUFFERS > 1
    srf->active_buffer = lcd_getactivebuffer();
    srf->pixels += LCD_W * LCD_H / PIXELS_PER_WORD * srf->active_buffer;
#else
    srf->active_buffer = 0;
#endif
}


gglsurface ggl_monochrome_bitmap(pixword *bits, size width, size height)
{
    gglsurface result = {
        .pixels = bits,
        .width  = width,
        .height = height,
        .bpp    = 1,
        .x      = 0,
        .y      = 0,
        .left   = 0,
        .top    = 0,
        .right  = width - 1,
        .bottom = height - 1,
    };
    return result;
}


// Global palette, can be used for grayscale conversion or for themes
color_t ggl_palette[PALETTE_SIZE] SYSTEM_GLOBAL;
