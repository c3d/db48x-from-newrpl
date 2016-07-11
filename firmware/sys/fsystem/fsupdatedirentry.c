/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"





// UPDATE DIRECTORY ENTRY IF FILE WAS MODIFIED
// DOES NOT MODIFY NAME, ONLY PROPERTIES

int FSUpdateDirEntry(FS_FILE *file)
{
unsigned char *buffer,*mainentry;


buffer=simpmallocb(32*file->DirEntryNum);
if(!buffer) return FS_ERROR;

FSSeek(file->Dir,file->DirEntryOffset,FSSEEK_SET);

if(FSReadLL(buffer,file->DirEntryNum<<5,file->Dir,FSystem.Volumes[file->Volume])!=file->DirEntryNum<<5)
{
simpfree(buffer);
return FS_ERROR;
}

mainentry=buffer+32*(file->DirEntryNum-1);
// write new properties
mainentry[11]=file->Attr;
//mainentry[12]=file->NTRes;
mainentry[13]=file->CrtTmTenth;
WriteInt32(mainentry+14,file->CreatTimeDate);
WriteInt16(mainentry+18,file->LastAccDate);
WriteInt16(mainentry+20,file->FirstCluster>>16);
WriteInt16(mainentry+26,file->FirstCluster);
WriteInt32(mainentry+28,(file->Attr&FSATTR_DIR)? 0:file->FileSize);
WriteInt32(mainentry+22,file->WriteTimeDate);

FSSeek(file->Dir,file->DirEntryOffset,FSSEEK_SET);

if(FSWriteLL(buffer,file->DirEntryNum<<5,file->Dir,FSystem.Volumes[file->Volume])!=file->DirEntryNum<<5)
{
simpfree(buffer);
return FS_ERROR;
}

simpfree(buffer);
return FS_OK;

}

