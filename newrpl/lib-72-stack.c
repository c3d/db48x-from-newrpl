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
#define LIBRARY_NUMBER  72


#define ERROR_LIST \
    ERR(BADSTACKINDEX,0)


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    ECMD(UNPROTECTSTACK,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    CMD(CLEAR,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(DEPTH,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(DROP,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(DROP2,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(DROPN,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(DUP,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(DUP2,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(DUPDUP,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(DUPN,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(NDUPN,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(NIP,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(OVER,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(PICK,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(PICK3,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(ROLL,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(ROLLD,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(ROT,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(SWAP,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(UNPICK,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(UNROT,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);

INCLUDE_ROMOBJECT(lib72menu_main);

ROMOBJECT unprotect_seco[]={
    MKPROLOG(DOCOL,2),
    MKOPCODE(LIBRARY_NUMBER,UNPROTECTSTACK),
    CMD_SEMI
};

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib72menu_main,
    (WORDPTR)unprotect_seco,
    0
};



void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    // LIBRARIES THAT DEFINE ONLY COMMANDS STILL HAVE TO RESPOND TO A FEW OVERLOADABLE OPERATORS
    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE) {
        // ONLY RESPOND TO EVAL, EVAL1 AND XEQ FOR THE COMMANDS DEFINED HERE
        // IN CASE OF COMMANDS TREATED AS OBJECTS (WHEN EMBEDDED IN LISTS)
        if( (OPCODE(CurOpcode)==OVR_EVAL)||
                (OPCODE(CurOpcode)==OVR_EVAL1)||
                (OPCODE(CurOpcode)==OVR_XEQ) )
        {
            // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORD saveOpcode=CurOpcode;
            CurOpcode=*rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode=saveOpcode;
            return;
        }
    }

    switch(OPCODE(CurOpcode))
    {
    case CLEAR:
        // ONLY CLEAR UP TO THE STACK PROTECTED AREA
        // DON'T THROW AN ERROR
        DSTop=DStkProtect;
        return;
    case DEPTH:
        rplNewBINTPush(rplDepthData(),DECBINT);
        return;
    case DROP:
        if(rplDepthData()<1) { rplError(ERR_BADARGCOUNT); return; }
        rplDropData(1);
        return;
    case DROP2:
        if(rplDepthData()<2) { rplError(ERR_BADARGCOUNT); return; }
        rplDropData(2);
        return;
    case DROPN:
    {
        if(rplDepthData()<1) {
        rplError(ERR_BADARGCOUNT);
         return;
        }
        BINT64 count=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;
        if(count<0 || rplDepthData()<count+1) {
            rplError(ERR_BADSTACKINDEX);
            return;
        }
        rplDropData(count+1);
        return;
    }
    case DUP:
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
         return;
        }
        rplPushData(rplPeekData(1));
        return;
    case DUP2:
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

         return;
        }
        rplPushData(rplPeekData(2));
        rplPushData(rplPeekData(2));
        return;
    case DUPDUP:
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

         return;
        }
        rplPushData(rplPeekData(1));
        rplPushData(rplPeekData(1));
        return;
    case DUPN:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

         return;
        }
        BINT64 count=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;
        if(count<0 || rplDepthData()<count+1) {
            rplError(ERR_BADSTACKINDEX);

            return;
        }
        rplDropData(1);
        BINT64 f;
        for(f=0;f<count;++f) {
            rplPushData(rplPeekData(count));
            if(Exceptions) return;
        }
        return;
    }
    case NDUPN:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

         return;
        }
        BINT64 count=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;
        if(count<0) {
            rplError(ERR_BADSTACKINDEX);

            return;
        }
        rplDropData(1);
        BINT64 f;
        for(f=1;f<count;++f) {
            rplPushData(rplPeekData(1));
            if(Exceptions) return;
        }
        rplNewBINTPush(count,DECBINT);
        return;
    }
    case NIP:
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

         return;
        }
        rplOverwriteData(2,rplPeekData(1));
        rplDropData(1);
        return;

    case OVER:
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

         return;
        }
        rplPushData(rplPeekData(2));
        return;
    case PICK:
    {

        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

         return;
        }

        BINT64 level=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;
        if( (level<1) || (rplDepthData()<1+level)) {
            rplError(ERR_BADSTACKINDEX);

         return;
        }

        rplOverwriteData(1,rplPeekData(1+level));

        return;
    }
    case PICK3:
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);

         return;
        }
        rplPushData(rplPeekData(3));
        return;
    case ROLL:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

         return;
        }

        BINT64 level=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;

        if( (level<1) || (rplDepthData()<1+level)) {
            rplError(ERR_BADSTACKINDEX);

         return;
        }

        rplDropData(1);

        WORDPTR objn=rplPeekData(level);
        WORDPTR *stkptr=DSTop-level;

        BINT64 count;
        for(count=1;count<level;++count,++stkptr) *stkptr=*(stkptr+1);

        rplOverwriteData(1,objn);

        return;
     }
    case ROLLD:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

         return;
        }

        BINT64 level=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;

        if( (level<1) || (rplDepthData()<1+level)) {
            rplError(ERR_BADSTACKINDEX);

         return;
        }

        rplDropData(1);

        WORDPTR objn=rplPeekData(1);
        WORDPTR *stkptr=DSTop-1;

        BINT64 count;
        for(count=1;count<level;++count,--stkptr) *stkptr=*(stkptr-1);

        rplOverwriteData(level,objn);

        return;
     }
    case ROT:
    {
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);

         return;
        }
        WORDPTR obj1=rplPeekData(1);
        rplOverwriteData(1,rplPeekData(3));
        rplOverwriteData(3,rplPeekData(2));
        rplOverwriteData(2,obj1);
        return;
    }

    case SWAP:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

         return;
        }
        WORDPTR obj=rplPeekData(1);
        rplOverwriteData(1,rplPeekData(2));
        rplOverwriteData(2,obj);
        return;
    }

    case UNPICK:
    {

        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

         return;
        }

        BINT64 level=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;

        if( (level<1) || (rplDepthData()<2+level)) {
            rplError(ERR_BADSTACKINDEX);

         return;
        }

        WORDPTR obj=rplPeekData(2);
        rplDropData(2);

        rplOverwriteData(level,obj);

        return;
    }

    case UNROT:
    {

        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);

         return;
        }
        WORDPTR obj1=rplPeekData(1);
        rplOverwriteData(1,rplPeekData(2));
        rplOverwriteData(2,rplPeekData(3));
        rplOverwriteData(3,obj1);
        return;
    }


    case UNPROTECTSTACK:
    {
        // THIS INTERNAL OPCODE PROVIDES SAFETY GUARD AGAINST DATA STACK PROTECTION
        // IF A PROGRAM FORGETS TO UNPROTECT THE STACK, IT WILL BE UNPROTECTED
        // AUTOMATICALLY ON EXIT
        BINT protlevel=(BINT) ((WORDPTR *)rplPopRet()-DStk);
        if((DStkBottom+protlevel>=DStkBottom) && (DStkBottom+protlevel<DSTop) )  DStkProtect=DStkBottom+protlevel;
        else DStkProtect=DStkBottom;
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
    {
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
