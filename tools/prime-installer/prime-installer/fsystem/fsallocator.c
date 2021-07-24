/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"

#include <stdlib.h>

unsigned int *simpmalloc(int words)
{
return malloc(words*4);
}

void simpfree(void *voidptr)
{
    free(voidptr);
}

unsigned char *simpmallocb(int bytes)
{
    return (unsigned char *)simpmalloc((bytes + 3) >> 2);
}
