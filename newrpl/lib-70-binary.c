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
#define LIBRARY_NUMBER  70

//@TITLE=Bitwise operations

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(STWS,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(RCWS,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(BOR,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(BAND,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(BXOR,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(BLSL,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(BLSR,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(BASR,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(BRL,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(BRR,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(BNOT,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(BADD,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(BSUB,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(BMUL,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(BDIV,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(BNEG,MKTOKENINFO(4,TITYPE_FUNCTION,1,2))

// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"

#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

//INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);

INCLUDE_ROMOBJECT(lib70menu_main);
INCLUDE_ROMOBJECT(lib70_basecycle);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[] = {
    //(WORDPTR)LIB_MSGTABLE,
    (WORDPTR) LIB_HELPTABLE,
    (WORDPTR) lib70menu_main,
    (WORDPTR) lib70_basecycle,
    0
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

    case STWS:
        //@SHORT_DESC=Store current word size in bits (0-63)

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // THIS IS A FLAG NUMBER
        int64_t wsize = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;

        if(wsize < 1)
            wsize = 1;
        if(wsize > 63)
            wsize = 63;

        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);
            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        low64[0] = (low64[0] & ~(0x3f << 4)) | (wsize << 4);

        rplDropData(1);
        return;

    case RCWS:
    {
        //@SHORT_DESC=Recall the currnent word size in bits

        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);
            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        //WORDPTR hi64=SystemFlags+5;

        int32_t wsize = (low64[0] >> 4) & 0x3f;

        rplNewint32_tPush(wsize, DECint32_t);

        return;
    }

    case BOR:
    {
        //@SHORT_DESC=Bitwise OR operation
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            rplListBinaryDoCmd();
            return;
        }

        int64_t num1, num2;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1 = rplReadNumberAsInt64(rplPeekData(2));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;

        num1 |= num2;

        if(num1 & (1LL << wsize)) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << wsize) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(2);
        rplNewint32_tPush(num1, base);
        return;
    }

    case BAND:
    {
        //@SHORT_DESC=Bitwise AND operator
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            rplListBinaryDoCmd();
            return;
        }

        int64_t num1, num2;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1 = rplReadNumberAsInt64(rplPeekData(2));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;

        num1 &= num2;

        if(num1 & (1LL << wsize)) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << wsize) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(2);
        rplNewint32_tPush(num1, base);
        return;
    }

    case BXOR:
    {
        //@SHORT_DESC=Bitwise XOR operation
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            rplListBinaryDoCmd();
            return;
        }

        int64_t num1, num2;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1 = rplReadNumberAsInt64(rplPeekData(2));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;

        num1 ^= num2;

        if(num1 & (1LL << wsize)) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << wsize) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(2);
        rplNewint32_tPush(num1, base);
        return;
    }

    case BLSL:
    {
        //@SHORT_DESC=Bitwise logical shift left
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            rplListBinaryDoCmd();
            return;
        }

        int64_t num1, num2;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1 = rplReadNumberAsInt64(rplPeekData(2));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;

        if((num2 > wsize) || (num2 < -wsize))
            num1 = 0;
        else if(num2 > 0)
            num1 <<= num2;
        else
            num1 = (int64_t) (((uint64_t) num1) >> (-num2));

        if(num1 & (1LL << wsize)) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << wsize) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(2);
        rplNewint32_tPush(num1, base);
        return;
    }

    case BLSR:
    {
        //@SHORT_DESC=Bitwise logical shift right
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            rplListBinaryDoCmd();
            return;
        }

        int64_t num1, num2;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1 = rplReadNumberAsInt64(rplPeekData(2));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);
            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;
        uint64_t wmask = (1ULL << wsize) - 1;

        num1 &= wmask | (1ULL << wsize);

        if((num2 > wsize) || (num2 < -wsize))
            num1 = 0;
        else if(num2 > 0)
            num1 = (int64_t) (((uint64_t) num1) >> num2);
        else
            num1 <<= -num2;

        if(num1 & (1LL << wsize)) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << wsize) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(2);
        rplNewint32_tPush(num1, base);
        return;
    }

    case BASR:
    {
        //@SHORT_DESC=Bitwise arithmetic shift right
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            rplListBinaryDoCmd();
            return;
        }

        int64_t num1, num2;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1 = rplReadNumberAsInt64(rplPeekData(2));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;

        if(num2 > 0)
            num1 >>= num2;
        else
            num1 <<= -num2;

        if(num1 & (1LL << (wsize - num2))) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << (wsize - num2)) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(2);
        rplNewint32_tPush(num1, base);
        return;
    }

    case BADD:
    {
        //@SHORT_DESC=Bitwise addition with overflow
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            rplListBinaryDoCmd();
            return;
        }

        int64_t num1, num2;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1 = rplReadNumberAsInt64(rplPeekData(2));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;

        num1 += num2;

        if(num1 & (1LL << wsize)) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << wsize) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(2);
        rplNewint32_tPush(num1, base);
        return;
    }

    case BSUB:
    {
        //@SHORT_DESC=Bitwise subtraction with overflow
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            rplListBinaryDoCmd();
            return;
        }

        int64_t num1, num2;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1 = rplReadNumberAsInt64(rplPeekData(2));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;

        num1 -= num2;

        if(num1 & (1LL << wsize)) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << wsize) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(2);
        rplNewint32_tPush(num1, base);
        return;
    }

    case BMUL:
    {
        //@SHORT_DESC=Bitwise multiplication
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            rplListBinaryDoCmd();
            return;
        }

        int64_t num1, num2;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1 = rplReadNumberAsInt64(rplPeekData(2));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;

        num1 *= num2;

        if(num1 & (1LL << wsize)) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << wsize) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(2);
        rplNewint32_tPush(num1, base);
        return;
    }

    case BDIV:
    {
        //@SHORT_DESC=Bitwise integer division
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            rplListBinaryDoCmd();
            return;
        }

        int64_t num1, num2;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1 = rplReadNumberAsInt64(rplPeekData(2));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;

        if(num2 == 0) {
            if(num1 != 0) {
                rplInfinityToRReg(0);
                if(rplTestSystemFlag(FL_COMPLEXMODE)) {
                    RReg[0].flags |= F_UNDINFINITY;
                }
                else if(num1 < 0)
                    RReg[0].flags |= F_NEGATIVE;
            }
            else
                rplNANToRReg(0);

            rplDropData(2);
            rplNewRealFromRRegPush(0);
            rplCheckResultAndError(&RReg[0]);
            return;
        }

        num1 /= num2;

        if(num1 & (1LL << wsize)) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << wsize) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(2);
        rplNewint32_tPush(num1, base);
        return;
    }

    case BRL:
    {
        //@SHORT_DESC=Bitwise rotate left
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            rplListBinaryDoCmd();
            return;
        }

        int64_t num1, num2;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1 = rplReadNumberAsInt64(rplPeekData(2));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;

        num2 %= wsize + 1;
        if(num2 < 0)
            num2 += wsize + 1;

        int64_t left, right;

        left = (num1 << num2);
        right = (num1 >> (wsize + 1 - num2)) & ((1LL << num2) - 1);

        num1 = left | right;

        if(num1 & (1LL << wsize)) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << wsize) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(2);
        rplNewint32_tPush(num1, base);
        return;
    }

    case BRR:
    {
        //@SHORT_DESC=Bitwise rotate right
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(2);

        if(ISLIST(*rplPeekData(2)) || ISLIST(*rplPeekData(1))) {
            rplListBinaryDoCmd();
            return;
        }

        int64_t num1, num2;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num2 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(ISNUMBER(*rplPeekData(2))) {
            num1 = rplReadNumberAsInt64(rplPeekData(2));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(2));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;

        num2 = -num2;
        num2 %= wsize + 1;
        if(num2 < 0)
            num2 += wsize + 1;

        int64_t left, right;

        left = (num1 << num2);
        right = (num1 >> (wsize + 1 - num2)) & ((1LL << num2) - 1);

        num1 = left | right;

        if(num1 & (1LL << wsize)) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << wsize) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(2);
        rplNewint32_tPush(num1, base);
        return;
    }

    case BNOT:
    {
        //@SHORT_DESC=Bitwise inversion of bits
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryDoCmd();
            return;
        }

        int64_t num1;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num1 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(1));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;

        num1 = ~num1;

        if(num1 & (1LL << wsize)) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << wsize) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(1);
        rplNewint32_tPush(num1, base);
        return;
    }

    case BNEG:
    {
        //@SHORT_DESC=Bitwise negation
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplStripTagStack(1);

        int64_t num1;
        int32_t base;
        if(ISNUMBER(*rplPeekData(1))) {
            num1 = rplReadNumberAsInt64(rplPeekData(1));
            if(Exceptions)
                return;
            base = LIBNUM(*rplPeekData(1));
        }
        else {
            rplError(ERR_INTEGEREXPECTED);

            return;
        }
        if(!ISBINDATA(*SystemFlags)) {
            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
            rplError(ERR_SYSTEMFLAGSINVALID);

            return;
        }
        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64 = SystemFlags + 1;
        int32_t wsize = (low64[0] >> 4) & 0x3f;

        num1 = -num1;

        if(num1 & (1LL << wsize)) {
            // SIGN EXTEND THE RESULT
            num1 |= ~((1LL << wsize) - 1);
        }
        else
            num1 &= ((1LL << wsize) - 1);

        rplDropData(1);
        rplNewint32_tPush(num1, base);
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
        if(MENUNUMBER(MenuCodeArg) > 0) {
            RetNum = ERR_NOTMINE;
            return;
        }
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR = ROMPTR_TABLE[MENUNUMBER(MenuCodeArg) + 1];
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
