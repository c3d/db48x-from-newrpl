/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  26
#define LIB_ENUM lib26_enum
#define LIB_NAMES lib26_names
#define LIB_HANDLER lib26_handler
#define LIB_NUMBEROFCMDS LIB26_NUMBEROFCMDS



// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(RE), \
    CMD(IM)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
/*
#define CMD_EXTRANAME \
    ""
#define CMD_EXTRAENUM \
    NOP
*/
//

// INTERNAL DECLARATIONS


// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST /*, CMD_EXTRAENUM*/ , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
char *LIB_NAMES[]= { CMD_LIST /*, CMD_EXTRANAME */ };
#undef CMD





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
/*
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
            RReg[1].data[0]=5;
            RReg[1].exp=-1;
            RReg[1].len=1;
            RReg[1].digits=1;
            RReg[1].flags&=MPD_DATAFLAGS;

            if(mpd_cmp(&Darg2,&RReg[1],&Context)==0)
                // THIS IS A SQUARE ROOT
                mpd_sqrt(&RReg[0],&Darg1,&Context);
            else mpd_pow(&RReg[0],&Darg1,&Darg2,&Context);
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
        case OVR_XEQ:
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

*/
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


        // COMPILE COMPLEX OBJECTS IN THE FORM ( X , Y ) BUT ALSO ACCEPT (X,Y) (NO SPACES)

        if(*((char * )TokenStart)=='(')
        {

            rplCompileAppend((WORD) MKPROLOG(LIBRARY_NUMBER,0));
            if(TokenLen>1) {
                NextTokenStart=((char *)TokenStart)+1;
                RetNum=OK_STARTCONSTRUCT;
            }
            else RetNum=OK_STARTCONSTRUCT;
            return;
        }
        // CHECK IF THE TOKEN IS THE CLOSING BRACKET

        if(((char * )TokenStart)[TokenLen-1]==')')
        {
            if(TokenLen>1) {
                BlankStart=NextTokenStart=((char * )TokenStart)+TokenLen-1;
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
            while(count && (((char)*ptr)!=',')) { ++ptr; --count; }
            if(count) {
                if(ptr==TokenStart) {
                    // STARTS WITH COMMA
                    if(TokenLen>1)  NextTokenStart=((char *)TokenStart)+1;
                    // WE DID NOT PRODUCE ANY OUTPUT, SO DON'T VALIDATE
                    RetNum=OK_CONTINUE_NOVALIDATE;
                    return;
                }
                // FOUND A COMMA IN THE MIDDLE, SPLIT THE TOKEN
                BlankStart=NextTokenStart=(WORDPTR)ptr;
                RetNum=ERR_NOTMINE_SPLITTOKEN;
                return;
            }

            // THERE IS NO COMMA IN THIS TOKEN

            RetNum=ERR_NOTMINE;
            return;


        }



        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

        return;

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        if(ISPROLOG(*DecompileObject)) {
            rplDecompAppendString((BYTEPTR)"(");

            // POINT TO THE REAL PART
            DecompileObject++;

            LIBHANDLER libhan=rplGetLibHandler(LIBNUM(*DecompileObject));
            if(libhan) (*libhan)();

            rplDecompAppendString((BYTEPTR)",");

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
        libDecompileCmds(LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
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
        else RetNum=ERR_INVALID;

        return;

    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;

}






