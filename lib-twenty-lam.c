/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// THIS LIBRARY PROVIDES ONLY COMPILATION OF IDENTS AFTER ALL OTHER LIBRARIES
// HAD A CHANCE TO IDENTIFY THEIR COMMANDS
// ANY LAM COMMANDS HAVE TO BE IN A SEPARATE LIBRARY



// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  20
#define LIB_ENUM lib20enum
#define LIB_NAMES lib20_names
#define LIB_HANDLER lib20_handler
#define LIB_NUMBEROFCMDS LIB20_NUMBEROFCMDS

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(LSTO), \
    CMD(LRCL), \
    CMD(ABND), \
    CMD(NULLLAM)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
#define CMD_EXTRANAME \
    "->"
#define CMD_EXTRAENUM \
    NEWLOCALENV


// THESE ARE SPECIAL OPCODES FOR THE COMPILER ONLY
// THE LOWER 16 BITS ARE THE NUMBER OF LAMS TO CREATE, OR THE INDEX OF LAM NUMBER TO STO/RCL
#define NEWNLOCALS 0x40000   // SPECIAL OPCODE TO CREATE NEW LOCAL VARIABLES
#define GETLAMN    0x20000   // SPECIAL OPCODE TO RCL THE CONTENT OF A LAM
#define PUTLAMN    0x10000   // SPECIAL OPCODE TO STO THE CONTENT OF A LAM

// INTERNAL DECLARATIONS

// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST , CMD_EXTRAENUM , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
char *LIB_NAMES[]= { CMD_LIST , CMD_EXTRANAME  };
#undef CMD




// INTERNAL RPL PROGRAM THAT CALLS ABND
WORD abnd_prog[]=
{
    (WORD)MKOPCODE(LIBRARY_NUMBER,ABND),  // JUST A WORD THAT WILL BE SKIPPED BY THE COMPILER
    (WORD)MKOPCODE(LIBRARY_NUMBER,ABND)   // THIS IS THE WORD THAT WILL BE EXECUTED
    // SEMI NOT NEEDED SINCE ABND ALREADY DOES SEMI
};

// INTERNAL SINT OBJECTS
WORD lam_baseseco_bint[]=
{
    (WORD)LAM_BASESECO
};

// INTERNAL SINT OBJECTS
WORD lam_errhandler_bint[]=
{
    (WORD)LAM_ERRHANDLER
};

// INTERNAL NULLLAM IDENT OBJECTS
WORD nulllam_ident[]=
{
    (WORD)MKOPCODE(LIBRARY_NUMBER,NULLLAM)
};



extern BINT64 powersof10[20];





void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        // NORMAL BEHAVIOR  ON A IDENT IS TO PUSH THE OBJECT ON THE STACK:
        rplPushData(IPtr);

        if(LIBNUM(CurOpcode)==LIBRARY_NUMBER+1) {
            // UNQUOTED LAM, NEED TO ALSO DO XEQ ON ITS CONTENTS
            {
                WORDPTR val=rplGetLAM(rplPeekData(1));
                if(!val) {
                    val=rplGetGlobal(rplPeekData(1));
                    if(!val) {
                        // INEXISTENT IDENT EVALS TO ITSELF, SO RETURN DIRECTLY
                        return;
                    }
                }
                rplOverwriteData(1,val);    // REPLACE THE FIRST LEVEL WITH THE VALUE
                LIBHANDLER han=rplGetLibHandler(LIBNUM(*val));  // AND EVAL THE OBJECT
                if(han) {
                    BINT SavedOpcode=CurOpcode;
                    CurOpcode=MKOPCODE(LIB_OVERLOADABLE,OVR_XEQ);
                    // EXECUTE THE OTHER LIBRARY DIRECTLY
                    (*han)();
                    // RESTORE THE PREVIOUS ONE ONLY IF THE HANDLER DID NOT CHANGE IT
                    if(CurOpcode==MKOPCODE(LIB_OVERLOADABLE,OVR_XEQ)) CurOpcode=SavedOpcode;
                }
                else {
                    // THE LIBRARY DOESN'T EXIST BUT THE OBJECT DOES?
                    // THIS CAN ONLY HAPPEN IF TRYING TO EXECUTE WITH A CUSTOM OBJECT
                    // WHOSE LIBRARY WAS UNINSTALLED AFTER BEING COMPILED (IT'S AN INVALID OBJECT)
                    Exceptions=EX_BADARGTYPE;
                    ExceptionPointer=IPtr;
                }

            return;
            }
        }
        else return;
    }

    if(OPCODE(CurOpcode)&0x70000) {
        // IT'S ONE OF THE COMPACT OPCODES
        BINT op=OPCODE(CurOpcode)>>16;
        BINT num=OPCODE(CurOpcode)&0xffff;
        if(num&0x8000) num|=0xFFFF0000; // GET NEGATIVE LAMS TOO!

        switch(op)
        {
        case 1: // PUTLAMn
        {
            if(rplDepthData()<1) {
                Exceptions=EX_BADARGCOUNT;
                ExceptionPointer=IPtr;
                return;
            }
            WORDPTR *local=rplGetLAMn(num);
            *local=rplPopData();
            return;
         }
        case 2: // GETLAMn
            rplPushData(*rplGetLAMn(num));
            return;

        case 4: // NEWNLOCALS
            // THIS ONE HAS TO CREATE 'N' LOCALS TAKING THE NAMES AND OBJECTS FROM THE STACK
            // AND ALSO HAS TO 'EVAL' THE NEXT OBJECT IN THE RUNSTREAM
            // THE STACK CONTAINS VAL1 VAL2 ... VALN LAM1 LAM2 ... LAMN
        {
            if(rplDepthData()<2*num) {
                Exceptions=EX_BADARGCOUNT;
                ExceptionPointer=IPtr;
                return;
            }

            // CHECK ALL ARGUMENTS
            BINT cnt=num;
            while(cnt) {
                if(!ISIDENT(*rplPeekData(cnt))) {
                        Exceptions=EX_BADARGTYPE;
                        ExceptionPointer=IPtr;
                        return;
                }
                --cnt;
            }

            // CREATE A NEW LAM ENVIRONMENT FOR THIS SECONDARY
            nLAMBase=LAMTop;    // POINT THE GETLAM BASE TO THIS SECONDARY'S LAMS
            rplCreateLAM(lam_baseseco_bint,rplPeekRet(1));  // PUT MARKER IN LAM STACK
            rplPushRet(abnd_prog);                          // PUT ABND IN THE STACK TO DO THE CLEANUP
            BINT offset=num;
            // NOW CREATE ALL LOCAL VARIABLES
            while(num) {
                rplCreateLAM(rplPeekData(num),rplPeekData(num+offset));
                --num;
            }
            // CLEAN THE STACK
            rplDropData(2*offset);


         }
            return;
        }
    }

    switch(OPCODE(CurOpcode))
    {
    case LSTO:
    {
        // STORE CONTENT INSIDE A LAM VARIABLE, CREATE A NEW VARIABLE IF NEEDED
        if(rplDepthData()<2) {
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            Exceptions=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }


        // FIND LOCAL VARIABLE IN THE CURRENT SCOPE ONLY
        WORDPTR *val=rplFindLAM(rplPeekData(1),0);
        BINT neednewenv=rplNeedNewLAMEnv();

        if(val && !neednewenv) {
            val[1]=rplPeekData(2);
            rplDropData(2);
        }
        else {
            // LAM WAS NOT FOUND, CREATE A NEW ONE
            if(neednewenv) {
                // A NEW LAM ENVIRONMENT NEEDS TO BE CREATED
                nLAMBase=LAMTop;
                rplCreateLAM(lam_baseseco_bint,rplPeekRet(1));
                // AND PUSH THE AUTOMATIC CLEANUP ROUTINE
                rplPushRet(abnd_prog);
            }
                // CREATE THE NEW VARIABLE WITHIN THE CURRENT ENVIRONMENT
                rplCreateLAM(rplPeekData(1),rplPeekData(2));
                rplDropData(2);
            }
    }
    return;
    case LRCL:
    {
        // RCL CONTENT FROM INSIDE A LAM VARIABLE
        if(rplDepthData()<1) {
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            Exceptions=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;

        }

        WORDPTR val=rplGetLAM(rplPeekData(1));
        if(val) {
            rplOverwriteData(1,val);
        }
        else {
            Exceptions=EX_VARUNDEF;
            ExceptionPointer=IPtr;
            return;
        }
    }
        return;
    case ABND:
        // CLEANUP ALL LAMS AND DO SEMI
        {

            if(*(nLAMBase+1)==rplPeekRet(1)) {
                // THIS WILL BE TRUE 99.9% OF THE TIMES
               LAMTop=nLAMBase;
            }
            else {
                // THERE'S SOME OTHER LAM CONSTRUCT OR CORRUPTED MARKERS, SEARCH FOR THE CORRECT MARKER
            WORDPTR *val=LAMTop;
            while( (val=rplGetNextLAMEnv(val)) )
            {
            // PURGE ALL LAMS AFTER THE MARKER
            if(*(val+1)==rplPeekRet(1)) { LAMTop=val; break; }
            }
            // SOMETHING BAD HAPPENED, THIS SECONDARY HAD NO LAM ENVIRONMENT BUT AN ABND WORD!
            if(!val) LAMTop=LAMs;
            }

            nLAMBase=rplGetNextLAMEnv(LAMTop);
            if(!nLAMBase) nLAMBase=LAMs;

            IPtr=rplPopRet();   // GET THE CALLER ADDRESS
            CurOpcode=*IPtr;    // SET THE WORD SO MAIN LOOP SKIPS THIS OBJECT, AND THE NEXT ONE IS EXECUTED
        }
            return;


    case NEWLOCALENV:
    // THIS COMMAND DOES NOTHING, IT'S JUST A MARKER FOR THE COMPILER

        return;


    // ADD MORE OPCODES HERE

    case OVR_EVAL:
    // RCL WHATEVER IS STORED IN THE LAM AND THEN XEQ ITS CONTENTS
    // NO ARGUMENT CHECKS! THAT SHOULD'VE BEEN DONE BY THE OVERLOADED "EVAL" DISPATCHER
    {
        WORDPTR val=rplGetLAM(rplPeekData(1));
        if(!val) {
            val=rplGetGlobal(rplPeekData(1));
            if(!val) {
                // INEXISTENT IDENT EVALS TO ITSELF, SO RETURN DIRECTLY
                return;
            }
        }
        rplOverwriteData(1,val);    // REPLACE THE FIRST LEVEL WITH THE VALUE
        CurOpcode=MKOPCODE(LIB_OVERLOADABLE,OVR_XEQ);
        LIBHANDLER han=rplGetLibHandler(LIBNUM(*val));  // AND EVAL THE OBJECT
        if(han) {
            // EXECUTE THE OTHER LIBRARY DIRECTLY
            (*han)();
        }
        else {
            // THE LIBRARY DOESN'T EXIST BUT THE OBJECT DOES?
            // THIS CAN ONLY HAPPEN IF TRYING TO EXECUTE WITH A CUSTOM OBJECT
            // WHOSE LIBRARY WAS UNINSTALLED AFTER BEING COMPILED (IT'S AN INVALID OBJECT)
            Exceptions=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            CurOpcode=*IPtr;
        }


    }
        return;

    case OVR_XEQ:
        // JUST KEEP THE IDENT ON THE STACK, UNEVALUATED
       return;





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


        // LSTO NEEDS SPECIAL CONSIDERATION TO CREATE LAMS AT COMPILE TIME

        if((TokenLen==4) && (!strncmp((char *)TokenStart,"LSTO",4)))
        {

            // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

            // CHECK IF THE PREVIOUS OBJECT IS A QUOTED IDENT?
            WORDPTR object,prevobject;
            if(ValidateTop<=RSTop) {
                // THERE'S NO ENVIRONMENT
                object=TempObEnd;   // START OF COMPILATION
            } else {
                object=*(ValidateTop-1);    // GET LATEST CONSTRUCT
                ++object;                   // AND SKIP THE PROLOG / ENTRY WORD
            }

            if(object<CompileEnd) {
            do {
                prevobject=object;
                object=rplSkipOb(object);
            } while(object<CompileEnd);

            // HERE PREVOBJECT CONTAINS THE LAST OBJECT THAT WAS COMPILED

            if(ISIDENT(*prevobject)) {
                // WE HAVE A HARD-CODED IDENT, CHECK IF IT EXISTS ALREADY

                // CHECK IF IT'S AN EXISTING LAM, COMPILE TO A PUTLAM OPCODE IF POSSIBLE

                WORDPTR *LAMptr=rplFindLAM(prevobject,1);


                if(LAMptr<LAMTopSaved) {
                    // THIS IS NOT A VALID LAM, LEAVE AS IDENT

                    rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LSTO));

                    // TRACK LAM CREATION IN THE CURRENT ENVIRONMENT

                    // DO WE NEED A NEW ENVIRONMENT?

                    if(rplNeedNewLAMEnvCompiler()) {    // CREATE A NEW ENVIRONMENT IF NEEDED
                     nLAMBase=LAMTop;
                     rplCreateLAM(lam_baseseco_bint,*(ValidateTop-1));
                    }
                    rplCreateLAM(prevobject,prevobject);
                    RetNum=OK_CONTINUE;
                    return;
                }

                if(LAMptr<nLAMBase) {
                    // THIS IS A LAM FROM AN UPPER CONSTRUCT
                    // WE CAN USE PUTLAM ONLY INSIDE LOOPS, NEVER ACROSS SECONDARIES

                    WORDPTR *env=nLAMBase;
                    WORD prolog;
                    do {
                        if(LAMptr>env) break;
                        prolog=**(env+1);   // GET THE PROLOG OF THE SECONDARY
                        if(ISPROLOG(prolog) && LIBNUM(prolog)==SECO) {
                        // LAMS ACROSS << >> SECONDARIES HAVE TO BE COMPILED AS IDENTS
                        rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LSTO));

                        RetNum=OK_CONTINUE;
                        return;
                        }
                        env=rplGetNextLAMEnv(env);
                    } while(env);



                }


                // SPECIAL CASE: WHEN A SECO DOESN'T HAVE ANY LOCALS YET
                // BUT LAMS FROM THE PREVIOUS SECO SHOULDN'T BE COMPILED TO GETLAMS

                // SCAN ALL CURRENT CONSTRUCTS TO FIND THE INNERMOST SECONDARY
                // THEN VERIFY IF THAT SECONDARY IS THE CURRENT LAM ENVIRONMENT

                // THIS IS TO FORCE ALL LAMS IN A SECO TO BE COMPILED AS IDENTS
                // INSTEAD OF PUTLAMS

                // LAMS ACROSS DOCOL'S ARE OK AND ALWAYS COMPILED AS PUTLAMS
                WORDPTR *scanenv=ValidateTop-1;

                while(scanenv>=RSTop) {
                    if( (LIBNUM(**scanenv)==SECO)&& (ISPROLOG(**scanenv))) {
                            // FOUND INNERMOST SECONDARY
                            if(*scanenv>*(nLAMBase+1)) {
                                // THE CURRENT LAM BASE IS OUTSIDE THE INNER SECONDARY
                            rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LSTO));
                            if(rplNeedNewLAMEnvCompiler()) {    // CREATE A NEW ENVIRONMENT IF NEEDED
                             nLAMBase=LAMTop;
                             rplCreateLAM(lam_baseseco_bint,*(ValidateTop-1));
                            }
                            rplCreateLAM(prevobject,prevobject);


                            RetNum=OK_CONTINUE;
                            return;
                            }
                            break;

                    }
                    --scanenv;
                }

                // IT'S A KNOWN LOCAL VARIABLE, COMPILE AS PUTLAM
                CompileEnd=prevobject;
                BINT Offset=((BINT)(LAMptr-nLAMBase))>>1;
                rplCompileAppend(MKOPCODE(DOIDENT,PUTLAMN+(Offset&0xffff)));
                RetNum=OK_CONTINUE;
                return;
            }


            }


            rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LSTO));
            RetNum=OK_CONTINUE;
            return;
        }




            // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
            // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER,LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors
        if(ISPROLOG(*DecompileObject)) {

            if(LIBNUM(*DecompileObject)==LIBRARY_NUMBER)
                // THIS IS A QUOTED IDENT
                rplDecompAppendChar('\'');

            BYTEPTR ptr=(BYTEPTR)(DecompileObject+OBJSIZE(*DecompileObject));
            if(ptr[3]==0)
                // WE HAVE A NULL-TERMINATED STRING, SO WE CAN USE THE STANDARD FUNCTION
                rplDecompAppendString((BYTEPTR) (DecompileObject+1));
            else
                rplDecompAppendString2((BYTEPTR)(DecompileObject+1),OBJSIZE(*DecompileObject)<<2);

            if(LIBNUM(*DecompileObject)==LIBRARY_NUMBER)
                // THIS IS A QUOTED IDENT
                rplDecompAppendChar('\'');

            RetNum=OK_CONTINUE;
            return;

        }

        switch(OPCODE(*DecompileObject)&0x70000)
        {
        case NEWNLOCALS:
        {
            rplDecompAppendString((BYTEPTR)"NEWLOCALS");
            BINT result=OPCODE(*DecompileObject)&0xffff;
            BINT digit=0;
            char basechr='0';
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
            RetNum=OK_CONTINUE;
            return;
        case GETLAMN:
        {
            rplDecompAppendString((BYTEPTR)"GETLAM");
            BINT result=OPCODE(*DecompileObject)&0xffff;
            if(result&0x8000) result|=0xFFFF0000;
            if(result<0) {
                rplDecompAppendChar('u');
                result=-result;
            }

            BINT digit=0;
            char basechr='0';
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
            RetNum=OK_CONTINUE;
            return;
        case PUTLAMN:
        {
            rplDecompAppendString((BYTEPTR)"PUTLAM");
            BINT result=OPCODE(*DecompileObject)&0xffff;
            if(result&0x8000) result|=0xFFFF0000;
            if(result<0) {
                rplDecompAppendChar('u');
                result=-result;
            }

            BINT digit=0;
            char basechr='0';
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
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart =
        // BlanksLen = Opcode/WORD of object

        // VALIDATE RETURNS:
        // RetNum =  enum CompileErrors



        return;
    }

    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;


}






