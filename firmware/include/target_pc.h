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
#ifndef TARGET_PC_PRIMEG1
// USABLE SCREEN WINDOW SIZE
#define SCREEN_WIDTH 131
#define SCREEN_HEIGHT 80
#define PIXELS_PER_WORD 8

#define SCREEN_BUFFERS 0


// DEFAULT COLOR MODE OF THE SYSTEM
#define DEFAULTBITSPERPIXEL 4
#define DEFAULTBITMAPMODE   1   // SAME AS BITMAP_RAW16G

// PHYSICAL SCREEN SIZE
//  WIDTH MUST BE AT LEAST ONE MORE THAN THE WINDOW SIZE
#define SCREEN_W 160
// HEIGHT MUST BE AT LEAST THE SAME AS WINDOW SIZE
#define SCREEN_H 80



#define ANN_X_COORD (SCREEN_WIDTH)
#else
// USABLE SCREEN WINDOW SIZE
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define PIXELS_PER_WORD 2
#define SCREEN_BUFFERS 2

// DEFAULT COLOR MODE OF THE SYSTEM
#define DEFAULTBITSPERPIXEL 16
#define DEFAULTBITMAPMODE   3   // SAME AS BITMAP_RAW64KC

// PHYSICAL SCREEN SIZE
//  WIDTH MUST BE AT LEAST ONE MORE THAN THE WINDOW SIZE
#define SCREEN_W 320
// HEIGHT MUST BE AT LEAST THE SAME AS WINDOW SIZE
#define SCREEN_H 240

#define ANN_X_COORD (SCREEN_WIDTH)

// Prime menu organization constants
#define MENU1_ENDX  ((44*SCREEN_WIDTH)/131)
#define MENU2_STARTX (MENU1_ENDX+1)
#define MENU2_ENDX  (1+(88*SCREEN_WIDTH)/131)
#undef  STATUSAREA_X
#define STATUSAREA_X  (MENU2_ENDX+1)

#endif


// DEFAULT CLOCK SPEEDS
#define HAL_SLOWCLOCK     6000000
#define HAL_USBCLOCK     48000000
#define HAL_FASTCLOCK   192000000

extern char PhysicalScreen[(SCREEN_W*SCREEN_H)*4/PIXELS_PER_WORD*SCREEN_BUFFERS];
extern char ExceptionScreen[(SCREEN_W*SCREEN_H)*4/PIXELS_PER_WORD];

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
extern unsigned char const __keyb_codefrombit[64];
extern unsigned char const __keyb_bitfromcode[64];

// Keyboard mapping macros  - MUST exist for all targets
#define KEYMAP_CODEFROMBIT(bit) (__keyb_codefrombit[bit])
#define KEYMAP_BITFROMCODE(code) (__keyb_bitfromcode[code])

#endif


#endif // TARGET_PC_H
