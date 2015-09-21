/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include "newrpl.h"
#include "libraries.h"


// SYSTEM UNIT NAME TABLE: CONTAINS NAMES FOR ALL SYSTEM DEFINED UNITS
const WORD const system_unit_names[]={
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('m',0,0,0),        // [0]='m'
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('g',0,0,0),        // [2]='g'
    MKPROLOG(DOIDENTSIPREFIX,1),TEXT2WORD('s',0,0,0),        // [4]='s'
    MKPROLOG(DOIDENT,1),TEXT2WORD('i','n',0,0),        // [6]='in'

};

// SYSTEM UNIT DEFINITION TABLE: CONTAINS THE VALUES OF ALL SYSTEM DEFINED UNITS
const WORD const system_unit_defs[]={
    MKPROLOG(DOUNIT,7),
    MKPROLOG(DOREAL,2),MAKEREALFLAGS(-4,1,0),254,       // 0.0254
    MKPROLOG(DOIDENT,1),TEXT2WORD('m',0,0,0),MAKESINT(1),MAKESINT(1),

};

// SYSTEM UNIT DEFINITION DIRECTORY: CONTAINS PONTERS TO NAME/VALUE PAIRS FOR ALL SYSTEM UNITS
const WORDPTR const system_unit_dir[]={
    &system_unit_names[0],&one_bint,            // 'm'=1
    &system_unit_names[2],&one_bint,            // 'g'=1
    &system_unit_names[4],&one_bint,            // 's'=1
    &system_unit_names[6],&system_unit_defs[0],            // 'in'=0.0254_m
    0,0                                         // NULL TERMINATED LIST
};


























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
if(!ISUNIT(*unitobj)) {
    rplPushData(unitobj);
    return 1;
}

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

WORDPTR rplUnitAssemble(BINT nlevels)
{
    // A SINGLE VALUE OBJECT WHERE ALL UNITS CANCELLED OUT
    // NO NEED TO CREATE A NEW OBJECT
    if(nlevels==1) return rplPeekData(1);
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
// RETURNS THE NUMBER OF ELEMENTS REMOVED FROM THE STACK
BINT rplUnitPopItem(BINT level)
{
    BINT nitems;

    if(level>rplDepthData()) return 0;

    if(ISIDENT(*rplPeekData(level))) nitems=3;
    else nitems=1;

    if(level-nitems<=0) {
        rplDropData(level);
        return nitems;
    }

    memmovew(DSTop-level,DSTop-level+nitems,(level-nitems)*sizeof(WORDPTR*)/sizeof(WORD));
    DSTop-=nitems;
    return nitems;
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
// REMOVES THE SECOND IDENTIFIER FROM THE STACK AND OVERWRITES THE FIRST ELEMENT WITH THE RESULT
// LOW LEVEL, NO CHECKS OF ANY KIND DONE HERE
// RETURNS THE NUMBER OF LEVELS CHANGED IN THE STACK (NEGATIVE=REMOVED ELEMENTS)
BINT rplUnitMulItem(BINT level1,BINT level2)
{
    if(ISIDENT(*rplPeekData(level1))) {
        if(!ISIDENT(*rplPeekData(level2))) {
            // ONE IS VALUE AND ONE IS IDENT, NOTHING TO DO
            return 0;
        }
        // MULTIPLY 2 IDENTIFIERS BY ADDING THEIR EXPONENTS
        if(!rplCompareIDENT(rplPeekData(level1),rplPeekData(level2))) return 0;   // NOTHING TO DO IF DIFFERENT IDENTS
        // COPY THE IDENTIFIER TO THE TOP OF STACK
        WORDPTR *stackptr=DSTop;

        rplPushData(rplPeekData(level1));

        // ADD THE EXPONENTS
        rplPushData(rplPeekData(level1));   // FIRST NUMERATOR
        rplPushData(rplPeekData(level1));   // FIRST DENOMINATOR
        rplPushData(rplPeekData(level2+2)); // SECOND NUMERATOR
        rplPushData(rplPeekData(level2+2)); // SECOND DENOMINATOR
        if(Exceptions) { DSTop=stackptr; return 0; }
        BINT sign=rplFractionAdd();
        if(Exceptions) { DSTop=stackptr; return 0; }

        rplFractionSimplify();
        if(Exceptions) { DSTop=stackptr; return 0; }

        if(sign) {
            // ADD THE SIGN TO THE NUMERATOR
            rplPushData(rplPeekData(2));
            rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NEG));
            if(Exceptions) { DSTop=stackptr; return 0; }

            rplOverwriteData(3,rplPeekData(1));
            rplDropData(1);
        }

        // OVERWRITE level1 WITH THE NEW VALUES
        rplOverwriteData(level1+3,rplPeekData(3));
        rplOverwriteData(level1+2,rplPeekData(2));
        rplOverwriteData(level1+1,rplPeekData(1));

        rplDropData(3);

        // NOW REMOVE THE ORIGINALS FROM THE STACK
         rplUnitPopItem(level2);
        return -3;
    }

    if(ISIDENT(*rplPeekData(level2))) {
        // ONE IS VALUE AND ONE IS IDENT, NOTHING TO DO
        return 0;
    }

    // NOT AN IDENTIFIER, USE THE OVERLOADED OPERATOR TO MULTIPLY

    WORDPTR *savestk=DSTop;
    rplUnitPickItem(level1);
    rplUnitPickItem(level2+1);
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_MUL));
    if(Exceptions) { DSTop=savestk; return 0; }

    rplOverwriteData(level1+1,rplPeekData(1));
    rplDropData(1);

    // JUST REMOVE THE ORIGINALS
    rplUnitPopItem(level2);

    return -1;
}


// TAKES 2 VALUES OR 2 IDENTIFIERS AND DOES level2=level2**exponent(level1)
// IF level1 IS A VALUE, THEN IT IS USED AS EXPONENT TO ANY VALUES IN level2 AND
// TO MULTIPLY ANY EXPONENTS IN level2 IDENTIFIERS
// IF level1 IS AN IDENTIFIER, ITS EXPONENT IS USED AS EXPONENT TO ANY VALUES IN level2 AND
// TO MULTIPLY ANY EXPONENTS IN level2 IDENTIFIERS
// DOES NOT REMOVE ANYTHING FROM THE STACK, MODIFIES level2 ON THE SPOT
// LOW LEVEL, NO CHECKS OF ANY KIND DONE HERE
void rplUnitPowItem(BINT level1,BINT level2)
{
    if(ISIDENT(*rplPeekData(level2))) {
        // POW 2 IDENTIFIERS BY MULTIPLYING THEIR EXPONENTS
        WORDPTR *stackptr=DSTop;
        BINT isident=ISIDENT(*rplPeekData(level1));
        if(isident) rplPushData(rplPeekData(level1-1));   // FIRST NUMERATOR
        else rplPushData(rplPeekData(level1));
        rplPushData(rplPeekData(level2)); // SECOND NUMERATOR
        rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_MUL));
        if(Exceptions) { DSTop=stackptr; return; }
        if(isident) {
            rplPushData(rplPeekData(level1-1));   // FIRST DENOMINATOR
            rplPushData(rplPeekData(level2)); // SECOND DENOMINATOR
            rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_MUL));
            if(Exceptions) { DSTop=stackptr; return; }
        } else {
            rplPushData(rplPeekData(level2)); // SECOND DENOMINATOR
        }

        rplFractionSimplify();
        if(Exceptions) { DSTop=stackptr; return; }

        // NOW OVERWRITE THE ORIGINALS FROM THE STACK
        rplOverwriteData(level2+1,rplPeekData(2));
        rplOverwriteData(level2,rplPeekData(1));

        rplDropData(2);

       return;
    }

    // NOT AN IDENTIFIER, USE THE OVERLOADED OPERATOR TO DO THE POWER
    WORDPTR *savestk=DSTop;
    rplUnitPickItem(level2);
    if(ISIDENT(*rplPeekData(level1+1))) {
        // DIVIDE THE NUMERATOR AND DENOMINATOR TO GET AN EXPONENT
        rplPushData(rplPeekData(level1));
        if(*rplPeekData(level1)!=MAKESINT(1)) {
            rplPushData(rplPeekData(level1-1));
            rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_DIV));
            if(Exceptions) { DSTop=savestk; return; }
        }
    } else rplUnitPickItem(level1+1);

    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_POW));
    if(Exceptions) { DSTop=savestk; return; }

    // HERE WE SHOULD HAVE A SINGLE VALUE
    rplOverwriteData(level2+1,rplPeekData(1));
    rplDropData(1);

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
    BINT lvl=nlevels,lvl2,reduction;

    while(lvl>0) {
        lvl2=rplUnitSkipItem(lvl);

        while(lvl2>0) {

                    reduction=rplUnitMulItem(lvl,lvl2);
                    if(Exceptions) return nlevels;
                    lvl+=reduction; // POINT TO THE NEXT ITEM, SINCE THIS ONE VANISHED
                    lvl2+=reduction;
                    nlevels+=reduction;
                    if(!reduction) lvl2=rplUnitSkipItem(lvl2);
        }

        // CHECK IF EXPONENT ENDED UP BEING ZERO FOR THIS UNIT

        if(*rplPeekData(lvl-1)==MAKESINT(0)) {
         // NEED TO REMOVE THIS UNIT FROM THE LIST
            BINT oldlvl=lvl;
            lvl=rplUnitSkipItem(lvl);
            rplUnitPopItem(oldlvl);
            nlevels-=oldlvl-lvl;
        } else  lvl=rplUnitSkipItem(lvl);

    }

    // AT THIS POINT THERE SHOULD BE ONLY ONE VALUE AND MANY UNITS
    // NEEDS TO BE SORTED SO THAT THE VALUE IS FIRST AND UNITS ARE
    // AFTERWARDS

    lvl=nlevels;

    while(lvl>0) {
        if(!ISIDENT(*rplPeekData(lvl))) break;
        lvl=rplUnitSkipItem(lvl);
    }

    if(lvl<=0) {
        // ERROR! SOMETHING HAPPENED AND THERE'S NO UNIT VALUE!
        // TRY TO FIX IT BY ADDING A 1
        rplPushData(one_bint);
        ++nlevels;
        lvl=1;
    }

    // UNROLL THIS VALUE TO THE BOTTOM OF THE UNIT
    WORDPTR value=rplPeekData(lvl);

    while(lvl!=nlevels) { rplOverwriteData(lvl,rplPeekData(lvl-1)); --lvl; }
    rplOverwriteData(nlevels,value);

    return nlevels;

}

// INVERT A SINGLE UNIT IDENTIFIER BY NEGATING ITS EXPONENT
// RECEIVE THE LEVEL OF THE STACK WHERE THE IDENTIFIER IS
void rplUnitInvert(BINT level)
{
if(!ISIDENT(*rplPeekData(level))) return;

WORDPTR savestk=DSTop;

rplPushData(rplPeekData(level-1));
rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NEG));
if(Exceptions) { DSTop=savestk; return; }
rplOverwriteData(level-1,rplPopData());

}

// DIVIDE TWO UNITS THAT WERE EXPLODED IN THE STACK (1..divlvl) and (divlvl+1 .. numlvl)
// DIVISION IS DONE IN 3 OPERATIONS:
// A) DIVIDE THE VALUES USING OVERLOADED OPERATORS
// B) INVERT THE UNIT PART OF THE DIVISOR
// C) MULTIPLY/SIMPLIFY THE UNIT PORTION
// RETURN THE NUMBER OF ELEMENTS AFTER THE SIMPLIFICATION
BINT rplUnitDivide(BINT numlvl,BINT divlvl)
{
    WORDPTR savestk=DSTop;

    rplPushData(rplPeekData(numlvl));   // GET THE VALUE
    rplPushData(rplPeekData(divlvl+1)); // GET THE VALUE OF THE DIVISOR

    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_DIV));
    if(Exceptions) { DSTop=savestk; return 0; }

    // NOW REPLACE THE NUMERATOR VALUE WITH THE RESULT
    rplOverwriteData(numlvl,rplPopData());
    // AND REMOVE THE VALUE OF THE DIVISOR FROM THE STACK
    rplUnitPopItem(divlvl);

    numlvl--;
    divlvl--;

    // NOW NEGATE ALL UNIT IDENTIFIER'S EXPONENTS
    while(divlvl>0) {
        rplUnitInvert(divlvl);
        divlvl=rplUnitSkipItem(divlvl);
    }

    // UNIT IS READY TO BE SIMPLIFIED

    return rplUnitSimplify(numlvl);


}

#define NUM_SIPREFIXES 21

// THIS IS THE TEXT OF THE 20 SI PREFIXES
// INDEX 0 MEANS NO PREFIX
const char * const siprefix_text[]={
    "",
    "Y",
    "Z",
    "E",
    "P",
    "T",
    "G",
    "M",
    "k",
    "h",
    "da",
    "d",
    "c",
    "m",
    "µ",
    "n",
    "p",
    "f",
    "a",
    "z",
    "y"
};

// EXPONENT MODIFICATION DUE TO SI PREFIX

const BINT const siprefix_exp[]={
        0,
        24,
        21,
        18,
        15,
        12,
        9,
        6,
        3,
        2,
        1,
        -1,
        -2,
        -3,
        -6,
        -9,
        -12,
        -15,
        -18,
        -21,
        -24
        };


// RETURN THE INDEX TO A SI PREFIX



BINT rplUnitGetSIPrefix(WORDPTR ident)
{
BINT k,len,ilen;
BYTEPTR istart,iend;

// FIND START AND END OF THE IDENT TEXT
istart=(BYTEPTR) (ident+1);
iend=rplSkipOb(ident);
while( (iend>istart) && (*(iend-1)==0)) --iend;
ilen=utf8nlen(istart,iend);
if(ilen<2) return 0;

// HERE WE ARE READY FOR STRING COMPARISON

for(k=1;k<NUM_SIPREFIXES;++k)
{
// LEN IN UNICODE CHARATERS OF THE SI PREFIX
if(k==10) len=2; else len=1;
if(!utf8ncmp(istart,siprefix_text[k],len)) {
 // FOUND A VALID PREFIX
 if(ilen>len) return k;
}
}

return 0;
}

// COMPARES AN IDENT TO A BASE IDENT FOR EQUALITY
// THE BASE IDENTIFIER IS NOT SUPPOSED TO HAVE ANY SI PREFIXES
// RETURNS 0 = THEY ARE DIFFERENT
// -1 = THEY ARE IDENTICAL
// n = THEY DIFFER ONLY IN THE SI PREFIX, OTHERWISE IDENTICAL UNIT (n=SI PREFIX INDEX)

BINT rplUnitCompare(WORDPTR ident,WORDPTR baseident)
{
    BINT siidx;

    if(rplCompareIDENT(ident,baseident)) return -1;

    if(LIBNUM(*baseident)!=DOIDENTSIPREFIX) return 0;   // BASE UNIT DOESN'T SUPPORT SI PREFIXES

    siidx=rplUnitGetSIPrefix(ident);

    if(siidx==0) return 0;      // THERE WAS NO PREFIX, SO THEY CANNOT BE THE SAME UNIT

    // THERE'S AN SI PREFIX, NEED TO COMPARE TEXT BY SKIPPING IT
    BYTEPTR st1,end1,st2,end2;
    BINT len1,len2;
    st1=(BYTEPTR) (ident+1);
    st2=(BYTEPTR) (baseident+1);
    end1=rplSkipOb(ident);
    end2=rplSkipOb(baseident);

    // FIND THE END IN BOTH IDENTS
    while( (end1>st1) && (*(end1-1)==0)) --end1;
    while( (end2>st2) && (*(end2-1)==0)) --end2;

    if(siidx==10) st1=utf8nskip(st1,end1,2);
    else st1=utf8nskip(st1,end1,1);

    // NOW DO THE COMPARISON BYTE BY BYTE
    if( (end1-st1)!=(end2-st2)) return 0;

    while(st1!=end1) {
        if(*st1!=*st2) return 0;
        ++st1;
        ++st2;
    }

    return siidx;

}








// GET THE DEFINITION OF A UNIT FROM ITS IDENTIFIER
// FIRST IT STRIPS ANY SI PREFIXES AND SEARCHES FOR
// BOTH THE ORIGINAL UNIT AND THE STRIPPED ONE
// RETURNS A POINTER WITHIN THE DIRECTORY ENTRY
// THE FIRST POINTER POINTS TO THE IDENTIFIER
// THE SECOND TO ITS VALUE
// THE SEARCH IS DONE FIRST IN THE USER'S UNIT DIRECTORY
// THEN IN THE SYSTEM BASE UNITS DEFINED IN ROM
// IF THE siindex POINTER IS NOT NULL, IT STORES THE
// INDEX TO THE SI PREFIX THAT WAS FOUND IN THE GIVEN NAME

WORDPTR *rplUnitFind(WORDPTR ident,BINT *siindex)
{
    const BYTEPTR const unitdir_name[]={(BYTEPTR)"UNITS"};

    WORDPTR unitdir_obj=rplGetSettingsbyName(unitdir_name,unitdir_name+5);
    WORDPTR baseid,baseunit;
    BINT result;
    WORDPTR *entry;

    if(unitdir_obj) {
        // FOUND UNITS DIRECTORY IN SETTINGS, SCAN IT TO FIND OUT IDENT
        entry=rplFindFirstInDir(unitdir_obj);
        while(entry) {
            baseid=entry[0];
            baseunit=entry[1];

            result=rplUnitCompare(ident,baseid);
            if(result) {
                // WE FOUND A MATCH!
                if(result<0) result=0;
                if(siindex) *siindex=result;
                return entry;
            }

            entry=rplFindNext(entry);
        }

        // NOT FOUND IN THE USERS DIR, TRY THE SYSTEM LIST

    }

    // SEARCH THROUGH THE SYSTEM UNITS

    entry=system_unit_dir;
    while(entry[0]) {
        baseid=entry[0];
        baseunit=entry[1];

        result=rplUnitCompare(ident,baseid);
        if(result) {
            // WE FOUND A MATCH!
            if(result<0) result=0;
            if(siindex) *siindex=result;
            return entry;
        }

        entry+=2;
    }

    // UNIT IS NOT DEFINED
    return 0;

}




// TAKE ONE UNIT IDENTIFIER GIVEN AT LEVEL, REMOVE IT FROM THE STACK WITH ITS EXPONENTS
// AND APPEND THE UNIT DEFINITION TO THE STACK, WITH VALUE AND COEFFICIENTS
// MODIFIED BY THE ORIGINAL EXPONENT
// IF UNIT IS ALREADY A BASE UNIT, LEAVE AS-IS
// RETURNS NUMBER OF LEVELS ADDED TO THE STACK

BINT rplUnitExpand(BINT level)
{
    if(ISIDENT(*rplPeekData(level))) {

            BINT siidx;
            WORDPTR *entry=rplUnitFind(rplPeekData(level),&siidx);

            if(!entry) {
                // NOT FOUND, KEEP AS-IS
                return 0;
            }

            WORDPTR *stktop=DSTop;

            // UNIT WAS FOUND



            BINT nlevels=rplUnitExplode(entry[1]);
            if(Exceptions) { DSTop=stktop; return 0; }

            if(nlevels==1) {
                if(siidx) {
                // UNIT WAS A BASE UNIT WITH AN SI PREFIX
                // ADD THE BASE UNIT WITHOUT THE PREFIX
                rplPushData(entry[0]);
                rplPushData(one_bint);
                rplPushData(one_bint);
                nlevels+=3;
                }
                else {
                    // UNIT WAS ALREADY A BASE UNIT, DO NOTHING
                    DSTop=stktop;
                    return 0;
                }
            }


            if(siidx) {
                // THERE WAS AN SI PREFIX, NEED TO INCLUDE IT IN THE MULTIPLIER
                REAL value;
                rplReadNumberAsReal(rplPeekData(nlevels),&value);
                value.exp+=siprefix_exp[siidx];
                WORDPTR newval=rplNewReal(&value);
                if(!newval) { DSTop=stktop; return 0; }
                rplOverwriteData(nlevels,newval);
            }

             // NOW APPLY THE EXPONENT!

            BINT lvl2=nlevels;

            while(lvl2>0) {
                rplUnitPowItem(level+nlevels,lvl2);
                lvl2=rplUnitSkipItem(lvl2);
            }

            // NOW REMOVE IT FROM THE STACK

            nlevels-=rplUnitPopItem(level+nlevels);

            return nlevels;

            }
    return 0;
}

// RECURSIVELY EXPAND ALL UNITS USING THEIR DEFINITIONS UNTIL A BASE IS REACHED
// RETURN THE NEW TOTAL NUMBER OF ELEMENTS
BINT rplUnitToBase(BINT nlevels)
{
    BINT lvl=nlevels,lvl2,morelevels;

    while(lvl>0) {
        morelevels=rplUnitExpand(lvl);

        if(!morelevels) lvl=rplUnitSkipItem(lvl);
        else {
            lvl+=morelevels;
            nlevels+=morelevels;
        }
    }

    // HERE ALL UNITS WERE EXPANDED TO THEIR BASES
    return nlevels;

}

// RETURN TRUE/FALSE IF THE UNIT IN THE FIRST nlevels ARE CONSISTENT
// WITH THE UNIT IN reflevel TO (nlevels+1)
// BOTH UNITS MUST BE EXPLODED AND REDUCED TO BASE BEFOREHAND
BINT rplUnitIsConsistent(BINT nlevels,BINT reflevel)
{
if(reflevel<nlevels) {
    BINT tmp=reflevel;
    reflevel=nlevels;
    nlevels=tmp;
}


if(nlevels!=reflevel-nlevels) return 0; // UNITS MUST HAVE THE SAME NUMBER OF IDENTS TO BE CONSISTENT

BINT lvl=nlevels,lvl2=reflevel;

while(lvl>0) {
    if(!ISIDENT(*rplPeekData(lvl))) { lvl=rplUnitSkipItem(lvl); continue; }
    WORDPTR mainident=rplPeekData(lvl);
    lvl2=reflevel;
    while(lvl2>nlevels) {
        if(rplCompareIDENT(mainident,rplPeekData(lvl2))) {
            // FOUND, COMPARE THE EXPONENTS

            if(ISREAL(*rplPeekData(lvl-1))) {
                // DO A REAL COMPARISON
                REAL num1,num2;

                rplReadNumberAsReal(rplPeekData(lvl-1),&num1);
                rplReadNumberAsReal(rplPeekData(lvl2-1),&num2);
                if(!eqReal(&num1,&num2)) return 0;  // INCONSISTENT UNITS
            } else {
                if(!rplCompareObjects(rplPeekData(lvl-1),rplPeekData(lvl2-1))) return 0; // INCONSISTENT UNITS
            }

            if(ISREAL(*rplPeekData(lvl-2))) {
                // DO A REAL COMPARISON
                REAL num1,num2;

                rplReadNumberAsReal(rplPeekData(lvl-2),&num1);
                rplReadNumberAsReal(rplPeekData(lvl2-2),&num2);
                if(!eqReal(&num1,&num2)) return 0;  // INCONSISTENT UNITS
            } else {
                if(!rplCompareObjects(rplPeekData(lvl-2),rplPeekData(lvl2-2))) return 0; // INCONSISTENT UNITS
            }

            // HERE WE HAVE A MATCH

            break;


        }

        lvl2=rplUnitSkipItem(lvl2);


    }

    if(lvl2==nlevels) return 0; // NO MATCH = INCONSISTENT UNITS

    lvl=rplUnitSkipItem(lvl);
}

return 1;
}
