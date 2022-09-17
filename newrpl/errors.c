/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "libraries.h"
#include "sysvars.h"
#include "newrpl.h"

extern ROMOBJECT nullptr_catastrophic_seco[];

// THIS IS THE TRAP THAT HANDLES REAL NUMBER EXCEPTIONS AND CONVERTS THEM
// TO RPL KERNEL EXCEPTIONS
void decTrapHandler(int32_t error)
{
    // SIMPLY COPY THE MATHEMATIC EXCEPTIONS INTO THE UPPER 16 BITS OF OUR EXCEPTION WORD
    Exceptions |= error;
    ExceptionPointer = IPtr;
    // RETURN AND KEEP GOING
    return;
}

// SET THE HANDLER THAT WILL TRAP EXCEPTIONS

void rplSetExceptionHandler(word_p Handler)
{
    // SAVE CURRENT ERROR HANDLERS
    if(!ErrorHandler)
        rplPushRet((word_p) nullptr_catastrophic_seco);
    else
        rplPushRet((word_p) ErrorHandler);
    if(!ErrorRSTop)
        rplPushRet((word_p) nullptr_catastrophic_seco);
    else
        rplPushRet((word_p) ErrorRSTop);
    if(!ErrorLAMTop)
        rplPushRet((word_p) nullptr_catastrophic_seco);
    else
        rplPushRet((word_p) ErrorLAMTop);
    if(!ErrornLAMBase)
        rplPushRet((word_p) nullptr_catastrophic_seco);
    else
        rplPushRet((word_p) ErrornLAMBase);

    ErrorHandler = Handler;
    ErrorLAMTop = LAMTop;
    ErrornLAMBase = nLAMBase;
    ErrorRSTop = RSTop;

}

// REMOVE THE HANDLERS SET BY rplSetExceptionHandler
// AND RESTORE THE PREVIOUS ONE WITHOUT RAISING AN EXCEPTION
void rplRemoveExceptionHandler()
{
    // RESTORE LAM ENVIRONMENT
    nLAMBase = ErrornLAMBase;
    LAMTop = ErrorLAMTop;

    // RESTORE RETURN STACK
    RSTop = ErrorRSTop;

    // RESTORE OLD ERROR HANDLER POINTERS
    ErrornLAMBase = (word_p *) rplPopRet();
    ErrorLAMTop = (word_p *) rplPopRet();
    ErrorRSTop = (word_p *) rplPopRet();
    ErrorHandler = (word_p) rplPopRet();

    if(ErrorHandler == (word_p) nullptr_catastrophic_seco)
        ErrorHandler = 0;
    if(ErrorLAMTop == (word_p *) nullptr_catastrophic_seco)
        ErrorLAMTop = 0;
    if(ErrorRSTop == (word_p *) nullptr_catastrophic_seco)
        ErrorRSTop = 0;
    if(ErrornLAMBase == (word_p *) nullptr_catastrophic_seco)
        ErrornLAMBase = 0;

}

void rplCatchException()
{
    // SAVE THE EXCEPTIONS
    TrappedExceptions = Exceptions;     // THE ERROR HANDLER CAN KNOW THE EXCEPTIONS BY LOOKING AT THIS VARIABLE
    // ExceptionPointer STILL POINTS TO THE WORD THAT CAUSED THE EXCEPTION
    TrappedErrorCode = ErrorCode;

    Exceptions = 0;     // RESET THE EXCEPTIONS TO ALLOW HANDLER TO RUN

    // DO NOT CLEANUP ANYTHING OR REMOVE THE HANDLER
    rplProtectData();   // PROTECT THE USER DATA STACK WITHIN THE ERROR HANDLER
    rplPushRet(IPtr);   // PUSH THE OFFENDING OPCODE TO RESUME AFTER THE ERROR HANDLER FINISHED
    rplSetExceptionHandler((word_p) error_reenter_seco);

    IPtr = rplPeekRet(4) - 1; /*=ErrorHandler (original)*/ ;
    CurOpcode = 0;

}

// RAISE AN RPL EXCEPTION
void rplException(WORD exception)
{
    Exceptions |= exception;
    ExceptionPointer = IPtr;
}

// RAISE A HARDWARE RPL EXCEPTION
void rplHardException(WORD exception)
{
    HWExceptions |= exception;
}

// RAISE A LIBRARY ERROR EXCEPTION
// AND SET THE ERROR CODE TO THE GIVEN OPCODE
void rplError(WORD errorcode)
{
    Exceptions |= EX_ERRORCODE;
    ErrorCode = errorcode;
    ExceptionPointer = IPtr;

}

// CLEAR ERRORS AFTER THEY WERE HANDLED
void rplClearErrors()
{
    Exceptions = 0;
}

// BLAME A USER COMMAND, NOT A ROM INTERNAL COMMAND FOR AN ERROR
void rplBlameUserCommand()
{
    int32_t depth = rplDepthRet();
    int32_t idx = 1;
    word_p cmdpointer = ExceptionPointer;

    while(!rplIsTempObPointer(cmdpointer) && (idx <= depth)) {
        cmdpointer = rplPeekRet(idx);
        ++idx;
    }

    if(idx <= depth)
        ExceptionPointer = cmdpointer;

}

// BLAMES AN ERROR TO THE GIVEN COMMAND (WITHIN A USER PROGRAM OR OBJECT)
void rplBlameError(word_p command)
{
    ExceptionPointer = command;
}
