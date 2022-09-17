/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

#ifndef TARGET_PRIME1
void ggl_pltnib(pixword *buf, size addr, color4_t color)
{
    color4_t *ptr = ((color4_t *) buf) + (addr >> 1);
    if (addr & 1)
        ptr->nibbles.high = color.value;
    else
        ptr->nibbles.low = color.value;
}
#endif // TARGET_PRIME1
