/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

void ggl_bitblt(gglsurface *dest, gglsurface *src, size width, size height)
{
    // COPIES A RECTANGULAR REGION FROM src TO dest
    // gglsurface CONTAINS THE BUFFER AND OFFSETS
    // width AND height ARE THE SIZE OF THE BLOCK TO MOVE

    // RESTRICTIONS: NO SAFETY CHECKS REGARDING MEMORY MOVEMENTS

    size doff, soff, line;

    doff = dest->y * dest->width + dest->x;
    soff = src->y * src->width + src->x;

    for (line = 0; line < height; ++line)
    {
        ggl_hblt(dest->pixels, doff, src->pixels, soff, width);
        doff += dest->width;
        soff += src->width;
    }
}

void ggl_bitbltclip(gglsurface *dest, gglsurface *src, size width, size height)
{
    // SOURCE CLIPPING FIRST - REDUNDANT
#ifndef TARGET_PRIME1
    /*
       if(src->x+width<0) return;
       if(src->x>=src->width) return;
       if(src->y+height<0) return;
       if(src->y>=src->height) return;
       if(src->x<0) { dest->x-=src->x; width+=src->x; src->x=0; }
       if(src->y<0) { dest->y-=src->y; height+=src->y; src->y=0; }
       if(src->x+width>src->width) { width=src->width-src->x; }
     */
#endif /* ! TARGET_PRIME1 */

    // DESTINATION CLIPPING ONLY
    if (dest->x > dest->right)
        return;
    if (dest->y > dest->bottom)
        return;
    if (dest->x + (coord) width < dest->left)
        return;
    if (dest->y + (coord) height < dest->top)
        return;
    if (dest->x < dest->left)
        dest->x = dest->left;
    if (dest->x + (coord) width > dest->right)
        width = dest->right - dest->x + 1;
    if (dest->y < dest->top)
    {
        src->y += dest->top - dest->y;
        height -= dest->top - dest->y;
        dest->y = dest->top;
    }
    if (dest->y + (coord) height > dest->bottom)
        height = dest->bottom - dest->y + 1;

    ggl_bitblt(dest, src, width, height);
}
