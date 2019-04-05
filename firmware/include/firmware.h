#ifndef FIRMWARE_H
#define FIRMWARE_H

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
#define USBD_REGS   0x05200000
#define USBH_REGS   0x04900000


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

// USB DEVICE CONFIGURATION PARAMETERS

// You can change these to give your code its own name.
#define STR_MANUFACTURER	{'n',0,'e',0,'w',0,'R',0,'P',0,'L',0,' ',0,'T',0,'e',0,'a',0,'m',0}
#define STR_MANUFLENGTH   22+2
#define STR_PRODUCT		{'n',0,'e',0,'w',0,'R',0,'P',0,'L',0,' ',0,'C',0,'a',0,'l',0,'c',0}
#define STR_PRODLENGTH   22+2

// These 4 numbers identify your device.  Set these to
// something that is (hopefully) not used by any others!
#define VENDOR_ID		0x3f0
#define PRODUCT_ID		0x121   // ORIGINAL VID/PID OF THE 50g/39gs/40g TARGET HARDWARE
//#define PRODUCT_ID		0x441   // ORIGINAL VID/PID OF THE Prime TARGET HARDWARE
#define RAWHID_USAGE_PAGE	0xFFAB	// recommended: 0xFF00 to 0xFFFF
#define RAWHID_USAGE		0x0200	// recommended: 0x0100 to 0xFFFF

// These determine the bandwidth that will be allocated
// for your communication.  You do not need to use it
// all, but allocating more than necessary means reserved
// bandwidth is no longer available to other USB devices.
#define RAWHID_TX_SIZE		64	// transmit packet size
#define RAWHID_TX_INTERVAL	2	// max # of ms between transmit packets
#define RAWHID_RX_SIZE		64	// receive packet size
#define RAWHID_RX_INTERVAL	2	// max # of ms between receive packets



#define ENDPOINT0_SIZE		8
#define RAWHID_INTERFACE	0
#define RAWHID_TX_ENDPOINT	1
#define RAWHID_RX_ENDPOINT	2


// USB SUBSYSTEM STATUS BITS
#define USB_STATUS_INIT                 1
#define USB_STATUS_CONNECTED            2
#define USB_STATUS_CONFIGURED           4
#define USB_STATUS_EP0TX                8
#define USB_STATUS_EP0RX                16
#define USB_STATUS_HIDTX                32
#define USB_STATUS_HIDRX                64
#define USB_STATUS_DATAREADY            128
#define USB_STATUS_REMOTEBUSY           256
#define USB_STATUS_REMOTERESPND         512
#define USB_STATUS_WAKEUPENABLED        1024
#define USB_STATUS_SUSPEND              2048
#define USB_STATUS_TESTMODE             4096
#define USB_STATUS_IGNORE               8192
#define USB_STATUS_DATARECEIVED         16384

// USB DATA BLOCK MARKERS
#define USB_BLOCKMARK_SINGLE 0xa0
#define USB_BLOCKMARK_MULTISTART 0xa5
#define USB_BLOCKMARK_MULTI  0xa1
#define USB_BLOCKMARK_MULTIEND   0xaf
#define USB_BLOCKMARK_GETSTATUS  0xcd
#define USB_BLOCKMARK_RESPONSE 0xce

// MAXIMUM SIZE OF A BLOCK OF DATA, LARGER BLOCKS WILL BE SPLIT INTO MULTIPLE SMALLER BLOCKS
#define USB_BLOCKSIZE      (RAWHID_TX_SIZE-8)

#define USB_TIMEOUT_MS     5000















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

