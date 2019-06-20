/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"


#ifndef CONFIG_NO_FSYSTEM



int FSChAttr(FS_FILE *file,int newattr)
{
int error;

if(!FSystem.Init) return FS_ERROR;

if(file==NULL) return FS_ERROR;

if(file->Volume&(~3)) return FS_ERROR;		// INVALID VOLUME --> FILE STRUCTURE CORRUPT


if( (file->Mode&FSMODE_WRITE) && (newattr&FSATTR_RDONLY)) {

// FINISH A WRITING SESSION

error=FSFlushBuffers(file);
if(error!=FS_OK) { return error; }

file->Mode&=~(FSMODE_WRITE|FSMODE_MODIFY|FSMODE_APPEND);	// DISABLE ALL WRITE MODES

}

file->Attr=newattr;

return FS_OK;
}


#endif
