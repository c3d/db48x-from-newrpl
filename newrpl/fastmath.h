/*
 * Copyright (c) 2017, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#ifndef FASTMATH_H
#define FASTMATH_H


// THIS IS A BASIC FIXED-POINT MATH LIBRARY TO BE USED WITH PLOT OBJECTS AND GRAPHICS IN GENERAL
// IT'S LOW-PRECISION 40.24 ARITHMETIC
// FAST, NO ERRORS OR EXCEPTIONS, OVERFLOW CHECK ONLY DURING MULTIPLICATION


typedef BINT64 FPINT;

union FPUNION {
    FPINT fp;
    WORD lo,hi;
};


extern FPINT mulFPINT(FPINT a,FPINT b);

extern FPINT divFPINT(FPINT a,FPINT b);






#endif // FASTMATH_H

