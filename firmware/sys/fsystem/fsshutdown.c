/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"

// SHUTDOWN FILE SYSTEM

int FSShutdown()
{
SD_CARD temp,*card=NULL;
FS_VOLUME *fs;
FS_FILE *file;
int i,j;

if(!FSystem.Init) return TRUE;

// CLOSE OPEN FILES AND UNMOUNT VOLUMES
// AND FREE ALL ALLOCATED TEMPORARY MEMORY
for(i=0;i<4;++i)
{
fs=FSystem.Volumes[i];
if(fs!=NULL) {
for(j=0;j<FS_MAXOPENFILES;++j)
{
if(fs->Files[j]!=NULL) {
//printf("closing files %d\n",j);
FSClose(fs->Files[j]);
}
}

// ALL DATA WAS WRITTEN TO VOLUME

// TODO: MARK CLEAN UNMOUNT IF NEEDED

// FLUSH ANY PENDING ITEMS IN THE CACHE
FSFlushFATCache(fs);

// TODO: UPDATE FSINFO HINTS FOR FAT32


// DELETE CURRENTDIR IF ANY
//printf("Freeing current Dir\n");
if(fs->CurrentDir!=&(fs->RootDir)) {
file=fs->CurrentDir;
fs->CurrentDir=&fs->RootDir;
while(file) file=FSFreeFile(file);
}
// FREE CLUSTER IN ROOTDIR WHEN FAT32
FSFreeChain(&fs->RootDir);
FSystem.Volumes[i]=NULL;
//printf("done\n");
card=fs->Disk;
simpfree(fs);
}
}
if(card) simpfree(card);

// CLOSE SD DEVICE
SDPowerDown();
SDIOSetup(&temp,TRUE);

FSystem.Init=0;

return TRUE;
}


