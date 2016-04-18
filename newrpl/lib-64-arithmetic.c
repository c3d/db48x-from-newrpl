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
    ECMD(PERCENTTOT,"%T",MKTOKENINFO(2,TITYPE_FUNCTION,2,2))


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


void binary_functions_list_handling(WORDPTR arg1, WORDPTR arg2)
{
    if(ISLIST(*arg1) && ISLIST(*arg2)) {

        WORDPTR *savestk=DSTop;
        WORDPTR newobj=rplAllocTempOb(2);
        if(!newobj) return;
        // CREATE A PROGRAM AND RUN THE DOLIST COMMAND
        newobj[0]=MKPROLOG(DOCOL,2);
        newobj[1]=CurOpcode;
        newobj[2]=CMD_SEMI;

        rplPushDataNoGrow((WORDPTR)two_bint);
        rplPushData(newobj);

        rplCallOperator(CMD_CMDDOLIST);

        if(Exceptions) {
            if(DSTop>savestk) DSTop=savestk;
        }

        // EXECUTION WILL CONTINUE AT DOLIST

        return;
    }
    else if(ISLIST(*arg1) && !ISLIST(*arg2)){

        BINT size1=rplObjSize(rplPeekData(1));
        WORDPTR *savestk=DSTop;

        WORDPTR newobj=rplAllocTempOb(2+size1);
        if(!newobj) return;

        // CREATE A PROGRAM AND RUN THE MAP COMMAND
        newobj[0]=MKPROLOG(DOCOL,2+size1);
        rplCopyObject(newobj+1,rplPeekData(1));
        newobj[size1+1]=CurOpcode;
        newobj[size1+2]=CMD_SEMI;

        rplDropData(1);
        rplPushData(newobj);

        rplCallOperator(CMD_MAP);

        if(Exceptions) {
            if(DSTop>savestk) DSTop=savestk;
        }

        // EXECUTION WILL CONTINUE AT MAP

        return;

    }
    else if(!ISLIST(*arg1) && ISLIST(*arg2)){
        BINT i;
        WORDPTR *savestk=DSTop;
        WORDPTR newobj=rplAllocTempOb(2);
        if(!newobj) return;

        BINT count = rplListLength(arg2);
        if (count > 0) {

            // Create a list same size as arg2, set every value to arg1
            for (i=0; i < count; ++i) {
                rplPushData(arg1);
            }
            rplNewBINTPush(count,DECBINT);
            rplCreateList();
            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
                return;
            }

            // replace arg1 scalar by new list: arg1 -> { arg1 arg1 arg1 .. }
            rplOverwriteData(3,rplPeekData(1));

            // procede like the 2-list version above
            // CREATE A PROGRAM AND RUN THE DOLIST COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplOverwriteData(1, (WORDPTR)two_bint);
            rplPushData(newobj);

            rplCallOperator(CMD_CMDDOLIST);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
                return;
            }
        }
        else {
            rplError(ERR_BADARGTYPE);
            return;
        }
    }
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

        if(ISREAL(*arg)) {
            REAL num;
            rplReadNumberAsReal(arg,&num);
            if(!isintegerReal(&num)) {
                rplError(ERR_INTEGEREXPECTED);
                return;
            }
        }

        rplStoreSettingsbyName((BYTEPTR)modulo_name,(BYTEPTR)(modulo_name+3),arg);
        rplDropData(1);
        return;
    }

    case POWMOD:
        {

            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR arg=rplPeekData(2);
            WORDPTR exp=rplPeekData(1);

            if(ISLIST(*arg) || ISLIST(*exp)){
                binary_functions_list_handling(arg,exp);
                return;
            }

            if(!ISNUMBER(*arg)) {
                rplError(ERR_BADARGTYPE);
                return;
            }

            WORDPTR mod=rplGetSettingsbyName((BYTEPTR)modulo_name,(BYTEPTR)modulo_name+3);
            if(!mod) mod=(WORDPTR)zero_bint;
            if( !ISNUMBER(*exp) || !ISNUMBER(*mod)) {
                rplError(ERR_BADARGTYPE);
                return;
            }

            if(ISBINT(*arg) && ISBINT(*exp) && ISBINT(*mod)) {

                BINT64 a=rplReadBINT(arg);
                BINT64 e=rplReadBINT(exp);
                BINT64 m=rplReadBINT(mod);

                if(m<2147483648LL) {  // MAXIMUM MOD WE CAN USE WITH BINTS

                a=powmodBINT(a,e,m);

                rplDropData(2);

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

                rplDropData(2);

                rplNewRealFromRRegPush(7);



            return;

    }

    case MOD:
    {

            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR arg=rplPeekData(2);
            WORDPTR mod=rplPeekData(1);

            if(ISLIST(*arg) || ISLIST(*mod)){
                binary_functions_list_handling(arg,mod);
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
                rplDropData(2);
                rplNewBINTPush(a%m,DECBINT);
                return;
            }
                // THERE'S REALS INVOLVED, DO IT ALL WITH REALS

                REAL a,m;
                rplReadNumberAsReal(arg,&a);
                rplReadNumberAsReal(mod,&m);

                divmodReal(&RReg[7],&RReg[6],&a,&m);

                rplDropData(2);

                rplNewRealFromRRegPush(6);

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
            binary_functions_list_handling(arg1,pct);
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
                binary_functions_list_handling(old_val,new_val);
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
                binary_functions_list_handling(old_val,new_val);
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



