/*
 * Copyright (c) 2016, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "ui.h"

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************



// REPLACE THE NUMBER
#define LIBRARY_NUMBER  80


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    ECMD(TOSYSBITMAP,"→SYSBITMAP",MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2))

#define ERROR_LIST \
    ERR(BITMAPEXPECTED,0), \
    ERR(UNSUPPORTEDBITMAP,1), \
    ERR(INVALIDCHECKSUM,2), \
    ERR(UNEXPECTEDENDOFDATA,3)



// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS \
    LIBRARY_NUMBER, \
    LIBRARY_NUMBER+1, \
    LIBRARY_NUMBER+2, \
    LIBRARY_NUMBER+3, \
    LIBRARY_NUMBER+4, \
    LIBRARY_NUMBER+5, \
    LIBRARY_NUMBER+6, \
    LIBRARY_NUMBER+7



// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************


INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib80_menu);


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib80_menu,

    0
};

const char * const bitmap_modes[]={
    "MONO",
    "16GR",
    "256G",
    "64KC",
    "ARGB",
    "OTHR",
    "INVA",
    "INVA"
};

// CONVERT RGB 0-255 TO GRAY 0-255 PER BT.709 HDTV FORMULA FOR LUMINANCE

#define LUMINANCE(r,g,b) ((((r)*55+(g)*183+(b)*18)+128)>>8)
#define RGB5TO8(comp) (((BINT)(comp)*2106)>>8)
#define RGB6TO8(comp) (((BINT)(comp)*1036)>>8)


// CONVERT A BITMAP FROM ANY FORMAT INTO THE DEFAULT DISPLAY FORMAT

WORDPTR rplBmpToDisplay(WORDPTR bitmap)
{
    if(!ISBITMAP(*bitmap)) {
        rplError(ERR_BITMAPEXPECTED);
        return 0;
    }


    BINT type=LIBNUM(*bitmap)&7;

    if(type==DEFAULTBITMAPMODE) return bitmap;  // NO CONVERSION NEEEDED

    BINT width=(BINT)bitmap[1];
    BINT height=(BINT)bitmap[2];

    BINT totalsize=(width*height*DEFAULTBITSPERPIXEL)+31;

    totalsize>>=5;  // BITMAP SIZE IN WORDS


    WORDPTR newbmp=rplAllocTempOb(totalsize+2);
    if(!newbmp) return 0;

    BINT npixels=width*height;



    // THIS IS FOR THE 50G HARDWARE, BUT FUTURE-PROOF FOR 16-BIT COLOR DISPLAYS AS WELL

#if DEFAULTBITMAPMODE == BITMAP_RAW16G
    switch(type)
    {
    case BITMAP_RAWMONO:

    {
        BYTEPTR destptr,srcptr;

        BINT mask=1,destmask=0;
        BINT pixel;

        srcptr=(BYTEPTR)(bitmap+3);
        destptr=(BYTEPTR)(newbmp+3);

    while(npixels) {

        // READ A PIXEL FROM SOURCE
        pixel=*srcptr&mask;
        // CONVERT TO PROPER FORMAT
        if(pixel) pixel=0xf; else pixel=0;

        // WRITE TO DESTINATION
        if(!destmask) *destptr=(BYTE)pixel;
        else *destptr|=(BYTE)(pixel<<4);

        //INCREASE SOURCE POINTER
        mask<<=1;
        if(mask>128) { ++srcptr; mask>>=8; }

        // INCREASE DEST PTR
        destmask^=1;
        if(!destmask) ++destptr;

        --npixels;
    }
    break;


    }

    case BITMAP_RAW16G:
    break;
    case BITMAP_RAW256G:

    {

        BYTEPTR destptr,srcptr;

        BINT destmask=0;
        BINT pixel;

        srcptr=(BYTEPTR)(bitmap+3);
        destptr=(BYTEPTR)(newbmp+3);

    while(npixels) {

        // READ A PIXEL FROM SOURCE
        pixel=*srcptr;
        // CONVERT TO PROPER FORMAT
        pixel=((255-pixel)+128)>>4;

        // WRITE TO DESTINATION
        if(!destmask) *destptr=(BYTE)pixel;
        else *destptr|=(BYTE)(pixel<<4);

        //INCREASE SOURCE POINTER
        ++srcptr;

        // INCREASE DEST PTR
        destmask^=1;
        if(!destmask) ++destptr;

        --npixels;
    }
    break;
    }
    case BITMAP_RAW64KC:

    {

        BYTEPTR destptr,srcptr;

        BINT destmask=0;
        BINT pixel;

        srcptr=(BYTEPTR)(bitmap+3);
        destptr=(BYTEPTR)(newbmp+3);

    while(npixels) {

        // READ A PIXEL FROM SOURCE
        pixel=srcptr[0]+256*srcptr[1];

        // CONVERT TO PROPER FORMAT
        pixel=LUMINANCE(RGB5TO8(pixel>>11),RGB6TO8((pixel>>5)&0x3f),RGB5TO8(pixel&0x1f));
        pixel=((255-pixel)+128)>>4;

        // WRITE TO DESTINATION
        if(!destmask) *destptr=(BYTE)pixel;
        else *destptr|=(BYTE)(pixel<<4);

        //INCREASE SOURCE POINTER
        srcptr+=2;

        // INCREASE DEST PTR
        destmask^=1;
        if(!destmask) ++destptr;

        --npixels;
    }
    break;
    }

    case BITMAP_RAWARGB:

    {

        BYTEPTR destptr,srcptr;

        BINT destmask=0;
        BINT pixel;

        srcptr=(BYTEPTR)(bitmap+3);
        destptr=(BYTEPTR)(newbmp+3);

    while(npixels) {

        // READ A PIXEL FROM SOURCE
        //pixel=srcptr[0]+256*srcptr[1];

        // CONVERT TO PROPER FORMAT
        pixel=LUMINANCE(srcptr[2],srcptr[1],srcptr[0]);
        pixel=((255-pixel)+128)>>4;

        // WRITE TO DESTINATION
        if(!destmask) *destptr=(BYTE)pixel;
        else *destptr|=(BYTE)(pixel<<4);

        //INCREASE SOURCE POINTER
        srcptr+=4;

        // INCREASE DEST PTR
        destmask^=1;
        if(!destmask) ++destptr;

        --npixels;
    }
    break;
    }



    }
#endif

    // ALL PIXELS CONVERTED

    return newbmp;

}


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
    {
        if((TokenLen==10) && (!utf8ncmp((char *)TokenStart,"BITMAPDATA",7))) {

            ScratchPointer4=CompileEnd;
            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER,0));
            RetNum=OK_NEEDMORE;
            return;
        }


            // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
            // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;
    }
    case OPCODE_COMPILECONT:
    {
        if((LIBNUM(*ScratchPointer4)&~7)!=LIBRARY_NUMBER) {
            // SOMETHING BAD HAPPENED, THERE'S NO BMPDATA HEADER
            RetNum=ERR_SYNTAX;
            return;
        }

        if(!(*ScratchPointer4&0x10000)) {
            // NEED TO INPUT THE BITMAP TYPE IN 4-LETTERS: MONO,16GR,256G,64KC,ARGB,OTHR
            if(((BINT)TokenLen!=(BYTEPTR)BlankStart-(BYTEPTR)TokenStart)||(TokenLen!=4)) {
                // THERE'S UNICODE CHARACTERS IN BETWEEN, THAT MAKES IT AN INVALID STRING
                // OR THERE'S NOT 4 CHARACTERS
                rplError(ERR_UNSUPPORTEDBITMAP);
                RetNum=ERR_SYNTAX;
                return;
            }
            if(!utf8ncmp((char *)TokenStart,"MONO",4)) *ScratchPointer4|=BITMAP_RAWMONO|0x10000;
            else if(!utf8ncmp((char *)TokenStart,"16GR",4)) *ScratchPointer4|=BITMAP_RAW16G|0x10000;
            else if(!utf8ncmp((char *)TokenStart,"256G",4)) *ScratchPointer4|=BITMAP_RAW256G|0x10000;
            else if(!utf8ncmp((char *)TokenStart,"64KC",4)) *ScratchPointer4|=BITMAP_RAW64KC|0x10000;
            else if(!utf8ncmp((char *)TokenStart,"ARGB",4)) *ScratchPointer4|=BITMAP_RAWARGB|0x10000;
            else if(!utf8ncmp((char *)TokenStart,"OTHR",4)) *ScratchPointer4|=BITMAP_EXTERNAL|0x10000;
            else {
                rplError(ERR_UNSUPPORTEDBITMAP);
                RetNum=ERR_SYNTAX;
                return;
            }
            RetNum=OK_NEEDMORE;
            return;
        }

            // HERE WE ALREADY HAVE THE TYPE OF BITMAP



       if(!(*ScratchPointer4&0x20000)) {
                // NEED TO CAPTURE THE WIDTH AND HEIGHT AS INTEGERS
                WORD value=0;
                BINT digit;
                BYTEPTR ptr=(BYTEPTR)TokenStart;

                while(ptr<(BYTEPTR)BlankStart) {
                    if((*ptr>='0')&&(*ptr<='9')) digit=*ptr-'0';
                    else {
                        RetNum=ERR_SYNTAX;
                        return;
                        }
                    value*=10;
                    value+=digit;
                    ++ptr;
                }

               rplCompileAppend(value);

               // IF THERE WERE 2 NUMBERS ALREADY, THEN MARK THE SIZE AS DONE
               if(CompileEnd-ScratchPointer4>=3) *ScratchPointer4|=0x20000;
               RetNum=OK_NEEDMORE;
               return;
       }


       // HERE WE ALREADY HAVE THE BITMAP TYPE, WIDTH AND HEIGHT


        // WE HAVE A SIZE
       WORD totalsize;

       if((*ScratchPointer4&0xff)==BITMAP_EXTERNAL) {
        // THE SIZE IS IN THE FIRST WORD, GET A MINIMAL OF 1 WORDS
        totalsize=CompileEnd-ScratchPointer4-3;
        if(LIBNUM(*ScratchPointer4)&1) totalsize-=2;
        if(totalsize>=1) {
            // WE ALREADY RECOVERED THE SIZE OF THE OBJECT
            totalsize=ScratchPointer4[3];
        }
        else totalsize=0xffffffff;
       }
       else {
        totalsize=ScratchPointer4[1]*ScratchPointer4[2];
        switch(*ScratchPointer4&0xff)
        {
        case BITMAP_RAWMONO:
        break;
        case BITMAP_RAW16G:
            totalsize*=4;
            break;
        case BITMAP_RAW256G:
            totalsize*=8;
            break;
        case BITMAP_RAW64KC:
            totalsize*=16;
            break;
        case BITMAP_RAWARGB:
        default:
            totalsize*=32;
            break;
        }
        totalsize+=31;
        totalsize>>=5;
       }


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
                    rplError(ERR_INVALIDCHECKSUM);
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
           else {
                if((CompileEnd-ScratchPointer4-3)==totalsize) {
                    //   DONE!  FIX THE PROLOG WITH THE RIGHT LIBRARY NUMBER AND SIZE
                    *ScratchPointer4=MKPROLOG(DOBITMAP+(*ScratchPointer4&0xff),totalsize+2);
                    RetNum=OK_CONTINUE;
                    return;
                }
                RetNum=ERR_SYNTAX;
                return;
            }

        // END OF TOKEN, NEED MORE!
        RetNum=OK_NEEDMORE;
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
            // DECOMPILE BITMAP

            rplDecompAppendString((BYTEPTR)"BITMAPDATA ");

            // TYPE
            rplDecompAppendString((BYTEPTR)bitmap_modes[LIBNUM(*DecompileObject)&7]);

            rplDecompAppendChar(' ');

            // SIZE
            BYTE buffer[50];
            BINT len=rplIntToString((BINT64)DecompileObject[1],DECBINT,buffer,buffer+50);

            rplDecompAppendString2(buffer,len);

            rplDecompAppendChar(' ');

            len=rplIntToString((BINT64)DecompileObject[2],DECBINT,buffer,buffer+50);

            rplDecompAppendString2(buffer,len);

            rplDecompAppendChar(' ');

            BINT size=OBJSIZE(*DecompileObject)-2;

            // OUTPUT THE DATA BY WORDS, WITH FOLLOWING ENCODING:
            // 32-BIT WORDS GO ENCODED IN 6 TEXT CHARACTERS
            // EACH CHARACTER CARRIES 6-BITS IN BASE64 ENCONDING
            // MOST SIGNIFICANT 6-BIT PACKET GOES FIRST
            // LAST PACKET HAS 2 LSB BITS TO COMPLETE THE 32-BIT WORDS
            // AND 4-BIT CHECKSUM. THE CHECKSUM IS THE SUM OF THE (16) 2-BIT PACKS IN THE WORD, MODULO 15



            BYTE encoder[7];

            encoder[6]=0;

            WORDPTR ptr=DecompileObject+3;
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

