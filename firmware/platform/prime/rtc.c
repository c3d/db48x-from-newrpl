/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <sys/rtc.h>

static volatile int __rtc_almint __SYSTEM_GLOBAL__;

// ENSURE RTCCON IS CORRECTLY SET
static void __rtc_check_device()
{
    union rtccon rtc;
    rtc.byte = __getRTCCon();

    if(rtc.clkrst | rtc.clksel | rtc.cntsel) {
        rtc.clkrst = rtc.clksel = rtc.cntsel = 0;
        __setRTCCon(rtc.byte);
    }

    return;
}

// CONTROL WRITE ACCESS TO THE BCD REGISTERS
static void __rtc_setwrite(int enabled)
{
    union rtccon rtc;
    rtc.byte = __getRTCCon();

    if(rtc.rtcen != enabled) {
        rtc.rtcen = enabled ? 1 : 0;
        __setRTCCon(rtc.byte);
    }

    __rtc_check_device();

    return;
}

static int __rtc_is_valid_dt(struct date dt)
{
    int mdays;

    if(dt.year < 2000 || dt.year > 2099)
        return 0;
    if(dt.mon < 1 || dt.mon > 12)
        return 0;
    mdays = month_days[dt.mon - 1] + ((dt.mon == 2) && (!(dt.year & 3)));
    if(dt.mday < 1 || dt.mday > mdays)
        return 0;

    return 1;
}

static int __rtc_is_valid_tm(struct time tm)
{
    if(tm.hour > 23)
        return 0;
    if(tm.min > 59)
        return 0;
    if(tm.sec > 59)
        return 0;

    return 1;
}



struct date rtc_getdate()
{
    struct date dt;
    int have_retried = 0, sec;

    __rtc_check_device();

  retry_get_date:
    dt.mday = __getRTCDay();
    dt.wday = __getRTCDow();    // 1 = MONDAY ... 7 = SUNDAY
    dt.mon = __getRTCMon();
    dt.year = __getRTCYear();
    sec = __getRTCSec();

    // CHECK IF A ROLLOVER MAY HAVE OCCURED
    if((sec == 0) && !have_retried) {
        have_retried = 1;
        goto retry_get_date;
    }

    dt.year += 2000;

    return dt;
}

int rtc_setdate(struct date dt)
{
    int have_retried = 0;

    if(!__rtc_is_valid_dt(dt))
        return 0;

    int dow =
            (dt.mday + (((dt.mon >
                        2) ? (dt.mon - 2) : (dt.mon + 10)) * 26 - 2) / 10 +
            (dt.year - 2000) + ((dt.year - 2000) >> 2)
            + (((dt.mon > 2) ? (dt.year - 2000) : (dt.year - 2001)) >> 2)
            - (((dt.mon > 2) ? (dt.year - 2000) : (dt.year - 2001)) << 1)
            ) % 7;      // DAY OF WEEK, 0 = SUNDAY ... 6 = SATURDAY

    if(dow == 0)
        dow = 7;        // CONVERT TO 1 = MONDAY ... 7 = SUNDAY

    dt.year -= 2000;
    __rtc_setwrite(1);

  retry_set_date:
    __setRTCDay(dt.mday);
    __setRTCDow(dow);
    __setRTCMon(dt.mon);
    __setRTCYear(dt.year);

    // CHECK IF A ROLLOVER MAY HAVE OCCURED
    if((__getRTCSec() == 0) && !have_retried) {
        have_retried = 1;
        goto retry_set_date;
    }

    __rtc_setwrite(0);

    return 1;
}

struct time rtc_gettime()
{
    struct time tm;
    int have_retried = 0;

    __rtc_check_device();

  retry_get_time:
    tm.min = __getRTCMin();
    tm.hour = __getRTCHour();
    tm.sec = __getRTCSec();

    // CHECK IF A ROLLOVER MAY HAVE OCCURED
    if((tm.sec == 0) && !have_retried) {
        have_retried = 1;
        goto retry_get_time;
    }

    return tm;
}

int rtc_settime(struct time tm)
{
    union rtccon rtc;

    if(!__rtc_is_valid_tm(tm))
        return 0;

    rtc.byte = __getRTCCon();
    rtc.clkrst = 1;
    rtc.rtcen = 1;
    __setRTCCon(rtc.byte);

    __setRTCSec(tm.sec);
    __setRTCMin(tm.min);
    __setRTCHour(tm.hour);

    rtc.clkrst = 0;
    rtc.rtcen = 0;
    __setRTCCon(rtc.byte);

    return 1;
}

void rtc_getdatetime(struct date *dt, struct time *tm)
{
    int have_retried = 0;

  retry_get_datetime:
    *dt = rtc_getdate();
    *tm = rtc_gettime();

    // CHECK IF A ROLLOVER MAY HAVE OCCURED
    if((__getRTCSec() == 0) && !have_retried) {
        have_retried = 1;
        goto retry_get_datetime;
    }

    return;
}

int rtc_setdatetime(struct date dt, struct time tm)
{
    int have_retried = 0;

  retry_set_datetime:
    if(!rtc_setdate(dt))
        return 0;
    if(!rtc_settime(tm))
        return 0;

    // CHECK IF A ROLLOVER MAY HAVE OCCURED
    if((__getRTCSec() == 0) && !have_retried) {
        have_retried = 1;
        goto retry_set_datetime;
    }

    return 1;
}


void rtc_getalarm(struct date *dt, struct time *tm, int *enabled)
{
    union rtcalm alrm;

    __rtc_check_device();

    alrm.byte = __getRTCAlm();

    *enabled = alrm.almen;

    if(alrm.secen)
        tm->sec = __getALMSec();
    else
        tm->sec = TIME_MAXSEC;
    if(alrm.minen)
        tm->min = __getALMMin();
    else
        tm->min = TIME_MAXMIN;
    if(alrm.houren)
        tm->hour = __getALMHour();
    else
        tm->hour = TIME_MAXHOUR;
    if(alrm.dateen)
        dt->mday = __getALMDay();
    else
        dt->mday = 0;
    if(alrm.monen)
        dt->mon = __getALMMon();
    else
        dt->mon = 0;
    if(alrm.yearen)
        dt->year = __getALMYear() + 2000;
    else
        dt->year = 0;

    return;
}

// SET ALARM INTERRUPT
int rtc_setalarm(struct date dt, struct time tm, int enabled)
{
    union rtcalm alrm;

    alrm.byte = 0;

    __rtc_check_device();

    if(tm.sec < 60) {
        alrm.secen = 1;
        __setALMSec(tm.sec);
    }
    if(tm.min < 60) {
        alrm.minen = 1;
        __setALMMin(tm.min);
    }
    if(tm.hour < 24) {
        alrm.houren = 1;
        __setALMHour(tm.hour);
    }
    if(dt.mday > 0) {
        alrm.dateen = 1;
        __setALMDay(dt.mday);
    }
    if(dt.mon > 0) {
        alrm.monen = 1;
        __setALMMon(dt.mon);
    }
    if(dt.year > 0) {
        alrm.yearen = 1;
        __setALMYear(dt.year - 2000);
    }

    alrm.almen = enabled ? 1 : 0;

    __setRTCAlm(alrm.byte);

    return 1;
}

int rtc_chkalrm()
{
    if(*SRCPND1 & (1 << INT_RTC)) {
        *SRCPND1 = (1 << INT_RTC);
        return 1;
    }

    if(__rtc_almint) {
        __rtc_almint = 0;
        return 1;
    }

    return 0;
}

void rtc_setaie(int enabled)
{
    union rtcalm alrm;

    alrm.byte = __getRTCAlm();
    alrm.almen = enabled ? 1 : 0;
    __setRTCAlm(alrm.byte);

    return;
}

// UNCOMMENT IF NEEDED
/*
void rtc_gettick(int *freq, int *enabled)
{
    union ticnt tick;

    tick.byte = __getRTCTic();

    if (tick.count)
        *freq = 128 / (tick.count + 1);
    else
        *freq = 0;

    *enabled = tick.enable;

    return;
}
*/
// SET PERIODIC INTERRUPT
int rtc_settick(int freq, int enabled)
{
    union ticnt tick;

    tick.byte = 0;

    if(freq > 127)
        return 0;
    if(freq != 0)
        tick.count = (128 / freq) - 1;

    tick.enable = enabled ? 1 : 0;

    __setRTCTic(tick.byte);

    return 1;
}

// SET TIME ROUND RESET
int rtc_setrnd_tm(int bound, int enabled)
{
    union rtcrst rnd_tm;

    switch (bound) {
    case 0:
        break;
    case 30:
    case 40:
    case 50:
        bound /= 10;
        break;
    default:
        return 0;
    }

    rnd_tm.seccr = bound;
    rnd_tm.srsten = enabled ? 1 : 0;

    __setRTCRst(rnd_tm.byte);

    return 1;
}

// IRQ HANDLERS
// UNCOMMENT IF NEEDED
/*void __rtc_alrmirq()
{
    __rtc_almint = 1;

    return;
}

void __rtc_tickirq()
{

    return;
}
*/
void __rtc_poweron()
{
    struct date rtc_dt, alrm_dt;
    struct time rtc_tm, alrm_tm;
    int enabled;

    // TODO if needed : Restaure Tick

    // ! S3C2410 BUG !
    // ON ALARM WAKE-UP, SRCPND IS NOT SET,
    // SO WE NEED TO CHECK ALARM DATE AND TIME.

    rtc_getalarm(&alrm_dt, &alrm_tm, &enabled);
    rtc_getdatetime(&rtc_dt, &rtc_tm);

    if(enabled) {
        if(alrm_dt.year > 0)
            if(alrm_dt.year != rtc_dt.year)
                return;
        if(alrm_dt.mon > 0)
            if(alrm_dt.mon != rtc_dt.mon)
                return;
        if(alrm_dt.mday > 0)
            if(alrm_dt.mday != rtc_dt.mday)
                return;
        if(alrm_tm.hour < 24)
            if(alrm_tm.hour != rtc_tm.hour)
                return;
        if(alrm_tm.min < 60)
            if(alrm_tm.min != rtc_tm.min)
                return;
        if(alrm_tm.sec < 60)
            if(alrm_tm.sec != (rtc_tm.sec))
                return;

        __rtc_almint = 1;
        return;
    }

    __rtc_almint = 0;

    return;
}

void __rtc_poweroff()
{
    __rtc_setwrite(0);

    // TODO if needed : Save Tick

    return;
}

// UNCOMMENT IF NEEDED
/*
void __rtc_setup()
{
    __irq_addhook(INT_RTC, (__interrupt__)&__rtc_alrmirq);
    __irq_addhook(INT_TICK, (__interrupt__)&__rtc_tickirq);

    __irq_unmask(INT_RTC);
    __irq_unmask(INT_TICK);

    return;
}
*/
void __rtc_reset()
{
    struct date dt;
    struct time tm;

    __rtc_check_device();
    rtc_settick(0, 0);
    __setRTCAlm(0);
    rtc_setrnd_tm(0, 0);

    rtc_getdatetime(&dt, &tm);

    // CHECK RTC TIME
    if(!__rtc_is_valid_dt(dt) || !__rtc_is_valid_tm(tm)) {
        dt.mday = 1;
        dt.mon = 1;
        dt.year = 2000;
        tm.hour = 0;
        tm.min = 0;
        tm.sec = 1;
        rtc_setdatetime(dt, tm);
    }

    __rtc_setwrite(0);

    __rtc_almint = 0;

    return;
}
