/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

#ifndef CONFIG_NO_FSYSTEM

static int isdot(char *string)
{
    if((string[0] == '.') && (string[1] == 0))
        return 1;
    return 0;
}

static int isdotdot(char *string)
{
    if((string[0] == '.') && (string[1] == '.') && (string[2] == 0))
        return 1;
    return 0;
}

// PARSE NAME, FIND FILE AND FILL THE ENTRY

int FSFindFile(char *name, FS_FILE * entry, int dirsvalid)
{
    char temp[257];
    int ntype, vol;
    int error, f;
    char *ptr, *ptrend;
    FS_FILE *startdir, *dir, *newdir;
    FS_VOLUME *fs;

    ntype = FSGetNameType(name);
//printf("name %s type=%02X\n",name,ntype);

    if(ntype < 0)
        return FS_BADNAME;

    if(!dirsvalid) {
        if((ntype & 8) || (ntype & 32))
            return FS_BADNAME;
    }

    ptr = name;

// OBTAIN VOLUME FOR THE FILE
    if(ntype & 1) {

        if(ntype & 16)
            ++ptr;      // HP-STYLE
        if((*ptr & 0xc0) == 0x40)
            vol = (*ptr & 31) - 3;      // DRIVE IS A LETTER
        else
            vol = *ptr - 51;
        if(vol < 0 || vol > 3)
            return FS_BADVOLUME;
        fs = FSystem.Volumes[vol];
        ptr += 2;       // POINT TO START OF NAME
    }
    else {
        fs = FSystem.Volumes[FSystem.CurrentVolume];
    }

    if(!fs)
        return FS_BADVOLUME;

// OBTAIN STARTING DIRECTORY

    if(ntype & 4) {
        startdir = &fs->RootDir;
        ++ptr;  // SKIP THE INITIAL SLASH
    }
    else
        startdir = fs->CurrentDir;

//printf("partial=%s\n",ptr);
    dir = startdir;

    if(ntype & 2) {
// CRAWL DIRECTORIES UNTIL WE REACH THE FINAL NAME
//printf("start crawl\n");
        while((ptrend = fsfindchar(ptr, NULL, "/\\"))) {
// EXTRACT DIRECTORY NAME
            memmoveb(temp, ptr, ptrend - ptr);
            temp[ptrend - ptr] = 0;
//printf("CD %s\n",temp);
            if(isdot(temp))     // IGNORE REFERENCES TO CURRENT DIR
            {
                ptr = ptrend + 1;
                continue;
            }
            if(isdotdot(temp)) {
                newdir = dir;
                dir = dir->Dir; // GET PARENT DIRECTORY WITHOUT SCANNING
                FSFreeFile(newdir);
                if(dir == NULL)
                    return FS_NOTFOUND; // GONE BEYOND ROOTDIR;
                ptr = ptrend + 1;
                continue;
            }

// CHECK FOR ALREADY OPENED DIRECTORIES IN THIS VOLUME
            for(f = 0; f < FS_MAXOPENFILES; ++f) {
// CHECK IF THIS FILE IS OR CONTAINS THE PATH WE NEED
                newdir = fs->Files[f];
                while(newdir) {
                    if(newdir->Dir == dir) {
// CHECK FOR SAME PARENT DIR AND SAME NAME
                        if((newdir->Name != NULL)
                                && FSNameCompare(temp, newdir->Name,
                                    FSystem.CaseMode)) {
                            f = FS_MAXOPENFILES;
                            break;
                        }
                    }
                    newdir = newdir->Dir;
                }
            }
// CHECK FOR CURRENTDIR
            if(!newdir) {
                newdir = fs->CurrentDir;
                while(newdir) {
                    if(newdir->Dir == dir) {
// CHECK FOR SAME PARENT DIR AND SAME NAME
                        if((newdir->Name != NULL)
                                && FSNameCompare(temp, newdir->Name,
                                    FSystem.CaseMode))
                            break;
                    }
                    newdir = newdir->Dir;
                }
            }

            if(!newdir) {
// IF IT'S NOT OPEN YET, SCAN THE DIRECTORY
                newdir = (FS_FILE *) simpmallocb(sizeof(FS_FILE));
                if(!newdir) {
                    while(dir != NULL)
                        dir = FSFreeFile(dir);
                    return FS_ERROR;
                }

// CLEAN ENTRY
                memsetb((void *)newdir, 0, sizeof(FS_FILE));

                error = FSFindEntry(temp, FSystem.CaseMode, newdir, dir);
                if(error != FS_OK) {
// CLEANUP PROCEDURE
                    while(dir != NULL)
                        dir = FSFreeFile(dir);
                    if(error == FS_EOF)
                        return FS_NOTFOUND;
                    return FS_ERROR;
                }
            }

            if(!(newdir->Attr & FSATTR_DIR))    // FILE FOUND BUT IT'S NOT A DIRECTORY
            {
                while(newdir != NULL)
                    newdir = FSFreeFile(newdir);
                return FS_NOTFOUND;
            }

            error = FSGetChain(newdir->FirstCluster, &newdir->Chain, fs);

            if(error != FS_OK) {
                while(newdir != NULL)
                    newdir = FSFreeFile(newdir);
                return FS_ERROR;
            }

            newdir->FileSize = FSGetChainSize(&newdir->Chain);
            if(fs->InitFlags & VOLFLAG_READONLY)
                newdir->Mode = FSMODE_READ | FSMODE_NOGROW;
            else {
                newdir->Mode = FSMODE_WRITE | FSMODE_MODIFY | FSMODE_READ;
                if(newdir->FileSize >= 65536 * 32)
                    newdir->Mode |= FSMODE_NOGROW;
            }
// DO NEXT DIR
            dir = newdir;
            ptr = ptrend + 1;

        }

    }

// HERE DIR IS THE DIRECTORY OF THE FILE
// ptr POINTS TO THE FILE NAME

//printf("final name=%s\n",ptr);

    if(isdot(ptr))      // NAME IS CURRENT DIR
    {
//printf("dot\n");
        ntype |= 34;
    }
    else if(isdotdot(ptr)) {
//printf("dotdot\n");
        newdir = dir;
        ntype |= 34;
        dir = dir->Dir; // GET PARENT DIRECTORY WITHOUT SCANNING
        FSFreeFile(newdir);
        if(dir == NULL)
            return FS_NOTFOUND; // GONE BEYOND ROOTDIR;
    }

    if(ntype & 32)      // NAME IS EMPTY
    {

        if(!dirsvalid) {
            while(dir != NULL)
                dir = FSFreeFile(dir);
            return FS_NOTFOUND;
        }
//printf("empty name w/dirsvalid\n");
        if(!(ntype & 2)) {
// EMPTY STRING PASSED AS ARGUMENT
//printf("null name\n");
            while(dir != NULL)
                dir = FSFreeFile(dir);
            return FS_NOTFOUND;
        }
// RETURN DIRECTORY FOUND
        if(FSFileIsReferenced(dir, fs)) {
// RETURN EXISTING DIRECTORY
//printf("existing dir returned\n");
            memsetb((void *)entry, 0, sizeof(FS_FILE));
            entry->Dir = dir;
            return FS_OPENDIR;
        }
// RETURN NEWLY CREATED DIRECTORY
//printf("new dir returned\n");
        memmoveb(entry, dir, sizeof(FS_FILE));
        simpfree(dir);
        return FS_OK;
    }

// CHECK TO SEE IF THIS FILE IS ALREADY OPEN OR REFERENCED
    for(f = 0; f <= FS_MAXOPENFILES; ++f) {
        if(f == FS_MAXOPENFILES)
            newdir = fs->CurrentDir;
// CHECK IF THIS FILE IS OR CONTAINS THE PATH WE NEED
        else
            newdir = fs->Files[f];
        while(newdir) {
            if(newdir->Dir == dir) {
// CHECK FOR SAME PARENT DIR AND SAME NAME
                if((newdir->Name != NULL)
                        && FSNameCompare(ptr, newdir->Name, FSystem.CaseMode)) {
                    f = FS_MAXOPENFILES + 1;
                    break;
                }
            }
            newdir = newdir->Dir;
        }
    }

    if(newdir) {
// FILE IS OPEN, RETURN CURRENT ENTRY
        memsetb((void *)entry, 0, sizeof(FS_FILE));
// RETURN NOT FOUND IF FILE IS A DIRECTORY
        if(!dirsvalid && (newdir->Attr & FSATTR_DIR))
            return FS_NOTFOUND;

        entry->Dir = newdir;
        return FS_OPENDIR;

    }

    error = FSFindEntry(ptr, FSystem.CaseMode, entry, dir);

//printf("findentry returned %d\n",error);
    if(error != FS_OK) {
// CLEANUP PROCEDURE
        while(dir != NULL)
            dir = FSFreeFile(dir);
        if(error == FS_EOF)
            return FS_NOTFOUND;
        return FS_ERROR;
    }

    if(!dirsvalid && (entry->Attr & FSATTR_DIR)) {
        simpfree(entry->Name);
        memsetb((void *)entry, 0, sizeof(FS_FILE));
        while(dir != NULL)
            dir = FSFreeFile(dir);
        return FS_NOTFOUND;
    }

    return FS_OK;

}

#endif
