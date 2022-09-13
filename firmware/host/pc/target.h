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
extern void halScreenUpdated();

// MAKE TIMEOUT VARIABLE SO WE CAN HAVE SHORT TIMEOUT FOR DEVICE DETECTION
#undef USB_TIMEOUT_MS
#define USB_TIMEOUT_MS usb_timeout
extern int usb_timeout;


// Target PC uses 50g screen and other capabilities for now
#ifndef TARGET_PC_PRIMEG1
// USABLE SCREEN WINDOW SIZE
#define LCD_W 131
#define LCD_H 80
#define PIXELS_PER_WORD 8

#define SCREEN_BUFFERS 1


// DEFAULT COLOR MODE OF THE SYSTEM
#define DEFAULTBITSPERPIXEL 4
#define DEFAULT_BITMAP_MODE   1   // SAME AS BITMAP_RAW16G

// PHYSICAL SCREEN SIZE
//  WIDTH MUST BE AT LEAST ONE MORE THAN THE WINDOW SIZE
#define LCD_SCANLINE 160
// HEIGHT MUST BE AT LEAST THE SAME AS WINDOW SIZE
#define LCD_H 80

// HP50G menu organization
#define MENU2_STARTX 0
#define MENU2_ENDX   SCREEN_W
#define MENU2_COUNT  3

#define ANN_X_COORD (LCD_W)

#else // TARGET_PC_PRIMEG1

// USABLE SCREEN WINDOW SIZE
#define LCD_W 320
#define LCD_H 240
#define PIXELS_PER_WORD 2
#define SCREEN_BUFFERS 2

// DEFAULT COLOR MODE OF THE SYSTEM
#define DEFAULTBITSPERPIXEL 16
#define DEFAULT_BITMAP_MODE   3   // SAME AS BITMAP_RAW64KC

// PHYSICAL SCREEN SIZE
//  WIDTH MUST BE AT LEAST ONE MORE THAN THE WINDOW SIZE
#define LCD_SCANLINE 320
// HEIGHT MUST BE AT LEAST THE SAME AS WINDOW SIZE
#define LCD_H 240

#define ANN_X_COORD (LCD_W)
#define MENU1_ENDX  ((44*LCD_W)/131)

// Prime menu organization constants
#define MENU2_STARTX (MENU1_ENDX+1)
#define MENU2_ENDX  (1+(88*LCD_W)/131)
#define MENU2_COUNT  2

#undef  STATUSAREA_X
#define STATUSAREA_X  (MENU2_ENDX+1)

#endif // TARGET_PC_PRIMEG1

// DEFAULT CLOCK SPEEDS
#define HAL_SLOWCLOCK     6000000
#define HAL_USBCLOCK     48000000
#define HAL_FASTCLOCK   192000000

extern char PhysicalScreen[(LCD_SCANLINE*LCD_H)*4/PIXELS_PER_WORD*SCREEN_BUFFERS];
extern char ExceptionScreen[(LCD_SCANLINE*LCD_H)*4/PIXELS_PER_WORD];

typedef unsigned int INTERRUPT_TYPE;















#ifndef TARGET_PC_PRIMEG1

// Keyboard remapping constants

// Keymatrix mask to isolate all shifts (Left, Right and Alpha)
#define KEYMATRIX_ALL_SHIFTS   0x7000000000000000LL
#define KEYMATRIX_ON           0x8000000000000000LL
#define KEYMATRIX_LSHIFTBIT(matrix)    (((matrix)>>61)&1)
#define KEYMATRIX_RSHIFTBIT(matrix)    (((matrix)>>62)&1)
#define KEYMATRIX_ALPHABIT(matrix)    (((matrix)>>60)&1)


// Keyboard mapping macros  - MUST exist for all targets
#define KEYMAP_CODEFROMBIT(bit) (bit)
#define KEYMAP_BITFROMCODE(code) (code)

#else

// Keyboard remapping constants

// Keymatrix mask to isolate all shifts (Left, Right and Alpha)
#define KEYMATRIX_ALL_SHIFTS   ((1LL<<26)|(1LL<<51)|(1LL<<63))
#define KEYMATRIX_ON           (1LL<<52)
#define KEYMATRIX_LSHIFTBIT(matrix)    (((matrix)>>51)&1)
#define KEYMATRIX_RSHIFTBIT(matrix)    (((matrix)>>63)&1)
#define KEYMATRIX_ALPHABIT(matrix)    (((matrix)>>26)&1)



// Matrix to KeyCode mapping - Defined in keyboard.c for this target
extern const unsigned char keyb_irq_codefrombit[64];
extern const unsigned char keyb_irq_bitfromcode[64];

// Keyboard mapping macros  - MUST exist for all targets
#define KEYMAP_CODEFROMBIT(bit) (keyb_irq_codefrombit[bit])
#define KEYMAP_BITFROMCODE(code) (keyb_irq_bitfromcode[code])

#endif


#endif // TARGET_PC_H
