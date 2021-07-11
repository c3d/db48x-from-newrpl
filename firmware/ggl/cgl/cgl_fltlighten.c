/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <xgl.h>

unsigned int cgl_fltlighten(unsigned word, int param)
{
int red,green,blue;
red=RGBRED(word);
red= red + (((31-red) * param)>>8);
green=RGBGREEN(word);
green= green + (((63-green) * param)>>8);
blue=RGBBLUE(word);
blue= blue + (((31-blue) * param)>>8);
return PACK_RGB16(red,green,blue);
}
