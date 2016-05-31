/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"


// CLEANUP FS_FILE STRUCTURES
FS_FILE *FSFreeFile(FS_FILE *file)
{
FS_FILE *orig=file->Dir;
FS_VOLUME *fs;

if(file==NULL) {
//printf("Trying to Free() NULL\n");
return NULL;
}

if(file->Volume&(~3)) {
//printf("Invalid volume at simpfree()\n");
return NULL;
}

// CHECK FOR REFERENCES BEFORE DELETING
fs=FSystem.Volumes[file->Volume];

if(!fs) return NULL;

if(FSFileIsReferenced(file,fs)) return NULL;

// NOT REFERENCED
// IT'S SAFE TO DELETE THIS FILE OBJECT

// FREE ENTIRE CLUSTER CHAIN
//printf("fffree cluster chain\n");
FSFreeChain(file);

// FREE READ/WRITE BUFFERS

if(file->RdBuffer.Data) simpfree(file->RdBuffer.Data);
if(file->WrBuffer.Data) simpfree(file->WrBuffer.Data);


//printf("done\n");
// FREE NAME
if(file->Name) simpfree(file->Name);
//printf("done free name\n");
// FREE FS_FILE
simpfree(file);
//printf("done free file\n");
return orig;
}
