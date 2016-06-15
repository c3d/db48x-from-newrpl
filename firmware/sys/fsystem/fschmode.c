/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"





int FSChMode(FS_FILE *file,int newmode)
{
int error;

if(!FSystem.Init) return FS_ERROR;

if(file==NULL) return FS_ERROR;

if(file->Volume&(~3)) return FS_ERROR;		// INVALID VOLUME --> FILE STRUCTURE CORRUPT


if(newmode& (FSMODE_APPEND | FSMODE_MODIFY)) newmode|=FSMODE_WRITE;

if( (file->Mode&FSMODE_WRITE) && (! (newmode&FSMODE_WRITE))) {

// FINISH WRITING SESSION AND GO READ-ONLY
error=FSFlushBuffers(file);
if(error!=FS_OK) { return error; }

}

if( (file->Attr&FSATTR_RDONLY) && (newmode&FSMODE_WRITE)) {
return FS_CANTWRITE;
}
file->Mode=newmode;

return FS_OK;
}

