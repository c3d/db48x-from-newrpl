/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

void ggl_rectp(gglsurface * srf, int x1, int y1, int x2, int y2, int *color)
{
// DRAWS A RECTANGLE BETWEEN x1,y1 and x2,y2 ALL INCLUSIVE
// color IS AN 8x8-BIT PATTERN (int color[8])
// RESTRICTIONS:
//        NO BOUNDARY CHECKS
//        y2>=y1 && x2>=x1

    while(y1 <= y2) {
        ggl_hline(srf, y1, x1, x2, color[y1 & 7]);
        ++y1;
    }

}
