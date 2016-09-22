/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


// CLOCK AND ALARM MANAGEMENT - HIGHER LEVEL API

#include <ui.h>

void halSetSystemDate(int d,int m,int y)
{
    rtc_setdate(d,m,y);
}

void halGetSystemDate(int *d,int *m,int *y,int *dow)
{
    int _d,_m,_y;

    do {
        _y=rtc_getyear();
        _m=rtc_getmon();
        _d=rtc_getday();
    } while( (_m!=rtc_getmon()) || (_y!=rtc_getyear()) );

    if(d) *d=_d;
    if(m) *m=_m;
    if(y) *y=_y;
    if(dow) *dow=rtc_getdow();


}

// MUST BE IN 24-HR FORMAT
void halSetSystemTime(int hr,int min,int sec)
{
    rtc_settime(hr,min,sec);
}

void halGetSystemTime(int *hr,int *min,int *sec)
{
   int h,m,s;

   // MAKE SURE READINGS ARE CONSISTENT
   do {
   h=rtc_gethour();
   m=rtc_getmin();
   s=rtc_getsec();
   } while( (m!=rtc_getmin()) || (h!=rtc_gethour()));

   if(hr) *hr=h;
   if(min) *min=m;
   if(sec) *sec=s;

}

// TODO: ADD ALARM FUNCTIONS HERE
