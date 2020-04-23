/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>

static void setup_hardware()
{
	// Set 3 LED pins as outputs
    uint32_t config = *GPCCON;
    config &= 0xFFFF03FF;
    config |= 0x00005400;
    *GPCCON = config;
}

__ARM_MODE__ void startup(int) __attribute__((naked, noreturn));
void startup(int prevstate)
{
	setup_hardware();

    uint32_t pins = *GPCDAT;
    // Pin GPC7 is red LED
    pins |= 0x000000E0;
    *GPCDAT = pins;

    while(1);
}
