/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "target_prime1.h"

extern void startup(int);

#define size_in_kb 256

struct Preamble preamble __attribute__((section(".preamble"))) = {
	.entrypoint = (uint32_t)&startup,
	.unused1 = 0,
    .copy_size = size_in_kb * 1024,
	.load_addr = 0x30000000,
    .load_size = size_in_kb * 1024,
	.cpu_arch = 0x4a3556, // "V5J"
	.cpuid = 0x36313432, // "2416"
	.unused2 = 0,
};
