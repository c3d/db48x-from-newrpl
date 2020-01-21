/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

void ggl_revblt(gglsurface * dest, gglsurface * src, int width, int height)
{
// REVERSE bitblt
// COPIES A RECTANGULAR REGION FROM src TO dest
// gglsurface CONTAINS THE BUFFER AND OFFSETS
// width AND height ARE THE SIZE OF THE BLOCK TO MOVE

// RESTRICTIONS: NO SAFETY CHECKS REGARDING MEMORY MOVEMENTS

    int doff, soff, line;

    doff = dest->y * dest->width + dest->x;
    soff = src->y * src->width + src->x;

    // REVERSE LOOP NEEDED
    doff += dest->width * height;
    soff += src->width * height;

    for(line = 0; line < height; ++line) {
        doff -= dest->width;
        soff -= src->width;
        ggl_hblt(dest->addr, doff, src->addr, soff, width);
    }

}
