/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

void ggl_rect(gglsurface * srf, int x1, int y1, int x2, int y2, int color)
{
// DRAWS A RECTANGLE BETWEEN x1,y1 and x2,y2 ALL INCLUSIVE
// color CAN BE AN 8-BIT PATTERN THAT REPEATS VERTICALLY
// RESTRICTIONS:
//        NO BOUNDARY CHECKS
//        y2>=y1 && x2>=x1

    while(y1 <= y2) {
        ggl_hline(srf, y1, x1, x2, color);
        ++y1;
    }

}

void ggl_cliprect(gglsurface * srf, int x1, int y1, int x2, int y2, int color)
{
    // SAME AS ggl_rect BUT WITH CLIPPING

    if(x1 > x2) {
        int tmp;
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if(y1 > y2) {
        int tmp;
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    if(x1 > srf->clipx2)
        return;
    if(y1 > srf->clipy2)
        return;
    if(y2 < srf->clipy)
        return;
    if(x2 < srf->clipx)
        return;

    if(x1 < srf->clipx)
        x1 = srf->clipx;
    if(y1 < srf->clipy)
        y1 = srf->clipy;
    if(x2 > srf->clipx2)
        x2 = srf->clipx2;
    if(y2 > srf->clipy2)
        y2 = srf->clipy2;

    // DRAWS A RECTANGLE BETWEEN x1,y1 and x2,y2 ALL INCLUSIVE
    // color CAN BE AN 8-BIT PATTERN THAT REPEATS VERTICALLY
    // RESTRICTIONS:
    //        NO BOUNDARY CHECKS
    //        y2>=y1 && x2>=x1

    while(y1 <= y2) {
        ggl_hline(srf, y1, x1, x2, color);
        ++y1;
    }
}
