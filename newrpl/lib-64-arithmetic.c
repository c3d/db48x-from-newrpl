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
#define LIBRARY_NUMBER  64

//@TITLE=Integer arithmetic and polynomials

#define ERROR_LIST \
        ERR(VECTOROFNUMBERSEXPECTED,0), \
        ERR(IABCUV_NO_SOLUTION,1), \
        ERR(POSITIVEINTEGEREXPECTED,2)

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(SETPREC,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(GETPREC,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(FLOOR,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(CEIL,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(IP,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(FP,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(MODSTO,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(MODRCL,MKTOKENINFO(6,TITYPE_FUNCTION,0,2)), \
    CMD(POWMOD,MKTOKENINFO(6,TITYPE_FUNCTION,2,2)), \
    CMD(MOD,MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    CMD(SQ,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(NEXTPRIME,MKTOKENINFO(9,TITYPE_FUNCTION,1,2)), \
    ECMD(FACTORIAL,"!",MKTOKENINFO(1,TITYPE_POSTFIXOP,1,3)), \
    ECMD(ISPRIME,"ISPRIME?",MKTOKENINFO(8,TITYPE_FUNCTION,1,2)), \
    CMD(MANT,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(XPON,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(SIGN,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    ECMD(PERCENT,"%",MKTOKENINFO(1,TITYPE_FUNCTION,2,2)), \
    ECMD(PERCENTCH,"%CH",MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    ECMD(PERCENTTOT,"%T",MKTOKENINFO(2,TITYPE_FUNCTION,2,2)), \
    CMD(GCD,MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    CMD(LCM,MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    CMD(IDIV2,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(IQUOT,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(ADDTMOD,MKTOKENINFO(7,TITYPE_FUNCTION,2,2)), \
    CMD(SUBTMOD,MKTOKENINFO(7,TITYPE_FUNCTION,2,2)), \
    CMD(MULTMOD,MKTOKENINFO(7,TITYPE_FUNCTION,2,2)), \
    CMD(PEVAL,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(PCOEF,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(IEGCD,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(IABCUV,MKTOKENINFO(6,TITYPE_FUNCTION,2,2)), \
    CMD(PTCHEBYCHEFF,MKTOKENINFO(11,TITYPE_FUNCTION,1,2)), \
    CMD(PLEGENDRE,MKTOKENINFO(8,TITYPE_FUNCTION,1,2)), \
    CMD(PHERMITE,MKTOKENINFO(7,TITYPE_FUNCTION,1,2)), \
    CMD(PTCHEBYCHEFF2,MKTOKENINFO(12,TITYPE_FUNCTION,1,2)), \
    CMD(PHERMITE2,MKTOKENINFO(8,TITYPE_FUNCTION,1,2)), \
    CMD(DIV2,MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    CMD(PDIV2,MKTOKENINFO(5,TITYPE_FUNCTION,2,2)), \
    CMD(PDER,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(PINT,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(PMUL,MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    CMD(PADD,MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    CMD(PSUB,MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    ECMD(IPPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    CMD(MIN,MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    CMD(MAX,MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    CMD(RND,MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    CMD(TRNC,MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    CMD(DIGITS,MKTOKENINFO(6,TITYPE_FUNCTION,3,2)), \
    CMD(PROOT,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(PREVPRIME,MKTOKENINFO(9,TITYPE_FUNCTION,1,2)), \
    CMD(FACTORS,MKTOKENINFO(7,TITYPE_FUNCTION,1,2))

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
INCLUDE_ROMOBJECT(lib64_menu_0_main);
INCLUDE_ROMOBJECT(lib64_menu_1_real);
INCLUDE_ROMOBJECT(lib64_menu_2_integer);
INCLUDE_ROMOBJECT(lib64_menu_3_module);
INCLUDE_ROMOBJECT(lib64_menu_4_polynomial);
INCLUDE_ROMOBJECT(lib64_menu_5_poly_fcn);

// INTERNAL RPL CODE FOR IP ON SYMBOLICS

ROMOBJECT ipsymbolic_seco[] = {
    MKPROLOG(DOCOL, 3),
    (CMD_OVR_NUM),      // DO ->NUM ON THE NUMERIC SYMBOLIC
    MKOPCODE(LIBRARY_NUMBER, IPPOST),   // POST-PROCESS RESULTS AND CLOSE THE LOOP
    CMD_SEMI
};

ROMOBJECT fpsymbolic_seco[] = {
    MKPROLOG(DOCOL, 6),
    (CMD_DUP),
    (CMD_OVR_NUM),      // DO ->NUM ON THE NUMERIC SYMBOLIC
    MKOPCODE(LIBRARY_NUMBER, IPPOST),   // POST-PROCESS RESULTS AND CLOSE THE LOOP
    (CMD_OVR_SUB),
    (CMD_AUTOSIMPLIFY), // SIMPLIFY BEFORE RETURN
    CMD_SEMI
};

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const const word_p ROMPTR_TABLE[] = { (word_p) LIB_MSGTABLE,
                                      (word_p) LIB_HELPTABLE,
                                      (word_p) lib64_menu_0_main,
                                      (word_p) lib64_menu_1_real,
                                      (word_p) lib64_menu_2_integer,
                                      (word_p) lib64_menu_3_module,
                                      (word_p) lib64_menu_4_polynomial,
                                      (word_p) lib64_menu_5_poly_fcn,
                                      (word_p) ipsymbolic_seco,
                                      (word_p) fpsymbolic_seco,
                                      0 };

const const char MOD_NAME[]     = "MOD";
const const char *MOD_END       = MOD_NAME + sizeof(MOD_NAME) - 1;

// COUNT THE NUMBER OF BITS IN A POSITIVE INTEGER
int rpl_log2(int64_t number, int bits)
{
    static const unsigned char log2_table[16] =
            { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };
    if(bits <= 4)
        return log2_table[number];
    bits >>= 1;
    if(number >> bits)
        return rpl_log2(number >> bits, bits) + bits;
    return rpl_log2(number, bits);
}

inline __attribute__((always_inline))
     int32_t Add_int32_t_is_Safe(int64_t op1, int64_t op2)
{
    int64_t maxop2;
    int64_t minop2;

    if(op1 > 0) {
        maxop2 = MAX_BINT - op1;
        minop2 = MIN_BINT;
    }
    else {
        maxop2 = MAX_BINT;
        minop2 = MIN_BINT - op1;
    }

    if((op2 > maxop2) || (op2 < minop2)) {
        return 0;
    }
    else {
        return 1;
    }

}

inline __attribute__((always_inline))
     int32_t Sub_int32_t_is_Safe(int64_t op1, int64_t op2)
{
    int64_t maxop2;
    int64_t minop2;

    if(op1 > 0) {
        maxop2 = MAX_BINT - op1;
        minop2 = MIN_BINT;
    }
    else {
        maxop2 = MAX_BINT;
        minop2 = MIN_BINT - op1;
    }

    if((-op2 > maxop2) || (-op2 < minop2)) {
        return 0;
    }
    else {
        return 1;
    }

}

inline __attribute__((always_inline))
     int32_t Mul_int32_t_is_Safe(int64_t op1, int64_t op2)
{
    if(op1 < 0)
        op1 = -op1;
    if(op2 < 0)
        op2 = -op2;
    if(op2 > op1) {
        int64_t tmp = op2;
        op2 = op1;
        op1 = tmp;
    }

    if(!(op2 >> 32)) {
        if(rpl_log2(op1, 64) + rpl_log2(op2, 32) < 63) {
            return 1;
        }
    }
    return 0;
}

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
            if(ISPROLOG(*rplPeekData(1))) {
                rplError(ERR_UNRECOGNIZEDOBJECT);
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
    case SETPREC:
    {
        //@SHORT_DESC=Set the current system precision
        //@NEW
        // TAKE AN INTEGER NUMBER FROM THE STACK
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        int64_t number = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;
        if(number < 4)
            number = 4;
        if(number > MAX_USERPRECISION)
            number = MAX_USERPRECISION;
        setPrecision(number);
        rplDropData(1);
        return;
    }

    case GETPREC:
    {
        //@SHORT_DESC=Get the current system precision
        //@NEW
        rplNewBINTPush(getPrecision(), DECBINT);
        return;
    }

    case FLOOR:
    {
        //@SHORT_DESC=Largest integer less than the input
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        word_p arg = rplPeekData(1);
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISSYMBOLIC(*arg)||ISIDENT(*arg)||ISCONSTANT(*arg)) {
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        if(ISUNIT(*arg)) {
            rplUnitUnaryDoCmd();
            return;
        }

        REAL rnum;
        if(ISint32_t(*arg)) {
            return;
        }
        rplReadNumberAsReal(arg, &rnum);
        if(Exceptions) {
            return;
        }
        if(isintegerReal(&rnum)) {

            return;
        }
        ipReal(&RReg[2], &rnum, 1);
        if((rnum.flags & F_NEGATIVE)) {
            RReg[2].data[0]++;
            RReg[2].flags |= F_NEGATIVE;
            normalize(&RReg[2]);
        }

        rplDropData(1);
        rplNewRealFromRRegPush(2);
        return;
    }

    case CEIL:
    {
        //@SHORT_DESC=Smallest integer larger than the input
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        word_p arg = rplPeekData(1);

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISSYMBOLIC(*arg)||ISIDENT(*arg)||ISCONSTANT(*arg)) {
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }
        if(ISUNIT(*arg)) {
            rplUnitUnaryDoCmd();
            return;
        }

        REAL rnum;
        if(ISint32_t(*arg)) {
            return;
        }
        rplReadNumberAsReal(arg, &rnum);
        if(Exceptions) {
            return;
        }
        if(isintegerReal(&rnum)) {

            return;
        }
        ipReal(&RReg[2], &rnum, 1);
        if(!(rnum.flags & F_NEGATIVE)) {
            RReg[2].data[0]++;

            normalize(&RReg[2]);
        }

        rplDropData(1);
        rplNewRealFromRRegPush(2);
        return;
    }

    case IP:
    {
        //@SHORT_DESC=Integer part of a number
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);
        word_p arg = rplPeekData(1);

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISint32_t(*arg))
            return;

        if(ISSYMBOLIC(*arg)||ISIDENT(*arg)||ISCONSTANT(*arg)) {
            // CHECK IF THE EXPRESSION IS NUMERIC
            if(rplSymbIsNumeric(arg)) {
                // COMPUTE THE EXPRESSION AND TAKE THE INTEGER PART, BUT DO IT IN RPL
                rplPushRet(IPtr);
                IPtr = (word_p) ipsymbolic_seco;
                CurOpcode = (CMD_OVR_NUM);      // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO
                return;
            }
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        if(ISUNIT(*arg)) {
            rplUnitUnaryDoCmd();
            return;
        }

        REAL rnum;
        rplReadNumberAsReal(arg, &rnum);
        if(Exceptions)
            return;

        ipReal(&RReg[1], &rnum, 1);
        rplDropData(1);
        rplNewRealFromRRegPush(1);
        return;
    }

    case IPPOST:
    {
        // EXPECTS A REAL ON THE STACK
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        REAL num;

        rplReadNumberAsReal(rplPeekData(1), &num);
        if(Exceptions)
            return;

        ipReal(&RReg[0], &num, 1);
        word_p newnum;
        if(inint64_tRange(&RReg[0]))
            newnum = rplNewBINT(getint64_tReal(&RReg[0]), DECBINT);
        else
            newnum = rplNewRealFromRReg(0);

        if(Exceptions)
            return;
        rplOverwriteData(1, newnum);

        return;
    }

    case FP:
    {
        //@SHORT_DESC=Fractional part of a number
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);
        word_p arg = rplPeekData(1);

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISSYMBOLIC(*arg)||ISIDENT(*arg)||ISCONSTANT(*arg)) {
            // CHECK IF THE EXPRESSION IS NUMERIC
            if(rplSymbIsNumeric(arg)) {
                // COMPUTE THE EXPRESSION AND TAKE THE FRACTION PART, BUT DO IT IN RPL
                rplPushRet(IPtr);
                IPtr = (word_p) fpsymbolic_seco;
                CurOpcode = (CMD_OVR_NUM);      // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO
                return;
            }
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        if(ISUNIT(*arg)) {
            rplUnitUnaryDoCmd();
            return;
        }

        if(ISint32_t(*arg)) {
            rplDropData(1);
            rplPushData((word_p) zero_bint);
            return;
        }

        REAL rnum;
        rplReadNumberAsReal(arg, &rnum);
        if(Exceptions)
            return;
        fracReal(&RReg[1], &rnum);
        rplDropData(1);
        rplNewRealFromRRegPush(1);
        return;
    }

    case FACTORIAL:
    {
        //@SHORT_DESC=Factorial of a number
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);
        word_p arg = rplPeekData(1);

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISUNIT(*arg)) {
            rplUnitUnaryDoCmdNonDimensional();
            return;
        }

        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            rplSymbApplyOperator(MKOPCODE(LIBRARY_NUMBER, FACTORIAL), 1);
            return;
        }

        if(!ISNUMBER(*arg)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }
        REAL rnum;
        rplReadNumberAsReal(arg, &rnum);
        if(Exceptions)
            return;
        if(!isintegerReal(&rnum)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }
        if(rnum.flags & F_NEGATIVE) {
            rplError(ERR_ARGOUTSIDEDOMAIN);
            return;
        }

        if(!inint32_tRange(&rnum)) {
            rplError(ERR_NUMBERTOOBIG);
            return;
        }

        int32_t n = (int32_t) rplReadNumberAsInt64(arg);

        int64_t result = factorialint32_t(n);
        if(Exceptions)
            return;

        rplDropData(1);
        if(result < 0)
            rplNewRealFromRRegPush(0);
        else
            rplNewBINTPush(result, DECBINT);
        return;

    }

    case ISPRIME:
    {
        //@SHORT_DESC=Return true/false (1/0) if a number is prime or not

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);
        word_p arg = rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISUNIT(*arg)) {
            rplUnitUnaryDoCmdNonDimensional();
            return;
        }

        if(!ISNUMBER(*arg)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        if(ISint32_t(*arg)) {
            int64_t n = rplReadint32_t(arg);

            if(isprimeint32_t(n))
                rplOverwriteData(1, (word_p) one_bint);
            else
                rplOverwriteData(1, (word_p) zero_bint);

        }
        else {

            REAL num;
            rplReadNumberAsReal(arg, &num);

            if(!isintegerReal(&num)) {
                rplError(ERR_INTEGEREXPECTED);
                return;
            }

            if(isprimeReal(&num))
                rplOverwriteData(1, (word_p) one_bint);
            else
                rplOverwriteData(1, (word_p) zero_bint);

        }
        return;
    }

    case NEXTPRIME:
    {
        //@SHORT_DESC=Smallest prime number larger than the input

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);
        word_p arg = rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISUNIT(*arg)) {
            rplUnitUnaryDoCmdNonDimensional();
            return;
        }

        if(!ISNUMBER(*arg)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        if(ISint32_t(*arg)) {
            int64_t n = rplReadint32_t(arg);

            int64_t next = nextprimeint32_t(n);
            if(next > 0) {
                rplNewBINTPush(next, DECBINT);
                if(Exceptions)
                    return;
                word_p ptr = rplPopData();
                rplOverwriteData(1, ptr);
                return;
            }
            // THE NEXT PRIME IS > 2^63, USE REALS INSTEAD

        }

        REAL num;
        rplReadNumberAsReal(arg, &num);

        if(!isintegerReal(&num)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        nextprimeReal(0, &num);
        rplDropData(1);
        rplNewRealFromRRegPush(0);

        return;
    }

    case PREVPRIME:
    {
        //@SHORT_DESC=Largest prime smaller than the input
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);
        word_p arg = rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*arg)) {

            rplListUnaryDoCmd();
            return;
        }

        if(ISUNIT(*arg)) {
            rplUnitUnaryDoCmdNonDimensional();
            return;
        }

        if(!ISNUMBER(*arg)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        if(ISint32_t(*arg)) {
            int64_t n = rplReadint32_t(arg);
            int64_t next, prev;
            int32_t previsprime;
            prev = n - 30;      // ARBITRARILY SCAN 1000 NUMBERS TO THE LEFT
            if(prev < 0)
                prev = 0;
            previsprime = 0;
            do {
                next = nextprimeint32_t(prev);
                if((next > n) && previsprime) {
                    rplNewBINTPush(prev, DECBINT);
                    if(Exceptions)
                        return;
                    word_p ptr = rplPopData();
                    rplOverwriteData(1, ptr);
                    return;
                }
                if(next > n) {
                    // NO PRIMES WITHIN 1000 NUMBERS, SUBTRACT OTHER 1000 AND GO
                    prev -= 30;
                    if(prev < 0)
                        prev = 0;
                }
                if(next > 0) {
                    // FOUND A PRIME BETWEEN prev AND n, USE IT FOR NEXT ITERATION
                    prev = next;
                    previsprime = 1;
                }

            }
            while(next > 0);
            // THE NEXT PRIME IS > 2^63, USE REALS INSTEAD

        }

        REAL num;

        rplReadNumberAsReal(arg, &num);

        if(!isintegerReal(&num)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        int32_t previsprime = 0;
        // USE RReg[5]=next
        // RReg[6]=prev
        rplint32_tToRReg(7, -30);
        addReal(&RReg[6], &num, &RReg[7]);
        if(RReg[6].flags & F_NEGATIVE)
            rplZeroToRReg(6);
        do {
            nextprimeReal(5, &RReg[6]);
            int32_t islarger = gtReal(&RReg[5], &num);
            if(islarger) {
                if(previsprime) {
                    rplDropData(1);
                    rplNewRealFromRRegPush(6);
                    return;
                }
                // NO PRIMES IN THIS GROUP, SUBTRACT OTHER 1000 AND REDO
                addReal(&RReg[0], &RReg[6], &RReg[7]);
                if(RReg[0].flags & F_NEGATIVE)
                    rplZeroToRReg(0);
                swapReal(&RReg[0], &RReg[6]);
            }
            else {
                // A PRIME WAS FOUND IN BETWEEN, USE IT FOR NEXT ITERATION
                swapReal(&RReg[5], &RReg[6]);   // prev=next;
                previsprime = 1;
            }

        }
        while(1);

        return;

    }
    case MODSTO:
    {
        //@SHORT_DESC=Set the current system modulo for all MOD operations
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        word_p arg = rplPeekData(1);

        if(!ISNUMBER(*arg)) {
            rplError(ERR_BADARGTYPE);
            return;
        }
        else {
            if(ISint32_t(*arg)) {
                int64_t m = rplReadint32_t(arg);
                if(m < 0)
                    m = -m;
                if(m < 2)
                    m = 2;
                word_p setting = rplNewBINT(m, DECBINT);
                rplStoreSettingsbyName(MOD_NAME, MOD_END, setting);
                rplDropData(1);

                return;
            }
            else {
                REAL num;

                rplReadNumberAsReal(arg, &num);

                if(!isintegerReal(&num)) {
                    rplError(ERR_INTEGEREXPECTED);
                    return;
                }
                if(num.flags & F_NEGATIVE)
                    num.flags ^= F_NEGATIVE;
                newRealFromint32_t(&RReg[0], 2, 0);
                if(ltReal(&num, &RReg[0])) {
                    newRealFromint32_t(&num, 2, 0);
                }
                word_p setting = rplNewReal(&num);
                rplStoreSettingsbyName(MOD_NAME, MOD_END, setting);
                rplDropData(1);
                return;
            }

        }

        return;
    }

    case MODRCL:
    {
        //@SHORT_DESC=Get the current system modulo
        word_p mod = rplGetSettingsbyName(MOD_NAME, MOD_END);
        if(!mod)
            mod = (word_p) zero_bint;
        if(!ISNUMBER(*mod)) {
            rplError(ERR_BADARGTYPE);
            return;
        }
        if(ISint32_t(*mod)) {
            int64_t m = rplReadint32_t(mod);
            rplNewBINTPush(m, DECBINT);
            return;
        }
        REAL m;
        rplReadNumberAsReal(mod, &m);
        rplNewRealPush(&m);
        return;
    }

    case POWMOD:
        //@SHORT_DESC=Power operator MOD the current system modulo
    case ADDTMOD:
        //@SHORT_DESC=Addition operator MOD the current system modulo
    case SUBTMOD:
        //@SHORT_DESC=Subtraction operator MOD the current system modulo
    case MULTMOD:
    {
        //@SHORT_DESC=Multiplication operator MOD the current system modulo

        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);
        word_p arg1 = rplPeekData(2);
        word_p arg2 = rplPeekData(1);

        if(ISLIST(*arg1) || ISLIST(*arg2)) {
            rplListBinaryDoCmd();
            return;
        }

        if(!ISNUMBER(*arg1)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        word_p mod = rplGetSettingsbyName(MOD_NAME, MOD_END);
        if(!mod)
            mod = (word_p) two_bint;
        if(!ISNUMBER(*arg2) || !ISNUMBER(*mod)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        if(ISint32_t(*arg1) && ISint32_t(*arg2) && ISint32_t(*mod)) {

            int64_t a1 = rplReadint32_t(arg1);
            int64_t a2 = rplReadint32_t(arg2);
            int64_t m = rplReadint32_t(mod);
            int64_t r;

            if(m < 2147483648LL) {
                int32_t isOK = 1;
                if(OPCODE(CurOpcode) == POWMOD) {
                    r = powmodint32_t(a1, a2, m);
                }
                else if((OPCODE(CurOpcode) == ADDTMOD)
                        && Add_int32_t_is_Safe(a1, a2)) {
                    r = (a1 + a2) % m;
                }
                else if((OPCODE(CurOpcode) == SUBTMOD)
                        && Sub_int32_t_is_Safe(a1, a2)) {
                    r = (a1 - a2) % m;
                }
                else if((OPCODE(CurOpcode) == MULTMOD)
                        && Mul_int32_t_is_Safe(a1, a2)) {
                    r = (a1 * a2) % m;
                }
                else {
                    isOK = 0;
                }

                if(isOK) {
                    if(r < 0) {
                        r += m;
                    }

                    rplDropData(2);

                    rplNewBINTPush(r, DECBINT);
                    return;
                }
            }
        }

        // FALL THROUGH
        // THERE'S REALS INVOLVED, DO IT ALL WITH REALS

        REAL a1, a2, m;
        rplReadNumberAsReal(arg1, &a1);
        rplReadNumberAsReal(arg2, &a2);
        rplReadNumberAsReal(mod, &m);

        if(!isintegerReal(&a1)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        if(!isintegerReal(&a2)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        if(!isintegerReal(&m)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        int32_t saveprec = Context.precdigits;
        int32_t moddigits = (intdigitsReal(&m) + 7) & ~7;
        int32_t numdigits = (intdigitsReal(&a1) + 7) & ~7;
        int32_t expdigits = (intdigitsReal(&a2) + 7) & ~7;

        moddigits *= 2;
        moddigits = (moddigits > numdigits) ? moddigits : numdigits;
        moddigits = (moddigits > expdigits) ? moddigits : expdigits;
        moddigits =
                (moddigits >
                Context.precdigits) ? moddigits : Context.precdigits;

        if(moddigits > MAX_USERPRECISION) {
            rplError(ERR_NUMBERTOOBIG);
            return;
        }

        //   AUTOMATICALLY INCREASE PRECISION TEMPORARILY

        Context.precdigits = moddigits;

        if(OPCODE(CurOpcode) == POWMOD) {
            powmodReal(&RReg[7], &a1, &a2, &m);
        }
        else if(OPCODE(CurOpcode) == ADDTMOD) {
            addReal(&RReg[0], &a1, &a2);
            divmodReal(&RReg[6], &RReg[7], &RReg[0], &m);
        }
        else if(OPCODE(CurOpcode) == SUBTMOD) {
            subReal(&RReg[0], &a1, &a2);
            divmodReal(&RReg[6], &RReg[7], &RReg[0], &m);
        }
        else if(OPCODE(CurOpcode) == MULTMOD) {
            mulReal(&RReg[0], &a1, &a2);
            divmodReal(&RReg[6], &RReg[7], &RReg[0], &m);
        }
        else {
            rplError(ERR_INVALID);
            return;
        }
        if(RReg[7].flags & F_NEGATIVE) {
            addReal(&RReg[7], &RReg[7], &m);
        }

        Context.precdigits = saveprec;

        rplDropData(2);

        rplNewRealFromRRegPush(7);
        rplCheckResultAndError(&RReg[7]);

        return;

    }

    case MOD:
        //@SHORT_DESC=Remainder of the integer division
    case IDIV2:
        //@SHORT_DESC=Integer division, get quoteiant and remainder
    case IQUOT:
        //@SHORT_DESC=Quotient of the integer division
    {

        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        word_p arg = rplPeekData(2);
        word_p mod = rplPeekData(1);
        int32_t isunit = 0;

        if(ISLIST(*arg) || ISLIST(*mod)) {
            rplListBinaryDoCmd();
            return;
        }
        else if((ISIDENT(*mod) || ISSYMBOLIC(*mod) || ISCONSTANT(*mod))
                || (ISIDENT(*arg) || ISSYMBOLIC(*arg) || ISCONSTANT(*mod))) {
            rplSymbApplyOperator(CurOpcode, 2);
            return;
        }

        // WORK WITH VALUES IF ANY ARGUMENT IS A UNIT
        if(ISUNIT(*arg) || ISUNIT(*mod)) {
            if(ISUNIT(*arg))
                ++arg;
            if(ISUNIT(*mod))
                ++mod;
            isunit = 1;
        }

        if(!ISNUMBER(*arg) || !ISNUMBER(*mod)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        if(ISint32_t(*arg) && ISint32_t(*mod)) {

            int64_t a = rplReadint32_t(arg);
            int64_t m = rplReadint32_t(mod);
            int64_t r, q;

            if(m == (int64_t) 0) {
                rplError(ERR_MATHDIVIDEBYZERO);
                return;
            }
            if(m < (1LL << 62)) {
                r = (a % m + m) % m;
                q = (a - r) / m;
                if(!isunit) {
                    rplDropData(2);
                    if(OPCODE(CurOpcode) == IDIV2 || OPCODE(CurOpcode) == IQUOT) {
                        rplNewBINTPush(q, DECBINT);
                    }
                    if(OPCODE(CurOpcode) == IDIV2 || OPCODE(CurOpcode) == MOD) {
                        rplNewBINTPush(r, DECBINT);
                    }
                    return;
                }
                else {
                    // APPLY UNITS TO THE NEW RESULTS
                    word_p newvalue;
                    word_p *stksave = DSTop;
                    switch (OPCODE(CurOpcode)) {
                    default:
                    case IDIV2:
                        newvalue = rplNewBINT(q, DECBINT);
                        if(!newvalue)
                            return;
                        newvalue = rplUnitApply(newvalue, rplPeekData(2));      // APPLY THE UNITS OF THE ARGUMENT
                        if(!newvalue)
                            return;
                        rplPushDataNoGrow(newvalue);
                        newvalue = rplUnitApply((word_p) one_bint, rplPeekData(2));    // GET THE UNITS OF THE MODULO
                        if(!newvalue) {
                            DSTop = stksave;
                            return;
                        }
                        rplPushDataNoGrow(newvalue);
                        rplCallOvrOperator(CMD_OVR_DIV);        // DIVIDE THE UNIT
                        if(Exceptions) {
                            DSTop = stksave;
                            return;
                        }
                        // HERE WE HAVE ON THE STACK THE QUOTIENT WITH PROPER UNITS (UNITS OF ARG/MOD)

                        newvalue = rplNewBINT(r, DECBINT);
                        if(!newvalue)
                            return;
                        newvalue = rplUnitApply(newvalue, rplPeekData(3));      // APPLY THE UNITS OF THE ARGUMENT
                        if(!newvalue)
                            return;
                        // HERE WE HAVE QUOTIENT AND REMAINDER WITH PROPER UNITS ON BOTH
                        rplOverwriteData(2, newvalue);
                        rplOverwriteData(2, rplPopData());
                        return;
                    case IQUOT:
                        newvalue = rplNewBINT(q, DECBINT);
                        if(!newvalue)
                            return;
                        newvalue = rplUnitApply(newvalue, rplPeekData(2));      // APPLY THE UNITS OF THE ARGUMENT
                        if(!newvalue)
                            return;
                        rplPushDataNoGrow(newvalue);
                        newvalue = rplUnitApply((word_p) one_bint, rplPeekData(2));    // GET THE UNITS OF THE MODULO
                        if(!newvalue) {
                            DSTop = stksave;
                            return;
                        }
                        rplPushDataNoGrow(newvalue);
                        rplCallOvrOperator(CMD_OVR_DIV);        // DIVIDE THE UNIT
                        if(Exceptions) {
                            DSTop = stksave;
                            return;
                        }
                        // HERE WE HAVE ON THE STACK THE QUOTIENT WITH PROPER UNITS (UNITS OF ARG/MOD)
                        rplOverwriteData(3, rplPeekData(1));
                        rplDropData(2);
                        return;
                    case MOD:
                        newvalue = rplNewBINT(r, DECBINT);
                        if(!newvalue)
                            return;
                        newvalue = rplUnitApply(newvalue, rplPeekData(2));      // APPLY THE UNITS OF THE ARGUMENT
                        if(!newvalue)
                            return;
                        // HERE WE HAVE QUOTIENT AND REMAINDER WITH PROPER UNITS ON BOTH
                        rplOverwriteData(2, newvalue);
                        rplDropData(1);
                        return;
                    }

                }
            }
        }
        // THERE'S REALS INVOLVED, DO IT ALL WITH REALS

        REAL a, m;
        rplReadNumberAsReal(arg, &a);
        rplReadNumberAsReal(mod, &m);

        if(iszeroReal(&m)) {
            rplError(ERR_MATHDIVIDEBYZERO);
            return;
        }

        int32_t saveprec = Context.precdigits;
        int32_t moddigits = (intdigitsReal(&m) + 7) & ~7;
        int32_t argdigits = (intdigitsReal(&a) + 7) & ~7;

        moddigits *= 2;
        moddigits = (moddigits > argdigits) ? moddigits : argdigits;
        moddigits =
                (moddigits >
                Context.precdigits) ? moddigits : Context.precdigits;

        if(moddigits > MAX_USERPRECISION) {
            rplError(ERR_NUMBERTOOBIG);
            return;
        }

        //   AUTOMATICALLY INCREASE PRECISION TEMPORARILY

        Context.precdigits = moddigits;

        divmodReal(&RReg[7], &RReg[6], &a, &m);
        // correct negative remainder
        if(RReg[6].flags & F_NEGATIVE) {
            addReal(&RReg[6], &RReg[6], &m);
            rplOneToRReg(0);
            subReal(&RReg[7], &RReg[7], &RReg[0]);
        }

        Context.precdigits = saveprec;

        // HERE RReg[7]=QUOTIENT, RReg[6]=REMAINDER
        if(!isunit) {
            rplDropData(2);

            if(OPCODE(CurOpcode) == IDIV2 || OPCODE(CurOpcode) == IQUOT) {
                rplNewRealFromRRegPush(7);
            }
            if(OPCODE(CurOpcode) == IDIV2 || OPCODE(CurOpcode) == MOD) {
                rplNewRealFromRRegPush(6);
            }
            rplCheckResultAndError(&RReg[6]);
            rplCheckResultAndError(&RReg[7]);

            return;
        }
        else {
            // APPLY UNITS TO THE NEW RESULTS
            // HERE RReg[7]=QUOTIENT, RReg[6]=REMAINDER

            word_p newvalue;
            word_p *stksave = DSTop;
            switch (OPCODE(CurOpcode)) {
            default:
            case IDIV2:
                newvalue = rplNewRealFromRReg(7);
                if(!newvalue)
                    return;
                newvalue = rplUnitApply(newvalue, rplPeekData(2));      // APPLY THE UNITS OF THE ARGUMENT
                if(!newvalue)
                    return;
                rplPushDataNoGrow(newvalue);
                newvalue = rplUnitApply((word_p) one_bint, rplPeekData(2));    // GET THE UNITS OF THE MODULO
                if(!newvalue) {
                    DSTop = stksave;
                    return;
                }
                rplPushDataNoGrow(newvalue);
                rplCallOvrOperator(CMD_OVR_DIV);        // DIVIDE THE UNIT
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }
                // HERE WE HAVE ON THE STACK THE QUOTIENT WITH PROPER UNITS (UNITS OF ARG/MOD)

                newvalue = rplNewRealFromRReg(6);
                if(!newvalue) {
                    DSTop = stksave;
                    return;
                }
                newvalue = rplUnitApply(newvalue, rplPeekData(3));      // APPLY THE UNITS OF THE ARGUMENT
                if(!newvalue) {
                    DSTop = stksave;
                    return;
                }
                // HERE WE HAVE QUOTIENT AND REMAINDER WITH PROPER UNITS ON BOTH
                rplOverwriteData(2, newvalue);
                rplOverwriteData(2, rplPopData());
                rplCheckResultAndError(&RReg[6]);
                rplCheckResultAndError(&RReg[7]);
                return;
            case IQUOT:
                newvalue = rplNewRealFromRReg(7);
                if(!newvalue)
                    return;
                newvalue = rplUnitApply(newvalue, rplPeekData(2));      // APPLY THE UNITS OF THE ARGUMENT
                if(!newvalue)
                    return;
                rplPushDataNoGrow(newvalue);
                newvalue = rplUnitApply((word_p) one_bint, rplPeekData(2));    // GET THE UNITS OF THE MODULO
                if(!newvalue) {
                    DSTop = stksave;
                    return;
                }
                rplPushDataNoGrow(newvalue);
                rplCallOvrOperator(CMD_OVR_DIV);        // DIVIDE THE UNIT
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }
                // HERE WE HAVE ON THE STACK THE QUOTIENT WITH PROPER UNITS (UNITS OF ARG/MOD)
                rplOverwriteData(3, rplPeekData(1));
                rplDropData(2);
                rplCheckResultAndError(&RReg[7]);
                return;
            case MOD:
                newvalue = rplNewRealFromRReg(6);
                if(!newvalue)
                    return;
                newvalue = rplUnitApply(newvalue, rplPeekData(2));      // APPLY THE UNITS OF THE ARGUMENT
                if(!newvalue)
                    return;
                // HERE WE HAVE QUOTIENT AND REMAINDER WITH PROPER UNITS ON BOTH
                rplOverwriteData(2, newvalue);
                rplDropData(1);
                rplCheckResultAndError(&RReg[6]);
                return;
            }

        }

    }

    case SQ:
    {
        //@SHORT_DESC=Square of the input
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        word_p arg = rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        // FOR ALL OTHER OBJECTS, JUST DO DUP *

        rplPushDataNoGrow(arg);

        rplCallOvrOperator((CMD_OVR_MUL));

        return;
    }

    case MANT:
    {
        //@SHORT_DESC=Mantissa of a real number (M*10<sup>exp</sup>)
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        word_p arg = rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        if(ISUNIT(*arg))
            ++arg;

        if(!ISNUMBER(*arg)) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        REAL rnum;
        int32_t digits;
        rplReadNumberAsReal(arg, &rnum);

        digits = sig_digits(rnum.data[rnum.len - 1]) + ((rnum.len - 1) << 3);

        rnum.exp = -digits + 1;
        rnum.flags &= ~F_NEGATIVE;

        rplDropData(1);
        rplNewRealPush(&rnum);
        return;
    }

    case XPON:
    {
        //@SHORT_DESC=Exponent of a number represented as (M*10<sup>exp</sup>)
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        word_p arg = rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        if(ISUNIT(*arg))
            ++arg;

        if(!ISNUMBER(*arg)) {
            rplError(ERR_REALEXPECTED);
            return;
        }
        REAL rnum;
        int32_t digits;
        rplReadNumberAsReal(arg, &rnum);

        digits = sig_digits(rnum.data[rnum.len - 1]) + ((rnum.len - 1) << 3);

        digits += rnum.exp - 1;

        rplDropData(1);
        rplNewBINTPush(digits, DECBINT);
        return;

    }

    case SIGN:
    {
        //@SHORT_DESC=Sign of a number
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStripTagStack(1);

        word_p arg = rplPeekData(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            // ARGUMENT IS SYMBOLIC, APPLY THE OPERATOR
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        if(ISUNIT(*arg)) {
            ++arg;
            // FALL THROUGH TO INTEGERS AND REALS, THIS IS DELIBERATE
        }

        if(ISint32_t(*arg)) {
            int64_t r = rplReadint32_t(arg);
            if(r > 0)
                rplOverwriteData(1, (word_p) one_bint);
            else {
                if(r < 0)
                    rplOverwriteData(1, (word_p) minusone_bint);
                else
                    rplOverwriteData(1, (word_p) zero_bint);
            }
            return;
        }

        if(ISREAL(*arg)) {
            REAL rnum;
            rplReadNumberAsReal(arg, &rnum);

            if(iszeroReal(&rnum))
                rplOverwriteData(1, (word_p) zero_bint);
            else {
                if(rnum.flags & F_NOTANUMBER)   // THIS INCLUDES BOTH NAN AND UNDIRECTED INFINITY
                {
                    rplNANToRReg(0);
                    word_p newobj = rplNewRealFromRReg(0);
                    if(!newobj)
                        return;
                    rplOverwriteData(1, newobj);
                    rplCheckResultAndError(&RReg[0]);
                    return;
                }
                if(rnum.flags & F_NEGATIVE)
                    rplOverwriteData(1, (word_p) minusone_bint);
                else
                    rplOverwriteData(1, (word_p) one_bint);
            }
            return;
        }

        if(ISCOMPLEX(*arg)) {

            // ADD COMPLEX POLAR SUPPORT
            int32_t cclass1 = rplComplexClass(arg);

            switch (cclass1) {
            case CPLX_ZERO:
                // UNDEFINED OR ZERO?
            {
                rplOverwriteData(1, (word_p) zero_bint);
                return;
            }
            case CPLX_NORMAL:
            {
                int32_t angmode;
                REAL Rarg, Iarg;

                rplReadCNumber(arg, &Rarg, &Iarg, &angmode);

                Context.precdigits += 8;
                mulReal(&RReg[2], &Rarg, &Rarg);
                mulReal(&RReg[3], &Iarg, &Iarg);
                addReal(&RReg[0], &RReg[2], &RReg[3]);

                Context.precdigits -= 8;

                hyp_sqrt(&RReg[0]);
                finalize(&RReg[0]);

                divReal(&RReg[1], &Rarg, &RReg[0]);
                divReal(&RReg[2], &Iarg, &RReg[0]);

                word_p newobj = rplNewComplex(&RReg[1], &RReg[2], ANGLENONE);
                if(!newobj)
                    return;
                rplOverwriteData(1, newobj);
                rplCheckResultAndError(&RReg[1]);
                rplCheckResultAndError(&RReg[2]);
                return;
            }

            case CPLX_INF | CPLX_POLAR:
            case CPLX_NORMAL | CPLX_POLAR:
            {
                // IT'S A NORMAL POLAR COMPLEX, NEED TO COMPUTE THE ARGUMENT BUT IN A PROPERLY REDUCED WAY
                int32_t angmode;
                REAL real, imag;

                rplReadCNumber(arg, &real, &imag, &angmode);

                if(Exceptions)
                    return;

                rplNormalizeComplex(&real, &imag, angmode);

                // RETURN AN ANGLE IN THE CURRENT SYSTEM
                rplOneToRReg(4);

                word_p newobj = rplNewComplex(&RReg[4], &imag, angmode);
                if(!newobj)
                    return;
                rplOverwriteData(1, newobj);
                rplCheckResultAndError(&RReg[4]);
                rplCheckResultAndError(&imag);
                return;
            }

            case CPLX_INF:
            case CPLX_INF | CPLX_MALFORMED:
            {
                int32_t angmode;
                word_p result;
                REAL real, imag;

                rplReadCNumber(arg, &real, &imag, &angmode);

                if(isinfiniteReal(&real)) {
                    // INFINITE IN THE REAL AXIS

                    if(!isinfiniteReal(&imag))  // CHECK IF THE IMAGINARY AXIS IS FINITE
                    {
                        if(real.flags & F_NEGATIVE)
                            result = (word_p) minusone_bint;
                        else
                            result = (word_p) one_bint;
                    }
                    else {
                        // SIGN OF UNDEFINED/MALFORMED INFINITY IS UNKNOWN
                        rplNANToRReg(0);
                        word_p newobj = rplNewRealFromRReg(0);
                        if(!newobj)
                            return;
                        rplOverwriteData(1, newobj);
                        rplCheckResultAndError(&RReg[0]);
                        return;
                    }
                    rplOverwriteData(1, result);
                    return;

                }
                // THE IMAGINARY AXIS HAS TO BE INFINTE
                rplZeroToRReg(0);
                rplOneToRReg(1);
                if(imag.flags & F_NEGATIVE)
                    RReg[1].flags |= F_NEGATIVE;

                word_p newobj = rplNewComplex(&RReg[0], &RReg[1], ANGLENONE);
                if(!newobj)
                    return;
                rplOverwriteData(1, newobj);
                return;

            }
            case CPLX_UNDINF:
            case CPLX_NAN:
            default:
            {
                rplNANToRReg(0);
                word_p newobj = rplNewRealFromRReg(0);
                if(!newobj)
                    return;
                rplOverwriteData(1, newobj);
                rplCheckResultAndError(&RReg[0]);
                return;
            }

            }

            return;

        }

        rplError(ERR_REALEXPECTED);
        return;
    }

    case PERCENT:
    {
        //@SHORT_DESC=Percentage of a number
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        word_p pct = rplPeekData(1);
        word_p arg1 = rplPeekData(2);

        if(ISUNIT(*arg1))
            ++arg1;

        if(ISLIST(*arg1) || ISLIST(*pct)) {
            rplListBinaryDoCmd();
            return;
        }
        else if((ISIDENT(*pct) || ISSYMBOLIC(*pct) || ISCONSTANT(*pct))
                || (ISIDENT(*arg1) || ISSYMBOLIC(*arg1) || ISCONSTANT(*arg1))) {
            rplSymbApplyOperator(CurOpcode, 2);
            return;
        }
        else if(ISNUMBER(*pct) && ISNUMBER(*arg1)) {
            REAL x;
            rplReadNumberAsReal(pct, &x);
            x.exp -= 2; // divide by 100
            // replace level 1 value
            word_p newnumber = rplNewReal(&x);
            if(!newnumber)
                return;
            rplOverwriteData(1, newnumber);
            rplCallOvrOperator(CMD_OVR_MUL);
            return;
        }
        else {
            rplError(ERR_BADARGTYPE);
            return;
        }
        return;
    }

    case PERCENTCH:
    {
        //@SHORT_DESC=Percentage of change on a number
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        word_p old_val = rplPeekData(2);
        word_p new_val = rplPeekData(1);

        if(ISLIST(*old_val) || ISLIST(*new_val)) {
            rplListBinaryDoCmd();
            return;
        }
        else if((ISIDENT(*new_val) || ISSYMBOLIC(*new_val)
                    || ISCONSTANT(*new_val)) || (ISIDENT(*old_val)
                    || ISSYMBOLIC(*old_val) || ISCONSTANT(*old_val))) {
            rplSymbApplyOperator(CurOpcode, 2);
            return;
        }
        else if(ISNUMBER(*new_val) && ISNUMBER(*old_val)) {

            // DO IT ALL WITH REALS
            // calculate 100*(y-x)/x
            // Same names as in the advanced user's reference manual
            REAL x, y;
            rplReadNumberAsReal(old_val, &x);
            rplReadNumberAsReal(new_val, &y);
            subReal(&RReg[1], &y, &x);  // delta
            RReg[1].exp += 2;   // multiply delta by 100
            divReal(&RReg[0], &RReg[1], &x);

            word_p newnumber = rplNewReal(&RReg[0]);
            if(!newnumber)
                return;
            // drop one value and replace level 1 value
            rplDropData(1);
            rplOverwriteData(1, newnumber);
            rplCheckResultAndError(&RReg[0]);
            return;
        }
        else if(ISUNIT(*new_val) || ISUNIT(*old_val)) {
            word_p *stksave = DSTop;

            rplPushDataNoGrow(new_val);
            rplPushDataNoGrow(old_val);
            rplCallOvrOperator(CMD_OVR_SUB);
            if(Exceptions) {
                DSTop = stksave;
                return;
            }
            rplNewBINTPush(100, DECBINT);
            if(Exceptions) {
                DSTop = stksave;
                return;
            }
            rplCallOvrOperator(CMD_OVR_MUL);
            if(Exceptions) {
                DSTop = stksave;
                return;
            }
            rplPushDataNoGrow(rplPeekData(3));
            rplCallOvrOperator(CMD_OVR_DIV);
            if(Exceptions) {
                DSTop = stksave;
                return;
            }
            rplOverwriteData(3, rplPeekData(1));
            rplDropData(2);
            return;
        }

        else {
            rplError(ERR_BADARGTYPE);
            return;
        }
        return;

    }

    case PERCENTTOT:
    {
        //@SHORT_DESC=Get percentage of a total
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        word_p old_val = rplPeekData(2);
        word_p new_val = rplPeekData(1);

        if(ISLIST(*old_val) || ISLIST(*new_val)) {
            rplListBinaryDoCmd();
            return;
        }
        else if((ISIDENT(*new_val) || ISSYMBOLIC(*new_val)
                    || ISCONSTANT(*new_val)) || (ISIDENT(*old_val)
                    || ISSYMBOLIC(*old_val) || ISCONSTANT(*old_val))) {
            rplSymbApplyOperator(CurOpcode, 2);
            return;
        }
        else if(ISNUMBER(*new_val) && ISNUMBER(*old_val)) {

            // DO IT ALL WITH REALS
            // calculate 100*y/x
            // Same names as in the advanced user's reference manual
            REAL x, y;
            rplReadNumberAsReal(old_val, &x);
            rplReadNumberAsReal(new_val, &y);
            y.exp += 2; // multiply by 100
            divReal(&RReg[0], &y, &x);

            word_p newnumber = rplNewReal(&RReg[0]);
            if(!newnumber)
                return;
            // drop one value and replace level 1 value
            rplDropData(1);
            rplOverwriteData(1, newnumber);

            return;

        }
        else if(ISUNIT(*new_val) || ISUNIT(*old_val)) {
            word_p *stksave = DSTop;

            rplPushDataNoGrow(new_val);
            rplNewBINTPush(100, DECBINT);
            if(Exceptions) {
                DSTop = stksave;
                return;
            }
            rplCallOvrOperator(CMD_OVR_MUL);
            if(Exceptions) {
                DSTop = stksave;
                return;
            }
            rplPushDataNoGrow(rplPeekData(3));
            rplCallOvrOperator(CMD_OVR_DIV);
            if(Exceptions) {
                DSTop = stksave;
                return;
            }
            rplOverwriteData(3, rplPeekData(1));
            rplDropData(2);
            return;
        }
        else {
            rplError(ERR_BADARGTYPE);
            return;
        }
        return;

    }

    case GCD:
        //@SHORT_DESC=Greatest common divisor
    case LCM:
        //@SHORT_DESC=Least common multiple
    case IEGCD:
    {
        //@SHORT_DESC=Extended euclidean algorithm

        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        word_p arg1 = rplPeekData(2);
        word_p arg2 = rplPeekData(1);
        word_p *cleanup = 0;

        if(ISLIST(*arg1) || ISLIST(*arg2)) {
            rplListBinaryDoCmd();
            return;
        }
        else if((ISIDENT(*arg1) || ISSYMBOLIC(*arg1) || ISCONSTANT(*arg1))
                || (ISIDENT(*arg2) || ISSYMBOLIC(*arg2) || ISCONSTANT(*arg2))) {
            rplSymbApplyOperator(CurOpcode, 2);
            return;
        }

        if(!ISNUMBER(*arg1) || !ISNUMBER(*arg2)) {
            int32_t nond1, nond2;
            nond1 = rplUnitIsNonDimensional(arg1);
            if(Exceptions)
                return;
            nond2 = rplUnitIsNonDimensional(rplPeekData(1 + nond1));
            if(Exceptions)
                return;
            if(nond1 + nond2 != 2) {
                rplDropData(nond1 + nond2);
                rplError(ERR_INCONSISTENTUNITS);
                return;
            }
            // FALLBACK TO THE INTEGER CASE...
            cleanup = DSTop - 2;
            arg1 = rplPeekData(2);
            arg2 = rplPeekData(1);

            if(!ISNUMBER(*arg1) || !ISNUMBER(*arg2)) {
                rplError(ERR_BADARGTYPE);
                if(cleanup)
                    DSTop = cleanup;
                return;
            }
        }

        int32_t isIEGCD = (OPCODE(CurOpcode) == IEGCD);
        int32_t chs1 = 0;
        int32_t chs2 = 0;
        int32_t swapped = 0;
        if(ISint32_t(*arg1) && ISint32_t(*arg2)) {

            int64_t a1 = rplReadint32_t(arg1);
            int64_t a2 = rplReadint32_t(arg2);
            int64_t r1, r2, r3, gcd;
            int64_t q, s1, s2, s3, t1, t2, t3, s, t;
            if(a1 < 0) {
                a1 = -a1;
                chs1 = 1;
            }
            if(a2 < 0) {
                a2 = -a2;
                chs2 = 1;
            }
            if(a1 > a2) {
                r1 = a1;
                r2 = a2;
            }
            else {
                r2 = a1;
                r1 = a2;
                swapped = 1;
            }
            if(r2 == (int64_t) 0) {
                rplError(ERR_MATHDIVIDEBYZERO);
                if(cleanup)
                    DSTop = cleanup;
                return;
            }
            // avoid swapping elements by loop unrolling
            int32_t notfinished = 1;
            if(isIEGCD) {
                s1 = 1;
                s2 = 0;
                t1 = 0;
                t2 = 1;
            }
            do {
                if(r2 != 0) {
                    r3 = r1 % r2;
                    if(isIEGCD) {
                        //remainder = (dividend%divisor + divisor)%divisor;  // also for negative numbers
                        //quotient = (dividend-remainder)/divisor;
                        q = (r1 - r3) / r2;
                        s3 = s1 - q * s2;
                        t3 = t1 - q * t2;
                    }
                }
                else {
                    gcd = r1;
                    notfinished = 0;
                    if(isIEGCD) {
                        s = s1;
                        t = t1;
                    }
                    break;
                }
                if(r3 != 0) {
                    r1 = r2 % r3;
                    if(isIEGCD) {
                        q = (r2 - r1) / r3;
                        s1 = s2 - q * s3;
                        t1 = t2 - q * t3;
                    }
                }
                else {
                    gcd = r2;
                    notfinished = 0;
                    if(isIEGCD) {
                        s = s2;
                        t = t2;
                    }
                    break;
                }
                if(r1 != 0) {
                    r2 = r3 % r1;
                    if(isIEGCD) {
                        q = (r3 - r2) / r1;
                        s2 = s3 - q * s1;
                        t2 = t3 - q * t1;
                    }
                }
                else {
                    gcd = r3;
                    notfinished = 0;
                    if(isIEGCD) {
                        s = s3;
                        t = t3;
                    }
                    break;
                }
            }
            while(notfinished);
            if(OPCODE(CurOpcode) == GCD) {
                if(cleanup)
                    DSTop = cleanup;
                rplDropData(2);
                rplNewBINTPush(gcd, DECBINT);
            }
            else if(isIEGCD) {
                if(cleanup)
                    DSTop = cleanup;
                rplDropData(2);
                rplNewBINTPush(gcd, DECBINT);
                if(!swapped) {
                    if(chs1) {
                        rplNewBINTPush(-s, DECBINT);
                    }
                    else {
                        rplNewBINTPush(s, DECBINT);
                    }
                    if(chs2) {
                        rplNewBINTPush(-t, DECBINT);
                    }
                    else {
                        rplNewBINTPush(t, DECBINT);
                    }
                }
                else {
                    if(chs1) {
                        rplNewBINTPush(-t, DECBINT);
                    }
                    else {
                        rplNewBINTPush(t, DECBINT);
                    }
                    if(chs2) {
                        rplNewBINTPush(-s, DECBINT);
                    }
                    else {
                        rplNewBINTPush(s, DECBINT);
                    }
                }
            }
            else        // LCM(a1,a2) = a1*a2/gcd(a1,a2)
            {
                REAL x, y, rgcd;
                rplLoadInt64AsReal(a1, &x);
                rplLoadInt64AsReal(a2, &y);
                rplLoadInt64AsReal(gcd, &rgcd);
                mulReal(&RReg[0], &x, &y);
                divReal(&RReg[4], &RReg[0], &rgcd);
                if((x.flags & F_APPROX) || (y.flags & F_APPROX))
                    RReg[4].flags |= F_APPROX;
                else
                    RReg[4].flags &= ~F_APPROX; // REMOVE THE APPROXIMATED FLAG AFTER TRUNCATION
                if(cleanup)
                    DSTop = cleanup;
                rplDropData(2);
                rplNewRealFromRRegPush(4);
            }
            return;
        }
        // THERE'S REALS INVOLVED, DO IT ALL WITH REALS

        rplNumberToRReg(1, arg1);
        rplNumberToRReg(2, arg2);

        if(!isintegerReal(&RReg[1])) {
            rplError(ERR_INTEGEREXPECTED);
            if(cleanup)
                DSTop = cleanup;
            return;
        }
        if(!isintegerReal(&RReg[2])) {
            rplError(ERR_INTEGEREXPECTED);
            if(cleanup)
                DSTop = cleanup;
            return;
        }

        if(RReg[1].flags & F_NEGATIVE) {
            RReg[1].flags ^= F_NEGATIVE;
            chs1 = 1;
        }
        if(RReg[2].flags & F_NEGATIVE) {
            RReg[2].flags ^= F_NEGATIVE;
            chs2 = 1;
        }
        if(gtReal(&RReg[2], &RReg[1])) {
            swapReal(&RReg[2], &RReg[1]);
            swapped = 1;
        }
        if(iszeroReal(&RReg[2])) {
            rplError(ERR_MATHDIVIDEBYZERO);
            if(cleanup)
                DSTop = cleanup;
            return;
        }

        int32_t saveprec = Context.precdigits;

        int32_t arg1digits = (intdigitsReal(&RReg[1]) + 7) & ~7;
        int32_t arg2digits = (intdigitsReal(&RReg[2]) + 7) & ~7;

        arg1digits *= 2;
        arg1digits = (arg1digits > arg2digits) ? arg1digits : arg2digits;
        arg1digits =
                (arg1digits >
                Context.precdigits) ? arg1digits : Context.precdigits;

        if(arg1digits > MAX_USERPRECISION) {
            rplError(ERR_NUMBERTOOBIG);
            if(cleanup)
                DSTop = cleanup;
            return;
        }

        //   AUTOMATICALLY INCREASE PRECISION TEMPORARILY

        Context.precdigits = arg1digits;

        // avoid swapping elements by loop unrolling
        int32_t notfinished = 1;
        const int32_t q = 0, r1 = 1, r2 = 2, r3 = 3, s1 = 4, s2 = 5, s3 = 6, t1 =
                7, t2 = 8, t3 = 9;
        int32_t igcd, s = 0, t = 0;
        REAL tmp;
        if(isIEGCD) {
            newRealFromint32_t(&RReg[s1], 1, 0);
            newRealFromint32_t(&RReg[s2], 0, 0);
            newRealFromint32_t(&RReg[t1], 0, 0);
            newRealFromint32_t(&RReg[t2], 1, 0);
            tmp.data = allocRegister();
        }
        do {
            if(!iszeroReal(&RReg[r2])) {
                divmodReal(&RReg[q], &RReg[r3], &RReg[r1], &RReg[r2]);
                if(isIEGCD) {
                    mulReal(&tmp, &RReg[q], &RReg[s2]);
                    subReal(&RReg[s3], &RReg[s1], &tmp);
                    mulReal(&tmp, &RReg[q], &RReg[t2]);
                    subReal(&RReg[t3], &RReg[t1], &tmp);
                    //s3 = s1 - q*s2;
                    //t3 = t1 - q*t2;
                }
            }
            else {
                igcd = r1;
                notfinished = 0;
                if(isIEGCD) {
                    s = s1;
                    t = t1;
                }
                break;
            }
            if(!iszeroReal(&RReg[r3])) {
                divmodReal(&RReg[q], &RReg[r1], &RReg[r2], &RReg[r3]);
                if(isIEGCD) {
                    mulReal(&tmp, &RReg[q], &RReg[s3]);
                    subReal(&RReg[s1], &RReg[s2], &tmp);
                    mulReal(&tmp, &RReg[q], &RReg[t3]);
                    subReal(&RReg[t1], &RReg[t2], &tmp);
                    //s1 = s2 - q*s3;
                    //t1 = t2 - q*t3;
                }
            }
            else {
                igcd = r2;
                notfinished = 0;
                if(isIEGCD) {
                    s = s2;
                    t = t2;
                }
                break;
            }
            if(!iszeroReal(&RReg[r1])) {
                divmodReal(&RReg[q], &RReg[r2], &RReg[r3], &RReg[r1]);
                if(isIEGCD) {
                    mulReal(&tmp, &RReg[q], &RReg[s1]);
                    subReal(&RReg[s2], &RReg[s3], &tmp);
                    mulReal(&tmp, &RReg[q], &RReg[t1]);
                    subReal(&RReg[t2], &RReg[t3], &tmp);
                    //s2 = s3 - q*s1;
                    //t2 = t3 - q*t1;
                }
            }
            else {
                igcd = r3;
                notfinished = 0;
                if(isIEGCD) {
                    s = s3;
                    t = t3;
                }
                break;
            }
        }
        while(notfinished);

        if(isIEGCD) {
            freeRegister(tmp.data);
        }

        if(OPCODE(CurOpcode) == GCD) {
            Context.precdigits = saveprec;
            if(cleanup)
                DSTop = cleanup;
            rplDropData(2);
            rplNewRealFromRRegPush(igcd);
            rplCheckResultAndError(&RReg[igcd]);
        }
        else if(isIEGCD) {
            Context.precdigits = saveprec;
            if(cleanup)
                DSTop = cleanup;
            rplDropData(2);
            rplNewRealFromRRegPush(igcd);
            rplCheckResultAndError(&RReg[igcd]);
            if(!swapped) {
                if(chs1) {
                    RReg[s].flags ^= F_NEGATIVE;
                    rplNewRealFromRRegPush(s);
                    rplCheckResultAndError(&RReg[s]);
                }
                else {
                    rplNewRealFromRRegPush(s);
                    rplCheckResultAndError(&RReg[s]);
                }
                if(chs2) {
                    RReg[t].flags ^= F_NEGATIVE;
                    rplNewRealFromRRegPush(t);
                    rplCheckResultAndError(&RReg[t]);
                }
                else {
                    rplNewRealFromRRegPush(t);
                    rplCheckResultAndError(&RReg[t]);
                }
            }
            else {
                if(chs1) {
                    RReg[t].flags ^= F_NEGATIVE;
                    rplNewRealFromRRegPush(t);
                    rplCheckResultAndError(&RReg[t]);
                }
                else {
                    rplNewRealFromRRegPush(t);
                    rplCheckResultAndError(&RReg[t]);
                }
                if(chs2) {
                    RReg[s].flags ^= F_NEGATIVE;
                    rplNewRealFromRRegPush(s);
                    rplCheckResultAndError(&RReg[s]);
                }
                else {
                    rplNewRealFromRRegPush(s);
                    rplCheckResultAndError(&RReg[s]);
                }
            }
        }
        else    // LCM(a1,a2) = a1*a2/gcd(a1,a2)
        {
            REAL x, y;
            rplReadNumberAsReal(arg1, &x);
            rplReadNumberAsReal(arg2, &y);
            mulReal(&RReg[0], &x, &y);
            divReal(&RReg[4], &RReg[0], &RReg[igcd]);
            if((x.flags & F_APPROX) || (y.flags & F_APPROX))
                RReg[4].flags |= F_APPROX;
            else
                RReg[4].flags &= ~F_APPROX;     // REMOVE THE APPROXIMATED FLAG AFTER TRUNCATION
            Context.precdigits = saveprec;
            if(cleanup)
                DSTop = cleanup;
            rplDropData(2);
            rplNewRealFromRRegPush(4);
            rplCheckResultAndError(&RReg[4]);
        }

        return;

    }

    case PEVAL:
    {
        //@SHORT_DESC=Evaluation of polynomial given as vector of coefficients
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        word_p vect_val = rplPeekData(2);
        word_p real_val = rplPeekData(1);

        if(ISLIST(*real_val)) {
            rplListBinaryDoCmd();
            return;
        }
        else if((ISIDENT(*vect_val) || ISSYMBOLIC(*vect_val)
                    || ISCONSTANT(*vect_val))) {
            rplSymbApplyOperator(CurOpcode, 2);
            return;
        }
        else if(ISMATRIX(*vect_val)) {

            int32_t rows = MATROWS(vect_val[1]), cols = MATCOLS(vect_val[1]);

            if(rows) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            word_p *savestk = DSTop;

            rplPushData(rplPeekData(2));
            if(Exceptions) {
                DSTop = savestk;
                return;
            }
            word_p *first = rplMatrixExplode();
            if(!first || Exceptions) {
                DSTop = savestk;
                return;
            }

            word_p result = rplPolyEvalEx(first, cols - 1, savestk - 1);
            if(!result || Exceptions) {
                DSTop = savestk;
                return;
            }

            if(ISSYMBOLIC(*result)) {
                result = rplSymbNumericReduce(result);
                if(!result || Exceptions) {
                    DSTop = savestk;
                    return;
                }
            }

            DSTop = savestk - 1;
            rplOverwriteData(1, result);

            return;

        }
        else {
            rplError(ERR_BADARGTYPE);
            return;
        }
        return;

    }

    case PCOEF:
    {
        //@SHORT_DESC=Coefficients of monic polynomial with the given roots
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        word_p vect_val = rplPeekData(1);

        if(ISMATRIX(*vect_val)) {

            int32_t rows = MATROWS(vect_val[1]), cols = MATCOLS(vect_val[1]);

            if(rows) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }
            int32_t f, icoef, j;

            for(f = 1; f <= cols; ++f) {
                word_p entry = rplMatrixFastGet(vect_val, 1, f);
                if(!ISNUMBER(*entry)) {
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }
            // DO IT ALL WITH REALS

            int32_t saveprec = Context.precdigits;

            int32_t argdigits = (cols + 7) & ~7;

            if(argdigits > MAX_USERPRECISION) {
                argdigits = MAX_USERPRECISION;
            }

            argdigits =
                    (argdigits >
                    Context.precdigits) ? argdigits : Context.precdigits;
            //   AUTOMATICALLY INCREASE PRECISION TEMPORARILY

            Context.precdigits = argdigits;

            word_p *Firstelem = DSTop;

            rplPushData((word_p) one_bint);
            if(Exceptions) {
                DSTop = Firstelem;
                Context.precdigits = saveprec;
                return;
            }

            for(icoef = 1; icoef <= cols; ++icoef) {
                word_p ai = rplMatrixFastGet(vect_val, 1, icoef);
                rplNumberToRReg(0, ai);
                RReg[0].flags ^= F_NEGATIVE;
                rplPushData((word_p) zero_bint);
                for(j = 1; j <= icoef; ++j) {
                    if(j == 1) {
                        rplNumberToRReg(1, *(Firstelem + j - 1));
                    }
                    else {
                        copyReal(&RReg[1], &RReg[2]);
                    }
                    rplNumberToRReg(2, *(Firstelem + j));

                    mulReal(&RReg[3], &RReg[0], &RReg[1]);
                    addReal(&RReg[4], &RReg[3], &RReg[2]);
                    word_p newnumber = rplNewReal(&RReg[4]);
                    if(!newnumber) {
                        DSTop = Firstelem;
                        Context.precdigits = saveprec;
                        return;
                    }
                    *(Firstelem + j) = newnumber;
                }
                if(Exceptions) {
                    DSTop = Firstelem;
                    Context.precdigits = saveprec;
                    return;
                }
            }

            Context.precdigits = saveprec;
            word_p pcoefs = rplMatrixCompose(0, cols + 1);
            if(!pcoefs)
                return;
            rplDropData(cols + 1);
            rplOverwriteData(1, pcoefs);

            return;

        }
        else {
            rplError(ERR_BADARGTYPE);
            return;
        }
        return;

    }

    case IABCUV:
    {
        //@SHORT_DESC=Find integers u,v to solve a*u+b*v=c
        if(rplDepthData() < 3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(3);

        word_p arg0 = rplPeekData(3);
        word_p arg1 = rplPeekData(2);
        word_p arg2 = rplPeekData(1);

//        if(ISLIST(*arg1) || ISLIST(*arg2)){
//            rplListBinaryDoCmd();
//            return;
//        }
//        else
        if((ISIDENT(*arg0) || ISSYMBOLIC(*arg0) || ISCONSTANT(*arg0))
                || (ISIDENT(*arg1) || ISSYMBOLIC(*arg1) || ISCONSTANT(*arg1))
                || (ISIDENT(*arg2) || ISSYMBOLIC(*arg2) || ISCONSTANT(*arg2))) {
            rplSymbApplyOperator(CurOpcode, 3);
            return;
        }

        if(!ISNUMBER(*arg0) || !ISNUMBER(*arg1) || !ISNUMBER(*arg2)) {
            rplError(ERR_BADARGTYPE);
            return;
        }

        word_p *savestk = DSTop;       // Drop arguments in case of error

        // Stack: A B C
        rplPushData(rplPeekData(3));
        rplPushData(rplPeekData(3));
        // Stack: A B C A B

        rplCallOperator(MKOPCODE(LIBRARY_NUMBER, IEGCD));
        if(Exceptions) {
            if(DSTop > savestk)
                DSTop = savestk;
            return;
        }

        // Stack: A B C GCD(A,B) S T
        //        6 5 4   3      2 1

        // check for Solution Condition: C MOD GCD(A,B) = 0
        word_p wp_c = rplPeekData(4);
        word_p wp_gcd_ab = rplPeekData(3);
        word_p wp_s = rplPeekData(2);
        word_p wp_t = rplPeekData(1);
        if(ISint32_t(*wp_s) && ISint32_t(*wp_t) && ISint32_t(*wp_c)
                && ISint32_t(*wp_gcd_ab)) {

            int64_t c = rplReadint32_t(wp_c);
            int64_t gcd = rplReadint32_t(wp_gcd_ab);
            int64_t r = (c % gcd + gcd) % gcd;
            if(r == 0) {
                int64_t q = (c - r) / gcd;
                int64_t s = rplReadint32_t(wp_s);
                int64_t t = rplReadint32_t(wp_t);
                s *= q;
                t *= q;
                rplDropData(6);
                rplNewBINTPush(s, DECBINT);
                rplNewBINTPush(t, DECBINT);
            }
            else {
                // ERROR: no soluton
                rplDropData(6);
                rplError(ERR_IABCUV_NO_SOLUTION);
                //DSTop=savestk;
                return;
            }

        }

        // THERE'S REALS INVOLVED, DO IT ALL WITH REALS
        rplNumberToRReg(1, wp_gcd_ab);
        rplNumberToRReg(2, wp_c);

        if(!isintegerReal(&RReg[2])) {
            if(DSTop > savestk)
                DSTop = savestk;
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        //           Q         R    =    A   /    B
        divmodReal(&RReg[4], &RReg[3], &RReg[2], &RReg[1]);
        if(iszeroReal(&RReg[3])) {
            rplNumberToRReg(6, wp_s);
            rplNumberToRReg(5, wp_t);

            mulReal(&RReg[2], &RReg[4], &RReg[6]);
            mulReal(&RReg[1], &RReg[4], &RReg[5]);

            rplDropData(6);
            rplNewRealFromRRegPush(2);
            rplNewRealFromRRegPush(1);
            rplCheckResultAndError(&RReg[2]);
            rplCheckResultAndError(&RReg[1]);
        }
        else {
            // ERROR: no soluton
            rplDropData(6);
            rplError(ERR_IABCUV_NO_SOLUTION);
            //DSTop=savestk; return;
            return;
        }
        return;
    }

    case PTCHEBYCHEFF:
        //@SHORT_DESC=Nth Tchebycheff polynomial
        //@NEW
    case PTCHEBYCHEFF2:
        //@SHORT_DESC=Nth Tchebycheff polynomial of the second kind
        //@NEW
    case PLEGENDRE:
        //@SHORT_DESC=Nth Legendre polynomial
        //@NEW
    case PHERMITE:
        //@SHORT_DESC=Nth Hermite polynomial as used by physics
        //@NEW
    case PHERMITE2:
        //@SHORT_DESC=Nth Hermite polynomial as used in probabilities
        //@NEW
    {
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        word_p arg = rplPeekData(1);

        if(ISLIST(*arg)) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISSYMBOLIC(*arg) || ISIDENT(*arg) || ISCONSTANT(*arg)) {
            rplSymbApplyOperator(CurOpcode, 1);
            return;
        }

        if(!ISNUMBER(*arg)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        REAL rnum;
        rplReadNumberAsReal(arg, &rnum);
        if(!isintegerReal(&rnum)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }
        if(rnum.flags & F_NEGATIVE) {
            rplError(ERR_POSITIVEINTEGEREXPECTED);
            //rplError(ERR_ARGOUTSIDEDOMAIN);
            return;
        }
        if(!inint32_tRange(&rnum)) {
            rplError(ERR_NUMBERTOOBIG);
            return;
        }

        int32_t n = (int32_t) rplReadNumberAsInt64(arg);
        /*
           if (n < 0) {
           rplError(ERR_POSITIVEINTEGEREXPECTED);
           return;
           }
           else */
        if(n > 65534)   // vector size limit
        {
            rplError(ERR_NUMBERTOOBIG);
            return;
        }

        rplDropData(1);

        word_p *savestk = DSTop;       // Drop arguments in case of error

        if(n == 0) {
            rplPushData((word_p) (one_bint));
            int elements = 1;
            word_p newmat = rplMatrixCompose(0, elements);
            if(newmat) {
                rplDropData(elements);
                rplPushData(newmat);
            }
            if(!newmat || Exceptions) {
                if(DSTop > savestk)
                    DSTop = savestk;
            }
            return;
        }
        else if(n == 1) {
            switch (OPCODE(CurOpcode)) {
            case PTCHEBYCHEFF:
            case PLEGENDRE:
            case PHERMITE2:
                rplPushData((word_p) (one_bint));
                rplPushData((word_p) (zero_bint));
                break;
            case PHERMITE:
            case PTCHEBYCHEFF2:
                rplPushData((word_p) (two_bint));
                rplPushData((word_p) (zero_bint));
                break;
            }
            int elements = 2;
            word_p newmat = rplMatrixCompose(0, elements);
            if(newmat) {
                rplDropData(elements);
                rplPushData(newmat);
            }
            if(!newmat || Exceptions) {
                if(DSTop > savestk)
                    DSTop = savestk;
            }
            return;
        }
        else {
            // reserve space for 2 vectors of length n+1
            rplExpandStack(2 * (n + 1));
            if(Exceptions) {
                if(DSTop > savestk)
                    DSTop = savestk;
                return;
            }

            int32_t saveprec = Context.precdigits;

            int32_t argdigits = (n + 7) & ~7;

            if(argdigits > MAX_USERPRECISION) {
                argdigits = MAX_USERPRECISION;
            }

            argdigits =
                    (argdigits >
                    Context.precdigits) ? argdigits : Context.precdigits;
            //   AUTOMATICALLY INCREASE PRECISION TEMPORARILY

            Context.precdigits = argdigits;

            int32_t i = 0, j = 0;
            for(i = 0; i < 2; ++i) {
                // polynomial has n+1 elements
                for(j = 0; j <= n; ++j) {
                    rplPushData((word_p) (zero_bint)); // fill with ZERO
                }
            }

            int evenodd = n % 2;
            int ii = 0;
            int cur = ii % 2;
            if(!evenodd)
                cur = 1 - cur;
            int oth = 1 - cur;
            // todo populate n=0 and n=1
            switch (OPCODE(CurOpcode)) {
            case PTCHEBYCHEFF:
            case PLEGENDRE:
            case PHERMITE2:
                rplOverwriteData(cur * (n + 1) + 1, (word_p) (one_bint));      //n=0
                rplOverwriteData(oth * (n + 1) + 2, (word_p) (one_bint));      //n=1
                break;
            case PHERMITE:
            case PTCHEBYCHEFF2:
                rplOverwriteData(cur * (n + 1) + 1, (word_p) (one_bint));      //n=0
                rplOverwriteData(oth * (n + 1) + 2, (word_p) (two_bint));      //n=1
                break;
            }

            // recrsive formula
            rplNumberToRReg(2, (word_p) (two_bint));
            for(i = 2; i < n + 1; ++i)  // i=n+1
            {
                rplLoadInt64AsReal(i - 1, &RReg[5]);     // n
                rplLoadInt64AsReal(2 * i - 1, &RReg[6]); // 2n+1
                rplLoadInt64AsReal(i, &RReg[7]); // n+1

                // switch via i mod 2
                int cur = i % 2;
                if(!evenodd)
                    cur = 1 - cur;
                int oth = 1 - cur;
                for(j = i; j >= 0; --j) {
                    rplNumberToRReg(0, rplPeekData(cur * (n + 1) + j + 1));     //previous
                    if(j > 0) {
                        rplNumberToRReg(1, rplPeekData(oth * (n + 1) + j));     // x*current (=shift left)
                    }
                    else {
                        rplNumberToRReg(1, (word_p) (zero_bint));      // the last is zero
                    }
                    switch (OPCODE(CurOpcode)) {
                    case PTCHEBYCHEFF:
                    case PTCHEBYCHEFF2:
                        mulReal(&RReg[3], &RReg[2], &RReg[1]);  // 2*x*current
                        subReal(&RReg[4], &RReg[3], &RReg[0]);  // 2*x*current - previous
                        break;
                    case PLEGENDRE:
                        mulReal(&RReg[3], &RReg[5], &RReg[0]);  // n*previous
                        mulReal(&RReg[0], &RReg[6], &RReg[1]);  // (2n+1)*x*current
                        subReal(&RReg[1], &RReg[0], &RReg[3]);  // (2n+1)*x*current - n*previous
                        divReal(&RReg[4], &RReg[1], &RReg[7]);  // 2*x*current - 2*n*previous
                        break;
                    case PHERMITE:
                        mulReal(&RReg[3], &RReg[5], &RReg[0]);  // n*previous
                        subReal(&RReg[0], &RReg[1], &RReg[3]);  // x*current - n*previous
                        mulReal(&RReg[4], &RReg[0], &RReg[2]);  // 2*x*current - 2*n*previous
                        break;
                    case PHERMITE2:
                        mulReal(&RReg[3], &RReg[5], &RReg[0]);  // n*previous
                        subReal(&RReg[4], &RReg[1], &RReg[3]);  // x*current - n*previous
                        break;
                    }

                    rplCheckResultAndError(&RReg[4]);   // next
                    word_p newnumber = rplNewReal(&RReg[4]);
                    if(!newnumber || Exceptions) {
                        if(DSTop > savestk)
                            DSTop = savestk;
                        Context.precdigits = saveprec;
                        return;
                    }
                    rplOverwriteData(cur * (n + 1) + j + 1, newnumber); // set value

                }
            }
            // we are done. next create vector
            int elements = n + 1;
            rplDropData(elements);      // drop the 2nd exploded vector from n=previous
            Context.precdigits = saveprec;
            word_p newmat = rplMatrixCompose(0, elements);     //create vector from stack

            if(newmat) {
                rplDropData(elements);
                rplPushData(newmat);
            }
            else {
                if(DSTop > savestk)
                    DSTop = savestk;
                return;
            }

        }
        return;
    }

    case DIV2:
        //@SHORT_DESC=Polynomial euclidean division as symbolic
        // TODO: THIS NEEDS TO RETURN A SYMBOLIC EXPRESSION
        return;
    case PDIV2:
        //@SHORT_DESC=Polynomial euclidean division as coefficient vector
    {
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        word_p *savestk = DSTop;       // Drop arguments in case of error

        word_p arg1 = rplPeekData(2);
        word_p arg2 = rplPeekData(1);

        // POLYNOMIAL DIVISION arg1/arg2 = quot , remainder
        if(ISMATRIX(*arg1) && ISMATRIX(*arg2)) {
            int32_t rows1 = MATROWS(arg1[1]), cols1 = MATCOLS(arg1[1]);
            int32_t rows2 = MATROWS(arg2[1]), cols2 = MATCOLS(arg2[1]);
            // Check for vector only
            if(rows1 || rows2) {
                if(DSTop > savestk)
                    DSTop = savestk;
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            // only numbers allowed
            int32_t f;
            for(f = 0; f < cols1; ++f) {
                word_p entry = rplMatrixFastGet(arg1, 1, f + 1);
                if(!ISNUMBER(*entry)) {
                    if(DSTop > savestk)
                        DSTop = savestk;
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }
            for(f = 0; f < cols2; ++f) {
                word_p entry = rplMatrixFastGet(arg2, 1, f + 1);
                if(!ISNUMBER(*entry)) {
                    if(DSTop > savestk)
                        DSTop = savestk;
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }

            // Eliminate leading zeros to get the real order
            int32_t leading_zeroes_arg1 = 0, leading_zeroes_arg2 = 0;
            for(f = 0; f < cols1; ++f) {
                word_p entry = rplMatrixFastGet(arg1, 1, f + 1);
                rplNumberToRReg(0, entry);
                if(iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg1;
                }
                else {
                    break;
                }
            }
            for(f = 0; f < cols2; ++f) {
                word_p entry = rplMatrixFastGet(arg2, 1, f + 1);
                rplNumberToRReg(0, entry);
                if(iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg2;
                }
                else {
                    break;
                }
            }

            if(Exceptions) {
                if(DSTop > savestk)
                    DSTop = savestk;
                return;
            }

            // now we know all leading zeroes
            if((cols1 - leading_zeroes_arg1) < (cols2 - leading_zeroes_arg2)) {
                rplPushData((word_p) (zero_bint));
                int32_t elements = 1;
                word_p newmat = rplMatrixCompose(0, elements);
                if(newmat) {
                    rplDropData(elements);
                    rplOverwriteData(1, rplPeekData(2));
                    rplOverwriteData(2, newmat);
                }
            }
            else {
                int32_t saveprec = Context.precdigits;
                int32_t argdigits = (saveprec + 2 * cols2 + 7) & ~7;
                if(argdigits > MAX_USERPRECISION) {
                    argdigits = MAX_USERPRECISION;
                }
                argdigits =
                        (argdigits >
                        Context.precdigits) ? argdigits : Context.precdigits;
                //   AUTOMATICALLY INCREASE PRECISION TEMPORARILY
                Context.precdigits = argdigits;

                // copy dividend
                for(f = leading_zeroes_arg1; f < cols1; ++f) {
                    word_p entry = rplMatrixFastGet(arg1, 1, f + 1);
                    rplPushData(entry);
                }
                word_p normalizer = rplMatrixFastGet(arg2, 1, leading_zeroes_arg2 + 1);        // divisor[0]
                rplNumberToRReg(0, normalizer);
                for(f = 0;
                        f <
                        (cols1 - leading_zeroes_arg1) - (cols2 -
                            leading_zeroes_arg2) + 1; ++f) {
                    rplNumberToRReg(1, rplPeekData((cols1 - leading_zeroes_arg1) - f)); // out[i]
                    divReal(&RReg[2], &RReg[1], &RReg[0]);
                    word_p newnumber = rplNewReal(&RReg[2]);
                    if(!newnumber || Exceptions) {
                        if(DSTop > savestk)
                            DSTop = savestk;
                        Context.precdigits = saveprec;
                        return;
                    }
                    rplOverwriteData((cols1 - leading_zeroes_arg1) - f, newnumber);     //out[i] /= normalizer
                    if(!iszeroReal(&RReg[2]))   // coef = RReg[2]
                    {
                        int32_t j;
                        for(j = leading_zeroes_arg2 + 1; j < cols2; ++j) {
                            word_p divj = rplMatrixFastGet(arg2, 1, j + 1);    // divisor[j]
                            rplNumberToRReg(1, divj);
                            mulReal(&RReg[1], &RReg[1], &RReg[2]);
                            rplNumberToRReg(3, rplPeekData((cols1 - leading_zeroes_arg1) - f - (j - leading_zeroes_arg2)));     // out[i]
                            subReal(&RReg[3], &RReg[3], &RReg[1]);
                            word_p newnumber = rplNewReal(&RReg[3]);
                            if(!newnumber || Exceptions) {
                                if(DSTop > savestk)
                                    DSTop = savestk;
                                Context.precdigits = saveprec;
                                return;
                            }
                            rplOverwriteData((cols1 - leading_zeroes_arg1) - f -
                                    (j - leading_zeroes_arg2), newnumber);
                        }
                    }
                }
                int32_t elements_remainder = (cols2 - leading_zeroes_arg2) - 1;
                int32_t leading_zeroes_remainder = 0;
                for(f = 0; f < elements_remainder; ++f) {
                    word_p entry = rplPeekData(elements_remainder - f);
                    rplNumberToRReg(0, entry);
                    if(iszeroReal(&RReg[0])) {
                        ++leading_zeroes_remainder;
                    }
                    else {
                        break;
                    }
                }
                int32_t nrem = elements_remainder - leading_zeroes_remainder;
                if(nrem < 1) {
                    nrem = 1;
                }
                word_p remainder = rplMatrixCompose(0, nrem);
                if(remainder) {
                    rplDropData(elements_remainder);
                    int32_t elements_quotient =
                            (cols1 - leading_zeroes_arg1) - (cols2 -
                            leading_zeroes_arg2) + 1;
                    word_p quotient = rplMatrixCompose(0, elements_quotient);
                    if(quotient) {
                        rplDropData(elements_quotient);
                        rplOverwriteData(2, quotient);
                        rplOverwriteData(1, remainder);
                    }
                    else {
                        if(DSTop > savestk)
                            DSTop = savestk;
                        Context.precdigits = saveprec;
                        return;
                    }
                }
                else {
                    if(DSTop > savestk)
                        DSTop = savestk;
                    Context.precdigits = saveprec;
                    return;
                }
                Context.precdigits = saveprec;
            }
        }
        else {
            if(DSTop > savestk)
                DSTop = savestk;
            rplError(ERR_VECTOROFNUMBERSEXPECTED);
        }
        return;
    }

    case PINT:
        //@SHORT_DESC=Integration of polynomials as coefficient vector
        //@NEW
    case PDER:
        //@SHORT_DESC=Derivative of polynomial as coefficient vector
        //@NEW
    {
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        word_p poly = rplPeekData(1);

        if(ISMATRIX(*poly)) {

            int32_t rows = MATROWS(poly[1]), cols = MATCOLS(poly[1]);

            if(rows) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }
            int32_t f;

            for(f = 1; f <= cols; ++f) {
                word_p entry = rplMatrixFastGet(poly, 1, f);
                if(!ISNUMBER(*entry)) {
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }
            // DO IT ALL WITH REALS
            word_p *savestk = DSTop;   // Drop arguments in case of error

            int32_t leading_zeroes_arg = 0;
            for(f = 0; f < cols; ++f) {
                word_p entry = rplMatrixFastGet(poly, 1, f + 1);
                rplNumberToRReg(0, entry);
                if(iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg;
                }
                else {
                    break;
                }
            }

            // copy trimmed polynomial and do operation
            const int32_t degree = cols - leading_zeroes_arg - 1;
            int32_t idegree, endcol, nout;
            if(OPCODE(CurOpcode) == PDER) {
                idegree = degree;
                endcol = cols - 1;
                nout = degree;
            }
            else {
                idegree = degree + 1;
                endcol = cols;
                nout = degree + 2;
            }
            for(f = leading_zeroes_arg; f < endcol; ++f, --idegree) {
                word_p entry = rplMatrixFastGet(poly, 1, f + 1);
                rplNumberToRReg(1, entry);
                rplint32_tToRReg(0, idegree);
                if(OPCODE(CurOpcode) == PDER) {
                    mulReal(&RReg[2], &RReg[1], &RReg[0]);
                }
                else {
                    divReal(&RReg[2], &RReg[1], &RReg[0]);
                }
                word_p newnumber = rplNewReal(&RReg[2]);
                if(!newnumber || Exceptions) {
                    if(DSTop > savestk)
                        DSTop = savestk;
                    return;
                }
                rplPushData(newnumber);
            }
            if(OPCODE(CurOpcode) == PINT) {
                rplPushData((word_p) (zero_bint));
            }
            word_p pout = rplMatrixCompose(0, nout);
            if(!pout || Exceptions) {
                if(DSTop > savestk)
                    DSTop = savestk;
                return;
            }
            rplDropData(nout);
            rplOverwriteData(1, pout);
        }
        else {
            rplError(ERR_VECTOREXPECTED);
        }
        return;
    }

    case PADD:
        //@SHORT_DESC=Addition of polynomials as coefficient vector
        //@NEW
    case PSUB:
    {
        //@SHORT_DESC=Subtraction of polynomials as coefficient vector
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        word_p arg1 = rplPeekData(2);
        word_p arg2 = rplPeekData(1);

        // POLYNOMIAL DIVISION
        if(ISMATRIX(*arg1) && ISMATRIX(*arg2)) {
            int32_t rows1 = MATROWS(arg1[1]), cols1 = MATCOLS(arg1[1]);
            int32_t rows2 = MATROWS(arg2[1]), cols2 = MATCOLS(arg2[1]);
            // Check for vector only
            if(rows1 || rows2) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            // only numbers allowed
            int32_t f;
            for(f = 0; f < cols1; ++f) {
                word_p entry = rplMatrixFastGet(arg1, 1, f + 1);
                if(!ISNUMBER(*entry)) {
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }
            for(f = 0; f < cols2; ++f) {
                word_p entry = rplMatrixFastGet(arg2, 1, f + 1);
                if(!ISNUMBER(*entry)) {
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }

            // Eliminate leading zeros to get the real degree
            int32_t leading_zeroes_arg1 = 0, leading_zeroes_arg2 = 0;
            for(f = 0; f < cols1; ++f) {
                word_p entry = rplMatrixFastGet(arg1, 1, f + 1);
                rplNumberToRReg(0, entry);
                if(iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg1;
                }
                else {
                    break;
                }
            }
            for(f = 0; f < cols2; ++f) {
                word_p entry = rplMatrixFastGet(arg2, 1, f + 1);
                rplNumberToRReg(0, entry);
                if(iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg2;
                }
                else {
                    break;
                }
            }

            if(Exceptions) {
                return;
            }

            int32_t nelem1 = cols1 - leading_zeroes_arg1;  // degree1 = nelem1 -1;
            int32_t nelem2 = cols2 - leading_zeroes_arg2;  // degree2 = nelem2 -1;

            int32_t nelem3 = nelem1 > nelem2 ? nelem1 : nelem2;    // degree = max(degree1, degree2)

            word_p *savestk = DSTop;   // Drop arguments in case of error

            for(f = 0; f < nelem3; ++f) {
                int32_t i1 = cols1 - nelem3 + f;
                int32_t i2 = cols2 - nelem3 + f;
                if(i1 < leading_zeroes_arg1) {
                    rplint32_tToRReg(1, 0);
                }
                else {
                    word_p entry = rplMatrixFastGet(arg1, 1, i1 + 1);
                    rplNumberToRReg(1, entry);
                }
                if(i2 < leading_zeroes_arg2) {
                    rplint32_tToRReg(2, 0);
                }
                else {
                    word_p entry = rplMatrixFastGet(arg2, 1, i2 + 1);
                    rplNumberToRReg(2, entry);
                }
                if(OPCODE(CurOpcode) == PADD) {
                    addReal(&RReg[0], &RReg[1], &RReg[2]);
                }
                else {
                    subReal(&RReg[0], &RReg[1], &RReg[2]);
                }
                word_p newnumber = rplNewReal(&RReg[0]);
                if(!newnumber || Exceptions) {
                    if(DSTop > savestk)
                        DSTop = savestk;
                    return;
                }
                rplPushData(newnumber);
            }
            int32_t leading_zeroes_pout = 0;
            for(f = 0; f < nelem3; ++f) {
                word_p entry = rplPeekData(nelem3 - f);
                rplNumberToRReg(0, entry);
                if(iszeroReal(&RReg[0])) {
                    ++leading_zeroes_pout;
                }
                else {
                    break;
                }
            }
            int32_t nout = nelem3 - leading_zeroes_pout;
            if(nout < 1) {
                nout = 1;
            }
            word_p poly = rplMatrixCompose(0, nout);
            if(!poly || Exceptions) {
                if(DSTop > savestk)
                    DSTop = savestk;
                return;
            }
            rplDropData(nelem3 + 1);
            rplOverwriteData(1, poly);

        }
        else {
            rplError(ERR_VECTOROFNUMBERSEXPECTED);
        }
        return;
    }

    case PMUL:
    {
        //@SHORT_DESC=Multiplication of polynomials as coefficient vectors
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        word_p arg1 = rplPeekData(2);
        word_p arg2 = rplPeekData(1);

        // POLYNOMIAL DIVISION
        if(ISMATRIX(*arg1) && ISMATRIX(*arg2)) {
            int32_t rows1 = MATROWS(arg1[1]), cols1 = MATCOLS(arg1[1]);
            int32_t rows2 = MATROWS(arg2[1]), cols2 = MATCOLS(arg2[1]);
            // Check for vector only
            if(rows1 || rows2) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            // only numbers allowed
            int32_t f;
            for(f = 0; f < cols1; ++f) {
                word_p entry = rplMatrixFastGet(arg1, 1, f + 1);
                if(!ISNUMBER(*entry)) {
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }
            for(f = 0; f < cols2; ++f) {
                word_p entry = rplMatrixFastGet(arg2, 1, f + 1);
                if(!ISNUMBER(*entry)) {
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
            }

            // Eliminate leading zeros to get the real degree
            int32_t leading_zeroes_arg1 = 0, leading_zeroes_arg2 = 0;
            for(f = 0; f < cols1; ++f) {
                word_p entry = rplMatrixFastGet(arg1, 1, f + 1);
                rplNumberToRReg(0, entry);
                if(iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg1;
                }
                else {
                    break;
                }
            }
            for(f = 0; f < cols2; ++f) {
                word_p entry = rplMatrixFastGet(arg2, 1, f + 1);
                rplNumberToRReg(0, entry);
                if(iszeroReal(&RReg[0])) {
                    ++leading_zeroes_arg2;
                }
                else {
                    break;
                }
            }

            if(Exceptions) {
                return;
            }

            int32_t nelem1 = cols1 - leading_zeroes_arg1;  // degree1 = nelem1 -1;
            int32_t nelem2 = cols2 - leading_zeroes_arg2;  // degree2 = nelem2 -1;

            int32_t nout = nelem1 + nelem2 - 1;    // nout = degree+1 = degree1+degree2+1

            word_p *savestk = DSTop;   // Drop arguments in case of error

            int32_t i1, i2, iout, g;

            for(f = 0; f < nout; ++f) {
                rplPushData((word_p) (zero_bint));
            }

            for(f = leading_zeroes_arg1, i1 = 0; f < cols1; ++f, ++i1) {
                word_p entry = rplMatrixFastGet(arg1, 1, f + 1);
                rplNumberToRReg(1, entry);
                if(!iszeroReal(&RReg[1])) {
                    for(g = leading_zeroes_arg2, i2 = 0; g < cols2; ++g, ++i2) {
                        word_p entry = rplMatrixFastGet(arg2, 1, g + 1);
                        rplNumberToRReg(2, entry);
                        if(!iszeroReal(&RReg[2])) {
                            mulReal(&RReg[0], &RReg[1], &RReg[2]);
                            iout = i1 + i2;
                            rplNumberToRReg(3, rplPeekData(nout - iout));
                            addReal(&RReg[2], &RReg[0], &RReg[3]);
                            word_p newnumber = rplNewReal(&RReg[2]);
                            if(!newnumber || Exceptions) {
                                if(DSTop > savestk)
                                    DSTop = savestk;
                                return;
                            }
                            rplOverwriteData(nout - iout, newnumber);
                        }
                    }
                }
            }
            word_p poly = rplMatrixCompose(0, nout);
            if(!poly || Exceptions) {
                if(DSTop > savestk)
                    DSTop = savestk;
                return;
            }
            rplDropData(nout + 1);
            rplOverwriteData(1, poly);

        }
        else {
            rplError(ERR_VECTOROFNUMBERSEXPECTED);
        }
        return;
    }

    case MIN:
        //@SHORT_DESC=Smallest of 2 objects
    {
        // COMPARE ANY 2 OBJECTS AND KEEP THE SMALLEST

        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            // THIS IS A COMPOSITE, NEED TO RUN AN RPL LOOP
            rplListBinaryDoCmd();
            return;
        }

        if(ISIDENT(*rplPeekData(2)) || ISSYMBOLIC(*rplPeekData(2)
                    || ISCONSTANT(*rplPeekData(2))) || ISIDENT(*rplPeekData(1))
                || ISSYMBOLIC(*rplPeekData(1)) || ISCONSTANT(*rplPeekData(1))) {
            rplSymbApplyOperator(CurOpcode, 2);
            return;
        }

        word_p *saveStk = DSTop;

        rplPushData(rplPeekData(2));
        rplPushData(rplPeekData(2));

        rplCallOvrOperator(CMD_OVR_LTE);
        if(Exceptions) {
            // CLEANUP THE STACK BEFORE RETURNING
            DSTop = saveStk;
            return;
        }

        if(rplIsFalse(rplPeekData(1))) {
            // KEEP THE SECOND OBJECT
            rplOverwriteData(3, rplPeekData(2));
        }

        rplDropData(2);

        return;
    }

    case MAX:
        //@SHORT_DESC=Largest of 2 objects
    {
        // COMPARE ANY 2 OBJECTS AND KEEP THE SMALLEST

        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            // THIS IS A COMPOSITE, NEED TO RUN AN RPL LOOP
            rplListBinaryDoCmd();
            return;
        }

        if(ISIDENT(*rplPeekData(2)) || ISSYMBOLIC(*rplPeekData(2)
                    || ISCONSTANT(*rplPeekData(2))) || ISIDENT(*rplPeekData(1))
                || ISSYMBOLIC(*rplPeekData(1)) || ISCONSTANT(*rplPeekData(1))) {
            rplSymbApplyOperator(CurOpcode, 2);
            return;
        }

        word_p *saveStk = DSTop;

        rplPushData(rplPeekData(2));
        rplPushData(rplPeekData(2));

        rplCallOvrOperator(CMD_OVR_GTE);
        if(Exceptions) {
            // CLEANUP THE STACK BEFORE RETURNING
            DSTop = saveStk;
            return;
        }

        if(rplIsFalse(rplPeekData(1))) {
            // KEEP THE SECOND OBJECT
            rplOverwriteData(3, rplPeekData(2));
        }

        rplDropData(2);

        return;
    }

    case RND:
        //@SHORT_DESC=Round a number to the given number of figures
    case TRNC:
        //@SHORT_DESC=Truncate a number to the given number of figures
    {
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(2);

        word_p arg = rplPeekData(2);
        word_p ndig = rplPeekData(1);

        if(ISLIST(*arg) || ISLIST(*ndig)) {
            rplListBinaryDoCmd();
            return;
        }

        if(ISIDENT(*arg) || ISSYMBOLIC(*arg) || ISCONSTANT(*arg)
                || ISIDENT(*ndig) || ISSYMBOLIC(*ndig) || ISCONSTANT(*ndig)) {
            rplSymbApplyOperator(CurOpcode, 2);
            return;
        }

        if(!ISNUMBER(*ndig)) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        word_p *savestk = DSTop;
        int64_t nd = rplReadNumberAsInt64(ndig), isunit = 0, unitlevels = 0;
        if(Exceptions)
            return;

        if(ISUNIT(*arg)) {
            unitlevels = rplUnitExplode(arg);
            if(Exceptions) {
                DSTop = savestk;
                return;
            }
            rplPushData(rplPeekData(unitlevels));
            rplPushData(rplPeekData(1));
            arg = rplPeekData(1);
            isunit = 1;
        }

        if(ISNUMBER(*arg)) {
            REAL r;
            rplReadNumberAsReal(arg, &r);
            if(Exceptions)
                return;

            if(OPCODE(CurOpcode) == RND)
                roundReal(&RReg[0], &r, nd);
            else
                truncReal(&RReg[0], &r, nd);

            word_p newresult;

            if(isintegerReal(&RReg[0])) {
                if(inint64_tRange(&RReg[0])) {
                    int64_t res = getint64_tReal(&RReg[0]);
                    newresult = rplNewBINT(res, DECBINT);
                }
                else
                    newresult = rplNewRealFromRReg(0);
            }
            else
                newresult = rplNewRealFromRReg(0);

            if(!newresult)
                return;
            rplDropData(1);
            rplOverwriteData(1, newresult);

        }
        else if(ISCOMPLEX(*arg)) {
            REAL Rarg, Iarg;
            int32_t cclass = rplComplexClass(arg);
            rplReadCNumberAsReal(arg, &Rarg);
            rplReadCNumberAsImag(arg, &Iarg);

            switch (cclass) {

            case CPLX_NORMAL:
            {
                // ROUND REAL AND IMAGINARY PARTS INDEPENDENTLY
                if(OPCODE(CurOpcode) == RND) {
                    roundReal(&RReg[0], &Rarg, nd);
                    roundReal(&RReg[1], &Iarg, nd);
                }
                else {
                    truncReal(&RReg[0], &Rarg, nd);
                    truncReal(&RReg[1], &Iarg, nd);
                }
                word_p newresult =
                        rplNewComplex(&RReg[0], &RReg[1], ANGLENONE);
                if(!newresult)
                    return;

                rplDropData(1);
                rplOverwriteData(1, newresult);
                break;
            }
            case CPLX_POLAR:
            {
                // ONLY ROUND THE MAGNITUDE, LEAVE THE ANGLE AS IS
                if(OPCODE(CurOpcode) == RND) {
                    roundReal(&RReg[0], &Rarg, nd);
                }
                else {
                    truncReal(&RReg[0], &Rarg, nd);
                }
                word_p newresult =
                        rplNewComplex(&RReg[0], &Iarg,
                        rplPolarComplexMode(arg));
                if(!newresult)
                    return;

                rplDropData(1);
                rplOverwriteData(1, newresult);
                break;

            }
            default:
            case CPLX_INF:
            case CPLX_MALFORMED:
            case CPLX_NAN:
            case CPLX_UNDINF:
            case CPLX_ZERO:
                return; // NOTHING TO ROUND

            }

        }
        else if(ISMATRIX(*arg)) {
            // ROUND EVERY ELEMENT IN THE MATRIX, AND APPLY THE OPERATOR TO EVERY SYMBOLIC ELEMENT IN THE MATRIX
            word_p *a;
            // DONT KEEP POINTER TO THE MATRICES, BUT POINTERS TO THE POINTERS IN THE STACK
            // AS THE OBJECTS MIGHT MOVE DURING THE OPERATION
            a = DSTop - 2;

            // a IS THE MATRIX

            // CHECK DIMENSIONS

            int32_t rowsa = MATROWS(*(*a + 1)), colsa = MATCOLS(*(*a + 1));

            int32_t totalelements = (rowsa) ? rowsa * colsa : colsa;

            int32_t j;

            // DO THE ELEMENT-BY-ELEMENT OPERATION

            for(j = 1; j <= totalelements; ++j) {
                rplPushData(rplMatrixFastGet(*a, 1, j));
                word_p arg = rplPeekData(1);
                if(ISIDENT(*arg) || ISSYMBOLIC(*arg) || ISIDENT(*ndig)
                        || ISSYMBOLIC(*ndig) || ISCONSTANT(*arg)
                        || ISCONSTANT(*ndig)) {
                    rplPushData(a[1]);
                    rplSymbApplyOperator(CurOpcode, 2);
                    if(Exceptions) {
                        DSTop = savestk;
                        return;
                    }
                    continue;
                }

                if(ISNUMBER(*arg)) {
                    REAL r;
                    rplReadNumberAsReal(arg, &r);
                    if(Exceptions) {
                        DSTop = savestk;
                        return;
                    }

                    if(OPCODE(CurOpcode) == RND)
                        roundReal(&RReg[0], &r, nd);
                    else
                        truncReal(&RReg[0], &r, nd);

                    word_p newresult;

                    if(isintegerReal(&RReg[0])) {
                        if(inint64_tRange(&RReg[0])) {
                            int64_t res = getint64_tReal(&RReg[0]);
                            newresult = rplNewBINT(res, DECBINT);
                        }
                        else
                            newresult = rplNewRealFromRReg(0);
                    }
                    else
                        newresult = rplNewRealFromRReg(0);

                    if(!newresult) {
                        DSTop = savestk;
                        return;
                    }
                    rplOverwriteData(1, newresult);
                    continue;
                }

                if(ISCOMPLEX(*arg)) {
                    REAL Rarg, Iarg;
                    int32_t cclass = rplComplexClass(arg);
                    rplReadCNumberAsReal(arg, &Rarg);
                    rplReadCNumberAsImag(arg, &Iarg);

                    switch (cclass) {

                    case CPLX_NORMAL:
                    {
                        // ROUND REAL AND IMAGINARY PARTS INDEPENDENTLY
                        if(OPCODE(CurOpcode) == RND) {
                            roundReal(&RReg[0], &Rarg, nd);
                            roundReal(&RReg[1], &Iarg, nd);
                        }
                        else {
                            truncReal(&RReg[0], &Rarg, nd);
                            truncReal(&RReg[1], &Iarg, nd);
                        }
                        word_p newresult =
                                rplNewComplex(&RReg[0], &RReg[1], ANGLENONE);
                        if(!newresult) {
                            DSTop = savestk;
                            return;
                        }

                        rplOverwriteData(1, newresult);
                        break;
                    }
                    case CPLX_POLAR:
                    {
                        // ONLY ROUND THE MAGNITUDE, LEAVE THE ANGLE AS IS
                        if(OPCODE(CurOpcode) == RND) {
                            roundReal(&RReg[0], &Rarg, nd);
                        }
                        else {
                            truncReal(&RReg[0], &Rarg, nd);
                        }
                        word_p newresult =
                                rplNewComplex(&RReg[0], &Iarg,
                                rplPolarComplexMode(arg));
                        if(!newresult) {
                            DSTop = savestk;
                            return;
                        }

                        rplOverwriteData(1, newresult);
                        break;

                    }
                    default:
                    case CPLX_INF:
                    case CPLX_MALFORMED:
                    case CPLX_NAN:
                    case CPLX_UNDINF:
                    case CPLX_ZERO:
                        break;  // NOTHING TO ROUND

                    }

                    continue;
                }

            }

            word_p newmat = rplMatrixCompose(rowsa, colsa);
            DSTop = a + 2;
            if(!newmat)
                return;
            rplOverwriteData(2, newmat);
            rplDropData(1);
        }
        else {
            rplError(ERR_BADARGTYPE);
            return;
        }

        if(isunit) {
            rplOverwriteData(unitlevels + 1, rplPeekData(1));
            rplDropData(1);
            word_p newunit = rplUnitAssemble(unitlevels);
            if(!newunit) {
                DSTop = savestk;
                return;
            }
            rplDropData(unitlevels + 1);
            rplOverwriteData(1, newunit);

        }

        return;
    }

    case DIGITS:
        //@SHORT_DESC=Extract digits from a real number
        //@NEW
        // EXTRACT DIGITS FROM A REAL NUMBER
        // GIVEN THE NUMBER AND POSITION START/END
        // POSITION IS GIVEN IN 10s POWERS (0 = UNITY, 1 = TENS, ETC)
    {
        if(rplDepthData() < 3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStripTagStack(3);

        if(ISLIST(*rplPeekData(3))) {
            rplListMultiArgDoCmd(3);
            return;
        }

        if(!ISNUMBER(*rplPeekData(3))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        if(!ISNUMBER(*rplPeekData(2)) || !ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        REAL re;
        int64_t start, end;

        rplReadNumberAsReal(rplPeekData(3), &re);
        start = rplReadNumberAsInt64(rplPeekData(2));
        end = rplReadNumberAsInt64(rplPeekData(1));

        if(Exceptions)
            return;

        if(start < end) {
            int64_t tmp = start;
            start = end;
            end = tmp;
        }

        re.exp -= end;
        ipReal(&RReg[0], &re, 1);
        RReg[0].exp += end;
        RReg[0].exp -= start + 1;
        fracReal(&RReg[1], &RReg[0]);
        RReg[1].exp += (start - end) + 1;
        rplDropData(3);
        if(inint64_tRange(&RReg[1])) {
            start = getint64_tReal(&RReg[1]);
            rplNewBINTPush(start, DECBINT);
        }
        else
            rplNewRealFromRRegPush(1);

        return;
    }

    case PROOT:
    {
        //@SHORT_DESC=All roots of a polynomial
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        word_p vect_val = rplPeekData(1);

        if(ISLIST(*vect_val)) {
            rplListUnaryDoCmd();
            return;
        }
        else if(ISMATRIX(*vect_val)) {

            int32_t cplxmode = rplTestSystemFlag(FL_COMPLEXMODE);
            rplSetSystemFlag(FL_COMPLEXMODE);

            int32_t rows = MATROWS(vect_val[1]), cols = MATCOLS(vect_val[1]);

            if(rows) {
                if(!cplxmode)
                    rplClrSystemFlag(FL_COMPLEXMODE);
                rplError(ERR_VECTOREXPECTED);
                return;
            }
            int32_t f;
            word_p *savestk = DSTop;

            for(f = 0; f < cols; ++f) {
                word_p entry = rplMatrixFastGet(vect_val, 1, f + 1);
                if(!ISNUMBERCPLX(*entry)) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
                rplPushData(entry);
            }

            word_p solution;
            REAL re;
            for(f = 1; f < cols - 1; ++f) {

                if(f > 1) {
                    // TEST IF PREVIOUS SOLUTION IS ALSO A SOLUTION OF THE DEFLATED POLYNOMIAL

                    solution = rplPolyEvalEx(savestk, cols - f, DSTop - 1);
                    if(!solution || Exceptions) {
                        if(!cplxmode)
                            rplClrSystemFlag(FL_COMPLEXMODE);
                        DSTop = savestk;
                        return;
                    }
                    rplPushData(solution);

                    rplCallOvrOperator(CMD_OVR_ABS);
                    if(Exceptions) {
                        if(!cplxmode)
                            rplClrSystemFlag(FL_COMPLEXMODE);
                        DSTop = savestk;
                        return;
                    }
                    rplReadNumberAsReal(rplPopData(), &re);
                    if(Exceptions) {
                        if(!cplxmode)
                            rplClrSystemFlag(FL_COMPLEXMODE);
                        DSTop = savestk;
                        return;
                    }

                    if(iszeroReal(&re)
                            || (intdigitsReal(&re) <
                                -Context.precdigits +
                                (Context.precdigits >> 3))) {
                        // THE CURRENT ROOT HAS MULTIPLICITY
                        rplPushData(rplPeekData(1));

                        // DEFLATE THE POLYNOMIAL
                        rplPolyDeflateEx(savestk, cols - f, DSTop - 1);
                        if(Exceptions) {
                            if(!cplxmode)
                                rplClrSystemFlag(FL_COMPLEXMODE);
                            DSTop = savestk;
                            return;
                        }
                        continue;
                    }

                }

                solution = rplPolyRootEx(savestk, cols - f);
                if(!solution || Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }

                // WE HAVE ONE SOLUTION!
                rplPushData(solution);
                if(Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }
                // DEFLATE THE POLYNOMIAL
                rplPolyDeflateEx(savestk, cols - f, DSTop - 1);
                if(Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }

                // NOW THE POLYNOMIAL IS ONE DEGREE LESS
            }

            // HERE WE HAVE ALL ROOTS OF THE POLYNOMIAL EXCEPT THE LAST ONE
            // THE POLYNOMIAL OF DEGREE 1 IS a0*X+a1 = 0
            // THEREFORE THE LAST ROOT IS X=-a1/a0
            rplPushData(savestk[1]);
            rplPushData(savestk[0]);
            if(Exceptions) {
                if(!cplxmode)
                    rplClrSystemFlag(FL_COMPLEXMODE);
                DSTop = savestk;
                return;
            }

            rplCallOvrOperator(CMD_OVR_DIV);
            if(Exceptions) {
                if(!cplxmode)
                    rplClrSystemFlag(FL_COMPLEXMODE);
                DSTop = savestk;
                return;
            }

            rplCallOvrOperator(CMD_OVR_NEG);
            if(Exceptions) {
                if(!cplxmode)
                    rplClrSystemFlag(FL_COMPLEXMODE);
                DSTop = savestk;
                return;
            }

            int32_t doerror = 0;
            if(!cplxmode) {
                // ISSUE AN ERROR IF ANY OF THE ROOTS ARE COMPLEX
                for(f = 1; f < cols; ++f) {
                    if(ISCOMPLEX(*rplPeekData(f))) {
                        doerror = 1;
                        break;
                    }
                }
            }

            solution = rplMatrixCompose(0, cols - 1);
            if(!solution) {
                if(!cplxmode)
                    rplClrSystemFlag(FL_COMPLEXMODE);
                DSTop = savestk;
                return;
            }
            DSTop = savestk;
            rplOverwriteData(1, solution);
            if(!cplxmode) {
                rplClrSystemFlag(FL_COMPLEXMODE);
                if(doerror)
                    rplError(ERR_COMPLEXRESULT);
            }

            return;

        }
        rplError(ERR_VECTOROFNUMBERSEXPECTED);
        return;

    }

    case FACTORS:

    {
        //@SHORT_DESC=Factorize a polynomial or number
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        word_p vect_val = rplPeekData(1);

        if(ISLIST(*vect_val)) {
            rplListUnaryDoCmd();
            return;
        }
        else if(ISMATRIX(*vect_val)) {

            int32_t cplxmode = rplTestSystemFlag(FL_COMPLEXMODE);
            rplSetSystemFlag(FL_COMPLEXMODE);

            int32_t rows = MATROWS(vect_val[1]), cols = MATCOLS(vect_val[1]);

            if(rows) {
                if(!cplxmode)
                    rplClrSystemFlag(FL_COMPLEXMODE);
                rplError(ERR_VECTOREXPECTED);
                return;
            }
            int32_t f, nroots;
            word_p *savestk = DSTop;

            for(f = 0; f < cols; ++f) {
                word_p entry = rplMatrixFastGet(vect_val, 1, f + 1);
                if(!ISNUMBERCPLX(*entry)) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    rplError(ERR_VECTOROFNUMBERSEXPECTED);
                    return;
                }
                rplPushData(entry);
            }

            word_p solution;
            REAL re;

            for(nroots = 0, f = 1; f < cols - 1; ++f) {

                if(f > 1) {
                    // TEST IF PREVIOUS SOLUTION IS ALSO A SOLUTION OF THE DEFLATED POLYNOMIAL

                    solution = rplPolyEvalEx(savestk, cols - f, DSTop - 2);
                    if(!solution || Exceptions) {
                        if(!cplxmode)
                            rplClrSystemFlag(FL_COMPLEXMODE);
                        DSTop = savestk;
                        return;
                    }
                    rplPushData(solution);

                    rplCallOvrOperator(CMD_OVR_ABS);
                    if(Exceptions) {
                        if(!cplxmode)
                            rplClrSystemFlag(FL_COMPLEXMODE);
                        DSTop = savestk;
                        return;
                    }
                    rplReadNumberAsReal(rplPopData(), &re);
                    if(Exceptions) {
                        if(!cplxmode)
                            rplClrSystemFlag(FL_COMPLEXMODE);
                        DSTop = savestk;
                        return;
                    }

                    if(iszeroReal(&re)
                            || (intdigitsReal(&re) < -Context.precdigits)) {
                        // THE CURRENT ROOT HAS MULTIPLICITY
                        rplPushData((word_p) one_bint);
                        rplCallOvrOperator(CMD_OVR_ADD);

                        // DEFLATE THE POLYNOMIAL
                        rplPolyDeflateEx(savestk, cols - f, DSTop - 2);
                        if(Exceptions) {
                            if(!cplxmode)
                                rplClrSystemFlag(FL_COMPLEXMODE);
                            DSTop = savestk;
                            return;
                        }
                        continue;
                    }

                }

                solution = rplPolyRootEx(savestk, cols - f);
                if(!solution || Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }

                // WE HAVE ONE SOLUTION!
                rplPushData(solution);
                rplPushData((word_p) one_bint);        // MULTIPLICITY
                ++nroots;
                if(Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }
                // DEFLATE THE POLYNOMIAL
                rplPolyDeflateEx(savestk, cols - f, DSTop - 2);
                if(Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }

                // NOW THE POLYNOMIAL IS ONE DEGREE LESS
            }

            // HERE WE HAVE ALL ROOTS OF THE POLYNOMIAL EXCEPT THE LAST ONE

            // FIRST CHECK IF THE LAST ROOT WE FOUND IS THE NEW ROOT

            if(f > 1) {
                // TEST IF PREVIOUS SOLUTION IS ALSO A SOLUTION OF THE DEFLATED POLYNOMIAL

                solution = rplPolyEvalEx(savestk, cols - f, DSTop - 2);
                if(!solution || Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }
                rplPushData(solution);

                rplCallOvrOperator(CMD_OVR_ABS);
                if(Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }
                rplReadNumberAsReal(rplPopData(), &re);
                if(Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }

                if(iszeroReal(&re)
                        || (intdigitsReal(&re) < -Context.precdigits)) {
                    // THE CURRENT ROOT HAS MULTIPLICITY
                    rplPushData((word_p) one_bint);
                    rplCallOvrOperator(CMD_OVR_ADD);
                    ++f;
                }

            }

            if(f == cols - 1) {

                // ONE LAST ROOT IS NEEDED
                // THE POLYNOMIAL OF DEGREE 1 IS a0*X+a1 = 0
                // THEREFORE THE LAST ROOT IS X=-a1/a0

                rplPushData(savestk[1]);
                rplPushData(savestk[0]);
                if(Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }

                rplCallOvrOperator(CMD_OVR_DIV);
                if(Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }

                rplCallOvrOperator(CMD_OVR_NEG);
                if(Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }
                rplPushData((word_p) one_bint);
                ++nroots;
            }

            int32_t doerror = 0;

            // ISSUE AN ERROR IF ANY OF THE ROOTS ARE COMPLEX
            for(f = 1; f <= nroots; ++f) {
                if((!cplxmode) && ISCOMPLEX(*rplPeekData(2 * f)))
                    doerror = 1;
                rplPushData(rplPeekData(2 * f));
                rplCallOvrOperator(CMD_OVR_NEG);        // CHANGE THE SIGN OF THE ROOT
                if(Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }
                rplOverwriteData(2 * f, rplPopData());
            }

            // PRESENT THE SOLUTION IN A BETTER FORMAT

            word_p *stksol = DSTop;
            for(f = 0; f < nroots; ++f) {
                rplPushData(stksol[-2 - 2 * f]);        // MAKE A COPY OF ALL FACTORS
                if(Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }
            }
            for(f = 0; f < nroots; ++f) {
                rplPushData(stksol[-1 - 2 * f]);        // MAKE A COPY OF ALL MULTIPLICITY NUMBERS
                if(Exceptions) {
                    if(!cplxmode)
                        rplClrSystemFlag(FL_COMPLEXMODE);
                    DSTop = savestk;
                    return;
                }
            }

            rplRemoveAtData(2 * nroots, 2 * nroots);

            solution = rplCreateListN(nroots, 1 + nroots, 0);
            if(!solution) {
                if(!cplxmode)
                    rplClrSystemFlag(FL_COMPLEXMODE);
                DSTop = savestk;
                return;
            }
            rplPushData(solution);
            solution = rplCreateListN(nroots, 2, 0);
            if(!solution) {
                if(!cplxmode)
                    rplClrSystemFlag(FL_COMPLEXMODE);
                DSTop = savestk;
                return;
            }
            rplPushData(solution);

            // PUTH THE LEADING FACTOR FIRST
            savestk[-1] = rplMatrixFastGet(savestk[-1], 1, 1);

            rplRemoveAtData(3, DSTop - savestk - 2);    // REMOVE ALL THE INTERMEDIATE VALUES FROM THE STACK

            if(!cplxmode) {
                rplClrSystemFlag(FL_COMPLEXMODE);
                if(doerror)
                    rplError(ERR_COMPLEXRESULT);
            }

            return;

        }
        else if(ISNUMBER(*vect_val)) {
            // FACTORIZE INTEGER NUMBERS AS WELL

            REAL num, mult;
            int32_t prec = Context.precdigits, isneg, exponent;
            int64_t onefactor, inum;
            word_p *savestk = DSTop;

            rplReadNumberAsReal(vect_val, &num);
            if(num.flags & F_NEGATIVE) {
                isneg = 1;
                num.flags ^= F_NEGATIVE;
            }
            else
                isneg = 0;

            if(!isintegerReal(&num)) {
                exponent = num.exp;     // REMEMBER THE EXPONENT
                num.exp = 0;    // AND FORCE IT TO BE INTEGER
            }
            else
                exponent = 0;

            copyReal(&RReg[6], &num);
            if(inint64_tRange(&RReg[6]))
                inum = getint64_tReal(&RReg[6]);
            else
                inum = -1;

            // USE THE LEAST PRECISION THAT WILL WORK FOR OUR INTEGER
            Context.precdigits = ((2 * intdigitsReal(&num)) + 7) & ~7;
            if(Context.precdigits < prec)
                Context.precdigits = prec;
            if(Context.precdigits > MAX_USERPRECISION)
                Context.precdigits = MAX_USERPRECISION;

#define     FACTORS_TRIVIALLIMIT    500

            if(inum < 0) {
                // FIRST, REMOVE FIRST FEW TRIVIAL FACTORS
                int32_t k = 2, count;
                RReg[0].exp = 0;
                RReg[0].flags = 0;
                RReg[0].len = 1;
                while(k < FACTORS_TRIVIALLIMIT) {
                    RReg[0].data[0] = k;
                    if(gtReal(&RReg[0], &RReg[6]))
                        break;
                    count = -1;
                    do {
                        divmodReal(&RReg[1], &RReg[2], &RReg[6], &RReg[0]);
                        swapReal(&RReg[6], &RReg[1]);
                        ++count;
                    }
                    while(iszeroReal(&RReg[2]));

                    swapReal(&RReg[6], &RReg[1]);
                    if(count > 0) {
                        rplNewBINTPush(k, DECBINT);
                        rplNewBINTPush(count, DECBINT);
                    }
                    k = nextprimeint32_t(k);
                }

            }
            else {
                int32_t k = 2, count;
                int64_t prev;
                while((k < FACTORS_TRIVIALLIMIT) && (k <= inum)) {
                    count = -1;
                    do {
                        prev = inum;
                        inum /= k;
                        ++count;
                    }
                    while(inum * k == prev);

                    inum = prev;
                    if(count > 0) {
                        rplNewBINTPush(k, DECBINT);
                        rplNewBINTPush(count, DECBINT);
                        if(Exceptions) {
                            DSTop = savestk;
                            Context.precdigits = prec;
                            return;
                        }

                        newRealFromint64_t(&RReg[6], inum, 0);

                    }
                    k = nextprimeint32_t(k);
                }

            }

            // NOW DO POLLARD RHO

            do {

                // EXIT IF THE NUMBER TO FACTOR IS ONE
                RReg[0].data[0] = 1;
                RReg[0].exp = 0;
                RReg[0].flags = 0;
                RReg[0].len = 1;

                if(!gtReal(&RReg[6], &RReg[0]))
                    break;

                onefactor = factorReal(&RReg[7], &RReg[6]);
                if(onefactor < 0) {
                    // CHECK FOR PERFECT SQUARES
                    //hyp_sqrt(&RReg[7]);
                    //ipReal(&RReg[1],&RReg[0],0);
                    //mulReal(&RReg[0],&RReg[1],&RReg[1]);

                    //if(eqReal(&RReg[0],&RReg[7])) swapReal(&RReg[1],&RReg[7]);  // USE THE SQUARE ROOT IF IT'S A PERFECT SQUARE

                    rplNewRealFromRRegPush(7);
                    if(eqReal(&RReg[7], &RReg[6])) {
                        rplPushData((word_p) one_bint);
                        break;
                    }
                }
                else {
                    //int64_t tmp=sqrtint64_t(onefactor);
                    //if(tmp*tmp==onefactor) onefactor=tmp;
                    if(onefactor == 1) {
                        rplNewRealFromRRegPush(6);
                        rplPushData((word_p) one_bint);

                        // AD A FACTOR (-1)^0 = 1 TO INDICATE WE GAVE UP
                        rplPushData((word_p) minusone_bint);
                        rplPushData((word_p) zero_bint);
                        break;  // TERMINATE, WE GAVE UP
                    }
                    rplNewBINTPush(onefactor, DECBINT);
                    if(onefactor == inum) {
                        rplPushData((word_p) one_bint);
                        break;
                    }
                }

                if(Exceptions) {
                    DSTop = savestk;
                    Context.precdigits = prec;
                    return;
                }
                // KEEP FACTORIZING THE NUMBER

                if(inum < 0) {
                    if(onefactor >= 0)
                        newRealFromint64_t(&RReg[7], onefactor, 0);
                    int32_t count = -1;
                    do {
                        divmodReal(&RReg[1], &RReg[0], &RReg[6], &RReg[7]);
                        swapReal(&RReg[1], &RReg[6]);
                        ++count;
                    }
                    while(iszeroReal(&RReg[0]));

                    swapReal(&RReg[1], &RReg[6]);

                    rplNewBINTPush(count, DECBINT);

                    if(inint64_tRange(&RReg[6]))
                        inum = getint64_tReal(&RReg[6]);
                }
                else {
                    int64_t prev;
                    int32_t count = -1;
                    do {
                        prev = inum;
                        inum /= onefactor;
                        ++count;
                    }
                    while(inum * onefactor == prev);

                    inum = prev;
                    rplNewBINTPush(count, DECBINT);

                    newRealFromint64_t(&RReg[6], inum, 0);
                }
                if(Exceptions) {
                    DSTop = savestk;
                    Context.precdigits = prec;
                    return;
                }
            }
            while(1);

            // HERE WE HAVE A LIST OF FACTORS IN THE STACK
            Context.precdigits = prec;
            if(Exceptions) {
                DSTop = savestk;
                return;
            }

            int32_t nfactors = (DSTop - savestk) / 2, f;

            if(nfactors == 0) {
                // SPECIAL CASES 0, 1 ,-1
                DSTop = savestk;
                rplPushData((word_p) one_bint);
                word_p list = rplCreateListN(1, 1, 1);
                if(!list)
                    return;
                rplPushDataNoGrow(list);
                rplPushData(list);
                return;
            }

            // GET THE FACTORS
            for(f = 0; f < nfactors; ++f) {
                rplPushData(savestk[2 * f]);
                if(Exceptions) {
                    DSTop = savestk;
                    return;
                }
            }
            // GET THE MULTIPLICITY
            if(exponent) {
                int32_t factor2done = 0, factor5done = 0;

                rplint32_tToRReg(0, exponent);
                rplint32_tToRReg(1, 2);
                rplint32_tToRReg(2, 5);
                for(f = 0; f < nfactors; ++f) {
                    rplReadNumberAsReal(savestk[1 + 2 * f], &mult);     // READ MULTIPLICITY
                    rplReadNumberAsReal(savestk[2 * f], &num);  // READ FACTOR
                    if(eqReal(&num, &RReg[1]))  // IF FACTOR IS 2
                    {
                        // ADD THE EXPONENT
                        addReal(&RReg[3], &mult, &RReg[0]);
                        factor2done = 1;
                        if(iszeroReal(&RReg[3])) {
                            // WE NEED TO REMOVE THE FACTOR
                            rplRemoveAtData(nfactors, 1);
                            --nfactors;
                        }
                        else
                            rplNewRealFromRRegPush(3);
                    }
                    else if(eqReal(&num, &RReg[2]))     // IF FACTOR IS 5
                    {
                        // ADD THE EXPONENT
                        addReal(&RReg[3], &mult, &RReg[0]);
                        factor5done = 1;
                        if(iszeroReal(&RReg[3])) {
                            // WE NEED TO REMOVE THE FACTOR
                            rplRemoveAtData(nfactors, 1);
                            --nfactors;
                        }
                        else
                            rplNewRealFromRRegPush(3);
                    }
                    else {
                        // OTHER FACTOR, JUST PUSH THE VALUE
                        rplPushData(savestk[1 + 2 * f]);
                    }

                    if(Exceptions) {
                        DSTop = savestk;
                        return;
                    }
                }
                // SEE IF WE NEED TO ADD A FACTOR

                if(!factor2done) {
                    // NEED TO ADD A FACTOR 2 WITH MULTIPLICITY exponent

                    rplExpandStack(2);
                    if(Exceptions) {
                        DSTop = savestk;
                        return;
                    }
                    memmovew(DSTop - nfactors + 1, DSTop - nfactors,
                            nfactors * sizeof(word_p) / sizeof(WORD));
                    ++DSTop;
                    rplOverwriteData(nfactors + 1, (word_p) two_bint); // ADD FACTOR 2
                    rplNewBINTPush(exponent, DECBINT);
                    ++nfactors;
                }
                if(!factor5done) {
                    // NEED TO ADD A FACTOR 2 WITH MULTIPLICITY exponent

                    rplExpandStack(2);
                    if(Exceptions) {
                        DSTop = savestk;
                        return;
                    }
                    memmovew(DSTop - nfactors + 1, DSTop - nfactors,
                            nfactors * sizeof(word_p) / sizeof(WORD));
                    ++DSTop;
                    rplOverwriteData(nfactors + 1, (word_p) five_bint);        // ADD FACTOR 5
                    rplNewBINTPush(exponent, DECBINT);
                    ++nfactors;
                }
            }
            else {
                for(f = 0; f < nfactors; ++f) {
                    rplPushData(savestk[1 + 2 * f]);
                    if(Exceptions) {
                        DSTop = savestk;
                        return;
                    }
                }

            }

            word_p newlist = rplCreateListN(nfactors, 1 + nfactors, 0);
            if(Exceptions) {
                DSTop = savestk;
                return;
            }
            rplPushData(newlist);       // PUSH THE LIST TO PRESERVE IT
            word_p mnewlist = rplCreateListN(nfactors, 2, 0);
            if(Exceptions) {
                DSTop = savestk;
                return;
            }
            newlist = rplPopData();     // RESTORE FROM STACK
            DSTop = savestk;
            if(isneg)
                rplOverwriteData(1, (word_p) minusone_bint);
            else
                rplOverwriteData(1, (word_p) one_bint);

            rplPushDataNoGrow(newlist);
            rplPushData(mnewlist);

            return;

        }
        rplError(ERR_VECTOROFNUMBERSEXPECTED);
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

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, ObjectIDHash=hash, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER, (word_p *) ROMPTR_TABLE, ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID, ObjectIDHash=hash
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((word_p *) ROMPTR_TABLE, ObjectID, ObjectIDHash);
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
        libProbeCmds((char **)LIB_NAMES, (int32_t *) LIB_TOKENINFO,
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
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL int32_t, .42 = HEX INTEGER
        if(ISPROLOG(*ObjectPTR)) {
            TypeInfo = LIBRARY_NUMBER * 100;
            DecompHints = 0;
            RetNum = OK_TOKENINFO | MKTOKENINFO(0, TITYPE_NOTALLOWED, 0, 1);
        }
        else {
            TypeInfo = 0;       // ALL COMMANDS ARE TYPE 0
            DecompHints = 0;
            libGetInfo2(*ObjectPTR, (char **)LIB_NAMES, (int32_t *) LIB_TOKENINFO,
                    LIB_NUMBEROFCMDS);
        }
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
        if(MENUNUMBER(MenuCodeArg) > 5) {
            RetNum = ERR_NOTMINE;
            return;
        }
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR = (word_p) ROMPTR_TABLE[MENUNUMBER(MenuCodeArg) + 2];
        RetNum = OK_CONTINUE;
        return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp, (word_p) LIB_HELPTABLE);
        return;
    }

    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(LibError, (word_p) LIB_MSGTABLE);
        return;
    }

    case OPCODE_LIBINSTALL:
        LibraryList = (word_p) libnumberlist;
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
