/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// LIBRARY 20 DEFINES THE COMMENTS OBJECTS
// AND ASSOCIATED FUNCTIONS

#include "libraries.h"

#ifndef COMMANDS_ONLY_PASS
#include "cmdcodes.h"
#include "newrpl.h"
#include "sysvars.h"
#include "utf8lib.h"
#endif

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************

// REPLACE THE NUMBER
#define LIBRARY_NUMBER  4079

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"

#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE

        // DO ABSOLUTELY NOTHING (IT'S JUST A MARKER)

        return;
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

        if((TokenLen > 2)
                && (!utf8ncmp2((char *)TokenStart, (char *)BlankStart, "##",
                        2))) {
            BYTEPTR numptr = (BYTEPTR) TokenStart;
            int32_t numwords = 0;
            numptr += 2;
            while(numptr < (BYTEPTR) BlankStart) {
                if((*numptr >= '0') && (*numptr <= '9')) {
                    numwords = numwords * 10 + (*numptr - '0');
                    ++numptr;
                    continue;
                }
                break;
            }

            // RESERVE NUMWORDS WORDS IN THE STREAM, TO BE REPLACED LATER BY THE POST-PROCESSOR WITH THE TEXT
            if(numwords < 1) {
                RetNum = ERR_SYNTAX;
                return;
            }

            if((numptr < (BYTEPTR) BlankStart) && (*numptr == ','))
                ++numptr;       // SKIP THE COMMA TO POINT TO THE TEXT TO REPLACE

            int32_t endoffset = ((BYTEPTR) CompileStringEnd) - numptr;

            if(numwords == 1)
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER, endoffset));
            else {
                rplCompileAppend(MKPROLOG(LIBRARY_NUMBER, numwords - 1));

                rplCompileAppend(endoffset);

                if(numwords > 2)
                    rplCompileAppendWords(numwords - 2);

            }

            RetNum = OK_CONTINUE;
            return;

        }

        RetNum = ERR_NOTMINE;
        return;

    case OPCODE_COMPILECONT:
    case OPCODE_DECOMPEDIT:
    case OPCODE_DECOMPILE:
    case OPCODE_VALIDATE:
    case OPCODE_CHECKOBJ:

        RetNum = OK_CONTINUE;
        return;

    case OPCODE_PROBETOKEN:
    case OPCODE_GETINFO:
    case OPCODE_GETROMID:
    case OPCODE_ROMID2PTR:
    case OPCODE_AUTOCOMPNEXT:

        RetNum = ERR_NOTMINE;
        return;

    case OPCODE_LIBINSTALL:
        LibraryList = (WORDPTR) libnumberlist;
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

#endif // COMMANDS_ONLY_PASS
