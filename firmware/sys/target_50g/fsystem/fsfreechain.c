/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"




// FREE CLUSTER CHAIN
void FSFreeChain(FS_FILE *file)
{
FS_FRAGMENT *fr,*temp;

fr=file->Chain.NextFragment;
while(fr!=NULL)
{
temp=fr;
fr=fr->NextFragment;
free(temp);
}
file->Chain.NextFragment=NULL;
file->Chain.StartAddr=file->Chain.EndAddr=0;
}
