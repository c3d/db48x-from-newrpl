/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/


#include "fsyspriv.h"



// FAT CONVERSION FUNCTIONS

int FSAddr2Cluster(int addr,FS_VOLUME *fs)
{
return  (addr-fs->Cluster0Addr)>>(fs->ClusterSize);
}

int FSCluster2FATEntry(int cluster,FS_VOLUME *fs)
{
switch(fs->FATType)
{
case 1:	// FAT12
return cluster+(cluster>>1)+fs->FirstFATAddr;
case 3:	// FAT 32
cluster<<=1;
case 2:
return (cluster<<1)+fs->FirstFATAddr;
}
return 0;
}

int FSAddr2FATEntry(int addr,FS_VOLUME *fs)
{
return FSCluster2FATEntry(FSAddr2Cluster(addr,fs),fs);
}

int FSCluster2Addr(int cluster,FS_VOLUME *fs)
{
return (cluster<<fs->ClusterSize)+fs->Cluster0Addr;
}


int FSFATEntry2Cluster(int addr,FS_VOLUME *fs)
{
addr-=fs->FirstFATAddr;
switch(fs->FATType)
{
case 1:	// FAT12
++addr;
return (addr*2)/3;
case 3:	// FAT 32
return addr>>2;
case 2:
return addr>>1;
}
return 0;
}

int FSFATEntry2Addr(int addr,FS_VOLUME *fs)
{
return FSCluster2Addr(FSFATEntry2Cluster(addr,fs),fs);
}
