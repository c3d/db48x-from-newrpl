/*
 * Copyright (c) 2018, Claudio Lapilli and the newRPL Team
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
#define LIBRARY_NUMBER  104

//@TITLE=Numeric solvers

#define ERROR_LIST \
        ERR(NOROOTFOUND,0)

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(NUMINT,MKTOKENINFO(6,TITYPE_FUNCTION,4,2)), \
    CMD(ROOT,MKTOKENINFO(4,TITYPE_FUNCTION,3,2))





// ADD MORE OPCODES HERE


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************
INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib104_menu);




// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib104_menu,
    0
};




void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    // LIBRARIES THAT DEFINE ONLY COMMANDS STILL HAVE TO RESPOND TO A FEW OVERLOADABLE OPERATORS
    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE) {
        // ONLY RESPOND TO EVAL, EVAL1 AND XEQ FOR THE COMMANDS DEFINED HERE
        // IN CASE OF COMMANDS TREATED AS OBJECTS (WHEN EMBEDDED IN LISTS)
        if( (OPCODE(CurOpcode)==OVR_EVAL)||
                (OPCODE(CurOpcode)==OVR_EVAL1)||
                (OPCODE(CurOpcode)==OVR_XEQ) )
        {
            // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            if(ISPROLOG(*rplPeekData(1))) {
                rplError(ERR_UNRECOGNIZEDOBJECT);
                return;
            }
            WORD saveOpcode=CurOpcode;
            CurOpcode=*rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode=saveOpcode;
            return;
        }
        // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
            if(OPCODE(CurOpcode)==OVR_SAME) {
                if(*rplPeekData(2)==*rplPeekData(1)) {
                    rplDropData(2);
                    rplPushTrue();
                } else {
                    rplDropData(2);
                    rplPushFalse();
                }

            }
            else {
                rplError(ERR_INVALIDOPCODE);
                return;
            }

    }


    switch(OPCODE(CurOpcode))
    {

case NUMINT:
    {
        //@SHORT_DESC=Numerical integration (adaptive Simpson)
        //@NEW
        // DOES NUMERIC INTEGRATION ON FUNCTION PROVIDED BY THE USER
        // TAKES A PROGRAM FROM THE STACK, START AND END LIMITS, AND ERROR TOLERANCE

        if(rplDepthData()<4) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISPROGRAM(*rplPeekData(4)) && !ISSYMBOLIC(*rplPeekData(4)) ) {
            rplError(ERR_PROGRAMEXPECTED);
            return;
        }

        if(!ISNUMBERCPLX(*rplPeekData(3)) || !ISNUMBERCPLX(*rplPeekData(2)))
        {
            rplError(ERR_COMPLEXORREALEXPECTED);
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        WORDPTR *dstkptr=DSTop;

#define ARG_USERFUNC  *(dstkptr-4)
#define ARG_A   *(dstkptr-3)
#define ARG_B   *(dstkptr-2)
#define ARG_ERROR *(dstkptr-1)

#define TOTAL_AREA *dstkptr
        rplPushDataNoGrow((WORDPTR)zero_bint);          // INITIAL AREA = 0

        // PREPARE FOR MAIN LOOP
        // MAIN LOOP NEEDS ERR A B C FA FB FC AREA ON THE STACK

        rplPushDataNoGrow(ARG_ERROR);                   // ERROR
        rplPushDataNoGrow(ARG_A);                       // A
        rplPushData(ARG_B);                             // B
        if(Exceptions) { DSTop=dstkptr;  return; }


        rplPushDataNoGrow(ARG_A);
        rplPushDataNoGrow(ARG_B);
        rplCallOvrOperator(CMD_OVR_ADD);
        if(Exceptions) { DSTop=dstkptr;  return; }
        rplPushData((WORDPTR)one_half_real);
        rplCallOvrOperator(CMD_OVR_MUL);                // C=(A+B)/2
        if(Exceptions) { DSTop=dstkptr;  return; }

        rplPushDataNoGrow(ARG_A);
        rplEvalUserFunc(ARG_USERFUNC,CMD_OVR_NUM);      // F(A)
        if(Exceptions) { DSTop=dstkptr;  return; }

        rplPushDataNoGrow(ARG_B);
        rplEvalUserFunc(ARG_USERFUNC,CMD_OVR_NUM);      // F(B)
        if(Exceptions) { DSTop=dstkptr;  return; }

        rplPushDataNoGrow(rplPeekData(3));  // C
        rplEvalUserFunc(ARG_USERFUNC,CMD_OVR_NUM);      // F(C)
        if(Exceptions) { DSTop=dstkptr;  return; }

        // COMPUTE INITIAL AREA APPROXIMATION
        // AREA = (F(A)+F(B)+4*F(C))*(B-A)/6
        rplPushData(rplPeekData(1));
        rplPushData((WORDPTR)four_bint);
        rplCallOvrOperator(CMD_OVR_MUL);                // 4*F(C)
        if(Exceptions) { DSTop=dstkptr;  return; }

        rplPushData(rplPeekData(4));    // F(A)
        rplCallOvrOperator(CMD_OVR_ADD);
        if(Exceptions) { DSTop=dstkptr;  return; }
        rplPushData(rplPeekData(4));    // F(B)
        rplCallOvrOperator(CMD_OVR_ADD);                // F(A)+F(B)+4*F(C)
        if(Exceptions) { DSTop=dstkptr;  return; }

        rplPushData(rplPeekData(6));    // B
        rplPushData(rplPeekData(8));    // A
        rplCallOvrOperator(CMD_OVR_SUB);
        if(Exceptions) { DSTop=dstkptr;  return; }
        rplCallOvrOperator(CMD_OVR_MUL);
        if(Exceptions) { DSTop=dstkptr;  return; }
        rplPushData((WORDPTR)six_bint);
        rplCallOvrOperator(CMD_OVR_DIV);    // AREA = (F(A)+F(B)+4*F(C)) * (B-A)/6

        rplPushData(rplPeekData(6));    // B
        rplPushData(rplPeekData(8));    // A
        rplCallOvrOperator(CMD_OVR_SUB);
        if(Exceptions) { DSTop=dstkptr;  return; }
        rplNewSINTPush(12,DECBINT);
        rplCallOvrOperator(CMD_OVR_DIV);

        // MAIN LOOP NEEDS: ERR A B C FA FB FC AREA H_12

        while(DSTop>dstkptr+1)
        {
            WORDPTR *argbase=DSTop-9;

#define     L_ERR argbase[0]
#define     L_A   argbase[1]
#define     L_B   argbase[2]
#define     L_C   argbase[3]
#define     L_FA  argbase[4]
#define     L_FB  argbase[5]
#define     L_FC  argbase[6]
#define     L_AREA argbase[7]
#define     L_H_12 argbase[8]

            // D=(A+C)/2

            rplPushData(L_A);
            rplPushData(L_C);
            rplCallOvrOperator(CMD_OVR_ADD);
            if(Exceptions) { DSTop=dstkptr;  return; }
            rplPushData((WORDPTR)one_half_real);
            rplCallOvrOperator(CMD_OVR_MUL);

            // F(D)
            rplPushData(rplPeekData(1));
            rplEvalUserFunc(ARG_USERFUNC,CMD_OVR_NUM);
            if(Exceptions) { DSTop=dstkptr;  return; }

            // E=(C+B)/2
            rplPushData(L_B);
            rplPushData(L_C);
            rplCallOvrOperator(CMD_OVR_ADD);
            if(Exceptions) { DSTop=dstkptr;  return; }
            rplPushData((WORDPTR)one_half_real);
            rplCallOvrOperator(CMD_OVR_MUL);

            // F(E)
            rplPushData(rplPeekData(1));
            rplEvalUserFunc(ARG_USERFUNC,CMD_OVR_NUM);
            if(Exceptions) { DSTop=dstkptr;  return; }

            // AREA_L=(F(A)+4*F(D)+F(C))*(B-A)/12
            rplPushData(rplPeekData(3));    // F(D)
            rplPushData((WORDPTR)four_bint);
            rplCallOvrOperator(CMD_OVR_MUL);
            if(Exceptions) { DSTop=dstkptr;  return; }
            rplPushData(L_FA);
            rplCallOvrOperator(CMD_OVR_ADD);
            if(Exceptions) { DSTop=dstkptr;  return; }
            rplPushData(L_FC);
            rplCallOvrOperator(CMD_OVR_ADD);
            if(Exceptions) { DSTop=dstkptr;  return; }

            rplPushData(L_H_12);
            rplCallOvrOperator(CMD_OVR_MUL);
            if(Exceptions) { DSTop=dstkptr;  return; }

            // AREA_R=(F(C)+4*F(E)+F(B))*(B-A)/12
            rplPushData(rplPeekData(2));    // F(E)
            rplPushData((WORDPTR)four_bint);
            rplCallOvrOperator(CMD_OVR_MUL);
            if(Exceptions) { DSTop=dstkptr;  return; }
            rplPushData(L_FB);
            rplCallOvrOperator(CMD_OVR_ADD);
            if(Exceptions) { DSTop=dstkptr;  return; }
            rplPushData(L_FC);
            rplCallOvrOperator(CMD_OVR_ADD);
            if(Exceptions) { DSTop=dstkptr;  return; }

            rplPushData(L_H_12);
            rplCallOvrOperator(CMD_OVR_MUL);
            if(Exceptions) { DSTop=dstkptr;  return; }


            // NEWERR=(AREA_L+AREA_R-L_AREA)/15
            rplPushData(rplPeekData(2));
            rplPushData(rplPeekData(2));
            rplCallOvrOperator(CMD_OVR_ADD);
            if(Exceptions) { DSTop=dstkptr;  return; }
            rplPushData(L_AREA);
            rplCallOvrOperator(CMD_OVR_SUB);
            if(Exceptions) { DSTop=dstkptr;  return; }
            rplNewSINTPush(15,DECBINT);
            rplCallOvrOperator(CMD_OVR_DIV);
            if(Exceptions) { DSTop=dstkptr;  return; }


            rplPushData(rplPeekData(1));
            rplCallOvrOperator(CMD_OVR_ABS);
            if(Exceptions) { DSTop=dstkptr;  return; }
            rplPushData(L_ERR);
            rplCallOvrOperator(CMD_OVR_LTE);
            if(Exceptions) { DSTop=dstkptr;  return; }

            if(rplIsFalse(rplPeekData(1))) {
                // IF ABS(NEWERR)<=L_ERR THEN AREA+=(AREA_L+AREA_R)+NEWERR
                // ELSE
                // PUT LEFT AND RIGHT PARTS ON THE STACK
                // PUSH RIGHT: L_ERR/2 C B E F(C) F(B) F(E) AREA_R L_H_12/2
                // OVERWRITE LEFT: L_ERR/2 A C D F(A) F(C) F(D) AREA_L L_H_12/2
                // HERE THERE'S 8 VALUES ON THE STACK OVER THE RIGHT PART: D F(D) E F(E) AREA_L AREA_R NEWERR FALSE

                rplOverwriteData(1,L_ERR);
                rplPushData((WORDPTR)one_half_real);
                rplCallOvrOperator(CMD_OVR_MUL);        // L_ERR/2
                if(Exceptions) { DSTop=dstkptr;  return; }

                rplPushData(L_H_12);
                rplPushData((WORDPTR)one_half_real);
                rplCallOvrOperator(CMD_OVR_MUL);        // PUSH L_H_12/2
                if(Exceptions) { DSTop=dstkptr;  return; }

                // IN THE STACK WE HAVE: L_ERR L_A L_B L_C L_FA L_FB L_FC L_AREA L_H_12  |  D     F(D)   E   F(E) AREA_L AREA_R NEWERR L_ERR/2 L_H_12/2
                //                                                                       |  9      8     7    6     5      4      3      2      1
                // WE WANT:             L_ERR/2 A   C   D  F(A) F(C) F(D) AREA_L L_H_12/2| L_ERR/2 C     B    E    F(C)   F(B)   F(E)  AREA_R L_H_12/2


                L_H_12=rplPeekData(1);              // L_H_12/2
                L_ERR=rplPeekData(2);               // L_ERR/2

                rplOverwriteData(2,rplPeekData(4)); // AREA_R
                rplOverwriteData(3,rplPeekData(6)); // F(E)
                rplOverwriteData(4,L_FB);           // F(B)

                L_AREA=rplPeekData(5);              // AREA_L
                rplOverwriteData(5,L_FC);           // F(C)
                rplOverwriteData(6,rplPeekData(7)); // E
                rplOverwriteData(7,L_B);            // B
                L_FC=rplPeekData(8);                // F(D)
                rplOverwriteData(8,L_C);            // C
                L_C=rplPeekData(9);                 // D
                rplOverwriteData(9,L_ERR);          // L_ERR/2

                L_FB=rplPeekData(5);                // F(C)
                L_B=rplPeekData(8);                 // C

                // DONE, WE HAVE LEFT AND RIGHT, NOW CLOSE THE LOOP

            }
            else {
                // IF ABS(NEWERR)<=L_ERR THEN AREA+=(AREA_L+AREA_R)+NEWERR

                // HERE THERE'S 8 NEW VALUES ON THE STACK: D F(D) E F(E) AREA_L AREA_R NEWERR TRUE

                rplOverwriteData(1,TOTAL_AREA);
                rplCallOvrOperator(CMD_OVR_ADD);
                if(Exceptions) { DSTop=dstkptr;  return; }
                rplCallOvrOperator(CMD_OVR_ADD);
                if(Exceptions) { DSTop=dstkptr;  return; }
                rplCallOvrOperator(CMD_OVR_ADD);
                if(Exceptions) { DSTop=dstkptr;  return; }

                TOTAL_AREA=rplPeekData(1);

                DSTop=argbase;  // DROP ALL VALUES FROM THE STACK AND CONTINUE THE LOOP
            }

#undef     L_ERR
#undef     L_A
#undef     L_B
#undef     L_C
#undef     L_FA
#undef     L_FB
#undef     L_FC
#undef     L_AREA
#undef     L_H_12

#undef ARG_USERFUNC
#undef ARG_A
#undef ARG_B
#undef ARG_ERROR

#undef TOTAL_AREA
        }

        // HERE THE STACK SHOULD CONTAIN ONLY THE TOTAL AREA + THE INITIAL ARGUMENTS

        rplOverwriteData(5,rplPeekData(1));
        rplDropData(4);


        return;
    }

    case ROOT:
        {
            //@SHORT_DESC=Root seeking
            //@INCOMPAT
            // NUMERIC ROOT FINDER ON FUNCTION PROVIDED BY THE USER
            // TAKES A PROGRAM FROM THE STACK, LEFT/RIGHT OF INITIAL INTERVAL AND ERROR TOLERANCE
            // USES BISECTION, CAN ONLY FIND REAL ROOTS

        if(rplDepthData()<4) {
            rplError(ERR_BADARGCOUNT);
            return;
        }


        if(!ISPROGRAM(*rplPeekData(4)) && !ISSYMBOLIC(*rplPeekData(4)) ) {
            rplError(ERR_PROGRAMEXPECTED);
            return;
        }

        if(!ISNUMBER(*rplPeekData(3)) || !ISNUMBER(*rplPeekData(2)))
        {
            rplError(ERR_REALEXPECTED);
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        WORDPTR *dstkptr=DSTop;

#define ARG_USERFUNC  *(dstkptr-4)
#define ARG_A   *(dstkptr-3)
#define ARG_B   *(dstkptr-2)
#define ARG_ERROR *(dstkptr-1)

        // PUSH COPIES OF A AND B
        rplPushDataNoGrow(ARG_A);
        rplPushDataNoGrow(ARG_B);

        // COMPUTE F(A)
        rplPushData(ARG_A);
        rplEvalUserFunc(ARG_USERFUNC,CMD_OVR_NUM);
        if(Exceptions) { DSTop=dstkptr;  return; }

        // COMPUTE F(B)
        rplPushData(ARG_B);
        rplEvalUserFunc(ARG_USERFUNC,CMD_OVR_NUM);
        if(Exceptions) { DSTop=dstkptr;  return; }

        REAL a,b,fa,fb;

        do {

        rplReadNumberAsReal(rplPeekData(4),&a);
        rplReadNumberAsReal(rplPeekData(3),&b);
        rplReadNumberAsReal(rplPeekData(2),&fa);
        rplReadNumberAsReal(rplPeekData(1),&fb);

        if( !((fa.flags^fb.flags)&F_NEGATIVE)) {
            // THERE'S NO ROOT IN THIS BRACKET, EXIT
            rplError(ERR_NOROOTFOUND);
            DSTop=dstkptr;
            return;
        }

        addReal(&RReg[0],&a,&b);
        newRealFromBINT(&RReg[1],5,-1);
        mulReal(&RReg[2],&RReg[0],&RReg[1]);    // C=(A+B)/2


        } while(1);

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
        ObjectPTR=(WORDPTR)ROMPTR_TABLE[MENUNUMBER(MenuCodeArg) + 2];
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



