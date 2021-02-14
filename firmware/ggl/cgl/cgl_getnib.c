/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <cgl.h>

int ggl_getnib(int *buf, int addr)
{
    register char *ptr = ((char *)buf) + (addr >> 1);
    register int a = (int)(*ptr);
    if(addr & 1)
        a >>= 4;
    return a & 0xf;
}

int ggl_getmonopix(char *buf, int addr)
{
    register char *ptr = ((char *)buf) + (addr >> 3);
    return (*ptr & (1 << (addr & 7))) ? 1 : 0;
}
