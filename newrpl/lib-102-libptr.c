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

//@TITLE=User Libraries

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(CRLIB,MKTOKENINFO(5,TITYPE_NOTALLOWED,0,2)), \
    CMD(ATTACH,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(DETACH,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(LIBMENU,MKTOKENINFO(7,TITYPE_NOTALLOWED,2,2)), \
    CMD(LIBMENUOTHR,MKTOKENINFO(11,TITYPE_NOTALLOWED,2,2)), \
    CMD(LIBMENULST,MKTOKENINFO(10,TITYPE_NOTALLOWED,2,2)), \
    CMD(LIBSTO,MKTOKENINFO(6,TITYPE_NOTALLOWED,2,2)), \
    CMD(LIBRCL,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(LIBDEFRCL,MKTOKENINFO(6,TITYPE_NOTALLOWED,2,2)), \
    CMD(LIBCLEAR,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2))






// ADD MORE OPCODES HERE

#define ERROR_LIST \
    ERR(INVALIDLIBID,0), \
    ERR(INVALIDVISIBLE,1), \
    ERR(LIBRARYEXPECTED,2), \
    ERR(UNABLETOATTACH,3)




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

ROMOBJECT libdata_dirname[]={
    MKPROLOG(DOIDENT,2),
    TEXT2WORD('L','I','B','D'),
    TEXT2WORD('A','T','A',0)
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

ROMOBJECT libmenu_ident[]={
    MKPROLOG(DOIDENT,2),
    TEXT2WORD('$','M','E','N'),
    TEXT2WORD('U',0,0,0)
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
    (WORDPTR)libdata_dirname,
    (WORDPTR)libid_ident,
    (WORDPTR)title_ident,
    (WORDPTR)libmenu_ident,
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
                if(libcmd>OPCODE(direntry[1][3])) return 0;

                return direntry[1]+OPCODE(direntry[1][libcmd+4]);
            }

        }

    } while((direntry=rplFindNext(direntry)));


   return 0;
}


// SAME AS GETLIBPTR, BUT THE LIBID AND COMMAND NUMBER ARE GIVEN SEPARATELY INSTEAD OF BY OBJECT

WORDPTR rplGetLibPtr2(WORD libid,WORD libcmd)
{

    WORDPTR libdir=rplGetSettings((WORDPTR)library_dirname);

    if(!libdir) return 0;

    WORDPTR *direntry=rplFindFirstByHandle(libdir);

    if(!direntry) return 0;

    do {

        if(ISIDENT(*direntry[0]) && (OBJSIZE(*direntry[0])==1)) {
            // COMPARE LIBRARY ID
            if(direntry[0][1] == libid) {
                // FOUND THE LIBRARY - GET THE COMMAND
                if(libcmd>OPCODE(direntry[1][3])) return 0;

                return direntry[1]+OPCODE(direntry[1][libcmd+4]);
            }

        }

    } while((direntry=rplFindNext(direntry)));


   return 0;
}




// FIND THE LIBRARY THAT CONTAINS THE GIVEN POINTER
// NO ARGUMENT CHECKS FOR SPEED

WORDPTR rplGetLibFromPointer(WORDPTR libptr)
{

    WORDPTR libdir=rplGetSettings((WORDPTR)library_dirname);

    if(!libdir) return 0;

    WORDPTR *direntry=rplFindFirstByHandle(libdir);

    if(!direntry) return 0;

    do {

        if(ISIDENT(*direntry[0]) && (OBJSIZE(*direntry[0])==1)) {
            // SEE IF THE POINTER IS INSIDE THE LIBRARY
            if( (libptr>=direntry[1])&&(libptr<rplSkipOb(direntry[1]))) {
                // FOUND THE LIBRARY - GET THE COMMAND
                return direntry[1];
            }

        }

    } while((direntry=rplFindNext(direntry)));


   return 0;
}





// GET THE NAME OF A LIBPTR

WORDPTR rplGetLibPtrName2(WORD libid,WORD libcmd)
{

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

WORDPTR rplGetLibPtrName(WORDPTR libptr)
{
    WORD libid=libptr[1];
    WORD libcmd=libptr[2];

    return rplGetLibPtrName2(libid,libcmd);
}



// GET THE TOKEN INFO OF A LIBPTR

WORDPTR rplGetLibPtrInfo(WORDPTR libptr)
{
WORDPTR result=rplGetLibPtrName(libptr);
if(result) return rplSkipOb(result);
return 0;
}

WORDPTR rplGetLibPtrHelp(WORD libid,WORD libcmd)
{
WORDPTR result=rplGetLibPtrName2(libid,libcmd);
if(result) return rplSkipOb(rplSkipOb(result));
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
    BINT idx;       //,ncommands;

    do {

        if(ISLIBRARY(*direntry[1])) {
            //ncommands=OPCODE(direntry[1][3]);
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
            cmd=rplSkipOb(cmd); // SKIP HELPTEXT
            if(cmd>=libend) break;
            cmd=rplSkipOb(cmd); // SKIP OBJECT

            ++idx;
            }
        }

    } while((direntry=rplFindNext(direntry)));


   return -1;
}

// RETURN LIBRARY AND INDEX TO THE COMMAND THAT HAS THE LONGEST MATCH
// AT THE BEGINNING OF THE TOKEN (USED FOR PROBING)
// ALSO SETS *cmdinfo TO THE COMMAND INFORMATION LIST (NAME / TOKENINFO / HELPINFO / OBJECT)

BINT64 rplProbeLibPtrIndex(BYTEPTR start,BYTEPTR end,WORDPTR *cmdinfo)
{

    WORDPTR libdir=rplGetSettings((WORDPTR)library_dirname);

    if(!libdir) return -1;

    WORDPTR *direntry=rplFindFirstByHandle(libdir);

    if(!direntry) return -1;

    WORDPTR cmd,libend;
    BINT idx;       //,ncommands;
    BINT len,maxlen,chosenidx;
    WORD chosenlib;
    WORDPTR chosencmd;

    maxlen=0;

    do {

        if(ISLIBRARY(*direntry[1])) {
            //ncommands=OPCODE(direntry[1][3]);
            idx=0;
            libend=rplSkipOb(direntry[1]);

            cmd=direntry[1]+OPCODE(direntry[1][3])+4;

            // SCAN THROUGH ALL THE COMMANDS
            while (cmd<libend) {
            len=rplGetIdentLength(cmd); // LENGTH IN BYTES
            if(end-start<len) len=end-start;

            if(rplCompareIDENTByName(cmd,start,start+len)) {
                // FOUND A MATCH
                if(len>maxlen) { chosencmd=cmd; chosenlib=direntry[1][2]; chosenidx=idx; maxlen=len; }
            }

            cmd=rplSkipOb(cmd); // SKIP NAME
            if(cmd>=libend) break;
            cmd=rplSkipOb(cmd); // SKIP TOKENINFO
            if(cmd>=libend) break;
            cmd=rplSkipOb(cmd); // SKIP HELPTEXT
            if(cmd>=libend) break;
            cmd=rplSkipOb(cmd); // SKIP OBJECT

            ++idx;
            }
        }

    } while((direntry=rplFindNext(direntry)));


    if(maxlen!=0) {
        if(cmdinfo) *cmdinfo=chosencmd;
        return (((BINT64)chosenidx)<<32)| chosenlib;
    }

   return -1;
}


WORDPTR *rplFindAttachedLibrary(WORDPTR ident)
{

    if(OBJSIZE(*ident)!=1) return 0;

    WORDPTR libdir=rplGetSettings((WORDPTR)library_dirname);

    if(!libdir) return 0;

    WORDPTR *direntry=rplFindFirstByHandle(libdir);

    if(!direntry) return 0;

    do {

        if(ISIDENT(*direntry[0]) && (OBJSIZE(*direntry[0])==1)) {
            // COMPARE LIBRARY ID
            if(direntry[0][1] == ident[1]) {
                // FOUND THE LIBRARY
                return direntry;
            }
        }

    } while((direntry=rplFindNext(direntry)));


   return 0;
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
        //@SHORT_DESC=Create a library from current directory
        //@INCOMPAT
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
         *                         SINT = { TOKENINFO SIMPLIFIED: NARGS*256+(ALLOWEDINSYMBOLICS?)
         *                         HELPTEXT = STRING TO DISPLAY COMMAND HELP
         *                         OBJECT = WHATEVER THIS NAMED OBJECT IS
         *                         REPEATS IDENT/SINT/OBJECT GROUPS UNTIL END OF LIBRARY
         *
         *                         IDX=0 (LIBPTR INDEX 0) ===> DEFAULT LIBRARY HANDLER (NULL NAME/HELP, ETC)
         *                         IDX=1 (LIBPTR INDEX 1) ===> LIBRARY TITLE (NULL NAME/HELP, ETC)
         *                         IDX=2 (LIBPTR INDEX 2) ===> LIBRARY MENU  (NULL NAME/HELP, ETC)
         *                         IDX=3 (LIBPTR INDEX 3) ===> RESERVED FOR FUTURE USE (NULL NAME/HELP, ETC)
         *                         IDX=4 TO N  ===> USER VISIBLE AND INVISIBLE COMMANDS.
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
        // { { IDENT NARGS ALLOWINSYMB HELPTEXT } { IDENT NARGS ALLOWINSYMB HELPTEXT } ... }


        // LIBRARY CREATION - PASS 1

        // CHECK FOR VALID $LIBID

        WORD libid=0;
        BINT nvisible,nhidden,nsizeextra,datasize;
        WORDPTR *object;
        WORDPTR *stksave=DSTop;

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
        // HANDLER USES ENTRY #0, IT TAKES A NULL-NAME (1 WORD), INFO=1 WORD, NULLHELP=1 WORD AND SIZE OF HANDLER
        object=rplFindGlobal((WORDPTR)handler_ident,0);

        if(object) {
            datasize+=3+rplObjSize(object[1]);
            rplPushData((WORDPTR)handler_ident);
            rplPushData(object[1]);
        }
        else {
            datasize+=3+rplObjSize((WORDPTR)defhandler_seco);
            rplPushData((WORDPTR)handler_ident);
            rplPushData((WORDPTR)defhandler_seco);
        }
        if(Exceptions) { DSTop=stksave; return; }



        // CHECK FOR VALID $TITLE

        object=rplFindGlobal((WORDPTR)title_ident,0);

        // TITLE USES ENTRY #1, SO IT TAKES A NULL-NAME (A SINT=1 WORD), INFO=1 WORD, AND THE SIZE OF THE STRING
        if(object) {
            datasize+=3+rplObjSize(object[1]);
            rplPushData((WORDPTR)title_ident);
            rplPushData(object[1]);
        }
        else {
            datasize+=3+rplObjSize((WORDPTR)empty_string);

            rplPushData((WORDPTR)title_ident);
            rplPushData((WORDPTR)empty_string);
        }
        if(Exceptions) { DSTop=stksave; return; }


        // CHECK FOR VALID $MENU

        object=rplFindGlobal((WORDPTR)libmenu_ident,0);

        // TITLE USES ENTRY #2, SO IT TAKES A NULL-NAME (A SINT=1 WORD), INFO=1 WORD, AND THE SIZE OF THE LIST
        if(object) {
            datasize+=3+rplObjSize(object[1]);
            rplPushData((WORDPTR)libmenu_ident);
            rplPushData(object[1]);
        }
        else {
            datasize+=3+rplObjSize((WORDPTR)empty_list);
            rplPushData((WORDPTR)libmenu_ident);
            rplPushData((WORDPTR)empty_list);
        }
        if(Exceptions) { DSTop=stksave; return;}


        // RESERVED FOR FUTURE USE, TAKES 1-WORD NULL NAME, 1-WORD INFO, 1-WORD NULL HELP, 1-WORD FOR THE ZERO_BINT
        rplPushData((WORDPTR)nulllam_ident);
        rplPushData((WORDPTR)zero_bint);
        if(Exceptions) { DSTop=stksave; return; }
        datasize+=4;


        // CHECK FOR VALID $VISIBLE

        object=rplFindGlobal((WORDPTR)visible_ident,0);


        if(object) {
            if(ISLIST(*object[1])) {
                nvisible=rplListLength(object[1]);
                int k;
                WORDPTR item=object[1]+1;
                for(k=0;k<nvisible;++k) {
                    if( ISLIST(*item) && (rplListLength(item)==4) ) {
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
                                if( ISNUMBER(*var) && ISNUMBER(*rplSkipOb(var)) && ISSTRING(*rplSkipOb(rplSkipOb(var)))) {
                                    var=rplSkipOb(rplSkipOb(var));
                                    datasize+=rplObjSize(var);
                                    item=rplSkipOb(item); continue;
                                }
                            }
                        }
                    }
                    nvisible=0;
                    break;
                }
                // ADJUST NUMBER OF VISIBLE VARIABLES
                nvisible=(DSTop-stksave-8)>>1;

            }
        }
        if(!nvisible) {
            DSTop=stksave;
            rplError(ERR_INVALIDVISIBLE);
            return;
        }

        //  CREATE A DEFAULT MENU FROM THE VISIBLE LIST
        //  BUT ONLY IF A CUSTOM MENU WASN'T GIVEN IN A $MENU VARIABLE
        if(stksave[5]==(WORDPTR)empty_list) {

            int k;
            BINT listsize=1+3*nvisible;     // ENDLIST + LIBPTR FOR EACH VISIBLE VARIABLE

            WORDPTR defmenu=rplAllocTempOb(listsize);
            // COMPUTE THE SIZE OF A MENU
            if(!defmenu) { DSTop=stksave; return; }

            defmenu[0]=MKPROLOG(DOLIST,listsize);
            for(k=0;k<nvisible;++k) {
                defmenu[1+3*k]=MKPROLOG(DOLIBPTR,2);
                defmenu[2+3*k]=libid;
                defmenu[3+3*k]=k+4;
            }
            defmenu[1+3*k]=CMD_ENDLIST;

            // REPLACE THE EMPTY LIST WITH THE NEW MENU
            stksave[5]=defmenu;

            // UPDATE THE SIZE COMPUTATION
            datasize-=rplObjSize((WORDPTR)empty_list);
            datasize+=listsize+1;

        }


        if(Exceptions) { DSTop=stksave; return; }

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
                            if(ignitem<endign) {
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

                            datasize+=rplObjSize(rplPeekData(2))+rplObjSize(rplPeekData(1))+2;  // NAME, INFO, NULL HELP, OBJECT

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

        BINT totalsize=3+(nvisible+nhidden)+datasize+nsizeextra;


        WORDPTR newobj=rplAllocTempOb(totalsize);

        if(!newobj) { DSTop=stksave; return; }

        // WRITE THE LIBRARY OBJECT:
        newobj[0]=MKPROLOG(DOLIBRARY,totalsize);
        newobj[1]=MKPROLOG(DOIDENT,1);
        newobj[2]=libid;
        newobj[3]=MAKESINT(nvisible+nhidden);

        int k,totaln=nvisible+nhidden;
        BINT offset=4+totaln;

        for(k=0;k<totaln;++k) {
            // ADD COMMAND NAME
            rplCopyObject(newobj+offset,(k>=4)? stksave[2*k] : (WORDPTR)nulllam_ident);

            offset+=rplObjSize(newobj+offset);

            // ADD INFO
            if((k>=4)&&(k<nvisible+4)) {
                WORDPTR info=rplSkipOb(stksave[2*k]);

                if(object && (info>object[1]) && (info<rplSkipOb(object[1]))) {

                BINT nargs=rplReadNumberAsBINT(info);
                BINT allow=!rplIsFalse(rplSkipOb(info));

                newobj[offset]=MAKESINT( (nargs<<8) | ((allow)? 1:0));
                }
                else newobj[offset]=MAKESINT(0);
            }
            else newobj[offset]=MAKESINT(0);

            offset++;

            // ADD HELP STRING
            {
            WORDPTR helpstring;
            helpstring=(WORDPTR)empty_string;
            if((k>=4)&&(k<nvisible+4)) {
                helpstring=rplSkipOb(stksave[2*k]);

                if(object && (helpstring>object[1]) && (helpstring<rplSkipOb(object[1]))) helpstring=rplSkipOb(rplSkipOb(helpstring));
                else helpstring=(WORDPTR)empty_string;
            }
            rplCopyObject(newobj+offset,helpstring);
            offset+=rplObjSize(helpstring);
            }

            // ADD THE POINTER TO THE OBJECT IN THE HASH TABLE
            newobj[4+k]=offset;

            // AND FINALLY ADD THE OBJECT ITSELF
                WORDPTR prog;

                prog=stksave[2*k+1];

                WORDPTR endprog=rplSkipOb(prog);
                WORDPTR *stktop=DSTop;


                while(prog!=endprog) {
                    if(ISUNQUOTEDIDENT(*prog)) {

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
                            if(ignitem<endign) {
                                // VARIABLE IS IN THE IGNORE LIST - NOTHING TO DO HERE
                                // JUST COPY THE OBJECT
                                rplCopyObject(newobj+offset,prog);
                                offset+=rplObjSize(prog);

                                prog=rplSkipOb(prog);
                                continue;
                            }


                        }

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
                                newobj[offset++]=(stkscan-stksave)/2;

                                BINT sizedelta=3-rplObjSize(prog);

                                WORDPTR *sptr=DSTop;
                                while(sptr<stktop) {
                                    if( (offset>=(*sptr-newobj)) && (offset<(rplSkipOb(*sptr)-newobj+sizedelta)) ) {
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

                            stktop=DSTop;
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


    case ATTACH:
    {
        //@SHORT_DESC=Install a library
        //@INCOMPAT
        // ATTACH A LIBRARY

        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISLIBRARY(*rplPeekData(1))) {
            rplError(ERR_LIBRARYEXPECTED);
            return;
        }

        WORDPTR libdir=rplGetSettings((WORDPTR)library_dirname);

        if(!libdir) {
            // CREATE THE DIRECTORY!
            WORDPTR *sethan=rplFindDirbyHandle(SettingsDir);
            if(!sethan) {
                rplError(ERR_DIRECTORYNOTFOUND);
                return;
            }

            libdir=rplCreateNewDir((WORDPTR)library_dirname,sethan);

            if(!libdir) return;

        }

        // STORE THE LIBRARY IN THE DIRECTORY
        WORDPTR lib=rplPeekData(1);
        WORDPTR *libdirentry=rplFindDirbyHandle(libdir);

        WORDPTR *var=rplFindGlobalInDir(lib+1,libdirentry,0);
        if(var) {
            // LIBRARY IS ALREADY INSTALLED, REPLACE
            var[0]=lib+1;
            var[1]=lib;
        }
        else rplCreateGlobalInDir(lib+1,lib,rplFindDirbyHandle(libdir));

        lib=rplPopData();

        rplPushDataNoGrow(lib+1);
        rplPushData(lib+lib[5]);
        return;

    }

    case DETACH:
    {
        //@SHORT_DESC=Uninstall a library
        //@INCOMPAT
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_INVALIDLIBID);
            return;
        }

        WORDPTR *var=rplFindAttachedLibrary(rplPeekData(1));

        if(var) rplPurgeForced(var);

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


        if(ISLIBPTR(*rplPeekData(1))) { // THIS IS A LIBPTR - NEEDS TO BE EXECUTED
            WORDPTR libobj=rplGetLibPtr(rplPeekData(1));

            if(!libobj) {
                rplError(ERR_MISSINGLIBRARY);
                return;
            }

            rplOverwriteData(1,libobj);
            rplCallOvrOperator(CMD_OVR_XEQ);
            return;
        }


        // IT'S A LIBRARY - DO NOTHING

        return;





    case LIBMENU:
     {
        //@SHORT_DESC=Show a menu within a library
        //@NEW
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        BINT64 mcode;
        mcode=rplReadNumberAsBINT(rplPeekData(1));
        if(Exceptions) return;
        WORD libid;

        if(ISIDENT(*rplPeekData(2))) libid=rplPeekData(2)[1];
        else {
            rplError(ERR_INVALIDLIBID);
            return;
        }

        mcode=(((BINT64)libid)<<32)| MKMENUCODE(0,DOLIBPTR,MENUNUMBER(mcode),MENUPAGE(mcode));
        WORDPTR newmenu=rplNewBINT(mcode,HEXBINT);
        if(!newmenu) return;


        rplPushDataNoGrow(newmenu);
        BINT menu=rplGetActiveMenu();
        rplSaveMenuHistory(menu);
        rplChangeMenu(menu,rplPopData());

        if(!Exceptions) rplDropData(2);

      return;
    }


    case LIBMENULST:
        //@SHORT_DESC=Show library menu in the last used menu
        //@NEW
    case LIBMENUOTHR:
        //@SHORT_DESC=Show library menu in the other menu
        //@NEW
     {

      if(rplDepthData()<2) {
          rplError(ERR_BADARGCOUNT);
          return;
      }
      rplStripTagStack(2);

      BINT64 mcode;
      mcode=rplReadNumberAsBINT(rplPeekData(1));
      if(Exceptions) return;
      WORD libid;

      if(ISIDENT(*rplPeekData(2))) libid=rplPeekData(2)[1];
      else {
          rplError(ERR_INVALIDLIBID);
          return;
      }

      mcode=(((BINT64)libid)<<32)| MKMENUCODE(0,DOLIBPTR,MENUNUMBER(mcode),MENUPAGE(mcode));
      WORDPTR newmenu=rplNewBINT(mcode,HEXBINT);
      if(!newmenu) return;


      BINT menu=rplGetLastMenu();
      if(CurOpcode==CMD_LIBMENUOTHR) {
          // USE THE OTHER MENU
          if(menu==1) menu=2;
          else menu=1;
      }

      rplPushDataNoGrow(newmenu);
      rplSaveMenuHistory(menu);

      rplChangeMenu(menu,rplPopData());

      if(!Exceptions) rplDropData(2);

    return;



    }


    case LIBSTO:
    {
        //@SHORT_DESC=Store private library data
        //@NEW
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        // FIND CURRENTLY EXECUTING LIBRARY
        WORDPTR library=rplGetLibFromPointer(IPtr);
        if(!library) {
            // STORE IT IN THE CURRENT DIRECTORY
            rplCallOperator(CMD_STO);
            return;
        }

        // FIND LIBDATA DIRECTORY, CREATE IF DOESN'T EXIST
        WORDPTR dirhandle=rplGetSettings((WORDPTR)libdata_dirname);
        if(!dirhandle) {
            rplPushDataNoGrow(library);
            dirhandle=rplCreateNewDir((WORDPTR)libdata_dirname,rplFindDirbyHandle(SettingsDir));
            library=rplPopData();
            if(!dirhandle || Exceptions) return;

        }

        // FIND LIBRARY DIRECTORY WITHIN LIBDATA, CREATE IF DOESN'T EXIST
        WORDPTR *dirptr=rplFindDirbyHandle(dirhandle);
        WORDPTR *var=rplFindGlobalInDir(library+1,dirptr,0);

        if(!var) dirhandle=rplCreateNewDir(library+1,dirptr);
        else dirhandle=var[1];
        if(!dirhandle || Exceptions) return;

        // FINALLY, FIND THE VARIABLE IN THE EXISTING DIRECTORY, CREATE OR MODIFY

        var=rplFindGlobalInDir(rplPeekData(1),rplFindDirbyHandle(dirhandle),0);

        if(!var) rplCreateGlobalInDir(rplPeekData(1),rplPeekData(2),rplFindDirbyHandle(dirhandle));
        else var[1]=rplPeekData(2);
        if(Exceptions) return;

        rplDropData(2);
        return;

    }


    case LIBRCL:
    {
        //@SHORT_DESC=Recall private library data
        //@NEW
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        // FIND CURRENTLY EXECUTING LIBRARY
        WORDPTR library=rplGetLibFromPointer(IPtr);
        if(!library) {
            // GET IT FROM THE CURRENT DIRECTORY
            rplCallOperator(CMD_RCL);
            return;
        }

        // FIND LIBDATA DIRECTORY
        WORDPTR dirhandle=rplGetSettings((WORDPTR)libdata_dirname);
        if(!dirhandle) {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

        // FIND LIBRARY DIRECTORY WITHIN LIBDATA
        WORDPTR *dirptr=rplFindDirbyHandle(dirhandle);
        WORDPTR *var=rplFindGlobalInDir(library+1,dirptr,0);

        if(!var) {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }
        else dirhandle=var[1];


        // FINALLY, FIND THE VARIABLE IN THE EXISTING DIRECTORY, CREATE OR MODIFY

        var=rplFindGlobalInDir(rplPeekData(1),rplFindDirbyHandle(dirhandle),0);

        if(!var) {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

        rplOverwriteData(1,var[1]);
        return;

    }

    case LIBDEFRCL:
    {
        //@SHORT_DESC=Recall private data with default value
        //@NEW
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        // FIND CURRENTLY EXECUTING LIBRARY
        WORDPTR library=rplGetLibFromPointer(IPtr);
        if(!library) {
            // GET IT FROM THE CURRENT DIRECTORY
            rplCallOperator(CMD_RCL);
            if(!Exceptions) { rplOverwriteData(2,rplPeekData(1)); rplDropData(1); }

            else {
                  // ONLY CLEAR ERRORS IF IT WAS CAUSED BY UNDEF VARIABLE
                if((Exceptions==EX_ERRORCODE)&&(ErrorCode==ERR_UNDEFINEDVARIABLE))
                {
                    // CLEAR ERROR AND LEAVE DEFAULT VALUE ON THE STACK
                    rplClearErrors();
                    rplDropData(1);
                }
            }
            return;
        }

        // FIND LIBDATA DIRECTORY
        WORDPTR dirhandle=rplGetSettings((WORDPTR)libdata_dirname);
        if(!dirhandle) {
            rplDropData(1);
            return;
        }

        // FIND LIBRARY DIRECTORY WITHIN LIBDATA
        WORDPTR *dirptr=rplFindDirbyHandle(dirhandle);
        WORDPTR *var=rplFindGlobalInDir(library+1,dirptr,0);

        if(!var) {
            rplDropData(1);
            return;
        }
        else dirhandle=var[1];


        // FINALLY, FIND THE VARIABLE IN THE EXISTING DIRECTORY, CREATE OR MODIFY

        var=rplFindGlobalInDir(rplPeekData(1),rplFindDirbyHandle(dirhandle),0);

        if(!var) {
            rplDropData(1);
            return;
        }

        rplOverwriteData(2,var[1]);
        rplDropData(1);
        return;

    }


    case LIBCLEAR:
    {
        //@SHORT_DESC=Purge all private data for a specific library
        //@NEW
        // PURGE ENTIRE DATA DIRECTORY, AND CALL THE LIBRARY HANDLER TO CREATE A NEW ONE

        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_INVALIDLIBID);
            return;
        }


            // FIND LIBDATA DIRECTORY
            WORDPTR dirhandle=rplGetSettings((WORDPTR)libdata_dirname);
            if(!dirhandle) {
                // DO NOTHING IF THE LIBRARY HAS NO DATA
                rplDropData(1);
                return;
            }

            // FIND LIBRARY DIRECTORY WITHIN LIBDATA
            WORDPTR *dirptr=rplFindDirbyHandle(dirhandle);
            WORDPTR *var2=rplFindGlobalInDir(rplPeekData(1),dirptr,0);

            if(!var2) {
                // DO NOTHING IF THE LIBRARY HAS NO DATA
                rplDropData(1);
                return;
            }
            else dirhandle=var2[1];

            rplPurgeDirByHandle(dirhandle);


        rplDropData(1);

      return;
    }








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
            rplCompileAppend(MKPROLOG(0,0));
            RetNum=OK_NEEDMORE;
            return;
        }


        // COMPILE MISSING LIBRARY ROMPOINTERS

        if((TokenLen==6) && (!utf8ncmp2((char *)TokenStart,(char *)BlankStart,"LIBPTR",6))) {
            ScratchPointer4=CompileEnd;
            rplCompileAppend(MKPROLOG(DOLIBPTR,2));
            RetNum=OK_NEEDMORE;
            return;
        }


        // COMPILE COMMANDS FOR ALL OTHER REGISTERED LIBRARIES

        BINT64 libidx=rplFindLibPtrIndex((BYTEPTR)TokenStart,(BYTEPTR)BlankStart);

        if(libidx>=0) {
            // FOUND A MATCH
            rplCompileAppend(MKPROLOG(DOLIBPTR,2));
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
        if(LIBNUM(*ScratchPointer4)==DOLIBPTR) {
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
                    if(rot>=32) {
                        ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)BlankStart);
                        break;
                    }
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
                else if((*ptr>='A')&&(*ptr<='F')) digit=*ptr-'A'+10;
                    else if((*ptr>='a')&&(*ptr<='f')) digit=*ptr-'a'+10;
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

            *ScratchPointer4=MKPROLOG(0,value);
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

        while(((CompileEnd-ScratchPointer4-1)<(BINT)OBJSIZE(*ScratchPointer4)))
        {
             do {
                if((*ptr>='0')&&(*ptr<='9')) dig=(*ptr+4);
                else if((*ptr>='A')&&(*ptr<='Z')) dig=(*ptr-65);
                else if((*ptr>='a')&&(*ptr<='z')) dig=(*ptr-71);
                else if(*ptr=='#') dig=62;
                else if(*ptr=='$') dig=63;
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
            if(ndigits || (((CompileEnd-ScratchPointer4-1)<(BINT)OBJSIZE(*ScratchPointer4)))) {
                // INCOMPLETE WORD, PREPARE FOR RESUME ON NEXT TOKEN
                rplCompileAppend(value);
                rplCompileAppend(ndigits | (checksum<<16));
                *ScratchPointer4|=0x00100000;
                RetNum=OK_NEEDMORE;
                return;
            }
            else *ScratchPointer4=MKPROLOG(DOLIBRARY,OBJSIZE(*ScratchPointer4));


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

                if(!name || (*name==CMD_NULLLAM)) {
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
                    BINT n=rplIntToString(DecompileObject[2],DECBINT,buffer,buffer+22);
                    ptr=buffer;
                    while(n--) { rplDecompAppendChar(*ptr); ++ptr; }


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
                    else if(encoder[k]==62) encoder[k]='#';
                    else encoder[k]='$';
                }

                ScratchPointer1=ptr;
                rplDecompAppendString(encoder);
                if(Exceptions) {
                    RetNum=ERR_INVALID;
                    return;
                }
                ptr=ScratchPointer1;

                ++ptr;

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

        // PROBE LIBRARY COMMANDS FIRST
        WORDPTR cmdinfo;
        BINT64 libptr=rplProbeLibPtrIndex((BYTEPTR)TokenStart,(BYTEPTR)BlankStart,&cmdinfo);

        if(libptr>=0) {
            // FOUND A MATCH!
            BINT len=utf8nlenst((char *)(cmdinfo+1),((char *)(cmdinfo+1)) + rplGetIdentLength(cmdinfo));
            BINT nargs=OPCODE(*rplSkipOb(cmdinfo));
            BINT allow=nargs&1;

            nargs>>=8;

            if(allow) RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_IDENT,nargs,2);
            else RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_NOTALLOWED,nargs,1);
            return;
        }

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
        TypeInfo=LIBNUM(*ObjectPTR)*100;
        DecompHints=0;

        if(ISLIBPTR(*ObjectPTR)) {

                // FOUND A MATCH!
                WORDPTR cmdinfo=rplGetLibPtrName(ObjectPTR);
                if(cmdinfo) {
                BINT nargs=OPCODE(*rplSkipOb(cmdinfo));
                BINT allow=nargs&1;
                BINT len;

                if(ISIDENT(*cmdinfo)) len=utf8nlenst((char *)(cmdinfo+1),(char *)(cmdinfo+1)+rplGetIdentLength(cmdinfo));
                else len=0;

                nargs>>=8;

                if(allow) RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_IDENT,nargs,2);
                else RetNum=OK_TOKENINFO | MKTOKENINFO(len,TITYPE_NOTALLOWED,nargs,1);
                return;
                }

        }


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


        RetNum=OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:


    {
        // AUTOCOMPLETE NAMES OF LIBRARIES
        // TokenStart = token string
        // TokenLen = token length
        // SuggestedOpcode = OPCODE OF THE CURRENT SUGGESTION, OR THE PROLOG OF THE OBJECT IF SUGGESTION IS AN OBJECT
        // SuggestedObject = POINTER TO AN OBJECT (ONLY VALID IF ISPROLOG(SuggestedOpcode)==True)


        // AUTOMCOMPLETE FIRST COMMANDS OF INSTALLED LIBRARIES

        WORDPTR libdir=rplGetSettings((WORDPTR)library_dirname);

        if(libdir) {

        WORD prevlibid=0;
        BINT previdx=0;
        if(ISPROLOG(SuggestedOpcode) && SuggestedObject) {
            if(ISLIBPTR(SuggestedOpcode)) {
                prevlibid=SuggestedObject[1];
                previdx=SuggestedObject[2];
            }
        }
        WORDPTR *direntry=rplFindFirstByHandle(libdir);

        if(direntry) {

            do {

                if(ISIDENT(*direntry[0]) && (OBJSIZE(*direntry[0])==1)) {

                    BINT nentries=OPCODE(direntry[1][3]);
                    // COMPARE LIBRARY ID
                    if(prevlibid)
                    {
                        // SKIP UNTIL WE FIND THE PREVIOUS LIBRARY
                        if(direntry[0][1]!=prevlibid) continue;
                        // FOUND IT, previdx HAS THE START INDEX
                        prevlibid=0;
                        if(previdx<=4) continue;    // THE PREVIOUS COMMAND WAS THE FIRST COMMAND, SKIP TO THE NEXT LIBRARY
                    } else previdx=nentries;

                    if(previdx>nentries) previdx=nentries;
                    WORDPTR nameptr;
                    do {
                        --previdx;
                            nameptr=rplSkipOb(direntry[1]+OPCODE(direntry[1][previdx+3]));
                            if(ISIDENT(*nameptr)) {
                                // COMPARE IDENT WITH THE GIVEN TOKEN
                                BINT len,idlen=rplGetIdentLength(nameptr);    // LENGTH IN BYTES
                                len=utf8nlen((char *)(nameptr+1),(char *)(nameptr+1)+idlen);  // LENGTH IN UNICODE CHARACTERS
                                if((len>=(BINT)TokenLen) && (!utf8ncmp2((char *)TokenStart,(char *)BlankStart,(char *)(nameptr+1),TokenLen))) {
                                    // WE HAVE A MATCH!
                                    // CREATE A NEW LIBPTR AND RETURN IT
                                    WORDPTR newobj=rplAllocTempOb(2);
                                    if(!newobj) { RetNum=ERR_NOTMINE; return; }

                                    newobj[0]=MKPROLOG(DOLIBPTR,2);
                                    newobj[1]=direntry[0][1];
                                    newobj[2]=previdx;


                                    RetNum=OK_CONTINUE;
                                    SuggestedObject=newobj;
                                    SuggestedOpcode=newobj[0];
                                    return;
                                }
                                BINT firstchar=utf82cp((char *)(nameptr+1),(char *)(nameptr+1)+idlen);

                                // CHECK FOR NON-STANDARD STARTING CHARACTERS
                                if( !(
                                            ((firstchar>='A') && (firstchar<='Z')) ||
                                            ((firstchar>='a') && (firstchar<='z')) )
                                        )
                                {
                                // SKIP THE FIRST CHARACTER AND CHECK AGAIN
                                    --len;
                                    if((len>=(BINT)TokenLen) && (!utf8ncmp2((char *)TokenStart,(char *)BlankStart,utf8skipst((char *)(nameptr+1),(char *)(nameptr+1)+4),TokenLen)))
                                    {
                                        // WE HAVE A MATCH!
                                        // CREATE A NEW LIBPTR AND RETURN IT
                                        WORDPTR newobj=rplAllocTempOb(2);
                                        if(!newobj) { RetNum=ERR_NOTMINE; return; }

                                        newobj[0]=MKPROLOG(DOLIBPTR,2);
                                        newobj[1]=direntry[0][1];
                                        newobj[2]=previdx;


                                        RetNum=OK_CONTINUE;
                                        SuggestedObject=newobj;
                                        SuggestedOpcode=newobj[0];
                                        return;
                                    }

                                }
                                // THERE WAS NO MATCH, DO THE NEXT NAME
                        }
                    } while(previdx>4);

                    // NO MATCHES ON THIS LIBRARY

                }

            } while((direntry=rplFindNext(direntry)));

            // NO MATCHES ON ANY LIBRARY

        }

        }

        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        return;
        }

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg (LOW WORD)
        // AND IN ArgNum2 (HI WORD)
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(LIBNUM(MenuCodeArg)==DOLIBRARY) {
            ObjectPTR=(WORDPTR)lib102_menu;
            RetNum=OK_CONTINUE;
            return;
        }
        WORDPTR libmenu=rplGetLibPtr2(ArgNum2,2);

        if(!libmenu) { RetNum=ERR_NOTMINE; return; }

        ObjectPTR=libmenu;
        RetNum=OK_CONTINUE;
       return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // IF THE OPCODE IS FROM A USER LIBRARY, LIBID IS IN ArgNum2
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(LIBNUM(CmdHelp)==DOLIBPTR) {
            // RETURN THE HELP FOR THAT COMMAND
            WORDPTR help=rplGetLibPtrHelp(ArgNum2,OPCODE(CmdHelp));
            if(!help) RetNum=ERR_NOTMINE;
            else {
                RetNum=OK_CONTINUE;
                ObjectPTR=help;
            }
            return;
        }
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

