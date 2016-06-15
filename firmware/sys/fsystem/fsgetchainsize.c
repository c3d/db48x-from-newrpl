/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"




// GET CLUSTER CHAIN SIZE

int FSGetChainSize(FS_FRAGMENT *fr)
{
int size=0;
while(fr!=NULL) {
size+=fr->EndAddr-fr->StartAddr;
fr=fr->NextFragment;
}
return size;
}
