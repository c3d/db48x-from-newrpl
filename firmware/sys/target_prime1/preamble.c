/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "target_prime1.h"

unsigned char preamble[0x20] __attribute__((section(".preamble"))) = {
    0x20, 0x00, 0x00, 0x30,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x00,
    0x00, 0x00, 0x00, 0x30,
    0x00, 0x00, 0x10, 0x00,
    0x56, 0x35, 0x4A, 0x00, 0x32, 0x34, 0x31, 0x36,
    0x00, 0x00, 0x00, 0x00 
};

extern void startup(int);

void _boot(int prevstate) __attribute__((naked))
        __attribute__((section(".codepreamble")));
__ARM_MODE__ void _boot(int prevstate)
{
    asm volatile ("b startup");
}
