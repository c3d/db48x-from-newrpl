/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

#ifndef TARGET_PRIME1
color4_t ggl_getnib(pixword *buf, size addr)
{
    color4_t *ptr = ((color4_t *) buf) + (addr >> 1);
    uint8_t value = (addr & 1) ? ptr->nibbles.high : ptr->nibbles.low;
    color4_t result = { .value =  value };
    return result;
}
#endif // TARGET_PRIME1

int ggl_getmonopix(char *buf, int addr)
{
    register char *ptr = ((char *) buf) + (addr >> 3);
    return (*ptr & (1 << (addr & 7))) ? 1 : 0;
}
