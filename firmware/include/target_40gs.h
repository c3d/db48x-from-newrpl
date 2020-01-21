/*
 * Copyright (c) 2014-2017 Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef TARGET_40GS_H
#define TARGET_40GS_H

#undef PREAMBLE_STRING
#define PREAMBLE_STRING "KINPOHP40G+IMAGE"

#undef RAM_END_PHYSICAL
#define RAM_END_PHYSICAL  0x08040000

#undef SCREEN_HEIGHT
#define SCREEN_HEIGHT 64

// CONFIGURATION OPTIONS

//#define CONFIG_LIGHT_MATH_TABLES  1
#define CONFIG_NO_SDCARD          1

#endif // TARGET_40GS_H
