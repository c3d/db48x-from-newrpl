/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

pixword ggl_optransp(pixword dest, pixword src, pixword weight)
{
    // APPLY 100% TRANSPARENCY MASK
    // tcol = TRANSPARENT COLOR IN src
    pixword res = 0;
    for (int f = 0; f < 8; ++f, src >>= 4, dest >>= 4)
    {
        res |= (src * (16 - weight) + dest * weight) >> 4;
        res = (res >> 4) | (res << 28);
    }
    return res;
}
