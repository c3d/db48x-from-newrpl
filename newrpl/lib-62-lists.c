/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
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
#define LIBRARY_NUMBER  62

#define ERROR_LIST \
    ERR(LISTEXPECTED,0), \
    ERR(INDEXOUTOFBOUNDS,1), \
    ERR(EMPTYLIST,2), \
    ERR(INVALIDLISTSIZE,3)

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    ECMD(ENDLIST,"}",MKTOKENINFO(1,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TOLIST,"→LIST",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(INNERCOMP,"LIST→",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(CMDDOLIST,"DOLIST",MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DOLISTPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DOLISTPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DOLISTERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    CMD(DOSUBS,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DOSUBSPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DOSUBSPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DOSUBSERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    CMD(MAP,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(MAPPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(MAPPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(MAPERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(MAPINNERCOMP,"MAPLIST→",MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    ECMD(EVALPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(EVALPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(EVALERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    CMD(STREAM,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STREAMPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STREAMPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STREAMERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(UNARYPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(UNARYPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(UNARYERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(BINARYPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(BINARYPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(BINARYERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TESTPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TESTPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TESTERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DELTALIST,"ΔLIST",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(SUMLIST,"ΣLIST",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(PRODLIST,"ΠLIST",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(OPLISTPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(OPLISTPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(OPLISTERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DELTAPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DELTAPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DELTAERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    CMD(PUT,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(PUTI,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(GET,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(GETI,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(HEAD,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(TAIL,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(ADD,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(SORT,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(REVLIST,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(ADDROT,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2))
// ADD MORE OPCODES HERE

#define ERROR_LIST \
ERR(LISTEXPECTED,0), \
ERR(INDEXOUTOFBOUNDS,1), \
ERR(EMPTYLIST,2), \
ERR(INVALIDLISTSIZE,3)

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER, LIBRARY_NUMBER+1


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS


// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************


INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib62_menu);


ROMOBJECT dolist_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,DOLISTPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,DOLISTPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,DOLISTERR),     // ERROR HANDLER
    CMD_SEMI
};

ROMOBJECT dosubs_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,DOSUBSPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,DOSUBSPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,DOSUBSERR),     // ERROR HANDLER
    CMD_SEMI
};

ROMOBJECT map_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,MAPPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,MAPPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,MAPERR),     // ERROR HANDLER
    CMD_SEMI
};

ROMOBJECT listeval_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,EVALPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,EVALPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,EVALERR),     // ERROR HANDLER
    CMD_SEMI
};

ROMOBJECT stream_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,STREAMPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,STREAMPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,STREAMERR),     // ERROR HANDLER
    CMD_SEMI
};


ROMOBJECT nsub_name[]={
    MKPROLOG(DOIDENT,1),
    (WORD)('N' | ('S'<<8) | ('U'<<16) | ('B'<<24))
};

ROMOBJECT endsub_name[]={
    MKPROLOG(DOIDENT,2),
    (WORD)('E' | ('N'<<8) | ('D'<<16) | ('S'<<24)),
    (WORD)('U' | ('B'<<8))

};

ROMOBJECT unary_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,UNARYPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,UNARYPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,UNARYERR),     // ERROR HANDLER
    CMD_SEMI
};

ROMOBJECT binary_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,BINARYPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,BINARYPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,BINARYERR),     // ERROR HANDLER
    CMD_SEMI
};

ROMOBJECT testop_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,TESTPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,TESTPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,TESTERR),     // ERROR HANDLER
    CMD_SEMI
};



ROMOBJECT oplist_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,OPLISTPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,OPLISTPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,OPLISTERR),     // ERROR HANDLER
    CMD_SEMI
};

ROMOBJECT deltalist_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,DELTAPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,DELTAPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,DELTAERR),     // ERROR HANDLER
    CMD_SEMI
};


ROMOBJECT empty_list[]={
    MKPROLOG(DOLIST,1),
    MKOPCODE(DOLIST,ENDLIST)
};

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib62_menu,
    (WORDPTR)dolist_seco,
    (WORDPTR)dosubs_seco,
    (WORDPTR)map_seco,
    (WORDPTR)stream_seco,
    (WORDPTR)nsub_name,
    (WORDPTR)endsub_name,
    (WORDPTR)unary_seco,
    (WORDPTR)binary_seco,
    (WORDPTR)testop_seco,
    (WORDPTR)oplist_seco,
    (WORDPTR)deltalist_seco,
    (WORDPTR)empty_list,

    0
};


// COMPARE TWO ITEMS WITHIN A LIST, BY CALLING THE OPERATOR CMP
// OPERATOR CMP MUST RETURN -1, 0 OR 1 IF B>A, B==A, OR A>B RESPECTIVELY

BINT rplListItemCompare(WORDPTR a,WORDPTR b)
{

    rplPushData(a);
    rplPushData(b);
    rplCallOvrOperator((CMD_OVR_CMP));
    if(Exceptions) return 0;
    BINT r=rplReadBINT(rplPopData());
    if(r==0) return (BINT)(a-b);
    return r;

}












void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        rplPushData(IPtr);
        return;
    }


    if(ISUNARYOP(CurOpcode)) {
        // ALL UNARY OPERATORS PASS THEIR OPERATION DIRECTLY TO EACH ELEMENT
        if(!ISPROLOG(*rplPeekData(1))) {
            // COMMAND AS ARGUMENT
            if( (OPCODE(CurOpcode)==OVR_EVAL)||
                    (OPCODE(CurOpcode)==OVR_EVAL1)||
                    (OPCODE(CurOpcode)==OVR_XEQ) )
            {

                WORD saveOpcode=CurOpcode;
                CurOpcode=*rplPopData();
                // RECURSIVE CALL
                LIB_HANDLER();
                CurOpcode=saveOpcode;
                return;
            }
            else {
                rplError(ERR_INVALIDOPCODE);
                return;
            }
        }

        if(OPCODE(CurOpcode)==OVR_XEQ) return;  // JUST LEAVE THE LIST ON THE STACK

        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISLIST(*rplPeekData(1))) {
            rplError(ERR_LISTEXPECTED);
            return;
        }

        // NOW CREATE A PROGRAM TO 'MAP'

        WORDPTR program=rplAllocTempOb(2);
        if(!program) {
            return;
        }

        program[0]=MKPROLOG(DOCOL,2);
        program[1]=CurOpcode;
        program[2]=CMD_SEMI;

        // HERE WE HAVE program = PROGRAM TO EXECUTE

        // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
        rplCreateLAMEnvironment(IPtr);

        rplCreateLAM((WORDPTR)nulllam_ident,program);     // LAM 1 = ROUTINE TO EXECUTE ON EVERY STEP
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1)+1);     // LAM 2 = NEXT ELEMENT TO BE PROCESSED
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));     // LAM 3 = LIST
        if(Exceptions) { rplCleanupLAMs(0); return; }


        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST

        rplPushRet(IPtr);
        if(OPCODE(CurOpcode)==OVR_EVAL) IPtr=(WORDPTR) listeval_seco;
        else IPtr=(WORDPTR) unary_seco;
        CurOpcode=CMD_OVR_EVAL;   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

        rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD USER PROGRAM

        return;

    }

    if(ISBINARYOP(CurOpcode)) {
        // ALL BINARY OPERATORS PASS THEIR OPERATIONS DIRECTLY TO EACH ELEMENT

        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if((!ISLIST(*rplPeekData(1)))  && (!ISLIST(*rplPeekData(2)))) {
            rplError(ERR_LISTEXPECTED);
            return;
        }



        // NOW CREATE A PROGRAM TO 'MAP'

        WORDPTR program=rplAllocTempOb(2);
        if(!program) {
            return;
        }

        program[0]=MKPROLOG(DOCOL,2);
        program[1]=CurOpcode;
        program[2]=CMD_SEMI;

        // HERE WE HAVE program = PROGRAM TO EXECUTE

        // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
        rplCreateLAMEnvironment(IPtr);

        rplCreateLAM((WORDPTR)nulllam_ident,program);     // LAM 1 = ROUTINE TO EXECUTE ON EVERY STEP
        if(Exceptions) { rplCleanupLAMs(0); return; }

        if(ISLIST(*rplPeekData(2))) rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(2)+1);     // LAM 2 = NEXT ELEMENT TO BE PROCESSED ON LIST1
        else rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(2));
        if(Exceptions) { rplCleanupLAMs(0); return; }

        if(ISLIST(*rplPeekData(1))) rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1)+1);     // LAM 3 = NEXT ELEMENT TO BE PROCESSED ON LIST2
        else rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(2));     // LAM 4 = LIST1
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));     // LAM 5 = LIST2
        if(Exceptions) { rplCleanupLAMs(0); return; }


        // HERE GETLAM1 = PROGRAM, GETLAM 2 and 3 = NEXT OBJECT ON EACH LIST, GETLAM 4 AND 5 = LISTS

        rplPushRet(IPtr);

        if(ISTESTOP(CurOpcode)) IPtr=(WORDPTR) testop_seco;
        else IPtr=(WORDPTR) binary_seco;
        CurOpcode=CMD_OVR_ADD;   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

        rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD USER PROGRAM

        return;

    }

    switch(OPCODE(CurOpcode))
    {
    case PUT:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR list=rplPeekData(3);
        WORDPTR *var=0;
        if(ISIDENT(*list)) {
            var=rplFindLAM(list,1);
            if(!var) {
                var=rplFindGlobal(list,1);
                if(!var) {
                    rplError(ERR_LISTEXPECTED);
                    return;
                }

            }
            list=*(var+1);
        }
        if(!ISLIST(*list)) {
            rplError(ERR_LISTEXPECTED);
            return;
        }

        if(!ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        BINT nitems=rplExplodeList(list);
        BINT position=rplReadNumberAsBINT(rplPeekData(nitems+3));
        if(Exceptions) return;

        if(position<1 || position>nitems) {
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }
        rplOverwriteData(nitems+2-position,rplPeekData(nitems+2));

        rplCreateList();

        rplOverwriteData(4,rplPeekData(1));
        rplDropData(3);

        if(var) {
            *(var+1)=rplPopData();
        }

    }
        return;
    case PUTI:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        // HERE TEH STACK HAS: LIST POSITION NEWOBJECT
        WORDPTR list=rplPeekData(3);
        WORDPTR *var=0;
        if(ISIDENT(*list)) {
            var=rplFindLAM(list,1);
            if(!var) {
                var=rplFindGlobal(list,1);
                if(!var) {
                    rplError(ERR_LISTEXPECTED);
                    return;
                }

            }
            list=*(var+1);
        }
        if(!ISLIST(*list)) {
            rplError(ERR_LISTEXPECTED);
            return;
        }

        if(!ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_REALEXPECTED);
            return;
        }


        BINT nitems=rplExplodeList(list);

        // HERE THE STACK IS: LIST POSITION NEWOBJECT OBJ1 OBJ2 ... OBJN N

        BINT position=rplReadNumberAsBINT(rplPeekData(nitems+3));
        if(Exceptions) return;

        if(position<1 || position>nitems) {
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }
        rplOverwriteData(nitems+2-position,rplPeekData(nitems+2));

        rplCreateList();

        // HERE THE STACK IS: LIST POSITION NEWOBJECT NEWLIST

        rplOverwriteData(4,rplPeekData(1));
        rplDropData(3);

        if(var) {
            *(var+1)=rplPeekData(1);
        }

        rplNewBINTPush(position+1,DECBINT);

    }

        return;

    case GET:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR list=rplPeekData(2);
        WORDPTR *var=0;
        if(ISIDENT(*list)) {
            var=rplFindLAM(list,1);
            if(!var) {
                var=rplFindGlobal(list,1);
                if(!var) {
                    rplError(ERR_LISTEXPECTED);

                    return;
                }

            }
            list=*(var+1);
        }
        if(!ISLIST(*list)) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        BINT nitems=rplExplodeList(list);
        BINT position=rplReadNumberAsBINT(rplPeekData(nitems+2));
        if(Exceptions) return;
        if(position<1 || position>nitems) {
            rplDropData(nitems+1);
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }
        rplOverwriteData(nitems+3,rplPeekData(nitems+2-position));
        rplDropData(nitems+2);

    }
        return;



    case GETI:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR list=rplPeekData(2);
        WORDPTR *var=0;
        if(ISIDENT(*list)) {
            var=rplFindLAM(list,1);
            if(!var) {
                var=rplFindGlobal(list,1);
                if(!var) {
                    rplError(ERR_LISTEXPECTED);

                    return;
                }

            }
            list=*(var+1);
        }
        if(!ISLIST(*list)) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);

            return;
        }

        BINT nitems=rplExplodeList(list);
        BINT position=rplReadNumberAsBINT(rplPeekData(nitems+2));
        if(Exceptions) return;
        if(position<1 || position>nitems) {
            rplDropData(nitems+1);
            rplError(ERR_INDEXOUTOFBOUNDS);

            return;
        }

        // HERE THE STACK IS: LIST POSITION OBJ1 ... OBJN N

        rplOverwriteData(nitems+1,rplPeekData(nitems+2-position));
        rplDropData(nitems);

        rplNewBINTPush(position+1,DECBINT);
        rplOverwriteData(2,rplPopData());
    }
        return;


    case HEAD:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR list=rplPeekData(1);

        if(!ISLIST(*list)) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        BINT nitems=rplExplodeList(list);
        if(Exceptions) return;
        if(nitems>0) {
            rplOverwriteData(nitems+2,rplPeekData(nitems+1));
            rplDropData(nitems+1);
        }
        else {
            rplDropData(1);
            rplError(ERR_EMPTYLIST);
            return;
        }


        return;
    }

    case TAIL:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR list=rplPeekData(1);

        if(!ISLIST(*list)) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        BINT nitems=rplExplodeList(list);
        if(Exceptions) return;

        rplDropData(1);
        rplNewBINTPush(nitems-1,DECBINT);

        rplCreateList();
        if(Exceptions) return;
        // HERE THE STACK HAS: LIST OBJ1 NEWLIST
        rplOverwriteData(3,rplPeekData(1));
        rplDropData(2);

        return;
    }

        return;

    case SORT:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR list=rplPeekData(1);

        if(!ISLIST(*list)) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        BINT nitems=rplListLength(list);

        if(nitems<2) return;

        rplDropData(1);

        rplExplodeList(list);
        if(Exceptions) return;


        // PERFORM BINARY INSERTION SORT

        WORDPTR *ptr,*ptr2,*endlimit,*startlimit,save;
        WORDPTR *left,*right;

        startlimit=DSTop-nitems;    // POINT TO SECOND ELEMENT IN THE LIST
        endlimit=DSTop-1;           // POINT AFTER THE LAST ELEMENT

        for(ptr=startlimit;ptr<endlimit;++ptr)
        {
            save=*ptr;

            left=startlimit-1;
            right=ptr-1;
            if(rplListItemCompare(*right,save)>0) {
               if(rplListItemCompare(save,*left)>0) {
            while(right-left>1) {
                if(rplListItemCompare(*(left+(right-left)/2),save)>0) {
                    right=left+(right-left)/2;
                }
                else {
                    left=left+(right-left)/2;
                }
            }
               } else right=left;
            // INSERT THE POINTER RIGHT BEFORE right
            for(ptr2=ptr;ptr2>right; ptr2-=1 ) *ptr2=*(ptr2-1);
            //memmoveb(right+1,right,(ptr-right)*sizeof(WORDPTR));
            *right=save;
            }
        }

        rplCreateList();
        return;
    }

    case REVLIST:
     {
        // CHECK ARGUMENTS
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR list=rplPeekData(1);

        if(!ISLIST(*list)) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        BINT nitems=rplListLength(list);

        if(nitems<2) return;

        rplDropData(1);

        rplExplodeList(list);
        if(Exceptions) return;


        // REVERSE ALL ELEMENTS IN THE LIST

        WORDPTR *endlimit,*startlimit,save;

        startlimit=DSTop-nitems-1;    // POINT TO FIRST ELEMENT IN THE LIST
        endlimit=DSTop-2;           // POINT TO THE LAST ELEMENT

        while(endlimit>startlimit) {
            save=*endlimit;
            *endlimit=*startlimit;
            *startlimit=save;
            ++startlimit;
            --endlimit;
        }

        rplCreateList();
        return;
    }

    case ENDLIST:
        return;
    case TOLIST:
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);

            return;
        }

        rplCreateList();

        return;
    case INNERCOMP:
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISLIST(*rplPeekData(1))) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        rplExplodeList(rplPopData());

        return;

        // **********************************************************
        // THE COMMANDS THAT FOLLOW ALL WORK TOGETHER TO IMPLEMENT DOLIST

    case CMDDOLIST:
    {
        BINT initdepth=rplDepthData();
        if(initdepth<3) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        if(!ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_REALEXPECTED);

            return;
        }

        // GET THE NUMBER OF LISTS

        BINT64 nlists=rplReadNumberAsBINT(rplPeekData(2));

        if(initdepth<2+nlists) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        WORDPTR program=rplPeekData(1);
        if(ISIDENT(*program)) {
            WORDPTR *var=rplFindLAM(program,1);
            if(!var) {
                var=rplFindGlobal(program,1);
                if(!var) {
                    rplError(ERR_LISTEXPECTED);

                    return;
                }
            }
            // HERE var HAS THE VARIABLE, GET THE CONTENTS
            program=*(var+1);
        }

        if(!ISPROGRAM(*program)) {
            rplError(ERR_PROGRAMEXPECTED);

            return;
        }

        // HERE WE HAVE program = PROGRAM TO EXECUTE, nlists = NUMBER OF LISTS

        // CHECK THAT ALL LISTS ARE ACTUALLY LISTS

        BINT f,l,length=-1;
        for(f=3;f<3+nlists;++f) {
            if(!ISLIST(*rplPeekData(f))) {
                rplError(ERR_LISTEXPECTED);
                return;
            }
            // MAKE SURE ALL LISTS ARE EQUAL LENGTH
            l=rplListLength(rplPeekData(f));
            if(length<0) length=l;
            else if(l!=length) {
                rplError(ERR_INVALIDLISTSIZE);
                return;
            }
        }

        if(length<1) {
            rplError(ERR_INVALIDLISTSIZE);
            return;
        }

        // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
        rplCreateLAMEnvironment(IPtr);
        // NOW CREATE A LOCAL VARIABLE FOR THE INDEX

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));     // LAM 1 = ROUTINE TO EXECUTE ON EVERY STEP

        WORDPTR newb=rplNewBINT(nlists,DECBINT);
        if(!newb) {
            rplCleanupLAMs(0);
            return;
        }

        rplCreateLAM((WORDPTR)nulllam_ident,newb);     // LAM 2 = BINT WITH NUMBER OF LISTS

        if(Exceptions) { rplCleanupLAMs(0); return; }

        newb=rplNewBINT(length,DECBINT);
        if(!newb) {
            rplCleanupLAMs(0);
            return;
        }

        rplCreateLAM((WORDPTR)nulllam_ident,newb);     // LAM 3 = BINT WITH NUMBER OF ITEMS PER LIST

        if(Exceptions) { rplCleanupLAMs(0); return; }

        newb=rplNewBINT(1,DECBINT);
        if(!newb) {
            rplCleanupLAMs(0);
            return;
        }

        rplCreateLAM((WORDPTR)nulllam_ident,newb);     // LAM 4 = 1, BINT WITH CURRENT INDEX IN TEH LOOP


        for(f=0;f<nlists;++f) {
            rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(2+nlists-f));     // LAM n+4 = LISTS IN REVERSE ORDER
            if(Exceptions) { rplCleanupLAMs(0); return; }
        }

        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NLISTS, GETLAM3 = LENGTH, GETLAM4 = INDEX, GETLAM 5 .. 4+N = LISTS IN REVERSE ORDER
        // nlists = NUMBER OF LISTS, length = NUMBER OF ARGUMENTS TO PROCESS


        // THIS NEEDS TO BE DONE IN 3 STEPS:
        // DOLIST WILL PREPARE THE LAMS FOR OPEN EXECUTION
        // DOLIST.PROCESS WILL PUSH THE LIST ELEMENTS AND EVAL THE PROGRAM
        // DOLIST.POSTPROCESS WILL CHECK IF ALL ELEMENTS WERE PROCESSED WITHOUT ERRORS, PACK THE LIST AND END
        //                    OR IT WILL EXECUTE DOLIST.PROCESS ONCE MORE

        // THE INITIAL CODE FOR DOLIST MUST TRANSFER FLOW CONTROL TO A
        // SECONDARY THAT CONTAINS :: DOLIST.PROCESS EVAL DOLIST.POSTPROCESS ;
        // DOLIST.POSTPROCESS WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
        // IN ORDER TO KEEP THE LOOP RUNNING

        rplPushRet(IPtr);
        IPtr=(WORDPTR) dolist_seco;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,CMDDOLIST);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

        rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD USER PROGRAM

        return;
    }

    case DOLISTPRE:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NLISTS, GETLAM3 = LENGTH, GETLAM4 = INDEX, GETLAM 5 .. 4+N = LISTS IN REVERSE ORDER
        // nlists = NUMBER OF LISTS, length = NUMBER OF ARGUMENTS TO PROCESS


        BINT64 nlists=rplReadBINT(*rplGetLAMn(2));
        BINT64 idx=rplReadBINT(*rplGetLAMn(4));
        BINT k;

        for(k=0;k<nlists;++k)
        {
            rplPushData(rplGetListElement(*rplGetLAMn(k+5),idx));
            if(Exceptions) { DSTop=rplUnprotectData(); rplCleanupLAMs(0); IPtr=rplPopRet(); CurOpcode=MKOPCODE(LIBRARY_NUMBER,CMDDOLIST); return; }
        }

        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE DOLISTERR WORD

        // NOW RECALL THE PROGRAM TO THE STACK

        rplPushData(*rplGetLAMn(1));

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case DOLISTPOST:
    {

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR IN THE USER PROGRAM

        BINT64 length=rplReadBINT(*rplGetLAMn(3));
        BINT64 nlists=rplReadBINT(*rplGetLAMn(2));
        BINT64 idx=rplReadBINT(*rplGetLAMn(4));

        if(idx<length) {
            // NEED TO DO ONE MORE LOOP
            ++idx;
            WORDPTR newbint=rplNewBINT(idx,DECBINT);
            if(Exceptions) {
                DSTop=rplUnprotectData();   // CLEANUP ALL INTERMEDIATE RESULTS
                rplCleanupLAMs(0);
                IPtr=rplPopRet();
                CurOpcode=MKOPCODE(LIBRARY_NUMBER,CMDDOLIST);
                return;
            }
            rplPutLAMn(4,newbint);  // STORE NEW INDEX

            IPtr=(WORDPTR) dolist_seco;   // CONTINUE THE LOOP
            // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
            return;
        }

        // ALL ELEMENTS WERE PROCESSED
        // FORM A LIST WITH ALL THE NEW ELEMENTS

        WORDPTR *prevDStk = rplUnprotectData();

        BINT newdepth=(BINT)(DSTop-prevDStk);

        rplNewBINTPush(newdepth,DECBINT);
        if(Exceptions) {
            DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
            rplCleanupLAMs(0);
            IPtr=rplPopRet();
            CurOpcode=MKOPCODE(LIBRARY_NUMBER,CMDDOLIST);
            return;
        }

        rplCreateList();
        if(Exceptions) {
            DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
            rplCleanupLAMs(0);
            IPtr=rplPopRet();
            CurOpcode=MKOPCODE(LIBRARY_NUMBER,CMDDOLIST);
            return;
        }
        // HERE THE STACK HAS: LIST1... LISTN N PROGRAM NEWLIST
        rplOverwriteData(nlists+3,rplPeekData(1));
        rplDropData(nlists+2);

        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,CMDDOLIST);
        return;
    }

    case DOLISTERR:
        // SAME PROCEDURE AS ENDERR
        rplRemoveExceptionHandler();
        rplPopRet();
        rplUnprotectData();
        rplRemoveExceptionHandler();

        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;
        ExceptionPointer=IPtr;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,CMDDOLIST);
        return;

        // **********************************************************
        // THE COMMANDS THAT FOLLOW ALL WORK TOGETHER TO IMPLEMENT DOSUBS

    case DOSUBS:
    {
        BINT initdepth=rplDepthData();
        if(initdepth<3) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        if(!ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_REALEXPECTED);

            return;
        }

        if(!ISLIST(*rplPeekData(3))) {
            rplError(ERR_LISTEXPECTED);

            return;
        }


        // GET THE NUMBER OF VALUES WE NEED TO USE IN EACH ITERATION


        BINT64 nvalues=rplReadNumberAsBINT(rplPeekData(2));

        WORDPTR program=rplPeekData(1);
        if(ISIDENT(*program)) {
            WORDPTR *var=rplFindLAM(program,1);
            if(!var) {
                var=rplFindGlobal(program,1);
                if(!var) {
                    rplError(ERR_PROGRAMEXPECTED);

                    return;
                }
            }
            // HERE var HAS THE VARIABLE, GET THE CONTENTS
            program=*(var+1);
        }


        if(!ISPROGRAM(*program)) {
            rplError(ERR_PROGRAMEXPECTED);
            return;
        }

        // HERE WE HAVE program = PROGRAM TO EXECUTE, nvalues = NUMBER OF VALUES TO USE EACH

        // CHECK THAT THE LIST ARE ACTUALLY LISTS

        BINT length,maxpos;
        length=rplListLength(rplPeekData(3));

        if(length<nvalues) {
            rplError(ERR_INVALIDLISTSIZE);
            return;
        }

        // POSITION OF THE LAST ITEM TO PROCESS
        maxpos=1+length-nvalues;

        // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
        rplCreateLAMEnvironment(IPtr);

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));     // LAM 1 = ROUTINE TO EXECUTE ON EVERY STEP

        WORDPTR newb=rplNewBINT(nvalues,DECBINT);
        if(!newb) {
            rplCleanupLAMs(0);
            return;
        }

        rplCreateLAM((WORDPTR)nulllam_ident,newb);     // LAM 2 = NUMBER OF ARGUMENTS TO PROCESS

        if(Exceptions) { rplCleanupLAMs(0); return; }

        newb=rplNewBINT(maxpos,DECBINT);
        if(!newb) {
            rplCleanupLAMs(0);
            return;
        }

        rplCreateLAM((WORDPTR)endsub_name,newb);     // LAM 3 = BINT WITH NUMBER OF TIMES TO RUN THE LOOP = ENDSUB

        if(Exceptions) { rplCleanupLAMs(0); return; }

        newb=rplNewBINT(1,DECBINT);
        if(!newb) {
            rplCleanupLAMs(0);
            return;
        }

        rplCreateLAM((WORDPTR)nsub_name,newb);     // LAM 4 = 1, BINT WITH CURRENT INDEX IN THE LOOP


        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(3));     // LAM 5 = LIST
        if(Exceptions) { rplCleanupLAMs(0); return; }

        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NVALUES, GETLAM3 = ENDSUB, GETLAM4 = NSUB, GETLAM 5 = LIST

        // THIS NEEDS TO BE DONE IN 3 STEPS:
        // DOSUBS WILL PREPARE THE LAMS FOR OPEN EXECUTION
        // DOSUBSPRE WILL PUSH THE LIST ELEMENTS AND EVAL THE PROGRAM
        // DOSUBSPOST WILL CHECK IF ALL ELEMENTS WERE PROCESSED WITHOUT ERRORS, PACK THE LIST AND END
        //                    OR IT WILL EXECUTE DOSUBSPRE ONCE MORE

        // THE INITIAL CODE FOR DOSUBS MUST TRANSFER FLOW CONTROL TO A
        // SECONDARY THAT CONTAINS :: DOSUBSPRE EVAL DOSUBSPOST ;
        // DOSUBSPOST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
        // IN ORDER TO KEEP THE LOOP RUNNING

        rplPushRet(IPtr);
        IPtr=(WORDPTR) dosubs_seco;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,DOSUBS);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

        rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD USER PROGRAM

        return;
    }

    case DOSUBSPRE:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NVALUES, GETLAM3 = ENDSUB, GETLAM4 = NSUB, GETLAM 5 = LIST


        BINT64 nvalues=rplReadBINT(*rplGetLAMn(2));
        BINT64 idx=rplReadBINT(*rplGetLAMn(4));
        BINT k;
        for(k=0;k<nvalues;++k)
        {
            rplPushData(rplGetListElement(*rplGetLAMn(5),idx+k));
            if(Exceptions) { DSTop=rplUnprotectData(); rplCleanupLAMs(0); IPtr=rplPopRet(); CurOpcode=MKOPCODE(LIBRARY_NUMBER,DOSUBS); return; }
        }

        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE DOSUBSERR WORD

        // NOW RECALL THE PROGRAM TO THE STACK

        rplPushData(*rplGetLAMn(1));

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case DOSUBSPOST:
    {

        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NVALUES, GETLAM3 = ENDSUB, GETLAM4 = NSUB, GETLAM 5 = LIST

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR IN THE USER PROGRAM

        BINT64 endsub=rplReadBINT(*rplGetLAMn(3));
        BINT64 idx=rplReadBINT(*rplGetLAMn(4));

        if(idx<endsub) {
            // NEED TO DO ONE MORE LOOP
            ++idx;
            WORDPTR newbint=rplNewBINT(idx,DECBINT);
            if(Exceptions) {
                DSTop=rplUnprotectData();   // CLEANUP ALL INTERMEDIATE RESULTS
                rplCleanupLAMs(0);
                IPtr=rplPopRet();
                CurOpcode=MKOPCODE(LIBRARY_NUMBER,DOSUBS);
                return;
            }
            rplPutLAMn(4,newbint);  // STORE NEW INDEX

            IPtr=(WORDPTR) dosubs_seco;   // CONTINUE THE LOOP
            // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
            return;
        }

        // ALL ELEMENTS WERE PROCESSED
        // FORM A LIST WITH ALL THE NEW ELEMENTS

        WORDPTR *prevDStk = rplUnprotectData();

        BINT newdepth=(BINT)(DSTop-prevDStk);

        rplNewBINTPush(newdepth,DECBINT);
        if(Exceptions) {
            DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
            rplCleanupLAMs(0);
            IPtr=rplPopRet();
            CurOpcode=MKOPCODE(LIBRARY_NUMBER,DOSUBS);
            return;
        }

        rplCreateList();
        if(Exceptions) {
            DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
            rplCleanupLAMs(0);
            IPtr=rplPopRet();
            CurOpcode=MKOPCODE(LIBRARY_NUMBER,DOSUBS);
            return;
        }
        // HERE THE STACK HAS: LIST1 N PROGRAM NEWLIST
        rplOverwriteData(4,rplPeekData(1));
        rplDropData(3);

        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,DOSUBS);
        return;
    }

    case DOSUBSERR:
        // SAME PROCEDURE AS ENDERR
        rplRemoveExceptionHandler();
        rplPopRet();
        rplUnprotectData();
        rplRemoveExceptionHandler();

        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;

        ExceptionPointer=IPtr;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,DOSUBS);
        return;


        // END OF DOSUBS
        // *****************************************************************


        // **********************************************************
        // THE COMMANDS THAT FOLLOW ALL WORK TOGETHER TO IMPLEMENT MAP

    case MAP:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        if(!ISLIST(*rplPeekData(2))) {
            rplError(ERR_LISTEXPECTED);

            return;
        }


        WORDPTR program=rplPeekData(1);
        if(ISIDENT(*program)) {
            WORDPTR *var=rplFindLAM(program,1);
            if(!var) {
                var=rplFindGlobal(program,1);
                if(!var) {
                    rplError(ERR_PROGRAMEXPECTED);

                    return;
                }
            }
            // HERE var HAS THE VARIABLE, GET THE CONTENTS
            program=*(var+1);
        }


        if(!ISPROGRAM(*program)) {
            rplError(ERR_PROGRAMEXPECTED);

            return;
        }

        // HERE WE HAVE program = PROGRAM TO EXECUTE

        // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
        rplCreateLAMEnvironment(IPtr);

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));     // LAM 1 = ROUTINE TO EXECUTE ON EVERY STEP
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(2)+1);     // LAM 2 = NEXT ELEMENT TO BE PROCESSED
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(2));     // LAM 3 = LIST
        if(Exceptions) { rplCleanupLAMs(0); return; }

        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST

        // THIS NEEDS TO BE DONE IN 3 STEPS:
        // MAP WILL PREPARE THE LAMS FOR OPEN EXECUTION
        // MAPPRE WILL PUSH THE LIST ELEMENTS AND EVAL THE PROGRAM
        // MAPPOST WILL CHECK IF ALL ELEMENTS WERE PROCESSED WITHOUT ERRORS, PACK THE LIST AND END
        //                    OR IT WILL EXECUTE MAPPRE ONCE MORE

        // THE INITIAL CODE FOR MAP MUST TRANSFER FLOW CONTROL TO A
        // SECONDARY THAT CONTAINS :: MAPPRE EVAL MAPPOST ;
        // MAPPOST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
        // IN ORDER TO KEEP THE LOOP RUNNING

        rplPushRet(IPtr);
        IPtr=(WORDPTR) map_seco;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

        rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD USER PROGRAM

        return;
    }

    case MAPPRE:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST


        WORDPTR nextobj=*rplGetLAMn(2);
        WORDPTR startobj;
        // EMPTY LISTS NEED TO BE HANDLED HERE (NO EVAL NEEDED)
        WORDPTR endmarker=rplSkipOb(*rplGetLAMn(3))-1;

        do {

            startobj=nextobj;

            while(ISLIST(*nextobj)) {
                // GET INSIDE THE LIST
                ++nextobj;
                // LEAVE A MARKER ON THE STACK. USE THE SECO OBJECT AS A MARKER TO SAVE STORAGE
                rplPushData((WORDPTR)map_seco);
            }

            while(*nextobj==MKOPCODE(LIBRARY_NUMBER,ENDLIST)) {
                if(nextobj==endmarker) {
                    // CLOSE THE MAIN LIST AND RETURN
                    WORDPTR *prevDStk = rplUnprotectData();

                    BINT newdepth=(BINT)(DSTop-prevDStk);

                    rplNewBINTPush(newdepth,DECBINT);
                    if(Exceptions) {
                        DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP);
                        return;
                    }

                    rplCreateList();
                    if(Exceptions) {
                        DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP);
                        return;
                    }

                    rplOverwriteData(3,rplPeekData(1));
                    rplDropData(2);

                    rplCleanupLAMs(0);
                    IPtr=rplPopRet();
                    CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP);
                    return;


                }
                else {
                    // CLOSE AN INNER LIST AND CONTINUE

                    WORDPTR *stkptr=DSTop-1;

                    while(*stkptr!=map_seco) --stkptr;  // FIND THE NEXT MARKER ON THE STACK
                    BINT nelements=(BINT)(DSTop-stkptr)-1;

                    rplNewBINTPush(nelements,DECBINT);
                    if(Exceptions) {
                        DSTop=rplUnprotectData();   // CLEANUP ALL INTERMEDIATE RESULTS
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP);
                        return;
                    }

                    rplCreateList();
                    if(Exceptions) {
                        DSTop=rplUnprotectData();   // CLEANUP ALL INTERMEDIATE RESULTS
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP);
                        return;
                    }

                    // NOW REMOVE THE MARKER FROM THE STACK
                    rplOverwriteData(2,rplPeekData(1));
                    rplDropData(1);


                }

                ++nextobj;

            }

        } while(nextobj!=startobj); // WE EXIT THE WHILE ONLY WHEN nextobj DIDN'T CHANGE WITHIN THE LOOP


        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE MAPERR WORD


        rplPutLAMn(2,nextobj);

        // PUSH THE NEXT OBJECT IN THE STACK
        rplPushData(nextobj);

        // NOW RECALL THE PROGRAM TO THE STACK

        rplPushData(*rplGetLAMn(1));

        if(Exceptions) { DSTop=rplUnprotectData(); rplCleanupLAMs(0); IPtr=rplPopRet(); CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP); return; }

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case MAPPOST:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR IN THE USER PROGRAM

        rplPutLAMn(2,rplSkipOb(*rplGetLAMn(2)));    // MOVE TO THE NEXT OBJECT IN THE LIST

        IPtr=(WORDPTR) map_seco;   // CONTINUE THE LOOP
        // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
        return;
    }
    case MAPERR:
        // SAME PROCEDURE AS ENDERR
        rplRemoveExceptionHandler();
        rplPopRet();
        rplUnprotectData();
        rplRemoveExceptionHandler();

        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;
        ExceptionPointer=IPtr;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP);
        return;


        // END OF MAP
        // *****************************************************************

        // **********************************************************
        // THE COMMANDS THAT FOLLOW ALL WORK TOGETHER TO IMPLEMENT EVAL AND
        // OTHER COMMANDS THAT DON'T PRODUCE A LIST AS OUTPUT

    case MAPINNERCOMP:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        if(!ISLIST(*rplPeekData(2))) {
            rplError(ERR_LISTEXPECTED);

            return;
        }


        WORDPTR program=rplPeekData(1);
        if(ISIDENT(*program)) {
            WORDPTR *var=rplFindLAM(program,1);
            if(!var) {
                var=rplFindGlobal(program,1);
                if(!var) {
                    rplError(ERR_PROGRAMEXPECTED);

                    return;
                }
            }
            // HERE var HAS THE VARIABLE, GET THE CONTENTS
            program=*(var+1);
        }


        if(!ISPROGRAM(*program)) {
            rplError(ERR_PROGRAMEXPECTED);

            return;
        }

        // HERE WE HAVE program = PROGRAM TO EXECUTE

        // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
        rplCreateLAMEnvironment(IPtr);

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));     // LAM 1 = ROUTINE TO EXECUTE ON EVERY STEP
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(2)+1);     // LAM 2 = NEXT ELEMENT TO BE PROCESSED
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(2));     // LAM 3 = LIST
        if(Exceptions) { rplCleanupLAMs(0); return; }

        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST

        // THIS NEEDS TO BE DONE IN 3 STEPS:
        // MAP WILL PREPARE THE LAMS FOR OPEN EXECUTION
        // MAPPRE WILL PUSH THE LIST ELEMENTS AND EVAL THE PROGRAM
        // MAPPOST WILL CHECK IF ALL ELEMENTS WERE PROCESSED WITHOUT ERRORS, PACK THE LIST AND END
        //                    OR IT WILL EXECUTE MAPPRE ONCE MORE

        // THE INITIAL CODE FOR MAP MUST TRANSFER FLOW CONTROL TO A
        // SECONDARY THAT CONTAINS :: MAPPRE EVAL MAPPOST ;
        // MAPPOST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
        // IN ORDER TO KEEP THE LOOP RUNNING

        rplPushRet(IPtr);
        IPtr=(WORDPTR) listeval_seco;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

        // REMOVE THE PROGRAM FROM THE STACK IN ORDER TO REUSE listeval_seco
        // THIS CAUSES THE PROGRAM TO DISAPPEAR FROM THE STACK IF THERE'S AN ERROR
        // IT'S ACCEPTABLE ONLY BECAUSE MAPINNERCOMP IS MAINLY FOR INTERNAL USE
        rplDropData(1);

        rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD USER PROGRAM

        return;
    }

    case EVALPRE:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST


        WORDPTR nextobj=*rplGetLAMn(2);
        WORDPTR startobj;
        // EMPTY LISTS NEED TO BE HANDLED HERE (NO EVAL NEEDED)
        WORDPTR endmarker=rplSkipOb(*rplGetLAMn(3))-1;

        do {

            startobj=nextobj;

            while(ISLIST(*nextobj)) {
                // GET INSIDE THE LIST
                ++nextobj;
                // LEAVE A MARKER ON THE STACK. USE THE SECO OBJECT AS A MARKER TO SAVE STORAGE
                rplPushData((WORDPTR)listeval_seco);
            }

            while(*nextobj==MKOPCODE(LIBRARY_NUMBER,ENDLIST)) {
                if(nextobj==endmarker) {
                    // CLOSE THE MAIN LIST AND RETURN
                    WORDPTR *prevDStk = rplUnprotectData();

                    BINT newdepth=(BINT)(DSTop-prevDStk);


                    while(newdepth>0) {
                        rplOverwriteData(newdepth+1,rplPeekData(newdepth));
                        --newdepth;
                    }
                    rplDropData(1);

                    rplCleanupLAMs(0);
                    IPtr=rplPopRet();
                    CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP);
                    return;


                }
                else {
                    // CLOSE AN INNER LIST AND CONTINUE

                    WORDPTR *stkptr=DSTop-1;

                    while(*stkptr!=listeval_seco) --stkptr;  // FIND THE NEXT MARKER ON THE STACK
                    BINT nelements=(BINT)(DSTop-stkptr)-1;

                    rplNewBINTPush(nelements,DECBINT);
                    if(Exceptions) {
                        DSTop=rplUnprotectData();   // CLEANUP ALL INTERMEDIATE RESULTS
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP);
                        return;
                    }

                    rplCreateList();
                    if(Exceptions) {
                        DSTop=rplUnprotectData();   // CLEANUP ALL INTERMEDIATE RESULTS
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP);
                        return;
                    }

                    // NOW REMOVE THE MARKER FROM THE STACK
                    rplOverwriteData(2,rplPeekData(1));
                    rplDropData(1);


                }

                ++nextobj;

            }

        } while(nextobj!=startobj); // WE EXIT THE WHILE ONLY WHEN nextobj DIDN'T CHANGE WITHIN THE LOOP


        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE MAPERR WORD


        rplPutLAMn(2,nextobj);

        // PUSH THE NEXT OBJECT IN THE STACK
        rplPushData(nextobj);

        // NOW RECALL THE PROGRAM TO THE STACK

        rplPushData(*rplGetLAMn(1));

        if(Exceptions) { DSTop=rplUnprotectData(); rplCleanupLAMs(0); IPtr=rplPopRet(); CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP); return; }

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case EVALPOST:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR IN THE USER PROGRAM

        rplPutLAMn(2,rplSkipOb(*rplGetLAMn(2)));    // MOVE TO THE NEXT OBJECT IN THE LIST

        IPtr=(WORDPTR) listeval_seco;   // CONTINUE THE LOOP
        // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
        return;
    }
    case EVALERR:

        // SAME PROCEDURE AS ENDERR
        rplRemoveExceptionHandler();
        rplPopRet();
        rplUnprotectData();
        rplRemoveExceptionHandler();

        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;
        ExceptionPointer=IPtr;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP);
        return;



        // **********************************************************
        // THE COMMANDS THAT FOLLOW ALL WORK TOGETHER TO IMPLEMENT STREAM

    case STREAM:
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        if(!ISLIST(*rplPeekData(2))) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        BINT length=rplListLength(rplPeekData(2));

        if(length<2) {
            rplError(ERR_INVALIDLISTSIZE);
            return;
        }


        WORDPTR program=rplPeekData(1);
        if(ISIDENT(*program)) {
            WORDPTR *var=rplFindLAM(program,1);
            if(!var) {
                var=rplFindGlobal(program,1);
                if(!var) {
                    rplError(ERR_PROGRAMEXPECTED);
                    return;
                }
            }
            // HERE var HAS THE VARIABLE, GET THE CONTENTS
            program=*(var+1);
        }


        if(!ISPROGRAM(*program)) {
            rplError(ERR_PROGRAMEXPECTED);
            return;
        }

        // HERE WE HAVE program = PROGRAM TO EXECUTE

        // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
        rplCreateLAMEnvironment(IPtr);

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));     // LAM 1 = ROUTINE TO EXECUTE ON EVERY STEP
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(2)+1);     // LAM 2 = NEXT ELEMENT TO BE PROCESSED
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(2));     // LAM 3 = LIST
        if(Exceptions) { rplCleanupLAMs(0); return; }

        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST

        rplPushRet(IPtr);
        IPtr=(WORDPTR) stream_seco;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,STREAM);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

        rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD USER PROGRAM

        // PUSH THE FIRST ELEMENT
        rplPushData(rplGetListElement(rplPeekData(2),1));


        return;
    }

    case STREAMPRE:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST


        WORDPTR nextobj;
        // EMPTY LISTS NEED TO BE HANDLED HERE (NO EVAL NEEDED)
        WORDPTR endmarker=rplSkipOb(*rplGetLAMn(3))-1;

        nextobj=rplSkipOb(*rplGetLAMn(2));

        if(nextobj==endmarker) {
            // CLOSE THE MAIN LIST AND RETURN
            WORDPTR *prevDStk = rplUnprotectData();

            BINT newdepth=(BINT)(DSTop-prevDStk);

            rplOverwriteData(2+newdepth,rplPeekData(1));
            rplDropData(1+newdepth);

            rplCleanupLAMs(0);
            IPtr=rplPopRet();
            CurOpcode=MKOPCODE(LIBRARY_NUMBER,STREAM);
            return;
        }


        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE MAPERR WORD

        rplPutLAMn(2,nextobj);

        // PUSH THE NEXT OBJECT IN THE STACK
        rplPushData(nextobj);

        // NOW RECALL THE PROGRAM TO THE STACK

        rplPushData(*rplGetLAMn(1));

        if(Exceptions) { rplRemoveExceptionHandler(); DSTop=rplUnprotectData(); rplCleanupLAMs(0); IPtr=rplPopRet(); CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP); return; }

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case STREAMPOST:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR IN THE USER PROGRAM

        IPtr=(WORDPTR) stream_seco;   // CONTINUE THE LOOP
        // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
        return;
    }
    case STREAMERR:

        // SAME PROCEDURE AS ENDERR
        rplRemoveExceptionHandler();
        rplPopRet();
        rplUnprotectData();
        rplRemoveExceptionHandler();

        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);

        IPtr=rplPopRet();

        // AND THROW THE EXCEPTIONS TO THE UPPER HANDLER
        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;
        ExceptionPointer=IPtr;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,STREAM);
        return;


        // END OF STREAM
        // *****************************************************************

        // *****************************************************************
        // THE COMMANDS THAT FOLLOW ALL WORK TOGETHER TO IMPLEMENT UNARY OPERATORS

    case UNARYPRE:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST


        WORDPTR nextobj=*rplGetLAMn(2);
        WORDPTR startobj;
        // EMPTY LISTS NEED TO BE HANDLED HERE (NO EVAL NEEDED)
        WORDPTR endmarker=rplSkipOb(*rplGetLAMn(3))-1;

        do {

            startobj=nextobj;

            while(ISLIST(*nextobj)) {
                // GET INSIDE THE LIST
                ++nextobj;
                // LEAVE A MARKER ON THE STACK. USE THE SECO OBJECT AS A MARKER TO SAVE STORAGE
                rplPushData((WORDPTR)map_seco);
            }

            while(*nextobj==MKOPCODE(LIBRARY_NUMBER,ENDLIST)) {
                if(nextobj==endmarker) {
                    // CLOSE THE MAIN LIST AND RETURN
                    WORDPTR *prevDStk = rplUnprotectData();

                    BINT newdepth=(BINT)(DSTop-prevDStk);

                    rplNewBINTPush(newdepth,DECBINT);
                    if(Exceptions) {
                        DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=CMD_OVR_EVAL;
                        return;
                    }

                    rplCreateList();
                    if(Exceptions) {
                        DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=CMD_OVR_EVAL;
                        return;
                    }

                    rplOverwriteData(2,rplPeekData(1));
                    rplDropData(1);

                    rplCleanupLAMs(0);
                    IPtr=rplPopRet();
                    CurOpcode=CMD_OVR_EVAL;
                    return;


                }
                else {
                    // CLOSE AN INNER LIST AND CONTINUE

                    WORDPTR *stkptr=DSTop-1;

                    while(*stkptr!=map_seco) --stkptr;  // FIND THE NEXT MARKER ON THE STACK
                    BINT nelements=(BINT)(DSTop-stkptr)-1;

                    rplNewBINTPush(nelements,DECBINT);
                    if(Exceptions) {
                        DSTop=rplUnprotectData();   // CLEANUP ALL INTERMEDIATE RESULTS
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=CMD_OVR_EVAL;
                        return;
                    }

                    rplCreateList();
                    if(Exceptions) {
                        DSTop=rplUnprotectData();   // CLEANUP ALL INTERMEDIATE RESULTS
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=CMD_OVR_EVAL;
                        return;
                    }

                    // NOW REMOVE THE MARKER FROM THE STACK
                    rplOverwriteData(2,rplPeekData(1));
                    rplDropData(1);


                }

                ++nextobj;

            }

        } while(nextobj!=startobj); // WE EXIT THE WHILE ONLY WHEN nextobj DIDN'T CHANGE WITHIN THE LOOP


        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE MAPERR WORD


        rplPutLAMn(2,nextobj);

        // PUSH THE NEXT OBJECT IN THE STACK
        rplPushData(nextobj);

        // NOW RECALL THE PROGRAM TO THE STACK

        rplPushData(*rplGetLAMn(1));

        if(Exceptions) { DSTop=rplUnprotectData(); rplCleanupLAMs(0); IPtr=rplPopRet(); CurOpcode=CMD_OVR_EVAL; return; }

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case UNARYPOST:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR IN THE USER PROGRAM

        rplPutLAMn(2,rplSkipOb(*rplGetLAMn(2)));    // MOVE TO THE NEXT OBJECT IN THE LIST

        IPtr=(WORDPTR) unary_seco;   // CONTINUE THE LOOP
        // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
        return;
    }
    case UNARYERR:
        // SAME PROCEDURE AS ENDERR
        rplRemoveExceptionHandler();
        rplPopRet();
        rplUnprotectData();
        rplRemoveExceptionHandler();
        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;
        ExceptionPointer=IPtr;
        CurOpcode=CMD_OVR_EVAL;
        return;


        // END OF UNARY OPERATORS
        // *****************************************************************

        // *****************************************************************
        // THE COMMANDS THAT FOLLOW ALL WORK TOGETHER TO IMPLEMENT BINARY OPERATORS

    case BINARYPRE:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 and 3 = NEXT OBJECT ON EACH LIST, GETLAM 4 AND 5 = LISTS


        WORDPTR nextobj1=*rplGetLAMn(2);
        WORDPTR nextobj2=*rplGetLAMn(3);

        // EMPTY LISTS NEED TO BE HANDLED HERE (NO EVAL NEEDED)

            if(*nextobj1==MKOPCODE(LIBRARY_NUMBER,ENDLIST)) {
                 if(ISLIST(**rplGetLAMn(5))) {
                    if(*nextobj2!=MKOPCODE(LIBRARY_NUMBER,ENDLIST)) {
                        // THE LISTS HAVE INVALID DIMENSIONS
                        rplError(ERR_INVALIDLISTSIZE);
                        DSTop=rplUnprotectData(); // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        ExceptionPointer=IPtr;
                        CurOpcode=(CMD_OVR_ADD);
                        return;
                    }
                 }

                    // CLOSE THE MAIN LIST AND RETURN
                    WORDPTR *prevDStk = rplUnprotectData();

                    BINT newdepth=(BINT)(DSTop-prevDStk);

                    rplNewBINTPush(newdepth,DECBINT);
                    if(Exceptions) {
                        DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=(CMD_OVR_ADD);
                        return;
                    }

                    rplCreateList();
                    if(Exceptions) {
                        DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=(CMD_OVR_ADD);
                        return;
                    }

                    rplOverwriteData(3,rplPeekData(1));
                    rplDropData(2);

                    rplCleanupLAMs(0);
                    IPtr=rplPopRet();
                    CurOpcode=(CMD_OVR_ADD);
                    return;


                }

            if(*nextobj2==MKOPCODE(LIBRARY_NUMBER,ENDLIST)) {
                 if(ISLIST(**rplGetLAMn(4))) {
                    if(*nextobj1!=MKOPCODE(LIBRARY_NUMBER,ENDLIST)) {
                        // THE LISTS HAVE INVALID DIMENSIONS
                        rplError(ERR_INVALIDLISTSIZE);
                        DSTop=rplUnprotectData(); // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        ExceptionPointer=IPtr;
                        CurOpcode=(CMD_OVR_ADD);
                        return;
                    }
                 }

                    // CLOSE THE MAIN LIST AND RETURN
                    WORDPTR *prevDStk = rplUnprotectData();

                    BINT newdepth=(BINT)(DSTop-prevDStk);

                    rplNewBINTPush(newdepth,DECBINT);
                    if(Exceptions) {
                        DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=(CMD_OVR_ADD);
                        return;
                    }

                    rplCreateList();
                    if(Exceptions) {
                        DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=(CMD_OVR_ADD);
                        return;
                    }

                    rplOverwriteData(3,rplPeekData(1));
                    rplDropData(2);

                    rplCleanupLAMs(0);
                    IPtr=rplPopRet();
                    CurOpcode=(CMD_OVR_ADD);
                    return;


                }



        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE MAPERR WORD


        rplPutLAMn(2,nextobj1);
        rplPutLAMn(3,nextobj2);

        // PUSH THE NEXT OBJECT IN THE STACK
        rplPushData(nextobj1);
        rplPushData(nextobj2);


        // NOW RECALL THE PROGRAM TO THE STACK

        rplPushData(*rplGetLAMn(1));

        if(Exceptions) { DSTop=rplUnprotectData(); rplCleanupLAMs(0); IPtr=rplPopRet(); CurOpcode=(CMD_OVR_ADD); return; }

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case BINARYPOST:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 and 3 = NEXT OBJECT ON EACH LIST, GETLAM 4 AND 5 = LISTS

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR IN THE USER PROGRAM

        if(ISLIST(**rplGetLAMn(4))) rplPutLAMn(2,rplSkipOb(*rplGetLAMn(2)));    // MOVE TO THE NEXT OBJECT IN THE LIST
        if(ISLIST(**rplGetLAMn(5))) rplPutLAMn(3,rplSkipOb(*rplGetLAMn(3)));    // MOVE TO THE NEXT OBJECT IN THE LIST

        IPtr=(WORDPTR) binary_seco;   // CONTINUE THE LOOP
        // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
        return;
    }
    case BINARYERR:
        // SAME PROCEDURE AS ENDERR
        rplRemoveExceptionHandler();
        rplPopRet();
        rplUnprotectData();
        rplRemoveExceptionHandler();
        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;
        ExceptionPointer=IPtr;
        CurOpcode=(CMD_OVR_ADD);
        return;


        // END OF BINARY OPERATORS
        // *****************************************************************

        // *****************************************************************
        // THE COMMANDS THAT FOLLOW ALL WORK TOGETHER TO IMPLEMENT BINARY OPERATORS THAT PERFORM A TRUE/FALSE TEST

    case TESTPRE:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 and 3 = NEXT OBJECT ON EACH LIST, GETLAM 4 AND 5 = LISTS


        WORDPTR nextobj1=*rplGetLAMn(2);
        WORDPTR nextobj2=*rplGetLAMn(3);

        // EMPTY LISTS NEED TO BE HANDLED HERE (NO EVAL NEEDED)

            if(*nextobj1==MKOPCODE(LIBRARY_NUMBER,ENDLIST)) {
                 if(ISLIST(**rplGetLAMn(5))) {
                    if(*nextobj2!=MKOPCODE(LIBRARY_NUMBER,ENDLIST)) {
                        // THE LISTS HAVE INVALID DIMENSIONS
                        rplError(ERR_INVALIDLISTSIZE);
                        DSTop=rplUnprotectData(); // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        ExceptionPointer=IPtr;
                        CurOpcode=(CMD_OVR_ADD);
                        return;
                    }
                 }

                    // CLOSE THE MAIN LIST AND RETURN
                    WORDPTR *prevDStk = rplUnprotectData();

                    BINT newdepth=(BINT)(DSTop-prevDStk);
                    BINT result=1;

                    if(newdepth) {
                        BINT f;
                        for(f=1;f<=newdepth;++f)
                        {
                        if(rplIsFalse(rplPeekData(f))) { result=0; break; }
                        }
                    } else {
                        // THIS CAN ONLY HAPPEN WHEN BOTH LISTS WERE EMPTY
                        WORD Opcode=*((*rplGetLAMn(1))+1);  // GET OPCODE FROM THE PROGRAM WE APPLIED

                        switch(OPCODE(Opcode))
                        {
                        case OVR_EQ:
                        case OVR_SAME:
                        case OVR_GTE:
                        case OVR_LTE:
                            // TWO EMPTY LISTS ARE EQUAL
                            result=1;
                            break;
                        default:
                            // ALL OTHER TESTS ARE FALSE
                            result=0;
                            break;
                        }


                    }
                    DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK

                    rplDropData(2); // REMOVE ORIGINAL ARGUMENTS
                    if(result) rplPushData((WORDPTR)one_bint);
                    else rplPushData((WORDPTR)zero_bint);

                    rplCleanupLAMs(0);
                    IPtr=rplPopRet();
                    CurOpcode=(CMD_OVR_ADD);
                    return;


                }

            if(*nextobj2==MKOPCODE(LIBRARY_NUMBER,ENDLIST)) {
                 if(ISLIST(**rplGetLAMn(4))) {
                    if(*nextobj1!=MKOPCODE(LIBRARY_NUMBER,ENDLIST)) {
                        // THE LISTS HAVE INVALID DIMENSIONS
                        rplError(ERR_INVALIDLISTSIZE);
                        DSTop=rplUnprotectData(); // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        ExceptionPointer=IPtr;
                        CurOpcode=(CMD_OVR_ADD);
                        return;
                    }
                 }

                    // CLOSE THE MAIN LIST AND RETURN
                    WORDPTR *prevDStk = rplUnprotectData();

                    BINT newdepth=(BINT)(DSTop-prevDStk);

                    rplNewBINTPush(newdepth,DECBINT);
                    if(Exceptions) {
                        DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=(CMD_OVR_ADD);
                        return;
                    }

                    rplCreateList();
                    if(Exceptions) {
                        DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        CurOpcode=(CMD_OVR_ADD);
                        return;
                    }

                    rplOverwriteData(3,rplPeekData(1));
                    rplDropData(2);

                    rplCleanupLAMs(0);
                    IPtr=rplPopRet();
                    CurOpcode=(CMD_OVR_ADD);
                    return;


                }



        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE MAPERR WORD


        rplPutLAMn(2,nextobj1);
        rplPutLAMn(3,nextobj2);

        // PUSH THE NEXT OBJECT IN THE STACK
        rplPushData(nextobj1);
        rplPushData(nextobj2);


        // NOW RECALL THE PROGRAM TO THE STACK

        rplPushData(*rplGetLAMn(1));

        if(Exceptions) { DSTop=rplUnprotectData(); rplCleanupLAMs(0); IPtr=rplPopRet(); CurOpcode=(CMD_OVR_ADD); return; }

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case TESTPOST:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 and 3 = NEXT OBJECT ON EACH LIST, GETLAM 4 AND 5 = LISTS

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR IN THE USER PROGRAM

        if(ISLIST(**rplGetLAMn(4))) rplPutLAMn(2,rplSkipOb(*rplGetLAMn(2)));    // MOVE TO THE NEXT OBJECT IN THE LIST
        if(ISLIST(**rplGetLAMn(5))) rplPutLAMn(3,rplSkipOb(*rplGetLAMn(3)));    // MOVE TO THE NEXT OBJECT IN THE LIST

        IPtr=(WORDPTR) testop_seco;   // CONTINUE THE LOOP
        // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
        return;
    }
    case TESTERR:
        // SAME PROCEDURE AS ENDERR
        rplRemoveExceptionHandler();
        rplPopRet();
        rplUnprotectData();
        rplRemoveExceptionHandler();
        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;
        ExceptionPointer=IPtr;
        CurOpcode=(CMD_OVR_SAME);
        return;


        // END OF BINARY OPERATORS
        // *****************************************************************














        // ***************************************************************************************
        // THE COMMANDS THAT FOLLOW ALL WORK TOGETHER TO IMPLEMENT SUMLIST, PRODLIST

    case PRODLIST:
    case SUMLIST:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        if(!ISLIST(*rplPeekData(1))) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        BINT length=rplListLength(rplPeekData(1));

        if(length<1) {
            rplError(ERR_INVALIDLISTSIZE);

            return;
        }

        // THIS DEVIATES FROM USERRPL: SUMLIST WITH A SINGLE ELEMENT RETURNS INVALID DIMENSION ERROR
        if(length==1) {
            // JUST RETURN THE ONLY ELEMENT
            rplPushData(rplGetListElement(rplPopData(),1));
            return;
        }


        WORDPTR program=rplAllocTempOb(2);
        if(!program) {
            return;
        }

        program[0]=MKPROLOG(DOCOL,2);
        if(OPCODE(CurOpcode)==SUMLIST) program[1]=(CMD_OVR_ADD);
        if(OPCODE(CurOpcode)==PRODLIST) program[1]=(CMD_OVR_MUL);
        program[2]=CMD_SEMI;

        // HERE WE HAVE program = PROGRAM TO EXECUTE

        // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
        rplCreateLAMEnvironment(IPtr);

        rplCreateLAM((WORDPTR)nulllam_ident,program);     // LAM 1 = ROUTINE TO EXECUTE ON EVERY STEP
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1)+1);     // LAM 2 = NEXT ELEMENT TO BE PROCESSED
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));     // LAM 3 = LIST
        if(Exceptions) { rplCleanupLAMs(0); return; }

        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST

        rplPushRet(IPtr);
        IPtr=(WORDPTR) oplist_seco;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,SUMLIST);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

        rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD USER PROGRAM

        // PUSH THE FIRST ELEMENT
        rplPushData(rplGetListElement(rplPeekData(1),1));


        return;
    }

    case OPLISTPRE:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST


        WORDPTR nextobj;
        // EMPTY LISTS NEED TO BE HANDLED HERE (NO EVAL NEEDED)
        WORDPTR endmarker=rplSkipOb(*rplGetLAMn(3))-1;

        nextobj=rplSkipOb(*rplGetLAMn(2));

        if(nextobj==endmarker) {
            // CLOSE THE MAIN LIST AND RETURN
            WORDPTR *prevDStk = rplUnprotectData();

            BINT newdepth=(BINT)(DSTop-prevDStk);

            rplOverwriteData(1+newdepth,rplPeekData(1));
            rplDropData(newdepth);

            rplCleanupLAMs(0);
            IPtr=rplPopRet();
            CurOpcode=MKOPCODE(LIBRARY_NUMBER,SUMLIST);
            return;
        }


        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE MAPERR WORD

        rplPutLAMn(2,nextobj);

        // PUSH THE NEXT OBJECT IN THE STACK
        rplPushData(nextobj);

        // NOW RECALL THE PROGRAM TO THE STACK

        rplPushData(*rplGetLAMn(1));

        if(Exceptions) { DSTop=rplUnprotectData(); rplCleanupLAMs(0); IPtr=rplPopRet(); CurOpcode=MKOPCODE(LIBRARY_NUMBER,SUMLIST); return; }

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case OPLISTPOST:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR IN THE USER PROGRAM

        IPtr=(WORDPTR) oplist_seco;   // CONTINUE THE LOOP
        // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
        return;
    }
    case OPLISTERR:
        // SAME PROCEDURE AS ENDERR
        rplRemoveExceptionHandler();
        rplPopRet();
        rplUnprotectData();
        rplRemoveExceptionHandler();
        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;
        ExceptionPointer=IPtr;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,SUMLIST);
        return;


        // END OF OPLIST
        // *****************************************************************

        // **********************************************************
        // THE COMMANDS THAT FOLLOW ALL WORK TOGETHER TO IMPLEMENT DELTALIST

    case DELTALIST:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        if(!ISLIST(*rplPeekData(1))) {
            rplError(ERR_LISTEXPECTED);

            return;
        }

        BINT length=rplListLength(rplPeekData(1));

        if(length<2) {
            rplError(ERR_INVALIDLISTSIZE);

            return;
        }


        WORDPTR program=rplAllocTempOb(2);
        if(!program) {
            return;
        }

        program[0]=MKPROLOG(DOCOL,2);
        program[1]=(CMD_OVR_SUB);
        program[2]=CMD_SEMI;

        // HERE WE HAVE program = PROGRAM TO EXECUTE

        // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
        rplCreateLAMEnvironment(IPtr);

        rplCreateLAM((WORDPTR)nulllam_ident,program);     // LAM 1 = ROUTINE TO EXECUTE ON EVERY STEP
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1)+1);     // LAM 2 = NEXT ELEMENT TO BE PROCESSED
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,rplPeekData(1));     // LAM 3 = LIST
        if(Exceptions) { rplCleanupLAMs(0); return; }

        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST

        rplPushRet(IPtr);
        IPtr=(WORDPTR) deltalist_seco;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,DELTALIST);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

        rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD USER PROGRAM

        // PUSH THE FIRST ELEMENT
        rplPushData(rplGetListElement(rplPeekData(1),1));


        return;
    }

    case DELTAPRE:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST


        WORDPTR nextobj;
        // EMPTY LISTS NEED TO BE HANDLED HERE (NO EVAL NEEDED)
        WORDPTR endmarker=rplSkipOb(*rplGetLAMn(3))-1;

        nextobj=rplSkipOb(*rplGetLAMn(2));

         if(nextobj==endmarker) {
            // CLOSE THE MAIN LIST AND RETURN

            // REMOVE THE PREVIOUS ARGUMENT
            rplPopData();

            WORDPTR *prevDStk = rplUnprotectData();

            BINT newdepth=(BINT)(DSTop-prevDStk);

            rplNewBINTPush(newdepth,DECBINT);
            if(Exceptions) {
                DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
                rplCleanupLAMs(0);
                IPtr=rplPopRet();
                CurOpcode=MKOPCODE(LIBRARY_NUMBER,DELTALIST);
                return;
            }

            rplCreateList();
            if(Exceptions) {
                DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
                rplCleanupLAMs(0);
                IPtr=rplPopRet();
                CurOpcode=MKOPCODE(LIBRARY_NUMBER,DELTALIST);
                return;
            }

            rplOverwriteData(2,rplPeekData(1));
            rplDropData(1);

            rplCleanupLAMs(0);
            IPtr=rplPopRet();
            CurOpcode=MKOPCODE(LIBRARY_NUMBER,DELTALIST);
            return;
        }


        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE MAPERR WORD

        rplPutLAMn(2,nextobj);

        // PUSH THE NEXT OBJECT IN THE STACK
        rplPushData(rplPeekData(1));
        rplOverwriteData(2,nextobj);

        // NOW RECALL THE PROGRAM TO THE STACK


        rplPushData(*rplGetLAMn(1));

        if(Exceptions) { DSTop=rplUnprotectData(); rplCleanupLAMs(0); IPtr=rplPopRet(); CurOpcode=MKOPCODE(LIBRARY_NUMBER,MAP); return; }

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case DELTAPOST:
    {
        // HERE GETLAM1 = PROGRAM, GETLAM 2 = NEXT OBJECT, GETLAM3 = LIST

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR IN THE USER PROGRAM

        rplPushData(*rplGetLAMn(2));    // PUSH LAST OBJECT AGAIN THE STACK FOR NEXT OPERATION
        IPtr=(WORDPTR) deltalist_seco;   // CONTINUE THE LOOP
        // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
        return;
    }
    case DELTAERR:
        // SAME PROCEDURE AS ENDERR
        rplRemoveExceptionHandler();
        rplPopRet();
        rplUnprotectData();
        rplRemoveExceptionHandler();
        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;
        ExceptionPointer=IPtr;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,DELTALIST);
        return;


        // END OF DELTALIST
        // *****************************************************************

    case ADD:
        // CONCATENATE LISTS
     {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        BINT size1,size2;
        WORDPTR obj1=rplPeekData(2),obj2=rplPeekData(1);
        if(ISPROLOG(*obj1)) size1=OBJSIZE(*obj1)+1;
        else size1=1;
        if(ISLIST(*obj1)) size1-=2; // DO NOT COUNT THE PROLOG AND ENDLIST MARKER IF THE LIST

        if(ISPROLOG(*obj2)) size2=OBJSIZE(*obj2)+1;
        else size2=1;
        if(ISLIST(*obj2)) size2-=2; // DO NOT COUNT THE PROLOG AND ENDLIST MARKER IF THE LIST

        WORDPTR newlist=rplAllocTempOb(size1+size2+1);
        if(!newlist) {
            return;
        }
        *newlist=MKPROLOG(LIBRARY_NUMBER,size1+size2+1);

        // DO NOT REUSE obj1, COULD'VE BEEN MOVED BY GC
        if(ISLIST(*rplPeekData(2))) memmovew(newlist+1,rplPeekData(2)+1,size1);
        else memmovew(newlist+1,rplPeekData(2),size1);

        if(ISLIST(*rplPeekData(1))) memmovew(newlist+1+size1,rplPeekData(1)+1,size2);
        else memmovew(newlist+1+size1,rplPeekData(1),size2);

        // CLOSE THE NEW LIST WITH ENDLIST

        newlist[size1+size2+1]=MKOPCODE(LIBRARY_NUMBER,ENDLIST);

        // PUSH IT ON THE STACK
        rplOverwriteData(2,newlist);
        rplDropData(1);

        return;

    }

    case ADDROT:
        // APPEND AN ELEMENT TO A LIST AND ROTATE IT
        // LIST ELEM NITEMS -> { Obj ... ObjN ELEM }
        // THE NEW LIST DROPS ELEMENTS TO BE NITEMS MAX.
     {
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISLIST(*rplPeekData(3))) {
            rplError(ERR_LISTEXPECTED);
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        BINT64 nitems=rplReadNumberAsBINT(rplPeekData(1));
        if(nitems<1) {
            rplError(ERR_POSITIVE_INTEGER_EXPECTED);
            return;
        }

        WORDPTR newlist=rplListAddRot(rplPeekData(3),rplPeekData(2),nitems);
        if(!newlist) return;
        rplDropData(2);
        rplOverwriteData(1,newlist);

        return;

    }

        // ADD MORE OPCODES HERE

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

        // CHECK IF THE TOKEN IS THE OPEN BRACKET

        if(*((char * )TokenStart)=='{')
        {

            rplCompileAppend((WORD) MKPROLOG(LIBRARY_NUMBER,0));
            if(TokenLen>1) {
                NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                RetNum=OK_STARTCONSTRUCT;
            }
            else RetNum=OK_STARTCONSTRUCT;
            return;
        }
        // CHECK IF THE TOKEN IS THE CLOSING BRACKET

        if(((char * )TokenStart)[TokenLen-1]=='}')
        {
            if(TokenLen>1) {
                BlankStart=NextTokenStart=(WORDPTR)(((char * )TokenStart)+TokenLen-1);
                RetNum=ERR_NOTMINE_SPLITTOKEN;
                return;
            }


            if(CurrentConstruct!=MKPROLOG(LIBRARY_NUMBER,0)) {
                RetNum=ERR_SYNTAX;
                return;
            }
            rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,ENDLIST));
            RetNum=OK_ENDCONSTRUCT;
            return;
        }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

        return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        if(ISPROLOG(*DecompileObject)) {
            if(!ISAUTOEXPLIST(*DecompileObject)) rplDecompAppendString((BYTEPTR)"{");
            RetNum=OK_STARTCONSTRUCT;
            return;
        }

        if(*DecompileObject==CMD_ENDLIST) {
            if(!ISAUTOEXPLIST(CurrentConstruct)) rplDecompAppendString((BYTEPTR)"}");
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
    if(ISPROLOG(*ObjectPTR)) {
        // BASIC CHECKS
        WORDPTR ptr,objend;

        objend=rplSkipOb(ObjectPTR);
        ptr=ObjectPTR+1;

        while((*ptr!=CMD_ENDLIST)&&(ptr<objend)) {

            // TODO: RECURSIVELY CHECK OBJECT VALIDITY IN HERE

            ptr=rplSkipOb(ptr);
        }

        if(ptr!=objend-1) { RetNum=ERR_INVALID; return; }
    }
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
        if(MENUNUMBER(MenuCodeArg)>0) {
            RetNum=ERR_NOTMINE;
            return;
        }
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR=(WORDPTR)lib62_menu;
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

