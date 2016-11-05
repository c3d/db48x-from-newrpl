/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <newrpl.h>
#include <ui.h>

#include <time.h>

void rtc_getdatetime(struct date *dt, struct time *tm)
{
    time_t now;
    struct tm *timest;

    time(&now);
    timest=localtime(&now);

    if(timest==NULL) return;

    if(dt) {
        dt->mday=timest->tm_mday;
        dt->mon=timest->tm_mon;
        dt->wday=timest->tm_wday;
        dt->year=timest->tm_year;
    }
    if(tm) {
        tm->hour=timest->tm_hour;
        tm->isdst=timest->tm_isdst;
        tm->min=timest->tm_min;
        tm->sec=timest->tm_sec;
    }


    return;
}

int rtc_setdatetime(struct date dt, struct time tm)
{
    UNUSED_ARGUMENT(dt);
    UNUSED_ARGUMENT(tm);
    //  DO NOTHING, DON'T ALLOW TO CHANGE THE RTC ON A PC
    return 0;

}

struct date rtc_getdate()
{
    struct date dt={0,0,0,0};
    time_t now;
    struct tm *timest;

    time(&now);
    timest=localtime(&now);

    if(timest==NULL) return dt;

        dt.mday=timest->tm_mday;
        dt.mon=timest->tm_mon;
        dt.wday=timest->tm_wday;
        dt.year=timest->tm_year;

        return dt;
}

int rtc_setdate(struct date dt)
{
    UNUSED_ARGUMENT(dt);
    //  DO NOTHING, DON'T ALLOW TO CHANGE THE RTC ON A PC
    return 0;
}

struct time rtc_gettime()
{
    struct time tm={0,0,0,0};
    time_t now;
    struct tm *timest;

    time(&now);
    timest=localtime(&now);

    if(timest==NULL) return tm;

    tm.hour=timest->tm_hour;
    tm.isdst=timest->tm_isdst;
    tm.min=timest->tm_min;
    tm.sec=timest->tm_sec;

    return tm;
}

int rtc_settime(struct time tm)
{
    UNUSED_ARGUMENT(tm);
    //  DO NOTHING, DON'T ALLOW TO CHANGE THE RTC ON A PC
    return 0;
}

void rtc_getalarm(struct date *dt, struct time *tm, int *enabled)
{
    UNUSED_ARGUMENT(dt);
    UNUSED_ARGUMENT(tm);


    if(enabled) *enabled=0;
    return;
}

// SET ALARM INTERRUPT
int rtc_setalarm(struct date dt, struct time tm, int enabled)
{

    UNUSED_ARGUMENT(dt);
    UNUSED_ARGUMENT(tm);
    UNUSED_ARGUMENT(enabled);

    // DON'T ALLOW ALARMS IN THE SIMULATOR
    return 0;
}

void rtc_gettick(int *freq, int *enabled)
{
 UNUSED_ARGUMENT(freq);
 // CAN'T GET RTC TICKS ON THE SIMULATOR
 if(enabled) *enabled=0;
 return;
}

// SET PERIODIC INTERRUPT
int rtc_settick(int freq, int enabled)
{
    UNUSED_ARGUMENT(freq);
    UNUSED_ARGUMENT(enabled);
    // CAN'T SET RTC TICKS ON THE SIMULATOR
    return 0;
}

