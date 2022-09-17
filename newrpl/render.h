/*
 * Copyright (c) 2017, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef RENDER_H
#define RENDER_H

#include "fastmath.h"

typedef struct
{
    FPINT x, y;
    int32_t type;
} CURVEPT;

#define TYPE_STARTPOINT 0
#define TYPE_LINE 1
#define TYPE_CTLPT 2
#define TYPE_CURVE 3
#define TYPE_CLOSEEND 4

#endif // RENDER_H
