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
	if (NANDRead(SDAddr + FS_VIRTUAL_FAT_ADDRESS, buffer, NumBytes) == 0) {
		return 0;
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
	if (NANDWrite(SDAddr + FS_VIRTUAL_FAT_ADDRESS, buffer, NumBytes) == 0) {
		return 0;
	}

	return NumBytes;
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
    return FALSE;
}
