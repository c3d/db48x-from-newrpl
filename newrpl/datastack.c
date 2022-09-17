/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "cmdcodes.h"
#include "libraries.h"
#include "newrpl.h"
#include "hal_api.h"

// GROW THE DATA STACK

void growDStk(WORD newtotalsize)
{
    word_p *newdstk;

    int32_t gc_done = 0;

    do {
        newtotalsize = (newtotalsize + 1023) & ~1023;

        newdstk = halGrowMemory(MEM_AREA_DSTK, DStk, newtotalsize);

        if(!newdstk) {
            if(!gc_done) {
                rplGCollect();
                ++gc_done;
            }
            else {
                rplException(EX_OUTOFMEM);
                return;
            }
        }

    }
    while(!newdstk);

    DSTop = DSTop - DStk + newdstk;
    DStk = newdstk;
    DStkSize = newtotalsize;

    // NO POINTERS SHOULD POINT INTO THE STACK, SO NOTHING TO FIX
}

void shrinkDStk(WORD newtotalsize)
{
    word_p *newdstk;

    newtotalsize = (newtotalsize + 1023) & ~1023;

    newdstk = halGrowMemory(MEM_AREA_DSTK, DStk, newtotalsize);

    if(!newdstk) {
        rplException(EX_OUTOFMEM);
        return;
    }

    DSTop = DSTop - DStk + newdstk;
    DStk = newdstk;
    DStkSize = newtotalsize;

    // NO POINTERS SHOULD POINT INTO THE STACK, SO NOTHING TO FIX
}

// DATA STACK ONLY STORES ADDRESS POINTERS
// NEVER DIRECTLY A WORD OR A COMMAND
// STACK IS "INCREASE AFTER" FOR STORE, "DECREASE BEFORE" FOR READ

// PUSH A POINTER ON THE STACK
// DOES NOT GROW THE STACK - GUARANTEED NOT TO CAUSE GC
// DON'T DO MORE THAN DSTKSLACK
void rplPushDataNoGrow(word_p p)
{
    *DSTop++ = p;
}

// PUSH A POINTER ON THE STACK
void rplPushData(word_p p)
{
    *DSTop++ = p;

    if(DStkSize <=
            (int32_t) ((DSTop - DStk +
                    DSTKSLACK) * sizeof(word_p) / sizeof(WORD)))
        growDStk((DSTop - DStk + DSTKSLACK) * sizeof(word_p) / sizeof(WORD));
    if(Exceptions)
        return;
}

// EXPAND THE STACK TO GUARANTEE THAT THERE'S SPACE TO PUSH numobjects MORE WITHOUT NEEDING ANY MEMORY
void rplExpandStack(int32_t numobjects)
{

    if(DStkSize <=
            (int32_t) ((DSTop - DStk + numobjects +
                    DSTKSLACK) * sizeof(word_p) / sizeof(WORD)))
        growDStk((DSTop - DStk + numobjects + DSTKSLACK) * sizeof(word_p) +
                sizeof(WORD));
    if(Exceptions)
        return;
}

// POP THE TOP OF THE STACK
word_p rplPopData()
{
    if(DSTop <= DStkProtect) {
        rplError(ERR_INTERNALEMPTYSTACK);
        return 0;
    }
    return *(--DSTop);
}

// DROP N LEVELS IN THE STACK
void rplDropData(int n)
{
    if(DSTop - DStkProtect < n) {
        rplError(ERR_INTERNALEMPTYSTACK);
        return;
    }
    DSTop -= n;
}

// REMOVE num LEVELS FROM level TO level+num-1 FROM THE STACK
void rplRemoveAtData(int32_t level, int32_t num)
{
    if(DSTop - DStkProtect < level + num - 1) {
        rplError(ERR_INTERNALEMPTYSTACK);
        return;
    }
    memmovew(DSTop - level - num + 1, DSTop - level + 1,
            (level - 1) * sizeof(word_p) / sizeof(WORD));
    DSTop -= num;
}

// THIS IS FOR LIBRARY USE ONLY, SO DON'T RAISE AN EXCEPTION, JUST RETURN AN NULL POINTER
// LEVELS = 1 ... DEPTH. DON'T CHECK FOR NEGATIVE NUMBERS!

// GET STACK DEPTH
inline int32_t rplDepthData()
{
    return (int32_t) (DSTop - DStkProtect);
}

// READ LEVEL 'N' WITHOUT REMOVING THE POINTER
inline word_p rplPeekData(int level)
{
    return *(DSTop - level);
}

// OVERWRITE LEVEL 'N' WITH A DIFFERENT POINTER
inline void rplOverwriteData(int level, word_p ptr)
{
    *(DSTop - level) = ptr;
}

// CLEAR THE STACK COMPLETELY
inline void rplClearData()
{
    DSTop = DStkProtect;
}

// PROTECT THE CURRENT DATA STACK FROM DROP AT THE CURRENT LEVEL

extern WORD unprotect_seco[];

word_p *rplProtectData()
{
    word_p *ret = DStkProtect;
    DStkProtect = DSTop;
    // ADD PROTECTION IN THE STACK FOR RECURSIVE USE
    rplPushRet((word_p) (DStk + (int32_t) (ret - DStkBottom)));
    rplPushRet(unprotect_seco);
    return ret;
}

// REMOVE STACK PROTECTION
word_p *rplUnprotectData()
{
    word_p *ret = DStkProtect;
    if(rplPeekRet(1) == unprotect_seco) {
        // REMOVE THE PROTECTION FROM THE RETURN STACK
        rplPopRet();
        int32_t protlevel = (int32_t) ((word_p *) rplPopRet() - DStk);
        if((DStkBottom + protlevel >= DStkBottom)
                && (DStkBottom + protlevel < DSTop))
            DStkProtect = DStkBottom + protlevel;
        else
            DStkProtect = DStkBottom;

    }
    else
        DStkProtect = DStkBottom;
    return ret;
}

// STACK SNAPSHOTS:
// SNAPSHOTS ARE NUMBERED FROM 1 TO N INCLUSIVE, WHERE N=rplCountSnapshots();
// SNAPSHOTS WORK LIKE A STACK OF STACKS.
// LEVEL 0 = CURRENT WORKING STACK, LEVEL 1 = LAST STORED SNAPSHOT, N=OLDEST STORED SNAPSHOT
// SWITCH TO A SNAPSHOT "ROLLS" IT DOWN TO LEVEL 0.
// TAKING A SNAPSHOT PUSHES THE CURRENT STACK TO SNAPSHOT LEVEL 1

// GET A COUNT OF HOW MANY SNAPSHOTS ARE CURRENTLY STORED IN THE STACK
int32_t rplCountSnapshots()
{
    int32_t count = 0;
    word_p *snapptr = DStkBottom;

    while(snapptr > DStk) {
        ++count;
        snapptr -= ((intptr_t) * (snapptr - 1)) + 1;
    }
    return count;
}

// REMOVES THE INDICATED SNAPSHOT
// MOVES THE ENTIRE DATA STACK, ALL POINTERS INTO THE STACK BECOME INVALID!
// MUST BE USED BY THE UI, NEVER WHILE AN RPL PROGRAM IS RUNNING.

void rplRemoveSnapshot(int32_t numsnap)
{
    word_p *snapptr = DStkBottom;
    word_p *prevptr;

    if(numsnap < 1)
        return;

    while((snapptr > DStk) && (numsnap > 0)) {
        prevptr = snapptr;
        snapptr -= ((intptr_t) * (snapptr - 1)) + 1;
        --numsnap;
    }

    if((numsnap > 0) || (snapptr < DStk)) {
        // INVALID SNAPSHOT, DON'T DELETE ANYTHING!
        return;
    }

    // MOVE THE ENTIRE STACK DOWN
    memmovew(snapptr, prevptr, (DSTop - prevptr) * (sizeof(void *) >> 2));
    // FIX THE POINTERS
    DSTop -= prevptr - snapptr;
    DStkBottom -= prevptr - snapptr;
    DStkProtect -= prevptr - snapptr;
    return;
}

// PUSH THE CURRENT STACK AS SNAPSHOT LEVEL1
// AND MAKE A COPY AS CURRENT STACK
void rplTakeSnapshot()
{
    word_p *top = DSTop, *bottom = DStkBottom;
    int32_t levels = top - bottom;
    // THIS IS NOT A POINTER, SO IT WILL CRASH IF AN APPLICATION TRIES TO BREAK
    // THE SNAPSHOT BARRIER
    *DSTop++ = NUMBER2PTR(levels);
    rplExpandStack(levels);
    if(Exceptions) {
        // RETURN WITHOUT MAKING AN UNDO MARK
        DSTop = top;
        if(Exceptions == EX_OUTOFMEM)
            Exceptions = 0;     // CLEAR OUT OF MEMORY ERROR
        return;
    }

    memcpyw(DSTop, bottom, levels * (sizeof(void *) >> 2));
    DStkProtect += levels + 1;
    DStkBottom += levels + 1;
    DSTop += levels;
    return;
}

// PUSH THE CURRENT STACK AS SNAPSHOT LEVEL1
// COPY ONLY nargs INTO THE CURRENT STACK
void rplTakeSnapshotN(int32_t nargs)
{
    word_p *top = DSTop, *bottom = DStkBottom;
    int32_t levels = top - bottom;
    if(levels < nargs)
        nargs = levels;
    // THIS IS NOT A POINTER, SO IT WILL CRASH IF AN APPLICATION TRIES TO BREAK
    // THE SNAPSHOT BARRIER
    *DSTop++ = NUMBER2PTR(levels);
    rplExpandStack(levels);
    if(Exceptions) {
        // RETURN WITHOUT MAKING AN UNDO MARK
        DSTop = top;
        if(Exceptions == EX_OUTOFMEM)
            Exceptions = 0;     // CLEAR OUT OF MEMORY ERROR

        return;
    }

    memcpyw(DSTop, top - nargs, nargs * (sizeof(void *) >> 2));
    DStkProtect += levels + 1;
    DStkBottom += levels + 1;
    DSTop += nargs;
    return;
}

// PUSH THE CURRENT STACK AS SNAPSHOT LEVEL1
// AND COPY ALL ITEMS TO CURRENT STACK
// THE SNAPSHOT WILL NOT CONTAIN THE FIRST nargs LEVEL OF THE STACK
// USED TO HIDE TEMPORARY ARGUMENTS DURING UNDO OPERATIONS
void rplTakeSnapshotHide(int32_t nargs)
{
    word_p *top = DSTop, *bottom = DStkBottom, hidefirst;
    int32_t levels = top - bottom;
    if(levels < nargs)
        nargs = levels;
    if(nargs < 0)
        nargs = 0;
    // THIS IS NOT A POINTER, SO IT WILL CRASH IF AN APPLICATION TRIES TO BREAK
    // THE SNAPSHOT BARRIER
    rplExpandStack(levels);
    if(Exceptions) {
        // RETURN WITHOUT MAKING AN UNDO MARK
        DSTop = top;
        if(Exceptions == EX_OUTOFMEM)
            Exceptions = 0;     // CLEAR OUT OF MEMORY ERROR

        return;
    }
    hidefirst = DSTop[-nargs];
    DSTop[-nargs] = NUMBER2PTR(levels - nargs);

    // COPY TO NEW STACK
    memmovew(DSTop - nargs + 1, bottom,
            (levels - nargs) * (sizeof(void *) >> 2));

    DStkProtect += levels - nargs + 1;
    DStkBottom += levels - nargs + 1;
    DSTop += levels - nargs + 1;
    DSTop[-nargs] = hidefirst;  // AND RESTORE THE DAMAGED WORD
    return;
}

// PUSH THE CURRENT STACK AS SNAPSHOT LEVEL1
// AND CLEAR THE CURRENT STACK

void rplTakeSnapshotAndClear()
{
    word_p *top = DSTop, *bottom = DStkBottom;
    int32_t levels = top - bottom;
    // THIS IS NOT A POINTER, SO IT WILL CRASH IF AN APPLICATION TRIES TO BREAK
    // THE SNAPSHOT BARRIER
    *DSTop++ = NUMBER2PTR(levels);

    DStkProtect += levels + 1;
    DStkBottom += levels + 1;
    return;
}

// RESET THE STACK TO A PREVIOUSLY SAVED SNAPSHOT
// THE CURRENT STACK WILL BE COMPELTELY OVERWRITTEN.
void rplRestoreSnapshot(int32_t numsnap)
{

    // RESTORE IS FORBIDDEN IN A PROTECTED STACK
    if(DStkProtect != DStkBottom)
        return;

    word_p *snapptr = DStkBottom;
    word_p *prevptr;

    if(numsnap < 1)
        return;

    while((snapptr > DStk) && (numsnap > 0)) {
        prevptr = snapptr;
        snapptr -= ((intptr_t) * (snapptr - 1)) + 1;
        --numsnap;
    }

    if((numsnap > 0) || (snapptr < DStk)) {
        // INVALID SNAPSHOT, DON'T DO ANYTHING!
        return;
    }

    int32_t levels = (intptr_t) * (prevptr - 1);

    rplExpandStack(levels - rplDepthData());
    if(Exceptions) {
        if(Exceptions == EX_OUTOFMEM)
            Exceptions = 0;     // CLEAR OUT OF MEMORY ERROR
        return;
    }

    // COPY THE SNAPSHOT TO CURRENT STACK
    memcpyw(DStkBottom, snapptr, levels * (sizeof(void *) >> 2));
    // ADJUST STACK POINTERS
    DSTop = DStkBottom + levels;
    DStkProtect = DStkBottom;   // PROTECTIONS ARE NOT SAVED WITHIN A SNAPSHOT

}

// ROLL A SNAPSHOT TO THE CURRENT STACK (REMOVING IT)
void rplRevertToSnapshot(int32_t numsnap)
{
    // REVERT IS FORBIDDEN IN A PROTECTED STACK
    if(DStkProtect != DStkBottom)
        return;
    rplRestoreSnapshot(numsnap);
    rplRemoveSnapshot(numsnap);
}

// RETURN THE STACK DEPTH IN THE GIVEN SNAPSHOT
int32_t rplDepthSnapshot(int32_t numsnap)
{
    word_p *snapptr = DStkBottom;
    word_p *prevptr;

    if(numsnap < 1)
        return 0;

    while((snapptr > DStk) && (numsnap > 0)) {
        prevptr = snapptr;
        snapptr -= ((intptr_t) * (snapptr - 1)) + 1;
        --numsnap;
    }

    if((numsnap > 0) || (snapptr < DStk)) {
        // INVALID SNAPSHOT, DON'T DO ANYTHING!
        return 0;
    }

    return (intptr_t) * (prevptr - 1);
}

// SAME AS rplPeekData() BUT IT CAN LOOK INTO SNAPSHOTS

word_p rplPeekSnapshot(int32_t numsnap, int32_t level)
{
    word_p *snapptr = DStkBottom;
    word_p *prevptr;

    if(numsnap < 1)
        return 0;

    while((snapptr > DStk) && (numsnap > 0)) {
        prevptr = snapptr;
        snapptr -= ((intptr_t) * (snapptr - 1)) + 1;
        --numsnap;
    }

    if((numsnap > 0) || (snapptr < DStk)) {
        // INVALID SNAPSHOT, DON'T DO ANYTHING!
        return 0;
    }

    int32_t levels = (intptr_t) * (prevptr - 1);

    if((level < 1) || (level > levels))
        return 0;       // DO NOT PEEK OUTSIDE THE CURRENT SNAPSHOT

    return *(prevptr - level - 1);
}

// DROPS THE CURRENT STACK AND MAKES THE PREVIOUS SNAPSHOT THE CURRENT STACK
// DOES NOT MOVE THE STACK, CAN BE USED FROM C

void rplDropCurrentStack()
{

    if(DStkBottom > DStk) {
        int32_t nlevels = ((intptr_t) * (DStkBottom - 1)) + 1;
        // FIX THE POINTERS
        DSTop = DStkBottom - 1;
        DStkBottom -= nlevels;
        DStkProtect -= nlevels;
    }
    else {
        DStkBottom = DStkProtect = DSTop = DStk;        // CLEAR THE STACK
    }
}

// REMOVE CURRENT STACK AND ALL SNAPSHOTS WHERE DStkBottom>newstkbottom
// THE SNAPSHOT WITH newstkbottom BECOMES THE CURRENT STACK
// USE TO REMOVE ALL SNAPSHOTS TAKEN BY A PROGRAM IN ONE SINGLE CLEANUP OPERATION

void rplCleanupSnapshots(word_p * newstkbottom)
{
    while((DStkBottom > newstkbottom) && (DStkBottom > DStk))
        rplDropCurrentStack();
}
