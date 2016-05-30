/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"



// WRITE ALL CACHED ENTRIES

int FSFlushFATCache(FS_VOLUME *fs)
{
char * sector;
int bl=fs->Disk->CurrentBLen; //fs->Disk->WriteBlockLen;
int f;
int sectaddr,error;
FS_CHAINBUFFER *ch;

sector=(char *)malloc(1<<bl);
if(!sector) return FS_ERROR;
/*
if(!SDDSetBlockLen(fs->Disk,bl)) { free(sector); return FS_ERROR; }
*/

bl=(1<<bl)-1;		// CREATE BITMASK

while((ch=fs->FATCache))
{
sectaddr=FSCluster2FATEntry(ch->Entries[0].Cluster,fs);
if((sectaddr&bl)==bl) {
// ENTRY IS PARTIALLY WITHIN BLOCKS - ONLY FAT12
if(ch->Entries[0].EntryValue&0x40000000) ++sectaddr;
}
sectaddr&=~bl;

error=SDDRead(sectaddr,bl+1,sector, fs->Disk);
if(error!=bl+1) { free(sector); return FS_ERROR; }

FSPatchFATBlock(sector,bl+1,sectaddr,fs,TRUE);		// PATCH AND FLUSH ENTRIES

// WRITE TO ALL ACTIVE FATS
for(f=fs->NumFATS-1;f>=0;--f)
{

error=SDDWrite(sectaddr+f*(fs->FATSize<<fs->SectorSize),bl+1,sector, fs->Disk);

if(error!=bl+1) { free(sector); return FS_ERROR; }
}



}
free(sector);
return FS_OK;
}
