/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

void ggl_vline(gglsurface *srf, int x, int yt, int yb, pattern_t colors)
{
    ggl_blit(srf, srf, x, x, yt, yb, 0, 0, ggl_op_set, colors, CLIP_NONE);
}

// SAME AS VLINE BU WITH CLIPPING
void ggl_clipvline(gglsurface *srf, int x, int yt, int yb, pattern_t colors)
{
    ggl_blit(srf, srf, x, x, yt, yb, 0, 0, ggl_op_set, colors, CLIP_DST);
}
