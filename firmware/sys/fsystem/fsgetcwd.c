/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

char *FSGetcwd(int Volume)
{
    FS_VOLUME *fs;

    int error = FSInit();
    if(error != FS_OK)
        return NULL;

    fs = FSystem.Volumes[Volume];
    if(!fs)
        return NULL;

    return FSGetFileName(fs->CurrentDir, FSNAME_HASPATH | FSNAME_ABSPATH);

}
#endif
