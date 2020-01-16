/*
 * Copyright (c) 2019, Claudio Lapilli and the newRPL Team
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
#define LIBRARY_NUMBER  4081

//@TITLE=Tag objects

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    ECMD(MKTAG,"→TAG",MKTOKENINFO(4,TITYPE_NOTALLOWED,2,2)), \
    CMD(DTAG,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2))






// ADD MORE OPCODES HERE

#define ERROR_LIST \
    ERR(INVALIDTAG,0)




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
INCLUDE_ROMOBJECT(lib4081_menu);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib4081_menu,

    0
};


// STRIP THE TAGS FROM N LEVELS OF THE STACK DURING ARGUMENT CHECKING
BINT rplStripTagStack(BINT nlevels)
{
    if(nlevels>DSTop-DStkBottom) nlevels=DSTop-DStkBottom;
    BINT k;
    BINT changed=0;
    for(k=1;k<=nlevels;++k) if(ISTAG(*DSTop[-k])) { changed=1; DSTop[-k]+=2+((ISPROLOG(*(DSTop[-k]+1)))? OBJSIZE(*(DSTop[-k]+1)):0); }
    return changed;
}

WORDPTR rplStripTag(WORDPTR object)
{
    if(ISTAG(*object)) return object+2+((ISPROLOG(object[1]))? OBJSIZE(object[1]):0);
    return object;
}
/* TAG OBJECT FORMAT:
 *
 * [0]=PROLOG
 * [1]=DOSTRING
 * [2..N-1]=STRING DATA
 * [N ...]=OBJECT PROLOG
*/



void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // JUST PUSH THE OBJECT ON THE STACK
        rplPushData(IPtr);
        return;
    }
    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE)
    {
        // THESE ARE OVERLOADABLE COMMANDS DISPATCHED FROM THE
        // OVERLOADABLE OPERATORS LIBRARY.

        if(OVR_GETNARGS(CurOpcode)==1) {

            if(!ISPROLOG(*rplPeekData(1))) {
                        // COMMAND AS ARGUMENT
                        if( (OPCODE(CurOpcode)==OVR_EVAL)||
                                (OPCODE(CurOpcode)==OVR_EVAL1)||
                                (OPCODE(CurOpcode)==OVR_XEQ) )
                        {

                            WORD saveOpcode=CurOpcode;
                            CurOpcode=*rplPopData();
                            // RECURSIVE CALL
                            LIB_HANDLER();
                            CurOpcode=saveOpcode;
                            return;
                        }

                    }



           }

        // STRIP TAGS FROM ALL ARGUMENTS
        rplStripTagStack(OVR_GETNARGS(CurOpcode));
        rplCallOvrOperator(CurOpcode);
        return;

    }   // END OF OVERLOADABLE OPERATORS



    switch(OPCODE(CurOpcode))
    {
    case MKTAG:
    {
        //@SHORT_DESC=Apply a tag to an object

        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISSTRING(*rplPeekData(2))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        // CHECK IF THE TAG IS VALID: IT MAY CONTAIN NO SPACES AND NO COLONS

        BINT len=rplStrLen(rplPeekData(2));
        //BYTEPTR ptr=(BYTEPTR)(rplPeekData(2)+1);

        if(len<1) {
            rplError(ERR_INVALIDTAG);
            return;
        }
        // ALLOW ARBITRARY STRINGS...
        /*
        BINT cp;
        while(len) {
            cp=utf82cp((char *)ptr,(char *)ptr+4);
            if((cp==' ')||(cp==':')) {
                rplError(ERR_INVALIDTAG);
                return;
            }
            ptr=(BYTEPTR)utf8skipst((char *)ptr,(char *)ptr+4);
            --len;
        }
        */

        // IF WE GOT HERE WE HAVE A VALID STRING
        BINT newsize=rplObjSize(rplPeekData(1))+rplObjSize(rplPeekData(2));
        WORDPTR newobj=rplAllocTempOb(newsize);
        if(!newobj) return;
        newobj[0]=MKPROLOG(DOTAG,newsize);
        rplCopyObject(newobj+1,rplPeekData(2));
        rplCopyObject(newobj+1+rplObjSize(newobj+1),rplPeekData(1));
        rplOverwriteData(2,newobj);
        rplDropData(1);
        return;
    }


    case DTAG:
    {
        //@SHORT_DESC=Remove a tag from an object

        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStripTagStack(1);

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
    {

            BYTEPTR ptr=(BYTEPTR)TokenStart,endcolon;

            if(*ptr!=':') {
                // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
                // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

            libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
            return;
            }
            // IF WE'VE BEEN HERE BEFORE, THEN WE ARE INSIDE THE TAG ITSELF
            if(CurrentConstruct==(UBINT)MKPROLOG(LIBRARY_NUMBER,0)) {
                // NEED TO MANUALLY COMPILE THE STRING
                BINT nbytes=(BYTEPTR)BlankStart-(BYTEPTR)TokenStart-1;
                BINT lastword=nbytes&3;
                if(lastword) lastword=4-lastword;
                ++ptr;
                rplCompileAppend(MKPROLOG(DOSTRING+lastword,(nbytes+3)>>2));
                while(nbytes>=4) { rplCompileAppend(TEXT2WORD(ptr[0],ptr[1],ptr[2],ptr[3])); ptr+=4; nbytes-=4; }
                if(nbytes==3) rplCompileAppend(TEXT2WORD(ptr[0],ptr[1],ptr[2],0));
                else if(nbytes==2) rplCompileAppend(TEXT2WORD(ptr[0],ptr[1],0,0));
                else if(nbytes==1) rplCompileAppend(TEXT2WORD(ptr[0],0,0,0));

                ptr+=nbytes;
                RetNum=OK_INCARGCOUNT;  // MARK WE ALREADY ADDED THE TAG
                return;

            }

            // WE FOUND A COLON, SEE IF THERE'S TWO
            endcolon=ptr+1;
            while( (endcolon<(BYTEPTR)BlankStart)&&(*endcolon!=':')) ++endcolon;
            if((endcolon>((BYTEPTR)BlankStart)-1) || (endcolon==ptr+1)) { RetNum=ERR_NOTMINE; return; }   // WE MUST HAVE 2 COLONS AT LEAST, AND SOMETHING ELSE AFTER THE LAST COLON

            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER,0));
            if(endcolon<((BYTEPTR)BlankStart)-1) NextTokenStart=(WORDPTR)(endcolon+1);
            BlankStart=(WORDPTR)endcolon;
            RetNum=OK_STARTCONSTRUCT_SPLITTOKEN;
            return;

    }
    //case OPCODE_COMPILECONT:

    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors
        if(ISPROLOG(*DecompileObject)) {

            rplDecompAppendChar(':');
            rplDecompAppendString2((BYTEPTR)(DecompileObject+2),rplStrSize(DecompileObject+1));
            rplDecompAppendChar(':');
            DecompileObject++;      // POINT TO THE STRING, IT WILL SKIP IT

            RetNum=OK_CONTINUE;
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

        if(LIBNUM(CurrentConstruct)==LIBRARY_NUMBER) RetNum=OK_ENDCONSTRUCT;
        else RetNum=OK_CONTINUE;
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

        // PROBE LIBRARY COMMANDS FIRST

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
        // LIBBRARY RETURNS: ObjectID=new ID, ObjectIDHash=hash, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER,(WORDPTR *)ROMPTR_TABLE,ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID, ObjectIDHash=hash
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID,ObjectIDHash);
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID


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

