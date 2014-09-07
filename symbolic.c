#include "newrpl.h"
#include "libraries.h"

// GLOBAL SUPPORT FUNCTIONS FOR SYMBOLICS
#define num_max(a,b) ((a)>(b)? (a):(b))
#define num_min(a,b) ((a)<(b)? (a):(b))

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


31= BRACKETS/PARENTHESIS/COMMA

16 = OVR_EVAL, OVR_XEQ
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

// APPLY AN OPERATOR WITH ARGUMENTS RECENTLY EVAL'ED, AND EVAL THE RESULT AS WELL
// SIMILAR TO APPLYING THE OPERATOR BUT IT ALSO DOES MINOR SIMPLIFICATION

#define SYMBITEMCOMPARE(item1,item2) ((BINT)LIBNUM(*(item2))-(BINT)LIBNUM(*(item1)))



void rplSymbEVALApplyOperator(WORD Opcode,BINT nargs)
{
    if(LIBNUM(Opcode)==LIB_OVERLOADABLE) {
        // TREAT SOME OPERATORS IN A SPECIAL WAY
        // TO APPLY SIMPLIFICATIONS

    switch(OPCODE(Opcode))
    {
    case OVR_ADD:
    {
        // SORT ARGUMENTS BY LIBRARY NUMBER
        WORDPTR *ptr,*ptr2,*endlimit,*startlimit,save;
        WORDPTR *left,*right;

        startlimit=DSTop-nargs+1;    // POINT TO SECOND ELEMENT IN THE LIST
        endlimit=DSTop;           // POINT AFTER THE LAST ELEMENT

        for(ptr=startlimit;ptr<endlimit;++ptr)
        {
            save=*ptr;

            left=startlimit-1;
            right=ptr-1;
            if(SYMBITEMCOMPARE(*right,save)>0) {
               if(SYMBITEMCOMPARE(save,*left)>0) {
            while(right-left>1) {
                if(SYMBITEMCOMPARE(*(left+(right-left)/2),save)>0) {
                    right=left+(right-left)/2;
                }
                else {
                    left=left+(right-left)/2;
                }
            }
               } else right=left;
            // INSERT THE POINTER RIGHT BEFORE right
            for(ptr2=ptr;ptr2>right; ptr2-=1 ) *ptr2=*(ptr2-1);
            //memmove(right+1,right,(ptr-right)*sizeof(WORDPTR));
            *right=save;
            }
        }

        // TODO: PREPROCESS EACH ARGUMENT
        // NEGATIVE NUMBERS BECOME (UMINUS POSITIVE NUMBER)


        // APPLY
        BINT k;
        for(k=1;k<nargs;++k) {
            rplCallOvrOperator(Opcode);
        }

    }
        return;
    }
    }
    else rplSymbApplyOperator(Opcode,nargs);
}




// DETERMINES WHETHER TWO SYMBOLIC OBJECTS ARE IDENTICAL OR NOT
// NO ARGUMENT CHECKS, NEVER THROWS EXCEPTIONS, DOES NOT ALTER THE STACK, DOES NOT ALLOCATE MEMORY

BINT rplSymbObjectMatch(WORDPTR baseobject,WORDPTR objptr)
{

    // START THE MATCHING PROCESS


    WORDPTR endofbase=rplSkipOb(baseobject);
    WORDPTR endofobj=rplSkipOb(objptr);

    while( (baseobject<endofbase) && (objptr<endofobj)) {

        if(ISSYMBOLIC(*baseobject)) baseobject=rplSymbUnwrap(baseobject);
        if(ISSYMBOLIC(*objptr)) objptr=rplSymbUnwrap(objptr);

        if(ISNUMBER(*baseobject)) {
            if(!ISNUMBER(*objptr)) return 0;   // MATCH FAILED

            mpd_t num1,num2;

            rplReadNumberAsReal(baseobject,&num1);
            rplReadNumberAsReal(objptr,&num2);

            if(mpd_cmp(&num1,&num2,&Context)) return 0; // MATCH FAILED
        }
        else
        if(!ISPROLOG(*baseobject)) {
            // IT'S AN OPCODE OF AN OPERATOR, OR AN INTEGER NUMBER. MUST MATCH EXACTLY
            if(*baseobject!=*objptr) return 0;   // MATCH FAILED
            ++baseobject;
            ++objptr;
            continue;
        }

        else
        if(ISSYMBOLIC(*baseobject)) {
            if(!ISSYMBOLIC(*objptr)) return 0; // MATCH FAILED
            // INCREASE THE POINTERS TO SCAN INSIDE THE SYMBOLIC
            ++baseobject;
            ++objptr;
            continue;
        }

        // ADD MORE SPECIAL CASES HERE

        // ALL OTHER OBJECTS **MUST** BE IDENTICAL, REGARDLESS OF TYPE
       else
       if(!rplCompareObjects(baseobject,objptr)) return 0;    // MATCH FAILED


        // AFTER A MATCH, PROCESS THE NEXT OBJECT
        baseobject=rplSkipOb(baseobject);
        objptr=rplSkipOb(objptr);

    }

    if((baseobject!=endofbase)||(objptr!=endofobj)) return 0;   // RETURN WITH "NO MATCH" RESULT

    return 1;
}











// SYMBOLIC EXPRESSION IN LEVEL 2
// RULE IN LEVEL 1
// CREATES A NEW LOCAL ENVIRONMENT, WITH THE FOLLOWING VARIABLES:
// GETLAM1 IS AN UNNAMED VARIABLE THAT WILL CONTAIN 1 IF THERE WAS A MATCH, 0 OTHERWISE
// GETLAM2 IS UNNAMED, AND WILL CONTAIN A POINTER INSIDE THE ORIGINAL SYMBOLIC WHERE THE LAST MATCH WAS FOUND, TO BE USED BY MATCHNEXT
// * ANY IDENTS THAT DON'T START WITH A . ARE CREATED AND SET EQUAL TO THE RIGHT SIDE OF THE RULE OPERATOR
// * ANY IDENTS THAT START WITH A PERIOD MATCH ANY EXPRESSION AS FOLLOWS:
// .X MATCHES ANY EXPRESSION (LARGEST MATCH POSSIBLE) AND DEFINES .X = 'FOUND EXPRESSION'
// .X.s MATCHES ANY EXPRESSION (SMALLEST MATCH POSSIBLE)
// .X.S SAME AS DEFAULT (LARGEST MATCH) (AN ALIAS FOR CONSISTENCY)
// .X.n MATCHES ANY NUMERIC EXPRESSION (SMALLEST MATCH) AND DEFINES .X.n = 'FOUND EXPRESSION'
// .X.N MATCHES ANY NUMERIC EXPRESSION (LARGEST MATCH) AND DEFINES .X.N = 'FOUND EXPRESSION'
// .X.I MATCHES ANY INTEGER .X.I = NUMBER
// .X.R MATCHES ANY NUMBER (REAL OR INTEGER, BUT WON'T MATCH FRACTIONS) .X.R = NUMBER.



void rplSymbRuleMatch()
{
    // MATCH A RULE IN THE CURRENT SYMBOLIC, DOES NOT MATCH RECURSIVELY INSIDE THE SYMBOLIC

    WORDPTR ruleleft,ruleright,ruleptr,objptr;

    ruleleft=rplSymbUnwrap(rplPeekData(1));
    if(!ruleleft) {
        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
        return;
    }
    if(!ISSYMBOLIC(*ruleleft)) {
        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
        return;
    }
    ++ruleleft;
    if(*ruleleft!=CMD_RULESEPARATOR) {
        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
        return;
    }
    ++ruleleft;
    ruleright=rplSkipOb(ruleleft);

    objptr=rplSymbUnwrap(rplPeekData(2));

    if(!objptr) {
        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
        return;
    }


    // START THE MATCHING PROCESS
    // CREATE A NEW ENVIRONMENT FOR LOCAL VARS

    // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
    rplCreateLAMEnvironment(IPtr);

    rplCreateLAM(nulllam_ident,zero_bint);  // GETLAM1 = MATCH OR NOT?
    rplCreateLAM(nulllam_ident,zero_bint);  // GETLAM2 = LAST OBJECT SCANNED

    // ... FROM HERE ON, THERE ARE NAMED LAM'S WITH THE PATTERNS

    if( (!ISSYMBOLIC(*objptr)) && (!ISIDENT(*objptr))) {
        // NOT A SYMBOLIC OBJECT, NOTHING TO MATCH
        // RETURN WITH A "DID NOT MATCH" IN GETLAM1
        return;
    }

    WORDPTR endofrule=rplSkipOb(ruleleft);
    WORDPTR endofobj=rplSkipOb(objptr);

    while( (ruleleft<endofrule) && (objptr<endofobj)) {

        if(ISSYMBOLIC(*ruleleft)) ruleleft=rplSymbUnwrap(ruleleft);
        if(ISSYMBOLIC(*objptr)) objptr=rplSymbUnwrap(objptr);

        if(ISSYMBOLIC(*ruleleft)) {
            if(!ISSYMBOLIC(*objptr)) break; // MATCH FAILED
            // INCREASE THE POINTERS TO SCAN INSIDE THE SYMBOLIC
            ++ruleleft;
            ++objptr;
            if(*ruleleft==MKOPCODE(LIB_OVERLOADABLE,OVR_ADD)) {
                // DO COMMUTATIVE/ASSOCIATIVE MATCHING
                //if(!rplSymbAdditionMatch(ruleleft-1,objptr-1)) break;


            }

            continue;
        }
        else
        if(ISIDENT(*ruleleft)) {
            // CHECK IF IT'S A PLACEHOLDER
            BINT len=OBJSIZE(*ruleleft)<<2;
            BINT shortlen;
            BYTE type=0;
            BYTEPTR varname=(BYTEPTR)(ruleleft+1);
            if(*((char *)varname)=='.') {
                // THIS IS A SPECIAL PLACEHOLDER
                BYTEPTR name2=varname+len-1;
                while(*name2==0) --name2;
                while(*name2!='.') --name2;
                if(name2==varname) {
                    // THERE WAS NO SUFFIX
                    type=0;
                } else {
                    if(name2+1<varname+len) type=*(name2+1);
                    else type=0;
                }

                // HERE WE HAVE: type HOLDS THE TYPE CHARACTER

                // IF THIS NAME ALREADY EXISTS, WE NEED TO MATCH ITS CONTENTS
                // IF IT DOESN'T, WE NEED TO FIND A PROPER MATCH

                WORDPTR lamptr=rplFindLAM(ruleleft,0);
                if(lamptr) {
                    // NAME ALREADY EXISTS, THE OBJECT MUST MATCH

                    if(!rplSymbObjectMatch(*(lamptr+1),objptr)) break; //  MATCH FAILED
                    // MATCH SUCCEEDED!

                } else {
                    ScratchPointer1=endofrule;
                    ScratchPointer2=endofobj;
                    ScratchPointer3=ruleleft;
                    ScratchPointer4=objptr;
                BINT newlam=rplCreateLAM(ruleleft,zero_bint);       // CREATE A PLACEHOLDER FOR THE LAM
                    endofrule=ScratchPointer1;
                    endofobj=ScratchPointer2;
                    ruleleft=ScratchPointer3;
                    objptr=ScratchPointer4;
                if(Exceptions) {
                 rplCleanupLAMs(0);
                 return;
                }
                // TODO: DO THE MATCH AND ASSIGN IT!
                BINT matchresult=0;
                switch(type)
                {
                case 'N':
                case 'n':
                   // ONLY MATCH A NUMERIC EXPRESSION
                   // AN EXPRESSION WHERE ALL ARGUMENTS ARE NOT IDENTS
                   // TODO:
                   // if(rplSymbIsNumeric(objptr)) matchresult=1;
                break;
                case 'I':
                    // ONLY MATCH AN INTEGER NUMBER
                    if(ISBINT(*objptr)) matchresult=1;
                break;
                case 'R':
                    if(ISNUMBER(*objptr)) matchresult=1;
                    break;
                case 0:
                default:
                    // MATCH ANY EXPRESSION
                    matchresult=1;
                    break;
                }

                if(matchresult) {
                    rplPutLAMn(newlam,objptr);
                }
                else break;

                }


            }   // END OF SPECIAL IDENTS
          else {
                // NORMAL IDENTS
                if(!rplCompareIDENT(ruleleft,objptr)) break;    // MATCH FAILED
            }

        }


        // ADD MORE SPECIAL CASES HERE

        // ALL OTHER OBJECTS **MUST** BE IDENTICAL, REGARDLESS OF TYPE
       else
       if(!rplSymbObjectMatch(ruleleft,objptr)) break;    // MATCH FAILED


        // AFTER A MATCH, PROCESS THE NEXT OBJECT
        ruleleft=rplSkipOb(ruleleft);
        objptr=rplSkipOb(objptr);


    }

    if((ruleleft!=endofrule)||(objptr!=endofobj)) return;   // RETURN WITH "NO MATCH" RESULT

    rplPutLAMn(1,one_bint);    // RETURN NULLLAM1 = 1
    rplPutLAMn(2,endofobj);    // NEXT OBJECT TO BE SCANNED

    return;
}

// COUNT HOW MANY ARGUMENTS THERE ARE FOR THE MAIN SYMBOLIC OPERATOR
// RETURNS ZERO FOR ATOMIC OBJECTS, OR THE NUMBER OF ARGUMENTS FOR AN OPERATOR OR FUNCTION
// IT FLATTENS ADDITION AND MULTIPLICATION

BINT rplSymbCountArguments(WORDPTR object)
{
    object=rplSymbUnwrap(object);
    if(!object) return 0;

    if(!ISSYMBOLIC(*object)) return 0;

    WORDPTR endofobj=rplSkipOb(object);
    WORD Opcode=*(object+1);
    if(ISPROLOG(Opcode)) return 0;
    if(ISBINT(Opcode)) return 0;

    // HERE WE ARE SURE THAT OPCODE IS AN OPERATOR
    object+=2;
    BINT count=0;

    while(object!=endofobj) {
        if(ISSYMBOLIC(*object)) object=rplSymbUnwrap(object);
        if(ISSYMBOLIC(*object)) {
        if(*(object+1)==Opcode) {
            // SAME OPERATION
            if( (Opcode==MKOPCODE(LIB_OVERLOADABLE,OVR_ADD)) || (Opcode==MKOPCODE(LIB_OVERLOADABLE,OVR_MUL))) { object+=2; continue; }
        }

        }
        ++count;
        object=rplSkipOb(object);
    }

    return count;

}


// SCAN AN OBJECT, RETURN 1 IF IT'S NUMERIC, 0 OTHERWISE

BINT rplSymbIsNumeric(WORDPTR object)
{

    WORDPTR endofobj=rplSkipOb(object);

    while(object!=endofobj) {
        if(ISSYMBOLIC(*object)) object=rplSymbUnwrap(object);
        if(ISSYMBOLIC(*object)) { ++object; continue; }
        if(ISIDENT(*object)) return 0;
        object=rplSkipOb(object);
    }

    return 1;
}


// SCAN AN OBJECT, RETURN 1 IF IT CONTAINS A SPECIAL RULE IDENT, 0 OTHERWISE
BINT rplSymbHasSpecialIdent(WORDPTR object)
{
    WORDPTR endofobj=rplSkipOb(object);

    while(object!=endofobj) {
        if(ISSYMBOLIC(*object)) object=rplSymbUnwrap(object);
        if(ISSYMBOLIC(*object)) { ++object; continue; }
        if(ISIDENT(*object)) {
            // CHECK IF IT'S A SPECIAL IDENT (ALL SPECIAL IDENTS BEGIN WITH A DOT)
            if(*((char *)(object+1))=='.') return 1;
        }
        object=rplSkipOb(object);
    }

    return 0;

}

// REPLACE AN OBJECT WITHIN A SYMBOLIC
// IT CREATES A NEW OBJECT, SO IT MIGHT TRIGGER GC
// USES ScratchPointers 1 THRU 3 DURING GC
// RETURNS A POINTER TO THE NEW OBJECT
// LOW LEVEL FUNCTION - NO ARGUMENT CHECKS!!!

WORDPTR rplSymbReplace(WORDPTR mainobj,WORDPTR arg,WORDPTR newarg)
{

    ScratchPointer1=mainobj;
    ScratchPointer2=arg;
    ScratchPointer3=newarg;
    WORDPTR newobj=rplAllocTempOb(rplObjSize(mainobj)-rplObjSize(arg)+rplObjSize(newarg)-1),ptr,end;
    if(Exceptions) return NULL;
    ptr=newobj;
    mainobj=ScratchPointer1;
    arg=ScratchPointer2;
    newarg=ScratchPointer3;
    while(mainobj!=arg) *ptr++=*mainobj++;
    end=rplSkipOb(newarg);
    while(newarg!=end) *ptr++=*newarg++;
    mainobj=rplSkipOb(arg);
    end=rplSkipOb(ScratchPointer1);
    while(mainobj!=end) *ptr++=*mainobj++;
    return newobj;
}


// EXPLODE A SYMBOLIC IN THE STACK IN REVERSE (LEVEL 1 CONTAINS THE FIRST OBJECT, LEVEL 2 THE SECOND, ETC.)
// INCLUDING OPERATORS
// USES ScratchPointer1 FOR GC PROTECTION
// RETURN THE NUMBER OF OBJECTS THAT ARE ON THE STACK

BINT rplSymbExplode(WORDPTR object)
{
    BINT count=0,countops=0,nargs;

    WORDPTR ptr,end,localend,numbers;

    ptr=object;
    end=rplSkipOb(object);

    while(ptr!=end) {
        if(ISSYMBOLIC(*ptr)) { ++ptr; continue; }
        if(! (ISPROLOG(*ptr) || ISBINT(*ptr))) ++countops;
        ++count;
        ptr=rplSkipOb(ptr);
    }

    // HERE count+countops IS THE NUMBER OF POINTERS WE NEED TO PUSH ON THE STACK
    ScratchPointer1=object;
    rplExpandStack(count+countops);  // EXPAND THE DATA STACK NOW, SO WE CAN MANUALLY PUSH IN BULK
    if(Exceptions) return 0;
    numbers=rplAllocTempOb(countops);
    if(!numbers) return 0;
    object=ScratchPointer1;
    ptr=DSTop+count+countops-1;
    end=rplSkipOb(object);
    countops=0;

    while(object!=end) {
        if(ISSYMBOLIC(*object)) {
            nargs=0;
            WORDPTR tmp=object+1,tmpend=rplSkipOb(object);
            while(tmp!=tmpend) {
                ++nargs;
                tmp=rplSkipOb(tmp);
            }
            // HERE nargs HAS THE NUMBER OF ARGUMENTS + 1 FOR THE OPCODE
            ++object;
            continue;
        }
        *ptr=object;
        if(! (ISPROLOG(*object) || ISBINT(*object))) { numbers[countops]=MAKESINT(nargs); --ptr; *ptr=&numbers[countops]; ++countops; }
        --ptr;
        object=rplSkipOb(object);
    }

    DSTop+=count+countops;

    return count+countops;

}

// REASSEMBLE A SYMBOLIC THAT WAS EXPLODED IN THE STACK
// DOES NOT CHECK FOR VALIDITY OF THE SYMBOLIC!

WORDPTR rplSymbImplode(BINT numobjects)
{

    WORDPTR *stkptr=DSTop-1;

    BINT size=0,narg;

    BINT f;

    for(f=0;f<numobjects;++f)
    {
        size+=rplObjSize(*stkptr);
        --stkptr;
    }

    // HERE size HAS THE TOTAL SIZE WE NEED TO ALLOCATE

    WORDPTR newobject=rplAllocTempOb(size),newptr,object;

    if(!newobject) return NULL;

    stkptr=DSTop-1;
    newptr=newobject;
    for(f=0;f<numobjects;++f)
    {
        object=*stkptr;
        if(!(ISPROLOG(*object)||ISBINT(*object))) {
            // WE HAVE AN OPCODE, START A SYMBOLIC RIGHT HERE
            *newptr++=MKPROLOG(DOSYMB,0);
            *newptr++=*object;    // STORE THE OPCODE
            --stkptr;
            ++f;
        }
        else {
            // COPY THE OBJECT
            WORDPTR endobj=rplSkipOb(object);
            while(object!=endobj) *newptr++=*object++;
        }

        --stkptr;

    }

    // HERE WE HAVE THE NEW OBJECT, BUT ALL SYMBOLIC SIZES ARE SET TO ZERO
    // NEED TO FIX THE SIZES ACCORDING TO THE NUMBER OF ARGUMENTS
    // newptr IS POINTING AT THE END OF THE NEW OBJECT

    for(;f>0;--f) {
        ++stkptr;
        object=*stkptr;
        if(!(ISPROLOG(*object)||ISBINT(*object))) {
            // FOUND AN OPERATOR, GET THE NUMBER OF ITEMS
            narg=OPCODE(**(stkptr-1));

            // PATCH THE LAST SYMBOLIC WITH ZERO FOR SIZE IN THE OBJECT
            WORDPTR scan=newobject,lastone=0;
            while(scan<newptr) {
                if(*scan==MKPROLOG(DOSYMB,0)) {
                    lastone=scan;
                    ++scan;
                    continue;
                }
                scan=rplSkipOb(scan);
            }
            // HERE lastone HAS THE LAST SYMBOLIC FOUND
            scan=lastone+1;
            while(narg) { scan=rplSkipOb(scan); --narg; }

            // AND PATCH THE SIZE
            *lastone=MKPROLOG(DOSYMB,scan-lastone-1);

        }

    }

    return newobject;

}

// SKIP ONE SYMBOLIC OBJECT IN ITS EXPLODED FORM IN THE STACK
// THE ARGUMENT IS A POINTER TO THE STACK, NOT THE OBJECT
// RETURS THE POINTER TO THE STACK ELEMENT THAT HAS THE NEXT OBEJCT

WORDPTR *rplSymbSkipInStack(WORDPTR *stkptr)
{
    if(ISPROLOG(**stkptr)) return --stkptr;
    if(ISBINT(**stkptr)) return --stkptr;

    // IT'S AN OPERATOR
    --stkptr;

    BINT nargs=OPCODE(**stkptr)-1;    // EXTRACT THE SINT
    --stkptr;
    while(nargs) {
        if(ISPROLOG(**stkptr)) { --stkptr; --nargs; continue; }
        if(ISBINT(**stkptr)) { --stkptr; --nargs; continue; }
        --stkptr;
        nargs+=OPCODE(**stkptr)-1;
        --stkptr;
        --nargs;
    }

    return stkptr;
}




// CONVERT A SYMBOLIC INTO CANONICAL FORM:
// A) NEGATIVE NUMBERS REPLACED WITH NEG(n)
// B) ALL SUBTRACTIONS REPLACED WITH ADDITION OF NEGATED ITEMS
// c) ALL NEG(A+B+...) = NEG(A)+NEG(B)+NEG(...)
// D) FLATTEN ALL ADDITION TREES

// E) ALL NEGATIVE POWERS REPLACED WITH a^-n = INV(a^n)
// F) ALL DIVISIONS REPLACED WITH MULTIPLICATION BY INV()
// G) ALL INV(A*B*...) = INV(A)*INV(B)*INV(...)
// G.2) ALL NEG(A*B*...) = NEG(A)*B*...
// H) FLATTEN ALL MULTIPLICATION TREES

const WORD const uminus_opcode[]={
    MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)
};
const WORD const add_opcode[]={
    MKOPCODE(LIB_OVERLOADABLE,OVR_ADD)
};
const WORD const inverse_opcode[]={
    MKOPCODE(LIB_OVERLOADABLE,OVR_INV)
};
const WORD const mul_opcode[]={
    MKOPCODE(LIB_OVERLOADABLE,OVR_MUL)
};


WORDPTR rplSymbCanonicalForm(WORDPTR object)
{
    BINT numitems=rplSymbExplode(object);
    BINT f;
    WORDPTR *stkptr,sobj,*endofstk;

    stkptr=DSTop-1;
    endofstk=stkptr-numitems;

    //*******************************************
    // SCAN THE SYMBOLIC FOR ITEM A)
    // A) NEGATIVE NUMBERS REPLACED WITH NEG(n)


    while(stkptr!=endofstk) {
        sobj=*stkptr;

        if(ISBINT(*sobj)) {
            // THE OBJECT IS AN INTEGER NUMBER
            BINT64 num=rplReadBINT(sobj);
            if(num<0) {
                num=-num;
                rplNewBINTPush(num,DECBINT);
                if(Exceptions) { DSTop=endofstk+1; return NULL; }
                WORDPTR newobj=rplPeekData(1);

                WORDPTR *ptr=DSTop-2;

                // MAKE A HOLE IN THE STACK TO ADD NEGATION
                while(ptr!=stkptr) {
                    *(ptr+2)=*ptr;
                    --ptr;
                }
                DSTop++;    // MOVE ONLY ONE SPOT, DROPPING THE NEW OBJECT IN THE SAME OPERATION
                stkptr[0]=newobj;
                stkptr[1]=two_bint;
                stkptr[2]=uminus_opcode;
            }
            }

        if(ISREAL(*sobj)) {
            // THE OBJECT IS A REAL NUMBER
            mpd_t dec;
            rplCopyRealToRReg(0,sobj);
            if(mpd_isnegative(&RReg[0])) {
                RReg[0].flags^=MPD_NEG; // MAKE IT POSITIVE
                rplNewRealFromRRegPush(0);
                if(Exceptions) { DSTop=endofstk+1; return NULL; }
                WORDPTR newobj=rplPeekData(1);

                WORDPTR *ptr=DSTop-2;

                // MAKE A HOLE IN THE STACK TO ADD NEGATION
                while(ptr!=stkptr) {
                    *(ptr+2)=*ptr;
                    --ptr;
                }
                DSTop++;    // MOVE ONLY ONE SPOT, DROPPING THE NEW OBJECT IN THE SAME OPERATION
                stkptr[0]=newobj;
                stkptr[1]=two_bint;
                stkptr[2]=uminus_opcode;
            }
            }

        --stkptr;
        }


    //*******************************************
    // SCAN THE SYMBOLIC FOR ITEM B)
    // B) ALL SUBTRACTIONS REPLACED WITH ADDITION OF NEGATED ITEMS

    stkptr=DSTop-1;

    while(stkptr!=endofstk) {
        sobj=*stkptr;

        if(*sobj==MKOPCODE(LIB_OVERLOADABLE,OVR_SUB)) {

                WORDPTR *secondarg=rplSymbSkipInStack(stkptr-2);

                WORDPTR *ptr=DSTop-1;

                // MAKE A HOLE IN THE STACK TO ADD NEGATION
                while(ptr!=secondarg) {
                    *(ptr+2)=*ptr;
                    --ptr;
                }
                DSTop+=2;   // 2 PLACES IN THE STACK ARE GUARANTEED BY STACK SLACK
                stkptr+=2;
                secondarg[1]=two_bint;
                secondarg[2]=uminus_opcode;
                *stkptr=add_opcode;
                stkptr--;
                rplExpandStack(2);  // NOW GROW THE STACK
                if(Exceptions) { DSTop=endofstk+1; return NULL; }
            }



        --stkptr;
        }




    //*******************************************
    // SCAN THE SYMBOLIC FOR ITEM C)
    // C) ALL NEG(A+B+...) = NEG(A)+NEG(B)+NEG(...)

    stkptr=DSTop-1;
    while(stkptr!=endofstk) {
        sobj=*stkptr;

        if(*sobj==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
            WORDPTR *nextarg=stkptr-2;

            if(**nextarg==MKOPCODE(LIB_OVERLOADABLE,OVR_ADD)) {
                // A SUM NEGATED? DISTRIBUTE THE OPERATOR OVER THE ARGUMENTS

                BINT nargs=OPCODE(**(nextarg-1))-1;

                BINT c;
                nextarg-=2;
                for(c=0;c<nargs;++c) {

                    if(**nextarg==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
                        // NEG/NEG = REMOVE THE NEGATION
                    WORDPTR *ptr=nextarg-1;
                    // AND REMOVE THE GAP
                    while(ptr!=DSTop-2) {
                    *ptr=*(ptr+2);
                    ++ptr;
                    }
                    DSTop-=2;
                    stkptr-=2;
                    nextarg-=2;
                    }
                    else {
                        // NEGATE THIS TERM
                        WORDPTR *ptr=DSTop-1;
                        // AND REMOVE THE GAP
                        while(ptr!=nextarg) {
                        *(ptr+2)=*ptr;
                        --ptr;
                        }

                        DSTop+=2;
                        stkptr+=2;
                        nextarg[1]=two_bint;
                        nextarg[2]=uminus_opcode;
                        nextarg+=2;
                    }

                    nextarg=rplSymbSkipInStack(nextarg);

                }
                // REMOVE THE ORIGINAL NEGATION
                WORDPTR *ptr=stkptr-1;
                // AND REMOVE THE GAP
                while(ptr!=DSTop-2) {
                *ptr=*(ptr+2);
                ++ptr;
                }
                DSTop-=2;
                stkptr-=2;

                }
                else stkptr--;
         }



        --stkptr;
        }






    //*******************************************
    // SCAN THE SYMBOLIC FOR ITEM D)
    // D) FLATTEN ALL ADDITION TREES

    stkptr=DSTop-1;
    while(stkptr!=endofstk) {
        sobj=*stkptr;

        if(*sobj==MKOPCODE(LIB_OVERLOADABLE,OVR_ADD)) {
                BINT nargs=OPCODE(**(stkptr-1))-1;

                BINT c,orignargs=nargs;
                WORDPTR *nextarg=stkptr-2;

                for(c=0;c<nargs;++c) {

                    if(**nextarg==MKOPCODE(LIB_OVERLOADABLE,OVR_ADD)) {
                        // FLATTEN BY REMOVING THE ADDITION
                    WORDPTR *ptr=nextarg-1;
                    nargs+=OPCODE(**(nextarg-1))-2;   // ADD THE ARGUMENTS TO THE BASE LOOP
                    // AND REMOVE THE GAP
                    while(ptr!=DSTop-2) {
                    *ptr=*(ptr+2);
                    ++ptr;
                    }
                    DSTop-=2;
                    stkptr-=2;
                    --c;
                    nextarg-=2;
                    }
                    else nextarg=rplSymbSkipInStack(nextarg);

                }

                // HERE stkptr IS POINTING TO THE ORIGINAL SUM COMMAND
                // STORE THE NEW TOTAL NUMBER OF ARGUMENTS
                if(orignargs!=nargs) {
                WORDPTR newnumber=rplNewSINT(nargs+1,DECBINT);
                if(!newnumber) { DSTop=endofstk+1; return NULL; }
                *(stkptr-1)=newnumber;
                }

                stkptr--;
         }



        --stkptr;
        }


    //*******************************************
    // SCAN THE SYMBOLIC FOR ITEM E)
    // E) ALL NEGATIVE POWERS REPLACED WITH a^-n = INV(a^n)

    stkptr=DSTop-1;

    while(stkptr!=endofstk) {
        sobj=*stkptr;

        if(*sobj==MKOPCODE(LIB_OVERLOADABLE,OVR_POW)) {

            WORDPTR *arg1=stkptr-2;
            WORDPTR *arg2=rplSymbSkipInStack(arg1);

            if(**arg2==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
                // NEGATIVE POWER DETECTED WE JUST NEED TO REPLACE THE UMINUS
                // WITH AN INV()

                // MOVE EVERYTHING TWO LEVELS UP
                WORDPTR *ptr=arg2-1;
                while(ptr!=stkptr+1) {
                    *ptr=*(ptr+2);
                    ++ptr;
                }

                *stkptr=inverse_opcode;
                --stkptr;
                *stkptr=two_bint;
            }
            }

        --stkptr;
        }

    //*******************************************
    // SCAN THE SYMBOLIC FOR ITEM F)
    // F) ALL DIVISIONS REPLACED WITH MULTIPLICATION BY INV()

    stkptr=DSTop-1;

    while(stkptr!=endofstk) {
        sobj=*stkptr;

        if(*sobj==MKOPCODE(LIB_OVERLOADABLE,OVR_DIV)) {

                WORDPTR *secondarg=rplSymbSkipInStack(stkptr-2);

                WORDPTR *ptr=DSTop-1;

                // MAKE A HOLE IN THE STACK TO ADD INVERSE
                while(ptr!=secondarg) {
                    *(ptr+2)=*ptr;
                    --ptr;
                }
                DSTop+=2;   // 2 PLACES IN THE STACK ARE GUARANTEED BY STACK SLACK
                stkptr+=2;
                secondarg[1]=two_bint;
                secondarg[2]=inverse_opcode;
                *stkptr=mul_opcode;
                stkptr--;
                rplExpandStack(2);  // NOW GROW THE STACK
                if(Exceptions) { DSTop=endofstk+1; return NULL; }
            }



        --stkptr;
        }


    //*******************************************
    // SCAN THE SYMBOLIC FOR ITEM G)
    // G) ALL INV(A*B*...) = INV(A)*INV(B)*INV(...)

    stkptr=DSTop-1;
    while(stkptr!=endofstk) {
        sobj=*stkptr;

        if(*sobj==MKOPCODE(LIB_OVERLOADABLE,OVR_INV)) {
            WORDPTR *nextarg=stkptr-2;

            if(**nextarg==MKOPCODE(LIB_OVERLOADABLE,OVR_MUL)) {

                BINT nargs=OPCODE(**(nextarg-1))-1;

                BINT c;
                nextarg-=2;
                for(c=0;c<nargs;++c) {

                    if(**nextarg==MKOPCODE(LIB_OVERLOADABLE,OVR_INV)) {
                        // INV/INV = REMOVE THE OPERATOR
                    WORDPTR *ptr=nextarg-1;
                    // AND REMOVE THE GAP
                    while(ptr!=DSTop-2) {
                    *ptr=*(ptr+2);
                    ++ptr;
                    }
                    DSTop-=2;
                    stkptr-=2;
                    nextarg-=2;
                    }
                    else {
                        // INVERT THIS TERM
                        WORDPTR *ptr=DSTop-1;
                        // AND REMOVE THE GAP
                        while(ptr!=nextarg) {
                        *(ptr+2)=*ptr;
                        --ptr;
                        }

                        DSTop+=2;
                        stkptr+=2;
                        nextarg[1]=two_bint;
                        nextarg[2]=inverse_opcode;
                        nextarg+=2;
                        rplExpandStack(2);
                        if(Exceptions){ DSTop=endofstk+1; return NULL; }
                    }

                    nextarg=rplSymbSkipInStack(nextarg);

                }
                // REMOVE THE ORIGINAL INVERSION
                WORDPTR *ptr=stkptr-1;
                // AND REMOVE THE GAP
                while(ptr!=DSTop-2) {
                *ptr=*(ptr+2);
                ++ptr;
                }
                DSTop-=2;
                stkptr-=2;

                }
                else stkptr--;
         }



        --stkptr;
        }

    //*******************************************
    // SCAN THE SYMBOLIC FOR ITEM G.2)
    // G.2) ALL NEG(A*B*...) = NEG(A)*B*...

    stkptr=DSTop-1;
    while(stkptr!=endofstk) {
        sobj=*stkptr;

        if(*sobj==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
            WORDPTR *nextarg=stkptr-2;

            if(**nextarg==MKOPCODE(LIB_OVERLOADABLE,OVR_MUL)) {

                WORDPTR tmp;

                // SWAP THE MUL WITH THE NEG
                tmp=*stkptr;
                *stkptr=*nextarg;
                *nextarg=tmp;
                tmp=*(stkptr-1);
                *(stkptr-1)=*(nextarg-1);
                *(nextarg-1)=tmp;
         }
        }



        --stkptr;
        }


    //*******************************************
    // SCAN THE SYMBOLIC FOR ITEM H)
    // H) FLATTEN ALL MULTIPLICATION TREES

    stkptr=DSTop-1;
    while(stkptr!=endofstk) {
        sobj=*stkptr;

        if(*sobj==MKOPCODE(LIB_OVERLOADABLE,OVR_MUL)) {
                BINT nargs=OPCODE(**(stkptr-1))-1;

                BINT c,orignargs=nargs;
                WORDPTR *nextarg=stkptr-2;

                for(c=0;c<nargs;++c) {

                    if(**nextarg==MKOPCODE(LIB_OVERLOADABLE,OVR_MUL)) {
                        // FLATTEN BY REMOVING THE ADDITION
                    WORDPTR *ptr=nextarg-1;
                    nargs+=OPCODE(**(nextarg-1))-2;   // ADD THE ARGUMENTS TO THE BASE LOOP
                    // AND REMOVE THE GAP
                    while(ptr!=DSTop-2) {
                    *ptr=*(ptr+2);
                    ++ptr;
                    }
                    DSTop-=2;
                    stkptr-=2;
                    --c;
                    nextarg-=2;
                    }
                    else nextarg=rplSymbSkipInStack(nextarg);

                }

                // HERE stkptr IS POINTING TO THE ORIGINAL MUL COMMAND
                // STORE THE NEW TOTAL NUMBER OF ARGUMENTS
                if(orignargs!=nargs) {
                WORDPTR newnumber=rplNewSINT(nargs+1,DECBINT);
                if(!newnumber) { DSTop=endofstk+1; return NULL; }
                *(stkptr-1)=newnumber;
                }

                stkptr--;
         }



        --stkptr;
        }


    //*******************************************
    // SCAN THE SYMBOLIC FOR ITEM I)
    // I) SORT ALL MULTIPLICATIONS WITH INV(...) LAST, NON-INVERSE FACTORS FIRST
    // ALSO, IF ALL FACTORS ARE INV(...), THEN ADD A BINT 1 AS FIRST ELEMENT (1/X)



    stkptr=DSTop-1;
    while(stkptr!=endofstk) {
        sobj=*stkptr;

        if(*sobj==MKOPCODE(LIB_OVERLOADABLE,OVR_MUL)) {
                BINT nargs=OPCODE(**(stkptr-1))-1;

                BINT c,orignargs=nargs;
                WORDPTR *nextarg=stkptr-2;
                WORDPTR *firstarg=nextarg;
                WORDPTR *firstinv=NULL;

                for(c=0;c<nargs;++c) {

                    if(**nextarg!=MKOPCODE(LIB_OVERLOADABLE,OVR_INV)) {
                            if(firstinv) {
                                // MOVE nextarg BEFORE firstinv
                                BINT nterms=nextarg-rplSymbSkipInStack(nextarg);
                                // HERE THE LAYOUT IS: DSTop ... firstinv... otherobj ... nextarg ... otherobj... end
                                //                               ^________________________|
                                // GROW STACK BY nterms
                                rplExpandStack(nterms);
                                if(Exceptions) { DSTop=endofstk+1; return NULL; }
                                // MOVE nextarg TO THE END OF STACK
                                WORDPTR *ptr=DSTop;
                                BINT f;
                                for(f=0;f<nterms;++f) ptr[f]=nextarg[-(nterms-1)+f];
                                // MOVE firstinv BACK
                                ptr=nextarg+1;
                                while(ptr<=firstinv) { ptr[-nterms]=*ptr; ++ptr; }
                                // MOVE nextarg BACK IN PLACE OF firstinv
                                ptr=DSTop+nterms-1;
                                for(f=0;f<nterms;++f,--ptr) firstinv[-f]=*ptr;
                                firstinv-=nterms;   // KEEP FIRST INV POINTING TO THE SAME PLACE
                                nextarg-=nterms;
                            }
                            else nextarg=rplSymbSkipInStack(nextarg);

                    } else {
                        if(!firstinv) firstinv=nextarg;
                        nextarg=rplSymbSkipInStack(nextarg);
                    }

                }

                if(firstarg==firstinv) {
                    // ALL FACTORS ARE INVERTED

                    // ADD A BINT 1 TO CREATE 1/X
                    WORDPTR *ptr=DSTop-1;

                    // MAKE A HOLE IN THE STACK TO ADD BINT ONE
                    while(ptr!=firstarg) {
                        *(ptr+1)=*ptr;
                        --ptr;
                    }
                    DSTop++;   // 2 PLACES IN THE STACK ARE GUARANTEED BY STACK SLACK
                    stkptr++;
                    firstarg[1]=one_bint;
                    // INCREASE THE COUNT OF OBJECTS
                    BINT64 numargs=OPCODE(*firstarg[2]);
                    ++numargs;
                    WORDPTR nnum=rplNewSINT(numargs,DECBINT);
                    if(Exceptions) { DSTop=endofstk+1; return NULL; }
                    firstarg[2]=nnum;
                    rplExpandStack(1);  // NOW GROW THE STACK
                    if(Exceptions) { DSTop=endofstk+1; return NULL; }
                }

                stkptr--;
         }



        --stkptr;
        }


    //*******************************************
    // SCAN THE SYMBOLIC FOR ITEM J)
    // J) ANY EXPRESSION STARTING WITH INV() NEEDS TO BE REPLACED WITH 1*INV(), EXCEPT MUL ARGUMENTS

    stkptr=DSTop-1;

    if(**stkptr==MKOPCODE(LIB_OVERLOADABLE,OVR_INV)) {
    // NEED TO ADD 1*INV() AT THE BEGINNING OF THE EXPRESSION

        DSTop+=3;   // 3 PLACES IN THE STACK ARE GUARANTEED BY STACK SLACK
        stkptr+=3;
        stkptr[0]=mul_opcode;
        stkptr[-1]=three_bint;
        stkptr[-2]=one_bint;
        rplExpandStack(3);  // NOW GROW THE STACK
        if(Exceptions) { DSTop=endofstk+1; return NULL; }

    }

    // AND NOW CHECK THE REST OF IT

    while(stkptr!=endofstk) {
        sobj=*stkptr;

        if(ISPROLOG(*sobj)||ISBINT(*sobj)) { --stkptr;  continue; }

        if(*sobj!=MKOPCODE(LIB_OVERLOADABLE,OVR_MUL)) {
                // EXCEPT MULTIPLICATIONS, CHECK IF ANY OTHER EXPRESSION STARTS WITH INV()

                BINT nargs=OPCODE(**(stkptr-1))-1;

                WORDPTR *argptr=stkptr-2;

                while(nargs) {
                if(**argptr==MKOPCODE(LIB_OVERLOADABLE,OVR_INV)) {
                // IN ANY OTHER CASE, NEED TO ADD 1*INV()

                    WORDPTR *ptr=DSTop-1;

                    // MAKE A HOLE IN THE STACK TO ADD BINT ONE
                    while(ptr!=argptr) {
                        *(ptr+3)=*ptr;
                        --ptr;
                    }
                    DSTop+=3;   // 3 PLACES IN THE STACK ARE GUARANTEED BY STACK SLACK
                    stkptr+=3;
                    argptr+=3;
                    argptr[0]=mul_opcode;
                    argptr[-1]=three_bint;
                    argptr[-2]=one_bint;
                    rplExpandStack(3);  // NOW GROW THE STACK
                    if(Exceptions) { DSTop=endofstk+1; return NULL; }

                }
                argptr=rplSymbSkipInStack(argptr);
                --nargs;
                }
        }
        --stkptr;
        }







    if(Exceptions) {
        DSTop=endofstk+1;
        return NULL;
    }

    WORDPTR finalsymb=rplSymbImplode(DSTop-1-endofstk);

    DSTop=endofstk+1;
    if(Exceptions) return NULL;

    return finalsymb;

}


// RECEIVES A NUMERATOR AND A DENOMINATOR NUMBERS IN THE STACK
// SIMPLIFIES BY DIVIDING BY THEIR GCD
// RETURNS 1 IF THERE WERE CHANGES, 0 IF NO SIMPLIFICATION WAS POSSIBLE

BINT rplFractionSimplify()
{

    if( (!ISNUMBER(*rplPeekData(1))) || (!ISNUMBER(*rplPeekData(2))) ) return 0;    // DON'T TRY TO SIMPLIFY IF NOT A NUMBER

    if( ISREAL(*rplPeekData(2)) || ISREAL(*rplPeekData(1)) ) {
        // TREAT ALL NUMBERS AS REALS

        BINT numneg,denneg;

        rplNumberToRReg(0,rplPeekData(2));  // REGISTER 0 = NUMERATOR
        rplNumberToRReg(1,rplPeekData(1));  // REGISTER 1 = DENOMINATOR

        // MAKE THEM BOTH POSITIVE
        numneg=RReg[0].flags&MPD_NEG;
        denneg=RReg[1].flags&MPD_NEG;
        RReg[0].flags^=numneg;
        RReg[1].flags^=denneg;

        // IF IT HAS FRACTIONAL PART, MAKE IT INTEGER BY MULTIPLYING BOTH NUM AND DEN BY 10^N
        if(RReg[0].exp<0) { RReg[1].exp+=-RReg[0].exp; RReg[0].exp=0;  }
        if(RReg[1].exp<0) { RReg[0].exp+=-RReg[1].exp; RReg[1].exp=0;  }

        // SWITCH TO MAX CONTEXT
        mpd_ssize_t previousprec=Context.prec;
        Context.prec=REAL_PRECISION_MAX;


        // FIND GCD
        mpd_t *big,*small,*tmpbig,*tmpsmall,*swap,*remainder;
        if(mpd_cmp(&RReg[0],&RReg[1],&Context)>0) { big=&RReg[0]; small=&RReg[1]; }
            else { big=&RReg[1]; small=&RReg[0]; }
        tmpbig=&RReg[2];
        tmpsmall=&RReg[3];
        remainder=&RReg[4];

        mpd_copy(tmpbig,big,&Context);
        mpd_copy(tmpsmall,small,&Context);

        while(!mpd_iszero(tmpsmall)) {

            mpd_rem(remainder,tmpbig,tmpsmall,&Context);

            swap=tmpbig;
            tmpbig=tmpsmall;
            tmpsmall=remainder;
            remainder=swap;

        }

        // HERE tmpbig = GCD(NUM,DEN)
        rplOneToRReg(5);
        if(mpd_cmp(tmpbig,&RReg[5],&Context)<=0) {
            // THERE'S NO COMMON DIVISOR, RETURN UNMODIFIED
            // THIS IS <=0 SO IT CATCHES 0/0
            return 0;
        }

        // SIMPLIFY
        mpd_div(&RReg[5],&RReg[0],tmpbig,&Context);
        mpd_div(&RReg[6],&RReg[1],tmpbig,&Context);

        // APPLY THE SIGN TO THE NUMERATOR ONLY
        RReg[5].flags|=numneg^denneg;


        // NOW TRY TO CONVERT THE REALS TO INTEGERS IF POSSIBLE
        uint32_t status=0;
        BINT64 num;
        num=mpd_qget_i64(&RReg[5],&status);
        if(!status) rplNewBINTPush(num,DECBINT);
        else rplNewRealFromRRegPush(5);
        if(Exceptions) return 0;
        status=0;
        num=mpd_qget_i64(&RReg[6],&status);
        if(!status) rplNewBINTPush(num,DECBINT);
        else rplNewRealFromRRegPush(6);
        if(Exceptions) { rplDropData(1); return 0; }

        rplOverwriteData(3,rplPeekData(1));
        rplOverwriteData(4,rplPeekData(2));
        rplDropData(2);
        return 1;

    }

    // BOTH NUMBERS ARE BINTS

    BINT64 bnum,bden;
    BINT64 tmpbig,tmpsmall,swap;
    BINT numneg,denneg;

    bnum=rplReadBINT(rplPeekData(2));
    bden=rplReadBINT(rplPeekData(1));

    // GET THE SIGNS
    if(bnum<0) { numneg=1; bnum=-bnum; } else numneg=0;
    if(bden<0) { denneg=1; bden=-bden; } else denneg=0;

    // CALCULATE THE GCD
    tmpbig=num_max(bnum,bden);
    tmpsmall=num_min(bnum,bden);

    while(tmpsmall>0) {

        while(tmpbig>=tmpsmall) tmpbig-=tmpsmall;

        swap=tmpbig;
        tmpbig=tmpsmall;
        tmpsmall=swap;

    }

    // HERE tmpbig HAS THE GCD

    if(tmpbig<=1) {
        // CHECK IF WE NEED TO CORRECT SIGNS
        if(!denneg) return 0;     // NO COMMON DIVISOR, SO RETURN WITH NO CHANGES
    }
    else {
    // SIMPLIFY
    bnum/=tmpbig;
    bden/=tmpbig;
    }

    // APPLY THE SIGN TO THE NUMERATOR ONLY
    if(numneg^denneg) bnum=-bnum;

    rplNewBINTPush(bnum,DECBINT);
    if(Exceptions) return 0;
    rplNewBINTPush(bden,DECBINT);
    if(Exceptions) { rplDropData(1); return 0; }

    rplOverwriteData(3,rplPeekData(1));
    rplOverwriteData(4,rplPeekData(2));
    rplDropData(2);
    return 1;


}

// CHECK IF ARGUMENT IN THE STACK IS A NUMERIC FRACTION
// RETURNS TRUE/FALSE

BINT rplSymbIsFractionInStack(WORDPTR *stkptr)
{

if(**stkptr==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
// COULD BE A NEGATIVE FRACTION -(1/2)
stkptr-=2;
}



//NOT A FRACTION UNLESS THERE'S A MULTIPLICATION
if(**stkptr==MKOPCODE(LIB_OVERLOADABLE,OVR_MUL)) {
    stkptr--;
BINT nargs=OBJSIZE(**stkptr)-1;
// NOT A FRACTION IF MORE THAN 2 ARGUMENTS
if(nargs!=2) return 0;
--stkptr;

WORDPTR *argptr=stkptr;

// CHECK THE NUMERATOR

if(**argptr==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
    if(!ISNUMBER(**(argptr-2))) return 0;
}
else if(!ISNUMBER(**argptr)) return 0;


argptr=rplSymbSkipInStack(argptr);

// CHECK THE DENOMINATOR
if(**argptr!=MKOPCODE(LIB_OVERLOADABLE,OVR_INV)) return 0;
argptr-=2;
if(**argptr==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
    if(!ISNUMBER(**(argptr-2))) return 0;
}
else if(!ISNUMBER(**argptr)) return 0;


}
else {
// SINGLE NUMBERS ARE ALSO CONSIDERED FRACTIONS N/1
  if(!ISNUMBER(**stkptr)) return 0;

}

return 1;
}

// EXTRACT AND PUSH PUSH NUMERATOR AND DENOMINATOR ON THE STACK
// DEAL WITH NEGATIVE NUMBERS
// DOES NOT CHECK FOR ARGUMENTS! CALLER TO USE rplSymbIsFractionInStack() TO VERIFY

void rplSymbFractionExtractNumDen(WORDPTR *stkptr)
{
    BINT negnum=0,negden=0;
    WORDPTR *savedstop=DSTop;

if(**stkptr==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
// COULD BE A NEGATIVE FRACTION -(1/2)
    negnum=1;
    stkptr-=2;
}

if(**stkptr==MKOPCODE(LIB_OVERLOADABLE,OVR_MUL)) {

    stkptr-=2;

    WORDPTR *argptr=stkptr;

    // CHECK THE NUMERATOR

    if(**argptr==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
       negnum^=1;
       rplPushData(*(argptr-2));
    }
    else rplPushData(*argptr);

    // NUMERATOR IS IN THE STACK
    if(negnum) {
        rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NEG));
        if(Exceptions) { DSTop=savedstop; return; }
    }


    argptr=rplSymbSkipInStack(argptr);

    // CHECK THE DENOMINATOR
    argptr-=2;  // SKIP THE INVERSE OPERATOR
    if(**argptr==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
       negden^=1;
       rplPushData(*(argptr-2));
    }
    else rplPushData(*argptr);

    // DENOMINATOR IS IN THE STACK
    if(negden) {
        rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NEG));
        if(Exceptions) { DSTop=savedstop; return; }
    }

}
else {
// SINGLE NUMBERS ARE ALSO CONSIDERED FRACTIONS N/1
    if(**stkptr==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
       negnum^=1;
       rplPushData(*(stkptr-2));
    }
    else rplPushData(*stkptr);

    // NUMERATOR IS IN THE STACK
    if(negnum) {
        rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NEG));
        if(Exceptions) { DSTop=savedstop; return; }
    }

    // DENOMINATOR IS ONE
    rplPushData(one_bint);

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

BINT rplSymbFractionAdd()
{
    BINT sign=0;
    rplPushData(rplPeekData(4));    // NUM1
    rplPushData(rplPeekData(2));    // DEN2
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_MUL));
    if(Exceptions) return 0;
    rplPushData(rplPeekData(3));    // NUM2
    rplPushData(rplPeekData(5));    // DEN1
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_MUL));
    if(Exceptions) return 0;
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_ADD));
    if(Exceptions) return 0;

    rplPushData(rplPeekData(4));     // DEN1
    rplPushData(rplPeekData(3));    // DEN2
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_MUL));
    if(Exceptions) return 0;

    // TODO: IF NUM OR DEN ARE NEGATIVE, CHANGE THE SIGN AND SET sign APPROPRIATELY

    // CHECK SIGN OF THE NUMERATOR
    rplPushData(rplPeekData(2));
    rplPushData(zero_bint);
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_LT));

    // RESULT OF COMPARISON OPERATORS IS ALWAYS A SINT OR A SYMBOLIC
    WORDPTR numsign=rplPeekData(1);
    rplDropData(1);
    if(ISBINT(*numsign)) {
        if(*numsign!=MAKESINT(0)) {
        rplPushData(rplPeekData(2));
        rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NEG));
        rplOverwriteData(3,rplPeekData(1));
        rplDropData(1);
        sign^=1;
        }
    }

    // CHECK SIGN OF THE DENOMINATOR JUST IN CASE
    rplPushData(rplPeekData(1));
    rplPushData(zero_bint);
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_LT));

    // RESULT OF COMPARISON OPERATORS IS ALWAYS A SINT OR A SYMBOLIC
    WORDPTR densign=rplPeekData(1);
    rplDropData(1);
    if(ISBINT(*densign)) {
        if(*densign!=MAKESINT(0)) {
        rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NEG));
        sign^=1;
        }
    }

    rplOverwriteData(6,rplPeekData(2));
    rplOverwriteData(5,rplPeekData(1));
    rplDropData(4);

    return sign;

}




// REMOVE A SYMBOLIC OBJECT THAT IS EXPANDED IN THE STACK
// RETURNS THE SIZE OF THE OBJECT IN WORDS, CALLER HAS TO UPDATE
// ANY POINTERS INTO THE STACK THAT ARE > obj

BINT rplSymbRemoveInStack(WORDPTR *obj)
{
    WORDPTR *end=rplSymbSkipInStack(obj);
    BINT offset=obj-end;
    ++end;
    ++obj;

    while(obj!=DSTop) { *end=*obj; ++end; ++obj; }
    return offset;
}

// MAKE ROOM IN STACK TO INSERT nwords IMMEDIATELY BEFORE here
// RETURNS nwords
BINT rplSymbInsertInStack(WORDPTR *here, BINT nwords)
{
    rplExpandStack(nwords);
    if(Exceptions) return 0;

    WORDPTR *ptr=DSTop-1;

    while(ptr!=here) { ptr[nwords]=*ptr; --ptr; }

    return nwords;
}

// REMOVE nwords IMMEDIATELY AFTER here (INCLUDED)
// RETURNS nwords
BINT rplSymbDeleteInStack(WORDPTR *here, BINT nwords)
{
    here++;

    while(here!=DSTop) { here[-nwords]=*here; ++here; }

    return nwords;
}


// TAKES A SYMBOLIC OBJECT AND PERFORMS NUMERIC SIMPLIFICATION:
// DONE! A) IN ALL OPS, EXCEPT MUL AND ADD, IF ALL ARGUMENTS ARE NUMERIC, THEN PERFORM THE OPERATION AND REPLACE BY THEIR RESULT
// B) IN ADD, ALL NUMERIC VALUES ARE ADDED TOGETHER AND REPLACED BY THEIR RESULT
// DONE! C.1) IN MUL, ALL NUMERATOR NUMERIC VALUES ARE MULTIPLIED TOGETHER AND REPLACED BY THEIR RESULT
// DONE! C.2) IN MUL, ALL DENOMINATOR NUMERIC VALUES ARE MULTIPLIED TOGETHER AND REPLACED BY THEIR RESULT
// D) IN ADD, IF TWO TERMS ARE NUMERIC EXPRESSIONS, PERFORM A FRACTION ADDITION (N1/D1+N2/D2=(N1*D2+N2*D1)/(D1*D2)
// DONE! E) IN MUL, ALL NUMERATOR AND DENOMINATOR NUMERICS ARE DIVIDED BY THEIR GCD (FRACTION SIMPLIFICATION)


WORDPTR rplSymbNumericReduce(WORDPTR object)
{
    BINT numitems=rplSymbExplode(object);
    BINT f,changed,origprec;
    WORDPTR *stkptr,sobj,*endofstk;

    origprec=Context.prec;

    endofstk=DSTop-1-numitems;

// SCAN THE SYMBOLIC

    changed=1;

    while(changed) {

        stkptr=DSTop-1;
        changed=0;

    while(stkptr!=endofstk) {
        sobj=*stkptr;

        if(ISPROLOG(*sobj)||ISBINT(*sobj)) { --stkptr;  continue; }

        if(*sobj==MKOPCODE(LIB_OVERLOADABLE,OVR_MUL)) {
            // SCAN ALL NUMERIC FACTORS IN THE NUMERATOR AND MULTIPLY TOGETHER


            WORDPTR *number;
            BINT nargs=OPCODE(**(stkptr-1))-1,redargs=0;
            WORDPTR *argptr=stkptr-2,*savedstop;
            BINT simplified=0,den_is_one=0,neg=0;

            savedstop=DSTop;

            for(f=0;f<nargs;++f) {
                if(!ISNUMBER(**argptr)) {
                    // CHECK IF IT'S A NEGATIVE NUMBER
                    if(**argptr==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
                        if(ISNUMBER(**(argptr-2))) {
                            rplPushData(*(argptr-2));
                            // NEGATE THE NUMBER
                            Context.prec=REAL_PRECISION_MAX;
                            Context.traps|=MPD_Inexact;         // THROW AN EXCEPTION WHEN RESULT IS INEXACT

                            rplCallOvrOperator(OVR_NEG);
                            if(Exceptions) { DSTop=endofstk+1; return NULL; }

                            Context.prec=origprec;
                            Context.traps&=~MPD_Inexact;         // BACK TO NORMAL

                            // REMOVE THE ARGUMENT FROM THE LIST

                            WORDPTR *ptr,*endofobj=rplSymbSkipInStack(argptr);   // POINT TO THE NEXT OBJECT
                            ptr=endofobj+1;
                            ++argptr;
                            stkptr-=(argptr-ptr);
                            // NOW CLOSE THE GAP
                            while(argptr!=DSTop) { *ptr=*argptr; ++argptr; ++ptr; }
                            DSTop=ptr;
                            argptr=endofobj;

                            // ARGUMENT WAS COMPLETELY REMOVED, NOW REDUCE THE ARGUMENT COUNT

                            ++redargs;
                            continue;
                            }
                        else {
                            // IT'S A NEGATIVE EXPRESSION, EXTRACT THE NEGATIVE SIGN AS A NUMERIC QUANTITY
                            neg^=1;
                            // REMOVE THE NEGATION
                            rplSymbDeleteInStack(argptr,2);
                            argptr-=2;
                            stkptr-=2;
                            DSTop-=2;

                        }
                    }
                }
                else {
                    // THIS IS A NUMBER
                    rplPushData(*(argptr));

                    // REMOVE THE ARGUMENT FROM THE LIST

                    WORDPTR *ptr,*endofobj=rplSymbSkipInStack(argptr);   // POINT TO THE NEXT OBJECT
                    ptr=endofobj+1;
                    ++argptr;
                    stkptr-=(argptr-ptr);
                    // NOW CLOSE THE GAP
                    while(argptr!=DSTop) { *ptr=*argptr; ++argptr; ++ptr; }
                    DSTop=ptr;
                    argptr=endofobj;

                    // ARGUMENT WAS COMPLETELY REMOVED, NOW REDUCE THE ARGUMENT COUNT

                    ++redargs;
                    continue;

                }


                argptr=rplSymbSkipInStack(argptr);
            }

            // HERE WE HAVE redargs VALUES IN THE STACK THAT NEED TO BE MULTIPLIED TOGETHER
            if(redargs>0) {
            Context.prec=REAL_PRECISION_MAX;
            Context.traps|=MPD_Inexact;         // THROW AN EXCEPTION WHEN RESULT IS INEXACT
            for(f=1;f<redargs;++f) {
                rplCallOvrOperator(OVR_MUL);
                if(Exceptions) { DSTop=endofstk+1; return NULL; }
            }



            Context.prec=origprec;
            Context.traps&=~MPD_Inexact;         // BACK TO NORMAL
            }
            else rplPushData(one_bint);     //  IF NO NUMERATOR, THEN MAKE IT = 1

            // HERE WE HAVE A NUMERATOR RESULT IN THE STACK! KEEP IT THERE FOR NOW

            // SCAN ALL NUMERIC FACTORS IN THE DENOMINATOR AND MULTIPLY TOGETHER
            BINT reddenom=0;
            argptr=stkptr-2;

            for(f=0;f<nargs-redargs;++f) {
                if(**argptr==MKOPCODE(LIB_OVERLOADABLE,OVR_INV)) {

                if(!ISNUMBER(**(argptr-2))) {
                    // CHECK IF IT'S A NEGATIVE NUMBER
                    if(**(argptr-2)==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
                        if(ISNUMBER(**(argptr-4))) {
                            rplPushData(*(argptr-4));
                            // NEGATE THE NUMBER
                            Context.prec=REAL_PRECISION_MAX;
                            Context.traps|=MPD_Inexact;         // THROW AN EXCEPTION WHEN RESULT IS INEXACT

                            rplCallOvrOperator(OVR_NEG);
                            if(Exceptions) { DSTop=endofstk+1; return NULL; }

                            Context.prec=origprec;
                            Context.traps&=~MPD_Inexact;         // BACK TO NORMAL

                            // REMOVE THE ARGUMENT FROM THE LIST

                            WORDPTR *ptr,*endofobj=rplSymbSkipInStack(argptr);   // POINT TO THE NEXT OBJECT
                            ptr=endofobj+1;
                            ++argptr;
                            stkptr-=(argptr-ptr);
                            // NOW CLOSE THE GAP
                            while(argptr!=DSTop) { *ptr=*argptr; ++argptr; ++ptr; }
                            DSTop=ptr;
                            argptr=endofobj;

                            // ARGUMENT WAS COMPLETELY REMOVED, NOW REDUCE THE ARGUMENT COUNT

                            ++reddenom;
                            continue;
                            }
                        else {
                            // IT'S A NEGATIVE EXPRESSION, EXTRACT THE SIGN
                            neg^=1;
                            // REMOVE THE NEGATION
                            rplSymbDeleteInStack(argptr-2,2);
                            argptr-=2;
                            stkptr-=2;
                            DSTop-=2;
                        }
                    }
                }
                else {
                    // THIS IS A NUMBER
                    rplPushData(*(argptr-2));

                    // REMOVE THE ARGUMENT FROM THE LIST

                    WORDPTR *ptr,*endofobj=rplSymbSkipInStack(argptr);   // POINT TO THE NEXT OBJECT
                    ptr=endofobj+1;
                    ++argptr;
                    stkptr-=(argptr-ptr);
                    // NOW CLOSE THE GAP
                    while(argptr!=DSTop) { *ptr=*argptr; ++argptr; ++ptr; }
                    DSTop=ptr;
                    argptr=endofobj;

                    // ARGUMENT WAS COMPLETELY REMOVED, NOW REDUCE THE ARGUMENT COUNT

                    ++reddenom;
                    continue;

                }
                }


                argptr=rplSymbSkipInStack(argptr);
            }

            // HERE WE HAVE reddenom VALUES IN THE STACK THAT NEED TO BE MULTIPLIED TOGETHER
            if(reddenom>0) {

            Context.prec=REAL_PRECISION_MAX;
            Context.traps|=MPD_Inexact;         // THROW AN EXCEPTION WHEN RESULT IS INEXACT
            for(f=1;f<reddenom;++f) {
                rplCallOvrOperator(OVR_MUL);
                if(Exceptions) { DSTop=endofstk+1; return NULL; }
            }

            Context.prec=origprec;
            Context.traps&=~MPD_Inexact;         // BACK TO NORMAL

            // DONE, WE HAVE NUMERATOR AND DENOMINATOR IN THE STACK

            // FIND THE GCD OF THE NUMERATOR AND DENOMINATOR

            // DIVIDE BOTH BY THE GCD
            simplified=rplFractionSimplify();

            }

            // PUT BOTH NUMBERS BACK IN PLACE

            {

                if(redargs>0) {

                    BINT n=1+((reddenom>0)? 1:0);
                    // IF NUMERATOR IS NEGATIVE, STORE AS POSITIVE AND SET neg
                    if(ISBINT(*rplPeekData(n))) {
                        BINT64 nnum=rplReadBINT(rplPeekData(n));
                        // MARK TO ADD THE SIGN LATER
                        if(nnum<0) {
                            neg^=1;
                        // KEEP THE NUMERATOR POSITIVE
                        WORDPTR newnum=rplNewBINT(-nnum,DECBINT);
                        if(!newnum) { DSTop=endofstk+1; return NULL; }
                        rplOverwriteData(n,newnum);
                        }

                    } else {
                        if(ISREAL(*rplPeekData(n))) {
                            mpd_t number;
                            rplReadReal(rplPeekData(n),&number);
                            if(mpd_isnegative(&number)) {
                                number.flags^=MPD_NEG;
                                neg^=1;
                                number.flags^=MPD_NEG;
                                WORDPTR newnum=rplNewReal(&number);
                                if(!newnum) { DSTop=endofstk+1; return NULL; }
                                rplOverwriteData(n,newnum);
                            }
                        }
                    }


                    // IF THERE WERE ANY FACTORS IN THE NUMERATOR, REPLACE WITH THE NEW RESULT
                WORDPTR *ptr=DSTop-1;


                // MAKE ROOM
                while(ptr!=stkptr-2) { ptr[1]=*ptr; --ptr; }

                ++stkptr;
                ++DSTop;

                *(stkptr-2)=rplPeekData(1+((reddenom>0)? 1:0)); // STORE THE NUMERATOR

                }

                if(reddenom>0) {
                    // IF THERE WERE ANY FACTORS IN THE DENOMINATOR, ADD THE RESULT

                    // IF DENOMINATOR IS ONE, THEN DON'T INCLUDE IT IN THE OBJECT

                    if(ISBINT(*rplPeekData(1))) {
                        BINT64 denom=rplReadBINT(rplPeekData(1));
                        if(denom==1) den_is_one=1;
                    } else {
                        if(ISREAL(*rplPeekData(1))) {
                            mpd_t number;
                            rplReadReal(rplPeekData(1),&number);
                            rplOneToRReg(0);
                            if(mpd_cmp(&number,&RReg[0],&Context)==0) den_is_one=1;
                        }
                    }


                    if(!den_is_one) {
                        // ONLY INSERT IN THE OBJECT IF THE DENOMINATOR IS NOT ONE

                        WORDPTR *endofobj=stkptr-2;
                        for(f=0;f<nargs-redargs-reddenom+((redargs>0)? 1:0);++f)
                            endofobj=rplSymbSkipInStack(endofobj);
                        WORDPTR *ptr=stkptr-2;
                        // FIND THE FIRST FACTOR IN THE DENOMINATOR
                        while(ptr!=endofobj) {
                            if(**ptr==MKOPCODE(LIB_OVERLOADABLE,OVR_INV)) break;
                            ptr=rplSymbSkipInStack(ptr);
                        }

                        // MAKE ROOM
                        endofobj=ptr;
                        ptr=DSTop-1;
                        while(ptr!=endofobj) { ptr[3]=*ptr; --ptr; }

                        stkptr+=3;
                        DSTop+=3;
                        ptr[1]=rplPeekData(1);  // STORE THE DENOMINATOR
                        ptr[2]=two_bint;
                        ptr[3]=inverse_opcode;

                    }
                    --DSTop;

                }

                DSTop--;

                if(neg) {
                    // HERE stkptr IS POINTING TO THE MULTIPLICATION
                    rplSymbInsertInStack(stkptr-2,2);
                    *stkptr=uminus_opcode;
                    *(stkptr-1)=two_bint;
                    stkptr+=2;
                    DSTop+=2;
                }

                if(redargs+reddenom) {
                    // UPDATE THE ARGUMENT COUNT
                    BINT newcount=nargs-redargs-reddenom;
                    if(redargs) ++newcount;
                    if(reddenom) {
                        ++newcount;
                        if(den_is_one) --newcount;
                    }


                    if(newcount<2)
                    {
                        // SINGLE ARGUMENT, SO REMOVE THE MULTIPLICATION
                        WORDPTR *ptr=stkptr-1;
                        while(ptr!=DSTop) { *ptr=*(ptr+2); ++ptr; }
                        DSTop-=2;
                        stkptr-=2;

                    }
                    else {
                        WORDPTR newnumber=rplNewSINT(newcount+1,DECBINT);
                        if(!newnumber) { DSTop=endofstk+1; return NULL; }
                        *(stkptr-1)=newnumber;
                    }



                    if(redargs>1 || reddenom>1 || simplified) changed=1;
                }
                --stkptr;
                continue;
            }


        }   // END OF MULTIPLICATION


        if(*sobj==MKOPCODE(LIB_OVERLOADABLE,OVR_ADD)) {
            // SCAN ALL NUMERIC FACTORS AND ADD TOGETHER (INCLUDING FRACTIONS)

            BINT nargs=OPCODE(**(stkptr-1))-1;
            WORDPTR *argptr=stkptr-2;

            WORDPTR *firstnum=NULL,*secondnum=NULL;

            for(f=0;f<nargs;++f)
            {
                if(rplSymbIsFractionInStack(argptr)) {
                    if(!firstnum) { firstnum=argptr; }
                    else {
                    secondnum=argptr;
                    break;
                    }
                }
                argptr=rplSymbSkipInStack(argptr);

            }

            if( (firstnum==NULL) || (secondnum==NULL) ) { --stkptr; continue; }

            // HERE WE HAVE 2 FRACTIONS OR NUMBERS READY TO ADD

            rplSymbFractionExtractNumDen(firstnum);
            rplSymbFractionExtractNumDen(secondnum);

            // NOW COMPUTE THE RESULT

            BINT isnegative=rplSymbFractionAdd();

            // AND REPLACE IT IN THE ORIGINAL

            // REMOVE ORIGINAL ARGUMENTS
            BINT offset;
            offset=rplSymbRemoveInStack(firstnum);
            DSTop-=offset;
            stkptr-=offset;
            firstnum-=offset;
            offset=rplSymbRemoveInStack(secondnum);
            DSTop-=offset;
            stkptr-=offset;
            firstnum-=offset;

            // AND INSERT THE NEW ONE

            BINT den_is_one=0;

            // CHECK IF DENOMINATOR IS ONE
            if(ISBINT(*rplPeekData(1))) {
                BINT64 denom=rplReadBINT(rplPeekData(1));
                if(denom==1) den_is_one=1;
            } else {
                if(ISREAL(*rplPeekData(1))) {
                    mpd_t number;
                    rplReadReal(rplPeekData(1),&number);
                    rplOneToRReg(0);
                    if(mpd_cmp(&number,&RReg[0],&Context)==0) den_is_one=1;
                }
            }



            offset=rplSymbInsertInStack(firstnum,1+((den_is_one)? 0:5)+(isnegative? 2:0));
            stkptr+=offset;
            DSTop+=offset;
            firstnum+=offset;

            // HERE FIRSTNUM POINTS TO THE START OF THE HOLE WE JUST OPENED
            if(isnegative) {
                if(den_is_one)
                {
                    firstnum[0]=uminus_opcode;
                    firstnum[-1]=two_bint;
                    firstnum[-2]=rplPeekData(2);
                }
                else {
                    firstnum[0]=uminus_opcode;
                    firstnum[-1]=two_bint;
                    firstnum[-2]=mul_opcode;
                    firstnum[-3]=three_bint;
                    firstnum[-4]=rplPeekData(2);
                    firstnum[-5]=inverse_opcode;
                    firstnum[-6]=two_bint;
                    firstnum[-7]=rplPeekData(1);
                }

            }
            else {
            if(den_is_one)
            {
                *firstnum=rplPeekData(2);
            }
            else {
                firstnum[0]=mul_opcode;
                firstnum[-1]=three_bint;
                firstnum[-2]=rplPeekData(2);
                firstnum[-3]=inverse_opcode;
                firstnum[-4]=two_bint;
                firstnum[-5]=rplPeekData(1);
            }
            }

            DSTop-=2;

            // UPDATE THE ARGUMENT COUNT


            if(nargs-1<2) {
                // REMOVE THE ADDITION IF THERE'S ONLY ONE ARGUMENT
                offset=rplSymbDeleteInStack(stkptr,2);
                DSTop-=offset;
                stkptr-=offset;
            }
            else {
                WORDPTR newobj=rplNewSINT(nargs,DECBINT);
                if(!newobj) return 0;
                *(stkptr-1)=newobj;
                --stkptr;
            }


            changed=1;
            continue;
        }



        if( (*sobj!=MKOPCODE(LIB_OVERLOADABLE,OVR_MUL)) && (*sobj!=MKOPCODE(LIB_OVERLOADABLE,OVR_ADD)) && (*sobj!=MKOPCODE(LIB_OVERLOADABLE,OVR_INV)) && (*sobj!=MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS))) {
                // EXCEPT ADDITION AND MULTIPLICATIONS, CHECK IF ALL ARGUMENTS ARE NUMERIC AND APPLY THE OPERATOR

                BINT nargs=OPCODE(**(stkptr-1))-1;
                WORDPTR *argptr=stkptr-2,*savedstop;
                BINT notanumber=0;
                for(f=0;f<nargs;++f) {
                    if(!ISNUMBER(**argptr)) {
                        // CHECK IF IT'S A NEGATIVE NUMBER
                        if(**argptr==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
                            if(!ISNUMBER(**(argptr-2))) {
                                notanumber=1;
                                break; }
                        }
                        else {
                        notanumber=1;
                        break; }
                    }
                    argptr=rplSymbSkipInStack(argptr);
                }

                if(notanumber) { --stkptr; continue; }

                savedstop=DSTop;

                // HERE ALL ARGUMENTS ARE SIMPLE NUMBERS, APPLY THE OPERATOR
                argptr=stkptr-2;
                for(f=0;f<nargs;++f) {
                    if(ISNUMBER(**argptr)) rplPushData(*argptr);
                    else {
                        // CHECK IF IT'S A NEGATIVE NUMBER
                        if(**argptr==MKOPCODE(LIB_OVERLOADABLE,OVR_UMINUS)) {
                            // WE KNOW FROM PREVIOUS LOOP THAT A NUMBER FOLLOWS
                            rplPushData(*(argptr-2));

                            // NEGATE THE NUMBER
                            Context.prec=REAL_PRECISION_MAX;
                            Context.traps|=MPD_Inexact;         // THROW AN EXCEPTION WHEN RESULT IS INEXACT

                            rplCallOvrOperator(OVR_NEG);
                            if(Exceptions) { DSTop=endofstk+1; return NULL; }

                            Context.prec=origprec;
                            Context.traps&=~MPD_Inexact;         // BACK TO NORMAL

                        }
                   }
                    argptr=rplSymbSkipInStack(argptr);

                }

                // CALL THE MAIN OPERATOR
                Context.prec=REAL_PRECISION_MAX;
                Context.traps|=MPD_Inexact;         // THROW AN EXCEPTION WHEN RESULT IS INEXACT

                rplCallOvrOperator(**stkptr);

                Context.prec=origprec;
                Context.traps&=~MPD_Inexact;         // BACK TO NORMAL

                if(!( (Exceptions>>16)&MPD_Inexact)) {

                    // THERE WERE EXCEPTIONS AND IS NOT BECAUSE OF INEXACT --> RETURN
                    if(Exceptions) { DSTop=endofstk+1; return NULL; }

                    // REPLACE A SINGLE ARGUMENT

                    // TODO: IF THE RESULT IS SYMBOLIC, NEED TO EXPAND BEFORE INSERTING, SO ADDITIONAL SIMPLIFICATION CAN BE DONE INSIDE

                    WORDPTR *ptr,*endofobj=rplSymbSkipInStack(stkptr);   // POINT TO THE NEXT OBJECT
                    ptr=endofobj+1;
                    *ptr=rplPeekData(1);
                    --DSTop;
                    ++ptr;
                    ++stkptr;
                    // NOW CLOSE THE GAP
                    while(stkptr!=DSTop) { *ptr=*stkptr; ++stkptr; ++ptr; }
                    DSTop=ptr;
                    stkptr=endofobj;
                    changed=1;
                    continue;

                }
                else {
                    // THE EXCEPTION WAS INEXACT
                    Exceptions&=0xffff; // MASK OUT ALL MATH EXCEPTIONS
                    DSTop=savedstop;    // CLEANUP THE STACK
                }



        }
        --stkptr;
        }

    }

// ...

    if(Exceptions) {
        DSTop=endofstk+1;
        return NULL;
    }

    WORDPTR finalsymb=rplSymbImplode(DSTop-1-endofstk);

    DSTop=endofstk+1;
    if(Exceptions) return NULL;

    return finalsymb;

}





// ATTEMPTS TO MATCH A RULE THAT IS A SERIES OF TERMS (OR FACTORS)
// THERE CAN BE 3 RESULTS:
// 0 = NO MATCH
// 1 = EXACT MATCH
// 2 = PARTIAL MATCH, THERE ARE TERMS THAT MATCH, AND SOME OTHER TERMS LEFT OVER.
//     THE RESULT IS COMPOSED OF A MATCHING AND A NON-MATCHING SET OF TERMS.

// THIS FUNCTION ASSUMES Opcode IS ASSOCIATIVE AND COMMUTATIVE.

BINT rplSymbCommutativeMatch(WORD Opcode,WORDPTR rulelist,WORDPTR objlist)
{
        // Opcode = MAIN OPCODE IN rulelist AND objlist (CAN BE OVR_ADD OR OVR_MUL)
        // rulelist = SYMBOLIC OBJECT { Opcode arg1 arg2 ... argN } TO MATCH FROM
        // objlist = OBJECT TO MATCH, IF IT'S NOT THE SAME OPERATION, IT'S CONSIDERED AS A SINGLE TERM { Opcode objlist }

    // EXPLODE objlist ON THE STACK

    // MATCH FIRST TERM IN rulelist WITH ANY TERM IN objlist, SORT objlist BRINGING THE TERM TO THE START OF THE LIST
    // MATCH NEXT TERM IN rulelist WITH ANY OF THE REMAINDER TERMS IN objlist, SORTING AS NEEDED
    // IF THE TERM IN rulelist IS A SPECIAL IDENT, MATCH AS MANY TERMS IN objlist AS REQUESTED AND DEFINE THE VARIABLE
    // AT THE END OF rulelist, IF THERE ARE ANY UNMATCHED TERMS THERE IS A PARTIAL MATCH

}


// RECURSIVE SYMBOLIC MATCH AND APPLY

// MATCH ONE TO ANY: TAKE ONE RULE AND MATCH IT WITH ALL TERMS IN A LIST, RECURSE AS NEEDED AND APPLY
// ALL VARIABLE STORAGE IS WITHIN A LOCAL ENVIRONMENT
// LAMS:
// 1 = RULE TO MATCH
// 2 = LIST OF OBJECTS,
// 3 = PTR TO CURRENT RULE OBJECT
// 4 = PTR TO CURRENT OBJECT
// 5 = MATCH STATUS,
// 6 = NUMBER OF OBJECTS IN CURRENT OBJECT
// 5 ... N = NAMED LAMS WITH ALL SPECIAL IDENTS IN THE RULE, WITH THEIR ASSIGNED VALUES SO FAR
// N+1 ... M = LIST OF OBJECTS IN CURRENT OBJECT, FOR SORTING

// LOGIC:
// CREATE A NEW ENVIRONMENT
// INITIALIZE ALL VARIABLES
// EXPLODE THE OBJECT
// CURRENT RULE OBJECT = START OF RULE
// TAKE CURRENT OBJECT AND MATCH CURRENT RULE...
//              IF RULE IS ATOMIC {
//                      IF RULE IS SPECIAL IDENT AND IS ALREADY DEFINED, TAKE VALUE OF IDENT + OBJECT AND GOTO LOGIC:
//                      IF RULE IS A SPECIAL IDENT NOT PREVIOUSLY DEFINED, DEFINE IT
//                      IF OBJECT IS ATOMIC, CHECK MATCH OR NOT
//                      IF OBJECT IS A SYMBOLIC AND OPCODE MATCHES RULE OPCODE, TAKE CURRENT RULE + CURRENT OBJECT AND GOTO LOGIC: (TO START WITH THE CURRENT OBJECT EXPLODED)
//              }
//              IF RULE IS SYMBOLIC OPERATION {
//                      EXPLODE THE RULE, AND DO A LOOP TAKING ONE RULE OBJECT TO MATCH WITH EACH OBJECT

void rplSymbRuleApply(WORDPTR expression,WORDPTR rule)
{
    // CREATE A SEPARATE LOCAL ENVIRONMENT
    rplCreateLAMEnvironment(expression);

    rplCreateLAM(nulllam_ident,zero_bint);      // LAM1 = MATCH RESULT IS THE FIRST LAM

    if(Exceptions) return;

    if(ISSYMBOLIC(expression)) expression=rplSymbUnwrap(expression);
    if(ISSYMBOLIC(rule)) rule=rplSymbUnwrap(rule);

    if(!rule || !expression) return;
    BINT rulesize=rplObjSize(rule),expsize=rplObjSize(expression);

    rplCreateLAM(nulllam_ident,expression);     // LAM2 = ORIGINAL EXPRESSION
    rplCreateLAM(nulllam_ident,rule);           // LAM3 = ORIGINAL RULE
    rplCreateLAM(nulllam_ident,expression);     // LAM4 = CURRENT EXPRESSION POINTER
    rplCreateLAM(nulllam_ident,rule);           // LAM5 = CURRENT RULE POINTER

    while(rplGetLAMn(5)-rplGetLAMn(2)<rulesize) {
//        if()



    }

    if(Exceptions) return;


}


// TAKES A SYMBOLIC FROM THE STACK AND:
// CHANGE THE SYMBOLIC TO CANONICAL FORM.
// ALL NUMERICAL TERMS ARE ADDED TOGETHER
// ALL NUMERICAL FACTORS IN THE NUMERATOR ARE MULTIPLIED TOGETHER
// ALL NUMERICAL FACTORS IN THE DENOMINATOR ARE MULTIPLIED TOGETHER
// SYMBOLIC FRACTIONS ARE REDUCED


void rplSymbAutoSimplify()
{

    WORDPTR newobj=rplSymbCanonicalForm(rplPeekData(1));
    if(newobj) rplOverwriteData(1,newobj);
    else return;

    newobj=rplSymbNumericReduce(rplPeekData(1));

    if(newobj) rplOverwriteData(1,newobj);
    return;
}
