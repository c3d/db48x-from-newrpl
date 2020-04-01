/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "cmdcodes.h"
#include "hal_api.h"
#include "libraries.h"
#include "newrpl.h"

// GROW THE RETURN STACK

void growRStk(WORD newtotalsize)
{
    WORDPTR *newrstk;

    BINT gc_done = 0;

    do {
        newtotalsize = (newtotalsize + 1023) & ~1023;

        newrstk = halGrowMemory(MEM_AREA_RSTK, RStk, newtotalsize);

        if(!newrstk) {
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
    while(!newrstk);

    RSTop += newrstk - RStk;
    RStk = newrstk;
    RStkSize = newtotalsize;
}

void shrinkRStk(WORD newtotalsize)
{
    WORDPTR *newrstk;

    newtotalsize = (newtotalsize + 1023) & ~1023;

    newrstk = halGrowMemory(MEM_AREA_RSTK, RStk, newtotalsize);

    if(!newrstk) {
        rplException(EX_OUTOFMEM);
        return;
    }

    RSTop += newrstk - RStk;
    RStk = newrstk;
    RStkSize = newtotalsize;
}

// RETURN STACK ONLY STORES ADDRESS POINTERS
// NEVER DIRECTLY A WORD OR A COMMAND
// STACK IS "INCREASE AFTER" FOR STORE, "DECREASE BEFORE" FOR READ

void rplPushRet(WORDPTR p)
{
// PUSH FIRST (USE THE GUARANTEED SLACK)
// SO THAT IF GROWING THE RETURN STACK TRIGGERS A GC, THE POINTER WILL BE AUTOMATICALLY FIXED
    *RSTop++ = p;

    if(RStkSize <= RSTop - RStk + RSTKSLACK)
        growRStk((WORD) (RSTop - RStk + RSTKSLACK + 1024));
    if(Exceptions)
        return;
}

// PUSH WITHOUT GROWING THE STACK - USE CAREFULLY, USES THE GUARANTEED SLACK ONLY
void rplPushRetNoGrow(WORDPTR p)
{
// PUSH FIRST (USE THE GUARANTEED SLACK)
    *RSTop++ = p;

}

WORDPTR rplPopRet()
{
    if(RSTop <= RStk) {
        rplError(ERR_INTERNALEMPTYRETSTACK);
        return 0;
    }
    return *(--RSTop);
}

void rplDropRet(int nlevels)
{
    if(RSTop < RStk + nlevels) {
        rplError(ERR_INTERNALEMPTYRETSTACK);
        return;
    }
    RSTop -= nlevels;

}

// LOW LEVEL VERSION FOR USE IN LIBRARIES
inline WORDPTR rplPeekRet(int level)
{
    return (RSTop - level < RStk) ? 0 : *(RSTop - level);
}

void rplClearRStk()
{
    RSTop = RStk;       // CLEAR RETURN STACK, TYPICAL AFTER UNTRAPPED ERRORS
}

// RETURN THE DEPTH OF THE RETURN STACK
BINT rplDepthRet()
{
    return RSTop - RStk;
}
