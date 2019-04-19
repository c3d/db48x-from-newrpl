/*
* Copyright (c) 2019, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include <libraries.h>
#include <ui.h>


// ADD-ON MEMORY ALLOCATOR - SHARED WITH FILE SYSTEM AND NEWRPL ENGINE
extern void init_simpalloc();
extern unsigned int *simpmalloc(int words);
extern unsigned char *simpmallocb(int bytes);
extern void simpfree(void *voidptr);




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

// GLOBAL VARIABLES OF THE USB SUBSYSTEM
extern volatile BINT __usb_drvstatus; // FLAGS TO INDICATE IF INITIALIZED, CONNECTED, SENDING/RECEIVING, ETC.
extern BYTE *__usb_bufptr[3];   // POINTERS TO BUFFERS FOR EACH ENDPOINT (0/1/2)
extern BINT __usb_count[3];   // REMAINING BYTES TO TRANSFER ON EACH ENDPOINT (0/1/2)
extern BINT __usb_padding[3] ;    // PADDING FOR OUTGOING TRANSFERS
extern BYTE __usb_rxtmpbuffer[RAWHID_RX_SIZE+1] ;  // TEMPORARY BUFFER FOR NON-BLOCKING CONTROL TRANSFERS
extern BYTE __usb_txtmpbuffer[RAWHID_TX_SIZE+1] ;  // TEMPORARY BUFFER FOR NON-BLOCKING CONTROL TRANSFERS
extern BYTEPTR __usb_rcvbuffer ;
extern BINT __usb_rcvtotal ;
extern BINT __usb_rcvpartial ;
extern WORD __usb_rcvcrc ;
extern WORD __usb_rcvcrcroll;
extern BINT __usb_rembigtotal;
extern BINT __usb_rembigoffset;
extern BINT __usb_localbigoffset;


extern BINT __usb_rcvblkmark ;    // TYPE OF RECEIVED BLOCK (ONE OF USB_BLOCKMARK_XXX CONSTANTS)
extern BYTEPTR __usb_sndbuffer ;
extern BINT __usb_sndtotal ;
extern BINT __usb_sndpartial ;

// GLOBAL VARIABLES FOR DOUBLE BUFFERED LONG TRANSACTIONS
extern BYTEPTR __usb_longbuffer[2];              // DOUBLE BUFFERING FOR LONG TRANSMISSIONS OF DATA
extern BINT __usb_longbufused[2];              // DOUBLE BUFFERING FOR LONG TRANSMISSIONS OF DATA
extern BINT __usb_longoffset,__usb_longrdoffset;
extern BINT __usb_longactbuffer,__usb_longrdbuffer;  // WHICH BUFFER IS BEING WRITTEN AND WHICH ONE IS BEING READ
extern BINT __usb_longlastsize;                  // LAST BLOCK SIZE IN A LONG TRANSMISSION
extern WORD __usb_longcrcroll;





extern const BYTE device_descriptor[];

extern const BYTE rawhid_hid_report_desc[];

extern const WORD const __crctable[256];


#define CONFIG1_DESC_SIZE        (9+9+9+7+7)
#define RAWHID_HID_DESC_OFFSET   (9+9)
extern const BYTE config1_descriptor[CONFIG1_DESC_SIZE];

struct usb_string_descriptor_struct {
    BYTE bLength;
    BYTE bDescriptorType;
    BYTE wString[];
};
extern const struct usb_string_descriptor_struct _usb_string0;
extern const struct usb_string_descriptor_struct _usb_string1;
extern const struct usb_string_descriptor_struct _usb_string2;

// This table defines which descriptor data is sent for each specific
// request from the host (in wValue and wIndex).
struct descriptor_list_struct {
    HALFWORD	wValue;
    HALFWORD	wIndex;
    const BYTE	*addr;
    BYTE		length;
};
extern struct descriptor_list_struct descriptor_list[];

#define NUM_DESC_LIST 7



//*******************************************************************************************************
// USB ROUTINES PREPARED TO RUN FROM RAM DURING FIRMWARE UPDATE - DO NOT REORGANIZE OR CHANGE THIS BLOCK
//*******************************************************************************************************

#define LONG_BUFFER_SIZE        32*USB_BLOCKSIZE

// FAKE MEMORY ALLOCATORS
BYTEPTR ram_memblocks[2];
int ram_memblocksused[2];

// DO NOT MOVE THIS FUNCTION, NEEDS TO BE THE FIRST IN THE MODULE
void *ram_simpmallocb(int nbytes)
{
    if(!ram_memblocksused[0]) { ram_memblocksused[0]=1; return ram_memblocks[0]; }
    if(!ram_memblocksused[1]) { ram_memblocksused[1]=1; return ram_memblocks[1]; }
    return 0;   // NOT ENOUGH MEMORY!
}

void ram_simpfree(void *buffer)
{
    if(buffer==ram_memblocks[0]) ram_memblocksused[0]=0;
    else if(buffer==ram_memblocks[1]) ram_memblocksused[1]=0;

}


// START A LONG DATA TRANSACTION OVER THE USB PORT
int ram_usb_receivelong_start()
{
    __usb_longactbuffer=0;
    __usb_longrdbuffer=0;
    __usb_longoffset=0;
    __usb_longrdoffset=0;

    // PRE-ALLOCATE THE MEMORY USING THE ROM ROUTINES BEFORE WE START RUNNING FROM RAM
    //__usb_longbuffer[0]=ram_simpmallocb(LONG_BUFFER_SIZE);      // PREALLOCATE 2 LARGE BUFFERS
    //__usb_longbuffer[1]=ram_simpmallocb(LONG_BUFFER_SIZE);
    if(!__usb_longbuffer[0]) return 0;
    if(!__usb_longbuffer[1]) return 0;
    __usb_longbufused[0]=0;
    __usb_longbufused[1]=0;
    __usb_longlastsize=-1;
    __usb_localbigoffset=0;
    return 1;
}

// HIGH LEVEL FUNCTION TO ACCESS A BLOCK OF DATA
BYTEPTR ram_usb_accessdata(int *blksize)
{
    if(!(__usb_drvstatus&USB_STATUS_DATAREADY)) return 0;
    if(blksize) *blksize=(__usb_rcvpartial>__usb_rcvtotal)? __usb_rcvtotal:__usb_rcvpartial;
    return __usb_rcvbuffer;
}

// GET THE TOTAL SIZE THE REMOTE IS TRYING TO SEND
int ram_usb_remotetotalsize()
{
    return __usb_rembigtotal;
}
// GET THE GLOBAL OFFSET THE REMOTE IS TRYING TO SEND
int ram_usb_remoteoffset()
{
    return __usb_rembigoffset;
}

void ram_usb_addremoteoffset(int bytes)
{
    __usb_rembigoffset+=bytes;
}

void ram_usb_setoffset(int offset)
{
    __usb_localbigoffset=offset;
}

inline void ram_cpu_waitforinterrupt()
{
    asm volatile ("stmfd sp!,{ r0 }");  // SAVE IRQ STACK PTR
    asm volatile ("mov r0,#0");
    asm volatile ("mcr p15,0,r0,c7,c0,4");
    asm volatile ("ldmia sp!,{ r0 }");  // SAVE IRQ STACK PTR

}



void ram_usb_waitdatareceived()
{
    while(__usb_drvstatus&(USB_STATUS_DATARECEIVED|USB_STATUS_HIDTX)) {
        if((__usb_drvstatus&(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED))!=(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED)) return;

        ram_cpu_waitforinterrupt();
    }
}


// HIGH LEVEL FUNCTION TO RELEASE A BLOCK OF DATA AND GET READY TO RECEIVE THE NEXT
void ram_usb_releasedata()
{
    if(!(__usb_drvstatus&USB_STATUS_DATAREADY)) return;
    if(__usb_rcvbuffer!=__usb_rxtmpbuffer) ram_simpfree(__usb_rcvbuffer);

    __usb_drvstatus&=~USB_STATUS_DATAREADY;

}

// SEND SOME FEEDBACK TO THE HOST ABOUT THE CURRENT TRANSMISSION
void ram_usb_sendfeedback()
{
   __usb_drvstatus|=USB_STATUS_DATARECEIVED;
}


// CALCULATE THE STANDARD CRC32 OF A BLOCK OF DATA
// USE A MATH REGISTER DATA AS TABLE. NEEDS TO BE INITIALIZED
#define RAM_CRCTABLE RReg[9].data

WORD ram_usb_crc32roll(WORD oldcrc,BYTEPTR data,BINT len)
{
    WORD crc=oldcrc^(-1);
    while(len--) crc=RAM_CRCTABLE[(crc ^ *data++) & 0xFF] ^ (crc >> 8);
    return crc^(-1);
}



// CHECK THE CRC OF THE LAST BLOCK RECEIVED
int ram_usb_checkcrc()
{
    if(!(__usb_drvstatus&USB_STATUS_DATAREADY)) return 0;
    WORD rcvdcrc=ram_usb_crc32roll(__usb_rcvcrcroll,__usb_rcvbuffer,(__usb_rcvpartial>__usb_rcvtotal)? __usb_rcvtotal:__usb_rcvpartial);
    __usb_rcvcrcroll=rcvdcrc;
    if(rcvdcrc!=__usb_rcvcrc) return 0;
    return 1;
}




int ram_mod_usblocksize(int value)
{
return value%USB_BLOCKSIZE;
}


// READ A 32-BIT WORD OF DATA
// RETURN 1=OK, 0=TIMEOUT, -1=TRANSMISSION ERROR

int ram_usb_receivelong_word(unsigned int *data)
{

        // WE DON'T HAVE ANY BUFFERS YET!

        {unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
        scrptr[6]=0xffffffff;}

        BINT datasize,byteoffset=__usb_localbigoffset,totalsize=0;

        if((__usb_longactbuffer==__usb_longrdbuffer)&&(__usb_longrdoffset>=__usb_longbufused[__usb_longrdbuffer])) {


        while(!(__usb_drvstatus&USB_STATUS_DATAREADY)) {
            // SINCE WE ARE UPDATING FIRMWARE, DON'T DO TIMEOUT, ONLY CABLE DISCONNECT
            if((__usb_drvstatus&(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED))!=(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED)) return 0;
            ram_cpu_waitforinterrupt();
        }

        }

        // THERE'S DATA AWAITING, MOVE IT TO ONE OF THE LARGE BUFFERS
        if(__usb_drvstatus&USB_STATUS_DATAREADY) {

        BYTEPTR data2=ram_usb_accessdata(&datasize);

        if(!totalsize) totalsize=ram_usb_remotetotalsize();

        if(!ram_usb_checkcrc()) {
            // WE NEED TO REQUEST THE CORRECT OFFSET, BUT IT'S IMPOSSIBLE IN LONG TRANSMISSIONS
            ram_usb_setoffset(-1);  // ABORT, SOMETHING WENT WRONG
            ram_usb_releasedata();
            ram_usb_sendfeedback();
            return -1;
        }

        // WE GOT A BLOCK AND THE CRC MATCHES, IT'S ALL WE NEED

        byteoffset+=datasize;
        ram_usb_setoffset(byteoffset);      // AND OUR LOCAL COUNTER

        if(datasize<USB_BLOCKSIZE) {
            // IT'S THE LAST BLOCK
        ram_usb_sendfeedback();

        ram_usb_waitdatareceived(); // WAIT FOR THE DATA RECEIVED ACKNOWLEDGEMENT TO BE SENT TO THE REMOTE, INDICATING WE FINISHED RECEIVING DATA

        // THIS IS REQUIRED ONLY AT END OF TRANSMISSION:
        ram_usb_setoffset(0);   // NEXT TRANSMISSION WE NEED TO REQUEST FROM OFFSET ZERO, RATHER THAN RESUME FROM WHERE WE LEFT OFF

        }
       
        // DONE RECEIVING BLOCKS OF DATA


        {unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
        scrptr[6]=0xF00F00F0;}

        // CHECK IF THE RECEIVED BLOCK IS OURS
        if((__usb_rcvblkmark==USB_BLOCKMARK_MULTISTART)||(__usb_rcvblkmark==USB_BLOCKMARK_SINGLE)) {
            if(__usb_longoffset) {
                // BAD BLOCK, WE CAN ONLY RECEIVE THE FIRST BLOCK ONCE, ABORT
                ram_usb_releasedata();
                return -1;
            }
            if(__usb_rcvblkmark==USB_BLOCKMARK_SINGLE) __usb_longlastsize=datasize;

        }
        if((__usb_rcvblkmark==USB_BLOCKMARK_MULTI)||(__usb_rcvblkmark==USB_BLOCKMARK_MULTIEND)) {
            if(!__usb_longoffset) {
                // BAD BLOCK, WE CAN ONLY RECEIVE THE FIRST BLOCK ONCE, ABORT
                ram_usb_releasedata();
                return -1;
            }
            if(__usb_rcvblkmark==USB_BLOCKMARK_MULTIEND) __usb_longlastsize=datasize;

        }

        // COPY ALL DATA TO CURRENTLY ACTIVE LARGE BUFFER

        if(__usb_longbufused[__usb_longactbuffer]+=datasize>LONG_BUFFER_SIZE) {
            // THE BUFFER IS FULL, START THE OTHER BUFFER

            __usb_longactbuffer^=1; // SWITCH WRITING TO THE OTHER BUFFER

            if(__usb_longactbuffer==__usb_longrdbuffer) {
                // BUFFER OVERRUN!
                ram_usb_setoffset(-1);  // ABORT, SOMETHING WENT WRONG
                ram_usb_releasedata();
                ram_usb_sendfeedback();
                return -1;
            }
            __usb_longbufused[__usb_longactbuffer]=0;

            // LET THE DRIVER KNOW ALL RESPONSES NEED TO TELL THE HOST WE ARE HALTED
            __usb_drvstatus|=USB_STATUS_DATAHALTED;
            // SINCE ONE BUFFER IS COMPLETE, SEND HALT FEEDBACK TO THE SENDER SO IT STOPS SENDING DATA UNTIL FURTHER NOTICE
            ram_usb_sendfeedback();


        }


        //ram_memmoveb(__usb_longbuffer[__usb_longactbuffer]+__usb_longbufused[__usb_longactbuffer],__usb_rcvbuffer,datasize);
        {
            int f;
            for(f=0;f<datasize;++f)
            {
                *(__usb_longbuffer[__usb_longactbuffer]+__usb_longbufused[__usb_longactbuffer]+f)=__usb_rcvbuffer[f];
            }
        }


        __usb_longbufused[__usb_longactbuffer]+=datasize;

        ram_usb_releasedata();

        // DONE RECEIVING ANOTHER PACKET


    }

    {unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
    scrptr[6]=0xF000000F;}
    // WE HAVE DATA, RETURN IT

    unsigned int *ptr=(unsigned int *)(__usb_longbuffer[__usb_longrdbuffer]+__usb_longrdoffset);

    if((__usb_longlastsize!=-1)&&(ram_mod_usblocksize(__usb_longoffset)>=__usb_longlastsize)) {
        // RELEASE THE LAST BUFFER IF WE HAVE ANY
        return -1; // END OF FILE!
    }

    if(data) *data=*ptr;

     __usb_longoffset+=4;
     __usb_longrdoffset+=4;

     if(__usb_longrdoffset>=__usb_longbufused[__usb_longrdbuffer]) {
         // WE EITHER FINISHED THE BUFFER OR ARE ALL CAUGHT UP WITH DATA
         if(__usb_longactbuffer==__usb_longrdbuffer) {
             // WE ARE ALL CAUGHT UP, RESET THE USED COUNTERS SO IT USES THE ENTIRE BUFFER AGAIN
             __usb_longbufused[__usb_longrdbuffer]=0;
             __usb_longrdoffset=0;
         }
         else {
          // SWITCH TO THE OTHER BUFFER AND RELEASE THIS ONE
             __usb_longrdbuffer=__usb_longactbuffer;
             __usb_longrdoffset=0;

             // LET THE HOST KNOW WE ARE NO LONGER HALTED
             __usb_drvstatus&=~USB_STATUS_DATAHALTED;
             ram_usb_sendfeedback();
         }
     }

    return 1;

}

// SEND FINAL BLOCK AND CLEANUP
int ram_usb_receivelong_finish()
{

    // CLEANUP
    // RELEASE A BUFFER IF WE HAVE ANY

    // NO NEED TO RELEASE PRE-ALLOCATED BUFFERS
    //if(__usb_longactbuffer!=-1) {
    //ram_simpfree(__usb_longbuffer[0]);
    //ram_simpfree(__usb_longbuffer[1]);
    //}

    __usb_longactbuffer=-1;
    __usb_longrdbuffer=-1;
    __usb_longoffset=0;
    __usb_longbuffer[0]=0;
    __usb_longbuffer[1]=0;

    __usb_drvstatus|=USB_STATUS_IGNORE;   // SIGNAL TO IGNORE PACKETS UNTIL END OF TRANSMISSION DETECTED
    if(__usb_drvstatus&USB_STATUS_DATAREADY) ram_usb_releasedata();

    while(__usb_drvstatus&USB_STATUS_IGNORE) {
         if((__usb_drvstatus&(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED))!=(USB_STATUS_CONFIGURED|USB_STATUS_INIT|USB_STATUS_CONNECTED)) break;
        ram_cpu_waitforinterrupt();
    }

    return 1;


}


void ram_usb_hwsetup()
{
    // NO NEED TO CHANGE CLOCK, IT WAS FIXED BY THE ROUTINES IN ROM ALREADY
    // MAKE SURE WE HAVE PCLK>20 MHz FOR USB COMMUNICATIONS TO WORK
    //if(__cpu_getPCLK()<20000000) cpu_setspeed(HAL_USBCLOCK);

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

void ram_usb_hwsuspend()
{

    *MISCCR|=(USBSUSPND0|USBSUSPND1);   // CHANGE TO SUSPEND MODE

    *CLKSLOW|=0x80;    // TURN OFF UPLL


}

void ram_usb_hwresume()
{
    *UPLLCON=0x78023;   // 48 MHZ CLOCK
    *CLKSLOW&=~0x80;    // MAKE SURE UPLL IS ON

    *MISCCR&=~(USBSUSPND0|USBSUSPND1);   // CHANGE TO NORMAL MODE

}











void inline ram_usb_checkpipe()
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

// TRANSMIT BYTES TO THE HOST IN EP0 ENDPOINT
// STARTS A NEW TRANSMISSION IF newtransmission IS TRUE, EVEN IF
// THERE ARE ZERO BYTES IN THE BUFFER
// FOR USE WITHIN ISR

void ram_usb_ep0_transmit(int newtransmission)
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
    while(__usb_count[0] && (cnt<EP0_FIFO_SIZE)) {
        *EP0_FIFO=(WORD) *__usb_bufptr[0];
        ++__usb_bufptr[0];
        ++cnt;
        --__usb_count[0];
    }

    if(__usb_count[0]==0) {
        // SEND ANY PADDING
        while( (__usb_padding[0]!=0) && (cnt<EP0_FIFO_SIZE)) {
        *EP0_FIFO=0;
        ++cnt;
        --__usb_padding[0];
        }
    }

    if((__usb_count[0]==0)&&(__usb_padding[0]==0))  {
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

void ram_usb_ep0_receive(int newtransmission)
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
    while(__usb_count[0] && (cnt<EP0_FIFO_SIZE)) {
        *__usb_bufptr[0]=(BYTE)*EP0_FIFO;
        ++__usb_bufptr[0];
        --__usb_count[0];
        ++cnt;
    }

    if(__usb_count[0]==0)  {
        *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;  // RECEIVED THE LAST PACKET
        __usb_drvstatus&=~USB_STATUS_EP0RX;
    }
    else {
        *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;               // RECIEVED PART OF THE BUFFER
        __usb_drvstatus|=USB_STATUS_EP0RX;      // AND KEEP RECEIVING MORE
    }
    }


}



void ram_ep0_irqservice()
{
    *INDEX_REG=0;   // SELECT ENDPOINT 0


    ram_usb_checkpipe();

    if(__usb_drvstatus&USB_STATUS_EP0TX) {

        ram_usb_ep0_transmit(0);
        ram_usb_checkpipe();

        return;
    }

    if( (*EP0_CSR) & EP0_OUT_PKT_RDY) {

        // PROCESS FIRST ANY ONGOING TRANSMISSIONS
        if(__usb_drvstatus&USB_STATUS_EP0RX) {
            ram_usb_ep0_receive(0);
            ram_usb_checkpipe();
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
                    __usb_bufptr[0]=(BYTEPTR) descriptor_list[k].addr;
                    if(length<descriptor_list[k].length) { __usb_count[0]=length; __usb_padding[0]=0; }
                    else { __usb_count[0]=descriptor_list[k].length; __usb_padding[0]=length-descriptor_list[k].length; }
                    *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
                    ram_usb_ep0_transmit(1);    // SEND 0-DATA STATUS STAGE
                    ram_usb_checkpipe();
                    return;
                }
            }

            // SPECIAL CASE - CALCULATOR SERIAL NUMBER STRING
            if((0x0303==value)&&(0x0409==index)) {
                // FOUND THE REQUESTED DESCRIPTOR
                __usb_bufptr[0]=__usb_rxtmpbuffer;
                __usb_rxtmpbuffer[0]=20+2;
                __usb_rxtmpbuffer[1]=3;

                // COPY THE SERIAL NUMBER - EXPAND ASCII TO UTF-16
                int n;
                BYTEPTR ptr=(BYTEPTR)SERIAL_NUMBER_ADDRESS;
                for(n=0;n<10;++n,++ptr) {
                    __usb_rxtmpbuffer[2+2*n]=*ptr;
                    __usb_rxtmpbuffer[3+2*n]=0;
                }


                if(length<__usb_rxtmpbuffer[0]) { __usb_count[0]=length; __usb_padding[0]=0; }
                else { __usb_count[0]=__usb_rxtmpbuffer[0]; __usb_padding[0]=length-__usb_rxtmpbuffer[0]; }
                *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
                ram_usb_ep0_transmit(1);    // SEND 0-DATA STATUS STAGE
                ram_usb_checkpipe();
                return;
            }



            // DON'T KNOW THE ANSWER TO THIS
            __usb_count[0]=0;
            __usb_padding[0]=length;    // SEND THE DATA AS REQUESTED, STALL AT THE END
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
            ram_usb_ep0_transmit(1);    // SEND DATA STATUS STAGE
            ram_usb_checkpipe();

            return;
        }
        case SET_ADDRESS:
        {
            __usb_count[0]=0;
            __usb_padding[0]=0;
            __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
            *FUNC_ADDR_REG=value|0x80;
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
            ram_usb_checkpipe();
            return;
        }
        case SET_CONFIGURATION:
        {
            // OUR DEVICE HAS ONE SINGLE CONFIGURATION AND IS SETUP
            // ON WAKEUP, SO NOTHING TO DO HERE BUT ACKNOWLEDGE

            if(value) __usb_drvstatus|=USB_STATUS_CONFIGURED;
            else __usb_drvstatus&=~USB_STATUS_CONFIGURED;
            __usb_count[0]=0;
            __usb_padding[0]=0;
            __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
            ram_usb_checkpipe();

            return;
        }
        case GET_CONFIGURATION:
        {
          BINT configvalue=(__usb_drvstatus&USB_STATUS_CONFIGURED)? 1:0;
          *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
          __usb_count[0]=1;
          __usb_padding[0]=0;
          __usb_bufptr[0]=__usb_rxtmpbuffer;
          __usb_rxtmpbuffer[0]=configvalue;
          ram_usb_ep0_transmit(1);    // SEND DATA STATUS STAGE
          ram_usb_checkpipe();

          return;
        }
        case GET_STATUS:
        {

            __usb_rxtmpbuffer[0]=__usb_rxtmpbuffer[1]=0;
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
            __usb_padding[0]=0;
            __usb_bufptr[0]=__usb_rxtmpbuffer;

            ram_usb_ep0_transmit(1);    // SEND DATA STATUS STAGE
            ram_usb_checkpipe();

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

            __usb_count[0]=0;
            __usb_padding[0]=0;
            __usb_drvstatus&=~USB_STATUS_EP0RX|USB_STATUS_EP0TX;
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
            ram_usb_checkpipe();

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

            __usb_count[0]=0;
            __usb_padding[0]=0;
            __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
            ram_usb_checkpipe();

            return;
        }


        }
        // UNKNOWN STANDARD REQUEST??
        // DON'T KNOW THE ANSWER TO THIS BUT KEEP THE PIPES RUNNING
        if((reqtype&USB_DIRECTION)==USB_DEV_TO_HOST) {
            // SEND THE RIGHT AMOUNT OF ZEROS TO THE HOST
            __usb_count[0]=0;
            __usb_padding[0]=length;
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
            ram_usb_ep0_transmit(1);
            ram_usb_checkpipe();

            return;
        }

        // THIS IS AN INCOMING REQUEST WITH NO DATA STAGE

        // READ ANY EXTRA DATA TO KEEP THE FIFO CLEAN
        while(length>0) { __usb_rxtmpbuffer[0]=*EP0_FIFO; --length; }

        __usb_count[0]=0;
        __usb_padding[0]=0;
        __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
        *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
        ram_usb_checkpipe();

        return;


        }

        if((reqtype&0x61)==0x21) {  // CLASS INTERFACE REQUESTS

        if(index==RAWHID_INTERFACE) {
            switch(request)
            {
            case HID_SET_REPORT:
                // GET DATA FROM HOST
                *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
                __usb_count[0]=RAWHID_TX_SIZE;
                __usb_padding[0]=0;
                __usb_bufptr[0]=__usb_rxtmpbuffer;    // FOR NOW, LET'S SEE WHAT TO DO WITH THIS LATER
                ram_usb_ep0_receive(1);
                return;
            case HID_GET_REPORT:
                // SEND DATA TO HOST - SEND ALL ZEROS
                *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
                __usb_count[0]=1;
                __usb_padding[0]=RAWHID_TX_SIZE-1;
                __usb_rxtmpbuffer[0]=(__usb_drvstatus&USB_STATUS_DATAREADY)? 1:0;
                __usb_bufptr[0]=__usb_rxtmpbuffer;    // SEND THE STATUS
                ram_usb_ep0_transmit(1);
                ram_usb_checkpipe();

                return;

           case HID_SET_IDLE:
                // SEND DATA TO HOST - SEND ALL ZEROS
                __usb_count[0]=0;
                __usb_padding[0]=0;
                __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
                *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
                ram_usb_checkpipe();
                return;

            }


        }
        // UNKNOWN CLASS REQUEST??
        // DON'T KNOW THE ANSWER TO THIS
        if((reqtype&USB_DIRECTION)==USB_DEV_TO_HOST) {
            // SEND THE RIGHT AMOUNT OF ZEROS TO THE HOST
            __usb_count[0]=0;
            __usb_padding[0]=length;
            *EP0_CSR|=EP0_SEND_STALL|EP0_SERVICED_OUT_PKT_RDY;
            ram_usb_ep0_transmit(1);
            ram_usb_checkpipe();

            return;
        }
                // READ ANY EXTRA DATA TO KEEP THE FIFO CLEAN
        if(length>EP0_FIFO_SIZE) length=EP0_FIFO_SIZE;
        while(length>0) { __usb_rxtmpbuffer[0]=*EP0_FIFO; --length; }
        __usb_count[0]=0;
        __usb_padding[0]=0;
        __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
        *EP0_CSR|=EP0_SEND_STALL|EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
        ram_usb_checkpipe();
        return;


        }

        // ADD OTHER REQUESTS HERE

        if((reqtype&USB_DIRECTION)==USB_DEV_TO_HOST) {
            // SEND THE RIGHT AMOUNT OF ZEROS TO THE HOST
            __usb_count[0]=0;
            __usb_padding[0]=length;
            *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY;
            ram_usb_ep0_transmit(1);
            ram_usb_checkpipe();

            return;
        }
                // READ ANY EXTRA DATA TO KEEP THE FIFO CLEAN
                if(length>EP0_FIFO_SIZE) length=EP0_FIFO_SIZE;
                while(length>0) { __usb_rxtmpbuffer[0]=*EP0_FIFO; --length; }
                __usb_count[0]=0;
                __usb_padding[0]=0;
                __usb_drvstatus&=~(USB_STATUS_EP0RX|USB_STATUS_EP0TX);
                *EP0_CSR|=EP0_SERVICED_OUT_PKT_RDY|EP0_DATA_END;
                ram_usb_checkpipe();

        return;

    }

}


void ram_usb_ep1_transmit(int newtransmission)
{

    if(!(__usb_drvstatus&USB_STATUS_CONNECTED)) return;

    if(newtransmission || (__usb_drvstatus&USB_STATUS_HIDTX)) {

    *INDEX_REG=RAWHID_TX_ENDPOINT;

    if( (*IN_CSR1_REG)&EPn_IN_PKT_RDY) {
        // PREVIOUS PACKET IS STILL BEING SENT, DON'T PUSH IT
        __usb_drvstatus|=USB_STATUS_HIDTX;      // AND KEEP TRANSMITTING
        return;
    }

    int cnt=0;
    while(__usb_count[1] && (cnt<EP1_FIFO_SIZE)) {
        *EP1_FIFO=(WORD) *__usb_bufptr[1];
        ++__usb_bufptr[1];
        ++cnt;
        --__usb_count[1];
    }

    if(__usb_count[1]==0) {
        // SEND ANY PADDING
        while( (__usb_padding[1]!=0) && (cnt<EP1_FIFO_SIZE)) {
        *EP1_FIFO=0;
        ++cnt;
        --__usb_padding[1];
        }
    }


        *IN_CSR1_REG|=EPn_IN_PKT_RDY;  // SEND THE LAST PACKET

        if((__usb_count[1]==0)&&(__usb_padding[1]==0))  {
        // RELEASE ANY ALLOCATED MEMORY
        if(__usb_sndbuffer!=__usb_txtmpbuffer) ram_simpfree(__usb_sndbuffer);
        }

       if((cnt==0)||(cnt!=EP1_FIFO_SIZE)) {
           __usb_drvstatus&=~USB_STATUS_HIDTX;
       } else __usb_drvstatus|=USB_STATUS_HIDTX;      // AND KEEP TRANSMITTING



    }



}


// RECEIVE BYTES FROM THE HOST IN EP2 ENDPOINT
// STARTS A NEW TRANSMISSION IF newtransmission IS TRUE, EVEN IF
// THERE ARE ZERO BYTES IN THE BUFFER
// FOR USE WITHIN ISR

void ram_usb_ep2_receive(int newtransmission)
{

    if(!(__usb_drvstatus&USB_STATUS_CONNECTED)) return;

    if(newtransmission || (__usb_drvstatus&USB_STATUS_HIDRX)) {

    *INDEX_REG=RAWHID_RX_ENDPOINT;

    if(!((*OUT_CSR1_REG)&EPn_OUT_PKT_RDY)) {
        // THEREŚ NO PACKETS AVAILABLE
        __usb_drvstatus|=USB_STATUS_HIDRX;      // AND KEEP RECEIVING
        return;
    }


    int fifocnt=(*OUT_FIFO_CNT1_REG)+256*(*OUT_FIFO_CNT2_REG);
    int cnt=0;

    if(newtransmission) {
        if(!(__usb_drvstatus&USB_STATUS_DATAREADY)) {

        __usb_rcvtotal=0;
        __usb_rcvpartial=0;
        __usb_rcvcrc=0;

        }
        if(!fifocnt)  {
            __usb_drvstatus&=~USB_STATUS_HIDRX;
            return;
        }



        // WE ARE READY TO RECEIVE A NEW DATA BLOCK




        if(fifocnt>8) {
        // READ THE HEADER
        WORD startmarker=*EP2_FIFO;

        if(startmarker==USB_BLOCKMARK_GETSTATUS) {
            // REQUESTED INFO, RESPOND EVEN IF PREVIOUS DATA WASN'T RETRIEVED

            WORD txsize=*EP2_FIFO;
            txsize|=(*EP2_FIFO)<<8;
            txsize|=(*EP2_FIFO)<<16;
            txsize|=(*EP2_FIFO)<<24;
            fifocnt-=4;

            WORD txoffset=*EP2_FIFO;
            txoffset|=(*EP2_FIFO)<<8;
            txoffset|=(*EP2_FIFO)<<16;
            txoffset|=(*EP2_FIFO)<<24;
            fifocnt-=4;

            __usb_rembigtotal=txsize;
            __usb_rembigoffset=txoffset;

            __usb_rcvcrcroll=0;

            __usb_count[1]=10;
            __usb_padding[1]=RAWHID_TX_SIZE-10;
            __usb_txtmpbuffer[0]=USB_BLOCKMARK_RESPONSE;
            __usb_txtmpbuffer[1]=(__usb_drvstatus&(USB_STATUS_DATAHALTED|USB_STATUS_DATAREADY))? 1:0;
            __usb_txtmpbuffer[2]=__usb_rembigtotal&0xff;
            __usb_txtmpbuffer[3]=(__usb_rembigtotal>>8)&0xff;
            __usb_txtmpbuffer[4]=(__usb_rembigtotal>>16)&0xff;
            __usb_txtmpbuffer[5]=(__usb_rembigtotal>>24)&0xff;
            __usb_txtmpbuffer[6]=__usb_localbigoffset&0xff;
            __usb_txtmpbuffer[7]=(__usb_localbigoffset>>8)&0xff;
            __usb_txtmpbuffer[8]=(__usb_localbigoffset>>16)&0xff;
            __usb_txtmpbuffer[9]=(__usb_localbigoffset>>24)&0xff;

            __usb_bufptr[1]=__usb_txtmpbuffer;    // SEND THE STATUS
            ram_usb_ep1_transmit(1);
            --fifocnt;

            *INDEX_REG=RAWHID_RX_ENDPOINT;

            // FLUSH THE FIFO
            BYTE sum=0;
            while(cnt<fifocnt) {
                sum+=(BYTE)*EP2_FIFO;
                ++cnt;
            }

            *OUT_CSR1_REG&=~EPn_OUT_PKT_RDY;  // RECEIVED THE PACKET
            __usb_drvstatus&=~USB_STATUS_HIDRX;
            return;

        }

        if(startmarker==USB_BLOCKMARK_RESPONSE) {
            // RECEIVING A RESPONSE FROM THE REMOTE

            BYTE remotebusy=*EP2_FIFO;    // READ THE NEXT BYTE WITH THE STATUS

            if(!remotebusy) __usb_drvstatus&=~USB_STATUS_REMOTEBUSY;
            else __usb_drvstatus|=USB_STATUS_REMOTEBUSY;

            WORD txsize=*EP2_FIFO;
            txsize|=(*EP2_FIFO)<<8;
            txsize|=(*EP2_FIFO)<<16;
            txsize|=(*EP2_FIFO)<<24;
            fifocnt-=4;

            WORD txoffset=*EP2_FIFO;
            txoffset|=(*EP2_FIFO)<<8;
            txoffset|=(*EP2_FIFO)<<16;
            txoffset|=(*EP2_FIFO)<<24;
            fifocnt-=4;

            __usb_rembigtotal=txsize;
            __usb_rembigoffset=txoffset;    // FILE SIZE AND OFFSET AS REQUESTED BY THE REMOTE

            __usb_drvstatus|=USB_STATUS_REMOTERESPND;

            fifocnt-=2;

            // FLUSH THE FIFO
            BYTE sum=0;
            while(cnt<fifocnt) {
                sum+=(BYTE)*EP2_FIFO;
                ++cnt;
            }

            *OUT_CSR1_REG&=~EPn_OUT_PKT_RDY;  // RECEIVED THE PACKET
            __usb_drvstatus&=~USB_STATUS_HIDRX;
            return;


        }


        if(__usb_drvstatus&USB_STATUS_DATAREADY) {
            // PREVIOUS DATA WASN'T RETRIEVED!
            // STALL UNTIL USER RETRIEVES
            *OUT_CSR1_REG|=EPn_OUT_SEND_STALL;
            return;
        }

        if((startmarker&0xf0)==USB_BLOCKMARK_SINGLE) {
        __usb_rcvtotal=(*EP2_FIFO);
        __usb_rcvtotal|=(*EP2_FIFO)<<8;
        __usb_rcvtotal|=(*EP2_FIFO)<<16;

        __usb_rcvpartial=-8;
        __usb_rcvcrc=(*EP2_FIFO);
        __usb_rcvcrc|=(*EP2_FIFO)<<8;
        __usb_rcvcrc|=(*EP2_FIFO)<<16;
        __usb_rcvcrc|=(*EP2_FIFO)<<24;

        __usb_rcvblkmark=startmarker;

        cnt+=8;

        BYTEPTR buf;

        // ONLY ALLOCATE MEMORY FOR DATA BLOCKS LARGER THAN 1 PACKET
        buf=ram_simpmallocb(__usb_rcvtotal);

        if(!buf) {
            __usb_rcvbuffer=__usb_rxtmpbuffer;
        } else {
            __usb_rcvbuffer=buf;
            __usb_bufptr[2]=__usb_rcvbuffer;
        }

        // IF THIS IS THE START OF A NEW TRANSMISSION, DON'T IGNORE IT
        if((__usb_rcvblkmark==USB_BLOCKMARK_SINGLE)||(__usb_rcvblkmark==USB_BLOCKMARK_MULTISTART)) {  __usb_rcvcrcroll=0; __usb_drvstatus&=~USB_STATUS_IGNORE; }

        }

        else {
            // BAD START MARKER, TREAT THE BLOCK AS ARBITRARY DATA, BUT WILL FAIL CRC CHECKS
            ++cnt;
            __usb_rcvtotal=fifocnt;
            __usb_rcvbuffer=__usb_rxtmpbuffer;
            __usb_rcvcrc=0;
            __usb_rcvpartial=0;
            __usb_bufptr[2]=__usb_rcvbuffer+1;
            __usb_rxtmpbuffer[0]=startmarker;
            __usb_rcvblkmark=0;
            __usb_drvstatus|=USB_STATUS_IGNORE;     // IGNORE THIS BLOCK

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
            __usb_drvstatus|=USB_STATUS_IGNORE;     // IGNORE THIS BLOCK

        }

   }
    else  if(__usb_rcvbuffer==__usb_rxtmpbuffer)  __usb_bufptr[2]=__usb_rxtmpbuffer;    // IF WE HAD NO MEMORY, RESET THE BUFFER FOR EVERY PARTIAL PACKET


    while(cnt<fifocnt) {
        *__usb_bufptr[2]=(BYTE)*EP2_FIFO;
        ++__usb_bufptr[2];
        ++cnt;
    }

    *OUT_CSR1_REG&=~EPn_OUT_PKT_RDY;  // RECEIVED THE PACKET

    __usb_rcvpartial+=fifocnt;

    if((fifocnt!=EP2_FIFO_SIZE)||(__usb_rcvpartial>=__usb_rcvtotal)) {
        // PARTIAL PACKET SIGNALS END OF TRANSMISSION

        // IF WE HAD NO MEMORY TO ALLOCATE A BLOCK, INDICATE THAT WE RECEIVED ONLY THE LAST PARTIAL PACKET
        if(__usb_rcvbuffer==__usb_rxtmpbuffer) {
            __usb_rcvpartial=__usb_bufptr[2]-__usb_rcvbuffer;
        }
        __usb_drvstatus&=~USB_STATUS_HIDRX;
        if(!(__usb_drvstatus&USB_STATUS_IGNORE)) __usb_drvstatus|=USB_STATUS_DATAREADY;
        else {
         // RELEASE THE BUFFER, THE USER WILL NEVER SEE IT
            if(__usb_rcvbuffer!=__usb_rxtmpbuffer) ram_simpfree(__usb_rcvbuffer);
        }
        if((__usb_rcvblkmark==0)||(__usb_rcvblkmark==USB_BLOCKMARK_SINGLE)||(__usb_rcvblkmark==USB_BLOCKMARK_MULTIEND))  __usb_drvstatus&=~USB_STATUS_IGNORE;    // STOP IGNORING PACKETS AFTER THIS ONE
        return;
    }

    __usb_drvstatus|=USB_STATUS_HIDRX;

    }

}


// SENDING INTERRUPT ENDPOINT
void ram_ep1_irqservice()
{
    // ONLY RESPOND ONCE EVERYTHING IS SETUP
    if(!(__usb_drvstatus&USB_STATUS_CONFIGURED)) return;

    *INDEX_REG=RAWHID_TX_ENDPOINT;

    if(__usb_drvstatus&USB_STATUS_HIDTX) {
        // TRANSMISSION IN PROGRESS
        ram_usb_ep1_transmit(0);
        return;
    }


    if(__usb_drvstatus&USB_STATUS_DATARECEIVED) {
        // SEND ACKNOWLEDGEMENT THAT WE RECEIVED SOME DATA AND WE WANT THE NEXT OFFSET
        __usb_count[1]=10;
        __usb_padding[1]=RAWHID_TX_SIZE-10;
        __usb_txtmpbuffer[0]=USB_BLOCKMARK_RESPONSE;
        __usb_txtmpbuffer[1]=(__usb_drvstatus&(USB_STATUS_DATAREADY|USB_STATUS_DATAHALTED))? 1:0;
        __usb_txtmpbuffer[2]=__usb_rembigtotal&0xff;
        __usb_txtmpbuffer[3]=(__usb_rembigtotal>>8)&0xff;
        __usb_txtmpbuffer[4]=(__usb_rembigtotal>>16)&0xff;
        __usb_txtmpbuffer[5]=(__usb_rembigtotal>>24)&0xff;
        __usb_txtmpbuffer[6]=__usb_localbigoffset&0xff;
        __usb_txtmpbuffer[7]=(__usb_localbigoffset>>8)&0xff;
        __usb_txtmpbuffer[8]=(__usb_localbigoffset>>16)&0xff;
        __usb_txtmpbuffer[9]=(__usb_localbigoffset>>24)&0xff;

        __usb_bufptr[1]=__usb_txtmpbuffer;    // SEND THE STATUS
        ram_usb_ep1_transmit(1);
        __usb_drvstatus&=~USB_STATUS_DATARECEIVED;
        return;
    }


    // NOTHING TO TRANSMIT

    // JUST REPLY WITH A ZERO DATA PACKET
    *IN_CSR1_REG|=EPn_IN_PKT_RDY;

    //if(*IN_CSR1_REG&EPn_IN_SENT_STALL) return;  // ALREADY DONE

    //*IN_CSR1_REG|=EPn_IN_SEND_STALL;

    return;
}


// RECEIVING DATA ENDPOINT
void ram_ep2_irqservice()
{
    // ONLY RESPOND ONCE EVERYTHING IS SETUP
    if(!(__usb_drvstatus&USB_STATUS_CONFIGURED)) return;

    *INDEX_REG=RAWHID_RX_ENDPOINT;

    if(__usb_drvstatus&USB_STATUS_HIDRX) {
        // TRANSMISSION IN PROGRESS
        ram_usb_ep2_receive(0);
        return;
    }

    // NOTHING TO RECEIVE, STALL
    if(*OUT_CSR1_REG&EPn_OUT_PKT_RDY) {
        // WE HAVE A PACKET, FIRST OF A TRANSMISSION
        ram_usb_ep2_receive(1);
        return;
    }

    if(*OUT_CSR1_REG&EPn_OUT_SENT_STALL) return;  // ALREADY DONE


    *OUT_CSR1_REG|=EPn_OUT_SEND_STALL;

    return;
}



// GENERAL INTERRUPT SERVICE ROUTINE - DISPATCH TO INDIVIDUAL ENDPOINT ROUTINES
void ram_usb_irqservice()
{

    if(!(__usb_drvstatus&USB_STATUS_INIT)) return;

    *INDEX_REG=0;

    if( !(*EP_INT_REG&7) && !(*USB_INT_REG&7) )
    {
        // WHAT ARE THESE INTERRUPTS FOR?
        if(__usb_drvstatus&(USB_STATUS_EP0TX|USB_STATUS_EP0RX)) ram_ep0_irqservice();
        return;
    }

    if(*EP_INT_REG&1) {
        ram_ep0_irqservice();
        *EP_INT_REG=1;
        return;
    }
    if(*EP_INT_REG&2) {
        ram_ep1_irqservice();
        *EP_INT_REG=2;
        return;
    }
    if(*EP_INT_REG&4) {
        ram_ep2_irqservice();
        *EP_INT_REG=4;
        return;
    }

    if(*USB_INT_REG&1) {
        // ENTER SUSPEND MODE
        ram_usb_hwsuspend();
        *USB_INT_REG=1;
        __usb_drvstatus|=USB_STATUS_SUSPEND;
        return;
    }

    if(*USB_INT_REG&2) {
        // RESUME FROM SUSPEND MODE
        ram_usb_hwresume();
        *USB_INT_REG=2;
        __usb_drvstatus&=~USB_STATUS_SUSPEND;
        return;
    }

    if(*USB_INT_REG&4) {
        // RESET RECEIVED
        if( (*PWR_REG)&USB_RESET) {
        __usb_drvstatus=USB_STATUS_INIT|USB_STATUS_CONNECTED;  // DECONFIGURE THE DEVICE
        ram_usb_hwsetup();      // AND MAKE SURE THE HARDWARE IS IN KNOWN STATE
        }
        *USB_INT_REG=4;
        return;
    }


}

extern unsigned int irq_table[32] ;

void __ram_irq_service() __attribute__ ((naked));
void __ram_irq_service()
{
    asm volatile ("stmfd sp!, {r0-r12,lr}");
    asm volatile ("mov r0,sp");
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0x1f");
    asm volatile ("msr cpsr_all,r1");   // SWITCH TO SYSTEM MODE
    asm volatile ("stmfd r0!,{sp,lr}"); // SAVE REGISTERS THAT WERE BANKED
    asm volatile ("stmfd sp!,{ r0 }");  // SAVE IRQ STACK PTR
    *HWREG(INT_REGS,0x0)=*HWREG(INT_REGS,0x10); // CLEAR SRCPENDING EARLY TO AVOID MISSING ANY OTHER INTERRUPTS
    (*( (__interrupt__) (irq_table[*HWREG(INT_REGS,0x14)]))) ();
    // CLEAR INTERRUPT PENDING FLAG
    register unsigned int a=1<<(*HWREG(INT_REGS,0x14));
    *HWREG(INT_REGS,0x10)=a;

    asm volatile ("ldmia sp!, { r0 }"); // GET IRQ STACK PTR
    asm volatile ("ldmia r0!, { sp, lr }"); // RESTORE USER STACK AND LR
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0xd");
    asm volatile ("msr cpsr_all,r1");   // SWITCH BACK TO IRQ MODE
    asm volatile ("ldmia sp!, {r0-r12,lr}");    // RESTORE ALL OTHER REGISTERS BACK
    asm volatile ("subs pc,lr,#4");
}









void ram_doreset()
{

    // DO A FULL RESET

 // SET THE PRESCALER OF THE WATCHDOG AS FAST AS POSSIBLE AND A REASONABLE COUNT (ABOUT 87ms)
    *HWREG(WDT_REGS,8)=0x8000;
    *HWREG(WDT_REGS,0)=0x21;

    // AND WAIT FOR IT TO HAPPEN
    while(1);

}

#define FLASH_SECTORSIZE 4096  // SECTOR SIZE IN BYTES
#define WRITE_HALFWORD_FLASH(address,data) *((volatile HALFWORD *)(address<<1))=data

// ERASE FLASH AREA BETWEEN GIVEN ADDRESSES (IN 32-BIT WORDS)

void ram_flasherase(WORDPTR address,int nwords)
{
volatile HALFWORD *ptr=(HALFWORD *)(((WORD)address)&~(FLASH_SECTORSIZE-1));  // FIND START OF SECTOR
nwords+=(((WORD)address)&(FLASH_SECTORSIZE-1))>>2;  // CORRECTION FOR MISALIGNED SECTORS

HALFWORD data,prevdata;



// START ERASING SECTORS OF FLASH
while(nwords>0) {
    if(((WORD)ptr>=0x4000)&&((WORD)ptr<0x00200000)) {  // NEVER ERASE THE BOOT LOADER!

        // DISABLE INTERRUPTS
        asm volatile ("stmfd sp!, {r0}");
        asm volatile ("mrs r0,cpsr_all");
        asm volatile ("orr r0,r0,#0xc0");
        asm volatile ("msr cpsr_all,r0");
        asm volatile ("ldmia sp!, { r0 }");

    // SECTOR ERASE COMMAND
    WRITE_HALFWORD_FLASH(0x5555,0x00AA);     // write data 0x00AA to device addr 0x5555
    WRITE_HALFWORD_FLASH(0x2AAA,0x0055);     // write data 0x0055 to device addr 0x2AAA
    WRITE_HALFWORD_FLASH(0x5555,0x0080);     // write data 0x0080 to device addr 0x5555
    WRITE_HALFWORD_FLASH(0x5555,0x00AA);     // write data 0x00AA to device addr 0x5555
    WRITE_HALFWORD_FLASH(0x2AAA,0x0055);     // write data 0x0055 to device addr 0x2AAA
    *ptr=0x0030;                             // write data 0x0030 to device sector addr

    // USE TOGGLE READY TO WAIT FOR COMPLETION
    prevdata=*ptr;
    data=prevdata^0xffff;
    while(data^prevdata)
    {
        prevdata=data;
        data=*ptr;
    }
    }

    // RE-ENABLE INTERRUPTS
    asm volatile ("stmfd sp!, {r1}");
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");
    asm volatile ("ldmia sp!, { r1 }");



    nwords-=(FLASH_SECTORSIZE>>2);  // IN 32-BIT WORDS
    ptr+=(FLASH_SECTORSIZE>>1);     // IN 16-BIT HALFWORDS
}
// DONE - FLASH WAS ERASED
}

// PROGRAM A 32-BIT WORD INTO FLASH

void ram_flashprogramword(WORDPTR address,WORD value)
{

    volatile HALFWORD *ptr=(HALFWORD *)address;
    HALFWORD data,prevdata;
            // DISABLE INTERRUPTS
            asm volatile ("stmfd sp!, {r0}");
            asm volatile ("mrs r0,cpsr_all");
            asm volatile ("orr r0,r0,#0xc0");
            asm volatile ("msr cpsr_all,r0");
            asm volatile ("ldmia sp!, { r0 }");


        // PROGRAM WORD COMMAND
        WRITE_HALFWORD_FLASH(0x5555,0x00AA);
        WRITE_HALFWORD_FLASH(0x2AAA,0x0055);
        WRITE_HALFWORD_FLASH(0x5555,0x00A0);
        *ptr=(HALFWORD)(value&0xffff);

        // USE TOGGLE READY TO WAIT FOR COMPLETION
        prevdata=*ptr;
        data=prevdata^0xffff;
        while(data^prevdata)
        {
            prevdata=data;
            data=*ptr;
        }

        ++ptr;

        // PROGRAM WORD COMMAND
        WRITE_HALFWORD_FLASH(0x5555,0x00AA);
        WRITE_HALFWORD_FLASH(0x2AAA,0x0055);
        WRITE_HALFWORD_FLASH(0x5555,0x00A0);
        *ptr=(HALFWORD)((value>>16)&0xffff);

        // USE TOGGLE READY TO WAIT FOR COMPLETION
        prevdata=*ptr;
        data=prevdata^0xffff;
        while(data^prevdata)
        {
            prevdata=data;
            data=*ptr;
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
void ram_receiveandflashfw(WORD flashsize)
{

do {

ram_usb_receivelong_start();

WORDPTR flash_address;
WORD flash_nwords,data;

data=0xffffffff;

if(ram_usb_receivelong_word((WORDPTR)&data)!=1)  ram_doreset(); // NOTHING ELSE TO DO

if(data!=TEXT2WORD('F','W','U','P'))  {
    {unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
    scrptr[10]=data;}
    while(1);
    ram_doreset(); // NOTHING ELSE TO DO
}

{unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
scrptr[4]=0xf0f0f0f0;}
if(ram_usb_receivelong_word((WORDPTR)&flash_address)!=1)  ram_doreset(); // NOTHING ELSE TO DO

{unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
scrptr[4]=0xffff6666;}
if(ram_usb_receivelong_word(&flash_nwords)!=1)  ram_doreset();
{unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
scrptr[4]=0x6666ffff;}

if((WORD)(flash_address+flash_nwords)>flashsize) {
    // ROM TOO BIG!
    ram_doreset();
}

if(((WORD)flash_address<0x4000))  {
    ram_doreset(); // PROTECT THE BOOTLOADER AT ALL COSTS
}

// DEBUG
{
    // SHOW SOME VISUALS
unsigned char *scrptr=(unsigned char *)MEM_PHYS_SCREEN;
scrptr+=65;
scrptr+=N_ALPHA*80;
*scrptr=(*scrptr&0xf)|0xf0;
}

// ERASE THE FLASH
{unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
scrptr[4]=0xf0f06666;}




ram_flasherase(flash_address,flash_nwords );    // ERASE ENOUGH FLASH BLOCKS TO PREPARE FOR FIRMWARE UPDATE

{unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
scrptr[4]=0xffffffff;}

{
    // SHOW SOME VISUALS
unsigned char *scrptr=(unsigned char *)MEM_PHYS_SCREEN;
scrptr+=65;
scrptr+=N_HOURGLASS*80;
*scrptr=(*scrptr&0xf)|0xf0;
}

while(flash_nwords--) {
    if(ram_usb_receivelong_word(&data)!=1) ram_doreset();

    ram_flashprogramword(flash_address,data);
    ++flash_address;

    // SHOW SOME VISUALS
    unsigned char *scrptr=(unsigned char *)MEM_PHYS_SCREEN;
    scrptr+=65;
    scrptr+=N_LEFTSHIFT*80;
    *scrptr=(*scrptr&0xf)|((((WORD)flash_address)&0xf000)>>12);
}

// WE FINISHED PROGRAMMING THE FLASH!

// SHOW SOME VISUALS
{
unsigned char *scrptr=(unsigned char *)MEM_PHYS_SCREEN;
scrptr+=65;
scrptr+=N_RIGHTSHIFT*80;
*scrptr=(*scrptr&0xf)|0xf0;
}


ram_usb_receivelong_finish();
} while(1);
//ram_doreset();

// NEVER RETURNS
}





void ram_startfwupdate()
{

    // AT THIS POINT, A USB CONNECTION HAS ALREADY BEEN ESTABLISHED
    // THIS ROUTINE WILL MAKE SURE WE RUN FROM RAM, ERASE ALL FLASH AND UPDATE THE FIRMWARE

     HALFWORD cfidata[50];   // ACTUALLY 36 WORDS ARE USED ONLY
    flash_CFIRead(cfidata);


    flash_prepareforwriting();

    {unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
    scrptr[2]=0xf0f0f0f0;}

    // CHECK THE SIZE OF RAM
    int flashsize=1<<(WORD)(cfidata[0x17]);

    // NOW COPY THE CODE TO RAM

    int needwords=(WORDPTR)&ram_startfwupdate-(WORDPTR)&ram_simpmallocb;

    rplExpandStack(needwords);
    if(Exceptions) return;

    WORDPTR codeblock=(WORDPTR)DSTop;    // STORE CODE ON TOP OF THE STACK

        // INITIALIZE FAKE MEMORY ALLOCATOR
        ram_memblocks[0]=(BYTEPTR)allocRegister();
        if(ram_memblocks[0]==0) { Exceptions|=EX_OUTOFMEM; return; }
        ram_memblocks[1]=(BYTEPTR)allocRegister();
        if(ram_memblocks[1]==0) { Exceptions|=EX_OUTOFMEM; return; }

        ram_memblocksused[0]=0;
        ram_memblocksused[1]=0;

        __usb_longbuffer[0]=simpmallocb(LONG_BUFFER_SIZE);      // PREALLOCATE 2 LARGE BUFFERS
        if(__usb_longbuffer[0]==0) { Exceptions|=EX_OUTOFMEM; return; }
        __usb_longbuffer[1]=simpmallocb(LONG_BUFFER_SIZE);
        if(__usb_longbuffer[1]==0) { Exceptions|=EX_OUTOFMEM; return; }

        if(ram_memblocks[0]==0)
    memmovew(codeblock,&ram_simpmallocb,needwords);

    // ALSO COPY THE CRC TABLE FROM ROM TO RAM
    int k;
    for(k=0;k<256;++k) RAM_CRCTABLE[k]=__crctable[k];


    // EVERYTHING IS NOW IN RAM

    {unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
    scrptr[2]=0xff00ff00;}
    // DISABLE ALL INTERRUPTS
    cpu_intoff();
    // MAKE SURE THE CODE WAS COPIED TO RAM BEFORE WE EXECUTE IT
    cpu_flushwritebuffers();

    // MOVE MAIN ISR TO RAM AS WELL
    *( (unsigned int *) 0x08000018)=(unsigned int) (codeblock+((WORDPTR)&__ram_irq_service-(WORDPTR)&ram_simpmallocb));

    {unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
    scrptr[2]=0xffff0000;}

    // AND MAKE SURE WE DON'T EXECUTE AN OLD COPY LEFT IN THE CACHE
    //cpu_flushicache();    // NO NEED,

    {unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
    scrptr[2]=0x0000ffff;}




    // SET INTERRUPT HANDLER IN RAM
    __irq_addhook(25,(void *)(codeblock + ((WORDPTR)&ram_usb_irqservice-(WORDPTR)&ram_simpmallocb)));

    // UNMASK ONLY THE ONE INTERRUPT WE NEED
    __irq_unmask(25);

    {unsigned int *scrptr=(unsigned int *)MEM_PHYS_SCREEN;
    scrptr[2]=0xffffffff;}

    void (*ptr)(int);

    ptr=(void *) (codeblock+((WORDPTR)&ram_receiveandflashfw-(WORDPTR)&ram_simpmallocb));

    (ptr)(flashsize);

    // THIS CAN NEVER RETURN, THERE WILL BE NO ROM HERE

    while(1);

}

