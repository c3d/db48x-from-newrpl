/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef ARITHMETIC_H
#define ARITHMETIC_H

// INTERNAL TRANSCENDENTAL FUNCTIONS

void decconst_PI(REAL * real);
void decconst_2PI(REAL * real);
void decconst_PI_2(REAL * real);
void decconst_PI_4(REAL * real);
void decconst_PI_200(REAL * real);
void decconst_PI_180(REAL * real);
void decconst_180_PI(REAL * real);
void decconst_200_PI(REAL * real);
void decconst_ln10(REAL * real);
void decconst_ln10_2(REAL * real);
void decconst_One(REAL * real);
void decconst_90(REAL * real);
void decconst_100(REAL * real);
void decconst_45(REAL * real);
void decconst_50(REAL * real);
void decconst_180(REAL * real);
void decconst_200(REAL * real);

void hyp_exp(REAL *);
void hyp_ln(REAL *);
void hyp_log(REAL *);
void hyp_alog(REAL *);
void hyp_pow(REAL * x, REAL * a);

void powReal(REAL * result, REAL * x, REAL * a);
void xrootReal(REAL * result, REAL * x, REAL * a);

void hyp_sqrt(REAL *);
void hyp_sinhcosh(REAL *);
void hyp_atanh(REAL *);
void hyp_asinh(REAL *);
void hyp_acosh(REAL *);

void trig_convertangle(REAL *, int32_t oldangmode, int32_t newangmode);
void trig_reduceangle(REAL * angle, int32_t angmode);
void trig_sincos(REAL *, int32_t angmode);
void trig_atan2(REAL *, REAL *, int32_t angmode);
void trig_asin(REAL *, int32_t angmode);
void trig_acos(REAL *, int32_t angmode);

int64_t factorialint32_t(int32_t n);
int64_t nextcbprimeint32_t(int64_t n);
int64_t nextprimeint32_t(int64_t n);
int32_t isprimeint32_t(int64_t n);
void nextprimeReal(int32_t regnum, REAL * n);
int32_t isprimeReal(REAL * n);

int64_t powmodint32_t(int64_t a, int64_t b, int64_t mod);
void powmodReal(REAL * result, REAL * a, REAL * b, REAL * mod);

#endif // ARITHMETIC_H
