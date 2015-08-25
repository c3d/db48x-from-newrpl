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
    BINT gc_done=0;
    do {
    newtotalsize=(newtotalsize+1023)&~1023;

    newdir=halGrowMemory(MEM_AREA_DIR,Directories,newtotalsize);

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

// CHECK IF AN IDENT IS QUOTED, IF NOT THEN
// CREATE A NEW QUOTED OBJECT AND RETURN IT
// MAY CAUSE GARBAGE COLLECTION
WORDPTR rplMakeIdentQuoted(WORDPTR ident)
{
    if(!ISHIDDENIDENT(*ident)) return ident;

    ident=rplMakeNewCopy(ident);

    //  CHANGE FROM A IDENTEVAL TO A REGULAR IDENT
    ident[0]-=MKOPCODE(DOIDENTEVAL-DOIDENT,0);

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

WORDPTR *scan=Directories;

while(scan<DirsTop) {
    if(*(scan+1)==handle) return scan;
    scan+=6+(*(*(scan+1)+1)<<1); //     POINT TO THE NEXT DIRECTORY
}

return 0;
}


// CREATE A NEW EMPTY DIRECTORY AT THE PARENT DIRECTORY

// NOTE: THIS FUNCTION RELIES ON DIRSLACK>8
// USES 2 SCRATCH POINTERS
// RETURNS POINTER TO HANDLE OBJECT
WORDPTR rplCreateNewDir(WORDPTR name,WORDPTR *parentdir)
{
    ScratchPointer1=name;

    WORDPTR *dirptr=rplMakeNewDir();    // MAY CAUSE A GC

    if(!dirptr) return NULL;

    // LINK TO PARENT DIR HANDLE
    dirptr[3]=parentdir[1];


    WORDPTR *var=rplFindGlobalInDir(ScratchPointer1,parentdir,0);
    if(var) {
        // THAT NAME IS BEING USED, OVERWRITE
        *(var+1)=(WORDPTR)dirptr[1];
        return dirptr[1];

    }
    else {
        // PROTECT THE HANDLE FROM MOVING DURING A GC
        ScratchPointer2=dirptr[1];
        // CREATE THE GLOBAL VARIABLE TO LINK THE DIRECTORY
        rplCreateGlobalInDir(ScratchPointer1,dirptr[1],parentdir);
        return ScratchPointer2;
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

    direntry[0]=(WORDPTR)dir_start_bint;
    direntry[1]=(WORDPTR)dirobj;
    direntry[2]=(WORDPTR)dir_parent_bint;
    direntry[3]=(WORDPTR)root_dir_handle;      // NO PARENT!!!
    direntry[4]=(WORDPTR)dir_end_bint;
    direntry[5]=(WORDPTR)dirobj;

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

WORDPTR *rplFindGlobalbyNameInDir(BYTEPTR name,BINT len,WORDPTR *parent,BINT scanparents)
{
    WORDPTR *direntry=parent+4;
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

WORDPTR *rplFindGlobalInDir(WORDPTR nameobj,WORDPTR *parent,BINT scanparents)
{
    WORDPTR *direntry=parent;
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

WORDPTR *rplFindGlobal(WORDPTR nameobj,BINT scanparents)
{
    return rplFindGlobalInDir(nameobj,CurrentDir,scanparents);
}

// GET A POINTER TO THE KEY/VALUD PAIR AT idx WITHIN directory
// RETURN NULL IF idx IS NOT WITHIN 0<idx<NITEMS IN DIRECTORY

WORDPTR *rplFindGlobalByIndexInDir(BINT idx,WORDPTR *directory)
{
    BINT nitems=*(directory[1]+1);

    if( (idx<0) || (idx>=nitems)) return 0;
    return directory+4+2*idx;
}

// SAME AS ABOVE BUT FOR CURRENT DIRECTORY ONLY
WORDPTR *rplFindGlobalByIndex(BINT idx)
{
    return rplFindGlobalByIndexInDir(idx,CurrentDir);
}

// GET TOTAL NUMBER OF VARIABLES IN THE DIRECTORY
BINT rplGetVarCountInDir(WORDPTR *directory)
{
    return *(directory[1]+1);
}

BINT rplGetVarCount()
{
    return *(CurrentDir[1]+1);
}


// GET TOTAL NUMBER OF VISIBLE VARIABLES IN THE DIRECTORY
// SKIPS ANY VARIABLES WHERE THE KEY IS NOT A NUMBER OR
// ANY IDENTS THAT ARE MARKED AS HIDDEN (BY PROLOG DOIDENTEVAL...)
BINT rplGetVisibleVarCountInDir(WORDPTR *directory)
{
    BINT n=0;
    WORDPTR *dirptr=directory+4;
    while(*dirptr!=dir_end_bint) {
        if(ISIDENT(**dirptr) && !ISHIDDENIDENT(**dirptr)) ++n;
        dirptr+=2;
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






// LOW LEVEL VERSION OF PURGE
void rplPurgeForced(WORDPTR *var)
{
    if(ISPROLOG(**(var+1)) && (LIBNUM(**(var+1))==DODIR)) {
        // TRYING TO PURGE AN ENTIRE DIRECTORY

        WORD dirsize=*(*(var+1)+1);

        // NEED TO USE PGDIR FOR THAT
        if(dirsize) return;


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

    }

    rplPurgeForced(var);
}







WORDPTR rplGetDirName(WORDPTR *dir)
{
    WORDPTR *parent=dir+3;

    parent=rplFindDirbyHandle(*parent);

    if(!parent) return 0;

    while(*parent!=dir_end_bint) {
        if(*(parent+1)==*(dir+1)) return *parent;
        parent+=2;
    }
    return 0;
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


void rplStoreSettings(WORDPTR nameobject,WORDPTR object)
{
    rplCreateGlobalInDir(nameobject,object,rplFindDirbyHandle(SettingsDir));
}

void rplStoreSettingsbyName(BYTEPTR name,BINT namelen,WORDPTR object)
{
    WORDPTR *setting=rplFindGlobalbyNameInDir(name,namelen,rplFindDirbyHandle(SettingsDir),0);
    if(setting) {
        setting[1]=object;
    }
}

// GET THE SETTINGS AND RETURN A POINTER TO THE OBJECT, OR NULL IF IT DOESN'T EXIST
WORDPTR rplGetSettings(WORDPTR nameobject)
{
    WORDPTR *setting=rplFindGlobalInDir(nameobject,rplFindDirbyHandle(SettingsDir),0);
    if(setting) return setting[1];
    return 0;
}

WORDPTR rplGetSettingsbyName(BYTEPTR name,BINT namelen)
{
    WORDPTR *setting=rplFindGlobalbyNameInDir(name,namelen,rplFindDirbyHandle(SettingsDir),0);
    if(setting) return setting[1];
    return 0;
}

