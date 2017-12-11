/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// CONVERT A POINTER INTO A ROMPTR ID
// IF IT FAILS, RETURNS 0

WORD rplConvertToRomptrID(WORDPTR ptr)
{
    BINT libnum=MAXLIBNUMBER;
    BINT SavedOpcode;
    LIBHANDLER han;

    SavedOpcode=CurOpcode;
    CurOpcode=MKOPCODE(libnum,OPCODE_GETROMID);

    while(libnum>=0) {
       han=rplGetLibHandler(libnum);
       if(han) {
           RetNum=ERR_INVALID;
           ObjectPTR=ptr;

           (*han)();

           if(RetNum==OK_CONTINUE) return ObjectID;
       }
       --libnum;
    }
    CurOpcode=SavedOpcode;

    // BAD POINTER OR NOT POINTING INTO ROM
    return 0;

}

// GET A POINTER FROM AN ID
// RETURNS 0 IF NOT A VALID ID
// OR NOT RECOGNIZED BY ITS LIBRARIES

WORDPTR rplConvertIDToPTR(WORD romptrid)
{
    if(!ISROMPTRID(romptrid)) return 0;

    BINT libnum=ROMPTRID_LIB(romptrid);

    BINT SavedOpcode;
    LIBHANDLER han;

    SavedOpcode=CurOpcode;
    CurOpcode=MKOPCODE(libnum,OPCODE_ROMID2PTR);

    han=rplGetLibHandler(libnum);
    if(!han) return 0;
    RetNum=ERR_INVALID;
    ObjectID=romptrid;

    (*han)();

    CurOpcode=SavedOpcode;

    if(RetNum==OK_CONTINUE) return ObjectPTR;

    return 0;

}












// BACKUP TEMPOB AND DIRECTORIES (NO STACK) TO EXTERNAL DEVICE

BINT rplBackup(int (*writefunc)(unsigned int,void *),void *OpaqueArgument)
{
    BINT offset;
    BINT k;
    // COMPACT TEMPOB AS MUCH AS POSSIBLE
    rplGCollect();

    // DUMP SYSTEM VARIABLES TO THE FILE


    // GENERIC SECTIONS
    struct {
        WORDPTR start;
        BINT nitems;
        BINT offwords;
    } sections[10];

    // FILL SECTIONS

    offset=1024;        // SAVE 1024 WORD ON THE FILE TO STORE SYSTEM VARIABLES
                        // AND SECTION INFORMATION

    k=0;
    // TEMPBLOCKS
    sections[k].start= (WORDPTR)TempBlocks;
    sections[k].nitems= TempBlocksEnd-TempBlocks;
    sections[k].offwords=offset;

    offset+=sections[k].nitems;
    ++k;

    // TEMPOB
    sections[k].start=(WORDPTR)TempOb;
    sections[k].nitems=TempObEnd-TempOb;
    sections[k].offwords=offset;

    offset+=sections[k].nitems;
    ++k;

    // TEMPOB AFTER END
    sections[k].start=(WORDPTR)TempObEnd;
    sections[k].nitems=TempObSize-TempObEnd;        // INCLUDE SPACE AFTER TEMPOBEND IN CASE THE BACKUP IS DONE DURING COMPILE/DECOMPILE
    sections[k].offwords=offset;

    offset+=sections[k].nitems;
    ++k;

    // DIRECTORIES
    sections[k].start=(WORDPTR)Directories;
    sections[k].nitems=DirsTop-Directories;
    sections[k].offwords=offset;

    offset+=sections[k].nitems;
    ++k;

    // SYSTEM POINTERS
    sections[k].start=(WORDPTR)GC_PTRUpdate;
    sections[k].nitems=MAX_GC_PTRUPDATE;
    sections[k].offwords=offset;

    offset+=sections[k].nitems;
    ++k;


    // FILL ALL OTHER SECTIONS
    for(;k<10;++k) {
        sections[k].start=0;
        sections[k].nitems=0;
        sections[k].offwords=offset;
    }


    BINT writeoff=0;

    // FIRST, WRITE SIGNATURE TO THE FILE
    if(!writefunc( TEXT2WORD('N','R','P','B'),OpaqueArgument)) return 0;
    writeoff++;
    // WRITE ALL 10 SECTIONS START OFFSET
    // END OFFSET IS THE START OF THE SECTION IMMEDIATELY AFTER

    for(k=0;k<10;++k) {
        if(!writefunc(sections[k].offwords,OpaqueArgument)) return 0;
        ++writeoff;
    }

    // TODO: WRITE OTHER SYSTEM VARIABLES HERE

    // FILL THE HEADER SECTION
    while(writeoff<1024) { if(!writefunc(0,OpaqueArgument)) return 0; ++writeoff; }

    // HERE WE ARE AT OFFSET 4096 (1024 WORDS)

    // DUMP TEMPBLOCKS TO THE FILE
    for(k=0;k<sections[0].nitems;++k) {
        if(!writefunc((BINT)(TempBlocks[k]-TempOb)+sections[1].offwords,OpaqueArgument)) return 0;        // WRITE BLOCKS AS OFFSET RELATIVE TO THE FILE INSTEAD OF POINTER
        ++writeoff;
    }

    // DUMP TEMPOB TO THE FILE
    for(k=0;k<sections[1].nitems;++k) {
        if(!writefunc(TempOb[k],OpaqueArgument)) return 0;        // WRITE TEMPOB AS-IS, NO POINTERS THERE
        ++writeoff;
    }
    // DUMP TEMPOB AFTER END TO THE FILE
    for(k=0;k<sections[2].nitems;++k) {
        if(!writefunc(TempObEnd[k],OpaqueArgument)) return 0;        // WRITE TEMPOB AS-IS, NO POINTERS THERE
        ++writeoff;
    }


    // DUMP DIRECTORIES TO THE FILE
    WORDPTR ptr;
    for(k=0;k<sections[3].nitems;++k) {
        ptr=Directories[k];
        if( (ptr>=TempOb) && (ptr<TempObEnd) ) {
            // VALID POINTER INTO TEMPOB, CONVERT INTO FILE OFFSET
            if(!writefunc( (BINT)(ptr-TempOb)+ sections[1].offwords,OpaqueArgument)) return 0;
        } else {
            // IF THE OBJECT IS NOT IN TEMPOB IS IN ROM

            // CONVERT TO ROMPTR ID

            WORD id=rplConvertToRomptrID(ptr);

            if(!id) {
                // INVALID POINTER! NEED TO FIX THE MEMORY BEFORE DOING BACKUP
                // WE DON'T DO THAT HERE
                return 0;
            }
            if(!writefunc(id,OpaqueArgument)) return 0;
        }
        ++writeoff;
    }

    // DUMP SYSTEM POINTERS
    for(k=0;k<sections[4].nitems;++k) {
        ptr=GC_PTRUpdate[k];
        if( (ptr>=TempOb) && (ptr<=TempObSize) ) {
            // VALID POINTER INTO TEMPOB, CONVERT INTO FILE OFFSET
            if(!writefunc( (BINT)(ptr-TempOb)+ sections[1].offwords,OpaqueArgument)) return 0;
        } else {
            // IF THE OBJECT IS NOT IN TEMPOB IS IN ROM

            // CONVERT TO ROMPTR ID

            WORD id=rplConvertToRomptrID(ptr);

            if(!id) {
                // INVALID POINTER! THESE ARE NORMAL IN THIS ARE, NO BIG DEAL
                // REPLACE WITH zero_bint FOR GOOD MEASURES
                id=rplConvertToRomptrID((WORDPTR)zero_bint);
            }
            if(!writefunc(id,OpaqueArgument)) return 0;
        }
        ++writeoff;
    }


    return 1;

}


// FULLY RESTORE TEMPOB AND DIRECTORIES FROM BACKUP
// WARNING: THIS IS A DESTRUCTIVE OPERATION
// IF SOMETHING GOES WRONG, IT CONTINUES UNTIL THE END
// RETURNS:
// -1 = BAD FILE, MEMORY DESTROYED
// 0 = BAD FILE, ORIGINAL MEMORY STILL INTACT
// 1 = RESTORE COMPLETED WITH NO ERRORS
// 2 = RESTORE COMPLETED WITH SOME ERRORS, RUN MEMFIX LATER

BINT rplRestoreBackup(WORD (*readfunc)(void *),void *OpaqueArgument)
{

    // GENERIC SECTIONS
    struct {
        WORDPTR start;
        BINT nitems;
        BINT offwords;
    } sections[10];

    BINT offset=0,k,errors=0;

    WORD data;

    //  CHECK FILE SIGNATURE
    data=readfunc(OpaqueArgument);
    ++offset;

    if(data!=TEXT2WORD('N','R','P','B')) return 0;

    // READ ALL 10 SECTIONS
    for(k=0;k<10;++k)
    {
        sections[k].offwords=readfunc(OpaqueArgument);
        ++offset;
        if(k>0) sections[k-1].nitems=sections[k].offwords-sections[k-1].offwords;
    }

    // NOW DETERMINE THE POINTERS

    sections[0].start=(WORDPTR)TempBlocks;
    sections[1].start=TempOb;
    sections[2].start=TempOb+sections[1].nitems;
    sections[3].start=(WORDPTR)Directories;
    sections[4].start=(WORDPTR)GC_PTRUpdate;


    // TODO: ADD OTHER SECTIONS HERE (STACKS, ETC)




    // ERASE ALL RPL MEMORY IN PREPARATION FOR RESTORE
    // ALL SECTIONS TO MINIMUM SIZE TO FREE AS MANY PAGES AS POSSIBLE
    TempObEnd=TempOb;
    TempBlocksEnd=TempBlocks;
    shrinkTempOb(1024); // GET SOME MEMORY FOR TEMPORARY OBJECT STORAGE
    shrinkTempBlocks(1024); // GET SOME MEMORY FOR TEMPORARY OBJECT BLOCKS

    RSTop=RStk;
    growRStk(1024);   // GET SOME MEMORY FOR RETURN STACK

    DSTop=DStkBottom=DStkProtect=DStk;
    growDStk(1024);   // GET SOME MEMORY FOR DATA STACK

    LAMTopSaved=LAMTop=nLAMBase=LAMs;
    growLAMs(1024); // GET SOME MEMORY FOR LAM ENVIRONMENTS

    CurrentDir=Directories;
    DirsTop=Directories;
    growDirs(1024); // MEMORY FOR ROOT DIRECTORY

    // RESIZE ALL SECTIONS TO THE REQUESTED SIZE
    if(sections[0].nitems+TEMPBLOCKSLACK>1024) growTempBlocks(sections[0].nitems+TEMPBLOCKSLACK);
    if(sections[1].nitems+sections[2].nitems>1024) growTempOb(sections[1].nitems+sections[2].nitems);
    if(sections[3].nitems+DIRSLACK>1024) growDirs(sections[3].nitems+DIRSLACK);

    if(Exceptions) return -1;

    // SKIP TO THE PROPER FILE OFFSET

    while(offset!=1024) { readfunc(OpaqueArgument); ++offset; }

    // HERE'S THE START OF TEMPBLOCKS
    for(k=0;k<sections[0].nitems;++k) {
        data=readfunc(OpaqueArgument);
        TempBlocks[k]=TempOb+(data-sections[1].offwords);
        ++offset;
    }
    if(Exceptions) return -1;

    TempBlocksEnd=TempBlocks+sections[0].nitems;

    // HERE'S THE START OF TEMPOB
    for(k=0;k<sections[1].nitems;++k)
    {
        data=readfunc(OpaqueArgument);
        TempOb[k]=data;
        ++offset;
    }
    if(Exceptions) return -1;

    TempObEnd=TempOb+sections[1].nitems;

    // AFTER TEMPOB
    for(k=0;k<sections[2].nitems;++k)
    {
        data=readfunc(OpaqueArgument);
        TempObEnd[k]=data;
        ++offset;
    }
    if(Exceptions) return -1;


    // HERE'S THE START OF THE DIRECTORIES

    for(k=0;k<sections[3].nitems;++k)
    {
        data=readfunc(OpaqueArgument);
        if(ISROMPTRID(data)) Directories[k]=rplConvertIDToPTR(data);
        else Directories[k]=TempOb+(data-sections[1].offwords);
        if(Directories[k]==0) ++errors; // JUST COUNT THE ERRORS BUT DON'T STOP, RECOVER AS MUCH AS POSSIBLE
        ++offset;
    }
    if(Exceptions) return -1;

    DirsTop=Directories+sections[3].nitems;

    // HERE'S THE START OF SYSTEM POINTERS

    for(k=0;k<sections[4].nitems;++k)
    {
        data=readfunc(OpaqueArgument);
        if(ISROMPTRID(data)) GC_PTRUpdate[k]=rplConvertIDToPTR(data);
        else {
            if((k==1)||(k==2)) {
                // EXTRA CHECKS FOR TempObEnd and TempObSize
             if(GC_PTRUpdate[k]!=TempOb+(data-sections[1].offwords)) ++errors;
            }

                GC_PTRUpdate[k]=TempOb+(data-sections[1].offwords);
        }
        ++offset;
    }


    // TODO: MORE SECTIONS OR OTHER CLEANUP HERE

    if(errors) return 2;
    return 1;
}

// THIS IS FOR DEBUGGING ERROR RECOVERY CODE
/*
#define DO_SOME_DAMAGE 0x123

BINT rplRestoreBackupMessedup(WORD (*readfunc)(void *),void *OpaqueArgument)
{

    // GENERIC SECTIONS
    struct {
        WORDPTR start;
        BINT nitems;
        BINT offwords;
    } sections[10];

    BINT offset=0,k,errors=0;

    WORD data;

    //  CHECK FILE SIGNATURE
    data=readfunc(OpaqueArgument);
    ++offset;

    if(data!=TEXT2WORD('N','R','P','B')) return 0;

    // READ ALL 10 SECTIONS
    for(k=0;k<10;++k)
    {
        sections[k].offwords=readfunc(OpaqueArgument);
        ++offset;
        if(k>0) sections[k-1].nitems=sections[k].offwords-sections[k-1].offwords;
    }

    // NOW DETERMINE THE POINTERS

    sections[0].start=(WORDPTR)TempBlocks;
    sections[1].start=TempOb;
    sections[2].start=TempOb+sections[1].nitems;
    sections[3].start=(WORDPTR)Directories;


    // TODO: ADD OTHER SECTIONS HERE (STACKS, ETC)




    // ERASE ALL RPL MEMORY IN PREPARATION FOR RESTORE
    // ALL SECTIONS TO MINIMUM SIZE TO FREE AS MANY PAGES AS POSSIBLE
    shrinkTempOb(1024); // GET SOME MEMORY FOR TEMPORARY OBJECT STORAGE
    TempObEnd=TempOb;
    shrinkTempBlocks(1024); // GET SOME MEMORY FOR TEMPORARY OBJECT BLOCKS
    TempBlocksEnd=TempBlocks;

    growRStk(1024);   // GET SOME MEMORY FOR RETURN STACK
    RSTop=RStk;
    growDStk(1024);   // GET SOME MEMORY FOR DATA STACK
    DSTop=DStkBottom=DStkProtect=DStk;

    growLAMs(1024); // GET SOME MEMORY FOR LAM ENVIRONMENTS
    LAMTopSaved=LAMTop=nLAMBase=LAMs;

    growDirs(1024); // MEMORY FOR ROOT DIRECTORY
    CurrentDir=Directories;
    DirsTop=Directories;

    // RESIZE ALL SECTIONS TO THE REQUESTED SIZE
    if(sections[0].nitems+TEMPBLOCKSLACK>1024) growTempBlocks(sections[0].nitems+TEMPBLOCKSLACK);
    if(sections[1].nitems+sections[2].nitems>1024) growTempOb(sections[1].nitems+sections[2].nitems);
    if(sections[3].nitems+DIRSLACK>1024) growDirs(sections[3].nitems+DIRSLACK);

    if(Exceptions) return -1;

    // SKIP TO THE PROPER FILE OFFSET

    while(offset!=1024) { readfunc(OpaqueArgument); ++offset; }

    // HERE'S THE START OF TEMPBLOCKS
    for(k=0;k<sections[0].nitems;++k) {
        data=readfunc(OpaqueArgument);
        TempBlocks[k]=TempOb+(data-sections[1].offwords);
        ++offset;
    }
    TempBlocksEnd=TempBlocks+sections[0].nitems;

    // HERE'S THE START OF TEMPOB
    for(k=0;k<sections[1].nitems;++k)
    {
        data=readfunc(OpaqueArgument);
        TempOb[k]=data;
        ++offset;
    }
    TempObEnd=TempOb+sections[1].nitems;

    // AFTER TEMPOB
    for(k=0;k<sections[2].nitems;++k)
    {
        data=readfunc(OpaqueArgument);
        TempObEnd[k]=data;
        ++offset;
    }


    // HERE'S THE START OF THE DIRECTORIES

    for(k=0;k<sections[3].nitems;++k)
    {
        data=readfunc(OpaqueArgument);
        if(ISROMPTRID(data)) Directories[k]=rplConvertIDToPTR(data)+DO_SOME_DAMAGE;
        else Directories[k]=TempOb+(data-sections[1].offwords);
        if(Directories[k]==0) ++errors; // JUST COUNT THE ERRORS BUT DON'T STOP, RECOVER AS MUCH AS POSSIBLE
        ++offset;
    }

    DirsTop=Directories+sections[3].nitems;

    // TODO: MORE SECTIONS OR OTHER CLEANUP HERE

    if(errors) return 2;
    return 1;
}

*/
