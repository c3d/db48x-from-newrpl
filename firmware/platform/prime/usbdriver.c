/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include <newrpl.h>
#include <ui.h>

// OTHER EXTERNAL FUNCTIONS NEEDED
extern int cpu_getPCLK();

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

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

// Descriptors are the data that your computer reads when it auto-detects
// this USB device (called "enumeration" in USB lingo).  The most commonly
// changed items are editable at the top of this file.  Changing things
// in here should only be done by those who've read chapter 9 of the USB
// spec and relevant portions of any USB class specifications!

const BYTE const device_descriptor[] = {
    18, // bLength
    1,  // bDescriptorType
    0x00, 0x02, // bcdUSB
    0,  // bDeviceClass
    0,  // bDeviceSubClass
    0,  // bDeviceProtocol
    EP0_FIFO_SIZE,     // bMaxPacketSize0
    LSB(VENDOR_ID), MSB(VENDOR_ID),     // idVendor
    LSB(PRODUCT_ID), MSB(PRODUCT_ID),   // idProduct
    0x00, 0x01, // bcdDevice
    1,  // iManufacturer
    2,  // iProduct
    3,  // iSerialNumber
    1   // bNumConfigurations
};

const BYTE const rawhid_hid_report_desc[] = {
    0x06, LSB(RAWHID_USAGE_PAGE), MSB(RAWHID_USAGE_PAGE),
    0x0A, LSB(RAWHID_USAGE), MSB(RAWHID_USAGE),
    0xA1, 0x01, // Collection 0x01
    0x75, 0x08, // report size = 8 bits
    0x15, 0x00, // logical minimum = 0
    0x26, 0xFF, 0x00,   // logical maximum = 255
    0x95, RAWHID_TX_SIZE,       // report count
    0x09, 0x01, // usage
    0x81, 0x02, // Input (array)
    0x95, RAWHID_RX_SIZE,       // report count
    0x09, 0x02, // usage
    0x91, 0x02, // Output (array)
    0xC0        // end collection
};

#define CONFIG1_DESC_SIZE        (9+9+9+7+7)
#define RAWHID_HID_DESC_OFFSET   (9+9)
const BYTE const config1_descriptor[CONFIG1_DESC_SIZE] = {
    // configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
    9,  // bLength;
    2,  // bDescriptorType;
    LSB(CONFIG1_DESC_SIZE),     // wTotalLength
    MSB(CONFIG1_DESC_SIZE),
    1,  // bNumInterfaces
    1,  // bConfigurationValue
    0,  // iConfiguration
    0xC0,       // bmAttributes
    50, // bMaxPower

    // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
    9,  // bLength
    4,  // bDescriptorType
    RAWHID_INTERFACE,   // bInterfaceNumber
    0,  // bAlternateSetting
    2,  // bNumEndpoints
    0x03,       // bInterfaceClass (0x03 = HID)
    0x00,       // bInterfaceSubClass (0x01 = Boot)
    0x00,       // bInterfaceProtocol (0x01 = Keyboard)
    0,  // iInterface
    // HID interface descriptor, HID 1.11 spec, section 6.2.1
    9,  // bLength
    0x21,       // bDescriptorType
    0x11, 0x01, // bcdHID
    0,  // bCountryCode
    1,  // bNumDescriptors
    0x22,       // bDescriptorType
    sizeof(rawhid_hid_report_desc),     // wDescriptorLength
    0,
    // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
    7,  // bLength
    5,  // bDescriptorType
    RAWHID_TX_ENDPOINT | 0x80,  // bEndpointAddress
    0x03,       // bmAttributes (0x03=intr)
    RAWHID_TX_SIZE, 0,  // wMaxPacketSize
    RAWHID_TX_INTERVAL, // bInterval
    // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
    7,  // bLength
    5,  // bDescriptorType
    RAWHID_RX_ENDPOINT, // bEndpointAddress
    0x03,       // bmAttributes (0x03=intr)
    RAWHID_RX_SIZE, 0,  // wMaxPacketSize
    RAWHID_RX_INTERVAL  // bInterval
};

// If you're desperate for a little extra code memory, these strings
// can be completely removed if iManufacturer, iProduct, iSerialNumber
// in the device desciptor are changed to zeros.

struct usb_string_descriptor_struct
{
    BYTE bLength;
    BYTE bDescriptorType;
    BYTE wString[];
};
const struct usb_string_descriptor_struct const _usb_string0 = {
    4,
    3,
    {0x09, 0x04}
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
const struct descriptor_list_struct
{
    uint16_t wValue;
    uint16_t wIndex;
    const BYTE *addr;
    BYTE length;
} const descriptor_list[] = {
    {0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
    {0x0200, 0x0000, config1_descriptor, sizeof(config1_descriptor)},
    {0x2200, RAWHID_INTERFACE, rawhid_hid_report_desc,
                sizeof(rawhid_hid_report_desc)},
    {0x2100, RAWHID_INTERFACE, config1_descriptor + RAWHID_HID_DESC_OFFSET, 9},
    {0x0300, 0x0000, (const BYTEPTR)&_usb_string0, 4},
    {0x0301, 0x0409, (const BYTEPTR)&_usb_string1, STR_MANUFLENGTH},
    {0x0302, 0x0409, (const BYTEPTR)&_usb_string2, STR_PRODLENGTH}
};

#define NUM_DESC_LIST ((int)(sizeof(descriptor_list)/sizeof(struct descriptor_list_struct)))

//********************************************************************
// END OF DEFINITIONS BORROWED FROM Teensy rawHID project.
// EVERYTHING BELOW WAS WRITTEN FROM SCRATCH BY THE newRPL Team

// NEW SIMPLIFIED GLOBALS

volatile BINT usb_drvstatus SYSTEM_GLOBAL;        // FLAGS TO INDICATE IF INITIALIZED, CONNECTED, SENDING/RECEIVING, ETC.

BINT usb_fileid SYSTEM_GLOBAL;    // CURRENT FILEID
BINT usb_fileid_seq SYSTEM_GLOBAL;        // SEQUENTIAL NUMBER TO MAKE FILEID UNIQUE
BINT usb_offset SYSTEM_GLOBAL;    // CURRENT OFFSET WITHIN THE FILE
WORD usb_crc32 SYSTEM_GLOBAL;     // CURRENT CRC32 OF DATA RECEIVED
BINT usb_lastgood_offset SYSTEM_GLOBAL;    // LAST KNOWN GOOD OFFSET WITHIN THE FILE
WORD usb_lastgood_crc SYSTEM_GLOBAL;     // LAST KNOWN MATCHING CRC32 OF DATA RECEIVED
BYTE usb_ctlbuffer[RAWHID_RX_SIZE + 1] SYSTEM_GLOBAL;     // BUFFER TO RECEIVE CONTROL PACKETS IN THE CONTROL CHANNEL
BYTE usb_tmprxbuffer[RAWHID_RX_SIZE + 1] SYSTEM_GLOBAL;   // TEMPORARY BUFFER TO RECEIVE DATA
BYTE usb_ctlrxbuffer[RAWHID_RX_SIZE + 1] SYSTEM_GLOBAL;   // TEMPORARY BUFFER TO RECEIVE CONTROL PACKETS
BYTE usb_ctltxbuffer[RAWHID_TX_SIZE + 1] SYSTEM_GLOBAL;   // TEMPORARY BUFFER TO TRANSMIT DATA

BYTE usb_rxtxbuffer[LONG_BUFFER_SIZE] SCRATCH_MEMORY;     // LARGE BUFFER TO RECEIVE AT LEAST 3 FULL FRAGMENTS
BINT usb_rxoffset SYSTEM_GLOBAL;  // STARTING OFFSET OF THE DATA IN THE RX BUFFER
volatile BINT usb_rxtxtop SYSTEM_GLOBAL;  // NUMBER OF BYTES USED IN THE RX BUFFER
volatile BINT usb_rxtxbottom SYSTEM_GLOBAL;       // NUMBER OF BYTES IN THE RX BUFFER ALREADY READ BY THE USER
volatile BINT usb_rxtotalbytes SYSTEM_GLOBAL;     // TOTAL BYTES ON THE FILE, 0 MEANS DON'T KNOW YET

BINT usb_txtotalbytes SYSTEM_GLOBAL;      // TOTAL BYTES ON THE FILE, 0 MEANS DON'T KNOW YET
BINT usb_txseq SYSTEM_GLOBAL;     // SEQUENTIAL NUMBER WITHIN A FRAGMENT OF DATA

BYTEPTR usb_ctlbufptr SYSTEM_GLOBAL;      // POINTER TO BUFFER DURING CONTROL CHANNEL TRANSFERS
BINT usb_ctlcount SYSTEM_GLOBAL;  // COUNT OF DATA DURING CONTROL CHANNEL TRANSFERS



//  END OF NEW GLOBALS
// ********************************

const WORD const crctable[256] = {
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
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
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

WORD usb_crc32roll(WORD oldcrc, BYTEPTR data, BINT len)
{
    WORD crc = oldcrc ^ 0xffffffff;
    while(len--)
        crc = crctable[(crc ^ *data++) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xffffffff;
}

static void usb_set_endpoints(void)
{
    *SCR = 3;       // ENABLE RESET AND SUSPEND MODES
    *FCON = 0;      // NO DMA
    *SSR = *SSR;    // CLEAR ALL ERROR CONDITIONS
    *EDR = 2;       // ONLY ENDPOINT 1 IS IN (TX), ALL OTHERS RX

    *IR = 0;     // SETUP ENDPOINT0
    *MPR = EP0_FIFO_SIZE;      // USE 64-BYTE PACKETS ON EP0
    *EP0CR = 0x0;
    *EP0SR = 0x13; // CLEAR

    *IR = 1;
    *MPR = EP1_FIFO_SIZE;      // USE 64-BYTE PACKETS ON EP1
    *ECR = 0x841;        // IN ENDPOINT FIFO_FLUSH
    *ESR = 0x762;
    *DCR = 0;      // NO DMA

    *IR = 2;
    *MPR = EP2_FIFO_SIZE;      // USE 64-BYTE PACKETS ON EP2
    *ECR = 0x1041;  // OUT ENDPOINT FIFO_FLUSH INTERRUPT MODE
    *ESR = 0x762;
    *DCR = 0;      // NO DMA

    // SET WHICH INTERRUPTS WE WANT

    *EIER = 0x7;                // ENABLE ONLY ENDPOINTS 0 1 AND 2 INTERRUPTS
    *EIR = 0x1ff;               // CLEAR ALL INTERRUPTS
}

static void usb_reset(void)
{
    // COMPLETELY REPROGRAM THE DEVICE BLOCK
    usb_set_endpoints();

    usb_drvstatus = USB_STATUS_INIT | USB_STATUS_CONNECTED;   // DECONFIGURE THE DEVICE
}

static void usb_reset_full(void)
{
    *URSTCON |= (URSTCON_FUNC_RESET | URSTCON_PHY_RESET);
    tmr_delay100us();

    *URSTCON &= ~(URSTCON_FUNC_RESET | URSTCON_PHY_RESET);
    tmr_delay100us();     // WAIT FOR THE PHY AND THE DEVICE BLOCK TO RESET

    usb_reset();
}

void usb_hwsetup()
{

    // POWER OFF FIRST TO CAUSE A FULL RESET OF THE USB BLOCK

    *UCLKCON &= ~0x80000000;     // SIGNAL THE HOST A DEVICE HAS BEEN DISCONNECTED

    *GPHCON = (*GPHCON & ~0x30000000) | 0x10000000; // SET GPH14 AS OUTPUT
    *GPHDAT &= ~0x4000; // GPH14 = LOW (TURN OFF PHY POWER REGULATOR)


    *PWRCFG &= ~0x10;      // POWER OFF PHY
    *HCLKCON &= ~0x1800;   // REMOVE HCLK FROM BOTH DEVICE AND HOST

    tmr_delay100us();     // LET EVERYTHING SETTLE DOWN AND DIE OFF

    *GPHUDP = (*GPHUDP & ~0x30000000) | 0x20000000; // ENABLE PULLUP
    *GPHDAT |= 0x4000; // GPH14 = HIGH (TURN ON PHY POWER REGULATOR)

    // ENABLE CLOCKS TO THE BLOCK
    *HCLKCON |= 0x1000;    // ENABLE HCLK INTO USB DEVICE

    *PHYCTRL = 0x12;        // DEVICE MODE, USE INTERNAL PLL, 12 MHz, CRYSTAL ENABLE
    *UCLKCON = 0x4;          // DON'T ENABLE PULL-UP ON D+ JUST YET, WAIT FOR FULL INITIALIZATION BEFORE DOING THAT
                             // USB HOST CLOCK CONTROL: DISABLED, DEVICE CLOCK: ENABLED
    *PHYPWR = 0x30;       // NORMAL OPERATION (manual shows must be 0x3 bits)

    *MISCCR &= ~0x1000;        // USB PADS TO NORMAL MODE

    *PWRCFG |= 0x10;       // POWER ON PHY

    usb_reset_full();

    *UCLKCON |= 0x80000000;     // CONNECT PULL UP TO SIGNAL THE HOST A DEVICE HAS BEEN CONNECTED

}

void usb_hwsuspend()
{
    //*PHYPWR = 0x31;
    //*MISCCR |= 0x1000;
    usb_drvstatus |= USB_STATUS_SUSPEND;
}

void usb_hwresume()
{
    //*PHYPWR = 0x30;
    //*MISCCR &= ~0x1000;
    usb_drvstatus &= ~USB_STATUS_SUSPEND;
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

    if(usb_drvstatus & USB_STATUS_INIT) {
        if(!force)
            return;
    }

    //usb_intcount=0;

    usb_drvstatus = USB_STATUS_INIT;

    usb_fileid = 0;
    usb_fileid_seq &= 0xff;
    usb_offset = 0;
    usb_crc32 = 0;    // RESET CRC32

    usb_txtotalbytes = 0;

    usb_rxoffset = 0;
    usb_rxtxtop = 0;  // NUMBER OF BYTES USED IN THE RX BUFFER
    usb_rxtxbottom = 0;       // NUMBER OF BYTES IN THE RX BUFFER ALREADY READ BY THE USER
    usb_rxtotalbytes = 0;     // DON'T KNOW THE TOTAL FILE SIZE YET

    usb_hwsetup();


    // SETUP CABLE DISCONNECT DETECTION

    *GPFCON = (*GPFCON & ~0xc0) | 0x80;    // SET GPF3 AS EINT3
    *GPFUDP = (*GPFUDP & ~0xc0) | 0x40;     // PULL DOWN ENABLED
    *EXTINT0 = (*EXTINT0 & ~0x7000) | (CABLE_IS_CONNECTED? 0x2000:0x4000);   // FALLING EDGE TRIGGERED



    if(!CABLE_IS_CONNECTED) {
        usb_shutdown(); // THERE'S NO CABLE, SHUTDOWN IMMEDIATELY
        return;
    }

    // SET INTERRUPT HANDLER
    irq_mask(25);
    irq_mask(3);

    irq_addhook(25, &usb_irqservice);
    irq_addhook(3, &usb_irqdisconnect);

    // ELIMINATE PREVIOUS DISCONNECT INTERRUPTS CAUSED BY NOISE AND USB INTERRUPTS
    irq_clrpending(25);
    irq_clrpending(3);

    irq_unmask(25);
    irq_unmask(3);

    usb_drvstatus |= USB_STATUS_CONNECTED;

    // TODO: SETUP COMMUNICATIONS BUFFERS
}

void usb_shutdown()
{

    if(!(usb_drvstatus & USB_STATUS_INIT))
        return;

    // MASK INTERRUPT AND REMOVE HANDLER
    irq_mask(25);
    irq_mask(3);

    irq_releasehook(25);


    // CLEANUP INTERRUPT SYSTEM
    *EIER = 0;                  // DISABLE ALL INTERRUPTS
    *EIR = 0x1f;                // CLEAR ALL PENDING INTERRUPTS



    // POWER OFF THE PHY AND CLOCKS
    *PHYPWR |= 0x1; // force usb suspend
    *UCLKCON &= ~0x80000000; // disable pull-up on d+ line
    *MISCCR |= 0x1000; // usb suspend mode

    *SCLKCON &= ~2;        // DISABLE USB HOST CLOCK
    *PWRCFG &= ~0x10;      // POWER OFF PHY
    *HCLKCON &= ~0x1800;   // REMOVE HCLK FROM BOTH DEVICE AND HOST

    *GPHCON = (*GPHCON & ~0x30000000) | 0x10000000; // SET GPH14 AS OUTPUT
    *GPHUDP = (*GPHUDP & ~0x30000000);              // DISABLE PULLUP/DOWN
    *GPHDAT &= ~0x4000; // GPH14 = LOW (TURN OFF PHY POWER REGULATOR)

    usb_fileid = 0;
    usb_drvstatus = 0;        // MARK UNCONFIGURED


    // SETUP CABLE DISCONNECT DETECTION

    *GPFCON = (*GPFCON & ~0xc0) | 0x80;    // SET GPF3 AS EINT3
    *GPFUDP = (*GPFUDP & ~0xc0) | 0x40;     // PULL DOWN ENABLED
    *EXTINT0 = (*EXTINT0 & ~0x7000) | (CABLE_IS_CONNECTED? 0x2000:0x4000);   // FALLING EDGE TRIGGERED

    irq_addhook(3, &usb_irqconnect);

    irq_unmask(3);
}

static void inline usb_flush_fifo0(WORD halfwords) {
    uint32_t sink;
    while (halfwords > 0) {
        sink = *EP0BR;
        --halfwords;
    }
    *EP0SR = EP0SR_RSR;          // CLEAR THAT WE RECEIVED THE PACKET
}

static void inline usb_flush_fifo2(WORD halfwords) {
    uint32_t sink;
    while (halfwords > 0) {
        sink = *EP2BR;
        --halfwords;
    }
    // ESR_RPS IS CLEARED AUTOMATICALLY
}

// IF usb_ctlbufptr IS NULL, usb_ctlcount ZEROES ARE SENT
static void usb_ep0_write_buffer(void)
{
    *IR = 0;

    int transfercount = (usb_ctlcount > EP0_FIFO_SIZE) ? EP0_FIFO_SIZE : usb_ctlcount;
    *BWCR = transfercount; // BWCR = WRITE COUNT - IN BYTES DESPITE THE FIFO USING HALF-WORDS

    int halfword = 0;
    while(transfercount > 1) {
        if (usb_ctlbufptr != NULL) {
            halfword = usb_ctlbufptr[0] | (usb_ctlbufptr[1]<<8);
        }
        *EP0BR = halfword;
        usb_ctlbufptr += 2;
        transfercount -= 2;
        usb_ctlcount -= 2;
    }

    if(transfercount == 1) {
        if (usb_ctlbufptr != NULL) {
            halfword = usb_ctlbufptr[0] ; // UPPER BYTE IS ZERO
        }
        *EP0BR = halfword;
        ++usb_ctlbufptr;
        --transfercount;
        --usb_ctlcount;
    }

    if (usb_ctlcount == 0) {
        // PACKET WAS THE LAST
        usb_drvstatus &= ~USB_STATUS_EP0TX; // FINISHED TRANSMITTING
    }
}

static void usb_ep1_write_ctrlbuffer(void)
{
    *IR = RAWHID_TX_ENDPOINT;

    *BWCR = RAWHID_TX_SIZE; // BWCR = WRITE COUNT - IN BYTES TO BE ABLE TO INDICATE UNEVEN BYTES

    int cnt = 0;
    int halfword;
    for (cnt = 0; cnt < RAWHID_TX_SIZE; cnt += 2) {
        halfword = usb_ctltxbuffer[cnt] | (usb_ctltxbuffer[cnt + 1]<<8);
        *EP1BR = halfword;
    }

    usb_drvstatus &= ~USB_STATUS_TXCTL;
    usb_drvstatus |= USB_STATUS_WAIT_FOR_ACK | USB_STATUS_SEND_ZERO_LENGTH_PACKET;
}

static void usb_ep1_send_zero_length_packet(void)
{
    *IR = RAWHID_TX_ENDPOINT;

    *BWCR = 0;
    usb_drvstatus |= USB_STATUS_WAIT_FOR_ACK;
    usb_drvstatus &= ~USB_STATUS_SEND_ZERO_LENGTH_PACKET;
}

// TRANSMIT BYTES TO THE HOST IN EP0 ENDPOINT
// STARTS A NEW TRANSMISSION EVEN IF
// THERE ARE ZERO BYTES IN THE BUFFER
// FOR USE WITHIN ISR
void usb_ep0_transmit(BYTEPTR buffer, BINT count)
{

    if(!(usb_drvstatus & USB_STATUS_CONNECTED))
        return;

    if (usb_drvstatus & (USB_STATUS_EP0TX | USB_STATUS_EP0RX)) {
        // THERE IS STILL SENT DATA THAT WASN'T ACKed OR DATA TO RECEIVE
        // DO NOT OVERWRITE VARIABLES
        return;
    }

    // THIS STATUS BIT IS SET BEFORE FIRST BYTE IS PUT INTO FIFO
    // AND IS CLEARED AFTER LAST BYTE IS PUT INTO FIFO
    usb_drvstatus |= USB_STATUS_EP0TX;

    usb_ctlbufptr = buffer;
    usb_ctlcount = count;

    usb_ep0_write_buffer();
}

static void usb_ep0_read_buffer(void)
{
    *IR = 0;

    int halfword;
    int lwo = *EP0SR & EP0SR_LWO;   // GET THE LAST WORD ODD BIT BEFORE IT'S AUTOMATICALLY CLEARED

        int total = usb_ctlcount;
    if (total > EP0_FIFO_SIZE) {
        total = EP0_FIFO_SIZE;
    }
    int fifocount = *BRCR & 0x3ff; //BRCR HAS uint16_tS
    if (total > (fifocount << 1)) {
        total = (fifocount << 1);
    }
    int transfercount = total;

    while(transfercount > 0) {
        halfword=*EP0BR;
        transfercount -= 2;
        --fifocount;

        if (transfercount == 0 && lwo) {
            *usb_ctlbufptr++ = LSB(halfword);
            --usb_ctlcount;
        } else {
            *usb_ctlbufptr++ = MSB(halfword);
            *usb_ctlbufptr++ = LSB(halfword);
            usb_ctlcount -= 2;
        }
    }

    // THERE WAS EXTRA DATA IN THE PACKET??
    // JUST IN CASE CLEAN THE FIFO
    // FIXME look if 2 packets are in fifo
    usb_flush_fifo0(fifocount);

    if(usb_ctlcount == 0) {
        usb_drvstatus &= ~USB_STATUS_EP0RX;
    }
        }

// RECEIVE BYTES FROM THE HOST IN EP0 ENDPOINT
// STARTS A NEW TRANSMISSION EVEN IF
// THERE ARE ZERO BYTES IN THE BUFFER
// FOR USE WITHIN ISR
void usb_ep0_receive(BYTEPTR buffer, BINT count)
{

    if(!(usb_drvstatus & USB_STATUS_CONNECTED))
        return;

    if (usb_drvstatus & (USB_STATUS_EP0TX | USB_STATUS_EP0RX)) {
        // THERE IS STILL SENT DATA THAT WASN'T ACKed OR DATA TO RECEIVE
        // DO NOT OVERWRITE VARIABLES
        return;
    }

    // THIS STATUS BIT IS SET BEFORE FIRST BYTE IS READ
    // AND IS CLEARED AFTER LAST BYTE IS READ
    usb_drvstatus |= USB_STATUS_EP0RX;

    usb_ctlbufptr = buffer;
    usb_ctlcount = count;

    usb_ep0_read_buffer();
}

ARM_MODE void usb_checkpipe()
{
    if (*EP0SR & EP0SR_SHT || *EP0CR & EP0CR_ESS) {
        // STALL HANDSHAKE SENT OR GOING TO BE SENT

        // CANCEL ALL ONGOING TRANSMISSIONS
        usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
        *EP0SR |= EP0SR_SHT;  // CLEAR SENT HANDSHAKE
        // DON'T CLEAR HANDSHAKE TO BE SENT
    }

    // FIXME SSR?
}

ARM_MODE void ep0_irqservice()
{
    *IR = 0;     // SELECT ENDPOINT 0

    usb_checkpipe();

    if (*EP0SR & EP0SR_TST) {
        *EP0SR = EP0SR_TST;

        // THERE IS STILL DATA WAITING, SEND NEXT BATCH
        if (usb_drvstatus & USB_STATUS_EP0TX) {
            usb_ep0_write_buffer();
            usb_checkpipe();
        }
    }

    if(*EP0SR & EP0SR_RSR) {

        // PROCESS FIRST ANY ONGOING TRANSMISSIONS
        if(usb_drvstatus & USB_STATUS_EP0RX) {
            usb_ep0_read_buffer();
            usb_checkpipe();
            return;
        }

        if (usb_ctlcount != 0) {
            // ONGOING OPERATION
            return;
        }


        // WE HAVE A PACKET
        BINT reqtype;
        BINT request;
        BINT value;
        BINT index;
        BINT length;
        BINT halfword;

        // READ ALL 8 BYTES FROM THE FIFO
        WORD fifocount = *BRCR & 0x3ff;

        halfword = *EP0BR;
        --fifocount;
        reqtype = MSB(halfword);
        request = LSB(halfword);

        halfword = *EP0BR;
        --fifocount;
        value = MSB(halfword) | (LSB(halfword) << 8);

        halfword = *EP0BR;
        --fifocount;
        index = MSB(halfword) | (LSB(halfword) << 8);

        halfword = *EP0BR;
        --fifocount;
        length = MSB(halfword) | (LSB(halfword) << 8);

        *EP0SR = EP0SR_RSR;          // CLEAR THAT WE RECEIVED THE PACKET

        // READ ANY EXTRA DATA TO KEEP THE FIFO CLEAN
        // FIXME check if two packets
        usb_flush_fifo0(fifocount);

        if((reqtype & 0x60) == 0)       // STANDARD REQUESTS
        {

            // PROCESS THE REQUEST
            switch (request) {
            case GET_DESCRIPTOR:
            {
                // SEND THE REQUESTED RESPONSE
                int k;
                for(k = 0; k < NUM_DESC_LIST; ++k) {
                    if((descriptor_list[k].wValue == value)
                            && (descriptor_list[k].wIndex == index)) {
                        // FOUND THE REQUESTED DESCRIPTOR
                        usb_ep0_transmit(
                            (BYTEPTR) descriptor_list[k].addr,
                            (length < descriptor_list[k].length) ? length : descriptor_list[k].length
                        );    // SEND 0-DATA STATUS STAGE
                        usb_checkpipe();
                        return;
                    }
                }

                // SPECIAL CASE - CALCULATOR SERIAL NUMBER STRING
                if((0x0303 == value) && (0x0409 == index)) {
                    // FOUND THE REQUESTED DESCRIPTOR
                    usb_ctlbuffer[0] = 20 + 2;
                    usb_ctlbuffer[1] = 3;

                    // COPY THE SERIAL NUMBER - EXPAND ASCII TO UTF-16
                    int n;
                    BYTEPTR ptr = (BYTEPTR) SERIAL_NUMBER_ADDRESS;
                    for(n = 0; n < 10; ++n, ++ptr) {
                        usb_ctlbuffer[2 + 2 * n] = *ptr;
                        usb_ctlbuffer[3 + 2 * n] = 0;
                    }
                    usb_ep0_transmit(
                        usb_ctlbuffer,
                        (length < usb_ctlbuffer[0]) ? length : usb_ctlbuffer[0]
                    );        // SEND 0-DATA STATUS STAGE
                    usb_checkpipe();
                    return;
                }

                // DON'T KNOW THE ANSWER TO THIS
                usb_ep0_transmit(NULL, length);    // SEND DATA STATUS STAGE
                *EP0CR |= EP0CR_ESS;          // STALL ENDPOINT 0
                usb_checkpipe();

                return;
            }
            case SET_ADDRESS:
            {
                usb_ctlcount = 0;
                usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
                //*FUNC_ADDR_REG = value | 0x80;    // ASSIGNED AUTOMATICALLY BY THE USB CORE TO THE FAR REGISTER
                usb_checkpipe();
                return;
            }
            case SET_CONFIGURATION:
            {
                // OUR DEVICE HAS ONE SINGLE CONFIGURATION AND IS SETUP
                // ON WAKEUP, SO NOTHING TO DO HERE BUT ACKNOWLEDGE

                if(value) {
                    usb_drvstatus |= USB_STATUS_CONFIGURED;

                    // NOTHING NEEDED ON S3C2416??
                    // SET AUTOMATIC RESPONSE ON FIRST TX INTERRUPT
                    //*INDEX_REG = 1;
                    //*IN_CSR1_REG |= EPn_IN_PKT_RDY;
                    //*INDEX_REG = 0;
                }
                else
                    usb_drvstatus &= ~USB_STATUS_CONFIGURED;
                usb_ctlcount = 0;
                usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);

                usb_checkpipe();

                return;
            }
            case GET_CONFIGURATION:
            {
                BINT configvalue =
                        (usb_drvstatus & USB_STATUS_CONFIGURED) ? 1 : 0;
                usb_ctlbuffer[0] = configvalue;
                usb_ep0_transmit(usb_ctlbuffer, 1);    // SEND DATA STATUS STAGE
                usb_checkpipe();

                return;
            }
            case GET_STATUS:
            {
                usb_ctlbuffer[0] = usb_ctlbuffer[1] = 0;
                switch (reqtype) {
                case 0x80:     // DEVICE GET STATUS
                    *(usb_ctlbuffer) =
                            (usb_drvstatus & USB_STATUS_WAKEUPENABLED) ? 2 :
                            0;
                    break;
                case 0x82:     // ENDPONT GET STATUS

                    // FIXME USE PAIRS EP0SR|EP0SR_SHT AND ESR|ESR_FSC
                    // OR EP0CR|EP0CR_ESS AND ECR|ECR_ESS?
                    *IR = index & 0x7;
                    if((index & 7) == 0) {
                        *(usb_ctlbuffer) =
                                ((*EP0SR) & EP0SR_SHT) ? 1 : 0;
                    }
                    else {
                        *(usb_ctlbuffer) |=
                                ((*ECR) & ECR_ESS) ? 1 : 0;
                    }
                    break;
                }

                // FOR NOW SEND THE DATA
                usb_ep0_transmit(usb_ctlbuffer, 2);    // SEND DATA STATUS STAGE
                usb_checkpipe();

                return;
            }
            case SET_FEATURE:
            {
                switch (reqtype) {
                case 0:        // DEVICE FEATURES
                    if(value == 1)
                        usb_drvstatus |= USB_STATUS_WAKEUPENABLED;
                    if(value == 2)
                        usb_drvstatus |= USB_STATUS_TESTMODE;
                    break;
                case 1:        // INTERFACE FEATURES
                    // NO INTERFACE FEATURES
                    break;
                case 2:        // ENDPOINT FEATURES
                    if(value == 0) {
                        // ENDPOINT_HALT FEATURE REQUEST

                        int endp = index & 7;
                        *IR = endp;
                        if(endp != 0)   // DO NOT  THE CONTROL ENDPOINT
                        {
                            *ECR |= ECR_ESS;
                        }
                    }
                    break;
                }

                usb_ctlcount = 0;
                usb_drvstatus &= ~USB_STATUS_EP0RX | USB_STATUS_EP0TX;
                usb_checkpipe();

                return;
            }
            case CLEAR_FEATURE:
            {
                switch (reqtype) {
                case 0:        // DEVICE FEATURES
                    if(value == 1)
                        usb_drvstatus &= ~USB_STATUS_WAKEUPENABLED;
                    if(value == 2)
                        usb_drvstatus &= ~USB_STATUS_TESTMODE;
                    break;
                case 1:        // INTERFACE FEATURES
                    // NO INTERFACE FEATURES
                    break;
                case 2:        // ENDPOINT FEATURES
                    if(value == 0) {
                        // ENDPOINT_HALT FEATURE REQUEST

                        int endp = index & 3;
                        *IR = endp;
                        if(endp == 1 || endp == 2)   // DO NOT STALL THE CONTROL ENDPOINT
                        {
                            *ECR |= ECR_FLUSH;   // FLUSH THE FIFO
                            *ECR &= ~ECR_ESS;    // CLEAR THE STALL CONDITION

                        }
                    }
                    break;
                }

                usb_ctlcount = 0;
                usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
                usb_checkpipe();

                return;
            }

            }
            // UNKNOWN STANDARD REQUEST??
            // DON'T KNOW THE ANSWER TO THIS BUT KEEP THE PIPES RUNNING
            if((reqtype & USB_DIRECTION) == USB_DEV_TO_HOST) {
                // SEND THE RIGHT AMOUNT OF ZEROS TO THE HOST
                usb_ep0_transmit(NULL, length);
                usb_checkpipe();

                return;
            }

            // THIS IS AN INCOMING REQUEST WITH NO DATA STAGE

            usb_ctlcount = 0;
            usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
            usb_checkpipe();

            return;

        }

        if((reqtype & 0x61) == 0x21)    // CLASS INTERFACE REQUESTS
        {

            if(index == RAWHID_INTERFACE) {
                switch (request) {
                case HID_SET_REPORT:
                    // GET DATA FROM HOST
                    usb_ep0_receive(usb_ctlbuffer, RAWHID_TX_SIZE);  // FOR NOW, LET'S SEE WHAT TO DO WITH THIS LATER
                    return;
                case HID_GET_REPORT:
                    // SEND DATA TO HOST - SEND ALL ZEROS
                    usb_ctlbuffer[0] =
                            (usb_drvstatus & USB_STATUS_RXDATA) ? 1 : 0;
                    for (int i = 1; i < RAWHID_TX_SIZE; ++i) {
                        usb_ctlbuffer[i] = 0;
                    }
                    usb_ep0_transmit(usb_ctlbuffer, RAWHID_TX_SIZE);
                    usb_checkpipe();

                    return;

                case HID_SET_IDLE:
                    // SEND DATA TO HOST - SEND ALL ZEROS
                    usb_ctlcount = 0;
                    usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
                    usb_checkpipe();
                    return;

                }

            }
            // UNKNOWN CLASS REQUEST??
            // DON'T KNOW THE ANSWER TO THIS
            if((reqtype & USB_DIRECTION) == USB_DEV_TO_HOST) {
                // SEND THE RIGHT AMOUNT OF ZEROS TO THE HOST
                usb_ep0_transmit(NULL, length);
                *EP0CR |= EP0CR_ESS;          // STALL ENDPOINT 0
                usb_checkpipe();

                return;
            }

            usb_ctlcount = 0;
            usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
            *EP0CR |= EP0CR_ESS;          // STALL ENDPOINT 0
            usb_checkpipe();
            return;

        }

        // ADD OTHER REQUESTS HERE

        if((reqtype & USB_DIRECTION) == USB_DEV_TO_HOST) {
            // SEND THE RIGHT AMOUNT OF ZEROS TO THE HOST
            usb_ep0_transmit(NULL, length);
            usb_checkpipe();

            return;
        }

        usb_ctlcount = 0;
        usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
        usb_checkpipe();

        return;

    }
}

static int inline ringbuffer_inc(int index, int offset)
{
    index += offset;
    if (index >= RING_BUFFER_SIZE) {
        index -= RING_BUFFER_SIZE;
    }
    return index;
}

static int inline ringbuffer_dec(int index, int offset)
{
    index -= offset;
    if (index < 0) {
        index += RING_BUFFER_SIZE;
    }
    return index;
}

// DRIVER - PACKET TRANSMISSION CALLED BY IRQ - NEVER CALLED BY USER
// SEND A CONTROL PACKET IF ONE AVAILABLE, OR ONE DATA PACKET IF AVAILABLE
void usb_ep1_transmit()
{

    if(!(usb_drvstatus & USB_STATUS_CONNECTED))
        return;

    if (usb_drvstatus & USB_STATUS_WAIT_FOR_ACK) {
        return;
    }

    if (usb_drvstatus & USB_STATUS_SEND_ZERO_LENGTH_PACKET) {
        usb_ep1_send_zero_length_packet();
        return;
    }

    if(usb_drvstatus & USB_STATUS_TXCTL) {
        // WE HAVE A CONTROL PACKET READY TO GO IN THE CONTROL BUFFER
        usb_ep1_write_ctrlbuffer();
        return;
    }

    if(usb_drvstatus & USB_STATUS_TXDATA) {
        // WE HAVE A DATA PACKET TO SEND

        *IR = RAWHID_TX_ENDPOINT;

        if(usb_drvstatus & USB_STATUS_HALT) {
            // REMOTE REQUESTED WE STOP SENDING DATA UNTIL IT PROCESSES IT
            // JUST REPLY WITH A ZERO DATA PACKET
            usb_ep1_send_zero_length_packet();
            return;
        }

        if(usb_drvstatus & USB_STATUS_ERROR) {
            // THE REMOTE DIDN'T GET IT, WE NEED TO RESEND
            // THE WANTED OFFSET WAS LEFT IN usb_rxoffset BY usb_receivecontrolpacket()
            if(usb_rxoffset != usb_offset) {
                // THE REMOTE WANTS A PREVIOUS CHUNK OF THE FILE
                // CHECK IF WE HAVE IT IN THE SOURCE BUFFER

                // usb_offset ALWAYS POINTS TO THE OFFSET OF usb_rxtxbottom = LAST BYTE SENT

                int bufoff = (int)usb_offset - (int)usb_rxoffset;
                int oldestdata = ringbuffer_dec(usb_rxtxbottom, usb_rxtxtop);
                if((bufoff < 0) || (bufoff > oldestdata) || (usb_rxoffset!=usb_lastgood_offset)) {
                    // WE DON'T HAVE THAT DATA STORED ANYMORE, ABORT THE FILE
                    usb_sendcontrolpacket(P_TYPE_ABORT);
                    usb_fileid = 0;
                    usb_offset = 0;
                    usb_crc32 = 0;
                    usb_rxtxtop = usb_rxtxbottom = 0;

                    usb_drvstatus &= ~USB_STATUS_TXDATA;
                    usb_drvstatus |= USB_STATUS_ERROR;

                    usb_ep1_write_ctrlbuffer();
                    return;
                }

                // ADJUST THE RING'S POSITION TO SEND THE RIGHT DATA
                usb_rxtxbottom = ringbuffer_dec(usb_rxtxbottom, bufoff);
                usb_offset = usb_rxoffset;
            }
            usb_txseq = 0;    // RESTART BACK THE SEQUENCE NUMBER
            usb_crc32 = usb_lastgood_crc;            // RESET THE CRC FROM HERE ON
            usb_drvstatus &= ~USB_STATUS_ERROR;       // REMOVE THE ERROR AND RESEND

        }

        int bufbytes;
        int p_type;
        int eof = 0;

        bufbytes = ringbuffer_dec(usb_rxtxtop, usb_rxtxbottom);

        if(bufbytes > USB_DATASIZE)
            bufbytes = USB_DATASIZE;    // DON'T SEND MORE THAN ONE PACKET AT A TIME

        // CHECK IF THESE ARE THE LAST FEW BYTES OF THE FILE
        if((int)usb_txtotalbytes - (int)usb_offset == bufbytes)
            eof = 1;
        else {
            if(bufbytes < USB_DATASIZE) {
                // WAIT FOR MORE DATA TO FILL UP THE PACKET, NO NEED TO SEND IT NOW
                // JUST REPLY WITH A ZERO DATA PACKET
                usb_ep1_send_zero_length_packet();
                return;
            }
        }

        p_type = usb_txseq + 1;

        if(eof)
            p_type |= 0x40;
        if(p_type == 32)
            p_type = 0x40;

        if(p_type==1) {
            // SAVE AT THE BEGINNING OF EACH FRAGMENT
            usb_lastgood_offset = usb_offset;
            usb_lastgood_crc = usb_crc32;
        }


        // SEND A FULL PACKET
        // RAWHID_TX_SIZE AND USB_DATASIZE ARE ALWAYS EVEN

        *BWCR = RAWHID_TX_SIZE;

        int halfword;

        halfword = p_type | (bufbytes << 8);
        *EP1BR = halfword;

        //halfword = (usb_fileid >> 8) | ((usb_fileid & 0xff) << 8);
        halfword = usb_fileid;
        *EP1BR = halfword;

        //halfword = ((usb_offset & 0xff) << 8) | ((usb_offset & 0xff00) >> 8);
        halfword = usb_offset & 0xffff;
        *EP1BR = halfword;

        //halfword = ((usb_offset & 0xff0000) >> 8) | ((usb_offset & 0xff000000) >> 24);
        halfword = usb_offset >> 16;
        *EP1BR = halfword;

        int cnt;
        for(cnt = 0; cnt < USB_DATASIZE; cnt += 2) {
            if(cnt >= bufbytes)
                *EP1BR = 0;
            else {
                halfword = usb_rxtxbuffer[usb_rxtxbottom];
                usb_crc32 = usb_crc32roll(usb_crc32, usb_rxtxbuffer + usb_rxtxbottom, 1);       // UPDATE THE CRC32
                usb_rxtxbottom = ringbuffer_inc(usb_rxtxbottom, 1);

                halfword |= (usb_rxtxbuffer[usb_rxtxbottom] << 8);
                usb_crc32 = usb_crc32roll(usb_crc32, usb_rxtxbuffer + usb_rxtxbottom, 1);       // UPDATE THE CRC32
                usb_rxtxbottom = ringbuffer_inc(usb_rxtxbottom, 1);

                *EP1BR = halfword;
            }
        }

        usb_offset += bufbytes;
        usb_txseq = p_type & 0x1f;

        usb_drvstatus |= USB_STATUS_WAIT_FOR_ACK | USB_STATUS_SEND_ZERO_LENGTH_PACKET;

        if(eof) {
            usb_sendcontrolpacket(P_TYPE_ENDOFFILE);
            usb_drvstatus &= ~USB_STATUS_TXDATA;

            // DONE SENDING ALL DATA

        }
        else {
            // AT EACH END OF FRAGMENT, SEND A CHECKPOINT BUT NOT AT END OF FILE
            if(p_type & 0x40)
                usb_sendcontrolpacket(P_TYPE_CHECKPOINT);

            // IF WE CONSUMED ALL THE BUFFER, SIGNAL THAT WE ARE DONE
            if(usb_rxtxtop == usb_rxtxbottom)
                usb_drvstatus &= ~USB_STATUS_TXDATA;

        }

        return;
    }

    // NOTHING TO TRANSMIT
}

// RECEIVE BYTES FROM THE HOST IN EP2 ENDPOINT
// FOR USE WITHIN ISR

void usb_ep2_receive()
{

    if(!(usb_drvstatus & USB_STATUS_CONNECTED))
        return;

    *IR = RAWHID_RX_ENDPOINT;

    if(!(*ESR & ESR_RPS)) {
        // THERE'S NO PACKETS AVAILABLE
        return;
    }

    int fifocnt = *BRCR & 0x3ff; // BRCR HAS uint16_tS;
    int lwo = *ESR & ESR_LWO;

    if(fifocnt < 4 || ( (fifocnt == 4) && lwo)) { // LESS THAN 8 BYTES
        // IGNORE THE MALFORMED PACKET
        usb_flush_fifo2(fifocnt);
        return;
    }

    // READ PACKET TYPE
    BYTEPTR rcvbuf;
    uint16_t halfword;

    // READ PACKET TYPE
    halfword = *EP2BR;
    --fifocnt;
    int p_type = MSB(halfword);

    if(p_type & 0x80) {
        // THIS IS A CONTROL PACKET
        // PUT IT IN THE CTL BUFFER AND NOTIFY THE USER
        usb_drvstatus |= USB_STATUS_RXCTL;

        rcvbuf = usb_ctlrxbuffer;
        rcvbuf[0] = MSB(halfword);
        rcvbuf[1] = LSB(halfword);
        rcvbuf += 2;
        while(fifocnt) {
            halfword = *EP2BR;
            --fifocnt;
            if (fifocnt == 0 && lwo) {
                rcvbuf[0] = LSB(halfword);
                ++rcvbuf;
            } else {
                rcvbuf[0] = MSB(halfword);
                rcvbuf[1] = LSB(halfword);
                rcvbuf += 2;
            }
        }
        usb_flush_fifo2(fifocnt);
        usb_receivecontrolpacket();
        return;
    }

    int cnt;

    // THIS IS A DATA PACKET
    // READ HEADER DIRECTLY INTO THE BUFFER


    rcvbuf = usb_tmprxbuffer;
    rcvbuf[0] = MSB(halfword);
    rcvbuf[1] = LSB(halfword);
    rcvbuf += 2;
    cnt = 2;
    while(cnt < 8) {
        halfword = *EP2BR;
        --fifocnt;
        rcvbuf[0] = MSB(halfword);
        rcvbuf[1] = LSB(halfword);
        cnt += 2;
        rcvbuf += 2;
    }

    USB_PACKET *pptr = (USB_PACKET *) usb_tmprxbuffer;

    // IS IT FROM THE SAME FILE?
    if(P_FILEID(pptr) != usb_fileid) {
        // DIFFERENT FILE? IGNORE
        // IGNORE THE PACKET
        usb_flush_fifo2(fifocnt);
        return;
    }

    // IS THE CORRECT OFFSET?
    if(pptr->p_offset != usb_offset) {
        //  WE MUST'VE MISSED SOMETHING
        if(!(usb_drvstatus & USB_STATUS_ERROR)) {
            usb_drvstatus |= USB_STATUS_ERROR;
            // SEND A REPORT NOW IF POSSIBLE, OTHERWISE THE ERROR INFO WILL GO IN THE NEXT REPORT
            if(!(usb_drvstatus & USB_STATUS_TXCTL))
                usb_sendcontrolpacket(P_TYPE_REPORT);
        }

        // IGNORE THE PACKET
        usb_flush_fifo2(fifocnt);
        return;

    }
    else {
        // GOT THE RIGHT OFFSET, RESET ERROR CONDITION IF ANY
        if(usb_drvstatus & USB_STATUS_ERROR) {
            // CRC AND OFFSET SHOULD BE CORRECTLY SET BY NOW
            // SO LIFT THE ERROR CONDITION
            usb_drvstatus &= ~USB_STATUS_ERROR;
        }
    }

    // DO WE HAVE ENOUGH ROOM AVAILABLE?
    int usedspace = ringbuffer_dec(usb_rxtxtop, usb_rxtxbottom);

    if(pptr->p_dataused > RING_BUFFER_SIZE - usedspace) {
        // DATA WON'T FIT IN THE BUFFER DUE TO OVERFLOW, ISSUE AN ERROR AND REQUEST RESEND
        usb_drvstatus |= USB_STATUS_ERROR;
        // SEND A REPORT NOW IF POSSIBLE, OTHERWISE THE ERROR INFO WILL GO IN THE NEXT REPORT
        if(!(usb_drvstatus & USB_STATUS_TXCTL))
            usb_sendcontrolpacket(P_TYPE_REPORT);

        // IGNORE THE PACKET
        usb_flush_fifo2(fifocnt);
        return;

    }

    // NEW PACKET IS READY TO BE RECEIVED, IF WE ARE STARTING A NEW FRAGMENT
    // SAVE RESTORE POINT TO REQUEST THE FRAGMENT TO BE RESENT IF IT FAILS
    if(p_type==1) {
        usb_lastgood_offset = usb_offset;
        usb_lastgood_crc = usb_crc32;
    }



    // WE HAVE NEW DATA, RECEIVE IT DIRECTLY AT THE BUFFER
    while(fifocnt && (cnt < pptr->p_dataused + 8)) {
        halfword = *EP2BR;
        --fifocnt;

        if ( (fifocnt != 0) || !lwo) {
            usb_rxtxbuffer[usb_rxtxtop] = MSB(halfword);
            usb_crc32 = usb_crc32roll(usb_crc32, usb_rxtxbuffer + usb_rxtxtop, 1);
            usb_rxtxtop = ringbuffer_inc(usb_rxtxtop, 1);
            ++cnt;
        }

        usb_rxtxbuffer[usb_rxtxtop] = LSB(halfword);
        usb_crc32 = usb_crc32roll(usb_crc32, usb_rxtxbuffer + usb_rxtxtop, 1);
        usb_rxtxtop = ringbuffer_inc(usb_rxtxtop, 1);
        ++cnt;
    }

    // AND FLUSH ANY UNUSED BYTES
    usb_flush_fifo2(fifocnt);

    // UPDATE THE BUFFERS
    usb_offset += pptr->p_dataused;
    usedspace += pptr->p_dataused;

    if(usedspace >= RING_BUFFER_SIZE / 2) {
        usb_drvstatus |= USB_STATUS_HALT;     // REQUEST HALT IF BUFFER IS HALF-FULL
        // SEND A REPORT NOW IF POSSIBLE, OTHERWISE THE ERROR INFO WILL GO IN THE NEXT REPORT
        if(!(usb_drvstatus & USB_STATUS_TXCTL))
            usb_sendcontrolpacket(P_TYPE_REPORT);
    }

    usb_drvstatus |= USB_STATUS_RXDATA;       // AND SIGNAL THAT WE HAVE DATA AVAILABLE
}

// SENDING INTERRUPT ENDPOINT
inline void ep1_irqservice()
{
    *IR = RAWHID_TX_ENDPOINT;

    // STALL CONDITION
    if (*ESR & ESR_FSC) {
        *ESR = ESR_FSC;
        return;
    }

    if (usb_drvstatus & USB_STATUS_WAIT_FOR_ACK) {
        if (*ESR & ESR_TPS) {
            *ESR = ESR_TPS;
            usb_drvstatus &= ~USB_STATUS_WAIT_FOR_ACK;
        } else {
            // WAIT
            return;
        }
    }
}

// RECEIVING DATA ENDPOINT
void ep2_irqservice()
{
    // ONLY RESPOND ONCE EVERYTHING IS SETUP
    if(!(usb_drvstatus & USB_STATUS_CONFIGURED))
        return;

    *IR = RAWHID_RX_ENDPOINT;

    // STALL SENT
    if (*ESR & ESR_FSC) {
        *ESR = ESR_FSC;
        return;
    }

    // FLUSHED
    if (*ESR & ESR_FFS) {
        *ECR |= ECR_FLUSH;
        return;
    }

    // NOTHING TO RECEIVE?
    if(*ESR & ESR_RPS) {
        // WE HAVE A PACKET, GO PROCESS IT
        usb_ep2_receive();
        return;
    }
}

// GENERAL INTERRUPT SERVICE ROUTINE - DISPATCH TO INDIVIDUAL ENDPOINT ROUTINES
void usb_irqservice()
{
    if(!(usb_drvstatus & USB_STATUS_INIT))
        return;

    usb_drvstatus |= USB_STATUS_INSIDEIRQ;

    uint32_t eir = *EIR & 0x01ff;

    // FILTERING OUT NON-CRITICAL EVENTS
    // - SPEED DETECTION END
    // THAT SHOULDN'T END INTERRUPT HANDLING
    uint32_t ssr = *SSR & 0xfc87;

    if (ssr & SSR_HFSUSP) {
        // ENTER SUSPEND MODE
        usb_hwsuspend();
            *SSR = SSR_HFSUSP;
            goto out;
        } else if (ssr & SSR_HFRM) {
            // RESUME FROM SUSPEND MODE
            usb_hwresume();
            *SSR = SSR_HFRM;
            goto out;
        } else if (ssr & SSR_HFRES) {
            // RESET RECEIVED
            usb_reset();
            *SSR = SSR_HFRES;
            goto out;
        } else if (ssr) {
            *SSR = *SSR;    // CLEAR ALL ERROR CONDITIONS
            usb_drvstatus|= USB_STATUS_ERROR;
            goto out;
    }

    if (eir) {
        if(eir & 1) {
            ep0_irqservice();
            *EIR = 1;
        }

        if (eir & 2) {
            ep1_irqservice();
            *EIR = 2;
        }

        if (eir & 4) {
            ep2_irqservice();
            *EIR = 4;
        }
    }

    // MAYBE OUTGOING ENDPOINTS HAVE SOMETHING TO SEND
    usb_ep1_transmit();

out:
    usb_drvstatus &= ~USB_STATUS_INSIDEIRQ;
}

void usb_init_data_transfer()
{
    usb_ep1_transmit();
}
