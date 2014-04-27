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

if(DStkSize<=DSTop-DStk+DSTKSLACK) growDStk(DSTop-DStk+DSTKSLACK+1024);
if(Exceptions) return;
}


// POP THE TOP OF THE STACK
WORDPTR rplPopData()
{
    if(DSTop<=DStk) {
        Exceptions|=EX_EMPTYSTACK;
        ExceptionPointer=IPtr;
        return 0;
    }
    return *(--DSTop);
}

// DROP N LEVELS IN THE STACK
void rplDropData(int n)
{
    if(DSTop-DStk<n) {
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
