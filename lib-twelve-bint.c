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


//#define _ISBINT(w) (((LIBNUM(w))&~3)==12)

// MACRO TO GET NUMBER OF BITS IN THE BASE
// 1= BINARY, 3=OCTAL, 4=HEX, AND 2=DECIMAL

#define GETBASE(libnum) ((libnum)-(DECBINT-2))

#define LIBFROMBASE(base) ((base)+(DECBINT-2))


#define MIN_SINT    -131072
#define MAX_SINT    +131071
#define MIN_BINT    -9223372036854775808LL
#define MAX_BINT    +9223372036854775807LL

BINT64 powersof10[20]={
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

char alldigits[]="0123456789ABCDEF";

void rplNewSINTPush(int num,int base)
{
    WORDPTR obj;
    obj=rplAllocTempOb(0);
    if(!obj) return;
    *obj=MKOPCODE(base,num&0x3ffff);
    rplPushData(obj);

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
// USES RREG[0] AS TEMPORARY DATA STORAGE FOR dec
void rplReadNumberAsReal(WORDPTR number,mpd_t*dec)
{
    if(ISREAL(*number)) rplReadReal(number,dec);
    else if(ISBINT(*number))  {
        // PROVIDE STORAGE
        dec->alloc=RReg[0].alloc;
        dec->data=RReg[0].data;
        mpd_set_i64(dec,rplReadBINT(number),&Context);
    }
    else {
        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
    }

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
        BINT64 op1,op2;
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
            // ADD TWO NUMBERS FROM THE STACK
            if(op1type||op2type) {
                if(op1type) {
                    rplBINTToRReg(1,op2);
                    mpd_add(&RReg[0],&rop1,&RReg[1],&Context);
                }

                if(op2type) {
                    // TODO: TRY TO RESPECT THE NUMBER TYPE OF THE FIRST ARGUMENT
                    rplBINTToRReg(1,op1);
                    mpd_add(&RReg[0],&RReg[1],&rop2,&Context);
                }
                rplRRegToRealPush(0);
                return;
            }
            op1+=op2;
            rplNewBINTPush(op1,LIBNUM(*arg1));
            return;

        case OVR_SUB:
            if(op1type||op2type) {
                if(op1type) {
                    rplBINTToRReg(1,-op2);
                    mpd_add(&RReg[0],&rop1,&RReg[1],&Context);
                }

                if(op2type) {
                    // TODO: TRY TO RESPECT THE NUMBER TYPE OF THE FIRST ARGUMENT
                    rplBINTToRReg(1,op1);
                    mpd_sub(&RReg[0],&RReg[1],&rop2,&Context);
                }
                rplRRegToRealPush(0);
                return;
            }
            op1-=op2;
            rplNewBINTPush(op1,LIBNUM(*arg1));
            return;

        case OVR_MUL:
            if(op1type||op2type) {
                if(op1type) {
                    rplBINTToRReg(1,op2);
                    mpd_mul(&RReg[0],&rop1,&RReg[1],&Context);
                }

                if(op2type) {
                    // TODO: TRY TO RESPECT THE NUMBER TYPE OF THE FIRST ARGUMENT
                    rplBINTToRReg(1,op1);
                    mpd_mul(&RReg[0],&RReg[1],&rop2,&Context);
                }
                rplRRegToRealPush(0);
                return;
            }
            // TODO: DETECT OVERFLOW, AND CONVERT TO INTEGERS IF SO
            op1*=op2;
            rplNewBINTPush(op1,LIBNUM(*arg1));
            return;

        case OVR_DIV:
            {
            if(op1type||op2type) {
                if(op1type) {
                    rplBINTToRReg(1,op2);
                    mpd_div(&RReg[0],&rop1,&RReg[1],&Context);
                }

                if(op2type) {
                    // TODO: TRY TO RESPECT THE NUMBER TYPE OF THE FIRST ARGUMENT
                    rplBINTToRReg(1,op1);
                    mpd_div(&RReg[0],&RReg[1],&rop2,&Context);
                }
                rplRRegToRealPush(0);
                return;
            }
            // TODO: BINT/BINT = REAL IN APPROX. MODE??
            rplBINTToRReg(1,op1);
            rplBINTToRReg(2,op2);
            mpd_div(&RReg[0],&RReg[1],&RReg[2],&Context);
            if(Exceptions) return;
            int status=0;
            BINT64 result=mpd_qget_i64(&RReg[0],&status);
            if(status) rplRRegToRealPush(0);
            else rplNewBINTPush(result,LIBNUM(*arg1));
            return;
            }

        case OVR_POW:
            {
            if(op1type||op2type) {
                if(op1type) {
                    rplBINTToRReg(1,op2);
                    mpd_pow(&RReg[0],&rop1,&RReg[1],&Context);
                }

                if(op2type) {
                    // TODO: TRY TO RESPECT THE NUMBER TYPE OF THE FIRST ARGUMENT
                    rplBINTToRReg(1,op1);
                    mpd_pow(&RReg[0],&RReg[1],&rop2,&Context);
                }
                rplRRegToRealPush(0);
                return;
            }


            // INTEGER POWER, USE REALS TO DEAL WITH NEGATIVE POWERS AND OVERFLOW
            rplBINTToRReg(1,op1);
            rplBINTToRReg(2,op2);
            mpd_pow(&RReg[0],&RReg[1],&RReg[2],&Context);
            int status=0;
            BINT64 result=mpd_qget_i64(&RReg[0],&status);
            if(status) rplRRegToRealPush(0);
            else rplNewBINTPush(result,LIBNUM(*arg1));
            return;
            }

        case OVR_EQ:
        {
        if(op1type||op2type) {
            if(op1type) {
                // ROUND TO INTEGER
                status=0;
                mpd_qround_to_intx(&RReg[1],&rop1,&Context,(uint32_t *)&status);
                // IF MPD_Rounded OR MPD_Inexact, IT CAN'T BE EQUAL TO A BINT
                if(status) rplNewSINTPush(0,DECBINT);
                else {
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&RReg[1],&RReg[0],&Context);
                if(res) res=0;
                else res=1;
                rplNewSINTPush(res,DECBINT);
                }
            }

            if(op2type) {
                // ROUND TO INTEGER
                status=0;
                mpd_qround_to_intx(&RReg[1],&rop2,&Context,(uint32_t *)&status);
                // IF MPD_Rounded OR MPD_Inexact, IT CAN'T BE EQUAL TO A BINT
                if(status) rplNewSINTPush(0,DECBINT);
                else {
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&RReg[1],&Context);
                if(res) res=0;
                else res=1;
                rplNewSINTPush(res,DECBINT);
                }
            }
            return;
        }
            // BOTH WERE INTEGERS
            if(op1==op2) rplNewSINTPush(1,DECBINT);
            else rplNewSINTPush(0,DECBINT);
            return;
        }

        case OVR_NOTEQ:
        {
        if(op1type||op2type) {
            if(op1type) {
                // ROUND TO INTEGER
                status=0;
                mpd_qround_to_intx(&RReg[1],&rop1,&Context,(uint32_t *)&status);
                // IF MPD_Rounded OR MPD_Inexact, IT CAN'T BE EQUAL TO A BINT
                if(status) rplNewSINTPush(1,DECBINT);
                else {
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&RReg[1],&RReg[0],&Context);
                if(res) res=1;
                rplNewSINTPush(res,DECBINT);
                }
            }

            if(op2type) {
                // ROUND TO INTEGER
                status=0;
                mpd_qround_to_intx(&RReg[1],&rop2,&Context,(uint32_t *)&status);
                // IF MPD_Rounded OR MPD_Inexact, IT CAN'T BE EQUAL TO A BINT
                if(status) rplNewSINTPush(1,DECBINT);
                else {
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&RReg[1],&Context);
                if(res) res=1;
                rplNewSINTPush(res,DECBINT);
                }
            }
            return;
        }
            if(op1==op2) rplNewSINTPush(0,DECBINT);
            else rplNewSINTPush(1,DECBINT);
            return;
        }
        case OVR_LT:
        {
        if(op1type||op2type) {
            if(op1type) {
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&rop1,&RReg[0],&Context);
                if(res<0) res=1;
                else res=0;
                rplNewSINTPush(res,DECBINT);
            }
            if(op2type) {
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&rop2,&Context);
                if(res<0) res=1;
                else res=0;
                rplNewSINTPush(res,DECBINT);
                }
            return;
            }

            if(op1<op2) rplNewSINTPush(1,DECBINT);
            else rplNewSINTPush(0,DECBINT);
            return;
        }
        case OVR_GT:
        {
        if(op1type||op2type) {
            if(op1type) {
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&rop1,&RReg[0],&Context);
                if(res>0) res=1;
                else res=0;
                rplNewSINTPush(res,DECBINT);
            }
            if(op2type) {
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&rop2,&Context);
                if(res>0) res=1;
                else res=0;
                rplNewSINTPush(res,DECBINT);
                }
            return;
            }

            if(op1>op2) rplNewSINTPush(1,DECBINT);
            else rplNewSINTPush(0,DECBINT);
            return;
        }



        case OVR_LTE:
        {
        if(op1type||op2type) {
            if(op1type) {
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&rop1,&RReg[0],&Context);
                if(res<=0) res=1;
                else res=0;
                rplNewSINTPush(res,DECBINT);
            }
            if(op2type) {
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&rop2,&Context);
                if(res<=0) res=1;
                else res=0;
                rplNewSINTPush(res,DECBINT);
                }
            return;
            }

            if(op1<=op2) rplNewSINTPush(1,DECBINT);
            else rplNewSINTPush(0,DECBINT);
            return;
        }
        case OVR_GTE:
        {
        if(op1type||op2type) {
            if(op1type) {
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&rop1,&RReg[0],&Context);
                if(res>=0) res=1;
                else res=0;
                rplNewSINTPush(res,DECBINT);
            }
            if(op2type) {
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&rop2,&Context);
                if(res>=0) res=1;
                else res=0;
                rplNewSINTPush(res,DECBINT);
                }
            return;
            }

            if(op1>=op2) rplNewSINTPush(1,DECBINT);
            else rplNewSINTPush(0,DECBINT);
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
                if(status) rplNewSINTPush(0,DECBINT);
                else {
                rplBINTToRReg(0,op2);
                int res=mpd_cmp(&RReg[1],&RReg[0],&Context);
                if(res) res=0;
                else res=1;
                rplNewSINTPush(res,DECBINT);
                }
            }

            if(op2type) {
                // ROUND TO INTEGER
                status=0;
                mpd_qround_to_intx(&RReg[1],&rop2,&Context,(uint32_t *)&status);
                // IF MPD_Rounded OR MPD_Inexact, IT CAN'T BE EQUAL TO A BINT
                if(status) rplNewSINTPush(0,DECBINT);
                else {
                rplBINTToRReg(0,op1);
                int res=mpd_cmp(&RReg[0],&RReg[1],&Context);
                if(res) res=0;
                else res=1;
                rplNewSINTPush(res,DECBINT);
                }
            }
            return;
        }
            // BOTH WERE INTEGERS
            if(op1==op2) rplNewSINTPush(1,DECBINT);
            else rplNewSINTPush(0,DECBINT);
            return;
        }




        case OVR_AND:
            if(op1type) op1=!mpd_iszero(&rop1);
            if(op2type) op2=!mpd_iszero(&rop2);
            if(op1&&op2) rplNewSINTPush(1,DECBINT);
            else rplNewSINTPush(0,DECBINT);
            return;
        case OVR_OR:
            if(op1type) op1=!mpd_iszero(&rop1);
            if(op2type) op2=!mpd_iszero(&rop2);
            if(op1||op2) rplNewSINTPush(1,DECBINT);
            else rplNewSINTPush(0,DECBINT);
            return;


        case OVR_INV:
            // INVERSE WILL ALWAYS BE A REAL, SINCE 1/N == 0 FOR ALL N>1 IN STRICT INTEGER MATH, ORIGINAL UserRPL DOES NOT SUPPORT INVERSE OF INTEGERS
            rplOneToRReg(0);
            rplBINTToRReg(1,op1);
            mpd_div(&RReg[2],&RReg[0],&RReg[1],&Context);
            rplRRegToRealPush(2);
            return;
        case OVR_NEG:
            op1=-op1;
            rplNewBINTPush(op1,LIBNUM(*arg1));
            return;
        case OVR_EVAL:
            // NOTHING TO DO, JUST KEEP THE ARGUMENT IN THE STACK
            rplPushData(arg1);
            return;
        case OVR_ABS:
            if(op1<0) rplNewBINTPush(-op1,LIBNUM(*arg1));
            else rplPushData(arg1);
            return;
        case OVR_NOT:
            if(op1) rplNewSINTPush(0,DECBINT);
            else rplNewSINTPush(1,DECBINT);
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
    BYTEPTR strptr;
    int base,libbase,digit,count,neg,argnum1;
    char basechr;

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
            if( (basechr=='h') || (basechr=='H')) { base=16; libbase=DECBINT+2; --argnum1; }
            if( (basechr=='o') || (basechr=='O')) { base=8; libbase=DECBINT+1; --argnum1; }
            if( (basechr=='b') || (basechr=='B')) { base=2; libbase=DECBINT-1; --argnum1; }
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

            base=GETBASE(LIBNUM(*DecompileObject));

            if(result<0) {
                rplDecompAppendChar('-');
                result=-result;
            }


            if(base==2) {
                // THIS IS A BASE-10 NUMBER
                digit=0;
                basechr='0';
                while(result<powersof10[digit]) ++digit;  // SKIP ALL LEADING ZEROS
                // NOW DECOMPILE THE NUMBER
                while(digit<18) {
                while(result>=powersof10[digit]) { ++basechr; result-=powersof10[digit]; }
                rplDecompAppendChar(basechr);
                ++digit;
                basechr='0';
                }
                basechr+=result;
                rplDecompAppendChar(basechr);
            }
            else {
            // THIS IS A BINARY, OCTAL OR HEXA NUMBER
            // base HAS THE NUMBER OF BITS PER DIGIT
            rplDecompAppendChar('#');

            digit=64-base;

            neg=(1<<base)-1;    // CREATE A MASK TO ISOLATE THE DIGIT

            // SKIP ALL LEADING ZEROS
            while(digit>0) {
                if( (result>>digit)&neg ) break;
                digit-=base;
            }
            // NOW DECOMPILE THE NUMBER
            while(digit>=0) {
                rplDecompAppendChar(alldigits[(result>>digit)&neg]);
                digit-=base;
            }

            // ADD BASE CHARACTER
            if(base==1) rplDecompAppendChar('b');
            if(base==3) rplDecompAppendChar('o');
            if(base==4) rplDecompAppendChar('h');

            }

            RetNum=OK_CONTINUE;

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        return;
    case OPCODE_VALIDATE:
        // VALIDATE RECEIVES OPCODES COMPILED BY OTHER LIBRARIES, TO BE INCLUDED WITHIN A COMPOSITE OWNED BY
        // THIS LIBRARY. EVERY COMPOSITE HAS TO EVALUATE IF THE OBJECT BEING COMPILED IS ALLOWED INSIDE THIS
        // COMPOSITE OR NOT. FOR EXAMPLE, A REAL MATRIX SHOULD ONLY ALLOW REAL NUMBERS INSIDE, ANY OTHER
        // OPCODES SHOULD BE REJECTED AND AN ERROR THROWN.
        // ArgPtr1 = token string
        // ArgNum1 = token length
        // ArgPtr2 =
        // ArgNum2 = Opcode/WORD of object

        // VALIDATE RETURNS:
        // RetNum =  enum CompileErrors

        // ONLY USED ON COMPOSITES, DO NOTHING

        return;


    default:
        // ALL OTHER OPCODES ARE SINT NUMBERS, JUST PUSH THEM ON THE STACK
        rplPushData(IPtr);
        return;

    }
// DON'T ISSUE A BAD_OPCODE ERROR SINCE ALL OPCODES ARE VALID SINT NUMBERS
}




