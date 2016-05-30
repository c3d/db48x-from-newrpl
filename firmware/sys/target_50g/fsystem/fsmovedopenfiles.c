/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"


// UPDATE ENTRY OFFSET IN ALL OPENFILES/DIRS WHILE REPACKING A DIRECTORY

void FSMovedOpenFiles(FS_FILE *dir,int entryoffset,int newoffset,FS_VOLUME *fs)
{
FS_FILE *ff;
int f;


ff=fs->CurrentDir;
while(ff!=NULL) {
if(ff->Dir==dir) {
if(ff->DirEntryOffset==entryoffset) ff->DirEntryOffset=newoffset;
}
ff=ff->Dir;
} 

for(f=0;f<FS_MAXOPENFILES;++f)
{
ff=fs->Files[f];
while(ff!=NULL) {
if(ff->Dir==dir) {
if(ff->DirEntryOffset==entryoffset) ff->DirEntryOffset=newoffset;
}
ff=ff->Dir;
} 
}

return;

}
