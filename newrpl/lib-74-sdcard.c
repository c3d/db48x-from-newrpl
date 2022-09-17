/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "libraries.h"

#ifndef COMMANDS_ONLY_PASS
#include "cmdcodes.h"
#include "hal_api.h"
#include "sysvars.h"
#endif

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************

// REPLACE THE NUMBER
#define LIBRARY_NUMBER  74

//@TITLE=SD Card

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
    CMD(SDFREE,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDARCHIVE,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDRESTORE,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SDGETPART,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2))

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
    ERR(NOTANRPLFILE,14), \
    ERR(INVALIDDATA,15)

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
const word_p const ROMPTR_TABLE[] = {
    (word_p) LIB_MSGTABLE,
    (word_p) LIB_HELPTABLE,
    (word_p) lib74_menu,
    0
};

#ifndef CONFIG_NO_FSYSTEM
// CONVERT FILE SYSTEM ERROR MESSAGE INTO THIS LIBRARY ERRORS
int32_t rplFSError2Error(int32_t err)
{
    switch (err) {
    case FS_EOF:       // END OF FILE
        return ERR_ENDOFFILE;
    case FS_BADNAME:   // INVALID FILE NAME
        return ERR_BADFILENAME;
    case FS_BADVOLUME: // INVALID DRIVE
        return ERR_BADVOLUME;
    case FS_NOTFOUND:  // FILE NOT FOUND
        return ERR_FILENOTFOUND;
    case FS_CANTWRITE: // WRITE FAILED
        return ERR_CANTWRITE;
    case FS_NOCARD:    // NO CARD INSERTED
        return ERR_NOCARD;
    case FS_CHANGED:   // CARD HAS CHANGED
        return ERR_CARDCHANGED;
    case FS_MAXFILES:  // MAXIMUM NUMBER OF FILES OPEN WAS EXCEEDED
        return ERR_MAXFILES;
    case FS_USED:      // FILE/DIRECTORY IS BEING USED
        return ERR_ALREADYOPEN;
    case FS_DISKFULL:  // DISK IS FULL
        return ERR_DISKFULL;
    case FS_EXIST:     // FILE ALREADY EXISTS
        return ERR_ALREADYEXISTS;
    case FS_INVHANDLE: // HANDLE IS NOT VALID
        return ERR_INVALIDHANDLE;
    default:
    case FS_ERROR:     // UNKNOWN ERROR (OR FUNCTION DOESN'T CARE)
        return ERR_UNKNOWNFSERROR;
    }
}

int32_t rplPathFromList(byte_p path, word_p list)
{
    word_p ptr = list + 1;
    word_p eol = rplSkipOb(list);
    int off = 0;

    while((*ptr != CMD_ENDLIST) && (ptr < eol)) {
        if(*ptr == CMD_HOME) {
            off = 0;
            ++ptr;
        }
        if(ptr != list + 1) {
            path[off] = '/';
            ++off;
        }
        int32_t len;
        if(ISSTRING(*ptr))
            len = rplStrSize(ptr);
        else {
            if(ISIDENT(*ptr))
                len = rplGetIdentLength(ptr);
            else {
                // INVALID OBJECT
                path[0] = 0;
                return 0;
            }
        }
        byte_p strptr = (byte_p) (ptr + 1);
        memcpyb(path + off, strptr, len);
        off += len;
        ptr = rplSkipOb(ptr);
    }

    path[off] = 0;

    return off;
}

int rplSDArchiveWriteWord(unsigned int data, void *opaque)
{

    FS_FILE *file = (FS_FILE *) opaque;
    if(FSWrite((byte_p) & data, 4, file) != 4)
        return 0;
    return 1;
}

WORD rplSDArchiveReadWord(void *opaque)
{
    WORD data;
    FS_FILE *file = (FS_FILE *) opaque;

    if(FSRead((byte_p) & data, 4, file) != 4)
        return 0;
    return data;
}

#endif

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }
    // LIBRARIES THAT DEFINE ONLY COMMANDS STILL HAVE TO RESPOND TO A FEW OVERLOADABLE OPERATORS
    if(LIBNUM(CurOpcode) == LIB_OVERLOADABLE) {
        // ONLY RESPOND TO EVAL, EVAL1 AND XEQ FOR THE COMMANDS DEFINED HERE
        // IN CASE OF COMMANDS TREATED AS OBJECTS (WHEN EMBEDDED IN LISTS)
        if((OPCODE(CurOpcode) == OVR_EVAL) ||
                (OPCODE(CurOpcode) == OVR_EVAL1) ||
                (OPCODE(CurOpcode) == OVR_XEQ)) {
            // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
            if(rplDepthData() < 1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORD saveOpcode = CurOpcode;
            CurOpcode = *rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode = saveOpcode;
            return;
        }
        // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
        if(OPCODE(CurOpcode) == OVR_SAME) {
            if(*rplPeekData(2) == *rplPeekData(1)) {
                rplDropData(2);
                rplPushTrue();
            }
            else {
                rplDropData(2);
                rplPushFalse();
            }
            return;
        }
        else {
            rplError(ERR_INVALIDOPCODE);
            return;
        }

    }

    switch (OPCODE(CurOpcode)) {

#ifndef CONFIG_NO_FSYSTEM

    case SDRESET:
    {
        //@SHORT_DESC=Reset the file system module
        //@NEW
        // REINIT FILE SYSTEM
        int error = FSRestart();
        if(error != FS_OK) {
            rplNewint32_tPush((int64_t) FSystem.CurrentVolume, HEXint32_t);
            rplError(rplFSError2Error(error));
        }
        return;
    }

    case SDSETPART:
    {
        //@SHORT_DESC=Set active partition
        //@NEW
        // SET CURRENT SD CARD PARTITION
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int64_t partnum = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;

        int32_t ismounted = FSVolumeInserted(partnum - 3);

        if(ismounted == FS_OK) {
            FSSetCurrentVolume(partnum - 3);
        }
        else {
            rplError(rplFSError2Error(ismounted));
        }

        rplDropData(1);

        return;

    }

    case SDSTO:
    {
        //@SHORT_DESC=Store a an object into a file
        //@NEW
        // STORE AN OBJECT DIRECTLY INTO A FILE
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p path = (byte_p) RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(path, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }
        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // TRY TO OPEN THE FILE

        FS_FILE *objfile;
        const char *const fileprolog = "NRPL";
        int err;
        err = FSOpen((char *)(RReg[0].data), FSMODE_WRITE, &objfile);
        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }
        int32_t objlen = rplObjSize(rplPeekData(2));
        if(FSWrite((unsigned char *)fileprolog, 4, objfile) != 4) {
            FSClose(objfile);
            rplError(ERR_CANTWRITE);
            return;
        }
        err = FSWrite((unsigned char *)rplPeekData(2), objlen * sizeof(WORD),
                objfile);
        if(err != objlen * (int32_t) sizeof(WORD)) {
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
        //@SHORT_DESC=Recall an object from a file
        //@NEW
        // RCL AN OBJECT DIRECTLY FROM A FILE
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p path = (byte_p) RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(path, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // TRY TO OPEN THE FILE

        FS_FILE *objfile;
        const char *const fileprolog = "NRPL";
        int err;
        err = FSOpen((char *)(RReg[0].data), FSMODE_READ, &objfile);
        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }
        int32_t objlen = FSFileLength(objfile);
        if(FSRead((unsigned char *)(RReg[0].data), 4, objfile) != 4) {
            FSClose(objfile);
            rplError(ERR_NOTANRPLFILE);
            return;
        }
        if((WORD) RReg[0].data[0] != *((WORD *) fileprolog)) {
            FSClose(objfile);
            rplError(ERR_NOTANRPLFILE);
            return;
        }

        // DROP THE NAME FROM THE STACK
        rplDropData(1);

        objlen -= 4;
        while(objlen >= 4) {
            if(FSRead((unsigned char *)(RReg[0].data), 4, objfile) != 4) {
                FSClose(objfile);
                rplError(ERR_NOTANRPLFILE);
                return;
            }
            int32_t objsize = rplObjSize((word_p) RReg[0].data);
            if(objsize * (int32_t) sizeof(WORD) < objlen) {
                FSClose(objfile);
                rplError(ERR_NOTANRPLFILE);
                return;
            }
            word_p newobj = rplAllocTempOb(objsize - 1);
            if(!newobj) {
                FSClose(objfile);
                return;
            }
            newobj[0] = (WORD) RReg[0].data[0];
            if(FSRead((unsigned char *)(newobj + 1),
                        (objsize - 1) * sizeof(WORD),
                        objfile) != (objsize - 1) * (int32_t) sizeof(WORD)) {
                FSClose(objfile);
                rplError(ERR_NOTANRPLFILE);
                return;
            }

            // OBJECT WAS READ SUCCESSFULLY
            // TODO: ASK THE LIBRARY TO VERIFY IF THE OBJECT IS VALID

            rplPushData(newobj);

            objlen -= objsize * sizeof(WORD);
        }

        // DONE READING ALL OBJECTS FROM THE FILE
        FSClose(objfile);

        // IF THERE ARE MULTIPLE OBJECTS, SHOULDN'T WE RETURN THE NUMBER?

        return;

    }

    case SDCHDIR:
    {
        //@SHORT_DESC=Change current directory
        //@NEW
        // CHANGE CURRENT DIRECTORY
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p path = (byte_p) RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(path, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // TRY TO CHANGE CURRENT DIR

        int32_t err = FSChdir((char *)path);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
        }
        else
            rplDropData(1);

        return;

    }

    case SDUPDIR:
    {
        //@SHORT_DESC=Change to parent directory
        //@NEW
        // TRY TO CHANGE CURRENT DIR

        int32_t err = FSChdir("..");

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
        }

        return;

    }

    case SDCRDIR:
    {
        //@SHORT_DESC=Create a new directory
        //@NEW
        // CREATE A NEW DIRECTORY
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p path = (byte_p) RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(path, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // TRY TO CHANGE CURRENT DIR

        int32_t err = FSMkdir((char *)path);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
        }
        else
            rplDropData(1);

        return;

    }

    case SDPGDIR:
    {
        //@SHORT_DESC=Delete an entire directory
        //@NEW
        // DELETE A DIRECTORY
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p path = (byte_p) RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(path, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // TRY TO DELETE CURRENT DIR

        int32_t err = FSRmdir((char *)path);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
        }
        else
            rplDropData(1);

        return;

    }

    case SDPURGE:
    {
        //@SHORT_DESC=Delete a file
        //@NEW
        // DELETE A FILE
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p path = (byte_p) RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(path, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // TRY TO DELETE CURRENT DIR

        int32_t err = FSDelete((char *)path);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
        }
        else
            rplDropData(1);

        return;

    }

    case SDOPENRD:
    {
        //@SHORT_DESC=Open a file for read-only operation
        //@NEW
        // OPEN A FILE FOR READ
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p path = (byte_p) RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(path, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // OPEN THE FILE
        FS_FILE *handle;
        int32_t err = FSOpen((char *)path, FSMODE_READ, &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

        rplNewint32_tPush(FSGetHandle(handle), HEXint32_t);

        return;

    }

    case SDOPENWR:
    {
        //@SHORT_DESC=Open a file for writing
        //@NEW
        // OPEN A FILE FOR WRITING, IF IT EXISTS IT WILL TRUNCATE THE FILE TO 0 BYTES
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p path = (byte_p) RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(path, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // OPEN THE FILE
        FS_FILE *handle;
        int32_t err =
                FSOpen((char *)path, FSMODE_WRITE | FSMODE_WRITEBUFFERS,
                &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

        rplNewint32_tPush(FSGetHandle(handle), HEXint32_t);

        return;

    }

    case SDOPENAPP:
    {
        //@SHORT_DESC=Open a file in append mode
        //@NEW
        // OPEN A FILE FOR APPEND
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p path = (byte_p) RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(path, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // OPEN THE FILE
        FS_FILE *handle;
        int32_t err = FSOpen((char *)path, FSMODE_WRITE | FSMODE_APPEND, &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

        rplNewint32_tPush(FSGetHandle(handle), HEXint32_t);

        return;

    }

    case SDOPENMOD:
    {
        //@SHORT_DESC=Open a file in modify mode
        //@NEW
        // OPEN A FILE FOR MODIFY, IF IT EXISTS IT DOESN'T TRUNCATE THE FILE, USE LSEEK TO POSITION AND WRITE SPECIFIC RECORDS
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p path = (byte_p) RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(path, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // OPEN THE FILE
        FS_FILE *handle;
        int32_t err = FSOpen((char *)path, FSMODE_WRITE | FSMODE_MODIFY, &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

        rplNewint32_tPush(FSGetHandle(handle), HEXint32_t);

        return;

    }

    case SDCLOSE:
    {
        //@SHORT_DESC=Close an open file
        //@NEW
        // CLOSE A HANDLE
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISint32_t(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        int64_t num = rplReadint32_t(rplPeekData(1));
        FS_FILE *handle;
        int32_t err = FSGetFileFromHandle(num, &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        err = FSClose(handle);
        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

        return;

    }

    case SDREADTEXT:
    {
        //@SHORT_DESC=Read text from an open file (UTF-8 encoding)
        //@NEW
        // READ THE GIVEN NUMBER OF UNICODE CHARACTERS FROM A FILE, RETURN THEM AS STRING OBJECT
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        if(!ISint32_t(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        if(!ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        int64_t num = rplReadint32_t(rplPeekData(1));
        int64_t nchars = rplReadNumberAsInt64(rplPeekData(2));
        int32_t bufsize = REAL_REGISTER_STORAGE * 4;
        byte_p tmpbuf = (byte_p) RReg[0].data;
        int64_t currentpos;
        if(Exceptions)
            return;

        FS_FILE *handle;
        int32_t err = FSGetFileFromHandle(num, &handle);
        int32_t readblock, charcount, bytecount, bufused;

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        currentpos = FSTell(handle);
        charcount = bytecount = 0;

        bufused = 0;

        do {

            readblock = (4 * nchars > (bufsize - bufused)) ? (bufsize - bufused) : 4 * nchars;  // TRY TO READ IT ALL IN ONE BLOCK

            err = FSRead((tmpbuf + bufused), readblock, handle);
            if(err == 0) {
                rplError(ERR_ENDOFFILE);
                return;
            }

            if(err != readblock)
                readblock = err;

            readblock += bufused;

            byte_p lastgood, ptr;
            lastgood = (byte_p) utf8rskipst((char *)tmpbuf + readblock, (char *)tmpbuf);       // FIND THE LAST GOOD UNICODE CODE POINT STARTER, MIGHT BE INCOMPLETE!

            ptr = tmpbuf;
            while((charcount < nchars) && (ptr < lastgood)) {
                ptr = (byte_p) utf8skipst((char *)ptr, (char *)lastgood);      // SKIP AND COUNT ONE CHARACTER
                ++charcount;
            }

            bytecount += ptr - tmpbuf;

            if((charcount == nchars) || (FSEof(handle))) {

                if(charcount < nchars) {
                    // THE LAST GOOD CHARACTER IS COMPLETE BECAUSE END OF FILE WAS REACHED AND NO COMBINERS WILL FOLLOW
                    bytecount += (tmpbuf + readblock) - lastgood;
                    lastgood = tmpbuf + readblock;
                    ++charcount;
                }

                // WE HAVE THEM ALL!
                word_p newstring = rplAllocTempOb((bytecount + 3) >> 2);
                if(!newstring) {
                    return;
                }

                // NOW GO BACK AND READ THE WHOLE STRING DIRECTLY INTO THE OBJECT
                err = FSSeek(handle, currentpos, FSSEEK_SET);
                if(err != FS_OK) {
                    rplError(rplFSError2Error(err));
                    return;
                }
                err = FSRead((unsigned char *)(newstring + 1), bytecount,
                        handle);
                if(err != bytecount) {
                    rplError(ERR_ENDOFFILE);
                    return;
                }

                // EVERYTHING WAS READ CORRECTLY
                *newstring = MAKESTRING(bytecount);

                rplDropData(1);
                rplOverwriteData(1, newstring);

                return;

            }

            // HERE WE NEED MORE CHARACTERS AND THE FILE HAS MORE DATA

            // MOVE THE SPARE BYTES TO THE START OF THE BUFFER
            bufused = readblock - (lastgood - tmpbuf);
            memmoveb(tmpbuf, lastgood, bufused);

        }
        while(!FSEof(handle));

        // THIS CAN'T EVER HAPPEN
        rplError(ERR_ENDOFFILE);
        return;

    }

    case SDWRITETEXT:
    {
        //@SHORT_DESC=Write text to a file (UTF-8 encoding)
        //@NEW
        // WRITE A STRING TO A FILE
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        if(!ISint32_t(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        if(!ISSTRING(*rplPeekData(2))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        int64_t num = rplReadint32_t(rplPeekData(1));
        byte_p stringstart = (byte_p) (rplPeekData(2) + 1);
        int32_t nbytes = rplStrSize(rplPeekData(2));

        FS_FILE *handle;
        int32_t err = FSGetFileFromHandle(num, &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        if(nbytes > 0) {
            err = FSWrite(stringstart, nbytes, handle);

            if(err < 0) {
                rplError(rplFSError2Error(err));
                return;
            }
            if(err != nbytes) {
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
        //@SHORT_DESC=Read one line of text from a file
        //@NEW
        // READ ONE LINE OF TEXT FROM A FILE, RETURN THEM AS STRING OBJECT
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISint32_t(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        int64_t num = rplReadint32_t(rplPeekData(1));

        int32_t bufsize = REAL_REGISTER_STORAGE * 4, readblock;
        byte_p tmpbuf = (byte_p) RReg[0].data, ptr;
        int64_t currentpos, bytecount;
        if(Exceptions)
            return;

        FS_FILE *handle;
        int32_t err = FSGetFileFromHandle(num, &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        currentpos = FSTell(handle);
        bytecount = 0;

        do {

            err = FSRead(tmpbuf, bufsize, handle);
            if(err == 0) {
                rplError(ERR_ENDOFFILE);
                return;
            }

            readblock = err;

            ptr = tmpbuf;
            while((*ptr != '\n') && readblock) {
                ++ptr;
                --readblock;
            }

            if(readblock) {
                // FOUND END OF LINE!
                bytecount += ptr + 1 - tmpbuf;  // INCLUDE THE NEWLINE IN THE RETURNED STRING

                break;

            }

            bytecount += err;
            // HERE WE NEED MORE CHARACTERS AND THE FILE HAS MORE DATA

        }
        while(!FSEof(handle));

        word_p newstring = rplAllocTempOb((bytecount + 3) >> 2);
        if(!newstring) {
            return;
        }

        // NOW GO BACK AND READ THE WHOLE STRING DIRECTLY INTO THE OBJECT
        err = FSSeek(handle, currentpos, FSSEEK_SET);
        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }
        err = FSRead((unsigned char *)(newstring + 1), bytecount, handle);
        if(err != bytecount) {
            rplError(ERR_ENDOFFILE);
            return;
        }

        // EVERYTHING WAS READ CORRECTLY
        *newstring = MAKESTRING(bytecount);

        rplOverwriteData(1, newstring);

        return;

    }

    case SDSEEKCUR:
        //@SHORT_DESC=Move position to given offset from the current point.
        //@NEW
    case SDSEEKEND:
        //@SHORT_DESC=Move position to given offset from end of file
        //@NEW
    case SDSEEKSTA:
    {
        //@SHORT_DESC=Move position to given offset from start of file
        //@NEW
        // MOVE THE FILE POINTER TO THE GIVEN OFFSET.
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        if(!ISint32_t(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        if(!ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }
        int32_t seek_from = FSSEEK_CUR;
        if(OPCODE(CurOpcode) == SDSEEKSTA)
            seek_from = FSSEEK_SET;
        if(OPCODE(CurOpcode) == SDSEEKEND)
            seek_from = FSSEEK_END;

        int64_t num = rplReadint32_t(rplPeekData(1));
        int64_t offset = rplReadNumberAsInt64(rplPeekData(2));
        if(Exceptions)
            return;

        FS_FILE *handle;
        int32_t err = FSGetFileFromHandle(num, &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        FSSeek(handle, offset, seek_from);

        rplDropData(2);

        return;
    }

    case SDTELL:
    {
        //@SHORT_DESC=Get the current position
        //@NEW
        // RETURN THE CURRENT OFFSET WITHIN THE FILE IN BYTES
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISint32_t(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        int64_t num = rplReadint32_t(rplPeekData(1));

        FS_FILE *handle;
        int32_t err = FSGetFileFromHandle(num, &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        int64_t pos = FSTell(handle);

        rplDropData(1);
        rplNewint32_tPush(pos, DECint32_t);

        return;
    }

    case SDFILESIZE:
    {
        //@SHORT_DESC=Get the file size in bytes
        //@NEW
        // RETURN THE SIZE OF THE FILE IN BYTES
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISint32_t(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        int64_t num = rplReadint32_t(rplPeekData(1));

        FS_FILE *handle;
        int32_t err = FSGetFileFromHandle(num, &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        int64_t len = FSFileLength(handle);

        rplDropData(1);
        rplNewint32_tPush(len, DECint32_t);

        return;
    }

    case SDEOF:

    {
        //@SHORT_DESC=Return true if last operation reached end of file
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISint32_t(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        int64_t num = rplReadint32_t(rplPeekData(1));

        FS_FILE *handle;
        int32_t err = FSGetFileFromHandle(num, &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);
        if(FSEof(handle))
            rplPushTrue();
        else
            rplPushFalse();

        return;
    }

    case SDOPENDIR:
    {
        //@SHORT_DESC=Open a directory to scan entries
        //@NEW
        // OPEN A DIRECTORY FOR ENTRY SCANNING
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p path = (byte_p) RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(path, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // OPEN THE FILE
        FS_FILE *handle;
        int32_t err = FSOpenDir((char *)path, &handle);

        if((err != FS_OK) && (err != FS_OPENDIR)) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

        rplNewint32_tPush(FSGetHandle(handle), HEXint32_t);

        return;

    }

    case SDNEXTFILE:
    {
        //@SHORT_DESC=Get the next entry in a directory that is a file
        //@NEW
        // GET NEXT FILE IN A DIRECTORY LISTING
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISint32_t(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        int64_t num = rplReadint32_t(rplPeekData(1));

        FS_FILE *handle;
        int32_t err = FSGetFileFromHandle(num, &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

        FS_FILE entry;

        do {
            err = FSGetNextEntry(&entry, handle);

            if(err != FS_OK) {
                rplError(rplFSError2Error(err));
                return;
            }

            // DON'T ACCEPT THE . AND .. ENTRIES
            if(entry.Name[0] == '.') {
                if(entry.Name[1] == 0) {
                    FSReleaseEntry(&entry);
                    continue;
                }
                if((entry.Name[1] == '.') && entry.Name[2] == 0) {
                    FSReleaseEntry(&entry);
                    continue;
                }
            }
            // DON'T ACCEPT ANY DIRECTORIES OR SPECIAL FILES
            if(entry.
                    Attr & (FSATTR_HIDDEN | FSATTR_DIR | FSATTR_SYSTEM |
                        FSATTR_VOLUME)) {
                FSReleaseEntry(&entry);
                continue;
            }

            break;

        }
        while(1);

        // PUT THE DATA ON A LIST

        word_p *dsave = DSTop;
        word_p newobj =
                rplCreateString((byte_p) entry.Name,
                (byte_p) entry.Name + stringlen(entry.Name));
        if(!newobj) {
            DSTop = dsave;
            FSReleaseEntry(&entry);
            return;
        }
        rplPushData(newobj);

        // PUT THE FILE ATTRIBUTES NEXT
        BYTE attr_string[6];
        attr_string[0] = (entry.Attr & FSATTR_RDONLY) ? 'R' : '_';
        attr_string[1] = (entry.Attr & FSATTR_HIDDEN) ? 'H' : '_';
        attr_string[2] = (entry.Attr & FSATTR_SYSTEM) ? 'S' : '_';
        attr_string[3] = (entry.Attr & FSATTR_DIR) ? 'D' : '_';
        attr_string[4] = (entry.Attr & FSATTR_VOLUME) ? 'V' : '_';
        attr_string[5] = (entry.Attr & FSATTR_ARCHIVE) ? 'A' : '_';

        newobj = rplCreateString(attr_string, attr_string + 6);
        if(!newobj) {
            DSTop = dsave;
            FSReleaseEntry(&entry);
            return;
        }
        rplPushData(newobj);

        // FILE SIZE
        rplNewint32_tPush(entry.FileSize, DECint32_t);
        if(Exceptions) {
            DSTop = dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // LAST MODIFIED DATE
        struct compact_tm date;
        FSGetWriteTime(&entry, &date);
        if(date.tm_mday != 0)   // DAY IS 1-BASED, THEREFORE DAY=0 MEANS DATE/TIME NOT SET
        {
            rplint32_tToRReg(0,
                    (int64_t) date.tm_year + 1900 + (int64_t) (date.tm_mon +
                        1) * 10000 + (int64_t) date.tm_mday * 1000000);
            RReg[0].exp -= 6;
            rplNewRealFromRRegPush(0);
            if(Exceptions) {
                DSTop = dsave;
                FSReleaseEntry(&entry);
                return;
            }
            // LAST MODIFIED TIME
            rplint32_tToRReg(0,
                    (int64_t) date.tm_sec + (int64_t) date.tm_min * 100 +
                    (int64_t) date.tm_hour * 10000);
            RReg[0].exp -= 4;
            rplNewRealFromRRegPush(0);
            if(Exceptions) {
                DSTop = dsave;
                FSReleaseEntry(&entry);
                return;
            }

        }
        else {
            rplPushFalse();
            rplPushFalse();
        }

        // THAT'S ENOUGH DATA
        FSReleaseEntry(&entry);

        return;

    }

    case SDNEXTENTRY:
    {
        //@SHORT_DESC=Get the next entry in a directory
        //@NEW
        // GET NEXT ENTRY IN A DIRECTORY LISTING
        // INCLUDE ALL SPECIAL AND HIDDEN FILES
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISint32_t(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        int64_t num = rplReadint32_t(rplPeekData(1));

        FS_FILE *handle;
        int32_t err = FSGetFileFromHandle(num, &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

        FS_FILE entry;

        err = FSGetNextEntry(&entry, handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        // PUT THE DATA ON A LIST

        word_p *dsave = DSTop;
        word_p newobj =
                rplCreateString((byte_p) entry.Name,
                (byte_p) entry.Name + stringlen(entry.Name));
        if(!newobj) {
            DSTop = dsave;
            FSReleaseEntry(&entry);
            return;
        }
        rplPushData(newobj);

        // PUT THE FILE ATTRIBUTES NEXT
        BYTE attr_string[6];
        attr_string[0] = (entry.Attr & FSATTR_RDONLY) ? 'R' : '_';
        attr_string[1] = (entry.Attr & FSATTR_HIDDEN) ? 'H' : '_';
        attr_string[2] = (entry.Attr & FSATTR_SYSTEM) ? 'S' : '_';
        attr_string[3] = (entry.Attr & FSATTR_DIR) ? 'D' : '_';
        attr_string[4] = (entry.Attr & FSATTR_VOLUME) ? 'V' : '_';
        attr_string[5] = (entry.Attr & FSATTR_ARCHIVE) ? 'A' : '_';

        newobj = rplCreateString(attr_string, attr_string + 6);
        if(!newobj) {
            DSTop = dsave;
            FSReleaseEntry(&entry);
            return;
        }
        rplPushData(newobj);

        // FILE SIZE
        rplNewint32_tPush(entry.FileSize, DECint32_t);
        if(Exceptions) {
            DSTop = dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // LAST MODIFIED DATE
        struct compact_tm date;
        FSGetWriteTime(&entry, &date);
        rplint32_tToRReg(0,
                (int64_t) date.tm_year + 1900 + (int64_t) (date.tm_mon +
                    1) * 10000 + (int64_t) date.tm_mday * 1000000);
        RReg[0].exp -= 6;
        rplNewRealFromRRegPush(0);
        if(Exceptions) {
            DSTop = dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // LAST MODIFIED TIME

        rplint32_tToRReg(0,
                (int64_t) date.tm_sec + (int64_t) date.tm_min * 100 +
                (int64_t) date.tm_hour * 10000);
        RReg[0].exp -= 4;
        rplNewRealFromRRegPush(0);
        if(Exceptions) {
            DSTop = dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // THAT'S ENOUGH DATA
        FSReleaseEntry(&entry);

        return;

    }
    case SDNEXTDIR:
    {
        //@SHORT_DESC=Get the next entry in a directory that is a subdirectory
        //@NEW
        // GET NEXT DIRECTORY IN A DIRECTORY LISTING
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISint32_t(*rplPeekData(1))) {
            rplError(ERR_INVALIDHANDLE);
            return;
        }

        int64_t num = rplReadint32_t(rplPeekData(1));

        FS_FILE *handle;
        int32_t err = FSGetFileFromHandle(num, &handle);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(1);

        FS_FILE entry;

        do {
            err = FSGetNextEntry(&entry, handle);

            if(err != FS_OK) {
                rplError(rplFSError2Error(err));
                return;
            }

            // DON'T ACCEPT THE . AND .. ENTRIES
            if(entry.Name[0] == '.') {
                if(entry.Name[1] == 0) {
                    FSReleaseEntry(&entry);
                    continue;
                }
                if((entry.Name[1] == '.') && entry.Name[2] == 0) {
                    FSReleaseEntry(&entry);
                    continue;
                }
            }
            // DON'T ACCEPT ANY SPECIAL FILES OR SPECIAL DIRECTORIES
            if(entry.Attr & (FSATTR_HIDDEN | FSATTR_SYSTEM | FSATTR_VOLUME)) {
                FSReleaseEntry(&entry);
                continue;
            }

            // DON'T ACCEPT FILES THAT ARE NOT DIRECTORIES
            if(!(entry.Attr & FSATTR_DIR)) {
                FSReleaseEntry(&entry);
                continue;
            }

            break;

        }
        while(1);

        // PUT THE DATA ON A LIST

        word_p *dsave = DSTop;
        word_p newobj =
                rplCreateString((byte_p) entry.Name,
                (byte_p) entry.Name + stringlen(entry.Name));
        if(!newobj) {
            DSTop = dsave;
            FSReleaseEntry(&entry);
            return;
        }
        rplPushData(newobj);

        // PUT THE FILE ATTRIBUTES NEXT
        BYTE attr_string[6];
        attr_string[0] = (entry.Attr & FSATTR_RDONLY) ? 'R' : '_';
        attr_string[1] = (entry.Attr & FSATTR_HIDDEN) ? 'H' : '_';
        attr_string[2] = (entry.Attr & FSATTR_SYSTEM) ? 'S' : '_';
        attr_string[3] = (entry.Attr & FSATTR_DIR) ? 'D' : '_';
        attr_string[4] = (entry.Attr & FSATTR_VOLUME) ? 'V' : '_';
        attr_string[5] = (entry.Attr & FSATTR_ARCHIVE) ? 'A' : '_';

        newobj = rplCreateString(attr_string, attr_string + 6);
        if(!newobj) {
            DSTop = dsave;
            FSReleaseEntry(&entry);
            return;
        }
        rplPushData(newobj);

        // FILE SIZE
        rplNewint32_tPush(entry.FileSize, DECint32_t);
        if(Exceptions) {
            DSTop = dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // LAST MODIFIED DATE
        struct compact_tm date;
        FSGetWriteTime(&entry, &date);
        rplint32_tToRReg(0,
                (int64_t) date.tm_year + 1900 + (int64_t) (date.tm_mon +
                    1) * 10000 + (int64_t) date.tm_mday * 1000000);
        RReg[0].exp -= 6;
        rplNewRealFromRRegPush(0);
        if(Exceptions) {
            DSTop = dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // LAST MODIFIED TIME

        rplint32_tToRReg(0,
                (int64_t) date.tm_sec + (int64_t) date.tm_min * 100 +
                (int64_t) date.tm_hour * 10000);
        RReg[0].exp -= 4;
        rplNewRealFromRRegPush(0);
        if(Exceptions) {
            DSTop = dsave;
            FSReleaseEntry(&entry);
            return;
        }

        // THAT'S ENOUGH DATA
        FSReleaseEntry(&entry);

        return;

    }

    case SDCOPY:
    {
        //@SHORT_DESC=Copy a file
        //@NEW
        // COPY A FILE
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        if(!ISIDENT(*rplPeekData(2)) && !ISSTRING(*rplPeekData(2))
                && !ISLIST(*rplPeekData(2))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p pathfrom = (byte_p) RReg[0].data;
        byte_p pathto = (byte_p) RReg[1].data;

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(pathto, rplPeekData(1) + 1, pathlen);
            pathto[pathlen] = 0;        // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(pathto, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(pathto, rplPeekData(1) + 1, pathlen);
            pathto[pathlen] = 0;        // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        if(ISIDENT(*rplPeekData(2))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(2));
            memmoveb(pathfrom, rplPeekData(2) + 1, pathlen);
            pathfrom[pathlen] = 0;      // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(2))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(pathfrom, rplPeekData(2))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(2))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(2));
            memmoveb(pathfrom, rplPeekData(2) + 1, pathlen);
            pathfrom[pathlen] = 0;      // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // FINALLY, COPY THE FILE
        FS_FILE *handleto, *handlefrom;
        int32_t err = FSOpen((char *)pathto, FSMODE_WRITE, &handleto);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        err = FSOpen((char *)pathfrom, FSMODE_READ, &handlefrom);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        // JUST READ AND WRITE, USE REAL NUMBERS AS STORAGE

        int32_t nbytes = FSFileLength(handlefrom);

        // WARNING: THIS ASSUMES REAL_REGISTER_STORAGE > 1024 BYTES
        // MAKE SURE THIS IS TRUE!

#if (REAL_REGISTER_STORAGE < 256)
#error This part of the code relies on at least 1024 bytes of storage per register.
#endif
        // IF WE HAD A LARGER BUFFER THIS COULD BE FASTER
        while(nbytes > 1024) {
            FSRead(pathto, 1024, handlefrom);
            err = FSWrite(pathto, 1024, handleto);
            if(err != 1024) {
                if(err <= 0)
                    rplError(rplFSError2Error(err));
                else
                    rplError(ERR_CANTWRITE);
                FSClose(handleto);
                FSClose(handlefrom);
                return;
            }
            nbytes -= 1024;
        }

        if(nbytes) {
            FSRead(pathto, nbytes, handlefrom);
            err = FSWrite(pathto, nbytes, handleto);
            if(err != nbytes) {
                if(err <= 0)
                    rplError(rplFSError2Error(err));
                else
                    rplError(ERR_CANTWRITE);
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
        //@SHORT_DESC=Move or rename a file
        //@NEW
        // MOVE/RENAME A FILE
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        if(!ISIDENT(*rplPeekData(2)) && !ISSTRING(*rplPeekData(2))
                && !ISLIST(*rplPeekData(2))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p pathfrom = (byte_p) RReg[0].data;
        byte_p pathto = (byte_p) RReg[1].data;

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(pathto, rplPeekData(1) + 1, pathlen);
            pathto[pathlen] = 0;        // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(pathto, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(pathto, rplPeekData(1) + 1, pathlen);
            pathto[pathlen] = 0;        // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        if(ISIDENT(*rplPeekData(2))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(2));
            memmoveb(pathfrom, rplPeekData(2) + 1, pathlen);
            pathfrom[pathlen] = 0;      // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(2))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(pathfrom, rplPeekData(2))) {
                rplError(ERR_BADFILENAME);
                return;
            }

        }
        else if(ISSTRING(*rplPeekData(2))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(2));
            memmoveb(pathfrom, rplPeekData(2) + 1, pathlen);
            pathfrom[pathlen] = 0;      // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // FINALLY, MOVE THE FILE
        int32_t err = FSRename((char *)pathfrom, (char *)pathto);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        rplDropData(2);

        return;
    }

    case SDPATH:
    {
        //@SHORT_DESC=Get the path to current directory
        //@NEW
        // RETURN THE CURRENT WORK DIRECTORY
        int32_t cvol = FSGetCurrentVolume();

        if(cvol < 0) {
            rplError(rplFSError2Error(cvol));
            return;
        }

        byte_p path = (byte_p) FSGetcwd(cvol), endpath;
        if(!path) {
            rplError(ERR_UNKNOWNFSERROR);
            return;
        }
        endpath = path;
        while(*endpath)
            ++endpath;
        word_p newstring = rplCreateString(path, endpath);

        if(!newstring)
            return;

        rplPushData(newstring);

        // VERY IMPORTANT TO RELEASE THE MEMORY
        // ALLOCATED BY THE FILE SYSTEM FOR THE PATH!
        simpfree(path);

        return;
    }

    case SDFREE:
    {
        //@SHORT_DESC=Get the free space in the current volume
        //@NEW
        // RETURN THE FREE SPACE IN BYTES IN THE CURRENT PARTITION
        int32_t cvol = FSGetCurrentVolume();

        if(cvol < 0) {
            rplError(rplFSError2Error(cvol));
            return;
        }

        int64_t size = FSGetVolumeFree(cvol);
        if(size < 0) {
            rplError(rplFSError2Error(size));
            return;
        }

        rplNewint32_tPush(size * 512, DECint32_t);

        return;
    }

    case SDARCHIVE:
    {
        //@SHORT_DESC=Create a full calculator backup on a file
        //@NEW
        // STORE A BACKUP OBJECT DIRECTLY INTO A FILE
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p path = (byte_p) RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(path, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }
        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // TRY TO OPEN THE FILE

        FS_FILE *objfile;
        int err;
        err = FSOpen((char *)(RReg[0].data), FSMODE_WRITE | FSMODE_WRITEBUFFERS,
                &objfile);

        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        // ALSO PACK AND SAVE THE STACK
        word_p stklist = 0;

        if(rplDepthData() > 1)
            stklist = rplCreateListN(rplDepthData() - 1, 2, 0);
        // JUST DON'T SAVE THE STACK IF THERE'S NOT ENOUGH MEMORY FOR IT
        if(stklist) {
            rplListAutoExpand(stklist);
            rplStoreSettings((word_p) stksave_ident, stklist);
        }
        else
            rplPurgeSettings((word_p) stksave_ident);

        err = rplBackup(&rplSDArchiveWriteWord, (void *)objfile);

        if(err != FS_OK) {
            FSClose(objfile);
            rplError(ERR_CANTWRITE);
            return;
        }
        FSClose(objfile);
        rplDropData(1);
        return;

    }

    case SDRESTORE:
    {
        //@SHORT_DESC=Restore from a backup stored in a file
        //@NEW
        // STORE A BACKUP OBJECT DIRECTLY INTO A FILE
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1)) && !ISSTRING(*rplPeekData(1))
                && !ISLIST(*rplPeekData(1))) {
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        byte_p path = (byte_p) RReg[0].data;

        // USE RReg[0] TO STORE THE FILE PATH

        if(ISIDENT(*rplPeekData(1))) {
            int32_t pathlen = rplGetIdentLength(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING
        }
        else if(ISLIST(*rplPeekData(1))) {
            // MAKE A PATH BY APPENDING ALL STRINGS/IDENTS
            if(!rplPathFromList(path, rplPeekData(1))) {
                rplError(ERR_BADFILENAME);
                return;
            }
        }
        else if(ISSTRING(*rplPeekData(1))) {
            // FULL PATH GIVEN
            int32_t pathlen = rplStrSize(rplPeekData(1));
            memmoveb(path, rplPeekData(1) + 1, pathlen);
            path[pathlen] = 0;  // NULL TERMINATED STRING

        }
        else {
            // TODO: ACCEPT TAGGED NAMES WHEN TAGS EXIST
            rplError(ERR_IDENTORPATHEXPECTED);
            return;
        }

        // TRY TO OPEN THE FILE

        FS_FILE *objfile;
        int err;

        err = FSOpen((char *)(RReg[0].data), FSMODE_READ, &objfile);
        if(err != FS_OK) {
            rplError(rplFSError2Error(err));
            return;
        }

        GCFlags = GC_IN_PROGRESS;       // MARK THAT A GC IS IN PROGRESS TO BLOCK ANY HARDWARE INTERRUPTS

        err = rplRestoreBackup(0, &rplSDArchiveReadWord, (void *)objfile);
        FSClose(objfile);

        switch (err) {
        case -1:
            // FILE WAS CORRUPTED, AND MEMORY WAS DESTROYED
            GCFlags = GC_COMPLETED;     // MARK THAT GC WAS COMPLETED
            FSShutdown();
            throw_dbgexception("Memory lost during restore",
                    EX_WIPEOUT | EX_RESET | EX_NOREG);
            // THIS WON'T RETURN, ONLY A RESET IS ACCEPTABLE AT THIS POINT
            return;
        case 0:
            // FILE WAS CORRUPTED, BUT MEMORY IS STILL INTACT
            rplError(ERR_INVALIDDATA);
            GCFlags = 0;        // NOTHING MOVED IN MEMORY, SO DON'T SIGNAL THAT A GC TOOK PLACE
            return;
        default:
        case 1:
            // SUCCESS! STILL NEED TO DO A WARMSTART
            // FALL THROUGH
        case 2:
            // SOME ERRORS, BUT rplWarmInit WILL FIX AUTOMATICALLY
            GCFlags = GC_COMPLETED;     // MARK THAT GC WAS COMPLETED SO HARDWARE INTERRUPTS ARE ACCEPTED AGAIN
            rplException(EX_POWEROFF | EX_HALRESET);    // REQUEST A COMPLETE HAL RESET
            return;
        }

        return;

    }

    case SDGETPART:
    {
        //@SHORT_DESC=Get the current partition number
        //@NEW
        // RETURN THE CURRENT VOLUME AS A NUMBER THAT CAN BE USED BY SDSETPART LATER
        int32_t cvol = FSGetCurrentVolume();

        if(cvol < 0) {
            rplError(rplFSError2Error(cvol));
            return;
        }

        rplNewint32_tPush(cvol + 3, DECint32_t);
        return;
    }

#endif

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

        libCompileCmds(LIBRARY_NUMBER, (char **)LIB_NAMES, NULL,
                LIB_NUMBEROFCMDS);
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
        libDecompileCmds((char **)LIB_NAMES, NULL, LIB_NUMBEROFCMDS);
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

        RetNum = OK_CONTINUE;
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
        libProbeCmds((char **)LIB_NAMES, (int32_t *) LIB_TOKENINFO,
                LIB_NUMBEROFCMDS);

        return;
    }

    case OPCODE_GETINFO:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL COMMAND OR OBJECT IN ObjectPTR
        // NEEDS TO RETURN INFORMATION ABOUT THE TYPE:
        // IN RetNum: RETURN THE MKTOKENINFO() DATA FOR THE SYMBOLIC COMPILER AND CAS
        // IN DecompHints: RETURN SOME HINTS FOR THE DECOMPILER TO DO CODE BEAUTIFICATION (TO BE DETERMINED)
        // IN TypeInfo: RETURN TYPE INFORMATION FOR THE TYPE COMMAND
        //             TypeInfo: TTTTFF WHERE TTTT = MAIN TYPE * 100 (NORMALLY THE MAIN LIBRARY NUMBER)
        //                                FF = 2 DECIMAL DIGITS FOR THE SUBTYPE OR FLAGS (VARIES DEPENDING ON LIBRARY)
        //             THE TYPE COMMAND WILL RETURN A REAL NUMBER TypeInfo/100
        // FOR NUMBERS: TYPE=10 (REALS), SUBTYPES = .01 = APPROX., .02 = INTEGER, .03 = APPROX. INTEGER
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL int32_t, .42 = HEX INTEGER

        if(ISPROLOG(*ObjectPTR)) {
            TypeInfo = LIBRARY_NUMBER * 100;
            DecompHints = 0;
            RetNum = OK_TOKENINFO | MKTOKENINFO(0, TITYPE_NOTALLOWED, 0, 1);
        }
        else {
            TypeInfo = 0;       // ALL COMMANDS ARE TYPE 0
            DecompHints = 0;
            libGetInfo2(*ObjectPTR, (char **)LIB_NAMES, (int32_t *) LIB_TOKENINFO,
                    LIB_NUMBEROFCMDS);
        }
        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, ObjectIDHash=hash, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER, (word_p *) ROMPTR_TABLE, ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID, ObjectIDHash=hash
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((word_p *) ROMPTR_TABLE, ObjectID, ObjectIDHash);
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) {
            RetNum = ERR_INVALID;
            return;
        }

        RetNum = OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER, (char **)LIB_NAMES,
                LIB_NUMBEROFCMDS);
        return;
    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(MENUNUMBER(MenuCodeArg) > 0) {
            RetNum = ERR_NOTMINE;
            return;
        }
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR = ROMPTR_TABLE[MENUNUMBER(MenuCodeArg) + 2];
        RetNum = OK_CONTINUE;
        return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp, (word_p) LIB_HELPTABLE);
        return;
    }
    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {

        libFindMsg(LibError, (word_p) LIB_MSGTABLE);
        return;
    }

    case OPCODE_LIBINSTALL:
        LibraryList = (word_p) libnumberlist;
        RetNum = OK_CONTINUE;
        return;
    case OPCODE_LIBREMOVE:
        return;

    }
    // UNHANDLED OPCODE...

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode) >= MIN_RESERVED_OPCODE) {
        RetNum = ERR_NOTMINE;
        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR

#ifndef CONFIG_NO_FSYSTEM
    rplError(ERR_INVALIDOPCODE);
#else
    rplError(ERR_NOCARD);
#endif

    return;

}

#endif
