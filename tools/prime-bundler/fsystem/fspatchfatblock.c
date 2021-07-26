/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

// PATCH A BLOCK THAT CORRESPONDS TO THE FAT WITH THE CACHED FAT ENTRIES
// BLOCK ADDRESS IS EXPECTED TO BE WORD-ALIGNED, SIZE IS WORD-MULTIPLE

void FSPatchFATBlock(unsigned char *buffer, int size, int addr, FS_VOLUME * fs,
        int flush)
{
    FS_CHAINBUFFER *ch, *prev, *tmp;
    int f, claddr;
    unsigned short *ptr;
    unsigned int *ptr32;

    ch = fs->FATCache;

    switch (fs->FATType) {
    case 1:
// FAT12
        while(ch) {
            for(f = 0; f < ch->Used; ++f) {
                claddr = ch->Entries[f].Cluster +
                        (ch->Entries[f].Cluster >> 1) - addr;
                if(claddr >= 0 && claddr <= size - 2) {
// ENTRY IS FULLY WITHIN BLOCK
                    if(ch->Entries[f].Cluster & 1)
                        WriteInt16(buffer + claddr,
                                (ReadInt16(buffer +
                                        claddr) & 0xf) | (ch->Entries[f].
                                    EntryValue << 4));
                    else
                        WriteInt16(buffer + claddr,
                                (ReadInt16(buffer +
                                        claddr) & 0xf000) | (ch->Entries[f].
                                    EntryValue & 0xfff));
                    if(flush)
                        ch->Entries[f].EntryValue |= 0xc0000000;        // MARK AS FULLY WRITTEN
                }
                else {
// PARTIALLY INCLUDED ENTRIES
                    if(claddr == -1) {
// ONLY HIGH-BYTE IS WITHIN BLOCK
                        if(ch->Entries[f].Cluster & 1)
                            *buffer = ch->Entries[f].EntryValue >> 4;
                        else
                            *buffer =
                                    ((*buffer) & 0xf0) | ((ch->Entries[f].
                                        EntryValue >> 8) & 0xf);
                        if(flush)
                            ch->Entries[f].EntryValue |= 0x80000000;    // MARK AS PARTIALLY WRITTEN
                    }
                    if(claddr == size - 1) {
// ONLY LOW-BYTE IS WITHIN BLOCK
                        if(ch->Entries[f].Cluster & 1)
                            buffer[size - 1] =
                                    (buffer[size -
                                        1] & 0xf) | (ch->Entries[f].
                                    EntryValue << 4);
                        else
                            buffer[size - 1] = ch->Entries[f].EntryValue;
                        if(flush)
                            ch->Entries[f].EntryValue |= 0x40000000;    // MARK AS PARTIALLY WRITTEN
                    }
                }
            }
            ch = ch->Next;
        }
        break;

        break;

    case 2:

        ptr = (unsigned short int *)buffer;
// FAT16
        while(ch) {
            for(f = 0; f < ch->Used; ++f) {
                claddr = (ch->Entries[f].Cluster << 1) - addr;
                if(claddr >= 0 && claddr <= size - 2) {
// ENTRY IS FULLY WITHIN BLOCK
                    ptr[claddr >> 1] = ch->Entries[f].EntryValue;
                    if(flush)
                        ch->Entries[f].EntryValue |= 0xc0000000;        // MARK AS FULLY WRITTEN
                }
            }
            ch = ch->Next;
        }
        break;

    case 3:
// FAT32

        ptr32 = (unsigned int *)buffer;

        while(ch) {
            for(f = 0; f < ch->Used; ++f) {
                claddr = (ch->Entries[f].Cluster << 2) - addr;
                if(claddr >= 0 && claddr <= size - 4) {
// ENTRY IS FULLY WITHIN BLOCK
                    ptr32[claddr >> 2] =
                            (ptr32[claddr >> 2] & 0xf0000000) | (ch->Entries[f].
                            EntryValue & 0xfffffff);
                    if(flush)
                        ch->Entries[f].EntryValue |= 0xc0000000;        // MARK AS FULLY WRITTEN
                }
            }
            ch = ch->Next;
        }
        break;

    }

    if(flush) {
// ELIMINATE VALUES THAT WERE COMPLETELY WRITTEN
        ch = fs->FATCache;
        prev = NULL;
        while(ch) {
            for(f = ch->Used - 1, ch->Used = 0; f >= 0; --f) {
                if((ch->Entries[f].EntryValue & 0xc0000000) == 0xc0000000) {
// VALUE CAN BE ELIMINATED
                    for(claddr = 0; claddr < ch->Used; ++claddr) {
                        ch->Entries[f + claddr].Cluster =
                                ch->Entries[f + claddr + 1].Cluster;
                        ch->Entries[f + claddr].EntryValue =
                                ch->Entries[f + claddr + 1].EntryValue;
                    }

                }
                else
                    ++ch->Used;

            }

            if(!ch->Used) {
// ELIMINATE FROM CHAINBUFFER AND FREE
                if(prev)
                    prev->Next = ch->Next;
                else
                    fs->FATCache = ch->Next;
                tmp = ch;
                ch = ch->Next;
                --fs->NumCache;
                simpfree(tmp);
            }
            else {
                prev = ch;
                ch = ch->Next;
            }
        }

    }

    return;
}

#endif
