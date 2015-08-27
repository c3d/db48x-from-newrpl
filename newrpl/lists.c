/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include "libraries.h"
#include "newrpl.h"


#define ENDLIST 0       // NOTE: COORDINATE THIS WITH LIBRARY 26, FIRST OPCODE MUST BE ENDLIST

// EXPAND A COMPOSITE IN THE STACK AND STORES THE NUMBER OF ELEMENTS AT THE END
// USES 2 SCRATCH POINTERS

BINT rplExplodeList(WORDPTR composite)
{
    BINT count=0;
    ScratchPointer1=composite+1;
    ScratchPointer2=composite+OBJSIZE(*composite);  // POINT TO THE END MARKER
    while(ScratchPointer1<ScratchPointer2) {
        rplPushData(ScratchPointer1);
        ScratchPointer1=rplSkipOb(ScratchPointer1);
        ++count;
    }
    rplNewBINTPush(count,DECBINT);
    return count;
}

BINT rplListLength(WORDPTR composite)
{
    BINT count=0;
    WORDPTR ptr=composite+1;
    WORDPTR end=composite+OBJSIZE(*composite);  // POINT TO THE END MARKER
    while(ptr<end) {
        ptr=rplSkipOb(ptr);
        ++count;
    }
    return count;
}



// RETURN THE LENGTH OF A "FLAT" LIST, AS IF LISTS INSIDE THE LIST WERE EXPLODED
BINT rplListLengthFlat(WORDPTR composite)
{
    BINT count=0,depth=0;
    WORDPTR ptr=composite+1;
    WORDPTR end=composite+OBJSIZE(*composite);  // POINT TO THE END MARKER
    while(ptr<end) {
        if(ISLIST(*ptr)) {
            ++depth;
            ++ptr;
        } else {
        ptr=rplSkipOb(ptr);
        if(*ptr==MKOPCODE(DOLIST,ENDLIST)) {
            ++ptr;
            --depth;
        }
        ++count;
        }
    }
    return count;

}


// GET AN ELEMENT FROM A "FLAT" VIEW OF THE LIST

WORDPTR rplGetListElementFlat(WORDPTR composite, BINT pos)
{
    BINT count=1;
    WORDPTR ptr=composite+1;
    WORDPTR end=composite+OBJSIZE(*composite);  // POINT TO THE END MARKER
    while(ptr<end && count<=pos) {
        if(ISLIST(*ptr)) {
            ++ptr;
        } else {
        if(count==pos) break;
        ptr=rplSkipOb(ptr);
        if(*ptr==MKOPCODE(DOLIST,ENDLIST)) {
            ++ptr;
        }
        ++count;
        }
    }
    if(ptr==end) return 0;
    return ptr;
}

WORDPTR rplGetListElement(WORDPTR composite, BINT pos)
{
    BINT count=1;
    WORDPTR ptr=composite+1;
    WORDPTR end=composite+OBJSIZE(*composite);  // POINT TO THE END MARKER
    while(ptr<end && count<pos) {
        ptr=rplSkipOb(ptr);
        ++count;
    }
    if(ptr==end) return 0;
    return ptr;
}

// RETURNS FALSE (0) IF THE ELEMENT IS NOT AT THE END OF A SUBLIST IN A "FLAT" LIST
// OTHERWISE RETURNS THE POSITION OF THE FIRST ELEMENT IN THE LIST THAT CONTAINS THE OBJECT

BINT rplIsLastElementFlat(WORDPTR composite, BINT pos)
{
    BINT count=1,depth=0,startpos=1;
    WORDPTR ptr=composite+1;
    WORDPTR end=composite+OBJSIZE(*composite);  // POINT TO THE END MARKER
    while(ptr<end && count<pos) {
        if(ISLIST(*ptr)) {
            ++depth;
            ++ptr;
            startpos=pos;
        } else {
        ptr=rplSkipOb(ptr);
        if(*ptr==MKOPCODE(DOLIST,ENDLIST)) {
            --depth;
            ++ptr;
        }
        ++count;
        }
    }

    if(ptr<end) {
    ptr=rplSkipOb(ptr);
    if(*ptr==MKOPCODE(DOLIST,ENDLIST)) return startpos;
    }
    return 0;
}



// CREATE A NEW LIST. STACK LEVEL 1 = NUMBER OF ELEMENTS, LEVELS 2.. N+1 = OBJECTS
// USES 1 SCRATCH POINTER
void rplCreateList()
{
    // NO ARGUMENT CHECKING
    BINT64 num=rplReadNumberAsBINT(rplPeekData(1));
    if(rplDepthData()<num+1) {
        Exceptions|=EX_BADARGCOUNT;
        ExceptionPointer=IPtr;
        return;
    }
    BINT size=1;    // 1 WORD FOR THE END MARKER
    BINT count;
    for(count=0;count<num;++count) {
        size+=rplObjSize(rplPeekData(2+count));
    }

    // ALLOCATE MEMORY
    WORDPTR list=rplAllocTempOb(size);
    if(!list) {
        return;
    }

    // CONSTRUCT THE OBJECT
    WORDPTR objptr=list+1;
    *list=MKPROLOG(DOLIST,size);
    for(count=num;count>0;--count) {
        rplCopyObject(objptr,rplPeekData(count+1));
        objptr+=rplObjSize(objptr);
    }
    *objptr=MKOPCODE(DOLIST,ENDLIST);

    rplDropData(num);
    rplOverwriteData(1,list);
}

