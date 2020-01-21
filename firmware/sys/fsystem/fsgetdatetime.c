/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"
#ifndef CONFIG_NO_FSYSTEM

void FSGetDateTime(unsigned int *datetime, unsigned int *hundredths)
{
    struct date dt;
    struct time tm;

    rtc_getdatetime(&dt, &tm);
    dt.year -= 1980;    // FIX FROM 1980 TO 2079
    *datetime =
            (tm.sec >> 1) | (tm.min << 5) | (tm.hour << 11) | (dt.
            mday << 16) | (dt.mon << 21) | (dt.year << 25);
    *hundredths = (tm.sec & 1) ? 100 : 0;
    return;
}
#endif
