/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */
#include <ggl.h>

#ifndef TARGET_PRIME1
// param = 0xMMMMNNNN where M is the color to replace, N is the color to replace
// with. For compatibility with color graphics, takes the upper 4 bits of the
// 5-bit blue component of an RGB16 color
#else  /* TARGET_PRIME1 */
// param = 0xMMMMNNNN where MMMMM is the RGB16 color to replace, NNNN is the
// RGB16 color to replace with.
#endif /* TARGET_PRIME1 */
pixword ggl_fltreplace(pixword color, pixword param)
{
#ifndef TARGET_PRIME1
    pixword res = 0;
    for (int f = 0; f < 8; ++f, color >>= 4)
    {
        // filter the pixel here
        if (((color & 0xf) == ((param >> 17) & 0xf)))
            res |= ((param >> 1) & 0xf);
        else
            res |= (color & 0xf);

        res = (res >> 4) | (res << 28);
    }
    return res;
#else  /* TARGET_PRIME1 */
    if (color == ((param >> 16) & 0xffff))
        color = param  & 0xffff;
    return color;
#endif /* TARGET_PRIME1 */
}
