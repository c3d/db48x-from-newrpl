/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// GROW THE DATA STACK

void growDStk(WORD newtotalsize)
{
    WORDPTR *newdstk;

    BINT gc_done=0;

    do {
    newtotalsize=(newtotalsize+1023)&~1023;

    newdstk=halGrowMemory(MEM_AREA_DSTK,DStk,newtotalsize);

    if(!newdstk) {
        if(!gc_done) { rplGCollect(); ++gc_done; }
        else {
            rplException(EX_OUTOFMEM);
            return;
        }
    }

    } while(!newdstk);


    DSTop=DSTop-DStk+newdstk;
    DStk=newdstk;
    DStkSize=newtotalsize;

        // NO POINTERS SHOULD POINT INTO THE STACK, SO NOTHING TO FIX
}

void shrinkDStk(WORD newtotalsize)
{
    WORDPTR *newdstk;

    newtotalsize=(newtotalsize+1023)&~1023;

    newdstk=halGrowMemory(MEM_AREA_DSTK,DStk,newtotalsize);

    if(!newdstk) {
            rplException(EX_OUTOFMEM);
            return;
        }

    DSTop=DSTop-DStk+newdstk;
    DStk=newdstk;
    DStkSize=newtotalsize;

        // NO POINTERS SHOULD POINT INTO THE STACK, SO NOTHING TO FIX
}




// DATA STACK ONLY STORES ADDRESS POINTERS
// NEVER DIRECTLY A WORD OR A COMMAND
// STACK IS "INCREASE AFTER" FOR STORE, "DECREASE BEFORE" FOR READ

// PUSH A POINTER ON THE STACK
// DOES NOT GROW THE STACK - GUARANTEED NOT TO CAUSE GC
// DON'T DO MORE THAN DSTKSLACK
void rplPushDataNoGrow(WORDPTR p)
{
    *DSTop++=p;
}

// PUSH A POINTER ON THE STACK
void rplPushData(WORDPTR p)
{
    *DSTop++=p;

if(DStkSize<=DSTop-DStk+DSTKSLACK) growDStk(DSTop-DStk+DSTKSLACK);
if(Exceptions) return;
}


// EXPAND THE STACK TO GUARANTEE THAT THERE'S SPACE TO PUSH numobjects MORE WITHOUT NEEDING ANY MEMORY
void rplExpandStack(BINT numobjects)
{
if(DStkSize<=DSTop-DStk+numobjects+DSTKSLACK) growDStk(DSTop-DStk+numobjects+DSTKSLACK);
if(Exceptions) return;
}



// POP THE TOP OF THE STACK
WORDPTR rplPopData()
{
    if(DSTop<=DStkProtect) {
        rplError(ERR_INTERNALEMPTYSTACK);
        return 0;
    }
    return *(--DSTop);
}

// DROP N LEVELS IN THE STACK
void rplDropData(int n)
{
    if(DSTop-DStkProtect<n) {
        rplError(ERR_INTERNALEMPTYSTACK);
        return;
    }
    DSTop-=n;
}

// THIS IS FOR LIBRARY USE ONLY, SO DON'T RAISE AN EXCEPTION, JUST RETURN AN NULL POINTER
// LEVELS = 1 ... DEPTH. DON'T CHECK FOR NEGATIVE NUMBERS!

// GET STACK DEPTH
inline BINT rplDepthData() { return (BINT)(DSTop-DStkProtect); }
// READ LEVEL 'N' WITHOUT REMOVING THE POINTER
inline WORDPTR rplPeekData(int level)  {  return *(DSTop-level); }
// OVERWRITE LEVEL 'N' WITH A DIFFERENT POINTER
inline void rplOverwriteData(int level,WORDPTR ptr)  {  *(DSTop-level)=ptr; }

// CLEAR THE STACK COMPLETELY
inline void rplClearData() { DSTop=DStkProtect; }

// PROTECT THE CURRENT DATA STACK FROM DROP AT THE CURRENT LEVEL


extern WORD unprotect_seco[];


WORDPTR *rplProtectData()
{
    WORDPTR *ret=DStkProtect;
    DStkProtect=DSTop;
    // ADD PROTECTION IN THE STACK FOR RECURSIVE USE
    rplPushRet((WORDPTR)(DStk+(BINT)(ret-DStkBottom)));
    rplPushRet(unprotect_seco);
    return ret;
}

// REMOVE STACK PROTECTION
WORDPTR *rplUnprotectData()
{
    WORDPTR *ret=DStkProtect;
    if(rplPeekRet(1)==unprotect_seco) {
        // REMOVE THE PROTECTION FROM THE RETURN STACK
        rplPopRet();
        BINT protlevel=(BINT) ((WORDPTR *)rplPopRet()-DStk);
        if((DStkBottom+protlevel>=DStkBottom) && (DStkBottom+protlevel<DSTop) )  DStkProtect=DStkBottom+protlevel;
        else DStkProtect=DStkBottom;

    } else DStkProtect=DStkBottom;
    return ret;
}

// STACK SNAPSHOTS:
// SNAPSHOTS ARE NUMBERED FROM 1 TO N INCLUSIVE, WHERE N=rplCountSnapshots();
// SNAPSHOTS WORK LIKE A STACK OF STACKS.
// LEVEL 0 = CURRENT WORKING STACK, LEVEL 1 = LAST STORED SNAPSHOT, N=OLDEST STORED SNAPSHOT
// SWITCH TO A SNAPSHOT "ROLLS" IT DOWN TO LEVEL 0.
// TAKING A SNAPSHOT PUSHES THE CURRENT STACK TO SNAPSHOT LEVEL 1


// GET A COUNT OF HOW MANY SNAPSHOTS ARE CURRENTLY STORED IN THE STACK
BINT rplCountSnapshots()
{
    BINT count=0;
    WORDPTR *snapptr=DStkBottom;

    while(snapptr>DStk) {
        ++count;
        snapptr-=((PTR2NUMBER)*(snapptr-1))+1;
    }
    return count;
}

// REMOVES THE INDICATED SNAPSHOT
// MOVES THE ENTIRE DATA STACK, ALL POINTERS INTO THE STACK BECOME INVALID!
// MUST BE USED BY THE UI, NEVER WHILE AN RPL PROGRAM IS RUNNING.

void rplRemoveSnapshot(BINT numsnap)
{
    WORDPTR *snapptr=DStkBottom;
    WORDPTR *prevptr;

    if(numsnap<1) return;

    while((snapptr>DStk)&&(numsnap>0)) {
        prevptr=snapptr;
        snapptr-=((PTR2NUMBER)*(snapptr-1))+1;
        --numsnap;
    }

    if((numsnap>0)||(snapptr<DStk)) {
        // INVALID SNAPSHOT, DON'T DELETE ANYTHING!
        return;
    }

    // MOVE THE ENTIRE STACK DOWN
    memmovew(snapptr,prevptr,(DSTop-prevptr)*(sizeof(void*)>>2));
    // FIX THE POINTERS
    DSTop-=prevptr-snapptr;
    DStkBottom-=prevptr-snapptr;
    DStkProtect-=prevptr-snapptr;
    return;
}

// PUSH THE CURRENT STACK AS SNAPSHOT LEVEL1
// AND MAKE A COPY AS CURRENT STACK
void rplTakeSnapshot()
{
    WORDPTR *top=DSTop,*bottom=DStkBottom;
    BINT levels=top-bottom;
    // THIS IS NOT A POINTER, SO IT WILL CRASH IF AN APPLICATION TRIES TO BREAK
    // THE SNAPSHOT BARRIER
    *DSTop++=NUMBER2PTR(levels);
    rplExpandStack(levels);
    if(Exceptions) {
        // RETURN WITHOUT MAKING AN UNDO MARK
        DSTop=top;
        return;
    }

    memcpyw(DSTop,bottom,levels*(sizeof(void*)>>2));
    DStkProtect+=levels+1;
    DStkBottom+=levels+1;
    DSTop+=levels;
    return;
}

// PUSH THE CURRENT STACK AS SNAPSHOT LEVEL1
// COPY ONLY nargs INTO THE CURRENT STACK
void rplTakeSnapshotN(BINT nargs)
{
    WORDPTR *top=DSTop,*bottom=DStkBottom;
    BINT levels=top-bottom;
    if(levels<nargs) nargs=levels;
    // THIS IS NOT A POINTER, SO IT WILL CRASH IF AN APPLICATION TRIES TO BREAK
    // THE SNAPSHOT BARRIER
    *DSTop++=NUMBER2PTR(levels);
    rplExpandStack(levels);
    if(Exceptions) {
        // RETURN WITHOUT MAKING AN UNDO MARK
        DSTop=top;
        return;
    }

    memcpyw(DSTop,top-nargs,nargs*(sizeof(void*)>>2));
    DStkProtect+=levels+1;
    DStkBottom+=levels+1;
    DSTop+=nargs;
    return;
}

// PUSH THE CURRENT STACK AS SNAPSHOT LEVEL1
// AND COPY ALL ITEMS TO CURRENT STACK
// THE SNAPSHOT WILL NOT CONTAIN THE FIRST nargs LEVEL OF THE STACK
// USED TO HIDE TEMPORARY ARGUMENTS DURING UNDO OPERATIONS
void rplTakeSnapshotHide(BINT nargs)
{
    WORDPTR *top=DSTop,*bottom=DStkBottom,hidefirst;
    BINT levels=top-bottom;
    if(levels<nargs) nargs=levels;
    if(nargs<0) nargs=0;
    // THIS IS NOT A POINTER, SO IT WILL CRASH IF AN APPLICATION TRIES TO BREAK
    // THE SNAPSHOT BARRIER
    rplExpandStack(levels);
    if(Exceptions) {
        // RETURN WITHOUT MAKING AN UNDO MARK
        DSTop=top;
        return;
    }
    hidefirst=DSTop[-nargs];
    DSTop[-nargs]=NUMBER2PTR(levels-nargs);

    // COPY TO NEW STACK
    memmovew(DSTop-nargs+1,bottom,(levels-nargs)*(sizeof(void*)>>2));

    DStkProtect+=levels-nargs+1;
    DStkBottom+=levels-nargs+1;
    DSTop+=levels-nargs+1;
    DSTop[-nargs]=hidefirst;    // AND RESTORE THE DAMAGED WORD
    return;
}

// PUSH THE CURRENT STACK AS SNAPSHOT LEVEL1
// AND CLEAR THE CURRENT STACK

void rplTakeSnapshotAndClear()
{
    WORDPTR *top=DSTop,*bottom=DStkBottom;
    BINT levels=top-bottom;
    // THIS IS NOT A POINTER, SO IT WILL CRASH IF AN APPLICATION TRIES TO BREAK
    // THE SNAPSHOT BARRIER
    *DSTop++=NUMBER2PTR(levels);

    DStkProtect+=levels+1;
    DStkBottom+=levels+1;
    return;
}

// RESET THE STACK TO A PREVIOUSLY SAVED SNAPSHOT
// THE CURRENT STACK WILL BE COMPELTELY OVERWRITTEN.
void rplRestoreSnapshot(BINT numsnap)
{

    // RESTORE IS FORBIDDEN IN A PROTECTED STACK
    if(DStkProtect!=DStkBottom) return;

    WORDPTR *snapptr=DStkBottom;
    WORDPTR *prevptr;

    if(numsnap<1) return;

    while((snapptr>DStk)&&(numsnap>0)) {
        prevptr=snapptr;
        snapptr-=((PTR2NUMBER)*(snapptr-1))+1;
        --numsnap;
    }

    if((numsnap>0)||(snapptr<DStk)) {
        // INVALID SNAPSHOT, DON'T DO ANYTHING!
        return;
    }


        BINT levels=(PTR2NUMBER)*(prevptr-1);

        rplExpandStack(levels-rplDepthData());
        if(Exceptions) {
            return;
        }

        // COPY THE SNAPSHOT TO CURRENT STACK
        memcpyw(DStkBottom,snapptr,levels*(sizeof(void*)>>2));
        // ADJUST STACK POINTERS
        DSTop=DStkBottom+levels;
        DStkProtect=DStkBottom; // PROTECTIONS ARE NOT SAVED WITHIN A SNAPSHOT

}

// ROLL A SNAPSHOT TO THE CURRENT STACK (REMOVING IT)
void rplRevertToSnapshot(BINT numsnap)
{
    // REVERT IS FORBIDDEN IN A PROTECTED STACK
    if(DStkProtect!=DStkBottom) return;
    rplRestoreSnapshot(numsnap);
    rplRemoveSnapshot(numsnap);
}

// RETURN THE STACK DEPTH IN THE GIVEN SNAPSHOT
BINT rplDepthSnapshot(BINT numsnap)
{
    WORDPTR *snapptr=DStkBottom;
    WORDPTR *prevptr;

    if(numsnap<1) return 0;

    while((snapptr>DStk)&&(numsnap>0)) {
        prevptr=snapptr;
        snapptr-=((PTR2NUMBER)*(snapptr-1))+1;
        --numsnap;
    }

    if((numsnap>0)||(snapptr<DStk)) {
        // INVALID SNAPSHOT, DON'T DO ANYTHING!
        return 0;
    }


        return (PTR2NUMBER)*(prevptr-1);
}

// SAME AS rplPeekData() BUT IT CAN LOOK INTO SNAPSHOTS

WORDPTR rplPeekSnapshot(BINT numsnap,BINT level)
{
    WORDPTR *snapptr=DStkBottom;
    WORDPTR *prevptr;

    if(numsnap<1) return 0;

    while((snapptr>DStk)&&(numsnap>0)) {
        prevptr=snapptr;
        snapptr-=((PTR2NUMBER)*(snapptr-1))+1;
        --numsnap;
    }

    if((numsnap>0)||(snapptr<DStk)) {
        // INVALID SNAPSHOT, DON'T DO ANYTHING!
        return 0;
    }


        BINT levels=(PTR2NUMBER)*(prevptr-1);

        if( (level<1) || (level>levels)) return 0;  // DO NOT PEEK OUTSIDE THE CURRENT SNAPSHOT

        return *(prevptr-level-1);
}
