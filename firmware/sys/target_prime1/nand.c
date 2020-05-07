/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "nand.h"
#include <newrpl.h>

#define NAND_PAGE_SIZE 2048
#define NAND_PACKET_SIZE 512

void NANDWriteProtect(void)
{
    *NFEBLK = *NFSBLK;
    *NFCONT |= 0x00020000;
}

static void NANDDisableChipSelect(void)
{
    *NFCONT |= 0x00000002;
}

static void NANDEnableChipSelect(void)
{
    *NFCONT &= ~0x00000002;
}

static void NANDWaitReady(void)
{
    while ((*NFSTAT & 0x00000010) == 0);
}

static void NANDClearReady(void)
{
    *NFSTAT |= 0x00000010;
}

// Reads up to 512 @num_bytes from NAND @read_address and writes to ram @target_address
// All three parameters support arbitrary byte alignment 
// Returns number of read bytes
static unsigned int NANDReadPacket(uint32_t read_address, unsigned int num_bytes, uint8_t *target_address)
{
    NANDEnableChipSelect();

    NANDClearReady();

    *NFCMMD = NAND_CMD_READ1st;

    // Page size is 2048 bytes, read in max 512 byte packets
    // NAND expects 5 address cycles
    unsigned int row = read_address >> 11;
    unsigned int column = read_address & 0x7ff;
    for (int i = 0; i < 2; ++i) {
        *NFADDR = column & 0xff;
        column >>= 8;
    }
    for (int i = 0; i < 3; ++i) {
        *NFADDR = row & 0xff;
        row >>= 8;
    }

    *NFCMMD = NAND_CMD_READ2nd;

    NANDWaitReady();

    // Reading the incomming data bytewise
    volatile uint8_t *NFDATA8 = (volatile uint8_t *)NFDATA;
    unsigned int count = 0;

    for (int i = 0; i < NAND_PACKET_SIZE; ++i) {
        *target_address = *NFDATA8;
        ++target_address;

        ++count;
        if (count == num_bytes)
            break;
    }
    
    NANDDisableChipSelect();

    return count;
}

void NANDRead(uint32_t read_address, unsigned int num_bytes, uint8_t *target_address)
{
    while (1) {
        unsigned int read = NANDReadPacket(read_address, num_bytes, target_address);
        num_bytes -= read;
        read_address += read;
        target_address += read;

        if (num_bytes == 0)
            break;
    }
}
