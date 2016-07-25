/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"




// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************



// REPLACE THE NUMBER
#define LIBRARY_NUMBER  74

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(SDRESET,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDSETPART,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDSTO,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDRCL,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDCHDIR,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDUPDIR,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDCRDIR,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDPGDIR,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDPURGE,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDOPENRD,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDOPENWR,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDOPENAPP,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDOPENMOD,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDCLOSE,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDREADTEXT,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDWRITETEXT,MKTOKENINFO(11,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDREADLINE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDSEEKSTA,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDSEEKEND,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDSEEKCUR,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDTELL,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDFILESIZE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDEOF,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDOPENDIR,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDNEXTFILE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDNEXTDIR,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDNEXTENTRY,MKTOKENINFO(11,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDMOVE,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDCOPY,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDPATH,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDFREE,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2))



// ADD MORE OPCODES HERE

#define ERROR_LIST \
    ERR(UNKNOWNFSERROR,0), \
    ERR(ENDOFFILE,1), \
    ERR(BADFILENAME,2), \
    ERR(BADVOLUME,3), \
    ERR(FILENOTFOUND,4), \
    ERR(CANTWRITE,5), \
    ERR(NOCARD,6), \
    ERR(CARDCHANGED,7), \
    ERR(MAXFILES,8), \
    ERR(ALREADYOPEN,9), \
    ERR(DISKFULL,10), \
    ERR(ALREADYEXISTS,11), \
    ERR(INVALIDHANDLE,12), \
    ERR(IDENTORPATHEXPECTED,13), \
    ERR(NOTANRPLFILE,14)




// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************


// ******************************************
// INCLUDE THIS FOR DEBUG ONLY - REMOVE WHEN DONE
#include "../firmware/sys/fsystem/fsyspriv.h"
// ******************************************




INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib74_menu);


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib74_menu,
    0
};

// CONVERT FILE SYSTEM ERROR MESSAGE INTO THIS LIBRARY ERRORS
BINT rplFSError2Error(BINT err)
{
    switch(err) {
    case FS_EOF:	    			// END OF FILE
        return ERR_ENDOFFILE;
    case FS_BADNAME:   			// INVALID FILE NAME
        return ERR_BADFILENAME;
    case FS_BADVOLUME: 			// INVALID DRIVE
        return ERR_BADVOLUME;
    case FS_NOTFOUND:  			// FILE NOT FOUND
        return ERR_FILENOTFOUND;
    case FS_CANTWRITE: 			// WRITE FAILED
        return ERR_CANTWRITE;
    case FS_NOCARD:    			// NO CARD INSERTED
        return ERR_NOCARD;
    case FS_CHANGED:    		// CARD HAS CHANGED
        return ERR_CARDCHANGED;
    case FS_MAXFILES:  			// MAXIMUM NUMBER OF FILES OPEN WAS EXCEEDED
        return ERR_MAXFILES;
    case FS_USED:      			// FILE/DIRECTORY IS BEING USED
        return ERR_ALREADYOPEN;
    case FS_DISKFULL:  		// DISK IS FULL
        return ERR_DISKFULL;
    case FS_EXIST:     		// FILE ALREADY EXISTS
        return ERR_ALREADYEXISTS;
    case FS_INVHANDLE: 		// HANDLE IS NOT VALID
        return ERR_INVALIDHANDLE;
    default:
    case FS_ERROR:      		    // UNKNOWN ERROR (OR FUNCTION DOESN'T CARE)
        return ERR_UNKNOWNFSERROR;
    }
}

BINT rplPathFromList(BYTEPTR path,WORDPTR list)
{
    WORDPTR ptr=list+1;
    WORDPTR eol=rplSkipOb(list);
    int off=0;

    while((*ptr!=CMD_ENDLIST)&&(ptr<eol))
    {
        if(*ptr==CMD_HOME) {
            off=0;
            ++ptr;
        }
        if(ptr!=list+1) {
            path[off]='/';
            ++off;
        }
        BINT len;
        if(ISSTRING(*ptr)) len=rplStrSize(ptr);
         else {
            if(ISIDENT(*ptr)) len=rplGetIdentLength(ptr);
            else {
                // INVALID OBJECT
                path[0]=0;
                return 0;
            }
        }
        BYTEPTR strptr=(BYTEPTR) (ptr+1);
        memcpyb(path+off,strptr,len);
        off+=len;
        ptr=rplSkipOb(ptr);
    }

    path[off]=0;

    return off;
}

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case SDRESET:
    {
        // REINIT FILE SYSTEM
        int error=FSRestart();
        if(error!=FS_OK) {
            rplNewBINTPush((BINT64)FSystem.CurrentVolume,HEXBINT);
            rplError(rplFSError2Error(error));
        }
        return;
    }

    case SDSETPART:
    {
        // SET CURRENT SD CARD PARTITION
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        BINT64 partnum=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;

        BINT ismounted=FSVolumeInserted(partnum);

        if(ismounted==FS_OK) {
            FSSetCurrentVolume(partnum);
       }
        else  {
         rplError(rplFSError2Error(ismounted));
        }

        rplDropData(1);

        return;

    }

    case SDSTO:
    {
        // STORE AN OBJECT DIRECTLY INTO A FILE
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(path,rplPeekData(1))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }
            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // TRY TO OPEN THE FILE

        FS_FILE *objfile;
        const char const *fileprolog="NRPL";
        int err;
        err=FSOpen((char *)(RReg[0].data),FSMODE_WRITE,&objfile);
        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
            }
        BINT objlen=rplObjSize(rplPeekData(2));
        if(FSWrite((unsigned char *)fileprolog,4,objfile)!=4) {
            FSClose(objfile);
            rplError(ERR_CANTWRITE);
            return;
        }
        err=FSWrite((unsigned char *)rplPeekData(2),objlen*sizeof(WORD),objfile);
        if(err!=objlen*(BINT)sizeof(WORD)) {
            FSClose(objfile);
            rplError(ERR_CANTWRITE);
            return;
        }
        FSClose(objfile);
        rplDropData(2);

        return;

    }

    case SDRCL:
    {
        // RCL AN OBJECT DIRECTLY FROM A FILE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(path,rplPeekData(1))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }

            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // TRY TO OPEN THE FILE

        FS_FILE *objfile;
        const char const *fileprolog="NRPL";
        int err;
        err=FSOpen((char *)(RReg[0].data),FSMODE_READ,&objfile);
        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
            }
        BINT objlen=FSFileLength(objfile);
        if(FSRead((unsigned char *)(RReg[0].data),4,objfile)!=4) {
            FSClose(objfile);
            rplError(ERR_NOTANRPLFILE);
            return;
        }
        if((WORD)RReg[0].data[0]!=*((WORD *)fileprolog)) {
            FSClose(objfile);
            rplError(ERR_NOTANRPLFILE);
            return;
        }

        // DROP THE NAME FROM THE STACK
        rplDropData(1);

        objlen-=4;
        while(objlen>=4) {
            if(FSRead((unsigned char *)(RReg[0].data),4,objfile)!=4) {
                FSClose(objfile);
                rplError(ERR_NOTANRPLFILE);
                return;
            }
            BINT objsize=rplObjSize((WORDPTR)RReg[0].data);
            if(objsize*(BINT)sizeof(WORD)<objlen) {
                FSClose(objfile);
                rplError(ERR_NOTANRPLFILE);
                return;
            }
            WORDPTR newobj=rplAllocTempOb(objsize-1);
            if(!newobj) {
                FSClose(objfile);
                return;
            }
            newobj[0]=(WORD)RReg[0].data[0];
            if(FSRead((unsigned char *)(newobj+1),(objsize-1)*sizeof(WORD),objfile)!=(objsize-1)*(BINT)sizeof(WORD)) {
                FSClose(objfile);
                rplError(ERR_NOTANRPLFILE);
                return;
            }

            // OBJECT WAS READ SUCCESSFULLY
            // TODO: ASK THE LIBRARY TO VERIFY IF THE OBJECT IS VALID

            rplPushData(newobj);


            objlen-=objsize*sizeof(WORD);
        }

        // DONE READING ALL OBJECTS FROM THE FILE
        FSClose(objfile);

        // IF THERE ARE MULTIPLE OBJECTS, SHOULDN'T WE RETURN THE NUMBER?

        return;

    }

    case SDCHDIR:
    {
        // CHANGE CURRENT DIRECTORY
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(path,rplPeekData(1))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // TRY TO CHANGE CURRENT DIR

        BINT err=FSChdir((char *)path);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
        }
        else rplDropData(1);

        return;

    }

    case SDUPDIR:
    {
        // TRY TO CHANGE CURRENT DIR

        BINT err=FSChdir("..");

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
        }

        return;

    }

    case SDCRDIR:
    {
        // CREATE A NEW DIRECTORY
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(path,rplPeekData(1))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // TRY TO CHANGE CURRENT DIR

        BINT err=FSMkdir((char *)path);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
        }
        else rplDropData(1);

        return;

    }

    case SDPGDIR:
    {
        // DELETE A DIRECTORY
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(path,rplPeekData(1))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // TRY TO DELETE CURRENT DIR

        BINT err=FSRmdir((char *)path);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
        }
        else rplDropData(1);

        return;

    }

    case SDPURGE:
    {
        // DELETE A FILE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(path,rplPeekData(1))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // TRY TO DELETE CURRENT DIR

        BINT err=FSDelete((char *)path);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
        }
        else rplDropData(1);

        return;

    }

    case SDOPENRD:
    {
        // OPEN A FILE FOR READ
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(path,rplPeekData(1))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // OPEN THE FILE
        FS_FILE *handle;
        BINT err=FSOpen((char *)path,FSMODE_READ,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

       rplNewBINTPush(FSGetHandle(handle),HEXBINT);

       return;



    }

    case SDOPENWR:
    {
        // OPEN A FILE FOR WRITING, IF IT EXISTS IT WILL TRUNCATE THE FILE TO 0 BYTES
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(path,rplPeekData(1))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // OPEN THE FILE
        FS_FILE *handle;
        BINT err=FSOpen((char *)path,FSMODE_WRITE,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

       rplNewBINTPush(FSGetHandle(handle),HEXBINT);

       return;



    }

    case SDOPENAPP:
    {
        // OPEN A FILE FOR APPEND
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(path,rplPeekData(1))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // OPEN THE FILE
        FS_FILE *handle;
        BINT err=FSOpen((char *)path,FSMODE_WRITE|FSMODE_APPEND,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

       rplNewBINTPush(FSGetHandle(handle),HEXBINT);

       return;



    }

    case SDOPENMOD:
    {
        // OPEN A FILE FOR MODIFY, IF IT EXISTS IT DOESN'T TRUNCATE THE FILE, USE LSEEK TO POSITION AND WRITE SPECIFIC RECORDS
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(path,rplPeekData(1))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // OPEN THE FILE
        FS_FILE *handle;
        BINT err=FSOpen((char *)path,FSMODE_WRITE|FSMODE_MODIFY,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

       rplNewBINTPush(FSGetHandle(handle),HEXBINT);

       return;



    }





    case SDCLOSE:
    {
        // CLOSE A HANDLE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        BINT64 num=rplReadBINT(rplPeekData(1));
        FS_FILE *handle;
        BINT err=FSGetFileFromHandle(num,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        err=FSClose(handle);
        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

       return;



    }

    case SDREADTEXT:
    {
        // READ THE GIVEN NUMBER OF UNICODE CHARACTERS FROM A FILE, RETURN THEM AS STRING OBJECT
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        if(!ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        BINT64 num=rplReadBINT(rplPeekData(1));
        BINT64 nchars=rplReadNumberAsBINT(rplPeekData(2));
        BINT bufsize=REAL_REGISTER_STORAGE*4;
        BYTEPTR tmpbuf=(BYTEPTR)RReg[0].data;
        BINT64 currentpos;
        if(Exceptions) return;

        FS_FILE *handle;
        BINT err=FSGetFileFromHandle(num,&handle);
        BINT readblock,charcount,bytecount,bufused;

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        currentpos=FSTell(handle);
        charcount=bytecount=0;

        bufused=0;

        do {

        readblock=(4*nchars>(bufsize-bufused))? (bufsize-bufused):4*nchars; // TRY TO READ IT ALL IN ONE BLOCK

        err=FSRead((tmpbuf+bufused),readblock,handle);
        if(err==0) {
            rplError(ERR_ENDOFFILE);
            return;
        }

        if(err!=readblock) readblock=err;

        readblock+=bufused;

        BYTEPTR lastgood,ptr;
        lastgood=(BYTEPTR)utf8rskipst((char *)tmpbuf+readblock,(char *)tmpbuf);  // FIND THE LAST GOOD UNICODE CODE POINT STARTER, MIGHT BE INCOMPLETE!

        ptr=tmpbuf;
        while((charcount<nchars)&&(ptr<lastgood)) {
            ptr=(BYTEPTR)utf8skipst((char *)ptr,(char *)lastgood);   // SKIP AND COUNT ONE CHARACTER
            ++charcount;
        }

        bytecount+=ptr-tmpbuf;

        if((charcount==nchars)||(FSEof(handle))) {

            if(charcount<nchars) {
                // THE LAST GOOD CHARACTER IS COMPLETE BECAUSE END OF FILE WAS REACHED AND NO COMBINERS WILL FOLLOW
                bytecount+=(tmpbuf+readblock)-lastgood;
                lastgood=tmpbuf+readblock;
                ++charcount;
            }

            // WE HAVE THEM ALL!
            WORDPTR newstring=rplAllocTempOb((bytecount+3)>>2);
            if(!newstring) {
                return;
            }

            // NOW GO BACK AND READ THE WHOLE STRING DIRECTLY INTO THE OBJECT
            err=FSSeek(handle,currentpos,FSSEEK_SET);
            if(err!=FS_OK) {
                rplError(rplFSError2Error(err));
                return;
            }
            err=FSRead((unsigned char *)(newstring+1),bytecount,handle);
            if(err!=bytecount)
            {
                rplError(ERR_ENDOFFILE);
                return;
            }

            // EVERYTHING WAS READ CORRECTLY
            *newstring=MAKESTRING(bytecount);

            rplDropData(1);
            rplOverwriteData(1,newstring);


            return;


        }

        // HERE WE NEED MORE CHARACTERS AND THE FILE HAS MORE DATA

        // MOVE THE SPARE BYTES TO THE START OF THE BUFFER
        bufused=readblock-(lastgood-tmpbuf);
        memmoveb(tmpbuf,lastgood,bufused);

        } while(!FSEof(handle));

        // THIS CAN'T EVER HAPPEN
        rplError(ERR_ENDOFFILE);
       return;



    }

    case SDWRITETEXT:
    {
        // WRITE A STRING TO A FILE
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        if(!ISSTRING(*rplPeekData(2))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        BINT64 num=rplReadBINT(rplPeekData(1));
        BYTEPTR stringstart=(BYTEPTR)(rplPeekData(2)+1);
        BINT nbytes=rplStrSize(rplPeekData(2));

        FS_FILE *handle;
        BINT err=FSGetFileFromHandle(num,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        if(nbytes>0) {
         err=FSWrite(stringstart,nbytes,handle);

         if(err<0) {
          rplError(rplFSError2Error(err));
          return;
         }
         if(err!=nbytes) {
           rplError(ERR_CANTWRITE);
           return;
         }

         }

         //  DONE WRITING
        rplDropData(2);


            return;

    }


    case SDREADLINE:
    {
        // READ ONE LINE OF TEXT FROM A FILE, RETURN THEM AS STRING OBJECT
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        BINT64 num=rplReadBINT(rplPeekData(1));

        BINT bufsize=REAL_REGISTER_STORAGE*4,readblock;
        BYTEPTR tmpbuf=(BYTEPTR)RReg[0].data,ptr;
        BINT64 currentpos,bytecount;
        if(Exceptions) return;

        FS_FILE *handle;
        BINT err=FSGetFileFromHandle(num,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        currentpos=FSTell(handle);
        bytecount=0;

        do {


        err=FSRead(tmpbuf,bufsize,handle);
        if(err==0) {
            rplError(ERR_ENDOFFILE);
            return;
        }

        readblock=err;

        ptr=tmpbuf;
        while( (*ptr!='\n')&& readblock) { ++ptr; --readblock; }

        if(readblock) {
            // FOUND END OF LINE!
            bytecount+=ptr+1-tmpbuf; // INCLUDE THE NEWLINE IN THE RETURNED STRING

            break;

        }

        bytecount+=err;
        // HERE WE NEED MORE CHARACTERS AND THE FILE HAS MORE DATA

        } while(!FSEof(handle));


        WORDPTR newstring=rplAllocTempOb((bytecount+3)>>2);
        if(!newstring) {
            return;
        }

        // NOW GO BACK AND READ THE WHOLE STRING DIRECTLY INTO THE OBJECT
        err=FSSeek(handle,currentpos,FSSEEK_SET);
        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }
        err=FSRead((unsigned char *)(newstring+1),bytecount,handle);
        if(err!=bytecount)
        {
            rplError(ERR_ENDOFFILE);
            return;
        }

        // EVERYTHING WAS READ CORRECTLY
        *newstring=MAKESTRING(bytecount);

        rplOverwriteData(1,newstring);

       return;

    }


    case SDSEEKCUR:
    case SDSEEKEND:
    case SDSEEKSTA:
    {
        // MOVE THE FILE POINTER TO THE GIVEN OFFSET.
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        if(!ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }
        BINT seek_from=FSSEEK_CUR;
        if(OPCODE(CurOpcode)==SDSEEKSTA) seek_from=FSSEEK_SET;
        if(OPCODE(CurOpcode)==SDSEEKEND) seek_from=FSSEEK_END;

        BINT64 num=rplReadBINT(rplPeekData(1));
        BINT64 offset=rplReadNumberAsBINT(rplPeekData(2));
        if(Exceptions) return;

        FS_FILE *handle;
        BINT err=FSGetFileFromHandle(num,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        FSSeek(handle,offset,seek_from);

        rplDropData(2);

        return;
    }

    case SDTELL:
    {
        // RETURN THE CURRENT OFFSET WITHIN THE FILE IN BYTES
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        BINT64 num=rplReadBINT(rplPeekData(1));

        FS_FILE *handle;
        BINT err=FSGetFileFromHandle(num,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        BINT64 pos=FSTell(handle);

        rplDropData(1);
        rplNewBINTPush(pos,DECBINT);

        return;
    }

    case SDFILESIZE:
    {
        // RETURN THE SIZE OF THE FILE IN BYTES
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        BINT64 num=rplReadBINT(rplPeekData(1));

        FS_FILE *handle;
        BINT err=FSGetFileFromHandle(num,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        BINT64 len=FSFileLength(handle);

        rplDropData(1);
        rplNewBINTPush(len,DECBINT);

        return;
    }


    case SDEOF:

    {
        // RETURN THE SIZE OF THE FILE IN BYTES
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        BINT64 num=rplReadBINT(rplPeekData(1));

        FS_FILE *handle;
        BINT err=FSGetFileFromHandle(num,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);
        if(FSEof(handle)) rplPushTrue();
        else rplPushFalse();

        return;
    }


    case SDOPENDIR:
    {
        // OPEN A DIRECTORY FOR ENTRY SCANNING
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR path=(BYTEPTR)RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(path,rplPeekData(1)+1,pathlen);
            path[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(path,rplPeekData(1))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(path,rplPeekData(1)+1,pathlen);
                path[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        // OPEN THE FILE
        FS_FILE *handle;
        BINT err=FSOpenDir((char *)path,&handle);

        if( (err!=FS_OK)&&(err!=FS_OPENDIR)) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

       rplNewBINTPush(FSGetHandle(handle),HEXBINT);

       return;



    }


    case SDNEXTFILE:
    {
        // GET NEXT FILE IN A DIRECTORY LISTING
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        BINT64 num=rplReadBINT(rplPeekData(1));

        FS_FILE *handle;
        BINT err=FSGetFileFromHandle(num,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

        FS_FILE entry;

        do {
        err=FSGetNextEntry(&entry,handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        // DON'T ACCEPT THE . AND .. ENTRIES
        if(entry.Name[0]=='.') {
            if(entry.Name[1]==0) { FSReleaseEntry(&entry); continue; }
            if((entry.Name[1]=='.') && entry.Name[2]==0) { FSReleaseEntry(&entry); continue; }
        }
        // DON'T ACCEPT ANY DIRECTORIES OR SPECIAL FILES
        if(entry.Attr&(FSATTR_HIDDEN|FSATTR_DIR|FSATTR_SYSTEM|FSATTR_VOLUME)) { FSReleaseEntry(&entry); continue; }

        break;

        } while(1);

        // PUT THE DATA ON A LIST

        WORDPTR *dsave=DSTop;
        WORDPTR newobj=rplCreateString((BYTEPTR)entry.Name,(BYTEPTR)entry.Name+stringlen(entry.Name));
        if(!newobj) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }
        rplPushData(newobj);

        // PUT THE FILE ATTRIBUTES NEXT
        BYTE attr_string[6];
        attr_string[0]=(entry.Attr&FSATTR_RDONLY)? 'R':'_';
        attr_string[1]=(entry.Attr&FSATTR_HIDDEN)? 'H':'_';
        attr_string[2]=(entry.Attr&FSATTR_SYSTEM)? 'S':'_';
        attr_string[3]=(entry.Attr&FSATTR_DIR)? 'D':'_';
        attr_string[4]=(entry.Attr&FSATTR_VOLUME)? 'V':'_';
        attr_string[5]=(entry.Attr&FSATTR_ARCHIVE)? 'A':'_';

        newobj=rplCreateString(attr_string,attr_string+6);
        if(!newobj) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }
        rplPushData(newobj);


        // FILE SIZE
        rplNewBINTPush(entry.FileSize,DECBINT);
        if(Exceptions) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // LAST MODIFIED DATE
        struct compact_tm date;
        FSGetWriteTime(&entry,&date);
        rplBINTToRReg(0,(BINT64)date.tm_year+1900+(BINT64)(date.tm_mon+1)*10000+(BINT64)date.tm_mday*1000000);
        RReg[0].exp-=6;
        rplNewRealFromRRegPush(0);
        if(Exceptions) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // LAST MODIFIED TIME

        rplBINTToRReg(0,(BINT64)date.tm_sec+(BINT64)date.tm_min*100+(BINT64)date.tm_hour*10000);
        RReg[0].exp-=4;
        rplNewRealFromRRegPush(0);
        if(Exceptions) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // THAT'S ENOUGH DATA
        FSReleaseEntry(&entry);


        return;



    }

    case SDNEXTENTRY:
    {
        // GET NEXT ENTRY IN A DIRECTORY LISTING
        // INCLUDE ALL SPECIAL AND HIDDEN FILES
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        BINT64 num=rplReadBINT(rplPeekData(1));

        FS_FILE *handle;
        BINT err=FSGetFileFromHandle(num,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

        FS_FILE entry;

        err=FSGetNextEntry(&entry,handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        // PUT THE DATA ON A LIST

        WORDPTR *dsave=DSTop;
        WORDPTR newobj=rplCreateString((BYTEPTR)entry.Name,(BYTEPTR)entry.Name+stringlen(entry.Name));
        if(!newobj) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }
        rplPushData(newobj);

        // PUT THE FILE ATTRIBUTES NEXT
        BYTE attr_string[6];
        attr_string[0]=(entry.Attr&FSATTR_RDONLY)? 'R':'_';
        attr_string[1]=(entry.Attr&FSATTR_HIDDEN)? 'H':'_';
        attr_string[2]=(entry.Attr&FSATTR_SYSTEM)? 'S':'_';
        attr_string[3]=(entry.Attr&FSATTR_DIR)? 'D':'_';
        attr_string[4]=(entry.Attr&FSATTR_VOLUME)? 'V':'_';
        attr_string[5]=(entry.Attr&FSATTR_ARCHIVE)? 'A':'_';

        newobj=rplCreateString(attr_string,attr_string+6);
        if(!newobj) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }
        rplPushData(newobj);


        // FILE SIZE
        rplNewBINTPush(entry.FileSize,DECBINT);
        if(Exceptions) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // LAST MODIFIED DATE
        struct compact_tm date;
        FSGetWriteTime(&entry,&date);
        rplBINTToRReg(0,(BINT64)date.tm_year+1900+(BINT64)(date.tm_mon+1)*10000+(BINT64)date.tm_mday*1000000);
        RReg[0].exp-=6;
        rplNewRealFromRRegPush(0);
        if(Exceptions) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // LAST MODIFIED TIME

        rplBINTToRReg(0,(BINT64)date.tm_sec+(BINT64)date.tm_min*100+(BINT64)date.tm_hour*10000);
        RReg[0].exp-=4;
        rplNewRealFromRRegPush(0);
        if(Exceptions) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // THAT'S ENOUGH DATA
        FSReleaseEntry(&entry);


        return;



    }
    case SDNEXTDIR:
    {
        // GET NEXT DIRECTORY IN A DIRECTORY LISTING
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINT(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        BINT64 num=rplReadBINT(rplPeekData(1));

        FS_FILE *handle;
        BINT err=FSGetFileFromHandle(num,&handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

        FS_FILE entry;



        do {
        err=FSGetNextEntry(&entry,handle);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        // DON'T ACCEPT THE . AND .. ENTRIES
        if(entry.Name[0]=='.') {
            if(entry.Name[1]==0) { FSReleaseEntry(&entry); continue; }
            if((entry.Name[1]=='.') && entry.Name[2]==0) { FSReleaseEntry(&entry); continue; }
        }
        // DON'T ACCEPT ANY SPECIAL FILES OR SPECIAL DIRECTORIES
        if(entry.Attr&(FSATTR_HIDDEN|FSATTR_SYSTEM|FSATTR_VOLUME)) { FSReleaseEntry(&entry); continue; }

        // DON'T ACCEPT FILES THAT ARE NOT DIRECTORIES
        if(!(entry.Attr&FSATTR_DIR)) { FSReleaseEntry(&entry); continue; }

        break;

        } while(1);

        // PUT THE DATA ON A LIST

        WORDPTR *dsave=DSTop;
        WORDPTR newobj=rplCreateString((BYTEPTR)entry.Name,(BYTEPTR)entry.Name+stringlen(entry.Name));
        if(!newobj) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }
        rplPushData(newobj);

        // PUT THE FILE ATTRIBUTES NEXT
        BYTE attr_string[6];
        attr_string[0]=(entry.Attr&FSATTR_RDONLY)? 'R':'_';
        attr_string[1]=(entry.Attr&FSATTR_HIDDEN)? 'H':'_';
        attr_string[2]=(entry.Attr&FSATTR_SYSTEM)? 'S':'_';
        attr_string[3]=(entry.Attr&FSATTR_DIR)? 'D':'_';
        attr_string[4]=(entry.Attr&FSATTR_VOLUME)? 'V':'_';
        attr_string[5]=(entry.Attr&FSATTR_ARCHIVE)? 'A':'_';

        newobj=rplCreateString(attr_string,attr_string+6);
        if(!newobj) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }
        rplPushData(newobj);


        // FILE SIZE
        rplNewBINTPush(entry.FileSize,DECBINT);
        if(Exceptions) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // LAST MODIFIED DATE
        struct compact_tm date;
        FSGetWriteTime(&entry,&date);
        rplBINTToRReg(0,(BINT64)date.tm_year+1900+(BINT64)(date.tm_mon+1)*10000+(BINT64)date.tm_mday*1000000);
        RReg[0].exp-=6;
        rplNewRealFromRRegPush(0);
        if(Exceptions) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // LAST MODIFIED TIME

        rplBINTToRReg(0,(BINT64)date.tm_sec+(BINT64)date.tm_min*100+(BINT64)date.tm_hour*10000);
        RReg[0].exp-=4;
        rplNewRealFromRRegPush(0);
        if(Exceptions) {
            DSTop=dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // THAT'S ENOUGH DATA
        FSReleaseEntry(&entry);


        return;



    }

    case SDCOPY:
    {
        // COPY A FILE
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        if(!ISIDENT(*rplPeekData(2)) && !ISSTRING(*rplPeekData(2)) && !ISLIST(*rplPeekData(2))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR pathfrom=(BYTEPTR)RReg[0].data;
        BYTEPTR pathto=(BYTEPTR)RReg[1].data;

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(pathto,rplPeekData(1)+1,pathlen);
            pathto[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(pathto,rplPeekData(1))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(pathto,rplPeekData(1)+1,pathlen);
                pathto[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        if(ISIDENT(*rplPeekData(2))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(2));
            memmoveb(pathfrom,rplPeekData(2)+1,pathlen);
            pathfrom[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(2))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(pathfrom,rplPeekData(2))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }



            }
            else if(ISSTRING(*rplPeekData(2))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(2));
                memmoveb(pathfrom,rplPeekData(2)+1,pathlen);
                pathfrom[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }


        // FINALLY, COPY THE FILE
        FS_FILE *handleto,*handlefrom;
        BINT err=FSOpen((char *)pathto,FSMODE_WRITE,&handleto);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        err=FSOpen((char *)pathfrom,FSMODE_READ,&handlefrom);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        // JUST READ AND WRITE, USE REAL NUMBERS AS STORAGE

        BINT nbytes=FSFileLength(handlefrom);

        // WARNING: THIS ASSUMES REAL_REGISTER_STORAGE > 1024 BYTES
        // MAKE SURE THIS IS TRUE!

#if (REAL_REGISTER_STORAGE < 256)
#error This part of the code relies on at least 1024 bytes of storage per register.
#endif
        // IF WE HAD A LARGER BUFFER THIS COULD BE FASTER
        while(nbytes>1024) {
            FSRead(pathto,1024,handlefrom);
            err=FSWrite(pathto,1024,handleto);
            if(err!=1024) {
            if(err<=0) rplError(rplFSError2Error(err));
            else rplError(ERR_CANTWRITE);
            FSClose(handleto);
            FSClose(handlefrom);
            return;
            }
            nbytes-=1024;
        }

        if(nbytes) {
            FSRead(pathto,nbytes,handlefrom);
            err=FSWrite(pathto,nbytes,handleto);
            if(err!=nbytes) {
            if(err<=0) rplError(rplFSError2Error(err));
            else rplError(ERR_CANTWRITE);
            FSClose(handleto);
            FSClose(handlefrom);
            return;
            }
        }

        FSClose(handleto);
        FSClose(handlefrom);

        rplDropData(2);

        return;
    }

    case SDMOVE:
    {
        // MOVE/RENAME A FILE
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1)) && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        if(!ISIDENT(*rplPeekData(2)) && !ISSTRING(*rplPeekData(2)) && !ISLIST(*rplPeekData(2))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        BYTEPTR pathfrom=(BYTEPTR)RReg[0].data;
        BYTEPTR pathto=(BYTEPTR)RReg[1].data;

        if(ISIDENT(*rplPeekData(1))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(1));
            memmoveb(pathto,rplPeekData(1)+1,pathlen);
            pathto[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(1))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(pathto,rplPeekData(1))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }



            }
            else if(ISSTRING(*rplPeekData(1))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(1));
                memmoveb(pathto,rplPeekData(1)+1,pathlen);
                pathto[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }

        if(ISIDENT(*rplPeekData(2))) {
            BINT pathlen=rplGetIdentLength(rplPeekData(2));
            memmoveb(pathfrom,rplPeekData(2)+1,pathlen);
            pathfrom[pathlen]=0;    // NULL TERMINATED STRING
        } else
            if(ISLIST(*rplPeekData(2))) {
                // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
                if(!rplPathFromList(pathfrom,rplPeekData(2))) {
                    rplError(ERR_BADFILENAME);
                    return;
                }



            }
            else if(ISSTRING(*rplPeekData(2))) {
                // FULL PATH GIVEN
                BINT pathlen=rplStrSize(rplPeekData(2));
                memmoveb(pathfrom,rplPeekData(2)+1,pathlen);
                pathfrom[pathlen]=0;    // NULL TERMINATED STRING

            }
            else {
                // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
                rplError(ERR_IDENTORPATHEXPECTED);
                return;
            }


        // FINALLY, MOVE THE FILE
        BINT err=FSRename((char *)pathfrom,(char *)pathto);

        if(err!=FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(2);

        return;
    }


case SDPATH:
    {
        // RETURN THE CURRENT WORK DIRECTORY
        BINT cvol=FSGetCurrentVolume();

        if(cvol<0) {
            rplError(rplFSError2Error(cvol));
            return;
        }

        BYTEPTR path=(BYTEPTR)FSGetcwd(cvol),endpath;
        if(!path) {
            rplError(ERR_UNKNOWNFSERROR);
            return;
        }
        endpath=path;
        while(*endpath) ++endpath;
        WORDPTR newstring=rplCreateString(path,endpath);

        if(!newstring) return;

        rplPushData(newstring);

        // VERY IMPORTANT TO RELEASE THE MEMORY
        // ALLOCATED BY THE FILE SYSTEM FOR THE PATH!
        simpfree(path);

        return;
    }

    case SDFREE:
        {
            // RETURN THE FREE SPACE IN BYTES IN THE CURRENT PARTITION
            BINT cvol=FSGetCurrentVolume();

            if(cvol<0) {
                rplError(rplFSError2Error(cvol));
                return;
            }

            BINT64 size=FSGetVolumeFree(cvol);
            if(size<0) {
                rplError(rplFSError2Error(size));
                return;
            }

            rplNewBINTPush(size*512,DECBINT);

            return;
        }

        // STANDARIZED OPCODES:
        // --------------------
        // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES

    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors


            // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
            // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds((char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_VALIDATE:
        // VALIDATE RECEIVES OPCODES COMPILED BY OTHER LIBRARIES, TO BE INCLUDED WITHIN A COMPOSITE OWNED BY
        // THIS LIBRARY. EVERY COMPOSITE HAS TO EVALUATE IF THE OBJECT BEING COMPILED IS ALLOWED INSIDE THIS
        // COMPOSITE OR NOT. FOR EXAMPLE, A REAL MATRIX SHOULD ONLY ALLOW REAL NUMBERS INSIDE, ANY OTHER
        // OPCODES SHOULD BE REJECTED AND AN ERROR THROWN.
        // Library receives:
        // CurrentConstruct = SET TO THE CURRENT ACTIVE CONSTRUCT TYPE
        // LastCompiledObject = POINTER TO THE LAST OBJECT THAT WAS COMPILED, THAT NEEDS TO BE VERIFIED

        // VALIDATE RETURNS:
        // RetNum =  OK_CONTINUE IF THE OBJECT IS ACCEPTED, ERR_INVALID IF NOT.


        RetNum=OK_CONTINUE;
        return;

    case OPCODE_PROBETOKEN:
        // PROBETOKEN FINDS A VALID WORD AT THE BEGINNING OF THE GIVEN TOKEN AND RETURNS
        // INFORMATION ABOUT IT. THIS OPCODE IS MANDATORY

        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND
        {
        libProbeCmds((char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);

        return;
        }


    case OPCODE_GETINFO:
        libGetInfo2(*DecompileObject,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER,(WORDPTR *)ROMPTR_TABLE,ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID);
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) { RetNum=ERR_INVALID; return; }

        RetNum=OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {\
        if(MENUNUMBER(MenuCodeArg)>0) { RetNum=ERR_NOTMINE; return; }
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR=ROMPTR_TABLE[MENUNUMBER(MenuCodeArg)+2];
        RetNum=OK_CONTINUE;
       return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp,(WORDPTR)LIB_HELPTABLE);
       return;
    }
    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {

        libFindMsg(LibError,(WORDPTR)LIB_MSGTABLE);
       return;
    }


    case OPCODE_LIBINSTALL:
        LibraryList=(WORDPTR)libnumberlist;
        RetNum=OK_CONTINUE;
        return;
    case OPCODE_LIBREMOVE:
        return;

    }
    // UNHANDLED OPCODE...

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode)>=MIN_RESERVED_OPCODE) {
        RetNum=ERR_NOTMINE;
        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    rplError(ERR_INVALIDOPCODE);

    return;


}


#endif
