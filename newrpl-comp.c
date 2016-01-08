/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include "newrpl.h"
#include "libraries.h"
#include "ui.h"

#include <stdio.h>
#include <string.h>












// DISPLAY AN ERROR MESSAGE
// USES ERROR CODE FROM SYSTEM Exceptions
// OUTPUTS THE ERROR TO THE GIVEN STREAM (USUALLY stderr)
void compShowErrorMsg(char *inputfile,char *mainbuffer,long long length,FILE *stream)
{
        int errbit;
        if(!Exceptions) return;

        fprintf(stream,"%s:%d:%d:",inputfile,1,1);

        if(Exceptions!=EX_ERRORCODE) {
            if(ExceptionPointer && (*ExceptionPointer!=0)) {  // ONLY IF THERE'S A VALID COMMAND TO BLAME
            WORDPTR cmdname=halGetCommandName(ExceptionPointer);
            if(cmdname) {
            BYTEPTR start=(BYTEPTR)(cmdname+1);
            BYTEPTR end=start+rplStrSize(cmdname);

            fwrite(start,1,end-start,stream);

            }
            }
            fprintf(stream," Exception: ");

            BINT ecode;
            for(errbit=0;errbit<8;++errbit)     // THERE'S ONLY A FEW EXCEPTIONS IN THE NEW ERROR MODEL
            {
            if(Exceptions&(1<<errbit)) {
                ecode=MAKEMSG(0,errbit);
                BYTEPTR message=halGetMessage(ecode);
                fwrite((char *)message,1,strlen((char *)message),stream);
                break;
            }
            }
        }
        else {
            // TRY TO DECOMPILE THE OPCODE THAT CAUSED THE ERROR
            if(ExceptionPointer &&(*ExceptionPointer!=0)) {  // ONLY IF THERE'S A VALID COMMAND TO BLAME
            WORDPTR cmdname=halGetCommandName(ExceptionPointer);
            if(cmdname) {
            BYTEPTR start=(BYTEPTR)(cmdname+1);
            BYTEPTR end=start+rplStrSize(cmdname);

            fwrite(start,1,end-start,stream);
            }
            }
            fprintf(stream," error: ");
            // TODO: GET NEW TRANSLATABLE MESSAGES
            BYTEPTR message=halGetMessage(ErrorCode);
            fwrite((char *)message,1,strlen((char *)message),stream);
        }
    fprintf(stream,"\n");
}

enum {
    OUTPUT_BINARY,
    OUTPUT_C
};



int main(int argc, char *argv[])
{

    char *mainbuffer;

    if(argc<2) {
    printf("NewRPL standalone compiler - Version 1.0\n");
    printf("Usage: newrpl-comp [-c] [-o <outputfile>] <filename.nrpl>\n");
    printf("\nOptions:\n");
    printf("\t\t-c\tOutput will be as C source code.\n");
    printf("\t\t-o <file>\tSpecify a output file name (defaults to filename.c or filename.binrpl)\n\n\n");
    return 0;
    }

    int argidx=1;
    int outputtype=OUTPUT_BINARY;
    int needoutputname=0;
    int needcleanup=0;
    char *outputfile=NULL;
    char *inputfile=NULL;
    while(argidx<argc) {
        if(needoutputname) { outputfile=argv[argidx]; needoutputname=0; }
        else  if((argv[argidx][0]=='-')&&(argv[argidx][1]=='c')&&(argv[argidx][2]==0)) outputtype=OUTPUT_C;
            else if((argv[argidx][0]=='-')&&(argv[argidx][1]=='o')) {
                if(argv[argidx][2]==0) needoutputname=1;
                else outputfile=argv[argidx]+2;
                } else inputfile=argv[argidx];

        ++argidx;
    }

    // HERE WE HAVE ALL ARGUMENTS PROCESSED
    if(!inputfile) {
        fprintf(stderr,"Error: No input file\n");
        return 1;
    }

    if(!outputfile) {

        // CREATE AN OUTPUT FILE NAME FROM THE INPUT FILE
        char *end=inputfile+strlen(inputfile)-1;
        while((end>inputfile)&&(*end!='.')&&(*end!='/')&&(*end!='\\')) --end;
        if(end<=inputfile) end=inputfile+strlen(inputfile);
        else if(*end!='.') end=inputfile+strlen(inputfile);
        needcleanup++;
        outputfile=malloc(end-inputfile+10);
        if(!outputfile) {
            fprintf(stderr,"Memory allocation error\n");
            return 1;
        }
        memmove(outputfile,inputfile,end-inputfile);
        strcpy(outputfile+(end-inputfile),(outputtype==OUTPUT_C)? ".c":".binrpl");

    }


    printf("Input file: %s\n",inputfile);
    printf("Output file: %s\n",outputfile);
    printf("Output type: %s\n",(outputtype==OUTPUT_C)? "C":"Binary");

    // READ THE INPUT FILE INTO A BUFFER
    FILE *f=fopen(inputfile,"rb");
    if(f==NULL) {
        fprintf(stderr,"Error: File not found %s\n",inputfile);
        if(needcleanup) free(outputfile);
        return 1;
    }
    fseek(f,0,SEEK_END);
    long long length=ftell(f);
    fseek(f,0,SEEK_SET);

    mainbuffer=malloc(length);
    if(!mainbuffer) {
        fprintf(stderr,"Memory allocation error\n");
        if(needcleanup) free(outputfile);
        return 1;
    }
    if(fread(mainbuffer,1,length,f)!=(size_t)length) {
        fprintf(stderr,"Can't read from input file\n");
        if(needcleanup) free(outputfile);
        return 1;
    }
    fclose(f);

    // HERE WE HAVE THE MAIN FILE


    // TODO: ADD SOME PREPROCESSING HERE


    // TODO: COMPILE THE CODE AND DISPLAY ANY ERRORS
    rplInit();

    WORDPTR newobject=rplCompile((BYTEPTR)mainbuffer,length,1);

    if(Exceptions) {
        compShowErrorMsg(inputfile,mainbuffer,length,stderr);
        if(needcleanup) free(outputfile);
        free(mainbuffer);
        return 1;
    }

    printf("Compile OK\n");
    // TODO: GET READY TO OUTPUT THE FILE

    return 0;

}
