/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// THIS LIBRARY PROVIDES COMPILATION OF STACK RELATED COMMANDS AND OTHER BASIC COMMANDS


// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  24
#define LIB_ENUM lib24enum
#define LIB_NAMES lib24_names
#define LIB_HANDLER lib24_handler
#define LIB_NUMBEROFCMDS LIB24_NUMBEROFCMDS

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(GARBAGE), \
    CMD(CLEAR), \
    CMD(DEPTH), \
    CMD(DROP), \
    CMD(DROP2), \
    CMD(DROPN), \
    CMD(DUP), \
    CMD(DUP2), \
    CMD(DUPDUP), \
    CMD(DUPN), \
    CMD(NDUPN), \
    CMD(NIP), \
    CMD(OVER), \
    CMD(PICK), \
    CMD(PICK3), \
    CMD(ROLL), \
    CMD(ROLLD), \
    CMD(ROT), \
    CMD(SWAP), \
    CMD(UNPICK), \
    CMD(UNROT)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY

#define CMD_EXTRANAME \
    ""
#define CMD_EXTRAENUM \
    UNPROTECTSTACK



// INTERNAL DECLARATIONS

// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST , CMD_EXTRAENUM , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
const char * const LIB_NAMES[]= { CMD_LIST , CMD_EXTRANAME  };
#undef CMD


const WORD const unprotect_seco[]={
    MKPROLOG(DOCOL,2),
    MKOPCODE(LIBRARY_NUMBER,UNPROTECTSTACK),
    CMD_SEMI
};




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
    case GARBAGE:
        rplGCollect();
        return;
    case CLEAR:
        // ONLY CLEAR UP TO THE STACK PROTECTED AREA
        // DON'T THROW AN ERROR
        DSTop=DStkProtect;
        return;
    case DEPTH:
        rplNewBINTPush(rplDepthData(),DECBINT);
        return;
    case DROP:
        rplDropData(1);
        return;
    case DROP2:
        rplDropData(2);
        return;
    case DROPN:
    {
        if(rplDepthData()<1) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        BINT64 count=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;
        if(count<0 || rplDepthData()<count+1) {
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }
        rplDropData(count+1);
        return;
    }
    case DUP:
        if(rplDepthData()<1) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        rplPushData(rplPeekData(1));
        return;
    case DUP2:
        if(rplDepthData()<2) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        rplPushData(rplPeekData(2));
        rplPushData(rplPeekData(2));
        return;
    case DUPDUP:
        if(rplDepthData()<1) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        rplPushData(rplPeekData(1));
        rplPushData(rplPeekData(1));
        return;
    case DUPN:
    {
        if(rplDepthData()<1) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        BINT64 count=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;
        if(count<0 || rplDepthData()<count+1) {
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }
        rplDropData(1);
        BINT64 f;
        for(f=0;f<count;++f) {
            rplPushData(rplPeekData(count));
            if(Exceptions) return;
        }
        return;
    }
    case NDUPN:
    {
        if(rplDepthData()<2) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        BINT64 count=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;
        if(count<0) {
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }
        rplDropData(1);
        BINT64 f;
        for(f=1;f<count;++f) {
            rplPushData(rplPeekData(1));
            if(Exceptions) return;
        }
        rplNewBINTPush(count,DECBINT);
        return;
    }
    case NIP:
        if(rplDepthData()<2) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        rplOverwriteData(2,rplPeekData(1));
        rplDropData(1);
        return;

    case OVER:
        if(rplDepthData()<2) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        rplPushData(rplPeekData(2));
        return;
    case PICK:
    {

        if(rplDepthData()<1) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }

        BINT64 level=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;
        if( (level<1) || (rplDepthData()<1+level)) {
         Exceptions|=EX_BADARGVALUE;
         ExceptionPointer=IPtr;
         return;
        }

        rplOverwriteData(1,rplPeekData(1+level));

        return;
    }
    case PICK3:
        if(rplDepthData()<3) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        rplPushData(rplPeekData(3));
        return;
    case ROLL:
    {
        if(rplDepthData()<1) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }

        BINT64 level=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;

        if( (level<1) || (rplDepthData()<1+level)) {
         Exceptions|=EX_BADARGVALUE;
         ExceptionPointer=IPtr;
         return;
        }

        rplDropData(1);

        WORDPTR objn=rplPeekData(level);
        WORDPTR *stkptr=DSTop-level;

        BINT64 count;
        for(count=1;count<level;++count,++stkptr) *stkptr=*(stkptr+1);

        rplOverwriteData(1,objn);

        return;
     }
    case ROLLD:
    {
        if(rplDepthData()<1) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }

        BINT64 level=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;

        if( (level<1) || (rplDepthData()<1+level)) {
         Exceptions|=EX_BADARGVALUE;
         ExceptionPointer=IPtr;
         return;
        }

        rplDropData(1);

        WORDPTR objn=rplPeekData(1);
        WORDPTR *stkptr=DSTop-1;

        BINT64 count;
        for(count=1;count<level;++count,--stkptr) *stkptr=*(stkptr-1);

        rplOverwriteData(level,objn);

        return;
     }
    case ROT:
    {
        if(rplDepthData()<3) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        WORDPTR obj1=rplPeekData(1);
        rplOverwriteData(1,rplPeekData(3));
        rplOverwriteData(3,rplPeekData(2));
        rplOverwriteData(2,obj1);
        return;
    }

    case SWAP:
    {
        if(rplDepthData()<2) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        WORDPTR obj=rplPeekData(1);
        rplOverwriteData(1,rplPeekData(2));
        rplOverwriteData(2,obj);
        return;
    }

    case UNPICK:
    {

        if(rplDepthData()<2) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }

        BINT64 level=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;

        if( (level<1) || (rplDepthData()<2+level)) {
         Exceptions|=EX_BADARGVALUE;
         ExceptionPointer=IPtr;
         return;
        }

        WORDPTR obj=rplPeekData(2);
        rplDropData(2);

        rplOverwriteData(level,obj);

        return;
    }

    case UNROT:
    {

        if(rplDepthData()<3) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        WORDPTR obj1=rplPeekData(1);
        rplOverwriteData(1,rplPeekData(2));
        rplOverwriteData(2,rplPeekData(3));
        rplOverwriteData(3,obj1);
        return;
    }


    case UNPROTECTSTACK:
    {
        // THIS INTERNAL OPCODE PROVIDES SAFETY GUARD AGAINST DATA STACK PROTECTION
        // IF A PROGRAM FORGETS TO UNPROTECT THE STACK, IT WILL BE UNPROTECTED
        // AUTOMATICALLY ON EXIT
        WORDPTR *oldstack=(WORDPTR *)rplPopRet();
        DStkProtect=oldstack;
        return;
    }

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
        // DecompileObject = Ptr to prolog of object to decompile
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
        RetNum=LIBRARY_NUMBER;
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








