/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"


#ifndef CONFIG_NO_FSYSTEM




int FSDelete(char *name)
{
FS_FILE *entry;
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


error=FSFindFile(name,entry,FALSE);

if(error!=FS_OK) { 
simpfree(entry); return error;
}

fs=FSystem.Volumes[entry->Volume];


error=FSDeleteDirEntry(entry);			// MARK ENTRY AS DELETED
if(error!=FS_OK) {
while(entry) entry=FSFreeFile(entry);
return error;
}

error=FSGetChain(entry->FirstCluster,&entry->Chain,fs);
if(error!=FS_OK) {
while(entry) entry=FSFreeFile(entry);
return error;
}

error=FSTruncateChain(entry,0);		// FREE CLUSTER CHAIN
while(entry) entry=FSFreeFile(entry);

if(error!=FS_OK) {
return error;
}

return FSFlushFATCache(fs);

}



#endif
