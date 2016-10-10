/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <newrpl.h>
#include <ui.h>

#include <time.h>

int rtc_getday()
{
    time_t now;
    struct tm *timest;

    time(&now);
    timest=localtime(&now);

    if(timest==NULL) return 0;
    else return timest->tm_mday;

}

int rtc_getmon()
{
    time_t now;
    struct tm *timest;

    time(&now);
    timest=localtime(&now);

    if(timest==NULL) return 0;
    else return timest->tm_mon+1;

}

int rtc_getyear()
{
    time_t now;
    struct tm *timest;

    time(&now);
    timest=localtime(&now);

    if(timest==NULL) return 0;
    else return timest->tm_year+1900;

}

int rtc_getdow()
{
    time_t now;
    struct tm *timest;

    time(&now);
    timest=localtime(&now);

    if(timest==NULL) return 0;
    else return (timest->tm_wday==0)? 7:timest->tm_wday;

}


int rtc_getsec()
{
    time_t now;
    struct tm *timest;

    time(&now);
    timest=localtime(&now);

    if(timest==NULL) return 0;
    else return timest->tm_sec;

}
int rtc_getmin()
{
    time_t now;
    struct tm *timest;

    time(&now);
    timest=localtime(&now);

    if(timest==NULL) return 0;
    else return timest->tm_min;

}
int rtc_gethour()
{
    time_t now;
    struct tm *timest;

    time(&now);
    timest=localtime(&now);

    if(timest==NULL) return 0;
    else return timest->tm_hour;

}

int rtc_setdate(int day,int month,int year)
{
    UNUSED_ARGUMENT(day);
    UNUSED_ARGUMENT(month);
    UNUSED_ARGUMENT(year);
//  DO NOTHING, DON'T ALLOW TO CHANGE THE RTC ON A PC
    return 0;
}

int rtc_settime(int hour,int min,int sec)
{
    UNUSED_ARGUMENT(hour);
    UNUSED_ARGUMENT(min);
    UNUSED_ARGUMENT(sec);
    //  DO NOTHING, DON'T ALLOW TO CHANGE THE RTC ON A PC
    return 0;
}
