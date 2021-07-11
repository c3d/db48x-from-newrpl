/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */
#include <xgl.h>

// param = 0xMMMMNNNN where MMMMM is the RGB16 color to replace, NNNN is the RGB16 color to replace with.
unsigned cgl_fltreplace(unsigned word, int param)
{
    if(word==((param>>16)&0xffff))  return param&0xffff;
     return word;
}
