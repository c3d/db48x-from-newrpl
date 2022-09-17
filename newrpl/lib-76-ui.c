/*
 * Copyright (c) 2016, Claudio Lapilli and the newRPL Team
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
#include "ui.h"
#endif

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************

// REPLACE THE NUMBER
#define LIBRARY_NUMBER  76

//@TITLE=User Interface

#define ERROR_LIST \
    ERR(EMPTYCLIPBOARD,0)

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(COPYCLIP,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(CUTCLIP,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(PASTECLIP,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(WAIT,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(KEYEVAL,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(KEY,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(DOFORM,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(EDINSERT,MKTOKENINFO(8,TITYPE_NOTALLOWED,2,2)), \
    CMD(EDREMOVE,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(EDLEFT,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(EDRIGHT,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(EDUP,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(EDDOWN,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(EDSTART,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(EDEND,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(EDLSTART,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(EDLEND,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(EDTOKEN,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(EDACTOKEN,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(EDMODE,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(SETTHEME,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(GETTHEME,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2))


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

INCLUDE_ROMOBJECT(lib76menu_main);
INCLUDE_ROMOBJECT(lib76menu_clip);
INCLUDE_ROMOBJECT(lib76menu_keyb);

INCLUDE_ROMOBJECT(invalid_string);

ROMOBJECT clipbd_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('C', 'l', 'i', 'p'),
    TEXT2WORD('B', 'd', 0, 0)
};

ROMOBJECT theme_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('U', 'I', 'T', 'h'),
    TEXT2WORD('e', 'm', 'e', 0)
};

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const word_p const ROMPTR_TABLE[] = {
    (word_p) LIB_MSGTABLE,
    (word_p) LIB_HELPTABLE,
    (word_p) lib76menu_main,
    (word_p) lib76menu_clip,
    (word_p) lib76menu_keyb,

    (word_p) clipbd_ident,
    (word_p) invalid_string,
    (word_p) theme_ident,

    0
};

//  PROCESS KEYS FOR THE WAIT COMMAND (ON CAN INTERRUPT THE WAIT)
int waitProcess(WORD keymsg)
{
    if(keymsg == (KM_PRESS | KB_ON)) {
        RetNum = 0;     // TERMINATE LOOP
        return -1;
    }
    return 1;   // RETURN AS IF ALL OTHER KEYS WERE PROCESSED, TO BLOCK DEFAULT HANDLERS FROM RUNNING
}

// ONLY BREAK ON PRESS AND LPRESS MESSAGES
// LEAVES THE KEYMSG IN ObjectPTR
int waitKeyProcess(WORD keymsg)
{
    switch (KM_MESSAGE(keymsg)) {
    case KM_PRESS:
    case KM_LPRESS:
    case KM_REPEAT:
    case KM_LREPEAT:
        // CAPTURE THE KEY MESSAGE
        RetNum = keymsg;
        return -1;
/*
    case KM_KEYDN:
        if(keymsg == (KM_KEYDN | KB_ON)) {
            RetNum = KM_PRESS | KB_ON;
            return -1;
        }
*/
    default:
        return 1;
    }
}

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
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

            WORD saveOpcode = CurOpcode;
            CurOpcode = *rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode = saveOpcode;
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

    case COPYCLIP:
    {
        //@SHORT_DESC=Copy an object to the clipboard
        //@NEW
        // STORE LEVEL 1 INTO .Settings/Clipbd
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStoreSettings((word_p) clipbd_ident, rplPeekData(1));

        return;

    }
    case CUTCLIP:
    {
        //@SHORT_DESC=Move an object to the clipboard
        //@NEW
        // STORE LEVEL 1 INTO .Settings/Clipbd
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStoreSettings((word_p) clipbd_ident, rplPopData());

        return;

    }

    case PASTECLIP:
    {
        //@SHORT_DESC=Insert the clipboard contents on the stack
        //@NEW
        word_p object = rplGetSettings((word_p) clipbd_ident);

        if(!object)
            rplError(ERR_EMPTYCLIPBOARD);
        else {
            if(ISAUTOEXPLIST(*object)) {
                int32_t nitems = rplListLength(object);
                rplExpandStack(nitems);
                if(Exceptions)
                    return;
                word_p ptr = object + 1;

                while(nitems--) {
                    rplPushData(ptr);
                    ptr = rplSkipOb(ptr);
                }

            }
            else
                rplPushData(object);

        }
        return;
    }

    case WAIT:
    {
        //@SHORT_DESC=Wait for a key press or a time lapse
        //@INCOMPAT
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        REAL timeout;
        rplReadNumberAsReal(rplPeekData(1), &timeout);
        timeout.exp += 3;       // CONVERT FROM SECONDS TO MILLISECONDS
        int64_t mstimeout = getint64_tReal(&timeout);

        WORD keymsg;

        RetNum = 0;

        if(mstimeout > 0) {
            halOuterLoop(mstimeout, &waitProcess, 0,
                    OL_NOCOMMS | OL_NOEXIT | OL_NOAUTOOFF | OL_NOCUSTOMKEYS |
                    OL_NODEFAULTKEYS | OL_EXITONERROR);
            rplDropData(1);
        }
        else {
            if(mstimeout < 0)
                mstimeout = -mstimeout;
            halOuterLoop(mstimeout, &waitKeyProcess, 0,
                    OL_NOCOMMS | OL_NOEXIT | OL_NOAUTOOFF | OL_NODEFAULTKEYS |
                    OL_NOCUSTOMKEYS | OL_LONGPRESS | OL_EXITONERROR);

            keymsg = RetNum;

            if(keymsg) {
                // PUSH THE KEY MESSAGE

                word_p keyname = rplMsg2KeyName(keymsg);
                if(!keyname)
                    return;
                rplOverwriteData(1, keyname);
            }
            else
                rplOverwriteData(1, (word_p) empty_string);
        }
        return;

    }

    case KEYEVAL:
    {
        //@SHORT_DESC=Simulate a keypress from within a program
        //@INCOMPAT
        // REMOVE A CUSTOM KEY DEFINITION
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISSTRING(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        int32_t keycode = rplKeyName2Msg(rplPeekData(1));

        if(!keycode) {
            rplError(ERR_INVALIDKEYNAME);
            return;
        }

        rplDropData(1);

        // EVALUATE A KEY THAT MAY CALL AN RPL PROGRAM FROM WITHIN RPL??
        // THIS IS POTENTIALLY DANGEROUS, AS MOST KEY DEFINITIONS ARE MEANT TO RUN FROM THE MAIN OUTER LOOP

        // POST KEYBOARD MESSAGES AS NEEDED, RETURN TO OUTER POL WITH HALT/AUTORESUME
        // AND LET IT ALL WORK.

        halPostKeyboardMessage(keycode);

        rplCallOperator(CMD_AUTOBKPOINT);

        return;

    }

    case KEY:
    {
        //@SHORT_DESC=Get instantaneous state of the keyboard
        //@INCOMPAT
        keymatrix key = keyb_getmatrix();

        if(!key) {
            rplPushData((word_p) zero_bint);
            return;
        }

        int32_t knum = 0;
        int k;
        for(k = 0; k < 63; ++k) {
            if(key & (1LL << k)) {
                word_p kn = rplMsg2KeyName(KEYMAP_CODEFROMBIT(k));

                if(!kn)
                    return;
                rplPushData(kn);

                ++knum;
            }

        }
        rplNewint32_tPush(knum, DECint32_t);

        return;

    }

    case DOFORM:
    {
        //@SHORT_DESC=Take a variable identifier with a form list
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStripTagStack(1);
        if(!ISLIST(*rplPeekData(1))) {
            rplError(ERR_LISTEXPECTED);
            return;
        }

        // STORE THE FORM DATA
        rplStoreSettings((word_p) currentform_ident, rplPeekData(1));

        if(Exceptions)
            return;

        uiUpdateForm(rplPeekData(1));

        rplDropData(1);
        halSwitch2Form();

        return;

    }

    case EDINSERT:
    {
        //@SHORT_DESC=Insert given text into the editor
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStripTagStack(1);
        if(!ISSTRING(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        word_p string = rplPeekData(1);
        byte_p start, end;
        start = (byte_p) (string + 1);
        end = start + rplStrSize(string);

        uiOpenAndInsertTextN(start, end);

        rplDropData(1);

        return;

    }

    case EDREMOVE:
    {
        //@SHORT_DESC=Remove characters in the editor at the cursor position
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStripTagStack(1);
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        if(!(halGetContext() & CONTEXT_INEDITOR))
            return;     // DO NOTHING UNLESS AN EDITOR IS OPEN

        int64_t nchars = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;

        uiRemoveCharacters(nchars);

        rplDropData(1);

        return;

    }
    case EDLEFT:
    {
        //@SHORT_DESC=Move cursor to the left in the editor
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStripTagStack(1);
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        if(!(halGetContext() & CONTEXT_INEDITOR))
            return;     // DO NOTHING UNLESS AN EDITOR IS OPEN

        int64_t nchars = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;

        uiCursorLeft(nchars);

        rplDropData(1);

        return;

    }
    case EDRIGHT:
    {
        //@SHORT_DESC=Move cursor to the right in the editor
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStripTagStack(1);
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        if(!(halGetContext() & CONTEXT_INEDITOR))
            return;     // DO NOTHING UNLESS AN EDITOR IS OPEN

        int64_t nchars = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;

        uiCursorRight(nchars);

        rplDropData(1);

        return;

    }
    case EDUP:
    {
        //@SHORT_DESC=Move cursor up in the editor
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStripTagStack(1);

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        if(!(halGetContext() & CONTEXT_INEDITOR))
            return;     // DO NOTHING UNLESS AN EDITOR IS OPEN

        int64_t nchars = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;

        uiCursorUp(nchars);

        rplDropData(1);

        return;

    }
    case EDDOWN:
    {
        //@SHORT_DESC=Move cursor down in the editor
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        if(!(halGetContext() & CONTEXT_INEDITOR))
            return;     // DO NOTHING UNLESS AN EDITOR IS OPEN

        int64_t nchars = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;

        uiCursorDown(nchars);

        rplDropData(1);

        return;

    }
    case EDSTART:
    {
        //@SHORT_DESC=Move cursor to the start of text in the editor
        //@NEW

        if(!(halGetContext() & CONTEXT_INEDITOR))
            return;     // DO NOTHING UNLESS AN EDITOR IS OPEN

        uiCursorStartOfText();

        return;

    }
    case EDEND:
    {
        //@SHORT_DESC=Move cursor to the end of text in the editor
        //@NEW

        if(!(halGetContext() & CONTEXT_INEDITOR))
            return;     // DO NOTHING UNLESS AN EDITOR IS OPEN

        uiCursorEndOfText();

        return;

    }

    case EDLSTART:
    {
        //@SHORT_DESC=Move cursor to the start of current line in the editor
        //@NEW

        if(!(halGetContext() & CONTEXT_INEDITOR))
            return;     // DO NOTHING UNLESS AN EDITOR IS OPEN

        uiCursorStartOfLine();

        return;

    }
    case EDLEND:
    {
        //@SHORT_DESC=Move cursor to the end of current line in the editor
        //@NEW

        if(!(halGetContext() & CONTEXT_INEDITOR))
            return;     // DO NOTHING UNLESS AN EDITOR IS OPEN

        uiCursorEndOfLine();

        return;

    }

    case EDTOKEN:
    {
        //@SHORT_DESC=Extract one full word at the cursor location in the editor
        //@NEW

        if(!(halGetContext() & CONTEXT_INEDITOR))
            return;     // DO NOTHING UNLESS AN EDITOR IS OPEN

        byte_p start, end;

        start = uiAutocompStringStart();
        end = uiAutocompStringTokEnd();

        word_p newstr = rplCreateString(start, end);

        if(!newstr)
            return;

        rplPushData(newstr);

        return;

    }

    case EDACTOKEN:
    {
        //@SHORT_DESC=Extract one word at the left of cursor location (suitable for autocomplete)
        //@NEW

        if(!(halGetContext() & CONTEXT_INEDITOR))
            return;     // DO NOTHING UNLESS AN EDITOR IS OPEN

        byte_p start, end;

        start = uiAutocompStringStart();
        end = uiAutocompStringEnd();

        word_p newstr = rplCreateString(start, end);

        if(!newstr)
            return;

        rplPushData(newstr);

        return;

    }

    case EDMODE:
    {
        //@SHORT_DESC=Change the cursor mode in the editor
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStripTagStack(1);

        if(!ISSTRING(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        if(!(halGetContext() & CONTEXT_INEDITOR))
            return;     // DO NOTHING UNLESS AN EDITOR IS OPEN

        byte_p str = (byte_p) (rplPeekData(1) + 1);
        if(rplStrLen(rplPeekData(1)) >= 1) {
            switch (*str) {
            case 'L':
            case 'C':
                halForceAlphaModeOn();
                halSetCmdLineMode(*str);
                break;
            case 'A':
            case 'D':
            case 'P':
                halForceAlphaModeOff();
                halSetCmdLineMode(*str);
                break;
            default:
                halForceAlphaModeOff();
                break;
            }

        }
        rplDropData(1);

        return;

    }
    case SETTHEME:
    {
        //@SHORT_DESC=Set system color theme
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStripTagStack(1);

        if(!ISLIST(*rplPeekData(1))) {
            rplError(ERR_LISTEXPECTED);
            return;
        }


        // Take a list of 64 integers and use them as palette entries
        // Also store the list in .Settings for permanent use


        if(rplListLength(rplPeekData(1))<PALETTE_SIZE) {
            rplError(ERR_INVALIDLISTSIZE);
            return;
        }

        int k;
        word_p obj=rplPeekData(1)+1;
        WORD palette[PALETTE_SIZE];
        uint64_t color;

        for(k=0;k<PALETTE_SIZE;++k)
        {
            color=rplReadNumberAsInt64(obj);
            if(Exceptions) return;
            palette[k]=(WORD)color;
            obj=rplSkipOb(obj);
        }

        // Here we were able to read all numbers without any errors, so it's a valid palette

        halSetupTheme(palette);

        // Store the list in .Settings for future use on poweron

        rplStoreSettings((word_p) theme_ident, rplPeekData(1));

        rplDropData(1);

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

        libCompileCmds(LIBRARY_NUMBER, (char **)LIB_NAMES, NULL,
                LIB_NUMBEROFCMDS);
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
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND
    {
        libProbeCmds((char **)LIB_NAMES, (int32_t *) LIB_TOKENINFO,
                LIB_NUMBEROFCMDS);

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
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL int32_t, .42 = HEX INTEGER

        if(ISPROLOG(*ObjectPTR)) {
            TypeInfo = LIBRARY_NUMBER * 100;
            DecompHints = 0;
            RetNum = OK_TOKENINFO | MKTOKENINFO(0, TITYPE_NOTALLOWED, 0, 1);
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
        if(ISPROLOG(*ObjectPTR)) {
            RetNum = ERR_INVALID;
            return;
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
        if(MENUNUMBER(MenuCodeArg) > 3) {
            RetNum = ERR_NOTMINE;
            return;
        }
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR = ROMPTR_TABLE[MENUNUMBER(MenuCodeArg) + 2];
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
