/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

#define __READ_ONLY__    __attribute__ ((section(".rodata")))

const char *const FSErrorMsgArray[] = {
    "Invalid error",
// FS_OK
    "OK",
// FS_ERROR
    "Unknown/hardware/memory error",
// FS_EOF
    "End of file",
// FS_BADNAME
    "Invalid filename",
// FS_BADVOLUME
    "Inexistent/unmounted volume",
// FS_NOTFOUND
    "File not found",
// FS_CANTWRITE
    "Write error",
// FS_NOCARD
    "No card inserted",
// FS_CHANGED
    "Card was changed",
// FS_MAXFILES
    "No more avail. handles",
// FS_OPENDIR
// FS_OPENFILE
// FS_USED
    "File/dir is open",
// FS_DISKFULL
    "Disk full",
// FS_EXIST
    "File exists"
};

// RETURN ERROR MESSAGE
const char *FSGetErrorMsg(int errornum)
{
    if(errornum > 1 || errornum < -11)
        errornum = 2;
    return FSErrorMsgArray[2 - errornum];
}

#endif
