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
    CMD(MODRCL,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
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
    CMD(MULTMOD,MKTOKENINFO(7,TITYPE_FUNCTION,2,2))


// ADD MORE OPCODES HERE


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

const char const modulo_name[]="MOD";

#define MIN_SINT    -131072
#define MAX_SINT    +131071
#define MAX_BINT    +9223372036854775807LL
#define MIN_BINT    (-9223372036854775807LL-1LL)

// COUNT THE NUMBER OF BITS IN A POSITIVE INTEGER
static int rpl_log2(BINT64 number,int bits)
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

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

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
                newRealFromBINT(&RReg[0],2);
                if (ltReal(&num, &RReg[0])) {
                    newRealFromBINT(&num,2);
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

                if (OPCODE(CurOpcode) == IDIV2 || OPCODE(CurOpcode) == IQUOT) {
                    if(!isintegerReal(&a)) {
                        rplError(ERR_INTEGEREXPECTED);
                        return;
                    }
                    if(!isintegerReal(&m)) {
                        rplError(ERR_INTEGEREXPECTED);
                        return;
                    }
                }

                divmodReal(&RReg[7],&RReg[6],&a,&m);
                // correct negative remainder
                if(RReg[6].flags&F_NEGATIVE) {
                    addReal(&RReg[6],&RReg[6],&m);
                    newRealFromBINT(&RReg[0],1);
                    subReal(&RReg[7],&RReg[7],&RReg[0]);
                }

                rplDropData(2);

                if (OPCODE(CurOpcode) == IDIV2 || OPCODE(CurOpcode) == IQUOT) {
                    rplNewRealFromRRegPush(7);
                }
                if (OPCODE(CurOpcode) == IDIV2 || OPCODE(CurOpcode) == MOD) {
                    rplNewRealFromRRegPush(6);
                }

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

            if(ISBINT(*arg1) && ISBINT(*arg2)) {

                BINT64 a1=rplReadBINT(arg1);
                BINT64 a2=rplReadBINT(arg2);
                BINT64 r1,r2,r3,gcd;
                if(a1 < 0) a1 = -a1;
                if(a2 < 0) a2 = -a2;
                if(a1 > a2){
                    r1 = a1;
                    r2 = a2;
                }
                else {
                    r2 = a1;
                    r1 = a2;
                }
                if (r2 == (BINT64)0) {
                    rplError(ERR_MATHDIVIDEBYZERO);
                }
                // avoid swapping elements by loop unrolling
                BINT notfinished = 1;
                do {
                    if(r2!=0){
                        r3 = r1%r2;
                    }
                    else {
                        gcd = r1;
                        notfinished = 0;
                        break;
                    }
                    if(r3!=0){
                        r1 = r2%r3;
                    }
                    else {
                        gcd = r2;
                        notfinished = 0;
                        break;
                    }
                    if(r1!=0){
                        r2 = r3%r1;
                    }
                    else {
                        gcd = r3;
                        notfinished = 0;
                        break;
                    }
                } while (notfinished);
                if (OPCODE(CurOpcode) == GCD) {
                    rplDropData(2);
                    rplNewBINTPush(gcd,DECBINT);
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

            BINT igcd = 0;
            if(RReg[1].flags&F_NEGATIVE) RReg[1].flags^=F_NEGATIVE;
            if(RReg[2].flags&F_NEGATIVE) RReg[2].flags^=F_NEGATIVE;
            if(gtReal(&RReg[2],&RReg[1])){
                swapReal(&RReg[2],&RReg[1]);
            }
            if (iszeroReal(&RReg[2])) {
                rplError(ERR_MATHDIVIDEBYZERO);
            }
            // avoid swapping elements by loop unrolling
            BINT notfinished = 1;
            do {
                if(!iszeroReal(&RReg[2])){
                    divmodReal(&RReg[0],&RReg[3],&RReg[1],&RReg[2]);
                }
                else {
                    igcd = 1;
                    notfinished = 0;
                    break;
                }
                if(!iszeroReal(&RReg[3])){
                    divmodReal(&RReg[0],&RReg[1],&RReg[2],&RReg[3]);
                }
                else {
                    igcd = 2;
                    notfinished = 0;
                    break;
                }
                if(!iszeroReal(&RReg[1])){
                    divmodReal(&RReg[0],&RReg[2],&RReg[3],&RReg[1]);
                }
                else {
                    igcd = 3;
                    notfinished = 0;
                    break;
                }
            } while (notfinished);
            if (OPCODE(CurOpcode) == GCD) {
                rplDropData(2);
                rplNewRealFromRRegPush(igcd);
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
                rplDropData(2);
                rplNewRealFromRRegPush(4);
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


#endif



