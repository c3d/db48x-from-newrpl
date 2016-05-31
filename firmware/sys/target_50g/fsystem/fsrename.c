/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"





int FSRename(char *oldname, char *newname)
{
FS_FILE *oldfile,*newfile;
int error;

error=FSInit();
if(error!=FS_OK) return error;

error=FSVolumePresent(FSystem.Volumes[FSystem.CurrentVolume]);
if(error!=FS_OK) { return error; }


error=FSOpen(oldname,FSMODE_READ,&oldfile);
if(error!=FS_OK) return error;




error=FSCreate(newname,oldfile->Attr,&newfile);
if(error!=FS_OK) {
FSClose(oldfile);
return error;
}

if(newfile->Volume!=oldfile->Volume) {
// CANNOT MOVE ACROSS VOLUMES

// TODO: DO A COMPLETE FILE COPY TO ANOTHER VOLUME
FSClose(oldfile);
//error=FSCloseAndDelete(newfile);
FSClose(newfile);
error=FSDelete(newname);

if(error!=FS_OK) return error;
return FS_BADVOLUME;
}



// COPY ALL PROPERTIES

newfile->Attr=oldfile->Attr;
newfile->CrtTmTenth=oldfile->CrtTmTenth;
newfile->LastAccDate=oldfile->LastAccDate;
newfile->CreatTimeDate=oldfile->CreatTimeDate;
newfile->WriteTimeDate=oldfile->WriteTimeDate;
newfile->FirstCluster=oldfile->FirstCluster;
oldfile->FirstCluster=0;
newfile->FileSize=oldfile->FileSize;
oldfile->FileSize=0;

memmoveb((void *)&(newfile->Chain),(void *)&(oldfile->Chain),sizeof(FS_FRAGMENT));
memsetb((void *)&(oldfile->Chain),0,sizeof(FS_FRAGMENT));

memsetb((void *)&(newfile->RdBuffer),0,sizeof(FS_BUFFER));
memsetb((void *)&(newfile->WrBuffer),0,sizeof(FS_BUFFER));


error=FSClose(newfile);
if(error!=FS_OK) return error;


//error=FSCloseAndDelete(oldfile);
FSClose(oldfile);
FSDelete(oldname);
return error;

};




