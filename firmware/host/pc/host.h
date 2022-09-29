/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef HOST_PC_H
#define HOST_PC_H

#undef MEM_PHYS_SCREEN
#define MEM_PHYS_SCREEN PhysicalScreen

#undef MEM_PHYS_EXSCREEN
#define MEM_PHYS_EXSCREEN ExceptionScreen

#undef DEFAULT_AUTOOFFTIME
#define DEFAULT_AUTOOFFTIME 0   // NO AUTO OFF ON A PC!

#undef ENABLE_ARM_ASSEMBLY  // THIS TARGET IS NOT ARM

#undef SYSTEM_GLOBAL
#define SYSTEM_GLOBAL

#undef DATAORDER1
#define DATAORDER1

#undef DATAORDER2
#define DATAORDER2

#undef DATAORDER3
#define DATAORDER3

#undef DATAORDERLAST
#define DATAORDERLAST

#undef SCRATCH_MEMORY
#define SCRATCH_MEMORY

#undef ROMOBJECTS
#define ROMOBJECTS

#undef ROMLINK
#define ROMLINK

// SIGNALS FOR OS-DRIVEN EVENTS
#undef halScreenUpdated
extern void halScreenUpdated();

// MAKE TIMEOUT VARIABLE SO WE CAN HAVE SHORT TIMEOUT FOR DEVICE DETECTION
#undef USB_TIMEOUT_MS
#define USB_TIMEOUT_MS usb_timeout
extern int usb_timeout;

extern char PhysicalScreen[(LCD_SCANLINE*LCD_H)*4/PIXELS_PER_WORD*SCREEN_BUFFERS];
extern char ExceptionScreen[(LCD_SCANLINE*LCD_H)*4/PIXELS_PER_WORD];

typedef unsigned INTERRUPT_TYPE;

#endif // TARGET_PC_H
