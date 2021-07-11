/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <xgl.h>

unsigned int cgl_opmask(unsigned int dest, unsigned int src, unsigned int tcol)
{
    // APPLY 100% TRANSPARENCY MASK
    // tcol = TRANSPARENT COLOR IN src
if(src==tcol) return dest;
return src;
}

// COPY src INTO dest, WITH tcol=TRANSPARENT COLOR IN src, AND THEN REPLACE BLACK COLOR IN src
// WITH THE COLOR GIVEN IN newcolor

unsigned int cgl_opmaskcol(unsigned int dest, unsigned int src, unsigned int tcol,
        unsigned int newcolor)
{
    // APPLY 100% TRANSPARENCY MASK
    // tcol = TRANSPARENT COLOR IN src
    // WHITE IN src replaced with newcolor (USEFUL IN FONTS)
    if(src==tcol) return dest;
    if(src==RGB_TO_RGB16(255,255,255)) return newcolor;
    return src;

}
