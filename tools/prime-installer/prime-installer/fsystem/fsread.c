/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

int FSRead(unsigned char *buffer, int nbytes, FS_FILE * file)
{
    FS_VOLUME *fs;
    int vol = 0;

// SAME AS LOW-LEVEL FUNCTION BUT W/SOME CHECKS

    if(!FSystem.Init)
        return FS_ERROR;

    if(file == NULL)
        return FS_ERROR;
    vol = file->Volume;
    if(vol > 3 || vol < 0)
        return FS_ERROR;        // INVALID VOLUME --> FILE STRUCTURE CORRUPT
    fs = FSystem.Volumes[vol];
    if(fs == NULL)
        return FS_ERROR;
    return FSReadLL(buffer, nbytes, file, fs);
}

#endif
