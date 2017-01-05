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
    CMD(ACKALL,MKTOKENINFO(6,TITYPE_NOTALLOWED,0,2)), \
    CMD(RCLALARM,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(STOALARM,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(DELALARM,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(FINDALARM,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(VERSION,MKTOKENINFO(7,TITYPE_NOTALLOWED,0,2)), \
    CMD(MEM,MKTOKENINFO(3,TITYPE_NOTALLOWED,0,2)), \
    CMD(BYTES,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(PEEK,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(POKE,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(NEWOB,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(GARBAGE,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(MEMCHECK,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(MEMFIX,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(READCFI,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(DOALARM,MKTOKENINFO(7,TITYPE_NOTALLOWED,0,2)), \
    CMD(ALRM,MKTOKENINFO(4,TITYPE_NOTALLOWED,0,2))
//    ECMD(CMDNAME,"CMDNAME",MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE

#define ERROR_LIST \
ERR(INVALIDDATE,0), \
ERR(INVALIDTIME,1), \
ERR(BADALARMNUM,2), \
ERR(INVALIDRPT,3), \
ERR(BADARGVALUE,4), \
ERR(PASTDUEALRM,5), \
ERR(ALRMSKIPPED,6), \
ERR(ALRMCORRUPT,7)


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
INCLUDE_ROMOBJECT(lib65_menu_0_time);
INCLUDE_ROMOBJECT(lib65_menu_1_memory);
INCLUDE_ROMOBJECT(lib65_menu_2_alarms);
INCLUDE_ROMOBJECT(newrpl_version);
INCLUDE_ROMOBJECT(alarms_ident);


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib65_menu_0_time,
    (WORDPTR)lib65_menu_1_memory,
    (WORDPTR)lib65_menu_2_alarms,
    (WORDPTR)newrpl_version,
    (WORDPTR)alarms_ident,
    0
};



// RETURN THE NUMBER OF BYTES OF AVAILABLE RAM.
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
// RETURN 1 ON SUCCESS OR
// RETURN 0 IF AN INVALID DATE IS GIVEN
BINT rplReadRealAsDate(REAL *date, struct date *dt)
{
    if (!rplReadRealAsDateNoCk(date, dt)) return 0;
    if (!rplIsValidDate(*dt)) return 0;

    return 1;
}

// READ A REAL AS A DATE WITHOUT PERFORMING DATE VALIDITY CHECK.
// REAL HAS THE FORM MM.DDYYYY OR DD.MMYYYY ,
// DEPENDING ON THE STATE OF THE FLAG 'FL_DATEFORMAT'.
// RETURN 1 ON SUCCESS OR
// RETURN 0 IF REAL IS OUT OF RANGE.
BINT rplReadRealAsDateNoCk(REAL *date, struct date *dt)
{
    REAL r_date;
    BINT year, month, day;

    cloneReal(&r_date, date);
    r_date.exp += 6;

    if (!inBINTRange(&r_date) || isNANorinfiniteReal(&r_date)
        || (r_date.flags & F_NEGATIVE))
            return 0;

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
    if (day   > DATE_MAXDAY)  return 0;
    if (month > DATE_MAXMON)  return 0;
    if (year  > DATE_MAXYEAR) return 0;

    dt->mday = day;
    dt->mon  = month;
    dt->year = year;

    return 1;
}

// READ A DATE AS A REAL.
// REAL HAS THE FORM MM.DDYYYY OR DD.MMYYYY ,
// DEPENDING ON THE STATE OF THE FLAG 'FL_DATEFORMAT'.
// THE RANGE OF ALLOWABLE DATES IS OCTOBER 15, 1582, TO DECEMBER 31, 9999.
// RETURN 1 ON SUCCESS OR
// RETURN 0 IF AN INVALID DATE IS GIVEN.
BINT rplReadDateAsReal(struct date dt, REAL *date)
{
    BINT b_date;

    if (!rplIsValidDate(dt)) return 0;

    b_date = dt.year;
    if (rplTestSystemFlag(FL_DATEFORMAT)) {
        b_date += dt.mon  * 10000;
        b_date += dt.mday * 1000000;
    } else {
        b_date += dt.mday * 10000;
        b_date += dt.mon  * 1000000;
    }

    newRealFromBINT(date, b_date, -6);

    return 1;
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

// READ A REAL AS A TIME.
// REAL HAS THE FORM 'HH.MMSS' IN THE 24H FORMAT.
// RETURN 1 ON SUCCESS OR
// RETURN 0 IF AN INVALID TIME IS GIVEN.
BINT rplReadRealAsTime(REAL *time, struct time *tm)
{
    REAL r_time;
    BINT hour, min, sec;

    cloneReal(&r_time, time);
    r_time.exp += 4;

    if (!inBINTRange(&r_time) || isNANorinfiniteReal(&r_time)
        || (r_time.flags & F_NEGATIVE))
            return 0;

    sec = getBINTReal(&r_time);
    hour = sec / 10000;
    sec -= hour * 10000;
    min = sec / 100;
    sec -= min * 100;

    // VERIFY A VALID TIME IS GIVEN
    if (min  > 59) return 0;
    if (sec  > 59) return 0;
    if (hour > 23) return 0;

    tm->hour = hour;
    tm->min  = min;
    tm->sec  = sec;

    return 1;
}

// READ A TIME AS A REAL.
// REAL HAS THE FORM 'HH.MMSS' IN THE 24H FORMAT.
// RETURN 1 ON SUCCESS OR
// RETURN 0 IF AN INVALID TIME IS GIVEN.
BINT rplReadTimeAsReal(struct time tm, REAL *time)
{
    BINT b_time;

    // VERIFY A VALID TIME IS GIVEN
    if (tm.min  > 59) return 0;
    if (tm.sec  > 59) return 0;
    if (tm.hour > 23) return 0;

    b_time  = tm.sec;
    b_time += tm.min * 100;
    b_time += tm.hour * 10000;

    newRealFromBINT(time, b_time, -4);

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

BINT64 rplDateToSeconds(struct date dt, struct time tm)
{
    BINT64 sec;

    sec  = tm.sec;
    sec += (BINT64)tm.min * 60;
    sec += (BINT64)tm.hour * 3600;
    sec += (BINT64)rplDateToDays(dt) * 24 * 3600;

    return sec;
}

void rplSecondsToDate(BINT64 sec, struct date *dt, struct time *tm)
{
    BINT days;

    days = sec / (24 * 3600);
    *dt = rplDaysToDate(days);

    sec -= (BINT64)days * 24 * 3600;

    tm->hour = sec / 3600;
    sec -= (BINT64)tm->hour * 3600;
    tm->min = sec / 60;
    tm->sec = sec - ((BINT64)tm->min * 60);

    return;
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


// ALARM FUNCTIONS
// USES ScratchPointer4&5

#define alarms   ScratchPointer4
#define alrm_obj ScratchPointer5

static BINT64 GetSysTime()
{
    struct date dt;
    struct time tm;

    halGetSystemDateTime(&dt, &tm);

    return rplDateToSeconds(dt, tm);
}

// RETURN A POINTER TO THE ALARMS LIST
// RETURN 0 ON ERROR
static WORDPTR InitAlarms()
{
    WORDPTR *Stacksave = DSTop;

    rplPushData((WORDPTR)zero_bint);
    rplPushData((WORDPTR)zero_bint);
    rplPushData((WORDPTR)two_bint);
    if (Exceptions) {
        DSTop = Stacksave;
        return 0;
    }
    rplCreateList();
    if (Exceptions) {
        DSTop = Stacksave;
        return 0;
    }
    alarms = rplPeekData(1);
    rplDropData(1);
    rplStoreSettings((WORDPTR)alarms_ident, alarms);

    return alarms;
}

static WORDPTR ResetAlarms()
{
    alarms = InitAlarms();

    rplError(ERR_ALRMCORRUPT);
    halSetNotification(N_CONNECTION, 0x0);

    return alarms;
}

// RETURN NULL ON ERROR
static WORDPTR GetAlarms()
{
    alarms = rplGetSettings((WORDPTR)alarms_ident);

    if (!alarms) {
        alarms = InitAlarms();
    } else {
        if (!ISLIST(*alarms))
            alarms = ResetAlarms();
        if (rplListLength(alarms) < 2)
            alarms = ResetAlarms();
    }

    return alarms;
}


// LOW-LEVEL ALARMS INTERFACE

// RETURN 1 ON SUCCESS
// RETURN 0 IF list ISN'T AN ALARM
static BINT ReadSysAlarm(WORDPTR list, struct alarm *alrm)
{
    WORDPTR obj;

    if (!ISLIST(*list))
        return 0;
    if (rplListLength(list) != 4)
        return 0;

    obj = rplGetListElement(list, 1);
    if (!ISBINT(*obj))
        return 0;
    alrm->time = rplReadBINT(obj);

    obj = rplGetListElement(list, 2);
    if (!ISBINT(*obj))
        return 0;
    alrm->rpt = (UBINT)rplReadBINT(obj);

    obj = rplGetListElement(list, 3);
    if (!ISBINT(*obj))
        return 0;
    alrm->flags = (BYTE)rplReadBINT(obj);

    alrm->obj = rplGetListElement(list, 4);

    return 1;
}

static void PushSysAlarm(struct alarm *alrm)
{
    WORDPTR *Stacksave = DSTop;

    alrm_obj = alrm->obj;
    rplNewBINTPush(alrm->time, DECBINT);
    rplNewBINTPush((BINT64)alrm->rpt, DECBINT);
    rplNewBINTPush((BINT64)alrm->flags, DECBINT);
    rplPushData(alrm_obj);

    rplNewBINTPush(4, DECBINT);
    if (Exceptions) goto rtn_cleanup;

    rplCreateList();
    if (Exceptions) goto rtn_cleanup;

    goto rtn;

rtn_cleanup:
    DSTop = Stacksave;
rtn:
    alrm->obj = alrm_obj;
    return;
}

// RETURN ALARM INDEX
// RETURN 0 ON ERROR
// CALLER MUST CALL ScanAlarms
static BINT AddSysAlarm(struct alarm *new_alrm)
{
    BINT    id, nalarms, first_due_id, past_due_id;
    WORDPTR *Stacksave = DSTop;
    struct alarm cur_alrm;

    alrm_obj = new_alrm->obj;
    alarms = GetAlarms();
    if (!alarms) goto rtn_cleanup;

    nalarms = rplExplodeList2(alarms) - 2;
    new_alrm->obj = alrm_obj;
    PushSysAlarm(new_alrm);
    if (Exceptions) goto rtn_cleanup;
    nalarms++;

    // TODO : Replace this O(n) search algorithm
    for (id = 1; id < nalarms; id++) {
        if (!ReadSysAlarm(rplPeekData((nalarms - id) + 1), &cur_alrm))
            goto rtn_reset;
        if (new_alrm->time < cur_alrm.time)
            break;
    }

    rplNewBINTPush((nalarms - id) + 1, DECBINT);
    if (Exceptions) goto rtn_cleanup;
    rplCallOperator(CMD_ROLLD);

    // MAINTAIN IDS CONSISTENCY
    first_due_id = rplReadBINT(rplPeekData(nalarms + 2));
    past_due_id = rplReadBINT(rplPeekData(nalarms + 1));
    if (id <= first_due_id) first_due_id++;
    if (id <= past_due_id) past_due_id++;
    rplOverwriteData(nalarms + 2, rplNewBINT(first_due_id, DECBINT));
    rplOverwriteData(nalarms + 1, rplNewBINT(past_due_id, DECBINT));

    rplNewBINTPush(nalarms + 2, DECBINT);
    if (Exceptions) goto rtn_cleanup;
    rplCreateList();
    if (Exceptions) goto rtn_cleanup;
    rplStoreSettings((WORDPTR)alarms_ident, rplPeekData(1));
    rplDropData(1);

    goto rtn;

rtn_reset:
    ResetAlarms();
rtn_cleanup:
    id = 0;
    DSTop = Stacksave;
rtn:
    new_alrm->obj = alrm_obj;
    return id;
}

// RETURN 0 IF NONEXISTENT ALARM
static BINT GetSysAlarm(BINT id, struct alarm *alrm)
{
    BINT    nalarms;

    if (id < 1)
        return 0;

    alarms = GetAlarms();
    if (!alarms)
        return 0;

    nalarms = rplListLength(alarms) - 2;
    if (id > nalarms)
        return 0;

    if (!ReadSysAlarm(rplGetListElement(alarms, id + 2), alrm)) {
        ResetAlarms();
        return 0;
    }

    return 1;
}

// RETURN 0 IF NONEXISTENT ALARM
// CALLER MUST CALL ScanAlarms
static BINT DelSysAlarm(BINT id)
{
    WORDPTR *Stacksave = DSTop;
    BINT    nalarms, first_due_id, past_due_id;

    if (id < 1)
        return 0;

    alarms = GetAlarms();
    if (!alarms)
        return 0;

    nalarms = rplExplodeList2(alarms) - 2;
    if ((id > nalarms) || Exceptions)
        goto rtn_cleanup;

    rplNewBINTPush((nalarms - id) + 1, DECBINT);
    if (Exceptions) goto rtn_cleanup;
    rplCallOperator(CMD_ROLL);
    rplDropData(1);
    nalarms--;

    // MAINTAIN IDS CONSISTENCY
    first_due_id = rplReadBINT(rplPeekData(nalarms + 2));
    past_due_id = rplReadBINT(rplPeekData(nalarms + 1));
    if (id < first_due_id)
        first_due_id--;
    else if (id == first_due_id)
        first_due_id = (id > nalarms) ? nalarms : id;
    if (id < past_due_id)
        past_due_id--;
    else if (id == past_due_id)
        past_due_id = (id > nalarms) ? nalarms : id;
    rplOverwriteData(nalarms + 2, rplNewBINT(first_due_id, DECBINT));
    rplOverwriteData(nalarms + 1, rplNewBINT(past_due_id, DECBINT));

    rplNewBINTPush(nalarms + 2, DECBINT);
    if (Exceptions) goto rtn_cleanup;
    rplCreateList();
    if (Exceptions) goto rtn_cleanup;
    rplStoreSettings((WORDPTR)alarms_ident, rplPeekData(1));
    rplDropData(1);

    return 1;

rtn_cleanup:
    DSTop = Stacksave;
    return 0;
}

// RETURN THE ALARM INDEX OF THE FIRST ALARM DUE AFTER THE
// SPECIFIED TIME
// RETURN 0 IF NOT FOUND
static BINT FindNextAlarm(BINT64 time, struct alarm *alrm)
{
    BINT id = 0;

    while (GetSysAlarm(++id, alrm)) {
        if (alrm->time >= time)
            return id;
    }

    return 0;
}

// RETURN NEW ALARM ID
// RETURN 0 ON ERROR
// CALLER MUST CALL ScanAlarms
static BINT UpdateSysAlarm(BINT id, struct alarm *alrm)
{
    alrm_obj = alrm->obj;
    if (!DelSysAlarm(id)) {
        alrm->obj = alrm_obj;
        return 0;
    }

    alrm->obj = alrm_obj;
    return AddSysAlarm(alrm);
}

// REPLACE ALARM OF INDEX id WITH alrm
// DO NOT SORT ALARMS !!!
// CALLER MUST CALL ScanAlarms
// RETURN 0 IF NONEXISTENT ALARM OR ON ERROR
static BINT ReplaceSysAlarm(BINT id, struct alarm *alrm)
{
    WORDPTR *Stacksave = DSTop;;
    BINT nalarms;

    if (id < 1)
        return 0;

    alrm_obj = alrm->obj;
    alarms = GetAlarms();
    if (!alarms)
        return 0;

    nalarms = rplExplodeList(alarms) - 2;
    if ((id > nalarms) || Exceptions)
        goto rtn_cleanup;

    alrm->obj = alrm_obj;
    PushSysAlarm(alrm);
    if (Exceptions) goto rtn_cleanup;

    rplOverwriteData((nalarms - id) + 3, rplPeekData(1));
    rplDropData(1);

    rplCreateList();
    if (Exceptions) goto rtn_cleanup;
    rplStoreSettings((WORDPTR)alarms_ident, rplPeekData(1));
    rplDropData(1);

    alrm->obj = alrm_obj;
    return 1;

rtn_cleanup:
    alrm->obj = alrm_obj;
    DSTop = Stacksave;
    return 0;
}

static BINT GetFirstAlarmId(BINT type)
{
    WORDPTR bint;

    if (type < DUE_ALM || type > PASTDUE_ALM)
        return 0;

    alarms = GetAlarms();
    if (!alarms)
        return 0;

    bint = rplGetListElement(alarms, 1 + type);
    if (!ISBINT(*bint)) {
        ResetAlarms();
        return 0;
    }

    return (BINT)rplReadBINT(bint);
}

static void SetFirstAlarmId(BINT type, BINT id)
{
    WORDPTR *Stacksave = DSTop;;
    BINT    nitems;

    if (type < DUE_ALM || type > PASTDUE_ALM)
        return;

    alarms = GetAlarms();
    if (!alarms)
            return;

    nitems = rplExplodeList(alarms);
    if (Exceptions) goto rtn_cleanup;

    rplOverwriteData((nitems + 1) - type, rplNewBINT(id, DECBINT));
    rplCreateList();
    if (Exceptions) goto rtn_cleanup;
    rplStoreSettings((WORDPTR)alarms_ident, rplPeekData(1));
    rplDropData(1);

    return;

rtn_cleanup:
    DSTop = Stacksave;
    return;
}

static BINT GetFirstAlarm(BINT type, struct alarm *alrm)
{
    BINT id = GetFirstAlarmId(type);

    if (!id) id++;

    while (GetSysAlarm(id, alrm)) {
        if (alrm->flags == type) {
            alrm_obj = alrm->obj;
            SetFirstAlarmId(type, id);
            alrm->obj = alrm_obj;
            return id;
        }
        id++;
    }
    SetFirstAlarmId(type, 0);

    return 0;
}

// @alrm:(OPT) ALARM TO BE USED INSTEAD OF THE STORED ALARM
// @id: ID OF THE STORED ALARM TO BE RESCHEDULED
// RETURN NEW ALARM ID IF IT WAS RESCHEDULED
// RETURN 0 IF THE ALARM WAS NOT RESCHEDULED
// CALLER MUST CALL ScanAlarms
static BINT RescheduleAlarm(BINT id, struct alarm *alrm)
{
    struct alarm stored;

    if (!alrm) {
        if (!GetSysAlarm(id, &stored))
            return 0;
        alrm = &stored;
    }

    if (!alrm->rpt)
        return 0;

    if (!alrm->ack && rplTestSystemFlag(FL_RESRPTALRM))
        return 0;

    alrm->flags = DUE_ALM;

    alrm->time += (((GetSysTime() - alrm->time) / alrm->rpt) + 1) * alrm->rpt;


    return UpdateSysAlarm(id, alrm);
}

static void ScheduleAlarm(struct alarm *alrm)
{
    struct date dt;
    struct time tm;

    rplSecondsToDate(alrm->time, &dt, &tm);
    halSetSystemAlarm(dt, tm, 1);

    return;
}

static void ScanFirstDue()
{
    struct alarm alrm;

    if (GetFirstAlarm(DUE_ALM, &alrm)) {
        if (alrm.time <= GetSysTime())
            alrm.time = GetSysTime() + 1;

        ScheduleAlarm(&alrm);
    } else {
        halDisableSystemAlarm();
    }

    return;
}

static void ScanPastDue()
{
    struct alarm alrm;

    if (!GetFirstAlarm(PASTDUE_ALM, &alrm))
        halSetNotification(N_CONNECTION, 0x0);

    return;
}

static void ScanAlarms()
{
    ScanFirstDue();
    ScanPastDue();

    return;
}

static void WarnAlarm()
{
    WORDPTR msg;
    char    *msg_start,
            *msg_end;

    msg = uiGetLibMsg(ERR_PASTDUEALRM);
    msg_start = (char *)(msg + 1);
    msg_end = msg_start + rplStrSize(msg);

    halShowMsgN(msg_start, msg_end);

    // TODO : BEEP

    return;
}

// APPOINTMENT ALARM ANNOUNCEMENT
// RETURN 1 IF ANY KEY WAS PRESSED
static BINT AckSequence(struct alarm *alrm)
{
    BINT i, ack;
    char    *msg_start,
            *msg_end;

    msg_start = (char *)(alrm->obj + 1);
    msg_end = msg_start + rplStrSize(alrm->obj);
    halShowMsgN(msg_start, msg_end);

    // TODO : Display alarm date/time
    // TODO : Display alarm text
    // TODO : Implement BEEP and WAIT for key

    for (i = 0; i < 30; i++) {
        tmr_delayms(100);
        ack = keyb_anymsg();
        if (ack) {
            keyb_flush();
            break;
        }
    }

    return ack;
}

// RETURN 1 IF THERE IS A PAST DUE ALARM
BINT rplTriggerAlarm()
{
    struct alarm alrm;
    BINT    id, new_id;
    WORDPTR *Stacksave = DSTop;

    id = GetFirstAlarm(DUE_ALM, &alrm);
    if (!id)
        return GetFirstAlarmId(PASTDUE_ALM) ? 1 : 0;

    alrm_obj = alrm.obj;
    if (ISSTRING(*alrm_obj)) {

        // APPOINTMENT ALARM

        alrm.ann = 1;

        if (AckSequence(&alrm)) {
            alrm.ack = 1;
            alrm.ann = 0;

            if (!RescheduleAlarm(id, &alrm)) {
                if (rplTestSystemFlag(FL_SAVACKALRM))
                    ReplaceSysAlarm(id, &alrm);
                else
                    DelSysAlarm(id);
            }
        } else {
            if (!RescheduleAlarm(id, &alrm))
                ReplaceSysAlarm(id, &alrm);
        }

    } else {

        // CONTROL ALARM

        alrm.ack = 1;
        alrm.ann = 1;

        new_id = RescheduleAlarm(id, &alrm);
        if (!new_id)
            new_id = ReplaceSysAlarm(id, &alrm);

        rplNewBINTPush(new_id, DECBINT);
        rplPushData(alrm_obj);
        if (Exceptions)
            DSTop = Stacksave;
        else
            uiCmdRun(CMD_OVR_EVAL);

    }

    ScanAlarms();

    return GetFirstAlarmId(PASTDUE_ALM) ? 1 : 0;
}

// RETURN 1 IF THERE IS A PAST DUE ALARM
BINT rplCheckAlarms()
{
    BINT64 start_tm;
    struct alarm first_due;

    if (GetFirstAlarmId(PASTDUE_ALM))
        WarnAlarm();

    // MISSED ALARM ?
    if (GetFirstAlarmId(DUE_ALM)) {
        start_tm = GetSysTime() - ((halTicks() / 100000) + 1);
        if ((GetFirstAlarm(DUE_ALM, &first_due)) && (first_due.time < start_tm))
            rplTriggerAlarm();
    }

    return GetFirstAlarmId(PASTDUE_ALM) ? 1 : 0;
}

void rplUpdateAlarms()
{
    struct alarm alrm;
    BINT   id = 0;
    BINT64 now;

    now = GetSysTime();

    while (GetSysAlarm(++id, &alrm) && (alrm.time <= now)) {
        if (alrm.flags == DUE_ALM) {
            alrm.flags = DISABLED_ALM;
            ReplaceSysAlarm(id, &alrm);
        }
    }

    if (GetSysAlarm(id, &alrm)) {
        SetFirstAlarmId(DUE_ALM, id);

        while (GetSysAlarm(id, &alrm)) {
            alrm.flags = DUE_ALM;
            ReplaceSysAlarm(id++, &alrm);
        }
    } else {
        SetFirstAlarmId(DUE_ALM, 0);
    }

    SetFirstAlarmId(PASTDUE_ALM, 0);
    ScanPastDue();

    return;
}

void rplSkipNextAlarm()
{
    struct alarm first_due;
    BINT    id;
    WORDPTR msg;
    char    *msg_start,
            *msg_end;

    id = GetFirstAlarm(DUE_ALM, &first_due);
    if (id) {
        first_due.dis = 1;
        ReplaceSysAlarm(id, &first_due);
        ScanFirstDue();

        msg = uiGetLibMsg(ERR_ALRMSKIPPED);
        msg_start = (char *)(msg + 1);
        msg_end = msg_start + rplStrSize(msg);

        halShowMsgN(msg_start, msg_end);
    }

    return;
}


// HIGHER-LEVEL ALARMS INTERFACE

// RETURN ALARM INDEX
BINT rplAddAlarm(struct alarm *alrm)
{
    BINT id;

    if (alrm->time > GetSysTime())
        alrm->flags = DUE_ALM;
    else
        alrm->flags = DISABLED_ALM;

    if (alrm->obj == NULL)
        alrm->obj = rplCreateString(0 ,0);

    id = AddSysAlarm(alrm);

    ScanFirstDue();

    return id;
}

// RETURN 0 IF NONEXISTENT ALARM
BINT rplGetAlarm(BINT id, struct alarm *alrm)
{
    return GetSysAlarm(id, alrm);
}

// IF id=0, PURGE ALL ALARMS
// RETURN 0 IF NONEXISTENT ALARM
BINT rplDelAlarm(BINT id)
{
    if (!id) {
        InitAlarms();
        return 1;
    }

    if (!DelSysAlarm(id))
        return 0;

    ScanAlarms();

    return 1;
}

// READ OBJECT AS ALARM
// RETURN 1 ON SUCCESS
// RETURN 0 ON ERROR AND SET ERROR NUMBER
BINT rplReadAlarm(WORDPTR obj, struct alarm *alrm)
{
    struct date  dt, sys_dt;
    struct time  tm;
    WORDPTR      alarm_dt = NULL,
                 alarm_tm = NULL,
                 alarm_obj = NULL,
                 alarm_rpt = NULL;
    BINT         lst_sz;
    BINT64       b_alarm_rpt;
    REAL         r_dt, r_tm;

    // EXTRACT ALARM DATA

    if (ISNUMBER(*obj)) {

        alarm_tm = obj;

    } else {

        if (!ISLIST(*obj)) {
            rplError(ERR_BADARGTYPE);
            return 0;
        }

        lst_sz = rplListLength(obj);

        if (lst_sz < 2 || lst_sz > 4) {
            rplError(ERR_INVALIDLISTSIZE);
            return 0;
        }

        switch (lst_sz) {
        case 4:
            alarm_rpt = rplGetListElement(obj, 4);
        case 3:
            alarm_obj = rplGetListElement(obj, 3);
        case 2:
            alarm_tm = rplGetListElement(obj, 2);
            alarm_dt = rplGetListElement(obj, 1);
        }

    }

    // CHECK AND READ ALARM DATA

    // 1 - ALARM TIME
    if (!ISNUMBER(*alarm_tm)) {
        rplError(ERR_BADARGTYPE);
        return 0;
    }

    rplReadNumberAsReal(alarm_tm, &r_tm);
    if (!rplReadRealAsTime(&r_tm, &tm)) {
        rplError(ERR_INVALIDTIME);
        return 0;
    }

    // 2 - ALARM DATE
    if (alarm_dt == NULL) {

        dt = halGetSystemDate();

    } else {

        if (!ISREAL(*alarm_dt)) {
            rplError(ERR_BADARGTYPE);
            return 0;
        }

        rplReadNumberAsReal(alarm_dt, &r_dt);
        if (!rplReadRealAsDateNoCk(&r_dt, &dt)) {
            rplError(ERR_INVALIDDATE);
            return 0;
        }

        if (!dt.year) {
            sys_dt = halGetSystemDate();
            dt.year = sys_dt.year;
        }

        if (!rplIsValidDate(dt)) {
            rplError(ERR_INVALIDDATE);
            return 0;
        }

    }

    alrm->time = rplDateToSeconds(dt, tm);

    // 3 - ALARM OBJECT
    alrm->obj = alarm_obj;

    // 4 - ALARM REPEAT
    if (alarm_rpt == NULL) {

        alrm->rpt = 0;

    } else {

        if (!ISNUMBER(*alarm_rpt)) {
            rplError(ERR_BADARGTYPE);
            return 0;
        }

        b_alarm_rpt = rplReadNumberAsBINT(alarm_rpt);
        if ((b_alarm_rpt < 0) || (b_alarm_rpt >= (1LL << 32))) {
            rplError(ERR_INVALIDRPT);
            return 0;
        }
        alrm->rpt = (UBINT)b_alarm_rpt;
    }

    return 1;
}

// USES RREG 0
void rplPushAlarm(struct alarm *alrm)
{
    WORDPTR *Stacksave = DSTop;
    struct date dt;
    struct time tm;

    alrm_obj = alrm->obj;

    rplSecondsToDate(alrm->time, &dt, &tm);
    rplReadDateAsReal(dt, &RReg[0]);
    rplNewRealFromRRegPush(0);

    rplReadTimeAsReal(tm, &RReg[0]);
    rplNewRealFromRRegPush(0);

    rplPushData(alrm_obj);

    rplNewBINTPush(alrm->rpt, DECBINT);

    rplNewBINTPush(4, DECBINT);
    if (Exceptions) goto rtn_cleanup;

    rplCreateList();
    if (Exceptions) goto rtn_cleanup;

    goto rtn;

rtn_cleanup:
    DSTop = Stacksave;
rtn:
    alrm->obj = alrm_obj;
    return;
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
        struct date dt, sys_dt;
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
        if (!rplReadRealAsDateNoCk(&r_date, &dt)) {
            rplError(ERR_INVALIDDATE);
            return;
        }

        if (!dt.year) {
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
        if (!rplReadRealAsDate(&r_date, &dt)) {
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
        if (!rplReadRealAsDate(&r_date, &dt)) {
            rplError(ERR_INVALIDDATE);
            return;
        }

        ddays = rplDateToDays(dt);

        rplReadReal(arg_date2, &r_date);
        if (!rplReadRealAsDate(&r_date, &dt)) {
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
        if (!rplReadRealAsTime(&r_time, &tm)) {
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

        if (isintegerReal(&r_dec))
            return;

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

        if (isintegerReal(&r_hms))
            return;

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
    case ACK:
    {
        struct alarm past_due;
        BINT past_due_id;

        past_due_id = GetFirstAlarm(PASTDUE_ALM, &past_due);
        if (!past_due_id)
            return;

        if (past_due.rpt) {
            RescheduleAlarm(past_due_id, &past_due);
            ScanAlarms();
            return;
        }

        if (rplTestSystemFlag(FL_SAVACKALRM)) {
            past_due.flags = PAST_ALM;
            ReplaceSysAlarm(past_due_id, &past_due);
            ScanPastDue();
            return;
        }

        rplDelAlarm(past_due_id);

        return;
    }
    case ACKALL:
    {
        while (GetFirstAlarmId(PASTDUE_ALM))
            rplCallOperator(CMD_ACK);

        return;
    }
    case RCLALARM:
    {
        struct alarm alrm;
        WORDPTR arg_id;
        BINT    id;

        if (rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        arg_id = rplPeekData(1);
        if (!ISNUMBER(*arg_id)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        id = rplReadNumberAsBINT(arg_id);

        if (!rplGetAlarm(id, &alrm)) {
            rplError(ERR_BADALARMNUM);
            return;
        }

        rplDropData(1);
        rplPushAlarm(&alrm);

        return;
    }
    case STOALARM:
    {
        WORDPTR      arg_alarm;
        BINT         alarm_id;
        struct alarm alrm;

        if (rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        arg_alarm = rplPeekData(1);
        if (!rplReadAlarm(arg_alarm, &alrm))
            return;

        alarm_id = rplAddAlarm(&alrm);

        rplDropData(1);
        rplNewBINTPush(alarm_id, DECBINT);

        return;
    }
    case DELALARM:
    {
        WORDPTR arg_id;
        BINT    id;

        if (rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        arg_id = rplPeekData(1);
        if (!ISNUMBER(*arg_id)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        id = rplReadNumberAsBINT(arg_id);

        if (!rplDelAlarm(id)) {
            rplError(ERR_BADALARMNUM);
            return;
        }

        rplDropData(1);

        return;
    }
    case FINDALARM:
    {
        struct date dt, sys_dt;
        struct time tm;
        struct alarm alrm;
        REAL    r_dt, r_tm;
        WORDPTR arg, arg_tm, arg_dt;
        BINT64  sec;

        if (rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        arg = rplPeekData(1);

        if (ISNUMBER(*arg)) {
            arg_dt = arg;
            if (ISREAL(*arg)) {
                tm.hour = 12;
                tm.min = 0;
                tm.sec = 0;
            } else {
                rplReadNumberAsReal(arg_dt, &r_dt);
                if (iszeroReal(&r_dt)) {
                    rplDropData(1);
                    rplNewBINTPush(GetFirstAlarmId(PASTDUE_ALM), DECBINT);
                    return;
                }
            }
        } else {
            if (!ISLIST(*arg)) {
                rplError(ERR_BADARGTYPE);
                return;
            }

            if (rplListLength(arg) != 2) {
                rplError(ERR_INVALIDLISTSIZE);
                return;
            }

            arg_tm = rplGetListElement(arg, 2);

            if (!ISNUMBER(*arg_tm)) {
                rplError(ERR_BADARGTYPE);
                return;
            }

            rplReadNumberAsReal(arg_tm, &r_tm);
            if (!rplReadRealAsTime(&r_tm, &tm)) {
                rplError(ERR_INVALIDTIME);
                return;
            }

            arg_dt = rplGetListElement(arg, 1);
        }

        if (!ISREAL(*arg_dt)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        rplReadReal(arg_dt, &r_dt);
        if (!rplReadRealAsDateNoCk(&r_dt, &dt)) {
            rplError(ERR_INVALIDDATE);
            return;
        }

        if (!dt.year) {
            sys_dt = halGetSystemDate();
            dt.year = sys_dt.year;
        }

        if (!rplIsValidDate(dt)) {
            rplError(ERR_INVALIDDATE);
            return;
        }

        rplDropData(1);
        sec = rplDateToSeconds(dt, tm);
        rplNewBINTPush(FindNextAlarm(sec, &alrm), DECBINT);

        return;
    }
    case DOALARM:
    {
        BINT id;
        char *msg_start,
             *msg_end;
        struct alarm alrm;

        id = GetFirstAlarmId(PASTDUE_ALM);

        if (!id)
            return;
        if (!GetSysAlarm(id, &alrm))
            return;

        alrm_obj = alrm.obj;
        rplNewBINTPush(id, DECBINT);

        msg_start = (char *)(alrm_obj + 1);
        msg_end = msg_start + rplStrSize(alrm_obj);
        halShowMsgN(msg_start, msg_end);

        return;
    }
    case ALRM: // ONLY FOR TESTS
    {


        return;
    }
    case MEM:
    {
        rplGCollect();
        rplNewBINTPush((BINT64)rplGetFreeMemory(), DECBINT);

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
    case VERSION:
    {
        rplPushData((WORDPTR)newrpl_version);
        rplCallOvrOperator(CMD_OVR_EVAL);

        return;
    }
    case GARBAGE:
        rplGCollect();
        return;
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
        // THIS OPCODE RECEIVES A POINTER TO AN RPL COMMAND OR OBJECT IN ObjectPTR
        // NEEDS TO RETURN INFORMATION ABOUT THE TYPE:
        // IN RetNum: RETURN THE MKTOKENINFO() DATA FOR THE SYMBOLIC COMPILER AND CAS
        // IN DecompHints: RETURN SOME HINTS FOR THE DECOMPILER TO DO CODE BEAUTIFICATION (TO BE DETERMINED)
        // IN TypeInfo: RETURN TYPE INFORMATION FOR THE TYPE COMMAND
        //             TypeInfo: TTTTFF WHERE TTTT = MAIN TYPE * 100 (NORMALLY THE MAIN LIBRARY NUMBER)
        //                                FF = 2 DECIMAL DIGITS FOR THE SUBTYPE OR FLAGS (VARIES DEPENDING ON LIBRARY)
        //             THE TYPE COMMAND WILL RETURN A REAL NUMBER TypeInfo/100
        // FOR NUMBERS: TYPE=10 (REALS), SUBTYPES = .01 = APPROX., .02 = INTEGER, .03 = APPROX. INTEGER
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL BINT, .42 = HEX INTEGER
        if(ISPROLOG(*ObjectPTR)) {
        TypeInfo=LIBRARY_NUMBER*100;
        DecompHints=0;
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_NOTALLOWED,0,1);
        }
        else {
            TypeInfo=0;     // ALL COMMANDS ARE TYPE 0
            DecompHints=0;
            libGetInfo2(*ObjectPTR,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        }
        return;



    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER,(WORDPTR *)ROMPTR_TABLE,ObjectPTR);

        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID);
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
        if(MENUNUMBER(MenuCodeArg)>2) {
            RetNum=ERR_NOTMINE;
            return;
        }
        ObjectPTR=(WORDPTR)ROMPTR_TABLE[MENUNUMBER(MenuCodeArg) + 2];
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
