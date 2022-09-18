/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

pixword ggl_optransp(pixword dst, pixword src, pixword arg)
{
    // Weighted proportionality, where 'arg' is the weight.
    // Brighter colors in 'arg' make the source more transparent.
    pixword result = 0;
    pixword max = (1U << BITS_PER_PIXEL) - 1;
    for (unsigned shift = 0; shift < BITS_PER_WORD; shift += BITS_PER_PIXEL)
    {
        // In all cases, a N x N multiplication uses 2N bits, so when
        // we shift by BITS_PER_PIXEL, we get back the original resolution.
        pixword accum = (src * (max - arg) + dst * arg) >> BITS_PER_PIXEL;
        result |= accum << shift;
    }
    return result;
}
