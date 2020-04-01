/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "hal_api.h"

// PROVIDE GARBAGE COLLECTION

// ATTEMPT ONE: CLASSIC MARK AND SWEEP

// memmoveb WILL PROPERLY WORK ON OVERLAPPING BLOCKS. CAREFUL IF USING OTHER CUSTOM IMPLEMENTATIONS
#define CloseHole(hole,start,end) memmovew((hole),(start), (WORD)(end-start))

// GET A TEMPBLOCK THAT CORRESPONDS OR CONTAINS THE GIVEN ADDRESS

WORDPTR *GetTempBlock(WORDPTR block)
{
    WORDPTR *left = TempBlocks;
    WORDPTR *right = TempBlocksEnd;
    WORDPTR *guess;
    do {
        guess = left + ((PTR2NUMBER) (right - left) / 2);       // BINARY SEARCH: DON'T USE left+right/2 BECAUSE OF OVERFLOW

        if(block == (WORDPTR) (((PTR2NUMBER) * guess) & ~((PTR2NUMBER) 3)))
            return guess;
        if(block > (WORDPTR) (((PTR2NUMBER) * guess) & ~((PTR2NUMBER) 3)))
            left = guess;
        else
            right = guess;
    }
    while(right - left > 1);

// BLOCK WASN'T FOUND, BUT IT'S WITHIN THE BLOCK
    return left;

}

// MARK ALL ADDRESSES IN TEMPBLOCK THAT ARE BEING REFERENCED

void Mark(WORDPTR * start, WORDPTR * end)
{
    WORDPTR *blockptr;
    while(start != end) {

        if((*start >= TempOb) && (*start < TempObEnd)) {
            // ONLY SEARCH POINTERS THAT POINT TO TEMPOB
            blockptr = GetTempBlock(*start);
            if(((WORDPTR) ((PTR2NUMBER) * blockptr & ~((PTR2NUMBER) 3))) ==
                    *start)
                *blockptr = (WORDPTR) ((PTR2NUMBER) * blockptr | 1);
            else
                *blockptr = (WORDPTR) ((PTR2NUMBER) * blockptr | 2);
        }

        ++start;
    }

}

/*
void CheckPTR(WORDPTR *start,WORDPTR *end)
{
WORDPTR *blockptr;
int cnt=0;

printf("TempOb=%08X, TempObEnd=%08X\n",(WORD)TempOb,(WORD)TempObEnd);
printf("TempObSize=%08X\n",(WORD)TempObSize);

while(start!=end) {

    if( (*start>=TempOb) && (*start<TempObEnd) ) {
        // ONLY SEARCH POINTERS THAT POINT TO TEMPOB
    }
    else {
        printf("Bad ptr [%d]: %08X\n",cnt,(WORD)*start);
    }

    ++cnt;
    ++start;
}

}
*/
void CheckTempBlocks()
{
    WORDPTR *ptr = TempBlocks;
    WORDPTR prevptr = NULL;
    int count = 0;

    while(ptr < TempBlocksEnd) {
        if((*ptr < TempOb) || (*ptr > TempObEnd))
            throw_dbgexception("Bad TempBlock", 1);
        if(*ptr < prevptr)
            throw_dbgexception("TempBlock out of order", 1);
        prevptr = *ptr;
        ++ptr;
        ++count;
    }
}

void Patch(WORDPTR * start, WORDPTR * end, WORDPTR startfrom, WORDPTR endfrom,
        BINT offset)
{
    while(start != end) {

        if((*start >= startfrom) && (*start < endfrom))
            *start += offset;
        ++start;
    }

}

// HIGH LEVEL GARBAGE COLLECTOR CALL
void rplGCollect()
{
    // FIRST, INITIALIZE THE COLLECTOR
    WORDPTR EndOfUsedMem, *EndOfRStk;
    int CompileBlock = 0;

    GCFlags = GC_IN_PROGRESS;

    // FOR DEBUG ONLY, VERIFY THAT AL TEMPBLOCKS ARE VALID
    //CheckTempBlocks();

    // MARK THE END OF USED TEMPOB, INCLUDING A PHANTOM BLOCK AFTER TempObEnd
    // WHICH IS USED DURING COMPILATION/DECOMPILATION
    // THIS BLOCK NEEDS TO BE PRESERVED AS A USED BLOCK!
    EndOfUsedMem = TempObEnd;
    EndOfRStk = RSTop;
    if((CompileEnd > EndOfUsedMem) && (CompileEnd <= TempObSize)) {
        EndOfUsedMem = CompileEnd;
        CompileBlock |= 1;
        if(DecompStringEnd == CompileEnd)
            CompileBlock |= 2;
    }
    if((DecompStringEnd > EndOfUsedMem) && (DecompStringEnd <= TempObSize)) {
        EndOfUsedMem =
                (WORDPTR) ((((PTR2NUMBER) DecompStringEnd) +
                    3) & ~((PTR2NUMBER) 3));
        CompileBlock |= 2;
    }

    if(CompileBlock) {
        // ADD THE BLOCK TO THE LIST USING THE SLACK SPACE TO AVOID TRIGGERING ANY ALLOCATION
        // MARK THE BLOCK AS USED AT THE SAME TIME
        *TempBlocksEnd++ = (WORDPTR) ((PTR2NUMBER) TempObEnd | 1);
        TempObEnd = EndOfUsedMem;
        if((ValidateTop > RSTop) && (ValidateTop < (RSTop + RStkSize)))
            EndOfRStk = ValidateTop;
    }

    *TempBlocksEnd = EndOfUsedMem;      // STORE THE END OF LAST BLOCK FOR CONVENIENCE (MARKED AS UNUSED)

    // MARK

    Mark(DStk, DSTop);  // DATA STACK

    Mark(RStk, RSTop);  // RETURN STACK

    Mark(LAMs, LAMTop); // LOCAL VARIABLES

    Mark(Directories, DirsTop); // GLOBAL VARIABLES

    Mark(GC_PTRUpdate, GC_PTRUpdate + MAX_GC_PTRUPDATE);        // SYSTEM POINTERS

//    CheckPTR(GC_PTRUpdate,GC_PTRUpdate+MAX_GC_PTRUPDATE);       // SYSTEM POINTERS
//    CheckTempBlocks();
    // SWEEP RUN

    WORDPTR StartBlock, EndBlock;
    WORDPTR *CheckIdx, *CleanIdx;
    BINT Offset;

    CheckIdx = TempBlocks;

    // FIND THE FIRST HOLE
    while(((PTR2NUMBER) (*CheckIdx)) & 3) {
        *CheckIdx = (WORDPTR) (((PTR2NUMBER) * CheckIdx) & ~((PTR2NUMBER) 3));  // CLEAN THE POINTER
        ++CheckIdx;     // NO NEED TO CHECK FOR END OF LIST, SINCE THE LAST BLOCK WAS MARKED UNUSED
    }

    CleanIdx = CheckIdx;
    EndBlock = *CheckIdx;
    do {

        // FIND THE END OF THE HOLE
        while(!(((PTR2NUMBER) (*CheckIdx)) & 3)) {

            if(CheckIdx == TempBlocksEnd) {

                EndOfUsedMem = EndBlock;
                // REMOVE PHANTOM BLOCK USING DURING COMPILE/DECOMPILE
                if(CompileBlock) {
                    // ALL POINTERS SHOULD'VE BEEN UPDATED AUTOMATICALLY
                    // EXCEPT THE ONES POINTING EXACTLY AT THE END OF USED MEMORY
                    if(CompileBlock & 2) {
                        if(DecompStringEnd > EndBlock)
                            DecompStringEnd = EndBlock;
                    }
                    if(CompileBlock & 1)
                        CompileEnd = EndBlock;
                    EndBlock = *(--CleanIdx);
                }

                // THIS HOLE IS AT THE END OF TEMPOB!
                // TRUNCATE TEMPOB
                TempObEnd = EndBlock;
                // TRUNCATE TEMPBLOCKS
                TempBlocksEnd = CleanIdx;

//            CheckPTR(GC_PTRUpdate,GC_PTRUpdate+MAX_GC_PTRUPDATE);       // SYSTEM POINTERS
//            CheckTempBlocks();

//            throw_exception("Successful GC",1);

                // RELEASE PAGES AT END OF TEMPOB AND TEMPBLOCKS
                shrinkTempOb(EndOfUsedMem - TempOb + TEMPOBSLACK);
                shrinkTempBlocks(TempBlocksEnd - TempBlocks + TEMPBLOCKSLACK);

                // AND ALL OTHER AREAS TOO
                shrinkDirs(DirsTop - Directories + DIRSLACK);
                shrinkLAMs(LAMTop - LAMs + LAMSLACK);
                shrinkRStk(EndOfRStk - RStk + RSTKSLACK);
                shrinkDStk(DSTop - DStk + DSTKSLACK);

                //halCheckMemoryMap();
                //halCheckRplMemory();

                GCFlags = GC_COMPLETED;

                return;
            }

            ++CheckIdx;
        }

        StartBlock =
                (WORDPTR) (((PTR2NUMBER) * (CheckIdx)) & ~((PTR2NUMBER) 3));
        Offset = (BINT) (EndBlock - StartBlock);

        // FIND END OF THE BLOCK TO MOVE

        while(((PTR2NUMBER) (*CheckIdx)) & 3) {
            *CleanIdx = (WORDPTR) ((PTR2NUMBER) (*CheckIdx + Offset) & ~((PTR2NUMBER) 3));      // STORE THE POINTER ALREADY PATCHED AT FINAL LOCATION
            ++CheckIdx; // NO NEED TO CHECK FOR END OF LIST, SINCE THE LAST BLOCK WAS MARKED UNUSED
            ++CleanIdx;
        }

        // MOVE THE MEMORY

        // HERE WE HAVE:
        // EndBlock = POINTER AFTER THE LAST COMPACTED BLOCK
        // StartBlock = POINTER TO THE BLOCK TO BE COMPACTED
        // *CheckIdx = END OF THE BLOCK TO BE COMPACTED, AND START OF THE NEXT HOLE

        CloseHole(EndBlock, StartBlock, *CheckIdx);

        // PATCH ALL POINTERS

        Patch(DStk, DSTop, StartBlock, *CheckIdx, Offset);      // DATA STACK

        Patch(RStk, EndOfRStk, StartBlock, *CheckIdx, Offset);  // RETURN STACK

        Patch(LAMs, LAMTop, StartBlock, *CheckIdx, Offset);     // LOCAL VARIABLES

        Patch(Directories, DirsTop, StartBlock, *CheckIdx, Offset);     // GLOBAL VARIABLES

        Patch(GC_PTRUpdate, GC_PTRUpdate + MAX_GC_PTRUPDATE, StartBlock, *CheckIdx, Offset);    // SYSTEM POINTERS

        EndBlock = *CheckIdx + Offset;  // END OF THE NEW COMPACTED BLOCK

    }
    while(1);

}
