/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// MAIN RUNSTREAM MANAGEMENT FOR newRPL

WORD RPLLastOpcode;

extern const WORD dotsettings_ident[];
extern const WORD flags_ident[];
extern const WORD bkpoint_seco[];





// GET A HANDLER FOR A LIBRARY
// IT'S FAST FOR SYSTEM LIBRARIES, MUCH SLOWER FOR USER LIBS

LIBHANDLER rplGetLibHandler(BINT libnum)
{
    if(libnum<MAXLOWLIBS) return LowLibRegistry[libnum];
    if(libnum>MAXLIBNUMBER-MAXSYSHILIBS) return SysHiLibRegistry[libnum-(MAXLIBNUMBER+1-MAXSYSHILIBS)];
    // DO A BINARY SEARCH FOR USER FUNCTIONS OTHERWISE
    BINT lo=0;
    BINT hi=NumHiLibs-1;
    BINT x;
    do {
    x=(hi+lo)/2;
    if(HiLibNumbers[x]==libnum) return HiLibRegistry[x];
    if(HiLibNumbers[x]>libnum) hi=x;
    else lo=x;
    } while(hi-lo>1);
    if(HiLibNumbers[hi]==libnum) return HiLibRegistry[hi];

    // LIBRARY NOT FOUND
    // DO NOT THROW AN EXCEPTION
    // LET THE HIGHER LEVEL FUNCTION DO IT
    return 0;
}


// DECREASE THE LIBRARY NUMBER, TO THE NEXT VALID HANDLER
BINT rplGetNextLib(BINT libnum)
{
    if(libnum>MAXLIBNUMBER-MAXSYSHILIBS) {
        --libnum;
        while((libnum>MAXLIBNUMBER-MAXSYSHILIBS) && (!SysHiLibRegistry[libnum-(MAXLIBNUMBER+1-MAXSYSHILIBS)])) --libnum;
        if(libnum>MAXLIBNUMBER-MAXSYSHILIBS) return libnum;
        if(NumHiLibs) return HiLibNumbers[NumHiLibs-1];
        // OTHERWISE CONTINUE SCANNING THE LOW LIBS
        libnum=MAXLOWLIBS;
    }

    if(libnum>MAXLOWLIBS) {
    // DO A BINARY SEARCH FOR USER FUNCTIONS OTHERWISE
        if(NumHiLibs>0) {
            BINT lo=0;
            BINT hi=NumHiLibs-1;
            BINT x;
            do {
                x=(hi+lo)/2;
                if(HiLibNumbers[x]==libnum) {
                    if(x>0) return HiLibNumbers[x-1];
                    libnum=MAXLOWLIBS;
                    break;
                }
                if(HiLibNumbers[x]>libnum) hi=x;
                else lo=x;
            } while(hi-lo>1);
            if(HiLibNumbers[hi]==libnum) {
                if(hi>0) return HiLibNumbers[hi-1];
                libnum=MAXLOWLIBS;
            }
        } else libnum=MAXLOWLIBS;

    }

    if(libnum<=MAXLOWLIBS) {
        --libnum;
        while((libnum>=0) && (!LowLibRegistry[libnum])) --libnum;
        return libnum;
    }

    return -1;
}



// REMOVE ALL REGISTERED LIBRARIES AND INSTALL ONLY CORE LIBRARIES PROVIDED IN ROM
void rplClearLibraries()
{
    // CLEAR ALL INSTALLED LIBRARIES
    int count;
    for(count=0;count<MAXLOWLIBS;++count)
    {
        LowLibRegistry[count]=0;
    }
    for(count=0;count<MAXSYSHILIBS;++count)
    {
        SysHiLibRegistry[count]=0;
    }


    // CLEAR ALL USER LIBS
    for(count=0;count<MAXHILIBS;++count)
    {
        HiLibRegistry[count]=0;
        HiLibNumbers[count]=0;
    }
    NumHiLibs=0;

}


void rplInstallCoreLibraries()
{
    // INSTALL ALL ROM LIBRARIES FROM A NULL-TERMINATED LIST
    LIBHANDLER *libptr=(LIBHANDLER *)ROMLibs;
    while(*libptr) { rplInstallLibrary(*libptr); ++libptr; }

}


// INSTALL A LIBRARY HANDLER, RETURN 1 ON SUCCESS, 0 ON FAILURE

BINT rplInstallLibrary(LIBHANDLER handler)
{
    HALFWORD *listnumbers;

    if(!handler) return 0;
    WORD savedOpcode=CurOpcode;
    CurOpcode=OPCODE_LIBINSTALL;
    RetNum=-1;
    (*handler)();   // CALL THE HANDLER TO GET THE LIBRARY NUMBER IN RetNum;
    CurOpcode=savedOpcode;
    if(RetNum==OK_CONTINUE) {
    listnumbers=(HALFWORD *)LibraryList;
    while((*listnumbers)||(listnumbers==(HALFWORD *)LibraryList)) {

    if(*listnumbers<MAXLOWLIBS) {
        if(LowLibRegistry[*listnumbers]) {
         // LIBRARY NUMBER ALREADY TAKEN OR LIB ALREADY INSTALLED
            if(LowLibRegistry[*listnumbers]==handler) return 1;   // ALREADY INSTALLED
            savedOpcode=CurOpcode;
            CurOpcode=OPCODE_LIBREMOVE;
            RetNum=-1;
            (*handler)();   // CALL THE HANDLER TO INDICATE LIBRARY IS BEING REMOVED;
            CurOpcode=savedOpcode;
            return 0;
        }
        LowLibRegistry[*listnumbers]=handler;
        ++listnumbers;
        continue;
    }
    if(*listnumbers>MAXLIBNUMBER-MAXSYSHILIBS) {
        if(SysHiLibRegistry[*listnumbers-(MAXLIBNUMBER-MAXSYSHILIBS+1)]) {
         // LIBRARY NUMBER ALREADY TAKEN OR LIB ALREADY INSTALLED
            if(SysHiLibRegistry[*listnumbers-(MAXLIBNUMBER-MAXSYSHILIBS+1)]==handler) return 1;   // ALREADY INSTALLED
            savedOpcode=CurOpcode;
            CurOpcode=OPCODE_LIBREMOVE;
            RetNum=-1;
            (*handler)();   // CALL THE HANDLER TO INDICATE LIBRARY IS BEING REMOVED;
            CurOpcode=savedOpcode;
            return 0;
        }
        SysHiLibRegistry[*listnumbers-(MAXLIBNUMBER-MAXSYSHILIBS+1)]=handler;
        ++listnumbers;
        continue;
    }

    LIBHANDLER oldhan=rplGetLibHandler(*listnumbers);

    if(oldhan) {
     // LIBRARY NUMBER ALREADY TAKEN OR LIB ALREADY INSTALLED
        if(oldhan==handler) return 1;   // ALREADY INSTALLED
        savedOpcode=CurOpcode;
        CurOpcode=OPCODE_LIBREMOVE;
        RetNum=-1;
        (*handler)();   // CALL THE HANDLER TO INDICATE LIBRARY IS BEING REMOVED;
        CurOpcode=savedOpcode;
    }

    // ADD LIBRARY
    if(NumHiLibs>=MAXHILIBS) return 0;
    BINT *ptr=HiLibNumbers+NumHiLibs;
    LIBHANDLER *libptr=HiLibRegistry+NumHiLibs;

    while(ptr>HiLibNumbers) {
     if((UBINT)ptr[-1]>*listnumbers) {
        ptr[0]=ptr[-1];
        libptr[0]=libptr[-1];
        --ptr;
        --libptr;
     }
     else break;
    }
    *ptr=*listnumbers;
    *libptr=handler;
    ++NumHiLibs;
    ++listnumbers;
    continue;
}

return 1;
    }
    else return 0;  // HANDLER FAILED TO REPORT ANY LIBRARY NUMBERS
}


void rplRemoveLibrary(BINT number)
{
    if(number<0 || number>MAXLIBNUMBER)  return;
    if(number<MAXLOWLIBS) {
        LowLibRegistry[number]=0;
        return;
    }
    if(number>MAXLIBNUMBER-MAXSYSHILIBS) {
        SysHiLibRegistry[number-(MAXLIBNUMBER-MAXSYSHILIBS+1)]=0;
        return;
    }

    // ADD LIBRARY
    if(NumHiLibs<=0) return;
    BINT *ptr=HiLibNumbers,found=0;
    LIBHANDLER *libptr=HiLibRegistry;

    while(ptr<HiLibNumbers+NumHiLibs) {
     if(*ptr==number) { found=1; break; }
     ++ptr;
     ++libptr;
    }
    ++ptr;
    ++libptr;
    while(ptr<HiLibNumbers+NumHiLibs) {
        ptr[-1]=*ptr;
        libptr[-1]=*libptr;
        ++ptr;
        ++libptr;
    }
    if(found) --NumHiLibs;
}

// RETURNS 0 = FINISHED OK
// 1 = SOME ERROR, MAY NEED CLEANUP
// 2 = EXECUTION PAUSED DUE TO POWEROFF
BINT rplRun(void)
// TAKE THE NEXT WORD AND EXECUTE IT
{
    LIBHANDLER han;

    do {
    RPLLastOpcode=CurOpcode=*IPtr;

    han=rplGetLibHandler(LIBNUM(CurOpcode));

    if(han) (*han)();
    else {
        rplError(ERR_MISSINGLIBRARY);
        // INVALID OPCODE = END OF EXECUTION (CANNOT BE TRAPPED BY HANDLER)
        return NEEDS_CLEANUP;
    }
    Exceptions|=HWExceptions;   // COPY HARDWARE EXCEPTIONS INTO EXCEPTIONS AT THIS POINT TO AVOID
                                // STOPPING IN THE MIDDLE OF A COMMAND
    if(Exceptions) {
        if(HWExceptions) HWExceptions&=EX_HWBKPOINT;    // CLEAR ANY EXCEPTIONS EXCEPT CHECK FOR BREAKPOINTS

        if(Exceptions&EX_HWBKPOINT) {
            if(!HaltedIPtr && !(Exceptions&~(EX_HWBKPOINT|EX_HWBKPTSKIP))) {   // MAKE SURE WE DON'T HALT ALREADY HALTED CODE OR INTERFERE WITH OTHER EXCEPTIONS
                // CHECK FOR BREAKPOINT TRIGGERS!
                int trigger=0;
                if(GET_BKPOINTFLAG(0)&BKPT_ENABLED) {
                    if(GET_BKPOINTFLAG(0)&BKPT_LOCATION) {
                        WORDPTR nextopcode=IPtr+1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);
                        if((nextopcode>=BreakPt1Pointer)&&(nextopcode<rplSkipOb(BreakPt1Pointer))) {
                            if(!(Exceptions&EX_HWBKPTSKIP)) trigger=1;
                        }
                    } else if(!(Exceptions&EX_HWBKPTSKIP)) trigger=1;

                    if(trigger) {
                    if(GET_BKPOINTFLAG(0)&BKPT_COND) {
                        // HALT CURRENT PROGRAM
                        // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                        HaltedIPtr=IPtr+1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);
                        HaltedRSTop=RSTop;  // SAVE RETURN STACK POINTER
                        HaltednLAMBase=nLAMBase;
                        HaltedLAMTop=LAMTop;

                        // PAUSE ALL HARDWARE BREAKPOINTS UNTIL CONDITION IS EXECUTED
                        BreakPtFlags|=BKPT_ALLPAUSED;

                        // PREPARE TO EXECUTE THE CONDITION - MUST BE A SECONDARY

                        rplPushDataNoGrow(BreakPt1Arg);
                        IPtr=(WORDPTR)bkpoint_seco;
                        CurOpcode=0;
                        Exceptions=0; //    CLEAR ERRORS AND GO...


                    } else {

                        // HALT CURRENT PROGRAM
                        // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                        HaltedIPtr=IPtr+1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);
                        HaltedRSTop=RSTop;  // SAVE RETURN STACK POINTER
                        HaltednLAMBase=nLAMBase;
                        HaltedLAMTop=LAMTop;

                        Exceptions=EX_HALT;

                    }

                    }
                }

                // TODO: ADD SAME CODE FOR BREKPOINTS 1 AND 2 HERE

                if(!trigger && (GET_BKPOINTFLAG(1)&BKPT_ENABLED)) {
                    if(GET_BKPOINTFLAG(1)&BKPT_LOCATION) {
                        WORDPTR nextopcode=IPtr+1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);
                        if((nextopcode>=BreakPt2Pointer)&&(nextopcode<rplSkipOb(BreakPt2Pointer))) {
                            if(!(Exceptions&EX_HWBKPTSKIP)) trigger=1;
                        }
                    } else if(!(Exceptions&EX_HWBKPTSKIP)) trigger=1;

                    if(trigger) {
                    if(GET_BKPOINTFLAG(1)&BKPT_COND) {
                        // HALT CURRENT PROGRAM
                        // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                        HaltedIPtr=IPtr+1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);
                        HaltedRSTop=RSTop;  // SAVE RETURN STACK POINTER
                        HaltednLAMBase=nLAMBase;
                        HaltedLAMTop=LAMTop;

                        // PAUSE ALL HARDWARE BREAKPOINTS UNTIL CONDITION IS EXECUTED
                        BreakPtFlags|=BKPT_ALLPAUSED;

                        // PREPARE TO EXECUTE THE CONDITION - MUST BE A SECONDARY

                        rplPushDataNoGrow(BreakPt2Arg);
                        IPtr=(WORDPTR)bkpoint_seco;
                        CurOpcode=0;
                        Exceptions=0; //    CLEAR ERRORS AND GO...


                    } else {

                        // HALT CURRENT PROGRAM
                        // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                        HaltedIPtr=IPtr+1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);
                        HaltedRSTop=RSTop;  // SAVE RETURN STACK POINTER
                        HaltednLAMBase=nLAMBase;
                        HaltedLAMTop=LAMTop;

                        Exceptions=EX_HALT;

                    }

                    }
                }

                if(!trigger && (GET_BKPOINTFLAG(2)&BKPT_ENABLED)) {
                    if(GET_BKPOINTFLAG(2)&BKPT_LOCATION) {
                        WORDPTR nextopcode=IPtr+1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);
                        if((nextopcode>=BreakPt3Pointer)&&(nextopcode<rplSkipOb(BreakPt3Pointer))) {
                            if(!(Exceptions&EX_HWBKPTSKIP)) trigger=1;
                        }
                    } else if(!(Exceptions&EX_HWBKPTSKIP)) trigger=1;

                    if(trigger) {

                    // SINGLE STEP BREAKPOINT DISABLES ITSELF AFTER IT'S TRIGGERED
                    SET_BKPOINTFLAG(2,GET_BKPOINTFLAG(2)&(~BKPT_ENABLED));

                    if(GET_BKPOINTFLAG(2)&BKPT_COND) {
                        // HALT CURRENT PROGRAM
                        // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                        HaltedIPtr=IPtr+1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);
                        HaltedRSTop=RSTop;  // SAVE RETURN STACK POINTER
                        HaltednLAMBase=nLAMBase;
                        HaltedLAMTop=LAMTop;

                        // PAUSE ALL HARDWARE BREAKPOINTS UNTIL CONDITION IS EXECUTED
                        BreakPtFlags|=BKPT_ALLPAUSED;

                        // PREPARE TO EXECUTE THE CONDITION - MUST BE A SECONDARY

                        rplPushDataNoGrow(BreakPt3Arg);
                        IPtr=(WORDPTR)bkpoint_seco;
                        CurOpcode=0;
                        Exceptions=0; //    CLEAR ERRORS AND GO...


                    } else {

                        // HALT CURRENT PROGRAM
                        // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
                        HaltedIPtr=IPtr+1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);
                        HaltedRSTop=RSTop;  // SAVE RETURN STACK POINTER
                        HaltednLAMBase=nLAMBase;
                        HaltedLAMTop=LAMTop;

                        Exceptions=EX_HALT;

                    }

                    }
                }



            } else {
                // CHECK IF WE ARE DONE WITH THE BREAKPOINT CONDITION ROUTINE
                // WARNING!!!: DO NOT MODIFY bkpoint_seco WITHOUT FIXING THIS!!
                if(IPtr==bkpoint_seco+11) {
                    // WE REACHED THE END OF CODE STATEMENT, THEREFORE THE BREAKPOINT WAS TRIGGERED
                    // UN-PAUSE ALL HARDWARE BREAKPOINTS
                    BreakPtFlags&=~BKPT_ALLPAUSED;

                    // JUST STAY HALTED AND ISSUE A BREAKPOINT
                    Exceptions=EX_HALT;
                }

            }
            Exceptions&=~(EX_HWBKPOINT|EX_HWBKPTSKIP);
            if(!Exceptions) {
                IPtr+=1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);
                continue;
            }
        }



        // HARD EXCEPTIONS FIRST, DO NOT ALLOW ERROR HANDLERS TO CATCH THESE ONES
        if(Exceptions&EX_EXITRPL) {
            Exceptions=0;
            rplClearRStk(); // CLEAR THE RETURN STACK
            rplClearLAMs(); // CLEAR ALL LOCAL VARIABLES
            ErrorHandler=0;
            return CLEAN_RUN; // DON'T ALLOW HANDLER TO TRAP THIS EXCEPTION
        }


        if(Exceptions&EX_HWHALT) {
            // HARDWARE-CAUSED HALT
            // EMULATE THE HALT INSTRUCTION HERE
            if(!HaltedIPtr) { // CAN'T HALT WITHIN AN ALREADY HALTED PROGRAM!

            // SAVE THE ADDRESS OF THE NEXT INSTRUCTION
            HaltedIPtr=IPtr+1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);

            HaltedRSTop=RSTop;  // SAVE RETURN STACK POINTER
            HaltednLAMBase=nLAMBase;
            HaltedLAMTop=LAMTop;
            Exceptions|=EX_HALT;    // CONVERT TO A NORMAL HALT
            }
        }
        if(Exceptions&EX_HALT) { rplSkipNext(); return CODE_HALTED; } // PREPARE TO RESUME ON NEXT CALL
        if(Exceptions&EX_POWEROFF) { rplSkipNext(); return CODE_HALTED; } // PREPARE AUTORESUME

        if(ErrorHandler) {
            // ERROR WAS TRAPPED BY A HANDLER
            rplCatchException();
        }
        else {
            // THERE IS NO ERROR HANDLER --> UNTRAPPED ERROR
            // SAVE THE EXCEPTIONS FOR ERRN AND ERRM
            TrappedExceptions=Exceptions;   // THE ERROR HANDLER CAN KNOW THE EXCEPTIONS BY LOOKING AT THIS VARIABLE
                                            // ExceptionPointer STILL POINTS TO THE WORD THAT CAUSED THE EXCEPTION
            TrappedErrorCode=ErrorCode;

            return NEEDS_CLEANUP;      // END EXECUTION IMMEDIATELY IF AN UNHANDLED EXCEPTION IS THROWN
        }
    }

    // SKIP TO THE NEXT INSTRUCTION / OBJECT BASED ON CurOpcode
    // NOTICE THAT CurOpcode MIGHT BE MODIFIED BY A LIBRARY HANDLER TO ALTER THE FLOW
    IPtr+=1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);

    } while(1);

}


void rplDisableSingleStep()
{
    BreakPt3Arg=0;
    BreakPt3Pointer=0;
    SET_BKPOINTFLAG(2,0);
}

void rplEnableSingleStep()
{
    BreakPt3Arg=0;
    BreakPt3Pointer=0;
    SET_BKPOINTFLAG(2,BKPT_ENABLED);
}

// CLEANUP THE RUN ENVIRONMENT AFTER SOME ERRORS
// USED WHEN rplRun RETURNS NON-ZERO
void rplCleanup()
{
    if(ErrorHandler) ErrorHandler=0;
    rplClearRStk(); // CLEAR THE RETURN STACK
    rplClearLAMs(); // CLEAR ALL LOCAL VARIABLES
    HaltedIPtr=0;   // REMOVE ANY HALTED PROGRAMS
    rplDisableSingleStep(); // DISABLE SINGLE-STEP BREAKPOINT
}


// CHANGE THE CURRENT RUNSTREAM POINTER
void rplSetEntryPoint(WORDPTR ip)
{
    IPtr=ip;
}

// SKIPS THE GIVEN OBJECT

inline WORDPTR rplSkipOb(WORDPTR ip)
{
    return ip+1+((ISPROLOG(*ip))? OBJSIZE(*ip):0);
}




// SKIPS THE NEXT OBJECT IN THE RUNSTREAM
inline void rplSkipNext()
{
    IPtr+=1+((ISPROLOG(*IPtr))? OBJSIZE(*IPtr):0);
}

// GET THE OBJECT SIZE IN WORDS, INCLUDING THE PROLOG
inline WORD rplObjSize(WORDPTR ip)
{
    return 1+((ISPROLOG(*ip))? OBJSIZE(*ip):0);
}

// ALLOCATES MEMORY AND MAKES AN EXACT DUPLICATE OF object
// USES ONE SCRATCH POINTER
// RETURNS NULL IF ERROR
WORDPTR rplMakeNewCopy(WORDPTR object)
{
    WORD prolog=*object;
    BINT size=0;
    if(ISPROLOG(prolog)) size=OBJSIZE(prolog);
    ScratchPointer1=object;

    WORDPTR newobj=rplAllocTempOb(size);

    if(!newobj) return 0;

    memmovew((void *)newobj,(void *)ScratchPointer1,1+size);
    return newobj;
}


// COPIES AN OBJECT FROM src TO dest
// SAFE EVEN IF OBJECTS OVERLAP
void rplCopyObject(WORDPTR dest, WORDPTR src)
{
    WORD prolog=*src;
    BINT size;
    if(ISPROLOG(prolog)) size=OBJSIZE(prolog);
    else size=0;

    memmovew((void *)dest,(void *)src,1+size);
}


// INITIALIZE SYSTEM FLAGS TO DEFAULT VALUE
void rplResetSystemFlags()
{
    // CREATE AN EMPTY LIST OF SYSTEM FLAGS
    SystemFlags=rplAllocTempOb(16);  // FOR NOW: 128 SYSTEM FLAGS IN 2 BINTS WITH 64 BITS EACH

    if(!SystemFlags) return;

    // 128 SYSTEM FLAGS
    SystemFlags[0]=MKPROLOG(DOLIST,16);  // PUT ALL SYSTEM FLAGS ON A LIST
    SystemFlags[1]=MKPROLOG(HEXBINT,2); // USE A BINT PROLOG
    SystemFlags[2]=(63<<4)|(1<<29);             // FLAGS 0-31 ARE IN SystemFlags[2], DEFAULTS: WORDSIZE=63, DEG, COMMENTS=ON
    SystemFlags[3]=0;                   // FLAGS 32-63 ARE IN SystemFlags[3]
    SystemFlags[4]=MKPROLOG(HEXBINT,2);
    SystemFlags[5]=0;                   // FLAGS 64-95 ARE IN SystemFlags[5]
    SystemFlags[6]=0;                   // FLAGS 96-127 ARE IN SystemFlags[6]

    SystemFlags[7]=MKPROLOG(HEXBINT,2);
    SystemFlags[8]=MKMENUCODE(0,68,2,0);  // MenuCode1 IS IN SystemFlags[8] AND INITIALIZED TO THE MAIN MENU
    SystemFlags[9]=MKMENUCODE(1,0,0,0); // MenuCode2 IS IN SystemFlags[9] AND INITIALIZED TO VARS

    // 128 USER FLAGS
    SystemFlags[10]=MKPROLOG(HEXBINT,2);
    SystemFlags[11]=0;
    SystemFlags[12]=0;
    SystemFlags[13]=MKPROLOG(HEXBINT,2);
    SystemFlags[14]=0;
    SystemFlags[15]=0;

    // FUTURE EXPANSION: ADD MORE FLAGS HERE

    SystemFlags[16]=CMD_ENDLIST;         // CLOSE THE LIST

}

// INITIALIZE THE RPL MACHINE
void rplInit(void)
{

    int k,count;

    for(k=0;k<MAX_GC_PTRUPDATE;++k) GC_PTRUpdate[k]=0;  // CLEAN UP ALL GC SAFE POINTERS


    RStk=0;
    DStk=0;
    Directories=0;
    LAMs=0;
    TempOb=0;
    TempBlocks=0;
    TempBlocksEnd=0;

    IPtr=0;  // INSTRUCTION POINTER SHOULD BE SET LATER TO A VALID RUNSTREAM
    HaltedIPtr=0;
    CurOpcode=0; // CURRENT OPCODE (WORD)
    TempObSize=0;    // TOTAL SIZE OF TEMPOB
    TempBlocksSize=0;
    DStkSize=0;    // TOTAL SIZE OF DATA STACK
    RStkSize=0;    // TOTAL SIZE OF RETURN STACK
    LAMSize=0;
    HWExceptions=Exceptions=0;   // NO EXCEPTIONS RAISED
    BreakPtFlags=0;              // DISABLE ALL BREAKPOINTS
    ExceptionPointer=0;

    rplClearLibraries();

    rplInstallCoreLibraries();

    growRStk(1024);   // GET SOME MEMORY FOR RETURN STACK
    growDStk(1024);   // GET SOME MEMORY FOR DATA STACK
    growTempBlocks(1024); // GET SOME MEMORY FOR TEMPORARY OBJECT BLOCKS
    growLAMs(1024); // GET SOME MEMORY FOR LAM ENVIRONMENTS
    growDirs(1024); // MEMORY FOR ROOT DIRECTORY
    growTempOb(1024); // GET SOME MEMORY FOR TEMPORARY OBJECT STORAGE

    RSTop=RStk; // TOP OF THE RETURN STACK
    DSTop=DStk; // TOP OF THE DATA STACK
    DStkProtect=DStkBottom=DStk;   // UNPROTECTED STACK
    TempObEnd=TempOb;     // END OF USED TEMPOB
    LAMTop=LAMs;
    nLAMBase=LAMTop;
    TempBlocksEnd=TempBlocks;
    CurrentDir=Directories;
    DirsTop=Directories;
    ErrorHandler=0;       // INITIALLY THERE'S NO ERROR HANDLER, AN EXCEPTION WILL EXIT THE RPL LOOP



    // INITIALIZE THE HOME DIRECTORY
    WORDPTR *HomeDir=rplMakeNewDir();
    // INITIALIZE THE SETTINGS DIRECTORY
    SettingsDir=(WORDPTR)rplCreateNewDir((WORDPTR)dotsettings_ident,HomeDir);

    rplResetSystemFlags();

    rplStoreSettings((WORDPTR)flags_ident,SystemFlags);


    // INITIALIZE THE FLOATING POINT CONTEXT
    initContext(32);

    // SEED THE RNG
    rplRandomSeed(rplRandomNext()^0xbad1dea);


    // INITIALIZE THE REAL REGISTERS
    for(count=0;count<REAL_REGISTERS;++count)
    {
        RReg[count].data=allocRegister();
        RReg[count].flags=0;
        RReg[count].exp=0;
        RReg[count].len=1;
    }
    // INITIALIZE TEMP STORAGE FOR INTEGER TO REAL CONVERSION
    BINT2RealIdx=0;

    // SET ERROR TRAP HANDLER


}


// INITIALIZE RPL ENGINE AFTER A WARM START
// ASSUME ALL VARRIABLES IN MEMORY ARE VALID
void rplWarmInit(void)
{
    int count;

    IPtr=0;  // INSTRUCTION POINTER SHOULD BE SET LATER TO A VALID RUNSTREAM
    HaltedIPtr=0;
    CurOpcode=0; // CURRENT OPCODE (WORD)

    HWExceptions=Exceptions=0;   // NO EXCEPTIONS RAISED
    BreakPtFlags=0;              // DISABLE ALL BREAKPOINTS
    ExceptionPointer=0;

    RSTop=RStk; // CLEAR RETURN STACK
    DSTop=DStk; // CLEAR DATA STACK
    DStkProtect=DStkBottom=DStk;   // UNPROTECTED STACK
    LAMTop=LAMs;        // CLEAR ALL LAMS
    nLAMBase=LAMTop;    // CLEAR ALL LAM ENVIRONMENTS
    CurrentDir=Directories; // SET CURRENT DIRECTORY TO HOME
    ErrorHandler=0;       // INITIALLY THERE'S NO ERROR HANDLER, AN EXCEPTION WILL EXIT THE RPL LOOP


    // CLEAR ALL INSTALLED LIBRARIES
    rplClearLibraries();
    rplInstallCoreLibraries();


    // INITIALIZE THE FLOATING POINT CONTEXT
    initContext(32);

    // SEED THE RNG
    rplRandomSeed(rplRandomNext()^0xbad1dea);


    // INITIALIZE THE REAL REGISTERS
    for(count=0;count<REAL_REGISTERS;++count)
    {
        RReg[count].data=allocRegister();
        RReg[count].flags=0;
        RReg[count].exp=0;
        RReg[count].len=1;
    }
    // INITIALIZE TEMP STORAGE FOR INTEGER TO REAL CONVERSION
    BINT2RealIdx=0;

    // FINALLY, CHECK EXISTING MEMORY FOR DAMAGE AND REPAIR AUTOMATICALLY
    rplVerifyTempOb(1);
    rplVerifyDirectories(1);

    // VERIFY IF SETTINGS AND ROOT DIRECTORY ARE PROPERLY SET

    WORDPTR *settings=rplFindGlobal((WORDPTR)dotsettings_ident,0);
    WORDPTR *flags;
    if(settings) SettingsDir=settings[1];
    else {
        // CREATE A NEW SETTINGS DIRECTORY
        SettingsDir=(WORDPTR)rplCreateNewDir((WORDPTR)dotsettings_ident,CurrentDir);
    }

    flags=rplFindGlobalInDir((WORDPTR)flags_ident,rplFindDirbyHandle(SettingsDir),0);
    if(flags && ISLIST(*flags[1]) && (OBJSIZE(*flags[1])>=10)) SystemFlags=flags[1];
    else {
        rplResetSystemFlags();

        rplStoreSettings((WORDPTR)flags_ident,SystemFlags);

    }


}


// INITIALIZE RPL ENGINE AFTER A POWER OFF
// ASSUME ALL VARRIABLES IN MEMORY ARE VALID
void rplHotInit()
{
    int count;



    IPtr=0;  // INSTRUCTION POINTER SHOULD BE SET LATER TO A VALID RUNSTREAM
    // KEEP THE HALTED POINTER AS-IS, USE THEM TO KEEP RUNNING AFTER POWEROFF
    CurOpcode=0; // CURRENT OPCODE (WORD)

    Exceptions=0;   // NO EXCEPTIONS RAISED
    BreakPtFlags=0;              // DISABLE ALL BREAKPOINTS
    ExceptionPointer=0;

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

    // INITIALIZE THE REAL REGISTERS
    for(count=0;count<REAL_REGISTERS;++count)
    {
        RReg[count].data=allocRegister();
        RReg[count].flags=0;
        RReg[count].exp=0;
        RReg[count].len=1;
    }
    // INITIALIZE TEMP STORAGE FOR INTEGER TO REAL CONVERSION
    BINT2RealIdx=0;

    // FINALLY, CHECK EXISTING MEMORY FOR DAMAGE AND REPAIR AUTOMATICALLY
    rplVerifyTempOb(1);
    rplVerifyDirectories(1);

    // VERIFY IF SETTINGS AND ROOT DIRECTORY ARE PROPERLY SET

    WORDPTR *settings=rplFindGlobalInDir((WORDPTR)dotsettings_ident,Directories,0);
    WORDPTR *flags;
    if(settings) SettingsDir=settings[1];
    else {
        // CREATE A NEW SETTINGS DIRECTORY
        SettingsDir=(WORDPTR)rplCreateNewDir((WORDPTR)dotsettings_ident,Directories);
    }

    flags=rplFindGlobalInDir((WORDPTR)flags_ident,rplFindDirbyHandle(SettingsDir),0);
    if(flags && ISLIST(*flags[1]) && (OBJSIZE(*flags[1])>=10)) SystemFlags=flags[1];
    else {
        rplResetSystemFlags();
        rplStoreSettings((WORDPTR)flags_ident,SystemFlags);

    }


}



// FOR DEBUG ONLY: SHOW STATUS OF THE EXECUTION ENVIRONMENT
#ifndef NDEBUG
void rplShowRuntimeState(void)
{
    printf("Used memory:\n-------------\n");
    printf("TempOb=%d words (%d allocated)\n",(WORD)(TempObEnd-TempOb),(WORD)(TempObSize-TempOb));
    printf("TempBlocks=%d words (%d allocated)\n",(WORD)(TempBlocksEnd-TempBlocks),TempBlocksSize);
    printf("Data Stack=%d words (%d allocated)\n",(WORD)(DSTop-DStk),DStkSize);
    printf("Return Stack=%d words (%d allocated)\n",(WORD)(RSTop-RStk),RStkSize);
    printf("Local vars=%d words (%d allocated)\n",(WORD)(LAMTop-LAMs),LAMSize);
    printf("Directories=%d words (%d allocated)\n",(WORD)(DirsTop-Directories),DirSize);
    if(Context.alloc_bmp) printf("************* Real numbers Memory leak!!!\n");
}
#endif
