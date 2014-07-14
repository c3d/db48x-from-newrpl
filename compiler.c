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


// REVERSE-SKIP AN OBJECT, FROM A POINTER TO AFTER THE OBJECT TO SKIP
// NO ARGUMENT CHECKS, DO NOT CALL UNLESS THERE'S A VALID OBJECT LIST
WORDPTR rplReverseSkipOb(WORDPTR list_start,WORDPTR after_object)
{
    WORDPTR next;
    while( (next=rplSkipOb(list_start))<after_object) list_start=next;
    if(next>after_object) return NULL;
    return list_start;
}


// APPLIES THE SYMBOLIC OPERATOR TO THE OUTPUT QUEUE
// ONLY CALLED BY THE COMPILER
// ON ENTRY: CompileEnd = top of the output stream (pointing after the last object)
//           *(ValidateTop-1) = START OF THE SYMBOLIC OBJECT
BINT rplInfixApply(WORD opcode,WORD tokeninfo)
{
    // FORMAT OF SYMBOLIC OBJECT:
    // DOSYMB PROLOG
    // OPCODE
    // ARG1 OBJECT (ARGUMENT LIST)
    // ARG2 OBJECT
    // ...
    // ARGn OBJECT
    // END OF SYMBOLIC OBJECT


    BINT nargs;
    WORDPTR ptr=CompileEnd,symbstart=*(ValidateTop-1)+1;

    //FIND THE START OF THE 'N' ARGUMENTS
    for(nargs=TI_NARGS(tokeninfo);(nargs>0) && ptr ;--nargs)
    {
        ptr=rplReverseSkipOb(symbstart,ptr);
    }

    if(nargs || (!ptr)) return 0; // TOO FEW ARGUMENTS!

    CompileEnd+=2;
    // ADJUST MEMORY AS NEEDED
    if( CompileEnd>=TempObSize) {
        // ENLARGE TEMPOB AS NEEDED
        growTempOb( ((WORD)(CompileEnd-TempOb))+TEMPOBSLACK);
        if(Exceptions) return 0;    // NOT ENOUGH MEMORY
    }

    // MOVE THE ENTIRE LIST TO MAKE ROOM FOR THE HEADER
    memmove(ptr+2,ptr,(CompileEnd-ptr-2)*sizeof(WORD));

    ptr[0]=MKPROLOG(DOSYMBOP,CompileEnd-ptr-1);
    ptr[1]=opcode;

    return 1;
}




// COMPILE A STRING AND RETURN A POINTER TO THE FIRST COMMAND/OBJECT
// IF addwrapper IS NON-ZERO, IT WILL WRAP THE CODE WITH :: ... ; EXITRPL
// (USED BY THE COMMAND LINE FOR IMMEDIATE COMMANDS)

WORDPTR rplCompile(BYTEPTR string,BINT length, BINT addwrapper)
{
    // COMPILATION USES TEMPOB
    CompileEnd=TempObEnd;

    // START COMPILATION LOOP
    BINT force_libnum,splittoken,validate,infixmode;
    BINT probe_libnum,probe_tokeninfo;
    LIBHANDLER handler,ValidateHandler;
    BINT libcnt,libnum;
    WORDPTR InfixOpTop;

    LAMTopSaved=LAMTop;     // SAVE LAM ENVIRONMENT

    ValidateTop=RSTop;
    ValidateHandler=NULL;

    force_libnum=-1;
    splittoken=0;
    infixmode=0;

    if(addwrapper) {
        rplCompileAppend(MKPROLOG(DOCOL,0));
        if(RStkSize<=(ValidateTop-RStk)) growRStk(ValidateTop-RStk+RSTKSLACK);
        if(Exceptions) { LAMTop=LAMTopSaved; return 0; }
        *ValidateTop++=CompileEnd-1; // POINTER TO THE WORD OF THE COMPOSITE, NEEDED TO STORE THE SIZE
    }

    NextTokenStart=(WORDPTR)string;
    CompileStringEnd=(WORDPTR)(string+length);


    // FIND THE START OF NEXT TOKEN
    while( (NextTokenStart<CompileStringEnd) && ((*((BYTEPTR)NextTokenStart)==' ') || (*((BYTEPTR)NextTokenStart)=='\t') || (*((BYTEPTR)NextTokenStart)=='\n') || (*((BYTEPTR)NextTokenStart)=='\r'))) NextTokenStart=(WORDPTR)(((BYTEPTR)NextTokenStart)+1);



    do {
         if(!splittoken) {
        TokenStart=NextTokenStart;
        BlankStart=TokenStart;
        while( (BlankStart<CompileStringEnd) && (*((char *)BlankStart)!=' ') && (*((char *)BlankStart)!='\t') && (*((char *)BlankStart)!='\n') && (*((char *)BlankStart)!='\r')) BlankStart=(WORDPTR)(((char *)BlankStart)+1);
        NextTokenStart=BlankStart;
        while( (NextTokenStart<CompileStringEnd) && ((*((char *)NextTokenStart)==' ') || (*((char *)NextTokenStart)=='\t') || (*((char *)NextTokenStart)=='\n') || (*((char *)NextTokenStart)=='\r'))) NextTokenStart=(WORDPTR)(((char *)NextTokenStart)+1);
        } else splittoken=0;

        TokenLen=(BINT)((BYTEPTR)BlankStart-(BYTEPTR)TokenStart);
        BlankLen=(BINT)((BYTEPTR)NextTokenStart-(BYTEPTR)BlankStart);
        CurrentConstruct=(BINT)((ValidateTop>RSTop)? **(ValidateTop-1):0);      // CARRIES THE WORD OF THE CURRENT CONSTRUCT/COMPOSITE
        ValidateHandler=rplGetLibHandler(LIBNUM(CurrentConstruct));
        LastCompiledObject=CompileEnd;
        if(force_libnum<0) {
            // SCAN THROUGH ALL THE LIBRARIES, FROM HIGH TO LOW, TO SEE WHICH ONE WANTS THE TOKEN
            libcnt=MAXLOWLIBS+NumHiLibs-1;
        }
        else libcnt=0;  // EXECUTE THE LOOP ONLY ONCE

        if(infixmode) {
         probe_libnum=-1;
         probe_tokeninfo=0;
        }

                while(libcnt>=0) {
                if(libcnt>=MAXLOWLIBS) { libnum=HiLibNumbers[libcnt-MAXLOWLIBS]; handler=HiLibRegistry[libcnt-MAXLOWLIBS]; }
                else {
                    if(force_libnum<0) { libnum=libcnt; handler=LowLibRegistry[libcnt]; }
                    else { libnum=force_libnum; handler=rplGetLibHandler(force_libnum); }
                }
                --libcnt;

                if(!handler) continue;

                if(infixmode==1) {
                 CurOpcode=MKOPCODE(libnum,OPCODE_PROBETOKEN);
                }
                else {
                if(force_libnum!=-1) CurOpcode=MKOPCODE(libnum,OPCODE_COMPILECONT);
                    else CurOpcode=MKOPCODE(libnum,OPCODE_COMPILE);
                }

                (*handler)();

                if(RetNum>=OK_TOKENINFO) {
                    // PROCESS THE INFORMATION ABOUT THE TOKEN
                    if(TI_TYPE(RetNum)==TITYPE_NOTALLOWED) {
                        // THIS TOKEN IS NOT ALLOWED IN SYMBOLICS
                        Exceptions|=EX_SYNTAXERROR;
                        ExceptionPointer=IPtr;
                        LAMTop=LAMTopSaved;
                        return 0;
                    }
                    if(TI_LENGTH(RetNum)>TI_LENGTH(probe_tokeninfo)) {
                        probe_libnum=libnum;
                        probe_tokeninfo=RetNum;
                    }
                }
                else
                switch(RetNum)
                {
                case OK_CONTINUE:
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;
                    validate=1;
                    break;
                case OK_CONTINUE_NOVALIDATE:
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;
                    break;

                case OK_STARTCONSTRUCT:
                    if(RStkSize<=(ValidateTop-RStk)) growRStk(ValidateTop-RStk+RSTKSLACK);
                    if(Exceptions) { LAMTop=LAMTopSaved; return 0; }
                    *ValidateTop++=CompileEnd-1; // POINTER TO THE WORD OF THE COMPOSITE, NEEDED TO STORE THE SIZE
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;
                    validate=1;
                    break;
                case OK_CHANGECONSTRUCT:
                    *(ValidateTop-1)=CompileEnd-1; // POINTER TO THE WORD OF THE COMPOSITE, NEEDED TO STORE THE SIZE
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;
                    break;
                case OK_INCARGCOUNT:
                    *(ValidateTop-1)=*(ValidateTop-1)+1; // POINTER TO THE WORD OF THE COMPOSITE, TEMPORARILY STORE THE NUMBER OF ARGUMENTS AS THE SIZE
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
                    if(ISPROLOG((BINT)**ValidateTop)) **ValidateTop=(**ValidateTop ^ OBJSIZE(**ValidateTop)) | ((((WORD)CompileEnd-(WORD)*ValidateTop)>>2)-1);    // STORE THE SIZE OF THE COMPOSITE IN THE WORD
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
                    validate=1;
                    break;
                case OK_SPLITTOKEN:
                    splittoken=1;
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;
                    validate=1;
                    break;
                case OK_STARTCONSTRUCT_SPLITTOKEN:
                    if(RStkSize<=(ValidateTop-RStk)) growRStk(ValidateTop-RStk+RSTKSLACK);
                    if(Exceptions) { LAMTop=LAMTopSaved; return 0; }
                    *ValidateTop++=CompileEnd-1; // POINTER TO THE WORD OF THE COMPOSITE, NEEDED TO STORE THE SIZE
                    splittoken=1;
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;
                    validate=1;
                    break;
                case OK_STARTCONSTRUCT_INFIX:
                    if(RStkSize<=(ValidateTop-RStk)) growRStk(ValidateTop-RStk+RSTKSLACK);
                    if(Exceptions) { LAMTop=LAMTopSaved; return 0; }
                    *ValidateTop++=CompileEnd-1; // POINTER TO THE WORD OF THE COMPOSITE, NEEDED TO STORE THE SIZE
                    infixmode=1;
                    InfixOpTop=ValidateTop;
                    probe_libnum=-1;
                    probe_tokeninfo=0;
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;
                    validate=1;
                    break;
                case OK_ENDCONSTRUCT_INFIX:

                if(infixmode) {
                    // FLUSH OUT ANY OPERATORS IN THE STACK
                    while(InfixOpTop>ValidateTop){
                        InfixOpTop-=2;
                        if(TI_TYPE(InfixOpTop[1])==TITYPE_OPENBRACKET) {
                            // MISSING BRACKET SOMEWHERE!
                            Exceptions|=EX_SYNTAXERROR;
                            ExceptionPointer=IPtr;
                            LAMTop=LAMTopSaved;
                            return 0;
                        }
                        if(!rplInfixApply(InfixOpTop[0],InfixOpTop[1]))
                        {
                            Exceptions|=EX_SYNTAXERROR;
                            ExceptionPointer=IPtr;
                            LAMTop=LAMTopSaved;
                            return 0;
                        }
                    }
                    infixmode=0;
                 }
                    --ValidateTop;
                    if(ValidateTop<RSTop) {
                        Exceptions|=EX_SYNTAXERROR;
                        ExceptionPointer=IPtr;
                        LAMTop=LAMTopSaved;
                        return 0;
                    }
                    if(ISPROLOG((BINT)**ValidateTop)) **ValidateTop=(**ValidateTop ^ OBJSIZE(**ValidateTop)) | ((((WORD)CompileEnd-(WORD)*ValidateTop)>>2)-1);    // STORE THE SIZE OF THE COMPOSITE IN THE WORD
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;
                    break;


                case ERR_NOTMINE:
                    break;
                case ERR_NOTMINE_SPLITTOKEN:
                    splittoken=1;
                    libcnt=EXIT_LOOP;
                    force_libnum=-1;
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
               if(infixmode) {
                // FINISHED PROBING FOR TOKENS
                if(probe_libnum<0) {
                    Exceptions|=EX_UNDEFINED;
                    ExceptionPointer=IPtr;
                }
                else {
                // GOT THE NEXT TOKEN IN THE STREAM
                    // COMPILE THE TOKEN

                    handler=rplGetLibHandler(probe_libnum);
                    CurOpcode=MKOPCODE(probe_libnum,OPCODE_COMPILE);

                    NextTokenStart=BlankStart=((BYTEPTR)TokenStart)+TI_LENGTH(probe_tokeninfo);
                    while( (NextTokenStart<CompileStringEnd) && ((*((char *)NextTokenStart)==' ') || (*((char *)NextTokenStart)=='\t') || (*((char *)NextTokenStart)=='\n') || (*((char *)NextTokenStart)=='\r'))) NextTokenStart=(WORDPTR)(((char *)NextTokenStart)+1);
                    TokenLen=(BINT)((BYTEPTR)BlankStart-(BYTEPTR)TokenStart);
                    BlankLen=(BINT)((BYTEPTR)NextTokenStart-(BYTEPTR)BlankStart);
                    CurrentConstruct=(BINT)((ValidateTop>RSTop)? **(ValidateTop-1):0);      // CARRIES THE WORD OF THE CURRENT CONSTRUCT/COMPOSITE
                    LastCompiledObject=CompileEnd;

                    RetNum=-1;
                    if(handler) (*handler)();

                    if(RetNum!=OK_CONTINUE) {
                        // THE LIBRARY ACCEPTED THE TOKEN DURING PROBE, SO WHAT COULD POSSIBLY GO WRONG?
                        Exceptions|=EX_SYNTAXERROR;
                        ExceptionPointer=IPtr;
                        LAMTop=LAMTopSaved;
                        return 0;
                    }

                    // HERE LastCompiledObject HAS THE NEW OBJECT/COMMAND

                    // IF IT'S AN ATOMIC OBJECT, JUST LEAVE IT THERE IN THE OUTPUT STREAM

                    // IF IT'S AN OPERATOR
                    if(TI_TYPE(probe_tokeninfo)>TITYPE_OPERATORS) {

                        // ALL OPERATORS AND COMMANDS ARE SINGLE-WORD, ONLY OBJECTS TAKE MORE SPACE
                        WORD Opcode=*LastCompiledObject;
                        // REMOVE THE OPERATOR FROM THE OUTPUT STREAM
                        CompileEnd=LastCompiledObject;

                        if(TI_TYPE(probe_tokeninfo)==TITYPE_OPENBRACKET) {
                            // PUSH THE NEW OPERATOR
                            if(RStkSize<=(InfixOpTop-(WORDPTR)RStk)) growRStk(InfixOpTop-(WORDPTR)RStk+RSTKSLACK);
                            if(Exceptions) { LAMTop=LAMTopSaved; return 0; }
                            InfixOpTop[0]=CompileEnd-TempObEnd;        // SAVE POSITION TO START COUNTING ARGUMENTS
                            InfixOpTop[1]=probe_tokeninfo;
                            InfixOpTop+=2;
                        }
                        else {

                        if((TI_TYPE(probe_tokeninfo)==TITYPE_CLOSEBRACKET)||(TI_TYPE(probe_tokeninfo)==TITYPE_COMMA)) {
                         // POP ALL OPERATORS OFF THE STACK UNTIL THE OPENING BRACKET IS FOUND

                            while(InfixOpTop>ValidateTop){
                                if((TI_TYPE(*(InfixOpTop-1))==TITYPE_OPENBRACKET)) break;
                                // POP OPERATORS OFF THE STACK AND APPLY TO OBJECTS
                                InfixOpTop-=2;
                                if(!rplInfixApply(InfixOpTop[0],InfixOpTop[1]))
                                {
                                    Exceptions|=EX_SYNTAXERROR;
                                    ExceptionPointer=IPtr;
                                    LAMTop=LAMTopSaved;
                                    return 0;
                                }
                            }


                            if(InfixOpTop<=ValidateTop) {
                                // OPENING BRACKET NOT FOUND, SYNTAX ERROR
                                    Exceptions|=EX_SYNTAXERROR;
                                    ExceptionPointer=IPtr;
                                    LAMTop=LAMTopSaved;
                                    return 0;
                            }

                            if(TI_TYPE(probe_tokeninfo)==TITYPE_CLOSEBRACKET) {
                            // COUNT THE NUMBER OF ARGUMENTS WE HAVE
                            // REMOVE THE OPENING BRACKET
                            InfixOpTop-=2;

                            BINT nargs=0;
                            WORDPTR list=TempObEnd+InfixOpTop[0];
                            WORDPTR ptr=CompileEnd;

                            while((ptr=rplReverseSkipOb(list,ptr))!=NULL) ++nargs;



                            // CHECK IF THE TOP OF STACK IS A FUNCTION
                            if(InfixOpTop>ValidateTop) {
                                if(TI_TYPE(*(InfixOpTop-1))==TITYPE_FUNCTION) {
                                    if(nargs!=TI_NARGS(*(InfixOpTop-1))) {
                                        Exceptions|=EX_BADARGCOUNT;
                                        ExceptionPointer=IPtr;
                                        LAMTop=LAMTopSaved;
                                        return 0;
                                    }
                                    // POP FUNCTION OFF THE STACK AND APPLY
                                    InfixOpTop-=2;

                                    if(!rplInfixApply(InfixOpTop[0],InfixOpTop[1]))
                                    {
                                        Exceptions|=EX_SYNTAXERROR;
                                        ExceptionPointer=IPtr;
                                        LAMTop=LAMTopSaved;
                                        return 0;
                                    }

                                }
                            }
                            } else {
                                // THE PARAMETER IS A COMMA
                                // DO NOTHING FOR NOW
                            }



                        }
                        else {

                        // IN INFIX MODE, USE RStk AS THE OPERATOR STACK, STARTING AT ValidateTop
                        while(InfixOpTop>ValidateTop){
                            if(TI_PRECEDENCE(*(InfixOpTop-1))<TI_PRECEDENCE(probe_tokeninfo)) {
                            // POP OPERATORS OFF THE STACK AND APPLY TO OBJECTS
                            InfixOpTop-=2;

                            if(!rplInfixApply(InfixOpTop[0],InfixOpTop[1]))
                            {
                                Exceptions|=EX_SYNTAXERROR;
                                ExceptionPointer=IPtr;
                                LAMTop=LAMTopSaved;
                                return 0;
                            }

                            } else {
                                if( (TI_TYPE(probe_tokeninfo)==TITYPE_BINARYOP_LEFT)&&(TI_PRECEDENCE(*(InfixOpTop-1))<=TI_PRECEDENCE(probe_tokeninfo)))
                                {
                                    InfixOpTop-=2;

                                    if(!rplInfixApply(InfixOpTop[0],InfixOpTop[1]))
                                    {
                                        Exceptions|=EX_SYNTAXERROR;
                                        ExceptionPointer=IPtr;
                                        LAMTop=LAMTopSaved;
                                        return 0;
                                    }

                                }
                                else break;
                            }
                        }
                        // PUSH THE NEW OPERATOR
                        if(RStkSize<=(InfixOpTop-(WORDPTR)RStk)) growRStk(InfixOpTop-(WORDPTR)RStk+RSTKSLACK);
                        if(Exceptions) { LAMTop=LAMTopSaved; return 0; }
                        InfixOpTop[0]=Opcode;
                        InfixOpTop[1]=probe_tokeninfo;
                        InfixOpTop+=2;
                        }
                        }

                    }


                }


               }
               else {
                    Exceptions|=EX_UNDEFINED;
                    ExceptionPointer=IPtr;
               }

                }

        // HERE WE HAVE A COMPILED OPCODE

    // SUBMIT THE LAST COMPILED OBJECT FOR VALIDATION WITH THE CURRENT CONSTRUCT
    if(validate) {
        if(ValidateHandler) {
            // CALL THE LIBRARY TO SEE IF IT'S OK TO HAVE THIS OBJECT
            CurOpcode=MKOPCODE(LIBNUM(CurrentConstruct),OPCODE_VALIDATE);
            (*ValidateHandler)();

            switch(RetNum)
            {
            case OK_INCARGCOUNT:
                **(ValidateTop-1)=**(ValidateTop-1)+1; // POINTER TO THE WORD OF THE COMPOSITE, TEMPORARILY STORE THE NUMBER OF ARGUMENTS AS THE SIZE
                break;

                case ERR_INVALID:
                Exceptions|=EX_SYNTAXERROR;
                ExceptionPointer=IPtr;
                LAMTop=LAMTopSaved;
                return 0;
            }

        }
        validate=0;
    }

    } while( (NextTokenStart<CompileStringEnd) && !Exceptions );

 if(addwrapper) {
     // JUST FINISHED THE STRING, NOW ADD THE END OF THE WRAPPER
     rplCompileAppend(CMD_SEMI);
     --ValidateTop;
     if(ValidateTop<RSTop) {
         Exceptions|=EX_SYNTAXERROR;
         ExceptionPointer=IPtr;
         LAMTop=LAMTopSaved;
         return 0;
     }
     if(ISPROLOG((BINT)**ValidateTop)) **ValidateTop|= (((WORD)CompileEnd-(WORD)*ValidateTop)>>2)-1;    // STORE THE SIZE OF THE COMPOSITE IN THE WORD
     rplCompileAppend(CMD_EXITRPL);
 }


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

enum {
    INFIX_OFF=0,
    INFIX_STARTSYMBOLIC,
    INFIX_STARTEXPRESSION,
    INFIX_FUNCNAME,
    INFIX_FUNCARGUMENT,
    INFIX_PREFIXOP,
    INFIX_PREFIXARG,
    INFIX_POSTFIXOP,
    INFIX_POSTFIXARG,
    INFIX_BINARYLEFT,
    INFIX_BINARYOP,
    INFIX_BINARYRIGHT,
    INFIX_ATOMIC
};












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
    BINT infixmode=0;
    WORDPTR InfixOpTop=RSTop;

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
    while(DecompileObject<EndOfObject)
    {
    // GET FIRST WORD
    // GET LIBRARY NUMBER
    // CALL LIBRARY HANDLER TO DECOMPILE
    han=rplGetLibHandler(LIBNUM(*DecompileObject));

    CurOpcode=MKOPCODE(0,OPCODE_DECOMPILE);

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
    case OK_STARTCONSTRUCT_INFIX:
        // PUSH THE SYMBOLIC ON A STACK AND SAVE THE COMPILER STATE
        if(RStkSize<=(InfixOpTop-(WORDPTR)RStk)) growRStk(InfixOpTop-(WORDPTR)RStk+RSTKSLACK);
        if(Exceptions) { LAMTop=LAMTopSaved; return 0; }
        InfixOpTop[1]=infixmode;
        InfixOpTop[0]=DecompileObject-EndOfObject;
        InfixOpTop+=2;
        ++DecompileObject;
        if(infixmode) {
            // SAVE PREVIOUS MODE AND START A SUBEXPRESSION
            infixmode=INFIX_STARTEXPRESSION;
        } else infixmode=INFIX_STARTSYMBOLIC;
        break;
    default:
        rplDecompAppendString((BYTEPTR)"INVALID_COMMAND");
        ++DecompileObject;
    break;
    }

end_of_expression:

    // LOOP UNTIL END OF DECOMPILATION ADDRESS IS REACHED
    if(infixmode) {
        // IN INFIX MODE, OBJECTS ARE LISTS, BUT FIRST ELEMENT
        // MIGHT BE AN OPERATOR, AND ARGUMENTS MIGHT NEED TO BE PROCESSED
        // IN DIFFERENT ORDER

        switch(infixmode)
        {
        case INFIX_STARTSYMBOLIC:
            rplDecompAppendChar('`');
            if(Exceptions) break;
        case INFIX_STARTEXPRESSION:
        {

            // EVALUATE THE TYPE OF SYMBOLIC OPERATOR
            // GET INFORMATION ABOUT THE TOKEN
                LIBHANDLER handler=rplGetLibHandler(LIBNUM(*DecompileObject));
                RetNum=0;
                if(handler) {

                 CurOpcode=MKOPCODE(LIBNUM(*DecompileObject),OPCODE_GETINFO);
                (*handler)();
                }

                if(RetNum<OK_TOKENINFO) RetNum=MKTOKENINFO(0,TITYPE_FUNCTION,0,20); //    TREAT LIKE A NORMAL FUNCTION, THAT WILL BE CALLED [INVALID] LATER

                if(TI_TYPE(RetNum)>=TITYPE_OPERATORS) {
                // PUSH THE OPERATOR ON THE STACK
                if(RStkSize<=(InfixOpTop-(WORDPTR)RStk)) growRStk(InfixOpTop-(WORDPTR)RStk+RSTKSLACK);
                if(Exceptions) { LAMTop=LAMTopSaved; return 0; }
                InfixOpTop[0]=*DecompileObject;
                InfixOpTop[1]=RetNum;
                InfixOpTop+=2;

                // CHECK PRECEDENCE TO SEE IF WE NEED PARENTHESIS
                if( (InfixOpTop-6)>=RSTop) {
                    // THERE'S AN OPERATOR IN THE STACK
                    if(ISPROLOG(*(InfixOpTop-6))) {
                        // THIS IS AN EXPRESSION START WITHOUT ANY OPERATORS
                        // NO NEED FOR PARENTHESIS
                    }
                    else {
                        if(TI_PRECEDENCE(*(InfixOpTop-5))<TI_PRECEDENCE(RetNum)) {
                            if(TI_TYPE(*(InfixOpTop-5))!=TITYPE_FUNCTION)   // DO NOT ADD PARENTHESIS TO FUNCTION ARGUMENTS!
                                rplDecompAppendChar('(');
                        }
                    }
                }


                switch(TI_TYPE(RetNum))
                {
                case TITYPE_BINARYOP_LEFT:
                case TITYPE_BINARYOP_RIGHT:
                    ++DecompileObject;
                    infixmode=INFIX_BINARYLEFT;
                break;
                case TITYPE_POSTFIXOP:
                    ++DecompileObject;
                    infixmode=INFIX_POSTFIXARG;
                    break;
                case TITYPE_PREFIXOP:
                    // DECOMPILE THE OPERATOR NOW!
                    CurOpcode=MKOPCODE(LIBNUM(*DecompileObject),OPCODE_DECOMPILE);
                    (*handler)();
                    // IGNORE THE RESULT OF DECOMPILATION
                    if(RetNum!=OK_CONTINUE) {
                        Exceptions|=EX_BADOPCODE;
                        ExceptionPointer=IPtr;
                        LAMTop=LAMTopSaved;     // RESTORE ENVIRONMENTS
                        return 0;
                    }
                    ++DecompileObject;
                    infixmode=INFIX_PREFIXARG;
                    break;

                case TITYPE_FUNCTION:
                default:
                    // DECOMPILE THE OPERATOR NOW, THEN ADD PARENTHESIS FOR THE LIST
                    CurOpcode=MKOPCODE(LIBNUM(*DecompileObject),OPCODE_DECOMPILE);
                    RetNum=-1;
                    if(handler) (*handler)();
                    // IGNORE THE RESULT OF DECOMPILATION
                    if(RetNum!=OK_CONTINUE) {
                        rplDecompAppendString((BYTEPTR)"##INVALID##");
                    }
                    rplDecompAppendChar('(');
                    ++DecompileObject;
                    infixmode=INFIX_FUNCARGUMENT;
                    break;
                }
                } else infixmode=INFIX_ATOMIC;

            break;
        }
        case INFIX_BINARYLEFT:
        {
            LIBHANDLER handler;
            // ADD THE OPERATOR AFTER THE LEFT OPERAND
            BINT libnum=LIBNUM(*(InfixOpTop-2));
            SavedDecompObject=DecompileObject;
            DecompileObject=InfixOpTop-2;
            CurOpcode=MKOPCODE(libnum,OPCODE_DECOMPILE);
            handler=rplGetLibHandler(libnum);
            RetNum=-1;

            if(handler) (*handler)();

            DecompileObject=SavedDecompObject;
            // IGNORE THE RESULT OF DECOMPILATION
            if(RetNum!=OK_CONTINUE) {
                rplDecompAppendString((BYTEPTR)"##INVALID##");
            }

            // NOW CHECK IF THE RIGHT ARGUMENT IS INDEED THE LAST ONE
            WORDPTR afternext=rplSkipOb(DecompileObject);
            WORDPTR EndofExpression = rplSkipOb(*(InfixOpTop-4)+EndOfObject);


            if(afternext==EndofExpression) {
                // THE NEXT ELEMENT IS THE LAST (IT SHOULD ALWAYS BE IF THE BINARY OPERATOR HAS ONLY 2 ARGUMENTS
                // BUT WE CAN PACK MORE TERMS ON THE SAME '+' OR '*' THIS WAY
                infixmode=INFIX_BINARYRIGHT;
            }
            // IF IT'S NOT, THEN KEEP IT AS THE LEFT OPERATOR FOR THE NEXT ARGUMENT
            break;

        }
        case INFIX_BINARYRIGHT:
        case INFIX_PREFIXARG:
        {
            // WE KNOW THIS IS THE LAST ARGUMENT
                // POP EXPRESSION FROM THE STACK
            // CHECK PRECEDENCE TO SEE IF WE NEED PARENTHESIS
            if( (InfixOpTop-6)>=RSTop) {
                // THERE'S AN OPERATOR IN THE STACK
                if(ISPROLOG(*(InfixOpTop-6))) {
                    // THIS IS AN EXPRESSION START WITHOUT ANY OPERATORS
                    // NO NEED FOR PARENTHESIS
                }
                else {
                    if(TI_PRECEDENCE(*(InfixOpTop-5))<TI_PRECEDENCE(*(InfixOpTop-1))) {
                        if(TI_TYPE(*(InfixOpTop-5))!=TITYPE_FUNCTION) // DON'T ADD PARENTHESIS TO FUNCTION ARGUMENTS
                        rplDecompAppendChar(')');
                    }
                }
            }
                InfixOpTop-=4;
                // RESTORE PREVIOUS EXPRESSION STATE
                infixmode=InfixOpTop[1];
                DecompileObject=rplSkipOb(*InfixOpTop+EndOfObject);
                goto end_of_expression;
        }
        break;
        case INFIX_POSTFIXARG:
        {
            LIBHANDLER handler;
            // ADD THE OPERATOR AFTER THE OPERAND
            BINT libnum=LIBNUM(*(InfixOpTop-2));
            SavedDecompObject=DecompileObject;
            DecompileObject=InfixOpTop-2;
            CurOpcode=MKOPCODE(libnum,OPCODE_DECOMPILE);
            handler=rplGetLibHandler(libnum);
            RetNum=-1;

            if(handler) (*handler)();

            DecompileObject=SavedDecompObject;
            // IGNORE THE RESULT OF DECOMPILATION
            if(RetNum!=OK_CONTINUE) {
                rplDecompAppendString((BYTEPTR)"##INVALID##");
            }

            // CHECK PRECEDENCE TO SEE IF WE NEED PARENTHESIS
            if( (InfixOpTop-6)>=RSTop) {
                // THERE'S AN OPERATOR IN THE STACK
                if(ISPROLOG(*(InfixOpTop-6))) {
                    // THIS IS AN EXPRESSION START WITHOUT ANY OPERATORS
                    // NO NEED FOR PARENTHESIS
                }
                else {
                    if(TI_PRECEDENCE(*(InfixOpTop-5))<TI_PRECEDENCE(*(InfixOpTop-1))) {
                        if(TI_TYPE(*(InfixOpTop-5))!=TITYPE_FUNCTION) // DON'T ADD PARENTHESIS TO FUNCTION ARGUMENTS
                        rplDecompAppendChar(')');
                    }
                }
            }
            // POP EXPRESSION FROM THE STACK
            InfixOpTop-=4;
            // RESTORE PREVIOUS EXPRESSION STATE
            infixmode=InfixOpTop[1];
            DecompileObject=rplSkipOb(*InfixOpTop+EndOfObject);
            goto end_of_expression;

        }
        break;

        case INFIX_FUNCARGUMENT:
        {
            // CHECK IF THIS IS THE LAST ARGUMENT
            WORDPTR EndofExpression = rplSkipOb(*(InfixOpTop-4)+EndOfObject);

            if(DecompileObject==EndofExpression) {
                rplDecompAppendChar(')');
                // END OF THIS EXPRESSION
                // POP EXPRESSION FROM THE STACK
                InfixOpTop-=4;
                // RESTORE PREVIOUS EXPRESSION STATE
                infixmode=InfixOpTop[1];
                DecompileObject=rplSkipOb(*InfixOpTop+EndOfObject);
                goto end_of_expression;
            }
            else {
                // IF NOT, KEEP PROCESSING ARGUMENTS
                rplDecompAppendChar(',');   // TODO: CHANGE THIS TO ; WHEN USING ',' AS DECIMAL POINT!
            }




        break;
        }
        case INFIX_ATOMIC:
        {
            // CHECK IF THIS IS THE LAST ARGUMENT
            WORDPTR EndofExpression = rplSkipOb(*(InfixOpTop-2)+EndOfObject);

            if(DecompileObject==EndofExpression) {
                // END OF THIS EXPRESSION
                // POP EXPRESSION FROM THE STACK
                InfixOpTop-=2;
                // RESTORE PREVIOUS EXPRESSION STATE
                infixmode=InfixOpTop[1];
                DecompileObject=rplSkipOb(*InfixOpTop+EndOfObject);
                if(!infixmode) rplDecompAppendChar('`');
                goto end_of_expression;
            }
            else {
                // IF NOT, KEEP PROCESSING A LIST OF EXPRESSIONS (???)
                rplDecompAppendChar(',');   // TODO: CHANGE THIS TO ; WHEN USING ',' AS DECIMAL POINT!
            }
            break;
        }

        default:
        break;



        }

    }
    else {
    if(DecompileObject<EndOfObject) rplDecompAppendChar(' ');
    if(Exceptions) break;
    }
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

