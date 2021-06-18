/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include <libraries.h>
#include <ui.h>
#include <fsystem.h>

void ram_doreset()
{
    throw_dbgexception("Calculator is about to reset",__EX_RESET | __EX_NOREG);

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
void ram_receiveandflashfw(BINT flashsize)
{
    int pass = 1, result, fileid;
    WORDPTR flash_address;
    WORD flash_nwords, data;
    WORD receivedwords;
    FS_FILE *newrplrom;
    int err;

    DRAWSURFACE scr;
    ggl_initscr(&scr);


    err = FSOpen("NEWRPL.ROM", FSMODE_WRITE, &newrplrom);
    if (err != FS_OK) {
        throw_dbgexception("Cannot open file in flash",__EX_RESET | __EX_CONT | __EX_NOREG);
        return;
    }



    ggl_hline(&scr,10,0,SCREEN_WIDTH-1,0xffffffff);
    ggl_hline(&scr,21,0,SCREEN_WIDTH-1,0xffffffff);
    ggl_rect(&scr,0,11,SCREEN_WIDTH-1,20,0);

    do {

        // SLEEP UNTIL WE GET SOMETHING
        while(!usb_hasdata())
            cpu_waitforinterrupt();

        result = fileid = usb_rxfileopen();
        if((!result) || usb_filetype(fileid) != 'W') {
            usb_rxfileclose(fileid);
            continue;
        }

        data = 0xffffffff;

// RECEIVE THE ENTIRE FILE, GET THE TOTAL NUMBER OF BYTES RECEIVED
// THIS WAY WE AVOID IRQS DURING FLASHING
        do {
        receivedwords = (usb_waitfordata(6000) + 3) >> 2;

        if(!receivedwords) {
            // COMMUNICATIONS MUST'VE TIMED OUT OR WE ARE HALTED
            throw_dbgexception("USB Comms timed out",__EX_RESET | __EX_CONT | __EX_NOREG);
            continue;
        } else break;
        } while(!receivedwords);

        if(usb_fileread(fileid, (BYTEPTR) & data, 4) < 4) {
            FSClose(newrplrom);
            throw_dbgexception("Read less than 4 bytes-signature",__EX_RESET);
            ram_doreset();      // NOTHING ELSE TO DO
        }

        if(data != TEXT2WORD('F', 'W', 'U', 'P')) {
            FSClose(newrplrom);
            throw_dbgexception("Bad FWUP signature",__EX_RESET);
            ram_doreset();      // NOTHING ELSE TO DO
        }

        if(usb_fileread(fileid, (BYTEPTR) & flash_address, 4) < 4) {
            FSClose(newrplrom);
            throw_dbgexception("Read less than 4 bytes-address",__EX_RESET);
            ram_doreset();      // NOTHING ELSE TO DO
        }

        if(usb_fileread(fileid, (BYTEPTR) & flash_nwords, 4) < 4) {
            FSClose(newrplrom);
            throw_dbgexception("Read less than 4 bytes-nwords",__EX_RESET);
            ram_doreset();
        }

        if(flash_nwords + 3 != receivedwords) {
            FSClose(newrplrom);
            throw_dbgexception("Words received mismatch",__EX_RESET);
            ram_doreset();
        }

        if((((WORD) flash_address) == 0xffffffff)) {

            // FINISH RECEIVING THE COMMAND
            usb_rxfileclose(fileid);

            // CLOSE THE FILE
            err = FSClose(newrplrom);
            if (err != FS_OK) {
                throw_dbgexception("Cannot close file in flash",__EX_RESET | __EX_CONT | __EX_NOREG);
                return;
            }

            // UNMOUNT THE VOLUME AND PROPERLY CLOSE THE FILE SYSTEM
            // ENSURE ALL BUFFERS WERE WRITTEN TO FLASH
            FSShutdown();


            // SHOW SOME VISUALS
            int k;
            for(k = 0; k < flashsize - 0x4000; k += 0x1000) {

                int pixel = (k) >> 14;
                ggl_vline(&scr,pixel,11,20,0);

            }

            for(; k >= 0; k -= 256) {
                int pixel = (k) >> 14;

                ggl_vline(&scr,pixel,11,20,0x66666666);
            }

            for(k = 0; k < flashsize - 0x4000; k += 256) {
                int pixel = (k) >> 14;

                ggl_vline(&scr,pixel,11,20,0xffffffff);
            }

            ram_doreset();      // HOST REQUESTED A RESET
        }

        if((WORD) (flash_address + flash_nwords) > flashsize) {
            FSClose(newrplrom);

            throw_dbgexception("Attempt to write beyond end of flash",__EX_RESET);

            ram_doreset();
        }

        if(((WORD) flash_address < 0x4000)) {
            FSClose(newrplrom);

            throw_dbgexception("Attempt to overwrite bootloader",__EX_RESET);

            ram_doreset();      // PROTECT THE BOOTLOADER AT ALL COSTS
        }


        unsigned char * dataptr = (unsigned char *) (__usb_rxtxbuffer + __usb_rxtxbottom);      // THIS POINTS TO THE NEXT BYTE TO READ

        err=FSWrite(dataptr,flash_nwords*sizeof(WORD),newrplrom);
        if(err!=flash_nwords*sizeof(WORD)) {
            FSClose(newrplrom);
            throw_dbgexception("Write to flash failed",__EX_RESET);
            ram_doreset();      // PROTECT THE BOOTLOADER AT ALL COSTS
        }

// WE FINISHED PROGRAMMING THE FLASH!

        usb_rxfileclose(fileid);

// SHOW SOME VISUAL FEEDBACK
        {

            int pixel = (((WORD) flash_address) - 0x4000) >> 14;

            ggl_vline(&scr,pixel,11,20,0xffffffff);

        }
    }
    while(1);
//ram_doreset();

// NEVER RETURNS
}


__ARM_MODE__ void ram_startfwupdate()
{
    // AT THIS POINT, A USB CONNECTION HAS ALREADY BEEN ESTABLISHED
    // THIS ROUTINE WILL UPDATE THE FIRMWARE

    ram_receiveandflashfw(2048*1024);

    // THIS CAN NEVER RETURN, THERE WILL BE NO NEWRPL.ROM HERE

    while(1);
}

