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
unsigned char a=*((volatile unsigned char *)RTC_REGS+offset);
return ((a&0xf0)>>1) + ((a&0xf0)>>3) + (a&0xf);
}

void __setRTCvalue(int offset,int value)
{
    int bcd=value/10;
    bcd=(bcd<<4)+(value-bcd*10);

    volatile unsigned char *a=((volatile unsigned char *)RTC_REGS+offset);
    *a=bcd;
}


#define __getRTCDay() __getRTCvalue(0x7c)
#define __getRTCMon() __getRTCvalue(0x84)
#define __getRTCYear() __getRTCvalue(0x88)
#define __getRTCSec() __getRTCvalue(0x70)
#define __getRTCMin() __getRTCvalue(0x74)
#define __getRTCHour() __getRTCvalue(0x78)

#define __getRTCDow() __getRTCvalue(0x80)


#define __setRTCDay(v) __setRTCvalue(0x7c,(v))
#define __setRTCMon(v) __setRTCvalue(0x84,(v))
#define __setRTCYear(v) __setRTCvalue(0x88,(v))
#define __setRTCSec(v) __setRTCvalue(0x70,(v))
#define __setRTCMin(v) __setRTCvalue(0x74,(v))
#define __setRTCHour(v) __setRTCvalue(0x78,(v))

#define __setRTCDow(v) __setRTCvalue(0x80,(v))


int rtc_getday()
{
    return __getRTCDay();
}

int rtc_getmon()
{
    return __getRTCMon();
}

int rtc_getyear()
{
    return __getRTCYear()+2000;
}

int rtc_getdow()
{
    return __getRTCDow();   // 1=MONDAY ... 7 = SUNDAY
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

int rtc_setdate(int day,int month,int year)
{
// VERIFY A VALID DATE IS GIVEN BEFORE SETTING IT
    if(day<1 || day>31) return 0;
    if(month<1 || month>12) return 0;
    if(year<2000 || year>2099) return 0;

    // NOW CHECK FOR PROPER DAY OF THE MONTH

    if( (month==4)||(month==6)||(month==9)||(month==11)) {
     if(day>30) {
         ++month;
         day=1;
     }
    }
    if(month==2) {
    if(!(year&3)) {
        //  LEAP YEAR, INCLUDES YEAR 2000
        if(day>29) {
         ++month;
         day-=29;
        }
    }
    else {
        if(day>28) {
         ++month;
         day-=28;
        }
    }
    }

    int dow=(
                day + (((month>2)? (month-2):(month+10))*26-2)/10
                + (year-2000) + ((year-2000)>>2)
                + (((month>2)? (year-2000):(year-2001))>>2) - (((month>2)? (year-2000):(year-2001))<<1)
                ) % 7;  // DAY OF WEEK, 0 = SUNDAY ... 6=SATURDAY

    if(dow==0) dow=7;   // CONVERT TO 1=MONDAY ... 7=SUNDAY


    __setRTCDay(1); // TO PREVENT ROLLOVER AT MIDNIGHT
    // WE HAVE A VALID DATE
    __setRTCYear(year-2000);
    __setRTCMon(month);
    __setRTCDow(dow);
    __setRTCDay(day);



    return 1;

}

int rtc_settime(int hour,int min,int sec)
{
    // CHECK FOR VALID TIME
    if(hour<0 || hour>23) return 0;
    if(min<0 || min>59) return 0;
    if(sec<0 || sec>59) return 0;

    __setRTCSec(1); // TO PREVENT ROLLOVER
    __setRTCHour(hour);
    __setRTCMin(min);
    __setRTCSec(sec);

    return 1;
}
