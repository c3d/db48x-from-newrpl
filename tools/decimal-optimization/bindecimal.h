#ifndef BINDECIMAL_H
#define BINDECIMAL_H

void bIntegerAdd(REAL * res, REAL * a, REAL * b);
void bIntegerAddShift(REAL * res, REAL * a, REAL * b, int bshift);
void bIntegerMul(REAL * res, REAL * a, REAL * b, int FPShift);
void bIntegerfromReal(REAL * res, REAL * number);
void RealfrombInteger(REAL * res, REAL * integer);

WORD bbintadd(WORD * res, WORD * a, WORD * b, int nwords);
WORD bbintacc(WORD * res, WORD * a, int nwords);
WORD bbintmulshortacc(WORD * res, WORD * a, WORD b, int nwords);
WORD bbintmullong(WORD * res, WORD * a, WORD * b, int nwords);
WORD bbintaccshiftright(WORD * res, WORD * a, int shift, int nwords);
void bbintneg(WORD * res, WORD * a, int nwords);
WORD bbintacc2n(WORD * res, int shift, int nwords);

#define CORDIC_MAXSYSEXP    6720
#define CORDIC_TABLESIZE    CORDIC_MAXSYSEXP/2
#define CORDIC_TABLEWORDS   CORDIC_MAXSYSEXP/32

extern const uint32_t const atan_binary[CORDIC_TABLESIZE * CORDIC_TABLEWORDS];
extern const uint32_t const K_binary[CORDIC_TABLESIZE * CORDIC_TABLEWORDS];
extern const uint32_t const two_exp_binary[];
extern const uint32_t const two_exp_offset[];

#endif // BINDECIMAL_H
