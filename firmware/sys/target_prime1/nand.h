#ifndef _NAND_H_
#define _NAND_H_

#include <stdint.h>

// Write protect whole NAND
void NANDWriteProtect(void);

// Reads @num_bytes bytes from NAND @read_address and writes to ram @target_address
// All three parameters support arbitrary byte alignment 
void NANDRead(uint32_t read_address, unsigned int num_bytes, uint8_t *target_address);

#endif
