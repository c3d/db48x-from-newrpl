/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "nand.h"
#include "target_prime1.h"
#include <newrpl.h>
#include <stdint.h>
#include "nand.h"

// Spare area layout
// 0x00 = Bad indicator
// 0x08-0x0F = 7 bytes ECC parity code for packet 1
// 0x18-0x1F = 7 bytes ECC parity code for packet 2
// 0x28-0x2F = 7 bytes ECC parity code for packet 3
// 0x38-0x3F = 7 bytes ECC parity code for packet 4

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
#define NFSTAT_ECCDecDone      0x00000040

#define NFECCERR0_ECCReady     0x40000000

#define NAND_CMD_READ1st       0x00
#define NAND_CMD_READ2nd       0x30
#define NAND_CMD_RND_OUT1st    0x05
#define NAND_CMD_RND_OUT2nd    0xe0

// Some registers are accessed with other data width than 32bit

// Data is read bytewise
#define NFDATA_8 (volatile uint8_t *)NFDATA

// error location is halfword
#define NFECCERR0_16 (volatile uint16_t *)NFECCERR0

// error bit pattern is byte
#define NFMLCBITPT_8 (volatile uint8_t *)NFMLCBITPT

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
} BFX;

static uint16_t nand_block_translation_table[NAND_NUM_BLOCKS];

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

static void NANDSetReadAddress(unsigned int row, unsigned int column)
{
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
    int count = (*NFECCERR0 >> 29) & 0x03;

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

static void NANDConfigureECCDecoder(void)
{
    // Message length 512 bytes
    *NFCONF &= ~NFCONF_MsgLength;

    // 4-bit ECC
    *NFCONF = (*NFCONF & ~NFCONF_ECCTypeMask) | NFCONF_ECCType4;

    // Initialize main and spare area ECC decoder
    // TODO why activate spare area decoding here?
    *NFCONT |= NFCONT_InitSECC | NFCONT_InitMECC;

    // Unlock main area ECC
    *NFCONT &= ~NFCONT_MainECCLock;
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

// Reads whole aligned page at @read_address into @target_address.
// Does ECC error correction.
// Does not correct block address.
// Returns 1 on success, 0 on error
static int NANDReadPage(uint32_t nand_read_address, uint8_t *target_address)
{
  NANDEnableChipSelect();

  NANDSetReadAddress(NAND_PAGE_NUMBER(nand_read_address), 0);

  NANDActivateECCDecoder();

  for (int packet = 0; packet < NAND_PACKETS_PER_PAGE; ++packet) {
    int column;

    NANDConfigureECCDecoder();

    // Read packet data
    for (int i = 0; i < NAND_PACKET_SIZE; ++i) {
      *target_address = *NFDATA_8;
      ++target_address;
    }

    // Change column to stored ECC parity from spare area
    NANDChangeReadColumn(NAND_PAGE_SIZE + 8 + (packet << 4));

    // Read ECC parity code for packet
    for (int i = 0; i < 7; ++i) {
      *NFDATA_8;
    }

    NANDWaitForECCDecoder();

    // Try to correct errors
    if (NANDECC4Correct(target_address - NAND_PACKET_SIZE) == 0) {
      return 0;
    }

    // Change column to next packet
    if (packet < (NAND_PACKETS_PER_PAGE - 1)) {
      NANDChangeReadColumn(NAND_PACKET_SIZE * (packet + 1));
    }
  }

  NANDDisableChipSelect();

  return 1;
}

// Reads first byte of spare area and checks it against validity conditions
// taken from BXCBOOT0.BIN from 20140331 firmware.
// Returns 1 if page @address lies in is valid, else 0.
static int NANDIsPageValid(uint32_t nand_address)
{
  NANDEnableChipSelect();

  NANDSetReadAddress(NAND_PAGE_NUMBER(nand_address), 1 << NAND_PAGE_BITS);

  uint8_t byte = *NFDATA_8;

  NANDDisableChipSelect();

  return (byte == 0x00 || byte == 0xf0) ? 0 : 1;
}

// Checks first, second and last page in block.
// Returns 1 if block @address lies in is valid, else 0.
static int NANDIsBlockValid(uint32_t nand_address)
{
  unsigned int page_0 = nand_address & NAND_BLOCK_MASK;
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
    if (NANDInitBlockTranslationTable() == 0) {
        return 0;
    }

    return 1;
}

static uint32_t NANDTranslateVirtualAddress(uint32_t virtual_address)
{
    uint32_t virtual_block_number = NAND_BLOCK_NUMBER(virtual_address);
    uint32_t offset = virtual_address & ~NAND_BLOCK_MASK;

    uint32_t nand_block_number = nand_block_translation_table[virtual_block_number];

    uint32_t nand_address = NAND_BLOCK_ADDR(nand_block_number) | offset;
    return nand_address;
}

int NANDReadPages(uint32_t virtual_read_address, uint8_t *target_address, unsigned int num_bytes)
{
    while (num_bytes > 0) {
        uint32_t nand_read_address = NANDTranslateVirtualAddress(virtual_read_address);

        if (NANDReadPage(nand_read_address, target_address) == 0) {
            return 0;
        }

        target_address += NAND_PAGE_SIZE;
        virtual_read_address += NAND_PAGE_SIZE;
        num_bytes -= NAND_PAGE_SIZE;
    }

    return 1;
}
