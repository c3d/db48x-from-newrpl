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
#define LIBRARY_NUMBER  88


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(BEGINPLOT,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(EDITPLOT,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(ENDPLOT,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(STROKECOL,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(STROKETYPE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(FILLCOL,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(FILLTYPE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(DOFILL,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(DOSTROKE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(MOVETO,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(LINETO,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(CIRCLE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(RECTANG,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(CTLNODE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(CURVE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(BGROUP,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(EGROUP,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(DOGROUP,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(BASEPT,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(TRANSLATE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(ROTATE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(SCALE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(CLEARTRANSF,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(SETFONT,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(TEXTHEIGHT,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(TEXTOUT,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2))

// ADD MORE PLOT COMMANDS HERE


#define ERROR_LIST \
    ERR(PLOTEXPECTED,0), \
    ERR(INVALIDPLOTCOMMAND,2), \
    ERR(INVALIDPLOTSIZE,2), \
    ERR(NOCURRENTPLOT,3)



// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS  LIBRARY_NUMBER,LIBRARY_NUMBER+1,LIBRARY_NUMBER+2,LIBRARY_NUMBER+3


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************


INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib88_menu);



ROMOBJECT cplot_ident[]= {
        MKPROLOG(DOIDENT,1),
        TEXT2WORD('C','P','l','t')
};





// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib88_menu,
    (WORDPTR)cplot_ident,

    0
};



// GET THE LENGTH OF A PLOT OBJECT FROM ITS PROLOG
#define PLTLEN(prolog) ((OBJSIZE(prolog)<<2)-(LIBNUM(prolog)&3))









// GET THE POINTER TO CPlt, BUT COPY IT TO THE END OF TEMPOB TO APPEND

WORDPTR rplCPlotGetPtr()
{
    WORDPTR *var=rplFindLAM((WORDPTR)cplot_ident,1);
    if(!var) return 0;
    if(rplSkipOb(var[1])==TempObEnd) return var[1];

    WORDPTR newobj=rplMakeNewCopy(var[1]);  // MAKE A NEW COPY THAT WE CAN STRETCH AND MODIFY
    if(!newobj) return 0;
    var[1]=newobj;          // REPLACE WITH A NEW COPY AT END OF TEMPOB
    return newobj;
}


#define GETBYTE(n,pos) ((BYTE)((((UBINT64)n)>>((pos)<<3))&0xff))



// APPEND A NUMBER TO THE CURRENT PLOT
void rplCPlotNumber(BINT64 num)
{
    WORDPTR obj=rplCPlotGetPtr();
    if(!obj) return;

    // PACK THE NUMBER INTO A UBINT64 (UP TO 5 BYTES USED)
    BINT used=0;
    BYTE bpack[8];
    BINT sign;

    if(num<0) { sign=8; num=-num; }
    else sign=0;

    while(num) {
        bpack[used]=num&0xff;
        ++used;
        num>>=8;
    }

    if(!used) { bpack[0]=0; used=1; }
    if(bpack[used-1]>0xf) { bpack[used]=0; ++used; }
    if(used>5) {
        // NUMBER TOO BIG
        rplError(ERR_NUMBERTOOBIG);
        return;
    }

    bpack[used-1]|=((used-1)|sign)<<4;  // STARTER BYTE


    // NEED TO STORE 'used' BYTES
    BYTEPTR ptr=((BYTEPTR) (obj+1))+PLTLEN(*obj);  // POINT TO THE NEXT BYTE
    BYTEPTR end=(BYTEPTR)(obj+1+OBJSIZE(*obj));

    // HERE ptr POINTS TO THE FIRST AVAILABLE BYTE IN THE OBJECT
    BINT needwords=(ptr+used+3-end)/4;
    if(needwords>0) {
        rplResizeLastObject(needwords);
        if(Exceptions) return;
        end+=needwords<<2;
    }

    // COPY THE FIRST BYTE MODIFIED:
    --used;
    *ptr++=bpack[used];
    BINT k;
    // THE REST OF THE NUMBER IS STORED IN LITTLE ENDIAN FORMAT
    for(k=0;k<used;++k)
    {
        *ptr++=bpack[k];
    }

    // UPDATE THE PROLOG
    *obj=MKPROLOG(DOPLOT+((end-ptr)&3),((WORDPTR)end)-(obj+1));

}

// APPEND A COMMAND TO THE CURRENT PLOT
void rplCPlotCmd(BINT cmd)
{
    WORDPTR obj=rplCPlotGetPtr();
    if(!obj) return;

    // NEED TO STORE 1 BYTE
    BYTEPTR ptr=((BYTEPTR) (obj+1))+PLTLEN(*obj);  // POINT TO THE NEXT BYTE
    BYTEPTR end=(BYTEPTR)(obj+1+OBJSIZE(*obj));

    // HERE ptr POINTS TO THE FIRST AVAILABLE BYTE IN THE OBJECT

    if(ptr>=end) {
        rplResizeLastObject(1);
        end+=4;
        if(Exceptions) return;
    }

    // COPY THE COMMAND

    *ptr++=(BYTE)(cmd&0xff);

    // UPDATE THE PROLOG
    *obj=MKPROLOG(DOPLOT+((end-ptr)&3),((WORDPTR)end)-(obj+1));


    return;
}


// SKIP TO THE NEXT OBJECT (NUMBER OR OPCODE)
// NO ARGUMENT CHECKS
BYTEPTR rplPlotSkip(BYTEPTR ptr)
{
    switch(*ptr>>4)
    {
    case 1:
    case 9:
        // 1-BYTE FOLLOWS
        return ptr+2;
    case 2:
    case 10:
        // 2-BYTE FOLLOWS
        return ptr+3;
    case 3:
    case 11:
        // 3-BYTE FOLLOWS
        return ptr+4;
    case 4:
    case 12:
        // 4-BYTE FOLLOWS
        return ptr+5;
    case 5:
        // STRING FOLLOWS, NEED TO READ 20-BIT LENGTH
    {
        BINT len=((*ptr&0xf)<<16)|(ptr[2]<<8)|(ptr[1]);
        return ptr+3+len;
    }
    case 6:
    default:
    case 0:
    case 8:
        // NO OTHER BYTES
        return ptr+1;
    }
}


// DECODE A NUMBER INSIDE A PLOT OBJECT

BINT64 rplPlotNumber2BINT(BYTEPTR ptr)
{

switch(*ptr>>4)
{
case 0:
    // NO OTHER BYTES
    return ptr[0]&0xf;
case 8:
    return -((BINT)ptr[0]&0xf);

case 1:
    // 1-BYTE FOLLOWS
    return ((((BINT)ptr[0]&0xf)<<8)|((BINT)ptr[1]));
case 9:
    // 1-BYTE FOLLOWS
    return -((((BINT)ptr[0]&0xf)<<8)|((BINT)ptr[1]));
case 2:
case 5:
    // 2-BYTE FOLLOWS
    return ((((BINT)ptr[0]&0xf)<<16)|((BINT)ptr[1])|(((BINT)ptr[2])<<8));
case 10:
    // 2-BYTE FOLLOWS
    return -((((BINT)ptr[0]&0xf)<<16)|((BINT)ptr[1])|(((BINT)ptr[2])<<8));
case 3:
    // 3-BYTE FOLLOWS
    return ((((BINT)ptr[0]&0xf)<<24)|((BINT)ptr[1])|(((BINT)ptr[2])<<8)|(((BINT)ptr[3])<<16));

case 11:
    // 3-BYTE FOLLOWS
    return -((((BINT)ptr[0]&0xf)<<24)|((BINT)ptr[1])|(((BINT)ptr[2])<<8)|(((BINT)ptr[3])<<16));
case 4:
    // 4-BYTE FOLLOWS
    return ((((BINT64)ptr[0]&0xf)<<32)|((BINT64)ptr[1])|(((BINT64)ptr[2])<<8)|(((BINT64)ptr[3])<<16)|(((BINT64)ptr[4])<<24));
case 12:
    // 4-BYTE FOLLOWS
    return -((((BINT64)ptr[0]&0xf)<<32)|((BINT64)ptr[1])|(((BINT64)ptr[2])<<8)|(((BINT64)ptr[3])<<16)|(((BINT64)ptr[4])<<24));
case 6:
default:
    // NO OTHER BYTES
    return 0;
}

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

    case BEGINPLOT:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISNUMBER(*rplPeekData(1)) || !ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        BINT64 width,height;

        width=rplReadNumberAsBINT(rplPeekData(2));
        if(Exceptions) return;

        height=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;

        if( (width<1) || (height<1)) {
            rplError(ERR_POSITIVEINTEGEREXPECTED);
            return;
        }

        // START A NEW EMPTY PLOT OBJECT AND STORE IT IN LOCAL VARIABLE CPlt
        WORDPTR newobj=rplAllocTempOb(0);
        if(!newobj) return;
        newobj[0]=MKPROLOG(DOPLOT,0);

        rplOverwriteData(2,(WORDPTR)newobj);
        rplOverwriteData(1,(WORDPTR)cplot_ident);
        rplCallOperator(CMD_LSTO);

        if(Exceptions) return;

        // NOW SET THE PLOT SIZE, THIS IS MANDATORY
        rplCPlotNumber(width);
        if(Exceptions) return;

        rplCPlotNumber(height);
        if(Exceptions) return;

        rplCPlotCmd(PLT_SETSIZE);
        return;

    }

    case EDITPLOT:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISPLOT(*rplPeekData(1)) ) {
            rplError(ERR_PLOTEXPECTED);
            return;
        }

        rplPushData((WORDPTR)cplot_ident);
        rplCallOperator(CMD_LSTO);

        if(Exceptions) return;


        // PLOT OBJECT IS READY FOR APPEND
        return;

    }


    case ENDPLOT:
    {
        // PUSH THE CURRENT PLOT OBJECT TO THE STACK
        WORDPTR val=rplGetLAM((WORDPTR)cplot_ident);
        if(val) rplPushData(val);
        else rplError(ERR_NOCURRENTPLOT);
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
        if((TokenLen==8) && (!utf8ncmp((char *)TokenStart,"PLOTDATA",8))) {

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
            // SOMETHING BAD HAPPENED, THERE'S NO PLOTDATA HEADER
            RetNum=ERR_SYNTAX;
            return;
        }

        union {
            WORD word;
            BYTE bytes[4];
        } temp;

        BINT count=(4-(LIBNUM(*ScratchPointer4)&3))&3; // GET NUMBER OF BYTES ALREADY WRITTEN IN LAST WORD
        BINT stringmode=LIBNUM(*ScratchPointer4)&4;     // ARE WE COMPILING A STRING?
        if(count) {
            --CompileEnd;
            temp.word=*CompileEnd;  // GET LAST WORD
        }
        BYTEPTR ptr=(BYTEPTR) TokenStart;

        if(stringmode) {
            BINT addedbytes=0;
            // COMPILE IT JUST LIKE A STRING
            do {
            while(count<4) {
                if(ptr==(BYTEPTR)NextTokenStart) {
                 // WE ARE AT THE END OF THE GIVEN STRING, STILL NO CLOSING QUOTE, SO WE NEED MORE

                    // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                    if(count) rplCompileAppend(temp.word);
                    *ScratchPointer4=MKPROLOG(DOPLOT+4+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                    RetNum=OK_NEEDMORE;

                    BYTEPTR nptr=(BYTEPTR)ScratchPointer3;  // RESTORE SAVED STRING START LOCATION


                    // PATCH THE STRING LENGTH SO FAR
                    addedbytes+=rplPlotNumber2BINT(nptr);   // ADD THE NEW BYTES TO THE ORIGINAL SIZE

                    if(addedbytes>=0x100000) {
                        // STRING IS TOO BIG!
                        RetNum=ERR_SYNTAX;
                        return;
                    }

                    // AND UPDATE THE NUMBER
                    nptr[0]=((addedbytes>>16)&0xf)|0x50;
                    nptr[2]=(addedbytes>>8)&0xff;
                    nptr[1]=(addedbytes)&0xff;

                    return;
                }

                if(*ptr=='\"') {
                    // END OF STRING!
                    ++ptr;
                    // WE HAVE REACHED THE END OF THE STRING
                    stringmode=0;

                    // UPDATE THE STRING SIZE AT START OF STRING
                    BYTEPTR nptr=(BYTEPTR)ScratchPointer3;  // RESTORE SAVED STRING START LOCATION

                    if(nptr+3>(BYTEPTR)CompileEnd) {
                        // SPECIAL CASE WHERE STRING IS SHORT AND INITIAL COUNT
                        // WASN'T STORED YET
                        // JUST STORE IT TEMPORARILY TO READ THE NUMBER PROPERLY
                        // USES TEMPOB GUARANTEED SLACK
                        *CompileEnd=temp.word;
                    }


                    addedbytes+=rplPlotNumber2BINT(nptr);   // ADD THE NEW BYTES TO THE ORIGINAL SIZE

                    if(addedbytes>=0x100000) {
                        // STRING IS TOO BIG!
                        RetNum=ERR_SYNTAX;
                        return;
                    }

                    // AND UPDATE THE NUMBER
                    nptr[0]=((addedbytes>>16)&0xf)|0x50;
                    nptr[2]=(addedbytes>>8)&0xff;
                    nptr[1]=(addedbytes)&0xff;


                    if(nptr+3>(BYTEPTR)CompileEnd) {
                        // SPECIAL CASE WHERE STRING IS SHORT AND INITIAL COUNT
                        // WASN'T STORED YET
                        temp.word=*CompileEnd;
                    }


                    // CONTINUE COMPILING
                    break;
                }

                if(count==0) temp.word=0;
                temp.bytes[count]=*ptr;
                ++addedbytes;
                ++count;
                ++ptr;


                }
                if(count==4) {
                //  WE HAVE A COMPLETE WORD HERE
                ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                rplCompileAppend(temp.word);
                ptr=(BYTEPTR)ScratchPointer1;

                count=0;
                }
                else break;


            } while(1);     // DANGEROUS! BUT WE WILL RETURN FROM THE CHECK WITHIN THE INNER LOOP;


        }
        else {

        if((TokenLen==7) && (!utf8ncmp((char *)TokenStart,"ENDPLOT",7))) {
            // ADD THE ENDPLOT COMMAND '~'
            if(count==0) temp.word=0;
            temp.bytes[count]='~';
            ++count;
            //  WE HAVE A COMPLETE WORD HERE
            rplCompileAppend(temp.word);

            *ScratchPointer4=MKPROLOG(DOPLOT+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
            RetNum=OK_CONTINUE;
            return;

        }

        }

        BINT64 Locale=rplGetSystemLocale();
        BINT ucode;
        BINT isnum=0;
        BINT64 number;

        do {
        while(count<4) {
            if(ptr>=(BYTEPTR)BlankStart) {
             // WE ARE AT THE END OF THE TOKEN, WE NEED MORE

                if(isnum) {
                                        // END THE NUMBER
                                        // COMPILE ITS BYTES
                                        // PACK THE NUMBER INTO A UBINT64 (UP TO 5 BYTES USED)
                                        BINT used=0;
                                        BYTE bpack[8];
                                        BINT sign;

                                        if(isnum<0) sign=8;
                                        else sign=0;

                                        while(number) {
                                            bpack[used]=number&0xff;
                                            ++used;
                                            number>>=8;
                                        }

                                        if(!used) { bpack[0]=0; used=1; }
                                        if(bpack[used-1]>0xf) { bpack[used]=0; ++used; }
                                        if(used>5) {
                                            // NUMBER TOO BIG
                                            rplError(ERR_NUMBERTOOBIG);
                                            RetNum=ERR_SYNTAX;
                                            return;
                                        }

                                        bpack[used-1]|=((used-1)|sign)<<4;  // STARTER BYTE


                                        if(count==0) temp.word=0;
                                        --used;
                                        temp.bytes[count]=bpack[used];
                                        ++count;
                                        BINT k;
                                        for(k=0;k<used;++k) {
                                        if(count>3) {
                                            ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                                            rplCompileAppend(temp.word);
                                            ptr=(BYTEPTR)ScratchPointer1;
                                            temp.word=0;
                                            count=0;
                                        }
                                        temp.bytes[count]=bpack[k];
                                        ++count;
                                        }

                                        isnum=0;

                }
                // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                if(count) rplCompileAppend(temp.word);
                *ScratchPointer4=MKPROLOG(DOPLOT+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                RetNum=OK_NEEDMORE;
                return;
            }

            ucode=utf82cp((char *)ptr,(char *)BlankStart);


            if(isnum==0) {
            if(ucode=='+') {
                // START A NUMBER
                isnum=1;
                number=0;
            }
            else if(ucode=='-') {
                // START A NUMBER
                isnum=-1;
                number=0;
            }
            else if((ucode>='0')&&(ucode<='9')) {
                // START A NUMBER
                isnum=1;
                number=ucode-'0';
            }
            else if((WORD)ucode==ARG_SEP(Locale)) {

                // SKIP ARGUMENT SEPARATORS
            }
            else if((ucode>=0x60)&&(ucode<=0x7f)) {
                // IT'S A PRIMITIVE
                if(count==0) temp.word=0;
                temp.bytes[count]=ucode;
                ++count;

            }
            else if(ucode=='\"') {
                // START A STRING COMPILE
                stringmode=1;
                ScratchPointer3=(WORDPTR)(((BYTEPTR)CompileEnd)+count); // POINT TO THE COMPILED START OF STRING

                if(count==0) temp.word=0;
                temp.bytes[count]=0x50;
                ++count;
                if(count==4) {
                //  WE HAVE A COMPLETE WORD HERE
                ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                rplCompileAppend(temp.word);
                ptr=(BYTEPTR)ScratchPointer1;

                count=0;
                temp.word=0;
                }
                temp.bytes[count]=0;
                ++count;
                if(count==4) {
                //  WE HAVE A COMPLETE WORD HERE
                ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                rplCompileAppend(temp.word);
                ptr=(BYTEPTR)ScratchPointer1;

                count=0;
                temp.word=0;
                }
                temp.bytes[count]=0;
                ++count;
                if(count==4) {
                //  WE HAVE A COMPLETE WORD HERE
                ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                rplCompileAppend(temp.word);
                ptr=(BYTEPTR)ScratchPointer1;

                count=0;
                temp.word=0;
                }



                ++ptr;


                            BINT addedbytes=0;
                            // COMPILE IT JUST LIKE A STRING
                            do {
                            while(count<4) {
                                if(ptr==(BYTEPTR)NextTokenStart) {
                                 // WE ARE AT THE END OF THE GIVEN STRING, STILL NO CLOSING QUOTE, SO WE NEED MORE

                                    // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                                    if(count) rplCompileAppend(temp.word);
                                    *ScratchPointer4=MKPROLOG(DOPLOT+4+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                                    RetNum=OK_NEEDMORE;

                                    // UPDATE THE STRING SIZE AT START OF STRING
                                    BYTEPTR nptr=(BYTEPTR)ScratchPointer3;  // RESTORE SAVED STRING START LOCATION

                                    addedbytes+=rplPlotNumber2BINT(nptr);   // ADD THE NEW BYTES TO THE ORIGINAL SIZE

                                    if(addedbytes>=0x100000) {
                                        // STRING IS TOO BIG!
                                        RetNum=ERR_SYNTAX;
                                        return;
                                    }

                                    // AND UPDATE THE NUMBER
                                    nptr[0]=0x50|((addedbytes>>16)&0xf);
                                    nptr[2]=(addedbytes>>8)&0xff;
                                    nptr[1]=(addedbytes)&0xff;


                                    return;
                                }

                                if(*ptr=='\"') {
                                    // END OF STRING!
                                    ++ptr;
                                    // WE HAVE REACHED THE END OF THE STRING
                                    stringmode=0;

                                    // UPDATE THE STRING SIZE AT START OF STRING
                                    BYTEPTR nptr=(BYTEPTR)ScratchPointer3;  // RESTORE SAVED STRING START LOCATION

                                    if(nptr+3>(BYTEPTR)CompileEnd) {
                                        // SPECIAL CASE WHERE STRING IS SHORT AND INITIAL COUNT
                                        // WASN'T STORED YET
                                        // JUST STORE IT TEMPORARILY TO READ THE NUMBER PROPERLY
                                        // USES TEMPOB GUARANTEED SLACK
                                        *CompileEnd=temp.word;
                                    }

                                    addedbytes+=rplPlotNumber2BINT(nptr);   // ADD THE NEW BYTES TO THE ORIGINAL SIZE

                                    if(addedbytes>=0x100000) {
                                        // STRING IS TOO BIG!
                                        RetNum=ERR_SYNTAX;
                                        return;
                                    }

                                    // AND UPDATE THE NUMBER
                                    nptr[0]=0x50|((addedbytes>>16)&0xf);
                                    nptr[2]=(addedbytes>>8)&0xff;
                                    nptr[1]=(addedbytes)&0xff;


                                    if(nptr+3>(BYTEPTR)CompileEnd) {
                                        // SPECIAL CASE WHERE STRING IS SHORT AND INITIAL COUNT
                                        // WASN'T STORED YET
                                        temp.word=*CompileEnd;
                                    }

                                    // CONTINUE COMPILING
                                    break;
                                }

                                if(count==0) temp.word=0;
                                temp.bytes[count]=*ptr;
                                ++addedbytes;
                                ++count;
                                ++ptr;


                                }
                                if(count==4) {
                                //  WE HAVE A COMPLETE WORD HERE
                                ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                                rplCompileAppend(temp.word);
                                ptr=(BYTEPTR)ScratchPointer1;

                                count=0;
                                }
                                else break;


                            } while(1);     // DANGEROUS! BUT WE WILL RETURN FROM THE CHECK WITHIN THE INNER LOOP;



            }
            else {
                // SYNTAX ERROR
                RetNum=ERR_SYNTAX;
                return;
            }

            }
            else {
               // WITHIN A NUMBER ONLY NUMBERS ARE ACCEPTED
                if((ucode>='0')&&(ucode<='9')) {
                                // ADD ANOTHER DIGIT
                                number=number*10+(ucode-'0');
                            }
                else if((WORD)ucode==ARG_SEP(Locale)) {

                    // END THE NUMBER
                    // COMPILE ITS BYTES
                    // PACK THE NUMBER INTO A UBINT64 (UP TO 5 BYTES USED)
                    BINT used=0;
                    BYTE bpack[8];
                    BINT sign;

                    if(isnum<0) sign=8;
                    else sign=0;

                    while(number) {
                        bpack[used]=number&0xff;
                        ++used;
                        number>>=8;
                    }

                    if(!used) { bpack[0]=0; used=1; }
                    if(bpack[used-1]>0xf) { bpack[used]=0; ++used; }
                    if(used>5) {
                        // NUMBER TOO BIG
                        rplError(ERR_NUMBERTOOBIG);
                        RetNum=ERR_SYNTAX;
                        return;
                    }

                    bpack[used-1]|=((used-1)|sign)<<4;  // STARTER BYTE


                    if(count==0) temp.word=0;
                    --used;
                    temp.bytes[count]=bpack[used];
                    ++count;
                    BINT k;
                    for(k=0;k<used;++k) {
                    if(count>3) {
                        ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                        rplCompileAppend(temp.word);
                        ptr=(BYTEPTR)ScratchPointer1;
                        temp.word=0;
                        count=0;
                    }
                    temp.bytes[count]=bpack[k];
                    ++count;
                    }

                    isnum=0;

                } else {
                    RetNum=ERR_SYNTAX;
                    return;
                }

            }

            ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)BlankStart);
            continue;

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
            // DECOMPILE PLOT OBJECT

            rplDecompAppendString((BYTEPTR)"PLOTDATA ");

            BINT64 Locale=rplGetSystemLocale();
            BYTEPTR ptr=(BYTEPTR)(DecompileObject+1);
            BYTEPTR end=ptr+PLTLEN(*DecompileObject);
            BINT needscomma=0;


            while(ptr<end) {

                if((*ptr>=0x60) && (*ptr<=0x7f)) {

                    if(*ptr=='~') {
                        break;
                    }


                    BINT off=ptr-(BYTEPTR)DecompileObject;
                    if(needscomma) rplDecompAppendUTF8(cp2utf8(ARG_SEP(Locale)));
                    else needscomma=1;
                    rplDecompAppendChar(*ptr);
                    ptr=((BYTEPTR)DecompileObject)+off;
                }
                else if(((*ptr>>4)&7)<0x5) {
                    // IT'S A NUMBER
                    BINT64 num=rplPlotNumber2BINT(ptr);

                    // NOW OUTPUT THE NUMBER IN DECIMAL

                    BYTE tmpbuffer[12]; // 2^36 USES 10 DIGITS + SIGN MAX.

                    BINT nbytes=rplIntToString(num,DECBINT,tmpbuffer,tmpbuffer+12);

                    BINT off=ptr-(BYTEPTR)DecompileObject;
                    if(needscomma) rplDecompAppendUTF8(cp2utf8(ARG_SEP(Locale)));
                    else needscomma=1;
                    rplDecompAppendString2(tmpbuffer,nbytes);

                    ptr=((BYTEPTR)DecompileObject)+off;

                }
                else if((*ptr>>4)==0x5){
                    // OUTPUT A STRING
                    BINT64 len=rplPlotNumber2BINT(ptr);

                    BINT off=ptr-(BYTEPTR)DecompileObject;
                    if(needscomma) rplDecompAppendUTF8(cp2utf8(ARG_SEP(Locale)));
                    else needscomma=1;
                    rplDecompAppendChar('\"');
                    rplDecompAppendString2(ptr+3,len);
                    rplDecompAppendChar('\"');
                    ptr=((BYTEPTR)DecompileObject)+off;

                }


                ptr=rplPlotSkip(ptr);

            }


            rplDecompAppendString((BYTEPTR)" ENDPLOT");

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


