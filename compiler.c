/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include <string.h>


#define EXIT_LOOP -10000

void rplCompileAppend(WORD word)
{
    *CompileEnd=word;
    CompileEnd++;
    // ADJUST MEMORY AS NEEDED
    if( CompileEnd>=TempObSize) {
        // ENLARGE TEMPOB AS NEEDED
        growTempOb( ((WORD)(CompileEnd-TempOb))+TEMPOBSLACK);
    }

}

// COMPILE A STRING AND RETURN A POINTER TO THE FIRST COMMAND/OBJECT
// IF addwrapper IS NON-ZERO, IT WILL WRAP THE CODE WITH :: ... ; EXITRPL
// (USED BY THE COMMAND LINE FOR IMMEDIATE COMMANDS)

WORDPTR rplCompile(BYTEPTR string, BINT addwrapper)
{
    // COMPILATION USES TEMPOB
    CompileEnd=TempObEnd;
    static const char const wrapperstart[]="::";
    static const char const wrapperend[]="; EXITRPL";

    // START COMPILATION LOOP
    BINT force_libnum;
    LIBHANDLER handler,ValidateHandler;
    BINT libcnt,libnum;

    LAMTopSaved=LAMTop;     // SAVE LAM ENVIRONMENT

    ValidateTop=RSTop;
    ValidateHandler=NULL;

    force_libnum=-1;

    if(addwrapper) { NextTokenStart=(WORDPTR)wrapperstart; addwrapper=1; }
    else NextTokenStart=(WORDPTR)string;

    do {


    // FIND THE START OF NEXT TOKEN
    while( (*((BYTEPTR)NextTokenStart)==' ') || (*((BYTEPTR)NextTokenStart)=='\t') || (*((BYTEPTR)NextTokenStart)=='\n') || (*((BYTEPTR)NextTokenStart)=='\r')) NextTokenStart=(WORDPTR)(((BYTEPTR)NextTokenStart)+1);



    do {
        TokenStart=NextTokenStart;
        BlankStart=TokenStart;
        while( (*((char *)BlankStart)) && (*((char *)BlankStart)!=' ') && (*((char *)BlankStart)!='\t') && (*((char *)BlankStart)!='\n') && (*((char *)BlankStart)!='\r')) BlankStart=(WORDPTR)(((char *)BlankStart)+1);
        NextTokenStart=BlankStart;
        while( (*((char *)NextTokenStart)==' ') || (*((char *)NextTokenStart)=='\t') || (*((char *)NextTokenStart)=='\n') || (*((char *)NextTokenStart)=='\r')) NextTokenStart=(WORDPTR)(((char *)NextTokenStart)+1);;

        TokenLen=(BINT)((BYTEPTR)BlankStart-(BYTEPTR)TokenStart);
        BlankLen=(BINT)((BYTEPTR)NextTokenStart-(BYTEPTR)BlankStart);
        CurrentConstruct=(BINT)((ValidateTop>RSTop)? **(ValidateTop-1):0);      // CARRIES THE WORD OF THE CURRENT CONSTRUCT/COMPOSITE

        if(force_libnum<0) {
            // SCAN THROUGH ALL THE LIBRARIES, FROM HIGH TO LOW, TO SEE WHICH ONE WANTS THE TOKEN
            libcnt=MAXLOWLIBS+NumHiLibs-1;
        }
        else libcnt=0;  // EXECUTE THE LOOP ONLY ONCE

                while(libcnt>=0) {
                if(libcnt>=MAXLOWLIBS) { libnum=HiLibNumbers[libcnt-MAXLOWLIBS]; handler=HiLibRegistry[libcnt-MAXLOWLIBS]; }
                else {
                    if(force_libnum<0) { libnum=libcnt; handler=LowLibRegistry[libcnt]; }
                    else { libnum=force_libnum; handler=rplGetLibHandler(force_libnum); }
                }
                --libcnt;

                if(!handler) continue;

                if(force_libnum!=-1) CurOpcode=MKOPCODE(libnum,OPCODE_COMPILECONT);
                    else CurOpcode=MKOPCODE(libnum,OPCODE_COMPILE);

                (*handler)();

                switch(RetNum)
                {
                case OK_CONTINUE:
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;
                    break;
                case OK_STARTCONSTRUCT:
                    if(RStkSize<=(ValidateTop-RStk)) growRStk(ValidateTop-RStk+RSTKSLACK);
                    if(Exceptions) { LAMTop=LAMTopSaved; return 0; }
                    *ValidateTop++=CompileEnd-1; // POINTER TO THE WORD OF THE COMPOSITE, NEEDED TO STORE THE SIZE
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;

                    break;
                case OK_CHANGECONSTRUCT:
                    *(ValidateTop-1)=CompileEnd-1; // POINTER TO THE WORD OF THE COMPOSITE, NEEDED TO STORE THE SIZE
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;
                    break;
                case OK_ENDCONSTRUCT:
                    --ValidateTop;
                    if(ValidateTop<RSTop) {
                        Exceptions|=EX_SYNTAXERROR;
                        ExceptionPointer=IPtr;
                        LAMTop=LAMTopSaved;
                        return 0;
                    }
                    if(ISPROLOG((BINT)**ValidateTop)) **ValidateTop|= (((WORD)CompileEnd-(WORD)*ValidateTop)>>2)-1;    // STORE THE SIZE OF THE COMPOSITE IN THE WORD
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;
                    break;

                case OK_NEEDMORE:
                    force_libnum=libnum;
                    libcnt=EXIT_LOOP;
                    break;
                case OK_NEEDMORESTARTCONST:
                    if(RStkSize<=(ValidateTop-RStk)) growRStk(ValidateTop-RStk+RSTKSLACK);
                    if(Exceptions) { LAMTop=LAMTopSaved; return 0; }
                    *ValidateTop++=CompileEnd-1; // POINTER TO THE WORD OF THE COMPOSITE, NEEDED TO STORE THE SIZE
                    force_libnum=libnum;
                    libcnt=EXIT_LOOP;
                    break;

                case OK_STARTVALIDATE:
                    if(!ValidateHandler) ValidateHandler=handler;
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;
                    break;

                case ERR_NOTMINE:
                    break;
                case ERR_INVALID:
                case ERR_SYNTAX:
                    Exceptions|=EX_SYNTAXERROR;
                    ExceptionPointer=IPtr;
                    LAMTop=LAMTopSaved;
                    return 0;

                }
            }

           if(libcnt>EXIT_LOOP) {
                    Exceptions|=EX_UNDEFINED;
                    ExceptionPointer=IPtr;

                }

        // HERE WE HAVE A COMPILED OPCODE
        // TODO: VALIDATE THE OPCODE WITH THE CURRENT VALIDATION LIBRARY


    } while( (*((BYTEPTR )NextTokenStart)) && !Exceptions );

 if(addwrapper==-1) { NextTokenStart=(WORDPTR) wrapperend; addwrapper=0; }  // JUST FINISHED THE STRING, NOW ADD THE END OF THE WRAPPER
 if(addwrapper==1) { NextTokenStart=(WORDPTR) string; addwrapper=-1; } // JUST FINISHED THE START OF THE WRAPPER, NOW COMPILE THE ACTUAL STRING



 } while( (*((BYTEPTR )NextTokenStart)) && !Exceptions);

// END OF STRING OBJECT WAS REACHED
    if(ValidateTop!=RSTop) {
        Exceptions|=EX_SYNTAXERROR;
        ExceptionPointer=IPtr;
    }

     LAMTop=LAMTopSaved; // RESTORE LAM ENVIRONMENT BEFORE RETURN

    if( (CompileEnd!=TempObEnd) && !Exceptions) {

   WORDPTR newobject=TempObEnd;

    // STORE BLOCK SIZE
   rplAddTempBlock(TempObEnd);
   TempObEnd=CompileEnd;
   return newobject;
    }

  return 0;

}


void rplDecompAppendChar(BYTE c)
{

    *((BYTEPTR)DecompStringEnd)=c;
    DecompStringEnd=(WORDPTR)(((BYTEPTR)DecompStringEnd)+1);

    if(!(((BINT)DecompStringEnd)&3)) {
        if( ((WORDPTR)((((WORD)DecompStringEnd)+3)&~3))+TEMPOBSLACK>=TempObSize) {
            // ENLARGE TEMPOB AS NEEDED
            growTempOb((((((BYTEPTR)DecompStringEnd)+3-(BYTEPTR)TempOb))>>2)+TEMPOBSLACK);
        }
    }

}

void rplDecompAppendString(BYTEPTR str)
{
    BINT len=strlen((char *)str);

        if( ((WORDPTR)((((WORD)DecompStringEnd)+len+3)&~3))+TEMPOBSLACK>=TempObSize) {
            // ENLARGE TEMPOB AS NEEDED
            growTempOb((((((BYTEPTR)DecompStringEnd)+len+3-(BYTEPTR)TempOb))>>2)+TEMPOBSLACK);
            // IF THERE'S NOT ENOUGH MEMORY, RETURN IMMEDIATELY
            if(Exceptions&EX_OUTOFMEM) return;
        }




    BYTEPTR ptr=(BYTEPTR) DecompStringEnd;

    while(*str!=0) {
        *ptr=*str;
        ++ptr;
        ++str;
    }
    DecompStringEnd=(WORDPTR) ptr;
}

void rplDecompAppendString2(BYTEPTR str,BINT len)
{
        if( ((WORDPTR)((((WORD)DecompStringEnd)+len+3)&~3))+TEMPOBSLACK>=TempObSize) {
            // ENLARGE TEMPOB AS NEEDED
            growTempOb((((((BYTEPTR)DecompStringEnd)+len+3-(BYTEPTR)TempOb))>>2)+TEMPOBSLACK);
            // IF THERE'S NOT ENOUGH MEMORY, RETURN IMMEDIATELY
            if(Exceptions&EX_OUTOFMEM) return;

        }



    BYTEPTR ptr=(BYTEPTR) DecompStringEnd;

    while(len) {
        *ptr=*str;
        ++ptr;
        ++str;
        --len;
    }
    DecompStringEnd=(WORDPTR) ptr;
}



// BASIC DECOMPILE ONE OBJECT
// RETURNS A NEW STRING OBJECT IN TEMPOB

WORDPTR rplDecompile(WORDPTR object)
{
    LIBHANDLER han;

    // START DECOMPILE LOOP
    DecompileObject=object;
    // CREATE A STRING AT THE END OF TEMPOB
    CompileEnd=TempObEnd;
    // SKIPOB TO DETERMINE END OF COMPILATION
    EndOfObject=rplSkipOb(object);

    LAMTopSaved=LAMTop; //SAVE LAM ENVIRONMENTS

    // HERE ALL POINTERS ARE STORED IN GC-UPDATEABLE AREA


    // CREATE EMPTY STRING AT END OF TEMPOB
    rplCompileAppend(MKPROLOG(DOSTRING,0));


    DecompStringEnd=CompileEnd;
    CurOpcode=MKOPCODE(0,OPCODE_DECOMPILE);
    while(DecompileObject<EndOfObject)
    {
    // GET FIRST WORD
    // GET LIBRARY NUMBER
    // CALL LIBRARY HANDLER TO DECOMPILE
    han=rplGetLibHandler(LIBNUM(*DecompileObject));


    if(!han) {
        RetNum=ERR_INVALID;
    } else (*han)();

    switch(RetNum)
    {
    case OK_CONTINUE:
        DecompileObject=rplSkipOb(DecompileObject);
        break;
    case OK_STARTCONSTRUCT:
        ++DecompileObject;
        break;
    default:
        rplDecompAppendString((BYTEPTR)"INVALID_COMMAND");
        ++DecompileObject;
    break;
    }

    // LOOP UNTIL END OF DECOMPILATION ADDRESS IS REACHED
    if(DecompileObject<EndOfObject) rplDecompAppendChar(' ');
    if(Exceptions) break;
    }

    // DONE, HERE WE HAVE THE STRING FINISHED

    // STORE THE SIZE OF THE STRING IN WORDS IN THE PROLOG
    *(CompileEnd-1)=MKPROLOG(DOSTRING+((-(WORD)DecompStringEnd)&3),(((WORD)DecompStringEnd-(WORD)CompileEnd)+3)>>2);

    CompileEnd=rplSkipOb(CompileEnd-1);

    LAMTop=LAMTopSaved;     // RESTORE ENVIRONMENTS
    if( !Exceptions) {


    // STORE BLOCK SIZE
   rplAddTempBlock(TempObEnd);
   WORDPTR newobject=TempObEnd;
   TempObEnd=CompileEnd;
   return newobject;
    }

   return 0;

}

