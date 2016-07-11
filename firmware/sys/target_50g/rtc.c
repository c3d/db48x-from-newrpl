/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <newrpl.h>
#include <ui.h>


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


int rtc_getday()
{
    int bcd=__getRTCDay();
}

int rtc_getmon()
{
    return __getRTCMon();
}

int rtc_getyear()
{
    return __getRTCYear()+2000;
}

int rtc_getsec()
{
    return __getRTCSec();
}
int rtc_getmin()
{
    return __getRTCMin();
}
int rtc_gethour()
{
    return __getRTCHour();
}

void rtc_setdate(int day,int month,int year)
{

}

void rtc_settime(int hour,int min,int sec)
{

}
