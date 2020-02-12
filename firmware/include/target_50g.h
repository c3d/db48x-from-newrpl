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
#define __ARM_MODE__ __attribute__((target("arm"))) __attribute__((noinline))

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

#define halScreenUpdated()    ((void)0)

// Firmware preamble string

#define PREAMBLE_STRING "KINPOUPDATEIMAGE"

// Address of the serial number in this hardware
#define SERIAL_NUMBER_ADDRESS 0x3ff0

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
#define SCREEN_WIDTH 131
#define SCREEN_HEIGHT 80
#define PIXELS_PER_WORD 8

// DEFAULT COLOR MODE OF THE SYSTEM
#define DEFAULTBITSPERPIXEL 4
#define DEFAULTBITMAPMODE   1   // SAME AS BITMAP_RAW16G

// TEXT DISPLAYING CAPABILITY

// PHYSICAL SCREEN SIZE
#define SCREEN_W 160
#define SCREEN_H 80
#define ANN_X_COORD 131
#define ANN_Y_COORD 0



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

#endif // TARGET_50G_H
