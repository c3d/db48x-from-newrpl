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
#define LIBRARY_NUMBER  102


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(CRLIB,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(EXLIB,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2))




// ADD MORE OPCODES HERE

#define ERROR_LIST \
    ERR(INVALIDLIBID,0), \
    ERR(INVALIDVISIBLE,1)




// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER, LIBRARY_NUMBER+1


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

// OTHER ROMOBJECTS
ROMOBJECT library_dirname[]={
    MKPROLOG(DOIDENT,1),
    TEXT2WORD('L','I','B',0)
};

ROMOBJECT libid_ident[]={
    MKPROLOG(DOIDENT,2),
    TEXT2WORD('$','L','I','B'),
    TEXT2WORD('I','D',0,0)
};

ROMOBJECT title_ident[]={
    MKPROLOG(DOIDENT,2),
    TEXT2WORD('$','T','I','T'),
    TEXT2WORD('L','E',0,0)
};

ROMOBJECT visible_ident[]={
    MKPROLOG(DOIDENT,2),
    TEXT2WORD('$','V','I','S'),
    TEXT2WORD('I','B','L','E')
};

ROMOBJECT ignore_ident[]={
    MKPROLOG(DOIDENT,2),
    TEXT2WORD('$','I','G','N'),
    TEXT2WORD('O','R','E',0)
};

ROMOBJECT handler_ident[]={
    MKPROLOG(DOIDENT,2),
    TEXT2WORD('$','H','A','N'),
    TEXT2WORD('D','L','E','R')
};

ROMOBJECT defhandler_seco[]={
    MKPROLOG(SECO,1),
    CMD_QSEMI
};

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib102_menu);


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib102_menu,
    (WORDPTR)library_dirname,
    (WORDPTR)libid_ident,
    (WORDPTR)title_ident,
    (WORDPTR)visible_ident,
    (WORDPTR)ignore_ident,
    (WORDPTR)handler_ident,
    (WORDPTR)defhandler_seco,
    0
};

/* LIBRARY OBJECT FORMAT:
 *
 * [0]=PROLOG
 * [1]=DOIDENT
 * [2]=LIBRARYID=1 WORD
 * [3]= N NUMBER OF COMMANDS IN THIS LIBRARY (AS A SINT)
 * [4..N+4]=OFFSET TABLE (ONE ENTRY FOR EACH COMMAND) (AS SINT - OFFSET MEASURED FROM START OF LIBRARY OBJECT)
 * [N+4 ...]=COMMAND DATA:
 *                         IDENT = NAME OF COMMAND
 *                         SINT = { TOKENINFO SIMPLIFIED: NARGS+1000*(ALLOWEDINSYMBOLICS?)
 *                         OBJECT = WHATEVER THIS NAMED OBJECT IS
 *                         REPEATS IDENT/SINT/OBJECT GROUPS UNTIL END OF LIBRARY
 *
*/

// FIND THE OBJECT WITHIN THE LIBRARY, CORRESPONDING TO A libptr OBJECT BEING EXECUTED
// NO ARGUMENT CHECKS FOR SPEED

WORDPTR rplGetLibPtr(WORDPTR libptr)
{
    WORD libid=libptr[1];
    WORD libcmd=libptr[2];

    WORDPTR libdir=rplGetSettings((WORDPTR)library_dirname);

    if(!libdir) return 0;

    WORDPTR *direntry=rplFindFirstByHandle(libdir);

    if(!direntry) return 0;

    do {

        if(ISIDENT(*direntry[0]) && (OBJSIZE(*direntry[0])==1)) {
            // COMPARE LIBRARY ID
            if(direntry[0][1] == libid) {
                // FOUND THE LIBRARY - GET THE COMMAND
                if(libcmd>direntry[1][3]) return 0;

                return direntry[1]+OPCODE(direntry[1][libcmd+4]);
            }

        }

    } while((direntry=rplFindNext(direntry)));


   return 0;
}

// GET COMMAND THE NAME OF A LIBPTR

WORDPTR rplGetLibPtrName(WORDPTR libptr)
{
    WORD libid=libptr[1];
    WORD libcmd=libptr[2];

    WORDPTR libdir=rplGetSettings((WORDPTR)library_dirname);

    if(!libdir) return 0;

    WORDPTR *direntry=rplFindFirstByHandle(libdir);

    if(!direntry) return 0;

    do {

        if(ISIDENT(*direntry[0]) && (OBJSIZE(*direntry[0])==1)) {
            // COMPARE LIBRARY ID
            if(direntry[0][1] == libid) {
                // FOUND THE LIBRARY - GET THE NAME
                if(libcmd>direntry[1][3]) return 0;
                WORDPTR nobj;
                if(libcmd==0) nobj=direntry[1]+OPCODE(direntry[1][3])+4;
                else nobj=rplSkipOb(direntry[1]+OPCODE(direntry[1][libcmd+3]));

                return nobj;
            }

        }

    } while((direntry=rplFindNext(direntry)));


   return 0;
}




// FIND A COMMAND BY NAME WITHIN A LIBRARY, RETURN ITS INDEX IN THE HIGH WORD, LIBRARY NAME IN ITS LOW WORD
// OR -1 IF NOT FOUND

BINT64 rplFindLibPtrIndex(BYTEPTR start,BYTEPTR end)
{

    WORDPTR libdir=rplGetSettings((WORDPTR)library_dirname);

    if(!libdir) return -1;

    WORDPTR *direntry=rplFindFirstByHandle(libdir);

    if(!direntry) return -1;

    WORDPTR cmd,libend;
    BINT idx,ncommands;

    do {

        if(ISLIBRARY(*direntry[1])) {
            ncommands=OPCODE(direntry[1][3]);
            idx=0;
            libend=rplSkipOb(direntry[1]);

            cmd=direntry[1]+OPCODE(direntry[1][3])+4;

            // SCAN THROUGH ALL THE COMMANDS
            while (cmd<libend) {
            if(rplCompareIDENTByName(cmd,start,end)) {
                // FOUND A MATCH
                return (((BINT64)idx)<<32)| direntry[1][2];
            }

            cmd=rplSkipOb(cmd); // SKIP NAME
            if(cmd>=libend) break;
            cmd=rplSkipOb(cmd); // SKIP TOKENINFO
            if(cmd>=libend) break;
            cmd=rplSkipOb(cmd); // SKIP OBJECT

            ++idx;
            }
        }

    } while((direntry=rplFindNext(direntry)));


   return -1;
}

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        if(ISLIBPTR(CurOpcode)) {
            WORDPTR libobject=rplGetLibPtr(IPtr);
            if(!libobject) {
                rplError(ERR_MISSINGLIBRARY);
                return;
            }
            rplPushData(libobject);
            rplCallOvrOperator(CMD_OVR_XEQ);
            return;
             }

        // JUST PUSH THE LIBRARY OBJECT ON THE STACK
        rplPushData(IPtr);
        return;
    }





    switch(OPCODE(CurOpcode))
    {
    case CRLIB:
    {
        // CREATE A LIBRARY FROM THE CURRENT DIRECTORY
        /* LIBRARY OBJECT FORMAT:
         *
         * [0]=PROLOG
         * [1]=DOIDENT
         * [2]=LIBRARYID=1 WORD
         * [3]= N NUMBER OF COMMANDS IN THIS LIBRARY (AS A SINT)
         * [4..N+4-1]=OFFSET TABLE (ONE ENTRY FOR EACH COMMAND) (AS SINT - OFFSET MEASURED FROM START OF LIBRARY OBJECT)
         * [N+4 ...]=COMMAND DATA:
         *                         IDENT = NAME OF COMMAND
         *                         SINT = { TOKENINFO SIMPLIFIED: NARGS+1000*(ALLOWEDINSYMBOLICS?)
         *                         OBJECT = WHATEVER THIS NAMED OBJECT IS
         *                         REPEATS IDENT/SINT/OBJECT GROUPS UNTIL END OF LIBRARY
         *
        */

        // VARIABLES THAT INFLUENCE CRLIB:
        // $LIBID  --> IT'S AN IDENT WITH UP TO 4 LETTERS OR NUMBERS. ONLY a-z, A-Z AND 0-9 ARE VALID CHARACTERS
        // $TITLE  --> STRING WITH A USER-FRIENDLY LIBRARY TITLE OR PROGRAM THAT SHOWS A SPLASHSCREEN
        // $VISIBLE --> LIST OF LISTS WITH IDENTS THAT WILL BE MADE USER-ACCESSIBLE. THE ORDER SHOULD BE PRESERVED ACROSS
        //              NEW LIBRARY VERSIONS. ALL OBJECTS IN THE DIRECTORY THAT ARE REFERENCED FROM WITHIN A
        //              PROGRAM IN THIS LIST WILL BE INCLUDED AND BE MADE INVISIBLE TO THE USER (UNNAMED).
        //              UNREFERENCED OBJECTS WILL NOT BE INCLUDED.
        // $IGNORE -->  LIST OF IDENTS THAT WILL NOT BE INCLUDED EVEN IF REFERENCED FROM A COMMAND.
        // $HANDLER --> IDENT OF A PROGRAM THAT WILL BECOME THE DEFAULT LIBRARY HANDLER - WILL BE CALLED TO
        //              INSTALL/REMOVE THE LIBRARY, SAVE/LOAD/RESET THE LIBRARY INTERNAL DATA AND RUN THE LIB AS AN APP
        //              IF NO HANDLER IS SUPPLIED, A DUMMY WILL BE ADDED TO THE LIBRARY

        // $VISIBLE HAS THE FOLLOWING FORMAT:
        // { { IDENT NARGS ALLOWINSYMB } { IDENT NARGS ALLOWINSYMB } ... }


        // LIBRARY CREATION - PASS 1

        // CHECK FOR VALID $LIBID

        WORD libid=0;
        BINT nvisible,nhidden,nsizeextra,datasize;
        WORDPTR *object;

        nvisible=nhidden=0;
        nsizeextra=datasize=0;


        object=rplFindGlobal((WORDPTR)libid_ident,0);

        if(object) {
            if(ISIDENT(*object[1]) && (OBJSIZE(*object[1])==1)) {
                libid=object[1][1];

                BYTEPTR ptr=(BYTEPTR)(object[1]+1);
                while(ptr<(BYTEPTR)(object[1]+2)) {
                    if( ((*ptr>='A')&&(*ptr<='Z')) || ((*ptr>='a')&&(*ptr<='z')) || ((*ptr>='0')&&(*ptr<='9'))) ++ptr;
                    else break;
                }
                while(ptr<(BYTEPTR)(object[1]+2)) {
                    if(*ptr) { libid=0; break; }
                    ++ptr;
                }

            }

        }

        if(!libid) {
            rplError(ERR_INVALIDLIBID);
            return;
        }

        // CHECK FOR VALID $HANDLER
        // HANDLER USES ENTRY #0, IT TAKES A NULL-NAME (1 WORD), INFO=1 WORD AND SIZE OF HANDLER

        datasize+=2+rplObjSize((WORDPTR)zero_bint);



        // CHECK FOR VALID $TITLE

        // TITLE USES ENTRY #1, SO IT TAKES A NULL-NAME (A SINT=1 WORD), INFO=1 WORD, AND THE SIZE OF THE STRING
        datasize+=2+rplObjSize((WORDPTR)empty_string);


        // CHECK FOR VALID $VISIBLE

        object=rplFindGlobal((WORDPTR)visible_ident,0);

        WORDPTR *stksave=DSTop;

        if(object) {
            if(ISLIST(*object[1])) {
                nvisible=rplListLength(object[1]);
                int k;
                WORDPTR item=object[1]+1;
                for(k=0;k<nvisible;++k) {
                    if( ISLIST(*item) && (rplListLength(item)==3) ) {
                        WORDPTR var=item+1;
                        if(ISIDENT(*var))  {
                            // CHECK FOR REFERENCES
                            WORDPTR *prog=rplFindGlobal(var,0);

                            if(prog) {
                                rplPushData(var);
                                rplPushData(prog[1]);
                                if(Exceptions) { DSTop=stksave; return; }
                                datasize+=rplObjSize(rplPeekData(2))+rplObjSize(rplPeekData(1))+1;
                                var=rplSkipOb(var);
                                if( ISNUMBER(*var) && ISNUMBER(*rplSkipOb(var))) {
                                    item=rplSkipOb(item); continue;
                                }
                            }
                        }
                    }
                    nvisible=0;
                    break;
                }
            }
        }
        if(!nvisible) {
            DSTop=stksave;
            rplError(ERR_INVALIDVISIBLE);
            return;
        }

        // SCAN ALL VISIBLE VARIABLES FOR REFERENCES TO HIDDEN ONES

        WORDPTR *stkptr=stksave;

        WORDPTR *ignore=rplFindGlobal((WORDPTR)ignore_ident,0);


        while(stkptr<DSTop) {
            WORDPTR prog=stkptr[1];
            WORDPTR endprog=rplSkipOb(prog);

            while(prog!=endprog) {
                if(ISUNQUOTEDIDENT(*prog)) {
                    WORDPTR *direntry=rplFindGlobal(prog,0);
                    if(direntry) {
                        // MAKE SURE IS NOT IN THE IGNORE LIST

                        if(ignore) {
                            WORDPTR ignitem=ignore[1];
                            WORDPTR endign=rplSkipOb(ignitem);
                            if(ISLIST(*ignitem)) ++ignitem;
                            while(ignitem<endign) {
                                if(rplCompareIDENT(ignitem,prog)) {
                                    // IS IN THE IGNORE LIST
                                    break;
                                }
                                ignitem=rplSkipOb(ignitem);
                            }
                            if(ignitem>=endign) {
                                // VARIABLE IS IN THE IGNORE LIST - NOTHING TO DO HERE
                                prog=rplSkipOb(prog);
                                continue;
                            }


                        }

                        // VARIABLE EXISTS - CHECK IF WE ALREADY HAVE IT IN THE LIST
                        WORDPTR *stkscan=stksave;

                        while(stkscan<DSTop) {
                            if(rplCompareIDENT(prog,*stkscan)) break;
                            stkscan+=2;
                        }
                        if(stkscan>=DSTop) {
                            // VARIABLE WASN'T SEEN BEFORE, ADD TO THE LIST FOR SCANNING
                            ScratchPointer1=prog;
                            ScratchPointer2=endprog;
                            rplPushData(direntry[0]);
                            rplPushData(direntry[1]);
                            prog=ScratchPointer1;
                            endprog=ScratchPointer2;
                            if(Exceptions) { DSTop=stksave; return; }

                            datasize+=rplObjSize(rplPeekData(2))+rplObjSize(rplPeekData(1))+1;

                        }
                        // EITHER WAY, THE REFERENCE NEEDS TO BE REPLACED
                        // THE IDENT WILL BE REPLACED WITH A LIBPTR OBJECT (3 WORDS)
                        // COMPUTE THE EXTRA SPACE NEEDED FOR THE REPLACEMENT
                        nsizeextra+=3-rplObjSize(prog);


                    }
                }

                if(ISPROGRAM(*prog) || ISLIST(*prog))
                {
                    // SCAN INTO PROGRAMS AND LISTS
                    ++prog;
                    continue;
                }

                prog=rplSkipOb(prog);


            }

            stkptr+=2;
        }

        nhidden=(DSTop-stksave);
        nhidden>>=1;
        nhidden-=nvisible;

        // PASS 2 - CREATION OF THE OBJECT

        BINT totalsize=3+(2+nvisible+nhidden)+datasize+nsizeextra;


        WORDPTR newobj=rplAllocTempOb(totalsize);

        if(!newobj) { DSTop=stksave; return; }

        // WRITE THE LIBRARY OBJECT:
        newobj[0]=MKPROLOG(DOLIBRARY,totalsize);
        newobj[1]=MKPROLOG(DOIDENT,1);
        newobj[2]=libid;
        newobj[3]=MAKESINT(2+nvisible+nhidden);

        int k,totaln=2+nvisible+nhidden;
        BINT offset=4+totaln;

        for(k=0;k<totaln;++k) {
            // ADD COMMAND NAME
            if(k>=2) rplCopyObject(newobj+offset,stksave[2*(k-2)]);
            else newobj[offset]=MAKESINT(0);    // NULL NAME = BINT ZERO

            offset+=rplObjSize(newobj+offset);

            // ADD INFO
            if((k>=2)&&(k<nvisible+2)) {
                WORDPTR info=rplSkipOb(stksave[2*(k-2)]);

                if(object && (info>object[1]) && (info<rplSkipOb(object[1]))) {

                BINT nargs=rplReadNumberAsBINT(info);
                BINT allow=!rplIsFalse(rplSkipOb(info));

                newobj[offset]=MAKESINT( (nargs<<8) | ((allow)? 1:0));
                }
                else newobj[offset]=MAKESINT(0);
            }
            else newobj[offset]=MAKESINT(0);

            offset++;

            // ADD THE POINTER TO THE OBJECT IN THE HASH TABLE
            newobj[4+k]=offset;

            // AND FINALLY ADD THE OBJECT ITSELF
                WORDPTR prog;

                switch(k)
                {
                case 0:
                {
                    // GET THE $HANDLER OBJECT
                    WORDPTR *han=rplFindGlobal((WORDPTR)handler_ident,0);

                    if(han) prog=han[1];
                    else prog=(WORDPTR)defhandler_seco;
                    break;
                }
                case 1:
                {
                    // GET THE $TITLE OBJECT
                    WORDPTR *tit=rplFindGlobal((WORDPTR)title_ident,0);

                    if(tit) prog=tit[1];
                    else prog=(WORDPTR)empty_string;

                    break;

                }
                default:
                    prog=stksave[2*(k-2)+1];
                }


                WORDPTR endprog=rplSkipOb(prog);
                WORDPTR *stktop=DSTop;


                while(prog!=endprog) {
                    if(ISUNQUOTEDIDENT(*prog)) {
                            // CHECK IF WE ALREADY HAVE IT IN THE LIST
                            WORDPTR *stkscan=stksave;


                            while(stkscan<DSTop) {
                                if(rplCompareIDENT(prog,*stkscan)) break;
                                stkscan+=2;
                            }
                            if(stkscan<DSTop) {
                                // VARIABLE WAS FOUND, REPLACE WITH LIBPTR
                                newobj[offset++]=MKPROLOG(DOLIBPTR,2);
                                newobj[offset++]=newobj[2];
                                newobj[offset++]=(stkscan-stksave)/2+2;

                                BINT sizedelta=3-rplObjSize(prog);

                                WORDPTR *sptr=DSTop;
                                while(sptr<stktop) {
                                    if( (offset>=(*sptr-newobj)) && (offset<(rplSkipOb(*sptr)-newobj)) ) {
                                        // PATCH THE SIZE
                                        **sptr+=sizedelta;
                                    }
                                    ++sptr;
                                }

                            }
                            else {
                             // JUST COPY THE OBJECT
                             rplCopyObject(newobj+offset,prog);
                             offset+=rplObjSize(prog);
                            }

                    }
                    else {
                        // ANY OTHER OBJECT, JUST COPY VERBATIM
                        if(ISPROGRAM(*prog) || ISLIST(*prog))
                        {
                            WORDPTR *tmp=DSTop;
                            DSTop=stktop;
                            // CREATE A STACK OF OBJECTS TO PATCH SIZE
                            ScratchPointer1=newobj;
                            ScratchPointer2=prog;
                            ScratchPointer3=endprog;
                            rplPushData(newobj+offset);
                            if(Exceptions) { DSTop=stksave; return; }

                            newobj=ScratchPointer1;
                            prog=ScratchPointer2;
                            endprog=ScratchPointer3;
                            DSTop=tmp;

                            newobj[offset]=*prog;
                            ++offset;
                            ++prog;
                            continue;
                        }
                        else {
                            // JUST COPY THE OBJECT
                            rplCopyObject(newobj+offset,prog);
                            offset+=rplObjSize(prog);
                        }

                    }






                prog=rplSkipOb(prog);

            }

        }

        // DONE, LIBRARY IS READY

        DSTop=stksave;
        rplPushData(newobj);

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


        if(ISLIBPTR(*rplPeekData(1))) { // THIS IS A LIBPTR - NEEDS TO BE EXECUTED
            WORDPTR libobj=rplGetLibPtr(rplPeekData(1));

            if(!libobj) {
                rplError(ERR_MISSINGLIBRARY);
                return;
            }

            rplPushData(libobj);
            rplCallOvrOperator(CMD_OVR_XEQ);
            return;
        }


        // IT'S A LIBRARY - DO NOTHING

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
    {
        if((TokenLen==7) && (!utf8ncmp2((char *)TokenStart,(char *)BlankStart,"LIBRARY",7))) {

            ScratchPointer4=CompileEnd;
            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER,0));
            RetNum=OK_NEEDMORE;
            return;
        }


        // COMPILE MISSING LIBRARY ROMPOINTERS

        if((TokenLen==6) && (!utf8ncmp2((char *)TokenStart,(char *)BlankStart,"LIBPTR",6))) {
            ScratchPointer4=CompileEnd;
            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER+1,2));
            RetNum=OK_NEEDMORE;
            return;
        }


        // COMPILE COMMANDS FOR ALL OTHER REGISTERED LIBRARIES

        BINT64 libidx=rplFindLibPtrIndex((BYTEPTR)TokenStart,(BYTEPTR)BlankStart);

        if(libidx>=0) {
            // FOUND A MATCH
            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER+1,2));
            rplCompileAppend((WORD)libidx);
            rplCompileAppend((WORD)(libidx>>32));
            RetNum=OK_CONTINUE;
            return;
        }


            // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
            // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;
    }
    case OPCODE_COMPILECONT:
    {
        if(LIBNUM(*ScratchPointer4)==LIBRARY_NUMBER+1) {
            // COMPILE A ROMPOINTER WITH A MISSING LIBRARY
            BYTEPTR ptr=(BYTEPTR)TokenStart;
            int rot=0;
            WORD libid=0,libcmd;
            BINT cp;
            while(ptr<(BYTEPTR)BlankStart) {
                if(*ptr=='.') break;
                cp=utf82cp((char *)ptr,(char *)BlankStart);
                if( ((cp>='A')&&(cp<='Z')) || ((cp>='a')&&(cp<='z')) || ((cp>='0')&&(cp<='9'))) {
                    libid|=cp<<rot;
                    rot+=8;
                    if(rot>=32) break;
                }
                else {
                    RetNum=ERR_SYNTAX;
                    return;
                }

                ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)BlankStart);
            }

            libcmd=0;
            while(ptr<(BYTEPTR)BlankStart) {
                if(*ptr=='.') { ++ptr; continue; }
                cp=*ptr-'0';
                if( (cp<0)||(cp>9)) {
                    RetNum=ERR_SYNTAX;
                    return;
                }
                libcmd*=10;
                libcmd+=cp;
                ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)BlankStart);
            }

            rplCompileAppend(libid);
            rplCompileAppend(libcmd);
            RetNum=OK_CONTINUE;
            return;
        }




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
            if(ISLIBPTR(*DecompileObject)) {
                // DECOMPILE A LIBPTR

                WORDPTR name=rplGetLibPtrName(DecompileObject);

                if(!name) {
                    // LIBPTRS WITHOUT A PROPER LIBRARY INSTALLED
                    rplDecompAppendString((BYTEPTR)"LIBPTR ");

                    BYTEPTR ptr=(BYTEPTR)(DecompileObject+1);
                    if(*ptr!=0) rplDecompAppendChar(*ptr);
                    ++ptr;
                    if(*ptr!=0) rplDecompAppendChar(*ptr);
                    ++ptr;
                    if(*ptr!=0) rplDecompAppendChar(*ptr);
                    ++ptr;
                    if(*ptr!=0) rplDecompAppendChar(*ptr);
                    ++ptr;
                    rplDecompAppendChar('.');
                    BYTE buffer[22];
                    BINT n=rplIntToString(DecompileObject[2],10,buffer,buffer+22);
                    ptr=buffer;
                    while(n) { rplDecompAppendChar(*ptr); ++ptr; }


                }
                else {
                // NAMED COMMAND
                rplDecompAppendString2((BYTEPTR)(name+1),rplGetIdentLength(name));
                }

                RetNum=OK_CONTINUE;
                return;

            }


            // DECOMPILE LIBRARY

            rplDecompAppendString((BYTEPTR)"LIBRARY ");
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

