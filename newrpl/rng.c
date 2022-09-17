
/*  Written in 2016 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

/* This is the successor to xorshift128+. It is the fastest full-period
   generator passing BigCrush without systematic failures, but due to the
   relatively short period it is acceptable only for applications with a
   mild amount of parallelism; otherwise, use a xorshift1024* generator.

   Beside passing BigCrush, this generator passes the PractRand test suite
   up to (and included) 16TB, with the exception of binary rank tests, as
   the lowest bit of this generator is an LSFR. The next bit is not an
   LFSR, but in the long run it will fail binary rank tests, too. The
   other bits have no LFSR artifacts.

   We suggest to use a sign test to extract a random Boolean value, and
   right shifts to extract subsets of bits.

   Note that the generator uses a simulated rotate operation, which most C
   compilers will turn into a single instruction. In Java, you can use
   Long.rotateLeft(). In languages that do not make low-level rotation
   instructions accessible xorshift128+ could be faster.

   The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to seed a splitmix64 generator and use its
   output to fill s. */

/* Modified and adapted for newRPL */

#include "newrpl.h"

uint64_t newRPL_rng[2];

static inline uint64_t rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

uint64_t rplRandomNext(void)
{
    const uint64_t s0 = newRPL_rng[0];
    uint64_t s1 = newRPL_rng[1];
    const uint64_t result = s0 + s1;

    s1 ^= s0;
    newRPL_rng[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14);     // a, b
    newRPL_rng[1] = rotl(s1, 36);       // c

    return result;
}

/* This is the jump function for the generator. It is equivalent
   to 2^64 calls to next(); it can be used to generate 2^64
   non-overlapping subsequences for parallel computations. */

static const uint64_t const JUMP[] =
        { 0xbeac0467eba5facbULL, 0xd86b048b86aa9922ULL };

void rplRandomJump(void)
{

    uint64_t s0 = 0;
    uint64_t s1 = 0;
    int i, b;

    for(i = 0; i < 2; i++)
        for(b = 0; b < 64; b++) {
            if(JUMP[i] & 1ULL << b) {
                s0 ^= newRPL_rng[0];
                s1 ^= newRPL_rng[1];
            }
            rplRandomNext();
        }

    newRPL_rng[0] = s0;
    newRPL_rng[1] = s1;
}

// RANDOM SEQUENCE STARTING FROM A SEED
// JUST SPLIT THE SEED AND RUN THE RNG 5 TIMES

void rplRandomSeed(uint64_t seed)
{
    /*  Based on SplitMix64 by Sebastiano Vigna (vigna@acm.org) */

    seed += 0x9e3779b97f4a7c15;
    seed = (seed ^ (seed >> 30)) * 0xbf58476d1ce4e5b9;
    seed = (seed ^ (seed >> 27)) * 0x94d049bb133111eb;
    seed = seed ^ (seed >> 31);

    newRPL_rng[0] = seed;

    seed += 0x9e3779b97f4a7c15;
    seed = (seed ^ (seed >> 30)) * 0xbf58476d1ce4e5b9;
    seed = (seed ^ (seed >> 27)) * 0x94d049bb133111eb;
    seed = seed ^ (seed >> 31);

    newRPL_rng[1] = seed;

}

// GET 8 RANDOM DECIMAL DIGITS
// USES ONLY 27 BITS TAKEN FROM THE RNG, BUT NOT THE LAST 27

int32_t rplRandom8Digits()
{
    uint64_t dig = rplRandomNext() >> 2;
    dig %= 100000000;
    return (int32_t) dig;
}
