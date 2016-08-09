/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"



// REDUCE FAT CHAIN IF NOT NEEDED

int FSTruncateChain(FS_FILE *file, unsigned int newsize)
{
FS_FRAGMENT *ch,*ch2;
unsigned int clsize,size=0,addr,fullsize;
FS_VOLUME *fs=FSystem.Volumes[file->Volume];

ch=&file->Chain;
//printf("truncate newsize=%d\n",newsize);
if(!ch->StartAddr) return FS_OK;

clsize=(1<<fs->ClusterSize)-1;

newsize+=clsize;
newsize&=~clsize;		// ROUND TO CLUSTERS

fullsize=FSGetChainSize(ch);

if(newsize>=fullsize) return FS_OK;

clsize=(clsize+1)>>9;

newsize>>=9;
fullsize>>=9;

while(ch) {
size+=ch->EndAddr-ch->StartAddr;
if(size>=newsize) break;
ch=ch->NextFragment;
}

// FOUND LAST FRAGMENT
if(size==newsize) {
// FRAGMENT FITS EXACTLY
if(FSWriteFATEntry(FSAddr2Cluster(ch->EndAddr,fs)-1,0xfffffff,fs)!=FS_OK) return FS_ERROR;	// MARK LAST CLUSTER AS END-OF-CHAIN

} else {
if(size>newsize) {
// TRUNCATE CHAIN
addr=ch->EndAddr-(size-newsize);
if(newsize) {
if(FSWriteFATEntry(FSAddr2Cluster(addr,fs)-1,0xfffffff,fs)!=FS_OK) return FS_ERROR;	// MARK LAST CLUSTER AS END-OF-CHAIN
}
else {
ch->StartAddr=0;		// CHAIN IS NOT ALLOCATED
file->FirstCluster=0;
}
while(addr!=ch->EndAddr) {
if(FSWriteFATEntry(FSAddr2Cluster(addr,fs),0,fs)!=FS_OK) return FS_ERROR;	// MARK CLUSTERS AS FREE
addr+=clsize;
}
ch->EndAddr-=size-newsize;
}
}


// FREE THE REST OF THE CLUSTER CHAIN

ch2=ch->NextFragment;
ch->NextFragment=NULL;

while(ch2) {
addr=ch2->StartAddr;
while(addr!=ch2->EndAddr) {
FSWriteFATEntry(FSAddr2Cluster(addr,fs),0,fs);	// MARK CLUSTERS AS FREE
addr+=clsize;
}
ch=ch2;
ch2=ch2->NextFragment;
simpfree(ch);
}


if(fs->InitFlags&2) fs->FreeSpace-=newsize-fullsize;
return FS_OK;

}


