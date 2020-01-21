/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

// FAT CONVERSION FUNCTIONS

unsigned int FSAddr2Cluster(unsigned int addr, FS_VOLUME * fs)
{
    return (addr - fs->Cluster0Addr) >> (fs->ClusterSize - 9);
}

unsigned int FSCluster2FATEntry(unsigned int cluster, FS_VOLUME * fs)
{
    switch (fs->FATType) {
    case 1:    // FAT12
        return cluster + (cluster >> 1);
    case 3:    // FAT 32
        cluster <<= 1;
// DELIBERATE FALL THRU
    case 2:
        return (cluster << 1);
    }
    return 0;
}

unsigned int FSAddr2FATEntry(unsigned int addr, FS_VOLUME * fs)
{
    return FSCluster2FATEntry(FSAddr2Cluster(addr, fs), fs);
}

unsigned int FSCluster2Addr(unsigned int cluster, FS_VOLUME * fs)
{
    return (cluster << (fs->ClusterSize - 9)) + fs->Cluster0Addr;
}

unsigned int FSFATEntry2Cluster(unsigned int addr, FS_VOLUME * fs)
{
    switch (fs->FATType) {
    case 1:    // FAT12
        ++addr;
        return (addr * 2) / 3;
    case 3:    // FAT 32
        return addr >> 2;
    case 2:
        return addr >> 1;
    }
    return 0;
}

unsigned int FSFATEntry2Addr(unsigned int addr, FS_VOLUME * fs)
{
    return FSCluster2Addr(FSFATEntry2Cluster(addr, fs), fs);
}

#endif
