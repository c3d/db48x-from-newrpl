/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#ifndef ARITHMETIC_H
#define ARITHMETIC_H

// INTERNAL TRANSCENDENTAL FUNCTIONS

void decconst_PI(REAL *real);
void decconst_2PI(REAL *real);
void decconst_PI_2(REAL *real);
void decconst_PI_4(REAL *real);
void decconst_PI_200(REAL *real);
void decconst_PI_180(REAL *real);
void decconst_180_PI(REAL *real);
void decconst_200_PI(REAL *real);
void decconst_ln10(REAL *real);
void decconst_ln10_2(REAL *real);
void decconst_One(REAL *real);
void decconst_90(REAL *real);
void decconst_100(REAL *real);
void decconst_45(REAL *real);
void decconst_50(REAL *real);
void decconst_180(REAL *real);
void decconst_200(REAL *real);


void hyp_exp(REAL *);
void hyp_expm(REAL *);
void hyp_ln(REAL *);
void hyp_lnp1(REAL *);
void hyp_log(REAL *);
void hyp_alog(REAL *);
void hyp_pow(REAL *x,REAL *a);


void powReal(REAL *result,REAL *x,REAL *a);
void xrootReal(REAL *result,REAL *x,REAL *a);

void hyp_sqrt(REAL *);
void hyp_sinhcosh(REAL *);
void hyp_tanh(REAL *);
void hyp_sinh(REAL *);
void hyp_cosh(REAL *);
void hyp_atanh(REAL *);
void hyp_asinh(REAL *);
void hyp_acosh(REAL *);

void trig_convertangle(REAL *,BINT oldangmode,BINT newangmode);
void trig_reduceangle(REAL *angle,BINT angmode);
void trig_sincos(REAL *,BINT angmode);
void trig_sin(REAL *,BINT angmode);
void trig_cos(REAL *,BINT angmode);
void trig_tan(REAL *,BINT angmode);
void trig_atan2(REAL *, REAL *, BINT angmode);
void trig_asin(REAL *,BINT angmode);
void trig_acos(REAL *, BINT angmode);


BINT64 factorialBINT(BINT n);
BINT64 nextcbprimeBINT(BINT64 n);
BINT64 nextprimeBINT(BINT64 n);
BINT isprimeBINT(BINT64 n);
void nextprimeReal(BINT regnum,REAL *n);
BINT isprimeReal(REAL *n);

BINT64 powmodBINT(BINT64 a, BINT64 b, BINT64 mod);
void powmodReal(REAL *result,REAL *a,REAL *b,REAL *mod);

void gcdReal(REAL *result,REAL *a,REAL *b);
BINT64 gcdBINT64(BINT64 a,BINT64 b);
BINT64 factorReal(REAL *result,REAL *n);






#endif // ARITHMETIC_H

