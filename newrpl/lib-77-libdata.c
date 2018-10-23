/*
 * Copyright (c) 2017, Claudio Lapilli and the newRPL Team
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
#define LIBRARY_NUMBER  77

//@TITLE=Arbitrary data containers

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(MKBINDATA,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(BINPUTB,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(BINGETB,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(BINPUTW,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(BINGETW,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(BINPUTOBJ,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(BINGETOBJ,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(BINMOVB,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(BINMOVW,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2))



// ADD MORE OPCODES HERE

#define ERROR_LIST \
    ERR(INVALIDSIZE,0), \
    ERR(BINDATAEXPECTED,1), \
    ERR(READOUTSIDEOBJECT,2), \
    ERR(WRITEOUTSIDEOBJECT,3)




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
INCLUDE_ROMOBJECT(lib77_menu);


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib77_menu,

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
        case MKBINDATA:
        {
            //@SHORT_DESC=Create binary data container object
            //@NEW
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            BINT64 sizebytes=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) return;


            if(sizebytes<0) {
                rplError(ERR_INVALIDSIZE);
                return;
            }

            sizebytes+=3;
            sizebytes>>=2;

            WORDPTR newobj=rplAllocTempOb(sizebytes);
            if(!newobj) return;

            newobj[0]=MKPROLOG(LIBRARY_NUMBER,sizebytes);

            // FOR SPEED, DON'T CLEAR THE DATA, USER IS RESPONSIBLE FOR DATA INITIALIZATION

            rplOverwriteData(1,newobj);
            return;


        }
    case BINMOVB:
    {
        //@SHORT_DESC=Copy binary data block into a binary data object
        //@NEW
        // ARGUMENTS: DEST_BINDATA DEST_OFFSET SOURCE_BINDATA SRC_OFFSET NBYTES

        if(rplDepthData()<5) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINDATA(*rplPeekData(5))) {
            rplError(ERR_BINDATAEXPECTED);
            return;
        }

        BINT destsize=sizeof(WORD)*rplObjSize(rplPeekData(5));

        BINT64 destoffset=rplReadNumberAsBINT(rplPeekData(4));

        if(Exceptions) return;  // A NUMBER WAS EXPECTED

        destoffset+=4;

        // DO NOT CHECK THE SOURCE OF THE DATA ON PURPOSE, BUT DO CHECK THAT THE NUMBER OF BYTES ARE AVAILABLE

        BINT64 srcoffset=rplReadNumberAsBINT(rplPeekData(2));
        if(Exceptions) return;  // A NUMBER WAS EXPECTED
        srcoffset+=4;
        BINT64 nbytes=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;  // A NUMBER WAS EXPECTED

        BINT srcsize=sizeof(WORD)*rplObjSize(rplPeekData(3));

        if(srcoffset+nbytes>srcsize) {\
            rplError(ERR_READOUTSIDEOBJECT);
            return;
        }

        if(destoffset+nbytes>destsize) {\
            rplError(ERR_WRITEOUTSIDEOBJECT);
            return;
        }

        // EVERYTHING WORKS

        WORDPTR newobj=rplMakeNewCopy(rplPeekData(5));
        if(!newobj) return; // NOT ENOUGH MEMORY
        memmoveb(((BYTEPTR)newobj)+destoffset,((BYTEPTR)rplPeekData(3))+srcoffset,nbytes);

        rplOverwriteData(5,newobj);
        rplDropData(4);
        return;

    }


    case BINGETB:
    {
        //@SHORT_DESC=Extract binary data as list of bytes
        //@NEW
        // ARGUMENTS: SOURCE_BINDATA SRC_OFFSET NBYTES

        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // DO NOT CHECK THE SOURCE OF THE DATA ON PURPOSE, BUT DO CHECK THAT THE NUMBER OF BYTES ARE AVAILABLE

        BINT64 srcoffset=rplReadNumberAsBINT(rplPeekData(2));
        if(Exceptions) return;  // A NUMBER WAS EXPECTED
        srcoffset+=4;
        BINT64 nbytes=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;  // A NUMBER WAS EXPECTED

        BINT srcsize=sizeof(WORD)*rplObjSize(rplPeekData(3));

        if(srcoffset+nbytes>srcsize) {\
            rplError(ERR_READOUTSIDEOBJECT);
            return;
        }


        // EVERYTHING WORKS

        WORDPTR newobj=rplAllocTempOb(nbytes+1);
        if(!newobj) return; // NOT ENOUGH MEMORY
        BINT k;

        BYTEPTR ptr=(BYTEPTR)rplPeekData(3);
        ptr+=srcoffset;

        for(k=1;k<=nbytes;++k,++ptr)
        {
            newobj[k]=MAKESINTH(*ptr);
        }
        newobj[k]=CMD_ENDLIST;
        newobj[0]=MKPROLOG(DOLIST,nbytes+1);

        rplOverwriteData(3,newobj);
        rplDropData(2);
        return;

    }

    case BINPUTB:
    {
        //@SHORT_DESC=Store bytes into binary data object
        //@NEW
        // ARGUMENTS: DEST_BINDATA DEST_OFFSET SOURCE_OBJECT NBYTES

        if(rplDepthData()<4) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        BINT destsize=sizeof(WORD)*rplObjSize(rplPeekData(4));

        BINT64 destoffset=rplReadNumberAsBINT(rplPeekData(3));

        if(Exceptions) return;  // A NUMBER WAS EXPECTED

        destoffset+=4;

        // DO NOT CHECK THE SOURCE OF THE DATA ON PURPOSE, BUT DO CHECK THAT THE NUMBER OF BYTES ARE AVAILABLE

        BINT64 nbytes=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;  // A NUMBER WAS EXPECTED


        if(destoffset+nbytes>destsize) {\
            rplError(ERR_WRITEOUTSIDEOBJECT);
            return;
        }

        if(ISNUMBER(*rplPeekData(2))) {
            // EXTRACT DATA FROM AN INTEGER NUMBER, STORE AS LITTLE ENDIAN BINARY INTEGER
            REAL num;

            rplReadNumberAsReal(rplPeekData(2),&num);
            if(Exceptions) return;



            WORDPTR newobj=rplMakeNewCopy(rplPeekData(4));
            if(!newobj) return; // NOT ENOUGH MEMORY


            int k;
            BINT byte_num;
            BYTEPTR ptr=(BYTEPTR)newobj;
            ptr+=destoffset;



            ipReal(&RReg[0],&num,0);

            RReg[0].flags&=~F_NEGATIVE;

            rplBINTToRReg(3,256);


            // EXTRACT BYTES FROM THE BINARY INTEGER
            for(k=0;k<nbytes;++k) {

            divmodReal(&RReg[1],&RReg[2],&RReg[0],&RReg[3]);

            byte_num=(BYTE)getBINTReal(&RReg[2]);
            if(num.flags&F_NEGATIVE) {
                byte_num^=0xff;
                if(!k) byte_num+=1;
            }

            ptr[k]=byte_num;
            swapReal(&RReg[1],&RReg[0]);

            }


            rplOverwriteData(4,newobj);
            rplDropData(3);
            return;
        }


        if(ISSTRING(*rplPeekData(2))) {
           // STORE THE STRING AS A STREAM OF UNICODE CODEPOINTS
           // ONLY THE LOWER 8 BITS OF EACH CODEPOINT IS STORED

            WORDPTR newobj=rplMakeNewCopy(rplPeekData(4));
            if(!newobj) return; // NOT ENOUGH MEMORY



            int k;
            BINT byte_num;
            BYTEPTR ptr=(BYTEPTR)newobj;
            ptr+=destoffset;

            BYTEPTR src=(BYTEPTR)rplPeekData(2);
            BYTEPTR srcend=src+4+rplStrSize(rplPeekData(2));
            BINT strlen=rplStrLenCp(rplPeekData(2));


            src+=4;


            // EXTRACT BYTES FROM THE UTF-8 STREAM
            for(k=0;k<nbytes;++k) {

            if(strlen>0) { byte_num=utf82cp((char *)src,(char *)srcend)&0xff; --strlen; src=(BYTEPTR)utf8skip((char *)src,(char *)srcend); }
            else byte_num=0;

            ptr[k]=byte_num;

            }


            rplOverwriteData(4,newobj);
            rplDropData(3);
            return;


        }

        if(ISLIST(*rplPeekData(2))) {
           // A LIST OF INTEGER NUMBERS, STORE THE LOWEST 8 BITS OF EACH

            BINT listlen=rplListLength(rplPeekData(2));
            WORDPTR listptr=rplGetListElement(rplPeekData(2),1);
            REAL num;
            int k;
            BINT byte_num;

            WORDPTR newobj=rplMakeNewCopy(rplPeekData(4));
            if(!newobj) return; // NOT ENOUGH MEMORY


            BYTEPTR ptr=(BYTEPTR)newobj;
            ptr+=destoffset;

            rplBINTToRReg(3,256);

            // EXTRACT BYTES FROM THE BINARY INTEGER
            for(k=0;k<nbytes;++k) {
            if(ISNUMBER(*listptr)) {
            rplReadNumberAsReal(listptr,&num);
            if(Exceptions) return;

            if(inBINTRange(&num)) byte_num=getBINTReal(&num)&0xff;
            else {
            divmodReal(&RReg[1],&RReg[2],&num,&RReg[3]);
            byte_num=(BYTE)getBINTReal(&RReg[2]);
            if(num.flags&F_NEGATIVE) {
                byte_num^=0xff;
                byte_num+=1;
            }

            }

            ptr[k]=byte_num;
            listptr=rplSkipOb(listptr);
            }
            else {
                if(k<listlen-1) {
                    rplError(ERR_INTEGEREXPECTED);
                    return;
                }
                ptr[k]=0;
            }

            }


            rplOverwriteData(4,newobj);
            rplDropData(3);
            return;


        }

        rplError(ERR_BADARGTYPE);
        return;


    }

    case BINPUTW:
    {
        //@SHORT_DESC=Store 32-bit words into binary data object
        //@NEW
        // ARGUMENTS: DEST_BINDATA DEST_OFFSET SOURCE_OBJECT NBYTES

        if(rplDepthData()<4) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        BINT destsize=rplObjSize(rplPeekData(4));

        BINT64 destoffset=rplReadNumberAsBINT(rplPeekData(3));

        if(Exceptions) return;  // A NUMBER WAS EXPECTED

        destoffset+=1;

        // DO NOT CHECK THE SOURCE OF THE DATA ON PURPOSE, BUT DO CHECK THAT THE NUMBER OF BYTES ARE AVAILABLE

        BINT64 nwords=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;  // A NUMBER WAS EXPECTED


        if(destoffset+nwords>destsize) {\
            rplError(ERR_WRITEOUTSIDEOBJECT);
            return;
        }

        if(ISNUMBER(*rplPeekData(2))) {
            // EXTRACT DATA FROM AN INTEGER NUMBER, STORE AS LITTLE ENDIAN BINARY INTEGER
            REAL num;

            rplReadNumberAsReal(rplPeekData(2),&num);
            if(Exceptions) return;



            WORDPTR newobj=rplMakeNewCopy(rplPeekData(4));
            if(!newobj) return; // NOT ENOUGH MEMORY


            int k;
            WORD word_num;
            WORDPTR ptr=newobj;
            ptr+=destoffset;



            ipReal(&RReg[0],&num,0);

            RReg[0].flags&=~F_NEGATIVE;

            rplBINTToRReg(3,0x100000000LL);


            // EXTRACT WORDS FROM THE BINARY INTEGER
            for(k=0;k<nwords;++k) {

            divmodReal(&RReg[1],&RReg[2],&RReg[0],&RReg[3]);

            word_num=getBINTReal(&RReg[2]);
            if(num.flags&F_NEGATIVE) {
                word_num^=0xffffffff;
                if(!k) word_num+=1;
            }

            ptr[k]=word_num;
            swapReal(&RReg[1],&RReg[0]);

            }


            rplOverwriteData(4,newobj);
            rplDropData(3);
            return;
        }


        if(ISSTRING(*rplPeekData(2))) {
           // STORE THE STRING AS A STREAM OF UNICODE CODEPOINTS
           // ONLY THE LOWER 8 BITS OF EACH CODEPOINT IS STORED

            WORDPTR newobj=rplMakeNewCopy(rplPeekData(4));
            if(!newobj) return; // NOT ENOUGH MEMORY



            int k;
            WORD word_num;
            WORDPTR ptr=newobj;
            ptr+=destoffset;

            BYTEPTR src=(BYTEPTR)rplPeekData(2);
            BYTEPTR srcend=src+4+rplStrSize(rplPeekData(2));
            BINT strlen=rplStrLenCp(rplPeekData(2));


            src+=4;


            // EXTRACT WORDS FROM THE UTF-8 STREAM
            for(k=0;k<nwords;++k) {

            if(strlen>0) { word_num=(WORD)utf82cp((char *)src,(char *)srcend); --strlen; src=(BYTEPTR)utf8skip((char *)src,(char *)srcend); }
            else word_num=0;

            ptr[k]=word_num;

            }


            rplOverwriteData(4,newobj);
            rplDropData(3);
            return;


        }

        if(ISLIST(*rplPeekData(2))) {
           // A LIST OF INTEGER NUMBERS, STORE THE LOWEST 32 BITS OF EACH

            BINT listlen=rplListLength(rplPeekData(2));
            WORDPTR listptr=rplGetListElement(rplPeekData(2),1);
            REAL num;
            int k;
            WORD word_num;

            WORDPTR newobj=rplMakeNewCopy(rplPeekData(4));
            if(!newobj) return; // NOT ENOUGH MEMORY


            WORDPTR ptr=newobj;
            ptr+=destoffset;

            rplBINTToRReg(3,0x100000000LL);

            // EXTRACT BYTES FROM THE BINARY INTEGER
            for(k=0;k<nwords;++k) {
            if(ISNUMBER(*listptr)) {
            rplReadNumberAsReal(listptr,&num);
            if(Exceptions) return;

            if(inBINTRange(&num)) word_num=getBINTReal(&num);
            else {
            divmodReal(&RReg[1],&RReg[2],&num,&RReg[3]);
            word_num=getBINTReal(&RReg[2]);
            if(num.flags&F_NEGATIVE) {
                word_num^=0xff;
                word_num+=1;
            }

            }

            ptr[k]=word_num;
            listptr=rplSkipOb(listptr);
            }
            else {
                if(k<listlen-1) {
                    rplError(ERR_INTEGEREXPECTED);
                    return;
                }
                ptr[k]=0;
            }

            }


            rplOverwriteData(4,newobj);
            rplDropData(3);
            return;


        }

        rplError(ERR_BADARGTYPE);
        return;


    }



    case BINMOVW:
    {
        //@SHORT_DESC=Copy 32-bit words between binary data objects
        //@NEW
        // ARGUMENTS: DEST_BINDATA DEST_OFFSET SOURCE_BINDATA SRC_OFFSET NWORDS

        if(rplDepthData()<5) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINDATA(*rplPeekData(5))) {
            rplError(ERR_BINDATAEXPECTED);
            return;
        }

        BINT destsize=rplObjSize(rplPeekData(5));

        BINT64 destoffset=rplReadNumberAsBINT(rplPeekData(4));

        if(Exceptions) return;  // A NUMBER WAS EXPECTED

        destoffset+=1;

        // DO NOT CHECK THE SOURCE OF THE DATA ON PURPOSE, BUT DO CHECK THAT THE NUMBER OF BYTES ARE AVAILABLE

        BINT64 srcoffset=rplReadNumberAsBINT(rplPeekData(2));
        if(Exceptions) return;  // A NUMBER WAS EXPECTED
        srcoffset+=1;
        BINT64 nwords=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;  // A NUMBER WAS EXPECTED

        BINT srcsize=rplObjSize(rplPeekData(3));

        if(srcoffset+nwords>srcsize) {\
            rplError(ERR_READOUTSIDEOBJECT);
            return;
        }

        if(destoffset+nwords>destsize) {\
            rplError(ERR_WRITEOUTSIDEOBJECT);
            return;
        }

        // EVERYTHING WORKS

        WORDPTR newobj=rplMakeNewCopy(rplPeekData(5));
        if(!newobj) return; // NOT ENOUGH MEMORY
        memmovew(newobj+destoffset,rplPeekData(3)+srcoffset,nwords);

        rplOverwriteData(5,newobj);
        rplDropData(4);
        return;

    }


    case BINGETW:
    {
        //@SHORT_DESC=Extract data from a binary data object as a list of 32-bit words
        //@NEW
        // ARGUMENTS: SOURCE_BINDATA SRC_OFFSET NBYTES

        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // DO NOT CHECK THE SOURCE OF THE DATA ON PURPOSE, BUT DO CHECK THAT THE NUMBER OF BYTES ARE AVAILABLE

        BINT64 srcoffset=rplReadNumberAsBINT(rplPeekData(2));
        if(Exceptions) return;  // A NUMBER WAS EXPECTED
        srcoffset+=1;
        BINT64 nwords=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;  // A NUMBER WAS EXPECTED

        BINT srcsize=rplObjSize(rplPeekData(3));

        if(srcoffset+nwords>srcsize) {
            rplError(ERR_READOUTSIDEOBJECT);
            return;
        }


        // EVERYTHING WORKS

        WORDPTR newobj=rplAllocTempOb(nwords*3+1);
        if(!newobj) return; // NOT ENOUGH MEMORY
        BINT k;

        WORDPTR ptr=rplPeekData(3);
        ptr+=srcoffset;

        for(k=0;k<nwords;++k,++ptr)
        {
            newobj[3*k+1]=MKPROLOG(HEXBINT,2);
            newobj[3*k+2]=*ptr;
            newobj[3*k+3]=0;
        }
        newobj[3*k+1]=CMD_ENDLIST;
        newobj[0]=MKPROLOG(DOLIST,3*nwords+1);

        rplOverwriteData(3,newobj);
        rplDropData(2);
        return;

    }

    case BINPUTOBJ:
    {
        //@SHORT_DESC=Store an entire object into a binary data container
        //@NEW
        // ARGUMENTS: DEST_BINDATA DEST_OFFSET SOURCE_OBJECT

        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISBINDATA(*rplPeekData(3))) {
            rplError(ERR_BINDATAEXPECTED);
            return;
        }

        BINT destsize=sizeof(WORD)*rplObjSize(rplPeekData(3));

        BINT64 destoffset=rplReadNumberAsBINT(rplPeekData(2));

        if(Exceptions) return;  // A NUMBER WAS EXPECTED

        destoffset+=4;

        // DO NOT CHECK THE SOURCE OF THE DATA ON PURPOSE, BUT DO CHECK THAT THE NUMBER OF BYTES ARE AVAILABLE

        BINT nbytes=sizeof(WORD)*rplObjSize(rplPeekData(1));

        if(destoffset+nbytes>destsize) {\
            rplError(ERR_WRITEOUTSIDEOBJECT);
            return;
        }

        // EVERYTHING WORKS

        WORDPTR newobj=rplMakeNewCopy(rplPeekData(3));
        if(!newobj) return; // NOT ENOUGH MEMORY
        memmoveb(((BYTEPTR)newobj)+destoffset,(BYTEPTR)rplPeekData(1),nbytes);

        rplOverwriteData(3,newobj);
        rplDropData(2);
        return;

    }


    case BINGETOBJ:
    {
        //@SHORT_DESC=Extract an entire object from a binary data container
        //@NEW
        // ARGUMENTS: SOURCE_BINDATA SRC_OFFSET

        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // DO NOT CHECK THE SOURCE OF THE DATA ON PURPOSE, BUT DO CHECK THAT THE NUMBER OF BYTES ARE AVAILABLE

        BINT64 srcoffset=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;  // A NUMBER WAS EXPECTED
        srcoffset+=4;

        BYTEPTR ptr=(BYTEPTR)rplPeekData(2);

        ptr+=srcoffset;

        WORD prolog=ptr[0] | (ptr[1]<<8) | (ptr[2]<<16) | (ptr[3]<<24);

        BINT nwords=(ISPROLOG(prolog)? OBJSIZE(prolog):0);

        if(((1+nwords)*sizeof(WORD)+srcoffset)>sizeof(WORD)*rplObjSize(rplPeekData(2))) {
            rplError(ERR_READOUTSIDEOBJECT);
            return;
        }

        // AT LEAST WE CAN READ AN OBJECT FROM THERE

        WORDPTR newobj=rplAllocTempOb(nwords);
        if(!newobj) return; // NOT ENOUGH MEMORY

        memmoveb(newobj,ptr,(1+nwords)*sizeof(WORD));   // EXTRACT THE ENTIRE OBJECT

        // VERIFY THE OBJECT JUST IN CASE

        if(!rplVerifyObject(newobj)) {
            rplError(ERR_MALFORMEDOBJECT);
            return;
        }


        rplOverwriteData(2,newobj);
        rplDropData(1);
        return;

    }

    case OVR_SAME:
    // COMPARE AS PLAIN OBJECTS, THIS INCLUDES SIMPLE COMMANDS IN THIS LIBRARY
        {
         BINT same=rplCompareObjects(rplPeekData(1),rplPeekData(2));
         rplDropData(2);
         if(same) rplPushTrue(); else rplPushFalse();
         return;
        }



    case OVR_ISTRUE:
    {
        rplOverwriteData(1,(WORDPTR)one_bint);
        return;
    }


    case OVR_FUNCEVAL:
    case OVR_EVAL:
    case OVR_EVAL1:
    case OVR_XEQ:
        // ALSO EXECUTE THE OBJECT
        if(!ISPROLOG(*rplPeekData(1))) {
               // EXECUTE THE COMMAND BY CALLING THE HANDLER DIRECTLY
                WORD saveOpcode=CurOpcode;
                CurOpcode=*rplPopData();
                // RECURSIVE CALL
                LIB_HANDLER();
                CurOpcode=saveOpcode;
                return;
            }

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

        if((TokenLen==7) && (!utf8ncmp2((char *)TokenStart,(char *)BlankStart,"BINDATA",7))) {

            ScratchPointer4=CompileEnd;
            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER,0));
            RetNum=OK_NEEDMORE;
            return;
        }


            // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
            // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
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

            rplDecompAppendString((BYTEPTR)"BINDATA ");
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

