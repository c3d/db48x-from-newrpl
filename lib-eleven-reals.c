// LIBRARY ONE DEFINES THE BASIC TYPES BINT AND SINT

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  11
#define LIB_ENUM lib11_enum
#define LIB_NAMES lib11_names
#define LIB_HANDLER lib11_handler
#define LIB_NUMBEROFCMDS LIB11_NUMBEROFCMDS

typedef union {
    WORD word;
    struct {
    signed exp:16;
    unsigned len:8,digits:4,flags:4;
    };
} REAL_HEADER;

mpd_context_t CompileContext = {
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
    memcpy(RReg[num].data,real+2,sizeof(mpd_ssize_t)*RReg[num].len);
}


// CREATE A NEW CALCULATOR REAL AND PUSH IT ON THE STACK
// SET THE VALUE TO THE GIVEN RREG
void rplRRegToRealPush(int num)
{

    REAL_HEADER real;

    WORDPTR newreal=rplAllocTempOb(RReg[num].len+1);
    if(!newreal) {
        Exceptions|=EX_OUTOFMEM;
        return;
    }
    // WRITE THE PROLOG
    *newreal=MKPROLOG(LIBRARY_NUMBER,1+RReg[num].len);
    // PACK THE INFORMATION
    real.flags=RReg[num].flags&0xf;
    real.len=RReg[num].len;
    real.digits=RReg[num].digits-((RReg[num].len-1)*9);
    real.exp=RReg[num].exp;
    // STORE THE PACKED EXPONENT WORD
    newreal[1]=real.word;

    BINT count;
    for(count=0;count<RReg[num].len;++count) {
        newreal[count+2]=(RReg[num].data[count]);      // STORE ALL THE MANTISSA WORDS
    }

    rplPushData(newreal);
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
            rplRRegToRealPush(0);
            return;

        case OVR_SUB:
            mpd_sub(&RReg[0],&Darg1,&Darg2,&Context);
            rplRRegToRealPush(0);
            return;

        case OVR_MUL:
            mpd_mul(&RReg[0],&Darg1,&Darg2,&Context);
            rplRRegToRealPush(0);
            return;

        case OVR_DIV:
            mpd_div(&RReg[0],&Darg1,&Darg2,&Context);
            rplRRegToRealPush(0);
            return;

        case OVR_POW:
            mpd_pow(&RReg[0],&Darg1,&Darg2,&Context);
            rplRRegToRealPush(0);
            return;

        case OVR_EQ:

            if(mpd_cmp(&Darg1,&Darg2,&Context)) rplNewSINTPush(0,DECBINT);
            else rplNewSINTPush(1,DECBINT);
            return;

        case OVR_NOTEQ:
            if(mpd_cmp(&Darg1,&Darg2,&Context)) rplNewSINTPush(1,DECBINT);
            else rplNewSINTPush(0,DECBINT);
            return;
        case OVR_LT:
            if(mpd_cmp(&Darg1,&Darg2,&Context)==-1) rplNewSINTPush(1,DECBINT);
            else rplNewSINTPush(0,DECBINT);
            return;
        case OVR_GT:
            if(mpd_cmp(&Darg1,&Darg2,&Context)==1) rplNewSINTPush(1,DECBINT);
            else rplNewSINTPush(0,DECBINT);
            return;
        case OVR_LTE:
            if(mpd_cmp(&Darg1,&Darg2,&Context)!=1) rplNewSINTPush(1,DECBINT);
            else rplNewSINTPush(0,DECBINT);
            return;
        case OVR_GTE:
            if(mpd_cmp(&Darg1,&Darg2,&Context)!=-1) rplNewSINTPush(1,DECBINT);
            else rplNewSINTPush(0,DECBINT);
            return;
        case OVR_SAME:
            if(mpd_cmp(&Darg1,&Darg2,&Context)) rplNewSINTPush(0,DECBINT);
            else rplNewSINTPush(1,DECBINT);
            return;
        case OVR_AND:
            if(mpd_iszero(&Darg1)||mpd_iszero(&Darg2)) rplNewSINTPush(0,DECBINT);
            else rplNewSINTPush(1,DECBINT);
            return;
        case OVR_OR:
            if(mpd_iszero(&Darg1)&&mpd_iszero(&Darg2)) rplNewSINTPush(0,DECBINT);
            else rplNewSINTPush(1,DECBINT);
            return;




        case OVR_INV:
            rplOneToRReg(1);
            mpd_div(&RReg[0],&RReg[1],&Darg1,&Context);
            rplRRegToRealPush(0);
            return;
        case OVR_NEG:
            mpd_qminus(&RReg[0],&Darg1,&Context,(uint32_t *)&status);
            rplRRegToRealPush(0);
            return;
        case OVR_EVAL:
            // NOTHING TO DO, JUST KEEP THE ARGUMENT IN THE STACK
            rplPushData(arg1);
            return;
        case OVR_ABS:
            mpd_abs(&RReg[0],&Darg1,&Context);
            rplRRegToRealPush(0);
            return;
        case OVR_NOT:
            if(mpd_iszero(&Darg1)) rplOneToRReg(0);
            else rplZeroToRReg(0);
            rplRRegToRealPush(0);
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
        BINT status=0;
        mpd_qset_string2(&RReg[0],(const char *)TokenStart,(const char *)BlankStart,&CompileContext,(uint32_t *)&status);

        if(status& (MPD_Conversion_syntax | MPD_Invalid_context | MPD_Invalid_operation | MPD_Malloc_error | MPD_Overflow | MPD_Underflow )) {
            // THERE WAS SOME ERROR DURING THE CONVERSION, PROBABLY A SYNTAX ERROR
            RetNum=ERR_NOTMINE;
            return;
        }

            // WRITE THE PROLOG
            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER,1+RReg[0].len));
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
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;

}





