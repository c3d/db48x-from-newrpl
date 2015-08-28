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
HALSCREEN halScreen __SYSTEM_GLOBAL__;


// CPU SPEED HIGH LEVEL
BINT halFlags __SYSTEM_GLOBAL__;
HEVENT halBusyEvent __SYSTEM_GLOBAL__;


// HIGH LEVEL KEYBOARD HANDLER
BINT halLongKeyPending __SYSTEM_GLOBAL__;

// COMMAND LINE EDITOR

