#include "newrpl.h"
#include "libraries.h"



// GLOBAL SUPPORT FUNCTIONS FOR SYMBOLICS


/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */



/* COMPILING A SYMBOLIC:
 *
 * Compiler will switch to infix mode with the return value: OK_STARTCONSTRUCT_INFIX
 * And will return to RPN on ENDCONSTRUCT.
 *
 * In infix mode, the compiler sends OPCODE_MAXTOKEN to all libraries.
 * Libraries must determine if the token string starts with a token provided by the library.
 * Libraries reply with OK_TOKENINFO + MKTOKENINFO(precedence,nargs,length), with length=maximum
 * number of characters that the compiled token will absorb (length<=TokenLen)
 * At the same time, libraries must return the precedence of the compiled token they detected and
 * the number of arguments that this operator/function needs from the stack, and whether it is left
 * or right associative.
 * The compiler will choose the library that absorbs the most characters, will split the token
 * and pass the new token to the library to compile using OPCODE_COMPILE.
 *
 *
 */

/* Operators precedence should be:


31= BRACKETS/PARENTHESIS

16 = OVR_EVAL, OVR_XEQ

12= OVR_OR
11= OVR_XOR
10= OVR_AND
9= OVR_EQ, OVR_NOTEQ, OVR_SAME
8= OVR_LT OVR_GT OVR_LTE OVR_GTE

6= OVR_ADD, OVR_SUB

5= OVR_MUL,OVR_DIV,OVR_INV

4= OVR_NEG, OVR_ISTRUE, OVR_NOT

3= OVR_ABS,OVR_POW


2 = ALL OTHER FUNCTIONS AND COMMANDS


1 = COMPLEX IDENT
1 = REAL IDENTS
1 = CONSTANT IDENT
1 = COMPOSITE OBJECT
1 = NUMERIC TYPES

*/

// RETURN THE OPCODE OF THE MAIN OPERATOR OF THE SYMBOLIC,
// OR ZERO IF AN ATOMIC OBJECT
// ABLE TO DIG THROUGH MULTIPLE LAYERS OF DOSYMB WRAPPING

WORD rplSymbMainOperator(WORDPTR symbolic)
{
    WORDPTR endptr=rplSkipOb(symbolic);
    while( (ISSYMBOLIC(*(symbolic+1))) && ((symbolic+1)<endptr)) ++symbolic;
    if(symbolic+1>=endptr) return 0;
    if(!ISPROLOG(*(symbolic+1))) return *(symbolic+1);
    return 0;
}

// PEEL OFF USELESS LAYERS OF DOSYMB WRAPPING
// DO NOT CALL FOR ANY OBJECTS OTHER THAN A SYMBOLIC
// NO ARGUMENT CHECKS
WORDPTR rplSymbUnwrap(WORDPTR symbolic)
{
    WORDPTR endptr=rplSkipOb(symbolic);
    while( (ISSYMBOLIC(*(symbolic+1))) && ((symbolic+1)<endptr)) ++symbolic;
    if(symbolic+1>=endptr) return 0;
    return symbolic;
}


// RETURN 1 IF THE OBJECT IS ALLOWED WITHIN A SYMBOLIC, OTHERWISE 0

BINT rplIsAllowedInSymb(WORDPTR object)
{
    // CALL THE GETINFO OPCODE TO SEE IF IT'S ALLOWED
    LIBHANDLER handler=rplGetLibHandler(LIBNUM(*object));
    WORD savedopcode=CurOpcode;
    // ARGUMENTS TO PASS TO THE HANDLER
    DecompileObject=object;
    RetNum=-1;
    CurOpcode=MKOPCODE(LIBNUM(*object),OPCODE_GETINFO);
    if(handler) (*handler)();

    // RESTORE ORIGINAL OPCODE
    CurOpcode=savedopcode;

    if(RetNum>OK_TOKENINFO) {
        if(TI_TYPE(RetNum)==TITYPE_NOTALLOWED) return 0;
        return 1;
    }
    return 0;
}


// TAKE 'nargs' ITEMS FROM THE STACK AND APPLY THE OPERATOR OPCODE
// LEAVE THE NEW SYMBOLIC OBJECT IN THE STACK
// NO ARGUMENT CHECKS!
void rplSymbApplyOperator(WORD Opcode,BINT nargs)
{
    BINT f;
    WORDPTR obj,ptr;
    BINT size=0;
    for(f=1;f<=nargs;++f) {
        obj=rplPeekData(f);
        if(ISSYMBOLIC(*obj)) obj=rplSymbUnwrap(obj);
        size+=rplObjSize(obj);
    }
    size+=1;

    WORDPTR newobject=rplAllocTempOb(size);
    if(!newobject) return;

    newobject[0]=MKPROLOG(DOSYMB,size);
    newobject[1]=Opcode;
    ptr=newobject+2;
    for(f=nargs;f>0;--f) {
        obj=rplPeekData(f);
        if(ISSYMBOLIC(*obj)) obj=rplSymbUnwrap(obj);
        else {
            // CHECK IF IT'S ALLOWED IN SYMBOLICS
            LIBHANDLER han=rplGetLibHandler(LIBNUM(*obj));
            WORD savedopc=CurOpcode;
            CurOpcode=MKOPCODE(LIBNUM(*obj),OPCODE_GETINFO);
            RetNum=-1;
            if(han) (*han)();
            CurOpcode=savedopc;
            if(RetNum>OK_TOKENINFO) {
                if(TI_TYPE(RetNum)==TITYPE_NOTALLOWED) {
                    Exceptions|=EX_BADARGTYPE;
                    ExceptionPointer=IPtr;
                    return;
                }
            }
            else {
                Exceptions|=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }

        }
        rplCopyObject(ptr,obj);
        // REPLACE QUOTED IDENT WITH UNQUOTED ONES FOR SYMBOLIC OBJECTS
        if(LIBNUM(*ptr)==DOIDENT) *ptr=MKPROLOG(DOIDENTEVAL,OBJSIZE(*ptr));

        ptr=rplSkipOb(ptr);
    }
    rplDropData(nargs-1);
    rplOverwriteData(1,newobject);
}


// CHANGE THE SYMBOLIC TO CANONICAL FORM.
// CANONICAL FORM APPLIES THE FOLLOWING RULES:
// SUBTRACTION AND DIVISION ARE FOLDED INTO ADDITION AND MULTIPLICATION WITH NEG() AND INV()
// SUCCESSIVE ADDITION AND MULTIPLICATION LISTS ARE FLATTENED
// NEGATIVE NUMBERS ARE REPLACED WITH UNARY MINUS AND POSITIVE ONES.
// NEG() OPERATOR IS REPLACED WITH UMINUS
// ALL NUMERICAL TERMS ARE ADDED TOGETHER
// ALL NUMERICAL FACTORS IN THE NUMERATOR ARE MULTIPLIED TOGETHER
// ALL NUMERICAL FACTORS IN THE DENOMINATOR ARE MULTIPLIED TOGETHER
// SYMBOLIC FRACTIONS ARE REDUCED

