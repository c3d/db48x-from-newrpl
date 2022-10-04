/*
 * Copyright (c) 2014-2017, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "cmdcodes.h"
#include "libraries.h"
#include "newrpl.h"

// THIS MODULE ORGANIZES THE NUMERIC SOLVERS

// EVALUATE POLYNOMIAL EXPLODED IN THE STACK, USING HORNER'S METHOD
// first POINTS TO THE HIGHEST DEGREE COEFFICIENT EXPLODED IN THE STACK
// value POINTS TO THE VALUE, IN ANY LOCATION IN THE STACK
// COEFFICIENTS CAN BE REAL, COMPLEX OR SYMBOLIC

// LOW-LEVEL - NO CHECKS DONE HERE

word_p rplPolyEvalEx(word_p * first, int32_t degree, word_p * value)
{
    int32_t k;

    rplPushData(first[0]);
    for(k = 1; k <= degree; ++k) {
        rplPushData(*value);
        rplCallOvrOperator(CMD_OVR_MUL);
        if(Exceptions)
            return 0;
        rplPushData(first[k]);
        rplCallOvrOperator(CMD_OVR_ADD);
        if(Exceptions)
            return 0;
        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions)
                return 0;
        }
    }
    return rplPopData();

}

// EVALUATE POLYNOMIAL DERIVATIVE FROM COEFFICIENTS EXPLODED IN THE STACK, USING HORNER'S METHOD
// deriv=1 FOR FIRST DERIVATIVE, 2 FOR SECOND, ETC.
// first POINTS TO THE HIGHEST DEGREE COEFFICIENT EXPLODED IN THE STACK
// value POINTS TO THE VALUE, IN ANY LOCATION IN THE STACK
// COEFFICIENTS CAN BE REAL, COMPLEX OR SYMBOLIC

// LOW-LEVEL - NO CHECKS DONE HERE

word_p rplPolyEvalDerivEx(int32_t deriv, word_p * first, int32_t degree,
        word_p * value)
{
    int32_t k, j, c;

    rplPushData(first[0]);
    for(c = degree, j = 1; j < deriv; ++j)
        c *= (c - j);
    rplNewBINTPush(c, DECBINT);
    if(Exceptions)
        return 0;
    rplCallOvrOperator(CMD_OVR_MUL);
    if(Exceptions)
        return 0;

    for(k = 1; k <= degree - deriv; ++k) {
        rplPushData(*value);
        rplCallOvrOperator(CMD_OVR_MUL);
        if(Exceptions)
            return 0;
        rplPushData(first[k]);
        for(c = degree - k, j = 1; j < deriv; ++j)
            c *= (c - j);
        rplNewBINTPush(c, DECBINT);
        if(Exceptions)
            return 0;
        rplCallOvrOperator(CMD_OVR_MUL);
        if(Exceptions)
            return 0;
        rplCallOvrOperator(CMD_OVR_ADD);
        if(Exceptions)
            return 0;
        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions)
                return 0;
        }
    }
    return rplPopData();

}

// FIND ONE ROOT OF NUMERIC POLYNOMIAL
// MAKE SURE ALL COEFFICIENTS ARE NUMERIC (REAL OR COMPLEX) - NO CHECKS DONE HERE
// USES LAGUERRE METHOD, IT FINDS COMPLEX ROOTS
// CALLER NEEDS TO SET FLAGS: COMPLEX MODE AND DON'T ERROR ON INFINITE RESULT TO AVOID PREMATURE EXIT

word_p rplPolyRootEx(word_p * first, int32_t degree)
{
    int32_t k;

    // EXTRACT THE MAGNITUDE OF ALL COEFFICIENTS
    for(k = 0; k <= degree; ++k) {
        rplPushData(first[k]);
        rplCallOvrOperator(CMD_OVR_ABS);
        if(Exceptions)
            return 0;
    }
    // UPPER BOUND= 1+1/|an|*max(a0...a(n-1)) [THIS IS THE CAUCHY BOUND]

    word_p *firstabs = DSTop - (degree + 1);

    // GET THE MAXIMUM VALUE
    for(k = 1; k <= degree; ++k) {
        rplPushData(firstabs[k]);
        if(k > 1) {
            rplCallOperator(CMD_MAX);
            if(Exceptions)
                return 0;
        }
    }
    rplPushData(firstabs[0]);   // an COEFFICIENT
    rplCallOvrOperator(CMD_OVR_DIV);
    if(Exceptions)
        return 0;
    rplPushData((word_p) one_bint);
    rplCallOvrOperator(CMD_OVR_ADD);
    if(Exceptions)
        return 0;

    // USE HALF OF THAT CIRCLE AS INITIAL GUESS
    newRealFromint32_t(&RReg[0], 5, -1);
    rplNewRealFromRRegPush(0);
    if(Exceptions)
        return 0;
    rplCallOvrOperator(CMD_OVR_MUL);
    firstabs[0] = (word_p) one_bint;
    firstabs[1] = rplPeekData(1);
    DSTop = firstabs + 2;       //    DROP ALL OTHER INTERMEDIATE VALUES

    // HERE WE HAVE THE POLYNOMIAL EXPLODED AND AN INITIAL GUESS

    int32_t oldprec = Context.precdigits;
    word_p pk;
    REAL err;
    // TEMPORARILY INCREASE PRECISION
    if(Context.precdigits <= REAL_PRECISION_MAX - 8)
        Context.precdigits += 8;

    do {

        rplPushData(firstabs[0]);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplCallOvrOperator(CMD_OVR_ABS);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplReadNumberAsReal(rplPopData(), &err);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        if(intdigitsReal(&err) < -(oldprec / 2 + 8))
            break;      // FOUND A ROOT!

        pk = rplPolyEvalEx(first, degree, firstabs + 1);
        if(!pk) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplPushData(pk);
        // ALSO CHECK FOR ZERO pk() SINCE IT CAN BLOW THE ALGORITHM
        rplPushData(rplPeekData(1));
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplCallOvrOperator(CMD_OVR_ABS);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplReadNumberAsReal(rplPopData(), &err);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        if(iszeroReal(&err) || (intdigitsReal(&err) < -(2 * oldprec))) {
            rplDropData(1);     // FOUND A ROOT!
            break;
        }

        //   HERE WE HAVE pk ON THE STACK, COMPUTE p'k and p''k

        pk = rplPolyEvalDerivEx(1, first, degree, firstabs + 1);
        if(!pk) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplPushData(pk);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplPushData(rplPeekData(2));
        rplCallOvrOperator(CMD_OVR_DIV);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }

        // HERE G=p'k/pk IS IN LEVEL ONE

        rplPushData(rplPeekData(1));
        rplPushData(rplPeekData(1));
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplCallOvrOperator(CMD_OVR_MUL);        // G^2
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplPushData(rplPeekData(1));
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }

        pk = rplPolyEvalDerivEx(2, first, degree, firstabs + 1);
        if(!pk) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplPushData(pk);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplPushData(rplPeekData(5));    // HERE WE HAVE pk,G,G^2,G^2, p''k AND NOW pk
        rplCallOvrOperator(CMD_OVR_DIV);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplCallOvrOperator(CMD_OVR_SUB);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }

        // HERE WE HAVE pk, G, G^2 AND H ON THE STACK

        rplNewBINTPush(degree, DECBINT);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplOverwriteData(5, rplPeekData(1));    // SAVE n FOR LATER, OVERWRITE pk
        rplCallOvrOperator(CMD_OVR_MUL);        // n*H
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplCallOvrOperator(CMD_OVR_SUB);        // G^2-n*H
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }

        rplNewBINTPush(1 - degree, DECBINT);    // -(n-1) = (1-n)
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }

        rplCallOvrOperator(CMD_OVR_MUL);        // (n-1)*(n*H-G^2)
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }

        rplCallOperator(CMD_SQRT);
        if(Exceptions) {
            Context.precdigits = oldprec;       // THIS COULD TRIGGER "ARGUMENT OUT OF BOUNDS" IF COMPLEX MODE IS NOT ENABLED
            return 0;
        }

        // HERE WE HAVE n, G, SQRT( (n-1)*(n*H-G^2) ) ON THE STACK

        rplPushData(rplPeekData(2));
        rplPushData(rplPeekData(2));
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplCallOvrOperator(CMD_OVR_ADD);        // G+SQRT(...)
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplPushData(rplPeekData(3));
        rplPushData(rplPeekData(3));
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplCallOvrOperator(CMD_OVR_SUB);        // G-SQRT(...)
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }

        rplPushData(rplPeekData(2));
        rplCallOvrOperator(CMD_OVR_ABS);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplPushData(rplPeekData(2));
        rplCallOvrOperator(CMD_OVR_ABS);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        rplCallOvrOperator(CMD_OVR_GT);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        // PICK THE MAX(ABS(G+SQRT(...)) , ABS(G-SQRT(...))
        // HERE WE HAVE n, G, SQRT( (n-1)*(n*H-G^2) )
        // G+SQRT(...) , G-SQRT(...), ON THE STACK
        rplOverwriteData(4, rplPeekData(rplIsFalse(rplPopData())? 1 : 2));
        rplDropData(3);

        // LEAVE n, G(+/-)SQRT(...)
        if(rplIsFalse(rplPeekData(1))) {
            // CAN'T DIVIDE BY ZERO, THIS CAN ONLY HAPPEN IF G==0 AND H==0
            // THIS MAY HAPPEN AT A LOCAL EXTREMUM
            // NOT SURE WHAT TO DO, FOR NOW JUST SET a=1000*error TO MOVE IT TO A DIFFERENT POINT
            rplDropData(2);
            newRealFromint32_t(&RReg[0], 1, -Context.precdigits + 12);
            rplNewRealFromRRegPush(0);
        }
        else
            rplCallOvrOperator(CMD_OVR_DIV);

        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }
        // FINALLY, WE HAVE x AND a ON THE STACK
        firstabs[0] = rplPeekData(1);
        rplCallOvrOperator(CMD_OVR_SUB);
        if(Exceptions) {
            Context.precdigits = oldprec;
            return 0;
        }

    }
    while(1);   // ADD SOME LOOP LIMIT HERE IN CASE IT DOESN'T CONVERGE!

    // FOUND A ROOT, CLEAN IT UP BEFORE SENDING

    pk = rplPeekData(1);

    if(ISCOMPLEX(*pk)) {
        int32_t cclass = rplComplexClass(pk);
        if(cclass == CPLX_ZERO)
            rplOverwriteData(1, (word_p) zero_bint);
        else if(cclass == CPLX_NORMAL) {
            REAL re, im;

            rplReadCNumberAsReal(pk, &re);
            rplReadCNumberAsImag(pk, &im);

            int32_t digre, digim;
            digre = intdigitsReal(&re);
            digim = intdigitsReal(&im);

            if((digim < -10) && (digre > digim + 10)) {
                // IF IM(r)<1E-10 AND RE(r)>10^10*IM(r)
                // TRY A REAL ROOT AND SEE IF IT'S STILL GOOD

                rplNewRealPush(&re);
                if(Exceptions) {
                    Context.precdigits = oldprec;
                    return 0;
                }
                pk = rplPolyEvalEx(first, degree, DSTop - 1);
                if(!pk) {
                    Context.precdigits = oldprec;
                    return 0;
                }
                rplPushData(pk);
                if(Exceptions) {
                    Context.precdigits = oldprec;
                    return 0;
                }
                rplCallOvrOperator(CMD_OVR_ABS);
                if(Exceptions) {
                    Context.precdigits = oldprec;
                    return 0;
                }
                rplReadNumberAsReal(rplPopData(), &err);
                if(Exceptions) {
                    Context.precdigits = oldprec;
                    return 0;
                }
                if(iszeroReal(&err) || (intdigitsReal(&err) < -(2 * oldprec))) {
                    pk = rplPopData();  // REAL ROOT ALONE IS STILL GOOD, USE IT
                    rplOverwriteData(1, pk);
                }
                else {
                    rplPopData();
                    pk = rplPeekData(1);
                }
            }
            else if((digre < -10) && (digim > digre + 10)) {
                // IF RE(r)<1E-10 AND IM()>10^10*RE()
                // TRY A PURE IMAGINARY ROOT AND SEE IF IT'S STILL GOOD

                rplZeroToRReg(0);
                rplNewComplexPush(&RReg[0], &im, ANGLENONE);
                if(Exceptions) {
                    Context.precdigits = oldprec;
                    return 0;
                }
                pk = rplPolyEvalEx(first, degree, DSTop - 1);
                if(!pk) {
                    Context.precdigits = oldprec;
                    return 0;
                }
                rplPushData(pk);
                if(Exceptions) {
                    Context.precdigits = oldprec;
                    return 0;
                }
                rplCallOvrOperator(CMD_OVR_ABS);
                if(Exceptions) {
                    Context.precdigits = oldprec;
                    return 0;
                }
                rplReadNumberAsReal(rplPopData(), &err);
                if(Exceptions) {
                    Context.precdigits = oldprec;
                    return 0;
                }
                if(iszeroReal(&err) || (intdigitsReal(&err) < -(2 * oldprec))) {
                    pk = rplPopData();  // IMAG. ROOT ALONE IS STILL GOOD, USE IT
                    rplOverwriteData(1, pk);
                }
                else {
                    rplPopData();
                    pk = rplPeekData(1);
                }
            }

        }

    }

    if(ISNUMBER(*pk)) {
        // ADJUST ROOTS TO BE INTEGER IF THEY ARE CLOSE TO IT

        REAL re;

        rplReadNumberAsReal(pk, &re);

        fracReal(&RReg[0], &re);

        if(!iszeroReal(&RReg[0]) && (intdigitsReal(&RReg[0]) < -10)) {

            ipReal(&RReg[1], &re, 0);

            rplNewRealPush(&RReg[1]);
            if(Exceptions) {
                Context.precdigits = oldprec;
                return 0;
            }
            pk = rplPolyEvalEx(first, degree, DSTop - 1);
            if(!pk) {
                Context.precdigits = oldprec;
                return 0;
            }
            rplPushData(pk);
            if(Exceptions) {
                Context.precdigits = oldprec;
                return 0;
            }
            rplCallOvrOperator(CMD_OVR_ABS);
            if(Exceptions) {
                Context.precdigits = oldprec;
                return 0;
            }
            rplReadNumberAsReal(rplPopData(), &err);
            if(Exceptions) {
                Context.precdigits = oldprec;
                return 0;
            }
            if(iszeroReal(&err) || (intdigitsReal(&err) < -(2 * oldprec))) {
                pk = rplPopData();      // INTEGER ROOT ALONE IS STILL GOOD, USE IT
                rplOverwriteData(1, pk);
            }
            else {
                rplPopData();
                pk = rplPeekData(1);
            }
        }

    }

    Context.precdigits = oldprec;
    rplDropData(2);
    return pk;
}

// DEFLATE POLYNOMIAL EXPLODED IN THE STACK, USING HORNER'S METHOD
// first POINTS TO THE HIGHEST DEGREE COEFFICIENT EXPLODED IN THE STACK
// value POINTS TO THE ROOT VALUE, IN ANY LOCATION IN THE STACK
// COEFFICIENTS CAN BE REAL, COMPLEX OR SYMBOLIC
// MODIFIES THE ORIGINAL COEFFICIENTS ON THE STACK
// THE LAST COEFFICIENT IS THE REMAINDER, WHICH IS ALSO RETURNED

// LOW-LEVEL - NO CHECKS DONE HERE

word_p rplPolyDeflateEx(word_p * first, int32_t degree, word_p * value)
{
    int32_t k;

    rplPushData(first[0]);
    for(k = 1; k <= degree; ++k) {
        rplPushData(*value);
        rplCallOvrOperator(CMD_OVR_MUL);
        if(Exceptions)
            return 0;
        rplPushData(first[k]);
        rplCallOvrOperator(CMD_OVR_ADD);
        if(Exceptions)
            return 0;
        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions)
                return 0;
        }
        first[k] = rplPeekData(1);
    }
    return rplPopData();

}

// EVALUATES A USER-DEFINED FUNCTION
// IT CAN BE A PROGRAM OR AN ALGEBRAIC IN THE FORM 'f(X,Y,Z)=EXPRESSION'
// IT WILL TAKE n ARGUMENTS FROM THE STACK, PUT IT IN LOCAL VARIABLES (X,Y,Z), THEN EVALUATE THE EXPRESSION WITH THE GIVEN OPCODE (->NUM OR EVAL)
// IF IT'S A PROGRAM, IT WILL XEQ THE PROGRAM, THEN RUN THE OPCODE ON THE RESULT

void rplEvalUserFunc(word_p arg_userfunc, WORD Opcode)
{
    word_p *dstksave = DSTop;
    if(ISSYMBOLIC(*arg_userfunc))
        arg_userfunc = rplSymbUnwrap(arg_userfunc);
    if(ISSYMBOLIC(*arg_userfunc) && (OBJSIZE(*arg_userfunc) > 3)
            && (arg_userfunc[1] == CMD_EQUATIONOPERATOR)
            && (ISSYMBOLIC(arg_userfunc[2]))
            && (arg_userfunc[3] == CMD_OVR_FUNCEVAL)) {
        // CREATE LAMS FOR ALL FUNCTION ARGUMENTS
        ScratchPointer1 = arg_userfunc + 4;     // FIRST ARGUMENT
        ScratchPointer2 = rplSkipOb(arg_userfunc + 2);  // END OF SYMBOLIC FUNCTION CALL
        rplCreateLAMEnvironment(arg_userfunc);
        if(Exceptions)
            return;
        int32_t nargs = 0;
        while(rplSkipOb(ScratchPointer1) < ScratchPointer2) {
            ++nargs;
            if(!ISIDENT(*ScratchPointer1)) {
                rplError(ERR_INVALIDUSERDEFINEDFUNCTION);
                DSTop = dstksave;
                rplCleanupLAMs(0);
                return;
            }
            rplPushData(ScratchPointer1);
            if(Exceptions) {
                DSTop = dstksave;
                rplCleanupLAMs(0);
                return;
            }
            ScratchPointer1 = rplSkipOb(ScratchPointer1);
        }
        if(rplDepthData() < 2 * nargs) {
            rplError(ERR_BADARGCOUNT);
            DSTop = dstksave;
            rplCleanupLAMs(0);
            return;
        }
        int32_t k;

        for(k = 0; k < nargs; ++k) {
            rplCreateLAM(DSTop[-k - 1], DSTop[-k - nargs - 1]);
            if(Exceptions) {
                DSTop = dstksave;
                rplCleanupLAMs(0);
                return;
            }
        }

        rplDropData(nargs);     // REMOVE ALL ARGUMENT NAMES

        rplPushData(ScratchPointer2);   // PUSH THE SYMBOLIC FUNCTION DEFINITION

        rplRunAtomic(Opcode);
        rplCleanupLAMs(0);

        if(Exceptions) {
            DSTop = dstksave;
            return;
        }
        if(DSTop - dstksave != 1) {
            rplError(ERR_INVALIDUSERDEFINEDFUNCTION);
            DSTop = dstksave;
            return;
        }
        // DONE, NOW REMOVE THE ARGUMENTS
        dstksave[-nargs] = rplPopData();
        rplDropData(nargs - 1);

    }
    else {
        // IT'S A PROGRAM
        rplPushData(arg_userfunc);
        rplRunAtomic(CMD_OVR_XEQ);
        if(Exceptions) {
            if(DSTop > dstksave)
                DSTop = dstksave;
            return;
        }
        rplRunAtomic(Opcode);
    }
}

// EVALUATES A USER-DEFINED LIST OF EQUATIONS
// IT CAN BE A PROGRAM OR AN ALGEBRAIC IN THE FORMS:
// 'EXPRESSION' (JUST AN EXPRESSION --> WILL BE ASSUMED AS 'EXPRESSION=0'
// 'EXPRESSION=EXPRESSION'
// 'EXPRESSION<=EXPRESSION' (ANY INEQUALITY)
// IT WILL TAKE nvars ARGUMENTS FROM THE STACK, PUT IT IN LOCAL VARIABLES FROM THE LIST OF VARIABLES (X,Y,Z),
// THEN EVALUATE THE EXPRESSION WITH THE OPCODE ->NUM
// IF IT'S A PROGRAM, IT WILL XEQ THE PROGRAM, THE PROGRAM IS EXPECTED TO RETURN A FUNCTION THAT CAN BE MINIMIZED
// IF IT'S A LIST OF EQUATIONS, IT RETURNS THE SUM OF:
// FOR ALL EXPRESSIONS OR EQUALITIES: '(LEFT-RIGHT)^2'
// FOR ALL TRUTH EXPRESSIONS (INEQUALITIES): +Inf IF FALSE, 0 IF TRUE

void rplEvalMultiUserFunc(word_p * listofeq, word_p * listofvars, int32_t nvars,
        int32_t minimizer)
{
    word_p *dstksave = DSTop;

    if(ISPROGRAM(**listofeq)) {
        // IT'S A PROGRAM
        rplPushData(*listofeq);
        rplRunAtomic(CMD_OVR_XEQ);
        if(Exceptions) {
            if(DSTop > dstksave)
                DSTop = dstksave;
            return;
        }
        if(DSTop != dstksave - nvars + 1) {
            rplError(ERR_INVALIDUSERDEFINEDFUNCTION);
            if(DSTop > dstksave)
                DSTop = dstksave;
            return;
        }
        rplRunAtomic(CMD_OVR_NUM);
        return;
    }

    int32_t woffset = 0, endoffset, j;

    endoffset = rplSkipOb(*listofeq) - *listofeq;
    if(ISLIST(**listofeq)) {
        ++woffset;
        --endoffset;
    }

    rplCreateLAMEnvironment(*listofeq);

    if(Exceptions)
        return;

    int32_t varname = 0, varends = rplSkipOb(*listofvars) - (*listofvars);

    if(ISLIST(**listofvars))
        varname++;

    for(j = 0; j < nvars; ++j) {
        if(ISIDENT(*(*listofvars + varname)))
            rplCreateLAM(*listofvars + varname, rplPeekData(nvars - j));
        if(Exceptions) {
            rplCleanupLAMs(*listofeq);
            DSTop = dstksave;
            return;
        }
        varname += rplObjSize(*listofvars + varname);
        if(varname > varends) {
            // USER DID NOT PROVIDE ENOUGH NAMES
            break;
        }
    }

    // HERE ALL VARIABLES HAVE BEEN DEFINED

    // EVALUATE ALL FUNCTIONS
    WORD eqtype;
    word_p *tmp;
    int32_t nresults = 0;
    REAL result;
    while(woffset < endoffset) {

        rplPushData(*listofeq + woffset);       // PUSH THE SYMBOLIC FUNCTION DEFINITION
        if(Exceptions) {
            rplCleanupLAMs(*listofeq);
            DSTop = dstksave;
            return;
        }

        tmp = DSTop;
        rplRunAtomic(CMD_OVR_NUM);

        if(Exceptions) {
            rplCleanupLAMs(*listofeq);
            DSTop = dstksave;
            return;
        }

        if(DSTop != tmp) {
            rplError(ERR_INVALIDUSERDEFINEDFUNCTION);
            rplCleanupLAMs(*listofeq);
            DSTop = dstksave;
            return;
        }

        if(!ISNUMBERORUNIT(*rplPeekData(1))) {
            rplError(ERR_REALVALUEDFUNCTIONSONLY);
            rplCleanupLAMs(*listofeq);
            DSTop = dstksave;
            return;
        }

        if(minimizer) {
            eqtype = rplSymbMainOperator(*listofeq + woffset);

            if(ISUNIT(*rplPeekData(1)))
                rplReadNumberAsReal(rplPeekData(1) + 1, &result);
            else
                rplReadNumberAsReal(rplPeekData(1), &result);

            switch (eqtype) {
            case CMD_OVR_EQ:
            case CMD_OVR_NOTEQ:
            case CMD_OVR_GT:
            case CMD_OVR_GTE:
            case CMD_OVR_LT:
            case CMD_OVR_LTE:
            case CMD_OVR_ISTRUE:
            case CMD_OVR_NOT:
            case CMD_OVR_OR:
            case CMD_OVR_AND:
            case CMD_OVR_XOR:
            {
                // EVALUATE AS +Inf WHEN FALSE, 0 WHEN TRUE
                if(iszeroReal(&result))
                    rplInfinityToRReg(0);
                else
                    rplZeroToRReg(0);
                break;
            }
            default:
            {
                // ALL OTHER EXPRESSIONS EVALUATE AS (EXPRESSION)^2
                mulReal(&RReg[0], &result, &result);
                break;
            }
            }

            word_p newreal = rplNewRealFromRReg(0);
            if(!newreal) {
                rplCleanupLAMs(*listofeq);
                DSTop = dstksave;
                return;
            }
            rplOverwriteData(1, newreal);
        }

        ++nresults;

        woffset += rplObjSize(*listofeq + woffset);
    }

    // WE ARE DONE EVALUATING FUNCTIONS, CLEAN UP THE LAM ENVIRONMENT
    rplCleanupLAMs(*listofeq);

    // WE HAVE nresults IN THE STACK, EITHER CREATE A LIST OR ADD THEM ALL TOGETHER
    if(minimizer) {
        rplZeroToRReg(0);
        for(j = 1; j <= nresults; ++j) {
            rplReadNumberAsReal(rplPeekData(j), &result);
            if(isNANorinfiniteReal(&result))
                rplInfinityToRReg(0);
            else {
                if(!isinfiniteReal(&RReg[0])) {
                    swapReal(&RReg[1], &RReg[0]);
                    addReal(&RReg[0], &RReg[1], &result);
                }
            }
        }
        rplDropData(nresults - 1);

        word_p newreal = rplNewRealFromRReg(0);
        if(!newreal) {
            DSTop = dstksave;
            return;
        }
        rplOverwriteData(1, newreal);
    }
    else {
        // PROVIDE A LIST OF RESULTS
        word_p newlist = rplCreateListN(nresults, 1, 0);
        if(!newlist) {
            DSTop = dstksave;
            return;
        }
        rplDropData(nresults - 1);
        rplOverwriteData(1, newlist);
    }

    // FINALLY, WE HAVE THE ORIGINAL STACK AND THE RESULT

}
