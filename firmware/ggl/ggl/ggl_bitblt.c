/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

void ggl_bitblt(gglsurface * dest, gglsurface * src, int width, int height)
{

// COPIES A RECTANGULAR REGION FROM src TO dest
// gglsurface CONTAINS THE BUFFER AND OFFSETS
// width AND height ARE THE SIZE OF THE BLOCK TO MOVE

// RESTRICTIONS: NO SAFETY CHECKS REGARDING MEMORY MOVEMENTS

    int doff, soff, line;

    doff = dest->y * dest->width + dest->x;
    soff = src->y * src->width + src->x;

    for(line = 0; line < height; ++line) {
        ggl_hblt(dest->addr, doff, src->addr, soff, width);
        doff += dest->width;
        soff += src->width;
    }
}

void ggl_bitbltclip(gglsurface * dest, gglsurface * src, int width, int height)
{
    // SOURCE CLIPPING FIRST - REDUNDANT
    /*
       if(src->x+width<0) return;
       if(src->x>=src->width) return;
       if(src->y+height<0) return;
       if(src->y>=src->height) return;
       if(src->x<0) { dest->x-=src->x; width+=src->x; src->x=0; }
       if(src->y<0) { dest->y-=src->y; height+=src->y; src->y=0; }
       if(src->x+width>src->width) { width=src->width-src->x; }
     */

    // DESTINATION CLIPPING ONLY
    if(dest->x > dest->clipx2)
        return;
    if(dest->y > dest->clipy2)
        return;
    if(dest->x + width < dest->clipx)
        return;
    if(dest->y + height < dest->clipy)
        return;
    if(dest->x < dest->clipx)
        dest->x = dest->clipx;
    if(dest->x + width > dest->clipx2)
        width = dest->clipx2 - dest->x + 1;
    if(dest->y < dest->clipy) {
        src->y += dest->clipy - dest->y;
        height -= dest->clipy - dest->y;
        dest->y = dest->clipy;
    }
    if(dest->y + height > dest->clipy2)
        height = dest->clipy2 - dest->y + 1;

    ggl_bitblt(dest, src, width, height);

}
