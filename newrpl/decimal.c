/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>

//#include <decimal.h>
//#include <stdlib.h>
//#include <stdio.h>

// *************************************************************************
// ************* LOW LEVEL API FOR DECIMAL LIBRARY ************************
// *************************************************************************

void initContext(WORD precision)
{
    if(precision > REAL_PRECISION_MAX)
        precision = REAL_PRECISION_MAX;
    Context.flags = 0;
    Context.precdigits = precision;
    // DON'T RESET MEMORY ALLOCATION HERE
    //Context.alloc_bmp=EMPTY_STORAGEBMP;
}

const int lowestzerobit[16] = {
    0,  // 0000
    1,  // 0001
    0,  // 0010
    2,  // 0011
    0,  // 0100
    1,  // 0101
    0,  // 0110
    3,  // 0111
    0,  // 1000
    1,  // 1001
    0,  // 1010
    2,  // 1011
    0,  // 1100
    1,  // 1101
    0,  // 1110
    -1  // 1111
};

BINT *allocRegister()
{
    WORD bmp = Context.alloc_bmp;
    int k;

    for(k = 0; k < 8; ++k) {
        if(lowestzerobit[bmp & 0xf] >= 0) {
            Context.alloc_bmp |= 1 << ((k << 2) + lowestzerobit[bmp & 0xf]);
            return (BINT *) Context.regdata + ((k << 2) +
                    lowestzerobit[bmp & 0xf]) * REAL_REGISTER_STORAGE;
        }
        bmp >>= 4;
    }

    // THIS SHOULD BE TOTAL PANIC!!!

    Context.flags |= CTX_OUTOFMEMORY;

    // TODO: PROPER MANAGEMENT ALTHOUGH THIS SHOULD NEVER HAPPEN!
    //printf("Total panic: Out of memory\n");
    //exit(-1);

    return 0;
}

void freeRegister(BINT * data)
{
    int regnum = (data - Context.regdata) / REAL_REGISTER_STORAGE;

    if(regnum < 0 || regnum > 31)
        return;

    Context.alloc_bmp ^= 1 << regnum;

}

const unsigned char const carry_table[64] = {
    0, 0, 1, 2, 2, 3, 4, 4, 5, 6, 6,
    7, 8, 8, 9, 10, 10, 11, 12, 12, 13,
    14, 14, 15, 16, 16, 17, 18, 18, 19, 20,
    20, 21, 22, 22, 23, 24, 24, 25, 26, 26,
    27, 28, 28, 29, 30, 30, 31, 32, 32, 33,
    34, 34, 35, 36, 36, 37, 38, 38, 39, 40,
    40, 41, 42
};

const UBINT const carry_table_mult[64] = {
    0U, 0U, 100000000U, 200000000U, 200000000U, 300000000U, 400000000U,
            400000000U, 500000000U, 600000000U, 600000000U,
    700000000U, 800000000U, 800000000U, 900000000U, 1000000000U, 1000000000U,
            1100000000U, 1200000000U, 1200000000U, 1300000000U,
    1400000000U, 1400000000U, 1500000000U, 1600000000U, 1600000000U,
            1700000000U, 1800000000U, 1800000000U, 1900000000U, 2000000000U,
    2000000000U, 2100000000U, 2200000000U, 2200000000U, 2300000000U,
            2400000000U, 2400000000U, 2500000000U, 2600000000U, 2600000000U,
    2700000000U, 2800000000U, 2800000000U, 2900000000U, 3000000000U,
            3000000000U, 3100000000U, 3200000000U, 3200000000U, 3300000000U,
    3400000000U, 3400000000U, 3500000000U, 3600000000U, 3600000000U,
            3700000000U, 3800000000U, 3800000000U, 3900000000U, 4000000000U,
    4000000000U, 4100000000U, 4200000000U
};

// NEW FORMAT USES 8-DIGIT WITH -1E8 TO 1E8 RANGE

// ALL NUMBERS ARE INTEGERS

// CARRY WILL BE DEFERRED AS MUCH AS POSSIBLE

// CORRECT CARRY FROM START TO end (END EXCLUDED, POSSIBLE END CARRY NEEDS TO BE HANDLED BY OTHER ROUTINES)
void carry_correct(BINT * start, BINT nwords)
{
    BINT *end = start + nwords - 1;
    /*
       while(start<end-1) {
       while(*start>=100000000) { ++*(start+1); *start-=100000000; }
       while(*start<0) { --*(start+1); *start+=100000000; }
       ++start;
       }
     */
    BINT carry = 0, word;
    while(start < end) {
        word = *start + carry;

        if(word < 0) {
            if(word <= -200000000) {
                carry = -carry_table[(-word) >> 26];
                word -= carry * 100000000;
            }
            else {
                if(word <= -100000000) {
                    carry = -1;
                    word += 100000000;
                }
                else
                    carry = 0;
            }
        }
        else {
            if(word >= 200000000) {
                carry = carry_table[word >> 26];
                word -= carry * 100000000;
            }
            else
                carry = 0;
        }
        // HERE word IS WITHIN +/-1E8
        if(word < 0) {
            word += 100000000;
            --carry;
        }
        else if(word >= 100000000) {
            word -= 100000000;
            ++carry;
        }
        *start = word;
        ++start;
    }

    *start += carry;

}

// SAME AS carry_correct() BUT ASSUMES ALL WORDS ARE POSITIVE, THIS IS TRUE DURING MULTIPLICATION
void carry_correct_pos(BINT * start, BINT nwords)
{
    BINT *end = start + nwords - 1;
    BINT carry = 0, word;
    while(start < end) {
        word = *start + carry;

        carry = carry_table[word >> 26];
        word -= carry * 100000000;      //(BINT)carry_table_mult[word>>26];
        // HERE word IS WITHIN +/-1E8
        if(word >= 100000000) {
            word -= 100000000;
            ++carry;
        }
        *start = word;
        ++start;
    }

    *start += carry;

}

// CHECK THE NUMBER RANGE, CHANGE TO INFINITY OR ZERO AS NEEDED
void checkrange(REAL * number)
{
    if(number->exp > MAX_EXPONENT) {

        // TODO: RAISE OVERFLOW EXCEPTION WHEN THIS HAPPENS
        number->data[0] = 0;
        number->len = 1;
        number->flags |= F_INFINITY | F_OVERFLOW;
        number->exp = 0;
        return;
    }

    if(number->exp < MIN_EXPONENT) {
        // TODO: RAISE UNDERFLOW EXCEPTION WHEN THIS HAPPENS
        // TURN TO ZERO
        number->data[0] = 0;
        number->len = 1;
        number->flags =
                ((number->
                    flags & F_NEGATIVE) ? F_NEGUNDERFLOW : F_POSUNDERFLOW) |
                F_APPROX;
        number->exp = 0;
        return;
    }

}

// FULLY NORMALIZE A NUMBER
void normalize(REAL * number)
{
    BINT *start, *end, *dest;
    BINT word, carry;
    dest = start = number->data;
    end = number->data + number->len;

    *end = 0;   // ADD A ZERO FOR POSSIBLE CARRY
    --end;
    // REMOVE LEFT ZEROES IF ANY
    while((*end == 0) && (end > start))
        --end;

    // REMOVE TRAILING ZEROS IF ANY
    while((*start == 0) && (start < end)) {
        ++start;
        number->exp += 8;
    }

    number->len = end - start + 1;
#ifdef ENABLE_ARM_ASSEMBLY

    carry = carry_correct_arm(start, dest, end, (char *)carry_table);
    dest += end - start + 1;
    start = end;

#else

    // CARRY CORRECT
    carry = 0;
    while(start <= end) {
        word = *start + carry;

        if(word < 0) {
            if(word <= -200000000) {
                carry = -carry_table[(-word) >> 26] - 1;
                word -= carry * 100000000;
            }
            else {
                if(word <= -100000000) {
                    carry = -2;
                    word += 200000000;
                }
                else
                    carry = 0;
            }
        }
        else {
            if(word >= 200000000) {
                carry = carry_table[word >> 26];
                word -= carry * 100000000;
            }
            else
                carry = 0;
        }
        // HERE word IS WITHIN +/-1E8
        if(word < 0) {
            word += 100000000;
            --carry;
        }
        else if(word >= 100000000) {
            word -= 100000000;
            ++carry;
        }
        *dest = word;
        ++start;
        ++dest;
    }
#endif

    if(carry) {
        // THERE'S CARRY ON THE LAST WORD
        ++number->len;
        *dest++ = carry;
    }

    // REMOVE LEFT ZEROES IF ANY (POSSIBLE CANCELLATION AFTER CARRY CORRECTION)
    --dest;
    while((*dest == 0) && (dest > number->data)) {
        --dest;
        --number->len;
    }

    if(*dest < 0) {
        // NUMBER IS NEGATIVE, MAKE ALL TERMS POSITIVE AND CHANGE THE GLOBAL SIGN
        *dest = -*dest - 1;
        --dest;
        while(dest > number->data) {
            *dest = 99999999 - *dest;
            --dest;
        }
        *dest = 100000000 - *dest;
        // CHECK FOR CARRY IN THE LAST WORD
        while((*dest >= 100000000) && (dest < number->data + number->len)) {
            *dest -= 100000000;
            ++dest;
            ++*dest;
        }

        number->flags ^= F_NEGATIVE;
        // FINAL TRIM FOR LEFT ZEROS
        while((number->data[number->len - 1] == 0) && (number->len > 1))
            --number->len;

    }

    // FINALLY, IF NUMBER IS ZERO, FORCE EXPONENT TO BE ZERO AND SIGN TO BE POSITIVE
    if((number->len == 1) && (number->data[0] == 0)) {
        number->exp = 0;
        number->flags &= ~F_NEGATIVE;
    }

    number->flags &= ~F_NOTNORMALIZED;

}

// APPLY ROUNDING IN-PLACE TO THE GIVEN NUMBER OF DIGITS
// OR JUST TRUNCATES THE NUMBER IF truncate IS NON-ZERO
/*
void round_real(REAL *r,int digits,int truncate)
{
    int dig=sig_digits(r->data[r->len-1]);

    if( ((r->len-1)<<3)+dig<=digits) return;    // NOTHING TO ROUND

    int trim=dig+((r->len-1)<<3)-digits;

    int smallshift=trim&7;

    trim=(trim+7)>>3;

    if(smallshift) {
        long_shift(r->data,r->len,8-smallshift);
        if(dig>smallshift) ++r->len;
        r->exp-=8-smallshift;
    }

    // DO COPY AND CARRY CORRECTION AT THE SAME TIME

    BINT *end=r->data+r->len;
    BINT *start=r->data+trim;
    BINT *dest=r->data;

    *end=0; // ADD A ZERO WORD FOR POSSIBLE FINAL CARRY

    // DO THE ACTUAL ROUNDING HERE
    if(!truncate && (r->data[trim-1]>=50000000)) r->data[trim]+=1;

    while(start<end) {
        *dest=*start;
        while(*dest>=100000000) { ++*(start+1); *dest-=100000000; }
        while(*dest<0) { --*(start+1); *dest+=100000000; }
        ++start;
        ++dest;
    }

    r->exp+=trim<<3;

    // DEAL WITH A POSSIBLE FINAL CARRY
    if(*end!=0) {
    *dest=*start;
    ++dest;
    }

    if(dest==r->data) {
        *dest=0;
        ++dest;
        r->flags&=~F_NEGATIVE;
    }

    r->len=dest-r->data;
    r->flags|=F_APPROX;

}
*/

// FASTER ROUNDING ROUTINE WITHOUT SHIFTING
// APPLY ROUNDING IN-PLACE TO THE GIVEN NUMBER OF DIGITS
// OR JUST TRUNCATES THE NUMBER IF truncate IS NON-ZERO

void round_real(REAL * r, int digits, int truncate)
{
    int dig = sig_digits(r->data[r->len - 1]);

    if(((r->len - 1) << 3) + dig <= digits)
        return; // NOTHING TO ROUND

    int trim = dig + ((r->len - 1) << 3) - digits;

    int smallshift = trim & 7;

    trim >>= 3;

    // DO COPY AND CARRY CORRECTION AT THE SAME TIME

    BINT *end = r->data + r->len;
    BINT *start = r->data + trim;
    BINT *dest = r->data;
    BINT lodigits = 0;

    *end = 0;   // ADD A ZERO WORD FOR POSSIBLE FINAL CARRY

    if(!truncate) {
        // DO THE ACTUAL ROUNDING HERE
        if(smallshift) {
            lodigits = r->data[trim];
            r->data[trim] = hi_digits_rounded(r->data[trim], smallshift);
            lodigits -= r->data[trim];
            if(r->data[trim] >= 100000000) {
                // CARRY PROPAGATION
                carry_correct(start, end - start + 1);
            }
        }
        else {
            if(trim > 0)
                lodigits = r->data[trim - 1];
            else
                lodigits = 0;
            if((trim > 0) && (r->data[trim - 1] >= 50000000))
                r->data[trim] += 1;
            if(r->data[trim] >= 100000000) {
                // CARRY PROPAGATION
                carry_correct(start, end - start + 1);
            }
        }
    }
    else {
        // JUST TRUNCATE, NO NEED TO ROUND
        if(smallshift)
            r->data[trim] = hi_digits(r->data[trim], smallshift);
    }
    // CHECK IF ALL TRIMMED WORDS ARE ZEROS
    if(trim > 0) {
        BINT *ptr = r->data + trim - 1;
        while(ptr >= r->data) {
            if(*ptr)
                break;
            --ptr;
        }
        if(ptr >= r->data)
            lodigits |= 1;
    }

    // SKIP TRAILING ZEROS IF ANY IN THE NUMBER
    while((*start == 0) && (start < end)) {
        ++start;
        ++trim;
    }
    // MOVE WORDS ONLY
    if(dest != start) {
        while(start < end) {
            *dest = *start;
            ++start;
            ++dest;
        }
    }
    else {
        dest = start = end;
    }

    r->exp += trim << 3;

    // DEAL WITH A POSSIBLE FINAL CARRY
    if(*end != 0) {
        *dest = *start;
        ++dest;
    }

    if(dest == r->data) {
        *dest = 0;
        ++dest;
        r->flags &= ~F_NEGATIVE;
    }

    r->len = dest - r->data;
    if(lodigits)
        r->flags |= F_APPROX;

}

// EXTRACT ONE DECIMAL DIGIT FROM A NUMBER, AT position
// THE POSITION IS n FOR THE 10^n DIGIT

// FULLY NORMALIZE, RANGE CHECK AND ROUND TO SYSTEM PRECISION

void finalize(REAL * number)
{
    checkrange(number);
    if(number->flags & (F_INFINITY | F_NOTANUMBER))
        return;
    normalize(number);

    round_real(number, Context.precdigits, 0);
}

// ADD A 64-BIT INTEGER TO A LONG NUMBER AT start

void add_single64(BINT * start, int64_t number)
{
    UWORDUNION tmp;
    BINT lo, hi;

    if(number < 0) {

        tmp.w = -number;

        /*
           if(tmp.w>1ULL<<58) {
           printf("Problem!!!\n");
           }
         */

//        if(((tmp.w32[1]<<6)|(tmp.w32[0]>>26))) {
        hi = (((tmp.w32[1] << 6) | (tmp.w32[0] >> 26)) * 2882303762ULL) >> 32;
        lo = tmp.w + hi * 4194967296U;
//        } else {
//        hi=0;
//        lo=tmp.w32[0];
//        }
        /*
           // DEBUG ONLY - JUST DOUBLE CHECK
           uint64_t test=-((uint64_t)lo+((uint64_t)hi*100000000ULL));

           if(test!=number) {
           printf("Error!");
           }
         */

        start[0] -= lo;
        start[1] -= hi;

        return;
    }
    else {
        tmp.w = number;

        /*
           if(tmp.w>1ULL<<58) {
           printf("Problem!!!\n");
           }
         */

//        if(((tmp.w32[1]<<6)|(tmp.w32[0]>>26))) {
        hi = (((tmp.w32[1] << 6) | (tmp.w32[0] >> 26)) * 2882303762ULL) >> 32;
        lo = tmp.w + hi * 4194967296U;
//        } else {
//        hi=0;
//        lo=tmp.w32[0];
//        }
        /*
           // DEBUG ONLY - JUST DOUBLE CHECK
           uint64_t test=((uint64_t)lo+((uint64_t)hi*100000000ULL));

           if(test!=number) {
           printf("Error!");
           }
         */

        start[0] += lo;
        start[1] += hi;

        return;
    }

}

// MULTIPLIES 2 WORDS X 2 WORDS IN 3 MULTIPLICATIONS USING KARATSUBA TRICK
// AND ACCUMULATE RESULT
void add_karatsuba(BINT * start, BINT * a, BINT * b)
{
    int64_t hi, lo, mid;
    UWORDUNION tmp1, tmp2, tmp3;
    BINT lo32_1, hi32_1, lo32_2, hi32_2, lo32_3, hi32_3;

    lo = a[0] * (int64_t) b[0];
    hi = a[1] * (int64_t) b[1];
    mid = (a[1] + a[0]) * (int64_t) (b[0] + b[1]) - hi - lo;

    // UNROLLED add_single64()
    // NUMBER IS GUARANTEED TO BE POSITIVE
    tmp1.w = lo;
    tmp2.w = mid;
    tmp3.w = hi;
    hi32_1 = (((tmp1.w32[1] << 6) | (tmp1.w32[0] >> 26)) * 2882303762ULL) >> 32;
    hi32_2 = (((tmp2.w32[1] << 6) | (tmp2.w32[0] >> 26)) * 2882303762ULL) >> 32;
    hi32_3 = (((tmp3.w32[1] << 6) | (tmp3.w32[0] >> 26)) * 2882303762ULL) >> 32;
    lo32_1 = tmp1.w + hi32_1 * 4194967296U;
    lo32_2 = tmp2.w + hi32_2 * 4194967296U;
    lo32_3 = tmp3.w + hi32_3 * 4194967296U;
    start[0] += lo32_1;
    start[1] += lo32_2 + hi32_1;
    start[2] += lo32_3 + hi32_2;
    start[3] += hi32_3;

}

// ADD nwords FROM n1 TO result, NO CARRY CHECK
// RESULT MUST BE INITIALIZED SOMEWHERE ELSE
void add_long(BINT * result, BINT * n1start, BINT nwords)
{
    PROTECT_WRITE_AREA(result, nwords);

    while(nwords >= 3) {
        result[nwords - 1] += n1start[nwords - 1];
        result[nwords - 2] += n1start[nwords - 2];
        result[nwords - 3] += n1start[nwords - 3];
        nwords -= 3;
    }

    while(nwords) {
        result[nwords - 1] += n1start[nwords - 1];
        --nwords;
    }
}

// NEW FASTER VERSION OF SHIFT, INCLUDES A SMALL MULTIPLICATIVE CONSTANT 0-31

const BINT const shiftmul_K1[] = {
    // CONSTANTS ARE 2^24/10^(8-N)*M
    0,
    1,
    16,
    167,
    1677,
    16777,
    167772,
    1677721,
    0,
    3,
    33,
    335,
    3355,
    33554,
    335544,
    3355443,
    0,
    5,
    50,
    503,
    5033,
    50331,
    503316,
    5033164,
    0,
    6,
    67,
    671,
    6710,
    67108,
    671088,
    6710886,
    0,
    8,
    83,
    838,
    8388,
    83886,
    838860,
    8388608,
    1,
    10,
    100,
    1006,
    10066,
    100663,
    1006632,
    10066329,
    1,
    11,
    117,
    1174,
    11744,
    117440,
    1174405,
    11744051,
    1,
    13,
    134,
    1342,
    13421,
    134217,
    1342177,
    13421772,
    1,
    15,
    150,
    1509,
    15099,
    150994,
    1509949,
    15099494,
    1,
    16,
    167,
    1677,
    16777,
    167772,
    1677721,
    16777216,
    1,
    18,
    184,
    1845,
    18454,
    184549,
    1845493,
    18454937,
    2,
    20,
    201,
    2013,
    20132,
    201326,
    2013265,
    20132659,
    2,
    21,
    218,
    2181,
    21810,
    218103,
    2181038,
    21810380,
    2,
    23,
    234,
    2348,
    23488,
    234881,
    2348810,
    23488102,
    2,
    25,
    251,
    2516,
    25165,
    251658,
    2516582,
    25165824,
    2,
    26,
    268,
    2684,
    26843,
    268435,
    2684354,
    26843545,
    2,
    28,
    285,
    2852,
    28521,
    285212,
    2852126,
    28521267,
    3,
    30,
    301,
    3019,
    30198,
    301989,
    3019898,
    30198988,
    3,
    31,
    318,
    3187,
    31876,
    318767,
    3187671,
    31876710,
    3,
    33,
    335,
    3355,
    33554,
    335544,
    3355443,
    33554432,
    3,
    35,
    352,
    3523,
    35232,
    352321,
    3523215,
    35232153,
    3,
    36,
    369,
    3690,
    36909,
    369098,
    3690987,
    36909875,
    3,
    38,
    385,
    3858,
    38587,
    385875,
    3858759,
    38587596,
    4,
    40,
    402,
    4026,
    40265,
    402653,
    4026531,
    40265318,
    4,
    41,
    419,
    4194,
    41943,
    419430,
    4194304,
    41943040,
    4,
    43,
    436,
    4362,
    43620,
    436207,
    4362076,
    43620761,
    4,
    45,
    452,
    4529,
    45298,
    452984,
    4529848,
    45298483,
    4,
    46,
    469,
    4697,
    46976,
    469762,
    4697620,
    46976204,
    4,
    48,
    486,
    4865,
    48653,
    486539,
    4865392,
    48653926,
    5,
    50,
    503,
    5033,
    50331,
    503316,
    5033164,
    50331648,
    5,
    52,
    520,
    5200,
    52009,
    520093,
    5200936,
    52009369,
};

const BINT const shiftmul_K2[] = {
    // CONSTANTS ARE 10^N*M
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    2,
    20,
    200,
    2000,
    20000,
    200000,
    2000000,
    20000000,
    3,
    30,
    300,
    3000,
    30000,
    300000,
    3000000,
    30000000,
    4,
    40,
    400,
    4000,
    40000,
    400000,
    4000000,
    40000000,
    5,
    50,
    500,
    5000,
    50000,
    500000,
    5000000,
    50000000,
    6,
    60,
    600,
    6000,
    60000,
    600000,
    6000000,
    60000000,
    7,
    70,
    700,
    7000,
    70000,
    700000,
    7000000,
    70000000,
    8,
    80,
    800,
    8000,
    80000,
    800000,
    8000000,
    80000000,
    9,
    90,
    900,
    9000,
    90000,
    900000,
    9000000,
    90000000,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    11,
    110,
    1100,
    11000,
    110000,
    1100000,
    11000000,
    110000000,
    12,
    120,
    1200,
    12000,
    120000,
    1200000,
    12000000,
    120000000,
    13,
    130,
    1300,
    13000,
    130000,
    1300000,
    13000000,
    130000000,
    14,
    140,
    1400,
    14000,
    140000,
    1400000,
    14000000,
    140000000,
    15,
    150,
    1500,
    15000,
    150000,
    1500000,
    15000000,
    150000000,
    16,
    160,
    1600,
    16000,
    160000,
    1600000,
    16000000,
    160000000,
    17,
    170,
    1700,
    17000,
    170000,
    1700000,
    17000000,
    170000000,
    18,
    180,
    1800,
    18000,
    180000,
    1800000,
    18000000,
    180000000,
    19,
    190,
    1900,
    19000,
    190000,
    1900000,
    19000000,
    190000000,
    20,
    200,
    2000,
    20000,
    200000,
    2000000,
    20000000,
    200000000,
    21,
    210,
    2100,
    21000,
    210000,
    2100000,
    21000000,
    210000000,
    22,
    220,
    2200,
    22000,
    220000,
    2200000,
    22000000,
    220000000,
    23,
    230,
    2300,
    23000,
    230000,
    2300000,
    23000000,
    230000000,
    24,
    240,
    2400,
    24000,
    240000,
    2400000,
    24000000,
    240000000,
    25,
    250,
    2500,
    25000,
    250000,
    2500000,
    25000000,
    250000000,
    26,
    260,
    2600,
    26000,
    260000,
    2600000,
    26000000,
    260000000,
    27,
    270,
    2700,
    27000,
    270000,
    2700000,
    27000000,
    270000000,
    28,
    280,
    2800,
    28000,
    280000,
    2800000,
    28000000,
    280000000,
    29,
    290,
    2900,
    29000,
    290000,
    2900000,
    29000000,
    290000000,
    30,
    300,
    3000,
    30000,
    300000,
    3000000,
    30000000,
    300000000,
    31,
    310,
    3100,
    31000,
    310000,
    3100000,
    31000000,
    310000000
};

const BINT const shift_constants[16] = {
    0, 1,
    429, 10,
    4294, 100,
    42949, 1000,
    429496, 10000,
    4294967, 100000,
    42949672, 1000000,
    429496729, 10000000
};

// SHIFT AN 8-DIGIT WORD TO THE RIGHT n PLACES (DIVIDE BY 10^n)
// word MUST BE POSITIVE
// n = 0-7

BINT shift_right(BINT word, BINT digits)
{
    if(!digits)
        return 0;

    BINT shift = (8 - digits) & 7;

    if(!shift)
        return word;
    BINT *consts = (BINT *) shift_constants + (shift << 1);

    UWORDUNION tmp;

    tmp.w = word * (uint64_t) * consts;

    tmp.w32[0] = word - tmp.w32[1] * shift_constants[((digits & 7) << 1) + 1];

    // DO ONE FINAL CARRY CORRECTION TO PROPERLY SPLIT THE DIGITS
    if(tmp.w32[0] >= (WORD) shift_constants[((digits & 7) << 1) + 1])
        ++tmp.w32[1];

    // HERE tmp.w32[0] HAS THE LOW DIGITS
    // tmp.w32[1] HAS THE HIGH DIGITS, RIGHT JUSTIFIED
    return tmp.w32[1];

}

// SHIFT AND SPLIT DIGITS
// RETURN A 64-BIT NUMBER WITH THE LOW N DIGITS IN THE LOW WORD
// AND THE HIGH (8-N) DIGITS IN THE HIGH WORD
int64_t shift_split(BINT word, BINT digits)
{
    if(!digits)
        return ((int64_t) word) << 32;

    BINT shift = (8 - digits) & 7;

    if(!shift)
        return word;
    BINT *consts = (BINT *) shift_constants + (shift << 1);

    UWORDUNION tmp;

    tmp.w = word * (uint64_t) * consts;

    tmp.w32[0] = word - tmp.w32[1] * shift_constants[((digits & 7) << 1) + 1];

    // DO ONE FINAL CARRY CORRECTION TO PROPERLY SPLIT THE DIGITS
    if(tmp.w32[0] >= (WORD) shift_constants[((digits & 7) << 1) + 1]) {
        ++tmp.w32[1];
        tmp.w32[0] -= (WORD) shift_constants[((digits & 7) << 1) + 1];
    }

    // HERE tmp.w32[0] HAS THE LOW DIGITS
    // tmp.w32[1] HAS THE HIGH DIGITS, RIGHT JUSTIFIED
    return (int64_t) tmp.w;

}

// ISOLATE LOW n DIGITS IN A WORD, DISCARD HI DIGITS

BINT lo_digits(BINT word, BINT digits)
{
    if(!digits)
        return 0;

    BINT shift = (8 - digits) & 7;

    if(!shift)
        return word;
    BINT *consts = (BINT *) shift_constants + (shift << 1);

    UWORDUNION tmp;

    tmp.w = word * (uint64_t) * consts;

    tmp.w32[0] = word - tmp.w32[1] * shift_constants[((digits & 7) << 1) + 1];

    // DO ONE FINAL CARRY CORRECTION TO PROPERLY SPLIT THE DIGITS
    if(tmp.w32[0] >= (WORD) shift_constants[((digits & 7) << 1) + 1]) {
        ++tmp.w32[1];
        tmp.w32[0] -= (WORD) shift_constants[((digits & 7) << 1) + 1];
    }

    // HERE tmp.w32[0] HAS THE LOW DIGITS
    // tmp.w32[1] HAS THE HIGH DIGITS, RIGHT JUSTIFIED
    return tmp.w32[0];
}

// ISOLATE HIGH (8-n) DIGITS IN A WORD, DISCARD LOW DIGITS
// CLEAR THE LOWER n DIGITS IN WORD
BINT hi_digits(BINT word, BINT digits)
{
    if(!digits)
        return 0;

    BINT shift = (8 - digits) & 7;

    if(!shift)
        return word;
    BINT *consts = (BINT *) shift_constants + (shift << 1);

    UWORDUNION tmp;

    tmp.w = word * (uint64_t) * consts;

    tmp.w32[0] = word - tmp.w32[1] * shift_constants[((digits & 7) << 1) + 1];

    // DO ONE FINAL CARRY CORRECTION TO PROPERLY SPLIT THE DIGITS
    if(tmp.w32[0] >= (WORD) shift_constants[((digits & 7) << 1) + 1]) {
        ++tmp.w32[1];
        tmp.w32[0] -= (WORD) shift_constants[((digits & 7) << 1) + 1];
    }

    // HERE tmp.w32[0] HAS THE LOW DIGITS
    // tmp.w32[1] HAS THE HIGH DIGITS, RIGHT JUSTIFIED
    return tmp.w32[1] * shift_constants[((digits & 7) << 1) + 1];
}

// ISOLATE HIGH (8-n) DIGITS IN A WORD, ROUND LOW DIGITS
// CLEAR THE LOWER n DIGITS IN WORD AFTER ROUNDING
BINT hi_digits_rounded(BINT word, BINT digits)
{
    if(digits == 8) {
        if(word >= 50000000)
            return 100000000;
        return 0;
    }

    BINT shift = (8 - digits) & 7;

    if(!shift)
        return word;    // WARNING: THIS IS NOT ROUNDED! digits==0 IS AN INVALID ARGUMENT
    BINT *consts = (BINT *) shift_constants + (shift << 1);

    UWORDUNION tmp;
    word += 5 * shift_constants[((digits & 7) << 1) - 1];
    tmp.w = word * (uint64_t) * consts;

    tmp.w32[0] = word - tmp.w32[1] * shift_constants[((digits & 7) << 1) + 1];

    // DO ONE FINAL CARRY CORRECTION TO PROPERLY SPLIT THE DIGITS
    if(tmp.w32[0] >= (WORD) shift_constants[((digits & 7) << 1) + 1]) {
        ++tmp.w32[1];
        tmp.w32[0] -= (WORD) shift_constants[((digits & 7) << 1) + 1];
    }

    // HERE tmp.w32[0] HAS THE LOW DIGITS
    // tmp.w32[1] HAS THE HIGH DIGITS, RIGHT JUSTIFIED
    return tmp.w32[1] * shift_constants[((digits & 7) << 1) + 1];
}

// SHIFT AND MULTIPLY IN ONE OPERATION:
// PERFORMS result+=n1start*mul*10^shift;
// mul IS A SMALL CONSTANT BETWEEN 1-31
// shift IS IN THE RANGE 0-7

// NEEDS CARRY CORRECTION AFTERWARDS

void add_long_mul_shift(BINT * result, BINT * n1start, BINT nwords, BINT shift,
        BINT mul)
{

    PROTECT_WRITE_AREA(result, nwords);

    BINT K1, K2;
    BINT hi, lo, hi2, lo2, hi3, lo3;

    K1 = shiftmul_K1[((mul - 1) << 3) + shift];
    K2 = shiftmul_K2[((mul - 1) << 3) + shift];

    while(nwords >= 3) {

        hi = (n1start[0] * (int64_t) K1) >> 24;  // 64-bit MULTIPLICATION
        hi2 = (n1start[1] * (int64_t) K1) >> 24; // 64-bit MULTIPLICATION
        hi3 = (n1start[2] * (int64_t) K1) >> 24; // 64-bit MULTIPLICATION

        lo = n1start[0] * K2 - hi * 100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW
        lo2 = n1start[1] * K2 - hi2 * 100000000;        // 32-BIT MULTIPLICATION WITH OVERFLOW
        lo3 = n1start[2] * K2 - hi3 * 100000000;        // 32-BIT MULTIPLICATION WITH OVERFLOW

        result[0] += lo;
        result[1] += hi + lo2;
        result[2] += hi2 + lo3;
        result[3] += hi3;

        result += 3;
        n1start += 3;
        nwords -= 3;
    }

    if(nwords == 2) {

        hi = (n1start[0] * (int64_t) K1) >> 24;  // 64-bit MULTIPLICATION
        hi2 = (n1start[1] * (int64_t) K1) >> 24; // 64-bit MULTIPLICATION

        lo = n1start[0] * K2 - hi * 100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW
        lo2 = n1start[1] * K2 - hi2 * 100000000;        // 32-BIT MULTIPLICATION WITH OVERFLOW

        result[0] += lo;
        result[1] += hi + lo2;
        result[2] += hi2;
        return;
    }

    if(nwords) {

        hi = (n1start[0] * (int64_t) K1) >> 24;  // 64-bit MULTIPLICATION

        lo = n1start[0] * K2 - hi * 100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW

        result[0] += lo;
        result[1] += hi;
    }

}

// SINGLE-STEP SHIFT-AND-ACCUMULATE
// MULTIPLIES BY 10^N AND ADDS INTO result
void add_long_shift(BINT * result, BINT * n1start, BINT nwords, BINT shift)
{

    PROTECT_WRITE_AREA(result, nwords);

    BINT *consts = (BINT *) shift_constants + ((shift & 7) << 1);

    // consts[0]=MULTIPLY-HIGH CONSTANT TO GET THE HIGH WORD
    // X=(2^32-10^8)/10^(8-N)+10^N

    UWORDUNION tmp;
    uint64_t a;

    while(nwords) {
        tmp.w = n1start[nwords - 1] * (uint64_t) * consts;
        result[nwords] += tmp.w32[1];
        a = n1start[nwords - 1] * (uint64_t) consts[1];
        tmp.w = a - tmp.w32[1] * 100000000ULL;
        result[nwords - 1] += tmp.w32[0];
        --nwords;
    }

}

// IN-PLACE SHIFT
// MULTIPLIES BY 10^N AND STORES INTO result
// shift HAS TO BE 0 TO 7
void long_shift(BINT * n1start, BINT nwords, BINT shift)
{

    PROTECT_WRITE_AREA(n1start, nwords);

    BINT *consts = (BINT *) shift_constants + ((shift & 7) << 1);

    // consts[0]=MULTIPLY-HIGH CONSTANT TO GET THE HIGH WORD
    // X=(2^32-10^8)/10^(8-N)+10^N

    UWORDUNION tmp;
    uint64_t a;
    BINT *nptr;
    nptr = n1start + nwords;

    *nptr = 0;

    --nptr;
    while(nptr >= n1start) {
        if(*nptr < 0) {
            tmp.w = (-*nptr) * (uint64_t) * consts;
            a = (-*nptr) * (uint64_t) consts[1];
            nptr[1] -= tmp.w32[1];
            tmp.w = a - tmp.w32[1] * 100000000ULL;
            if(tmp.w32[0] >= 100000000) {
                tmp.w32[0] -= 100000000;
                nptr[1]--;
            }
            *nptr = -tmp.w32[0];
        }
        else {
            tmp.w = (*nptr) * (uint64_t) * consts;
            a = (*nptr) * (uint64_t) consts[1];
            nptr[1] += tmp.w32[1];
            tmp.w = a - tmp.w32[1] * 100000000ULL;
            if(tmp.w32[0] >= 100000000) {
                tmp.w32[0] -= 100000000;
                nptr[1]++;
            }
            *nptr = tmp.w32[0];
        }
        --nptr;
    }

}

// COUNT NUMBER OF SIGNIFICANT USED DIGITS IN A WORD
// WORD MUST BE NORMALIZED AND >0

BINT sig_digits(BINT word)
{

    if(word >= 10000) {
        if(word >= 1000000) {
            // CAN BE 7 OR 8 DIGITS
            if(word >= 10000000)
                return 8;
            else
                return 7;
        }
        else {
            // CAN BE 5 OR 6 DIGITS
            if(word >= 100000)
                return 6;
            else
                return 5;
        }
    }
    else {
        if(word >= 100) {
            // CAN BE 3 OR 4 DIGITS
            if(word >= 1000)
                return 4;
            else
                return 3;
        }
        else {
            // CAN BE 1 OR 2 DIGITS
            if(word >= 10)
                return 2;
            else
                return 1;
        }
    }
}

// LEFT-JUSTIFY THE DATA OF THE NUMBER
// WITHOUT CHANGING THE VALUE
// ADDS TRAILING ZEROS, SO IT MAY NOT BE POSSIBLE TO DETERMINE
// THE ACTUAL NUMBER OF SIGNIFICANT DIGITS
// AFTER THIS OPERATION

void left_justify(REAL * number)
{
    int ndig = sig_digits(number->data[number->len - 1]);

    if(ndig == 8)
        return; // NOTHING TO SHIFT

    ndig = 8 - ndig;    // DIGITS NEEDED FOR THE SHIFT TO FILL THE MOST SIGNIFICANT WORD

    number->exp -= ndig;        // CORRECT THE EXPONENT
    long_shift(number->data, number->len, ndig);        // AND DO THE SHIFT

}

// JUSTIFY THE DATA OF THE NUMBER SO THAT DECIMAL POINT IS AT WORD BOUNDARY
// WITHOUT CHANGING THE VALUE
// ADDS TRAILING ZEROS, SO IT MAY NOT BE POSSIBLE TO DETERMINE
// THE ACTUAL NUMBER OF SIGNIFICANT DIGITS
// AFTER THIS OPERATION

void int_justify(REAL * number)
{
    if(number->exp >= 0)
        return;

    int ndig = (-number->exp) & 7;

    if(!ndig)
        return; // NOTHING TO SHIFT

    int digits =
            ((number->len - 1) << 3) + sig_digits(number->data[number->len -
                1]);
    ndig = 8 - ndig;    // DIGITS NEEDED FOR THE SHIFT TO LINE-UP THE EXPONENT WITH A MULTIPLE OF 8

    number->exp -= ndig;        // CORRECT THE EXPONENT
    long_shift(number->data, number->len, ndig);        // AND DO THE SHIFT

    number->len = (digits + ndig + 7) >> 3;
}

// SAME BUT SUBTRACTING, NO CARRY CHECKS

void sub_long(BINT * result, BINT * n1start, BINT nwords)
{
    while(nwords >= 3) {
        result[nwords - 1] -= n1start[nwords - 1];
        result[nwords - 2] -= n1start[nwords - 2];
        result[nwords - 3] -= n1start[nwords - 3];
        nwords -= 3;
    }

    while(nwords) {
        result[nwords - 1] -= n1start[nwords - 1];
        --nwords;
    }

}

void sub_long_mul_shift(BINT * result, BINT * n1start, BINT nwords, BINT shift,
        BINT mul)
{

    PROTECT_WRITE_AREA(result, nwords);

    BINT K1, K2;
    BINT hi, lo, hi2, lo2, hi3, lo3;

    K1 = shiftmul_K1[((mul - 1) << 3) + shift];
    K2 = shiftmul_K2[((mul - 1) << 3) + shift];

    while(nwords >= 3) {

        hi = (n1start[0] * (int64_t) K1) >> 24;  // 64-bit MULTIPLICATION
        hi2 = (n1start[1] * (int64_t) K1) >> 24; // 64-bit MULTIPLICATION
        hi3 = (n1start[2] * (int64_t) K1) >> 24; // 64-bit MULTIPLICATION

        lo = n1start[0] * K2 - hi * 100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW
        lo2 = n1start[1] * K2 - hi2 * 100000000;        // 32-BIT MULTIPLICATION WITH OVERFLOW
        lo3 = n1start[2] * K2 - hi3 * 100000000;        // 32-BIT MULTIPLICATION WITH OVERFLOW

        result[0] -= lo;
        result[1] -= hi + lo2;
        result[2] -= hi2 + lo3;
        result[3] -= hi3;

        result += 3;
        n1start += 3;
        nwords -= 3;
    }

    if(nwords == 2) {

        hi = (n1start[0] * (int64_t) K1) >> 24;  // 64-bit MULTIPLICATION
        hi2 = (n1start[1] * (int64_t) K1) >> 24; // 64-bit MULTIPLICATION

        lo = n1start[0] * K2 - hi * 100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW
        lo2 = n1start[1] * K2 - hi2 * 100000000;        // 32-BIT MULTIPLICATION WITH OVERFLOW

        result[0] -= lo;
        result[1] -= hi + lo2;
        result[2] -= hi2;
        return;
    }

    if(nwords) {

        hi = (n1start[0] * (int64_t) K1) >> 24;  // 64-bit MULTIPLICATION

        lo = n1start[0] * K2 - hi * 100000000;  // 32-BIT MULTIPLICATION WITH OVERFLOW

        result[0] -= lo;
        result[1] -= hi;
    }

}

// SINGLE-STEP SHIFT-AND-ACCUMULATE
// MULTIPLIES BY 10^N AND ADDS INTO result
void sub_long_shift(BINT * result, BINT * n1start, BINT nwords, BINT shift)
{

    PROTECT_WRITE_AREA(result, nwords);

    BINT *consts = (BINT *) shift_constants + ((shift & 7) << 1);

    // consts[0]=MULTIPLY-HIGH CONSTANT TO GET THE HIGH WORD
    // X=(2^32-10^8)/10^(8-N)+10^N

    UWORDUNION tmp;
    uint64_t a;

    while(nwords) {
        tmp.w = n1start[nwords - 1] * (uint64_t) * consts;
        result[nwords] -= tmp.w32[1];
        a = n1start[nwords - 1] * (uint64_t) consts[1];
        tmp.w = a - tmp.w32[1] * 100000000ULL;
        result[nwords - 1] -= tmp.w32[0];
        --nwords;
    }

}

void zero_words(BINT * ptr, BINT nwords)
{

    PROTECT_WRITE_AREA(ptr, nwords);

    while(nwords >= 3) {
        ptr[nwords - 1] = 0;
        ptr[nwords - 2] = 0;
        ptr[nwords - 3] = 0;
        nwords -= 3;
    }

    while(nwords > 0) {
        ptr[nwords - 1] = 0;
        --nwords;
    }

}

void copy_words(BINT * ptr, BINT * source, BINT nwords)
{

    PROTECT_WRITE_AREA(ptr, nwords);

    if(ptr > source) {
        while(nwords >= 3) {
            ptr[nwords - 1] = source[nwords - 1];
            ptr[nwords - 2] = source[nwords - 2];
            ptr[nwords - 3] = source[nwords - 3];
            nwords -= 3;
        }

        while(nwords > 0) {
            ptr[nwords - 1] = source[nwords - 1];
            --nwords;
        }
    }
    else {
        int len = nwords;
        while(nwords >= 3) {
            ptr[len - nwords] = source[len - nwords];
            ptr[len - nwords + 1] = source[len - nwords + 1];
            ptr[len - nwords + 2] = source[len - nwords + 2];
            nwords -= 3;
        }

        while(nwords > 0) {
            ptr[len - nwords] = source[len - nwords];
            --nwords;
        }

    }

}

void copyReal(REAL * dest, REAL * src)
{
    dest->exp = src->exp;
    dest->flags = src->flags;
    dest->len = src->len;
    copy_words(dest->data, src->data, src->len);
}

// CLONES THE REAL STRUCTURE, BUT POINTS TO THE SAME DATA
void cloneReal(REAL * dest, REAL * src)
{
    dest->exp = src->exp;
    dest->flags = src->flags;
    dest->len = src->len;
    dest->data = src->data;
}

void swapReal(REAL * n1, REAL * n2)
{
    REAL tmp;
    tmp.exp = n1->exp;
    tmp.data = n1->data;
    tmp.flags = n1->flags;
    tmp.len = n1->len;

    n1->exp = n2->exp;
    n1->data = n2->data;
    n1->flags = n2->flags;
    n1->len = n2->len;

    n2->exp = tmp.exp;
    n2->data = tmp.data;
    n2->flags = tmp.flags;
    n2->len = tmp.len;
}

// ADDS 2 REAL NUMBERS AT FULL PRECISION
// NUMBERS SHOULD BE NORMALIZED

void add_real(REAL * r, REAL * a, REAL * b)
{
    REAL c, *result = r;

    if((result->data == a->data) || (result->data == b->data)) {
        // STORE RESULT INTO ALTERNATIVE LOCATION TO PREVENT OVERWRITE
        result = &c;
        result->data = allocRegister();
    }

    // GUARANTEE THAT a>b
    // COMPARE THE NUMBER OF INTEGER DIGITS
    if(sig_digits(a->data[a->len - 1]) + ((a->len - 1) << 3) + a->exp <
            sig_digits(b->data[b->len - 1]) + ((b->len - 1) << 3) + b->exp) {
        REAL *tmp = a;
        a = b;
        b = tmp;
    }
    else if(sig_digits(a->data[a->len - 1]) + ((a->len - 1) << 3) + a->exp ==
            sig_digits(b->data[b->len - 1]) + ((b->len - 1) << 3) + b->exp) {
        if(a->data[a->len - 1] < b->data[b->len - 1]) {
            REAL *tmp = a;
            a = b;
            b = tmp;
        }
    }

    // RESULT WILL TAKE THE SIGN OF a, CANNOT BE ALTERED SINCE b<a
    result->flags = a->flags & F_NEGATIVE;

    // ALIGN b WITH a
    int bshift = b->exp - a->exp;
    int wordshift;
    int smallshift;

    if(bshift < 0) {
        // SHIFT RIGHT
        wordshift = -bshift >> 3;
        smallshift = (-bshift) & 7;

        // MAKE SURE THE SMALL SHIFT IS ALWAYS ON THE MULTIPLY SIDE
        if(smallshift) {
            ++wordshift;
            smallshift = 8 - smallshift;
        }
        wordshift = -wordshift;
    }
    else {
        wordshift = bshift >> 3;
        smallshift = bshift & 7;
    }

    int totalwords = (wordshift < 0) ? a->len - wordshift : a->len;
    int skipbwords = 0, alen = a->len;

    if(totalwords > (Context.precdigits + 23) >> 3) {
        //  THERE'S UNNECESSARY WORDS FOR THE CURRENT PRECISION, SKIP SOME
        skipbwords = totalwords;
        totalwords = ((Context.precdigits + 7) >> 3) + 2;
        skipbwords -= totalwords;
        if(wordshift > 0) {
            skipbwords -= wordshift;
            if(skipbwords < 0)
                skipbwords = 0;
        }
        result->flags |= F_APPROX;
    }

    // COPY THE VAULE a TO THE result
    if(alen > totalwords) {
        // NEED TO SKIP WORDS IN a AS WELL
        //skipbwords+=alen-totalwords;
        wordshift -= alen - totalwords;
        copy_words(result->data, a->data + alen - totalwords, totalwords);
        alen = totalwords;
    }
    else {
        zero_words(result->data, totalwords - alen);
        copy_words(result->data + totalwords - alen, a->data, alen);
    }
    // AND ADD A ZERO FOR POSSIBLE CARRY
    result->data[totalwords] = 0;

    if(skipbwords < b->len) {
        // ADD ONLY IF NUMBERS OVERLAP
        if(smallshift) {
            if((a->flags ^ b->flags) & F_NEGATIVE) {
#ifdef ENABLE_ARM_ASSEMBLY
                sub_long_mul_shift_arm(result->data + totalwords - alen +
                        wordshift + skipbwords, b->data + skipbwords,
                        b->len - skipbwords, (smallshift << 16) | 1);
#else
                sub_long_mul_shift(result->data + totalwords - alen +
                        wordshift + skipbwords, b->data + skipbwords,
                        b->len - skipbwords, smallshift, 1);
#endif
            }
            else {
#ifdef ENABLE_ARM_ASSEMBLY
                add_long_mul_shift_arm(result->data + totalwords - alen +
                        wordshift + skipbwords, b->data + skipbwords,
                        b->len - skipbwords, (smallshift << 16) | 1);
#else
                add_long_mul_shift(result->data + totalwords - alen +
                        wordshift + skipbwords, b->data + skipbwords,
                        b->len - skipbwords, smallshift, 1);
#endif
            }
        }
        else {
            if((a->flags ^ b->flags) & F_NEGATIVE)
                sub_long(result->data + totalwords - alen + wordshift +
                        skipbwords, b->data + skipbwords, b->len - skipbwords);
            else
                add_long(result->data + totalwords - alen + wordshift +
                        skipbwords, b->data + skipbwords, b->len - skipbwords);
        }

    }
    else {
        // THERE'S NO OVERLAP WITHIN THE CURRENT PRECISION
        // DON'T DO THE ADDITION, BUT MARK THE RESULT AS APPROXIMATED

        result->flags |= F_APPROX;

    }
    // NO CARRY CORRECTION

    result->len = totalwords;
    result->flags |= F_NOTNORMALIZED | ((a->flags | b->flags) & F_APPROX);
    result->exp = a->exp - ((totalwords - a->len) << 3);

    // NO ROUNDING OR TRUNCATION

    if(result != r) {
        // COPY THE RESULT TO THE ORIGINALLY REQUESTED LOCATION
        freeRegister(r->data);
        r->data = result->data;
        r->exp = result->exp;
        r->flags = result->flags;
        r->len = result->len;
    }

}

// ADDS 2 REAL NUMBERS, MULTIPLY SECOND OPERAND BY SMALL CONSTANT MULT: 0<MULT<=31
// r=a+b*mult
// NUMBERS SHOULD BE NORMALIZED

void add_real_mul(REAL * r, REAL * a, REAL * b, BINT mult)
{
    REAL c, *result = r;

    if((result->data == a->data) || (result->data == b->data)) {
        // STORE RESULT INTO ALTERNATIVE LOCATION TO PREVENT OVERWRITE
        result = &c;
        result->data = allocRegister();
    }

    /*
       // GUARANTEE THAT a>b
       // COMPARE THE NUMBER OF INTEGER DIGITS
       if(sig_digits(a->data[a->len-1])+((a->len-1)<<3)+a->exp<sig_digits(b->data[b->len-1])+((b->len-1)<<3)+b->exp) {
       REAL *tmp=a;
       a=b;
       b=tmp;
       }
       else if(sig_digits(a->data[a->len-1])+((a->len-1)<<3)+a->exp==sig_digits(b->data[b->len-1])+((b->len-1)<<3)+b->exp) {
       if(a->data[a->len-1]<b->data[b->len-1]) {
       REAL *tmp=a;
       a=b;
       b=tmp;
       }
       }
     */

    // RESULT WILL TAKE THE SIGN OF a, CANNOT BE ALTERED SINCE b<a
    result->flags = a->flags & F_NEGATIVE;

    // ALIGN b WITH a
    int bshift = b->exp - a->exp;
    int wordshift;
    int smallshift;

    if(bshift < 0) {
        // SHIFT RIGHT
        wordshift = -bshift >> 3;
        smallshift = (-bshift) & 7;

        // MAKE SURE THE SMALL SHIFT IS ALWAYS ON THE MULTIPLY SIDE
        if(smallshift) {
            ++wordshift;
            smallshift = 8 - smallshift;
        }
        wordshift = -wordshift;
    }
    else {
        wordshift = bshift >> 3;
        smallshift = bshift & 7;
    }

    int totalwords, apos;
    if(wordshift < 0) {
        if(a->len - wordshift > b->len)
            totalwords = a->len - wordshift;
        else
            totalwords = b->len;
        apos = -wordshift;
    }
    else {
        if(a->len > b->len + wordshift)
            totalwords = a->len;
        else
            totalwords = b->len + wordshift;
        apos = 0;
    }
    int trimwords = 0;

    if(totalwords > ((Context.precdigits + 23) >> 3)) {
        //  THERE'S UNNECESSARY WORDS FOR THE CURRENT PRECISION, SKIP SOME
        trimwords = totalwords - ((Context.precdigits + 23) >> 3);
        totalwords = (Context.precdigits + 23) >> 3;
    }

    // COPY THE VAULE a TO THE result

    if(apos - trimwords > 0) {
        zero_words(result->data, apos - trimwords);
        copy_words(result->data + apos - trimwords, a->data, a->len);
    }
    else
        copy_words(result->data, a->data + trimwords - apos,
                a->len - trimwords + apos);
    if(a->len - trimwords + apos >= 0) {
        if(totalwords > a->len + apos - trimwords)
            zero_words(result->data + apos - trimwords + a->len,
                    totalwords - a->len - apos + trimwords);
    }
    else {
        zero_words(result->data, totalwords);
    }

    // AND ADD A ZERO FOR POSSIBLE CARRY
    result->data[totalwords] = 0;

    int skipbwords = trimwords - apos - wordshift;
    if(skipbwords <= 0) {
        wordshift -= trimwords;
        skipbwords = 0;
    }
    if(apos + wordshift + b->len > -1) {
        // ADD ONLY IF NUMBERS OVERLAP
        if((a->flags ^ b->flags) & F_NEGATIVE) {
#ifdef ENABLE_ARM_ASSEMBLY
            sub_long_mul_shift_arm(result->data + apos + wordshift,
                    b->data + skipbwords, b->len - skipbwords,
                    (smallshift << 16) | mult);
#else

            sub_long_mul_shift(result->data + apos + wordshift,
                    b->data + skipbwords, b->len - skipbwords, smallshift,
                    mult);
#endif
        }
        else {
#ifdef ENABLE_ARM_ASSEMBLY
            add_long_mul_shift_arm(result->data + apos + wordshift,
                    b->data + skipbwords, b->len - skipbwords,
                    (smallshift << 16) | mult);
#else
            add_long_mul_shift(result->data + apos + wordshift,
                    b->data + skipbwords, b->len - skipbwords, smallshift,
                    mult);
#endif
        }

    }
    // NO CARRY CORRECTION

    result->len = totalwords + 1;       // +1 IN CASE b>a AND THE SHIFT ADDED A WORD
    result->flags |=
            F_NOTNORMALIZED | ((a->flags | b->
                flags) & F_APPROX) | (trimwords ? F_APPROX : 0);
    result->exp = a->exp - ((apos - trimwords) << 3);

    // NO ROUNDING OR TRUNCATION

    if(result != r) {
        // COPY THE RESULT TO THE ORIGINALLY REQUESTED LOCATION
        freeRegister(r->data);
        r->data = result->data;
        r->exp = result->exp;
        r->flags = result->flags;
        r->len = result->len;
    }

}

// SUBTRACTS 2 REAL NUMBERS AT FULL PRECISION
// NUMBERS SHOULD BE NORMALIZED

void sub_real(REAL * result, REAL * a, REAL * b)
{
    REAL c;
    // MAKE A NEGATIVE b WITH THE SAME DATA
    c.data = b->data;
    c.exp = b->exp;
    c.flags = b->flags ^ F_NEGATIVE;
    c.len = b->len;
    // AND USE STANDARD ADDITION
    add_real(result, a, &c);
}

void sub_real_mul(REAL * r, REAL * a, REAL * b, BINT mult)
{
    REAL c;
    // MAKE A NEGATIVE b WITH THE SAME DATA
    c.data = b->data;
    c.exp = b->exp;
    c.flags = b->flags ^ F_NEGATIVE;
    c.len = b->len;
    // AND USE STANDARD ADDITION
    add_real_mul(r, a, &c, mult);
}

// ACCUMULATE SMALL INTEGER INTO AN EXISTING REAL (MODIFYING THE ARGUMENT)
// result+=number*10^exponent;
// 0<number<10
// USED BY CORDIC ALGORITHM
void acc_real_int(REAL * result, BINT number, BINT exponent)
{
    BINT wordshift = exponent - result->exp, smallshift;
    if(wordshift < 0) {
        // NEED TO MOVE THE WHOLE NUMBER TO ADD THIS
        if((-wordshift) & 7) {
            smallshift = 8 - ((-wordshift) & 7);
            wordshift = ((-wordshift) >> 3) + 1;
        }
        else {
            smallshift = 0;
            wordshift = (-wordshift) >> 3;
        }
        if(result->len + wordshift > (Context.precdigits >> 3) + 2)
            return;
        copy_words(result->data + wordshift, result->data, result->len);
        zero_words(result->data, wordshift);
        if(smallshift)
            number *= shiftmul_K2[smallshift];
        result->data[0] = number;
        result->exp -= wordshift << 3;
        result->len += wordshift;
        return;
    }
    smallshift = wordshift & 7;
    wordshift >>= 3;
    if(wordshift >= result->len) {
        if(wordshift - result->len > ((Context.precdigits + 7) >> 3) + 2) {
            // NUMBERS DON'T OVERLAP AT CURRENT PRECISION, RETURN THE INTEGER
            result->data[0] = number;   // IF NUMBER IS NEGATIVE IT NEEDS NORMALIZATION
            result->exp = exponent;
            result->len = 1;
            result->flags = 0;
            return;
        }

        if(wordshift >= REAL_REGISTER_STORAGE) {
            // NEED TO SHIFT THE WHOLE NUMBER DOWN

            // NUMBERS DO OVERLAP
            int trim = wordshift - ((Context.precdigits + 7) >> 3) + 2;
            copy_words(result->data, result->data + trim, result->len - trim);
            result->len -= trim;
            result->exp += trim << 3;
            wordshift -= trim;
        }
        zero_words(result->data + result->len, wordshift + 1 - result->len);
        result->len = wordshift + 1;
    }
    if(smallshift)
        number *= shiftmul_K2[smallshift];
    if(result->flags & F_NEGATIVE)
        result->data[wordshift] -= number;
    else
        result->data[wordshift] += number;

}

// MULTIPLY 2 REAL NUMBERS AND ACCUMULATE RESULT
// ALL COEFFICIENTS **MUST** BE POSITIVE
// USES NAIVE METHOD WITH THE KARATSUBA TRICK TO GET A 25% SPEEDUP

void mul_real(REAL * r, REAL * a, REAL * b)
{
    REAL c, *result = r;
    int64_t hi, lo, mid;
    UWORDUNION tmp1, tmp2, tmp3;
    BINT lo32_1, hi32_1, lo32_2, hi32_2, lo32_3, hi32_3;

    if((result->data == a->data) || (result->data == b->data)) {
        // STORE RESULT INTO ALTERNATIVE LOCATION TO PREVENT OVERWRITE
        result = &c;
        result->data = allocRegister();
    }

    result->flags =
            F_NOTNORMALIZED | ((a->flags ^ b->flags) & F_NEGATIVE) | ((a->
                flags | b->flags) & F_APPROX);
    result->exp = a->exp + b->exp;
    result->len = a->len + b->len;

    zero_words(result->data, result->len);

    // MAKE SURE b IS THE SHORTEST NUMBER
    if(a->len < b->len) {
        REAL *tmp = a;
        a = b;
        b = tmp;
    }

#ifdef ENABLE_ARM_ASSEMBLY

    mul_real_arm(result->data, a->data, b->data, (a->len << 16) | (b->len));

#else

    int i, j;

    i = 0;

    // DOUBLE WORD, KARATSUBA 3-MULT LOOP
    while(i < b->len - 1) {
        j = 0;
        while(j < a->len - 1) {

            //add_karatsuba(result->data+i+j,a->data+j,b->data+i);
            // INLINED add_karatsuba

            lo = a->data[j] * (int64_t) b->data[i];
            hi = a->data[j + 1] * (int64_t) b->data[i + 1];
            mid = (a->data[j + 1] + a->data[j]) * (int64_t) (b->data[i] +
                    b->data[i + 1]) - hi - lo;

            // UNROLLED add_single64()
            // NUMBER IS GUARANTEED TO BE POSITIVE
            tmp1.w = lo;
            tmp2.w = mid;
            tmp3.w = hi;
            hi32_1 = (((tmp1.w32[1] << 6) | (tmp1.w32[0] >> 26)) *
                    2882303762ULL) >> 32;
            hi32_2 = (((tmp2.w32[1] << 6) | (tmp2.w32[0] >> 26)) *
                    2882303762ULL) >> 32;
            hi32_3 = (((tmp3.w32[1] << 6) | (tmp3.w32[0] >> 26)) *
                    2882303762ULL) >> 32;
            lo32_1 = tmp1.w + hi32_1 * 4194967296U;
            lo32_2 = tmp2.w + hi32_2 * 4194967296U;
            lo32_3 = tmp3.w + hi32_3 * 4194967296U;
            result->data[i + j] += lo32_1;
            result->data[i + j + 1] += lo32_2 + hi32_1;
            result->data[i + j + 2] += lo32_3 + hi32_2;
            result->data[i + j + 3] += hi32_3;

            j += 2;
        }
        if(j < a->len) {
            //add_single64(result->data+i+j,a->data[j]*(uint64_t)b->data[i]);
            //add_single64(result->data+i+1+j,a->data[j]*(uint64_t)b->data[i+1]);

            // UNROLLED add_single64()
            // NUMBER IS GUARANTEED TO BE POSITIVE
            tmp1.w = a->data[j] * (uint64_t) b->data[i];
            tmp2.w = a->data[j] * (uint64_t) b->data[i + 1];
            hi32_1 = (((tmp1.w32[1] << 6) | (tmp1.w32[0] >> 26)) *
                    2882303762ULL) >> 32;
            hi32_2 = (((tmp2.w32[1] << 6) | (tmp2.w32[0] >> 26)) *
                    2882303762ULL) >> 32;
            lo32_1 = tmp1.w + hi32_1 * 4194967296U;
            lo32_2 = tmp2.w + hi32_2 * 4194967296U;
            result->data[i + j] += lo32_1;
            result->data[i + j + 1] += lo32_2 + hi32_1;
            result->data[i + j + 2] += hi32_2;

        }

        if((i != 0) && !(i & 7))
            carry_correct_pos(result->data + i - 8, a->len + 9);
        i += 2;
    }

    // SINGLE WORD LOOP
    while(i < b->len) {
        j = 0;
        while(j < a->len) {
            //add_single64(result->data+i+j,a->data[j]*(uint64_t)b->data[i]);

            // UNROLLED add_single64()
            // NUMBER IS GUARANTEED TO BE POSITIVE
            tmp1.w = a->data[j] * (uint64_t) b->data[i];
            hi32_1 = (((tmp1.w32[1] << 6) | (tmp1.w32[0] >> 26)) *
                    2882303762ULL) >> 32;
            lo32_1 = tmp1.w + hi32_1 * 4194967296U;
            result->data[i + j] += lo32_1;
            result->data[i + j + 1] += hi32_1;

            ++j;
        }
        if((i != 0) && !(i & 7))
            carry_correct_pos(result->data + i - 8, a->len + 9);
        ++i;
    }

#endif

    // DONE - NO CARRY CORRECTION OR NORMALIZATION HERE

    if(result != r) {
        // COPY THE RESULT TO THE ORIGINALLY REQUESTED LOCATION
        freeRegister(r->data);
        r->data = result->data;
        r->exp = result->exp;
        r->flags = result->flags;
        r->len = result->len;
    }

}

// PERFORM KARATSUBA MULTIPLICATION m x m WORDS
// IF m IS ODD, THE SUBDIVISION LEAVES ONE WORD OUT
// SO LAST WORD IS A SINGLE X m MULTIPLICATION

void mul_long_karatsuba(BINT * result, BINT * a, BINT * b, BINT m)
{

    if(m <= 2) {
        if(m == 1)
            add_single64(result, a[0] * (int64_t) b[0]);
        else
            add_karatsuba(result, a, b);
        return;
    }

    REAL c;

    // CAREFUL, THIS MAY ALLOCATE UP TO 8 REGISTERS FOR TEMP STORAGE
    c.data = allocRegister();

    int newm = m / 2;
    // GET THE LOW PRODUCT
    zero_words(c.data, 2 * newm + 1);
    mul_long_karatsuba(c.data, a, b, newm);
    // ADD IT X0*Y0
    add_long(result, c.data, 2 * newm + 1);
    // ADD IT IN THE MIDDLE POSITION
    add_long(result + newm, c.data, 2 * newm + 1);

    zero_words(c.data, 2 * newm + 1);
    // DO THE HIGH PRODUCT
    mul_long_karatsuba(c.data, a + newm, b + newm, newm);
    // ADDIT IN THE HIGH AND MIDDLE POSITIONS
    add_long(result + 2 * newm, c.data, 2 * newm + 1);
    add_long(result + newm, c.data, 2 * newm + 1);

    // DO THE MIDDLE PRODUCT
    zero_words(c.data, 2 * m + 1);
    // X0-X1
    copy_words(c.data, a, newm);
    sub_long(c.data, a + newm, newm);
    // Y0-Y1
    copy_words(c.data + newm, b, newm);
    sub_long(c.data + newm, b + newm, newm);
    // (X0-X1)*(Y0-Y1)
    mul_long_karatsuba(c.data + m, c.data, c.data + newm, newm);
    // AND ADD IT TO THE RESULT
    sub_long(result + newm, c.data + m, 2 * newm + 1);

    if(m & 1) {
        // ADD THE MISSING SINGLE WORD

        int j;

        // ADD SECOND OPERAND WORD BY THE WHOLE FIRST OPERAND
        for(j = 0; j < m; ++j)
            add_single64(result + m - 1 + j, a[j] * (int64_t) b[m - 1]);

        // ADD FIRST OPERAND WORD BY THE SECOND OPERAND, EXCEPT THE LEFT OVER WORD
        for(j = 0; j < m - 1; ++j)
            add_single64(result + m - 1 + j, b[j] * (int64_t) a[m - 1]);

    }

    carry_correct(result, 2 * m + 1);

    freeRegister(c.data);
}

// MULTIPLY 2 REALS AND ACCUMULATE IN result
// USES FULL KARATSUBA METHOD ADAPTED FOR UNBALANCED OPERANDS TOO
// THIS IS THE OUTER CODE SHELL WITH PROPER INITIALIZATION

void mul_real2(REAL * r, REAL * a, REAL * b)
{
    // THIS INTRO IS ONLY NEEDED IN THE MAIN MULTIPLICATION ROUTINE
    // ALL RECURSIVE CALLS DON'T NEED THIS, THEY ACCUMULATE DIRECTLY
    // ON THE MAIN RESULT

    REAL c, *result = r;

    if((result->data == a->data) || (result->data == b->data)) {
        // STORE RESULT INTO ALTERNATIVE LOCATION TO PREVENT OVERWRITE
        result = &c;
        result->data = allocRegister();
    }

    result->flags = F_NOTNORMALIZED | ((a->flags ^ b->flags) & F_NEGATIVE);
    result->exp = a->exp + b->exp;
    result->len = a->len + b->len;

    zero_words(result->data, result->len);

    // MAKE SURE b IS THE SHORTEST NUMBER
    if(a->len < b->len) {
        REAL *tmp = a;
        a = b;
        b = tmp;
    }

    // DEAL WITH UNBALANCED OPERANDS FIRST

    // WITH a HAVING n WORDS AND b HAVING m WORDS,
    // SPLIT a INTO BLOCKS OF m WORDS
    // AND USE KARATSUBA

    int wordoffset = 0;

    while(wordoffset + b->len <= a->len) {
        // MULTIPLY THE BLOCK
        mul_long_karatsuba(result->data + wordoffset, a->data + wordoffset,
                b->data, b->len);
        // AND MOVE ON TO NEXT BLOCK
        wordoffset += b->len;
    }

    // THEN USE NAIVE MULTIPLICATION FOR THE REST

    if(wordoffset < a->len) {
        int i, j;

        i = 0;

        // DOUBLE WORD, KARATSUBA 3-MULT LOOP
        while(i < b->len - 1) {
            j = wordoffset;
            while(j < a->len - 1) {
                add_karatsuba(result->data + i + j, a->data + j, b->data + i);
                j += 2;
            }
            if(j < a->len) {
                add_single64(result->data + i + j,
                        a->data[j] * (uint64_t) b->data[i]);
                add_single64(result->data + i + 1 + j,
                        a->data[j] * (uint64_t) b->data[i + 1]);
            }
            if(!(i & 7))
                carry_correct(result->data, result->len);
            i += 2;
        }

        // SINGLE WORD LOOP
        while(i < b->len) {
            j = wordoffset;
            while(j < a->len) {
                add_single64(result->data + i + j,
                        a->data[j] * (uint64_t) b->data[i]);
                ++j;
            }
            if(!(i & 7))
                carry_correct(result->data, result->len);

            ++i;
        }

    }

    if(result != r) {
        // COPY THE RESULT TO THE ORIGINALLY REQUESTED LOCATION
        freeRegister(r->data);
        r->data = result->data;
        r->exp = result->exp;
        r->flags = result->flags;
        r->len = result->len;
    }

    // DONE! THIS NUMBER IS NOT NORMALIZED,
    // BUT THAT'S ONLY DONE IN HIGHER LEVEL ROUTINES THAT DEAL WITH SPECIALS

    result->flags |= F_NOTNORMALIZED;

}

// CONVERT TEXT TO A REAL NUMBER
// IT IS UTF8 COMPLIANT, WILL RETURN ERROR IF THERE'S
// ANY INVALID CHARACTERS IN THE text, BETWEEN text AND end

const int const pow10_8[8] = {
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000
};

void newRealFromText(REAL * result, char *text, char *end, uint64_t chars)
{
    int digits = 0;
    int exp = 0;
    int expdone = 0;
    int dotdone = 0;
    BINT word = 0;


    result->flags = 0;
    result->exp = 0;

    if(end <= text) {
        result->len = 0;
        result->exp = RERR_EMPTYSTRING;
        result->flags = F_ERROR;
        return;
    }

    // GET POSSIBLE SIGN
    if(*text == '+')
        ++text;
    else if(*text == '-') {
        result->flags ^= F_NEGATIVE;
        ++text;
    }

    // GET POSSIBLE END DOT, AFTER EXPONENT - ACCEPT BOTH DOT AND TILDE
    if(end[-1] == '.' || end[-1] == '~') {
        result->flags |= F_APPROX;
        --end;
    }

    // HANDLE SPECIALS
    {
        const char *infinitystring = "∞";
        const char *undinfinitystring = "∞̅";
        char *textptr = text, *ptr = (char *)undinfinitystring;
        while(*ptr != 0) {
            if(*textptr != *ptr)
                break;
            ++textptr;
            ++ptr;
        }

        if(*ptr == 0) {
            // THERE WAS AN UNDEFINED INFINITY SIGN THERE
            if(textptr != end) {
                // NOTHING SHOULD BE AFTER INFINITY
                result->len = textptr - text;
                result->exp = RERR_EXTRACHARS;
                result->flags = F_ERROR;
                return;
            }
            result->data[0] = 0;
            result->len = 1;
            result->exp = 0;
            result->flags |= F_UNDINFINITY;
            return;

        }
        textptr = text;
        ptr = (char *)infinitystring;
        while(*ptr != 0) {
            if(*textptr != *ptr)
                break;
            ++textptr;
            ++ptr;
        }
        if(*ptr == 0) {
            // THERE WAS AN INFINITY SIGN THERE
            if(textptr != end) {
                // NOTHING SHOULD BE AFTER INFINITY
                result->len = textptr - text;
                result->exp = RERR_EXTRACHARS;
                result->flags = F_ERROR;
                return;
            }
            result->data[0] = 0;
            result->len = 1;
            result->exp = 0;
            result->flags |= F_INFINITY;
            return;

        }

    }

    // GET DIGITS
    while(end > text) {
        end = utf8rskip(end, text);

        if(expdone == -1) {
            if((*end != 'e') && (*end != 'E')) {
                result->len = end - text;
                result->exp = RERR_BADEXPONENT;
                result->flags = F_ERROR;
                return;
            }
            else {
                if(!digits) {
                    // TEXT ENDS IN LETTER 'E', NO DIGITS AFTERWARDS
                    result->len = end - text;
                    result->exp = RERR_BADEXPONENT;
                    result->flags = F_ERROR;
                    return;
                }

                // EXPONENT LETTER ACCEPTED
                result->exp = word;
                word = 0;
                digits = 0;
                expdone = 1;
                continue;
            }
        }

        if((*end >= '0') && (*end <= '9')) {
            word += (*end - '0') * pow10_8[digits & 7];
            ++digits;
            if(!(digits & 7)) {
                expdone = 1;
                if(digits > REAL_REGISTER_STORAGE * 8) {
                    // EXCEEDED THE NUMBER OF DIGITS ALLOWED
                    // APPLY ROUNDING AND REMOVE ONE WORD
                    if(result->data[0] >= 50000000)
                        ++result->data[1];
                    int j;
                    for(j = 0; j < REAL_REGISTER_STORAGE - 1; ++j)
                        result->data[j] = result->data[j + 1];

                    digits -= 8;
                }
                result->data[(digits >> 3) - 1] = word;
                word = 0;
            }
            continue;
        }

        if((WORD) utf82cp(end, end + 4) == DECIMAL_DOT(chars)) {
            if(dotdone) {
                result->len = end - text;
                result->exp = RERR_DOUBLEDOT;
                result->flags = F_ERROR;
                return;
            }

            if(digits == 0) {
                // IN CASE DECIMAL_DOT IS '.'
                if(*end == '.' || *end == '~')
                    result->flags |= F_APPROX;
                else {
                    result->len = end - text;
                    result->exp = RERR_NODIGITS;
                    result->flags = F_ERROR;
                    return;
                }
            }
            else
                dotdone = 1;

            if(!expdone)
                expdone = 1;
            exp = -digits;
            continue;

        }

        if(*end == '-') {
            if(!expdone) {
                word = -word;
                expdone = -1;
                continue;
            }
            else {
                result->len = end - text;
                result->exp = RERR_BADSIGN;
                result->flags = F_ERROR;
                return;
            }
        }
        if(*end == '+') {
            if(!expdone) {
                expdone = -1;
                continue;
            }
            else {
                result->len = end - text;
                result->exp = RERR_BADSIGN;
                result->flags = F_ERROR;
                return;
            }
        }

        if((*end == 'e') || (*end == 'E')) {

            if(expdone) {
                result->len = end - text;
                result->exp = RERR_BADEXPONENT;
                result->flags = F_ERROR;
                return;
            }
            else {
                if(!digits) {
                    // TEXT ENDS IN LETTER 'E', NO DIGITS AFTERWARDS
                    result->len = end - text;
                    result->exp = RERR_NODIGITS;
                    result->flags = F_ERROR;
                    return;
                }

                // EXPONENT LETTER ACCEPTED
                result->exp = word;
                word = 0;
                digits = 0;
                expdone = 1;
                continue;
            }

        }

        if((WORD) utf82cp(end, end + 4) == FRAC_SEP(chars)) {
            // ONLY ACCEPTED IF WITHIN THE FRACTIONAL PART
            if(dotdone) {
                if(FRAC_SEP(chars) != THOUSAND_SEP(chars)) {
                    result->len = end - text;
                    result->exp = RERR_INVALIDCHAR;
                    result->flags = F_ERROR;
                    return;
                }
                // OTHERWISE IT'S A THOUSAND SEPARATOR
            }
            if(!expdone)
                expdone = 1;
            continue;
        }

        if((WORD) utf82cp(end, end + 4) == THOUSAND_SEP(chars)) {
            // ONLY ACCEPTED IN THE INTEGER PART
            // BUT WE DON'T KNOW SINCE WE ARE GOING BACKWARDS
            // JUST IGNORE THE CHARACTER
            if(!dotdone)
                dotdone = 1;
            if(!expdone)
                expdone = 1;
            continue;
        }

        result->len = end - text;
        result->exp = RERR_INVALIDCHAR;
        result->flags = F_ERROR;
        return;

    }

    if(!digits) {
        // TEXT HAD NO NUMERIC DIGITS??
        result->len = end - text;
        result->exp = RERR_NODIGITS;
        result->flags = F_ERROR;
        return;
    }

    result->data[(digits >> 3)] = word;
    result->len = (digits + 7) >> 3;
    result->exp += exp;

// TODO: DO SOME CLEANUP...

// CHECK EXPONENT RANGE, REPLACE WITH INFINITY AS NEEDED

    if(result->exp > MAX_EXPONENT) {
        // CHANGE TO +/- INFINITY
        result->flags |= F_INFINITY | F_OVERFLOW;
        result->data[0] = 0;
        result->len = 1;
        result->exp = 0;
        return;
    }

    if(result->exp < MIN_EXPONENT) {
        // CHANGE TO APPROX. ZERO
        result->flags =
                F_APPROX | ((result->
                    flags & F_NEGATIVE) ? (F_NEGATIVE | F_NEGUNDERFLOW) :
                F_POSUNDERFLOW);
        result->data[0] = 0;
        result->len = 1;
        result->exp = 0;
        return;
    }

// NUMBER MAY HAVE CARRY PROBLEMS ONLY IF > 2*REAL_PRECISION_MAX DIGITS WERE IN THE STRING
// REDUCE len IF MSW ARE ZERO

    normalize(result);

// ONLY THING NOT DONE HERE IS TRUNCATION TO CURRENT SYSTEM PRECISION, BUT SINCE THIS NUMBER IS
// CONVERTED FROM A STRING, IT'S BETTER TO LEAVE IT AS-IS.

}

// CONVERT A REAL TO FORMATTED TEXT AS FOLLOWS:

// MINIMUM BUFFER SIZE = PRECISION * 4/3 + 10
// 4/3 FOR DIGIT SEPARATOR EVERY 3 DIGITS
// 10 = '-',...,'.',...,'e','-','00000','.'

// format = BIT FLAGS AS FOLLOWS:
// BITS 0-11 = NUMBER OF DIGITS TO DISPLAY (0-4095)

// BIT 12 = 1 -> DECOMPILE FOR CODE
//        = 0 -> DECOMPILE FOR DISPLAY
// BIT 13 = 1 -> SCIENTIFIC NOTATION N.NNNEXXX
//        = 0 -> NORMAL NNNNNN.NNNN
// BIT 14 = 1 -> ENGINEERING NOTATION, MAKE EXPONENT MULTIPLE OF 3
//        = 0 -> LEAVE EXPONENT AS-IS
// BIT 15 = 1 -> FORCE SIGN +1 INSTEAD OF 1
//        = 0 -> SIGN ONLY IF NEGATIVE
// BIT 16 = 1 -> FORCE SIGN E+1 ON EXPONENT
//        = 0 -> EXPONENT SIGN ONLY IF NEGATIVE
// BIT 17 = 1 -> DO NOT ADD TRAILING DOT FOR APPROX. NUMBERS
//        = 0 -> APPROX. NUMBERS USE TRAILING DOT
// BIT 18 = 1 -> ADD TRAILING ZEROS IF NEEDED TO COMPLETE THE NUMBER OF DIGITS (FIX MODE)
// BIT 19 = 1 -> SUPPRESS ZERO EXPONENTS IN SCI MODE
//        = 0 -> IN SCI MODE, USE ZERO EXPONENTS
// BIT 20 = 1 -> ADD SEPARATOR EVERY x DIGITS FOR FRACTION PART
// BIT 21 = 1 -> ADD SEPARATOR EVERY x DIGITS FOR INTEGER PART
// BITS 22-25 = NUMBER OF DIGITS x TO SHOW IN EACH GROUP

// BITS 26-31 = RESERVED FOR FUTURE USE

// EXPLANATION:
// BITS 0-11: IN NORMAL FORMAT: MAX. NUMBER OF DECIMAL FIGURES AFTER THE DOT
//            IN SCIENTIFIC OR ENG NOTATION: MAX TOTAL NUMBER OF SIGNIFICANT FIGURES
// BIT 12: DECOMPILE FOR CODE IGNORES THE NUMBER OF DIGITS, IT INCLUDES ALL DIGITS ON THE STRING
//         ALSO IGNORES SEPARATORS EVERY 3 DIGITS, AND IGNORES THE TRAILING DOT DISABLE BIT
// BIT 13: NNNN.MMMM OR N.NNNMMMMEXXX
// BIT 14: ONLY IF BIT 13 IS SET, CHANGE THE EXPONENT TO BE A MULTIPLE OF 3
// BIT 15: DO +1 INSTEAD OF JUST 1
// BIT 16: DO 1E+10 INSTEAD OF 1E10
// BIT 17: DON'T SHOW 3. IF A NUMBER IS APPROXIMATED, ONLY 3
// BIT 18: SHOW NNNN.MMMM0000 WHEN THE NUMBER OF AVAILABLE DIGITS IS LESS THAN THE REQUESTED NUMBER
// BIT 19: DON'T SHOW ZERO EXPONENT 2.5 INSTEAD OF 2.5E0 IN SCI AND ENG
// BIT 20: SHOW NNNNNNN.MMM MMM MMM
// BIT 21: SHOW NNN,NNN,NNN,NNN.MMMMMM
// BITS 22,23,24,25: NUMBER OF DIGITS IN A GROUP 0-15

// BITS 26-31: RESERVED FOR FUTURE USE

// THE SEPARATORS AND DECIMAL ARE GIVEN IN THE chars ARGUMENT
// LSB = DECIMAL DOT
// 2ND = THOUSAND SEPARATOR
// 3RD = DECIMAL DIGIT SEPARATOR
// MSB = EXPONENT CHARACTER (E OR e)

// ALL FOUR CHARACTERS ARE PACKED IN A 32-BIT WORD

void word2digits(BINT word, char *digits)
{
    int f;

    for(f = 7; f >= 0; --f, ++digits) {
        *digits = '0';
        while(word >= pow10_8[f]) {
            ++digits[0];
            word -= pow10_8[f];
        }
    }
}

// ADD ROUND THE LAST DIGIT OF A STRING
// RETURN THE NUMBER OF DIGITS ADDED TO THE STREAM

int round_in_string(char *start, char *end, int format, uint64_t chars,
        char rounddigit)
{
    if(rounddigit < '5')
        return 0;

    int dotpos = -1;

    char *ptr = utf8rskip(end, start);
    while(ptr >= start) {
        WORD ucode = (WORD) utf82cp(ptr, end);
        if(ucode == FRAC_SEP(chars)) {
            ptr = utf8rskip(ptr, start);
            continue;
        }
        if(ucode == THOUSAND_SEP(chars)) {
            ptr = utf8rskip(ptr, start);
            continue;
        }
        if(ucode == DECIMAL_DOT(chars)) {
            dotpos = ptr - start;
            ptr = utf8rskip(ptr, start);
            continue;
        }
        if((ucode == '-') || (ucode == '+')) {
            ++ptr;
            break;
        }
        if(ucode == '9')
            *ptr = '0';
        else {
            // IT HAS TO BE A DIGIT OTHER THAN 9
            ++(*ptr);
            return 0;
        }
        // THIS CAN ONLY HAPPEN IF THE NUMBER WAS 9
        if(ptr > start)
            ptr = utf8rskip(ptr, start);
        else
            --ptr;
    }

    if(ptr < start)
        ptr = start;
    // THERE WAS CARRY OUTSIDE OF THE EXISTING DIGITS, NEED TO
    // INSERT ONE DIGIT IN THE STREAM, AND POSSIBLY A SEPARATOR

    int adddigit = 1, changeexp = 0;

    if(dotpos == -1)
        dotpos = end - ptr;     // IF THERE WAS NO DOT, IT'S AN INTEGER

    int sep_spacing = SEP_SPACING(format);
    if(!sep_spacing)
        sep_spacing = 3;

    if((format & FMT_NUMSEPARATOR) && !(format & FMT_CODE)) {
        // COUNT HOW MANY DIGITS FROM ptr TO THE DOT

        if((utf8nlen(ptr, ptr + dotpos) + 1) % (sep_spacing + 1) == 0) {
            // NEED TO INSERT AN ADDITIONAL SEPARATOR
            WORD utfsep = cp2utf8(THOUSAND_SEP(chars));
            int nbytes = 0;
            while(utfsep >> (nbytes * 8))
                ++nbytes;
            char *p = end + nbytes - 1;
            while(p >= ptr + nbytes) {
                *p = *(p - nbytes);
                --p;
            }
            p = ptr;
            end += nbytes;
            adddigit += nbytes;
            while(nbytes) {
                *p++ = utfsep & 0xff;
                utfsep >>= 8;
                --nbytes;
            }

        }

    }

    if(format & FMT_SCI) {

        if(format & FMT_ENG) {
            if((format & FMT_NUMSEPARATOR) && !(format & FMT_CODE)) {
                char *p = ptr;
                int intdigits = 0;
                // COUNT NUMBER OF DIGITS IN THE INTEGER PART
                while(p < start + dotpos) {
                    if((*p >= '0') && (*p <= '9'))
                        ++intdigits;
                    ++p;
                }

                if(intdigits == 3) {

                    // MOVE THE DECIMAL DOT 3 PLACES
                    char *p = utf8skip(start + dotpos, end);    // POINT AFTER THE DOT
                    int diff = (p - start) - dotpos;
                    // REMOVE THE OLD DECIMAL DOT
                    while(p >= ptr + diff) {
                        *p = *(p - diff);
                        --p;
                    }

                    // ADD A NEW DOT
                    WORD utfsep = cp2utf8(DECIMAL_DOT(chars));
                    int nbytes = 0;
                    while(utfsep >> (nbytes * 8))
                        ++nbytes;
                    p = ptr;
                    while(nbytes) {
                        *p++ = utfsep & 0xff;
                        utfsep >>= 8;
                        --nbytes;
                    }

                    // REMOVE ANY THOUSAND SEPARATOR BETWEEN THIS CODE AND THE OLD DOT

                    while(p < start + dotpos) {
                        if((WORD) utf82cp(p, end) == THOUSAND_SEP(chars)) {
                            // REMOVE IT, MOVING THE ENTIRE STRING BACK
                            char *p2 = utf8skip(p, end);
                            diff = p2 - p;
                            while(p < end) {
                                *p = *(p + diff);
                                ++p;
                            }
                            p = p2;
                            end -= diff;
                            dotpos -= diff;
                        }
                        else
                            p = utf8skip(p, end);
                    }

                    changeexp = 3;

                    if(format & FMT_FRACSEPARATOR) {
                        // MOVING THE DOT 3 PLACES MEANS ALL SEPARATORS NEED TO BE
                        // COMPLETELY REORGANIZED BASED ON THE NEW DOT POSITION
                        // THIS CAN ONLY HAPPEN IF ALL TRAILING NUMBERS ARE ZEROS

                        // HERE ptr POINTS TO THE NEW DECIMAL DOT
                        char *p = utf8skip(ptr, end);
                        WORD utfsep = cp2utf8(FRAC_SEP(chars));
                        int nbytes = 0;
                        while(utfsep >> (nbytes * 8))
                            ++nbytes;

                        int nzeros = 0;
                        while(p != end) {
                            if(*p == '0')
                                ++nzeros;
                            ++p;
                        }
                        p = utf8skip(ptr, end);
                        int fpos;
                        for(fpos = 0; fpos < nzeros; ++fpos) {
                            if((fpos > 0) && (fpos % sep_spacing == 0)) {
                                int c = nbytes;
                                WORD val = utfsep;
                                // INSERT A SEPARATOR
                                while(c) {
                                    *p++ = (val & 0xff);
                                    val >>= 8;
                                    --c;
                                }
                            }
                            *p++ = '0';
                        }
                        end = p;

                    }

                }
            }
            else {
                // THERE'S NO THOUSAND SEPARATOR
                int pos = ptr - start;
                if(dotpos - pos > 3) {
                    // MOVE DECIMAL POINT
                    while(dotpos != pos) {
                        start[dotpos] = start[dotpos - 1];
                        --dotpos;
                    }
                }

            }

        }
        else
            changeexp = 1;

    }

    char prev = '1', tmp;
    while(ptr != end + adddigit - 1) {
        if(format & FMT_SCI) {
            // DON'T MOVE DECIMAL POINT IN SCI MODE
            // EXPONENT VALUE WILL HAVE TO BE CORRECTED
            // IN MAIN FUNCTION
            if(!(format & FMT_ENG)) {
                if((WORD) utf82cp(ptr,
                            end + adddigit - 1) == THOUSAND_SEP(chars))
                    ptr = utf8skip(ptr, end + adddigit - 1);
                if((WORD) utf82cp(ptr,
                            end + adddigit - 1) == DECIMAL_DOT(chars))
                    ptr = utf8skip(ptr, end + adddigit - 1);
                if((WORD) utf82cp(ptr, end + adddigit - 1) == FRAC_SEP(chars))
                    ptr = utf8skip(ptr, end + adddigit - 1);
            }
            else {
                // IN ENG MODE, MIGHT HAVE TO MOVE THE DECIMAL POINT 3
                // PLACES DUE TO ROUNDING

                // TODO

            }
        }
        tmp = *ptr;
        *ptr = prev;
        prev = tmp;
        ++ptr;
    }
    *ptr = prev;

    if((WORD) utf82cp(utf8rskip(ptr, start),
                end + adddigit - 1) == FRAC_SEP(chars))
        --adddigit;

    return adddigit | (changeexp << 16);

}

// RETURN A POINTER TO THE END OF THE STRING (NOT 0 TERMINATED)

char *formatReal(REAL * number, char *buffer, BINT format, uint64_t chars)
{
    int totaldigits, integer, frac, realexp, leftzeros, sep_spacing;
    int nbnumsep, nbfracsep, nbdot;
    int dotpos;
    int wantdigits = format & 0xfff;
    int wantzeros;
    int countdigits = 0, totalcount;
    int idx = 0;
    char approxSign = rplTestSystemFlag(FL_APPROXSIGN) ? '.' : '~';

    // HANDLE SPECIALS FIRST

    if((number->flags & F_UNDINFINITY) == F_UNDINFINITY) {
        const char *undinfinitystring = "∞̅";
        while(*undinfinitystring != 0) {
            buffer[idx++] = *undinfinitystring;
            ++undinfinitystring;
        }
        return buffer + idx;
    }

    if(number->flags & F_INFINITY) {
        if(number->flags & F_NEGATIVE)
            buffer[idx++] = '-';
        else if(format & FMT_FORCESIGN)
            buffer[idx++] = '+';
        const char *infinitystring = "∞";
        while(*infinitystring != 0) {
            buffer[idx++] = *infinitystring;
            ++infinitystring;
        }
        return buffer + idx;
    }

    if(number->flags & F_NOTANUMBER) {
        if(number->flags & F_NEGATIVE)
            buffer[idx++] = '-';
        else if(format & FMT_FORCESIGN)
            buffer[idx++] = '+';
        const char *nanstring = "NaN";
        while(*nanstring != 0) {
            buffer[idx++] = *nanstring;
            ++nanstring;
        }
        return buffer + idx;
    }

    totaldigits =
            ((number->len - 1) << 3) + sig_digits(number->data[number->len -
                1]);

    // GET SEPARATOR SPACING, DEFAULT TO 3 IF NOT SPECIFIED
    sep_spacing = SEP_SPACING(format);
    if(!sep_spacing)
        sep_spacing = 3;

    if(format & FMT_CODE)
        wantdigits = totaldigits;
    if(format & FMT_SCI) {
        realexp = -(totaldigits - 1) - number->exp;

        if(format & FMT_ENG) {
            if(format & FMT_PREFEXPMSK) {
                // FORCED EXPONENT
                realexp = -PREFERRED_EXP(format);
            }
            else {
                // AUTO ADJUST EXPONENT
                if(realexp < 0)
                    realexp += (-realexp) % 3;
                else if(realexp % 3)
                    realexp += 3 - realexp % 3;
            }
        }

    }
    else
        realexp = 0;
    integer = totaldigits + number->exp + realexp;
    if(integer <= 0) {
        leftzeros = -integer + 1;
        dotpos = 1;
        integer = 0;
        if(format & FMT_CODE) {
            wantzeros = leftzeros;
        }
        else {
            if(leftzeros - 1 > wantdigits) {
                wantzeros = wantdigits + 1;
                wantdigits = 0;
            }
            else {
                wantzeros = leftzeros;
                wantdigits -= leftzeros - 1;
            }
        }
    }
    else {
        leftzeros = 0;
        wantzeros = 0;
        dotpos = integer;
        if(format & FMT_CODE) { if(wantdigits>integer) wantdigits -= integer; else wantdigits=0; }
    }
    frac = totaldigits - integer;

    totalcount = integer + wantzeros + wantdigits;

    // GET THE SIZE OF SEPARATORS
    {
        WORD utfsep = cp2utf8(THOUSAND_SEP(chars));
        nbnumsep = 0;
        while(utfsep >> (nbnumsep * 8))
            ++nbnumsep;
        utfsep = cp2utf8(FRAC_SEP(chars));
        nbfracsep = 0;
        while(utfsep >> (nbfracsep * 8))
            ++nbfracsep;
        utfsep = cp2utf8(DECIMAL_DOT(chars));
        nbdot = 0;
        while(utfsep >> (nbdot * 8))
            ++nbdot;

    }

    if((format & FMT_NUMSEPARATOR) && !(format & FMT_CODE)) {
        int numsep = integer / sep_spacing;
        if((integer % sep_spacing) == 0)
            --numsep;
        if(integer == 0)
            ++numsep;
        dotpos += numsep * nbnumsep;
    }

    // START OUTPUT

    // SIGN

    if(number->flags & F_NEGATIVE) {
        buffer[idx++] = '-';
        ++dotpos;
    }
    else if(format & FMT_FORCESIGN) {
        buffer[idx++] = '+';
        ++dotpos;
    }

    // LEFT ZEROS

    while(leftzeros && (countdigits < wantzeros)) {
        if(countdigits == totalcount) {
            // THIS DIGIT IS OUTSIDE THE REQUESTED DIGITS
            // USE IT FOR ROUNDING ==> NOTHING TO DO, JUST BREAK
            break;

        }
        // INSERT DOT AS NEEDED
        if(idx == dotpos) {
            WORD ucode = cp2utf8(DECIMAL_DOT(chars));
            while(ucode) {
                buffer[idx++] = ucode & 0xff;
                ucode >>= 8;
            }
        }
        // INSERT SEPARATOR AS NEEDED
        else if(!(format & FMT_CODE) && (format & FMT_FRACSEPARATOR)
                && ((idx - dotpos + nbfracsep - nbdot) % (sep_spacing +
                        nbfracsep) == 0)) {
            WORD ucode = cp2utf8(FRAC_SEP(chars));
            while(ucode) {
                buffer[idx++] = ucode & 0xff;
                ucode >>= 8;
            }

        }
        buffer[idx++] = '0';
        ++countdigits;
        --leftzeros;
    }

    // SIGNIFICANT DIGITS

    char worddigits[8];

    int j = number->len - 1;
    int digitcount = 0;
    int i = 0;

    //   INTEGER PART
    while(digitcount < integer) {
        if(i == 0) {
            if(j >= 0)
                word2digits(number->data[j], worddigits);
            else
                word2digits(0, worddigits);
            if(j == number->len - 1)
                i = 8 - sig_digits(number->data[j]);
            --j;
        }
        if(countdigits == totalcount) {
            // THIS DIGIT IS OUTSIDE THE REQUESTED DIGITS
            // USE IT FOR ROUNDING
            int result;
            result = round_in_string(buffer, buffer + idx, format, chars,
                    worddigits[i]);
            idx += result & 0xffff;

            realexp -= result >> 16;
            break;

        }
        // INSERT DOT AS NEEDED
        if(idx == dotpos) {
            WORD ucode = cp2utf8(DECIMAL_DOT(chars));
            while(ucode) {
                buffer[idx++] = ucode & 0xff;
                ucode >>= 8;
            }
        }
        // INSERT SEPARATOR AS NEEDED
        else if(!(format & FMT_CODE) && (format & FMT_NUMSEPARATOR)
                && digitcount
                && (((dotpos - idx) % (sep_spacing + nbnumsep)) == 0)) {
            WORD ucode = cp2utf8(THOUSAND_SEP(chars));
            while(ucode) {
                buffer[idx++] = ucode & 0xff;
                ucode >>= 8;
            }
        }

        buffer[idx++] = worddigits[i];
        ++countdigits;
        ++digitcount;
        ++i;
        i &= 7;
    }

    // FRACTIONAL DIGITS
    digitcount = 0;
    while(digitcount < frac) {
        if(i == 0) {
            word2digits(number->data[j], worddigits);
            if(j == number->len - 1)
                i = 8 - sig_digits(number->data[j]);
            --j;
        }
        if(countdigits == totalcount) {
            // THIS DIGIT IS OUTSIDE THE REQUESTED DIGITS
            // USE IT FOR ROUNDING

            // CHECK IF MORE ZEROS WERE NEEDED, THEN NO ROUNDING NEEDS TO BE DONE
            if(leftzeros)
                break;

            int result;
            result = round_in_string(buffer, buffer + idx, format, chars,
                    worddigits[i]);
            idx += result & 0xffff;
            dotpos += result & 0xffff;
            realexp -= result >> 16;
            break;

        }
        // INSERT DOT AS NEEDED
        if(idx == dotpos) {
            WORD ucode = cp2utf8(DECIMAL_DOT(chars));
            while(ucode) {
                buffer[idx++] = ucode & 0xff;
                ucode >>= 8;
            }
        }
        // INSERT SEPARATOR AS NEEDED
        else if(!(format & FMT_CODE) && (format & FMT_FRACSEPARATOR)
                && (((idx - dotpos + nbfracsep - nbdot) % (sep_spacing +
                            nbfracsep)) == 0)) {
            WORD ucode = cp2utf8(FRAC_SEP(chars));
            while(ucode) {
                buffer[idx++] = ucode & 0xff;
                ucode >>= 8;
            }
        }

        buffer[idx++] = worddigits[i];
        ++countdigits;
        ++digitcount;
        ++i;
        i &= 7;
    }

    // TRAILING ZEROS

    if(format & FMT_TRAILINGZEROS) {
        while(digitcount < wantdigits) {
            // INSERT DOT AS NEEDED
            if(idx == dotpos) {
                WORD ucode = cp2utf8(DECIMAL_DOT(chars));
                while(ucode) {
                    buffer[idx++] = ucode & 0xff;
                    ucode >>= 8;
                }
            }
            // INSERT SEPARATOR AS NEEDED
            else if(!(format & FMT_CODE) && (format & FMT_FRACSEPARATOR)
                    && (((idx - dotpos + nbfracsep - nbdot) % (sep_spacing +
                                nbfracsep)) == 0)) {
                WORD ucode = cp2utf8(FRAC_SEP(chars));
                while(ucode) {
                    buffer[idx++] = ucode & 0xff;
                    ucode >>= 8;
                }
            }

            buffer[idx++] = '0';
            ++countdigits;
            ++digitcount;
        }
    }
    else {
        // NO TRAILING ZEROS, REMOVE ZEROS IF NUMBER WASN'T RIGHT-JUSTIFIED
        if(idx > dotpos) {
            char *prevbuf;

            WORD ucode;
            do {
                prevbuf = utf8rskip(&buffer[idx], buffer);
                ucode = utf82cp(prevbuf, &buffer[idx]);
                if((ucode == '0') || (ucode == FRAC_SEP(chars))) {
                    idx = prevbuf - buffer;
                }
                else
                    break;
            }
            while(prevbuf > buffer);
            if(prevbuf - buffer == dotpos) {
                // ALSO REMOVE DECIMAL POINT
                idx = prevbuf - buffer;
            }
        }
    }

    if((number->flags & F_APPROX) && (!(format & FMT_NOTRAILDOT)
                || (format & FMT_CODE)))
        buffer[idx++] = approxSign;

    // EXPONENT

    if(format & FMT_SCI) {
        if(!((format & FMT_SUPRESSEXP) && ((realexp == 0)
                        || (format & FMT_PREFEXPMSK)))) {
            buffer[idx++] = EXP_LETTER(format);
            realexp = -realexp;
            if(realexp < 0) {
                buffer[idx++] = '-';
                realexp = -realexp;
            }
            else if(format & FMT_EXPSIGN)
                buffer[idx++] = '+';
            int sd = sig_digits(realexp);

            word2digits(realexp, worddigits);
            sd = 8 - sd;

            while(sd < 8)
                buffer[idx++] = worddigits[sd++];
        }
    }

    //buffer[idx]=0;

    return buffer + idx;
}

BINT formatlengthReal(REAL * number, BINT format, uint64_t locale)
{
    int totaldigits, integer, realexp, leftzeros, sep_spacing;
    int wantdigits = format & 0xfff;
    int trailzeros;
    int wantzeros;
    int totalcount;
    int idx = 0;

    if((number->flags & F_UNDINFINITY) == F_UNDINFINITY) {
        const char *undinfinitystring = "∞̅";
        while(*undinfinitystring != 0) {
            idx++;
            ++undinfinitystring;
        }
        return idx;
    }

    if(number->flags & F_INFINITY) {
        if(number->flags & F_NEGATIVE)
            idx++;
        else if(format & FMT_FORCESIGN)
            idx++;
        const char *infinitystring = "∞";
        while(*infinitystring != 0) {
            idx++;
            ++infinitystring;
        }
        return idx;
    }

    if(number->flags & F_NOTANUMBER) {
        if(number->flags & F_NEGATIVE)
            idx++;
        else if(format & FMT_FORCESIGN)
            idx++;
        const char *nanstring = "NaN";
        while(*nanstring != 0) {
            idx++;
            ++nanstring;
        }
        return idx;
    }

    totaldigits =
            ((number->len - 1) << 3) + sig_digits(number->data[number->len -
                1]);

    // GET SEPARATOR SPACING, DEFAULT TO 3 IF NOT SPECIFIED
    sep_spacing = SEP_SPACING(format);
    if(!sep_spacing)
        sep_spacing = 3;

    if(format & FMT_CODE)
        wantdigits = totaldigits;
    if(format & FMT_SCI) {
        realexp = -(totaldigits - 1) - number->exp;

        if(format & FMT_ENG) {
            if(format & FMT_PREFEXPMSK) {
                // FORCED EXPONENT
                realexp = -PREFERRED_EXP(format);
            }
            else {
                // AUTO ADJUST EXPONENT
                if(realexp < 0)
                    realexp += (-realexp) % 3;
                else if(realexp % 3)
                    realexp += 3 - realexp % 3;
            }
        }
    }
    else
        realexp = 0;
    integer = totaldigits + number->exp + realexp;
    if(integer <= 0) {
        leftzeros = -integer + 1;
        integer = 0;
        if(format & FMT_CODE) {
            wantzeros = leftzeros;
        }
        else {
            if(leftzeros - 1 > wantdigits) {
                wantzeros = wantdigits + 1;
                wantdigits = 0;
            }
            else {
                wantzeros = leftzeros;
                wantdigits -= leftzeros - 1;
            }
        }
    }
    else {
        leftzeros = 0;
        wantzeros = 0;
    }
    trailzeros = wantdigits + integer - totaldigits;
    if(trailzeros < 0)
        trailzeros = 0;
    else {
        wantdigits -= trailzeros;
        if(wantdigits < 0)
            wantdigits = 0;
        if(!(format & FMT_TRAILINGZEROS))
            trailzeros = 0;
    }

    totalcount = integer + wantzeros + wantdigits + trailzeros;
    if(format & (FMT_NUMSEPARATOR | FMT_FRACSEPARATOR)) {
        int nbnumsep, nbfracsep;
        // GET THE SIZE OF SEPARATORS
        {
            WORD utfsep = cp2utf8(THOUSAND_SEP(locale));
            nbnumsep = 0;
            while(utfsep >> (nbnumsep * 8))
                ++nbnumsep;
            utfsep = cp2utf8(FRAC_SEP(locale));
            nbfracsep = 0;
            while(utfsep >> (nbfracsep * 8))
                ++nbfracsep;
        }

        if(format & FMT_NUMSEPARATOR) {
            int numsep = integer / sep_spacing;
            if((integer % sep_spacing) == 0)
                --numsep;
            if(integer == 0)
                ++numsep;
            totalcount += numsep * nbnumsep;
        }

        if(format & FMT_FRACSEPARATOR) {
            int numfsep = (wantzeros + wantdigits + trailzeros) / sep_spacing;
            if(((wantzeros + wantdigits + trailzeros) % sep_spacing) == 0)
                --numfsep;
            if((wantzeros + wantdigits + trailzeros) == 0)
                ++numfsep;
            totalcount += numfsep * nbfracsep;
        }
    }

    totalcount += 1;    // SIGN AND DECIMAL DOT
    {
        WORD utfdot = cp2utf8(DECIMAL_DOT(locale));
        int nbdot = 0;
        while(utfdot >> (nbdot * 8))
            ++nbdot;
        totalcount += nbdot;
    }

    if((number->flags & F_APPROX) && (!(format & FMT_NOTRAILDOT)
                || (format & FMT_CODE)))
        totalcount++;   // TRAILING DOT

    // EXPONENT

    if(format & FMT_SCI) {
        if(!((format & FMT_SUPRESSEXP) && (realexp == 0))) {
            if(realexp < 0)
                totalcount += sig_digits(-realexp) + 2;
            else
                totalcount += sig_digits(realexp) + 2;  // DIGITS + SIGN + E LETTER
        }
    }

    return totalcount;
}

// DIVIDES 2 REALS (OBTAIN DIVISION ONLY, NOT REMAINDER)
// OBTAIN AT LEAST MAXDIGITS SIGNIFICANT FIGURES
// USES LONG DIVISION ALGORITHM

void div_real(REAL * r, REAL * num, REAL * d, int maxdigits)
{

    REAL c, *result = r;
    REAL dd, *div = d;

    if((result->data == num->data) || (result->data == div->data)) {
        // STORE RESULT INTO ALTERNATIVE LOCATION TO PREVENT OVERWRITE
        result = &c;
        result->data = allocRegister();
    }

    result->flags =
            F_NOTNORMALIZED | ((num->flags ^ div->flags) & F_NEGATIVE) | ((num->
                flags | div->flags) & F_APPROX);
    result->len = (maxdigits + 7) >> 3;
    result->len += 2;   // DO EXTRA 16 DIGITS FOR PROPER ROUNDING

    if(result->len < num->len)
        result->len = num->len;

    if(sig_digits(div->data[div->len - 1]) < 8) {
        dd.data = allocRegister();
        dd.exp = d->exp;
        dd.flags = d->flags;
        dd.len = d->len;
        div = &dd;
        copy_words(dd.data, d->data, dd.len);
        // LEFT-JUSTIFY THE DIVISOR
        left_justify(div);
    }
    else
        dd.data = 0;

    // COMPUTE THE EXPONENT BASED ON THE FIRST WORD
    int numexp = ((num->len - 1) << 3) + num->exp;
    int divexp = ((div->len - 1) << 3) + div->exp;

    result->exp = -((result->len - 1) << 3) + numexp - divexp;

    //zero_words(result->data,result->len);

    int resword = result->len - 1, remword;
    int j;

    // DETERMINE MULTIPLICATIVE INVERSE OF THE FIRST WORD
    uint64_t inverse = ((1ULL << 63) / div->data[div->len - 1]) >> 26;
    uint64_t invhi = (100000000ULL << 28) / div->data[div->len - 1];

    REAL remainder;

    remainder.data = allocRegister();
    remainder.exp = num->exp;
    remainder.flags = num->flags;
    remainder.len = div->len + result->len;
    zero_words(remainder.data, remainder.len - num->len);
    copy_words(remainder.data + remainder.len - num->len, num->data, num->len);
    remainder.data[remainder.len] = 0;  // EXTRA ZERO PADDING FOR THE ALGORITHM

    remword = remainder.len - 1;        // TAKE THE FIRST WORD TO START
    BINT tempres;
    int64_t tmp64;
    while(resword >= 0) {
        // COMPUTE A NEW WORD OF THE QUOTIENT
        result->data[resword] = 0;
        tmp64 = 0;
        do {
            if(remainder.data[remword + 1] < 0)
                tempres = -(((-remainder.data[remword + 1]) * invhi) >> 28);
            else
                tempres = (remainder.data[remword + 1] * invhi) >> 28;
            if(remainder.data[remword] < 0)
                tempres -= ((-remainder.data[remword]) * inverse) >> 37;
            else
                tempres += (remainder.data[remword] * inverse) >> 37;

            if(tempres) {
                // SUBTRACT FROM THE REMAINDER
                for(j = 0; j < div->len; ++j)
                    add_single64(remainder.data + remword + 1 - div->len + j,
                            -(int64_t) tempres * div->data[j]);
                carry_correct(remainder.data + remword + 1 - div->len,
                        div->len);
            }
            else {
                if(tmp64) {
                    if(tmp64 < 0)
                        tempres = -1;
                    else
                        tempres = 1;
                    // SUBTRACT FROM THE REMAINDER
                    for(j = 0; j < div->len; ++j)
                        add_single64(remainder.data + remword + 1 - div->len +
                                j, -(int64_t) tempres * div->data[j]);
                    carry_correct(remainder.data + remword + 1 - div->len,
                            div->len);
                }
            }
            // CORRECT THE MOST SIGNIFICANT WORD OF THE REMAINDER
            tmp64 = (int64_t) remainder.data[remword] +
                    (int64_t) remainder.data[remword + 1] * 100000000LL;
            if(tmp64 > 2147483648LL || tmp64 < -214783648LL) {
                carry_correct(remainder.data + remword, 2);
            }
            else {
                remainder.data[remword] = tmp64;
                remainder.data[remword + 1] = 0;
            }

            result->data[resword] += tempres;

        }
        while((tmp64 > div->data[div->len - 1]));

        --resword;
        --remword;
    }

    BINT *ptr = remainder.data + remword + 2 - div->len;
    BINT *divptr = div->data;
    BINT eq = 1, zero = 1;
    while((ptr < remainder.data + remword + 2)) {
        if(*ptr != *divptr)
            eq = 0;
        if(*ptr != 0)
            zero = 0;
        ++ptr;
        ++divptr;
    }
    if(eq) {
        // THIS WAS AN EXACT DIVISION
        // NEED TO INCREASE BY ONE THE RESULT
        ++result->data[0];
    }
    else {
        if(!zero)
            result->flags |= F_APPROX;
    }

    freeRegister(remainder.data);
    if(dd.data)
        freeRegister(dd.data);

    if(result != r) {
        // COPY THE RESULT TO THE ORIGINALLY REQUESTED LOCATION
        freeRegister(r->data);
        r->data = result->data;
        r->exp = result->exp;
        r->flags = result->flags;
        r->len = result->len;
    }

}

// DIVIDE A NUMBER USING NEWTON-RAPHSON INVERSION

void div_real_nr(REAL * result, REAL * num, REAL * div)
{

    REAL xi;

    // FIRST ROUGH APPROXIMATION
    xi.data = allocRegister();  // WARNING, NO CHECK FOR OUT OF MEMORY
    //xi.data[0]=1;
    //xi.len=1;
    //xi.flags=0;
    //xi.exp=-(sig_digits(div->data[div->len-1])+((div->len-1)<<3)+div->exp);   // ESTIMATE OF THE INVERSE BY THE EXPONENT

    REAL one;
    one.data = allocRegister();
    one.data[0] = 1;
    one.len = 1;
    one.flags = 0;
    one.exp = 0;

    div_real(&xi, &one, div, 32);       // ROUGH INITIAL APPROXIMATION  TO 32 DIGITS

    REAL tmp[2];
    tmp[0].data = allocRegister();
    tmp[1].data = allocRegister();

    // X(i+1)=Xi + Xi*(1-D*Xi)
    do {
        mul_real2(&tmp[0], div, &xi);   // D*Xi
        normalize(&tmp[0]);
        round_real(&tmp[0], Context.precdigits + 8, 0);
        sub_real(&tmp[1], &one, &tmp[0]);       // 1-D*Xi
        normalize(&tmp[1]);
        round_real(&tmp[1], Context.precdigits + 8, 0);
        mul_real2(&tmp[0], &xi, &tmp[1]);       // Xi*(1-D*Xi)
        normalize(&tmp[0]);
        round_real(&tmp[0], Context.precdigits + 8, 0);
        add_real(&tmp[1], &xi, &tmp[0]);        // Xi + Xi*(1-D*Xi)
        normalize(&tmp[1]);
        round_real(&tmp[1], Context.precdigits + 8, 0);
        sub_real(&tmp[0], &xi, &tmp[1]);
        normalize(&tmp[0]);
        round_real(&tmp[0], Context.precdigits, 0);
        copyReal(&xi, &tmp[1]);

    }
    while((tmp[0].len > 1) || (tmp[0].data[0] > 10000));        // END ONCE WE OBTAIN LESS THAN 4 DIGITS DIFFERENCE

    mul_real(result, num, &xi);
    finalize(result);

    freeRegister(xi.data);
    freeRegister(tmp[0].data);
    freeRegister(tmp[1].data);
    freeRegister(one.data);
}

// *************************************************************************
// ************* HIGH LEVEL API FOR DECIMAL LIBRARY ************************
// *************************************************************************

// INITIALIZE A REAL, OBTAIN STORAGE FOR IT.

void initReal(REAL * a)
{
    a->data = allocRegister();
    a->exp = 0;
    a->flags = 0;
    a->len = 0;
}

// RELEASE MEMORY USED BY REAL

void destroyReal(REAL * a)
{
    freeRegister(a->data);
}

// SELECT WORKING PRECISION

void setPrecision(BINT prec)
{
    if(prec < 0)
        prec = 32;
    if(prec > REAL_PRECISION_MAX)
        prec = REAL_PRECISION_MAX;
    Context.precdigits = prec;
}

// GET THE CURRENT PRECISION

BINT getPrecision()
{
    return Context.precdigits;
}

// MAKE A REAL OUT OF AN INTEGER
void newRealFromBINT(REAL * result, BINT number, BINT exp10)
{
    if(number < 0) {
        result->flags = F_NEGATIVE;
        number = -number;
    }
    else
        result->flags = 0;

    result->exp = exp10;

    if(number < 100000000) {
        result->data[0] = number;
        result->len = 1;
        return;
    }
    result->data[0] = 0;
    result->data[1] = 0;
    add_single64(result->data, (int64_t) number);
    carry_correct(result->data, 2);
    result->len = 2;
}

// MAKE A REAL OUT OF A 64-BIT INTEGER
void newRealFromint64_t(REAL * result, int64_t number, BINT exp10)
{
    if(number < 0) {
        result->flags = F_NEGATIVE;
        number = -number;
    }
    else
        result->flags = 0;

    result->exp = exp10;

    if(number < 100000000) {
        // SINGLE WORD CASE
        result->data[0] = number;
        result->len = 1;
        return;
    }
    if(number < 10000000000000000LL) {
        // UP TO 10^16 CAN BE HANDLED BY add_single64();
        result->data[0] = 0;
        result->data[1] = 0;
        add_single64(result->data, (int64_t) number);
        carry_correct(result->data, 2);
        result->len = 2;
        return;
    }

    // DEAL WITH NUMBER THAT IS MORE THAN 16-DIGITS
    result->data[0] = 0;
    result->data[1] = 0;
    result->data[2] = 0;
    // THIS IS 2^48 IN 8-DIGIT BASE
    const BINT two_48[2] = { 76710656, 2814749 };
    int64_t hibits = number >> 48;
    number &= (1LL << 48) - 1;
    add_single64(result->data, number);
    add_karatsuba(result->data, (BINT *) & hibits, (BINT *) two_48);
    carry_correct(result->data, 3);
    result->len = 3;
    return;
}

// ADDITION OF 2 REALS
// DEALS WITH SPECIALS AND FULLY FINALIZE THE ANSWER

void addReal(REAL * result, REAL * a, REAL * b)
{

    if((a->flags | b->flags) & (F_INFINITY | F_NOTANUMBER)) {
        if((a->flags & F_UNDINFINITY) == F_UNDINFINITY) {
            if(b->flags & (F_INFINITY | F_NOTANUMBER)) {
                // THE RESULT IS NOT A NUMBER
                result->data[0] = 0;
                result->exp = 0;
                result->len = 1;
                result->flags = F_NOTANUMBER;
                return;
            }
            // SINCE b IS A NUMBER, THE RESULT IS STILL UNDIRECTED INFINITY
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = F_UNDINFINITY;
            return;
        }

        if((b->flags & F_UNDINFINITY) == F_UNDINFINITY) {
            if(b->flags & (F_INFINITY | F_NOTANUMBER)) {
                // THE RESULT IS NOT A NUMBER
                result->data[0] = 0;
                result->exp = 0;
                result->len = 1;
                result->flags = F_NOTANUMBER;
                return;
            }
            // SINCE a IS A NUMBER, THE RESULT IS STILL UNDIRECTED INFINITY
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = F_UNDINFINITY;
            return;

        }

        if((a->flags | b->flags) & F_NOTANUMBER) {
            // THE RESULT IS NOT A NUMBER
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = F_NOTANUMBER;
            return;
        }
        // DEAL WITH SPECIALS
        if(a->flags & F_INFINITY) {
            if(((b->flags ^ a->flags) & (F_INFINITY | F_NEGATIVE)) ==
                    F_NEGATIVE) {
                // INF - INF IS UNDETERMINED, ALL OTHER COMBINATIONS RETURN INFINITY
                result->data[0] = 0;
                result->exp = 0;
                result->len = 1;
                result->flags = F_NOTANUMBER;
                return;
            }
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = a->flags;
            return;
        }
        if(b->flags & F_INFINITY) {
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = b->flags;
            return;
        }
    }

    if(a->len == 1 && a->data[0] == 0)
        copyReal(result, b);
    else if(b->len == 1 && b->data[0] == 0)
        copyReal(result, a);
    else
        add_real(result, a, b);
    finalize(result);
}

// ADDITION OF 2 REALS
// DEALS WITH SPECIALS AND FULLY FINALIZE THE ANSWER

void subReal(REAL * result, REAL * a, REAL * b)
{

    REAL c;
    // MAKE A NEGATIVE b WITH THE SAME DATA
    c.data = b->data;
    c.exp = b->exp;
    c.flags = b->flags ^ F_NEGATIVE;
    c.len = b->len;
    // AND USE STANDARD ADDITION
    addReal(result, a, &c);
}

// MULTIPLICATION OF 2 REALS
// DEALS WITH SPECIALS AND FULLY FINALIZES ANSWER

void mulReal(REAL * result, REAL * a, REAL * b)
{

    if((a->flags | b->flags) & (F_INFINITY | F_NOTANUMBER)) {

        if((a->flags & F_UNDINFINITY) == F_UNDINFINITY) {
            if((b->flags & (F_INFINITY | F_NOTANUMBER)) == F_NOTANUMBER) {
                // THE RESULT IS NOT A NUMBER
                result->data[0] = 0;
                result->exp = 0;
                result->len = 1;
                result->flags = F_NOTANUMBER;
                return;

            }
            if(!(b->flags & (F_INFINITY | F_NOTANUMBER)) && (b->len == 1)
                    && (b->data[0] == 0)) {
                // INF * 0 = NOT A NUMBER
                result->data[0] = 0;
                result->exp = 0;
                result->len = 1;
                result->flags = F_NOTANUMBER;
                return;

            }
            // IN ALL OTHER CASES, THIS IS UNDIRECTED INFINITY
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = F_UNDINFINITY;
            return;
        }

        if((b->flags & F_UNDINFINITY) == F_UNDINFINITY) {
            if((a->flags & (F_INFINITY | F_NOTANUMBER)) == F_NOTANUMBER) {
                // THE RESULT IS NOT A NUMBER
                result->data[0] = 0;
                result->exp = 0;
                result->len = 1;
                result->flags = F_NOTANUMBER;
                return;

            }
            if(!(a->flags & (F_INFINITY | F_NOTANUMBER)) && (a->len == 1)
                    && (a->data[0] == 0)) {
                // INF * 0 = NOT A NUMBER
                result->data[0] = 0;
                result->exp = 0;
                result->len = 1;
                result->flags = F_NOTANUMBER;
                return;

            }
            // IN ALL OTHER CASES, THIS IS UNDIRECTED INFINITY
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = F_UNDINFINITY;
            return;
        }

        if((a->flags | b->flags) & F_NOTANUMBER) {
            // THE RESULT IS NOT A NUMBER
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = F_NOTANUMBER;
            return;
        }
        // DEAL WITH SPECIALS
        if(a->flags & F_INFINITY) {
            if(!(b->flags & F_INFINITY) && (b->len == 1) && (b->data[0] == 0)) {
                // INF * 0 IS UNDETERMINED, ALL OTHER COMBINATIONS RETURN INFINITY
                result->data[0] = 0;
                result->exp = 0;
                result->len = 1;
                result->flags = F_NOTANUMBER;
                return;
            }
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = a->flags ^ (b->flags & F_NEGATIVE);
            return;
        }
        if(b->flags & F_INFINITY) {
            if(!(a->flags & F_INFINITY) && (a->len == 1) && (a->data[0] == 0)) {
                // INF * 0 IS UNDETERMINED, ALL OTHER COMBINATIONS RETURN INFINITY
                result->data[0] = 0;
                result->exp = 0;
                result->len = 1;
                result->flags = F_NOTANUMBER;
                return;
            }
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = b->flags ^ (a->flags & F_NEGATIVE);
            return;
        }
    }

    mul_real(result, a, b);
    finalize(result);

}

// DIVIDE 2 REALS, DEAL WITH SPECIALS

void divReal(REAL * result, REAL * a, REAL * b)
{

    if((a->flags | b->flags) & (F_INFINITY | F_NOTANUMBER)) {

        if((a->flags & F_UNDINFINITY) == F_UNDINFINITY) {
            if(b->flags & (F_INFINITY | F_NOTANUMBER)) {
                // THE RESULT IS NOT A NUMBER
                result->data[0] = 0;
                result->exp = 0;
                result->len = 1;
                result->flags = F_NOTANUMBER;
                return;
            }
            // b IS A NUMBER
            if((b->len == 1) && (b->data[0] == 0)) {
                // INF / 0 = NOT A NUMBER
                result->data[0] = 0;
                result->exp = 0;
                result->len = 1;
                result->flags = F_NOTANUMBER;
                return;
            }
            // ALL OTHER CASES THIS IS STILL UNDIRECTED INFINITY
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = F_UNDINFINITY;
            return;
        }

        if((b->flags & F_UNDINFINITY) == F_UNDINFINITY) {
            if(a->flags & (F_INFINITY | F_NOTANUMBER)) {
                // THE RESULT IS NOT A NUMBER
                result->data[0] = 0;
                result->exp = 0;
                result->len = 1;
                result->flags = F_NOTANUMBER;
                return;
            }
            // a IS A NUMBER, a/INF = 0

            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = 0;
            return;
        }

        if((a->flags | b->flags) & F_NOTANUMBER) {
            // THE RESULT IS NOT A NUMBER
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = F_NOTANUMBER;
            return;
        }
        // DEAL WITH SPECIALS
        if(a->flags & F_INFINITY) {
            if((b->flags & F_INFINITY) || ((b->len == 1) && (b->data[0] == 0))) {
                // INF/INF AND INF/0 IS UNDETERMINED, ALL OTHER COMBINATIONS RETURN INFINITY
                result->data[0] = 0;
                result->exp = 0;
                result->len = 1;
                result->flags = F_NOTANUMBER;
                return;
            }
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = a->flags ^ (b->flags & F_NEGATIVE);
            return;
        }
        if(b->flags & F_INFINITY) {
            // INF/INF WAS CAUGHT PREVIOUSLY, SO ALL CASES HERE RETURN 0
            result->data[0] = 0;
            result->exp = 0;
            result->len = 1;
            result->flags = (a->flags ^ b->flags) & F_NEGATIVE;
            return;
        }
    }

    if((b->len == 1) && (b->data[0] == 0)) {
        // DIVISION BY ZERO, RETURN INFINITY
        result->data[0] = 0;
        result->exp = 0;
        result->len = 1;
        if((a->len == 1) && (a->data[0] == 0)) {
            // 0/0 = UNDEFINED
            result->flags = ((a->flags ^ b->flags) & F_NEGATIVE) | F_NOTANUMBER;
        }
        else
            result->flags = ((a->flags ^ b->flags) & F_NEGATIVE) | F_INFINITY;
        return;
    }

    div_real(result, a, b, Context.precdigits);
    finalize(result);

}

// RETURN THE INTEGER QUOTIENT AND REMAINDER OF AN INTEGER DIVISION
// quotient= IP(a/b)
// remainder= a - quotient*b

void divmodReal(REAL * quotient, REAL * remainder, REAL * a, REAL * b)
{

    if((a->flags | b->flags) & (F_INFINITY | F_NOTANUMBER)) {

        if((a->flags & F_UNDINFINITY) == F_UNDINFINITY) {
            if(b->flags & (F_INFINITY | F_NOTANUMBER)) {
                // THE RESULT IS NOT A NUMBER
                quotient->data[0] = remainder->data[0] = 0;
                quotient->exp = remainder->exp = 0;
                quotient->len = remainder->len = 1;
                quotient->flags = remainder->flags = F_NOTANUMBER;
                return;
            }
            // b IS A NUMBER
            if((b->len == 1) && (b->data[0] == 0)) {
                // INF / 0 = NOT A NUMBER
                quotient->data[0] = remainder->data[0] = 0;
                quotient->exp = remainder->exp = 0;
                quotient->len = remainder->len = 1;
                quotient->flags = remainder->flags = F_NOTANUMBER;
                return;
            }
            // ALL OTHER CASES THIS IS STILL UNDIRECTED INFINITY
            quotient->data[0] = remainder->data[0] = 0;
            quotient->exp = remainder->exp = 0;
            quotient->len = remainder->len = 1;
            quotient->flags = remainder->flags = F_UNDINFINITY;
            return;
        }

        if((b->flags & F_UNDINFINITY) == F_UNDINFINITY) {
            if(a->flags & (F_INFINITY | F_NOTANUMBER)) {
                // THE RESULT IS NOT A NUMBER
                quotient->data[0] = remainder->data[0] = 0;
                quotient->exp = remainder->exp = 0;
                quotient->len = remainder->len = 1;
                quotient->flags = remainder->flags = F_NOTANUMBER;
                return;
            }
            // a IS A NUMBER, a/INF = 0

            quotient->data[0] = remainder->data[0] = 0;
            quotient->exp = remainder->exp = 0;
            quotient->len = remainder->len = 1;
            quotient->flags = remainder->flags = a->flags & F_NEGATIVE;
            return;
        }

        if((a->flags | b->flags) & F_NOTANUMBER) {
            // THE RESULT IS NOT A NUMBER
            quotient->data[0] = remainder->data[0] = 0;
            quotient->exp = remainder->exp = 0;
            quotient->len = remainder->len = 1;
            quotient->flags = remainder->flags = F_NOTANUMBER;
            return;
        }
        // DEAL WITH SPECIALS
        if(a->flags & F_INFINITY) {
            if((b->flags & F_INFINITY) || ((b->len == 1) && (b->data[0] == 0))) {
                // INF/INF AND INF/0 IS UNDETERMINED, ALL OTHER COMBINATIONS RETURN INFINITY
                quotient->data[0] = remainder->data[0] = 0;
                quotient->exp = remainder->exp = 0;
                quotient->len = remainder->len = 1;
                quotient->flags = remainder->flags = F_NOTANUMBER;
                return;
            }
            quotient->data[0] = remainder->data[0] = 0;
            quotient->exp = remainder->exp = 0;
            quotient->len = remainder->len = 1;
            quotient->flags = remainder->flags =
                    (a->flags ^ b->flags) & F_NEGATIVE;
            return;
        }
        if(b->flags & F_INFINITY) {
            // INF/INF WAS CAUGHT PREVIOUSLY, SO ALL CASES HERE RETURN 0
            quotient->data[0] = remainder->data[0] = 0;
            quotient->exp = remainder->exp = 0;
            quotient->len = remainder->len = 1;
            quotient->flags = remainder->flags = 0;
            return;
        }
    }

    if((b->len == 1) && (b->data[0] == 0)) {
        // DIVISION BY ZERO, RETURN INFINITY
        quotient->data[0] = remainder->data[0] = 0;
        quotient->exp = remainder->exp = 0;
        quotient->len = remainder->len = 1;
        quotient->flags = remainder->flags =
                ((a->flags ^ b->flags) & F_NEGATIVE) | F_INFINITY;
        return;
    }

// MAKE SURE WE ARE USING DIFFERENT STORAGE FOR INTERMEDIATE RESULTS

    REAL c, *quot = quotient;
    REAL d, *rem = remainder;

    if((quot->data == a->data) || (quot->data == b->data)) {
        // STORE RESULT INTO ALTERNATIVE LOCATION TO PREVENT OVERWRITE
        quot = &c;
        quot->data = allocRegister();
    }

    if((rem->data == a->data) || (rem->data == b->data)) {
        // STORE RESULT INTO ALTERNATIVE LOCATION TO PREVENT OVERWRITE
        rem = &d;
        rem->data = allocRegister();
    }

// COMPUTE THE MINIMUM NUMBER OF WORDS TO OBTAIN INTEGER PART

    BINT ndigits =
            ((a->len - 1) << 3) + sig_digits(a->data[a->len - 1]) + a->exp -
            ((b->len - 1) << 3) - sig_digits(b->data[b->len - 1]) - b->exp;

    if(ndigits > REAL_PRECISION_MAX)
        ndigits = REAL_PRECISION_MAX;

    if(ndigits < 0) {
// NUMERATOR IS SMALLER THAN DIVISOR, QUOTIENT IS ZERO
        quot->data[0] = 0;
        quot->len = 1;
        quot->exp = 0;
        quot->flags = 0;
    }
    else {
        div_real(quot, a, b, ndigits);
        quot->flags = (quot->flags & ~F_APPROX) | ((a->flags | b->flags) & F_APPROX);   // ONLY APPROX. IF EITHER ARGUMENT WAS APPROX.
        normalize(quot);
    }
    if(quot->exp < 0) {
        BINT ndigits =
                sig_digits(quot->data[quot->len - 1]) + ((quot->len - 1) << 3);
        if(ndigits + quot->exp < 0) {
            // RESULT OF DIVISION IS ZERO, RETURN a
            if(rem != a)
                copyReal(rem, a);

            if(quot != quotient) {
                // COPY THE RESULT TO THE ORIGINALLY REQUESTED LOCATION
                freeRegister(quotient->data);
                quotient->data = quot->data;
                quotient->exp = quot->exp;
                quotient->flags = quot->flags;
                quotient->len = quot->len;
            }

            if(rem != remainder) {
                // COPY THE RESULT TO THE ORIGINALLY REQUESTED LOCATION
                freeRegister(remainder->data);
                remainder->data = rem->data;
                remainder->exp = rem->exp;
                remainder->flags = rem->flags;
                remainder->len = rem->len;
            }

            return;
        }
        round_real(quot, ndigits + quot->exp, 1);
        normalize(quot);
        if((a->flags & F_APPROX) || (b->flags & F_APPROX))
            quot->flags |= F_APPROX;
        else
            quot->flags &= ~F_APPROX;   // REMOVE THE APPROXIMATED FLAG AFTER TRUNCATION
    }
    mul_real(rem, quot, b);
    normalize(rem);
    sub_real(rem, a, rem);
    finalize(rem);

    if(quot != quotient) {
        // COPY THE RESULT TO THE ORIGINALLY REQUESTED LOCATION
        freeRegister(quotient->data);
        quotient->data = quot->data;
        quotient->exp = quot->exp;
        quotient->flags = quot->flags;
        quotient->len = quot->len;
    }

    if(rem != remainder) {
        // COPY THE RESULT TO THE ORIGINALLY REQUESTED LOCATION
        freeRegister(remainder->data);
        remainder->data = rem->data;
        remainder->exp = rem->exp;
        remainder->flags = rem->flags;
        remainder->len = rem->len;
    }

}

// ROUND A REAL NUMBER TO A CERTAIN NUMBER OF DIGITS AFTER DECIMAL DOT
// IF NFIGURES IS NEGATIVE, NFIGURES = TOTAL NUMBER OF SIGNIFICANT DIGITS
// HANDLE SPECIALS

void roundReal(REAL * result, REAL * num, BINT nfigures)
{
    if(result != num)
        copyReal(result, num);

    if(result->flags & (F_INFINITY | F_NOTANUMBER))
        return;

    if(nfigures < 0)
        round_real(result, -nfigures, 0);
    else {
        BINT ndigits =
                sig_digits(result->data[result->len - 1]) + ((result->len -
                    1) << 3);
        nfigures += result->exp;
        if(nfigures >= 0)
            return;     // NOTHING TO DO
        if(ndigits + nfigures < 0) {
            // RETURN ZERO
            result->data[0] = 0;
            result->len = 1;
            result->exp = 0;
            result->flags = 0;
            return;
        }
        round_real(result, ndigits + nfigures, 0);
    }

}

// TRUNCATE A REAL NUMBER TO A CERTAIN NUMBER OF DIGITS AFTER DECIMAL DOT
// IF NFIGURES IS NEGATIVE, NFIGURES = TOTAL NUMBER OF SIGNIFICANT DIGITS
// HANDLE SPECIALS

void truncReal(REAL * result, REAL * num, BINT nfigures)
{
    if(result != num)
        copyReal(result, num);

    if(result->flags & (F_INFINITY | F_NOTANUMBER))
        return;

    if(nfigures < 0)
        round_real(result, -nfigures, 1);
    else {
        BINT ndigits =
                sig_digits(result->data[result->len - 1]) + ((result->len -
                    1) << 3);
        nfigures += result->exp;
        if(nfigures >= 0)
            return;     // NOTHING TO DO
        if(ndigits + nfigures < 0) {
            // RETURN ZERO
            result->data[0] = 0;
            result->len = 1;
            result->exp = 0;
            result->flags = 0;
            return;
        }
        round_real(result, ndigits + nfigures, 1);
    }

}

// RETURN THE INTEGER PART (TRUNCATED)
// RETURN THE NUMBER ALIGNED AND RIGHT-JUSTIFIED IF align IS TRUE
void ipReal(REAL * result, REAL * num, BINT align)
{
    truncReal(result, num, 0);

    if(align) {
        int_justify(result);
        normalize(result);
    }
}

// RETURN THE FRACTION PART ONLY
void fracReal(REAL * result, REAL * num)
{

    if(result != num)
        copyReal(result, num);

    if(result->flags & (F_INFINITY | F_NOTANUMBER))
        return;

    if(result->exp >= 0) {
        // RETURN 0, THERE'S NO FRACTIONAL PART
        result->data[0] = 0;
        result->len = 1;
        result->exp = 0;
        result->flags &= ~F_NEGATIVE;
        return;
    }
    BINT digits =
            sig_digits(result->data[result->len - 1]) + ((result->len -
                1) << 3);

    if(digits + result->exp <= 0)
        return; // NOTHING TO DO, NUMBER IS ALL FRACTIONAL

    // TRIM NUMBER
    result->len = (-result->exp + 7) >> 3;
    if((-result->exp) & 7)
        result->data[result->len - 1] =
                lo_digits(result->data[result->len - 1], (-result->exp) & 7);

    // REMOVE LEFT ZEROES
    while((result->len > 1) && (result->data[result->len - 1] == 0))
        --result->len;

    // DONE, NO CARRY CORRECTION NEEDED
}

// COMPARE 2 REALS, RETURN 1 WHEN a<b

BINT ltReal(REAL * a, REAL * b)
{
    // TEST 1, SIGN
    if((a->flags ^ b->flags) & F_NEGATIVE) {
        // BOTH NUMBERS HAVE DIFFERENT SIGNS
        if(a->flags & F_NEGATIVE)
            return 1;
        else
            return 0;
    }

    // NUMBERS HAVE THE SAME SIGNS

    // CHECK FOR SPECIALS
    if((a->flags | b->flags) & F_NOTANUMBER) {
        // IF ANY OF THE NUMBERS IS NOT A NUMBER, THE COMPARISON MAKES NO SENSE
        // COMPARISONS ARE ALWAYS FALSE
        return 0;
    }
    if(a->flags & F_INFINITY) {
        if(a->flags & F_NEGATIVE) {
            // A <= B
            if(b->flags & F_INFINITY)
                return 0;
            return 1;
        }
        else {
            // A >= B
            return 0;
        }
    }

    // TEST 1.B, IF BOTH HAVE THE SAME SIGN, CHECK FOR ZERO

    if((a->len == 1) && (a->data[0] == 0)) {
        if((b->len == 1) && (b->data[0] == 0))
            return 0;   // A==B
        return 1;       // A<B SINCE B>0 PER SIGN TEST
    }
    if((b->len == 1) && (b->data[0] == 0))
        return 0;       // A>0

    // TEST 2, POSITION OF THE MOST SIGNIFICANT DIGIT

    int digits_a, digits_b;

    digits_a = sig_digits(a->data[a->len - 1]) + ((a->len - 1) << 3) + a->exp;
    digits_b = sig_digits(b->data[b->len - 1]) + ((b->len - 1) << 3) + b->exp;

    if(digits_a > digits_b) {
        // ABS(A)>ABS(B)
        if(a->flags & F_NEGATIVE)
            return 1;   // A<B
        return 0;
    }
    if(digits_a < digits_b) {
        // ABS(A)<ABS(B)
        if(a->flags & F_NEGATIVE)
            return 0;   // A>B
        return 1;
    }

    // HERE NUMBERS HAVE THE SAME SIGN AND SAME ORDER OF MAGNITUDE
    // NEED TO DO A SUBTRACTION
    REAL c;
    c.data = allocRegister();

    sub_real(&c, a, b);

    normalize(&c);

    if(c.len == 1 && c.data[0] == 0) {
        freeRegister(c.data);
        return 0;
    }
    freeRegister(c.data);
    if(c.flags & F_NEGATIVE)
        return 1;
    return 0;
}

BINT gtReal(REAL * a, REAL * b)
{
    // TEST 1, SIGN
    if((a->flags ^ b->flags) & F_NEGATIVE) {
        // BOTH NUMBERS HAVE DIFFERENT SIGNS
        if(a->flags & F_NEGATIVE)
            return 0;
        else
            return 1;
    }

    // NUMBERS HAVE THE SAME SIGNS

    // CHECK FOR SPECIALS
    if((a->flags | b->flags) & F_NOTANUMBER) {
        // IF ANY OF THE NUMBERS IS NOT A NUMBER, THE COMPARISON MAKES NO SENSE
        // COMPARISONS ARE ALWAYS FALSE
        return 0;
    }
    if(a->flags & F_INFINITY) {
        if(!(a->flags & F_NEGATIVE)) {
            // A >= B
            if(b->flags & F_INFINITY)
                return 0;
            return 1;
        }
        else {
            // A <= B
            return 0;
        }
    }

    // TEST 1.B, IF BOTH HAVE THE SAME SIGN, CHECK FOR ZERO

    if((b->len == 1) && (b->data[0] == 0)) {
        if((a->len == 1) && (a->data[0] == 0))
            return 0;   // A==B
        return 1;       // A>B SINCE A>0 PER SIGN TEST
    }
    if((a->len == 1) && (a->data[0] == 0))
        return 0;       // A<B

    // TEST 2, POSITION OF THE MOST SIGNIFICANT DIGIT

    int digits_a, digits_b;

    digits_a = sig_digits(a->data[a->len - 1]) + ((a->len - 1) << 3) + a->exp;
    digits_b = sig_digits(b->data[b->len - 1]) + ((b->len - 1) << 3) + b->exp;

    if(digits_a > digits_b) {
        // ABS(A)>ABS(B)
        if(a->flags & F_NEGATIVE)
            return 0;   // A<B
        return 1;
    }
    if(digits_a < digits_b) {
        // ABS(A)<ABS(B)
        if(a->flags & F_NEGATIVE)
            return 1;   // A>B
        return 0;
    }

    // HERE NUMBERS HAVE THE SAME SIGN AND SAME ORDER OF MAGNITUDE
    // NEED TO DO A SUBTRACTION
    REAL c;
    c.data = allocRegister();

    sub_real(&c, a, b);

    normalize(&c);

    if(c.len == 1 && c.data[0] == 0) {
        freeRegister(c.data);   // A==B
        return 0;
    }
    freeRegister(c.data);
    if(c.flags & F_NEGATIVE)
        return 0;
    return 1;
}

BINT lteReal(REAL * a, REAL * b)
{
    return !gtReal(a, b);
}

BINT gteReal(REAL * a, REAL * b)
{
    return !ltReal(a, b);
}

BINT eqReal(REAL * a, REAL * b)
{
    // TEST 1, SIGN
    if((a->flags ^ b->flags) & F_NEGATIVE) {
        return 0;
    }

    // NUMBERS HAVE THE SAME SIGNS

    // CHECK FOR SPECIALS
    if((a->flags | b->flags) & F_NOTANUMBER) {
        // IF ANY OF THE NUMBERS IS NOT A NUMBER, THE COMPARISON MAKES NO SENSE
        // COMPARISONS ARE ALWAYS FALSE
        return 0;
    }
    if(a->flags & F_INFINITY) {
        if(b->flags & F_INFINITY)
            return 1;
        return 0;
    }

    // TEST 1.B, IF BOTH HAVE THE SAME SIGN, CHECK FOR ZERO

    if((a->len == 1) && (a->data[0] == 0)) {
        if((b->len == 1) && (b->data[0] == 0))
            return 1;   // A==B
        return 0;
    }
    if((b->len == 1) && (b->data[0] == 0))
        return 0;       // A!=0

    // TEST 2, POSITION OF THE MOST SIGNIFICANT DIGIT

    int digits_a, digits_b;

    digits_a = sig_digits(a->data[a->len - 1]) + ((a->len - 1) << 3) + a->exp;
    digits_b = sig_digits(b->data[b->len - 1]) + ((b->len - 1) << 3) + b->exp;

    if(digits_a != digits_b)
        return 0;

    // HERE NUMBERS HAVE THE SAME SIGN AND SAME ORDER OF MAGNITUDE
    // NEED TO DO A SUBTRACTION
    REAL c;
    c.data = allocRegister();

    sub_real(&c, a, b);

    normalize(&c);

    if(c.len == 1 && c.data[0] == 0) {
        freeRegister(c.data);   // A==B
        return 1;
    }
    freeRegister(c.data);
    return 0;
}

// RETURN -1 IF A<B, 0 IF A==B AND 1 IF A>B, -2 IF NAN
// NAN HANDLING IS NOT CONSISTENT WITH OTHER TESTS
// ALL OTHER TESTS FAIL ON NAN, THERE'S NO FAIL CODE IN cmpReal

BINT cmpReal(REAL * a, REAL * b)
{
    // TEST 1, SIGN
    if((a->flags ^ b->flags) & F_NEGATIVE) {
        // BOTH NUMBERS HAVE DIFFERENT SIGNS
        if(a->flags & F_NEGATIVE)
            return -1;
        else
            return 1;
    }

    // NUMBERS HAVE THE SAME SIGNS

    // CHECK FOR SPECIALS
    if((a->flags | b->flags) & F_NOTANUMBER) {
        // IF ANY OF THE NUMBERS IS NOT A NUMBER, THE COMPARISON MAKES NO SENSE
        // COMPARISONS ARE ALWAYS FALSE
        return -2;
    }
    if(a->flags & F_INFINITY) {
        if((a->flags & F_NEGATIVE)) {
            // A <= B
            if(b->flags & F_INFINITY)
                return 0;
            return -1;
        }
        else {
            // A >= B
            if(b->flags & F_INFINITY)
                return 0;
            return 1;
        }
    }

    // TEST 1.B, IF BOTH HAVE THE SAME SIGN, CHECK FOR ZERO

    if((a->len == 1) && (a->data[0] == 0)) {
        if((b->len == 1) && (b->data[0] == 0))
            return 0;   // A==B
        return -1;      // A<B SINCE B>0 PER SIGN TEST
    }
    if((b->len == 1) && (b->data[0] == 0))
        return 1;       // A>B SINCE A>0 PER SIGN TEST

    // TEST 2, POSITION OF THE MOST SIGNIFICANT DIGIT

    int digits_a, digits_b;

    digits_a = sig_digits(a->data[a->len - 1]) + ((a->len - 1) << 3) + a->exp;
    digits_b = sig_digits(b->data[b->len - 1]) + ((b->len - 1) << 3) + b->exp;

    if(digits_a > digits_b) {
        // ABS(A)>ABS(B)
        if(a->flags & F_NEGATIVE)
            return -1;  // A<B
        return 1;
    }
    if(digits_a < digits_b) {
        // ABS(A)<ABS(B)
        if(a->flags & F_NEGATIVE)
            return 1;   // A>B
        return -1;
    }

    // HERE NUMBERS HAVE THE SAME SIGN AND SAME ORDER OF MAGNITUDE
    // NEED TO DO A SUBTRACTION
    REAL c;
    c.data = allocRegister();

    sub_real(&c, a, b);

    normalize(&c);

    if(c.len == 1 && c.data[0] == 0) {
        freeRegister(c.data);   // A==B
        return 0;
    }
    freeRegister(c.data);
    if(c.flags & F_NEGATIVE)
        return -1;
    return 1;
}

// TRUE IF REAL IS ZERO

BINT iszeroReal(REAL * n)
{
    if(n->flags & (F_INFINITY | F_NOTANUMBER))
        return 0;
    if(n->len != 1)
        return 0;
    if(n->data[0] != 0)
        return 0;
    return 1;
}

// TRUE IF THE REAL IS INIFINTY (+ OR -)

BINT isinfiniteReal(REAL * n)
{
    if(n->flags & F_INFINITY)
        return 1;
    return 0;
}

BINT isundinfiniteReal(REAL * n)
{
    if((n->flags & F_UNDINFINITY) == F_UNDINFINITY)
        return 1;
    return 0;
}

BINT isNANReal(REAL * n)
{
    if(n->flags & F_NOTANUMBER)
        return 1;
    return 0;
}

BINT isNANorinfiniteReal(REAL * n)
{
    if(n->flags & F_UNDINFINITY)
        return 1;
    return 0;
}

BINT signofReal(REAL * n)
{
    if(n->flags & F_NEGATIVE)
        return -1;
    else
        return 1;
}

BINT inBINTRange(REAL * n)
{

    if(n->flags & (F_NOTANUMBER | F_INFINITY))
        return 0;

    const BINT const max_bint[] = {
        2147483647,
        214748364,
        21474836,
        2147483,
        214748,
        21474,
        2147,
        214,
        21,
        2
    };
    int digits = ((n->len - 1) << 3) + sig_digits(n->data[n->len - 1]) + n->exp;

    if(digits < 10)
        return 1;
    if(digits > 10)
        return 0;

    // THE NUMBER HAS EXACTLY 10 DIGITS
    if(n->exp >= 0) {
        int64_t result;
        if(n->len > 1)
            result = n->data[1] * 100000000LL;
        else
            result = 0;
        result += n->data[0];
        // THE NUMBER IS MISSING SOME DIGITS
        if(result <= max_bint[n->exp])
            return 1;
        return 0;
    }

    // NEGATIVE EXPONENT
    int idx = (-n->exp) >> 3;

    switch ((-n->exp) & 7) {
    case 0:
        if(n->data[idx + 1] > 21)
            return 0;
        if(n->data[idx + 1] < 21)
            return 1;
        if(n->data[idx] <= 47483647)
            return 1;
        return 0;
    case 1:
    {
        if(n->data[idx + 1] > 214)
            return 0;
        if(n->data[idx + 1] < 214)
            return 1;
        if(shift_right(n->data[idx], 1) <= 7483647)
            return 1;
        return 0;
    }
    case 2:
    {
        if(n->data[idx + 1] > 2147)
            return 0;
        if(n->data[idx + 1] < 2147)
            return 1;
        if(shift_right(n->data[idx], 2) <= 483647)
            return 1;
        return 0;
    }

    case 3:
    {
        if(n->data[idx + 1] > 21474)
            return 0;
        if(n->data[idx + 1] < 21474)
            return 1;
        if(shift_right(n->data[idx], 3) <= 83647)
            return 1;
        return 0;
    }

    case 4:
    {
        if(n->data[idx + 1] > 214748)
            return 0;
        if(n->data[idx + 1] < 214748)
            return 1;
        if(shift_right(n->data[idx], 4) <= 3647)
            return 1;
        return 0;
    }

    case 5:
    {
        if(n->data[idx + 1] > 2147483)
            return 0;
        if(n->data[idx + 1] < 2147483)
            return 1;
        if(shift_right(n->data[idx], 5) <= 647)
            return 1;
        return 0;
    }

    case 6:
    {
        if(n->data[idx + 1] > 21474836)
            return 0;
        if(n->data[idx + 1] < 21474836)
            return 1;
        if(shift_right(n->data[idx], 6) <= 47)
            return 1;
        return 0;
    }

    case 7:
    {
        if(n->data[idx + 2] > 2)
            return 0;
        if(n->data[idx + 2] < 2)
            return 1;
        if(n->data[idx + 1] > 14748364)
            return 0;
        if(n->data[idx + 1] < 14748364)
            return 1;
        if(shift_right(n->data[idx], 7) <= 7)
            return 1;
    }
    }
    return 0;

}

BINT inint64_tRange(REAL * n)
{

    if(n->flags & (F_NOTANUMBER | F_INFINITY))
        return 0;

    static const int64_t const max_bint64[] = {
        9223372036854775807,
        922337203685477580,
        92233720368547758,
        9223372036854775,
        922337203685477,
        92233720368547,
        9223372036854,
        922337203685,
        92233720368,
        9223372036,
        922337203,
        92233720,
        9223372,
        922337,
        92233,
        9223,
        922,
        92,
        9
    };

    // 922 33720368 54775807

    int digits = ((n->len - 1) << 3) + sig_digits(n->data[n->len - 1]) + n->exp;

    if(digits < 19)
        return 1;
    if(digits > 19)
        return 0;

    // THE NUMBER HAS EXACTLY 19 DIGITS
    if(n->exp > 0) {
        int64_t result;
        if(n->len > 2)
            result = n->data[2] * 1000000000000000LL;
        else
            result = 0;
        if(n->len > 1)
            result += n->data[1] * 100000000LL;
        result += n->data[0];
        // THE NUMBER IS MISSING SOME DIGITS
        if(result <= max_bint64[n->exp])
            return 1;
        return 0;
    }
    if(n->exp == 0) {
        if(n->data[2] < 922)
            return 1;
        if(n->data[2] > 922)
            return 0;
        if(n->data[1] < 33720368)
            return 1;
        if(n->data[1] > 33720368)
            return 0;
        if(n->data[0] <= 54775807)
            return 1;
        return 0;
    }

    // NEGATIVE EXPONENT
    int idx = (-n->exp) >> 3;

    switch ((-n->exp) & 7) {
    case 0:
        if(n->data[idx + 2] < 922)
            return 1;
        if(n->data[idx + 2] > 922)
            return 0;
        if(n->data[idx + 1] < 33720368)
            return 1;
        if(n->data[idx + 1] > 33720368)
            return 0;
        if(n->data[idx] <= 54775807)
            return 1;
        return 0;
    case 1:
    {
        if(n->data[idx + 2] < 9223)
            return 1;
        if(n->data[idx + 2] > 9223)
            return 0;
        if(n->data[idx + 1] < 37203685)
            return 1;
        if(n->data[idx + 1] > 37203685)
            return 0;
        if(shift_right(n->data[idx], 1) <= 4775807)
            return 1;
        return 0;
    }
    case 2:
    {
        if(n->data[idx + 2] < 92233)
            return 1;
        if(n->data[idx + 2] > 92233)
            return 0;
        if(n->data[idx + 1] < 72036854)
            return 1;
        if(n->data[idx + 1] > 72036854)
            return 0;
        if(shift_right(n->data[idx], 2) <= 775807)
            return 1;
        return 0;
    }

    case 3:
    {
        if(n->data[idx + 2] < 922337)
            return 1;
        if(n->data[idx + 2] > 922337)
            return 0;
        if(n->data[idx + 1] < 20368547)
            return 1;
        if(n->data[idx + 1] > 20368547)
            return 0;
        if(shift_right(n->data[idx], 3) <= 75807)
            return 1;
        return 0;
    }

    case 4:
    {
        if(n->data[idx + 2] < 9223372)
            return 1;
        if(n->data[idx + 2] > 9223372)
            return 0;
        if(n->data[idx + 1] < 3685477)
            return 1;
        if(n->data[idx + 1] > 3685477)
            return 0;
        if(shift_right(n->data[idx], 4) <= 5807)
            return 1;
        return 0;
    }

    case 5:
    {
        if(n->data[idx + 2] < 92233720)
            return 1;
        if(n->data[idx + 2] > 92233720)
            return 0;
        if(n->data[idx + 1] < 36854775)
            return 1;
        if(n->data[idx + 1] > 36854775)
            return 0;
        if(shift_right(n->data[idx], 5) <= 807)
            return 1;
        return 0;
    }

    case 6:
    {
        if(n->data[idx + 3] < 9)
            return 1;
        if(n->data[idx + 3] > 9)
            return 0;
        if(n->data[idx + 2] < 22337203)
            return 1;
        if(n->data[idx + 2] > 22337203)
            return 0;
        if(n->data[idx + 1] < 68547758)
            return 1;
        if(n->data[idx + 1] > 68547758)
            return 0;
        if(shift_right(n->data[idx], 6) <= 07)
            return 1;
        return 0;
    }

    case 7:
    {
        if(n->data[idx + 3] < 92)
            return 1;
        if(n->data[idx + 3] > 92)
            return 0;
        if(n->data[idx + 2] < 23372036)
            return 1;
        if(n->data[idx + 2] > 23372036)
            return 0;
        if(n->data[idx + 1] < 85477580)
            return 1;
        if(n->data[idx + 1] > 85477580)
            return 0;
        if(shift_right(n->data[idx], 7) <= 7)
            return 1;
        return 0;
    }
    }
    return 0;

}

// TRUE IF A REAL IS INTEGER
// DOES NOT NEED TO BE ALIGNED

BINT isintegerReal(REAL * n)
{
    if(n->flags & (F_NOTANUMBER | F_INFINITY))
        return 0;

    if(n->exp >= 0)
        return 1;

    int k;
    for(k = 0; k < ((-n->exp) >> 3); ++k)
        if(n->data[k] != 0)
            return 0;

    if(lo_digits(n->data[(-n->exp) >> 3], (-n->exp) & 7) != 0)
        return 0;

    return 1;
}

// EXTRACT A 32-BIT INTEGER FROM A REAL
BINT getBINTReal(REAL * n)
{
    BINT result;

    int digits = ((n->len - 1) << 3) + sig_digits(n->data[n->len - 1]) + n->exp;

    // THIS SHOULDN'T HAPPEN, USER SHOULD CHECK IF WITHIN RANGE OF A BINT BEFORE CALLING
    if(digits > 10)
        return 0;
    if(digits <= 0)
        return 0;

    // THE NUMBER HAS EXACTLY 19 DIGITS
    if(n->exp >= 0) {
        if(n->len > 1)
            result = n->data[1] * 100000000;
        else
            result = 0;
        result += n->data[0];

        // THE NUMBER IS MISSING SOME DIGITS, RESTORE THEM
        if(n->exp > 7)
            result *= 100000000;
        if(n->flags & F_NEGATIVE)
            result = -result;
        return result * shiftmul_K2[n->exp & 7];
    }

    // NEGATIVE EXPONENT
    int idx = (-n->exp) >> 3;
    int nwords = n->len - idx;
    int rshift = ((-n->exp) & 7);
    int lshift = (8 - rshift) & 7;
    BINT carry = 0;
    int64_t tmp;
    result = 0;
    while(nwords--) {
        result *= 100000000LL;
        result += carry * shiftmul_K2[lshift];
        tmp = shift_split(n->data[idx + nwords], rshift);
        result += tmp >> 32;
        carry = tmp & 0xffffffff;
    }

    if(n->flags & F_NEGATIVE)
        return -result;
    return result;

}

// EXTRACT A 64-BIT INTEGER FROM A REAL
int64_t getint64_tReal(REAL * n)
{
    int64_t result;

    int digits = ((n->len - 1) << 3) + sig_digits(n->data[n->len - 1]) + n->exp;

    // THIS SHOULDN'T HAPPEN, USER SHOULD CHECK IF WITHIN RANGE OF A BINT BEFORE CALLING
    if(digits > 19)
        return 0;
    if(digits <= 0)
        return 0;

    // THE NUMBER HAS EXACTLY 10 DIGITS
    if(n->exp >= 0) {
        if(n->len > 2)
            result = n->data[2] * 10000000000000000LL;
        else
            result = 0;
        if(n->len > 1)
            result += n->data[1] * 100000000LL;
        result += n->data[0];

        // THE NUMBER IS MISSING SOME DIGITS, RESTORE THEM
        if(n->exp > 15)
            result *= 10000000000000000LL;
        else if(n->exp > 7)
            result *= 100000000LL;
        if(n->flags & F_NEGATIVE)
            result = -result;
        return result * shiftmul_K2[n->exp & 7];
    }

    // NEGATIVE EXPONENT
    int idx = (-n->exp) >> 3;
    int nwords = n->len - idx;
    int rshift = ((-n->exp) & 7);
    int lshift = (8 - rshift) & 7;
    BINT carry = 0;
    int64_t tmp;
    result = 0;
    while(nwords--) {
        result *= 100000000;
        result += carry * shiftmul_K2[lshift];
        tmp = shift_split(n->data[idx + nwords], rshift);
        result += tmp >> 32;
        carry = tmp & 0xffffffff;
    }

    if(n->flags & F_NEGATIVE)
        return -result;
    return result;
}

BINT isoddReal(REAL * r)
{
    BINT position = 0;
    BINT digits;

    int dig = sig_digits(r->data[r->len - 1]);

    BINT max_pos = ((r->len - 1) << 3) + dig + r->exp;
    BINT min_pos = r->exp;

    if(position < min_pos)
        return 0;
    if(position > max_pos)
        return 0;

    int trim = position - min_pos;

    int smallshift = trim & 7;

    trim >>= 3;

    if(trim >= r->len)
        digits = 0;
    else
        digits = shift_split(r->data[trim], smallshift) >> 32;

    if(digits & 1)
        return 1;

    return 0;

}

// RETURN THE NUMBER OF DIGITS OF THE INTEGER PART OF THE REAL
BINT intdigitsReal(REAL * r)
{
    return ((r->len - 1) << 3) + sig_digits(r->data[r->len - 1]) + r->exp;
}

// RETURN THE NUMBER OF STORED TRAILING ZEROS
BINT trailzerosReal(REAL * r)
{
    BINT count = 0;
    BINT off = 0;
    while((r->data[off] == 0) && (off < r->len)) {
        ++off;
        count += 8;
    }

    if(off == r->len)
        return count;
    if(r->data[off] == 0)
        return count + 8;

    if(lo_digits(r->data[off], 4)) {
        if(lo_digits(r->data[off], 2)) {
            // COULD BE 0 OR 1 ZEROS
            if(lo_digits(r->data[off], 1))
                return count;
            else
                return count + 1;
        }
        else {
            // COULD BE 2 OR 3 ZEROS
            if(lo_digits(r->data[off], 3))
                return count + 2;
            else
                return count + 3;
        }
    }
    else {
        if(lo_digits(r->data[off], 6)) {
            // COULD BE 4 OR 5 ZEROS
            if(lo_digits(r->data[off], 5))
                return count + 4;
            else
                return count + 5;
        }
        else {
            // COULD BE 6 OR 7 ZEROS
            if(lo_digits(r->data[off], 7))
                return count + 6;
            else
                return count + 7;
        }

    }

}

// *************************************************************************
// **************************** END DECIMAL LIBRARY ************************
// *************************************************************************
