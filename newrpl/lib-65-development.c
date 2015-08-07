/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

#include <stdio.h>



extern void trig_sincos(REAL *ptr);
extern void trig_atan2(REAL *x,REAL *y);







// LIB 65 PROVIDES COMMANDS THAT ARE ONLY TEMPORARY AND USED FOR DEBUG/DEVELOPMENT

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  65
#define LIB_ENUM lib65_enum
#define LIB_NAMES lib65_names
#define LIB_HANDLER lib65_handler
#define LIB_NUMBEROFCMDS LIB65_NUMBEROFCMDS

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(TRANSCENTABLE), \
    CMD(WRITETABLE), \
    CMD(CLN), \
    CMD(SINCOSH),\
    CMD(CATANH),\
    CMD(CSQRT), \
    CMD(MPDPOW)


// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
/*#define CMD_EXTRANAME \
    "VOID",

#define CMD_EXTRAENUM \
    VOID
*/

// INTERNAL DECLARATIONS


// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST /*, CMD_EXTRAENUM */, LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
const char * const LIB_NAMES[]= { CMD_LIST /*, CMD_EXTRANAME*/ };
#undef CMD


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
        return;
    }

    switch(OPCODE(CurOpcode))
    {
/*
    case TRANSCENTABLE:
        // TERMPORARY USE ONLY, CONVERT A REAL AS A 'C' STRING OF 32-BIT WORDS
        // FORMAT IS: EXPONENT, NUM.WORDS, MANTISA0 , ... ALL MANTISSA WORDS.
    {
        REAL dec;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadReal(rplPeekData(1),&dec);

#define TABLES_OUTPUT_PRECISION 2016

        // FOR THE TABLES, WE'LL ROUND TO THE NEAREST INTEGER, DISCARDING THE LAST 9 DIGITS (1 WORD)
        int exponent=dec.digits+dec.exp;
        dec.exp=TABLES_OUTPUT_PRECISION-dec.digits;
        CONTEXT newctx;
        memcpy(&newctx,&Context,sizeof(CONTEXT));

        newctx.round=MPD_ROUND_HALF_UP;
        mpd_round_to_intx(&RReg[0],&dec,&newctx);
        RReg[0].exp=-TABLES_OUTPUT_PRECISION+exponent;


        if(Exceptions) return;
        BINT strsize = (RReg[0].len + 3)*12/4;   // THERE'S 9 DIGITS PER WORD, PLUS A COMMA AND A SPACE = 11 DIGITS + 1 EXTRA JUST IN CASE = 12 BYTES = 3 WORDS
        WORDPTR string=rplAllocTempOb(strsize);
        if(!string) return;
        BYTEPTR strptr=(BYTEPTR) &(string[1]);
*/
//        BINT charswritten=sprintf((char *)strptr,"%d, /* %d WORDS */ ",RReg[0].exp,RReg[0].len);
/*
        strptr+=charswritten;
        int count;
        for(count=0;count<RReg[0].len;++count)
        {
            charswritten=sprintf((char *)strptr,"%d, ",RReg[0].data[count]);
            strptr+=charswritten;
        }

        charswritten=sprintf((char *)strptr,"\n");
        strptr+=charswritten;

        // DETERMINE TOTAL NUMBER OF BYTES WRITTEN
        charswritten=strptr-(BYTEPTR) &(string[1]);

        WORDPTR strend=(WORDPTR)  ((((WORD)strptr)+3)&~3);

        string[0]=MKPROLOG(DOSTRING | ((4-(charswritten&3))&3), strend-string-1);

        // TRUNCATE OBJECT AT END OF TEMPOB
        rplTruncateLastObject(strend);

        rplOverwriteData(1,string);
        return;

    }
    case WRITETABLE:
    {
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }

        WORDPTR string=rplPeekData(1);

        if(!ISSTRING(*string)) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        // APPEND THE STRING TO A FILE NAMED OUTPUT.TXT
        FILE *han=fopen("output.txt","a");
        if(!han) return;
        BINT nwords;
        BYTEPTR charptr;
        // NOW PRINT THE STRING OBJECT
        nwords=OBJSIZE(*string);
        charptr=(BYTEPTR) (string+1);
        for(;nwords>1;--nwords,charptr+=4)
        {
            fprintf(han,"%c%c%c%c",charptr[0],charptr[1],charptr[2],charptr[3]);
        }
        // LAST WORD MAY CONTAIN LESS THAN 4 CHARACTERS
        nwords=4-(LIBNUM(*string)&3);
        for(;nwords>0;--nwords,charptr++)
        {
            fprintf(han,"%c",*charptr);
        }

        fclose(han);

        rplDropData(1);
        return;
        }
        */
    case CLN:
    {
        REAL x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        hyp_ln(&x);
        if(Exceptions) return;

/*
        RReg[0].exp+=Context.prec;
        mpd_round_to_intx(&RReg[7],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[7].exp-=Context.prec;
        mpd_reduce(&RReg[0],&RReg[7],&Context);
*/
        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }
    case SINCOSH:
    {
        REAL x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        hyp_sinhcosh(&x);
        if(Exceptions) return;

/*
        RReg[0].exp+=Context.prec;
        mpd_round_to_intx(&RReg[7],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[7].exp-=Context.prec;
        mpd_reduce(&RReg[0],&RReg[7],&Context);
*/
        rplDropData(1);
        rplNewRealFromRRegPush(2);
        rplNewRealFromRRegPush(1);
        return;

    }

    case CATANH:
    {
        REAL x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        hyp_atanh(&x);
        if(Exceptions) return;

/*
        RReg[0].exp+=Context.prec;
        mpd_round_to_intx(&RReg[7],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[7].exp-=Context.prec;
        mpd_reduce(&RReg[0],&RReg[7],&Context);
*/
        rplDropData(1);
        rplNewRealFromRRegPush(2);
        rplNewRealFromRRegPush(1);
        rplNewRealFromRRegPush(0);
        return;

    }

    case CSQRT:
    {
        REAL x;
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        rplReadNumberAsReal(rplPeekData(1),&x);

        if(Exceptions) return;

        hyp_sqrt(&x);
        if(Exceptions) return;

/*
        RReg[0].exp+=Context.prec;
        mpd_round_to_intx(&RReg[7],&RReg[0],&Context);  // ROUND TO THE REQUESTED PRECISION
        RReg[7].exp-=Context.prec;
        mpd_reduce(&RReg[0],&RReg[7],&Context);
*/
        rplDropData(1);
        rplNewRealFromRRegPush(0);
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
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;


}





