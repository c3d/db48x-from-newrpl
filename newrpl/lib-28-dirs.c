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
#define LIBRARY_NUMBER  28


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(STO,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(RCL,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(INCR,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(DECR,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(PURGE,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(CRDIR,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(PGDIR,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(UPDIR,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(HOME,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(PATH,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2))

//    ECMD(CMDNAME,"CMDNAME",MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

// THESE ARE SPECIAL OPCODES FOR THE COMPILER ONLY
// THE LOWER 16 BITS ARE THE NUMBER OF LAMS TO CREATE, OR THE INDEX OF LAM NUMBER TO STO/RCL
#define NEWNLOCALS 0x40000   // SPECIAL OPCODE TO CREATE NEW LOCAL VARIABLES
#define GETLAMN    0x20000   // SPECIAL OPCODE TO RCL THE CONTENT OF A LAM
#define PUTLAMN    0x10000   // SPECIAL OPCODE TO STO THE CONTENT OF A LAM


ROMOBJECT dir_start_bint[]=
{
    (WORD)DIR_START_MARKER
};
ROMOBJECT dir_end_bint[]=
{
    (WORD)DIR_END_MARKER
};
ROMOBJECT dir_parent_bint[]=
{
    (WORD)DIR_PARENT_MARKER
};
ROMOBJECT root_dir_handle[]=
{
    (WORD)MKPROLOG(DODIR,1),
    (WORD)0
};

ROMOBJECT home_opcode[]=
{
    (WORD)MKOPCODE(LIBRARY_NUMBER,HOME)
};

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)dir_start_bint,
    (WORDPTR)dir_parent_bint,
    (WORDPTR)dir_end_bint,
    (WORDPTR)root_dir_handle,
    (WORDPTR)home_opcode,
    0
};









void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        // THIS SHOULD NEVER HAPPEN, AS DIRECTORY OBJECTS ARE SPECIAL HANDLES
        // THEY ARE NEVER USED IN THE MIDDLE OF THE CODE
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case STO:
    {
        // STORE CONTENT INSIDE A LAM OR GLOBAL VARIABLE, CREATE A NEW "GLOBAL" VARIABLE IF NEEDED
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        WORDPTR *val=rplFindLAM(rplPeekData(1),1);



        if(val) {
            if(ISDIR(*val[1])) {
                rplError(ERR_CANTOVERWRITEDIR);
                return;
            }
            val[1]=rplPeekData(2);
            rplDropData(2);
        }
        else {
            // LAM WAS NOT FOUND, TRY A GLOBAL
            val=rplFindGlobal(rplPeekData(1),0);
            if(val) {
                if(ISDIR(*val[1])) {
                    rplError(ERR_CANTOVERWRITEDIR);
                    return;
                }
            }
            // HANDLE SPECIAL CASE OF STORING DIRECTORY OBJECTS
            WORDPTR obj=rplPeekData(2);
            if(LIBNUM(*obj)==DODIR) {
                WORDPTR *sourcedir=rplFindDirbyHandle(obj);
                if(sourcedir) {
                    WORDPTR *newdir=rplDeepCopyDir(sourcedir);
                    if(newdir) {
                        if(val) {
                            *(newdir+3)=*(rplGetDirfromGlobal(val)+1);   // SET PARENT DIR
                            *(val+1)=*(newdir+1);                   // AND NEW HANDLE
                        }
                        else {
                            *(newdir+3)=*(CurrentDir+1);
                            WORDPTR name=rplMakeIdentQuoted(rplPeekData(1));
                            rplCreateGlobal(name,*(newdir+1));
                        }
                        rplDropData(2);
                        return;
                    }
                }
            }



            if(val) {
                val[1]=rplPeekData(2);
            }
            else {
                // CREATE A NEW GLOBAL VARIABLE
                WORDPTR name=rplMakeIdentQuoted(rplPeekData(1));
                rplCreateGlobal(name,rplPeekData(2));
            }
            rplDropData(2);
        }
    }
    return;


    case RCL:
    {
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;

        }

        WORDPTR val=rplGetLAM(rplPeekData(1));
        if(val) {
            rplOverwriteData(1,val);
        }
        else {
            // NO LAM, TRY A GLOBAL
            val=rplGetGlobal(rplPeekData(1));
            if(val) {
                rplOverwriteData(1,val);
            }
            else {
                rplError(ERR_UNDEFINEDVARIABLE);
            return;
            }
        }
    }
        return;

    case INCR:
        {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;

        }

        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        WORDPTR *var=rplFindLAM(rplPeekData(1),1);
        if(!var) var=rplFindGlobal(rplPeekData(1),1);
        if(var) {
                if(ISDIR(*var[1])) {
                    rplError(ERR_CANTOVERWRITEDIR);
                    return;
                }

            rplOverwriteData(1,*(var+1));
            rplPushData((WORDPTR)one_bint);       // PUSH THE NUMBER ONE

            // CALL THE OVERLOADED OPERATOR '+'

            rplCallOvrOperator(OVR_ADD);

            if(Exceptions) return;


            *(var+1)=rplPeekData(1);      // STORE THE INCREMENTED COUNTER
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

        }
        return;


    case DECR:
    {
    if(rplDepthData()<1) {
        rplError(ERR_BADARGCOUNT);
        return;
    }
    // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

    if(!ISIDENT(*rplPeekData(1))) {
        rplError(ERR_IDENTEXPECTED);
        return;

    }

    // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

    WORDPTR *var=rplFindLAM(rplPeekData(1),1);
    if(!var) var=rplFindGlobal(rplPeekData(1),1);
    if(var) {
        if(ISDIR(*var[1])) {
            rplError(ERR_CANTOVERWRITEDIR);
            return;
        }


        rplOverwriteData(1,*(var+1));
        rplPushData((WORDPTR)one_bint);       // PUSH THE NUMBER ONE

        // CALL THE OVERLOADED OPERATOR '+'

        rplCallOvrOperator(OVR_SUB);

        if(Exceptions) return;


        *(var+1)=rplPeekData(1);      // STORE THE INCREMENTED COUNTER
    }
    else {
        rplError(ERR_UNDEFINEDVARIABLE);
        return;
    }

    }
    return;



    case PURGE:
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)


        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }


        rplPurgeGlobal(rplPeekData(1));
        if(!Exceptions) rplDropData(1);
        return;

    case CRDIR:
    {
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;

        }
        WORDPTR name=rplMakeIdentQuoted(rplPeekData(1));
        rplCreateNewDir(name,CurrentDir);

        rplDropData(1);

     }
    return;
    case PGDIR:
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {

            WORDPTR *savestk=DSTop;
            WORDPTR newobj=rplAllocTempOb(2);
            if(!newobj) return;
            // CREATE A PROGRAM AND RUN THE MAP COMMAND
            newobj[0]=MKPROLOG(DOCOL,2);
            newobj[1]=CurOpcode;
            newobj[2]=CMD_SEMI;

            rplPushData(newobj);

            rplCallOperator(CMD_MAP);

            if(Exceptions) {
                if(DSTop>savestk) DSTop=savestk;
            }

            // EXECUTION WILL CONTINUE AT MAP

            return;
        }


        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        // TODO: ALSO ACCEPT A LIST OF VARS, WHEN WE HAVE LISTS!

        rplPurgeDir(rplPeekData(1));
        if(!Exceptions) rplDropData(1);
        return;

    case UPDIR:
    {
        WORDPTR *dir=rplGetParentDir(CurrentDir);
        if(dir) CurrentDir=dir;
        return;
    }
    return;
    case HOME:
        CurrentDir=Directories;
        return;
    case PATH:

    {
        WORDPTR *scandir=CurrentDir;
        BINT nitems=0;
        WORDPTR *stksave=DSTop;

        while(scandir!=Directories) {
            WORDPTR name=rplGetDirName(scandir);
            if(name) {
                rplPushData(name);
                if(Exceptions) { DSTop=stksave; return; }
                ++nitems;
            }
            scandir=rplGetParentDir(scandir);
        }

        if(scandir==Directories) {
            rplPushData((WORDPTR)home_opcode);
            ++nitems;
            if(Exceptions) { DSTop=stksave; return; }
        }

        if(nitems==0) rplPushData((WORDPTR)empty_list);
        else {
            rplNewBINTPush(nitems,DECBINT);
            rplCreateList();
        }
        if(Exceptions) DSTop=stksave;
        return;
    }


    // ADD MORE OPCODES HERE

    case OVR_EVAL:
    case OVR_EVAL1:
    case OVR_XEQ:
    // EVALUATING THE OBJECT HAS TO CHANGE THE CURRENT DIRECTORY INTO THIS ONE
    {
        if(!ISPROLOG(*rplPeekData(1))) {
        WORD saveOpcode=CurOpcode;
        CurOpcode=*rplPopData();
        // RECURSIVE CALL
        LIB_HANDLER();
        CurOpcode=saveOpcode;
        return;
        }

        WORDPTR *dir=rplFindDirbyHandle(rplPeekData(1));

        if(!dir) {
        rplError(ERR_DIRECTORYNOTFOUND);
        return; //  LEAVE THE OBJECT UNEVALUATED. IT'S AN ORPHAN DIRECTORY OBJECT???
        }
        CurrentDir=dir;
        rplDropData(1);
        return;
    }
        return;
    case OVR_NUM:
        // DO NOTHING
        return;

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

        if(ISPROLOG(*DecompileObject)) {
            rplDecompAppendString2((BYTEPTR)"DIRObject",9);

            return;

        }




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

        if(ISPROLOG(*ObjectPTR)) {
        // DIRECTORY OBJECTS CAN ONLY HAVE 1 WORD
        if(OBJSIZE(*ObjectPTR)!=1) { RetNum=ERR_INVALID; return; }

        // ANY OTHER CHECKS TO DIRECTORY STRUCTURE ARE DANGEROUS TO BE DONE HERE
        }
        RetNum=OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        return;


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




