/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include <libraries.h>
#include <ui.h>

INTERRUPT_TYPE cpu_intoff_nosave();

// OTHER EXTERNAL FUNCTIONS NEEDED
extern int cpu_getPCLK();

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

extern const BYTE const device_descriptor[];

#define CONFIG1_DESC_SIZE        (9+9+9+7+7)
#define RAWHID_HID_DESC_OFFSET   (9+9)
extern const BYTE const config1_descriptor[CONFIG1_DESC_SIZE];

struct usb_string_descriptor_struct
{
    BYTE bLength;
    BYTE bDescriptorType;
    BYTE wString[];
};
extern const struct usb_string_descriptor_struct const _usb_string0;
extern const struct usb_string_descriptor_struct const _usb_string1;

extern const struct usb_string_descriptor_struct const _usb_string2;

extern const struct descriptor_list_struct
{
    uint16_t wValue;
    uint16_t wIndex;
    const BYTE *addr;
    BYTE length;
} const descriptor_list[];

// MAKE SURE THIS MATCHES THE DEVICE DESCRIPTORES IN usbdriver.c
#define NUM_DESC_LIST 7 //((int)(sizeof(descriptor_list)/sizeof(struct descriptor_list_struct)))

// NEW SIMPLIFIED GLOBALS

extern volatile int32_t usb_drvstatus;   // FLAGS TO INDICATE IF INITIALIZED, CONNECTED, SENDING/RECEIVING, ETC.

extern int32_t usb_fileid;       // CURRENT FILEID
extern int32_t usb_fileid_seq;   // SEQUENTIAL NUMBER TO MAKE FILEID UNIQUE
extern int32_t usb_offset;       // CURRENT OFFSET WITHIN THE FILE
extern WORD usb_crc32;        // CURRENT CRC32 OF DATA RECEIVED
extern BYTE usb_ctlbuffer[RAWHID_RX_SIZE + 1];        // BUFFER TO RECEIVE CONTROL PACKETS IN THE CONTROL CHANNEL
extern BYTE usb_tmprxbuffer[RAWHID_RX_SIZE + 1];      // TEMPORARY BUFFER TO RECEIVE DATA
extern BYTE usb_ctlrxbuffer[RAWHID_RX_SIZE + 1];      // TEMPORARY BUFFER TO RECEIVE CONTROL PACKETS
extern BYTE usb_ctltxbuffer[RAWHID_TX_SIZE + 1];      // TEMPORARY BUFFER TO TRANSMIT DATA

extern BYTE usb_rxbuffer[LONG_BUFFER_SIZE];   // LARGE BUFFER TO RECEIVE AT LEAST 3 FULL FRAGMENTS
extern int32_t usb_rxoffset;     // STARTING OFFSET OF THE DATA IN THE RX BUFFER
extern volatile int32_t usb_rxtxtop;     // NUMBER OF BYTES USED IN THE RX BUFFER
extern volatile int32_t usb_rxtxbottom;  // NUMBER OF BYTES IN THE RX BUFFER ALREADY READ BY THE USER
extern volatile int32_t usb_rxtotalbytes;        // TOTAL BYTES ON THE FILE, 0 MEANS DON'T KNOW YET

extern int32_t usb_txtotalbytes; // TOTAL BYTES ON THE FILE, 0 MEANS DON'T KNOW YET
extern int32_t usb_txseq;        // SEQUENTIAL NUMBER WITHIN A FRAGMENT OF DATA

extern BYTEPTR usb_ctlbufptr; // POINTER TO BUFFER DURING CONTROL CHANNEL TRANSFERS
extern int32_t usb_ctlcount;     // COUNT OF DATA DURING CONTROL CHANNEL TRANSFERS
extern int32_t usb_ctlpadding;   // COUNT OF DATA DURING CONTROL CHANNEL TRANSFERS

//  END OF NEW GLOBALS
// ********************************

// SOME INTERNAL FORWARD DECLARATIONS
ARM_MODE void usbram_sendcontrolpacket(int packet_type);

extern const WORD const crctable[256];

// CALCULATE THE STANDARD CRC32 OF A BLOCK OF DATA
#define RAM_CRCTABLE RReg[9].data

ARM_MODE WORD ramusb_crc32roll(WORD oldcrc, BYTEPTR data, int32_t len)
{
    WORD crc = oldcrc ^ 0xffffffff;
    while(len--)
        crc = RAM_CRCTABLE[(crc ^ *data++) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xffffffff;
}

ARM_MODE void rammemmoveb(void *_dest, const void *_source, int nbytes)
{
    register char *dest = (char *)_dest;
    register char *source = (char *)_source;
    register int going = (_dest > _source) ? (-1) : (1);
    if(going == -1) {
        dest += nbytes - 1;
        source += nbytes - 1;
    }

    while(nbytes--) {
        *dest = *source;
        dest += going;
        source += going;
    }
}

// PUT THE CPU IN "DOZE" MODE
ARM_MODE void ramcpu_waitforinterrupt()
{
    register unsigned int var = 0;
    asm volatile ("mcr p15,0,%0,c7,c0,4"::"r" (var));
}

ARM_MODE void ramusb_hwsetup()
{

    *CLKCON &= ~0x40;   // POWER DOWN USB HOST TO MAKE SURE HOST AND DEVICE AREN'T WORKING AT ONCE
    *CLKCON |= 0x80;    // POWER UP USB DEVICE

    *UPLLCON = 0x78023; // 48 MHZ CLOCK
    *CLKSLOW &= ~0x80;  // MAKE SURE UPLL IS ON

    *MISCCR &= ~(USBSUSPND0 | USBSUSPND1 | USBPAD);     // SET DEVICE MODE, CHANGE TO NORMAL MODE

    // DEBUG: FOR NOW DON'T ALLOW SUSPEND
    *PWR_REG = 0;       // ALLOW THE DEVICE TO ENTER SUSPEND MODE

    *FUNC_ADDR_REG = 0x80;      // RESET TO DEFAULT ADDRESS

    *INDEX_REG = 0;     // SETUP ENDPOINT0
    *MAXP_REG = 1;      // USE 8-BYTE PACKETS
    *MAXP_REG2 = 1;     // USE 8-BYTE PACKETS
    *EP0_CSR = 0xc0;    // CLEAR ANYTHING PENDING

    *INDEX_REG = 1;
    *MAXP_REG = 8;      // USE 64-BYTE PACKETS ON EP1
    *MAXP_REG2 = 8;     // USE 64-BYTE PACKETS ON EP1
    *IN_CSR1_REG = 0x48;        // CLR_DATA TOGGLE + FIFO_FLUSH
    *IN_CSR2_REG = 0X20;        // CONFIGURE AS IN ENDPOINT
    *OUT_CSR1_REG = 0x80;       // SET CLR_DATA_TOGGLE
    *OUT_CSR2_REG = 0;
    *INDEX_REG = 2;
    *MAXP_REG = 8;      // USE 64-BYTE PACKETS ON EP2
    *MAXP_REG2 = 8;     // USE 64-BYTE PACKETS ON EP2
    *IN_CSR1_REG = 0x48;        // CLR_DATA TOGGLE + FIFO_FLUSH
    *IN_CSR2_REG = 0;   // CONFIGURE AS OUT ENDPOINT
    *OUT_CSR1_REG = 0x80;       // SET CLR_DATA_TOGGLE
    *OUT_CSR2_REG = 0;

    // SET WHICH INTERRUPTS WE WANT
    *USB_INT_EN_REG = 0x7;      // ENABLE RESET, RESUME AND SUSPEND INTERRUPTS
    *EP_INT_EN_REG = 0x7;       // ENABLE ONLY EP0, EP1 AND EP2 INTERRUPTS
    *USB_INT_REG = 0x7; // CLEAR ALL PENDING INTERRUPTS
    *EP_INT_REG = 0x1f; // CLEAR ALL PENDING INTERRUPTS

    // SETUP CABLE DISCONNECT DETECTION
    *HWREG(IO_REGS, 0x50) = (*HWREG(IO_REGS, 0x50) & (~0xc)) | 0x8;     // GPF1 SET TO EINT1
    *HWREG(IO_REGS, 0x88) = (*HWREG(IO_REGS, 0x88) & (~0x70)) | 0x20;   // CHANGE TO FALLING EDGE TRIGGERED

}

ARM_MODE void ramusb_hwsuspend()
{

    *MISCCR |= (USBSUSPND0 | USBSUSPND1);       // CHANGE TO SUSPEND MODE

    *CLKSLOW |= 0x80;   // TURN OFF UPLL

}

ARM_MODE void ramusb_hwresume()
{
    *UPLLCON = 0x78023; // 48 MHZ CLOCK
    *CLKSLOW &= ~0x80;  // MAKE SURE UPLL IS ON

    *MISCCR &= ~(USBSUSPND0 | USBSUSPND1);      // CHANGE TO NORMAL MODE

}

ARM_MODE void ramusb_irqservice();
ARM_MODE void ramusb_sendcontrolpacket(int packet_type);
ARM_MODE void ramusb_receivecontrolpacket();

// TRANSMIT BYTES TO THE HOST IN EP0 ENDPOINT
// STARTS A NEW TRANSMISSION IF newtransmission IS TRUE, EVEN IF
// THERE ARE ZERO BYTES IN THE BUFFER
// FOR USE WITHIN ISR

ARM_MODE void ramusb_ep0_transmit(int newtransmission)
{

    if(!(usb_drvstatus & USB_STATUS_CONNECTED))
        return;

    if(newtransmission || (usb_drvstatus & USB_STATUS_EP0TX)) {

        *INDEX_REG = 0;

        if((*EP0_CSR) & EP0_IN_PKT_RDY) {
            // PREVIOUS PACKET IS STILL BEING SENT, DON'T PUSH IT
            usb_drvstatus |= USB_STATUS_EP0TX;        // AND KEEP TRANSMITTING
            return;
        }

        int cnt = 0;
        while(usb_ctlcount && (cnt < EP0_FIFO_SIZE)) {
            *EP0_FIFO = (WORD) * usb_ctlbufptr;
            ++usb_ctlbufptr;
            ++cnt;
            --usb_ctlcount;
        }

        if(usb_ctlcount == 0) {
            // SEND ANY PADDING
            while((usb_ctlpadding != 0) && (cnt < EP0_FIFO_SIZE)) {
                *EP0_FIFO = 0;
                ++cnt;
                --usb_ctlpadding;
            }
        }

        if((usb_ctlcount == 0) && (usb_ctlpadding == 0)) {
            *EP0_CSR |= EP0_IN_PKT_RDY | EP0_DATA_END;  // SEND THE LAST PACKET

            //usb_intdata[usb_intcount++]=0xEEEE0000 | cnt;

            usb_drvstatus &= ~USB_STATUS_EP0TX;
        }
        else {
            *EP0_CSR |= EP0_IN_PKT_RDY; // SEND PART OF THE BUFFER
            //usb_intdata[usb_intcount++]=0xAAAA0000 | cnt;
            usb_drvstatus |= USB_STATUS_EP0TX;        // AND KEEP TRANSMITTING
        }
    }

}

// TRANSMIT BYTES TO THE HOST IN EP0 ENDPOINT
// STARTS A NEW TRANSMISSION IF newtransmission IS TRUE, EVEN IF
// THERE ARE ZERO BYTES IN THE BUFFER
// FOR USE WITHIN ISR

ARM_MODE void ramusb_ep0_receive(int newtransmission)
{

    if(!(usb_drvstatus & USB_STATUS_CONNECTED))
        return;

    if(newtransmission || (usb_drvstatus & USB_STATUS_EP0RX)) {

        *INDEX_REG = 0;

        if(!((*EP0_CSR) & EP0_OUT_PKT_RDY)) {
            // PREVIOUS PACKET IS STILL BEING SENT, DON'T PUSH IT
            usb_drvstatus |= USB_STATUS_EP0RX;        // AND KEEP TRANSMITTING
            return;
        }

        int cnt = 0;
        while(usb_ctlcount && (cnt < EP0_FIFO_SIZE)) {
            *usb_ctlbufptr = (BYTE) * EP0_FIFO;
            ++usb_ctlbufptr;
            --usb_ctlcount;
            ++cnt;
        }

        if(usb_ctlcount == 0) {
            *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY | EP0_DATA_END;        // RECEIVED THE LAST PACKET
            usb_drvstatus &= ~USB_STATUS_EP0RX;
        }
        else {
            *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY;       // RECIEVED PART OF THE BUFFER
            usb_drvstatus |= USB_STATUS_EP0RX;        // AND KEEP RECEIVING MORE
        }
    }

}

ARM_MODE void ramusb_checkpipe()
{
    if((*EP0_CSR) & EP0_SETUP_END) {
        // SOMETHING HAPPENED, CLEAR THE CONDITION TO ALLOW RETRY
        *EP0_CSR |= EP0_SERVICED_SETUP_END;
        // CANCEL ALL ONGOING TRANSMISSIONS
        usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);

    }
    if((*EP0_CSR) & EP0_SENT_STALL) {
        // CLEAR ANY PREVIOUS STALL CONDITION
        *EP0_CSR = 0;   // CLEAR SEND_STALL AND SENT_STALL SIGNALS
        // CANCEL ALL ONGOING TRANSMISSIONS
        usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
        // AND CONTINUE PROCESSING ANY OTHER INTERRUPTS
    }

}

ARM_MODE void ramep0_irqservice()
{
    *INDEX_REG = 0;     // SELECT ENDPOINT 0

    ramusb_checkpipe();

    if(usb_drvstatus & USB_STATUS_EP0TX) {

        ramusb_ep0_transmit(0);
        ramusb_checkpipe();

        return;
    }

    if((*EP0_CSR) & EP0_OUT_PKT_RDY) {

        // PROCESS FIRST ANY ONGOING TRANSMISSIONS
        if(usb_drvstatus & USB_STATUS_EP0RX) {
            ramusb_ep0_receive(0);
            ramusb_checkpipe();
            return;
        }

        // WE HAVE A PACKET
        int32_t reqtype;
        int32_t request;
        int32_t value;
        int32_t index;
        int32_t length;

        // READ ALL 8 BYTES FROM THE FIFO

        reqtype = *EP0_FIFO;
        request = *EP0_FIFO;
        value = *EP0_FIFO;
        value |= (*EP0_FIFO) << 8;
        index = *EP0_FIFO;
        index |= (*EP0_FIFO) << 8;
        length = *EP0_FIFO;
        length |= (*EP0_FIFO) << 8;

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
                        usb_ctlbufptr = (BYTEPTR) descriptor_list[k].addr;
                        if(length < descriptor_list[k].length) {
                            usb_ctlcount = length;
                            usb_ctlpadding = 0;
                        }
                        else {
                            usb_ctlcount = descriptor_list[k].length;
                            usb_ctlpadding =
                                    length - descriptor_list[k].length;
                        }
                        *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY;
                        ramusb_ep0_transmit(1); // SEND 0-DATA STATUS STAGE
                        ramusb_checkpipe();
                        return;
                    }
                }

                // SPECIAL CASE - CALCULATOR SERIAL NUMBER STRING
                if((0x0303 == value) && (0x0409 == index)) {
                    // FOUND THE REQUESTED DESCRIPTOR
                    usb_ctlbufptr = usb_ctlbuffer;
                    usb_ctlbuffer[0] = 20 + 2;
                    usb_ctlbuffer[1] = 3;

                    // COPY THE SERIAL NUMBER - EXPAND ASCII TO UTF-16
                    int n;
                    BYTEPTR ptr = (BYTEPTR) SERIAL_NUMBER_ADDRESS;
                    for(n = 0; n < 10; ++n, ++ptr) {
                        usb_ctlbuffer[2 + 2 * n] = *ptr;
                        usb_ctlbuffer[3 + 2 * n] = 0;
                    }

                    if(length < usb_ctlbuffer[0]) {
                        usb_ctlcount = length;
                        usb_ctlpadding = 0;
                    }
                    else {
                        usb_ctlcount = usb_ctlbuffer[0];
                        usb_ctlpadding = length - usb_ctlbuffer[0];
                    }
                    *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY;
                    ramusb_ep0_transmit(1);     // SEND 0-DATA STATUS STAGE
                    ramusb_checkpipe();
                    return;
                }

                // DON'T KNOW THE ANSWER TO THIS
                usb_ctlcount = 0;
                usb_ctlpadding = length;      // SEND THE DATA AS REQUESTED, STALL AT THE END
                *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY;
                ramusb_ep0_transmit(1); // SEND DATA STATUS STAGE
                ramusb_checkpipe();

                return;
            }
            case SET_ADDRESS:
            {
                usb_ctlcount = 0;
                usb_ctlpadding = 0;
                usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
                *FUNC_ADDR_REG = value | 0x80;
                *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY | EP0_DATA_END;
                ramusb_checkpipe();
                return;
            }
            case SET_CONFIGURATION:
            {
                // OUR DEVICE HAS ONE SINGLE CONFIGURATION AND IS SETUP
                // ON WAKEUP, SO NOTHING TO DO HERE BUT ACKNOWLEDGE

                if(value)
                    usb_drvstatus |= USB_STATUS_CONFIGURED;
                else
                    usb_drvstatus &= ~USB_STATUS_CONFIGURED;
                usb_ctlcount = 0;
                usb_ctlpadding = 0;
                usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
                *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY | EP0_DATA_END;
                ramusb_checkpipe();

                return;
            }
            case GET_CONFIGURATION:
            {
                int32_t configvalue =
                        (usb_drvstatus & USB_STATUS_CONFIGURED) ? 1 : 0;
                *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY;
                usb_ctlcount = 1;
                usb_ctlpadding = 0;
                usb_ctlbufptr = usb_ctlbuffer;
                usb_ctlbuffer[0] = configvalue;
                ramusb_ep0_transmit(1); // SEND DATA STATUS STAGE
                ramusb_checkpipe();

                return;
            }
            case GET_STATUS:
            {

                usb_ctlbuffer[0] = usb_ctlbuffer[1] = 0;
                switch (reqtype) {
                case 0x80:     // DEVICE GET STATUS
                    *(usb_ctlbufptr) =
                            (usb_drvstatus & USB_STATUS_WAKEUPENABLED) ? 2 :
                            0;
                    break;
                case 0x82:     // ENDPONT GET STATUS
                    *INDEX_REG = index & 0x7;
                    if((index & 7) == 0) {
                        *(usb_ctlbufptr) =
                                ((*EP0_CSR) & EP0_SEND_STALL) ? 1 : 0;
                    }
                    else {
                        *(usb_ctlbufptr) |=
                                ((*OUT_CSR1_REG) & EPn_OUT_SEND_STALL) ? 1 : 0;
                        *(usb_ctlbufptr) |=
                                ((*IN_CSR1_REG) & EPn_IN_SEND_STALL) ? 1 : 0;
                    }
                    break;
                }

                // FOR NOW SEND THE DATA
                *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY;
                usb_ctlcount = 2;
                usb_ctlpadding = 0;
                usb_ctlbufptr = usb_ctlbuffer;

                ramusb_ep0_transmit(1); // SEND DATA STATUS STAGE
                ramusb_checkpipe();

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
                        *INDEX_REG = endp;
                        if(endp != 0)   // DO NOT STALL THE CONTROL ENDPOINT
                        {
                            *OUT_CSR1_REG |= EPn_OUT_SEND_STALL;
                            *IN_CSR1_REG |= EPn_IN_SEND_STALL;
                        }
                    }
                    break;
                }

                usb_ctlcount = 0;
                usb_ctlpadding = 0;
                usb_drvstatus &= ~USB_STATUS_EP0RX | USB_STATUS_EP0TX;
                *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY | EP0_DATA_END;
                ramusb_checkpipe();

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
                        *INDEX_REG = endp;
                        if(endp == 1)   // DO NOT STALL THE CONTROL ENDPOINT
                        {
                            *IN_CSR1_REG |=
                                    EPn_IN_FIFO_FLUSH | EPn_IN_CLR_DATA_TOGGLE;
                            *IN_CSR1_REG &=
                                    ~(EPn_IN_SEND_STALL |
                                    EPn_IN_CLR_DATA_TOGGLE);
                        }
                        if(endp == 2) {
                            *OUT_CSR1_REG |=
                                    EPn_OUT_FIFO_FLUSH |
                                    EPn_OUT_CLR_DATA_TOGGLE;
                            *OUT_CSR1_REG &=
                                    ~(EPn_OUT_SEND_STALL |
                                    EPn_OUT_CLR_DATA_TOGGLE);
                        }
                    }
                    break;
                }

                usb_ctlcount = 0;
                usb_ctlpadding = 0;
                usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
                *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY | EP0_DATA_END;
                ramusb_checkpipe();

                return;
            }

            }
            // UNKNOWN STANDARD REQUEST??
            // DON'T KNOW THE ANSWER TO THIS BUT KEEP THE PIPES RUNNING
            if((reqtype & USB_DIRECTION) == USB_DEV_TO_HOST) {
                // SEND THE RIGHT AMOUNT OF ZEROS TO THE HOST
                usb_ctlcount = 0;
                usb_ctlpadding = length;
                *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY;
                ramusb_ep0_transmit(1);
                ramusb_checkpipe();

                return;
            }

            // THIS IS AN INCOMING REQUEST WITH NO DATA STAGE

            // READ ANY EXTRA DATA TO KEEP THE FIFO CLEAN
            while(length > 0) {
                usb_ctlbuffer[0] = *EP0_FIFO;
                --length;
            }

            usb_ctlcount = 0;
            usb_ctlpadding = 0;
            usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
            *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY | EP0_DATA_END;
            ramusb_checkpipe();

            return;

        }

        if((reqtype & 0x61) == 0x21)    // CLASS INTERFACE REQUESTS
        {

            if(index == RAWHID_INTERFACE) {
                switch (request) {
                case HID_SET_REPORT:
                    // GET DATA FROM HOST
                    *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY;
                    usb_ctlcount = RAWHID_TX_SIZE;
                    usb_ctlpadding = 0;
                    usb_ctlbufptr = usb_ctlbuffer;  // FOR NOW, LET'S SEE WHAT TO DO WITH THIS LATER
                    ramusb_ep0_receive(1);
                    return;
                case HID_GET_REPORT:
                    // SEND DATA TO HOST - SEND ALL ZEROS
                    *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY;
                    usb_ctlcount = 1;
                    usb_ctlpadding = RAWHID_TX_SIZE - 1;
                    usb_ctlbuffer[0] =
                            (usb_drvstatus & USB_STATUS_RXDATA) ? 1 : 0;
                    usb_ctlbufptr = usb_ctlbuffer;  // SEND THE STATUS
                    ramusb_ep0_transmit(1);
                    ramusb_checkpipe();

                    return;

                case HID_SET_IDLE:
                    // SEND DATA TO HOST - SEND ALL ZEROS
                    usb_ctlcount = 0;
                    usb_ctlpadding = 0;
                    usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
                    *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY | EP0_DATA_END;
                    ramusb_checkpipe();
                    return;

                }

            }
            // UNKNOWN CLASS REQUEST??
            // DON'T KNOW THE ANSWER TO THIS
            if((reqtype & USB_DIRECTION) == USB_DEV_TO_HOST) {
                // SEND THE RIGHT AMOUNT OF ZEROS TO THE HOST
                usb_ctlcount = 0;
                usb_ctlpadding = length;
                *EP0_CSR |= EP0_SEND_STALL | EP0_SERVICED_OUT_PKT_RDY;
                ramusb_ep0_transmit(1);
                ramusb_checkpipe();

                return;
            }
            // READ ANY EXTRA DATA TO KEEP THE FIFO CLEAN
            if(length > EP0_FIFO_SIZE)
                length = EP0_FIFO_SIZE;
            while(length > 0) {
                usb_ctlbuffer[0] = *EP0_FIFO;
                --length;
            }
            usb_ctlcount = 0;
            usb_ctlpadding = 0;
            usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
            *EP0_CSR |=
                    EP0_SEND_STALL | EP0_SERVICED_OUT_PKT_RDY | EP0_DATA_END;
            ramusb_checkpipe();
            return;

        }

        // ADD OTHER REQUESTS HERE

        if((reqtype & USB_DIRECTION) == USB_DEV_TO_HOST) {
            // SEND THE RIGHT AMOUNT OF ZEROS TO THE HOST
            usb_ctlcount = 0;
            usb_ctlpadding = length;
            *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY;
            ramusb_ep0_transmit(1);
            ramusb_checkpipe();

            return;
        }
        // READ ANY EXTRA DATA TO KEEP THE FIFO CLEAN
        if(length > EP0_FIFO_SIZE)
            length = EP0_FIFO_SIZE;
        while(length > 0) {
            usb_ctlbuffer[0] = *EP0_FIFO;
            --length;
        }
        usb_ctlcount = 0;
        usb_ctlpadding = 0;
        usb_drvstatus &= ~(USB_STATUS_EP0RX | USB_STATUS_EP0TX);
        *EP0_CSR |= EP0_SERVICED_OUT_PKT_RDY | EP0_DATA_END;
        ramusb_checkpipe();

        return;

    }

}

// DRIVER - PACKET TRANSMISSION CALLED BY IRQ - NEVER CALLED BY USER
// SEND A CONTROL PACKET IF ONE AVAILABLE, OR ONE DATA PACKET IF AVAILABLE
ARM_MODE void ramusb_ep1_transmit()
{

    if(!(usb_drvstatus & USB_STATUS_CONNECTED))
        return;

    *INDEX_REG = RAWHID_TX_ENDPOINT;

    if((*IN_CSR1_REG) & EPn_IN_PKT_RDY) {
        // PREVIOUS PACKET IS STILL BEING SENT, DON'T PUSH IT
        // WAIT FOR THE NEXT INTERRUPT
        return;
    }

    if(usb_drvstatus & USB_STATUS_TXCTL) {
        // WE HAVE A CONTROL PACKET READY TO GO IN THE CONTROL BUFFER

        int cnt;
        for(cnt = 0; cnt < RAWHID_TX_SIZE; ++cnt)
            *EP1_FIFO = (WORD) usb_ctltxbuffer[cnt];

        *IN_CSR1_REG |= EPn_IN_PKT_RDY; // SEND THE PACKET

        usb_drvstatus &= ~USB_STATUS_TXCTL;
        return;
    }

    if(usb_drvstatus & USB_STATUS_TXDATA) {
        // WE HAVE A DATA PACKET TO SEND

        if(usb_drvstatus & USB_STATUS_HALT) {
            // REMOTE REQUESTED WE STOP SENDING DATA UNTIL IT PROCESSES IT
            // JUST REPLY WITH A ZERO DATA PACKET
            *IN_CSR1_REG |= EPn_IN_PKT_RDY;
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
                int oldestdata = usb_rxtxbottom - usb_rxtxtop;
                if(oldestdata < 0)
                    oldestdata += RING_BUFFER_SIZE;
                if((bufoff < 0) || (bufoff > oldestdata)) {
                    // WE DON'T HAVE THAT DATA STORED ANYMORE, ABORT THE FILE
                    ramusb_sendcontrolpacket(P_TYPE_ABORT);
                    usb_fileid = 0;
                    usb_offset = 0;
                    usb_crc32 = 0;
                    usb_rxtxtop = usb_rxtxbottom = 0;

                    usb_drvstatus &= ~USB_STATUS_TXDATA;
                    usb_drvstatus |= USB_STATUS_ERROR;

                    int cnt;
                    for(cnt = 0; cnt < RAWHID_TX_SIZE; ++cnt)
                        *EP1_FIFO = (WORD) usb_ctltxbuffer[cnt];

                    *IN_CSR1_REG |= EPn_IN_PKT_RDY;     // SEND THE PACKET

                    usb_drvstatus &= ~USB_STATUS_TXCTL;
                    return;
                }

                // ADJUST THE RING'S POSITION TO SEND THE RIGHT DATA
                usb_rxtxbottom -= bufoff;
                if(usb_rxtxbottom < 0)
                    usb_rxtxbottom += RING_BUFFER_SIZE;
                usb_offset = usb_rxoffset;
            }
            usb_txseq = 0;    // RESTART BACK THE SEQUENCE NUMBER
            usb_crc32 = 0;    // RESET THE CRC FROM HERE ON
            usb_drvstatus &= ~USB_STATUS_ERROR;       // REMOVE THE ERROR AND RESEND

        }

        int bufbytes;
        int p_type;
        int eof = 0;

        bufbytes = usb_rxtxtop - usb_rxtxbottom;
        if(bufbytes < 0)
            bufbytes += RING_BUFFER_SIZE;

        if(bufbytes > USB_DATASIZE)
            bufbytes = USB_DATASIZE;    // DON'T SEND MORE THAN ONE PACKET AT A TIME

        // CHECK IF THESE ARE THE LAST FEW BYTES OF THE FILE
        if((int)usb_txtotalbytes - (int)usb_offset == bufbytes)
            eof = 1;
        else {
            if(bufbytes < USB_DATASIZE) {
                // WAIT FOR MORE DATA TO FILL UP THE PACKET, NO NEED TO SEND IT NOW
                // JUST REPLY WITH A ZERO DATA PACKET
                *IN_CSR1_REG |= EPn_IN_PKT_RDY;
                return;
            }
        }

        p_type = usb_txseq + 1;
        if(eof)
            p_type |= 0x40;
        if(p_type == 32)
            p_type = 0x40;

        // SEND A FULL PACKET

        *EP1_FIFO = p_type;
        *EP1_FIFO = bufbytes;
        *EP1_FIFO = usb_fileid & 0xff;
        *EP1_FIFO = (usb_fileid >> 8) & 0xff;
        *EP1_FIFO = usb_offset & 0xff;
        *EP1_FIFO = (usb_offset >> 8) & 0xff;
        *EP1_FIFO = (usb_offset >> 16) & 0xff;
        *EP1_FIFO = (usb_offset >> 24) & 0xff;

        int cnt;
        for(cnt = 0; cnt < USB_DATASIZE; ++cnt) {
            if(cnt >= bufbytes)
                *EP1_FIFO = 0;
            else {
                *EP1_FIFO = usb_rxtxbuffer[usb_rxtxbottom];
                usb_crc32 = ramusb_crc32roll(usb_crc32, usb_rxtxbuffer + usb_rxtxbottom, 1);    // UPDATE THE CRC32
                ++usb_rxtxbottom;
                if(usb_rxtxbottom >= RING_BUFFER_SIZE)
                    usb_rxtxbottom -= RING_BUFFER_SIZE;
            }
        }

        *IN_CSR1_REG |= EPn_IN_PKT_RDY; // SEND THE PACKET

        usb_offset += bufbytes;
        usb_txseq = p_type & 0x1f;

        if(eof) {
            ramusb_sendcontrolpacket(P_TYPE_ENDOFFILE);
            usb_drvstatus &= ~USB_STATUS_TXDATA;

            // DONE SENDING ALL DATA

        }
        else {
            // AT EACH END OF FRAGMENT, SEND A CHECKPOINT BUT NOT AT END OF FILE
            if(p_type & 0x40)
                ramusb_sendcontrolpacket(P_TYPE_CHECKPOINT);

            // IF WE CONSUMED ALL THE BUFFER, SIGNAL THAT WE ARE DONE
            if(usb_rxtxtop == usb_rxtxbottom)
                usb_drvstatus &= ~USB_STATUS_TXDATA;

        }

        return;
    }

    // NOTHING TO TRANSMIT

    // JUST REPLY WITH A ZERO DATA PACKET
    *IN_CSR1_REG |= EPn_IN_PKT_RDY;
}

// RECEIVE BYTES FROM THE HOST IN EP2 ENDPOINT
// FOR USE WITHIN ISR

ARM_MODE void ramusb_ep2_receive()
{

    if(!(usb_drvstatus & USB_STATUS_CONNECTED))
        return;

    *INDEX_REG = RAWHID_RX_ENDPOINT;

    if(!((*OUT_CSR1_REG) & EPn_OUT_PKT_RDY)) {
        // THERE'S NO PACKETS AVAILABLE
        return;
    }

    int fifocnt = (*OUT_FIFO_CNT1_REG) + 256 * (*OUT_FIFO_CNT2_REG);
    int cnt = 0;

    if(fifocnt < 8) {
        // FLUSH THE FIFO, IGNORE THE MALFORMED PACKET
        BYTE sum = 0;
        while(cnt < fifocnt) {
            sum += (BYTE) * EP2_FIFO;
            ++cnt;
        }

        *OUT_CSR1_REG &= ~EPn_OUT_PKT_RDY;      // RECEIVED THE PACKET
        return;
    }

    // READ PACKET TYPE
    int p_type = *EP2_FIFO;
    BYTEPTR rcvbuf;

    if(p_type & 0x80) {
        // THIS IS A CONTROL PACKET
        // PUT IT IN THE CTL BUFFER AND NOTIFY THE USER
        usb_drvstatus |= USB_STATUS_RXCTL;

        rcvbuf = usb_ctlrxbuffer;
        cnt = 1;
        rcvbuf[0] = p_type;
        ++rcvbuf;
        while(cnt < fifocnt) {
            *rcvbuf = (BYTE) * EP2_FIFO;
            ++cnt;
            ++rcvbuf;
        }

        *OUT_CSR1_REG &= ~EPn_OUT_PKT_RDY;      // RECEIVED THE PACKET

        ramusb_receivecontrolpacket();
        return;

    }

    // THIS IS A DATA PACKET
    // READ DATA DIRECTLY INTO THE BUFFER
    rcvbuf = usb_tmprxbuffer;
    cnt = 1;
    rcvbuf[0] = p_type;
    ++rcvbuf;
    while(cnt < 8) {
        *rcvbuf = (BYTE) * EP2_FIFO;
        ++cnt;
        ++rcvbuf;
    }

    USB_PACKET *pptr = (USB_PACKET *) usb_tmprxbuffer;

    // IS IT FROM THE SAME FILE?
    if(P_FILEID(pptr) != usb_fileid) {
        // DIFFERENT FILE? IGNORE
        // FLUSH THE FIFO, IGNORE THE PACKET
        BYTE sum = 0;
        while(cnt < fifocnt) {
            sum += (BYTE) * EP2_FIFO;
            ++cnt;
        }

        *OUT_CSR1_REG &= ~EPn_OUT_PKT_RDY;      // RECEIVED THE PACKET
        return;

    }

    // IS THE CORRECT OFFSET?
    if(pptr->p_offset != usb_offset) {
        //  WE MUST'VE MISSED SOMETHING
        if(!(usb_drvstatus & USB_STATUS_ERROR)) {
            usb_drvstatus |= USB_STATUS_ERROR;
            // SEND A REPORT NOW IF POSSIBLE, OTHERWISE THE ERROR INFO WILL GO IN THE NEXT REPORT
            if(!(usb_drvstatus & USB_STATUS_TXCTL))
                ramusb_sendcontrolpacket(P_TYPE_REPORT);
        }
        // FLUSH THE FIFO, IGNORE THE PACKET
        BYTE sum = 0;
        while(cnt < fifocnt) {
            sum += (BYTE) * EP2_FIFO;
            ++cnt;
        }

        *OUT_CSR1_REG &= ~EPn_OUT_PKT_RDY;      // RECEIVED THE PACKET
        return;

    }
    else {
        // GOT THE RIGHT OFFSET, RESET ERROR CONDITION IF ANY
        if(usb_drvstatus & USB_STATUS_ERROR) {
            usb_crc32 = 0;
            usb_drvstatus &= ~USB_STATUS_ERROR;
        }
    }

    // DO WE HAVE ENOUGH ROOM AVAILABLE?
    int usedspace = usb_rxtxtop - usb_rxtxbottom;
    if(usedspace < 0)
        usedspace += RING_BUFFER_SIZE;

    if(pptr->p_dataused > RING_BUFFER_SIZE - usedspace) {
        // DATA WON'T FIT IN THE BUFFER DUE TO OVERFLOW, ISSUE AN ERROR AND REQUEST RESEND
        usb_drvstatus |= USB_STATUS_ERROR;
        // SEND A REPORT NOW IF POSSIBLE, OTHERWISE THE ERROR INFO WILL GO IN THE NEXT REPORT
        if(!(usb_drvstatus & USB_STATUS_TXCTL))
            ramusb_sendcontrolpacket(P_TYPE_REPORT);

        // FLUSH THE FIFO, IGNORE THE PACKET
        BYTE sum = 0;
        while(cnt < fifocnt) {
            sum += (BYTE) * EP2_FIFO;
            ++cnt;
        }

        *OUT_CSR1_REG &= ~EPn_OUT_PKT_RDY;      // RECEIVED THE PACKET
        return;

    }

    // WE HAVE NEW DATA, RECEIVE IT DIRECTLY AT THE BUFFER
    rcvbuf = usb_rxtxbuffer + usb_rxtxtop;

    while((cnt < fifocnt) && (cnt < pptr->p_dataused + 8)) {
        *rcvbuf = (BYTE) * EP2_FIFO;
        // UPDATE THE CRC
        usb_crc32 = ramusb_crc32roll(usb_crc32, rcvbuf, 1);
        ++cnt;
        ++rcvbuf;
        if(rcvbuf == usb_rxtxbuffer + RING_BUFFER_SIZE)
            rcvbuf -= RING_BUFFER_SIZE;
    }

    // AND FLUSH ANY UNUSED BYTES
    BYTE sum = 0;
    while(cnt < fifocnt) {
        sum += (BYTE) * EP2_FIFO;
        ++cnt;
    }

    *OUT_CSR1_REG &= ~EPn_OUT_PKT_RDY;  // RECEIVED THE PACKET

    // UPDATE THE BUFFERS
    usb_rxtxtop += pptr->p_dataused;
    if(usb_rxtxtop >= RING_BUFFER_SIZE)
        usb_rxtxtop -= RING_BUFFER_SIZE;
    usb_offset += pptr->p_dataused;
    usedspace += pptr->p_dataused;

    // FOR FIRMWARE UPDATE, WE KNOW THE BUFFER IS LARGER THAN THE 4KBYTE BLOCKS BEING TRANSFERRED, SO WE NEVER NEED TO HALT
    /*
       if(usedspace>=RING_BUFFER_SIZE/2) {
       usb_drvstatus|=USB_STATUS_HALT;  // REQUEST HALT IF BUFFER IS HALF-FULL
       // SEND A REPORT NOW IF POSSIBLE, OTHERWISE THE ERROR INFO WILL GO IN THE NEXT REPORT
       if(!(usb_drvstatus&USB_STATUS_TXCTL))  ramusb_sendcontrolpacket(P_TYPE_REPORT);
       }
     */

    usb_drvstatus |= USB_STATUS_RXDATA;       // AND SIGNAL THAT WE HAVE DATA AVAILABLE
}

// SENDING INTERRUPT ENDPOINT
ARM_MODE void ramep1_irqservice()
{
    ramusb_ep1_transmit();
}

// RECEIVING DATA ENDPOINT
ARM_MODE void ramep2_irqservice()
{
    // ONLY RESPOND ONCE EVERYTHING IS SETUP
    if(!(usb_drvstatus & USB_STATUS_CONFIGURED))
        return;

    *INDEX_REG = RAWHID_RX_ENDPOINT;

    // NOTHING TO RECEIVE,
    if(*OUT_CSR1_REG & EPn_OUT_PKT_RDY) {
        // WE HAVE A PACKET, GO PROCESS IT
        ramusb_ep2_receive();
        return;
    }

    // GETTING INTERRUPTS WITHOUT PACKETS? SOMETHING IS WRONG,

    //if(*OUT_CSR1_REG&EPn_OUT_SENT_) return;  // ALREADY DONE

    //*OUT_CSR1_REG|=EPn_OUT_SEND_STALL;

    return;
}

// GENERAL INTERRUPT SERVICE ROUTINE - DISPATCH TO INDIVIDUAL ENDPOINT ROUTINES
ARM_MODE void ramusb_irqservice()
{
    if(!(usb_drvstatus & USB_STATUS_INIT))
        return;
    usb_drvstatus |= USB_STATUS_INSIDEIRQ;

    *INDEX_REG = 0;

    if(!(*EP_INT_REG & 7) && !(*USB_INT_REG & 7)) {
        // WHAT ARE THESE INTERRUPTS FOR?
        if(usb_drvstatus & (USB_STATUS_EP0TX | USB_STATUS_EP0RX))
            ramep0_irqservice();
        usb_drvstatus &= ~USB_STATUS_INSIDEIRQ;
        return;
    }

    if(*EP_INT_REG & 1) {
        ramep0_irqservice();
        *EP_INT_REG = 1;

    }

    // ALWAYS SERVICE THE ENDPOINTS
    ramep1_irqservice();
    ramep2_irqservice();
    *EP_INT_REG = 6;

    if(*USB_INT_REG & 1) {
        // ENTER SUSPEND MODE
        ramusb_hwsuspend();
        *USB_INT_REG = 1;
        usb_drvstatus |= USB_STATUS_SUSPEND;
        usb_drvstatus &= ~USB_STATUS_INSIDEIRQ;
        return;
    }

    if(*USB_INT_REG & 2) {
        // RESUME FROM SUSPEND MODE
        ramusb_hwresume();
        *USB_INT_REG = 2;
        usb_drvstatus &= ~USB_STATUS_SUSPEND;
        usb_drvstatus &= ~USB_STATUS_INSIDEIRQ;
        return;
    }

    if(*USB_INT_REG & 4) {
        // RESET RECEIVED
        if((*PWR_REG) & USB_RESET) {
            usb_drvstatus = USB_STATUS_INIT | USB_STATUS_CONNECTED;   // DECONFIGURE THE DEVICE
            ramusb_hwsetup();   // AND MAKE SURE THE HARDWARE IS IN KNOWN STATE
        }
        *USB_INT_REG = 4;
        usb_drvstatus &= ~USB_STATUS_INSIDEIRQ;
        return;
    }

    usb_drvstatus &= ~USB_STATUS_INSIDEIRQ;

}

//********************************************
// NEW HARDWARE-INDEPENDENT CODE (MODIFIED TO RUN FROM RAM)
//********************************************

// TRANSMIT ONE CONTROL PACKET

ARM_MODE void ramusb_sendcontrolpacket(int packet_type)
{
    if(!(usb_drvstatus & USB_STATUS_INSIDEIRQ)) {
        while(usb_drvstatus & USB_STATUS_TXCTL);      // THERE'S ANOTHER CONTROL PACKET, WAIT FOR IT TO BE SENT
    }

    // NOW PREPARE THE NEXT CONTROL PACKET IN THE BUFFER
    USB_PACKET *p = (USB_PACKET *) usb_ctltxbuffer;
    //memsetb(usb_ctltxbuffer,0,RAWHID_TX_SIZE+1);
    {
        int k;
        for(k = 0; k < RAWHID_TX_SIZE + 1; ++k)
            usb_ctltxbuffer[k] = 0;
    }

    switch (packet_type) {
    case P_TYPE_GETSTATUS:
        p->p_type = P_TYPE_GETSTATUS;
        p->p_fileidLSB = (BYTE) (usb_fileid & 0xff);
        p->p_fileidMSB = (BYTE) (usb_fileid >> 8);
        usb_drvstatus &= ~USB_STATUS_RXRCVD;
        break;
    case P_TYPE_CHECKPOINT:
        p->p_type = P_TYPE_CHECKPOINT;
        p->p_fileidLSB = (BYTE) (usb_fileid & 0xff);
        p->p_fileidMSB = (BYTE) (usb_fileid >> 8);
        p->p_offset = usb_offset;
        p->p_data[0] = usb_crc32 & 0xff;
        p->p_data[1] = (usb_crc32 >> 8) & 0xff;
        p->p_data[2] = (usb_crc32 >> 16) & 0xff;
        p->p_data[3] = (usb_crc32 >> 24) & 0xff;
        usb_drvstatus &= ~USB_STATUS_RXRCVD;
        break;

    case P_TYPE_ENDOFFILE:
        p->p_type = P_TYPE_ENDOFFILE;
        p->p_fileidLSB = (BYTE) (usb_fileid & 0xff);
        p->p_fileidMSB = (BYTE) (usb_fileid >> 8);
        p->p_offset = usb_offset;
        p->p_data[0] = usb_crc32 & 0xff;
        p->p_data[1] = (usb_crc32 >> 8) & 0xff;
        p->p_data[2] = (usb_crc32 >> 16) & 0xff;
        p->p_data[3] = (usb_crc32 >> 24) & 0xff;
        usb_drvstatus &= ~USB_STATUS_RXRCVD;
        break;

    case P_TYPE_ABORT:
        p->p_type = P_TYPE_ABORT;
        p->p_fileidLSB = (BYTE) (usb_fileid & 0xff);
        p->p_fileidMSB = (BYTE) (usb_fileid >> 8);
        break;

    case P_TYPE_REPORT:
        p->p_type = P_TYPE_REPORT;
        p->p_fileidLSB = (BYTE) (usb_fileid & 0xff);
        p->p_fileidMSB = (BYTE) (usb_fileid >> 8);
        p->p_offset = usb_offset;
        p->p_data[0] = (usb_drvstatus & USB_STATUS_HALT) ? 1 : 0;
        p->p_data[1] = (usb_drvstatus & USB_STATUS_ERROR) ? 1 : 0;
        p->p_data[2] = (usb_rxtotalbytes) ? 1 : 0;
        break;

    default:
        return; // INVALID PACKET TO SEND
    }
    usb_drvstatus |= USB_STATUS_TXCTL;        // INDICATE THE DRIVER WE HAVE TO SEND A CONTROL PACKET

}

// CALLED WHEN A REPORT ARRIVED FROM THE OTHER SIDE, PROCESS DEPENDING ON WHAT WE ARE DOING
ARM_MODE void ramusb_receivecontrolpacket()
{
    if(usb_drvstatus & USB_STATUS_RXCTL) {

        USB_PACKET *ctl = (USB_PACKET *) usb_ctlrxbuffer;

        switch (ctl->p_type) {
        case P_TYPE_GETSTATUS:
        {
            if(!usb_fileid) {
                // START RECEIVING A NEW TRANSMISSION
                usb_fileid = P_FILEID(ctl);
                usb_offset = 0;
                usb_crc32 = 0;
                usb_rxoffset = 0;
                usb_rxtxtop = 0;      // NUMBER OF BYTES USED IN THE RX BUFFER
                usb_rxtxbottom = 0;   // NUMBER OF BYTES IN THE RX BUFFER ALREADY READ BY THE USER
                usb_rxtotalbytes = 0; // DON'T KNOW THE TOTAL FILE SIZE YET
                usb_drvstatus &=
                        ~(USB_STATUS_HALT | USB_STATUS_ERROR | USB_STATUS_EOF);

            }
            // SEND A REPORT, IF WE HAVE AN EXISTING TRANSMISSION, IT WILL REPLY WITH THE OLD FILEID
            ramusb_sendcontrolpacket(P_TYPE_REPORT);
            break;
        }
        case P_TYPE_CHECKPOINT:
        {

            if(usb_fileid == P_FILEID(ctl)) {

                if(usb_drvstatus & USB_STATUS_ERROR) {
                    // IGNORE THE CHECKPOINT, THERE WAS A PRIOR ERROR
                    break;
                }
                usb_drvstatus &= ~USB_STATUS_ERROR;   // REMOVE ERROR SIGNAL

                int used = usb_rxtxtop - usb_rxtxbottom;
                if(used < 0)
                    used += RING_BUFFER_SIZE;
                if(usb_rxoffset + used != ctl->p_offset) {
                    // SOMETHING WENT WRONG, WE DISAGREE ON THE FILE SIZE
                    usb_drvstatus |= USB_STATUS_ERROR;        // SIGNAL TO RESEND FROM CURRENT OFFSET
                }
                WORD crc = ctl->p_data[0];
                crc |= ((WORD) ctl->p_data[1]) << 8;
                crc |= ((WORD) ctl->p_data[2]) << 16;
                crc |= ((WORD) ctl->p_data[3]) << 24;
                if(usb_crc32 != crc)
                    usb_drvstatus |= USB_STATUS_ERROR;        // SIGNAL TO RESEND FROM CURRENT OFFSET

                // SEND THE REPORT
                ramusb_sendcontrolpacket(P_TYPE_REPORT);
            }
            break;
        }
        case P_TYPE_ENDOFFILE:
        {
            if(usb_fileid == P_FILEID(ctl)) {

                if(usb_drvstatus & USB_STATUS_ERROR) {
                    // IGNORE THE END OF FILE, THERE WAS A PRIOR ERROR
                    break;
                }

                // SAME AS FOR A CHECKPOINT, BUT SET TOTAL BYTE COUNT
                usb_drvstatus &= ~USB_STATUS_ERROR;   // REMOVE ERROR SIGNAL

                if(usb_offset != ctl->p_offset) {
                    // SOMETHING WENT WRONG, WE DISAGREE ON THE FILE SIZE
                    usb_drvstatus |= USB_STATUS_ERROR;        // SIGNAL TO RESEND FROM CURRENT OFFSET
                }
                WORD crc = ctl->p_data[0];
                crc |= ((WORD) ctl->p_data[1]) << 8;
                crc |= ((WORD) ctl->p_data[2]) << 16;
                crc |= ((WORD) ctl->p_data[3]) << 24;
                if(usb_crc32 != crc)
                    usb_drvstatus |= USB_STATUS_ERROR;        // SIGNAL TO RESEND FROM CURRENT OFFSET

                // SET TOTAL BYTES TO INDICATE WE RECEIVED THE LAST OF IT
                if(!(usb_drvstatus & USB_STATUS_ERROR))
                    usb_rxtotalbytes = ctl->p_offset;

                // SEND A REPORT
                ramusb_sendcontrolpacket(P_TYPE_REPORT);
            }
            break;

        }
        case P_TYPE_ABORT:
        {
            if((usb_fileid == P_FILEID(ctl)) || (P_FILEID(ctl) == 0xffff)) {
                // REMOTE REQUESTED TO ABORT WHATEVER WE WERE DOING
                usb_drvstatus &=
                        ~(USB_STATUS_TXDATA | USB_STATUS_TXCTL |
                        USB_STATUS_RXDATA | USB_STATUS_HALT | USB_STATUS_ERROR |
                        USB_STATUS_RXCTL | USB_STATUS_EOF);

                // ABORT ALL TRANSACTIONS
                usb_fileid = 0;
                usb_offset = 0;
                usb_crc32 = 0;
                usb_rxtxbottom = 0;
                usb_rxtxtop = 0;
                usb_rxoffset = 0;
                usb_rxtotalbytes = 0;
                usb_txtotalbytes = 0;

            }

            usb_drvstatus |= USB_STATUS_RXRCVD;

            // DO NOT REPLY TO AN ABORT CONDITION

            break;

        }

        case P_TYPE_REPORT:
        {
            if(usb_fileid == P_FILEID(ctl)) {

                // UPDATE FLAGS WITH THE STATUS OF THE REMOTE
                if(ctl->p_data[0])
                    usb_drvstatus |= USB_STATUS_HALT;
                else
                    usb_drvstatus &= ~USB_STATUS_HALT;
                if(ctl->p_data[1]) {
                    // SIGNAL THE ERROR AND LEAVE THE REQUESTED OFFSET AT rxoffset
                    usb_drvstatus |= USB_STATUS_ERROR;
                    usb_rxoffset = ctl->p_offset;
                }
                if(ctl->p_data[2]) {
                    // SIGNAL THAT THE REMOTE ACKNOWLEDGED THE END OF FILE MARK
                    usb_drvstatus |= USB_STATUS_EOF;
                }
                else
                    usb_drvstatus &= ~USB_STATUS_EOF;

            }
            else {
                // WE RECEIVED A REPORT OF THE WRONG FILE, THE REMOTE ISN'T EVEN WORKING ON OUR FILE YET
                usb_drvstatus |= USB_STATUS_ERROR;
            }

            usb_drvstatus |= USB_STATUS_RXRCVD;
            break;
        }
        default:
            // DO NOTHING
            break;

        }

    }

    // SIGNAL THAT IT WAS PROCESSED
    usb_drvstatus &= ~USB_STATUS_RXCTL;

}

ARM_MODE int ramusb_isconnected()
{
    if(usb_drvstatus & USB_STATUS_CONNECTED)
        return 1;
    return 0;
}

ARM_MODE int ramusb_isconfigured()
{
    if(usb_drvstatus & USB_STATUS_CONFIGURED)
        return 1;
    return 0;
}

// HIGH LEVEL FUNCTION TO SEE IF THERE'S ANY DATA FROM THE USB DRIVER
ARM_MODE int ramusb_hasdata()
{
    if((usb_drvstatus & USB_STATUS_RXDATA)
            && (usb_rxtxtop != usb_rxtxbottom)) {
        int bytesready = usb_rxtxtop - usb_rxtxbottom;
        if(bytesready < 0)
            bytesready += RING_BUFFER_SIZE;
        return bytesready;
    }
    return 0;
}

// HIGH LEVEL FUNCTION TO BLOCK UNTIL DATA ARRIVES
// WAIT UNTIL WE GET AT LEAST nbytes OR TIMEOUT
// RETURN 0 IF TIMEOUT
ARM_MODE int ramusb_waitfordata(int nbytes)
{
    int hasbytes;

    hasbytes = ramusb_hasdata();

    while(hasbytes < nbytes) {
        if((usb_drvstatus & (USB_STATUS_CONFIGURED | USB_STATUS_INIT |
                        USB_STATUS_CONNECTED)) !=
                (USB_STATUS_CONFIGURED | USB_STATUS_INIT |
                    USB_STATUS_CONNECTED))
            return 0;

        //ramcpu_waitforinterrupt();

        hasbytes = ramusb_hasdata();

        //if(usb_drvstatus&USB_STATUS_HALT) {
        // NO MORE DATA WILL COME BECAUSE OUR BUFFERS ARE FULL, EMPTY THE BUFFERS BY RETURNING WHAT WE HAVE SO FAR
        //    break;
        //}

        if(usb_rxtotalbytes) {
            // WE FINISHED RECEIVING THE FILE
            break;
        }

        ramcpu_waitforinterrupt();

    }

    return hasbytes;
}

// START RECEIVING A FILE, WHETHER IT WAS COMPLETELY RECEIVED YET OR NOT
// RETURNS THE FILEID OR 0 IF FAILS
ARM_MODE int ramusb_rxfileopen()
{
    if(!ramusb_hasdata())
        return 0;
    return usb_fileid;
}

// RETURN HOW MANY BYTES ARE READY TO BE READ
ARM_MODE int ramusb_rxbytesready(int fileid)
{
    if(fileid != (int)usb_fileid)
        return 0;
    int bytesready = usb_rxtxtop - usb_rxtxbottom;
    if(bytesready < 0)
        bytesready += RING_BUFFER_SIZE;
    return bytesready;
}

// RETRIEVE BYTES THAT WERE ALREADY RECEIVED
ARM_MODE int ramusb_fileread(int fileid, BYTEPTR dest, int nbytes)
{
    if(fileid != (int)usb_fileid)
        return 0;

    if(nbytes <= 0)
        return 0;

    // WAIT FOR ENOUGH BYTES TO BECOME AVAILABLE
    int bytescopied = 0;

    do {

        int available = ramusb_waitfordata(nbytes);

        if(!available)
            return 0;

        if(available >= nbytes)
            available = nbytes;

        // QUICK COPY IF WE ALREADY HAVE ENOUGH BYTES

        if(usb_rxtxbottom + available > RING_BUFFER_SIZE) {
            // SPLIT THE COPY IN TWO OPERATIONS
            rammemmoveb(dest, usb_rxtxbuffer + usb_rxtxbottom,
                    RING_BUFFER_SIZE - usb_rxtxbottom);
            rammemmoveb(dest + (RING_BUFFER_SIZE - usb_rxtxbottom),
                    usb_rxtxbuffer,
                    available - (RING_BUFFER_SIZE - usb_rxtxbottom));
        }
        else {
            rammemmoveb(dest, usb_rxtxbuffer + usb_rxtxbottom, available);
        }

        usb_rxtxbottom += available;
        usb_rxoffset += available;
        if(usb_rxtxbottom >= RING_BUFFER_SIZE)
            usb_rxtxbottom -= RING_BUFFER_SIZE;
        dest += available;
        nbytes -= available;
        bytescopied += available;

        if(usb_rxtotalbytes && (usb_rxoffset >= usb_rxtotalbytes)) {
            usb_drvstatus |= USB_STATUS_EOF;
            nbytes = 0;
        }

        // SEE IF COMMS WERE HALTED
        if(usb_drvstatus & USB_STATUS_HALT) {
            // WE EMPTIED THE BUFFERS, RELEASE THE HALT THEN WAIT SOME MORE
            int usedspace = usb_rxtxtop - usb_rxtxbottom;
            if(usedspace < 0)
                usedspace += RING_BUFFER_SIZE;

            // RELEASE THE HALT IF BUFFERS ARE LESS THAN QUARTER FULL
            if(usedspace <= RING_BUFFER_SIZE / 4) {
                usb_drvstatus &= ~USB_STATUS_HALT;
                if(!(usb_drvstatus & USB_STATUS_ERROR))
                    ramusb_sendcontrolpacket(P_TYPE_REPORT);    // NOTIFY WE LIFTED THE HALT ONLY IF THERE WERE NO ERRORS, OTHERWISE LET THE DRIVER FIX THE ERROR FIRST
            }
        }

    }
    while(nbytes > 0);

    return bytescopied;
}

ARM_MODE int ramusb_eof(int fileid)
{
    if(fileid != usb_fileid)
        return 0;

    return (usb_drvstatus & USB_STATUS_EOF) ? 1 : 0;
}

// CLOSE THE FILE RELEASE ANY PENDING DATA
ARM_MODE int ramusb_rxfileclose(int fileid)
{
    if(fileid != usb_fileid)
        return 0;

    if(!usb_rxtotalbytes) {
        // IF WE STILL DIDN'T RECEIVE THE ENTIRE FILE, ABORT IT
        ramusb_sendcontrolpacket(P_TYPE_ABORT);

        // WAIT FOR THE CONTROL PACKET TO BE SENT
        while(usb_drvstatus & USB_STATUS_TXCTL) {

            if((usb_drvstatus & (USB_STATUS_CONFIGURED | USB_STATUS_INIT |
                            USB_STATUS_CONNECTED)) !=
                    (USB_STATUS_CONFIGURED | USB_STATUS_INIT |
                        USB_STATUS_CONNECTED))
                return 0;

            ramcpu_waitforinterrupt();
        }

    }

    // AND PUT THE DRIVER TO IDLE
    usb_fileid_seq = usb_fileid & 0xff;
    usb_fileid = 0;
    usb_rxtxtop = 0;
    usb_rxtxbottom = 0;
    usb_rxoffset = 0;
    usb_rxtotalbytes = 0;
    usb_drvstatus &=
            ~(USB_STATUS_EOF | USB_STATUS_HALT | USB_STATUS_ERROR |
            USB_STATUS_RXDATA);
    return 1;
}

//*******************************************************************************************************
// USB ROUTINES PREPARED TO RUN FROM RAM DURING FIRMWARE UPDATE - DO NOT REORGANIZE OR CHANGE THIS BLOCK
//*******************************************************************************************************

// RECEIVE BYTES FROM THE HOST IN EP2 ENDPOINT
// STARTS A NEW TRANSMISSION IF newtransmission IS TRUE, EVEN IF
// THERE ARE ZERO BYTES IN THE BUFFER
// FOR USE WITHIN ISR

extern unsigned int irq_table[32];

void ramirq_service() __attribute__((naked));
ARM_MODE void ramirq_service()
{
    asm volatile ("stmfd sp!, {r0-r12,lr}");
    asm volatile ("mov r0,sp");
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0x1f");
    asm volatile ("msr cpsr_all,r1");   // SWITCH TO SYSTEM MODE
    asm volatile ("stmfd r0!,{sp,lr}"); // SAVE REGISTERS THAT WERE BANKED
    asm volatile ("stmfd sp!,{ r0 }");  // SAVE IRQ STACK PTR
    *HWREG(INT_REGS, 0x0) = *HWREG(INT_REGS, 0x10);     // CLEAR SRCPENDING EARLY TO AVOID MISSING ANY OTHER INTERRUPTS
    (*((__interrupt__) (irq_table[*HWREG(INT_REGS, 0x14)]))) ();
    // CLEAR INTERRUPT PENDING FLAG
    register unsigned int a = 1 << (*HWREG(INT_REGS, 0x14));
    *HWREG(INT_REGS, 0x10) = a;

    asm volatile ("ldmia sp!, { r0 }"); // GET IRQ STACK PTR
    asm volatile ("ldmia r0!, { sp, lr }");     // RESTORE USER STACK AND LR
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0xd");
    asm volatile ("msr cpsr_all,r1");   // SWITCH BACK TO IRQ MODE
    asm volatile ("ldmia sp!, {r0-r12,lr}");    // RESTORE ALL OTHER REGISTERS BACK
    asm volatile ("subs pc,lr,#4");
}

// DO A FULL RESET
ARM_MODE void ram_doreset()
{

    // TURN OFF THE LCD
    volatile unsigned int *LCDCON1 = (unsigned int *)LCD_REGS;
    *LCDCON1 = (*LCDCON1) & (0xFFFFFFFE);

    *IO_GPDCON = (*IO_GPDCON & ~0xC000) | 0x4000;       // SET GPD7 AS OUTPUT
    *IO_GPDUP &= ~0x80; // ENABLE PULLUPS
    *IO_GPDDAT &= ~0x80;        // DISCONNECT POWER TO THE LCD WITH GPD7

// SET THE PRESCALER OF THE WATCHDOG AS FAST AS POSSIBLE AND A REASONABLE COUNT (ABOUT 87ms)
    *HWREG(WDT_REGS, 8) = 0x8000;
    *HWREG(WDT_REGS, 0) = 0x21;

    // AND WAIT FOR IT TO HAPPEN
    while(1);

}

#define FLASH_SECTORSIZE 4096   // SECTOR SIZE IN BYTES
#define WRITE_uint16_t_FLASH(address,data) *((volatile uint16_t *)(address<<1))=data

// ERASE FLASH AREA BETWEEN GIVEN ADDRESSES (IN 32-BIT WORDS)

ARM_MODE void ram_flasherase(WORDPTR address, int nwords)
{
    volatile uint16_t *ptr = (uint16_t *) (((WORD) address) & ~(FLASH_SECTORSIZE - 1)); // FIND START OF SECTOR
    nwords += (((WORD) address) & (FLASH_SECTORSIZE - 1)) >> 2; // CORRECTION FOR MISALIGNED SECTORS

    uint16_t data, prevdata;

// START ERASING SECTORS OF FLASH
    while(nwords > 0) {
        if(((WORD) ptr >= 0x4000) && ((WORD) ptr < 0x00200000)) // NEVER ERASE THE BOOT LOADER!
        {

            // DISABLE INTERRUPTS
            asm volatile ("stmfd sp!, {r0}");
            asm volatile ("mrs r0,cpsr_all");
            asm volatile ("orr r0,r0,#0xc0");
            asm volatile ("msr cpsr_all,r0");
            asm volatile ("ldmia sp!, { r0 }");

            // SECTOR ERASE COMMAND
            WRITE_uint16_t_FLASH(0x5555, 0x00AA);       // write data 0x00AA to device addr 0x5555
            WRITE_uint16_t_FLASH(0x2AAA, 0x0055);       // write data 0x0055 to device addr 0x2AAA
            WRITE_uint16_t_FLASH(0x5555, 0x0080);       // write data 0x0080 to device addr 0x5555
            WRITE_uint16_t_FLASH(0x5555, 0x00AA);       // write data 0x00AA to device addr 0x5555
            WRITE_uint16_t_FLASH(0x2AAA, 0x0055);       // write data 0x0055 to device addr 0x2AAA
            *ptr = 0x0030;      // write data 0x0030 to device sector addr

            // USE TOGGLE READY TO WAIT FOR COMPLETION
            prevdata = *ptr;
            data = prevdata ^ 0xffff;
            while(data ^ prevdata) {
                prevdata = data;
                data = *ptr;
            }
        }

        // RE-ENABLE INTERRUPTS
        asm volatile ("stmfd sp!, {r1}");
        asm volatile ("mrs r1,cpsr_all");
        asm volatile ("bic r1,r1,#0xc0");
        asm volatile ("msr cpsr_all,r1");
        asm volatile ("ldmia sp!, { r1 }");

        nwords -= (FLASH_SECTORSIZE >> 2);      // IN 32-BIT WORDS
        ptr += (FLASH_SECTORSIZE >> 1); // IN 16-BIT uint16_tS
    }
// DONE - FLASH WAS ERASED
}

// PROGRAM A 32-BIT WORD INTO FLASH

ARM_MODE void ram_flashprogramword(WORDPTR address, WORD value)
{

    volatile uint16_t *ptr = (uint16_t *) address;
    uint16_t data, prevdata;
    // DISABLE INTERRUPTS
    asm volatile ("stmfd sp!, {r0}");
    asm volatile ("mrs r0,cpsr_all");
    asm volatile ("orr r0,r0,#0xc0");
    asm volatile ("msr cpsr_all,r0");
    asm volatile ("ldmia sp!, { r0 }");

    // PROGRAM WORD COMMAND
    WRITE_uint16_t_FLASH(0x5555, 0x00AA);
    WRITE_uint16_t_FLASH(0x2AAA, 0x0055);
    WRITE_uint16_t_FLASH(0x5555, 0x00A0);
    *ptr = (uint16_t) (value & 0xffff);

    // USE TOGGLE READY TO WAIT FOR COMPLETION
    prevdata = *ptr;
    data = prevdata ^ 0xffff;
    while(data ^ prevdata) {
        prevdata = data;
        data = *ptr;
    }

    ++ptr;

    // PROGRAM WORD COMMAND
    WRITE_uint16_t_FLASH(0x5555, 0x00AA);
    WRITE_uint16_t_FLASH(0x2AAA, 0x0055);
    WRITE_uint16_t_FLASH(0x5555, 0x00A0);
    *ptr = (uint16_t) ((value >> 16) & 0xffff);

    // USE TOGGLE READY TO WAIT FOR COMPLETION
    prevdata = *ptr;
    data = prevdata ^ 0xffff;
    while(data ^ prevdata) {
        prevdata = data;
        data = *ptr;
    }

    // RE-ENABLE INTERRUPTS
    asm volatile ("stmfd sp!, {r1}");
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");
    asm volatile ("ldmia sp!, { r1 }");

}

// FLASH PROGRAMMING PROTOCOL:
// USES LONG TRANSMISSION PROTOCOL SAME AS USED FOR RAM BACKUP OBJECTS
// 4 BYTES='FWUP' HEADER (INSTEAD OF 'NRPB')
// 4 BYTES=START ADDRESS OF THIS BLOCK, USE 0XFFFFFFFF TO END FLASH PROGRAMMING AND RESET
// 4 BYTES=NUMBER OF 32-BIT WORDS TO PROGRAM
// [DATA]= NWORDS 32-BIT WORDS SENT IN LSB

// DEVICE WILL KEEP LISTENING FOR ADDITIONAL BLOCKS
// UNTIL A BLOCK WITH OFFSET 0xFFFFFFFF IS SENT, THEN IT WILL RESET

// MAIN PROCEDURE TO RECEIVE AND FLASH FIRMWARE FROM RAM
ARM_MODE void ram_receiveandflashfw(int32_t flashsize)
{
    int pass = 1, result, fileid;
    WORDPTR flash_address;
    WORD flash_nwords, data;
    WORD receivedwords;

    do {

        // SLEEP UNTIL WE GET SOMETHING
        while(!ramusb_hasdata())
            ramcpu_waitforinterrupt();

        result = fileid = ramusb_rxfileopen();
        if((!result) || usb_filetype(fileid) != 'W') {
            ramusb_rxfileclose(fileid);
            continue;
        }

        data = 0xffffffff;

// RECEIVE THE ENTIRE FILE, GET THE TOTAL NUMBER OF BYTES RECEIVED
// THIS WAY WE AVOID IRQS DURING FLASHING
        receivedwords = (ramusb_waitfordata(6000) + 3) >> 2;

        if(ramusb_fileread(fileid, (BYTEPTR) & data, 4) < 4) {
            ram_doreset();      // NOTHING ELSE TO DO
        }

        if(data != TEXT2WORD('F', 'W', 'U', 'P')) {
            ram_doreset();      // NOTHING ELSE TO DO
        }

        if(ramusb_fileread(fileid, (BYTEPTR) & flash_address, 4) < 4) {
            ram_doreset();      // NOTHING ELSE TO DO
        }

        if(ramusb_fileread(fileid, (BYTEPTR) & flash_nwords, 4) < 4) {
            ram_doreset();
        }

        if(flash_nwords + 3 != receivedwords) {
            ram_doreset();
        }

        if((((WORD) flash_address) == 0xffffffff)) {

            // FINISH RECEIVING THE COMMAND
            ramusb_rxfileclose(fileid);

            // SHOW SOME VISUALS
            int k;
            for(k = 0; k < flashsize - 0x4000; k += 0x1000) {
                unsigned int *scrptr = (unsigned int *)MEM_PHYS_SCREEN;
                int pixel = (k) >> 14;
                scrptr[20 + (pixel >> 3)] &= ~(0xf << ((pixel & 7) << 2));
                scrptr[40 + (pixel >> 3)] &= ~(0xf << ((pixel & 7) << 2));
                scrptr[60 + (pixel >> 3)] &= ~(0xf << ((pixel & 7) << 2));
            }

            for(; k >= 0; k -= 4) {
                unsigned int *scrptr = (unsigned int *)MEM_PHYS_SCREEN;
                int pixel = (k) >> 14;

                scrptr[20 + (pixel >> 3)] |= (0x6 << ((pixel & 7) << 2));
                scrptr[40 + (pixel >> 3)] |= (0x6 << ((pixel & 7) << 2));
                scrptr[60 + (pixel >> 3)] |= (0x6 << ((pixel & 7) << 2));
            }

            for(k = 0; k < flashsize - 0x4000; k += 4) {
                unsigned int *scrptr = (unsigned int *)MEM_PHYS_SCREEN;
                int pixel = (k) >> 14;

                scrptr[20 + (pixel >> 3)] |= (0xf << ((pixel & 7) << 2));
                scrptr[40 + (pixel >> 3)] |= (0xf << ((pixel & 7) << 2));
                scrptr[60 + (pixel >> 3)] |= (0xf << ((pixel & 7) << 2));
            }

            ram_doreset();      // HOST REQUESTED A RESET
        }

        if((WORD) (flash_address + flash_nwords) > flashsize) {
            ram_doreset();
        }

        if(((WORD) flash_address < 0x4000)) {

            ram_doreset();      // PROTECT THE BOOTLOADER AT ALL COSTS
        }

// ERASE THE FLASH
        ram_flasherase(flash_address, flash_nwords);    // ERASE ENOUGH FLASH BLOCKS TO PREPARE FOR FIRMWARE UPDATE

        WORDPTR dataptr = (WORDPTR) (usb_rxtxbuffer + usb_rxtxbottom);      // THIS POINTS TO THE NEXT BYTE TO READ

        while(flash_nwords--) {
            ram_flashprogramword(flash_address, *dataptr);
            ++flash_address;
            ++dataptr;
        }

// WE FINISHED PROGRAMMING THE FLASH!

        ramusb_rxfileclose(fileid);

// SHOW SOME VISUAL FEEDBACK
        {
            unsigned int *scrptr = (unsigned int *)MEM_PHYS_SCREEN;
            int pixel = (((WORD) flash_address) - 0x4000) >> 14;
            scrptr[20 + (pixel >> 3)] |= (0xf << ((pixel & 7) << 2));
            scrptr[40 + (pixel >> 3)] |= (0xf << ((pixel & 7) << 2));
            scrptr[60 + (pixel >> 3)] |= (0xf << ((pixel & 7) << 2));
        }
    }
    while(1);
//ram_doreset();

// NEVER RETURNS
}

// USE SCRATCH AREA TO EXECUTE TEMPORARY CODE FROM RAM
WORD scratch_buffer[2500] SCRATCH_MEMORY;

ARM_MODE void ram_startfwupdate()
{

    // AT THIS POINT, A USB CONNECTION HAS ALREADY BEEN ESTABLISHED
    // THIS ROUTINE WILL MAKE SURE WE RUN FROM RAM, ERASE ALL FLASH AND UPDATE THE FIRMWARE

    uint16_t cfidata[50];       // ACTUALLY 36 WORDS ARE USED ONLY
    flash_CFIRead(cfidata);

    flash_prepareforwriting();

    // ADD SOME VISUAL FEEDBACK
    {
        unsigned int *scrptr = (unsigned int *)MEM_PHYS_SCREEN;
        int k;
        for(k = 0; k < 16; ++k) {
            scrptr[k] = 0xffffffff;
            scrptr[20 + k] = 0;
            scrptr[40 + k] = 0;
            scrptr[60 + k] = 0;
            scrptr[80 + k] = 0xffffffff;
        }

    }

    // CHECK THE SIZE OF RAM
    int flashsize = 1 << (WORD) (cfidata[0x17]);

    // NOW COPY THE CODE TO RAM

    int needwords =
            (WORDPTR) & ram_startfwupdate - (WORDPTR) & ramusb_crc32roll;

    WORDPTR codeblock = (WORDPTR) scratch_buffer;     // STORE CODE ON TOP OF THE STACK

    memmovew(codeblock, &ramusb_crc32roll, needwords);

    // ALSO COPY THE CRC TABLE FROM ROM TO RAM
    memmovew(RAM_CRCTABLE, crctable, 256);

    // EVERYTHING IS NOW IN RAM

    // DISABLE ALL INTERRUPTS
    cpu_intoff_nosave();
    // MAKE SURE THE CODE WAS COPIED TO RAM BEFORE WE EXECUTE IT
    cpu_flushwritebuffers();

    // MOVE MAIN ISR TO RAM AS WELL
    *((unsigned int *)0x08000018) =
            (unsigned int)(codeblock + ((WORDPTR) & ramirq_service -
                (WORDPTR) & ramusb_crc32roll));

    // AND MAKE SURE WE DON'T EXECUTE AN OLD COPY LEFT IN THE CACHE
    cpu_flushicache();

    // SET INTERRUPT HANDLER IN RAM
    irq_addhook(25,
            (void *)(codeblock + ((WORDPTR) & ramusb_irqservice -
                    (WORDPTR) & ramusb_crc32roll)));

    // UNMASK ONLY THE ONE INTERRUPT WE NEED
    irq_unmask(25);

    void (*ptr)(int);

    ptr = (void *)(codeblock + ((WORDPTR) & ram_receiveandflashfw -
                (WORDPTR) & ramusb_crc32roll));

    (ptr) (flashsize);

    // THIS CAN NEVER RETURN, THERE WILL BE NO ROM HERE

    while(1);

}
