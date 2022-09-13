/*
 * Copyright (c) 2014-2017 Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef TARGET_48GII_H
#define TARGET_48GII_H

#include <platform/50g/target.h>

#undef PREAMBLE_STRING
#define PREAMBLE_STRING "KINPOHP48GIIMAGE"

// Override the device string on the USB bus
#undef  STR_PRODUCT
#define STR_PRODUCT		{'n',0,'e',0,'w',0,'R',0,'P',0,'L',0,' ',0,'4',0,'8',0,'g',0,'2',0}
#undef  STR_PRODLENGTH
#define STR_PRODLENGTH   22+2


#undef RAM_END_PHYSICAL
#define RAM_END_PHYSICAL  0x08040000

#undef LCD_H
#define LCD_H 64

// CONFIGURATION OPTIONS

#define CONFIG_USE_LCD_ALTERNATIVE_SETTINGS {0x45F2, 0x45F4, 0x45F6, 0x45F8, 0x46ED, 0x45FE, 0x46F0, 0x46F2, 0x46F7, 0x46FE, 0x47EF, 0x47F2, 0x47F4, 0x47F7, 0x47FA, 0x47FF }



//#define CONFIG_LIGHT_MATH_TABLES  1

// THIS WILL REMOVE THE SD CARD SUPPORT IN ROM TO SAVE SOME SPACE
// FOR NOW IT'S NOT NEEDED SO SD CARD WILL REMAIN SUPPORTED ON THIS PLATFORM
//#define CONFIG_NO_FSYSTEM         1

#endif // TARGET_48GII_H
