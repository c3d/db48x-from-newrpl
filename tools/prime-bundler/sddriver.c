
/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

// THIS IS A STUB DRIVER TO ALLOW COMPILATION ON A PC
// COMMANDS WILL ACT AS IF THERE WAS NO CARD INSERTED

#include "fsystem/fsyspriv.h"


#define UNUSED_ARGUMENT(a) (void)(a)
// SD MODULE

// GLOBAL VARIABLES     FOR SD CARD EMULATION

#include <stdio.h>
#include <string.h>

volatile int sd_inserted;
volatile int sd_nsectors;     // TOTAL SIZE OF SD CARD IN 512-BYTE SECTORS
volatile int sd_RCA;
volatile unsigned char *sd_buffer;    // BUFFER WITH THE ENTIRE CONTENTS OF THE SD CARD

int SDCardInserted()
{
    return sd_inserted;
}

int SDCardWriteProtected()
{
    return 0;
}

// FULLY INITIALIZE THE SDCARD INTERFACE
// RETURNS TRUE IF THERE IS A CARD
// FALSE IF THERE'S NO CARD

int SDInit(SD_CARD * card)
{
    UNUSED_ARGUMENT(card);
    if(sd_inserted)
        return TRUE;
    return FALSE;
}

int SDIOSetup(SD_CARD * card, int shutdown)
{
    UNUSED_ARGUMENT(card);
    if(!shutdown)
        return TRUE;
    return FALSE;
}

void SDPowerDown()
{
}

void SDPowerUp()
{
}

int SDSelect(int RCA)
{
    if(RCA == sd_RCA)
        return TRUE;
    return 0;
}

// READS WORDS DIRECTLY INTO BUFFER
// AT THE CURRENT BLOCK LENGTH
// CARD MUST BE SELECTED
int SDDRead(uint64_t SDAddr, int NumBytes, unsigned char *buffer,
        SD_CARD * card)
{
    UNUSED_ARGUMENT(card);
    UNUSED_ARGUMENT(SDAddr);
    UNUSED_ARGUMENT(NumBytes);
    UNUSED_ARGUMENT(buffer);
    if(sd_inserted && sd_RCA) {
        // NO ARGUMENT CHECKS!
        memmove(buffer, (unsigned char *)sd_buffer + SDAddr, NumBytes);
        return NumBytes;
    }
    return FALSE;
}

int SDCardInit(SD_CARD * card)
{
    UNUSED_ARGUMENT(card);
    if(sd_inserted) {
        card->SysFlags = 31;    // 1=SDIO interface setup, 2=SDCard initialized, 4=Valid RCA obtained, 8=Bus configured OK, 16=SDHC
        card->Rca = sd_RCA = 0x10;
        card->BusWidth = 4;
        card->MaxBlockLen = 9;
        card->WriteBlockLen = 9;
        card->CurrentBLen = 9;
        card->CardSize = sd_nsectors;
        card->CID[0] = 0;
        card->CID[1] = 0;
        card->CID[2] = 0;
        card->CID[3] = 0;
        return TRUE;
    }

    return FALSE;
}

// WRITE BYTES AT SPECIFIC ADDRESS
// AT THE CURRENT BLOCK LENGTH
// CARD MUST BE SELECTED
int SDDWrite(uint64_t SDAddr, int NumBytes, unsigned char *buffer,
        SD_CARD * card)
{
    UNUSED_ARGUMENT(card);
    UNUSED_ARGUMENT(SDAddr);
    UNUSED_ARGUMENT(NumBytes);
    UNUSED_ARGUMENT(buffer);

    // DEBUG ONLY
    if(SDAddr > ((uint64_t) sd_nsectors << 9))
        return 0;

    if(SDAddr + NumBytes > ((uint64_t) sd_nsectors << 9))
        return 0;

    if(sd_inserted && sd_RCA) {
        // NO ARGUMENT CHECKS!
        memmoveb((unsigned char *)sd_buffer + SDAddr, buffer, NumBytes);
        return NumBytes;
    }

    return FALSE;

}