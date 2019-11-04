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
    ECMD(MKTAG,"→TAG" MKTOKENINFO(4,TITYPE_NOTALLOWED,2,2)), \
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

            //  THIS SHOULDN'T HAPPEN, TAGS SHOULD BE IGNORED BY OVERLOADED OPERATORS
            rplError(ERR_INVALIDOPCODE);
            return;


        }

    }   // END OF OVERLOADABLE OPERATORS



    switch(OPCODE(CurOpcode))
    {
    case MKTAG:
    {
        //@SHORT_DESC=Apply a tag to an object

        return;
    }


    case DTAG:
    {
        //@SHORT_DESC=Remove a tag from an object

        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplOverwriteData(1,rplSkipOb(rplPeekData(1)+1));

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

            if(*ptr!=':') { RetNum=ERR_NOTMINE; return; }
            if(CurrentConstruct==MKPROLOG(LIBRARY_NUMBER,0)) {
                // NEED TO ADD THE STRING
                BINT nbytes=(BYTEPTR)BlankStart-(BYTEPTR)TokenStart-1;
                BINT lastword=nbytes&3;
                if(lastword) lastword=4-lastword;
                rplCompileAppend(MKPROLOG(DOSTRING+lastword,nbytes>>2);
                // TODO: KEEP GOING FROM HERE



            }
            endcolon=ptr+1;
            while( (endcolon<(BYTEPTR)BlankStart)&&(*endcolon!=':')) ++endcolon;
            if(endcolon>=((BYTEPTR)BlankStart)-1) { RetNum=ERR_NOTMINE; return; }   // WE MUST HAVE 2 COLONS AT LEAST, AND SOMETHING ELSE AFTER THE LAST COLON

            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER,0));
            NextTokenStart=endcolon+1;
            BlankStart=endcolon;
            RetNum=OK_STARTCONSTRUCT_SPLITTOKEN;
            return;

            // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
            // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;
    }
    case OPCODE_COMPILECONT:
    {
        if(LIBNUM(*ScratchPointer4)==DOLIBPTR) {
            // COMPILE A ROMPOINTER WITH A MISSING LIBRARY
            BYTEPTR ptr=(BYTEPTR)TokenStart;
            int rot=0;
            WORD libid=0,libcmd;
            BINT cp;
            while(ptr<(BYTEPTR)BlankStart) {
                if(*ptr=='.') break;
                cp=utf82cp((char *)ptr,(char *)BlankStart);
                if( ((cp>='A')&&(cp<='Z')) || ((cp>='a')&&(cp<='z')) || ((cp>='0')&&(cp<='9'))) {
                    libid|=cp<<rot;
                    rot+=8;
                    if(rot>=32) {
                        ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)BlankStart);
                        break;
                    }
                }
                else {
                    RetNum=ERR_SYNTAX;
                    return;
                }

                ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)BlankStart);
            }

            libcmd=0;
            while(ptr<(BYTEPTR)BlankStart) {
                if(*ptr=='.') { ++ptr; continue; }
                cp=*ptr-'0';
                if( (cp<0)||(cp>9)) {
                    RetNum=ERR_SYNTAX;
                    return;
                }
                libcmd*=10;
                libcmd+=cp;
                ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)BlankStart);
            }

            rplCompileAppend(libid);
            rplCompileAppend(libcmd);
            RetNum=OK_CONTINUE;
            return;
        }




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
                else if((*ptr>='A')&&(*ptr<='F')) digit=*ptr-'A'+10;
                    else if((*ptr>='a')&&(*ptr<='f')) digit=*ptr-'a'+10;
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

            *ScratchPointer4=MKPROLOG(0,value);
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

        while(((CompileEnd-ScratchPointer4-1)<(BINT)OBJSIZE(*ScratchPointer4)))
        {
             do {
                if((*ptr>='0')&&(*ptr<='9')) dig=(*ptr+4);
                else if((*ptr>='A')&&(*ptr<='Z')) dig=(*ptr-65);
                else if((*ptr>='a')&&(*ptr<='z')) dig=(*ptr-71);
                else if(*ptr=='#') dig=62;
                else if(*ptr=='$') dig=63;
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
            if(ndigits || (((CompileEnd-ScratchPointer4-1)<(BINT)OBJSIZE(*ScratchPointer4)))) {
                // INCOMPLETE WORD, PREPARE FOR RESUME ON NEXT TOKEN
                rplCompileAppend(value);
                rplCompileAppend(ndigits | (checksum<<16));
                *ScratchPointer4|=0x00100000;
                RetNum=OK_NEEDMORE;
                return;
            }
            else *ScratchPointer4=MKPROLOG(DOLIBRARY,OBJSIZE(*ScratchPointer4));


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
            if(ISLIBPTR(*DecompileObject)) {
                // DECOMPILE A LIBPTR

                WORDPTR name=rplGetLibPtrName(DecompileObject);

                if(!name || (*name==CMD_NULLLAM)) {
                    // LIBPTRS WITHOUT A PROPER LIBRARY INSTALLED
                    rplDecompAppendString((BYTEPTR)"LIBPTR ");

                    BYTEPTR ptr=(BYTEPTR)(DecompileObject+1);
                    if(*ptr!=0) rplDecompAppendChar(*ptr);
                    ++ptr;
                    if(*ptr!=0) rplDecompAppendChar(*ptr);
                    ++ptr;
                    if(*ptr!=0) rplDecompAppendChar(*ptr);
                    ++ptr;
                    if(*ptr!=0) rplDecompAppendChar(*ptr);
                    ++ptr;
                    rplDecompAppendChar('.');
                    BYTE buffer[22];
                    BINT n=rplIntToString(DecompileObject[2],DECBINT,buffer,buffer+22);
                    ptr=buffer;
                    while(n--) { rplDecompAppendChar(*ptr); ++ptr; }


                }
                else {
                // NAMED COMMAND
                rplDecompAppendString2((BYTEPTR)(name+1),rplGetIdentLength(name));
                }

                RetNum=OK_CONTINUE;
                return;

            }


            // DECOMPILE LIBRARY

            rplDecompAppendString((BYTEPTR)"LIBRARY ");
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
                    else if(encoder[k]==62) encoder[k]='#';
                    else encoder[k]='$';
                }

                ScratchPointer1=ptr;
                rplDecompAppendString(encoder);
                if(Exceptions) {
                    RetNum=ERR_INVALID;
                    return;
                }
                ptr=ScratchPointer1;

                ++ptr;

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

        // PROBE LIBRARY COMMANDS FIRST
        WORDPTR cmdinfo;
        BINT64 libptr=rplProbeLibPtrIndex((BYTEPTR)TokenStart,(BYTEPTR)BlankStart,&cmdinfo);

        if(libptr>=0) {
            // FOUND A MATCH!
            BINT len=utf8nlenst((char *)(cmdinfo+1),((char *)(cmdinfo+1)) + rplGetIdentLength(cmdinfo));
            BINT nargs=OPCODE(*rplSkipOb(cmdinfo));
            BINT allow=nargs&1;

            nargs>>=8;

            if(allow) RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_IDENT,nargs,2);
            else RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_NOTALLOWED,nargs,1);
            return;
        }

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
        TypeInfo=LIBNUM(*ObjectPTR)*100;
        DecompHints=0;

        if(ISLIBPTR(*ObjectPTR)) {

                // FOUND A MATCH!
                WORDPTR cmdinfo=rplGetLibPtrName(ObjectPTR);
                if(cmdinfo) {
                BINT nargs=OPCODE(*rplSkipOb(cmdinfo));
                BINT allow=nargs&1;
                BINT len;

                if(ISIDENT(*cmdinfo)) len=utf8nlenst((char *)(cmdinfo+1),(char *)(cmdinfo+1)+rplGetIdentLength(cmdinfo));

                nargs>>=8;

                if(allow) RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_IDENT,nargs,2);
                else RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_NOTALLOWED,nargs,1);
                return;
                }

        }


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


    {
        // AUTOCOMPLETE NAMES OF LIBRARIES
        // TokenStart = token string
        // TokenLen = token length
        // SuggestedOpcode = OPCODE OF THE CURRENT SUGGESTION, OR THE PROLOG OF THE OBJECT IF SUGGESTION IS AN OBJECT
        // SuggestedObject = POINTER TO AN OBJECT (ONLY VALID IF ISPROLOG(SuggestedOpcode)==True)


        // AUTOMCOMPLETE FIRST COMMANDS OF INSTALLED LIBRARIES

        WORDPTR libdir=rplGetSettings((WORDPTR)library_dirname);

        if(libdir) {

        WORD prevlibid=0;
        BINT previdx=0;
        if(ISPROLOG(SuggestedOpcode) && SuggestedObject) {
            if(ISLIBPTR(SuggestedOpcode)) {
                prevlibid=SuggestedObject[1];
                previdx=SuggestedObject[2];
            }
        }
        WORDPTR *direntry=rplFindFirstByHandle(libdir);

        if(direntry) {

            do {

                if(ISIDENT(*direntry[0]) && (OBJSIZE(*direntry[0])==1)) {

                    BINT nentries=OPCODE(direntry[1][3]);
                    // COMPARE LIBRARY ID
                    if(prevlibid)
                    {
                        // SKIP UNTIL WE FIND THE PREVIOUS LIBRARY
                        if(direntry[0][1]!=prevlibid) continue;
                        // FOUND IT, previdx HAS THE START INDEX
                        prevlibid=0;
                        if(previdx<=4) continue;    // THE PREVIOUS COMMAND WAS THE FIRST COMMAND, SKIP TO THE NEXT LIBRARY
                    } else previdx=nentries;

                    if(previdx>nentries) previdx=nentries;
                    WORDPTR nameptr;
                    do {
                        --previdx;
                            nameptr=rplSkipOb(direntry[1]+OPCODE(direntry[1][previdx+3]));
                            if(ISIDENT(*nameptr)) {
                                // COMPARE IDENT WITH THE GIVEN TOKEN
                                BINT len,idlen=rplGetIdentLength(nameptr);    // LENGTH IN BYTES
                                len=utf8nlen((char *)(nameptr+1),(char *)(nameptr+1)+idlen);  // LENGTH IN UNICODE CHARACTERS
                                if((len>=(BINT)TokenLen) && (!utf8ncmp2((char *)TokenStart,(char *)BlankStart,(char *)(nameptr+1),TokenLen))) {
                                    // WE HAVE A MATCH!
                                    // CREATE A NEW LIBPTR AND RETURN IT
                                    WORDPTR newobj=rplAllocTempOb(2);
                                    if(!newobj) { RetNum=ERR_NOTMINE; return; }

                                    newobj[0]=MKPROLOG(DOLIBPTR,2);
                                    newobj[1]=direntry[0][1];
                                    newobj[2]=previdx;


                                    RetNum=OK_CONTINUE;
                                    SuggestedObject=newobj;
                                    SuggestedOpcode=newobj[0];
                                    return;
                                }
                                BINT firstchar=utf82cp((char *)(nameptr+1),(char *)(nameptr+1)+idlen);

                                // CHECK FOR NON-STANDARD STARTING CHARACTERS
                                if( !(
                                            ((firstchar>='A') && (firstchar<='Z')) ||
                                            ((firstchar>='a') && (firstchar<='z')) )
                                        )
                                {
                                // SKIP THE FIRST CHARACTER AND CHECK AGAIN
                                    --len;
                                    if((len>=(BINT)TokenLen) && (!utf8ncmp2((char *)TokenStart,(char *)BlankStart,utf8skipst((char *)(nameptr+1),(char *)(nameptr+1)+4),TokenLen)))
                                    {
                                        // WE HAVE A MATCH!
                                        // CREATE A NEW LIBPTR AND RETURN IT
                                        WORDPTR newobj=rplAllocTempOb(2);
                                        if(!newobj) { RetNum=ERR_NOTMINE; return; }

                                        newobj[0]=MKPROLOG(DOLIBPTR,2);
                                        newobj[1]=direntry[0][1];
                                        newobj[2]=previdx;


                                        RetNum=OK_CONTINUE;
                                        SuggestedObject=newobj;
                                        SuggestedOpcode=newobj[0];
                                        return;
                                    }

                                }
                                // THERE WAS NO MATCH, DO THE NEXT NAME
                        }
                    } while(previdx>4);

                    // NO MATCHES ON THIS LIBRARY

                }

            } while((direntry=rplFindNext(direntry)));

            // NO MATCHES ON ANY LIBRARY

        }

        }

        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        return;
        }

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg (LOW WORD)
        // AND IN ArgNum2 (HI WORD)
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(LIBNUM(MenuCodeArg)==DOLIBRARY) {
            ObjectPTR=(WORDPTR)lib102_menu;
            RetNum=OK_CONTINUE;
            return;
        }
        WORDPTR libmenu=rplGetLibPtr2(ArgNum2,2);

        if(!libmenu) { RetNum=ERR_NOTMINE; return; }

        ObjectPTR=libmenu;
        RetNum=OK_CONTINUE;
       return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // IF THE OPCODE IS FROM A USER LIBRARY, LIBID IS IN ArgNum2
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(LIBNUM(CmdHelp)==DOLIBPTR) {
            // RETURN THE HELP FOR THAT COMMAND
            WORDPTR help=rplGetLibPtrHelp(ArgNum2,OPCODE(CmdHelp));
            if(!help) RetNum=ERR_NOTMINE;
            else {
                RetNum=OK_CONTINUE;
                ObjectPTR=help;
            }
            return;
        }
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

