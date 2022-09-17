/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */
#include <ggl.h>

pixword ggl_fltdarken(pixword color, pixword param)
{
    pixword  res = 0;
    for (int f = 0; f < 8; ++f, color >>= 4)
    {
        // filter the pixel here
        if (((color & 0xf) + param) >= 0xf)
            res |= 0xf;
        else
            res |= (color & 0xf) + param;

        res = (res >> 4) | (res << 28);
    }
    return res;
}
