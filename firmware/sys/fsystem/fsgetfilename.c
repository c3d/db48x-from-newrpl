/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

// RETURN TYPE OF PATH
// 1 == Name include drive
// 2 == Name include path
// 4 == Path is absolute
// 8 == File ends in slash
// 16== Drive is HP style
// 32== Name is empty
// BITS 16-31 = DEPTH OF PATH (0=CURRENT DIR OR ROOT)


char *FSGetFileName(FS_FILE *file,int flags)
{
char *ptr,*partial,*tmp;
FS_FILE *dir,*enddir=NULL;
int length=1;

// COMPUTE STRING LENGTH

if( !(flags&FSNAME_EMPTY) && (file->Name!=NULL)) length+=(int)stringlen(file->Name);

if(flags&FSNAME_HASPATH) {
if(!(flags&FSNAME_ABSPATH)) enddir=FSystem.Volumes[file->Volume]->CurrentDir;
dir=file->Dir;
while(dir && (dir!=enddir))
{
if(dir->Name!=NULL) length+=(int)stringlen(dir->Name)+1;
dir=dir->Dir;
}
if(dir==NULL) flags|=FSNAME_ABSPATH;	// FORCE ABSOLUTE PATH
}

if(flags&FSNAME_ABSPATH) ++length;
if(flags&FSNAME_ENDSLASH) ++length;
if(flags&FSNAME_HASVOL) {
length+=2;
if(flags&FSNAME_VOLHP) ++length;
}

// ALLOCATE THE STRING
ptr=(char *)simpmallocb(length);
if(!ptr) return NULL;

// GENERATE NAME
partial=ptr;

// VOLUME INFORMATION
if(flags&FSNAME_HASVOL) {
if(flags&FSNAME_VOLHP) { *partial=':'; ++partial; }
partial[0]='3'+file->Volume;
partial[1]=':';
partial+=2;
}

// STARTING SLASH IF ABSOLUTE PATH

if(flags&FSNAME_ABSPATH) { *partial='\\'; ++partial; }

*partial=0;

if(flags&FSNAME_HASPATH) {
dir=file->Dir;
while(dir && (dir!=enddir))
{
if(dir->Name!=NULL) {
// MAKE ROOM
memmoveb(partial+stringlen(dir->Name)+1,partial,stringlen(partial)+1);
stringcpy(partial,dir->Name);
tmp=partial;
while(*tmp!=0) ++tmp;
*tmp='\\';
}
dir=dir->Dir;
}
while(*partial) ++partial;
}


if(!(flags&FSNAME_EMPTY))
{
if(file->Name!=NULL) stringcpy(partial,file->Name);
while(*partial) ++partial;
}

if(flags&FSNAME_ENDSLASH) {
if(*(partial-1)!='\\') {
*partial='\\';
++partial;
}
}
else {
if(*(partial-1)=='\\') --partial;
}
*partial=0;

return ptr;
}
#endif
