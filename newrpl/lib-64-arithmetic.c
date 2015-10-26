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
#define LIB_TOKENINFO lib64_tokeninfo
#define LIB_NUMBEROFCMDS LIB64_NUMBEROFCMDS

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(SETPREC,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(GETPREC,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(FLOOR,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(CEIL,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(IP,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(FP,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(POWMOD,MKTOKENINFO(6,TITYPE_FUNCTION,3,2))

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
#define CMD_EXTRANAME \
    "!", \
    "ISPRIME?"

#define CMD_EXTRAENUM \
    FACTORIAL, \
    ISPRIME

#define CMD_EXTRAINFO \
    MKTOKENINFO(1,TITYPE_POSTFIXOP,1,3), \
    MKTOKENINFO(8,TITYPE_FUNCTION,1,2)



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
#undef CMD







void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case SETPREC:
    {
        // TAKE AN INTEGER NUMBER FROM THE STACK
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        BINT64 number=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;
        if(number<4) number=4;
        if(number>MAX_USERPRECISION) number=MAX_USERPRECISION;
        setPrecision(number);
        rplDropData(1);
        return;
    }

    case GETPREC:
    {
        rplNewBINTPush(getPrecision(),DECBINT);
        return;
    }

    case FLOOR:
        {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR arg=rplPeekData(1);
        REAL rnum;
        if(ISBINT(*arg)) return;
        rplReadNumberAsReal(rplPeekData(1),&rnum);
        if(Exceptions) return;
        truncReal(&RReg[1],&rnum,0);
        if(Exceptions) return;
        rplDropData(1);
        rplNewRealFromRRegPush(1);
        return;
        }

    case CEIL:
        {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR arg=rplPeekData(1);
        if(ISBINT(*arg)) return;
        REAL rnum;
        rplReadNumberAsReal(rplPeekData(1),&rnum);
        if(Exceptions) return;
        fracReal(&RReg[1],&rnum);
        ipReal(&RReg[2],&rnum,1);
        if(!iszeroReal(&RReg[1])) {
            RReg[2].data[0]++;
            normalize(&RReg[2]);
        }
        rplDropData(1);
        rplNewRealFromRRegPush(2);
        return;
        }

    case IP:
    {
    if(rplDepthData()<1) {
        rplError(ERR_BADARGCOUNT);

        return;
    }
    WORDPTR arg=rplPeekData(1);
    if(ISBINT(*arg)) return;
    REAL rnum;
    rplReadNumberAsReal(arg,&rnum);
    if(Exceptions) return;
    ipReal(&RReg[1],&rnum,1);
    rplDropData(1);
    rplNewRealFromRRegPush(1);
    return;
    }

    case FP:
    {
    if(rplDepthData()<1) {
        rplError(ERR_BADARGCOUNT);

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
    fracReal(&RReg[1],&rnum);
    rplDropData(1);
    rplNewRealFromRRegPush(1);
    return;
    }

    case FACTORIAL:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);

        if(ISSYMBOLIC(*arg)||ISIDENT(*arg)) {
         rplSymbApplyOperator(MKOPCODE(LIBRARY_NUMBER,FACTORIAL),1);
         return;
        }

        if(!ISNUMBER(*arg)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }
        REAL rnum;
        rplReadNumberAsReal(arg,&rnum);
        if(Exceptions) return;
        if(!isintegerReal(&rnum)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }
        if(rnum.flags&F_NEGATIVE) {
            rplError(ERR_ARGOUTSIDEDOMAIN);
            return;
        }

        if(!inBINTRange(&rnum)) {
            rplError(ERR_NUMBERTOOBIG);
            return;
        }

        BINT n=(BINT)rplReadNumberAsBINT(arg);

        BINT64 result=factorialBINT(n);
        if(Exceptions) return;

        rplDropData(1);
        if(result<0) rplNewRealFromRRegPush(0);
        else rplNewBINTPush(result,DECBINT);
        return;

    }

    case ISPRIME:
    {


        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR arg=rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }


        if(!ISNUMBER(*arg)) {
            rplError(ERR_BADARGTYPE);
            return;
        }


        if(ISBINT(*arg)) {
            BINT64 n=rplReadBINT(arg);

            if(isprimeBINT(n)) rplOverwriteData(1,one_bint);
            else rplOverwriteData(1,zero_bint);

        } else {
            REAL *num;
            rplReadNumberAsReal(arg,&num);

            if(!isintegerReal(&num)) {
                rplError(ERR_INTEGEREXPECTED);
                return;
            }

            if(isprimeReal(&num)) rplOverwriteData(1,one_bint);
            else rplOverwriteData(1,zero_bint);

        }
        return;
    }
    case POWMOD:
        {


            if(rplDepthData()<3) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR arg=rplPeekData(3);

            // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
            // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
            if(ISLIST(*arg)) {

                BINT size1=rplObjSize(rplPeekData(1))+rplObjSize(rplPeekData(2));
                WORDPTR *savestk=DSTop;

                WORDPTR newobj=rplAllocTempOb(2+size1);
                if(!newobj) return;

                // CREATE A PROGRAM AND RUN THE MAP COMMAND
                newobj[0]=MKPROLOG(DOCOL,2+size1);
                rplCopyObject(newobj+1,rplPeekData(2));
                rplCopyObject(rplSkipOb(newobj+1),rplPeekData(1));
                newobj[size1+1]=CurOpcode;
                newobj[size1+2]=CMD_SEMI;

                rplDropData(2);
                rplPushData(newobj);

                rplCallOperator(CMD_MAP);

                if(Exceptions) {
                    if(DSTop>savestk) DSTop=savestk;
                }

                // EXECUTION WILL CONTINUE AT MAP

                return;
            }


            if(!ISNUMBER(*arg)) {
                rplError(ERR_BADARGTYPE);
                return;
            }

            WORDPTR exp=rplPeekData(2);
            WORDPTR mod=rplPeekData(1);

            if( !ISNUMBER(*exp) || !ISNUMBER(*mod)) {
                rplError(ERR_BADARGTYPE);
                return;
            }

            if(ISBINT(*arg) && ISBINT(*exp) && ISBINT(*mod)) {

                BINT64 a=rplReadBINT(arg);
                BINT64 e=rplReadBINT(exp);
                BINT64 m=rplReadBINT(mod);

                if(m<2147483648) {  // MAXIMUM MOD WE CAN USE WITH BINTS

                a=powmodBINT(a,e,m);

                rplNewBINTPush(a,DECBINT);
                return;
                }

            }
                // THERE'S REALS INVOLVED, DO IT ALL WITH REALS

                REAL a,e,m;
                rplReadNumberAsReal(arg,&a);
                rplReadNumberAsReal(exp,&e);
                rplReadNumberAsReal(mod,&m);

                if(!isintegerReal(&a)) {
                    rplError(ERR_INTEGEREXPECTED);
                    return;
                }

                if(!isintegerReal(&e)) {
                    rplError(ERR_INTEGEREXPECTED);
                    return;
                }

                if(!isintegerReal(&m)) {
                    rplError(ERR_INTEGEREXPECTED);
                    return;
                }

                BINT saveprec=Context.precdigits;
                BINT moddigits=(intdigitsReal(&m)+7)&~7;
                BINT numdigits=(intdigitsReal(&a)+7)&~7;
                BINT expdigits=(intdigitsReal(&e)+7)&~7;

                moddigits*=2;
                moddigits=(moddigits>numdigits)? moddigits:numdigits;
                moddigits=(moddigits>expdigits)? moddigits:expdigits;
                moddigits=(moddigits>Context.precdigits)? moddigits:Context.precdigits;

                if(moddigits>MAX_USERPRECISION) {
                    rplError(ERR_NUMBERTOOBIG);
                    return;
                }

                //   AUTOMATICALLY INCREASE PRECISION TEMPORARILY

                Context.precdigits=moddigits;

                powmodReal(&RReg[7],&a,&e,&m);

                Context.precdigits=saveprec;

                rplNewRealFromRRegPush(7);



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
    case OPCODE_AUTOCOMPNEXT:
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




