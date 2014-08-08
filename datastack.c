/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "hal.h"


// GROW THE DATA STACK

void growDStk(WORD newtotalsize)
{
    WORDPTR *newdstk;

    BINT gc_done=0;

    do {
    newtotalsize=(newtotalsize+1023)&~1023;

    newdstk=hal_growmem(DStk,newtotalsize);

    if(!newdstk) {
        if(!gc_done) { rplGCollect(); ++gc_done; }
        else {
        Exceptions|=EX_OUTOFMEM;
        ExceptionPointer=IPtr;
        return;
        }
    }

    } while(!newdstk);


    DSTop=DSTop-DStk+newdstk;
    DStk=newdstk;
    DStkSize=newtotalsize;

        // NO POINTERS SHOULD POINT INTO THE STACK, SO NOTHING TO FIX
}

// DATA STACK ONLY STORES ADDRESS POINTERS
// NEVER DIRECTLY A WORD OR A COMMAND
// STACK IS "INCREASE AFTER" FOR STORE, "DECREASE BEFORE" FOR READ


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
        Exceptions|=EX_EMPTYSTACK;
        ExceptionPointer=IPtr;
        return 0;
    }
    return *(--DSTop);
}

// DROP N LEVELS IN THE STACK
void rplDropData(int n)
{
    if(DSTop-DStkProtect<n) {
        Exceptions|=EX_EMPTYSTACK;
        ExceptionPointer=IPtr;
        return;
    }
    DSTop-=n;
}

// THIS IS FOR LIBRARY USE ONLY, SO DON'T RAISE AN EXCEPTION, JUST RETURN AN NULL POINTER
// LEVELS = 1 ... DEPTH. DON'T CHECK FOR NEGATIVE NUMBERS!

// GET STACK DEPTH
inline BINT rplDepthData() { return (BINT)(DSTop-DStk); }
// READ LEVEL 'N' WITHOUT REMOVING THE POINTER
inline WORDPTR rplPeekData(int level)  {  return *(DSTop-level); }
// OVERWRITE LEVEL 'N' WITH A DIFFERENT POINTER
inline void rplOverwriteData(int level,WORDPTR ptr)  {  *(DSTop-level)=ptr; }

// PROTECT THE CURRENT DATA STACK FROM DROP AT THE CURRENT LEVEL


extern WORD unprotect_seco[];


WORDPTR *rplProtectData()
{
    WORDPTR *ret=DStkProtect;
    DStkProtect=DSTop;
    // ADD PROTECTION IN THE STACK FOR RECURSIVE USE
    rplPushRet(ret);
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
        DStkProtect=rplPopRet();
    } else DStkProtect=DStk;
    return ret;
}
