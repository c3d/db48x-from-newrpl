/*
 * Copyright (c) 2014-2017 Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef TARGET_39GS_H
#define TARGET_39GS_H

#undef PREAMBLE_STRING
#define PREAMBLE_STRING "KINPOHP39G+IMAGE"

#undef RAM_END_PHYSICAL
#define RAM_END_PHYSICAL  0x08040000

#undef SCREEN_HEIGHT
#define SCREEN_HEIGHT 64

// CONFIGURATION OPTIONS

// THIS WILL REMOVE THE SD CARD SUPPORT IN ROM TO SAVE SOME SPACE
// FOR NOW IT'S NOT NEEDED SO SD CARD WILL REMAIN SUPPORTED ON THIS PLATFORM
//#define CONFIG_NO_FSYSTEM         1

#endif // TARGET_39GS_H
