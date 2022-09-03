/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

int ggl_mkcolor(int color)
{
    // RETURNS A SOLID PATTERN WITH ALL 8 PIXELS SET TO color
    color &= 0xf;
    color |= color << 4;
    color |= color << 8;
    color |= color << 16;

    return color;
}
