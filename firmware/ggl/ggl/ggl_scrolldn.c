/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <ggl.h>


void ggl_scrolldn(gglsurface *dest,int width, int height, int npixels)
{

// SCROLLS A RECTANGULAR REGION DOWN npixels
// gglsurface CONTAINS THE BUFFER AND OFFSETS
// width AND height ARE THE SIZE OF THE BLOCK TO MOVE

// RESTRICTIONS: NO SAFETY CHECKS REGARDING MEMORY MOVEMENTS

gglsurface srf;

srf.addr=dest->addr;
srf.width=dest->width;
srf.x=dest->x;
srf.y=dest->y+npixels;

ggl_revblt(&srf,dest,width,height-npixels);

}


