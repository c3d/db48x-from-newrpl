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

// GLOBAL SUPPORT FUNCTIONS FOR SYMBOLICS
#define num_max(a,b) ((a)>(b)? (a):(b))
#define num_min(a,b) ((a)<(b)? (a):(b))

// THIS IS FOR DEBUGGING ONLY - *MUST* BE REMOVED FOR RELEASE
//#define RULEDEBUG 1

/* COMPILING A SYMBOLIC:
 *
 * Compiler will switch to infix mode with the return value: OK_STARTCONSTRUCT_INFIX
 * And will return to RPN on ENDCONSTRUCT.
 *
 * In infix mode, the compiler sends OPCODE_PROBETOKEN to all libraries.
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

31= BRACKETS/PARENTHESIS/COMMA

16 = OVR_EVAL, OVR_EVAL1, OVR_XEQ
14 = RULESEPARATOR, EQUATIONEQUAL
13= OVR_OR
12= OVR_XOR
11= OVR_AND
10= OVR_EQ, OVR_NOTEQ
9= OVR_LT OVR_GT OVR_LTE OVR_GTE

8= OVR_ADD
7= OVR_SUB

6= OVR_MUL
5= OVR_DIV,OVR_INV

4= OVR_NEG, OVR_UMINUS, OVR_UPLUS

3= OVR_POW

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

WORD rplSymbMainOperator(word_p symbolic)
{
    word_p endptr = rplSkipOb(symbolic);
    while(((symbolic + 1) < endptr) && (ISSYMBOLIC(*(symbolic + 1))))
        ++symbolic;
    if(!ISSYMBOLIC(*symbolic))
        return 0;
    if(!ISPROLOG(*(symbolic + 1)) && !ISint32_t(*(symbolic + 1))
            && !ISCONSTANT(*(symbolic + 1)))
        return *(symbolic + 1);
    return 0;
}

// SAME AS ABOVE, BUT RETURN A POINTER TO THE OPCODE WITHIN THE SYMBOLIC
// BETTER TO PUSH ON STACK OR LAMS
word_p rplSymbMainOperatorPTR(word_p symbolic)
{
    word_p endptr = rplSkipOb(symbolic);
    while((ISSYMBOLIC(*(symbolic + 1))) && ((symbolic + 1) < endptr))
        ++symbolic;
    if(!ISSYMBOLIC(*symbolic))
        return 0;
    if(!ISPROLOG(*(symbolic + 1)) && !ISint32_t(*(symbolic + 1))
            && !ISCONSTANT(*(symbolic + 1)))
        return (symbolic + 1);
    return 0;
}

// PEEL OFF USELESS LAYERS OF DOSYMB WRAPPING
// DO NOT CALL FOR ANY OBJECTS OTHER THAN A SYMBOLIC
// NO ARGUMENT CHECKS
word_p rplSymbUnwrap(word_p symbolic)
{
    word_p endptr = rplSkipOb(symbolic);
    while(((symbolic + 1) < endptr) && (ISSYMBOLIC(*(symbolic + 1))))
        ++symbolic;
    return symbolic;
}

// RETURN 1 IF THE OBJECT IS ALLOWED WITHIN A SYMBOLIC, OTHERWISE 0

int32_t rplIsAllowedInSymb(word_p object)
{
    // CALL THE GETINFO OPCODE TO SEE IF IT'S ALLOWED
    LIBHANDLER handler = rplGetLibHandler(LIBNUM(*object));
    WORD savedopcode = CurOpcode;
    // ARGUMENTS TO PASS TO THE HANDLER
    DecompileObject = object;
    RetNum = 0;
    CurOpcode = MKOPCODE(LIBNUM(*object), OPCODE_GETINFO);
    if(handler)
        (*handler) ();

    // RESTORE ORIGINAL OPCODE
    CurOpcode = savedopcode;

    if(RetNum > OK_TOKENINFO) {
        if(TI_TYPE(RetNum) == TITYPE_NOTALLOWED)
            return 0;
        if(TI_TYPE(RetNum) == TITYPE_LIST) {
            // RECURSIVELY VERIFY THAT ALL ELEMENTS IN THE LIST ARE ALLOWED IN A SYMBOLIC!
            int32_t nitems = rplListLengthFlat(object);
            word_p objflat = rplGetListElementFlat(object, 1);
            while(nitems--) {
                CurOpcode = MKOPCODE(LIBNUM(*objflat), OPCODE_GETINFO);
                DecompileObject = objflat;
                RetNum = 0;
                LIBHANDLER han = rplGetLibHandler(LIBNUM(*objflat));
                if(han)
                    (*han) ();
                if(RetNum > OK_TOKENINFO) {
                    if(TI_TYPE(RetNum) == TITYPE_NOTALLOWED)
                        return 0;
                }
                else
                    return 0;

                objflat = rplGetNextListElementFlat(object, objflat);
            }
            // IF WE MADE IT HERE, ALL ITEMS ARE ACCEPTABLE
            CurOpcode = savedopcode;

        }

        return 1;
    }
    return 0;
}

// OBTAIN INFORMATION ABOUT A COMMAND/OPERATOR

int32_t rplSymbGetTokenInfo(word_p object)
{
    // CALL THE GETINFO OPCODE TO SEE IF IT'S ALLOWED
    LIBHANDLER handler = rplGetLibHandler(LIBNUM(*object));
    WORD savedopcode = CurOpcode;
    // ARGUMENTS TO PASS TO THE HANDLER
    DecompileObject = object;
    RetNum = 0;
    CurOpcode = MKOPCODE(LIBNUM(*object), OPCODE_GETINFO);
    if(handler)
        (*handler) ();

    // RESTORE ORIGINAL OPCODE
    CurOpcode = savedopcode;

    if(RetNum > OK_TOKENINFO)
        return RetNum & 0x3fffffff;
    return 0;
}

// TAKE 'nargs' ITEMS FROM THE STACK AND APPLY THE OPERATOR OPCODE
// LEAVE THE NEW SYMBOLIC OBJECT IN THE STACK
// NO ARGUMENT CHECKS!
void rplSymbApplyOperator(WORD Opcode, int32_t nargs)
{
    int32_t f;
    word_p obj, ptr;
    int32_t size = 0;
    for(f = 1; f <= nargs; ++f) {
        obj = rplPeekData(f);
        if(ISSYMBOLIC(*obj)) {
            obj = rplSymbUnwrap(obj);
            if(ISPROLOG(obj[1]) || ISint32_t(obj[1]))
                ++obj;  // POINT TO THE SINGLE OBJECT WITHIN THE SYMBOLIC WRAPPER
        }
        size += rplObjSize(obj);
    }
    size += 1;

    word_p newobject = rplAllocTempOb(size);
    if(!newobject)
        return;

    newobject[0] = MKPROLOG(DOSYMB, size);
    newobject[1] = Opcode;
    ptr = newobject + 2;
    for(f = nargs; f > 0; --f) {
        obj = rplPeekData(f);
        if(ISSYMBOLIC(*obj)) {
            obj = rplSymbUnwrap(obj);
            if(ISPROLOG(obj[1]) || ISint32_t(obj[1]))
                ++obj;  // POINT TO THE SINGLE OBJECT WITHIN THE SYMBOLIC WRAPPER
        }
        else {
            if(!rplIsAllowedInSymb(obj)) {
                rplError(ERR_NOTALLOWEDINSYMBOLICS);
                return;
            }
        }
        if(ISLIST(*obj)) {
            int32_t listsize = OBJSIZE(*obj);
            memmovew(ptr + 2, obj + 1, listsize - 1);
            ptr[0] = MKPROLOG(DOSYMB, listsize);
            ptr[1] = CMD_LISTOPENBRACKET;
            ptr += listsize + 1;        // OVERWRITE THE ENDLIST COMMAND IN THE LIST, NOT NEEDED IN SYMBOLICS
        }
        else {
            rplCopyObject(ptr, obj);
            // REPLACE QUOTED IDENT WITH UNQUOTED ONES FOR SYMBOLIC OBJECTS
            if(ISIDENT(*ptr))
                *ptr = SETLIBNUMBIT(*ptr, UNQUOTED_BIT);
            ptr = rplSkipOb(ptr);
        }
    }
    rplDropData(nargs - 1);
    rplOverwriteData(1, newobject);
}

// CONVERT A COMPLEX NUMBER TO A SYMBOLIC OBJECT REPRESENTATION
word_p rplComplexToSymb(word_p complex)
{
    if(!ISCOMPLEX(*complex))
            return complex;
    int32_t size, resize, imsize;
    size = rplObjSize(complex) + 5;     // DOSYMB + RE() DOSYMB * IM() i
    resize = rplObjSize(complex +1);
    imsize = rplObjSize(rplSkipOb(complex +1));

    int32_t ispolar = ISANGLE(*rplSkipOb(complex +1));
    if(ispolar)
        size += 4;      // DOSYMB * RE() DOSYMB ^ e DOSYMB * i IM()
    word_p newobj = rplAllocTempOb(size);
    if(!newobj)
        return 0;
    if(ispolar) {
        newobj[0] = MKPROLOG(DOSYMB, size);
        newobj[1] = CMD_OVR_MUL;
        memmovew(newobj + 2, complex +1, resize);
        newobj[2 + resize] = MKPROLOG(DOSYMB, imsize + 8);
        newobj[3 + resize] = CMD_OVR_POW;
        newobj[4 + resize] = MKPROLOG(DOCONST, 1);
        newobj[5 + resize] = MAKESINT(OPCODE(CMD_ECONST));
        newobj[6 + resize] = MKPROLOG(DOSYMB, (imsize + 4));
        newobj[7 + resize] = CMD_OVR_MUL;
        newobj[8 + resize] = MKPROLOG(DOCONST, 2);
        if(rplTestSystemFlag(FL_PREFERJ))
            newobj[9 + resize] = MAKESINT(OPCODE(CMD_JCONST));
        else
            newobj[9 + resize] = MAKESINT(OPCODE(CMD_ICONST));
        newobj[10 + resize] = newobj[9 + resize];
        memmovew(newobj + 11 + resize, rplSkipOb(complex +1), imsize);
        return newobj;
    }

    newobj[0] = MKPROLOG(DOSYMB, size);
    newobj[1] = CMD_OVR_ADD;
    memmovew(newobj + 2, complex +1, resize);
    newobj[2 + resize] = MKPROLOG(DOSYMB, (imsize + 4));
    newobj[3 + resize] = CMD_OVR_MUL;
    memmovew(newobj + 4 + resize, rplSkipOb(complex +1), imsize);
    newobj[4 + resize + imsize] = MKPROLOG(DOCONST, 2);
    if(rplTestSystemFlag(FL_PREFERJ))
        newobj[5 + resize + imsize] = MAKESINT(OPCODE(CMD_JCONST));
    else
        newobj[5 + resize + imsize] = MAKESINT(OPCODE(CMD_ICONST));
    newobj[6 + resize + imsize] = newobj[5 + resize + imsize];
    return newobj;
}

// ADD SYMBOLIC WRAP TO AN OBJECT (NO CHECKS, EXCEPT IT WILL PROCESS LISTS AND MATRICES
word_p rplSymbWrap(word_p obj)
{
    if(ISSYMBOLIC(*obj))
        return obj;
    if(!ISPROLOG(*obj) && !ISint32_t(*obj) && !ISCONSTANT(*obj))
        return obj;

    word_p firstobj, endobj, ptr, destptr;
    int32_t wrapcount;

    if(ISMATRIX(*obj)) {
        firstobj = rplMatrixGetFirstObj(obj);
        endobj = rplSkipOb(obj);

        wrapcount = 0;

        // FIRST PASS, DETERMINE HOW MANY WRAPS ARE NEEDED
        ptr = firstobj;
        while(ptr != endobj) {
            if(!ISSYMBOLIC(*ptr))
                ++wrapcount;
            ptr = rplSkipOb(ptr);
        }

        ScratchPointer1 = obj;
        word_p newobj = rplAllocTempOb(rplObjSize(obj) + wrapcount - 1);
        if(!newobj)
            return obj;

        // RELOAD ALL POINTERS DUE TO POSSIBLE GC
        firstobj = ScratchPointer1 + (firstobj - obj);
        endobj = ScratchPointer1 + (endobj - obj);
        obj = ScratchPointer1;

        memmovew(newobj, obj, firstobj - obj);  // COPY MATRIX DATA AND HASH TABLES
        newobj[0] += wrapcount; // FIX THE OBJECT SIZE
        ptr = firstobj;
        destptr = newobj + (firstobj - obj);

        while(ptr != endobj) {
            int32_t size = rplObjSize(ptr);
            if(!ISSYMBOLIC(*ptr)) {
                destptr[0] = MKPROLOG(DOSYMB, size);
                ++destptr;

                // PATCH ALL HASH TABLE OFFSETS
                int32_t tablesize = firstobj - obj - 2;
                word_p tableptr = newobj + 2;
                int32_t k;
                for(k = 0; k < tablesize; ++k) {
                    if(tableptr[k] >= destptr - newobj)
                        tableptr[k]++;
                }

            }
            memmovew(destptr, ptr, size);
            ptr += size;
            destptr += size;
        }

        return newobj;

    }

    if(ISLIST(*obj)) {
        firstobj = obj + 1;
        endobj = rplSkipOb(obj);

        wrapcount = 0;

        // FIRST PASS, DETERMINE HOW MANY WRAPS ARE NEEDED
        ptr = firstobj;
        while(ptr != endobj) {
            if(ISLIST(*ptr)) {
                ++ptr;
                continue;
            }
            if(!ISPROLOG(*ptr) && !ISint32_t(*ptr) && !ISCONSTANT(*ptr)) {
                ++ptr;
                continue;
            }
            if(ISMATRIX(*ptr)) {
                ptr = rplSkipOb(ptr);
                continue;
            }
            if(!ISSYMBOLIC(*ptr))
                ++wrapcount;
            ptr = rplSkipOb(ptr);
        }

        ScratchPointer1 = obj;
        word_p newobj = rplAllocTempOb(rplObjSize(obj) + wrapcount - 1);
        if(!newobj)
            return obj;

        // RELOAD ALL POINTERS DUE TO POSSIBLE GC
        firstobj = ScratchPointer1 + (firstobj - obj);
        endobj = ScratchPointer1 + (endobj - obj);
        obj = ScratchPointer1;

        newobj[0] = obj[0];     // COPY LIST PROLOG
        newobj[0] += wrapcount; // FIX THE OBJECT SIZE
        ptr = firstobj;
        destptr = newobj + 1;

        while(ptr != endobj) {
            int32_t size = rplObjSize(ptr);

            if(ISLIST(*ptr)) {
                destptr[0] = ptr[0] + wrapcount;
                ++destptr;
                ++ptr;
                continue;
            }
            if(!ISPROLOG(*ptr) && !ISint32_t(*ptr) && !ISCONSTANT(*ptr)) {
                *destptr = *ptr;
                ++destptr;
                ++ptr;
                continue;
            }
            if(!ISSYMBOLIC(*ptr) && !ISMATRIX(*ptr)) {
                destptr[0] = MKPROLOG(DOSYMB, size);
                ++destptr;
                --wrapcount;
            }
            memmovew(destptr, ptr, size);
            ptr += size;
            destptr += size;
        }

        return newobj;

    }

    if(ISCOMPLEX(*obj)) {
        word_p newsymb = rplComplexToSymb(obj);
        if(!newsymb)
            return obj;
        return newsymb;
    }

    // ALL OTHER OBJECTS ARE ATOMIC, ADD A SYMBOLIC WRAP

    int32_t size = rplObjSize(obj);

    ScratchPointer1 = obj;
    word_p newobject = rplAllocTempOb(size);
    obj = ScratchPointer1;
    if(!newobject)
        return obj;

    newobject[0] = MKPROLOG(DOSYMB, size);
    ptr = newobject + 1;
    memmovew(ptr, obj, size);
    // REPLACE QUOTED IDENT WITH UNQUOTED ONES FOR SYMBOLIC OBJECTS
    if(ISIDENT(*ptr))
        *ptr = SETLIBNUMBIT(*ptr, UNQUOTED_BIT);

    return newobject;

}

//  ANALYZE 'nargs' ITEMS ON THE STACK AND WRAP THEM INTO A SYMBOLIC OBJECT
// WHENEVER POSSIBLE (MOSTLY FOR NUMBERS)
// NO ARGUMENT CHECKS!
void rplSymbWrapN(int32_t level, int32_t nargs)
{
    int32_t f;
    word_p obj;
    for(f = 0; f < nargs; ++f) {
        obj = rplPeekData(f + level);
        rplOverwriteData(f + level, rplSymbWrap(obj));
    }
}

// EXPLODE A SYMBOLIC IN THE STACK IN REVERSE (LEVEL 1 CONTAINS THE FIRST OBJECT, LEVEL 2 THE SECOND, ETC.)
// INCLUDING OPERATORS
// USES ScratchPointer1 FOR GC PROTECTION
// RETURN THE NUMBER OF OBJECTS THAT ARE ON THE STACK

int32_t rplSymbExplode(word_p object)
{
    int32_t count = 0, countops = 0, nargs = 0;

    word_p ptr, end, numbers;
    word_p *sptr;

    ptr = object;
    end = rplSkipOb(object);

    while(ptr != end) {
        if(ISSYMBOLIC(*ptr)) {
            ++ptr;
            continue;
        }
        if(!(ISPROLOG(*ptr) || ISint32_t(*ptr) || ISCONSTANT(*ptr)))
            ++countops;
        ++count;
        ptr = rplSkipOb(ptr);
    }

    // HERE count+countops IS THE NUMBER OF POINTERS WE NEED TO PUSH ON THE STACK
    ScratchPointer1 = object;
    rplExpandStack(count + countops);   // EXPAND THE DATA STACK NOW, SO WE CAN MANUALLY PUSH IN BULK
    if(Exceptions)
        return 0;
    numbers = rplAllocTempOb(countops);
    if(!numbers)
        return 0;
    object = ScratchPointer1;
    sptr = DSTop + count + countops - 1;
    end = rplSkipOb(object);
    countops = 0;

    while(object != end) {
        if(ISSYMBOLIC(*object)) {
            nargs = 0;
            word_p tmp = object + 1, tmpend = rplSkipOb(object);
            while(tmp != tmpend) {
                ++nargs;
                tmp = rplSkipOb(tmp);
            }
            // HERE nargs HAS THE NUMBER OF ARGUMENTS + 1 FOR THE OPCODE
            ++object;
            continue;
        }
        *sptr = object;
        if(!(ISPROLOG(*object) || ISint32_t(*object) || ISCONSTANT(*object))) {
            numbers[countops] = MAKESINT(nargs);
            --sptr;
            *sptr = &numbers[countops];
            ++countops;
        }
        --sptr;
        object = rplSkipOb(object);
    }

    DSTop += count + countops;

    return count + countops;

}

// EXPLODE A SYMBOLIC IN THE STACK
// BUT ONLY EXPLODE THE OUTERMOST OPERATOR, KEEPING ITS ARGUMENTS UNEXPLODED
// USES ScratchPointer1 THRU 3 FOR GC PROTECTION
// RETURN THE NUMBER OF OBJECTS THAT ARE ON THE STACK
// LEVEL 1=OPERATOR, LEVEL2=NARGS, LEVEL3=LAST ARG ... LEVEL 2+NARGS = 1ST ARG

int32_t rplSymbExplodeOneLevel(word_p object)
{
    int32_t count = 0, countops = 0;

    ScratchPointer1 = rplSymbUnwrap(object) + 1;
    ScratchPointer2 = rplSkipOb(object);

    while(ScratchPointer1 < ScratchPointer2) {
        if(!(ISPROLOG(*ScratchPointer1) || ISint32_t(*ScratchPointer1)
                    || ISCONSTANT(*ScratchPointer1))) {
            ScratchPointer3 = ScratchPointer1;
            ++countops;
        }
        else {
            rplPushData(ScratchPointer1);
            ++count;
        }
        ScratchPointer1 = rplSkipOb(ScratchPointer1);
    }

    if(countops) {
        rplNewSINTPush(count, DECint32_t);
        rplPushData(ScratchPointer3);
        ++countops;
    }
    return count + countops;

}

// REASSEMBLE A SYMBOLIC THAT WAS EXPLODED IN THE STACK
// DOES NOT CHECK FOR VALIDITY OF THE SYMBOLIC!

word_p rplSymbImplode(word_p * exprstart)
{

    word_p *stkptr = exprstart;
    int32_t numobjects = 1, addcount = 0;
    int32_t size = 0, narg;

    int32_t f;

    for(f = 0; f < numobjects; ++f) {
        if(addcount) {
            numobjects += OBJSIZE(**stkptr) - 1;
            addcount = 0;
        }
        if((!ISint32_t(**stkptr)) && (!ISPROLOG(**stkptr)
                    && !ISCONSTANT(**stkptr))) {
            addcount = 1;
            ++numobjects;
        }
        size += rplObjSize(*stkptr);
        --stkptr;
    }

    // HERE size HAS THE TOTAL SIZE WE NEED TO ALLOCATE

    word_p newobject = rplAllocTempOb(size), newptr, object;

    if(!newobject)
        return 0;

    stkptr = exprstart;
    newptr = newobject;
    for(f = 0; f < numobjects; ++f) {
        object = *stkptr;
        if(!(ISPROLOG(*object) || ISint32_t(*object) || ISCONSTANT(*object))) {
            // WE HAVE AN OPCODE, START A SYMBOLIC RIGHT HERE
            *newptr++ = MKPROLOG(DOSYMB, 0);
            *newptr++ = *object;        // STORE THE OPCODE
            --stkptr;
            ++f;
        }
        else {
            if(ISLIST(*object)) {
                // CONVERT THE LIST INTO A SYMBOLIC LIST CONSTRUCTOR OPERATOR
                *newptr++ = MKPROLOG(DOSYMB, rplObjSize(object) - 1);
                if(ISAUTOEXPLIST(*object))
                    *newptr++ = CMD_CLISTOPENBRACKET;
                else
                    *newptr++ = CMD_LISTOPENBRACKET;
                // NOW COPY THE OBJECTS IN THE LIST
                word_p endobj = rplSkipOb(object) - 1; // STOP AT THE CMD_ENDLIST OBJECT
                int32_t k;
                object++;
                while(object != endobj) {
                    if(*object == CMD_ENDLIST) {
                        ++object;
                        continue;
                    }
                    if(ISLIST(*object)) {
                        *newptr++ = MKPROLOG(DOSYMB, rplObjSize(object) - 1);
                        if(ISAUTOEXPLIST(*object))
                            *newptr++ = CMD_CLISTOPENBRACKET;
                        else
                            *newptr++ = CMD_LISTOPENBRACKET;
                        object++;
                        continue;
                    }
                    for(k = 0; k < (int32_t) rplObjSize(object); ++k)
                        *newptr++ = *object++;
                }

            }
            else {
                if(f == 0) {
                    // FIRST OBJECT NEEDS A SYMBOLIC WRAPPER EVEN WITHOUT AN OPCODE
                    *newptr++ = MKPROLOG(DOSYMB, rplObjSize(object));
                }
                // COPY THE OBJECT
                word_p endobj = rplSkipOb(object);
                word_p prolog = newptr;
                while(object != endobj)
                    *newptr++ = *object++;
                if(ISQUOTEDIDENT(*prolog))
                    *prolog = MKPROLOG(LIBNUM(*prolog) | UNQUOTED_BIT, OBJSIZE(*prolog));       // FORCE UNQUOTED IDENTS WITHIN A SYMBOLIC
            }
        }

        --stkptr;

    }

    // HERE WE HAVE THE NEW OBJECT, BUT ALL SYMBOLIC SIZES ARE SET TO ZERO
    // NEED TO FIX THE SIZES ACCORDING TO THE NUMBER OF ARGUMENTS
    // newptr IS POINTING AT THE END OF THE NEW OBJECT

    for(; f > 0; --f) {
        ++stkptr;
        object = *stkptr;
        if(!(ISPROLOG(*object) || ISint32_t(*object) || ISCONSTANT(*object))) {
            // FOUND AN OPERATOR, GET THE NUMBER OF ITEMS
            narg = OPCODE(**(stkptr - 1));

            // PATCH THE LAST SYMBOLIC WITH ZERO FOR SIZE IN THE OBJECT
            word_p scan = newobject, lastone = 0;
            while(scan < newptr) {
                if(*scan == MKPROLOG(DOSYMB, 0)) {
                    lastone = scan;
                    ++scan;
                    continue;
                }
                scan = rplSkipOb(scan);
            }
            // HERE lastone HAS THE LAST SYMBOLIC FOUND
            scan = lastone + 1;
            while(narg) {
                scan = rplSkipOb(scan);
                --narg;
            }

            // AND PATCH THE SIZE
            *lastone = MKPROLOG(DOSYMB, scan - lastone - 1);

        }

    }

    return newobject;

}

// SKIP ONE SYMBOLIC OBJECT IN ITS EXPLODED FORM IN THE STACK
// THE ARGUMENT IS A POINTER TO THE STACK, NOT THE OBJECT
// RETURS THE POINTER TO THE STACK ELEMENT THAT HAS THE NEXT OBEJCT

word_p *rplSymbSkipInStack(word_p * stkptr)
{
    if(ISPROLOG(**stkptr))
        return --stkptr;
    if(ISint32_t(**stkptr) || ISCONSTANT(**stkptr))
        return --stkptr;

    // IT'S AN OPERATOR
    --stkptr;

    int32_t nargs = OPCODE(**stkptr) - 1;  // EXTRACT THE SINT
    --stkptr;
    while(nargs) {
        if(ISPROLOG(**stkptr)) {
            --stkptr;
            --nargs;
            continue;
        }
        if(ISint32_t(**stkptr) || ISCONSTANT(**stkptr)) {
            --stkptr;
            --nargs;
            continue;
        }
        --stkptr;
        nargs += OPCODE(**stkptr) - 1;
        --stkptr;
        --nargs;
    }

    return stkptr;
}

// CONVERT A SYMBOLIC INTO CANONICAL FORM:
// A.0) ALL COMPLEX NUMBERS REPLACED WITH THEIR SYMBOLIC REPRESENTATION
// A) NEGATIVE NUMBERS REPLACED WITH NEG(n)
// B) ALL SUBTRACTIONS REPLACED WITH ADDITION OF NEGATED ITEMS
// c) ALL NEG(A+B+...) = NEG(A)+NEG(B)+NEG(...)
// C.2) ALL NEG(NEG(...)) REPLACED WITH (...)
// D) FLATTEN ALL ADDITION TREES
// E) ALL NEGATIVE POWERS REPLACED WITH a^-n = INV(a^n)
// E.1) ALL UNIT POWERS ARE REMOVED (a^1 = a)
// F) ALL DIVISIONS REPLACED WITH MULTIPLICATION BY INV()
// G) ALL INV(A*B*...) = INV(...)*INV(B)*INV(A) (INVERTING THE ORDER OF TERMS)
// G.2) ALL NEG(A*B*...) = NEG(A)*B*...
// G.3) ALL INV(INV(...) = ...
// H) FLATTEN ALL MULTIPLICATION TREES
// THE FOLLOWING ADDITIONAL TRANSFORMATIONS ARE ONLY FOR DISPLAY:
// I) REORDER MULTIPLICATION TO PLACE INV() ITEMS LAST (FOR VISUAL AID ONLY)
// J) IF MULTIPLICATION STARTS WITH INV() PREPEND 1* SO IT DISPLAYS AS 1/... (FOR VISUAL AID ONLY)

ROMOBJECT uminus_opcode[] = {
    (CMD_OVR_UMINUS)
};

ROMOBJECT add_opcode[] = {
    (CMD_OVR_ADD)
};

ROMOBJECT inverse_opcode[] = {
    (CMD_OVR_INV)
};

ROMOBJECT mul_opcode[] = {
    (CMD_OVR_MUL)
};

word_p *rplSymbExplodeCanonicalForm(word_p object, int32_t for_display)
{
    int32_t numitems = rplSymbExplode(object);
    word_p *stkptr, sobj, *endofstk;

    stkptr = DSTop - 1;
    endofstk = stkptr - numitems;

    // *******************************************
    // SCAN THE SYMBOLIC FOR ITEM A)
    // A.0) COMPLEX NUMBER REPLACED WITH SYMBOLIC COMPLEX
    // A) NEGATIVE NUMBERS REPLACED WITH NEG(n)

    while(stkptr != endofstk) {
        sobj = *stkptr;

        if(ISCOMPLEX(*sobj)) {
            word_p newobj = rplComplexToSymb(sobj);
            if(!newobj) {
                DSTop = endofstk + 1;
                return 0;
            }
            int32_t numnew = rplSymbExplode(newobj);
            rplExpandStack(numnew);
            if(Exceptions) {
                DSTop = endofstk + 1;
                return 0;
            }
            // INSERT THE NEW EXPLODED COMPLEX INTO THE SYMBOLIC
            memmovew(stkptr + numnew, stkptr + 1,
                    ((endofstk + numitems -
                            stkptr) * sizeof(word_p)) / sizeof(WORD));
            memmovew(stkptr, DSTop - numnew,
                    (numnew * sizeof(word_p)) / sizeof(WORD));
            DSTop--;
            stkptr += numnew;
            numitems += numnew - 1;
            sobj = *stkptr;
        }

        if(ISint32_t(*sobj)) {
            // THE OBJECT IS AN INTEGER NUMBER
            int64_t num = rplReadint32_t(sobj);
            if(num < 0) {
                num = -num;
                rplNewint32_tPush(num, LIBNUM(*sobj));
                if(Exceptions) {
                    DSTop = endofstk + 1;
                    return 0;
                }
                word_p newobj = rplPeekData(1);

                word_p *ptr = DSTop - 2;

                // MAKE A HOLE IN THE STACK TO ADD NEGATION
                while(ptr != stkptr) {
                    *(ptr + 2) = *ptr;
                    --ptr;
                }
                DSTop++;        // MOVE ONLY ONE SPOT, DROPPING THE NEW OBJECT IN THE SAME OPERATION
                rplExpandStack(1);
                stkptr[0] = newobj;
                stkptr[1] = (word_p) two_bint;
                stkptr[2] = (word_p) uminus_opcode;
            }
        }

        if(ISREAL(*sobj)) {
            // THE OBJECT IS A REAL NUMBER
            REAL dec;
            rplReadReal(sobj, &dec);
            if(dec.flags & F_NEGATIVE) {
                if(!iszeroReal(&dec))
                    dec.flags ^= F_NEGATIVE;    // MAKE IT POSITIVE
                rplNewRealPush(&dec);
                if(Exceptions) {
                    DSTop = endofstk + 1;
                    return 0;
                }
                word_p newobj = rplPeekData(1);

                word_p *ptr = DSTop - 2;

                // MAKE A HOLE IN THE STACK TO ADD NEGATION
                while(ptr != stkptr) {
                    *(ptr + 2) = *ptr;
                    --ptr;
                }
                DSTop++;        // MOVE ONLY ONE SPOT, DROPPING THE NEW OBJECT IN THE SAME OPERATION
                rplExpandStack(1);
                stkptr[0] = newobj;
                stkptr[1] = (word_p) two_bint;
                stkptr[2] = (word_p) uminus_opcode;
            }
        }

        --stkptr;
    }

    // *******************************************
    // SCAN THE SYMBOLIC FOR ITEM B)
    // B) ALL SUBTRACTIONS REPLACED WITH ADDITION OF NEGATED ITEMS

    stkptr = DSTop - 1;

    while(stkptr != endofstk) {
        sobj = *stkptr;

        if(*sobj == (CMD_OVR_SUB)) {

            word_p *secondarg = rplSymbSkipInStack(stkptr - 2);

            word_p *ptr = DSTop - 1;

            // MAKE A HOLE IN THE STACK TO ADD NEGATION
            while(ptr != secondarg) {
                *(ptr + 2) = *ptr;
                --ptr;
            }
            DSTop += 2; // 2 PLACES IN THE STACK ARE GUARANTEED BY STACK SLACK
            stkptr += 2;
            secondarg[1] = (word_p) two_bint;
            secondarg[2] = (word_p) uminus_opcode;
            *stkptr = (word_p) add_opcode;
            stkptr--;
            rplExpandStack(2);  // NOW GROW THE STACK
            if(Exceptions) {
                DSTop = endofstk + 1;
                return 0;
            }
        }

        --stkptr;
    }

    // *******************************************
    // SCAN THE SYMBOLIC FOR ITEM C)
    // C) ALL NEG(A+B+...) = NEG(A)+NEG(B)+NEG(...)

    stkptr = DSTop - 1;
    while(stkptr != endofstk) {
        sobj = *stkptr;

        if(*sobj == (CMD_OVR_UMINUS)) {
            word_p *nextarg = stkptr - 2;

            if(**nextarg == (CMD_OVR_ADD)) {
                // A SUM NEGATED? DISTRIBUTE THE OPERATOR OVER THE ARGUMENTS

                int32_t nargs = OPCODE(**(nextarg - 1)) - 1;

                int32_t c;
                nextarg -= 2;
                for(c = 0; c < nargs; ++c) {

                    if(**nextarg == (CMD_OVR_UMINUS)) {
                        // NEG/NEG = REMOVE THE NEGATION
                        word_p *ptr = nextarg - 1;
                        // AND REMOVE THE GAP
                        while(ptr != DSTop - 2) {
                            *ptr = *(ptr + 2);
                            ++ptr;
                        }
                        DSTop -= 2;
                        stkptr -= 2;
                        nextarg -= 2;
                    }
                    else {
                        // NEGATE THIS TERM
                        word_p *ptr = DSTop - 1;
                        // AND REMOVE THE GAP
                        while(ptr != nextarg) {
                            *(ptr + 2) = *ptr;
                            --ptr;
                        }

                        DSTop += 2;
                        stkptr += 2;
                        nextarg[1] = (word_p) two_bint;
                        nextarg[2] = (word_p) uminus_opcode;
                        nextarg += 2;
                    }

                    nextarg = rplSymbSkipInStack(nextarg);

                }
                // REMOVE THE ORIGINAL NEGATION
                word_p *ptr = stkptr - 1;
                // AND REMOVE THE GAP
                while(ptr != DSTop - 2) {
                    *ptr = *(ptr + 2);
                    ++ptr;
                }
                DSTop -= 2;
                stkptr -= 2;

            }
            else
                stkptr--;
        }

        --stkptr;
    }

    // *******************************************
    // SCAN THE SYMBOLIC FOR ITEM D)
    // D) FLATTEN ALL ADDITION TREES

    stkptr = DSTop - 1;
    while(stkptr != endofstk) {
        sobj = *stkptr;

        if(*sobj == (CMD_OVR_ADD)) {
            int32_t nargs = OPCODE(**(stkptr - 1)) - 1;

            int32_t c, orignargs = nargs;
            word_p *nextarg = stkptr - 2;

            for(c = 0; c < nargs; ++c) {

                if(**nextarg == (CMD_OVR_ADD)) {
                    // FLATTEN BY REMOVING THE ADDITION
                    word_p *ptr = nextarg - 1;
                    nargs += OPCODE(**(nextarg - 1)) - 2;       // ADD THE ARGUMENTS TO THE BASE LOOP
                    // AND REMOVE THE GAP
                    while(ptr != DSTop - 2) {
                        *ptr = *(ptr + 2);
                        ++ptr;
                    }
                    DSTop -= 2;
                    stkptr -= 2;
                    --c;
                    nextarg -= 2;
                }
                else
                    nextarg = rplSymbSkipInStack(nextarg);

            }

            // HERE stkptr IS POINTING TO THE ORIGINAL SUM COMMAND
            // STORE THE NEW TOTAL NUMBER OF ARGUMENTS
            if(orignargs != nargs) {
                word_p newnumber = rplNewSINT(nargs + 1, DECint32_t);
                if(!newnumber) {
                    DSTop = endofstk + 1;
                    return 0;
                }
                *(stkptr - 1) = newnumber;
            }

            stkptr--;
        }

        --stkptr;
    }

    // *******************************************
    // SCAN THE SYMBOLIC FOR ITEM E)
    // E) ALL NEGATIVE POWERS REPLACED WITH a^-n = INV(a^n)

    stkptr = DSTop - 1;

    while(stkptr != endofstk) {
        sobj = *stkptr;

        if(*sobj == (CMD_OVR_POW)) {

            word_p *arg1 = stkptr - 2;
            word_p *arg2 = rplSymbSkipInStack(arg1);

            if(**arg2 == (CMD_OVR_UMINUS)) {
                // NEGATIVE POWER DETECTED WE JUST NEED TO REPLACE THE UMINUS
                // WITH AN INV()

                // MOVE EVERYTHING TWO LEVELS UP
                word_p *ptr = arg2 - 1;
                while(ptr != stkptr + 1) {
                    *ptr = *(ptr + 2);
                    ++ptr;
                }

                *stkptr = (word_p) inverse_opcode;
                --stkptr;
                *stkptr = (word_p) two_bint;
            }
        }

        --stkptr;
    }

    // *******************************************
    // SCAN THE SYMBOLIC FOR ITEM F)
    // F) ALL DIVISIONS REPLACED WITH MULTIPLICATION BY INV()

    stkptr = DSTop - 1;

    while(stkptr != endofstk) {
        sobj = *stkptr;

        if(*sobj == (CMD_OVR_DIV)) {

            word_p *secondarg = rplSymbSkipInStack(stkptr - 2);

            word_p *ptr = DSTop - 1;

            // MAKE A HOLE IN THE STACK TO ADD INVERSE
            while(ptr != secondarg) {
                *(ptr + 2) = *ptr;
                --ptr;
            }
            DSTop += 2; // 2 PLACES IN THE STACK ARE GUARANTEED BY STACK SLACK
            stkptr += 2;
            secondarg[1] = (word_p) two_bint;
            secondarg[2] = (word_p) inverse_opcode;
            *stkptr = (word_p) mul_opcode;
            stkptr--;
            rplExpandStack(2);  // NOW GROW THE STACK
            if(Exceptions) {
                DSTop = endofstk + 1;
                return 0;
            }
        }

        --stkptr;
    }

    // *******************************************
    // SCAN THE SYMBOLIC FOR ITEM G)
    // G) ALL INV(A*B*...) = INV(...)*INV(B)*INV(A)

    stkptr = DSTop - 1;
    while(stkptr != endofstk) {
        sobj = *stkptr;

        if(*sobj == (CMD_OVR_INV)) {
            word_p *nextarg = stkptr - 2;

            if(**nextarg == (CMD_OVR_MUL)) {

                int32_t nargs = OPCODE(**(nextarg - 1)) - 1;

                int32_t c;
                nextarg -= 2;
                for(c = 0; c < nargs; ++c) {

                    if(**nextarg == (CMD_OVR_INV)) {
                        // INV/INV = REMOVE THE OPERATOR
                        word_p *ptr = nextarg - 1;
                        // AND REMOVE THE GAP
                        while(ptr != DSTop - 2) {
                            *ptr = *(ptr + 2);
                            ++ptr;
                        }
                        DSTop -= 2;
                        stkptr -= 2;
                        nextarg -= 2;
                    }
                    else {
                        // INVERT THIS TERM
                        word_p *ptr = DSTop - 1;
                        // AND REMOVE THE GAP
                        while(ptr != nextarg) {
                            *(ptr + 2) = *ptr;
                            --ptr;
                        }

                        DSTop += 2;
                        stkptr += 2;
                        nextarg[1] = (word_p) two_bint;
                        nextarg[2] = (word_p) inverse_opcode;
                        nextarg += 2;
                        rplExpandStack(2);
                        if(Exceptions) {
                            DSTop = endofstk + 1;
                            return 0;
                        }
                    }

                    nextarg = rplSymbSkipInStack(nextarg);

                }

                nextarg = stkptr - 4;
                // INVERT THE ORDER OF TERMS
                for(c = 1; c <= nargs / 2; ++c) {

                    word_p *ptr = nextarg;
                    word_p *nextptr = rplSymbSkipInStack(ptr);
                    word_p *otherarg = nextptr, *endofotherarg;
                    int32_t k;
                    for(k = 0; k < nargs - 2 * c; ++k)
                        otherarg = rplSymbSkipInStack(otherarg);        // FIND THE ARGUMENT TO SWAP PLACES WITH

                    endofotherarg = rplSymbSkipInStack(otherarg);
                    // DO THE SWAP
                    if(otherarg - endofotherarg <= ptr - nextptr) {
                        // FIRST ARGUMENT IS BIGGER
                        int32_t offset =
                                (ptr - nextptr) - (otherarg - endofotherarg);
                        rplExpandStack(ptr - nextptr);  // NOW GROW THE STACK
                        if(Exceptions) {
                            DSTop = endofstk + 1;
                            return 0;
                        }
                        memmovew(DSTop, nextptr + 1, (ptr - nextptr) * sizeof(word_p) / sizeof(WORD)); // COPY FIRST ARGUMENT OUT OF THE WAY
                        // COPY ARGUMENT FROM THE END
                        nextptr = otherarg + offset;
                        while(otherarg != endofotherarg) {
                            *ptr = *otherarg;
                            --ptr;
                            --otherarg;
                        }
                        // SHIFT ALL OTHER ARGUMENTS
                        while(ptr != nextptr) {
                            *ptr = ptr[-offset];
                            --ptr;
                        }
                        ptr = DSTop;
                        // MOVE BACK ORIGINAL ARGUMENT
                        while(endofotherarg < nextptr) {
                            ++endofotherarg;
                            *endofotherarg = *ptr;
                            ++ptr;
                        }

                    }
                    else {
                        // LAST ARGUMENT IS BIGGER
                        int32_t offset =
                                (otherarg - endofotherarg) - (ptr - nextptr);
                        rplExpandStack(otherarg - endofotherarg);       // NOW GROW THE STACK
                        if(Exceptions) {
                            DSTop = endofstk + 1;
                            return 0;
                        }
                        memmovew(DSTop, endofotherarg + 1, (otherarg - endofotherarg) * sizeof(word_p) / sizeof(WORD));        // COPY LAST ARGUMENT OUT OF THE WAY
                        // COPY ARGUMENT FROM THE END
                        otherarg = nextptr - offset;
                        while(nextptr < ptr) {
                            ++endofotherarg;
                            ++nextptr;
                            *endofotherarg = *nextptr;
                        }
                        // SHIFT ALL OTHER ARGUMENTS
                        while(endofotherarg != otherarg) {
                            ++endofotherarg;
                            *endofotherarg = endofotherarg[offset];
                        }
                        otherarg = DSTop;
                        // MOVE BACK ORIGINAL ARGUMENT
                        while(endofotherarg < ptr) {
                            ++endofotherarg;
                            *endofotherarg = *otherarg;
                            ++otherarg;
                        }

                    }

                    nextarg = rplSymbSkipInStack(nextarg);

                }

                // REMOVE THE ORIGINAL INVERSION
                word_p *ptr = stkptr - 1;
                // AND REMOVE THE GAP
                while(ptr != DSTop - 2) {
                    *ptr = *(ptr + 2);
                    ++ptr;
                }
                DSTop -= 2;
                stkptr -= 2;

            }
            else
                stkptr--;
        }

        --stkptr;
    }

    // *******************************************
    // SCAN THE SYMBOLIC FOR ITEM G.2)
    // G.2) ALL NEG(A*B*...) = NEG(A)*B*...

    stkptr = DSTop - 1;
    while(stkptr != endofstk) {
        sobj = *stkptr;

        if(*sobj == (CMD_OVR_UMINUS)) {
            word_p *nextarg = stkptr - 2;

            if(**nextarg == (CMD_OVR_MUL)) {

                word_p tmp;

                // SWAP THE MUL WITH THE NEG
                tmp = *stkptr;
                *stkptr = *nextarg;
                *nextarg = tmp;
                tmp = *(stkptr - 1);
                *(stkptr - 1) = *(nextarg - 1);
                *(nextarg - 1) = tmp;
            }
        }

        --stkptr;
    }

    // *******************************************
    // SCAN THE SYMBOLIC FOR ITEM G.3)
    // G.3) ALL NEG(NEG(...)) = (...)

    stkptr = DSTop - 1;
    while(stkptr != endofstk) {
        sobj = *stkptr;

        if(*sobj == (CMD_OVR_UMINUS)) {
            word_p *nextarg = stkptr - 2;

            if(**nextarg == (CMD_OVR_UMINUS)) {
                // NEG/NEG = REMOVE THE NEGATION
                word_p *ptr = nextarg - 1;
                // AND REMOVE THE GAP
                while(ptr != DSTop - 4) {
                    *ptr = *(ptr + 4);
                    ++ptr;
                }
                DSTop -= 4;
                stkptr -= 4;
            }
            else
                stkptr--;
        }

        --stkptr;
    }

    // *******************************************
    // SCAN THE SYMBOLIC FOR ITEM H)
    // H) FLATTEN ALL MULTIPLICATION TREES

    stkptr = DSTop - 1;
    while(stkptr != endofstk) {
        sobj = *stkptr;

        if(*sobj == (CMD_OVR_MUL)) {
            int32_t nargs = OPCODE(**(stkptr - 1)) - 1;

            int32_t c, orignargs = nargs;
            word_p *nextarg = stkptr - 2;

            for(c = 0; c < nargs; ++c) {

                if(**nextarg == (CMD_OVR_MUL)) {
                    // FLATTEN BY REMOVING THE ADDITION
                    word_p *ptr = nextarg - 1;
                    nargs += OPCODE(**(nextarg - 1)) - 2;       // ADD THE ARGUMENTS TO THE BASE LOOP
                    // AND REMOVE THE GAP
                    while(ptr != DSTop - 2) {
                        *ptr = *(ptr + 2);
                        ++ptr;
                    }
                    DSTop -= 2;
                    stkptr -= 2;
                    --c;
                    nextarg -= 2;
                }
                else
                    nextarg = rplSymbSkipInStack(nextarg);

            }

            // HERE stkptr IS POINTING TO THE ORIGINAL MUL COMMAND
            // STORE THE NEW TOTAL NUMBER OF ARGUMENTS
            if(orignargs != nargs) {
                word_p newnumber = rplNewSINT(nargs + 1, DECint32_t);
                if(!newnumber) {
                    DSTop = endofstk + 1;
                    return 0;
                }
                *(stkptr - 1) = newnumber;
            }

            stkptr--;
        }

        --stkptr;
    }

    if(for_display) {
        // *******************************************
        // SCAN THE SYMBOLIC FOR ITEM I)
        // I) SORT ALL MULTIPLICATIONS WITH INV(...) LAST, NON-INVERSE FACTORS FIRST
        // ALSO, IF ALL FACTORS ARE INV(...), THEN ADD A int32_t 1 AS FIRST ELEMENT (1/X)

        stkptr = DSTop - 1;
        while(stkptr != endofstk) {
            sobj = *stkptr;

            if(*sobj == (CMD_OVR_MUL)) {
                int32_t nargs = OPCODE(**(stkptr - 1)) - 1;

                int32_t c;
                word_p *nextarg = stkptr - 2;
                word_p *firstarg = nextarg;
                word_p *firstinv = 0;

                for(c = 0; c < nargs; ++c) {

                    if(**nextarg != (CMD_OVR_INV)) {
                        if(firstinv) {
                            // MOVE nextarg BEFORE firstinv
                            int32_t nterms = nextarg - rplSymbSkipInStack(nextarg);
                            // HERE THE LAYOUT IS: DSTop ... firstinv... otherobj ... nextarg ... otherobj... end
                            //                               ^________________________|
                            // GROW STACK BY nterms
                            rplExpandStack(nterms);
                            if(Exceptions) {
                                DSTop = endofstk + 1;
                                return 0;
                            }
                            // MOVE nextarg TO THE END OF STACK
                            word_p *ptr = DSTop;
                            int32_t f;
                            for(f = 0; f < nterms; ++f)
                                ptr[f] = nextarg[-(nterms - 1) + f];
                            // MOVE firstinv BACK
                            ptr = nextarg + 1;
                            while(ptr <= firstinv) {
                                ptr[-nterms] = *ptr;
                                ++ptr;
                            }
                            // MOVE nextarg BACK IN PLACE OF firstinv
                            ptr = DSTop + nterms - 1;
                            for(f = 0; f < nterms; ++f, --ptr)
                                firstinv[-f] = *ptr;
                            firstinv -= nterms; // KEEP FIRST INV POINTING TO THE SAME PLACE
                            nextarg -= nterms;
                        }
                        else
                            nextarg = rplSymbSkipInStack(nextarg);

                    }
                    else {
                        if(!firstinv)
                            firstinv = nextarg;
                        nextarg = rplSymbSkipInStack(nextarg);
                    }

                }

                if(firstarg == firstinv) {
                    // ALL FACTORS ARE INVERTED

                    // ADD A int32_t 1 TO CREATE 1/X
                    word_p *ptr = DSTop - 1;

                    // MAKE A HOLE IN THE STACK TO ADD int32_t ONE
                    while(ptr != firstarg) {
                        *(ptr + 1) = *ptr;
                        --ptr;
                    }
                    DSTop++;    // 2 PLACES IN THE STACK ARE GUARANTEED BY STACK SLACK
                    stkptr++;
                    firstarg[1] = (word_p) one_bint;
                    // INCREASE THE COUNT OF OBJECTS
                    int64_t numargs = OPCODE(*firstarg[2]);
                    ++numargs;
                    word_p nnum = rplNewSINT(numargs, DECint32_t);
                    if(Exceptions) {
                        DSTop = endofstk + 1;
                        return 0;
                    }
                    firstarg[2] = nnum;
                    rplExpandStack(1);  // NOW GROW THE STACK
                    if(Exceptions) {
                        DSTop = endofstk + 1;
                        return 0;
                    }
                }

                stkptr--;
            }

            --stkptr;
        }

        // *******************************************
        // SCAN THE SYMBOLIC FOR ITEM J)
        // J) ANY EXPRESSION STARTING WITH INV() NEEDS TO BE REPLACED WITH 1*INV(), EXCEPT MUL ARGUMENTS

        stkptr = DSTop - 1;

        if(**stkptr == (CMD_OVR_INV)) {
            // NEED TO ADD 1*INV() AT THE BEGINNING OF THE EXPRESSION

            DSTop += 3; // 3 PLACES IN THE STACK ARE GUARANTEED BY STACK SLACK
            stkptr += 3;
            stkptr[0] = (word_p) mul_opcode;
            stkptr[-1] = (word_p) three_bint;
            stkptr[-2] = (word_p) one_bint;
            rplExpandStack(3);  // NOW GROW THE STACK
            if(Exceptions) {
                DSTop = endofstk + 1;
                return 0;
            }

        }

        // AND NOW CHECK THE REST OF IT

        while(stkptr != endofstk) {
            sobj = *stkptr;

            if(ISPROLOG(*sobj) || ISint32_t(*sobj) || ISCONSTANT(*sobj)) {
                --stkptr;
                continue;
            }

            if(*sobj != (CMD_OVR_MUL)) {
                // EXCEPT MULTIPLICATIONS, CHECK IF ANY OTHER EXPRESSION STARTS WITH INV()

                int32_t nargs = OPCODE(**(stkptr - 1)) - 1;

                word_p *argptr = stkptr - 2;

                while(nargs) {
                    if(**argptr == (CMD_OVR_INV)) {
                        // IN ANY OTHER CASE, NEED TO ADD 1*INV()

                        word_p *ptr = DSTop - 1;

                        // MAKE A HOLE IN THE STACK TO ADD int32_t ONE
                        while(ptr != argptr) {
                            *(ptr + 3) = *ptr;
                            --ptr;
                        }
                        DSTop += 3;     // 3 PLACES IN THE STACK ARE GUARANTEED BY STACK SLACK
                        stkptr += 3;
                        argptr += 3;
                        argptr[0] = (word_p) mul_opcode;
                        argptr[-1] = (word_p) three_bint;
                        argptr[-2] = (word_p) one_bint;
                        rplExpandStack(3);      // NOW GROW THE STACK
                        if(Exceptions) {
                            DSTop = endofstk + 1;
                            return 0;
                        }

                    }
                    argptr = rplSymbSkipInStack(argptr);
                    --nargs;
                }
            }
            --stkptr;
        }

    }   // END OF TRANSFORMATIONS FOR DISPLAY

    if(Exceptions) {
        DSTop = endofstk + 1;
        return 0;
    }

    return DSTop - 1;

}

// CONVERT A SYMBOLIC OBJECT TO CANONICAL FORM
word_p rplSymbCanonicalForm(word_p object, int32_t fordisplay)
{
    word_p *result = rplSymbExplodeCanonicalForm(object, fordisplay);

    if(!result)
        return 0;

    word_p finalsymb = rplSymbImplode(result);

    DSTop = rplSymbSkipInStack(result) + 1;
    if(Exceptions)
        return 0;

    return finalsymb;

}

// RECEIVES A NUMERATOR AND A DENOMINATOR NUMBERS IN THE STACK
// SIMPLIFIES BY DIVIDING BY THEIR GCD
// RETURNS 1 IF THERE WERE CHANGES, 0 IF NO SIMPLIFICATION WAS POSSIBLE

int32_t rplFractionSimplify()
{

    if((!ISNUMBER(*rplPeekData(1))) || (!ISNUMBER(*rplPeekData(2))))
        return 0;       // DON'T TRY TO SIMPLIFY IF NOT A NUMBER

    int32_t isapprox;
    isapprox = ISAPPROX(*rplPeekData(2)) | ISAPPROX(*rplPeekData(1));

    if(isapprox || ISREAL(*rplPeekData(2)) || ISREAL(*rplPeekData(1))) {
        // TREAT ALL NUMBERS AS REALS

        int32_t numneg, denneg;

        rplNumberToRReg(0, rplPeekData(2));     // REGISTER 0 = NUMERATOR
        rplNumberToRReg(1, rplPeekData(1));     // REGISTER 1 = DENOMINATOR

        if(!isapprox) {
            // MAKE THEM BOTH POSITIVE
            numneg = RReg[0].flags & F_NEGATIVE;
            denneg = RReg[1].flags & F_NEGATIVE;
            RReg[0].flags ^= numneg;
            RReg[1].flags ^= denneg;

            // IF IT HAS FRACTIONAL PART, MAKE IT INTEGER BY MULTIPLYING BOTH NUM AND DEN BY 10^N
            if(RReg[0].exp < 0) {
                RReg[1].exp += -RReg[0].exp;
                RReg[0].exp = 0;
            }
            if(RReg[1].exp < 0) {
                RReg[0].exp += -RReg[1].exp;
                RReg[1].exp = 0;
            }

            // SWITCH TO MAX CONTEXT
            WORD previousprec = Context.precdigits;
            Context.precdigits = REAL_PRECISION_MAX;

            // FIND GCD
            REAL *big, *small, *tmpbig, *tmpsmall, *swap, *remainder;
            if(cmpReal(&RReg[0], &RReg[1]) > 0) {
                big = &RReg[0];
                small = &RReg[1];
            }
            else {
                big = &RReg[1];
                small = &RReg[0];
            }
            tmpbig = &RReg[2];
            tmpsmall = &RReg[3];
            remainder = &RReg[4];

            copyReal(tmpbig, big);
            copyReal(tmpsmall, small);

            while(!iszeroReal(tmpsmall)) {

                divmodReal(&RReg[7], remainder, tmpbig, tmpsmall);

                swap = tmpbig;
                tmpbig = tmpsmall;
                tmpsmall = remainder;
                remainder = swap;

            }

            // HERE tmpbig = GCD(NUM,DEN)
            rplOneToRReg(5);
            if(cmpReal(tmpbig, &RReg[5]) <= 0) {
                // THERE'S NO COMMON DIVISOR, RETURN UNMODIFIED
                // THIS IS <=0 SO IT CATCHES 0/0
                Context.precdigits = previousprec;
                return 0;
            }

            // SIMPLIFY
            divReal(&RReg[5], &RReg[0], tmpbig);
            divReal(&RReg[6], &RReg[1], tmpbig);

            // APPLY THE SIGN TO THE NUMERATOR ONLY
            RReg[5].flags |= numneg ^ denneg;

            // HERE RREG[5]=NEW NUMERATOR, RREG[6]=NEW DENOMINATOR
            Context.precdigits = previousprec;
        }
        else {
            // JUST COMPUTE THE DIVISION
            divReal(&RReg[5], &RReg[0], &RReg[1]);
            // AND SET DENOMINATOR TO ONE
            rplOneToRReg(6);

        }

        // NOW TRY TO CONVERT THE REALS TO INTEGERS IF POSSIBLE
        int64_t num;
        if(isintegerReal(&RReg[5]) && inint64_tRange(&RReg[5])) {
            num = getint64_tReal(&RReg[5]);
            rplNewint32_tPush(num, DECint32_t | isapprox);
        }
        else {
            rplNewRealFromRRegPush(5);
        }

        if(Exceptions)
            return 0;
        if(isintegerReal(&RReg[6]) && inint64_tRange(&RReg[6])) {
            num = getint64_tReal(&RReg[6]);
            rplNewint32_tPush(num, DECint32_t | isapprox);
        }
        else {
            rplNewRealFromRRegPush(6);
        }
        if(Exceptions) {
            rplDropData(1);
            return 0;
        }

        rplOverwriteData(3, rplPeekData(1));
        rplOverwriteData(4, rplPeekData(2));
        rplDropData(2);
        return 1;

    }

    // BOTH NUMBERS ARE EXACT int32_tS

    int64_t bnum, bden;
    int64_t tmpbig, tmpsmall, swap;
    int32_t numneg, denneg;

    bnum = rplReadint32_t(rplPeekData(2));
    bden = rplReadint32_t(rplPeekData(1));

    // GET THE SIGNS
    if(bnum < 0) {
        numneg = 1;
        bnum = -bnum;
    }
    else
        numneg = 0;
    if(bden < 0) {
        denneg = 1;
        bden = -bden;
    }
    else
        denneg = 0;

    // CALCULATE THE GCD
    tmpbig = num_max(bnum, bden);
    tmpsmall = num_min(bnum, bden);

    while(tmpsmall > 0) {

        while(tmpbig >= tmpsmall)
            tmpbig -= tmpsmall;

        swap = tmpbig;
        tmpbig = tmpsmall;
        tmpsmall = swap;

    }

    // HERE tmpbig HAS THE GCD

    if(tmpbig <= 1) {
        // CHECK IF WE NEED TO CORRECT SIGNS
        if(!denneg)
            return 0;   // NO COMMON DIVISOR, SO RETURN WITH NO CHANGES
    }
    else {
        // SIMPLIFY
        bnum /= tmpbig;
        bden /= tmpbig;
    }

    // APPLY THE SIGN TO THE NUMERATOR ONLY
    if(numneg ^ denneg)
        bnum = -bnum;

    rplNewint32_tPush(bnum, DECint32_t);
    if(Exceptions)
        return 0;
    rplNewint32_tPush(bden, DECint32_t);
    if(Exceptions) {
        rplDropData(1);
        return 0;
    }

    rplOverwriteData(3, rplPeekData(1));
    rplOverwriteData(4, rplPeekData(2));
    rplDropData(2);
    return 1;

}

// CHECK IF ARGUMENT IN THE STACK IS A NUMERIC FRACTION
// RETURNS TRUE/FALSE

int32_t rplSymbIsFractionInStack(word_p * stkptr)
{

    if(**stkptr == (CMD_OVR_UMINUS)) {
// COULD BE A NEGATIVE FRACTION -(1/2)
        stkptr -= 2;
    }

//NOT A FRACTION UNLESS THERE'S A MULTIPLICATION
    if(**stkptr == (CMD_OVR_MUL)) {
        stkptr--;
        int32_t nargs = OBJSIZE(**stkptr) - 1;
// NOT A FRACTION IF MORE THAN 2 ARGUMENTS
        if(nargs != 2)
            return 0;
        --stkptr;

        word_p *argptr = stkptr;

// CHECK THE NUMERATOR

        if(**argptr == (CMD_OVR_UMINUS)) {
            if(!ISNUMBERORUNIT(**(argptr - 2)))
                return 0;
        }
        else if(!ISNUMBERORUNIT(**argptr))
            return 0;

        argptr = rplSymbSkipInStack(argptr);

// CHECK THE DENOMINATOR
        if(**argptr != (CMD_OVR_INV))
            return 0;
        argptr -= 2;
        if(**argptr == (CMD_OVR_UMINUS)) {
            if(!ISNUMBERORUNIT(**(argptr - 2)))
                return 0;
        }
        else if(!ISNUMBERORUNIT(**argptr))
            return 0;

    }
    else {
// SINGLE NUMBERS ARE ALSO CONSIDERED FRACTIONS N/1
        if(!ISNUMBERORUNIT(**stkptr))
            return 0;

    }

    return 1;
}

// EXTRACT AND PUSH PUSH NUMERATOR AND DENOMINATOR ON THE STACK
// DEAL WITH NEGATIVE NUMBERS
// DOES NOT CHECK FOR ARGUMENTS! CALLER TO USE rplSymbIsFractionInStack() TO VERIFY

void rplSymbFractionExtractNumDen(word_p * stkptr)
{
    int32_t negnum = 0, negden = 0;
    word_p *savedstop = DSTop;

    if(**stkptr == (CMD_OVR_UMINUS)) {
// COULD BE A NEGATIVE FRACTION -(1/2)
        negnum = 1;
        stkptr -= 2;
    }

    if(**stkptr == (CMD_OVR_MUL)) {

        stkptr -= 2;

        word_p *argptr = stkptr;

        // CHECK THE NUMERATOR

        if(**argptr == (CMD_OVR_UMINUS)) {
            negnum ^= 1;
            rplPushData(*(argptr - 2));
        }
        else
            rplPushData(*argptr);

        // NUMERATOR IS IN THE STACK
        if(negnum) {
            rplCallOvrOperator((CMD_OVR_NEG));
            if(Exceptions) {
                DSTop = savedstop;
                return;
            }
        }

        argptr = rplSymbSkipInStack(argptr);

        // CHECK THE DENOMINATOR
        argptr -= 2;    // SKIP THE INVERSE OPERATOR
        if(**argptr == (CMD_OVR_UMINUS)) {
            negden ^= 1;
            rplPushData(*(argptr - 2));
        }
        else
            rplPushData(*argptr);

        // DENOMINATOR IS IN THE STACK
        if(negden) {
            rplCallOvrOperator((CMD_OVR_NEG));
            if(Exceptions) {
                DSTop = savedstop;
                return;
            }
        }

    }
    else {
// SINGLE NUMBERS ARE ALSO CONSIDERED FRACTIONS N/1
        if(**stkptr == (CMD_OVR_UMINUS)) {
            negnum ^= 1;
            rplPushData(*(stkptr - 2));
        }
        else
            rplPushData(*stkptr);

        // NUMERATOR IS IN THE STACK
        if(negnum) {
            rplCallOvrOperator((CMD_OVR_NEG));
            if(Exceptions) {
                DSTop = savedstop;
                return;
            }
        }

        // DENOMINATOR IS ONE
        rplPushData((word_p) one_bint);

    }

    return;
}

// TAKE A NUMERIC FRACTION STORED IN THE STACK AS:
// 4:      NUM1
// 3:      DEN1
// 2:      NUM2
// 1:      DEN2

// REPLACE WITH:
// 2:    NUM1*DEN2+NUM2*DEN1
// 1:    DEN1*DEN2

// DOES NOT APPLY ANY SIMPLIFICATION
// MAKES RESULTING NUM AND DEN POSITIVE, AND RETURNS THE SIGN OF THE RESULTING FRACTION 0=POSITIVE, 1=NEGATIVE

int32_t rplFractionAdd()
{
    int32_t sign = 0;
    rplPushData(rplPeekData(4));        // NUM1
    rplPushData(rplPeekData(2));        // DEN2
    rplCallOvrOperator((CMD_OVR_MUL));
    if(Exceptions)
        return 0;
    rplPushData(rplPeekData(3));        // NUM2
    rplPushData(rplPeekData(5));        // DEN1
    rplCallOvrOperator((CMD_OVR_MUL));
    if(Exceptions)
        return 0;
    rplCallOvrOperator((CMD_OVR_ADD));
    if(Exceptions)
        return 0;

    rplPushData(rplPeekData(4));        // DEN1
    rplPushData(rplPeekData(3));        // DEN2
    rplCallOvrOperator((CMD_OVR_MUL));
    if(Exceptions)
        return 0;

    // TODO: IF NUM OR DEN ARE NEGATIVE, CHANGE THE SIGN AND SET sign APPROPRIATELY

    // CHECK SIGN OF THE NUMERATOR
    rplPushData(rplPeekData(2));
    rplPushData((word_p) zero_bint);
    rplCallOvrOperator((CMD_OVR_LT));

    // RESULT OF COMPARISON OPERATORS IS ALWAYS A SINT OR A SYMBOLIC
    word_p numsign = rplPeekData(1);
    rplDropData(1);
    if(ISint32_t(*numsign)) {
        if(*numsign != MAKESINT(0)) {
            rplPushData(rplPeekData(2));
            rplCallOvrOperator((CMD_OVR_NEG));
            rplOverwriteData(3, rplPeekData(1));
            rplDropData(1);
            sign ^= 1;
        }
    }

    // CHECK SIGN OF THE DENOMINATOR JUST IN CASE
    rplPushData(rplPeekData(1));
    rplPushData((word_p) zero_bint);
    rplCallOvrOperator((CMD_OVR_LT));

    // RESULT OF COMPARISON OPERATORS IS ALWAYS A SINT OR A SYMBOLIC
    word_p densign = rplPeekData(1);
    rplDropData(1);
    if(ISint32_t(*densign)) {
        if(*densign != MAKESINT(0)) {
            rplCallOvrOperator((CMD_OVR_NEG));
            sign ^= 1;
        }
    }

    rplOverwriteData(6, rplPeekData(2));
    rplOverwriteData(5, rplPeekData(1));
    rplDropData(4);

    return sign;

}

// REMOVE A SYMBOLIC OBJECT THAT IS EXPANDED IN THE STACK
// RETURNS THE SIZE OF THE OBJECT IN WORDS, CALLER HAS TO UPDATE
// ANY POINTERS INTO THE STACK THAT ARE > obj

int32_t rplSymbRemoveInStack(word_p * obj)
{
    word_p *end = rplSymbSkipInStack(obj);
    int32_t offset = obj - end;
    ++end;
    ++obj;

    while(obj != DSTop) {
        *end = *obj;
        ++end;
        ++obj;
    }
    return offset;
}

// MAKE ROOM IN STACK TO INSERT nwords IMMEDIATELY BEFORE here
// RETURNS nwords
int32_t rplSymbInsertInStack(word_p * here, int32_t nwords)
{
    rplExpandStack(nwords);
    if(Exceptions)
        return 0;

    word_p *ptr = DSTop - 1;

    while(ptr != here) {
        ptr[nwords] = *ptr;
        --ptr;
    }

    return nwords;
}

// REMOVE nwords IMMEDIATELY AFTER here (INCLUDED)
// RETURNS nwords
int32_t rplSymbDeleteInStack(word_p * here, int32_t nwords)
{
    here++;

    while(here != DSTop) {
        here[-nwords] = *here;
        ++here;
    }

    return nwords;
}

// TAKES A SYMBOLIC OBJECT AND PERFORMS NUMERIC SIMPLIFICATION:
// DONE! A) IN ALL OPS, EXCEPT MUL AND ADD, IF ALL ARGUMENTS ARE NUMERIC, THEN PERFORM THE OPERATION AND REPLACE BY THEIR RESULT
// B) IN ADD, ALL NUMERIC VALUES ARE ADDED TOGETHER AND REPLACED BY THEIR RESULT
// DONE! C.1) IN MUL, ALL NUMERATOR NUMERIC VALUES ARE MULTIPLIED TOGETHER AND REPLACED BY THEIR RESULT
// DONE! C.2) IN MUL, ALL DENOMINATOR NUMERIC VALUES ARE MULTIPLIED TOGETHER AND REPLACED BY THEIR RESULT
// D) IN ADD, IF TWO TERMS ARE NUMERIC EXPRESSIONS, PERFORM A FRACTION ADDITION (N1/D1+N2/D2=(N1*D2+N2*D1)/(D1*D2)
// DONE! E) IN MUL, ALL NUMERATOR AND DENOMINATOR NUMERICS ARE DIVIDED BY THEIR GCD (FRACTION SIMPLIFICATION)

word_p rplSymbNumericReduce(word_p object)
{
    int32_t numitems = rplSymbExplode(object);
    int32_t f, changed, origprec;
    word_p *stkptr, sobj, *endofstk;

    origprec = Context.precdigits;

    endofstk = DSTop - 1 - numitems;

// SCAN THE SYMBOLIC

    changed = 1;

    while(changed) {

        stkptr = DSTop - 1;
        changed = 0;

        while(stkptr != endofstk) {
            sobj = *stkptr;

            if(ISPROLOG(*sobj) || ISint32_t(*sobj) || ISCONSTANT(*sobj)) {
                --stkptr;
                continue;
            }

            if((*sobj == (CMD_OVR_MUL)) || (*sobj == CMD_SYMBTOUNIT)) {
                // SCAN ALL NUMERIC FACTORS IN THE NUMERATOR AND MULTIPLY TOGETHER

                int32_t nargs = OPCODE(**(stkptr - 1)) - 1, redargs = 0;
                word_p *argptr = stkptr - 2;
                int32_t simplified = 0, den_is_one = 0, num_is_one = 0, neg =
                        0, approx = 0;

                for(f = 0; f < nargs; ++f) {
                    if(!ISNUMBERORUNIT(**argptr)) {
                        // CHECK IF IT'S A NEGATIVE NUMBER
                        if(**argptr == (CMD_OVR_UMINUS)) {
                            if(ISNUMBERORUNIT(**(argptr - 2))) {
                                if(ISAPPROX(**(argptr - 2)))
                                    ++approx;
                                rplPushData(*(argptr - 2));
                                // NEGATE THE NUMBER

                                rplCallOvrOperator(OVR_NEG);
                                if(Exceptions) {
                                    rplBlameError(sobj);
                                    DSTop = endofstk + 1;
                                    return 0;
                                }

                                // REMOVE THE ARGUMENT FROM THE LIST

                                word_p *ptr, *endofobj = rplSymbSkipInStack(argptr);   // POINT TO THE NEXT OBJECT
                                ptr = endofobj + 1;
                                ++argptr;
                                stkptr -= (argptr - ptr);
                                // NOW CLOSE THE GAP
                                while(argptr != DSTop) {
                                    *ptr = *argptr;
                                    ++argptr;
                                    ++ptr;
                                }
                                DSTop = ptr;
                                argptr = endofobj;

                                // ARGUMENT WAS COMPLETELY REMOVED, NOW REDUCE THE ARGUMENT COUNT

                                ++redargs;
                                continue;
                            }
                            else {
                                // IT'S A NEGATIVE EXPRESSION, EXTRACT THE NEGATIVE SIGN AS A NUMERIC QUANTITY
                                neg ^= 1;
                                // REMOVE THE NEGATION
                                rplSymbDeleteInStack(argptr, 2);
                                argptr -= 2;
                                stkptr -= 2;
                                DSTop -= 2;

                            }
                        }
                    }
                    else {
                        // THIS IS A NUMBER
                        if(ISAPPROX(**argptr))
                            ++approx;
                        rplPushData(*(argptr));

                        // REMOVE THE ARGUMENT FROM THE LIST

                        word_p *ptr, *endofobj = rplSymbSkipInStack(argptr);   // POINT TO THE NEXT OBJECT
                        ptr = endofobj + 1;
                        ++argptr;
                        stkptr -= (argptr - ptr);
                        // NOW CLOSE THE GAP
                        while(argptr != DSTop) {
                            *ptr = *argptr;
                            ++argptr;
                            ++ptr;
                        }
                        DSTop = ptr;
                        argptr = endofobj;

                        // ARGUMENT WAS COMPLETELY REMOVED, NOW REDUCE THE ARGUMENT COUNT

                        ++redargs;
                        continue;

                    }

                    argptr = rplSymbSkipInStack(argptr);
                }

                // HERE WE HAVE redargs VALUES IN THE STACK THAT NEED TO BE MULTIPLIED TOGETHER
                if(redargs > 0) {
                    if(!approx)
                        Context.precdigits = REAL_PRECISION_MAX;
                    for(f = 1; f < redargs; ++f) {
                        rplCallOvrOperator(OVR_MUL);
                        if(Exceptions) {
                            rplBlameError(sobj);
                            DSTop = endofstk + 1;
                            return 0;
                        }
                    }

                    Context.precdigits = origprec;
                }
                else
                    rplPushData((word_p) one_bint);    //  IF NO NUMERATOR, THEN MAKE IT = 1

                // HERE WE HAVE A NUMERATOR RESULT IN THE STACK! KEEP IT THERE FOR NOW

                // SCAN ALL NUMERIC FACTORS IN THE DENOMINATOR AND MULTIPLY TOGETHER
                int32_t reddenom = 0, approxdenom = 0, addfactors = 0;
                argptr = stkptr - 2;

                for(f = 0; f < nargs - redargs; ++f) {
                    if(**argptr == (CMD_OVR_INV)) {

                        if(!ISNUMBERORUNIT(**(argptr - 2))) {
                            // CHECK IF IT'S A NEGATIVE NUMBER
                            if(**(argptr - 2) == (CMD_OVR_UMINUS)) {
                                if(ISNUMBERORUNIT(**(argptr - 4))) {
                                    if(ISAPPROX(**(argptr - 4)))
                                        ++approxdenom;
                                    rplPushData(*(argptr - 4));
                                    // NEGATE THE NUMBER
                                    Context.precdigits = REAL_PRECISION_MAX;
                                    //Context.traps|=MPD_Inexact;         // THROW AN EXCEPTION WHEN RESULT IS INEXACT

                                    rplCallOvrOperator(OVR_NEG);
                                    if(Exceptions) {
                                        DSTop = endofstk + 1;
                                        return 0;
                                    }

                                    Context.precdigits = origprec;
                                    //Context.traps&=~MPD_Inexact;         // BACK TO NORMAL

                                    // REMOVE THE ARGUMENT FROM THE LIST

                                    word_p *ptr, *endofobj = rplSymbSkipInStack(argptr);       // POINT TO THE NEXT OBJECT
                                    ptr = endofobj + 1;
                                    ++argptr;
                                    stkptr -= (argptr - ptr);
                                    // NOW CLOSE THE GAP
                                    while(argptr != DSTop) {
                                        *ptr = *argptr;
                                        ++argptr;
                                        ++ptr;
                                    }
                                    DSTop = ptr;
                                    argptr = endofobj;

                                    // ARGUMENT WAS COMPLETELY REMOVED, NOW REDUCE THE ARGUMENT COUNT

                                    ++reddenom;
                                    continue;
                                }
                                else {
                                    // IT'S A NEGATIVE EXPRESSION, EXTRACT THE SIGN
                                    neg ^= 1;
                                    // REMOVE THE NEGATION
                                    rplSymbDeleteInStack(argptr - 2, 2);
                                    argptr -= 2;
                                    stkptr -= 2;
                                    DSTop -= 2;
                                }
                            }
                        }
                        else {
                            // THIS IS A NUMBER
                            if(ISAPPROX(**(argptr - 2)))
                                ++approxdenom;
                            rplPushData(*(argptr - 2));

                            // REMOVE THE ARGUMENT FROM THE LIST

                            word_p *ptr, *endofobj = rplSymbSkipInStack(argptr);       // POINT TO THE NEXT OBJECT
                            ptr = endofobj + 1;
                            ++argptr;
                            stkptr -= (argptr - ptr);
                            // NOW CLOSE THE GAP
                            while(argptr != DSTop) {
                                *ptr = *argptr;
                                ++argptr;
                                ++ptr;
                            }
                            DSTop = ptr;
                            argptr = endofobj;

                            // ARGUMENT WAS COMPLETELY REMOVED, NOW REDUCE THE ARGUMENT COUNT

                            ++reddenom;
                            continue;

                        }
                    }

                    argptr = rplSymbSkipInStack(argptr);
                }

                // HERE WE HAVE reddenom VALUES IN THE STACK THAT NEED TO BE MULTIPLIED TOGETHER
                if(reddenom > 0) {

                    if(!approxdenom)
                        Context.precdigits = REAL_PRECISION_MAX;

                    for(f = 1; f < reddenom; ++f) {
                        rplCallOvrOperator(OVR_MUL);
                        if(Exceptions) {
                            rplBlameError(sobj);
                            DSTop = endofstk + 1;
                            return 0;
                        }
                    }

                    Context.precdigits = origprec;

                    // DONE, WE HAVE NUMERATOR AND DENOMINATOR IN THE STACK
                    if(!approx && !approxdenom) {
                        // FIND THE GCD OF THE NUMERATOR AND DENOMINATOR

                        // DIVIDE BOTH BY THE GCD
                        simplified = rplFractionSimplify();

                    }
                    else {
                        // DO THE DIVISION NOW
                        rplCallOvrOperator(OVR_DIV);
                        if(Exceptions) {
                            rplBlameError(sobj);
                            DSTop = endofstk + 1;
                            return 0;
                        }
                        // AND PUSH A DENOMINATOR OF 1
                        rplPushData((word_p) one_bint);
                        simplified = 1;
                    }
                }

                // PUT BOTH NUMBERS BACK IN PLACE

                {

                    if((redargs > 0) || (reddenom > 0)) {

                        int32_t n = 1 + ((reddenom > 0) ? 1 : 0);
                        // IF NUMERATOR IS NEGATIVE, STORE AS POSITIVE AND SET neg
                        if(ISint32_t(*rplPeekData(n))) {
                            int64_t nnum = rplReadint32_t(rplPeekData(n));
                            // MARK TO ADD THE SIGN LATER
                            if(nnum < 0) {
                                neg ^= 1;
                                // KEEP THE NUMERATOR POSITIVE
                                word_p newnum = rplNewint32_t(-nnum, DECint32_t);
                                if(!newnum) {
                                    DSTop = endofstk + 1;
                                    return 0;
                                }
                                rplOverwriteData(n, newnum);
                            }
                            if(nnum == 1)
                                num_is_one = 1;

                        }
                        else {
                            if(ISREAL(*rplPeekData(n))) {
                                REAL number;
                                rplReadReal(rplPeekData(n), &number);
                                if(number.flags & F_NEGATIVE) {
                                    number.flags ^= F_NEGATIVE;
                                    neg ^= 1;
                                    word_p newnum = rplNewReal(&number);
                                    if(!newnum) {
                                        DSTop = endofstk + 1;
                                        return 0;
                                    }
                                    rplOverwriteData(n, newnum);
                                }
                                rplOneToRReg(0);
                                if(eqReal(&number, &RReg[0]))
                                    num_is_one = 1;
                            }
                        }

                        if(!(num_is_one && (reddenom + redargs < nargs))) {
                            // ONLY ADD THE NUMERATOR ONE WHEN THERE'S NO OTHER (NON-NUMERIC) FACTORS

                            // IF THERE WERE ANY FACTORS IN THE NUMERATOR, REPLACE WITH THE NEW RESULT
                            word_p *ptr = DSTop - 1;

                            // MAKE ROOM
                            while(ptr != stkptr - 2) {
                                ptr[1] = *ptr;
                                --ptr;
                            }

                            ++stkptr;
                            ++DSTop;

                            ++addfactors;

                            *(stkptr - 2) = rplPeekData(1 + ((reddenom > 0) ? 1 : 0));  // STORE THE NUMERATOR

                        }

                    }

                    if(reddenom > 0) {
                        // IF THERE WERE ANY FACTORS IN THE DENOMINATOR, ADD THE RESULT

                        // IF DENOMINATOR IS ONE, THEN DON'T INCLUDE IT IN THE OBJECT

                        if(ISint32_t(*rplPeekData(1))) {
                            int64_t denom = rplReadint32_t(rplPeekData(1));
                            if(denom == 1)
                                den_is_one = 1;
                        }
                        else {
                            if(ISREAL(*rplPeekData(1))) {
                                REAL number;
                                rplReadReal(rplPeekData(1), &number);
                                rplOneToRReg(0);
                                if(cmpReal(&number, &RReg[0]) == 0)
                                    den_is_one = 1;
                            }
                        }

                        if(!den_is_one) {
                            // ONLY INSERT IN THE OBJECT IF THE DENOMINATOR IS NOT ONE
                            int32_t actualargs =
                                    nargs - redargs - reddenom + ((redargs >
                                        0) ? 1 : 0);
                            if(num_is_one && (reddenom + redargs < nargs))
                                --actualargs;
                            word_p *endofobj = stkptr - 2;
                            for(f = 0; f < actualargs; ++f)
                                endofobj = rplSymbSkipInStack(endofobj);
                            word_p *ptr = stkptr - 2;
                            // FIND THE FIRST FACTOR IN THE DENOMINATOR
                            while(ptr != endofobj) {
                                if(**ptr == (CMD_OVR_INV))
                                    break;
                                ptr = rplSymbSkipInStack(ptr);
                            }

                            // MAKE ROOM
                            endofobj = ptr;
                            ptr = DSTop - 1;
                            while(ptr != endofobj) {
                                ptr[3] = *ptr;
                                --ptr;
                            }

                            stkptr += 3;
                            DSTop += 3;
                            ptr[1] = rplPeekData(1);    // STORE THE DENOMINATOR
                            ptr[2] = (word_p) two_bint;
                            ptr[3] = (word_p) inverse_opcode;
                            ++addfactors;

                        }
                        --DSTop;

                    }

                    DSTop--;

                    if(neg) {
                        // HERE stkptr IS POINTING TO THE MULTIPLICATION
                        rplSymbInsertInStack(stkptr - 2, 2);
                        *stkptr = (word_p) uminus_opcode;
                        *(stkptr - 1) = (word_p) two_bint;
                        stkptr += 2;
                        DSTop += 2;
                    }

                    if(redargs + reddenom) {
                        // UPDATE THE ARGUMENT COUNT
                        int32_t newcount = nargs - redargs - reddenom + addfactors;

                        if(newcount < 2) {
                            // SINGLE ARGUMENT, SO REMOVE THE MULTIPLICATION
                            word_p *ptr = stkptr - 1;
                            while(ptr != DSTop) {
                                *ptr = *(ptr + 2);
                                ++ptr;
                            }
                            DSTop -= 2;
                            stkptr -= 2;

                        }
                        else {
                            word_p newnumber =
                                    rplNewSINT(newcount + 1, DECint32_t);
                            if(!newnumber) {
                                DSTop = endofstk + 1;
                                return 0;
                            }
                            *(stkptr - 1) = newnumber;
                        }

                        if((redargs > 1) || (reddenom > 1) || simplified)
                            changed = 1;
                    }
                    --stkptr;
                    continue;
                }

            }   // END OF MULTIPLICATION

            if(*sobj == (CMD_OVR_ADD)) {
                // SCAN ALL NUMERIC FACTORS AND ADD TOGETHER (INCLUDING FRACTIONS)

                int32_t nargs = OPCODE(**(stkptr - 1)) - 1;
                word_p *argptr = stkptr - 2;

                word_p *firstnum = 0, *secondnum = 0;

                for(f = 0; f < nargs; ++f) {
                    if(rplSymbIsFractionInStack(argptr)) {
                        if(!firstnum) {
                            firstnum = argptr;
                        }
                        else {
                            secondnum = argptr;
                            break;
                        }
                    }
                    argptr = rplSymbSkipInStack(argptr);

                }

                if((firstnum == 0) || (secondnum == 0)) {
                    --stkptr;
                    continue;
                }

                // HERE WE HAVE 2 FRACTIONS OR NUMBERS READY TO ADD

                rplSymbFractionExtractNumDen(firstnum);
                rplSymbFractionExtractNumDen(secondnum);

                // NOW COMPUTE THE RESULT

                int32_t isnegative = rplFractionAdd();

                if(Exceptions) {
                    rplBlameError(sobj);
                    DSTop = endofstk + 1;
                    return 0;
                }

                // AND REPLACE IT IN THE ORIGINAL

                // REMOVE ORIGINAL ARGUMENTS
                int32_t offset;
                offset = rplSymbRemoveInStack(firstnum);
                DSTop -= offset;
                stkptr -= offset;
                firstnum -= offset;
                offset = rplSymbRemoveInStack(secondnum);
                DSTop -= offset;
                stkptr -= offset;
                firstnum -= offset;

                // AND INSERT THE NEW ONE

                int32_t den_is_one = 0;

                // CHECK IF DENOMINATOR IS ONE
                if(ISint32_t(*rplPeekData(1))) {
                    int64_t denom = rplReadint32_t(rplPeekData(1));
                    if(denom == 1)
                        den_is_one = 1;
                }
                else {
                    if(ISREAL(*rplPeekData(1))) {
                        REAL number;
                        rplReadReal(rplPeekData(1), &number);
                        rplOneToRReg(0);
                        if(cmpReal(&number, &RReg[0]) == 0)
                            den_is_one = 1;
                    }
                }

                offset = rplSymbInsertInStack(firstnum,
                        1 + ((den_is_one) ? 0 : 5) + (isnegative ? 2 : 0));
                stkptr += offset;
                DSTop += offset;
                firstnum += offset;

                // HERE FIRSTNUM POINTS TO THE START OF THE HOLE WE JUST OPENED
                if(isnegative) {
                    if(den_is_one) {
                        firstnum[0] = (word_p) uminus_opcode;
                        firstnum[-1] = (word_p) two_bint;
                        firstnum[-2] = rplPeekData(2);
                    }
                    else {
                        firstnum[0] = (word_p) uminus_opcode;
                        firstnum[-1] = (word_p) two_bint;
                        firstnum[-2] = (word_p) mul_opcode;
                        firstnum[-3] = (word_p) three_bint;
                        firstnum[-4] = rplPeekData(2);
                        firstnum[-5] = (word_p) inverse_opcode;
                        firstnum[-6] = (word_p) two_bint;
                        firstnum[-7] = rplPeekData(1);
                    }

                }
                else {
                    if(den_is_one) {
                        *firstnum = rplPeekData(2);
                    }
                    else {
                        firstnum[0] = (word_p) mul_opcode;
                        firstnum[-1] = (word_p) three_bint;
                        firstnum[-2] = rplPeekData(2);
                        firstnum[-3] = (word_p) inverse_opcode;
                        firstnum[-4] = (word_p) two_bint;
                        firstnum[-5] = rplPeekData(1);
                    }
                }

                DSTop -= 2;

                // UPDATE THE ARGUMENT COUNT

                if(nargs - 1 < 2) {
                    // REMOVE THE ADDITION IF THERE'S ONLY ONE ARGUMENT
                    offset = rplSymbDeleteInStack(stkptr, 2);
                    DSTop -= offset;
                    stkptr -= offset;
                }
                else {
                    word_p newobj = rplNewSINT(nargs, DECint32_t);
                    if(!newobj)
                        return 0;
                    *(stkptr - 1) = newobj;
                    --stkptr;
                }

                changed = 1;
                continue;
            }

            if((*sobj != (CMD_OVR_MUL)) && (*sobj != (CMD_OVR_ADD))
                    && (*sobj != (CMD_OVR_INV)) && (*sobj != (CMD_OVR_UMINUS))
                    && (*sobj != (CMD_EQUATIONOPERATOR))) {
                // EXCEPT ADDITION AND MULTIPLICATIONS, CHECK IF ALL ARGUMENTS ARE NUMERIC AND APPLY THE OPERATOR

                int32_t nargs = OPCODE(**(stkptr - 1)) - 1;
                word_p *argptr = stkptr - 2, *savedstop;
                int32_t notanumber = 0, approxnumber = 0;
                for(f = 0; f < nargs; ++f) {
                    if(!ISNUMBERORUNIT(**argptr)) {
                        // CHECK IF IT'S A NEGATIVE NUMBER
                        if(**argptr == (CMD_OVR_UMINUS)) {
                            if(!ISNUMBERORUNIT(**(argptr - 2))) {
                                notanumber = 1;
                                break;
                            }
                            // CHECK IF THERE'S AN APPROXIMATE OR EXACT NUMBER
                            if(ISAPPROX(**(argptr - 2)))
                                ++approxnumber;
                        }
                        else {
                            notanumber = 1;
                            break;
                        }
                    }
                    else {
                        // CHECK IF IT'S AN APPROXIMATE OR EXACT NUMBER
                        if(ISAPPROX(**argptr))
                            ++approxnumber;
                    }
                    argptr = rplSymbSkipInStack(argptr);
                }

                if(notanumber) {
                    --stkptr;
                    continue;
                }

                savedstop = DSTop;

                // HERE ALL ARGUMENTS ARE SIMPLE NUMBERS, APPLY THE OPERATOR
                // WE ALSO KNOW IF THERE WERE ANY ARGUMENTS THAT WERE APPROXIMATED
                argptr = stkptr - 2;
                for(f = 0; f < nargs; ++f) {
                    if(ISNUMBERORUNIT(**argptr))
                        rplPushData(*argptr);
                    else {
                        // CHECK IF IT'S A NEGATIVE NUMBER
                        if(**argptr == (CMD_OVR_UMINUS)) {
                            // WE KNOW FROM PREVIOUS LOOP THAT A NUMBER FOLLOWS
                            rplPushData(*(argptr - 2));

                            // NEGATE THE NUMBER
                            Context.precdigits = REAL_PRECISION_MAX;

                            rplCallOvrOperator(OVR_NEG);
                            if(Exceptions) {
                                rplBlameError(sobj);
                                DSTop = endofstk + 1;
                                return 0;
                            }

                            Context.precdigits = origprec;

                        }
                    }
                    argptr = rplSymbSkipInStack(argptr);

                }

                // TRACK ANGLE CONVERSIONS
                rplClrSystemFlag(FL_FORCED_RAD);

                // SPECIAL CASE: FOR BRACKET OPERATORS WE NEED TO KEEP THEM AS SYMBOLIC
                if((**stkptr == CMD_OPENBRACKET)
                        || (**stkptr == CMD_LISTOPENBRACKET)
                        || (**stkptr == CMD_CLISTOPENBRACKET))
                    rplSymbApplyOperator(**stkptr, nargs);
                else
                    rplCallOperator(**stkptr);
                if(Exceptions) {
                    rplBlameError(*stkptr);
                    DSTop = endofstk + 1;
                    return 0;
                }

                if(rplTestSystemFlag(FL_FORCED_RAD)
                        || !ISNUMBERORUNIT(*rplPeekData(1))) {

                    // THIS IS STRANGE, A COMMAND WITH NUMERIC INPUT SHOULD RETURN A NUMERIC OUTPUT

                    // IT'S LIKELY A COMPLEX NUMBER, A VECTOR/MATRIX OR A LIST, OR SOME WEIRD ANGLE CONVERSION WITH LOSS OF ANGULAR UNIT
                    // IN ANY CASE, DON'T TRY TO SIMPLIFY

                    // TODO: IF THE RESULT IS SYMBOLIC, NEED TO EXPAND BEFORE INSERTING, SO ADDITIONAL SIMPLIFICATION CAN BE DONE INSIDE

                    word_p *ptr, *endofobj = rplSymbSkipInStack(stkptr);       // POINT TO THE NEXT OBJECT
                    ptr = endofobj + 1;
                    *ptr = rplPeekData(1);
                    --DSTop;
                    ++ptr;
                    ++stkptr;
                    // NOW CLOSE THE GAP
                    while(stkptr != DSTop) {
                        *ptr = *stkptr;
                        ++stkptr;
                        ++ptr;
                    }
                    DSTop = ptr;
                    stkptr = endofobj;
                    changed = 1;
                    continue;

                }
                else {
                    // IT'S A NUMBER, CHECK IF IT'S AN APPROXIMATED ANSWER
                    if((ISAPPROX(*rplPeekData(1))) && !approxnumber) {
                        // WE GAVE EXACT NUMBERS, CAN'T ACCEPT AN INEXACT ANSWER
                        DSTop = savedstop;      // CLEANUP THE STACK
                    }
                    else {
                        // REPLACE A SINGLE ARGUMENT

                        word_p *ptr, *endofobj = rplSymbSkipInStack(stkptr);   // POINT TO THE NEXT OBJECT
                        ptr = endofobj + 1;
                        *ptr = rplPeekData(1);
                        --DSTop;
                        ++ptr;
                        ++stkptr;
                        // NOW CLOSE THE GAP
                        while(stkptr != DSTop) {
                            *ptr = *stkptr;
                            ++stkptr;
                            ++ptr;
                        }
                        DSTop = ptr;
                        stkptr = endofobj;
                        changed = 1;
                        continue;

                    }

                }

            }
            --stkptr;
        }

    }

// ...

    if(Exceptions) {
        DSTop = endofstk + 1;
        return 0;
    }

    word_p finalsymb = rplSymbImplode(DSTop - 1);

    DSTop = endofstk + 1;
    if(Exceptions)
        return 0;

    return finalsymb;

}

// TAKES A SYMBOLIC FROM THE STACK AND:
// CHANGE THE SYMBOLIC TO CANONICAL FORM.
// ALL NUMERICAL TERMS ARE ADDED TOGETHER
// ALL NUMERICAL FACTORS IN THE NUMERATOR ARE MULTIPLIED TOGETHER
// ALL NUMERICAL FACTORS IN THE DENOMINATOR ARE MULTIPLIED TOGETHER
// SYMBOLIC FRACTIONS ARE REDUCED

void rplSymbAutoSimplify()
{

    word_p newobj = rplSymbCanonicalForm(rplPeekData(1), 0);
    if(newobj)
        rplOverwriteData(1, newobj);
    else
        return;

    newobj = rplSymbNumericReduce(rplPeekData(1));

    if(newobj)
        rplOverwriteData(1, newobj);
    return;
}

// RETURN TRUE/FALSE IF THE GIVEN SYMBOLIC IS A RULE

int32_t rplSymbIsRule(word_p ptr)
{
    if(!ISSYMBOLIC(*ptr))
        return 0;
    if(rplSymbMainOperator(ptr) == CMD_RULESEPARATOR)
        return 1;
    return 0;
}

// RETURN 1 IF THE GIVEN SYMBOLIC IS ONLY NUMERIC
// RETURN 0 IF THE GIVEN SYMBOLIC HAS ANY VARIABLES OR CALLS A CUSTOM USER FUNCTION
int32_t rplSymbIsNumeric(word_p ptr)
{
    word_p endofobj = rplSkipOb(ptr);

    if(ISNUMBERCPLX(*ptr))
        return 1;

    while(ptr < endofobj) {
        if(ISSYMBOLIC(*ptr))
            ptr = rplSymbUnwrap(ptr) + 1;
        if(ISIDENT(*ptr))
            return 0;
        if(*ptr == CMD_OVR_FUNCEVAL)
            return 0;

        ptr = rplSkipOb(ptr);

    }

    return 1;

}

// RETURN 1 IF THE GIVEN SYMBOLIC IS ZERO, WITHOUT EVALUATING ANY VARIABLES
// RETURN 0 IF THE GIVEN SYMBOLIC HAS ANY VARIABLES, CALLS A CUSTOM USER FUNCTION OR HAS ANY NON-ATOMIC OPERATION
// IF THE OBJECT IS NUMERIC, ALSO CHECK FOR ZERO

int32_t rplSymbIsZero(word_p ptr)
{
    word_p obj = ptr, end = rplSkipOb(obj);
    int32_t onezero = 0, allzeros = 1, optype = 0;
    if(ISSYMBOLIC(*obj))
        ++obj;
    while(obj != end) {
        if(!(ISPROLOG(*obj) || ISint32_t(*obj) || ISCONSTANT(*obj))) {
            // SOME KIND OF OPERATION, THE ONLY THING ALLOWED IS UNARY PLUS OR MINUS BEFORE THE NUMBER ZERO
            switch (*obj) {
            case CMD_OVR_UMINUS:
            case CMD_OVR_UPLUS:
            case CMD_OVR_POW:
                return rplSymbIsZero(obj + 1);

            case CMD_OVR_MUL:
                obj++;
                break;
            case CMD_OVR_ADD:
                obj++;
                optype = 1;
                break;
            case CMD_OVR_INV:
                obj++;
                if(ISREAL(*obj)) {
                    REAL n;
                    rplReadReal(obj, &n);
                    if(isinfiniteReal(&n) || isundinfiniteReal(&n))
                        return 1;
                }
                return 0;
            default:
                return 0;       // ALL OTHER COMMANDS AND OPERATORS ARE NOT KNOWN TO BE ZERO UNTIL EVALUATED
            }
            continue;
        }
        // HERE WE HAVE AN OBJECT
        if(ISSYMBOLIC(*obj)) {
            if(rplSymbIsZero(obj))
                onezero = 1;
            else
                allzeros = 0;
        }
        else {
            if(rplIsNumberZero(obj))
                onezero = 1;
            else
                allzeros = 0;
        }
        obj = rplSkipOb(obj);
    }

    if(optype == 0) {
        if(onezero)
            return 1;
        return 0;
    }
    if(allzeros)
        return 1;
    return 0;
}

// FULLY COMPUTE A NUMERIC SYMBOLIC, DOES ->NUM ATOMICALLY

void rplSymbNumericCompute()
{
    word_p *stksave = DSTop;
    word_p *ptr = DSTop - 1;
    int32_t endoffset = rplObjSize(*ptr), partialoff = endoffset;
    WORD opcode = 0;
    int32_t offset = 0;

    while(offset < endoffset) {

        if(offset == partialoff) {
            if(opcode)
                rplCallOperator(opcode);        // THIS BETTER BE ATOMIC OR IT WILL CRASH BADLY
            if(Exceptions) {
                DSTop = stksave;
                return;
            }
            opcode = 0;
        }

        if(ISSYMBOLIC(*(*ptr + offset))) {
            partialoff = offset + rplObjSize(*ptr + offset);
            offset = (rplSymbUnwrap(*ptr + offset) - *ptr) + 1;
        }

        if(ISNUMBERCPLX(*(*ptr + offset))) {
            rplPushData(rplConstant2Number(*ptr + offset));
        }
        else if(ISIDENT(*(*ptr + offset))) {
            DSTop = stksave;
            return;
        }
        else if(!ISPROLOG(*(*ptr + offset))) {
            if(*(*ptr + offset) == CMD_OVR_FUNCEVAL) {
                DSTop = stksave;
                return;
            }
            opcode = *(*ptr + offset);
        }

        offset += rplObjSize((*ptr + offset));

    }

    if(offset == partialoff) {
        if(opcode)
            rplCallOperator(opcode);    // THIS BETTER BE ATOMIC OR IT WILL CRASH BADLY
        if(Exceptions) {
            DSTop = stksave;
            return;
        }
    }

    int32_t nitems = DSTop - stksave;
    DSTop = stksave;
    if(nitems > 1)
        return;
    rplOverwriteData(1, *stksave);

}

// ********************************************************************************************************************
// START OF NEW AND IMPROVED RULE ALGORITHM FROM SCRATCH
// ********************************************************************************************************************

// EXPLODE A SYMBOLIC IN THE STACK
// BUT ONLY EXPLODE THE OUTERMOST OPERATOR, KEEPING ITS ARGUMENTS UNEXPLODED
// USES ScratchPointer1 THRU 3 FOR GC PROTECTION
// RETURN THE NUMBER OF OBJECTS THAT ARE ON THE STACK
// LEVEL 1=OPERATOR, LEVEL2=NARGS, LEVEL3=LAST ARG ... LEVEL 2+NARGS = 1ST ARG

int32_t rplSymbExplodeOneLevel2(word_p object)
{
    int32_t count = 0, countops = 0;

    if(ISSYMBOLIC(*object))
        ScratchPointer1 = rplSymbUnwrap(object) + 1;
    else
        ScratchPointer1 = object;
    ScratchPointer2 = rplSkipOb(object);

    while(ScratchPointer1 < ScratchPointer2) {
        if(!(ISPROLOG(*ScratchPointer1) || ISint32_t(*ScratchPointer1)
                    || ISCONSTANT(*ScratchPointer1))) {
            ScratchPointer3 = ScratchPointer1;
            ++countops;
        }
        else {
            rplPushData(ScratchPointer1);
            ++count;
        }
        ScratchPointer1 = rplSkipOb(ScratchPointer1);
    }

    if(countops) {
        rplNewSINTPush(count, DECint32_t);
        rplPushData(ScratchPointer3);
        ++countops;
    }
    return count + countops;

}

// RETURN 1 IF THE GIVEN SYMBOLIC IS A SINGLE NUMBER
// RETURN 0 IF THE GIVEN SYMBOLIC HAS ANY VARIABLES OR CALLS A CUSTOM USER FUNCTION
int32_t rplSymbIsANumber(word_p ptr)
{
    if(ISSYMBOLIC(*ptr))
        ptr = rplSymbUnwrap(ptr) + 1;

    if(ISNUMBERCPLX(*ptr))
        return 1;

    return 0;

}

int32_t rplSymbIsIntegerNumber(word_p ptr)
{
    if(ISSYMBOLIC(*ptr))
        ptr = rplSymbUnwrap(ptr) + 1;

    if(ISint32_t(*ptr))
        return 1;
    if(ISREAL(*ptr)) {
        REAL n;
        rplReadNumberAsReal(ptr, &n);
        if(isintegerReal(&n))
            return 1;
    }

    return 0;

}

int32_t rplSymbIsOddNumber(word_p ptr)
{
    if(ISSYMBOLIC(*ptr))
        ptr = rplSymbUnwrap(ptr) + 1;

    if(ISint32_t(*ptr)) {
        int64_t n;
        n = rplReadNumberAsInt64(ptr);
        if(n & 1)
            return 1;
        return 0;
    }
    if(ISREAL(*ptr)) {
        REAL n;
        rplReadNumberAsReal(ptr, &n);

        if(isintegerReal(&n) && isoddReal(&n))
            return 1;
    }

    return 0;

}

enum
{
    OPMATCH = 0,
    ARGMATCH = 1,
    RESTARTMATCH = 2,
    BACKTRACK = 3,
    ARGDONE = 4,
    ARGDONEEXTRA = 5
};

#define FINDARGUMENT(exp,nargs,argidx) (exp[-2-(nargs)+(argidx)])

typedef struct
{
    word_p *left, *right;
    int32_t leftnargs, rightnargs;
    int32_t leftidx, rightidx, leftrot, lrotbase;
    int32_t leftdepth, rightdepth;
    int32_t nlams;
} TRACK_STATE;

// NO ARGUMENT CHECKS, INTERNAL USE ONLY, ptr MUST NOT BE NULL, stkbase MUST POINT AFTER VALID DATA ON THE STACK

// EXPECTED STACK FORMAT:
// LARGN ... LARG1 LNARGS LOP RARGN ... RARG1 RNARGS ROP LIDX RIDX LROT LAMN_VALUE LAMN_NAME ... LAM1_VALUE LAM1_NAME NLAMS
// INPUT/OUTPUT:     left->|                   right->|                                                               stkbase->|

static void reloadPointers(word_p * stkbase, TRACK_STATE * ptr)
{
    ptr->nlams = rplReadint32_t(*(stkbase - 1));
    ptr->right = stkbase - 2 * ptr->nlams - 5;  // POINT TO THE RIGHT OPERATOR
    ptr->left = ptr->right - 1;
    if(!ISPROLOG(**(ptr->right)) && !ISint32_t(**(ptr->right))
            && !ISCONSTANT(**(ptr->right))) {
        ptr->rightnargs = rplReadint32_t(*((ptr->right) - 1));
        ptr->left--;
    }
    else
        ptr->rightnargs = 0;
    ptr->left -= ptr->rightnargs;
    if(!ISPROLOG(**(ptr->left)) && !ISint32_t(**(ptr->left))
            && !ISCONSTANT(**(ptr->right)))
        ptr->leftnargs = rplReadint32_t(*((ptr->left) - 1));
    else
        ptr->leftnargs = 0;

    {
        int64_t tmpint = rplReadint32_t(ptr->right[1]);
        ptr->leftidx = (int32_t) tmpint;
        ptr->leftdepth = (int32_t) (tmpint >> 32);
    }
    {
        int64_t tmpint = rplReadint32_t(ptr->right[2]);
        ptr->rightidx = (int32_t) tmpint;
        ptr->rightdepth = (int32_t) (tmpint >> 32);
    }
    {
        int64_t tmpint = rplReadint32_t(ptr->right[3]);
        ptr->leftrot = (int32_t) tmpint;
        ptr->lrotbase = (int32_t) (tmpint >> 32);
    }

}

// REPLACE ALL LAMS IN CURRENT ENVIRONMENT WITH THE ONES FROM THE STACK
static void reloadLAMs(word_p * orgrule, TRACK_STATE * ptr)
{
    rplCleanupLAMs(0);
    rplCreateLAMEnvironment(*orgrule);
    if(Exceptions)
        return;
    rplCreateLAM((word_p) nulllam_ident, (word_p) zero_bint);
    int32_t oldnlams = rplLAMCount(0) - 1;
    while(oldnlams < ptr->nlams) {
        rplCreateLAM((word_p) nulllam_ident, (word_p) zero_bint);
        if(Exceptions)
            return;
        ++oldnlams;
    }
    // COPY ALL LAM DEFINITIONS
    memmovew(nLAMBase + 4, ptr->right + 4,
            ptr->nlams * 2 * sizeof(word_p) / sizeof(WORD));
    LAMTop = nLAMBase + 4 + 2 * ptr->nlams;
}

// UPDATE ONLY NUMERIC COUNTERS, EXCEPT nlams
// ALL POINTERS IN ptr MUST BE VALID
static void updateCounters(TRACK_STATE * ptr)
{
    ptr->right[1] =
            rplNewint32_t((((int64_t) ptr->leftdepth) << 32) + ptr->leftidx,
            DECint32_t);
    if(Exceptions)
        return;
    ptr->right[2] =
            rplNewint32_t((((int64_t) ptr->rightdepth) << 32) + ptr->rightidx,
            DECint32_t);
    if(Exceptions)
        return;
    ptr->right[3] =
            rplNewint32_t((((int64_t) ptr->lrotbase) << 32) + ptr->leftrot,
            DECint32_t);
    if(Exceptions)
        return;
}

// SAVE CURRENT LAM ENVIRONMENT TO THE STACK
static void updateLAMs(TRACK_STATE * ptr)
{
    int32_t newnlams = rplLAMCount(0) - 1;
    if(ptr->nlams < newnlams) {
        rplExpandStack(2 * (newnlams - ptr->nlams));
        if(Exceptions)
            return;
    }
    if(ptr->right + 5 + 2 * ptr->nlams != DSTop) {
        // THIS UPDATE IS HAPPENING IN THE MIDDLE OF THE STACK, NOT AT THE END
        // MAKE A HOLE IN THE STACK
        memmovew(ptr->right + 5 + newnlams, ptr->right + 5 + 2 * ptr->nlams,
                (DSTop - (ptr->right + 5 +
                        2 * ptr->nlams)) * sizeof(word_p) / sizeof(WORD));
    }

    if(newnlams >= 0) {
        // STORE ALL LAMS
        memmovew(ptr->right + 4, nLAMBase + 4,
                newnlams * 2 * sizeof(word_p) / sizeof(WORD));
        ptr->right[4 + 2 * newnlams] = rplNewSINT(newnlams, DECint32_t);
        if(Exceptions)
            return;
        DSTop += 2 * (newnlams - ptr->nlams);
        ptr->nlams = newnlams;
    }

}

// REPLACES THE RIGHT PART OF THE RULE AT THE CURRENT LOCATION
// ONLY THE ARGUMENTS (THE OPERATORS ARE ASSUMED TO MATCH)

void rplSymbReplaceMatchHere(word_p * rule, int32_t startleftarg)
{
    word_p ruleleft = rplSymbMainOperatorPTR(*rule);
    if(*ruleleft == CMD_RULESEPARATOR)
        ++ruleleft;     // POINT TO THE FIRST ARGUMENT, OTHERWISE THE ENTIRE SYMBOLIC IS AN EXPRESSION TO MATCH BUT NOT A RULE TO REPLACE
    else
        return; // NOTHING TO REPLACE IF NOT A RULE
    word_p ruleright = rplSkipOb(ruleleft);
    word_p *stksave = DSTop;
    TRACK_STATE s;

    reloadPointers(DSTop, &s);

    // GET EXPRESSION TO REPLACE WITH
    word_p *expstart = rplSymbExplodeCanonicalForm(ruleright, 0);
    if(Exceptions) {
        DSTop = stksave;
        return;
    }
    word_p *lamenv = rplGetNextLAMEnv(LAMTop);
    if(!lamenv) {
        DSTop = stksave;
        return;
    }
    int32_t nlams = rplLAMCount(lamenv), k;

    // REPLACE ALL LAMS WITH THEIR VALUES
    while(expstart >= stksave) {
        if(ISIDENT(**expstart)) {
            // FIND MATCHING LAM AND REPLACE
            for(k = 1; k <= nlams; ++k) {
                if(rplCompareIDENT(*expstart, *rplGetLAMnNameEnv(lamenv, k))) {
                    *expstart = *rplGetLAMnEnv(lamenv, k);
                }
            }
        }
        --expstart;
    }

    word_p newsymb = rplSymbImplode(DSTop - 1);
    if(!newsymb) {
        DSTop = stksave;
        return;
    }
    // HERE WE FINALLY HAVE THE RIGHT PART EXPRESSION READY TO REPLACE
    DSTop = stksave;
    if(s.leftnargs) {
        if((s.rightnargs || (s.leftdepth > s.rightdepth))
                && (s.leftnargs != s.rightnargs)) {
            // THERE'S EXTRA ARGUMENTS, MUST BE ADDITION OR MULTIPLICATION, JUST REPLACE ARGUMENTS
            if((s.leftdepth > s.rightdepth) && startleftarg > 1)
                --startleftarg; // CORRECT FOR BADLY UPDATED ARGUMENTS WHEN

            FINDARGUMENT(s.left, s.leftnargs, startleftarg) = newsymb;
            if(s.rightnargs > 1) {
                rplRemoveAtData(DSTop - &FINDARGUMENT(s.left, s.leftnargs,
                            startleftarg + s.rightnargs) + 1, s.rightnargs - 1);
                s.leftnargs -= s.rightnargs - 1;
                s.left -= s.rightnargs - 1;
                s.left[-1] = rplNewSINT(s.leftnargs, DECint32_t);
            }
        }

        else {
            // SINCE ALL ARGUMENTS ARE REPLACED, ALSO REPLACE THE OPERATOR
            *s.left = newsymb;
            rplRemoveAtData(DSTop - &FINDARGUMENT(s.left, s.leftnargs,
                        1 + s.leftnargs), s.leftnargs + 1);
            s.left -= s.leftnargs + 1;
            s.leftidx = s.leftnargs = 0;

        }
    }
    else
        *s.left = newsymb;

    // REPLACEMENT WAS PERFORMED AT THIS LEVEL, NOW PROPAGATE UPSTREAM

    word_p *DSBase = DSTop;

    while(s.leftidx >= 0) {
        if(s.lrotbase) {
            // PASS THE ARGUMENTS TO THE PARENT EXPRESSION
            TRACK_STATE p;
            reloadPointers(s.left - ((s.leftnargs) ? (1 + s.leftnargs) : 0),
                    &p);
            FINDARGUMENT(p.left, p.leftnargs,
                    (s.leftidx + s.leftrot >
                        p.leftnargs) ? s.leftidx + s.leftrot -
                    p.leftnargs : s.leftidx + s.leftrot) =
                    FINDARGUMENT(s.left, s.leftnargs, s.leftidx);
            DSBase = s.left - ((s.leftnargs) ? (1 + s.leftnargs) : 0);
            reloadPointers(DSBase, &s);
        }
        else {
            if(s.leftnargs) {
                // ASSEMBLE THE NEW OBJECT
                rplExpandStack(s.leftnargs);
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }
                for(k = 1; k <= s.leftnargs; ++k)
                    *DSTop++ = FINDARGUMENT(s.left, s.leftnargs, k);
                rplSymbApplyOperator(**s.left, s.leftnargs);
            }
            else {
                rplPushData(*s.left);
            }
            if(Exceptions) {
                DSTop = stksave;
                return;
            }

            // HERE WE HAVE THE UPDATED OBJECT TO BE REPLACED UPSTREAM

            DSBase = s.left - ((s.leftnargs) ? (1 + s.leftnargs) : 0);
            reloadPointers(DSBase, &s);

            if(s.leftidx >= 0) {
                // REPLACE THE LEFT ARGUMENT WITH THE NEW OBJECT
                if(s.leftnargs) {
                    if(s.leftidx > s.leftnargs)
                        FINDARGUMENT(s.left, s.leftnargs,
                                s.leftidx - s.leftnargs) = rplPopData();
                    else
                        FINDARGUMENT(s.left, s.leftnargs, s.leftidx) =
                                rplPopData();
                }
                else
                    *s.left = rplPopData();

            }
            else {
                // REPLACE THE ORIGINAL EXPRESSION IN WHOLE
                s.right[-1] = rplPopData();

            }

        }
    }

}

// MATCH A RULE AGAINST AN EXPRESSION
// ARGUMENTS:
// SYMBOLIC EXPRESSION IN LEVEL 2
// RULE IN LEVEL 1

// RETURNS:
// NUMBER OF MATCHES FOUND = NUMBER OF NEW LOCAL ENVIRONMENTS CREATED (NEED TO BE CLEANED UP BY CALLER)
// EACH MATCH FOUND CREATES A NEW LOCAL ENVIRONMENT, WITH THE FOLLOWING VARIABLES:
// GETLAM1 IS UNNAMED, AND WILL CONTAIN A POINTER INSIDE THE ORIGINAL SYMBOLIC WHERE THE MATCH WAS FOUND, TO BE USED BY REPLACE (INTERNAL USE)
// ANY IDENTS THAT START WITH A PERIOD IN THE RULE DEFINITION WILL HAVE A DEFINITION IN THIS ENVIRONMENT

// STACK ON RETURN: ORIGINAL ARGUMENTS UNTOUCHED + THE RESULTING EXPRESSION AFTER APPLYING THE RULE
// MAY TRIGGER GC
// USES ALL SCRATCHPOINTERS
// CALLER NEEDS TO CLEANUP ALL LOCAL ENVIRONMENTS. LOCAL ENVIRONMENTS ARE OWNED BY THE RULE ARGUMENT.

// NO ARGUMENT CHECKS!

// NEW VERSION 3 IMPLEMENTATION

int32_t rplSymbRuleMatch()
{
    // MAKE SURE BOTH EXPRESSION AND RULE ARE IN CANONIC FORM
    word_p newexp = rplSymbCanonicalForm(rplPeekData(2), 0);
    if(!newexp || Exceptions) {
        return 0;
    }
    rplPushDataNoGrow(newexp);
    newexp = rplSymbCanonicalForm(rplPeekData(2), 0);
    if(!newexp || Exceptions) {
        return 0;
    }
    rplPushDataNoGrow(newexp);

    // ******************************************************
    // DEBUG ONLY AREA
    // ******************************************************
#ifdef RULEDEBUG
    printf("START RULE MATCH 3: ");

    word_p string = rplDecompile(DSTop[-2], DECOMP_EDIT | DECOMP_NOHINTS);
    if(string) {
        BYTE strbyte[1024];
        memmoveb(strbyte, (byte_p) (string + 1), rplStrSize(string));
        strbyte[rplStrSize(string)] = 0;
        printf("exp=%s", strbyte);
    }
    string = rplDecompile(DSTop[-1], DECOMP_EDIT | DECOMP_NOHINTS);
    if(string) {
        BYTE strbyte[1024];
        memmoveb(strbyte, (byte_p) (string + 1), rplStrSize(string));
        strbyte[rplStrSize(string)] = 0;
        printf("  rule=%s", strbyte);
    }
    printf("\n");
    fflush(stdout);
#endif

    word_p *expression = DSTop - 2;
    word_p *rule = DSTop - 1;
    word_p *orgrule = DSTop - 3;
    word_p *marker;
    word_p *lamsave = LAMTop, *lamcurrent = nLAMBase, *stkbottom =
            DStkBottom, *basestkbottom;

    word_p ruleleft = rplSymbMainOperatorPTR(*rule);
    if(*ruleleft == CMD_RULESEPARATOR)
        ++ruleleft;     // POINT TO THE FIRST ARGUMENT, OTHERWISE THE ENTIRE SYMBOLIC IS AN EXPRESSION TO MATCH BUT NOT A RULE TO REPLACE
    int32_t ruleleftoffset = ruleleft - *rule;
    int32_t matchtype, matchstarted;
    int32_t baselevel;

    TRACK_STATE s;

    rplTakeSnapshotN(2);
    if(Exceptions) {
        rplCleanupSnapshots(stkbottom);
        DSTop = expression;
        LAMTop = lamsave;
        nLAMBase = lamcurrent;
        return 0;
    }

    basestkbottom = DStkBottom;

// PUSH LOOP STOPPERS
    rplPushData((word_p) minusone_bint);
    if(Exceptions) {
        rplCleanupSnapshots(stkbottom);
        DSTop = expression;
        LAMTop = lamsave;
        nLAMBase = lamcurrent;
        return 0;
    }
    rplPushData((word_p) minusone_bint);
    if(Exceptions) {
        rplCleanupSnapshots(stkbottom);
        DSTop = expression;
        LAMTop = lamsave;
        nLAMBase = lamcurrent;
        return 0;
    }
    rplPushData((word_p) minusone_bint);
    if(Exceptions) {
        rplCleanupSnapshots(stkbottom);
        DSTop = expression;
        LAMTop = lamsave;
        nLAMBase = lamcurrent;
        return 0;
    }
    rplPushData((word_p) zero_bint);
    if(Exceptions) {
        rplCleanupSnapshots(stkbottom);
        DSTop = expression;
        LAMTop = lamsave;
        nLAMBase = lamcurrent;
        return 0;
    }

// PUSH LOOP INITIAL ARGUMENTS
// PUSH THE LEFT
    marker = DSTop;
    rplSymbExplodeOneLevel2(*expression);
    if(Exceptions) {
        rplCleanupSnapshots(stkbottom);
        DSTop = expression;
        LAMTop = lamsave;
        nLAMBase = lamcurrent;
        return 0;
    }
    s.left = DSTop - 1;
// PUSH THE RIGHT
    rplSymbExplodeOneLevel2(*rule + ruleleftoffset);
    if(Exceptions) {
        rplCleanupSnapshots(stkbottom);
        DSTop = expression;
        LAMTop = lamsave;
        nLAMBase = lamcurrent;
        return 0;
    }
    s.right = DSTop - 1;

// LEFTIDX AND RIGHTIDX PLUS LEFTDEPTH AND RIGHTDEPTH
    rplPushData((word_p) zero_bint);
    if(Exceptions) {
        rplCleanupSnapshots(stkbottom);
        DSTop = expression;
        LAMTop = lamsave;
        nLAMBase = lamcurrent;
        return 0;
    }
    rplPushData((word_p) zero_bint);
    if(Exceptions) {
        rplCleanupSnapshots(stkbottom);
        DSTop = expression;
        LAMTop = lamsave;
        nLAMBase = lamcurrent;
        return 0;
    }
// LROTBASE AND LEFTROT
    rplPushData((word_p) zero_bint);
    if(Exceptions) {
        rplCleanupSnapshots(stkbottom);
        DSTop = expression;
        LAMTop = lamsave;
        nLAMBase = lamcurrent;
        return 0;
    }
// NLAMS
    rplPushData((word_p) zero_bint);
    if(Exceptions) {
        rplCleanupSnapshots(stkbottom);
        DSTop = expression;
        LAMTop = lamsave;
        nLAMBase = lamcurrent;
        return 0;
    }

    baselevel = DSTop - DStkBottom;
    matchtype = OPMATCH;
    matchstarted = 0;

// CREATE LAM ENVIRONMENT
    rplCreateLAMEnvironment(*orgrule);
    if(Exceptions) {
        rplCleanupSnapshots(stkbottom);
        DSTop = expression;
        LAMTop = lamsave;
        nLAMBase = lamcurrent;
        return 0;
    }
    rplCreateLAM((word_p) nulllam_ident, (word_p) zero_bint);
    if(Exceptions) {
        rplCleanupSnapshots(stkbottom);
        DSTop = expression;
        LAMTop = lamsave;
        nLAMBase = lamcurrent;
        return 0;
    }

    do {

        switch (matchtype) {
        case OPMATCH:
        {
            // GET POINTERS TO THE LEFT AND RIGHT EXPRESSIONS, AND ARGUMENT COUNTS
            reloadPointers(DSTop, &s);

            // ******************************************************
            // DEBUG ONLY AREA
            // ******************************************************
#ifdef RULEDEBUG
            printf("OPMATCH: [%d,%d] ", s.leftdepth, s.rightdepth);

            word_p string =
                    rplDecompile(*s.left, DECOMP_EDIT | DECOMP_NOHINTS);
            if(string) {
                BYTE strbyte[1024];
                memmoveb(strbyte, (byte_p) (string + 1), rplStrSize(string));
                strbyte[rplStrSize(string)] = 0;
                printf("left=%s", strbyte);
            }
            string = rplDecompile(*s.right, DECOMP_EDIT | DECOMP_NOHINTS);
            if(string) {
                BYTE strbyte[1024];
                memmoveb(strbyte, (byte_p) (string + 1), rplStrSize(string));
                strbyte[rplStrSize(string)] = 0;
                printf("  right=%s", strbyte);
            }
            printf("\n");
            fflush(stdout);
#endif
            // ******************************************************
            //      END DEBUG ONLY AREA
            // ******************************************************

            // OPERATOR COMPARISON, CHECK IF OPERATORS ARE IDENTICAL
            if(s.rightnargs) {
                // THE RIGHT PART HAS AN OPERATOR, CHECK IF THE LEFT IS IDENTICAL
                if(s.leftnargs && ((**s.right) == (**s.left))) {
                    // OPERATOR MATCHES, MOVE ON TO ARGUMENT BY ARGUMENT
                    s.leftidx = s.rightidx = 1;
                    if((**s.left == CMD_OVR_ADD) || (**s.right == CMD_OVR_MUL)) {
                        // KEEP PARENT UP TO DATE IN THE STACK
                        updateCounters(&s);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }
                        updateLAMs(&s);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }

                        // CREATE A COPY OF THE ENTIRE LEVEL TO DO A SUB-ROTATION
                        word_p *topoflevel =
                                s.left - ((s.leftnargs) ? (1 +
                                    s.leftnargs) : 0);
                        rplExpandStack(DSTop - topoflevel);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }
                        int32_t k;
                        for(k = 0; k < DSTop - topoflevel; ++k)
                            DSTop[k] = topoflevel[k];
                        s.left += k;
                        s.right += k;
                        DSTop += k;
                        s.leftrot = 0;
                        s.lrotbase = s.leftidx;

                        updateCounters(&s);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }

                        rplTakeSnapshot();
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }

                    }
                    else {
                        s.leftrot = s.lrotbase = 0;
                        updateCounters(&s);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }
                    }
                    matchtype = ARGMATCH;
                    matchstarted = 1;
                    break;
                }
                // NO MATCH, TRY PASSING IT TO THE ARGUMENTS
                if((DSTop - DStkBottom == baselevel) && s.leftnargs
                        && !matchstarted) {
                    // RECURSE INTO FIRST ARGUMENT
                    // PUSH THE LEFT
                    ++s.leftidx;

                    if(s.leftidx <= s.leftnargs) {
                        updateCounters(&s);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }

                        rplSymbExplodeOneLevel2(FINDARGUMENT(s.left,
                                    s.leftnargs, s.leftidx));
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }
                        // PUSH THE RIGHT
                        rplSymbExplodeOneLevel2(*rule + ruleleftoffset);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }
                        s.right = DSTop - 1;
                        s.leftdepth++;  // INCREASE THE DEPTH OF THE LEFT ARGUMENT
                        s.leftidx = s.rightidx = 0;
                        s.lrotbase = s.leftrot = 0;
                        // LEFTARG AND RIGHTARG
                        rplPushData((word_p) zero_bint);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }
                        rplPushData((word_p) zero_bint);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }
                        rplPushData((word_p) zero_bint);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }
                        rplPushData((word_p) zero_bint);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }
                        updateLAMs(&s);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }

                        baselevel = DSTop - DStkBottom;
                        matchtype = OPMATCH;
                    }
                    else
                        matchtype = BACKTRACK;

                }
                else
                    matchtype = BACKTRACK;
            }
            else {
                // RIGHT PART DOESN'T HAVE AN OPERATOR, CHECK FOR SPECIAL IDENTS
                if(ISIDENT(**s.right)) {
                    // IF THE RIGHT EXPRESSION IS A SINGLE IDENTIFIER
                    word_p *lamname = rplFindLAM(*s.right, 0);
                    if(lamname) {
                        WORD firstchars = (*s.right)[1];
                        firstchars &= 0xffff;

                        switch (firstchars) {
                        case TEXT2WORD('.', 'm', 0, 0):
                        case TEXT2WORD('.', 'M', 0, 0):
                        {
                            // GET POINTERS TO THE PARENT EXPRESSION
                            TRACK_STATE p;
                            reloadPointers(s.left - ((s.leftnargs) ? (1 +
                                            s.leftnargs) : 0), &p);

                            if(p.lrotbase && (p.leftrot > 0)
                                    && (**p.left == CMD_OVR_MUL)) {
                                // DO NOT ACCEPT ANY MATCH THAT HAS ANY ROTATION WHEN MULTIPLICATION IS ACTIVE
                                matchtype = BACKTRACK;
                                break;
                            }
                        }
                            // DELIBERATE FALL-THROUGH TO THE GENERAL CASE
                        default:

                            // REPLACE THE ENTIRE right PART WITH ITS VALUE, EXPLODED
                            DSTop = s.left + 1;
                            // PUSH THE RIGHT
                            rplSymbExplodeOneLevel2(lamname[1]);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }
                            s.right = DSTop - 1;

                            s.leftidx = s.rightidx = 0;
                            s.leftrot = s.lrotbase = 0;
                            s.nlams = 0;
                            // PUSH NEW INDEX
                            rplNewint32_tPush(((int64_t) s.leftdepth) << 32,
                                    DECint32_t);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }
                            rplNewint32_tPush(((int64_t) s.rightdepth) << 32,
                                    DECint32_t);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }
                            rplPushData((word_p) zero_bint);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }
                            rplPushData((word_p) zero_bint);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }
                            updateLAMs(&s);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }

                            break;
                        }

                    }
                    else {

                        WORD firstchars = (*s.right)[1];
                        firstchars &= 0xffff;

                        switch (firstchars) {
                        case TEXT2WORD('.', 'x', 0, 0):
                            // x = Match smallest expression with any variables or constants, assuming commutative addition and multiplication
                        {
                            if(s.leftnargs) {
                                // ASSIGN THE WHOLE EXPRESSION FROM THE PARENT TREE

                                // GET POINTERS TO THE PARENT EXPRESSION
                                TRACK_STATE p;
                                reloadPointers(s.left - ((s.leftnargs) ? (1 +
                                                s.leftnargs) : 0), &p);

                                int32_t attr = rplGetIdentAttr(*s.right);

                                if(attr) {
                                    // MATCH THE ATTRIBUTES
                                    int32_t otherattr;

                                    if(p.leftidx >= 1
                                            && p.leftidx <= p.leftnargs)
                                        otherattr =
                                                rplSymbGetAttr(FINDARGUMENT(p.
                                                    left, p.leftnargs,
                                                    p.leftidx));
                                    else if(p.leftidx == -1)
                                        otherattr = rplSymbGetAttr(*p.left);
                                    else
                                        otherattr = 0;
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }
                                    if(((attr | IDATTR_ALLTYPES) & otherattr) !=
                                            attr) {
                                        // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                }

                                // JUST CAPTURE THE CURRENT ARGUMENT
                                if(p.leftidx >= 1 && p.leftidx <= p.leftnargs)
                                    rplCreateLAM(*s.right, FINDARGUMENT(p.left,
                                                p.leftnargs, p.leftidx));
                                else if(p.leftidx == -1)
                                    rplCreateLAM(*s.right, *p.left);
                                // else SOMETHING IS WRONG IN THE EXPRESSION.

                                s.leftidx = s.leftnargs;
                                updateCounters(&s);

                            }
                            else {
                                int32_t attr = rplGetIdentAttr(*s.right);

                                if(attr) {
                                    // MATCH THE ATTRIBUTES
                                    int32_t otherattr;

                                    otherattr = rplSymbGetAttr(*s.left);
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    if(((attr | IDATTR_ALLTYPES) & otherattr) !=
                                            attr) {
                                        // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                }

                                rplCreateLAM(*s.right, *s.left);

                            }
                            matchtype = ARGDONE;
                            if(baselevel == DSTop - DStkBottom) {
                                // IF THIS IS THE BASE LEVEL, WE NEED TO UPDATE IT WITH THE NEW LAMS
                                baselevel += 2;
                            }
                            updateLAMs(&s);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }

                            break;
                        }
                        case TEXT2WORD('.', 'X', 0, 0):
                            // X = Match largest expression with any variables or constants, assuming commutative addition and multiplication
                            // TAKE ALL ARGUMENTS OF THE OPERATION
                        {

                            // GET POINTERS TO THE PARENT EXPRESSION
                            TRACK_STATE p;
                            reloadPointers(s.left - ((s.leftnargs) ? (1 +
                                            s.leftnargs) : 0), &p);

                            /*if((**p.left==CMD_OVR_ADD)||(**p.left==CMD_OVR_MUL))  // IT'S A COMMUTATIVE/ASSOCIATIVE OPERATOR
                               {
                               int32_t k;

                               if(p.rightidx!=p.rightnargs) { // THE SPECIAL IDENT IS NOT THE LAST ARGUMENT

                               word_p tmp=FINDARGUMENT(p.right,p.rightnargs,p.rightnargs);

                               if(ISIDENT(*tmp)) {
                               // CHECK IF THIS IS ANOTHER SPECIAL IDENT AND BREAK THE INFINITE LOOP
                               if( (((tmp[1])&0xffff) == TEXT2WORD('.','X',0,0))||(((tmp[1])&0xffff) == TEXT2WORD('.','M',0,0))) {
                               // ALSO CHECK IF IT'S THE SAME VARIABLE USED LATER, THAT DOESN'T COUNT
                               if(rplCompareIDENT(*s.right,tmp)) {
                               // BREAK THE LOOP, JUST ASSIGN THE CURRENT ARGUMENT
                               int32_t attr=rplGetIdentAttr(*s.right);

                               if(attr) {
                               // MATCH THE ATTRIBUTES
                               int32_t otherattr;

                               otherattr=rplSymbGetAttr(*s.left);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               if(((attr|IDATTR_ALLTYPES)&otherattr)!=attr) {
                               // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                               matchtype=BACKTRACK;
                               break;
                               }
                               }
                               rplCreateLAM(*s.right,*s.left);
                               matchtype=ARGDONE;
                               updateLAMs(&s);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               break;
                               }
                               }

                               }

                               // ROT THE RIGHT ARGUMENTS TO SEND THE EXPANSIVE EXPRESSION LAST

                               for(k=p.rightidx;k<p.rightnargs;++k)  FINDARGUMENT(p.right,p.rightnargs,k)=FINDARGUMENT(p.right,p.rightnargs,k+1);
                               FINDARGUMENT(p.right,p.rightnargs,k)=tmp;

                               // THEN REPLACE THE right PART ON THE CURRENT EXPRESSION AND REDO THIS OP_MATCH
                               DSTop=s.left+1;

                               // PUSH THE RIGHT
                               rplSymbExplodeOneLevel2(FINDARGUMENT(p.right,p.rightnargs,p.rightidx));
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }
                               s.right=DSTop-1;

                               // LEFTARG AND RIGHTARG
                               rplNewSINTPush(s.leftidx,DECint32_t);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }
                               rplNewSINTPush(s.rightidx,DECint32_t);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }
                               rplPushData((word_p)zero_bint);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }
                               rplPushData((word_p)zero_bint);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }
                               s.nlams=0;
                               updateLAMs(&s);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               // WARNING: THIS BECOMES AN INFINITE LOOP WHEN THERE'S 2 EXPANSIVE VARIABLES ON THE SAME SUM

                               break;
                               }

                               // OTHERWISE CREATE A SYMBOLIC WITH THE SAME OPERATOR AND ALL REMAINING ARGUMENTS
                               // AND ASSIGN IT TO THIS VARIABLE
                               for(k=p.leftidx;k<=p.leftnargs;++k) {
                               rplPushData(FINDARGUMENT(p.left,p.leftnargs,k));
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }
                               }
                               if(p.leftnargs>p.leftidx) {
                               rplSymbApplyOperator(**p.left,p.leftnargs-p.leftidx+1);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }
                               }

                               int32_t attr=rplGetIdentAttr(*s.right);

                               if(attr) {
                               // MATCH THE ATTRIBUTES
                               int32_t otherattr;

                               otherattr=rplSymbGetAttr(rplPeekData(1));
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               if(((attr|IDATTR_ALLTYPES)&otherattr)!=attr) {
                               // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                               rplPopData(1);
                               matchtype=BACKTRACK;
                               break;
                               }
                               }

                               rplCreateLAM(*s.right,rplPopData());
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               if(s.lrotbase) {
                               // COPY THE ROTATED ARGUMENTS FROM THE LEFT INTO THE UPPER LEVEL
                               // TO MAKE SURE THEY ARE PICKED UP ON REPLACEMENT

                               int32_t k;
                               // COPY ALL ARGUMENTS TO THE UPPER ENVIRONMENT
                               for(k=1;k<=p.leftnargs;++k) FINDARGUMENT(p.left,p.leftnargs,k)=FINDARGUMENT(s.left,s.leftnargs,k);

                               }

                               DSTop=s.left-( (s.leftnargs)? (1+s.leftnargs):0); // REMOVE THIS LEVEL

                               // FINALLY, REMOVE ALL EXTRA ARGUMENTS FROM THE PARENT
                               if(p.leftnargs>p.leftidx) {
                               rplRemoveAtData(DSTop-&FINDARGUMENT(p.left,p.leftnargs,p.leftnargs),p.leftnargs-p.leftidx);
                               p.left-=p.leftnargs-p.leftidx;
                               p.right-=p.leftnargs-p.leftidx;
                               if(baselevel>p.left-DStkBottom) baselevel-=p.leftnargs-p.leftidx;
                               p.left[-1]=rplNewSINT(p.leftidx,DECint32_t);
                               p.leftnargs=p.leftidx;
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }
                               }
                               // UPDATE THE VARIABLES ON THIS LEVEL
                               updateLAMs(&p);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               matchtype=ARGDONE;

                               break;

                               }

                               // IN ALL OTHER CASES, JUST ASSIGN THE CURRENT ARGUMENT
                               if(s.leftnargs) {
                               // ASSIGN THE WHOLE EXPRESSION FROM THE PARENT TREE

                               int32_t attr=rplGetIdentAttr(*s.right);

                               if(attr) {
                               // MATCH THE ATTRIBUTES
                               int32_t otherattr;

                               if(p.leftidx>=1 && p.leftidx<=p.leftnargs) otherattr=rplSymbGetAttr(FINDARGUMENT(p.left,p.leftnargs,p.leftidx));
                               else if(p.leftidx==-1) otherattr=rplSymbGetAttr(*p.left);
                               else otherattr=0;

                               if(((attr|IDATTR_ALLTYPES)&otherattr)!=attr) {
                               // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                               matchtype=BACKTRACK;
                               break;
                               }

                               }

                               if(p.leftidx>=1 && p.leftidx<=p.leftnargs) rplCreateLAM(*s.right,FINDARGUMENT(p.left,p.leftnargs,p.leftidx));
                               else if(p.leftidx==-1) rplCreateLAM(*s.right,*p.left);
                               // else SOMETHING IS WRONG IN THE EXPRESSION.

                               s.leftidx=s.leftnargs;
                               updateCounters(&s);

                               } else {
                               int32_t attr=rplGetIdentAttr(*s.right);

                               if(attr) {
                               // MATCH THE ATTRIBUTES
                               int32_t otherattr;

                               otherattr=rplSymbGetAttr(*s.left);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               if(((attr|IDATTR_ALLTYPES)&otherattr)!=attr) {
                               // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                               matchtype=BACKTRACK;
                               break;
                               }

                               }

                               rplCreateLAM(*s.right,*s.left);

                               }
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               updateLAMs(&s);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               matchtype=ARGDONE;
                               break;

                             */

                            if((**p.left == CMD_OVR_ADD)
                                    || (**p.left == CMD_OVR_MUL)) {
                                // IT'S NON-COMMUTATIVE BUT ASSOCIATIVE OPERATOR, TAKE ALL ARGUMENTS LESS WHAT'S LEFT ON THE RIGHT SIDE
                                int32_t k;

                                int32_t otherright = p.rightnargs - p.rightidx;    // OTHER ARGUMENTS ON THE RIGHT OPERATOR AFTER THIS ONE
                                int32_t available = p.leftnargs - p.leftidx + 1 - otherright;      // NUMBER OF ARGUMENTS AVAILABLE FOR THIS EXPRESSION

                                if(available > 1) {
                                    // CREATE A SYMBOLIC WITH THE SAME OPERATOR AND ALL REMAINING ARGUMENTS
                                    // AND ASSIGN IT TO THIS VARIABLE
                                    for(k = p.leftidx;
                                            k <= p.leftnargs - otherright;
                                            ++k) {
                                        rplPushData(FINDARGUMENT(p.left,
                                                    p.leftnargs, k));
                                        if(Exceptions) {
                                            rplCleanupSnapshots(stkbottom);
                                            DSTop = expression;
                                            LAMTop = lamsave;
                                            nLAMBase = lamcurrent;
                                            return 0;
                                        }
                                    }

                                    rplSymbApplyOperator(**p.left, available);
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    int32_t attr = rplGetIdentAttr(*s.right);

                                    if(attr) {
                                        // MATCH THE ATTRIBUTES
                                        int32_t otherattr;

                                        otherattr =
                                                rplSymbGetAttr(rplPeekData(1));
                                        if(Exceptions) {
                                            rplCleanupSnapshots(stkbottom);
                                            DSTop = expression;
                                            LAMTop = lamsave;
                                            nLAMBase = lamcurrent;
                                            return 0;
                                        }

                                        if(((attr | IDATTR_ALLTYPES) &
                                                    otherattr) != attr) {
                                            // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                            rplDropData(1);
                                            matchtype = BACKTRACK;
                                            break;
                                        }

                                    }

                                    rplCreateLAM(*s.right, rplPopData());
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    DSTop = s.left - ((s.leftnargs) ? (1 + s.leftnargs) : 0);   // REMOVE THIS LEVEL
                                    if(baselevel > DSTop - DStkBottom)
                                        baselevel = DSTop - DStkBottom;
                                    p.leftdepth =
                                            p.rightdepth + s.leftdepth -
                                            s.rightdepth;

                                    // FINALLY, REMOVE ALL EXTRA ARGUMENTS FROM THE PARENT
                                    rplRemoveAtData(DSTop -
                                            &FINDARGUMENT(p.left, p.leftnargs,
                                                p.leftnargs - otherright),
                                            available - 1);
                                    p.left -= available - 1;
                                    p.right -= available - 1;
                                    if(baselevel > p.left - DStkBottom)
                                        baselevel -= available - 1;
                                    p.left[-1] =
                                            rplNewSINT(p.leftnargs -
                                            (available - 1), DECint32_t);

                                    if(baselevel == DSTop - DStkBottom)
                                        baselevel += 2;
                                    updateCounters(&p);
                                    updateLAMs(&p);
                                    updateLAMs(&p);
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    matchtype = ARGDONE;

                                    break;

                                }

                            }
                            // IN ALL OTHER CASES, JUST ASSIGN THE CURRENT ARGUMENT
                            if(s.leftnargs) {
                                /*
                                   if(p.lrotbase && p.leftrot && (**p.left==CMD_OVR_MUL)) {
                                   // DO NOT ACCEPT ANY MATCH THAT HAS ANY ROTATION WHEN MULTIPLICATION IS ACTIVE
                                   matchtype=BACKTRACK;
                                   break;
                                   }
                                 */

                                int32_t attr = rplGetIdentAttr(*s.right);

                                if(attr) {
                                    // MATCH THE ATTRIBUTES
                                    int32_t otherattr;

                                    if(p.leftidx >= 1
                                            && p.leftidx <= p.leftnargs)
                                        otherattr =
                                                rplSymbGetAttr(FINDARGUMENT(p.
                                                    left, p.leftnargs,
                                                    p.leftidx));
                                    else if(p.leftidx == -1)
                                        otherattr = rplSymbGetAttr(*p.left);
                                    else
                                        otherattr = 0;

                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    if(((attr | IDATTR_ALLTYPES) & otherattr) !=
                                            attr) {
                                        // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                }

                                // ASSIGN THE WHOLE EXPRESSION FROM THE PARENT TREE

                                if(p.leftidx >= 1 && p.leftidx <= p.leftnargs)
                                    rplCreateLAM(*s.right, FINDARGUMENT(p.left,
                                                p.leftnargs, p.leftidx));
                                else if(p.leftidx == -1)
                                    rplCreateLAM(*s.right, *p.left);
                                // else SOMETHING IS WRONG IN THE EXPRESSION.

                                s.leftidx = s.leftnargs;
                                updateCounters(&s);

                            }
                            else {
                                int32_t attr = rplGetIdentAttr(*s.right);

                                if(attr) {
                                    // MATCH THE ATTRIBUTES
                                    int32_t otherattr;

                                    otherattr = rplSymbGetAttr(*s.left);
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    if(((attr | IDATTR_ALLTYPES) & otherattr) !=
                                            attr) {
                                        // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                }
                                rplCreateLAM(*s.right, *s.left);

                            }
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }
                            if(baselevel == DSTop - DStkBottom) {
                                // IF THIS IS THE BASE LEVEL, WE NEED TO UPDATE IT WITH THE NEW LAMS
                                baselevel += 2;
                            }
                            updateLAMs(&s);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }

                            matchtype = ARGDONE;
                            break;

                        }
                        case TEXT2WORD('.', 'm', 0, 0):
                            // m = Match smallest expression with any variables or constants, assuming commutative addition but non-commutative multiplication (use this with matrix expressions)
                        {

                            // GET POINTERS TO THE PARENT EXPRESSION
                            TRACK_STATE p;
                            reloadPointers(s.left - ((s.leftnargs) ? (1 +
                                            s.leftnargs) : 0), &p);

                            if(p.lrotbase && (p.leftrot > 0)
                                    && (**p.left == CMD_OVR_MUL)) {
                                // DO NOT ACCEPT ANY MATCH THAT HAS ANY ROTATION WHEN MULTIPLICATION IS ACTIVE
                                matchtype = BACKTRACK;
                                break;
                            }

                            if(s.leftnargs) {
                                // JUST CAPTURE THE CURRENT ARGUMENT

                                int32_t attr = rplGetIdentAttr(*s.right);

                                if(attr) {
                                    // MATCH THE ATTRIBUTES
                                    int32_t otherattr;

                                    if(p.leftidx >= 1
                                            && p.leftidx <= p.leftnargs)
                                        otherattr =
                                                rplSymbGetAttr(FINDARGUMENT(p.
                                                    left, p.leftnargs,
                                                    p.leftidx));
                                    else if(p.leftidx == -1)
                                        otherattr = rplSymbGetAttr(*p.left);
                                    else
                                        otherattr = 0;
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    if(((attr | IDATTR_ALLTYPES) & otherattr) !=
                                            attr) {
                                        // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                }

                                if(p.leftidx >= 1 && p.leftidx <= p.leftnargs)
                                    rplCreateLAM(*s.right, FINDARGUMENT(p.left,
                                                p.leftnargs, p.leftidx));
                                else if(p.leftidx == -1)
                                    rplCreateLAM(*s.right, *p.left);
                                // else SOMETHING IS WRONG IN THE EXPRESSION.
                                if(Exceptions) {
                                    rplCleanupSnapshots(stkbottom);
                                    DSTop = expression;
                                    LAMTop = lamsave;
                                    nLAMBase = lamcurrent;
                                    return 0;
                                }

                                s.leftidx = s.leftnargs;
                                updateCounters(&s);

                            }
                            else {
                                int32_t attr = rplGetIdentAttr(*s.right);

                                if(attr) {
                                    // MATCH THE ATTRIBUTES
                                    int32_t otherattr;

                                    otherattr = rplSymbGetAttr(*s.left);
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    if(((attr | IDATTR_ALLTYPES) & otherattr) !=
                                            attr) {
                                        // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                }

                                rplCreateLAM(*s.right, *s.left);
                            }
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }
                            if(baselevel == DSTop - DStkBottom) {
                                // IF THIS IS THE BASE LEVEL, WE NEED TO UPDATE IT WITH THE NEW LAMS
                                baselevel += 2;
                            }
                            updateLAMs(&s);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }

                            matchtype = ARGDONE;
                            break;
                        }
                        case TEXT2WORD('.', 'M', 0, 0):
                            // M = Same as m but largest expression
                            // TAKE ALL ARGUMENTS OF THE OPERATION
                        {
                            // GET POINTERS TO THE PARENT EXPRESSION
                            TRACK_STATE p;
                            reloadPointers(s.left - ((s.leftnargs) ? (1 +
                                            s.leftnargs) : 0), &p);

                            if(p.lrotbase && p.leftrot) {
                                matchtype = BACKTRACK;
                                break;
                            }
                            /*
                               if(**p.left==CMD_OVR_ADD)  // IT'S A COMMUTATIVE/ASSOCIATIVE OPERATOR
                               {
                               int32_t k;

                               if(p.rightidx!=p.rightnargs) { // THE SPECIAL IDENT IS NOT THE LAST ARGUMENT

                               word_p tmp=FINDARGUMENT(p.right,p.rightnargs,p.rightnargs);

                               if(ISIDENT(*tmp)) {
                               // CHECK IF THIS IS ANOTHER SPECIAL IDENT AND BREAK THE INFINITE LOOP
                               if( (((tmp[1])&0xffff) == TEXT2WORD('.','X',0,0))||(((tmp[1])&0xffff) == TEXT2WORD('.','M',0,0))) {
                               // ALSO CHECK IF IT'S THE SAME VARIABLE USED LATER, THAT DOESN'T COUNT
                               if(rplCompareIDENT(*s.right,tmp)) {
                               // BREAK THE LOOP, JUST ASSIGN THE CURRENT ARGUMENT
                               int32_t attr=rplGetIdentAttr(*s.right);

                               if(attr) {
                               // MATCH THE ATTRIBUTES
                               int32_t otherattr;

                               otherattr=rplSymbGetAttr(s.left);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               if(((attr|IDATTR_ALLTYPES)&otherattr)!=attr) {
                               // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                               matchtype=BACKTRACK;
                               break;
                               }

                               }

                               rplCreateLAM(*s.right,*s.left);
                               updateLAMs(&s);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               matchtype=ARGDONE;
                               break;
                               }
                               }

                               }

                               // ROT THE RIGHT ARGUMENTS TO SEND THE EXPANSIVE EXPRESSION LAST

                               for(k=p.rightidx;k<p.rightnargs;++k)  FINDARGUMENT(p.right,p.rightnargs,k)=FINDARGUMENT(p.right,p.rightnargs,k+1);
                               FINDARGUMENT(p.right,p.rightnargs,k)=tmp;

                               // THEN REPLACE THE right PART ON THE CURRENT EXPRESSION AND REDO THIS OP_MATCH
                               DSTop=s.left+1;

                               // PUSH THE RIGHT
                               rplSymbExplodeOneLevel2(FINDARGUMENT(p.right,p.rightnargs,p.rightidx));
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }
                               s.right=DSTop-1;

                               s.leftrot=s.lrotbase=0;
                               s.nlams=0;
                               rplExpandStack(4);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }
                               DSTop+=3;
                               updateCounters(&s);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }
                               *DSTop++=(word_p)zero_bint;

                               updateLAMs(&s);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               break;
                               }

                               // OTHERWISE CREATE A SYMBOLIC WITH THE SAME OPERATOR AND ALL REMAINING ARGUMENTS
                               // AND ASSIGN IT TO THIS VARIABLE
                               for(k=p.leftidx;k<=p.leftnargs;++k) {
                               rplPushData(FINDARGUMENT(p.left,p.leftnargs,k));
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }
                               }
                               if(p.leftnargs>p.leftidx) {
                               rplSymbApplyOperator(**p.left,p.leftnargs-p.leftidx+1);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }
                               }

                               int32_t attr=rplGetIdentAttr(*s.right);

                               if(attr) {
                               // MATCH THE ATTRIBUTES
                               int32_t otherattr;

                               otherattr=rplSymbGetAttr(rplPeekData(1));
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               if(((attr|IDATTR_ALLTYPES)&otherattr)!=attr) {
                               // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                               rplDropData(1);
                               matchtype=BACKTRACK;
                               break;
                               }

                               }

                               rplCreateLAM(*s.right,rplPopData());
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               DSTop=s.left-( (s.leftnargs)? (1+s.leftnargs):0); // REMOVE THIS LEVEL

                               // FINALLY, REMOVE ALL EXTRA ARGUMENTS FROM THE PARENT
                               if(p.leftnargs>p.leftidx) {
                               rplRemoveAtData(DSTop-&FINDARGUMENT(p.left,p.leftnargs,p.leftnargs),p.leftnargs-p.leftidx);
                               p.left-=p.leftnargs-p.leftidx;
                               if(baselevel>p.left-DStkBottom) baselevel-=p.leftnargs-p.leftidx;
                               p.left[-1]=rplNewSINT(p.leftidx,DECint32_t);
                               }

                               updateLAMs(&p);
                               if(Exceptions) { rplCleanupSnapshots(stkbottom); DSTop=expression; LAMTop=lamsave; nLAMBase=lamcurrent; return 0; }

                               matchtype=ARGDONE;

                               break;

                               }
                               else */
                            if((**p.left == CMD_OVR_ADD)
                                    || (**p.left == CMD_OVR_MUL)) {
                                // IT'S NON-COMMUTATIVE BUT ASSOCIATIVE OPERATOR, TAKE ALL ARGUMENTS LESS WHAT'S LEFT ON THE RIGHT SIDE
                                int32_t k;

                                int32_t otherright = p.rightnargs - p.rightidx;    // OTHER ARGUMENTS ON THE RIGHT OPERATOR AFTER THIS ONE
                                int32_t available = p.leftnargs - p.leftidx + 1 - otherright;      // NUMBER OF ARGUMENTS AVAILABLE FOR THIS EXPRESSION

                                if(available > 1) {
                                    // CREATE A SYMBOLIC WITH THE SAME OPERATOR AND ALL REMAINING ARGUMENTS
                                    // AND ASSIGN IT TO THIS VARIABLE
                                    for(k = p.leftidx;
                                            k <= p.leftnargs - otherright;
                                            ++k) {
                                        rplPushData(FINDARGUMENT(p.left,
                                                    p.leftnargs, k));
                                        if(Exceptions) {
                                            rplCleanupSnapshots(stkbottom);
                                            DSTop = expression;
                                            LAMTop = lamsave;
                                            nLAMBase = lamcurrent;
                                            return 0;
                                        }
                                    }

                                    rplSymbApplyOperator(**p.left, available);
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    int32_t attr = rplGetIdentAttr(*s.right);

                                    if(attr) {
                                        // MATCH THE ATTRIBUTES
                                        int32_t otherattr;

                                        otherattr =
                                                rplSymbGetAttr(rplPeekData(1));
                                        if(Exceptions) {
                                            rplCleanupSnapshots(stkbottom);
                                            DSTop = expression;
                                            LAMTop = lamsave;
                                            nLAMBase = lamcurrent;
                                            return 0;
                                        }

                                        if(((attr | IDATTR_ALLTYPES) &
                                                    otherattr) != attr) {
                                            // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                            rplDropData(1);
                                            matchtype = BACKTRACK;
                                            break;
                                        }

                                    }

                                    rplCreateLAM(*s.right, rplPopData());
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    DSTop = s.left - ((s.leftnargs) ? (1 + s.leftnargs) : 0);   // REMOVE THIS LEVEL
                                    if(baselevel > DSTop - DStkBottom)
                                        baselevel = DSTop - DStkBottom;
                                    p.leftdepth =
                                            p.rightdepth + s.leftdepth -
                                            s.rightdepth;

                                    // FINALLY, REMOVE ALL EXTRA ARGUMENTS FROM THE PARENT
                                    rplRemoveAtData(DSTop -
                                            &FINDARGUMENT(p.left, p.leftnargs,
                                                p.leftnargs - otherright),
                                            available - 1);
                                    p.left -= available - 1;
                                    p.right -= available - 1;
                                    if(baselevel > p.left - DStkBottom)
                                        baselevel -= available - 1;
                                    p.left[-1] =
                                            rplNewSINT(p.leftnargs -
                                            (available - 1), DECint32_t);

                                    if(baselevel == DSTop - DStkBottom)
                                        baselevel += 2;
                                    updateCounters(&p);
                                    updateLAMs(&p);
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    matchtype = ARGDONE;

                                    break;

                                }

                            }
                            // IN ALL OTHER CASES, JUST ASSIGN THE CURRENT ARGUMENT
                            if(s.leftnargs) {

                                if(p.lrotbase && p.leftrot
                                        && (**p.left == CMD_OVR_MUL)) {
                                    // DO NOT ACCEPT ANY MATCH THAT HAS ANY ROTATION WHEN MULTIPLICATION IS ACTIVE
                                    matchtype = BACKTRACK;
                                    break;
                                }

                                int32_t attr = rplGetIdentAttr(*s.right);

                                if(attr) {
                                    // MATCH THE ATTRIBUTES
                                    int32_t otherattr;

                                    if(p.leftidx >= 1
                                            && p.leftidx <= p.leftnargs)
                                        otherattr =
                                                rplSymbGetAttr(FINDARGUMENT(p.
                                                    left, p.leftnargs,
                                                    p.leftidx));
                                    else if(p.leftidx == -1)
                                        otherattr = rplSymbGetAttr(*p.left);
                                    else
                                        otherattr = 0;

                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    if(((attr | IDATTR_ALLTYPES) & otherattr) !=
                                            attr) {
                                        // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                }

                                // ASSIGN THE WHOLE EXPRESSION FROM THE PARENT TREE

                                if(p.leftidx >= 1 && p.leftidx <= p.leftnargs)
                                    rplCreateLAM(*s.right, FINDARGUMENT(p.left,
                                                p.leftnargs, p.leftidx));
                                else if(p.leftidx == -1)
                                    rplCreateLAM(*s.right, *p.left);
                                // else SOMETHING IS WRONG IN THE EXPRESSION.

                                s.leftidx = s.leftnargs;
                                updateCounters(&s);

                            }
                            else {
                                int32_t attr = rplGetIdentAttr(*s.right);

                                if(attr) {
                                    // MATCH THE ATTRIBUTES
                                    int32_t otherattr;

                                    otherattr = rplSymbGetAttr(*s.left);
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    if(((attr | IDATTR_ALLTYPES) & otherattr) !=
                                            attr) {
                                        // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                }
                                rplCreateLAM(*s.right, *s.left);

                            }
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }
                            if(baselevel == DSTop - DStkBottom) {
                                // IF THIS IS THE BASE LEVEL, WE NEED TO UPDATE IT WITH THE NEW LAMS
                                baselevel += 2;
                            }
                            updateLAMs(&s);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }

                            matchtype = ARGDONE;
                            break;
                        }
                        case TEXT2WORD('.', 'n', 0, 0):
                            // n = Match only a single number (real or integer)
                            if(rplSymbIsANumber(*s.left)) {

                                int32_t attr = rplGetIdentAttr(*s.right);

                                if(attr) {
                                    // MATCH THE ATTRIBUTES
                                    int32_t otherattr;

                                    otherattr = rplSymbGetAttr(*s.left);
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    if(((attr | IDATTR_ALLTYPES) & otherattr) !=
                                            attr) {
                                        // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                }

                                rplCreateLAM(*s.right, *s.left);

                                if(baselevel == DSTop - DStkBottom) {
                                    // IF THIS IS THE BASE LEVEL, WE NEED TO UPDATE IT WITH THE NEW LAMS
                                    baselevel += 2;
                                }
                                updateLAMs(&s);
                                if(Exceptions) {
                                    rplCleanupSnapshots(stkbottom);
                                    DSTop = expression;
                                    LAMTop = lamsave;
                                    nLAMBase = lamcurrent;
                                    return 0;
                                }

                                matchtype = ARGDONE;
                            }
                            else
                                matchtype = BACKTRACK;
                            break;

                        case TEXT2WORD('.', 'N', 0, 0):
                            // N = Match the largest expression involving only numbers (real or integer)
                        {
                            // GET POINTERS TO THE PARENT EXPRESSION
                            TRACK_STATE p;
                            reloadPointers(s.left - ((s.leftnargs) ? (1 +
                                            s.leftnargs) : 0), &p);

                            //int32_t otherright=p.rightnargs-p.rightidx;  // OTHER ARGUMENTS ON THE RIGHT OPERATOR AFTER THIS ONE
                            //int32_t available=p.leftnargs-otherright; // NUMBER OF ARGUMENTS AVAILABLE FOR THIS EXPRESSION

                            if((**p.left == CMD_OVR_ADD)
                                    || (**p.left == CMD_OVR_MUL)) {
                                // IT'S NON-COMMUTATIVE BUT ASSOCIATIVE OPERATOR, TAKE ALL ARGUMENTS LESS WHAT'S LEFT ON THE RIGHT SIDE
                                int32_t k;
                                int32_t taken;

                                int32_t otherright = p.rightnargs - p.rightidx;    // OTHER ARGUMENTS ON THE RIGHT OPERATOR AFTER THIS ONE
                                int32_t available = p.leftnargs - p.leftidx + 1 - otherright;      // NUMBER OF ARGUMENTS AVAILABLE FOR THIS EXPRESSION

                                if(available > 1) {
                                    // CREATE A SYMBOLIC WITH THE SAME OPERATOR AND ALL REMAINING ARGUMENTS
                                    // AND ASSIGN IT TO THIS VARIABLE
                                    for(k = p.leftidx, taken = 0;
                                            k <= p.leftnargs - otherright;
                                            ++k) {
                                        word_p object =
                                                FINDARGUMENT(p.left,
                                                p.leftnargs, k);
                                        if(rplSymbIsNumeric(object)) {
                                            rplPushData(object);
                                            ++taken;

                                        }
                                        else
                                            break;
                                        if(Exceptions) {
                                            rplCleanupSnapshots(stkbottom);
                                            DSTop = expression;
                                            LAMTop = lamsave;
                                            nLAMBase = lamcurrent;
                                            return 0;
                                        }
                                    }
                                    if(taken > 1) {
                                        rplSymbApplyOperator(**p.left, taken);
                                        if(Exceptions) {
                                            rplCleanupSnapshots(stkbottom);
                                            DSTop = expression;
                                            LAMTop = lamsave;
                                            nLAMBase = lamcurrent;
                                            return 0;
                                        }
                                    }

                                    if(!taken) {
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                    int32_t attr = rplGetIdentAttr(*s.right);

                                    if(attr) {
                                        // MATCH THE ATTRIBUTES
                                        int32_t otherattr;

                                        otherattr =
                                                rplSymbGetAttr(rplPeekData(1));
                                        if(Exceptions) {
                                            rplCleanupSnapshots(stkbottom);
                                            DSTop = expression;
                                            LAMTop = lamsave;
                                            nLAMBase = lamcurrent;
                                            return 0;
                                        }

                                        if(((attr | IDATTR_ALLTYPES) &
                                                    otherattr) != attr) {
                                            // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                            rplDropData(1);
                                            matchtype = BACKTRACK;
                                            break;
                                        }

                                    }

                                    rplCreateLAM(*s.right, rplPopData());
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    DSTop = s.left - ((s.leftnargs) ? (1 + s.leftnargs) : 0);   // REMOVE THIS LEVEL
                                    if(baselevel > DSTop - DStkBottom)
                                        baselevel = DSTop - DStkBottom;
                                    p.leftdepth =
                                            p.rightdepth + s.leftdepth -
                                            s.rightdepth;

                                    // FINALLY, REMOVE ALL EXTRA ARGUMENTS FROM THE PARENT
                                    if(taken > 1) {
                                        rplRemoveAtData(DSTop -
                                                &FINDARGUMENT(p.left,
                                                    p.leftnargs,
                                                    p.leftidx + taken - 1),
                                                taken - 1);
                                        p.left -= taken - 1;
                                        p.right -= taken - 1;
                                        if(baselevel > p.left - DStkBottom)
                                            baselevel -= taken - 1;
                                        p.left[-1] =
                                                rplNewSINT(p.leftnargs -
                                                (taken - 1), DECint32_t);
                                    }

                                    if(baselevel == DSTop - DStkBottom)
                                        baselevel += 2;
                                    updateCounters(&p);
                                    updateLAMs(&p);
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    matchtype = ARGDONE;

                                    break;

                                }

                            }
                            // IN ALL OTHER CASES, JUST ASSIGN THE CURRENT ARGUMENT
                            if(s.leftnargs) {

                                int32_t attr = rplGetIdentAttr(*s.right);

                                if(attr) {
                                    // MATCH THE ATTRIBUTES
                                    int32_t otherattr;

                                    if(p.leftidx >= 1
                                            && p.leftidx <= p.leftnargs)
                                        otherattr =
                                                rplSymbGetAttr(FINDARGUMENT(p.
                                                    left, p.leftnargs,
                                                    p.leftidx));
                                    else if(p.leftidx == -1)
                                        otherattr = rplSymbGetAttr(*p.left);
                                    else
                                        otherattr = 0;

                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    if(((attr | IDATTR_ALLTYPES) & otherattr) !=
                                            attr) {
                                        // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                }

                                // ASSIGN THE WHOLE EXPRESSION FROM THE PARENT TREE
                                word_p expression = 0;

                                if(p.leftidx >= 1 && p.leftidx <= p.leftnargs)
                                    expression =
                                            FINDARGUMENT(p.left, p.leftnargs,
                                            p.leftidx);
                                else if(p.leftidx == -1)
                                    expression = *p.left;
                                // else SOMETHING IS WRONG IN THE EXPRESSION.
                                if(expression && rplSymbIsNumeric(expression))
                                    rplCreateLAM(*s.right, expression);
                                else {
                                    matchtype = BACKTRACK;
                                    break;
                                }

                                s.leftidx = s.leftnargs;
                                updateCounters(&s);

                            }
                            else {
                                int32_t attr = rplGetIdentAttr(*s.right);

                                if(attr) {
                                    // MATCH THE ATTRIBUTES
                                    int32_t otherattr;

                                    otherattr = rplSymbGetAttr(*s.left);
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    if(((attr | IDATTR_ALLTYPES) & otherattr) !=
                                            attr) {
                                        // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                }
                                if(rplSymbIsNumeric(*s.left)) {
                                    rplCreateLAM(*s.right, *s.left);
                                }
                                else {
                                    matchtype = BACKTRACK;
                                    break;
                                }

                            }
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }
                            if(baselevel == DSTop - DStkBottom) {
                                // IF THIS IS THE BASE LEVEL, WE NEED TO UPDATE IT WITH THE NEW LAMS
                                baselevel += 2;
                            }
                            updateLAMs(&s);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }

                            matchtype = ARGDONE;
                            break;
                        }

                        case TEXT2WORD('.', 'i', 0, 0):
                            // i = Match only a single integer number
                            if(rplSymbIsIntegerNumber(*s.left)) {
                                int32_t attr = rplGetIdentAttr(*s.right);

                                if(attr) {
                                    // MATCH THE ATTRIBUTES
                                    int32_t otherattr;

                                    otherattr = rplSymbGetAttr(*s.left);
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    if(((attr | IDATTR_ALLTYPES) & otherattr) !=
                                            attr) {
                                        // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                }

                                rplCreateLAM(*s.right, *s.left);
                                matchtype = ARGDONE;
                            }
                            else
                                matchtype = BACKTRACK;
                            break;
                        case TEXT2WORD('.', 'o', 0, 0):
                            // i = Match only a single odd integer number
                            if(rplSymbIsOddNumber(*s.left)) {

                                rplCreateLAM(*s.right, *s.left);
                                matchtype = ARGDONE;
                            }
                            else
                                matchtype = BACKTRACK;
                            break;
                        case TEXT2WORD('.', 'e', 0, 0):
                            // i = Match only a single even integer number
                            if(rplSymbIsOddNumber(*s.left))
                                matchtype = BACKTRACK;
                            else {
                                if(rplSymbIsIntegerNumber(*s.left)) {
                                    rplCreateLAM(*s.right, *s.left);
                                    matchtype = ARGDONE;
                                }
                                else
                                    matchtype = BACKTRACK;
                            }
                            break;

                        case TEXT2WORD('.', 'v', 0, 0):
                            // v = Match a single variable name
                            if(ISIDENT(**s.left)) {
                                int32_t attr = rplGetIdentAttr(*s.right);

                                if(attr) {
                                    // MATCH THE ATTRIBUTES
                                    int32_t otherattr;

                                    otherattr = rplSymbGetAttr(*s.left);

                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    if(((attr | IDATTR_ALLTYPES) & otherattr) !=
                                            attr) {
                                        // DO NOT ACCEPT ANY MATCH THAT HAS AT LEAST THE REQUIRED BITS
                                        matchtype = BACKTRACK;
                                        break;
                                    }

                                }

                                rplCreateLAM(*s.right, *s.left);
                                matchtype = ARGDONE;
                            }
                            else
                                matchtype = BACKTRACK;
                            break;
                        default:
                            // THIS IS NOT A SPECIAL IDENT, JUST COMPARE THE IDENTS
                            if(rplCompareIDENT(*s.left, *s.right))
                                matchtype = ARGDONE;
                            else
                                matchtype = BACKTRACK;
                        }
                    }
                }
                else {
                    // NO OPERATOR, JUST AN OBJECT, NOTHING SPECIAL
                    rplPushData(*s.left);
                    rplPushData(*s.right);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    rplCallOvrOperator(CMD_OVR_SAME);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    if(rplIsFalse(rplPopData()))
                        matchtype = BACKTRACK;
                    else
                        matchtype = ARGDONE;
                }

            }
            break;
        }
        case ARGMATCH:
        {
            // UPDATE POINTERS
            reloadPointers(DSTop, &s);

            // ******************************************************
            // DEBUG ONLY AREA
            // ******************************************************
#ifdef RULEDEBUG
            printf("ARGMATCH: %d/%d,%d/%d [%d,%d]", s.leftidx, s.leftnargs,
                    s.rightidx, s.rightnargs, s.leftdepth, s.rightdepth);
            if((s.leftidx > 0) && (s.leftidx <= s.leftnargs)) {
                word_p string =
                        rplDecompile(FINDARGUMENT(s.left, s.leftnargs,
                            s.leftidx), DECOMP_EDIT | DECOMP_NOHINTS);
                if(string) {
                    BYTE strbyte[1024];
                    memmoveb(strbyte, (byte_p) (string + 1),
                            rplStrSize(string));
                    strbyte[rplStrSize(string)] = 0;
                    printf("  left=%s", strbyte);
                }
            }
            if((s.rightidx > 0) && (s.rightidx <= s.rightnargs)) {
                word_p string =
                        rplDecompile(FINDARGUMENT(s.right, s.rightnargs,
                            s.rightidx), DECOMP_EDIT | DECOMP_NOHINTS);
                if(string) {
                    BYTE strbyte[1024];
                    memmoveb(strbyte, (byte_p) (string + 1),
                            rplStrSize(string));
                    strbyte[rplStrSize(string)] = 0;
                    printf("  right=%s", strbyte);
                }
            }
            printf("\n");
            fflush(stdout);
#endif
            // ******************************************************
            //      END DEBUG ONLY AREA
            // ******************************************************

            if(s.leftidx <= s.leftnargs) {
                if(s.rightidx > s.rightnargs) {
                    // THERE'S EXTRA ARGUMENTS IN THE LEFT PART, COULD BE A PARTIAL MATCH
                    matchtype = ARGDONEEXTRA;
                }
                else {
                    // RECURSE INTO THE ARGUMENT
                    // PUSH THE LEFT
                    rplSymbExplodeOneLevel2(FINDARGUMENT(s.left, s.leftnargs,
                                s.leftidx));
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    // PUSH THE RIGHT
                    rplSymbExplodeOneLevel2(FINDARGUMENT(s.right, s.rightnargs,
                                s.rightidx));
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    s.right = DSTop - 1;

                    // LEFTARG AND RIGHTARG
                    rplNewint32_tPush(((int64_t) s.leftdepth + 1) << 32, DECint32_t);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    rplNewint32_tPush(((int64_t) s.rightdepth + 1) << 32, DECint32_t);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    rplPushData((word_p) zero_bint);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    rplPushData((word_p) zero_bint);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }

                    matchtype = OPMATCH;
                }
            }
            else {
                // WE NEED MORE ARGUMENTS! THIS IS NOT A MATCH
                matchtype = BACKTRACK;
            }
            break;
        }
        case RESTARTMATCH:
        {
            // BACK TO START LEVEL
            DSTop = DStkBottom + baselevel;

            // DISCARD/REMOVE ALL PREVIOUS SNAPSHOTS, KEEP ONLY THE CURRENT THREAD
            while(DStkBottom > basestkbottom) {
                rplRemoveSnapshot(1);
            }

            reloadPointers(DSTop, &s);

            // ******************************************************
            // DEBUG ONLY AREA
            // ******************************************************
#ifdef RULEDEBUG
            printf("RESTARTMATCH: %d/%d,%d/%d [%d,%d]", s.leftidx, s.leftnargs,
                    s.rightidx, s.rightnargs, s.leftdepth, s.rightdepth);
            if((s.leftidx > 0) && (s.leftidx <= s.leftnargs)) {
                word_p string =
                        rplDecompile(FINDARGUMENT(s.left, s.leftnargs,
                            s.leftidx), DECOMP_EDIT | DECOMP_NOHINTS);
                if(string) {
                    BYTE strbyte[1024];
                    memmoveb(strbyte, (byte_p) (string + 1),
                            rplStrSize(string));
                    strbyte[rplStrSize(string)] = 0;
                    printf("  left=%s", strbyte);
                }
            }
            if((s.rightidx > 0) && (s.rightidx <= s.rightnargs)) {
                word_p string =
                        rplDecompile(FINDARGUMENT(s.right, s.rightnargs,
                            s.rightidx), DECOMP_EDIT | DECOMP_NOHINTS);
                if(string) {
                    BYTE strbyte[1024];
                    memmoveb(strbyte, (byte_p) (string + 1),
                            rplStrSize(string));
                    strbyte[rplStrSize(string)] = 0;
                    printf("  right=%s", strbyte);
                }
            }
            printf("\n");
            fflush(stdout);
#endif
            // ******************************************************
            //      END DEBUG ONLY AREA
            // ******************************************************

            if(s.leftidx < s.leftnargs) {

                // ADVANCE TO THE NEXT ARGUMENT

                ++s.leftidx;
                // RECURSE INTO THE ARGUMENT
                updateCounters(&s);
                if(Exceptions) {
                    rplCleanupSnapshots(stkbottom);
                    DSTop = expression;
                    LAMTop = lamsave;
                    nLAMBase = lamcurrent;
                    return 0;
                }

                // PUSH THE LEFT
                rplSymbExplodeOneLevel2(FINDARGUMENT(s.left, s.leftnargs,
                            s.leftidx));
                if(Exceptions) {
                    rplCleanupSnapshots(stkbottom);
                    DSTop = expression;
                    LAMTop = lamsave;
                    nLAMBase = lamcurrent;
                    return 0;
                }
                // PUSH THE RIGHT
                rplSymbExplodeOneLevel2(*rule + ruleleftoffset);
                if(Exceptions) {
                    rplCleanupSnapshots(stkbottom);
                    DSTop = expression;
                    LAMTop = lamsave;
                    nLAMBase = lamcurrent;
                    return 0;
                }
                s.right = DSTop - 1;

                // LEFTARG AND RIGHTARG
                rplPushData((word_p) zero_bint);
                if(Exceptions) {
                    rplCleanupSnapshots(stkbottom);
                    DSTop = expression;
                    LAMTop = lamsave;
                    nLAMBase = lamcurrent;
                    return 0;
                }
                rplPushData((word_p) zero_bint);
                if(Exceptions) {
                    rplCleanupSnapshots(stkbottom);
                    DSTop = expression;
                    LAMTop = lamsave;
                    nLAMBase = lamcurrent;
                    return 0;
                }
                rplPushData((word_p) zero_bint);
                if(Exceptions) {
                    rplCleanupSnapshots(stkbottom);
                    DSTop = expression;
                    LAMTop = lamsave;
                    nLAMBase = lamcurrent;
                    return 0;
                }
                rplPushData((word_p) zero_bint);
                if(Exceptions) {
                    rplCleanupSnapshots(stkbottom);
                    DSTop = expression;
                    LAMTop = lamsave;
                    nLAMBase = lamcurrent;
                    return 0;
                }

                baselevel = DSTop - DStkBottom;
                matchtype = OPMATCH;
                matchstarted = 0;

                // ******************************************************
                // DEBUG ONLY AREA
                // ******************************************************
#ifdef RULEDEBUG
                printf("RESTARTED: %d/%d,%d/%d", s.leftidx, s.leftnargs,
                        s.rightidx, s.rightnargs);
                if((s.leftidx > 0) && (s.leftidx <= s.leftnargs)) {
                    word_p string =
                            rplDecompile(FINDARGUMENT(s.left, s.leftnargs,
                                s.leftidx), DECOMP_EDIT | DECOMP_NOHINTS);
                    if(string) {
                        BYTE strbyte[1024];
                        memmoveb(strbyte, (byte_p) (string + 1),
                                rplStrSize(string));
                        strbyte[rplStrSize(string)] = 0;
                        printf("  left=%s", strbyte);
                    }
                }
                if((s.rightidx > 0) && (s.rightidx <= s.rightnargs)) {
                    word_p string =
                            rplDecompile(FINDARGUMENT(s.right, s.rightnargs,
                                s.rightidx), DECOMP_EDIT | DECOMP_NOHINTS);
                    if(string) {
                        BYTE strbyte[1024];
                        memmoveb(strbyte, (byte_p) (string + 1),
                                rplStrSize(string));
                        strbyte[rplStrSize(string)] = 0;
                        printf("  right=%s", strbyte);
                    }
                }
                printf("\n");
                fflush(stdout);
#endif
                // ******************************************************
                //      END DEBUG ONLY AREA
                // ******************************************************

                rplCleanupLAMs(0);
                // CREATE LAM ENVIRONMENT
                rplCreateLAMEnvironment(*orgrule);
                if(Exceptions) {
                    rplCleanupSnapshots(stkbottom);
                    DSTop = expression;
                    LAMTop = lamsave;
                    nLAMBase = lamcurrent;
                    return 0;
                }
                rplCreateLAM((word_p) nulllam_ident, (word_p) zero_bint);
                if(Exceptions) {
                    rplCleanupSnapshots(stkbottom);
                    DSTop = expression;
                    LAMTop = lamsave;
                    nLAMBase = lamcurrent;
                    return 0;
                }

            }
            else {
                // LAST ARGUMENT WAS ALREADY COMPARED, BACKTRACK TO THE PREVIOUS BASE

                DSTop = s.left - ((s.leftnargs) ? (1 + s.leftnargs) : 0);
                baselevel = DSTop - DStkBottom;
                // ******************************************************
                // DEBUG ONLY AREA
                // ******************************************************
#ifdef RULEDEBUG
                printf("RESTARTMATCH UP");
                printf("\n");
                fflush(stdout);
#endif
                // ******************************************************
                //      END DEBUG ONLY AREA
                // ******************************************************

            }

            break;
        }
        case BACKTRACK:
        {

            reloadPointers(DSTop, &s);

            // ******************************************************
            // DEBUG ONLY AREA
            // ******************************************************
#ifdef RULEDEBUG
            printf("BACKTRACK: %d/%d,%d/%d [%d,%d]", s.leftidx, s.leftnargs,
                    s.rightidx, s.rightnargs, s.leftdepth, s.rightdepth);
            if((s.leftidx > 0) && (s.leftidx <= s.leftnargs)) {
                word_p string =
                        rplDecompile(FINDARGUMENT(s.left, s.leftnargs,
                            s.leftidx), DECOMP_EDIT | DECOMP_NOHINTS);
                if(string) {
                    BYTE strbyte[1024];
                    memmoveb(strbyte, (byte_p) (string + 1),
                            rplStrSize(string));
                    strbyte[rplStrSize(string)] = 0;
                    printf("  left=%s", strbyte);
                }
            }
            if((s.rightidx > 0) && (s.rightidx <= s.rightnargs)) {
                word_p string =
                        rplDecompile(FINDARGUMENT(s.right, s.rightnargs,
                            s.rightidx), DECOMP_EDIT | DECOMP_NOHINTS);
                if(string) {
                    BYTE strbyte[1024];
                    memmoveb(strbyte, (byte_p) (string + 1),
                            rplStrSize(string));
                    strbyte[rplStrSize(string)] = 0;
                    printf("  right=%s", strbyte);
                }
            }
            printf("\n");
            fflush(stdout);
#endif
            // ******************************************************
            //      END DEBUG ONLY AREA
            // ******************************************************

            if(DStkBottom > basestkbottom) {
                // THERE'S A SNAPSHOT STORED, RESTORE FROM SNAPSHOT AND RESUME

                rplDropCurrentStack();
                reloadPointers(DSTop, &s);
                reloadLAMs(orgrule, &s);

                // DO A ROT OF THE ARGUMENTS IF NEEDED

                if(s.lrotbase) {
                    if(s.leftnargs && (s.leftidx > 0)
                            && (s.leftidx < s.leftnargs)
                            && ((**s.left == CMD_OVR_MUL)
                                || (**s.left == CMD_OVR_ADD))) {

                        if(s.leftrot < s.leftnargs - s.leftidx) {
                            if((s.rightdepth == 0) && (s.lrotbase == 1)
                                    && (s.leftrot == 0) && (s.rightidx == 1)
                                    && (s.leftnargs > s.rightnargs)) {
                                // SPECIAL CASE: SCAN WITHOUT ROTATION FIRST TO FIND FIRST MATCH
                                if(s.leftnargs - s.leftidx + 1 <= s.rightnargs) {
                                    // NO MORE ARGUMENTS TO SCAN, PROCEED TO ROTATION OF ARGUMENTS
                                    s.leftidx = 1;
                                }
                                else {
                                    // SCAN THE NEXT ARGUMENT
                                    ++s.leftidx;
                                    matchtype = ARGMATCH;

                                    updateCounters(&s);
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    // AND RECREATE THE SNAPSHOT WE DROPPED ABOVE
                                    rplTakeSnapshot();
                                    if(Exceptions) {
                                        rplCleanupSnapshots(stkbottom);
                                        DSTop = expression;
                                        LAMTop = lamsave;
                                        nLAMBase = lamcurrent;
                                        return 0;
                                    }

                                    // ******************************************************
                                    // DEBUG ONLY AREA
                                    // ******************************************************
#ifdef RULEDEBUG
                                    printf("ARGUMENT SCAN");
                                    printf("\n");
                                    fflush(stdout);
#endif
                                    // ******************************************************
                                    //      END DEBUG ONLY AREA
                                    // ******************************************************

                                    break;
                                }

                            }

                            // COMMUTATIVE OPERATORS NEED TO ROT THE ARGUMENTS AND TRY AGAIN
                            word_p tmp =
                                    FINDARGUMENT(s.left, s.leftnargs,
                                    s.leftidx);
                            int32_t k;
                            for(k = s.leftidx; k < s.leftnargs; ++k)
                                FINDARGUMENT(s.left, s.leftnargs, k) =
                                        FINDARGUMENT(s.left, s.leftnargs,
                                        k + 1);
                            FINDARGUMENT(s.left, s.leftnargs, s.leftnargs) =
                                    tmp;
                            matchtype = ARGMATCH;
                            ++s.leftrot;
                            updateCounters(&s);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }

                            // AND RECREATE THE SNAPSHOT WE DROPPED ABOVE
                            rplTakeSnapshot();
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }

                            // ******************************************************
                            // DEBUG ONLY AREA
                            // ******************************************************
#ifdef RULEDEBUG
                            printf("ARGUMENT ROT1");
                            printf("\n");
                            fflush(stdout);
#endif
                            // ******************************************************
                            //      END DEBUG ONLY AREA
                            // ******************************************************
                        }
                        else {
                            if(s.leftrot
                                    && (s.leftrot == s.leftnargs - s.leftidx)) {
                                // ALL ARGUMENTS COMPLETE
                                matchtype = BACKTRACK;

                                // ******************************************************
                                // DEBUG ONLY AREA
                                // ******************************************************
#ifdef RULEDEBUG
                                printf("BACKTRACKED UP");
                                printf("\n");
                                fflush(stdout);
#endif
                                // ******************************************************
                                //      END DEBUG ONLY AREA
                                // ******************************************************

                                break;
                            }

                        }
                    }
                    else {
                        // NON-COMMUTATIVE MATCH BACKTRACK FROM A SNAPSHOT, THIS SHOULD NEVER HAPPEN (???)

                        // ******************************************************
                        // DEBUG ONLY AREA
                        // ******************************************************
#ifdef RULEDEBUG
                        printf("ERROR IN BACKTRACK: %d/%d,%d/%d", s.leftidx,
                                s.leftnargs, s.rightidx, s.rightnargs);
                        if((s.leftidx > 0) && (s.leftidx <= s.leftnargs)) {
                            word_p string =
                                    rplDecompile(FINDARGUMENT(s.left,
                                        s.leftnargs, s.leftidx),
                                    DECOMP_EDIT | DECOMP_NOHINTS);
                            if(string) {
                                BYTE strbyte[1024];
                                memmoveb(strbyte, (byte_p) (string + 1),
                                        rplStrSize(string));
                                strbyte[rplStrSize(string)] = 0;
                                printf("  left=%s", strbyte);
                            }
                        }
                        if((s.rightidx > 0) && (s.rightidx <= s.rightnargs)) {
                            word_p string =
                                    rplDecompile(FINDARGUMENT(s.right,
                                        s.rightnargs, s.rightidx),
                                    DECOMP_EDIT | DECOMP_NOHINTS);
                            if(string) {
                                BYTE strbyte[1024];
                                memmoveb(strbyte, (byte_p) (string + 1),
                                        rplStrSize(string));
                                strbyte[rplStrSize(string)] = 0;
                                printf("  right=%s", strbyte);
                            }
                        }
                        printf("\n");
                        fflush(stdout);
#endif
                        // ******************************************************
                        //      END DEBUG ONLY AREA
                        // ******************************************************

                        break;

                    }
                }
            }
            else {
                // NO MORE SNAPSHOTS, THIS IS A STANDARD BACKTRACK ON THE MAIN THREAD
                if(s.leftidx < s.leftnargs) {

                    // ADVANCE TO THE NEXT ARGUMENT

                    ++s.leftidx;
                    // RECURSE INTO THE ARGUMENT
                    updateCounters(&s);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    // PUSH THE LEFT
                    rplSymbExplodeOneLevel2(FINDARGUMENT(s.left, s.leftnargs,
                                s.leftidx));
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    // PUSH THE RIGHT
                    rplSymbExplodeOneLevel2(*rule + ruleleftoffset);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    s.right = DSTop - 1;

                    // LEFTARG AND RIGHTARG
                    rplNewint32_tPush(((int64_t) s.leftdepth + 1) << 32, DECint32_t);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    rplPushData((word_p) zero_bint);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    rplPushData((word_p) zero_bint);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    rplPushData((word_p) zero_bint);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    s.nlams = 0;

                    baselevel = DSTop - DStkBottom;
                    matchtype = OPMATCH;
                    matchstarted = 0;

                    // ******************************************************
                    // DEBUG ONLY AREA
                    // ******************************************************
#ifdef RULEDEBUG
                    printf("BACKTRACKED: %d/%d,%d/%d", s.leftidx, s.leftnargs,
                            s.rightidx, s.rightnargs);
                    if((s.leftidx > 0) && (s.leftidx <= s.leftnargs)) {
                        word_p string =
                                rplDecompile(FINDARGUMENT(s.left, s.leftnargs,
                                    s.leftidx), DECOMP_EDIT | DECOMP_NOHINTS);
                        if(string) {
                            BYTE strbyte[1024];
                            memmoveb(strbyte, (byte_p) (string + 1),
                                    rplStrSize(string));
                            strbyte[rplStrSize(string)] = 0;
                            printf("  left=%s", strbyte);
                        }
                    }
                    if((s.rightidx > 0) && (s.rightidx <= s.rightnargs)) {
                        word_p string =
                                rplDecompile(FINDARGUMENT(s.right, s.rightnargs,
                                    s.rightidx), DECOMP_EDIT | DECOMP_NOHINTS);
                        if(string) {
                            BYTE strbyte[1024];
                            memmoveb(strbyte, (byte_p) (string + 1),
                                    rplStrSize(string));
                            strbyte[rplStrSize(string)] = 0;
                            printf("  right=%s", strbyte);
                        }
                    }
                    printf("\n");
                    fflush(stdout);
#endif
                    // ******************************************************
                    //      END DEBUG ONLY AREA
                    // ******************************************************

                    // CLEAN THE PREVIOUS ENVIRONMENT
                    rplCleanupLAMs(0);

                    // CREATE LAM ENVIRONMENT
                    rplCreateLAMEnvironment(*orgrule);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    rplCreateLAM((word_p) nulllam_ident, (word_p) zero_bint);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }

                }

                else {
                    // ALL ARGUMENTS DONE
                    // KEEP BACKTRACKING
                    DSTop = s.left - ((s.leftnargs) ? (1 + s.leftnargs) : 0);   // DROP ENTIRE LEVEL

                    // ******************************************************
                    // DEBUG ONLY AREA
                    // ******************************************************
#ifdef RULEDEBUG
                    printf("BACKTRACKED UP");
                    printf("\n");
                    fflush(stdout);
#endif
                    // ******************************************************
                    //      END DEBUG ONLY AREA
                    // ******************************************************
                }
            }
            break;
        }
        case ARGDONE:
        {
            reloadPointers(DSTop, &s);

            // ******************************************************
            // DEBUG ONLY AREA
            // ******************************************************
#ifdef RULEDEBUG
            printf("ARGDONE: %d/%d,%d/%d [%d,%d]", s.leftidx, s.leftnargs,
                    s.rightidx, s.rightnargs, s.leftdepth, s.rightdepth);
            if((s.leftidx > 0) && (s.leftidx <= s.leftnargs)) {
                word_p string =
                        rplDecompile(FINDARGUMENT(s.left, s.leftnargs,
                            s.leftidx), DECOMP_EDIT | DECOMP_NOHINTS);
                if(string) {
                    BYTE strbyte[1024];
                    memmoveb(strbyte, (byte_p) (string + 1),
                            rplStrSize(string));
                    strbyte[rplStrSize(string)] = 0;
                    printf("  left=%s", strbyte);
                }
            }
            if((s.rightidx > 0) && (s.rightidx <= s.rightnargs)) {
                word_p string =
                        rplDecompile(FINDARGUMENT(s.right, s.rightnargs,
                            s.rightidx), DECOMP_EDIT | DECOMP_NOHINTS);
                if(string) {
                    BYTE strbyte[1024];
                    memmoveb(strbyte, (byte_p) (string + 1),
                            rplStrSize(string));
                    strbyte[rplStrSize(string)] = 0;
                    printf("  right=%s", strbyte);
                }
            }
            printf("\n");
            fflush(stdout);
#endif
            // ******************************************************
            //      END DEBUG ONLY AREA
            // ******************************************************

            ++s.leftidx;
            ++s.rightidx;

            if(s.leftidx <= s.leftnargs) {
                if(s.rightidx > s.rightnargs) {
                    // THERE'S EXTRA ARGUMENTS IN THE LEFT PART
                    if((**s.left == CMD_OVR_ADD) || (**s.left == CMD_OVR_MUL)) {

                        if(baselevel == DSTop - DStkBottom) {
                            // WE HAVE A COMPLETE MATCH
                            rplSymbReplaceMatchHere(rule,
                                    s.leftidx - s.rightnargs);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }

                            // ******************************************************
                            // DEBUG ONLY AREA
                            // ******************************************************
#ifdef RULEDEBUG
                            printf("REPLACED (EXTRA LEFT): ");
                            word_p string =
                                    rplDecompile(*DStkBottom,
                                    DECOMP_EDIT | DECOMP_NOHINTS);
                            if(string) {
                                BYTE strbyte[1024];
                                memmoveb(strbyte, (byte_p) (string + 1),
                                        rplStrSize(string));
                                strbyte[rplStrSize(string)] = 0;
                                printf("  expression=%s", strbyte);
                            }
                            printf("\n");
                            fflush(stdout);
#endif
                            // ******************************************************
                            //      END DEBUG ONLY AREA
                            // ******************************************************
                            // UPDATE ALL POINTERS AS THE EXPRESSION MOVED IN THE STACK
                            baselevel = DSTop - DStkBottom;
                            reloadPointers(DSTop, &s);

                            // STORE THE PART OF THE EXPRESSION BEING REPLACED
                            {
                                TRACK_STATE p;
                                reloadPointers(s.left - ((s.leftnargs) ? (1 +
                                                s.leftnargs) : 0), &p);

                                if((p.leftidx > 0)
                                        && (p.leftidx <= p.leftnargs))
                                    rplPutLAMn(1, FINDARGUMENT(p.left,
                                                p.leftnargs, p.leftidx));
                                else
                                    rplPutLAMn(1, *p.left);

                            }
                            // CREATE LAM ENVIRONMENT
                            rplCreateLAMEnvironment(*orgrule);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }
                            rplCreateLAM((word_p) nulllam_ident,
                                    (word_p) zero_bint);
                            if(Exceptions) {
                                rplCleanupSnapshots(stkbottom);
                                DSTop = expression;
                                LAMTop = lamsave;
                                nLAMBase = lamcurrent;
                                return 0;
                            }
                            s.rightidx = s.rightnargs ? 1 : 0;

                            s.lrotbase = s.leftrot = 0;
                            // UPDATE IDX COUNT TO RESET RIGHT SIDE
                            updateCounters(&s);

                            matchtype = RESTARTMATCH;

                        }
                        else {
                            // IT'S AN INNER OPERATION
                            if(s.lrotbase) {

                                // GET POINTERS TO THE PARENT EXPRESSION
                                TRACK_STATE p;
                                reloadPointers(s.left - ((s.leftnargs) ? (1 +
                                                s.leftnargs) : 0), &p);

                                // COPY THE ROTATED ARGUMENTS FROM THE LEFT INTO THE UPPER LEVEL
                                // TO MAKE SURE THEY ARE PICKED UP ON REPLACEMENT

                                int32_t k;
                                // COPY ALL ARGUMENTS TO THE UPPER ENVIRONMENT
                                for(k = 1; k <= p.leftnargs; ++k)
                                    FINDARGUMENT(p.left, p.leftnargs, k) =
                                            FINDARGUMENT(s.left, s.leftnargs,
                                            k);

                                // ALL ARGUMENTS ARE DONE, PASS IT TO THE UPPER LEVEL
                                DSTop = s.left - ((s.leftnargs) ? (1 +
                                            s.leftnargs) : 0);
                                // ADVANCE UPPER LEVEL POINTERS
                                p.leftidx = s.leftidx - 1;
                                p.rightidx = s.rightidx - 1;
                                updateCounters(&p);
                                if(Exceptions) {
                                    rplCleanupSnapshots(stkbottom);
                                    DSTop = expression;
                                    LAMTop = lamsave;
                                    nLAMBase = lamcurrent;
                                    return 0;
                                }

                                // ******************************************************
                                // DEBUG ONLY AREA
                                // ******************************************************
#ifdef RULEDEBUG
                                printf("ARGDONE UP");
                                printf("\n");
                                fflush(stdout);
#endif
                                // ******************************************************
                                //      END DEBUG ONLY AREA
                                // ******************************************************

                            }
                            else {
                                // EXTRA ARGUMENTS MEANS NO MATCH
                                matchtype = BACKTRACK;
                            }
                        }
                    }
                    else
                        matchtype = BACKTRACK;

                }
                else {
                    // WE HAVE MORE ARGUMENTS IN THE RIGHT PART, KEEP COMPARING

                    if(s.lrotbase && (s.leftidx > s.lrotbase)
                            && (s.leftidx < s.leftnargs)) {

                        // KEEP PARENT UP TO DATE IN THE STACK
                        updateCounters(&s);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }
                        updateLAMs(&s);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }

                        // CREATE A COPY OF THE ENTIRE LEVEL TO DO A SUB-ROTATION
                        word_p *topoflevel =
                                s.left - ((s.leftnargs) ? (1 +
                                    s.leftnargs) : 0);
                        rplExpandStack(DSTop - topoflevel);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }
                        int32_t k;
                        for(k = 0; k < DSTop - topoflevel; ++k)
                            DSTop[k] = topoflevel[k];
                        s.left += k;
                        s.right += k;
                        DSTop += k;
                        s.leftrot = 0;
                        s.lrotbase = s.leftidx;

                        updateCounters(&s);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }

                        rplTakeSnapshot();
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }

                    }
                    updateCounters(&s);
                    if(Exceptions) {
                        rplCleanupSnapshots(stkbottom);
                        DSTop = expression;
                        LAMTop = lamsave;
                        nLAMBase = lamcurrent;
                        return 0;
                    }
                    matchtype = ARGMATCH;       // KEEP MATCHING ARGUMENTS
                }
            }
            else {
                // NO MORE ARGUMENT ON THE LEFT SIDE
                if(s.rightidx <= s.rightnargs) {
                    // WE NEED MORE ARGUMENTS! THIS IS NOT A MATCH
                    matchtype = BACKTRACK;
                }
                else {
                    // WE FINISHED ALL THE ARGUMENTS IN THIS LEVEL, PASS IT BACK TO UPPER LEVEL UNLESS THIS IS THE BASE LEVEL
                    if(baselevel == DSTop - DStkBottom) {
                        // WE HAVE A COMPLETE MATCH
                        // STORE THE PART OF THE EXPRESSION BEING REPLACED
                        {
                            // GET POINTERS TO THE PARENT EXPRESSION
                            TRACK_STATE p;
                            reloadPointers(s.left - ((s.leftnargs) ? (1 +
                                            s.leftnargs) : 0), &p);

                            if((p.leftidx > 0) && (p.leftidx <= p.leftnargs))
                                rplPutLAMn(1, FINDARGUMENT(p.left, p.leftnargs,
                                            p.leftidx));
                            else
                                rplPutLAMn(1, *p.left);
                        }
                        if((s.leftidx - s.rightnargs >= 1)
                                && (s.leftidx - s.rightnargs < s.leftnargs))
                            rplSymbReplaceMatchHere(rule,
                                    s.leftidx - s.rightnargs);
                        else
                            rplSymbReplaceMatchHere(rule, 1);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }
                        // ******************************************************
                        // DEBUG ONLY AREA
                        // ******************************************************
#ifdef RULEDEBUG
                        printf("REPLACED (EXACT): ");
                        word_p string =
                                rplDecompile(*DStkBottom,
                                DECOMP_EDIT | DECOMP_NOHINTS);
                        if(string) {
                            BYTE strbyte[1024];
                            memmoveb(strbyte, (byte_p) (string + 1),
                                    rplStrSize(string));
                            strbyte[rplStrSize(string)] = 0;
                            printf("  expression=%s", strbyte);
                        }
                        printf("\n");
                        fflush(stdout);
#endif
                        // ******************************************************
                        //      END DEBUG ONLY AREA
                        // ******************************************************

                        // UPDATE ALL POINTERS AS THE EXPRESSION MOVED IN THE STACK
                        baselevel = DSTop - DStkBottom;
                        reloadPointers(DSTop, &s);

                        // CREATE LAM ENVIRONMENT
                        rplCreateLAMEnvironment(*orgrule);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }
                        rplCreateLAM((word_p) nulllam_ident,
                                (word_p) zero_bint);
                        if(Exceptions) {
                            rplCleanupSnapshots(stkbottom);
                            DSTop = expression;
                            LAMTop = lamsave;
                            nLAMBase = lamcurrent;
                            return 0;
                        }

                        matchtype = RESTARTMATCH;

                    }
                    else {
                        // IF THIS WAS A SUB-ENVIRONMENT FOR A ROT, CLEANUP

                        if(s.lrotbase) {

                            // COPY THE ROTATED ARGUMENTS FROM THE LEFT INTO THE UPPER LEVEL
                            // TO MAKE SURE THEY ARE PICKED UP ON REPLACEMENT
                            {
                                // GET POINTERS TO THE PARENT EXPRESSION
                                TRACK_STATE p;
                                reloadPointers(s.left - ((s.leftnargs) ? (1 +
                                                s.leftnargs) : 0), &p);

                                int32_t k;
                                // COPY ALL ARGUMENTS TO THE UPPER ENVIRONMENT
                                for(k = 1; k <= p.leftnargs; ++k)
                                    FINDARGUMENT(p.left, p.leftnargs, k) =
                                            FINDARGUMENT(s.left, s.leftnargs,
                                            k);
                                k = p.leftnargs - s.leftnargs;
                                if(k > 0)
                                    rplRemoveAtData(DSTop -
                                            &FINDARGUMENT(p.left, s.leftnargs,
                                                s.leftnargs) - 1, k);
                                p.left -= k;    // REMOVE ADDITIONAL ARGUMENTS IF NEEDED

                                p.right -= k;
                                s.left -= k;
                                s.right -= k;
                                if(baselevel > p.left - DStkBottom)
                                    baselevel -= k;

                                // AND UPDATE THE INDEX OF THE PARENT EXPRESSION TOO
                                p.leftidx = s.leftidx - 1;
                                p.rightidx = s.rightidx - 1;
                                updateCounters(&p);

                                if(Exceptions) {
                                    rplCleanupSnapshots(stkbottom);
                                    DSTop = expression;
                                    LAMTop = lamsave;
                                    nLAMBase = lamcurrent;
                                    return 0;
                                }

                            }

                        }
                    }

                    // ALL ARGUMENTS ARE DONE, PASS IT TO THE UPPER LEVEL
                    DSTop = s.left - ((s.leftnargs) ? (1 + s.leftnargs) : 0);
                    // ******************************************************
                    // DEBUG ONLY AREA
                    // ******************************************************
#ifdef RULEDEBUG
                    printf("ARGDONE UP");
                    printf("\n");
                    fflush(stdout);
#endif
                    // ******************************************************
                    //      END DEBUG ONLY AREA
                    // ******************************************************

                }
            }
            break;
        }

        case ARGDONEEXTRA:
        {
            reloadPointers(DSTop, &s);
            // ******************************************************
            // DEBUG ONLY AREA
            // ******************************************************
#ifdef RULEDEBUG
            printf("ARGDONEEXTRA: %d/%d,%d/%d [%d,%d]", s.leftidx, s.leftnargs,
                    s.rightidx, s.rightnargs, s.leftdepth, s.rightdepth);
            if((s.leftidx > 0) && (s.leftidx <= s.leftnargs)) {
                word_p string =
                        rplDecompile(FINDARGUMENT(s.left, s.leftnargs,
                            s.leftidx), DECOMP_EDIT | DECOMP_NOHINTS);
                if(string) {
                    BYTE strbyte[1024];
                    memmoveb(strbyte, (byte_p) (string + 1),
                            rplStrSize(string));
                    strbyte[rplStrSize(string)] = 0;
                    printf("  left=%s", strbyte);
                }
            }
            if((s.rightidx > 0) && (s.rightidx <= s.rightnargs)) {
                word_p string =
                        rplDecompile(FINDARGUMENT(s.right, s.rightnargs,
                            s.rightidx), DECOMP_EDIT | DECOMP_NOHINTS);
                if(string) {
                    BYTE strbyte[1024];
                    memmoveb(strbyte, (byte_p) (string + 1),
                            rplStrSize(string));
                    strbyte[rplStrSize(string)] = 0;
                    printf("  right=%s", strbyte);
                }
            }
            printf("\n");
            fflush(stdout);
#endif
            // ******************************************************
            //      END DEBUG ONLY AREA
            // ******************************************************

            // TODO: HANDLE THE CASE OF PARTIAL MATCH
            // ALL ARGUMENTS ARE DONE, PASS IT TO THE UPPER LEVEL
            DSTop = s.left - ((s.leftnargs) ? (1 + s.leftnargs) : 0);
            break;
        }

            // ADD MORE STATES HERE

        }       // END OF SWITCH

    }
    while(DSTop > marker);

    if((matchtype == BACKTRACK) || (matchtype == RESTARTMATCH))
        rplCleanupLAMs(0);      // THE LAST BACKTRACK ENDED IN FAILURE, DISCARD THE ENVIRONMENT
// COPY THE RESULTING EXPRESSION
    *expression = *DStkBottom;
    rplDropCurrentStack();
    DSTop = expression + 1;     // KEEP THE RESULTING EXPRESSION ON THE STACK

// COUNT HOW MANY RESULTS WERE FOUND
    int32_t found = 0;
    word_p *lamenv = LAMTop;
    while(lamenv > lamsave) {
        lamenv = rplGetNextLAMEnv(lamenv);
        if(lamenv)
            ++found;
    }

    return found;
}

// ATTRIBUTES OF AN IDENTIFIER FOR THE CAS:
// IF NO ATTRIBUTE IS ADDED TO THE NAME OF THE VARIABLE, A VARIABLE IS CONSIDERED FINITE AND REAL (OR FINITE AND COMPLEX IF COMPLEX MODE IS ENABLED)

// ATTRIBUTES HAVE 2 DIGITS (MINIMUM): VARnmp
// WHERE nmp ARE SUBSCRIPT ATTRIBUTE DIGITS (0-9), AND NOT PART OF THE VARIABLE NAME

// n=0 --> THERE'S NO HINT ABOUT TYPE OF VARIABLE
// n=1 --> VARIABLE IC KNOWN NOT TO BE INFINITE (REAL OR COMPLEX)
// n=2 --> VARIABLE IS KNOWN TO BE REAL BUT MAY BE INFINITE
// n=3 --> VARIABLE IS KNOWN TO BE REAL (EVEN IF COMPLEX MODE IS ON)
// n=4 --> VARIABLE IS KNOWN TO BE COMPLEX BUT MAY BE ALSO INFINITE
// n=5 --> VARIABLE IS KNOWN TO BE COMPLEX (EVEN IF COMPLEX MODE IS OFF)
// n=6 --> VARIABLE IS A MATRIX
// n=8 --> INTERNALLY UNKNOWN RESULT

// FOR REAL VARIABLES AND REALS:
// m=0 --> NO HINTS REGARDING SIGN (COULD BE ANYTHING)
// m=1 --> VARIABLE CANNOT BE ZERO (COMPLEX OR REAL)
// m=2 --> VARIABLE CANNOT BE <0 (FOR REAL)
// m=3 --> VARIABLE CANNOT BE <0 AND CANNOT BE 0
// m=4 --> VARIABLE CANNOT BE >0 (FOR REAL)
// m=5 --> VARIABLE CANNOT BE >0 AND CANNOT BE 0

// ADDITIONAL ATTRIBUTES FOR REALS:
// p=0 --> NO HINTS REGARDING INTEGER STATUS
// p=1 --> VARIABLE IS KNOWN TO BE INTEGER
// p=2 --> VARIABLE IS KNOWN TO BE ODD
// p=3 --> VARIABLE IS KNOWN TO BE AN ODD INTEGER
// p=4 --> VARIABLE IS KNOWN TO BE EVEN
// p=5 --> VARIABLE IS KNOWN TO BE AN EVEN INTEGER

// USING ATTRIBUTES IN RULES:
// IF A SPECIAL VARIABLE '.vX' HAS ATTRIBUTES, IT WILL ONLY MATCH VARIABLES THAT HAVE THE EXACT SAME ATTRIBUTES.
// IF A SPECIAL VARIABLE MATCHES AN ENTIRE SUB-EXPRESSION AND HAS ATTRIBUTES, THE ATTRIBUTES ARE COMPUTED AS FOLLOWS:

// IF A KNOWN OPERATION INVOLVES A MATRIX (+, *, ^ THE USUAL OPERATORS), THE RESULT IS A MATRIX (n=6)
// IF AN OPERATION INVOLVES A COMPLEX, THE RESULT WILL BE COMPLEX (n=4)
// IF AN OPERATION INVOLVES ANY VARIABLE WITH BIT 1 SET (n=odd), THE RESULT WILL HAVE BIT 1 SET (POSSIBLY INFINITE RESULT)
// UNKNOWN/CUSTOM COMMANDS OR FUNCTIONS WILL SET n=8 ON THE RESULT

// THE ATTRIBUTE OF THE SPECIAL VARIABLE AND THE MATCHING EXPRESSION MUST BE IDENTICAL.

// COMPUTE THE RESULTING ATTRIBUTES OF A SYMBOLIC EXPRESSION
int32_t rplSymbCombineAttr(WORD operator, int32_t rattr, int32_t attr)
{
    // NOW COMBINE THE ATTRIBUTES BASED ON THE TYPE OF OPERATOR

    // SET DEFAULT ATTRIBUTES FOR VARIABLES THAT DON'T HAVE THEM (REAL IN REAL MODE, COMPLEX IN COMPLEX MODE)
    if(!attr || !rattr) {
        if(rplTestSystemFlag(FL_COMPLEXMODE)) {
            if(!attr)
                attr = IDATTR_ISINFCPLX;
            if(!rattr)
                rattr = IDATTR_ISINFCPLX;
        }
        else {
            if(!attr)
                attr = IDATTR_ISINFREAL;
            if(!rattr)
                rattr = IDATTR_ISINFREAL;
        }
    }

    if(operator) {
        switch (operator) {
        case CMD_OVR_ADD:
        {
            if(rattr == -1) {
                rattr = attr;   // THIS IS THE FIRST ARGUMENT, JUST COPY THE ATTRIBUTES
                break;
            }

            switch (rattr & IDATTR_nMASK) {
            case IDATTR_ISREAL:
            case IDATTR_ISINFREAL:
                // ADDING/SUBTRACTING REALS

                if((attr & IDATTR_nMASK) > IDATTR_ISREAL) {
                    rattr = attr;       // HIGHER TYPES TAKE PRECEDENCE (COMPLEX+REAL = COMPLEX, MATRIX + REAL/CPLX = MATRIX (REAL/CPLX k IS ASSUMED AS k*I)
                    break;
                }
                // WE ARE ADDING REAL TO REALS
                if(!(attr & IDATTR_ISNOTINF))
                    rattr &= ~IDATTR_ISNOTINF;  // IF THE NEW ARGUMENT CAN BE INFINITE, SO CAN THE RESULT
                if((rattr & IDATTR_GTEZERO) && (attr & IDATTR_GTEZERO)) {
                    rattr |= IDATTR_GTEZERO | (attr & IDATTR_NOTZERO);  // X>=0, Y>=0 MEANS X+Y>=0 AND IF ANY OF THE TWO >0, THEN X+Y>0
                }
                else if((rattr & IDATTR_LTEZERO) && (attr & IDATTR_LTEZERO)) {
                    rattr |= IDATTR_LTEZERO | (attr & IDATTR_NOTZERO);  // X<=0, Y<=0 MEANS X+Y<=0 AND IF ANY OF THE TWO <0, THEN X+Y<0
                }
                else
                    rattr &= ~(IDATTR_LTEZERO | IDATTR_GTEZERO | IDATTR_NOTZERO);       // ALL OTHER CASES MEANS X+Y COULD BE ANYTHING

                rattr = (rattr & (~IDATTR_INTEGER)) | (rattr & attr & IDATTR_INTEGER);  // RESULT IS INTEGER ONLY IF BOTH ARE INTEGERS

                if(!(rattr & (IDATTR_ODD | IDATTR_EVEN))
                        || !(attr & (IDATTR_ODD | IDATTR_EVEN)))
                    rattr &= ~(IDATTR_ODD | IDATTR_EVEN);       // IF THERE'S NO ODD/EVEN INFO ON ONE OF THE NUMBERS, REMOVE IT FROM THE RESULT AS WELL
                else {
                    // BOTH NUMBERS ARE ODD/EVEN, LET'S ADD THEM
                    if(rattr & attr & IDATTR_ODD)
                        rattr |= IDATTR_EVEN;   //  RESULT IS EVEN IF BOTH NUMBERS ARE ODD
                    else
                        rattr = (rattr & (~IDATTR_EVEN)) | ((rattr & attr) & IDATTR_EVEN);      // RESULT IS EVEN ONLY IF BOTH NUMBERS ARE EVEN

                    rattr = (rattr & (~IDATTR_ODD)) | ((rattr ^ attr) & IDATTR_ODD);    // RESULT IS ODD ONLY IF ONE OF THEM IS ODD

                }

                break;

            case IDATTR_ISCPLX:
            case IDATTR_ISINFCPLX:

                if((attr & IDATTR_nMASK) > IDATTR_ISCPLX) {
                    rattr = attr;       // HIGHER TYPES TAKE PRECEDENCE
                    break;
                }
                // WE ARE ADDING COMPLEX TO COMPLEX OR COMPLEX+REAL
                if(!(attr & IDATTR_ISNOTINF))
                    rattr &= ~IDATTR_ISNOTINF;  // IF THE NEW ARGUMENT CAN BE INFINITE, SO CAN THE RESULT

                rattr &= ~IDATTR_mMASK; // REMOVE ALL OTHER HINTS, ADDING 2 COMPLEX NUMBERS TELLS US NOTHING ABOUT THE RESULT

                break;

            case IDATTR_ISMATRIX:

                if((attr & IDATTR_nMASK) > IDATTR_ISMATRIX) {
                    rattr = attr;       // HIGHER TYPES TAKE PRECEDENCE
                    break;
                }

                // TODO: POSSIBLY ADD ATTRIBUTES FOR DIAGONAL MATRICES, ETC.

                rattr &= ~IDATTR_mMASK; // REMOVE ALL OTHER HINTS, ADDING MATRICES TELLS US NOTHING ABOUT THE RESULT

                break;

            default:
                rattr = IDATTR_ISUNKNOWN;
                break;
            }

            break;
        }

        case CMD_OVR_MUL:
        {
            if(rattr == -1) {
                rattr = attr;   // THIS IS THE FIRST ARGUMENT, JUST COPY THE ATTRIBUTES
                break;
            }

            switch (rattr & IDATTR_nMASK) {
            case IDATTR_ISREAL:
            case IDATTR_ISINFREAL:
                // MULTIPLYING REALS

                if((attr & IDATTR_nMASK) > IDATTR_ISREAL) {
                    rattr = attr;       // HIGHER TYPES TAKE PRECEDENCE (COMPLEX+REAL = COMPLEX, MATRIX + REAL/CPLX = MATRIX (REAL/CPLX k IS ASSUMED AS k*I)
                    break;
                }
                // WE ARE ADDING REAL TO REALS
                if(!(attr & IDATTR_ISNOTINF))
                    rattr &= ~IDATTR_ISNOTINF;  // IF THE NEW ARGUMENT CAN BE INFINITE, SO CAN THE RESULT
                // AND IF BOTH X AND Y ARE NON-ZERO, THEN X*Y IS NON-ZERO
                rattr = (rattr & ~IDATTR_NOTZERO) | (rattr & attr &
                        IDATTR_NOTZERO);

                if((rattr & IDATTR_GTEZERO) && (attr & IDATTR_GTEZERO)) {
                    // X>=0, Y>=0 MEANS X*Y>=0   WHICH IS ALREADY THE CASE
                }
                else if((rattr & IDATTR_LTEZERO) && (attr & IDATTR_LTEZERO)) {
                    rattr &= ~IDATTR_LTEZERO;
                    rattr |= IDATTR_GTEZERO;    // X<=0, Y<=0 MEANS X*Y>=0
                }
                else if((rattr & IDATTR_GTEZERO) && (attr & IDATTR_LTEZERO)) {
                    rattr &= ~IDATTR_GTEZERO;
                    rattr |= IDATTR_LTEZERO;    // X>=0, Y<=0 MEANS X*Y<=0
                }
                else if((rattr & IDATTR_LTEZERO) && (attr & IDATTR_GTEZERO)) {
                    // X<=0, Y>=0 MEANS X*Y<=0  WHICH IS ALREADY THE CASE
                }
                else
                    rattr &= ~(IDATTR_LTEZERO | IDATTR_GTEZERO);        // ALL OTHER CASES MEANS X*Y COULD BE ANYTHING

                rattr = (rattr & (~IDATTR_INTEGER)) | (rattr & attr & IDATTR_INTEGER);  // RESULT IS INTEGER ONLY IF BOTH ARE INTEGERS

                if(!(rattr & (IDATTR_ODD | IDATTR_EVEN))
                        || !(attr & (IDATTR_ODD | IDATTR_EVEN)))
                    rattr &= ~(IDATTR_ODD | IDATTR_EVEN);       // IF THERE'S NO ODD/EVEN INFO ON ONE OF THE NUMBERS, REMOVE IT FROM THE RESULT AS WELL
                else {
                    if(rattr & attr & IDATTR_ODD)
                        rattr |= IDATTR_EVEN;   //  RESULT IS EVEN IF BOTH NUMBERS ARE ODD
                    else
                        rattr = (rattr & (~IDATTR_EVEN)) | ((rattr | attr) & IDATTR_EVEN);      // RESULT IS EVEN ONLY IF BOTH NUMBERS ARE EVEN

                    rattr = (rattr & (~IDATTR_ODD)) | ((rattr & attr) & IDATTR_ODD);    // RESULT IS ODD ONLY IF BOTH OF THEM ARE ODD
                }

                break;

            case IDATTR_ISCPLX:
            case IDATTR_ISINFCPLX:

                if((attr & IDATTR_nMASK) > IDATTR_ISCPLX) {
                    rattr = attr;       // HIGHER TYPES TAKE PRECEDENCE
                    break;
                }
                // WE ARE MULTIPLYING COMPLEX TO COMPLEX OR COMPLEX*REAL
                if(!(attr & IDATTR_ISNOTINF))
                    rattr &= ~IDATTR_ISNOTINF;  // IF THE NEW ARGUMENT CAN BE INFINITE, SO CAN THE RESULT
                // AND IF BOTH X AND Y ARE NON-ZERO, THEN X*Y IS NON-ZERO
                rattr = (rattr & ~IDATTR_NOTZERO) | (rattr & attr &
                        IDATTR_NOTZERO);

                rattr &= ~IDATTR_NOTZERO;       // REMOVE ALL OTHER HINTS, MULTIPLYING 2 COMPLEX NUMBERS TELLS US NOTHING ABOUT THE RESULT

                break;

            case IDATTR_ISMATRIX:

                if((attr & IDATTR_nMASK) > IDATTR_ISMATRIX) {
                    rattr = attr;       // HIGHER TYPES TAKE PRECEDENCE
                    break;
                }

                // TODO: POSSIBLY ADD ATTRIBUTES FOR DIAGONAL MATRICES, ETC.

                rattr &= ~IDATTR_mMASK; // REMOVE ALL OTHER HINTS, MULTIPLYING MATRICES TELLS US NOTHING ABOUT THE RESULT

                break;

            default:
                rattr = IDATTR_ISUNKNOWN;
                break;
            }

            break;
        }

        case CMD_OVR_NEG:
        case CMD_OVR_UMINUS:
        {
            rattr = attr;

            switch (rattr & IDATTR_nMASK) {
            case IDATTR_ISREAL:
            case IDATTR_ISINFREAL:
                // NEGATING A REAL NUMBER

                if((rattr & IDATTR_GTEZERO)) {
                    // X>=0,  MEANS -X<=0
                    rattr &= ~IDATTR_GTEZERO;
                    rattr |= IDATTR_LTEZERO;

                }
                else if((rattr & IDATTR_LTEZERO)) {
                    rattr &= ~IDATTR_LTEZERO;
                    rattr |= IDATTR_GTEZERO;    // X<=0, MEANS -X>=0
                }

                break;

            case IDATTR_ISCPLX:
            case IDATTR_ISINFCPLX:
            case IDATTR_ISMATRIX:
            default:
                break;
            }

            break;
        }

        case CMD_OVR_UPLUS:

            // DO ABSOLUTELY NOTHING
            rattr = attr;

            break;

        case CMD_OVR_POW:
        {
            if(rattr == -1) {
                rattr = attr;
                break;
            }

            switch (rattr & IDATTR_nMASK) {
            case IDATTR_ISREAL:
            case IDATTR_ISINFREAL:
                // REAL X, computing X^a

                if((attr & IDATTR_nMASK) > IDATTR_ISCPLX) {
                    rattr = IDATTR_ISUNKNOWN;   // CAN ONLY USE REALS OR COMPLEX AS EXPONENTS
                    break;
                }
                if((attr & IDATTR_nMASK) > IDATTR_ISREAL) {
                    rattr = IDATTR_ISINFCPLX;   // COMPLEX EXPONENT ALWAYS RESULTS IN COMPLEX RESULT
                    break;
                }

                // WE HAVE REAL BASE AND REAL EXPONENTS
                if(!(attr & IDATTR_ISNOTINF))
                    rattr &= ~IDATTR_ISNOTINF;  // IF THE NEW ARGUMENT CAN BE INFINITE, SO CAN THE RESULT

                rattr = (rattr & (~IDATTR_INTEGER)) | (rattr & attr & IDATTR_INTEGER);  // RESULT IS INTEGER ONLY IF BOTH ARE INTEGERS

                // INTEGER RESULTS KEEP THEIR ODD/INTEGER PROPERTIES REGARDLESS OF EXPONENT
                if(!(rattr & IDATTR_INTEGER))
                    rattr &= ~(IDATTR_ODD | IDATTR_EVEN);       // IF THE RESULT IS NOT INTEGER, WE DON'T KNOW IF IT WILL BE ODD OR EVEN

                if(!(rattr & IDATTR_GTEZERO)) {
                    // NEGATIVE BASE
                    if(!(attr & IDATTR_INTEGER)) {
                        rattr = IDATTR_ISINFCPLX;       // NEGATIVE BASE TO A REAL EXPONENT IS IN GENERAL A COMPLEX NUMBER
                        break;
                    }
                }

                if((attr & IDATTR_pMASK) == (IDATTR_INTEGER | IDATTR_EVEN)) {
                    // EVEN INTEGER POWERS ALWAYS RESULT IN x^a >=0
                    rattr &= ~IDATTR_LTEZERO;
                    rattr |= IDATTR_GTEZERO;
                }
                else if((attr & IDATTR_pMASK) == (IDATTR_INTEGER | IDATTR_ODD)) {
                    // ODD INTEGER POWERS KEEP THEIR SIGN, DO NOTHING

                }
                else {
                    // IF WE DON'T KNOW IF IT'S ODD OR EVEN
                }

                break;

            case IDATTR_ISCPLX:
            case IDATTR_ISINFCPLX:

                // COMPLEX Z, computing Z^a

                if((attr & IDATTR_nMASK) > IDATTR_ISCPLX) {
                    rattr = IDATTR_ISUNKNOWN;   // CAN ONLY USE REALS OR COMPLEX AS EXPONENTS
                    break;
                }
                if((attr & IDATTR_nMASK) > IDATTR_ISREAL) {
                    break;      // COMPLEX EXPONENT ALWAYS RESULTS IN COMPLEX RESULT
                }

                // WE HAVE COMPLEX BASE AND REAL EXPONENTS
                if(!(attr & IDATTR_ISNOTINF))
                    rattr &= ~IDATTR_ISNOTINF;  // IF THE EXPONENT CAN BE INFINITE, SO CAN THE RESULT

                break;

            case IDATTR_ISMATRIX:

                if((attr & IDATTR_nMASK) > IDATTR_ISREAL) {
                    rattr = IDATTR_ISUNKNOWN;   // CAN ONLY USE REAL EXPONENTS FOR MATRICES
                    break;
                }

                // WE HAVE MATRIX BASE AND REAL EXPONENTS

                // TODO: DEAL WITH SPECIAL MATRICES, FOR NOW JUST KEEP THE RESULT AS A MATRIX

                break;

            default:
                rattr = IDATTR_ISUNKNOWN;
                break;
            }

            break;
        }

        case CMD_OVR_XROOT:
        {
            if(rattr == -1) {
                rattr = attr;
                break;
            }

            switch (rattr & IDATTR_nMASK) {
            case IDATTR_ISREAL:
            case IDATTR_ISINFREAL:
                // REAL X, computing X^(1/a)

                if((attr & IDATTR_nMASK) > IDATTR_ISCPLX) {
                    rattr = IDATTR_ISUNKNOWN;   // CAN ONLY USE REALS OR COMPLEX AS EXPONENTS
                    break;
                }
                if((attr & IDATTR_nMASK) > IDATTR_ISREAL) {
                    rattr = IDATTR_ISINFCPLX;   // COMPLEX EXPONENT ALWAYS RESULTS IN COMPLEX RESULT
                    break;
                }

                // WE HAVE REAL BASE AND REAL EXPONENTS
                if(!(attr & IDATTR_ISNOTINF))
                    rattr &= ~IDATTR_ISNOTINF;  // IF THE EXPONENT CAN BE INFINITE, SO CAN THE RESULT

                if(!rplTestSystemFlag(FL_COMPLEXMODE)) {
                    // COMPLEX MODE DISABLED

                    if((rattr & (IDATTR_GTEZERO | IDATTR_LTEZERO)) !=
                            IDATTR_GTEZERO) {
                        // COULD HAVE NEGATIVE BASE
                        rattr = IDATTR_ISINFCPLX;
                        break;
                    }
                }
                else {
                    // IN COMPLEX MODE, THE RESULT IS ALWAYS COMPLEX
                }

                // FORGET ANYTHING ELSE WE KNEW ABOUT THE BASE
                rattr &= ~(IDATTR_pMASK);

                break;

            case IDATTR_ISCPLX:
            case IDATTR_ISINFCPLX:

                // COMPLEX Z, computing Z^(1/a)

                if((attr & IDATTR_nMASK) > IDATTR_ISCPLX) {
                    rattr = IDATTR_ISUNKNOWN;   // CAN ONLY USE REALS OR COMPLEX AS EXPONENTS
                    break;
                }
                if((attr & IDATTR_nMASK) > IDATTR_ISREAL) {
                    break;      // COMPLEX EXPONENT ALWAYS RESULTS IN COMPLEX RESULT
                }

                // WE HAVE COMPLEX BASE AND REAL EXPONENTS
                if(!(attr & IDATTR_ISNOTINF))
                    rattr &= ~IDATTR_ISNOTINF;  // IF THE EXPONENT CAN BE INFINITE, SO CAN THE RESULT

                break;

            case IDATTR_ISMATRIX:

                if((attr & IDATTR_nMASK) > IDATTR_ISREAL) {
                    rattr = IDATTR_ISUNKNOWN;   // CAN ONLY USE REAL EXPONENTS FOR MATRICES
                    break;
                }

                // WE HAVE MATRIX BASE AND REAL EXPONENTS

                // TODO: DEAL WITH SPECIAL MATRICES, FOR NOW JUST KEEP THE RESULT AS A MATRIX

                break;

            default:
                rattr = IDATTR_ISUNKNOWN;
                break;
            }

            break;
        }

        case CMD_OVR_EQ:
        case CMD_OVR_NOTEQ:
        case CMD_OVR_GT:
        case CMD_OVR_LT:
        case CMD_OVR_GTE:
        case CMD_OVR_LTE:
        case CMD_OVR_OR:
        case CMD_OVR_AND:
        case CMD_OVR_NOT:
        case CMD_OVR_ISTRUE:
            // ALL TESTS RESULT IN 1/0 THEREFORE IT'S A REAL, NON-INFINITE, >=0 INTEGER.
            rattr = IDATTR_ISREAL | IDATTR_INTEGER | IDATTR_GTEZERO;
            break;

        case CMD_OVR_CMP:
            // CMP RESULTS IN -1/1/0 THEREFORE IT'S A REAL, NON-INFINITE, INTEGER.
            rattr = IDATTR_ISREAL | IDATTR_INTEGER;
            break;

        case CMD_OVR_INV:
        {
            rattr = attr;

            switch (rattr & IDATTR_nMASK) {
            case IDATTR_ISREAL:
            case IDATTR_ISINFREAL:
                // INVERSE OF A REAL NUMBER

                if(!(rattr & IDATTR_NOTZERO)) {
                    rattr |= IDATTR_ISINFREAL;  // RESULT COULD BE INFINITE UNLESS EXPRESSION IS MARKED AS NON-ZERO
                }
                rattr &= ~IDATTR_pMASK; // THE INVERSE OF AN INTEGER WON'T BE AN INTEGER. THE INVERSE OF A REAL IS NOT GUARANTEED TO BE AN INTEGER EITHER
                break;

            case IDATTR_ISCPLX:
            case IDATTR_ISINFCPLX:
                rattr |= IDATTR_ISINFCPLX;
                break;
            case IDATTR_ISMATRIX:
                // JUST LEAVE IT AS A MATRIX
                break;
            default:
                break;
            }

            break;
        }

        case CMD_OVR_ABS:
            // THE NUMBER IS A SCALAR REGARDLESS OF INPUT
            if(!(attr & IDATTR_ISNOTINF))
                rattr = IDATTR_ISINFREAL | IDATTR_GTEZERO;
            else
                rattr = IDATTR_ISREAL | IDATTR_GTEZERO;
            break;

        default:
            rattr = IDATTR_ISUNKNOWN;
            break;
            break;

        }

        // HERE WE HAVE rattr WITH THE RESULTING ATTRIBUTE OF THE OPERATION

    }
    else
        rattr = attr;
    return rattr;
}

int32_t rplSymbGetAttr(word_p object)
{
    int32_t rattr = -1, attr;
    word_p ptr, endofobj, finishedobject;
    WORD operator;
    word_p *stksave = DSTop;

    rplPushDataNoGrow(object);
    rplPushDataNoGrow(object);
    rplNewint32_tPush(rattr, DECint32_t);
    if(Exceptions) {
        DSTop = stksave;
        return 0;
    }
    operator= 0;
    finishedobject = 0;
    attr = -1;

    while(DSTop > stksave) {
        rattr = rplReadint32_t(rplPopData());
        ptr = rplPopData();
        ptr = rplSymbUnwrap(ptr);
        object = rplPopData();
        endofobj = rplSkipOb(object);
        operator= rplSymbMainOperator(object);
        if(finishedobject == object)
            continue;   // WE ARE ALREADY DONE WITH THIS LEVEL
        finishedobject = 0;
        if(attr != -1) {
            // IF WE ARE RESTORING FROM AN INNER OBJECT
            // WE NEED TO COMBINE THE ATTRIBUTES ACCORDING TO THE OPERATOR
            rattr = rplSymbCombineAttr(operator, rattr, attr);
            attr = -1;
            if(ptr >= endofobj) {
                // WE ARE DONE ALREADY
                finishedobject = object;
                attr = rattr;
                continue;
            }
        }

        if(ISSYMBOLIC(*ptr)) {

            rplPushDataNoGrow(object);
            rplPushDataNoGrow(rplSkipOb(ptr));
            ScratchPointer1 = ptr;
            rplNewint32_tPush(rattr, DECint32_t);
            if(Exceptions) {
                DSTop = stksave;
                return 0;
            }
            // RE-READ POINTERS FROM STACK IN CASE THEY MOVED DURING GC
            ptr = ScratchPointer1;
            object = ptr;
            endofobj = rplSkipOb(ptr);

            ++ptr;
            operator= 0;
            rattr = -1;
            attr = -1;
        }

        if(!ISPROLOG(*ptr) && !ISint32_t(*ptr) && !ISCONSTANT(*ptr)) {
            operator=* ptr;
            ++ptr;
        }

        while(ptr < endofobj) {
            if(ISNUMBER(*ptr)) {
                REAL n;
                rplReadNumberAsReal(ptr, &n);
                if(!iszeroReal(&n)) {
                    if(isNANorinfiniteReal(&n))
                        attr = IDATTR_ISINFREAL;
                    else {
                        attr = IDATTR_ISREAL | IDATTR_GTEZERO | IDATTR_NOTZERO;
                        if(isintegerReal(&n)) {
                            attr |= IDATTR_INTEGER;
                            if(isoddReal(&n))
                                attr |= IDATTR_ODD;
                            else
                                attr |= IDATTR_EVEN;
                        }
                    }

                }
                else
                    attr = IDATTR_ISREAL;

            }
            else if(ISCOMPLEX(*ptr)) {
                attr = IDATTR_ISCPLX;
            }
            else if(ISIDENT(*ptr)) {
                attr = rplGetIdentAttr(ptr);
            }
            else if(ISCONSTANT(*ptr)) {

                switch(TI_TYPE(rplSymbGetTokenInfo(ptr)))
                {
                case TITYPE_REAL:
                    attr = IDATTR_ISREAL | IDATTR_GTEZERO | IDATTR_NOTZERO;
                    break;
                case TITYPE_REALINF:
                    attr = IDATTR_ISINFREAL ;
                    break;
                case TITYPE_REALNEG:
                    attr = IDATTR_ISREAL | IDATTR_LTEZERO | IDATTR_NOTZERO;
                    break;
                case TITYPE_COMPLEX:
                    attr = IDATTR_ISCPLX | IDATTR_NOTZERO;
                    break;
                case TITYPE_COMPLEXINF:
                    attr = IDATTR_ISINFCPLX ;
                    break;
                case TITYPE_MATRIX:
                    attr = IDATTR_ISMATRIX;
                    break;
                case TITYPE_UNIT:
                    attr = 0;
                    break;
                default:
                    attr = 0;
                    break;

                }


            }
            else if(ISSYMBOLIC(*ptr)) {
                // RECURSE
                rplPushDataNoGrow(object);
                rplPushDataNoGrow(rplSkipOb(ptr));
                ScratchPointer1 = ptr;
                rplNewint32_tPush(rattr, DECint32_t);
                if(Exceptions) {
                    DSTop = stksave;
                    return 0;
                }
                // RE-READ POINTERS FROM STACK IN CASE THEY MOVED DURING GC
                ptr = ScratchPointer1;
                object = ptr;
                endofobj = rplSkipOb(ptr);
                operator= rplSymbMainOperator(ptr);
                ++ptr;
                rattr = -1;
                attr = -1;

            }
            else
                attr = 0;

            if(operator)
                rattr = rplSymbCombineAttr(operator, rattr, attr);
            else
                rattr = attr;

            ptr = rplSkipOb(ptr);
        }

        // HERE WE FINISHED COMPUTING THE ATTRIBUTE OF A SYMBOLIC
        // USE IT AS THE ARGUMENT ATTRIBUTE FOR THE NEXT OPERATION
        finishedobject = object;
        attr = rattr;

    }

    // WE FINISHED, attr HAS THE ATTRIBUTE WE ARE LOOKING FOR

    // SET DEFAULT ATTRIBUTES FOR VARIABLES THAT DON'T HAVE THEM (REAL IN REAL MODE, COMPLEX IN COMPLEX MODE)
    if(!attr) {
        if(rplTestSystemFlag(FL_COMPLEXMODE))
            attr = IDATTR_ISINFCPLX;
        else
            attr = IDATTR_ISINFREAL;
    }

    return attr;
}

// REPLACE ALL OCCURRENCES OF A VARIABLE NAME WITH ANOTHER VARIABLE NAME
// Uses ScratchPointer1,2 and 3

word_p rplSymbReplaceVar(word_p symb, word_p findvar, word_p newvar)
{
    word_p *savestk = DSTop;
    ScratchPointer2 = findvar;
    ScratchPointer3 = newvar;

    int32_t n = rplSymbExplode(symb);
    if(Exceptions) {
        DSTop = savestk;
        return 0;
    }

    word_p *stkptr = DSTop - 1;
    findvar = ScratchPointer2;
    newvar = ScratchPointer3;

    int32_t k;
    for(k = 0; k < n; ++k) {
        if(rplCompareIDENT(*stkptr, findvar))
            *stkptr = newvar;
        --stkptr;
    }

    ScratchPointer1 = rplSymbImplode(DSTop - 1);
    DSTop = savestk;
    return ScratchPointer1;
}
