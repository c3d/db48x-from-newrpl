/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"



// MARK DIRECTORY ENTRY AS DELETED

int FSDeleteDirEntry(FS_FILE *file)
{
char *buffer,*mainentry;
int f,bytesread;

buffer=(char *)simpmallocb(32*(file->DirEntryNum+1));
if(!buffer) return FS_ERROR;

FSSeek(file->Dir,file->DirEntryOffset,FSSEEK_SET);

bytesread=FSReadLL(buffer,file->DirEntryNum<<5,file->Dir,FSystem.Volumes[file->Volume]);
if(bytesread<(file->DirEntryNum<<5))
{
simpfree(buffer);
return FS_ERROR;
}


mainentry=buffer;

for(f=0;f<file->DirEntryNum;++f,mainentry+=32)
{
mainentry[0]=0xe5;
}


FSSeek(file->Dir,file->DirEntryOffset,FSSEEK_SET);

if(FSWriteLL(buffer,bytesread,file->Dir,FSystem.Volumes[file->Volume])!=bytesread)
{
simpfree(buffer);
return FS_ERROR;
}

simpfree(buffer);
return FS_OK;

}

