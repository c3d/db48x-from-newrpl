/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "libraries.h"

#ifndef COMMANDS_ONLY_PASS
#include "cmdcodes.h"
#include "hal_api.h"
#include "newrpl.h"
#include "sysvars.h"
#endif

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************

// REPLACE THE NUMBER
#define LIBRARY_NUMBER  66

//@TITLE=Transcendental functions

#define ERROR_LIST \
    ERR(ARGOUTSIDEDOMAIN,0)

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(SIN,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(COS,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(TAN,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(ASIN,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(ACOS,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(ATAN,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(ATAN2,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(LN,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(EXP,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(SINH,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(COSH,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(TANH,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(ASINH,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(ACOSH,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(ATANH,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(LOG,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(ALOG,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    ECMD(SQRT,"√",MKTOKENINFO(1,TITYPE_PREFIXOP,1,3)), \
    CMD(EXPM,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(LNP1,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    ECMD(PINUM,"π0",MKTOKENINFO(2,TITYPE_NOTALLOWED,0,2))

// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"

#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);

INCLUDE_ROMOBJECT(lib66_main);
INCLUDE_ROMOBJECT(lib66_trig);
INCLUDE_ROMOBJECT(lib66_hyp);
INCLUDE_ROMOBJECT(lib66_log);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[] = {
    (WORDPTR) LIB_MSGTABLE,
    (WORDPTR) LIB_HELPTABLE,

    (WORDPTR) lib66_main,
    (WORDPTR) lib66_trig,
    (WORDPTR) lib66_hyp,
    (WORDPTR) lib66_log
};

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    // LIBRARIES THAT DEFINE ONLY COMMANDS STILL HAVE TO RESPOND TO A FEW OVERLOADABLE OPERATORS
    if(LIBNUM(CurOpcode) == LIB_OVERLOADABLE) {
        // ONLY RESPOND TO EVAL, EVAL1 AND XEQ FOR THE COMMANDS DEFINED HERE
        // IN CASE OF COMMANDS TREATED AS OBJECTS (WHEN EMBEDDED IN LISTS)
        if((OPCODE(CurOpcode) == OVR_EVAL) ||
                (OPCODE(CurOpcode) == OVR_EVAL1) ||
                (OPCODE(CurOpcode) == OVR_XEQ)) {
            // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
            if(rplDepthData() < 1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORD saveOpcode = CurOpcode;
            CurOpcode = *rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode = saveOpcode;
            return;
        }

        if((OPCODE(CurOpcode) == OVR_NUM)) {
            // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
            if(rplDepthData() < 1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORD saveOpcode = CurOpcode;
            CurOpcode = *rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode = saveOpcode;
            return;
        }

        // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
        if(OPCODE(CurOpcode) == OVR_SAME) {
            if(*rplPeekData(2) == *rplPeekData(1)) {
                rplDropData(2);
                rplPushTrue();
            }
            else {
                rplDropData(2);
                rplPushFalse();
            }
            return;
        }
        else {
            rplError(ERR_INVALIDOPCODE);
            return;

        }

    }

    switch (OPCODE(CurOpcode)) {
    case PINUM:
    {
        //@SHORT_DESC=Numeric constant π with twice the current system precision
        //@NEW
        REAL pi;

        decconst_PI(&pi);

        rplNewRealPush(&pi);
        return;

    }
    case SIN:
    {
        //@SHORT_DESC=Compute the sine
        REAL dec;
        BINT angmode;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_INF | CPLX_POLAR:
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            rplClrSystemFlag(FL_FORCED_RAD);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            rplClrSystemFlag(FL_FORCED_RAD);
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        // SUPPORT FOR COMPLEX ARGUMENTS
        if(ISCOMPLEX(*arg)) {
            rplClrSystemFlag(FL_FORCED_RAD);
            REAL re, im;
            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // WE GOT A POLAR COMPLEX NUMBER
                // CONVERT TO POLAR COORDINATES
                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

            }
            else {
                copyReal(&RReg[8], &re);
                copyReal(&RReg[9], &im);
            }

            // COMPUTE AS SIN(X+I*Y)=SIN(X)*COSH(Y)+COS(X)*SINH(Y)*i

            trig_sincos(&RReg[8], ANGLERAD);

            normalize(&RReg[6]);
            normalize(&RReg[7]);

            swapReal(&RReg[8], &RReg[6]);
            swapReal(&RReg[9], &RReg[7]);

            // NOW THE Y COMPONENT IS IN RReg[7]

            hyp_sinhcosh(&RReg[7]);

            // RReg[0]=cosh
            // RReg[1]=sinh

            normalize(&RReg[0]);
            normalize(&RReg[1]);

            mulReal(&RReg[3], &RReg[9], &RReg[0]);
            mulReal(&RReg[4], &RReg[8], &RReg[1]);

            // RETURN THE COMPLEX NUMBER

            if(angmode != ANGLENONE) {
                // NEED TO RETURN A POLAR NUMBER

                rplRect2Polar(&RReg[3], &RReg[4], angmode);

                WORDPTR newcmplx = rplNewComplex(&RReg[0], &RReg[1], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[1]);

                return;

            }

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[3], &RReg[4], angmode);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[3]);
            rplCheckResultAndError(&RReg[4]);

            return;

        }

        angmode = ANGLEMODE(*arg);

        if(angmode == ANGLENONE) {
            if(rplTestSystemFlag(FL_FORCED_RAD))
                angmode = ANGLERAD;
            else
                angmode =
                        rplTestSystemFlag(FL_ANGLEMODE1) |
                        (rplTestSystemFlag(FL_ANGLEMODE2) << 1);
        }
        rplClrSystemFlag(FL_FORCED_RAD);

        // REAL  ARGUMENTS
        if(ISANGLE(*arg))
            rplReadNumberAsReal(arg + 1, &dec);
        else
            rplReadNumberAsReal(arg, &dec);

        if(Exceptions)
            return;

        trig_sin(&dec, angmode);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);      // SIN
        return;

    }
    case COS:
    {
        //@SHORT_DESC=Compute the cosine
        REAL dec;
        BINT angmode;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            rplListUnaryDoCmd();

            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_INF | CPLX_POLAR:
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            rplClrSystemFlag(FL_FORCED_RAD);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);

            }
            rplClrSystemFlag(FL_FORCED_RAD);
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        // SUPPORT FOR COMPLEX ARGUMENTS
        if(ISCOMPLEX(*arg)) {
            rplClrSystemFlag(FL_FORCED_RAD);
            REAL re, im;
            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // WE GOT A POLAR COMPLEX NUMBER
                // CONVERT TO POLAR COORDINATES
                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

            }
            else {
                copyReal(&RReg[8], &re);
                copyReal(&RReg[9], &im);
            }

            // COMPUTE AS SIN(X+I*Y)=COS(X)*COSH(Y)-SIN(X)*SINH(Y)*i

            trig_sincos(&RReg[8], ANGLERAD);

            normalize(&RReg[6]);
            normalize(&RReg[7]);

            swapReal(&RReg[9], &RReg[6]);       // RReg[9]=cos
            swapReal(&RReg[8], &RReg[7]);       // RReg[8]=sin

            // NOW THE Y COMPONENT IS IN RReg[6]

            hyp_sinhcosh(&RReg[6]);

            // RReg[0]=cosh
            // RReg[1]=sinh

            normalize(&RReg[0]);
            normalize(&RReg[1]);

            mulReal(&RReg[3], &RReg[9], &RReg[0]);
            mulReal(&RReg[4], &RReg[8], &RReg[1]);

            RReg[4].flags ^= F_NEGATIVE;

            // RETURN THE COMPLEX NUMBER

            if(angmode != ANGLENONE) {
                // NEED TO RETURN A POLAR NUMBER

                rplRect2Polar(&RReg[3], &RReg[4], angmode);

                WORDPTR newcmplx = rplNewComplex(&RReg[0], &RReg[1], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[1]);

                return;

            }

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[3], &RReg[4], angmode);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[3]);
            rplCheckResultAndError(&RReg[4]);

            return;

        }

        angmode = ANGLEMODE(*arg);

        if(angmode == ANGLENONE) {
            if(rplTestSystemFlag(FL_FORCED_RAD))
                angmode = ANGLERAD;
            else
                angmode =
                        rplTestSystemFlag(FL_ANGLEMODE1) |
                        (rplTestSystemFlag(FL_ANGLEMODE2) << 1);
        }
        rplClrSystemFlag(FL_FORCED_RAD);

        if(ISANGLE(*arg))
            rplReadNumberAsReal(arg + 1, &dec);
        else
            rplReadNumberAsReal(arg, &dec);

        if(Exceptions)
            return;

        trig_cos(&dec, angmode);

        finalize(&RReg[0]);
        rplDropData(1);
        rplNewRealFromRRegPush(0);      // COS
        return;

    }
    case TAN:
    {
        //@SHORT_DESC=Compute the tangent
        REAL dec;
        BINT angmode;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            rplListUnaryDoCmd();

            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_INF | CPLX_POLAR:
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            rplClrSystemFlag(FL_FORCED_RAD);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            rplClrSystemFlag(FL_FORCED_RAD);
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        // SUPPORT FOR COMPLEX ARGUMENTS
        if(ISCOMPLEX(*arg)) {
            rplClrSystemFlag(FL_FORCED_RAD);
            REAL re, im;
            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // WE GOT A POLAR COMPLEX NUMBER
                // CONVERT TO POLAR COORDINATES
                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

            }
            else {
                copyReal(&RReg[8], &re);
                copyReal(&RReg[9], &im);
            }

            // COMPUTE AS FOLLOWS:
            // K = (COS(X)*COSH(Y))^2+(SIN(X)*SINH(Y))^2
            // TAN(X+I*Y)=COS(X)*SIN(X)/K+SINH(Y)*COSH(Y)/K*i

            trig_sincos(&RReg[8], ANGLERAD);

            normalize(&RReg[6]);
            normalize(&RReg[7]);

            swapReal(&RReg[9], &RReg[6]);       // RReg[9]=cos
            swapReal(&RReg[8], &RReg[7]);       // RReg[8]=sin

            // NOW THE Y COMPONENT IS IN RReg[6]

            hyp_sinhcosh(&RReg[6]);

            // RReg[0]=cosh
            // RReg[1]=sinh

            normalize(&RReg[0]);
            normalize(&RReg[1]);

            // RReg[0]=K
            mulReal(&RReg[5], &RReg[0], &RReg[9]);      // cosh*cos
            mulReal(&RReg[4], &RReg[5], &RReg[5]);      // ^2
            mulReal(&RReg[5], &RReg[1], &RReg[8]);      // sinh*sin
            mulReal(&RReg[3], &RReg[5], &RReg[5]);      // ^2
            addReal(&RReg[2], &RReg[3], &RReg[4]);      // K

            mulReal(&RReg[5], &RReg[8], &RReg[9]);      // sin*cos
            divReal(&RReg[3], &RReg[5], &RReg[2]);      // sin*cos/K
            mulReal(&RReg[5], &RReg[0], &RReg[1]);      // sinh*cosh
            divReal(&RReg[4], &RReg[5], &RReg[2]);      // sinh*cosh/K

            // RETURN THE COMPLEX NUMBER

            if(angmode != ANGLENONE) {
                // NEED TO RETURN A POLAR NUMBER

                rplRect2Polar(&RReg[3], &RReg[4], angmode);

                WORDPTR newcmplx = rplNewComplex(&RReg[0], &RReg[1], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[1]);

                return;

            }

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[3], &RReg[4], angmode);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[3]);
            rplCheckResultAndError(&RReg[4]);

            return;

        }

        angmode = ANGLEMODE(*arg);

        if(angmode == ANGLENONE) {
            if(rplTestSystemFlag(FL_FORCED_RAD))
                angmode = ANGLERAD;
            else
                angmode =
                        rplTestSystemFlag(FL_ANGLEMODE1) |
                        (rplTestSystemFlag(FL_ANGLEMODE2) << 1);
        }
        rplClrSystemFlag(FL_FORCED_RAD);

        if(ISANGLE(*arg))
            rplReadNumberAsReal(arg + 1, &dec);
        else
            rplReadNumberAsReal(arg, &dec);

        if(Exceptions)
            return;

        trig_tan(&dec, angmode);
        finalize(&RReg[0]);
        rplDropData(1);
        rplNewRealFromRRegPush(0);      // TAN
        rplCheckResultAndError(&RReg[0]);

        return;

    }
    case ASIN:
    {
        //@SHORT_DESC=Compute the arcsine
        REAL y;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_INF | CPLX_POLAR:
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {

            REAL re, im, one;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // WE GOT A POLAR COMPLEX NUMBER
                // CONVERT TO RECT. COORDINATES
                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

                re.data = allocRegister();
                im.data = allocRegister();

                copyReal(&re, &RReg[8]);
                copyReal(&im, &RReg[9]);

            }

            // COMPLEX ARGUMENT, SAME METHOD AS REAL ARGUMENT OUTSIDE DOMAIN
            // COMPUTE FROM
            //ASIN(Z)=-i*LN(i*Z+SQRT(1-Z^2))

            // Z^2 = (re^2-im^2+2*i*re*im)
            // 1-Z^2 = (1-re^2+im^2)-i*(2*re*im)
            // BUT WE NEED IT IN POLAR FORM TO COMPUTE SQRT
            // SQRT(r) = SQRT(SQRT( (1-re^2+im^2)^2 + 4*re^2*im^2 ))
            // arg = atan2( 2*re*im , 1-re^2+im^2 )
            // NOW ADD i*Z, SO WE NEED IT BACK IN CARTESIAN COORDINATES
            // FINALLY, LN REQUIRES ARGUMENT IN POLAR COORDINATES AGAIN

            decconst_One(&one);
            mulReal(&RReg[0], &re, &re);
            mulReal(&RReg[1], &im, &im);

            subReal(&RReg[6], &one, &RReg[0]);
            addReal(&RReg[8], &RReg[6], &RReg[1]);      // 1-re^2+im^2
            mulReal(&RReg[5], &re, &im);
            addReal(&RReg[9], &RReg[5], &RReg[5]);      // -2*re*im
            RReg[9].flags ^= F_NEGATIVE;

            // CONVERT TO POLAR
            trig_atan2(&RReg[9], &RReg[8], ANGLERAD);

            normalize(&RReg[0]);

            newRealFromBINT(&RReg[2], 5, -1);

            mulReal(&RReg[1], &RReg[0], &RReg[2]);      // ANGLE/2 TO GET THE SQUARE ROOT

            mulReal(&RReg[2], &RReg[8], &RReg[8]);
            mulReal(&RReg[3], &RReg[9], &RReg[9]);
            addReal(&RReg[4], &RReg[2], &RReg[3]);      // r^2

            swapReal(&RReg[1], &RReg[8]);       // SAVE THE ANGLE

            hyp_sqrt(&RReg[4]); // r
            normalize(&RReg[0]);
            hyp_sqrt(&RReg[0]); // sqrt(r)
            normalize(&RReg[0]);
            swapReal(&RReg[0], &RReg[9]);       // SAVE SQRT(r)

            trig_sincos(&RReg[8], ANGLERAD);

            normalize(&RReg[6]);
            normalize(&RReg[7]);
            // RReg[6]= cos
            // RReg[7]= sin

            mulReal(&RReg[0], &RReg[9], &RReg[6]);      // REAL COMPONENT
            mulReal(&RReg[1], &RReg[9], &RReg[7]);      // IMAG. COMPONENT

            // NOW ADD i*Z = -im+i*re

            subReal(&RReg[8], &RReg[0], &im);
            addReal(&RReg[9], &RReg[1], &re);

            // CONVERT TO POLAR TO COMPUTE THE LN()

            trig_atan2(&RReg[9], &RReg[8], ANGLERAD);
            normalize(&RReg[0]);

            mulReal(&RReg[1], &RReg[8], &RReg[8]);
            mulReal(&RReg[2], &RReg[9], &RReg[9]);
            addReal(&RReg[4], &RReg[1], &RReg[2]);

            swapReal(&RReg[0], &RReg[9]);       // KEEP THE ANGLE, THIS IS THE IMAGINARY PART OF THE LN()

            hyp_sqrt(&RReg[4]);
            normalize(&RReg[0]);

            hyp_ln(&RReg[0]);

            finalize(&RReg[0]); // LN(r')

            // GOT THE CARTESIAN COMPLEX RESULT

            RReg[0].flags ^= F_NEGATIVE;        // -i*ln(...)=

            if(angmode != ANGLENONE) {
                // RELEASE THE EXTRA ALLOCATED MEMORY
                freeRegister(re.data);
                freeRegister(im.data);

                // RETURN A POLAR COMPLEX IN THE SAME ORIGINAL MODE

                mulReal(&RReg[1], &RReg[0], &RReg[0]);
                mulReal(&RReg[2], &RReg[9], &RReg[9]);
                addReal(&RReg[3], &RReg[1], &RReg[2]);
                swapReal(&RReg[0], &RReg[8]);   // PRESERVE THE REAL PART FOR THE ANGLE
                hyp_sqrt(&RReg[3]);
                finalize(&RReg[0]);

                swapReal(&RReg[0], &RReg[8]);

                trig_atan2(&RReg[9], &RReg[0], angmode);

                finalize(&RReg[0]);

                WORDPTR newcmplx = rplNewComplex(&RReg[0], &RReg[8], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[8]);

                return;

            }

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[9], &RReg[0], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[9]);

            return;

        }

        // REAL ARGUMENTS

        rplReadNumberAsReal(rplPeekData(1), &y);

        if(Exceptions)
            return;
        REAL one, pi_2;
        decconst_One(&one);
        BINT signy = y.flags & F_NEGATIVE;
        y.flags ^= signy;

        if(gtReal(&y, &one)) {
            // REAL ARGUMENT, COMPLEX RESULT
            if(!rplTestSystemFlag(FL_COMPLEXMODE)) {
                rplError(ERR_ARGOUTSIDEDOMAIN);
                return;
            }

            y.flags ^= signy;

            // COMPUTE FROM
            //ASIN(Z)=-i*LN(i*Z+SQRT(1-Z^2))

            mulReal(&RReg[1], &y, &y);
            subReal(&RReg[2], &RReg[1], &one);  // Z^2-1 = -(1-Z^2) SINCE WE KNOW IT'S NEGATIVE

            hyp_sqrt(&RReg[2]);
            normalize(&RReg[0]);

            swapReal(&RReg[8], &RReg[0]);

            // THE RESULT OF THE SQRT IS i*RReg[8], SO ADD i*Z

            addReal(&RReg[9], &RReg[8], &y);

            // FOR THE LN, WE HAVE THE MAGNITUDE IN RReg[9], THE ANGLE IS +/- PI/2
            // DEPENDING ON THE SIGN

            decconst_PI_2(&pi_2);
            pi_2.flags = RReg[9].flags & F_NEGATIVE;
            RReg[9].flags &= ~F_NEGATIVE;       // MAKE SURE THE MAGNITUDE IS POSITIVE FOR LN()

            hyp_ln(&RReg[9]);   // -i*ln(Z')=Theta-i*ln(r)
            finalize(&RReg[0]);
            RReg[0].flags ^= F_NEGATIVE;

            copyReal(&RReg[1], &pi_2);
            finalize(&RReg[1]); // ROUND PI/2 TO THE PROPER NUMBER OF DIGITS

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[1], &RReg[0], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[0]);

            return;
        }

        BINT angmode;
        angmode =
                rplTestSystemFlag(FL_ANGLEMODE1) |
                (rplTestSystemFlag(FL_ANGLEMODE2) << 1);

        y.flags ^= signy;
        trig_asin(&y, angmode);

        finalize(&RReg[0]);

        WORDPTR newangle = rplNewAngleFromReal(&RReg[0], angmode);
        if(!newangle)
            return;

        rplOverwriteData(1, newangle);

        return;

    }

    case ACOS:
    {
        //@SHORT_DESC=Compute the arccosine
        REAL y;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_INF | CPLX_POLAR:

        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {

            REAL re, im, one;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // WE GOT A POLAR COMPLEX NUMBER
                // CONVERT TO RECT. COORDINATES
                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

                re.data = allocRegister();
                im.data = allocRegister();

                copyReal(&re, &RReg[8]);
                copyReal(&im, &RReg[9]);

            }

            // COMPLEX ARGUMENT, SAME METHOD AS REAL ARGUMENT OUTSIDE DOMAIN
            // COMPUTE FROM
            //ACOS(Z)=-i*LN(Z+i*SQRT(1-Z^2))

            // Z^2 = (re^2-im^2+2*i*re*im)
            // 1-Z^2 = (1-re^2+im^2)-i*(2*re*im)
            // BUT WE NEED IT IN POLAR FORM TO COMPUTE SQRT
            // SQRT(r) = SQRT(SQRT( (1-re^2+im^2)^2 + 4*re^2*im^2 ))
            // arg = atan2( 2*re*im , 1-re^2+im^2 )
            // NOW ADD i*Z, SO WE NEED IT BACK IN CARTESIAN COORDINATES
            // FINALLY, LN REQUIRES ARGUMENT IN POLAR COORDINATES AGAIN

            decconst_One(&one);
            mulReal(&RReg[0], &re, &re);
            mulReal(&RReg[1], &im, &im);

            subReal(&RReg[6], &one, &RReg[0]);
            addReal(&RReg[8], &RReg[6], &RReg[1]);      // 1-re^2+im^2
            mulReal(&RReg[5], &re, &im);
            addReal(&RReg[9], &RReg[5], &RReg[5]);      // -2*re*im
            RReg[9].flags ^= F_NEGATIVE;

            // CONVERT TO POLAR
            trig_atan2(&RReg[9], &RReg[8], ANGLERAD);

            normalize(&RReg[0]);

            newRealFromBINT(&RReg[2], 5, -1);

            mulReal(&RReg[1], &RReg[0], &RReg[2]);      // ANGLE/2 TO GET THE SQUARE ROOT

            mulReal(&RReg[2], &RReg[8], &RReg[8]);
            mulReal(&RReg[3], &RReg[9], &RReg[9]);
            addReal(&RReg[4], &RReg[2], &RReg[3]);      // r^2

            swapReal(&RReg[1], &RReg[8]);       // SAVE THE ANGLE

            hyp_sqrt(&RReg[4]); // r
            normalize(&RReg[0]);
            hyp_sqrt(&RReg[0]); // sqrt(r)
            normalize(&RReg[0]);
            swapReal(&RReg[0], &RReg[9]);       // SAVE SQRT(r)

            trig_sincos(&RReg[8], ANGLERAD);

            normalize(&RReg[6]);
            normalize(&RReg[7]);
            // RReg[6]= cos
            // RReg[7]= sin

            mulReal(&RReg[0], &RReg[9], &RReg[6]);      // REAL COMPONENT
            mulReal(&RReg[1], &RReg[9], &RReg[7]);      // IMAG. COMPONENT

            // MULTIPLYING BY i
            RReg[1].flags ^= F_NEGATIVE;

            // NOW ADD Z

            addReal(&RReg[8], &RReg[1], &re);
            addReal(&RReg[9], &RReg[0], &im);

            // CONVERT TO POLAR TO COMPUTE THE LN()

            trig_atan2(&RReg[9], &RReg[8], ANGLERAD);
            normalize(&RReg[0]);

            mulReal(&RReg[1], &RReg[8], &RReg[8]);
            mulReal(&RReg[2], &RReg[9], &RReg[9]);
            addReal(&RReg[4], &RReg[1], &RReg[2]);

            swapReal(&RReg[0], &RReg[9]);       // KEEP THE ANGLE, THIS IS THE IMAGINARY PART OF THE LN()

            hyp_sqrt(&RReg[4]);
            normalize(&RReg[0]);

            hyp_ln(&RReg[0]);

            finalize(&RReg[0]); // LN(r')

            RReg[0].flags ^= F_NEGATIVE;        // -i*ln(...)=

            if(angmode != ANGLENONE) {
                // RELEASE THE EXTRA ALLOCATED MEMORY
                freeRegister(re.data);
                freeRegister(im.data);

                // RETURN A POLAR COMPLEX IN THE SAME ORIGINAL MODE

                mulReal(&RReg[1], &RReg[0], &RReg[0]);
                mulReal(&RReg[2], &RReg[9], &RReg[9]);
                addReal(&RReg[3], &RReg[1], &RReg[2]);
                swapReal(&RReg[0], &RReg[8]);   // PRESERVE THE REAL PART FOR THE ANGLE
                hyp_sqrt(&RReg[3]);
                finalize(&RReg[0]);

                swapReal(&RReg[0], &RReg[8]);

                trig_atan2(&RReg[9], &RReg[0], angmode);

                finalize(&RReg[0]);

                WORDPTR newcmplx = rplNewComplex(&RReg[0], &RReg[8], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[8]);

                return;

            }

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[9], &RReg[0], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[9]);

            return;

        }

        // REAL ARGUMENTS

        rplReadNumberAsReal(rplPeekData(1), &y);

        if(Exceptions)
            return;
        REAL one, pi;
        decconst_One(&one);
        BINT signy = y.flags & F_NEGATIVE;
        y.flags ^= signy;

        if(gtReal(&y, &one)) {
            // REAL ARGUMENT, COMPLEX RESULT
            if(!rplTestSystemFlag(FL_COMPLEXMODE)) {
                rplError(ERR_ARGOUTSIDEDOMAIN);
                return;
            }

            y.flags ^= signy;

            // COMPUTE FROM
            //ACOS(Z)=-i*LN(Z+i*SQRT(1-Z^2))

            mulReal(&RReg[1], &y, &y);
            subReal(&RReg[2], &RReg[1], &one);  // Z^2-1 WE KNOW IT'S POSITIVE SINCE ABS(Z)>1

            hyp_sqrt(&RReg[2]);
            normalize(&RReg[0]);

            swapReal(&RReg[8], &RReg[0]);

            // THE RESULT OF THE SQRT IS i*RReg[8]
            // MULTIPLY i*i*RReg[8]=-RReg[8]

            // NOW ADD Z:

            subReal(&RReg[9], &y, &RReg[8]);

            // FOR THE LN, WE HAVE THE MAGNITUDE IN RReg[9], THE ANGLE IS 0 OR PI
            // DEPENDING ON THE SIGN

            if(RReg[9].flags & F_NEGATIVE) {
                decconst_PI(&pi);
                RReg[9].flags &= ~F_NEGATIVE;   // MAKE SURE THE MAGNITUDE IS POSITIVE FOR LN()
                copyReal(&RReg[8], &pi);
                finalize(&RReg[8]);     // ROUND PI TO THE PROPER NUMBER OF DIGITS
            }
            else
                rplZeroToRReg(8);

            hyp_ln(&RReg[9]);   // -i*ln(Z')=Theta-i*ln(r)
            finalize(&RReg[0]);
            RReg[0].flags ^= F_NEGATIVE;

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[0], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[0]);

            return;
        }

        BINT angmode;
        angmode =
                rplTestSystemFlag(FL_ANGLEMODE1) |
                (rplTestSystemFlag(FL_ANGLEMODE2) << 1);

        y.flags ^= signy;
        trig_acos(&y, angmode);

        finalize(&RReg[0]);

        WORDPTR newangle = rplNewAngleFromReal(&RReg[0], angmode);
        if(!newangle)
            return;

        rplOverwriteData(1, newangle);

        return;

    }

    case ATAN:
        // CALCULATE ATAN FROM ATAN2(Y,1)
    {
        //@SHORT_DESC=Compute the arctangent
        REAL y;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        {
            REAL re, im, pi2;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            angmode =
                    rplTestSystemFlag(FL_ANGLEMODE1) |
                    (rplTestSystemFlag(FL_ANGLEMODE2) << 1);

            switch (angmode) {
            case ANGLEDEG:
            case ANGLEDMS:
                decconst_90(&pi2);
                break;
            case ANGLEGRAD:
                decconst_100(&pi2);
                break;
            case ANGLERAD:
            default:
                decconst_PI_2(&pi2);
                break;
            }

            pi2.flags |= (re.flags & F_NEGATIVE);

            WORDPTR newang = rplNewAngleFromReal(&pi2, angmode);
            if(!newang)
                return;
            rplOverwriteData(1, newang);

        }
        case CPLX_INF | CPLX_MALFORMED:
        {
            REAL re, im, pi2;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            angmode =
                    rplTestSystemFlag(FL_ANGLEMODE1) |
                    (rplTestSystemFlag(FL_ANGLEMODE2) << 1);

            switch (angmode) {
            case ANGLEDEG:
            case ANGLEDMS:
                decconst_90(&pi2);
                break;
            case ANGLEGRAD:
                decconst_100(&pi2);
                break;
            case ANGLERAD:
            default:
                decconst_PI_2(&pi2);
                break;
            }

            pi2.flags |= (im.flags & F_NEGATIVE);

            WORDPTR newang = rplNewAngleFromReal(&pi2, angmode);
            if(!newang)
                return;
            rplOverwriteData(1, newang);

        }
            break;
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {
            // ATAN OF A COMPLEX NUMBER
            REAL re, im, one;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // WE GOT A POLAR COMPLEX NUMBER
                // CONVERT TO RECT. COORDINATES
                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

                re.data = allocRegister();
                im.data = allocRegister();

                copyReal(&re, &RReg[8]);
                copyReal(&im, &RReg[9]);

            }

            decconst_One(&one);

            // ATAN(Z)=1/2*i*(ln(1-i*Z)-ln(1+i*Z))
            // = 1/2*i*ln( (1-i*Z)/(1+i*Z) )

            // z'=1-i*Z = (1+im)-i*re
            // Z'=1+i*Z = (1-im)+i*re
            // z'/Z' = [(1+im)-i*re]*[(1-im)-i*re]/[ [(1-im)+i*re]*[(1-im)-i*re] ]
            // Numerator = (1+im)*(1-im) - (1+im)*i*re - (1-im)*i*re + i^2*re^2
            // (1 - im^2) - i*re*(1+im+1-im) - re^2
            // = 1 - im^2 - 2*i*re - re^2
            // = (1-im^2-re^2) - i*2*re
            // Denominator = (1-im^2) - (i*re)^2
            // = 1-im^2+re^2 (real number)
            // RESULT:
            // Re(1-i*Z)/(1+i*Z) = (1-im^2-re^2)/(1-im^2+re^2)
            // Im(1-i*Z)/(1+i*Z) = -2*re/(1-im^2+re^2)

            // WARNING, THERE COULD BE LOSS OF PRECISION HERE WHEN im^2+re^2 IS CLOSE TO ONE
            mulReal(&RReg[0], &im, &im);
            mulReal(&RReg[1], &re, &re);
            subReal(&RReg[2], &one, &RReg[0]);  // 1-im^2
            subReal(&RReg[3], &RReg[2], &RReg[1]);      // 1-im^2-re^2
            addReal(&RReg[4], &RReg[2], &RReg[1]);      // 1-im^2+re^2
            addReal(&RReg[5], &re, &re);        // 2*re
            RReg[5].flags ^= F_NEGATIVE;

            divReal(&RReg[8], &RReg[3], &RReg[4]);      // Re((1-i*Z)/(1+i*Z))
            divReal(&RReg[9], &RReg[5], &RReg[4]);      // Im((1-i*Z)/(1+i*Z))

            // NOW TAKE THE LOGARITHM

            //i*ln(Z)=i*(ln(r)+i*Theta)=-Theta+i*ln(r)

            trig_atan2(&RReg[9], &RReg[8], ANGLERAD);
            normalize(&RReg[0]);

            mulReal(&RReg[1], &RReg[8], &RReg[8]);
            mulReal(&RReg[2], &RReg[9], &RReg[9]);
            addReal(&RReg[4], &RReg[1], &RReg[2]);

            swapReal(&RReg[0], &RReg[9]);       // KEEP THE ANGLE, THIS IS THE IMAGINARY PART OF THE LN()

            hyp_sqrt(&RReg[4]);
            normalize(&RReg[0]);

            hyp_ln(&RReg[0]);

            finalize(&RReg[0]); // LN(r)

            // RETURN A CARTESIAN COMPLEX

            RReg[9].flags ^= F_NEGATIVE;        // i*ln(...)=

            newRealFromBINT(&RReg[2], 5, -1);

            mulReal(&RReg[3], &RReg[0], &RReg[2]);      // IMAGINARY PART
            swapReal(&RReg[3], &RReg[0]);
            mulReal(&RReg[4], &RReg[9], &RReg[2]);      // REAL PART
            swapReal(&RReg[9], &RReg[4]);

            if(angmode != ANGLENONE) {
                // RELEASE THE EXTRA ALLOCATED MEMORY
                freeRegister(re.data);
                freeRegister(im.data);

                // RETURN A POLAR COMPLEX IN THE SAME ORIGINAL MODE

                mulReal(&RReg[1], &RReg[0], &RReg[0]);
                mulReal(&RReg[2], &RReg[9], &RReg[9]);
                addReal(&RReg[3], &RReg[1], &RReg[2]);
                swapReal(&RReg[0], &RReg[8]);   // PRESERVE THE REAL PART FOR THE ANGLE
                hyp_sqrt(&RReg[3]);
                finalize(&RReg[0]);

                swapReal(&RReg[0], &RReg[8]);

                trig_atan2(&RReg[0], &RReg[9], angmode);

                finalize(&RReg[0]);

                WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[0], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[8]);

                return;

            }

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[9], &RReg[0], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[9]);

            return;

        }

        rplReadNumberAsReal(rplPeekData(1), &y);

        if(Exceptions)
            return;
        // WARNING: TRANSCENDENTAL FUNCTIONS OVERWRITE ALL RREGS. INITIAL ARGUMENTS ARE PASSED ON RREG 0, 1 AND 2, SO USING 7 IS SAFE.
        rplOneToRReg(7);

        BINT angmode;
        angmode =
                rplTestSystemFlag(FL_ANGLEMODE1) |
                (rplTestSystemFlag(FL_ANGLEMODE2) << 1);

        trig_atan2(&y, &RReg[7], angmode);
        finalize(&RReg[0]);

        WORDPTR newangle = rplNewAngleFromReal(&RReg[0], angmode);
        if(!newangle)
            return;

        rplOverwriteData(1, newangle);

        rplCheckResultAndError(&RReg[1]);

        return;

    }
    case ATAN2:
        // CALCULATE ATAN IN THE RANGE -PI,+PI
    {
        //@SHORT_DESC=Compute arctangent(y/x)
        REAL y, x;
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1), arg2 = rplPeekData(2);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)
                || ISSYMBOLIC(*arg2) || ISIDENT(*arg2) || ISCONSTANT(*arg2)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 2);
            return;
        }

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg) || ISLIST(*arg2)) {

            rplListBinaryDoCmd();
            return;
        }

        rplReadNumberAsReal(rplPeekData(2), &y);
        rplReadNumberAsReal(rplPeekData(1), &x);

        if(Exceptions)
            return;

        BINT angmode;
        angmode =
                rplTestSystemFlag(FL_ANGLEMODE1) |
                (rplTestSystemFlag(FL_ANGLEMODE2) << 1);

        trig_atan2(&y, &x, angmode);

        finalize(&RReg[0]);

        WORDPTR newangle = rplNewAngleFromReal(&RReg[0], angmode);
        if(!newangle)
            return;

        rplOverwriteData(2, newangle);
        rplDropData(1);

        rplCheckResultAndError(&RReg[0]);
        return;

    }

    case LN:
    {
        //@SHORT_DESC=Compute natural logarithm
        REAL x;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_INF | CPLX_POLAR:
        {
            rplDropData(1);
            rplInfinityToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        }
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        {
            // RETURN -INFINITY AND SET OVERFLOW
            rplInfinityToRReg(0);
            RReg[0].flags |= F_NEGATIVE;
            rplDropData(1);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        }

        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {
            // LOGARITHM OF A COMPLEX NUMBER
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // ALREADY IN POLAR FORM

                // ONLY ACCEPT THE ANGLE IN RADIANS
                trig_convertangle(&im, angmode, ANGLERAD);

                swapReal(&RReg[0], &RReg[9]);

                hyp_ln(&re);
                normalize(&RReg[0]);

                // SINCE LN() MAPS FROM POLAR TO CARTESIAN PLANE, ALWAYS RETURN CARTESIAN COMPLEX

                WORDPTR newcmplx = rplNewComplex(&RReg[9], &RReg[0], ANGLENONE);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[9]);

                return;

            }

            // CONVERT TO POLAR AND COMPUTE LN()

            mulReal(&RReg[0], &re, &re);
            mulReal(&RReg[1], &im, &im);
            addReal(&RReg[2], &RReg[0], &RReg[1]);

            hyp_sqrt(&RReg[2]);
            normalize(&RReg[0]);
            hyp_ln(&RReg[0]);
            finalize(&RReg[0]);

            swapReal(&RReg[8], &RReg[0]);       // SAVE ln(r) IN RReg[8]

            trig_atan2(&im, &re, ANGLERAD);
            finalize(&RReg[0]);

            WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[0], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[8]);
            rplCheckResultAndError(&RReg[0]);

            return;

        }

        rplReadNumberAsReal(rplPeekData(1), &x);

        if(Exceptions)
            return;

        if(x.flags & F_NEGATIVE) {
            if(rplTestSystemFlag(FL_COMPLEXMODE)) {
                // RETURN THE COMPLEX RESULT

                x.flags ^= F_NEGATIVE;

                hyp_ln(&x);
                finalize(&RReg[0]);

                REAL pi;

                decconst_PI(&pi);

                copyReal(&RReg[1], &pi);

                finalize(&RReg[1]);

                WORDPTR newcmplx = rplNewComplex(&RReg[0], &RReg[1], ANGLENONE);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);

                return;

            }
            rplError(ERR_ARGOUTSIDEDOMAIN);
            return;
        }

        hyp_ln(&x);
        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }

    case EXP:
    {
        //@SHORT_DESC=Compute exponential function
        REAL dec;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(re.flags & F_NEGATIVE) {
                rplOverwriteData(1, (WORDPTR) zero_bint);
            }
            else {
                rplDropData(1);
                rplInfinityToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        }
        case CPLX_INF | CPLX_POLAR:
        {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            REAL pi2;
            switch (angmode) {
            case ANGLEDEG:
            case ANGLEDMS:
                decconst_90(&pi2);
                break;
            case ANGLEGRAD:
                decconst_100(&pi2);
                break;
            case ANGLERAD:
            default:
                decconst_PI_2(&pi2);
            }

            im.flags &= ~F_NEGATIVE;

            switch (cmpReal(&im, &pi2)) {
            case 1:    // ARG(Z)> PI/2
                rplOverwriteData(1, (WORDPTR) zero_bint);
                return;
            case -1:
                rplDropData(1);
                rplUndInfinityToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            default:
            case 0:    // ARG(Z)== PI/2
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            }

        }

        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // IN POLAR FORM, CONVERT TO CARTESIAN

                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

            }
            else {
                copyReal(&RReg[8], &re);
                copyReal(&RReg[9], &im);
            }

            // e^(A+i*B)= e^A * e^(i*B)

            hyp_exp(&RReg[8]);
            normalize(&RReg[0]);

            swapReal(&RReg[8], &RReg[0]);

            if(angmode != ANGLENONE) {

                trig_convertangle(&RReg[9], ANGLERAD, angmode);

                // AND RETURN THE POLAR COMPLEX

                WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[0], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[8]);
                rplCheckResultAndError(&RReg[0]);

                return;

            }

            // AND RETURN THE POLAR COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[9], ANGLERAD);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[8]);
            rplCheckResultAndError(&RReg[9]);

            return;

        }

        rplReadNumberAsReal(rplPeekData(1), &dec);
        if(Exceptions)
            return;

        hyp_exp(&dec);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);      // EXP
        return;

    }
    case SINH:
    {
        //@SHORT_DESC=Compute the hyperbolic sine
        REAL dec;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            rplDropData(1);
            rplInfinityToRReg(0);
            RReg[0].flags |= re.flags & F_NEGATIVE;
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        }

        case CPLX_INF | CPLX_POLAR:
        {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(iszeroReal(&im)) {
                // SINH(Inf) = Inf
                rplDropData(1);
                rplInfinityToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            }
            REAL pi;
            switch (angmode) {
            case ANGLEDEG:
            case ANGLEDMS:
                decconst_180(&pi);
                break;
            case ANGLEGRAD:
                decconst_200(&pi);
                break;
            case ANGLERAD:
            default:
                // NO NUMBER CAN BE EXACTLY EQUAL TO PI OR PI/2, THEREFORE IT'S UNDINF
                rplDropData(1);
                rplUndInfinityToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            }

            im.flags &= ~F_NEGATIVE;

            if(eqReal(&im, &pi)) {
                // SINH(-Inf)=-Inf
                rplDropData(1);
                rplInfinityToRReg(0);
                RReg[0].flags |= F_NEGATIVE;
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            }

            addReal(&RReg[0], &im, &im);

            if(eqReal(&RReg[0], &pi)) {
                // SINH(+/-i*Inf)=NaN
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            }

            // SINH(Z*Inf) = UndInf
            rplDropData(1);
            rplUndInfinityToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;

        }
        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {
            // SINH OF A COMPLEX NUMBER

            REAL re, im, one;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            decconst_One(&one);

            if(angmode != ANGLENONE) {
                // IN POLAR FORM, CONVERT TO CARTESIAN

                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

            }
            else {
                copyReal(&RReg[8], &re);
                copyReal(&RReg[9], &im);
            }

            // SINH()=(e^2Z-1)/2e^Z
            // e^2Z = (e^Z)^2
            // (a+i*b)^2=a^2+2*a*i*b-b^2= (a^2-b^2) + (2*a*b)*i

            // e^(A+i*B)= e^A * e^(i*B)

            hyp_exp(&RReg[8]);
            normalize(&RReg[0]);

            swapReal(&RReg[8], &RReg[0]);

            // CONVERT TO CARTESIAN TO DO THE SUBTRACTION

            trig_sincos(&RReg[9], ANGLERAD);

            // RReg[6]= cos
            // RReg[7]= sin
            normalize(&RReg[6]);
            normalize(&RReg[7]);

            mulReal(&RReg[0], &RReg[6], &RReg[8]);
            mulReal(&RReg[1], &RReg[7], &RReg[8]);

            swapReal(&RReg[0], &RReg[8]);
            swapReal(&RReg[1], &RReg[9]);

            // (e^Z)^2 -1 =
            // (a+i*b)^2=a^2+2*a*i*b-b^2= (a^2-b^2) + (2*a*b)*i

            mulReal(&RReg[0], &RReg[8], &RReg[8]);      // a^2
            mulReal(&RReg[1], &RReg[9], &RReg[9]);      // b^2
            subReal(&RReg[2], &RReg[0], &RReg[1]);
            subReal(&RReg[3], &RReg[2], &one);  // (a^2-b^2-1)

            mulReal(&RReg[4], &RReg[8], &RReg[9]);
            addReal(&RReg[5], &RReg[4], &RReg[4]);      // (2*a*b)

            // (a+i*b)/(c+i*d)=(a+i*b)*(c-i*d)/(c^2+d^2)
            // (a*c+b*d) + i * (b*c-a*d) /(c^2+d^2)

            addReal(&RReg[2], &RReg[0], &RReg[1]);      // a^2+b^2
            addReal(&RReg[4], &RReg[2], &RReg[2]);      // 2*(a^2+b^2)

            // COMPUTE REAL PART
            mulReal(&RReg[0], &RReg[3], &RReg[8]);      // a*c
            mulReal(&RReg[1], &RReg[5], &RReg[9]);      // d*b
            addReal(&RReg[2], &RReg[0], &RReg[1]);      // a*c+b*d
            divReal(&RReg[6], &RReg[2], &RReg[4]);      // /2*(a^2+B^2)=  REAL PART OF RESULT

            // COMPUTE IMAG PART
            mulReal(&RReg[0], &RReg[5], &RReg[8]);      // b*c
            mulReal(&RReg[1], &RReg[3], &RReg[9]);      // a*d
            subReal(&RReg[2], &RReg[0], &RReg[1]);      // b*c-a*d
            divReal(&RReg[7], &RReg[2], &RReg[4]);      // /2*(a^2+B^2)=  IMAG PART OF RESULT

            if(angmode != ANGLENONE) {

                // RETURN A POLAR COMPLEX IN THE SAME ORIGINAL MODE

                swapReal(&RReg[6], &RReg[8]);
                swapReal(&RReg[7], &RReg[9]);

                mulReal(&RReg[1], &RReg[8], &RReg[8]);
                mulReal(&RReg[2], &RReg[9], &RReg[9]);
                addReal(&RReg[3], &RReg[1], &RReg[2]);
                hyp_sqrt(&RReg[3]);
                finalize(&RReg[0]);

                swapReal(&RReg[0], &RReg[8]);

                trig_atan2(&RReg[0], &RReg[9], angmode);

                finalize(&RReg[0]);

                WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[0], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[8]);

                return;

            }

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[6], &RReg[7], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[9]);

            return;

        }

        rplReadNumberAsReal(rplPeekData(1), &dec);
        if(Exceptions)
            return;

        hyp_sinh(&dec);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);      // SINH
        return;

    }

    case COSH:
    {
        //@SHORT_DESC=Compute the hyperbolic cosine
        REAL dec;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        {
            // COSH(+/-Inf)= +Inf
            rplDropData(1);
            rplInfinityToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        }

        case CPLX_INF | CPLX_POLAR:
        {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(iszeroReal(&im)) {
                // COSH(Inf) = Inf
                rplDropData(1);
                rplInfinityToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            }
            REAL pi;
            switch (angmode) {
            case ANGLEDEG:
            case ANGLEDMS:
                decconst_180(&pi);
                break;
            case ANGLEGRAD:
                decconst_200(&pi);
                break;
            case ANGLERAD:
            default:
                // NO NUMBER CAN BE EXACTLY EQUAL TO PI OR PI/2, THEREFORE IT'S UNDINF
                rplDropData(1);
                rplUndInfinityToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            }

            im.flags &= ~F_NEGATIVE;

            if(eqReal(&im, &pi)) {
                // COSH(-Inf)=+Inf
                rplDropData(1);
                rplInfinityToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            }

            addReal(&RReg[0], &im, &im);

            if(eqReal(&RReg[0], &pi)) {
                // COSH(+/-i*Inf)=NaN
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            }

            // COSH(Z*Inf) = UndInf
            rplDropData(1);
            rplUndInfinityToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;

        }
        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {
            // COSH OF A COMPLEX NUMBER

            REAL re, im, one;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            decconst_One(&one);

            if(angmode != ANGLENONE) {
                // IN POLAR FORM, CONVERT TO CARTESIAN

                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

            }
            else {
                copyReal(&RReg[8], &re);
                copyReal(&RReg[9], &im);
            }

            // COSH()=(e^2Z+1)/2e^Z
            // e^2Z = (e^Z)^2
            // (a+i*b)^2=a^2+2*a*i*b-b^2= (a^2-b^2) + (2*a*b)*i

            // e^(A+i*B)= e^A * e^(i*B)

            hyp_exp(&RReg[8]);
            normalize(&RReg[0]);

            swapReal(&RReg[8], &RReg[0]);

            // CONVERT TO CARTESIAN TO DO THE SUBTRACTION

            trig_sincos(&RReg[9], ANGLERAD);

            // RReg[6]= cos
            // RReg[7]= sin
            normalize(&RReg[6]);
            normalize(&RReg[7]);

            mulReal(&RReg[0], &RReg[6], &RReg[8]);
            mulReal(&RReg[1], &RReg[7], &RReg[8]);

            swapReal(&RReg[0], &RReg[8]);
            swapReal(&RReg[1], &RReg[9]);

            // (e^Z)^2 +1 =
            // (a+i*b)^2=a^2+2*a*i*b-b^2= (a^2-b^2) + (2*a*b)*i

            mulReal(&RReg[0], &RReg[8], &RReg[8]);      // a^2
            mulReal(&RReg[1], &RReg[9], &RReg[9]);      // b^2
            subReal(&RReg[2], &RReg[0], &RReg[1]);
            addReal(&RReg[3], &RReg[2], &one);  // (a^2-b^2+1)

            mulReal(&RReg[4], &RReg[8], &RReg[9]);
            addReal(&RReg[5], &RReg[4], &RReg[4]);      // (2*a*b)

            // (a+i*b)/(c+i*d)=(a+i*b)*(c-i*d)/(c^2+d^2)
            // (a*c+b*d) + i * (b*c-a*d) /(c^2+d^2)

            addReal(&RReg[2], &RReg[0], &RReg[1]);      // a^2+b^2
            addReal(&RReg[4], &RReg[2], &RReg[2]);      // 2*(a^2+b^2)

            // COMPUTE REAL PART
            mulReal(&RReg[0], &RReg[3], &RReg[8]);      // a*c
            mulReal(&RReg[1], &RReg[5], &RReg[9]);      // d*b
            addReal(&RReg[2], &RReg[0], &RReg[1]);      // a*c+b*d
            divReal(&RReg[6], &RReg[2], &RReg[4]);      // /2*(a^2+B^2)=  REAL PART OF RESULT

            // COMPUTE IMAG PART
            mulReal(&RReg[0], &RReg[5], &RReg[8]);      // b*c
            mulReal(&RReg[1], &RReg[3], &RReg[9]);      // a*d
            subReal(&RReg[2], &RReg[0], &RReg[1]);      // b*c-a*d
            divReal(&RReg[7], &RReg[2], &RReg[4]);      // /2*(a^2+B^2)=  IMAG PART OF RESULT

            if(angmode != ANGLENONE) {

                // RETURN A POLAR COMPLEX IN THE SAME ORIGINAL MODE

                swapReal(&RReg[6], &RReg[8]);
                swapReal(&RReg[7], &RReg[9]);

                mulReal(&RReg[1], &RReg[8], &RReg[8]);
                mulReal(&RReg[2], &RReg[9], &RReg[9]);
                addReal(&RReg[3], &RReg[1], &RReg[2]);
                hyp_sqrt(&RReg[3]);
                finalize(&RReg[0]);

                swapReal(&RReg[0], &RReg[8]);

                trig_atan2(&RReg[0], &RReg[9], angmode);

                finalize(&RReg[0]);

                WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[0], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[8]);

                return;

            }

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[6], &RReg[7], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[9]);

            return;

        }

        rplReadNumberAsReal(rplPeekData(1), &dec);
        if(Exceptions)
            return;

        hyp_cosh(&dec);
        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);      // COSH
        return;

    }

    case TANH:

    {
        //@SHORT_DESC=Compute the hyperbolic tangent
        REAL dec;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            // TANH(+/-Inf)=+/- 1
            rplDropData(1);
            if(re.flags & F_NEGATIVE)
                rplNewBINTPush(-1, DECBINT);
            else
                rplPushData((WORDPTR) one_bint);
            return;
        }

        case CPLX_INF | CPLX_POLAR:
        {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            REAL pi2;
            switch (angmode) {
            case ANGLEDEG:
            case ANGLEDMS:
                decconst_90(&pi2);
                break;
            case ANGLEGRAD:
                decconst_100(&pi2);
                break;
            case ANGLERAD:
            default:
                decconst_PI_2(&pi2);
            }

            im.flags &= ~F_NEGATIVE;

            switch (cmpReal(&im, &pi2)) {
            case 1:    // ARG(Z)> PI/2
                rplDropData(1);
                rplNewBINTPush(-1, DECBINT);
                return;
            case -1:
                rplOverwriteData(1, (WORDPTR) one_bint);
                return;
            default:
            case 0:    // ARG(Z)== PI/2
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;

            }

        }
        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {
            // TANH OF A COMPLEX NUMBER

            REAL re, im, one;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            decconst_One(&one);

            if(angmode != ANGLENONE) {
                // IN POLAR FORM, CONVERT TO CARTESIAN

                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[0], &RReg[6], &re);
                mulReal(&RReg[1], &RReg[7], &re);
                addReal(&RReg[8], &RReg[0], &RReg[0]);
                addReal(&RReg[9], &RReg[1], &RReg[1]);

            }
            else {
                addReal(&RReg[8], &re, &re);
                addReal(&RReg[9], &im, &im);
            }

            // TANH()=(e^2Z-1)/(e^2Z+1)

            hyp_exp(&RReg[8]);
            normalize(&RReg[0]);

            swapReal(&RReg[8], &RReg[0]);

            // CONVERT TO CARTESIAN TO DO THE SUBTRACTION

            trig_sincos(&RReg[9], ANGLERAD);

            // RReg[6]= cos
            // RReg[7]= sin
            normalize(&RReg[6]);
            normalize(&RReg[7]);

            mulReal(&RReg[0], &RReg[6], &RReg[8]);
            mulReal(&RReg[1], &RReg[7], &RReg[8]);

            swapReal(&RReg[0], &RReg[8]);
            swapReal(&RReg[1], &RReg[9]);

            // (e^2Z)+1 and (e^2Z)-1

            addReal(&RReg[0], &RReg[8], &one);  // c= REAL PART (DENOMINATOR)
            subReal(&RReg[1], &RReg[8], &one);  // a= REAL PART (NUMERATOR)

            // (a+i*b)/(c+i*b) = (a+i*b)*(c-i*b)/(c^2+b^2)
            // = (a*c+b^2) + i* (b*c-b*a)
            // = (a*c+b^2) + i*b*(c-a)

            mulReal(&RReg[2], &RReg[0], &RReg[1]);      // a*c
            mulReal(&RReg[3], &RReg[9], &RReg[9]);      // b^2
            mulReal(&RReg[4], &RReg[0], &RReg[0]);      // c^2
            subReal(&RReg[5], &RReg[0], &RReg[1]);      // c-a
            mulReal(&RReg[7], &RReg[9], &RReg[5]);      // b*(c-a)
            addReal(&RReg[6], &RReg[3], &RReg[2]);      // a*c+b^2
            addReal(&RReg[5], &RReg[3], &RReg[4]);      // b^2+c^2
            divReal(&RReg[8], &RReg[6], &RReg[5]);
            divReal(&RReg[9], &RReg[7], &RReg[5]);

            // HERE 8 AND 9 HAVE THE RESULT'S REAL AND IMAGINARY PARTS

            if(angmode != ANGLENONE) {

                // RETURN A POLAR COMPLEX IN THE SAME ORIGINAL MODE

                mulReal(&RReg[1], &RReg[8], &RReg[8]);
                mulReal(&RReg[2], &RReg[9], &RReg[9]);
                addReal(&RReg[3], &RReg[1], &RReg[2]);
                hyp_sqrt(&RReg[3]);
                finalize(&RReg[0]);

                swapReal(&RReg[0], &RReg[8]);

                trig_atan2(&RReg[0], &RReg[9], angmode);

                finalize(&RReg[0]);

                WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[0], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[8]);

                return;

            }

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[9], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[8]);
            rplCheckResultAndError(&RReg[9]);

            return;

        }

        rplReadNumberAsReal(rplPeekData(1), &dec);
        if(Exceptions)
            return;

        hyp_tanh(&dec);

        normalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);      // TANH
        return;

    }

    case ASINH:
    {
        //@SHORT_DESC=Compute the hyperbolic arcsine
        REAL x;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            // ASINH(+/-Inf)=+/- Inf
            rplDropData(1);
            rplInfinityToRReg(0);
            RReg[0].flags |= re.flags & F_NEGATIVE;
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        }

        case CPLX_INF | CPLX_POLAR:
        {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            REAL pi2;
            switch (angmode) {
            case ANGLEDEG:
            case ANGLEDMS:
                decconst_90(&pi2);
                break;
            case ANGLEGRAD:
                decconst_100(&pi2);
                break;
            case ANGLERAD:
            default:
                decconst_PI_2(&pi2);
            }

            im.flags &= ~F_NEGATIVE;

            switch (cmpReal(&im, &pi2)) {
            case 1:    // ARG(Z)> PI/2
                rplDropData(1);
                rplInfinityToRReg(0);
                RReg[0].flags |= F_NEGATIVE;
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            case -1:
                rplDropData(1);
                rplInfinityToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            default:
            case 0:    // ARG(Z)== PI/2
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;

            }

        }
        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {

            REAL re, im, one;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // WE GOT A POLAR COMPLEX NUMBER
                // CONVERT TO RECT. COORDINATES
                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

                re.data = allocRegister();
                im.data = allocRegister();

                copyReal(&re, &RReg[8]);
                copyReal(&im, &RReg[9]);

            }

            // COMPLEX ARGUMENT, SAME METHOD AS REAL ARGUMENT OUTSIDE DOMAIN
            // COMPUTE FROM
            //ASINH(Z)=LN(Z+SQRT(1+Z^2))

            // Z^2 = (re^2-im^2+2*i*re*im)
            // 1+Z^2 = (1+re^2-im^2)+i*(2*re*im)
            // BUT WE NEED IT IN POLAR FORM TO COMPUTE SQRT
            // SQRT(r) = SQRT(SQRT( (1+re^2-im^2)^2 + 4*re^2*im^2 ))
            // arg = atan2( 2*re*im , 1+re^2-im^2 )
            // NOW ADD Z, SO WE NEED IT BACK IN CARTESIAN COORDINATES
            // FINALLY, LN REQUIRES ARGUMENT IN POLAR COORDINATES AGAIN

            decconst_One(&one);
            mulReal(&RReg[0], &re, &re);
            mulReal(&RReg[1], &im, &im);

            addReal(&RReg[6], &one, &RReg[0]);
            subReal(&RReg[8], &RReg[6], &RReg[1]);      // 1+re^2-im^2
            mulReal(&RReg[5], &re, &im);
            addReal(&RReg[9], &RReg[5], &RReg[5]);      // 2*re*im

            // CONVERT TO POLAR
            trig_atan2(&RReg[9], &RReg[8], ANGLERAD);

            normalize(&RReg[0]);

            newRealFromBINT(&RReg[2], 5, -1);

            mulReal(&RReg[1], &RReg[0], &RReg[2]);      // ANGLE/2 TO GET THE SQUARE ROOT

            mulReal(&RReg[2], &RReg[8], &RReg[8]);
            mulReal(&RReg[3], &RReg[9], &RReg[9]);
            addReal(&RReg[4], &RReg[2], &RReg[3]);      // r^2

            swapReal(&RReg[1], &RReg[8]);       // SAVE THE ANGLE

            hyp_sqrt(&RReg[4]); // r
            normalize(&RReg[0]);
            hyp_sqrt(&RReg[0]); // sqrt(r)
            normalize(&RReg[0]);
            swapReal(&RReg[0], &RReg[9]);       // SAVE SQRT(r)

            trig_sincos(&RReg[8], ANGLERAD);

            normalize(&RReg[6]);
            normalize(&RReg[7]);
            // RReg[6]= cos
            // RReg[7]= sin

            mulReal(&RReg[0], &RReg[9], &RReg[6]);      // REAL COMPONENT
            mulReal(&RReg[1], &RReg[9], &RReg[7]);      // IMAG. COMPONENT

            // NOW ADD Z = re+i*im

            addReal(&RReg[8], &RReg[0], &re);
            addReal(&RReg[9], &RReg[1], &im);

            // CONVERT TO POLAR TO COMPUTE THE LN()

            trig_atan2(&RReg[9], &RReg[8], ANGLERAD);
            normalize(&RReg[0]);

            mulReal(&RReg[1], &RReg[8], &RReg[8]);
            mulReal(&RReg[2], &RReg[9], &RReg[9]);
            addReal(&RReg[4], &RReg[1], &RReg[2]);

            swapReal(&RReg[0], &RReg[9]);       // KEEP THE ANGLE, THIS IS THE IMAGINARY PART OF THE LN()

            hyp_sqrt(&RReg[4]);
            normalize(&RReg[0]);

            hyp_ln(&RReg[0]);

            finalize(&RReg[0]); // LN(r')

            // GOT THE CARTESIAN COMPLEX RESULT

            if(angmode != ANGLENONE) {
                // RELEASE THE EXTRA ALLOCATED MEMORY
                freeRegister(re.data);
                freeRegister(im.data);

                // RETURN A POLAR COMPLEX IN THE SAME ORIGINAL MODE

                mulReal(&RReg[1], &RReg[0], &RReg[0]);
                mulReal(&RReg[2], &RReg[9], &RReg[9]);
                addReal(&RReg[3], &RReg[1], &RReg[2]);
                swapReal(&RReg[0], &RReg[8]);   // PRESERVE THE REAL PART FOR THE ANGLE
                hyp_sqrt(&RReg[3]);
                finalize(&RReg[0]);

                swapReal(&RReg[0], &RReg[8]);

                trig_atan2(&RReg[9], &RReg[0], angmode);

                finalize(&RReg[0]);

                WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[0], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[8]);

                return;

            }

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[0], &RReg[9], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[9]);

            return;

        }

        rplReadNumberAsReal(rplPeekData(1), &x);

        if(Exceptions)
            return;

        hyp_asinh(&x);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }

        return;

    case ACOSH:
    {
        //@SHORT_DESC=Compute the hyperbolic arccosine
        REAL x;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        case CPLX_INF | CPLX_POLAR:
        case CPLX_INF | CPLX_MALFORMED:
        {
            rplDropData(1);
            rplInfinityToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        }

        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {

            REAL re, im, one;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // WE GOT A POLAR COMPLEX NUMBER
                // CONVERT TO RECT. COORDINATES
                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

                re.data = allocRegister();
                im.data = allocRegister();

                copyReal(&re, &RReg[8]);
                copyReal(&im, &RReg[9]);

            }

            // COMPLEX ARGUMENT, SAME METHOD AS REAL ARGUMENT OUTSIDE DOMAIN
            // COMPUTE FROM
            //ACOSH(Z)=LN(Z+SQRT(1+Z)*SQRT(Z-1))

            // Z^2 = (re^2-im^2+2*i*re*im)
            // 1-Z^2 = (1-re^2+im^2)-i*(2*re*im)
            // BUT WE NEED IT IN POLAR FORM TO COMPUTE SQRT
            // SQRT(r) = SQRT(SQRT( (1-re^2+im^2)^2 + 4*re^2*im^2 ))
            // arg = atan2( 2*re*im , 1-re^2+im^2 )
            // NOW ADD i*Z, SO WE NEED IT BACK IN CARTESIAN COORDINATES
            // FINALLY, LN REQUIRES ARGUMENT IN POLAR COORDINATES AGAIN

            decconst_One(&one);
            addReal(&RReg[9], &re, &one);

            // CONVERT TO POLAR
            trig_atan2(&im, &RReg[9], ANGLERAD);        // atan(im/(re+1))

            normalize(&RReg[0]);

            swapReal(&RReg[8], &RReg[0]);

            subReal(&RReg[9], &re, &one);

            // CONVERT TO POLAR
            trig_atan2(&im, &RReg[9], ANGLERAD);        // atan(im/(re-1))

            normalize(&RReg[0]);

            addReal(&RReg[1], &RReg[8], &RReg[0]);

            newRealFromBINT(&RReg[2], 5, -1);

            mulReal(&RReg[9], &RReg[1], &RReg[2]);      // (ANG1+ANG2)/2 TO GET THE SQUARE ROOT

            addReal(&RReg[0], &re, &one);
            mulReal(&RReg[1], &RReg[0], &RReg[0]);
            mulReal(&RReg[2], &im, &im);
            addReal(&RReg[3], &RReg[1], &RReg[2]);      // (re+1)^2+im^2

            subReal(&RReg[0], &re, &one);
            mulReal(&RReg[1], &RReg[0], &RReg[0]);
            addReal(&RReg[4], &RReg[1], &RReg[2]);      // (re-1)^2+im^2

            mulReal(&RReg[5], &RReg[3], &RReg[4]);      // r1^2*r2^2

            hyp_sqrt(&RReg[5]); // r1*r2=sqrt(r1^2*r2^2)
            normalize(&RReg[0]);
            hyp_sqrt(&RReg[0]); // sqrt(r1*r2)
            normalize(&RReg[0]);
            swapReal(&RReg[0], &RReg[8]);       // HERE (RReg[8],RReg[9]) = SQRT(Z+1)*SQRT(Z-1)

            trig_sincos(&RReg[9], ANGLERAD);

            normalize(&RReg[6]);
            normalize(&RReg[7]);
            // RReg[6]= cos
            // RReg[7]= sin

            mulReal(&RReg[0], &RReg[8], &RReg[6]);      // REAL COMPONENT
            mulReal(&RReg[1], &RReg[8], &RReg[7]);      // IMAG. COMPONENT

            // NOW ADD Z

            addReal(&RReg[8], &RReg[0], &re);
            addReal(&RReg[9], &RReg[1], &im);

            // CONVERT TO POLAR TO COMPUTE THE LN()

            trig_atan2(&RReg[9], &RReg[8], ANGLERAD);
            normalize(&RReg[0]);

            mulReal(&RReg[1], &RReg[8], &RReg[8]);
            mulReal(&RReg[2], &RReg[9], &RReg[9]);
            addReal(&RReg[4], &RReg[1], &RReg[2]);

            swapReal(&RReg[0], &RReg[9]);       // KEEP THE ANGLE, THIS IS THE IMAGINARY PART OF THE LN()

            hyp_sqrt(&RReg[4]);
            normalize(&RReg[0]);

            hyp_ln(&RReg[0]);

            finalize(&RReg[0]); // LN(r')

            if(angmode != ANGLENONE) {
                // RELEASE THE EXTRA ALLOCATED MEMORY
                freeRegister(re.data);
                freeRegister(im.data);

                // RETURN A POLAR COMPLEX IN THE SAME ORIGINAL MODE

                mulReal(&RReg[1], &RReg[0], &RReg[0]);
                mulReal(&RReg[2], &RReg[9], &RReg[9]);
                addReal(&RReg[3], &RReg[1], &RReg[2]);
                swapReal(&RReg[0], &RReg[8]);   // PRESERVE THE REAL PART FOR THE ANGLE
                hyp_sqrt(&RReg[3]);
                finalize(&RReg[0]);

                swapReal(&RReg[0], &RReg[8]);

                trig_atan2(&RReg[9], &RReg[0], angmode);

                finalize(&RReg[0]);

                WORDPTR newcmplx = rplNewComplex(&RReg[0], &RReg[8], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[8]);

                return;

            }

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[0], &RReg[9], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[9]);

            return;

        }

        rplReadNumberAsReal(rplPeekData(1), &x);

        if(Exceptions)
            return;

        rplOneToRReg(0);

        if(gtReal(&RReg[0], &x)) {
            if(rplTestSystemFlag(FL_COMPLEXMODE)) {
                // RETURN THE COMPLEX RESULT
                //ACOSH(x)=LN(x+SQRT(1+x)*SQRT(x-1))

                addReal(&RReg[1], &RReg[0], &x);
                subReal(&RReg[2], &x, &RReg[0]);
                BINT i = 0;
                if(RReg[1].flags & F_NEGATIVE)
                    ++i;
                if(RReg[2].flags & F_NEGATIVE)
                    ++i;

                mulReal(&RReg[3], &RReg[1], &RReg[2]);
                RReg[3].flags &= ~F_NEGATIVE;   // MAKE SURE IT'S POSITIVE

                hyp_sqrt(&RReg[3]);
                normalize(&RReg[0]);
                switch (i) {

                case 1:
                    // LN() OF THE COMPLEX NUMBER X+i*SQRT(1-X^2) = ACOS(X)

                    // USE REAL PART = 0 AND IMAGINARY PART = ACOS(X)

                    trig_acos(&x, ANGLERAD);
                    finalize(&RReg[0]);

                    swapReal(&RReg[9], &RReg[0]);
                    rplZeroToRReg(8);

                    break;
                case 2:
                    RReg[0].flags |= F_NEGATIVE;
                    // FALL THROUGH
                default:
                case 0:
                    addReal(&RReg[8], &x, &RReg[0]);

                    // LN OF A NEGATIVE REAL NUMBER
                    RReg[8].flags &= ~F_NEGATIVE;
                    hyp_ln(&RReg[8]);

                    finalize(&RReg[0]);
                    swapReal(&RReg[0], &RReg[8]);
                    REAL pi;
                    decconst_PI(&pi);

                    copyReal(&RReg[9], &pi);
                    finalize(&RReg[9]); // TRUNCATE PI TO THE GIVEN PRECISION?

                    break;
                }

                // RETURN THE CARTESIAN COMPLEX

                WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[9], ANGLENONE);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[8]);
                rplCheckResultAndError(&RReg[9]);

                return;

            }
            // TODO: EXPAND THIS TO RETURN COMPLEX VALUES
            rplError(ERR_ARGOUTSIDEDOMAIN);
            return;
        }

        hyp_acosh(&x);
        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }

    case ATANH:
    {
        //@SHORT_DESC=Compute the hyperbolic arctangent
        REAL x;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        {
            REAL pi2;
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            decconst_PI_2(&pi2);

            copyReal(&RReg[0], &pi2);
            finalize(&RReg[0]); // TRIM EXCESS DIGITS
            RReg[0].flags |= (~re.flags) & F_NEGATIVE;
            rplZeroToRReg(1);
            rplDropData(1);
            rplNewComplexPush(&RReg[1], &RReg[0], ANGLENONE);
            if(!rplTestSystemFlag(FL_COMPLEXMODE)) {
                if(!Exceptions)
                    rplError(ERR_COMPLEXRESULT);
            }
            return;
        }

        case CPLX_INF | CPLX_MALFORMED:
        {
            REAL pi2;
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            decconst_PI_2(&pi2);

            copyReal(&RReg[0], &pi2);
            finalize(&RReg[0]); // TRIM EXCESS DIGITS
            RReg[0].flags |= im.flags & F_NEGATIVE;
            rplZeroToRReg(1);
            rplDropData(1);
            rplNewComplexPush(&RReg[1], &RReg[0], ANGLENONE);
            if(!rplTestSystemFlag(FL_COMPLEXMODE)) {
                if(!Exceptions)
                    rplError(ERR_COMPLEXRESULT);
            }
            return;
        }

        case CPLX_INF | CPLX_POLAR:

        {
            REAL pi2;
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            switch (angmode) {
            case ANGLEDEG:
            case ANGLEDMS:
                decconst_90(&pi2);
                break;
            case ANGLEGRAD:
                decconst_100(&pi2);
                break;
            case ANGLERAD:
            default:
                decconst_PI_2(&pi2);
            }

            copyReal(&RReg[0], &pi2);
            finalize(&RReg[0]); // TRIM EXCESS DIGITS
            im.flags &= ~F_NEGATIVE;

            if(cmpReal(&im, &pi2) >= 0)
                RReg[0].flags |= F_NEGATIVE;

            rplZeroToRReg(1);
            rplDropData(1);
            rplNewComplexPush(&RReg[1], &RReg[0], ANGLENONE);
            if(!rplTestSystemFlag(FL_COMPLEXMODE)) {
                if(!Exceptions)
                    rplError(ERR_COMPLEXRESULT);
            }
            return;
        }

        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {

            REAL re, im, one;
            BINT angmode;

            decconst_One(&one);

            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // WE GOT A POLAR COMPLEX NUMBER
                // CONVERT TO RECT. COORDINATES
                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

                re.data = allocRegister();
                im.data = allocRegister();

                copyReal(&re, &RReg[8]);
                copyReal(&im, &RReg[9]);

            }

            // COMPUTE ATANH(Z) = 1/2 LN( (1+Z)/(1-Z) )

            // FIRST DO (1+Z)/(1-Z) = [ (2*(1-re)/((1-re)^2+im^2)-1 ] + i* (2*im/((1-re)^2+im^2))

            subReal(&RReg[0], &one, &re);
            mulReal(&RReg[1], &RReg[0], &RReg[0]);
            mulReal(&RReg[2], &im, &im);
            addReal(&RReg[3], &RReg[1], &RReg[2]);      // DENOMINATOR = (1-re)^2+im^2
            addReal(&RReg[5], &RReg[0], &RReg[0]);      // 2*(1-re)
            divReal(&RReg[4], &RReg[5], &RReg[3]);      // 2*(1-re)/[...]
            subReal(&RReg[8], &RReg[4], &one);  // REAL PART = 2*(1-re)/[...] - 1
            addReal(&RReg[5], &im, &im);
            divReal(&RReg[9], &RReg[5], &RReg[3]);      // IMAG. PART = 2*im /[...]

            // NOW TAKE THE LN()

            // CONVERT TO POLAR TO COMPUTE THE LN()

            trig_atan2(&RReg[9], &RReg[8], ANGLERAD);
            normalize(&RReg[0]);

            mulReal(&RReg[1], &RReg[8], &RReg[8]);
            mulReal(&RReg[2], &RReg[9], &RReg[9]);
            addReal(&RReg[4], &RReg[1], &RReg[2]);

            swapReal(&RReg[0], &RReg[9]);       // KEEP THE ANGLE, THIS IS THE IMAGINARY PART OF THE LN()

            hyp_sqrt(&RReg[4]);
            normalize(&RReg[0]);

            hyp_ln(&RReg[0]);

            finalize(&RReg[0]); // LN(r')

            newRealFromBINT(&RReg[1], 5, -1);

            mulReal(&RReg[8], &RReg[1], &RReg[0]);      // 1/2* LN(...)
            mulReal(&RReg[2], &RReg[1], &RReg[9]);

            swapReal(&RReg[9], &RReg[2]);

            if(angmode != ANGLENONE) {
                // RELEASE THE EXTRA ALLOCATED MEMORY
                freeRegister(re.data);
                freeRegister(im.data);

                // RETURN A POLAR COMPLEX IN THE SAME ORIGINAL MODE

                mulReal(&RReg[1], &RReg[8], &RReg[8]);
                mulReal(&RReg[2], &RReg[9], &RReg[9]);
                addReal(&RReg[3], &RReg[1], &RReg[2]);
                hyp_sqrt(&RReg[3]);
                finalize(&RReg[0]);

                swapReal(&RReg[0], &RReg[8]);

                trig_atan2(&RReg[9], &RReg[0], angmode);

                finalize(&RReg[0]);

                WORDPTR newcmplx = rplNewComplex(&RReg[0], &RReg[8], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[8]);

                return;

            }

            // RETURN A CARTESIAN COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[9], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[8]);
            rplCheckResultAndError(&RReg[9]);

            return;

        }

        // REAL NUMBERS

        rplReadNumberAsReal(rplPeekData(1), &x);

        if(Exceptions)
            return;

        rplOneToRReg(0);
        BINT signx = x.flags;
        x.flags &= ~F_NEGATIVE;
        BINT ismorethan1 = cmpReal(&x, &RReg[0]);
        x.flags = signx;

        if(ismorethan1 == 1)    // x > 1.0
        {
            if(rplTestSystemFlag(FL_COMPLEXMODE)) {
                // TODO: COMPUTE COMPLEX ANSWER

                // ATANH(x)= 1/2*LN( (1+X)/(1-X) )

                addReal(&RReg[1], &RReg[0], &x);
                subReal(&RReg[2], &RReg[0], &x);
                divReal(&RReg[8], &RReg[1], &RReg[2]);

                // LN OF A NEGATIVE REAL

                RReg[8].flags &= ~F_NEGATIVE;

                // HANDLE SPECIAL VALUES
                if(isinfiniteReal(&RReg[8])) {
                    rplError(ERR_INFINITERESULT);
                    return;
                }

                hyp_ln(&RReg[8]);
                normalize(&RReg[0]);

                newRealFromBINT(&RReg[1], 5, -1);

                mulReal(&RReg[8], &RReg[0], &RReg[1]);

                REAL pi;

                decconst_PI_2(&pi);

                copyReal(&RReg[1], &pi);

                if(signx & F_NEGATIVE)
                    RReg[8].flags ^= F_NEGATIVE;
                else
                    RReg[1].flags ^= F_NEGATIVE;

                finalize(&RReg[1]);

                WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[1], ANGLENONE);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[8]);

                return;

            }
            rplError(ERR_ARGOUTSIDEDOMAIN);
            return;

        }

        if(ismorethan1 == 0) {
            rplInfinityToRReg(0);
            if(signx & F_NEGATIVE)
                RReg[0].flags |= F_NEGATIVE;

            rplDropData(1);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        }

        hyp_atanh(&x);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        rplCheckResultAndError(&RReg[0]);

        return;

    }

        return;

    case SQRT:
    {
        //@SHORT_DESC=Compute the square root
        REAL x, y;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(re.flags & F_NEGATIVE) {

                REAL pi2;
                angmode =
                        rplTestSystemFlag(FL_ANGLEMODE1) |
                        (rplTestSystemFlag(FL_ANGLEMODE2) << 1);

                switch (angmode) {
                case ANGLEDEG:
                case ANGLEDMS:
                    decconst_90(&pi2);
                    break;
                case ANGLEGRAD:
                    decconst_100(&pi2);
                    break;
                case ANGLERAD:
                default:
                    decconst_PI_2(&pi2);
                }

                // RETURN POLAR INFINITY
                rplInfinityToRReg(1);
                rplDropData(1);
                rplNewComplexPush(&RReg[1], &pi2, angmode);

                if(!rplTestSystemFlag(FL_COMPLEXMODE)) {
                    if(!Exceptions)
                        rplError(ERR_COMPLEXRESULT);
                }
                rplCheckResultAndError(&RReg[1]);
                return;
            }

            // JUST RETURN INFINITY
            rplInfinityToRReg(1);
            rplDropData(1);
            rplNewRealPush(&RReg[1]);
            rplCheckResultAndError(&RReg[1]);
            return;
        }

        case CPLX_INF | CPLX_MALFORMED:
        {
            REAL re, im;
            BINT angmode;
            REAL pi4;

            rplReadCNumber(arg, &re, &im, &angmode);

            angmode =
                    rplTestSystemFlag(FL_ANGLEMODE1) |
                    (rplTestSystemFlag(FL_ANGLEMODE2) << 1);

            switch (angmode) {
            case ANGLEDEG:
            case ANGLEDMS:
                decconst_45(&pi4);
                break;
            case ANGLEGRAD:
                decconst_50(&pi4);
                break;
            case ANGLERAD:
            default:
                decconst_PI_4(&pi4);
            }

            if(im.flags & F_NEGATIVE) {
                // PI/4-PI = -3/4 PI
                addReal(&RReg[0], &pi4, &pi4);
                addReal(&RReg[1], &RReg[0], &pi4);
                RReg[1].flags |= F_NEGATIVE;
            }
            else {
                copyReal(&RReg[1], &pi4);
                finalize(&RReg[1]);
            }

            // RETURN POLAR INFINITY
            rplInfinityToRReg(0);
            rplDropData(1);
            rplNewComplexPush(&RReg[0], &pi4, angmode);

            if(!rplTestSystemFlag(FL_COMPLEXMODE)) {
                if(!Exceptions)
                    rplError(ERR_COMPLEXRESULT);
            }
            rplCheckResultAndError(&RReg[0]);
            return;
        }

        case CPLX_INF | CPLX_POLAR:

        {
            REAL pi;
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            switch (angmode) {
            case ANGLEDEG:
            case ANGLEDMS:
                decconst_180(&pi);
                break;
            case ANGLEGRAD:
                decconst_200(&pi);
                break;
            case ANGLERAD:
            default:
                decconst_PI(&pi);
            }

            // COMPUTE NEW DIRECTION
            if(angmode != ANGLEDMS) {

                // DIVIDE BY TWO = * 5/10
                rplBINTToRReg(0, 5);
                mulReal(&RReg[1], &im, &RReg[0]);
                RReg[1].exp--;

            }
            else {
                BINT isodddeg, isoddmin;

                isodddeg = isoddReal(&im);
                im.exp += 2;
                isoddmin = isoddReal(&im);
                im.exp -= 2;

                // DIVIDE BY TWO = * 5/10
                rplBINTToRReg(0, 5);
                mulReal(&RReg[1], &im, &RReg[0]);
                RReg[1].exp--;

                if(isodddeg) {
                    rplBINTToRReg(0, 2);
                    RReg[0].exp--;
                    subReal(&RReg[2], &RReg[1], &RReg[0]);
                    swapReal(&RReg[2], &RReg[1]);
                }
                if(isoddmin) {
                    rplBINTToRReg(0, 2);
                    RReg[0].exp -= 3;
                    subReal(&RReg[2], &RReg[1], &RReg[0]);
                    swapReal(&RReg[2], &RReg[1]);
                }

            }

            // HERE WE HAVE THE NEW DIRECTION

            rplInfinityToRReg(0);
            rplDropData(1);
            rplNewComplexPush(&RReg[0], &RReg[1], angmode);
            if(!rplTestSystemFlag(FL_COMPLEXMODE)) {
                if(!Exceptions)
                    rplError(ERR_COMPLEXRESULT);
            }
            rplCheckResultAndError(&RReg[0]);
            return;
        }

        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
        default:
        {
            if(!ISNUMBER(*arg)) {
                // ALL OTHER OBJECT TYPES DO
                // obj ^ 0.5

                RReg[0].data[0] = 5;
                RReg[0].exp = -1;
                RReg[0].flags = 0;
                RReg[0].len = 1;

                rplNewRealFromRRegPush(0);

                rplCallOvrOperator(CMD_OVR_POW);

                return;
            }
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        }
        case CPLX_ZERO:
        {
            rplDropData(1);
            rplPushData((WORDPTR) zero_bint);
            return;
        }
        case CPLX_NORMAL:
        {

            if(ISCOMPLEX(*arg)) {
                rplReadCNumberAsReal(arg, &x);
                rplReadCNumberAsImag(arg, &y);

                if(!iszeroReal(&y)) {
                    mulReal(&RReg[0], &x, &x);
                    mulReal(&RReg[1], &y, &y);
                    addReal(&RReg[7], &RReg[0], &RReg[1]);

                    hyp_sqrt(&RReg[7]); // RReg[0]=sqrt(x^2+y^2);
                    normalize(&RReg[0]);
                    addReal(&RReg[1], &RReg[0], &x);
                    subReal(&RReg[2], &RReg[0], &x);    // ONLY POW AND XROOT USE REGISTER 8, SO THIS IS SAFE

                    RReg[0].exp = -1;
                    RReg[0].len = 1;
                    RReg[0].flags = 0;
                    RReg[0].data[0] = 5;

                    mulReal(&RReg[7], &RReg[1], &RReg[0]);
                    mulReal(&RReg[8], &RReg[2], &RReg[0]);

                    // TRAP: WHEN y<<x SQRT(x^2+y^2) == SQRT(x^2) WHICH COULD LOSE HALF THE SIGNIFICANT DIGITS
                    // THEREFORE SQRT(X^2)-X OR SQRT(X^2)-X COULD BE NEGATIVE
                    if(RReg[7].flags & F_NEGATIVE)
                        rplZeroToRReg(7);
                    if(RReg[8].flags & F_NEGATIVE)
                        rplZeroToRReg(8);

                    hyp_sqrt(&RReg[7]); // THIS IS THE REAL PART OF THE RESULT

                    finalize(&RReg[0]);
                    swapReal(&RReg[9], &RReg[0]);       // SAVE THIS RESULT

                    hyp_sqrt(&RReg[8]); // THIS IS THE IMAGINARY PART

                    finalize(&RReg[0]);
                    RReg[0].flags |= y.flags & F_NEGATIVE;

                    // DONE, RETURN THE COMPLEX ROOTS
                    rplDropData(1);
                    rplNewComplexPush(&RReg[9], &RReg[0], ANGLENONE);
                    if(!rplTestSystemFlag(FL_COMPLEXMODE))
                        rplError(ERR_COMPLEXRESULT);
                    return;

                }

            }
            else {

                rplReadNumberAsReal(rplPeekData(1), &x);

                if(Exceptions)
                    return;
            }

            BINT iscplx = x.flags & F_NEGATIVE;
            if(iscplx && !rplTestSystemFlag(FL_COMPLEXMODE)) {
                rplError(ERR_ARGOUTSIDEDOMAIN);
                return;
            }

            x.flags &= ~F_NEGATIVE;

            hyp_sqrt(&x);
            finalize(&RReg[0]);

            rplDropData(1);
            if(iscplx) {
                rplZeroToRReg(1);
                rplNewComplexPush(&RReg[1], &RReg[0], ANGLENONE);
            }
            else
                rplNewRealFromRRegPush(0);
            return;

        }
        case CPLX_NORMAL | CPLX_POLAR:
        {
            REAL pi;
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            switch (angmode) {
            case ANGLEDEG:
            case ANGLEDMS:
                decconst_180(&pi);
                break;
            case ANGLEGRAD:
                decconst_200(&pi);
                break;
            case ANGLERAD:
            default:
                decconst_PI(&pi);
            }

            // COMPUTE NEW DIRECTION
            if(angmode != ANGLEDMS) {

                // DIVIDE BY TWO = * 5/10
                rplBINTToRReg(0, 5);
                RReg[0].exp--;
                mulReal(&RReg[8], &im, &RReg[0]);
            }
            else {
                BINT isodddeg, isoddmin;

                isodddeg = isoddReal(&im);
                im.exp += 2;
                isoddmin = isoddReal(&im);
                im.exp -= 2;

                // DIVIDE BY TWO
                RReg[0].data[0] = 5;
                RReg[0].len = 1;
                RReg[0].flags = 0;
                RReg[0].exp = -1;

                mulReal(&RReg[8], &im, &RReg[0]);

                if(isodddeg) {
                    RReg[0].data[0] = 2;
                    RReg[0].exp = -1;
                    subReal(&RReg[2], &RReg[8], &RReg[0]);
                    swapReal(&RReg[2], &RReg[8]);
                }
                if(isoddmin) {
                    RReg[0].data[0] = 2;
                    RReg[0].exp = -3;
                    subReal(&RReg[2], &RReg[8], &RReg[0]);
                    swapReal(&RReg[2], &RReg[8]);
                }

            }

            // HERE WE HAVE THE NEW DIRECTION IN RReg[8]

            hyp_sqrt(&re);
            finalize(&RReg[0]);

            rplDropData(1);
            rplNewComplexPush(&RReg[0], &RReg[8], angmode);
            if(!rplTestSystemFlag(FL_COMPLEXMODE)) {
                if(!Exceptions)
                    rplError(ERR_COMPLEXRESULT);
            }
            rplCheckResultAndError(&RReg[0]);
            return;
        }

        }

        // UNREACHABLE CODE
        return;
    }

    case LOG:
    {
        //@SHORT_DESC=Compute logarithm in base 10
        REAL x;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_INF | CPLX_POLAR:
        {
            rplDropData(1);
            rplInfinityToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        }
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        {
            // RETURN -INFINITY AND SET OVERFLOW
            rplInfinityToRReg(0);
            RReg[0].flags |= F_NEGATIVE;
            rplDropData(1);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        }

        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {
            // LOGARITHM OF A COMPLEX NUMBER
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // ALREADY IN POLAR FORM

                // ONLY ACCEPT THE ANGLE IN RADIANS
                trig_convertangle(&im, angmode, ANGLERAD);

                REAL ln10;
                decconst_ln10(&ln10);
                divReal(&RReg[9], &RReg[0], &ln10);

                hyp_log(&re);
                normalize(&RReg[0]);

                // SINCE LN() MAPS FROM POLAR TO CARTESIAN PLANE, ALWAYS RETURN CARTESIAN COMPLEX

                WORDPTR newcmplx = rplNewComplex(&RReg[9], &RReg[0], ANGLENONE);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);
                rplCheckResultAndError(&RReg[9]);

                return;

            }

            // CONVERT TO POLAR AND COMPUTE LN()

            mulReal(&RReg[0], &re, &re);
            mulReal(&RReg[1], &im, &im);
            addReal(&RReg[2], &RReg[0], &RReg[1]);

            hyp_sqrt(&RReg[2]);
            normalize(&RReg[0]);
            hyp_log(&RReg[0]);
            finalize(&RReg[0]);

            swapReal(&RReg[8], &RReg[0]);       // SAVE log(r) IN RReg[8]

            trig_atan2(&im, &re, ANGLERAD);
            normalize(&RReg[0]);
            REAL ln10;
            decconst_ln10(&ln10);
            divReal(&RReg[9], &RReg[0], &ln10);

            WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[9], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[8]);
            rplCheckResultAndError(&RReg[0]);

            return;

        }

        rplReadNumberAsReal(rplPeekData(1), &x);

        if(Exceptions)
            return;

        if(x.flags & F_NEGATIVE) {
            if(rplTestSystemFlag(FL_COMPLEXMODE)) {
                // RETURN THE COMPLEX RESULT

                x.flags ^= F_NEGATIVE;

                hyp_log(&x);
                finalize(&RReg[0]);

                REAL pi, ln10;

                decconst_PI(&pi);
                decconst_ln10(&ln10);

                divReal(&RReg[1], &pi, &ln10);

                WORDPTR newcmplx = rplNewComplex(&RReg[0], &RReg[1], ANGLENONE);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);

                return;

            }
            rplError(ERR_ARGOUTSIDEDOMAIN);
            return;
        }

        hyp_log(&x);
        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }

    case ALOG:
    {
        //@SHORT_DESC=Compute anti-logarithm in base 10
        REAL dec;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(re.flags & F_NEGATIVE) {
                rplOverwriteData(1, (WORDPTR) zero_bint);
            }
            else {
                rplDropData(1);
                rplInfinityToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        }
        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_INF | CPLX_POLAR:
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // IN POLAR FORM, CONVERT TO CARTESIAN

                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

            }
            else {
                copyReal(&RReg[8], &re);
                copyReal(&RReg[9], &im);
            }

            // MULTIPLY BY ln(10)
            REAL ln10;
            decconst_ln10(&ln10);

            mulReal(&RReg[0], &RReg[8], &ln10);
            swapReal(&RReg[8], &RReg[0]);

            mulReal(&RReg[0], &RReg[9], &ln10);
            swapReal(&RReg[9], &RReg[0]);

            // JUST REGULAR EXP FROM NOW ON

            // e^(A+i*B)= e^A * e^(i*B)

            hyp_exp(&RReg[8]);
            normalize(&RReg[0]);

            swapReal(&RReg[8], &RReg[0]);

            if(angmode != ANGLENONE) {

                trig_convertangle(&RReg[9], ANGLERAD, angmode);

                // AND RETURN THE POLAR COMPLEX

                WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[0], angmode);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[8]);
                rplCheckResultAndError(&RReg[0]);

                return;

            }

            // AND RETURN THE POLAR COMPLEX

            WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[9], ANGLERAD);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[8]);
            rplCheckResultAndError(&RReg[9]);

            return;

        }

        rplReadNumberAsReal(arg, &dec);
        if(Exceptions)
            return;

        RReg[4].data[0] = 10;
        RReg[4].exp = 0;
        RReg[4].len = 1;
        RReg[4].flags = 0;

        hyp_pow(&RReg[4], &dec);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);      // EXP
        return;

    }

    case EXPM:
    {
        //@SHORT_DESC=Compute exp(x)-1
        REAL dec;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(re.flags & F_NEGATIVE) {
                rplOverwriteData(1, (WORDPTR) minusone_bint);
            }
            else {
                rplDropData(1);
                rplInfinityToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        }
        case CPLX_INF | CPLX_POLAR:
        {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            REAL pi2;
            switch (angmode) {
            case ANGLEDEG:
            case ANGLEDMS:
                decconst_90(&pi2);
                break;
            case ANGLEGRAD:
                decconst_100(&pi2);
                break;
            case ANGLERAD:
            default:
                decconst_PI_2(&pi2);
            }

            im.flags &= ~F_NEGATIVE;

            switch (cmpReal(&im, &pi2)) {
            case 1:    // ARG(Z)> PI/2
                rplOverwriteData(1, (WORDPTR) minusone_bint);
                return;
            case -1:
                rplDropData(1);
                rplUndInfinityToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            default:
            case 0:    // ARG(Z)== PI/2
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            }

        }

        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // IN POLAR FORM, CONVERT TO CARTESIAN

                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[8], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

            }
            else {
                copyReal(&RReg[8], &re);
                copyReal(&RReg[9], &im);
            }

            // e^(A+i*B)= e^A * e^(i*B)

            hyp_exp(&RReg[8]);
            normalize(&RReg[0]);

            swapReal(&RReg[8], &RReg[0]);

            // HERE WE HAVE A POLAR COMPLEX, NEED TO SUBTRACT ONE!
            // CONVERT TO CARTESIAN

            trig_sincos(&RReg[9], ANGLERAD);

            // RReg[6]= cos
            // RReg[7]= sin
            normalize(&RReg[6]);
            normalize(&RReg[7]);

            mulReal(&RReg[0], &RReg[6], &RReg[8]);
            mulReal(&RReg[1], &RReg[7], &RReg[8]);

            REAL one;
            decconst_One(&one);
            subReal(&RReg[2], &RReg[0], &one);

            WORDPTR newcmplx = rplNewComplex(&RReg[2], &RReg[1], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[2]);
            rplCheckResultAndError(&RReg[1]);

            return;

        }

        rplReadNumberAsReal(rplPeekData(1), &dec);
        if(Exceptions)
            return;

        hyp_expm(&dec);

        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);      // EXP
        return;

    }

    case LNP1:
    {
        //@SHORT_DESC=Compute ln(x+1)
        REAL x;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        WORDPTR arg = rplPeekData(1);
        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        // HANDLE SPECIALS
        BINT cclass = rplComplexClass(arg);
        switch (cclass) {
        case CPLX_INF:
        case CPLX_INF | CPLX_MALFORMED:
        case CPLX_INF | CPLX_POLAR:
        {
            rplDropData(1);
            rplInfinityToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        }
        case CPLX_UNDINF:
            rplDropData(1);
            rplNANToRReg(0);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        case CPLX_NAN:
            if(!ISCOMPLEX(*arg) && !ISREAL(*arg) && !ISANGLE(*arg))
                rplError(ERR_BADARGTYPE);
            else {
                rplDropData(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
            }
            return;
        case CPLX_ZERO:
        {
            rplZeroToRReg(0);
            rplDropData(1);
            rplNewRealFromRRegPush(0);
            return;
        }

        case CPLX_NORMAL:
        case CPLX_NORMAL | CPLX_POLAR:
        default:
            break;
        }

        if(ISCOMPLEX(*arg)) {
            // LOGARITHM OF A COMPLEX NUMBER
            REAL re, im;
            BINT angmode;

            rplReadCNumber(arg, &re, &im, &angmode);

            if(angmode != ANGLENONE) {
                // IN POLAR FORM, CONVERT TO CARTESIAN

                trig_sincos(&im, angmode);

                // RReg[6]= cos
                // RReg[7]= sin
                normalize(&RReg[6]);
                normalize(&RReg[7]);

                mulReal(&RReg[4], &RReg[6], &re);
                mulReal(&RReg[9], &RReg[7], &re);

            }
            else {
                copyReal(&RReg[4], &re);
                copyReal(&RReg[9], &im);
            }

            REAL one;
            decconst_One(&one);
            addReal(&RReg[8], &RReg[4], &one);  // (RE+1)+i*IM

            // CONVERT TO POLAR AND COMPUTE LN()

            mulReal(&RReg[0], &RReg[8], &RReg[8]);
            mulReal(&RReg[1], &RReg[9], &RReg[9]);
            addReal(&RReg[2], &RReg[0], &RReg[1]);
            if(iszeroReal(&RReg[2])) {
                // LN(0)
                // RETURN -INFINITY AND SET OVERFLOW
                rplInfinityToRReg(0);
                RReg[0].flags |= F_NEGATIVE;
                rplDropData(1);
                rplNewRealFromRRegPush(0);
                rplCheckResultAndError(&RReg[0]);
                return;
            }
            hyp_sqrt(&RReg[2]);
            normalize(&RReg[0]);
            hyp_ln(&RReg[0]);
            finalize(&RReg[0]);

            swapReal(&RReg[8], &RReg[0]);       // SAVE ln(r) IN RReg[8]

            trig_atan2(&RReg[9], &RReg[0], ANGLERAD);
            finalize(&RReg[0]);

            WORDPTR newcmplx = rplNewComplex(&RReg[8], &RReg[0], ANGLENONE);
            if((!newcmplx) || Exceptions)
                return;

            rplOverwriteData(1, newcmplx);

            rplCheckResultAndError(&RReg[8]);
            rplCheckResultAndError(&RReg[0]);

            return;

        }

        rplReadNumberAsReal(rplPeekData(1), &x);

        if(Exceptions)
            return;

        REAL one;
        decconst_One(&one);
        addReal(&RReg[0], &x, &one);

        if(iszeroReal(&RReg[0])) {
            // RETURN -INFINITY AND SET OVERFLOW
            rplInfinityToRReg(0);
            RReg[0].flags |= F_NEGATIVE;
            rplDropData(1);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        }
        if(RReg[0].flags & F_NEGATIVE) {
            if(rplTestSystemFlag(FL_COMPLEXMODE)) {
                // RETURN THE COMPLEX RESULT

                RReg[0].flags ^= F_NEGATIVE;

                hyp_ln(&RReg[0]);
                finalize(&RReg[0]);

                REAL pi;

                decconst_PI(&pi);

                copyReal(&RReg[1], &pi);

                finalize(&RReg[1]);

                WORDPTR newcmplx = rplNewComplex(&RReg[0], &RReg[1], ANGLENONE);
                if((!newcmplx) || Exceptions)
                    return;

                rplOverwriteData(1, newcmplx);

                rplCheckResultAndError(&RReg[0]);

                return;

            }
            rplError(ERR_ARGOUTSIDEDOMAIN);
            return;
        }

        hyp_lnp1(&x);
        finalize(&RReg[0]);

        rplDropData(1);
        rplNewRealFromRRegPush(0);
        return;

    }

        // ADD MORE OPCODES HERE

        // STANDARIZED OPCODES:
        // --------------------
        // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES

    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors

        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER, (char **)LIB_NAMES, NULL,
                LIB_NUMBEROFCMDS);

        return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds((char **)LIB_NAMES, NULL, LIB_NUMBEROFCMDS);
        return;
    case OPCODE_VALIDATE:
        // VALIDATE RECEIVES OPCODES COMPILED BY OTHER LIBRARIES, TO BE INCLUDED WITHIN A COMPOSITE OWNED BY
        // THIS LIBRARY. EVERY COMPOSITE HAS TO EVALUATE IF THE OBJECT BEING COMPILED IS ALLOWED INSIDE THIS
        // COMPOSITE OR NOT. FOR EXAMPLE, A REAL MATRIX SHOULD ONLY ALLOW REAL NUMBERS INSIDE, ANY OTHER
        // OPCODES SHOULD BE REJECTED AND AN ERROR THROWN.
        // Library receives:
        // CurrentConstruct = SET TO THE CURRENT ACTIVE CONSTRUCT TYPE
        // LastCompiledObject = POINTER TO THE LAST OBJECT THAT WAS COMPILED, THAT NEEDS TO BE VERIFIED

        // VALIDATE RETURNS:
        // RetNum =  OK_CONTINUE IF THE OBJECT IS ACCEPTED, ERR_INVALID IF NOT.

        RetNum = OK_CONTINUE;
        return;

    case OPCODE_PROBETOKEN:
        // PROBETOKEN FINDS A VALID WORD AT THE BEGINNING OF THE GIVEN TOKEN AND RETURNS
        // INFORMATION ABOUT IT. THIS OPCODE IS MANDATORY

        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND
    {
        libProbeCmds((char **)LIB_NAMES, (BINT *) LIB_TOKENINFO,
                LIB_NUMBEROFCMDS);

        return;
    }

    case OPCODE_GETINFO:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL COMMAND OR OBJECT IN ObjectPTR
        // NEEDS TO RETURN INFORMATION ABOUT THE TYPE:
        // IN RetNum: RETURN THE MKTOKENINFO() DATA FOR THE SYMBOLIC COMPILER AND CAS
        // IN DecompHints: RETURN SOME HINTS FOR THE DECOMPILER TO DO CODE BEAUTIFICATION (TO BE DETERMINED)
        // IN TypeInfo: RETURN TYPE INFORMATION FOR THE TYPE COMMAND
        //             TypeInfo: TTTTFF WHERE TTTT = MAIN TYPE * 100 (NORMALLY THE MAIN LIBRARY NUMBER)
        //                                FF = 2 DECIMAL DIGITS FOR THE SUBTYPE OR FLAGS (VARIES DEPENDING ON LIBRARY)
        //             THE TYPE COMMAND WILL RETURN A REAL NUMBER TypeInfo/100
        // FOR NUMBERS: TYPE=10 (REALS), SUBTYPES = .01 = APPROX., .02 = INTEGER, .03 = APPROX. INTEGER
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL BINT, .42 = HEX INTEGER

        if(ISPROLOG(*ObjectPTR)) {
            TypeInfo = LIBRARY_NUMBER * 100;
            DecompHints = 0;
            RetNum = OK_TOKENINFO | MKTOKENINFO(0, TITYPE_NOTALLOWED, 0, 1);
        }
        else {
            TypeInfo = 0;       // ALL COMMANDS ARE TYPE 0
            DecompHints = 0;
            libGetInfo2(*ObjectPTR, (char **)LIB_NAMES, (BINT *) LIB_TOKENINFO,
                    LIB_NUMBEROFCMDS);
        }
        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, ObjectIDHash=hash, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        RetNum = ERR_NOTMINE;
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID, ObjectIDHash=hash
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        RetNum = ERR_NOTMINE;
        return;
    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID

        if(ISPROLOG(*ObjectPTR)) {
            RetNum = ERR_INVALID;
            return;
        }

        RetNum = OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER, (char **)LIB_NAMES,
                LIB_NUMBEROFCMDS);
        return;

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if((MENUNUMBER(MenuCodeArg) <= 3))
            ObjectPTR = ROMPTR_TABLE[MENUNUMBER(MenuCodeArg) + 2];
        else
            ObjectPTR = (WORDPTR) empty_list;

        RetNum = OK_CONTINUE;
        return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp, (WORDPTR) LIB_HELPTABLE);
        return;
    }
    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {

        libFindMsg(LibError, (WORDPTR) LIB_MSGTABLE);
        return;
    }

    case OPCODE_LIBINSTALL:
        LibraryList = (WORDPTR) libnumberlist;
        RetNum = OK_CONTINUE;
        return;
    case OPCODE_LIBREMOVE:
        return;

    }
    // UNHANDLED OPCODE...

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode) >= MIN_RESERVED_OPCODE) {
        RetNum = ERR_NOTMINE;
        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    rplError(ERR_INVALIDOPCODE);

    return;

}

#endif
