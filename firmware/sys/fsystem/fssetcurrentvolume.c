/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"


#ifndef CONFIG_NO_FSYSTEM


int FSSetCurrentVolume(int VolNumber)
{
int error=FSInit();
if(error!=FS_OK) return error;

if(FSVolumeMounted(VolNumber)) {
FSystem.CurrentVolume=VolNumber;
return FS_OK;
}
return FS_BADVOLUME;
}

#endif



