/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

void ggl_initscr(gglsurface *srf)
{
srf->addr=(int *)MEM_PHYS_SCREEN;
srf->width=LCD_W;
srf->x=srf->y=0;
srf->clipx=srf->clipy=0;
srf->clipx2=SCREEN_WIDTH-1;
srf->clipy2=SCREEN_HEIGHT-1;
}
