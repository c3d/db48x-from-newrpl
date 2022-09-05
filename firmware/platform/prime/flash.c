/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "hal_api.h"
#include <target.h>
#include <newrpl.h>
#include <stdint.h>
#include <stddef.h>
#include "nand.h"

#define NFCONF_MsgLength       0x02000000
#define NFCONF_ECCTypeMask     0x01800000
#define NFCONF_ECCType4        0x01000000

#define NFCONT_Reg_nCE0        0x00000002
#define NFCONT_InitSECC        0x00000010
#define NFCONT_InitMECC        0x00000020
#define NFCONT_MainECCLock     0x00000080
#define NFCONT_LockTight       0x00020000
#define NFCONT_ECCDirection    0x00040000

#define NFSTAT_RnB_TransDetect 0x00000010
#define NFSTAT_IllegalAccess   0x00000020
#define NFSTAT_ECCDecDone      0x00000040

#define NFECCERR0_ECCReady     0x40000000

#define NAND_STATUS_REGISTER_FAIL         0x01
#define NAND_STATUS_REGISTER_WRITEPROTECT 0x80
#define NAND_STATUS_REGISTER_READY        0x40

#define NAND_CMD_READ1st         0x00
#define NAND_CMD_READ2nd         0x30
#define NAND_CMD_RND_OUT1st      0x05
#define NAND_CMD_RND_OUT2nd      0xe0
#define NAND_CMD_PAGE_PROGRAM1st 0x80
#define NAND_CMD_PAGE_PROGRAM2nd 0x10
#define NAND_CMD_READ_STATUS     0x70
#define NAND_CMD_BLOCK_ERASE1st  0x60
#define NAND_CMD_BLOCK_ERASE2nd  0xd0
#define NAND_CMD_CHIP_RESET      0xff

// Some registers are accessed with other data width than 32bit

// Data is read bytewise
#define NFDATA_8 (volatile uint8_t *)NFDATA

// error location is halfword
#define NFECCERR0_16 (volatile uint16_t *)NFECCERR0

// error bit pattern is byte
#define NFMLCBITPT_8 (volatile uint8_t *)NFMLCBITPT

typedef struct {
    uint8_t bad;            // 0xff = good, 0x00 = bad
    uint8_t other;          // 0xfe
    uint8_t unused[6];      // 0xff
    uint32_t parity_low;    // parity bytes 1-4 from NFMECC0
    uint32_t parity_high;   // parity bytes 5-7 from NFMECC1
}  __attribute__ ((packed)) SparePacket;

typedef struct {
    SparePacket packets[4];
}  __attribute__ ((packed)) SparePage;

typedef struct {
    uint32_t unused1;
    uint32_t unused2;
    uint32_t magic; // "BFX\0"
    uint16_t size; // 0x1800
    uint16_t num_blocks; // NAND_NUM_BLOCKS
    uint16_t num_bytes; // 2*num_blocks
    uint16_t block_size; // NAND_BLOCK_SIZE
    uint16_t num_blocks2; // NAND_NUM_BLOCKS
    uint16_t good_block_count;
    uint8_t unused3[488];

    // locked blocks are
    // - first 2 good blocks containing bootloaders
    // - bad blocks
    uint16_t locked_blocks[2816];
}  __attribute__ ((packed)) BFX;

static uint16_t __SCRATCH_MEMORY__ nand_block_translation_table[NAND_NUM_BLOCKS];
static uint8_t __SCRATCH_MEMORY__ nand_buffer[NAND_BLOCK_SIZE];

void NANDWriteProtect(void)
{
    *NFEBLK = *NFSBLK;
    *NFCONT |= NFCONT_LockTight;
}

static inline void NANDDisableChipSelect(void)
{
    *NFCONT |= NFCONT_Reg_nCE0;
}

static inline void NANDEnableChipSelect(void)
{
    *NFCONT &= ~NFCONT_Reg_nCE0;
}

// Returns NAND_STATUS
static int NANDWaitReady(void)
{
    unsigned int start,end;
    // Flash datasheet lists Block Erase as the longest operation with a max. 10 ms
    // So in 12 ms it MUST have finished or there was an error.
    __tmr_setuptimeoutms(12,&start,&end);

    while (1) {
        if ((*NFSTAT & NFSTAT_RnB_TransDetect) != 0) {
            return NAND_STATUS_OK;
        }

        if (__tmr_timedout(start,end)) {
            return NAND_STATUS_TIMEOUT;
        }
    }
}

static inline void NANDClearReady(void)
{
    *NFSTAT |= NFSTAT_RnB_TransDetect;
}

static inline void NANDClearECCDecoderDone(void)
{
    *NFSTAT |= NFSTAT_ECCDecDone;
}

static inline void NANDActivateECCDecoder(void)
{
    *NFCONT &= ~NFCONT_ECCDirection;
}

static inline void NANDActivateECCEncoder(void)
{
    *NFCONT |= NFCONT_ECCDirection;
}

static inline void NANDUnlockMainAreaECC(void)
{
    *NFCONT &= ~NFCONT_MainECCLock;
}

static inline void NANDLockMainAreaECC(void)
{
    *NFCONT |= NFCONT_MainECCLock;
}

static void NANDSetRowAddress(unsigned int row)
{
    for (int i = 0; i < 3; ++i) {
        *NFADDR = row & 0xff;
        row >>= 8;
    }
}

static void NANDSetAddress(unsigned int row, unsigned int column)
{
    // NAND expects 5 address cycles
    for (int i = 0; i < 2; ++i) {
        *NFADDR = column & 0xff;
        column >>= 8;
    }
    NANDSetRowAddress(row);
}

static void NANDChangeReadColumn(unsigned int column)
{
    *NFCMMD = NAND_CMD_RND_OUT1st;

    *NFADDR = column & 0xff;
    *NFADDR = (column >> 8) & 0xff;

    *NFCMMD = NAND_CMD_RND_OUT2nd;
}

// Returns NAND_STATUS
static int NANDECC4Correct(uint8_t *data)
{
    int count = (*NFECCERR0 >> 26) & 0x07;

    if (count == 0) {
        return NAND_STATUS_OK;
    } else if (count > 4) {
        return NAND_STATUS_UNCORRECTABLE;
    }

    volatile uint16_t *error_location = NFECCERR0_16;
    volatile uint8_t *bit_pattern = NFMLCBITPT_8;

    while (count) {
        data[*error_location] ^= *bit_pattern;
        ++bit_pattern;
        ++error_location;
        --count;
    }

    return NAND_STATUS_OK;
}

static void NANDConfigureECC(void)
{
    // Message length 512 bytes
    *NFCONF &= ~NFCONF_MsgLength;

    // 4-bit ECC
    *NFCONF = (*NFCONF & ~NFCONF_ECCTypeMask) | NFCONF_ECCType4;

    // Initialize main and spare area ECC decoder
    // TODO why activate spare area decoding here?
    *NFCONT |= NFCONT_InitSECC | NFCONT_InitMECC;
}

static void NANDWaitForECCDecoder(void)
{
    // Wait for decoder
    while ((*NFSTAT & NFSTAT_ECCDecDone) == 0) {
        // NOTE BL1 implements a timeout counting to 0x200000 continuing on timeout
    }

    NANDClearECCDecoderDone();

    // Wait for ECC ready
    while ((*NFECCERR0 & NFECCERR0_ECCReady) == 0) {
        // NOTE BL1 implements a timeout counting to 0x200000 continuing on timeout
    }
}

int NANDReadPage(uint32_t nand_address, uint8_t *target_address)
{
    int retval = NAND_STATUS_OK;
    int result;

    NANDEnableChipSelect();

    *NFCMMD = NAND_CMD_READ1st;

    NANDSetAddress(NAND_PAGE_NUMBER(nand_address), 0);

    NANDClearReady();

    *NFCMMD = NAND_CMD_READ2nd;

    result = NANDWaitReady();
    if (result != NAND_STATUS_OK && retval == NAND_STATUS_OK) {
        retval = result;
    }

    NANDActivateECCDecoder();

    for (int packet = 0; packet < NAND_PACKETS_PER_PAGE; ++packet) {
        int column;

        NANDConfigureECC();

        NANDUnlockMainAreaECC();

        // Read packet data
        for (int i = 0; i < NAND_PACKET_SIZE; ++i) {
          *target_address = *NFDATA_8;
          ++target_address;
        }

        // Change column to stored ECC parity from spare area
        NANDChangeReadColumn(NAND_PAGE_SIZE + offsetof(SparePage, packets[packet].parity_low));

        // Read ECC parity code for packet
        for (int i = 0; i < 7; ++i) {
          *NFDATA_8;
        }

        NANDWaitForECCDecoder();

        // Try to correct errors.
        // Return existance of uncorrectable bits but continue
        result = NANDECC4Correct(target_address - NAND_PACKET_SIZE);
        if (result != NAND_STATUS_OK && retval == NAND_STATUS_OK) {
            retval = result;
        }

        // Change column to next packet
        if (packet < (NAND_PACKETS_PER_PAGE - 1)) {
          NANDChangeReadColumn(NAND_PACKET_SIZE * (packet + 1));
        }
    }

    NANDDisableChipSelect();

    return retval;
}

// Reads first byte of spare area and checks it against validity conditions
// taken from BXCBOOT0.BIN from 20140331 firmware.
// Returns NAND_STATUS_OK if page is ok, NAND_STATUS_BAD if page is bad,
// other NAND_STATUS on error.
static int NANDIsPageBad(uint32_t nand_address)
{
    int retval = NAND_STATUS_OK;
    int result;

    NANDEnableChipSelect();

    NANDClearReady();

    *NFCMMD = NAND_CMD_READ1st;

    NANDSetAddress(NAND_PAGE_NUMBER(nand_address), NAND_PAGE_SIZE);

    *NFCMMD = NAND_CMD_READ2nd;

    result = NANDWaitReady();
    if (result != NAND_STATUS_OK && retval == NAND_STATUS_OK) {
        retval = result;
    }

    uint8_t byte = *NFDATA_8;

    NANDDisableChipSelect();

    if (retval != NAND_STATUS_OK) {
        return retval;
    }
    return (byte == 0x00 || byte == 0xf0) ? NAND_STATUS_BAD : NAND_STATUS_OK;
}

// Checks first, second and last page in block. If any of these has bad marker
// whole block is bad.
int NANDIsBlockBad(uint32_t nand_address)
{
    int result;

    unsigned int page_0 = NAND_BLOCK_START(nand_address);
    result = NANDIsPageBad(page_0);
    if (result != NAND_STATUS_OK) {
        return result;
    }

    unsigned int page_1 = page_0 + NAND_PAGE_SIZE;
    result = NANDIsPageBad(page_1);
    if (result != NAND_STATUS_OK) {
        return result;
    }

    unsigned int page_last = page_0 + NAND_BLOCK_SIZE - NAND_PAGE_SIZE;
    result = NANDIsPageBad(page_last);
    if (result != NAND_STATUS_OK) {
        return result;
    }

    return NAND_STATUS_OK;
}

static void __attribute__ ((noinline)) busy_wait(unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i) {
        // asm statement with data dependency and potential side effect
        // can't be optimized away
        __asm__ volatile("" : "+g" (i) : :);
    }
}

// Returns NAND_STATUS.
static int NANDCheckWrite(void)
{
    if ((*NFSTAT & NFSTAT_IllegalAccess) != 0) {
        return NAND_STATUS_ILLEGAL_ACCESS;
    }

    *NFCMMD = NAND_CMD_READ_STATUS;

    busy_wait(3);

    for (int i = 0; i < 1024; ++i) {
        uint8_t data = *NFDATA_8;

        // Make sure we are no longer busy
        if ((data & NAND_STATUS_REGISTER_READY) == 0) {
            continue;
        }

        // Also check for write protection
        if ((data & NAND_STATUS_REGISTER_WRITEPROTECT) == 0) {
            return NAND_STATUS_WRITE_PROTECT;
        }

        // Finally, check for pass/fail
        if ((data & NAND_STATUS_REGISTER_FAIL) == 0) {
            return NAND_STATUS_OK;
        }
    }

    return NAND_STATUS_WRITE_FAIL;
}

// Sets bad marker in spare for first packet to 0x00.
// Returns first error should errors occur.
// Returns NAND_STATUS.
static int NANDMarkPageBad(uint32_t nand_address)
{
    int retval = NAND_STATUS_OK;
    int result;

    NANDEnableChipSelect();

    *NFCMMD = NAND_CMD_PAGE_PROGRAM1st;

    NANDSetAddress(NAND_PAGE_NUMBER(nand_address), NAND_PAGE_SIZE);

    *NFDATA_8 = 0;

    NANDClearReady();

    *NFCMMD = NAND_CMD_PAGE_PROGRAM2nd;

    result = NANDWaitReady();
    if (result != NAND_STATUS_OK && retval == NAND_STATUS_OK) {
        retval = result;
    }

    result = NANDCheckWrite();
    if (result != NAND_STATUS_OK && retval == NAND_STATUS_OK) {
        retval = result;
    }

    NANDDisableChipSelect();

    return retval;
}

int NANDMarkBlockBad(uint32_t nand_address)
{
    return NANDMarkPageBad(NAND_BLOCK_START(nand_address));
}

int NANDWritePage(uint32_t nand_address, uint8_t const *source_address)
{
    SparePage spare;
    int retval = NAND_STATUS_OK;
    int result;

    // initialize spare
    for (int i = 0; i < sizeof(SparePage); ++i) {
        *((uint8_t *)&spare + i) = 0xff;
    }

    NANDEnableChipSelect();

    *NFCMMD = NAND_CMD_PAGE_PROGRAM1st;

    NANDSetAddress(NAND_PAGE_NUMBER(nand_address), 0);

    NANDActivateECCEncoder();

    for (int packet = 0; packet < NAND_PACKETS_PER_PAGE; ++packet) {
        NANDConfigureECC();

        busy_wait(500);

        NANDUnlockMainAreaECC();

        // Write packet data
        for (int i = 0; i < NAND_PACKET_SIZE; ++i) {
            *NFDATA_8 = *source_address;
            ++source_address;
        }

        busy_wait(4);

        SparePacket *spare_packet = &spare.packets[packet];
        spare_packet->bad = 0xff;
        spare_packet->other = 0xfe;
        spare_packet->parity_low = *NFMECC0;
        spare_packet->parity_high = *NFMECC1;
    }

    // Write spare data
    uint8_t *sparepointer = (uint8_t *)&spare;
    for (int i = 0; i < sizeof(SparePage); ++i) {
        *NFDATA_8 = *sparepointer;
        ++sparepointer;
    }

    NANDClearReady();

    *NFCMMD = NAND_CMD_PAGE_PROGRAM2nd;

    result = NANDWaitReady();
    if (result != NAND_STATUS_OK && retval == NAND_STATUS_OK) {
        retval = result;
    }

    result = NANDCheckWrite();
    if (result != NAND_STATUS_OK && retval == NAND_STATUS_OK) {
        retval = result;
    }

    NANDDisableChipSelect();

    return retval;
}

// Returns NAND_STATUS.
static int NANDInitBlockTranslationTable(void)
{
    BFX bfx;
    for (int i = 0; i < sizeof(BFX) / NAND_PAGE_SIZE; ++i) {
        NANDReadPage(0x2000 + i * NAND_PAGE_SIZE, (uint8_t *)(&bfx) + i * NAND_PAGE_SIZE);
    }

    if (bfx.magic != 0x00584642) {
        return NAND_STATUS_NO_TABLE;
    }

    int l = 0;
    int n = 0;
    for (int block = 0; block < NAND_NUM_BLOCKS; ++block) {
        if (bfx.locked_blocks[l] == block) {
            ++l;
        } else {
            nand_block_translation_table[n] = block;
            ++n;
        }
    }

    return NAND_STATUS_OK;
}

// Returns NAND_STATUS.
static int NANDReset()
{
    int retval = NAND_STATUS_OK;
    int result;

    NANDEnableChipSelect();
    NANDClearReady();

    *NFCMMD = NAND_CMD_CHIP_RESET;

    result = NANDWaitReady();
    if (result != NAND_STATUS_OK && retval == NAND_STATUS_OK) {
        retval = result;
    }

    NANDDisableChipSelect();

    return retval;
}

int NANDInit(void)
{
    int result;

    // BASIC HARDWARE INITIALIZATION
    *NFCONF = 0x1007770; // Set Page Size, TWRPH1=7, TWRPH0=7, TACLS=7, ECCType=4-bitECC, MesgLength=512-bytes
    *NFCONT = 0Xf7;      // Disable Chip Select, NAND Controller Enabled, Init MECC and SECC, Lock MECC, SECC

    *GPMCON =  (*GPMCON & ~0xc) | 0x8;  // Set GPM1 as FRnB (NAND ready/busy signal)

    *GPLCON = (*GPLCON & ~0x0C000000) | 0x4000000;  // Set GPL13 as output, connected to WP (NAND write protect)
    *GPLUDP = (*GPLUDP & ~0x0C000000) ;               // Disable pull up/down
    *GPLDAT |= 0x2000;                                // Active high = write enabled

    *NFSTAT = 0x70;     // Clear all flags

    result = NANDReset();
    if (result != NAND_STATUS_OK) {
        return result;
    }

    result = NANDInitBlockTranslationTable();
    if (result != NAND_STATUS_OK) {
        return result;
    }

    return NAND_STATUS_OK;
}

int NANDBlockErase(uint32_t nand_address)
{
    int retval = NAND_STATUS_OK;
    int result;

    NANDEnableChipSelect();

    *NFCMMD = NAND_CMD_BLOCK_ERASE1st;

    NANDSetRowAddress(NAND_PAGE_NUMBER(NAND_BLOCK_START(nand_address)));

    NANDClearReady();

    *NFCMMD = NAND_CMD_BLOCK_ERASE2nd;

    result = NANDWaitReady();
    if (result != NAND_STATUS_OK && retval == NAND_STATUS_OK) {
        retval = result;
    }

    // PRIME firmware does this but in combination with a more primitive write check routine
    busy_wait(100);

    // NOTE BL2 does not check NFSTAT_IllegalAccess here
    result = NANDCheckWrite();
    if (result != NAND_STATUS_OK && retval == NAND_STATUS_OK) {
        retval = result;
    }

    NANDDisableChipSelect();

    return retval;
}

static uint32_t NANDTranslateVirtualAddress(uint32_t virtual_address)
{
    uint32_t virtual_block_number = NAND_BLOCK_NUMBER(virtual_address);
    uint32_t offset = virtual_address & ~NAND_BLOCK_MASK;

    uint32_t nand_block_number = nand_block_translation_table[virtual_block_number];

    uint32_t nand_address = NAND_BLOCK_ADDR(nand_block_number) | offset;
    return nand_address;
}

int NANDRead(uint32_t virtual_address, uint8_t *target_address, unsigned int num_bytes)
{
    int retval = NAND_STATUS_OK;
    int result;

    while (num_bytes != 0) {
        uint32_t page_start = NAND_PAGE_START(virtual_address);
        uint32_t offset = virtual_address - page_start;

        uint32_t nand_read_address = NANDTranslateVirtualAddress(page_start);
        result = NANDReadPage(nand_read_address, nand_buffer);
        if (result != NAND_STATUS_OK && retval == NAND_STATUS_OK) {
            retval = result;
        }

        int bytes_used = NAND_PAGE_SIZE - offset;
        if (num_bytes < bytes_used) {
            bytes_used = num_bytes;
        }

        memcpyb(target_address, nand_buffer + offset, bytes_used);

        virtual_address += bytes_used;
        target_address += bytes_used;
        num_bytes -= bytes_used;
    }

    return retval;
}

int NANDReadBlock(uint32_t nand_address, uint8_t *target_address)
{
    int retval = NAND_STATUS_OK;
    int result;

    for (int page = 0; page < NAND_PAGES_PER_BLOCK; ++page) {
        result = NANDReadPage(nand_address + page * NAND_PAGE_SIZE, target_address + page * NAND_PAGE_SIZE);
        if (result != NAND_STATUS_OK && retval == NAND_STATUS_OK) {
            retval = result;
        }
    }

    return retval;
}

int NANDWriteBlock(uint32_t nand_address, uint8_t const *source_address)
{
    int result;

    for (int page = 0; page < NAND_PAGES_PER_BLOCK; ++page) {
        result = NANDWritePage(nand_address + page * NAND_PAGE_SIZE, source_address + page * NAND_PAGE_SIZE);
        if (result != NAND_STATUS_OK) {
            return result;
        }
    }

    return NAND_STATUS_OK;
}

int NANDWrite(uint32_t virtual_address, uint8_t const *source_address, unsigned int num_bytes)
{
    int result;

    while (num_bytes != 0) {
        uint32_t block_start = NAND_BLOCK_START(virtual_address);
        uint32_t offset = virtual_address - block_start;

        uint32_t nand_write_address = NANDTranslateVirtualAddress(block_start);

        if ( (offset != 0) || (num_bytes < NAND_BLOCK_SIZE)) {
            result = NANDReadBlock(nand_write_address, nand_buffer);
            if (result != NAND_STATUS_OK) {
                // Don't try to continue or wrong data would be written
                return result;
            }
        }

        int bytes_used = NAND_BLOCK_SIZE - offset;
        if (num_bytes < bytes_used) {
            bytes_used = num_bytes;
        }

        memcpyb(nand_buffer + offset, source_address, bytes_used);

        result = NANDBlockErase(nand_write_address);
        if (result != NAND_STATUS_OK) {
            // Don't try to continue writing over unknown state
            return result;
        }

        result = NANDWriteBlock(nand_write_address, nand_buffer);
        if (result != NAND_STATUS_OK) {
            return result;
        }

        virtual_address += bytes_used;
        source_address += bytes_used;
        num_bytes -= bytes_used;
    }

    return NAND_STATUS_OK;
}
