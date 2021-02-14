/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <cgl.h>

unsigned int ggl_fltinvert(unsigned word, int param)
{
    (void)param;        // THIS IS JUST TO SILENCE THE UNUSED ARGUMENT WARNING

    register int f;
    register unsigned int res = 0;
    for(f = 0; f < 8; ++f, word >>= 4) {
        // filter the pixel here
        res |= 0xf - (word & 0xf);

        res = (res >> 4) | (res << 28);
    }
    return res;
}
