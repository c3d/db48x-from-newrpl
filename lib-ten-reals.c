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
#define LIBRARY_NUMBER  10
#define LIB_ENUM lib10_enum
#define LIB_NAMES lib10 _names
#define LIB_HANDLER lib10_handler
#define LIB_NUMBEROFCMDS LIB10_NUMBEROFCMDS

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,LIBRARY_NUMBER|APPROX_BIT,0 };



typedef union {
    WORD word;
    struct {
    signed exp:16;
    unsigned len:8,digits:4,flags:4;
    };
} REAL_HEADER;

const mpd_context_t CompileContext = {
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


// SET THE REGISTER TO THE NUMBER 0NE
void rplOneToRReg(int num)
{
    RReg[num].digits=1;
    RReg[num].exp=0;
    RReg[num].flags=MPD_STATIC | MPD_STATIC_DATA;
    RReg[num].len=1;
    RReg[num].data[0]=1;
}

// SET THE REGISTER TO ZERO
void rplZeroToRReg(int num)
{
    RReg[num].digits=1;
    RReg[num].exp=0;
    RReg[num].flags=MPD_STATIC | MPD_STATIC_DATA;
    RReg[num].len=1;
    RReg[num].data[0]=0;
}

void rplBINTToRReg(int num,BINT64 value)
{
    // CLEAR ALL FLAGS
    RReg[num].flags=MPD_STATIC|MPD_STATIC_DATA;
    mpd_set_i64(&RReg[num],value,&Context);
}


// EXTRACT A CALCULATOR REAL INTO AN EXISTING mpd_t STRUCTURE
// DATA IS **NOT** COPIED
// DO **NOT** USE THIS FUNCTION WITH RREG REGISTERS
void rplReadReal(WORDPTR real,mpd_t *dec)
{
REAL_HEADER *head=(REAL_HEADER *)(real+1);
dec->flags=MPD_STATIC | MPD_STATIC_DATA;
dec->alloc=OBJSIZE(*real)-1;
dec->data=(uint32_t *) (real+2);
dec->len=head->len;
dec->exp=head->exp;
dec->flags|=head->flags;
dec->digits=head->digits+(head->len-1)*9;
}

// EXTRACT A CALCULATOR REAL INTO A RREG REGISTER
void rplCopyRealToRReg(int num,WORDPTR real)
{
    REAL_HEADER *head=(REAL_HEADER *)(real+1);
    RReg[num].flags=MPD_STATIC | MPD_STATIC_DATA;
    RReg[num].len=head->len;
    RReg[num].exp=head->exp;
    RReg[num].flags|=head->flags;
    RReg[num].digits=head->digits+(head->len-1)*9;
    memcpyw(RReg[num].data,real+2,RReg[num].len);
}


// CREATE A NEW CALCULATOR REAL AND PUSH IT ON THE STACK
// SET THE VALUE TO THE GIVEN RREG
void rplNewRealFromRRegPush(int num)
{
    WORDPTR newreal=rplNewRealFromRReg(num);
    if(newreal) rplPushData(newreal);
}

void rplNewApproxRealFromRRegPush(int num)
{
    WORDPTR newreal=rplNewRealFromRReg(num);
    if(newreal) {
        *newreal|=MKOPCODE(APPROX_BIT,0);
        rplPushData(newreal);
    }
}


void rplNewRealPush(mpd_t *num)
{
    WORDPTR newreal=rplNewReal(num);
    if(newreal) rplPushData(newreal);
}

void rplNewApproxRealPush(mpd_t *num)
{
    WORDPTR newreal=rplNewReal(num);
    if(newreal) {
        *newreal|=MKOPCODE(APPROX_BIT,0);
        rplPushData(newreal);
    }
}

// ALLOCATE MEMORY AND STORE A REAL ON IT
WORDPTR rplNewReal(mpd_t *num)
{

    REAL_HEADER real;
    BINT correction;

    WORDPTR newreal=rplAllocTempOb(num->len+1);
    if(!newreal) {
        Exceptions|=EX_OUTOFMEM;
        return 0;
    }

    // REMOVE ALL TRAILING ZEROES
    correction=0;
    while(correction<num->len-1)
    {
        if(num->data[correction]!=0) break;
        ++correction;
    }


    // WRITE THE PROLOG
    *newreal=MKPROLOG(LIBRARY_NUMBER,1+num->len-correction);
    // PACK THE INFORMATION
    real.flags=num->flags&0xf;
    real.len=num->len-correction;
    real.digits=num->digits-((num->len-1)*9);
    real.exp=num->exp+correction*9;
    // STORE THE PACKED EXPONENT WORD
    newreal[1]=real.word;

    BINT count;
    for(count=0;count<num->len-correction;++count) {
        newreal[count+2]=(num->data[count+correction]);      // STORE ALL THE MANTISSA WORDS
    }

    return newreal;
}
WORDPTR rplNewRealFromRReg(int num)
{
return rplNewReal(&RReg[num]);
}

// STORE A REAL IN dest, AND RETURN A POINTER RIGHT AFTER THE OBJECT
// DOES NOT ALLOCATE MEMORY FROM THE SYSTEM
// USED INTERNALLY FOR COMPOSITE OBJECTS

WORDPTR rplRRegToRealInPlace(int num,WORDPTR dest,BINT isapprox)
{

    REAL_HEADER real;
    BINT correction;

    // REMOVE ALL TRAILING ZEROES
    correction=0;
    while(correction<RReg[num].len-1)
    {
        if(RReg[num].data[correction]!=0) break;
        ++correction;
    }


    // WRITE THE PROLOG
    *dest=MKPROLOG((LIBRARY_NUMBER|(isapprox? APPROX_BIT:0)),1+RReg[num].len-correction);
    // PACK THE INFORMATION
    real.flags=RReg[num].flags&0xf;
    real.len=RReg[num].len-correction;
    real.digits=RReg[num].digits-((RReg[num].len-1)*9);
    real.exp=RReg[num].exp+correction*9;
    // STORE THE PACKED EXPONENT WORD
    dest[1]=real.word;

    BINT count;
    for(count=0;count<RReg[num].len-correction;++count) {
        dest[count+2]=(RReg[num].data[count+correction]);      // STORE ALL THE MANTISSA WORDS
    }

    return dest+count+2;

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
        mpd_t Darg1,Darg2;
        int status;

        if(rplDepthData()<nargs) {
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(nargs==1) {
            // UNARY OPERATORS
            arg1=rplPeekData(1);
            if(!ISREAL(*arg1)) {
                Exceptions=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }
            rplReadReal(arg1,&Darg1);
            rplDropData(1);
        }
        else {
            arg1=rplPeekData(2);
            arg2=rplPeekData(1);

            if(!ISREAL(*arg1) || !ISREAL(*arg2)) {
                Exceptions=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
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
            mpd_add(&RReg[0],&Darg1,&Darg2,&Context);
            if(ISAPPROX(*arg1|*arg2)) rplNewApproxRealFromRRegPush(0);
            else rplNewRealFromRRegPush(0);
            return;

        case OVR_SUB:
            mpd_sub(&RReg[0],&Darg1,&Darg2,&Context);
            if(ISAPPROX(*arg1|*arg2)) rplNewApproxRealFromRRegPush(0);
            else rplNewRealFromRRegPush(0);
            return;

        case OVR_MUL:
            mpd_mul(&RReg[0],&Darg1,&Darg2,&Context);
            if(ISAPPROX(*arg1|*arg2)) rplNewApproxRealFromRRegPush(0);
            else rplNewRealFromRRegPush(0);
            return;

        case OVR_DIV:
            mpd_div(&RReg[0],&Darg1,&Darg2,&Context);
            if(ISAPPROX(*arg1|*arg2)) rplNewApproxRealFromRRegPush(0);
            else rplNewRealFromRRegPush(0);
            return;

        case OVR_POW:
            RReg[1].data[0]=5;
            RReg[1].exp=-1;
            RReg[1].len=1;
            RReg[1].digits=1;
            RReg[1].flags&=MPD_DATAFLAGS;

            Context.status&=~MPD_Inexact;
            if(mpd_cmp(&Darg2,&RReg[1],&Context)==0)
                // THIS IS A SQUARE ROOT
                mpd_sqrt(&RReg[0],&Darg1,&Context);
            else mpd_pow(&RReg[0],&Darg1,&Darg2,&Context);

            if(ISAPPROX(*arg1|*arg2) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(0);
            else rplNewRealFromRRegPush(0);
            return;

        case OVR_EQ:

            if(mpd_cmp(&Darg1,&Darg2,&Context)) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;

        case OVR_NOTEQ:
            if(mpd_cmp(&Darg1,&Darg2,&Context)) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        case OVR_LT:
            if(mpd_cmp(&Darg1,&Darg2,&Context)==-1) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        case OVR_GT:
            if(mpd_cmp(&Darg1,&Darg2,&Context)==1) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        case OVR_LTE:
            if(mpd_cmp(&Darg1,&Darg2,&Context)!=1) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        case OVR_GTE:
            if(mpd_cmp(&Darg1,&Darg2,&Context)!=-1) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        case OVR_SAME:
            if(mpd_cmp(&Darg1,&Darg2,&Context)) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        case OVR_AND:
            if(mpd_iszero(&Darg1)||mpd_iszero(&Darg2)) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        case OVR_OR:
            if(mpd_iszero(&Darg1)&&mpd_iszero(&Darg2)) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        case OVR_CMP:
            rplNewSINTPush(mpd_cmp(&Darg1,&Darg2,&Context),DECBINT);
            return;





        case OVR_INV:
            rplOneToRReg(1);
            Context.status&=~MPD_Inexact;
            mpd_div(&RReg[0],&RReg[1],&Darg1,&Context);
            if(ISAPPROX(*arg1) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(0);
            else rplNewRealFromRRegPush(0);
            return;
        case OVR_NEG:
        case OVR_UMINUS:
            mpd_qminus(&RReg[0],&Darg1,&Context,(uint32_t *)&status);
            if(ISAPPROX(*arg1)) rplNewApproxRealFromRRegPush(0);
            else rplNewRealFromRRegPush(0);
            return;
        case OVR_EVAL:
        case OVR_EVAL1:
        case OVR_XEQ:
        case OVR_NUM:
            // NOTHING TO DO, JUST KEEP THE ARGUMENT IN THE STACK
            rplPushData(arg1);
            return;
        case OVR_ABS:
            mpd_abs(&RReg[0],&Darg1,&Context);
            if(ISAPPROX(*arg1) || (Context.status&MPD_Inexact)) rplNewApproxRealFromRRegPush(0);
            else rplNewRealFromRRegPush(0);
            return;
        case OVR_NOT:
            if(mpd_iszero(&Darg1)) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
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



        // COMPILE A NUMBER TO A REAL
    {
        if(LIBNUM(CurOpcode)&APPROX_BIT) {
            // DO NOT COMPILE ANYTHING WHEN CALLED WITH THE UPPER (APPROX) LIBRARY NUMBER
            RetNum=ERR_NOTMINE;
            return;
        }

        BINT status=0;
        BYTEPTR strptr=(BYTEPTR )TokenStart;
        BINT isapprox=0;
        BINT tlen=TokenLen;

        if(strptr[tlen-1]=='.') {
         // NUMBERS ENDING IN A DOT ARE APPROXIMATED
            isapprox|=APPROX_BIT;
            --tlen;
        }


        mpd_qset_string2(&RReg[0],(const char *)TokenStart,(const char *)(strptr+tlen),&CompileContext,(uint32_t *)&status);

        if(status& (MPD_Conversion_syntax | MPD_Invalid_context | MPD_Invalid_operation | MPD_Malloc_error | MPD_Overflow | MPD_Underflow )) {
            // THERE WAS SOME ERROR DURING THE CONVERSION, PROBABLY A SYNTAX ERROR
            RetNum=ERR_NOTMINE;
            return;
        }

            // WRITE THE PROLOG
            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER|isapprox,1+RReg[0].len));
            // PACK THE INFORMATION
            REAL_HEADER real;
            real.flags=RReg[0].flags&0xf;
            real.len=RReg[0].len;
            real.digits=RReg[0].digits-((RReg[0].len-1)*9);
            real.exp=RReg[0].exp;

            rplCompileAppend(real.word);      // CAREFUL: THIS IS FOR LITTLE ENDIAN SYSTEMS ONLY!
            BINT count;
            for(count=0;count<RReg[0].len;++count) {
                rplCompileAppend(RReg[0].data[count]);      // STORE ALL THE MANTISSA WORDS
            }
            RetNum=OK_CONTINUE;
     return;
    }
    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Byte Ptr to end of current string. Write here with rplDecompAppendString(); rplDecompAppendChar();
    {
        mpd_t realnum;

        rplReadReal(DecompileObject,&realnum);


        // CONVERT TO STRING
        // THIS NEEDS TO BE CHANGED, SINCE IT RELIES ON MALLOC
        // NEED A FUNCTION THAT CALLS rplDecompAppendChar DIRECTLY

        BYTEPTR string;
        BINT len=(BINT)mpd_to_sci_size((char **)&string,&realnum,1);
        if(string) {
        rplDecompAppendString2(string,len);
        if(LIBNUM(*DecompileObject)&APPROX_BIT) rplDecompAppendChar('.');
        mpd_free(string);
        RetNum=OK_CONTINUE;
        }
        else RetNum=ERR_INVALID;

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
            MODE_EXPSIGN,
            MODE_EXP
        };
        BINT mode=MODE_IP;
        BYTE num;
        int f,exitfor=0;
        BYTEPTR ptr=(BYTEPTR)TokenStart;

        for(f=0;f<(int)TokenLen;++f,++ptr) {
            num=*ptr;
            switch(mode)
            {
            case MODE_IP:
                if(num=='.') { mode=MODE_FP; break; }
                if(num=='e' || num=='E') { mode=MODE_EXPSIGN; break; }
                if(num<'0' || num>'9') { exitfor=1; break; }
                break;
            case MODE_FP:
                if(num=='e' || num=='E') { mode=MODE_EXPSIGN; break; }
                if(num<'0' || num>'9') { exitfor=1; break; }
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
    case OPCODE_LIBINSTALL:
        RetNum=(UBINT)libnumberlist;
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





