/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include <newrpl.h>
#include <ui.h>

#include "hidapi.h"

// ADD-ON MEMORY ALLOCATOR - SHARED WITH FILE SYSTEM AND NEWRPL ENGINE
extern void init_simpalloc();
extern unsigned int *simpmalloc(int words);
extern unsigned char *simpmallocb(int bytes);
extern void simpfree(void *voidptr);

#define RAWHID_TX_SIZE  64
#define RAWHID_RX_SIZE  64
// AN ARBITRARY MARKER
#define USB_BLOCKSTART_MARKER 0xab


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

// FOR PCS USE HIDAPI LIBRARY TO EXPOSE THE CONNECTED DEVICE AS A HOST
hid_device *__usb_curdevice;
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

WORD usb_crc32(BYTEPTR data,BINT len)
{
    WORD crc=-1;
    while(len--) crc=__crctable[(crc ^ *data++) & 0xFF] ^ (crc >> 8);
    return crc^(-1);
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

void usb_irqservice();

void usb_init(int force)
{

    if(!force && (__usb_drvstatus&USB_STATUS_INIT)) return;

    __usb_drvstatus=USB_STATUS_INIT;

    if(__usb_curdevice) __usb_drvstatus|=USB_STATUS_CONNECTED|USB_STATUS_CONFIGURED;

}

void usb_shutdown()
{

    if(!__usb_drvstatus&USB_STATUS_INIT) return;

    if(__usb_drvstatus&USB_STATUS_DATAREADY) {
        usb_releasedata();
    }

    if(__usb_curdevice) {
        hid_close(__usb_curdevice);
        __usb_curdevice=0;
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


// GENERAL INTERRUPT SERVICE ROUTINE - DISPATCH TO INDIVIDUAL ENDPOINT ROUTINES
void usb_irqservice()
{

    if(!__usb_curdevice) usb_shutdown();

    if(!(__usb_drvstatus&USB_STATUS_INIT)) usb_init(0);

    if(!(__usb_drvstatus&USB_STATUS_CONFIGURED)) return;

    if(__usb_drvstatus&USB_STATUS_DATAREADY) {
        // PREVIOUS DATA WASN RETRIEVED!
        // STALL UNTIL USER RETRIEVES
        return;
    }

    int fifocnt=hid_read_timeout(__usb_curdevice,__usb_tmpbuffer,RAWHID_RX_SIZE,1);

    if(fifocnt<0) {
        // SOME KIND OF ERROR - DISCONNECT
        hid_close(__usb_curdevice);
        __usb_curdevice=0;
        usb_shutdown();
        return;
    }

    int cnt=0;
    if(!(__usb_drvstatus&USB_STATUS_HIDRX)) {
        // IF THERE'S ANY DATA - START TO RECEIVE A NEW DATA BLOCK


            __usb_rcvtotal=0;
            __usb_rcvpartial=0;
            __usb_rcvcrc=0;



            // WE ARE READY TO RECEIVE A NEW DATA BLOCK

            if(fifocnt>8) {
            // READ THE HEADER
            WORD startmarker=__usb_tmpbuffer[0];

            if(startmarker==USB_BLOCKSTART_MARKER) {
            __usb_rcvtotal=__usb_tmpbuffer[1];
            __usb_rcvtotal|=__usb_tmpbuffer[2]<<8;
            __usb_rcvtotal|=__usb_tmpbuffer[3]<<16;

            __usb_rcvpartial=-8;
            __usb_rcvcrc=__usb_tmpbuffer[4];
            __usb_rcvcrc|=__usb_tmpbuffer[5]<<8;
            __usb_rcvcrc|=__usb_tmpbuffer[6]<<16;
            __usb_rcvcrc|=__usb_tmpbuffer[7]<<24;

            cnt+=8;

            BYTEPTR buf;

            // ONLY ALLOCATE MEMORY FOR DATA BLOCKS LARGER THAN 1 PACKET
            if(__usb_rcvtotal>RAWHID_RX_SIZE-8) buf=simpmallocb(__usb_rcvtotal);
            else buf=__usb_tmpbuffer;

            if(!buf) {
                __usb_rcvbuffer=__usb_tmpbuffer;
            } else {
                __usb_rcvbuffer=buf;
                __usb_bufptr[2]=__usb_rcvbuffer;
            }

            memmoveb(__usb_tmpbuffer,__usb_tmpbuffer+8,fifocnt-8);

            }

            else {
                // BAD START MARKER, TREAT THE BLOCK AS ARBITRARY DATA
                cnt+=fifocnt;
                __usb_rcvtotal=fifocnt;
                __usb_rcvbuffer=__usb_tmpbuffer;
                __usb_rcvcrc=0;
                __usb_rcvpartial=0;
                __usb_bufptr[2]=__usb_rcvbuffer+fifocnt;

            }




            }
            else {
                // NO PACKET STARTER - TREAT AS ARBITRARY DATA
                __usb_rcvtotal=fifocnt;
                __usb_rcvbuffer=__usb_tmpbuffer;
                __usb_rcvcrc=0;
                __usb_rcvpartial=0;
                __usb_bufptr[2]=__usb_rcvbuffer;
            }

       }
        else  if(__usb_rcvbuffer==__usb_tmpbuffer)  __usb_bufptr[2]=__usb_tmpbuffer;    // IF WE HAD NO MEMORY, RESET THE BUFFER FOR EVERY PARTIAL PACKET

        if(__usb_bufptr[2]!=__usb_tmpbuffer) {
        BYTEPTR tmpptr=__usb_tmpbuffer;
        while(cnt<fifocnt) {
            *__usb_bufptr[2]=*tmpptr;
            ++__usb_bufptr[2];
            ++tmpptr;
            ++cnt;
        }
        } else {
            __usb_bufptr[2]+=fifocnt-cnt;
            cnt=fifocnt;
        }

        __usb_rcvpartial+=fifocnt;

        if((fifocnt!=RAWHID_RX_SIZE)||(__usb_rcvpartial>=__usb_rcvtotal)) {
            // PARTIAL PACKET SIGNALS END OF TRANSMISSION

            // IF WE HAD NO MEMORY TO ALLOCATE A BLOCK, INDICATE THAT WE RECEIVED ONLY THE LAST PARTIAL PACKET
            if(__usb_rcvbuffer==__usb_tmpbuffer) {
                __usb_rcvpartial=__usb_bufptr[2]-__usb_rcvbuffer;
            }
            __usb_drvstatus&=~USB_STATUS_HIDRX;
            if(__usb_rcvtotal!=0)
                __usb_drvstatus|=USB_STATUS_DATAREADY;
            return;
        }

        __usb_drvstatus|=USB_STATUS_HIDRX;





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

// CHECK THE CRC OF THE LAST BLOCK RECEIVED
int usb_checkcrc()
{
    if(!(__usb_drvstatus&USB_STATUS_DATAREADY)) return 0;
    WORD rcvdcrc=usb_crc32(__usb_rcvbuffer,(__usb_rcvpartial>__usb_rcvtotal)? __usb_rcvtotal:__usb_rcvpartial);

    if(rcvdcrc!=__usb_rcvcrc) return 0;
    return 1;
}


// HIGH LEVEL FUNCTION TO SEND ANYTHING TO THE OTHER SIDE
int usb_transmitdata(BYTEPTR data,BINT size)
{
    if(!usb_isconfigured()) return 0;

    if(!__usb_curdevice) {
        __usb_drvstatus=USB_STATUS_INIT;
        return 0;
    }
    int err;

    if(size<=0) return 0;   // BAD SIZE

    BYTEPTR buf;

    // ONLY ALLOCATE MEMORY FOR DATA BLOCKS LARGER THAN 1 PACKET
    if(size>RAWHID_TX_SIZE-8) buf=simpmallocb(size+8);
    else buf=__usb_tmpbuffer;

    if(!buf) return 0;      // FAILED TO SEND - NOT ENOUGH MEMORY

    __usb_bufptr[1]=buf;

    __usb_count[1]=size+8;

    buf[0]=USB_BLOCKSTART_MARKER;        // START OF BLOCK MARKER
    buf[1]=size&0xff;
    buf[2]=(size>>8)&0xff;
    buf[3]=(size>>16)&0xff;

    WORD crc=usb_crc32(data,size);

    buf[4]=crc&0xff;
    buf[5]=(crc>>8)&0xff;
    buf[6]=(crc>>16)&0xff;
    buf[7]=(crc>>24)&0xff;

    memmoveb(buf+8,data,size);



    err=hid_write(__usb_curdevice,__usb_bufptr[1],__usb_count[1]);

    if(buf!=__usb_tmpbuffer) simpfree(buf);

    if(err<=0) {
        // CANNOT WRITE TO USB? - DISCONNECT
        hid_close(__usb_curdevice);
        __usb_curdevice=0;
        usb_shutdown();
        return 0;
    }

    return 1;

}
