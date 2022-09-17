/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "ui.h"

// ALLOCATES MEMORY FROM TEMPOB
// SIZE IS THE NUMBER OF WORDS REQUIRED, NOT COUNTING THE PROLOG!
// IT AUTOMATICALLY RESERVES ONE EXTRA WORD FOR THE PROLOG

WORDPTR rplAllocTempOb(WORD size)
{
    // SIMPLY ADD A NEW BLOCK AT END OF CHAIN

    // LARGE OBJECT SUPPORT
    WORD adjustedsize = (size>0x3ffff)? ((size+1+1023)&~0x3ff):size;     // ROUND TO WHOLE 4K PAGES WHEN LARGE SIZE REQUESTED, ALSO ALLOCATE 1 EXTRA WORD

    if(adjustedsize>0xffffc00) return 0;                                // HARD LIMIT ON LARGE OBJECTS = 1_GiB - 4_kiB

    if(TempObEnd + adjustedsize + 1 + TEMPOBLARGESLACK > TempObSize) {
        // ENLARGE TEMPOB AS NEEDED
        growTempOb((int32_t) (TempObEnd - TempOb) + adjustedsize + 1 + TEMPOBLARGESLACK);
        if(Exceptions)
            return 0;
    }

    rplAddTempBlock(TempObEnd);
    if(Exceptions)
        return 0;

    WORDPTR ptr = TempObEnd;
    TempObEnd += adjustedsize + 1;
    if(size>0x3ffff) TempObEnd[-1]=size;                      // STORE REQUESTED SIZE AT THE LAST WORD OF THE LARGE BLOCK, SO GRANULARITY IS NOT LOST
    return ptr;
}

// SAME BUT FOR MORE CRITICAL TASK THAT NEED TO WORK ON LOW-MEM CONDITIONS
WORDPTR rplAllocTempObLowMem(WORD size)
{
    // SIMPLY ADD A NEW BLOCK AT END OF CHAIN

    // LARGE OBJECT SUPPORT
    WORD adjustedsize = (size>0x3ffff)? ((size+1+1023)&~0x3ff):size;     // ROUND TO WHOLE 4K PAGES WHEN LARGE SIZE REQUESTED, ALSO ALLOCATE 1 EXTRA WORD

    if(adjustedsize>0xffffc00) return 0;                                // HARD LIMIT ON LARGE OBJECTS = 1_GiB - 4_kiB

    if(TempObEnd + adjustedsize + 1 + TEMPOBSLACK > TempObSize) {
        // ENLARGE TEMPOB AS NEEDED
        growTempOb((int32_t) (TempObEnd - TempOb) + adjustedsize + 1 + TEMPOBSLACK);
        if(Exceptions)
            return 0;
    }

    rplAddTempBlock(TempObEnd);
    if(Exceptions)
        return 0;

    WORDPTR ptr = TempObEnd;
    TempObEnd += adjustedsize + 1;
    if(size>0x3ffff) TempObEnd[-1]=size;                      // STORE REQUESTED SIZE AT THE LAST WORD OF THE LARGE BLOCK, SO GRANULARITY IS NOT LOST
    return ptr;
}

// TRUNCATES A RECENTLY ALLOCATED BLOCK AT THE END OF TEMPOB
// IT HAS TO BE THE LAST BLOCK ALLOCATED WITH ALLOCTEMPOB
void rplTruncateLastObject(WORDPTR newend)
{

    if(newend <= *(TempBlocksEnd - 1)) {
        // TRIED TO TRUNCATE BEYOND THE LAST OBJECT, CAN'T ALLOW THAT
        // REMOVE LAST ALLOCATED BLOCK COMPLETELY
        --TempBlocksEnd;
        TempObEnd = *TempBlocksEnd;
        return;
    }

    // LARGE BLOCK SUPPORT
    if((TempObEnd-*(TempBlocksEnd-1))>0x40000) {
        // THE LAST BLOCK IS A LARGE BLOCK, MAKE SURE AN EXTRA WORD IS ALLOCATED AND THE 4KB GRANULARITY IS RESPECTED
        WORD newsize=newend - *(TempBlocksEnd-1);
        WORD adjustedsize = (newsize>0x40000)? ((newsize+1+1023)&~0x3ff):newsize;     // ROUND TO WHOLE 4K PAGES WHEN LARGE SIZE REQUESTED, ALSO ALLOCATE 1 EXTRA WORD

        TempObEnd= *(TempBlocksEnd-1)+adjustedsize;
        if(newsize>0x40000) TempObEnd[-1]=newsize-1;

    }
    else TempObEnd = newend;
}

// RESIZE THE LAST OBJECT BY APPENDING WORDS AT THE END OF TEMPOB
// DOES NOT CREATE A NEW OBJECT OR A NEW BLOCK
// APPEND additionalsize WORDS TO THE END OF TEMPOB

void rplResizeLastObject(WORD additionalsize)
{


    // LARGE OBJECT SUPPORT
    WORD orgsize=TempObEnd-*(TempBlocksEnd-1);
    if(orgsize>0x40000) orgsize=*(TempObEnd-1);     // GET THE ACTUAL ORIGINAL SIZE OF THE BLOCK AS RECORDED
    WORD adjustedsize = ((orgsize+additionalsize)>0x3ffff)? ((orgsize+additionalsize+1+1023)&~0x3ff):(orgsize+additionalsize);     // ROUND TO WHOLE 4K PAGES WHEN LARGE SIZE REQUESTED, ALSO ALLOCATE 1 EXTRA WORD

    if(adjustedsize>0xffffc00) {
        rplException(EX_OUTOFMEM);
        return;                                // HARD LIMIT ON LARGE OBJECTS = 1_GiB - 4_kiB
    }

    if(TempObEnd + (adjustedsize-orgsize) + TEMPOBSLACK > TempObSize) {
        // ENLARGE TEMPOB AS NEEDED
        growTempOb((int32_t) (TempObEnd - TempOb) + (adjustedsize-orgsize) + TEMPOBSLACK);
        if(Exceptions)
            return;
    }

    TempObEnd += (adjustedsize-orgsize);

    if(orgsize+additionalsize>0x3FFFF) TempObEnd[-1]=orgsize+additionalsize;

}

// BORROW THE PATCH FUNCTION FROM THE GARBAGE COLLECTOR
void Patch(WORDPTR * start, WORDPTR * end, WORDPTR startfrom, WORDPTR endfrom,
        int32_t offset);

// GROW THE TEMPORARY OBJECT MEMORY

void growTempOb(WORD newtotalsize)
{
    WORDPTR *newtempob;
    WORD slack = newtotalsize - (WORD) (TempObEnd - TempOb);
    int32_t gc_done = 0;

    do {
        newtotalsize = (newtotalsize + 1023) & ~1023;

        newtempob =
                halGrowMemory(MEM_AREA_TEMPOB, (WORDPTR *) TempOb,
                newtotalsize);

        if(!newtempob) {
            if(!gc_done) {
                rplGCollect();
                ++gc_done;
                newtotalsize = (WORD) (TempObEnd - TempOb) + slack;
            }
            else {
                rplException(EX_OUTOFMEM);
                return;
            }
        }

    }
    while(!newtempob);

    if(TempOb && (((WORDPTR *) TempOb) != newtempob)) {
        // TEMPOB HAD TO BE MOVED IN MEMORY
        // FIX ALL DSTK/RSTK/TEMPBLOCKS/DIRECTORIES/LAMS POINTERS

        Patch(DStk, DSTop, TempOb, TempObSize, newtempob - (WORDPTR *) TempOb); // DATA STACK

        Patch(RStk, RSTop, TempOb, TempObSize, newtempob - (WORDPTR *) TempOb); // RETURN STACK

        Patch(LAMs, LAMTop, TempOb, TempObSize, newtempob - (WORDPTR *) TempOb);        // LOCAL VARIABLES

        Patch(Directories, DirsTop, TempOb, TempObSize, newtempob - (WORDPTR *) TempOb);        // GLOBAL VARIABLES

        Patch(GC_PTRUpdate, GC_PTRUpdate + MAX_GC_PTRUPDATE, TempOb, TempObSize + 1, newtempob - (WORDPTR *) TempOb);   // SYSTEM POINTERS, USE TempObSize+1 TO UPDATE POINTERS POINTING TO END OF TEMPOB TOO

        Patch(TempBlocks, TempBlocksEnd, TempOb, TempObSize, newtempob - (WORDPTR *) TempOb);   // ALL TEMPBLOCK POINTERS

    }
    TempOb = (WORDPTR) newtempob;
    TempObSize = TempOb + newtotalsize;
// FOR DEBUG ONLY
    //halCheckRplMemory();
}

// SHRINK THE TEMPORARY OBJECT MEMORY
// SAME AS GROW BUT DOESN'T GARBAGE COLLECT AND RETRY

void shrinkTempOb(WORD newtotalsize)
{
    WORDPTR *newtempob;

    newtotalsize = (newtotalsize + 1023) & ~1023;

    newtempob =
            halGrowMemory(MEM_AREA_TEMPOB, (WORDPTR *) TempOb, newtotalsize);

    if(!newtempob) {
        rplException(EX_OUTOFMEM);
        return;
    }
    if(TempOb && (((WORDPTR *) TempOb) != newtempob)) {
        // TEMPOB HAD TO BE MOVED IN MEMORY
        // FIX ALL DSTK/RSTK/TEMPBLOCKS/DIRECTORIES/LAMS POINTERS

        Patch(DStk, DSTop, TempOb, TempObSize, newtempob - (WORDPTR *) TempOb); // DATA STACK

        Patch(RStk, RSTop, TempOb, TempObSize, newtempob - (WORDPTR *) TempOb); // RETURN STACK

        Patch(LAMs, LAMTop, TempOb, TempObSize, newtempob - (WORDPTR *) TempOb);        // LOCAL VARIABLES

        Patch(Directories, DirsTop, TempOb, TempObSize, newtempob - (WORDPTR *) TempOb);        // GLOBAL VARIABLES

        Patch(GC_PTRUpdate, GC_PTRUpdate + MAX_GC_PTRUPDATE, TempOb, TempObSize + 1, newtempob - (WORDPTR *) TempOb);   // SYSTEM POINTERS, USE TempObSize+1 TO UPDATE POINTERS POINTING TO END OF TEMPOB TOO

        Patch(TempBlocks, TempBlocksEnd, TempOb, TempObSize, newtempob - (WORDPTR *) TempOb);   // ALL TEMPBLOCK POINTERS

    }
    TempOb = (WORDPTR) newtempob;
    TempObSize = TempOb + newtotalsize;
}

void growTempBlocks(WORD newtotalsize)
{
    WORDPTR *newtempblocks;
    WORD slack = newtotalsize - (WORD) (TempBlocksEnd - TempBlocks);
    int32_t gc_done = 0;

    do {
        newtotalsize = (newtotalsize + 1023) & ~1023;

        newtempblocks =
                halGrowMemory(MEM_AREA_TEMPBLOCKS, TempBlocks, newtotalsize);

        if(!newtempblocks) {
            if(!gc_done) {
                rplGCollect();
                ++gc_done;
                newtotalsize = (WORD) ((TempBlocksEnd + slack) - TempBlocks);
            }
            else {
                rplException(EX_OUTOFMEM);
                return;
            }
        }

    }
    while(!newtempblocks);

    TempBlocksEnd = TempBlocksEnd - TempBlocks + newtempblocks;
    TempBlocks = newtempblocks;
    TempBlocksSize = newtotalsize;
    if(TempBlocksEnd >= TempBlocks + TempBlocksSize)
        throw_dbgexception("Bad TempBlocksEnd!", 1);

    // NOTHING TO FIX
}

void shrinkTempBlocks(WORD newtotalsize)
{
    WORDPTR *newtempblocks;

    newtotalsize = (newtotalsize + 1023) & ~1023;

    newtempblocks =
            halGrowMemory(MEM_AREA_TEMPBLOCKS, TempBlocks, newtotalsize);

    if(!newtempblocks) {
        rplException(EX_OUTOFMEM);
        return;
    }
    TempBlocksEnd = TempBlocksEnd - TempBlocks + newtempblocks;
    TempBlocks = newtempblocks;
    TempBlocksSize = newtotalsize;

    if(TempBlocksEnd >= TempBlocks + TempBlocksSize)
        throw_dbgexception("Bad TempBlocksEnd!", EX_CONT | EX_RPLREGS);
    // NOTHING TO FIX
}

// TEMPBLOCKS ARE INCREASE AFTER FOR WRITE, DECREASE BEFORE FOR READ.

void rplAddTempBlock(WORDPTR block)
{
    *TempBlocksEnd++ = block;

    if(TempBlocksEnd + TEMPBLOCKSLACK >= TempBlocks + TempBlocksSize)
        growTempBlocks(TempBlocksEnd - TempBlocks + TEMPBLOCKSLACK + 1024);
    if(Exceptions)
        return;
}
