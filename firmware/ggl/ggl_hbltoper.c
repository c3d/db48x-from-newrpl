/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

void ggl_hbltoper(pixword *dest, int destoff, pixword *src, int srcoff, size npixels, int param, ggloperator_fn foperator)
{
#ifndef TARGET_PRIME1
    // SAME AS hblt BUT THIS ONE APPLIES AN ARBITRARY OPERATOR BETWEEN 2 SURFACES
#endif /* ! TARGET_PRIME1 */
    // COPIES npixels NIBBLES FROM src TO dest
    // dest AND src ARE WORD ALIGNED ADDRESSES
    // destoff AND srcoff ARE OFFSETS IN NIBBLES (PIXELS) FROM dest AND src
#ifndef TARGET_PRIME1
    // tcolor INDICATES WHICH OF THE 16 COLORS IS TRANSPARENT
    // RESTRICTIONS:
    //    npixels<=512 (NO CHECKS MADE HERE)

    while (npixels > 504)
#else  /* TARGET_PRIME1 */
    if (src > dest)
#endif /* TARGET_PRIME1 */
    {
#ifndef TARGET_PRIME1
        ggl_hbltoper(dest, destoff, src, srcoff, 504, param, foperator);
        npixels -= 504;
        destoff += 504;
        srcoff += 504;
    }

    pixword tempmem[HBLT_BUFFER];

    // CALCULATE ROTATION
    int      rot = ((srcoff & 7) - (destoff & 7)) * 4;
    unsigned a, b = 0;
    pixword  *ptr = tempmem, *start = src + (srcoff >> 3), *end = src + ((srcoff + npixels - 1) >> 3);
#else  /* TARGET_PRIME1 */
        unsigned short int *pdest = (unsigned short int *) dest + destoff;
        unsigned short int *psrc  = (unsigned short int *) src + srcoff;
#endif /* TARGET_PRIME1 */

#ifndef TARGET_PRIME1
    // FIRST STAGE: COPY AND ROTATION

    ++end;
    if (rot < 0)
    {
        // ROTATION RIGHT
        rot = -rot;
        while (start <= end)
        {
            a    = *start;
            *ptr = (a << rot) | (b >> (32 - rot));
            b    = a;
            ++ptr;
            ++start;
        }
    }
    else if (rot > 0)
    {
        // ROTATION LEFT
        b = *start;
        ++start;
        while (start <= end)
#else  /* TARGET_PRIME1 */
        while (npixels--)
#endif /* TARGET_PRIME1 */
        {
#ifndef TARGET_PRIME1
            a    = *start;
            *ptr = (b >> rot) | (a << (32 - rot));
            b    = a;
            ++ptr;
            ++start;
#else  /* TARGET_PRIME1 */
            *pdest = (*foperator)(*pdest, *psrc++, param);
            ++pdest;
#endif /* TARGET_PRIME1 */
        }
    }
    else
    {
#ifndef TARGET_PRIME1
        // ROTATION IS ZERO, JUST COPY
        while (start <= end)
#else  /* TARGET_PRIME1 */
        unsigned short int *pdest = (unsigned short int *) dest + destoff + npixels - 1;
        unsigned short int *psrc  = (unsigned short int *) src + srcoff + npixels - 1;

        while (npixels--)
#endif /* TARGET_PRIME1 */
        {
#ifndef TARGET_PRIME1
            *ptr = *start;
            ++ptr;
            ++start;
#else  /* TARGET_PRIME1 */
            *pdest = (*foperator)(*pdest, *psrc--, param);
            --pdest;
#endif /* TARGET_PRIME1 */
        }
    }
#ifndef TARGET_PRIME1

    // SECOND STAGE: MASK AND UPDATE

    int ml = ggl_leftmask(destoff), mr = ggl_rightmask(destoff + npixels - 1);
    ptr   = tempmem;
    start = dest + (destoff >> 3);
    end   = dest + ((destoff + npixels - 1) >> 3);

    if (start == end)
    {
        // single word operation
        ml |= mr;
        *start = (*start & ml) | (((*foperator)(*start, *ptr, param)) & (~ml));
        return;
    }

    *start = (*start & ml) | (((*foperator)(*start, *ptr, param)) & (~ml));
    ++start;
    ++ptr;

    while (start != end)
    {
        *start = (*foperator)(*start, *ptr, param);
        ++start;
        ++ptr;
    }

    *start = (*start & mr) | (((*foperator)(*start, *ptr, param)) & (~mr));
#endif /* ! TARGET_PRIME1 */
}

#ifndef TARGET_PRIME1
// TABLE TO EXPAND MONOCHROME BITMAP TO 16-GRAYS

const const unsigned int ggl_mono2gray[256] = {
    0x0,        0xF,        0xF0,       0xFF,       0xF00,      0xF0F,      0xFF0,      0xFFF,      0xF000,
    0xF00F,     0xF0F0,     0xF0FF,     0xFF00,     0xFF0F,     0xFFF0,     0xFFFF,     0xF0000,    0xF000F,
    0xF00F0,    0xF00FF,    0xF0F00,    0xF0F0F,    0xF0FF0,    0xF0FFF,    0xFF000,    0xFF00F,    0xFF0F0,
    0xFF0FF,    0xFFF00,    0xFFF0F,    0xFFFF0,    0xFFFFF,    0xF00000,   0xF0000F,   0xF000F0,   0xF000FF,
    0xF00F00,   0xF00F0F,   0xF00FF0,   0xF00FFF,   0xF0F000,   0xF0F00F,   0xF0F0F0,   0xF0F0FF,   0xF0FF00,
    0xF0FF0F,   0xF0FFF0,   0xF0FFFF,   0xFF0000,   0xFF000F,   0xFF00F0,   0xFF00FF,   0xFF0F00,   0xFF0F0F,
    0xFF0FF0,   0xFF0FFF,   0xFFF000,   0xFFF00F,   0xFFF0F0,   0xFFF0FF,   0xFFFF00,   0xFFFF0F,   0xFFFFF0,
    0xFFFFFF,   0xF000000,  0xF00000F,  0xF0000F0,  0xF0000FF,  0xF000F00,  0xF000F0F,  0xF000FF0,  0xF000FFF,
    0xF00F000,  0xF00F00F,  0xF00F0F0,  0xF00F0FF,  0xF00FF00,  0xF00FF0F,  0xF00FFF0,  0xF00FFFF,  0xF0F0000,
    0xF0F000F,  0xF0F00F0,  0xF0F00FF,  0xF0F0F00,  0xF0F0F0F,  0xF0F0FF0,  0xF0F0FFF,  0xF0FF000,  0xF0FF00F,
    0xF0FF0F0,  0xF0FF0FF,  0xF0FFF00,  0xF0FFF0F,  0xF0FFFF0,  0xF0FFFFF,  0xFF00000,  0xFF0000F,  0xFF000F0,
    0xFF000FF,  0xFF00F00,  0xFF00F0F,  0xFF00FF0,  0xFF00FFF,  0xFF0F000,  0xFF0F00F,  0xFF0F0F0,  0xFF0F0FF,
    0xFF0FF00,  0xFF0FF0F,  0xFF0FFF0,  0xFF0FFFF,  0xFFF0000,  0xFFF000F,  0xFFF00F0,  0xFFF00FF,  0xFFF0F00,
    0xFFF0F0F,  0xFFF0FF0,  0xFFF0FFF,  0xFFFF000,  0xFFFF00F,  0xFFFF0F0,  0xFFFF0FF,  0xFFFFF00,  0xFFFFF0F,
    0xFFFFFF0,  0xFFFFFFF,  0xF0000000, 0xF000000F, 0xF00000F0, 0xF00000FF, 0xF0000F00, 0xF0000F0F, 0xF0000FF0,
    0xF0000FFF, 0xF000F000, 0xF000F00F, 0xF000F0F0, 0xF000F0FF, 0xF000FF00, 0xF000FF0F, 0xF000FFF0, 0xF000FFFF,
    0xF00F0000, 0xF00F000F, 0xF00F00F0, 0xF00F00FF, 0xF00F0F00, 0xF00F0F0F, 0xF00F0FF0, 0xF00F0FFF, 0xF00FF000,
    0xF00FF00F, 0xF00FF0F0, 0xF00FF0FF, 0xF00FFF00, 0xF00FFF0F, 0xF00FFFF0, 0xF00FFFFF, 0xF0F00000, 0xF0F0000F,
    0xF0F000F0, 0xF0F000FF, 0xF0F00F00, 0xF0F00F0F, 0xF0F00FF0, 0xF0F00FFF, 0xF0F0F000, 0xF0F0F00F, 0xF0F0F0F0,
    0xF0F0F0FF, 0xF0F0FF00, 0xF0F0FF0F, 0xF0F0FFF0, 0xF0F0FFFF, 0xF0FF0000, 0xF0FF000F, 0xF0FF00F0, 0xF0FF00FF,
    0xF0FF0F00, 0xF0FF0F0F, 0xF0FF0FF0, 0xF0FF0FFF, 0xF0FFF000, 0xF0FFF00F, 0xF0FFF0F0, 0xF0FFF0FF, 0xF0FFFF00,
    0xF0FFFF0F, 0xF0FFFFF0, 0xF0FFFFFF, 0xFF000000, 0xFF00000F, 0xFF0000F0, 0xFF0000FF, 0xFF000F00, 0xFF000F0F,
    0xFF000FF0, 0xFF000FFF, 0xFF00F000, 0xFF00F00F, 0xFF00F0F0, 0xFF00F0FF, 0xFF00FF00, 0xFF00FF0F, 0xFF00FFF0,
    0xFF00FFFF, 0xFF0F0000, 0xFF0F000F, 0xFF0F00F0, 0xFF0F00FF, 0xFF0F0F00, 0xFF0F0F0F, 0xFF0F0FF0, 0xFF0F0FFF,
    0xFF0FF000, 0xFF0FF00F, 0xFF0FF0F0, 0xFF0FF0FF, 0xFF0FFF00, 0xFF0FFF0F, 0xFF0FFFF0, 0xFF0FFFFF, 0xFFF00000,
    0xFFF0000F, 0xFFF000F0, 0xFFF000FF, 0xFFF00F00, 0xFFF00F0F, 0xFFF00FF0, 0xFFF00FFF, 0xFFF0F000, 0xFFF0F00F,
    0xFFF0F0F0, 0xFFF0F0FF, 0xFFF0FF00, 0xFFF0FF0F, 0xFFF0FFF0, 0xFFF0FFFF, 0xFFFF0000, 0xFFFF000F, 0xFFFF00F0,
    0xFFFF00FF, 0xFFFF0F00, 0xFFFF0F0F, 0xFFFF0FF0, 0xFFFF0FFF, 0xFFFFF000, 0xFFFFF00F, 0xFFFFF0F0, 0xFFFFF0FF,
    0xFFFFFF00, 0xFFFFFF0F, 0xFFFFFFF0, 0xFFFFFFFF
};

#endif /* ! TARGET_PRIME1 */
void ggl_monohbltoper(pixword       *dest,
                      int            destoff,
                      unsigned char *src,
                      int            srcoff,
                      int            npixels,
                      int            param,
                      ggloperator_fn foperator)
{
#ifndef TARGET_PRIME1
    // SAME AS hbltoper BUT SOURCE BITMAP IS MONOCHROME, AND CONVERTED TO 16-GRAYS
    // ON-THE-FLY COPIES npixels NIBBLES FROM src TO dest dest IS WORD ALIGNED
    // ADDRESSES, src IS BYTE ALIGNED destoff AND srcoff ARE OFFSETS IN NIBBLES
    // (PIXELS) FROM dest AND src tcolor INDICATES WHICH OF THE 16 COLORS IS
    // TRANSPARENT RESTRICTIONS:
    //    npixels<=512 (NO CHECKS MADE HERE)

    while (npixels > 504)
    {
        ggl_monohbltoper(dest, destoff, src, srcoff, 504, param, foperator);
        npixels -= 504;
        destoff += 504;
        srcoff += 504;
    }

    pixword tempmem[HBLT_BUFFER];

    // CALCULATE ROTATION
    int            rot = ((srcoff & 7) - (destoff & 7)) * 4;
    unsigned       a, b = 0;
    pixword       *ptr   = tempmem, *dstart, *dend;
    unsigned char *start = src + (srcoff >> 3), *end = src + ((srcoff + npixels - 1) >> 3);

    // FIRST STAGE: COPY AND ROTATION

    ++end;
    if (rot < 0)
    {
        // ROTATION RIGHT
        rot = -rot;
        while (start <= end)
        {
            a    = ggl_mono2gray[*start];
            *ptr = (a << rot) | (b >> (32 - rot));
            b    = a;
            ++ptr;
            ++start;
        }
    }
    else if (rot > 0)
    {
        // ROTATION LEFT
        b = ggl_mono2gray[*start];
        ++start;
        while (start <= end)
        {
            a    = ggl_mono2gray[*start];
            *ptr = (b >> rot) | (a << (32 - rot));
            b    = a;
            ++ptr;
            ++start;
        }
    }
    else
    {
        // ROTATION IS ZERO, JUST COPY
        while (start <= end)
        {
            *ptr = ggl_mono2gray[*start];
            ++ptr;
            ++start;
        }
    }

    // SECOND STAGE: MASK AND UPDATE

    int ml = ggl_leftmask(destoff), mr = ggl_rightmask(destoff + npixels - 1);
    ptr    = tempmem;
    dstart = dest + (destoff >> 3);
    dend   = dest + ((destoff + npixels - 1) >> 3);

    if (dstart == dend)
    {
        // single word operation
        ml |= mr;
        *dstart = (*dstart & ml) | (((*foperator)(*dstart, *ptr, param)) & (~ml));
        return;
    }

    *dstart = (*dstart & ml) | (((*foperator)(*dstart, *ptr, param)) & (~ml));
    ++dstart;
    ++ptr;
#else  /* TARGET_PRIME1 */
    // COPIES npixels NIBBLES FROM src TO dest
    // dest AND src ARE WORD ALIGNED ADDRESSES
    // destoff AND srcoff ARE OFFSETS IN NIBBLES (PIXELS) FROM dest AND src
#endif /* TARGET_PRIME1 */

#ifndef TARGET_PRIME1
    while (dstart != dend)
    {
        *dstart = (*foperator)(*dstart, *ptr, param);
        ++dstart;
        ++ptr;
#else  /* TARGET_PRIME1 */
    unsigned short int *pdest = (unsigned short int *) dest + destoff;
    unsigned char      *psrc  = (unsigned char *) src;
    unsigned char       mask  = 1 << (srcoff & 7);
    while (npixels--)
    {
        *pdest = (*foperator)(*pdest, ((psrc[srcoff >> 3] & mask) ? RGB_TO_RGB16(255, 255, 255).value : 0), param);
        ++pdest;
        ++srcoff;
        if (mask & 0x80)
            mask = 1;
        else
            mask <<= 1;
#endif /* TARGET_PRIME1 */
    }
#ifndef TARGET_PRIME1

    *dstart = (*dstart & mr) | (((*foperator)(*dstart, *ptr, param)) & (~mr));
#endif /* ! TARGET_PRIME1 */
}
