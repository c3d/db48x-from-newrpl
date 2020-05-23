/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "nand.h"
#include <newrpl.h>

#define NAND_BLOCK_SIZE 0x20000
#define NAND_BLOCK_BITS 17
#define NAND_BLOCK_MASK 0xfffe0000
#define NAND_PAGE_SIZE 0x800
#define NAND_PAGE_BITS 11
#define NAND_PAGE_MASK 0xfffff800
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

// Reads @num_bytes from @row and @column and writes to ram @target_address.
// Is able to read spare field. No boundary checks are made.
static void NANDReadUnjustifiedFull(unsigned int row, unsigned int column, unsigned int num_bytes, uint8_t *target_address)
{
    NANDEnableChipSelect();

    NANDClearReady();

    *NFCMMD = NAND_CMD_READ1st;

    // NAND expects 5 address cycles
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

    for (int i = 0; i < num_bytes; ++i) {
        *target_address = *NFDATA8;
        ++target_address;
    }
    
    NANDDisableChipSelect();
}

// Reads up to @num_bytes from NAND @read_address and writes to ram @target_address.
// Number of read bytes is limited by page boundary and packet size.
// All three parameters support arbitrary byte alignment.
// Returns number of read bytes.
static unsigned int NANDReadUnjustifiedPart(uint32_t read_address, unsigned int num_bytes, uint8_t *target_address)
{
    // Page size is 2048 bytes, read in max 512 byte packets
    unsigned int row = read_address >> NAND_PAGE_BITS;
    unsigned int column = read_address & ~NAND_PAGE_MASK;

    unsigned int bytes_to_read = NAND_PACKET_SIZE;    
    if (num_bytes < bytes_to_read) {
        bytes_to_read = num_bytes;
    }
    unsigned int const row_end = (read_address & NAND_PAGE_MASK) + NAND_PAGE_SIZE;
    unsigned int const bytes_to_row_end = row_end - read_address;
    if (bytes_to_row_end < bytes_to_read) {
        bytes_to_read = bytes_to_row_end;
    }

    NANDReadUnjustifiedFull(row, column, bytes_to_read, target_address);
    return bytes_to_read;
}

// Reads first byte of spare field and checks it against validity conditions
// taken from BXCBOOT0.BIN from 20140331 firmware.
// Returns 1 if page @address lies in is valid, else 0.
static int NANDIsPageValid(uint32_t address)
{
    uint8_t byte;
    unsigned int row = address >> NAND_PAGE_BITS;
    unsigned int column = 1 << NAND_PAGE_BITS;
    NANDReadUnjustifiedFull(row, column, 1, &byte);
    return (byte == 0x00 || byte == 0xf0) ? 0 : 1;
}

// Checks first, second and last page in block.
// Returns 1 if block @address lies in is valid, else 0.
int NANDIsBlockValid(uint32_t address) // FIXME static after test
{
    unsigned int page_0 = address & NAND_BLOCK_MASK;
    if (!NANDIsPageValid(page_0)) {
        return 0;
    }

    unsigned int page_1 = page_0 + NAND_PAGE_SIZE;
    if (!NANDIsPageValid(page_1)) {
        return 0;
    }

    unsigned int page_last = page_0 + NAND_BLOCK_SIZE - NAND_PAGE_SIZE;
    if (!NANDIsPageValid(page_last)) {
        return 0;
    }

    return 1;
}

void NANDRead(uint32_t read_address, unsigned int num_bytes, uint8_t *target_address)
{
    while (1) {
        unsigned int read = NANDReadUnjustifiedPart(read_address, num_bytes, target_address);
        num_bytes -= read;
        read_address += read;
        target_address += read;

        if (num_bytes == 0)
            break;
    }
}
