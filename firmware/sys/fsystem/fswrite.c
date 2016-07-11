/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"




// WRITE DATA TO A FILE

int FSWrite(unsigned char *buffer,int nbytes,FS_FILE *file)
{


if(!FSystem.Init) return FS_ERROR;

if(!file) return FS_ERROR;

if(!(file->Mode&FSMODE_WRITE)) return FS_ERROR;	// WRITE NOT ALLOWED
if(file->Attr&FSATTR_RDONLY) return FS_ERROR;

if(file->Mode&FSMODE_APPEND) file->CurrentOffset=file->FileSize;		// ONLY AT THE END OF FILE

// CHANGE THIS WITH BUFFERED VERSION WHEN AVAILABLE
nbytes=FSWriteLL(buffer,nbytes,file,FSystem.Volumes[file->Volume]);

if(!(file->Mode&FSMODE_MODIFY)) {
file->FileSize=file->CurrentOffset;		// TRUNCATE FILE
}

return nbytes;
}
