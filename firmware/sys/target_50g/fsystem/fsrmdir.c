/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"

static int isdot(char *string)
{
    if((string[0]=='.')&&(string[1]==0)) return 1;
    return 0;
}
static int isdotdot(char *string)
{
    if((string[0]=='.')&&(string[1]='.')&&(string[2]==0)) return 1;
    return 0;
}



int FSRmdir(char *name)
{
int error;
FS_FILE *dir;
FS_FILE entry;

error=FSOpenDir(name,&dir);

if(error!=FS_OK) return error;

while(error==FS_OK) {
error=FSGetNextEntry(&entry,dir);
if(error==FS_OK) {
if(!isdot(entry.Name) && !isdotdot(entry.Name)) {
FSReleaseEntry(&entry);
FSClose(dir);
return FS_USED;
}
FSReleaseEntry(&entry);
}
}

if(error!=FS_EOF) { FSClose(dir); return error; }

// DIRECTORY IS EMPTY, OK TO REMOVE

return FSCloseAndDelete(dir);

}

