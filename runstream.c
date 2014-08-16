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
#define EXIT_LOOP -1000

BINT64 RPLTicks=0;


// GET A HANDLER FOR A LIBRARY
// IT'S FAST FOR SYSTEM LIBRARIES, MUCH SLOWER FOR USER LIBS

LIBHANDLER rplGetLibHandler(BINT libnum)
{
    if(libnum<MAXLOWLIBS) return LowLibRegistry[libnum];
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



void rplRun(void)
// TAKE THE NEXT WORD AND EXECUTE IT
{
    LIBHANDLER han;

    do {
    CurOpcode=*IPtr;

    if(CurOpcode==CMD_EXITRPL)
        return;  // END OF EXECUTION

    han=rplGetLibHandler(LIBNUM(CurOpcode));

    if(han) (*han)();
    if(Exceptions) {
        if(ErrorHandler) {
            // ERROR WAS TRAPPED BY A HANDLER
            rplCatchException();
        }
        else {
            // THERE IS NO ERROR HANDLER --> UNTRAPPED ERROR
                rplClearRStk(); // CLEAR THE RETURN STACK
                rplClearLAMs(); // CLEAR ALL LOCAL VARIABLES

            return;      // END EXECUTION IMMEDIATELY IF AN UNHANDLED EXCEPTION IS THROWN
        }
    }

    ++RPLTicks;

    // SKIP TO THE NEXT INSTRUCTION / OBJECT BASED ON CurOpcode
    // NOTICE THAT CurOpcode MIGHT BE MODIFIED BY A LIBRARY HANDLER TO ALTER THE FLOW
    IPtr+=1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);

    } while(han);

    Exceptions=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    // INVALID OPCODE = END OF EXECUTION.
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


// COPIES AN OBJECT FROM src TO dest
void rplCopyObject(WORDPTR dest, WORDPTR src)
{
    WORD prolog=*src;
    BINT size;
    if(ISPROLOG(prolog)) size=OBJSIZE(prolog);
    else size=0;

    memmove((void *)dest,(void *)src,4+(size<<2));
}


extern LIBHANDLER ROMLibs[];
// INITIALIZE THE RPL MACHINE
void rplInit(void)
{

    RStk=0;
    DStk=0;
    Directories=0;
    LAMs=0;
    TempOb=0;
    TempBlocks=0;

    IPtr=0;  // INSTRUCTION POINTER SHOULD BE SET LATER TO A VALID RUNSTREAM
    CurOpcode=0; // CURRENT OPCODE (WORD)
    TempObSize=0;    // TOTAL SIZE OF TEMPOB
    TempBlocksSize=0;
    DStkSize=0;    // TOTAL SIZE OF DATA STACK
    RStkSize=0;    // TOTAL SIZE OF RETURN STACK
    LAMSize=0;
    Exceptions=0;   // NO EXCEPTIONS RAISED
    ExceptionPointer=0;




    growRStk(1024);   // GET SOME MEMORY FOR RETURN STACK
    growDStk(1024);   // GET SOME MEMORY FOR DATA STACK
    growTempBlocks(1024); // GET SOME MEMORY FOR TEMPORARY OBJECT BLOCKS
    growLAMs(1024); // GET SOME MEMORY FOR LAM ENVIRONMENTS
    growDirs(1024); // MEMORY FOR ROOT DIRECTORY
    growTempOb(1024); // GET SOME MEMORY FOR TEMPORARY OBJECT STORAGE

    RSTop=RStk; // TOP OF THE RETURN STACK
    DSTop=DStk; // TOP OF THE DATA STACK
    DStkProtect=DStk;   // UNPROTECTED STACK
    TempObEnd=TempOb;     // END OF USED TEMPOB
    LAMTop=LAMs;
    nLAMBase=LAMTop;
    TempBlocksEnd=TempBlocks;
    CurrentDir=Directories;
    DirsTop=Directories;
    ErrorHandler=0;       // INITIALLY THERE'S NO ERROR HANDLER, AN EXCEPTION WILL EXIT THE RPL LOOP

    // CLEAR ALL INSTALLED LIBRARIES
    int count;
    for(count=0;count<MAXLOWLIBS;++count)
    {
        LowLibRegistry[count]=0;
    }

    // CLEAR ALL USER LIBS
    for(count=0;count<MAXHILIBS;++count)
    {
        HiLibRegistry[count]=0;
        HiLibNumbers[count]=0;
    }
    NumHiLibs=0;



    // INSTALL SYSTEM LIBRARIES
    for(count=0; (count<MAXLOWLIBS) && (ROMLibs[count]);++count)
    {
        LowLibRegistry[count]=ROMLibs[count];
    }

    // INSTALL SYSTEM HILIBS
    for(count=0; (count<MAXHILIBS) && (ROMLibs2[count]);++count)
    {
        HiLibRegistry[count]=ROMLibs2[count];
        HiLibNumbers[count]=ROMLibs2Num[count];
    }
    NumHiLibs=count;


    // INITIALIZE THE HOME DIRECTORY
    rplMakeNewDir();
    // INITIALIZE THE SETTINGS DIRECTORY
    SettingsDir=rplMakeNewDir();

    // CREATE AN EMPTY LIST OF SYSTEM FLAGS
    SystemFlags=rplAllocTempOb(7);  // FOR NOW: 128 SYSTEM FLAGS IN 2 BINTS WITH 64 BITS EACH

    if(!SystemFlags) return;

    SystemFlags[0]=MKPROLOG(DOLIST,7);  // PUT ALL SYSTEM FLAGS ON A LIST
    SystemFlags[1]=MKPROLOG(DECBINT,2); // USE A BINT PROLOG
    SystemFlags[2]=0;                   // FLAGS 0-31 ARE IN SystemFlags[2]
    SystemFlags[3]=0;                   // FLAGS 32-63 ARE IN SystemFlags[3]
    SystemFlags[4]=MKPROLOG(DECBINT,2);
    SystemFlags[5]=0;                   // FLAGS 64-95 ARE IN SystemFlags[5]
    SystemFlags[6]=0;                   // FLAGS 96-127 ARE IN SystemFlags[6]
                                        // FUTURE EXPANSION: ADD MORE FLAGS HERE
    SystemFlags[7]=CMD_ENDLIST;         // CLOSE THE LIST


    // INITIALIZE THE FLOATING POINT CONTEXT
    mpd_init(&Context,18);

    // LIMIT THE EXPONENT TO A 16 BIT VALUE FOR EASIER STORAGE
    mpd_qsetemax(&Context,29999);
    mpd_qsetemin(&Context,-29999);



    // INITIALIZE THE REAL REGISTERS
    for(count=0;count<REAL_REGISTERS;++count)
    {
        RReg[count].alloc=REAL_REGISTER_STORAGE;  // NUMBER OF ALLOCATED WORDS
        RReg[count].data=RDigits+EXTRA_STORAGE+count*REAL_REGISTER_STORAGE;
        RReg[count].flags=MPD_STATIC|MPD_STATIC_DATA;
        RReg[count].digits=0;
        RReg[count].exp=0;
        RReg[count].len=1;
        RReg[count].digits=0;
    }
    // INITIALIZE TEMP STORAGE FOR INTEGER TO REAL CONVERSION
    BINT2RealIdx=0;

    // SET ERROR TRAP HANDLER
    mpd_traphandler=&MPDTrapHandler;
    Context.traps=MPD_Clamped |
        MPD_Conversion_syntax   |
        MPD_Division_by_zero    |
        MPD_Division_impossible |
        MPD_Division_undefined  |
        MPD_Fpu_error           |
        //MPD_Inexact             |
        MPD_Invalid_context     |
        MPD_Invalid_operation   |
        MPD_Malloc_error        |
        MPD_Not_implemented     |
        MPD_Overflow            |
        //MPD_Rounded             |
        //MPD_Subnormal           |
        MPD_Underflow           ;



}

// FOR DEBUG ONLY: SHOW STATUS OF THE EXECUTION ENVIRONMENT
extern mpd_uint_t MPD_RegistersUsed;
void rplShowRuntimeState(void)
{
    printf("Used memory:\n-------------\n");
    printf("TempOb=%d words (%d allocated)\n",(WORD)(TempObEnd-TempOb),(WORD)(TempObSize-TempOb));
    printf("TempBlocks=%d words (%d allocated)\n",(WORD)(TempBlocksEnd-TempBlocks),TempBlocksSize);
    printf("Data Stack=%d words (%d allocated)\n",(WORD)(DSTop-DStk),DStkSize);
    printf("Return Stack=%d words (%d allocated)\n",(WORD)(RSTop-RStk),RStkSize);
    printf("Local vars=%d words (%d allocated)\n",(WORD)(LAMTop-LAMs),LAMSize);
    printf("Directories=%d words (%d allocated)\n",(WORD)(DirsTop-Directories),DirSize);
    if(MPD_RegistersUsed) printf("************* MPD Memory leak!!!\n");
}
