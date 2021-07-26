/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

// LOW-LEVEL READ FROM FILE'S CURRENT POSITION

int FSReadLL(unsigned char *buffer, int nbytes, FS_FILE * file, FS_VOLUME * fs)
{
    int bytesread, totalcount, bytescachedl, bytescachedr;
    uint64_t currentaddr;
    FS_FRAGMENT *fr;

// RETURN IF EOF
    if(file->CurrentOffset >= file->FileSize) {
        return 0;
    }
// TRUNCATE IF TRYING TO READ BEYOND EOF

// INITIALIZE READ BUFFER IF NEEDED
    if(!file->RdBuffer.Data) {
// MALLOC ONE SECTOR READ BUFFER
        file->RdBuffer.Data = simpmallocb(512);
        if(!file->RdBuffer.Data)
            return 0;   // NOT ENOUGH MEMORY FOR READ BUFFER, FAIL TO READ
        file->RdBuffer.Used = 0;
    }

    if(file->CurrentOffset + nbytes > file->FileSize)
        nbytes = file->FileSize - file->CurrentOffset;

// VERIFY IF DATA IS ALREADY IN BUFFER
    bytescachedl = 0;
    bytescachedr = 0;

    if(file->RdBuffer.Used) {
        int offsetdiff = file->CurrentOffset - file->RdBuffer.Offset;

        if(offsetdiff < 0)      // CURRENTOFFSET TO THE LEFT
        {

            if(offsetdiff + nbytes > 0) // THERE IS OVERLAP WITH READ BUFFER!
            {
                if(offsetdiff + nbytes <= 512)  // END-OF-READ IS WITHIN BUFFER
                {

                    bytescachedr = offsetdiff + nbytes;
                    //printf("End hit=%d\n",bytescachedr);
                    memmoveb(buffer - offsetdiff, file->RdBuffer.Data,
                            bytescachedr);
                    nbytes -= bytescachedr;
                }
                else    // READ BUFFER IS COMPLETELY WITHIN READ SECTION
                {
                    // IGNORE THIS CASE, DON'T SPLIT THE READ OPERATION IN HALF

                }
            }
        }
        else    // CURENTOFFSET TO THE RIGHT
        {
            if(offsetdiff < 512)        // READ SECTION START WITHIN BLOCK
            {
                if(offsetdiff + nbytes <= 512)  // READ SECTION COMPLETELY WITHIN READ BUFFER
                {
                    //printf("total hit\n");
                    memmoveb(buffer, file->RdBuffer.Data + offsetdiff, nbytes);
                    file->CurrentOffset += nbytes;
                    return nbytes;
                }
                else    // READ SECTION BEGIN IN READ BUFFER
                {
                    bytescachedl = 512 - offsetdiff;
                    //printf("start hit=%d\n",bytescachedl);
                    memmoveb(buffer, file->RdBuffer.Data + offsetdiff,
                            bytescachedl);
                    file->CurrentOffset += bytescachedl;
                    nbytes -= bytescachedl;
                    buffer += bytescachedl;
                }
            }

        }

    }

    fr = &file->Chain;
    currentaddr = (((uint64_t) fr->StartAddr) << 9) + file->CurrentOffset;

// FIND STARTING ADDRESS
    while(currentaddr >= (((uint64_t) fr->EndAddr) << 9)) {

        currentaddr -= ((uint64_t) fr->EndAddr) << 9;

        fr = fr->NextFragment;
        if(fr == NULL) {
            // MALFORMED CLUSTER CHAIN!!! CLUSTER CHAIN IS SHORTER THAN FileSize
            return 0;
        }
        currentaddr += ((uint64_t) fr->StartAddr) << 9;

    }

// START READING FULL FRAGMENTS
    totalcount = bytescachedl;

    /*
       // SET BLOCK LENGTH TO OPTIMIZE READ SPEED
       temp=nbytes;
       temp2=0;
       while(temp) { temp>>=1; ++temp2; }
       temp2-=2;                        // USE BLOCK LEN=(nbytes/4) aprox.
       if(temp2<5) temp2=5;                                     // MINIMUM BLOCK LEN=32 BYTES;

       if(!SDDSetBlockLen(fs->Disk,temp2)) {
       // ERROR
       return totalcount;
       }
     */

    while(nbytes + currentaddr > (((uint64_t) fr->EndAddr) << 9)) {

        bytesread =
                SDDRead(currentaddr,
                (((uint64_t) fr->EndAddr) << 9) - currentaddr, buffer,
                fs->Disk);
        totalcount += bytesread;
        file->CurrentOffset += bytesread;
        if((uint64_t) bytesread !=
                (((uint64_t) fr->EndAddr) << 9) - currentaddr) {
// ERROR READING LAST BLOCK, RETURN WHAT WAS READ SO FAR
            return totalcount;
        }
        nbytes -= bytesread;
        fr = fr->NextFragment;
        if(fr == NULL) {
            // MALFORMED CLUSTER CHAIN!!! CLUSTER CHAIN IS SHORTER THAN FileSize
            return totalcount;
        }
        currentaddr = ((uint64_t) fr->StartAddr) << 9;
        buffer += bytesread;
    }

    if(nbytes) {
        int readnow = ((currentaddr + nbytes) & (~511)) - currentaddr;

        if(readnow > 0) {
// READ SECTORS DIRECTLY INTO BUFFER
            bytesread = SDDRead(currentaddr, readnow, buffer, fs->Disk);

            totalcount += bytesread;
            file->CurrentOffset += bytesread;
            currentaddr += bytesread;
            buffer += bytesread;
            nbytes -= bytesread;
            if(bytesread != readnow)
                return totalcount;
            readnow = 0;
        }

// READ LAST SECTOR IN CACHE
        currentaddr += readnow;
        if((!file->RdBuffer.Used) || (file->CurrentOffset + readnow != (unsigned int)file->RdBuffer.Offset))    // CHECK IF SECTOR ALREADY IN CACHE
        {

            if(file->CurrentOffset < file->FileSize)    // CHECK IF ANY MORE BYTES TO CACHE
            {

// UPDATE FRAGMENT AS NEEDED
                while(currentaddr >= (((uint64_t) fr->EndAddr) << 9)) {

                    currentaddr -= (((uint64_t) fr->EndAddr) << 9);

                    fr = fr->NextFragment;
                    if(fr == NULL) {
                        // CURRENTADDR POINTING EXACTLY AT END-OF-FILE, NOTHING TO READ
                        return 0;
                    }
                    currentaddr += (((uint64_t) fr->StartAddr) << 9);

                }
                bytesread =
                        SDDRead(currentaddr, 512, file->RdBuffer.Data,
                        fs->Disk);

                if(bytesread != 512) {
                    file->RdBuffer.Used = 0;
                    return totalcount;
                }

                file->RdBuffer.Offset = file->CurrentOffset + readnow;
                file->RdBuffer.Used = 1;

            }
        }

        memmoveb(buffer, file->RdBuffer.Data - readnow, nbytes);

        totalcount += nbytes;
        file->CurrentOffset += nbytes;

    }

    totalcount += bytescachedr;
    file->CurrentOffset += bytescachedr;

    return totalcount;

}
#endif
