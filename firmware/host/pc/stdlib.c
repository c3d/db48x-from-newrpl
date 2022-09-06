/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

#define WONT_RETURN     do {} while(1)

void abort()
{
    throw_exception("ABORT CALLED", EX_RESET);
    WONT_RETURN;
}
