/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "hal.h"
#include "libraries.h"


// GROW THE LAM STACK

void growLAMs(WORD newtotalsize)
{
    WORDPTR *newlam;
    BINT gc_done=0;

    do {
    newtotalsize=(newtotalsize+1023)&~1023;

    newlam=halGrowMemory(MEM_AREA_LAM,LAMs,newtotalsize);

    if(!newlam) {
        if(!gc_done) { rplGCollect(); ++gc_done; }
        else {
            rplException(EX_OUTOFMEM);
        return;
        }
    }

    } while(!newlam);

        LAMTop=LAMTop-LAMs+newlam;
        nLAMBase=nLAMBase-LAMs+newlam;
        LAMs=newlam;
        LAMSize=newtotalsize;
}

void shrinkLAMs(WORD newtotalsize)
{
    WORDPTR *newlam;

    newtotalsize=(newtotalsize+1023)&~1023;

    newlam=halGrowMemory(MEM_AREA_LAM,LAMs,newtotalsize);

    if(!newlam) {
            rplException(EX_OUTOFMEM);
        return;
        }

        LAMTop=LAMTop-LAMs+newlam;
        nLAMBase=nLAMBase-LAMs+newlam;
        LAMs=newlam;
        LAMSize=newtotalsize;
}



// LAM STACK IS INCREASE AFTER FOR STORE, DECREASE BEFORE FOR READ
// RETURN THE LAM NUMBER IN THE CURRENT ENVIRONMENT (FOR USE WITH FAST GETLAMn FUNCTIONS)
BINT rplCreateLAM(WORDPTR nameobj,WORDPTR value)
{
    *LAMTop++=nameobj;
    *LAMTop++=value;

    if(LAMSize<=LAMTop-LAMs+LAMSLACK) growLAMs((WORD)(LAMTop-LAMs+LAMSLACK+1024));
    if(Exceptions) return 0;
    return (LAMTop-2-nLAMBase)>>1;

}


// CREATE A NEW TOP LAM ENVIRONMENT, SET THE OWNER TO THE GIVEN OBJECT
void rplCreateLAMEnvironment(WORDPTR owner)
{
    nLAMBase=LAMTop;
    rplCreateLAM((WORDPTR)lam_baseseco_bint,owner);
}


BINT rplCompareIDENTByName(WORDPTR id1,BYTEPTR name,BYTEPTR nameend)
{
BINT len=nameend-name;
BINT nwords=(len+3)>>2;
BINT extra=(nwords<<2)-len;
if ((*id1!=MKPROLOG(DOIDENT,nwords))&&(*id1!=MKPROLOG(DOIDENTEVAL,nwords))) return 0;

BYTEPTR ptr=(BYTEPTR) (id1+1);
while(len) {
    if(*ptr!=*name) return 0;
    ++ptr;
    ++name;
    --len;
}

// NAMES ARE IDENTICAL UP TO HERE
// CHECK IF THE STORED NAME HAS THE SAME PADDING
while(extra)
{
    if(*ptr) return 0;
    ++ptr;
    --extra;
}

return 1;
}

// COMPARE OBJECTS FOR EQUALITY IN THEIR DEFINITION
BINT rplCompareIDENT(WORDPTR id1,WORDPTR id2)
{
BINT nwords;

nwords=rplObjSize(id1);
// ADDED THIS SPECIAL CASE FOR IDENTS WITH VARIOUS FLAGS TO BE TREATED AS SAME
if( ISIDENT(*id1) && ISIDENT(*id2) )
{
    if(OBJSIZE(*id1)!=OBJSIZE(*id2)) return 0;
    ++id1;
    ++id2;
    --nwords;
}


 while(nwords) {
     if(*id1!=*id2) return 0;
     ++id1;
     ++id2;
     --nwords;
 }
return 1;
}

// COMPARE OBJECTS FOR EQUALITY IN THEIR DEFINITION
BINT rplCompareObjects(WORDPTR id1,WORDPTR id2)
{
if(id1==id2) return 1;

BINT nwords;

nwords=rplObjSize(id1);

while(nwords) {
     if(*id1!=*id2) return 0;
     ++id1;
     ++id2;
     --nwords;
 }
return 1;
}

// FINDS A LAM, AND RETURNS THE ADDRESS OF THE KEY/VALUE PAIR WITHIN THE LAM ENVIRONMENT
// DOES NOT STOP FOR CURRENT SECONDARY

WORDPTR *rplFindLAMbyName(BYTEPTR name, BINT len, BINT scanparents)
{
    WORDPTR *ltop=LAMTop,*stop=scanparents? LAMs: (nLAMBase? nLAMBase:LAMs);

while(ltop>stop) {
    ltop-=2;
    if(rplCompareIDENTByName(*ltop,name,name+len)) return ltop;
}
return 0;
}



WORDPTR *rplFindLAM(WORDPTR nameobj,BINT scanparents)
{
    WORDPTR *ltop=LAMTop,*stop=scanparents? LAMs: (nLAMBase? nLAMBase:LAMs);
while(ltop>stop) {
    ltop-=2;
    if(rplCompareIDENT(nameobj,*ltop)) return ltop;
}
return 0;
}

// RECLAIMS A LAM VALUE, FROM CURRENT SECO OR PARENTS

WORDPTR rplGetLAM(WORDPTR nameobj)
{
    WORDPTR *ltop=LAMTop;
while(ltop>LAMs) {
    ltop-=2;
    if(rplCompareIDENT(nameobj,*ltop)) return *(ltop+1);
}
return 0;
}

// VERY FAST GETLAM, NO ERROR CHECKS!
// ONLY USED BY SYSTEM LIBRARIES
inline WORDPTR *rplGetLAMn(BINT idx)
{
    return nLAMBase+2*idx+1;
}

// RETURN A POINTER TO THE THE NAME OF THE LAM, INSTEAD OF ITS CONTENTS
inline WORDPTR *rplGetLAMnName(BINT idx)
{
    return nLAMBase+2*idx;
}

// VERY FAST GETLAM, NO ERROR CHECKS!
// ONLY USED BY SYSTEM LIBRARIES
// GET THE CONTENT OF A LAM IN A GIVEN ENVIRONMENT
inline WORDPTR *rplGetLAMnEnv(WORDPTR *LAMEnv,BINT idx)
{
    return (WORDPTR *)(LAMEnv+2*idx+1);
}

// RETURN THE NAME OF THE LAM, INSTEAD OF ITS CONTENTS
// IN THE GIVEN ENVIRONMENT
inline WORDPTR *rplGetLAMnNameEnv(WORDPTR *LAMEnv,BINT idx)
{
    return (WORDPTR *)(LAMEnv+2*idx);
}


// FAST PUTLAM, NO ERROR CHECKS!
inline void rplPutLAMn(BINT idx,WORDPTR object)
{
    nLAMBase[2*idx+1]=object;
}

// COUNT HOW MANY LAMS IN THE GIVEN ENVIRONMENT
// LAMS ARE NUMBERED FROM 1 TO THE RETURNED NUMBER (INCLUSIVE)
// IF GIVEN ENVIRONMENT IS NULL, RETURN COUNT ON THE TOP ENVIRONMENT
BINT rplLAMCount(WORDPTR *LAMEnvironment)
{
    // FIND THE END OF THE GIVEN ENVIRONMENT
    if(!LAMEnvironment) LAMEnvironment=nLAMBase;
    WORDPTR *endofenv=LAMEnvironment;
    endofenv+=2;
    while(endofenv<LAMTop) { if(*endofenv==lam_baseseco_bint) break; endofenv+=2; }
    return ((BINT)(endofenv-LAMEnvironment-2))>>1;
}


// REMOVE ALL LAMS CREATED BY THE GIVEN SECONDARY AND BELOW
// OR JUST THE LAST ENVIRONMENT IF currentseco==0
void rplCleanupLAMs(WORDPTR currentseco)
{
    WORDPTR *ltop=LAMTop;
while(ltop>LAMs) {
    ltop-=2;
    if(**ltop==LAM_BASESECO) {
            if(((*(ltop+1))==currentseco)||(currentseco==0)) {
                LAMTop=ltop;
                nLAMBase=rplGetNextLAMEnv(LAMTop);
                return;
            }
    }
}
// NOT FOUND, LEAVE IT ALONE (THIS SECO MAY NOT HAVE CREATED ANY LOCALS)
return;

}


// FIND THE NEXT ENVIRONMENT MARKER FROM THE GIVEN STARTING POINT
// STARTING POINT IS NOT INCLUDED IN THE SEARCH, SO CALLS
// PASSING THE PREVIOUS ENVIRONMENT AS STARTING POINT WILL
// FIND THE NEXT ONE. TO GET THE INNERMOST ENVIRONMENT
// GIVE LAMTop AS STARTING POINT

WORDPTR *rplGetNextLAMEnv(WORDPTR *startpoint)
{
while(startpoint>LAMs) {
    startpoint-=2;
    if(**startpoint==LAM_BASESECO) return startpoint;
    }
// NOT FOUND, RETURN NULL
return 0;
}



// DETERMINE WHETHER A NEW ENVIRONMENT IS NEEDED TO CREATE A LOCAL
// BASED ON CURRENT EXECUTION STATE:
// A) IF CURRENT SECONDARY DOESN'T HAVE AN ENVIRONMENT, CREATE ONE
// B) IF EXISTING ENVIRONMENT IS INSIDE A LOOP OR OTHER CONSTRUCT, NO NEED FOR ONE
BINT rplNeedNewLAMEnv()
{
    // FIRST DETERMINE THE ADDRESS OF THE CURRENT SECONDARY
    WORDPTR *rsptr=RSTop-1;
    WORDPTR seco=0;

    while(rsptr>=RStk) {
        if(ISPROLOG(**rsptr) && ((LIBNUM(**rsptr)==SECO)||(LIBNUM(**rsptr)==DOCOL)))
        {
            seco=*rsptr;
            break;
        }
        --rsptr;
    }

    // NOW DETERMINE IF THE CURRENT SECONDARY HAS AN ENVIRONMENT
    if(!seco) {
        // NOT IN A SECONDARY OR IN A SECONDARY INDIRECTLY EXECUTED WITH EVAL

        if(nLAMBase!=LAMTop && nLAMBase>=LAMs) {
            // THERE IS AN EXISTING ENVIRONMENT
            rsptr=RSTop-1;
            if(rsptr<RStk) return 0;    // THERE'S NO RETURN ADDRESS, WE ARE EXECUTING FREE COMMANDS FROM THE COMMAND LINE
            if( **rsptr==(CMD_OVR_EVAL)) {
                // WE ARE EXECUTING A SECONDARY THAT WAS 'EVAL'd, NEED A NEW ENVIRONMENT
                return 1;
            }
            if( ISPROLOG(**rsptr) && (LIBNUM(**rsptr)==DOIDENTEVAL)) {
                // WE ARE EXECUTING A SECONDARY THAT WAS DIRECTLY EXECUTED FROM AN IDENT, NEED A NEW ENVIRONMENT
                return 1;
            }

            return 0;
        }

        return 1; // WE ARE NOT INSIDE ANY SECONDARY, SO AN ENVIRONMENT IS NEEDED
    }


    if(nLAMBase>=LAMs && nLAMBase<LAMTop) {
        if((ISPROLOG(**(nLAMBase+1)) && ((LIBNUM(*(nLAMBase+1))==SECO)||(LIBNUM(*(nLAMBase+1))==DOCOL)))
        || (**(nLAMBase+1)==(CMD_OVR_EVAL))) {
            // THIS ENVIRONMENT BELONGS TO A SECONDARY
            if(*(nLAMBase+1)==seco) {
                // FOUND AN ENVIRONMENT THAT BELONGS TO THE CURRENT SECONDARY, SO NO NEED FOR ONE
                return 0;
            }
            else return 1;  // THE CURRENT ENVIRONMENT BELONGS TO A DIFFERENT SECONDARY, CREATE A NEW ONE
        }
        else {
            // CURRENT ENVIRONMENT BELONGS TO SOMETHING NOT-A-SECO (LOOPS, ETC.)
            rsptr=RSTop-1;
            if(rsptr<RStk) return 0;    // THERE'S NO RETURN ADDRESS, WE ARE EXECUTING FREE COMMANDS FROM THE COMMAND LINE
            if( **rsptr==(CMD_OVR_EVAL)) {
                // WE ARE EXECUTING A SECONDARY THAT WAS 'EVAL'd, NEED A NEW ENVIRONMENT
                return 1;
            }
            if( ISPROLOG(**rsptr) && (LIBNUM(**rsptr)==DOIDENTEVAL)) {
                // WE ARE EXECUTING A SECONDARY THAT WAS DIRECTLY EXECUTED FROM AN IDENT, NEED A NEW ENVIRONMENT
                return 1;
            }

            // NO NEED FOR AN ENVIRONMENT
            return 0;
        }
    }

    // THIS SECONDARY DOESN'T HAVE ANY LOCALS
    return 1;


}



// SAME AS rplNeedNewLAMEnv() BUT TO BE USED DURING COMPILATION, FOR LOCAL VAR TRACING.
BINT rplNeedNewLAMEnvCompiler()
{
    // FIRST DETERMINE THE ADDRESS OF THE CURRENT SECONDARY
    WORDPTR *rsptr=ValidateTop-1;
    WORDPTR seco=0;

    while(rsptr>=RSTop) {
        if(ISPROLOG(**rsptr) && ((LIBNUM(**rsptr)==SECO)||(LIBNUM(**rsptr)==DOCOL)))
        {
            seco=*rsptr;
            break;
        }
        --rsptr;
    }

    // NOW DETERMINE IF THE CURRENT SECONDARY HAS AN ENVIRONMENT
    if(!seco) {
        // NOT IN A SECONDARY OR IN A SECONDARY INDIRECTLY EXECUTED WITH EVAL

        if(nLAMBase!=LAMTop && nLAMBase>=LAMTopSaved) {
            // THERE IS AN EXISTING ENVIRONMENT
            return 0;
        }


        return 1; // WE ARE NOT INSIDE ANY SECONDARY, SO AN ENVIRONMENT IS NEEDED
    }


    if(nLAMBase>=LAMTopSaved && nLAMBase<LAMTop) {
        if(ISPROLOG(**(nLAMBase+1)) && ((LIBNUM(**(nLAMBase+1))==SECO)||(LIBNUM(**(nLAMBase+1))==DOCOL))) {
            // THIS ENVIRONMENT BELONGS TO A SECONDARY
            if(*(nLAMBase+1)==seco) {
                // FOUND AN ENVIRONMENT THAT BELONGS TO THE CURRENT SECONDARY, SO NO NEED FOR ONE
                return 0;
            }
            else return 1;  // THE CURRENT ENVIRONMENT BELONGS TO A DIFFERENT SECONDARY, CREATE A NEW ONE
        }
        else {
            // CURRENT ENVIRONMENT BELONGS TO SOMETHING NOT-A-SECO (LOOPS)
            // NO NEED FOR AN ENVIRONMENT
            return 0;
        }
    }

    // THIS SECONDARY DOESN'T HAVE ANY LOCALS
    return 1;


}


void rplClearLAMs()
{
    LAMTop=nLAMBase=LAMs;
}
