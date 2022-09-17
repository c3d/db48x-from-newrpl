/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

pixword ggl_fltlighten(pixword color, pixword param)
{
#ifndef TARGET_PRIME1
    pixword res = 0;
    for (int f = 0; f < 8; ++f, color >>= 4)
    {
        // filter the pixel here
        if ((color & 0xf) > param)
            res |= (color & 0xf) - param;

        res = (res >> 4) | (res << 28);
    }
    return res;
#else  /* TARGET_PRIME1 */
    int red, green, blue;
    color_t col = { .value = color };
    red   = RGBRED(col);
    red   = red + (((31 - red) * param) >> 8);
    green = RGBGREEN(col);
    green = green + (((63 - green) * param) >> 8);
    blue  = RGBBLUE(col);
    blue  = blue + (((31 - blue) * param) >> 8);
    return ggl_solid_pattern(PACK_RGB16(red, green, blue)).bits;
#endif /* TARGET_PRIME1 */
}
