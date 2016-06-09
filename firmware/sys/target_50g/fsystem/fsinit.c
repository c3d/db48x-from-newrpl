/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"


// INITIALIZE FILESYSTEM

typedef void (*myfunctype)(void);

void FSHardReset()
{
   memsetw(&FSystem,0,sizeof(FS_PUBLIC)>>2);
   init_simpalloc();
}

int FSInit()
{
int f;
FS_VOLUME *fs;

if(FSystem.Init) return FS_OK;

SD_CARD *dsk=(SD_CARD *)simpmallocb(sizeof(SD_CARD));
if(dsk==NULL) return FS_ERROR;

fs=(FS_VOLUME *)simpmallocb(sizeof(FS_VOLUME));
if(fs==NULL) { simpfree(dsk); return FS_ERROR; }

if(!SDCardInit(dsk)) { FSystem.CurrentVolume=dsk->SysFlags; simpfree(fs); simpfree(dsk); return FS_NOCARD; }
// SELECT CARD
if(!SDSelect(dsk->Rca)) { SDIOSetup(dsk,TRUE);  simpfree(fs); simpfree(dsk); return FS_ERROR; }

memsetb((void *) &FSystem.Volumes,0,4*sizeof(FS_VOLUME *));
FSystem.CurrentVolume=0xff;
FSystem.CaseMode=FSCASE_SENSHP;
// DO MULTIPLE MOUNT
for(f=0;f<4;++f)
{
if(FSMountVolume(dsk,fs,f))
{
if(FSystem.CurrentVolume==0xff) FSystem.CurrentVolume=f;
FSystem.Volumes[f]=fs;
fs=(FS_VOLUME *)simpmallocb(sizeof(FS_VOLUME));
if(fs==NULL) break;
}
}
if(fs) simpfree(fs);

if(FSystem.CurrentVolume==0xff) {
// NO VOLUMES WERE MOUNTED
SD_CARD temp;
SDPowerDown();
SDIOSetup(&temp,FS_OK);
FSystem.Init=0;
simpfree(dsk);
return FS_ERROR;
}
FSystem.Init=1;

// MODIFIED FOR NEWRPL: THERE'S NO EXIT ON FIRMWARE
//atexit((myfunctype)&FSShutdown);

return FS_OK;
}


