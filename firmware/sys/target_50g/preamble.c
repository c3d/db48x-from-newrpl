/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <ui.h>

#define __ARM_MODE__ __attribute__((target("arm"))) __attribute__((noinline))

char kinpo_preamble[16] __attribute__( (section (".preamble")))
= PREAMBLE_STRING;

unsigned int filler[2] __attribute__( (section (".preamble"))) = { 0, 0 };

extern void startup(int);

void _boot(int prevstate) __attribute__ ((naked)) __attribute__ ((section (".codepreamble")));
__ARM_MODE__ void _boot(int prevstate)
{
    asm volatile ("b startup");
}

