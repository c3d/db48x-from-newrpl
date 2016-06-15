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
#define LIBRARY_NUMBER  10


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

//#define COMMAND_LIST
#define ERROR_LIST \
        ERR(REALEXPECTED,0), \
        ERR(REALSNOTSUPPORTED,1), \
        ERR(INFINITERESULT,2), \
        ERR(UNDEFINEDRESULT,3), \
        ERR(NUMBERTOOBIG,4), \
        ERR(MATHDIVIDEBYZERO,5), \
        ERR(MATHOVERFLOW,6), \
        ERR(MATHUNDERFLOW,7), \
        ERR(COMPLEXRESULT,8)



// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER,LIBRARY_NUMBER|APPROX_BIT


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

ROMOBJECT one_real[]=
{
    (WORD)MKPROLOG(DOREAL,2),MAKEREALFLAGS(0,1,0),1
};

INCLUDE_ROMOBJECT(LIB_MSGTABLE);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
     (WORDPTR)LIB_MSGTABLE,
     (WORDPTR)one_real,
    0
};



/*
const CONTEXT CompileContext = {
    .prec=REAL_PRECISION_MAX,
    .emax=REAL_EXPONENT_MAX,
    .emin=REAL_EXPONENT_MIN,
    .traps=0,
    .status=0,
    .newtrap=0,
    .round=MPD_ROUND_HALF_EVEN,
    .clamp=MPD_CLAMP_DEFAULT,
    .allcr=0

};
*/


// SET THE REGISTER TO THE NUMBER 0NE
void rplOneToRReg(int num)
{
    RReg[num].exp=0;
    RReg[num].flags=0;
    RReg[num].len=1;
    RReg[num].data[0]=1;
}

// SET THE REGISTER TO ZERO
void rplZeroToRReg(int num)
{
    RReg[num].exp=0;
    RReg[num].flags=0;
    RReg[num].len=1;
    RReg[num].data[0]=0;
}

void rplInfinityToRReg(int num)
{
    RReg[num].exp=0;
    RReg[num].flags=F_INFINITY;
    RReg[num].len=1;
    RReg[num].data[0]=0;
}

void rplNANToRReg(int num)
{
    RReg[num].exp=0;
    RReg[num].flags=F_NOTANUMBER;
    RReg[num].len=1;
    RReg[num].data[0]=0;
}


void rplBINTToRReg(int num,BINT64 value)
{
    newRealFromBINT64(&RReg[num],value);
}


// EXTRACT A CALCULATOR REAL INTO AN EXISTING REAL STRUCTURE
// DATA IS **NOT** COPIED
// DO **NOT** USE THIS FUNCTION WITH RREG REGISTERS
void rplReadReal(WORDPTR real,REAL *dec)
{
REAL_HEADER *head=(REAL_HEADER *)(real+1);
dec->flags=0;
dec->data=(BINT *) (real+2);
dec->len=head->len;
dec->exp=head->exp;
dec->flags|=head->flags;
}

// EXTRACT A CALCULATOR REAL INTO A RREG REGISTER
void rplCopyRealToRReg(int num,WORDPTR real)
{
    REAL_HEADER *head=(REAL_HEADER *)(real+1);
    RReg[num].flags=0;
    RReg[num].len=head->len;
    RReg[num].exp=head->exp;
    RReg[num].flags|=head->flags;
    memcpyw(RReg[num].data,real+2,RReg[num].len);
}


// CREATE A NEW CALCULATOR REAL AND PUSH IT ON THE STACK
// SET THE VALUE TO THE GIVEN RREG
void rplNewRealFromRRegPush(int num)
{
    WORDPTR newreal=rplNewRealFromRReg(num);
    if(newreal) rplPushData(newreal);
}

void rplNewRealPush(REAL *num)
{
    WORDPTR newreal=rplNewReal(num);
    if(newreal) rplPushData(newreal);
}

// STORE A REAL ON THE GIVEN POINTER
// DOES NOT ALLOCATE MEMORY, USED FOR COMPOSITES
WORDPTR rplNewRealInPlace(REAL *num,WORDPTR newreal)
{

    REAL_HEADER real;
    BINT correction;

    // REMOVE ALL TRAILING ZEROES
    correction=0;
    while(correction<num->len-1)
    {
        if(num->data[correction]!=0) break;
        ++correction;
    }


    // WRITE THE PROLOG
    *newreal=MKPROLOG((num->flags&F_APPROX)? APPROX_BIT | LIBRARY_NUMBER : LIBRARY_NUMBER,1+num->len-correction);
    // PACK THE INFORMATION
    real.flags=num->flags&0xf;
    real.len=num->len-correction;
    real.exp=num->exp+correction*8;
    // STORE THE PACKED EXPONENT WORD
    newreal[1]=real.word;

    BINT count;
    for(count=0;count<num->len-correction;++count) {
        newreal[count+2]=(num->data[count+correction]);      // STORE ALL THE MANTISSA WORDS
    }

    return newreal+count+2;
}


// ALLOCATE MEMORY AND STORE A REAL ON IT
WORDPTR rplNewReal(REAL *num)
{

    ScratchPointer1=(WORDPTR)num->data;

    WORDPTR newreal=rplAllocTempOb(num->len+1);
    if(!newreal) {
        return 0;
    }

    num->data=(BINT *)ScratchPointer1;

    rplNewRealInPlace(num,newreal);

    return newreal;
}


WORDPTR rplNewRealFromRReg(int num)
{
return rplNewReal(&RReg[num]);
}

// STORE A REAL IN dest, AND RETURN A POINTER RIGHT AFTER THE OBJECT
// DOES NOT ALLOCATE MEMORY FROM THE SYSTEM
// USED INTERNALLY FOR COMPOSITE OBJECTS

WORDPTR rplRRegToRealInPlace(int num,WORDPTR dest)
{
    return rplNewRealInPlace(&RReg[num],dest);
}


// STORE A REAL ON THE COMPILER OUTPUT STREAM
void rplCompileReal(REAL *num)
{

    REAL_HEADER real;
    BINT correction;

    // REMOVE ALL TRAILING ZEROES
    correction=0;
    while(correction<num->len-1)
    {
        if(num->data[correction]!=0) break;
        ++correction;
    }


    // WRITE THE PROLOG
    rplCompileAppend(MKPROLOG((num->flags&F_APPROX)? APPROX_BIT | LIBRARY_NUMBER : LIBRARY_NUMBER,1+num->len-correction));
    // PACK THE INFORMATION
    real.flags=num->flags&0xf;
    real.len=num->len-correction;
    real.exp=num->exp+correction*8;
    // STORE THE PACKED EXPONENT WORD
    rplCompileAppend(real.word);

    BINT count;
    for(count=0;count<num->len-correction;++count) {
        rplCompileAppend(num->data[count+correction]);      // STORE ALL THE MANTISSA WORDS
        if(Exceptions) return;
    }

}


// CHECKS THE RESULT AND ISSUE ERRORS/EXCEPTIONS AS NEEDED
void rplCheckResultAndError(REAL *real)
{
    if(real->flags&F_INFINITY) {
        if(!rplTestSystemFlag(FL_INIFINITEERROR)) rplError(ERR_INFINITERESULT);
        else rplSetSystemFlag(FL_INFINITE);
    }
    if(real->flags&F_OVERFLOW) {
        if(!rplTestSystemFlag(FL_OVERFLOWERROR)) rplError(ERR_MATHOVERFLOW);
        else rplSetSystemFlag(FL_OVERFLOW);
    }
    if(real->flags&F_NEGUNDERFLOW) {
        if(!rplTestSystemFlag(FL_UNDERFLOWERROR)) rplError(ERR_MATHUNDERFLOW);
        else rplSetSystemFlag(FL_NEGUNDERFLOW);
    }
    if(real->flags&F_POSUNDERFLOW) {
        if(!rplTestSystemFlag(FL_UNDERFLOWERROR)) rplError(ERR_MATHUNDERFLOW);
        else rplSetSystemFlag(FL_POSUNDERFLOW);
    }
    if(real->flags&F_NOTANUMBER) {
        rplError(ERR_UNDEFINEDRESULT);
    }

}

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // NORMAL BEHAVIOR FOR A REAL IS TO PUSH THE OBJECT ON THE STACK:
        rplPushData(IPtr);
        return;
    }

    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE)
    {
        // THESE ARE OVERLOADABLE COMMANDS DISPATCHED FROM THE
        // OVERLOADABLE OPERATORS LIBRARY.

        // PROVIDE BEHAVIOR FOR OVERLOADABLE OPERATORS HERE
#define arg1 ScratchPointer1
#define arg2 ScratchPointer2

        int nargs=OVR_GETNARGS(CurOpcode);
        REAL Darg1,Darg2;

        if(rplDepthData()<nargs) {
             rplError(ERR_BADARGCOUNT);
            return;
        }
        if(nargs==1) {
            // UNARY OPERATORS
            arg1=rplPeekData(1);
            if(!ISREAL(*arg1)) {
                rplError(ERR_REALEXPECTED);
                return;
            }
            rplReadReal(arg1,&Darg1);
            rplDropData(1);
        }
        else {
            arg1=rplPeekData(2);
            arg2=rplPeekData(1);

            if(!ISREAL(*arg1) || !ISREAL(*arg2)) {
                rplError(ERR_REALEXPECTED);
                return;
            }

            rplReadReal(arg1,&Darg1);
            rplReadReal(arg2,&Darg2);
            rplDropData(2);
        }

        switch(OPCODE(CurOpcode))
        {
        case OVR_ADD:
            // ADD TWO BINTS FROM THE STACK
            addReal(&RReg[0],&Darg1,&Darg2);
            rplNewRealFromRRegPush(0);
            if(!Exceptions) rplCheckResultAndError(&RReg[0]);

            return;

        case OVR_SUB:
            subReal(&RReg[0],&Darg1,&Darg2);
            rplNewRealFromRRegPush(0);
            if(!Exceptions) rplCheckResultAndError(&RReg[0]);

            return;

        case OVR_MUL:
            mulReal(&RReg[0],&Darg1,&Darg2);
            rplNewRealFromRRegPush(0);
            if(!Exceptions) rplCheckResultAndError(&RReg[0]);

            return;

        case OVR_DIV:
            divReal(&RReg[0],&Darg1,&Darg2);
            rplNewRealFromRRegPush(0);
            if(!Exceptions) rplCheckResultAndError(&RReg[0]);

            return;

        case OVR_POW:
        {
            RReg[1].data[0]=5;
            RReg[1].exp=-1;
            RReg[1].len=1;
            RReg[1].flags=0;

            if(eqReal(&Darg2,&RReg[1]))
                // THIS IS A SQUARE ROOT
            {
                if(Darg1.flags&F_NEGATIVE) {

                    if(rplTestSystemFlag(FL_COMPLEXMODE)) {
                    // COMPLEX RESULT!
                    Darg1.flags^=F_NEGATIVE;
                    hyp_sqrt(&Darg1);
                    finalize(&RReg[0]);

                    swapReal(&RReg[6],&RReg[0]);
                    newRealFromBINT(&RReg[7],90);

                    BINT angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

                    trig_convertangle(&RReg[7],ANGLEDEG,angmode);

                    rplCheckResultAndError(&RReg[0]);
                    rplCheckResultAndError(&RReg[6]);


                    rplNewComplexPush(&RReg[6],&RReg[0],angmode);

                    return;

                    }
                    else {
                        rplError(ERR_COMPLEXRESULT);
                        return;
                    }


                }


                hyp_sqrt(&Darg1);
                finalize(&RReg[0]);
            }
            else {
                if(Darg1.flags&F_NEGATIVE) {


                    // NEGATIVE NUMBER RAISED TO A REAL POWER
                    // a^n= ABS(a)^n * (cos(n*pi)+i*sin(n*pi))

                    // USE DEG TO AVOID LOSS OF PRECISION WITH PI

                    newRealFromBINT(&RReg[7],180);

                    mulReal(&RReg[0],&Darg2,&RReg[7]);
                    divmodReal(&RReg[1],&RReg[9],&RReg[0],&RReg[7]);    // REDUCE TO FIRST CIRCLE

                    if(!rplTestSystemFlag(FL_COMPLEXMODE) && !iszeroReal(&RReg[9])) {
                        rplError(ERR_COMPLEXRESULT);
                        return;
                    }
                    Darg1.flags^=F_NEGATIVE;    // MAKE IT POSITIVE

                    powReal(&RReg[8],&Darg1,&Darg2);    // ONLY RReg[9] IS PRESERVED

                    BINT angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

                    trig_convertangle(&RReg[9],ANGLEDEG,angmode);

                    rplCheckResultAndError(&RReg[0]);
                    rplCheckResultAndError(&RReg[8]);

                    // RETURN A POLAR COMPLEX
                    rplNewComplexPush(&RReg[8],&RReg[0],angmode);

                    return;


                }

                powReal(&RReg[0],&Darg1,&Darg2);

            }

            rplNewRealFromRRegPush(0);
            if(!Exceptions) rplCheckResultAndError(&RReg[0]);

            return;
        }

        case OVR_XROOT:
            RReg[1].data[0]=2;
            RReg[1].exp=0;
            RReg[1].len=1;
            RReg[1].flags=0;

            if(eqReal(&Darg2,&RReg[1]))
                // THIS IS A SQUARE ROOT
            {

                if(Darg1.flags&F_NEGATIVE) {

                    if(rplTestSystemFlag(FL_COMPLEXMODE)) {
                    // COMPLEX RESULT!
                    Darg1.flags^=F_NEGATIVE;
                    hyp_sqrt(&Darg1);
                    finalize(&RReg[0]);

                    swapReal(&RReg[6],&RReg[0]);
                    newRealFromBINT(&RReg[7],90);

                    BINT angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

                    trig_convertangle(&RReg[7],ANGLEDEG,angmode);

                    rplCheckResultAndError(&RReg[0]);
                    rplCheckResultAndError(&RReg[6]);

                    rplNewComplexPush(&RReg[6],&RReg[0],angmode);

                    return;

                    }
                    else {
                        rplError(ERR_COMPLEXRESULT);
                        return;
                    }

                }


                hyp_sqrt(&Darg1);
                finalize(&RReg[0]);
            }
            else {
                if(Darg1.flags&F_NEGATIVE) {


                    // NEGATIVE NUMBER RAISED TO A REAL POWER
                    // a^(1/n)= ABS(a)^(1/n) * (cos(pi/n)+i*sin(pi/n))

                    // USE DEG TO AVOID LOSS OF PRECISION WITH PI

                    newRealFromBINT(&RReg[7],180);

                    divReal(&RReg[0],&RReg[7],&Darg2);
                    divmodReal(&RReg[1],&RReg[9],&RReg[0],&RReg[7]);    // REDUCE TO FIRST CIRCLE

                    if(!rplTestSystemFlag(FL_COMPLEXMODE) && !iszeroReal(&RReg[9])) {
                        rplError(ERR_COMPLEXRESULT);
                        return;
                    }
                    Darg1.flags^=F_NEGATIVE;    // MAKE IT POSITIVE

                    xrootReal(&RReg[8],&Darg1,&Darg2);    // ONLY RReg[9] IS PRESERVED

                    BINT angmode=rplTestSystemFlag(FL_ANGLEMODE1)|(rplTestSystemFlag(FL_ANGLEMODE2)<<1);

                    trig_convertangle(&RReg[9],ANGLEDEG,angmode);

                    rplCheckResultAndError(&RReg[0]);
                    rplCheckResultAndError(&RReg[8]);

                    // RETURN A POLAR COMPLEX
                    rplNewComplexPush(&RReg[8],&RReg[0],angmode);

                    return;


                }



                xrootReal(&RReg[0],&Darg1,&Darg2);

            }

            rplNewRealFromRRegPush(0);
            if(!Exceptions) rplCheckResultAndError(&RReg[0]);

            return;


        case OVR_EQ:

            if(eqReal(&Darg1,&Darg2)) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;

        case OVR_NOTEQ:
            if(!eqReal(&Darg1,&Darg2)) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        case OVR_LT:
            if(ltReal(&Darg1,&Darg2)) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        case OVR_GT:
            if(gtReal(&Darg1,&Darg2)) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        case OVR_LTE:
            if(!gtReal(&Darg1,&Darg2)) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        case OVR_GTE:
            if(!ltReal(&Darg1,&Darg2)) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        case OVR_SAME:
            if(eqReal(&Darg1,&Darg2)) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        case OVR_AND:
            if(iszeroReal(&Darg1)||iszeroReal(&Darg2)) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        case OVR_OR:
            if(iszeroReal(&Darg1)&&iszeroReal(&Darg2)) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        case OVR_CMP:
            rplNewSINTPush(cmpReal(&Darg1,&Darg2),DECBINT);
            return;








        case OVR_INV:
            rplOneToRReg(1);
            divReal(&RReg[0],&RReg[1],&Darg1);
            rplNewRealFromRRegPush(0);
            if(!Exceptions) rplCheckResultAndError(&RReg[0]);

            return;
        case OVR_NEG:
        case OVR_UMINUS:
        {
            if((Darg1.flags&(F_INFINITY|F_NOTANUMBER)) || !(Darg1.len==1 && Darg1.data[0]==0)) Darg1.flags^=F_NEGATIVE;
            rplNewRealPush(&Darg1);
        }
            return;
        case OVR_EVAL:
        case OVR_EVAL1:
        case OVR_XEQ:
        case OVR_NUM:
            // NOTHING TO DO, JUST KEEP THE ARGUMENT IN THE STACK
            rplPushData(arg1);
            return;
        case OVR_ABS:
            Darg1.flags&=~F_NEGATIVE;
            rplNewRealPush(&Darg1);
            return;
        case OVR_NOT:
            if(iszeroReal(&Darg1)) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;


        // ADD MORE case's HERE
        default:
            rplError(ERR_REALSNOTSUPPORTED);
            return;


        }
        return;
#undef arg1
#undef arg2
    }   // END OF OVERLOADABLE OPERATORS


    switch(OPCODE(CurOpcode))
    {
    // STANDARIZED OPCODES:
    // --------------------
    // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES


    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlankLen = blanks length

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors



        // COMPILE A NUMBER TO A REAL
    {
        if(LIBNUM(CurOpcode)&APPROX_BIT) {
            // DO NOT COMPILE ANYTHING WHEN CALLED WITH THE UPPER (APPROX) LIBRARY NUMBER
            RetNum=ERR_NOTMINE;
            return;
        }

        BYTEPTR strptr=(BYTEPTR )TokenStart;
        BINT isapprox=0;
        BINT tlen=TokenLen;

        UBINT64  locale=rplGetSystemLocale();


        newRealFromText(&RReg[0],(char *)strptr,utf8nskip((char *)strptr,(char *)BlankStart,tlen),locale);


        if(RReg[0].flags&F_ERROR) {
            // THERE WAS SOME ERROR DURING THE CONVERSION, PROBABLY A SYNTAX ERROR
            // THE EXPONENT IN THE REAL GIVES MORE ERROR INFO
            // THE LEN GIVES POSITION OF ERROR

            if(RReg[0].len>0) {
                // THERE WAS AT LEAST SOME NUMBERS, TRY TO SPLIT THE TOKEN
                if(!rplIsValidIdent(((BYTEPTR)TokenStart)+RReg[0].len,(BYTEPTR)BlankStart)) {
                    RetNum=ERR_SYNTAX;
                    return;
                }
            }

            RetNum=ERR_NOTMINE;
            return;
        }

            if(RReg[0].flags&F_OVERFLOW) {
                rplError(ERR_MATHOVERFLOW);
                RetNum=ERR_INVALID;
                return;
            }
            if(RReg[0].flags&(F_NEGUNDERFLOW|F_POSUNDERFLOW)) {
                rplError(ERR_MATHUNDERFLOW);
                RetNum=ERR_INVALID;
                return;
            }


            if(RReg[0].flags&F_APPROX) isapprox=APPROX_BIT;
            // WRITE THE PROLOG
            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER|isapprox,1+RReg[0].len));
            // PACK THE INFORMATION
            REAL_HEADER real;
            real.flags=RReg[0].flags&0xf;
            real.len=RReg[0].len;
            real.exp=RReg[0].exp;

            rplCompileAppend(real.word);      // CAREFUL: THIS IS FOR LITTLE ENDIAN SYSTEMS ONLY!
            BINT count;
            for(count=0;count<RReg[0].len;++count) {
                rplCompileAppend(RReg[0].data[count]);      // STORE ALL THE MANTISSA WORDS
            }
            RetNum=OK_CONTINUE;
     return;
    }

    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Byte Ptr to end of current string. Write here with rplDecompAppendString(); rplDecompAppendChar();
    {
        REAL realnum;

        rplReadReal(DecompileObject,&realnum);


        // CONVERT TO STRING

        NUMFORMAT fmt;

        BINT Format,sign;

        rplGetSystemNumberFormat(&fmt);


        sign=realnum.flags&F_NEGATIVE;

        realnum.flags^=sign;

        if(iszeroReal(&realnum)) Format=fmt.MiddleFmt;
        else if(ltReal(&realnum,&(fmt.SmallLimit))) Format=fmt.SmallFmt;
        else if(gtReal(&realnum,&(fmt.BigLimit))) Format=fmt.BigFmt;
        else Format=fmt.MiddleFmt;

        realnum.flags^=sign;

        if(CurOpcode==OPCODE_DECOMPEDIT) Format|=FMT_CODE;

        // ESTIMATE THE MAXIMUM STRING LENGTH AND RESERVE THE MEMORY

        BYTEPTR string;

        BINT len=formatlengthReal(&realnum,Format,fmt.Locale);

        // RESERVE THE MEMORY FIRST
        rplDecompAppendString2(0,len);

        // NOW USE IT
        string=(BYTEPTR)DecompStringEnd;
        string-=len;

        if(Exceptions) {
            RetNum=ERR_INVALID;
            return;
        }
        DecompStringEnd=(WORDPTR) formatReal(&realnum,(char *)string,Format,fmt.Locale);

        RetNum=OK_CONTINUE;

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors
    }
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

        if(LIBNUM(CurOpcode)&APPROX_BIT) {
            // DO NOT COMPILE ANYTHING WHEN CALLED WITH THE UPPER (APPROX) LIBRARY NUMBER
            RetNum=ERR_NOTMINE;
            return;
        }

        enum {
            MODE_IP=0,
            MODE_FP,
            MODE_EXPLETTER,
            MODE_EXPSIGN,
            MODE_EXP
        };
        WORD Locale=rplGetSystemLocale();
        BINT mode=MODE_IP;
        BYTE num;
        int f,exitfor=0;
        BYTEPTR ptr=(BYTEPTR)TokenStart;

        for(f=0;f<(int)TokenLen;++f,++ptr) {
            num=*ptr;
            switch(mode)
            {
            case MODE_IP:
                if(num==DECIMAL_DOT(Locale)) { mode=MODE_FP; break; }
                //if(num==THOUSAND_SEP(Locale)) { break; }
                if((f!=0) && (num=='e' || num=='E' || num==EXP_LETTER(Locale))) { mode=MODE_EXPSIGN; break; }
                if(num<'0' || num>'9') { exitfor=1; break; }
                break;
            case MODE_FP:
                //if(num==FRAC_SEP(Locale)) { break; }
                if(num=='.') { mode=MODE_EXPLETTER; break; }
                if(num=='e' || num=='E' || num==EXP_LETTER(Locale)) { mode=MODE_EXPSIGN; break; }
                if(num<'0' || num>'9') { exitfor=1; break; }
                break;
            case MODE_EXPLETTER:
                if(num=='e' || num=='E' || num==EXP_LETTER(Locale)) { mode=MODE_EXPSIGN; break; }
                exitfor=1;
                break;
            case MODE_EXPSIGN:
                if(num=='+' || num=='-') { mode=MODE_EXP; break; }
                if(num<'0' || num>'9') { exitfor=1; break; }
                mode=MODE_EXP;
                break;
            case MODE_EXP:
                if(num<'0' || num>'9') { exitfor=1; break; }
                break;
            }
            if(exitfor) break;
        }

        if(mode==MODE_EXPSIGN) --f;
        if(f==0) RetNum=ERR_NOTMINE;

        else {
            if(num=='.') RetNum=OK_TOKENINFO | MKTOKENINFO(f+1,TITYPE_REAL,0,1);
            else RetNum=OK_TOKENINFO | MKTOKENINFO(f,TITYPE_REAL,0,1);
        }

        return;
    }

    case OPCODE_GETINFO:
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_REAL,0,1);
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

        if(ISPROLOG(*ObjectPTR)) {
            if(OBJSIZE(*ObjectPTR)<2) { RetNum=ERR_INVALID; return; }
            REAL r;
            rplReadReal(ObjectPTR,&r);
            // CHECK PROPER LENGTH
            if((WORD)(r.len+1)!=OBJSIZE(*ObjectPTR)) { RetNum=ERR_INVALID; return; }
            // CHECK FOR CORRUPTED DATA
            BINT k;
            for(k=0;k<r.len;++k) {
                // IF THE NUMBER IS NOT NORMALIZED, ASSUME IT WAS CORRUPTED
                if( (r.data[k]<0) || (r.data[k]>=100000000) ) { RetNum=ERR_INVALID; return; }
            }
        }
        RetNum=OK_CONTINUE;
        return;


    case OPCODE_AUTOCOMPNEXT:
        //libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        RetNum=ERR_NOTMINE;
        return;

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
