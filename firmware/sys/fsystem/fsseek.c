/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"




// ASSUME SEEK_... CONSTANTS ARE DEFINED SOMEWHERE ELSE
// ACCEPTS SETTING OFFSET BEYOND END-OF-FILE

int FSSeek(FS_FILE *file,int Offset,int position)
{
int from;
from=0;
if(position==FSSEEK_END) from=file->FileSize;
if(position==FSSEEK_CUR) from=file->CurrentOffset;

from+=Offset;

if(from<0) from=0;
file->CurrentOffset=from;
return FS_OK;

}
