/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"




int FSMkdir(char *name)
{
int error;
int parentcluster;
FS_FILE *newdir;
char *buffer,*mainentry;

buffer=(char *)simpmallocb(512);
if(!buffer) return FS_ERROR;


// CREATE THE FILE
error=FSCreate(name,FSATTR_DIR,&newdir);
if(error!=FS_OK) { simpfree(buffer); return error; }


error=FSExpandChain(newdir,96);
if(error!=FS_OK) { simpfree(buffer); return error; }

newdir->FileSize=FSGetChainSize(&newdir->Chain);
newdir->Mode=FSMODE_MODIFY | FSMODE_WRITE;
mainentry=buffer;

mainentry[0]='.';
memsetb(mainentry+1,32,10);
// write new properties
mainentry[11]=FSATTR_DIR;
mainentry[12]=0;
mainentry[13]=newdir->CrtTmTenth;
WriteInt32((char *)mainentry+14,newdir->CreatTimeDate);
WriteInt16((char *)mainentry+18,newdir->LastAccDate);
WriteInt16((char *)mainentry+20,newdir->FirstCluster>>16);
WriteInt32((char *)mainentry+22,newdir->WriteTimeDate);
WriteInt16((char *)mainentry+26,newdir->FirstCluster);
WriteInt32((char *)mainentry+28,0);

mainentry+=32;
memsetb(mainentry+2,32,9);
mainentry[0]='.';
mainentry[1]='.';
// write new properties
mainentry[11]=FSATTR_DIR;
mainentry[12]=0;
mainentry[13]=newdir->CrtTmTenth;
WriteInt32((char *)mainentry+14,newdir->CreatTimeDate);
WriteInt16((char *)mainentry+18,newdir->LastAccDate);

if(newdir->Dir->Dir!=NULL) parentcluster=newdir->Dir->FirstCluster;
else parentcluster=0;	// MARK ROOT DIR AS CLUSTER 0 EVEN FOR FAT32


WriteInt16((char *)mainentry+20,parentcluster>>16);
WriteInt32((char *)mainentry+22,newdir->WriteTimeDate);
WriteInt16((char *)mainentry+26,parentcluster);
WriteInt32((char *)mainentry+28,0);

mainentry+=32;
memsetb(mainentry,0,448);

error=FSWrite(buffer,512,newdir);

if(error!=512) { simpfree(buffer); FSClose(newdir); return error; }

memsetb(buffer,0,64);
while(!FSEof(newdir)) {
error=FSWrite(buffer,512,newdir);
if(error!=512) { simpfree(buffer); FSClose(newdir); return error; }
}

simpfree(buffer);
return FSClose(newdir);


}

