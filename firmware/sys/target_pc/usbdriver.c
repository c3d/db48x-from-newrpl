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



// FOR PCS USE HIDAPI LIBRARY TO EXPOSE THE CONNECTED DEVICE AS A HOST
hid_device *__usb_curdevice;
// FOR PCS OR SYSTEMS WHERE THE DRIVER WILL RUN ON A SEPARATE THREAD
int __usb_paused;

// GLOBAL VARIABLES OF THE USB SUBSYSTEM
volatile BINT __usb_drvstatus __SYSTEM_GLOBAL__; // FLAGS TO INDICATE IF INITIALIZED, CONNECTED, SENDING/RECEIVING, ETC.
BYTE *__usb_bufptr[3] __SYSTEM_GLOBAL__;   // POINTERS TO BUFFERS FOR EACH ENDPOINT (0/1/2)
BINT __usb_count[3]   __SYSTEM_GLOBAL__;   // REMAINING BYTES TO TRANSFER ON EACH ENDPOINT (0/1/2)
BINT __usb_padding[3] __SYSTEM_GLOBAL__;    // PADDING FOR OUTGOING TRANSFERS
BYTE __usb_rxtmpbuffer[RAWHID_TX_SIZE+1] __SYSTEM_GLOBAL__;  // TEMPORARY BUFFER FOR NON-BLOCKING CONTROL TRANSFERS
BYTE __usb_txtmpbuffer[RAWHID_TX_SIZE+1] __SYSTEM_GLOBAL__;  // TEMPORARY BUFFER FOR NON-BLOCKING CONTROL TRANSFERS
BYTEPTR __usb_rcvbuffer;
WORD __usb_rcvtotal __SYSTEM_GLOBAL__;
WORD __usb_rcvpartial __SYSTEM_GLOBAL__;
WORD __usb_rcvcrc __SYSTEM_GLOBAL__;
WORD __usb_rcvcrcroll __SYSTEM_GLOBAL__;
BINT __usb_rcvblkmark __SYSTEM_GLOBAL__;    // TYPE OF RECEIVED BLOCK (ONE OF USB_BLOCKMARK_XXX CONSTANTS)
BYTEPTR __usb_sndbuffer __SYSTEM_GLOBAL__;
BINT __usb_sndtotal __SYSTEM_GLOBAL__;
BINT __usb_sndpartial __SYSTEM_GLOBAL__;
volatile BINT __usb_rembigtotal __SYSTEM_GLOBAL__;
volatile BINT __usb_rembigoffset __SYSTEM_GLOBAL__;
BINT __usb_localbigoffset __SYSTEM_GLOBAL__;


// GLOBAL VARIABLES FOR DOUBLE BUFFERED LONG TRANSACTIONS
BYTEPTR __usb_longbuffer[2];              // DOUBLE BUFFERING FOR LONG TRANSMISSIONS OF DATA
BINT __usb_longoffset;
BINT __usb_longactbuffer;                 // WHICH BUFFER IS BEING WRITTEN
BINT __usb_longlastsize;                  // LAST BLOCK SIZE IN A LONG TRANSMISSION
WORD __usb_longcrcroll;



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

WORD usb_crc32roll(WORD oldcrc,BYTEPTR data,BINT len)
{
    WORD crc=oldcrc^(-1);
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

    if(!(__usb_drvstatus&USB_STATUS_INIT)) return;

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
do {
    if(!__usb_curdevice) { usb_shutdown(); return; }

    if(!(__usb_drvstatus&USB_STATUS_INIT)) usb_init(0);

    if(!(__usb_drvstatus&USB_STATUS_CONFIGURED)) return;

    if(__usb_drvstatus&USB_STATUS_DATAREADY) {
        // PREVIOUS DATA WASN RETRIEVED!
        // STALL UNTIL USER RETRIEVES
        return;
    }

    if(__usb_drvstatus&USB_STATUS_DATARECEIVED) {
        // SEND ACKNOWLEDGEMENT THAT WE RECEIVED SOME DATA AND WE WANT THE NEXT OFFSET
        BYTE tmpbuf[RAWHID_TX_SIZE+1];

        memsetb(tmpbuf,0,RAWHID_TX_SIZE+1);

        tmpbuf[0]=0;        // REPORT ID
        tmpbuf[1]=USB_BLOCKMARK_RESPONSE;
        tmpbuf[2]=(__usb_drvstatus&USB_STATUS_DATAREADY)? 1:0;
        tmpbuf[3]=__usb_rembigtotal&0xff;
        tmpbuf[4]=(__usb_rembigtotal>>8)&0xff;
        tmpbuf[5]=(__usb_rembigtotal>>16)&0xff;
        tmpbuf[6]=(__usb_rembigtotal>>24)&0xff;
        tmpbuf[7]=__usb_localbigoffset&0xff;
        tmpbuf[8]=(__usb_localbigoffset>>8)&0xff;
        tmpbuf[9]=(__usb_localbigoffset>>16)&0xff;
        tmpbuf[10]=(__usb_localbigoffset>>24)&0xff;

        hid_write(__usb_curdevice,tmpbuf,RAWHID_TX_SIZE+1);

        __usb_drvstatus&=~USB_STATUS_DATARECEIVED;
        continue;
    }


    int fifocnt=hid_read_timeout(__usb_curdevice,__usb_rxtmpbuffer,RAWHID_RX_SIZE,1);

    if(fifocnt<0) {
        // SOME KIND OF ERROR - DISCONNECT
        hid_close(__usb_curdevice);
        __usb_curdevice=0;
        usb_shutdown();
        return;
    }

    if(fifocnt==0)  // EITHER TIMEOUT OR NO DATA
        return;
    int cnt=0;
    if(!(__usb_drvstatus&USB_STATUS_HIDRX)) {
        // IF THERE'S ANY DATA - START TO RECEIVE A NEW DATA BLOCK


            __usb_rcvtotal=0;
            __usb_rcvpartial=0;
            __usb_rcvcrc=0;




            // WE ARE READY TO RECEIVE A NEW DATA BLOCK

            if(fifocnt>8) {
            // READ THE HEADER
            WORD startmarker=__usb_rxtmpbuffer[0];

            if(startmarker==USB_BLOCKMARK_GETSTATUS) {
                // REQUESTED INFO, RESPOND EVEN IF PREVIOUS DATA WASN'T RETRIEVED

                WORD txsize=__usb_rxtmpbuffer[1];
                txsize|=__usb_rxtmpbuffer[2]<<8;
                txsize|=__usb_rxtmpbuffer[3]<<16;
                txsize|=__usb_rxtmpbuffer[4]<<24;
                fifocnt-=4;

                WORD txoffset=__usb_rxtmpbuffer[5];
                txoffset|=(__usb_rxtmpbuffer[6])<<8;
                txoffset|=(__usb_rxtmpbuffer[7])<<16;
                txoffset|=(__usb_rxtmpbuffer[8])<<24;
                fifocnt-=4;

                __usb_rembigtotal=txsize;
                __usb_rembigoffset=txoffset;

                __usb_rcvcrcroll=0;


                BYTE tmpbuf[RAWHID_TX_SIZE+1];

                memsetb(tmpbuf,0,RAWHID_TX_SIZE+1);

                tmpbuf[0]=0;        // REPORT ID
                tmpbuf[1]=USB_BLOCKMARK_RESPONSE;
                tmpbuf[2]=(__usb_drvstatus&USB_STATUS_DATAREADY)? 1:0;
                tmpbuf[3]=__usb_rembigtotal&0xff;
                tmpbuf[4]=(__usb_rembigtotal>>8)&0xff;
                tmpbuf[5]=(__usb_rembigtotal>>16)&0xff;
                tmpbuf[6]=(__usb_rembigtotal>>24)&0xff;
                tmpbuf[7]=__usb_localbigoffset&0xff;
                tmpbuf[8]=(__usb_localbigoffset>>8)&0xff;
                tmpbuf[9]=(__usb_localbigoffset>>16)&0xff;
                tmpbuf[10]=(__usb_localbigoffset>>24)&0xff;

                hid_write(__usb_curdevice,tmpbuf,RAWHID_TX_SIZE+1);

                __usb_drvstatus&=~USB_STATUS_HIDRX;
                return;

            }

            if(startmarker==USB_BLOCKMARK_RESPONSE) {
                // RECEIVING A RESPONSE FROM THE REMOTE


                BYTE remotebusy=__usb_rxtmpbuffer[1];    // READ THE NEXT BYTE WITH THE STATUS

                if(!remotebusy) __usb_drvstatus&=~USB_STATUS_REMOTEBUSY;
                else __usb_drvstatus|=USB_STATUS_REMOTEBUSY;

                WORD txsize=__usb_rxtmpbuffer[2];
                txsize|=(__usb_rxtmpbuffer[3])<<8;
                txsize|=(__usb_rxtmpbuffer[4])<<16;
                txsize|=(__usb_rxtmpbuffer[5])<<24;


                WORD txoffset=__usb_rxtmpbuffer[6];
                txoffset|=(__usb_rxtmpbuffer[7])<<8;
                txoffset|=(__usb_rxtmpbuffer[8])<<16;
                txoffset|=(__usb_rxtmpbuffer[9])<<24;


                __usb_rembigtotal=txsize;
                __usb_rembigoffset=txoffset;    // FILE SIZE AND OFFSET AS REQUESTED BY THE REMOTE

                __usb_drvstatus|=USB_STATUS_REMOTERESPND;

                __usb_drvstatus&=~USB_STATUS_HIDRX;
                return;


            }


            if((startmarker&0xf0)==USB_BLOCKMARK_SINGLE) {
            __usb_rcvpartial=fifocnt;
            __usb_rcvtotal=__usb_rxtmpbuffer[1];
            __usb_rcvtotal|=__usb_rxtmpbuffer[2]<<8;
            __usb_rcvtotal|=__usb_rxtmpbuffer[3]<<16;

            __usb_rcvpartial=-8;
            __usb_rcvcrc=__usb_rxtmpbuffer[4];
            __usb_rcvcrc|=__usb_rxtmpbuffer[5]<<8;
            __usb_rcvcrc|=__usb_rxtmpbuffer[6]<<16;
            __usb_rcvcrc|=__usb_rxtmpbuffer[7]<<24;

            __usb_rcvblkmark=startmarker;



            cnt+=8;

            BYTEPTR buf;

            // ONLY ALLOCATE MEMORY FOR DATA BLOCKS LARGER THAN 1 PACKET
            buf=simpmallocb((__usb_rcvpartial<__usb_rcvtotal)? __usb_rcvtotal:__usb_rcvpartial);


            if(!buf) {
                __usb_rcvbuffer=__usb_rxtmpbuffer;
            } else {
                __usb_rcvbuffer=buf;
                __usb_bufptr[2]=__usb_rcvbuffer;
            }

            // IF THIS IS THE START OF A NEW TRANSMISSION, DON'T IGNORE IT
            if((__usb_rcvblkmark==USB_BLOCKMARK_SINGLE)||(__usb_rcvblkmark==USB_BLOCKMARK_MULTISTART)) { __usb_drvstatus&=~USB_STATUS_IGNORE; __usb_rcvcrcroll=0; }

            memmoveb(__usb_rxtmpbuffer,__usb_rxtmpbuffer+8,fifocnt-8);

            }

            else {
                // BAD START MARKER, TREAT THE BLOCK AS ARBITRARY DATA
                cnt++;
                __usb_rcvtotal=fifocnt;
                __usb_rcvbuffer=__usb_rxtmpbuffer;
                __usb_rcvcrc=0;
                __usb_rcvpartial=0;
                __usb_bufptr[2]=__usb_rcvbuffer+fifocnt;
                __usb_rcvblkmark=0;

                __usb_drvstatus|=USB_STATUS_IGNORE;
            }




            }
            else {
                // NO PACKET STARTER - TREAT AS ARBITRARY DATA
                __usb_rcvtotal=fifocnt;
                __usb_rcvbuffer=__usb_rxtmpbuffer;
                __usb_rcvcrc=0;
                __usb_rcvpartial=0;
                __usb_bufptr[2]=__usb_rcvbuffer;
                __usb_rcvblkmark=0;
                __usb_drvstatus|=USB_STATUS_IGNORE;

            }

       }
        else  if(__usb_rcvbuffer==__usb_rxtmpbuffer)  __usb_bufptr[2]=__usb_rxtmpbuffer;    // IF WE HAD NO MEMORY, RESET THE BUFFER FOR EVERY PARTIAL PACKET

        if(__usb_bufptr[2]!=__usb_rxtmpbuffer) {
        BYTEPTR tmpptr=__usb_rxtmpbuffer;
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
            if(__usb_rcvbuffer==__usb_rxtmpbuffer) {
                __usb_rcvpartial=__usb_bufptr[2]-__usb_rcvbuffer;
            }
            __usb_drvstatus&=~USB_STATUS_HIDRX;
            if(!(__usb_drvstatus&USB_STATUS_IGNORE)) { if(__usb_rcvtotal>0) __usb_drvstatus|=USB_STATUS_DATAREADY; }
            else {
             // RELEASE THE BUFFER, THE USER WILL NEVER SEE IT
                if(__usb_rcvbuffer!=__usb_rxtmpbuffer) simpfree(__usb_rcvbuffer);
            }
            if((__usb_rcvblkmark==0)||(__usb_rcvblkmark==USB_BLOCKMARK_SINGLE)||(__usb_rcvblkmark==USB_BLOCKMARK_MULTIEND))  __usb_drvstatus&=~USB_STATUS_IGNORE;    // STOP IGNORING PACKETS AFTER THIS ONE

            return;
        }

        __usb_drvstatus|=USB_STATUS_HIDRX;


} while(1); // KEEP PROCESSING UNTIL THERE'S NO MORE PACKETS


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

// HIGH LEVEL FUNCTION TO BLOCK UNTIL DATA ARRIVES
// RETURN 0 IF TIMEOUT
int usb_waitfordata()
{
    tmr_t start=tmr_ticks(),end;

    while(!(__usb_drvstatus&USB_STATUS_DATAREADY)) {
        if((__usb_drvstatus&(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED))!=(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED)) return 0;

        cpu_waitforinterrupt();

        end=tmr_ticks();
        if(tmr_ticks2ms(start,end)>USB_TIMEOUT_MS) {
            // MORE THAN 1 SECOND TO SEND 1 BLOCK? TIMEOUT - CLEANUP AND RETURN
            return 0;
        }
    }
    return 1;
}

int usb_datablocktype()
{
    if(!(__usb_drvstatus&USB_STATUS_DATAREADY)) return 0;
    return __usb_rcvblkmark;

}




// HIGH LEVEL FUNCTION TO ACCESS A BLOCK OF DATA
BYTEPTR usb_accessdata(int *blksize)
{
    if(!(__usb_drvstatus&USB_STATUS_DATAREADY)) return 0;
    if(blksize) *blksize=(__usb_rcvpartial>__usb_rcvtotal)? __usb_rcvtotal:__usb_rcvpartial;
    return __usb_rcvbuffer;
}

// HIGH LEVEL FUNCTION TO RELEASE A BLOCK OF DATA AND GET READY TO RECEIVE THE NEXT
void usb_releasedata()
{
    if(!(__usb_drvstatus&USB_STATUS_DATAREADY)) return;
    if(__usb_rcvbuffer!=__usb_rxtmpbuffer) simpfree(__usb_rcvbuffer);

    __usb_drvstatus&=~USB_STATUS_DATAREADY;
}


// SEND SOME FEEDBACK TO THE HOST ABOUT THE CURRENT TRANSMISSION
void usb_sendfeedback()
{
    __usb_drvstatus|=USB_STATUS_DATARECEIVED;
}

void usb_restartcrc()
{
    __usb_rcvcrcroll=0;
}
// CHECK THE CRC OF THE LAST BLOCK RECEIVED
int usb_checkcrc()
{
    if(!(__usb_drvstatus&USB_STATUS_DATAREADY)) return 0;
    WORD rcvdcrc=usb_crc32roll(__usb_rcvcrcroll,__usb_rcvbuffer,(__usb_rcvpartial>__usb_rcvtotal)? __usb_rcvtotal:__usb_rcvpartial);
    __usb_rcvcrcroll=rcvdcrc;
    if(rcvdcrc!=__usb_rcvcrc)
        return 0;
    return 1;
}

// RETURN TRUE IF THE REMOTE SIDE IS READY TO RECEIVE A NEW TRANSMISSION
int usb_remoteready(int size,int offset)
{
    if(!usb_isconfigured())
        return 0;

    if(__usb_drvstatus&(USB_STATUS_HIDRX|USB_STATUS_HIDTX))
        return 0;

    __usb_drvstatus|=USB_STATUS_REMOTEBUSY;
    __usb_drvstatus&=~USB_STATUS_REMOTERESPND;


    BYTE tmpbuf[RAWHID_TX_SIZE+1];

    memsetb(tmpbuf,0,RAWHID_TX_SIZE+1);

    tmpbuf[1]=USB_BLOCKMARK_GETSTATUS;
    tmpbuf[2]=size&0xff;
    tmpbuf[3]=(size>>8)&0xff;
    tmpbuf[4]=(size>>16)&0xff;
    tmpbuf[5]=(size>>24)&0xff;
    tmpbuf[6]=offset&0xff;
    tmpbuf[7]=(offset>>8)&0xff;
    tmpbuf[8]=(offset>>16)&0xff;
    tmpbuf[9]=(offset>>24)&0xff;

    // PAUSE THE OTHER THREAD TO MAKE SURE WE DON'T INTERFERE
    int oldpaused=__usb_paused;
    __usb_paused=1;
    while(__usb_paused>=0) ;

    int err=hid_write(__usb_curdevice,tmpbuf,RAWHID_TX_SIZE+1);


    if(err<=0)
        return 0; // NOT READY - ERROR

    err=hid_read_timeout(__usb_curdevice,tmpbuf,RAWHID_RX_SIZE,USB_TIMEOUT_MS);

    if(err<=0)
        return 0; // NOT READY - ERROR

    if(tmpbuf[0]!=USB_BLOCKMARK_RESPONSE)
        return 0;

    // RECEIVING A RESPONSE FROM THE REMOTE


    BYTE remotebusy=tmpbuf[1];    // READ THE NEXT BYTE WITH THE STATUS

    if(!remotebusy) __usb_drvstatus&=~USB_STATUS_REMOTEBUSY;
    else __usb_drvstatus|=USB_STATUS_REMOTEBUSY;

    WORD txsize=tmpbuf[2];
    txsize|=(tmpbuf[3])<<8;
    txsize|=(tmpbuf[4])<<16;
    txsize|=(tmpbuf[5])<<24;


    WORD txoffset=tmpbuf[6];
    txoffset|=(tmpbuf[7])<<8;
    txoffset|=(tmpbuf[8])<<16;
    txoffset|=(tmpbuf[9])<<24;


    __usb_rembigtotal=txsize;
    __usb_rembigoffset=txoffset;    // FILE SIZE AND OFFSET AS REQUESTED BY THE REMOTE

    __usb_drvstatus|=USB_STATUS_REMOTERESPND;

    __usb_drvstatus&=~USB_STATUS_HIDRX;

    __usb_paused=oldpaused;


    return !(int)remotebusy;

}

// HIGH LEVEL FUNCTION TO SEND ANYTHING TO THE OTHER SIDE
int usb_transmitdata(BYTEPTR data,BINT size)
{

    // MAKE SURE THERE'S NO OTHER PENDING TRANSMISSIONS AND EVERYTHING IS CONNECTED
    if((__usb_drvstatus&0xff)!= ( USB_STATUS_CONFIGURED
                                 |USB_STATUS_CONNECTED
                                 |USB_STATUS_INIT       ) ) return 0;

    if(size<=0) return 0;   // BAD SIZE


    BINT blksize,sent=0,bufsize=0,firstblock=1;
    BYTEPTR buf=0;
    WORD crcroll=0;


    __usb_rembigoffset=0;


    if(!usb_remoteready(size,sent))
    {
      __usb_drvstatus&=~USB_STATUS_HIDTX;  // FORCE-END THE TRANSMISSION
      if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
      return 0;
    }

    tmr_t start,end;
    __usb_drvstatus&=~USB_STATUS_REMOTERESPND;  // CLEAR THE REMOTE RESPONSE FLAG

    while(size!=__usb_rembigoffset) {

        if((__usb_drvstatus&(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED))!=(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED))
        {
            if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
            return 0;
        }

        if(__usb_drvstatus&USB_STATUS_DATAREADY) {
            // WHY ARE WE RECEIVING DATA WHILE SENDING? THE REMOTE NEEDS TO BE PATIENT - JUST IGNORE ALL DATA RECEIVED UNTIL WE ARE DONE TRANSMITTING
            usb_releasedata();
        }


        if(__usb_drvstatus&USB_STATUS_REMOTERESPND) {
            // HOLD IT, THE REMOTE TRIED TO TELL US SOMETHING
            if(__usb_drvstatus&USB_STATUS_REMOTEBUSY) {
                // GIVE THE REMOTE A CHANCE, WAIT FOR A WHILE
                start=tmr_ticks();
                while(!usb_remoteready(size,sent))
                {
                    if( (__usb_drvstatus&USB_STATUS_REMOTERESPND)&&(__usb_drvstatus&USB_STATUS_REMOTEBUSY)) {
                        end=tmr_ticks();
                        if(tmr_ticks2ms(start,end)>USB_TIMEOUT_MS) {
                            // TIMEOUT - CLEANUP AND RETURN
                            __usb_drvstatus&=~USB_STATUS_HIDTX;  // FORCE-END THE TRANSMISSION
                            if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
                            return 0;
                        }
                    }
                    else {
                            // FAILED FOR REASON OTHER THAN BEING BUSY
                            __usb_drvstatus&=~USB_STATUS_HIDTX;  // FORCE-END THE TRANSMISSION
                            if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
                            return 0;
                    }
                }
                // NOW WE ARE READY AND HAVE A RECENT RESPONSE THAT UPDATED bigoffset
            }

            // REMOTE IS NOT BUSY, DO THEY WANT A DIFFERENT PART?
            if((__usb_rembigoffset>=0)&&(__usb_rembigoffset<sent)) {

                sent=__usb_rembigoffset;   // SEND AGAIN THE BLOCK THE REMOTE WANTS TO GET
                crcroll=0;
                firstblock=1;

                // INFORM THE REMOTE WE ARE ABOUT TO CHANGE THE OFFSET
                start=tmr_ticks();
                while(!usb_remoteready(size,sent))
                {
                    if( (__usb_drvstatus&USB_STATUS_REMOTERESPND)&&(__usb_drvstatus&USB_STATUS_REMOTEBUSY)) {
                        end=tmr_ticks();
                        if(tmr_ticks2ms(start,end)>USB_TIMEOUT_MS) {
                            // TIMEOUT - CLEANUP AND RETURN
                            __usb_drvstatus&=~USB_STATUS_HIDTX;  // FORCE-END THE TRANSMISSION
                            if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
                            return 0;
                        }
                    }
                    else {
                            // FAILED FOR REASON OTHER THAN BEING BUSY
                            __usb_drvstatus&=~USB_STATUS_HIDTX;  // FORCE-END THE TRANSMISSION
                            if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
                            return 0;
                    }
                }



            }
            if(__usb_rembigoffset<0) {
                // THE REMOTE WANTS TO ABORT
                __usb_drvstatus&=~USB_STATUS_HIDTX;  // FORCE-END THE TRANSMISSION
                if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
                return 0;
            }

            __usb_drvstatus&=~USB_STATUS_REMOTERESPND;  // CLEAR THE RESPONSE

        }




    blksize=size-sent;

    if(blksize==0) {
        // WE ALREADY SENT THE WHOLE FILE, WAIT FOR ACKNOWLEDGMENT OR TIME OUT
        cpu_waitforinterrupt();
        end=tmr_ticks();
        if(tmr_ticks2ms(start,end)>USB_TIMEOUT_MS) {
            // THE REMOTE NEVER ACKNOWLEDGED RECEIVING THE DATA
            if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
            return 0;

        }
        continue;
    }


    if(blksize>USB_BLOCKSIZE) blksize=USB_BLOCKSIZE;

    if(bufsize<blksize+9) {
    // ONLY ALLOCATE MEMORY FOR DATA BLOCKS LARGER THAN 1 PACKET
        if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS

    if(blksize>RAWHID_TX_SIZE-8) {
        buf=simpmallocb(blksize+9); // GET A NEW BUFFER WITH ENOUGH SPACE
        bufsize=blksize+9;
     }
    else {
        buf=__usb_txtmpbuffer;
        bufsize=RAWHID_TX_SIZE+9;
    }
    if(!buf) return 0;      // FAILED TO SEND - NOT ENOUGH MEMORY
    }

    if(firstblock) {
        // EITHER THE FIRST BLOCK IN A MULTI OR A SINGLE BLOCK
        if(blksize>=size) buf[1]=USB_BLOCKMARK_SINGLE;
        else { buf[1]=USB_BLOCKMARK_MULTISTART; firstblock=0; }
    } else {
        // EITHER A MIDDLE BLOCK OR FINAL BLOCK
        if(sent+blksize>=size) buf[1]=USB_BLOCKMARK_MULTIEND;
        else buf[1]=USB_BLOCKMARK_MULTI;
    }



    __usb_sndbuffer=buf;
    __usb_bufptr[1]=__usb_sndbuffer;

    __usb_count[1]=blksize+8;
    __usb_padding[1]=(blksize+8)&(RAWHID_TX_SIZE-1);        // PAD WITH ZEROS THE LAST PACKET
    if(__usb_padding[1]) __usb_padding[1]=RAWHID_TX_SIZE-__usb_padding[1];


    buf[0]=0;   // REPORT ID



    buf[2]=blksize&0xff;
    buf[3]=(blksize>>8)&0xff;
    buf[4]=(blksize>>16)&0xff;

    WORD crc=usb_crc32roll(crcroll,data+sent,blksize);
    crcroll=crc;

    buf[5]=crc&0xff;
    buf[6]=(crc>>8)&0xff;
    buf[7]=(crc>>16)&0xff;
    buf[8]=(crc>>24)&0xff;

    memmoveb(buf+9,data+sent,blksize);

    __usb_drvstatus&=~USB_STATUS_REMOTERESPND;  // CLEAR THE REMOTE RESPONSE FLAG

    // PAUSE THE OTHER THREAD TO MAKE SURE WE DON'T INTERFERE
    int oldpaused=__usb_paused;
    __usb_paused=1;
    while(__usb_paused>=0) ;


    int err=hid_write(__usb_curdevice,__usb_bufptr[1],__usb_count[1]+1);


    if(err<=0) {
        // CANNOT WRITE TO USB? - DISCONNECT
        if(buf!=__usb_txtmpbuffer) simpfree(buf);
        hid_close(__usb_curdevice);
        __usb_curdevice=0;
        usb_shutdown();
        return 0;
    }


    __usb_paused=oldpaused;

    // PROCESS THE NEXT BLOCK
    sent+=blksize;
    }

    if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS

    return 1;

}






// HIGH LEVEL FUNCTION TO SEND ONE SINGLE BLOCK TO THE OTHER SIDE
int usb_transmitlong_block(BYTEPTR data,BINT size,BINT isfirst)
{

    // MAKE SURE THERE'S NO OTHER PENDING TRANSMISSIONS AND EVERYTHING IS CONNECTED
    if((__usb_drvstatus&0xff)!= ( USB_STATUS_CONFIGURED
                                 |USB_STATUS_CONNECTED
                                 |USB_STATUS_INIT       ) ) return 0;

    if(size<0) return 0;   // BAD SIZE


    BINT blksize,sent=__usb_rembigoffset,bufsize=0,firstblock=isfirst;
    BYTEPTR buf=0;
    tmr_t start,end;


    if(firstblock)
    {
        __usb_longcrcroll=0;
        __usb_rembigoffset=0;
        sent=0;
        // ONLY AT THE BEGINNING SEND THE HEADER WITH THE TOTAL LENGTH SET TO ZERO TO INDICATE A LONG TRANSMISSION
        if(!usb_remoteready(0,sent))
        {
            __usb_drvstatus&=~USB_STATUS_HIDTX;  // FORCE-END THE TRANSMISSION
            if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
            return 0;
        }


        __usb_drvstatus&=~USB_STATUS_REMOTERESPND;  // CLEAR THE REMOTE RESPONSE FLAG

    }




        if((__usb_drvstatus&(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED))!=(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED))
        {
            if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
            return 0;
        }

        if(__usb_drvstatus&USB_STATUS_DATAREADY) {
            // WHY ARE WE RECEIVING DATA WHILE SENDING? THE REMOTE NEEDS TO BE PATIENT - JUST IGNORE ALL DATA RECEIVED UNTIL WE ARE DONE TRANSMITTING
            usb_releasedata();
        }


        if(__usb_drvstatus&USB_STATUS_REMOTERESPND) {
            // HOLD IT, THE REMOTE TRIED TO TELL US SOMETHING
            if(__usb_drvstatus&USB_STATUS_REMOTEBUSY) {
                // GIVE THE REMOTE A CHANCE, WAIT FOR A WHILE
                start=tmr_ticks();
                while(!usb_remoteready(0,sent))
                {
                    if( (__usb_drvstatus&USB_STATUS_REMOTERESPND)&&(__usb_drvstatus&USB_STATUS_REMOTEBUSY)) {
                        end=tmr_ticks();
                        if(tmr_ticks2ms(start,end)>USB_TIMEOUT_MS) {
                            // TIMEOUT - CLEANUP AND RETURN
                            __usb_drvstatus&=~USB_STATUS_HIDTX;  // FORCE-END THE TRANSMISSION
                            if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
                            return 0;
                        }
                    }
                    else {
                        // FAILED FOR REASON OTHER THAN BEING BUSY
                        __usb_drvstatus&=~USB_STATUS_HIDTX;  // FORCE-END THE TRANSMISSION
                        if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
                        return 0;
                    }
                }
                // NOW WE ARE READY AND HAVE A RECENT RESPONSE THAT UPDATED bigoffset
            }

            // REMOTE IS NOT BUSY, DO THEY WANT A DIFFERENT PART?
            if((__usb_rembigoffset>=0)&&(__usb_rembigoffset<sent)) {

                // THIS IS NOT POSSIBLE ON LONG TRANSMISSIONS SINCE IT'S DONE WORD-BY-WORD, CAN'T GO BACK
                __usb_drvstatus&=~USB_STATUS_HIDTX;  // FORCE-END THE TRANSMISSION
                if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
                return 0;
                }




            if(__usb_rembigoffset<0) {
                // THE REMOTE WANTS TO ABORT
                __usb_drvstatus&=~USB_STATUS_HIDTX;  // FORCE-END THE TRANSMISSION
                if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS
                return 0;
            }

            __usb_drvstatus&=~USB_STATUS_REMOTERESPND;  // CLEAR THE RESPONSE

        }
        // FINALLY, READY TO SEND THE PACKAGE



        blksize=size;

        if(blksize>USB_BLOCKSIZE) blksize=USB_BLOCKSIZE;

        if(bufsize<blksize+9) {
            // ONLY ALLOCATE MEMORY FOR DATA BLOCKS LARGER THAN 1 PACKET
            if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS

            if(blksize>RAWHID_TX_SIZE-8) {
                buf=simpmallocb(blksize+9); // GET A NEW BUFFER WITH ENOUGH SPACE
                bufsize=blksize+9;
            }
            else {
                buf=__usb_txtmpbuffer;
                bufsize=RAWHID_TX_SIZE+9;
            }
            if(!buf) return 0;      // FAILED TO SEND - NOT ENOUGH MEMORY
        }

        if(firstblock) {
            // EITHER THE FIRST BLOCK IN A MULTI OR A SINGLE BLOCK
            buf[1]=USB_BLOCKMARK_MULTISTART;
        } else {
            // EITHER A MIDDLE BLOCK OR FINAL BLOCK
            if(blksize<USB_BLOCKSIZE) buf[1]=USB_BLOCKMARK_MULTIEND;
            else buf[1]=USB_BLOCKMARK_MULTI;
        }



        __usb_sndbuffer=buf;
        __usb_bufptr[1]=__usb_sndbuffer;

        __usb_count[1]=blksize+8;
        __usb_padding[1]=(blksize+8)&(RAWHID_TX_SIZE-1);        // PAD WITH ZEROS THE LAST PACKET
        if(__usb_padding[1]) __usb_padding[1]=RAWHID_TX_SIZE-__usb_padding[1];


        buf[0]=0;   // REPORT ID



        buf[2]=blksize&0xff;
        buf[3]=(blksize>>8)&0xff;
        buf[4]=(blksize>>16)&0xff;

        WORD crc=usb_crc32roll(__usb_longcrcroll,data+8,blksize);
        __usb_longcrcroll=crc;

        buf[5]=crc&0xff;
        buf[6]=(crc>>8)&0xff;
        buf[7]=(crc>>16)&0xff;
        buf[8]=(crc>>24)&0xff;

        memmoveb(buf+9,data+8,blksize);

        __usb_drvstatus&=~USB_STATUS_REMOTERESPND;  // CLEAR THE REMOTE RESPONSE FLAG

        // PAUSE THE OTHER THREAD TO MAKE SURE WE DON'T INTERFERE
        int oldpaused=__usb_paused;
        __usb_paused=1;
        while(__usb_paused>=0) ;


        int err=hid_write(__usb_curdevice,__usb_bufptr[1],__usb_count[1]+1);


        if(err<=0) {
            // CANNOT WRITE TO USB? - DISCONNECT
            if(buf!=__usb_txtmpbuffer) simpfree(buf);
            hid_close(__usb_curdevice);
            __usb_curdevice=0;
            usb_shutdown();
            return 0;
        }


        __usb_paused=oldpaused;

        if((buf!=0) &&(buf!=__usb_txtmpbuffer)) simpfree(buf);   // RELEASE BUFFERS

    return 1;

}



// START A LONG DATA TRANSACTION OVER THE USB PORT
int usb_transmitlong_start()
{
    // USE 1024 BYTE PACKETS

    __usb_longbuffer[0]=simpmallocb(USB_BLOCKSIZE+8);
    if(!__usb_longbuffer[0]) return 0;  // NOT ENOUGH MEMORY TO DO IT!
    __usb_longbuffer[1]=simpmallocb(USB_BLOCKSIZE+8);
    if(!__usb_longbuffer[1]) {
        simpfree(__usb_longbuffer[0]);
        __usb_longbuffer[0]=0;
        return 0;  // NOT ENOUGH MEMORY TO DO IT!
    }

    __usb_longactbuffer=0;
    __usb_longoffset=0;

    return 1;
}

// WRITE A 32-BIT WORD OF DATA
int usb_transmitlong_word(unsigned int data)
{
    unsigned int *ptr=(unsigned int *)(__usb_longbuffer[__usb_longactbuffer]+8+(__usb_longoffset%USB_BLOCKSIZE));

    *ptr=data;
    __usb_longoffset+=4;

    if((__usb_longoffset>0)&&((__usb_longoffset%USB_BLOCKSIZE)==0)) {

    if(!usb_transmitlong_block(__usb_longbuffer[__usb_longactbuffer],USB_BLOCKSIZE,(__usb_longoffset<=USB_BLOCKSIZE)? 1:0)) {
        __usb_longactbuffer=0;
        __usb_longoffset=0;
        if(__usb_longbuffer[1]) simpfree(__usb_longbuffer[1]);
        if(__usb_longbuffer[0]) simpfree(__usb_longbuffer[0]);
        __usb_longbuffer[0]=0;
        __usb_longbuffer[1]=0;
        return 0;
    }
    __usb_longactbuffer^=1;

    }


    return 1;

}

// SEND FINAL BLOCK AND CLEANUP
int usb_transmitlong_finish()
{
    int success=1;
    success=usb_transmitlong_block(__usb_longbuffer[__usb_longactbuffer],__usb_longoffset%USB_BLOCKSIZE,(__usb_longoffset<=USB_BLOCKSIZE)? 1:0);

    // CLEANUP
    __usb_longactbuffer=0;
    __usb_longoffset=0;
    if(__usb_longbuffer[1]) simpfree(__usb_longbuffer[1]);
    if(__usb_longbuffer[0]) simpfree(__usb_longbuffer[0]);
    __usb_longbuffer[0]=0;
    __usb_longbuffer[1]=0;
    return success;

}

// START A LONG DATA TRANSACTION OVER THE USB PORT
int usb_receivelong_start()
{
    __usb_longactbuffer=-1;
    __usb_longoffset=0;
    __usb_longlastsize=-1;
    return 1;
}

// READ A 32-BIT WORD OF DATA
int usb_receivelong_word(unsigned int *data)
{
    if(__usb_longactbuffer==-1) {
        // WE DON'T HAVE ANY BUFFERS YET!
        // WAIT UNTIL WE DO, TIMEOUT IN 2
        // READ THE DATA AND PUT IT ON THE STACK
        BINT datasize,byteoffset=0,totalsize=0;



        // WAIT FOR NEXT BLOCK IN A MULTIPART TRANSACTION
        if(!usb_waitfordata()) {
            usb_ignoreuntilend();
            return -1;
        }

        BYTEPTR data2=usb_accessdata(&datasize);

        if(!totalsize) totalsize=usb_remotetotalsize();
        if(byteoffset!=usb_remoteoffset()) {
            // WE NEED TO REQUEST THE CORRECT OFFSET, BUT IT'S IMPOSSIBLE IN LONG TRANSMISSIONS
            usb_setoffset(-1);  // ABORT, SOMETHING WENT WRONG
            usb_releasedata();
            usb_sendfeedback();
            return -1;
        }


        if(!usb_checkcrc()) {
            // WE NEED TO REQUEST THE CORRECT OFFSET, BUT IT'S IMPOSSIBLE IN LONG TRANSMISSIONS
            usb_setoffset(-1);  // ABORT, SOMETHING WENT WRONG
            usb_releasedata();
            usb_sendfeedback();
            return -1;
        }

        // WE GOT A BLOCK AND THE CRC MATCHES, IT'S ALL WE NEED

        byteoffset+=datasize;
        usb_addremoteoffset(datasize);  // ADVANCE THE COUNTER OF REMOTE DATA RECEIVED FROM THE REMOTE
        usb_setoffset(byteoffset);      // AND OUR LOCAL COUNTER

        if(datasize<USB_BLOCKSIZE) {
        usb_sendfeedback();

        usb_waitdatareceived(); // WAIT FOR THE DATA RECEIVED ACKNOWLEDGEMENT TO BE SENT TO THE REMOTE, INDICATING WE FINISHED RECEIVING DATA

        // THIS IS REQUIRED ONLY AT END OF TRANSMISSION:
        usb_setoffset(0);   // NEXT TRANSMISSION WE NEED TO REQUEST FROM OFFSET ZERO, RATHER THAN RESUME FROM WHERE WE LEFT OFF

        }

        // DONE RECEIVING BLOCKS OF DATA
        

        // CHECK IF THE RECEIVED BLOCK IS OURS
        if((__usb_rcvblkmark==USB_BLOCKMARK_MULTISTART)||(__usb_rcvblkmark==USB_BLOCKMARK_SINGLE)) {
            if(__usb_longoffset) {
                // BAD BLOCK, WE CAN ONLY RECEIVE THE FIRST BLOCK ONCE, ABORT
                usb_releasedata();
                return -1;
            }
            if(__usb_rcvblkmark==USB_BLOCKMARK_SINGLE) __usb_longlastsize=datasize;

        }
        if((__usb_rcvblkmark==USB_BLOCKMARK_MULTI)||(__usb_rcvblkmark==USB_BLOCKMARK_MULTIEND)) {
            if(!__usb_longoffset) {
                // BAD BLOCK, WE CAN ONLY RECEIVE THE FIRST BLOCK ONCE, ABORT
                usb_releasedata();
                return -1;
            }
            if(__usb_rcvblkmark==USB_BLOCKMARK_MULTIEND) __usb_longlastsize=datasize;

        }

        // TAKE OWNERSHIP OF THE BUFFER
        __usb_longactbuffer=0;
        __usb_longbuffer[0]=__usb_rcvbuffer;
        __usb_rcvbuffer=__usb_rxtmpbuffer;  // DON'T LET IT AUTOMATICALLY FREE OUR BUFFER


       usb_releasedata();


    }

    // WE HAVE DATA, RETURN IT

    unsigned int *ptr=(unsigned int *)(__usb_longbuffer[__usb_longactbuffer]+(__usb_longoffset&(USB_BLOCKSIZE-1)));

    if((__usb_longlastsize!=-1)&&((__usb_longoffset%USB_BLOCKSIZE)>=__usb_longlastsize)) {
        // RELEASE THE LAST BUFFER IF WE HAVE ANY
        if(__usb_longbuffer[__usb_longactbuffer])  simpfree(__usb_longbuffer[__usb_longactbuffer]);
        __usb_longbuffer[__usb_longactbuffer]=0;
        __usb_longactbuffer=-1;
        return -1; // END OF FILE!
    }

    if(data) *data=*ptr;
    __usb_longoffset+=4;

    if((__usb_longoffset%USB_BLOCKSIZE)==0) {
        // END OF THIS BLOCK, RELEASE IT WHILE WE WAIT FOR THE NEXT ONE
        if(__usb_longbuffer[__usb_longactbuffer])  simpfree(__usb_longbuffer[__usb_longactbuffer]);
        __usb_longbuffer[__usb_longactbuffer]=0;
        __usb_longactbuffer=-1;
    }

    return 1;


}

// SEND FINAL BLOCK AND CLEANUP
int usb_receivelong_finish()
{
    // CLEANUP
    // RELEASE A BUFFER IF WE HAVE ANY
    if((__usb_longactbuffer!=-1)&&(__usb_longbuffer[__usb_longactbuffer]!=0))  simpfree(__usb_longbuffer[__usb_longactbuffer]);

    __usb_longactbuffer=-1;
    __usb_longoffset=0;
    __usb_longbuffer[0]=0;
    __usb_longbuffer[1]=0;
    return 1;

}


void usb_ignoreuntilend()
{
  __usb_drvstatus|=USB_STATUS_IGNORE;   // SIGNAL TO IGNORE PACKETS UNTIL END OF TRANSMISSION DETECTED
}

// GET THE TOTAL SIZE THE REMOTE IS TRYING TO SEND
int usb_remotetotalsize()
{
    return __usb_rembigtotal;
}
// GET THE GLOBAL OFFSET THE REMOTE IS TRYING TO SEND
int usb_remoteoffset()
{
    return __usb_rembigoffset;
}

void usb_addremoteoffset(int bytes)
{
    __usb_rembigoffset+=bytes;
}

void usb_setoffset(int offset)
{
    __usb_localbigoffset=offset;
}

void usb_waitdatareceived()
{
    tmr_t start=tmr_ticks(),end;
    while(__usb_drvstatus&USB_STATUS_DATARECEIVED) {
         if((__usb_drvstatus&(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED))!=(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED)) return;
        cpu_waitforinterrupt();
        end=tmr_ticks();
        if(tmr_ticks2ms(start,end)>USB_TIMEOUT_MS) {
            // TOO LONG TO ACKNOWLEDGE LAST PACKET WAS RECEIVED
            return;
        }
    }

}
