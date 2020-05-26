#ifndef _NAND_H_
#define _NAND_H_

#include <stdint.h>

#define NAND_SIZE         0x10000000 // 256Mb
#define NAND_BLOCK_SIZE   0x00020000 // 128kb
#define NAND_PAGE_SIZE    0x00000800 // 2kb
#define NAND_PACKET_SIZE  0x00000200 // 512b

#define NAND_BLOCK_MASK   0xfffe0000
#define NAND_PAGE_MASK    0xfffff800
#define NAND_PACKET_MASK  0xfffffe00

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

// Write protect whole NAND
void NANDWriteProtect(void);

// @read_address needs to be page aligned.
// @num_bytes needs to be multiple of NAND_PAGE_SIZE.
// Corrects block address.
// Does ECC error correction.
// Returns 1 on success, 0 on error.
int NANDReadPages(uint32_t virtual_read_address, uint8_t *target_address, unsigned int num_bytes);

#endif
