#ifndef FIRMWARE_H
#define FIRMWARE_H

// Firmware preamble string

#define PREAMBLE_STRING "KINPOUPDATEIMAGE"


#define DEFAULT_AUTOOFFTIME 3


// CONSTANTS THAT CHANGE WITH DIFFERENT TARGETS
#define RAM_BASE_PHYSICAL 0x08000000
#define RAM_END_PHYSICAL  0x08080000

#define MEM_PHYS_SCREEN  0x08006000
#define MEM_PHYS_EXSCREEN  0x08007900
#define MEM_VIRT_SCREEN  0x08006000

#define MEM_SYSGLOBALS   0x02005000

#define MEM_DSTKMMU      0x08008400

#define MEM_RSTKMMU      0x08008800

#define MEM_LAMMMU       0x08008C00

#define MEM_DIRMMU       0x08009000

#define MEM_TEMPBLKMMU   0x08009400

#define MEM_TEMPOBMMU    0x08009800


#define MEM_REVERSEMMU   0x0800A800

// HARDWARE CONSTANTS
#define MEM_REGS	0x04800000
#define SD_REGS		0x05A00000
#define IO_REGS 	0x05600000
#define LCD_REGS    0x04D00000
#define CLK_REGS    0x04C00000
#define RTC_REGS    0x05700000
#define TMR_REGS	0x05100000
#define WDT_REGS	0x05300000
#define INT_REGS	0x04A00000
#define ADC_REGS	0x05800000


// CLOCK MODE CONSTANTS
#define CLK_1MHZ 0x16000000
#define CLK_6MHZ 0x11000000
#define CLK_12MHZ 0x10000000
#define CLK_48MHZ 0x78023
#define CLK_75MHZ 0x43012
#define CLK_120MHZ 0x5c080
#define CLK_152MHZ 0x44011
#define CLK_192MHZ 0x58011


#define NUM_EVENTS  5   // NUMBER OF SIMULTANEOUS TIMED EVENTS

#define USER_MODE 0x10
#define FIQ_MODE  0x11
#define IRQ_MODE  0x12
#define SVC_MODE  0x13
#define ABT_MODE  0x17
#define UND_MODE  0x1b
#define SYS_MODE  0x1f




// VIDEO MODE CONSTANTS
#define MODE_MONO 0
#define MODE_4GRAY 1
#define MODE_16GRAY 2

// USABLE SCREEN SIZE
#define SCREEN_WIDTH 131
#define SCREEN_HEIGHT 80
#define STATUSAREA_X  66

// DEFAULT COLOR MODE OF THE SYSTEM

#define DEFAULTBITSPERPIXEL 4
#define DEFAULTBITMAPMODE   1       // SAME AS BITMAP_RAW16G


// MAIN EXCEPTION PROCESSOR

#define __EX_CONT 1		// SHOW CONTINUE OPTION
#define __EX_EXIT 2		// SHOW EXIT OPTION
#define __EX_WARM 4		// SHOW WARMSTART OPTION
#define __EX_RESET 8	// SHOW RESET OPTION
#define __EX_NOREG 16	// DON'T SHOW REGISTERS
#define __EX_WIPEOUT 32	// FULL MEMORY WIPEOUT AND WARMSTART
#define __EX_RPLREGS 64 // SHOW RPL REGISTERS INSTEAD
#define __EX_RPLEXIT 128 // SHOW EXIT OPTION, IT RESUMES EXECUTION AFTER SETTING Exception=EX_EXITRPL



// GENERAL HARDWARE REGISTER MACRO
#define HWREG(base,off) ( (volatile unsigned int *) (((int)base+(int)off)))


// TEXT DISPLAYING CAPABILITY

// PHYSICAL SCREEN SIZE
#define SCREEN_W 160
#define SCREEN_H 80

// STYLE DEFINITION CONSTANTS
#define CAPTIONHEIGHT 7
#define SOFTMENUHEIGHT 6
#define SCROLLBARWIDTH 3
#define BORDERWIDTH 1

#define CURSORBLINKSPEED 40000



// SPECIAL CONSTANTS FOR PROPER COMPILATION OF FIRMWARE IMAGE
#define __SYSTEM_GLOBAL__ __attribute__((section (".system_globals")))
#define __DATAORDER1__ __attribute__((section (".data_1")))
#define __DATAORDER2__ __attribute__((section (".data_2")))
#define __DATAORDER3__ __attribute__((section (".data_3")))
#define __DATAORDERLAST__ __attribute__((section (".data_last")))
#define __SCRATCH_MEMORY__ __attribute__((section (".scratch_memory")))
#define __ROMOBJECT__ __attribute__((section (".romobjects")))
#define __ROMLINK__  __attribute__((section (".romlink")))

#define __ENABLE_ARM_ASSEMBLY__ 1

// REDEFINE SOME CONSTANTS FOR THE VARIOUS TARGETS
#ifdef TARGET_PC
#include <target_pc.h>
#endif

#ifdef TARGET_39GS
#include <target_39gs.h>
#endif

#ifdef TARGET_40GS
#include <target_40gs.h>
#endif




#endif // FIRMWARE_H

