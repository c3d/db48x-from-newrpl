/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

unsigned int ggl_optransp(unsigned int dest,unsigned int src, int weight)
{
    // APPLY 100% TRANSPARENCY MASK
    // tcol = TRANSPARENT COLOR IN src
    register int f;
    register unsigned int res =0;
    for(f=0;f<8;++f,src>>=4,dest>>=4)
    {

        res|=((src&0xf)*(16-weight)+(dest&0xf)*weight)>>4;
        res= (res>>4) | (res<<28);
        
    }
    return res;
}


