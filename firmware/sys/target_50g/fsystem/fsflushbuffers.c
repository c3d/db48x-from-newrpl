/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"





int FSFlushBuffers(FS_FILE *file)
{
int error;
FS_VOLUME *fs;

// FINISH A WRITING SESSION

error=FSVolumePresent(FSystem.Volumes[file->Volume]);
if(error!=FS_OK) { return error; }


if(file->Mode&FSMODE_WRITE) {
// TO DO:
// FLUSH WRITE BUFFERS

if(!(file->Mode&FSMODE_MODIFY)) {
error=FSTruncateChain(file,file->FileSize);	// TRUNCATE CHAIN UNLESS IN MODIFY MODE
if(error!=FS_OK) return error;
}

}

// WRITE UPDATED DIRECTORY ENTRY IF FILE HAS CHANGED
if(file->Dir) {
error=FSUpdateDirEntry(file);
if(error!=FS_OK) return error;
}



fs=FSystem.Volumes[file->Volume];

// FLUSH ALL FAT CHAINS
error=FSFlushFATCache(fs);
if(error!=FS_OK) return error;


return FS_OK;
}
