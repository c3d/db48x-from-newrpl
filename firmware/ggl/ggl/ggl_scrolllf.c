/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

void ggl_scrolllf(gglsurface * dest, int width, int height, int npixels)
{

// SCROLLS A RECTANGULAR REGION LEFT npixelsup
// gglsurface CONTAINS THE BUFFER AND OFFSETS
// width AND height ARE THE SIZE OF THE BLOCK TO MOVE

// RESTRICTIONS: NO SAFETY CHECKS REGARDING MEMORY MOVEMENTS

    gglsurface srf;

    srf.addr = dest->addr;
    srf.width = dest->width;
    srf.x = dest->x + npixels;
    srf.y = dest->y;

    ggl_bitblt(dest, &srf, width - npixels, height);

}
