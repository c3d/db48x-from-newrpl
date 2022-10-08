/*
 * Copyright (c) 2019, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "libraries.h"

#ifndef COMMANDS_ONLY_PASS
#include "cmdcodes.h"
#include "hal_api.h"
#include "newrpl.h"
#include "sysvars.h"
#endif

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************

// REPLACE THE NUMBER
#define LIBRARY_NUMBER  55

//@TITLE=Constants

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

// SPECIAL CASE: LOWEST BIT OF CONSTANTS INDICATE IF CONSTANT IS COMPLEX OR REAL
// MAKE SURE ALL COMPLEX CONSTANTS ARE IN ODD NUMBERS IN THE LIST

#define COMMAND_LIST \
    ECMD(PICONST,"π",MK_TOKEN_INFO(1,TITYPE_REAL,0,1)), \
    ECMD(ICONST,"і",MK_TOKEN_INFO(1,TITYPE_COMPLEX,0,1)), \
    ECMD(ECONST,"е",MK_TOKEN_INFO(1,TITYPE_REAL,0,1)), \
    ECMD(JCONST,"ј",MK_TOKEN_INFO(1,TITYPE_COMPLEX,0,1))

// ADD MORE OPCODES HERE

#define ERROR_LIST \
ERR(CONSTANTEXPECTED,0), \
ERR(UNDEFINEDCONSTANT,1)

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS    LIBRARY_NUMBER

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"

#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib55_menu);

INCLUDE_ROMOBJECT(lib55_pi);
INCLUDE_ROMOBJECT(lib55_i);
INCLUDE_ROMOBJECT(lib55_e);
INCLUDE_ROMOBJECT(lib55_j);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const word_p const ROMPTR_TABLE[] = {
    (word_p) LIB_HELPTABLE,
    (word_p) LIB_MSGTABLE,
    (word_p) lib55_menu,
    // HERE ADD THE VALUES OF THE CONSTANTS AS RPL OBJECTS. ALL CONSTANTS NEED TO HAVE THEIR NUMERIC OBJECTS IN ROM
    (word_p) lib55_pi,
    (word_p) lib55_i,
    (word_p) lib55_e,
    (word_p) lib55_j,

    0
};

// CONVERT A CONSTANT TO A NUMBER
// RETURNS EITHER A NEW OBJECT WITH THE NUMERIC REPRESENTATION OF THE CONSTANT
// OR THE SAME OBJECT AS BEFORE.
// RETURNS OBJECTS IN ROM, VERY FAST, NEVER TRIGGERS GC OR ALLOCATES ANY MEMORY
// DRAWBACK: IT'S ALWAYS AT MAXIMUM PRECISION (2000 DIGITS)
word_p rplConstant2Number(word_p object)
{
    if(!ISCONSTANT(*object))
        return object;
    return ROMPTR_TABLE[OPCODE(object[1]) + 3]; // GET THE OPCODE FOR THE SYMBOL
}

void LIB_HANDLER()
{
    if(IS_PROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        // NORMAL BEHAVIOR  ON A IDENT IS TO PUSH THE OBJECT ON THE STACK:
        rplPushData(IPtr);

        return;
    }

    // PROCESS OVERLOADED OPERATORS FIRST
    if(LIBNUM(CurOpcode) == LIB_OVERLOADABLE) {

        if(ISUNARYOP(CurOpcode)) {
            // APPLY UNARY OPERATOR DIRECTLY TO THE CONTENTS OF THE VARIABLE
            switch (OPCODE(CurOpcode)) {
            case OVR_EVAL1:
                // LEAVE IT ON THE STACK
                return;

            case OVR_FUNCEVAL:
            {
                rplError(ERR_INVALIDUSERDEFINEDFUNCTION);
                return;
            }
            case OVR_EVAL:
                // LEAVE IT ON THE STACK
                return;
            case OVR_NUM:
            {
                rplOverwriteData(1, rplConstant2Number(rplPeekData(1)));
                return;
            }

            case OVR_ISTRUE:
                // CONSTANTS FALSE WHEN ZERO ONLY
                rplOverwriteData(1, rplConstant2Number(rplPeekData(1)));
                rplCallOperator(CurOpcode);
                return;

            case OVR_XEQ:
                // JUST KEEP THE CONSTANT ON THE STACK, UNEVALUATED
                return;

            default:
                // PASS AL OTHER OPERATORS DIRECTLY AS A SYMBOLIC OBJECT
            {
                LIBHANDLER symblib = rplGetLibHandler(DOSYMB);
                (*symblib) ();
                return;
            }
            }

        }       // END OF UNARY OPERATORS

        else if(ISBINARYOP(CurOpcode)) {

            switch (OPCODE(CurOpcode)) {
            case OVR_SAME:
            {
                int32_t same = rplCompareObjects(rplPeekData(1), rplPeekData(2));
                rplDropData(2);
                if(same)
                    rplPushTrue();
                else
                    rplPushFalse();
                return;
            }
            default:
            {
                // PASS AL OTHER OPERATORS DIRECTLY AS A SYMBOLIC OBJECT
                LIBHANDLER symblib = rplGetLibHandler(DOSYMB);
                (*symblib) ();
                return;
            }
            }
        }

    }

    switch (OPCODE(CurOpcode)) {

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
        if(RetNum == OK_CONTINUE) {
            // ENCAPSULATE THE OPCODE INSIDE AN OBJECT
            rplCompileAppend(MK_OPCODE(DECBINT, OPCODE(*(CompileEnd - 1))));
            if(CompileEnd[-2] & 1) {
                // THIS IS A COMPLEX CONSTANT, JUST REPEAT THE OPCODE
                rplCompileAppend(*(CompileEnd - 1));
                CompileEnd[-3] = MK_PROLOG(LIBRARY_NUMBER, 2);
            }
            else
                CompileEnd[-2] = MK_PROLOG(LIBRARY_NUMBER, 1);
        }
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
        if(IS_PROLOG(*DecompileObject)) {
            ++DecompileObject;
            libDecompileCmds((char **)LIB_NAMES, NULL, LIB_NUMBEROFCMDS);
            --DecompileObject;
        }
        else
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
        // RetNum =  OK_TOKENINFO | MK_TOKEN_INFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND
    {
        libProbeCmds((char **)LIB_NAMES, (int32_t *) LIB_TOKENINFO,
                LIB_NUMBEROFCMDS);

        return;
    }

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
        // FOR CONSTANTS: TYPE=55 (CONSTANT), SUBTYPE = 0.10 = REAL, 0.11 = NEGATIVE REAL, 0.12 = INFINITE REAL, 0.20 = COMPLEX, 0.22 = INF COMPLEX, 0.30 = MATRIX, 0.40 = UNIT (ASSUMED REAL VALUE)
        if(IS_PROLOG(*ObjectPTR)) {
            TypeInfo = LIBRARY_NUMBER * 100;
            DecompHints = 0;
            libGetInfo2(ObjectPTR[1], (char **)LIB_NAMES,
                    (int32_t *) LIB_TOKENINFO, LIB_NUMBEROFCMDS);
            if(TI_TYPE(RetNum) == TITYPE_REAL)
                TypeInfo += 10;
            else if(TI_TYPE(RetNum) == TITYPE_REALNEG)
                TypeInfo += 11;
            else if(TI_TYPE(RetNum) == TITYPE_REALINF)
                TypeInfo += 12;
            else if(TI_TYPE(RetNum) == TITYPE_COMPLEX)
                TypeInfo += 20;
            else if(TI_TYPE(RetNum) == TITYPE_COMPLEXINF)
                TypeInfo += 22;
            else if(TI_TYPE(RetNum) == TITYPE_UNIT)
                TypeInfo += 40;
            else if(TI_TYPE(RetNum) == TITYPE_MATRIX)
                TypeInfo += 30;
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
        if(IS_PROLOG(*ObjectPTR)) {

            if(OBJSIZE(*ObjectPTR) != 1) {
                RetNum = ERR_INVALID;   // WRONG SIZE
                return;
            }

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
        if(MENU_NUMBER(MenuCodeArg) > 0) {
            RetNum = ERR_NOTMINE;
            return;
        }
        ObjectPTR = (word_p) lib55_menu;
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
    rplError(ERR_INVALIDOPCODE);

    return;

}

#endif
