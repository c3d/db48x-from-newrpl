#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// PROVIDE GARBAGE COLLECTION

// ATTEMPT ONE: CLASSIC MARK AND SWEEP


// memmove WILL PROPERLY WORK ON OVERLAPPING BLOCKS. CAREFUL IF USING OTHER CUSTOM IMPLEMENTATIONS
#define CloseHole(hole,start,end) memmove((hole),(start), (WORD)(end)-(WORD)(start))


// GET A TEMPBLOCK THAT CORRESPONDS OR CONTAINS THE GIVEN ADDRESS

WORDPTR *GetTempBlock(WORDPTR block)
{
 WORDPTR *left=TempBlocks;
 WORDPTR *right=TempBlocksEnd;
 WORDPTR *guess;
 do {
     guess=left+((WORD)(right-left)/2);     // BINARY SEARCH: DON'T USE left+right/2 BECAUSE OF OVERFLOW

     if(block==(WORDPTR) (((WORD)*guess)&~3)) return guess;
     if(block>(WORDPTR) (((WORD)*guess)&~3)) left=guess;
     else right=guess;
 } while(right-left>1);

 // BLOCK WASN'T FOUND, BUT IT'S WITHIN THE BLOCK
 return left;

}




// MARK ALL ADDRESSES IN TEMPBLOCK THAT ARE BEING REFERENCED

void Mark(WORDPTR *start,WORDPTR *end)
{
WORDPTR *blockptr;
while(start!=end) {

    if( (*start>=TempOb) && (*start<TempObEnd) ) {
        // ONLY SEARCH POINTERS THAT POINT TO TEMPOB
        blockptr=GetTempBlock(*start);
        if( ((WORDPTR) ((WORD)*blockptr&~3))==*start) *blockptr=(WORDPTR)((WORD)*blockptr|1);
        else *blockptr=(WORDPTR)((WORD)*blockptr|2);
    }

    ++start;
}

}

void Patch(WORDPTR *start,WORDPTR *end,WORDPTR startfrom,WORDPTR endfrom,BINT offset)
{
while(start!=end) {

    if( (*start>=startfrom) && (*start<endfrom) ) *start+=offset;
    ++start;
}

}





// HIGH LEVEL GARBAGE COLLECTOR CALL
void rplGCollect()
{
    // FIRST, INITIALIZE THE COLLECTOR
    WORDPTR EndOfUsedMem;
    int CompileBlock=0;

    // MARK THE END OF USED TEMPOB, INCLUDING A PHANTOM BLOCK AFTER TempObEnd
    // WHICH IS USED DURING COMPILATION/DECOMPILATION
    // THIS BLOCK NEEDS TO BE PRESERVED AS A USED BLOCK!
    EndOfUsedMem=(CompileEnd>TempObEnd)? CompileEnd:TempObEnd;
    EndOfUsedMem=(DecompStringEnd>EndOfUsedMem)? DecompStringEnd:EndOfUsedMem;

    if(EndOfUsedMem>TempObEnd) {
        // ADD THE BLOCK TO THE LIST USING THE SLACK SPACE TO AVOID TRIGGERING ANY ALLOCATION
        // MARK THE BLOCK AS USED AT THE SAME TIME
        *TempBlocksEnd++=(WORDPTR)((WORD)TempObEnd|1);
        TempObEnd=EndOfUsedMem;
        CompileBlock=1;
    }

    *TempBlocksEnd=TempObEnd;   // STORE THE END OF LAST BLOCK FOR CONVENIENCE (MARKED AS UNUSED)

    // MARK

    Mark(DStk,DSTop);               // DATA STACK

    Mark(RStk,RSTop);               // RETURN STACK

    Mark(LAMs,LAMTop);              // LOCAL VARIABLES

    Mark(Directories,DirsTop);      // GLOBAL VARIABLES

    Mark(GC_PTRUpdate,GC_PTRUpdate+MAX_GC_PTRUPDATE);       // SYSTEM POINTERS

    // SWEEP RUN

    WORDPTR StartBlock,EndBlock;
    WORDPTR *CheckIdx,*CleanIdx;
    BINT Offset;

    CheckIdx=TempBlocks;



    // FIND THE FIRST HOLE
    while(((WORD)(*CheckIdx))&3) {
        *CheckIdx=(WORDPTR)(((WORD)*CheckIdx)&~3);  // CLEAN THE POINTER
        ++CheckIdx;        // NO NEED TO CHECK FOR END OF LIST, SINCE THE LAST BLOCK WAS MARKED UNUSED
    }

    CleanIdx=CheckIdx;
    EndBlock=*CheckIdx;
    do {

    // FIND THE END OF THE HOLE
    while(!(((WORD)(*CheckIdx))&3)) {

        if(CheckIdx==TempBlocksEnd) {

            // REMOVE PHANTOM BLOCK USING DURING COMPILE/DECOMPILE
            if(CompileBlock) {
                CompileEnd=DecompStringEnd=EndBlock;
                EndBlock=*(--CleanIdx);
            }


            // THIS HOLE IS AT THE END OF TEMPOB!
            // TRUNCATE TEMPOB
            TempObEnd=EndBlock;
            // TRUNCATE TEMPBLOCKS
            TempBlocksEnd=CleanIdx;





            // RELEASE PAGES AT END OF TEMPOB AND TEMPBLOCKS
            growTempOb(TempObEnd-TempOb+TEMPOBSLACK);
            growTempBlocks(TempBlocksEnd-TempBlocks+TEMPBLOCKSLACK);
            return;
        }

        ++CheckIdx;
    }

    StartBlock=(WORDPTR)(((WORD)*(CheckIdx))&~3);
    Offset=(BINT)(EndBlock-StartBlock);

    // FIND END OF THE BLOCK TO MOVE

    while(((WORD)(*CheckIdx))&3) {
        *CleanIdx=(WORDPTR)((WORD) (*CheckIdx+Offset)&~3);    // STORE THE POINTER ALREADY PATCHED AT FINAL LOCATION
        ++CheckIdx;        // NO NEED TO CHECK FOR END OF LIST, SINCE THE LAST BLOCK WAS MARKED UNUSED
        ++CleanIdx;
    }


    // MOVE THE MEMORY

    // HERE WE HAVE:
    // EndBlock = POINTER AFTER THE LAST COMPACTED BLOCK
    // StartBlock = POINTER TO THE BLOCK TO BE COMPACTED
    // *CheckIdx = END OF THE BLOCK TO BE COMPACTED, AND START OF THE NEXT HOLE

    CloseHole(EndBlock,StartBlock,*CheckIdx);


    // PATCH ALL POINTERS

    Patch(DStk,DSTop,StartBlock,*CheckIdx,Offset);      // DATA STACK

    Patch(RStk,RSTop,StartBlock,*CheckIdx,Offset);      // RETURN STACK

    Patch(LAMs,LAMTop,StartBlock,*CheckIdx,Offset);       // LOCAL VARIABLES

    Patch(Directories,DirsTop,StartBlock,*CheckIdx,Offset);      // GLOBAL VARIABLES

    Patch(GC_PTRUpdate,GC_PTRUpdate+MAX_GC_PTRUPDATE,StartBlock,*CheckIdx,Offset);       // SYSTEM POINTERS

    EndBlock=*CheckIdx+Offset; // END OF THE NEW COMPACTED BLOCK

    } while(1);

}




