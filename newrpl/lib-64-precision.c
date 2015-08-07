/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
// LIB 64 PROVIDES COMMANDS THAT DEAL WITH ROUNDING AND PRECISION

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  64
#define LIB_ENUM lib64_enum
#define LIB_NAMES lib64_names
#define LIB_HANDLER lib64_handler
#define LIB_NUMBEROFCMDS LIB64_NUMBEROFCMDS

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(SETPREC), \
    CMD(FLOOR), \
    CMD(CEIL), \
    CMD(IP), \
    CMD(FP)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
/*#define CMD_EXTRANAME \
    "VOID",

#define CMD_EXTRAENUM \
    VOID
*/

// INTERNAL DECLARATIONS


// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST /*, CMD_EXTRAENUM */, LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
const char * const LIB_NAMES[]= { CMD_LIST /*, CMD_EXTRANAME*/ };
#undef CMD


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case SETPREC:
    {
        // TAKE AN INTEGER NUMBER FROM THE STACK
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        BINT64 number=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;
        if(number<9) number=9;
        if(number>REAL_PRECISION_MAX) number=REAL_PRECISION_MAX;
        mpd_qsetprec(&Context,number);
        rplDropData(1);
        return;
    }

    case FLOOR:
        {
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        REAL rnum;
        if(ISBINT(*arg)) return;
        rplReadNumberAsReal(rplPeekData(1),&rnum);
        if(Exceptions) return;
        mpd_floor(&RReg[1],&rnum,&Context);
        if(Exceptions) return;
        rplDropData(1);
        rplNewRealFromRRegPush(1);
        return;
        }

    case CEIL:
        {
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if(ISBINT(*arg)) return;
        REAL rnum;
        rplReadNumberAsReal(rplPeekData(1),&rnum);
        if(Exceptions) return;
        mpd_ceil(&RReg[1],&rnum,&Context);
        if(Exceptions) return;
        rplDropData(1);
        rplNewRealFromRRegPush(1);
        return;
        }

    case IP:
    {
    if(rplDepthData()<1) {
        Exceptions|=EX_BADARGCOUNT;
        ExceptionPointer=IPtr;
        return;
    }
    WORDPTR arg=rplPeekData(1);
    if(ISBINT(*arg)) return;
    REAL rnum;
    rplReadNumberAsReal(arg,&rnum);
    if(Exceptions) return;
    REALrunc(&RReg[1],&rnum,&Context);
    if(Exceptions) return;
    rplDropData(1);
    rplNewRealFromRRegPush(1);
    return;
    }

    case FP:
    {
    if(rplDepthData()<1) {
        Exceptions|=EX_BADARGCOUNT;
        ExceptionPointer=IPtr;
        return;
    }
    WORDPTR arg=rplPeekData(1);
    if(ISBINT(*arg)) {
        rplDropData(1);
        rplPushData((WORDPTR)zero_bint);
        return;
    }
    REAL rnum;
    rplReadNumberAsReal(arg,&rnum);
    if(Exceptions) return;
    REALrunc(&RReg[1],&rnum,&Context);
    if(Exceptions) return;
    mpd_sub(&RReg[2],&rnum,&RReg[1],&Context);
    if(Exceptions) return;
    rplDropData(1);
    rplNewRealFromRRegPush(2);
    return;
    }


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
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;


}




