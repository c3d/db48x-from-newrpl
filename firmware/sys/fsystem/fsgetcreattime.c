/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM


void FSGetCreatTime(FS_FILE *file,struct compact_tm *dt)
{
if(file->CreatTimeDate) {
dt->tm_sec=((file->CreatTimeDate&31)<<1) + file->CrtTmTenth/100;
dt->tm_min=(file->CreatTimeDate>>5)&63;
dt->tm_hour=(file->CreatTimeDate>>11)&31;
dt->tm_mday=(file->CreatTimeDate>>16)&31;
dt->tm_mon=((file->CreatTimeDate>>21)&15) -1;		// month from 0 to 11
dt->tm_year=(file->CreatTimeDate>>25)+80;	// COUNT YEARS FROM 1900 INSTEAD OF 1980
}
else {
    dt->tm_sec=0;
    dt->tm_min=0;
    dt->tm_hour=0;
    dt->tm_mday=0;
    dt->tm_mon=0;
    dt->tm_year=80;
}
dt->tm_wday=0;
//dt->tm_yday=0;
dt->tm_isdst=0;

return;
}
#endif
