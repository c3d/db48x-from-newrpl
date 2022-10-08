/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "libraries.h"

#ifndef COMMANDS_ONLY_PASS
#include "cmdcodes.h"
#include "newrpl.h"
#include "sysvars.h"
#endif

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************

// REPLACE THE NUMBER
#define LIBRARY_NUMBER  4080

//@TITLE=Local Variables (merge?)

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    ECMD(NEWLOCALENV,"→",MK_TOKEN_INFO(1,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"

#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

#define NEWNLOCALS 0x40000      // SPECIAL OPCODE TO CREATE NEW LOCAL VARIABLES

void LIB_HANDLER()
{
    if(IS_PROLOG(CurOpcode)) {
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    switch (OPCODE(CurOpcode)) {

        // STANDARIZED OPCODES:
        // --------------------
        // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES

        /*
           case NEWLOCALENV:
           //@SHORT_DESC=@HIDE
         */

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
        // BUT ONLY IF WE ARE WITHIN A NEWLOCALENV CONSTRUCT

        if((TokenLen == 1)
                && (!utf8ncmp2((char *)TokenStart, (char *)BlankStart, "«",
                        1))) {
            if((CurrentConstruct & ~0xffff) != MK_OPCODE(LIBRARY_NUMBER,
                        NEWLOCALENV)) {
                RetNum = ERR_NOTMINE;
                return;
            }

            // COUNT HOW MANY LAMS ARE IN THE CONSTRUCT
            ScratchPointer1 = *(ValidateTop - 1);
            int32_t lamcount = 0;

            // INITIALIZE AN ENVIRONMENT FOR COMPILE TIME
            rplCreateLAMEnvironment(ScratchPointer1);
            ++ScratchPointer1;  // SKIP THE START OF CONSTRUCT WORD
            while(ScratchPointer1 < CompileEnd) {
                rplCreateLAM(ScratchPointer1, ScratchPointer1); // CREATE ALL THE LAMS FOR FUTURE COMPILATION
                ScratchPointer1 = rplSkipOb(ScratchPointer1);
                ++lamcount;
            }

            if(!lamcount) {
                RetNum = ERR_SYNTAX;
                return;
            }
            // NOW REPLACE THE -> WORD FOR A STANDARD <<

            ScratchPointer1 = *(ValidateTop - 1);
            *ScratchPointer1 = MK_PROLOG(SECO, 0);       // STANDARD SECONDARY PROLOG SO ALL LAMS ARE CREATED INSIDE OF IT
            CurrentConstruct = MK_PROLOG(SECO, 0);
            rplCompileAppend((WORD) MK_OPCODE(DOIDENT, NEWNLOCALS + lamcount));  // OPCODE TO CREATE ALL THESE LAMS
            RetNum = OK_CONTINUE;
            return;
        }

        // CHECK IF THE TOKEN IS THE NEW LOCAL

        if((TokenLen == 1)
                && (!utf8ncmp2((char *)TokenStart, (char *)BlankStart, "→",
                        1))) {
            rplCompileAppend(CMD_XEQSECO);      // EVAL THE NEXT SECO IN THE RUNSTREAM
            rplCompileAppend(MK_OPCODE(LIBRARY_NUMBER, NEWLOCALENV));    // PUT A MARKER
            RetNum = OK_STARTCONSTRUCT;
            return;
        }

        RetNum = ERR_NOTMINE;
        return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        // THIS LIBRARY DOES NOT GENERATE ANY OPCODES!
        RetNum = ERR_INVALID;
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

        if(IS_PROLOG(*LastCompiledObject)) {
            if(ISIDENT(*LastCompiledObject)) {
                RetNum = OK_INCARGCOUNT;
                return;
            }
            if(ISSYMBOLIC(*LastCompiledObject)) {
                int32_t lamcount = CurrentConstruct & 0xffff;
                if(!lamcount) {
                    RetNum = ERR_INVALID;
                    return;
                }

                // NOW REPLACE THE -> WORD FOR A STANDARD <<

                **(ValidateTop - 1) = MK_PROLOG(SECO, 0);        // STANDARD SECONDARY PROLOG SO ALL LAMS ARE CREATED INSIDE OF IT
                CurrentConstruct = MK_PROLOG(SECO, 0);

                rplCompileInsert(LastCompiledObject, MK_OPCODE(DOIDENT,
                            NEWNLOCALS + lamcount));
                rplCompileAppend(CMD_OVR_EVAL1);
                rplCompileAppend(CMD_QSEMI);
                RetNum = OK_ENDCONSTRUCT;
                return;
            }
        }
        else {
            // NOT AN OBJECT, THERE'S ONLY A COUPLE OF ACCEPTED COMMANDS HERE
            if((LIBNUM(*LastCompiledObject) == DOIDENT)
                    && (((*LastCompiledObject) & 0x70000) == NEWNLOCALS)) {
                RetNum = OK_CONTINUE;
                return;
            }

        }
        RetNum = ERR_INVALID;
        return;

    case OPCODE_GETINFO:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL COMMAND OR OBJECT IN ObjectPTR
        // NEEDS TO RETURN INFORMATION ABOUT THE TYPE:
        // IN RetNum: RETURN THE MK_TOKEN_INFO() DATA FOR THE SYMBOLIC COMPILER AND CAS
        // IN DecompHints: RETURN SOME HINTS FOR THE DECOMPILER TO DO CODE BEAUTIFICATION (TO BE DETERMINED)
        // IN TypeInfo: RETURN TYPE INFORMATION FOR THE TYPE COMMAND
        //             TypeInfo: TTTTFF WHERE TTTT = MAIN TYPE * 100 (NORMALLY THE MAIN LIBRARY NUMBER)
        //                                FF = 2 DECIMAL DIGITS FOR THE SUBTYPE OR FLAGS (VARIES DEPENDING ON LIBRARY)
        //             THE TYPE COMMAND WILL RETURN A REAL NUMBER TypeInfo/100
        // FOR NUMBERS: TYPE=10 (REALS), SUBTYPES = .01 = APPROX., .02 = INTEGER, .03 = APPROX. INTEGER
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL int32_t, .42 = HEX INTEGER
        if(IS_PROLOG(*ObjectPTR)) {
            TypeInfo = LIBRARY_NUMBER * 100;
            DecompHints = 0;
            RetNum = OK_TOKENINFO | MK_TOKEN_INFO(0, TITYPE_NOTALLOWED, 0, 1);
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

        RetNum = ERR_NOTMINE;
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID, ObjectIDHash=hash
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        RetNum = ERR_NOTMINE;
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID

        if(IS_PROLOG(*ObjectPTR)) {
            RetNum = ERR_INVALID;
            return;
        }
        RetNum = OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER, (char **)LIB_NAMES,
                LIB_NUMBEROFCMDS);
        return;

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
    rplError(ERR_INVALIDOPCODE);

    return;

}

#endif
