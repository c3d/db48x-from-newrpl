/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"

int __getRTCvalue(int offset)
{
unsigned char a=*((unsigned char *)RTC_REGS+offset);
return ((a&0xf0)>>1) + ((a&0xf0)>>3) + (a&0xf);
}

#define __getRTCDay() __getRTCvalue(0x7c)
#define __getRTCMon() __getRTCvalue(0x84)
#define __getRTCYear() __getRTCvalue(0x88)
#define __getRTCSec() __getRTCvalue(0x70)
#define __getRTCMin() __getRTCvalue(0x74)
#define __getRTCHour() __getRTCvalue(0x78)


void FSGetDateTime(unsigned int *datetime,unsigned int *hundredths)
{
int min,sec,hour,day,mon,year,flag=0;

do {
year=__getRTCYear();
mon=__getRTCMon();
day=__getRTCDay();
hour=__getRTCHour();
min=__getRTCMin();
sec=__getRTCSec();
if(!sec) flag=~flag;
} while(flag);
year+=20;
if(year>=100) year-=100;		// FIX FROM 1980 TO 2079
*datetime=(sec>>1)|(min<<5)|(hour<<11)|(day<<16)|(mon<<21)|(year<<25);
*hundredths=(sec&1)? 100:0;
return;
}
