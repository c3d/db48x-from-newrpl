/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

pixword ggl_opmask(pixword dest, pixword src, pixword tcol)
{
    // APPLY 100% TRANSPARENCY MASK
    // tcol = TRANSPARENT COLOR IN src
#ifndef TARGET_PRIME1
    pixword res = 0;
    for (int f = 0; f < 8; ++f, src >>= 4, dest >>= 4)
    {
        if (src == tcol)
            res |= dest & 0xf;
        else
            res |= src & 0xf;
        res = (res >> 4) | (res << 28);
    }
    return res;
#else  /* TARGET_PRIME1 */
    if (src == tcol)
        return dest;
    return src;
#endif /* TARGET_PRIME1 */
}

// COPY src INTO dest, WITH tcol=TRANSPARENT COLOR IN src, AND THEN REPLACE
// BLACK COLOR IN src WITH THE COLOR GIVEN IN newcolor

pixword ggl_opmaskcol(pixword dest, pixword src, pixword tcol, pixword newcolor)
{
    // APPLY 100% TRANSPARENCY MASK
    // tcol = TRANSPARENT COLOR IN src
#ifndef TARGET_PRIME1
    pixword res = 0;
    for (int f = 0; f < 8; ++f, src >>= 4, dest >>= 4)
    {
        if (src == tcol)
            res |= dest & 0xf;
        else
            res |= src & newcolor;
        res = (res >> 4) | (res << 28);
    }
    return res;
#else  /* TARGET_PRIME1 */
    // WHITE IN src replaced with newcolor (USEFUL IN FONTS)
    if (src == tcol)
        return dest;
    color_t white = RGB_TO_RGB16(255, 255, 255);
    if (src == white.value)
        return newcolor;
    return src;
#endif /* TARGET_PRIME1 */
}
