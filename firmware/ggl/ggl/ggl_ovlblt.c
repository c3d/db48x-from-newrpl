/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

void ggl_ovlblt(gglsurface * dest, gglsurface * src, int width, int height)
{

// SAME AS ggl_bitblt BUT WITH OVERLAPPING CHECK
// IT COPIES CORRECTLY WHEN dest AND src ZONES OVERLAP

// COPIES A RECTANGULAR REGION FROM src TO dest
// gglsurface CONTAINS THE BUFFER AND OFFSETS
// width AND height ARE THE SIZE OF THE BLOCK TO MOVE

// RESTRICTIONS: NO SAFETY CHECKS REGARDING MEMORY MOVEMENTS

    int doff, soff;
    int *dst, *dend, *st;       //,*send;

// CHECK FOR OVERLAPPING ZONES

    doff = dest->y * dest->width + dest->x;
    soff = src->y * src->width + src->x;

    dst = dest->addr + (doff >> 3);
    st = src->addr + (soff >> 3);
    dend = dst + ((height * dest->width) >> 3);
    //send=st+((height*src->width)>>3);

    if(dst >= st && dst <= dend)
        ggl_revblt(dest, src, width, height);
    else
        ggl_bitblt(dest, src, width, height);
}
