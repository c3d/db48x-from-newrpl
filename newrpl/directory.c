/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "hal.h"
#include "libraries.h"

extern const WORD const dir_start_bint[];
extern const WORD const dir_end_bint[];
extern const WORD const dir_parent_bint[];
extern const WORD const root_dir_handle[];

// GROW THE DIRECTORY REGION

void growDirs(WORD newtotalsize)
{
    WORDPTR *newdir;
    BINT gc_done = 0;
    do {
        newtotalsize = (newtotalsize + 1023) & ~1023;

        newdir = halGrowMemory(MEM_AREA_DIR, Directories, newtotalsize);

        if(!newdir) {
            if(!gc_done) {
                rplGCollect();
                ++gc_done;
            }
            else {
                rplException(EX_OUTOFMEM);
                return;
            }
        }

    }
    while(!newdir);

    CurrentDir = CurrentDir - Directories + newdir;
    DirsTop = DirsTop - Directories + newdir;
    Directories = newdir;
    DirSize = newtotalsize;
}

// SIMILAR TO GROW, BUT DOES NOT DO GARBAGE COLLECTION
// USED TO RELEASE PAGES RIGHT BEFORE A GC
void shrinkDirs(WORD newtotalsize)
{
    WORDPTR *newdir;

    newtotalsize = (newtotalsize + 1023) & ~1023;

    newdir = halGrowMemory(MEM_AREA_DIR, Directories, newtotalsize);

    if(!newdir) {
        rplException(EX_OUTOFMEM);
        return;
    }
    CurrentDir = CurrentDir - Directories + newdir;
    DirsTop = DirsTop - Directories + newdir;
    Directories = newdir;
    DirSize = newtotalsize;

}

// CHECK IF AN IDENT IS QUOTED, IF NOT THEN
// CREATE A NEW QUOTED OBJECT AND RETURN IT
// MAY CAUSE GARBAGE COLLECTION
// USES 1 SCRATCH POINTER
WORDPTR rplMakeIdentQuoted(WORDPTR ident)
{
    if(ISQUOTEDIDENT(*ident))
        return ident;

    ident = rplMakeNewCopy(ident);

    //  CHANGE FROM A IDENTEVAL TO A REGULAR IDENT
    if(ident)
        ident[0] &= ~MKOPCODE(UNQUOTED_BIT, 0);

    return ident;

}

// CHECK IF AN IDENT IS UNQUOTED, IF NOT THEN
// CREATE A NEW UNQUOTED OBJECT AND RETURN IT
// MAY CAUSE GARBAGE COLLECTION
WORDPTR rplMakeIdentUnquoted(WORDPTR ident)
{
    if(ISUNQUOTEDIDENT(*ident))
        return ident;

    ident = rplMakeNewCopy(ident);

    //  CHANGE FROM A IDENTEVAL TO A REGULAR IDENT
    if(ident)
        ident[0] |= MKOPCODE(UNQUOTED_BIT, 0);

    return ident;

}

// USES ONE SCRATCHPOINTER AND MAY CAUSE GC
WORDPTR rplMakeIdentHidden(WORDPTR ident)
{
    if(ISHIDDENIDENT(*ident))
        return ident;

    ident = rplMakeNewCopy(ident);

    //  CHANGE FROM A IDENTEVAL TO A REGULAR IDENT
    if(ident)
        ident[0] |= MKOPCODE(HIDDEN_BIT, 0);

    return ident;

}

// CHECK IF AN IDENT IS UNQUOTED, IF NOT THEN
// CREATE A NEW UNQUOTED OBJECT AND RETURN IT
// MAY CAUSE GARBAGE COLLECTION
WORDPTR rplMakeIdentVisible(WORDPTR ident)
{
    if(!ISHIDDENIDENT(*ident))
        return ident;

    ident = rplMakeNewCopy(ident);

    //  CHANGE FROM A IDENTEVAL TO A REGULAR IDENT
    if(ident)
        ident[0] &= ~MKOPCODE(HIDDEN_BIT, 0);

    return ident;
}

WORDPTR rplMakeIdentReadOnly(WORDPTR ident)
{
    if(ISLOCKEDIDENT(*ident))
        return ident;

    ident = rplMakeNewCopy(ident);

    //  CHANGE FROM A IDENTEVAL TO A REGULAR IDENT
    if(ident)
        ident[0] |= MKOPCODE(READONLY_BIT, 0);

    return ident;

}

// CHECK IF AN IDENT IS LOCKED, IF NOT THEN
// CREATE A NEW UNLOCKED OBJECT AND RETURN IT
// MAY CAUSE GARBAGE COLLECTION
WORDPTR rplMakeIdentWriteable(WORDPTR ident)
{
    if(!ISLOCKEDIDENT(*ident))
        return ident;

    ident = rplMakeNewCopy(ident);

    //  CHANGE FROM A IDENTEVAL TO A REGULAR IDENT
    if(ident)
        ident[0] &= ~MKOPCODE(READONLY_BIT, 0);

    return ident;
}

// DIRECTORY STRUCTURE:

// PAIRS OF KEY/VALUES
// 1) KEY=STARTDIR MARKER, VALUE=DIR OBJECT
// 2) KEY=PARENTDIR MARKER, VALUE=PARENTDIR OBJECT
// ... KEYS
// n) KEY=ENDDIR MARKER, VALUE=DIR OBJECT

// DIRECTORY OBJECT STRUCTURE:
// 32-BIT PROLOG
// 32-BIT WORD WITH THE NUMBER OF ITEMS IN THE DIR

#define MakeNewHole(start,end,nwords)    memmovew(start+nwords,start,(PTR2NUMBER)(end-start)*(sizeof(void*)>>2))

// DIRS STACK IS INCREASE AFTER FOR STORE, DECREASE BEFORE FOR READ

// CREATE A GLOBAL IN THE GIVEN DIRECTORY

void rplCreateGlobalInDir(WORDPTR nameobj, WORDPTR value, WORDPTR * parentdir)
{
    WORDPTR *direntry = parentdir + 4;  // POINT TO THE FIRST ENTRY IN THE DIRECTORY

    // OPEN A HOLE FOR A NEW VARIABLE AT BEGINNING OF CURRENT DIR
    MakeNewHole(direntry, DirsTop, 2);
    // INCREASE THE END OF DIRS
    DirsTop += 2;
    *direntry = nameobj;
    *(direntry + 1) = value;
    // PATCH THE CURRENT DIRECTORY SIZE
    WORDPTR size = *(parentdir + 1) + 1;
    ++*size;

    // FIX THE CURRENT DIR IN CASE IT MOVED
    if(CurrentDir >= direntry)
        CurrentDir += 2;

    if(DirSize <= DirsTop - Directories + DIRSLACK)
        growDirs((WORD) (DirsTop - Directories + DIRSLACK));

    //if(Exceptions) return;

}

// LOW-LEVEL: MAKE ROOM FOR N GLOBALS, ALL ASSIGNED ZERO_BINT FOR NAME AND VALUE

WORDPTR *rplCreateNGlobalsInDir(BINT n, WORDPTR * parentdir)
{
    WORDPTR *direntry = parentdir + 4;  // POINT TO THE FIRST ENTRY IN THE DIRECTORY

    if(DirSize <=
            (BINT) (DirsTop - Directories + DIRSLACK +
                ((2 * n * sizeof(WORDPTR)) / sizeof(WORD))))
        growDirs((WORD) (DirsTop - Directories + DIRSLACK +
                    ((2 * n * sizeof(WORDPTR)) / sizeof(WORD))));
    if(Exceptions)
        return 0;

    // OPEN A HOLE FOR A NEW VARIABLE AT BEGINNING OF CURRENT DIR
    MakeNewHole(direntry, DirsTop, 2 * n);
    // INCREASE THE END OF DIRS
    DirsTop += 2 * n;
    int k;
    for(k = 0; k < 2 * n; ++k) {
        direntry[k] = (WORDPTR) zero_bint;
    }
    // PATCH THE CURRENT DIRECTORY SIZE
    WORDPTR size = *(parentdir + 1) + 1;
    *size += n;

    // FIX THE CURRENT DIR IN CASE IT MOVED
    if(CurrentDir >= direntry)
        CurrentDir += 2 * n;

    return direntry;

}

void rplCreateGlobal(WORDPTR nameobj, WORDPTR value)
{
    rplCreateGlobalInDir(nameobj, value, CurrentDir);
}

// FIND THE GIVEN DIRECTORY, OR RETURN NULL

WORDPTR *rplFindDirbyHandle(WORDPTR handle)
{

    WORDPTR *scan = Directories;

    while(scan < DirsTop) {
        if(*(scan + 1) == handle)
            return scan;
        scan += 6 + (*(*(scan + 1) + 1) << 1);  //     POINT TO THE NEXT DIRECTORY
    }

    return 0;
}

// CREATE A NEW EMPTY DIRECTORY AT THE PARENT DIRECTORY

// NOTE: THIS FUNCTION RELIES ON DIRSLACK>8
// USES 2 SCRATCH POINTERS
// RETURNS POINTER TO HANDLE OBJECT
WORDPTR rplCreateNewDir(WORDPTR nameobj, WORDPTR * parentdir)
{
    ScratchPointer1 = nameobj;

    WORDPTR *dirptr = rplMakeNewDir();  // MAY CAUSE A GC

    if(!dirptr)
        return NULL;

    // LINK TO PARENT DIR HANDLE
    dirptr[3] = parentdir[1];

    WORDPTR *var = rplFindGlobalInDir(ScratchPointer1, parentdir, 0);
    if(var) {
        // THAT NAME IS BEING USED, OVERWRITE
        *(var + 1) = (WORDPTR) dirptr[1];
        return dirptr[1];

    }
    else {
        // PROTECT THE HANDLE FROM MOVING DURING A GC
        ScratchPointer2 = dirptr[1];
        // CREATE THE GLOBAL VARIABLE TO LINK THE DIRECTORY
        rplCreateGlobalInDir(ScratchPointer1, dirptr[1], parentdir);
        return ScratchPointer2;
    }

}

// LOW LEVEL VERSION CREATES AN UNLINKED DIRECTORY, WITH NO PARENT

WORDPTR *rplMakeNewDir()
{
    // CREATE THE DIRECTORY OBJECT FIRST
    WORDPTR dirobj = rplAllocTempOb(1);

    if(!dirobj)
        return 0;

    // EMPTY DIRECTORY OBJECT
    dirobj[0] = MKPROLOG(DODIR, 1);
    dirobj[1] = 0;

    // NOW CREATE THE NEW DIR

    WORDPTR *direntry = DirsTop;
    DirsTop += 6;

    direntry[0] = (WORDPTR) dir_start_bint;
    direntry[1] = (WORDPTR) dirobj;
    direntry[2] = (WORDPTR) dir_parent_bint;
    direntry[3] = (WORDPTR) root_dir_handle;    // NO PARENT!!!
    direntry[4] = (WORDPTR) dir_end_bint;
    direntry[5] = (WORDPTR) dirobj;

    if(DirSize <= DirsTop - Directories + DIRSLACK)
        growDirs((WORD) (DirsTop - Directories + DIRSLACK));
    if(Exceptions)
        return 0;

    return direntry;
}

// GET THE ADDRESS OF THE PARENT DIRECTORY
WORDPTR *rplGetParentDir(WORDPTR * directory)
{
    if(!directory)
        return 0;
    return rplFindDirbyHandle(directory[3]);
}

// FINDS A GLOBAL, AND RETURNS THE ADDRESS OF THE KEY/VALUE PAIR WITHIN THE DIRECTORY ENVIRONMENT

WORDPTR *rplFindGlobalbyName(BYTEPTR name, BYTEPTR nameend, BINT scanparents)
{
    WORDPTR *direntry = CurrentDir + 4;
    WORDPTR parentdir;

    if(!CurrentDir)
        return 0;

    do {
        parentdir = *(direntry - 3);
        while(direntry < DirsTop) {
            if(**direntry == DIR_END_MARKER)
                break;
            if(rplCompareIDENTByName(*direntry, name, nameend))
                return direntry;
            direntry += 2;
        }
        direntry = rplFindDirbyHandle(parentdir);
    }
    while(scanparents && direntry);
    return 0;
}

WORDPTR *rplFindGlobalbyNameInDir(BYTEPTR name, BYTEPTR nameend,
        WORDPTR * parent, BINT scanparents)
{
    WORDPTR *direntry = parent + 4;
    WORDPTR parentdir;

    if(!parent)
        return 0;
    do {
        parentdir = *(direntry - 3);
        while(direntry < DirsTop) {
            if(**direntry == DIR_END_MARKER)
                break;
            if(rplCompareIDENTByName(*direntry, name, nameend))
                return direntry;
            direntry += 2;
        }
        direntry = rplFindDirbyHandle(parentdir);
    }
    while(scanparents && direntry);
    return 0;
}

WORDPTR *rplFindGlobalInDir(WORDPTR nameobj, WORDPTR * parent, BINT scanparents)
{
    WORDPTR *direntry = parent;
    WORDPTR parentdir;

    if(!parent)
        return 0;

    do {
        parentdir = *(direntry + 3);
        direntry += 4;  // SKIP SELF REFERENCE AND PARENT DIR
        while(direntry < DirsTop) {
            if(**direntry == DIR_END_MARKER)
                break;
            if(rplCompareIDENT(*direntry, nameobj))
                return direntry;
            direntry += 2;
        }
        direntry = rplFindDirbyHandle(parentdir);
    }
    while(scanparents && direntry);
    return 0;
}

WORDPTR *rplFindGlobal(WORDPTR nameobj, BINT scanparents)
{
    return rplFindGlobalInDir(nameobj, CurrentDir, scanparents);
}

// GET A POINTER TO THE KEY/VALUD PAIR AT idx WITHIN directory
// RETURN NULL IF idx IS NOT WITHIN 0<idx<NITEMS IN DIRECTORY

WORDPTR *rplFindVisibleGlobalByIndexInDir(BINT idx, WORDPTR * directory)
{
    BINT nitems = *(directory[1] + 1);

    if((idx < 0) || (idx >= nitems))
        return 0;
    BINT k = 4;
    while(*directory[k] != DIR_END_MARKER) {
        if(ISIDENT(*directory[k]) && !ISHIDDENIDENT(*directory[k]))
            --idx;
        if(idx < 0)
            return directory + k;
        k += 2;
    }
    return 0;
}

// SAME AS ABOVE BUT FOR CURRENT DIRECTORY ONLY
WORDPTR *rplFindVisibleGlobalByIndex(BINT idx)
{
    return rplFindVisibleGlobalByIndexInDir(idx, CurrentDir);
}

// GET A POINTER TO THE KEY/VALUD PAIR AT idx WITHIN directory
// RETURN NULL IF idx IS NOT WITHIN 0<idx<NITEMS IN DIRECTORY

WORDPTR *rplFindGlobalByIndexInDir(BINT idx, WORDPTR * directory)
{
    BINT nitems = *(directory[1] + 1);

    if((idx < 0) || (idx >= nitems))
        return 0;
    return directory + 4 + 2 * idx;
}

// SAME AS ABOVE BUT FOR CURRENT DIRECTORY ONLY
WORDPTR *rplFindGlobalByIndex(BINT idx)
{
    return rplFindGlobalByIndexInDir(idx, CurrentDir);
}

// GET TOTAL NUMBER OF VARIABLES IN THE DIRECTORY
BINT rplGetVarCountInDir(WORDPTR * directory)
{
    return *(directory[1] + 1);
}

BINT rplGetVarCount()
{
    return *(CurrentDir[1] + 1);
}

// GET TOTAL NUMBER OF VISIBLE VARIABLES IN THE DIRECTORY
// SKIPS ANY VARIABLES WHERE THE KEY IS NOT A NUMBER OR
// ANY IDENTS THAT ARE MARKED AS HIDDEN (BY PROLOG DOIDENTEVAL...)
BINT rplGetVisibleVarCountInDir(WORDPTR * directory)
{
    BINT n = 0;
    WORDPTR *dirptr = directory + 4;
    while(**dirptr != DIR_END_MARKER) {
        if(ISIDENT(**dirptr) && !ISHIDDENIDENT(**dirptr))
            ++n;
        dirptr += 2;
    }
    return n;
}

BINT rplGetVisibleVarCount()
{
    return rplGetVisibleVarCountInDir(CurrentDir);
}

// RCL A GLOBAL, RETURN POINTER TO ITS VALUE
// LOOKS IN CURRENT DIR AND PARENT DIRECTORIES
WORDPTR rplGetGlobal(WORDPTR nameobj)
{
    WORDPTR *var = rplFindGlobal(nameobj, 1);
    if(var)
        return *(var + 1);
    return 0;
}

// OBTAIN THE DIRECTORY IN WHICH THIS GLOBAL IS
// WORK FROM A POINTER TO THE KEY/VALUE PAIR OF THE GLOBAL
// AND RETURN THE START OF THE DIRECTORY

WORDPTR *rplGetDirfromGlobal(WORDPTR * var)
{
    while((var >= Directories) && (*var) && (**var != DIR_START_MARKER))
        var -= 2;
    return var;
}

// LOW LEVEL VERSION OF PURGE
void rplPurgeForced(WORDPTR * var)
{
    if(ISPROLOG(**(var + 1)) && (LIBNUM(**(var + 1)) == DODIR)) {
        // TRYING TO PURGE AN ENTIRE DIRECTORY

        WORD dirsize = *(*(var + 1) + 1);
        WORDPTR *emptydir = rplFindDirbyHandle(*(var + 1));

        if(dirsize) {
            // EITHER DIRECTORY IS FULL OR THIS IS AN ORPHAN HANDLER
            if(emptydir) {
                // DIRECTORY IS NOT EMPTY!
                return;
            }

            // DIRECTORY NO LONGER EXISTS, PROBABLY WIPED OUT PREVIOUSLY, IT'S OK TO PURGE

        }

        // REMOVE THE EMPTY DIR

        if(CurrentDir == emptydir) {
            CurrentDir = rplFindDirbyHandle(emptydir[3]);       // CHANGE CURRENT DIR TO PARENT
        }

        if(emptydir) {
            MakeNewHole(emptydir + 6, DirsTop, -6);     // THIS WILL REMOVE THE EMPTY DIRECTORY IN MEMORY
            if(CurrentDir >= emptydir)
                CurrentDir -= 6;
            DirsTop -= 6;
        }

    }

    MakeNewHole(var + 2, DirsTop, -2);  // AND REMOVE THE ENTRY
    // UPDATE THE DIRECTORY COUNT
    WORDPTR *dir = rplGetDirfromGlobal(var);
    --*(*(dir + 1) + 1);

    if(CurrentDir >= var)
        CurrentDir -= 2;
    DirsTop -= 2;

    return;
}

// PURGE A SINGLE VARIABLE

void rplPurgeGlobal(WORDPTR nameobj)
{
    WORDPTR *var = rplFindGlobal(nameobj, 0);

    if(!var) {
        //rplError(ERR_UNDEFINEDVARIABLE);
        return;
    }

    if(ISPROLOG(**(var + 1)) && (LIBNUM(**(var + 1)) == DODIR)) {
        // TRYING TO PURGE AN ENTIRE DIRECTORY

        WORD dirsize = *(*(var + 1) + 1);

        // NEED TO USE PGDIR FOR THAT
        if(dirsize) {
            rplError(ERR_NONEMPTYDIRECTORY);
            return;
        }

    }

    if(ISLOCKEDIDENT(**var)) {
        rplError(ERR_READONLYVARIABLE);
        return;
    }

    rplPurgeForced(var);

    // PURGE ALL PROPERTIES OF THIS VARIABLE TOO
    while((var = rplFindGlobalPropInDir(nameobj, 0, CurrentDir, 0))) {

        WORD prop = rplGetIdentProp(var[0]);

        if(prop == IDPROP_DEFN) {
            WORDPTR *stksave = DSTop;
            rplPushDataNoGrow(nameobj);
            rplUpdateDependencyTree(nameobj, CurrentDir, var[1],
                    (WORDPTR) zero_bint);
            nameobj = *stksave;
            DSTop = stksave;
            // SEARCH AGAIN, SINCE DIRECTORIES WERE UPDATED BY THE DEPENDENCY TREE
            var = rplFindGlobalPropInDir(nameobj, 0, CurrentDir, 0);
        }

        rplPurgeForced(var);
    }

}

WORDPTR rplGetDirName(WORDPTR * dir)
{
    WORDPTR *parent = dir + 3;

    parent = rplFindDirbyHandle(*parent);

    if(!parent)
        return 0;

    while(**parent != DIR_END_MARKER) {
        if(*(parent + 1) == *(dir + 1))
            return *parent;
        parent += 2;
    }
    return 0;
}

// FILL A BUFFER WITH POINTERS TO THE NAME OBJECTS TO ALL PARENTS
// FIRST POINTER IN THE BUFFER IS GIVEN DIR'S NAME, LAST POINTER IS HOME
// OR THE NAME AT THE MAXIMUM REQUESTED DEPTH
// buffer MUST BE PREALLOCATED AND CONTAIN SPACE FOR UP TO maxdepth POINTERS
// RETURNS THE NUMBER OF POINTERS STORED IN buffer
BINT rplGetFullPath(WORDPTR * dir, WORDPTR * buffer, BINT maxdepth)
{
    WORDPTR *parent, *pptr;
    BINT nptrs = 0;

    while(dir && (nptrs < maxdepth)) {
        parent = rplFindDirbyHandle(dir[3]);

        if(!parent)
            return nptrs;
        pptr = parent;
        while(**pptr != DIR_END_MARKER) {
            if(*(pptr + 1) == *(dir + 1))
                break;
            pptr += 2;
        }

        if(**pptr != DIR_END_MARKER) {
            // FOUND THE NAME
            buffer[nptrs] = *pptr;
            ++nptrs;
            dir = parent;
        }
    }
    return nptrs;
}

// CREATE A COMPLETELY NEW COPY OF THE DIRECTORY
// AND ALL ITS SUBDIRECTORIES
WORDPTR *rplDeepCopyDir(WORDPTR * sourcedir)
{
    WORDPTR *targetdir = rplMakeNewDir();
    if(!targetdir)
        return 0;
    WORDPTR *srcvars = sourcedir + 4;
    while((**srcvars != DIR_END_MARKER) && (**srcvars != DIR_START_MARKER)) {
        if(LIBNUM(**(srcvars + 1)) == DODIR) {
            //POINTS TO A DIRECTORY, NEED TO DEEP-COPY THAT ONE TOO
            WORDPTR *subdir =
                    rplDeepCopyDir(rplFindDirbyHandle(*(srcvars + 1)));

            if(subdir) {
                WORDPTR subhandle = *(subdir + 1);
                rplCreateGlobalInDir(*srcvars, subhandle, targetdir);
                // CREATION OF A GLOBAL MIGHT HAVE MOVED subdir, SO WE NEED TO FIND IT
                subdir = rplFindDirbyHandle(subhandle);
                *(subdir + 3) = *(targetdir + 1);
            }
        }
        else
            rplCreateGlobalInDir(*srcvars, *(srcvars + 1), targetdir);
        srcvars += 2;
    }

    return targetdir;

}

// FIND FIRST ENTRY IN A DIRECTORY BY HANDLE
// RETURN NULL IF INVALID OR EMPTY DIRECTORY
WORDPTR *rplFindFirstByHandle(WORDPTR dirhandle)
{
    WORDPTR *direntry = rplFindDirbyHandle(dirhandle);
    if(!direntry)
        return 0;
    if(*direntry[4] == DIR_END_MARKER)
        return 0;       // DIRECTORY WAS EMPTY
    return direntry + 4;        // FIRST ACTUAL VARIABLE IN THE DIRECTORY
}

// FIND FIRST ENTRY IN A DIRECTORY BY HANDLE
// RETURN NULL IF INVALID OR EMPTY DIRECTORY
WORDPTR *rplFindFirstInDir(WORDPTR * directory)
{
    WORDPTR *direntry = directory;
    if(!direntry)
        return 0;
    if(*direntry[4] == DIR_END_MARKER)
        return 0;       // DIRECTORY WAS EMPTY
    return direntry + 4;        // FIRST ACTUAL VARIABLE IN THE DIRECTORY
}

// RETURN THE NEXT ENTRY IN A DIRECTORY
// OR NULL IF END OF DIR
WORDPTR *rplFindNext(WORDPTR * direntry)
{
    direntry += 2;
    if(**direntry == DIR_END_MARKER)
        return 0;
    return direntry;
}

// PURGE EVERYTHING IN THE DIRECTORY, RECURSIVELY

void rplWipeDir(WORDPTR * directory)
{
    WORDPTR *direntry = directory + 4;
    WORDPTR *Stacksave = DSTop;

    if(!directory)
        return;

    // PUSH THE HANDLE TO PURGE LATER
    rplNewBINTPush(direntry - directory, DECBINT);
    rplPushData(directory[1]);
    if(Exceptions) {
        DSTop = Stacksave;
        return;
    }

    while(DSTop != Stacksave) {

        directory = rplFindDirbyHandle(rplPopData());
        direntry = directory + rplReadBINT(rplPopData());
        if(!directory)
            continue;   // SAFEGUARD IN CASE OF DUPLICATED ENTRIES POINTING TO THE SAME DIRECTORY

        // CONTINUE SCANNING

        while(**direntry != DIR_END_MARKER) {

            if(ISPROLOG(**(direntry + 1))
                    && (LIBNUM(**(direntry + 1)) == DODIR)) {
                // TRYING TO PURGE AN ENTIRE DIRECTORY

                WORD dirsize = *(*(direntry + 1) + 1);

                // NEED TO USE PGDIR FOR THAT
                if(dirsize) {
                    // NON-EMPTY DIR, RECURSE INTO IT
                    ScratchPointer1 = *(direntry + 1);  // PROTECT THE HANDLE FROM GC
                    rplNewBINTPush(direntry + 2 - directory, DECBINT);  // RESUME AT THE FOLLOWING ENTRY
                    rplPushData(directory[1]);
                    if(Exceptions) {
                        DSTop = Stacksave;
                        return;
                    }
                    directory = rplFindDirbyHandle(ScratchPointer1);
                    direntry = directory + 4;
                    continue;
                }
                else {
                    // REMOVE THE EMPTY DIR

                    WORDPTR *emptydir = rplFindDirbyHandle(*(direntry + 1));

                    if(CurrentDir == emptydir) {
                        CurrentDir = rplFindDirbyHandle(emptydir[3]);   // CHANGE CURRENT DIR TO PARENT
                    }

                    if(emptydir) {
                        MakeNewHole(emptydir + 6, DirsTop, -6); // THIS WILL REMOVE THE EMPTY DIRECTORY IN MEMORY
                        if(CurrentDir >= emptydir)
                            CurrentDir -= 6;
                        DirsTop -= 6;
                        if(directory >= emptydir)
                            directory -= 6;
                        if(direntry >= emptydir)
                            direntry -= 6;
                    }

                }

            }

            direntry += 2;

        }

        // FINISHED SCANNING, THE DIRECTORY NOW CONTAINS ALL VARIABLES AND EMPTY DIRS

        // REMOVE ALL VARIABLES
        direntry += 2;
        MakeNewHole(direntry, DirsTop, -(direntry - directory));        // THIS WILL REMOVE ALL ENTRIES AT ONCE
        if(CurrentDir >= direntry)
            CurrentDir -= direntry - directory;
        DirsTop -= direntry - directory;
        direntry = directory;

    }

    // DONE, THE GIVEN DIRECTORY NO LONGER EXISTS
    return;

}

void rplPurgeDir(WORDPTR nameobj)
{
    WORDPTR *var = rplFindGlobal(nameobj, 0);

    if(!var) {
        rplError(ERR_DIRECTORYNOTFOUND);
        return;
    }

    if(ISPROLOG(**(var + 1)) && (LIBNUM(**(var + 1)) == DODIR)) {
        // TRYING TO PURGE AN ENTIRE DIRECTORY
        WORDPTR *dir = rplFindDirbyHandle(*(var + 1));

        // PROTECT THE DIRECTORY NAME IN THE STACK
        rplPushDataNoGrow(nameobj);

        // RECURSIVELY WIPE OUT THE DIRECTORY
        rplWipeDir(dir);
        if(Exceptions)
            return;
        // RESTORE DIRECTORY NAME
        nameobj = rplPopData();

        // GET DIRECTORY POINTERS FROM NAME AGAIN, SINCE WIPE DIR MIGHT'VE MOVED THE POINTERS!
        var = rplFindGlobal(nameobj, 0);
        rplPurgeForced(var);
        return;

    }

    rplError(ERR_DIRECTORYNOTFOUND);
}

void rplPurgeDirByHandle(WORDPTR handle)
{

    WORDPTR *dir = rplFindDirbyHandle(handle);
    WORDPTR parenthandle;

    if(dir) {

        rplPushDataNoGrow(handle);
        // FIND A HANDLE TO THE PARENT DIR
        // AND PRESERVE THE HANDLE OBJECT
        if(*dir[2] == *dir_parent_bint) {
            parenthandle = dir[3];
            if(parenthandle)
                rplPushDataNoGrow(parenthandle);
            else
                parenthandle = 0;
        }
        else
            parenthandle = 0;

        // RECURSIVELY WIPE OUT THE DIRECTORY
        rplWipeDir(dir);

        if(parenthandle)
            parenthandle = rplPopData();
        handle = rplPopData();
        if(Exceptions)
            return;
        // GET DIRECTORY POINTERS FROM NAME AGAIN, SINCE WIPE DIR MIGHT'VE MOVED THE POINTERS!
        if(parenthandle) {
            dir = rplFindDirbyHandle(parenthandle);
            if(!dir)
                return;

            while(**dir != DIR_END_MARKER) {
                if(*(dir + 1) == handle)
                    break;
                dir += 2;
            }
            if(**dir != DIR_END_MARKER)
                rplPurgeForced(dir);
        }

        return;
    }

    rplError(ERR_DIRECTORYNOTFOUND);

}

// CREATE OR MODIFY VARIABLES IN THE SETTINGS DIRECTORY
void rplStoreSettings(WORDPTR nameobject, WORDPTR object)
{
    if(!ISDIR(*SettingsDir))
        return;
    WORDPTR *setting =
            rplFindGlobalInDir(nameobject, rplFindDirbyHandle(SettingsDir), 0);
    if(setting)
        setting[1] = object;
    else
        rplCreateGlobalInDir(nameobject, object,
                rplFindDirbyHandle(SettingsDir));
}

void rplStoreSettingsbyName(BYTEPTR name, BYTEPTR nameend, WORDPTR object)
{
    if(!ISDIR(*SettingsDir))
        return;

    WORDPTR *setting =
            rplFindGlobalbyNameInDir(name, nameend,
            rplFindDirbyHandle(SettingsDir), 0);
    if(setting) {
        setting[1] = object;
    }
    else {

        ScratchPointer2 = object;
        WORDPTR nameobject = rplCreateIDENT(DOIDENT, name, nameend);
        if(!nameobject)
            return;
        rplCreateGlobalInDir(nameobject, ScratchPointer2,
                rplFindDirbyHandle(SettingsDir));
    }
}

// GET THE SETTINGS AND RETURN A POINTER TO THE OBJECT, OR NULL IF IT DOESN'T EXIST
WORDPTR rplGetSettings(WORDPTR nameobject)
{
    if(!ISDIR(*SettingsDir))
        return 0;
    WORDPTR *setting =
            rplFindGlobalInDir(nameobject, rplFindDirbyHandle(SettingsDir), 0);
    if(setting)
        return setting[1];
    return 0;
}

WORDPTR rplGetSettingsbyName(BYTEPTR name, BYTEPTR nameend)
{
    if(!ISDIR(*SettingsDir))
        return 0;
    WORDPTR *setting =
            rplFindGlobalbyNameInDir(name, nameend,
            rplFindDirbyHandle(SettingsDir), 0);
    if(setting)
        return setting[1];
    return 0;
}

// PURGE A SINGLE VARIABLE

void rplPurgeSettings(WORDPTR nameobj)
{
    if(!ISDIR(*SettingsDir))
        return;
    WORDPTR *var =
            rplFindGlobalInDir(nameobj, rplFindDirbyHandle(SettingsDir), 0);

    if(!var) {
        return;
    }

    if(ISPROLOG(**(var + 1)) && (LIBNUM(**(var + 1)) == DODIR)) {
        // TRYING TO PURGE AN ENTIRE DIRECTORY

        WORD dirsize = *(*(var + 1) + 1);

        // NEED TO USE PGDIR FOR THAT
        if(dirsize) {
            rplError(ERR_NONEMPTYDIRECTORY);
            return;
        }

    }

    rplPurgeForced(var);
}

// RETURN TRUE IF A VARIABLE IS VISIBLE IN A DIRECTORY
BINT rplIsVarVisible(WORDPTR * var)
{
    if(ISHIDDENIDENT(**var))
        return 1;
    return 0;
}

// RETURN TRUE IF A VARIABLE IS LOCKED IN A DIRECTORY
BINT rplIsVarReadOnly(WORDPTR * var)
{
    if(ISLOCKEDIDENT(**var))
        return 1;
    return 0;
}

// RETURN TRUE IF A VARIABLE IS IS A DIRECTORY
BINT rplIsVarDirectory(WORDPTR * var)
{
    if(ISPROLOG(**(var + 1)) && (LIBNUM(**(var + 1)) == DODIR))
        return 1;
    return 0;
}

// RETURN TRUE IF A VARIABLE IS IS AN EMPTY DIRECTORY
BINT rplIsVarEmptyDir(WORDPTR * var)
{
    if(ISPROLOG(**(var + 1)) && (LIBNUM(**(var + 1)) == DODIR)) {
        WORD dirsize = *(*(var + 1) + 1);
        WORDPTR *emptydir = rplFindDirbyHandle(*(var + 1));

        if(dirsize) {
            // EITHER DIRECTORY IS FULL OR THIS IS AN ORPHAN HANDLER
            if(!emptydir)
                return 1;       // DIRECTORY IS AN ORPHAN DIR, CONSIDER IT EMPTY
            return 0;
        }
        else
            return 1;
    }
    return 0;
}

// RETURNS A POINTER TO THE START OF A DIRECTORY
// FROM A LIST OF DIRECTORIES IN A PATH
// uselastname=0 --> DISREGARD THE LAST NAME (ASSUME IT'S A VARIABLE)
// uselastname=1 --> THE ENTIRE LIST IS A PATH

// RETURN NULL IF ANY DIRECTORY IS NOT FOUND
// PATH CAN BE ABSOLUTE OR RELATIVE TO CurrentDir
// LIST MAY CONTAIN HOME AND UPDIR

WORDPTR *rplFindDirFromPath(WORDPTR pathlist, BINT uselastname)
{
    WORDPTR ident, last;
    if(!ISLIST(*pathlist))
        return CurrentDir;
    WORDPTR *dir = CurrentDir;

    last = rplSkipOb(pathlist) - 1;     // POINT TO CMD_ENDLIST

    if(!uselastname) {
        ident = pathlist + 1;
        while(rplSkipOb(ident) < last)
            ident = rplSkipOb(ident);

        last = ident;   // HERE IDENT POINTS TO THE LAST OBJECT IN THE LIST
    }

    ident = pathlist + 1;

    while(ident < last) {
        if(*ident == CMD_HOME)
            dir = Directories;
        else if(*ident == CMD_UPDIR)
            dir = rplGetParentDir(dir);
        else if(ISIDENT(*ident)) {
            dir = rplFindGlobalInDir(ident, dir, 0);
            if(dir) {
                if(!ISDIR(*dir[1])) {
                    dir = 0;
                }
                else
                    dir = rplFindDirbyHandle(dir[1]);
            }
        }
        else
            dir = 0;

        if(!dir) {
            rplError(ERR_DIRECTORYNOTFOUND);
            return 0;
        }
        ident = rplSkipOb(ident);
    }

    return dir;
}

// FIND A SPECIFIC PROPERTY. IF propname==0 FINDS ANY PROPERTY OF THE GIVEN VARIABLE
WORDPTR *rplFindGlobalPropInDir(WORDPTR nameobj, WORD propname,
        WORDPTR * parent, BINT scanparents)
{
    WORDPTR *direntry = parent;
    WORDPTR parentdir;
    BINT idlen = rplGetIdentLength(nameobj);
    BYTEPTR nameptr = (BYTEPTR) (nameobj + 1);

    if(!parent)
        return 0;

    do {
        parentdir = *(direntry + 3);
        direntry += 4;  // SKIP SELF REFERENCE AND PARENT DIR
        while(direntry < DirsTop) {
            if(**direntry == DIR_END_MARKER)
                break;

            // FIND A PROPERTY NAME = VARNAME...PROPNAME
            if(ISIDENT(**direntry)
                    && (rplGetIdentLength(*direntry) == idlen + 7)) {
                // LONG ENOUGH TO BE A PROPERTY OF THIS VARIABLE
                BYTEPTR ptr = (BYTEPTR) (direntry[0] + 1);
                int k;
                for(k = 0; k < idlen; ++k)
                    if(ptr[k] != nameptr[k])
                        break;

                if(k == idlen) {
                    // NAME MATCHES
                    if((ptr[idlen] == '.') && (ptr[idlen + 1] == '.')
                            && (ptr[idlen + 2] == '.')) {
                        // FOUND A PROPERTY, CHECK IF IT'S THE SAME

                        WORD prop =
                                propname ? (ptr[idlen + 3] | (ptr[idlen +
                                        4] << 8) | (ptr[idlen +
                                        5] << 16) | (ptr[idlen + 6] << 24)) : 0;
                        if(prop == propname) {
                            // CORRECT PROPERTY AND CORRECT VARIABLE LENGTH, NOW CHECK IF THE NAME MATCHES
                            return direntry;
                        }
                    }
                }

            }
            direntry += 2;
        }
        direntry = rplFindDirbyHandle(parentdir);
    }
    while(scanparents && direntry);
    return 0;
}

// SAME AS CREATE GLOBAL, BUT CREATES A PROPERTY NAME AND MARKS IT HIDDEN
// USES 2 SCRATCH POINTERS
void rplCreateGlobalPropInDir(WORDPTR nameobj, WORD propname, WORDPTR value,
        WORDPTR * parentdir)
{
    WORDPTR newname;
    BINT len = rplGetIdentLength(nameobj);
    ScratchPointer1 = value;
    ScratchPointer2 = nameobj;
    newname = rplAllocTempOb((len + 10) >> 2);
    nameobj = ScratchPointer2;
    value = ScratchPointer1;
    if(!newname)
        return;
    newname[0] = MKPROLOG(DOIDENTHIDDEN, (len + 10) >> 2);
    newname[(len + 10) >> 2] = 0;       // FILL LAST WORD WITH ZEROS
    memmovew(newname + 1, nameobj + 1, OBJSIZE(*nameobj));
    BYTEPTR string = (BYTEPTR) (newname + 1);
    string[len] = string[len + 1] = string[len + 2] = '.';
    string[len + 3] = propname & 0xff;
    string[len + 4] = (propname >> 8) & 0xff;
    string[len + 5] = (propname >> 16) & 0xff;
    string[len + 6] = (propname >> 24) & 0xff;

    rplCreateGlobalInDir(newname, value, parentdir);

}

// START AUTO-EVALUATION OF DEPENDENCIES
void rplDoAutoEval(WORDPTR varname, WORDPTR * indir)
{
    // STEP 1: CREATE A LIST OF VARIABLES TO RECALCULATE
    WORDPTR *stksave = DSTop;
    rplPushDataNoGrow(varname); // SAVE THE ROOT VARIABLE TO EVALUATE
    WORDPTR *stkptr = stksave;
    WORDPTR *var;

    if(!indir)
        indir = CurrentDir;

    ScratchPointer1 = varname;  // PRESERVE POINTER FROM GC

    while(stkptr < DSTop) {
        // CHECK IF THE VARIABLE HAS DEPENDENCIES
        var = rplFindGlobalInDir(*stkptr, indir, 0);

        if(var && (IDENTHASATTR(*var[0]))) {

            WORD attr = rplGetIdentAttr(var[0]);
            if(attr & IDATTR_DEPEND) {
                var = rplFindGlobalPropInDir(*stkptr, IDPROP_DEPN, indir, 0);
                if(var) {
                    // EXPLODE THE DEPENDENCY LIST ON THE STACK
                    if(ISLIST(*var[1])) {
                        BINT k = rplListLength(var[1]);
                        rplExpandStack(k);
                        if(Exceptions) {
                            DSTop = stksave;
                            return;
                        }
                        WORDPTR item = var[1] + 1;
                        while(k--) {
                            if(ISIDENT(*item)) {
                                WORDPTR *stkptr2 = stksave;

                                while(stkptr2 < stkptr) {
                                    if(*stkptr2 == item) {
                                        // THIS VARIABLE HAD BEEN CALCULATED EARLIER, REMOVE IT
                                        memmovew(stkptr2, stkptr2 + 1,
                                                DSTop - stkptr2 - 1);
                                        --DSTop;
                                        --stkptr;
                                    }
                                    else
                                        ++stkptr2;
                                }
                                // NOW WE HAVE GUARANTEE THAT THE VARIABLE WILL BE EVALUATED ONCE AND ONLY ONCE
                                rplPushDataNoGrow(item);

                            }
                            item = rplSkipOb(item);
                        }

                        // ALL VARIABLES WERE PLACED IN STACK

                    }

                }
            }

        }

        ++stkptr;

    }

    // DONE CREATING LIST OF VARIABLES THAT NEED TO BE RECALCULATED

    // IS THIS REALLY A PROBLEM?
    if(*stksave != ScratchPointer1) {
        // THE MAIN VARIABLE WAS MOVED DOWN THE LIST!
        // THIS MEANS CIRCULAR REFERENCE
        rplError(ERR_CIRCULARREFERENCE);
        DSTop = stksave;
        return;
    }

    // NOW DO ->NUM ON THE 'Defn' PROPERTY OF ALL VARIABLES ON THE LIST AND STORE THEIR RESULTS
    stkptr = stksave + 1;

    while(stkptr < DSTop) {
        // FIND VARIABLE 'Defn' PROPERTY
        // CHECK IF THE VARIABLE HAS DEPENDENCIES
        var = rplFindGlobalInDir(*stkptr, indir, 0);

        if(var && (IDENTHASATTR(*var[0]))) {

            WORD attr = rplGetIdentAttr(var[0]);
            if(attr & IDATTR_DEFN) {
                WORDPTR *varcalc =
                        rplFindGlobalPropInDir(*stkptr, IDPROP_DEFN, indir, 0);
                if(varcalc) {
                    // FOUND A FORMULA OR PROGRAM, IF A PROGRAM, RUN XEQ THEN ->NUM
                    // IF A FORMULA OR ANYTHING ELSE, ->NUM
                    rplPushData(varcalc[1]);
                    WORDPTR *stkcheck = DSTop;
                    if(ISPROGRAM(*varcalc[1]))
                        rplRunAtomic(CMD_OVR_XEQ);
                    if(Exceptions) {
                        rplBlameError(varcalc[0]);      // AT LEAST SHOW WHERE THE ERROR CAME FROM
                        if(DSTop > stksave)
                            DSTop = stksave;
                        return;
                    }
                    rplRunAtomic(CMD_OVR_NUM);
                    if(Exceptions) {
                        if(!((Exceptions & EX_ERRORCODE)
                                    && (ErrorCode == ERR_UNDEFINEDVARIABLE)))
                            rplBlameError(varcalc[0]);  // AT LEAST SHOW WHERE THE ERROR CAME FROM
                        if(DSTop > stksave)
                            DSTop = stksave;
                        return;
                    }

                    // WE GOT THE RESULT ON THE STACK
                    if(DSTop != stkcheck) {
                        rplError(ERR_BADARGCOUNT);
                        rplBlameError(varcalc[0]);      // AT LEAST SHOW WHERE THE ERROR CAME FROM
                        DSTop = stksave;
                        return;
                    }
                    // STORE THE NEW RESULT AND CONTINUE
                    var[1] = rplPopData();
                }
            }
        }
        ++stkptr;
    }

    // ALL VARIABLES WERE RECOMPUTED

    DSTop = stksave;
}

// GET THE 4-LETTER PROPERTY IDENTIFIER AS A 32-BIT WORD
// OR ZERO IF THERE'S NO PROPERTY IN THE IDENT
WORD rplGetIdentProp(WORDPTR ident)
{
    WORD prop = 0;
    BYTEPTR ptr = (BYTEPTR) (ident + 1);
    BYTEPTR end = (BYTEPTR) rplSkipOb(ident);

    if(IDENTHASATTR(*ident))
        end -= 4;
    while(end[-1] == 0)
        --end;

    // FIND THE TRIPLE DOT SEPARATOR
    if(end - ptr < 8)
        return 0;
    ptr = end - 7;
    if((ptr[0] != '.') || (ptr[1] != '.') || (ptr[2] != '.'))
        return 0;
    ptr += 3;
    prop = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
    return prop;
}

// RETURN AN IDENT WITH ANY PROPERTIES REMOVED
WORDPTR rplMakeIdentNoProps(WORDPTR ident)
{
    BYTEPTR ptr = (BYTEPTR) (ident + 1);
    BYTEPTR start = ptr, end = (BYTEPTR) rplSkipOb(ident);

    if(IDENTHASATTR(*ident))
        end -= 4;
    while(end[-1] == 0)
        --end;

    // FIND THE TRIPLE DOT SEPARATOR
    if(end - ptr < 8)
        return ident;
    ptr = end - 7;
    if((ptr[0] != '.') || (ptr[1] != '.') || (ptr[2] != '.'))
        return ident;

    WORDPTR newident = rplAllocTempOb((ptr - start + 3) >> 2);
    if(newident) {
        newident[0] = MKPROLOG(DOIDENT, (ptr - start + 3) >> 2);
        BINT nchars = ptr - start;
        int k;
        ptr = (BYTEPTR) (newident + 1);
        for(k = 0; k < nchars; ++k)
            ptr[k] = start[k];
        while(k & 3)
            ptr[k++] = 0;
    }

    return newident;

}

// INTERNAL FUNCTION TO CLEAN UP LOCALS FROM THE STACK
// LAM ENVIRONMENT ENDS WITH cmd COMMAND
static void ClrLAMonStack(WORDPTR * bottom, WORDPTR cmdlist)
{
    WORDPTR *stkptr = DSTop - 1;
    WORDPTR find;
    while(stkptr >= bottom) {
        if(!ISPROLOG(**stkptr)) {
            find = cmdlist;
            while(*find) {
                if(**stkptr == *find)
                    break;
                ++find;
            }
            if(*find)
                break;
        }
        --stkptr;
    }
    if(stkptr < bottom)
        return;

    // FOUND THE MARKER
    WORDPTR *scan = stkptr + 1;
    while(scan < DSTop) {
        if(ISIDENT(**scan)) {
            if(!(rplGetIdentAttr(*scan) & IDATTR_DONTUSE)) {
                // KEEP THIS GLOBAL DEPENDENCY
                *stkptr = *scan;
                ++stkptr;
            }
        }
        ++scan;
    }

    DSTop = stkptr;
}

// PUSH ALL DEPENDENCIES OF A PROGRAM ON THE STACK, RETURN THE NUMBER OF THEM
// USES 3 SCRATCHPOINTERS
BINT rplGetDependency(WORDPTR program)
{
    if(!program)
        return 0;
    WORDPTR *savestk = DSTop;

    WORDPTR end = rplSkipOb(program);
    WORDPTR ptr = program;
    while(ptr < end) {
        // HANDLE A FEW SPECIAL CASES
        if(ISPROGRAM(*ptr)) {
            ++ptr;      // GO INSIDE ALL PROGRAMS
            continue;
        }
        if(ISLIST(*ptr)) {
            ++ptr;      // GO INSIDE ALL LISTS
            continue;
        }
        if(ISSYMBOLIC(*ptr)) {
            ++ptr;      // GO INSIDE ALL SYMBOLICS
            continue;
        }
        if(ISMATRIX(*ptr)) {
            ptr = rplMatrixGetFirstObj(ptr);    // GO INSIDE ALL MATRIX OBJECTS
            continue;
        }

        if(((*ptr) & ~0xffff) == MKOPCODE(DOIDENT, NEWNLOCALS)) {
            // NEW LOCAL VAR DEFINITIONS ARE NOT REALLY DEPENDENCIES, MARK AS NOT-DEPENDENCY
            BINT nlocals = *ptr & 0xffff;
            ScratchPointer2 = ptr;
            ScratchPointer3 = end;
            while(nlocals) {
                WORDPTR varattr =
                        rplSetIdentAttr(rplPeekData(nlocals), IDATTR_DONTUSE,
                        IDATTR_DONTUSE);
                if(!varattr) {
                    DSTop = savestk;
                    return 0;
                }
                rplOverwriteData(nlocals, varattr);
                --nlocals;
            }
            ptr = ScratchPointer2;
            end = ScratchPointer3;

        }

        if(!ISPROLOG(*ptr)) {
            switch (*ptr) {
            case CMD_FOR:
            {
                ScratchPointer3 = end;
                rplPushDataNoGrow(ptr);
                rplPushData(ptr + 1);
                WORDPTR varattr =
                        rplSetIdentAttr(rplPeekData(1), IDATTR_DONTUSE,
                        IDATTR_DONTUSE);
                if(!varattr) {
                    DSTop = savestk;
                    return 0;
                }
                ptr = rplSkipOb(rplPeekData(1));
                end = ScratchPointer3;
                rplOverwriteData(1, varattr);
                break;
            }
            case CMD_START:
            case CMD_DO:
            case CMD_WHILE:
            case CMD_CASE:
            case CMD_IF:
            case CMD_IFERR:
            {
                ScratchPointer3 = end;
                rplPushData(ptr);
                ptr = rplPeekData(1) + 1;
                end = ScratchPointer3;

                break;
            }
            case CMD_THEN:
            case CMD_THENERR:
            case CMD_THENCASE:
            {
                const WORD const then_list[] =
                        { CMD_IF, CMD_IFERR, CMD_CASE, 0 };
                ClrLAMonStack(savestk, (WORDPTR) then_list);
                rplPushData(ptr);
                ptr = rplPeekData(1) + 1;
                break;
            }
            case CMD_ELSE:
            case CMD_ELSEERR:
            {
                const WORD const else_list[] = { CMD_THEN, CMD_THENERR, 0 };
                ClrLAMonStack(savestk, (WORDPTR) else_list);
                break;
                ScratchPointer3 = end;
                rplPushData(ptr);
                ptr = rplPeekData(1) + 1;
                end = ScratchPointer3;
                break;
            }

            case CMD_NEXT:
            case CMD_STEP:
            {
                const WORD const next_list[] = { CMD_FOR, CMD_START, 0 };
                ClrLAMonStack(savestk, (WORDPTR) next_list);
                break;
            }
            case CMD_ENDIF:
            case CMD_ENDERR:
            case CMD_ENDTHEN:
            {
                const WORD const endif_list[] =
                        { CMD_THEN, CMD_ELSE, CMD_THENERR, CMD_ELSEERR,
                            CMD_THENCASE, 0 };
                ClrLAMonStack(savestk, (WORDPTR) endif_list);
                break;
            }
            case CMD_ENDDO:
            case CMD_ENDWHILE:
            case CMD_ENDCASE:
            {
                const WORD const end_list[] =
                        { CMD_DO, CMD_WHILE, CMD_CASE, 0 };
                ClrLAMonStack(savestk, (WORDPTR) end_list);
                break;
            }

            }
        }

        if(ISIDENT(*ptr)) {
            ScratchPointer2 = ptr;
            ScratchPointer3 = end;
            rplPushData(ptr);
            end = ScratchPointer3;
            ptr = ScratchPointer2;
        }

        ptr = rplSkipOb(ptr);
    }

    // DONE EXTRACTING, NOW CLEANUP THE STACK

    WORDPTR *stkptr = savestk;
    WORDPTR *dest = stkptr;
    while(stkptr != DSTop) {
        if(ISIDENT(**stkptr)) {
            if(!(rplGetIdentAttr(*stkptr) & IDATTR_DONTUSE)) {
                // KEEP THIS GLOBAL
                *dest = *stkptr;
                ++stkptr;
                ++dest;
                continue;
            }

        }
        ++stkptr;
    }

    DSTop = stkptr;

    return DSTop - savestk;
}

// GIVEN AN EQUATION OR A PROGRAM, REMOVE
// USES 5 SCRATCHPOINTERS
void rplUpdateDependencyTree(WORDPTR varname, WORDPTR * dir, WORDPTR olddefn,
        WORDPTR newdefn)
{

    WORDPTR *stkbase = DSTop;
    BINT oldnum, newnum;

    ScratchPointer4 = varname;
    ScratchPointer5 = newdefn;
    oldnum = rplGetDependency(olddefn);
    newdefn = ScratchPointer5;
    newnum = rplGetDependency(newdefn);
    varname = ScratchPointer4;

    BINT k;
    BINT hasdepend;
    for(k = 0; k < oldnum; ++k) {
        WORDPTR *var = rplFindGlobalPropInDir(stkbase[k], IDPROP_DEPN, dir, 0);
        if(var) {
            if(ISLIST(*var[1])) {
                WORDPTR ptr = var[1] + 1, end = rplSkipOb(var[1]);
                BINT totalsize = 0, addvar = 0;

                // COMPUTE THE SIZE OF THE LIST AFTER REMOVING THE OLD DEPENDENCY
                while(ptr < end) {
                    if(ISIDENT(*ptr) && !rplCompareIDENT(ptr, varname))
                        totalsize += rplObjSize(ptr);
                    ptr = rplSkipOb(ptr);
                }

                // ADD THE SIZE OF THE NEW VARNAME IF IT IS IN THE NEW DEPENDENCY
                BINT j;
                for(j = 0; j < newnum; ++j) {
                    if(rplCompareIDENT(stkbase[k], stkbase[oldnum + j])) {
                        totalsize += rplObjSize(varname);
                        ++addvar;
                        // REMOVE FROM THE NEW DEPENDENCY LIST
                        WORDPTR *movptr = stkbase + oldnum + j;
                        while(movptr < DSTop - 1) {
                            *movptr = *(movptr + 1);
                            ++movptr;
                        }
                        --DSTop;
                        --newnum;
                        break;
                    }
                }

                if(totalsize > 0) {
                    ScratchPointer1 = varname;
                    WORDPTR newlist = rplAllocTempOb(totalsize + 1), newlptr;
                    if(!newlist) {
                        DSTop = stkbase;
                        return;
                    }
                    varname = ScratchPointer1;

                    newlist[0] = MKPROLOG(DOLIST, totalsize + 1);
                    ptr = var[1];
                    end = rplSkipOb(ptr);
                    ++ptr;
                    newlptr = newlist + 1;
                    // COPY ALL OLD DEPENDENT VARS EXCEPT THE NEW ONE
                    while(ptr < end) {
                        if(ISIDENT(*ptr) && !rplCompareIDENT(ptr, varname)) {
                            rplCopyObject(newlptr, ptr);
                            newlptr = rplSkipOb(newlptr);
                        }
                        ptr = rplSkipOb(ptr);
                    }
                    if(addvar) {
                        rplCopyObject(newlptr, varname);
                        newlptr = rplSkipOb(newlptr);
                    }

                    *newlptr = CMD_ENDLIST;

                    // HERE newlptr-newlist == totalsize+1

                    var[1] = newlist;
                    hasdepend = 1;
                }
                else {
                    // REMOVE THE PROPERTY ALTOGETHER
                    rplPurgeForced(var);
                    hasdepend = 0;
                }

            }
            else
                hasdepend = 0;
        }
        else {
            // THIS DEPENDENCY TREE WAS BROKEN,CHECK IF WE SHOULD ADD THE DEPENDENCY
            hasdepend = 0;
            BINT j, addvar = 0;
            for(j = 0; j < newnum; ++j) {
                if(rplCompareIDENT(stkbase[k], stkbase[oldnum + j])) {
                    ++addvar;
                    // REMOVE FROM THE NEW DEPENDENCY LIST
                    WORDPTR *movptr = stkbase + oldnum + j;
                    while(movptr < DSTop - 1) {
                        *movptr = *(movptr + 1);
                        ++movptr;
                    }
                    --DSTop;
                    --newnum;
                    break;
                }
            }

            if(addvar) {
                ScratchPointer1 = varname;
                WORDPTR newlist = rplAllocTempOb(rplObjSize(varname) + 1);
                if(!newlist) {
                    DSTop = stkbase;
                    return;
                }
                varname = ScratchPointer1;

                newlist[0] = MKPROLOG(DOLIST, rplObjSize(varname) + 1);
                rplCopyObject(newlist + 1, varname);
                newlist[1] = MKPROLOG(LIBNUM(newlist[1]) & ~(UNQUOTED_BIT), OBJSIZE(newlist[1]));       // MAKE SURE THE IDENT IS QUOTED
                newlist[rplObjSize(varname) + 1] = CMD_ENDLIST;
                ScratchPointer3 = varname;
                rplCreateGlobalPropInDir(stkbase[k], IDPROP_DEPN, newlist, dir);
                varname = ScratchPointer3;
                hasdepend = 1;
            }

        }

        // FIX THE HASDEPEND ATTRIBUTE

        var = rplFindGlobalInDir(stkbase[k], dir, 0);
        if(var) {
            WORD attr = rplGetIdentAttr(var[0]);
            if((attr & IDATTR_DEPEND) && !hasdepend) {
                ScratchPointer2 = varname;
                WORDPTR newname = rplSetIdentAttr(var[0], 0, IDATTR_DEPEND);
                if(newname)
                    var[0] = newname;
                varname = ScratchPointer2;
            }
            else if(!(attr & IDATTR_DEPEND) && hasdepend) {
                ScratchPointer2 = varname;
                WORDPTR newname =
                        rplSetIdentAttr(var[0], IDATTR_DEPEND, IDATTR_DEPEND);
                if(newname)
                    var[0] = newname;
                varname = ScratchPointer2;

            }
        }

    }

// DONE UPDATING OLD DEPENDENCIES, NOW ADD THE NEW ONES

    for(k = 0; k < newnum; ++k) {
        WORDPTR *var =
                rplFindGlobalPropInDir(stkbase[oldnum + k], IDPROP_DEPN, dir,
                0);
        if(var) {
            if(ISLIST(*var[1])) {
                WORDPTR ptr = var[1] + 1, end = rplSkipOb(var[1]);
                BINT totalsize = 0;

                // COMPUTE THE SIZE OF THE LIST AFTER REMOVING THE OLD DEPENDENCY
                while(ptr < end) {
                    if(ISIDENT(*ptr) && !rplCompareIDENT(ptr, varname))
                        totalsize += rplObjSize(ptr);
                    ptr = rplSkipOb(ptr);
                }

                // ADD THE SIZE OF THE NEW VARNAME
                totalsize += rplObjSize(varname);

                ScratchPointer1 = varname;
                WORDPTR newlist = rplAllocTempOb(totalsize + 1), newlptr;
                if(!newlist) {
                    DSTop = stkbase;
                    return;
                }
                varname = ScratchPointer1;

                newlist[0] = MKPROLOG(DOLIST, totalsize + 1);
                ptr = var[1] + 1;
                end = rplSkipOb(ptr);
                newlptr = newlist + 1;
                // COPY ALL OLD DEPENDENT VARS EXCEPT THE NEW ONE
                while(ptr < end) {
                    if(ISIDENT(*ptr) && !rplCompareIDENT(ptr, varname)) {
                        rplCopyObject(newlptr, ptr);
                        newlptr = rplSkipOb(newlptr);
                    }
                    ptr = rplSkipOb(ptr);
                }
                rplCopyObject(newlptr, varname);
                newlptr = rplSkipOb(newlptr);

                *newlptr = CMD_ENDLIST;

                // HERE newlptr-newlist == totalsize+1

                var[1] = newlist;
                continue;
            }
        }
        // NOT A LIST? CORRUPTED DEPENDENCY TREE, CREATE A NEW ONE

        ScratchPointer1 = varname;
        WORDPTR newlist = rplAllocTempOb(rplObjSize(varname) + 1);
        if(!newlist) {
            DSTop = stkbase;
            return;
        }
        varname = ScratchPointer1;

        newlist[0] = MKPROLOG(DOLIST, rplObjSize(varname) + 1);
        rplCopyObject(newlist + 1, varname);
        newlist[1] &= ~MKOPCODE(UNQUOTED_BIT, 0);       // MAKE SURE THE IDENT IS QUOTED
        newlist[rplObjSize(varname) + 1] = CMD_ENDLIST;
        ScratchPointer3 = varname;

        if(var)
            var[1] = newlist;
        else
            rplCreateGlobalPropInDir(stkbase[k], IDPROP_DEPN, newlist, dir);
        varname = ScratchPointer3;

    }

// SET THE HASDEPEND ATTRIBUTE ON ALL NEW DEPENDENCIES
    for(k = 0; k < newnum; ++k) {
        WORDPTR *var = rplFindGlobalInDir(stkbase[oldnum + k], dir, 0);
        if(var) {
            WORD attr = rplGetIdentAttr(var[0]);
            if(!(attr & IDATTR_DEPEND)) {
                WORDPTR newname =
                        rplSetIdentAttr(var[0], IDATTR_DEPEND, IDATTR_DEPEND);
                if(newname)
                    var[0] = newname;

            }
        }
    }
// ALL DEPENDENCIES UPDATED
    DSTop = stkbase;

}

// COMPUTE SIZE OF DIRECTORY TREE (INCLUDES SIZE OF ALL NAMES, OBJECTS, AND PACKED SUBDIRECTORY OBJECTS)

BINT rplGetDirSize(WORDPTR * directory)
{
    WORDPTR *entry = rplFindFirstInDir(directory);
    BINT size = 0;
    while(entry) {
        if(ISIDENT(*entry[0])) {
            // ONLY CONSIDER DIRECTORY ENTRIES THAT HAVE PROPER NAMES
            size += rplObjSize(entry[0]);
            if(ISDIR(*entry[1])) {
                WORDPTR *subdir = rplFindDirbyHandle(entry[1]);
                if(!subdir)
                    size += rplObjSize(entry[1]);
                else
                    size += rplGetDirSize(subdir) + 1;
            }
            else
                size += rplObjSize(entry[1]);
        }
        entry = rplFindNext(entry);
    }

    return size;
}

// PACKS AN ENTIRE DIRECTORY TREE IN A PREALLOCATED AREA OF MEMORY

void rplPackDirinPlace(WORDPTR * directory, WORDPTR where)
{
    WORDPTR ptr = where + 1;
    WORDPTR *entry = rplFindFirstInDir(directory);
    while(entry) {
        if(ISIDENT(*entry[0])) {
            // ONLY PACK ENTRIES THAT HAVE A VALID NAME
            rplCopyObject(ptr, entry[0]);
            ptr = rplSkipOb(ptr);

            if(ISDIR(*entry[1])) {
                WORDPTR *subdir = rplFindDirbyHandle(entry[1]);
                if(!subdir)
                    rplCopyObject(ptr, entry[1]);
                else
                    rplPackDirinPlace(subdir, ptr);
            }
            else
                rplCopyObject(ptr, entry[1]);
            ptr = rplSkipOb(ptr);
        }
        entry = rplFindNext(entry);
    }
    *where = MKPROLOG(DOPACKDIR, (ptr - where) - 1);

}
