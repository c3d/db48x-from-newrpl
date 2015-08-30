/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "hal.h"


// ALLOCATES MEMORY FROM TEMPOB
// SIZE IS THE NUMBER OF WORDS REQUIRED, NOT COUNTING THE PROLOG!
// IT AUTOMATICALLY RESERVES ONE EXTRA WORD FOR THE PROLOG

WORDPTR rplAllocTempOb(WORD size)
{
    // SIMPLY ADD A NEW BLOCK AT END OF CHAIN

    if( TempObEnd+size+1+TEMPOBSLACK>TempObSize) {
        // ENLARGE TEMPOB AS NEEDED
        growTempOb((BINT)(TempObEnd-TempOb)+size+1+TEMPOBSLACK);
        if(Exceptions) return 0;
    }

    rplAddTempBlock(TempObEnd);
    if(Exceptions) return 0;

    WORDPTR ptr=TempObEnd;
    TempObEnd+=size+1;
    return ptr;
}


// TRUNCATES A RECENTLY ALLOCATED BLOCK AT THE END OF TEMPOB
// IT HAS TO BE THE LAST BLOCK ALLOCATED WITH ALLOCTEMPOB
void rplTruncateLastObject(WORDPTR newend)
{

    if(newend<=*(TempBlocksEnd-1)) {
        // TRIED TO TRUNCATE BEYOND THE LAST OBJECT, CAN'T ALLOW THAT
        // REMOVE LAST ALLOCATED BLOCK COMPLETELY
        --TempBlocksEnd;
        TempObEnd=*TempBlocksEnd;
        return;
    }
    TempObEnd=newend;
}

// RESIZE THE LAST OBJECT BY APPENDING WORDS AT THE END OF TEMPOB
// DOES NOT CREATE A NEW OBJECT OR A NEW BLOCK
// APPEND additionalsize WORDS TO THE END OF TEMPOB

void rplResizeLastObject(WORD additionalsize)
{
    if( TempObEnd+additionalsize+TEMPOBSLACK>TempObSize) {
        // ENLARGE TEMPOB AS NEEDED
        growTempOb((BINT)(TempObEnd-TempOb)+additionalsize+TEMPOBSLACK);
        if(Exceptions) return;
    }

    TempObEnd+=additionalsize;

}



// BORROW THE PATCH FUNCTION FROM THE GARBAGE COLLECTOR
extern void Patch(WORDPTR *start,WORDPTR *end,WORDPTR startfrom,WORDPTR endfrom,BINT offset);

// GROW THE TEMPORARY OBJECT MEMORY

void growTempOb(WORD newtotalsize)
{
    WORDPTR *newtempob;
    WORD slack=newtotalsize-(WORD)(TempObEnd-TempOb);
    BINT gc_done=0;

    do {
    newtotalsize=(newtotalsize+1023)&~1023;

    newtempob=halGrowMemory(MEM_AREA_TEMPOB,(WORDPTR *)TempOb,newtotalsize);

    if(!newtempob) {
        if(!gc_done) { rplGCollect(); ++gc_done; newtotalsize=(WORD)(TempObEnd-TempOb)+slack; }
        else {
            rplException(EX_OUTOFMEM);
        return;
        }
    }

    } while(!newtempob);

    if(TempOb && (((WORDPTR *)TempOb)!=newtempob)) {
            // TEMPOB HAD TO BE MOVED IN MEMORY
            // FIX ALL DSTK/RSTK/TEMPBLOCKS/DIRECTORIES/LAMS POINTERS

            Patch(DStk,DSTop,TempOb,TempObSize,newtempob-(WORDPTR *)TempOb);      // DATA STACK

            Patch(RStk,RSTop,TempOb,TempObSize,newtempob-(WORDPTR *)TempOb);      // RETURN STACK

            Patch(LAMs,LAMTop,TempOb,TempObSize,newtempob-(WORDPTR *)TempOb);       // LOCAL VARIABLES

            Patch(Directories,DirsTop,TempOb,TempObSize,newtempob-(WORDPTR *)TempOb);      // GLOBAL VARIABLES

            Patch(GC_PTRUpdate,GC_PTRUpdate+MAX_GC_PTRUPDATE,TempOb,TempObSize+1,newtempob-(WORDPTR *)TempOb);       // SYSTEM POINTERS, USE TempObSize+1 TO UPDATE POINTERS POINTING TO END OF TEMPOB TOO

            Patch(TempBlocks,TempBlocksEnd,TempOb,TempObSize,newtempob-(WORDPTR *)TempOb); // ALL TEMPBLOCK POINTERS

        }
        TempOb=(WORDPTR) newtempob;
        TempObSize=TempOb+newtotalsize;
// FOR DEBUG ONLY
        //halCheckRplMemory();
}

// SHRINK THE TEMPORARY OBJECT MEMORY
// SAME AS GROW BUT DOESN'T GARBAGE COLLECT AND RETRY

void shrinkTempOb(WORD newtotalsize)
{
    WORDPTR *newtempob;

    newtotalsize=(newtotalsize+1023)&~1023;

    newtempob=halGrowMemory(MEM_AREA_TEMPOB,(WORDPTR *)TempOb,newtotalsize);

    if(!newtempob) {
        rplException(EX_OUTOFMEM);
        return;
        }

     TempOb=(WORDPTR) newtempob;
     TempObSize=TempOb+newtotalsize;
}




void growTempBlocks(WORD newtotalsize)
{
    WORDPTR *newtempblocks;
    WORD slack=newtotalsize-(WORD)(TempBlocksEnd-TempBlocks);
    BINT gc_done=0;

    do {
    newtotalsize=(newtotalsize+1023)&~1023;

    newtempblocks=halGrowMemory(MEM_AREA_TEMPBLOCKS,TempBlocks,newtotalsize);

    if(!newtempblocks) {
        if(!gc_done) { rplGCollect(); ++gc_done; newtotalsize=(WORD)((TempBlocksEnd+slack)-TempBlocks); }
        else {
            rplException(EX_OUTOFMEM);
        return;
        }
    }

    } while(!newtempblocks);

        TempBlocksEnd=TempBlocksEnd-TempBlocks+newtempblocks;
        TempBlocks=newtempblocks;
        TempBlocksSize=newtotalsize;
        if(TempBlocksEnd>=TempBlocks+TempBlocksSize) throw_dbgexception("Bad TempBlocksEnd!",1);

        // NOTHING TO FIX
}

void shrinkTempBlocks(WORD newtotalsize)
{
    WORDPTR *newtempblocks;

    newtotalsize=(newtotalsize+1023)&~1023;

    newtempblocks=halGrowMemory(MEM_AREA_TEMPBLOCKS,TempBlocks,newtotalsize);

    if(!newtempblocks) {
        rplException(EX_OUTOFMEM);
        return;
        }
        TempBlocksEnd=TempBlocksEnd-TempBlocks+newtempblocks;
        TempBlocks=newtempblocks;
        TempBlocksSize=newtotalsize;

        if(TempBlocksEnd>=TempBlocks+TempBlocksSize) throw_dbgexception("Bad TempBlocksEnd!",1);
        // NOTHING TO FIX
}


// TEMPBLOCKS ARE INCREASE AFTER FOR WRITE, DECREASE BEFORE FOR READ.

void rplAddTempBlock(WORDPTR block)
{
    *TempBlocksEnd++=block;

if(TempBlocksEnd+TEMPBLOCKSLACK>=TempBlocks+TempBlocksSize)
    growTempBlocks(TempBlocksEnd-TempBlocks+TEMPBLOCKSLACK+1024);
if(Exceptions) return;
}
