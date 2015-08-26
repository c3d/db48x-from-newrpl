/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "memory.h"
#include "libraries.h"
#include "hal.h"

// MAIN RUNSTREAM MANAGEMENT FOR newRPL
#define EXIT_LOOP -1000

WORD RPLLastOpcode;

extern const WORD dotsettings_ident[];
extern const WORD flags_ident[];





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
    while(*listnumbers) {

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


void rplRun(void)
// TAKE THE NEXT WORD AND EXECUTE IT
{
    LIBHANDLER han;

    do {
    RPLLastOpcode=CurOpcode=*IPtr;

    han=rplGetLibHandler(LIBNUM(CurOpcode));

    if(han) (*han)();
    else {
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
        rplClearRStk(); // CLEAR THE RETURN STACK
        rplClearLAMs(); // CLEAR ALL LOCAL VARIABLES
        // INVALID OPCODE = END OF EXECUTION (CANNOT BE TRAPPED BY HANDLER)
        return;
    }
    if(Exceptions) {
        if(Exceptions==EX_EXITRPL) {
            Exceptions=0;
            rplClearRStk(); // CLEAR THE RETURN STACK
            rplClearLAMs(); // CLEAR ALL LOCAL VARIABLES
            return; // DON'T ALLOW HANDLER TO TRAP THIS EXCEPTION
        }
        if(ErrorHandler) {
            // ERROR WAS TRAPPED BY A HANDLER
            rplCatchException();
        }
        else {
            // THERE IS NO ERROR HANDLER --> UNTRAPPED ERROR
            if(!(Exceptions&EX_BKPOINT)) {  // DON'T CLEANUP IF A BREAKPOINT WAS REACHED
                rplClearRStk(); // CLEAR THE RETURN STACK
                rplClearLAMs(); // CLEAR ALL LOCAL VARIABLES
            }
            else rplSkipNext(); // PREPARE TO RESUME ON NEXT CALL
            return;      // END EXECUTION IMMEDIATELY IF AN UNHANDLED EXCEPTION IS THROWN
        }
    }

    // SKIP TO THE NEXT INSTRUCTION / OBJECT BASED ON CurOpcode
    // NOTICE THAT CurOpcode MIGHT BE MODIFIED BY A LIBRARY HANDLER TO ALTER THE FLOW
    IPtr+=1+((ISPROLOG(CurOpcode))? OBJSIZE(CurOpcode):0);

    } while(1);

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
    CurOpcode=0; // CURRENT OPCODE (WORD)
    TempObSize=0;    // TOTAL SIZE OF TEMPOB
    TempBlocksSize=0;
    DStkSize=0;    // TOTAL SIZE OF DATA STACK
    RStkSize=0;    // TOTAL SIZE OF RETURN STACK
    LAMSize=0;
    Exceptions=0;   // NO EXCEPTIONS RAISED
    ExceptionPointer=0;

    rplClearLibraries();

    // INSTALL ALL ROM LIBRARIES FROM A NULL-TERMINATED LIST
    LIBHANDLER *libptr=(LIBHANDLER *)ROMLibs;
    while(*libptr) { rplInstallLibrary(*libptr); ++libptr; }


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

    // CREATE AN EMPTY LIST OF SYSTEM FLAGS
    SystemFlags=rplAllocTempOb(7);  // FOR NOW: 128 SYSTEM FLAGS IN 2 BINTS WITH 64 BITS EACH

    if(!SystemFlags) return;

    SystemFlags[0]=MKPROLOG(DOLIST,7);  // PUT ALL SYSTEM FLAGS ON A LIST
    SystemFlags[1]=MKPROLOG(HEXBINT,2); // USE A BINT PROLOG
    SystemFlags[2]=(63<<4)|(1<<29)|(7<<10);             // FLAGS 0-31 ARE IN SystemFlags[2], DEFAULTS: WORDSIZE=63, DEG, COMMENTS=ON, 7*8=56 UNDO LEVELS
    SystemFlags[3]=0;                   // FLAGS 32-63 ARE IN SystemFlags[3]
    SystemFlags[4]=MKPROLOG(HEXBINT,2);
    SystemFlags[5]=0;                   // FLAGS 64-95 ARE IN SystemFlags[5]
    SystemFlags[6]=0;                   // FLAGS 96-127 ARE IN SystemFlags[6]
                                        // FUTURE EXPANSION: ADD MORE FLAGS HERE
    SystemFlags[7]=CMD_ENDLIST;         // CLOSE THE LIST


    rplStoreSettings((WORDPTR)flags_ident,SystemFlags);


    // INITIALIZE THE FLOATING POINT CONTEXT
    initContext(32);

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
    CurOpcode=0; // CURRENT OPCODE (WORD)

    Exceptions=0;   // NO EXCEPTIONS RAISED
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
    // INSTALL ALL ROM LIBRARIES FROM A NULL-TERMINATED LIST
    LIBHANDLER *libptr=(LIBHANDLER *)ROMLibs;
    while(*libptr) { rplInstallLibrary(*libptr); ++libptr; }


    // INITIALIZE THE FLOATING POINT CONTEXT
    initContext(32);

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
    if(settings) SettingsDir=settings[1];
    else {
        // CREATE THE SETTINGS DIRECTORY
        // INITIALIZE THE SETTINGS DIRECTORY
        SettingsDir=(WORDPTR)rplCreateNewDir((WORDPTR)dotsettings_ident,CurrentDir);

        // CREATE AN EMPTY LIST OF SYSTEM FLAGS
        SystemFlags=rplAllocTempOb(7);  // FOR NOW: 128 SYSTEM FLAGS IN 2 BINTS WITH 64 BITS EACH

        if(!SystemFlags) return;

        SystemFlags[0]=MKPROLOG(DOLIST,7);  // PUT ALL SYSTEM FLAGS ON A LIST
        SystemFlags[1]=MKPROLOG(HEXBINT,2); // USE A BINT PROLOG
        SystemFlags[2]=(63<<4)|(1<<29)|(7<<10);             // FLAGS 0-31 ARE IN SystemFlags[2], DEFAULTS: WORDSIZE=63, DEG, COMMENTS=ON, 7*8=56 UNDO LEVELS
        SystemFlags[3]=0;                   // FLAGS 32-63 ARE IN SystemFlags[3]
        SystemFlags[4]=MKPROLOG(HEXBINT,2);
        SystemFlags[5]=0;                   // FLAGS 64-95 ARE IN SystemFlags[5]
        SystemFlags[6]=0;                   // FLAGS 96-127 ARE IN SystemFlags[6]
                                            // FUTURE EXPANSION: ADD MORE FLAGS HERE
        SystemFlags[7]=CMD_ENDLIST;         // CLOSE THE LIST


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
