/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


// CLOCK AND ALARM MANAGEMENT - HIGHER LEVEL API

#include <ui.h>


void halGetSystemDateTime(struct date *dt, struct time *tm)
{
    rtc_getdatetime(dt, tm);

    return;
}

struct date halGetSystemDate()
{
    return rtc_getdate();
}

int halSetSystemDate(struct date dt)
{
    return rtc_setdate(dt);
}

struct time halGetSystemTime()
{
    return rtc_gettime();
}

int halSetSystemTime(struct time tm)
{
    return rtc_settime(tm);
}

void halGetSystemAlarm(struct date *dt, struct time *tm, int *enabled)
{
    rtc_getalarm(dt, tm, enabled);

    return;
}

int halSetSystemAlarm(struct date dt, struct time tm, int enabled)
{
    return rtc_setalarm(dt, tm, enabled);
}

void halAnnounceAlarm()
{/*
    halFlags |= HAL_ALARMEVENT;
    halSetNotification(N_LOWBATTERY,0xf);
*/
    return;
}

void halDoAlarmEvent()
{
    // APPOINTMENT ALARM
    halShowMsg("Alarm Event");

    // CONTROL ALARM


    halFlags &= ~HAL_DOALARM;

    return;
}
