/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <stdint.h>

#if (defined(__LP64__) || defined(_WIN64)) && !( defined(TARGET_50G) || defined(TARGET_39GS) || defined(TARGET_40GS) || defined(TARGET_48GII) || defined(TARGET_PRIME1))
typedef uint64_t PTR2NUMBER;
#define NUMBER2PTR(a) ((WORDPTR)((UBINT64)(a)))
#else
typedef uint32_t PTR2NUMBER;
#define NUMBER2PTR(a) ((WORDPTR)((WORD)(a)))
#endif


void *memcpyb(void *trg, const void *src, int n)
{
    void *r = trg;
    char *t8 = (char *)trg;
    char *s8 = (char *)src;

#define ESIZE (int)sizeof(int)
#define CSIZE (int)(4*ESIZE)

    if(n >= CSIZE && !(((PTR2NUMBER) t8 & (ESIZE - 1))
                || ((PTR2NUMBER) s8 & (ESIZE - 1)))) {

        // source & target properly aligned
        // use word copy...

        unsigned long *T, *S;

        T = (unsigned long *)t8;
        S = (unsigned long *)s8;

        for(; n >= CSIZE;) {
            *T++ = *S++;
            *T++ = *S++;
            *T++ = *S++;
            *T++ = *S++;
            n -= CSIZE;
        }

        for(; n >= ESIZE;) {
            *T++ = *S++;
            n -= ESIZE;
        }

        if(n > 0) {

            t8 = (char *)T;
            s8 = (char *)S;

            for(; n--;)
                *t8++ = *s8++;
        }
    }
    else if(n > 0)
        for(; n--;)
            *t8++ = *s8++;

    return r;
}

// COPY ASCENDING BY WORDS
void memcpyw(void *dest, const void *source, int nwords)
{
    int *destint = (int *)dest;
    int *sourceint = (int *)source;
    while(nwords) {
        *destint = *sourceint;
        ++destint;
        ++sourceint;
        --nwords;
    }

}

// COPY DESCENDING BY WORDS
void memcpywd(void *dest, const void *source, int nwords)
{
    int *destint = ((int *)dest) + nwords;
    int *sourceint = ((int *)source) + nwords;
    while(nwords) {
        *--destint = *--sourceint;
        --nwords;
    }

}

// MOVE MEMORY, HANDLE OVERLAPPING BLOCKS PROPERLY
void memmovew(void *dest, const void *source, int nwords)
{
    int offset = ((int *)dest) - ((int *)source);
    int *ptr = (int *)source;
    if(offset > 0) {
        ptr += nwords - 1;
        while(nwords > 0) {
            ptr[offset] = *ptr;
            --ptr;
            --nwords;
        }
    }
    else {
        while(nwords > 0) {
            ptr[offset] = *ptr;
            ++ptr;
            --nwords;
        }
    }
}

void *memmoveb(void *_dest, const void *_source, int nbytes)
{
    register char *dest = (char *)_dest;
    register char *source = (char *)_source;
    register int going = (_dest > _source) ? (-1) : (1);
    if(going == -1) {
        dest += nbytes - 1;
        source += nbytes - 1;
    }

    while(nbytes--) {
        *dest = *source;
        dest += going;
        source += going;
    }
    return _dest;
}

void memsetw(void *dest, int value, int nwords)
{
    int *_dest = (int *)dest;
    while(nwords) {
        *_dest++ = value;
        --nwords;
    }
}

void *memsetb(void *_dest, int value, int nbytes)
{
    char *destc = (char *)_dest;

    int val2 = value & 0xff;
    val2 |= (val2 << 8) | (val2 << 16) | (val2 << 24);
    while(nbytes > 0 && (((PTR2NUMBER) destc) & 3)) {
        *destc = value;
        ++destc;
        nbytes--;
    }

    int *desti = (int *)destc;

    while(nbytes >= 4) {
        *desti = val2;
        ++desti;
        nbytes -= 4;
    }

    destc = (char *)desti;

    while(nbytes) {
        *destc = value;
        ++destc;
        nbytes--;
    }
    return (void *)destc;
}

int strncmp(const char *s1, const char *s2, unsigned long num)
{
    if(num > 0) {
        while(num > 0) {
            if(*s1 != *s2)
                break;
            if(*s1 == '\0')
                return 0;

            s1++;
            s2++;
            num--;
        }

        if(num > 0) {
            if(*s1 == '\0')
                return -1;
            if(*s2 == '\0')
                return 1;

            return (((unsigned char)*s1) - ((unsigned char)*s2));
        }
    }

    return 0;
}

int stringlen(const char *s)
{
    int c = 0;
    while(*s++)
        c++;
    return c;
}

char *stringcpy(char *t, const char *s)
{
    char *r = t;
    while(*s)
        *t++ = *s++;
    *t = '\0';
    return r;
}

int safe_stringcpy(char *t, int bsize, const char *s)
{
    int n = stringlen(s);
    if(n + 1 > bsize)
        return 1;
    memmoveb(t, s, n + 1);
    return 0;
}

