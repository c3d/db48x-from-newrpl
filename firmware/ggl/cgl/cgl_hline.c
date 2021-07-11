/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <xgl.h>

// VERSION DRAWS HORIZONTAL LINES IN A COLOR BITMAP
// Supports 16-bit per pixel RGB555
// Color is a single color, no pattern supported

void cgl_hline(gglsurface * srf, int y, int xl, int xr, int color)
{
    // PAINTS A HORIZONTAL LINE FROM xl TO xr BOTH INCLUSIVE
    // color=COLORED PATTERN TO USE, 8 PIXELS - 1 NIBBLE PER PIXEL
    //         PATTERN IS ALWAYS WORD ALIGNED

    // RESTRICTIONS: xr>=xl
    //                 y MUST BE VALID

unsigned short int *ptr=(unsigned short int *)srf->addr+y*srf->width+xl;

while(xl<=xr) {
    *ptr++=color;
    ++xl;
}

}

void cgl_cliphline(gglsurface * srf, int y, int xl, int xr, int color)
{
    // PAINTS A HORIZONTAL LINE FROM xl TO xr BOTH INCLUSIVE
    // color=COLORED PATTERN TO USE, 8 PIXELS - 1 NIBBLE PER PIXEL
    //         PATTERN IS ALWAYS WORD ALIGNED

    // RESTRICTIONS: xr>=xl
    //                 y MUST BE VALID

    if(y < srf->clipy)
        return;
    if(y > srf->clipy2)
        return;
    if(xr < srf->clipx)
        return;
    if(xl > srf->clipx2)
        return;

    if(xl < srf->clipx)
        xl = srf->clipx;
    if(xr > srf->clipx2)
        xr = srf->clipx2;

    cgl_hline(srf,y,xl,xr,color);
}
