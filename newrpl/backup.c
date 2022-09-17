/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "libraries.h"
#include "newrpl.h"
#include "sysvars.h"

// CONVERT A POINTER INTO A ROMPTR ID
// IF IT FAILS, RETURNS 0

uint64_t rplConvertToRomptrID(WORDPTR ptr)
{
    BINT libnum = MAXLIBNUMBER;
    BINT SavedOpcode;
    LIBHANDLER han;

    SavedOpcode = CurOpcode;
    CurOpcode = MKOPCODE(libnum, OPCODE_GETROMID);

    while(libnum >= 0) {
        han = rplGetLibHandler(libnum);
        if(han) {
            RetNum = ERR_INVALID;
            ObjectPTR = ptr;

            (*han) ();

            if(RetNum == OK_CONTINUE)
                return ((uint64_t) ObjectID) | (((uint64_t) ObjectIDHash) << 32);
        }
        libnum = rplGetNextLib(libnum);
    }
    CurOpcode = SavedOpcode;

    // BAD POINTER OR NOT POINTING INTO ROM
    return 0;

}

// GET A POINTER FROM AN ID
// RETURNS 0 IF NOT A VALID ID
// OR NOT RECOGNIZED BY ITS LIBRARIES

WORDPTR rplConvertIDToPTR(uint64_t romptrid)
{
    if(!ISROMPTRID(romptrid))
        return 0;

    BINT libnum = ROMPTRID_LIB(romptrid);

    BINT SavedOpcode;
    LIBHANDLER han;

    SavedOpcode = CurOpcode;
    CurOpcode = MKOPCODE(libnum, OPCODE_ROMID2PTR);

    han = rplGetLibHandler(libnum);
    if(!han)
        return 0;
    RetNum = ERR_INVALID;
    ObjectID = romptrid;
    ObjectIDHash = romptrid >> 32;

    (*han) ();

    CurOpcode = SavedOpcode;

    if(RetNum == OK_CONTINUE)
        return ObjectPTR;

    return 0;

}

// BACKUP TEMPOB AND DIRECTORIES (NO STACK) TO EXTERNAL DEVICE

BINT rplBackup(int (*writefunc)(unsigned int, void *), void *OpaqueArgument)
{
    BINT offset;
    BINT k;
    // COMPACT TEMPOB AS MUCH AS POSSIBLE
    rplGCollect();

    // DUMP SYSTEM VARIABLES TO THE FILE

    // GENERIC SECTIONS
    struct
    {
        WORDPTR start;
        BINT nitems;
        BINT offwords;
    } sections[10];

    // FILL SECTIONS

    offset = 1024;      // SAVE 1024 WORD ON THE FILE TO STORE SYSTEM VARIABLES
    // AND SECTION INFORMATION

    k = 0;
    // TEMPBLOCKS
    sections[k].start = (WORDPTR) TempBlocks;
    sections[k].nitems = TempBlocksEnd - TempBlocks;
    sections[k].offwords = offset;

    offset += sections[k].nitems;
    ++k;

    // TEMPOB
    sections[k].start = (WORDPTR) TempOb;
    sections[k].nitems = TempObEnd - TempOb;
    sections[k].offwords = offset;

    offset += sections[k].nitems;
    ++k;

    // TEMPOB AFTER END
    sections[k].start = (WORDPTR) TempObEnd;
    sections[k].nitems = TempObSize - TempObEnd;        // INCLUDE SPACE AFTER TEMPOBEND IN CASE THE BACKUP IS DONE DURING COMPILE/DECOMPILE
    sections[k].offwords = offset;

    offset += sections[k].nitems;
    ++k;

    // DIRECTORIES
    sections[k].start = (WORDPTR) Directories;
    sections[k].nitems = DirsTop - Directories;
    sections[k].offwords = offset;

    offset += sections[k].nitems * 2;
    ++k;

    // SYSTEM POINTERS
    sections[k].start = (WORDPTR) GC_PTRUpdate;
    sections[k].nitems = (MAX_GC_PTRUPDATE);
    sections[k].offwords = offset;

    offset += sections[k].nitems * 2;
    ++k;

    // STACK
    sections[k].start = (WORDPTR) DStk;
    sections[k].nitems = (DSTop - DStk);
    sections[k].offwords = offset;

    offset += sections[k].nitems * 2;
    ++k;

    // FILL ALL OTHER SECTIONS
    for(; k < 10; ++k) {
        sections[k].start = 0;
        sections[k].nitems = 0;
        sections[k].offwords = offset;
    }

    BINT writeoff = 0;

    // FIRST, WRITE SIGNATURE TO THE FILE
    if(!writefunc(TEXT2WORD('N', 'R', 'P', 'B'), OpaqueArgument))
        return 0;
    writeoff++;
    // WRITE ALL 10 SECTIONS START OFFSET
    // END OFFSET IS THE START OF THE SECTION IMMEDIATELY AFTER

    for(k = 0; k < 10; ++k) {
        if(!writefunc(sections[k].offwords, OpaqueArgument))
            return 0;
        ++writeoff;
    }

    // WRITE ALL 10 SECTIONS nitems
    for(k = 0; k < 10; ++k) {
        if(!writefunc(sections[k].nitems, OpaqueArgument))
            return 0;
        ++writeoff;
    }

    // TODO: WRITE OTHER SYSTEM VARIABLES HERE

    // FILL THE HEADER SECTION
    while(writeoff < 1024) {
        if(!writefunc(0, OpaqueArgument))
            return 0;
        ++writeoff;
    }

    // HERE WE ARE AT OFFSET 4096 (1024 WORDS)

    // DUMP TEMPBLOCKS TO THE FILE
    for(k = 0; k < sections[0].nitems; ++k) {
        if(!writefunc((BINT) (TempBlocks[k] - TempOb) + sections[1].offwords,
                    OpaqueArgument))
            return 0;   // WRITE BLOCKS AS OFFSET RELATIVE TO THE FILE INSTEAD OF POINTER
        ++writeoff;
    }

    // DUMP TEMPOB TO THE FILE
    for(k = 0; k < sections[1].nitems; ++k) {
        if(!writefunc(TempOb[k], OpaqueArgument))
            return 0;   // WRITE TEMPOB AS-IS, NO POINTERS THERE
        ++writeoff;
    }
    // DUMP TEMPOB AFTER END TO THE FILE
    for(k = 0; k < sections[2].nitems; ++k) {
        if(!writefunc(TempObEnd[k], OpaqueArgument))
            return 0;   // WRITE TEMPOB AS-IS, NO POINTERS THERE
        ++writeoff;
    }

    // DUMP DIRECTORIES TO THE FILE
    WORDPTR ptr;
    for(k = 0; k < sections[3].nitems; ++k) {
        ptr = Directories[k];
        if((ptr >= TempOb) && (ptr < TempObEnd)) {
            // VALID POINTER INTO TEMPOB, CONVERT INTO FILE OFFSET
            if(!writefunc((BINT) (ptr - TempOb) + sections[1].offwords,
                        OpaqueArgument))
                return 0;
        }
        else {
            // IF THE OBJECT IS NOT IN TEMPOB, IS IN ROM

            // CONVERT TO ROMPTR ID

            uint64_t id = rplConvertToRomptrID(ptr);

            if(!id) {
                // INVALID POINTER! NEED TO FIX THE MEMORY BEFORE DOING BACKUP
                // WE DON'T DO THAT HERE
                return 0;
            }
            if(!writefunc(id, OpaqueArgument))
                return 0;
            if(ROMPTRID_OFF(id) == 31) {
                ++writeoff;
                if(!writefunc((id >> 32), OpaqueArgument))
                    return 0;
            }
        }
        ++writeoff;
    }

    while(writeoff < sections[4].offwords) {
        if(!writefunc(0, OpaqueArgument))
            return 0;
        ++writeoff;
    }

    // DUMP SYSTEM POINTERS
    for(k = 0; k < sections[4].nitems; ++k) {
        ptr = GC_PTRUpdate[k];
        if((ptr >= TempOb) && (ptr <= TempObSize)) {
            // VALID POINTER INTO TEMPOB, CONVERT INTO FILE OFFSET
            if(!writefunc((BINT) (ptr - TempOb) + sections[1].offwords,
                        OpaqueArgument))
                return 0;
        }
        else {

            uint64_t id;
            if(!ptr) {
                // NULL POINTER, STORE AS-IS
                id = 0;
            }
            else {
                // IF THE OBJECT IS NOT IN TEMPOB IS IN ROM

                // CONVERT TO ROMPTR ID

                id = rplConvertToRomptrID(ptr);

                if(!id) {
                    // INVALID POINTER! THESE ARE NORMAL IN THIS ARE, NO BIG DEAL
                    // REPLACE WITH zero_bint FOR GOOD MEASURES
                    id = rplConvertToRomptrID((WORDPTR) zero_bint);
                }
            }
            if(!writefunc(id, OpaqueArgument))
                return 0;
            if(ROMPTRID_OFF(id) == 31) {
                ++writeoff;
                if(!writefunc((id >> 32), OpaqueArgument))
                    return 0;
            }
        }
        ++writeoff;
    }

    while(writeoff < sections[5].offwords) {
        if(!writefunc(0, OpaqueArgument))
            return 0;
        ++writeoff;
    }

    // DUMP DATA STACK POINTERS
    for(k = 0; k < sections[5].nitems; ++k) {
        ptr = DStk[k];
        if((ptr >= TempOb) && (ptr <= TempObSize)) {
            // VALID POINTER INTO TEMPOB, CONVERT INTO FILE OFFSET
            if(!writefunc((BINT) (ptr - TempOb) + sections[1].offwords,
                        OpaqueArgument))
                return 0;
        }
        else {

            uint64_t id;

            // SPECIAL CASE - HANDLE OFFSET NUMBERS STORED INSIDE THE STACK
            if((intptr_t) ptr <= (intptr_t) sections[5].nitems) {
                // THIS IS A NUMBER, NOT A POINTER
                //  IN THE STACK, THESE ARE STACK MARKERS
                // REPLACE WITH A SPECIAL ROMPTRID FOR LIBRARY 0, ID=63, offset=31
                id = (((uint64_t) ((intptr_t) ptr)) << 32) | MKROMPTRID(0, 63,
                        31);
            }
            else {
                // IF THE OBJECT IS NOT IN TEMPOB IS IN ROM

                // CONVERT TO ROMPTR ID

                id = rplConvertToRomptrID(ptr);

                if(!id)
                    id = rplConvertToRomptrID((WORDPTR) zero_bint);
            }

            if(!writefunc(id, OpaqueArgument))
                return 0;
            if(ROMPTRID_OFF(id) == 31) {
                ++writeoff;
                if(!writefunc((id >> 32), OpaqueArgument))
                    return 0;
            }

        }
        ++writeoff;
    }

    while(writeoff < sections[6].offwords) {
        if(!writefunc(0, OpaqueArgument))
            return 0;
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

BINT rplRestoreBackup(BINT includestack, WORD(*readfunc) (void *),
        void *OpaqueArgument)
{

    // GENERIC SECTIONS
    struct
    {
        WORDPTR start;
        BINT nitems;
        BINT offwords;
    } sections[10];

    BINT offset = 0, k, errors = 0;

    WORD data;

    //  CHECK FILE SIGNATURE
    data = readfunc(OpaqueArgument);
    ++offset;

    if(data != TEXT2WORD('N', 'R', 'P', 'B'))
        return 0;

    // READ ALL 10 SECTIONS
    for(k = 0; k < 10; ++k) {
        sections[k].offwords = readfunc(OpaqueArgument);
        ++offset;
    }

    // READ nitems, RESTORE FROM OFFSETS IF NOT PRESENT (BACKWARDS COMPATIBLE WITH OLD BACKUP FORMAT)
    for(k = 0; k < 10; ++k) {
        sections[k].nitems = readfunc(OpaqueArgument);
        ++offset;
        if((k > 0) && (sections[k - 1].nitems == 0))
            sections[k - 1].nitems =
                    sections[k].offwords - sections[k - 1].offwords;
    }

    // RESET RPL ENVIRONMENT
    // ERASE ALL RPL MEMORY IN PREPARATION FOR RESTORE
    // ALL SECTIONS TO MINIMUM SIZE TO FREE AS MANY PAGES AS POSSIBLE
    rplInit();

    /*
       // ERASE ALL RPL MEMORY IN PREPARATION FOR RESTORE
       // ALL SECTIONS TO MINIMUM SIZE TO FREE AS MANY PAGES AS POSSIBLE
       shrinkTempOb(1024); // GET SOME MEMORY FOR TEMPORARY OBJECT STORAGE
       shrinkTempBlocks(1024); // GET SOME MEMORY FOR TEMPORARY OBJECT BLOCKS
       TempObEnd=TempOb;
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
     */

    // NOW DETERMINE THE POINTERS

    sections[0].start = (WORDPTR) TempBlocks;
    sections[1].start = TempOb;
    sections[2].start = TempOb + sections[1].nitems;
    sections[3].start = (WORDPTR) Directories;
    sections[4].start = (WORDPTR) GC_PTRUpdate;

    // RESIZE ALL SECTIONS TO THE REQUESTED SIZE
    if(sections[0].nitems + TEMPBLOCKSLACK > 1024)
        growTempBlocks(sections[0].nitems + TEMPBLOCKSLACK);
    if(sections[1].nitems + sections[2].nitems > 1024)
        growTempOb(sections[1].nitems + sections[2].nitems);
    if((sections[3].nitems) + DIRSLACK > 1024)
        growDirs(sections[3].nitems + DIRSLACK);
    if(includestack) {
        if(sections[5].nitems + DSTKSLACK > 1024)
            growDirs(sections[5].nitems + DSTKSLACK);
    }

    if(Exceptions)
        return -1;

    // SKIP TO THE PROPER FILE OFFSET

    while(offset < 1024) {
        readfunc(OpaqueArgument);
        ++offset;
    }

    // HERE'S THE START OF TEMPBLOCKS
    for(k = 0; k < sections[0].nitems; ++k) {
        data = readfunc(OpaqueArgument);
        TempBlocks[k] = TempOb + (data - sections[1].offwords);
        ++offset;
    }
    if(Exceptions)
        return -1;

    TempBlocksEnd = TempBlocks + sections[0].nitems;

    while(offset < sections[1].offwords) {
        readfunc(OpaqueArgument);
        ++offset;
    }

    // HERE'S THE START OF TEMPOB
    for(k = 0; k < sections[1].nitems; ++k) {
        data = readfunc(OpaqueArgument);
        TempOb[k] = data;
        ++offset;

        // ***DEBUG: INTENTIONAL DELAY TO FOCE BUFFER SWITCH
//       {
//       tmr_t start=tmr_ticks(),end;
//       do end=tmr_ticks(); while(tmr_ticks2ms(start,end)<2);
//       }

    }
    if(Exceptions)
        return -1;

    TempObEnd = TempOb + sections[1].nitems;

    while(offset < sections[2].offwords) {
        readfunc(OpaqueArgument);
        ++offset;
    }

    // AFTER TEMPOB
    for(k = 0; k < sections[2].nitems; ++k) {
        data = readfunc(OpaqueArgument);
        TempObEnd[k] = data;
        ++offset;
    }
    if(Exceptions)
        return -1;

    while(offset < sections[3].offwords) {
        readfunc(OpaqueArgument);
        ++offset;
    }

    // HERE'S THE START OF THE DIRECTORIES

    for(k = 0; k < sections[3].nitems; ++k) {
        data = readfunc(OpaqueArgument);
        if(ISROMPTRID(data)) {
            WORD hash = 0;

            if(ROMPTRID_OFF(data) == 31) {
                hash = readfunc(OpaqueArgument);
                ++offset;
            }
            Directories[k] =
                    rplConvertIDToPTR(((uint64_t) data) | (((uint64_t) hash) <<
                        32));
        }
        else
            Directories[k] = TempOb + (data - sections[1].offwords);
        if(Directories[k] == 0)
            ++errors;   // JUST COUNT THE ERRORS BUT DON'T STOP, RECOVER AS MUCH AS POSSIBLE
        ++offset;
    }
    if(Exceptions)
        return -1;

    DirsTop = Directories + sections[3].nitems;

    while(offset < sections[4].offwords) {
        readfunc(OpaqueArgument);
        ++offset;
    }

    // HERE'S THE START OF SYSTEM POINTERS

    for(k = 0; k < sections[4].nitems; ++k) {
        data = readfunc(OpaqueArgument);
        if(ISROMPTRID(data)) {

            WORD hash = 0;

            if(ROMPTRID_OFF(data) == 31) {
                hash = readfunc(OpaqueArgument);
                ++offset;
            }

            GC_PTRUpdate[k] =
                    rplConvertIDToPTR(((uint64_t) data) | (((uint64_t) hash) <<
                        32));

        }
        else {
            if(!data)
                GC_PTRUpdate[k] = 0;    // RESTORE NULL POINTERS
            else {
                if((k == 1) || (k == 2)) {
                    // EXTRA CHECKS FOR TempObEnd and TempObSize
                    if(GC_PTRUpdate[k] !=
                            TempOb + (data - sections[1].offwords))
                        ++errors;
                }

                GC_PTRUpdate[k] = TempOb + (data - sections[1].offwords);
            }
        }
        ++offset;
    }

    // Do Not preserve HALTED status, KILL any application since all RStk Pointers are lost
    HaltedIPtr = 0;

    if(includestack) {
        // OPTIONALLY EXTRACT THE DATA STACK

        while(offset < sections[5].offwords) {
            readfunc(OpaqueArgument);
            ++offset;
        }
        DStkProtect = DStkBottom = DStk;

        for(k = 0; k < sections[5].nitems; ++k) {
            data = readfunc(OpaqueArgument);
            if(ISROMPTRID(data)) {
                WORD hash = 0;

                if(ROMPTRID_OFF(data) == 31) {
                    hash = readfunc(OpaqueArgument);
                    ++offset;
                }

                if(data == MKROMPTRID(0, 63, 31)) {
                    DStk[k] = NUMBER2PTR(hash);
                    DStkProtect = DStkBottom = DStk + k + 1;
                }
                else
                    DStk[k] =
                            rplConvertIDToPTR(((uint64_t) data) | (((uint64_t)
                                    hash) << 32));

            }
            else {
                if(((BINT) data) <= 0) {
                    // OLD FORMAT - THESE WERE NUMBERS USED IN THE STACK SNAPSHOTS
                    DStk[k] = NUMBER2PTR(-(BINT) data);
                    DStkProtect = DStkBottom = DStk + k + 1;
                }
                else {
                    // THESE ARE OFFSETS INTO TEMPOB
                    DStk[k] = TempOb + (data - sections[1].offwords);

                }
            }
            ++offset;
        }
        // SET DATA STACK POINTERS ACCORDINGLY
        DSTop = DStk + k;

    }
    // TODO: MORE SECTIONS OR OTHER CLEANUP HERE

    if(errors)
        return 2;
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
