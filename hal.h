/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef HAL_H
#define HAL_H

#ifdef TARGET_PC_SIMULATOR
#include <stdlib.h>
#include <string.h>

#define MAX_RAM 16*1024    // 64KBYTES PER AREA = 6*64 TOTAL

#endif

// PULL IN ALL TYPE DEFINITIONS
#include "newrpl.h"

// COMMON DEFINITIONS FOR ALL HARDWARE PLATFORMS
WORDPTR *halGrowMemory(WORDPTR *base,BINT newsize);


#endif // HAL_H
