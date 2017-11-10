/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include <newrpl.h>
#include <ui.h>

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
#define EPn_IN_SEND_STALL   0x10
#define EPn_OUT_SEND_STALL  0x20

#define EP0_FIFO_SIZE    8

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
#define RAWHID_RX_INTERVAL	8	// max # of ms between receive packets


/**************************************************************************
 *
 *  Endpoint Buffer Configuration
 *
 **************************************************************************/

#define ENDPOINT0_SIZE		8
#define RAWHID_INTERFACE	0
#define RAWHID_TX_ENDPOINT	1
#define RAWHID_RX_ENDPOINT	2

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
    0x00, 0x02,				// bcdUSB
    0,					// bDeviceClass
    0,					// bDeviceSubClass
    0,					// bDeviceProtocol
    ENDPOINT0_SIZE,				// bMaxPacketSize0
    LSB(VENDOR_ID), MSB(VENDOR_ID),		// idVendor
    LSB(PRODUCT_ID), MSB(PRODUCT_ID),	// idProduct
    0x00, 0x01,				// bcdDevice
    1,					// iManufacturer
    2,					// iProduct
    0,					// iSerialNumber
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

#define NUM_DESC_LIST (sizeof(descriptor_list)/sizeof(struct descriptor_list_struct))




//********************************************************************
// END OF DEFINITIONS BORROWED FROM Teensy rawHID project.
// EVERYTHING BELOW WAS WRITTEN FROM SCRATCH BY THE newRPL Team

// INITIALIZE USB SUBSYSTEM, POWER, PINS
#define USB_STATUS_INIT                 1
#define USB_STATUS_EP0TX                2
#define USB_STATUS_EP0RX                4
#define USB_STATUS_EP1TX                8
#define USB_STATUS_EP1RX                16
#define USB_STATUS_EP2TX                32
#define USB_STATUS_EP2RX                64
#define USB_STATUS_CONNECTED            128
#define USB_STATUS_CONFIGURED           256
#define USB_STATUS_SUSPEND              512
#define USB_STATUS_WAKEUPENABLED        1024
#define USB_STATUS_TESTMODE             2048


// GLOBAL VARIABLES OF THE USB SUBSYSTEM
BINT __usb_drvstatus __SYSTEM_GLOBAL__; // FLAGS TO INDICATE IF INITIALIZED, CONNECTED, SENDING/RECEIVING, ETC.
BYTE *__usb_bufptr[3] __SYSTEM_GLOBAL__;   // POINTERS TO BUFFERS FOR EACH ENDPOINT (0/1/2)
BINT __usb_count[3]   __SYSTEM_GLOBAL__;   // REMAINING BYTES TO TRANSFER ON EACH ENDPOINT (0/1/2)
BYTE __usb_tmpbuffer[RAWHID_TX_SIZE] __SYSTEM_GLOBAL__;  // TEMPORARY BUFFER FOR NON-BLOCKING CONTROL TRANSFERS


void usb_hwsetup()
{
    *CLKCON&=~0x40;     // POWER DOWN USB HOST TO MAKE SURE HOST AND DEVICE AREN'T WORKING AT ONCE
    *CLKCON|=0x80;      // POWER UP USB DEVICE

    *UPLLCON=0x78023;   // 48 MHZ CLOCK
    *CLKSLOW&=~0x80;    // MAKE SURE UPLL IS ON

    *MISCCR&=~(USBSUSPND0|USBSUSPND1|USBPAD);   // SET DEVICE MODE, CHANGE TO NORMAL MODE

    // DEBUG: FOR NOW DON'T ALLOW SUSPEND
    *PWR_REG=0; // ALLOW THE DEVICE TO ENTER SUSPEND MODE

    // SET WHICH INTERRUPTS WE WANT
    *USB_INT_EN_REG=0x7;    // ENABLE RESET, RESUME AND SUSPEND INTERRUPTS
    *EP_INT_EN_REG=0x7;     // ENABLE ONLY EP0, EP1 AND EP2 INTERRUPTS
    *USB_INT_REG=0x7;      // CLEAR ALL PENDING INTERRUPTS
    *EP_INT_REG=0x1f;       // CLEAR ALL PENDING INTERRUPTS

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

void usb_init()
{

    if(__usb_drvstatus&USB_STATUS_INIT) return;

    __usb_drvstatus=USB_STATUS_INIT;

    usb_hwsetup();

    // SET INTERRUPT HANDLER
    __irq_mask(25);

    __irq_addhook(25,&usb_irqservice);

    __irq_unmask(25);

    // TODO: SETUP COMMUNICATIONS BUFFERS

}

void usb_shutdown()
{

    if(!__usb_drvstatus&USB_STATUS_INIT) return;
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

    // TODO: RELEASE COMMUNICATIONS BUFFERS

}


// TRANSMIT BYTES TO THE HOST IN EP0 ENDPOINT
// STARTS A NEW TRANSMISSION IF newtransmission IS TRUE, EVEN IF
// THERE ARE ZERO BYTES IN THE BUFFER
// FOR USE WITHIN ISR

void usb_ep0_transmit(int newtransmission)
{

    if(!__usb_drvstatus&USB_STATUS_CONNECTED) return;

    if(newtransmission || (__usb_drvstatus&USB_STATUS_EP0TX)) {

    *INDEX_REG=0;

    if(*EP0_CSR&EP0_IN_PKT_RDY) {
        // PREVIOUS PACKET IS STILL BEING SENT, DON'T PUSH IT
        __usb_drvstatus|=USB_STATUS_EP0TX;      // AND KEEP TRANSMITTING
        return;
    }

    int cnt=0;
    while(__usb_count[0] && (cnt<EP0_FIFO_SIZE)) {
        *EP0_FIFO=(WORD) *__usb_bufptr[0];
        ++__usb_bufptr[0];
        ++cnt;
        --__usb_count[0];
    }

    if(__usb_count[0]==0)  {
        *EP0_CSR|=EP0_IN_PKT_RDY|EP0_DATA_END;  // SEND THE LAST PACKET
        __usb_drvstatus&=~USB_STATUS_EP0TX;
    }
    else {
        *EP0_CSR|=EP0_IN_PKT_RDY;               // SEND PART OF THE BUFFER
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

    if(!__usb_drvstatus&USB_STATUS_CONNECTED) return;

    if(newtransmission || (__usb_drvstatus&USB_STATUS_EP0RX)) {

    *INDEX_REG=0;

    if(*EP0_CSR&EP0_IN_PKT_RDY) {
        // PREVIOUS PACKET IS STILL BEING SENT, DON'T PUSH IT
        __usb_drvstatus|=USB_STATUS_EP0TX;      // AND KEEP TRANSMITTING
        return;
    }

    int cnt=0;
    while(__usb_count[0] && (cnt<EP0_FIFO_SIZE)) {
        *__usb_bufptr[0]=(BYTE)*EP0_FIFO;
        ++__usb_bufptr[0];
        --__usb_count[0];
        ++cnt;
    }

    if(__usb_count[0]==0)  {
        *EP0_CSR|=EP0_OUT_PKT_RDY|EP0_DATA_END;  // RECEIVED THE LAST PACKET
        __usb_drvstatus&=~USB_STATUS_EP0RX;
    }
    else {
        *EP0_CSR|=EP0_OUT_PKT_RDY;               // RECIEVED PART OF THE BUFFER
        __usb_drvstatus|=USB_STATUS_EP0RX;      // AND KEEP RECEIVING MORE
    }
    }


}












void ep0_irqservice()
{
    *INDEX_REG=0;   // SELECT ENDPOINT 0

    if( (*EP0_CSR) & EP0_SETUP_END) {
        // SOMETHING HAPPENED, CLEAR THE CONDITION TO ALLOW RETRY
        __usb_drvstatus|=8192;
        *EP0_CSR|=EP0_SERVICED_SETUP_END;
        if( (*EP0_CSR) & EP0_OUT_PKT_RDY) {
        *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
        __usb_count[0]=0;
        usb_ep0_transmit(1);    // SEND 0-DATA STATUS STAGE
        }
        return;
    }


    if( (*EP0_CSR) & EP0_OUT_PKT_RDY) {

        // PROCESS FIRST ANY ONGOING TRANSMISSIONS
        if(__usb_drvstatus&USB_STATUS_EP0RX) {
            usb_ep0_receive(0);
            return;
        }


        // WE HAVE A PACKET
        BINT reqtype;
        BINT request;
        BINT value;
        BINT index;
        BINT length;

        // READ ALL 8 BYTES FROM THE FIFO

        __usb_tmpbuffer[0]=reqtype=*EP0_FIFO;
        __usb_tmpbuffer[1]=request=*EP0_FIFO;
        __usb_tmpbuffer[2]=value=*EP0_FIFO;
        __usb_tmpbuffer[3]=value|=(*EP0_FIFO)<<8;
        __usb_tmpbuffer[4]=index=*EP0_FIFO;
        __usb_tmpbuffer[5]=index|=(*EP0_FIFO)<<8;
        __usb_tmpbuffer[6]=length=*EP0_FIFO;
        __usb_tmpbuffer[7]=length|=(*EP0_FIFO)<<8;


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
                    __usb_drvstatus|=16384; // FOR DEBUG ONLY

                    // FOUND THE REQUESTED DESCRIPTOR
                    __usb_bufptr[0]=(BYTEPTR) descriptor_list[k].addr;
                    if(length<descriptor_list[k].length) __usb_count[0]=length;
                    else __usb_count[0]=descriptor_list[k].length;
                    *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
                    usb_ep0_transmit(1);    // SEND 0-DATA STATUS STAGE
                    return;
                }
            }
            // DON'T KNOW THE ANSWER TO THIS
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_SEND_STALL;
            return;
        }
        case SET_ADDRESS:
        {
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
            __usb_count[0]=0;
            *FUNC_ADDR_REG=value|0x80;
            usb_ep0_transmit(1);    // SEND 0-DATA STATUS STAGE
            return;
        }
        case SET_CONFIGURATION:
        {
            // OUR DEVICE HAS ONE SINGLE CONFIGURATION AND IS SETUP
            // ON WAKEUP, SO NOTHING TO DO HERE BUT ACKNOWLEDGE

            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
            __usb_count[0]=0;
            if(value) __usb_drvstatus|=USB_STATUS_CONFIGURED;
            else __usb_drvstatus&=~USB_STATUS_CONFIGURED;
            usb_ep0_transmit(1);    // SEND 0-DATA STATUS STAGE
            return;
        }
        case GET_CONFIGURATION:
        {
          BINT configvalue=(__usb_drvstatus&USB_STATUS_CONFIGURED)? 1:0;
          *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
          __usb_count[0]=1;
          __usb_bufptr[0]=__usb_tmpbuffer;
          __usb_tmpbuffer[0]=configvalue;
          usb_ep0_transmit(1);    // SEND DATA STATUS STAGE
          return;
        }
        case GET_STATUS:
        {
            __usb_drvstatus|=1024;

            __usb_tmpbuffer[0]=__usb_tmpbuffer[1]=0;
            switch(reqtype) {
            case 0x80:  // DEVICE GET STATUS
                *(__usb_bufptr[0])=(__usb_drvstatus&USB_STATUS_WAKEUPENABLED)? 2:0;
                break;
            case 0x82:  // ENDPONT GET STATUS
                *INDEX_REG=index&0x7;
                if((index&7)==0) {
                    *(__usb_bufptr[0])=((*EP0_CSR)&EP0_SEND_STALL)? 1:0;
                }
                else {
                    *(__usb_bufptr[0])|=((*OUT_CSR1_REG)&EPn_OUT_SEND_STALL)? 1:0;
                    *(__usb_bufptr[0])|=((*IN_CSR1_REG)&EPn_IN_SEND_STALL)? 1:0;
                }
                break;
            }

            // FOR NOW SEND THE DATA
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
            __usb_count[0]=2;
            __usb_bufptr[0]=__usb_tmpbuffer;

            usb_ep0_transmit(1);    // SEND DATA STATUS STAGE
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

            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
            __usb_count[0]=0;
            usb_ep0_transmit(1);    // SEND 0-DATA STATUS STAGE
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
                    if(endp!=0) {   // DO NOT STALL THE CONTROL ENDPOINT
                        *OUT_CSR1_REG&=~EPn_OUT_SEND_STALL;
                        *IN_CSR1_REG&=~EPn_IN_SEND_STALL;
                    }
                }
             break;
            }

            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
            __usb_count[0]=0;
            usb_ep0_transmit(1);    // SEND 0-DATA STATUS STAGE
            return;
        }


        }
        // UNKNOWN STANDARD REQUEST??
        // DON'T KNOW THE ANSWER TO THIS
        *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_SEND_STALL;
        return;


        }

        if((reqtype&0x60)==0x20) {  // CLASS REQUESTS

        if(index==RAWHID_INTERFACE) {
            switch(request)
            {
            case HID_SET_REPORT:
                // GET DATA FROM HOST
                *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
                __usb_count[0]=RAWHID_TX_SIZE;
                __usb_bufptr[0]=__usb_tmpbuffer;    // FOR NOW, LET'S SEE WHAT TO DO WITH THIS LATER
                usb_ep0_receive(1);
                return;
            case HID_GET_REPORT:
                // SEND DATA TO HOST
                *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
                __usb_count[0]=RAWHID_TX_SIZE;
                __usb_bufptr[0]=__usb_tmpbuffer;    // FOR NOW SEND WHATEVER WAS STORED THERE
                usb_ep0_transmit(1);
                return;




            }


        }
        // UNKNOWN CLASS REQUEST??
        // DON'T KNOW THE ANSWER TO THIS
        *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_SEND_STALL;
        return;

        }

        // ADD OTHER REQUESTS HERE

        // ENOUGH REQUESTS FOR NOW
        *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_SEND_STALL;
        return;

    }

    if( !((*EP0_CSR) & EP0_IN_PKT_RDY)) {
        // SEND PACKAGES IF WE HAVE ANY ONGOING TRANSMISSION

        if(__usb_drvstatus&USB_STATUS_EP0TX) {
            usb_ep0_transmit(0);
            return;
        }

        // THERE'S NO TRANSMISSIONS, DON'T SENT ANYTHING



    }
}

void ep1_irqservice()
{

}

void ep2_irqservice()
{

}


// GENERAL INTERRUPT SERVICE ROUTINE - DISPATCH TO INDIVIDUAL ENDPOINT ROUTINES
void usb_irqservice()
{

    if(!__usb_drvstatus&USB_STATUS_INIT) return;

    if(*EP_INT_REG&1) {
        ep0_irqservice();
        *EP_INT_REG=1;
        return;
    }
    if(*EP_INT_REG&2) {
        ep1_irqservice();
        *EP_INT_REG=2;
        return;
    }
    if(*EP_INT_REG&4) {
        ep2_irqservice();
        *EP_INT_REG=4;
        return;
    }

    if(*USB_INT_REG&1) {
        // ENTER SUSPEND MODE
        usb_hwsuspend();
        *USB_INT_REG=1;
        __usb_drvstatus|=USB_STATUS_SUSPEND;
        return;
    }

    if(*USB_INT_REG&2) {
        // RESUME FROM SUSPEND MODE
        usb_hwresume();
        *USB_INT_REG=2;
        __usb_drvstatus&=~USB_STATUS_SUSPEND;
        return;
    }

    if(*USB_INT_REG&4) {
        // RESET RECEIVED
        __usb_drvstatus=USB_STATUS_INIT|USB_STATUS_CONNECTED;  // DECONFIGURE THE DEVICE
        usb_hwsetup();      // AND MAKE SURE THE HARDWARE IS IN KNOWN STATE
        return;
    }


}


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
