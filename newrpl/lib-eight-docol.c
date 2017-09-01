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
#define LIBRARY_NUMBER  8


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(EXITRPL,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    ECMD(AUTOBKPOINT,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(XEQSECO,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(SEMI,";",MKTOKENINFO(1,TITYPE_NOTALLOWED,1,2)), \
    CMD(EVAL1NEXT,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ERROR_REENTER,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    CMD(RESUME,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(DOERR,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(ERRN,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(ERRM,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(ERR0,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(HALT,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ENDOFCODE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    CMD(CONT,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(SST,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(SSTIN,"SST↓",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(KILL,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(SETBKPOINT,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(CLRBKPOINT,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(DBUG,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(BLAMEERR,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(RETRYSEMI,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2))







// ADD MORE OPCODES HERE

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO

#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

ROMOBJECT errormsg_ident[]={
    MKPROLOG(DOIDENT,2),
    TEXT2WORD('E','r','r','o'),
    TEXT2WORD('r','M','s','g')
};



ROMOBJECT error_reenter_seco[]={
    CMD_XEQSECO,
    CMD_ERROR_REENTER
};

ROMOBJECT bkpoint_seco[]={
    MKPROLOG(DOCOL,12),
    CMD_IFERR,
    CMD_OVR_XEQ,
    CMD_OVR_NOT,
    CMD_THENERR,
    MAKESINT(1),
    CMD_ENDERR,
    CMD_IF,
    CMD_THEN,
    CMD_CONT,
    CMD_ENDIF,
    CMD_ENDOFCODE,
    CMD_SEMI
};



INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib8_menu_debug);
INCLUDE_ROMOBJECT(lib8_menu_error);


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib8_menu_debug,
    (WORDPTR)lib8_menu_error,
    (WORDPTR)errormsg_ident,
    (WORDPTR)error_reenter_seco,
    (WORDPTR)bkpoint_seco,
    0
};

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE

        rplPushRet(IPtr);       // PUSH CURRENT POINTER AS THE RETURN ADDRESS. AT THIS POINT, IPtr IS POINTING TO THIS SECONDARY WORD
                                // BUT THE MAIN LOOP WILL ALWAYS SKIP TO THE NEXT OBJECT AFTER A SEMI.
        CurOpcode=MKPROLOG(LIBRARY_NUMBER,0); // ALTER THE SIZE OF THE SECONDARY TO ZERO WORDS, SO THE NEXT EXECUTED INSTRUCTION WILL BE THE FIRST IN THIS SECONDARY
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case EXITRPL:
        rplException(EX_EXITRPL);
        return;

    case ENDOFCODE:
    {
        // SAME AS HALT BUT DON'T AFFECT THE HALTED PROGRAM
        rplException(EX_HALT);

        // ONCE IT'S HALTED, DISABLE SINGLE STEP MODE
        rplDisableSingleStep();

        return;

    }
    case AUTOBKPOINT:
    {
        if(HaltedIPtr) {
            return; // CAN'T HALT WITHIN AN ALREADY HALTED PROGRAM!
        }
        HaltedIPtr=IPtr+1;    // SAVE CODE LOCATION AFTER HALT
        HaltedRSTop=RSTop;  // SAVE RETURN STACK POINTER
        HaltednLAMBase=nLAMBase;
        HaltedLAMTop=LAMTop;
        rplException(EX_HALT|EX_AUTORESUME);
        // ONCE IT'S HALTED, DISABLE SINGLE STEP MODE
        rplDisableSingleStep();

        return;
    }
    case HALT:
    {
        if(HaltedIPtr) {
            return; // CAN'T HALT WITHIN AN ALREADY HALTED PROGRAM!
        }
        HaltedIPtr=IPtr+1;    // SAVE CODE LOCATION AFTER HALT
        HaltedRSTop=RSTop;  // SAVE RETURN STACK POINTER
        HaltednLAMBase=nLAMBase;
        HaltedLAMTop=LAMTop;
        rplException(EX_HALT);
        // ONCE IT'S HALTED, DISABLE SINGLE STEP MODE
        rplDisableSingleStep();

        return;
    }

    case CONT:
    {
        if(!HaltedIPtr) return;

        // UN-PAUSE ALL HARDWARE BREAKPOINTS. THIS IS NEEDED FOR breakpt_seco ONLY.
        BreakPtFlags&=~BKPT_ALLPAUSED;

        // CONTINUE HALTED EXECUTION
        if(RSTop>=HaltedRSTop) {
            IPtr=HaltedIPtr-1;
            RSTop=HaltedRSTop;
            if(LAMTop>=HaltedLAMTop) LAMTop=HaltedLAMTop;
            if(nLAMBase>=HaltednLAMBase) nLAMBase=HaltednLAMBase;
            HaltedIPtr=0;
            if(HWExceptions&EX_HWBKPOINT) HWExceptions|=EX_HWBKPTSKIP;  // SKIP ONE SO AT LEAST IT EXECUTES ONE OPCODE
        }
        // CurOpcode IS A COMMAND, SO THE HALT INSTRUCTION WILL BE SKIPPED
        return;
    }
    case SSTIN:
    {
        // SINGLE STEP A HALTED PROGRAM
        if(!HaltedIPtr) return;

        rplEnableSingleStep();

        // AND THIS IS THE SAME AS "CONT" FROM HERE ON
        BreakPtFlags&=~BKPT_ALLPAUSED;

        HWExceptions|=EX_HWBKPOINT;

        // CONTINUE HALTED EXECUTION
        if(RSTop>=HaltedRSTop) {
            IPtr=HaltedIPtr-1;
            RSTop=HaltedRSTop;
            if(LAMTop>=HaltedLAMTop) LAMTop=HaltedLAMTop;
            if(nLAMBase>=HaltednLAMBase) nLAMBase=HaltednLAMBase;
            HaltedIPtr=0;
            if(HWExceptions&EX_HWBKPOINT) HWExceptions|=EX_HWBKPTSKIP;  // SKIP ONE SO AT LEAST IT EXECUTES ONE OPCODE
        }
        // CurOpcode IS A COMMAND, SO THE HALT INSTRUCTION WILL BE SKIPPED
        return;

    }

    case SST:
    {
        // SINGLE STEP A HALTED PROGRAM BUT SKIP OVER INNER CODE
        if(!HaltedIPtr) return;


        if(ISPROGRAM(*HaltedIPtr) || ISIDENT(*HaltedIPtr) || (*HaltedIPtr==CMD_OVR_EVAL)||(*HaltedIPtr==CMD_OVR_XEQ)||(*HaltedIPtr==CMD_OVR_EVAL1)) {
                // SKIP OVER TO THE END OF THE CODE
                // USE THE SINGLE STEP WITH A SPECIFIC LOCATION SO IT ONLY STOPS AFTER THE CODE FINISHED
                SET_BKPOINTFLAG(2,BKPT_LOCATION|BKPT_ENABLED);
                BreakPt3Arg=0;
                BreakPt3Pointer=rplSkipOb(HaltedIPtr);
        }
        else rplEnableSingleStep();

        // AND THIS IS THE SAME AS "CONT" FROM HERE ON
        BreakPtFlags&=~BKPT_ALLPAUSED;

        HWExceptions|=EX_HWBKPOINT;

        // CONTINUE HALTED EXECUTION
        if(RSTop>=HaltedRSTop) {
            IPtr=HaltedIPtr-1;
            RSTop=HaltedRSTop;
            if(LAMTop>=HaltedLAMTop) LAMTop=HaltedLAMTop;
            if(nLAMBase>=HaltednLAMBase) nLAMBase=HaltednLAMBase;
            HaltedIPtr=0;
            if(HWExceptions&EX_HWBKPOINT) HWExceptions|=EX_HWBKPTSKIP;  // SKIP ONE SO AT LEAST IT EXECUTES ONE OPCODE
        }
        // CurOpcode IS A COMMAND, SO THE HALT INSTRUCTION WILL BE SKIPPED
        return;

    }

    case KILL:
    {   // KILL THE HALTED PROGRAM
        if(!HaltedIPtr) return;
        HaltedIPtr=0;
        if(RSTop>HaltedRSTop) {
            // REMOVE ALL INFORMATION LEFT BY THE HALTED PROGRAM, KEEP THE CURRENT ONE
            memmovew(RStk,HaltedRSTop,(RSTop-HaltedRSTop)*(sizeof(WORDPTR *)/sizeof(WORD)));
            RSTop-=HaltedRSTop-RStk;
        } else RSTop=RStk;
        if(LAMTop>HaltedLAMTop) {
            // REMOVE ALL INFORMATION LEFT BY THE HALTED PROGRAM, KEEP THE CURRENT ONE
            memmovew(LAMs,HaltedLAMTop,(LAMTop-HaltedLAMTop)*(sizeof(WORDPTR *)/sizeof(WORD)));
            LAMTop-=HaltedLAMTop-LAMs;
            nLAMBase-=HaltedLAMTop-LAMs;
        } else LAMTop=nLAMBase=LAMs;

        return;
    }


    case DBUG:
    {

        if(HaltedIPtr) return;      // DO NOTHING IF A PROGRAM IS ALREADY BEING DEBUGGED

        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISPROGRAM(*rplPeekData(1))) {
            rplError(ERR_PROGRAMEXPECTED);
            return;
        }

        // AND XEQ  AND IMMEDIATELY HALT THE SECONDARY

        rplPushRet(IPtr);       // PUSH CURRENT POINTER AS THE RETURN ADDRESS.
        HaltedIPtr=rplPopData()+1;      // SET NEW IPTR TO THE PROGRAM, TO BE EXECUTED
        HaltedRSTop=RSTop;  // SAVE RETURN STACK POINTER
        HaltednLAMBase=nLAMBase;
        HaltedLAMTop=LAMTop;
        rplException(EX_HALT);
        // ONCE IT'S HALTED, DISABLE SINGLE STEP MODE
        rplDisableSingleStep();

        return;
    }

    case XEQSECO:
        // IF THE NEXT OBJECT IN THE SECONDARY
        // IS A SECONDARY, IT EVALUATES IT INSTEAD OF PUSHING IT ON THE STACK
        ++IPtr;
        if(ISPROLOG(*IPtr)&& (LIBNUM(*IPtr)==SECO)) {
            CurOpcode=MKPROLOG(SECO,0); // MAKE IT SKIP INSIDE THE SECONDARY
            rplPushRet(IPtr);
            return;
        }
        // HAVE NO EFFECT ON ANY OTHER OBJECTS
        --IPtr;
        return;

    case SEMI:
        // POP THE RETURN ADDRESS
        IPtr=rplPopRet();   // GET THE CALLER ADDRESS
        if(IPtr) CurOpcode=*IPtr;    // SET THE WORD SO MAIN LOOP SKIPS THIS OBJECT, AND THE NEXT ONE IS EXECUTED
        return;

    case RETRYSEMI:
        // POP THE RETURN ADDRESS BUT RETRY THE OBJECT/COMMAND AT IPtr
        IPtr=rplPopRet()-1;
        //   CurOpcode IS ALREADY CMD_RETRYSEMI WITH A SIZE OF ONE WORD, SO IT WILL RETRY WHATEVER WAS ON THE STACK
        return;

    case EVAL1NEXT:
        // DO EVAL1 ON THE NEXT OBJECT IN THE SECONDARY, THEN SKIP IT
        if(ISPROLOG(*(IPtr+1))) {   // ONLY SKIP OBJECTS, NOT COMMANDS TO GUARANTEE IT CAN'T BREAK OUT OF LOOPS OR SECONDARIES
        ++IPtr;
        rplPushData(IPtr);
        rplCallOvrOperator(CMD_OVR_EVAL1);
        }
        // SINCE IPtr POINTS TO THE NEXT OBJECT, IT WILL BE SKIPPED
        return;

    case ERROR_REENTER:
        // THIS IS CALLED ONLY WHEN A SPECIAL ERROR THROWS AN ERROR, TO CLEANUP AND EXIT
        // JUST LIKE A NORMAL ERROR HANDLER WOULD
        // THE RETURN STACK CAN NEVER BE EMPTY (CHECK THAT) AND IT SHOULD HAVE:
        // ON NORMAL RETURN FROM AN ERROR HANDLER:
        // 1: IPtr TO RESUME
        // 2: ... 3: DATA STACK PROTECTION
        // 4: ... SAVED USER ERROR HANDLER
    {
        if(rplDepthRet()<7) {
            // THIS OPCODE WAS NOT CALLED FROM WITHIN AN ERROR HANDLER, JUST DO EXITRPL
            rplException(EX_EXITRPL);
            return;
        }

            rplRemoveExceptionHandler();    //  REMOVE SECOND RE-ENTER HANDLER
            rplRemoveExceptionHandler();    //  REMOVE ORIGINAL RE-ENTER ERROR HANDLER
            rplPopRet();                    // REMOVE RESUME ADDRESS
            DSTop=rplUnprotectData();       // CLEANUP STACK FROM ANYTHING THE FAILING ERROR HANDLER DID
            rplRemoveExceptionHandler();    //  REMOVE ORIGINAL USER ERROR HANDLER
            // THROW THE ERROR TO THE PREVIOUS ERROR HANDLER, AS THIS ONE COULDN'T HANDLE IT
            if(TrappedExceptions==EX_ERRORCODE) rplError(TrappedErrorCode);
            else rplException(TrappedExceptions);
            return; // AND CONTINUE EXECUTION AS IF SEMI WAS EXECUTED
    }


    case RESUME:
        // THIS IS CALLED FROM WITHIN A SPECIAL ERROR HANDLER, TO RESUME EXECUTION WHERE IT STOPPED
        // JUST LIKE A NORMAL ERROR HANDLER WOULD
        // THE RETURN STACK CAN NEVER BE EMPTY (CHECK THAT) AND IT SHOULD HAVE:
        // ON NORMAL RETURN FROM AN ERROR HANDLER:
        // 1: ... SAVED ERROR HANDLER ...
        // 5: IPtr TO RESUME
        // 6: ... 7: DATA STACK PROTECTION
        // 8: ... SAVED USER ERROR HANDLER
    {
        if(rplDepthRet()<11) {
            // THIS OPCODE WAS NOT CALLED FROM WITHIN AN ERROR HANDLER, JUST DO NOP
            return;
        }
        if(ErrorHandler!=(WORDPTR)error_reenter_seco) {
            // NOT WITHIN AN ERROR HANDLER, DO NOTHING
            return;
        }

            rplRemoveExceptionHandler();    // REMOVE THE REENTRANT EXCEPTION HANDLER
            IPtr=rplPopRet();                    // RESUME AT THIS ADDRESS
            CurOpcode=*IPtr;                     // MAKE SURE THE OFFENDING OBJECT IS SKIPPED
            DSTop=rplUnprotectData();       // CLEANUP STACK FROM ANYTHING THE ERROR HANDLER DID
            // rplRemoveExceptionHandler();    // LEAVE THE ORIGINAL USER ERROR HANDLER ACTIVE TO TRAP MORE ERRORS

            return; // AND RESUME EXECUTION LIKE NOTHING HAPPENED!
    }

    case DOERR:
        // THROW AN ERROR BY EITHER A STRING OR ERROR CODE
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(ISNUMBER(*rplPeekData(1))) {
            BINT64 errorcode=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;
            if((errorcode<0)||(errorcode>=0x7ffff)) {
                rplError(ERR_BADERRORCODE);
                return;
            }

            // DO MORE CHECKS ON THE ERROR CODE

            // TRY TO GET A MESSAGE FROM A LIBRARY
            LIBHANDLER han=rplGetLibHandler(LIBFROMMSG(errorcode));
            if(!han) {
                rplError(ERR_BADERRORCODE);
                return;
            }
            WORD SavedOpcode=CurOpcode;
            BINT SavedException=Exceptions;
            BINT SavedErrorCode=ErrorCode;

            Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE LIBRARY TO RUN
            CurOpcode=MKOPCODE(LIBFROMMSG(errorcode),OPCODE_LIBMSG);
            LibError=MAKESINT(errorcode);
            RetNum=-1;
            (*han)();

            Exceptions=SavedException;
            ErrorCode=SavedErrorCode;
            CurOpcode=SavedOpcode;

            if(RetNum!=OK_CONTINUE) {
                rplError(ERR_BADERRORCODE);
                return;
            }

            rplDropData(1);
            rplError(MAKESINT(errorcode));
            rplBlameUserCommand();
            return;
        }

        if(ISSTRING(*rplPeekData(1)))
        {
          rplStoreSettings((WORDPTR)errormsg_ident,rplPeekData(1));
          if(!Exceptions)
          {
              rplDropData(1);
              rplError(MAKEMSG(LIBRARY_NUMBER,0));
              rplBlameUserCommand();
          }
          return;
        }

        rplError(ERR_BADERRORCODE);
        return;



    }

    case ERRN:
    {
        // GET THE PREVIOUS ERROR CODE
        BINT msgcode;
        if(TrappedExceptions==EX_ERRORCODE) msgcode=TrappedErrorCode;
        else {
            msgcode=0;
            if(TrappedExceptions) {
            int errbit;
            for(errbit=0;errbit<8;++errbit) if(TrappedExceptions&(1<<errbit)) { msgcode=MAKEMSG(0,1<<errbit); break; }
            if(!msgcode) msgcode=ERR_UNKNOWNEXCEPTION;
            }
        }

        rplNewSINTPush(msgcode,HEXBINT);
        return;

    }

    case ERRM:
    {
        // GET THE PREVIOUS ERROR CODE
        BINT msgcode;
        WORDPTR string=0;
        if(TrappedExceptions==EX_ERRORCODE) msgcode=TrappedErrorCode;
        else {
            int errbit;
            msgcode=0;
            for(errbit=0;errbit<8;++errbit) if(Exceptions&(1<<errbit)) { msgcode=MAKEMSG(0,TrappedExceptions); break; }
            if(!msgcode) msgcode=ERR_UNKNOWNEXCEPTION;
        }


        // TRY TO GET A MESSAGE FROM A LIBRARY
        LIBHANDLER han=rplGetLibHandler(LIBFROMMSG(msgcode));
        if(!han) {
            string=(WORDPTR)empty_string;
        }
        else {
        WORD SavedOpcode=CurOpcode;
        BINT SavedException=Exceptions;
        BINT SavedErrorCode=ErrorCode;

        Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE LIBRARY TO RUN
        CurOpcode=MKOPCODE(LIBFROMMSG(msgcode),OPCODE_LIBMSG);
        LibError=msgcode;
        RetNum=-1;
        (*han)();

        Exceptions=SavedException;
        ErrorCode=SavedErrorCode;
        CurOpcode=SavedOpcode;

        if(RetNum!=OK_CONTINUE) string=(WORDPTR)empty_string;
        else string=ObjectPTR;
        }

        rplPushData(string);

        return;

    }

    case ERR0:
    {
        // CLEAR ALL ERROR CODES
        TrappedExceptions=0;
        TrappedErrorCode=0;
        ErrorCode=0;
        Exceptions=0;
        return;
    }


    case SETBKPOINT:
    {
        // SET A BREAKPOINT ANYWHERE IN A BLOCK OF CODE
        // << ... CODE ... >> OFFSET [ << ... CONDITION ... >> ]
        // OFFSET IS THE NUMBER OF OBJECTS TO SKIP FROM START OF CODE
        // 1 == FIRST WORD INSIDE SECONDARY, 0 == ANYWHERE WITHIN SECONDARY
        // BREAKPOINT WILL TRIGGER IF INSTRUCTION POINTER IS AT OR INSIDE THE WORD/OBJECT AT OFFSET.
        // << ... CONDITION ... >> IS AN OPTIONAL ARGUMENT WITH A PROGRAM
        // THAT LEAVES 1 OR 0 (TRUE/FALSE) IN THE STACK. MUST MAKE NO OTHER CHANGES!
        // BREAKPOINT WILL TRIGGER IF CONDITION IS TRUE.

        WORDPTR code,offset,condition;

        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(ISPROGRAM(*rplPeekData(2))) {
            code=rplPeekData(2);
            offset=rplPeekData(1);
            condition=0;

        } else {
            code=rplPeekData(3);
            offset=rplPeekData(2);
            condition=rplPeekData(1);
        }

        if(!ISPROGRAM(*code)) {
            rplError(ERR_PROGRAMEXPECTED);
            return;
        }
        if(!ISNUMBER(*offset)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        if(condition) {
            if(!ISPROGRAM(*condition)) {
                rplError(ERR_PROGRAMEXPECTED);
                return;
            }
        }

        BINT64 off=rplReadNumberAsBINT(offset);

        if(off<0) {
            rplError(ERR_POSITIVEINTEGEREXPECTED);
            return;
        }

        WORDPTR ptr=code+1;
        --off;
        while(off--) {
            ptr=rplSkipOb(ptr);
            if(ptr>=rplSkipOb(code)) {
                // OUT OF BOUNDS, JUST TRIGGER ON THE ENTIRE SECONDARY
                ptr=code;
                break;
            }
        }

        // HERE WE HAVE THE LOCATION OF THE BREAKPOINT

        BreakPt1Arg=condition;
        BreakPt1Pointer=ptr;

        BINT flags=BKPT_ENABLED|BKPT_LOCATION;
        if(condition) flags|=BKPT_COND;

        SET_BKPOINTFLAG(0,flags);

        if(condition) rplDropData(3);
        else rplDropData(2);

        HWExceptions|=EX_HWBKPOINT;

        return;

    }
    case CLRBKPOINT:
    {
        // REMOVE THE BREAK POINT

        SET_BKPOINTFLAG(0,0);
        BreakPt1Arg=0;
        BreakPt1Pointer=0;

        HWExceptions&=~EX_HWBKPOINT;

        return;

    }

    case BLAMEERR:
        // THROW AN ERROR BY EITHER A STRING OR ERROR CODE
        // AND BLAME A CERTAIN PROGRAM BY STRING OR IDENT
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }


        if(ISNUMBER(*rplPeekData(1))) {
            BINT64 errorcode=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;
            if((errorcode<0)||(errorcode>=0x7ffff)) {
                rplError(ERR_BADERRORCODE);
                return;
            }

            // DO MORE CHECKS ON THE ERROR CODE

            // TRY TO GET A MESSAGE FROM A LIBRARY
            LIBHANDLER han=rplGetLibHandler(LIBFROMMSG(errorcode));
            if(!han) {
                rplError(ERR_BADERRORCODE);
                return;
            }
            WORD SavedOpcode=CurOpcode;
            BINT SavedException=Exceptions;
            BINT SavedErrorCode=ErrorCode;

            Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE LIBRARY TO RUN
            CurOpcode=MKOPCODE(LIBFROMMSG(errorcode),OPCODE_LIBMSG);
            LibError=MAKESINT(errorcode);
            RetNum=-1;
            (*han)();

            Exceptions=SavedException;
            ErrorCode=SavedErrorCode;
            CurOpcode=SavedOpcode;

            if(RetNum!=OK_CONTINUE) {
                rplError(ERR_BADERRORCODE);
                return;
            }

            rplDropData(1);

            WORDPTR blame=rplPopData();
            if(!ISSTRING(*blame) && !ISIDENT(*blame)) blame=0;

            rplError(MAKESINT(errorcode));
            BlameCmd=0; // REMOVE THE GUI PROVIDED NAME TO BLAME
            if(blame) rplBlameError(blame); else rplBlameUserCommand();
            return;
        }

        if(ISSTRING(*rplPeekData(1)))
        {
          rplStoreSettings((WORDPTR)errormsg_ident,rplPeekData(1));
          if(!Exceptions)
          {
              rplDropData(1);
              WORDPTR blame=rplPopData();
              if(!ISSTRING(*blame) && !ISIDENT(*blame)) blame=0;

              rplError(MAKEMSG(LIBRARY_NUMBER,0));
              BlameCmd=0; // REMOVE THE GUI PROVIDED NAME TO BLAME
              if(blame) rplBlameError(blame); else rplBlameUserCommand();
          }
          return;
        }

        rplError(ERR_BADERRORCODE);
        return;



    }
        // ADD MORE OPCODES HERE


    case OVR_SAME:
    // COMPARE PROGRAMS AS PLAIN OBJECTS, THIS INCLUDES SIMPLE COMMANDS IN THIS LIBRARY
        {
         BINT same=rplCompareObjects(rplPeekData(1),rplPeekData(2));
         rplDropData(2);
         if(same) rplPushTrue(); else rplPushFalse();
         return;
        }



    case OVR_ISTRUE:
    case OVR_EVAL:
    case OVR_EVAL1:
        // EXECUTE THE OBJECT
    case OVR_XEQ:
        // ALSO EXECUTE THE OBJECT
        if(!ISPROLOG(*rplPeekData(1))) {
            switch(*rplPeekData(1))
            {
            case CMD_EXITRPL:
            case CMD_AUTOBKPOINT:
            case CMD_XEQSECO:
            case CMD_SEMI:
            case CMD_EVAL1NEXT:
            case CMD_ERROR_REENTER:
            case CMD_RESUME:
            case CMD_HALT:
            case CMD_ENDOFCODE:
            {   // THESE COMMANDS CANNOT BE EVALUATED AS OBJECTS, NEED TO BE IN A RUNSTREAM
                rplDropData(1);
                return;
            }
            default:
            {   // EXECUTE THE COMMAND BY CALLING THE HANDLER DIRECTLY
                WORD saveOpcode=CurOpcode;
                CurOpcode=*rplPopData();
                // RECURSIVE CALL
                LIB_HANDLER();
                CurOpcode=saveOpcode;
                return;
            }

            }


        }
        rplPushRet(IPtr);       // PUSH CURRENT POINTER AS THE RETURN ADDRESS.
        IPtr=rplPopData();      // SET NEW IPTR TO THE PROGRAM, TO BE EXECUTED
        CurOpcode=MKPROLOG(LIBRARY_NUMBER,0); // ALTER THE SIZE OF THE SECONDARY TO ZERO WORDS, SO THE NEXT EXECUTED INSTRUCTION WILL BE THE FIRST IN THIS SECONDARY
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

       if((TokenLen==2) && (!utf8ncmp((char * )TokenStart,"::",2)))
       {
           rplCompileAppend((WORD) MKPROLOG(LIBRARY_NUMBER,0));
           RetNum=OK_STARTCONSTRUCT;
           return;
       }
       // CHECK IF THE TOKEN IS SEMI

       if(((TokenLen==1) && (!utf8ncmp((char *)TokenStart,";",1))))
       {
           if(CurrentConstruct!=MKPROLOG(LIBRARY_NUMBER,0)) {
               RetNum=ERR_SYNTAX;
               return;
           }
           rplCleanupLAMs(*(ValidateTop-1));
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,SEMI));
           RetNum=OK_ENDCONSTRUCT;
           return;
       }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

        // SINCE THIS IS THE LAST LIBRARY TO BE EVALUATED, DO ONE LAST PASS TO COMPILE IT AS AN IDENT
        // EITHER LAM OR IN USEROB
     return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        if(ISPROLOG(*DecompileObject)) {
            rplDecompAppendString((BYTEPTR)"::");
            RetNum=OK_STARTCONSTRUCT;
            return;
        }

        // CHECK IF THE TOKEN IS SEMI

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,SEMI))
        {
            if(! (ISPROLOG(CurrentConstruct)&&(LIBNUM(CurrentConstruct)==LIBRARY_NUMBER))) {
                RetNum=ERR_SYNTAX;
                return;
            }
            rplCleanupLAMs(*(ValidateTop-1));
            rplDecompAppendString((BYTEPTR)";");
            RetNum=OK_ENDCONSTRUCT;
            return;
        }

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,XEQSECO)) {
            rplDecompAppendString((BYTEPTR)"→");
            RetNum=OK_STARTCONSTRUCT;
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

        // THIS OPCODE RECEIVES A POINTER TO AN RPL COMMAND OR OBJECT IN ObjectPTR
        // NEEDS TO RETURN INFORMATION ABOUT THE TYPE:
        // IN RetNum: RETURN THE MKTOKENINFO() DATA FOR THE SYMBOLIC COMPILER AND CAS
        // IN DecompHints: RETURN SOME HINTS FOR THE DECOMPILER TO DO CODE BEAUTIFICATION (TO BE DETERMINED)
        // IN TypeInfo: RETURN TYPE INFORMATION FOR THE TYPE COMMAND
        //             TypeInfo: TTTTFF WHERE TTTT = MAIN TYPE * 100 (NORMALLY THE MAIN LIBRARY NUMBER)
        //                                FF = 2 DECIMAL DIGITS FOR THE SUBTYPE OR FLAGS (VARIES DEPENDING ON LIBRARY)
        //             THE TYPE COMMAND WILL RETURN A REAL NUMBER TypeInfo/100
        // FOR NUMBERS: TYPE=10 (REALS), SUBTYPES = .01 = APPROX., .02 = INTEGER, .03 = APPROX. INTEGER
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL BINT, .42 = HEX INTEGER

        if(ISPROLOG(*ObjectPTR)) {
        TypeInfo=LIBRARY_NUMBER*100;
        DecompHints=0;
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_NOTALLOWED,0,1);
        }
        else {
            TypeInfo=0;     // ALL COMMANDS ARE TYPE 0
            DecompHints=0;

            libGetInfo2(*ObjectPTR,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        }
        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER,(WORDPTR *)ROMPTR_TABLE,ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID);
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

        while((*ptr!=CMD_SEMI)&&(ptr<objend)) {

            // TODO: RECURSIVELY CHECK OBJECT VALIDITY IN HERE

            ptr=rplSkipOb(ptr);
        }

        if(ptr!=objend-1) { RetNum=ERR_INVALID; return; }

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
    {
        if(MENUNUMBER(MenuCodeArg)>1) {
            RetNum=ERR_NOTMINE;
            return;
        }
        ObjectPTR=ROMPTR_TABLE[MENUNUMBER(MenuCodeArg)+1];
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
        // RETURN A CUSTOM MESSAGE STORED IN SETTINGS
        WORDPTR string=rplGetSettings((WORDPTR)errormsg_ident);
        if(!string) string=(WORDPTR)empty_string;
        ObjectPTR=string;
        RetNum=OK_CONTINUE;
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
