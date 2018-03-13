/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************



// REPLACE THE NUMBER
#define LIBRARY_NUMBER  64

#define ERROR_LIST \
        ERR(VECTOROFNUMBERSEXPECTED,0), \
        ERR(IABCUV_NO_SOLUTION,1), \
        ERR(POSITIVEINTEGEREXPECTED,2)

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(SETPREC,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(GETPREC,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(FLOOR,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(CEIL,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(IP,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(FP,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(MODSTO,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(MODRCL,MKTOKENINFO(6,TITYPE_FUNCTION,0,2)), \
    CMD(POWMOD,MKTOKENINFO(6,TITYPE_FUNCTION,2,2)), \
    CMD(MOD,MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    CMD(SQ,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(NEXTPRIME,MKTOKENINFO(9,TITYPE_FUNCTION,1,2)), \
    ECMD(FACTORIAL,"!",MKTOKENINFO(1,TITYPE_POSTFIXOP,1,3)), \
    ECMD(ISPRIME,"ISPRIME?",MKTOKENINFO(8,TITYPE_FUNCTION,1,2)), \
    CMD(MANT,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(XPON,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(SIGN,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    ECMD(PERCENT,"%",MKTOKENINFO(1,TITYPE_FUNCTION,2,2)), \
    ECMD(PERCENTCH,"%CH",MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    ECMD(PERCENTTOT,"%T",MKTOKENINFO(2,TITYPE_FUNCTION,2,2)), \
    CMD(GCD,MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    CMD(LCM,MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    CMD(IDIV2,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(IQUOT,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(ADDTMOD,MKTOKENINFO(7,TITYPE_FUNCTION,2,2)), \
    CMD(SUBTMOD,MKTOKENINFO(7,TITYPE_FUNCTION,2,2)), \
    CMD(MULTMOD,MKTOKENINFO(7,TITYPE_FUNCTION,2,2)), \
    CMD(PEVAL,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(PCOEF,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(IEGCD,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(IABCUV,MKTOKENINFO(6,TITYPE_FUNCTION,2,2)), \
    CMD(TCHEBYCHEFF,MKTOKENINFO(11,TITYPE_FUNCTION,1,2)), \
    CMD(LEGENDRE,MKTOKENINFO(8,TITYPE_FUNCTION,1,2)), \
    CMD(HERMITE,MKTOKENINFO(7,TITYPE_FUNCTION,1,2)), \
    CMD(TCHEBYCHEFF2,MKTOKENINFO(12,TITYPE_FUNCTION,1,2)), \
    CMD(HERMITE2,MKTOKENINFO(8,TITYPE_FUNCTION,1,2)), \
    CMD(DIV2,MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    CMD(PDIV2,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(PDER,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(PINT,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(PMUL,MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    CMD(PADD,MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    CMD(PSUB,MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    ECMD(IPPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    CMD(MIN,MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    CMD(MAX,MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    CMD(RND,MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    CMD(TRNC,MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    CMD(DIGITS,MKTOKENINFO(6,TITYPE_FUNCTION,3,2)), \
    CMD(PROOT,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(PREVPRIME,MKTOKENINFO(9,TITYPE_FUNCTION,1,2))




// ADD MORE OPCODES HERE


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
INCLUDE_ROMOBJECT(lib64_menu_0_main);
INCLUDE_ROMOBJECT(lib64_menu_1_real);
INCLUDE_ROMOBJECT(lib64_menu_2_integer);
INCLUDE_ROMOBJECT(lib64_menu_3_module);
INCLUDE_ROMOBJECT(lib64_menu_4_polynomial);
INCLUDE_ROMOBJECT(lib64_menu_5_poly_fcn);


// INTERNAL RPL CODE FOR IP ON SYMBOLICS

ROMOBJECT ipsymbolic_seco[]={
    MKPROLOG(DOCOL,3),
    (CMD_OVR_NUM),    // DO ->NUM ON THE NUMERIC SYMBOLIC
    MKOPCODE(LIBRARY_NUMBER,IPPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    CMD_SEMI
};

ROMOBJECT fpsymbolic_seco[]={
    MKPROLOG(DOCOL,6),
    (CMD_DUP),
    (CMD_OVR_NUM),    // DO ->NUM ON THE NUMERIC SYMBOLIC
    MKOPCODE(LIBRARY_NUMBER,IPPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    (CMD_OVR_SUB),
    (CMD_AUTOSIMPLIFY),   // SIMPLIFY BEFORE RETURN
    CMD_SEMI
};














// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib64_menu_0_main,
    (WORDPTR)lib64_menu_1_real,
    (WORDPTR)lib64_menu_2_integer,
    (WORDPTR)lib64_menu_3_module,
    (WORDPTR)lib64_menu_4_polynomial,
    (WORDPTR)lib64_menu_5_poly_fcn,
    (WORDPTR)ipsymbolic_seco,
    (WORDPTR)fpsymbolic_seco,
    0
};

const char const modulo_name[]="MOD";

#define MIN_SINT    -131072
#define MAX_SINT    +131071
#define MAX_BINT    +9223372036854775807LL
#define MIN_BINT    (-9223372036854775807LL-1LL)

// COUNT THE NUMBER OF BITS IN A POSITIVE INTEGER
int rpl_log2(BINT64 number,int bits)
{
    static const unsigned char log2_table[16]={0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4};
    if(bits<=4) return log2_table[number];
    bits>>=1;
    if(number>>bits) return rpl_log2(number>>bits,bits)+bits;
    return rpl_log2(number,bits);
}

inline __attribute__((always_inline)) BINT Add_BINT_is_Safe(BINT64 op1, BINT64 op2)
{
    BINT64 maxop2;
    BINT64 minop2;

    if(op1>0) {
        maxop2=MAX_BINT-op1;
        minop2=MIN_BINT;
    }
    else {
        maxop2=MAX_BINT;
        minop2=MIN_BINT-op1;
    }

    if( (op2>maxop2)||(op2<minop2)) {
        return 0;
    }
    else {
        return 1;
    }

}

inline __attribute__((always_inline)) BINT Sub_BINT_is_Safe(BINT64 op1, BINT64 op2)
{
    BINT64 maxop2;
    BINT64 minop2;

    if(op1>0) {
        maxop2=MAX_BINT-op1;
        minop2=MIN_BINT;
    }
    else {
        maxop2=MAX_BINT;
        minop2=MIN_BINT-op1;
    }

    if( (-op2>maxop2)||(-op2<minop2)) {
        return 0;
    }
    else {
        return 1;
    }

}

inline __attribute__((always_inline)) BINT Mul_BINT_is_Safe(BINT64 op1, BINT64 op2)
{
    if(op1<0) op1=-op1;
    if(op2<0) op2=-op2;
    if(op2>op1) { BINT64 tmp=op2; op2=op1; op1=tmp; }

    if(!(op2>>32)) {
        if(rpl_log2(op1,64)+rpl_log2(op2,32)<63) {
            return 1;
        }
    }
    return 0;
}


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    // LIBRARIES THAT DEFINE ONLY COMMANDS STILL HAVE TO RESPOND TO A FEW OVERLOADABLE OPERATORS
    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE) {
        // ONLY RESPOND TO EVAL, EVAL1 AND XEQ FOR THE COMMANDS DEFINED HERE
        // IN CASE OF COMMANDS TREATED AS OBJECTS (WHEN EMBEDDED IN LISTS)
        if( (OPCODE(CurOpcode)==OVR_EVAL)||
                (OPCODE(CurOpcode)==OVR_EVAL1)||
                (OPCODE(CurOpcode)==OVR_XEQ) )
        {
            // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            if(ISPROLOG(*rplPeekData(1))) {
                rplError(ERR_UNRECOGNIZEDOBJECT);
                return;
            }
            WORD saveOpcode=CurOpcode;
            CurOpcode=*rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode=saveOpcode;
            return;
        }
        // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
            if(OPCODE(CurOpcode)==OVR_SAME) {
                if(*rplPeekData(2)==*rplPeekData(1)) {
                    rplDropData(2);
                    rplPushTrue();
                } else {
                    rplDropData(2);
                    rplPushFalse();
                }

            }
            else {
                rplError(ERR_INVALIDOPCODE);
                return;
            }

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

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        REAL rnum;
        if(ISBINT(*arg)) return;
        rplReadNumberAsReal(rplPeekData(1),&rnum);
        if(Exceptions) return;
        if(isintegerReal(&rnum)) {
            return;
        }
        ipReal(&RReg[2],&rnum,1);
        if((rnum.flags&F_NEGATIVE)) {
            RReg[2].data[0]++;
            RReg[2].flags|=F_NEGATIVE;
            normalize(&RReg[2]);
        }
        rplDropData(1);
        rplNewRealFromRRegPush(2);
        return;
        }

    case CEIL:
        {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR arg=rplPeekData(1);

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISBINT(*arg)) return;
        REAL rnum;
        rplReadNumberAsReal(rplPeekData(1),&rnum);
        if(Exceptions) return;
        if(isintegerReal(&rnum)) {
            return;
        }
        ipReal(&RReg[2],&rnum,1);
        if(!(rnum.flags&F_NEGATIVE)) {
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

    if(ISLIST(*arg)) {
        rplListUnaryDoCmd();
        return;
    }

    if(ISBINT(*arg)) return;

    if(ISSYMBOLIC(*arg)) {
        // CHECK IF THE EXPRESSION IS NUMERIC
        if(rplSymbIsNumeric(arg)) {
            // COMPUTE THE EXPRESSION AND TAKE THE INTEGER PART, BUT DO IT IN RPL
            rplPushRet(IPtr);
            IPtr=(WORDPTR) ipsymbolic_seco;
            CurOpcode=(CMD_OVR_NUM);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO
            return;
        }
        rplSymbApplyOperator(CurOpcode,1);
        return;
    }


    REAL rnum;
    rplReadNumberAsReal(arg,&rnum);
    if(Exceptions) return;



    ipReal(&RReg[1],&rnum,1);
    rplDropData(1);
    rplNewRealFromRRegPush(1);
    return;
    }

case IPPOST:
    {
    // EXPECTS A REAL ON THE STACK
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        REAL num;

        rplReadNumberAsReal(rplPeekData(1),&num);
        if(Exceptions) return;

        ipReal(&RReg[0],&num,1);
        WORDPTR newnum;
        if(inBINT64Range(&RReg[0])) newnum=rplNewBINT(getBINT64Real(&RReg[0]),DECBINT);
        else newnum=rplNewRealFromRReg(0);

        if(Exceptions) return;
        rplOverwriteData(1,newnum);

        return;
     }

    case FP:
    {
    if(rplDepthData()<1) {
        rplError(ERR_BADARGCOUNT);

        return;
    }
    WORDPTR arg=rplPeekData(1);

    if(ISLIST(*arg)) {
        rplListUnaryDoCmd();
        return;
    }


    if(ISSYMBOLIC(*arg)) {
        // CHECK IF THE EXPRESSION IS NUMERIC
        if(rplSymbIsNumeric(arg)) {
            // COMPUTE THE EXPRESSION AND TAKE THE FRACTION PART, BUT DO IT IN RPL
            rplPushRet(IPtr);
            IPtr=(WORDPTR) fpsymbolic_seco;
            CurOpcode=(CMD_OVR_NUM);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO
            return;
        }
        rplSymbApplyOperator(CurOpcode,1);
        return;
    }


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

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

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
            rplListUnaryDoCmd();
            return;
        }


        if(!ISNUMBER(*arg)) {
            rplError(ERR_BADARGTYPE);
            return;
        }


        if(ISBINT(*arg)) {
            BINT64 n=rplReadBINT(arg);

            if(isprimeBINT(n)) rplOverwriteData(1,(WORDPTR)one_bint);
            else rplOverwriteData(1,(WORDPTR)zero_bint);

        } else {
            REAL num;
            rplReadNumberAsReal(arg,&num);

            if(!isintegerReal(&num)) {
                rplError(ERR_INTEGEREXPECTED);
                return;
            }

            if(isprimeReal(&num)) rplOverwriteData(1,(WORDPTR)one_bint);
            else rplOverwriteData(1,(WORDPTR)zero_bint);

        }
        return;
    }

    case NEXTPRIME:
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

            BINT64 next=nextprimeBINT(n);
            if(next>0) {
                rplNewBINTPush(next,DECBINT);
                if(Exceptions) return;
                WORDPTR ptr=rplPopData();
                rplOverwriteData(1,ptr);
                return;
            }
            // THE NEXT PRIME IS > 2^63, USE REALS INSTEAD

        }

        REAL num;
            rplReadNumberAsReal(arg,&num);

            if(!isintegerReal(&num)) {
                rplError(ERR_INTEGEREXPECTED);
                return;
            }

            nextprimeReal(0,&num);
            rplDropData(1);
            rplNewRealFromRRegPush(0);

        return;
    }


    case PREVPRIME:
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
            BINT64 next,prev;
            BINT previsprime;
            prev=n-30;    // ARBITRARILY SCAN 1000 NUMBERS TO THE LEFT
            if(prev<0) prev=0;
            previsprime=0;
            do {
            next=nextprimeBINT(prev);
            if((next>n) && previsprime) {
                rplNewBINTPush(prev,DECBINT);
                if(Exceptions) return;
                WORDPTR ptr=rplPopData();
                rplOverwriteData(1,ptr);
                return;
            }
            if(next>n) {
               // NO PRIMES WITHIN 1000 NUMBERS, SUBTRACT OTHER 1000 AND GO
                prev-=30;
                if(prev<0) prev=0;
            }
            if(next>0) {
                // FOUND A PRIME BETWEEN prev AND n, USE IT FOR NEXT ITERATION
                prev=next;
                previsprime=1;
            }

            } while(next>0);
            // THE NEXT PRIME IS > 2^63, USE REALS INSTEAD

        }

        REAL num;
            rplReadNumberAsReal(arg,&num);

            if(!isintegerReal(&num)) {
                rplError(ERR_INTEGEREXPECTED);
                return;
            }

            BINT previsprime=0;
            // USE RReg[5]=next
            // RReg[6]=prev
            rplBINTToRReg(7,-30);
            addReal(&RReg[6],&num,&RReg[7]);
            if(RReg[6].flags&F_NEGATIVE) rplZeroToRReg(6);
            do {
            nextprimeReal(5,&RReg[6]);
            BINT islarger=gtReal(&RReg[5],&num);
            if(islarger) {
                if(previsprime) {
                rplDropData(1);
                rplNewRealFromRRegPush(6);
                return;
                }
                // NO PRIMES IN THIS GROUP, SUBTRACT OTHER 1000 AND REDO
                addReal(&RReg[0],&RReg[6],&RReg[7]);
                if(RReg[0].flags&F_NEGATIVE) rplZeroToRReg(0);
                swapReal(&RReg[0],&RReg[6]);
            }
            else {
                // A PRIME WAS FOUND IN BETWEEN, USE IT FOR NEXT ITERATION
                swapReal(&RReg[5],&RReg[6]);    // prev=next;
                previsprime=1;
            }

            } while(1);

        return;


    }
    case MODSTO:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR arg=rplPeekData(1);

        if(!ISNUMBER(*arg)) {
            rplError(ERR_BADARGTYPE);
            return;
        }
        else {
            if (ISBINT(*arg)) {
                BINT64 m=rplReadBINT(arg);
                if (m < 0) m = -m;
                if (m < 2) m = 2;
                rplStoreSettingsbyName((BYTEPTR)modulo_name,(BYTEPTR)(modulo_name+3),rplNewBINT(m, DECBINT));
                rplDropData(1);

                return;
            }
            else {
                REAL num;
                rplReadNumberAsReal(arg,&num);
                if(!isintegerReal(&num)) {
                    rplError(ERR_INTEGEREXPECTED);
                    return;
                }
                if(num.flags&F_NEGATIVE) num.flags^=F_NEGATIVE;
                newRealFromBINT(&RReg[0],2,0);
                if (ltReal(&num, &RReg[0])) {
                    newRealFromBINT(&num,2,0);
                }
                rplStoreSettingsbyName((BYTEPTR)modulo_name,(BYTEPTR)(modulo_name+3),rplNewReal(&num));
                rplDropData(1);
                return;
            }

        }

        return;
    }

    case MODRCL:
    {
        WORDPTR mod=rplGetSettingsbyName((BYTEPTR)modulo_name,(BYTEPTR)modulo_name+3);
        if(!mod) mod=(WORDPTR)zero_bint;
        if(!ISNUMBER(*mod)) {
            rplError(ERR_BADARGTYPE);
            return;
        }
        if (ISBINT(*mod)) {
            BINT64 m=rplReadBINT(mod);
            rplNewBINTPush(m,DECBINT);
            return;
        }
        REAL m;
        rplReadNumberAsReal(mod,&m);
        rplNewRealPush(&m);
        return;
    }

    case POWMOD:
    case ADDTMOD:
    case SUBTMOD:
    case MULTMOD:
    {

        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR arg1=rplPeekData(2);
        WORDPTR arg2=rplPeekData(1);

        if(ISLIST(*arg1) || ISLIST(*arg2)){
            rplListBinaryDoCmd();
            return;
        }

        if(!ISNUMBER(*arg1)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        WORDPTR mod=rplGetSettingsbyName((BYTEPTR)modulo_name,(BYTEPTR)modulo_name+3);
        if(!mod) mod=(WORDPTR)two_bint;
        if( !ISNUMBER(*arg2) || !ISNUMBER(*mod)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        if(ISBINT(*arg1) && ISBINT(*arg2) && ISBINT(*mod)) {

            BINT64 a1=rplReadBINT(arg1);
            BINT64 a2=rplReadBINT(arg2);
            BINT64 m=rplReadBINT(mod);
            BINT64 r;

            if (m<2147483648LL) {
                BINT isOK = 1;
                if (OPCODE(CurOpcode) == POWMOD) {
                    r = powmodBINT(a1,a2,m);
                }
                else if ((OPCODE(CurOpcode) == ADDTMOD) && Add_BINT_is_Safe(a1,a2)) {
                    r = (a1+a2)%m;
                }
                else if ((OPCODE(CurOpcode) == SUBTMOD) && Sub_BINT_is_Safe(a1,a2)) {
                    r = (a1-a2)%m;
                }
                else if ((OPCODE(CurOpcode) == MULTMOD) && Mul_BINT_is_Safe(a1,a2)) {
                    r = (a1*a2)%m;
                }
                else {
                    isOK = 0;
                }

                if(isOK) {
                    if (r < 0) {
                        r+=m;
                    }

                    rplDropData(2);

                    rplNewBINTPush(r,DECBINT);
                    return;
                }
            }
        }

        // FALL THROUGH
        // THERE'S REALS INVOLVED, DO IT ALL WITH REALS

        REAL a1,a2,m;
        rplReadNumberAsReal(arg1,&a1);
        rplReadNumberAsReal(arg2,&a2);
        rplReadNumberAsReal(mod,&m);

        if(!isintegerReal(&a1)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        if(!isintegerReal(&a2)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        if(!isintegerReal(&m)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        BINT saveprec=Context.precdigits;
        BINT moddigits=(intdigitsReal(&m)+7)&~7;
        BINT numdigits=(intdigitsReal(&a1)+7)&~7;
        BINT expdigits=(intdigitsReal(&a2)+7)&~7;

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

        if (OPCODE(CurOpcode) == POWMOD) {
            powmodReal(&RReg[7],&a1,&a2,&m);
        }
        else if (OPCODE(CurOpcode) == ADDTMOD) {
            addReal(&RReg[0], &a1, &a2);
            divmodReal(&RReg[6],&RReg[7],&RReg[0],&m);
        }
        else if (OPCODE(CurOpcode) == SUBTMOD) {
            subReal(&RReg[0], &a1, &a2);
            divmodReal(&RReg[6],&RReg[7],&RReg[0],&m);
        }
        else if (OPCODE(CurOpcode) == MULTMOD) {
            mulReal(&RReg[0], &a1, &a2);
            divmodReal(&RReg[6],&RReg[7],&RReg[0],&m);
        }
        else {
            rplError(ERR_INVALID);
            return;
        }
        if(RReg[7].flags&F_NEGATIVE) {
            addReal(&RReg[7],&RReg[7],&m);
        }

        Context.precdigits=saveprec;

        rplDropData(2);

        rplNewRealFromRRegPush(7);
        rplCheckResultAndError(&RReg[7]);

        return;

    }

    case MOD:
    case IDIV2:
    case IQUOT:
    {

            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR arg=rplPeekData(2);
            WORDPTR mod=rplPeekData(1);

            if(ISLIST(*arg) || ISLIST(*mod)){
                rplListBinaryDoCmd();
                return;
            }
            else if((ISIDENT(*mod) || ISSYMBOLIC(*mod)) || (ISIDENT(*arg) || ISSYMBOLIC(*arg))){
                    rplSymbApplyOperator(CurOpcode,2);
                    return;
            }

            if( !ISNUMBER(*arg) || !ISNUMBER(*mod)) {
                rplError(ERR_BADARGTYPE);
                return;
            }

            if(ISBINT(*arg) && ISBINT(*mod)) {

                BINT64 a=rplReadBINT(arg);
                BINT64 m=rplReadBINT(mod);
                BINT64 r,q;

                if (m == (BINT64)0) {
                    rplError(ERR_MATHDIVIDEBYZERO);
                    return;
                }
                if(m<(1LL<<62)) {
                r = (a%m + m)%m;
                q = (a-r)/m;
                rplDropData(2);
                if (OPCODE(CurOpcode) == IDIV2 || OPCODE(CurOpcode) == IQUOT) {
                    rplNewBINTPush(q,DECBINT);
                }
                if (OPCODE(CurOpcode) == IDIV2 || OPCODE(CurOpcode) == MOD) {
                    rplNewBINTPush(r,DECBINT);
                }
                return;
                }
            }
                // THERE'S REALS INVOLVED, DO IT ALL WITH REALS

                REAL a,m;
                rplReadNumberAsReal(arg,&a);
                rplReadNumberAsReal(mod,&m);

                BINT saveprec=Context.precdigits;
                if (OPCODE(CurOpcode) == IDIV2 || OPCODE(CurOpcode) == IQUOT) {
                    if(!isintegerReal(&a)) {
                        rplError(ERR_INTEGEREXPECTED);
                        return;
                    }
                    if(!isintegerReal(&m)) {
                        rplError(ERR_INTEGEREXPECTED);
                        return;
                    }
                    BINT moddigits=(intdigitsReal(&m)+7)&~7;
                    BINT argdigits=(intdigitsReal(&a)+7)&~7;

                    moddigits*=2;
                    moddigits=(moddigits>argdigits)? moddigits:argdigits;
                    moddigits=(moddigits>Context.precdigits)? moddigits:Context.precdigits;

                    if(moddigits>MAX_USERPRECISION) {
                        rplError(ERR_NUMBERTOOBIG);
                        return;
                    }

                    //   AUTOMATICALLY INCREASE PRECISION TEMPORARILY

                    Context.precdigits=moddigits;
                }

                if (iszeroReal(&m)) {
                    rplError(ERR_MATHDIVIDEBYZERO);
                    return;
                }

                divmodReal(&RReg[7],&RReg[6],&a,&m);
                // correct negative remainder
                if(RReg[6].flags&F_NEGATIVE) {
                    addReal(&RReg[6],&RReg[6],&m);
                    newRealFromBINT(&RReg[0],1,0);
                    subReal(&RReg[7],&RReg[7],&RReg[0]);
                }

                Context.precdigits=saveprec;

                rplDropData(2);

                if (OPCODE(CurOpcode) == IDIV2 || OPCODE(CurOpcode) == IQUOT) {
                    rplNewRealFromRRegPush(7);
                }
                if (OPCODE(CurOpcode) == IDIV2 || OPCODE(CurOpcode) == MOD) {
                    rplNewRealFromRRegPush(6);
                }
                rplCheckResultAndError(&RReg[6]);
                rplCheckResultAndError(&RReg[7]);

            return;

    }

    case SQ:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR arg=rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }


        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }


        // FOR ALL OTHER OBJECTS, JUST DO DUP *

        rplPushData(arg);

        rplCallOvrOperator((CMD_OVR_MUL));

        return;
     }

    case MANT:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR arg=rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }


        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        if(!ISNUMBER(*arg)) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        REAL rnum;
        BINT digits;
        rplReadNumberAsReal(rplPopData(),&rnum);

        digits=sig_digits(rnum.data[rnum.len-1])+((rnum.len-1)<<3);

        rnum.exp=-digits+1;
        rnum.flags&=~F_NEGATIVE;

        rplNewRealPush(&rnum);
        return;
    }

    case XPON:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR arg=rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        if(!ISNUMBER(*arg)) {
            rplError(ERR_REALEXPECTED);
            return;
        }
        REAL rnum;
        BINT digits;
        rplReadNumberAsReal(rplPopData(),&rnum);

        digits=sig_digits(rnum.data[rnum.len-1])+((rnum.len-1)<<3);

        digits+=rnum.exp-1;
        rplNewBINTPush(digits,DECBINT);
        return;

    }

    case SIGN:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }


        WORDPTR arg=rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if( ISSYMBOLIC(*arg) || ISIDENT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        if(ISUNIT(*arg)) {
            WORDPTR *stkclean=DSTop;
            BINT nlevels=rplUnitExplode(rplPeekData(1));
            if(Exceptions) { DSTop=stkclean; return; }
            rplOverwriteData(nlevels+1,rplPeekData(nlevels));

            rplDropData(nlevels);         // POP EVERYTHING EXCEPT THE VALUE

            arg=rplPeekData(1);
            // FALL THROUGH TO INTEGERS AND REALS, THIS IS DELIBERATE
        }



        if(ISBINT(*arg)) {
            BINT64 r=rplReadBINT(arg);
            if(r>0) rplOverwriteData(1,(WORDPTR)one_bint);
            else {
                if(r<0) rplOverwriteData(1,(WORDPTR)minusone_bint);
                else rplOverwriteData(1,(WORDPTR)zero_bint);
            }
            return;
        }

        if(ISREAL(*arg)) {
            REAL rnum;
            rplReadNumberAsReal(rplPeekData(1),&rnum);

            if(iszeroReal(&rnum)) rplOverwriteData(1,(WORDPTR)zero_bint);
            else {
                if(rnum.flags&F_NEGATIVE) rplOverwriteData(1,(WORDPTR)minusone_bint);
                else rplOverwriteData(1,(WORDPTR)one_bint);
            }
            return;
        }

        if(ISCOMPLEX(*arg)) {
            REAL Rarg,Iarg;

            rplRealPart(arg,&Rarg);
            rplImaginaryPart(arg,&Iarg);

            Context.precdigits+=8;
            mulReal(&RReg[2],&Rarg,&Rarg);
            mulReal(&RReg[3],&Iarg,&Iarg);
            addReal(&RReg[0],&RReg[2],&RReg[3]);

            Context.precdigits-=8;

            hyp_sqrt(&RReg[0]);
            finalize(&RReg[0]);

            divReal(&RReg[1],&Rarg,&RReg[0]);
            divReal(&RReg[2],&Iarg,&RReg[0]);


            rplNewComplexPush(&RReg[1],&RReg[2],ANGLENONE);

            return;

        }


        rplError(ERR_REALEXPECTED);
        return;
    }

    case PERCENT:
    {

        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR pct=rplPeekData(1);
        WORDPTR arg1=rplPeekData(2);

        if(ISLIST(*arg1) || ISLIST(*pct)){
            rplListBinaryDoCmd();
            return;
        }
        else if((ISIDENT(*pct) || ISSYMBOLIC(*pct)) || (ISIDENT(*arg1) || ISSYMBOLIC(*arg1))){
                rplSymbApplyOperator(CurOpcode,2);
                return;
        }
        else if(ISNUMBER(*pct) && ISNUMBER(*arg1) ){
            REAL x;
            rplReadNumberAsReal(pct,&x);
            x.exp -= 2; // divide by 100
            // replace level 1 value
            WORDPTR newnumber=rplNewReal(&x);
            if(!newnumber) return;
            rplOverwriteData(1,newnumber);
            rplCallOvrOperator(CMD_OVR_MUL);
            return;
        }
        else {
            rplError(ERR_BADARGTYPE);
            return;
        }
        return;
    }

    case PERCENTCH:
    {

            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR old_val=rplPeekData(2);
            WORDPTR new_val=rplPeekData(1);

            if(ISLIST(*old_val) || ISLIST(*new_val)){
                rplListBinaryDoCmd();
                return;
            }
            else if((ISIDENT(*new_val) || ISSYMBOLIC(*new_val)) || (ISIDENT(*old_val) || ISSYMBOLIC(*old_val))){
                    rplSymbApplyOperator(CurOpcode,2);
                    return;
            }
            else if(ISNUMBER(*new_val) && ISNUMBER(*old_val) ){

                // DO IT ALL WITH REALS
                // calculate 100*(y-x)/x
                // Same names as in the advanced user's reference manual
                REAL x,y;
                rplReadNumberAsReal(old_val,&x);
                rplReadNumberAsReal(new_val,&y);
                subReal(&RReg[1],&y,&x); // delta
                RReg[1].exp += 2; // multiply delta by 100
                divReal(&RReg[0],&RReg[1],&x);

                WORDPTR newnumber=rplNewReal(&RReg[0]);
                if(!newnumber) return;
                // drop one value and replace level 1 value
                rplDropData(1);
                rplOverwriteData(1,newnumber);

                return;
            }
            else {
                rplError(ERR_BADARGTYPE);
                return;
            }
            return;


    }

    case PERCENTTOT:
    {

            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR old_val=rplPeekData(2);
            WORDPTR new_val=rplPeekData(1);

            if(ISLIST(*old_val) || ISLIST(*new_val)){
                rplListBinaryDoCmd();
                return;
            }
            else if((ISIDENT(*new_val) || ISSYMBOLIC(*new_val)) || (ISIDENT(*old_val) || ISSYMBOLIC(*old_val))){
                    rplSymbApplyOperator(CurOpcode,2);
                    return;
            }
            else if(ISNUMBER(*new_val) && ISNUMBER(*old_val) ){

                // DO IT ALL WITH REALS
                // calculate 100*y/x
                // Same names as in the advanced user's reference manual
                REAL x,y;
                rplReadNumberAsReal(old_val,&x);
                rplReadNumberAsReal(new_val,&y);
                y.exp += 2; // multiply by 100
                divReal(&RReg[0],&y,&x);

                WORDPTR newnumber=rplNewReal(&RReg[0]);
                if(!newnumber) return;
                // drop one value and replace level 1 value
                rplDropData(1);
                rplOverwriteData(1,newnumber);

                return;

            }
            else {
                rplError(ERR_BADARGTYPE);
                return;
            }
            return;

    }

    case GCD:
    case LCM:
    case IEGCD:
    {

            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR arg1=rplPeekData(2);
            WORDPTR arg2=rplPeekData(1);

            if(ISLIST(*arg1) || ISLIST(*arg2)){
                rplListBinaryDoCmd();
                return;
            }
            else if((ISIDENT(*arg1) || ISSYMBOLIC(*arg1)) || (ISIDENT(*arg2) || ISSYMBOLIC(*arg2))){
                    rplSymbApplyOperator(CurOpcode,2);
                    return;
            }

            if( !ISNUMBER(*arg1) || !ISNUMBER(*arg2)) {
                rplError(ERR_BADARGTYPE);
                return;
            }

            BINT isIEGCD = (OPCODE(CurOpcode) == IEGCD);
            BINT chs1 = 0;
            BINT chs2 = 0;
            BINT swapped = 0;
            if(ISBINT(*arg1) && ISBINT(*arg2)) {

                BINT64 a1=rplReadBINT(arg1);
                BINT64 a2=rplReadBINT(arg2);
                BINT64 r1,r2,r3,gcd;
                BINT64 q,s1,s2,s3,t1,t2,t3,s,t;
                if(a1 < 0) {
                    a1 = -a1;
                    chs1 = 1;
                }
                if(a2 < 0) {
                    a2 = -a2;
                    chs2 = 1;
                }
                if(a1 > a2){
                    r1 = a1;
                    r2 = a2;
                }
                else {
                    r2 = a1;
                    r1 = a2;
                    swapped = 1;
                }
                if (r2 == (BINT64)0) {
                    rplError(ERR_MATHDIVIDEBYZERO);
                    return;
                }
                // avoid swapping elements by loop unrolling
                BINT notfinished = 1;
                if (isIEGCD) {
                    s1 = 1;
                    s2 = 0;
                    t1 = 0;
                    t2 = 1;
                }
                do {
                    if(r2!=0){
                        r3 = r1%r2;
                        if(isIEGCD){
                            //remainder = (dividend%divisor + divisor)%divisor;  // also for negative numbers
                            //quotient = (dividend-remainder)/divisor;
                            q = (r1-r3)/r2;
                            s3 = s1 - q*s2;
                            t3 = t1 - q*t2;
                        }
                    }
                    else {
                        gcd = r1;
                        notfinished = 0;
                        if(isIEGCD){
                            s = s1;
                            t = t1;
                        }
                        break;
                    }
                    if(r3!=0){
                        r1 = r2%r3;
                        if(isIEGCD){
                            q = (r2-r1)/r3;
                            s1 = s2 - q*s3;
                            t1 = t2 - q*t3;
                        }
                    }
                    else {
                        gcd = r2;
                        notfinished = 0;
                        if(isIEGCD){
                            s = s2;
                            t = t2;
                        }
                        break;
                    }
                    if(r1!=0){
                        r2 = r3%r1;
                        if(isIEGCD){
                            q = (r3-r2)/r1;
                            s2 = s3 - q*s1;
                            t2 = t3 - q*t1;
                        }
                    }
                    else {
                        gcd = r3;
                        notfinished = 0;
                        if(isIEGCD){
                            s = s3;
                            t = t3;
                        }
                        break;
                    }
                } while (notfinished);
                if (OPCODE(CurOpcode) == GCD) {
                    rplDropData(2);
                    rplNewBINTPush(gcd,DECBINT);
                }
                else if (isIEGCD) {
                    rplDropData(2);
                    rplNewBINTPush(gcd,DECBINT);
                    if(!swapped){
                        if (chs1) {
                            rplNewBINTPush(-s,DECBINT);
                        }
                        else {
                            rplNewBINTPush(s,DECBINT);
                        }
                        if (chs2) {
                            rplNewBINTPush(-t,DECBINT);
                        }
                        else {
                            rplNewBINTPush(t,DECBINT);
                        }
                    }
                    else {
                        if (chs1) {
                            rplNewBINTPush(-t,DECBINT);
                        }
                        else {
                            rplNewBINTPush(t,DECBINT);
                        }
                        if (chs2) {
                            rplNewBINTPush(-s,DECBINT);
                        }
                        else {
                            rplNewBINTPush(s,DECBINT);
                        }
                    }
                }
                else // LCM(a1,a2) = a1*a2/gcd(a1,a2)
                {
                    REAL x,y,rgcd;
                    rplLoadBINTAsReal(a1,&x);
                    rplLoadBINTAsReal(a2,&y);
                    rplLoadBINTAsReal(gcd,&rgcd);
                    mulReal(&RReg[0],&x,&y);
                    divReal(&RReg[4],&RReg[0],&rgcd);
                    if((x.flags&F_APPROX)||(y.flags&F_APPROX)) RReg[4].flags|=F_APPROX;
                    else RReg[4].flags&=~F_APPROX;    // REMOVE THE APPROXIMATED FLAG AFTER TRUNCATION
                    rplDropData(2);
                    rplNewRealFromRRegPush(4);
                }
                return;
            }
            // THERE'S REALS INVOLVED, DO IT ALL WITH REALS

            rplNumberToRReg(1, arg1);
            rplNumberToRReg(2, arg2);

            if(!isintegerReal(&RReg[1])) {
                rplError(ERR_INTEGEREXPECTED);
                return;
            }
            if(!isintegerReal(&RReg[2])) {
                rplError(ERR_INTEGEREXPECTED);
                return;
            }

            if(RReg[1].flags&F_NEGATIVE) {
                RReg[1].flags^=F_NEGATIVE;
                chs1 = 1;
            }
            if(RReg[2].flags&F_NEGATIVE) {
                RReg[2].flags^=F_NEGATIVE;
                chs2 = 1;
            }
            if(gtReal(&RReg[2],&RReg[1])) {
                swapReal(&RReg[2],&RReg[1]);
                swapped = 1;
            }
            if (iszeroReal(&RReg[2])) {
                rplError(ERR_MATHDIVIDEBYZERO);
                return;
            }

            BINT saveprec=Context.precdigits;

            BINT arg1digits=(intdigitsReal(&RReg[1])+7)&~7;
            BINT arg2digits=(intdigitsReal(&RReg[2])+7)&~7;

            arg1digits*=2;
            arg1digits=(arg1digits>arg2digits)? arg1digits:arg2digits;
            arg1digits=(arg1digits>Context.precdigits)? arg1digits:Context.precdigits;

            if(arg1digits>MAX_USERPRECISION) {
                rplError(ERR_NUMBERTOOBIG);
                return;
            }

            //   AUTOMATICALLY INCREASE PRECISION TEMPORARILY

            Context.precdigits=arg1digits;

            // avoid swapping elements by loop unrolling
            BINT notfinished = 1;
            const BINT q=0,r1=1,r2=2,r3=3,s1=4,s2=5,s3=6,t1=7,t2=8,t3=9;
            BINT igcd,s=0,t=0;
            REAL tmp;
            if (isIEGCD) {
                newRealFromBINT(&RReg[s1],1,0);
                newRealFromBINT(&RReg[s2],0,0);
                newRealFromBINT(&RReg[t1],0,0);
                newRealFromBINT(&RReg[t2],1,0);
                tmp.data=allocRegister();
            }
            do {
                if(!iszeroReal(&RReg[r2])){
                    divmodReal(&RReg[q],&RReg[r3],&RReg[r1],&RReg[r2]);
                    if(isIEGCD){
                        mulReal(&tmp,&RReg[q],&RReg[s2]);
                        subReal(&RReg[s3],&RReg[s1],&tmp);
                        mulReal(&tmp,&RReg[q],&RReg[t2]);
                        subReal(&RReg[t3],&RReg[t1],&tmp);
                        //s3 = s1 - q*s2;
                        //t3 = t1 - q*t2;
                    }
                }
                else {
                    igcd = r1;
                    notfinished = 0;
                    if(isIEGCD){
                        s = s1;
                        t = t1;
                    }
                    break;
                }
                if(!iszeroReal(&RReg[r3])){
                    divmodReal(&RReg[q],&RReg[r1],&RReg[r2],&RReg[r3]);
                    if(isIEGCD){
                        mulReal(&tmp,&RReg[q],&RReg[s3]);
                        subReal(&RReg[s1],&RReg[s2],&tmp);
                        mulReal(&tmp,&RReg[q],&RReg[t3]);
                        subReal(&RReg[t1],&RReg[t2],&tmp);
                        //s1 = s2 - q*s3;
                        //t1 = t2 - q*t3;
                    }
                }
                else {
                    igcd = r2;
                    notfinished = 0;
                    if(isIEGCD){
                        s = s2;
                        t = t2;
                    }
                    break;
                }
                if(!iszeroReal(&RReg[r1])){
                    divmodReal(&RReg[q],&RReg[r2],&RReg[r3],&RReg[r1]);
                    if(isIEGCD){
                        mulReal(&tmp,&RReg[q],&RReg[s1]);
                        subReal(&RReg[s2],&RReg[s3],&tmp);
                        mulReal(&tmp,&RReg[q],&RReg[t1]);
                        subReal(&RReg[t2],&RReg[t3],&tmp);
                        //s2 = s3 - q*s1;
                        //t2 = t3 - q*t1;
                    }
                }
                else {
                    igcd = r3;
                    notfinished = 0;
                    if(isIEGCD){
                        s = s3;
                        t = t3;
                    }
                    break;
                }
            } while (notfinished);

            if (isIEGCD) {
                freeRegister(tmp.data);
            }

            if (OPCODE(CurOpcode) == GCD) {
                Context.precdigits=saveprec;
                rplDropData(2);
                rplNewRealFromRRegPush(igcd);
                rplCheckResultAndError(&RReg[igcd]);
            }
            else if (isIEGCD) {
                Context.precdigits=saveprec;
                rplDropData(2);
                rplNewRealFromRRegPush(igcd);
                rplCheckResultAndError(&RReg[igcd]);
                if(!swapped){
                    if (chs1) {
                        RReg[s].flags^=F_NEGATIVE;
                        rplNewRealFromRRegPush(s);
                        rplCheckResultAndError(&RReg[s]);
                    }
                    else {
                        rplNewRealFromRRegPush(s);
                        rplCheckResultAndError(&RReg[s]);
                    }
                    if (chs2) {
                        RReg[t].flags^=F_NEGATIVE;
                        rplNewRealFromRRegPush(t);
                        rplCheckResultAndError(&RReg[t]);
                    }
                    else {
                        rplNewRealFromRRegPush(t);
                        rplCheckResultAndError(&RReg[t]);
                    }
                }
                else {
                    if (chs1) {
                        RReg[t].flags^=F_NEGATIVE;
                        rplNewRealFromRRegPush(t);
                        rplCheckResultAndError(&RReg[t]);
                    }
                    else {
                        rplNewRealFromRRegPush(t);
                        rplCheckResultAndError(&RReg[t]);
                    }
                    if (chs2) {
                        RReg[s].flags^=F_NEGATIVE;
                        rplNewRealFromRRegPush(s);
                        rplCheckResultAndError(&RReg[s]);
                    }
                    else {
                        rplNewRealFromRRegPush(s);
                        rplCheckResultAndError(&RReg[s]);
                    }
                }
            }
            else // LCM(a1,a2) = a1*a2/gcd(a1,a2)
            {
                REAL x,y;
                rplReadNumberAsReal(arg1,&x);
                rplReadNumberAsReal(arg2,&y);
                mulReal(&RReg[0],&x,&y);
                divReal(&RReg[4],&RReg[0],&RReg[igcd]);
                if((x.flags&F_APPROX)||(y.flags&F_APPROX)) RReg[4].flags|=F_APPROX;
                else RReg[4].flags&=~F_APPROX;    // REMOVE THE APPROXIMATED FLAG AFTER TRUNCATION
                Context.precdigits=saveprec;
                rplDropData(2);
                rplNewRealFromRRegPush(4);
                rplCheckResultAndError(&RReg[4]);
            }



            return;

    }

    case PEVAL:
    {

            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR vect_val=rplPeekData(2);
            WORDPTR real_val=rplPeekData(1);


            if(ISLIST(*real_val)) {
                rplListBinaryDoCmd();
                return;
            }
            else if((ISIDENT(*vect_val) || ISSYMBOLIC(*vect_val)))
            {
                    rplSymbApplyOperator(CurOpcode,2);
                    return;
            }
            else if(ISMATRIX(*vect_val) ){

                BINT rows=MATROWS(vect_val[1]),cols=MATCOLS(vect_val[1]);

                if(rows) {
                    rplError(ERR_VECTOREXPECTED);
                    return;
                }

                WORDPTR *savestk=DSTop;

                rplPushData(rplPeekData(2));
                if(Exceptions) { DSTop=savestk; return; }
                WORDPTR *first=rplMatrixExplode();
                if(!first || Exceptions) { DSTop=savestk; return; }

                WORDPTR result=rplPolyEvalEx(first,cols-1,savestk-1);
                if(!result || Exceptions) { DSTop=savestk; return; }

                if(ISSYMBOLIC(*result)) {
                    result=rplSymbNumericReduce(result);
                   if(!result || Exceptions) { DSTop=savestk; return; }
                }

                DSTop=savestk-1;
                rplOverwriteData(1,result);

                return;

            }
            else {
                rplError(ERR_BADARGTYPE);
                return;
            }
            return;

    }

    case PCOEF:
    {

            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORDPTR vect_val=rplPeekData(1);

            if(ISMATRIX(*vect_val)){

                BINT rows=MATROWS(vect_val[1]),cols=MATCOLS(vect_val[1]);

                if(rows) {
                    rplError(ERR_VECTOREXPECTED);
                    return;
                }
                BINT f,icoef,j;

                for(f=1;f<=cols;++f) {
                    WORDPTR entry=rplMatrixFastGet(vect_val,1,f);
                    if(!ISNUMBER(*entry)) {
                        rplError(ERR_VECTOROFNUMBERSEXPECTED);
                        return;
                    }
                }
                // DO IT ALL WITH REALS

                BINT saveprec=Context.precdigits;

                BINT argdigits=(cols+7)&~7;

                if(argdigits>MAX_USERPRECISION) {
                    argdigits=MAX_USERPRECISION;
                }

                argdigits=(argdigits>Context.precdigits)? argdigits:Context.precdigits;
                //   AUTOMATICALLY INCREASE PRECISION TEMPORARILY

                Context.precdigits=argdigits;

                WORDPTR *Firstelem=DSTop;

                rplPushData((WORDPTR)one_bint);
                if(Exceptions) {
                    DSTop=Firstelem;
                    Context.precdigits=saveprec;
                    return;
                }

                for(icoef=1;icoef<=cols;++icoef){
                    WORDPTR ai=rplMatrixFastGet(vect_val,1,icoef);
                    rplNumberToRReg(0,ai);
                    RReg[0].flags^=F_NEGATIVE;
                    rplPushData((WORDPTR)zero_bint);
                    for (j=1; j<=icoef; ++j){
                        if (j==1){
                            rplNumberToRReg(1, *(Firstelem+j-1));
                        }
                        else {
                            copyReal(&RReg[1], &RReg[2]);
                        }
                        rplNumberToRReg(2, *(Firstelem+j));

                        mulReal(&RReg[3],&RReg[0],&RReg[1]);
                        addReal(&RReg[4],&RReg[3],&RReg[2]);
                        WORDPTR newnumber=rplNewReal(&RReg[4]);
                        if(!newnumber) {
                            DSTop=Firstelem;
                            Context.precdigits=saveprec;
                            return;
                        }
                        *(Firstelem+j)=newnumber;
                    }
                    if(Exceptions) {
                        DSTop=Firstelem;
                        Context.precdigits=saveprec;
                        return;
                    }
                }

                Context.precdigits=saveprec;
                WORDPTR pcoefs=rplMatrixCompose(0,cols+1);
                if(!pcoefs) return;
                rplDropData(cols+1);
                rplOverwriteData(1,pcoefs);


                return;

            }
            else {
                rplError(ERR_BADARGTYPE);
                return;
            }
            return;

    }

    case IABCUV:
    {
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR arg0=rplPeekData(3);
        WORDPTR arg1=rplPeekData(2);
        WORDPTR arg2=rplPeekData(1);

//        if(ISLIST(*arg1) || ISLIST(*arg2)){
//            rplListBinaryDoCmd();
//            return;
//        }
//        else
        if((ISIDENT(*arg0) || ISSYMBOLIC(*arg0)) || (ISIDENT(*arg1) || ISSYMBOLIC(*arg1)) || (ISIDENT(*arg2) || ISSYMBOLIC(*arg2))){
                rplSymbApplyOperator(CurOpcode,3);
                return;
        }

        if( !ISNUMBER(*arg0) ||  !ISNUMBER(*arg1) || !ISNUMBER(*arg2)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        WORDPTR *savestk=DSTop; // Drop arguments in case of error

        // Stack: A B C
        rplPushData(rplPeekData(3));
        rplPushData(rplPeekData(3));
        // Stack: A B C A B

        rplCallOperator(MKOPCODE(LIBRARY_NUMBER,IEGCD));
        if(Exceptions) {
            if(DSTop>savestk) DSTop=savestk;
            return;
        }

        // Stack: A B C GCD(A,B) S T
        //        6 5 4   3      2 1

        // check for Solution Condition: C MOD GCD(A,B) = 0
        WORDPTR wp_c=rplPeekData(4);
        WORDPTR wp_gcd_ab=rplPeekData(3);
        WORDPTR wp_s=rplPeekData(2);
        WORDPTR wp_t=rplPeekData(1);
        if(ISBINT(*wp_s) && ISBINT(*wp_t) && ISBINT(*wp_c) && ISBINT(*wp_gcd_ab)) {

            BINT64 c=rplReadBINT(wp_c);
            BINT64 gcd=rplReadBINT(wp_gcd_ab);
            BINT64 r = (c%gcd + gcd)%gcd;
            if (r == 0) {
                BINT64 q = (c-r)/gcd;
                BINT64 s = rplReadBINT(wp_s);
                BINT64 t = rplReadBINT(wp_t);
                s*=q;
                t*=q;
                rplDropData(6);
                rplNewBINTPush(s,DECBINT);
                rplNewBINTPush(t,DECBINT);
            }
            else {
                // ERROR: no soluton
                rplDropData(6);
                rplError(ERR_IABCUV_NO_SOLUTION);
                //DSTop=savestk;
                return;
            }

        }

        // THERE'S REALS INVOLVED, DO IT ALL WITH REALS
        rplNumberToRReg(1, wp_gcd_ab);
        rplNumberToRReg(2, wp_c);

        if(!isintegerReal(&RReg[2])) {
            if(DSTop>savestk) DSTop=savestk;
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        //           Q         R    =    A   /    B
        divmodReal(&RReg[4],&RReg[3],&RReg[2],&RReg[1]);
        if(iszeroReal(&RReg[3])) {
            rplNumberToRReg(6, wp_s);
            rplNumberToRReg(5, wp_t);

            mulReal(&RReg[2],&RReg[4],&RReg[6]);
            mulReal(&RReg[1],&RReg[4],&RReg[5]);

            rplDropData(6);
            rplNewRealFromRRegPush(2);
            rplNewRealFromRRegPush(1);
            rplCheckResultAndError(&RReg[2]);
            rplCheckResultAndError(&RReg[1]);
        }
        else {
            // ERROR: no soluton
            rplDropData(6);
            rplError(ERR_IABCUV_NO_SOLUTION);
            //DSTop=savestk; return;
            return;
        }
        return;
    }

    case TCHEBYCHEFF:
    case TCHEBYCHEFF2:
    case LEGENDRE:
    case HERMITE:
    case HERMITE2:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR arg=rplPeekData(1);

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISSYMBOLIC(*arg)||ISIDENT(*arg)) {
         rplSymbApplyOperator(CurOpcode,1);
         return;
        }

        if(!ISNUMBER(*arg)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        REAL rnum;
        rplReadNumberAsReal(arg,&rnum);
        if(!isintegerReal(&rnum)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }
        if(rnum.flags&F_NEGATIVE) {
            rplError(ERR_POSITIVEINTEGEREXPECTED);
            //rplError(ERR_ARGOUTSIDEDOMAIN);
            return;
        }
        if(!inBINTRange(&rnum)) {
            rplError(ERR_NUMBERTOOBIG);
            return;
        }

        BINT n=(BINT)rplReadNumberAsBINT(arg);
        /*
        if (n < 0) {
            rplError(ERR_POSITIVEINTEGEREXPECTED);
            return;
        }
        else */
        if (n > 65534) { // vector size limit
            rplError(ERR_NUMBERTOOBIG);
            return;
        }

        rplDropData(1);

        WORDPTR *savestk=DSTop; // Drop arguments in case of error

        if (n == 0) {
            rplPushData((WORDPTR)(one_bint));
            int elements = 1;
            WORDPTR newmat=rplMatrixCompose(0,elements);
            if(newmat) {
                rplDropData(elements);
                rplPushData(newmat);
            }
            if(!newmat || Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }
            return;
        }
        else if (n == 1) {
            switch (OPCODE(CurOpcode)) {
            case TCHEBYCHEFF:
            case LEGENDRE:
            case HERMITE2:
                rplPushData((WORDPTR)(one_bint));
                rplPushData((WORDPTR)(zero_bint));
                break;
            case HERMITE:
            case TCHEBYCHEFF2:
                rplPushData((WORDPTR)(two_bint));
                rplPushData((WORDPTR)(zero_bint));
                break;
            }
            int elements = 2;
            WORDPTR newmat=rplMatrixCompose(0,elements);
            if(newmat) {
                rplDropData(elements);
                rplPushData(newmat);
            }
            if(!newmat || Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }
            return;
        }
        else {
            // reserve space for 2 vectors of length n+1
            rplExpandStack(2*(n+1));
            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
                return;
            }

            BINT saveprec=Context.precdigits;

            BINT argdigits=(n+7)&~7;

            if(argdigits>MAX_USERPRECISION) {
                argdigits=MAX_USERPRECISION;
            }

            argdigits=(argdigits>Context.precdigits)? argdigits:Context.precdigits;
            //   AUTOMATICALLY INCREASE PRECISION TEMPORARILY

            Context.precdigits=argdigits;


            BINT i=0, j=0;
            for (i = 0; i < 2; ++i)
            {
                // polynomial has n+1 elements
                for (j = 0; j <= n; ++j) {
                    rplPushData((WORDPTR)(zero_bint)); // fill with ZERO
                }
            }

            int evenodd = n%2;
            int ii = 0;
            int cur = ii%2;
            if  (!evenodd) cur = 1 - cur;
            int oth = 1 - cur;
            // todo populate n=0 and n=1
            switch (OPCODE(CurOpcode)) {
            case TCHEBYCHEFF:
            case LEGENDRE:
            case HERMITE2:
                rplOverwriteData(cur*(n+1)+1,(WORDPTR)(one_bint)); //n=0
                rplOverwriteData(oth*(n+1)+2,(WORDPTR)(one_bint)); //n=1
                break;
            case HERMITE:
            case TCHEBYCHEFF2:
                rplOverwriteData(cur*(n+1)+1,(WORDPTR)(one_bint)); //n=0
                rplOverwriteData(oth*(n+1)+2,(WORDPTR)(two_bint)); //n=1
                break;
            }

            // recrsive formula
            rplNumberToRReg(2, (WORDPTR)(two_bint));
            for (i = 2; i < n+1; ++i) {         // i=n+1
                rplLoadBINTAsReal(i-1, &RReg[5]);   // n
                rplLoadBINTAsReal(2*i-1, &RReg[6]); // 2n+1
                rplLoadBINTAsReal(i, &RReg[7]);     // n+1

                // switch via i mod 2
                int cur = i%2;
                if  (!evenodd) cur = 1 - cur;
                int oth = 1-cur;
                for (j = i; j >= 0; --j) {
                    rplNumberToRReg(0, rplPeekData(cur*(n+1)+j+1)); //previous
                    if (j > 0) {
                        rplNumberToRReg(1, rplPeekData(oth*(n+1)+j)); // x*current (=shift left)
                    }
                    else {
                        rplNumberToRReg(1, (WORDPTR)(zero_bint)); // the last is zero
                    }
                    switch (OPCODE(CurOpcode)) {
                    case TCHEBYCHEFF:
                    case TCHEBYCHEFF2:
                        mulReal(&RReg[3], &RReg[2], &RReg[1]); // 2*x*current
                        subReal(&RReg[4], &RReg[3], &RReg[0]); // 2*x*current - previous
                        break;
                    case LEGENDRE:
                        mulReal(&RReg[3], &RReg[5], &RReg[0]); // n*previous
                        mulReal(&RReg[0], &RReg[6], &RReg[1]); // (2n+1)*x*current
                        subReal(&RReg[1], &RReg[0], &RReg[3]); // (2n+1)*x*current - n*previous
                        divReal(&RReg[4], &RReg[1], &RReg[7]); // 2*x*current - 2*n*previous
                        break;
                    case HERMITE:
                        mulReal(&RReg[3], &RReg[5], &RReg[0]); // n*previous
                        subReal(&RReg[0], &RReg[1], &RReg[3]); // x*current - n*previous
                        mulReal(&RReg[4], &RReg[0], &RReg[2]); // 2*x*current - 2*n*previous
                        break;
                    case HERMITE2:
                        mulReal(&RReg[3], &RReg[5], &RReg[0]); // n*previous
                        subReal(&RReg[4], &RReg[1], &RReg[3]); // x*current - n*previous
                        break;
                    }

                    rplCheckResultAndError(&RReg[4]); // next
                    WORDPTR newnumber=rplNewReal(&RReg[4]);
                    if(!newnumber || Exceptions) {
                        if(DSTop>savestk) DSTop=savestk;
                        Context.precdigits=saveprec;
                        return;
                    }
                    rplOverwriteData(cur*(n+1)+j+1,newnumber); // set value

                }
            }
            // we are done. next create vector
            int elements = n+1;
            rplDropData(elements); // drop the 2nd exploded vector from n=previous
            Context.precdigits=saveprec;
            WORDPTR newmat=rplMatrixCompose(0,elements); //create vector from stack

            if(newmat) {
                rplDropData(elements);
                rplPushData(newmat);
            }
            else {
                if(DSTop>savestk) DSTop=savestk;
                return;
            }

        }
        return;
    }

    case PDIV2:
    case DIV2:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR *savestk=DSTop; // Drop arguments in case of error

        WORDPTR arg1=rplPeekData(2);
        WORDPTR arg2=rplPeekData(1);

        // POLYNOMIAL DIVISION arg1/arg2 = quot , remainder
        if(ISMATRIX(*arg1) && ISMATRIX(*arg2)) {
            BINT rows1=MATROWS(arg1[1]),cols1=MATCOLS(arg1[1]);
            BINT rows2=MATROWS(arg2[1]),cols2=MATCOLS(arg2[1]);
            // Check for vector only
            if(rows1 || rows2) {
                if(DSTop>savestk) DSTop=savestk;
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            // only numbers allowed
            BINT f;
            for(f=0;f<cols1;++f) {
                WORDPTR entry=rplMatrixFastGet(arg1,1,f+1);
                if(!ISNUMBER(*entry)) {
                    if(DSTop>savestk) DSTop=savestk;
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }
            for(f=0;f<cols2;++f) {
                WORDPTR entry=rplMatrixFastGet(arg2,1,f+1);
                if(!ISNUMBER(*entry)) {
                    if(DSTop>savestk) DSTop=savestk;
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }

            // Eliminate leading zeros to get the real order
            BINT leading_zeroes_arg1 = 0, leading_zeroes_arg2 = 0;
            for(f=0;f<cols1;++f) {
                WORDPTR entry=rplMatrixFastGet(arg1,1,f+1);
                rplNumberToRReg(0, entry);
                if (iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg1;
                } else { break; }
            }
            for(f=0;f<cols2;++f) {
                WORDPTR entry=rplMatrixFastGet(arg2,1,f+1);
                rplNumberToRReg(0, entry);
                if (iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg2;
                } else { break; }
            }

            if (Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
                return;
            }

            // now we know all leading zeroes
            if ((cols1-leading_zeroes_arg1) < (cols2-leading_zeroes_arg2)) {
                rplPushData((WORDPTR)(zero_bint));
                BINT elements = 1;
                WORDPTR newmat=rplMatrixCompose(0,elements);
                if(newmat) {
                    rplDropData(elements);
                    rplOverwriteData(1,rplPeekData(2));
                    rplOverwriteData(2,newmat);
                }
            } else {
                BINT saveprec=Context.precdigits;
                BINT argdigits=(saveprec+2*cols2+7)&~7;
                if(argdigits>MAX_USERPRECISION) {
                    argdigits=MAX_USERPRECISION;
                }
                argdigits=(argdigits>Context.precdigits)? argdigits:Context.precdigits;
                //   AUTOMATICALLY INCREASE PRECISION TEMPORARILY
                Context.precdigits=argdigits;

                // copy dividend
                for(f=leading_zeroes_arg1;f<cols1;++f) {
                    WORDPTR entry=rplMatrixFastGet(arg1,1,f+1);
                    rplPushData(entry);
                }
                WORDPTR normalizer = rplMatrixFastGet(arg2,1,leading_zeroes_arg2+1); // divisor[0]
                rplNumberToRReg(0, normalizer);
                for (f = 0; f < (cols1-leading_zeroes_arg1)-(cols2-leading_zeroes_arg2)+1; ++f) {
                    rplNumberToRReg(1,rplPeekData((cols1-leading_zeroes_arg1)-f)); // out[i]
                    divReal(&RReg[2], &RReg[1], &RReg[0]);
                    WORDPTR newnumber=rplNewReal(&RReg[2]);
                    if(!newnumber || Exceptions) {
                        if(DSTop>savestk) DSTop=savestk;
                        Context.precdigits=saveprec;
                        return;
                    }
                    rplOverwriteData((cols1-leading_zeroes_arg1)-f, newnumber); //out[i] /= normalizer
                   if (!iszeroReal(&RReg[2])) {             // coef = RReg[2]
                        BINT j;
                        for (j =leading_zeroes_arg2+1; j < cols2; ++j) {
                            WORDPTR divj = rplMatrixFastGet(arg2,1,j+1); // divisor[j]
                            rplNumberToRReg(1, divj);
                            mulReal(&RReg[1], &RReg[1], &RReg[2]);
                            rplNumberToRReg(3,rplPeekData((cols1-leading_zeroes_arg1)-f-(j-leading_zeroes_arg2))); // out[i]
                            subReal(&RReg[3], &RReg[3], &RReg[1]);
                            WORDPTR newnumber=rplNewReal(&RReg[3]);
                            if(!newnumber || Exceptions) {
                                if(DSTop>savestk) DSTop=savestk;
                                Context.precdigits=saveprec;
                                return;
                            }
                            rplOverwriteData((cols1-leading_zeroes_arg1)-f-(j-leading_zeroes_arg2), newnumber);
                        }
                    }
                }
                BINT elements_remainder = (cols2-leading_zeroes_arg2)-1;
                BINT leading_zeroes_remainder = 0;
                for (f = 0; f < elements_remainder; ++f) {
                    WORDPTR entry=rplPeekData(elements_remainder-f);
                    rplNumberToRReg(0, entry);
                    if (iszeroReal(&RReg[0])) {
                        ++leading_zeroes_remainder;
                    } else { break; }
                }
                BINT nrem = elements_remainder-leading_zeroes_remainder;
                if (nrem < 1) {
                   nrem=1;
                }
                WORDPTR remainder=rplMatrixCompose(0,nrem);
                if(remainder) {
                    rplDropData(elements_remainder);
                    BINT elements_quotient = (cols1-leading_zeroes_arg1)-(cols2-leading_zeroes_arg2)+1;
                    WORDPTR quotient=rplMatrixCompose(0,elements_quotient);
                    if(quotient) {
                        rplDropData(elements_quotient);
                        rplOverwriteData(2,quotient);
                        rplOverwriteData(1,remainder);
                    }
                    else {
                        if(DSTop>savestk) DSTop=savestk;
                        Context.precdigits=saveprec;
                        return;
                    }
                }
                else {
                    if(DSTop>savestk) DSTop=savestk;
                    Context.precdigits=saveprec;
                    return;
                }
                Context.precdigits=saveprec;
            }
        }
        else {
            if(DSTop>savestk) DSTop=savestk;
            rplError(ERR_VECTOROFNUMBERSEXPECTED);
        }
        return;
    }

    case PINT:
    case PDER:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR poly=rplPeekData(1);

        if(ISMATRIX(*poly)){

            BINT rows=MATROWS(poly[1]),cols=MATCOLS(poly[1]);

            if(rows) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }
            BINT f;

            for(f=1;f<=cols;++f) {
                WORDPTR entry=rplMatrixFastGet(poly,1,f);
                if(!ISNUMBER(*entry)) {
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }
            // DO IT ALL WITH REALS
            WORDPTR *savestk=DSTop; // Drop arguments in case of error

            BINT leading_zeroes_arg = 0;
            for(f=0;f<cols;++f) {
                WORDPTR entry=rplMatrixFastGet(poly,1,f+1);
                rplNumberToRReg(0, entry);
                if (iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg;
                } else { break; }
            }

            // copy trimmed polynomial and do operation
            const BINT degree=cols-leading_zeroes_arg-1;
            BINT idegree, endcol, nout;
            if (OPCODE(CurOpcode) == PDER) {
                idegree = degree;
                endcol = cols-1;
                nout = degree;
            } else {
                idegree = degree+1;
                endcol = cols;
                nout = degree+2;
            }
            for(f=leading_zeroes_arg;f<endcol;++f,--idegree) {
                WORDPTR entry=rplMatrixFastGet(poly,1,f+1);
                rplNumberToRReg(1, entry);
                rplBINTToRReg(0,idegree);
                if (OPCODE(CurOpcode) == PDER) {
                    mulReal(&RReg[2], &RReg[1], &RReg[0]);
                } else {
                    divReal(&RReg[2], &RReg[1], &RReg[0]);
                }
                WORDPTR newnumber=rplNewReal(&RReg[2]);
                if(!newnumber || Exceptions) {
                    if(DSTop>savestk) DSTop=savestk;
                    return;
                }
                rplPushData(newnumber);
            }
            if (OPCODE(CurOpcode) == PINT) {
                rplPushData((WORDPTR)(zero_bint));
            }
            WORDPTR pout=rplMatrixCompose(0,nout);
            if(!pout || Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
                return;
            }
            rplDropData(nout);
            rplOverwriteData(1,pout);
        }
        else {
            rplError(ERR_VECTOREXPECTED);
        }
        return;
    }

    case PADD:
    case PSUB:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR arg1=rplPeekData(2);
        WORDPTR arg2=rplPeekData(1);

        // POLYNOMIAL DIVISION
        if(ISMATRIX(*arg1) && ISMATRIX(*arg2)) {
            BINT rows1=MATROWS(arg1[1]),cols1=MATCOLS(arg1[1]);
            BINT rows2=MATROWS(arg2[1]),cols2=MATCOLS(arg2[1]);
            // Check for vector only
            if(rows1 || rows2) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            // only numbers allowed
            BINT f;
            for(f=0;f<cols1;++f) {
                WORDPTR entry=rplMatrixFastGet(arg1,1,f+1);
                if(!ISNUMBER(*entry)) {
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }
            for(f=0;f<cols2;++f) {
                WORDPTR entry=rplMatrixFastGet(arg2,1,f+1);
                if(!ISNUMBER(*entry)) {
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }

            // Eliminate leading zeros to get the real degree
            BINT leading_zeroes_arg1 = 0, leading_zeroes_arg2 = 0;
            for(f=0;f<cols1;++f) {
                WORDPTR entry=rplMatrixFastGet(arg1,1,f+1);
                rplNumberToRReg(0, entry);
                if (iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg1;
                } else { break; }
            }
            for(f=0;f<cols2;++f) {
                WORDPTR entry=rplMatrixFastGet(arg2,1,f+1);
                rplNumberToRReg(0, entry);
                if (iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg2;
                } else { break; }
            }

            if (Exceptions) {
                return;
            }

            BINT nelem1 = cols1-leading_zeroes_arg1; // degree1 = nelem1 -1;
            BINT nelem2 = cols2-leading_zeroes_arg2; // degree2 = nelem2 -1;

            BINT nelem3 = nelem1 > nelem2 ? nelem1 : nelem2; // degree = max(degree1, degree2)

            WORDPTR *savestk=DSTop; // Drop arguments in case of error

            for (f  = 0; f < nelem3; ++f) {
                BINT i1 = cols1-nelem3+f;
                BINT i2 = cols2-nelem3+f;
                if (i1 < leading_zeroes_arg1) {
                    rplBINTToRReg(1,0);
                } else {
                    WORDPTR entry=rplMatrixFastGet(arg1,1,i1+1);
                    rplNumberToRReg(1, entry);
                }
                if (i2 < leading_zeroes_arg2) {
                    rplBINTToRReg(2,0);
                } else {
                    WORDPTR entry=rplMatrixFastGet(arg2,1,i2+1);
                    rplNumberToRReg(2, entry);
                }
                if (OPCODE(CurOpcode) == PADD) {
                    addReal(&RReg[0], &RReg[1], &RReg[2]);
                } else {
                    subReal(&RReg[0], &RReg[1], &RReg[2]);
                }
                WORDPTR newnumber=rplNewReal(&RReg[0]);
                if(!newnumber || Exceptions) {
                    if(DSTop>savestk) DSTop=savestk;
                    return;
                }
                rplPushData(newnumber);
            }
            BINT leading_zeroes_pout = 0;
            for (f = 0; f < nelem3; ++f) {
                WORDPTR entry=rplPeekData(nelem3-f);
                rplNumberToRReg(0, entry);
                if (iszeroReal(&RReg[0])) {
                    ++leading_zeroes_pout;
                } else { break; }
            }
            BINT nout = nelem3-leading_zeroes_pout;
            if (nout < 1) {
               nout=1;
            }
            WORDPTR poly=rplMatrixCompose(0,nout);
            if(!poly || Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
                return;
            }
            rplDropData(nelem3+1);
            rplOverwriteData(1,poly);

        }
        else {
            rplError(ERR_VECTOROFNUMBERSEXPECTED);
        }
        return;
    }

    case PMUL:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR arg1=rplPeekData(2);
        WORDPTR arg2=rplPeekData(1);

        // POLYNOMIAL DIVISION
        if(ISMATRIX(*arg1) && ISMATRIX(*arg2)) {
            BINT rows1=MATROWS(arg1[1]),cols1=MATCOLS(arg1[1]);
            BINT rows2=MATROWS(arg2[1]),cols2=MATCOLS(arg2[1]);
            // Check for vector only
            if(rows1 || rows2) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            // only numbers allowed
            BINT f;
            for(f=0;f<cols1;++f) {
                WORDPTR entry=rplMatrixFastGet(arg1,1,f+1);
                if(!ISNUMBER(*entry)) {
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }
            for(f=0;f<cols2;++f) {
                WORDPTR entry=rplMatrixFastGet(arg2,1,f+1);
                if(!ISNUMBER(*entry)) {
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }

            // Eliminate leading zeros to get the real degree
            BINT leading_zeroes_arg1 = 0, leading_zeroes_arg2 = 0;
            for(f=0;f<cols1;++f) {
                WORDPTR entry=rplMatrixFastGet(arg1,1,f+1);
                rplNumberToRReg(0, entry);
                if (iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg1;
                } else { break; }
            }
            for(f=0;f<cols2;++f) {
                WORDPTR entry=rplMatrixFastGet(arg2,1,f+1);
                rplNumberToRReg(0, entry);
                if (iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg2;
                } else { break; }
            }

            if (Exceptions) {
                return;
            }

            BINT nelem1 = cols1-leading_zeroes_arg1; // degree1 = nelem1 -1;
            BINT nelem2 = cols2-leading_zeroes_arg2; // degree2 = nelem2 -1;

            BINT nout = nelem1 + nelem2 - 1; // nout = degree+1 = degree1+degree2+1

            WORDPTR *savestk=DSTop; // Drop arguments in case of error

            BINT i1, i2, iout, g;

            for (f = 0; f < nout; ++f) {
                rplPushData((WORDPTR)(zero_bint));
            }

            for(f=leading_zeroes_arg1, i1=0;f<cols1;++f, ++i1) {
                WORDPTR entry=rplMatrixFastGet(arg1,1,f+1);
                rplNumberToRReg(1, entry);
                if (!iszeroReal(&RReg[1])) {
                    for(g=leading_zeroes_arg2, i2=0;g<cols2;++g, ++i2) {
                        WORDPTR entry=rplMatrixFastGet(arg2,1,g+1);
                        rplNumberToRReg(2, entry);
                        if (!iszeroReal(&RReg[2])) {
                            mulReal(&RReg[0], &RReg[1], &RReg[2]);
                            iout = i1+i2;
                            rplNumberToRReg(3, rplPeekData(nout-iout));
                            addReal(&RReg[2], &RReg[0], &RReg[3]);
                            WORDPTR newnumber=rplNewReal(&RReg[2]);
                            if(!newnumber || Exceptions) {
                                if(DSTop>savestk) DSTop=savestk;
                                return;
                            }
                            rplOverwriteData(nout-iout, newnumber);
                        }
                    }
                }
            }
            WORDPTR poly=rplMatrixCompose(0,nout);
            if(!poly || Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
                return;
            }
            rplDropData(nout+1);
            rplOverwriteData(1,poly);

        }
        else {
            rplError(ERR_VECTOROFNUMBERSEXPECTED);
        }
        return;
    }


    case MIN:
    {
        // COMPARE ANY 2 OBJECTS AND KEEP THE SMALLEST

        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(ISLIST(*rplPeekData(2))||ISLIST(*rplPeekData(1))) {
            // THIS IS A COMPOSITE, NEED TO RUN AN RPL LOOP
            rplListBinaryDoCmd();
            return;
        }

        if(ISIDENT(*rplPeekData(1)) || ISSYMBOLIC(*rplPeekData(2)) || ISIDENT(*rplPeekData(1)) || ISSYMBOLIC(*rplPeekData(1)))
        {
            rplSymbApplyOperator(CurOpcode,2);
            return;
        }


        WORDPTR *saveStk=DSTop;

        rplPushData(rplPeekData(2));
        rplPushData(rplPeekData(2));

        rplCallOvrOperator(CMD_OVR_LTE);
        if(Exceptions) {
            // CLEANUP THE STACK BEFORE RETURNING
            DSTop=saveStk;
            return;
        }

        if(rplIsFalse(rplPeekData(1))) {
            // KEEP THE SECOND OBJECT
            rplOverwriteData(3,rplPeekData(2));
        }

        rplDropData(2);

        return;
    }


    case MAX:
    {
        // COMPARE ANY 2 OBJECTS AND KEEP THE SMALLEST

        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(ISLIST(*rplPeekData(2))||ISLIST(*rplPeekData(1))) {
            // THIS IS A COMPOSITE, NEED TO RUN AN RPL LOOP
            rplListBinaryDoCmd();
            return;
        }

        if(ISIDENT(*rplPeekData(1)) || ISSYMBOLIC(*rplPeekData(2)) || ISIDENT(*rplPeekData(1)) || ISSYMBOLIC(*rplPeekData(1)))
        {
            rplSymbApplyOperator(CurOpcode,2);
            return;
        }


        WORDPTR *saveStk=DSTop;

        rplPushData(rplPeekData(2));
        rplPushData(rplPeekData(2));

        rplCallOvrOperator(CMD_OVR_GTE);
        if(Exceptions) {
            // CLEANUP THE STACK BEFORE RETURNING
            DSTop=saveStk;
            return;
        }

        if(rplIsFalse(rplPeekData(1))) {
            // KEEP THE SECOND OBJECT
            rplOverwriteData(3,rplPeekData(2));
        }

        rplDropData(2);

        return;
    }


    case RND:
    case TRNC:
    {
    if(rplDepthData()<2) {
        rplError(ERR_BADARGCOUNT);

        return;
    }
    WORDPTR arg=rplPeekData(2);
    WORDPTR ndig=rplPeekData(1);

    if(ISLIST(*arg)||ISLIST(*ndig)) {
        rplListBinaryDoCmd();
        return;
    }

    if(ISIDENT(*arg) || ISSYMBOLIC(*arg) || ISIDENT(*ndig) || ISSYMBOLIC(*ndig) ) {
        rplSymbApplyOperator(CurOpcode,2);
        return;
    }

    if(!ISNUMBER(*ndig)) {
        rplError(ERR_INTEGEREXPECTED);
        return;
    }


    WORDPTR *savestk=DSTop;
    BINT64 nd=rplReadNumberAsBINT(ndig),isunit=0,unitlevels=0;
    if(Exceptions) return;

    if(ISUNIT(*arg)) {
        unitlevels=rplUnitExplode(arg);
        if(Exceptions) { DSTop=savestk; return; }
        rplPushData(rplPeekData(unitlevels));
        rplPushData(rplPeekData(1));
        arg=rplPeekData(1);
        isunit=1;
    }


    if(ISNUMBER(*arg)) {
        REAL r;
        rplReadNumberAsReal(arg,&r);
        if(Exceptions) return;

        if(OPCODE(CurOpcode)==RND) roundReal(&RReg[0],&r,nd);
        else truncReal(&RReg[0],&r,nd);

        WORDPTR newresult;

        if(isintegerReal(&RReg[0])) {
        if(inBINT64Range(&RReg[0])) {
            BINT64 res=getBINT64Real(&RReg[0]);
            newresult=rplNewBINT(res,DECBINT);
        }
        else newresult=rplNewRealFromRReg(0);
        } else newresult=rplNewRealFromRReg(0);

        if(!newresult) return;
        rplDropData(1);
        rplOverwriteData(1,newresult);


    } else if(ISCOMPLEX(*arg)) {
        REAL Rarg,Iarg;
        BINT cclass=rplComplexClass(arg);
        rplReadCNumberAsReal(arg,&Rarg);
        rplReadCNumberAsImag(arg,&Iarg);

        switch(cclass) {


        case CPLX_NORMAL:
        {
            // ROUND REAL AND IMAGINARY PARTS INDEPENDENTLY
            if(OPCODE(CurOpcode)==RND) {
                roundReal(&RReg[0],&Rarg,nd);
                roundReal(&RReg[1],&Iarg,nd);
            }
            else {
                truncReal(&RReg[0],&Rarg,nd);
                truncReal(&RReg[1],&Iarg,nd);
            }
            WORDPTR newresult=rplNewComplex(&RReg[0],&RReg[1],ANGLENONE);
            if(!newresult) return;

            rplDropData(1);
            rplOverwriteData(1,newresult);
            break;
        }
        case CPLX_POLAR:
        {
         // ONLY ROUND THE MAGNITUDE, LEAVE THE ANGLE AS IS
            if(OPCODE(CurOpcode)==RND) {
                roundReal(&RReg[0],&Rarg,nd);
            }
            else {
                truncReal(&RReg[0],&Rarg,nd);
            }
            WORDPTR newresult=rplNewComplex(&RReg[0],&Iarg,rplPolarComplexMode(arg));
            if(!newresult) return;

            rplDropData(1);
            rplOverwriteData(1,newresult);
            break;

        }
        default:
        case CPLX_INF:
        case CPLX_MALFORMED:
        case CPLX_NAN:
        case CPLX_UNDINF:
        case CPLX_ZERO:
            return; // NOTHING TO ROUND

        }


    } else if(ISMATRIX(*arg)) {
        // ROUND EVERY ELEMENT IN THE MATRIX, AND APPLY THE OPERATOR TO EVERY SYMBOLIC ELEMENT IN THE MATRIX
        WORDPTR *a;
        // DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
        // AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
        a=DSTop-2;

        // a IS THE MATRIX

        // CHECK DIMENSIONS

        BINT rowsa=MATROWS(*(*a+1)),colsa=MATCOLS(*(*a+1));

        BINT totalelements=(rowsa)? rowsa*colsa:colsa;

        BINT j;

        // DO THE ELEMENT-BY-ELEMENT OPERATION

        for(j=1;j<=totalelements;++j) {
         rplPushData(rplMatrixFastGet(*a,1,j));
         WORDPTR arg=rplPeekData(1);
             if(ISIDENT(*arg) || ISSYMBOLIC(*arg) || ISIDENT(*ndig) || ISSYMBOLIC(*ndig) ) {
                 rplPushData(a[1]);
                 rplSymbApplyOperator(CurOpcode,2);
                 if(Exceptions) { DSTop=savestk; return; }
                 continue;
             }

             if(ISNUMBER(*arg)) {
                 REAL r;
                 rplReadNumberAsReal(arg,&r);
                 if(Exceptions) { DSTop=savestk; return; }

                 if(OPCODE(CurOpcode)==RND) roundReal(&RReg[0],&r,nd);
                 else truncReal(&RReg[0],&r,nd);

                 WORDPTR newresult;

                 if(isintegerReal(&RReg[0])) {
                 if(inBINT64Range(&RReg[0])) {
                     BINT64 res=getBINT64Real(&RReg[0]);
                     newresult=rplNewBINT(res,DECBINT);
                 }
                 else newresult=rplNewRealFromRReg(0);
                 } else newresult=rplNewRealFromRReg(0);

                 if(!newresult) { DSTop=savestk; return; }
                 rplOverwriteData(1,newresult);
                 continue;
             }

             if(ISCOMPLEX(*arg)) {
                 REAL Rarg,Iarg;
                 BINT cclass=rplComplexClass(arg);
                 rplReadCNumberAsReal(arg,&Rarg);
                 rplReadCNumberAsImag(arg,&Iarg);

                 switch(cclass) {


                 case CPLX_NORMAL:
                 {
                     // ROUND REAL AND IMAGINARY PARTS INDEPENDENTLY
                     if(OPCODE(CurOpcode)==RND) {
                         roundReal(&RReg[0],&Rarg,nd);
                         roundReal(&RReg[1],&Iarg,nd);
                     }
                     else {
                         truncReal(&RReg[0],&Rarg,nd);
                         truncReal(&RReg[1],&Iarg,nd);
                     }
                     WORDPTR newresult=rplNewComplex(&RReg[0],&RReg[1],ANGLENONE);
                     if(!newresult) { DSTop=savestk; return; }

                     rplOverwriteData(1,newresult);
                     break;
                 }
                 case CPLX_POLAR:
                 {
                  // ONLY ROUND THE MAGNITUDE, LEAVE THE ANGLE AS IS
                     if(OPCODE(CurOpcode)==RND) {
                         roundReal(&RReg[0],&Rarg,nd);
                     }
                     else {
                         truncReal(&RReg[0],&Rarg,nd);
                     }
                     WORDPTR newresult=rplNewComplex(&RReg[0],&Iarg,rplPolarComplexMode(arg));
                     if(!newresult) { DSTop=savestk; return; }

                     rplOverwriteData(1,newresult);
                     break;

                 }
                 default:
                 case CPLX_INF:
                 case CPLX_MALFORMED:
                 case CPLX_NAN:
                 case CPLX_UNDINF:
                 case CPLX_ZERO:
                     break; // NOTHING TO ROUND

                 }

                continue;
             }


         }

        WORDPTR newmat=rplMatrixCompose(rowsa,colsa);
        DSTop=a+2;
        if(!newmat) return;
        rplOverwriteData(2,newmat);
        rplDropData(1);
    }
    else {
    rplError(ERR_BADARGTYPE);
    return;
    }

    if(isunit) {
        rplOverwriteData(unitlevels+1,rplPeekData(1));
        rplDropData(1);
        WORDPTR newunit=rplUnitAssemble(unitlevels);
        if(!newunit) { DSTop=savestk; return; }
        rplDropData(unitlevels+1);
        rplOverwriteData(1,newunit);

    }


    return;
    }

case DIGITS:
        // EXTRACT DIGITS FROM A REAL NUMBER
        // GIVEN THE NUMBER AND POSITION START/END
        // POSITION IS GIVEN IN 10s POWERS (0 = UNITY, 1 = TENS, ETC)
    {
     if(rplDepthData()<3) {
         rplError(ERR_BADARGCOUNT);
         return;
     }


     if(ISLIST(*rplPeekData(3)))
     {
         rplListMultiArgDoCmd(3);
         return;
     }


     if(!ISNUMBER(*rplPeekData(3))) {
         rplError(ERR_REALEXPECTED);
         return;
     }

     if(!ISNUMBER(*rplPeekData(2)) || !ISNUMBER(*rplPeekData(1))) {
         rplError(ERR_INTEGEREXPECTED);
         return;
     }

     REAL re;
     BINT64 start,end;

     rplReadNumberAsReal(rplPeekData(3),&re);
     start=rplReadNumberAsBINT(rplPeekData(2));
     end=rplReadNumberAsBINT(rplPeekData(1));

     if(Exceptions) return;

     if(start<end) {
         BINT64 tmp=start;
         start=end;
         end=tmp;
     }

     re.exp-=end;
     ipReal(&RReg[0],&re,1);
     RReg[0].exp+=end;
     RReg[0].exp-=start+1;
     fracReal(&RReg[1],&RReg[0]);
     RReg[1].exp+=(start-end)+1;
     rplDropData(3);
     if(inBINT64Range(&RReg[1])) { start=getBINT64Real(&RReg[1]);  rplNewBINTPush(start,DECBINT); }
     else rplNewRealFromRRegPush(1);

     return;
    }



case PROOT:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR vect_val=rplPeekData(1);



        if(ISLIST(*vect_val)) {
            rplListUnaryDoCmd();
            return;
        }
        else if(ISMATRIX(*vect_val)){

            BINT cplxmode=rplTestSystemFlag(FL_COMPLEXMODE);
            rplSetSystemFlag(FL_COMPLEXMODE);

            BINT rows=MATROWS(vect_val[1]),cols=MATCOLS(vect_val[1]);

            if(rows) {
                if(!cplxmode) rplClrSystemFlag(FL_COMPLEXMODE);
                rplError(ERR_VECTOREXPECTED);
                return;
            }
            BINT f;
            WORDPTR *savestk=DSTop;

            for(f=0;f<cols;++f) {
                WORDPTR entry=rplMatrixFastGet(vect_val,1,f+1);
                if(!ISNUMBERCPLX(*entry)) {
                    if(!cplxmode) rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop=savestk;
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
                rplPushData(entry);
            }

            WORDPTR solution;
            REAL re;
            for(f=1;f<cols-1;++f) {

            if(f>1) {
                // TEST IF PREVIOUS SOLUTION IS ALSO A SOLUTION OF THE DEFLATED POLYNOMIAL

                solution=rplPolyEvalEx(savestk,cols-f,DSTop-1);
                if(!solution || Exceptions) {
                    if(!cplxmode) rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop=savestk;
                    return;
                }
                rplPushData(solution);

                rplCallOvrOperator(CMD_OVR_ABS);
                if(Exceptions) {
                    if(!cplxmode) rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop=savestk;
                    return;
                }
                rplReadNumberAsReal(rplPopData(),&re);
                if(Exceptions) {
                    if(!cplxmode) rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop=savestk;
                    return;
                }

                if(iszeroReal(&re) || (intdigitsReal(&re)<-Context.precdigits)) {
                    // THE CURRENT ROOT HAS MULTIPLICITY
                    rplPushData(rplPeekData(1));


                // DEFLATE THE POLYNOMIAL
                rplPolyDeflateEx(savestk,cols-f,DSTop-1);
                if(Exceptions) {
                    if(!cplxmode) rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop=savestk;
                    return;
                }
                continue;
                }

            }

            solution=rplPolyRootEx(savestk,cols-f);
            if(!solution || Exceptions) {
                if(!cplxmode) rplClrSystemFlag(FL_COMPLEXMODE);
                DSTop=savestk;
                return;
            }

            // WE HAVE ONE SOLUTION!
            rplPushData(solution);
            if(Exceptions) {
                if(!cplxmode) rplClrSystemFlag(FL_COMPLEXMODE);
                DSTop=savestk;
                return;
            }
            // DEFLATE THE POLYNOMIAL
            rplPolyDeflateEx(savestk,cols-f,DSTop-1);
            if(Exceptions) {
                if(!cplxmode) rplClrSystemFlag(FL_COMPLEXMODE);
                DSTop=savestk;
                return;
            }

            // NOW THE POLYNOMIAL IS ONE DEGREE LESS
            }

            // HERE WE HAVE ALL ROOTS OF THE POLYNOMIAL EXCEPT THE LAST ONE
            // THE POLYNOMIAL OF DEGREE 1 IS a0*X+a1 = 0
            // THEREFORE THE LAST ROOT IS X=-a1/a0
            rplPushData(savestk[1]);
            rplPushData(savestk[0]);
            if(Exceptions) {
                if(!cplxmode) rplClrSystemFlag(FL_COMPLEXMODE);
                DSTop=savestk;
                return;
            }

            rplCallOvrOperator(CMD_OVR_DIV);
            if(Exceptions) {
                if(!cplxmode) rplClrSystemFlag(FL_COMPLEXMODE);
                DSTop=savestk;
                return;
            }

            rplCallOvrOperator(CMD_OVR_NEG);
            if(Exceptions) {
                if(!cplxmode) rplClrSystemFlag(FL_COMPLEXMODE);
                DSTop=savestk;
                return;
            }

            BINT doerror=0;
            if(!cplxmode) {
                // ISSUE AN ERROR IF ANY OF THE ROOTS ARE COMPLEX
            for(f=1;f<cols;++f) {
                if(ISCOMPLEX(*rplPeekData(f))) { doerror=1; break; }
            }
            }

            solution=rplMatrixCompose(0,cols-1);
            if(!solution) {
                if(!cplxmode) rplClrSystemFlag(FL_COMPLEXMODE);
                DSTop=savestk;
                return;
            }
            DSTop=savestk;
            rplOverwriteData(1,solution);
            if(!cplxmode) {
                rplClrSystemFlag(FL_COMPLEXMODE);
                if(doerror) rplError(ERR_COMPLEXRESULT);
            }

            return;

        }
        rplError(ERR_VECTOROFNUMBERSEXPECTED);
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
    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        return;

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(MENUNUMBER(MenuCodeArg)>5) {
            RetNum=ERR_NOTMINE;
            return;
        }
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
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



