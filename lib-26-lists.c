/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// LIBRARY ZERO HAS SPECIAL RUNSTREAM OPERATORS

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL
// LIB0 PROVIDES EXIT FROM RPL, BREAKPOINTS AND RUNSTREAM MANIPULATION OPCODES

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  26
#define LIB_ENUM lib26_enum
#define LIB_NAMES lib26_names
#define LIB_HANDLER lib26_handler
#define LIB_NUMBEROFCMDS LIB26_NUMBEROFCMDS

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(PUT), \
    CMD(PUTI), \
    CMD(GET), \
    CMD(GETI), \
    CMD(HEAD), \
    CMD(TAIL)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
#define CMD_EXTRANAME \
    "}", \
    "->LIST", \
    "LIST->", \
    "DOLIST", \
    "", \
    "", \
    ""

#define CMD_EXTRAENUM \
    ENDLIST, \
    TOLIST, \
    INNERCOMP, \
    CMDDOLIST, \
    DOLISTPRE, \
    DOLISTPOST, \
    DOLISTERR



// INTERNAL DECLARATIONS


// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_EXTRAENUM , CMD_LIST ,  LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
char *LIB_NAMES[]= { CMD_EXTRANAME , CMD_LIST  };
#undef CMD


extern WORD abnd_prog[];
extern WORD lam_baseseco_bint[];
extern WORD nulllam_ident[];

const WORD const dolist_seco[]={
 MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,DOLISTPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    MKOPCODE(LIB_OVERLOADABLE,OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,DOLISTPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,DOLISTERR),     // ERROR HANDLER
    CMD_SEMI
};










void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        rplPushData(IPtr);
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case PUT:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<3) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }

        WORDPTR list=rplPeekData(3);
        WORDPTR *var=0;
        if(ISIDENT(*list)) {
            var=rplFindLAM(list,1);
            if(!var) {
                var=rplFindGlobal(list,1);
                if(!var) {
                    Exceptions|=EX_BADARGTYPE;
                    ExceptionPointer=IPtr;
                    return;
                }

            }
            list=*(var+1);
        }
        if(!ISLIST(*list)) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        if(!ISNUMBER(*rplPeekData(2))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        BINT nitems=rplExplodeList(list);
        BINT position=rplReadNumberAsBINT(rplPeekData(nitems+3));
        if(Exceptions) return;

        if(position<1 || position>nitems) {
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
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
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
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
                    Exceptions|=EX_BADARGTYPE;
                    ExceptionPointer=IPtr;
                    return;
                }

            }
            list=*(var+1);
        }
        if(!ISLIST(*list)) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        if(!ISNUMBER(*rplPeekData(2))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }


        BINT nitems=rplExplodeList(list);

        // HERE THE STACK IS: LIST POSITION NEWOBJECT OBJ1 OBJ2 ... OBJN N

        BINT position=rplReadNumberAsBINT(rplPeekData(nitems+3));
        if(Exceptions) return;

        if(position<1 || position>nitems) {
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
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
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR list=rplPeekData(2);
        WORDPTR *var=0;
        if(ISIDENT(*list)) {
            var=rplFindLAM(list,1);
            if(!var) {
                var=rplFindGlobal(list,1);
                if(!var) {
                    Exceptions|=EX_BADARGTYPE;
                    ExceptionPointer=IPtr;
                    return;
                }

            }
            list=*(var+1);
        }
        if(!ISLIST(*list)) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        BINT nitems=rplExplodeList(list);
        BINT position=rplReadNumberAsBINT(rplPeekData(nitems+2));
        if(Exceptions) return;
        if(position<1 || position>nitems) {
            rplDropData(nitems+1);
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
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
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR list=rplPeekData(2);
        WORDPTR *var=0;
        if(ISIDENT(*list)) {
            var=rplFindLAM(list,1);
            if(!var) {
                var=rplFindGlobal(list,1);
                if(!var) {
                    Exceptions|=EX_BADARGTYPE;
                    ExceptionPointer=IPtr;
                    return;
                }

            }
            list=*(var+1);
        }
        if(!ISLIST(*list)) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        BINT nitems=rplExplodeList(list);
        BINT position=rplReadNumberAsBINT(rplPeekData(nitems+2));
        if(Exceptions) return;
        if(position<1 || position>nitems) {
            rplDropData(nitems+1);
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
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
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR list=rplPeekData(1);

        if(!ISLIST(*list)) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
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
            Exceptions|=EX_INVALID_DIM;
            ExceptionPointer=IPtr;
            return;
        }


        return;
    }

    case TAIL:
    {
        // CHECK ARGUMENTS
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR list=rplPeekData(1);

        if(!ISLIST(*list)) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
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
    case ENDLIST:
        return;
    case TOLIST:
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        rplCreateList();

        return;
    case INNERCOMP:
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(!ISLIST(*rplPeekData(1))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        rplExplodeList(rplPopData());

        return;


    case CMDDOLIST:
    {
        BINT initdepth=rplDepthData();
        if(initdepth<3) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }

        if(!ISNUMBER(*rplPeekData(2))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        // GET THE NUMBER OF LISTS

        BINT64 nlists=rplReadNumberAsBINT(rplPeekData(2));

        if(initdepth<2+nlists) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }

        WORDPTR program=rplPeekData(1);
        if(ISIDENT(*program)) {
            WORDPTR *var=rplFindLAM(program,1);
            if(!var) {
                var=rplFindGlobal(program,1);
                if(!var) {
                    Exceptions|=EX_UNDEFINED;
                    ExceptionPointer=IPtr;
                    return;
                }
            }
            // HERE var HAS THE VARIABLE, GET THE CONTENTS
            program=*(var+1);
        }

        if(!ISPROGRAM(*program)) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
       }

        // HERE WE HAVE program = PROGRAM TO EXECUTE, nlists = NUMBER OF LISTS

        // CHECK THAT ALL LISTS ARE ACTUALLY LISTS

        BINT f,l,length=-1;
        for(f=3;f<3+nlists;++f) {
            if(!ISLIST(*rplPeekData(f))) {
                Exceptions|=EX_BADARGTYPE;
                ExceptionPointer=IPtr;
                return;
            }
            // MAKE SURE ALL LISTS ARE EQUAL LENGTH
            l=rplListLength(rplPeekData(f));
            if(length<0) length=l;
            else if(l!=length) {
                Exceptions|=EX_INVALID_DIM;
                ExceptionPointer=IPtr;
                return;
            }
        }

        if(length<1) {
            Exceptions|=EX_INVALID_DIM;
            ExceptionPointer=IPtr;
            return;
        }

        // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
        nLAMBase=LAMTop;    // POINT THE GETLAM BASE TO THE NEW ENVIRONMENT
        rplCreateLAM(lam_baseseco_bint,IPtr);  // PUT MARKER IN LAM STACK, SET DOLIST AS THE OWNER
        // NOW CREATE A LOCAL VARIABLE FOR THE INDEX

        rplCreateLAM(nulllam_ident,rplPeekData(1));     // LAM 1 = ROUTINE TO EXECUTE ON EVERY STEP

        WORDPTR newb=rplNewBINT(nlists,DECBINT);
        if(!newb) {
            Exceptions|=EX_OUTOFMEM;
            ExceptionPointer=IPtr;
            rplCleanupLAMs(0);
            return;
        }

        rplCreateLAM(nulllam_ident,newb);     // LAM 2 = BINT WITH NUMBER OF LISTS

        if(Exceptions) { rplCleanupLAMs(0); return; }

        newb=rplNewBINT(length,DECBINT);
        if(!newb) {
            Exceptions|=EX_OUTOFMEM;
            ExceptionPointer=IPtr;
            rplCleanupLAMs(0);
            return;
        }

        rplCreateLAM(nulllam_ident,newb);     // LAM 3 = BINT WITH NUMBER OF ITEMS PER LIST

        if(Exceptions) { rplCleanupLAMs(0); return; }

        newb=rplNewBINT(1,DECBINT);
        if(!newb) {
            Exceptions|=EX_OUTOFMEM;
            ExceptionPointer=IPtr;
            rplCleanupLAMs(0);
            return;
        }

        rplCreateLAM(nulllam_ident,newb);     // LAM 4 = 1, BINT WITH CURRENT INDEX IN TEH LOOP


        for(f=0;f<nlists;++f) {
        rplCreateLAM(nulllam_ident,rplPeekData(2+nlists-f));     // LAM n+4 = LISTS IN REVERSE ORDER
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
            for(k=nlists;k>0;--k)
            {
                rplPushData(rplGetListElement(*rplGetLAMn(k+4),idx));
                if(Exceptions) { rplCleanupLAMs(0); IPtr=rplPopRet(); CurOpcode=MKOPCODE(LIBRARY_NUMBER,DOLIST); return; }
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
                CurOpcode=MKOPCODE(LIBRARY_NUMBER,DOLIST);
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
            CurOpcode=MKOPCODE(LIBRARY_NUMBER,DOLIST);
            return;
        }

        rplCreateList();
        if(Exceptions) {
            DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
            rplCleanupLAMs(0);
            IPtr=rplPopRet();
            CurOpcode=MKOPCODE(LIBRARY_NUMBER,DOLIST);
            return;
        }
        // HERE THE STACK HAS: LIST1... LISTN N PROGRAM NEWLIST
        rplOverwriteData(nlists+3,rplPeekData(1));
        rplDropData(nlists+2);

        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,DOLIST);
        return;
    }

    case DOLISTERR:
        // JUST CLEANUP AND EXIT
        rplUnprotectData();
        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ExceptionPointer=IPtr;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,DOLIST);
        return;


    case OVR_EVAL:
        // TODO: PERFORM EVAL ON EACH OBJECT INSIDE THE LIST
        // SAME AS {...} 1 << EVAL >> DOLIST
        return;
    case OVR_XEQ:
        // JUST LEAVE THE LIST ON THE STACK
        return;


        break;

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

       if((TokenLen==1) && (!strncmp((char * )TokenStart,"{",1)))
       {
           rplCompileAppend((WORD) MKPROLOG(LIBRARY_NUMBER,0));
           RetNum=OK_STARTCONSTRUCT;
           return;
       }
       // CHECK IF THE TOKEN IS THE CLOSING BRACKET

       if(((TokenLen==1) && (!strncmp((char *)TokenStart,"}",1))))
       {
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
        libCompileCmds(LIBRARY_NUMBER,LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

        // SINCE THIS IS THE LAST LIBRARY TO BE EVALUATED, DO ONE LAST PASS TO COMPILE IT AS AN IDENT
        // EITHER LAM OR IN USEROB
     return;

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        if(ISPROLOG(*DecompileObject)) {
            rplDecompAppendString((BYTEPTR)" { ");
            RetNum=OK_STARTCONSTRUCT;
            return;
        }


        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds(LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_VALIDATE:
        // VALIDATE RECEIVES OPCODES COMPILED BY OTHER LIBRARIES, TO BE INCLUDED WITHIN A COMPOSITE OWNED BY
        // THIS LIBRARY. EVERY COMPOSITE HAS TO EVALUATE IF THE OBJECT BEING COMPILED IS ALLOWED INSIDE THIS
        // COMPOSITE OR NOT. FOR EXAMPLE, A REAL MATRIX SHOULD ONLY ALLOW REAL NUMBERS INSIDE, ANY OTHER
        // OPCODES SHOULD BE REJECTED AND AN ERROR THROWN.
        // TokenStart = token string
        // TokenLen = token length
        // ArgNum2 = Opcode/WORD of object

        // VALIDATE RETURNS:
        // RetNum =  enum CompileErrors



        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;

}




