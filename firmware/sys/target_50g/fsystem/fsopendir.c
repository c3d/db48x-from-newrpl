/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"



// OPEN DIRECTORIES FOR ENTRY SCANNING

int FSOpenDir(char *name,FS_FILE **fileptr)
{
FS_FILE *entry,*old;
int error;
FS_VOLUME *fs;


error=FSInit();
if(error!=FS_OK) return error;


entry=(FS_FILE *)simpmallocb(sizeof(FS_FILE));
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

entry->Mode=FSMODE_READ;


}
else {
// USE AN ALREADY OPENED DIRECTORY
old=entry;
entry=entry->Dir;
simpfree(old);
if(!(entry->Attr&FSATTR_DIR)) {
// CLEANUP PROCEDURE
return FS_NOTFOUND;
}

fs=FSystem.Volumes[entry->Volume];
//printf("fs=%08X\n",(unsigned int)fs);
}





for(error=0;error<FS_MAXOPENFILES;++error)
{
if(fs->Files[error]==NULL) break;
}

if(error==FS_MAXOPENFILES) {
while(entry!=NULL) entry=FSFreeFile(entry);
return FS_MAXFILES; 
}


fs->Files[error]=entry;
*fileptr=entry;
entry->CurrentOffset=0;
return FS_OK;
}
