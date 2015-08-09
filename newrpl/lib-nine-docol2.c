/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// LIBRARY ONE HAS RUNSTREAM OPERATORS FOR USERRPL
// AND DEFINES THE << >> CODE OBJECT

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL



// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  9
#define LIB_ENUM lib9enum
#define LIB_NAMES lib9_names
#define LIB_HANDLER lib9_handler
#define LIB_NUMBEROFCMDS LIB9_NUMBEROFCMDS

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(IF), \
    CMD(THEN), \
    CMD(ELSE), \
    CMD(ENDIF),   \
    CMD(CASE), \
    CMD(THENCASE), \
    CMD(ENDTHEN), \
    CMD(ENDCASE), \
    CMD(FOR), \
    CMD(START), \
    CMD(NEXT), \
    CMD(STEP), \
    CMD(DO), \
    CMD(UNTIL), \
    CMD(ENDDO), \
    CMD(WHILE), \
    CMD(REPEAT), \
    CMD(ENDWHILE), \
    CMD(IFERR), \
    CMD(THENERR), \
    CMD(ELSEERR), \
    CMD(ENDERR)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
#define CMD_EXTRANAME \
    "»"
#define CMD_EXTRAENUM \
    QSEMI



// INTERNAL DECLARATIONS



// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST , CMD_EXTRAENUM , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
const char * const LIB_NAMES[]= { CMD_LIST , CMD_EXTRANAME  };
#undef CMD


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
    // NO ARGUMENT CHECKS! THAT SHOULD'VE BEEN DONE BY THE OVERLOADED "EVAL" DISPATCHER
        rplPushRet(IPtr);       // PUSH CURRENT POINTER AS THE RETURN ADDRESS. AT THIS POINT, IPtr IS POINTING TO THIS SECONDARY WORD
                                // BUT THE MAIN LOOP WILL ALWAYS SKIP TO THE NEXT OBJECT AFTER A SEMI.
        IPtr=rplPopData();
        CurOpcode=MKPROLOG(LIBRARY_NUMBER,0); // ALTER THE SIZE OF THE SECONDARY TO ZERO WORDS, SO THE NEXT EXECUTED INSTRUCTION WILL BE THE FIRST IN THIS SECONDARY
    return;

    case IF:
    // THIS COMMAND DOES NOTHING, IT'S JUST A MARKER FOR THE COMPILER
        return;

    case THEN:
        {
        // BY DEFINITION, BINT 0 OR REAL 0.0 = FALSE, EVERYTHING ELSE IS TRUE
        if(rplDepthData()<1) {
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
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
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
            // EXTRACT OBJECT INTO A GC-SAFE POINTER
            ScratchPointer1=rplPopData();

            if(IS_FALSE(*ScratchPointer1)) {
                // SKIP ALL OBJECTS UNTIL ENDTHEN
                while(*IPtr!=MKOPCODE(LIBRARY_NUMBER,ENDTHEN)) rplSkipNext();
            }
        }
        return;
    case ENDTHEN:
        // IF THIS GETS EXECUTED, IT'S BECAUSE THE THEN CLAUSE WAS TRUE, SO SKIP UNTIL ENDCASE
        while(*IPtr!=MKOPCODE(LIBRARY_NUMBER,ENDCASE)) rplSkipNext();
        return;

    case ENDCASE:
        return;

    case FOR:
    {
        // DEFINE 3 LAMS, THE FIRST WILL BE THE LOW LIMIT, THEN THE HIGH LIMIT, THEN THE ITERATOR
        if(rplDepthData()<2) {
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
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
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
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
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
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
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
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
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }

        // BY DEFINITION, BINT 0 OR REAL 0.0 = FALSE, EVERYTHING ELSE IS TRUE

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
        IPtr=rplPeekRet(1);
        CurOpcode=*IPtr;
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
            rplDecompAppendString((BYTEPTR)"«");
            RetNum=OK_STARTCONSTRUCT;
            return;
        }


        // CHECK IF THE TOKEN IS SEMI

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,QSEMI))
        {
            if(! (ISPROLOG(CurrentConstruct)&&(LIBNUM(CurrentConstruct)==LIBRARY_NUMBER))) {
                RetNum=ERR_SYNTAX;
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
             RetNum=ERR_SYNTAX;
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
             RetNum=ERR_SYNTAX;
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
             RetNum=ERR_SYNTAX;
             return;

        }



        // WHILE ... REPEAT ... END

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,DO)) {

                rplDecompAppendString((BYTEPTR)"DO");

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
                    RetNum=ERR_SYNTAX;
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




