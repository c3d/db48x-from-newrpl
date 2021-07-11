/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <xgl.h>

void cgl_vline(gglsurface * srf, int x, int yt, int yb, int color)
{
    // PAINTS A VERTICAL LINE FROM yt TO yb BOTH INCLUSIVE
    // color=number from 0 to 15
    // RESTRICTIONS: yb>=yt

    int offset = srf->width * yt + x;

    unsigned short *ptr=(unsigned short *)srf->addr+offset;
    while(yt <= yb) {
        *ptr=(unsigned short int)color;
        ptr += srf->width;
        ++yt;
    }

}

// SAME AS VLINE BU WITH CLIPPING
void cgl_clipvline(gglsurface * srf, int x, int yt, int yb, int color)
{
    // PAINTS A VERTICAL LINE FROM yt TO yb BOTH INCLUSIVE
    // color=number from 0 to 15
    // RESTRICTIONS: yb>=yt

    if(yt > srf->clipy2)
        return;
    if(yb < srf->clipy)
        return;

    if(yt < srf->clipy)
        yt = srf->clipy;
    if(yb > srf->clipy2)
        yb = srf->clipy2;

    if(x < srf->clipx)
        return;
    if(x > srf->clipx2)
        return;

   cgl_vline(srf,x,yt,yb,color);

}
