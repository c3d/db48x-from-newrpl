/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <ggl.h>

void ggl_hbltfilter(int *dest,int destoff, int npixels, int param,gglfilter filterfunc)

{

// APPLIES A UNARY OPERATOR (A FILTER) TO A SERIES OF NIBBLES
// THE OPERATOR SHOULD ACT WORD BY WORD
// dest IS A WORD ALIGNED ADDRESSES
// destoff OFFSET IN NIBBLES (PIXELS) FROM dest 
// param IS AN ARBITRARY PARAMETER PASSED TO THE FILTER FUNCTION
// filterfunc IS THE CUSTOM FILTER FUNCTION

    int *start,*end;

    // MASK AND UPDATE

    int ml=ggl_leftmask(destoff),mr=ggl_rightmask(destoff+npixels-1);
    start=dest+(destoff>>3);
    end=dest+((destoff+npixels-1)>>3);

    if(start==end) {
        // single word operation
        ml|=mr;
        *start= (*start & ml) | ( ((*filterfunc)(*start,param)) & (~ml));
        return;
    }

    *start= (*start & ml) | ( ((*filterfunc)(*start,param)) & (~ml));
    ++start;

    while(start!=end)
    {
        *start=(*filterfunc)(*start,param);
        ++start;
    }

    *start=  (*start & mr) | (((*filterfunc)(*start,param)) & (~mr));

}

