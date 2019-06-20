/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"



#ifndef CONFIG_NO_FSYSTEM



int FSCloseAndDelete(FS_FILE *file)
{
int f,error;
FS_VOLUME *fs;

if(!FSystem.Init) return FS_ERROR;

if(file==NULL) return FS_ERROR;

if(file->Volume&(~3)) return FS_ERROR;		// INVALID VOLUME --> FILE STRUCTURE CORRUPT


error=FSVolumePresent(FSystem.Volumes[file->Volume]);
if(error!=FS_OK) { return error; }


fs=FSystem.Volumes[file->Volume];





// REMOVE FROM OPENED FILE LIST
//printf("Remove from open list\n");

for(f=0;f<FS_MAXOPENFILES;++f)
{
if(fs->Files[f]==file) { fs->Files[f]=NULL; break; }
}


// DO NOT DELETE IF DIRECTORY IS BEING REFERENCED
if(FSFileIsReferenced(file,fs)) {
// DO NOT DELETE, STANDARD CLOSE
return FSClose(file);
}


error=FSTruncateChain(file,0);	// TRUNCATE CHAIN UNLESS IN MODIFY MODE

// FLUSH ALL FAT CHAINS
error=FSFlushFATCache(fs);
if(error!=FS_OK) {
return error;
}

error=FSDeleteDirEntry(file);			// MARK ENTRY AS DELETED
if(error!=FS_OK) {
return error;
}


//printf("Free file\n");
// FREE ALLOCATED MEMORY
while((file=FSFreeFile(file)));
//printf("free done\n");
return FS_OK;
}

#endif
