/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */
#include <cgl.h>

// param = 0xMN where M is the color to replace, N is the color to replace with.
unsigned ggl_fltreplace(unsigned word, int param)
{
    register int f;
    register unsigned int res = 0;
    for(f = 0; f < 8; ++f, word >>= 4) {
        // filter the pixel here
        if(((word & 0xf) == ((param >> 4) & 0xf)))
            res |= (param & 0xf);
        else
            res |= (word & 0xf);

        res = (res >> 4) | (res << 28);

    }
    return res;
}
