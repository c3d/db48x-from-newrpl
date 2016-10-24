/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
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
#define LIBRARY_NUMBER  24


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(CHR,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(NUM,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TOSTR,"→STR",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(FROMSTR,"STR→",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(SREV,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(NTOKENS,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(NTHTOKEN,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(NTHTOKENPOS,MKTOKENINFO(11,TITYPE_NOTALLOWED,1,2)), \
    CMD(TRIM,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(RTRIM,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2))


// ADD MORE OPCODES HERE


#define ERROR_LIST \
    ERR(STRINGEXPECTED,0), \
    ERR(INVALIDCODEPOINT,1)


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER,LIBRARY_NUMBER+1,LIBRARY_NUMBER+2,LIBRARY_NUMBER+3


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************












// GET THE LENGTH OF A STRING FROM ITS PROLOG
#define STRLEN(prolog) ((OBJSIZE(prolog)<<2)-(LIBNUM(prolog)&3))

ROMOBJECT empty_string[] = {
    MKPROLOG(DOSTRING,0)
};


INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib24_menu);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib24_menu,
    (WORDPTR)empty_string,
    0
};





// COMPUTE THE STRING LENGTH IN CHARACTERS
BINT rplStrLen(WORDPTR string)
{
    if(ISSTRING(*string))  {
        BINT len=STRLEN(*string);
        BYTEPTR start=(BYTEPTR)(string+1);
        return utf8nlen((char *)start,(char *)(start+len));
    }
    return 0;
}

// COMPUTE THE STRING LENGTH IN BYTES
BINT rplStrSize(WORDPTR string)
{
    if(ISSTRING(*string))  return STRLEN(*string);
    return 0;
}




// FIX THE PROLOG OF A STRING TO MATCH THE DESIRED LENGTH IN BYTES
// LOW-LEVEL FUNCTION, DOES NOT ACTUALLY RESIZE THE OBJECT
void rplSetStringLength(WORDPTR string,BINT length)
{
    BINT padding=(4-((length)&3))&3;

    *string=MKPROLOG(DOSTRING+padding,(length+3)>>2);
}


// ADDITIONAL API TO WORK WITH STRINGS FROM OTHER LIBRARIES

// RETURN AN OFFSET TO THE START OF THE REQUESTED LINE
// IF LINE<1 OR LINE>NUMBER OF LINES IN THE STRING, RETURNS -1
//
BINT rplStringGetLinePtr(WORDPTR str,BINT line)
{
    if(!ISSTRING(*str)) return -1;
    BYTEPTR start=(BYTEPTR) (str+1),ptr;
    BINT len=STRLEN(*str);
    BINT count=1;

    ptr=start;
    while(count<line) {
        while((ptr-start<len) && (*ptr!='\n')) ++ptr;
        if(ptr-start>=len) return -1;
        ++count;
        ++ptr;

    }

    if(ptr-start>=len) return -1;   // THIS CAN ONLY HAPPEN ON STRINGS TERMINATED IN A NEWLINE WHEN THE NEXT LINE IS REQUESTED
    return ptr-start;
}

BINT rplStringGetNextLine(WORDPTR str,BINT prevlineoff)
{
    if(!ISSTRING(*str)) return -1;
    BYTEPTR start=(BYTEPTR) (str+1),ptr;
    BINT len=STRLEN(*str);

    ptr=start+prevlineoff;
    while((ptr-start<len) && (*ptr!='\n')) ++ptr;

    if(*ptr=='\n') ++ptr;

    if(ptr-start>len) return -1;

    return ptr-start;
}


BINT rplStringCountLines(WORDPTR str)
{
    if(!ISSTRING(*str)) return -1;
    BYTEPTR start=(BYTEPTR) (str+1),ptr;
    BINT len=STRLEN(*str);
    BINT count=1;

    ptr=start;
    while(ptr-start<len) {
        if(*ptr=='\n') ++count;
        ++ptr;
    }

    return count;

}

// CREATE A NEW STRING OBEJCT AND RETURN ITS ADDRESS
// RETURNS NULL IF ANY ERRORS
WORDPTR rplCreateString(BYTEPTR text,BYTEPTR textend)
{
    BINT lenbytes=textend-text;
    BINT len=(lenbytes+3)>>2;
    if(lenbytes<0) return 0;
    WORDPTR newstring=rplAllocTempOb(len);
    if(newstring) {
        BYTEPTR ptr=(BYTEPTR) (newstring+1);
        while(text!=textend) *ptr++=*text++;
        while(((PTR2NUMBER)ptr)&3) *ptr++=0;

        rplSetStringLength(newstring,lenbytes);

        return newstring;
    }
    return 0;
}


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        // NORMAL BEHAVIOR IS TO PUSH THE OBJECT ON THE STACK:

        rplPushData(IPtr);
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case CHR:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        BINT nitems;
        WORDPTR list,item;

        if(ISLIST(*rplPeekData(1))) {
            list=rplPeekData(1);
            // CONVERT A LIST OF UNICODE CODE POINTS INTO UTF8 STRING
            nitems=rplListLengthFlat(list);
            item=rplGetListElementFlat(list,1);
        } else {

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_BADARGTYPE);
            return;
        }
        nitems=1;
        item=rplPeekData(1);
        list=0;
        }

        WORDPTR newstring=rplAllocTempOb(nitems);   // ALLOCATE 4 BYTES PER CHARACTER, TRUNCATE THE OBJECT LATER
        if(!newstring)
            return;

        BYTEPTR strptr=(BYTEPTR) (newstring+1);    // START OF NEW STRING AFTER THE PROLOG

        BINT64 ucode;
        WORD utfchar;
        BINT len;

        while(nitems--) {

        ucode=rplReadNumberAsBINT(item);
        if(Exceptions) {
            rplTruncateLastObject(newstring);       // COMPLETELY REMOVE THE OBJECT
            return;
        }


        utfchar=cp2utf8((UBINT)ucode);

        if(utfchar==(WORD)-1) {
            rplError(ERR_INVALIDCODEPOINT);
            rplTruncateLastObject(newstring);       // COMPLETELY REMOVE THE OBJECT
            return;
        }

        len= (utfchar&0xffff0000)? ((utfchar&0xff000000)? 4:3) : ((utfchar&0xff00)? 2:1);
        while(len--) { *strptr=utfchar&0xff; ++strptr; utfchar>>=8; }

        if(nitems) {
            item=rplGetNextListElementFlat(list,item);
            if(!item) break;
        }
        }

        // DONE ENCODING UTF-8 STRING

        len = strptr- (BYTEPTR)(newstring+1);
        rplSetStringLength(newstring,len);

        // TRIM UNUSED MEMORY
        rplTruncateLastObject(rplSkipOb(newstring));

        rplOverwriteData(1,newstring);
        return;
    }
    case NUM:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISSTRING(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        if(rplStrLen(rplPeekData(1))<1) {
            rplOverwriteData(1,(WORDPTR)zero_bint);
            return;
        }
        BYTEPTR string=(BYTEPTR) (rplPeekData(1)+1);

        BINT utfchar=utf82cp((char *) string,(char *)(string+4));

        rplNewBINTPush(utfchar,HEXBINT);
        if(Exceptions) return;
        rplOverwriteData(2,rplPeekData(1));
        rplDropData(1);
        return;

    }
    case TOSTR:
        // VERY IMPORTANT: DECOMPILE FUNCTION
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

     WORDPTR string=rplDecompile(rplPeekData(1),0);
     if(!string) { ExceptionPointer=IPtr; return; }   // THERE WAS AN ERROR, TAKE OWNERSHIP OF IT
     rplOverwriteData(1,string);
    }
        return;

    case FROMSTR:
        // COMPILER FUNCTION, FOR STR-> AND ->OBJ COMMANDS
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR string=rplPeekData(1);
        if(!ISSTRING(*string)) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }
        BINT length=STRLEN(*string);
        WORDPTR newobj=rplCompile((BYTEPTR)(string+1),length,1);

        if(!newobj) { ExceptionPointer=IPtr; return; }   // THERE WAS AN ERROR, TAKE OWNERSHIP OF IT

        rplDropData(1);
        rplPushRet(IPtr);   // PUSH RETURN ADDRESS

        IPtr=newobj;
        CurOpcode=0;        // TRANSFER CONTROL TO THE NEW SECONDARY
        return;
    }


    // ADD MORE OPCODES HERE

    case OVR_ADD:
        // APPEND TWO STRINGS
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        ScratchPointer1=rplPeekData(2);
        ScratchPointer2=rplPeekData(1);

        if(!ISSTRING(*ScratchPointer1)) {
        WORDPTR string=rplDecompile(ScratchPointer1,0);
        if(!string) { ExceptionPointer=IPtr; return; }   // THERE WAS AN ERROR, TAKE OWNERSHIP OF IT
        ScratchPointer1=string;
        }

        if(!ISSTRING(*ScratchPointer2)) {
        WORDPTR string=rplDecompile(ScratchPointer2,0);
        if(!string) { ExceptionPointer=IPtr; return; }   // THERE WAS AN ERROR, TAKE OWNERSHIP OF IT
        ScratchPointer2=string;
        }

        BINT len1=STRLEN(*ScratchPointer1);
        BINT len2=STRLEN(*ScratchPointer2);

        WORDPTR newobject=rplAllocTempOb((len1+len2+3)>>2);
        if(!newobject) {
            rplException(EX_OUTOFMEM);
           return;
        }
        // COPY BOTH STRINGS
        memmoveb(newobject+1,ScratchPointer1+1,len1);
        memmoveb( ((BYTEPTR)newobject)+len1+4,ScratchPointer2+1,len2);

        BINT padding=(4-((len1+len2)&3))&3;

        *newobject=MKPROLOG(DOSTRING+padding,(len1+len2+3)>>2);

        rplOverwriteData(2,newobject);
        rplDropData(1);

    }
    return;

    case OVR_EVAL:
    case OVR_EVAL1:
    case OVR_XEQ:
        // JUST LEAVE THE OBJECT ON THE STACK WHERE IT IS, UNLESS IT'S A COMMAND DEFINED BY THIS LIBRARY
        if(!ISPROLOG(*rplPeekData(1))) {
        WORD saveOpcode=CurOpcode;
        CurOpcode=*rplPopData();
        // RECURSIVE CALL
        LIB_HANDLER();
        CurOpcode=saveOpcode;
        return;
        }
        return;

    case OVR_SAME:
    case OVR_EQ:

        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if( (!ISSTRING(*rplPeekData(2))) || (!ISSTRING(*rplPeekData(1)))) {
            rplOverwriteData(2,(WORDPTR)zero_bint);
            rplDropData(1);
            return;
        }

        if(rplCompareObjects(rplPeekData(1),rplPeekData(2))) rplOverwriteData(2,(WORDPTR)one_bint);
        else rplOverwriteData(2,(WORDPTR)zero_bint);
        rplDropData(1);
        return;


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

        if(*((BYTEPTR)TokenStart)=='\"') {
            // START A STRING


            ScratchPointer4=CompileEnd;     // SAVE CURRENT COMPILER POINTER TO FIX THE OBJECT AT THE END

            rplCompileAppend(MKPROLOG(DOSTRING,0));

            union {
                WORD word;
                BYTE bytes[4];
            } temp;

            BINT count=0;
            BYTEPTR ptr=(BYTEPTR) TokenStart;
            ++ptr;  // SKIP THE QUOTE
            do {
            while(count<4) {
                if(ptr==(BYTEPTR)NextTokenStart) {
                 // WE ARE AT THE END OF THE GIVEN STRING, STILL NO CLOSING QUOTE, SO WE NEED MORE

                    // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                    if(count) rplCompileAppend(temp.word);
                    *ScratchPointer4=MKPROLOG(DOSTRING+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                    RetNum=OK_NEEDMORE;
                    return;
                }
                if(*ptr=='\"') {
                    // END OF STRING!
                    ++ptr;
                    if(ptr!=(BYTEPTR)BlankStart) {
                        // QUOTE IN THE MIDDLE OF THE TOKEN IS A SYNTAX ERROR
                        RetNum=ERR_INVALID;
                        return;
                    }
                    // WE HAVE REACHED THE END OF THE STRING
                    if(count) {
                    rplCompileAppend(temp.word);
                    }
                    *ScratchPointer4=MKPROLOG(DOSTRING+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                    RetNum=OK_CONTINUE;
                    return;
                }
                if(count==0) temp.word=0;
                temp.bytes[count]=*ptr;
                ++count;
                ++ptr;
                }
                //  WE HAVE A COMPLETE WORD HERE
                ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                rplCompileAppend(temp.word);
                ptr=(BYTEPTR)ScratchPointer1;

                count=0;


            } while(1);     // DANGEROUS! BUT WE WILL RETURN FROM THE CHECK WITHIN THE INNER LOOP;
            //  THIS IS UNREACHABLE CODE HERE

        }








        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;


    case OPCODE_COMPILECONT:
        // CONTINUE COMPILING STRING
    {
        union {
            WORD word;
            BYTE bytes[4];
        } temp;

        BINT count=(4-(LIBNUM(*ScratchPointer4)&3))&3; // GET NUMBER OF BYTES ALREADY WRITTEN IN LAST WORD
        if(count) {
            --CompileEnd;
            temp.word=*CompileEnd;  // GET LAST WORD
        }
        BYTEPTR ptr=(BYTEPTR) TokenStart;
        do {
        while(count<4) {
            if(ptr==(BYTEPTR)NextTokenStart) {
             // WE ARE AT THE END OF THE GIVEN STRING, STILL NO CLOSING QUOTE, SO WE NEED MORE

                // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                if(count) rplCompileAppend(temp.word);
                *ScratchPointer4=MKPROLOG(DOSTRING+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                RetNum=OK_NEEDMORE;
                return;
            }

            if(*ptr=='\"') {
                // END OF STRING!
                ++ptr;
                if(ptr!=(BYTEPTR)BlankStart) {
                    // QUOTE IN THE MIDDLE OF THE TOKEN IS A SYNTAX ERROR
                    RetNum=ERR_INVALID;
                    return;
                }
                // WE HAVE REACHED THE END OF THE STRING
                if(count) rplCompileAppend(temp.word);
                *ScratchPointer4=MKPROLOG(DOSTRING+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                RetNum=OK_CONTINUE;
                return;
            }
            if(count==0) temp.word=0;
            temp.bytes[count]=*ptr;
            ++count;
            ++ptr;


            }
            //  WE HAVE A COMPLETE WORD HERE
            ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
            rplCompileAppend(temp.word);
            ptr=(BYTEPTR)ScratchPointer1;

            count=0;


        } while(1);     // DANGEROUS! BUT WE WILL RETURN FROM THE CHECK WITHIN THE INNER LOOP;
        //  THIS IS UNREACHABLE CODE HERE

    }
    case OPCODE_DECOMPEDIT:
    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors
        if(ISPROLOG(*DecompileObject)) {
            rplDecompAppendChar('\"');
            rplDecompAppendString2((BYTEPTR)(DecompileObject+1),(OBJSIZE(*DecompileObject)<<2)-(LIBNUM(*DecompileObject)&3));
            rplDecompAppendChar('\"');

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
        libGetInfo2(*DecompileObject,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
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
        // ObjectPTR = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID);
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
    {
        // STRINGS ARE ALWAYS VALID, EVEN IF THEY CONTAIN INVALID UTF-8 SEQUENCES

        RetNum=OK_CONTINUE;
        return;
    }

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        return;

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(MENUNUMBER(MenuCodeArg)>0) {
            RetNum=ERR_NOTMINE;
            return;
        }
        ObjectPTR=(WORDPTR)lib24_menu;
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
