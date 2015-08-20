/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// GENERAL SYSTEM SANITY CHECK IMPLEMENTATION

// DETECT DATA CORRUPTION AND PREVENT BAD OBJECTS FROM BEING IMPORTED

// TAKE AN OBJECT AND CALL THE LIBRARY TO VALIDATE ITS DATA PAYLOAD
// RETURN TRUE IF OBJECT IS VALID, ZERO IF NOT

// DOES NOT CHECK FOR VALIDITY OF THE OBJECT POINTER

BINT rplVerifyObject(WORDPTR obj)
{
    int libnum=LIBNUM(*obj);
    LIBHANDLER han=rplGetLibHandler(libnum);
    if(!han) return 0;
    // EXECUTE THE LIBRARY DIRECTLY
    BINT SavedOpcode=CurOpcode;

    CurOpcode=MKOPCODE(libnum,OPCODE_CHECKOBJ);

    RetNum=ERR_INVALID;
    ObjectPTR=obj;

    (*han)();

    CurOpcode=SavedOpcode;

    if(RetNum!=OK_CONTINUE) return 0;

    return 1;
}

// VERIFY IF A POINTER TO AN OBJECT IS VALID
// OBJECT POINTERS HAVE TO EITHER:
// A) POINT WITHIN TEMPOB
// B) POINT TO AN OBJECT IN ROM



BINT rplVerifyObjPointer(WORDPTR ptr)
{
// CHECK IF POINTER IS WITHIN TEMPOB
if((ptr>=TempOb) && (ptr<TempObEnd)) return 1;

// POINTER IS NOT, CHECK IF IT'S A VALID ROM POINTER
// BY ASKING LIBRARIES TO RECOGNIZE IT
BINT libnum=MAXLIBNUMBER;
BINT SavedOpcode;
LIBHANDLER han;

SavedOpcode=CurOpcode;
CurOpcode=MKOPCODE(libnum,OPCODE_GETROMID);


while(libnum>=0) {
   han=rplGetLibHandler(libnum);
   if(han) {
       RetNum=ERR_INVALID;
       ObjectPTR=ptr;

       (*han)();

       if(RetNum==OK_CONTINUE) return 1;
   }
   --libnum;
}
CurOpcode=SavedOpcode;

// BAD POINTER
return 0;

}

// CHECK DATA STACK INTEGRITY
// DATA STACK CAN ONLY CONTAIN VALID OBJECT POINTERS

// WILL REPLACE INVALID POINTERS WITH zero_bint IF fix IS TRUE
// OTHERWISE LEAVES THE INVALID DATA AS-IS
// RETURNS 0 IF INVALID POINTERS FOUND (EVEN IF PATCHED)
// RETURNS 1 IF ALL POINTERS ON STACK ARE VALID AND POINT TO VALID OBJECTS

BINT rplVerifyDStack(BINT fix)
{
BINT errors=0;
WORDPTR *stkptr=DSTop;
WORDPTR *bottom=DStkBottom;

do {

while(stkptr>=bottom) {
    if(!rplVerifyObjPointer(*stkptr)) {
        if(!fix) return 0;
        // FIX THE BAD POINTER
        *stkptr=zero_bint;
        ++errors;
    } else {
        // POINTER IS GOOD, CHECK OBJECT
        if(!rplVerifyObject(*stkptr)) {
            if(!fix) return 0;
            *stkptr=zero_bint;
            ++errors;
        }
    }
    --stkptr;
}

// FINISHED CURRENT STACK, DO THE NEXT SNAPSHOT
if(bottom>DStk) {
    --bottom;
    BINT levels=(PTR2NUMBER) *bottom;
    if( (levels<=0) || ((bottom-levels)<DStk) || ((bottom-levels)>=DSTop)) {
        // INVALID SNAPSHOT!!
        if(!fix) return 0;
        *bottom=(WORDPTR) (bottom-DStk);
        ++errors;
    }
} else break;
} while(bottom>=DStk);

if(errors) return 0;
return 1;
}

