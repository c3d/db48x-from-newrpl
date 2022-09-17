/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// HARDWARE DEPENDENT LAYER
#include "ui.h"

#ifdef TARGET_PC

// NEWSIZE IS IN 32-BIT WORDS!
word_p *halGrowMemory(int32_t zone, word_p * base, int32_t newsize)
{
    UNUSED_ARGUMENT(zone);

    if(!base)
        return malloc(MAX_RAM * (sizeof(void *) >> 2));
    else {
        if((newsize << 2) > MAX_RAM)
            return 0;
        return base;
    }
}

// COPY ASCENDING BY WORDS
void memcpyw(void *dest, const void *source, int nwords)
{
    pixword *destint = (int *)dest;
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
    pixword *destint = ((int *)dest) + nwords;
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

#endif // TARGET_PC

// FOR OTHER TARGETS, THIS FUNCTION WILL BE SUPPLIED BY THE FIRMWARE
