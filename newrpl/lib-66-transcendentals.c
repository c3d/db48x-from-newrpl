/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
// LIB 66 PROVIDES COMMANDS THAT DEAL WITH TRANSCENDENTAL FUNCTIONS

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  66
#define LIB_ENUM lib66_enum
#define LIB_NAMES lib66_names
#define LIB_HANDLER lib66_handler
#define LIB_TOKENINFO lib66_tokeninfo
#define LIB_NUMBEROFCMDS LIB66_NUMBEROFCMDS

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(SIN,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(COS,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(TAN,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(ASIN,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(ACOS,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(ATAN,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(ATAN2,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(LN,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(EXP,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(SINH,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(COSH,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(TANH,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(ASINH,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(ACOSH,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(ATANH,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(DEG,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(GRAD,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(RAD,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2))


// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
#define CMD_EXTRANAME \
    "√",

#define CMD_EXTRAENUM \
    SQRT

#define CMD_EXTRAINFO \
    MKTOKENINFO(1,TITYPE_PREFIXOP,1,3)

// INTERNAL DECLARATIONS


// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a,b) a
enum LIB_ENUM { CMD_LIST , CMD_EXTRAENUM , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a,b) #a
const char * const LIB_NAMES[]= { CMD_LIST , CMD_EXTRANAME };
#undef CMD

#define CMD(a,b) b
const BINT const LIB_TOKENINFO[]=
{
        CMD_LIST ,
        CMD_EXTRAINFO
};


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_INVALIDOBJECT);
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case SIN:
    {
        REAL dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }

        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }


        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        // PRECONVERT ARGUMENT IF ITS IN ANYTHING OTHER THAN RADIANS

        if(intdigitsReal(&dec)>MAX_USERPRECISION) {
            // TODO: NOT REALLY OVERFLOW, BUT ARGUMENT TOO BIG
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }



        trig_sincos(&dec);

        finalize(&RReg[7]);

        rplDropData(1);
        rplNewRealFromRRegPush(7);       // SIN
        return;

    }
    case COS:
    {
        REAL dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }


        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        trig_sincos(&dec);

        finalize(&RReg[6]);
        rplDropData(1);
        rplNewRealFromRRegPush(6);       // COS
        return;

    }
    case TAN:
    {
        REAL dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        trig_sincos(&dec);
        normalize(&RReg[6]);
        normalize(&RReg[7]);
        if(iszeroReal(&RReg[6])) {
            rplInfinityToRReg(2);
            RReg[2].flags^=RReg[7].flags&F_NEGATIVE;
        }
        else {
            divReal(&RReg[2],&RReg[7],&RReg[6]);
        }

        rplDropData(1);
        rplNewRealFromRRegPush(2);       // SIN/COS

        return;

    }
    case ASIN:
    {
        REAL y;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&y);

        if(Exceptions) return;
        // WARNING: TRANSCENDENTAL FUNCTIONS OVERWRITE ALL RREGS. INITIAL ARGUMENTS ARE PASSED ON RREG 0, 1 AND 2, SO USING 7 IS SAFE.
        rplOneToRReg(7);
        BINT signy=y.flags&F_NEGATIVE;
        y.flags^=signy;

        if(gtReal(&y,&RReg[7])) {
            // TODO: INCLUDE COMPLEX ARGUMENTS HERE
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }
        y.flags^=signy;
        trig_asin(&y);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);       // RESULTING ANGLE

        return;

    }

    case ACOS:
    {
        REAL y;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&y);

        if(Exceptions) return;
        // WARNING: TRANSCENDENTAL FUNCTIONS OVERWRITE ALL RREGS. INITIAL ARGUMENTS ARE PASSED ON RREG 0, 1 AND 2, SO USING 7 IS SAFE.
        rplOneToRReg(7);
        BINT signy=y.flags&F_NEGATIVE;
        y.flags^=signy;
        if(gtReal(&y,&RReg[7])) {
            // TODO: INCLUDE COMPLEX ARGUMENTS HERE
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }
        y.flags^=signy;

        trig_acos(&y);
        normalize(&RReg[0]);
        // FIRST ELIMINATE BAD DIGITS SO ACOS(1)=0 INSTEAD OF SMALL NUMBER
        truncReal(&RReg[0],&RReg[0],Context.precdigits+6);
        round_real(&RReg[0],Context.precdigits,0);

        rplDropData(1);
        rplNewRealFromRRegPush(0);       // RESULTING ANGLE

        return;

    }



    case ATAN:
        // CALCULATE ATAN FROM ATAN2(Y,1)
    {
        REAL y;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&y);

        if(Exceptions) return;
        // WARNING: TRANSCENDENTAL FUNCTIONS OVERWRITE ALL RREGS. INITIAL ARGUMENTS ARE PASSED ON RREG 0, 1 AND 2, SO USING 7 IS SAFE.
        rplOneToRReg(7);


        trig_atan2(&y,&RReg[7]);
        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);       // RESULTING ANGLE

        return;

    }
    case ATAN2:
        // CALCULATE ATAN IN THE RANGE -PI,+PI
    {
        REAL y,x;
        if(rplDepthData()<2) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,2);
            return;
        }

        rplReadNumberAsReal(rplPeekData(2),&y);
        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        trig_atan2(&y,&x);

        finalize(&RReg[0]);

        rplDropData(2);
        rplNewRealFromRRegPush(0);       // RESULTING ANGLE

        return;

    }

    case LN:
    {
        REAL x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        if(iszeroReal(&x)) {
            // RETURN -INFINITY AND SET OVERFLOW
            // TODO: IMPLEMENT FLAGS TO AVOID THROWING AN ERROR
            rplInfinityToRReg(0);
            Exceptions|=EX_MATHOVERFLOW;
            ExceptionPointer=IPtr;
            return;
        }

        if(x.flags&F_NEGATIVE) {
            // TODO: RETURN COMPLEX VALUE!
            // FOR NOW JUST THROW AN EXCEPTION
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }

        hyp_ln(&x);
        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }

    case EXP:
    {
        REAL dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }

        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_exp(&dec);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);       // EXP
        return;


    }
    case SINH:
    {
        REAL dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_sinhcosh(&dec);

        finalize(&RReg[2]);

        rplDropData(1);
        rplNewRealFromRRegPush(2);       // SINH
        return;

    }

    case COSH:
    {
        REAL dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_sinhcosh(&dec);
        finalize(&RReg[1]);

        rplDropData(1);
        rplNewRealFromRRegPush(1);       // COSH
        return;

    }

    case TANH:

    {
        REAL dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&dec);
        if(Exceptions) return;

        hyp_sinhcosh(&dec);

        normalize(&RReg[2]);
        normalize(&RReg[1]);

        // TANH=SINH/COSH
        divReal(&RReg[0],&RReg[2],&RReg[1]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);       // TANH
        return;

    }


    case ASINH:
    {
        REAL x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }


        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        hyp_asinh(&x);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }

    return;



    case ACOSH:
    {
        REAL x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        rplOneToRReg(0);

        if(gtReal(&RReg[0],&x)) {
            // TODO: EXPAND THIS TO RETURN COMPLEX VALUES
           Exceptions|=EX_BADARGVALUE;
           ExceptionPointer=IPtr;
           return;
        }

        hyp_acosh(&x);
        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }

    case ATANH:
    {
        REAL x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        rplOneToRReg(0);
        BINT signx=x.flags;
        x.flags&=~F_NEGATIVE;
        BINT ismorethan1=cmpReal(&x,&RReg[0]);
        x.flags=signx;

        if(ismorethan1==1) {    // x > 1.0
            // TODO: CHANGE THIS ERROR INTO COMPLEX RESULTS!
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;
        }
        if(ismorethan1==0)
        {
            // TODO: IMPLEMENT INFINITY FLAGS TO THROW EXCEPTION ON INFINITY
            rplInfinityToRReg(0);
            if(signx&F_NEGATIVE) RReg[0].flags|=F_NEGATIVE;

            rplDropData(1);
            rplNewRealFromRRegPush(0);
            return;
        }

        hyp_atanh(&x);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }


    return;


    case SQRT:
    {
        REAL x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        if(x.flags&F_NEGATIVE) {
            // TODO: EXPAND THIS TO RETURN COMPLEX VALUES
           Exceptions|=EX_BADARGVALUE;
           ExceptionPointer=IPtr;
           return;
        }

        hyp_sqrt(&x);
        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }

    case DEG:
        rplClrSystemFlag(-17);
        rplClrSystemFlag(-18);
        return;
    case RAD:
        rplSetSystemFlag(-17);
        rplClrSystemFlag(-18);
        return;
    case GRAD:
        rplClrSystemFlag(-17);
        rplSetSystemFlag(-18);
        return;


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




