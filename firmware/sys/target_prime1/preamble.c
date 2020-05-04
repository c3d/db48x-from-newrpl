/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "target_prime1.h"

extern void startup(int);

struct Preamble {
	uint32_t entrypoint;
	uint32_t unused1;
	uint32_t copy_size;
	uint32_t load_addr;
	uint32_t load_size;
	uint32_t cpu_arch;
	uint32_t cpuid;
	uint32_t unused2;
} __attribute__ ((packed));

struct Preamble preamble __attribute__((section(".preamble"))) = {
	.entrypoint = (uint32_t)&startup,
	.unused1 = 0,
    .copy_size = 128*1024,
	.load_addr = 0x30000000,
    .load_size = 128*1024,
	.cpu_arch = 0x4a3556, // "V5J"
	.cpuid = 0x36313432, // "2416"
	.unused2 = 0,
};
