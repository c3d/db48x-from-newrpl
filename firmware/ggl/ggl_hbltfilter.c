/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

void ggl_hbltfilter(int *dest, int destoff, int npixels, int param, gglfilter filterfunc)
{
#ifndef TARGET_PRIME1
    // APPLIES A UNARY OPERATOR (A FILTER) TO A SERIES OF NIBBLES
    // THE OPERATOR SHOULD ACT WORD BY WORD
    // dest IS A WORD ALIGNED ADDRESSES
    // destoff OFFSET IN NIBBLES (PIXELS) FROM dest
    // param IS AN ARBITRARY PARAMETER PASSED TO THE FILTER FUNCTION
    // filterfunc IS THE CUSTOM FILTER FUNCTION
#else  /* TARGET_PRIME1 */
    // COPIES npixels NIBBLES FROM src TO dest
    // dest AND src ARE WORD ALIGNED ADDRESSES
    // destoff AND srcoff ARE OFFSETS IN NIBBLES (PIXELS) FROM dest AND src
    unsigned short int *pdest = (unsigned short int *) dest + destoff;
#endif /* TARGET_PRIME1 */

#ifndef TARGET_PRIME1
    int *start, *end;

    // MASK AND UPDATE

    int  ml = ggl_leftmask(destoff), mr = ggl_rightmask(destoff + npixels - 1);
    start = dest + (destoff >> 3);
    end   = dest + ((destoff + npixels - 1) >> 3);

    if (start == end)
    {
        // single word operation
        ml |= mr;
        *start = (*start & ml) | (((*filterfunc)(*start, param)) & (~ml));
        return;
    }

    *start = (*start & ml) | (((*filterfunc)(*start, param)) & (~ml));
    ++start;

    while (start != end)
#else  /* TARGET_PRIME1 */
    while (npixels--)
#endif /* TARGET_PRIME1 */
    {
#ifndef TARGET_PRIME1
        *start = (*filterfunc)(*start, param);
        ++start;
#else  /* TARGET_PRIME1 */
        *pdest = (*filterfunc)(*pdest, param);
        ++pdest;
#endif /* TARGET_PRIME1 */
    }
#ifndef TARGET_PRIME1

    *start = (*start & mr) | (((*filterfunc)(*start, param)) & (~mr));
#endif /* ! TARGET_PRIME1 */
}
