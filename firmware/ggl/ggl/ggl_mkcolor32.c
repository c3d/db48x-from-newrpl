/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

void ggl_mkcolor32(int col, int *pattern)
{
    // RETURNS A DITHERED PATTERN FOR 32 COLORS
    // color= from 0 to 30 --> 30 and 31 = both black
    int color = col >> 1, mask = (col & 1) ? 0x01010101 : 0;
    int c;
    color &= 0xf;
    color |= color << 4;
    color |= color << 8;
    color |= color << 16;

    for(c = 0; c < 8; ++c) {
        pattern[c] = color + ((c & 1) ? (mask << 4) : mask);
    }
}
