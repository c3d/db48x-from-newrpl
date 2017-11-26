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

#define RAWHID_TX_SIZE  64
#define RAWHID_RX_SIZE  64


// INITIALIZE USB SUBSYSTEM, POWER, PINS
#define USB_STATUS_INIT                 1
#define USB_STATUS_CONNECTED            2
#define USB_STATUS_CONFIGURED           4
#define USB_STATUS_EP0TX                8
#define USB_STATUS_EP0RX                16
#define USB_STATUS_HIDTX                32
#define USB_STATUS_HIDRX                64
#define USB_STATUS_DATAREADY            128
#define USB_STATUS_SUSPEND              512
#define USB_STATUS_WAKEUPENABLED        1024
#define USB_STATUS_TESTMODE             2048


// GLOBAL VARIABLES OF THE USB SUBSYSTEM
BINT __usb_drvstatus __SYSTEM_GLOBAL__; // FLAGS TO INDICATE IF INITIALIZED, CONNECTED, SENDING/RECEIVING, ETC.
BYTE *__usb_bufptr[3] __SYSTEM_GLOBAL__;   // POINTERS TO BUFFERS FOR EACH ENDPOINT (0/1/2)
BINT __usb_count[3]   __SYSTEM_GLOBAL__;   // REMAINING BYTES TO TRANSFER ON EACH ENDPOINT (0/1/2)
BINT __usb_padding[3] __SYSTEM_GLOBAL__;    // PADDING FOR OUTGOING TRANSFERS
BYTE __usb_tmpbuffer[RAWHID_TX_SIZE] __SYSTEM_GLOBAL__;  // TEMPORARY BUFFER FOR NON-BLOCKING CONTROL TRANSFERS
BYTEPTR __usb_rcvbuffer;
WORD __usb_rcvtotal __SYSTEM_GLOBAL__;
WORD __usb_rcvpartial __SYSTEM_GLOBAL__;
WORD __usb_rcvcrc __SYSTEM_GLOBAL__;


void usb_hwsetup()
{

}

void usb_hwsuspend()
{


}

void usb_hwresume()
{

}

void usb_irqservice();

void usb_init(int force)
{

    if(!force && (__usb_drvstatus&USB_STATUS_INIT)) return;

    __usb_drvstatus=USB_STATUS_INIT;

    usb_hwsetup();
}

void usb_shutdown()
{

    if(!__usb_drvstatus&USB_STATUS_INIT) return;

    if(__usb_drvstatus&USB_STATUS_DATAREADY) {
        usb_releasedata();
    }

    __usb_drvstatus=0;  // MARK UNCONFIGURED

}



inline void usb_checkpipe()
{

}







void usb_ep1_transmit(int newtransmission)
{

    if(!(__usb_drvstatus&USB_STATUS_CONNECTED)) return;

    if(newtransmission || (__usb_drvstatus&USB_STATUS_HIDTX)) {

    }


}

// TRANSMIT BYTES TO THE HOST IN EP2 ENDPOINT
// STARTS A NEW TRANSMISSION IF newtransmission IS TRUE, EVEN IF
// THERE ARE ZERO BYTES IN THE BUFFER
// FOR USE WITHIN ISR

void usb_ep2_receive(int newtransmission)
{

    if(!(__usb_drvstatus&USB_STATUS_CONNECTED)) return;

    if(newtransmission || (__usb_drvstatus&USB_STATUS_HIDRX)) {


    }

}


// SENDING INTERRUPT ENDPOINT
void ep1_irqservice()
{
}


// RECEIVING DATA ENDPOINT
void ep2_irqservice()
{
}


// GENERAL INTERRUPT SERVICE ROUTINE - DISPATCH TO INDIVIDUAL ENDPOINT ROUTINES
void usb_irqservice()
{



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


// HIGH LEVEL FUNCTION TO SEE IF THERE'S ANY DATA FROM THE USB DRIVER
int usb_hasdata()
{
    if(__usb_drvstatus&USB_STATUS_DATAREADY) return 1;
    return 0;
}

// HIGH LEVEL FUNCTION TO ACCESS A BLOCK OF DATA
BYTEPTR usb_accessdata(int *blksize)
{
    if(!(__usb_drvstatus&USB_STATUS_DATAREADY)) return 0;
    if(blksize) *blksize=__usb_rcvpartial;
    return __usb_rcvbuffer;
}

// HIGH LEVEL FUNCTION TO RELEASE A BLOCK OF DATA AND GET READY TO RECEIVE THE NEXT
void usb_releasedata()
{
    if(!(__usb_drvstatus&USB_STATUS_DATAREADY)) return;
    if(__usb_rcvbuffer!=__usb_tmpbuffer) simpfree(__usb_rcvbuffer);

    __usb_drvstatus&=~USB_STATUS_DATAREADY;
}
