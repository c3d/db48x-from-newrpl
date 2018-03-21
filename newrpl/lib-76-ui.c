/*
 * Copyright (c) 2016, Claudio Lapilli and the newRPL Team
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
    CMD(KEY,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2))

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
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('C','l','i','p'),
        TEXT2WORD('B','d',0,0)

};




// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib76menu_main,
    (WORDPTR)lib76menu_clip,
    (WORDPTR)lib76menu_keyb,

    (WORDPTR)clipbd_ident,
    (WORDPTR)invalid_string,

    0
};


//  PROCESS KEYS FOR THE WAIT COMMAND (ON CAN INTERRUPT THE WAIT)
int waitProcess(BINT keymsg)
{
    if(keymsg==(KM_KEYDN|KB_ON)) { RetNum=0; return -1; }   // TERMINATE LOOP
    return 1;   // RETURN AS IF ALL OTHER KEYS WERE PROCESSED, TO BLOCK DEFAULT HANDLERS FROM RUNNING
}

// ONLY BREAK ON PRESS AND LPRESS MESSAGES
// LEAVES THE KEYMSG IN ObjectPTR
int waitKeyProcess(BINT keymsg)
{
    switch(KM_MESSAGE(keymsg))
    {
    case KM_PRESS:
    case KM_LPRESS:
    case KM_REPEAT:
    case KM_LREPEAT:
        // CAPTURE THE KEY MESSAGE
        RetNum=keymsg;
        return -1;
    case KM_KEYDN:
        if(keymsg==(KM_KEYDN|KB_ON)) { RetNum=KM_PRESS|KB_ON; return -1; }
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
        // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
            if(OPCODE(CurOpcode)==OVR_SAME) {
                if(*rplPeekData(2)==*rplPeekData(1)) {
                    rplDropData(2);
                    rplPushTrue();
                } else {
                    rplDropData(2);
                    rplPushFalse();
                }

            }
            else {
                rplError(ERR_INVALIDOPCODE);
                return;
            }

    }

    switch(OPCODE(CurOpcode))
    {

    case COPYCLIP:
    {
        //@SHORT_DESC=Copy an object to the clipboard
        //@NEW
        // STORE LEVEL 1 INTO .Settings/Clipbd
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStoreSettings((WORDPTR)clipbd_ident,rplPeekData(1));

        return;

    }
    case CUTCLIP:
    {
        //@SHORT_DESC=Move an object to the clipboard
        //@NEW
        // STORE LEVEL 1 INTO .Settings/Clipbd
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStoreSettings((WORDPTR)clipbd_ident,rplPopData());

        return;

    }

    case PASTECLIP:
    {
        //@SHORT_DESC=Insert the clipboard contents on the stack
        //@NEW
        WORDPTR object=rplGetSettings((WORDPTR)clipbd_ident);

        if(!object) rplError(ERR_EMPTYCLIPBOARD);
        else {
            if(ISAUTOEXPLIST(*object)) {
                BINT nitems=rplListLength(object);
                rplExpandStack(nitems);
                if(Exceptions) return;
                WORDPTR ptr=object+1;

                while(nitems--) { rplPushData(ptr); ptr=rplSkipOb(ptr); }


            }
            else rplPushData(object);

        }
        return;
    }


    case WAIT:
    {
        //@SHORT_DESC=Wait for a key press or a time lapse
        //@INCOMPAT
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        REAL timeout;
        rplReadNumberAsReal(rplPeekData(1),&timeout);
        timeout.exp+=3;    // CONVERT FROM SECONDS TO MILLISECONDS
        BINT64 mstimeout=getBINT64Real(&timeout);

        BINT keymsg;

        RetNum=0;

        if(mstimeout>0) {
            halOuterLoop(mstimeout,&waitProcess,0,OL_NOCOMMS|OL_NOEXIT|OL_NOAUTOOFF|OL_NOCUSTOMKEYS|OL_NODEFAULTKEYS|OL_EXITONERROR);
            rplDropData(1);
        }
        else  {
           if(mstimeout<0) mstimeout=-mstimeout;
               halOuterLoop(mstimeout,&waitKeyProcess,0,OL_NOCOMMS|OL_NOEXIT|OL_NOAUTOOFF|OL_NODEFAULTKEYS|OL_NOCUSTOMKEYS|OL_LONGPRESS|OL_EXITONERROR);

        keymsg=RetNum;

        if(keymsg) {
        // PUSH THE KEY MESSAGE

        WORDPTR keyname=rplMsg2KeyName(keymsg);
        if(!keyname) return;
        rplOverwriteData(1,keyname);
        }
        else rplOverwriteData(1,(WORDPTR)empty_string);
        }
        return;

    }

    case KEYEVAL:
    {
        //@SHORT_DESC=Simulate a keypress from within a program
        //@INCOMPAT
        // REMOVE A CUSTOM KEY DEFINITION
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISSTRING(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        BINT keycode=rplKeyName2Msg(rplPeekData(1));

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
        keymatrix key=keyb_getmatrix();


        if(!key) {
            rplPushData((WORDPTR)zero_bint);
            return;
        }

        BINT knum=0;
        int k;
        for(k=0;k<63;++k) {
            if(key&(1LL<<k)) {
                WORDPTR kn=rplMsg2KeyName(k);

                if(!kn) return;
                rplPushData(kn);

                ++knum;
            }

        }
        rplNewBINTPush(knum,DECBINT);

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
        if(MENUNUMBER(MenuCodeArg)>3) { RetNum=ERR_NOTMINE; return; }
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

