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
    ECMD(STOADD,"STO+",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STOSUB,"STO-",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STOMUL,"STO*",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STODIV,"STO/",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(SINV,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(SNEG,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(SCONJ,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(INCR,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(DECR,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(PURGE,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(CRDIR,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(PGDIR,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(UPDIR,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(HOME,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(PATH,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(VARS,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(ALLVARS,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(ORDER,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(QUOTEID,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(UNQUOTEID,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(HIDEVAR,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(UNHIDEVAR,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(CLVAR,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(LOCKVAR,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(UNLOCKVAR,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2))




//    ECMD(CMDNAME,"CMDNAME",MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE
#define ERROR_LIST \
    ERR(NONEMPTYDIRECTORY,0), \
    ERR(DIRECTORYNOTFOUND,1), \
    ERR(CANTOVERWRITEDIR,2), \
    ERR(READONLYVARIABLE,3)


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


INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib28_menu);


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib28_menu,
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
            if(ISLOCKEDIDENT(*val[0])) {
                rplError(ERR_READONLYVARIABLE);
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
                if(ISLOCKEDIDENT(*val[0])) {
                    rplError(ERR_READONLYVARIABLE);
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

    case STOADD:
        {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;

        }

        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        WORDPTR *stksave=DSTop;

        WORDPTR *var=rplFindLAM(rplPeekData(1),1);
        if(!var) var=rplFindGlobal(rplPeekData(1),1);
        if(var) {
                if(ISDIR(*var[1])) {
                    rplError(ERR_CANTOVERWRITEDIR);
                    return;
                }
                if(ISLOCKEDIDENT(*var[0])) {
                    rplError(ERR_READONLYVARIABLE);
                    return;
                }


            rplPushData(*(var+1));

            rplPushData(rplPeekData(3));       // PUSH THE OBJECT TO ADD

            // CALL THE OVERLOADED OPERATOR '+'

            rplCallOvrOperator(CMD_OVR_ADD);

            if(Exceptions) { DSTop=stksave; return; }

            *(var+1)=rplPeekData(1);      // STORE THE INCREMENTED COUNTER
            rplDropData(3);
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

        }
        return;


    case STOSUB:
        {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;

        }

        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        WORDPTR *stksave=DSTop;

        WORDPTR *var=rplFindLAM(rplPeekData(1),1);
        if(!var) var=rplFindGlobal(rplPeekData(1),1);
        if(var) {
                if(ISDIR(*var[1])) {
                    rplError(ERR_CANTOVERWRITEDIR);
                    return;
                }
                if(ISLOCKEDIDENT(*var[0])) {
                    rplError(ERR_READONLYVARIABLE);
                    return;
                }

            rplPushData(*(var+1));

            rplPushData(rplPeekData(3));       // PUSH THE OBJECT TO OPERATE

            // CALL THE OVERLOADED OPERATOR '-'

            rplCallOvrOperator(CMD_OVR_SUB);

            if(Exceptions) { DSTop=stksave; return; }

            *(var+1)=rplPeekData(1);      // STORE THE INCREMENTED COUNTER
            rplDropData(3);
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

        }
        return;


    case STOMUL:
        {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;

        }

        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        WORDPTR *stksave=DSTop;

        WORDPTR *var=rplFindLAM(rplPeekData(1),1);
        if(!var) var=rplFindGlobal(rplPeekData(1),1);
        if(var) {
                if(ISDIR(*var[1])) {
                    rplError(ERR_CANTOVERWRITEDIR);
                    return;
                }
                if(ISLOCKEDIDENT(*var[0])) {
                    rplError(ERR_READONLYVARIABLE);
                    return;
                }

            rplPushData(*(var+1));

            rplPushData(rplPeekData(3));       // PUSH THE OBJECT TO OPERATE

            // CALL THE OVERLOADED OPERATOR '*'

            rplCallOvrOperator(CMD_OVR_MUL);

            if(Exceptions) { DSTop=stksave; return; }

            *(var+1)=rplPeekData(1);      // STORE THE INCREMENTED COUNTER
            rplDropData(3);
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

        }
        return;


    case STODIV:
        {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;

        }

        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        WORDPTR *stksave=DSTop;

        WORDPTR *var=rplFindLAM(rplPeekData(1),1);
        if(!var) var=rplFindGlobal(rplPeekData(1),1);
        if(var) {
                if(ISDIR(*var[1])) {
                    rplError(ERR_CANTOVERWRITEDIR);
                    return;
               }
                if(ISLOCKEDIDENT(*var[0])) {
                    rplError(ERR_READONLYVARIABLE);
                    return;
                }

            rplPushData(*(var+1));

            rplPushData(rplPeekData(3));       // PUSH THE OBJECT TO OPERATE

            // CALL THE OVERLOADED OPERATOR '/'

            rplCallOvrOperator(CMD_OVR_DIV);

            if(Exceptions) { DSTop=stksave; return; }

            *(var+1)=rplPeekData(1);      // STORE THE INCREMENTED COUNTER
            rplDropData(3);
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

        }
        return;

    case SINV:
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
                if(ISLOCKEDIDENT(*var[0])) {
                    rplError(ERR_READONLYVARIABLE);
                    return;
                }

            rplPushData(*(var+1));

            // CALL THE OVERLOADED OPERATOR 'INV'

            rplCallOvrOperator(CMD_OVR_INV);

            if(Exceptions) return;


            *(var+1)=rplPeekData(1);      // STORE THE NEW VALUE
            rplDropData(2);
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

        }
        return;

    case SNEG:
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
                if(ISLOCKEDIDENT(*var[0])) {
                    rplError(ERR_READONLYVARIABLE);
                    return;
                }

            rplPushData(*(var+1));

            // CALL THE OVERLOADED OPERATOR

            rplCallOvrOperator(CMD_OVR_NEG);

            if(Exceptions) return;


            *(var+1)=rplPeekData(1);      // STORE THE NEW VALUE
            rplDropData(2);
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

        }
        return;

    case SCONJ:
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
                if(ISLOCKEDIDENT(*var[0])) {
                    rplError(ERR_READONLYVARIABLE);
                    return;
                }

            rplPushData(*(var+1));

            // CALL THE COMMAND

            rplCallOperator(CMD_CONJ);

            if(Exceptions) return;


            *(var+1)=rplPeekData(1);      // STORE THE NEW VALUE
            rplDropData(2);
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
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
                if(ISLOCKEDIDENT(*var[0])) {
                    rplError(ERR_READONLYVARIABLE);
                    return;
                }

            rplOverwriteData(1,*(var+1));
            rplPushData((WORDPTR)one_bint);       // PUSH THE NUMBER ONE

            // CALL THE OVERLOADED OPERATOR '+'

            rplCallOvrOperator(CMD_OVR_ADD);

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
        if(ISLOCKEDIDENT(*var[0])) {
            rplError(ERR_READONLYVARIABLE);
            return;
        }


        rplOverwriteData(1,*(var+1));
        rplPushData((WORDPTR)one_bint);       // PUSH THE NUMBER ONE

        // CALL THE OVERLOADED OPERATOR '+'

        rplCallOvrOperator(CMD_OVR_SUB);

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

            rplCallOperator(CMD_MAPINNERCOMP);

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

            rplCallOperator(CMD_MAPINNERCOMP);

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
            // REVERSE THE ORDER IN THE STACK, AS HOME IS THE LAST ONE

            BINT f;
            WORDPTR obj;
            for(f=1;f<=nitems/2;++f) {
                obj=rplPeekData(f);
                rplOverwriteData(f,rplPeekData(nitems+1-f));
                rplOverwriteData(nitems+1-f,obj);
            }
            rplNewBINTPush(nitems,DECBINT);
            rplCreateList();
        }
        if(Exceptions) DSTop=stksave;
        return;
    }

    case VARS:
    {
       WORDPTR *varptr=rplFindFirstInDir(CurrentDir);
       WORDPTR *stksave=DSTop;
       BINT nitems=0;

       while(varptr) {
           if(ISIDENT(*varptr[0]) && !ISHIDDENIDENT(*varptr[0])) { rplPushData(varptr[0]); ++nitems; }
           varptr=rplFindNext(varptr);
       }

       if(nitems==0) rplPushData((WORDPTR)empty_list);
       else {
           rplNewBINTPush(nitems,DECBINT);
           if(Exceptions) { DSTop=stksave; return; }
           rplCreateList();
           if(Exceptions) { DSTop=stksave; return; }
       }


      return;
     }

    case ALLVARS:
    {
       WORDPTR *varptr=rplFindFirstInDir(CurrentDir);
       WORDPTR *stksave=DSTop;
       BINT nitems=0;

       while(varptr) {
           rplPushData(varptr[0]);
           ++nitems;
           varptr=rplFindNext(varptr);
       }

       if(nitems==0) rplPushData((WORDPTR)empty_list);
       else {
           rplNewBINTPush(nitems,DECBINT);
           if(Exceptions) { DSTop=stksave; return; }
           rplCreateList();
           if(Exceptions) { DSTop=stksave; return; }
       }


      return;
     }


     case ORDER:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISLIST(*rplPeekData(1))) {
            rplError(ERR_LISTEXPECTED);
            return;
        }

        WORDPTR nextname=rplPeekData(1)+1;
        WORDPTR endoflist=rplSkipOb(rplPeekData(1));

        WORDPTR *firstentry=rplFindFirstInDir(CurrentDir);
        WORDPTR *foundentry;

        while((*nextname!=CMD_ENDLIST)&&(nextname<endoflist))
        {
            foundentry=rplFindGlobal(nextname,0);
            if(foundentry) {
                WORDPTR name,val;
                // BRING THIS ENTRY TO THE PROPER LOCATION
                name=foundentry[0];
                val=foundentry[1];
                memmovew(firstentry+2,firstentry,(foundentry-firstentry)*(sizeof(void *)>>2));
                firstentry[0]=name;
                firstentry[1]=val;
                firstentry+=2;
            }

            nextname=rplSkipOb(nextname);
        }

        // ALL VARIABLES SORTED
        rplDropData(1);
        return;
    }




    case QUOTEID:
    {
    if(rplDepthData()<1) {
        rplError(ERR_BADARGCOUNT);
        return;
    }
    if(!ISIDENT(*rplPeekData(1))) {
        rplError(ERR_IDENTEXPECTED);
        return;
    }

    WORDPTR ident=rplMakeIdentQuoted(rplPeekData(1));

    if(!ident) return;  // SOME ERROR OCCURRED

    rplOverwriteData(1,ident);

    return;

    }
    case UNQUOTEID:
    {
    if(rplDepthData()<1) {
        rplError(ERR_BADARGCOUNT);
        return;
    }
    if(!ISIDENT(*rplPeekData(1))) {
        rplError(ERR_IDENTEXPECTED);
        return;
    }

    WORDPTR ident=rplMakeIdentUnquoted(rplPeekData(1));

    if(!ident) return;  // SOME ERROR OCCURRED

    rplOverwriteData(1,ident);

    return;

    }

    case HIDEVAR:
        {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }


        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryDoCmd();
            return;
        }


        // ONLY ACCEPT IDENTS AS KEYS

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;

        }

        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        WORDPTR *var=rplFindLAM(rplPeekData(1),1);
        if(!var) var=rplFindGlobal(rplPeekData(1),1);
        if(var) {
            WORDPTR ident=rplMakeIdentHidden(var[0]);
            if(!ident) return;
            if(ident!=var[0]) var[0]=ident;
            rplDropData(1);
            return;
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }



    case UNHIDEVAR:
    {
    if(rplDepthData()<1) {
        rplError(ERR_BADARGCOUNT);
        return;
    }
    if(ISLIST(*rplPeekData(1))) {
        rplListUnaryDoCmd();
        return;
    }



    // ONLY ACCEPT IDENTS AS KEYS

    if(!ISIDENT(*rplPeekData(1))) {
        rplError(ERR_IDENTEXPECTED);
        return;

    }

    // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

    WORDPTR *var=rplFindLAM(rplPeekData(1),1);
    if(!var) var=rplFindGlobal(rplPeekData(1),1);
    if(var) {
        WORDPTR ident=rplMakeIdentVisible(var[0]);
        if(!ident) return;
        if(ident!=var[0]) var[0]=ident;
        rplDropData(1);
        return;
    }
    else {
        rplError(ERR_UNDEFINEDVARIABLE);
        return;
    }

    }

    // ADD MORE OPCODES HERE


    case CLVAR:
    {
        // PURGE ALL VARIABLES AND EMPTY SUBDIRECTORIES IN CURRENT DIR

        WORDPTR *var;

        BINT idx=0;

        while((var=rplFindGlobalByIndexInDir(idx,CurrentDir)))
        {
            // PURGE THE VARIABLE UNLESS IT'S EITHER LOCKED OR
            // IT'S A NON-EMPTY DIRECTORY
            if(rplIsVarReadOnly(var) || (rplIsVarDirectory(var)&&!rplIsVarEmptyDir(var))) ++idx;
            else rplPurgeForced(var);
            if(Exceptions) return;
        }

        return;

    }

    case LOCKVAR:
    {
    if(rplDepthData()<1) {
        rplError(ERR_BADARGCOUNT);
        return;
    }

    if(ISLIST(*rplPeekData(1))) {
        rplListUnaryDoCmd();
        return;
    }

    // ONLY ACCEPT IDENTS AS KEYS

    if(!ISIDENT(*rplPeekData(1))) {
        rplError(ERR_IDENTEXPECTED);
        return;

    }

    // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

    WORDPTR *var=rplFindLAM(rplPeekData(1),1);
    if(!var) var=rplFindGlobal(rplPeekData(1),1);
    if(var) {
        WORDPTR ident=rplMakeIdentReadOnly(var[0]);
        if(!ident) return;
        if(ident!=var[0]) var[0]=ident;
        rplDropData(1);
        return;
    }
    else {
        rplError(ERR_UNDEFINEDVARIABLE);
        return;
    }

    }

case UNLOCKVAR:
    {
    if(rplDepthData()<1) {
        rplError(ERR_BADARGCOUNT);
        return;
    }


    if(ISLIST(*rplPeekData(1))) {
        rplListUnaryDoCmd();
        return;
    }

    // ONLY ACCEPT IDENTS AS KEYS

    if(!ISIDENT(*rplPeekData(1))) {
        rplError(ERR_IDENTEXPECTED);
        return;

    }

    // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

    WORDPTR *var=rplFindLAM(rplPeekData(1),1);
    if(!var) var=rplFindGlobal(rplPeekData(1),1);
    if(var) {
        WORDPTR ident=rplMakeIdentWriteable(var[0]);
        if(!ident) return;
        if(ident!=var[0]) var[0]=ident;
        rplDropData(1);
        return;
    }
    else {
        rplError(ERR_UNDEFINEDVARIABLE);
        return;
    }

    }








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


        // LSTO NEEDS SPECIAL CONSIDERATION TO CREATE LAMS AT COMPILE TIME

        if((TokenLen==3) && (!utf8ncmp((char *)TokenStart,"STO",3)))
        {

            // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

            // CHECK IF THE PREVIOUS OBJECT IS A QUOTED IDENT?
            WORDPTR object,prevobject;
            if(ValidateTop<=ValidateBottom) {
                // THERE'S NO ENVIRONMENT
                object=TempObEnd;   // START OF COMPILATION
            } else {
                object=*(ValidateTop-1);    // GET LATEST CONSTRUCT
                ++object;                   // AND SKIP THE PROLOG / ENTRY WORD
            }

            if(object<CompileEnd) {
            do {
                prevobject=object;
                object=rplSkipOb(object);
            } while(object<CompileEnd);

            // HERE PREVOBJECT CONTAINS THE LAST OBJECT THAT WAS COMPILED

            if(ISIDENT(*prevobject)) {
                // WE HAVE A HARD-CODED IDENT, CHECK IF IT EXISTS ALREADY

                // CHECK IF IT'S AN EXISTING LAM, COMPILE TO A PUTLAM OPCODE IF POSSIBLE

                WORDPTR *LAMptr=rplFindLAM(prevobject,1);


                if(LAMptr<LAMTopSaved) {
                    // THIS IS NOT A VALID LAM, LEAVE AS IDENT

                    rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,STO));

                    RetNum=OK_CONTINUE;
                    return;
                }

                if(LAMptr<nLAMBase) {
                    // THIS IS A LAM FROM AN UPPER CONSTRUCT
                    // WE CAN USE PUTLAM ONLY INSIDE LOOPS, NEVER ACROSS SECONDARIES

                    WORDPTR *env=nLAMBase;
                    WORD prolog;
                    do {
                        if(LAMptr>env) break;
                        prolog=**(env+1);   // GET THE PROLOG OF THE SECONDARY
                        if(ISPROLOG(prolog) && LIBNUM(prolog)==SECO) {
                        // LAMS ACROSS << >> SECONDARIES HAVE TO BE COMPILED AS IDENTS
                        rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,STO));

                        RetNum=OK_CONTINUE;
                        return;
                        }
                        env=rplGetNextLAMEnv(env);
                    } while(env);



                }


                // SPECIAL CASE: WHEN A SECO DOESN'T HAVE ANY LOCALS YET
                // BUT LAMS FROM THE PREVIOUS SECO SHOULDN'T BE COMPILED TO GETLAMS

                // SCAN ALL CURRENT CONSTRUCTS TO FIND THE INNERMOST SECONDARY
                // THEN VERIFY IF THAT SECONDARY IS THE CURRENT LAM ENVIRONMENT

                // THIS IS TO FORCE ALL LAMS IN A SECO TO BE COMPILED AS IDENTS
                // INSTEAD OF PUTLAMS

                // LAMS ACROSS DOCOL'S ARE OK AND ALWAYS COMPILED AS PUTLAMS WHEN USING STO, CREATE NEW ONE WHEN USING LSTO
                WORDPTR *scanenv=ValidateTop-1;

                while(scanenv>=ValidateBottom) {
                    if( ((LIBNUM(**scanenv)==SECO))&& (ISPROLOG(**scanenv))) {
                            // FOUND INNERMOST SECONDARY
                            if(*scanenv>*(nLAMBase+1)) {
                                // THE CURRENT LAM BASE IS OUTSIDE THE INNER SECONDARY
                            rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,STO));
                            RetNum=OK_CONTINUE;
                            return;
                            }
                            break;

                    }
                    --scanenv;
                }


                // IT'S A KNOWN LOCAL VARIABLE, COMPILE AS PUTLAM
                BINT Offset=((BINT)(LAMptr-nLAMBase))>>1;

                // ONLY USE PUTLAM IF OFFSET IS WITHIN RANGE
                if(Offset<=32767 && Offset>=-32768) {
                CompileEnd=prevobject;
                rplCompileAppend(MKOPCODE(DOIDENT,PUTLAMN+(Offset&0xffff)));
                }
                else {
                    rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,STO));
                }

                RetNum=OK_CONTINUE;
                return;
            }

            }


            rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,STO));
            RetNum=OK_CONTINUE;
            return;
        }


        if((TokenLen==3) && (!utf8ncmp((char *)TokenStart,"RCL",3)))
        {

            // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

            // CHECK IF THE PREVIOUS OBJECT IS A QUOTED IDENT?
            WORDPTR object,prevobject;
            if(ValidateTop<=ValidateBottom) {
                // THERE'S NO ENVIRONMENT
                object=TempObEnd;   // START OF COMPILATION
            } else {
                object=*(ValidateTop-1);    // GET LATEST CONSTRUCT
                ++object;                   // AND SKIP THE PROLOG / ENTRY WORD
            }

            if(object<CompileEnd) {
            do {
                prevobject=object;
                object=rplSkipOb(object);
            } while(object<CompileEnd);

            // HERE PREVOBJECT CONTAINS THE LAST OBJECT THAT WAS COMPILED

            if(ISIDENT(*prevobject)) {
                // WE HAVE A HARD-CODED IDENT, CHECK IF IT EXISTS ALREADY

                // CHECK IF IT'S AN EXISTING LAM, COMPILE TO A GETLAM OPCODE IF POSSIBLE

                WORDPTR *LAMptr=rplFindLAM(prevobject,1);


                if(LAMptr<LAMTopSaved) {
                    // THIS IS NOT A VALID LAM, LEAVE AS IDENT

                    rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,RCL));
                    RetNum=OK_CONTINUE;
                    return;
                }

                if(LAMptr<nLAMBase) {
                    // THIS IS A LAM FROM AN UPPER CONSTRUCT
                    // WE CAN USE GETLAM ONLY INSIDE LOOPS, NEVER ACROSS SECONDARIES

                    WORDPTR *env=nLAMBase;
                    WORD prolog;
                    do {
                        if(LAMptr>env) break;
                        prolog=**(env+1);   // GET THE PROLOG OF THE SECONDARY
                        if(ISPROLOG(prolog) && LIBNUM(prolog)==SECO) {
                        // LAMS ACROSS << >> SECONDARIES HAVE TO BE COMPILED AS IDENTS
                        rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,RCL));

                        RetNum=OK_CONTINUE;
                        return;
                        }
                        env=rplGetNextLAMEnv(env);
                    } while(env);



                }


                // SPECIAL CASE: WHEN A SECO DOESN'T HAVE ANY LOCALS YET
                // BUT LAMS FROM THE PREVIOUS SECO SHOULDN'T BE COMPILED TO GETLAMS

                // SCAN ALL CURRENT CONSTRUCTS TO FIND THE INNERMOST SECONDARY
                // THEN VERIFY IF THAT SECONDARY IS THE CURRENT LAM ENVIRONMENT

                // THIS IS TO FORCE ALL LAMS IN A SECO TO BE COMPILED AS IDENTS
                // INSTEAD OF GETLAMS

                // LAMS ACROSS DOCOL'S ARE OK AND ALWAYS COMPILED AS GETLAMS
                WORDPTR *scanenv=ValidateTop-1;

                while(scanenv>=ValidateBottom) {
                    if( (LIBNUM(**scanenv)==SECO)&& (ISPROLOG(**scanenv))) {
                            // FOUND INNERMOST SECONDARY
                            if(*scanenv>*(nLAMBase+1)) {
                                // THE CURRENT LAM BASE IS OUTSIDE THE INNER SECONDARY
                            rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,RCL));
                            RetNum=OK_CONTINUE;
                            return;
                            }
                            break;

                    }
                    --scanenv;
                }

                // IT'S A KNOWN LOCAL VARIABLE, COMPILE AS GETLAM
                BINT Offset=((BINT)(LAMptr-nLAMBase))>>1;

                if(Offset<=32767 && Offset>=-32768) {
                CompileEnd=prevobject;
                rplCompileAppend(MKOPCODE(DOIDENT,GETLAMN+(Offset&0xffff)));
                }
                else {
                    rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,RCL));
                }
                RetNum=OK_CONTINUE;
                return;
            }


            }


            rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,RCL));
            RetNum=OK_CONTINUE;
            return;
        }













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
            RetNum=OK_CONTINUE;
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
        // THIS OPCODE RECEIVES A POINTER TO AN RPL COMMAND OR OBJECT IN ObjectPTR
        // NEEDS TO RETURN INFORMATION ABOUT THE TYPE:
        // IN RetNum: RETURN THE MKTOKENINFO() DATA FOR THE SYMBOLIC COMPILER AND CAS
        // IN DecompHints: RETURN SOME HINTS FOR THE DECOMPILER TO DO CODE BEAUTIFICATION (TO BE DETERMINED)
        // IN TypeInfo: RETURN TYPE INFORMATION FOR THE TYPE COMMAND
        //             TypeInfo: TTTTFF WHERE TTTT = MAIN TYPE * 100 (NORMALLY THE MAIN LIBRARY NUMBER)
        //                                FF = 2 DECIMAL DIGITS FOR THE SUBTYPE OR FLAGS (VARIES DEPENDING ON LIBRARY)
        //             THE TYPE COMMAND WILL RETURN A REAL NUMBER TypeInfo/100
        // FOR NUMBERS: TYPE=10 (REALS), SUBTYPES = .01 = APPROX., .02 = INTEGER, .03 = APPROX. INTEGER
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL BINT, .42 = HEX INTEGER
        if(ISPROLOG(*ObjectPTR)) {
        TypeInfo=LIBRARY_NUMBER*100;
        DecompHints=0;
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_NOTALLOWED,0,1);
        }
        else {
            TypeInfo=0;     // ALL COMMANDS ARE TYPE 0
            DecompHints=0;
            libGetInfo2(*ObjectPTR,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        }
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

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(MENUNUMBER(MenuCodeArg)>0) {
            RetNum=ERR_NOTMINE;
            return;
        }
        ObjectPTR=(WORDPTR)lib28_menu;
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




