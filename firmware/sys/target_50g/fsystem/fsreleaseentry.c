/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"




// CLEANUP STATICALLY CREATED FS_FILE STRUCTURES
void FSReleaseEntry(FS_FILE *file)
{
FSFreeChain(file);
if(file->Name) {
free(file->Name);
file->Name=NULL;
}
}
