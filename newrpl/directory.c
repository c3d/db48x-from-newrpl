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
            rplException(EX_OUTOFMEM);
        return;
        }
    }

    } while(!newdir);

        CurrentDir=CurrentDir-Directories+newdir;
        DirsTop=DirsTop-Directories+newdir;
        Directories=newdir;
        DirSize=newtotalsize;
}

// SIMILAR TO GROW, BUT DOES NOT DO GARBAGE COLLECTION
// USED TO RELEASE PAGES RIGHT BEFORE A GC
void shrinkDirs(WORD newtotalsize)
{
    WORDPTR *newdir;

    newtotalsize=(newtotalsize+1023)&~1023;

    newdir=halGrowMemory(MEM_AREA_DIR,Directories,newtotalsize);

    if(!newdir) {
        rplException(EX_OUTOFMEM);
        return;
        }
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
    if(ISQUOTEDIDENT(*ident)) return ident;

    ident=rplMakeNewCopy(ident);

    //  CHANGE FROM A IDENTEVAL TO A REGULAR IDENT
    if(ident) ident[0]&=~MKOPCODE(UNQUOTED_BIT,0);

    return ident;

}

// CHECK IF AN IDENT IS UNQUOTED, IF NOT THEN
// CREATE A NEW UNQUOTED OBJECT AND RETURN IT
// MAY CAUSE GARBAGE COLLECTION
WORDPTR rplMakeIdentUnquoted(WORDPTR ident)
{
    if(ISUNQUOTEDIDENT(*ident)) return ident;

    ident=rplMakeNewCopy(ident);

    //  CHANGE FROM A IDENTEVAL TO A REGULAR IDENT
    if(ident) ident[0]|=MKOPCODE(UNQUOTED_BIT,0);

    return ident;


}


WORDPTR rplMakeIdentHidden(WORDPTR ident)
{
    if(ISHIDDENIDENT(*ident)) return ident;

    ident=rplMakeNewCopy(ident);

    //  CHANGE FROM A IDENTEVAL TO A REGULAR IDENT
    if(ident) ident[0]|=MKOPCODE(HIDDEN_BIT,0);

    return ident;

}

// CHECK IF AN IDENT IS UNQUOTED, IF NOT THEN
// CREATE A NEW UNQUOTED OBJECT AND RETURN IT
// MAY CAUSE GARBAGE COLLECTION
WORDPTR rplMakeIdentVisible(WORDPTR ident)
{
    if(!ISHIDDENIDENT(*ident)) return ident;

    ident=rplMakeNewCopy(ident);

    //  CHANGE FROM A IDENTEVAL TO A REGULAR IDENT
    if(ident) ident[0]&=~MKOPCODE(HIDDEN_BIT,0);

    return ident;
}


WORDPTR rplMakeIdentReadOnly(WORDPTR ident)
{
    if(ISLOCKEDIDENT(*ident)) return ident;

    ident=rplMakeNewCopy(ident);

    //  CHANGE FROM A IDENTEVAL TO A REGULAR IDENT
    if(ident) ident[0]|=MKOPCODE(READONLY_BIT,0);

    return ident;

}

// CHECK IF AN IDENT IS LOCKED, IF NOT THEN
// CREATE A NEW UNLOCKED OBJECT AND RETURN IT
// MAY CAUSE GARBAGE COLLECTION
WORDPTR rplMakeIdentWriteable(WORDPTR ident)
{
    if(!ISLOCKEDIDENT(*ident)) return ident;

    ident=rplMakeNewCopy(ident);

    //  CHANGE FROM A IDENTEVAL TO A REGULAR IDENT
    if(ident) ident[0]&=~MKOPCODE(READONLY_BIT,0);

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

    if(DirSize<=DirsTop-Directories+DIRSLACK) growDirs((WORD)(DirsTop-Directories+DIRSLACK));


    //if(Exceptions) return;

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
WORDPTR rplCreateNewDir(WORDPTR nameobj,WORDPTR *parentdir)
{
    ScratchPointer1=nameobj;

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
    direntry[3]=(WORDPTR)root_dir_handle;     // NO PARENT!!!
    direntry[4]=(WORDPTR)dir_end_bint;
    direntry[5]=(WORDPTR)dirobj;

    if(DirSize<=DirsTop-Directories+DIRSLACK) growDirs((WORD)(DirsTop-Directories+DIRSLACK));
    if(Exceptions) return 0;

    return direntry;
}



// GET THE ADDRESS OF THE PARENT DIRECTORY
WORDPTR *rplGetParentDir(WORDPTR *directory)
{
    if(!directory) return 0;
    return rplFindDirbyHandle(directory[3]);
}


// FINDS A GLOBAL, AND RETURNS THE ADDRESS OF THE KEY/VALUE PAIR WITHIN THE DIRECTORY ENVIRONMENT

WORDPTR *rplFindGlobalbyName(BYTEPTR name,BYTEPTR nameend,BINT scanparents)
{
    WORDPTR *direntry=CurrentDir+4;
    WORDPTR parentdir;

    if(!CurrentDir) return 0;

    do {
    parentdir=*(direntry-3);
    while(direntry<DirsTop) {
        if(**direntry==DIR_END_MARKER) break;
        if(rplCompareIDENTByName(*direntry,name,nameend)) return direntry;
        direntry+=2;
    }
    direntry=rplFindDirbyHandle(parentdir);
    } while(scanparents && direntry);
return 0;
}

WORDPTR *rplFindGlobalbyNameInDir(BYTEPTR name,BYTEPTR nameend,WORDPTR *parent,BINT scanparents)
{
    WORDPTR *direntry=parent+4;
    WORDPTR parentdir;

    if(!parent) return 0;
    do {
    parentdir=*(direntry-3);
    while(direntry<DirsTop) {
        if(**direntry==DIR_END_MARKER) break;
        if(rplCompareIDENTByName(*direntry,name,nameend)) return direntry;
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

    if(!parent) return 0;

    do {
    parentdir=*(direntry+3);
    direntry+=4;    // SKIP SELF REFERENCE AND PARENT DIR
    while(direntry<DirsTop) {
        if(**direntry==DIR_END_MARKER) break;
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

WORDPTR *rplFindVisibleGlobalByIndexInDir(BINT idx,WORDPTR *directory)
{
    BINT nitems=*(directory[1]+1);

    if( (idx<0) || (idx>=nitems)) return 0;
    BINT k=4;
    while(*directory[k]!=DIR_END_MARKER) {
        if(ISIDENT(*directory[k]) && !ISHIDDENIDENT(*directory[k])) --idx;
        if(idx<0) return directory+k;
        k+=2;
    }
    return 0;
}

// SAME AS ABOVE BUT FOR CURRENT DIRECTORY ONLY
WORDPTR *rplFindVisibleGlobalByIndex(BINT idx)
{
    return rplFindVisibleGlobalByIndexInDir(idx,CurrentDir);
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
    while(**dirptr!=DIR_END_MARKER) {
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
    while((var>=Directories) && (*var) && (**var!=DIR_START_MARKER) ) var-=2;
    return var;
}






// LOW LEVEL VERSION OF PURGE
void rplPurgeForced(WORDPTR *var)
{
    if(ISPROLOG(**(var+1)) && (LIBNUM(**(var+1))==DODIR)) {
        // TRYING TO PURGE AN ENTIRE DIRECTORY

        WORD dirsize=*(*(var+1)+1);
        WORDPTR *emptydir=rplFindDirbyHandle(*(var+1));

        if(dirsize) {
            // EITHER DIRECTORY IS FULL OR THIS IS AN ORPHAN HANDLER
            if(emptydir) {
                // DIRECTORY IS NOT EMPTY!
                return;
            }

            // DIRECTORY NO LONGER EXISTS, PROBABLY WIPED OUT PREVIOUSLY, IT'S OK TO PURGE

        }


        // REMOVE THE EMPTY DIR


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
    WORDPTR *var=rplFindGlobal(nameobj,0);

    if(!var) {
        //rplError(ERR_UNDEFINEDVARIABLE);
        return;
    }

    if(ISPROLOG(**(var+1)) && (LIBNUM(**(var+1))==DODIR)) {
        // TRYING TO PURGE AN ENTIRE DIRECTORY

        WORD dirsize=*(*(var+1)+1);

        // NEED TO USE PGDIR FOR THAT
        if(dirsize) {
            rplError(ERR_NONEMPTYDIRECTORY);
            return;
        }

    }

    if(ISLOCKEDIDENT(**var)) { rplError(ERR_READONLYVARIABLE); return; }

    rplPurgeForced(var);
}






WORDPTR rplGetDirName(WORDPTR *dir)
{
    WORDPTR *parent=dir+3;

    parent=rplFindDirbyHandle(*parent);

    if(!parent) return 0;

    while(**parent!=DIR_END_MARKER) {
        if(*(parent+1)==*(dir+1)) return *parent;
        parent+=2;
    }
    return 0;
}

// FILL A BUFFER WITH POINTERS TO THE NAME OBJECTS TO ALL PARENTS
// FIRST POINTER IN THE BUFFER IS GIVEN DIR'S NAME, LAST POINTER IS HOME
// OR THE NAME AT THE MAXIMUM REQUESTED DEPTH
// buffer MUST BE PREALLOCATED AND CONTAIN SPACE FOR UP TO maxdepth POINTERS
// RETURNS THE NUMBER OF POINTERS STORED IN buffer
BINT rplGetFullPath(WORDPTR *dir,WORDPTR *buffer,BINT maxdepth)
{
    WORDPTR *parent,*pptr;
    BINT nptrs=0;

    while(dir && (nptrs<maxdepth)) {
    parent=rplFindDirbyHandle(dir[3]);

    if(!parent) return nptrs;
    pptr=parent;
    while(**pptr!=DIR_END_MARKER) {
        if(*(pptr+1)==*(dir+1)) break;
        pptr+=2;
    }

    if(**pptr!=DIR_END_MARKER) {
        // FOUND THE NAME
        buffer[nptrs]=*pptr;
        ++nptrs;
        dir=parent;
    }
    }
    return nptrs;
}


// CREATE A COMPLETELY NEW COPY OF THE DIRECTORY
// AND ALL ITS SUBDIRECTORIES
WORDPTR *rplDeepCopyDir(WORDPTR *sourcedir)
{
    WORDPTR *targetdir=rplMakeNewDir();
    if(!targetdir) return 0;
    WORDPTR *srcvars=sourcedir+4;
    while(**srcvars!=DIR_END_MARKER) {
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


// FIND FIRST ENTRY IN A DIRECTORY BY HANDLE
// RETURN NULL IF INVALID OR EMPTY DIRECTORY
WORDPTR *rplFindFirstByHandle(WORDPTR dirhandle)
{
    WORDPTR *direntry=rplFindDirbyHandle(dirhandle);
    if(!direntry) return 0;
    if(*direntry[4]==DIR_END_MARKER) return 0;  // DIRECTORY WAS EMPTY
    return direntry+4;  // FIRST ACTUAL VARIABLE IN THE DIRECTORY
}
// FIND FIRST ENTRY IN A DIRECTORY BY HANDLE
// RETURN NULL IF INVALID OR EMPTY DIRECTORY
WORDPTR *rplFindFirstInDir(WORDPTR *directory)
{
    WORDPTR *direntry=directory;
    if(!direntry) return 0;
    if(*direntry[4]==DIR_END_MARKER) return 0;  // DIRECTORY WAS EMPTY
    return direntry+4;  // FIRST ACTUAL VARIABLE IN THE DIRECTORY
}
// RETURN THE NEXT ENTRY IN A DIRECTORY
// OR NULL IF END OF DIR
WORDPTR *rplFindNext(WORDPTR *direntry)
{
    direntry+=2;
    if(**direntry==DIR_END_MARKER) return 0;
    return direntry;
}



// PURGE EVERYTHING IN THE DIRECTORY, RECURSIVELY

void rplWipeDir(WORDPTR *directory)
{
    WORDPTR *direntry=directory+4;
    WORDPTR *Stacksave=DSTop;

    if(!directory) return ;

    // PUSH THE HANDLE TO PURGE LATER
    rplNewBINTPush(direntry-directory,DECBINT);
    rplPushData(directory[1]);

    while(DSTop!=Stacksave) {

        directory=rplFindDirbyHandle(rplPopData());
        direntry=directory+rplReadBINT(rplPopData());
        if(!directory) continue;    // SAFEGUARD IN CASE OF DUPLICATED ENTRIES POINTING TO THE SAME DIRECTORY


    // CONTINUE SCANNING

    while(**direntry!=DIR_END_MARKER) {

    if(ISPROLOG(**(direntry+1)) && (LIBNUM(**(direntry+1))==DODIR)) {
        // TRYING TO PURGE AN ENTIRE DIRECTORY

        WORD dirsize=*(*(direntry+1)+1);

        // NEED TO USE PGDIR FOR THAT
        if(dirsize) {
            // NON-EMPTY DIR, RECURSE INTO IT
            ScratchPointer1=*(direntry+1);  // PROTECT THE HANDLE FROM GC
            rplNewBINTPush(direntry+2-directory,DECBINT);   // RESUME AT THE FOLLOWING ENTRY
            rplPushData(directory[1]);
            if(Exceptions) { DSTop=Stacksave; return; }
            directory=rplFindDirbyHandle(ScratchPointer1);
            direntry=directory+4;
            continue;
        }
        else {
        // REMOVE THE EMPTY DIR

        WORDPTR *emptydir=rplFindDirbyHandle(*(direntry+1));

        if(CurrentDir==emptydir) {
            CurrentDir=rplFindDirbyHandle(emptydir[3]); // CHANGE CURRENT DIR TO PARENT
        }

        if(emptydir) {
            MakeNewHole(emptydir+6,DirsTop,-6);     // THIS WILL REMOVE THE EMPTY DIRECTORY IN MEMORY
            if(CurrentDir>=emptydir) CurrentDir-=6;
            DirsTop-=6;
            if(directory>=emptydir) directory-=6;
            if(direntry>=emptydir) direntry-=6;
        }

        }

    }

    direntry+=2;

    }

    // FINISHED SCANNING, THE DIRECTORY NOW CONTAINS ALL VARIABLES AND EMPTY DIRS

    // REMOVE ALL VARIABLES
    direntry+=2;
    MakeNewHole(direntry,DirsTop,-(direntry-directory));     // THIS WILL REMOVE ALL ENTRIES AT ONCE
    if(CurrentDir>=direntry) CurrentDir-=direntry-directory;
    DirsTop-=direntry-directory;
    direntry=directory;

    }

    // DONE, THE GIVEN DIRECTORY NO LONGER EXISTS
    return;

}


void rplPurgeDir(WORDPTR nameobj)
{
    WORDPTR *var=rplFindGlobal(nameobj,1);

    if(!var) {
        rplError(ERR_DIRECTORYNOTFOUND);
        return;
    }

    if(ISPROLOG(**(var+1)) && (LIBNUM(**(var+1))==DODIR)) {
        // TRYING TO PURGE AN ENTIRE DIRECTORY
        WORDPTR *dir=rplFindDirbyHandle(*(var+1));

        // RECURSIVELY WIPE OUT THE DIRECTORY
        rplWipeDir(dir);
        // GET DIRECTORY POINTERS FROM NAME AGAIN, SINCE WIPE DIR MIGHT'VE MOVED THE POINTERS!
        var=rplFindGlobal(nameobj,1);
        rplPurgeForced(var);
        return;

        }

    rplError(ERR_DIRECTORYNOTFOUND);
}




// CREATE OR MODIFY VARIABLES IN THE SETTINGS DIRECTORY
void rplStoreSettings(WORDPTR nameobject,WORDPTR object)
{
    if(!ISDIR(*SettingsDir)) return;
    WORDPTR *setting=rplFindGlobalInDir(nameobject,rplFindDirbyHandle(SettingsDir),0);
    if(setting) setting[1]=object;
    else rplCreateGlobalInDir(nameobject,object,rplFindDirbyHandle(SettingsDir));
}

void rplStoreSettingsbyName(BYTEPTR name,BYTEPTR nameend,WORDPTR object)
{
    if(!ISDIR(*SettingsDir)) return;

    WORDPTR *setting=rplFindGlobalbyNameInDir(name,nameend,rplFindDirbyHandle(SettingsDir),0);
    if(setting) {
        setting[1]=object;
    } else {

        ScratchPointer2=object;
        WORDPTR nameobject=rplCreateIDENT(DOIDENT,name,nameend);
        if(!nameobject) return;
        rplCreateGlobalInDir(nameobject,ScratchPointer2,rplFindDirbyHandle(SettingsDir));
    }
}

// GET THE SETTINGS AND RETURN A POINTER TO THE OBJECT, OR NULL IF IT DOESN'T EXIST
WORDPTR rplGetSettings(WORDPTR nameobject)
{
    if(!ISDIR(*SettingsDir)) return 0;
    WORDPTR *setting=rplFindGlobalInDir(nameobject,rplFindDirbyHandle(SettingsDir),0);
    if(setting) return setting[1];
    return 0;
}

WORDPTR rplGetSettingsbyName(BYTEPTR name,BYTEPTR nameend)
{
    if(!ISDIR(*SettingsDir)) return 0;
    WORDPTR *setting=rplFindGlobalbyNameInDir(name,nameend,rplFindDirbyHandle(SettingsDir),0);
    if(setting) return setting[1];
    return 0;
}

// PURGE A SINGLE VARIABLE

void rplPurgeSettings(WORDPTR nameobj)
{
    if(!ISDIR(*SettingsDir)) return;
    WORDPTR *var=rplFindGlobalInDir(nameobj,rplFindDirbyHandle(SettingsDir),0);

    if(!var) {
        return;
    }

    if(ISPROLOG(**(var+1)) && (LIBNUM(**(var+1))==DODIR)) {
        // TRYING TO PURGE AN ENTIRE DIRECTORY

        WORD dirsize=*(*(var+1)+1);

        // NEED TO USE PGDIR FOR THAT
        if(dirsize) {
            rplError(ERR_NONEMPTYDIRECTORY);
            return;
        }

    }

    rplPurgeForced(var);
}


// RETURN TRUE IF A VARIABLE IS VISIBLE IN A DIRECTORY
BINT rplIsVarVisible(WORDPTR *var)
{
    if(ISHIDDENIDENT(**var)) return 1;
    return 0;
}
// RETURN TRUE IF A VARIABLE IS LOCKED IN A DIRECTORY
BINT rplIsVarReadOnly(WORDPTR *var)
{
    if(ISLOCKEDIDENT(**var)) return 1;
    return 0;
}

// RETURN TRUE IF A VARIABLE IS IS A DIRECTORY
BINT rplIsVarDirectory(WORDPTR *var)
{
    if(ISPROLOG(**(var+1)) && (LIBNUM(**(var+1))==DODIR)) return 1;
    return 0;
}

// RETURN TRUE IF A VARIABLE IS IS AN EMPTY DIRECTORY
BINT rplIsVarEmptyDir(WORDPTR *var)
{
    if(ISPROLOG(**(var+1)) && (LIBNUM(**(var+1))==DODIR)) {
        WORD dirsize=*(*(var+1)+1);
        WORDPTR *emptydir=rplFindDirbyHandle(*(var+1));

        if(dirsize) {
            // EITHER DIRECTORY IS FULL OR THIS IS AN ORPHAN HANDLER
            if(!emptydir) return 1;  // DIRECTORY IS AN ORPHAN DIR, CONSIDER IT EMPTY
            return 0;
        }
        else return 1;
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

WORDPTR *rplFindDirFromPath(WORDPTR pathlist,BINT uselastname)
{
    WORDPTR ident, last;
    if(!ISLIST(*pathlist)) return CurrentDir;
    WORDPTR *dir=CurrentDir;

    last=rplSkipOb(pathlist)-1; // POINT TO CMD_ENDLIST

    if(!uselastname) {
        ident=pathlist+1;
        while(rplSkipOb(ident)<last) ident=rplSkipOb(ident);

        last=ident; // HERE IDENT POINTS TO THE LAST OBJECT IN THE LIST
    }

    ident=pathlist+1;


    while(ident<last) {
        if(*ident==CMD_HOME) dir=Directories;
        else if(*ident==CMD_UPDIR) dir=rplGetParentDir(dir);
        else if(ISIDENT(*ident)) {
            dir=rplFindGlobalInDir(ident,dir,0);
            if(dir) {
                if(!ISDIR(*dir[1])) { dir=0; }
                else dir=rplFindDirbyHandle(dir[1]);
            }
        }
        else dir=0;

        if(!dir) {
            rplError(ERR_DIRECTORYNOTFOUND);
            return 0;
        }
        ident=rplSkipOb(ident);
    }

    return dir;
}
