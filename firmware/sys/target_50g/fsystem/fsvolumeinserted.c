/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"

// RETURN FS_OK IF VOLUME IS MOUNTED AND THE CARD IS INSERTED
// RETURN FS_NOCARD IF NO CARD IS INSERTED
// RETURN FS_CHANGED IF VOLUME IS MOUNTED BUT ANOTHER CARD WAS INTRODUCED

int FSVolumeInserted(int VolNumber)
{
int error=FSInit();
if(error!=FS_OK) return error;
if(VolNumber<0 || VolNumber>3) return FS_BADVOLUME;
if(FSystem.Volumes[VolNumber]!=NULL) return FSVolumePresent(FSystem.Volumes[VolNumber]);
return FS_BADVOLUME;

}


