/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "hal.h"

// GROW THE RETURN STACK

void growRStk(WORD newtotalsize)
{
    WORDPTR *newrstk;

    BINT gc_done=0;

    do {
    newtotalsize=(newtotalsize+1023)&~1023;

    newrstk=halGrowMemory(MEM_AREA_RSTK,RStk,newtotalsize);

    if(!newrstk) {
        if(!gc_done) { rplGCollect(); ++gc_done; }
        else {
            rplException(EX_OUTOFMEM);
            return;
        }
    }

    } while(!newrstk);

        RSTop+=newrstk-RStk;
        RStk=newrstk;
        RStkSize=newtotalsize;
}

// RETURN STACK ONLY STORES ADDRESS POINTERS
// NEVER DIRECTLY A WORD OR A COMMAND
// STACK IS "INCREASE AFTER" FOR STORE, "DECREASE BEFORE" FOR READ


void rplPushRet(WORDPTR p)
{
// PUSH FIRST (USE THE GUARANTEED SLACK)
// SO THAT IF GROWING THE RETURN STACK TRIGGERS A GC, THE POINTER WILL BE AUTOMATICALLY FIXED
*RSTop++=p;

if(RStkSize<=RSTop-RStk+RSTKSLACK) growRStk((WORD)(RSTop-RStk+RSTKSLACK+1024));
if(Exceptions) return;
}

WORDPTR rplPopRet()
{
    if(RSTop<=RStk) {
        Exceptions|=EX_EMPTYRSTK;
        ExceptionPointer=IPtr;
        return 0;
    }
    return *(--RSTop);
}

// LOW LEVEL VERSION FOR USE IN LIBRARIES
inline WORDPTR rplPeekRet(int level)
{
    return (RSTop-level<RStk)? 0: *(RSTop-level);
}

void rplClearRStk()
{
    RSTop=RStk; // CLEAR RETURN STACK, TYPICAL AFTER UNTRAPPED ERRORS
}
