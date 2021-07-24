/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

int FSClose(FS_FILE * file)
{
    int f, error;
    FS_VOLUME *fs;

    if(!FSystem.Init)
        return FS_ERROR;

    if(file == NULL)
        return FS_ERROR;

    if(file->Volume & (~3))
        return FS_ERROR;        // INVALID VOLUME --> FILE STRUCTURE CORRUPT
//printf("close buffers\n");
    error = FSFlushBuffers(file);
    if(error != FS_OK)
        return error;

    fs = FSystem.Volumes[file->Volume];
//printf("remove open list\n");
// REMOVE FROM OPEN FILE LIST
    for(f = 0; f < FS_MAXOPENFILES; ++f) {
        if(fs->Files[f] == file) {
            fs->Files[f] = NULL;
            break;
        }
    }
//printf("free structure\n");
    if(!FSFileIsReferenced(file, fs))   // FREE ALLOCATED MEMORY
        while((file = FSFreeFile(file)));
//printf("done close\n");
    return FS_OK;
}

#endif
