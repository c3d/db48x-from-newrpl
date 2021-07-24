/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

// GET CLUSTER CHAIN SIZE

unsigned int FSGetChainSize(FS_FRAGMENT * fr)
{
    int size = 0;
    while(fr != NULL) {
        size += fr->EndAddr - fr->StartAddr;
        fr = fr->NextFragment;
    }
    return size << 9;
}

#endif
