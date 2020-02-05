/*
 * Copyright (c) 2014-2017 Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef TARGET_40GS_H
#define TARGET_40GS_H

#include <target_50g.h>

#undef PREAMBLE_STRING
#define PREAMBLE_STRING "KINPOHP40G+IMAGE"

#undef RAM_END_PHYSICAL
#define RAM_END_PHYSICAL  0x08040000

#undef SCREEN_HEIGHT
#define SCREEN_HEIGHT 64

// CONFIGURATION OPTIONS

//#define CONFIG_LIGHT_MATH_TABLES  1

// THIS WILL REMOVE THE SD CARD SUPPORT IN ROM TO SAVE SOME SPACE
// FOR NOW IT'S NOT NEEDED SO SD CARD WILL REMAIN SUPPORTED ON THIS PLATFORM
//#define CONFIG_NO_FSYSTEM         1

#endif // TARGET_40GS_H
