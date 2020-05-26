/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include "../fsystem/fsyspriv.h"
#include "nand.h"

#define FS_VIRTUAL_FAT_ADDRESS 0x502000

int SDCardInit(SD_CARD * card)
{
	card->SysFlags = 0x1f;
	card->Rca = 0x00010000;
	return TRUE;
}

int SDDRead(uint64_t SDAddr, int NumBytes, unsigned char *buffer,
        SD_CARD * card)
{
	SDAddr += FS_VIRTUAL_FAT_ADDRESS;

	uint8_t page_buffer[NumBytes + 2 * NAND_PAGE_SIZE];

	// Page aligned virtual address
	uint32_t virtual_read_start_addr = SDAddr & NAND_PAGE_MASK;
	uint32_t virtual_read_end_addr = (SDAddr + NumBytes + NAND_PAGE_SIZE - 1) & NAND_PAGE_MASK;
	uint32_t bytes_to_read = virtual_read_end_addr - virtual_read_start_addr;

	if (NANDReadPages(virtual_read_start_addr, page_buffer, bytes_to_read) == 0) {
		return FALSE;
	}

	uint32_t offset = SDAddr - virtual_read_start_addr;
	for (int i = 0; i < NumBytes; ++i) {
		buffer[i] = page_buffer[i + offset];
	}

    return NumBytes;
}

void SDPowerDown()
{
}

void SDPowerUp()
{
}

int SDCardInserted()
{
    return TRUE;
}

int SDDWrite(uint64_t SDAddr, int NumBytes, unsigned char *buffer,
        SD_CARD * card)
{
	return 0;
}

int SDSelect(int RCA)
{
	return TRUE;
}

int SDIOSetup(SD_CARD * card, int shutdown)
{
	return (shutdown) ? FALSE : TRUE;
}

int SDCardWriteProtected()
{
    return TRUE;
}
