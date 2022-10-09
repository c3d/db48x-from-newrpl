/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

// FILLS IN A FS_FILE STRUCTURE WITH THE DIRECTORY ENTRY
// name MUST BE A VALID NAME (NO ABSOLUTE PATH ALLOWED, NO VALIDITY CHECK)
// caseflag== 0 --> CASE-SENSITIVE NAME COMPARISON
//             == 1 --> CASE-SENSITIVE W/SEMICOLON STRIPPING (CALCULATOR-FRIENDLY)
//         == 2 --> CASE-INSENSITIVE NAME COMPARISON
// returns FS_EOF if not found, FS_OK if found or FS_ERROR if there's a problem

int FSFindEntry(char *name, int caseflags, FS_FILE * entry, FS_FILE * dir)
{
    FS_VOLUME *fs;
    int order, diroffset;
    int namelen;
    int nentries;
    unsigned char checksum;
    unsigned char buffer[32], *morebuff;
    unsigned char *ptr;
    unsigned char shortn[35];   // 3*11+'.'+ '\000' CHARACTERS

    if(!FSystem.Init)
        return FS_ERROR;
    if(!dir || !entry)
        return FS_ERROR;

    fs = FSystem.Volumes[dir->Volume];

    if(!fs)
        return FS_ERROR;

// DETERMINE LENGTH OF STRING
    namelen = (int)strlen((char *)name);
//printf("Search... %s\n",name);
//printf("dir size=%d\n",dir->FileSize);
//keyb_get_keyM(1);
    FSSeek(dir, 0, FSSEEK_SET);

    while((checksum = FSReadLL(buffer, 32, dir, fs)) == 32) {
//printf("x");
        if(buffer[0] == 0)
            return FS_EOF;
        if(buffer[0] == 0xe5)
            continue;   // DELETED ENTRY, USE NEXT ENTRY
        if((buffer[11] & FSATTR_LONGMASK) == FSATTR_LONGNAME) {

//      printf("LFN entry found\n");
            if((fs->FATType != 3) && (((buffer[0x1a] | buffer[0x1b]) != 0)
                        || ((buffer[0x1c] | buffer[0x1d] | buffer[0x1e] |
                                buffer[0x1f]) == 0))) {
                // THIS IS NOT A VALID LFN ENTRY, POSSIBLY USED BY OTHER OS'S
                // JUST IGNORE IT
                continue;
            }

            // TREAT AS LONG FILENAME
            if(!(buffer[0] & 0X40))
                continue;       // ORPHAN ENTRY, SKIP
            diroffset = dir->CurrentOffset - 32;
//      printf("last entry\n");
            // FOUND LAST ENTRY OF A NAME
            nentries = buffer[0] & 0x3f;
            morebuff = simpmallocb(32 * nentries);
            if(morebuff == NULL) {
                return FS_ERROR;
            }
            if(FSReadLL(morebuff, 32 * nentries, dir, fs) != 32 * nentries) {
                simpfree(morebuff);
                if(FSEof(dir))
                    return FS_EOF;
                else
                    return FS_ERROR;
            }

//      printf("Read %d entries\n",nentries);
//      keyb_get_keyM(1);

//      printf("Checking validity\n");
            // VERIFY THAT ENTRIES ARE VALID
            ptr = morebuff;
            for(order = nentries - 1; (order != 0) && ((*ptr & 0x3f) == order);
                    --order, ptr += 32) {
                if(ptr[13] != buffer[13])
                    break;      // VERIFY CHECKSUM
            }
//      printf("entries valid\n");
            if(order) {
//              printf("failed entries checksum test\n");
//              keyb_get_keyM(1);
                // ENTRIES ARE ORPHANS, DISCARD AND CONTINUE SEARCHING
                FSSeek(dir, -32 * (order + 1), FSSEEK_CUR);     // REWIND TO NEXT UNKNOWN ENTRY
                simpfree(morebuff);
                continue;
            }
            // VERIFY THAT SHORT ENTRY FOLLOWS LONG NAME

            if(((ptr[11] & FSATTR_LONGMASK) == FSATTR_LONGNAME) || (*ptr == 0)
                    || (*ptr == 0xe5)) {
//      printf("no valid shortname follows\n");
//      keyb_get_keyM(1);

                // VALID SHORT ENTRY NOT FOUND
                simpfree(morebuff);
                if(*ptr == 0)
                    return FS_EOF;
                if(*ptr != 0xe5)
                    FSSeek(dir, -32, FSSEEK_CUR);       // REWIND LAST ENTRY
                continue;
            }

//      printf("calculating checksum\n");
            // CALCULATE CHECKSUM
            checksum = 0;
            for(order = 0; order < 11; ++order, ++ptr) {
//      printf("%c",*ptr);
                checksum =
                        (((checksum << 7) & 0x80) | ((checksum >> 1) & 0x7f)) +
                        *ptr;
            }

//      printf("Calc. checksum=%02X\n",checksum);
            if(checksum != buffer[13]) {
                // FAILED CHECKSUM, SKIP ORPHANS AND CONTINUE
//      printf("failed checksum\n");
//      keyb_get_keyM(1);
                simpfree(morebuff);
                FSSeek(dir, -32, FSSEEK_CUR);   // REWIND LAST ENTRY
                continue;
            }
//      printf("All valid!!!\n");

            // VALID ENTRY FOUND, FILL STRUCTURE AND RETURN
            entry->Name = (char *)simpmallocb(nentries * 13 * 3 + 1);   // ALLOCATE 3-BYTES PER CHARACTER (MINIMUM FOR UCS-2 TO UTF-8 ENCODING)
            if(entry->Name == NULL) {
                simpfree(morebuff);
                return FS_ERROR;
            }

            ptr -= 11;
            // REPACK LONG NAME
            char *nameptr = entry->Name;
            for(order = 1; order < nentries; ++order) {
                nameptr = FSPackName(nameptr, (char *)ptr - (order << 5));
            }
            nameptr = FSPackName(nameptr, (char *)buffer);
            *nameptr = 0;       // FORCE NULL-TERMINATED STRING

            memmoveb(buffer, ptr, 32);  // COPY MAIN (SHORT) ENTRY TO buffer
            simpfree(morebuff);

            //printf("Comparing=%s\n",entry->Name);
            if(!FSNameCompare(entry->Name, name, caseflags)) {
                // NOT THIS FILE, COMPARE SHORT FILENAME
                if(namelen > 12)        // CANNOT BE A SHORT ENTRY
                {
                    simpfree(entry->Name);
                    continue;
                }
                // CHECK IF NAME IS GIVEN BY SHORT ENTRY
                FSPackShortName((char *)shortn, (char *)buffer);
                //printf("Comparing=%s\n",shortn);
                if(!FSNameCompare((char *)shortn, name, caseflags)) {
                    // NOT THIS ENTRY
                    //printf("Not this one...\n");
//      keyb_get_keyM(1);
                    simpfree(entry->Name);
                    continue;
                }

            }

        }

        else {
            // IT'S A SHORT NAME ENTRY
            if(namelen > 12)
                continue;       // CANNOT BE A SHORT NAME
            diroffset = dir->CurrentOffset - 32;
            nentries = 0;
            entry->Name = (char *)simpmallocb(35);
            if(entry->Name == NULL)
                return FS_ERROR;

            FSPackShortName(entry->Name, (char *)buffer);

            //printf("Comparing %s\n",entry->Name);

            if(!FSNameCompare(entry->Name, name, caseflags)) {
                //printf("Not this one...(short)\n");
//      keyb_get_keyM(1);

                simpfree(entry->Name);
                continue;

            }
        }

        // NOW FILL THE COMMON FIELDS
        if(caseflags == FSCASE_SENSHP)
            FSStripSemi(entry->Name);
        entry->Mode = 0;
        entry->Volume = fs->VolNumber;
        entry->Attr = buffer[11];
        entry->NTRes = buffer[12];
        entry->CrtTmTenth = buffer[13];
        entry->LastAccDate = ReadInt16(buffer + 18);
        entry->CreatTimeDate = ReadInt32(buffer + 14);
        entry->WriteTimeDate = ReadInt32(buffer + 22);
        entry->FirstCluster = buffer[26] + (buffer[27] << 8);
        if(fs->FATType == 3)
            entry->FirstCluster |= (buffer[20] << 16) + (buffer[21] << 24);
        entry->FileSize = ReadInt32(buffer + 28);
        entry->CurrentOffset = 0;
        entry->DirEntryOffset = diroffset;
        entry->DirEntryNum = nentries + 1;
        entry->Dir = dir;
        memsetb((void *)&(entry->Chain), 0, sizeof(FS_FRAGMENT));
        memsetb((void *)&(entry->RdBuffer), 0, sizeof(FS_BUFFER));
        memsetb((void *)&(entry->WrBuffer), 0, sizeof(FS_BUFFER));
        return FS_OK;
    }

    if(dir->CurrentOffset >= dir->FileSize)
        return FS_EOF;
    else {
        return FS_ERROR;
    }
}

#endif
