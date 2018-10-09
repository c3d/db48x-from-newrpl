/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// STANDARD COMPILER FOR COMMAND TOKENS
// COMMON TO ALL LIBRARIES THAT DEFINE ONLY COMMANDS
// STARTING TO COUNT FROM COMMAND NUMBER 0
void libCompileCmds(BINT libnum,char *libnames[],WORD libopcodes[],int numcmds)
{
    int idx;
    int len;
    for(idx=0;idx<numcmds;++idx)
    {
        len=utf8len((char *)libnames[idx]);
        if((len!=0) && (len==(BINT)TokenLen) && (!utf8ncmp2((char *)TokenStart,(char *)BlankStart,(char *)libnames[idx],len)))
       {
            if(libopcodes) rplCompileAppend((WORD) MKOPCODE(libnum,libopcodes[idx]));
           else rplCompileAppend((WORD) MKOPCODE(libnum,idx));
           RetNum=OK_CONTINUE;
           return;
       }
    }
    RetNum=ERR_NOTMINE;
}

// STANDARD DECOMPILER FOR COMMAND TOKENS
// COMMON TO ALL LIBRARIES THAT DEFINE ONLY COMMANDS

void libDecompileCmds(char *libnames[],WORD libopcodes[],int numcmds)
{
WORD opc=OPCODE(*DecompileObject);
int idx;

if(libopcodes) {
    for(idx=0;idx<numcmds;++idx)
    {
        if(libopcodes[idx]==opc) break;
    }
} else idx=opc;
if(idx>=numcmds) {
    RetNum=ERR_INVALID;
    return;
}

rplDecompAppendString((BYTEPTR)libnames[idx]);
RetNum=OK_CONTINUE;
}



// STANDARD PROBETOKEN FOR COMMANDS
// COMMON TO ALL LIBRARIES THAT DEFINE ONLY COMMANDS
// STARTING TO COUNT FROM COMMAND NUMBER 0
void libProbeCmds(char *libnames[],BINT tokeninfo[],int numcmds)
{
    int idx;
    int len;
    int maxidx=-1,maxlen=0;

    // SCAN THROUGH ALL COMMANDS AND FIND LONGEST MATCH
    for(idx=0;idx<numcmds;++idx)
    {
        len=utf8len((char *)libnames[idx]);
        if((len>0) && (len<=(BINT)TokenLen) && (!utf8ncmp2((char *)TokenStart,(char *)BlankStart,(char *)libnames[idx],len)))
        {
            // WE HAVE A MATCH, STORE THE INDEX BEFORE WE MAKE ANY DECISIONS
            if(len>maxlen) { maxidx=idx; maxlen=len; }
       }
    }

    if(maxlen!=0) {
    if(tokeninfo) {
        RetNum=OK_TOKENINFO | tokeninfo[maxidx];
    } else RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_NOTALLOWED,0,0);
    }
    else RetNum=ERR_NOTMINE;
}


// STANDARD GETINFO FOR COMMANDS
// COMMON TO ALL LIBRARIES THAT DEFINE ONLY COMMANDS
// STARTING TO COUNT FROM COMMAND NUMBER 0
// THIS VERSION TAKES A VECTOR WITH OPCODE NUMBERS
void libGetInfo(WORD opcode,char *libnames[],WORD libopcodes[],BINT tokeninfo[],int numcmds)
    {
        int idx;
        int len;
        opcode=OPCODE(opcode);
        for(idx=0;idx<numcmds;++idx)
        {
            if(libopcodes[idx]==opcode)
           {
                if(tokeninfo) {
                    RetNum=OK_TOKENINFO | tokeninfo[idx];
                } else {
                    len=utf8len(libnames[idx]);
                    RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_NOTALLOWED,0,0);
                }
               return;
           }
        }
        RetNum=ERR_NOTMINE;
    }

// STANDARD GETINFO FOR COMMANDS
// COMMON TO ALL LIBRARIES THAT DEFINE ONLY COMMANDS
// THIS VERSION ASSUMES THE INDEX IS THE OPCODE NUMBER
void libGetInfo2(WORD opcode,char *libnames[],BINT tokeninfo[],int numcmds)
    {
        int idx;
        int len;
        idx=OPCODE(opcode);
        if(idx<numcmds)
        {
                if(tokeninfo) {
                    RetNum=OK_TOKENINFO | tokeninfo[idx];
                } else {
                    len=utf8len(libnames[idx]);
                    RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_NOTALLOWED,0,0);
                }
               return;

        }
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_NOTALLOWED,0,0);
    }


void libGetRomptrID(BINT libnum,WORDPTR *table,WORDPTR ptr)
{
    BINT idx=0;
    while(table[idx]) {
        if( (ptr>=table[idx]) && (ptr<table[idx]+rplObjSize(table[idx]))) {
            BINT offset=ptr-table[idx];
            ObjectID=MKROMPTRID(libnum,idx,offset);
            RetNum=OK_CONTINUE;
            return;
        }
        ++idx;
    }
    RetNum=ERR_NOTMINE;
    return;
}

void libGetPTRFromID(WORDPTR *table,WORD id)
{
    BINT idx=0;
    while(table[idx]) ++idx;
    if(ROMPTRID_IDX(id)>=idx) {
        RetNum=ERR_NOTMINE;
        return;
    }
    ObjectPTR=table[ROMPTRID_IDX(id)]+ROMPTRID_OFF(id);
    RetNum=OK_CONTINUE;
}


// STANDARD AUTOCOMPLETE FOR COMMANDS
// COMMON TO ALL LIBRARIES THAT DEFINE COMMANDS
// STARTING TO COUNT FROM COMMAND NUMBER 0
void libAutoCompleteNext(BINT libnum,char *libnames[],int numcmds)
{
    // TokenStart = token string
    // TokenLen = token length
    // SuggestedOpcode = OPCODE OF THE CURRENT SUGGESTION, OR THE PROLOG OF THE OBJECT IF SUGGESTION IS AN OBJECT
    // SuggestedObject = POINTER TO AN OBJECT (ONLY VALID IF ISPROLOG(SuggestedOpcode)==True)

    WORD Prolog=SuggestedOpcode;

    if(LIBNUM(Prolog)<(WORD)libnum) {
        // COMMANDS ARE SUGGESTED BEFORE ANY OBJECTS
        // SO IF THE PREVIOUS RESULT WAS AN OBJECT, WE ARE DONE HERE
        RetNum=ERR_NOTMINE;
        return;
    }
    BINT idx,len;

    if(!ISPROLOG(Prolog) && (LIBNUM(Prolog)==(WORD)libnum)) idx=OPCODE(Prolog)-1;
    else idx=numcmds-1;

    while(idx>=0) {
        len=utf8len((char *)libnames[idx]);
        if((len>=(BINT)TokenLen) && (!utf8ncmp2((char *)TokenStart,(char *)BlankStart,(char *)libnames[idx],TokenLen)))
        {
            // WE HAVE THE NEXT MATCH
            SuggestedOpcode=MKOPCODE(libnum,idx);
            RetNum=OK_CONTINUE;
            return;
        }
        // NOW CHECK IF FIRST LETTER OF COMMAND IS NOT A LETTER
        if( ((*libnames[idx]>='A') && (*libnames[idx]<='Z')) ||
            ((*libnames[idx]>='a') && (*libnames[idx]<='z')) )
        {
            --idx;
        }
        else {
        // SKIP THE FIRST CHARACTER AND CHECK AGAIN
            --len;
            if((len>=(BINT)TokenLen) && (!utf8ncmp2((char *)TokenStart,(char *)BlankStart,utf8skipst((char *)libnames[idx],(char *)libnames[idx]+4),TokenLen)))
            {
                // WE HAVE THE NEXT MATCH
                SuggestedOpcode=MKOPCODE(libnum,idx);
                RetNum=OK_CONTINUE;
                return;
            }

            --idx;

        }
    }

    RetNum=ERR_NOTMINE;
}
/*
void libAutoCompletePrev(BINT libnum,char *libnames[],int numcmds)
{
    // TokenStart = token string
    // TokenLen = token length
    // SuggestedOpcode = OPCODE OF THE CURRENT SUGGESTION, OR 0 IF SUGGESTION IS AN OBJECT
    // SuggestedObject = POINTER TO AN OBJECT (ONLY VALID IF SuggestedOpcode==0)

    WORD Prolog=SuggestedOpcode;

    if(!Prolog) Prolog=*SuggestedObject;

    if(LIBNUM(Prolog)>libnum) {
        // ALREADY PAST HERE
        RetNum=ERR_NOTMINE;
        return;
    }

    BINT idx,len;

    if(LIBNUM(Prolog)==libnum) {
        if(ISPROLOG(Prolog)) idx=0;
        else idx=OPCODE(Prolog)+1;
    }
    else idx=0;

    while(idx<numcmds) {
        len=utf8len((char *)libnames[idx]);
        if((len>=(BINT)TokenLen) && (!utf8ncmp2((char *)TokenStart,(char *)BlankStart,(char *)libnames[idx],TokenLen)))
        {
            // WE HAVE THE NEXT MATCH
            SuggestedOpcode=MKOPCODE(libnum,idx);
            RetNum=OK_CONTINUE;
            return;
        }
        ++idx;
    }

    RetNum=ERR_NOTMINE;
}
*/

// LOOSELY BASED ON JDB2, CHANGES INITIAL PRIME NUMBER AND USES MERSENNE PRIME 2^13-1 AS MULTIPLIER
WORD libComputeHash(WORDPTR object)
{
WORDPTR end=rplSkipOb(object);
WORD hash=115127;
while(object!=end) {
    hash=((hash<<13)-hash)+ *object;
    ++object;
}
return hash;
}



// FINDS A TEXT MESSAGE IN A TABLE IN THE FORM
// { #MSGNUMBER "Message" ...  0 }
// SET ObjectPTR TO THE TEXT MESSAGE AND RETURN IF THE MESSAGE IS FOUND
// THE KEY CAN BE EITHER #MSGNUMBER OR A COMMAND, OR THE HASH OF AN OBJECT

void libFindMsg(BINT message,WORDPTR table)
{

    WORD key;

    if(!ISLIST(*table)) { RetNum=ERR_NOTMINE; return; }
    ++table;
    while(*table!=CMD_ENDLIST) {
        key=*table;
        if(ISPROLOG(key)) {
            // THE KEY IS AN OBJECT, USE AN OBJECT HASH
            key=libComputeHash(table);
        }
        if(key==(WORD)message) {
            ObjectPTR=rplSkipOb(table);
            if(!ISSTRING(*ObjectPTR)) { RetNum=ERR_NOTMINE; return; }
            RetNum=OK_CONTINUE;
            return;
        }
        table=rplSkipOb(table);
        if(*table==CMD_ENDLIST) { RetNum=ERR_NOTMINE; return; }
        table=rplSkipOb(table);
    }
}
