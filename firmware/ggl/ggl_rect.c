/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ggl.h>

void ggl_rect(gglsurface *srf, int x1, int y1, int x2, int y2, pattern_t color)
{
    ggl_blit(srf, srf, x1, x2, y1, y2, 0, 0, ggl_op_set, color, CLIP_NONE);
}

void ggl_cliprect(gglsurface *srf, int x1, int y1, int x2, int y2, pattern_t c)
{
    ggl_blit(srf, srf, x1, x2, y1, y2, 0, 0, ggl_op_set, c, CLIP_DST);
}
