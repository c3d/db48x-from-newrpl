/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// LIBRARY ONE DEFINES THE BASIC TYPES BINT AND SINT

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
#define LIBRARY_NUMBER  12

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

//#define COMMAND_LIST
//    CMD(STRIPCOMMENTS,MKTOKENINFO(13,TITYPE_NOTALLOWED,1,2))
//    ECMD(CMDNAME,"CMDNAME",MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE

#define ERROR_LIST \
    ERR(INTEGEREXPECTED,0), \
    ERR(INTEGERSNOTSUPPORTED,1)

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS \
            DECBINT,BINBINT,OCTBINT,HEXBINT, \
            DECBINT|APPROX_BIT,BINBINT|APPROX_BIT, \
            OCTBINT|APPROX_BIT,HEXBINT|APPROX_BIT

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"

#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

// MACRO TO GET NUMBER OF BITS IN THE BASE
// 1= BINARY, 3=OCTAL, 4=HEX, AND 2=DECIMAL

#define GETBASE(libnum) (((libnum)-(BINBINT-2))>>1)

#define LIBFROMBASE(base) ((base<<1)+(BINBINT-2))

const uint64_t const powersof10[20] = {
    1000000000000000000LL,
    100000000000000000LL,
    10000000000000000LL,
    1000000000000000LL,
    100000000000000LL,
    10000000000000LL,
    1000000000000LL,
    100000000000LL,
    10000000000LL,
    1000000000LL,
    100000000LL,
    10000000LL,
    1000000LL,
    100000LL,
    10000LL,
    1000LL,
    100LL,
    10LL,
    1LL,
    0LL
};

// INTERNAL SINT OBJECTS
ROMOBJECT zero_bint[] = {
    (WORD) MAKESINT(0)
};

ROMOBJECT one_bint[] = {
    (WORD) MAKESINT(1)
};

ROMOBJECT minusone_bint[] = {
    (WORD) MAKESINT(-1)
};

ROMOBJECT two_bint[] = {
    (WORD) MAKESINT(2)
};

ROMOBJECT three_bint[] = {
    (WORD) MAKESINT(3)
};

ROMOBJECT four_bint[] = {
    (WORD) MAKESINT(4)
};

ROMOBJECT five_bint[] = {
    (WORD) MAKESINT(5)
};

ROMOBJECT six_bint[] = {
    (WORD) MAKESINT(6)
};

ROMOBJECT seven_bint[] = {
    (WORD) MAKESINT(7)
};

ROMOBJECT eight_bint[] = {
    (WORD) MAKESINT(8)
};

ROMOBJECT nine_bint[] = {
    (WORD) MAKESINT(9)
};

ROMOBJECT ten_bint[] = {
    (WORD) MAKESINT(10)
};

ROMOBJECT eleven_bint[] = {
    (WORD) MAKESINT(11)
};

ROMOBJECT twelve_bint[] = {
    (WORD) MAKESINT(12)
};

ROMOBJECT thirteen_bint[] = {
    (WORD) MAKESINT(13)
};

ROMOBJECT fourteen_bint[] = {
    (WORD) MAKESINT(14)
};

ROMOBJECT fifteen_bint[] = {
    (WORD) MAKESINT(15)
};

INCLUDE_ROMOBJECT(LIB_MSGTABLE);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[] = {
    (WORDPTR) LIB_MSGTABLE,
    (WORDPTR) zero_bint,
    (WORDPTR) one_bint,
    (WORDPTR) two_bint,
    (WORDPTR) three_bint,
    (WORDPTR) minusone_bint,
    (WORDPTR) ten_bint,
    (WORDPTR) four_bint,
    (WORDPTR) five_bint,
    (WORDPTR) six_bint,
    (WORDPTR) seven_bint,
    (WORDPTR) eight_bint,
    (WORDPTR) nine_bint,
    (WORDPTR) eleven_bint,
    (WORDPTR) twelve_bint,
    (WORDPTR) thirteen_bint,
    (WORDPTR) fourteen_bint,
    (WORDPTR) fifteen_bint,

    0
};

const char const alldigits[] = "0123456789ABCDEF";

WORDPTR rplNewSINT(int num, int base)
{
    WORDPTR obj;
    obj = rplAllocTempOb(0);
    if(!obj)
        return NULL;
    *obj = MKOPCODE(base, num & 0x3ffff);
    return obj;
}

void rplNewSINTPush(int num, int base)
{
    WORDPTR obj;
    obj = rplAllocTempOb(0);
    if(!obj)
        return;
    *obj = MKOPCODE(base, num & 0x3ffff);
    rplPushData(obj);

}

WORDPTR rplNewBINT(int64_t num, int base)
{
    WORDPTR obj;

    if((num >= MIN_SINT) && (num <= MAX_SINT)) {
        obj = rplAllocTempOb(0);
        if(!obj)
            return NULL;
        *obj = MKOPCODE(base, num & 0x3ffff);
    }
    else {
        obj = rplAllocTempOb(2);
        if(!obj)
            return NULL;

        obj[0] = (MKPROLOG(base, 2));
        obj[1] = ((WORD) (num & 0xffffffff));   // CAREFUL: THIS IS FOR LITTLE ENDIAN SYSTEMS ONLY!
        obj[2] = ((WORD) ((num >> 32) & 0xffffffff));
    }

    return obj;
}

// WRITE AN INTEGER TO THE GIVEN DESTINATION. RETURN A POINTER AFTER THE LAST WRITTEN WORD
WORDPTR rplWriteBINT(int64_t num, int base, WORDPTR dest)
{

    if((num >= MIN_SINT) && (num <= MAX_SINT)) {
        *dest = MKOPCODE(base, num & 0x3ffff);
        return ++dest;
    }
    else {
        dest[0] = (MKPROLOG(base, 2));
        dest[1] = ((WORD) (num & 0xffffffff));  // CAREFUL: THIS IS FOR LITTLE ENDIAN SYSTEMS ONLY!
        dest[2] = ((WORD) ((num >> 32) & 0xffffffff));
        return dest + 3;
    }

}

// WRITE AN INTEGER TO THE GIVEN DESTINATION. RETURN A POINTER AFTER THE LAST WRITTEN WORD
void rplCompileBINT(int64_t num, int base)
{

    if((num >= MIN_SINT) && (num <= MAX_SINT)) {
        rplCompileAppend(MKOPCODE(base, num & 0x3ffff));
    }
    else {
        rplCompileAppend(MKPROLOG(base, 2));
        rplCompileAppend((WORD) (num & 0xffffffff));    // CAREFUL: THIS IS FOR LITTLE ENDIAN SYSTEMS ONLY!
        rplCompileAppend((WORD) ((num >> 32) & 0xffffffff));
    }
}

void rplNewBINTPush(int64_t num, int base)
{
    WORDPTR obj;

    if((num >= MIN_SINT) && (num <= MAX_SINT)) {
        obj = rplAllocTempOb(0);
        if(!obj)
            return;
        *obj = MKOPCODE(base, num & 0x3ffff);
    }
    else {
        obj = rplAllocTempOb(2);
        if(!obj)
            return;

        obj[0] = (MKPROLOG(base, 2));
        obj[1] = ((WORD) (num & 0xffffffff));   // CAREFUL: THIS IS FOR LITTLE ENDIAN SYSTEMS ONLY!
        obj[2] = ((WORD) ((num >> 32) & 0xffffffff));
    }

    rplPushData(obj);
}

int64_t rplReadBINT(WORDPTR ptr)
{
    int64_t result;
    if(ISPROLOG(*ptr))
        // THERE'S A PAYLOAD, READ THE NUMBER
        result = *((int64_t *) (ptr + 1));
    else {
        result = OPCODE(*ptr);
        if(result & 0x20000)
            result |= 0xFFFFFFFFFFFc0000;       // SIGN EXTEND
    }
    return result;
}

// A FEW FUNCTIONS THAT DEAL WITH TRUE/FALSE

void rplPushFalse()
{
    rplPushDataNoGrow((WORDPTR) zero_bint);
}

void rplPushTrue()
{
    rplPushDataNoGrow((WORDPTR) one_bint);

}

BINT rplIsFalse(WORDPTR objptr)
{
    objptr = rplConstant2Number(objptr);

    if(ISANGLE(*objptr))
        ++objptr;       // POINT TO THE NUMBER INSIDE THE ANGLE

    if(IS_FALSE(*objptr))
        return 1;
    if(ISBINT(*objptr)) {
        if(rplReadBINT(objptr) == 0)
            return 1;
        return 0;
    }
    if(ISREAL(*objptr)) {
        REAL dec;
        rplReadReal(objptr, &dec);
        if(iszeroReal(&dec))
            return 1;
        return 0;
    }

    if(ISCOMPLEX(*objptr)) {
        REAL re, im;
        BINT angmode;

        rplReadCNumber(objptr, &re, &im, &angmode);
        return rplIsZeroComplex(&re, &im, angmode);
    }

    return 0;
}

BINT rplIsTrue(WORDPTR objptr)
{
    return rplIsFalse(objptr) ^ 1;
}

BINT rplIsNegative(WORDPTR objptr)
{
    objptr = rplConstant2Number(objptr);
    if(ISANGLE(*objptr))
        ++objptr;       // POINT TO THE NUMBER INSIDE THE ANGLE

    if(ISBINT(*objptr)) {
        if(rplReadBINT(objptr) < 0)
            return 1;
        return 0;
    }
    if(ISREAL(*objptr)) {
        REAL dec;
        rplReadReal(objptr, &dec);
        if(dec.flags & F_NEGATIVE)
            return 1;
        return 0;
    }

    if(ISCOMPLEX(*objptr)) {
        REAL re, im;
        BINT angmode;

        rplReadCNumber(objptr, &re, &im, &angmode);
        if(re.flags & F_NEGATIVE)
            return 1;
        return 0;
    }

    return 0;
}

// READS A SINT, BINT OR REAL INTO A REAL NUMBER REGISTER
void rplNumberToRReg(int num, WORDPTR number)
{
    number = rplConstant2Number(number);
    if(ISREAL(*number))
        rplCopyRealToRReg(num, number);
    else if(ISBINT(*number))
        rplBINTToRReg(num, rplReadBINT(number));
    else {
        rplError(ERR_REALEXPECTED);
    }
}

// READ A SINT, BINT OR REAL AS A 64-BIT INTEGER
// ROUNDING A REAL IS BY TRUNCATION
// DOES CHECK FOR OVERFLOW!

int64_t rplReadNumberAsBINT(WORDPTR number)
{
    int64_t value;
  readnumberbint_recheck:
    number = rplConstant2Number(number);

    if(ISANGLE(*number))
        ++number;
    if(ISREAL(*number)) {
        REAL dec;
        rplReadReal(number, &dec);
        if(!inint64_tRange(&dec)) {
            rplError(ERR_NUMBERTOOBIG);
            return 0;
        }
        value = getint64_tReal(&dec);
        return value;
    }
    else if(ISBINT(*number))
        return rplReadBINT(number);
    else {
        if(ISTAG(*number)) {
            number = rplStripTag(number);
            goto readnumberbint_recheck;
        }
        rplError(ERR_REALEXPECTED);
    }
    return 0;
}

// READ A NUMBER AS A REAL
// CAREFUL!
// dec SHOULD BE UNINITIALIZED (WITH NO STORAGE ALLOCATED)
// DO **NOT** USE WITH RREG REGISTERS OR DATA CORRUPTION MIGHT OCCUR!!
// TEMPORARY DATA STORAGE FOR UP TO 4 NUMBERS
// IF CALLED MORE THAN 4 TIMES IT MIGHT OVERWRITE THE PREVIOUS

void rplReadNumberAsReal(WORDPTR number, REAL * dec)
{
  readnumber_recheck:
    number = rplConstant2Number(number);

    if(ISANGLE(*number))
        ++number;
    if(ISREAL(*number))
        rplReadReal(number, dec);
    else if(ISBINT(*number)) {
        // PROVIDE STORAGE
        dec->data = RDigits + BINT2RealIdx * BINT_REGISTER_STORAGE;
        newRealFromint64_t(dec, rplReadBINT(number), 0);
        if(ISAPPROX(*number))
            dec->flags |= F_APPROX;
        ++BINT2RealIdx;
        if(BINT2RealIdx >= BINT2REAL)
            BINT2RealIdx = 0;
    }
    else {
        if(ISTAG(*number)) {
            number = rplStripTag(number);
            goto readnumber_recheck;
        }
        rplError(ERR_REALEXPECTED);
    }

}

// READ A NUMBER AS A REAL
// CAREFUL!
// dec SHOULD BE UNINITIALIZED (WITH NO STORAGE ALLOCATED)
// DO **NOT** USE WITH RREG REGISTERS OR DATA CORRUPTION MIGHT OCCUR!!
// TEMPORARY DATA STORAGE FOR UP TO 4 NUMBERS
// IF CALLED MORE THAN 4 TIMES IT MIGHT OVERWRITE THE PREVIOUS

void rplLoadBINTAsReal(int64_t number, REAL * dec)
{
    // PROVIDE STORAGE
    dec->data = RDigits + BINT2RealIdx * BINT_REGISTER_STORAGE;
    newRealFromint64_t(dec, number, 0);
    ++BINT2RealIdx;
    if(BINT2RealIdx >= BINT2REAL)
        BINT2RealIdx = 0;
}

// COUNT THE NUMBER OF BITS IN A POSITIVE INTEGER
static int rpl_log2(int64_t number, int bits)
{
    static const unsigned char const log2_table[16] =
            { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };
    if(bits <= 4)
        return log2_table[number];
    bits >>= 1;
    if(number >> bits)
        return rpl_log2(number >> bits, bits) + bits;
    return rpl_log2(number, bits);
}

// CONVERT TO STRING AND RETURN THE NUMBER OF BYTES OUTPUT
BINT rplIntToString(int64_t number, BINT base, BYTEPTR buffer, BYTEPTR endbuffer)
{

    base -= DOBINT;

    if(base == 2) {
        // THIS IS A BASE-10 NUMBER
        // CONVERT TO STRING

        REAL realnum;

        BINT sign;

        NUMFORMAT fmt;

        rplGetSystemNumberFormat(&fmt);

        rplLoadBINTAsReal(number, &realnum);

        sign = realnum.flags & F_NEGATIVE;

        realnum.flags ^= sign;

        realnum.flags ^= sign;

        // ESTIMATE THE MAXIMUM STRING LENGTH AND RESERVE THE MEMORY

        BYTEPTR string;

        BINT len = formatlengthReal(&realnum, 0, fmt.Locale);

        if(len + 1 > endbuffer - buffer)
            return 0;

        // NOW USE IT
        string = (BYTEPTR) buffer;
        return (BYTEPTR) formatReal(&realnum, (char *)string, 0,
                fmt.Locale) - string;

    }
    else {
        // THIS IS A BINARY, OCTAL OR HEXA NUMBER
        // base HAS THE NUMBER OF BITS PER DIGIT
        BYTEPTR ptr = buffer;
        uint64_t unumber;
        BINT digit, neg;

        if(number < 0) {
            *ptr++ = '-';
            if(ptr >= endbuffer)
                return 0;
            unumber = -number;
        }
        else
            unumber = number;

        *ptr++ = '#';
        if(ptr >= endbuffer)
            return 0;

        if(base >= 3)
            digit = 60;
        else
            digit = 62;

        neg = (1 << base) - 1;  // CREATE A MASK TO ISOLATE THE DIGIT

        // SKIP ALL LEADING ZEROS
        while(digit > 0) {
            if((unumber >> digit) & neg)
                break;
            digit -= base;
        }
        // NOW DECOMPILE THE NUMBER
        while(digit >= 0) {
            *ptr++ = alldigits[(unumber >> digit) & neg];
            if(ptr >= endbuffer)
                return 0;
            digit -= base;
        }

        // ADD BASE CHARACTER
        if(base == 1)
            *ptr++ = 'b';
        if(base == 3)
            *ptr++ = 'o';
        if(base == 4)
            *ptr++ = 'h';

        return ptr - buffer;

    }

}

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // NORMAL BEHAVIOR FOR A BINT IS TO PUSH THE OBJECT ON THE STACK:
        rplPushData(IPtr);
        return;
    }

    if(LIBNUM(CurOpcode) == LIB_OVERLOADABLE) {
        // THESE ARE OVERLOADABLE COMMANDS DISPATCHED FROM THE
        // OVERLOADABLE OPERATORS LIBRARY.

        // PROVIDE BEHAVIOR FOR OVERLOADABLE OPERATORS HERE
        int64_t op1 = 0, op2 = 0;
        REAL rop1, rop2;
        int op1type = 0, op2type = 0;
        int op1app = 0, op2app = 0;
        int op1base = DECBINT;

        // USE GC-SAFE POINTERS, NEVER LOCAL COPIES OF POINTERS INTO TEMPOB
#define arg1 ScratchPointer1
#define arg2 ScratchPointer2

        int nargs = OVR_GETNARGS(CurOpcode);

        if(rplDepthData() < nargs) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(nargs == 1) {
            // UNARY OPERATORS
            arg1 = rplPeekData(1);
            if(!ISBINT(*arg1)) {
                rplError(ERR_INTEGEREXPECTED);
                return;
            }
            op1 = rplReadBINT(arg1);
            rplDropData(1);

        }
        else {
            arg1 = rplPeekData(2);
            arg2 = rplPeekData(1);

            if(ISREAL(*arg1)) {
                rplReadReal(arg1, &rop1);
                op1type = 1;
                op1app = rop1.flags & F_APPROX;
            }
            else {
                if(ISBINT(*arg1)) {
                    op1 = rplReadBINT(arg1);
                    op1type = 0;
                    op1app = ISAPPROX(*arg1);
                    op1base = LIBNUM(*arg1) & ~APPROX_BIT;
                }
                else {
                    op1 = 0;
                    op1type = -1;
                    op1app = 0;
                }
            }
            if(ISREAL(*arg2)) {
                rplReadReal(arg2, &rop2);
                op2type = 1;
                op1app = rop2.flags & F_APPROX;
            }
            else {
                if(ISBINT(*arg2)) {
                    op2 = rplReadBINT(arg2);
                    op2type = 0;
                    op2app = ISAPPROX(*arg2);
                }
                else {
                    op2 = 0;
                    op2type = -1;
                    op2app = 0;
                }
            }

            if((op1type < 0) || (op2type < 0)) {
                switch (OPCODE(CurOpcode)) {
                case OVR_SAME:
                case OVR_EQ:
                    rplDropData(2);
                    rplPushFalse();
                    break;
                case OVR_NOTEQ:
                    rplDropData(2);
                    rplPushTrue();
                    break;
                default:
                    rplError(ERR_REALEXPECTED);
                }
                return;
            }

            rplDropData(2);
        }

        switch (OPCODE(CurOpcode)) {
        case OVR_ADD:
        {
            // ADD TWO NUMBERS FROM THE STACK
            if(op1type || op2type) {
                if(op1type) {
                    rplBINTToRReg(1, op2);
                    addReal(&RReg[0], &rop1, &RReg[1]);

                    if(op1app || op2app)
                        RReg[0].flags |= F_APPROX;
                }

                if(op2type) {
                    // TODO: TRY TO RESPECT THE NUMBER TYPE OF THE FIRST ARGUMENT
                    rplBINTToRReg(1, op1);

                    addReal(&RReg[0], &RReg[1], &rop2);
                    if(op1base != DECBINT) {
                        if(isintegerReal(&RReg[0]) && inint64_tRange(&RReg[0])) {
                            rplNewBINTPush(getint64_tReal(&RReg[0]),
                                    op1base | ((op1app
                                            || op2app) ? APPROX_BIT : 0));
                            if(!Exceptions)
                                rplCheckResultAndError(&RReg[0]);
                            return;
                        }

                    }
                    if(op1app || op2app)
                        RReg[0].flags |= F_APPROX;
                }

                rplNewRealFromRRegPush(0);
                if(!Exceptions)
                    rplCheckResultAndError(&RReg[0]);

                return;
            }

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
                // CONVERT BOTH TO REALS
                rplBINTToRReg(1, op1);
                rplBINTToRReg(2, op2);

                addReal(&RReg[0], &RReg[1], &RReg[2]);

                if(op1app || op2app)
                    RReg[0].flags |= F_APPROX;

                rplNewRealFromRRegPush(0);
                if(!Exceptions)
                    rplCheckResultAndError(&RReg[0]);

                return;
            }
            rplNewBINTPush(op1 + op2,
                    LIBNUM(*arg1) | (LIBNUM(*arg2) & APPROX_BIT));
            return;
        }
        case OVR_SUB:
        {
            if(op1type || op2type) {
                if(op1type) {
                    rplBINTToRReg(1, -op2);

                    addReal(&RReg[0], &rop1, &RReg[1]);
                    if(op1app || op2app)
                        RReg[0].flags |= F_APPROX;

                }

                if(op2type) {
                    rplBINTToRReg(1, op1);

                    subReal(&RReg[0], &RReg[1], &rop2);

                    if(op1base != DECBINT) {
                        if(isintegerReal(&RReg[0]) && inint64_tRange(&RReg[0])) {
                            rplNewBINTPush(getint64_tReal(&RReg[0]),
                                    op1base | ((op1app
                                            || op2app) ? APPROX_BIT : 0));
                            if(!Exceptions)
                                rplCheckResultAndError(&RReg[0]);
                            return;
                        }

                    }

                    if(op1app || op2app)
                        RReg[0].flags |= F_APPROX;

                }
                rplNewRealFromRRegPush(0);
                if(!Exceptions)
                    rplCheckResultAndError(&RReg[0]);

                return;
            }

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
                // CONVERT BOTH TO REALS
                rplBINTToRReg(1, op1);
                rplBINTToRReg(2, op2);

                subReal(&RReg[0], &RReg[1], &RReg[2]);

                if(op1app || op2app)
                    RReg[0].flags |= F_APPROX;

                rplNewRealFromRRegPush(0);
                if(!Exceptions)
                    rplCheckResultAndError(&RReg[0]);

                return;
            }
            rplNewBINTPush(op1 - op2,
                    LIBNUM(*arg1) | (LIBNUM(*arg2) & APPROX_BIT));
            return;
        }
        case OVR_MUL:
            if(op1type || op2type) {
                if(op1type) {
                    rplBINTToRReg(1, op2);

                    mulReal(&RReg[0], &rop1, &RReg[1]);
                    if(op1app || op2app)
                        RReg[0].flags |= F_APPROX;

                }

                if(op2type) {
                    // TODO: TRY TO RESPECT THE NUMBER TYPE OF THE FIRST ARGUMENT
                    rplBINTToRReg(1, op1);

                    mulReal(&RReg[0], &RReg[1], &rop2);

                    if(op1base != DECBINT) {
                        if(isintegerReal(&RReg[0]) && inint64_tRange(&RReg[0])) {
                            rplNewBINTPush(getint64_tReal(&RReg[0]),
                                    op1base | ((op1app
                                            || op2app) ? APPROX_BIT : 0));
                            if(!Exceptions)
                                rplCheckResultAndError(&RReg[0]);
                            return;
                        }

                    }

                    if(op1app || op2app)
                        RReg[0].flags |= F_APPROX;

                }
                rplNewRealFromRRegPush(0);
                if(!Exceptions)
                    rplCheckResultAndError(&RReg[0]);

                return;
            }
            // DETECT OVERFLOW, AND CONVERT TO REALS IF SO

            // O1*O2 > 2^63 --> LOG2(O1)+LOG2(O2) > LOG2(2^63)
            // LOG2(O1)+LOG2(O2) > 63 MEANS OVERFLOW
            BINT sign1 = (op1 < 0) ^ (op2 < 0);

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
                    op1 *= op2;
                    if(sign1)
                        rplNewBINTPush(-op1,
                                LIBNUM(*arg1) | (LIBNUM(*arg2) & APPROX_BIT));
                    else
                        rplNewBINTPush(op1,
                                LIBNUM(*arg1) | (LIBNUM(*arg2) & APPROX_BIT));
                    return;
                }
            }

            rplBINTToRReg(1, op1);
            rplBINTToRReg(2, op2);

            mulReal(&RReg[0], &RReg[1], &RReg[2]);

            if(op1app || op2app)
                RReg[0].flags |= F_APPROX;

            rplNewRealFromRRegPush(0);
            if(!Exceptions)
                rplCheckResultAndError(&RReg[0]);

            return;

        case OVR_DIV:
        {
            if(op1type || op2type) {
                if(op1type) {
                    rplBINTToRReg(1, op2);

                    divReal(&RReg[0], &rop1, &RReg[1]);

                    if(rplTestSystemFlag(FL_COMPLEXMODE)) {
                        if(op2 == 0 && !iszeroReal(&rop1))
                            RReg[0].flags |= F_UNDINFINITY;
                    }
                    if(op1app || op2app)
                        RReg[0].flags |= F_APPROX;

                }

                if(op2type) {
                    // TODO: TRY TO RESPECT THE NUMBER TYPE OF THE FIRST ARGUMENT
                    rplBINTToRReg(1, op1);

                    divReal(&RReg[0], &RReg[1], &rop2);

                    if(rplTestSystemFlag(FL_COMPLEXMODE)) {
                        if(iszeroReal(&rop2) && op1 != 0)
                            RReg[0].flags |= F_UNDINFINITY;
                    }

                    if(op1base != DECBINT) {
                        if(isintegerReal(&RReg[0]) && inint64_tRange(&RReg[0])) {
                            rplNewBINTPush(getint64_tReal(&RReg[0]),
                                    op1base | ((op1app
                                            || op2app) ? APPROX_BIT : 0));
                            if(!Exceptions)
                                rplCheckResultAndError(&RReg[0]);
                            return;
                        }

                    }

                    if(op1app || op2app)
                        RReg[0].flags |= F_APPROX;

                }

                rplNewRealFromRRegPush(0);
                if(!Exceptions)
                    rplCheckResultAndError(&RReg[0]);

                return;
            }
            rplBINTToRReg(1, op1);
            rplBINTToRReg(2, op2);

            divReal(&RReg[0], &RReg[1], &RReg[2]);

            if(rplTestSystemFlag(FL_COMPLEXMODE)) {
                if(op2 == 0 && op1 != 0)
                    RReg[0].flags |= F_UNDINFINITY;
            }

            if(op1app || op2app)
                RReg[0].flags |= F_APPROX;

            if(!(isintegerReal(&RReg[0]) && inint64_tRange(&RReg[0]))) {
                rplNewRealFromRRegPush(0);
            }
            else {
                int64_t result = getint64_tReal(&RReg[0]);
                rplNewBINTPush(result,
                        LIBNUM(*arg1) | (LIBNUM(*arg2) & APPROX_BIT));
            }
            if(!Exceptions)
                rplCheckResultAndError(&RReg[0]);
            return;
        }

        case OVR_POW:
        {
            if(op1type || op2type) {

                if(op1type) {
                    // INTEGER POWER OF A REAL NUMBER
                    BINT isneg = op2 & 1;
                    if(rop1.flags & F_NEGATIVE)
                        rop1.flags ^= F_NEGATIVE;
                    else
                        isneg = 0;

                    rplBINTToRReg(1, op2);
                    powReal(&RReg[0], &rop1, &RReg[1]);

                    if(isneg)
                        RReg[0].flags |= F_NEGATIVE;

                }

                if(op2type) {

                    if(op1 < 0) {
                        // REAL POWER OF NEGATIVE INTEGER, RESULT IS IN GENERAL A COMPLEX NUMBER

                        // NEGATIVE NUMBER RAISED TO A REAL POWER
                        // a^n= ABS(a)^n * (cos(n*pi)+i*sin(n*pi))

                        // USE DEG TO AVOID LOSS OF PRECISION WITH PI

                        newRealFromBINT(&RReg[7], 180, 0);

                        mulReal(&RReg[0], &rop2, &RReg[7]);
                        divmodReal(&RReg[1], &RReg[9], &RReg[0], &RReg[7]);     // REDUCE TO FIRST CIRCLE

                        if(!rplTestSystemFlag(FL_COMPLEXMODE)
                                && !iszeroReal(&RReg[9])) {
                            rplError(ERR_COMPLEXRESULT);
                            return;
                        }

                        if(isoddReal(&RReg[1])) subReal(&RReg[9],&RReg[9],&RReg[7]);


                        rplBINTToRReg(6, -op1);

                        powReal(&RReg[8], &RReg[6], &rop2);     // ONLY RReg[9] IS PRESERVED

                        BINT angmode =
                                rplTestSystemFlag(FL_ANGLEMODE1) |
                                (rplTestSystemFlag(FL_ANGLEMODE2) << 1);

                        trig_convertangle(&RReg[9], ANGLEDEG, angmode);

                        rplCheckResultAndError(&RReg[8]);
                        rplCheckResultAndError(&RReg[0]);
                        // RETURN A POLAR COMPLEX
                        rplNewComplexPush(&RReg[8], &RReg[0], angmode);

                        return;

                    }

                    rplBINTToRReg(1, op1);
                    powReal(&RReg[0], &RReg[1], &rop2);

                }
                rplNewRealFromRRegPush(0);
                if(!Exceptions)
                    rplCheckResultAndError(&RReg[0]);

                return;
            }

            // INTEGER POWER, USE REALS TO DEAL WITH NEGATIVE POWERS AND OVERFLOW
            BINT isneg = ((op1 < 0) ? (op2 & 1) : 0);
            if(op1 < 0)
                op1 = -op1;
            rplBINTToRReg(1, op1);
            rplBINTToRReg(2, op2);

            powReal(&RReg[0], &RReg[1], &RReg[2]);

            if(isneg)
                RReg[0].flags |= F_NEGATIVE;

            if(isintegerReal(&RReg[0]) && inint64_tRange(&RReg[0])) {
                int64_t result = getint64_tReal(&RReg[0]);
                rplNewBINTPush(result,
                        LIBNUM(*arg1) | (LIBNUM(*arg2) & APPROX_BIT));
            }
            else
                rplNewRealFromRRegPush(0);
            if(!Exceptions)
                rplCheckResultAndError(&RReg[0]);

            return;
        }

        case OVR_XROOT:
        {
            if(op1type)
                copyReal(&RReg[6], &rop1);
            else
                rplBINTToRReg(6, op1);
            if(op1app)
                RReg[6].flags |= F_APPROX;
            if(op2type)
                copyReal(&RReg[7], &rop2);
            else
                rplBINTToRReg(7, op2);
            if(op2app)
                RReg[7].flags |= F_APPROX;

            if(iszeroReal(&RReg[7])) {
                // 0TH ROOTS OF ALL REALS ARE UNDEFINED
                rplOneToRReg(1);
                rplNANToRReg(0);
                rplNewRealFromRRegPush(0);

                if(eqReal(&RReg[6], &RReg[1])) {
                    rplError(ERR_UNDEFINEDRESULT);
                }
                else
                    rplError(ERR_ARGOUTSIDEDOMAIN);
                return;
            }

            if(iszeroReal(&RReg[6])) {
                // NTH ROOTS OF ZERO ARE ZERO
                if(iszeroReal(&RReg[7])) {
                    rplNANToRReg(0);
                    rplNewRealFromRRegPush(0);
                    rplError(ERR_UNDEFINEDRESULT);
                }
                else
                    rplNewRealFromRRegPush(6);
                return;
            }

            BINT isneg = RReg[6].flags & F_NEGATIVE;
            if(RReg[6].flags & F_NEGATIVE) {
                // RESULT IS LIKELY COMPLEX
                // a^(1/n)= ABS(a)^(1/n) * (cos(pi/n)+i*sin(pi/n))

                // USE DEG TO AVOID LOSS OF PRECISION WITH PI

                newRealFromBINT(&RReg[5], 180, 0);

                divReal(&RReg[0], &RReg[5], &RReg[7]);
                divmodReal(&RReg[1], &RReg[9], &RReg[0], &RReg[5]);     // REDUCE TO FIRST CIRCLE

                if(!rplTestSystemFlag(FL_COMPLEXMODE) && !iszeroReal(&RReg[9])) {
                    rplError(ERR_COMPLEXRESULT);
                    return;
                }
                if(isoddReal(&RReg[1])) subReal(&RReg[9],&RReg[9],&RReg[5]);

                RReg[6].flags ^= F_NEGATIVE;    // MAKE IT POSITIVE

                // HERE WE LEFT THE NEW ANGLE IN RReg[9]
            }

            xrootReal(&RReg[8], &RReg[6], &RReg[7]);

            if(!isneg) {
                if(isintegerReal(&RReg[8]) && inint64_tRange(&RReg[8])) {
                    int64_t result = getint64_tReal(&RReg[8]);
                    BINT base;
                    if(op1type)
                        base = DECBINT;
                    else
                        base = LIBNUM(*arg1);
                    rplNewBINTPush(result, base | (LIBNUM(*arg2) & APPROX_BIT));
                }
                else
                    rplNewRealFromRRegPush(8);
                if(!Exceptions)
                    rplCheckResultAndError(&RReg[8]);

                return;
            }

            // RETURN A POLAR COMPLEX

            BINT angmode =
                    rplTestSystemFlag(FL_ANGLEMODE1) |
                    (rplTestSystemFlag(FL_ANGLEMODE2) << 1);

            trig_convertangle(&RReg[9], ANGLEDEG, angmode);

            rplCheckResultAndError(&RReg[0]);
            rplCheckResultAndError(&RReg[8]);

            // RETURN A POLAR COMPLEX
            rplNewComplexPush(&RReg[8], &RReg[0], angmode);

            return;

        }

        case OVR_EQ:
        {
            if(op1type || op2type) {
                if(op1type) {
                    // ROUND TO INTEGER
                    rplBINTToRReg(0, op2);
                    int res = cmpReal(&rop1, &RReg[0]);
                    if(res)
                        rplPushData((WORDPTR) zero_bint);
                    else
                        rplPushData((WORDPTR) one_bint);
                }

                if(op2type) {
                    // ROUND TO INTEGER
                    rplBINTToRReg(0, op1);
                    int res = cmpReal(&RReg[0], &rop2);
                    if(res)
                        rplPushData((WORDPTR) zero_bint);
                    else
                        rplPushData((WORDPTR) one_bint);
                }
                return;
            }
            // BOTH WERE INTEGERS
            if(op1 == op2)
                rplPushData((WORDPTR) one_bint);
            else
                rplPushData((WORDPTR) zero_bint);
            return;
        }

        case OVR_NOTEQ:
        {
            if(op1type || op2type) {
                if(op1type) {
                    // ROUND TO INTEGER
                    rplBINTToRReg(0, op2);
                    int res = cmpReal(&rop1, &RReg[0]);
                    if(res)
                        rplPushData((WORDPTR) one_bint);
                    else
                        rplPushData((WORDPTR) zero_bint);

                }

                if(op2type) {
                    // ROUND TO INTEGER
                    rplBINTToRReg(0, op1);
                    int res = cmpReal(&RReg[0], &rop2);
                    if(res)
                        rplPushData((WORDPTR) one_bint);
                    else
                        rplPushData((WORDPTR) zero_bint);
                }
                return;
            }
            if(op1 == op2)
                rplPushData((WORDPTR) zero_bint);
            else
                rplPushData((WORDPTR) one_bint);
            return;
        }
        case OVR_LT:
        {
            if(op1type || op2type) {
                if(op1type) {
                    rplBINTToRReg(0, op2);
                    int res = cmpReal(&rop1, &RReg[0]);
                    if(res < 0)
                        rplPushData((WORDPTR) one_bint);
                    else
                        rplPushData((WORDPTR) zero_bint);
                }
                if(op2type) {
                    rplBINTToRReg(0, op1);
                    int res = cmpReal(&RReg[0], &rop2);
                    if(res < 0)
                        rplPushData((WORDPTR) one_bint);
                    else
                        rplPushData((WORDPTR) zero_bint);
                }
                return;
            }

            if(op1 < op2)
                rplPushData((WORDPTR) one_bint);
            else
                rplPushData((WORDPTR) zero_bint);
            return;
        }
        case OVR_GT:
        {
            if(op1type || op2type) {
                if(op1type) {
                    rplBINTToRReg(0, op2);
                    int res = cmpReal(&rop1, &RReg[0]);
                    if(res > 0)
                        rplPushData((WORDPTR) one_bint);
                    else
                        rplPushData((WORDPTR) zero_bint);
                }
                if(op2type) {
                    rplBINTToRReg(0, op1);
                    int res = cmpReal(&RReg[0], &rop2);
                    if(res > 0)
                        rplPushData((WORDPTR) one_bint);
                    else
                        rplPushData((WORDPTR) zero_bint);
                }
                return;
            }

            if(op1 > op2)
                rplPushData((WORDPTR) one_bint);
            else
                rplPushData((WORDPTR) zero_bint);
            return;
        }

        case OVR_LTE:
        {
            if(op1type || op2type) {
                if(op1type) {
                    rplBINTToRReg(0, op2);
                    int res = cmpReal(&rop1, &RReg[0]);
                    if(res <= 0)
                        rplPushData((WORDPTR) one_bint);
                    else
                        rplPushData((WORDPTR) zero_bint);
                }
                if(op2type) {
                    rplBINTToRReg(0, op1);
                    int res = cmpReal(&RReg[0], &rop2);
                    if(res <= 0)
                        rplPushData((WORDPTR) one_bint);
                    else
                        rplPushData((WORDPTR) zero_bint);
                }
                return;
            }

            if(op1 <= op2)
                rplPushData((WORDPTR) one_bint);
            else
                rplPushData((WORDPTR) zero_bint);
            return;
        }
        case OVR_GTE:
        {
            if(op1type || op2type) {
                if(op1type) {
                    rplBINTToRReg(0, op2);
                    int res = cmpReal(&rop1, &RReg[0]);
                    if(res >= 0)
                        rplPushData((WORDPTR) one_bint);
                    else
                        rplPushData((WORDPTR) zero_bint);
                }
                if(op2type) {
                    rplBINTToRReg(0, op1);
                    int res = cmpReal(&RReg[0], &rop2);
                    if(res >= 0)
                        rplPushData((WORDPTR) one_bint);
                    else
                        rplPushData((WORDPTR) zero_bint);
                }
                return;
            }

            if(op1 >= op2)
                rplPushData((WORDPTR) one_bint);
            else
                rplPushData((WORDPTR) zero_bint);
            return;
        }
        case OVR_SAME:
        {
            if(op1type || op2type) {
                if(op1type) {
                    // IF IT'S NOT INTEGER, CAN'T BE EQUAL TO ONE
                    if(!isintegerReal(&rop1))
                        rplPushData((WORDPTR) zero_bint);
                    else {
                        rplBINTToRReg(0, op2);
                        int res = eqReal(&rop1, &RReg[0]);
                        if(res)
                            rplPushData((WORDPTR) one_bint);
                        else
                            rplPushData((WORDPTR) zero_bint);
                    }
                }

                if(op2type) {
                    if(!isintegerReal(&rop2))
                        rplPushData((WORDPTR) zero_bint);
                    else {
                        rplBINTToRReg(0, op1);
                        int res = eqReal(&RReg[0], &rop2);
                        if(res)
                            rplPushData((WORDPTR) one_bint);
                        else
                            rplPushData((WORDPTR) zero_bint);
                    }
                }
                return;
            }
            // BOTH WERE INTEGERS
            if(op1 == op2)
                rplPushData((WORDPTR) one_bint);
            else
                rplPushData((WORDPTR) zero_bint);
            return;
        }

        case OVR_AND:
            if(op1type)
                op1 = !iszeroReal(&rop1);
            if(op2type)
                op2 = !iszeroReal(&rop2);
            if(op1 && op2)
                rplPushData((WORDPTR) one_bint);
            else
                rplPushData((WORDPTR) zero_bint);
            return;
        case OVR_OR:
            if(op1type)
                op1 = !iszeroReal(&rop1);
            if(op2type)
                op2 = !iszeroReal(&rop2);
            if(op1 || op2)
                rplPushData((WORDPTR) one_bint);
            else
                rplPushData((WORDPTR) zero_bint);
            return;

        case OVR_CMP:
        {
            if(op1type || op2type) {
                if(op1type) {
                    rplBINTToRReg(0, op2);
                    int res = cmpReal(&rop1, &RReg[0]);
                    if(res < 0)
                        rplPushData((WORDPTR) minusone_bint);
                    else if(res > 0)
                        rplPushData((WORDPTR) one_bint);
                    else
                        rplPushData((WORDPTR) zero_bint);
                }
                if(op2type) {
                    rplBINTToRReg(0, op1);
                    int res = cmpReal(&RReg[0], &rop2);
                    if(res < 0)
                        rplPushData((WORDPTR) minusone_bint);
                    else if(res > 0)
                        rplPushData((WORDPTR) one_bint);
                    else
                        rplPushData((WORDPTR) zero_bint);
                }
                return;
            }

            if(op1 > op2)
                rplPushData((WORDPTR) one_bint);
            else if(op1 < op2)
                rplPushData((WORDPTR) minusone_bint);
            else
                rplPushData((WORDPTR) zero_bint);
            return;
        }

        case OVR_INV:
            // INVERSE WILL ALWAYS BE A REAL, SINCE 1/N == 0 FOR ALL N>1 IN STRICT INTEGER MATH, ORIGINAL UserRPL DOES NOT SUPPORT INVERSE OF INTEGERS

            if(op1 == 0) {
                if(rplTestSystemFlag(FL_COMPLEXMODE))
                    rplUndInfinityToRReg(2);
                else
                    rplInfinityToRReg(2);
            }
            else {
                rplOneToRReg(0);
                rplBINTToRReg(1, op1);
                divReal(&RReg[2], &RReg[0], &RReg[1]);
            }

            rplNewRealFromRRegPush(2);
            if(!Exceptions)
                rplCheckResultAndError(&RReg[2]);

            return;
        case OVR_NEG:
        case OVR_UMINUS:
            op1 = -op1;
            rplNewBINTPush(op1, LIBNUM(*arg1));
            return;
        case OVR_UPLUS:
        case OVR_FUNCEVAL:
        case OVR_EVAL:
        case OVR_EVAL1:
        case OVR_XEQ:
        case OVR_NUM:
            // NOTHING TO DO, JUST KEEP THE ARGUMENT IN THE STACK
            rplPushData(arg1);
            return;
        case OVR_ABS:
            if(op1 < 0)
                rplNewBINTPush(-op1, LIBNUM(*arg1));
            else
                rplPushData(arg1);
            return;
        case OVR_NOT:
            if(op1)
                rplPushData((WORDPTR) zero_bint);
            else
                rplPushData((WORDPTR) one_bint);
            return;
        case OVR_ISTRUE:
            if(op1)
                rplPushData((WORDPTR) one_bint);
            else
                rplPushData((WORDPTR) zero_bint);
            return;

            // ADD MORE case's HERE
        default:
            rplError(ERR_INTEGERSNOTSUPPORTED);
            return;

        }
        return;
#undef arg1
#undef arg2
    }   // END OF OVERLOADABLE OPERATORS

    int64_t result;
    uint64_t uresult;
    BYTEPTR strptr, strend;
    int base, libbase, digit, count, neg, argnum1;
    char basechr;

    if(OPCODE(CurOpcode) >= MIN_RESERVED_OPCODE) {
        switch (OPCODE(CurOpcode)) {
            // STANDARIZED OPCODES:
            // --------------------
            // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES

        case OPCODE_COMPILE:
            // COMPILE RECEIVES:
            // TokenStart = token string
            // TokenLen = token length
            // ArgPtr2 = token blanks afterwards
            // ArgNum2 = blanks length

            // COMPILE RETURNS:
            // RetNum =  enum CompileErrors
        {
            if(LIBNUM(CurOpcode) != DOBINT) {
                // DO NOT COMPILE ANYTHING WHEN CALLED WITH THE UPPER (APPROX) LIBRARY NUMBER
                RetNum = ERR_NOTMINE;
                return;
            }

            uint64_t Locale = rplGetSystemLocale();
            // COMPILE A NUMBER TO A SINT OR A BINT, DEPENDING ON THE ACTUAL NUMERIC VALUE
            result = 0;
            strptr = (BYTEPTR) TokenStart;
            strend = (BYTEPTR) BlankStart;
            base = 10;
            libbase = DECBINT;
            neg = 0;
            argnum1 = TokenLen; // LOCAL COPY

            if(*strptr == '-') {
                neg = 1;
                ++strptr;
                --argnum1;
            }
            else if(*strptr == '+') {
                neg = 0;
                ++strptr;
                --argnum1;
            }

            if(argnum1 <= 0) {
                RetNum = ERR_NOTMINE;
                return;
            }

            if(*strptr == '#') {
                ++strptr;
                --argnum1;
                if(argnum1 <= 0) {
                    RetNum = ERR_NOTMINE;
                    return;
                }

                // THIS IS A NUMBER WITH A BASE
                basechr = strend[-1];
                if(basechr == '.') {
                    // NUMBERS ENDING IN A DOT ARE APPROXIMATED
                    libbase |= APPROX_BIT;
                    --argnum1;
                    --strend;
                    basechr = strend[-1];
                }

                if((basechr == 'd') || (basechr == 'D')) {
                    --argnum1;
                    --strend;
                }
                if((basechr == 'h') || (basechr == 'H')) {
                    base = 16;
                    libbase = HEXBINT | (libbase & APPROX_BIT);
                    --argnum1;
                    --strend;
                }
                if((basechr == 'o') || (basechr == 'O')) {
                    base = 8;
                    libbase = OCTBINT | (libbase & APPROX_BIT);
                    --argnum1;
                    --strend;
                }
                if((basechr == 'b') || (basechr == 'B')) {
                    base = 2;
                    libbase = BINBINT | (libbase & APPROX_BIT);
                    --argnum1;
                    --strend;
                }
            }

            if(argnum1 <= 0) {
                RetNum = ERR_NOTMINE;
                return;
            }

            if(strend[-1] == '.') {
                // NUMBERS ENDING IN A DOT ARE APPROXIMATED
                libbase |= APPROX_BIT;
                --argnum1;
                --strend;
            }

            for(count = 0; count < argnum1; ++count) {
                digit = utf82cp((char *)strptr, (char *)strend);
                strptr = (BYTEPTR) utf8skipst((char *)strptr, (char *)strend);
                if(digit < 0) {
                    RetNum = ERR_NOTMINE;
                    return;
                }
                if((base == 10) && ((WORD) digit == THOUSAND_SEP(Locale)))
                    continue;
                if((digit >= '0') && (digit <= '9'))
                    digit -= 48;
                else if((digit >= 'a') && (digit <= 'f'))
                    digit -= 87;
                else if((digit >= 'A') && (digit <= 'F'))
                    digit -= 55;
                else
                    digit += 100;

                if((digit >= 0) && (digit < base)) {
                    if(((result >> 32) * base) >> 31) {
                        // OVERFLOW, CANNOT BE AN INTEGER
                        RetNum = ERR_NOTMINE;
                        return;
                    }
                    result = result * base + digit;
                    if(result >> 63) {
                        // OVERFLOW, CANNOT BE AN INTEGER
                        RetNum = ERR_NOTMINE;
                        return;
                    }
                }
                else {
                    // AN INVALID DIGIT
                    RetNum = ERR_NOTMINE;
                    return;
                }
            }

            // NOTHING ABOVE DEALS WITH MEMORY, SO NO PROBLEMS WITH GC
            // FROM NOW ON, ANY POINTERS TO THE STRING BECOME INVALID
            // DUE TO POSSIBLE GC

            if(!count) {
                // NO DIGITS?
                RetNum = ERR_NOTMINE;
                return;
            }

            // FINISHED CONVERSION, NOW COMPILE TO SINT OR BINT AS NEEDED
            if(neg)
                result = -result;

            if((result >= MIN_SINT) && (result <= MAX_SINT)) {
                rplCompileAppend(MKOPCODE(libbase, result & 0x3ffff));
                RetNum = OK_CONTINUE;
                return;
            }
            rplCompileAppend(MKPROLOG(libbase, 2));
            rplCompileAppend((WORD) (result & 0xffffffff));     // CAREFUL: THIS IS FOR LITTLE ENDIAN SYSTEMS ONLY!
            rplCompileAppend((WORD) ((result >> 32) & 0xffffffff));
            RetNum = OK_CONTINUE;
            return;
        }
        case OPCODE_DECOMPEDIT:

        case OPCODE_DECOMPILE:
            // DECOMPILE RECEIVES:
            // DecompileObject = Ptr to WORD of object to decompile
            // DecompStringEnd = Byte Ptr to end of current string. Write here with rplDecompAppendString(); rplDecompAppendChar();

            if(ISPROLOG(*DecompileObject)) {
                // THERE'S A PAYLOAD, READ THE NUMBER
                result = *((int64_t *) (DecompileObject + 1));
            }
            else {
                result = OPCODE(*DecompileObject);
                if(result & 0x20000)
                    result |= 0xFFFFFFFFFFFc0000;       // SIGN EXTEND
            }

            base = GETBASE(LIBNUM(*DecompileObject) & ~APPROX_BIT);

            if(base == 2) {
                // THIS IS A BASE-10 NUMBER
                // CONVERT TO STRING

                REAL realnum;

                NUMFORMAT fmt;

                BINT Format, sign;

                rplGetSystemNumberFormat(&fmt);

                rplReadNumberAsReal(DecompileObject, &realnum);

                sign = realnum.flags & F_NEGATIVE;

                realnum.flags ^= sign;

                if(iszeroReal(&realnum))
                    Format = fmt.MiddleFmt;
                else if(ltReal(&realnum, &(fmt.SmallLimit)))
                    Format = fmt.SmallFmt;
                else if(gtReal(&realnum, &(fmt.BigLimit)))
                    Format = fmt.BigFmt;
                else
                    Format = fmt.MiddleFmt;

                realnum.flags ^= sign;

                if(CurOpcode == OPCODE_DECOMPEDIT)
                    Format |= FMT_CODE;

                // ESTIMATE THE MAXIMUM STRING LENGTH AND RESERVE THE MEMORY

                BYTEPTR string;

                BINT len = formatlengthReal(&realnum, Format, fmt.Locale);

                // realnum DATA MIGHT MOVE DUE TO GC, NEEDS TO BE PROTECTED
                ScratchPointer1 = (WORDPTR) realnum.data;
                ScratchPointer2 = (WORDPTR) fmt.SmallLimit.data;
                ScratchPointer3 = (WORDPTR) fmt.BigLimit.data;

                // RESERVE THE MEMORY FIRST
                rplDecompAppendString2(0, len);

                realnum.data = (BINT *) ScratchPointer1;
                fmt.SmallLimit.data = (BINT *) ScratchPointer2;
                fmt.BigLimit.data = (BINT *) ScratchPointer3;

                // NOW USE IT
                string = (BYTEPTR) DecompStringEnd;
                string -= len;

                if(Exceptions) {
                    RetNum = ERR_INVALID;
                    return;
                }

                DecompStringEnd =
                        (WORDPTR) formatReal(&realnum, (char *)string, Format,
                        fmt.Locale);

            }
            else {
                // THIS IS A BINARY, OCTAL OR HEXA NUMBER
                // base HAS THE NUMBER OF BITS PER DIGIT
                if(result < 0) {
                    rplDecompAppendChar('-');
                    uresult = -result;
                }
                else
                    uresult = result;

                rplDecompAppendChar('#');

                if(base >= 3)
                    digit = 60;
                else
                    digit = 62;

                neg = (1 << base) - 1;  // CREATE A MASK TO ISOLATE THE DIGIT

                // SKIP ALL LEADING ZEROS
                while(digit > 0) {
                    if((uresult >> digit) & neg)
                        break;
                    digit -= base;
                }
                // NOW DECOMPILE THE NUMBER
                while(digit >= 0) {
                    rplDecompAppendChar(alldigits[(uresult >> digit) & neg]);
                    digit -= base;
                }

                // ADD BASE CHARACTER
                if(base == 1)
                    rplDecompAppendChar('b');
                if(base == 3)
                    rplDecompAppendChar('o');
                if(base == 4)
                    rplDecompAppendChar('h');

                // ADD TRAILING DOT ON APPROXIMATED NUMBERS
                if(LIBNUM(*DecompileObject) & APPROX_BIT)
                    rplDecompAppendChar('.');
            }

            RetNum = OK_CONTINUE;

            //DECOMPILE RETURNS
            // RetNum =  enum DecompileErrors

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
            // COMPILE RECEIVES:
            // TokenStart = token string
            // TokenLen = token length
            // ArgPtr2 = token blanks afterwards
            // ArgNum2 = blanks length

            // COMPILE RETURNS:
            // RetNum =  OK_TOKENINFO | MKTOKENINFO(...), or ERR_NOTMINE IF NO TOKEN IS FOUND

        {

            // COMPILE A NUMBER TO A SINT OR A BINT, DEPENDING ON THE ACTUAL NUMERIC VALUE
            result = 0;
            strptr = (BYTEPTR) TokenStart;
            base = 10;
            libbase = DECBINT;
            neg = 0;
            argnum1 = TokenLen; // LOCAL COPY

            // SIGN IS HANDLED AS UNARY OPERATOR IN SYMBOLICS
            /*
               if(*strptr=='-') { neg=1; ++strptr; --argnum1; }
               else if(*strptr=='+') { neg=0; ++strptr; --argnum1; }
             */

            if(LIBNUM(CurOpcode) != DOBINT) {
                // DO NOT COMPILE ANYTHING WHEN CALLED WITH THE UPPER (APPROX) LIBRARY NUMBER
                RetNum = ERR_NOTMINE;
                return;
            }

            if(*strptr == '#') {
                ++strptr;
                --argnum1;
                // THIS IS A NUMBER WITH A BASE, FIND THE BASE CHARACTER
                base = 0;
                neg = 1;        // REUSED VARIABLE TO INDICATE THAT THE NUMBER SPECIFIED THE BASE EXPLICITLY

                for(count = 0; count < argnum1; ++count) {
                    digit = strptr[count];
                    if((digit >= '0') && (digit <= '9'))
                        digit -= 48;
                    else if((digit >= 'a') && (digit <= 'f'))
                        digit -= 87;
                    else if((digit >= 'A') && (digit <= 'F'))
                        digit -= 55;
                    else
                        digit += 100;

                    if((digit < 0) || (digit >= 16)) {
                        // THIS IS AN INVALID NUMERIC CHARACTER, MARK THE END OF TOKEN
                        basechr = strptr[count];
                        argnum1 = count;
                        if((basechr == 'd') || (basechr == 'D')) {
                            base = 10;
                        }
                        else if((basechr == 'h') || (basechr == 'H')) {
                            base = 16;
                        }
                        else if((basechr == 'o') || (basechr == 'O')) {
                            base = 8;
                        }
                        else if((basechr == 'b') || (basechr == 'B')) {
                            base = 2;
                        }
                        else {
                            basechr = strptr[count - 1];
                            if((basechr == 'd') || (basechr == 'D')) {
                                base = 10;
                            }
                            else if((basechr == 'h') || (basechr == 'H')) {
                                base = 16;
                            }
                            else if((basechr == 'o') || (basechr == 'O')) {
                                base = 8;
                            }
                            else if((basechr == 'b') || (basechr == 'B')) {
                                base = 2;
                            }
                            else {
                                // SYNTAX ERROR OR NOT A VALID NUMBER
                                RetNum = ERR_NOTMINE;
                                return;
                            }
                            argnum1 = count - 1;
                        }
                        break;
                    }

                }

                if(base == 0) {
                    // SYNTAX ERROR, NUMBER DOES NOT HAVE A PROPER BASE SPECIFICATION
                    RetNum = ERR_NOTMINE;
                    return;
                }
            }

            // NOW WITH A PROPER BASE SELECTED, VERIFY THAT ALL DIGITS ARE NUMERIC

            for(count = 0; count < argnum1; ++count) {
                digit = strptr[count];
                if((digit >= '0') && (digit <= '9'))
                    digit -= 48;
                else if((digit >= 'a') && (digit <= 'f'))
                    digit -= 87;
                else if((digit >= 'A') && (digit <= 'F'))
                    digit -= 55;
                else
                    digit += 100;

                if((digit >= 0) && (digit < base)) {
                    if(((result >> 32) * base) >> 31) {
                        // OVERFLOW, CANNOT BE AN INTEGER
                        RetNum = ERR_NOTMINE;
                        return;
                    }
                    result = result * base + digit;
                }
                else {
                    // AN INVALID DIGIT
                    if(count == 0) {
                        RetNum = ERR_NOTMINE;
                        return;
                    }
                    else {
                        if(neg) {
                            // IF THE BASE WAS SPECIFIED EXPLICITLY, THERE CANNOT BE ILLEGAL DIGITS
                            RetNum = ERR_NOTMINE;
                            return;
                        }
                        if(digit == ('.' + 100))
                            ++count;
                        // REPORT AS MANY VALID DIGITS AS POSSIBLE
                        RetNum = OK_TOKENINFO | MKTOKENINFO((strptr + count) -
                                (BYTEPTR) TokenStart, TITYPE_INTEGER, 0, 1);
                        return;
                    }
                }
            }
            // ALL DIGITS WERE CORRECT
            RetNum = OK_TOKENINFO | MKTOKENINFO((strptr + argnum1) -
                    (BYTEPTR) TokenStart, TITYPE_INTEGER, 0, 1);
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
        {
            TypeInfo =
                    DOREAL * 100 + 2 + ((LIBNUM(*ObjectPTR) -
                        LIBRARY_NUMBER) & 1) + ((LIBNUM(*ObjectPTR) -
                        LIBRARY_NUMBER) >> 1) * 10;
            DecompHints = 0;
            RetNum = OK_TOKENINFO | MKTOKENINFO(0, TITYPE_INTEGER, 0, 1);
            return;
        }
        case OPCODE_GETROMID:
            // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
            // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
            // ObjectPTR = POINTER TO ROM OBJECT
            // LIBBRARY RETURNS: ObjectID=new ID, ObjectIDHash=hash, RetNum=OK_CONTINUE
            // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

            libGetRomptrID(LIBRARY_NUMBER, (WORDPTR *) ROMPTR_TABLE, ObjectPTR);
            return;
        case OPCODE_ROMID2PTR:
            // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
            // ObjectID = ID, ObjectIDHash=hash
            // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
            // OR RetNum= ERR_NOTMINE;

            libGetPTRFromID((WORDPTR *) ROMPTR_TABLE, ObjectID, ObjectIDHash);
            return;

        case OPCODE_CHECKOBJ:
            // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
            // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
            // ObjectPTR = POINTER TO THE OBJECT TO CHECK
            // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
            if(ISPROLOG(*ObjectPTR)) {
                if(OBJSIZE(*ObjectPTR) != 2) {
                    RetNum = ERR_INVALID;
                    return;
                }
            }

            RetNum = OK_CONTINUE;
            return;

        case OPCODE_AUTOCOMPNEXT:
            //libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
            RetNum = ERR_NOTMINE;
            return;

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

        default:

            RetNum = ERR_NOTMINE;
            return;

        }
    }
    else {
        // ALL OTHER OPCODES ARE SINT NUMBERS, JUST PUSH THEM ON THE STACK
        rplPushData(IPtr);
        return;
    }
// DON'T ISSUE A BAD_OPCODE ERROR SINCE ALL OPCODES ARE VALID SINT NUMBERS
}

#endif
