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
    ECMD(TOUTF,"→UTF8",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(FROMUTF,"UTF8→",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TOSTR,"→STR",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(FROMSTR,"STR→",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(SREV,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(NTOKENS,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(NTHTOKEN,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(NTHTOKENPOS,MKTOKENINFO(11,TITYPE_NOTALLOWED,1,2)), \
    CMD(TRIM,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(RTRIM,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(SSTRLEN,"STRLEN",MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(STRLENCP,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TONFC,"→NFC",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(SREPL,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2))


// ADD MORE OPCODES HERE


#define ERROR_LIST \
    ERR(STRINGEXPECTED,0), \
    ERR(INVALIDCODEPOINT,1), \
    ERR(EMPTYSTRING,2)


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





// COMPUTE THE STRING LENGTH IN CODE POINTS
BINT rplStrLenCp(WORDPTR string)
{
    if(ISSTRING(*string))  {
        BINT len=STRLEN(*string);
        BYTEPTR start=(BYTEPTR)(string+1);
        return utf8nlen((char *)start,(char *)(start+len));
    }
    return 0;
}

// COMPUTE THE STRING LENGTH IN CHARACTERS
BINT rplStrLen(WORDPTR string)
{
    if(ISSTRING(*string))  {
        BINT len=STRLEN(*string);
        BYTEPTR start=(BYTEPTR)(string+1);
        return utf8nlenst((char *)start,(char *)(start+len));
    }
    return 0;
}


// COMPUTE THE STRING LENGTH IN BYTES
BINT rplStrSize(WORDPTR string)
{
    if(ISSTRING(*string))  return STRLEN(*string);
    return 0;
}


// COMPARE 2 STRINGS
BINT rplStringCompare(WORDPTR str1,WORDPTR str2)
{
    if(str1==str2) return 1;

    BINT nwords;
    BINT padding=LIBNUM(*str1)&3;

    nwords=OBJSIZE(*str1);

    while(nwords) {
         if(*str1!=*str2) return 0;
         ++str1;
         ++str2;
         --nwords;
     }

    // HERE str1 AND str2 ARE POINTING TO THE FINAL WORD, MASK ANY UNUSED BYTES


    // PADDING HAS THE NUMBER OF BYTES UNUSED IN THE LAST WORD
    WORD mask=(1LL<<(8*(4-padding)))-1LL;

    if( (*str1^*str2)&mask ) return 0;

    return 1;


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

    if(prevlineoff>=len) return -1;
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
// USES SCRATCHPOINTER1 IN CASE text IS IN TEMPOB
WORDPTR rplCreateString(BYTEPTR text,BYTEPTR textend)
{
    BINT lenbytes=textend-text;
    BINT len=(lenbytes+3)>>2;
    if(lenbytes<0) return 0;
    ScratchPointer1=(WORDPTR) text;
    WORDPTR newstring=rplAllocTempOb(len);
    if(newstring) {
        text=(BYTEPTR)ScratchPointer1;
        textend=text+lenbytes;
        BYTEPTR ptr=(BYTEPTR) (newstring+1);
        while(text!=textend) *ptr++=*text++;
        while(((PTR2NUMBER)ptr)&3) *ptr++=0;

        rplSetStringLength(newstring,lenbytes);

        return newstring;
    }
    return 0;
}

// CREATE A NEW STRING OBEJCT BY SIZE AND RETURN ITS ADDRESS
// RETURNS NULL IF ANY ERRORS
// MAY TRIGGER A GC
WORDPTR rplCreateStringBySize(BINT lenbytes)
{
    BINT len=(lenbytes+3)>>2;
    if(lenbytes<0) return 0;
    WORDPTR newstring=rplAllocTempOb(len);
    if(newstring) {

        rplSetStringLength(newstring,lenbytes);

        return newstring;
    }
    return 0;
}




// SKIP ANY SEPARATOR CHARACTERS AT start. ANY CHARACTER IN sepstart/end
// IS CONSIDERED A SEPARATOR AND WILL BE SKIPPED
BYTEPTR rplSkipSep(BYTEPTR start,BYTEPTR end,BYTEPTR sepstart,BYTEPTR sepend)
{
    BYTEPTR sepptr,sepnext,ptr;

    while(start!=end) {
        sepptr=sepstart;
        while(sepptr!=sepend) {
            sepnext=(BYTEPTR)utf8skipst((char *)sepptr,(char *)sepend);
            ptr=start;
            while(sepptr!=sepnext) {
                if(*ptr!=*sepptr) break;
                ++ptr;
                ++sepptr;
            }
            if(sepptr==sepnext) { sepptr=sepstart; break; } // THERE WAS A MATCH!
            // NO MATCH, KEEP GOING
            sepptr=sepnext;
            }
            if(sepptr==sepend) return start; // THERE WAS NO MATCH

            // CHARACTER WAS A SEPARATOR, KEEP GOING
            start=ptr;
        }

    // ALL WERE SEPARATORS

    return start;
}

// SKIP ANY NON-SEPARATOR CHARACTERS AT start. ANY CHARACTER IN sepstart/end
// IS CONSIDERED A SEPARATOR AND WILL STOP THE SEARCH
BYTEPTR rplNextSep(BYTEPTR start,BYTEPTR end,BYTEPTR sepstart,BYTEPTR sepend)
{
    BYTEPTR sepptr,sepnext,ptr;

    while(start!=end) {
        sepptr=sepstart;
        while(sepptr!=sepend) {
            sepnext=(BYTEPTR)utf8skipst((char *)sepptr,(char *)sepend);
            ptr=start;
            while(sepptr!=sepnext) {
                if(*ptr!=*sepptr) break;
                ++ptr;
                ++sepptr;
            }
            if(sepptr==sepnext) return start; // THERE WAS A MATCH!
            // NO MATCH, KEEP GOING
            sepptr=sepnext;
            }
            // THERE WAS NO MATCH

            // KEEP GOING
            start=(BYTEPTR)utf8skipst((char *)start,(char *)end);
        }

    // THERE WERE NO SEPARATORS UNTIL THE END

    return start;
}




BINT rplCountTokens(BYTEPTR start,BYTEPTR end,BYTEPTR sepstart,BYTEPTR sepend)
{
    BINT count=0;

    BYTEPTR token=rplSkipSep(start,end,sepstart,sepend);
    BYTEPTR nextblank;

    while(token!=end) {
        nextblank=rplNextSep(token,end,sepstart,sepend);
        ++count;
        if(nextblank==end) break;
        token=rplSkipSep(nextblank,end,sepstart,sepend);
    }
    return count;
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
    case TOUTF:
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
    case FROMUTF:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISSTRING(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        WORDPTR strobj=rplPeekData(1);
        ScratchPointer1=(strobj+1);
        ScratchPointer2=(WORDPTR)(((BYTEPTR)ScratchPointer1)+STRLEN(*strobj)-1);

        BINT utfchar,count=0;

        while(ScratchPointer1<=ScratchPointer2) {

        utfchar=utf82cp((char *) ScratchPointer1,((char *)ScratchPointer2)+1);
        rplNewBINTPush(utfchar,HEXBINT);
        ++count;
        if(Exceptions) return;

        ScratchPointer1=(WORDPTR)utf8skip((char *)ScratchPointer1,((char *)ScratchPointer2)+1);
        }

        rplNewBINTPush(count,DECBINT);
        rplCreateList();

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

    case SREV:
    {
        // REVERSE A UTF-8 STRING
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISSTRING(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        WORDPTR newobj=rplAllocTempOb(OBJSIZE(*rplPeekData(1)));
        if(!newobj) return;

        WORDPTR oldobj=rplPeekData(1);

        newobj[0]=oldobj[0];        // BOTH STRINGS WILL BE THE EXACT SAME SIZE

        BYTEPTR newptr,oldptr,endptr;
        oldptr=(BYTEPTR)(oldobj+1);     // START OF STRING
        endptr=oldptr+STRLEN(*oldobj);

        newptr=(BYTEPTR)(newobj+1);
        newptr+=STRLEN(*oldobj);        // END OF STRING

        BINT nbytes;
        while(oldptr!=endptr)
        {
            nbytes=(BYTEPTR)utf8skipst((char *)oldptr,(char *)endptr)-oldptr;
            newptr-=nbytes;
            memcpyb(newptr,oldptr,nbytes);
            oldptr+=nbytes;
        }

        rplOverwriteData(1,newobj);

        return;

    }


     case NTOKENS:
    {
     // STRING SEPSTRING -> n
       if(rplDepthData()<2) {
           rplError(ERR_BADARGCOUNT);
           return;
       }
       if(!ISSTRING(*rplPeekData(1)) || !ISSTRING(*rplPeekData(2))) {
           rplError(ERR_STRINGEXPECTED);
           return;
       }

       BYTEPTR strstart,strend;
       BYTEPTR sepstart,sepend;

       strstart=(BYTEPTR) (rplPeekData(2)+1);
       strend=strstart+STRLEN(*rplPeekData(2));

       sepstart=(BYTEPTR) (rplPeekData(1)+1);
       sepend=sepstart+STRLEN(*rplPeekData(1));

       BINT count=rplCountTokens(strstart,strend,sepstart,sepend);

       rplDropData(2);

       rplNewBINTPush(count,DECBINT);

       return;

    }
     case NTHTOKEN:
    {
     // EXTRACT THE NTH TOKEN IN THE STRING

     // STRING STRINGSEP N -> STRING

        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISSTRING(*rplPeekData(3)) || !ISSTRING(*rplPeekData(2))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }


        BYTEPTR strstart,strend;
        BYTEPTR sepstart,sepend;

        strstart=(BYTEPTR) (rplPeekData(3)+1);
        strend=strstart+STRLEN(*rplPeekData(3));

        sepstart=(BYTEPTR) (rplPeekData(2)+1);
        sepend=sepstart+STRLEN(*rplPeekData(2));

        BINT n=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;

        if(n<1) {
            strstart=strend;    // RETURN EMPTY TOKEN
        }
        while((n>0)&&(strstart!=strend)) {
            strstart=rplSkipSep(strstart,strend,sepstart,sepend);
            --n;
            if(n<=0) break;
            strstart=rplNextSep(strstart,strend,sepstart,sepend);
        }

        rplDropData(3);
        WORDPTR newstring=rplCreateString(strstart,rplNextSep(strstart,strend,sepstart,sepend));
        if(!newstring) return;

        rplPushData(newstring);

        return;

    }
     case NTHTOKENPOS:
    {
     // EXTRACT THE NTH TOKEN IN THE STRING

     // STRING STRINGSEP N -> POS

        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISSTRING(*rplPeekData(3)) || !ISSTRING(*rplPeekData(2))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }


        BYTEPTR strstart,strend;
        BYTEPTR sepstart,sepend;

        strstart=(BYTEPTR) (rplPeekData(3)+1);
        strend=strstart+STRLEN(*rplPeekData(3));

        sepstart=(BYTEPTR) (rplPeekData(2)+1);
        sepend=sepstart+STRLEN(*rplPeekData(2));

        BINT n=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;

        if(n<1) {
            strstart=strend;    // RETURN EMPTY TOKEN
        }
        while((n>0)&&(strstart!=strend)) {
            strstart=rplSkipSep(strstart,strend,sepstart,sepend);
            --n;
            if(n<=0) break;
            strstart=rplNextSep(strstart,strend,sepstart,sepend);
        }

        // WE HAVE THE TOKEN, COMPUTE THE POSITION
        BINT pos;

        if(strstart==strend) pos=-1;
        else {

        pos=1;
        BYTEPTR ptr=(BYTEPTR) (rplPeekData(3)+1);
        while(ptr!=strstart) {
            ptr=(BYTEPTR)utf8skipst((char *)ptr,(char *)strend);
            ++pos;
        }


        }

        rplDropData(3);
        rplNewBINTPush(pos,DECBINT);
        return;

    }

     case TRIM:
    {
        // ELIMINATE ANY SEPARATORS AT THE END OF STRING

        // STRING STRINGSEP -> STRING

           if(rplDepthData()<2) {
               rplError(ERR_BADARGCOUNT);
               return;
           }
           if(!ISSTRING(*rplPeekData(2)) || !ISSTRING(*rplPeekData(1))) {
               rplError(ERR_STRINGEXPECTED);
               return;
           }

           BYTEPTR strstart,strend;
           BYTEPTR sepstart,sepend;

           strstart=(BYTEPTR) (rplPeekData(2)+1);
           strend=strstart+STRLEN(*rplPeekData(2));

           sepstart=(BYTEPTR) (rplPeekData(1)+1);
           sepend=sepstart+STRLEN(*rplPeekData(1));

           BYTEPTR lastsep=0,nextsep;

           while((strstart!=strend)) {
               lastsep=strstart;
               strstart=rplSkipSep(strstart,strend,sepstart,sepend);
               if(strstart==strend) break;  // STRING ENDS IN BLANKS
               nextsep=rplNextSep(strstart,strend,sepstart,sepend);
               if(nextsep==strend) { lastsep=strend; break; } // STRING DOESN'T END IN BLANKS
               strstart=nextsep;
               }

           if( !lastsep ||(lastsep==strend)) {
               rplDropData(1);
               return;    // NOTHING TO TRIM
           }

           BYTEPTR ptr=(BYTEPTR) (rplPeekData(2)+1);

           WORDPTR newstr=rplCreateString(ptr,lastsep);
           if(!newstr) return;

           rplDropData(1);
           rplOverwriteData(1,newstr);
           return;

    }

     case RTRIM:
    {
        // ELIMINATE ANY SEPARATORS AT THE START OF STRING

        // STRING STRINGSEP -> STRING

           if(rplDepthData()<2) {
               rplError(ERR_BADARGCOUNT);
               return;
           }
           if(!ISSTRING(*rplPeekData(2)) || !ISSTRING(*rplPeekData(1))) {
               rplError(ERR_STRINGEXPECTED);
               return;
           }

           BYTEPTR strstart,strend;
           BYTEPTR sepstart,sepend;

           strstart=(BYTEPTR) (rplPeekData(2)+1);
           strend=strstart+STRLEN(*rplPeekData(2));

           sepstart=(BYTEPTR) (rplPeekData(1)+1);
           sepend=sepstart+STRLEN(*rplPeekData(1));

           BYTEPTR firsttok;

               firsttok=rplSkipSep(strstart,strend,sepstart,sepend);

           if(firsttok==strstart) {
               rplDropData(1);
               return;    // NOTHING TO TRIM
           }

           WORDPTR newstr=rplCreateString(firsttok,strend);
           if(!newstr) return;

           rplDropData(1);
           rplOverwriteData(1,newstr);
           return;

    }
    case SSTRLEN:
    {
        // COMPUTE STRING LENGTH IN CHARACTERS

           if(rplDepthData()<1) {
               rplError(ERR_BADARGCOUNT);
               return;
           }
           if(!ISSTRING(*rplPeekData(1))) {
               rplError(ERR_STRINGEXPECTED);
               return;
           }

           WORDPTR string=rplPeekData(1);
           rplDropData(1);
           rplNewBINTPush(rplStrLen(string),DECBINT);

        return;
    }

    case STRLENCP:
    {
        // COMPUTE STRING LENGTH IN CODE POINTS

           if(rplDepthData()<1) {
               rplError(ERR_BADARGCOUNT);
               return;
           }
           if(!ISSTRING(*rplPeekData(1))) {
               rplError(ERR_STRINGEXPECTED);
               return;
           }

           WORDPTR string=rplPeekData(1);
           rplDropData(1);
           rplNewBINTPush(rplStrLenCp(string),DECBINT);

        return;

    }


    case TONFC:
    {
        // NORMALIZE ANY STRING BY NFC
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISSTRING(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        WORDPTR newstring=rplCreateStringBySize(4); // NEW STRING AT THE END OF TEMPOB
        if(!newstring) return;
        BYTEPTR start=(BYTEPTR) (rplPeekData(1)+1);
        BYTEPTR end=start+rplStrSize(rplPeekData(1));
        BYTEPTR nstrptr;
        BINT totalsize=4,size=0;
        BINT nbytes,k;
        nstrptr=(BYTEPTR) (newstring+1);

        while(start<end)
        {
            nbytes=utf82NFC((char *)start,(char *)end);
            k=0;
            while(unicodeBuffer[k]!=0) {

                UBINT cp=cp2utf8(unicodeBuffer[k]);


                while(cp&0xff) {


                if(size==totalsize) {
                ScratchPointer1=newstring;
                ScratchPointer2=(WORDPTR)start;
                ScratchPointer3=(WORDPTR)(end-1);
                rplResizeLastObject(1);
                totalsize+=4;
                newstring=ScratchPointer1;
                start=(BYTEPTR)ScratchPointer2;
                end=((BYTEPTR)ScratchPointer3)+1;
                nstrptr=(BYTEPTR) (newstring+1);
                }

                nstrptr[size]=cp&0xff;
                cp>>=8;
                ++size;
                }
                ++k;
           }
           start+=nbytes;

        }

        rplSetStringLength(newstring,size);
        rplOverwriteData(1,newstring);

      return;
     }

    // ADD MORE OPCODES HERE

     case SREPL:
    {
        // MULTIPLE FIND AND REPLACE
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

            if(ISLIST(*rplPeekData(3))||ISLIST(*rplPeekData(2))||ISLIST(*rplPeekData(1))) {
                rplListMultiArgDoCmd(3);
                return;
            }


            if(!ISSTRING(*rplPeekData(3)) || !ISSTRING(*rplPeekData(2)) || !ISSTRING(*rplPeekData(1)) ) {
                rplError(ERR_STRINGEXPECTED);
                return;
            }

            BINT lenstr1,lenfind,lenfindcp,pos,maxpos,sizestr1,sizefind,sizerepl;
            BYTEPTR str1,find,repl,end1;


            pos=1;



            lenstr1=rplStrLen(rplPeekData(3));
            lenfind=rplStrLen(rplPeekData(2));
            lenfindcp=rplStrLenCp(rplPeekData(2));
            sizestr1=rplStrSize(rplPeekData(3));
            sizefind=rplStrSize(rplPeekData(2));
            sizerepl=rplStrSize(rplPeekData(1));

            if(lenfind>lenstr1) {
                // WILL NEVER FIND A LONGER STRING INSIDE A SHORT ONE
                rplDropData(2);
                rplPushData((WORDPTR)zero_bint);
                return;
            }

            maxpos=lenstr1-lenfind+1;

            WORDPTR newstring=rplCreateStringBySize(1);
            BINT newsize=0,rcount=0;

            repl=(BYTEPTR)(rplPeekData(1)+1);
            find=(BYTEPTR)(rplPeekData(2)+1);
            str1=(BYTEPTR)(rplPeekData(3)+1);
            end1=str1+sizestr1;
            BYTEPTR nextchar=str1;

            // DO SEARCH AND REPLACE

            for(;pos<=maxpos;++pos)
            {
                if(utf8ncmp2((char *)nextchar,(char *)end1,(char *)find,lenfindcp)==0) {
                    // FOUND A MATCH, COPY THE STRING SO FAR AND THE REPLACEMENT
                    BINT newsize2=newsize+(nextchar-str1)+sizerepl;
                    if( ((newsize2+3)>>2)>((newsize+3)>>2) ) {
                        BINT endoff=end1-str1;
                        ScratchPointer1=(WORDPTR)str1;
                        ScratchPointer2=(WORDPTR)find;
                        ScratchPointer3=(WORDPTR)repl;
                        ScratchPointer4=newstring;
                        rplResizeLastObject(((newsize2+3)>>2)-((newsize+3)>>2) );
                        if(Exceptions) return;
                        str1=(BYTEPTR)ScratchPointer1;
                        find=(BYTEPTR)ScratchPointer2;
                        repl=(BYTEPTR)ScratchPointer3;
                        newstring=ScratchPointer4;
                        end1=str1+endoff;
                    }

                    memmoveb(((BYTEPTR)(newstring+1))+newsize,str1,nextchar-str1);
                    memmoveb(((BYTEPTR)(newstring+1))+newsize+(nextchar-str1),repl,sizerepl);

                    newsize=newsize2;

                    nextchar+=sizefind;
                    str1=nextchar;
                    pos+=lenfind-1;
                    ++rcount;
                }
                else {
                nextchar=(BYTEPTR)utf8skipst((char *)nextchar,(char *)end1);
                }

            }

            // NOT FOUND
            // FOUND A MATCH, COPY THE STRING SO FAR AND THE REPLACEMENT
            BINT newsize2=newsize+(end1-str1);
            if( ((newsize2+3)>>2)>((newsize+3)>>2) ) {
                BINT endoff=end1-str1;
                ScratchPointer1=(WORDPTR)str1;
                ScratchPointer2=(WORDPTR)find;
                ScratchPointer3=(WORDPTR)repl;
                ScratchPointer4=newstring;
                rplResizeLastObject(((newsize2+3)>>2)-((newsize+3)>>2) );
                if(Exceptions) return;
                str1=(BYTEPTR)ScratchPointer1;
                find=(BYTEPTR)ScratchPointer2;
                repl=(BYTEPTR)ScratchPointer3;
                newstring=ScratchPointer4;
                end1=str1+endoff;
            }

            if(rcount) {
            memmoveb(((BYTEPTR)(newstring+1))+newsize,str1,end1-str1);
            rplSetStringLength(newstring,newsize2);
            rplDropData(3);
            rplPushDataNoGrow(newstring);
            rplNewBINTPush(rcount,DECBINT);
            return;
            }
            rplDropData(2);
            rplPushData((WORDPTR)zero_bint);
            return;
     }


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
        WORDPTR string=rplDecompile(ScratchPointer1,DECOMP_NOHINTS);
        if(!string) { ExceptionPointer=IPtr; return; }   // THERE WAS AN ERROR, TAKE OWNERSHIP OF IT
        ScratchPointer1=string;
        }

        if(!ISSTRING(*ScratchPointer2)) {
        WORDPTR string=rplDecompile(ScratchPointer2,DECOMP_NOHINTS);
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

    case OVR_FUNCEVAL:
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
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
            if(!ISPROLOG(*rplPeekData(2))|| !ISPROLOG(*rplPeekData(1))) {
                if(*rplPeekData(2)==*rplPeekData(1)) {
                    rplDropData(2);
                    rplPushTrue();
                } else {
                    rplDropData(2);
                    rplPushFalse();
                }
                return;

            }
            else {
                rplError(ERR_INVALIDOPCODE);
                return;
            }

        // DELIBERATED FALL-THROUGH TO OVR_EQ WHEN THERE'S NO COMMANDS INVOLVED

    }
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

        if(rplStringCompare(rplPeekData(1),rplPeekData(2))) rplOverwriteData(2,(WORDPTR)one_bint);
        else rplOverwriteData(2,(WORDPTR)zero_bint);
        rplDropData(1);
        return;

    case OVR_NOTEQ:

        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if( (!ISSTRING(*rplPeekData(2))) || (!ISSTRING(*rplPeekData(1)))) {
            rplOverwriteData(2,(WORDPTR)one_bint);
            rplDropData(1);
            return;
        }

        if(rplStringCompare(rplPeekData(1),rplPeekData(2))) rplOverwriteData(2,(WORDPTR)zero_bint);
        else rplOverwriteData(2,(WORDPTR)one_bint);
        rplDropData(1);
        return;
    case OVR_ISTRUE:
    {
        if(rplStrSize(rplPeekData(1))) rplOverwriteData(1,(WORDPTR)one_bint);
        else rplOverwriteData(1,(WORDPTR)zero_bint);
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

        if(*((BYTEPTR)TokenStart)=='\"') {
            // START A STRING


            ScratchPointer4=CompileEnd;     // SAVE CURRENT COMPILER POINTER TO FIX THE OBJECT AT THE END

            rplCompileAppend(MKPROLOG(DOSTRING,0));

            union {
                WORD word;
                BYTE bytes[4];
            } temp;

            BINT count=0,escape=0,code=0;
            BYTEPTR ptr=(BYTEPTR) TokenStart;
            ++ptr;  // SKIP THE QUOTE
            do {
            while(count<4) {
                if(ptr==(BYTEPTR)NextTokenStart) {
                 // WE ARE AT THE END OF THE GIVEN STRING, STILL NO CLOSING QUOTE, SO WE NEED MORE
                    if(escape>1) {
                        // OTHER THAN A HEX CHARACTER, END UNICODE ENTRY
                        UBINT codebytes=cp2utf8(code);
                        do {
                        if(count==0) temp.word=0;
                        temp.bytes[count]=codebytes&0xff;
                        ++count;
                        codebytes>>=8;
                        if(count>=4) {
                            //  WE HAVE A COMPLETE WORD HERE
                            ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                            rplCompileAppend(temp.word);
                            ptr=(BYTEPTR)ScratchPointer1;
                            count=0;
                        }
                        }while(codebytes);
                        escape=0;
                    }


                    // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                    if(count) rplCompileAppend(temp.word);
                    *ScratchPointer4=MKPROLOG(DOSTRING+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                    RetNum=OK_NEEDMORE;
                    return;
                }
                if( (*ptr=='\\')&&(!escape)) { escape=1; ++ptr; continue; }
                if( (*ptr=='\"') && (escape!=1)) {
                    if(escape>1) {
                        // OTHER THAN A HEX CHARACTER, END UNICODE ENTRY
                        UBINT codebytes=cp2utf8(code);
                        do {
                        if(count==0) temp.word=0;
                        temp.bytes[count]=codebytes&0xff;
                        ++count;
                        codebytes>>=8;
                        if(count>=4) {
                            //  WE HAVE A COMPLETE WORD HERE
                            ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                            rplCompileAppend(temp.word);
                            ptr=(BYTEPTR)ScratchPointer1;
                            count=0;
                        }
                        }while(codebytes);
                        escape=0;
                    }


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

                if( (escape==1) && ((*ptr=='U')||(*ptr=='u'))) { escape++; code=0; ++ptr; continue; }
                if(escape>5) {
                    // ALREADY HAVE 4 DIGITS, WE ARE DONE
                    UBINT codebytes=cp2utf8(code);
                    do {
                    if(count==0) temp.word=0;
                    temp.bytes[count]=codebytes&0xff;
                    ++count;
                    codebytes>>=8;
                    if(count>=4) {
                        //  WE HAVE A COMPLETE WORD HERE
                        ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                        rplCompileAppend(temp.word);
                        ptr=(BYTEPTR)ScratchPointer1;
                        count=0;
                    }
                    } while(codebytes);
                    escape=0;
                }
                if(escape>1 && ((*ptr>='0')&&(*ptr<='9'))) { ++escape; code=code*16+(*ptr-'0'); ++ptr; continue; }
                if(escape>1 && ((*ptr>='A')&&(*ptr<='F'))) { ++escape; code=code*16+(*ptr-'A'); ++ptr; continue; }
                if(escape>1 && ((*ptr>='a')&&(*ptr<='f'))) { ++escape; code=code*16+(*ptr-'A'); ++ptr; continue; }

                if(escape>1) {
                    // OTHER THAN A HEX CHARACTER, END UNICODE ENTRY
                    UBINT codebytes=cp2utf8(code);
                    do {
                    if(count==0) temp.word=0;
                    temp.bytes[count]=codebytes&0xff;
                    ++count;
                    codebytes>>=8;
                    if(count>=4) {
                        //  WE HAVE A COMPLETE WORD HERE
                        ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                        rplCompileAppend(temp.word);
                        ptr=(BYTEPTR)ScratchPointer1;
                        count=0;
                    }
                    }while(codebytes);
                    escape=0;
                }

                if(count==0) temp.word=0;
                temp.bytes[count]=*ptr;
                ++count;
                ++ptr;
                escape=0;
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

        BINT escape=0,code=0;

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
                if(escape>1) {
                    // OTHER THAN A HEX CHARACTER, END UNICODE ENTRY
                    UBINT codebytes=cp2utf8(code);
                    do {
                    if(count==0) temp.word=0;
                    temp.bytes[count]=codebytes&0xff;
                    ++count;
                    codebytes>>=8;
                    if(count>=4) {
                        //  WE HAVE A COMPLETE WORD HERE
                        ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                        rplCompileAppend(temp.word);
                        ptr=(BYTEPTR)ScratchPointer1;
                        count=0;
                    }
                    }while(codebytes);
                    escape=0;
                }

                // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                if(count) rplCompileAppend(temp.word);
                *ScratchPointer4=MKPROLOG(DOSTRING+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                RetNum=OK_NEEDMORE;
                return;
            }
            if( (*ptr=='\\')&&(!escape)) { escape=1; ++ptr; continue; }
            if( (*ptr=='\"') && (escape!=1)) {
                if(escape>1) {
                    // OTHER THAN A HEX CHARACTER, END UNICODE ENTRY
                    UBINT codebytes=cp2utf8(code);
                    do {
                    if(count==0) temp.word=0;
                    temp.bytes[count]=codebytes&0xff;
                    ++count;
                    codebytes>>=8;
                    if(count>=4) {
                        //  WE HAVE A COMPLETE WORD HERE
                        ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                        rplCompileAppend(temp.word);
                        ptr=(BYTEPTR)ScratchPointer1;
                        count=0;
                    }
                    }while(codebytes);
                    escape=0;
                }


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

            if( (escape==1) && ((*ptr=='U')||(*ptr=='u'))) { escape++; code=0; ++ptr; continue; }
            if(escape>5) {
                // ALREADY HAVE 4 DIGITS, WE ARE DONE
                UBINT codebytes=cp2utf8(code);
                do {
                if(count==0) temp.word=0;
                temp.bytes[count]=codebytes&0xff;
                ++count;
                codebytes>>=8;
                if(count>=4) {
                    //  WE HAVE A COMPLETE WORD HERE
                    ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                    rplCompileAppend(temp.word);
                    ptr=(BYTEPTR)ScratchPointer1;
                    count=0;
                }
                }while(codebytes);
                escape=0;
            }
            if(escape>1 && ((*ptr>='0')&&(*ptr<='9'))) { ++escape; code=code*16+(*ptr-'0'); ++ptr; continue; }
            if(escape>1 && ((*ptr>='A')&&(*ptr<='F'))) { ++escape; code=code*16+(*ptr-'A'); ++ptr; continue; }
            if(escape>1 && ((*ptr>='a')&&(*ptr<='f'))) { ++escape; code=code*16+(*ptr-'A'); ++ptr; continue; }

            if(escape>1) {
                // OTHER THAN A HEX CHARACTER, END UNICODE ENTRY
                UBINT codebytes=cp2utf8(code);
                do {
                if(count==0) temp.word=0;
                temp.bytes[count]=codebytes&0xff;
                ++count;
                codebytes>>=8;
                if(count>=4) {
                    //  WE HAVE A COMPLETE WORD HERE
                    ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                    rplCompileAppend(temp.word);
                    ptr=(BYTEPTR)ScratchPointer1;
                    count=0;
                }
                }while(codebytes);
                escape=0;
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
    {
        // SAME AS DECOMPILE BUT NEED TO ESCAPE THE BACKSLASH CHARACTER AND THE DOUBLE QUOTE
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors
        if(ISPROLOG(*DecompileObject)) {
            rplDecompAppendChar('\"');
            BYTEPTR start=(BYTEPTR)(DecompileObject+1);
            BYTEPTR end=start+(OBJSIZE(*DecompileObject)<<2)-(LIBNUM(*DecompileObject)&3);
            BYTEPTR ptr;
            for(ptr=start;ptr<end;++ptr)
            {
                if(*ptr=='\\') {
                    rplDecompAppendString2(start,ptr-start);
                    rplDecompAppendChar('\\');
                    start=ptr;
                    continue;
                }
                if(*ptr=='\"') {
                    rplDecompAppendString2(start,ptr-start);
                    rplDecompAppendChar('\\');
                    start=ptr;
                    continue;
                }
                if(*ptr==0) {
                    rplDecompAppendString2(start,ptr-start);
                    rplDecompAppendString((BYTEPTR)"\\U0000");
                    start=ptr+1;
                    continue;
                }
            }
            if(ptr>start) rplDecompAppendString2(start,ptr-start);
            rplDecompAppendChar('\"');

            RetNum=OK_CONTINUE;
            return;

        }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds((char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
        return;

    }
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
