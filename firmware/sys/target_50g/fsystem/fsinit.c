/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"


// INITIALIZE FILESYSTEM

typedef void (*myfunctype)(void);



int FSInit()
{
int f;
FS_VOLUME *fs;

if(FSystem.Init) return FS_OK;

SD_CARD *dsk=(SD_CARD *)malloc(sizeof(SD_CARD));
if(dsk==NULL) return FS_ERROR;

fs=(FS_VOLUME *)malloc(sizeof(FS_VOLUME));
if(fs==NULL) { free(dsk); return FS_ERROR; }

if(!SDCardInit(dsk)) { free(fs); free(dsk); return FS_NOCARD; }
// SELECT CARD
if(!SDSelect(dsk->Rca)) { SDIOSetup(dsk,TRUE);  free(fs); free(dsk); return FS_ERROR; }

memset((void *) &FSystem.Volumes,0,4*sizeof(FS_VOLUME *));
FSystem.CurrentVolume=0xff;
FSystem.CaseMode=FSCASE_SENSHP;
// DO MULTIPLE MOUNT
for(f=0;f<4;++f)
{
if(FSMountVolume(dsk,fs,f))
{
if(FSystem.CurrentVolume==0xff) FSystem.CurrentVolume=f;
FSystem.Volumes[f]=fs;
fs=(FS_VOLUME *)malloc(sizeof(FS_VOLUME));
if(fs==NULL) break;
}
}
if(fs) free(fs);

if(FSystem.CurrentVolume==0xff) {
// NO VOLUMES WERE MOUNTED
SD_CARD temp;
SDPowerDown();
SDIOSetup(&temp,FS_OK);
FSystem.Init=0;
free(dsk);
return FS_ERROR;
}
FSystem.Init=1;
atexit((myfunctype)&FSShutdown);

return FS_OK;
}


