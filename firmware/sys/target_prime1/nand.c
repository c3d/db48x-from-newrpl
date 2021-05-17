/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "hal_api.h"
#include "target_prime1.h"
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

#define NAND_STATUS_FAIL         0x01

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

static inline void NANDWaitReady(void)
{
    while ((*NFSTAT & NFSTAT_RnB_TransDetect) == 0);
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

// Returns 1 if ok, 0 if uncorrectable
static int NANDECC4Correct(uint8_t *data)
{
    int count = (*NFECCERR0 >> 26) & 0x07;

    if (count == 0) {
        return 1;
    } else if (count > 4) {
        return 0;
    }

    volatile uint16_t *error_location = NFECCERR0_16;
    volatile uint8_t *bit_pattern = NFMLCBITPT_8;

    while (count) {
        data[*error_location] ^= *bit_pattern;
        ++bit_pattern;
        ++error_location;
        --count;
    }

    return 1;
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
    int retval = 1;

    NANDEnableChipSelect();

    *NFCMMD = NAND_CMD_READ1st;

    NANDSetAddress(NAND_PAGE_NUMBER(nand_address), 0);

    NANDClearReady();

    *NFCMMD = NAND_CMD_READ2nd;

    NANDWaitReady();

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
        if (NANDECC4Correct(target_address - NAND_PACKET_SIZE) == 0) {
          retval = 0;
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
// Returns 1 if page @address lies in is bad, else 0.
static int NANDIsPageBad(uint32_t nand_address)
{
    NANDEnableChipSelect();

    NANDClearReady();

    *NFCMMD = NAND_CMD_READ1st;

    NANDSetAddress(NAND_PAGE_NUMBER(nand_address), NAND_PAGE_SIZE);

    *NFCMMD = NAND_CMD_READ2nd;

    NANDWaitReady();

    uint8_t byte = *NFDATA_8;

    NANDDisableChipSelect();

    return (byte == 0x00 || byte == 0xf0) ? 1 : 0;
}

// Checks first, second and last page in block. If any of these has bad marker
// whole block is bad.
int NANDIsBlockBad(uint32_t nand_address)
{
    unsigned int page_0 = NAND_BLOCK_START(nand_address);
    if (NANDIsPageBad(page_0)) {
        return 1;
    }

    unsigned int page_1 = page_0 + NAND_PAGE_SIZE;
    if (NANDIsPageBad(page_1)) {
        return 1;
    }

    unsigned int page_last = page_0 + NAND_BLOCK_SIZE - NAND_PAGE_SIZE;
    if (NANDIsPageBad(page_last)) {
        return 1;
    }

    return 0;
}

static void __attribute__ ((noinline)) busy_wait(unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i) {
        // asm statement with data dependency and potential side effect
        // can't be optimized away
        __asm__ volatile("" : "+g" (i) : :);
    }
}

// Returns 1 on success, 0 on error.
static int NANDCheckWrite(void)
{
    if ((*NFSTAT & NFSTAT_IllegalAccess) != 0) {
        return 0;
    }

    *NFCMMD = NAND_CMD_READ_STATUS;

    busy_wait(3);

    for (int i = 0; i < 1024; ++i) {
        if ((*NFDATA_8 & NAND_STATUS_FAIL) == 0) {
            return 1;
        }
    }
    return 0;
}

// Sets bad marker in spare for first packet to 0x00.
// Returns 1 on success, 0 on error.
static int NANDMarkPageBad(uint32_t nand_address)
{
    int retval = 1;

    NANDEnableChipSelect();

    *NFCMMD = NAND_CMD_PAGE_PROGRAM1st;

    NANDSetAddress(NAND_PAGE_NUMBER(nand_address), NAND_PAGE_SIZE);

    *NFDATA_8 = 0;

    NANDClearReady();

    *NFCMMD = NAND_CMD_PAGE_PROGRAM2nd;

    NANDWaitReady();

    if (NANDCheckWrite() == 0) {
        retval = 0;
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
    int retval = 1;

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

    NANDWaitReady();

    if (NANDCheckWrite() == 0) {
        retval = 0;
    }

    NANDDisableChipSelect();

    return retval;
}

// Returns 1 on success, 0 on error.
static int NANDInitBlockTranslationTable(void)
{
    BFX bfx;
    for (int i = 0; i < sizeof(BFX) / NAND_PAGE_SIZE; ++i) {
        NANDReadPage(0x2000 + i * NAND_PAGE_SIZE, (uint8_t *)(&bfx) + i * NAND_PAGE_SIZE);
    }

    if (bfx.magic != 0x00584642) {
        return 0;
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

    return 1;
}

int NANDInit(void)
{
    // BASIC HARDWARE INITIALIZATION
    *NFCONF = 0x1007770; // Set Page Size, TWRPH1=7, TWRPH0=7, TACLS=7, ECCType=4-bitECC, MesgLength=512-bytes
    *NFCONT = 0Xf7;      // Disable Chip Select, NAND Controller Enabled, Init MECC and SECC, Lock MECC, SECC

    *GPMCON =  (*GPMCON & ~0xc) | 0x8;  // Set GPM1 as FRnB (NAND ready/busy signal)

    *NFSTAT = 0x70;     // Clear all flags

    NANDEnableChipSelect();

    *NFCMMD = NAND_CMD_CHIP_RESET;

    NANDWaitReady();

    NANDDisableChipSelect();


    if (NANDInitBlockTranslationTable() == 0) {
        return 0;
    }

    return 1;
}

int NANDBlockErase(uint32_t nand_address)
{
    int retval = 1;

    NANDEnableChipSelect();

    *NFCMMD = NAND_CMD_BLOCK_ERASE1st;

    NANDSetRowAddress(NAND_PAGE_NUMBER(NAND_BLOCK_START(nand_address)));

    NANDClearReady();

    *NFCMMD = NAND_CMD_BLOCK_ERASE2nd;

    NANDWaitReady();

    // NOTE BL2 does not check NFSTAT_IllegalAccess here
    if (NANDCheckWrite() == 0) {
        retval = 0;
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
    while (num_bytes != 0) {
        uint32_t page_start = NAND_PAGE_START(virtual_address);
        uint32_t offset = virtual_address - page_start;

        uint32_t nand_read_address = NANDTranslateVirtualAddress(page_start);
        if (NANDReadPage(nand_read_address, nand_buffer) == 0) {
            return 0;
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

    return 1;
}

int NANDReadBlock(uint32_t nand_address, uint8_t *target_address)
{
    int retval = 1;
    
    for (int page = 0; page < NAND_PAGES_PER_BLOCK; ++page) {
        if (NANDReadPage(nand_address + page * NAND_PAGE_SIZE, target_address + page * NAND_PAGE_SIZE) == 0) {
            retval = 0;
        }
    }
    
    return retval;
}

int NANDWriteBlock(uint32_t nand_address, uint8_t const *source_address)
{
    for (int page = 0; page < NAND_PAGES_PER_BLOCK; ++page) {
        if (NANDWritePage(nand_address + page * NAND_PAGE_SIZE, source_address + page * NAND_PAGE_SIZE) == 0) {
            return 0;
        }
    }

    return 1;
}

int NANDWrite(uint32_t virtual_address, uint8_t const *source_address, unsigned int num_bytes)
{
    while (num_bytes != 0) {
        uint32_t block_start = NAND_BLOCK_START(virtual_address);
        uint32_t offset = virtual_address - block_start;

        uint32_t nand_write_address = NANDTranslateVirtualAddress(block_start);

        if (offset != 0 || num_bytes < NAND_BLOCK_SIZE) {
            if (NANDReadBlock(nand_write_address, nand_buffer) == 0) {
                return 0;
            }
        }

        int bytes_used = NAND_BLOCK_SIZE - offset;
        if (num_bytes < bytes_used) {
            bytes_used = num_bytes;
        }

        memcpyb(nand_buffer + offset, source_address, bytes_used);

        if (NANDBlockErase(nand_write_address) == 0) {
            return 0;
        }

        if (NANDWriteBlock(nand_write_address, nand_buffer) == 0) {
            return 0;
        }

        virtual_address += bytes_used;
        source_address += bytes_used;
        num_bytes -= bytes_used;
    }

    return 1;
}
