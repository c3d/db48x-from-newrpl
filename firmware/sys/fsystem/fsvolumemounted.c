/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

// RETURNS TRUE IF VOLUME EXISTS IN THE CURRENTLY INSERTED CARD

int FSVolumeMounted(int VolNumber)
{

int error=FSInit();
if(error!=FS_OK) return FALSE;

if(VolNumber<0 || VolNumber>3) return FALSE;
if(FSystem.Volumes[VolNumber]!=NULL) return TRUE;
return FALSE;
}
#endif
