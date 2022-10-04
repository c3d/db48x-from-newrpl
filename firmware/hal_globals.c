/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>
// SYSTEM GLOBAL VARIABLES, MAPPED TO RAM 0X08004000 (8 KBYTES MAXIMUM!)
// THESE VARIABLES ARE NOT PERSISTENT ACROSS ON/OFF, REBOOT, ETC.
// VARIABLES CANNOT HAVE STATIC INITIAL VALUES, MUST BE INITIALIZED BY SOFTWARE.

// BASIC HAL VARIABLES
HALSCREEN halScreen SYSTEM_GLOBAL;

// CPU SPEED HIGH LEVEL
int32_t halFlags SYSTEM_GLOBAL;
void (*halProcesses[3])(void)SYSTEM_GLOBAL;

HEVENT halBusyEvent SYSTEM_GLOBAL;
HEVENT halTimeoutEvent SYSTEM_GLOBAL;

// High level keyboard handler
int32_t halKeyMenuSwitch  SYSTEM_GLOBAL;

// RENDER CACHE
word_p halCacheContents[3 * MAX_RENDERCACHE_ENTRIES] SYSTEM_GLOBAL;
WORD halCacheEntry SYSTEM_GLOBAL;
