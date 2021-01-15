/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include <newrpl.h>
#include <ui.h>

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

//********************************************
// NEW HARDWARE-INDEPENDENT CODE
//********************************************

// TRANSMIT ONE CONTROL PACKET

void usb_sendcontrolpacket(int packet_type)
{
    if(!(__usb_drvstatus & USB_STATUS_INSIDEIRQ)) {
        while(__usb_drvstatus & USB_STATUS_TXCTL);      // THERE'S ANOTHER CONTROL PACKET, WAIT FOR IT TO BE SENT
    }

    // NOW PREPARE THE NEXT CONTROL PACKET IN THE BUFFER
    USB_PACKET *p = (USB_PACKET *) __usb_ctltxbuffer;
    memsetb(__usb_ctltxbuffer, 0, RAWHID_TX_SIZE + 1);

    usb_mutex_lock();

    switch (packet_type) {
    case P_TYPE_GETSTATUS:
        p->p_type = P_TYPE_GETSTATUS;
        p->p_fileidLSB = (BYTE) (__usb_fileid & 0xff);
        p->p_fileidMSB = (BYTE) (__usb_fileid >> 8);
        __usb_drvstatus &= ~USB_STATUS_RXRCVD;
        break;
    case P_TYPE_CHECKPOINT:
        p->p_type = P_TYPE_CHECKPOINT;
        p->p_fileidLSB = (BYTE) (__usb_fileid & 0xff);
        p->p_fileidMSB = (BYTE) (__usb_fileid >> 8);
        p->p_offset = __usb_offset;
        p->p_data[0] = __usb_crc32 & 0xff;
        p->p_data[1] = (__usb_crc32 >> 8) & 0xff;
        p->p_data[2] = (__usb_crc32 >> 16) & 0xff;
        p->p_data[3] = (__usb_crc32 >> 24) & 0xff;
        __usb_drvstatus &= ~USB_STATUS_RXRCVD;
        //__usb_drvstatus|=USB_STATUS_HALT;
        break;

    case P_TYPE_ENDOFFILE:
        p->p_type = P_TYPE_ENDOFFILE;
        p->p_fileidLSB = (BYTE) (__usb_fileid & 0xff);
        p->p_fileidMSB = (BYTE) (__usb_fileid >> 8);
        p->p_offset = __usb_offset;
        p->p_data[0] = __usb_crc32 & 0xff;
        p->p_data[1] = (__usb_crc32 >> 8) & 0xff;
        p->p_data[2] = (__usb_crc32 >> 16) & 0xff;
        p->p_data[3] = (__usb_crc32 >> 24) & 0xff;
        __usb_drvstatus &= ~USB_STATUS_RXRCVD;
        break;

    case P_TYPE_ABORT:
        p->p_type = P_TYPE_ABORT;
        p->p_fileidLSB = (BYTE) (__usb_fileid & 0xff);
        p->p_fileidMSB = (BYTE) (__usb_fileid >> 8);
        break;

    case P_TYPE_REPORT:
        p->p_type = P_TYPE_REPORT;
        p->p_fileidLSB = (BYTE) (__usb_fileid & 0xff);
        p->p_fileidMSB = (BYTE) (__usb_fileid >> 8);
        p->p_offset = __usb_offset;
        p->p_data[0] = (__usb_drvstatus & USB_STATUS_HALT) ? 1 : 0;
        p->p_data[1] = (__usb_drvstatus & USB_STATUS_ERROR) ? 1 : 0;
        p->p_data[2] = (__usb_rxtotalbytes) ? 1 : 0;
        break;

    default:
        usb_mutex_unlock();
        return; // INVALID PACKET TO SEND
    }
    __usb_drvstatus |= USB_STATUS_TXCTL;        // INDICATE THE DRIVER WE HAVE TO SEND A CONTROL PACKET
    usb_mutex_unlock();
}

// CALLED WHEN A REPORT ARRIVED FROM THE OTHER SIDE, PROCESS DEPENDING ON WHAT WE ARE DOING
void usb_receivecontrolpacket()
{
    if(__usb_drvstatus & USB_STATUS_RXCTL) {

        USB_PACKET *ctl = (USB_PACKET *) __usb_ctlrxbuffer;

        switch (ctl->p_type) {
        case P_TYPE_GETSTATUS:
        {
            if(!__usb_fileid) {
                usb_mutex_lock();

                // START RECEIVING A NEW TRANSMISSION
                __usb_fileid = P_FILEID(ctl);
                __usb_offset = 0;
                __usb_crc32 = 0;
                __usb_rxoffset = 0;
                __usb_rxtxtop = 0;      // NUMBER OF BYTES USED IN THE RX BUFFER
                __usb_rxtxbottom = 0;   // NUMBER OF BYTES IN THE RX BUFFER ALREADY READ BY THE USER
                __usb_rxtotalbytes = 0; // DON'T KNOW THE TOTAL FILE SIZE YET
                __usb_drvstatus &=
                        ~(USB_STATUS_HALT | USB_STATUS_ERROR | USB_STATUS_EOF);

                usb_mutex_unlock();
            }
            // SEND A REPORT, IF WE HAVE AN EXISTING TRANSMISSION, IT WILL REPLY WITH THE OLD FILEID
            usb_sendcontrolpacket(P_TYPE_REPORT);
            break;
        }
        case P_TYPE_CHECKPOINT:
        {

            if(__usb_fileid == P_FILEID(ctl)) {
                usb_mutex_lock();

                if(__usb_drvstatus & USB_STATUS_ERROR) {
                    // IGNORE THE CHECKPOINT, THERE WAS A PRIOR ERROR
                    usb_mutex_unlock();
                    break;
                }
                __usb_drvstatus &= ~USB_STATUS_ERROR;   // REMOVE ERROR SIGNAL

                int used = __usb_rxtxtop - __usb_rxtxbottom;
                if(used < 0)
                    used += RING_BUFFER_SIZE;
                if(__usb_rxoffset + used != ctl->p_offset) {
                    // SOMETHING WENT WRONG, WE DISAGREE ON THE FILE SIZE
                    __usb_drvstatus |= USB_STATUS_ERROR;        // SIGNAL TO RESEND FROM CURRENT OFFSET
                }
                WORD crc = ctl->p_data[0];
                crc |= ((WORD) ctl->p_data[1]) << 8;
                crc |= ((WORD) ctl->p_data[2]) << 16;
                crc |= ((WORD) ctl->p_data[3]) << 24;
                if(__usb_crc32 != crc) {
                    __usb_drvstatus |= USB_STATUS_ERROR;        // SIGNAL TO RESEND FROM CURRENT OFFSET
                }

                usb_mutex_unlock();

                // SEND THE REPORT
                usb_sendcontrolpacket(P_TYPE_REPORT);
            }
            break;
        }
        case P_TYPE_ENDOFFILE:
        {
            if(__usb_fileid == P_FILEID(ctl)) {
                usb_mutex_lock();
                
                if(__usb_drvstatus & USB_STATUS_ERROR) {
                    // IGNORE THE END OF FILE, THERE WAS A PRIOR ERROR
                    usb_mutex_unlock();
                    break;
                }

                // SAME AS FOR A CHECKPOINT, BUT SET TOTAL BYTE COUNT
                __usb_drvstatus &= ~USB_STATUS_ERROR;   // REMOVE ERROR SIGNAL

                if(__usb_offset != ctl->p_offset) {
                    // SOMETHING WENT WRONG, WE DISAGREE ON THE FILE SIZE
                    __usb_drvstatus |= USB_STATUS_ERROR;        // SIGNAL TO RESEND FROM CURRENT OFFSET
                }
                WORD crc = ctl->p_data[0];
                crc |= ((WORD) ctl->p_data[1]) << 8;
                crc |= ((WORD) ctl->p_data[2]) << 16;
                crc |= ((WORD) ctl->p_data[3]) << 24;
                if(__usb_crc32 != crc)
                    __usb_drvstatus |= USB_STATUS_ERROR;        // SIGNAL TO RESEND FROM CURRENT OFFSET

                // SET TOTAL BYTES TO INDICATE WE RECEIVED THE LAST OF IT
                if(!(__usb_drvstatus & USB_STATUS_ERROR))
                    __usb_rxtotalbytes = ctl->p_offset;
                
                usb_mutex_unlock();

                // SEND A REPORT
                usb_sendcontrolpacket(P_TYPE_REPORT);
            }
            break;

        }
        case P_TYPE_ABORT:
        {
            usb_mutex_lock();

            if((__usb_fileid == P_FILEID(ctl)) || (P_FILEID(ctl) == 0xffff)) {

                // REMOTE REQUESTED TO ABORT WHATEVER WE WERE DOING
                __usb_drvstatus &=
                        ~(USB_STATUS_TXDATA | USB_STATUS_TXCTL |
                        USB_STATUS_RXDATA | USB_STATUS_HALT | USB_STATUS_ERROR |
                        USB_STATUS_RXCTL | USB_STATUS_EOF);

                // ABORT ALL TRANSACTIONS
                __usb_fileid = 0;
                __usb_offset = 0;
                __usb_crc32 = 0;
                __usb_rxtxbottom = 0;
                __usb_rxtxtop = 0;
                __usb_rxoffset = 0;
                __usb_rxtotalbytes = 0;
                __usb_txtotalbytes = 0;

            }

            __usb_drvstatus |= USB_STATUS_RXRCVD;

            usb_mutex_unlock();

            // DO NOT REPLY TO AN ABORT CONDITION

            break;

        }

        case P_TYPE_REPORT:
        {
            usb_mutex_lock();

            if(__usb_fileid == P_FILEID(ctl)) {

                // UPDATE FLAGS WITH THE STATUS OF THE REMOTE
                if(ctl->p_data[0])
                    __usb_drvstatus |= USB_STATUS_HALT;
                else
                    __usb_drvstatus &= ~USB_STATUS_HALT;
                if(ctl->p_data[1]) {
                    // SIGNAL THE ERROR AND LEAVE THE REQUESTED OFFSET AT rxoffset
                    __usb_drvstatus |= USB_STATUS_ERROR;
                    __usb_rxoffset = ctl->p_offset;
                }
                else {
                    __usb_drvstatus &= ~USB_STATUS_ERROR;
                }
                if(ctl->p_data[2]) {
                    // SIGNAL THAT THE REMOTE ACKNOWLEDGED THE END OF FILE MARK
                    __usb_drvstatus |= USB_STATUS_EOF;
                }
                else
                    __usb_drvstatus &= ~USB_STATUS_EOF;

            }
            else {
                // WE RECEIVED A REPORT OF THE WRONG FILE, THE REMOTE ISN'T EVEN WORKING ON OUR FILE YET
                __usb_drvstatus |= USB_STATUS_ERROR;
            }

            __usb_drvstatus |= USB_STATUS_RXRCVD;

            usb_mutex_unlock();

            break;
        }
        default:
            // DO NOTHING
            break;

        }

    }

    usb_mutex_lock();
    // SIGNAL THAT IT WAS PROCESSED
    __usb_drvstatus &= ~USB_STATUS_RXCTL;
    usb_mutex_unlock();
}

int usb_isconnected()
{
    if(__usb_drvstatus & USB_STATUS_CONNECTED)
        return 1;
    return 0;
}

int usb_isconfigured()
{
    if(__usb_drvstatus & USB_STATUS_CONFIGURED)
        return 1;
    return 0;
}

// HIGH LEVEL FUNCTION TO SEE IF THERE'S ANY DATA FROM THE USB DRIVER
int usb_hasdata()
{
    if((__usb_drvstatus & USB_STATUS_RXDATA)
            && (__usb_rxtxtop != __usb_rxtxbottom)) {
        int bytesready = __usb_rxtxtop - __usb_rxtxbottom;
        if(bytesready < 0)
            bytesready += RING_BUFFER_SIZE;
        return bytesready;
    }
    return 0;
}

// HIGH LEVEL FUNCTION TO BLOCK UNTIL DATA ARRIVES
// WAIT UNTIL WE GET AT LEAST nbytes OR TIMEOUT
// RETURN 0 IF TIMEOUT
int usb_waitfordata(int nbytes)
{
    tmr_t start = tmr_ticks(), end;
    int prevbytes = 0, hasbytes;

    hasbytes = usb_hasdata();

    while(hasbytes < nbytes) {
        if((__usb_drvstatus & (USB_STATUS_CONFIGURED | USB_STATUS_INIT |
                        USB_STATUS_CONNECTED)) !=
                (USB_STATUS_CONFIGURED | USB_STATUS_INIT |
                    USB_STATUS_CONNECTED))
            return 0;

        end = tmr_ticks();
        if(tmr_ticks2ms(start, end) > USB_TIMEOUT_MS) {
            // TIMEOUT - CLEANUP AND RETURN
            return 0;
        }

        hasbytes = usb_hasdata();
        if(hasbytes != prevbytes)
            start = tmr_ticks();        // RESET THE TIMEOUT IF WE GET SOME DATA ON THE WIRE
        prevbytes = hasbytes;

        if(__usb_drvstatus & USB_STATUS_HALT) {
            // NO MORE DATA WILL COME BECAUSE OUR BUFFERS ARE FULL, EMPTY THE BUFFERS BY RETURNING WHAT WE HAVE SO FAR
            break;
        }

        if(__usb_rxtotalbytes) {
            // WE GOT ALL THE DATA IN THE FILE, NO MORE DATA IS COMING
            break;
        }

        cpu_waitforinterrupt();

    }

    return hasbytes;
}

// HIGH LEVEL FUNCTION TO ACCESS A BLOCK OF DATA
BYTEPTR usb_accessdata(int *datasize)
{
    int bytes;
    if((__usb_drvstatus & USB_STATUS_RXDATA)
            && (__usb_rxtxtop != __usb_rxtxbottom)) {
        bytes = __usb_rxtxtop - __usb_rxtxbottom;
        if(bytes < 0)
            bytes = RING_BUFFER_SIZE - __usb_rxtxbottom;
    }
    else
        bytes = 0;

    if(bytes && datasize)
        *datasize = bytes;
    return __usb_rxtxbuffer + __usb_rxtxbottom;
}

// HIGH LEVEL FUNCTION TO RELEASE A BLOCK OF DATA AND GET READY TO RECEIVE THE NEXT
void usb_releasedata(int datasize)
{
    if(!(__usb_drvstatus & USB_STATUS_RXDATA))
        return;

    __usb_rxtxbottom += datasize;
    if(__usb_rxtxbottom >= RING_BUFFER_SIZE)
        __usb_rxtxbottom -= RING_BUFFER_SIZE;
}

// WAIT FOR A CONTROL PACKET TO COME BACK FROM THE REMOTE
int usb_waitforreport()
{
    tmr_t start, end;

    // WAIT FOR ALL PREVIOUS DATA TO BE SENT COMPLETELY
    start = tmr_ticks();
    while(!(__usb_drvstatus & USB_STATUS_RXRCVD)) {
        end = tmr_ticks();
        if(tmr_ticks2ms(start, end) > USB_TIMEOUT_MS) {
            return 0;
        }
    }
    return 1;
}

// RETRIEVE LAST CONTROL PACKET WE RECEIVED
USB_PACKET *usb_getreport()
{
    if(__usb_drvstatus & USB_STATUS_RXRCVD)
        return (USB_PACKET *) __usb_ctlrxbuffer;
    return 0;
}

// RELEASE THE CONTROL PACKET
void usb_releasereport()
{
    usb_mutex_lock();
    __usb_drvstatus &= ~USB_STATUS_RXRCVD;
    usb_mutex_unlock();
}

// START TRANSMISSION OF A FILE
// file_type = 'O','B','W', OR 'D', SEE SPECS
int usb_txfileopen(int file_type)
{
    tmr_t start, end;

    // WAIT FOR ALL PREVIOUS DATA TO BE SENT COMPLETELY
    start = tmr_ticks();
    while(__usb_drvstatus & USB_STATUS_TXDATA) {
        end = tmr_ticks();
        if(tmr_ticks2ms(start, end) > USB_TIMEOUT_MS) {
            return 0;
        }
    }

    __usb_rxtxtop = __usb_rxtxbottom = 0;
    __usb_txseq = 0;    // FIRST PACKET NUMBER
    __usb_offset = 0;
    __usb_crc32 = 0;    // RESET CRC32
    __usb_txtotalbytes = 0;
    // CREATE A NEW FILEID
    ++__usb_fileid_seq;
    __usb_fileid_seq &= 0xff;
    __usb_fileid = (file_type << 8) & 0xff00;
    __usb_fileid += (WORD) __usb_fileid_seq;

    // INDICATE WE ARE STARTING A TRANSMISSION, WAIT FOR REMOTE TO BE AVAILABLE
    start = tmr_ticks();
    do {
        do {
            end = tmr_ticks();
            if(tmr_ticks2ms(start, end) > USB_TIMEOUT_MS) {
                __usb_fileid = 0;
                return 0;       // FAIL IF TIMEOUT
            }

            usb_sendcontrolpacket(P_TYPE_GETSTATUS);
            if(!usb_waitforreport()) {
                __usb_fileid = 0;
                return 0;       // FAIL IF TIMEOUT
            }
            USB_PACKET *ptr = usb_getreport();

            if(P_FILEID(ptr) == __usb_fileid) {
                // THE REMOTE WANTS TO ABORT THIS FILE ALREADY?
                if(ptr->p_type == P_TYPE_ABORT)
                    return 0;   // FAIL DUE TO ABORT

                if(ptr->p_type == P_TYPE_REPORT) {
                    usb_releasereport();
                    break;
                }
            }
            else {
                // REPLYING WITH A DIFFERENT FILEID, PERHAPS IT'S STILL CLOSING THE PREVIOUS FILE
                // KEEP WAITING
            }
            usb_releasereport();
        }
        while(1);

    }
    while(__usb_drvstatus & (USB_STATUS_ERROR | USB_STATUS_HALT));

    // WE ARE READY TO START!

    return __usb_fileid;

}

// WRITE BYTES TO A FILE BEING SENT

int usb_filewrite(int fileid, BYTEPTR data, int nbytes)
{
    if(fileid != __usb_fileid)
        return 0;

    tmr_t start, end;
    int available, sent;

    start = tmr_ticks();
    sent = 0;

    // WAIT FOREVER UNTIL WE ARE DONE WRITING, BUT TIMEOUT ON ERRORS
    while(nbytes > 0) {
        if((__usb_drvstatus & (USB_STATUS_CONFIGURED | USB_STATUS_INIT |
                        USB_STATUS_CONNECTED)) !=
                (USB_STATUS_CONFIGURED | USB_STATUS_INIT |
                    USB_STATUS_CONNECTED))
            return 0;

        end = tmr_ticks();
        if(tmr_ticks2ms(start, end) > USB_TIMEOUT_MS)
            return 0;

        if(!__usb_fileid)
            return 0;   // FILE WAS ABORTED

        while(__usb_drvstatus & USB_STATUS_INSIDEIRQ);  // MAKE SURE WE THE DRIVER DOESN'T CHANGE THE BUFFER POINTERS
        if(__usb_drvstatus & (USB_STATUS_ERROR))
            continue;   // DO NOT FILL UP THE BUFFER WHEN THERE'S AN ERROR, WE MIGHT NEED OLD DATA

        available = __usb_rxtxbottom - __usb_rxtxtop;
        if(available <= 0) {
            available += RING_BUFFER_SIZE;
            if(available >= 4)
                available -= 4; // DO NOT FILL UP THE BUFFER ALL THE WAY, LEAVE THE LAST WORD TO SEPARATE TOP AND BOTTOM
            else
                available = 0;
        }
        else {
            if(available >= 4)
                available -= 4;
            else
                available = 0;
        }

        if(available > nbytes)
            available = nbytes;

        if(available && (!(__usb_drvstatus & USB_STATUS_HALT))) {
            start = tmr_ticks();

            if(available > RING_BUFFER_SIZE - __usb_rxtxtop) {
                // SPLIT MEMORY IN 2 COPIES
                memmoveb(__usb_rxtxbuffer + __usb_rxtxtop, data + sent, RING_BUFFER_SIZE - __usb_rxtxtop);      // MOVE THE DATA TO THE RING BUFFER
                memmoveb(__usb_rxtxbuffer, data + sent + RING_BUFFER_SIZE - __usb_rxtxtop, available - (RING_BUFFER_SIZE - __usb_rxtxtop));     // MOVE THE DATA TO THE RING BUFFER
            }
            else
                memmoveb(__usb_rxtxbuffer + __usb_rxtxtop, data + sent, available);     // MOVE THE DATA TO THE RING BUFFER
            // THIS OPERATION SHOULD BE ATOMIC SINCE IT MODIFIES THE BUFFER
            int new__usb_rxtxtop = __usb_rxtxtop + available;
            if(new__usb_rxtxtop >= RING_BUFFER_SIZE)
                new__usb_rxtxtop -= RING_BUFFER_SIZE;
            __usb_rxtxtop = new__usb_rxtxtop;
            usb_mutex_lock();
            __usb_drvstatus |= USB_STATUS_TXDATA;
            usb_mutex_unlock();
            nbytes -= available;
            sent += available;
        }

    }
    return 1;
}

int usb_txfileclose(int fileid)
{
    if(fileid != __usb_fileid)
        return 0;

    while(__usb_drvstatus & USB_STATUS_INSIDEIRQ);      // MAKE SURE WE ARE OUT OF THE IRQ HANDLER
    // SET THE TOTAL SIZE OF THE FILE BASED ON THE LAST BUFFER SENT
    int total = __usb_rxtxtop - __usb_rxtxbottom;
    if(total < 0)
        total += RING_BUFFER_SIZE;
    __usb_txtotalbytes = __usb_offset + total;

    // SIGNAL THAT WE HAVE A NEW BUFFER READY
    // IF NOT CURRENTLY SENDING ANYTHING, A ZERO-BYTE PACKET WILL BE SENT INDICATING END-OF-FILE
    usb_mutex_lock();
    __usb_drvstatus |= USB_STATUS_TXDATA;
    usb_mutex_unlock();

    // BLOCK UNTIL TRANSMISSION IS COMPLETE
    tmr_t start, end;
    int prevoffset = 0, result = 1;
    start = tmr_ticks();
    do {

        if((__usb_drvstatus & (USB_STATUS_CONFIGURED | USB_STATUS_INIT |
                        USB_STATUS_CONNECTED)) !=
                (USB_STATUS_CONFIGURED | USB_STATUS_INIT |
                    USB_STATUS_CONNECTED))
            return 0;

        if(__usb_drvstatus & USB_STATUS_EOF)
            break;      // WE RECEIVED ACKNOWLEDGMENT OF END-OF-FILE

        if(!__usb_fileid) {
            result = 0;

            break;
        }       // COMMUNICATION WAS ABORTED

        if(prevoffset != __usb_offset)
            start = tmr_ticks();        // MEASURE TIMEOUT SINCE LAST TIME WE SENT A PACKET
        prevoffset = __usb_offset;
        end = tmr_ticks();
        if(tmr_ticks2ms(start, end) > USB_TIMEOUT_MS) {

            result = 0;
            break;
        }       // FAIL IF TIMEOUT

        cpu_waitforinterrupt();

    }
    while(1);

    __usb_fileid = 0;   // CLOSE THE FILE

    return result;
}

// START RECEIVING A FILE, WHETHER IT WAS COMPLETELY RECEIVED YET OR NOT
// RETURNS THE FILEID OR 0 IF FAILS
int usb_rxfileopen()
{
    if(!usb_hasdata())
        return 0;
    return (int)__usb_fileid;
}

// RETURN HOW MANY BYTES ARE READY TO BE READ
int usb_rxbytesready(int fileid)
{
    if(fileid != (int)__usb_fileid)
        return 0;
    int bytesready = __usb_rxtxtop - __usb_rxtxbottom;
    if(bytesready < 0)
        bytesready += RING_BUFFER_SIZE;
    return bytesready;
}

// RETRIEVE BYTES THAT WERE ALREADY RECEIVED
int usb_fileread(int fileid, BYTEPTR dest, int nbytes)
{
    if(fileid != (int)__usb_fileid)
        return 0;

    if(nbytes <= 0)
        return 0;

    // WAIT FOR ENOUGH BYTES TO BECOME AVAILABLE
    int bytescopied = 0;

    do {

        int available = usb_waitfordata(nbytes);

        if(!available)
            return bytescopied;

        if(available >= nbytes)
            available = nbytes;

        // QUICK COPY IF WE ALREADY HAVE ENOUGH BYTES

        if(__usb_rxtxbottom + available > RING_BUFFER_SIZE) {
            // SPLIT THE COPY IN TWO OPERATIONS
            memmoveb(dest, __usb_rxtxbuffer + __usb_rxtxbottom,
                    RING_BUFFER_SIZE - __usb_rxtxbottom);
            memmoveb(dest + (RING_BUFFER_SIZE - __usb_rxtxbottom),
                    __usb_rxtxbuffer,
                    available - (RING_BUFFER_SIZE - __usb_rxtxbottom));
        }
        else {
            memmoveb(dest, __usb_rxtxbuffer + __usb_rxtxbottom, available);
        }

        __usb_rxtxbottom += available;
        __usb_rxoffset += available;
        if(__usb_rxtxbottom >= RING_BUFFER_SIZE)
            __usb_rxtxbottom -= RING_BUFFER_SIZE;
        dest += available;
        nbytes -= available;
        bytescopied += available;

        if(__usb_rxtotalbytes && (__usb_rxoffset >= __usb_rxtotalbytes)) {
            usb_mutex_lock();
            __usb_drvstatus |= USB_STATUS_EOF;
            usb_mutex_unlock();
            nbytes = 0;
        }

        // SEE IF COMMS WERE HALTED
        if(__usb_drvstatus & USB_STATUS_HALT) {
            // WE EMPTIED THE BUFFERS, RELEASE THE HALT THEN WAIT SOME MORE
            int usedspace = __usb_rxtxtop - __usb_rxtxbottom;
            if(usedspace < 0)
                usedspace += RING_BUFFER_SIZE;

            // RELEASE THE HALT IF BUFFERS ARE LESS THAN QUARTER FULL
            if(usedspace <= RING_BUFFER_SIZE / 4) {
                usb_mutex_lock();
                __usb_drvstatus &= ~USB_STATUS_HALT;
                usb_mutex_unlock();
                if(!(__usb_drvstatus & USB_STATUS_ERROR))
                    usb_sendcontrolpacket(P_TYPE_REPORT);       // NOTIFY WE LIFTED THE HALT ONLY IF THERE WERE NO ERRORS, OTHERWISE LET THE DRIVER FIX THE ERROR FIRST
            }
        }

    }
    while(nbytes > 0);

    return bytescopied;

}

int usb_eof(int fileid)
{
    if(fileid != __usb_fileid)
        return 0;

    return (__usb_drvstatus & USB_STATUS_EOF) ? 1 : 0;
}

// CLOSE THE FILE RELEASE ANY PENDING DATA
int usb_rxfileclose(int fileid)
{
    if(fileid != __usb_fileid)
        return 0;

    if(!__usb_rxtotalbytes) {
        // IF WE STILL DIDN'T RECEIVE THE ENTIRE FILE, ABORT IT
        usb_sendcontrolpacket(P_TYPE_ABORT);
    }

    tmr_t start, end;
    // WAIT FOR ANY CONTROL PACKETS TO BE SENT
    start = tmr_ticks();
    while(__usb_drvstatus & USB_STATUS_TXCTL) {

        if((__usb_drvstatus & (USB_STATUS_CONFIGURED | USB_STATUS_INIT |
                        USB_STATUS_CONNECTED)) !=
                (USB_STATUS_CONFIGURED | USB_STATUS_INIT |
                    USB_STATUS_CONNECTED))
            return 0;

        cpu_waitforinterrupt();
        end = tmr_ticks();
        if(tmr_ticks2ms(start, end) > USB_TIMEOUT_MS) {
            break;
        }
    }

    // AND PUT THE DRIVER TO IDLE
    __usb_fileid_seq = __usb_fileid & 0xff;
    __usb_fileid = 0;
    __usb_rxtxtop = 0;
    __usb_rxtxbottom = 0;
    __usb_rxoffset = 0;
    __usb_rxtotalbytes = 0;
    usb_mutex_lock();
    __usb_drvstatus &=
            ~(USB_STATUS_EOF | USB_STATUS_HALT | USB_STATUS_ERROR |
            USB_STATUS_RXDATA);
    usb_mutex_unlock();
    return 1;
}
