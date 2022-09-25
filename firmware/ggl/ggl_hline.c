/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

void ggl_hline(gglsurface *srf, int y, int xl, int xr, pattern_t color)
{
    ggl_blit(srf, srf, xl, xr, y, y, 0, 0, ggl_op_set, color, CLIP_NONE);
}

void ggl_cliphline(gglsurface *srf, int y, int xl, int xr, pattern_t color)
{
    ggl_blit(srf, srf, xl, xr, y, y, 0, 0, ggl_op_set, color, CLIP_DST);
}
