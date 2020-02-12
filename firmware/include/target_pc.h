/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef TARGET_PC_H
#define TARGET_PC_H


#undef MEM_PHYS_SCREEN
#define MEM_PHYS_SCREEN PhysicalScreen

#undef MEM_PHYS_EXSCREEN
#define MEM_PHYS_EXSCREEN ExceptionScreen

#undef DEFAULT_AUTOOFFTIME
#define DEFAULT_AUTOOFFTIME 0   // NO AUTO OFF ON A PC!

#undef __ENABLE_ARM_ASSEMBLY__  // THIS TARGET IS NOT ARM

#undef __SYSTEM_GLOBAL__
#define __SYSTEM_GLOBAL__

#undef __DATAORDER1__
#define __DATAORDER1__

#undef __DATAORDER2__
#define __DATAORDER2__

#undef __DATAORDER3__
#define __DATAORDER3__

#undef __DATAORDERLAST__
#define __DATAORDERLAST__

#undef __SCRATCH_MEMORY__
#define __SCRATCH_MEMORY__

#undef __ROMOBJECT__
#define __ROMOBJECT__

#undef __ROMLINK__
#define __ROMLINK__

// SIGNALS FOR OS-DRIVEN EVENTS
extern void halScreenUpdated();

// MAKE TIMEOUT VARIABLE SO WE CAN HAVE SHORT TIMEOUT FOR DEVICE DETECTION
#undef USB_TIMEOUT_MS
#define USB_TIMEOUT_MS __usb_timeout
extern int __usb_timeout;


// Target PC uses 50g screen and other capabilities for now

// USABLE SCREEN SIZE
#define SCREEN_WIDTH 131
#define SCREEN_HEIGHT 80
#define PIXELS_PER_WORD 8

// PHYSICAL SCREEN SIZE
#define SCREEN_W 160
#define SCREEN_H 80

#define ANN_X_COORD (SCREEN_WIDTH)

// DEFAULT COLOR MODE OF THE SYSTEM
#define DEFAULTBITSPERPIXEL 4
#define DEFAULTBITMAPMODE   1   // SAME AS BITMAP_RAW16G

// DEFAULT CLOCK SPEEDS
#define HAL_SLOWCLOCK     6000000
#define HAL_USBCLOCK     48000000
#define HAL_FASTCLOCK   192000000

extern char PhysicalScreen[(SCREEN_W*SCREEN_H)/(PIXELS_PER_WORD/4)];
extern char ExceptionScreen[(SCREEN_W*SCREEN_H)/(PIXELS_PER_WORD/4)];



#endif // TARGET_PC_H
