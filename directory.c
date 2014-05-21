/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "hal.h"
#include "libraries.h"


WORD dir_start_bint[]=
{
    (WORD)DIR_START_MARKER
};
WORD dir_end_bint[]=
{
    (WORD)DIR_END_MARKER
};
WORD dir_parent_bint[]=
{
    (WORD)DIR_PARENT_MARKER
};



// GROW THE DIRECTORY REGION

void growDirs(WORD newtotalsize)
{
    WORDPTR *newdir;
    BINT gc_done=0;
    do {
    newtotalsize=(newtotalsize+1023)&~1023;

    newdir=hal_growmem(Directories,newtotalsize);

    if(!newdir) {
        if(!gc_done) { rplGCollect(); ++gc_done; }
        else {
        Exceptions|=EX_OUTOFMEM;
        ExceptionPointer=IPtr;
        return;
        }
    }

    } while(!newdir);

        CurrentDir=CurrentDir-Directories+newdir;
        DirsTop=DirsTop-Directories+newdir;
        Directories=newdir;
        DirSize=newtotalsize;
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



#define MakeNewHole(start,end,nwords)    memmove(start+nwords,start,((WORD)(end-start))<<2)


// DIRS STACK IS INCREASE AFTER FOR STORE, DECREASE BEFORE FOR READ

// CREATE A GLOBAL IN THE GIVEN DIRECTORY

void rplCreateGlobalInDir(WORDPTR nameobj,WORDPTR value,WORDPTR *parentdir)
{
    WORDPTR *direntry=parentdir+4; // POINT TO THE FIRST ENTRY IN THE DIRECTORY

    // OPEN A HOLE FOR A NEW VARIABLE AT BEGINNING OF CURRENT DIR
    MakeNewHole(direntry,DirsTop,2);
    // INCREASE THE END OF DIRS
    DirsTop+=2;
    *direntry=nameobj;
    *(direntry+1)=value;
    // PATCH THE CURRENT DIRECTORY SIZE
    WORDPTR size=*(parentdir+1)+1;
    ++*size;

    // FIX THE CURRENT DIR IN CASE IT MOVED
    if(CurrentDir>=direntry) CurrentDir+=2;


    if(DirSize<=DirsTop-Directories+DIRSLACK) growDirs((WORD)(DirsTop-Directories+DIRSLACK+1024));
    if(Exceptions) return;

}


void rplCreateGlobal(WORDPTR nameobj,WORDPTR value)
{
    rplCreateGlobalInDir(nameobj,value,CurrentDir);
}


// FIND THE GIVEN DIRECTORY, OR RETURN NULL

WORDPTR *rplFindDirbyHandle(WORDPTR handle)
{

if(!handle) return 0;

WORDPTR *scan=Directories;

while(scan<DirsTop) {
    if(*(scan+1)==handle) return scan;
    scan+=6+(*(*(scan+1)+1)<<1); //     POINT TO THE NEXT DIRECTORY
}

return 0;
}


// CREATE A NEW EMPTY DIRECTORY AT THE PARENT DIRECTORY

// NOTE: THIS FUNCTION RELIES ON DIRSLACK>8
void rplCreateNewDir(WORDPTR name,WORDPTR *parentdir)
{
    WORDPTR *dirptr=rplMakeNewDir();

    if(!dirptr) return;

    // LINK TO PARENT DIR HANDLE
    dirptr[3]=parentdir[1];

    WORDPTR *var=rplFindGlobal(name,0);
    if(var) {
        *(var+1)=(WORDPTR)dirptr;
    }
    else {
        // CREATE THE GLOBAL VARIABLE TO LINK THE DIRECTORY
        rplCreateGlobalInDir(name,dirptr[1],parentdir);
    }
}

// LOW LEVEL VERSION CREATES AN UNLINKED DIRECTORY, WITH NO PARENT

WORDPTR *rplMakeNewDir()
{
    // CREATE THE DIRECTORY OBJECT FIRST
    WORDPTR dirobj=rplAllocTempOb(1);

    if(!dirobj) return 0;

    // EMPTY DIRECTORY OBJECT
    dirobj[0]=MKPROLOG(DODIR,1);
    dirobj[1]=0;

    // NOW CREATE THE NEW DIR

    WORDPTR *direntry=DirsTop;
    DirsTop+=6;

    direntry[0]=dir_start_bint;
    direntry[1]=dirobj;
    direntry[2]=dir_parent_bint;
    direntry[3]=0;      // NO PARENT!!!
    direntry[4]=dir_end_bint;
    direntry[5]=dirobj;

    if(DirSize<=DirsTop-Directories+DIRSLACK) growDirs((WORD)(DirsTop-Directories+DIRSLACK+1024));
    if(Exceptions) return 0;

    return direntry;
}



// GET THE ADDRESS OF THE PARENT DIRECTORY
WORDPTR *rplGetParentDir(WORDPTR *directory)
{
    return rplFindDirbyHandle(directory[3]);
}


// FINDS A GLOBAL, AND RETURNS THE ADDRESS OF THE KEY/VALUE PAIR WITHIN THE DIRECTORY ENVIRONMENT

WORDPTR *rplFindGlobalbyName(BYTEPTR name,BINT len,BINT scanparents)
{
    WORDPTR *direntry=CurrentDir+4;
    WORDPTR parentdir;

    do {
    parentdir=*(direntry-3);
    while(direntry<DirsTop) {
        if(*direntry==dir_end_bint) break;
        if(rplCompareIDENTByName(*direntry,name,len)) return direntry;
        direntry+=2;
    }
    direntry=rplFindDirbyHandle(parentdir);
    } while(scanparents && direntry);
return 0;
}

WORDPTR *rplFindGlobal(WORDPTR nameobj,BINT scanparents)
{
    WORDPTR *direntry=CurrentDir;
    WORDPTR parentdir;

    do {
    parentdir=*(direntry+3);
    direntry+=4;    // SKIP SELF REFERENCE AND PARENT DIR
    while(direntry<DirsTop) {
        if(*direntry==dir_end_bint) break;
        if(rplCompareIDENT(*direntry,nameobj)) return direntry;
        direntry+=2;
    }
    direntry=rplFindDirbyHandle(parentdir);
    } while(scanparents && direntry);
return 0;
}


// RCL A GLOBAL, RETURN POINTER TO ITS VALUE
// LOOKS IN CURRENT DIR AND PARENT DIRECTORIES
WORDPTR rplGetGlobal(WORDPTR nameobj)
{
    WORDPTR *var=rplFindGlobal(nameobj,1);
    if(var) return *(var+1);
    return 0;
}


// OBTAIN THE DIRECTORY IN WHICH THIS GLOBAL IS
// WORK FROM A POINTER TO THE KEY/VALUE PAIR OF THE GLOBAL
// AND RETURN THE START OF THE DIRECTORY

WORDPTR *rplGetDirfromGlobal(WORDPTR *var)
{
    while((*var!=dir_start_bint)&& (var>Directories)) var-=2;
    return var;
}



// PURGE A SINGLE VARIABLE

void rplPurgeGlobal(WORDPTR nameobj)
{
    WORDPTR *var=rplFindGlobal(nameobj,1);

    if(!var) {
        Exceptions|=EX_VARUNDEF;
        ExceptionPointer=IPtr;
        return;
    }

    if(ISPROLOG(**(var+1)) && (LIBNUM(**(var+1))==DODIR)) {
        // TRYING TO PURGE AN ENTIRE DIRECTORY

        WORD dirsize=*(*(var+1)+1);

        // NEED TO USE PGDIR FOR THAT
        if(dirsize) {
            Exceptions|=EX_NONEMPTYDIR;
            ExceptionPointer=IPtr;
            return;
        }

        // REMOVE THE EMPTY DIR

        WORDPTR *emptydir=rplFindDirbyHandle(*(var+1));

        if(CurrentDir==emptydir) {
            CurrentDir=rplFindDirbyHandle(emptydir[3]); // CHANGE CURRENT DIR TO PARENT
        }

        if(emptydir) {
            MakeNewHole(emptydir+6,DirsTop,-6);     // THIS WILL REMOVE THE EMPTY DIRECTORY IN MEMORY
            if(CurrentDir>=emptydir) CurrentDir-=6;
            DirsTop-=6;
        }

    }

    MakeNewHole(var+2,DirsTop,-2);      // AND REMOVE THE ENTRY
    // UPDATE THE DIRECTORY COUNT
    WORDPTR *dir=rplGetDirfromGlobal(var);
    --*(*(dir+1)+1);

    if(CurrentDir>=var) CurrentDir-=2;
    DirsTop-=2;

    return;
}

WORDPTR rplGetDirName(WORDPTR *dir)
{
    WORDPTR *parent=dir+3;
    if(!*parent) return 0;
    parent=rplFindDirbyHandle(*parent);


    while(*parent!=dir_end_bint) {
        if(*(parent+1)==*(dir+1)) return *parent;
        parent+=2;
    }
    return 0;
}



// CREATE A LIST WITH THE FULL PATH TO THE GIVEN DIRECTORY
// dir IS A POINTER TO THE DIRECTORY, NOT A HANDLE!

WORDPTR *rplGetPath(WORDPTR *dir)
{
    // TODO: NEED TO HAVE LISTS IMPLEMENTED TO RETURN A LIST!!
    return NULL;
}


// CREATE A COMPLETELY NEW COPY OF THE DIRECTORY
// AND ALL ITS SUBDIRECTORIES
WORDPTR *rplDeepCopyDir(WORDPTR *sourcedir)
{
    WORDPTR *targetdir=rplMakeNewDir();
    if(!targetdir) return 0;
    WORDPTR *srcvars=sourcedir+4;
    while(*srcvars!=dir_end_bint) {
        if(LIBNUM(**(srcvars+1))==DODIR) {
            //POINTS TO A DIRECTORY, NEED TO DEEP-COPY THAT ONE TOO
            WORDPTR *subdir=rplDeepCopyDir(rplFindDirbyHandle(*(srcvars+1)));

            if(subdir) {
                WORDPTR subhandle=*(subdir+1);
                rplCreateGlobalInDir(*srcvars,subhandle,targetdir);
                // CREATION OF A GLOBAL MIGHT HAVE MOVED subdir, SO WE NEED TO FIND IT
                subdir=rplFindDirbyHandle(subhandle);
                *(subdir+3)=*(targetdir+1);
            }
        }
        else rplCreateGlobalInDir(*srcvars,*(srcvars+1),targetdir);
        srcvars+=2;
        }

    return targetdir;

}
