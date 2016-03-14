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



extern void lib4079_handler();








// DISPLAY AN ERROR MESSAGE
// USES ERROR CODE FROM SYSTEM Exceptions
// OUTPUTS THE ERROR TO THE GIVEN STREAM (USUALLY stderr)
void compShowErrorMsg(char *inputfile,char *mainbuffer,FILE *stream)
{
        int errbit;
        if(!Exceptions) return;
        char *position = (char *)TokenStart;
        char *linestart=NULL;

        // COMPUTE LINE NUMBER
        int linenum=1;

        while(position>mainbuffer) {
            --position;
            if(*position=='\n') {
                ++linenum;
                if(!linestart) linestart=position+1;
            }
        }

        // COUNT CHARACTERS FROM START OF LINE
        position=(char *)TokenStart;

        while(*linestart=='\r') ++linestart;

        int posnum=utf8nlen(linestart,position)+1;


        fprintf(stream,"%s:%d:%d:",inputfile,linenum,posnum);

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
            fprintf(stderr,"error: Memory allocation error\n");
            return 1;
        }
        memmove(outputfile,inputfile,end-inputfile);
        strcpy(outputfile+(end-inputfile),(outputtype==OUTPUT_C)? ".c":".binrpl");

    }


    // READ THE INPUT FILE INTO A BUFFER
    FILE *f=fopen(inputfile,"rb");
    if(f==NULL) {
        fprintf(stderr,"error: File not found %s\n",inputfile);
        if(needcleanup) free(outputfile);
        return 1;
    }
    fseek(f,0,SEEK_END);
    long long length=ftell(f);
    fseek(f,0,SEEK_SET);

    mainbuffer=malloc(length);
    if(!mainbuffer) {
        fprintf(stderr,"error: Memory allocation error\n");
        if(needcleanup) free(outputfile);
        return 1;
    }
    if(fread(mainbuffer,1,length,f)!=(size_t)length) {
        fprintf(stderr,"error: Can't read from input file\n");
        if(needcleanup) free(outputfile);
        free(mainbuffer);
        return 1;
    }
    fclose(f);

    // HERE WE HAVE THE MAIN FILE
    rplInit();
    rplSetSystemFlag(FL_STRIPCOMMENTS);
    rplInstallLibrary(lib4079_handler);


    // IDENTIFY CHUNKS OF CODE

    char *chunk=mainbuffer;
    int numchunks=0;
    char *chunkstart[65537]; // MAXIMUM NUMBER OF VARIABLES IN A SINGLE FILE


    while(chunk-mainbuffer<length) {

    if(!utf8ncmp(chunk,"@#name",6)) {
        // FOUND START OF CHUNK
        chunkstart[numchunks]=chunk;
        ++numchunks;
        if(numchunks>65535) {
            fprintf(stderr,"error: Too many chunks in same file.\n");
            if(needcleanup) free(outputfile);
            free(mainbuffer);
            return 1;

        }

    }

    //SKIP TO THE NEXT LINE
    while( (*chunk!='\n')&&(*chunk!='\r')&&(chunk-mainbuffer<length)) ++chunk;
    while( ((*chunk=='\n')||(*chunk=='\r'))&&(chunk-mainbuffer<length)) ++chunk;

    }


    if(numchunks==0) {
        chunkstart[numchunks]=mainbuffer;
        ++numchunks;
    }


    chunkstart[numchunks]=chunk;



    f=fopen(outputfile,"wb");
    if(f==NULL) {
        fprintf(stderr,"error: Can't open %s for writing.\n",outputfile);
        if(needcleanup) free(outputfile);
        free(mainbuffer);
        return 1;
    }

    if(outputtype==OUTPUT_BINARY) {
        // WRITE THE BINARY FILE MARKER
        WORD marker[3]= {
            MKPROLOG(HEXBINT,2),        // PROLOG OF A 64-BIT BINT
            0x4c50526e,                 // STRING "nRPL"
            1                           // VERSION OF THE newRPL BINARY FORMAT
        };

        fwrite(marker,4,3,f);
    }
    else {
        fprintf(f,"// newRPL binary version 1.0\n\n");
        fprintf(f,"#include \"libraries.h\"\n\n");
    }



    // COMPILE ALL CHUNKS


    int k;
    char *start,*end;
    for(k=0;k<numchunks;++k) {

        start=chunkstart[k];
        end=chunkstart[k+1];

        if(!utf8ncmp(start,"@#name",6)) {
        // SKIP TO THE NEXT LINE FOR THE REAL DATA
        while( (*start!='\n')&&(*start!='\r')&&(start<end)) ++start;
        while( ((*start=='\n')||(*start=='\r'))&&(start<end)) ++start;
        }

       if(end>start) {

    WORDPTR newobject=rplCompile((BYTEPTR)start,end-start,1);

    if(Exceptions) {
        compShowErrorMsg(inputfile,mainbuffer,stderr);
        fclose(f);
        remove(outputfile);

        if(needcleanup) free(outputfile);
        free(mainbuffer);
        return 1;
    }

        // OUTPUT THE CHUNK

        if(outputtype==OUTPUT_C) {

            if(rplObjSize(newobject)>2) {
            // OUTPUT C FORMATTED CODE

            char *objname;
            char *nameend;
            if(!utf8ncmp(chunkstart[k],"@#name",6)) {
                objname=chunkstart[k]+6;
                while( (*objname==' ') || (*objname=='\t')) ++objname;
                nameend=objname;
                while( (*nameend!='\n') && (*nameend!='\r') && (*nameend!=' ') && (*nameend!='\t')) ++nameend;
            } else { nameend=objname=NULL; }

            fprintf(f,"ROMOBJECT ");
            if(nameend<=objname) fprintf(f,"chunk%05d",k+1);
            else fwrite(objname,1,nameend-objname,f);

            fprintf(f,"[]= {\n");

            WORDPTR p=newobject+1,endp=rplSkipOb(newobject)-1;

            int wordcount=0;
            while(p<endp) {

                if(LIBNUM(*p)==4079) {
                    // THIS IS A PSEUDO-OBJECT, NEEDS TO BE REPLACED WITH TEXT
                    int textoffset;
                    int givenwords,storedwords;
                    if(ISPROLOG(*p)) { textoffset=p[1]; storedwords=OBJSIZE(*p)+1; }
                    else { textoffset=OBJSIZE(*p); storedwords=1; }

                    if(textoffset>0) {

                    char *foundtext=end-textoffset,*endtext;
                    if(foundtext>=start) {
                        // IT'S A VALID POINTER INTO THE TEXT!

                        // FIND THE END OF THE TEXT
                        givenwords=1;
                        endtext=foundtext;
                        while(endtext<end) {
                            if(*endtext==',') ++givenwords;
                            if(*endtext==' ') break;
                            if(*endtext=='\t') break;
                            if(*endtext=='\n') break;
                            if(*endtext=='\r') break;
                            ++endtext;
                        }

                        // REMOVED THIS CHECK TO ALLOW FOR C MACROS THAT EXPAND TO MORE THAN ONE WORD
                        //if(givenwords==storedwords) {
                        // IT'S A VALID SPECIAL OBJECT
                        fwrite(foundtext,1,endtext-foundtext,f);
                        p+=storedwords;
                        if(p!=endp) fprintf(f,",");
                        if((wordcount&7)+storedwords>7)    fprintf(f,"\n");
                        wordcount+=storedwords;
                        continue;

                        //}




                    }

                    rplError(ERR_SYNTAXERROR);
                    TokenStart=(WORDPTR)foundtext;
                    compShowErrorMsg(inputfile,mainbuffer,stderr);

                }

                    // IF IT FALLS THROUGH, THEN IT'S AN INVALID SPECIAL SEQUENCE
                    // JUST CONTINUE NORMALLY, AS IT COULD BE RANDOM DATA
                }


                fprintf(f,"0x%08X",*p);
                if(p!=endp-1) fprintf(f,",");

                wordcount++;
                if((wordcount&7)==0) fprintf(f,"\n");
                ++p;
            }


            if((wordcount&7)!=0) fprintf(f,"\n");

            fprintf(f,"};\n\n\n");
            }

        }
        else {
            if(rplObjSize(newobject)>2) {
            // RAW BINARY OUTPUT
            fwrite(newobject+1,4,rplObjSize(newobject)-2,f);
            }
        }



       }

        // AND MOVE ON TO THE NEXT ONE

    }



    // CLOSE THE OUTPUT FILE
    fclose(f);

    return 0;

}
