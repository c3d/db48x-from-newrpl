#ifndef _NAND_H_
#define _NAND_H_

#include <stdint.h>

// All NAND code is hardcoded for newtype NANDs with device code 0xda.
// 256Mb 8-bit.

#define NAND_SIZE         0x10000000 // 256Mb
#define NAND_BLOCK_SIZE   0x00020000 // 128kb
#define NAND_PAGE_SIZE    0x00000800 // 2kb
#define NAND_PACKET_SIZE  0x00000200 // 512b

#define NAND_BLOCK_MASK   0xfffe0000
#define NAND_PAGE_MASK    0xfffff800
#define NAND_PACKET_MASK  0xfffffe00

#define NAND_BLOCK_START(addr) (addr & NAND_BLOCK_MASK)
#define NAND_PAGE_START(addr) (addr & NAND_PAGE_MASK)

#define NAND_BLOCK_BITS           17
#define NAND_PAGE_BITS            11
#define NAND_PACKET_BITS           9

#define NAND_NUM_BLOCKS       (NAND_SIZE / NAND_BLOCK_SIZE)       // 2048
#define NAND_PAGES_PER_BLOCK  (NAND_BLOCK_SIZE / NAND_PAGE_SIZE)  // 64
#define NAND_PACKETS_PER_PAGE (NAND_PAGE_SIZE / NAND_PACKET_SIZE) // 4

#define NAND_BLOCK_ADDR(number) (number << NAND_BLOCK_BITS)

#define NAND_BLOCK_NUMBER(addr) (addr >> NAND_BLOCK_BITS)
#define NAND_PAGE_NUMBER(addr) (addr >> NAND_PAGE_BITS)
#define NAND_PACKET_NUMBER_IN_PAGE(addr) ((addr & ~NAND_PAGE_MASK) >> NAND_PACKET_BITS)

// Initializes NAND handling and needs to be called before any other NAND function.
// Returns 1 on success, 0 on error.
int NANDInit(void);

// Write protect whole NAND.
void NANDWriteProtect(void);

// Checks if block at @nand_address is bad.
// Returns 1 if block @address lies in is bad, else 0.
int NANDIsBlockBad(uint32_t nand_address);

// Marks block at @nand_address bad.
// Returns 1 on success, 0 on error.
int NANDMarkBlockBad(uint32_t nand_address);

// Block at @nand_address gets erased.
// Returns 1 on success, 0 on error.
int NANDBlockErase(uint32_t nand_address);

// Reads whole aligned page at @nand_address into @target_address.
// Tries to restore as much information as possible should error occur.
// Does ECC error correction.
// Does not correct block address.
// Returns 1 on success, 0 on error.
int NANDReadPage(uint32_t nand_address, uint8_t *target_address);

// Writes data from @source_address to whole aligned page at @nand_address.
// Returns 1 on success, 0 on error.
int NANDWritePage(uint32_t nand_address, uint8_t *source_address);

// Reads @num_bytes from @virtual_address to @target_address
// @virtual_address and @num_bytes need not be page aligned.
// Tries to restore as much information as possible should error occur.
// Corrects block address according to BL2 block translation table.
// Does ECC error correction.
// Returns 1 on success, 0 on error.
int NANDRead(uint32_t virtual_address, uint8_t *target_address, unsigned int num_bytes);

#endif
