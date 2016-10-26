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
        ERR(POSITIVE_INTEGER_EXPECTED,2)

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
    CMD(HERMITE,MKTOKENINFO(7,TITYPE_FUNCTION,1,2))

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
INCLUDE_ROMOBJECT(lib64_menu);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib64_menu,
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

    if(ISLIST(*arg)) {
        rplListUnaryDoCmd();
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
            rplListBinaryDoCmd(arg1,arg2);
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
                rplListBinaryDoCmd(arg,mod);
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
            rplListBinaryDoCmd(arg1,pct);
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
                rplListBinaryDoCmd(old_val,new_val);
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
                rplListBinaryDoCmd(old_val,new_val);
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
                rplListBinaryDoCmd(arg1,arg2);
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

            /*
            if(ISLIST(*real_val)) {
                rplListBinaryDoCmd(real_val,vect_val);
                return;
            }
            else if((ISIDENT(*vect_val) || ISSYMBOLIC(*vect_val)) || (ISIDENT(*real_val) || ISSYMBOLIC(*real_val))){
                    rplSymbApplyOperator(CurOpcode,2);
                    return;
            }
            else */ if(ISMATRIX(*vect_val) && ISNUMBER(*real_val) ){

                BINT rows=MATROWS(vect_val[1]),cols=MATCOLS(vect_val[1]);

                if(rows) {
                    rplError(ERR_VECTOREXPECTED);
                    return;
                }
                BINT f;

                for(f=0;f<cols;++f) {
                    WORDPTR entry=rplMatrixFastGet(vect_val,1,f+1);
                    if(!ISNUMBER(*entry)) {
                        rplError(ERR_VECTOROFNUMBERSEXPECTED);
                        return;
                    }
                }
                // DO IT ALL WITH REALS
                // use Horner scheme
                rplNumberToRReg(0,real_val);
                rplNumberToRReg(1,(WORDPTR)zero_bint);

                for(f=0;f<cols;++f) {
                    WORDPTR entry=rplMatrixFastGet(vect_val,1,f+1);
                    rplNumberToRReg(3,entry);
                    mulReal(&RReg[2], &RReg[1], &RReg[0]);
                    addReal(&RReg[1], &RReg[2], &RReg[3]);
                }

                WORDPTR newnumber=rplNewReal(&RReg[1]);
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

                WORDPTR *Firstelem=DSTop;

                rplPushData((WORDPTR)one_bint);
                if(Exceptions) {
                    DSTop=Firstelem;
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
                            return;
                        }
                        *(Firstelem+j)=newnumber;
                    }
                    if(Exceptions) {
                        DSTop=Firstelem;
                        return;
                    }
                }

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
//            rplListBinaryDoCmd(arg1,arg2);
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
    case LEGENDRE:
    case HERMITE:
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
            rplError(ERR_POSITIVE_INTEGER_EXPECTED);
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
            rplError(ERR_POSITIVE_INTEGER_EXPECTED);
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
                rplPushData((WORDPTR)(one_bint));
                rplPushData((WORDPTR)(zero_bint));
                break;
            case HERMITE:
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
            for (int i = 0; i < 2; ++i)
            {
                // polynomial has n+1 elements
                for (int j = 0; j <= n; ++j) {
                    rplPushData((WORDPTR)(zero_bint));
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
                rplOverwriteData(cur*(n+1)+1,(WORDPTR)(one_bint));
                rplOverwriteData(oth*(n+1)+2,(WORDPTR)(one_bint));
                break;
            case HERMITE:
                rplOverwriteData(cur*(n+1)+1,(WORDPTR)(one_bint));
                rplOverwriteData(oth*(n+1)+2,(WORDPTR)(two_bint));
                break;
            }

            // recrsive formula
            rplNumberToRReg(2, (WORDPTR)(two_bint));
            for (int i = 2; i < n+1; ++i) {         // i=n+1
                rplLoadBINTAsReal(i-1, &RReg[5]);   // n
                rplLoadBINTAsReal(2*i-1, &RReg[6]); // 2n+1
                rplLoadBINTAsReal(i, &RReg[7]);     // n+1

                // switch via i mod 2
                int cur = i%2;
                if  (!evenodd) cur = 1 - cur;
                int oth = 1-cur;
                for (int j = i; j >= 0; --j) {
                    rplNumberToRReg(0, rplPeekData(cur*(n+1)+j+1)); //previous
                    if (j > 0) {
                        rplNumberToRReg(1, rplPeekData(oth*(n+1)+j)); // x*current (=shift left)
                    }
                    else {
                        rplNumberToRReg(1, (WORDPTR)(zero_bint)); // the last is zero
                    }
                    switch (OPCODE(CurOpcode)) {
                    case TCHEBYCHEFF:
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
                    }

                    rplCheckResultAndError(&RReg[4]); // next
                    WORDPTR newnumber=rplNewReal(&RReg[4]);
                    if(!newnumber || Exceptions) {
                        if(DSTop>savestk) DSTop=savestk;
                        return;
                    }
                    rplOverwriteData(cur*(n+1)+j+1,newnumber);

                }
            }
            int elements = n+1;
            rplDropData(elements);
            WORDPTR newmat=rplMatrixCompose(0,elements);

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
        libGetInfo2(*DecompileObject,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_AUTOCOMPNEXT:
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
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR=(WORDPTR)lib64_menu;
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



