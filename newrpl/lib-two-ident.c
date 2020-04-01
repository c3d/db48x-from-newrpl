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

// THIS LIBRARY PROVIDES ONLY COMPILATION OF IDENTS AFTER ALL OTHER LIBRARIES
// HAD A CHANCE TO IDENTIFY THEIR COMMANDS
// ANY LAM COMMANDS HAVE TO BE IN A SEPARATE LIBRARY

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************

// REPLACE THE NUMBER
#define LIBRARY_NUMBER  2

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

//#define COMMAND_LIST
//    CMD(CMDNAME,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2))
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

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
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

        // COMPILE IDENTS QUOTED AND UNQUOTED, AND IF CurrentConstruct== NEWLOCALENV THEN ADD QUOTES
    {
        BYTEPTR tok = (BYTEPTR) TokenStart;
        BINT len = TokenLen;

        if(*tok == '\'') {
            // QUOTED IDENT OR ALGEBRAIC OBJECT
            if(*utf8nskip((char *)tok, (char *)BlankStart,
                        TokenLen - 1) != '\'') {
                // NOT A SIMPLE IDENT, THEN IT'S A SYMBOLIC EXPRESSION
                rplCompileAppend(MKPROLOG(DOSYMB, 0));

                if(TokenLen > 1) {
                    NextTokenStart = (WORDPTR) (((char *)TokenStart) + 1);
                }
                RetNum = OK_STARTCONSTRUCT_INFIX;
                return;
            }

            ++tok;
            len -= 2;

            if(!rplIsValidIdent(tok, ((BYTEPTR) BlankStart) - 1)) {

                // NOT A SIMPLE IDENT, THEN IT'S A SYMBOLIC EXPRESSION
                rplCompileAppend(MKPROLOG(DOSYMB, 0));

                if(TokenLen > 1) {
                    NextTokenStart = (WORDPTR) (((char *)TokenStart) + 1);
                }
                RetNum = OK_STARTCONSTRUCT_INFIX;
                return;
            }

            rplCompileIDENT(DOIDENT, tok, ((BYTEPTR) BlankStart) - 1);

            RetNum = OK_CONTINUE;
            return;
        }

        // UNQUOTED IDENTS

        if(!rplIsValidIdent(tok, (BYTEPTR) BlankStart)) {

            // DISABLE IMPLICIT MULTIPLICATION, DOESN'T WORK IN SYMBOLICS ANYWAY
            /*
               if( (*tok>='0')&&(*tok<='9')) {
               // IDENT STARTS WITH A NUMBER, THERE'S IMPLICIT MULTIPLICATION

               LIBHANDLER hanreal=rplGetLibHandler(DOREAL);
               WORD saveopcode=CurOpcode;
               RetNum=-1;
               CurOpcode=MKOPCODE(DOREAL,OPCODE_PROBETOKEN);
               (*hanreal)();
               CurOpcode=saveopcode;

               if(RetNum<OK_TOKENINFO) {
               RetNum=ERR_SYNTAX;
               return;
               }
               // WE POSSIBLY HAVE IMPLICIT MULTIPLICATION BETWEEN REAL AND IDENT
               BINT numberlen=TI_LENGTH(RetNum);
               BYTEPTR splitpoint=(BYTEPTR)utf8nskip((char *)tok,(char *)BlankStart,numberlen);
               BINT splitoff=splitpoint-(BYTEPTR)TokenStart;
               if(rplIsValidIdent(splitpoint,(BYTEPTR)BlankStart)) {
               // CONFIRMED IMPLICIT MULTIPLICATION
               // TRY TO COMPILE AS NUMBER IDENT *
               UBINT64 locale=rplGetSystemLocale();
               newRealFromText(&RReg[0],(char *)tok,(char *)splitpoint,locale);
               if(RReg[0].flags&F_ERROR) {
               RetNum=ERR_SYNTAX;
               return;
               }

               if(isintegerReal(&RReg[0]) && inBINT64Range(&RReg[0])) {
               rplCompileBINT(getBINT64Real(&RReg[0]),(RReg[0].flags&F_APPROX)? DECBINTAPP:DECBINT);
               }
               else rplCompileReal(&RReg[0]);
               tok=((BYTEPTR)TokenStart)+splitoff;
               } else {
               RetNum=ERR_SYNTAX;
               return;
               }

               } else {
               RetNum=ERR_SYNTAX;
               return;
               }
             */
            RetNum = ERR_NOTMINE;
            return;
        }

        if(LIBNUM(CurrentConstruct) == LIBNUM(CMD_NEWLOCALENV)) {
            // INSIDE THIS CONSTRUCT WE NEED TO QUOTE ALL
            // IDENTS

            rplCompileIDENT(DOIDENT, tok, (BYTEPTR) BlankStart);

            RetNum = OK_CONTINUE;
            return;
        }
        if(CurrentConstruct == MKPROLOG(DOSYMB, 0)) {
            // INSIDE SYMBOLICS, ALL IDENTS ARE UNQUOTED

            rplCompileIDENT(DOIDENTEVAL, tok, (BYTEPTR) BlankStart);

            RetNum = OK_CONTINUE;
            return;
        }

        // CHECK IF IT'S A LAM, COMPILE TO A GETLAM OPCODE IF IT IS

        WORDPTR *LAMptr = rplFindLAMbyName(tok, len, 1);

        if(LAMptr < LAMTopSaved) {
            // THIS IS NOT A VALID LAM, COMPILE AS AN UNQUOTED IDENT

            rplCompileIDENT(DOIDENTEVAL, tok, (BYTEPTR) BlankStart);

            RetNum = OK_CONTINUE;
            return;
        }

        if(LAMptr < nLAMBase) {
            // THIS IS A LAM FROM AN UPPER CONSTRUCT
            // WE CAN USE GETLAM ONLY INSIDE LOOPS, NEVER ACROSS SECONDARIES

            WORDPTR *env = nLAMBase;
            WORD prolog;
            do {
                if(LAMptr > env)
                    break;
                prolog = **(env + 1);   // GET THE PROLOG OF THE SECONDARY
                if(ISPROLOG(prolog) && LIBNUM(prolog) == SECO) {
                    // LAMS ACROSS << >> SECONDARIES HAVE TO BE COMPILED AS IDENTS
                    rplCompileIDENT(DOIDENTEVAL, tok, (BYTEPTR) BlankStart);

                    RetNum = OK_CONTINUE;
                    return;
                }
                env = rplGetNextLAMEnv(env);
            }
            while(env);

        }

        // SPECIAL CASE: WHEN A SECO DOESN'T HAVE ANY LOCALS YET
        // BUT LAMS FROM THE PREVIOUS SECO SHOULDN'T BE COMPILED TO GETLAMS

        // SCAN ALL CURRENT CONSTRUCTS TO FIND THE INNERMOST SECONDARY
        // THEN VERIFY IF THAT SECONDARY IS THE CURRENT LAM ENVIRONMENT

        // THIS IS TO FORCE ALL LAMS IN A SECO TO BE COMPILED AS IDENTS
        // INSTEAD OF GETLAMS

        // DOCOL'S ARE OK AND ALWAYS COMPILED AS GETLAMS
        WORDPTR *scanenv = ValidateTop - 1;

        while(scanenv >= ValidateBottom) {
            if((LIBNUM(**scanenv) == SECO) && (ISPROLOG(**scanenv))) {
                // FOUND INNERMOST SECONDARY
                if(*scanenv > *(nLAMBase + 1)) {
                    // THE CURRENT LAM BASE IS OUTSIDE THE INNER SECONDARY
                    rplCompileIDENT(DOIDENTEVAL, tok, (BYTEPTR) BlankStart);

                    RetNum = OK_CONTINUE;
                    return;
                }
                break;

            }
            --scanenv;
        }

        // IT'S A KNOWN LOCAL VARIABLE, COMPILE AS GETLAM
        // BUT ONLY IF WE ARE NOT INSIDE A COMPOSITE (LIST, ARRAY, ETC)
        if((CurrentConstruct == MKPROLOG(DOLIST, 0)) || (CurrentConstruct == MKPROLOG(DOMATRIX, 0))     // ADD HERE ARRAYS LATER
                ) {
            rplCompileIDENT(DOIDENTEVAL, tok, (BYTEPTR) BlankStart);

            RetNum = OK_CONTINUE;
            return;
        }
        BINT Offset = ((BINT) (LAMptr - nLAMBase)) >> 1;

        if(Offset <= 32767 && Offset >= -32768) {
            rplCompileAppend(MKOPCODE(DOIDENT,
                        GETLAMNEVAL + (Offset & 0xffff)));
        }
        else {
            rplCompileIDENT(DOIDENTEVAL, tok, (BYTEPTR) BlankStart);
        }

        RetNum = OK_CONTINUE;
        return;

    }

        return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        // NOTHING TO DECOMPILE HERE. THIS LIBRARY DOES NOT DEFINE ANY COMMANDS OR OBJECTS.
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

        RetNum = OK_CONTINUE;
        return;
    case OPCODE_PROBETOKEN:
    {
        BYTEPTR tokptr, tokend, lastgood, tokstart;
        BINT maxlen, len;

        tokstart = (BYTEPTR) TokenStart;
        tokend = (BYTEPTR) BlankStart;

        while((*tokstart == '.') && (tokstart < tokend))
            ++tokstart; // SKIP THE INITIAL DOTS

        tokptr = tokstart;

        tokptr = (BYTEPTR) utf8skipst((char *)tokptr, (char *)tokend);
        for(maxlen = 0, len = tokptr - (BYTEPTR) TokenStart; tokptr <= tokend;
                ++len) {
            // TEST IF WE COULD ABSORB ATTRIBUTES
            if(*tokptr == ':') {
                // TRY INCLUDING THE ATTRIBUTES FIRST
                BYTEPTR tokattr = tokptr + 1;
                while((*tokattr != ':') && (tokattr < tokend))
                    ++tokattr;

                if((tokattr < tokend)
                        && (rplIsValidIdent((BYTEPTR) tokstart, tokattr + 1))) {
                    // ABSORB ATTRIBUTES, THEY ARE VALID
                    tokptr = tokattr + 1;
                    maxlen = tokptr - (BYTEPTR) TokenStart;
                    lastgood = tokptr;
                    break;
                }

            }

            if(!rplIsValidIdent((BYTEPTR) tokstart, tokptr))
                break;
            maxlen = len;
            lastgood = tokptr;
            if(tokptr == tokend)
                break;
            tokptr = (BYTEPTR) utf8skipst((char *)tokptr, (char *)tokend);
        }
        if(maxlen > 0)
            RetNum = OK_TOKENINFO | MKTOKENINFO(utf8nlen((char *)TokenStart,
                        (char *)lastgood), TITYPE_IDENT, 0, 1);
        else
            RetNum = ERR_NOTMINE;
        return;
    }

    case OPCODE_GETINFO:
    {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS OR COMMANDS
        RetNum = ERR_INVALID;
        return;
    }

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

        if(ISPROLOG(*ObjectPTR)) {
            RetNum = ERR_INVALID;
            return;
        }
        RetNum = OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        //libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
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

#endif
