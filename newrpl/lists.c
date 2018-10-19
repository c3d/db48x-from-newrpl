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


// EXPAND A COMPOSITE IN THE STACK
// DOESN'T PUSH THE NUMBER OF ELEMENTS, JUST RETURNS IT
// USES 2 SCRATCH POINTERS

BINT rplExplodeList2(WORDPTR composite)
{
    BINT count=0;
    ScratchPointer1=composite+1;
    ScratchPointer2=composite+OBJSIZE(*composite);  // POINT TO THE END MARKER
    while(ScratchPointer1<ScratchPointer2) {
        rplPushData(ScratchPointer1);
        ScratchPointer1=rplSkipOb(ScratchPointer1);
        ++count;
    }
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
    if(pos<1) return 0;
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


// GET NEXT ELEMENT FROM A "FLAT" VIEW OF THE LIST

WORDPTR rplGetNextListElementFlat(WORDPTR composite, WORDPTR elem)
{
    WORDPTR ptr=elem;
    WORDPTR end=composite+OBJSIZE(*composite);  // POINT TO THE END MARKER
    while(ptr<end) {
        ptr=rplSkipOb(ptr);
        if(ISLIST(*ptr)) ++ptr;
        if(*ptr!=MKOPCODE(DOLIST,ENDLIST)) break;
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

void rplCreateList()
{
    // NO ARGUMENT CHECKING
    BINT64 num=rplReadNumberAsBINT(rplPeekData(1));
    if(rplDepthData()<num+1) {
        rplError(ERR_BADARGCOUNT);
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


// CREATE A NEW LIST. STACK LEVEL A.. N+A = OBJECTS
WORDPTR rplCreateListN(BINT num,BINT level,BINT remove)
{
    // NO ARGUMENT CHECKING
    BINT size=1;    // 1 WORD FOR THE END MARKER
    BINT count;
    for(count=level;count<level+num;++count) {
        size+=rplObjSize(rplPeekData(count));
    }

    // ALLOCATE MEMORY
    WORDPTR list=rplAllocTempOb(size);
    if(!list) {
        return 0;
    }

    // CONSTRUCT THE OBJECT
    WORDPTR objptr=list+1;
    *list=MKPROLOG(DOLIST,size);
    for(count=num;count>0;--count) {
        rplCopyObject(objptr,rplPeekData(level+count-1));
        objptr+=rplObjSize(objptr);
    }
    *objptr=MKOPCODE(DOLIST,ENDLIST);

    if(remove) rplRemoveAtData(level,num);

    return list;
}

// SET THE AUTO EXPAND BIT ON A LIST
void rplListAutoExpand(WORDPTR list)
{
    if(!ISLIST(*list)) return;
    *list|=MKPROLOG(1,0);
}


// List handling for funtions with 2 argument
void rplListBinaryDoCmd()
{
    if(ISLIST(*rplPeekData(2)) && ISLIST(*rplPeekData(1))) {

        WORDPTR *savestk=DSTop;
        WORDPTR newobj=rplAllocTempOb(2);
        if(!newobj) return;
        // CREATE A PROGRAM AND RUN THE DOLIST COMMAND
        newobj[0]=MKPROLOG(DOCOL,2);
        newobj[1]=CurOpcode;
        newobj[2]=CMD_SEMI;

        rplPushDataNoGrow((WORDPTR)two_bint);
        rplPushData(newobj);

        rplSetSystemFlag(FL_LISTCMDCLEANUP);

        rplCallOperator(CMD_CMDDOLIST);

        if(Exceptions) {
            if(DSTop>savestk) DSTop=savestk;
        }

        // EXECUTION WILL CONTINUE AT DOLIST

        return;
    }
    else if(ISLIST(*rplPeekData(2)) && !ISLIST(*rplPeekData(1))){

        BINT size1=rplObjSize(rplPeekData(1));
        WORDPTR *savestk=DSTop;

        WORDPTR newobj=rplAllocTempOb(2+size1);
        if(!newobj) return;

        // CREATE A PROGRAM AND RUN THE MAP COMMAND
        newobj[0]=MKPROLOG(DOCOL,2+size1);
        rplCopyObject(newobj+1,rplPeekData(1));
        newobj[size1+1]=CurOpcode;
        newobj[size1+2]=CMD_SEMI;

        rplDropData(1);
        rplPushData(newobj);

        rplSetSystemFlag(FL_LISTCMDCLEANUP);

        rplCallOperator(CMD_MAP);

        if(Exceptions) {
            if(DSTop>savestk) DSTop=savestk;
        }

        // EXECUTION WILL CONTINUE AT MAP

        return;

    }
    else if(!ISLIST(*rplPeekData(2)) && ISLIST(*rplPeekData(1))){

        BINT size1=rplObjSize(rplPeekData(2));
        WORDPTR *savestk=DSTop;

        WORDPTR newobj=rplAllocTempOb(3+size1);
        if(!newobj) return;

        // CREATE A PROGRAM AND RUN THE MAP COMMAND
        newobj[0]=MKPROLOG(DOCOL,3+size1);
        rplCopyObject(newobj+1,rplPeekData(2));
        newobj[size1+1]=CMD_SWAP;
        newobj[size1+2]=CurOpcode;
        newobj[size1+3]=CMD_SEMI;

        rplOverwriteData(2,rplPeekData(1));

        rplDropData(1);
        rplPushData(newobj);

        rplSetSystemFlag(FL_LISTCMDCLEANUP);

        rplCallOperator(CMD_MAP);

        if(Exceptions) {
            if(DSTop>savestk) DSTop=savestk;
        }

        // EXECUTION WILL CONTINUE AT MAP

        return;
    }
}

// List handling for funtions with 1 argument
void rplListUnaryDoCmd()
{

    WORDPTR *savestk=DSTop;
    WORDPTR newobj=rplAllocTempOb(2);
    if(!newobj) return;
    // CREATE A PROGRAM AND RUN THE MAP COMMAND
    newobj[0]=MKPROLOG(DOCOL,2);
    newobj[1]=CurOpcode;
    newobj[2]=CMD_SEMI;

    rplPushData(newobj);
    rplSetSystemFlag(FL_LISTCMDCLEANUP);

    rplCallOperator(CMD_MAP);

    if(Exceptions) {
        if(DSTop>savestk) DSTop=savestk;
    }

    // EXECUTION WILL CONTINUE AT MAP

    return;
}

// List handling for funtions with 1 argument
void rplListUnaryNoResultDoCmd()
{

    WORDPTR *savestk=DSTop;
    WORDPTR newobj=rplAllocTempOb(2);
    if(!newobj) return;
    // CREATE A PROGRAM AND RUN THE MAP COMMAND
    newobj[0]=MKPROLOG(DOCOL,2);
    newobj[1]=CurOpcode;
    newobj[2]=CMD_SEMI;

    rplPushData(newobj);
    rplSetSystemFlag(FL_LISTCMDCLEANUP);

    rplCallOperator(CMD_MAPINNERCOMP);

    if(Exceptions) {
        if(DSTop>savestk) DSTop=savestk;
    }

    // EXECUTION WILL CONTINUE AT MAP

    return;
}


// APPEND AN ITEM TO THE END OF THE LIST, DROP THE FIRST
// ELEMENT IF NEEDED TO KEEP THE LIST AT N ELEMENTS MAX.
// USES ScratchPointers 1 THRU 3
WORDPTR rplListAddRot(WORDPTR list,WORDPTR object,BINT nmax)
{
    BINT nitems;
    WORDPTR *savestk=DSTop;

    ScratchPointer3=object;
    nitems=rplExplodeList2(list);
    if(Exceptions) { DSTop=savestk; return 0; }

    int k,offset=nitems-(nmax-1);

    if(offset>0) {
        for(k=nmax-1;k>0;--k) rplOverwriteData(k+offset,rplPeekData(k));
        rplDropData(offset);
    }
    rplPushData(ScratchPointer3);
    WORDPTR newlist=rplCreateListN( ((offset>0)? nmax : (nitems+1)) ,1,1);
    if(Exceptions) { DSTop=savestk; return 0; }
    return newlist;
}

// CREATE A NEW LIST REPLACING THE OBJECT AT position WITH THE GIVEN object
// RETURNS POINTER TO NEW LIST, CAN TRIGGER GC.
// USES SCRATCHPOINTERS 1 AND 2
WORDPTR rplListReplace(WORDPTR list,BINT position,WORDPTR object)
{
BINT newobjsize=rplObjSize(object);
WORDPTR oldobject=rplGetListElement(list,position);
if(!oldobject) return 0; // INVALID INDEX?
BINT oldobjsize=rplObjSize(oldobject);
BINT oldobjoffset=oldobject-list;
ScratchPointer1=list;
ScratchPointer2=object;
WORDPTR newlist=rplAllocTempOb(OBJSIZE(*list)+newobjsize-oldobjsize);
if(!newlist) return 0;
*newlist=MKPROLOG(DOLIST,OBJSIZE(*list)+newobjsize-oldobjsize);
memmovew(newlist+1,ScratchPointer1+1,oldobjoffset-1);
memmovew(newlist+oldobjoffset,ScratchPointer2,newobjsize);
memmovew(newlist+oldobjoffset+newobjsize,ScratchPointer1+oldobjoffset+oldobjsize,OBJSIZE(*ScratchPointer1)-(oldobjoffset+oldobjsize-1));
return newlist;
}

// CREATE A NEW LIST REPLACING MULTIPLE OBJECTS AT position WITH THE GIVEN object (IF A LIST, IT'S EXPLODED ON REPLACEMENT)
// RETURNS POINTER TO NEW LIST, CAN TRIGGER GC.
// USES SCRATCHPOINTERS 1 AND 2
WORDPTR rplListReplaceMulti(WORDPTR list,BINT position,WORDPTR object)
{
BINT numnewobj,llen,endpos;
BINT newobjsize,oldobjsize,oldobjoffset;
WORDPTR oldobject;

newobjsize=rplObjSize(object);

if(ISLIST(*object)) { numnewobj=rplListLength(object); newobjsize-=2; ++object; }
else numnewobj=1;

endpos=position+numnewobj;
llen=rplListLength(list);
if(endpos>llen) endpos=llen+1;


oldobject=rplGetListElement(list,position);
if(!oldobject) return 0; // INVALID INDEX?
oldobjsize=(endpos==llen+1)? (OBJSIZE(*list)-(oldobject-list)) : (rplGetListElement(list,endpos)-oldobject);
oldobjoffset=oldobject-list;
ScratchPointer1=list;
ScratchPointer2=object;
WORDPTR newlist=rplAllocTempOb(OBJSIZE(*list)+newobjsize-oldobjsize);
if(!newlist) return 0;
*newlist=MKPROLOG(DOLIST,OBJSIZE(*list)+newobjsize-oldobjsize);
memmovew(newlist+1,ScratchPointer1+1,oldobjoffset-1);
memmovew(newlist+oldobjoffset,ScratchPointer2,newobjsize);
memmovew(newlist+oldobjoffset+newobjsize,ScratchPointer1+oldobjoffset+oldobjsize,OBJSIZE(*ScratchPointer1)-(oldobjoffset+oldobjsize-1));
return newlist;
}


// List handling for funtions with many arguments, only one of them can be a list

void rplListMultiArgDoCmd(BINT nargs)
{
    BINT k;
    for(k=nargs;k>=1;--k) {

    if(ISLIST(*rplPeekData(k))) {

            BINT j;
            BINT size1=0;

            // GET THE SIZE OF ALL OTHER ARGUMENTS
            for(j=1;j<=nargs;++j) {
            if(j!=k) size1+=rplObjSize(rplPeekData(j));
            }

        WORDPTR *savestk=DSTop;

        WORDPTR newobj=rplAllocTempOb(7+6+size1),newcmd;

        if(!newobj) return;

        // CREATE A PROGRAM AND RUN THE MAP COMMAND
        newobj[0]=MKPROLOG(DOCOL,6+size1);

            // COPY ALL OTHER ARGUMENTS
            BINT offset=1;
            for(j=nargs;j>=1;--j) {
            if(j!=k) {
            rplCopyObject(newobj+offset,rplPeekData(j));
            offset+=rplObjSize(rplPeekData(j));
            }
            }
            // NOW MAKE SURE THE ARGUMENTS ARE IN THE PROPER ORDER
            if(k!=nargs) {
            newobj[offset]=MAKESINT(nargs);
            ++offset;
            newobj[offset]=CMD_ROLL;
            ++offset;
            if(k!=1) {
            newobj[offset]=MAKESINT(k);
            ++offset;
            newobj[offset]=CMD_ROLLD;
            ++offset;
            }
            }
            // AND DO THE OPERATION
            newobj[offset]=CurOpcode;
            ++offset;
            newobj[offset]=CMD_SEMI;
            ++offset;

            newcmd=newobj+offset;
            // ANOTHER SECONDARY THAT DOES MAP THEN CLEANS UP THE STACK
            newobj[offset]=MKPROLOG(SECO,6);
            ++offset;
            newobj[offset]=CMD_MAP;
            ++offset;
            newobj[offset]=MAKESINT(nargs+1);
            ++offset;
            newobj[offset]=CMD_ROLLD;
            ++offset;
            newobj[offset]=MAKESINT(nargs);
            ++offset;
            newobj[offset]=CMD_DROPN;
            ++offset;
            newobj[offset]=CMD_SEMI;
            ++offset;

        rplPushDataNoGrow(rplPeekData(k));
        rplPushDataNoGrow(newobj);
        rplPushData(newcmd);

        rplSetSystemFlag(FL_LISTCMDCLEANUP);

        rplCallOvrOperator(CMD_OVR_XEQ);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
                return;
            }

        // EXECUTION WILL CONTINUE AT MAP

        return;

    }

    }

}


// ATOMIC SAME OPERATION FOR LISTS
// INPUT ARGUMENTS ARE 2 OBJECTS (LISTS) ON THE STACK
// RETURNS 1 IF THEY ARE THE SAME

BINT rplListSame()
{
WORDPTR *a=DSTop-2;
WORDPTR *b=DSTop-1;
BINT aend,bend;
BINT aptr,bptr;

aend=rplObjSize(*a);
bend=rplObjSize(*b);

aptr=bptr=0;

while((aptr<aend)&&(bptr<bend))
{
    if(ISLIST((*a)[aptr])) ++aptr;
    if(ISLIST((*b)[bptr])) ++bptr;

    rplPushDataNoGrow((*a)+aptr);
    rplPushDataNoGrow((*b)+bptr);
    rplCallOvrOperator(CMD_OVR_SAME);
    if(Exceptions) return 0;

    if(rplIsFalse(rplPopData())) return 0;
    aptr+=rplObjSize((*a)+aptr);
    bptr+=rplObjSize((*b)+bptr);
}
if((aptr!=aend)||(bptr!=bend)) return 0;
return 1;

}
