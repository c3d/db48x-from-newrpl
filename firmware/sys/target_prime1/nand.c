/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "nand.h"
#include <newrpl.h>

#define NAND_ROW_SIZE 2048
#define NAND_ROW_BITS 11
#define NAND_COLUMN_MASK 0x7ff
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

// Reads up to @num_bytes from NAND @read_address and writes to ram @target_address
// All three parameters support arbitrary byte alignment 
// Returns number of read bytes
static unsigned int NANDReadPacket(uint32_t read_address, unsigned int num_bytes, uint8_t *target_address)
{
    NANDEnableChipSelect();

    NANDClearReady();

    *NFCMMD = NAND_CMD_READ1st;

    // Page size is 2048 bytes, read in max 512 byte packets
    // NAND expects 5 address cycles
    unsigned int row = read_address >> NAND_ROW_BITS;
    unsigned int column = read_address & NAND_COLUMN_MASK;
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

    // Reading the incoming data bytewise
    volatile uint8_t *NFDATA8 = (volatile uint8_t *)NFDATA;

    unsigned int bytes_to_read = NAND_PACKET_SIZE;    
    if (num_bytes < bytes_to_read) {
        bytes_to_read = num_bytes;
    }
    unsigned int const row_end = (read_address & ~NAND_COLUMN_MASK) + NAND_ROW_SIZE;
    unsigned int const bytes_to_row_end = row_end - read_address;
    if (bytes_to_row_end < bytes_to_read) {
        bytes_to_read = bytes_to_row_end;
    }

    for (int i = 0; i < bytes_to_read; ++i) {
        *target_address = *NFDATA8;
        ++target_address;
    }
    
    NANDDisableChipSelect();

    return bytes_to_read;
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
