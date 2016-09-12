/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"


// INITIALIZE FILESYSTEM

typedef void (*myfunctype)(void);



// HARD RESET USED ONLY DURING POWER ON SEQUENCE
void FSHardReset()
{
   memsetw(&FSystem,0,sizeof(FS_PUBLIC)>>2);
   init_simpalloc();
   SDIOSetup(NULL,0);   // INITIALIZE PINS AND CARD DETECTION INTERRUPTS
   if(!SDCardInserted()) SDIOSetup(NULL,1); // POWER OFF SD CONTROLLER IF NO CARD INSERTED
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

int FSIsInit()
{
    if(FSystem.Init) return 1;
    return 0;
}

int FSCardInserted()
{
    if(SDCardInserted()) return 1;
    else return 0;
}

int FSCardIsSDHC()
{
    if(!FSystem.Init) return 0;
    if(FSystem.Volumes[FSystem.CurrentVolume]->Disk->SysFlags&16) return 1;
    return 0;
}

int FSIsDirty()
{
    if(!FSystem.Init) return 0;

    int vol;

    for(vol=0;vol<4;++vol)
    {
        if(FSystem.Volumes[vol]) {
            if(FSystem.Volumes[vol]->NumCache) return 1;    // VOLUME NEEDS TO UPDATE THE FAT
            if((FSystem.Volumes[vol]->InitFlags&(VOLFLAG_UPDATEHINT|VOLFLAG_HINTDIRTY))==(VOLFLAG_UPDATEHINT|VOLFLAG_HINTDIRTY)) return 1; // VOLUME NEEDS TO UPDATE ITS HINTS
            int f;
            for(f=0;f<FS_MAXOPENFILES;++f) {
                if(FSystem.Volumes[vol]->Files[f]!=NULL) {
                    if(FSystem.Volumes[vol]->Files[f]->Mode&FSMODE_WRITE) return 1;  // VOLUME HAS OPEN FILES FOR WRITING
                }
            }
        }
    }
    return 0;
}
