/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>



unsigned int ggl_fltlighten(unsigned word,int param)
{
    register int f;
    register unsigned int res=0;
    for(f=0;f<8;++f,word>>=4)
    {
        // filter the pixel here
        if((word&0xf)>(unsigned)param) res|=(word&0xf)-param;

        res= (res>>4) | (res<<28);
    }
    return res;
}
