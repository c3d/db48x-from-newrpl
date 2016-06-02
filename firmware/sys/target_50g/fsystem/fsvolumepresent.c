/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

// DETECT IF A VOLUME IS STILL PRESENT

int FSVolumePresent(FS_VOLUME *fs)
{
int f;

if(!SDCardInserted()) return FS_NOCARD;

SDSelect(0);
if(SDSelect(fs->Disk->Rca)) return FS_OK;		// RCA IS VALID--> CARD WAS NEVER REMOVED
// CARD WAS REMOVED, RESET CARD
SD_CARD newcard;
if(!SDCardInit(&newcard)) return FS_NOCARD;

// CARD MAY HAVE BEEN REMOVED AND REINSERTED
for(f=0;f<4;++f)
{
if(newcard.CID[f]!=fs->Disk->CID[f]) {
// IT'S A DIFFERENT CARD
return FS_CHANGED;
}
}

// SAME CARD WAS OUT AND BACK IN
fs->Disk->Rca=newcard.Rca;
if(SDSelect(fs->Disk->Rca)) return FS_OK;
return FS_ERROR;
}
