/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"





// RETURN FILE HANDLE NUMBER, GIVEN FS_FILE * POINTER

int FSGetHandle(FS_FILE *file)
{
FS_VOLUME *fs;
int f;

if(!FSystem.Init) return FS_INVHANDLE;

if(file==NULL) return FS_INVHANDLE;

if(file->Volume&(~3)) return FS_INVHANDLE;		// INVALID VOLUME --> FILE STRUCTURE CORRUPT

fs=FSystem.Volumes[file->Volume];

if(fs==NULL) return FS_INVHANDLE;

for(f=0;f<FS_MAXOPENFILES;++f)
{
if(fs->Files[f]==file) return (3+f)+file->Volume*0x10000;

}

return FS_INVHANDLE;
}


int FSGetFileFromHandle(int handle,FS_FILE **file)
{
int k=(handle&0xffff)-3;
if(k<0) return FS_INVHANDLE;
FS_VOLUME *fs=FSystem.Volumes[handle>>16];

if(fs==NULL) return FS_INVHANDLE;

FS_FILE *f=fs->Files[k];
if(f==NULL) return FS_INVHANDLE;

*file=f;
return FS_OK;

}


