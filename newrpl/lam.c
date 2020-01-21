/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "hal.h"
#include "libraries.h"

// THESE ARE THE ONLY CHARACTERS THAT ARE FORBIDDEN IN AN IDENTIFIER
// ALSO FORBIDDEN IS THE ARGUMENT SEPARATOR IF NOT INCLUDED IN THIS LIST
const char const forbiddenChars[] =
        "+-*/\\{}[]()#!^;:<>=, \"\'_`@|√«»≤≥≠∡∞";

// SUBSCRIPT CHARS USED TO DESCRIBE VARIABLE ATTRIBUTES
const char const subscriptChars[] = "₀₁₂₃₄₅₆₇₈₉";

// GROW THE LAM STACK

void growLAMs(WORD newtotalsize)
{
    WORDPTR *newlam;
    BINT gc_done = 0;

    do {
        newtotalsize = (newtotalsize + 1023) & ~1023;

        newlam = halGrowMemory(MEM_AREA_LAM, LAMs, newtotalsize);

        if(!newlam) {
            if(!gc_done) {
                rplGCollect();
                ++gc_done;
            }
            else {
                rplException(EX_OUTOFMEM);
                return;
            }
        }

    }
    while(!newlam);

    LAMTop = LAMTop - LAMs + newlam;
    nLAMBase = nLAMBase - LAMs + newlam;
    LAMs = newlam;
    LAMSize = newtotalsize;
}

void shrinkLAMs(WORD newtotalsize)
{
    WORDPTR *newlam;

    newtotalsize = (newtotalsize + 1023) & ~1023;

    newlam = halGrowMemory(MEM_AREA_LAM, LAMs, newtotalsize);

    if(!newlam) {
        rplException(EX_OUTOFMEM);
        return;
    }

    LAMTop = LAMTop - LAMs + newlam;
    nLAMBase = nLAMBase - LAMs + newlam;
    LAMs = newlam;
    LAMSize = newtotalsize;
}

// LAM STACK IS INCREASE AFTER FOR STORE, DECREASE BEFORE FOR READ
// RETURN THE LAM NUMBER IN THE CURRENT ENVIRONMENT (FOR USE WITH FAST GETLAMn FUNCTIONS)
BINT rplCreateLAM(WORDPTR nameobj, WORDPTR value)
{
    *LAMTop++ = nameobj;
    *LAMTop++ = value;

    if(LAMSize <= LAMTop - LAMs + LAMSLACK)
        growLAMs((WORD) (LAMTop - LAMs + LAMSLACK + 1024));
    if(Exceptions)
        return 0;
    return (LAMTop - 2 - nLAMBase) >> 1;

}

// CREATE A NEW TOP LAM ENVIRONMENT, SET THE OWNER TO THE GIVEN OBJECT
void rplCreateLAMEnvironment(WORDPTR owner)
{
    nLAMBase = LAMTop;
    rplCreateLAM((WORDPTR) lam_baseseco_bint, owner);
}

BINT rplCompareIDENTByName(WORDPTR id1, BYTEPTR name, BYTEPTR nameend)
{
    BINT len = nameend - name;
    BINT nwords = (len + 3) >> 2;
    BINT extra = (nwords << 2) - len;
    if((((*id1) & MKPROLOG(0xff1, 0xfffff)) != MKPROLOG(DOIDENT, nwords))
            && (((*id1) & MKPROLOG(0xff1, 0xfffff)) != MKPROLOG(DOIDENTATTR,
                    nwords + 1)))
        return 0;
    BYTEPTR ptr = (BYTEPTR) (id1 + 1);
    while(len) {
        if(*ptr != *name)
            return 0;
        ++ptr;
        ++name;
        --len;
    }

// NAMES ARE IDENTICAL UP TO HERE
// CHECK IF THE STORED NAME HAS THE SAME PADDING
    while(extra) {
        if(*ptr)
            return 0;
        ++ptr;
        --extra;
    }

    return 1;
}

// COMPARE OBJECTS FOR EQUALITY IN THEIR DEFINITION
BINT rplCompareIDENT(WORDPTR id1, WORDPTR id2)
{
    BINT nwords, nwords2;

    nwords = rplObjSize(id1);
    if(LIBNUM(*id1) & HASATTR_BIT)
        --nwords;       // DO NOT COMPARE ATTRIBUTES
    nwords2 = rplObjSize(id2);
    if(LIBNUM(*id2) & HASATTR_BIT)
        --nwords2;      // DO NOT COMPARE ATTRIBUTES

// ADDED THIS SPECIAL CASE FOR IDENTS WITH VARIOUS FLAGS TO BE TREATED AS SAME
    if(ISIDENT(*id1) && ISIDENT(*id2)) {
        if(nwords != nwords2)
            return 0;
        ++id1;
        ++id2;
        --nwords;
    }

    while(nwords) {
        if(*id1 != *id2)
            return 0;
        ++id1;
        ++id2;
        --nwords;
    }
    return 1;
}

// GET THE ATTRIBUTES WORD OF AN IDENT, OR 0 IF IT DOESN'T HAVE ANY

WORD rplGetIdentAttr(WORDPTR name)
{
    if(IDENTHASATTR(*name))
        return name[OBJSIZE(*name)];
    return 0;
}

// RETURN A NEW IDENT WITH THE ATTRIBUTES CHANGED
// ONLY BITS THAT ARE ONE IN attrmask WILL BE CHANGED
// USES ONE SCRATCHPOINTER
WORDPTR rplSetIdentAttr(WORDPTR name, WORD attr, WORD attrmask)
{
    WORDPTR newobj;
    if(LIBNUM(*name) & HASATTR_BIT)
        newobj = rplMakeNewCopy(name);
    else {
        newobj = rplAllocTempOb(OBJSIZE(*name) + 1);
        if(!newobj)
            return 0;
        rplCopyObject(newobj, name);
        newobj[OBJSIZE(*name) + 1] = 0;
        newobj[0]++;
    }
    if(!newobj)
        return 0;
    // HERE WE HAVE A NEW IDENT READY TO ACCEPT ATTRIBUTES
    newobj[0] |= MKPROLOG(HASATTR_BIT, 0);
    newobj[OBJSIZE(*newobj)] = (newobj[OBJSIZE(*newobj)] & (~attrmask)) | attr;
    return newobj;
}

// COMPARE OBJECTS FOR EQUALITY IN THEIR DEFINITION
BINT rplCompareObjects(WORDPTR id1, WORDPTR id2)
{
    if(id1 == id2)
        return 1;

    BINT nwords;

    nwords = rplObjSize(id1);

    while(nwords) {
        if(*id1 != *id2)
            return 0;
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
    WORDPTR *ltop = LAMTop, *stop =
            scanparents ? LAMs : (nLAMBase ? nLAMBase : LAMs);

    while(ltop > stop) {
        ltop -= 2;
        if(rplCompareIDENTByName(*ltop, name, name + len))
            return ltop;
        if(**ltop == LAM_PRIVATEVAR)
            return 0;
    }
    return 0;
}

WORDPTR *rplFindLAM(WORDPTR nameobj, BINT scanparents)
{
    WORDPTR *ltop = LAMTop, *stop =
            scanparents ? LAMs : (nLAMBase ? nLAMBase : LAMs);
    while(ltop > stop) {
        ltop -= 2;
        if(rplCompareIDENT(nameobj, *ltop))
            return ltop;
        if(**ltop == LAM_PRIVATEVAR)
            return 0;
    }
    return 0;
}

// RECLAIMS A LAM VALUE, FROM CURRENT SECO OR PARENTS

WORDPTR rplGetLAM(WORDPTR nameobj)
{
    WORDPTR *ltop = LAMTop;
    while(ltop > LAMs) {
        ltop -= 2;
        if(rplCompareIDENT(nameobj, *ltop))
            return *(ltop + 1);
        if(**ltop == LAM_PRIVATEVAR)
            return 0;
    }
    return 0;
}

// VERY FAST GETLAM, NO ERROR CHECKS!
// ONLY USED BY SYSTEM LIBRARIES
inline WORDPTR *rplGetLAMn(BINT idx)
{
    return nLAMBase + 2 * idx + 1;
}

// RETURN A POINTER TO THE THE NAME OF THE LAM, INSTEAD OF ITS CONTENTS
inline WORDPTR *rplGetLAMnName(BINT idx)
{
    return nLAMBase + 2 * idx;
}

// VERY FAST GETLAM, NO ERROR CHECKS!
// ONLY USED BY SYSTEM LIBRARIES
// GET THE CONTENT OF A LAM IN A GIVEN ENVIRONMENT
inline WORDPTR *rplGetLAMnEnv(WORDPTR * LAMEnv, BINT idx)
{
    return (WORDPTR *) (LAMEnv + 2 * idx + 1);
}

// RETURN THE NAME OF THE LAM, INSTEAD OF ITS CONTENTS
// IN THE GIVEN ENVIRONMENT
inline WORDPTR *rplGetLAMnNameEnv(WORDPTR * LAMEnv, BINT idx)
{
    return (WORDPTR *) (LAMEnv + 2 * idx);
}

// FAST PUTLAM, NO ERROR CHECKS!
inline void rplPutLAMn(BINT idx, WORDPTR object)
{
    nLAMBase[2 * idx + 1] = object;
}

// COUNT HOW MANY LAMS IN THE GIVEN ENVIRONMENT
// LAMS ARE NUMBERED FROM 1 TO THE RETURNED NUMBER (INCLUSIVE)
// IF GIVEN ENVIRONMENT IS NULL, RETURN COUNT ON THE TOP ENVIRONMENT
BINT rplLAMCount(WORDPTR * LAMEnvironment)
{
    // FIND THE END OF THE GIVEN ENVIRONMENT
    if(!LAMEnvironment)
        LAMEnvironment = nLAMBase;
    WORDPTR *endofenv = LAMEnvironment;
    endofenv += 2;
    while(endofenv < LAMTop) {
        if(*endofenv == lam_baseseco_bint)
            break;
        endofenv += 2;
    }
    return ((BINT) (endofenv - LAMEnvironment - 2)) >> 1;
}

// REMOVE ALL LAMS CREATED BY THE GIVEN SECONDARY AND BELOW
// OR JUST THE LAST ENVIRONMENT IF currentseco==0
void rplCleanupLAMs(WORDPTR currentseco)
{
    WORDPTR *ltop = LAMTop;
    while(ltop > LAMs) {
        ltop -= 2;
        if(**ltop == LAM_ENVOWNER) {
            if(((*(ltop + 1)) == currentseco) || (currentseco == 0)) {
                LAMTop = ltop;
                nLAMBase = rplGetNextLAMEnv(LAMTop);
                if(!nLAMBase)
                    nLAMBase = LAMs;
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

WORDPTR *rplGetNextLAMEnv(WORDPTR * startpoint)
{
    while(startpoint > LAMs) {
        startpoint -= 2;
        if(**startpoint == LAM_ENVOWNER)
            return startpoint;
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
    WORDPTR *rsptr = RSTop - 1;
    WORDPTR seco = 0;

    while(rsptr >= RStk) {
        if(ISPROLOG(**rsptr) && ((LIBNUM(**rsptr) == SECO)
                    || (LIBNUM(**rsptr) == DOCOL))) {
            seco = *rsptr;
            break;
        }
        if((**rsptr == (CMD_OVR_EVAL)) || (**rsptr == (CMD_OVR_EVAL1))
                || (**rsptr == (CMD_OVR_XEQ))) {
            // WE ARE EXECUTING A SECONDARY THAT WAS 'EVAL'd, NEED A NEW ENVIRONMENT
            break;
        }
        if(ISPROLOG(**rsptr) && (LIBNUM(**rsptr) == DOIDENTEVAL)) {
            // WE ARE EXECUTING A SECONDARY THAT WAS DIRECTLY EXECUTED FROM AN IDENT, NEED A NEW ENVIRONMENT
            break;
        }

        --rsptr;
    }

    // NOW DETERMINE IF THE CURRENT SECONDARY HAS AN ENVIRONMENT
    if(!seco) {
        // NOT IN A SECONDARY OR IN A SECONDARY INDIRECTLY EXECUTED WITH EVAL

        if(nLAMBase != LAMTop && nLAMBase >= LAMs) {
            // THERE IS AN EXISTING ENVIRONMENT
            rsptr = RSTop - 1;
            while((rsptr >= RStk) && (**rsptr == CMD_ABND))
                --rsptr;

            if(rsptr < RStk)
                return 0;       // THERE'S NO RETURN ADDRESS, WE ARE EXECUTING FREE COMMANDS FROM THE COMMAND LINE
            if((**rsptr == (CMD_OVR_EVAL)) || (**rsptr == (CMD_OVR_EVAL1))
                    || (**rsptr == (CMD_OVR_XEQ))) {
                if(*(nLAMBase + 1) != *rsptr) {
                    // WE ARE EXECUTING A SECONDARY THAT WAS 'EVAL'd, NEED A NEW ENVIRONMENT
                    return 1;
                }
            }
            if(ISPROLOG(**rsptr) && (LIBNUM(**rsptr) == DOIDENTEVAL)) {
                if(*(nLAMBase + 1) != *rsptr) {
                    // WE ARE EXECUTING A SECONDARY THAT WAS DIRECTLY EXECUTED FROM AN IDENT, NEED A NEW ENVIRONMENT
                    return 1;
                }
            }

            return 0;
        }

        return 1;       // WE ARE NOT INSIDE ANY SECONDARY, SO AN ENVIRONMENT IS NEEDED
    }

    if(nLAMBase >= LAMs && nLAMBase < LAMTop) {
        if((ISPROLOG(**(nLAMBase + 1)) && ((LIBNUM(**(nLAMBase + 1)) == SECO)
                        || (LIBNUM(**(nLAMBase + 1)) == DOCOL)))
                || (**(nLAMBase + 1) == (CMD_OVR_EVAL))
                || (**(nLAMBase + 1) == (CMD_OVR_EVAL1))
                || (**(nLAMBase + 1) == (CMD_OVR_XEQ))) {
            // THIS ENVIRONMENT BELONGS TO A SECONDARY
            if(*(nLAMBase + 1) == seco) {
                // FOUND AN ENVIRONMENT THAT BELONGS TO THE CURRENT SECONDARY, SO NO NEED FOR ONE
                return 0;
            }
            else
                return 1;       // THE CURRENT ENVIRONMENT BELONGS TO A DIFFERENT SECONDARY, CREATE A NEW ONE
        }
        else {
            // CURRENT ENVIRONMENT BELONGS TO SOMETHING NOT-A-SECO (LOOPS, ETC.)
            rsptr = RSTop - 1;

            if(rsptr < RStk)
                return 0;       // THERE'S NO RETURN ADDRESS, WE ARE EXECUTING FREE COMMANDS FROM THE COMMAND LINE
            if((**rsptr == (CMD_OVR_EVAL)) || (**rsptr == (CMD_OVR_EVAL1))
                    || (**rsptr == (CMD_OVR_XEQ))) {
                // WE ARE EXECUTING A SECONDARY THAT WAS 'EVAL'd, NEED A NEW ENVIRONMENT
                return 1;
            }
            if(ISPROLOG(**rsptr) && (LIBNUM(**rsptr) == DOIDENTEVAL)) {
                // WE ARE EXECUTING A SECONDARY THAT WAS DIRECTLY EXECUTED FROM AN IDENT, NEED A NEW ENVIRONMENT
                return 1;
            }

            while(rsptr >= RStk) {
                if(*rsptr == *(nLAMBase + 1))
                    return 0;   // CURRENT ENVIRONMENT IS INSIDE CURRENT SECO
                if(*rsptr == seco)
                    return 1;   // CURRENT SECO IS INSIDE THE CURRENT ENVIRONMENT
                --rsptr;
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
    WORDPTR *rsptr = ValidateTop - 1;
    WORDPTR seco = 0;

    while(rsptr >= ValidateBottom) {
        if(ISPROLOG(**rsptr) && ((LIBNUM(**rsptr) == SECO)
                    || (LIBNUM(**rsptr) == DOCOL))) {
            seco = *rsptr;
            break;
        }
        --rsptr;
    }

    // NOW DETERMINE IF THE CURRENT SECONDARY HAS AN ENVIRONMENT
    if(!seco) {
        // NOT IN A SECONDARY OR IN A SECONDARY INDIRECTLY EXECUTED WITH EVAL

        if(nLAMBase != LAMTop && nLAMBase >= LAMTopSaved) {
            // THERE IS AN EXISTING ENVIRONMENT
            return 0;
        }

        return 1;       // WE ARE NOT INSIDE ANY SECONDARY, SO AN ENVIRONMENT IS NEEDED
    }

    if(nLAMBase >= LAMTopSaved && nLAMBase < LAMTop) {

        if(ISPROLOG(**(nLAMBase + 1)) && ((LIBNUM(**(nLAMBase + 1)) == SECO)
                    || (LIBNUM(**(nLAMBase + 1)) == DOCOL))) {
            // THIS ENVIRONMENT BELONGS TO A SECONDARY
            if(*(nLAMBase + 1) == seco) {
                // FOUND AN ENVIRONMENT THAT BELONGS TO THE CURRENT SECONDARY, SO NO NEED FOR ONE
                return 0;
            }
            else
                return 1;       // THE CURRENT ENVIRONMENT BELONGS TO A DIFFERENT SECONDARY, CREATE A NEW ONE
        }
        else {
            // CURRENT ENVIRONMENT BELONGS TO SOMETHING NOT-A-SECO (LOOPS)

            // FIND OUT IF THE CURRENT SECONDARY IS INSIDE THE CURRENT ENVIRONMENT
            rsptr = ValidateTop - 1;
            while(rsptr >= ValidateBottom) {
                if(*rsptr == *(nLAMBase + 1)) {
                    // FOUND THE CURRENT ENVIRONMENT OWNER INSIDE THE CURRENT SECO, NO NEED FOR ENVIRONMENT
                    return 0;
                }
                if(*rsptr == seco)
                    return 1;   // CURRENT SECO IS INSIDE THE OWNER OF THE CURRENT ENVIRONMENT, CREATE A NEW ONE
                --rsptr;
            }

            // THIS SHOULD BE UNREACHABLE CODE SINCE seco IS GUARANTEED TO BE ON THE LIST
            // NO NEED FOR AN ENVIRONMENT
            return 0;
        }
    }

    // THIS SECONDARY DOESN'T HAVE ANY LOCALS
    return 1;

}

void rplClearLAMs()
{
    LAMTop = nLAMBase = LAMs;
}

void rplCompileIDENT(BINT libnum, BYTEPTR tok, BYTEPTR tokend)
{
    // CHECK IF THERE'S SUBSCRIPT ATTRIBUTES TO THIS IDENT
    WORD attr = 0;
    BYTEPTR lastchar = (BYTEPTR) utf8rskipst((char *)tokend, (char *)tok);

    BYTEPTR subs = (BYTEPTR) subscriptChars;

    while(subs - (BYTEPTR) subscriptChars < 30) {

        if(!utf8ncmp((char *)lastchar, (char *)tokend, (char *)subs,
                    subscriptChars + 30, 1)) {
            // HAS ATTRIBUTE
            attr <<= 4;
            attr |= ((subs - (BYTEPTR) subscriptChars) / 3);
            subs = (BYTEPTR) subscriptChars;
            if(lastchar == tok)
                break;
            lastchar = (BYTEPTR) utf8rskipst((char *)lastchar, (char *)tok);
            continue;
        }
        subs = (BYTEPTR) utf8skip((char *)subs, (char *)subscriptChars + 30);
    }

    tokend = (BYTEPTR) utf8skipst((char *)lastchar, (char *)tokend);

    // CHECK IF THERE'S MNEMONIC ATTRIBUTES

    if(*lastchar == ':') {
        --lastchar;
        while((lastchar > tok) && (*lastchar != ':'))
            --lastchar;

        if(*lastchar == ':') {

            attr = rplDecodeAttrib(lastchar + 1, tokend - 1);

        }

        tokend = lastchar;

    }

    // WE HAVE A VALID QUOTED IDENT, CREATE THE OBJECT
    BINT lenwords = (tokend - tok + 3) >> 2;
    BINT len = tokend - tok;
    ScratchPointer1 = (WORDPTR) tok;
    if(attr) {
        ++lenwords;
        libnum |= HASATTR_BIT;
    }
    rplCompileAppend(MKPROLOG(libnum, lenwords));
    WORD nextword;
    tok = (BYTEPTR) ScratchPointer1;
    while(len > 3) {
        // WARNING: THIS IS LITTLE ENDIAN ONLY!
        nextword = tok[0] + (tok[1] << 8) + (tok[2] << 16) + (tok[3] << 24);
        ScratchPointer1 = (WORDPTR) tok;
        rplCompileAppend(nextword);
        tok = (BYTEPTR) ScratchPointer1;
        tok += 4;
        len -= 4;
    }
    if(len) {
        nextword = 0;
        BINT rot = 0;
        while(len) {
            // WARNING: THIS IS LITTLE ENDIAN ONLY!
            nextword |= (*tok) << rot;
            --len;
            ++tok;
            rot += 8;
        }
        rplCompileAppend(nextword);
    }

    if(attr)
        rplCompileAppend(attr);
    // DONE

}

// ALLOCATES MEMORY AND CREATES AN IDENT OBJECT
// RETURNS NULL ON ERROR, DOESN'T CHECK IF IDENT IS VALID!
// USER MUST CALL rplIsValidIdent() BEFORE CALLING THIS FUNCTION

WORDPTR rplCreateIDENT(BINT libnum, BYTEPTR tok, BYTEPTR tokend)
{
    // CREATE THE OBJECT
    BINT lenwords = (tokend - tok + 3) >> 2;
    BINT len = tokend - tok;

    ScratchPointer1 = (WORDPTR) tok;

    WORDPTR newobj = rplAllocTempOb(lenwords), newptr;
    if(!newobj)
        return 0;
    newptr = newobj;
    *newptr = MKPROLOG(libnum, lenwords);
    ++newptr;
    WORD nextword;
    tok = (BYTEPTR) ScratchPointer1;
    while(len > 3) {
        // WARNING: THIS IS LITTLE ENDIAN ONLY!
        nextword = tok[0] + (tok[1] << 8) + (tok[2] << 16) + (tok[3] << 24);
        *newptr = nextword;
        ++newptr;
        tok += 4;
        len -= 4;
    }
    if(len) {
        nextword = 0;
        BINT rot = 0;
        while(len) {
            // WARNING: THIS IS LITTLE ENDIAN ONLY!
            nextword |= (*tok) << rot;
            --len;
            ++tok;
            rot += 8;
        }
        *newptr = nextword;
    }
    // DONE
    return newobj;
}

// DECODE AN ATTRIBUTE STRING: RETURN ATTRIBUTE OR -1 IF INVALID
// ATTRIBUTE STRINGS CAN BE:

// ONE LETTER FOR TYPE:
// R=REAL
// C=COMPLEX
// Z=INTEGER
// O=ODD INTEGER
// E=EVEN INTEGER
// M=MATRIX/VECTOR
// ?=UNKNOWN OBJECT TYPE (LIST OR OTHER)
// *=NO ASSUMPTIONS MADE

// ONE OPTIONAL MODIFIER LETTER ∞ IF THE VARIABLE MAY BE INFINITE

// 2 OPTIONAL CHARACTERS FOR SIGN:
// >0
// ≥0
// ≠0
// ≤0
// <0

BINT rplDecodeAttrib(BYTEPTR st, BYTEPTR end)
{
    BINT attr = 0;
    if(st >= end)
        return -1;
    switch (*st) {
    case 'R':
        attr = IDATTR_ISINFREAL;
        break;
    case 'C':
        attr = IDATTR_ISINFCPLX;
        break;
    case 'Z':
        attr = IDATTR_ISINFREAL | IDATTR_INTEGER;
        break;
    case 'O':
        attr = IDATTR_ISINFREAL | IDATTR_INTEGER | IDATTR_ODD;
        break;
    case 'E':
        attr = IDATTR_ISINFREAL | IDATTR_INTEGER | IDATTR_EVEN;
        break;
    case 'M':
        attr = IDATTR_ISMATRIX;
        break;
    case '?':
        attr = IDATTR_ISUNKNOWN;
        break;
    case '*':
        if(end - st != 1)
            return -1;
        break;
    default:
        return -1;
    }

    ++st;
    if(st >= end) {
        if(attr & (IDATTR_ISINFCPLX | IDATTR_ISINFREAL))
            attr |= IDATTR_ISNOTINF;
        return attr;
    }

    if(!utf8ncmp2((char *)st, (char *)end, "∞", 1)) {
        // SKIP THE INFINITY MODIFIER
        st = (BYTEPTR) utf8skipst((char *)st, (char *)end);
    }
    else if(attr & (IDATTR_ISINFCPLX | IDATTR_ISINFREAL))
        attr |= IDATTR_ISNOTINF;

    if(st >= end)
        return attr;

    // IF THERE'S ANYTHING ELSE, IT MUST END IN ZERO
    if(end[-1] != '0')
        return -1;

    if(!utf8ncmp2((char *)st, (char *)end, "≥", 1)) {
        attr |= IDATTR_GTEZERO;
        st = (BYTEPTR) utf8skipst((char *)st, (char *)end);
    }
    else if(!utf8ncmp2((char *)st, (char *)end, "≤", 1)) {
        attr |= IDATTR_LTEZERO;
        st = (BYTEPTR) utf8skipst((char *)st, (char *)end);
    }
    else if(!utf8ncmp2((char *)st, (char *)end, "≠", 1)) {
        attr |= IDATTR_NOTZERO;
        st = (BYTEPTR) utf8skipst((char *)st, (char *)end);
    }
    else if(*st == '>') {
        attr |= IDATTR_GTEZERO | IDATTR_NOTZERO;
        ++st;
    }
    else if(*st == '<') {
        attr |= IDATTR_LTEZERO | IDATTR_NOTZERO;
        ++st;
    }

    if(end - st != 1)
        return -1;

    return attr;

}

BINT rplIsValidIdent(BYTEPTR tok, BYTEPTR tokend)
{
    BYTEPTR ptr;
    BINT char1, char2;
    BINT argsep;
    BYTEPTR attribend, attribst;

    if(tokend <= tok)
        return 0;

    attribst = attribend = tokend;

    argsep = ARG_SEP(rplGetSystemLocale());

    // SKIP ANY INITIAL DOTS
    while((tok != tokend) && (*tok == '.'))
        ++tok;

    if(tok == tokend)
        return 0;       // CAN'T BE ONLY DOTS

    // IDENT CANNOT START WITH A NUMBER
    if((((char)*tok) >= '0') && (((char)*tok) <= '9'))
        return 0;

    // IDENT MAY HAVE VALID ATTRIBUTES/HINTS
    if(((char)tokend[-1]) == ':') {
        --tokend;
        attribend = tokend;
        while((tokend > tok) && (tokend[-1] != ':'))
            --tokend;

        if(tokend - tok < 2)
            return 0;   // SINGLE COLON IS NOT A PROPER ATTRIBUTE
        attribst = tokend;
        --tokend;
        if(rplDecodeAttrib(attribst, attribend) < 0)
            return 0;   // INVALID ATTRIBUTES?
    }

    // OR CONTAIN ANY OF THE FORBIDDEN CHARACTERS
    while(tok != tokend) {
        ptr = (BYTEPTR) forbiddenChars;
        char1 = utf82cp((char *)tok, (char *)tokend);
        do {
            char2 = utf82cp((char *)ptr, (char *)ptr + 4);
            if(char1 == char2)
                return 0;
            ptr = (BYTEPTR) utf8skip((char *)ptr, (char *)ptr + 4);
        }
        while(*ptr);

        if(char1 == argsep)
            return 0;   // DON'T ALLOW THE ARGUMENT SEPARATOR

        tok = (BYTEPTR) utf8skip((char *)tok, (char *)tokend);
    }
    return 1;
}

// DETERMINE THE LENGTH OF AN IDENT STRING IN BYTES (NOT CHARACTERS)
BINT rplGetIdentLength(WORDPTR ident)
{
    BINT len = OBJSIZE(*ident);
    if(LIBNUM(*ident) & HASATTR_BIT)
        --len;
    if(!len)
        return 0;

    WORD lastword = *(ident + len);
    BINT usedbytes = 0;
    while(!(lastword & 0xff000000) && (usedbytes < 4)) {
        lastword <<= 8;
        ++usedbytes;
    }
    usedbytes = 4 - usedbytes;

    return ((len - 1) << 2) + usedbytes;
}

// MAKE A NEW COPY OF THE CURRENT ENVIRONMENT
void rplDupLAMEnv()
{
    WORDPTR *ptr = nLAMBase, *endptr = LAMTop;
    while(ptr < endptr) {
        rplCreateLAM(ptr[0], ptr[1]);
        if(Exceptions)
            return;
        ptr += 2;
    }
    nLAMBase = ptr;
}
