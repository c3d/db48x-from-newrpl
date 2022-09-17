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
#include "sysvars.h"

#define ENDLIST 0       // NOTE: COORDINATE THIS WITH LIBRARY 26, FIRST OPCODE MUST BE ENDLIST

// EXPAND A COMPOSITE IN THE STACK AND STORES THE NUMBER OF ELEMENTS AT THE END
// USES 2 SCRATCH POINTERS

int32_t rplExplodeList(word_p composite)
{
    int32_t count = 0;
    ScratchPointer1 = composite + 1;
    ScratchPointer2 = composite + OBJSIZE(*composite);  // POINT TO THE END MARKER
    while(ScratchPointer1 < ScratchPointer2) {
        rplPushData(ScratchPointer1);
        ScratchPointer1 = rplSkipOb(ScratchPointer1);
        ++count;
    }
    rplNewint32_tPush(count, DECint32_t);
    return count;
}

// EXPAND A COMPOSITE IN THE STACK
// DOESN'T PUSH THE NUMBER OF ELEMENTS, JUST RETURNS IT
// USES 2 SCRATCH POINTERS

int32_t rplExplodeList2(word_p composite)
{
    int32_t count = 0;
    ScratchPointer1 = composite + 1;
    ScratchPointer2 = composite + OBJSIZE(*composite);  // POINT TO THE END MARKER
    while(ScratchPointer1 < ScratchPointer2) {
        rplPushData(ScratchPointer1);
        ScratchPointer1 = rplSkipOb(ScratchPointer1);
        ++count;
    }
    return count;
}

int32_t rplListLength(word_p composite)
{
    int32_t count = 0;
    word_p ptr = composite + 1;
    word_p end = composite + OBJSIZE(*composite);      // POINT TO THE END MARKER
    while(ptr < end) {
        ptr = rplSkipOb(ptr);
        ++count;
    }
    return count;
}

// RETURN THE LENGTH OF A "FLAT" LIST, AS IF LISTS INSIDE THE LIST WERE EXPLODED
int32_t rplListLengthFlat(word_p composite)
{
    int32_t count = 0, depth = 0;
    word_p ptr = composite + 1;
    word_p end = composite + OBJSIZE(*composite);      // POINT TO THE END MARKER
    while(ptr < end) {
        if(ISLIST(*ptr)) {
            ++depth;
            ++ptr;
        }
        else {
            ptr = rplSkipOb(ptr);
            if(*ptr == MKOPCODE(DOLIST, ENDLIST)) {
                ++ptr;
                --depth;
            }
            ++count;
        }
    }
    return count;

}

// GET AN ELEMENT FROM A "FLAT" VIEW OF THE LIST

word_p rplGetListElementFlat(word_p composite, int32_t pos)
{
    int32_t count = 1;
    word_p ptr = composite + 1;
    word_p end = composite + OBJSIZE(*composite);      // POINT TO THE END MARKER
    while(ptr < end && count <= pos) {
        if(ISLIST(*ptr)) {
            ++ptr;
        }
        else {
            if(count == pos)
                break;
            ptr = rplSkipOb(ptr);
            if(*ptr == MKOPCODE(DOLIST, ENDLIST)) {
                ++ptr;
            }
            ++count;
        }
    }
    if(ptr == end)
        return 0;
    return ptr;
}

word_p rplGetListElement(word_p composite, int32_t pos)
{
    if(pos < 1)
        return 0;
    int32_t count = 1;
    word_p ptr = composite + 1;
    word_p end = composite + OBJSIZE(*composite);      // POINT TO THE END MARKER
    while(ptr < end && count < pos) {
        ptr = rplSkipOb(ptr);
        ++count;
    }
    if(ptr == end)
        return 0;
    return ptr;
}

// GET NEXT ELEMENT FROM A "FLAT" VIEW OF THE LIST

word_p rplGetNextListElementFlat(word_p composite, word_p elem)
{
    word_p ptr = elem;
    word_p end = composite + OBJSIZE(*composite);      // POINT TO THE END MARKER
    while(ptr < end) {
        ptr = rplSkipOb(ptr);
        if(ISLIST(*ptr))
            ++ptr;
        if(*ptr != MKOPCODE(DOLIST, ENDLIST))
            break;
    }
    if(ptr == end)
        return 0;
    return ptr;
}

// RETURNS FALSE (0) IF THE ELEMENT IS NOT AT THE END OF A SUBLIST IN A "FLAT" LIST
// OTHERWISE RETURNS THE POSITION OF THE FIRST ELEMENT IN THE LIST THAT CONTAINS THE OBJECT

int32_t rplIsLastElementFlat(word_p composite, int32_t pos)
{
    int32_t count = 1, depth = 0, startpos = 1;
    word_p ptr = composite + 1;
    word_p end = composite + OBJSIZE(*composite);      // POINT TO THE END MARKER
    while(ptr < end && count < pos) {
        if(ISLIST(*ptr)) {
            ++depth;
            ++ptr;
            startpos = pos;
        }
        else {
            ptr = rplSkipOb(ptr);
            if(*ptr == MKOPCODE(DOLIST, ENDLIST)) {
                --depth;
                ++ptr;
            }
            ++count;
        }
    }

    if(ptr < end) {
        ptr = rplSkipOb(ptr);
        if(*ptr == MKOPCODE(DOLIST, ENDLIST))
            return startpos;
    }
    return 0;
}

// CREATE A NEW LIST. STACK LEVEL 1 = NUMBER OF ELEMENTS, LEVELS 2.. N+1 = OBJECTS

void rplCreateList()
{
    // NO ARGUMENT CHECKING
    int64_t num = rplReadNumberAsInt64(rplPeekData(1));
    if(rplDepthData() < num + 1) {
        rplError(ERR_BADARGCOUNT);
        return;
    }
    int32_t size = 1;      // 1 WORD FOR THE END MARKER
    int32_t count;
    for(count = 0; count < num; ++count) {
        size += rplObjSize(rplPeekData(2 + count));
    }

    // ALLOCATE MEMORY
    word_p list = rplAllocTempOb(size);
    if(!list) {
        return;
    }

    // CONSTRUCT THE OBJECT
    word_p objptr = list + 1;
    *list = MKPROLOG(DOLIST, size);
    for(count = num; count > 0; --count) {
        rplCopyObject(objptr, rplPeekData(count + 1));
        objptr += rplObjSize(objptr);
    }
    *objptr = MKOPCODE(DOLIST, ENDLIST);

    rplDropData(num);
    rplOverwriteData(1, list);
}

// CREATE A NEW LIST. STACK LEVEL A.. N+A = OBJECTS
word_p rplCreateListN(int32_t num, int32_t level, int32_t remove)
{
    // NO ARGUMENT CHECKING
    int32_t size = 1;      // 1 WORD FOR THE END MARKER
    int32_t count;
    for(count = level; count < level + num; ++count) {
        size += rplObjSize(rplPeekData(count));
    }

    // ALLOCATE MEMORY
    word_p list = rplAllocTempOb(size);
    if(!list) {
        return 0;
    }

    // CONSTRUCT THE OBJECT
    word_p objptr = list + 1;
    *list = MKPROLOG(DOLIST, size);
    for(count = num; count > 0; --count) {
        rplCopyObject(objptr, rplPeekData(level + count - 1));
        objptr += rplObjSize(objptr);
    }
    *objptr = MKOPCODE(DOLIST, ENDLIST);

    if(remove)
        rplRemoveAtData(level, num);

    return list;
}

// SET THE AUTO EXPAND BIT ON A LIST
void rplListAutoExpand(word_p list)
{
    if(!ISLIST(*list))
        return;
    *list |= MKPROLOG(1, 0);
}

// List handling for funtions with 2 argument
void rplListBinaryDoCmd()
{
    if(ISLIST(*rplPeekData(2)) && ISLIST(*rplPeekData(1))) {

        if(rplListLength(rplPeekData(1)) != rplListLength(rplPeekData(2))) {
            rplError(ERR_INVALIDLISTSIZE);
            return;
        }
        word_p *savestk = DSTop;
        word_p newobj = rplAllocTempOb(2);
        if(!newobj)
            return;
        // CREATE A PROGRAM AND RUN THE DOLIST COMMAND
        newobj[0] = MKPROLOG(DOCOL, 2);
        newobj[1] = CurOpcode;
        newobj[2] = CMD_SEMI;

        rplPushDataNoGrow((word_p) two_bint);
        rplPushData(newobj);

        rplSetSystemFlag(FL_LISTCMDCLEANUP);

        rplCallOperator(CMD_CMDDOLIST);

        if(Exceptions) {
            if(DSTop > savestk)
                DSTop = savestk;
        }

        // EXECUTION WILL CONTINUE AT DOLIST

        return;
    }
    else if(ISLIST(*rplPeekData(2)) && !ISLIST(*rplPeekData(1))) {

        int32_t size1 = rplObjSize(rplPeekData(1));
        word_p *savestk = DSTop;

        word_p newobj = rplAllocTempOb(2 + size1);
        if(!newobj)
            return;

        // CREATE A PROGRAM AND RUN THE MAP COMMAND
        newobj[0] = MKPROLOG(DOCOL, 2 + size1);
        rplCopyObject(newobj + 1, rplPeekData(1));
        newobj[size1 + 1] = CurOpcode;
        newobj[size1 + 2] = CMD_SEMI;

        rplDropData(1);
        rplPushData(newobj);

        rplSetSystemFlag(FL_LISTCMDCLEANUP);

        rplCallOperator(CMD_MAP);

        if(Exceptions) {
            if(DSTop > savestk)
                DSTop = savestk;
        }

        // EXECUTION WILL CONTINUE AT MAP

        return;

    }
    else if(!ISLIST(*rplPeekData(2)) && ISLIST(*rplPeekData(1))) {

        int32_t size1 = rplObjSize(rplPeekData(2));
        word_p *savestk = DSTop;

        word_p newobj = rplAllocTempOb(3 + size1);
        if(!newobj)
            return;

        // CREATE A PROGRAM AND RUN THE MAP COMMAND
        newobj[0] = MKPROLOG(DOCOL, 3 + size1);
        rplCopyObject(newobj + 1, rplPeekData(2));
        newobj[size1 + 1] = CMD_SWAP;
        newobj[size1 + 2] = CurOpcode;
        newobj[size1 + 3] = CMD_SEMI;

        rplOverwriteData(2, rplPeekData(1));

        rplDropData(1);
        rplPushData(newobj);

        rplSetSystemFlag(FL_LISTCMDCLEANUP);

        rplCallOperator(CMD_MAP);

        if(Exceptions) {
            if(DSTop > savestk)
                DSTop = savestk;
        }

        // EXECUTION WILL CONTINUE AT MAP

        return;
    }
}

// List handling for funtions with 2 argument - Drops the resulting list off the stack
void rplListBinaryNoResultDoCmd()
{
    if(ISLIST(*rplPeekData(2)) && ISLIST(*rplPeekData(1))) {

        if(rplListLength(rplPeekData(1)) != rplListLength(rplPeekData(2))) {
            rplError(ERR_INVALIDLISTSIZE);
            return;
        }
        word_p newobj = rplAllocTempOb(5);
        if(!newobj)
            return;
        // CREATE A PROGRAM AND RUN THE DOLIST COMMAND
        newobj[0] = MKPROLOG(DOCOL, 5);
        newobj[1] = CurOpcode;
        newobj[2] = CMD_SEMI;
        newobj[3] = CMD_CMDDOLIST;
        newobj[4] = CMD_DROP;
        newobj[5] = CMD_SEMI;

        rplPushDataNoGrow((word_p) two_bint);
        rplPushDataNoGrow(newobj);

        rplSetSystemFlag(FL_LISTCMDCLEANUP);

        rplPushRet(IPtr);
        IPtr = newobj + 2;
        CurOpcode = CMD_CMDDOLIST;

        // EXECUTION WILL CONTINUE AT DOLIST

        return;
    }
    else if(ISLIST(*rplPeekData(2)) && !ISLIST(*rplPeekData(1))) {

        int32_t size1 = rplObjSize(rplPeekData(1));
        word_p *savestk = DSTop;

        word_p newobj = rplAllocTempOb(2 + size1);
        if(!newobj)
            return;

        // CREATE A PROGRAM AND RUN THE MAP COMMAND
        newobj[0] = MKPROLOG(DOCOL, 2 + size1);
        rplCopyObject(newobj + 1, rplPeekData(1));
        newobj[size1 + 1] = CurOpcode;
        newobj[size1 + 2] = CMD_SEMI;

        rplDropData(1);
        rplPushData(newobj);

        rplSetSystemFlag(FL_LISTCMDCLEANUP);

        rplCallOperator(CMD_MAPINNERCOMP);

        if(Exceptions) {
            if(DSTop > savestk)
                DSTop = savestk;
        }

        // EXECUTION WILL CONTINUE AT MAP

        return;

    }
    else if(!ISLIST(*rplPeekData(2)) && ISLIST(*rplPeekData(1))) {

        int32_t size1 = rplObjSize(rplPeekData(2));
        word_p *savestk = DSTop;

        word_p newobj = rplAllocTempOb(3 + size1);
        if(!newobj)
            return;

        // CREATE A PROGRAM AND RUN THE MAP COMMAND
        newobj[0] = MKPROLOG(DOCOL, 3 + size1);
        rplCopyObject(newobj + 1, rplPeekData(2));
        newobj[size1 + 1] = CMD_SWAP;
        newobj[size1 + 2] = CurOpcode;
        newobj[size1 + 3] = CMD_SEMI;

        rplOverwriteData(2, rplPeekData(1));

        rplDropData(1);
        rplPushData(newobj);

        rplSetSystemFlag(FL_LISTCMDCLEANUP);

        rplCallOperator(CMD_MAPINNERCOMP);

        if(Exceptions) {
            if(DSTop > savestk)
                DSTop = savestk;
        }

        // EXECUTION WILL CONTINUE AT MAP

        return;
    }
}

// List handling for funtions with 1 argument
void rplListUnaryDoCmd()
{

    word_p *savestk = DSTop;
    word_p newobj = rplAllocTempOb(2);
    if(!newobj)
        return;
    // CREATE A PROGRAM AND RUN THE MAP COMMAND
    newobj[0] = MKPROLOG(DOCOL, 2);
    newobj[1] = CurOpcode;
    newobj[2] = CMD_SEMI;

    rplPushData(newobj);
    rplSetSystemFlag(FL_LISTCMDCLEANUP);

    rplCallOperator(CMD_MAP);

    if(Exceptions) {
        if(DSTop > savestk)
            DSTop = savestk;
    }

    // EXECUTION WILL CONTINUE AT MAP

    return;
}

// List handling for funtions with 1 argument
// Uses DOLIST instead of MAP so it doesn't recurse into the sublists
void rplListUnaryNonRecursiveDoCmd()
{

    word_p *savestk = DSTop;
    word_p newobj = rplAllocTempOb(2);
    if(!newobj)
        return;
    // CREATE A PROGRAM AND RUN THE MAP COMMAND
    newobj[0] = MKPROLOG(DOCOL, 2);
    newobj[1] = CurOpcode;
    newobj[2] = CMD_SEMI;

    rplPushDataNoGrow((word_p) one_bint);
    rplPushData(newobj);
    rplSetSystemFlag(FL_LISTCMDCLEANUP);

    rplCallOperator(CMD_CMDDOLIST);

    if(Exceptions) {
        if(DSTop > savestk)
            DSTop = savestk;
    }

    // EXECUTION WILL CONTINUE AT DOLIST

    return;
}

// List handling for funtions with 1 argument
void rplListUnaryNoResultDoCmd()
{

    word_p *savestk = DSTop;
    word_p newobj = rplAllocTempOb(2);
    if(!newobj)
        return;
    // CREATE A PROGRAM AND RUN THE MAP COMMAND
    newobj[0] = MKPROLOG(DOCOL, 2);
    newobj[1] = CurOpcode;
    newobj[2] = CMD_SEMI;

    rplPushData(newobj);
    rplSetSystemFlag(FL_LISTCMDCLEANUP);

    rplCallOperator(CMD_MAPINNERCOMP);

    if(Exceptions) {
        if(DSTop > savestk)
            DSTop = savestk;
    }

    // EXECUTION WILL CONTINUE AT MAP

    return;
}

// APPEND AN ITEM TO THE END OF THE LIST, DROP THE FIRST
// ELEMENT IF NEEDED TO KEEP THE LIST AT N ELEMENTS MAX.
// USES ScratchPointers 1 THRU 3
word_p rplListAddRot(word_p list, word_p object, int32_t nmax)
{
    int32_t nitems;
    word_p *savestk = DSTop;

    ScratchPointer3 = object;
    nitems = rplExplodeList2(list);
    if(Exceptions) {
        DSTop = savestk;
        return 0;
    }

    int k, offset = nitems - (nmax - 1);

    if(offset > 0) {
        for(k = nmax - 1; k > 0; --k)
            rplOverwriteData(k + offset, rplPeekData(k));
        rplDropData(offset);
    }
    rplPushData(ScratchPointer3);
    word_p newlist =
            rplCreateListN(((offset > 0) ? nmax : (nitems + 1)), 1, 1);
    if(Exceptions) {
        DSTop = savestk;
        return 0;
    }
    return newlist;
}

// CREATE A NEW LIST REPLACING THE OBJECT AT position WITH THE GIVEN object
// RETURNS POINTER TO NEW LIST, CAN TRIGGER GC.
// USES SCRATCHPOINTERS 1 AND 2
word_p rplListReplace(word_p list, int32_t position, word_p object)
{
    int32_t newobjsize = rplObjSize(object);
    word_p oldobject = rplGetListElement(list, position);
    if(!oldobject)
        return 0;       // INVALID INDEX?
    int32_t oldobjsize = rplObjSize(oldobject);
    int32_t oldobjoffset = oldobject - list;
    int32_t newsize = OBJSIZE(*list) + newobjsize - oldobjsize;
    ScratchPointer1 = list;
    ScratchPointer2 = object;
    word_p newlist = rplAllocTempOb(newsize);
    if(!newlist)
        return 0;
    *newlist = MKPROLOG(DOLIST, newsize);
    memmovew(newlist + 1, ScratchPointer1 + 1, oldobjoffset - 1);
    memmovew(newlist + oldobjoffset, ScratchPointer2, newobjsize);
    memmovew(newlist + oldobjoffset + newobjsize,
            ScratchPointer1 + oldobjoffset + oldobjsize,
            OBJSIZE(*ScratchPointer1) - (oldobjoffset + oldobjsize - 1));
    return newlist;
}

// CREATE A NEW LIST REPLACING MULTIPLE OBJECTS AT position WITH THE GIVEN object (IF A LIST, IT'S EXPLODED ON REPLACEMENT)
// RETURNS POINTER TO NEW LIST, CAN TRIGGER GC.
// USES SCRATCHPOINTERS 1 AND 2
word_p rplListReplaceMulti(word_p list, int32_t position, word_p object)
{
    int32_t numnewobj, llen, endpos;
    int32_t newobjsize, oldobjsize, oldobjoffset;
    word_p oldobject;

    newobjsize = rplObjSize(object);

    if(ISLIST(*object)) {
        numnewobj = rplListLength(object);
        newobjsize -= 2;
        ++object;
    }
    else
        numnewobj = 1;

    endpos = position + numnewobj;
    llen = rplListLength(list);
    if(endpos > llen)
        endpos = llen + 1;

    oldobject = rplGetListElement(list, position);
    if(!oldobject)
        return 0;       // INVALID INDEX?
    oldobjsize =
            (endpos ==
            llen + 1) ? (OBJSIZE(*list) - (oldobject -
                list)) : (rplGetListElement(list, endpos) - oldobject);
    oldobjoffset = oldobject - list;
    ScratchPointer1 = list;
    ScratchPointer2 = object;
    word_p newlist = rplAllocTempOb(OBJSIZE(*list) + newobjsize - oldobjsize);
    if(!newlist)
        return 0;
    *newlist = MKPROLOG(DOLIST, OBJSIZE(*list) + newobjsize - oldobjsize);
    memmovew(newlist + 1, ScratchPointer1 + 1, oldobjoffset - 1);
    memmovew(newlist + oldobjoffset, ScratchPointer2, newobjsize);
    memmovew(newlist + oldobjoffset + newobjsize,
            ScratchPointer1 + oldobjoffset + oldobjsize,
            OBJSIZE(*ScratchPointer1) - (oldobjoffset + oldobjsize - 1));
    return newlist;
}

// List handling for funtions with many arguments, only one of them can be a list

void rplListMultiArgDoCmd(int32_t nargs)
{
    int32_t k;
    for(k = nargs; k >= 1; --k) {

        if(ISLIST(*rplPeekData(k))) {

            int32_t j;
            int32_t size1 = 0;

            // GET THE SIZE OF ALL OTHER ARGUMENTS
            for(j = 1; j <= nargs; ++j) {
                if(j != k)
                    size1 += rplObjSize(rplPeekData(j));
            }

            word_p *savestk = DSTop;

            word_p newobj = rplAllocTempOb(7 + 6 + size1), newcmd;

            if(!newobj)
                return;

            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0] = MKPROLOG(DOCOL, 6 + size1);

            // COPY ALL OTHER ARGUMENTS
            int32_t offset = 1;
            for(j = nargs; j >= 1; --j) {
                if(j != k) {
                    rplCopyObject(newobj + offset, rplPeekData(j));
                    offset += rplObjSize(rplPeekData(j));
                }
            }
            // NOW MAKE SURE THE ARGUMENTS ARE IN THE PROPER ORDER
            if(k != nargs) {
                newobj[offset] = MAKESINT(nargs);
                ++offset;
                newobj[offset] = CMD_ROLL;
                ++offset;
                if(k != 1) {
                    newobj[offset] = MAKESINT(k);
                    ++offset;
                    newobj[offset] = CMD_ROLLD;
                    ++offset;
                }
            }
            // AND DO THE OPERATION
            newobj[offset] = CurOpcode;
            ++offset;
            newobj[offset] = CMD_SEMI;
            ++offset;

            newcmd = newobj + offset;
            // ANOTHER SECONDARY THAT DOES MAP THEN CLEANS UP THE STACK
            newobj[offset] = MKPROLOG(SECO, 6);
            ++offset;
            newobj[offset] = CMD_MAP;
            ++offset;
            newobj[offset] = MAKESINT(nargs + 1);
            ++offset;
            newobj[offset] = CMD_ROLLD;
            ++offset;
            newobj[offset] = MAKESINT(nargs);
            ++offset;
            newobj[offset] = CMD_DROPN;
            ++offset;
            newobj[offset] = CMD_SEMI;
            ++offset;

            rplPushDataNoGrow(rplPeekData(k));
            rplPushDataNoGrow(newobj);
            rplPushData(newcmd);

            rplSetSystemFlag(FL_LISTCMDCLEANUP);

            rplCallOvrOperator(CMD_OVR_XEQ);

            if(Exceptions) {
                if(DSTop > savestk)
                    DSTop = savestk;
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

int32_t rplListSame()
{
    word_p *a = DSTop - 2;
    word_p *b = DSTop - 1;
    int32_t aend, bend;
    int32_t aptr, bptr;

    aend = rplObjSize(*a);
    bend = rplObjSize(*b);

    aptr = bptr = 0;

    while((aptr < aend) && (bptr < bend)) {
        if(ISLIST((*a)[aptr]))
            ++aptr;
        if(ISLIST((*b)[bptr]))
            ++bptr;

        rplPushDataNoGrow((*a) + aptr);
        rplPushDataNoGrow((*b) + bptr);
        rplCallOvrOperator(CMD_OVR_SAME);
        if(Exceptions)
            return 0;

        if(rplIsFalse(rplPopData()))
            return 0;
        aptr += rplObjSize((*a) + aptr);
        bptr += rplObjSize((*b) + bptr);
    }
    if((aptr != aend) || (bptr != bend))
        return 0;
    return 1;

}

void rplListExpandCases()
{
    int32_t nelem1, nelem2, size1, size2;
    word_p list1, list2;

    list1 = rplPeekData(2);
    list2 = rplPeekData(1);

    if(!ISLIST(*list1))
        return; // ONLY EXPAND IF BOTH ARE LISTS
    if(!ISLIST(*list2))
        return;
    if((!ISAUTOEXPLIST(*list1)) && (!ISAUTOEXPLIST(*list2)))
        return; // AND AT LEAST ONE OF THEM IS A CASE LIST

    nelem1 = rplListLength(list1);
    nelem2 = rplListLength(list2);

    size1 = rplObjSize(list1) - 2;
    size2 = rplObjSize(list2) - 2;

    // FIRST LIST WILL HAVE nelem1*nelem2 ITEMS, WITH EACH INDIVIDUAL ITEM REPEATED nelem2 CONSECUTIVE TIMES
    // SECOND LIST WIL HAVE nelem1*nelem2 ITEMS, WITH THE ENTIRE LIST REPEATED nelem1 TIMES

    word_p newlist1 = rplAllocTempOb(size1 * nelem2 + 1);
    if(!newlist1)
        return;
    rplPushDataNoGrow(newlist1);

    word_p newlist2 = rplAllocTempOb(size2 * nelem1 + 1);
    if(!newlist2) {
        rplDropData(1);
        return;
    }

    // RE-READ ALL POINTERS IN CASE OF GC
    newlist1 = rplPopData();
    list1 = rplPeekData(2) + 1;
    list2 = rplPeekData(1) + 1;

    // COPY THE FIRST EXPANDED LIST
    // EACH ELEMENT REPEATED nelem2 TIMES
    newlist1[0] = MKPROLOG(DOCASELIST, size1 * nelem2);
    newlist1[size1 * nelem2 + 1] = CMD_ENDLIST;

    word_p src, srcptr, dest, srcend;
    int32_t k;

    src = list1;
    dest = newlist1 + 1;

    do {
        srcend = rplSkipOb(src);
        for(k = 0; k < nelem2; ++k) {
            srcptr = src;
            while(srcptr < srcend)
                *dest++ = *srcptr++;

        }
        src = srcend;
    }
    while(src < list1 + size1);

    // COPY THE SECOND EXPANDED LIST
    // THE ENTIRE LIST REPEATED nelem1 TIMES
    newlist2[0] = MKPROLOG(DOCASELIST, size2 * nelem1);
    newlist2[size2 * nelem1 + 1] = CMD_ENDLIST;

    src = list2;
    dest = newlist2 + 1;

    for(k = 0; k < nelem1; ++k) {
        memmovew(dest, src, size2);
        dest += size2;
    }

    // DONE, REPLACE THEM IN THE STACK
    rplOverwriteData(1, newlist2);
    rplOverwriteData(2, newlist1);
}

// RETURN TRUE IF ANY OF THE OBJECTS WITHIN A LIST IS A LIST
int32_t rplListHasLists(word_p list)
{
    word_p endlist = rplSkipOb(list);
    ++list;
    while(list < endlist) {
        if(ISLIST(*list))
            return 1;
        list = rplSkipOb(list);
    }
    return 0;
}
