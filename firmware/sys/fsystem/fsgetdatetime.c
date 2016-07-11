/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"



void FSGetDateTime(unsigned int *datetime,unsigned int *hundredths)
{
int min,sec,hour,day,mon,year,flag=0;

do {
year=rtc_getyear();
mon=rtc_getmon();
day=rtc_getday();
hour=rtc_gethour();
min=rtc_getmin();
sec=rtc_getsec();
if(!sec) flag=~flag;
} while(flag);
year-=1980;		// FIX FROM 1980 TO 2079
*datetime=(sec>>1)|(min<<5)|(hour<<11)|(day<<16)|(mon<<21)|(year<<25);
*hundredths=(sec&1)? 100:0;
return;
}
