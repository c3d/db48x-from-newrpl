
/*
 * Copyright (c) 2017, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// THIS IS A BASIC FIXED-POINT MATH LIBRARY TO BE USED WITH PLOT OBJECTS AND GRAPHICS IN GENERAL
// IT'S LOW-PRECISION 40.24 ARITHMETIC
// FAST, NO ERRORS OR EXCEPTIONS, OVERFLOW ONLY ON MULTIPLICATION

#include "newrpl.h"
#include "fastmath.h"

#define HI(n) (((uint64_t)n)>>32)
#define LO(n) (((uint64_t)n)&0xffffffff)

//FPINT addFPINT(FPINT a,FPINT b) { return a+b; }
//FPINT subFPINT(FPINT a,FPINT b) { return a-b; }

// MULTIPLY TWO FP NUMBERS
FPINT mulFPINT(FPINT a, FPINT b)
{
    // PROPER MULTIPLICATION SEQUENCE TO AVOID OVERFLOWS
    BINT sign = 0;
    uint64_t res;
    if(a < 0) {
        sign ^= 1;
        a = -a;
    }
    if(b < 0) {
        sign ^= 1;
        b = -b;
    }

    res = (LO(a) * LO(b)) >> 32;
    res += HI(a) * LO(b) + LO(a) * HI(b);
    if(HI(a) * HI(b) + (res >> 32) >= 0x1000000000000ULL) {
        // OVERFLOW!!!
        // RETURN MAXIMUM INTEGER WITH CORRECT SIGN
        if(sign)
            return (1ULL << 63);
        return (1ULL << 63) - 1;
    }

    res = (res >> 16) + ((HI(a) * HI(b)) << 16);
    if(sign)
        return -res;
    return res;

}

// DIVIDE TWO FP NUMBERS
FPINT divFPINT(FPINT a, FPINT b)
{
    BINT sign = 0;
    uint64_t res;
    if(a < 0) {
        sign ^= 1;
        a = -a;
    }
    if(b < 0) {
        sign ^= 1;
        b = -b;
    }

    res = b / a;
    a -= res * b;

    res <<= 24;

    BINT k;
    for(k = 23; (a != 0) && (k >= 0); k--) {
        a <<= 1;
        if(a > b) {
            res |= 1 << k;
            a -= b;
        }
    }
    if(a) {
        if((a << 1) > b)
            ++res;      // ROUND CORRECTLY THE LAST DIGIT
    }
    if(sign)
        return -res;
    return res;
}

// COMPUTE SIN AND COS AS A 0.24 FIXED POINT NUMBERS
// RETURNS THE HIGH WORD=SIN(), LOW WORD=COS()

// MACROS TO EXTRACT THE COSINE AND SINE FROM THE 64-BIT RESULT
// USING MULTIPLE TYPE CASTS TO FORCE SIGN EXTENSION
#define FPCOS(word) ((FPINT)((BINT)(LO(word))))
#define FPSIN(word) ((FPINT)((BINT)(HI(word))))

#define CMULT(constant,x) (((constant)*(x))>>24)

// ANGLE NORMALIZED IN HALF-TURNS (1 AND -1 MEANS 180 DEG, 0.5 = 90 DEG, -0.5 = -90 DEG)

uint64_t sincosFPINT(FPINT angle)
{
    BINT negsin = 0, negcos = 0;
    if(angle < 0) {
        negsin = 1;
        angle = -angle;
    }
    angle &= 0x1ffffff; // REDUCE TO FIRST TURN
    if(angle >= 0x1800000) {
        angle = 0x2000000 - angle;
        negsin ^= 1;
    }
    else if(angle >= 0x1000000) {
        angle -= 0x1000000;
        negsin ^= 1;
        negcos ^= 1;
    }
    else if(angle > 0x800000) {
        angle = 0x1000000 - angle;
        negcos ^= 1;
    }

    // ANGLE IS REDUCED TO -0.5/+0.5 WHERE POLYNOMIAL INTERPOLATION IS VALID

    // DO HORNER WITH PRECOMPUTED CONSTANTS (0.32 FIXED POINT)
    // ANGLE IS ALWAYS 0.24 THEREFORE MULTIPLICATION NEVER OVERFLOWS!

    FPINT C1, C2, ang2;

    ang2 = CMULT(angle, angle);
    C1 = CMULT(ang2, CMULT(ang2, CMULT(-61297037LL,
                    ang2) + 769880445LL) - 3746706455LL) + 3037000500LL;
    C2 = CMULT(CMULT(CMULT(-231543704, ang2) + 1959187590, ang2) - 4770292713,
            angle);

    BINT sin, cos;

    sin = ((C1 - C2) + 0x80) >> 8;
    cos = ((C1 + C2) + 0x80) >> 8;

    if(negsin)
        sin = -sin;
    if(negcos)
        cos = -cos;

    return (((uint64_t) ((UBINT) sin)) << 32) + ((uint64_t) ((UBINT) cos));

}
