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
#define LIBRARY_NUMBER  8


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define CMD_LIST \
    CMD(EXITRPL,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(BKPOINT,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(XEQSECO,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    ECMD(SEMI,";",MKTOKENINFO(1,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifdef COMMANDS_ONLY_PASS

// CLEANUP FOR OTHER LIBRARIES TO DEFINE THEIR OWN

#undef LIBRARY_NUMBER
#undef LIB_ENUM
#undef LIB_CMDS
#undef LIB_NAMES
#undef LIB_HANDLER
#undef LIB_NUMBEROFCMDS
#undef ROMPTR_TABLE



#else

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE

        rplPushRet(IPtr);       // PUSH CURRENT POINTER AS THE RETURN ADDRESS. AT THIS POINT, IPtr IS POINTING TO THIS SECONDARY WORD
                                // BUT THE MAIN LOOP WILL ALWAYS SKIP TO THE NEXT OBJECT AFTER A SEMI.
        CurOpcode=MKPROLOG(LIBRARY_NUMBER,0); // ALTER THE SIZE OF THE SECONDARY TO ZERO WORDS, SO THE NEXT EXECUTED INSTRUCTION WILL BE THE FIRST IN THIS SECONDARY
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case EXITRPL:
        rplException(EX_EXITRPL);
        return;
    case BREAKPOINT:
        rplException(EX_BKPOINT);
        return;
    case XEQSECO:
        // IF THE NEXT OBJECT IN THE SECONDARY
        // IS A SECONDARY, IT EVALUATES IT INSTEAD OF PUSHING IT ON THE STACK
        ++IPtr;
        CurOpcode=*IPtr;
        if(ISPROLOG(CurOpcode)&& (LIBNUM(CurOpcode)==SECO)) {
            CurOpcode=MKPROLOG(SECO,0); // MAKE IT SKIP INSIDE THE SECONDARY
            rplPushRet(IPtr);
            return;
        }
        // HAVE NO EFFECT ON ANY OTHER OBJECTS
        --IPtr;
        CurOpcode=*IPtr;
        return;

    case SEMI:
        // POP THE RETURN ADDRESS
        IPtr=rplPopRet();   // GET THE CALLER ADDRESS
        CurOpcode=*IPtr;    // SET THE WORD SO MAIN LOOP SKIPS THIS OBJECT, AND THE NEXT ONE IS EXECUTED
        return;


    // ADD MORE OPCODES HERE
    case OVR_EVAL:
    case OVR_EVAL1:
        // EXECUTE THE OBJECT
    case OVR_XEQ:
        // ALSO EXECUTE THE OBJECT
        if(!ISPROLOG(*rplPeekData(1))) return;
        rplPushRet(IPtr);       // PUSH CURRENT POINTER AS THE RETURN ADDRESS.
        IPtr=rplPopData();      // SET NEW IPTR TO THE PROGRAM, TO BE EXECUTED
        CurOpcode=MKPROLOG(LIBRARY_NUMBER,0); // ALTER THE SIZE OF THE SECONDARY TO ZERO WORDS, SO THE NEXT EXECUTED INSTRUCTION WILL BE THE FIRST IN THIS SECONDARY
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

        // CHECK IF THE TOKEN IS THE OBJECT DOCOL

       if((TokenLen==2) && (!utf8ncmp((char * )TokenStart,"::",2)))
       {
           rplCompileAppend((WORD) MKPROLOG(LIBRARY_NUMBER,0));
           RetNum=OK_STARTCONSTRUCT;
           return;
       }
       // CHECK IF THE TOKEN IS SEMI

       if(((TokenLen==1) && (!utf8ncmp((char *)TokenStart,";",1))))
       {
           if(CurrentConstruct!=MKPROLOG(LIBRARY_NUMBER,0)) {
               RetNum=ERR_SYNTAX;
               return;
           }
           rplCleanupLAMs(*(ValidateTop-1));
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,SEMI));
           RetNum=OK_ENDCONSTRUCT;
           return;
       }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

        // SINCE THIS IS THE LAST LIBRARY TO BE EVALUATED, DO ONE LAST PASS TO COMPILE IT AS AN IDENT
        // EITHER LAM OR IN USEROB
     return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        if(ISPROLOG(*DecompileObject)) {
            rplDecompAppendString((BYTEPTR)"::");
            RetNum=OK_STARTCONSTRUCT;
            return;
        }

        // CHECK IF THE TOKEN IS SEMI

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,SEMI))
        {
            if(! (ISPROLOG(CurrentConstruct)&&(LIBNUM(CurrentConstruct)==LIBRARY_NUMBER))) {
                RetNum=ERR_SYNTAX;
                return;
            }
            rplCleanupLAMs(*(ValidateTop-1));
            rplDecompAppendString((BYTEPTR)";");
            RetNum=OK_ENDCONSTRUCT;
            return;
        }

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,XEQSECO)) {
            rplDecompAppendString((BYTEPTR)"→");
            RetNum=OK_STARTCONSTRUCT;
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

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        RetNum=ERR_NOTMINE;
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        RetNum=ERR_NOTMINE;
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) {
            // BASIC CHECKS
        WORDPTR ptr,objend;

        objend=rplSkipOb(ObjectPTR);
        ptr=ObjectPTR+1;

        while((*ptr!=CMD_SEMI)&&(ptr<objend)) {

            // TODO: RECURSIVELY CHECK OBJECT VALIDITY IN HERE

            ptr=rplSkipOb(ptr);
        }

        if(ptr!=objend-1) { RetNum=ERR_INVALID; return; }

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
