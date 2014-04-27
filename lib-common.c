#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// STANDARD COMPILER FOR COMMAND TOKENS
// COMMON TO ALL LIBRARIES THAT DEFINE ONLY COMMANDS
// STARTING TO COUNT FROM COMMAND NUMBER 0
void libCompileCmds(int libnum,char *libnames[],int libopcodes[],int numcmds)
{
    int idx;
    int len;
    for(idx=0;idx<numcmds;++idx)
    {
        len=strlen((char *)libnames[idx]);
        if((len==ArgNum1) && (!strncmp((char *)TokenStart,(char *)libnames[idx],len)))
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

void libDecompileCmds(char *libnames[],int libopcodes[],int numcmds)
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

