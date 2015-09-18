/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include "newrpl.h"
#include "libraries.h"




// EXPLODES THE UNIT OBJECT IN THE STACK
// RETURNS THE NUMBER OF LEVELS USED IN THE STACK
// (n): VALUE
// (n-1): IDENTIFIER
// (n-2): NUMERATOR
// (n-3): DENOMINATOR
// (...): [MORE IDENTIFIERS AND THEIR EXPONENTS]
// (3): LAST IDENTIFIER
// (2): NUMERATOR
// (1): DENOMINATOR

BINT rplUnitExplode(WORDPTR unitobj)
{
    WORDPTR *savestk=DSTop;
if(!ISUNIT(*unitobj)) return 0;

ScratchPointer1=unitobj;
ScratchPointer2=rplSkipOb(unitobj);
BINT count=0;
++ScratchPointer1;
while(ScratchPointer1<ScratchPointer2) {
    // PUSH ALL OBJECTS IN THE STACK AS-IS
    rplPushData(ScratchPointer1);
    ++count;
    if(Exceptions) { DSTop=savestk; return 0; }
    ScratchPointer1=rplSkipOb(ScratchPointer1);
}
return count;
}

// COMPOSE A UNIT OBJECT FROM AN EXPLODED UNIT IN THE STACK
// INVERSE OPERATION OR rplUnitExplode()
// EXPECTS AN EXPLODED UNIT IN THE STACK
// DOES NOT CLEAN UP THE STACK
// WARNING: THIS IS LOW-LEVEL, NO VALIDITY CHECKS DONE HERE

WORDPTR rplUnitImplode(BINT nlevels)
{
    // COMPUTE THE REQUIRED SIZE
    BINT size=0;
    BINT lvl;

    for(lvl=1;lvl<=nlevels;++lvl) size+=rplObjSize(rplPeekData(lvl));

    // ALLOCATE MEMORY FOR THE NEW OBJECT
    WORDPTR newobj=rplAllocTempOb(size);
    WORDPTR newptr;
    if(!newobj) return 0;

    // AND FILL IT UP
    newobj[0]=MKPROLOG(DOUNIT,size);
    newptr=newobj+1;
    for(lvl=nlevels;lvl>=1;--lvl) {
    rplCopyObject(newptr,rplPeekData(lvl));
    newptr=rplSkipOb(newptr);
    }

    return newobj;

}

// REMOVE AN ITEM AT THE GIVEN LEVEL OF THE STACK, INCLUDING ITS EXPONENT IF IT'S AN IDENT

void rplUnitPopItem(BINT level)
{
    BINT nitems;

    if(level>rplDepthData()) return;

    if(ISIDENT(*rplPeekData(level))) nitems=3;
    else nitems=1;

    if(level-nitems<=0) {
        rplPopData(level);
        return;
    }

    memmove(DSTop-level,DSTop-level+nitems,level-nitems);
    DSTop-=nitems;
}


// COPY THE ITEM AT THE BOTTOM OF THE STACK
void rplUnitPickItem(BINT level)
{
    BINT nitems;

    if(level>rplDepthData()) return;

    if(ISIDENT(*rplPeekData(level))) nitems=3;
    else nitems=1;

    if(level-nitems<0) return;

    while(nitems--) rplPushData(rplPeekData(level));

}

// TAKES 2 VALUES OR 2 IDENTIFIERS AND MULTIPLIES THEM TOGETHER
// REMOVES BOTH ORIGINALS FROM THE STACK AND ADDS THE RESULT AT THE BOTTOM OF THE STACK
// LOW LEVEL, NO CHECKS OF ANY KIND DONE HERE

void rplUnitMulItem(BINT level1,BINT level2)
{
    if(ISIDENT(*rplPeekData(level1)) && ISIDENT(*rplPeekData(level2))) {
        // MULTIPLY 2 IDENTIFIERS BY ADDING THEIR EXPONENTS
        if(!rplCompareIDENT(rplPeekData(level1),rplPeekData(level2))) return;   // NOTHING TO DO IF DIFFERENT IDENTS
        // COPY THE IDENTIFIER TO THE TOP OF STACK
        WORDPTR *stackptr=DSTop;

        rplPushData(rplPeekData(level1));

        // ADD THE EXPONENTS
        rplPushData(rplPeekData(level1));   // FIRST NUMERATOR
        rplPushData(rplPeekData(level1));   // FIRST DENOMINATOR
        rplPushData(rplPeekData(level2+2)); // SECOND NUMERATOR
        rplPushData(rplPeekData(level2+2)); // SECOND DENOMINATOR
        if(Exceptions) { DSTop=stackptr; return; }
        BINT sign=rplFractionAdd();
        if(Exceptions) { DSTop=stackptr; return; }

        rplFractionSimplify();
        if(Exceptions) { DSTop=stackptr; return; }

        if(sign) {
            // ADD THE SIGN TO THE NUMERATOR
            rplPushData(rplPeekData(2));
            rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NEG));
            if(Exceptions) { DSTop=stackptr; return; }

            rplOverwriteData(3,rplPeekData(1));
            rplDropData(1);
        }


        // NOW REMOVE THE ORIGINALS FROM THE STACK
        if(level1>level2) {
        rplUnitPopItem(level1+3);
        rplUnitPopItem(level2+3);
        }
        else {
            rplUnitPopItem(level2+3);
            rplUnitPopItem(level1+3);
        }
        return;
    }

    // NOT AN IDENTIFIER, USE THE OVERLOADED OPERATOR TO MULTIPLY
    WORDPTR *savestk=DSTop;
    rplUnitPickItem(level1);
    rplUnitPickItem(level2+1);
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_MUL));
    if(Exceptions) { DSTop=savestk; return; }

    // JUST REMOVE THE ORIGINALS
    if(level1>level2) {
        rplUnitPopItem(level1+1);
        rplUnitPopItem(level2+1);
    }
    else  {
        rplUnitPopItem(level2+1);
        rplUnitPopItem(level1+1);
    }
}

// SKIPS AN ITEM (VALUE OR IDENT) AND RETURNS THE LEVEL OF THE NEXT ITEM
BINT rplUnitSkipItem(BINT level)
{
    if(ISIDENT(*rplPeekData(level))) return level-3;
    return level-1;
}

// SIMPLIFY A UNIT IN THE STACK BY COLLAPSING REPEATED IDENTS
// AND PERFORMING FRACTION SIMPLIFICATION OF EXPONENTS
// RETURNS THE NUMBER OF LEVELS LEFT IN THE STACK
// AFTER SIMPLIFICATION
BINT rplUnitSimplify(BINT nlevels)
{
    BINT lvl=nlevels,lvl2;

    while(lvl>0) {
        lvl2=rplUnitSkipItem(lvl);

        while(lvl2>0) {
            if(ISIDENT(*rplPeekData(lvl2))) {
                if(rplCompareIDENT(rplPeekData(lvl),rplPeekData(lvl2))) {
                   // SAME IDENTIFIER, COLLAPSE THESE TWO


                }
            } else {
                // NOT AN IDENT, MULTIPLY TO THE CURRENT VALUE


            }
         lvl2=rplUnitSkipItem(lvl2);
        }
        lvl=rplUnitSkipItem(lvl);
    }



}

// INVERT A SINGLE UNIT IDENTIFIER BY NEGATING ITS EXPONENT
// RECEIVE THE LEVEL OF THE STACK WHERE THE IDENTIFIER IS
void rplUnitInvert(BINT level)
{

}

// TAKE ONE UNIT IDENTIFIER GIVEN AT LEVEL, REMOVE IT FROM THE STACK WITH ITS EXPONENTS
// AND APPEND THE UNIT DEFINITION TO THE STACK, WITH VALUE AND COEFFICIENTS
// MODIFIED BY THE ORIGINAL EXPONENT
// IF UNIT IS ALREADY A BASE UNIT, DROP IT TO THE TOP OF THE STACK
void rplUnitExpand(BINT level)
{

}

// RECURSIVELY EXPAND ALL UNITS USING THEIR DEFINITIONS UNTIL A BASE IS REACHED

BINT rplUnitToBase(BINT nlevels)
{

}

// SORT UNIT IDENTIFIERS IN ORDER FOR PROPER COMPARISON
// TAKES 2 UNITS EXPLODED IN THE STACK
// THE TOP nlevels OF THE STACK CONTAIN THE UNIT TO BE SORTED
// THE LEVELS FROM reflevel TO (nlevels+1) HAVE THE UNIT TO BE USED AS REFERENCE
// IDENTIFIERS IN THE UNIT WILL BE REORGANIZED TO MATCH THE ORDER OF THE
// REFERENCE UNIT. ANY EXTRA UNITS WILL BE AT THE TOP OF THE STACK IN NO
// PARTICULAR ORDER

BINT rplUnitSort(BINT nlevels, BINT reflevel)
{

}

// RETURN TRUE/FALSE IF THE UNIT IN THE FIRST nlevels ARE CONSISTENT
// WITH THE UNIT IN reflevel TO (nlevels+1)
// BOTH UNITS MUST BE EXPLODED AND REDUCED TO BASE IF NECESSARY
BINT rplUnitIsConsistent(BINT nlevels,BINT reflevel)
{

}
