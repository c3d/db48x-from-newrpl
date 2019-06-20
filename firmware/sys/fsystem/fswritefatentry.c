/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM


// ADD A FAT ENTRY TO THE CACHE

int FSWriteFATEntry(unsigned int cluster,int value,FS_VOLUME *fs)
{
FS_CHAINBUFFER *ch;
int f;
ch=fs->FATCache;
//printf("FAT cluster=%04X, val=%04X\n",cluster,value);
//keyb_getkeyM(1);
while(ch)
{
for(f=0;f<ch->Used;++f)
{
if(ch->Entries[f].Cluster==cluster) {
// EXISTING ENTRY, REPLACE
ch->Entries[f].EntryValue=value&0x0fffffff;
return FS_OK;
}

}
ch=ch->Next;
}

// ENTRY DOESN'T EXIST, FIND AN EMPTY SLOT

retry:

ch=fs->FATCache;

while(ch)
{
if(ch->Used<16) {
ch->Entries[ch->Used].Cluster=cluster;
ch->Entries[ch->Used].EntryValue=value&0x0fffffff;
++ch->Used;
return FS_OK;
}
ch=ch->Next;
}

// NOT ENOUGH SPACE, ADD A NEW CACHEBUFFER

if(fs->NumCache>FS_MAXFATCACHE) if(FSFlushFATCache(fs)!=FS_OK) return FS_ERROR;

ch=(FS_CHAINBUFFER *)simpmallocb(sizeof(FS_CHAINBUFFER));
if(!ch) {
if(fs->NumCache) {
if(FSFlushFATCache(fs)!=FS_OK) return FS_ERROR;
goto retry;
}
return FS_ERROR;

}

ch->Next=fs->FATCache;
ch->Entries[0].Cluster=cluster;
ch->Entries[0].EntryValue=value&0x0fffffff;
ch->Used=1;
fs->FATCache=ch;
++fs->NumCache;

return FS_OK;

}
#endif
