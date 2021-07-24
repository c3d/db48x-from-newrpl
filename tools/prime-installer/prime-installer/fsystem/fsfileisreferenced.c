/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

// CHECK IF FILE IS REFERENCED, RETURNS TRUE/FALSE

int FSFileIsReferenced(FS_FILE * file, FS_VOLUME * fs)
{
    FS_FILE *ff;
    int f;

    if(file == &fs->RootDir)
        return TRUE;

    ff = fs->CurrentDir;
    while(ff != NULL) {
        if(ff == file)
            return TRUE;
        ff = ff->Dir;
    }

    for(f = 0; f < FS_MAXOPENFILES; ++f) {
        ff = fs->Files[f];
        while(ff != NULL) {
            if(ff == file)
                return TRUE;
            ff = ff->Dir;
        }
    }

    return FALSE;

}

#endif
