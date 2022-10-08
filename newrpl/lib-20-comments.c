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
#include "hal_api.h"
#include "newrpl.h"
#include "sysvars.h"
#endif

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************

//@TITLE=Comments

// REPLACE THE NUMBER
#define LIBRARY_NUMBER  20

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(STRIPCOMMENTS,MK_TOKEN_INFO(13,TITYPE_NOTALLOWED,1,2))
//    ECMD(CMDNAME,"CMDNAME",MK_TOKEN_INFO(7,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER,LIBRARY_NUMBER+1,LIBRARY_NUMBER+2,LIBRARY_NUMBER+3

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"

#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

INCLUDE_ROMOBJECT(LIB_HELPTABLE);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const word_p const ROMPTR_TABLE[] = {
    (word_p) LIB_HELPTABLE,
    0
};

// FIX THE PROLOG OF A STRING TO MATCH THE DESIRED LENGTH IN CHARACTERS
// LOW-LEVEL FUNCTION, DOES NOT ACTUALLY RESIZE THE OBJECT
void rplSetCommentLength(word_p string, int32_t length)
{
    int32_t padding = (4 - ((length) & 3)) & 3;

    *string = MK_PROLOG(LIBRARY_NUMBER + padding, (length + 3) >> 2);
}

void LIB_HANDLER()
{
    if(IS_PROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE

        // DO ABSOLUTELY NOTHING (IT'S JUST A COMMENT)

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
            if(IS_PROLOG(*rplPeekData(1))) {
                // DO-NOTHING
                rplPopData();
                return;
            }
            WORD saveOpcode = CurOpcode;
            CurOpcode = *rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode = saveOpcode;
            return;
        }
        if(OPCODE(CurOpcode) == OVR_ISTRUE) {
            rplOverwriteData(1, (word_p) one_bint);
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
    case STRIPCOMMENTS:
    {
        //@SHORT_DESC=Remove all comments from a compiled program
        //@NEW
        // TODO: IMPLEMENT THIS
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISPROGRAM(*rplPeekData(1))) {
            rplError(ERR_PROGRAMEXPECTED);
            return;
        }

        // SCAN THE EXECUTABLE TO DETERMINE SIZE WITHOUT COMMENTS
        int32_t newsize = 1;

        word_p ptr, end;
        word_p *Stacksave = DSTop;

        ptr = rplPeekData(1);
        end = rplSkipOb(ptr);
        ++ptr;

        // RECURSIVE SCAN
        do {

            if(Stacksave != DSTop) {
                // CONTINUE OBJECT WHERE WE LEFT OFF
                newsize += rplReadint32_t(rplPopData());
                end = rplSkipOb(rplPopData());
            }

            while(ptr != end) {
                if(ISPROGRAM(*ptr)) {
                    rplPushData(ptr);   // PUSH THE CURRENT OBJECT
                    rplNewBINTPush(newsize, DECBINT);
                    if(Exceptions) {
                        DSTop = Stacksave;
                        return;
                    }
                    ptr = rplPeekData(2);       // RE-READ POINTERS IN CASE OF GC
                    end = rplSkipOb(ptr);
                    newsize = 1;
                    ++ptr;
                    continue;
                }
                if(ISLIST(*ptr)) {
                    rplPushData(ptr);   // PUSH THE CURRENT OBJECT
                    rplNewBINTPush(newsize, DECBINT);
                    if(Exceptions) {
                        DSTop = Stacksave;
                        return;
                    }
                    ptr = rplPeekData(2);       // RE-READ POINTERS IN CASE OF GC
                    end = rplSkipOb(ptr);
                    newsize = 1;
                    ++ptr;
                    continue;
                }

                if(ISCOMMENT(*ptr)) {
                    // CHECK IF A COMMENT IS PERMANENT, OTHERWISE SKIP
                    int32_t len = OBJSIZE(*ptr);
                    if(!((len > 0) && ((ptr[1] & 0xff) == '@')
                                && (((ptr[1] >> 8) & 0xff) != '@'))) {
                        // NOT A PERMANENT COMMENT SKIP AND CONTINUE
                        ptr = rplSkipOb(ptr);
                        continue;
                    }

                }

                // ALL OTHER OBJECTS NEED TO BE KEPT
                newsize += rplObjSize(ptr);
                ptr = rplSkipOb(ptr);

            }

            // FINISHED ONE OBJECT, CONTINUE IF THERE'S MORE OBJECTS IN THE STACK

        }
        while(DSTop != Stacksave);

        // HERE newsize HAS THE TOTAL SIZE OF THE NEW OBJECT WITHOUT COMMENTS

        ptr = rplAllocTempOb(newsize - 1);
        if(!ptr)
            return;

        ScratchPointer1 = ptr;  // SAFEKEEPING AGAINST POSSIBLE GC DURING RECURSIVE COPY
        ScratchPointer2 = ptr;  // RUNNING POINTER, DESTINATION WHERE TO COPY
        ScratchPointer3 = ptr;  // START OF DESTINATION OBJECT, USED TO PATCH THE FINAL SIZE

        // SECOND PASS, COPY TO NEW OBJECT
        ptr = rplPeekData(1);
        end = rplSkipOb(ptr);
        *ScratchPointer2 = MK_PROLOG(LIBNUM(*ptr), 0);
        ++ScratchPointer2;
        ++ptr;
        newsize = 1;

        // RECURSIVE SCAN
        do {

            if(Stacksave != DSTop) {
                // CONTINUE OBJECT WHERE WE LEFT OFF
                newsize += rplReadint32_t(rplPopData());
                end = rplSkipOb(rplPopData());
                ScratchPointer3 = rplPopData();
            }

            while(ptr != end) {
                if(ISPROGRAM(*ptr)) {
                    rplPushDataNoGrow(ScratchPointer2);
                    rplPushData(ptr);   // PUSH THE CURRENT OBJECT
                    rplNewBINTPush(newsize, DECBINT);
                    if(Exceptions) {
                        DSTop = Stacksave;
                        return;
                    }
                    ptr = rplPeekData(2);       // RE-READ POINTERS IN CASE OF GC
                    end = rplSkipOb(ptr);
                    *ScratchPointer2 = MK_PROLOG(LIBNUM(*ptr), 0);
                    ++ScratchPointer2;
                    newsize = 1;
                    ++ptr;
                    continue;
                }
                if(ISLIST(*ptr)) {
                    rplPushDataNoGrow(ScratchPointer2);
                    rplPushData(ptr);   // PUSH THE CURRENT OBJECT
                    rplNewBINTPush(newsize, DECBINT);
                    if(Exceptions) {
                        DSTop = Stacksave;
                        return;
                    }
                    ptr = rplPeekData(1);       // RE-READ POINTERS IN CASE OF GC
                    end = rplSkipOb(ptr);
                    *ScratchPointer2 = MK_PROLOG(LIBNUM(*ptr), 0);
                    ++ScratchPointer2;
                    newsize = 1;
                    ++ptr;
                    continue;
                }

                if(ISCOMMENT(*ptr)) {
                    // CHECK IF A COMMENT IS PERMANENT, OTHERWISE SKIP
                    int32_t len = OBJSIZE(*ptr);
                    if(!((len > 0) && ((ptr[1] & 0xff) == '@')
                                && (((ptr[1] >> 8) & 0xff) != '@'))) {
                        // NOT A PERMANENT COMMENT SKIP AND CONTINUE
                        ptr = rplSkipOb(ptr);
                        continue;
                    }

                }

                // ALL OTHER OBJECTS NEED TO BE KEPT
                rplCopyObject(ScratchPointer2, ptr);
                newsize += rplObjSize(ptr);
                ptr = rplSkipOb(ptr);
                ScratchPointer2 = rplSkipOb(ScratchPointer2);
            }

            // FINISHED ONE OBJECT, CONTINUE IF THERE'S MORE OBJECTS IN THE STACK
            *ScratchPointer3 = MK_PROLOG( LIBNUM(*ScratchPointer3), newsize - 1);

        }
        while(DSTop != Stacksave);

        // DONE, PUT THE NEW OBJECT IN THE STACK NOW

        rplOverwriteData(1, ScratchPointer1);

        return;
    }

        // ADD MORE OPCODES HERE

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

        if(*((utf8_p) TokenStart) == '@') {
            // START A STRING

            ScratchPointer4 = CompileEnd;       // SAVE CURRENT COMPILER POINTER TO FIX THE OBJECT AT THE END

            rplCompileAppend(MK_PROLOG(LIBRARY_NUMBER, 0));

            union
            {
                WORD word;
                BYTE bytes[4];
            } temp;

            int32_t count = 0, mode = 1, endmark = 0;
            utf8_p ptr = (utf8_p) TokenStart;
            ++ptr;      // SKIP THE INITIAL AT

            // mode==1 MEANS JUST A STANDARD COMMENT
            // mode==2 MEANS @@ COMMENT THAT IS PERMANENT (SINGLE LINE)
            // mode==3 MEANS @@@ MULTILINE COMMENT

            do {
                while(count < 4) {
                    if(ptr == (utf8_p) NextTokenStart) {
                        // WE ARE AT THE END OF THE GIVEN STRING, STILL NO CLOSING QUOTE, SO WE NEED MORE

                        // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                        if(count)
                            rplCompileAppend(temp.word);
                        *ScratchPointer4 =
                                MK_PROLOG(LIBRARY_NUMBER + ((4 - count) & 3),
                                (WORD) (CompileEnd - ScratchPointer4) - 1);
                        RetNum = OK_NEEDMORE;
                        return;
                    }

                    if(*ptr == '@') {
                        if(mode < 0x40000000)
                            ++mode;
                        else {
                            ++endmark;
                        }
                    }
                    else
                        mode |= 0x40000000;

                    if(((*ptr == '\n') && (mode <= 0x40000002))
                            || ((mode & ~0x40000000) == endmark)) {
                        // END OF LINE = END OF COMMENT
                        temp.bytes[count] = *ptr;
                        ++count;
                        ++ptr;
                        // WE HAVE REACHED THE END OF THE COMMENT
                        if(count) {
                            rplCompileAppend(temp.word);
                        }
                        *ScratchPointer4 =
                                MK_PROLOG(LIBRARY_NUMBER + ((4 - count) & 3),
                                (WORD) (CompileEnd - ScratchPointer4) - 1);

                        if(ptr < (utf8_p) BlankStart) {
                            //   FOUND THE AT SYMBOL WITHIN THE COMMENT ITSELF, SPLIT THE TOKEN
                            TokenStart = (word_p) ptr;
                            RetNum = OK_SPLITTOKEN;
                        }
                        else
                            RetNum = OK_CONTINUE;

                        // DROP THE COMMENT DEPENDING ON FLAGS
                        if((mode != 0x40000002)
                                && (rplTestSystemFlag(FL_STRIPCOMMENTS) == 1))
                            CompileEnd = ScratchPointer4;

                        return;
                    }
                    if(count == 0)
                        temp.word = 0;
                    temp.bytes[count] = *ptr;
                    ++count;
                    ++ptr;
                }
                //  WE HAVE A COMPLETE WORD HERE
                ScratchPointer1 = (word_p) ptr;        // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                rplCompileAppend(temp.word);
                ptr = (utf8_p) ScratchPointer1;

                count = 0;

            }
            while(1);   // DANGEROUS! BUT WE WILL RETURN FROM THE CHECK WITHIN THE INNER LOOP;
            //  THIS IS UNREACHABLE CODE HERE

        }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER, (char **)LIB_NAMES, NULL,
                LIB_NUMBEROFCMDS);
        return;

    case OPCODE_COMPILECONT:
        // CONTINUE COMPILING STRING
    {
        union
        {
            WORD word;
            BYTE bytes[4];
        } temp;
        int32_t mode = 1, endmark = 0;
        if(CompileEnd > ScratchPointer4 + 1) {
            // GET THE FIRST WORD TO EXTRACT THE COMMENT MODE
            temp.word = *(ScratchPointer4 + 1);

            if(temp.bytes[0] == '@') {
                ++mode;
                if(temp.bytes[1] == '@')
                    ++mode;
            }
        }

        mode |= 0x40000000;

        int32_t count = (4 - (LIBNUM(*ScratchPointer4) & 3)) & 3;  // GET NUMBER OF BYTES ALREADY WRITTEN IN LAST WORD

        if(count) {
            --CompileEnd;
            temp.word = *CompileEnd;    // GET LAST WORD
        }
        else
            temp.word = 0;
        utf8_p ptr = (utf8_p) TokenStart;
        do {
            while(count < 4) {
                if(ptr == (utf8_p) NextTokenStart) {
                    // WE ARE AT THE END OF THE GIVEN STRING, STILL NO CLOSING QUOTE, SO WE NEED MORE

                    // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                    if(count)
                        rplCompileAppend(temp.word);
                    *ScratchPointer4 =
                            MK_PROLOG(LIBRARY_NUMBER + ((4 - count) & 3),
                            (WORD) (CompileEnd - ScratchPointer4) - 1);
                    RetNum = OK_NEEDMORE;
                    return;
                }

                if(*ptr == '@') {
                    if(mode < 0x40000000)
                        ++mode;
                    else {
                        ++endmark;
                    }
                }
                else
                    mode |= 0x40000000;

                if(((*ptr == '\n') && (mode <= 0x40000002))
                        || ((mode & ~0x40000000) == endmark)) {
                    // END OF COMMENT
                    temp.bytes[count] = *ptr;
                    ++count;

                    ++ptr;
                    // WE HAVE REACHED THE END OF THE STRING
                    if(count)
                        rplCompileAppend(temp.word);
                    *ScratchPointer4 =
                            MK_PROLOG(LIBRARY_NUMBER + ((4 - count) & 3),
                            (WORD) (CompileEnd - ScratchPointer4) - 1);

                    if(ptr < (utf8_p) BlankStart) {
                        //   FOUND THE AT SYMBOL WITHIN THE COMMENT ITSELF, SPLIT THE TOKEN
                        TokenStart = (word_p) ptr;
                        RetNum = OK_SPLITTOKEN;
                    }
                    else
                        RetNum = OK_CONTINUE;

                    // DROP THE COMMENT DEPENDING ON FLAGS
                    if((mode != 0x40000002)
                            && (rplTestSystemFlag(FL_STRIPCOMMENTS) == 1))
                        CompileEnd = ScratchPointer4;

                    return;
                }
                if(count == 0)
                    temp.word = 0;
                temp.bytes[count] = *ptr;
                ++count;
                ++ptr;

            }
            //  WE HAVE A COMPLETE WORD HERE
            ScratchPointer1 = (word_p) ptr;    // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
            rplCompileAppend(temp.word);
            ptr = (utf8_p) ScratchPointer1;

            count = 0;

        }
        while(1);       // DANGEROUS! BUT WE WILL RETURN FROM THE CHECK WITHIN THE INNER LOOP;
        //  THIS IS UNREACHABLE CODE HERE

    }
    case OPCODE_DECOMPEDIT:
    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors
        if(IS_PROLOG(*DecompileObject)) {
            rplDecompAppendChar('@');
            int32_t len =
                    (OBJSIZE(*DecompileObject) << 2) -
                    (LIBNUM(*DecompileObject) & 3);
            utf8_p string = (utf8_p) (DecompileObject + 1);
            /*
               if(string[len-1]=='\n') {
               // COMMENT ENDS IN NEWLINE
               if((len>1) && (string[len-2]=='\r')) --len;
               rplDecompAppendString2(string,len-1);
               if(!rplDecompDoHintsWidth(HINT_NLAFTER))    // ADD A NEWLINE AND RESPECT THE INDENTATION
               {
               // END THE COMMENT WITH COMMENT SYMBOL SINCE NO NEWLINE WAS ADDED!
               rplDecompAppendChar('@');
               while((len>0) &&(*string=='@')) { rplDecompAppendChar('@'); ++string; --len; }
               }
               }
               else */
            rplDecompAppendString2(string, len);

            RetNum = OK_CONTINUE;
            return;

        }

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

        //libGetRomptrID(LIBRARY_NUMBER,(word_p *)ROMPTR_TABLE,ObjectPTR);
        RetNum = ERR_NOTMINE;
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectPTR = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        //libGetPTRFromID((word_p *)ROMPTR_TABLE,ObjectID,ObjectIDHash);
        RetNum = ERR_NOTMINE;
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
    {
        // STRINGS ARE ALWAYS VALID, EVEN IF THEY CONTAIN INVALID UTF-8 SEQUENCES

        RetNum = OK_CONTINUE;
        return;
    }

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER, (char **)LIB_NAMES,
                LIB_NUMBEROFCMDS);
        return;

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
        RetNum = ERR_NOTMINE;
        return;

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
        RetNum = ERR_NOTMINE;
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

#endif // COMMANDS_ONLY_PASS
