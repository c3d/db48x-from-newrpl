/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

#define ggl_op(surface, left, right, top, bottom, clip, blit)
do
{

} while (0)

void ggl_op(gglsurface *s,
            int         left,
            int         right,
            int         top,
            int         bottom,
             pattern_t   pat)
// ----------------------------------------------------------------------------
//   Draw a patterned rectangle within the given limits
// ----------------------------------------------------------------------------
{
    if (left < s->left)
        left = s->left;
    if (right > s->right)
        right = s->right;
    if (top < s->top)
        top = s->top;
    if (bottom > s->bottom)
        bottom = s->bottom;

    if (right < left || bottom < top)
        return;

    pixword *lp       = ggl_pixel_ptr(s, left, top);
    pixword *rp       = ggl_pixel_ptr(s, right, top);
    pixword  lm       = ~0U << ((left * BITS_PER_PIXEL) % BITS_PER_WORD);
    pixword  rm       = ~0U >> (BITS_PER_WORD - (left * BITS_PER_PIXEL) % BITS_PER_WORD);
    size     scanline = ggl_offset(s, 0, 1);
    pat               = ggl_rotate_pattern(pat, left + top);

    if (lp == rp)
    {
        // Left and right are the same word: single mask for left and right
        pixword mask = lm & rm;
        for (coord y = top; y <= bottom; y++, lp += scanline)
        {
            *lp = (pat.word & mask) | (*lp & ~mask);
            pat = ggl_rotate_pattern(pat, 1);
        }
    }
    else
    {
        // Real masking required for the left column
        for (coord y = top; y <= bottom; y++, lp += scanline)
        {
            pixword *p = lp;
            *p = (pat.word & lm) | (*p & ~lm);
            p++;
            pat.bits = ggl_rotate_pattern_bits(pat.bits, BITS_PER_WORD);
            while (p < rp)
            {
                *p++ = pat.word;
                pat.bits = ggl_rotate_pattern_bits(pat.bits, BITS_PER_WORD);
            }
            *p = (pat.word & rm) | (*p & ~rm);
        }
    }
}


void ggl_rect(gglsurface *srf, int x1, int y1, int x2, int y2, pattern_t color)
{
    // DRAWS A RECTANGLE BETWEEN x1,y1 and x2,y2 ALL INCLUSIVE
    // color CAN BE AN 8-BIT PATTERN THAT REPEATS VERTICALLY
    // RESTRICTIONS:
    //        NO BOUNDARY CHECKS
    //        y2>=y1 && x2>=x1

    while (y1 <= y2)
    {
        ggl_hline(srf, y1, x1, x2, color);
        ++y1;
    }
}

void ggl_cliprect(gglsurface *srf, int x1, int y1, int x2, int y2, color_t color)
{
    // SAME AS ggl_rect BUT WITH CLIPPING

    if (x1 > x2)
    {
        int tmp;
        tmp = x1;
        x1  = x2;
        x2  = tmp;
    }
    if (y1 > y2)
    {
        int tmp;
        tmp = y1;
        y1  = y2;
        y2  = tmp;
    }

    if (x1 > srf->right)
        return;
    if (y1 > srf->bottom)
        return;
    if (y2 < srf->top)
        return;
    if (x2 < srf->left)
        return;

    if (x1 < srf->left)
        x1 = srf->left;
    if (y1 < srf->top)
        y1 = srf->top;
    if (x2 > srf->right)
        x2 = srf->right;
    if (y2 > srf->bottom)
        y2 = srf->bottom;

    // DRAWS A RECTANGLE BETWEEN x1,y1 and x2,y2 ALL INCLUSIVE
    // color CAN BE AN 8-BIT PATTERN THAT REPEATS VERTICALLY
    // RESTRICTIONS:
    //        NO BOUNDARY CHECKS
    //        y2>=y1 && x2>=x1

    while (y1 <= y2)
    {
        ggl_hline(srf, y1, x1, x2, color);
        ++y1;
    }
}
