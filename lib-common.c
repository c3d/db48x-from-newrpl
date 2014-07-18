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
        len=strlen((char *)libnames[idx]);
        if((len==(BINT)TokenLen) && (!strncmp((char *)TokenStart,(char *)libnames[idx],len)))
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
int opc=OPCODE(*DecompileObject),idx;

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
void libProbeCmds(BINT libnum,char *libnames[],BINT tokeninfo[],int numcmds)
{
    int idx;
    int len;
    for(idx=0;idx<numcmds;++idx)
    {
        len=strlen((char *)libnames[idx]);
        if((len<=(BINT)TokenLen) && (!strncmp((char *)TokenStart,(char *)libnames[idx],len)))
       {
            if(tokeninfo) {
                RetNum=OK_TOKENINFO | tokeninfo[idx];
            } else RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_NOTALLOWED,0,0);
           return;
       }
    }
    RetNum=ERR_NOTMINE;
}
// STANDARD GETINFO FOR COMMANDS
// COMMON TO ALL LIBRARIES THAT DEFINE ONLY COMMANDS
// STARTING TO COUNT FROM COMMAND NUMBER 0
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
                    len=strlen(libnames[idx]);
                    RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_NOTALLOWED,0,0);
                }
               return;
           }
        }
        RetNum=ERR_NOTMINE;
    }
