/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"


// LOW-LEVEL WRITE FUNCTION

int FSWriteLL(unsigned char *buffer,int nbytes,FS_FILE *file,FS_VOLUME *fs)
{
int currentaddr,bytesread,totalcount,error;
FS_FRAGMENT *fr;



if(file->CurrentOffset+nbytes>file->FileSize)  {
if(file->Mode&FSMODE_NOGROW) return FS_DISKFULL;		// FILE EXPANSION NOT ALLOWED

error=FSExpandChain(file,file->CurrentOffset+nbytes);

if(error!=FS_OK)
return error;
}

fr=&file->Chain;
currentaddr=fr->StartAddr+file->CurrentOffset;

// FIND STARTING ADDRESS
while (currentaddr>=fr->EndAddr) {
	currentaddr-=fr->EndAddr;
	fr=fr->NextFragment;
	if(fr==NULL) return FS_ERROR;
	currentaddr+=fr->StartAddr;
}


// START READING FULL FRAGMENTS
totalcount=0;


// INVALIDATE READ BUFFERS
if(file->RdBuffer.Used) file->RdBuffer.Used=0;


// SET WRITE BLOCK LENGTH
//if(!SDDSetBlockLen(fs->Disk,fs->Disk->WriteBlockLen)) {
// ERROR
//return FS_ERROR;
//}

while(nbytes+currentaddr>fr->EndAddr) {


bytesread=SDDWrite(currentaddr,fr->EndAddr-currentaddr,buffer, fs->Disk);

totalcount+=bytesread;
file->CurrentOffset+=bytesread;
// ADJUST FILE SIZE, GROW BUT DON'T TRUNCATE
if(file->CurrentOffset>file->FileSize) file->FileSize=file->CurrentOffset;

if(bytesread!=fr->EndAddr-currentaddr) {
// ERROR WRITING LAST BLOCK, RETURN WHAT WAS READ SO FAR

return totalcount;
}
nbytes-=bytesread;
fr=fr->NextFragment;
	if(fr==NULL) {
			// MALFORMED CLUSTER CHAIN!!! CLUSTER CHAIN IS SHORTER THAN FileSize
			return totalcount;
	}
currentaddr=fr->StartAddr;
buffer+=bytesread;
}

if(nbytes) {
bytesread=SDDWrite(currentaddr,nbytes,buffer, fs->Disk);

totalcount+=bytesread;
file->CurrentOffset+=bytesread;
// ADJUST FILE SIZE, GROW BUT DON'T TRUNCATE
if(file->CurrentOffset>file->FileSize) file->FileSize=file->CurrentOffset;
}

return totalcount;

}

