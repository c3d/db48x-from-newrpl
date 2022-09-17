/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

#ifdef TARGET_PRIME1
// VERSION DRAWS HORIZONTAL LINES IN A COLOR BITMAP
// Supports 16-bit per pixel RGB555
// Color is a single color, no pattern supported

#endif /* TARGET_PRIME1 */
void ggl_hline(gglsurface *srf, int y, int xl, int xr, pattern_t color)
{
    // PAINTS A HORIZONTAL LINE FROM xl TO xr BOTH INCLUSIVE
    // color=COLORED PATTERN TO USE, 8 PIXELS - 1 NIBBLE PER PIXEL
    //         PATTERN IS ALWAYS WORD ALIGNED

    // RESTRICTIONS: xr>=xl
    //                 y MUST BE VALID

    pixword       cword = color.bits;
#ifndef TARGET_PRIME1
    int           loff  = (y * srf->width + xl);
    int           roff  = (y * srf->width + xr);
    register int *left  = (int *) srf->pixels + (loff >> 3);
    register int *right = (int *) srf->pixels + (roff >> 3);
    int           ml = ggl_leftmask(loff), mr = ggl_rightmask(roff);

    if (left == right)
    {
        // single word operation
        ml |= mr;
        *left = (*left & ml) | (cword & (~ml));
        return;
    }

    *left = (*left & ml) | (cword & (~ml));
    ++left;
    while (left != right)
    {
        *left = cword;
        ++left;
    }

    *right = (*right & mr) | (cword & (~mr));

#else /* TARGET_PRIME1 */

    unsigned short int *ptr = (unsigned short int *) srf->pixels + y * srf->width + xl;
    while (xl <= xr)
    {
        *ptr++ = cword;
        ++xl;
    }

#endif /* TARGET_PRIME1 */
}

void ggl_cliphline(gglsurface *srf, int y, int xl, int xr, pattern_t color)
{
    // PAINTS A HORIZONTAL LINE FROM xl TO xr BOTH INCLUSIVE
    // color=COLORED PATTERN TO USE, 8 PIXELS - 1 NIBBLE PER PIXEL
    //         PATTERN IS ALWAYS WORD ALIGNED

    // RESTRICTIONS: xr>=xl
    //                 y MUST BE VALID

    if (y < srf->top)
        return;
    if (y > srf->bottom)
        return;
    if (xr < srf->left)
        return;
    if (xl > srf->right)
        return;

    if (xl < srf->left)
        xl = srf->left;
    if (xr > srf->right)
        xr = srf->right;

#ifndef TARGET_PRIME1
    int           loff  = (y * srf->width + xl);
    int           roff  = (y * srf->width + xr);
    register int *left  = (int *) srf->pixels + (loff >> 3);
    register int *right = (int *) srf->pixels + (roff >> 3);
    int           ml = ggl_leftmask(loff), mr = ggl_rightmask(roff);
    pixword       cword = color.bits;

    if (left == right)
    {
        // single word operation
        ml |= mr;
        *left = (*left & ml) | (cword & (~ml));
        return;
    }

    *left = (*left & ml) | (cword & (~ml));
    ++left;
    while (left != right)
    {
        *left = cword;
        ++left;
    }

    *right = (*right & mr) | (cword & (~mr));
#else  /* TARGET_PRIME1 */
    ggl_hline(srf, y, xl, xr, color);
#endif /* TARGET_PRIME1 */
}
