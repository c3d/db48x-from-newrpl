/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "cmdcodes.h"
#include "hal_api.h"
#include "libraries.h"
#include "newrpl.h"
#include "sysvars.h"

// MAIN RUNSTREAM MANAGEMENT FOR newRPL

WORD RPLLastOpcode;

extern const WORD dotsettings_ident[];
extern const WORD flags_ident[];
extern const WORD bkpoint_seco[];

// GET A HANDLER FOR A LIBRARY
// IT'S FAST FOR SYSTEM LIBRARIES, MUCH SLOWER FOR USER LIBS

LIBHANDLER rplGetLibHandler(int32_t libnum)
{
    if(libnum < MAXLOWLIBS)
        return LowLibRegistry[libnum];
    if(libnum > MAXLIBNUMBER - MAXSYSHILIBS)
        return SysHiLibRegistry[libnum - (MAXLIBNUMBER + 1 - MAXSYSHILIBS)];
    // DO A BINARY SEARCH FOR USER FUNCTIONS OTHERWISE
    int32_t lo = 0;
    int32_t hi = NumHiLibs - 1;
    int32_t x;
    do {
        x = (hi + lo) / 2;
        if(HiLibNumbers[x] == libnum)
            return HiLibRegistry[x];
        if(HiLibNumbers[x] > libnum)
            hi = x;
        else
            lo = x;
    }
    while(hi - lo > 1);
    if(HiLibNumbers[hi] == libnum)
        return HiLibRegistry[hi];

    // LIBRARY NOT FOUND
    // DO NOT THROW AN EXCEPTION
    // LET THE HIGHER LEVEL FUNCTION DO IT
    return 0;
}

// DECREASE THE LIBRARY NUMBER, TO THE NEXT VALID HANDLER
int32_t rplGetNextLib(int32_t libnum)
{
    if(libnum > MAXLIBNUMBER - MAXSYSHILIBS) {
        --libnum;
        while((libnum > MAXLIBNUMBER - MAXSYSHILIBS)
                && (!SysHiLibRegistry[libnum - (MAXLIBNUMBER + 1 -
                            MAXSYSHILIBS)]))
            --libnum;
        if(libnum > MAXLIBNUMBER - MAXSYSHILIBS)
            return libnum;
        if(NumHiLibs)
            return HiLibNumbers[NumHiLibs - 1];
        // OTHERWISE CONTINUE SCANNING THE LOW LIBS
        libnum = MAXLOWLIBS;
    }

    if(libnum > MAXLOWLIBS) {
        // DO A BINARY SEARCH FOR USER FUNCTIONS OTHERWISE
        if(NumHiLibs > 0) {
            int32_t lo = 0;
            int32_t hi = NumHiLibs - 1;
            int32_t x;
            do {
                x = (hi + lo) / 2;
                if(HiLibNumbers[x] == libnum) {
                    if(x > 0)
                        return HiLibNumbers[x - 1];
                    libnum = MAXLOWLIBS;
                    break;
                }
                if(HiLibNumbers[x] > libnum)
                    hi = x;
                else
                    lo = x;
            }
            while(hi - lo > 1);
            if(HiLibNumbers[hi] == libnum) {
                if(hi > 0)
                    return HiLibNumbers[hi - 1];
                libnum = MAXLOWLIBS;
            }
        }
        else
            libnum = MAXLOWLIBS;

    }

    if(libnum <= MAXLOWLIBS) {
        --libnum;
        while((libnum >= 0) && (!LowLibRegistry[libnum]))
            --libnum;
        return libnum;
    }

    return -1;
}

// REMOVE ALL REGISTERED LIBRARIES AND INSTALL ONLY CORE LIBRARIES PROVIDED IN ROM
void rplClearLibraries()
{
    // CLEAR ALL INSTALLED LIBRARIES
    int count;
    for(count = 0; count < MAXLOWLIBS; ++count) {
        LowLibRegistry[count] = 0;
    }
    for(count = 0; count < MAXSYSHILIBS; ++count) {
        SysHiLibRegistry[count] = 0;
    }

    // CLEAR ALL USER LIBS
    for(count = 0; count < MAXHILIBS; ++count) {
        HiLibRegistry[count] = 0;
        HiLibNumbers[count] = 0;
    }
    NumHiLibs = 0;

}

void rplInstallCoreLibraries()
{
    // INSTALL ALL ROM LIBRARIES FROM A NULL-TERMINATED LIST
    LIBHANDLER *libptr = (LIBHANDLER *) ROMLibs;
    while(*libptr) {
        rplInstallLibrary(*libptr);
        ++libptr;
    }

}

// INSTALL A LIBRARY HANDLER, RETURN 1 ON SUCCESS, 0 ON FAILURE

int32_t rplInstallLibrary(LIBHANDLER handler)
{
    uint16_t *listnumbers;

    if(!handler)
        return 0;
    WORD savedOpcode = CurOpcode;
    CurOpcode = OPCODE_LIBINSTALL;
    RetNum = -1;
    (*handler) ();      // CALL THE HANDLER TO GET THE LIBRARY NUMBER IN RetNum;
    CurOpcode = savedOpcode;
    if(RetNum == OK_CONTINUE) {
        listnumbers = (uint16_t *) LibraryList;
        while((*listnumbers) || (listnumbers == (uint16_t *) LibraryList)) {

            if(*listnumbers < MAXLOWLIBS) {
                if(LowLibRegistry[*listnumbers]) {
                    // LIBRARY NUMBER ALREADY TAKEN OR LIB ALREADY INSTALLED
                    if(LowLibRegistry[*listnumbers] == handler)
                        return 1;       // ALREADY INSTALLED
                    savedOpcode = CurOpcode;
                    CurOpcode = OPCODE_LIBREMOVE;
                    RetNum = -1;
                    (*handler) ();      // CALL THE HANDLER TO INDICATE LIBRARY IS BEING REMOVED;
                    CurOpcode = savedOpcode;
                    return 0;
                }
                LowLibRegistry[*listnumbers] = handler;
                ++listnumbers;
                continue;
            }
            if(*listnumbers > MAXLIBNUMBER - MAXSYSHILIBS) {
                if(SysHiLibRegistry[*listnumbers - (MAXLIBNUMBER -
                                MAXSYSHILIBS + 1)]) {
                    // LIBRARY NUMBER ALREADY TAKEN OR LIB ALREADY INSTALLED
                    if(SysHiLibRegistry[*listnumbers - (MAXLIBNUMBER -
                                    MAXSYSHILIBS + 1)] == handler)
                        return 1;       // ALREADY INSTALLED
                    savedOpcode = CurOpcode;
                    CurOpcode = OPCODE_LIBREMOVE;
                    RetNum = -1;
                    (*handler) ();      // CALL THE HANDLER TO INDICATE LIBRARY IS BEING REMOVED;
                    CurOpcode = savedOpcode;
                    return 0;
                }
                SysHiLibRegistry[*listnumbers - (MAXLIBNUMBER - MAXSYSHILIBS +
                            1)] = handler;
                ++listnumbers;
                continue;
            }

            LIBHANDLER oldhan = rplGetLibHandler(*listnumbers);

            if(oldhan) {
                // LIBRARY NUMBER ALREADY TAKEN OR LIB ALREADY INSTALLED
                if(oldhan == handler)
                    return 1;   // ALREADY INSTALLED
                savedOpcode = CurOpcode;
                CurOpcode = OPCODE_LIBREMOVE;
                RetNum = -1;
                (*handler) ();  // CALL THE HANDLER TO INDICATE LIBRARY IS BEING REMOVED;
                CurOpcode = savedOpcode;
            }

            // ADD LIBRARY
            if(NumHiLibs >= MAXHILIBS)
                return 0;
            int32_t *ptr = HiLibNumbers + NumHiLibs;
            LIBHANDLER *libptr = HiLibRegistry + NumHiLibs;

            while(ptr > HiLibNumbers) {
                if((uint32_t) ptr[-1] > *listnumbers) {
                    ptr[0] = ptr[-1];
                    libptr[0] = libptr[-1];
                    --ptr;
                    --libptr;
                }
                else
                    break;
            }
            *ptr = *listnumbers;
            *libptr = handler;
            ++NumHiLibs;
            ++listnumbers;
            continue;
        }

        return 1;
    }
    else
        return 0;       // HANDLER FAILED TO REPORT ANY LIBRARY NUMBERS
}

void rplRemoveLibrary(int32_t number)
{
    if(number < 0 || number > MAXLIBNUMBER)
        return;
    if(number < MAXLOWLIBS) {
        LIBHANDLER han = LowLibRegistry[number];
        // REMOVE ALL LIBRARIES REGISTERED WITH THAT SAME HANDLE
        if(han) {
            int32_t k;
            for(k = 0; k < MAXLOWLIBS; ++k)
                if(LowLibRegistry[k] == han)
                    LowLibRegistry[k] = 0;
        }
        return;
    }
    if(number > MAXLIBNUMBER - MAXSYSHILIBS) {
        LIBHANDLER han =
                SysHiLibRegistry[number - (MAXLIBNUMBER - MAXSYSHILIBS + 1)];
        // REMOVE ALL LIBRARIES REGISTERED WITH THAT SAME HANDLE
        if(han) {
            int32_t k;
            for(k = 0; k < MAXSYSHILIBS; ++k)
                if(SysHiLibRegistry[k] == han)
                    SysHiLibRegistry[k] = 0;
        }
        return;
    }

    if(NumHiLibs <= 0)
        return;
    int32_t *ptr = HiLibNumbers, found = 0;
    LIBHANDLER *libptr = HiLibRegistry;
    LIBHANDLER han;

    while(ptr < HiLibNumbers + NumHiLibs) {
        if(*ptr == number) {
            han = *libptr;
            found = 1;
            break;
        }
        ++ptr;
        ++libptr;
    }
    if(!found)
        return;
    ++ptr;
    ++libptr;
    while(ptr < HiLibNumbers + NumHiLibs) {
        if(*libptr == han) {
            ++found;
            continue;
        }
        ptr[-found] = *ptr;
        libptr[-found] = *libptr;
        ++ptr;
        ++libptr;
    }
    NumHiLibs -= found;
}

// RETURNS 0 = FINISHED OK
// 1 = SOME ERROR, MAY NEED CLEANUP
// 2 = EXECUTION PAUSED DUE TO POWEROFF
int32_t rplRun(void)
// TAKE THE NEXT WORD AND EXECUTE IT
{
    LIBHANDLER han;
    // CLEAR TEMPORARY SYSTEM FLAG ON EVERY SEPARATE EXECUTION
    rplClrSystemFlag(FL_FORCED_RAD);

        int32_t rpnmode =
                rplTestSystemFlag(FL_MODERPN) | (rplTestSystemFlag(FL_EXTENDEDRPN)
                << 1);
    if(!rpnmode) {
        do {
            RPLLastOpcode = CurOpcode = *IPtr;

            han = rplGetLibHandler(LIBNUM(CurOpcode));

            if(han)
                (*han) ();
            else {
                rplError(ERR_MISSINGLIBRARY);
                // INVALID OPCODE = END OF EXECUTION (CANNOT BE TRAPPED BY HANDLER)
                return NEEDS_CLEANUP;
            }
            Exceptions |= HWExceptions; // COPY HARDWARE EXCEPTIONS INTO EXCEPTIONS AT THIS POINT TO AVOID
            // STOPPING IN THE MIDDLE OF A COMMAND
            if(Exceptions) {

                if(HWExceptions)
                    HWExceptions &= EX_HWBKPOINT;       // CLEAR ANY EXCEPTIONS EXCEPT CHECK FOR BREAKPOINTS

                if(Exceptions & EX_HWBKPOINT) {
                    if(!HaltedIPtr && !(Exceptions & ~(EX_HWBKPOINT | EX_HWBKPTSKIP)))  // MAKE SURE WE DON'T HALT ALREADY HALTED CODE OR INTERFERE WITH OTHER EXCEPTIONS
                    {
                        // CHECK FOR BREAKPOINT TRIGGERS!
                        int trigger = 0;
                        if(GET_BKPOINTFLAG(0) & BKPT_ENABLED) {
                            if(GET_BKPOINTFLAG(0) & BKPT_LOCATION) {
                                word_p nextopcode =
                                        IPtr + 1 +
                                        ((IS_PROLOG(CurOpcode)) ?
                                        OBJSIZE(CurOpcode) : 0);
                                if((nextopcode >= BreakPt1Pointer)
                                        && (nextopcode <
                                            rplSkipOb(BreakPt1Pointer))) {
                                    if(!(Exceptions & EX_HWBKPTSKIP))
                                        trigger = 1;
                                }
                            }
                            else if(!(Exceptions & EX_HWBKPTSKIP))
                                trigger = 1;

                            if(trigger) {
                                if(GET_BKPOINTFLAG(0) & BKPT_COND) {
                                    // HALT CURRENT PROGRAM
                                    // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                                    HaltedIPtr =
                                            IPtr + 1 +
                                            ((IS_PROLOG(CurOpcode)) ?
                                            OBJSIZE(CurOpcode) : 0);
                                    HaltedRSTop = RSTop;        // SAVE RETURN STACK POINTER
                                    HaltednLAMBase = nLAMBase;
                                    HaltedLAMTop = LAMTop;

                                    // PAUSE ALL HARDWARE BREAKPOINTS UNTIL CONDITION IS EXECUTED
                                    BreakPtFlags |= BKPT_ALLPAUSED;

                                    // PREPARE TO EXECUTE THE CONDITION - MUST BE A SECONDARY

                                    rplPushDataNoGrow(BreakPt1Arg);
                                    IPtr = (word_p) bkpoint_seco;
                                    CurOpcode = 0;
                                    Exceptions = 0;     //    CLEAR ERRORS AND GO...

                                }
                                else {

                                    // HALT CURRENT PROGRAM
                                    // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                                    HaltedIPtr =
                                            IPtr + 1 +
                                            ((IS_PROLOG(CurOpcode)) ?
                                            OBJSIZE(CurOpcode) : 0);
                                    HaltedRSTop = RSTop;        // SAVE RETURN STACK POINTER
                                    HaltednLAMBase = nLAMBase;
                                    HaltedLAMTop = LAMTop;

                                    Exceptions = EX_HALT;

                                }

                            }
                        }

                        // TODO: ADD SAME CODE FOR BREKPOINTS 1 AND 2 HERE

                        if(!trigger && (GET_BKPOINTFLAG(1) & BKPT_ENABLED)) {
                            if(GET_BKPOINTFLAG(1) & BKPT_LOCATION) {
                                word_p nextopcode =
                                        IPtr + 1 +
                                        ((IS_PROLOG(CurOpcode)) ?
                                        OBJSIZE(CurOpcode) : 0);
                                if((nextopcode >= BreakPt2Pointer)
                                        && (nextopcode <
                                            rplSkipOb(BreakPt2Pointer))) {
                                    if(!(Exceptions & EX_HWBKPTSKIP))
                                        trigger = 1;
                                }
                            }
                            else if(!(Exceptions & EX_HWBKPTSKIP))
                                trigger = 1;

                            if(trigger) {
                                if(GET_BKPOINTFLAG(1) & BKPT_COND) {
                                    // HALT CURRENT PROGRAM
                                    // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                                    HaltedIPtr =
                                            IPtr + 1 +
                                            ((IS_PROLOG(CurOpcode)) ?
                                            OBJSIZE(CurOpcode) : 0);
                                    HaltedRSTop = RSTop;        // SAVE RETURN STACK POINTER
                                    HaltednLAMBase = nLAMBase;
                                    HaltedLAMTop = LAMTop;

                                    // PAUSE ALL HARDWARE BREAKPOINTS UNTIL CONDITION IS EXECUTED
                                    BreakPtFlags |= BKPT_ALLPAUSED;

                                    // PREPARE TO EXECUTE THE CONDITION - MUST BE A SECONDARY

                                    rplPushDataNoGrow(BreakPt2Arg);
                                    IPtr = (word_p) bkpoint_seco;
                                    CurOpcode = 0;
                                    Exceptions = 0;     //    CLEAR ERRORS AND GO...

                                }
                                else {

                                    // HALT CURRENT PROGRAM
                                    // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                                    HaltedIPtr =
                                            IPtr + 1 +
                                            ((IS_PROLOG(CurOpcode)) ?
                                            OBJSIZE(CurOpcode) : 0);
                                    HaltedRSTop = RSTop;        // SAVE RETURN STACK POINTER
                                    HaltednLAMBase = nLAMBase;
                                    HaltedLAMTop = LAMTop;

                                    Exceptions = EX_HALT;

                                }

                            }
                        }

                        if(!trigger && (GET_BKPOINTFLAG(2) & BKPT_ENABLED)) {
                            if(GET_BKPOINTFLAG(2) & BKPT_LOCATION) {
                                word_p nextopcode =
                                        IPtr + 1 +
                                        ((IS_PROLOG(CurOpcode)) ?
                                        OBJSIZE(CurOpcode) : 0);
                                if((nextopcode >= BreakPt3Pointer)
                                        && (nextopcode <
                                            rplSkipOb(BreakPt3Pointer))) {
                                    if(!(Exceptions & EX_HWBKPTSKIP))
                                        trigger = 1;
                                }
                            }
                            else if(!(Exceptions & EX_HWBKPTSKIP))
                                trigger = 1;

                            if(trigger) {

                                // SINGLE STEP BREAKPOINT DISABLES ITSELF AFTER IT'S TRIGGERED
                                SET_BKPOINTFLAG(2,
                                        GET_BKPOINTFLAG(2) & (~BKPT_ENABLED));

                                if(GET_BKPOINTFLAG(2) & BKPT_COND) {
                                    // HALT CURRENT PROGRAM
                                    // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                                    HaltedIPtr =
                                            IPtr + 1 +
                                            ((IS_PROLOG(CurOpcode)) ?
                                            OBJSIZE(CurOpcode) : 0);
                                    HaltedRSTop = RSTop;        // SAVE RETURN STACK POINTER
                                    HaltednLAMBase = nLAMBase;
                                    HaltedLAMTop = LAMTop;

                                    // PAUSE ALL HARDWARE BREAKPOINTS UNTIL CONDITION IS EXECUTED
                                    BreakPtFlags |= BKPT_ALLPAUSED;

                                    // PREPARE TO EXECUTE THE CONDITION - MUST BE A SECONDARY

                                    rplPushDataNoGrow(BreakPt3Arg);
                                    IPtr = (word_p) bkpoint_seco;
                                    CurOpcode = 0;
                                    Exceptions = 0;     //    CLEAR ERRORS AND GO...

                                }
                                else {

                                    // HALT CURRENT PROGRAM
                                    // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                                    HaltedIPtr =
                                            IPtr + 1 +
                                            ((IS_PROLOG(CurOpcode)) ?
                                            OBJSIZE(CurOpcode) : 0);
                                    HaltedRSTop = RSTop;        // SAVE RETURN STACK POINTER
                                    HaltednLAMBase = nLAMBase;
                                    HaltedLAMTop = LAMTop;

                                    Exceptions = EX_HALT;

                                }

                            }
                        }

                    }
                    else {
                        // CHECK IF WE ARE DONE WITH THE BREAKPOINT CONDITION ROUTINE
                        // WARNING!!!: DO NOT MODIFY bkpoint_seco WITHOUT FIXING THIS!!
                        if(IPtr == bkpoint_seco + 11) {
                            // WE REACHED THE END OF CODE STATEMENT, THEREFORE THE BREAKPOINT WAS TRIGGERED
                            // UN-PAUSE ALL HARDWARE BREAKPOINTS
                            BreakPtFlags &= ~BKPT_ALLPAUSED;

                            // JUST STAY HALTED AND ISSUE A BREAKPOINT
                            Exceptions = EX_HALT;
                        }

                    }
                    Exceptions &= ~(EX_HWBKPOINT | EX_HWBKPTSKIP);
                    if(!Exceptions) {
                        IPtr += 1 +
                                ((IS_PROLOG(CurOpcode)) ? OBJSIZE(CurOpcode) :
                                0);
                        continue;
                    }
                }

                // HARD EXCEPTIONS FIRST, DO NOT ALLOW ERROR HANDLERS TO CATCH THESE ONES
                if(Exceptions & EX_EXITRPL) {
                    Exceptions = 0;
                    rplClearRStk();     // CLEAR THE RETURN STACK
                    rplClearLAMs();     // CLEAR ALL LOCAL VARIABLES
                    ErrorHandler = 0;
                    return CLEAN_RUN;   // DON'T ALLOW HANDLER TO TRAP THIS EXCEPTION
                }

                if(Exceptions & EX_HWHALT) {
                    // HARDWARE-CAUSED HALT
                    // EMULATE THE HALT INSTRUCTION HERE
                    if(!HaltedIPtr)     // CAN'T HALT WITHIN AN ALREADY HALTED PROGRAM!
                    {

                        // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                        HaltedIPtr =
                                IPtr + 1 +
                                ((IS_PROLOG(CurOpcode)) ? OBJSIZE(CurOpcode) :
                                0);

                        HaltedRSTop = RSTop;    // SAVE RETURN STACK POINTER
                        HaltednLAMBase = nLAMBase;
                        HaltedLAMTop = LAMTop;
                        Exceptions |= EX_HALT;  // CONVERT TO A NORMAL HALT
                    }
                }
                if(Exceptions & EX_HALT) {
                    rplSkipNext();      // PREPARE TO RESUME ON NEXT CALL
                    return CODE_HALTED;
                }
                if(Exceptions & EX_POWEROFF) {
                    rplSkipNext();      // PREPARE AUTORESUME
                    return CODE_HALTED;
                }

                if(ErrorHandler) {
                    // ERROR WAS TRAPPED BY A HANDLER
                    rplCatchException();
                }
                else {
                    // THERE IS NO ERROR HANDLER --> UNTRAPPED ERROR
                    // SAVE THE EXCEPTIONS FOR ERRN AND ERRM
                    TrappedExceptions = Exceptions;     // THE ERROR HANDLER CAN KNOW THE EXCEPTIONS BY LOOKING AT THIS VARIABLE
                    // ExceptionPointer STILL POINTS TO THE WORD THAT CAUSED THE EXCEPTION
                    TrappedErrorCode = ErrorCode;

                    return NEEDS_CLEANUP;       // END EXECUTION IMMEDIATELY IF AN UNHANDLED EXCEPTION IS THROWN
                }
            }

            // SKIP TO THE NEXT INSTRUCTION / OBJECT BASED ON CurOpcode
            // NOTICE THAT CurOpcode MIGHT BE MODIFIED BY A LIBRARY HANDLER TO ALTER THE FLOW
            IPtr += 1 + ((IS_PROLOG(CurOpcode)) ? OBJSIZE(CurOpcode) : 0);

        }
        while(1);
    }
    else {
        if(!LastRegisterT)
            LastRegisterT = (word_p) zero_bint;
        if(DStkProtect!=DStkBottom) {
            // STACK IS NOT SETUP FOR RPN, NEEDS TO BE FILLED
            // PROVIDE RPN-MODE STACK BEHAVIOR
            int32_t nlevels = (rpnmode & 2) ? 8 : 4;
            if(DSTop-DStkBottom>=nlevels) DStkProtect=DSTop-nlevels;
            else DStkProtect=DStkBottom;
        }

        do {
            RPLLastOpcode = CurOpcode = *IPtr;

            han = rplGetLibHandler(LIBNUM(CurOpcode));

            if(han)
                (*han) ();
            else {
                rplError(ERR_MISSINGLIBRARY);
                // INVALID OPCODE = END OF EXECUTION (CANNOT BE TRAPPED BY HANDLER)
                return NEEDS_CLEANUP;
            }
            Exceptions |= HWExceptions; // COPY HARDWARE EXCEPTIONS INTO EXCEPTIONS AT THIS POINT TO AVOID
            // STOPPING IN THE MIDDLE OF A COMMAND
            if(Exceptions) {


                if(HWExceptions)
                    HWExceptions &= EX_HWBKPOINT;       // CLEAR ANY EXCEPTIONS EXCEPT CHECK FOR BREAKPOINTS

                if(Exceptions & EX_HWBKPOINT) {
                    if(!HaltedIPtr && !(Exceptions & ~(EX_HWBKPOINT | EX_HWBKPTSKIP)))  // MAKE SURE WE DON'T HALT ALREADY HALTED CODE OR INTERFERE WITH OTHER EXCEPTIONS
                    {
                        // CHECK FOR BREAKPOINT TRIGGERS!
                        int trigger = 0;
                        if(GET_BKPOINTFLAG(0) & BKPT_ENABLED) {
                            if(GET_BKPOINTFLAG(0) & BKPT_LOCATION) {
                                word_p nextopcode =
                                        IPtr + 1 +
                                        ((IS_PROLOG(CurOpcode)) ?
                                        OBJSIZE(CurOpcode) : 0);
                                if((nextopcode >= BreakPt1Pointer)
                                        && (nextopcode <
                                            rplSkipOb(BreakPt1Pointer))) {
                                    if(!(Exceptions & EX_HWBKPTSKIP))
                                        trigger = 1;
                                }
                            }
                            else if(!(Exceptions & EX_HWBKPTSKIP))
                                trigger = 1;

                            if(trigger) {
                                if(GET_BKPOINTFLAG(0) & BKPT_COND) {
                                    // HALT CURRENT PROGRAM
                                    // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                                    HaltedIPtr =
                                            IPtr + 1 +
                                            ((IS_PROLOG(CurOpcode)) ?
                                            OBJSIZE(CurOpcode) : 0);
                                    HaltedRSTop = RSTop;        // SAVE RETURN STACK POINTER
                                    HaltednLAMBase = nLAMBase;
                                    HaltedLAMTop = LAMTop;

                                    // PAUSE ALL HARDWARE BREAKPOINTS UNTIL CONDITION IS EXECUTED
                                    BreakPtFlags |= BKPT_ALLPAUSED;

                                    // PREPARE TO EXECUTE THE CONDITION - MUST BE A SECONDARY

                                    rplPushDataNoGrow(BreakPt1Arg);
                                    IPtr = (word_p) bkpoint_seco;
                                    CurOpcode = 0;
                                    Exceptions = 0;     //    CLEAR ERRORS AND GO...

                                }
                                else {

                                    // HALT CURRENT PROGRAM
                                    // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                                    HaltedIPtr =
                                            IPtr + 1 +
                                            ((IS_PROLOG(CurOpcode)) ?
                                            OBJSIZE(CurOpcode) : 0);
                                    HaltedRSTop = RSTop;        // SAVE RETURN STACK POINTER
                                    HaltednLAMBase = nLAMBase;
                                    HaltedLAMTop = LAMTop;

                                    Exceptions = EX_HALT;

                                }

                            }
                        }

                        // TODO: ADD SAME CODE FOR BREKPOINTS 1 AND 2 HERE

                        if(!trigger && (GET_BKPOINTFLAG(1) & BKPT_ENABLED)) {
                            if(GET_BKPOINTFLAG(1) & BKPT_LOCATION) {
                                word_p nextopcode =
                                        IPtr + 1 +
                                        ((IS_PROLOG(CurOpcode)) ?
                                        OBJSIZE(CurOpcode) : 0);
                                if((nextopcode >= BreakPt2Pointer)
                                        && (nextopcode <
                                            rplSkipOb(BreakPt2Pointer))) {
                                    if(!(Exceptions & EX_HWBKPTSKIP))
                                        trigger = 1;
                                }
                            }
                            else if(!(Exceptions & EX_HWBKPTSKIP))
                                trigger = 1;

                            if(trigger) {
                                if(GET_BKPOINTFLAG(1) & BKPT_COND) {
                                    // HALT CURRENT PROGRAM
                                    // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                                    HaltedIPtr =
                                            IPtr + 1 +
                                            ((IS_PROLOG(CurOpcode)) ?
                                            OBJSIZE(CurOpcode) : 0);
                                    HaltedRSTop = RSTop;        // SAVE RETURN STACK POINTER
                                    HaltednLAMBase = nLAMBase;
                                    HaltedLAMTop = LAMTop;

                                    // PAUSE ALL HARDWARE BREAKPOINTS UNTIL CONDITION IS EXECUTED
                                    BreakPtFlags |= BKPT_ALLPAUSED;

                                    // PREPARE TO EXECUTE THE CONDITION - MUST BE A SECONDARY

                                    rplPushDataNoGrow(BreakPt2Arg);
                                    IPtr = (word_p) bkpoint_seco;
                                    CurOpcode = 0;
                                    Exceptions = 0;     //    CLEAR ERRORS AND GO...

                                }
                                else {

                                    // HALT CURRENT PROGRAM
                                    // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                                    HaltedIPtr =
                                            IPtr + 1 +
                                            ((IS_PROLOG(CurOpcode)) ?
                                            OBJSIZE(CurOpcode) : 0);
                                    HaltedRSTop = RSTop;        // SAVE RETURN STACK POINTER
                                    HaltednLAMBase = nLAMBase;
                                    HaltedLAMTop = LAMTop;

                                    Exceptions = EX_HALT;

                                }

                            }
                        }

                        if(!trigger && (GET_BKPOINTFLAG(2) & BKPT_ENABLED)) {
                            if(GET_BKPOINTFLAG(2) & BKPT_LOCATION) {
                                word_p nextopcode =
                                        IPtr + 1 +
                                        ((IS_PROLOG(CurOpcode)) ?
                                        OBJSIZE(CurOpcode) : 0);
                                if((nextopcode >= BreakPt3Pointer)
                                        && (nextopcode <
                                            rplSkipOb(BreakPt3Pointer))) {
                                    if(!(Exceptions & EX_HWBKPTSKIP))
                                        trigger = 1;
                                }
                            }
                            else if(!(Exceptions & EX_HWBKPTSKIP))
                                trigger = 1;

                            if(trigger) {

                                // SINGLE STEP BREAKPOINT DISABLES ITSELF AFTER IT'S TRIGGERED
                                SET_BKPOINTFLAG(2,
                                        GET_BKPOINTFLAG(2) & (~BKPT_ENABLED));

                                if(GET_BKPOINTFLAG(2) & BKPT_COND) {
                                    // HALT CURRENT PROGRAM
                                    // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                                    HaltedIPtr =
                                            IPtr + 1 +
                                            ((IS_PROLOG(CurOpcode)) ?
                                            OBJSIZE(CurOpcode) : 0);
                                    HaltedRSTop = RSTop;        // SAVE RETURN STACK POINTER
                                    HaltednLAMBase = nLAMBase;
                                    HaltedLAMTop = LAMTop;

                                    // PAUSE ALL HARDWARE BREAKPOINTS UNTIL CONDITION IS EXECUTED
                                    BreakPtFlags |= BKPT_ALLPAUSED;

                                    // PREPARE TO EXECUTE THE CONDITION - MUST BE A SECONDARY

                                    rplPushDataNoGrow(BreakPt3Arg);
                                    IPtr = (word_p) bkpoint_seco;
                                    CurOpcode = 0;
                                    Exceptions = 0;     //    CLEAR ERRORS AND GO...

                                }
                                else {

                                    // HALT CURRENT PROGRAM
                                    // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                                    HaltedIPtr =
                                            IPtr + 1 +
                                            ((IS_PROLOG(CurOpcode)) ?
                                            OBJSIZE(CurOpcode) : 0);
                                    HaltedRSTop = RSTop;        // SAVE RETURN STACK POINTER
                                    HaltednLAMBase = nLAMBase;
                                    HaltedLAMTop = LAMTop;

                                    Exceptions = EX_HALT;

                                }

                            }
                        }

                    }
                    else {
                        // CHECK IF WE ARE DONE WITH THE BREAKPOINT CONDITION ROUTINE
                        // WARNING!!!: DO NOT MODIFY bkpoint_seco WITHOUT FIXING THIS!!
                        if(IPtr == bkpoint_seco + 11) {
                            // WE REACHED THE END OF CODE STATEMENT, THEREFORE THE BREAKPOINT WAS TRIGGERED
                            // UN-PAUSE ALL HARDWARE BREAKPOINTS
                            BreakPtFlags &= ~BKPT_ALLPAUSED;

                            // JUST STAY HALTED AND ISSUE A BREAKPOINT
                            Exceptions = EX_HALT;
                        }

                    }
                    Exceptions &= ~(EX_HWBKPOINT | EX_HWBKPTSKIP);
                    if(!Exceptions) {
                        IPtr += 1 +
                                ((IS_PROLOG(CurOpcode)) ? OBJSIZE(CurOpcode) :
                                0);
                        continue;
                    }
                }

                // HARD EXCEPTIONS FIRST, DO NOT ALLOW ERROR HANDLERS TO CATCH THESE ONES
                if(Exceptions & EX_EXITRPL) {
                    Exceptions = 0;
                    rplClearRStk();     // CLEAR THE RETURN STACK
                    rplClearLAMs();     // CLEAR ALL LOCAL VARIABLES
                    ErrorHandler = 0;
                    return CLEAN_RUN;   // DON'T ALLOW HANDLER TO TRAP THIS EXCEPTION
                }

                if(Exceptions & EX_HWHALT) {
                    // HARDWARE-CAUSED HALT
                    // EMULATE THE HALT INSTRUCTION HERE
                    if(!HaltedIPtr)     // CAN'T HALT WITHIN AN ALREADY HALTED PROGRAM!
                    {

                        // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                        HaltedIPtr =
                                IPtr + 1 +
                                ((IS_PROLOG(CurOpcode)) ? OBJSIZE(CurOpcode) :
                                0);

                        HaltedRSTop = RSTop;    // SAVE RETURN STACK POINTER
                        HaltednLAMBase = nLAMBase;
                        HaltedLAMTop = LAMTop;
                        Exceptions |= EX_HALT;  // CONVERT TO A NORMAL HALT
                    }
                }
                if(Exceptions & EX_HALT) {
                    rplSkipNext();      // PREPARE TO RESUME ON NEXT CALL
                    return CODE_HALTED;
                }
                if(Exceptions & EX_POWEROFF) {
                    rplSkipNext();      // PREPARE AUTORESUME
                    return CODE_HALTED;
                }

                if(ErrorHandler) {
                    // ERROR WAS TRAPPED BY A HANDLER
                    rplCatchException();
                }
                else {
                    // THERE IS NO ERROR HANDLER --> UNTRAPPED ERROR
                    // SAVE THE EXCEPTIONS FOR ERRN AND ERRM
                    TrappedExceptions = Exceptions;     // THE ERROR HANDLER CAN KNOW THE EXCEPTIONS BY LOOKING AT THIS VARIABLE
                    // ExceptionPointer STILL POINTS TO THE WORD THAT CAUSED THE EXCEPTION
                    TrappedErrorCode = ErrorCode;

                    return NEEDS_CLEANUP;       // END EXECUTION IMMEDIATELY IF AN UNHANDLED EXCEPTION IS THROWN
                }
            }

            // PROVIDE RPN-MODE STACK BEHAVIOR
            int32_t nlevels = (rpnmode & 2) ? 8 : 4;

            // RPN STACK CORRECTION
            if(rplDepthData() > nlevels)
                rplRemoveAtData(nlevels + 1, rplDepthData() - nlevels); // TRIM THE STACK IF MORE THAN 8 LEVELS
            if(rplDepthData() < nlevels)        // FILL THE STACK WITH THE T REGISTER IF LESS THAN 8 NUMBERS
            {
                int32_t offset=nlevels-rplDepthData();
                rplExpandStack(offset);
                // DISREGARD OF EXCEPTIONS, IF OUT OF MOEMORY WE SHOULD STILL HAVE ENOUGH SLACK IN THE STACK
                DSTop+=offset;
                int32_t k;
                DStkProtect=DSTop-nlevels;
                for(k = 1; k <= nlevels-offset; ++k)
                    DSTop[- k] = DSTop[-k-offset];
                for(; k <= nlevels; ++k)
                    DSTop[- k] = LastRegisterT;
            }
            LastRegisterT = DStkProtect[0];

            // SKIP TO THE NEXT INSTRUCTION / OBJECT BASED ON CurOpcode
            // NOTICE THAT CurOpcode MIGHT BE MODIFIED BY A LIBRARY HANDLER TO ALTER THE FLOW
            IPtr += 1 + ((IS_PROLOG(CurOpcode)) ? OBJSIZE(CurOpcode) : 0);

        }
        while(1);

    }

}

// EXECUTES ONE RPL COMMAND ATOMICALLY, RETURNS ONLY WHEN DONE
// RETURNS 0 = FINISHED OK
// 1 = SOME ERROR, MAY NEED CLEANUP
// 2 = EXECUTION PAUSED DUE TO POWEROFF
int32_t rplRunAtomic(WORD opcode)
// TAKE THE NEXT WORD AND EXECUTE IT
{
    word_p obj = rplAllocTempObLowMem(2);
    if(obj) {
        obj[0] = opcode;
        obj[1] = CMD_ENDOFCODE;
        obj[2] = CMD_QSEMI;     // THIS IS FOR SAFETY REASONS

        int32_t rsave, lamsave, nlambase, retvalue;
        WORD exceptsave, errcodesave;
        // PRESERVE VARIOUS STACK POINTERS
        rplPushDataNoGrow(obj); // PRESERVE POINTER IN CASE OF GC

        rplSetExceptionHandler(0);      // SAVE CURRENT EXCEPTION HANDLERS
        rplPushRet(IPtr);       // SAVE THE CURRENT INSTRUCTION POINTER

        rsave = RSTop - RStk;   // PROTECT THE RETURN STACK
        lamsave = LAMTop - LAMs;        // PROTECT LAM ENVIRONMENTS
        nlambase = nLAMBase - LAMs;

        obj = rplPopData();     // RESTORE POINTER

        rplSetEntryPoint(obj);

        do {
            retvalue = rplRun();
            if(retvalue) {
                if(Exceptions & (EX_POWEROFF | EX_HALT | EX_HWHALT |
                            EX_HWBKPOINT | EX_HWBKPTSKIP)) {
                    if(!HaltedIPtr)
                        break;

                    // UN-PAUSE ALL HARDWARE BREAKPOINTS. THIS IS NEEDED FOR breakpt_seco ONLY.
                    BreakPtFlags &= ~BKPT_ALLPAUSED;

                    // CONTINUE HALTED EXECUTION
                    if(RSTop >= HaltedRSTop) {
                        IPtr = HaltedIPtr - 1;
                        RSTop = HaltedRSTop;
                        if(LAMTop >= HaltedLAMTop)
                            LAMTop = HaltedLAMTop;
                        if(nLAMBase >= HaltednLAMBase)
                            nLAMBase = HaltednLAMBase;
                        HaltedIPtr = 0;
                        if(HWExceptions & EX_HWBKPOINT)
                            HWExceptions |= EX_HWBKPTSKIP;      // SKIP ONE SO AT LEAST IT EXECUTES ONE OPCODE
                    }
                    Exceptions = 0;
                    continue;
                }
                else    // STOP EXECUTION ON OTHER EXCEPTIONS
                    break;
            }
        }
        while(retvalue);

        // MANUAL RESTORE
        int32_t allgood = 1;
        if(RSTop >= RStk + rsave)
            RSTop = RStk + rsave;       // IF RSTop<RStk+rsave THE RETURN STACK WAS COMPLETELY CORRUPTED, SHOULD NEVER HAPPEN BUT...
        else {
            rplCleanup();
            allgood = 0;
        }
        if(LAMTop >= LAMs + lamsave)
            LAMTop = LAMs + lamsave;    // OTHERWISE THE LAM ENVIRONMENTS WERE DESTROYED, SHOULD NEVER HAPPEN BUT...
        else {
            rplCleanup();
            allgood = 0;
        }
        if(nLAMBase >= LAMs + nlambase)
            nLAMBase = LAMs + nlambase; // OTHERWISE THE LAM ENVIRONMENTS WERE DESTROYED, SHOULD NEVER HAPPEN BUT...
        else {
            rplCleanup();
            allgood = 0;
        }

        if(allgood && (Exceptions == EX_HALT) && (*(IPtr - 1) == CMD_ENDOFCODE)) {
            // EVERYTHING FINISHED EXECUTION WITHOUT ANY TROUBLES

            Exceptions = 0;

            // RESTORE THE IP POINTER
            IPtr = rplPopRet();

            // AND THE ERROR HANDLERS
            rplRemoveExceptionHandler();

            return 0;
        }

        // THERE WERE ERRORS DURING EXECUTION
        exceptsave = Exceptions;
        errcodesave = ErrorCode;

        Exceptions = 0;

        // RESTORE THE IP POINTER
        IPtr = rplPopRet();

        if(!Exceptions) {

            // SOME ERROR BUT NO STACK CORRUPTION, TRY TO RETURN CLEAN

            // AND THE ERROR HANDLERS
            rplRemoveExceptionHandler();

            Exceptions = exceptsave;
            ErrorCode = errcodesave;

        }

        // IF EVERYTHING WENT WELL, HERE WE HAVE THE SAME ENVIRONMENT AS BEFORE
        // IF SOMETHING GOT CORRUPTED, WE SHOULD HAVE AN INTERNAL EMPTY RSTACK ERROR
        return retvalue;

    }
    return 0;

}

void rplDisableSingleStep()
{
    BreakPt3Arg = 0;
    BreakPt3Pointer = 0;
    SET_BKPOINTFLAG(2, 0);
}

void rplEnableSingleStep()
{
    BreakPt3Arg = 0;
    BreakPt3Pointer = 0;
    SET_BKPOINTFLAG(2, BKPT_ENABLED);
}

// CLEANUP THE RUN ENVIRONMENT AFTER SOME ERRORS
// USED WHEN rplRun RETURNS NON-ZERO
void rplCleanup()
{
    if(ErrorHandler)
        ErrorHandler = 0;
    rplClearRStk();     // CLEAR THE RETURN STACK
    rplClearLAMs();     // CLEAR ALL LOCAL VARIABLES
    HaltedIPtr = 0;     // REMOVE ANY HALTED PROGRAMS
    rplDisableSingleStep();     // DISABLE SINGLE-STEP BREAKPOINT
}

// CHANGE THE CURRENT RUNSTREAM POINTER
void rplSetEntryPoint(word_p ip)
{
    IPtr = ip;
}

// SKIPS THE GIVEN OBJECT

inline word_p rplSkipOb(word_p ip)
{
    return ip + 1 + ((IS_PROLOG(*ip)) ? OBJSIZE(*ip) : 0);
}

// SKIPS THE NEXT OBJECT IN THE RUNSTREAM
inline void rplSkipNext()
{
    IPtr += 1 + ((IS_PROLOG(*IPtr)) ? OBJSIZE(*IPtr) : 0);
}

// GET THE OBJECT SIZE IN WORDS, INCLUDING THE PROLOG
inline WORD rplObjSize(word_p ip)
{
    return 1 + ((IS_PROLOG(*ip)) ? OBJSIZE(*ip) : 0);
}

// ALLOCATES MEMORY AND MAKES AN EXACT DUPLICATE OF object
// USES ONE SCRATCH POINTER
// RETURNS NULL IF ERROR
word_p rplMakeNewCopy(word_p object)
{
    WORD prolog = *object;
    int32_t size = 0;
    if(IS_PROLOG(prolog))
        size = OBJSIZE(prolog);
    ScratchPointer1 = object;

    word_p newobj = rplAllocTempOb(size);

    if(!newobj)
        return 0;

    memmovew((void *)newobj, (void *)ScratchPointer1, 1 + size);
    return newobj;
}

// COPIES AN OBJECT FROM src TO dest
// SAFE EVEN IF OBJECTS OVERLAP
void rplCopyObject(word_p dest, word_p src)
{
    WORD prolog = *src;
    int32_t size;
    if(IS_PROLOG(prolog))
        size = OBJSIZE(prolog);
    else
        size = 0;

    memmovew((void *)dest, (void *)src, 1 + size);
}

// INITIALIZE SYSTEM FLAGS TO DEFAULT VALUE
void rplResetSystemFlags()
{
    // CREATE AN EMPTY BINDATA OF SYSTEM FLAGS
    SystemFlags = rplAllocTempOb(8);    // FOR NOW: 128 SYSTEM FLAGS IN 4 WORDS WITH 32 BITS EACH, PLUS MENU CODE WORDS

    if(!SystemFlags)
        return;

    // 128 SYSTEM FLAGS
    SystemFlags[0] = MK_PROLOG(DOBINDATA, 8);    // PUT ALL SYSTEM FLAGS ON A LIST
    SystemFlags[1] = (63 << 4) | (1 << 29);     // FLAGS 0-31 ARE IN SystemFlags[1], DEFAULTS: WORDSIZE=63, DEG, COMMENTS=ON
    SystemFlags[2] = 0; // FLAGS 32-63 ARE IN SystemFlags[2]
    SystemFlags[3] = 0; // FLAGS 64-95 ARE IN SystemFlags[3]
    SystemFlags[4] = 0; // FLAGS 96-127 ARE IN SystemFlags[4]

    SystemFlags[5] = MK_MENU_CODE(0, 68, 2, 0);   // MenuCode1 IS IN SystemFlags[5] AND INITIALIZED TO THE MAIN MENU
    SystemFlags[6] = MK_MENU_CODE(1, 0, 0, 0);    // MenuCode2 IS IN SystemFlags[6] AND INITIALIZED TO VARS
    SystemFlags[7] = 0; // LIBID OF Menu1
    SystemFlags[8] = 0; // LIBID OF Menu2

    // FUTURE EXPANSION: ADD MORE FLAGS HERE

}

void rplInitMemoryAllocator()
{
    Context.alloc_bmp = EMPTY_STORAGEBMP;

    int count;
    // INITIALIZE THE REAL REGISTERS
    for(count = 0; count < REAL_REGISTERS; ++count) {
        RReg[count].data = allocRegister();
        RReg[count].flags = 0;
        RReg[count].exp = 0;
        RReg[count].len = 1;
    }
    // INITIALIZE TEMP STORAGE FOR INTEGER TO REAL CONVERSION
    int32_t2RealIdx = 0;

    // INITIALIZE MEMORY ALLOCATOR FOR OTHER USES
    init_simpalloc();

}

// INITIALIZE THE RPL MACHINE
void rplInit(void)
{

    int k;

    for(k = 0; k < MAX_GC_PTRUPDATE; ++k)
        GC_PTRUpdate[k] = 0;    // CLEAN UP ALL GC SAFE POINTERS

    RStk = 0;
    DStk = 0;
    Directories = 0;
    LAMs = 0;
    TempOb = 0;
    TempBlocks = 0;
    TempBlocksEnd = 0;

    IPtr = 0;   // INSTRUCTION POINTER SHOULD BE SET LATER TO A VALID RUNSTREAM
    HaltedIPtr = 0;
    CurOpcode = 0;      // CURRENT OPCODE (WORD)
    TempObSize = 0;     // TOTAL SIZE OF TEMPOB
    TempBlocksSize = 0;
    DStkSize = 0;       // TOTAL SIZE OF DATA STACK
    RStkSize = 0;       // TOTAL SIZE OF RETURN STACK
    LAMSize = 0;
    HWExceptions = Exceptions = 0;      // NO EXCEPTIONS RAISED
    BreakPtFlags = 0;   // DISABLE ALL BREAKPOINTS
    ExceptionPointer = 0;

    rplClearLibraries();

    rplInstallCoreLibraries();

    growRStk(1024);     // GET SOME MEMORY FOR RETURN STACK
    growDStk(1024);     // GET SOME MEMORY FOR DATA STACK
    growTempBlocks(1024);       // GET SOME MEMORY FOR TEMPORARY OBJECT BLOCKS
    growLAMs(1024);     // GET SOME MEMORY FOR LAM ENVIRONMENTS
    growDirs(1024);     // MEMORY FOR ROOT DIRECTORY
    growTempOb(1024);   // GET SOME MEMORY FOR TEMPORARY OBJECT STORAGE

    RSTop = RStk;       // TOP OF THE RETURN STACK
    DSTop = DStk;       // TOP OF THE DATA STACK
    DStkProtect = DStkBottom = DStk;    // UNPROTECTED STACK
    TempObEnd = TempOb; // END OF USED TEMPOB
    LAMTop = LAMs;
    nLAMBase = LAMTop;
    TempBlocksEnd = TempBlocks;
    CurrentDir = Directories;
    DirsTop = Directories;
    ErrorHandler = 0;   // INITIALLY THERE'S NO ERROR HANDLER, AN EXCEPTION WILL EXIT THE RPL LOOP

    // INITIALIZE THE HOME DIRECTORY
    word_p *HomeDir = rplMakeNewDir();
    // INITIALIZE THE SETTINGS DIRECTORY
    SettingsDir =
            (word_p) rplCreateNewDir((word_p) dotsettings_ident, HomeDir);

    rplResetSystemFlags();

    rplStoreSettings((word_p) flags_ident, SystemFlags);

    // INITIALIZE THE FLOATING POINT CONTEXT
    initContext(32);

    // SEED THE RNG
    rplRandomSeed(rplRandomNext() ^ 0xbad1dea);

    // SET ERROR TRAP HANDLER

    // RESET ALL USER REGISTERS TO zero_bint

    for(k = GC_UserRegisters - GC_PTRUpdate; k < MAX_GC_PTRUPDATE; ++k)
        GC_PTRUpdate[k] = (word_p) zero_bint;

}

// INITIALIZE RPL ENGINE AFTER A WARM START
// ASSUME ALL VARRIABLES IN MEMORY ARE VALID
void rplWarmInit(void)
{

    IPtr = 0;   // INSTRUCTION POINTER SHOULD BE SET LATER TO A VALID RUNSTREAM
    HaltedIPtr = 0;
    CurOpcode = 0;      // CURRENT OPCODE (WORD)

    HWExceptions = Exceptions = 0;      // NO EXCEPTIONS RAISED
    BreakPtFlags = 0;   // DISABLE ALL BREAKPOINTS
    ExceptionPointer = 0;

    RSTop = RStk;       // CLEAR RETURN STACK
    DSTop = DStk;       // CLEAR DATA STACK
    DStkProtect = DStkBottom = DStk;    // UNPROTECTED STACK
    LAMTop = LAMs;      // CLEAR ALL LAMS
    nLAMBase = LAMTop;  // CLEAR ALL LAM ENVIRONMENTS
    CurrentDir = Directories;   // SET CURRENT DIRECTORY TO HOME
    ErrorHandler = 0;   // INITIALLY THERE'S NO ERROR HANDLER, AN EXCEPTION WILL EXIT THE RPL LOOP

    // CLEAR ALL INSTALLED LIBRARIES
    rplClearLibraries();
    rplInstallCoreLibraries();

    // INITIALIZE THE FLOATING POINT CONTEXT
    initContext(32);

    // SEED THE RNG
    rplRandomSeed(rplRandomNext() ^ 0xbad1dea);

    // FINALLY, CHECK EXISTING MEMORY FOR DAMAGE AND REPAIR AUTOMATICALLY
    rplVerifyTempOb(1);
    rplVerifyDirectories(1);

    // VERIFY IF SETTINGS AND ROOT DIRECTORY ARE PROPERLY SET

    word_p *settings = rplFindGlobal((word_p) dotsettings_ident, 0);
    word_p *flags;
    if(settings)
        SettingsDir = settings[1];
    else {
        // CREATE A NEW SETTINGS DIRECTORY
        SettingsDir =
                (word_p) rplCreateNewDir((word_p) dotsettings_ident,
                CurrentDir);
    }

    flags = rplFindGlobalInDir((word_p) flags_ident,
            rplFindDirbyHandle(SettingsDir), 0);
    if(flags && ISBINDATA(*flags[1]) && (OBJSIZE(*flags[1]) >= 8))
        SystemFlags = flags[1];
    else {
        // EXISTING FLAGS ARE NOT VALID
        rplResetSystemFlags();

        if(flags && ISLIST(*flags[1])) {
            // CONVERT FLAGS STORED AS THE OLD LIST FORMAT TO THE NEW BINDATA FORMAT

            int32_t nitems = rplListLength(flags[1]);
            if(nitems >= 4) {
                // IT ALL CHECKS OUT, DO THE MAGIC:

                uint64_t value;
                word_p nptr = SystemFlags + 1; // DATA OF THE FIRST 64-BIT INTEGER
                word_p numptr;
                uint64_t *uptr;
                int32_t k;
                for(k = 1; k <= 4; ++k) {
                    numptr = rplGetListElement(flags[1], k);
                    if(numptr && ISint32_t(*numptr))
                        value = rplReadint32_t(numptr);
                    else
                        value = 0;
                    uptr = (uint64_t *) nptr;
                    *uptr = value;
                    nptr += 2;
                }

            }
        }

        rplStoreSettings((word_p) flags_ident, SystemFlags);

    }

    // RESET ALL USER REGISTERS TO zero_bint

    int32_t k;
    for(k = GC_UserRegisters - GC_PTRUpdate; k < MAX_GC_PTRUPDATE; ++k)
        GC_PTRUpdate[k] = (word_p) zero_bint;

}

// INITIALIZE RPL ENGINE AFTER A POWER OFF
// ASSUME ALL VARRIABLES IN MEMORY ARE VALID
void rplHotInit()
{

    IPtr = 0;   // INSTRUCTION POINTER SHOULD BE SET LATER TO A VALID RUNSTREAM
    // KEEP THE HALTED POINTER AS-IS, USE THEM TO KEEP RUNNING AFTER POWEROFF
    CurOpcode = 0;      // CURRENT OPCODE (WORD)

    HWExceptions = Exceptions = 0;      // NO EXCEPTIONS RAISED
    BreakPtFlags = 0;   // DISABLE ALL BREAKPOINTS
    ExceptionPointer = 0;

    //RSTop=RStk; // CLEAR RETURN STACK
    // KEEP DATA STACK INTACT
    //DSTop=DStk; // CLEAR DATA STACK
    //DStkProtect=DStkBottom=DStk;   // UNPROTECTED STACK
    //LAMTop=LAMs;        // CLEAR ALL LAMS
    //nLAMBase=LAMTop;    // CLEAR ALL LAM ENVIRONMENTS

    // KEEP CURRENT DIR
    //CurrentDir=Directories; // SET CURRENT DIRECTORY TO HOME
    //ErrorHandler=0;       // INITIALLY THERE'S NO ERROR HANDLER, AN EXCEPTION WILL EXIT THE RPL LOOP

    // CLEAR ALL INSTALLED LIBRARIES
    rplClearLibraries();
    rplInstallCoreLibraries();

    // RE-INITIALIZE THE FLOATING POINT CONTEXT BUT KEEP CURRENT PRECISION
    initContext(Context.precdigits);

    // FINALLY, CHECK EXISTING MEMORY FOR DAMAGE AND REPAIR AUTOMATICALLY
    rplVerifyTempOb(1);
    rplVerifyDirectories(1);

    // VERIFY IF SETTINGS AND ROOT DIRECTORY ARE PROPERLY SET

    word_p *settings =
            rplFindGlobalInDir((word_p) dotsettings_ident, Directories, 0);
    word_p *flags;
    if(settings)
        SettingsDir = settings[1];
    else {
        // CREATE A NEW SETTINGS DIRECTORY
        SettingsDir =
                (word_p) rplCreateNewDir((word_p) dotsettings_ident,
                Directories);
    }

    flags = rplFindGlobalInDir((word_p) flags_ident,
            rplFindDirbyHandle(SettingsDir), 0);
    if(flags && ISBINDATA(*flags[1]) && (OBJSIZE(*flags[1]) >= 8))
        SystemFlags = flags[1];
    else {
        // EXISTING FLAGS ARE NOT VALID
        rplResetSystemFlags();

        if(flags && ISLIST(*flags[1])) {
            // CONVERT FLAGS STORED AS THE OLD LIST FORMAT TO THE NEW BINDATA FORMAT

            int32_t nitems = rplListLength(flags[1]);
            if(nitems >= 4) {
                // IT ALL CHECKS OUT, DO THE MAGIC:

                uint64_t value;
                word_p nptr = SystemFlags + 1; // DATA OF THE FIRST 64-BIT INTEGER
                word_p numptr;
                uint64_t *uptr;
                int32_t k;
                for(k = 1; k <= 4; ++k) {
                    numptr = rplGetListElement(flags[1], k);
                    if(numptr && ISint32_t(*numptr))
                        value = rplReadint32_t(numptr);
                    else
                        value = 0;
                    uptr = (uint64_t *) nptr;
                    *uptr = value;
                    nptr += 2;
                }

            }
        }

        rplStoreSettings((word_p) flags_ident, SystemFlags);

    }

}

// FOR DEBUG ONLY: SHOW STATUS OF THE EXECUTION ENVIRONMENT
#ifndef NDEBUG
void rplShowRuntimeState(void)
{
    printf("Used memory:\n-------------\n");
    printf("TempOb=%d words (%d allocated)\n", (WORD) (TempObEnd - TempOb),
            (WORD) (TempObSize - TempOb));
    printf("TempBlocks=%d words (%d allocated)\n",
            (WORD) (TempBlocksEnd - TempBlocks), TempBlocksSize);
    printf("Data Stack=%d words (%d allocated)\n", (WORD) (DSTop - DStk),
            DStkSize);
    printf("Return Stack=%d words (%d allocated)\n", (WORD) (RSTop - RStk),
            RStkSize);
    printf("Local vars=%d words (%d allocated)\n", (WORD) (LAMTop - LAMs),
            LAMSize);
    printf("Directories=%d words (%d allocated)\n",
            (WORD) (DirsTop - Directories), DirSize);
    if(Context.alloc_bmp)
        printf("************* Real numbers Memory leak!!!\n");
}
#endif
