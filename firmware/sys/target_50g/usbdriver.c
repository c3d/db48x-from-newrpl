/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include <newrpl.h>
#include <ui.h>


// ADD-ON MEMORY ALLOCATOR - SHARED WITH FILE SYSTEM AND NEWRPL ENGINE
extern void init_simpalloc();
extern unsigned int *simpmalloc(int words);
extern unsigned char *simpmallocb(int bytes);
extern void simpfree(void *voidptr);

// OTHER EXTERNAL FUNCTIONS NEEDED
extern int __cpu_getPCLK();



/* ********************************************

  UNIVERSAL FILE TRANSMISSION PROTOCOL FOR USB

  ALL PACKETS ARE 64 BYTES

  PACKET: FILE FRAGMENT (FIRST)

  1 BYTE  = PACKET TYPE AND SEQUENCE NUMBER (0 TO 0x3F) UP TO 63 PACKETS PER FRAGMENT, 0 = FIRST PACKET OF THE FRAGMENT
            LAST PACKET OF THE FRAGMENT HAS BIT 0x40 SET
  1 BYTE  = DATA BYTES USED IN THIS PACKET (NORMALLY 56 FOR FULL PACKET, MAY BE PARTIALLY FULL OR EMPTY)
  2 BYTES = FILE ID
  4 BYTES = FRAGMENT OFFSET
  56 BYTES = DATA

  PACKET: CONTROL PACKETS

  GET_STATUS PACKET
  1 BYTES = PACKET TYPE 0x80 = GET_STATUS (SENT TO GET A STATUS FROM THE REMOTE)
  1 BYTE  = 0 (NO DATA IN THIS PACKET)
  2 BYTES = FILE ID (GET INFORMATION ABOUT CURRENT OR PREVIOUS FILE SENT)
  (ALL OTHER BYTES EMPTY)

  CHECKPOINT PACKET
  1 BYTES = PACKET TYPE 0x81 = CHECKPOINT
  1 BYTE = 0 (NO DATA)
  2 BYTES = FILE ID (MUST MATCH FRAGMENTS)
  4 BYTES = TOTAL BYTES SENT SO FAR (OFFSET AFTER DATA SENT)
  4 BYTES = PARTIAL CRC32 OF THE DATA UP UNTIL GIVEN OFFSET

  END_OF_FILE PACKET
  1 BYTES = PACKET TYPE 0x82 = END_OF_FILE
  1 BYTE = 0 (NO DATA)
  2 BYTES = FILE ID (MUST MATCH FRAGMENTS)
  4 BYTES = TOTAL SIZE (CURRENT OFFSET AFTER SENDING ALL DATA)
  4 BYTES = FINAL CRC32 OF THE DATA

  ABORT PACKET
  1 BYTES = PACKET TYPE 0X83 = ABORT
  1 BYTE = 0 (NO DATA)
  2 BYTES = FILEID (MUST MATCH FILEID)

  ABORT PACKET CAN BE SENT BY BOTH SENDER AND RECEIVER
  IF SENDER INITIATES ABORT, IT MUST STOP SENDING ALL DATA IN THAT FILE, RECEIVER WILL CANCEL TRANSMISSION AND DISCARD ALL DATA ASSOCIATED WITH FILEID
  IF RECEIVER INITIATES ABORT, IT MUST DISCARD ALL DATA ASSOCIATED WITH FILEID, AND IGNORE ALL INCOMING PACKETS USING THAT FILEID. SENDER WILL STOP SENDING DATA AS SOON AS IT RECEIVES THE ABORT REQUEST.

  STATUS_REPORT PACKET
  1 BYTES = PACKET TYPE 0x84 = STATUS_REPORT
  1 BYTE  = 0 (NO DATA IN THIS PACKET)
  2 BYTES = FILE ID (0 MEANS IDLE, NOT RECEIVING ANYTHING)
  4 BYTES = HIGHEST RECEIVED OFFSET SO FAR
  1 BYTES = 0 = OK TO RECEIVE MORE DATA, 1 = ONE BUFFER IS FULL, HALT DATA UNTIL IT PROGRAM READS IT (OTHER BUFFER IS STILL AVAILABLE TO RECEIVE PACKETS THAT WERE ALREADY SENT)
  1 BYTES = 0 = CRC32 OK SO FAR, 1 = CRC OR OTHER ERROR: RESEND FRAGMENTS FROM THE GIVEN OFFSET



  COMMUNICATION PROTOCOL:

  A FILE WILL BE SENT IN FRAGMENTS OF 32 PACKETS

  TO START SENDING A FILE:

  A)
  TX: GET_STATUS (WITH NEW FILEID, AND WAITS FOR ANSWER)
  RX: STATUS_REPORT (TO SEE IF IT'S READY TO RECEIVE DATA)

  IF BUSY, SENDER SENDS GOES BACK TO A) AND SEND GET_STATUS REPORT EVERY 200 MS

  B) SENDER HAS THE OK TO SEND DATA

  TX: DATA PACKETS 0 TO 31 TO COMPLETE A FRAGMENT

  TX: SEND CHECKPOINT PACKET OR END_OF_FILE PACKET
  RX: SEND STATUS_REPORT AFTER RECEIVING CHECKPOINT PACKET


  WHEN RECEIVER GETS A CHECKPOINT, REPORT THE STATUS IMMEDIATELY
  SENDER NEEDS TO WAIT FOR THE STATUS REPORT BEFORE SENDING NEXT FRAGMENT, TIMEOUT IF TOO MUCH TIME PASSED


  IF SENDER STARTS A NEW TRANSMISSION (FILEID CHANGED) WITHOUT SENDING AN ABORT OR END_OF_FILE (OR THE RECEIVER MISSED THEM), RECEIVER NEEDS TO ABORT THE PREVIOUS FILEID, THEN START RECEIVING THE NEW FILE

  WHEN STATUS_REPORT INDICATES AN ERROR, SENDER NEEDS TO RESEND FROM THE OFFSET REPORTED BY THE RECEIVER.
  IF DATA THAT CAN'T BE RESENT (LIKE FOR BACKUP OBJECTS), SENDER NEEDS TO SEND AN ABORT PACKET

  IF AT ANY POINT THERE'S A TIMEOUT: RECEIVER ABORTS ENTIRE FILEID AND REVERTS TO IDLE STATE WITHOUT SENDING ABORT SIGNAL
                                     SENDER ABORTS THE CURRENT FILEID WITHOUT SENDING THE ABORT SIGNAL

  ALL FILES, LARGE AND SMALL AND ARCHIVES AND FIRMWARE UPDATES WILL USE THE SAME PROTOCOL
  FILEID WILL BE USED TO IDENTIFY THE TYPE OF TRANSMISSION:
  MSB = 'O' FOR REGULAR RPL OBJECTS TO BE RECEIVED BY AUTORECV
  MSB = 'B' FOR BACKUP OBJECTS TO BE RECEIVED BY USBRESTORE
  MSB = 'W' FOR FIRMWARE UPDATES
  MSB = 'D' FOR RAW BINARY DATA (NOT TO BE INTERPRETED AS RPL OBJECTS)
  LSB SHOULD BE INCREMENTED EACH TIME A FILE IS CREATED

*/



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


//********************************************************************
// THE DESCRIPTOR DATA BELOW WAS COPIED AND PORTED FROM THE Teensy rawHID
// EXAMPLE IN AGREEMENT WITH THE LICENSE BELOW:

/* Teensy RawHID example
 * http://www.pjrc.com/teensy/rawhid.html
 * Copyright (c) 2009 PJRC.COM, LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above description, website URL and copyright notice and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


/**************************************************************************
 *
 *  Configurable Options
 *
 **************************************************************************/


/**************************************************************************
 *
 *  Descriptor Data
 *
 **************************************************************************/

// standard control endpoint request types
#define GET_STATUS			0
#define CLEAR_FEATURE			1
#define SET_FEATURE			3
#define SET_ADDRESS			5
#define GET_DESCRIPTOR			6
#define GET_CONFIGURATION		8
#define SET_CONFIGURATION		9
#define GET_INTERFACE			10
#define SET_INTERFACE			11
// HID (human interface device)
#define HID_GET_REPORT			1
#define HID_GET_IDLE			2
#define HID_GET_PROTOCOL		3
#define HID_SET_REPORT			9
#define HID_SET_IDLE			10
#define HID_SET_PROTOCOL		11

// other control definitions
#define EP_TYPE_CONTROL			0x00
#define EP_TYPE_BULK_IN			0x81
#define EP_TYPE_BULK_OUT		0x80
#define EP_TYPE_INTERRUPT_IN		0xC1
#define EP_TYPE_INTERRUPT_OUT		0xC0
#define EP_TYPE_ISOCHRONOUS_IN		0x41
#define EP_TYPE_ISOCHRONOUS_OUT		0x40

#define EP_SINGLE_BUFFER		0x02
#define EP_DOUBLE_BUFFER		0x06

#define EP_SIZE(s)	((s) > 32 ? 0x30 :	\
            ((s) > 16 ? 0x20 :	\
            ((s) > 8  ? 0x10 :	\
                        0x00)))

#define MAX_ENDPOINT		4

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)


// Descriptors are the data that your computer reads when it auto-detects
// this USB device (called "enumeration" in USB lingo).  The most commonly
// changed items are editable at the top of this file.  Changing things
// in here should only be done by those who've read chapter 9 of the USB
// spec and relevant portions of any USB class specifications!


const BYTE const device_descriptor[] = {
    18,					// bLength
    1,					// bDescriptorType
    0x10, 0x01,				// bcdUSB
    0,					// bDeviceClass
    0,					// bDeviceSubClass
    0,					// bDeviceProtocol
    ENDPOINT0_SIZE,				// bMaxPacketSize0
    LSB(VENDOR_ID), MSB(VENDOR_ID),		// idVendor
    LSB(PRODUCT_ID), MSB(PRODUCT_ID),	// idProduct
    0x00, 0x01,				// bcdDevice
    1,					// iManufacturer
    2,					// iProduct
    3,					// iSerialNumber
    1					// bNumConfigurations
};

const BYTE const rawhid_hid_report_desc[] = {
    0x06, LSB(RAWHID_USAGE_PAGE), MSB(RAWHID_USAGE_PAGE),
    0x0A, LSB(RAWHID_USAGE), MSB(RAWHID_USAGE),
    0xA1, 0x01,				// Collection 0x01
    0x75, 0x08,				// report size = 8 bits
    0x15, 0x00,				// logical minimum = 0
    0x26, 0xFF, 0x00,			// logical maximum = 255
    0x95, RAWHID_TX_SIZE,			// report count
    0x09, 0x01,				// usage
    0x81, 0x02,				// Input (array)
    0x95, RAWHID_RX_SIZE,			// report count
    0x09, 0x02,				// usage
    0x91, 0x02,				// Output (array)
    0xC0					// end collection
};


#define CONFIG1_DESC_SIZE        (9+9+9+7+7)
#define RAWHID_HID_DESC_OFFSET   (9+9)
const BYTE const config1_descriptor[CONFIG1_DESC_SIZE] = {
    // configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
    9, 					// bLength;
    2,					// bDescriptorType;
    LSB(CONFIG1_DESC_SIZE),			// wTotalLength
    MSB(CONFIG1_DESC_SIZE),
    1,					// bNumInterfaces
    1,					// bConfigurationValue
    0,					// iConfiguration
    0xC0,					// bmAttributes
    50,					// bMaxPower

    // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
    9,					// bLength
    4,					// bDescriptorType
    RAWHID_INTERFACE,			// bInterfaceNumber
    0,					// bAlternateSetting
    2,					// bNumEndpoints
    0x03,					// bInterfaceClass (0x03 = HID)
    0x00,					// bInterfaceSubClass (0x01 = Boot)
    0x00,					// bInterfaceProtocol (0x01 = Keyboard)
    0,					// iInterface
    // HID interface descriptor, HID 1.11 spec, section 6.2.1
    9,					// bLength
    0x21,					// bDescriptorType
    0x11, 0x01,				// bcdHID
    0,					// bCountryCode
    1,					// bNumDescriptors
    0x22,					// bDescriptorType
    sizeof(rawhid_hid_report_desc),		// wDescriptorLength
    0,
    // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
    7,					// bLength
    5,					// bDescriptorType
    RAWHID_TX_ENDPOINT | 0x80,		// bEndpointAddress
    0x03,					// bmAttributes (0x03=intr)
    RAWHID_TX_SIZE, 0,			// wMaxPacketSize
    RAWHID_TX_INTERVAL,			// bInterval
    // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
    7,					// bLength
    5,					// bDescriptorType
    RAWHID_RX_ENDPOINT,			// bEndpointAddress
    0x03,					// bmAttributes (0x03=intr)
    RAWHID_RX_SIZE, 0,			// wMaxPacketSize
    RAWHID_RX_INTERVAL			// bInterval
};

// If you're desperate for a little extra code memory, these strings
// can be completely removed if iManufacturer, iProduct, iSerialNumber
// in the device desciptor are changed to zeros.

struct usb_string_descriptor_struct {
    BYTE bLength;
    BYTE bDescriptorType;
    BYTE wString[];
};
const struct usb_string_descriptor_struct const _usb_string0 = {
    4,
    3,
    {0x09,0x04}
};
const struct usb_string_descriptor_struct const _usb_string1 = {
    STR_MANUFLENGTH,
    3,
    STR_MANUFACTURER
};
const struct usb_string_descriptor_struct const _usb_string2 = {
    STR_PRODLENGTH,
    3,
    STR_PRODUCT
};

// This table defines which descriptor data is sent for each specific
// request from the host (in wValue and wIndex).
const struct descriptor_list_struct {
    HALFWORD	wValue;
    HALFWORD	wIndex;
    const BYTE	*addr;
    BYTE		length;
} const descriptor_list[] = {
    {0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
    {0x0200, 0x0000, config1_descriptor, sizeof(config1_descriptor)},
    {0x2200, RAWHID_INTERFACE, rawhid_hid_report_desc, sizeof(rawhid_hid_report_desc)},
    {0x2100, RAWHID_INTERFACE, config1_descriptor+RAWHID_HID_DESC_OFFSET, 9},
    {0x0300, 0x0000, (const BYTEPTR )&_usb_string0, 4},
    {0x0301, 0x0409, (const BYTEPTR )&_usb_string1, STR_MANUFLENGTH},
    {0x0302, 0x0409, (const BYTEPTR )&_usb_string2, STR_PRODLENGTH}
};

#define NUM_DESC_LIST ((int)(sizeof(descriptor_list)/sizeof(struct descriptor_list_struct)))




//********************************************************************
// END OF DEFINITIONS BORROWED FROM Teensy rawHID project.
// EVERYTHING BELOW WAS WRITTEN FROM SCRATCH BY THE newRPL Team

#define LONG_BUFFER_SIZE        32*RAWHID_RX_SIZE
/*
// GLOBAL VARIABLES OF THE USB SUBSYSTEM
BINT __usb_drvstatus __SYSTEM_GLOBAL__; // FLAGS TO INDICATE IF INITIALIZED, CONNECTED, SENDING/RECEIVING, ETC.
BYTE *__usb_bufptr[3] __SYSTEM_GLOBAL__;   // POINTERS TO BUFFERS FOR EACH ENDPOINT (0/1/2)
BINT __usb_count[3]   __SYSTEM_GLOBAL__;   // REMAINING BYTES TO TRANSFER ON EACH ENDPOINT (0/1/2)
BINT __usb_padding[3] __SYSTEM_GLOBAL__;    // PADDING FOR OUTGOING TRANSFERS
BYTE __usb_rxtmpbuffer[RAWHID_RX_SIZE+1] __SYSTEM_GLOBAL__;  // TEMPORARY BUFFER FOR NON-BLOCKING CONTROL TRANSFERS
BYTE __usb_txtmpbuffer[RAWHID_TX_SIZE+1] __SYSTEM_GLOBAL__;  // TEMPORARY BUFFER FOR NON-BLOCKING CONTROL TRANSFERS
BYTEPTR __usb_rcvbuffer __SYSTEM_GLOBAL__;
BINT __usb_rcvtotal __SYSTEM_GLOBAL__;
BINT __usb_rcvpartial __SYSTEM_GLOBAL__;
WORD __usb_rcvcrc __SYSTEM_GLOBAL__;
WORD __usb_rcvcrcroll __SYSTEM_GLOBAL__;
BINT __usb_rcvblkmark __SYSTEM_GLOBAL__;    // TYPE OF RECEIVED BLOCK (ONE OF USB_BLOCKMARK_XXX CONSTANTS)
BYTEPTR __usb_sndbuffer __SYSTEM_GLOBAL__;
BINT __usb_sndtotal __SYSTEM_GLOBAL__;
BINT __usb_sndpartial __SYSTEM_GLOBAL__;
BINT __usb_rembigtotal __SYSTEM_GLOBAL__;
BINT __usb_rembigoffset __SYSTEM_GLOBAL__;
BINT __usb_localbigoffset __SYSTEM_GLOBAL__;

// GLOBAL VARIABLES FOR DOUBLE BUFFERED LONG TRANSACTIONS
BYTEPTR __usb_longbuffer[2];              // DOUBLE BUFFERING FOR LONG TRANSMISSIONS OF DATA
BINT __usb_longbufused[2];              // DOUBLE BUFFERING FOR LONG TRANSMISSIONS OF DATA
BINT __usb_longoffset,__usb_longrdoffset;
BINT __usb_longactbuffer,__usb_longrdbuffer;  // WHICH BUFFER IS BEING WRITTEN AND WHICH ONE IS BEING READ
BINT __usb_longlastsize;                  // LAST BLOCK SIZE IN A LONG TRANSMISSION
WORD __usb_longcrcroll;
*/
// ********************************
// NEW SIMPLIFIED DEFINITIONS

// PACKET STRUCTURE
typedef struct {
    BYTE p_type;
    BYTE p_dataused;
    BYTE p_fileidLSB;
    BYTE p_fileidMSB;
    WORD p_offset;
    BYTE p_data[56];
} USB_PACKET;

// MAKE A 16 BIT FILEID FROM ITS PARTS
#define P_FILEID(pptr)  (( (WORD)((pptr)->p_fileidLSB))|(((WORD)((pptr)->p_fileidMSB))<<8))

// MAKE A CRC32 FOR STATUS REPORT AND CHECKPOINT PACKETS
#define P_CRC32(pptr)  (( (WORD)((pptr)->p_data[0]))|(((WORD)((pptr)->p_data[1]))<<8))|(((WORD)((pptr)->p_data[2]))<<16))|(((WORD)((pptr)->p_data[3]))<<24)))
// CHECK STATUS BYTE ON REPORT PACKETS
#define P_BUFSTATUS(pptr) ((pptr)->p_data[0])
// CHECK CRC AND OTHER ERROR CONDITIONS ON REPORT PACKETS
#define P_CRCSTATUS(pptr) ((pptr)->p_data[1])

// DISTINGUISH DATA PACKETS FROM CONTROL PACKETS
#define P_ISFRAGMENT(pptr) (!(((pptr)->p_type)&0x80))

// PACKET TYPES
#define P_TYPE_GETSTATUS    0x80
#define P_TYPE_CHECKPOINT   0x81
#define P_TYPE_ENDOFFILE    0x82
#define P_TYPE_ABORT        0x83
#define P_TYPE_REPORT       0x84

// NEW SIMPLIFIED GLOBALS

volatile BINT __usb_drvstatus __SYSTEM_GLOBAL__; // FLAGS TO INDICATE IF INITIALIZED, CONNECTED, SENDING/RECEIVING, ETC.

WORD __usb_fileid __SYSTEM_GLOBAL__;  // CURRENT FILEID
BINT __usb_fileid_seq __SYSTEM_GLOBAL__;  // SEQUENTIAL NUMBER TO MAKE FILEID UNIQUE
WORD __usb_offset __SYSTEM_GLOBAL__;  // CURRENT OFFSET WITHIN THE FILE
WORD __usb_crc32 __SYSTEM_GLOBAL__;   // CURRENT CRC32 OF DATA RECEIVED
BYTE __usb_ctlbuffer[RAWHID_RX_SIZE+1] __SYSTEM_GLOBAL__;  // BUFFER TO RECEIVE CONTROL PACKETS IN THE CONTROL CHANNEL
BYTE __usb_tmprxbuffer[RAWHID_RX_SIZE+1] __SYSTEM_GLOBAL__;  // TEMPORARY BUFFER TO RECEIVE DATA
BYTE __usb_ctlrxbuffer[RAWHID_RX_SIZE+1] __SYSTEM_GLOBAL__;  // TEMPORARY BUFFER TO RECEIVE CONTROL PACKETS
BYTE __usb_ctltxbuffer[RAWHID_TX_SIZE+1] __SYSTEM_GLOBAL__;  // TEMPORARY BUFFER TO TRANSMIT DATA

BYTEPTR __usb_rxbuffer __SYSTEM_GLOBAL__;              // LARGE BUFFER TO RECEIVE AT LEAST 3 FULL FRAGMENTS
WORD    __usb_rxoffset __SYSTEM_GLOBAL__;              // STARTING OFFSET OF THE DATA IN THE RX BUFFER
WORD    __usb_rxused __SYSTEM_GLOBAL__;                // NUMBER OF BYTES USED IN THE RX BUFFER
WORD    __usb_rxread __SYSTEM_GLOBAL__;                // NUMBER OF BYTES IN THE RX BUFFER ALREADY READ BY THE USER
WORD    __usb_rxtotalbytes __SYSTEM_GLOBAL__;          // TOTAL BYTES ON THE FILE, 0 MEANS DON'T KNOW YET

BYTEPTR __usb_txbuffer __SYSTEM_GLOBAL__;              // LARGE BUFFER POINTING TO AN ENTIRE FILE TO TRANSMIT
WORD    __usb_txoffset __SYSTEM_GLOBAL__;              // STARTING OFFSET OF THE DATA IN THE TX BUFFER
WORD    __usb_txtotalbytes __SYSTEM_GLOBAL__;              // TOTAL BYTES ON THE FILE, 0 MEANS DON'T KNOW YET
WORD    __usb_txused __SYSTEM_GLOBAL__;                // NUMBER OF BYTES USED IN THE TX BUFFER
BINT    __usb_txseq __SYSTEM_GLOBAL__;                // SEQUENTIAL NUMBER WITHIN A FRAGMENT OF DATA

BYTEPTR __usb_ctlbufptr __SYSTEM_GLOBAL__;             // POINTER TO BUFFER DURING CONTROL CHANNEL TRANSFERS
BINT __usb_ctlcount   __SYSTEM_GLOBAL__;                 // COUNT OF DATA DURING CONTROL CHANNEL TRANSFERS
BINT __usb_ctlpadding __SYSTEM_GLOBAL__;                 // COUNT OF DATA DURING CONTROL CHANNEL TRANSFERS




//  END OF NEW GLOBALS
// ********************************

// SOME INTERNAL FORWARD DECLARATIONS
void usb_sendcontrolpacket(int packet_type);



const WORD const __crctable[256] =
{
 0, 0x77073096, 0xEE0E612C, 0x990951BA,
 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
 0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
 0x9C0906A9, 0xEB0Eransmitdata363F, 0x72076785, 0x05005713,
 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
 0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

// CALCULATE THE STANDARD CRC32 OF A BLOCK OF DATA

WORD usb_crc32roll(WORD oldcrc,BYTEPTR data,BINT len)
{
    WORD crc=oldcrc^(-1);
    while(len--) crc=__crctable[(crc ^ *data++) & 0xFF] ^ (crc >> 8);
    return crc^(-1);
}




void usb_hwsetup()
{
    // MAKE SURE WE HAVE PCLK>20 MHz FOR USB COMMUNICATIONS TO WORK
    if(__cpu_getPCLK()<20000000) cpu_setspeed(HAL_USBCLOCK);

    *CLKCON&=~0x40;     // POWER DOWN USB HOST TO MAKE SURE HOST AND DEVICE AREN'T WORKING AT ONCE
    *CLKCON|=0x80;      // POWER UP USB DEVICE

    *UPLLCON=0x78023;   // 48 MHZ CLOCK
    *CLKSLOW&=~0x80;    // MAKE SURE UPLL IS ON

    *MISCCR&=~(USBSUSPND0|USBSUSPND1|USBPAD);   // SET DEVICE MODE, CHANGE TO NORMAL MODE

    // DEBUG: FOR NOW DON'T ALLOW SUSPEND
    *PWR_REG=0; // ALLOW THE DEVICE TO ENTER SUSPEND MODE

    *FUNC_ADDR_REG=0x80;    // RESET TO DEFAULT ADDRESS

    *INDEX_REG=0;       // SETUP ENDPOINT0
    *MAXP_REG=1;        // USE 8-BYTE PACKETS
    *MAXP_REG2=1;        // USE 8-BYTE PACKETS
    *EP0_CSR=0xc0;      // CLEAR ANYTHING PENDING

    *INDEX_REG=1;
    *MAXP_REG=8;        // USE 64-BYTE PACKETS ON EP1
    *MAXP_REG2=8;        // USE 64-BYTE PACKETS ON EP1
    *IN_CSR1_REG=0x48;  // CLR_DATA TOGGLE + FIFO_FLUSH
    *IN_CSR2_REG=0X20;  // CONFIGURE AS IN ENDPOINT
    *OUT_CSR1_REG=0x80; // SET CLR_DATA_TOGGLE
    *OUT_CSR2_REG=0;
    *INDEX_REG=2;
    *MAXP_REG=8;        // USE 64-BYTE PACKETS ON EP2
    *MAXP_REG2=8;        // USE 64-BYTE PACKETS ON EP2
    *IN_CSR1_REG=0x48;  // CLR_DATA TOGGLE + FIFO_FLUSH
    *IN_CSR2_REG=0;  // CONFIGURE AS OUT ENDPOINT
    *OUT_CSR1_REG=0x80; // SET CLR_DATA_TOGGLE
    *OUT_CSR2_REG=0;


    // SET WHICH INTERRUPTS WE WANT
    *USB_INT_EN_REG=0x7;    // ENABLE RESET, RESUME AND SUSPEND INTERRUPTS
    *EP_INT_EN_REG=0x7;     // ENABLE ONLY EP0, EP1 AND EP2 INTERRUPTS
    *USB_INT_REG=0x7;      // CLEAR ALL PENDING INTERRUPTS
    *EP_INT_REG=0x1f;       // CLEAR ALL PENDING INTERRUPTS

    // SETUP CABLE DISCONNECT DETECTION
    *HWREG(IO_REGS,0x50)=(*HWREG(IO_REGS,0x50)&(~0xc))|0x8;      // GPF1 SET TO EINT1
    *HWREG(IO_REGS,0x88)=(*HWREG(IO_REGS,0x88)&(~0x70))|0x20;    // CHANGE TO FALLING EDGE TRIGGERED


}

void usb_hwsuspend()
{

    *MISCCR|=(USBSUSPND0|USBSUSPND1);   // CHANGE TO SUSPEND MODE

    *CLKSLOW|=0x80;    // TURN OFF UPLL


}

void usb_hwresume()
{
    *UPLLCON=0x78023;   // 48 MHZ CLOCK
    *CLKSLOW&=~0x80;    // MAKE SURE UPLL IS ON

    *MISCCR&=~(USBSUSPND0|USBSUSPND1);   // CHANGE TO NORMAL MODE

}

void usb_irqservice();


void usb_irqdisconnect()
{
    // CALLED WHEN THE CABLE IS DISCONNECTED
    usb_shutdown();

}

void usb_irqconnect()
{
    // CALLED WHEN THE CABLE IS DISCONNECTED
    usb_init(0);

}




void usb_init(int force)
{

    if(__usb_drvstatus&USB_STATUS_INIT) {
        if(!force) return;
        usb_shutdown(); // FORCE A SHUTDOWN TO RESET THE PHY COMPLETELY
    }

    //__usb_intcount=0;

    __usb_drvstatus=USB_STATUS_INIT;

    usb_hwsetup();


    // SET INTERRUPT HANDLER
    __irq_mask(25);
    __irq_mask(1);

    __irq_addhook(25,&usb_irqservice);
    __irq_addhook(1,&usb_irqdisconnect);


    // ELIMINATE PREVIOUS DISCONNECT INTERRUPTS CAUSED BY NOISE
    *HWREG(INT_REGS,0)=2;       // REMOVE ANY PENDING INTERRUPTS FROM THIS SOURCE
    *HWREG(INT_REGS,0X10)=2;       // REMOVE ANY PENDING INTERRUPTS FROM THIS SOURCE


    __irq_unmask(25);
    __irq_unmask(1);


    if(CABLE_IS_CONNECTED) __usb_drvstatus|=USB_STATUS_CONNECTED;
    // TODO: SETUP COMMUNICATIONS BUFFERS

}


void usb_shutdown()
{

    if(!(__usb_drvstatus&USB_STATUS_INIT)) return;



    // MASK INTERRUPT AND REMOVE HANDLER
    __irq_mask(25);
    __irq_releasehook(25);

    // CLEANUP INTERRUPT SYSTEM
    *USB_INT_EN_REG=0x0;    // DISABLE INTERRUPTS
    *EP_INT_EN_REG=0x0;     // DISABLE INTERRUPTS
    *USB_INT_REG=0x7;      // CLEAR ALL PENDING INTERRUPTS
    *EP_INT_REG=0x1f;       // CLEAR ALL PENDING INTERRUPTS

    // STOP THE CLOCK
    *CLKSLOW|=0x80;    // SHUT DOWN UPLL

    *CLKCON&=~0xc0;     // POWER DOWN BOTH USB DEVICE AND HOST

    __usb_drvstatus=0;  // MARK UNCONFIGURED

    __irq_mask(1);


    // SETUP CABLE CONNECT INTERRUPT
    *HWREG(IO_REGS,0x50)=(*HWREG(IO_REGS,0x50)&(~0xc))|0x8;      // GPF1 SET TO EINT1
    *HWREG(IO_REGS,0x88)=(*HWREG(IO_REGS,0x88)&(~0x70))|0x40;    // CHANGE TO RAISING EDGE TRIGGERED


    __irq_addhook(1,&usb_irqconnect);

    *HWREG(INT_REGS,0)=2;       // REMOVE ANY PENDING INTERRUPTS FROM THIS SOURCE
    *HWREG(INT_REGS,0X10)=2;       // REMOVE ANY PENDING INTERRUPTS FROM THIS SOURCE


    __irq_unmask(1);

}



// TRANSMIT BYTES TO THE HOST IN EP0 ENDPOINT
// STARTS A NEW TRANSMISSION IF newtransmission IS TRUE, EVEN IF
// THERE ARE ZERO BYTES IN THE BUFFER
// FOR USE WITHIN ISR

void usb_ep0_transmit(int newtransmission)
{

    if(!(__usb_drvstatus&USB_STATUS_CONNECTED)) return;

    if(newtransmission || (__usb_drvstatus&USB_STATUS_EP0TX)) {

    *INDEX_REG=0;

    if( (*EP0_CSR)&EP0_IN_PKT_RDY) {
        // PREVIOUS PACKET IS STILL BEING SENT, DON'T PUSH IT
        __usb_drvstatus|=USB_STATUS_EP0TX;      // AND KEEP TRANSMITTING
        return;
    }

    int cnt=0;
    while(__usb_ctlcount && (cnt<EP0_FIFO_SIZE)) {
        *EP0_FIFO=(WORD) *__usb_ctlbufptr;
        ++__usb_ctlbufptr;
        ++cnt;
        --__usb_ctlcount;
    }

    if(__usb_ctlcount==0) {
        // SEND ANY PADDING
        while( (__usb_ctlpadding!=0) && (cnt<EP0_FIFO_SIZE)) {
        *EP0_FIFO=0;
        ++cnt;
        --__usb_ctlpadding;
        }
    }

    if((__usb_ctlcount==0)&&(__usb_ctlpadding==0))  {
        *EP0_CSR|=EP0_IN_PKT_RDY|EP0_DATA_END;  // SEND THE LAST PACKET

        //__usb_intdata[__usb_intcount++]=0xEEEE0000 | cnt;

        __usb_drvstatus&=~USB_STATUS_EP0TX;
    }
    else {
        *EP0_CSR|=EP0_IN_PKT_RDY;               // SEND PART OF THE BUFFER
        //__usb_intdata[__usb_intcount++]=0xAAAA0000 | cnt;
        __usb_drvstatus|=USB_STATUS_EP0TX;      // AND KEEP TRANSMITTING
    }
    }


}



// TRANSMIT BYTES TO THE HOST IN EP0 ENDPOINT
// STARTS A NEW TRANSMISSION IF newtransmission IS TRUE, EVEN IF
// THERE ARE ZERO BYTES IN THE BUFFER
// FOR USE WITHIN ISR

void usb_ep0_receive(int newtransmission)
{

    if(!(__usb_drvstatus&USB_STATUS_CONNECTED)) return;

    if(newtransmission || (__usb_drvstatus&USB_STATUS_EP0RX)) {

    *INDEX_REG=0;

    if(!((*EP0_CSR)&EP0_OUT_PKT_RDY)) {
        // PREVIOUS PACKET IS STILL BEING SENT, DON'T PUSH IT
        __usb_drvstatus|=USB_STATUS_EP0RX;      // AND KEEP TRANSMITTING
        return;
    }

    int cnt=0;
    while(__usb_ctlcount && (cnt<EP0_FIFO_SIZE)) {
        *__usb_ctlbufptr=(BYTE)*EP0_FIFO;
        ++__usb_ctlbufptr;
        --__usb_ctlcount;
        ++cnt;
    }

    if(__usb_ctlcount==0)  {
        *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;  // RECEIVED THE LAST PACKET
        __usb_drvstatus&=~USB_STATUS_EP0RX;
    }
    else {
        *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;               // RECIEVED PART OF THE BUFFER
        __usb_drvstatus|=USB_STATUS_EP0RX;      // AND KEEP RECEIVING MORE
    }
    }


}




void inline usb_checkpipe()
{
    if( (*EP0_CSR) & EP0_SETUP_END) {
        // SOMETHING HAPPENED, CLEAR THE CONDITION TO ALLOW RETRY
        *EP0_CSR|= EP0_SERVICED_SETUP_END;
        // CANCEL ALL ONGOING TRANSMISSIONS
        __usb_drvstatus&=~ (USB_STATUS_EP0RX|USB_STATUS_EP0TX);

    }
    if( (*EP0_CSR) & EP0_SENT_STALL) {
        // CLEAR ANY PREVIOUS STALL CONDITION
        *EP0_CSR=0;   // CLEAR SEND_STALL AND SENT_STALL SIGNALS
        // CANCEL ALL ONGOING TRANSMISSIONS
        __usb_drvstatus&=~ (USB_STATUS_EP0RX|USB_STATUS_EP0TX);
        // AND CONTINUE PROCESSING ANY OTHER INTERRUPTS
    }

}





void ep0_irqservice()
{
    *INDEX_REG=0;   // SELECT ENDPOINT 0


    usb_checkpipe();

    if(__usb_drvstatus&USB_STATUS_EP0TX) {

        usb_ep0_transmit(0);
        usb_checkpipe();

        return;
    }

    if( (*EP0_CSR) & EP0_OUT_PKT_RDY) {

        // PROCESS FIRST ANY ONGOING TRANSMISSIONS
        if(__usb_drvstatus&USB_STATUS_EP0RX) {
            usb_ep0_receive(0);
            usb_checkpipe();
            return;
        }


        // WE HAVE A PACKET
        BINT reqtype;
        BINT request;
        BINT value;
        BINT index;
        BINT length;

        // READ ALL 8 BYTES FROM THE FIFO

        reqtype=*EP0_FIFO;
        request=*EP0_FIFO;
        value=*EP0_FIFO;
        value|=(*EP0_FIFO)<<8;
        index=*EP0_FIFO;
        index|=(*EP0_FIFO)<<8;
        length=*EP0_FIFO;
        length|=(*EP0_FIFO)<<8;

        /*
        __usb_intdata[__usb_intcount++]= 0xffff0000 | (reqtype<<8) | (request);
        __usb_intdata[__usb_intcount++]=(value<<16)|index;
        __usb_intdata[__usb_intcount++]=(length<<16) | 0xffff;
        */

        if((reqtype&0x60)==0) {   // STANDARD REQUESTS


        // PROCESS THE REQUEST
        switch(request) {
        case GET_DESCRIPTOR:
        {
            // SEND THE REQUESTED RESPONSE
            int k;
            for(k=0;k<NUM_DESC_LIST;++k)
            {
                if((descriptor_list[k].wValue==value)&&(descriptor_list[k].wIndex==index))
                {
                    // FOUND THE REQUESTED DESCRIPTOR
                    __usb_ctlbufptr=(BYTEPTR) descriptor_list[k].addr;
                    if(length<descriptor_list[k].length) { __usb_ctlcount=length; __usb_ctlpadding=0; }
                    else { __usb_ctlcount=descriptor_list[k].length; __usb_ctlpadding=length-descriptor_list[k].length; }
                    *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
                    usb_ep0_transmit(1);    // SEND 0-DATA STATUS STAGE
                    usb_checkpipe();
                    return;
                }
            }

            // SPECIAL CASE - CALCULATOR SERIAL NUMBER STRING
            if((0x0303==value)&&(0x0409==index)) {
                // FOUND THE REQUESTED DESCRIPTOR
                __usb_ctlbufptr=__usb_ctlbuffer;
                __usb_ctlbuffer[0]=20+2;
                __usb_ctlbuffer[1]=3;

                // COPY THE SERIAL NUMBER - EXPAND ASCII TO UTF-16
                int n;
                BYTEPTR ptr=(BYTEPTR)SERIAL_NUMBER_ADDRESS;
                for(n=0;n<10;++n,++ptr) {
                    __usb_ctlbuffer[2+2*n]=*ptr;
                    __usb_ctlbuffer[3+2*n]=0;
                }


                if(length<__usb_ctlbuffer[0]) { __usb_ctlcount=length; __usb_ctlpadding=0; }
                else { __usb_ctlcount=__usb_ctlbuffer[0]; __usb_ctlpadding=length-__usb_ctlbuffer[0]; }
                *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
                usb_ep0_transmit(1);    // SEND 0-DATA STATUS STAGE
                usb_checkpipe();
                return;
            }



            // DON'T KNOW THE ANSWER TO THIS
            __usb_ctlcount=0;
            __usb_ctlpadding=length;    // SEND THE DATA AS REQUESTED, STALL AT THE END
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
            usb_ep0_transmit(1);    // SEND DATA STATUS STAGE
            usb_checkpipe();

            return;
        }
        case SET_ADDRESS:
        {
            __usb_ctlcount=0;
            __usb_ctlpadding=0;
            __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
            *FUNC_ADDR_REG=value|0x80;
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
            usb_checkpipe();
            return;
        }
        case SET_CONFIGURATION:
        {
            // OUR DEVICE HAS ONE SINGLE CONFIGURATION AND IS SETUP
            // ON WAKEUP, SO NOTHING TO DO HERE BUT ACKNOWLEDGE

            if(value) __usb_drvstatus|=USB_STATUS_CONFIGURED;
            else __usb_drvstatus&=~USB_STATUS_CONFIGURED;
            __usb_ctlcount=0;
            __usb_ctlpadding=0;
            __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
            usb_checkpipe();

            return;
        }
        case GET_CONFIGURATION:
        {
          BINT configvalue=(__usb_drvstatus&USB_STATUS_CONFIGURED)? 1:0;
          *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
          __usb_ctlcount=1;
          __usb_ctlpadding=0;
          __usb_ctlbufptr=__usb_ctlbuffer;
          __usb_ctlbuffer[0]=configvalue;
          usb_ep0_transmit(1);    // SEND DATA STATUS STAGE
          usb_checkpipe();

          return;
        }
        case GET_STATUS:
        {

            __usb_ctlbuffer[0]=__usb_ctlbuffer[1]=0;
            switch(reqtype) {
            case 0x80:  // DEVICE GET STATUS
                *(__usb_ctlbufptr)=(__usb_drvstatus&USB_STATUS_WAKEUPENABLED)? 2:0;
                break;
            case 0x82:  // ENDPONT GET STATUS
                *INDEX_REG=index&0x7;
                if((index&7)==0) {
                    *(__usb_ctlbufptr)=((*EP0_CSR)&EP0_SEND_STALL)? 1:0;
                }
                else {
                    *(__usb_ctlbufptr)|=((*OUT_CSR1_REG)&EPn_OUT_SEND_STALL)? 1:0;
                    *(__usb_ctlbufptr)|=((*IN_CSR1_REG)&EPn_IN_SEND_STALL)? 1:0;
                }
                break;
            }

            // FOR NOW SEND THE DATA
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
            __usb_ctlcount=2;
            __usb_ctlpadding=0;
            __usb_ctlbufptr=__usb_ctlbuffer;

            usb_ep0_transmit(1);    // SEND DATA STATUS STAGE
            usb_checkpipe();

            return;
        }
        case SET_FEATURE:
        {
            switch(reqtype)
            {
            case 0: // DEVICE FEATURES
                if(value==1) __usb_drvstatus|=USB_STATUS_WAKEUPENABLED;
                if(value==2) __usb_drvstatus|=USB_STATUS_TESTMODE;
            break;
            case 1: // INTERFACE FEATURES
                // NO INTERFACE FEATURES
                break;
            case 2: // ENDPOINT FEATURES
                if(value==0) {
                    // ENDPOINT_HALT FEATURE REQUEST

                    int endp=index&7;
                    *INDEX_REG=endp;
                    if(endp!=0) {   // DO NOT STALL THE CONTROL ENDPOINT
                        *OUT_CSR1_REG|=EPn_OUT_SEND_STALL;
                        *IN_CSR1_REG|=EPn_IN_SEND_STALL;
                    }
                }
             break;
            }

            __usb_ctlcount=0;
            __usb_ctlpadding=0;
            __usb_drvstatus&=~USB_STATUS_EP0RX|USB_STATUS_EP0TX;
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
            usb_checkpipe();

            return;
        }
        case CLEAR_FEATURE:
        {
            switch(reqtype)
            {
            case 0: // DEVICE FEATURES
                if(value==1) __usb_drvstatus&=~USB_STATUS_WAKEUPENABLED;
                if(value==2) __usb_drvstatus&=~USB_STATUS_TESTMODE;
            break;
            case 1: // INTERFACE FEATURES
                // NO INTERFACE FEATURES
                break;
            case 2: // ENDPOINT FEATURES
                if(value==0) {
                    // ENDPOINT_HALT FEATURE REQUEST

                    int endp=index&3;
                    *INDEX_REG=endp;
                    if(endp==1) {   // DO NOT STALL THE CONTROL ENDPOINT
                        *IN_CSR1_REG|=EPn_IN_FIFO_FLUSH|EPn_IN_CLR_DATA_TOGGLE;
                        *IN_CSR1_REG&=~(EPn_IN_SEND_STALL|EPn_IN_CLR_DATA_TOGGLE);
                    }
                    if(endp==2) {
                        *OUT_CSR1_REG|=EPn_OUT_FIFO_FLUSH|EPn_OUT_CLR_DATA_TOGGLE;
                        *OUT_CSR1_REG&=~(EPn_OUT_SEND_STALL|EPn_OUT_CLR_DATA_TOGGLE);
                    }
                }
             break;
            }

            __usb_ctlcount=0;
            __usb_ctlpadding=0;
            __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
            usb_checkpipe();

            return;
        }


        }
        // UNKNOWN STANDARD REQUEST??
        // DON'T KNOW THE ANSWER TO THIS BUT KEEP THE PIPES RUNNING
        if((reqtype&USB_DIRECTION)==USB_DEV_TO_HOST) {
            // SEND THE RIGHT AMOUNT OF ZEROS TO THE HOST
            __usb_ctlcount=0;
            __usb_ctlpadding=length;
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
            usb_ep0_transmit(1);
            usb_checkpipe();

            return;
        }

        // THIS IS AN INCOMING REQUEST WITH NO DATA STAGE

        // READ ANY EXTRA DATA TO KEEP THE FIFO CLEAN
        while(length>0) { __usb_ctlbuffer[0]=*EP0_FIFO; --length; }

        __usb_ctlcount=0;
        __usb_ctlpadding=0;
        __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
        *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
        usb_checkpipe();

        return;


        }

        if((reqtype&0x61)==0x21) {  // CLASS INTERFACE REQUESTS

        if(index==RAWHID_INTERFACE) {
            switch(request)
            {
            case HID_SET_REPORT:
                // GET DATA FROM HOST
                *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
                __usb_ctlcount=RAWHID_TX_SIZE;
                __usb_ctlpadding=0;
                __usb_ctlbufptr=__usb_ctlbuffer;    // FOR NOW, LET'S SEE WHAT TO DO WITH THIS LATER
                usb_ep0_receive(1);
                return;
            case HID_GET_REPORT:
                // SEND DATA TO HOST - SEND ALL ZEROS
                *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
                __usb_ctlcount=1;
                __usb_ctlpadding=RAWHID_TX_SIZE-1;
                __usb_ctlbuffer[0]=(__usb_drvstatus&USB_STATUS_RXDATA)? 1:0;
                __usb_ctlbufptr=__usb_ctlbuffer;    // SEND THE STATUS
                usb_ep0_transmit(1);
                usb_checkpipe();

                return;

           case HID_SET_IDLE:
                // SEND DATA TO HOST - SEND ALL ZEROS
                __usb_ctlcount=0;
                __usb_ctlpadding=0;
                __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
                *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
                usb_checkpipe();
                return;

            }


        }
        // UNKNOWN CLASS REQUEST??
        // DON'T KNOW THE ANSWER TO THIS
        if((reqtype&USB_DIRECTION)==USB_DEV_TO_HOST) {
            // SEND THE RIGHT AMOUNT OF ZEROS TO THE HOST
            __usb_ctlcount=0;
            __usb_ctlpadding=length;
            *EP0_CSR|=EP0_SEND_STALL|EP0_SERVICED_OUT_PKT_RDY;
            usb_ep0_transmit(1);
            usb_checkpipe();

            return;
        }
                // READ ANY EXTRA DATA TO KEEP THE FIFO CLEAN
        if(length>EP0_FIFO_SIZE) length=EP0_FIFO_SIZE;
        while(length>0) { __usb_ctlbuffer[0]=*EP0_FIFO; --length; }
        __usb_ctlcount=0;
        __usb_ctlpadding=0;
        __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
        *EP0_CSR|=EP0_SEND_STALL|EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
        usb_checkpipe();
        return;


        }

        // ADD OTHER REQUESTS HERE

        if((reqtype&USB_DIRECTION)==USB_DEV_TO_HOST) {
            // SEND THE RIGHT AMOUNT OF ZEROS TO THE HOST
            __usb_ctlcount=0;
            __usb_ctlpadding=length;
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
            usb_ep0_transmit(1);
            usb_checkpipe();

            return;
        }
                // READ ANY EXTRA DATA TO KEEP THE FIFO CLEAN
                if(length>EP0_FIFO_SIZE) length=EP0_FIFO_SIZE;
                while(length>0) { __usb_ctlbuffer[0]=*EP0_FIFO; --length; }
                __usb_ctlcount=0;
                __usb_ctlpadding=0;
                __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
                *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
                usb_checkpipe();

        return;

    }

}



// DRIVER - PACKET TRANSMISSION CALLED BY IRQ - NEVER CALLED BY USER
// SEND A CONTROL PACKET IF ONE AVAILABLE, OR ONE DATA PACKET IF AVAILABLE
void usb_ep1_transmit()
{

    if(!(__usb_drvstatus&USB_STATUS_CONNECTED)) return;

    *INDEX_REG=RAWHID_TX_ENDPOINT;

    if( (*IN_CSR1_REG)&EPn_IN_PKT_RDY) {
        // PREVIOUS PACKET IS STILL BEING SENT, DON'T PUSH IT
        // WAIT FOR THE NEXT INTERRUPT
        return;
    }

    if(__usb_drvstatus&USB_STATUS_TXCTL) {
        // WE HAVE A CONTROL PACKET READY TO GO IN THE CONTROL BUFFER

        int cnt;
        for(cnt=0;cnt<RAWHID_TX_SIZE;++cnt) *EP1_FIFO=(WORD) __usb_ctltxbuffer[cnt];

         *IN_CSR1_REG|=EPn_IN_PKT_RDY;  // SEND THE PACKET

        __usb_drvstatus&=~USB_STATUS_TXCTL;
        return;
    }

    if(__usb_drvstatus&USB_STATUS_TXDATA) {
        // WE HAVE A DATA PACKET TO SEND
        int bufoff=__usb_offset-__usb_txoffset;
        int bufbytes;
        int p_type;
        int eof=0;
        if(bufoff<0)
        {
         // THE CURRENT OFFSET IS BAD! ABORT THE FILE
            usb_sendcontrolpacket(P_TYPE_ABORT);
            __usb_fileid=0;
            __usb_offset=0;
            __usb_txused=0;
            __usb_txoffset=0;
            __usb_crc32=0;

            __usb_drvstatus&=~USB_STATUS_TXDATA;

            // SEND THE CONTROL PACKET NOW
            int cnt;
            for(cnt=0;cnt<RAWHID_TX_SIZE;++cnt) *EP1_FIFO=(WORD) __usb_ctltxbuffer[cnt];

             *IN_CSR1_REG|=EPn_IN_PKT_RDY;  // SEND THE PACKET

            __usb_drvstatus&=~USB_STATUS_TXCTL;
            return;
        }


        bufbytes=__usb_txused-bufoff;
        if(bufbytes>USB_DATASIZE) bufbytes=USB_DATASIZE;    // DON'T SEND MORE THAN ONE PACKET AT A TIME

        // CHECK IF THESE ARE THE LAST FEW BYTES OF THE FILE
        if((int)__usb_txtotalbytes-(int)__usb_offset == bufbytes) eof=1;

        p_type=__usb_txseq+1;
        if(eof) p_type|=0x40;
        if(p_type==32) p_type=0x40;


        // SEND A FULL PACKET

        *EP1_FIFO=p_type;
        *EP1_FIFO=bufbytes;
        *EP1_FIFO=__usb_fileid&0xff;
        *EP1_FIFO=(__usb_fileid>>8)&0xff;
        *EP1_FIFO=__usb_offset&0xff;
        *EP1_FIFO=(__usb_offset>>8)&0xff;
        *EP1_FIFO=(__usb_offset>>16)&0xff;
        *EP1_FIFO=(__usb_offset>>24)&0xff;

        int cnt;
        for(cnt=0;cnt<USB_DATASIZE;++cnt) {
            if(cnt>=bufbytes) *EP1_FIFO=0;
            else *EP1_FIFO=(WORD) __usb_ctltxbuffer[cnt];
        }

         *IN_CSR1_REG|=EPn_IN_PKT_RDY;  // SEND THE PACKET

        __usb_offset+=bufbytes;
        __usb_txseq=p_type&0x1f;
        if(eof) {
        usb_sendcontrolpacket(P_TYPE_ENDOFFILE);
        __usb_drvstatus&=~USB_STATUS_TXDATA;

        __usb_txused=0; // NO MORE DATA IN THE BUFFER
        __usb_txoffset=0;
        // LEAVE THE FILEID FOR FUTURE REPORTS

        }
        else {
            // AT EACH END OF FRAGMENT, SEND A CHECKPOINT BUT NOT AT END OF FILE
            if(p_type&0x40) usb_sendcontrolpacket(P_TYPE_CHECKPOINT);

            // IF WE CONSUMED ALL THE BUFFER, SIGNAL THAT WE ARE DONE
            if(__usb_txused+__usb_txoffset==__usb_offset) __usb_drvstatus&=~USB_STATUS_TXDATA;


        }


        return;
    }

    // NOTHING TO TRANSMIT

   // JUST REPLY WITH A ZERO DATA PACKET
   *IN_CSR1_REG|=EPn_IN_PKT_RDY;
}


// RECEIVE BYTES FROM THE HOST IN EP2 ENDPOINT
// FOR USE WITHIN ISR

void usb_ep2_receive()
{

    if(!(__usb_drvstatus&USB_STATUS_CONNECTED)) return;

    *INDEX_REG=RAWHID_RX_ENDPOINT;

    if(!((*OUT_CSR1_REG)&EPn_OUT_PKT_RDY)) {
        // THERE'S NO PACKETS AVAILABLE
        return;
    }

    int fifocnt=(*OUT_FIFO_CNT1_REG)+256*(*OUT_FIFO_CNT2_REG);
    int cnt=0;

    if(fifocnt<8) {
        // FLUSH THE FIFO, IGNORE THE MALFORMED PACKET
        BYTE sum=0;
        while(cnt<fifocnt) {
            sum+=(BYTE)*EP2_FIFO;
            ++cnt;
        }

        *OUT_CSR1_REG&=~EPn_OUT_PKT_RDY;  // RECEIVED THE PACKET
        return;
    }



    // READ PACKET TYPE
    int p_type=*EP2_FIFO;
    BYTEPTR rcvbuf;

    if(p_type&0x80) {
        // THIS IS A CONTROL PACKET
        // PUT IT IN THE CTL BUFFER AND NOTIFY THE USER
        __usb_drvstatus|=USB_STATUS_RXCTL;


        rcvbuf=__usb_ctlrxbuffer;
        cnt=1;
        rcvbuf[0]=p_type;
        ++rcvbuf;
        while(cnt<fifocnt) {
            *rcvbuf=(BYTE)*EP2_FIFO;
            ++cnt;
            ++rcvbuf;
        }

        *OUT_CSR1_REG&=~EPn_OUT_PKT_RDY;  // RECEIVED THE PACKET

        return;

    }

     // THIS IS A DATA PACKET
     // READ DATA DIRECTLY INTO THE BUFFER
    rcvbuf=__usb_tmprxbuffer;
    cnt=1;
    rcvbuf[0]=p_type;
    ++rcvbuf;
    while(cnt<8) {
        *rcvbuf=(BYTE)*EP2_FIFO;
        ++cnt;
        ++rcvbuf;
    }

    USB_PACKET *pptr=(USB_PACKET *)__usb_tmprxbuffer;

    // IS IT FROM THE SAME FILE?
    if(P_FILEID(pptr) != __usb_fileid) {
    // DIFFERENT FILE? IGNORE
        // FLUSH THE FIFO, IGNORE THE PACKET
        BYTE sum=0;
        while(cnt<fifocnt) {
            sum+=(BYTE)*EP2_FIFO;
            ++cnt;
        }

        *OUT_CSR1_REG&=~EPn_OUT_PKT_RDY;  // RECEIVED THE PACKET
        return;


    }

    // IS THE CORRECT OFFSET?
    if(pptr->p_offset!=__usb_offset) {
    //  WE MUST'VE MISSED SOMETHING
    __usb_drvstatus|=USB_STATUS_ERROR;
    // SEND A REPORT NOW IF POSSIBLE, OTHERWISE THE ERROR INFO WILL GO IN THE NEXT REPORT
    if(!(__usb_drvstatus&USB_STATUS_TXCTL))  usb_sendcontrolpacket(P_TYPE_REPORT);

    // FLUSH THE FIFO, IGNORE THE PACKET
    BYTE sum=0;
    while(cnt<fifocnt) {
        sum+=(BYTE)*EP2_FIFO;
        ++cnt;
    }

    *OUT_CSR1_REG&=~EPn_OUT_PKT_RDY;  // RECEIVED THE PACKET
    return;


    }

    // MAKE SOME ROOM IF WE CAN

    if(__usb_rxread==__usb_rxused) {
        // BUFFERS WERE READ BY THE USER COMPLETELY
        // START FROM A CLEAN BUFFER
        __usb_rxoffset+=__usb_rxused;
        __usb_rxused=0;
        __usb_rxread=0;
        __usb_drvstatus&=~USB_STATUS_HALT;
    }




    // DO WE HAVE ENOUGH ROOM AVAILABLE?
    if(__usb_rxused+pptr->p_dataused>2*LONG_BUFFER_SIZE) {
     // DATA WON'T FIT IN THE BUFFER DUE TO OVERFLOW, ISSUE AN ERROR AND REQUEST RESEND
        __usb_drvstatus|=USB_STATUS_ERROR;
        // SEND A REPORT NOW IF POSSIBLE, OTHERWISE THE ERROR INFO WILL GO IN THE NEXT REPORT
        if(!(__usb_drvstatus&USB_STATUS_TXCTL))  usb_sendcontrolpacket(P_TYPE_REPORT);

        // FLUSH THE FIFO, IGNORE THE PACKET
        BYTE sum=0;
        while(cnt<fifocnt) {
            sum+=(BYTE)*EP2_FIFO;
            ++cnt;
        }

        *OUT_CSR1_REG&=~EPn_OUT_PKT_RDY;  // RECEIVED THE PACKET
        return;

    }


    // WE HAVE NEW DATA, RECEIVE IT DIRECTLY AT THE BUFFER
    rcvbuf=__usb_rxbuffer+__usb_rxused;

    while((cnt<fifocnt)&&(cnt<pptr->p_dataused+8)) {
            *rcvbuf=(BYTE)*EP2_FIFO;
            ++cnt;
            ++rcvbuf;
        }

    // AND FLUSH ANY UNUSED BYTES
    BYTE sum;
    while(cnt<fifocnt) {
            sum+=(BYTE)*EP2_FIFO;
            ++cnt;
        }

    // UPDATE THE CRC
    __usb_crc32=usb_crc32roll(__usb_crc32,__usb_rxbuffer+__usb_rxused,pptr->p_dataused);

    // UPDATE THE BUFFERS
    __usb_rxused+=pptr->p_dataused;
    __usb_offset+=pptr->p_dataused;

    if(__usb_rxused>=LONG_BUFFER_SIZE) __usb_drvstatus|=USB_STATUS_HALT;  // REQUEST HALT IF BUFFER IS HALF-FULL

    __usb_drvstatus|=USB_STATUS_RXDATA; // AND SIGNAL THAT WE HAVE DATA AVAILABLE
}



// SENDING INTERRUPT ENDPOINT
inline void ep1_irqservice()
{
    usb_ep1_transmit();
}

// RECEIVING DATA ENDPOINT
void ep2_irqservice()
{
    // ONLY RESPOND ONCE EVERYTHING IS SETUP
    if(!(__usb_drvstatus&USB_STATUS_CONFIGURED)) return;

    *INDEX_REG=RAWHID_RX_ENDPOINT;

    // NOTHING TO RECEIVE, STALL
    if(*OUT_CSR1_REG&EPn_OUT_PKT_RDY) {
        // WE HAVE A PACKET, GO PROCESS IT
        usb_ep2_receive();
        return;
    }

    // GETTING INTERRUPTS WITHOUT PACKETS? SOMETHING IS WRONG, STALL

    if(*OUT_CSR1_REG&EPn_OUT_SENT_STALL) return;  // ALREADY DONE


    *OUT_CSR1_REG|=EPn_OUT_SEND_STALL;

    return;
}


// GENERAL INTERRUPT SERVICE ROUTINE - DISPATCH TO INDIVIDUAL ENDPOINT ROUTINES
void usb_irqservice()
{
    if(!(__usb_drvstatus&USB_STATUS_INIT)) return;
    __usb_drvstatus|=USB_STATUS_INSIDEIRQ;

    *INDEX_REG=0;

   if( !(*EP_INT_REG&7) && !(*USB_INT_REG&7) )
    {
        // WHAT ARE THESE INTERRUPTS FOR?
        if(__usb_drvstatus&(USB_STATUS_EP0TX|USB_STATUS_EP0RX)) ep0_irqservice();
         __usb_drvstatus&=~USB_STATUS_INSIDEIRQ;
        return;
    }

    if(*EP_INT_REG&1) {
        ep0_irqservice();
        *EP_INT_REG=1;
        __usb_drvstatus&=~USB_STATUS_INSIDEIRQ;
        return;
    }
    if(*EP_INT_REG&2) {
        ep1_irqservice();
        *EP_INT_REG=2;
        __usb_drvstatus&=~USB_STATUS_INSIDEIRQ;
        return;
    }
    if(*EP_INT_REG&4) {
        ep2_irqservice();
        *EP_INT_REG=4;
        __usb_drvstatus&=~USB_STATUS_INSIDEIRQ;
        return;
    }

    if(*USB_INT_REG&1) {
        // ENTER SUSPEND MODE
        usb_hwsuspend();
        *USB_INT_REG=1;
        __usb_drvstatus|=USB_STATUS_SUSPEND;
        __usb_drvstatus&=~USB_STATUS_INSIDEIRQ;
        return;
    }

    if(*USB_INT_REG&2) {
        // RESUME FROM SUSPEND MODE
        usb_hwresume();
        *USB_INT_REG=2;
        __usb_drvstatus&=~USB_STATUS_SUSPEND;
        __usb_drvstatus&=~USB_STATUS_INSIDEIRQ;
        return;
    }

    if(*USB_INT_REG&4) {
        // RESET RECEIVED
        if( (*PWR_REG)&USB_RESET) {
        __usb_drvstatus=USB_STATUS_INIT|USB_STATUS_CONNECTED;  // DECONFIGURE THE DEVICE
        usb_hwsetup();      // AND MAKE SURE THE HARDWARE IS IN KNOWN STATE
        }
        *USB_INT_REG=4;
        __usb_drvstatus&=~USB_STATUS_INSIDEIRQ;
        return;
    }


}





//********************************************
// NEW HARDWARE-INDEPENDENT CODE
//********************************************

int usb_isconnected()
{
    if(__usb_drvstatus&USB_STATUS_CONNECTED) return 1;
    return 0;
}

int usb_isconfigured()
{
    if(__usb_drvstatus&USB_STATUS_CONFIGURED) return 1;
    return 0;
}


// HIGH LEVEL FUNCTION TO SEE IF THERE'S ANY DATA FROM THE USB DRIVER
int usb_hasdata()
{
    if((__usb_drvstatus&USB_STATUS_RXDATA)&&(__usb_rxused>__usb_rxread)) return 1;
    return 0;
}



// HIGH LEVEL FUNCTION TO BLOCK UNTIL DATA ARRIVES
// RETURN 0 IF TIMEOUT
int usb_waitfordata()
{
    tmr_t start=tmr_ticks(),end;

    while(!(__usb_drvstatus&USB_STATUS_RXDATA)) {
        if((__usb_drvstatus&(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED))!=(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED)) return 0;
        if(__usb_drvstatus&USB_STATUS_RXCTL) usb_processreport();

        cpu_waitforinterrupt();

        end=tmr_ticks();
        if(tmr_ticks2ms(start,end)>USB_TIMEOUT_MS) {
            // MORE THAN 1 SECOND TO SEND 1 BLOCK? TIMEOUT - CLEANUP AND RETURN
            return 0;
        }
    }
    return 1;
}

// HIGH LEVEL FUNCTION TO ACCESS A BLOCK OF DATA
BYTEPTR usb_accessdata(int *datasize)
{
    if(!(__usb_drvstatus&USB_STATUS_RXDATA)) return 0;
    if(datasize) *datasize=__usb_rxused-__usb_rxread;
    return __usb_rxbuffer+__usb_rxread;
}

// HIGH LEVEL FUNCTION TO RELEASE A BLOCK OF DATA AND GET READY TO RECEIVE THE NEXT
void usb_releasedata(int datasize)
{
    if(!(__usb_drvstatus&USB_STATUS_RXDATA)) return;
    __usb_rxread+=datasize;
}





// TRANSMIT ONE CONTROL PACKET

void usb_sendcontrolpacket(int packet_type)
{
    while(__usb_drvstatus&USB_STATUS_TXCTL);    // THERE'S ANOTHER CONTROL PACKET, WAIT FOR IT TO BE SENT

    // NOW PREPARE THE NEXT CONTROL PACKET IN THE BUFFER
    USB_PACKET *p=(USB_PACKET *)__usb_ctltxbuffer;
    memsetb(__usb_ctltxbuffer,0,RAWHID_TX_SIZE+1);

    switch(packet_type)
    {
    case P_TYPE_GETSTATUS:
        p->p_type=P_TYPE_GETSTATUS;
        p->p_fileidLSB=__usb_fileid&0xff;
        p->p_fileidMSB=__usb_fileid>>8;
        break;
    case P_TYPE_CHECKPOINT:
        p->p_type=P_TYPE_CHECKPOINT;
        p->p_fileidLSB=__usb_fileid&0xff;
        p->p_fileidMSB=__usb_fileid>>8;
        p->p_offset=__usb_txoffset;
        p->p_data[0]=__usb_crc32&0xff;
        p->p_data[1]=(__usb_crc32>>8)&0xff;
        p->p_data[2]=(__usb_crc32>>16)&0xff;
        p->p_data[3]=(__usb_crc32>>24)&0xff;
        break;

    case P_TYPE_ENDOFFILE:
        p->p_type=P_TYPE_ENDOFFILE;
        p->p_fileidLSB=__usb_fileid&0xff;
        p->p_fileidMSB=__usb_fileid>>8;
        p->p_offset=__usb_txoffset;
        p->p_data[0]=__usb_crc32&0xff;
        p->p_data[1]=(__usb_crc32>>8)&0xff;
        p->p_data[2]=(__usb_crc32>>16)&0xff;
        p->p_data[3]=(__usb_crc32>>24)&0xff;
        break;

    case P_TYPE_ABORT:
        p->p_type=P_TYPE_ABORT;
        p->p_fileidLSB=__usb_fileid&0xff;
        p->p_fileidMSB=__usb_fileid>>8;
        break;

    case P_TYPE_REPORT:
        p->p_type=P_TYPE_REPORT;
        p->p_fileidLSB=__usb_fileid&0xff;
        p->p_fileidMSB=__usb_fileid>>8;
        p->p_offset=__usb_rxoffset;
        p->p_data[0]=(__usb_drvstatus&USB_STATUS_HALT)? 1:0;
        p->p_data[1]=(__usb_drvstatus&USB_STATUS_ERROR)? 1:0;
        break;

    default:
        return;     // INVALID PACKET TO SEND
    }
    __usb_drvstatus|=USB_STATUS_TXCTL;  // INDICATE THE DRIVER WE HAVE TO SEND A CONTROL PACKET

}

// CALLED WHEN A REPORT ARRIVED FROM THE OTHER SIDE, PROCESS DEPENDING ON WHAT WE ARE DOING
void usb_receivecontrolpacket()
{
    if(__usb_drvstatus&USB_STATUS_RXCTL) {

        USB_PACKET *ctl=(USB_PACKET *)__usb_ctlrxbuffer;

        switch(ctl->p_type)
        {
        case P_TYPE_GETSTATUS:
        {
        if(!__usb_fileid) {
         // START RECEIVING A NEW TRANSMISSION
            __usb_fileid=(WORD)ctl->p_fileidLSB+256*(WORD)ctl->p_fileidMSB;
            __usb_offset=0;
            __usb_crc32=0;
            __usb_rxoffset=0;
            __usb_rxused=0;                // NUMBER OF BYTES USED IN THE RX BUFFER
            __usb_rxread=0;                // NUMBER OF BYTES IN THE RX BUFFER ALREADY READ BY THE USER
            __usb_rxtotalbytes=0;          // DON'T KNOW THE TOTAL FILE SIZE YET
            __usb_drvstatus&=~(USB_STATUS_HALT|USB_STATUS_ERROR);

        }
        // SEND A REPORT, IF WE HAVE AN EXISTING TRANSMISSION, IT WILL REPLY WITH THE OLD FILEID
        usb_sendcontrolpacket(P_TYPE_REPORT);
        break;
        }
        case P_TYPE_CHECKPOINT:
        {

           if(__usb_fileid==(WORD)ctl->p_fileidLSB+256*(WORD)ctl->p_fileidMSB) {

               __usb_drvstatus&=~USB_STATUS_ERROR;    // REMOVE ERROR SIGNAL

               if(__usb_rxoffset+__usb_rxused!=ctl->p_offset) {
                   // SOMETHING WENT WRONG, WE DISAGREE ON THE FILE SIZE
                   __usb_drvstatus|=USB_STATUS_ERROR;    // SIGNAL TO RESEND FROM CURRENT OFFSET
               }
               WORD crc=ctl->p_data[0];
               crc|=((WORD)ctl->p_data[1])<<8;
               crc|=((WORD)ctl->p_data[2])<<16;
               crc|=((WORD)ctl->p_data[3])<<24;
               if(__usb_crc32!=crc) __usb_drvstatus|=USB_STATUS_ERROR;    // SIGNAL TO RESEND FROM CURRENT OFFSET

            // SEND THE REPORT
               usb_sendcontrolpacket(P_TYPE_REPORT);
           }
        break;
        }
        case P_TYPE_ENDOFFILE:
        {
            if(__usb_fileid==(WORD)ctl->p_fileidLSB+256*(WORD)ctl->p_fileidMSB) {
            // SAME AS FOR A CHECKPOINT, BUT SET TOTAL BYTE COUNT
                __usb_drvstatus&=~USB_STATUS_ERROR;    // REMOVE ERROR SIGNAL

                if(__usb_rxoffset+__usb_rxused!=ctl->p_offset) {
                    // SOMETHING WENT WRONG, WE DISAGREE ON THE FILE SIZE
                    __usb_drvstatus|=USB_STATUS_ERROR;    // SIGNAL TO RESEND FROM CURRENT OFFSET
                }
                WORD crc=ctl->p_data[0];
                crc|=((WORD)ctl->p_data[1])<<8;
                crc|=((WORD)ctl->p_data[2])<<16;
                crc|=((WORD)ctl->p_data[3])<<24;
                if(__usb_crc32!=crc) __usb_drvstatus|=USB_STATUS_ERROR;    // SIGNAL TO RESEND FROM CURRENT OFFSET

                // SET TOTAL BYTES TO INDICATE WE RECEIVED THE LAST OF IT
                if(!(__usb_drvstatus&USB_STATUS_ERROR)) __usb_rxtotalbytes=ctl->p_offset;


             // SEND A REPORT
                usb_sendcontrolpacket(P_TYPE_REPORT);
            }
            break;


        }
        case P_TYPE_ABORT:
        {
            if(__usb_fileid==(WORD)ctl->p_fileidLSB+256*(WORD)ctl->p_fileidMSB) {
            // REMOTE REQUESTED TO ABORT WHATEVER WE WERE DOING
            __usb_drvstatus&=~(USB_STATUS_TXDATA|USB_STATUS_TXCTL|USB_STATUS_RXDATA|USB_STATUS_HALT|USB_STATUS_ERROR|USB_STATUS_RXCTL);

            // ABORT ALL TRANSACTIONS
            __usb_fileid=0;
            __usb_offset=0;
            __usb_crc32=0;
            __usb_rxread=0;
            __usb_rxused=0;
            __usb_rxoffset=0;
            __usb_rxtotalbytes=0;
            __usb_txoffset=0;
            __usb_txused=0;
            __usb_txtotalbytes=0;

            // DO NOT REPLY TO AN ABORT CONDITION

            break;

        }

        case P_TYPE_REPORT:
        {
         if(__usb_fileid==(WORD)ctl->p_fileidLSB+256*(WORD)ctl->p_fileidMSB) {

         // UPDATE FLAGS WITH THE STATUS OF THE REMOTE
         if(ctl->p_data[0]) __usb_drvstatus|=USB_STATUS_HALT;
         else __usb_drvstatus&=~USB_STATUS_HALT;
         if(ctl->p_data[0]) {
             // SIGNAL THE ERROR AND LEAVE THE REQUESTED OFFSET AT rxoffset
             __usb_drvstatus|=USB_STATUS_ERROR;
             __usb_rxoffset=ctl->p_offset;
         }
         else __usb_drvstatus&=~USB_STATUS_ERROR;

         break;

        }




    }

}




// WAIT FOR A CONTROL PACKET TO COME BACK FROM THE REMOTE
int usb_waitforreport()
{
    tmr_t start,end;

    // WAIT FOR ALL PREVIOUS DATA TO BE SENT COMPLETELY
    start=tmr_ticks();
    while(!(__usb_drvstatus&USB_STATUS_RXCTL)) {
        end=tmr_ticks();
        if(tmr_ticks2ms(start,end)>USB_TIMEOUT_MS) {
        return 0;
        }
        }
    return 1;
}


// RETRIEVE LAST CONTROL PACKET WE RECEIVED
USB_PACKET *usb_getreport()
{
    if(__usb_drvstatus&USB_STATUS_RXCTL)  return __usb_ctlrxbuffer;
}

// RELEASE THE CONTROL PACKET
void usb_releasereport()
{
    __usb_drvstatus&=~USB_STATUS_RXCTL;
}


// START TRANSMISSION OF A FILE
// file_type = 'O','B','W', OR 'D', SEE SPECS
int usb_txfileopen(int file_type)
{
    tmr_t start,end;

    // WAIT FOR ALL PREVIOUS DATA TO BE SENT COMPLETELY
    start=tmr_ticks();
    while(__usb_drvstatus&USB_STATUS_TXDATA) {
        end=tmr_ticks();
        if(tmr_ticks2ms(start,end)>USB_TIMEOUT_MS) {
        return 0;
        }
        }


    // CREATE A NEW FILEID
    __usb_fileid=(file_type<<8)&0xff00;
    __usb_fileid+=__usb_fileid_seq;
    ++__usb_fileid_seq;
    __usb_txbuffer=0;   // NULL BUFFER UNTIL USER PROVIDES DATA
    __usb_txused=0;
    __usb_txoffset=0;

    __usb_txoffset=0;   // RESET OFFSET
    __usb_crc32=0;      // RESET CRC32

    // INDICATE WE ARE STARTING A TRANSMISSION, WAIT FOR REMOTE TO BE AVAILABLE
    int busy,error;

    start=tmr_ticks();
    do {
        end=tmr_ticks();
        if(tmr_ticks2ms(start,end)>USB_TIMEOUT_MS) return 0;    // FAIL IF TIMEOUT

        usb_sendcontrolpacket(P_TYPE_GETSTATUS);
        do {
            if(!usb_waitforreport()) return 0;                  // FAIL IF TIMEOUT
            USB_PACKET *ptr=usb_getreport();

            if(P_FILEID(ptr)==__usb_fileid) {
                // THE REMOTE WANTS TO ABORT THIS FILE ALREADY?
                if(ptr->p_type==P_TYPE_ABORT) return 0;         // FAIL DUE TO ABORT

                if(ptr->p_type==P_TYPE_REPORT) {
                    busy=ptr->p_data[0];
                    error=ptr->p_data[1];
                    usb_releasereport();
                    break;
                }
            }
            usb_releasereport();
        } while(1);

    } while(busy||error);

    // WE ARE READY TO START!

    return 1;

}

// WRITE BYTES TO A FILE BEING SENT

int usb_filewrite(BYTEPTR data,int nbytes)
{
    // WAIT FOREVER UNTIL WE ARE DONE WRITING, BUT RETURN IF WE GET A REPORT
    while(__usb_drvstatus&USB_STATUS_TXDATA) {
        USB_PACKET *report=usb_getreport();
        if(report) return 0;
    }

    __usb_txbuffer=data;
    __usb_txused=nbytes;
    __usb_txoffset=__usb_offset;
    __usb_drvstatus|=USB_STATUS_TXDATA; // SIGNAL THAT WE HAVE A NEW BUFFER READY
    return 1;
}

int usb_txfileclose()
{
    if(!(__usb_drvstatus&USB_STATUS_TXDATA)) {
        // ZERO-DATA PACKET WILL BE SENT TO INDICATE END-OF-FILE
        __usb_txtotalbytes=__usb_offset;
        __usb_txbuffer=__usb_tmprxbuffer;
        __usb_txused=0;
        __usb_txoffset=__usb_offset;
        __usb_drvstatus|=USB_STATUS_TXDATA; // SIGNAL THAT WE HAVE A NEW BUFFER READY
    }
    else {
     // THERE'S DATA IN TRANSMISSION, JUST SET THE TOTAL SIZE OF THE FILE
     __usb_txtotalbytes=__usb_offset+__usb_txused;
    }
    return 1;
}

// START RECEIVING A FILE, WHETHER IT WAS COMPLETELY RECEIVED YET OR NOT
// RETURNS THE FILEID OR 0 IF FAILS
int usb_rxfileopen()
{
if(!usb_hasdata()) return 0;
return __usb_fileid;
}

// RETURN HOW MANY BYTES ARE READY TO BE READ
int usb_rxbytesready()
{
return __usb_rxused-__usb_rxread;
}

// RETRIEVE BYTES THAT WERE ALREADY RECEIVED
int usb_fileread(BYTEPTR dest,int nbytes)
{
    // QUICK COPY IF WE ALREADY HAVE ENOUGH BYTES
    if(nbytes<=__usb_rxused-__usb_rxread) {
      memmoveb(dest,__usb_rxbuffer+__usb_rxread,nbytes);
      __usb_rxread+=nbytes;
      return 1;
    }

    // WAIT FOR ENOUGH BYTES TO BECOME AVAILABLE
    int busy,error;

    tmr_t start,end;

    start=tmr_ticks();
    do {
        end=tmr_ticks();
        if(tmr_ticks2ms(start,end)>USB_TIMEOUT_MS) return 0;    // FAIL IF TIMEOUT

        usb_waitfordata();
            if(!usb_waitforreport()) return 0;                  // FAIL IF TIMEOUT
            USB_PACKET *ptr=usb_getreport();

            if(P_FILEID(ptr)==__usb_fileid) {
                // THE REMOTE WANTS TO ABORT THIS FILE ALREADY?
                if(ptr->p_type==P_TYPE_ABORT) return 0;         // FAIL DUE TO ABORT

                if(ptr->p_type==P_TYPE_REPORT) {
                    busy=ptr->p_data[0];
                    error=ptr->p_data[1];
                    usb_releasereport();
                    break;
                }
            }
            usb_releasereport();
        } while(1);

    } while(busy||error);

}

// CLOSE THE FILE RELEASE ANY PENDING DATA
int usb_rxclose()
{

}
