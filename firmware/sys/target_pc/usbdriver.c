/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include <newrpl.h>
#include <ui.h>

#include "hidapi.h"

// OTHER EXTERNAL FUNCTIONS NEEDED
hid_device *__usb_curdevice;
// THIS IS EXCLUSIVE TO THE PC VERSION
char __usb_devicepath[8192];

volatile int __usb_paused;
int __usb_timeout;

// OTHER BIT DEFINITIONS
#define USB_RESET        8

#define EP0_FIFO_SIZE    8
#define EP1_FIFO_SIZE    64
#define EP2_FIFO_SIZE    64

#define USB_DIRECTION   0x80
#define USB_DEV_TO_HOST 0x80

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

// NEW SIMPLIFIED GLOBALS

volatile BINT __usb_drvstatus __SYSTEM_GLOBAL__;        // FLAGS TO INDICATE IF INITIALIZED, CONNECTED, SENDING/RECEIVING, ETC.

BINT __usb_fileid __SYSTEM_GLOBAL__;    // CURRENT FILEID
BINT __usb_fileid_seq __SYSTEM_GLOBAL__;        // SEQUENTIAL NUMBER TO MAKE FILEID UNIQUE
BINT __usb_offset __SYSTEM_GLOBAL__;    // CURRENT OFFSET WITHIN THE FILE
WORD __usb_crc32 __SYSTEM_GLOBAL__;     // CURRENT CRC32 OF DATA RECEIVED
BINT __usb_lastgood_offset __SYSTEM_GLOBAL__;    // LAST KNOWN GOOD OFFSET WITHIN THE FILE
WORD __usb_lastgood_crc __SYSTEM_GLOBAL__;     // LAST KNOWN GOOD CRC32 OF DATA RECEIVED
BYTE __usb_ctlbuffer[RAWHID_RX_SIZE + 1] __SYSTEM_GLOBAL__;     // BUFFER TO RECEIVE CONTROL PACKETS IN THE CONTROL CHANNEL
BYTE __usb_tmprxbuffer[RAWHID_RX_SIZE + 1] __SYSTEM_GLOBAL__;   // TEMPORARY BUFFER TO RECEIVE DATA
BYTE __usb_ctlrxbuffer[RAWHID_RX_SIZE + 1] __SYSTEM_GLOBAL__;   // TEMPORARY BUFFER TO RECEIVE CONTROL PACKETS
BYTE __usb_ctltxbuffer[RAWHID_TX_SIZE + 1] __SYSTEM_GLOBAL__;   // TEMPORARY BUFFER TO TRANSMIT DATA

BYTE __usb_rxtxbuffer[LONG_BUFFER_SIZE];        // LARGE BUFFER TO RECEIVE AT LEAST 3 FULL FRAGMENTS
BINT __usb_rxoffset __SYSTEM_GLOBAL__;  // STARTING OFFSET OF THE DATA IN THE RX BUFFER
volatile BINT __usb_rxtxtop __SYSTEM_GLOBAL__;  // NUMBER OF BYTES USED IN THE RX BUFFER
volatile BINT __usb_rxtxbottom __SYSTEM_GLOBAL__;       // NUMBER OF BYTES IN THE RX BUFFER ALREADY READ BY THE USER
volatile BINT __usb_rxtotalbytes __SYSTEM_GLOBAL__;     // TOTAL BYTES ON THE FILE, 0 MEANS DON'T KNOW YET

BINT __usb_txtotalbytes __SYSTEM_GLOBAL__;      // TOTAL BYTES ON THE FILE, 0 MEANS DON'T KNOW YET
BINT __usb_txseq __SYSTEM_GLOBAL__;     // SEQUENTIAL NUMBER WITHIN A FRAGMENT OF DATA

BYTEPTR __usb_ctlbufptr __SYSTEM_GLOBAL__;      // POINTER TO BUFFER DURING CONTROL CHANNEL TRANSFERS
BINT __usb_ctlcount __SYSTEM_GLOBAL__;  // COUNT OF DATA DURING CONTROL CHANNEL TRANSFERS
BINT __usb_ctlpadding __SYSTEM_GLOBAL__;        // COUNT OF DATA DURING CONTROL CHANNEL TRANSFERS

//  END OF NEW GLOBALS
// ********************************

static WORD __crctable[256] = {
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
        crc = __crctable[(crc ^ *data++) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xffffffff;
}

void usb_hwsetup()
{

}

void usb_hwsuspend()
{

}

void usb_hwresume()
{

}

//void usb_irqservice();

void usb_irqdisconnect()
{
    // CALLED WHEN THE CABLE IS DISCONNECTED

    if(__usb_curdevice)
        hid_close(__usb_curdevice);

    usb_mutex_lock();
    __usb_curdevice = 0;
    __usb_devicepath[0] = 0;

    __usb_fileid = 0;

    __usb_drvstatus = 0;        // MARK UNCONFIGURED
    usb_mutex_unlock();
}

void usb_irqconnect()
{

    // CALLED WHEN THE CABLE IS CONNECTED, CALLED FROM WITHIN THE THREAD

    usb_mutex_lock();
    __usb_drvstatus = USB_STATUS_INIT | USB_STATUS_CONNECTNOW;
    usb_mutex_unlock();

    // WE HAVE A PATH, TRY TO OPEN THE DEVICE
    __usb_curdevice = hid_open_path((const char *)__usb_devicepath);

    usb_mutex_lock();
    if(!__usb_curdevice)
        __usb_devicepath[0] = 0;

    __usb_fileid = 0;
    __usb_fileid_seq &= 0xff;
    __usb_offset = 0;
    __usb_crc32 = 0;    // RESET CRC32

    __usb_txtotalbytes = 0;

    __usb_rxoffset = 0;
    __usb_rxtxtop = 0;  // NUMBER OF BYTES USED IN THE RX BUFFER
    __usb_rxtxbottom = 0;       // NUMBER OF BYTES IN THE RX BUFFER ALREADY READ BY THE USER
    __usb_rxtotalbytes = 0;     // DON'T KNOW THE TOTAL FILE SIZE YET

    __usb_drvstatus |= USB_STATUS_CONNECTED | USB_STATUS_CONFIGURED;
    usb_mutex_unlock();
}

void usb_init(int force)
{

    if(__usb_drvstatus & USB_STATUS_INIT) {
        if(!force)
            return;
        usb_shutdown(); // FORCE A SHUTDOWN TO RESET THE PHY COMPLETELY
    }

    int oldpause = __usb_paused;

    usb_mutex_lock();
    __usb_drvstatus = USB_STATUS_INIT | USB_STATUS_CONNECTNOW;
    usb_mutex_unlock();

    __usb_paused = 0;

    while(__usb_drvstatus & USB_STATUS_CONNECTNOW);

    if(oldpause) {
        __usb_paused = 1;
        while(__usb_paused >= 0);
    }

    __usb_fileid = 0;
    __usb_fileid_seq &= 0xff;
    __usb_offset = 0;
    __usb_crc32 = 0;    // RESET CRC32

    __usb_txtotalbytes = 0;

    __usb_rxoffset = 0;
    __usb_rxtxtop = 0;  // NUMBER OF BYTES USED IN THE RX BUFFER
    __usb_rxtxbottom = 0;       // NUMBER OF BYTES IN THE RX BUFFER ALREADY READ BY THE USER
    __usb_rxtotalbytes = 0;     // DON'T KNOW THE TOTAL FILE SIZE YET

    // TODO: SETUP COMMUNICATIONS BUFFERS

}

// THESE ARE MEANT TO BE CALLED FROM OUTSIDE THE IRQ HANDLER

void usb_shutdown()
{

    if(!(__usb_drvstatus & USB_STATUS_INIT))
        return;

    int oldpause = __usb_paused;

    usb_mutex_lock();
    __usb_drvstatus = USB_STATUS_INIT | USB_STATUS_DISCONNECTNOW;
    usb_mutex_unlock();

    __usb_paused = 0;

    while(__usb_drvstatus & USB_STATUS_DISCONNECTNOW);

    usb_mutex_lock();
    __usb_fileid = 0;
    __usb_drvstatus = 0;        // MARK UNCONFIGURED
    usb_mutex_unlock();

    if(oldpause) {
        __usb_paused = 1;
        while(__usb_paused >= 0);
    }

}

inline void usb_checkpipe()
{

}

// DRIVER - PACKET TRANSMISSION CALLED BY IRQ - NEVER CALLED BY USER
// SEND A CONTROL PACKET IF ONE AVAILABLE, OR ONE DATA PACKET IF AVAILABLE
void usb_ep1_transmit()
{

    if(!(__usb_drvstatus & USB_STATUS_CONNECTED))
        return;

    if(__usb_drvstatus & USB_STATUS_TXCTL) {
        // WE HAVE A CONTROL PACKET READY TO GO IN THE CONTROL BUFFER

        if(__usb_curdevice) {
            // ADD THE REPORT BYTE IN THE BUFFER
            memmoveb(__usb_ctltxbuffer + 1, __usb_ctltxbuffer, 64);
            __usb_ctltxbuffer[0] = 0;
            hid_write(__usb_curdevice, __usb_ctltxbuffer, RAWHID_TX_SIZE + 1);
        }

        usb_mutex_lock();
        __usb_drvstatus &= ~USB_STATUS_TXCTL;
        usb_mutex_unlock();
        return;
    }

    if(__usb_drvstatus & USB_STATUS_TXDATA) {

        if(__usb_drvstatus & USB_STATUS_HALT) {
            // DON'T SEND ANY DATA IF THE REMOTE REQUESTED HALT
            return;
        }
        if(__usb_drvstatus & USB_STATUS_WAIT_FOR_ACK) {
            // DON'T SEND ANY DATA IF WE ARE WAITING FOR AN ANSWER
            return;
        }


        // WE HAVE A DATA PACKET TO SEND
        if(__usb_drvstatus & USB_STATUS_ERROR) {
            // THE REMOTE DIDN'T GET IT, WE NEED TO RESEND
            // THE WANTED OFFSET WAS LEFT IN __usb_rxoffset BY usb_receivecontrolpacket()
            if(__usb_rxoffset != __usb_offset) {
                // THE REMOTE WANTS A PREVIOUS CHUNK OF THE FILE
                // CHECK IF WE HAVE IT IN THE SOURCE BUFFER

                // __usb_offset ALWAYS POINTS TO THE OFFSET OF __usb_rxtxbottom = LAST BYTE SENT

                int bufoff = (int)__usb_offset - (int)__usb_rxoffset;
                int oldestdata = __usb_rxtxbottom - __usb_rxtxtop;
                if(oldestdata < 0)
                    oldestdata += RING_BUFFER_SIZE;
                if((bufoff < 0) || (bufoff > oldestdata) ) {
                    // WE DON'T HAVE THAT DATA STORED ANYMORE, ABORT THE FILE
                    usb_sendcontrolpacket(P_TYPE_ABORT);

                    usb_mutex_lock();
                    __usb_fileid = 0;
                    __usb_offset = 0;
                    __usb_crc32 = 0;

                    __usb_rxtxtop = __usb_rxtxbottom = 0;

                    __usb_drvstatus &= ~USB_STATUS_TXDATA;
                    __usb_drvstatus |= USB_STATUS_ERROR;
                    usb_mutex_unlock();

                    if(__usb_curdevice) {
                        memmoveb(__usb_ctltxbuffer + 1, __usb_ctltxbuffer, 64);
                        __usb_ctltxbuffer[0] = 0;
                        hid_write(__usb_curdevice, __usb_ctltxbuffer,
                                RAWHID_TX_SIZE + 1);
                    }

                    usb_mutex_lock();
                    __usb_drvstatus &= ~USB_STATUS_TXCTL;
                    usb_mutex_unlock();
                    return;
                }

                // ADJUST THE RING'S POSITION TO SEND THE RIGHT DATA
                __usb_rxtxbottom -= bufoff;
                if(__usb_rxtxbottom < 0)
                    __usb_rxtxbottom += RING_BUFFER_SIZE;
                __usb_offset = __usb_rxoffset;
            }
            usb_mutex_lock();
            __usb_txseq = 0;    // RESTART BACK THE SEQUENCE NUMBER
            __usb_crc32 = __usb_lastgood_crc;    // RESET THE CRC FROM HERE ON
            __usb_drvstatus &= ~USB_STATUS_ERROR;       // REMOVE THE ERROR AND RESEND
            usb_mutex_unlock();
        }

        int bufbytes;
        int p_type;
        int eof = 0;

        bufbytes = __usb_rxtxtop - __usb_rxtxbottom;
        if(bufbytes < 0)
            bufbytes += RING_BUFFER_SIZE;

        if(bufbytes > USB_DATASIZE) {
            bufbytes = USB_DATASIZE;    // DON'T SEND MORE THAN ONE PACKET AT A TIME
        }

        // CHECK IF THESE ARE THE LAST FEW BYTES OF THE FILE
        if((int)__usb_txtotalbytes - (int)__usb_offset == bufbytes)
            eof = 1;
        else {
            if(bufbytes < USB_DATASIZE) {
                // WAIT FOR MORE DATA TO FILL UP THE PACKET, NO NEED TO SEND IT NOW
                return;
            }
        }

        p_type = __usb_txseq + 1;
        if(eof)
            p_type |= 0x40;
        if(p_type == 32)
            p_type = 0x40;

        if(p_type==1) {
            // SAVE AT THE BEGINNING OF EACH FRAGMENT
            __usb_lastgood_offset = __usb_offset;
            __usb_lastgood_crc = __usb_crc32;
        }

        // SEND A FULL PACKET
        BYTE tmpbuf[RAWHID_TX_SIZE + 1];
        tmpbuf[0] = 0;

        tmpbuf[1] = (BYTE) p_type;
        tmpbuf[2] = (BYTE) bufbytes;
        tmpbuf[3] = __usb_fileid & 0xff;
        tmpbuf[4] = (__usb_fileid >> 8) & 0xff;
        tmpbuf[5] = __usb_offset & 0xff;
        tmpbuf[6] = (__usb_offset >> 8) & 0xff;
        tmpbuf[7] = (__usb_offset >> 16) & 0xff;
        tmpbuf[8] = (__usb_offset >> 24) & 0xff;

        // COPY THE BYTES

        {
            int k;
            for(k = 0; k < USB_DATASIZE; ++k) {
                if(k < bufbytes) {
                    tmpbuf[9 + k] = __usb_rxtxbuffer[__usb_rxtxbottom];
                    __usb_crc32 = usb_crc32roll(__usb_crc32, __usb_rxtxbuffer + __usb_rxtxbottom, 1);   // UPDATE THE CRC32
                    ++__usb_rxtxbottom;
                    if(__usb_rxtxbottom >= RING_BUFFER_SIZE)
                        __usb_rxtxbottom -= RING_BUFFER_SIZE;

                }
                else
                    tmpbuf[9 + k] = 0;
            }
        }

        if(__usb_curdevice)
            hid_write(__usb_curdevice, tmpbuf, RAWHID_TX_SIZE + 1);

        __usb_offset += bufbytes;
        __usb_txseq = p_type & 0x1f;
        if(eof) {
            usb_sendcontrolpacket(P_TYPE_ENDOFFILE);
            usb_mutex_lock();
            __usb_drvstatus &= ~USB_STATUS_TXDATA;
            usb_mutex_unlock();
            // DONE SENDING ALL DATA

        }
        else {
            // AT EACH END OF FRAGMENT, SEND A CHECKPOINT BUT NOT AT END OF FILE
            if(p_type & 0x40)
                usb_sendcontrolpacket(P_TYPE_CHECKPOINT);

            // IF WE CONSUMED ALL THE BUFFER, SIGNAL THAT WE ARE DONE
            if(__usb_rxtxtop == __usb_rxtxbottom) {
                usb_mutex_lock();
                __usb_drvstatus &= ~USB_STATUS_TXDATA;
                usb_mutex_unlock();
            }

        }

        return;
    }

    // NOTHING TO TRANSMIT

}

// RECEIVE BYTES FROM THE HOST IN EP2 ENDPOINT
// FOR USE WITHIN ISR

void usb_ep2_receive()
{

    if(!(__usb_drvstatus & USB_STATUS_CONNECTED))
        return;

    BYTE tmpbuf[RAWHID_RX_SIZE + 1];
    int fifocnt;
    fifocnt = hid_read_timeout(__usb_curdevice, tmpbuf, RAWHID_RX_SIZE, 0);

    if(fifocnt <= 0) {
        usb_mutex_lock();
        if(fifocnt == -1)
            __usb_drvstatus &= ~(USB_STATUS_CONNECTED | USB_STATUS_CONFIGURED);
        // THERE'S NO PACKETS AVAILABLE
        __usb_drvstatus &= ~USB_STATUS_NOWAIT;  // GO TO SLEEP IF NEEDED, NOTHING ON THE WIRE
        usb_mutex_unlock();
        return;
    }

    usb_mutex_lock();
    __usb_drvstatus |= USB_STATUS_NOWAIT;       // THERE COULD BE MORE DATA, DON'T SLEEP UNTIL ALL DATA IS RETRIEVED
    usb_mutex_unlock();

    int cnt = 0;

    // READ PACKET TYPE
    int p_type = tmpbuf[0];
    BYTEPTR rcvbuf;

    if(p_type & 0x80) {
        usb_mutex_lock();
        // THIS IS A CONTROL PACKET
        // PUT IT IN THE CTL BUFFER AND NOTIFY THE USER
        __usb_drvstatus |= USB_STATUS_RXCTL;
        usb_mutex_unlock();

        rcvbuf = __usb_ctlrxbuffer;
        cnt = 1;
        rcvbuf[0] = (BYTE) p_type;
        ++rcvbuf;
        while(cnt < fifocnt) {
            *rcvbuf = tmpbuf[cnt];
            ++cnt;
            ++rcvbuf;
        }

        usb_receivecontrolpacket();
        return;

    }

    // THIS IS A DATA PACKET
    // READ DATA DIRECTLY INTO THE BUFFER
    rcvbuf = __usb_tmprxbuffer;
    cnt = 1;
    rcvbuf[0] = (BYTE) p_type;
    ++rcvbuf;
    while(cnt < 8) {
        *rcvbuf = tmpbuf[cnt];
        ++cnt;
        ++rcvbuf;
    }

    USB_PACKET *pptr = (USB_PACKET *) __usb_tmprxbuffer;

    // IS IT FROM THE SAME FILE?
    if(P_FILEID(pptr) != __usb_fileid) {
        // DIFFERENT FILE? IGNORE
        // FLUSH THE FIFO, IGNORE THE PACKET
        return;
    }

    // IS THE CORRECT OFFSET?
    if(pptr->p_offset != __usb_offset) {
        //  WE MUST'VE MISSED SOMETHING

        if(!(__usb_drvstatus & USB_STATUS_ERROR)) {
            usb_mutex_lock();
            __usb_drvstatus |= USB_STATUS_ERROR;
            usb_mutex_unlock();
            // SEND A REPORT NOW IF POSSIBLE, OTHERWISE THE ERROR INFO WILL GO IN THE NEXT REPORT
            if(!(__usb_drvstatus & USB_STATUS_TXCTL))
                usb_sendcontrolpacket(P_TYPE_REPORT);
        }

        // FLUSH THE FIFO, IGNORE THE PACKET
        return;

    }
    else {
        // GOT THE RIGHT OFFSET, RESET ERROR CONDITION IF ANY
        if(__usb_drvstatus & USB_STATUS_ERROR) {
            usb_mutex_lock();
            // CRC AND OFFSET SHOULD BE CORRECTLY SET BY NOW
            // SO LIFT THE ERROR CONDITION
            __usb_drvstatus &= ~USB_STATUS_ERROR;
            usb_mutex_unlock();
        }
    }

    // DO WE HAVE ENOUGH ROOM AVAILABLE?
    int usedspace = __usb_rxtxtop - __usb_rxtxbottom;
    if(usedspace < 0)
        usedspace += RING_BUFFER_SIZE;

    if(pptr->p_dataused > RING_BUFFER_SIZE - usedspace) {
        usb_mutex_lock();
        // DATA WON'T FIT IN THE BUFFER DUE TO OVERFLOW, ISSUE AN ERROR AND REQUEST RESEND
        __usb_drvstatus |= USB_STATUS_ERROR;
        usb_mutex_unlock();
        // SEND A REPORT NOW IF POSSIBLE, OTHERWISE THE ERROR INFO WILL GO IN THE NEXT REPORT
        if(!(__usb_drvstatus & USB_STATUS_TXCTL))
            usb_sendcontrolpacket(P_TYPE_REPORT);

        // FLUSH THE FIFO, DISCARD THE DATA
        return;

    }

    // NEW PACKET IS READY TO BE RECEIVED, IF WE ARE STARTING A NEW FRAGMENT
    // SAVE RESTORE POINT TO REQUEST THE FRAGMENT TO BE RESENT IF IT FAILS
    if(p_type==1) {
        __usb_lastgood_offset = __usb_offset;
        __usb_lastgood_crc = __usb_crc32;
    }

    // WE HAVE NEW DATA, RECEIVE IT DIRECTLY AT THE BUFFER
    rcvbuf = __usb_rxtxbuffer + __usb_rxtxtop;

    while((cnt < fifocnt) && (cnt < pptr->p_dataused + 8)) {
        *rcvbuf = tmpbuf[cnt];
        ++cnt;
        ++rcvbuf;
        if(rcvbuf == __usb_rxtxbuffer + RING_BUFFER_SIZE)
            rcvbuf -= RING_BUFFER_SIZE;

    }

    // UPDATE THE CRC
    __usb_crc32 = usb_crc32roll(__usb_crc32, tmpbuf + 8, pptr->p_dataused);

    // UPDATE THE BUFFERS
    __usb_rxtxtop += pptr->p_dataused;
    if(__usb_rxtxtop >= RING_BUFFER_SIZE)
        __usb_rxtxtop -= RING_BUFFER_SIZE;
    __usb_offset += pptr->p_dataused;
    usedspace += pptr->p_dataused;

    if(usedspace >= RING_BUFFER_SIZE / 2) {
        usb_mutex_lock();
        __usb_drvstatus |= USB_STATUS_HALT;     // REQUEST HALT IF BUFFER IS HALF-FULL
        usb_mutex_unlock();
        // SEND A REPORT NOW IF POSSIBLE, OTHERWISE THE ERROR INFO WILL GO IN THE NEXT REPORT
        if(!(__usb_drvstatus & USB_STATUS_TXCTL))
            usb_sendcontrolpacket(P_TYPE_REPORT);
    }

    usb_mutex_lock();
    __usb_drvstatus |= USB_STATUS_RXDATA;       // AND SIGNAL THAT WE HAVE DATA AVAILABLE
    usb_mutex_unlock();
}

// SENDING INTERRUPT ENDPOINT
void ep1_irqservice()
{
    usb_ep1_transmit();
}

// RECEIVING DATA ENDPOINT
void ep2_irqservice()
{
    // ONLY RESPOND ONCE EVERYTHING IS SETUP
    if(!(__usb_drvstatus & USB_STATUS_CONFIGURED))
        return;

    // WE HAVE A PACKET, GO PROCESS IT
    usb_ep2_receive();

}

// GENERAL INTERRUPT SERVICE ROUTINE - DISPATCH TO INDIVIDUAL ENDPOINT ROUTINES
void usb_irqservice()
{
    if(__usb_drvstatus & USB_STATUS_CONNECTNOW) {
        // WE HAVE A PATH, TRY TO OPEN THE DEVICE
        __usb_curdevice = hid_open_path((const char *)__usb_devicepath);
        usb_mutex_lock();
        if(!__usb_curdevice)
            __usb_devicepath[0] = 0;
        else
            __usb_drvstatus |= USB_STATUS_CONNECTED | USB_STATUS_CONFIGURED;
        __usb_drvstatus &= ~USB_STATUS_CONNECTNOW;
        usb_mutex_unlock();
        return;
    }

    if(__usb_drvstatus & USB_STATUS_DISCONNECTNOW) {
        // WE HAVE A PATH, TRY TO OPEN THE DEVICE
        if(__usb_curdevice)
            hid_close(__usb_curdevice);
        __usb_curdevice = 0;
        __usb_devicepath[0] = 0;
        hid_exit();
        usb_mutex_lock();
        __usb_drvstatus &= ~USB_STATUS_DISCONNECTNOW;
        usb_mutex_unlock();
        return;
    }

    if(!((__usb_drvstatus & (USB_STATUS_INIT | USB_STATUS_CONNECTED |
                        USB_STATUS_CONFIGURED)) ==
                (USB_STATUS_INIT | USB_STATUS_CONNECTED |
                    USB_STATUS_CONFIGURED)))
        return;

    if(!__usb_curdevice)
        return; // SHOULDN'T HAPPEN, BUT JUST IN CASE A THREAD CLOSED THE HANDLE AFTER WE ENTERED HERE

    usb_mutex_lock();
    __usb_drvstatus |= USB_STATUS_INSIDEIRQ;
    usb_mutex_unlock();

    ep1_irqservice();

    ep2_irqservice();

    usb_mutex_lock();
    __usb_drvstatus &= ~USB_STATUS_INSIDEIRQ;
    usb_mutex_unlock();

    return;
}

void usb_init_data_transfer()
{}
