/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef USB_H
#define USB_H

#include "newrpl_types.h"

// These 4 numbers identify your device.  Set these to
// something that is (hopefully) not used by any others!
#define VENDOR_ID		0x3f0    // VID of HP Inc.
#define PRODUCT_ID		0x121   // ORIGINAL VID/PID OF THE 50g/39gs/40g TARGET HARDWARE
//#define PRODUCT_ID        0x441   // PID OF THE Prime UPDATE MODE [NW280AA, G8X92AA] TARGET HARDWARE
//#define PRODUCT_ID        0x1541 // ORIGINAL PID OF Prime [NW280AA, G8X92AA] TARGET HARDWARE

// You can change these to give your code its own name.
#define STR_MANUFACTURER	{'n',0,'e',0,'w',0,'R',0,'P',0,'L',0,' ',0,'T',0,'e',0,'a',0,'m',0}
#define STR_MANUFLENGTH   22+2
#define STR_PRODUCT		{'n',0,'e',0,'w',0,'R',0,'P',0,'L',0,' ',0,'C',0,'a',0,'l',0,'c',0}
#define STR_PRODLENGTH   22+2

#define RAWHID_USAGE_PAGE	0xFFAB  // recommended: 0xFF00 to 0xFFFF
#define RAWHID_USAGE		0x0200  // recommended: 0x0100 to 0xFFFF

// These determine the bandwidth that will be allocated
// for your communication.  You do not need to use it
// all, but allocating more than necessary means reserved
// bandwidth is no longer available to other USB devices.
#define RAWHID_TX_SIZE		64      // transmit packet size
#define RAWHID_TX_INTERVAL	3       // max # of ms between transmit packets
#define RAWHID_RX_SIZE		64      // receive packet size
#define RAWHID_RX_INTERVAL	2       // max # of ms between receive packets

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
#define USB_STATUS_INSIDEIRQ            32      // DRIVER IS PROCESSING IRQ, POSSIBLE MESSING WITH HARDWARE
#define USB_STATUS_TXCTL                64      // THERE'S A CONTROL PACKET READY TO SEND
#define USB_STATUS_TXDATA               128     // THERE'S DATA READY TO BE SENT
#define USB_STATUS_RXCTL                256     // A CONTROL PACKET ARRIVED
#define USB_STATUS_RXDATA               512     // THERE'S DATA AVAILABLE TO READ
#define USB_STATUS_ERROR                1024    // THERE WAS SOME ERROR WITH THE RECEIVED DATA
#define USB_STATUS_HALT                 2048    // WE NEED TO TELL THE HOST TO STOP SENDING DATA
#define USB_STATUS_EOF                  4096    // READ UP TO END-OF-FILE
#define USB_STATUS_RXRCVD               8192
#define USB_STATUS_WAKEUPENABLED        16384
#define USB_STATUS_TESTMODE             32768
#define USB_STATUS_SUSPEND              65536
#define USB_STATUS_CONNECTNOW           (1<<17)
#define USB_STATUS_DISCONNECTNOW        (1<<18)
#define USB_STATUS_NOWAIT               (1<<19)
#define USB_STATUS_WAIT_FOR_ACK         (1<<20) // PACKET HAS BEEN SENT ON EP1, WAIT FOR ACK
#define USB_STATUS_SEND_ZERO_LENGTH_PACKET (1<<21) // SEND ZERO LENGTH PACKET NEXT

// MAXIMUM SIZE OF A BLOCK OF DATA, LARGER BLOCKS WILL BE SPLIT INTO MULTIPLE SMALLER BLOCKS
#define USB_DATASIZE      (RAWHID_TX_SIZE-8)

#define USB_TIMEOUT_MS     5000

// PACKET STRUCTURE
typedef struct
{
    BYTE p_type;
    BYTE p_dataused;
    BYTE p_fileidLSB;
    BYTE p_fileidMSB;
    BINT p_offset;
    BYTE p_data[56];
} USB_PACKET;

// MAKE A 16 BIT FILEID FROM ITS PARTS
#define P_FILEID(pptr)  (( (BINT)((pptr)->p_fileidLSB))|(((BINT)((pptr)->p_fileidMSB))<<8))

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

#define LONG_BUFFER_SIZE        3*32*RAWHID_RX_SIZE
#define RING_BUFFER_SIZE        ((int)sizeof(__usb_rxtxbuffer))

// NEW SIMPLIFIED GLOBALS

extern volatile BINT __usb_drvstatus;   // FLAGS TO INDICATE IF INITIALIZED, CONNECTED, SENDING/RECEIVING, ETC.

extern BINT __usb_fileid;       // CURRENT FILEID
extern BINT __usb_fileid_seq;   // SEQUENTIAL NUMBER TO MAKE FILEID UNIQUE
extern BINT __usb_offset;       // CURRENT OFFSET WITHIN THE FILE
extern WORD __usb_crc32;        // CURRENT CRC32 OF DATA RECEIVED
extern BYTE __usb_ctlbuffer[RAWHID_RX_SIZE + 1];        // BUFFER TO RECEIVE CONTROL PACKETS IN THE CONTROL CHANNEL
extern BYTE __usb_tmprxbuffer[RAWHID_RX_SIZE + 1];      // TEMPORARY BUFFER TO RECEIVE DATA
extern BYTE __usb_ctlrxbuffer[RAWHID_RX_SIZE + 1];      // TEMPORARY BUFFER TO RECEIVE CONTROL PACKETS
extern BYTE __usb_ctltxbuffer[RAWHID_TX_SIZE + 1];      // TEMPORARY BUFFER TO TRANSMIT DATA

extern BYTE __usb_rxtxbuffer[LONG_BUFFER_SIZE]; // LARGE BUFFER TO RECEIVE AT LEAST 3 FULL FRAGMENTS
extern BINT __usb_rxoffset;     // STARTING OFFSET OF THE DATA IN THE RX BUFFER
extern volatile BINT __usb_rxtxtop;     // NUMBER OF BYTES USED IN THE RX BUFFER
extern volatile BINT __usb_rxtxbottom;  // NUMBER OF BYTES IN THE RX BUFFER ALREADY READ BY THE USER
extern volatile BINT __usb_rxtotalbytes;        // TOTAL BYTES ON THE FILE, 0 MEANS DON'T KNOW YET

extern BINT __usb_txtotalbytes; // TOTAL BYTES ON THE FILE, 0 MEANS DON'T KNOW YET
extern BINT __usb_txseq;        // SEQUENTIAL NUMBER WITHIN A FRAGMENT OF DATA

extern BYTEPTR __usb_ctlbufptr; // POINTER TO BUFFER DURING CONTROL CHANNEL TRANSFERS
extern BINT __usb_ctlcount;     // COUNT OF DATA DURING CONTROL CHANNEL TRANSFERS
extern BINT __usb_ctlpadding;   // COUNT OF DATA DURING CONTROL CHANNEL TRANSFERS

#endif
