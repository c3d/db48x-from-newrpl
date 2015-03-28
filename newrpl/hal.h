/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef HAL_H
#define HAL_H

#define UNUSED_ARGUMENT(a) (void)(a)


#ifdef TARGET_PC
#include <stdlib.h>
#include <string.h>

#define MAX_RAM 16*1024    // 16KBYTES PER AREA = 6*16 TOTAL, ON 64-BIT SYSTEMS IT'S 32KBYTES

#define SCRATCH_MEMORY
#define PERSISTENT_MEMORY

#define throw_exception(a,b) { printf(a); exit(b); }
#define throw_dbgexception(a,b) { printf(a); exit(b); }



#else

#define SCRATCH_MEMORY __attribute__((section (".scratch_memory")))
#define PERSISTENT_MEMORY __attribute__((section (".persistent_memory")))

#endif

// PULL IN ALL TYPE DEFINITIONS
#include "newrpl.h"

enum {
    MEM_AREA_RSTK=0,
    MEM_AREA_DSTK,
    MEM_AREA_DIR,
    MEM_AREA_LAM,
    MEM_AREA_TEMPOB,
    MEM_AREA_TEMPBLOCKS
};


// COMMON DEFINITIONS FOR ALL HARDWARE PLATFORMS
WORDPTR *halGrowMemory(BINT zone,WORDPTR *base,BINT newsize);

// BASIC MEMORY MOVEMENT ROUTINES (FOR WORD ALIGNED DATA)
void memmovew(void *dest,const void *source,int nwords);
void memcpyw(void *dest,const void *source,int nwords);



#endif // HAL_H
