/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"
#ifndef CONFIG_NO_FSYSTEM
#include <time.h>

void FSGetDateTime(unsigned int *datetime, unsigned int *hundredths)
{
   time_t ttime = time(NULL);
   struct tm dt = *localtime(&ttime);



    dt.tm_year -= 1980;    // FIX FROM 1980 TO 2079
    *datetime =
            (dt.tm_sec >> 1) | (dt.tm_min << 5) | (dt.tm_hour << 11) | (dt.tm_mday << 16) | (dt.tm_mon << 21) | (dt.tm_year << 25);
    *hundredths = (dt.tm_sec & 1) ? 100 : 0;
    return;
}
#endif
