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
#define LIBRARY_NUMBER  30


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(RE,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(IM,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(ARG,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(CONJ,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    ECMD(CPLX2REAL,"C→R",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(REAL2CPLX,"R→C",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE

#define ERROR_LIST \
ERR(COMPLEXEXPECTED,0), \
ERR(COMPLEXORREALEXPECTED,1), \
ERR(COMPLEXNOTSUPPORTED,2), \
ERR(NOTALLOWEDINCOMPLEX,3)


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

// USED TO DEFINE A REAL CONSTANT ZERO
const uint32_t const zero_data[1]={0};

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib30_menu);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib30_menu,
    0
};

// DECODE A COMPLEX NUMBER INTO A REAL
// DOES NOT ALLOCATE ANY MEMORY, MIGHT USE RREG[8] IF STORAGE IS NEEDED

void rplRealPart(WORDPTR complex,REAL *real)
{
    rplReadNumberAsReal(++complex,real);
}

void rplImaginaryPart(WORDPTR complex,REAL *imag)
{
    rplReadNumberAsReal(rplSkipOb(++complex),imag);
}


// GETS THE REAL PART OF ANY NUMBER: IF BINT OR REAL, GET THE NUMBER. IF COMPLEX, RETURN THE REAL PART.
void rplReadCNumberAsReal(WORDPTR complex,REAL *real)
{
    if(ISCOMPLEX(*complex)) ++complex;
    rplReadNumberAsReal(complex,real);
}

void rplReadCNumberAsImag(WORDPTR complex,REAL *imag)
{
    if(ISCOMPLEX(*complex)) rplImaginaryPart(complex,imag);
    else {
        // SET IMAG TO ZERO
        imag->data=(BINT *)zero_data;
        imag->exp=0;
        imag->flags=0;
        imag->len=1;
    }
}


// CREATE COMPLEX NUMBER FROM 2 RREG'S AND PUSH IT ON THE STACK

void rplNewComplexPush(REAL *real,REAL *imag)
{
    if(iszeroReal(imag)) {
        // IT'S A REAL NUMBER, THERE'S NO IMAGINARY PART
        rplNewRealPush(real);
        return;
    }
    BINT size=4+real->len+imag->len;

    ScratchPointer1=(WORDPTR)real->data;
    ScratchPointer2=(WORDPTR)imag->data;

    WORDPTR newobject=rplAllocTempOb(size);
    WORDPTR parts;
    if(!newobject) {
        return;
    }

    real->data=(BINT *)ScratchPointer1;
    imag->data=(BINT *)ScratchPointer2;

    parts=rplNewRealInPlace(real,newobject+1);
    parts=rplNewRealInPlace(imag,parts);
    newobject[0]=MKPROLOG(LIBRARY_NUMBER,parts-newobject-1);

    rplTruncateLastObject(parts);

    rplPushData(newobject);

}


// CREATE COMPLEX NUMBER FROM 2 RREG'S AND PUSH IT ON THE STACK

void rplRRegToComplexPush(BINT real,BINT imag)
{
    rplNewComplexPush( &RReg[real],&RReg[imag]);
}






// CREATE COMPLEX NUMBER FROM 2 RREG'S AT ADDRESS dest
// AND RETURN POINTER IMMEDIATELY AFTER THE NUMBER
// DOES NOT ALLOCATE MEMORY FROM THE SYSTEM
// USED INTERNALLY FOR COMPOSITES

WORDPTR rplRRegToComplexInPlace(BINT real,BINT imag,WORDPTR dest)
{
    if(iszeroReal(&RReg[imag])) {
        // IT'S A REAL NUMBER, THERE'S NO IMAGINARY PART
        return rplRRegToRealInPlace(real,dest);
    }
    WORDPTR parts;
    parts=rplRRegToRealInPlace(real,dest+1);
    parts=rplRRegToRealInPlace(imag,parts);
    dest[0]=MKPROLOG(LIBRARY_NUMBER,parts-dest-1);

    return parts;
}


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // NORMAL BEHAVIOR FOR A COMPLEX IS TO PUSH THE OBJECT ON THE STACK:
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
        REAL Rarg1,Iarg1,Rarg2,Iarg2;

        if(rplDepthData()<nargs) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(nargs==1) {
            // UNARY OPERATORS
            arg1=rplPeekData(1);
            if(!ISCOMPLEX(*arg1)) {
                if(!ISPROLOG(*arg1)) {
                    // ALLOW EXECUTION OF COMMANDS AS OBJECTS
                    if( (OPCODE(CurOpcode)==OVR_EVAL)||
                            (OPCODE(CurOpcode)==OVR_EVAL1)||
                            (OPCODE(CurOpcode)==OVR_XEQ) )
                    {
                        // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
                        WORD saveOpcode=CurOpcode;
                        CurOpcode=*rplPopData();
                        // RECURSIVE CALL
                        LIB_HANDLER();
                        CurOpcode=saveOpcode;
                        return;
                    }



                }
                rplError(ERR_COMPLEXEXPECTED);
                return;
            }

            rplReadCNumberAsReal(arg1,&Rarg1);
            rplReadCNumberAsImag(arg1,&Iarg1);
            rplDropData(1);
        }
        else {
            arg1=rplPeekData(2);
            arg2=rplPeekData(1);

            if(!ISNUMBERCPLX(*arg1) || !ISNUMBERCPLX(*arg2)) {
                rplError(ERR_COMPLEXORREALEXPECTED);
                return;
            }

            rplReadCNumberAsReal(arg1,&Rarg1);
            rplReadCNumberAsImag(arg1,&Iarg1);
            rplReadCNumberAsReal(arg2,&Rarg2);
            rplReadCNumberAsImag(arg2,&Iarg2);

            rplDropData(2);
        }

        switch(OPCODE(CurOpcode))
        {
        case OVR_ADD:
            // ADD THE REAL PART FIRST
            addReal(&RReg[0],&Rarg1,&Rarg2);
            addReal(&RReg[1],&Iarg1,&Iarg2);
            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[1]);

            rplRRegToComplexPush(0,1);
            return;

        case OVR_SUB:
            subReal(&RReg[0],&Rarg1,&Rarg2);
            subReal(&RReg[1],&Iarg1,&Iarg2);
            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[1]);

            rplRRegToComplexPush(0,1);

            return;

        case OVR_MUL:

            Context.precdigits+=8;
            mulReal(&RReg[0],&Rarg1,&Rarg2);
            mulReal(&RReg[1],&Iarg1,&Iarg2);
            subReal(&RReg[2],&RReg[0],&RReg[1]);
            mulReal(&RReg[0],&Rarg1,&Iarg2);
            mulReal(&RReg[1],&Iarg1,&Rarg2);
            Context.precdigits-=8;
            addReal(&RReg[3],&RReg[0],&RReg[1]);
            finalize(&RReg[2]);
            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[1]);
            rplRRegToComplexPush(2,3);
            return;

        case OVR_DIV:

            if(iszeroReal(&Rarg2)&&iszeroReal(&Iarg2)) {
                // HANDLE SPECIALS
                rplError(ERR_MATHDIVIDEBYZERO);
                return;
            }
            // (a+b*i)/(c+d*i) = (a+b*i)*(c-d*i)/((c+d*i)*(c-d*i)) = (a*c+b*d)/(c^2+d^2) + (b*c-a*d)/(c^2+d^2)*i
            Context.precdigits+=8;
            mulReal(&RReg[0],&Rarg1,&Rarg2);
            mulReal(&RReg[1],&Iarg1,&Iarg2);
            addReal(&RReg[2],&RReg[0],&RReg[1]);
            mulReal(&RReg[0],&Iarg1,&Rarg2);
            mulReal(&RReg[1],&Rarg1,&Iarg2);
            subReal(&RReg[3],&RReg[0],&RReg[1]);
            mulReal(&RReg[0],&Rarg2,&Rarg2);
            mulReal(&RReg[1],&Iarg2,&Iarg2);
            addReal(&RReg[4],&RReg[0],&RReg[1]);

            Context.precdigits-=8;

            // TODO: CHECK FOR DIV BY ZERO AND ISSUE ERROR
            divReal(&RReg[0],&RReg[2],&RReg[4]);
            divReal(&RReg[1],&RReg[3],&RReg[4]);

            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[1]);

            rplRRegToComplexPush(0,1);

            return;

        case OVR_POW:
        {
            Context.precdigits+=8;

            mulReal(&RReg[0],&Rarg1,&Rarg1);
            mulReal(&RReg[1],&Iarg1,&Iarg1);
            addReal(&RReg[2],&RReg[0],&RReg[1]);

            Context.precdigits-=8;


            hyp_sqrt(&RReg[2]);

            normalize(&RReg[0]);
            // RReg[8] = r
            copyReal(&RReg[8],&RReg[0]);

            trig_atan2(&Iarg1,&Rarg1,0);

            // RReg[9]=Theta
            copyReal(&RReg[9],&RReg[0]);

            // HERE WE HAVE 'Z' IN POLAR COORDINATES

            if(iszeroReal(&Iarg2)) {
                // REAL POWER OF A COMPLEX NUMEBR
                //Z^n= e^(ln(Z^n)) = e^(n*ln(Z)) = e^(n*[ln(r)+i*Theta)
                //Z^n= e^(n*ln(r))*e^(i*Theta*n) = r^n * e(i*Theta*n)
                //Z^n= r^n * cos(Theta*n) + i* r^n * sin(Theta*n)

                if(iszeroReal(&Rarg2)) {
                    // Z^0 = 1, UNLESS Z=0

                    if(iszeroReal(&Rarg1)&&iszeroReal(&Iarg1)) {
                        rplError(ERR_UNDEFINEDRESULT);
                        return;
                    }

                    rplPushData((WORDPTR)one_bint);
                    return;

                }

                hyp_ln(&RReg[8]);
                normalize(&RReg[0]);
                
                Context.precdigits+=8;

                // RReg[8]=n*ln(r);
                mulReal(&RReg[2],&RReg[0],&Rarg2);

                Context.precdigits-=8;


                hyp_exp(&RReg[2]);
                normalize(&RReg[0]);

                // RReg[8]=r^n
                copyReal(&RReg[8],&RReg[0]);

                Context.precdigits+=8;

                mulReal(&RReg[2],&Rarg2,&RReg[9]);

                Context.precdigits-=8;


                trig_sincos(&RReg[2],0);
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                // RESULT IS RReg[6]=cos(Theta*n) AND RReg[7]=sin(Theta*n)

                // RReg[0]=r^n*cos(Theta*n)
                mulReal(&RReg[0],&RReg[6],&RReg[8]);
                mulReal(&RReg[1],&RReg[7],&RReg[8]);


                rplRRegToComplexPush(0,1);
                return;
            }

            // COMPLEX NUMBER TO COMPLEX POWER

            // Z^w = e^ ( [a*ln(r) - b*Theta] ) * (cos[[b*ln(r)+a*Theta]+ i * sin[b*ln(r)+a*Theta] )
            // SO FAR WE HAVE RREG[8]=r, RREG[9]=Theta

            hyp_ln(&RReg[8]);
            normalize(&RReg[0]);

            // RREG[0]=ln(r)
            Context.precdigits+=8;

            mulReal(&RReg[1],&RReg[0],&Rarg2); // RReg[1]=a*ln(r)
            mulReal(&RReg[2],&RReg[0],&Iarg2); // RReg[2]=b*ln(r)
            mulReal(&RReg[3],&RReg[9],&Rarg2); // RReg[3]=a*Theta
            mulReal(&RReg[4],&RReg[9],&Iarg2); // RReg[3]=b*Theta
            subReal(&RReg[8],&RReg[1],&RReg[4]);   // RReg[8]=a*ln(r)-b*Theta
            addReal(&RReg[9],&RReg[2],&RReg[3]);   // RReg[9]=b*ln(r)+a*Theta

            Context.precdigits-=8;

            hyp_exp(&RReg[8]);
            normalize(&RReg[0]);

            copyReal(&RReg[8],&RReg[0]);

            trig_sincos(&RReg[9],0);
            normalize(&RReg[6]);
            normalize(&RReg[7]);

            mulReal(&RReg[0],&RReg[6],&RReg[8]);
            mulReal(&RReg[1],&RReg[7],&RReg[8]);

            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[1]);
            rplRRegToComplexPush(0,1);

            return;
        }
        case OVR_EQ:
         if(!eqReal(&Rarg1,&Rarg2)||!eqReal(&Iarg1,&Iarg2)) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        case OVR_NOTEQ:
            if(!eqReal(&Rarg1,&Rarg2)||!eqReal(&Iarg1,&Iarg2)) rplPushData((WORDPTR)one_bint);
               else rplPushData((WORDPTR)zero_bint);
               return;
/*
        // COMPARISONS ARE NOT DEFINED FOR COMPLEX NUMBERS
        case OVR_LT:
        case OVR_GT:
        case OVR_LTE:
        case OVR_GTE:



*/
        case OVR_SAME:
            if(!eqReal(&Rarg1,&Rarg2)||!eqReal(&Iarg1,&Iarg2)) rplPushData((WORDPTR)zero_bint);
               else rplPushData((WORDPTR)one_bint);
               return;
        case OVR_AND:
            if( (iszeroReal(&Rarg1)&&iszeroReal(&Iarg1))||(iszeroReal(&Rarg2)&&iszeroReal(&Iarg2))) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        case OVR_OR:
            if( (iszeroReal(&Rarg1)&&iszeroReal(&Iarg1))&&(iszeroReal(&Rarg2)&&iszeroReal(&Iarg2))) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        case OVR_XOR:
            if( (iszeroReal(&Rarg1)&&iszeroReal(&Iarg1))&&(iszeroReal(&Rarg2)&&iszeroReal(&Iarg2))) rplPushData((WORDPTR)zero_bint);
            else {
                if( !(iszeroReal(&Rarg1)&&iszeroReal(&Iarg1))&&!(iszeroReal(&Rarg2)&&iszeroReal(&Iarg2))) rplPushData((WORDPTR)zero_bint);
                else rplPushData((WORDPTR)one_bint);
            }
            return;

        case OVR_INV:
                // 1/(a+b*i) = (a/(a^2+b^2) - b/(a^2/b^2) i
                if( (iszeroReal(&Rarg1)&&iszeroReal(&Iarg1)) ) {
                    rplError(ERR_MATHDIVIDEBYZERO);
                    return;
                }

                Context.precdigits+=8;

                mulReal(&RReg[2],&Rarg1,&Rarg1);
                mulReal(&RReg[3],&Iarg1,&Iarg1);
                addReal(&RReg[4],&RReg[2],&RReg[3]);
                Context.precdigits-=8;

                divReal(&RReg[0],&Rarg1,&RReg[4]);
                divReal(&RReg[1],&Iarg1,&RReg[4]);
                RReg[1].flags^=F_NEGATIVE;
                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[1]);

                rplRRegToComplexPush(0,1);

                return;

        case OVR_NEG:
            if(!iszeroReal(&Rarg1)) Rarg1.flags^=F_NEGATIVE;
            if(!iszeroReal(&Iarg1)) Iarg1.flags^=F_NEGATIVE;

            rplNewComplexPush(&Rarg1,&Iarg1);

            return;
        case OVR_ABS:
                Context.precdigits+=8;
                mulReal(&RReg[2],&Rarg1,&Rarg1);
                mulReal(&RReg[3],&Iarg1,&Iarg1);
                addReal(&RReg[0],&RReg[2],&RReg[3]);

                Context.precdigits-=8;

                hyp_sqrt(&RReg[0]);
                finalize(&RReg[0]);

                rplNewRealFromRRegPush(0);
                return;
        case OVR_NOT:
            if(iszeroReal(&Rarg1)&&iszeroReal(&Iarg1)) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;




        case OVR_EVAL:
        case OVR_EVAL1:
        case OVR_XEQ:
        case OVR_NUM:
            // NOTHING TO DO, JUST KEEP THE ARGUMENT IN THE STACK
            rplPushData(arg1);
            return;


        // ADD MORE case's HERE
        default:
            rplError(ERR_COMPLEXNOTSUPPORTED);
        return;


        }
        return;
#undef arg1
#undef arg2


    }   // END OF OVERLOADABLE OPERATORS


    switch(OPCODE(CurOpcode))
    {

    case RE:
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISNUMBERCPLX(*rplPeekData(1))) {
            rplError(ERR_COMPLEXORREALEXPECTED);
            return;
        }
        if(ISCOMPLEX(*rplPeekData(1))) {
            WORDPTR real=rplPeekData(1)+1;
            rplOverwriteData(1,real);
        }
        // IF IT IS A REAL NUMBER, THEN LEAVE THE REAL ON THE STACK
        return;

    case IM:
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISNUMBERCPLX(*rplPeekData(1))) {
            rplError(ERR_COMPLEXORREALEXPECTED);
            return;
        }
        if(ISCOMPLEX(*rplPeekData(1))) {
            WORDPTR imag=rplSkipOb(rplPeekData(1)+1);
            rplOverwriteData(1,imag);
        } else {
            // NON-COMPLEX NUMBERS HAVE IMAGINARY PART = 0
            rplDropData(1);
            rplPushData((WORDPTR)zero_bint);
        }
        return;
    case ARG:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISNUMBERCPLX(*rplPeekData(1))) {
            rplError(ERR_COMPLEXORREALEXPECTED);
            return;
        }
            REAL real,imag;

            rplReadCNumberAsReal(rplPeekData(1),&real);
            rplReadCNumberAsImag(rplPeekData(1),&imag);

            BINT angmode;
            if(!rplTestSystemFlag(FL_ANGLEMODE1)) {
                // RADIANS MODE IS NOT SET, NEED TO PREPARE ARGUMENT
                if(rplTestSystemFlag(FL_ANGLEMODE2)) angmode=2;
                else angmode=1;
            } else angmode=0;

            trig_atan2(&imag,&real,angmode);
            finalize(&RReg[0]);

            rplDropData(1);
            rplNewRealFromRRegPush(0);

        return;
    }
    case CONJ:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISNUMBERCPLX(*rplPeekData(1))) {
            rplError(ERR_COMPLEXORREALEXPECTED);
            return;
        }
            REAL real,imag;

            rplReadCNumberAsReal(rplPeekData(1),&real);
            rplReadCNumberAsImag(rplPeekData(1),&imag);
            if(!iszeroReal(&imag)) imag.flags^=F_NEGATIVE;

            rplDropData(1);
            rplNewComplexPush(&real,&imag);

        return;
    }



    case CPLX2REAL:
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISCOMPLEX(*rplPeekData(1))) {
            rplError(ERR_COMPLEXEXPECTED);
            return;
        }
        ScratchPointer1=rplPeekData(1);
        rplPushData(ScratchPointer1+1);
        rplPushData(rplSkipOb(ScratchPointer1+1));
    return;

    case REAL2CPLX:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISNUMBER(*rplPeekData(1)) || !ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        // CONSTRUCT THE COMPLEX NUMBER
        BINT sizer=rplObjSize(rplPeekData(2));
        BINT sizei=rplObjSize(rplPeekData(1));
        WORDPTR newcplx=rplAllocTempOb(sizer+sizei);
        if(!newcplx) {
            return;
        }

        *newcplx=MKPROLOG(LIBRARY_NUMBER,sizer+sizei);
        rplCopyObject(newcplx+1,rplPeekData(2));
        rplCopyObject(newcplx+1+sizer,rplPeekData(1));

        rplDropData(2);
        rplPushData(newcplx);

    return;
    }







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

    {
        // COMPILE COMPLEX OBJECTS IN THE FORM ( X , Y ) BUT ALSO ACCEPT (X,Y) (NO SPACES)

        if(*((char * )TokenStart)=='(')
        {

            rplCompileAppend((WORD) MKPROLOG(LIBRARY_NUMBER,0));
            if(TokenLen>1) {
                NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                RetNum=OK_STARTCONSTRUCT;
            }
            else RetNum=OK_STARTCONSTRUCT;
            return;
        }
        // CHECK IF THE TOKEN IS THE CLOSING BRACKET

        if(((char * )TokenStart)[TokenLen-1]==')')
        {
            if(TokenLen>1) {
                BlankStart=NextTokenStart=(WORDPTR)(((char * )TokenStart)+TokenLen-1);
                RetNum=ERR_NOTMINE_SPLITTOKEN;
                return;
            }


            if(CurrentConstruct!=MKPROLOG(LIBRARY_NUMBER,2)) {
                RetNum=ERR_SYNTAX;
                return;
            }
            RetNum=OK_ENDCONSTRUCT;
            return;
        }

        // CHECK IF THE CURRENT CONSTRUCT IS A COMPLEX NUMBER AND IT CONTAINS A COMMA

        if( (LIBNUM(CurrentConstruct)==LIBRARY_NUMBER) && ISPROLOG(CurrentConstruct))
            {
            BINT count=TokenLen;
            BYTEPTR ptr=(BYTEPTR)TokenStart;
            WORD Locale=rplGetSystemLocale();


            // THERE'S 3 PLACES TO HAVE A COMMA
            // ENDING TOKEN
            // STARTING A TOKEN
            // IN THE MIDDLE WOULD BE PART OF A NUMBER

            if(*ptr==ARG_SEP(Locale)) {
               // STARTS WITH COMMA
                    if(TokenLen>1)  NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                    // WE DID NOT PRODUCE ANY OUTPUT, SO DON'T VALIDATE
                    RetNum=OK_CONTINUE_NOVALIDATE;
                    return;
            }
            if(ptr[count-1]==ARG_SEP(Locale)) {
                // ENDS WITH A COMMA, SPLIT THE TOKEN
                BlankStart=NextTokenStart=(WORDPTR)(ptr+count-1);
                RetNum=ERR_NOTMINE_SPLITTOKEN;
                return;
            }

            while(count && (*ptr!=ARG_SEP(Locale))) { ++ptr; --count; }

            if(count) {

              // THERE'S A COMMA IN THE MIDDLE
                  // SPLIT THE TOKEN
                  BlankStart=NextTokenStart=(WORDPTR)(ptr);
                  RetNum=ERR_NOTMINE_SPLITTOKEN;
                  return;
              }


            // THERE IS NO COMMA IN THIS TOKEN

            RetNum=ERR_NOTMINE;
            return;


        }



        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

        return;
    }
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        if(ISPROLOG(*DecompileObject)) {
            WORD Locale=rplGetSystemLocale();

            rplDecompAppendString((BYTEPTR)"(");

            // POINT TO THE REAL PART
            DecompileObject++;

            LIBHANDLER libhan=rplGetLibHandler(LIBNUM(*DecompileObject));
            if(libhan) (*libhan)();

            rplDecompAppendChar(ARG_SEP(Locale));

            // POINT TO THE IMAGINARY PART
            DecompileObject=rplSkipOb(DecompileObject);

            libhan=rplGetLibHandler(LIBNUM(*DecompileObject));
            if(libhan) (*libhan)();

            rplDecompAppendString((BYTEPTR)")");

            RetNum=OK_CONTINUE;

            return;
        }


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
        if(ISNUMBER(*LastCompiledObject)) RetNum=OK_INCARGCOUNT;
        else {
            rplError(ERR_NOTALLOWEDINCOMPLEX);
            RetNum=ERR_INVALID;
        }

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

        if(ISPROLOG(*ObjectPTR)) {
        WORDPTR real,imag;

        // CHECK MINIMUM SIZE
        if(OBJSIZE(*ObjectPTR)<2) { RetNum=ERR_INVALID; return; }
        // CHECK IF REAL PART IS A VALID NUMBER
        real=ObjectPTR+1;
        if(!ISNUMBER(*real)) { RetNum=ERR_INVALID; return; }
        // CHECK IF SIZE OF REAL PART FITS WITHIN COMPLEX OBJECT
        imag=rplSkipOb(real);
        if(imag>=rplSkipOb(ObjectPTR)) { RetNum=ERR_INVALID; return; }
        // CHECK IF IMAGINARY PART IS A VALID NUMBER
        if(!ISNUMBER(*imag)) { RetNum=ERR_INVALID; return; }
        // CHECK IF SIZE OF IMAGINARY PART IS CORRECT
        if(rplSkipOb(imag)!=rplSkipOb(ObjectPTR)) { RetNum=ERR_INVALID; return; }

        }
        RetNum=OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {\
        if(MENUNUMBER(MenuCodeArg)>0) RetNum=ERR_NOTMINE;
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR=ROMPTR_TABLE[MENUNUMBER(MenuCodeArg)+2];
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



