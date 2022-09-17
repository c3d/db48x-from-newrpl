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
    throw_dbgexception("Calculator is about to reset",EX_RESET | EX_NOREG);
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
void ram_receiveandflashfw(int32_t flashsize)
{
    int pass = 1, fileid;
    word_p flash_address;
    WORD flash_nwords, data;
    WORD receivedwords;
    FS_FILE *newrplrom;
    int err;

    // Going the easy way storing the firmware image in ram at a fixed start address
    // between this code and stack. From here on the machine gets reset anyway.
    unsigned char * const ram_fw_start = (unsigned char *)RAM_BASE_PHYSICAL + 0x1000000;
    unsigned char *ram_fw_pointer = ram_fw_start;
    int fw_size = 0;

    gglsurface scr;
    ggl_initscr(&scr);

    ggl_hline(&scr,10,0,LCD_W-1,ggl_solid(PAL_GRAY15));
    ggl_hline(&scr,21,0,LCD_W-1,ggl_solid(PAL_GRAY15));
    ggl_rect(&scr,0,11,LCD_W-1,20,ggl_solid(PAL_GRAY0));

    do {

        // SLEEP UNTIL WE GET SOMETHING
        while(!usb_hasdata())
            cpu_waitforinterrupt();

        fileid = usb_rxfileopen();
        if (!fileid) {
            continue;
        }

        if(usb_filetype(fileid) != 'W') {
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
                throw_dbgexception("USB Comms timed out",EX_RESET | EX_CONT | EX_NOREG);
                continue;
            } else break;
        } while(!receivedwords);

        if(usb_fileread(fileid, (byte_p) & data, 4) < 4) {
            throw_dbgexception("Read less than 4 bytes-signature",EX_RESET);
            ram_doreset();      // NOTHING ELSE TO DO
        }

        if(data != TEXT2WORD('F', 'W', 'U', 'P')) {
            throw_dbgexception("Bad FWUP signature",EX_RESET);
            ram_doreset();      // NOTHING ELSE TO DO
        }

        if(usb_fileread(fileid, (byte_p) & flash_address, 4) < 4) {
            throw_dbgexception("Read less than 4 bytes-address",EX_RESET);
            ram_doreset();      // NOTHING ELSE TO DO
        }

        if(usb_fileread(fileid, (byte_p) & flash_nwords, 4) < 4) {
            throw_dbgexception("Read less than 4 bytes-nwords",EX_RESET);
            ram_doreset();
        }

        if(flash_nwords + 3 != receivedwords) {
            throw_dbgexception("Words received mismatch",EX_RESET);
            ram_doreset();
        }

        if((((WORD) flash_address) == 0xffffffff)) {

            // FINISH RECEIVING THE COMMAND
            usb_rxfileclose(fileid);

            // Write the whole file at once
            err = FSOpen("NEWRPL.ROM", FSMODE_WRITE, &newrplrom);
            if (err != FS_OK) {
                throw_dbgexception("Cannot open file in flash",EX_RESET | EX_CONT | EX_NOREG);
                return;
            }

            err=FSWrite(ram_fw_start, fw_size, newrplrom);
            if(err!=fw_size) {
                FSClose(newrplrom);
                throw_dbgexception("Write to flash failed",EX_RESET);
                ram_doreset();
            }

            // CLOSE THE FILE
            err = FSClose(newrplrom);
            if (err != FS_OK) {
                throw_dbgexception("Cannot close file in flash",EX_RESET | EX_CONT | EX_NOREG);
                return;
            }

            // UNMOUNT THE VOLUME AND PROPERLY CLOSE THE FILE SYSTEM
            // ENSURE ALL BUFFERS WERE WRITTEN TO FLASH
            FSShutdown();

            // SHOW SOME VISUALS
            int k;
            for(k = 0; k < flashsize - 0x4000; k += 0x1000) {

                int pixel = (k) >> 14;
                ggl_vline(&scr,pixel,11,20,ggl_solid(PAL_GRAY0));

            }

            for(; k >= 0; k -= 256) {
                int pixel = (k) >> 14;

                ggl_vline(&scr,pixel,11,20,ggl_solid(PAL_GRAY6));
            }

            for(k = 0; k < flashsize - 0x4000; k += 256) {
                int pixel = (k) >> 14;

                ggl_vline(&scr,pixel,11,20,ggl_solid(PAL_GRAY15));
            }

            ram_doreset();      // HOST REQUESTED A RESET
        }

        // Accumulating the firmware in RAM
        const unsigned char * dataptr = (unsigned char *) (usb_rxtxbuffer + usb_rxtxbottom);      // THIS POINTS TO THE NEXT BYTE TO READ
        int flash_nbytes = flash_nwords * sizeof(WORD);
        memcpyb(ram_fw_pointer, dataptr, flash_nbytes);
        ram_fw_pointer += flash_nbytes;
        fw_size += flash_nbytes;

        usb_rxfileclose(fileid);

        // SHOW SOME VISUAL FEEDBACK
        {

            int pixel = (((WORD) flash_address) - 0x4000) >> 14;

            ggl_vline(&scr,pixel,11,20,ggl_solid(PAL_GRAY15));

        }
    }
    while(1);
    // NEVER RETURNS
}


ARM_MODE void ram_startfwupdate()
{
    // AT THIS POINT, A USB CONNECTION HAS ALREADY BEEN ESTABLISHED
    // THIS ROUTINE WILL UPDATE THE FIRMWARE

    ram_receiveandflashfw(2048*1024);

    // THIS CAN NEVER RETURN, THERE WILL BE NO NEWRPL.ROM HERE

    while(1);
}
