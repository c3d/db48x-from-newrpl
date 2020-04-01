/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "target_prime1.h"

extern void startup(int);

void _boot(int prevstate) __attribute__((naked))
        __attribute__((section(".codepreamble")));
__ARM_MODE__ void _boot(int prevstate)
{
    asm volatile ("b startup");
}
