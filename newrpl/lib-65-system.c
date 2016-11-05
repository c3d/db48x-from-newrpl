/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

//#include <stdio.h>

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************



// REPLACE THE NUMBER
#define LIBRARY_NUMBER  65


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATELY

#define COMMAND_LIST \
    ECMD(SETDATE,"→DATE",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DATEADD,"DATE+",MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    ECMD(SETTIME,"→TIME",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TOHMS,"→HMS",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(FROMHMS,"HMS→",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(HMSADD,"HMS+",MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    ECMD(HMSSUB,"HMS-",MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    CMD(TICKS,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(CLKADJ,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(DATE,MKTOKENINFO(4,TITYPE_FUNCTION,0,2)), \
    CMD(DDAYS,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(TIME,MKTOKENINFO(4,TITYPE_FUNCTION,0,2)), \
    CMD(TSTR,MKTOKENINFO(4,TITYPE_NOTALLOWED,2,2)), \
    CMD(ACK,MKTOKENINFO(3,TITYPE_NOTALLOWED,0,2)), \
    CMD(MEM,MKTOKENINFO(3,TITYPE_NOTALLOWED,0,2)), \
    CMD(VERSION,MKTOKENINFO(7,TITYPE_NOTALLOWED,0,2)), \
    CMD(MEMCHECK,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(MEMFIX,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(READCFI,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(PEEK,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(POKE,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(ALRM,MKTOKENINFO(4,TITYPE_NOTALLOWED,0,2)), \
    CMD(TICK,MKTOKENINFO(4,TITYPE_NOTALLOWED,0,2))
//    ECMD(CMDNAME,"CMDNAME",MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE

#define ERROR_LIST \
ERR(INVALIDDATE,0), \
ERR(INVALIDTIME,1), \
ERR(BADARGVALUE,2)


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************


INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib65_menu);
//INCLUDE_ROMOBJECT(versobj);
//INCLUDE_ROMOBJECT(copyobj);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib65_menu,
//    (WORDPTR)versobj,
//    (WORDPTR)copyobj,
    0
};

// RETURNS THE NUMBER OF BYTES OF AVAILABLE RAM.
BINT rplGetFreeMemory()
{
    BINT mem = 0;

    mem += (DSTop - DStk) & 0x3ff;
    mem += (RSTop - RStk) & 0x3ff;
    mem += (LAMs - LAMTop) & 0x3ff;
    mem += (DirsTop - Directories) & 0x3ff;
    mem += (TempBlocksEnd - TempBlocks) & 0x3ff;
    mem += (TempObEnd - TempOb) & 0x3ff;

    mem  = ((6 * 1024) - mem) << 2;
    mem += halGetFreePages() << 12;

    return mem;
}


// DATE AND TIME FUNCTIONS

static const unsigned char const month_days[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

BINT rplGetMonthDays(BINT month, BINT year)
{
    return month_days[month - 1] + ((month == 2) && ISLEAPYEAR(year));
}

// READ A REAL AS A DATE.
// REAL HAS THE FORM MM.DDYYYY OR DD.MMYYYY ,
// DEPENDING ON THE STATE OF THE FLAG 'FL_DATEFORMAT'.
// THE RANGE OF ALLOWABLE DATES IS OCTOBER 15, 1582, TO DECEMBER 31, 9999.
// RETURN 0 ON SUCCESS OR
// RETURN 1 IF AN INVALID DATE IS GIVEN
BINT rplReadRealAsDate(REAL *date, struct date *dt)
{
    if (rplReadRealAsDateNoCk(date, dt)) return 1;
    if (!rplIsValidDate(*dt)) return 1;

    return 0;
}

// READ A REAL AS A DATE WITHOUT PERFORMING DATE VALIDITY CHECK.
// REAL HAS THE FORM MM.DDYYYY OR DD.MMYYYY ,
// DEPENDING ON THE STATE OF THE FLAG 'FL_DATEFORMAT'.
// RETURN 0 ON SUCCESS OR
// RETURN 1 IF REAL IS OUT OF RANGE.
BINT rplReadRealAsDateNoCk(REAL *date, struct date *dt)
{
    REAL r_date;
    BINT year, month, day;

    cloneReal(&r_date, date);
    r_date.exp += 6;

    if (!inBINTRange(&r_date) || isNANorinfiniteReal(&r_date)
        || (r_date.flags & F_NEGATIVE))
            return 1;

    year  = getBINTReal(&r_date);
    month = year  / 1000000;
    year -= month * 1000000;
    day   = year  / 10000;
    year -= day   * 10000;

    if (rplTestSystemFlag(FL_DATEFORMAT)) {
        BINT swap = month;
        month = day;
        day = swap;
    }

    // CHECK VALUES RANGE
    if (day   > DATE_MAXDAY)  return 1;
    if (month > DATE_MAXMON)  return 1;
    if (year  > DATE_MAXYEAR) return 1;

    dt->mday = day;
    dt->mon  = month;
    dt->year = year;

    return 0;
}

// READ A DATE AS A REAL.
// REAL HAS THE FORM MM.DDYYYY OR DD.MMYYYY ,
// DEPENDING ON THE STATE OF THE FLAG 'FL_DATEFORMAT'.
// THE RANGE OF ALLOWABLE DATES IS OCTOBER 15, 1582, TO DECEMBER 31, 9999.
// RETURN 0 ON SUCCESS OR
// RETURN 1 IF AN INVALID DATE IS GIVEN.
BINT rplReadDateAsReal(struct date dt, REAL *date)
{
    BINT b_date;

    if (!rplIsValidDate(dt)) return 1;

    b_date = dt.year;
    if (rplTestSystemFlag(FL_DATEFORMAT)) {
        b_date += dt.mon  * 10000;
        b_date += dt.mday * 1000000;
    } else {
        b_date += dt.mday * 10000;
        b_date += dt.mon  * 1000000;
    }

    newRealFromBINT(date, b_date, -6);

    return 0;
}

// CHECK DATE IS CONFORM TO THE GREGORIAN CALENDAR.
// THE RANGE OF ALLOWABLE DATES IS OCTOBER 15, 1582, TO DECEMBER 31, 9999.
// RETURN 1 IF A VALID DATE IS GIVEN, 0 OTHERWISE.
BINT rplIsValidDate(struct date dt)
{
    if (dt.year < 1582 || dt.year > 9999) return 0;
    if (dt.mon  < 1    || dt.mon  > 12) return 0;
    if (dt.mday < 1    || dt.mday > rplGetMonthDays(dt.mon, dt.year)) return 0;

    if (dt.year == 1582) {
        if (dt.mon < 10) return 0;
        if ((dt.mon == 10) && (dt.mday < 15)) return 0;
    }

    return 1;
}

// CONVERT DATE TO DAY NUMBER.
// OCTOBER 15, 1582 HAS NUMBER 1.
// RETURN DAY NUMBER.
BINT rplDateToDays(struct date dt)
{
    BINT days;

    if(dt.mon < 3) {
        dt.year--;
        dt.mon += 12;
    }

    days = (365*dt.year) + (dt.year/4) - (dt.year/100) + (dt.year/400);
    days += (306 * (dt.mon+1)) / 10;
    days += dt.mday - 578163;

    return days;
}

// CONVERT DAY NUMBER TO DATE.
// OCTOBER 15, 1582 HAS NUMBER 1.
// RETURN DATE.
struct date rplDaysToDate(BINT days)
{
    struct date dt;
    BINT        mon, year, ddays;

    // ADD DAY NUMBER OF OCTOBER 15, 1582, SINCE JANUARY 0, 0
    days += 578040;

    year = (BINT)((10000*(BINT64)days + 14780)/3652425);

    // REMAINING DAYS
    ddays = days - (year*365 + year/4 - year/100 + year/400);
    if (ddays < 0) {
        year--;
        ddays = days - (year*365 + year/4 - year/100 + year/400);
    }

    // MONTHS (COULD BE > 12)
    mon = ((100 * ddays) + 52) / 3060;

    dt.year = (year + (mon + 2)/12);
    dt.mon  = (((mon + 2) % 12) + 1);
    dt.mday = (ddays - ((mon * 306) + 5)/10 + 1);

    return dt;
}

// READ A REAL AS A TIME.
// REAL HAS THE FORM 'HH.MMSS' IN THE 24H FORMAT.
// RETURN 0 ON SUCCESS OR
// RETURN 1 IF AN INVALID TIME IS GIVEN.
BINT rplReadRealAsTime(REAL *time, struct time *tm)
{
    REAL r_time;
    BINT hour, min, sec;

    cloneReal(&r_time, time);
    r_time.exp += 4;

    if (!inBINTRange(&r_time) || isNANorinfiniteReal(&r_time)
        || (r_time.flags & F_NEGATIVE))
            return -1;

    sec = getBINTReal(&r_time);
    hour = sec / 10000;
    sec -= hour * 10000;
    min = sec / 100;
    sec -= min * 100;

    // VERIFY A VALID TIME IS GIVEN
    if (min  > 59) return 1;
    if (sec  > 59) return 1;
    if (hour > 23) return 1;

    tm->hour = hour;
    tm->min  = min;
    tm->sec  = sec;

    return 0;
}

// READ A TIME AS A REAL.
// REAL HAS THE FORM 'HH.MMSS' IN THE 24H FORMAT.
// RETURN 0 ON SUCCESS OR
// RETURN 1 IF AN INVALID TIME IS GIVEN.
BINT rplReadTimeAsReal(struct time tm, REAL *time)
{
    BINT b_time;

    // VERIFY A VALID TIME IS GIVEN
    if (tm.min  > 59) return 1;
    if (tm.sec  > 59) return 1;
    if (tm.hour > 23) return 1;

    b_time  = tm.sec;
    b_time += tm.min * 100;
    b_time += tm.hour * 10000;

    newRealFromBINT(time, b_time, -4);

    return 0;
}

// CONVERTS A REAL NUMBER REPRESENTING HOURS OR
// DEGREES WITH A DECIMAL FRACTION TO HH.MMSSs FORMAT.
// USES RREG 0 - 5
void rplDecimalToHMS(REAL *dec, REAL *hms)
{
#define HH RReg[0]
#define MM RReg[1]
#define SS RReg[2]

    // SPLIT DECIMAL INTO:
    // IP (HH) ; FP1 (RREG[3])
    ipReal(&HH, dec, 1);
    fracReal(&RReg[3], dec);

    // FP2(RREG[5]) = FP1*0.6
    // MM = TRUNC(FP2, 2)
    rplZeroToRReg(4);
    RReg[4].data[0] = 6;
    RReg[4].exp--;
    mulReal(&RReg[5], &RReg[3], &RReg[4]);

    /*
        if (RReg[5].flags&F_APPROX) {
            //rplNewBINTPush(0,DECBINT);
            roundReal(&RReg[5], &RReg[5], RReg[5].len);
            RReg[5].flags &= ~F_APPROX;
        }*/

    truncReal(&MM, &RReg[5], 2);

    // SS = (FP1 - FP2)*0.6
    RReg[5].exp += 2;
    fracReal(&RReg[5], &RReg[5]);
    RReg[5].exp -= 2;
    mulReal(&SS, &RReg[5], &RReg[4]);

    // HMS = HH + MM + SS
    addReal(&RReg[3], &HH, &MM);
    addReal(hms, &RReg[3], &SS);

    return;

#undef HH
#undef MM
#undef SS
}

// CONVERTS A REAL NUMBER IN HH.MMSSs FORMAT TO
// ITS DECIMAL FORM (HOURS OR DEGREES WITH A
// DECIMAL FRACTION).
// USES RREG 0 - 5
void rplHMSToDecimal(REAL *hms, REAL *dec)
{
#define HD RReg[0]
#define MD RReg[1]
#define SD RReg[2]

    REAL sdec;

    cloneReal(&sdec, hms);
    ipReal(&HD, hms, 1);

    // SD = SS / .6
    sdec.exp += 2;
    fracReal(&RReg[4], &sdec);
    RReg[4].exp -= 2;

    rplZeroToRReg(3);
    RReg[3].data[0] = 6;
    RReg[3].exp--;

    divReal(&SD, &RReg[4], &RReg[3]);

    // MD = (MM + sdec) / .6
    ipReal(&RReg[4], &sdec, 1);
    RReg[4].exp -= 2;
    fracReal(&RReg[4], &RReg[4]);
    addReal(&RReg[5], &SD, &RReg[4]);
    divReal(&MD, &RReg[5], &RReg[3]);

    // DEC = HD + MD
    addReal(dec, &HD, &MD);

    return;

#undef HD
#undef MD
#undef SD
}


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    if(ISUNARYOP(CurOpcode)) {
        if(!ISPROLOG(*rplPeekData(1))) {
            if( (OPCODE(CurOpcode)==OVR_EVAL)||
                    (OPCODE(CurOpcode)==OVR_EVAL1)||
                    (OPCODE(CurOpcode)==OVR_XEQ) )
            {

                WORD saveOpcode=CurOpcode;
                CurOpcode=*rplPopData();
                // RECURSIVE CALL
                LIB_HANDLER();
                CurOpcode=saveOpcode;
                return;
            }
            else {
                rplError(ERR_INVALIDOPCODE);
                return;
            }
        }
        else {
            rplError(ERR_UNRECOGNIZEDOBJECT);
            return;
        }
    }


    switch(OPCODE(CurOpcode))
    {
    case TICKS:
        // RETURN SYSTEM CLOCK
    {
        BINT64 ticks=halTicks();
        rplNewBINTPush(ticks,DECBINT);
        return;
    }
    case DATE:
    {
        struct date dt;

        dt = halGetSystemDate();

        rplReadDateAsReal(dt, &RReg[0]);
        rplNewRealFromRRegPush(0);

        return;
    }
    case SETDATE:
    {
        struct date dt;
        WORDPTR     arg_date;
        REAL        r_date;

        if (rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        arg_date = rplPeekData(1);
        if (!ISREAL(*arg_date)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        rplReadReal(arg_date, &r_date);
        if (rplReadRealAsDateNoCk(&r_date, &dt)) {
            rplError(ERR_INVALIDDATE);
            return;
        }

        if (!dt.year) {
            struct date sys_dt;
            sys_dt = halGetSystemDate();
            dt.year = sys_dt.year;
        }

        if (!rplIsValidDate(dt)) {
            rplError(ERR_INVALIDDATE);
            return;
        }

        // CHECK FOR DATES SILENTLY REJECTED
        if ((dt.year > 1990) && (dt.year < 2000)) {
            rplDropData(1);
            return;
        }

        // CHECK THE RANGE OF ALLOWABLE DATES
        if ((dt.year < 2000) || (dt.year > 2090)) {
            rplError(ERR_INVALIDDATE);
            return;
        }

        halSetSystemDate(dt);
        rplDropData(1);

        return;
    }
    case DATEADD:
    {
        WORDPTR     arg_date, arg_days;
        struct date dt;
        BINT        days, day_num;
        REAL        r_date, r_days;

        if (rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        arg_days = rplPeekData(1);
        arg_date = rplPeekData(2);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if (ISLIST(*arg_days) || ISLIST(*arg_date)) {
            rplListBinaryDoCmd(arg_days, arg_date);
            return;
        }

        if (!ISNUMBER(*arg_days) || !ISREAL(*arg_date)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        rplReadReal(arg_date, &r_date);
        if (rplReadRealAsDate(&r_date, &dt)) {
            rplError(ERR_INVALIDDATE);
            return;
        }

        day_num = rplDateToDays(dt);

        rplReadNumberAsReal(arg_days, &r_days);
        roundReal(&RReg[0], &r_days, 0);
        days = getBINTReal(&RReg[0]);

        day_num += days;
        if ((day_num < 1) || (day_num > 3074324)) {
            rplError(ERR_BADARGVALUE);
            return;
        }

        dt = rplDaysToDate(day_num);
        rplReadDateAsReal(dt, &RReg[0]);
        rplDropData(2);
        rplNewRealFromRRegPush(0);

        return;
    }
    case DDAYS:
    {
        struct date dt;
        WORDPTR     arg_date1, arg_date2;
        BINT        ddays;
        REAL        r_date;

        if (rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        arg_date1 = rplPeekData(1);
        arg_date2 = rplPeekData(2);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if (ISLIST(*arg_date1) || ISLIST(*arg_date2)) {
            rplListBinaryDoCmd(arg_date1, arg_date2);
            return;
        }

        if (!ISREAL(*arg_date1) || !ISREAL(*arg_date2)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        rplReadReal(arg_date1, &r_date);
        if (rplReadRealAsDate(&r_date, &dt)) {
            rplError(ERR_INVALIDDATE);
            return;
        }

        ddays = rplDateToDays(dt);

        rplReadReal(arg_date2, &r_date);
        if (rplReadRealAsDate(&r_date, &dt)) {
            rplError(ERR_INVALIDDATE);
            return;
        }
        ddays -= rplDateToDays(dt);

        rplDropData(2);
        rplNewBINTPush((BINT64)ddays, DECBINT);

        return;
    }
    case TIME:
    {
        struct time tm;

        tm = halGetSystemTime();

        rplReadTimeAsReal(tm, &RReg[0]);
        rplNewRealFromRRegPush(0);

        return;
    }
    case SETTIME:
    {
        struct time tm;
        WORDPTR     arg_time;
        REAL        r_time;

        if (rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        arg_time = rplPeekData(1);
        if (!ISNUMBER(*arg_time)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        rplReadNumberAsReal(arg_time, &r_time);
        if (rplReadRealAsTime(&r_time, &tm)) {
            rplError(ERR_INVALIDTIME);
            return;
        }

        halSetSystemTime(tm);
        rplDropData(1);

        return;
    }
    case TOHMS:
    {
        WORDPTR arg_dec;
        REAL    r_dec;

        if (rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        arg_dec = rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if (ISLIST(*arg_dec)) {
            rplListUnaryDoCmd();
            return;
        }

        if (ISBINT(*arg_dec))
            return;

        if (!ISREAL(*arg_dec)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        rplReadReal(arg_dec, &r_dec);

        if (isNANorinfiniteReal(&r_dec)) {
            rplError(ERR_BADARGVALUE);
            return;
        }

        rplDecimalToHMS(&r_dec, &RReg[6]);

        rplDropData(1);
        rplNewRealFromRRegPush(6);

        return;
    }
    case FROMHMS:
    {
        WORDPTR arg_hms;
        REAL    r_hms;

        if (rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        arg_hms = rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if (ISLIST(*arg_hms)) {
            rplListUnaryDoCmd();
            return;
        }

        if (ISBINT(*arg_hms))
            return;

        if (!ISREAL(*arg_hms)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        rplReadReal(arg_hms, &r_hms);

        if (isNANorinfiniteReal(&r_hms)) {
            rplError(ERR_BADARGVALUE);
            return;
        }

        rplHMSToDecimal(&r_hms, &RReg[7]);

        rplDropData(1);
        rplNewRealFromRRegPush(7);

        return;
    }
    case HMSADD:
    {
        WORDPTR arg_hms1, arg_hms2;
        REAL    r_hms1, r_hms2;

        if (rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        arg_hms1 = rplPeekData(1);
        arg_hms2 = rplPeekData(2);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if (ISLIST(*arg_hms1) || ISLIST(*arg_hms2)) {
            rplListBinaryDoCmd(arg_hms1, arg_hms2);
            return;
        }

        if (!ISNUMBER(*arg_hms1) || !ISNUMBER(*arg_hms2)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        rplReadNumberAsReal(arg_hms1, &r_hms1);
        rplReadNumberAsReal(arg_hms2, &r_hms2);

        if (isNANorinfiniteReal(&r_hms1) || isNANorinfiniteReal(&r_hms2)) {
            rplError(ERR_BADARGVALUE);
            return;
        }

        rplHMSToDecimal(&r_hms1, &RReg[6]);
        rplHMSToDecimal(&r_hms2, &RReg[7]);

        addReal(&RReg[8], &RReg[7], &RReg[6]);

        rplDecimalToHMS(&RReg[8], &RReg[6]);

        rplDropData(2);
        rplNewRealFromRRegPush(6);

        return;
    }
    case HMSSUB:
    {
        WORDPTR arg_hms1, arg_hms2;
        REAL    r_hms1, r_hms2;

        if (rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        arg_hms1 = rplPeekData(1);
        arg_hms2 = rplPeekData(2);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if (ISLIST(*arg_hms1) || ISLIST(*arg_hms2)) {
            rplListBinaryDoCmd(arg_hms1, arg_hms2);
            return;
        }

        if (!ISNUMBER(*arg_hms1) || !ISNUMBER(*arg_hms2)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        rplReadNumberAsReal(arg_hms1, &r_hms1);
        rplReadNumberAsReal(arg_hms2, &r_hms2);

        if (isNANorinfiniteReal(&r_hms1) || isNANorinfiniteReal(&r_hms2)) {
            rplError(ERR_BADARGVALUE);
            return;
        }

        rplHMSToDecimal(&r_hms1, &RReg[7]);
        rplHMSToDecimal(&r_hms2, &RReg[8]);

        subReal(&RReg[9], &RReg[8], &RReg[7]);

        rplDecimalToHMS(&RReg[9], &RReg[7]);

        rplDropData(2);
        rplNewRealFromRRegPush(7);

        return;
    }
    case MEM:
    {
        rplGCollect();
        rplNewBINTPush((BINT64)rplGetFreeMemory(), DECBINT);

        return;
    }/*
    case VERSION:
    {
        rplPushData(versobj);
        rplPushData(copyobj);
        return;
    }*/
    case ACK: // ONLY FOR TESTS
    {
        halSetNotification(N_CONNECTION, 0);

        return;
    }
    case TICK: // ONLY FOR TESTS
    {
        int freq, enabled;

        rtc_gettick(&freq, &enabled);

        if (enabled) {
            rtc_settick(0, 0);
            halSetNotification(N_CONNECTION,0);
        } else {
            rtc_settick(2, 1);
        }

        return;
    }
    case ALRM: // ONLY FOR TESTS
    {
        struct date dt;
        struct time tm;
        unsigned char alrm;
        unsigned char con;
        int enabled = 0;

        alrm = (*((volatile unsigned char *)(RTC_REGS + 0x50)));
        con = (*((volatile unsigned char *)(RTC_REGS + 0x40)));
        rplNewBINTPush((BINT64)alrm,BINBINT);
        rplNewBINTPush((BINT64)con,BINBINT);

        dt = halGetSystemDate();
        tm = halGetSystemTime();

        if (tm.sec < 50) {
            tm.sec += 10;
            enabled = 1;
        }

        if (!halSetSystemAlarm(dt, tm, enabled)) {
            rplError(ERR_BADARGVALUE);
            return;
        }

        halGetSystemAlarm(&dt, &tm, &enabled);

        if (rplReadDateAsReal(dt, &RReg[0])) {
            rplError(ERR_INVALIDDATE);
            return;
        }
        rplNewRealFromRRegPush(0);

        if (rplReadTimeAsReal(tm, &RReg[0])) {
            rplError(ERR_INVALIDTIME);
            return;
        }
        rplNewRealFromRRegPush(0);

        alrm = (*((volatile unsigned char *)(RTC_REGS + 0x50)));
        con = (*((volatile unsigned char *)(RTC_REGS + 0x40)));
        rplNewBINTPush((BINT64)alrm,BINBINT);
        rplNewBINTPush((BINT64)con,BINBINT);

        return;
    }
    case MEMCHECK:
    {
        // SYSTEM SANITY CHECK

        if(rplVerifyDStack(0)) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        if(rplVerifyRStack()) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        if(rplVerifyTempOb(0)) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        if(rplVerifyDirectories(0)) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);

        return;

    }
    case MEMFIX:
    {
        if(rplVerifyDStack(0)) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        if(rplVerifyRStack()) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        if(rplVerifyTempOb(1)) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        if(rplVerifyDirectories(0)) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        return;
    }
    case READCFI:
    {
        unsigned short buffer[100];

        flash_CFIRead(buffer);

        WORDPTR newobj=rplCreateString((BYTEPTR)buffer,(BYTEPTR)buffer+6);

        rplPushData(newobj);

     return;
    }

    case PEEK:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

       BINT64 addr=rplReadBINT(rplPeekData(1));

       if((addr<0)||(addr>0xffffffffLL)) {
        rplError(ERR_ARGOUTSIDEDOMAIN);
        return;
       }

       rplDropData(1);

       rplNewBINTPush((BINT64) (*(NUMBER2PTR(addr&0xffffffff))),HEXBINT);

       return;
    }
    case POKE:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }
        if(!ISBINT(*rplPeekData(2))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

       BINT64 addr=rplReadBINT(rplPeekData(2));
       BINT64 value=rplReadBINT(rplPeekData(1));

       if((addr<0)||(addr>0xffffffffLL)) {
        rplError(ERR_ARGOUTSIDEDOMAIN);
        return;
       }


       rplDropData(2);
       WORDPTR ptr=NUMBER2PTR(addr&0xffffffff);
       *ptr=(WORD)(value&0xffffffffLL);

       return;
    }



        /*
    case DONUM:
    {
        BINT f;
        REAL r;

        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&r);
        copyReal(&RReg[0],&r);
        left_justify(&RReg);

        printf("// LEN=%d\n",RReg[0].len);

        for(f=0;f<RReg[0].len;++f) {
            printf("%d, ",RReg[0].data[f]);
        }

        return;


    }*/

    // ADD MORE OPCODES HERE

   // STANDARIZED OPCODES:
    // --------------------
    // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES


    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors


        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

     return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds((char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_VALIDATE:
        // VALIDATE RECEIVES OPCODES COMPILED BY OTHER LIBRARIES, TO BE INCLUDED WITHIN A COMPOSITE OWNED BY
        // THIS LIBRARY. EVERY COMPOSITE HAS TO EVALUATE IF THE OBJECT BEING COMPILED IS ALLOWED INSIDE THIS
        // COMPOSITE OR NOT. FOR EXAMPLE, A REAL MATRIX SHOULD ONLY ALLOW REAL NUMBERS INSIDE, ANY OTHER
        // OPCODES SHOULD BE REJECTED AND AN ERROR THROWN.
        // Library receives:
        // CurrentConstruct = SET TO THE CURRENT ACTIVE CONSTRUCT TYPE
        // LastCompiledObject = POINTER TO THE LAST OBJECT THAT WAS COMPILED, THAT NEEDS TO BE VERIFIED

        // VALIDATE RETURNS:
        // RetNum =  OK_CONTINUE IF THE OBJECT IS ACCEPTED, ERR_INVALID IF NOT.


        RetNum=OK_CONTINUE;
        return;

    case OPCODE_PROBETOKEN:
        // PROBETOKEN FINDS A VALID WORD AT THE BEGINNING OF THE GIVEN TOKEN AND RETURNS
        // INFORMATION ABOUT IT. THIS OPCODE IS MANDATORY

        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND
        {
        libProbeCmds((char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);

        return;
        }


    case OPCODE_GETINFO:
        libGetInfo2(*DecompileObject,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        return;



    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        RetNum=ERR_NOTMINE;
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        RetNum=ERR_NOTMINE;
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) { RetNum=ERR_INVALID; return; }

        RetNum=OK_CONTINUE;
        return;
    case OPCODE_AUTOCOMPNEXT:
        // THIS OPCODE RECEIVES
        // TokenStart = token string
        // TokenLen = token length
        // SuggestedOpcode = OPCODE OF THE CURRENT SUGGESTION, OR 0 IF SUGGESTION IS AN OBJECT
        // SuggestedObject = POINTER TO AN OBJECT (ONLY VALID IF SuggestedOpcode==0)
        // IF SuggestedOpcode LIBRARY NUMBER<THIS LIBRARY
        // IT MUST RETURN ERR_NOTMINE
        // IF SuggestedOpcode OR SuggestedObject BELONG TO THIS LIBRARY,
        // SEARCH MUST CONTINUE AFTER THAT SUGGESTION
        // IF A NEW SUGGESTION IS FOUND, RETURN OK_CONTINUE, AND SET
        // SuggestedOpcode TO THE NEXT SUGGESTION, or 0xffffffff IF SUGGESTION IS AN OBJECT
        // IN SUCH CASE, SuggestedObject MUST BE SET TO POINT TO THE OBJECT IN QUESTION
        // IF THERE ARE NO MORE SUGGESTIONS, RETURN ERR_NOTMINE

        // AUTOCOMP RETURNS:
        // RetNum =  OK_CONTINUE
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND

        // NEXT OPCODE SCANS IN DECREASING LIBRARY AND DECREASING OPCODE NUMBER

        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(MENUNUMBER(MenuCodeArg)>0) {
            RetNum=ERR_NOTMINE;
            return;
        }
        ObjectPTR=(WORDPTR)lib65_menu;
        RetNum=OK_CONTINUE;
        return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp,(WORDPTR)LIB_HELPTABLE);
        return;
    }
    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {

        libFindMsg(LibError,(WORDPTR)LIB_MSGTABLE);
        return;
    }
    case OPCODE_LIBINSTALL:
        LibraryList=(WORDPTR)libnumberlist;
        RetNum=OK_CONTINUE;
        return;
    case OPCODE_LIBREMOVE:
        return;




    }
    // UNHANDLED OPCODE...

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode)>=MIN_RESERVED_OPCODE) {
        RetNum=ERR_NOTMINE;
        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    rplError(ERR_INVALIDOPCODE);

    return;
}





#endif
