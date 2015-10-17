/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// THIS LIBRARY PROVIDES COMPILATION OF STACK RELATED COMMANDS AND OTHER BASIC COMMANDS


// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  72
#define LIB_ENUM lib72enum
#define LIB_NAMES lib72_names
#define LIB_HANDLER lib72_handler
#define LIB_NUMBEROFCMDS LIB72_NUMBEROFCMDS
#define ROMPTR_TABLE    romptr_table72

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };


// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(GARBAGE), \
    CMD(CLEAR), \
    CMD(DEPTH), \
    CMD(DROP), \
    CMD(DROP2), \
    CMD(DROPN), \
    CMD(DUP), \
    CMD(DUP2), \
    CMD(DUPDUP), \
    CMD(DUPN), \
    CMD(NDUPN), \
    CMD(NIP), \
    CMD(OVER), \
    CMD(PICK), \
    CMD(PICK3), \
    CMD(ROLL), \
    CMD(ROLLD), \
    CMD(ROT), \
    CMD(SWAP), \
    CMD(UNPICK), \
    CMD(UNROT)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY

#define CMD_EXTRANAME \
    ""
#define CMD_EXTRAENUM \
    UNPROTECTSTACK



// INTERNAL DECLARATIONS

// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST , CMD_EXTRAENUM , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
const char * const LIB_NAMES[]= { CMD_LIST , CMD_EXTRANAME  };
#undef CMD


ROMOBJECT unprotect_seco[]={
    MKPROLOG(DOCOL,2),
    MKOPCODE(LIBRARY_NUMBER,UNPROTECTSTACK),
    CMD_SEMI
};

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
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

    switch(OPCODE(CurOpcode))
    {
    case GARBAGE:
        rplGCollect();
        return;
    case CLEAR:
        // ONLY CLEAR UP TO THE STACK PROTECTED AREA
        // DON'T THROW AN ERROR
        DSTop=DStkProtect;
        return;
    case DEPTH:
        rplNewBINTPush(rplDepthData(),DECBINT);
        return;
    case DROP:
        rplDropData(1);
        return;
    case DROP2:
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
    case OPCODE_AUTOCOMPPREV:
        libAutoCompletePrev(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
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








