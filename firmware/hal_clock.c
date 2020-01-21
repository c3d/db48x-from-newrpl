/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// HARDWARE CLOCK AND ALARM MANAGEMENT - HIGHER LEVEL API

#include <newrpl.h>
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
    int ret = rtc_setdate(dt);

    rplUpdateAlarms();

    return ret;
}

struct time halGetSystemTime()
{
    return rtc_gettime();
}

int halSetSystemTime(struct time tm)
{
    int ret = rtc_settime(tm);

    rplUpdateAlarms();

    return ret;
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

int halCheckSystemAlarm()
{
    return rtc_chkalrm();
}

void halDisableSystemAlarm()
{
    rtc_setaie(0);

    return;
}
