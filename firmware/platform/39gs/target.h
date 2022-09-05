/*
 * Copyright (c) 2014-2017 Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef TARGET_39GS_H
#define TARGET_39GS_H

#include <platform/50g/target.h>

#undef PREAMBLE_STRING
#define PREAMBLE_STRING "KINPOHP39G+IMAGE"

// Override the device string on the USB bus
#undef  STR_PRODUCT
#define STR_PRODUCT		{'n',0,'e',0,'w',0,'R',0,'P',0,'L',0,' ',0,'3',0,'9',0,'g',0,'s',0}
#undef  STR_PRODLENGTH
#define STR_PRODLENGTH   22+2


#undef RAM_END_PHYSICAL
#define RAM_END_PHYSICAL  0x08040000

#undef SCREEN_HEIGHT
#define SCREEN_HEIGHT 64

// CONFIGURATION OPTIONS

#define CONFIG_USE_LCD_ALTERNATIVE_SETTINGS {0x46f1,0x46f3,0x46f5,0x46f7,0x46f9,0x46fb,0x46fd,0x47f0,0x47f1,0x47f3,0x47f5,0x47f7,0x47f9,0x47fb,0x47fd,0x47ff}


//#define CONFIG_LIGHT_MATH_TABLES  1

// THIS WILL REMOVE THE SD CARD SUPPORT IN ROM TO SAVE SOME SPACE
// FOR NOW IT'S NOT NEEDED SO SD CARD WILL REMAIN SUPPORTED ON THIS PLATFORM
//#define CONFIG_NO_FSYSTEM         1

#endif // TARGET_39GS_H
