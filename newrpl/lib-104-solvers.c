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
        ERR(NOROOTFOUND,0), \
        ERR(LISTOFEQEXPECTED,1), \
        ERR(INVALIDLISTOFVARS,2), \
        ERR(INVALIDVARRANGE,3), \
        ERR(REALVALUEDFUNCTIONSONLY,4), \
        ERR(TRYDIFFERENTRANGE,5)

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(NUMINT,MKTOKENINFO(6,TITYPE_FUNCTION,4,2)), \
    CMD(ROOT,MKTOKENINFO(4,TITYPE_FUNCTION,3,2)), \
    CMD(MSOLVE,MKTOKENINFO(4,TITYPE_FUNCTION,5,2))




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
INCLUDE_ROMOBJECT(lib104_TVMmenu);



// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib104_menu,
    (WORDPTR)lib104_TVMmenu,

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
                return;
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
                //*** TESTING: DO NOT HALVE THE ERROR EACH TIME
                //plPushData((WORDPTR)one_half_real);
                //rplCallOvrOperator(CMD_OVR_MUL);        // L_ERR/2
                //if(Exceptions) { DSTop=dstkptr;  return; }

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

        // MAKE SURE THE ERROR IS POSITIVE
        rplCallOvrOperator(CMD_OVR_ABS);
        if(Exceptions) { DSTop=dstkptr;  return; }

        // PUSH COPIES OF A AND B
        rplPushDataNoGrow(ARG_A);
        rplPushDataNoGrow(ARG_B);
        rplCallOvrOperator(CMD_OVR_LTE);
        if(Exceptions) { DSTop=dstkptr;  return; }

        if(rplIsFalse(rplPopData())) {
            // SWAP POINTS A AND B
            WORDPTR tmp;
            tmp=ARG_A;
            ARG_A=ARG_B;
            ARG_B=tmp;
        }

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

        REAL a,b,fa,fb,fc,err;

        do {

        rplReadNumberAsReal(rplPeekData(4),&a);
        rplReadNumberAsReal(rplPeekData(3),&b);
        rplReadNumberAsReal(rplPeekData(2),&fa);
        rplReadNumberAsReal(rplPeekData(1),&fb);
        rplReadNumberAsReal(ARG_ERROR,&err);

        if( !((fa.flags^fb.flags)&F_NEGATIVE)) {

            if(iszeroReal(&fa)) {
                // WE ALREADY HAVE A ROOT, RETURN IT
                rplNewRealPush(&a);
                ARG_USERFUNC=rplPeekData(1);    // OVERWRITE FIRST ARGUMENT WITH THE RESULT
                DSTop=dstkptr-3;    // RESTORE AND DROP 3 ARGUMENTS
                return;
            }
            if(iszeroReal(&fb)) {
                // WE ALREADY HAVE A ROOT, RETURN IT
                rplNewRealPush(&b);
                ARG_USERFUNC=rplPeekData(1);    // OVERWRITE FIRST ARGUMENT WITH THE RESULT
                DSTop=dstkptr-3;    // RESTORE AND DROP 3 ARGUMENTS
                return;
            }
            // THERE'S NO ROOT IN THIS BRACKET, EXIT
            rplError(ERR_NOROOTFOUND);
            DSTop=dstkptr;
            return;
        }

        BINT aff=fa.flags;

        fa.flags&=~F_NEGATIVE;
        fb.flags&=~F_NEGATIVE;      // TAKE ABSOLUTE VALUE FOR COMPARISON

        subReal(&RReg[0],&b,&a);    // B-A IS GUARANTEED TO BE POSITIVE
        if(ltReal(&RReg[0],&err)) break;    // WE REACHED THE TOLERANCE

        newRealFromBINT(&RReg[1],5,-1);

        // IMPROVEMENT FOR FASTER CONVERGENCE, COMMENT TO DISABLE
        if(gtReal(&fb,&fa)) {
            copyReal(&RReg[6],&fb);     // BIG VALUE
            addReal(&RReg[4],&fa,&fa);  // 2*SMALL
            addReal(&RReg[5],&fa,&RReg[4]); // 3*SMALL
            if(ltReal(&RReg[5],&fb)) {  // BIG VALUE > 3*SMALL
                // USE THE QUARTER POINT INSTEAD OF THE CENTER
                newRealFromBINT(&RReg[1],25,-2);
            }
        } else {
            copyReal(&RReg[6],&fa);     // BIG VALUE
            addReal(&RReg[4],&fb,&fb);  // 2*SMALL
            addReal(&RReg[5],&fb,&RReg[4]); // 3*SMALL
            if(ltReal(&RReg[5],&fa)) {  // BIG VALUE > 3*SMALL
                // USE THE QUARTER POINT INSTEAD OF THE CENTER
                newRealFromBINT(&RReg[1],75,-2);
            }
        }



        mulReal(&RReg[2],&RReg[0],&RReg[1]);    // (B-A)*(COEFF=0.25,0.5 OR 0.75)
        addReal(&RReg[0],&a,&RReg[2]);  // C= EITHER AT 1/4, 1/2 OR 3/4 POSITION


        // COMPUTE F(C)
        rplNewRealFromRRegPush(0);
        rplPushData(rplPeekData(1));            // LEAVE C AND F(C) ON THE STACK
        if(Exceptions) { DSTop=dstkptr;  return; }
        rplEvalUserFunc(ARG_USERFUNC,CMD_OVR_NUM);
        if(Exceptions) { DSTop=dstkptr;  return; }

        rplReadNumberAsReal(rplPeekData(1),&fc);

        if((fc.flags&F_NEGATIVE)==(aff&F_NEGATIVE)) {
            // DISCARD POINT A

            // STACK HERE HAS A,B,F(A),F(B),C,F(C)
            rplOverwriteData(4,rplPeekData(1));
            rplOverwriteData(6,rplPeekData(2));
        }
        else {
            // DISCARD POINT B

            // STACK HERE HAS A,B,F(A),F(B),C,F(C)
            rplOverwriteData(3,rplPeekData(1));
            rplOverwriteData(5,rplPeekData(2));
        }
        rplDropData(2);

        } while(1);

        // HERE THE REALS a,b,fa,fb ARE VALID, and RReg[0]=b-a
        // AND IS GUARANTEED THAT B-A < TOLERANCE

        // RETURN THE BEST-GUESS BY INTERPOLATION

        if(iszeroReal(&fa)) {
            // WE ALREADY HAVE A ROOT, RETURN IT
            rplNewRealPush(&a);
            ARG_USERFUNC=rplPeekData(1);    // OVERWRITE FIRST ARGUMENT WITH THE RESULT
            DSTop=dstkptr-3;    // RESTORE AND DROP 3 ARGUMENTS
            return;
        }
        if(iszeroReal(&fb)) {
            // WE ALREADY HAVE A ROOT, RETURN IT
            rplNewRealPush(&b);
            ARG_USERFUNC=rplPeekData(1);    // OVERWRITE FIRST ARGUMENT WITH THE RESULT
            DSTop=dstkptr-3;    // RESTORE AND DROP 3 ARGUMENTS
            return;
        }

        fa.flags^=F_NEGATIVE;
        mulReal(&RReg[1],&RReg[0],&fa); // (b-a)*(0-fa)
        addReal(&RReg[2],&fb,&fa);      // (fb-fa)
        divReal(&RReg[3],&RReg[1],&RReg[2]); // (b-a)*(0-fa)/(fb-fa)
        addReal(&RReg[0],&a,&RReg[3]);  // c=a+(b-a)*(0-fa)/(fb-fa)
        rplNewRealFromRRegPush(0);
        ARG_USERFUNC=rplPeekData(1);    // OVERWRITE FIRST ARGUMENT WITH THE RESULT

        DSTop=dstkptr-3;    // RESTORE AND DROP 3 ARGUMENTS

        return;
    }



    case MSOLVE:
    {
        //@SHORT_DESC=Multiple non-linear equation solver/optimization search
        //@INCOMPAT
        // NUMERIC OPTIMIZATION BASED ON NELDER-MEAD SIMPLEX METHOD
        // TAKES A LIST OF EQUATIONS OR ONE PROGRAM FROM THE STACK
        // A LIST OF VARIABLE NAMES
        // A LIST WITH MINIMUM VALUES FOR ALL VARIABLES
        // A LIST WITH MAXIMUM VALUES FOR ALL VARIABLES
        // TOLERANCE

        // WHEN A LIST OF EQUATIONS IS GIVEN, EQUATIONS WILL BE:
        // A) EXPRESSIONS --> WILL SOLVE FOR MINIMUM OF (EXPRESSION)^2
        // B) EQUATIONS --> WILL SOLVE FOR MINIMUM OF (LEFT-RIGHT)^2
        // C) INEQUALITIES --> WILL SOLVE FOR MINIMUM OF (1/T-1) WHERE T=TRUE/FALSE RESULT OF THE INEQUALITY (+Inf when result is false, 0 when true)

        // WHEN A PROGRAM IS GIVEN: THE PROGRAM WILL TAKE N VALUES FROM THE STACK, CORRESPONDING TO THE LIST OF VARIABLES AND RETURN A SINGLE SCALAR

        // THE EQUIVALENT PROGRAM OF A LIST OF EQUATIONS IS THE SUM OF THE SQUARES OF ALL EXPRESSIONS AND INEQUALITIES

        // ALGORITHM WILL RETURN:
        // A LIST WITH THE BEST SET OF VALUES FOR ALL THE VARIABLES, AT A POINT THAT MINIMIZES THE VALUE GIVEN BY THE PROGRAM OR THE SUM OF THE SQUARES OF ALL EQUATIONS
        // A LIST WITH THE RESIDUAL VALUE OF ALL EXPRESSIONS AFTER REPLACING THE SET OF VALUES


        if(rplDepthData()<5) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISPROGRAM(*rplPeekData(5)) && !ISSYMBOLIC(*rplPeekData(5)) && !ISLIST(*rplPeekData(5))) {
            rplError(ERR_LISTOFEQEXPECTED);
            return;
        }

        if(!ISLIST(*rplPeekData(4))) {
            rplError(ERR_INVALIDLISTOFVARS);
            return;
        }

        if(!ISLIST(*rplPeekData(3))|| !ISLIST(*rplPeekData(2))) {
            rplError(ERR_INVALIDVARRANGE);
            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        WORDPTR *listmin=DSTop-3;
        WORDPTR *listmax=DSTop-2;
        WORDPTR *listofeq=DSTop-5;
        WORDPTR *listofvars=DSTop-4;
        WORDPTR *toler=DSTop-1;
        WORDPTR *stksave=DSTop,*pointarray=DSTop;


        BINT nvars=rplListLength(*listofvars),i,j;

        if( (rplListLength(*listmin)!=nvars) || (rplListLength(*listmax)!=nvars)) {
            rplError(ERR_INVALIDVARRANGE);
            return;
        }

        WORDPTR vptr=rplGetListElement(*listofvars,1),minptr=rplGetListElement(*listmin,1),maxptr=rplGetListElement(*listmax,1);

        // CHECK THAT ALL THE VARIABLES ARE IDENTS
        for(j=0;j<nvars;++j) {
            if(!ISIDENT(*vptr)) {
                rplError(ERR_INVALIDLISTOFVARS);
                return;
            }
            if(!ISNUMBER(*minptr) || !ISNUMBER(*maxptr)) {
                rplError(ERR_INVALIDVARRANGE);
                return;
            }
            vptr=rplSkipOb(vptr);
            minptr=rplSkipOb(minptr);
            maxptr=rplSkipOb(maxptr);
        }

        // WE HAVE ALL VALID VARIABLE NAMES AND RANGE

        // BASIC ALGORITHM:

        // PHASE 1
        // 1.1 - GENERATE N+1 N-TUPLES WITHIN THE GIVEN VARIABLE RANGES, COMPUTE THE SCALAR VALUE, ADDED AT THE END OF THE N-TUPLE { X1 ... XN F(X) }, DISCARD ANY POINTS THAT PRODUCE +Inf

        REAL x1,x2,fx,tolerance;
        BINT tries,maxtries;
        maxtries=2*nvars;  // 2 BITS PER VARIABLE INDICATING STATE: 0=ORIGINAL, 1=MID-POINT, 2=CLOSE-QUARTER, 3=FAR QUARTER
        if(maxtries>8) maxtries=1<<8;
        else maxtries=1<<maxtries;


        for(j=0;j<=nvars;++j)
        {

            tries=0;
            while(tries<maxtries) {

            for(i=1;i<=nvars;++i) {
                // EXTRACT A VECTOR USING A MIX OF MINIMUM AND MAXIMUM COORDINATES, THE INITIAL CLOUD WIL COVER ROUGHLY HALF OF THE GIVEN AREA
                switch( (tries>>(2*(i-1)))&3) {
                case 0:
                default:
                    // USE EITHER MIN. OR MAX POINT
                    if(i<=j) rplPushData(rplGetListElement(*listmin,i));
                    else rplPushData(rplGetListElement(*listmax,i));
                    if(Exceptions) { DSTop=stksave; return; }
                    break;
                case 1:
                    // USE QUARTER POINT CLOSER TO MIN OR MAX DEPENDING ON j
                    rplReadNumberAsReal(rplGetListElement(*listmin,i),&x1);
                    rplReadNumberAsReal(rplGetListElement(*listmax,i),&x2);

                    if(i>j) swapReal(&x1,&x2);

                    addReal(&RReg[1],&x1,&x1);
                    addReal(&RReg[2],&x1,&RReg[1]);
                    addReal(&RReg[0],&RReg[2],&x2);     // 3*X1+X2

                    newRealFromBINT(&RReg[1],25,-2); // RReg[1]=0.25
                    mulReal(&RReg[2],&RReg[1],&RReg[0]);    // (3*x1+x2)/4
                    rplNewRealFromRRegPush(2);
                    if(Exceptions) { DSTop=stksave; return; }
                    break;

                case 2:
                    // USE QUARTER POINT FARTHER FROM MIN OR MAX DEPENDING ON j
                    rplReadNumberAsReal(rplGetListElement(*listmin,i),&x1);
                    rplReadNumberAsReal(rplGetListElement(*listmax,i),&x2);

                    if(i<=j) swapReal(&x1,&x2);

                    addReal(&RReg[1],&x1,&x1);
                    addReal(&RReg[2],&x1,&RReg[1]);
                    addReal(&RReg[0],&RReg[2],&x2);     // 3*X1+X2

                    newRealFromBINT(&RReg[1],25,-2); // RReg[1]=0.25
                    mulReal(&RReg[2],&RReg[1],&RReg[0]);    // (3*x1+x2)/4
                    rplNewRealFromRRegPush(2);
                    if(Exceptions) { DSTop=stksave; return; }
                    break;
                case 3:
                    // USE MID POINT BETWEEN MIN AND MAX
                    rplReadNumberAsReal(rplGetListElement(*listmin,i),&x1);
                    rplReadNumberAsReal(rplGetListElement(*listmax,i),&x2);

                    addReal(&RReg[0],&x1,&x2);
                    newRealFromBINT(&RReg[1],5,-1); // RReg[1]=0.5
                    mulReal(&RReg[2],&RReg[1],&RReg[0]);    // (x1+x2)/2
                    rplNewRealFromRRegPush(2);
                    if(Exceptions) { DSTop=stksave; return; }
                    break;


                }
            }



            rplEvalMultiUserFunc(listofeq,listofvars,nvars,1);    // EVALUATE THE EXPRESSION TO MINIMIZE, PUSHES THE VALUE ON THE STACK, LEAVES THE VECTOR IN IT
            if(Exceptions) { DSTop=stksave; return; }

            rplReadNumberAsReal(rplPeekData(1),&fx);

            if(!isNANorinfiniteReal(&fx)) break;

            // THE FUNCTION HAD NO VALUE AT THIS POINT, LET'S PICK A DIFFERENT POINT
            rplDropData(nvars+1);
            ++tries;
            }

            if(tries>=maxtries) {
                rplError(ERR_TRYDIFFERENTRANGE);
                DSTop=stksave; return;
                break;
            }



            //************************* DEBUG ONLY ***************************
            /*
            char Buffer0[1000],Buffer1[1000],Buffer2[1000];
            rplReadNumberAsReal(rplPeekData(3),&x1);
            rplReadNumberAsReal(rplPeekData(2),&x2);
            rplReadNumberAsReal(rplPeekData(1),&fx);
            *formatReal(&x1,Buffer0,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&x2,Buffer1,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&fx,Buffer2,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;

            printf("INI: X=%s     , Y=%s       , F(P)=%s\n",Buffer0,Buffer1,Buffer2); fflush(stdout);
            */
            //************************* DEBUG ONLY ***************************



            // NOW CREATE THE POINT VECTOR
            WORDPTR newpoint=rplCreateListN(nvars+1,1,1);
            if(Exceptions) { DSTop=stksave; return; }

            rplPushData(newpoint);

        }

        // HERE WE HAVE N+1 POINTS AS LISTS { X1 ... XN F(X) }

        // 1.2 - SORT THE POINTS FROM LOW TO HIGH F(X), HIGHEST F(X) AT STACK LEVEL 1

        for(i=1;i<=nvars;++i) {
            for(j=nvars;j>=i;--j) {
                rplReadNumberAsReal(rplGetListElement(rplPeekData(j),nvars+1),&x1);
                rplReadNumberAsReal(rplGetListElement(rplPeekData(j+1),nvars+1),&x2);
                if(ltReal(&x1,&x2)) {
                    // SWAP
                    WORDPTR tmp=rplPeekData(j+1);
                    rplOverwriteData(j+1,rplPeekData(j));
                    rplOverwriteData(j,tmp);
                }
            }
        }

#define USE_ORIGINAL_SIMPLEX_METHOD 0

#if USE_ORIGINAL_SIMPLEX_METHOD
        // MAIN LOOP:
        BINT loopcount=1;
        do {

        //printf("Pass=%d\n",loopcount);
        //loopcount++;

        // 1.3 - CALCULATE CENTROID OF ALL POINTS EXCEPT THE WORST - C
        BINT weight=1;
        rplPushDataNoGrow(rplPeekData(2));
        // STARTING WEIGHT IS 1
        for(j=4;j<=nvars+2;++j) {
            rplPushData(rplPeekData(j));
            if(Exceptions) { DSTop=stksave; return; }

            weight+=j-2;
            //++weight;
            rplNewBINTPush(j-2,DECBINT);
            if(Exceptions) { DSTop=stksave; return; }
            rplRunAtomic(CMD_OVR_MUL);
            if(Exceptions) { DSTop=stksave; return; }
            rplRunAtomic(CMD_OVR_ADD);
            if(Exceptions) { DSTop=stksave; return; }
        }
        rplNewBINTPush(weight,DECBINT);
        rplRunAtomic(CMD_OVR_DIV);
        if(Exceptions) { DSTop=stksave; return; }

        // 1.4 - EXIT WHEN EVERY COMPONENT OF (X0-C) < TOLERANCE

        rplReadNumberAsReal(*toler,&tolerance);
        tolerance.flags&=~F_NEGATIVE;

        // PRIME CRITERIA: THERE'S NO PROGRESS BETWEEN WORST AND BEST POINTS
        rplReadNumberAsReal(rplGetListElement(pointarray[0],nvars+1),&x1);
        rplReadNumberAsReal(rplGetListElement(pointarray[nvars],nvars+1),&x2);

        subReal(&RReg[0],&x1,&x2);
        RReg[0].flags&=~F_NEGATIVE;
        RReg[0].exp+=3; // COMPARE WITH TOLERANCE/1000

        // BREAK IF THERE'S NO PROGRESS BUT NOT IF WE ARE CLOSE TO ZERO (GUARANTEED GLOBAL MINIMUM)
        if(lteReal(&RReg[0],&tolerance) && (intdigitsReal(&x1)>intdigitsReal(&tolerance))) break;  // WITHIN TOLERANCE, EXIT MAIN LOOP


        if(nvars>=2) {
            // GENERAL CASE, CHECK THE DISTANCE BETWEEN THE CENTROID AND THE BEST POINT
        for(j=1;j<=nvars;++j)
        {
            rplReadNumberAsReal(rplGetListElement(rplPeekData(nvars+2),j),&x1);
            rplReadNumberAsReal(rplGetListElement(rplPeekData(1),j),&x2);
            subReal(&RReg[0],&x1,&x2);
            RReg[0].flags&=~F_NEGATIVE;

            if(gtReal(&RReg[0],&tolerance)) break;  // OUT OF TOLERANCE, NO NEED TO CHECK ANYMORE, KEEP WORKING
        }
        if(j>nvars) break;  // ALL COMPONENTS WITHIN TOLERANCE, BREAK OUT OF THE MAIN LOOP
        }
        else {
            // SPECIAL CASE FOR SINGLE-VARIABLE PROBLEMS
            // CHECK THE DISTANCE BETWEEN THE BEST AND WORST POINTS
            rplReadNumberAsReal(rplGetListElement(rplPeekData(2),1),&x1);
            rplReadNumberAsReal(rplGetListElement(rplPeekData(1),1),&x2);
            subReal(&RReg[0],&x1,&x2);
            RReg[0].flags&=~F_NEGATIVE;

            if(lteReal(&RReg[0],&tolerance)) break;  // WITHIN TOLERANCE, EXIT MAIN LOOP


        }

        // 1.5 - CHOOSE NEW POINT BY REFLECTION OF THE WORST POINT P = C+(C-W)


        for(j=1;j<=nvars;++j)
        {
            rplReadNumberAsReal(rplGetListElement(pointarray[nvars+1],j),&x1);   // C(j)
            rplReadNumberAsReal(rplGetListElement(pointarray[nvars],j),&x2);   // W(j)

            addReal(&RReg[0],&x1,&x1);
            subReal(&RReg[1],&RReg[0],&x2); // P(j)=2*C(j)-W(j)
            rplNewRealFromRRegPush(1);
            if(Exceptions) { DSTop=stksave; return; }
        }

        // 1.6 - CALCULATE SCALAR VALUE AT P:

        rplEvalMultiUserFunc(listofeq,listofvars,nvars,1);    // EVALUATE THE EXPRESSION TO MINIMIZE, PUSHES THE VALUE ON THE STACK, LEAVES THE VECTOR IN IT
        if(Exceptions) { DSTop=stksave; return; }


        //                                    IF F(P) IS BETWEEN F(X0) AND F(XN), THEN ACCEPT P, REPLACE X(N+1) AND LOOP

        rplReadNumberAsReal(rplGetListElement(pointarray[0],nvars+1),&x1);   // F(X0)
        rplReadNumberAsReal(rplGetListElement(pointarray[nvars-1],nvars+1),&x2);   // F(Xn)
        rplReadNumberAsReal(rplPeekData(1),&fx);  // F(P)


        if(gteReal(&fx,&x1) && ltReal(&fx,&x2)) {
            // REPLACE POINT AND CONTINUE
            /*
            //************************* DEBUG ONLY ***************************
            char Buffer0[1000],Buffer1[1000],Buffer2[1000];
            rplReadNumberAsReal(rplPeekData(3),&x1);
            rplReadNumberAsReal(rplPeekData(2),&x2);
            rplReadNumberAsReal(rplPeekData(1),&fx);
            *formatReal(&x1,Buffer0,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&x2,Buffer1,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&fx,Buffer2,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;

            printf("REF: X=%s     , Y=%s       , F(P)=%s\n",Buffer0,Buffer1,Buffer2); fflush(stdout);
            //************************* DEBUG ONLY ***************************
            */

            WORDPTR newpt=rplCreateListN(nvars+1,1,1);
            if(!newpt) { DSTop=stksave; return; }
            // UPDATE SINCE IT COULD'VE MOVED
            rplReadNumberAsReal(rplGetListElement(newpt,nvars+1),&fx);  // F(P)

            // INSERT IN THE PROPER PLACE IN THE LIST
            for(j=nvars-1;j>=0;--j) {
                rplReadNumberAsReal(rplGetListElement(pointarray[j],nvars+1),&x1);   // F(Xj)
                if(gtReal(&fx,&x1)) { pointarray[j+1]=newpt; break; }
                else pointarray[j+1]=pointarray[j];
            }
            if(j<0) pointarray[0]=newpt;


            rplDropData(1); // DROP CENTROID
            continue;

        }

        //                                    IF F(P)<F(X0) --> GET ANOTHER P' AT TWICE THE DISTANCE FROM C IN THE SAME DIRECTION: P'=P+(P-C), IF F(P')<F(P) USE P', OTHERWISE USE P, REPLACE AND LOOP

        if(ltReal(&fx,&x1)) {

            // P'=2P-C
            for(j=1;j<=nvars;++j)
            {
                rplReadNumberAsReal(rplGetListElement(pointarray[nvars+1],j),&x2);   // C(j)
                rplReadNumberAsReal(rplPeekData(nvars+1),&x1);   // P(j)

                addReal(&RReg[0],&x1,&x1);
                subReal(&RReg[1],&RReg[0],&x2); // P'(j)=2*P(j)-C(j)
                rplNewRealFromRRegPush(1);
                if(Exceptions) { DSTop=stksave; return; }
            }

            // F(P')
            rplEvalMultiUserFunc(listofeq,listofvars,nvars,1);    // EVALUATE THE EXPRESSION TO MINIMIZE, PUSHES THE VALUE ON THE STACK, LEAVES THE VECTOR IN IT
            if(Exceptions) { DSTop=stksave; return; }

            rplReadNumberAsReal(rplPeekData(1),&fx);  // F(P')
            rplReadNumberAsReal(rplPeekData(nvars+2),&x1);  // F(P)

            if(ltReal(&fx,&x1)) rplRemoveAtData(nvars+2,nvars+1);   // KEEP P'
            else rplDropData(nvars+1);  // OR KEEP P

            // NOW REPLACE WORST POINT WITH P

            /*
            //************************* DEBUG ONLY ***************************
            char Buffer0[1000],Buffer1[1000],Buffer2[1000];
            rplReadNumberAsReal(rplPeekData(3),&x1);
            rplReadNumberAsReal(rplPeekData(2),&x2);
            rplReadNumberAsReal(rplPeekData(1),&fx);
            *formatReal(&x1,Buffer0,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&x2,Buffer1,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&fx,Buffer2,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;

            printf("EXP: X=%s     , Y=%s       , F(P)=%s\n",Buffer0,Buffer1,Buffer2); fflush(stdout);
            //************************* DEBUG ONLY ***************************
            */
            // REPLACE POINT AND CONTINUE
            WORDPTR newpt=rplCreateListN(nvars+1,1,1);
            if(!newpt) { DSTop=stksave; return; }
            // UPDATE SINCE IT COULD'VE MOVED
            rplReadNumberAsReal(rplGetListElement(newpt,nvars+1),&fx);  // F(P)

            // INSERT IN THE PROPER PLACE IN THE LIST
            for(j=nvars-1;j>=0;--j) {
                rplReadNumberAsReal(rplGetListElement(pointarray[j],nvars+1),&x1);   // F(Xj)
                if(gtReal(&fx,&x1)) { pointarray[j+1]=newpt; break; }
                else pointarray[j+1]=pointarray[j];
            }
            if(j<0) pointarray[0]=newpt;

            rplDropData(1);     // DROP CENTROID

            continue;

        }

        //                                    IF F(P)>=F(XN) (NOT N+1)! --> GET ANOTHER P''= C + (W-C)/2 BETWEEN THE CENTROID AND WORST POINT, IF F(P'')<F(XN+1) USE P'', REPLACE AND LOOP
        //                                                                 ELSE OF F(P'') IS NO BETTER THAN THE WORST POINT, SHRINK ALL POINTS TOWARDS THE BEST
        //                                                                 Xi=X0+(Xi-X0)/2

        // IF WE ARE HERE, IT IS GUARANTEED THAT F(P)>=F(XN)

        // P''=(C+W)/2
        for(j=1;j<=nvars;++j)
        {
            rplReadNumberAsReal(rplGetListElement(pointarray[nvars+1],j),&x2);   // C(j)
            rplReadNumberAsReal(rplGetListElement(pointarray[nvars],j),&x1);   // W(j)

            addReal(&RReg[0],&x1,&x2);
            newRealFromBINT(&RReg[2],5,-1);
            mulReal(&RReg[1],&RReg[0],&RReg[2]);    //  (C(j)+W(j))/2
            rplNewRealFromRRegPush(1);
            if(Exceptions) { DSTop=stksave; return; }
        }

        // F(P'')
        rplEvalMultiUserFunc(listofeq,listofvars,nvars,1);    // EVALUATE THE EXPRESSION TO MINIMIZE, PUSHES THE VALUE ON THE STACK, LEAVES THE VECTOR IN IT
        if(Exceptions) { DSTop=stksave; return; }

        rplReadNumberAsReal(rplPeekData(1),&fx);  // F(P'')
        rplReadNumberAsReal(rplGetListElement(pointarray[nvars],nvars+1),&x1);  // F(W)

        if(ltReal(&fx,&x1)) {

            rplRemoveAtData(nvars+2,nvars+1);   // KEEP P''

            /*
            //************************* DEBUG ONLY ***************************
            char Buffer0[1000],Buffer1[1000],Buffer2[1000];
            rplReadNumberAsReal(rplPeekData(3),&x1);
            rplReadNumberAsReal(rplPeekData(2),&x2);
            rplReadNumberAsReal(rplPeekData(1),&fx);
            *formatReal(&x1,Buffer0,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&x2,Buffer1,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&fx,Buffer2,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;

            printf("CON: X=%s     , Y=%s       , F(P)=%s\n",Buffer0,Buffer1,Buffer2); fflush(stdout);
            //************************* DEBUG ONLY ***************************
            */



            // REPLACE POINT AND CONTINUE
            WORDPTR newpt=rplCreateListN(nvars+1,1,1);
            if(!newpt) { DSTop=stksave; return; }
            // UPDATE SINCE IT COULD'VE MOVED
            rplReadNumberAsReal(rplGetListElement(newpt,nvars+1),&fx);  // F(P)

            // INSERT IN THE PROPER PLACE IN THE LIST
            for(j=nvars-1;j>=0;--j) {
                rplReadNumberAsReal(rplGetListElement(pointarray[j],nvars+1),&x1);   // F(Xj)
                if(gtReal(&fx,&x1)) { pointarray[j+1]=newpt; break; }
                else pointarray[j+1]=pointarray[j];
            }
            if(j<0) pointarray[0]=newpt;

            rplDropData(1);
            continue;


        }

        // P'' IS WORSE, WE ARE GOING NOWHERE. SHRINK ENTIRE GROUP OF POINTS TOWARDS THE BEST VALUE

        rplDropData((nvars+1)*2+1); // DROP P(i), F(P), P'', F(P'') AND THE C POINT LIST

        for(j=1;j<=nvars;++j) {
            newRealFromBINT(&RReg[4],5,-1); // 1/2

            for(i=1;i<=nvars;++i) {
            rplReadNumberAsReal(rplGetListElement(pointarray[0],i),&x1);    // X0(i)
            rplReadNumberAsReal(rplGetListElement(pointarray[j],i),&x2);    // Xj(i)

            addReal(&RReg[0],&x1,&x2);
            mulReal(&RReg[1],&RReg[0],&RReg[4]);    // (X0(i)+Xj(i))/2

            rplNewRealFromRRegPush(1);
            if(Exceptions) { DSTop=stksave; return; }
            }

            rplEvalMultiUserFunc(listofeq,listofvars,nvars,1);    // EVALUATE THE EXPRESSION TO MINIMIZE, PUSHES THE VALUE ON THE STACK, LEAVES THE VECTOR IN IT
            if(Exceptions) { DSTop=stksave; return; }

            /*
            //************************* DEBUG ONLY ***************************
            char Buffer0[1000],Buffer1[1000],Buffer2[1000];
            rplReadNumberAsReal(rplPeekData(3),&x1);
            rplReadNumberAsReal(rplPeekData(2),&x2);
            rplReadNumberAsReal(rplPeekData(1),&fx);
            *formatReal(&x1,Buffer0,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&x2,Buffer1,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&fx,Buffer2,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;

            printf("SHR: X=%s     , Y=%s       , F(P)=%s\n",Buffer0,Buffer1,Buffer2); fflush(stdout);
            //************************* DEBUG ONLY ***************************
            */
            // REPLACE POINT
            WORDPTR newpt=rplCreateListN(nvars+1,1,1);
            if(!newpt) { DSTop=stksave; return; }

            pointarray[j]=newpt;        // REPLACE THE POINTS WITH THE NEW POINT


        }

        // SORT THE POINTS AGAIN
        // 1.2 - SORT THE POINTS FROM LOW TO HIGH F(X), HIGHEST F(X) AT STACK LEVEL 1

        for(i=1;i<=nvars;++i) {
            for(j=nvars;j>=i;--j) {
                rplReadNumberAsReal(rplGetListElement(rplPeekData(j),nvars+1),&x1);
                rplReadNumberAsReal(rplGetListElement(rplPeekData(j+1),nvars+1),&x2);
                if(ltReal(&x1,&x2)) {
                    // SWAP
                    WORDPTR tmp=rplPeekData(j+1);
                    rplOverwriteData(j+1,rplPeekData(j));
                    rplOverwriteData(j,tmp);
                }
            }
        }


        } while(1);

#else

        // PHAM IMPROVEMENT TO NELDER-MEAD METHOD - QUASI GRADIENTS

        // MAIN LOOP:
        BINT noprogress=0; //loopcount=1;
        do {

        //printf("Pass=%d\n",loopcount);
        //loopcount++;

        // 1.3 - CALCULATE CENTROID OF ALL POINTS EXCEPT THE WORST - C
        BINT weight=1;
        rplPushDataNoGrow(rplPeekData(2));
        // STARTING WEIGHT IS 1
        for(j=4;j<=nvars+2;++j) {
            rplPushData(rplPeekData(j));
            if(Exceptions) { DSTop=stksave; return; }

            weight+=j-2;
            //++weight;
            rplNewBINTPush(j-2,DECBINT);
            if(Exceptions) { DSTop=stksave; return; }
            rplRunAtomic(CMD_OVR_MUL);
            if(Exceptions) { DSTop=stksave; return; }
            rplRunAtomic(CMD_OVR_ADD);
            if(Exceptions) { DSTop=stksave; return; }
        }
        rplNewBINTPush(weight,DECBINT);
        rplRunAtomic(CMD_OVR_DIV);
        if(Exceptions) { DSTop=stksave; return; }

        // 1.4 - EXIT WHEN EVERY COMPONENT OF (X0-C) < TOLERANCE

        rplReadNumberAsReal(*toler,&tolerance);
        tolerance.flags&=~F_NEGATIVE;

        // PRIME CRITERIA: THERE'S NO PROGRESS BETWEEN WORST AND BEST POINTS
        rplReadNumberAsReal(rplGetListElement(pointarray[0],nvars+1),&x1);
        rplReadNumberAsReal(rplGetListElement(pointarray[nvars],nvars+1),&x2);

        subReal(&RReg[0],&x1,&x2);
        RReg[0].flags&=~F_NEGATIVE;
        RReg[0].exp+=3; // COMPARE WITH TOLERANCE/1000

        // BREAK IF THERE'S NO PROGRESS BUT NOT IF WE ARE CLOSE TO ZERO (GUARANTEED GLOBAL MINIMUM)
        if(lteReal(&RReg[0],&tolerance) && (intdigitsReal(&x1)>intdigitsReal(&tolerance))) {
            if(noprogress>3) break;  // WITHIN TOLERANCE, EXIT MAIN LOOP
            ++noprogress;
        } else noprogress=0;


        if(nvars>=2) {
            // GENERAL CASE, CHECK THE DISTANCE BETWEEN THE CENTROID AND THE BEST POINT
        for(j=1;j<=nvars;++j)
        {
            rplReadNumberAsReal(rplGetListElement(rplPeekData(nvars+2),j),&x1);
            rplReadNumberAsReal(rplGetListElement(rplPeekData(1),j),&x2);
            subReal(&RReg[0],&x1,&x2);
            RReg[0].flags&=~F_NEGATIVE;

            if(gtReal(&RReg[0],&tolerance)) break;  // OUT OF TOLERANCE, NO NEED TO CHECK ANYMORE, KEEP WORKING
        }
        if(j>nvars) break;  // ALL COMPONENTS WITHIN TOLERANCE, BREAK OUT OF THE MAIN LOOP
        }
        else {
            // SPECIAL CASE FOR SINGLE-VARIABLE PROBLEMS
            // CHECK THE DISTANCE BETWEEN THE BEST AND WORST POINTS
            rplReadNumberAsReal(rplGetListElement(rplPeekData(2),1),&x1);
            rplReadNumberAsReal(rplGetListElement(rplPeekData(1),1),&x2);
            subReal(&RReg[0],&x1,&x2);
            RReg[0].flags&=~F_NEGATIVE;

            if(lteReal(&RReg[0],&tolerance)) break;  // WITHIN TOLERANCE, EXIT MAIN LOOP


        }

        // 1.5 - CHOOSE NEW POINT BY REFLECTION OF THE WORST POINT P = C+(C-W)


        for(j=1;j<=nvars;++j)
        {
            rplReadNumberAsReal(rplGetListElement(pointarray[nvars+1],j),&x1);   // C(j)
            rplReadNumberAsReal(rplGetListElement(pointarray[nvars],j),&x2);   // W(j)

            addReal(&RReg[0],&x1,&x1);
            subReal(&RReg[1],&RReg[0],&x2); // P(j)=2*C(j)-W(j)
            rplNewRealFromRRegPush(1);
            if(Exceptions) { DSTop=stksave; return; }
        }

        // 1.6 - CALCULATE SCALAR VALUE AT P:

        rplEvalMultiUserFunc(listofeq,listofvars,nvars,1);    // EVALUATE THE EXPRESSION TO MINIMIZE, PUSHES THE VALUE ON THE STACK, LEAVES THE VECTOR IN IT
        if(Exceptions) { DSTop=stksave; return; }



        rplReadNumberAsReal(rplGetListElement(pointarray[0],nvars+1),&x1);   // F(X0)
        rplReadNumberAsReal(rplGetListElement(pointarray[nvars-1],nvars+1),&x2);   // F(Xn)
        rplReadNumberAsReal(rplPeekData(1),&fx);  // F(P)


        //   IF F(P)<F(X0) --> GET ANOTHER P' AT TWICE THE DISTANCE FROM C IN THE SAME DIRECTION: P'=P+(P-C), IF F(P')<F(P) USE P', OTHERWISE USE P, REPLACE AND LOOP

        if(ltReal(&fx,&x1)) {

            // P'=2P-C
            for(j=1;j<=nvars;++j)
            {
                rplReadNumberAsReal(rplGetListElement(pointarray[nvars+1],j),&x2);   // C(j)
                rplReadNumberAsReal(rplPeekData(nvars+1),&x1);   // P(j)

                addReal(&RReg[0],&x1,&x1);
                subReal(&RReg[1],&RReg[0],&x2); // P'(j)=2*P(j)-C(j)
                rplNewRealFromRRegPush(1);
                if(Exceptions) { DSTop=stksave; return; }
            }

            // F(P')
            rplEvalMultiUserFunc(listofeq,listofvars,nvars,1);    // EVALUATE THE EXPRESSION TO MINIMIZE, PUSHES THE VALUE ON THE STACK, LEAVES THE VECTOR IN IT
            if(Exceptions) { DSTop=stksave; return; }

            rplReadNumberAsReal(rplPeekData(1),&fx);  // F(P')
            rplReadNumberAsReal(rplPeekData(nvars+2),&x1);  // F(P)

            if(ltReal(&fx,&x1)) rplRemoveAtData(nvars+2,nvars+1);   // KEEP P'
            else rplDropData(nvars+1);  // OR KEEP P

            // NOW REPLACE WORST POINT WITH P


            //************************* DEBUG ONLY ***************************
            /*
            {
            char Buffer0[1000],Buffer1[1000],Buffer2[1000];
            rplReadNumberAsReal(rplPeekData(3),&x1);
            rplReadNumberAsReal(rplPeekData(2),&x2);
            rplReadNumberAsReal(rplPeekData(1),&fx);
            *formatReal(&x1,Buffer0,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&x2,Buffer1,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&fx,Buffer2,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;

            printf("EXP: X=%s     , Y=%s       , F(P)=%s\n",Buffer0,Buffer1,Buffer2); fflush(stdout);
            }
            */
            //************************* DEBUG ONLY ***************************

            // REPLACE POINT AND CONTINUE
            WORDPTR newpt=rplCreateListN(nvars+1,1,1);
            if(!newpt) { DSTop=stksave; return; }
            // UPDATE SINCE IT COULD'VE MOVED
            rplReadNumberAsReal(rplGetListElement(newpt,nvars+1),&fx);  // F(P)

            // INSERT IN THE PROPER PLACE IN THE LIST
            for(j=nvars-1;j>=0;--j) {
                rplReadNumberAsReal(rplGetListElement(pointarray[j],nvars+1),&x1);   // F(Xj)
                if(gtReal(&fx,&x1)) { pointarray[j+1]=newpt; break; }
                else pointarray[j+1]=pointarray[j];
            }
            if(j<0) pointarray[0]=newpt;

            rplDropData(1);     // DROP CENTROID

            continue;

        }



        //************************* DEBUG ONLY ***************************
        /*
        char Buffer0[1000],Buffer1[1000],Buffer2[1000];
        rplReadNumberAsReal(rplPeekData(3),&x1);
        rplReadNumberAsReal(rplPeekData(2),&x2);
        rplReadNumberAsReal(rplPeekData(1),&fx);
        *formatReal(&x1,Buffer0,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
        *formatReal(&x2,Buffer1,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
        *formatReal(&fx,Buffer2,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;

        printf("P??: X=%s     , Y=%s       , F(P)=%s\n",Buffer0,Buffer1,Buffer2); fflush(stdout);
        */
        //************************* DEBUG ONLY ***************************

        // REFLECTION DIDN'T IMPROVE ON OUR BEST POINT, FIND A POINT THROUGH GRADIENTS TO SEE IF WE CAN DO BETTER


                // 1.3 - GET A POINT WITH THE QUASI-GRADIENTS

                // PUT ALL THE COORDINATES FROM EACH POINT
                for(j=1;j<=nvars;++j) {
                    rplPushData(rplGetListElement(pointarray[j-1],j));
                    if(Exceptions) { DSTop=stksave; return; }
                }

                rplEvalMultiUserFunc(listofeq,listofvars,nvars,1);    // EVALUATE THE EXPRESSION TO MINIMIZE, PUSHES THE VALUE ON THE STACK, LEAVES THE VECTOR IN IT
                if(Exceptions) { DSTop=stksave; return; }


                rplReadNumberAsReal(rplPeekData(1),&fx);    // READ F(e)
                // COMPUTE THE QUASI-GRADIENTS

                rplZeroToRReg(8);   // KEEP THE MAX ABS. VALUE HERE
                for(j=1;j<=nvars;++j)
                {
                    rplReadNumberAsReal(rplPeekData(nvars+2-j),&x1);    // READ Xe(j)
                    rplReadNumberAsReal(rplGetListElement(pointarray[(j&1)? j:(j-2)],j),&x2); // READ Xj-1(j) or Xj+1(j)

                    subReal(&RReg[1],&x2,&x1);

                    if(!iszeroReal(&RReg[1])) {
                        rplReadNumberAsReal(rplGetListElement(pointarray[(j&1)? j:(j-2)],nvars+1),&x2); // READ F(Xj-1) or F(Xj+1)
                        subReal(&RReg[2],&x2,&fx);
                        divReal(&RReg[0],&RReg[2],&RReg[1]);    // dF/dXj= (F(Xj-1)-F(e))  /  (Xj-1(j) - Xe(j))
                    }
                    else rplZeroToRReg(0);  // USE ZERO GRADIENT IF DIVISION BY ZERO


                    WORDPTR newreal=rplNewRealFromRReg(0);
                    if(!newreal) { DSTop=stksave; return; }

                    rplOverwriteData(nvars+2-j,newreal);

                    RReg[0].flags&=~F_NEGATIVE;

                    if(gtReal(&RReg[0],&RReg[8])) {
                        swapReal(&RReg[0],&RReg[8]);  // SAVE THE MAXIMUM ABS. VALUE
                        swapReal(&RReg[1],&RReg[7]);  // AND SAVE THE DELTA THAT PRODUCED IT
                    }
                }

                if(!iszeroReal(&RReg[8])) {
                    RReg[7].flags&=~F_NEGATIVE;
                // NORMALIZE THE GRADIENT VECTOR TO THE COORDINATE WITH MAXIMUM ABSOLUTE VALUE
                    divReal(&RReg[2],&RReg[7],&RReg[8]);
                for(j=1;j<=nvars;++j)
                {
                    rplReadNumberAsReal(rplPeekData(nvars+2-j),&x1);
                    if(iszeroReal(&x1)) {
                        copyReal(&RReg[0],&RReg[2]);
                        RReg[0].exp-=3;                    // ADD DISTURBANCE TO BREAK THE CYCLE WHEN ALL POINTS ARE COLINEAR
                    }
                    else mulReal(&RReg[0],&x1,&RReg[2]);
                    WORDPTR newreal=rplNewRealFromRReg(0);
                    if(!newreal) { DSTop=stksave; return; }

                    rplOverwriteData(nvars+2-j,newreal);
                }
                }

                // HERE WE HAVE P, F(P), G

                // 1.5 - CHOOSE NEW POINT BY REFLECTION OF THE WORST POINT P = X0 - G


                for(j=1;j<=nvars;++j)
                {
                    rplReadNumberAsReal(rplGetListElement(pointarray[0],j),&x1);   // X0(j)
                    rplReadNumberAsReal(rplPeekData(nvars+2-j),&x2);   // G(j)

                    subReal(&RReg[0],&x1,&x2); // G'(j)=X0(j)-G(j)
                    WORDPTR newreal=rplNewRealFromRReg(0);
                    if(!newreal) { DSTop=stksave; return; }

                    rplOverwriteData(nvars+2-j,newreal);
                }

                rplDropData(1); // DROP THE OLD VALUE F(e)



                // 1.6 - CALCULATE SCALAR VALUE AT THE NEW POINT G'

                rplEvalMultiUserFunc(listofeq,listofvars,nvars,1);    // EVALUATE THE EXPRESSION TO MINIMIZE, PUSHES THE VALUE ON THE STACK, LEAVES THE VECTOR IN IT
                if(Exceptions) { DSTop=stksave; return; }


                //************************* DEBUG ONLY ***************************
                /*
                {
                char Buffer0[1000],Buffer1[1000],Buffer2[1000];
                rplReadNumberAsReal(rplPeekData(3),&x1);
                rplReadNumberAsReal(rplPeekData(2),&x2);
                rplReadNumberAsReal(rplPeekData(1),&fx);
                *formatReal(&x1,Buffer0,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
                *formatReal(&x2,Buffer1,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
                *formatReal(&fx,Buffer2,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;

                printf("GR?: X=%s     , Y=%s       , F(P)=%s\n",Buffer0,Buffer1,Buffer2); fflush(stdout);
                }
                */
                //************************* DEBUG ONLY ***************************

                rplReadNumberAsReal(rplPeekData(1),&x1);        // F(G')
                rplReadNumberAsReal(rplPeekData(nvars+2),&fx);  // F(P)
                rplReadNumberAsReal(rplGetListElement(pointarray[nvars-1],nvars+1),&x2);   // F(Xn)

                if(ltReal(&x1,&x2)) {
                    // F(G')<F(P) --> WE FOUND A BETTER POINT, KEEP IT
                    rplRemoveAtData(nvars+2,nvars+1);
                    swapReal(&x1,&fx);
                    //printf("Chose Gradient\n");
                }
                else rplDropData(nvars+1);  // OTHERWISE KEEP THE ORIGINAL POINT P


        //  IF F(P)< F(XN), THEN ACCEPT P, REPLACE X(N+1) AND LOOP

        if(ltReal(&fx,&x2)) {
            // REPLACE POINT AND CONTINUE

            //************************* DEBUG ONLY ***************************
            /*
            char Buffer0[1000],Buffer1[1000],Buffer2[1000];
            rplReadNumberAsReal(rplPeekData(3),&x1);
            rplReadNumberAsReal(rplPeekData(2),&x2);
            rplReadNumberAsReal(rplPeekData(1),&fx);
            *formatReal(&x1,Buffer0,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&x2,Buffer1,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&fx,Buffer2,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;

            printf("REF: X=%s     , Y=%s       , F(P)=%s\n",Buffer0,Buffer1,Buffer2); fflush(stdout);
            */
            //************************* DEBUG ONLY ***************************


            WORDPTR newpt=rplCreateListN(nvars+1,1,1);
            if(!newpt) { DSTop=stksave; return; }
            // UPDATE SINCE IT COULD'VE MOVED
            rplReadNumberAsReal(rplGetListElement(newpt,nvars+1),&fx);  // F(P)

            // INSERT IN THE PROPER PLACE IN THE LIST
            for(j=nvars-1;j>=0;--j) {
                rplReadNumberAsReal(rplGetListElement(pointarray[j],nvars+1),&x1);   // F(Xj)
                if(gtReal(&fx,&x1)) { pointarray[j+1]=newpt; break; }
                else pointarray[j+1]=pointarray[j];
            }
            if(j<0) pointarray[0]=newpt;


            rplDropData(1); // DROP CENTROID
            continue;

        }


        //                                    IF F(P)>=F(XN) (NOT N+1)! --> GET ANOTHER P''= C + (W-C)/2 BETWEEN THE CENTROID AND WORST POINT, IF F(P'')<F(XN+1) USE P'', REPLACE AND LOOP
        //                                                                 ELSE OF F(P'') IS NO BETTER THAN THE WORST POINT, SHRINK ALL POINTS TOWARDS THE BEST
        //                                                                 Xi=X0+(Xi-X0)/2

        // IF WE ARE HERE, IT IS GUARANTEED THAT F(P)>=F(XN)

        // P''=(C+W)/2
        for(j=1;j<=nvars;++j)
        {
            rplReadNumberAsReal(rplGetListElement(pointarray[nvars+1],j),&x2);   // C(j)
            rplReadNumberAsReal(rplGetListElement(pointarray[nvars],j),&x1);   // W(j)

            addReal(&RReg[0],&x1,&x2);
            newRealFromBINT(&RReg[2],5,-1);
            mulReal(&RReg[1],&RReg[0],&RReg[2]);    //  (C(j)+W(j))/2
            rplNewRealFromRRegPush(1);
            if(Exceptions) { DSTop=stksave; return; }
        }

        // F(P'')
        rplEvalMultiUserFunc(listofeq,listofvars,nvars,1);    // EVALUATE THE EXPRESSION TO MINIMIZE, PUSHES THE VALUE ON THE STACK, LEAVES THE VECTOR IN IT
        if(Exceptions) { DSTop=stksave; return; }

        rplReadNumberAsReal(rplPeekData(1),&fx);  // F(P'')
        rplReadNumberAsReal(rplGetListElement(pointarray[nvars],nvars+1),&x1);  // F(W)

        if(ltReal(&fx,&x1)) {

            rplRemoveAtData(nvars+2,nvars+1);   // KEEP P''


            //************************* DEBUG ONLY ***************************
            /*
            char Buffer0[1000],Buffer1[1000],Buffer2[1000];
            rplReadNumberAsReal(rplPeekData(3),&x1);
            rplReadNumberAsReal(rplPeekData(2),&x2);
            rplReadNumberAsReal(rplPeekData(1),&fx);
            *formatReal(&x1,Buffer0,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&x2,Buffer1,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&fx,Buffer2,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;

            printf("CON: X=%s     , Y=%s       , F(P)=%s\n",Buffer0,Buffer1,Buffer2); fflush(stdout);
            */
            //************************* DEBUG ONLY ***************************




            // REPLACE POINT AND CONTINUE
            WORDPTR newpt=rplCreateListN(nvars+1,1,1);
            if(!newpt) { DSTop=stksave; return; }
            // UPDATE SINCE IT COULD'VE MOVED
            rplReadNumberAsReal(rplGetListElement(newpt,nvars+1),&fx);  // F(P)

            // INSERT IN THE PROPER PLACE IN THE LIST
            for(j=nvars-1;j>=0;--j) {
                rplReadNumberAsReal(rplGetListElement(pointarray[j],nvars+1),&x1);   // F(Xj)
                if(gtReal(&fx,&x1)) { pointarray[j+1]=newpt; break; }
                else pointarray[j+1]=pointarray[j];
            }
            if(j<0) pointarray[0]=newpt;

            rplDropData(1);
            continue;


        }

        // P'' IS WORSE, WE ARE GOING NOWHERE. SHRINK ENTIRE GROUP OF POINTS TOWARDS THE BEST VALUE

        rplDropData((nvars+1)*2+1); // DROP P(i), F(P), P'', F(P'') AND THE C POINT LIST

        for(j=1;j<=nvars;++j) {
            newRealFromBINT(&RReg[4],5,-1); // 1/2

            for(i=1;i<=nvars;++i) {
            rplReadNumberAsReal(rplGetListElement(pointarray[0],i),&x1);    // X0(i)
            rplReadNumberAsReal(rplGetListElement(pointarray[j],i),&x2);    // Xj(i)

            addReal(&RReg[0],&x1,&x2);
            mulReal(&RReg[1],&RReg[0],&RReg[4]);    // (X0(i)+Xj(i))/2

            rplNewRealFromRRegPush(1);
            if(Exceptions) { DSTop=stksave; return; }
            }

            rplEvalMultiUserFunc(listofeq,listofvars,nvars,1);    // EVALUATE THE EXPRESSION TO MINIMIZE, PUSHES THE VALUE ON THE STACK, LEAVES THE VECTOR IN IT
            if(Exceptions) { DSTop=stksave; return; }


            //************************* DEBUG ONLY ***************************
            /*
            char Buffer0[1000],Buffer1[1000],Buffer2[1000];
            rplReadNumberAsReal(rplPeekData(3),&x1);
            rplReadNumberAsReal(rplPeekData(2),&x2);
            rplReadNumberAsReal(rplPeekData(1),&fx);
            *formatReal(&x1,Buffer0,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&x2,Buffer1,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;
            *formatReal(&fx,Buffer2,FMT_CODE,MAKELOCALE('.',' ',' ',','))=0;

            printf("SHR: X=%s     , Y=%s       , F(P)=%s\n",Buffer0,Buffer1,Buffer2); fflush(stdout);
            */
            //************************* DEBUG ONLY ***************************

            // REPLACE POINT
            WORDPTR newpt=rplCreateListN(nvars+1,1,1);
            if(!newpt) { DSTop=stksave; return; }

            pointarray[j]=newpt;        // REPLACE THE POINTS WITH THE NEW POINT


        }

        // SORT THE POINTS AGAIN
        // 1.2 - SORT THE POINTS FROM LOW TO HIGH F(X), HIGHEST F(X) AT STACK LEVEL 1

        for(i=1;i<=nvars;++i) {
            for(j=nvars;j>=i;--j) {
                rplReadNumberAsReal(rplGetListElement(rplPeekData(j),nvars+1),&x1);
                rplReadNumberAsReal(rplGetListElement(rplPeekData(j+1),nvars+1),&x2);
                if(ltReal(&x1,&x2)) {
                    // SWAP
                    WORDPTR tmp=rplPeekData(j+1);
                    rplOverwriteData(j+1,rplPeekData(j));
                    rplOverwriteData(j,tmp);
                }
            }
        }


        } while(1);







#endif





        // HERE WE HAVE A SOLUTION!!
        //printf("Solution @ Pass=%d",loopcount);
        // IN THE STACK WE HAVE ORIGINAL 4 ARGUMENTS
        // THEN N+1 POINTS AND THEIR CENTROID

        rplDropData(nvars+1);       // DROP THE CENTROID AND ALL POINTS EXCEPT THE BEST POINT, THIS IS OUR SOLUTION
        rplExplodeList2(rplPeekData(1));
        if(Exceptions) { DSTop=stksave; return; }
        rplDropData(1);
        WORDPTR newlist=rplCreateListN(nvars,1,0);
        if(!newlist) { DSTop=stksave; return; }

        rplOverwriteData(nvars+1,newlist);  // KEEP THE NEW LIST WITHOUT THE RESIDUAL
        rplEvalMultiUserFunc(listofeq,listofvars,nvars,0);    // EVALUATE THE EXPRESSIONS BUT GET A LIST WITH THE RESULTS INSTEAD
        if(Exceptions) { DSTop=stksave; return; }

        stksave[-5]=rplPeekData(nvars+2);
        stksave[-4]=rplPeekData(1);
        DSTop=stksave-3;    // RESTORE STACK AND DROP 3 ARGUMENTS
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
        if(MENUNUMBER(MenuCodeArg)>1) {
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



