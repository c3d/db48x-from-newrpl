/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


// $Header: fsexpandchain.c,r42 2006-02-18 04:23:47 ingo $


#include "fsyspriv.h"



#ifndef CONFIG_NO_FSYSTEM



// EXPAND A CLUSTER CHAIN TO CONTAIN AT LEAST newtotalsize BYTES
int FSExpandChain(FS_FILE *file,unsigned int newtotalsize)
{
int needed,taken;
unsigned int size=0;
unsigned int fixcluster;
FS_FRAGMENT *fr=&file->Chain;
FS_FRAGMENT *newfr;
FS_VOLUME *fs=FSystem.Volumes[file->Volume];

if(!FSystem.Init) return FS_ERROR;
if(!fs) return FS_ERROR;
//printf("Expand started\n");

while(fr->NextFragment) {
//printf("fr=%04X to %04X\n",FSAddr2Cluster(fr->StartAddr,fs),FSAddr2Cluster(fr->EndAddr,fs)-1);
size+=fr->EndAddr-fr->StartAddr;
fr=fr->NextFragment;
}



if(fr->StartAddr!=0) { size+=fr->EndAddr-fr->StartAddr; 
//printf("fr=%04X to %04X\n",FSAddr2Cluster(fr->StartAddr,fs),FSAddr2Cluster(fr->EndAddr,fs)-1);
}


// NUMBER OF NEW CLUSTERS TO ALLOCATE
needed=(((uint64_t)newtotalsize-(((uint64_t)size)<<9))+ (1<<fs->ClusterSize) -1 )>>fs->ClusterSize;
if(needed<=0) return FS_OK;		// NO NEED TO ADD CLUSTERS TO THE CHAIN

needed<<=(fs->ClusterSize-9);
//printf("needed=%d\n",needed);
if((fs->InitFlags&VOLFLAG_FREESPACEVALID)&&(fs->FreeSpace<(unsigned int)needed)) return FS_DISKFULL;

do {

if(!fs->FreeAreaSize) {
// IF CALCFREESPACE FAILS TO READ THERE'S A BIG PROBLEM, ABORT
    int tmperr=FSScanFreeSpace(fs,FSAddr2Cluster(fs->NextFreeCluster,fs));
    if(tmperr!=FS_OK) return tmperr;
}

if(fs->FreeAreaSize>(unsigned int)needed) taken=needed;
else taken=fs->FreeAreaSize;

//printf("taken=%d\n",taken);
// TAKE FREE SPACE
if(fr->EndAddr==fs->NextFreeCluster) {
// EXPAND CURRENT FRAGMENT
//printf("expand current frag\n");

fixcluster=fr->EndAddr;
fr->EndAddr+=taken;
// WRITE FAT ENTRIES
while(fixcluster!=fr->EndAddr)
{
FSWriteFATEntry(FSAddr2Cluster(fixcluster,fs)-1,FSAddr2Cluster(fixcluster,fs),fs);
fixcluster+=1<<(fs->ClusterSize-9);
}
FSWriteFATEntry(FSAddr2Cluster(fixcluster,fs)-1,0xfffffff,fs);		// END-OF-CHAIN MARKER
//printf("st=%08X, e=%08X\n",fr->StartAddr,fr->EndAddr);
}
else {
//printf("new frag\n");
// CREATE NEW FRAGMENT
if(fr->StartAddr!=0) {
newfr=(FS_FRAGMENT *)simpmallocb(sizeof(FS_FRAGMENT));
if(!newfr) return FS_ERROR;
}
else {
newfr=fr;
file->FirstCluster=FSAddr2Cluster(fs->NextFreeCluster,fs);
}

newfr->StartAddr=fs->NextFreeCluster;
newfr->EndAddr=newfr->StartAddr+taken;
fr->NextFragment=newfr;
newfr->NextFragment=NULL;
if(fr!=newfr) {
// LINK OLD CHAIN W/NEW FRAGMENT
FSWriteFATEntry(FSAddr2Cluster(fr->EndAddr,fs)-1,FSAddr2Cluster(newfr->StartAddr,fs),fs);
}
fr=newfr;
//printf("st=%08X, e=%08X\n",fr->StartAddr,fr->EndAddr);
fixcluster=fr->StartAddr+(1<<(fs->ClusterSize-9));
while(fixcluster!=fr->EndAddr)
{
FSWriteFATEntry(FSAddr2Cluster(fixcluster,fs)-1,FSAddr2Cluster(fixcluster,fs),fs);
fixcluster+=1<<(fs->ClusterSize-9);
}
FSWriteFATEntry(FSAddr2Cluster(fixcluster,fs)-1,0xfffffff,fs);		// END-OF-CHAIN MARKER

}

fs->NextFreeCluster+=taken;
fs->FreeAreaSize-=taken;
if(fs->InitFlags&VOLFLAG_FREESPACEVALID) fs->FreeSpace-=taken;
fs->InitFlags|=VOLFLAG_HINTDIRTY;
needed-=taken;


} while(needed);
return FS_OK;
}





#endif
