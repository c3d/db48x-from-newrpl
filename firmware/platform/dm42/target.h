// ****************************************************************************
//  target.h                                                      BD48X project
// ****************************************************************************
//
//   File Description:
//
//     DM42 parameters
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
//   (C) 2014-2020 Claudio Lapilli and the newRPL Team
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************

#ifndef TARGET_DM42_H
#define TARGET_DM42_H

#include <stdint.h>

// Screen size
#define LCD_W 400
#define LCD_H 240
#define LCD_SCANLINE 416
#define LCD_H 240
#define SCREEN_BUFFERS 2

// Soft menu tab size
#define MENU_TAB_SPACE      1
#define MENU_TAB_INSET      2
#define MENU_TAB_WIDTH      ((LCD_W - 5 * MENU_TAB_SPACE) / 6)
#define MENU_TAB_HEIGHT     (FONT_HEIGHT(FONT_MENU) + 2 * MENU_TAB_INSET)

// Special constants for proper compilation of firmware image
#define SYSTEM_GLOBAL  __attribute__((section(".system_globals")))
#define DATAORDER1     __attribute__((section(".data_1")))
#define DATAORDER2     __attribute__((section(".data_2")))
#define DATAORDER3     __attribute__((section(".data_3")))
#define DATAORDERLAST  __attribute__((section(".data_last")))
#define SCRATCH_MEMORY __attribute__((section(".scratch_memory")))
#define ROMOBJECTS     __attribute__((section(".romobjects")))
#define ROMLINK        __attribute__((section(".romlink")))

#ifndef TARGET_PC
typedef struct {
	uint32_t mask1;
	uint32_t mask2;
} INTERRUPT_TYPE;
#endif

// This preamble preceeds PRIME_OS.ROM
struct Preamble {
	uint32_t entrypoint;
	uint32_t unused1;
	uint32_t copy_size;
	uint32_t load_addr;
	uint32_t load_size;
	uint32_t cpu_arch;
	uint32_t cpuid;
	uint32_t unused2;
} __attribute__ ((packed));


/*
    KEYBOARD BIT MAP
    ----------------
    This is the bit number in the 64-bit keymatrix.
    Bit set means key is pressed.
    Note that DMCP does not define keys as bitmaps,
    but rather using keycodes.

      +--------+--------+--------+--------+--------+--------+
      |   F1   |   F2   |   F3   |   F4   |   F5   |   F6   |
      |   38   |   39   |   40   |   41   |   42   |   43   |
      +--------+--------+--------+--------+--------+--------+
    S |  Sum-  |  y^x   |  x^2   |  10^x  |  e^x   |  GTO   |
      |  Sum+  |  1/x   |  Sqrt  |  Log   |  Ln    |  XEQ   |
      |   1    |   2    |   3    |   4    |   5    |   6    |
    A |   A    |   B    |   C    |   D    |   E    |   F    |
      +--------+--------+--------+--------+--------+--------+
    S | Complx |   %    |  Pi    |  ASIN  |  ACOS  |  ATAN  |
      |  STO   |  RCL   |  R_dwn |   SIN  |   COS  |   TAN  |
      |   7    |   8    |   9    |   10   |   11   |   12   |
    A |   G    |   H    |   I    |    J   |    K   |    L   |
      +--------+--------+--------+--------+--------+--------+
    S |     Alpha       | Last x |  MODES |  DISP  |  CLEAR |
      |     ENTER       |  x<>y  |  +/-   |   E    |   <--  |
      |       13        |   14   |   15   |   16   |   17   |
    A |                 |    M   |    N   |    O   |        |
      +--------+--------+-+------+----+---+-------++--------+
    S |   BST  | Solver   |  Int f(x) |  Matrix   |  STAT   |
      |   Up   |    7     |     8     |     9     |   /     |
      |   18   |   19     |    20     |    21     |   22    |
    A |        |    P     |     Q     |     R     |    S    |
      +--------+----------+-----------+-----------+---------+
    S |   SST  |  BASE    |  CONVERT  |  FLAGS    |  PROB   |
      |  Down  |    4     |     5     |     6     |    x    |
      |   23   |   24     |    25     |    26     |   27    |
    A |        |    T     |     U     |     V     |    W    |
      +--------+----------+-----------+-----------+---------+
    S |        | ASSIGN   |  CUSTOM   |  PGM.FCN  |  PRINT  |
      |  SHIFT |    1     |     2     |     3     |    -    |
      |   28   |   29     |    30     |    31     |   32    |
    A |        |    X     |     Y     |     Z     |    -    |
      +--------+----------+-----------+-----------+---------+
    S |  OFF   |  TOP.FCN |   SHOW    |   PRGM    | CATALOG |
      |  EXIT  |    0     |     .     |    R/S    |    +    |
      |   33   |   34     |    35     |    36     |   37    |
    A |        |    :     |     .     |     ?     |   ' '   |
      +--------+----------+-----------+-----------+---------+

*/

#define KB_ALPHA             28         //! Alpha
#define KB_ON                33         //! ON
#define KB_ESC               33         //! Exit
#define KB_DOT               53         //! Dot
#define KB_SPC               36         //! Space
#define KB_RUNSTOP           36         //! R/S
#define KB_QUESTION          36         //! ?
#define KB_SHIFT             28         //! Shift
#define KB_LSHIFT            28         //! Left shift
#define KB_RSHIFT            28         //! Right shift

#define KB_ADD               37         //! +
#define KB_SUB               32         //! -
#define KB_MUL               27         //! *
#define KB_DIV               22         //! /

#define KB_ENT               13         //! ENTER
#define KB_BKS               17         //! backspace
#define KB_UP                18         //! up arrow
#define KB_DN                23         //! down arrow
#define KB_LF                18         //! left arrow
#define KB_RT                23         //! right arrow

#define KB_F1                38         //! Function key 1
#define KB_F2                39         //! Function key 2
#define KB_F3                40         //! Function key 3
#define KB_F4                41         //! Function key 4
#define KB_F5                42         //! Function key 5
#define KB_F6                43         //! Function key 6

#define KB_0                 34         //! 0
#define KB_1                 29         //! 1
#define KB_2                 30         //! 2
#define KB_3                 31         //! 3
#define KB_4                 24         //! 4
#define KB_5                 25         //! 5
#define KB_6                 26         //! 6
#define KB_7                 19         //! 7
#define KB_8                 20         //! 8
#define KB_9                 21         //! 9

#define KB_A                  1         //! A
#define KB_B                  2         //! B
#define KB_C                  3         //! C
#define KB_D                  4         //! D
#define KB_E                  5         //! E
#define KB_F                  6         //! F
#define KB_G                  7         //! G
#define KB_H                  8         //! H
#define KB_I                  9         //! I
#define KB_J                 10         //! J
#define KB_K                 11         //! K
#define KB_L                 12         //! L
#define KB_M                 14         //! M
#define KB_N                 15         //! N
#define KB_O                 16         //! O
#define KB_P                 19         //! P
#define KB_Q                 20         //! Q
#define KB_R                 21         //! R
#define KB_S                 22         //! S
#define KB_T                 24         //! T
#define KB_U                 25         //! U
#define KB_V                 26         //! V
#define KB_W                 27         //! W
#define KB_X                 29         //! X
#define KB_Y                 30         //! Y
#define KB_Z                 31         //! Z

// Prime-specific keys (using their names) all map to 'RCL'
#define KB_APPS               8         //! APPS key (prime only)
#define KB_SYMB               8         //! SYMB key (prime only)
#define KB_HELP               8         //! HELP key (prime only)
#define KB_HOME               8         //! HOME key (prime only)
#define KB_PLOT               8         //! PLOT key (prime only)
#define KB_VIEW               8         //! VIEW key (prime only)
#define KB_CAS                8         //! CAS key (prime only)
#define KB_NUM                8         //! NUM key (prime only)
#define KB_MENU               8         //! MENU key (prime only)

#define halScreenUpdated()    ((void)0)

// DEFAULT CLOCK SPEEDS
#define HAL_SLOWCLOCK    100000000
#define HAL_USBCLOCK     400000000
#define HAL_FASTCLOCK    400000000

#define DEFAULT_AUTOOFFTIME 3

// DEFAULT COLOR MODE OF THE SYSTEM
#define BITS_PER_PIXEL        1
#define DEFAULT_BITMAP_MODE   0 // SAME AS BITMAP_RAWMONO
#define PIXELS_PER_WORD       64


// Low level timer functions for hardware setup

// Do a single delay 100 usec
void tmr_delay100us();
// Do a single delay 10 msec
void tmr_delay10ms();
// Do a single delay 20 msec
void tmr_delay20ms();
// Prepare for an open loop timeout
void tmr_setuptimeoutms(int delayms,unsigned int *start,unsigned int *end);
// Check if clock timed out or not
int tmr_timedout(unsigned int start,unsigned int end);

#define ENABLE_ARM_ASSEMBLY 1

// Magic word "NRPL"
#define NEWRPL_MAGIC   0x4C50524E

// Flight recorders must be tiny on DM42 (not much RAM)
#define FLIGHT_RECORDER(name, descr)    RECORDER(name, 2, descr)

// Display of battery level is in volts on the DM42
#define BATTERY_FORMAT  "%d.%02dV", battery/1000, battery/10 % 100

#include <host.h>

#endif // TARGET_DM42_H
