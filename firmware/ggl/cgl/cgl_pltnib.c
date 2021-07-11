/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <xgl.h>

void ggl_pltnib(int *buf, int addr, int color)
{
    register char *ptr = ((char *)buf) + (addr >> 1);
    if(addr & 1)
        *ptr = (*ptr & 0xf) | ((color << 4) & 0xf0);
    else
        *ptr = (*ptr & 0xf0) | (color & 0xf);
}
