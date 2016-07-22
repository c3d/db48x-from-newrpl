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
unsigned char * sector;
int bl=fs->Disk->CurrentBLen; //fs->Disk->WriteBlockLen;
int f;
unsigned int sectaddr;
int error;
FS_CHAINBUFFER *ch;

sector=simpmallocb(1<<bl);
if(!sector) return FS_ERROR;
/*
if(!SDDSetBlockLen(fs->Disk,bl)) { simpfree(sector); return FS_ERROR; }
*/

bl=(1<<bl)-1;		// CREATE BITMASK

while((ch=fs->FATCache))
{
sectaddr=FSCluster2FATEntry(ch->Entries[0].Cluster,fs);
if((sectaddr&bl)==(unsigned int)bl) {
// ENTRY IS PARTIALLY WITHIN BLOCKS - ONLY FAT12
if(ch->Entries[0].EntryValue&0x40000000) ++sectaddr;
}
sectaddr&=~bl;

error=SDDRead((((uint64_t)fs->FirstFATAddr)<<9)+sectaddr,bl+1,sector, fs->Disk);
if(error!=bl+1) { simpfree(sector); return FS_ERROR; }

FSPatchFATBlock(sector,bl+1,sectaddr,fs,TRUE);		// PATCH AND FLUSH ENTRIES

// WRITE TO ALL ACTIVE FATS
for(f=fs->NumFATS-1;f>=0;--f)
{

error=SDDWrite((((uint64_t)fs->FirstFATAddr)<<9)+sectaddr+f*(fs->FATSize<<fs->SectorSize),bl+1,sector, fs->Disk);

if(error!=bl+1) { simpfree(sector); return FS_ERROR; }
}



}
simpfree(sector);
return FS_OK;
}
