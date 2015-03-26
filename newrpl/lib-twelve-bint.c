/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// LIBRARY ONE DEFINES THE BASIC TYPES BINT AND SINT

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  12
#define LIB_ENUM lib12_enum
#define LIB_NAMES lib12_names
#define LIB_HANDLER lib12_handler
#define LIB_NUMBEROFCMDS LIB12_NUMBEROFCMDS

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ DECBINT,BINBINT,OCTBINT,HEXBINT,
                                              DECBINT|APPROX_BIT,BINBINT|APPROX_BIT,
                                              OCTBINT|APPROX_BIT,HEXBINT|APPROX_BIT,0 };

//#define _ISBINT(w) (((LIBNUM(w))&~3)==12)

// MACRO TO GET NUMBER OF BITS IN THE BASE
// 1= BINARY, 3=OCTAL, 4=HEX, AND 2=DECIMAL

#define GETBASE(libnum) (((libnum)-(BINBINT-2))>>1)

#define LIBFROMBASE(base) ((base<<1)+(BINBINT-2))


#define MIN_SINT    -131072
#define MAX_SINT    +131071
#define MAX_BINT    +9223372036854775807LL
#define MIN_BINT    (-9223372036854775807LL-1LL)

const UBINT64 const powersof10[20]={
    1000000000000000000LL,
    100000000000000000LL,
    10000000000000000LL,
    1000000000000000LL,
    100000000000000LL,
    10000000000000LL,
    1000000000000LL,
    100000000000LL,
    10000000000LL,
    1000000000LL,
    100000000LL,
    10000000LL,
    1000000LL,
    100000LL,
    10000LL,
    1000LL,
    100LL,
    10LL,
    1LL,
    0LL
};


// INTERNAL SINT OBJECTS
const WORD const zero_bint[]=
{
    (WORD)MAKESINT(0)
};

const WORD const one_bint[]=
{
    (WORD)MAKESINT(1)
};

const WORD const minusone_bint[]=
{
    (WORD)MAKESINT(-1)
};


const WORD const two_bint[]=
{
    (WORD)MAKESINT(2)
};

const WORD const three_bint[]=
{
    (WORD)MAKESINT(3)
};

const char const alldigits[]="0123456789ABCDEF";

WORDPTR rplNewSINT(int num,int base)
{
    WORDPTR obj;
    obj=rplAllocTempOb(0);
    if(!obj) return NULL;
    *obj=MKOPCODE(base,num&0x3ffff);
    return obj;
}


void rplNewSINTPush(int num,int base)
{
    WORDPTR obj;
    obj=rplAllocTempOb(0);
    if(!obj) return;
    *obj=MKOPCODE(base,num&0x3ffff);
    rplPushData(obj);

}

WORDPTR rplNewBINT(BINT64 num,int base)
{
    WORDPTR obj;

    if((num>=MIN_SINT)&&(num<=MAX_SINT)) {
        obj=rplAllocTempOb(0);
        if(!obj) return NULL;
        *obj=MKOPCODE(base,num&0x3ffff);
    }
    else {
        obj=rplAllocTempOb(2);
        if(!obj) return NULL;

        obj[0]=(MKPROLOG(base,2));
        obj[1]=((WORD)(num&0xffffffff));      // CAREFUL: THIS IS FOR LITTLE ENDIAN SYSTEMS ONLY!
        obj[2]=((WORD)( (num>>32)&0xffffffff));
    }

    return obj;
}


// WRITE AN INTEGER TO THE GIVEN DESTINATION. RETURN A POINTER AFTER THE LAST WRITTEN WORD
WORDPTR rplWriteBINT(BINT64 num,int base,WORDPTR dest)
{

    if((num>=MIN_SINT)&&(num<=MAX_SINT)) {
        *dest=MKOPCODE(base,num&0x3ffff);
        return ++dest;
    }
    else {
        dest[0]=(MKPROLOG(base,2));
        dest[1]=((WORD)(num&0xffffffff));      // CAREFUL: THIS IS FOR LITTLE ENDIAN SYSTEMS ONLY!
        dest[2]=((WORD)( (num>>32)&0xffffffff));
        return dest+3;
    }

}



void rplNewBINTPush(BINT64 num,int base)
{
    WORDPTR obj;

    if((num>=MIN_SINT)&&(num<=MAX_SINT)) {
        obj=rplAllocTempOb(0);
        if(!obj) return;
        *obj=MKOPCODE(base,num&0x3ffff);
    }
    else {
        obj=rplAllocTempOb(2);
        if(!obj) return;

        obj[0]=(MKPROLOG(base,2));
        obj[1]=((WORD)(num&0xffffffff));      // CAREFUL: THIS IS FOR LITTLE ENDIAN SYSTEMS ONLY!
        obj[2]=((WORD)( (num>>32)&0xffffffff));
    }

    rplPushData(obj);
}


BINT64 rplReadBINT(WORDPTR ptr)
{
    BINT64 result;
    if(ISPROLOG(*ptr))
            // THERE'S A PAYLOAD, READ THE NUMBER
            result= *((BINT64 *)(ptr+1));
        else {
        result=OPCODE(*ptr);
        if(result&0x20000) result|=0xFFFFFFFFFFFc0000;  // SIGN EXTEND
        }
    return result;
}

// A FEW FUNCTIONS THAT DEAL WITH TRUE/FALSE

void rplPushFalse()
{
    rplPushData((WORDPTR)zero_bint);
}

void rplPushTrue()
{
    rplPushData((WORDPTR)one_bint);

}

BINT rplIsFalse(WORDPTR objptr)
{
    if(IS_FALSE(*objptr)) return 1;
    if(ISREAL(*objptr)) {
        mpd_t dec;
        rplReadReal(objptr,&dec);
        if(mpd_iszero(&dec)) return 1;
    }

    return 0;
}

BINT rplIsTrue(WORDPTR objptr)
{
    if(IS_FALSE(*objptr)) return 0;
    if(ISREAL(*objptr)) {
        mpd_t dec;
        rplReadReal(objptr,&dec);
        if(mpd_iszero(&dec)) return 0;
    }

    return 1;
}








// READS A SINT, BINT OR REAL INTO A REAL NUMBER REGISTER
void rplNumberToRReg(int num,WORDPTR number)
{
    if(ISREAL(*number)) rplCopyRealToRReg(num,number);
    else if(ISBINT(*number)) rplBINTToRReg(num,rplReadBINT(number));
    else {
        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
    }
}


// READ A SINT, BINT OR REAL AS A 64-BIT INTEGER
// ROUNDING A REAL IS BY TRUNCATION
// DOES NOT CHECK FOR OVERFLOW!
// USES RREG[0]
BINT64 rplReadNumberAsBINT(WORDPTR number)
{
    BINT64 value;
    if(ISREAL(*number)) {
        mpd_t dec;
        int status;
        rplReadReal(number,&dec);
        // CONVERT TO INTEGER BY TRUNCATION
        mpd_qtrunc(&RReg[0],&dec,&Context,(uint32_t *)&status);
        status=0;
        value=mpd_qget_i64(&RReg[0],(uint32_t *)&status);
        if(status) {
            Exceptions|=status<<16;
            ExceptionPointer=IPtr;
            return 0;
        }
        return value;
    }
    else if(ISBINT(*number)) return rplReadBINT(number);
    else {
        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
    }
    return 0;
}

// READ A NUMBER AS A REAL
// CAREFUL!
// dec SHOULD BE UNINITIALIZED (WITH NO STORAGE ALLOCATED)
// DO **NOT** USE WITH RREG REGISTERS OR DATA CORRUPTION MIGHT OCCUR!!
// TEMPORARY DATA STORAGE FOR UP TO 4 NUMBERS
// IF CALLED MORE THAN 4 TIMES IT MIGHT OVERWRITE THE PREVIOUS

void rplReadNumberAsReal(WORDPTR number,mpd_t*dec)
{
    if(ISREAL(*number)) rplReadReal(number,dec);
    else if(ISBINT(*number))  {
        // PROVIDE STORAGE
        dec->alloc=BINT_REGISTER_STORAGE;
        dec->data=RDigits+BINT2RealIdx*BINT_REGISTER_STORAGE;
        dec->flags=MPD_STATIC|MPD_STATIC_DATA;
        mpd_set_i64(dec,rplReadBINT(number),&Context);
        ++BINT2RealIdx;
        if(BINT2RealIdx>=BINT2REAL) BINT2RealIdx=0;
    }
    else {
        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
    }

}


// COUNT THE NUMBER OF BITS IN A POSITIVE INTEGER
static int rpl_log2(BINT64 number,int bits)
{
    static const unsigned char const log2_table[16]={0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4};
    if(bits<=4) return log2_table[number];
    bits>>=1;
    if(number>>bits) return rpl_log2(number>>bits,bits)+bits;
    return rpl_log2(number,bits);
}





void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // NORMAL BEHAVIOR FOR A BINT IS TO PUSH THE OBJECT ON THE STACK:
        rplPushData(IPtr);
        return;
    }

    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE)
    {
        // THESE ARE OVERLOADABLE COMMANDS DISPATCHED FROM THE
        // OVERLOADABLE OPERATORS LIBRARY.

        // PROVIDE BEHAVIOR FOR OVERLOADABLE OPERATORS HERE
        BINT64 op1=0,op2=0;
        mpd_t rop1,rop2;
        int op1type=0,op2type=0;
        int status;

        // USE GC-SAFE POINTERS, NEVER LOCAL COPIES OF POINTERS INTO TEMPOB
#define arg1 ScratchPointer1
#define arg2 ScratchPointer2

        int nargs=OVR_GETNARGS(CurOpcode);

        if(rplDepthData()<nargs) {
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(nargs==1) {
            // UNARY OPERATORS
            arg1=rplPeekData(1);
            if(!ISBINT(*arg1)) {
                Exceptions=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }
            op1=rplReadBINT(arg1);
            rplDropData(1);

        }
        else {
            arg1=rplPeekData(2);
            arg2=rplPeekData(1);

            if(!(ISBINT(*arg1)||ISREAL(*arg1)) || !(ISBINT(*arg2)||ISREAL(*arg2))) {
                Exceptions=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }

            if(ISREAL(*arg1)) { rplReadReal(arg1,&rop1); op1type=1; }
            else { op1=rplReadBINT(arg1); op1type=0; }
            if(ISREAL(*arg2)) { rplReadReal(arg2,&rop2); op2type=1; }
            else { op2=rplReadBINT(arg2); op2type=0; }
            rplDropData(2);
        }

        switch(OPCODE(CurOpcode))
        {
        case OVR_ADD:
        {
            // ADD TWO NUMBERS FROM THE STACK
            if(op1type||op2type) {
                if(op1type) {
                    rplBINTToRReg(1,op2);
                    Context.status&=~MPD_Inexact;
                    mpd_add(&RReg[0],&rop1,&RReg[1],&Context);
                }

                if(op2type) {
                    // TODO: TRY TO RESPECT THE NUMBER TYPE OF THE FIRST ARGUMENT
                    rplBINTToRReg(1,op1);
                    Context.status&=~MPD_Inexact;
                    mpd_add(&RReg[0],&RReg[1],&rop2,&Context);
                }
                if(ISAPPROX(*arg1|*arg2) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(0);
                else rplNewRealFromRRegPush(0);
                return;
            }

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
                // CONVERT BOTH TO REALS
                rplBINTToRReg(1,op1);
                rplBINTToRReg(2,op2);
                Context.status&=~MPD_Inexact;
                mpd_add(&RReg[0],&RReg[1],&RReg[2],&Context);
                if(ISAPPROX(*arg1|*arg2) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(0);
                else rplNewRealFromRRegPush(0);
                return;
            }
            rplNewBINTPush(op1+op2,LIBNUM(*arg1)|(LIBNUM(*arg2)&APPROX_BIT));
            return;
        }
        case OVR_SUB:
        {
            if(op1type||op2type) {
                if(op1type) {
                    rplBINTToRReg(1,-op2);
                    Context.status&=~MPD_Inexact;
                    mpd_add(&RReg[0],&rop1,&RReg[1],&Context);
                }

                if(op2type) {
                    // TODO: TRY TO RESPECT THE NUMBER TYPE OF THE FIRST ARGUMENT
                    rplBINTToRReg(1,op1);
                    Context.status&=~MPD_Inexact;
                    mpd_sub(&RReg[0],&RReg[1],&rop2,&Context);
                }
                if(ISAPPROX(*arg1|*arg2) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(0);
                else rplNewRealFromRRegPush(0);
                return;
            }

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
                // CONVERT BOTH TO REALS
                rplBINTToRReg(1,op1);
                rplBINTToRReg(2,op2);
                Context.status&=~MPD_Inexact;
                mpd_sub(&RReg[0],&RReg[1],&RReg[2],&Context);
                if(ISAPPROX(*arg1|*arg2) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(0);
                else rplNewRealFromRRegPush(0);
                return;
            }
            rplNewBINTPush(op1-op2,LIBNUM(*arg1)|(LIBNUM(*arg2)&APPROX_BIT));
            return;
        }
        case OVR_MUL:
            if(op1type||op2type) {
                if(op1type) {
                    rplBINTToRReg(1,op2);
                    Context.status&=~MPD_Inexact;
                    mpd_mul(&RReg[0],&rop1,&RReg[1],&Context);
                }

                if(op2type) {
                    // TODO: TRY TO RESPECT THE NUMBER TYPE OF THE FIRST ARGUMENT
                    rplBINTToRReg(1,op1);
                    Context.status&=~MPD_Inexact;
                    mpd_mul(&RReg[0],&RReg[1],&rop2,&Context);
                }
                if(ISAPPROX(*arg1|*arg2) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(0);
                else rplNewRealFromRRegPush(0);
                return;
            }
            // DETECT OVERFLOW, AND CONVERT TO REALS IF SO

            // O1*O2 > 2^63 --> LOG2(O1)+LOG2(O2) > LOG2(2^63)
            // LOG2(O1)+LOG2(O2) > 63 MEANS OVERFLOW
            BINT sign1=(op1<0)^(op2<0);

            if(op1<0) op1=-op1;
            if(op2<0) op2=-op2;
            if(op2>op1) { BINT64 tmp=op2; op2=op1; op1=tmp; }

            if(!(op2>>32)) {
                if(rpl_log2(op1,64)+rpl_log2(op2,32)<63) {
                    op1*=op2;
                    if(sign1) rplNewBINTPush(-op1,LIBNUM(*arg1)|(LIBNUM(*arg2)&APPROX_BIT));
                    else rplNewBINTPush(op1,LIBNUM(*arg1)|(LIBNUM(*arg2)&APPROX_BIT));
                    return;
                }
            }

            rplBINTToRReg(1,op1);
            rplBINTToRReg(2,op2);
            Context.status&=~MPD_Inexact;
            mpd_mul(&RReg[0],&RReg[1],&RReg[2],&Context);
            if(Exceptions) return;
            if(ISAPPROX(*arg1|*arg2) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(0);
            else rplNewRealFromRRegPush(0);
            return;

        case OVR_DIV:
            {
            if(op1type||op2type) {
                if(op1type) {
                    rplBINTToRReg(1,op2);
                    Context.status&=~MPD_Inexact;
                    mpd_div(&RReg[0],&rop1,&RReg[1],&Context);
                }

                if(op2type) {
                    // TODO: TRY TO RESPECT THE NUMBER TYPE OF THE FIRST ARGUMENT
                    rplBINTToRReg(1,op1);
                    Context.status&=~MPD_Inexact;
                    mpd_div(&RReg[0],&RReg[1],&rop2,&Context);
                }
                if(ISAPPROX(*arg1|*arg2) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(0);
                else rplNewRealFromRRegPush(0);
                return;
            }
            rplBINTToRReg(1,op1);
            rplBINTToRReg(2,op2);
            Context.status&=~MPD_Inexact;
            mpd_div(&RReg[0],&RReg[1],&RReg[2],&Context);
            if(Exceptions) return;
            uint32_t status=0;
            BINT64 result=mpd_qget_i64(&RReg[0],&status);
            if(status) {
                if(ISAPPROX(*arg1|*arg2) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(0);
                else rplNewRealFromRRegPush(0);
            }
            else rplNewBINTPush(result,LIBNUM(*arg1)|(LIBNUM(*arg2)&APPROX_BIT));
            return;
            }

        case OVR_POW:
            {
            if(op1type||op2type) {
                Context.status&=~MPD_Inexact;
                if(op1type) {
                    rplBINTToRReg(1,op2);
                    mpd_pow(&RReg[0],&rop1,&RReg[1],&Context);
                }

                if(op2type) {
                    rplBINTToRReg(1,op1);
                    mpd_pow(&RReg[0],&RReg[1],&rop2,&Context);
                }
                if(ISAPPROX(*arg1|*arg2) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(0);
                else rplNewRealFromRRegPush(0);
                return;
            }


            // INTEGER POWER, USE REALS TO DEAL WITH NEGATIVE POWERS AND OVERFLOW
            rplBINTToRReg(1,op1);
            rplBINTToRReg(2,op2);
            Context.status&=~MPD_Inexact;
            mpd_pow(&RReg[0],&RReg[1],&RReg[2],&Context);
            uint32_t status=0;
            BINT64 result=mpd_qget_i64(&RReg[0],&status);
            if(status) {
                if(ISAPPROX(*arg1|*arg2) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(0);
                else rplNewRealFromRRegPush(0);
            }
            else rplNewBINTPush(result,LIBNUM(*arg1)|(LIBNUM(*arg2)&APPROX_BIT));
            return;
            }

        case OVR_EQ:
        {
        if(op1type||op2type) {
            if(op1type) {
                // ROUND TO INTEGER
                status=0;
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&rop1,&RReg[0],&Context);
                if(res) rplPushData((WORDPTR)zero_bint);
                else rplPushData((WORDPTR)one_bint);
            }

            if(op2type) {
                // ROUND TO INTEGER
                status=0;
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&rop2,&Context);
                if(res) rplPushData((WORDPTR)zero_bint);
                else rplPushData((WORDPTR)one_bint);
            }
            return;
        }
            // BOTH WERE INTEGERS
            if(op1==op2) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        }

        case OVR_NOTEQ:
        {
        if(op1type||op2type) {
            if(op1type) {
                // ROUND TO INTEGER
                status=0;
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&rop1,&RReg[0],&Context);
                if(res) rplPushData((WORDPTR)one_bint);
                else rplPushData((WORDPTR)zero_bint);

            }

            if(op2type) {
                // ROUND TO INTEGER
                status=0;
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&rop2,&Context);
                if(res) rplPushData((WORDPTR)one_bint);
                else rplPushData((WORDPTR)zero_bint);
            }
            return;
        }
            if(op1==op2) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        }
        case OVR_LT:
        {
        if(op1type||op2type) {
            if(op1type) {
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&rop1,&RReg[0],&Context);
                if(res<0) rplPushData((WORDPTR)one_bint);
                else rplPushData((WORDPTR)zero_bint);
            }
            if(op2type) {
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&rop2,&Context);
                if(res<0) rplPushData((WORDPTR)one_bint);
                else rplPushData((WORDPTR)zero_bint);
                }
            return;
            }

            if(op1<op2) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        }
        case OVR_GT:
        {
        if(op1type||op2type) {
            if(op1type) {
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&rop1,&RReg[0],&Context);
                if(res>0) rplPushData((WORDPTR)one_bint);
                else rplPushData((WORDPTR)zero_bint);
            }
            if(op2type) {
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&rop2,&Context);
                if(res>0) rplPushData((WORDPTR)one_bint);
                else rplPushData((WORDPTR)zero_bint);
                }
            return;
            }

            if(op1>op2) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        }



        case OVR_LTE:
        {
        if(op1type||op2type) {
            if(op1type) {
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&rop1,&RReg[0],&Context);
                if(res<=0) rplPushData((WORDPTR)one_bint);
                else rplPushData((WORDPTR)zero_bint);
            }
            if(op2type) {
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&rop2,&Context);
                if(res<=0) rplPushData((WORDPTR)one_bint);
                else rplPushData((WORDPTR)zero_bint);
                }
            return;
            }

            if(op1<=op2) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        }
        case OVR_GTE:
        {
        if(op1type||op2type) {
            if(op1type) {
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&rop1,&RReg[0],&Context);
                if(res>=0) rplPushData((WORDPTR)one_bint);
                else rplPushData((WORDPTR)zero_bint);
            }
            if(op2type) {
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&rop2,&Context);
                if(res>=0) rplPushData((WORDPTR)one_bint);
                else rplPushData((WORDPTR)zero_bint);
                }
            return;
            }

            if(op1>=op2) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        }
        case OVR_SAME:
        {
        if(op1type||op2type) {
            if(op1type) {
                // ROUND TO INTEGER
                status=0;
                mpd_qround_to_intx(&RReg[1],&rop1,&Context,(uint32_t *)&status);
                // IF MPD_Rounded OR MPD_Inexact, IT CAN'T BE EQUAL TO A BINT
                if(status) rplPushData((WORDPTR)zero_bint);
                else {
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&RReg[1],&RReg[0],&Context);
                if(res) rplPushData((WORDPTR)zero_bint);
                else rplPushData((WORDPTR)one_bint);
                }
            }

            if(op2type) {
                // ROUND TO INTEGER
                status=0;
                mpd_qround_to_intx(&RReg[1],&rop2,&Context,(uint32_t *)&status);
                // IF MPD_Rounded OR MPD_Inexact, IT CAN'T BE EQUAL TO A BINT
                if(status) rplPushData((WORDPTR)zero_bint);
                else {
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&RReg[1],&Context);
                if(res) rplPushData((WORDPTR)zero_bint);
                else rplPushData((WORDPTR)one_bint);
                }
            }
            return;
        }
            // BOTH WERE INTEGERS
            if(op1==op2) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        }




        case OVR_AND:
            if(op1type) op1=!mpd_iszero(&rop1);
            if(op2type) op2=!mpd_iszero(&rop2);
            if(op1&&op2) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        case OVR_OR:
            if(op1type) op1=!mpd_iszero(&rop1);
            if(op2type) op2=!mpd_iszero(&rop2);
            if(op1||op2) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;

        case OVR_CMP:
        {
        if(op1type||op2type) {
            if(op1type) {
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&rop1,&RReg[0],&Context);
                if(res<0) rplPushData((WORDPTR)minusone_bint);
                else if(res>0) rplPushData((WORDPTR)one_bint);
                        else rplPushData((WORDPTR)zero_bint);
            }
            if(op2type) {
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&rop2,&Context);
                if(res<0) rplPushData((WORDPTR)minusone_bint);
                else if(res>0) rplPushData((WORDPTR)one_bint);
                        else rplPushData((WORDPTR)zero_bint);
                }
            return;
            }

            if(op1>op2) rplPushData((WORDPTR)one_bint);
            else if(op1<op2) rplPushData((WORDPTR)minusone_bint);
                else rplPushData((WORDPTR)zero_bint);
            return;
        }






        case OVR_INV:
            // INVERSE WILL ALWAYS BE A REAL, SINCE 1/N == 0 FOR ALL N>1 IN STRICT INTEGER MATH, ORIGINAL UserRPL DOES NOT SUPPORT INVERSE OF INTEGERS
            rplOneToRReg(0);
            rplBINTToRReg(1,op1);
            Context.status&=~MPD_Inexact;
            mpd_div(&RReg[2],&RReg[0],&RReg[1],&Context);
            if(ISAPPROX(*arg1|*arg2) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(2);
            else rplNewRealFromRRegPush(2);
            return;
        case OVR_NEG:
        case OVR_UMINUS:
            op1=-op1;
            rplNewBINTPush(op1,LIBNUM(*arg1));
            return;
        case OVR_EVAL:
        case OVR_EVAL1:
        case OVR_XEQ:
        case OVR_NUM:
            // NOTHING TO DO, JUST KEEP THE ARGUMENT IN THE STACK
            rplPushData(arg1);
            return;
        case OVR_ABS:
            if(op1<0) rplNewBINTPush(-op1,LIBNUM(*arg1));
            else rplPushData(arg1);
            return;
        case OVR_NOT:
            if(op1) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;









        // ADD MORE case's HERE
        default:
            Exceptions=EX_BADARGTYPE;   // RETURN BAD TYPE SINCE THIS LIBRARY DOES NOT OVERLOAD THE OPERATOR
            ExceptionPointer=IPtr;
            return;


        }
        return;
#undef arg1
#undef arg2
    }   // END OF OVERLOADABLE OPERATORS


    BINT64 result;
    UBINT64 uresult;
    BYTEPTR strptr;
    int base,libbase,digit,count,neg,argnum1;
    char basechr;

    if(OPCODE(CurOpcode)>=MIN_RESERVED_OPCODE) {
    switch(OPCODE(CurOpcode))
    {
    // STANDARIZED OPCODES:
    // --------------------
    // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES


    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // ArgPtr2 = token blanks afterwards
        // ArgNum2 = blanks length

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors

        if(LIBNUM(CurOpcode)&APPROX_BIT) {
            // DO NOT COMPILE ANYTHING WHEN CALLED WITH THE UPPER (APPROX) LIBRARY NUMBER
            RetNum=ERR_NOTMINE;
            return;
        }

        // COMPILE A NUMBER TO A SINT OR A BINT, DEPENDING ON THE ACTUAL NUMERIC VALUE
        result=0;
        strptr=(BYTEPTR )TokenStart;
        base=10;
        libbase=DECBINT;
        neg=0;
        argnum1=TokenLen;    // LOCAL COPY

        if(*strptr=='-') { neg=1; ++strptr; --argnum1; }
        else if(*strptr=='+') { neg=0; ++strptr; --argnum1; }

        if(*strptr=='#') {
            ++strptr;
            --argnum1;
            // THIS IS A NUMBER WITH A BASE
            basechr=strptr[argnum1-1];

            if( (basechr=='d') || (basechr=='D')) { --argnum1; }
            if( (basechr=='h') || (basechr=='H')) { base=16; libbase=HEXBINT; --argnum1; }
            if( (basechr=='o') || (basechr=='O')) { base=8; libbase=OCTBINT; --argnum1; }
            if( (basechr=='b') || (basechr=='B')) { base=2; libbase=BINBINT; --argnum1; }
        }

        if(strptr[argnum1-1]=='.') {
         // NUMBERS ENDING IN A DOT ARE APPROXIMATED
            libbase|=APPROX_BIT;
            --argnum1;
        }


            for(count=0;count<argnum1;++count) {
                digit=strptr[count];
                if((digit>='0')&&(digit<='9')) digit-=48;
                else if((digit>='a')&&(digit<='f')) digit-=87;
                else if((digit>='A')&&(digit<='F')) digit-=55;
                else digit+=100;

                if((digit>=0) && (digit<base))
                {
                    if( ((result>>32)*base)>>31 ) {
                        // OVERFLOW, CANNOT BE AN INTEGER
                        RetNum=ERR_NOTMINE;
                        return;
                    }
                    result=result*base+digit;
                }
                else {
                 // AN INVALID DIGIT
                    RetNum=ERR_NOTMINE;
                    return;
                }
            }

            // NOTHING ABOVE DEALS WITH MEMORY, SO NO PROBLEMS WITH GC
            // FROM NOW ON, ANY POINTERS TO THE STRING BECOME INVALID
            // DUE TO POSSIBLE GC

            if(!count) {
             // NO DIGITS?
                RetNum=ERR_NOTMINE;
                return;
            }


            // FINISHED CONVERSION, NOW COMPILE TO SINT OR BINT AS NEEDED
            if(neg) result=-result;

            if((result>=MIN_SINT)&&(result<=MAX_SINT)) {
                rplCompileAppend(MKOPCODE(libbase,result&0x3ffff));
                RetNum=OK_CONTINUE;
                return;
            }
            rplCompileAppend(MKPROLOG(libbase,2));
            rplCompileAppend((WORD)(result&0xffffffff));      // CAREFUL: THIS IS FOR LITTLE ENDIAN SYSTEMS ONLY!
            rplCompileAppend((WORD)( (result>>32)&0xffffffff));
            RetNum=OK_CONTINUE;
     return;

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Byte Ptr to end of current string. Write here with rplDecompAppendString(); rplDecompAppendChar();

        if(ISPROLOG(*DecompileObject)) {
                // THERE'S A PAYLOAD, READ THE NUMBER
                result= *((BINT64 *)(DecompileObject+1));
            }
            else {
            result=OPCODE(*DecompileObject);
            if(result&0x20000) result|=0xFFFFFFFFFFFc0000;  // SIGN EXTEND
            }

            base=GETBASE(LIBNUM(*DecompileObject)&~APPROX_BIT);

            if(result<0) {
                rplDecompAppendChar('-');
                uresult=-result;
            } else uresult=result;



            if(base==2) {
                // THIS IS A BASE-10 NUMBER
                digit=0;
                basechr='0';
                while(uresult<powersof10[digit]) ++digit;  // SKIP ALL LEADING ZEROS
                // NOW DECOMPILE THE NUMBER
                while(digit<18) {
                while(uresult>=powersof10[digit]) { ++basechr; uresult-=powersof10[digit]; }
                rplDecompAppendChar(basechr);
                ++digit;
                basechr='0';
                }
                basechr+=uresult;
                rplDecompAppendChar(basechr);
            }
            else {
            // THIS IS A BINARY, OCTAL OR HEXA NUMBER
            // base HAS THE NUMBER OF BITS PER DIGIT
            rplDecompAppendChar('#');

            if(base>=3) digit=60;
            else digit=62;

            neg=(1<<base)-1;    // CREATE A MASK TO ISOLATE THE DIGIT

            // SKIP ALL LEADING ZEROS
            while(digit>0) {
                if( (uresult>>digit)&neg ) break;
                digit-=base;
            }
            // NOW DECOMPILE THE NUMBER
            while(digit>=0) {
                rplDecompAppendChar(alldigits[(uresult>>digit)&neg]);
                digit-=base;
            }

            // ADD BASE CHARACTER
            if(base==1) rplDecompAppendChar('b');
            if(base==3) rplDecompAppendChar('o');
            if(base==4) rplDecompAppendChar('h');

            }

            // ADD TRAILING DOT ON APPROXIMATED NUMBERS
            if(LIBNUM(*DecompileObject)&APPROX_BIT) rplDecompAppendChar('.');

            RetNum=OK_CONTINUE;

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

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
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // ArgPtr2 = token blanks afterwards
        // ArgNum2 = blanks length

        // COMPILE RETURNS:
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...), or ERR_NOTMINE IF NO TOKEN IS FOUND

    {

        // COMPILE A NUMBER TO A SINT OR A BINT, DEPENDING ON THE ACTUAL NUMERIC VALUE
         result=0;
        strptr=(BYTEPTR )TokenStart;
        base=10;
        libbase=DECBINT;
        neg=0;
        argnum1=TokenLen;    // LOCAL COPY

        // SIGN IS HANDLED AS UNARY OPERATOR IN SYMBOLICS
        /*
        if(*strptr=='-') { neg=1; ++strptr; --argnum1; }
        else if(*strptr=='+') { neg=0; ++strptr; --argnum1; }
        */

        if(LIBNUM(CurOpcode)&APPROX_BIT) {
            // DO NOT COMPILE ANYTHING WHEN CALLED WITH THE UPPER (APPROX) LIBRARY NUMBER
            RetNum=ERR_NOTMINE;
            return;
        }

        if(*strptr=='#') {
            ++strptr;
            --argnum1;
            // THIS IS A NUMBER WITH A BASE, FIND THE BASE CHARACTER
            base=0;
            neg=1;  // REUSED VARIABLE TO INDICATE THAT THE NUMBER SPECIFIED THE BASE EXPLICITLY

            for(count=0;count<argnum1;++count) {
                digit=strptr[count];
                if((digit>='0')&&(digit<='9')) digit-=48;
                else if((digit>='a')&&(digit<='f')) digit-=87;
                else if((digit>='A')&&(digit<='F')) digit-=55;
                else digit+=100;

                if((digit<0) || (digit>=16)) {
                    // THIS IS AN INVALID NUMERIC CHARACTER, MARK THE END OF TOKEN
                    basechr=strptr[count];
                    argnum1=count;
                    if( (basechr=='d') || (basechr=='D')) { base=10; }
                    else if( (basechr=='h') || (basechr=='H')) { base=16; }
                    else if( (basechr=='o') || (basechr=='O')) { base=8; }
                    else if( (basechr=='b') || (basechr=='B')) { base=2; }
                    else {
                        basechr=strptr[count-1];
                        if( (basechr=='d') || (basechr=='D')) { base=10; }
                        else if( (basechr=='h') || (basechr=='H')) { base=16; }
                        else if( (basechr=='o') || (basechr=='O')) { base=8; }
                        else if( (basechr=='b') || (basechr=='B')) { base=2; }
                        else {
                            // SYNTAX ERROR OR NOT A VALID NUMBER
                            RetNum=ERR_NOTMINE;
                            return;
                        }
                        argnum1=count-1;
                    }
                    break;
                }

            }

            if(base==0) {
                // SYNTAX ERROR, NUMBER DOES NOT HAVE A PROPER BASE SPECIFICATION
                RetNum=ERR_NOTMINE;
                return;
            }
        }

        // NOW WITH A PROPER BASE SELECTED, VERIFY THAT ALL DIGITS ARE NUMERIC

            for(count=0;count<argnum1;++count) {
                digit=strptr[count];
                if((digit>='0')&&(digit<='9')) digit-=48;
                else if((digit>='a')&&(digit<='f')) digit-=87;
                else if((digit>='A')&&(digit<='F')) digit-=55;
                else digit+=100;

                if((digit>=0) && (digit<base))
                {
                    if( ((result>>32)*base)>>31 ) {
                        // OVERFLOW, CANNOT BE AN INTEGER
                        RetNum=ERR_NOTMINE;
                        return;
                    }
                    result=result*base+digit;
                }
                else {
                 // AN INVALID DIGIT
                    if(count==0) {
                    RetNum=ERR_NOTMINE;
                    return;
                    } else {
                        if(neg) {
                            // IF THE BASE WAS SPECIFIED EXPLICITLY, THERE CANNOT BE ILLEGAL DIGITS
                            RetNum=ERR_NOTMINE;
                            return;
                        }
                        if(digit==('.'+100)) ++count;
                        // REPORT AS MANY VALID DIGITS AS POSSIBLE
                        RetNum=OK_TOKENINFO | MKTOKENINFO((strptr+count)-(BYTEPTR)TokenStart,TITYPE_INTEGER,0,1);
                        return;
                    }
                }
            }
            // ALL DIGITS WERE CORRECT
            RetNum=OK_TOKENINFO | MKTOKENINFO((strptr+argnum1)-(BYTEPTR)TokenStart,TITYPE_INTEGER,0,1);
            return;
    }
    case OPCODE_GETINFO:
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_INTEGER,0,1);
        return;
    case OPCODE_LIBINSTALL:
        RetNum=(UBINT)libnumberlist;
        return;
    case OPCODE_LIBREMOVE:
        return;


    default:

        RetNum=ERR_NOTMINE;
        return;


    }
    }
    else {
        // ALL OTHER OPCODES ARE SINT NUMBERS, JUST PUSH THEM ON THE STACK
        rplPushData(IPtr);
        return;
    }
// DON'T ISSUE A BAD_OPCODE ERROR SINCE ALL OPCODES ARE VALID SINT NUMBERS
}



