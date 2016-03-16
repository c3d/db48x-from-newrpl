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
#define LIBRARY_NUMBER  9


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(IF,MKTOKENINFO(2,TITYPE_NOTALLOWED,1,2)), \
    CMD(THEN,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(ELSE,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(ENDIF,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)),   \
    CMD(CASE,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(THENCASE,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(ENDTHEN,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(ENDCASE,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(FOR,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(START,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(NEXT,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(STEP,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(DO,MKTOKENINFO(2,TITYPE_NOTALLOWED,1,2)), \
    CMD(UNTIL,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(ENDDO,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(WHILE,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(REPEAT,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(ENDWHILE,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(IFERR,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(THENERR,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(ELSEERR,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(ENDERR,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    ECMD(QSEMI,"»",MKTOKENINFO(1,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE
#define ERROR_LIST \
    ERR(PROGRAMEXPECTED,0)

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib9_menu);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
     (WORDPTR)LIB_MSGTABLE,
     (WORDPTR)LIB_HELPTABLE,
     (WORDPTR)lib9_menu
};



void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        // NORMAL BEHAVIOR IS TO PUSH THE OBJECT ON THE STACK:

        rplPushData(IPtr);
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case QSEMI:
        // POP THE RETURN ADDRESS
        IPtr=rplPopRet();   // GET THE CALLER ADDRESS
        CurOpcode=*IPtr;    // SET THE WORD SO MAIN LOOP SKIPS THIS OBJECT, AND THE NEXT ONE IS EXECUTED
        return;


    // ADD MORE OPCODES HERE

    case OVR_EVAL:
    case OVR_EVAL1:
    case OVR_XEQ:
    // EXECUTE THE SECONDARY THAT'S ON THE STACK
        if(ISPROLOG(*rplPeekData(1))) {
        rplPushRet(IPtr);       // PUSH CURRENT POINTER AS THE RETURN ADDRESS. AT THIS POINT, IPtr IS POINTING TO THIS SECONDARY WORD
                                // BUT THE MAIN LOOP WILL ALWAYS SKIP TO THE NEXT OBJECT AFTER A SEMI.
        IPtr=rplPopData();
        CurOpcode=MKPROLOG(LIBRARY_NUMBER,0); // ALTER THE SIZE OF THE SECONDARY TO ZERO WORDS, SO THE NEXT EXECUTED INSTRUCTION WILL BE THE FIRST IN THIS SECONDARY
        return;
        }

     // NOT A SECONDARY, IT HAS TO BE A COMMAND
     // DO NOTHING
     rplDropData(1);

    return;

    case IF:
    // THIS COMMAND DOES NOTHING, IT'S JUST A MARKER FOR THE COMPILER
        return;

    case THEN:
        {
        // BY DEFINITION, BINT 0 OR REAL 0.0 = FALSE, EVERYTHING ELSE IS TRUE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
            // EXTRACT THE OBJECT INTO A GC-SAFE POINTER
            ScratchPointer1=rplPopData();

            if(IS_FALSE(*ScratchPointer1)) {
                // SKIP ALL OBJECTS UNTIL NEXT ELSE OR END
                int count=0;
                while( (count!=0) || ((*IPtr!=MKOPCODE(LIBRARY_NUMBER,ELSE))&&(*IPtr!=MKOPCODE(LIBRARY_NUMBER,ENDIF)))) {
                    if(*IPtr==MKOPCODE(LIBRARY_NUMBER,IF)) ++count;
                    if(*IPtr==MKOPCODE(LIBRARY_NUMBER,ENDIF)) --count;
                    if(*IPtr==MKOPCODE(LIBRARY_NUMBER,QSEMI)) { --IPtr; return; }  // MALFORMED, JUST EXIT AT THE SEMI
                    if(*IPtr==CMD_SEMI) { --IPtr; return; }   // MALFORMED, JUST EXIT AT THE SEMI
                    rplSkipNext();
                }
            }
       }
        return;
    case ELSE:
        // SKIP ALL OBJECTS UNTIL NEXT END
        {
            int count=0;
        while(count || (*IPtr!=MKOPCODE(LIBRARY_NUMBER,ENDIF))) {
            if(*IPtr==MKOPCODE(LIBRARY_NUMBER,IF)) ++count;
            if(*IPtr==MKOPCODE(LIBRARY_NUMBER,ENDIF)) --count;
            if(*IPtr==MKOPCODE(LIBRARY_NUMBER,QSEMI)) { --IPtr; return; }   // MALFORMED, JUST EXIT AT THE SEMI
            if(*IPtr==CMD_SEMI) { --IPtr; return; }   // MALFORMED, JUST EXIT AT THE SEMI
            rplSkipNext();
        }
        }
        return;
    case ENDIF:
        return;

    case CASE:
        return;

    case THENCASE:
        {
        // BY DEFINITION, BINT 0 OR REAL 0.0 = FALSE, EVERYTHING ELSE IS TRUE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
            // EXTRACT OBJECT INTO A GC-SAFE POINTER
            ScratchPointer1=rplPopData();

            if(IS_FALSE(*ScratchPointer1)) {
                // SKIP ALL OBJECTS UNTIL ENDTHEN
                while(*IPtr!=MKOPCODE(LIBRARY_NUMBER,ENDTHEN)) {
                    if(*IPtr==MKOPCODE(LIBRARY_NUMBER,QSEMI)) { --IPtr; return; }   // MALFORMED, JUST EXIT AT THE SEMI
                    if(*IPtr==CMD_SEMI) { --IPtr; return; }   // MALFORMED, JUST EXIT AT THE SEMI
                    rplSkipNext();
                }
            }
        }
        return;
    case ENDTHEN:
        // IF THIS GETS EXECUTED, IT'S BECAUSE THE THEN CLAUSE WAS TRUE, SO SKIP UNTIL ENDCASE
        while(*IPtr!=MKOPCODE(LIBRARY_NUMBER,ENDCASE)) {
            if(*IPtr==MKOPCODE(LIBRARY_NUMBER,QSEMI)) { --IPtr; return; }   // MALFORMED, JUST EXIT AT THE SEMI
            if(*IPtr==CMD_SEMI) { --IPtr; return; }   // MALFORMED, JUST EXIT AT THE SEMI
            rplSkipNext();
        }
        return;

    case ENDCASE:
        return;

    case FOR:
    {
        // DEFINE 3 LAMS, THE FIRST WILL BE THE LOW LIMIT, THEN THE HIGH LIMIT, THEN THE ITERATOR
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // FIND OUT THE DIRECTION OF THE LOOP

        // MAKE 2DUP
        ScratchPointer3=(WORDPTR)DSTop;
        rplPushData(rplPeekData(2));
        rplPushData(rplPeekData(2));
        rplCallOvrOperator(OVR_GT);

        if(Exceptions) { DSTop=(WORDPTR *)ScratchPointer3; return; }

        ScratchPointer3=IPtr;       // THIS IS POINTING AT THE FOR STATEMENT
        BINT depth=1;               // TRACK NESTED FOR LOOPS
        while( depth || ((*ScratchPointer3!=MKOPCODE(LIBRARY_NUMBER,NEXT)) && (*ScratchPointer3!=MKOPCODE(LIBRARY_NUMBER,STEP)))) {
            ScratchPointer3=rplSkipOb(ScratchPointer3);
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,FOR)) ++depth;
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,START)) ++depth;
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,NEXT)) --depth;
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,STEP)) --depth;
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,QSEMI)) { rplDropData(1); IPtr=ScratchPointer3-1; return; }  // MALFORMED, JUST EXIT AT THE SEMI
            if(*ScratchPointer3==CMD_SEMI) { rplDropData(1); IPtr=ScratchPointer3-1; return; }   // MALFORMED, JUST EXIT AT THE SEMI

        }

        // CREATE A NEW LAM ENVIRONMENT FOR THIS FOR CONSTRUCT
        rplCreateLAMEnvironment(ScratchPointer3);
        rplPushRet(ScratchPointer3);                    // PUT THE RETURN ADDRESS AT THE END OF THE LOOP
        rplPushRet((WORDPTR)abnd_prog);                          // PUT ABND IN THE STACK TO DO THE CLEANUP
        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(3));
        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(2));
        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));
        rplCreateLAM(IPtr+1,rplPeekData(3));            // LAM COUNTER INITIALIZED WITH THE STARTING VALUE

        // CLEAN THE STACK
        rplDropData(3);

        ++IPtr;
        CurOpcode=*IPtr;                              // ADVANCE THE POINTER TO THE IDENT TO BE SKIPPED
        rplPushRet(IPtr);                             // PUT THE LOOP CLAUSE IN THE STACK TO DO THE LOOP

        return;
     }
    case START:
    {
        // DEFINE 3 LAMS, THE FIRST WILL BE THE LOW LIMIT, THEN THE HIGH LIMIT, THEN THE ITERATOR
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // MAKE 2DUP
        ScratchPointer3=(WORDPTR)DSTop;
        rplPushData(rplPeekData(2));
        rplPushData(rplPeekData(2));
        rplCallOvrOperator(OVR_GT);

        if(Exceptions) { DSTop=(WORDPTR *)ScratchPointer3; return; }

        ScratchPointer3=IPtr;       // THIS IS POINTING AT THE FOR STATEMENT
        BINT depth=1;               // TRACK NESTED FOR LOOPS
        while( depth || ((*ScratchPointer3!=MKOPCODE(LIBRARY_NUMBER,NEXT)) && (*ScratchPointer3!=MKOPCODE(LIBRARY_NUMBER,STEP)))) {
            ScratchPointer3=rplSkipOb(ScratchPointer3);
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,FOR)) ++depth;
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,START)) ++depth;
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,NEXT)) --depth;
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,STEP)) --depth;
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,QSEMI)) { rplDropData(1); IPtr=ScratchPointer3-1; return; }  // MALFORMED, JUST EXIT AT THE SEMI
            if(*ScratchPointer3==CMD_SEMI) { rplDropData(1); IPtr=ScratchPointer3-1; return; }   // MALFORMED, JUST EXIT AT THE SEMI

        }

        // CREATE A NEW LAM ENVIRONMENT FOR THIS FOR CONSTRUCT
        rplCreateLAMEnvironment(ScratchPointer3);
        rplPushRet(ScratchPointer3);                    // PUT THE RETURN ADDRESS AT THE END OF THE LOOP
        rplPushRet((WORDPTR)abnd_prog);                          // PUT ABND IN THE STACK TO DO THE CLEANUP
        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(3));
        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(2));
        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));
        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(3));            // LAM COUNTER INITIALIZED WITH THE STARTING VALUE
        // CLEAN THE STACK
        rplDropData(3);

        rplPushRet(IPtr);                             // PUT THE LOOP CLAUSE IN THE STACK TO DO THE LOOP

        return;
     }



    case NEXT:

    {
        // INCREMENT THE COUNTER
        if(*rplGetLAMn(0)!=IPtr) {
            // MALFORMED FOR/NEXT LOOP
            return;
        }
        rplPushData(*rplGetLAMn(4));     // COUNTER;
        rplPushData((WORDPTR)one_bint);       // PUSH THE NUMBER ONE

        // CALL THE OVERLOADED OPERATOR '+'

        rplCallOvrOperator(OVR_ADD);

        if(Exceptions) return;

        WORDPTR *counter=rplGetLAMn(4);

        *counter=rplPopData();      // STORE THE INCREMENTED COUNTER


        // CHECK IF COUNTER IS LESS THAN OR EQUAL THAN LIMIT
        rplPushData(*counter);     // COUNTER
        rplPushData(*rplGetLAMn(2));      // HIGHER LIMIT

        // CHECK IF COUNTER IS LESS THAN LIMIT
        // BY CALLING THE OVERLOADED OPERATOR <= (LTE) OR >= GTE

        if(IS_FALSE(**rplGetLAMn(3))) rplCallOvrOperator(OVR_LTE);
        else rplCallOvrOperator(OVR_GTE);

        WORDPTR result=rplPopData();

        if(IS_FALSE(*result)) {
            // EXIT THE LOOP BY DROPPING THE RETURN STACK
            rplPopRet();
        }
        else rplPushRet(rplPeekRet(1));     // RDUP TO CONTINUE THE LOOP

        // JUMP TO THE TOP RETURN STACK, EITHER THE LOOP OR THE ABND WORD
        IPtr=rplPopRet();
        CurOpcode=*IPtr;
     return;
    }

    case STEP:

    {
        // INCREMENT THE COUNTER
        // THE NUMBER TO ADD IS EXPECTED TO BE ON THE STACK
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // INCREMENT THE COUNTER
        if(*rplGetLAMn(0)!=IPtr) {
            // MALFORMED FOR/NEXT LOOP
            return;
        }
        rplPushData(*rplGetLAMn(4));     // COUNTER;

        // CALL THE OVERLOADED OPERATOR '+'

        rplCallOvrOperator(OVR_ADD);

        if(Exceptions) return;

        WORDPTR *counter=rplGetLAMn(4);

        *counter=rplPopData();      // STORE THE INCREMENTED COUNTER


        // CHECK IF COUNTER IS LESS THAN OR EQUAL THAN LIMIT
        rplPushData(*counter);     // COUNTER
        rplPushData(*rplGetLAMn(2));      // HIGHER LIMIT

        // CHECK IF COUNTER IS LESS THAN LIMIT
        // BY CALLING THE OVERLOADED OPERATOR <= (LTE)

        if(IS_FALSE(**rplGetLAMn(3))) rplCallOvrOperator(OVR_LTE);
        else rplCallOvrOperator(OVR_GTE);

        WORDPTR result=rplPopData();

        if(IS_FALSE(*result)) {
            // EXIT THE LOOP BY DROPPING THE RETURN STACK
            rplPopRet();
        }
        else rplPushRet(rplPeekRet(1));     // RDUP TO CONTINUE THE LOOP

        // JUMP TO THE TOP RETURN STACK, EITHER THE LOOP OR THE ABND WORD
        IPtr=rplPopRet();
        CurOpcode=*IPtr;
     return;
    }


    case DO:
    {
        ScratchPointer3=IPtr;       // THIS IS POINTING AT THE FOR STATEMENT
        BINT depth=1;               // TRACK NESTED LOOPS
        while( depth || (*ScratchPointer3!=MKOPCODE(LIBRARY_NUMBER,ENDDO))) {
            ScratchPointer3=rplSkipOb(ScratchPointer3);
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,DO)) ++depth;
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,ENDDO)) --depth;
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,QSEMI)) { IPtr=ScratchPointer3-1; return; }  // MALFORMED, JUST EXIT AT THE SEMI
            if(*ScratchPointer3==CMD_SEMI) { IPtr=ScratchPointer3-1; return; }   // MALFORMED, JUST EXIT AT THE SEMI

        }

        rplPushRet(ScratchPointer3);                    // PUT THE RETURN ADDRESS AT THE END OF THE LOOP

        // ALWAYS CREATE A TEMPORARY VARIABLE ENVIRONMENT WHEN ENTERING A WHILE LOOP
        rplCreateLAMEnvironment(ScratchPointer3);
        rplPushRet((WORDPTR)abnd_prog);



        rplPushRet(IPtr);                             // PUT THE LOOP CLAUSE IN THE STACK TO DO THE LOOP

        return;
    }
    case UNTIL:
        return;

    case ENDDO:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(*rplGetLAMn(0)!=IPtr) {
            // MALFORMED LOOP
            return;
        }

        // BY DEFINITION, BINT 0 OR REAL 0.0 = FALSE, EVERYTHING ELSE IS TRUE

        WORDPTR result=rplPopData();

        if(!IS_FALSE(*result)) {
            // EXIT THE LOOP BY DROPPING THE RETURN STACK
            rplPopRet();
        }
        else rplPushRet(rplPeekRet(1));     // RDUP TO CONTINUE THE LOOP

        // JUMP TO THE TOP RETURN STACK, EITHER THE LOOP OR THE ABND WORD
        IPtr=rplPopRet();
        CurOpcode=*IPtr;
     return;
    }
    case WHILE:
    {
        ScratchPointer3=IPtr;       // THIS IS POINTING AT THE FOR STATEMENT
        BINT depth=1;               // TRACK NESTED LOOPS
        while( depth || (*ScratchPointer3!=MKOPCODE(LIBRARY_NUMBER,ENDWHILE))) {
            ScratchPointer3=rplSkipOb(ScratchPointer3);
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,WHILE)) ++depth;
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,ENDWHILE)) --depth;
            if(*ScratchPointer3==MKOPCODE(LIBRARY_NUMBER,QSEMI)) { IPtr=ScratchPointer3-1; return; }  // MALFORMED, JUST EXIT AT THE SEMI
            if(*ScratchPointer3==CMD_SEMI) { IPtr=ScratchPointer3-1; return; }   // MALFORMED, JUST EXIT AT THE SEMI

        }

        rplPushRet(ScratchPointer3);                    // PUT THE RETURN ADDRESS AT THE END OF THE LOOP
        // ALWAYS CREATE A TEMPORARY VARIABLE ENVIRONMENT WHEN ENTERING A WHILE LOOP
        rplCreateLAMEnvironment(ScratchPointer3);
        rplPushRet((WORDPTR)abnd_prog);

        rplPushRet(IPtr);                             // PUT THE LOOP CLAUSE IN THE STACK TO DO THE LOOP

        return;
    }
    case REPEAT:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // BY DEFINITION, BINT 0 OR REAL 0.0 = FALSE, EVERYTHING ELSE IS TRUE
        if(*rplGetLAMn(0)!=IPtr) {
            // MALFORMED LOOP
            return;
        }

        WORDPTR result=rplPopData();

        if(IS_FALSE(*result)) {
            // EXIT THE LOOP BY DROPPING THE RETURN STACK
            rplPopRet();
            // JUMP TO THE TOP RETURN STACK, EITHER THE LOOP OR THE ABND WORD
            IPtr=rplPopRet();
            CurOpcode=*IPtr;
        }



        return;
    }
    case ENDWHILE:
        // JUMP TO THE TOP RETURN STACK TO REPEAT THE LOOP
        if(rplDepthRet()>0) {
        IPtr=rplPeekRet(1);
        CurOpcode=*IPtr;
        }
     return;

    case IFERR:
        {
        // SETUP AN ERROR TRAP
        WORDPTR errhandler=rplSkipOb(IPtr);     // START AFTER THE IFERR BYTECODE
        // SKIP TO THE NEXT THENERR OPCODE, BUT TAKING INTO ACCOUNT POSSIBLY NESTED IFERR STATEMENTS
        {
            int count=0;
        while(count || (*errhandler!=MKOPCODE(LIBRARY_NUMBER,THENERR))) {
            if(*errhandler==MKOPCODE(LIBRARY_NUMBER,IFERR)) ++count;
            if(*errhandler==MKOPCODE(LIBRARY_NUMBER,ENDERR)) --count;
            if(*errhandler==MKOPCODE(LIBRARY_NUMBER,QSEMI)) { IPtr=errhandler-1; return; }  // MALFORMED, JUST EXIT AT THE SEMI
            if(*errhandler==CMD_SEMI) { IPtr=errhandler-1; return; }  // MALFORMED, JUST EXIT AT THE SEMI

            errhandler=rplSkipOb(errhandler);
        }
        }

        // SET THE EXCEPTION HANDLER AFTER THE THENERR WORD
        rplSetExceptionHandler(errhandler+1);

        // ALL SET, ANYTHING EXECUTED FROM NOW ON WILL BE TRAPPED
        }
        return;
    case THENERR:
        // IF THIS IS EXECUTED, THEN THERE WERE NO ERRORS, REMOVE THE ERROR TRAP AND SKIP TO THE ENDERR MARKER
        rplRemoveExceptionHandler();

        {
            int count=0;
        while(count || ( (*IPtr!=MKOPCODE(LIBRARY_NUMBER,ENDERR)) && (*IPtr!=MKOPCODE(LIBRARY_NUMBER,ELSEERR)))) {
            if(*IPtr==MKOPCODE(LIBRARY_NUMBER,IFERR)) ++count;
            if(*IPtr==MKOPCODE(LIBRARY_NUMBER,ENDERR)) --count;
            if(*IPtr==MKOPCODE(LIBRARY_NUMBER,QSEMI)) { --IPtr; return; }   // MALFORMED, JUST EXIT AT THE SEMI
            if(*IPtr==CMD_SEMI) { --IPtr; return; }   // MALFORMED, JUST EXIT AT THE SEMI
            rplSkipNext();
        }
        }
        return;
    case ELSEERR:
        // THIS WOULD ONLY BE EXECUTED AT THE END OF AN ERROR TRAP. SKIP TO THE ENDERR MARKER
    {
        int count=0;
    while(count ||  (*IPtr!=MKOPCODE(LIBRARY_NUMBER,ENDERR)) ) {
        if(*IPtr==MKOPCODE(LIBRARY_NUMBER,IFERR)) ++count;
        if(*IPtr==MKOPCODE(LIBRARY_NUMBER,ENDERR)) --count;
        if(*IPtr==MKOPCODE(LIBRARY_NUMBER,QSEMI)) { --IPtr; return; }   // MALFORMED, JUST EXIT AT THE SEMI
        if(*IPtr==CMD_SEMI) { --IPtr; return; }   // MALFORMED, JUST EXIT AT THE SEMI
        rplSkipNext();
    }
    }

        return;
    case ENDERR:
        // THIS IS ONLY EXECUTED WHEN EXITING AN ERROR HANDLER
        // THERE'S NOTHING ELSE TO DO
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

        // CHECK IF THE TOKEN IS THE OBJECT DOCOL

       if((TokenLen==1) && (!utf8ncmp((char *)TokenStart,"«",1)))
       {
           rplCompileAppend((WORD) MKPROLOG(LIBRARY_NUMBER,0));
           RetNum=OK_STARTCONSTRUCT;
           return;
       }

       // CHECK IF THE TOKEN IS SEMI

       if((TokenLen==1) && (!utf8ncmp((char *)TokenStart,"»",1)))
       {
           if(CurrentConstruct!=MKPROLOG(LIBRARY_NUMBER,0)) {
               RetNum=ERR_SYNTAX;
               return;
           }
           rplCleanupLAMs(*(ValidateTop-1));
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,QSEMI));
           RetNum=OK_ENDCONSTRUCT;
           return;
       }

       // SPECIAL CONSTRUCTS

       // IF... THEN... ELSE... END
       // IFERR ... THEN ... END
       // AND ALSO CASE ... THEN ... END ... THEN ... END ... END

       if((TokenLen==2) && (!utf8ncmp((char *)TokenStart,"IF",2)))
       {
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,IF));
           RetNum=OK_STARTCONSTRUCT;
           return;
       }

       if((TokenLen==5) && (!utf8ncmp((char *)TokenStart,"IFERR",2)))
       {
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,IFERR));
           RetNum=OK_STARTCONSTRUCT;
           return;
       }


       if((TokenLen==4) && (!utf8ncmp((char *)TokenStart,"CASE",4)))
       {
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,CASE));
           RetNum=OK_STARTCONSTRUCT;
           return;
       }

       if((TokenLen==4) && (!utf8ncmp((char *)TokenStart,"THEN",4)))
       {
           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,IF)) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,THEN));
               RetNum=OK_CHANGECONSTRUCT;
               return;
           }
           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,IFERR)) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,THENERR));
               RetNum=OK_CHANGECONSTRUCT;
               return;
           }

           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,CASE)) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,THENCASE));
               RetNum=OK_STARTCONSTRUCT;
               return;
           }

           RetNum=ERR_SYNTAX;
           return;
       }

       if((TokenLen==7) && (!utf8ncmp((char *)TokenStart,"THENERR",7)))
       {
           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,IFERR)) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,THENERR));
               RetNum=OK_CHANGECONSTRUCT;
               return;
           }

           RetNum=ERR_SYNTAX;
           return;
       }

       if((TokenLen==8) && (!utf8ncmp((char *)TokenStart,"THENCASE",8)))
       {
           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,CASE)) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,THENCASE));
               RetNum=OK_STARTCONSTRUCT;
               return;
           }

           RetNum=ERR_SYNTAX;
           return;
       }


       if((TokenLen==4) && (!utf8ncmp((char *)TokenStart,"ELSE",4)))
       {
           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,THEN)) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ELSE));
               RetNum=OK_CHANGECONSTRUCT;
               return;
           }
           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,THENERR)) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ELSEERR));
               RetNum=OK_CHANGECONSTRUCT;
               return;
           }

           RetNum=ERR_SYNTAX;
           return;

       }

       if((TokenLen==7) && (!utf8ncmp((char *)TokenStart,"ELSEERR",7)))
       {
           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,THENERR)) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ELSEERR));
               RetNum=OK_CHANGECONSTRUCT;
               return;
           }

           RetNum=ERR_SYNTAX;
           return;

       }


       if((TokenLen==3) && (!utf8ncmp((char *)TokenStart,"END",3)))
       {
           // ENDIF OPCODE
           if( (CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,THEN)) || (CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,ELSE))) {
           rplCleanupLAMs(*(ValidateTop-1));
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ENDIF));
           RetNum=OK_ENDCONSTRUCT;
           return;
           }
           // ENDERR
           if( (CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,THENERR)) || (CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,ELSEERR))) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ENDERR));
               RetNum=OK_ENDCONSTRUCT;
               return;
           }

           // ENDTHEN
           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,THENCASE)) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ENDTHEN));
               RetNum=OK_ENDCONSTRUCT;
               return;
           }
           // ENDCASE
           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,CASE)) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ENDCASE));
               RetNum=OK_ENDCONSTRUCT;
               return;
           }

           // ENDDO

           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,UNTIL)) {
               rplCleanupLAMs(0);
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ENDDO));
               RetNum=OK_ENDCONSTRUCT;
               return;
           }

           // ENDWHILE

           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,REPEAT)) {
               rplCleanupLAMs(0);
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ENDWHILE));
               RetNum=OK_ENDCONSTRUCT;
               return;
           }


           rplError(ERR_MISPLACEDEND);
           RetNum=ERR_SYNTAX;
           return;

       }

       // NOW ALL THE DIFFERENT END'S SEPARATED


       if((TokenLen==5) && (!utf8ncmp((char *)TokenStart,"ENDIF",5)))
       {
           // ENDIF OPCODE
           if( (CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,THEN)) || (CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,ELSE))) {
           rplCleanupLAMs(*(ValidateTop-1));
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ENDIF));
           RetNum=OK_ENDCONSTRUCT;
           return;
           }
           rplError(ERR_MISPLACEDEND);
           RetNum=ERR_SYNTAX;
           return;

       }

       if((TokenLen==6) && (!utf8ncmp((char *)TokenStart,"ENDERR",6)))
       {
           // ENDERR
           if( (CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,THENERR)) || (CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,ELSEERR))) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ENDERR));
               RetNum=OK_ENDCONSTRUCT;
               return;
           }

           rplError(ERR_MISPLACEDEND);
           RetNum=ERR_SYNTAX;
           return;

       }

       if((TokenLen==7) && (!utf8ncmp((char *)TokenStart,"ENDTHEN",7)))
       {

           // ENDTHEN
           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,THENCASE)) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ENDTHEN));
               RetNum=OK_ENDCONSTRUCT;
               return;
           }

           rplError(ERR_MISPLACEDEND);
           RetNum=ERR_SYNTAX;
           return;

       }

       if((TokenLen==7) && (!utf8ncmp((char *)TokenStart,"ENDCASE",7)))
       {
           // ENDCASE
           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,CASE)) {
               rplCleanupLAMs(*(ValidateTop-1));
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ENDCASE));
               RetNum=OK_ENDCONSTRUCT;
               return;
           }

           rplError(ERR_MISPLACEDEND);
           RetNum=ERR_SYNTAX;
           return;

       }

       if((TokenLen==5) && (!utf8ncmp((char *)TokenStart,"ENDDO",5)))
       {
           // ENDDO

           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,UNTIL)) {
               rplCleanupLAMs(0);
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ENDDO));
               RetNum=OK_ENDCONSTRUCT;
               return;
           }

           rplError(ERR_MISPLACEDEND);
           RetNum=ERR_SYNTAX;
           return;

       }

       if((TokenLen==8) && (!utf8ncmp((char *)TokenStart,"ENDWHILE",8)))
       {
           // ENDWHILE

           if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,REPEAT)) {
               rplCleanupLAMs(0);
               rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ENDWHILE));
               RetNum=OK_ENDCONSTRUCT;
               return;
           }

           rplError(ERR_MISPLACEDEND);
           RetNum=ERR_SYNTAX;
           return;

       }

        // FOR... NEXT AND FOR... STEP
        // START... NEXT AND START... STEP

       if((TokenLen==3) && (!utf8ncmp((char *)TokenStart,"FOR",3)))
       {
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,FOR));
           RetNum=OK_NEEDMORESTARTCONST;
           return;
       }

       if((TokenLen==5) && (!utf8ncmp((char *)TokenStart,"START",5)))
       {
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,START));
           rplCreateLAMEnvironment(CompileEnd-1);

           RetNum=OK_STARTCONSTRUCT;
           return;
       }

       if((TokenLen==4) && (!utf8ncmp((char *)TokenStart,"NEXT",4)))
       {
           if( (CurrentConstruct!=MKOPCODE(LIBRARY_NUMBER,FOR))&&(CurrentConstruct!=MKOPCODE(LIBRARY_NUMBER,START))) {
                RetNum=ERR_SYNTAX;
                return;
           }
           // DO A COUPLE OF CHECKS

           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,NEXT));
           rplCleanupLAMs(*(ValidateTop-1));
           RetNum=OK_ENDCONSTRUCT;
           return;
       }

       if((TokenLen==4) && (!utf8ncmp((char *)TokenStart,"STEP",4)))
       {
           if( (CurrentConstruct!=MKOPCODE(LIBRARY_NUMBER,FOR))&&(CurrentConstruct!=MKOPCODE(LIBRARY_NUMBER,START))) {
                RetNum=ERR_SYNTAX;
                return;
           }
           // DO A COUPLE OF CHECKS

           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,STEP));
           rplCleanupLAMs(*(ValidateTop-1));
           RetNum=OK_ENDCONSTRUCT;
           return;
       }



       // DO ... UNTIL ... END

       if((TokenLen==2) && (!utf8ncmp((char *)TokenStart,"DO",2)))
       {
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,DO));

           rplCreateLAMEnvironment(CompileEnd-1);

           RetNum=OK_STARTCONSTRUCT;
           return;
       }

       if((TokenLen==5) && (!utf8ncmp((char *)TokenStart,"UNTIL",5)))
       {
           if(CurrentConstruct!=MKOPCODE(LIBRARY_NUMBER,DO)) {
               RetNum=ERR_SYNTAX;
               return;
            }
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,UNTIL));
           RetNum=OK_CHANGECONSTRUCT;
           return;
       }

       // WHILE ... REPEAT ... END

               if((TokenLen==5) && (!utf8ncmp((char *)TokenStart,"WHILE",5)))
               {
                   rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,WHILE));
                   rplCreateLAMEnvironment(CompileEnd-1);
                   RetNum=OK_STARTCONSTRUCT;
                   return;
               }

               if((TokenLen==6) && (!utf8ncmp((char *)TokenStart,"REPEAT",6)))
               {
                   if(CurrentConstruct!=MKOPCODE(LIBRARY_NUMBER,WHILE)) {
                       RetNum=ERR_SYNTAX;
                       return;
                    }
                   rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,REPEAT));
                   RetNum=OK_CHANGECONSTRUCT;
                   return;
               }




        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;









    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors
        if(ISPROLOG(*DecompileObject)) {
            if(CurrentConstruct!=CMD_XEQSECO)
                rplDecompAppendString((BYTEPTR)"«");
            RetNum=OK_STARTCONSTRUCT;
           return;
        }


        // CHECK IF THE TOKEN IS SEMI

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,QSEMI))
        {
            if(! (ISPROLOG(CurrentConstruct)&&(LIBNUM(CurrentConstruct)==LIBRARY_NUMBER))) {
                rplDecompAppendString((BYTEPTR)"»");
                RetNum=OK_CONTINUE;
                return;
            }
            rplCleanupLAMs(*(ValidateTop-1));
            rplDecompAppendString((BYTEPTR)"»");
            RetNum=OK_ENDCONSTRUCT;
            return;
        }






         // FOR... NEXT AND FOR... STEP
         // START... NEXT AND START... STEP

    if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,FOR)) {

            rplDecompAppendString((BYTEPTR)"FOR");

            // CREATE A NEW ENVIRONMENT
            rplCreateLAMEnvironment(DecompileObject);
            rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);        // NULLLAM FOR THE COMPILER
            rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);        // NULLLAM FOR THE COMPILER
            rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);        // NULLLAM FOR THE COMPILER
            rplCreateLAM(rplSkipOb(DecompileObject),(WORDPTR)zero_bint);      // LAM COUNTER

            RetNum=OK_STARTCONSTRUCT;
            return;
        }

    if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,START)) {

            rplDecompAppendString((BYTEPTR)"START");

            // CREATE A NEW ENVIRONMENT
            rplCreateLAMEnvironment(DecompileObject);
            rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);        // NULLLAM FOR THE COMPILER
            rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);        // NULLLAM FOR THE COMPILER
            rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);        // NULLLAM FOR THE COMPILER
            rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);      // LAM COUNTER

            RetNum=OK_STARTCONSTRUCT;
            return;
        }

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,NEXT))
        {
            // NEXT
             if((CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,FOR))||(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,START))) {
                rplCleanupLAMs(*(ValidateTop-1));
                rplDecompAppendString((BYTEPTR)"NEXT");
                RetNum=OK_ENDCONSTRUCT;
                return;
            }
             rplDecompAppendString((BYTEPTR)"NEXT");
             RetNum=OK_CONTINUE;
             return;

        }

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,STEP))
        {
            // STEP
             if((CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,FOR))||(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,START))) {
                rplCleanupLAMs(*(ValidateTop-1));
                rplDecompAppendString((BYTEPTR)"STEP");
                RetNum=OK_ENDCONSTRUCT;
                return;
            }
             rplDecompAppendString((BYTEPTR)"STEP");
             RetNum=OK_CONTINUE;
             return;

        }

        // DO ... UNTIL ... END

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,DO)) {

                rplDecompAppendString((BYTEPTR)"DO");

                // CREATE A NEW ENVIRONMENT
                rplCreateLAMEnvironment(DecompileObject);

                RetNum=OK_STARTCONSTRUCT;
                return;
            }

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,ENDDO))
        {
            // ENDDO
             if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,DO)) {
                rplCleanupLAMs(*(ValidateTop-1));
                rplDecompAppendString((BYTEPTR)"END");
                RetNum=OK_ENDCONSTRUCT;
                return;
            }
             rplDecompAppendString((BYTEPTR)"END");
             RetNum=OK_CONTINUE;
             return;

        }



        // WHILE ... REPEAT ... END

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,WHILE)) {

                rplDecompAppendString((BYTEPTR)"WHILE");

                // CREATE A NEW ENVIRONMENT
                rplCreateLAMEnvironment(DecompileObject);

                RetNum=OK_STARTCONSTRUCT;
                return;
            }

                if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,ENDWHILE)) {
                    // ENDWHILE

                    if(CurrentConstruct==MKOPCODE(LIBRARY_NUMBER,WHILE)) {
                        rplCleanupLAMs(*(ValidateTop-1));
                        rplDecompAppendString((BYTEPTR)"END");
                        RetNum=OK_ENDCONSTRUCT;
                        return;
                    }
                    rplDecompAppendString((BYTEPTR)"END");
                    RetNum=OK_CONTINUE;
                    return;

                }

                // TODO: ADD A FLAG TO CONTROL IF USER WANTS ENDIF INSTEAD OF END, ETC.

                if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,THENERR)) {
                    rplDecompAppendString((BYTEPTR)"THEN");
                    RetNum=OK_CONTINUE;
                    return;
                }
                if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,THENCASE)) {
                    rplDecompAppendString((BYTEPTR)"THEN");
                    RetNum=OK_CONTINUE;
                    return;
                }
                if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,ENDIF)) {
                    rplDecompAppendString((BYTEPTR)"END");
                    RetNum=OK_CONTINUE;
                    return;
                }
                if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,ENDERR)) {
                    rplDecompAppendString((BYTEPTR)"END");
                    RetNum=OK_CONTINUE;
                    return;
                }
                if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,ENDTHEN)) {
                    rplDecompAppendString((BYTEPTR)"END");
                    RetNum=OK_CONTINUE;
                    return;
                }
                if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,ENDCASE)) {
                    rplDecompAppendString((BYTEPTR)"END");
                    RetNum=OK_CONTINUE;
                    return;
                }
                if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,ELSEERR)) {
                    rplDecompAppendString((BYTEPTR)"ELSE");
                    RetNum=OK_CONTINUE;
                    return;
                }



        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds((char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
        return;

    case OPCODE_COMPILECONT:
        // COMPILE THE IDENT IMMEDIATELY AFTER A FOR LOOP
    {
        BYTEPTR tokst=(BYTEPTR)TokenStart;
        BYTEPTR tokend=(BYTEPTR)BlankStart;

        if(*tokst=='\'') {
            // IT'S A QUOTED IDENT
            if((tokend>tokst+2) && (*(tokend-1)=='\'')) {
                --tokend;
                ++tokst;
            }
            else {
                RetNum=ERR_SYNTAX;
                return;
            }

        }

        if(!rplIsValidIdent(tokst,tokend)) {
         RetNum=ERR_SYNTAX;
         return;
        }

        ScratchPointer2=CompileEnd;


        // CAREFUL, COMPILEIDENT USES ScratchPointer1!!!
        rplCompileIDENT(DOIDENT,tokst,tokend);

        rplCreateLAMEnvironment(ScratchPointer2-1);
        rplCreateLAM((WORDPTR)nulllam_ident,ScratchPointer2);        // NULLLAM FOR THE COMPILER
        rplCreateLAM((WORDPTR)nulllam_ident,ScratchPointer2);        // NULLLAM FOR THE COMPILER
        rplCreateLAM((WORDPTR)nulllam_ident,ScratchPointer2);        // NULLLAM FOR THE COMPILER
        rplCreateLAM(ScratchPointer2,ScratchPointer2);      // LAM COUNTER INITIALIZED WITH THE STARTING VALUE


        RetNum=OK_CONTINUE;
        return;
    }


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
        // BASIC CHECKS
        WORDPTR ptr,objend;

        objend=rplSkipOb(ObjectPTR);
        ptr=ObjectPTR+1;

        while((*ptr!=MKOPCODE(LIBRARY_NUMBER,QSEMI))&&(ptr<objend)) {

            // TODO: RECURSIVELY CHECK OBJECT VALIDITY IN HERE

            ptr=rplSkipOb(ptr);
        }

        if(ptr!=objend-1) { RetNum=ERR_INVALID; return; }

        }

        RetNum=OK_CONTINUE;
        return;


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

