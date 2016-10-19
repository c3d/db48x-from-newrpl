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
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(TICKS,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(DATE,MKTOKENINFO(4,TITYPE_FUNCTION,0,2)), \
    ECMD(SETDATE,"→DATE",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DATEADD,"DATE+",MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(DDAYS,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(TIME,MKTOKENINFO(4,TITYPE_FUNCTION,0,2)), \
    ECMD(SETTIME,"→TIME",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(MEM,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(MEMCHECK,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(MEMFIX,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(READCFI,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(PEEK,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(POKE,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2))
//    ECMD(CMDNAME,"CMDNAME",MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************


// RETURN RPL'S FREE RAM MEMORY IN BYTES
BINT rplGetFreeMemory()
{
    BINT mem = 0;

    mem += (DSTop - DStk) & 0x3ff;
    mem += (RSTop - RStk) & 0x3ff;
    mem += (LAMs - LAMTop) & 0x3ff;
    mem += (DirsTop - Directories) & 0x3ff;
    mem += (TempBlocksEnd - TempBlocks) & 0x3ff;
    mem += (TempObEnd - TempOb) & 0x3ff;

    mem = ((6 * 1024) - mem) << 2;
    mem += halGetFreePages() << 12;

    return mem;
}

// EXPLODE A REAL REPRESENTING A DATE INTO BINTS
// ACCORDING TO THE SYSTEM DATE FORMAT FLAG.
// USES RREG 0
void explode_date(BINT *day, BINT *month, BINT *year, REAL *date)
{
    rplReadNumberAsReal((WORDPTR)date, &RReg[0]);
    RReg[0].exp += 6;

    if (inBINTRange(&RReg[0])) {
        *year = getBINTReal(&RReg[0]);
        *month = *year / 1000000;
        *year -= *month * 1000000;
        *day = *year / 10000;
        *year -= *day * 10000;

        if (rplTestSystemFlag(FL_DATEFORMAT)) {
            BINT swap = *month;
            *month = *day;
            *day = swap;
        }
    }

    return;
}
/*
// BUILD A REAL REPRESENTING A DATE FROM BINTS
// ACCORDING TO THE SYSTEM DATE FORMAT FLAG
// USES RREG 0
void build_date(BINT d, BINT m, BINT y, REAL *date)
{
    BINT _date;

    _date = y;
    if (rplTestSystemFlag(FL_DATEFORMAT)) {
        _date += m * 10000;
        _date += d * 1000000;
    } else {
        _date += d * 10000;
        _date += m * 1000000;
    }

    return;
}
*/

// EXPLODE A REAL REPRESENTING A TIME IN HH.MMSS FORM, INTO BINTS
// USES RREG 0
void explode_time(BINT *hr, BINT *mn, BINT *sec, REAL *time)
{
    rplReadNumberAsReal((WORDPTR)time, &RReg[0]);
    RReg[0].exp += 4;

    if (inBINTRange(&RReg[0])) {
        *sec = getBINTReal(&RReg[0]);
        *hr = *sec / 10000;
        *sec -= *hr * 10000;
        *mn = *sec / 100;
        *sec -= *mn * 100;
    }

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
        BINT date, day, month, year, tmp;

        halGetSystemDate(&day, &month, &year, &tmp);

        date = year;
        if (rplTestSystemFlag(FL_DATEFORMAT)) {
            tmp = month;
            month = day;
            day = tmp;
        }
        date += day * 10000;
        date += month * 1000000;

        newRealFromBINT(&RReg[0], date, -6);
        rplNewRealPush(&RReg[0]);

        return;
    }
    case SETDATE:
    {
        BINT day, month, year;
        WORDPTR arg_date;

        if (rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        arg_date = rplPeekData(1);

        if(!ISREAL(*arg_date)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        explode_date(&day, &month, &year, (REAL *)arg_date);

        if (!year) {
            BINT tmp, tmp2, tmp3;
            halGetSystemDate(&tmp, &tmp2, &year, &tmp3);
        }

        if (rplTestSystemFlag(FL_DATEFORMAT)) {
            BINT swap = month;
            month = day;
            day = swap;
        }

        halSetSystemDate(day, month, year);
        rplDropData(1);

        return;
    }/*
    case DATEADD:
    {
        WORDPTR arg_date, arg_days;
        BINT day, month, year, days;

        if (rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        arg_days = rplPeekData(1);
        arg_date = rplPeekData(2);

        if(!ISREAL(*arg_days) || !ISREAL(*arg_date)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        date_explode(&day, &month, &year, (REAL *)arg_date);

        // NEED 'RND' COMMAND TO KEEP COMPATIBILITY
        // FOR NOW, WE TRUNCATE THE NUMBER OF DAYS
        days = getBINTReal((REAL *)arg_days);

        // 2:Date 1:%
        // RND
        // DATE+
        return;
    }*/
    case TIME:
    {
        BINT time, hr, mn, sec;

        halGetSystemTime(&hr, &mn, &sec);

        time = sec;
        time += mn * 100;
        time += hr * 10000;

        newRealFromBINT(&RReg[0], time, -4);
        rplNewRealPush(&RReg[0]);

        return;
    }
    case SETTIME:
    {
        BINT hr, mn, sec;

        if (rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISREAL(*rplPeekData(1))) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        explode_time(&hr, &mn, &sec, (REAL *)rplPeekData(1));
        halSetSystemTime(hr, mn, sec);
        rplDropData(1);

        return;
    }
    case MEM:
    {
        rplGCollect();
        rplNewBINTPush((BINT64)rplGetFreeMemory(), DECBINT);

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
