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
#define getRTCVal(r)  BCD2BIN(*(r))

#define getRTCCon()   (*RTCCON)
#define getRTCTic()   (*TICNT0)
#define getRTCAlm()   (*RTCALM)
#define getRTCRst()   (*RTCRST)

#define getALMSec()   getRTCVal(ALMSEC)
#define getALMMin()   getRTCVal(ALMMIN)
#define getALMHour()  getRTCVal(ALMHOUR)
#define getALMDay()   getRTCVal(ALMDATE)
#define getALMMon()   getRTCVal(ALMMON)
#define getALMYear()  getRTCVal(ALMYEAR)

#define getRTCSec()   getRTCVal(BCDSEC)
#define getRTCMin()   getRTCVal(BCDMIN)
#define getRTCHour()  getRTCVal(BCDHOUR)
#define getRTCDay()   getRTCVal(BCDDATE)
#define getRTCDow()   getRTCVal(BCDDAY)
#define getRTCMon()   getRTCVal(BCDMON)
#define getRTCYear()  getRTCVal(BCDYEAR)

// WRITE TO RTC REGISTERS
#define setRTCVal(r, v) (*(r)  = (unsigned char)BIN2BCD(v))

#define setRTCCon(v)  (*RTCCON = (unsigned char)v)
#define setRTCTic(v)  (*TICNT0  = (unsigned char)v)
#define setRTCAlm(v)  (*RTCALM = (unsigned char)v)
#define setRTCRst(v)  (*RTCRST = (unsigned char)v)

#define setALMSec(v)  setRTCVal(ALMSEC,  v)
#define setALMMin(v)  setRTCVal(ALMMIN,  v)
#define setALMHour(v) setRTCVal(ALMHOUR, v)
#define setALMDay(v)  setRTCVal(ALMDATE, v)
#define setALMMon(v)  setRTCVal(ALMMON,  v)
#define setALMYear(v) setRTCVal(ALMYEAR, v)

#define setRTCSec(v)  setRTCVal(BCDSEC,  v)
#define setRTCMin(v)  setRTCVal(BCDMIN,  v)
#define setRTCHour(v) setRTCVal(BCDHOUR, v)
#define setRTCDay(v)  setRTCVal(BCDDATE, v)
#define setRTCDow(v)  setRTCVal(BCDDAY,  v)
#define setRTCMon(v)  setRTCVal(BCDMON,  v)
#define setRTCYear(v) setRTCVal(BCDYEAR, v)

#endif // RTC_H
