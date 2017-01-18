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
#define LIBRARY_NUMBER  78


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(SETSTKFONT,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)) \



// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************



// THIS LIBRARY DEPENDS ON THE FONTS INSTALLED IN THE FIRMWARE
// SO IT'S NOT HARDWARE-INDEPENDENT.


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)Font_5A,
    (WORDPTR)Font_5B,
    (WORDPTR)Font_5C,
    (WORDPTR)Font_6A,
    (WORDPTR)Font_6m,
    (WORDPTR)Font_7A,
    (WORDPTR)Font_8A,
    (WORDPTR)Font_8B,
    (WORDPTR)Font_8C,
    (WORDPTR)Font_8D,
    0
};



void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // JUST PUSH THE OBJECT ON THE STACK
        rplPushData(IPtr);
        return;
    }

    switch(OPCODE(CurOpcode))
    {
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

        if((TokenLen==8) && (!utf8ncmp((char *)TokenStart,"FONTDATA",8))) {

            ScratchPointer4=CompileEnd;
            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER,0));
            RetNum=OK_NEEDMORE;
            return;
        }


            // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
            // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        RetNum=ERR_NOTMINE;
        //libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;
    case OPCODE_COMPILECONT:
    {
        if(OBJSIZE(*ScratchPointer4)==0) {
            // NEED TO OBTAIN THE SIZE IN WORDS FIRST
            // GIVEN AS A HEX NUMBER

            if((BINT)TokenLen!=(BYTEPTR)BlankStart-(BYTEPTR)TokenStart) {
                // THERE'S UNICODE CHARACTERS IN BETWEEN, THAT MAKES IT AN INVALID STRING
                RetNum=ERR_SYNTAX;
                return;
            }

            BYTEPTR ptr=(BYTEPTR)TokenStart;
            WORD value=0;
            BINT digit;
            while(ptr<(BYTEPTR)BlankStart) {
                if((*ptr>='0')&&(*ptr<='9')) digit=*ptr-'0';
                else if((*ptr>='A')&&(*ptr<='F')) digit=*ptr-'A';
                    else if((*ptr>='a')&&(*ptr<='f')) digit=*ptr-'a';
                    else {
                    RetNum=ERR_SYNTAX;
                    return;
                    }
                value<<=4;
                value|=digit;
                ++ptr;
            }

            // WE GOT THE PAYLOAD SIZE IN WORDS
            if(value>0x3ffff) {
                RetNum=ERR_INVALID;
                return;
            }

            *ScratchPointer4=MKPROLOG(LIBRARY_NUMBER,value);
            RetNum=OK_NEEDMORE;
            return;

        }

        // WE HAVE A SIZE
        // DO WE NEED ANY MORE DATA?

        BYTEPTR ptr=(BYTEPTR)TokenStart;

        WORD value=0;
        WORD checksum=0;
        BINT ndigits=0;
        BINT dig;

        if(LIBNUM(*ScratchPointer4)&1) {
            // CONTINUE WHERE WE LEFT OFF
            --CompileEnd;
            ndigits=(*CompileEnd)&0xffff;
            checksum=(*CompileEnd)>>16;
            --CompileEnd;
            value=*CompileEnd;
            *ScratchPointer4&=~0x00100000;
        }

        while((CompileEnd-ScratchPointer4-1)<(BINT)OBJSIZE(*ScratchPointer4))
        {
            do {
                if((*ptr>='0')&&(*ptr<='9')) dig=(*ptr+4);
                else if((*ptr>='A')&&(*ptr<='Z')) dig=(*ptr-65);
                else if((*ptr>='a')&&(*ptr<='z')) dig=(*ptr-71);
                else if(*ptr=='+') dig=62;
                else if(*ptr=='/') dig=63;
                else {
                    // INVALID CHARACTER!
                    RetNum=ERR_SYNTAX;
                    return;
                }

            // STILL NEED MORE WORDS, KEEP COMPILING
            if(ndigits==5) {
                value<<=2;


                value|=dig&3;
                checksum+=dig&3;
                if((checksum&0xf)!=((dig>>2)&0xf)) {
                    RetNum=ERR_INVALID;
                    return;
                }
                // CHECKSUM PASSED, IT'S A VALID WORD
                rplCompileAppend(value);
                value=0;
                ndigits=0;
                checksum=0;
            }
            else {
            value<<=6;
            value|=dig;
            checksum+=(dig&3)+((dig>>2)&3)+((dig>>4)&3);
            ++ndigits;
            }
            ++ptr;
            } while(ptr!=(BYTEPTR)BlankStart);
            if(ndigits) {
                // INCOMPLETE WORD, PREPARE FOR RESUME ON NEXT TOKEN
                rplCompileAppend(value);
                rplCompileAppend(ndigits | (checksum<<16));
                *ScratchPointer4|=0x00100000;
            }


        }

        RetNum=OK_CONTINUE;
        return;
     }
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors
        if(ISPROLOG(*DecompileObject)) {
            // DECOMPILE FONT

            rplDecompAppendString((BYTEPTR)"FONTDATA ");
            BINT size=OBJSIZE(*DecompileObject);
            BINT k,zero=1,nibble;
            for(k=4;k>=0;--k) {
                nibble= (size>>(k*4))&0xf;
                if(!zero || nibble) {
                    nibble+=48;
                    if(nibble>=58) nibble+=7;
                    rplDecompAppendChar(nibble);
                    zero=0;
                }
            }

            rplDecompAppendChar(' ');


            // OUTPUT THE DATA BY WORDS, WITH FOLLOWING ENCODING:
            // 32-BIT WORDS GO ENCODED IN 6 TEXT CHARACTERS
            // EACH CHARACTER CARRIES 6-BITS IN BASE64 ENCONDING
            // MOST SIGNIFICANT 6-BIT PACKET GOES FIRST
            // LAST PACKET HAS 2 LSB BITS TO COMPLETE THE 32-BIT WORDS
            // AND 4-BIT CHECKSUM. THE CHECKSUM IS THE SUM OF THE (16) 2-BIT PACKS IN THE WORD, MODULO 15

            BYTE encoder[7];

            encoder[6]=0;

            WORDPTR ptr=DecompileObject+1;
            BINT nwords=0;

            while(size) {
                // ENCODE THE 6 CHARACTERS
                int k;
                BINT chksum=0;
                for(k=0;k<5;++k) { encoder[k]=((*ptr)>>(26-6*k))&0x3f; chksum+=(encoder[k]&3)+((encoder[k]>>2)&3)+((encoder[k]>>4)&3); }
                encoder[5]=(*ptr)&3;
                chksum+=*ptr&3;
                encoder[5]|=(chksum&0xf)<<2;

                // NOW CONVERT TO BASE64
                for(k=0;k<6;++k)
                {
                    if(encoder[k]<26) encoder[k]+=65;
                    else if(encoder[k]<52) encoder[k]+=71;
                    else if(encoder[k]<62) encoder[k]-=4;
                    else if(encoder[k]==62) encoder[k]='+';
                    else encoder[k]='/';
                }

                rplDecompAppendString(encoder);
                if(Exceptions) {
                    RetNum=ERR_INVALID;
                    return;
                }

                ++nwords;
                if(nwords==8) { rplDecompAppendChar(' '); nwords=0; }

                --size;

            }

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
