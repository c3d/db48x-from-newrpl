/*
 * Copyright (c) 2014-2017 Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef TARGET_48GII_H
#define TARGET_48GII_H

#include <target_50g.h>

#undef PREAMBLE_STRING
#define PREAMBLE_STRING "KINPOHP48GIIMAGE"

#undef RAM_END_PHYSICAL
#define RAM_END_PHYSICAL  0x08040000

#undef SCREEN_HEIGHT
#define SCREEN_HEIGHT 64

// CONFIGURATION OPTIONS

#define CONFIG_USE_LCD_ALTERNATIVE_SETTINGS {0x45F2, 0x45F4, 0x45F6, 0x45F8, 0x46ED, 0x45FE, 0x46F0, 0x46F2, 0x46F7, 0x46FE, 0x47EF, 0x47F2, 0x47F4, 0x47F7, 0x47FA, 0x47FF }



//#define CONFIG_LIGHT_MATH_TABLES  1

// THIS WILL REMOVE THE SD CARD SUPPORT IN ROM TO SAVE SOME SPACE
// FOR NOW IT'S NOT NEEDED SO SD CARD WILL REMAIN SUPPORTED ON THIS PLATFORM
//#define CONFIG_NO_FSYSTEM         1

#endif // TARGET_48GII_H
