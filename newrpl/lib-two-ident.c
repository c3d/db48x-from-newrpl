/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// THIS LIBRARY PROVIDES ONLY COMPILATION OF IDENTS AFTER ALL OTHER LIBRARIES
// HAD A CHANCE TO IDENTIFY THEIR COMMANDS
// ANY LAM COMMANDS HAVE TO BE IN A SEPARATE LIBRARY


// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************



// REPLACE THE NUMBER
#define LIBRARY_NUMBER  2


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

//#define COMMAND_LIST
//    CMD(CMDNAME,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2))
//    ECMD(CMDNAME,"CMDNAME",MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************



// THESE ARE SPECIAL OPCODES FOR THE COMPILER ONLY
// THE LOWER 16 BITS ARE THE NUMBER OF LAMS TO CREATE, OR THE INDEX OF LAM NUMBER TO STO/RCL
//#define NEWNLOCALS 0x40000   // SPECIAL OPCODE TO CREATE NEW LOCAL VARIABLES
#define GETLAMNEVAL    0x30000   // SPECIAL OPCODE TO RCL THE CONTENT OF A LAM AND EVAL (XEQ ITS CONTENT)
//#define GETLAMN    0x20000   // SPECIAL OPCODE TO RCL THE CONTENT OF A LAM
//#define PUTLAMN    0x10000   // SPECIAL OPCODE TO STO THE CONTENT OF A LAM


void rplCompileIDENT(BINT libnum,BYTEPTR tok,BYTEPTR tokend)
{
    // WE HAVE A VALID QUOTED IDENT, CREATE THE OBJECT
    BINT lenwords=(tokend-tok+3)>>2;
    BINT len=tokend-tok;
    ScratchPointer1=(WORDPTR)tok;
    rplCompileAppend(MKPROLOG(libnum,lenwords));
    WORD nextword;
    tok=(BYTEPTR )ScratchPointer1;
    while(len>3) {
        // WARNING: THIS IS LITTLE ENDIAN ONLY!
        nextword=tok[0]+(tok[1]<<8)+(tok[2]<<16)+(tok[3]<<24);
        ScratchPointer1=(WORDPTR)tok;
        rplCompileAppend(nextword);
        tok=(BYTEPTR )ScratchPointer1;
        tok+=4;
        len-=4;
    }
    if(len) {
    nextword=0;
    BINT rot=0;
    while(len) {
        // WARNING: THIS IS LITTLE ENDIAN ONLY!
        nextword|=(*tok)<<rot;
        --len;
        ++tok;
        rot+=8;
    }
    rplCompileAppend(nextword);
    }
    // DONE

}

// ALLOCATES MEMORY AND CREATES AN IDENT OBJECT
// RETURNS NULL ON ERROR, DOESN'T CHECK IF IDENT IS VALID!
// USER MUST CALL rplIsValidIdent() BEFORE CALLING THIS FUNCTION

WORDPTR rplCreateIDENT(BINT libnum,BYTEPTR tok,BYTEPTR tokend)
{
    // CREATE THE OBJECT
    BINT lenwords=(tokend-tok+3)>>2;
    BINT len=tokend-tok;

    ScratchPointer1=(WORDPTR)tok;

    WORDPTR newobj=rplAllocTempOb(lenwords),newptr;
    if(!newobj) return 0;
    newptr=newobj;
    *newptr=MKPROLOG(libnum,lenwords);
    ++newptr;
    WORD nextword;
    tok=(BYTEPTR )ScratchPointer1;
    while(len>3) {
        // WARNING: THIS IS LITTLE ENDIAN ONLY!
        nextword=tok[0]+(tok[1]<<8)+(tok[2]<<16)+(tok[3]<<24);
        *newptr=nextword;
        ++newptr;
        tok+=4;
        len-=4;
    }
    if(len) {
    nextword=0;
    BINT rot=0;
    while(len) {
        // WARNING: THIS IS LITTLE ENDIAN ONLY!
        nextword|=(*tok)<<rot;
        --len;
        ++tok;
        rot+=8;
    }
    *newptr=nextword;
    }
    // DONE
 return newobj;
}

// THESE ARE THE ONLY CHARACTERS THAT ARE FORBIDDEN IN AN IDENTIFIER
const char const forbiddenChars[]="+-*/\\{}[]()#!^;:<>=, \"\'_`@|√«»≤≥≠∡";


BINT rplIsValidIdent(BYTEPTR tok,BYTEPTR tokend)
{
    BYTEPTR ptr;
    BINT char1,char2;
    if(tokend<=tok) return 0;

    // SKIP ANY INITIAL DOTS
    while((tok!=tokend)&&(*tok=='.')) ++tok;

    // IDENT CANNOT START WITH A NUMBER
    if( (((char)*tok)>='0') && (((char)*tok)<='9')) return 0;

    // OR CONTAIN ANY OF THE FORBIDDEN CHARACTERS
    while(tok!=tokend)
    {
        ptr=(BYTEPTR )forbiddenChars;
        char1=utf82cp((char *)tok,(char *)tokend);
        do {
        char2=utf82cp((char *)ptr,(char *)ptr+4);
        if(char1==char2) return 0;
        ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)ptr+4);
        } while(*ptr);
        tok=(BYTEPTR)utf8skip((char *)tok,(char *)tokend);
    }
    return 1;
}

// DETERMINE THE LENGTH OF AN IDENT STRING IN BYTES (NOT CHARACTERS)
BINT rplGetIdentLength(WORDPTR ident)
{
    BINT len=OBJSIZE(*ident);
    WORD lastword=*(ident+len);
    BINT usedbytes=0;
    while( !(lastword&0xff000000) && (usedbytes<4) ) { lastword<<=8; ++usedbytes; }
    usedbytes=4-usedbytes;

    return ((len-1)<<2)+usedbytes;
}


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
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

       // COMPILE IDENTS QUOTED AND UNQUOTED, AND IF CurrentConstruct== NEWLOCALENV THEN ADD QUOTES
        {
        BYTEPTR tok=(BYTEPTR )TokenStart;
        BINT len=TokenLen;

        if(*tok=='\'') {
                // QUOTED IDENT OR ALGEBRAIC OBJECT
                if(*utf8nskip((char *)tok,(char *)BlankStart,TokenLen-1)!='\'') {
                    // NOT A SIMPLE IDENT, THEN IT'S A SYMBOLIC EXPRESSION
                    rplCompileAppend(MKPROLOG(DOSYMB,0));

                    if(TokenLen>1) {
                        NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                    }
                    RetNum=OK_STARTCONSTRUCT_INFIX;
                    return;
                }

                ++tok;
                len-=2;

                if(!rplIsValidIdent(tok,((BYTEPTR)BlankStart)-1)) {

                        // NOT A SIMPLE IDENT, THEN IT'S A SYMBOLIC EXPRESSION
                        rplCompileAppend(MKPROLOG(DOSYMB,0));

                        if(TokenLen>1) {
                            NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                        }
                        RetNum=OK_STARTCONSTRUCT_INFIX;
                        return;
                }


                rplCompileIDENT(DOIDENT,tok,((BYTEPTR)BlankStart)-1);

                RetNum=OK_CONTINUE;
                return;
        }



        // UNQUOTED IDENTS


        if(!rplIsValidIdent(tok,(BYTEPTR)BlankStart)) {
            if( (*tok>='0')&&(*tok<='9')) {
                // IDENT STARTS WITH A NUMBER, THERE'S IMPLICIT MULTIPLICATION


                LIBHANDLER hanreal=rplGetLibHandler(DOREAL);
                WORD saveopcode=CurOpcode;
                RetNum=-1;
                CurOpcode=MKOPCODE(DOREAL,OPCODE_PROBETOKEN);
                    (*hanreal)();
                CurOpcode=saveopcode;

                    if(RetNum<OK_TOKENINFO) {
                        RetNum=ERR_SYNTAX;
                        return;
                    }
                // WE POSSIBLY HAVE IMPLICIT MULTIPLICATION BETWEEN REAL AND IDENT
                BINT numberlen=TI_LENGTH(RetNum);
                BYTEPTR splitpoint=(BYTEPTR)utf8nskip((char *)tok,(char *)BlankStart,numberlen);
                BINT splitoff=splitpoint-(BYTEPTR)TokenStart;
                if(rplIsValidIdent(splitpoint,(BYTEPTR)BlankStart)) {
                    // CONFIRMED IMPLICIT MULTIPLICATION
                    // TRY TO COMPILE AS NUMBER IDENT *
                    WORD  locale=rplGetSystemLocale();
                    newRealFromText(&RReg[0],(char *)tok,(char *)splitpoint,locale);
                    if(RReg[0].flags&F_ERROR) {
                        RetNum=ERR_SYNTAX;
                        return;
                    }

                    if(isintegerReal(&RReg[0]) && inBINT64Range(&RReg[0])) {
                        rplCompileBINT(getBINT64Real(&RReg[0]),(RReg[0].flags&F_APPROX)? DECBINTAPP:DECBINT);
                    }
                    else rplCompileReal(&RReg[0]);
                    tok=((BYTEPTR)TokenStart)+splitoff;
                }


            } else {
         RetNum=ERR_SYNTAX;
         return;
            }
        }

        if(CurrentConstruct==CMD_NEWLOCALENV) {
            // INSIDE THIS CONSTRUCT WE NEED TO QUOTE ALL
            // IDENTS

            rplCompileIDENT(DOIDENT,tok,(BYTEPTR)BlankStart);

            RetNum=OK_CONTINUE;
            return;
        }
        if(CurrentConstruct==MKPROLOG(DOSYMB,0)) {
            // INSIDE SYMBOLICS, ALL IDENTS ARE UNQUOTED

            rplCompileIDENT(DOIDENTEVAL,tok,(BYTEPTR)BlankStart);

            RetNum=OK_CONTINUE;
            return;
        }


        // CHECK IF IT'S A LAM, COMPILE TO A GETLAM OPCODE IF IT IS

        WORDPTR *LAMptr=rplFindLAMbyName(tok,len,1);


        if(LAMptr<LAMTopSaved) {
            // THIS IS NOT A VALID LAM, COMPILE AS AN UNQUOTED IDENT

            rplCompileIDENT(DOIDENTEVAL,tok,(BYTEPTR)BlankStart);

            RetNum=OK_CONTINUE;
            return;
        }

        if(LAMptr<nLAMBase) {
            // THIS IS A LAM FROM AN UPPER CONSTRUCT
            // WE CAN USE GETLAM ONLY INSIDE LOOPS, NEVER ACROSS SECONDARIES

            WORDPTR *env=nLAMBase;
            WORD prolog;
            do {
                if(LAMptr>env) break;
                prolog=**(env+1);   // GET THE PROLOG OF THE SECONDARY
                if(ISPROLOG(prolog) && LIBNUM(prolog)==SECO) {
                // LAMS ACROSS << >> SECONDARIES HAVE TO BE COMPILED AS IDENTS
                rplCompileIDENT(DOIDENTEVAL,tok,(BYTEPTR)BlankStart);

                RetNum=OK_CONTINUE;
                return;
                }
                env=rplGetNextLAMEnv(env);
            } while(env);



        }


        // SPECIAL CASE: WHEN A SECO DOESN'T HAVE ANY LOCALS YET
        // BUT LAMS FROM THE PREVIOUS SECO SHOULDN'T BE COMPILED TO GETLAMS

        // SCAN ALL CURRENT CONSTRUCTS TO FIND THE INNERMOST SECONDARY
        // THEN VERIFY IF THAT SECONDARY IS THE CURRENT LAM ENVIRONMENT

        // THIS IS TO FORCE ALL LAMS IN A SECO TO BE COMPILED AS IDENTS
        // INSTEAD OF GETLAMS

        // DOCOL'S ARE OK AND ALWAYS COMPILED AS GETLAMS
        WORDPTR *scanenv=ValidateTop-1;

        while(scanenv>=RSTop) {
            if( (LIBNUM(**scanenv)==SECO)&& (ISPROLOG(**scanenv))) {
                    // FOUND INNERMOST SECONDARY
                    if(*scanenv>*(nLAMBase+1)) {
                        // THE CURRENT LAM BASE IS OUTSIDE THE INNER SECONDARY
                    rplCompileIDENT(DOIDENTEVAL,tok,(BYTEPTR)BlankStart);

                    RetNum=OK_CONTINUE;
                    return;
                    }
                    break;

            }
            --scanenv;
        }

        // IT'S A KNOWN LOCAL VARIABLE, COMPILE AS GETLAM
        // BUT ONLY IF WE ARE NOT INSIDE A COMPOSITE (LIST, ARRAY, ETC)
        if( (CurrentConstruct==MKPROLOG(DOLIST,0)) ||
            (CurrentConstruct==MKPROLOG(DOMATRIX,0)) // ADD HERE ARRAYS LATER
                ) {
            rplCompileIDENT(DOIDENTEVAL,tok,(BYTEPTR)BlankStart);

            RetNum=OK_CONTINUE;
            return;
        }
        BINT Offset=((BINT)(LAMptr-nLAMBase))>>1;

        if(Offset<=32767 && Offset>=-32768) {
        rplCompileAppend(MKOPCODE(DOIDENT,GETLAMNEVAL+(Offset&0xffff)));
        }
        else {
            rplCompileIDENT(DOIDENTEVAL,tok,(BYTEPTR)BlankStart);
        }

        RetNum=OK_CONTINUE;
        return;





        }


     return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors


        // NOTHING TO DECOMPILE HERE. THIS LIBRARY DOES NOT DEFINE ANY COMMANDS OR OBJECTS.

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
    {
        BYTEPTR tokptr,tokend;
        BINT maxlen,len;

        tokptr=(BYTEPTR)TokenStart;
        tokend=(BYTEPTR)BlankStart;

        tokptr=(BYTEPTR)utf8skipst((char *)tokptr,(char *)tokend);
        for(maxlen=0,len=1;tokptr<=tokend;++len) {
            if(!rplIsValidIdent((BYTEPTR)TokenStart,tokptr)) break;
            maxlen=len;
            if(tokptr==tokend) break;
            tokptr=(BYTEPTR)utf8skipst((char *)tokptr,(char *)tokend);
        }
        if(maxlen>0) RetNum=OK_TOKENINFO | MKTOKENINFO(maxlen,TITYPE_IDENT,0,1);
        else RetNum=ERR_NOTMINE;
        return;
    }

    case OPCODE_GETINFO:
    {
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_IDENT,0,1);
        return;
    }

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        RetNum=ERR_NOTMINE;
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        RetNum=ERR_NOTMINE;
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
        //libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        RetNum=ERR_NOTMINE;
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
