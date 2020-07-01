/*
* Copyright (c) 2014-2016, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#ifndef RTC_H
#define RTC_H

static const unsigned char const month_days[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

// REAL TIME CLOCK CONTROL (RTCCON) REGISTER
union rtccon
{
    unsigned char byte; // Represents all fields.
    struct
    {
        unsigned int rtcen:1,   // RTC control enable.
            clksel:1,   // BCD clock select.
            cntsel:1,   // BCD count select.
            clkrst:1;   // RTC clock count reset.
    };
};

// RTC ALARM CONTROL (RTCALM) REGISTER
union rtcalm
{
    unsigned char byte; // Represents all fields.
    struct
    {
        unsigned int secen:1,   // Second alarm enable.
            minen:1,    // Minute alarm enable.
            houren:1,   // Hour alarm enable.
            dateen:1,   // Date alarm enable.
            monen:1,    // Month alarm enable.
            yearen:1,   // Year alarm enable.
            almen:1;    // Alarm global enable.
    };
};

// TICK TIME COUNT (TICNT) REGISTER
union ticnt
{
    unsigned char byte; // Represents all fields.
    struct
    {
        unsigned int count:7,   // Tick time count value (1~127).
            enable:1;   // Tick time interrupt enable.
    };
};

// RTC ROUND RESET (RTCRST) REGISTER
union rtcrst
{
    unsigned char byte; // Represents all fields.
    struct
    {
        unsigned int seccr:3,   // Round boundary for second carry generation.
            srsten:1;   // Round second reset enable.
    };
};

// INTERRUPT SERVICE NUMBER
#define INT_TICK        8
#define INT_RTC         30

#define BCD2BIN(x)      (((x) & 0x0f) + ((x) >> 4) * 10)
#define BIN2BCD(x)      ((((x) / 10) << 4) + (x) % 10)

// READ FROM RTC REGISTERS
#define __getRTCVal(r)  BCD2BIN(*(r))

#define __getRTCCon()   (*RTCCON)
#define __getRTCTic()   (*TICNT0)
#define __getRTCAlm()   (*RTCALM)
#define __getRTCRst()   (*RTCRST)

#define __getALMSec()   __getRTCVal(ALMSEC)
#define __getALMMin()   __getRTCVal(ALMMIN)
#define __getALMHour()  __getRTCVal(ALMHOUR)
#define __getALMDay()   __getRTCVal(ALMDATE)
#define __getALMMon()   __getRTCVal(ALMMON)
#define __getALMYear()  __getRTCVal(ALMYEAR)

#define __getRTCSec()   __getRTCVal(BCDSEC)
#define __getRTCMin()   __getRTCVal(BCDMIN)
#define __getRTCHour()  __getRTCVal(BCDHOUR)
#define __getRTCDay()   __getRTCVal(BCDDATE)
#define __getRTCDow()   __getRTCVal(BCDDAY)
#define __getRTCMon()   __getRTCVal(BCDMON)
#define __getRTCYear()  __getRTCVal(BCDYEAR)

// WRITE TO RTC REGISTERS
#define __setRTCVal(r, v) (*(r)  = (unsigned char)BIN2BCD(v))

#define __setRTCCon(v)  (*RTCCON = (unsigned char)v)
#define __setRTCTic(v)  (*TICNT0  = (unsigned char)v)
#define __setRTCAlm(v)  (*RTCALM = (unsigned char)v)
#define __setRTCRst(v)  (*RTCRST = (unsigned char)v)

#define __setALMSec(v)  __setRTCVal(ALMSEC,  v)
#define __setALMMin(v)  __setRTCVal(ALMMIN,  v)
#define __setALMHour(v) __setRTCVal(ALMHOUR, v)
#define __setALMDay(v)  __setRTCVal(ALMDATE, v)
#define __setALMMon(v)  __setRTCVal(ALMMON,  v)
#define __setALMYear(v) __setRTCVal(ALMYEAR, v)

#define __setRTCSec(v)  __setRTCVal(BCDSEC,  v)
#define __setRTCMin(v)  __setRTCVal(BCDMIN,  v)
#define __setRTCHour(v) __setRTCVal(BCDHOUR, v)
#define __setRTCDay(v)  __setRTCVal(BCDDATE, v)
#define __setRTCDow(v)  __setRTCVal(BCDDAY,  v)
#define __setRTCMon(v)  __setRTCVal(BCDMON,  v)
#define __setRTCYear(v) __setRTCVal(BCDYEAR, v)

#endif // RTC_H
