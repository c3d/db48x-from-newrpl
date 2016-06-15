/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"



// CHANGE CURRENT DIRECTORY

int FSChdir(char *name)
{
FS_FILE *entry,*old;
int error;
FS_VOLUME *fs;

error=FSInit();
if(error!=FS_OK) return error;


entry=(FS_FILE *) simpmallocb(sizeof(FS_FILE));
if(entry==NULL) return FS_ERROR;

// CLEAN ENTRY
memsetb((void *)entry,0,sizeof(FS_FILE));


// CHECK IF CARD IS PRESENT

error=FSVolumePresent(FSystem.Volumes[FSystem.CurrentVolume]);
if(error!=FS_OK) { simpfree(entry); return error; }


error=FSFindFile(name,entry,TRUE);
if((error!=FS_OK) && (error!=FS_OPENDIR)) { simpfree(entry); return error; }

if(error==FS_OK) {
// NEWLY OPENED DIR
//printf("chdir to new dir\n");
if(!(entry->Attr&FSATTR_DIR)) {
// CLEANUP PROCEDURE
while(entry!=NULL) entry=FSFreeFile(entry);
return FS_NOTFOUND;
}


fs=FSystem.Volumes[entry->Volume];


// GET FILE CLUSTER CHAIN
error=FSGetChain(entry->FirstCluster,&entry->Chain,fs);

if(error!=FS_OK) {
while(entry!=NULL) entry=FSFreeFile(entry);
return FS_ERROR;
}
entry->FileSize=FSGetChainSize(&entry->Chain);
}
else {
// USE AN ALREADY OPENED DIRECTORY
old=entry;
entry=entry->Dir;
simpfree(old);
if(!(entry->Attr&FSATTR_DIR)) return FS_NOTFOUND;
fs=FSystem.Volumes[entry->Volume];
}



old=fs->CurrentDir;
fs->CurrentDir=entry;
while(old!=NULL) old=FSFreeFile(old);

return FS_OK;

}

