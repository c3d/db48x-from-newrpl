/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"




// END-OF-FILE DETECTION
int FSEof(FS_FILE *file)
{
return (file->CurrentOffset>=file->FileSize)? TRUE:FALSE;
}
