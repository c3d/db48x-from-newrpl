/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"





int FSOpen(char *name, int mode, FS_FILE **fileptr)
{
FS_FILE *entry;
int error;
FS_VOLUME *fs;

*fileptr=NULL;

error=FSInit();
if(error!=FS_OK) return error;



entry=(FS_FILE *)simpmallocb(sizeof(FS_FILE));
if(entry==NULL) return FS_ERROR;

// CLEAN ENTRY
memsetb((void *)entry,0,sizeof(FS_FILE));


if( mode& (FSMODE_APPEND|FSMODE_MODIFY) ) mode|=FSMODE_WRITE; // APPEND/MODIFY IMPLY WRITE


// CHECK IF CARD IS PRESENT

error=FSVolumePresent(FSystem.Volumes[FSystem.CurrentVolume]);
if(error!=FS_OK) { simpfree(entry); return error; }


error=FSFindFile(name,entry,FALSE);
if(error!=FS_OK) { 

if(			error==FS_NOTFOUND && 
			( mode &FSMODE_WRITE ) &&
			!(mode&FSMODE_NOCREATE)
			)
{
// CREATE FILE
//printf("try to create\n");
error=FSCreate(name,0,fileptr);
//printf("create=%d\n",error);
if(error==FS_OK) {
(*fileptr)->Mode=mode;
}
}

simpfree(entry); return error;


}



fs=FSystem.Volumes[entry->Volume];


// REMOVED: NEW BEHAVIOR OF FINDFILE WILL RETURN FS_OPENFILE
/*
if(FSFileIsOpen(entry,fs))
{
// REFUSE TO OPEN A FILE IF IT'S ALREADY OPEN,
while(entry!=NULL) entry=FSFreeFile(entry);
return FS_OPENFILE;
}
*/

// CHECK IF DISK HAS AVAILABLE FILES TO OPEN

for(error=0;error<FS_MAXOPENFILES;++error)
{
if(fs->Files[error]==NULL) break;
}

if(error==FS_MAXOPENFILES) {
while(entry!=NULL) entry=FSFreeFile(entry);
return FS_MAXFILES; 
}

//printf("Files available\n");

entry->Mode=mode;


if(entry->Attr&FSATTR_VOLUME) {	// VOLUME FILES OR DIRECTORIES CANNOT BE OPEN
// CLEANUP PROCEDURE
while(entry!=NULL) entry=FSFreeFile(entry);
return FS_NOTFOUND;
}
if( (entry->Attr&FSATTR_RDONLY) && (mode&FSMODE_WRITE)) {	// CANT OPEN READ ONLY FILES FOR WRITE
// CLEANUP PROCEDURE
while(entry!=NULL) entry=FSFreeFile(entry);
return FS_CANTWRITE;
}

// GET FILE CLUSTER CHAIN
error=FSGetChain(entry->FirstCluster,&entry->Chain,fs);

if(error!=FS_OK) {
while(entry!=NULL) entry=FSFreeFile(entry);
return FS_ERROR;
}

if(entry->Attr&FSATTR_DIR) {					// FOR DIRECTORIES, SET FILE SIZE ACCORDING TO CHAIN
entry->FileSize=FSGetChainSize(&entry->Chain);

}

if((mode&(FSMODE_WRITE | FSMODE_APPEND | FSMODE_MODIFY))==FSMODE_WRITE) entry->FileSize=0; // TRUNCATE

if(mode&FSMODE_APPEND) entry->CurrentOffset=entry->FileSize;

if(entry->Attr&FSATTR_DIR) {					// FOR DIRECTORIES, SET FILE SIZE ACCORDING TO CHAIN
if(entry->FileSize>=65536*32) entry->Mode|=FSMODE_NOGROW;

}

// UPDATE FILE ACCESS DATES
unsigned int dtime,hundreth;
FSGetDateTime(&dtime,&hundreth);

entry->LastAccDate=dtime>>16;		// UPDATE LAST ACCESS DATE
if(mode&FSMODE_WRITE) entry->WriteTimeDate=dtime;	// UPDATE LAST WRITE DATE IF OPENED FOR WRITING


for(error=0;error<FS_MAXOPENFILES;++error)
{
if(fs->Files[error]==NULL) {
fs->Files[error]=entry;
break;
}
}

*fileptr=entry;
return FS_OK;

}
