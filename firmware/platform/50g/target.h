/*
 * Copyright (c) 2014-2020 Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef TARGET_50G_H
#define TARGET_50G_H

// GENERAL HARDWARE REGISTER MACRO
#define HWREG(base,off) ( (volatile unsigned int *) (((int)base+(int)off)))
#define ARM_MODE __attribute__((target("arm"))) __attribute__((noinline))

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
#define USBD_REGS   0x05200000
#define USBH_REGS   0x04900000

// HARDWARE PORTS FOR S3C2410 - USB DEVICE

#define CLKCON              HWREG(CLK_REGS,0xc)

#define FUNC_ADDR_REG       HWREG(USBD_REGS,0x140)
#define PWR_REG             HWREG(USBD_REGS,0x144)
#define EP_INT_REG          HWREG(USBD_REGS,0x148)
#define USB_INT_REG         HWREG(USBD_REGS,0x158)
#define EP_INT_EN_REG       HWREG(USBD_REGS,0x15c)
#define USB_INT_EN_REG      HWREG(USBD_REGS,0x16c)
#define INDEX_REG           HWREG(USBD_REGS,0x178)
#define EP0_FIFO            HWREG(USBD_REGS,0x1c0)
#define EP1_FIFO            HWREG(USBD_REGS,0x1c4)
#define EP2_FIFO            HWREG(USBD_REGS,0x1c8)
#define EP3_FIFO            HWREG(USBD_REGS,0x1cc)
#define EP4_FIFO            HWREG(USBD_REGS,0x1d0)

// INDEXED REGISTERS
#define EP0_CSR             HWREG(USBD_REGS,0x184)
#define IN_CSR1_REG         HWREG(USBD_REGS,0x184)
#define IN_CSR2_REG         HWREG(USBD_REGS,0x188)
#define MAXP_REG            HWREG(USBD_REGS,0x180)
#define MAXP_REG2           HWREG(USBD_REGS,0x18c)
#define OUT_CSR1_REG        HWREG(USBD_REGS,0x190)
#define OUT_CSR2_REG        HWREG(USBD_REGS,0x194)
#define OUT_FIFO_CNT1_REG   HWREG(USBD_REGS,0x198)
#define OUT_FIFO_CNT2_REG   HWREG(USBD_REGS,0x19c)

// MISCELLANEOUS REGISTERS
#define MISCCR              HWREG(IO_REGS,0x80)
#define UPLLCON             HWREG(CLK_REGS,0x8)
#define CLKCON              HWREG(CLK_REGS,0xc)
#define CLKSLOW             HWREG(CLK_REGS,0x10)

#define CABLE_IS_CONNECTED  (*HWREG(IO_REGS,0x54)&2)

// VARIOUS STATES AND PIN CONSTANTS
#define USBSUSPND1  (1<<13)
#define USBSUSPND0  (1<<13)
#define USBPAD      (1<<3)

// CONTROL ENDPOINT CSR
#define EP0_OUT_PKT_RDY 1
#define EP0_IN_PKT_RDY  2
#define EP0_SENT_STALL  4
#define EP0_DATA_END    8
#define EP0_SETUP_END   16
#define EP0_SEND_STALL  32
#define EP0_SERVICED_OUT_PKT_RDY 64
#define EP0_SERVICED_SETUP_END 128

// OTHER ENDPOINTS CSR
#define EPn_OUT_SEND_STALL      0x20
#define EPn_IN_PKT_RDY          0x1
#define EPn_IN_FIFO_FLUSH       0x8
#define EPn_IN_SEND_STALL       0x10
#define EPn_IN_SENT_STALL       0x20
#define EPn_IN_CLR_DATA_TOGGLE  0x40
#define EPn_OUT_PKT_RDY          0x1
#define EPn_OUT_FIFO_FLUSH       0x10
#define EPn_OUT_SEND_STALL       0x20
#define EPn_OUT_SENT_STALL       0x40
#define EPn_OUT_CLR_DATA_TOGGLE  0x80

// OTHER BIT DEFINITIONS
#define USB_RESET        8

#define EP0_FIFO_SIZE    8
#define EP1_FIFO_SIZE    64
#define EP2_FIFO_SIZE    64

#define USB_DIRECTION   0x80
#define USB_DEV_TO_HOST 0x80

#define IO_GPDCON HWREG(IO_REGS,0x30)
#define IO_GPDDAT HWREG(IO_REGS,0x34)
#define IO_GPDUP HWREG(IO_REGS,0x38)
#define GPFCON HWREG(IO_REGS,0x50)
#define GPGCON HWREG(IO_REGS,0x60)
#define GPGDAT HWREG(IO_REGS,0x64)
#define GPGUP HWREG(IO_REGS,0x68)
#define EXTINT0 HWREG(IO_REGS,0x88)
#define EXTINT1 HWREG(IO_REGS,0x8c)
#define EINTMASK HWREG(IO_REGS,0xa4)
#define EINTPEND HWREG(IO_REGS,0xa8)
#define GPD(a) HWREG(IO_REGS,0x30+a)
#define GPE(a) HWREG(IO_REGS,0x40+a)
#define GPF(a) HWREG(IO_REGS,0x50+a)

#define SRCPND HWREG(INT_REGS,0x0)
#define INTMSK HWREG(INT_REGS,0x8)
#define INTPND HWREG(INT_REGS,0x10)

#define SDICON HWREG(SD_REGS,0)
#define SDIPRE HWREG(SD_REGS,0x4)
#define SDICARG HWREG(SD_REGS,0x8)
#define SDICCON HWREG(SD_REGS,0xc)
#define SDICSTA HWREG(SD_REGS,0x10)
#define SDIRSP0 HWREG(SD_REGS,0x14)
#define SDIRSP1 HWREG(SD_REGS,0x18)
#define SDIRSP2 HWREG(SD_REGS,0x1c)
#define SDIRSP3 HWREG(SD_REGS,0x20)
#define SDIDTIMER HWREG(SD_REGS,0x24)
#define SDIBSIZE HWREG(SD_REGS,0x28)
#define SDIDCON HWREG(SD_REGS,0x2c)
#define SDIDCNT HWREG(SD_REGS,0x30)
#define SDIDSTA HWREG(SD_REGS,0x34)
#define SDIFSTA HWREG(SD_REGS,0x38)
#define SDIDAT HWREG(SD_REGS,0x3c)
#define SDIIMSK HWREG(SD_REGS,0x40)


#define RTCCON  HWREG(RTC_REGS, 0x40)
#define TICNT0  HWREG(RTC_REGS, 0x44)
#define TICNT2  HWREG(RTC_REGS, 0x48)
#define TICNT1  HWREG(RTC_REGS, 0x4c)
#define RTCALM  HWREG(RTC_REGS, 0x50)
#define ALMSEC  HWREG(RTC_REGS, 0x54)
#define ALMMIN  HWREG(RTC_REGS, 0x58)
#define ALMHOUR HWREG(RTC_REGS, 0x5c)
#define ALMDATE HWREG(RTC_REGS, 0x60)
#define ALMMON  HWREG(RTC_REGS, 0x64)
#define ALMYEAR HWREG(RTC_REGS, 0x68)
#define BCDSEC  HWREG(RTC_REGS, 0x70)
#define BCDMIN  HWREG(RTC_REGS, 0x74)
#define BCDHOUR HWREG(RTC_REGS, 0x78)
#define BCDDATE HWREG(RTC_REGS, 0x7c)
#define BCDDAY  HWREG(RTC_REGS, 0x80)
#define BCDMON  HWREG(RTC_REGS, 0x84)
#define BCDYEAR HWREG(RTC_REGS, 0x88)
#define TICKCNT HWREG(RTC_REGS, 0x90)

#define RTCRST  HWREG(RTC_REGS, 0x6c)












#define halScreenUpdated()    ((void)0)

// Firmware preamble string

#define PREAMBLE_STRING "KINPOUPDATEIMAGE"

// Override the device string on the USB bus
#undef  STR_PRODUCT
#define STR_PRODUCT		{'n',0,'e',0,'w',0,'R',0,'P',0,'L',0,' ',0,'5',0,'0',0,'g',0}
#undef  STR_PRODLENGTH
#define STR_PRODLENGTH   20+2





// Address of the serial number in this hardware
#define SERIAL_NUMBER_ADDRESS 0x3ff0

#define DEFAULT_AUTOOFFTIME 3

// CONSTANTS THAT CHANGE WITH DIFFERENT TARGETS
#define RAM_BASE_PHYSICAL 0x08000000
#define RAM_END_PHYSICAL  0x08080000

#define SCREEN_BUFFERS  1
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



#define MEM_PHYSTACK    0x40000ffc      // PHYSICAL LOCATION OF THE "C" STACK (TOP OF STACK, DECREASES DOWN TO 0x40000000)
#define MEM_ROM         0x00000000      // VIRTUAL (AND PHYSICAL) ROM LOCATION (UP TO 4 MBytes)
#define MEM_DSTK        0x00400000      // DATA STACK VIRTUAL LOCATION (UP TO 4 MB)
#define MEM_RSTK        0x00800000      // RETURN STACK VIRTUAL LOCATION (UP TO 4 MB)
#define MEM_LAM         0x00C00000      // LOCAL VARIABLES VIRTUAL LOCATION (UP TO 4 MB)
#define MEM_DIRS        0x01000000      // GLOBAL DIRECTORIES VIRTUAL LOCATION (UP TO 4 MB)
#define MEM_TEMPBLOCKS  0x01400000      // BLOCK INDEX FOR TEMPOB VIRTUAL LOCATION (UP TO 4 MB)
#define MEM_TEMPOB      0x01800000      // GLOBAL OBJECT ALLOCATION MEMORY VIRTUAL LOCATION (UP TO 8 MB)
#define MEM_SYSTEM      0x02000000      // SYSTEM RAM (FIXED AMOUNT, MAPPED AT THE BEGINNING OF THE PHYSICAL RAM)
#define MEM_SRAM        0x03000000      // ON-CHIP SRAM
#define MEM_HARDWARE    0x04800000      // HARDWARE AND PERIPHERALS

/* PHYSICAL RAM LAYOUT:

  // FIRST 32 KBYTES ARE USED BY THE BOOTLOADER, SO NOTHING PERSISTENT CAN GO THERE.

  // VOLATILE MEMORY - LOST AFTER RESET/POWEROFF
  0x08000000 - EXCEPTION HANDLERS
  0x08000020 - SYSTEM VARIABLES USED BY BOOTLOADER TO DETERMINE POWER-OFF STATE
  0x08000100 - SCRATCH MEMORY (RREG MEMORY = 504 WORDS PER REGISTER, 8 REGISTERS)
  0x08005000 - SYSTEM GLOBAL VARIABLES (4 KB)
  0x08006000 - 0x08007900h SCREEN (8 KB)

  // PERSISTENT MEMORY
  0x08008000 - MMU TABLE BASE (1 KB) (MUST BE 32 KBYTE ALIGNED) - 1K MAPS FROM 0x000|00000 TO 0x0FF|00000 INCLUSIVE
  0x08008400 - DSTK 2ND TABLE (1 KB) 1K MAPS 256 4K PAGES = 1 MB OF RAM
  0x08008800 - RSTK 2ND TABLE (1 KB)
  0x08008C00 - LAM 2ND TABLE (1 KB)
  0x08009000 - DIRS 2ND TABLE (1 KB)
  0x08009400 - TEMPBLOCKS 2ND TABLE (1 KB)
  0x08009800 - TEMPOB 2ND TABLE (4 KB) 1K MAPS 4 MB OF RAM MAX (SPACE LEFT FOR FUTURE VIRTUAL MEMORY)
  0x0800A800 - REVERSE WALK TABLE (1 KB) - REVERSE MAP PHYSICAL TO VIRTUAL - INDICATES WHICH RAM PAGES ARE USED/FREE
  0x0800AC00 - 1 KB = PERSISTENT VARIABLES OF THE RPL CORE + MPD library MALLOC AREA (32K)
  0x0800XXXX - FIRST AVAILABLE PAGE OF RAM, AFTER END OF ALL PERSISTENT VARIABLES, ROUNDED TO 4K

 TOTAL USED FOR SYSTEM + MMU TABLES = 26 KBYTES (484 KBYTES AVAILABLE FOR THE USER)

*/













// CLOCK MODE CONSTANTS
#define CLK_1MHZ 0x16000000
#define CLK_6MHZ 0x11000000
#define CLK_12MHZ 0x10000000
#define CLK_48MHZ 0x78023
#define CLK_75MHZ 0x43012
#define CLK_120MHZ 0x5c080
#define CLK_152MHZ 0x44011
#define CLK_192MHZ 0x58011

// DEFAULT CLOCK SPEEDS
#define HAL_SLOWCLOCK     6000000
#define HAL_USBCLOCK     48000000
#define HAL_FASTCLOCK   192000000

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
#define LCD_W 131
#define LCD_H 80
#define PIXELS_PER_WORD 8

#define MENU2_STARTX 0
#define MENU2_ENDX  (1+(88*LCD_W)/131)
#define MENU2_COUNT  2

// DEFAULT COLOR MODE OF THE SYSTEM
#define BITS_PER_PIXEL 4
#define DEFAULT_BITMAP_MODE   1   // SAME AS BITMAP_RAW16G

// TEXT DISPLAYING CAPABILITY

// PHYSICAL SCREEN SIZE
#define LCD_SCANLINE 160
#define LCD_H 80
#define ANN_X_COORD 131
#define ANN_Y_COORD 0



// STYLE DEFINITION CONSTANTS
#define CAPTIONHEIGHT 7
#define SOFTMENUHEIGHT 6
#define SCROLLBARWIDTH 3
#define BORDERWIDTH 1

#define CURSORBLINKSPEED 40000

// SPECIAL CONSTANTS FOR PROPER COMPILATION OF FIRMWARE IMAGE
#define SYSTEM_GLOBAL __attribute__((section (".system_globals")))
#define DATAORDER1 __attribute__((section (".data_1")))
#define DATAORDER2 __attribute__((section (".data_2")))
#define DATAORDER3 __attribute__((section (".data_3")))
#define DATAORDERLAST __attribute__((section (".data_last")))
#define SCRATCH_MEMORY __attribute__((section (".scratch_memory")))
#define ROMOBJECTS __attribute__((section (".romobjects")))
#define ROMLINK  __attribute__((section (".romlink")))

#define ENABLE_ARM_ASSEMBLY 1

typedef unsigned INTERRUPT_TYPE;


/*

KEYBOARD BIT MAP
----------------
This is the bit number in the 64-bit keymatrix.
Bit set means key is pressed.

    A]-+  B]-+  C]-+  D]-+  E]-+  F]-+
    |41|  |42|  |43|  |44|  |45|  |46|
    +--+  +--+  +--+  +--+  +--+  +--+

    G]-+  H]-+  I]-+        UP]+
    |47|  |53|  |54|        |49|
    +--+  +--+  +--+  LF]+  +--+  RT]+
                      |50|  DN]+  |52|
    J]-+  K]-+  L]-+  +--+  |51|  +--+
    |55|  |57|  |58|        +--+
    +--+  +--+  +--+

    M]--+  N]--+  O]--+  P]--+  BKS]+
    | 33|  | 25|  | 17|  | 09|  | 01|
    +---+  +---+  +---+  +---+  +---+

    Q]--+  R]--+  S]--+  T]--+  U]--+
    | 34|  | 26|  | 18|  | 10|  | 02|
    +---+  +---+  +---+  +---+  +---+

    V]--+  W]--+  X]--+  Y]--+  /]--+
    | 35|  | 27|  | 19|  | 11|  | 03|
    +---+  +---+  +---+  +---+  +---+

    AL]-+  7]--+  8]--+  9]--+  *]--+
    | 60|  | 28|  | 20|  | 12|  | 04|
    +---+  +---+  +---+  +---+  +---+

    LS]-+  4]--+  5]--+  6]--+  -]--+
    | 61|  | 29|  | 21|  | 13|  | 05|
    +---+  +---+  +---+  +---+  +---+

    RS]-+  1]--+  2]--+  3]--+  +]--+AL_API
    | 62|  | 30|  | 22|  | 14|  | 06|
    +---+  +---+  +---+  +---+  +---+

    ON]-+  0]--+  .]--+  SP]-+  EN]-+
    | 63|  | 31|  | 23|  | 15|  | 07|
    +---+  +---+  +---+  +---+  +---+

*/

#define KB_ALPHA             60         //! ALPHA
#define KB_ON                63         //! ON
#define KB_ESC               63         //! ESC -> ON
#define KB_DOT               23         //! .
#define KB_SPC               15         //! SPC
#define KB_QUESTION           0         //! ? not mapped
#define KB_SHIFT             61         //! Shift -> left shift
#define KB_LSHIFT            61         //! left shift
#define KB_RSHIFT            62         //! right shift

#define KB_ADD               6          //! +
#define KB_SUB               5          //! -
#define KB_MUL               4          //! *
#define KB_DIV               3          //! / (Z)

#define KB_ENT               7          //! ENT
#define KB_BKS               1          //! backspace
#define KB_UP                49         //! up arrow
#define KB_DN                51         //! down arrow
#define KB_LF                50         //! left arrow
#define KB_RT                52         //! right arrow

#define KB_F1                41         //! Function key 1
#define KB_F2                42         //! Function key 2
#define KB_F3                43         //! Function key 3
#define KB_F4                44         //! Function key 4
#define KB_F5                45         //! Function key 5
#define KB_F6                46         //! Function key 6

#define KB_0                 31         //! 0
#define KB_1                 30         //! 1
#define KB_2                 22         //! 2
#define KB_3                 14         //! 3
#define KB_4                 29         //! 4
#define KB_5                 21         //! 5
#define KB_6                 13         //! 6
#define KB_7                 28         //! 7
#define KB_8                 20         //! 8
#define KB_9                 12         //! 9

#define KB_A                 41         //! F1 (A)
#define KB_B                 42         //! F2 (B)
#define KB_C                 43         //! F3 (C)
#define KB_D                 44         //! F4 (D)
#define KB_E                 45         //! F5 (E)
#define KB_F                 46         //! F6 (F)
#define KB_G                 47         //! APPS (G)
#define KB_H                 53         //! MODE (H)
#define KB_I                 54         //! TOOL (I)
#define KB_J                 55         //! VAR (J)
#define KB_K                 57         //! STO (K)
#define KB_L                 58         //! NXT (L)
#define KB_M                 33         //! HIST (M)
#define KB_N                 25         //! EVAL (N)
#define KB_O                 17         //! ' (O)
#define KB_P                  9         //! SYMB (P)
#define KB_Q                 34         //! Y^X (Q)
#define KB_R                 26         //! Sqrt (R)
#define KB_S                 18         //! SIN (S)
#define KB_T                 10         //! COS (T)
#define KB_U                  2         //! TAN (U)
#define KB_V                 35         //! EEX (V)
#define KB_W                 27         //! +/- (W)
#define KB_X                 19         //! X (X)
#define KB_Y                 11         //! 1/X (Y)
#define KB_Z                  3         //! / (Z)

// Prime-specific keys (using their names) all map to 'P'
#define KB_APPS               9         //! APPS key (prime only)
#define KB_SYMB               9         //! SYMB key (prime only)
#define KB_HELP               9         //! HELP key (prime only)
#define KB_HOME               9         //! HOME key (prime only)
#define KB_PLOT               9         //! PLOT key (prime only)
#define KB_VIEW               9         //! VIEW key (prime only)
#define KB_CAS                9         //! CAS key (prime only)
#define KB_NUM                9         //! NUM key (prime only)
#define KB_MENU               9         //! MENU key (prime only)

#include <host.h>

#endif // TARGET_50G_H
