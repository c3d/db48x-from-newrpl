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
#define LIBRARY_NUMBER  52

//@TITLE=Operations with Matrices and vectors

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    ECMD(TOARRAY,"→ARRY",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ARRAYDECOMP,"ARRY→",MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TOCOL,"→COL",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ADDCOL,"COL+",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(REMCOL,"COL-",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(FROMCOL,"COL→",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TODIAG,"→DIAG",MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    ECMD(FROMDIAG,"DIAG→",MKTOKENINFO(4,TITYPE_FUNCTION,2,2)), \
    ECMD(TOROW,"→ROW",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(ADDROW,"ROW+",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(REMROW,"ROW-",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(FROMROW,"ROW→",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TOV2,"→V2",MKTOKENINFO(3,TITYPE_FUNCTION,2,2)), \
    ECMD(TOV3,"→V3",MKTOKENINFO(3,TITYPE_FUNCTION,3,2)), \
    ECMD(FROMV,"V→",MKTOKENINFO(2,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DOMATPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DOMATPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(DOMATERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    /*CMD(AUGMENT,MKTOKENINFO(7,TITYPE_FUNCTION,1,2)),*/ \
    CMD(AXL,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    /*CMD(AXM,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)),*/ \
    CMD(BASIS,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(CHOLESKY,MKTOKENINFO(8,TITYPE_FUNCTION,1,2)), \
    CMD(CNRM,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(CON,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(COND,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(CROSS,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(CSWP,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(DET,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(DIAGMAP,MKTOKENINFO(7,TITYPE_FUNCTION,1,2)), \
    CMD(DOT,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(EGV,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(EGVL,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(GRAMSCHMIDT,MKTOKENINFO(11,TITYPE_FUNCTION,1,2)), \
    CMD(HADAMARD,MKTOKENINFO(8,TITYPE_FUNCTION,1,2)), \
    CMD(HILBERT,MKTOKENINFO(7,TITYPE_FUNCTION,1,2)), \
    CMD(IBASIS,MKTOKENINFO(6,TITYPE_FUNCTION,1,2)), \
    CMD(IDN,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(IMAGE,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(ISOM,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(JORDAN,MKTOKENINFO(6,TITYPE_FUNCTION,1,2)), \
    CMD(KER,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(LQ,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(LSQ,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(LU,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(MAD,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(MKISOM,MKTOKENINFO(6,TITYPE_FUNCTION,1,2)), \
    CMD(PMINI,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(QR,MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    CMD(RANK,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(RANM,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(RCI,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(RCIJ,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(RDM,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(REF,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(RNRM,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(RREF,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(RREFMOD,MKTOKENINFO(7,TITYPE_FUNCTION,1,2)), \
    CMD(RSD,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(RSWP,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(SCHUR,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(SNRM,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(SRAD,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(SVD,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(SVL,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(SYLVESTER,MKTOKENINFO(9,TITYPE_FUNCTION,1,2)), \
    CMD(TRACE,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(TRAN,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(TRN,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    CMD(VANDERMONDE,MKTOKENINFO(11,TITYPE_FUNCTION,1,2)), \
    CMD(LDUP,MKTOKENINFO(4,TITYPE_FUNCTION,1,2))
    



// ADD MORE OPCODES HERE


#define ERROR_LIST \
    ERR(MATRIXEXPECTED,0), \
    ERR(INVALIDDIMENSION,1), \
    ERR(NOTALLOWEDINMATRIX,2), \
    ERR(INCOMPATIBLEDIMENSION,3), \
    ERR(MATRIXORREALEXPECTED,4), \
    ERR(SQUAREMATRIXONLY,5), \
    ERR(VECTOREXPECTED,6), \
    ERR(MISPLACEDBRACKETS,7), \
    ERR(MALFORMEDMATRIX,8), \
    ERR(SINGULARMATRIX,9)




// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************


ROMOBJECT matrixeval_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,DOMATPRE),     // PREPARE EACH ELEMENT
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,DOMATPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,DOMATERR),     // ERROR HANDLER
    CMD_SEMI
};

ROMOBJECT matrixeval1_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,DOMATPRE),     // PREPARE EACH ELEMENT
    (CMD_OVR_EVAL1),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,DOMATPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,DOMATERR),     // ERROR HANDLER
    CMD_SEMI
};

ROMOBJECT matrixtonum_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,DOMATPRE),     // PREPARE EACH ELEMENT
    (CMD_OVR_NUM),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,DOMATPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,DOMATERR),     // ERROR HANDLER
    CMD_SEMI
};

ROMOBJECT matrixistrue_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,DOMATPRE),     // PREPARE EACH ELEMENT
    (CMD_OVR_ISTRUE),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,DOMATPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,DOMATERR),     // ERROR HANDLER
    CMD_SEMI
};

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib52_menu);


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
     (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)matrixeval_seco,
   (WORDPTR)matrixeval1_seco,
   (WORDPTR)matrixtonum_seco,
    (WORDPTR)matrixistrue_seco,


    (WORDPTR)lib52_menu,
    // ADD MORE MENUS HERE
    0
};




void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        rplPushData(IPtr);
        return;
    }


    if(ISUNARYOP(CurOpcode)) {
        if(!ISPROLOG(*rplPeekData(1))) {
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

        // UNARY OPERATORS
        switch(OPCODE(CurOpcode))
        {

        case OVR_INV:
        {
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR a=rplPeekData(1);

            if(!ISMATRIX(*a)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            rplMatrixInvert();
            return;
        }
        case OVR_ABS:
        {
           // COMPUTE THE FROBENIUS NORM ON MATRICES
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR a=rplPeekData(1);

            if(!ISMATRIX(*a)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            rplMatrixNorm();
            return;

        }
        case OVR_NEG:
        {
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR a=rplPeekData(1);

            if(!ISMATRIX(*a)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            if(rplMatrixIsPolar(a)) rplMatrixNegPolar();
            else rplMatrixNeg();
            return;

        }
        case OVR_EVAL1:
            // EVAL NEEDS TO SCAN THE MATRIX, EVAL EACH ARGUMENT SEPARATELY AND REBUILD IT.
        {

            WORDPTR object=rplPeekData(1),mainobj;
            if(!ISMATRIX(*object)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }
            mainobj=object;

            // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
            rplCreateLAMEnvironment(IPtr);

            object=rplMatrixGetFirstObj(object);
            WORDPTR endobject=rplSkipOb(mainobj);

            rplCreateLAM((WORDPTR)nulllam_ident,endobject);     // LAM 1 = END OF CURRENT LIST
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,object);     // LAM 2 = NEXT OBJECT TO PROCESS
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,mainobj);     // LAM 3 = ORIGINAL MATRIX
            if(Exceptions) { rplCleanupLAMs(0); return; }

            // GETLAM 1 = END OF MATRIX, GETLAM2 = OBJECT, GETLAM3 = ORIGINAL MATRIX

            // THIS NEEDS TO BE DONE IN 3 STEPS:
            // EVAL WILL PREPARE THE LAMS FOR OPEN EXECUTION
            // MATPRE WILL PUSH THE NEXT OBJECT IN THE STACK
            // MATPOST WILL CHECK IF THE ARGUMENT WAS PROCESSED WITHOUT ERRORS,
            // AND CLOSE THE LOOP TO PROCESS MORE ARGUMENTS

            // THE INITIAL CODE FOR EVAL MUST TRANSFER FLOW CONTROL TO A
            // SECONDARY THAT CONTAINS :: MATPRE EVAL MATPOST ;
            // MATPOST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
            // IN ORDER TO KEEP THE LOOP RUNNING

            rplPushRet(IPtr);
            IPtr=(WORDPTR)  matrixeval1_seco;
            CurOpcode=(CMD_OVR_EVAL1);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

            rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD EVALUATION

            return;
        }

        case OVR_FUNCEVAL:
            // EVALUATING A MATRIX OR VECTOR AS A FUNCTION GETS THE ELEMENT

        {
            WORDPTR comp=rplPeekData(1);
                WORDPTR posobj;
                BINT rows,cols,ndims;
                BINT posrow,poscol;
                rows=rplMatrixRows(comp);
                cols=rplMatrixCols(comp);

                if(!rows) {
                    // THIS IS A VECTOR
                    ndims=1;
                    rows=1;
                } else ndims=2; // IT'S A 2D MATRIX


                if(rplDepthData()<ndims+1) {
                    rplError(ERR_INVALIDPOSITION);
                    return;
                }

                // CHECK IF WE HAVE THE RIGHT POSITION

                    posobj=rplPeekData(2);

                    if(ISSYMBOLIC(*posobj)) {
                        if(!rplSymbIsNumeric(posobj)) {
                            rplError(ERR_INVALIDPOSITION);
                            return;
                        }

                        WORDPTR *stksave=DSTop;
                        rplPushData(posobj);
                        rplSymbNumericCompute();
                        if(Exceptions) { DSTop=stksave; return; }

                        posobj=rplPopData();


                    }

                    poscol=rplReadNumberAsBINT(posobj);
                    if(Exceptions) {
                        rplError(ERR_INVALIDPOSITION);
                        return;
                    }

                    if(ndims==2) {
                      // READ THE SECOND COORDINATE (COLUMN)
                      posobj=rplPeekData(3);

                      if(ISSYMBOLIC(*posobj)) {
                          if(!rplSymbIsNumeric(posobj)) {
                              rplError(ERR_INVALIDPOSITION);
                              return;
                          }

                          WORDPTR *stksave=DSTop;
                          rplPushData(posobj);
                          rplSymbNumericCompute();
                          if(Exceptions) { DSTop=stksave; return; }

                          posobj=rplPopData();


                      }




                      posrow=rplReadNumberAsBINT(posobj);
                      if(Exceptions) {
                          rplError(ERR_INVALIDPOSITION);
                          return;
                      }

                    } else posrow=1;

                // CHECK IF THE POSITION IS WITHIN THE MATRIX

                if( (posrow<1) || (posrow>rows) || (poscol<1) || (poscol>cols)) {
                    rplError(ERR_INDEXOUTOFBOUNDS);
                    return;
                }

                WORDPTR item=rplMatrixGet(comp,posrow,poscol);
                if(!item) {
                    rplError(ERR_INDEXOUTOFBOUNDS);
                    return;
                }

                rplOverwriteData(1+ndims,item);
                rplDropData(ndims);
            return;

        }

        case OVR_EVAL:
            // EVAL NEEDS TO SCAN THE MATRIX, EVAL EACH ARGUMENT SEPARATELY AND REBUILD IT.
        {
            WORDPTR object=rplPeekData(1),mainobj;
            if(!ISMATRIX(*object)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }
            mainobj=object;

            // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
            rplCreateLAMEnvironment(IPtr);

            object=rplMatrixGetFirstObj(object);
            WORDPTR endobject=rplSkipOb(mainobj);

            rplCreateLAM((WORDPTR)nulllam_ident,endobject);     // LAM 1 = END OF CURRENT LIST
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,object);     // LAM 2 = NEXT OBJECT TO PROCESS
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,mainobj);     // LAM 3 = ORIGINAL MATRIX
            if(Exceptions) { rplCleanupLAMs(0); return; }

            // GETLAM 1 = END OF MATRIX, GETLAM2 = OBJECT, GETLAM3 = ORIGINAL MATRIX

            // THIS NEEDS TO BE DONE IN 3 STEPS:
            // EVAL WILL PREPARE THE LAMS FOR OPEN EXECUTION
            // MATPRE WILL PUSH THE NEXT OBJECT IN THE STACK
            // MATPOST WILL CHECK IF THE ARGUMENT WAS PROCESSED WITHOUT ERRORS,
            // AND CLOSE THE LOOP TO PROCESS MORE ARGUMENTS

            // THE INITIAL CODE FOR EVAL MUST TRANSFER FLOW CONTROL TO A
            // SECONDARY THAT CONTAINS :: MATPRE EVAL MATPOST ;
            // MATPOST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
            // IN ORDER TO KEEP THE LOOP RUNNING

            rplPushRet(IPtr);
            IPtr=(WORDPTR)  matrixeval_seco;
            CurOpcode=(CMD_OVR_EVAL);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

            rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD EVALUATION

            return;
        }
        case OVR_NUM:
            // EVAL NEEDS TO SCAN THE MATRIX, EVAL EACH ARGUMENT SEPARATELY AND REBUILD IT.
        {
            WORDPTR object=rplPeekData(1),mainobj;
            if(!ISMATRIX(*object)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }
            mainobj=object;

            // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
            rplCreateLAMEnvironment(IPtr);

            object=rplMatrixGetFirstObj(object);
            WORDPTR endobject=rplSkipOb(mainobj);

            rplCreateLAM((WORDPTR)nulllam_ident,endobject);     // LAM 1 = END OF CURRENT LIST
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,object);     // LAM 2 = NEXT OBJECT TO PROCESS
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,mainobj);     // LAM 3 = ORIGINAL MATRIX
            if(Exceptions) { rplCleanupLAMs(0); return; }

            // GETLAM 1 = END OF MATRIX, GETLAM2 = OBJECT, GETLAM3 = ORIGINAL MATRIX

            // THIS NEEDS TO BE DONE IN 3 STEPS:
            // EVAL WILL PREPARE THE LAMS FOR OPEN EXECUTION
            // MATPRE WILL PUSH THE NEXT OBJECT IN THE STACK
            // MATPOST WILL CHECK IF THE ARGUMENT WAS PROCESSED WITHOUT ERRORS,
            // AND CLOSE THE LOOP TO PROCESS MORE ARGUMENTS

            // THE INITIAL CODE FOR EVAL MUST TRANSFER FLOW CONTROL TO A
            // SECONDARY THAT CONTAINS :: MATPRE EVAL MATPOST ;
            // MATPOST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
            // IN ORDER TO KEEP THE LOOP RUNNING

            rplPushRet(IPtr);
            IPtr=(WORDPTR)  matrixtonum_seco;
            CurOpcode=(CMD_OVR_NUM);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

            rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD EVALUATION

            return;
        }

        case OVR_ISTRUE:
            // EVAL NEEDS TO SCAN THE MATRIX, EVAL EACH ARGUMENT SEPARATELY AND REBUILD IT.
        {
            WORDPTR object=rplPeekData(1),mainobj;
            if(!ISMATRIX(*object)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }
            mainobj=object;

            // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
            rplCreateLAMEnvironment(IPtr);

            object=rplMatrixGetFirstObj(object);
            WORDPTR endobject=rplSkipOb(mainobj);

            rplCreateLAM((WORDPTR)nulllam_ident,endobject);     // LAM 1 = END OF CURRENT LIST
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,object);     // LAM 2 = NEXT OBJECT TO PROCESS
            if(Exceptions) { rplCleanupLAMs(0); return; }

            rplCreateLAM((WORDPTR)nulllam_ident,mainobj);     // LAM 3 = ORIGINAL MATRIX
            if(Exceptions) { rplCleanupLAMs(0); return; }

            // GETLAM 1 = END OF MATRIX, GETLAM2 = OBJECT, GETLAM3 = ORIGINAL MATRIX

            // THIS NEEDS TO BE DONE IN 3 STEPS:
            // EVAL WILL PREPARE THE LAMS FOR OPEN EXECUTION
            // MATPRE WILL PUSH THE NEXT OBJECT IN THE STACK
            // MATPOST WILL CHECK IF THE ARGUMENT WAS PROCESSED WITHOUT ERRORS,
            // AND CLOSE THE LOOP TO PROCESS MORE ARGUMENTS

            // THE INITIAL CODE FOR EVAL MUST TRANSFER FLOW CONTROL TO A
            // SECONDARY THAT CONTAINS :: MATPRE EVAL MATPOST ;
            // MATPOST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
            // IN ORDER TO KEEP THE LOOP RUNNING

            rplPushRet(IPtr);
            IPtr=(WORDPTR)  matrixistrue_seco;
            CurOpcode=(CMD_OVR_ISTRUE);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

            rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD EVALUATION

            return;
        }



        case OVR_XEQ:
            // XEQ NEEDS TO LEAVE THE MATRIX ON THE STACK
            return;

        }

    }

    if(ISBINARYOP(CurOpcode)) {

        switch(OPCODE(CurOpcode))
        {
        case OVR_ADD:
        {
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);

                return;
            }
            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            if(!ISMATRIX(*a))
            {
                // CHECK DIMENSIONS OF MATRIX B
                // OPERATION IS VALID IF B IS A SCALAR
                if(ISMATRIX(*b)){
                    BINT rows=rplMatrixRows(b);
                    BINT cols=rplMatrixCols(b);
                    if( (rows<2)&&(cols==1)) {
                        // OPERATION IS SCALAR, ACCEPT IT
                        WORDPTR scalar=rplMatrixGet(b,1,1);
                        rplOverwriteData(1,scalar);
                        rplCallOvrOperator(CurOpcode);
                        return;
                    }
                }
                    rplError(ERR_INCOMPATIBLEDIMENSION);
                    return;
            }
            if(!ISMATRIX(*b))
            {
                // CHECK DIMENSIONS OF MATRIX A
                // OPERATION IS VALID IF A IS A SCALAR
                if(ISMATRIX(*a)){
                    BINT rows=rplMatrixRows(a);
                    BINT cols=rplMatrixCols(a);
                    if( (rows<2)&&(cols==1)) {
                        // OPERATION IS SCALAR, ACCEPT IT
                        WORDPTR scalar=rplMatrixGet(a,1,1);
                        rplOverwriteData(2,scalar);
                        rplCallOvrOperator(CurOpcode);
                        return;
                    }
                }
                    rplError(ERR_INCOMPATIBLEDIMENSION);
                    return;
            }

            // HERE WE KNOW BOTH ARGUMENTS ARE MATRICES, HANDLE POLAR VECTORS AS SPECIAL CASE

            if(rplMatrixIsPolar(a)||rplMatrixIsPolar(b)) rplMatrixAddPolar(0);
            else rplMatrixAdd();
            return;
        }
        case OVR_SUB:
        {
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            if(!ISMATRIX(*a))
            {
                // CHECK DIMENSIONS OF MATRIX B
                // OPERATION IS VALID IF B IS A SCALAR
                if(ISMATRIX(*b)){
                    BINT rows=rplMatrixRows(b);
                    BINT cols=rplMatrixCols(b);
                    if( (rows<2)&&(cols==1)) {
                        // OPERATION IS SCALAR, ACCEPT IT
                        WORDPTR scalar=rplMatrixGet(b,1,1);
                        rplOverwriteData(1,scalar);
                        rplCallOvrOperator(CurOpcode);
                        return;
                    }
                }
                    rplError(ERR_INCOMPATIBLEDIMENSION);
                    return;
            }
            if(!ISMATRIX(*b))
            {
                // CHECK DIMENSIONS OF MATRIX A
                // OPERATION IS VALID IF A IS A SCALAR
                if(ISMATRIX(*a)){
                    BINT rows=rplMatrixRows(a);
                    BINT cols=rplMatrixCols(a);
                    if( (rows<2)&&(cols==1)) {
                        // OPERATION IS SCALAR, ACCEPT IT
                        WORDPTR scalar=rplMatrixGet(a,1,1);
                        rplOverwriteData(2,scalar);
                        rplCallOvrOperator(CurOpcode);
                        return;
                    }
                }
                    rplError(ERR_INCOMPATIBLEDIMENSION);
                    return;
            }

            if(rplMatrixIsPolar(a)||rplMatrixIsPolar(b)) rplMatrixAddPolar(1);
            else rplMatrixSub();
            return;
        }
        case OVR_MUL:
        {
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            // NEED TO DETECT SCALAR * MATRIX AND MATRIX * SCALAR CASES
            if((ISNUMBERCPLX(*a)
                  || ISSYMBOLIC(*a)
                  || ISIDENT(*a))) {
                // SCALAR BY MATRIX
                rplMatrixMulScalar();

                return;
            }
            if((ISNUMBERCPLX(*b)
                  || ISSYMBOLIC(*b)
                  || ISIDENT(*b))) {
                // SCALAR BY MATRIX
                rplMatrixMulScalar();

                return;
            }

            // HERE IT HAS TO BE MATRIX x MATRIX

            if(!ISMATRIX(*a) || !ISMATRIX(*b)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            rplMatrixMul();
            return;
        }

        case OVR_DIV:
        {
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            // NEED TO DETECT SCALAR / MATRIX AND MATRIX / SCALAR CASES
            if((ISNUMBERCPLX(*a)
                  || ISSYMBOLIC(*a)
                  || ISIDENT(*a))) {
                // SCALAR BY MATRIX
                rplError(ERR_MATRIXEXPECTED);
                return;
            }
            if((ISNUMBERCPLX(*b)
                  || ISSYMBOLIC(*b)
                  || ISIDENT(*b))) {
                // MATRIX DIV BY SCALAR
                rplMatrixDivScalar();

                return;
            }

            // HERE IT HAS TO BE MATRIX / MATRIX

            if(!ISMATRIX(*a) || !ISMATRIX(*b)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            // PERFORM A*INV(B)

            rplMatrixInvert();
            if(Exceptions) return;
            rplMatrixMul();
            return;


        }

        case OVR_POW:
        {
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORDPTR a=rplPeekData(2),b=rplPeekData(1);
            BINT neg=0;

            // ONLY MATRIX RAISED TO NUMERIC POWER IS SUPPORTED
            if(!ISMATRIX(*a))
            {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            while(ISSYMBOLIC(*b)) {
                b=rplSymbUnwrap(b);
                ++b;    // POINT TO EITHER THE SINGLE OBJECT INSIDE THE SYMBOLIC OR THE OPCODE
                if((*b==CMD_OVR_UMINUS)||(*b==CMD_OVR_NEG)) { neg^=1; ++b; }

            }

            if( !ISNUMBER(*b)) {
                rplError(ERR_INTEGEREXPECTED);
                return;
            }

            b=rplConstant2Number(b);    // CONVERT ANY CONSTANTS TO NUMERIC EQUIVALENT

            if(ISREAL(*b)) {
                REAL real;
                rplReadReal(b,&real);
                if(!isintegerReal(&real)) {
                    rplError(ERR_INTEGEREXPECTED);
                    return;
                }

            }
                BINT rows=MATROWS(a[1]),cols=MATCOLS(a[1]);

                if(rows!=cols) {
                    rplError(ERR_SQUAREMATRIXONLY);
                    return;
                }

                // TODO: CHECK FOR INTEGER RANGE AND ISSUE "Integer too large" ERROR
                BINT64 exp=rplReadNumberAsBINT(b);
                if(Exceptions) return;
                if(neg) exp=-exp;
                rplPopData();
                if(exp<0) {
                 rplMatrixInvert();
                 if(Exceptions) return;
                 exp=-exp;
                }

                BINT hasresult=0;
                while(exp) {
                if(exp&1) {
                    if(!hasresult) { rplPushData(rplPeekData(1));   // DUP THE CURRENT MATRIX
                                     hasresult=1;
                                }
                                else {
                                        rplPushData(rplPeekData(2));
                                        rplPushData(rplPeekData(2));
                                        rplMatrixMul();
                                        if(Exceptions) return;
                                        rplOverwriteData(3,rplPeekData(1));
                                        rplPopData();
                                 }
                }
                exp>>=1;
                if(exp) {
                rplPushData(rplPeekData(1));
                rplMatrixMul();
                if(Exceptions) return;
                }
                }

                rplPopData();
                return;
            }

        case OVR_SAME:
        {
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);

                return;
            }
            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            if(!ISMATRIX(*a))
            {
                // CHECK DIMENSIONS OF MATRIX B
                // OPERATION IS VALID IF B IS A SCALAR
                if(ISMATRIX(*b)){
                    BINT rows=rplMatrixRows(b);
                    BINT cols=rplMatrixCols(b);
                    if( (rows<2)&&(cols==1)) {
                        // OPERATION IS SCALAR, ACCEPT IT
                        WORDPTR scalar=rplMatrixGet(b,1,1);
                        rplOverwriteData(1,scalar);
                        rplCallOvrOperator(CurOpcode);
                        return;
                    }
                }

                // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
                    if(!ISPROLOG(*a)|| !ISPROLOG(*b)) {
                        if(*a==*b) {
                            rplDropData(2);
                            rplPushTrue();
                        } else {
                            rplDropData(2);
                            rplPushFalse();
                        }
                        return;

                    }
                    // ALWAYS RETURN FALSE IF DIFFERENT DIMENSIONS
                    rplDropData(2);
                    rplPushFalse();
                    return;
            }
            if(!ISMATRIX(*b))
            {
                // CHECK DIMENSIONS OF MATRIX A
                // OPERATION IS VALID IF A IS A SCALAR
                if(ISMATRIX(*a)){
                    BINT rows=rplMatrixRows(a);
                    BINT cols=rplMatrixCols(a);
                    if( (rows<2)&&(cols==1)) {
                        // OPERATION IS SCALAR, ACCEPT IT
                        WORDPTR scalar=rplMatrixGet(a,1,1);
                        rplOverwriteData(2,scalar);
                        rplCallOvrOperator(CurOpcode);
                        return;
                    }
                }
                // ALWAYS RETURN FALSE IF DIFFERENT DIMENSIONS
                rplDropData(2);
                rplPushFalse();
                    return;
            }

            // MATRIX HAS COMPATIBLE DIMENSIONS FOR ADDITION, NOW APPLY "SAME" TO ALL ELEMENTS
            rplMatrixSame();
            return;


        }
        case OVR_EQ:
        {
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);

                return;
            }
            WORDPTR a=rplPeekData(2),b=rplPeekData(1);

            if(!ISMATRIX(*a))
            {
                // CHECK DIMENSIONS OF MATRIX B
                // OPERATION IS VALID IF B IS A SCALAR
                if(ISMATRIX(*b)){
                    BINT rows=rplMatrixRows(b);
                    BINT cols=rplMatrixCols(b);
                    if( (rows<2)&&(cols==1)) {
                        // OPERATION IS SCALAR, ACCEPT IT
                        WORDPTR scalar=rplMatrixGet(b,1,1);
                        rplOverwriteData(1,scalar);
                        rplCallOvrOperator(CurOpcode);
                        return;
                    }
                }

                // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
                    if(!ISPROLOG(*a)|| !ISPROLOG(*b)) {
                        if(*a==*b) {
                            rplDropData(2);
                            rplPushTrue();
                        } else {
                            rplDropData(2);
                            rplPushFalse();
                        }
                        return;

                    }
                    // ALWAYS RETURN FALSE IF DIFFERENT DIMENSIONS
                    rplDropData(2);
                    rplPushFalse();
                    return;
            }
            if(!ISMATRIX(*b))
            {
                // CHECK DIMENSIONS OF MATRIX A
                // OPERATION IS VALID IF A IS A SCALAR
                if(ISMATRIX(*a)){
                    BINT rows=rplMatrixRows(a);
                    BINT cols=rplMatrixCols(a);
                    if( (rows<2)&&(cols==1)) {
                        // OPERATION IS SCALAR, ACCEPT IT
                        WORDPTR scalar=rplMatrixGet(a,1,1);
                        rplOverwriteData(2,scalar);
                        rplCallOvrOperator(CurOpcode);
                        return;
                    }
                }
                // ALWAYS RETURN FALSE IF DIFFERENT DIMENSIONS
                rplDropData(2);
                rplPushFalse();
                    return;
            }

            // MATRIX HAS COMPATIBLE DIMENSIONS FOR ADDITION, NOW APPLY "EQ" TO ALL ELEMENTS
            rplMatrixEqual();
            return;


        }
        break;
        }
    }

    switch(OPCODE(CurOpcode))
    {
    case TOARRAY:
    {
        //@SHORT_DESC=Assemble an array from its elements
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        BINT64 rows,cols;
        WORDPTR *Savestk=DSTop;

        if(ISLIST(*rplPeekData(1))) {
            rplExplodeList(rplPeekData(1));
            BINT ndims=rplReadNumberAsBINT(rplPopData());
            if((ndims<1) || (ndims>2)) {
                DSTop=Savestk;
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

            cols=rplReadNumberAsBINT(rplPopData());
            if(Exceptions) {
                DSTop=Savestk;
                return;
            }

            if(ndims==2) {

                rows=rplReadNumberAsBINT(rplPopData());
                if(Exceptions) {
                    DSTop=Savestk;
                    return;
                }



            } else rows=0;


            if( (rows<0)||(rows>65535)||(cols<1)||(cols>65535))  {
                DSTop=Savestk;
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

            // REMOVE THE LIST
            rplDropData(1);

            }
        else {
            // IT HAS TO BE A NUMBER

            cols=rplReadNumberAsBINT(rplPopData());
            if(Exceptions) {
                DSTop=Savestk;
                return;
            }

            rows=0;

            if((cols<1)||(cols>65535))  {
                DSTop=Savestk;
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

        }

        // HERE WE HAVE PROPER ROWS AND COLUMNS
        BINT elements=(rows)? rows*cols:cols;

        if(rplDepthData()<elements) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR newmat=rplMatrixCompose(rows,cols);

        if(newmat) {
            rplDropData(elements);
            rplPushData(newmat);
        }

        return;
        
       }
    case ARRAYDECOMP:
    {
        //@SHORT_DESC=Split an array into its elements
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISMATRIX(*rplPeekData(1))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        WORDPTR matrix=rplPeekData(1);
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

        WORDPTR *elem=rplMatrixExplode();
        if(Exceptions) return;

        // NOW REMOVE THE ORIGINAL MATRIX FROM THE STACK
        memmovew(elem-1,elem,(DSTop-elem)*(sizeof(WORDPTR *)/sizeof(WORD))); // ADDED sizeof() ONLY FOR 64-BIT COMPATIBILITY

        DSTop--;


        if(rows) rplNewBINTPush(rows,DECBINT);
        rplNewBINTPush(cols,DECBINT);
        rplPushData((WORDPTR)((rows)? two_bint : one_bint));
        rplCreateList();


        return;
    }
    case TOCOL:
    {
        //@SHORT_DESC=Split an array into column vectors
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISMATRIX(*rplPeekData(1))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        WORDPTR matrix=rplPeekData(1);
        WORDPTR *matptr=DSTop-1;
        WORDPTR elem;
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
        BINT nrows=(rows)? rows:1;
        BINT i,j;

        for(j=1;j<=cols;++j) {
        for(i=1;i<=nrows;++i) rplPushData(rplMatrixFastGet(*matptr,i,j));
        if(rows) {
            elem=rplMatrixCompose(0,nrows);
            if(!elem) return;
            rplDropData(nrows);
        } else elem=rplPopData();

        rplPushData(*matptr);
        rplOverwriteData(2,elem);
        matptr=DSTop-1;
        }

        rplDropData(1);
        rplNewBINTPush(cols,DECBINT);

        return;

    }
    case ADDCOL:
    {
        //@SHORT_DESC=Instert a column into an array
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISMATRIX(*rplPeekData(3))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }
        if(!ISMATRIX(*rplPeekData(2)) && !ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_MATRIXORREALEXPECTED);
            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        BINT64 nelem=rplReadNumberAsBINT(rplPeekData(1));





        WORDPTR matrix=rplPeekData(3);
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

        WORDPTR *matptr=DSTop-3;

        if( (nelem<1)||(nelem>cols+1)) {
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }


        if(!rows) {
            // ADD ELEMENTS TO A VECTOR

            // CHECK VALID TYPES FOR MATRIX ELEMENTS
            if(!ISNUMBERCPLX(*rplPeekData(2)) && !ISSYMBOLIC(*rplPeekData(2))
                  && !ISIDENT(*rplPeekData(2)))
             {
                rplError(ERR_NOTALLOWEDINMATRIX);
                return;
            }

            WORDPTR *first=rplMatrixNewEx(1,cols+1);

            if(!first) {
                return;
            }


            BINT j;
            WORDPTR *stkelem;
            for(j=1;j<nelem;++j) {
                stkelem=rplMatrixFastGetEx(first,cols+1,1,j);
                *stkelem=rplMatrixFastGet(*matptr,1,j);
            }

            stkelem=rplMatrixFastGetEx(first,cols+1,1,j);
            *stkelem=matptr[1]; // THE NEW ELEMENT MIGHT HAVE MOVED, SO GET IT FROM THE STACK
            ++j;

            for(;j<=cols+1;++j) {
                stkelem=rplMatrixFastGetEx(first,cols+1,1,j);
                *stkelem=rplMatrixFastGet(*matptr,1,j-1);
            }

            // MAKE A NEW VECTOR

            WORDPTR newmat=rplMatrixCompose(0,cols+1);
            if(!newmat) {
                DSTop=matptr+3;
                return;
            }

            rplDropData(cols+4);
            rplPushData(newmat);
            return;

        }

        // ADD A VECTOR OR A MATRIX TO A MATRIX

        if(!ISMATRIX(*rplPeekData(2))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }



        WORDPTR mat2=rplPeekData(2);
        BINT rows2=MATROWS(mat2[1]),cols2=MATCOLS(mat2[1]);


        // CHECK PROPER SIZE

        if(!rows2) {
            // MAKE IT A COLUMN VECTOR
            rows2=cols2;
            cols2=1;
        }

        if(rows2!=rows) {
            rplError(ERR_INVALIDDIMENSION);
            return;
        }

        // ADD THE COLUMNS

        WORDPTR *first=rplMatrixNewEx(rows,cols+cols2);

        if(!first) {
            return;
        }


        BINT i,j;
        WORDPTR *stkelem;
        for(j=1;j<nelem;++j) {
            for(i=1;i<=rows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols+cols2,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j);
            }
        }

        for(;j<nelem+cols2;++j) {
            for(i=1;i<=rows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols+cols2,i,j);
            *stkelem=rplMatrixFastGet(matptr[1],i,j-nelem+1);
            }
        }

        for(;j<=cols+cols2;++j) {
            for(i=1;i<=rows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols+cols2,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j-cols2);
            }
        }

        // MAKE A NEW VECTOR

        WORDPTR newmat=rplMatrixCompose(rows,cols+cols2);
        if(!newmat) {
            DSTop=matptr+3;
            return;
        }

        rplDropData(rows*(cols+cols2)+3);
        rplPushData(newmat);
        return;
    }
    case REMCOL:
    {
        //@SHORT_DESC=Remove a column from an array
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISMATRIX(*rplPeekData(2))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        BINT64 ncol=rplReadNumberAsBINT(rplPeekData(1));

        WORDPTR matrix=rplPeekData(2);
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
        BINT nrows=(rows)? rows:1;

        WORDPTR *matptr=DSTop-2;

        if(cols<=1) {
            rplError(ERR_INVALIDDIMENSION);
            return;
        }


        if( (ncol<1)||(ncol>cols)) {
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }

        // MAKE THE NEW MATRIX WITHOUT ONE COLUMN

        WORDPTR *first=rplMatrixNewEx(rows,cols-1);

        if(!first) {
            return;
        }


        BINT i,j;
        WORDPTR *stkelem;
        for(j=1;j<ncol;++j) {
            for(i=1;i<=nrows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols-1,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j);
            }
        }

        // SEPARATE THE COLUMN VECTOR/ELEMENT
        for(i=1;i<=nrows;++i) {
        rplPushData(rplMatrixFastGet(*matptr,i,j));
        }


        for(;j<=cols-1;++j) {
            for(i=1;i<=nrows;++i) {
            stkelem=rplMatrixFastGetEx(first,cols-1,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j+1);
            }
        }


        // MAKE THE VECTOR FROM THE ELEMENTS
        WORDPTR newmat;

        if(rows) {
            newmat=rplMatrixCompose(0,nrows);
            if(!newmat) {
                DSTop=matptr+2;
                return;
            }
            rplDropData(nrows);

        } else newmat=rplPopData();

        matptr[1]=newmat;   //  OVERWRITE THE FIRST ARGUMENT WITH THE VECTOR


        // MAKE A NEW VECTOR/MATRIX

        newmat=rplMatrixCompose(rows,cols-1);
        if(!newmat) {
            DSTop=matptr+2;
            return;
        }

        rplDropData(nrows*(cols-1));

        rplOverwriteData(2,newmat);

        return;

    }
    case FROMCOL:
        {
        //@SHORT_DESC=Assemble a matrix from its columns

        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        BINT64 nelem=rplReadNumberAsBINT(rplPeekData(1));

        if(rplDepthData()<nelem+1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        BINT i,j;
        BINT veclen=0;

        for(i=2;i<=nelem+1;++i) {
            if(ISMATRIX(*rplPeekData(i))) {
                WORDPTR matrix=rplPeekData(i);
                BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

                if(rows) {
                    rplError(ERR_VECTOREXPECTED);
                    return;
                }

                if(veclen) {
                    if(cols!=veclen) {
                        rplError(ERR_INVALIDDIMENSION);
                        return;
                    }
                } else {
                    if(i==2) veclen=cols;
                    else {
                        // DON'T ALLOW MIX OF VECTOR/NUMBERS
                        rplError(ERR_REALEXPECTED);
                        return;
                    }
                }
            }
            else {
                if(! (ISNUMBERCPLX(*rplPeekData(i))
                      || ISSYMBOLIC(*rplPeekData(i))
                      || ISIDENT(*rplPeekData(i)))) {
                    rplError(ERR_NOTALLOWEDINMATRIX);
                            return;
                }

                if(veclen) {
                    rplError(ERR_VECTOREXPECTED);
                    return;
                }

            }
        }

        // HERE WE HAVE ALL ELEMENTS PROPERLY VALIDATED

        if(veclen) {
            // EXPAND ANY VECTORS AND THEN COMPOSE THE MATRIX
            WORDPTR *base=DSTop;
            for(j=1;j<=veclen;++j) {
            for(i=nelem+1;i>=2;i--) {
                rplPushData(rplMatrixGet(base[-i],1,j));
            }
            }
            WORDPTR newmat=rplMatrixCompose(veclen,nelem);

            if(!newmat) { DSTop=base; return; }

            DSTop=base-nelem;
            rplOverwriteData(1,newmat);
            return;
        }

        // THESE ARE SIMPLE ELEMENTS, MAKE A VECTOR

        rplDropData(1);

        WORDPTR newmat=rplMatrixCompose(0,nelem);

        if(!newmat) { return; }

        rplDropData(nelem-1);
        rplOverwriteData(1,newmat);
        return;



        }
    case TODIAG:
    {
        //@SHORT_DESC=Extract diagonal elements from a matrix
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISMATRIX(*rplPeekData(1))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        WORDPTR matrix=rplPeekData(1);
        WORDPTR *matptr=DSTop-1;
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
        BINT nrows=(rows)? rows:1;
        BINT i;

        if(nrows>cols) nrows=cols;

        for(i=1;i<=nrows;++i) rplPushData(rplMatrixFastGet(*matptr,i,i));

        matrix=rplMatrixCompose(0,nrows);
        if(!matrix) { DSTop=matptr+1; return; }
        rplDropData(nrows);
        rplOverwriteData(1,matrix);

        return;

    }
    case FROMDIAG:
        {
        //@SHORT_DESC=Create a matrix with the given diagonal elements

        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISMATRIX(*rplPeekData(2))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }
        WORDPTR matrix=rplPeekData(2);
        BINT drows=MATROWS(matrix[1]),dcols=MATCOLS(matrix[1]);

        if(drows) {
            rplError(ERR_VECTOREXPECTED);
            return;
        }

        BINT64 rows,cols;
        WORDPTR *Savestk=DSTop;

        if(ISLIST(*rplPeekData(1))) {
            rplExplodeList(rplPeekData(1));
            BINT ndims=rplReadNumberAsBINT(rplPopData());
            if((ndims<1) || (ndims>2)) {
                DSTop=Savestk;
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

            cols=rplReadNumberAsBINT(rplPopData());
            if(Exceptions) {
                DSTop=Savestk;
                return;
            }

            if(ndims==2) {

                rows=rplReadNumberAsBINT(rplPopData());
                if(Exceptions) {
                    DSTop=Savestk;
                    return;
                }



            } else rows=0;


            if( (rows<0)||(rows>65535)||(cols<1)||(cols>65535))  {
                DSTop=Savestk;
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

            }
        else {
            // IT HAS TO BE A NUMBER

            cols=rplReadNumberAsBINT(rplPeekData(1));
            if(Exceptions) {
                DSTop=Savestk;
                return;
            }

            rows=0;

            if((cols<1)||(cols>65535))  {
                DSTop=Savestk;
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

        }

        // HERE WE HAVE PROPER ROWS AND COLUMNS
        BINT elements=(rows)? rows*cols:cols;

        BINT i;
        WORDPTR *first=DSTop;
        for(i=0;i<elements;++i) rplPushData((WORDPTR)zero_bint);

        if(Exceptions) { DSTop=Savestk; return; }

        matrix=rplPeekData(elements+2);    // READ AGAIN IN CASE IT MOVED

        if(dcols>cols) dcols=cols;
        if(dcols>rows) dcols=rows? rows:1;

        for(i=1;i<=dcols;++i) {
            *rplMatrixFastGetEx(first,cols,i,i)=rplMatrixFastGet(matrix,1,i);
        }

        WORDPTR newmat=rplMatrixCompose(rows,cols);

        if(newmat) {
            rplDropData(elements+1);
            rplOverwriteData(1,newmat);
        } else DSTop=Savestk;

        return;

       }

    case TOROW:
    {
        //@SHORT_DESC=Split an array into its row vectors
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISMATRIX(*rplPeekData(1))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        WORDPTR matrix=rplPeekData(1);
        WORDPTR *matptr=DSTop-1;
        WORDPTR elem;
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
        BINT nrows=(rows)? rows:1;
        BINT i,j;

        if(!rows) {
            // USE VECTORS AS VERTICAL COLUMNS
            nrows=cols;
            cols=1;
        }

        for(i=1;i<=nrows;++i) {
        for(j=1;j<=cols;++j) rplPushData(rplMatrixFastGet(*matptr,i,j));
        if(rows) {
            elem=rplMatrixCompose(0,cols);
            if(!elem) return;
            rplDropData(cols);
        } else elem=rplPopData();

        rplPushData(*matptr);
        rplOverwriteData(2,elem);
        matptr=DSTop-1;
        }

        rplDropData(1);
        rplNewBINTPush(nrows,DECBINT);

        return;

    }

    case ADDROW:
    {
        //@SHORT_DESC=Insert a row into an array
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISMATRIX(*rplPeekData(3))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }
        if(!ISMATRIX(*rplPeekData(2)) && !ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_MATRIXORREALEXPECTED);
            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        BINT64 nelem=rplReadNumberAsBINT(rplPeekData(1));





        WORDPTR matrix=rplPeekData(3);
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

        WORDPTR *matptr=DSTop-3;

        if( (nelem<1)||(nelem>cols+1)) {
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }


        if(!rows) {
            // ADD ELEMENTS TO A VECTOR

            // CHECK VALID TYPES FOR MATRIX ELEMENTS
            if(!ISNUMBERCPLX(*rplPeekData(2)) && !ISSYMBOLIC(*rplPeekData(2))
                  && !ISIDENT(*rplPeekData(2)))
             {
                rplError(ERR_NOTALLOWEDINMATRIX);
                return;
            }

            WORDPTR *first=rplMatrixNewEx(1,cols+1);

            if(!first) {
                return;
            }


            BINT j;
            WORDPTR *stkelem;
            for(j=1;j<nelem;++j) {
                stkelem=rplMatrixFastGetEx(first,cols+1,1,j);
                *stkelem=rplMatrixFastGet(*matptr,1,j);
            }

            stkelem=rplMatrixFastGetEx(first,cols+1,1,j);
            *stkelem=matptr[1]; // THE NEW ELEMENT MIGHT HAVE MOVED, SO GET IT FROM THE STACK
            ++j;

            for(;j<=cols+1;++j) {
                stkelem=rplMatrixFastGetEx(first,cols+1,1,j);
                *stkelem=rplMatrixFastGet(*matptr,1,j-1);
            }

            // MAKE A NEW VECTOR

            WORDPTR newmat=rplMatrixCompose(0,cols+1);
            if(!newmat) {
                DSTop=matptr+3;
                return;
            }

            rplDropData(cols+4);
            rplPushData(newmat);
            return;

        }

        // ADD A VECTOR OR A MATRIX TO A MATRIX

        if(!ISMATRIX(*rplPeekData(2))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }



        WORDPTR mat2=rplPeekData(2);
        BINT rows2=MATROWS(mat2[1]),cols2=MATCOLS(mat2[1]);


        // CHECK PROPER SIZE

        if(!rows2) {
            // MAKE IT A ROW VECTOR
            rows2=1;
        }

        if(cols2!=cols) {
            rplError(ERR_INVALIDDIMENSION);
            return;
        }

        // ADD THE ROWS

        WORDPTR *first=rplMatrixNewEx(rows+rows2,cols);

        if(!first) {
            return;
        }


        BINT i,j;
        WORDPTR *stkelem;
        for(i=1;i<nelem;++i) {
            for(j=1;j<=cols;++j) {
            stkelem=rplMatrixFastGetEx(first,cols,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j);
            }
        }

        for(;i<nelem+rows2;++i) {
            for(j=1;j<=cols;++j) {
            stkelem=rplMatrixFastGetEx(first,cols,i,j);
            *stkelem=rplMatrixFastGet(matptr[1],i-nelem+1,j);
            }
        }

        for(;i<=rows+rows2;++i) {
            for(j=1;j<=cols;++j) {
            stkelem=rplMatrixFastGetEx(first,cols,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i-rows2,j);
            }
        }

        // MAKE A NEW VECTOR

        WORDPTR newmat=rplMatrixCompose(rows+rows2,cols);
        if(!newmat) {
            DSTop=matptr+3;
            return;
        }

        rplDropData(cols*(rows+rows2)+3);
        rplPushData(newmat);
        return;
    }

    case REMROW:
    {
        //@SHORT_DESC=Remove a row from an array
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISMATRIX(*rplPeekData(2))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        BINT64 nrow=rplReadNumberAsBINT(rplPeekData(1));

        WORDPTR matrix=rplPeekData(2);
        BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
        BINT nrows=(rows)? rows:1;

        WORDPTR *matptr=DSTop-2;

        if(!rows) {
            // MAKE IT A COLUMN VECTOR
            nrows=cols;
            cols=1;
        }

        if(nrows<=1) {
            rplError(ERR_INVALIDDIMENSION);
            return;
        }


        if( (nrow<1)||(nrow>nrows)) {
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }

        // MAKE THE NEW MATRIX WITHOUT ONE ROW

        WORDPTR *first=rplMatrixNewEx(nrows-1,cols);

        if(!first) {
            return;
        }


        BINT i,j;
        WORDPTR *stkelem;
        for(i=1;i<nrow;++i) {
            for(j=1;j<=cols;++j) {
            stkelem=rplMatrixFastGetEx(first,cols,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i,j);
            }
        }

        // SEPARATE THE ROW VECTOR/ELEMENT
        for(j=1;j<=cols;++j) {
        rplPushData(rplMatrixFastGet(*matptr,i,j));
        }


        for(;i<=nrows-1;++i) {
            for(j=1;j<=cols;++j) {
            stkelem=rplMatrixFastGetEx(first,cols,i,j);
            *stkelem=rplMatrixFastGet(*matptr,i+1,j);
            }
        }


        // MAKE THE VECTOR FROM THE ELEMENTS
        WORDPTR newmat;

        if(rows) {
            newmat=rplMatrixCompose(0,cols);
            if(!newmat) {
                DSTop=matptr+2;
                return;
            }
            rplDropData(cols);

        } else newmat=rplPopData();

        matptr[1]=newmat;   //  OVERWRITE THE FIRST ARGUMENT WITH THE VECTOR


        // MAKE A NEW VECTOR/MATRIX
        if(rows) newmat=rplMatrixCompose(nrows-1,cols);
        else newmat=rplMatrixCompose(0,nrows-1);
        if(!newmat) {
            DSTop=matptr+2;
            return;
        }

        rplDropData((nrows-1)*cols);

        rplOverwriteData(2,newmat);

        return;

    }

    case FROMROW:
    {
        //@SHORT_DESC=Assemble an array from its rows

    if(rplDepthData()<1) {
        rplError(ERR_BADARGCOUNT);
        return;
    }
    if(!ISNUMBER(*rplPeekData(1))) {
        rplError(ERR_INTEGEREXPECTED);
        return;
    }

    BINT64 nelem=rplReadNumberAsBINT(rplPeekData(1));

    if(rplDepthData()<nelem+1) {
        rplError(ERR_BADARGCOUNT);
        return;
    }

    BINT i,j;
    BINT veclen=0;

    for(i=2;i<=nelem+1;++i) {
        if(ISMATRIX(*rplPeekData(i))) {
            WORDPTR matrix=rplPeekData(i);
            BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);

            if(rows) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

            if(veclen) {
                if(cols!=veclen) {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }
            } else {
                if(i==2) veclen=cols;
                else {
                    // DON'T ALLOW MIX OF VECTOR/NUMBERS
                    rplError(ERR_REALEXPECTED);
                    return;
                }
            }
        }
        else {
            if(! (ISNUMBERCPLX(*rplPeekData(i))
                  || ISSYMBOLIC(*rplPeekData(i))
                  || ISIDENT(*rplPeekData(i)))) {
                rplError(ERR_NOTALLOWEDINMATRIX);
                        return;
            }

            if(veclen) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }

        }
    }

    // HERE WE HAVE ALL ELEMENTS PROPERLY VALIDATED

    if(veclen) {
        // EXPAND ANY VECTORS AND THEN COMPOSE THE MATRIX
        WORDPTR *base=DSTop;
        for(i=nelem+1;i>=2;i--) {
        for(j=1;j<=veclen;++j) {
            rplPushData(rplMatrixGet(base[-i],1,j));
        }
        }
        WORDPTR newmat=rplMatrixCompose(nelem,veclen);

        if(!newmat) { DSTop=base; return; }

        DSTop=base-nelem;
        rplOverwriteData(1,newmat);
        return;
    }

    // THESE ARE SIMPLE ELEMENTS, MAKE A VECTOR

    rplDropData(1);

    WORDPTR newmat=rplMatrixCompose(0,nelem);

    if(!newmat) { return; }

    rplDropData(nelem-1);
    rplOverwriteData(1,newmat);
    return;



    }



    case TOV2:
    {
        //@SHORT_DESC=Assemble a vector from two values

    // PACK 2 VALUES INTO A VECTOR

    // ALL ARGUMENT CHECKS ARE DONE IN rplMatrixCompose

    WORDPTR newmat=rplMatrixCompose(0,2);
    if(!newmat) return;

    rplDropData(1);
    rplOverwriteData(1,newmat);

    return;

    }
    case TOV3:
    {
        //@SHORT_DESC=Assemble a vector from three values
        // PACK 3 VALUES INTO A VECTOR

    // ALL ARGUMENT CHECKS ARE DONE IN rplMatrixCompose

    WORDPTR newmat=rplMatrixCompose(0,3);
    if(!newmat) return;

    rplDropData(2);
    rplOverwriteData(1,newmat);

    return;

    }

    case FROMV:
    {
        //@SHORT_DESC=Split a vector into its elements

    // EXPLODE A VECTOR INTO ITS COMPONENTS
    if(rplDepthData()<1) {
        rplError(ERR_BADARGCOUNT);
        return;
    }

    if(!ISMATRIX(*rplPeekData(1))) {
        rplError(ERR_MATRIXEXPECTED);
        return;
    }
    WORDPTR matrix=rplPeekData(1);
    BINT rows=MATROWS(matrix[1]);

    if( (rows!=0) && (rows!=1)) {
        rplError(ERR_VECTOREXPECTED);
        return;
    }

    WORDPTR *first=rplMatrixExplode();
    // NOW REMOVE THE ORIGINAL MATRIX
    memmovew(first-1,first,(DSTop-first)*(sizeof(void*)>>2));
    rplDropData(1);
    return;

    }

    case DOMATPRE:
    {


        // GETLAM 1 = END OF MATRIX, GETLAM2 = OBJECT, GETLAM3 = ORIGINAL MATRIX


        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE DOLISTERR WORD

        // NOW RECALL THE MATRIX OBJECT TO THE STACK

        rplPushData(*rplGetLAMn(2));

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case DOMATPOST:
    {

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR DOING THE EVALUATION

        // GETLAM 1 = END OF MATRIX, GETLAM2 = OBJECT, GETLAM3 = ORIGINAL MATRIX
        WORDPTR endobj=*rplGetLAMn(1),
                nextobj=rplSkipOb(*rplGetLAMn(2)),
                matrix=*rplGetLAMn(3);

        if(nextobj<endobj) {
            // NEED TO DO ONE MORE LOOP
            rplPutLAMn(2,nextobj);  // STORE NEW OBJECT

            IPtr-=3;   // CONTINUE THE LOOP, MAKE NEXT INSTRUCTION BE DOMATPRE ON ANY LOOP (2 INSTRUCTIONS BACK)
            // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
            return;
        }

        // ALL ELEMENTS WERE PROCESSED
        // FORM A NEW MATRIX WITH ALL THE NEW ELEMENTS

        WORDPTR *prevDStk = rplUnprotectData();

        BINT newdepth=(BINT)(DSTop-prevDStk);

        // COMPUTE THE REQUIRED SIZE

        if(OPCODE(*(IPtr-1))==OVR_ISTRUE) {
            // SPECIAL CASE ISTRUE DOESN'T RETURN A MATRIX
            BINT istrue=0;
            BINT k;
            for(k=1;k<=newdepth;++k) {
                if(!rplIsFalse(rplPeekData(k))) istrue=1;
            }

            // HERE THE STACK HAS: MATRIX ELEM1... ELEMN
            rplDropData(newdepth);


            if(istrue) rplOverwriteData(1,(WORDPTR)one_bint);
            else rplOverwriteData(1,(WORDPTR)zero_bint);

            rplCleanupLAMs(0);
            CurOpcode=*(IPtr-1);    // BLAME THE OPERATOR IN QUESTION
            IPtr=rplPopRet();       // AND RETURN
            return;
        }

        BINT totalsize=rplMatrixGetFirstObj(matrix)-matrix;
        BINT k;
        for(k=1;k<=newdepth;++k) {
            totalsize+=rplObjSize(rplPeekData(k));
        }

        // NOW ALLOCATE THE NEW MATRIX

         WORDPTR newmat=rplAllocTempOb(totalsize-1);
        if( (!newmat) || Exceptions) {
            DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
            rplCleanupLAMs(0);      // CLEANUP LOCAL VARIABLES
            CurOpcode=*(IPtr-1);    // BLAME THE OPERATOR IN QUESTION
            IPtr=rplPopRet();       // AND RETURN
            return;
        }

        // RE-READ ALL POINTERS, SINCE THEY COULD'VE MOVED
        matrix=*rplGetLAMn(3);
        nextobj=rplMatrixGetFirstObj(matrix);       // FIRST OBJECT = END OF TABLES
        newmat[0]=MKPROLOG(DOMATRIX,totalsize-1);
        newmat[1]=matrix[1];    // SAME SIZE AS ORIGINAL MATRIX

        BINT nelem=nextobj-matrix-2;
        BINT oldidx;
        WORDPTR oldobj,newobj,firstobj,oldfirst,oldptr;

        // FILL THE MATRIX WITH ALL THE OBJECTS FROM THE STACK
        firstobj=newobj=rplMatrixGetFirstObj(newmat);        // STORE NEW OBJECTS HERE
        for(k=newdepth;k>=1;--k) { rplCopyObject(newobj,rplPeekData(k)); newobj=rplSkipOb(newobj); }

        oldfirst=rplMatrixGetFirstObj(matrix);

        for(k=0;k<nelem;++k) {
            // GET THE INDEX NUMBER OF THE OBJECT FROM THE OFFSET
            oldobj=matrix+matrix[2+k];
            for(oldidx=0,oldptr=oldfirst;oldptr<oldobj;++oldidx) oldptr=rplSkipOb(oldptr);
            // FIND THE OBJECT ON THE NEW MATRIX
            for(newobj=firstobj;oldidx>0;--oldidx) newobj=rplSkipOb(newobj);
            // STORE THE NEW OFFSET
            newmat[2+k]=newobj-newmat;
        }



        // HERE THE STACK HAS: MATRIX ELEM1... ELEMN
        rplOverwriteData(newdepth+1,newmat);
        rplDropData(newdepth);

        rplCleanupLAMs(0);
        CurOpcode=*(IPtr-1);    // BLAME THE OPERATOR IN QUESTION
        IPtr=rplPopRet();       // AND RETURN
        return;
    }

    case DOMATERR:
    {
        // SAME PROCEDURE AS ENDERR
        rplRemoveExceptionHandler();
        rplPopRet();
        rplUnprotectData();
        rplRemoveExceptionHandler();

        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        CurOpcode=*(IPtr-2);    // BLAME THE OPERATOR IN QUESTION
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;
        ExceptionPointer=IPtr;
        return;
    }

    case AXL:
    {
        //@SHORT_DESC=Convert a matrix to list and vice versa
        // CONVERT MATRIX TO LIST AND VICE VERSA
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(ISMATRIX(*rplPeekData(1))) {
            // MATRIX TO LIST

            BINT cols,rows,i,j;
            WORDPTR *mat=DSTop-1;
            cols=rplMatrixCols(*mat);
            rows=rplMatrixRows(*mat);

            if(rows) {
            for(i=1;i<=rows;++i)
            {
                for(j=1;j<=cols;++j)
                {
                    rplPushData(rplMatrixFastGet(*mat,i,j));
                }
                rplNewBINTPush(cols,DECBINT);
                rplCreateList();
                if(Exceptions) { DSTop=mat+1; return; }
            }
            rplNewBINTPush(rows,DECBINT);
            rplCreateList();
            if(Exceptions) { DSTop=mat+1; return; }
            }
            else {
                for(j=1;j<=cols;++j)
                {
                    rplPushData(rplMatrixFastGet(*mat,1,j));
                }
                rplNewBINTPush(cols,DECBINT);
                rplCreateList();
                if(Exceptions) { DSTop=mat+1; return; }
            }

            rplOverwriteData(1,rplPopData());

            return;
        }

        if(ISLIST(*rplPeekData(1))) {
            // LIST TO MATRIX

            BINT cols,rows,i,j,tmpcols;
            WORDPTR *lst=DSTop-1,lstptr;
            cols=0;
            // DETERMINE IF DIMENSIONS ARE SUITABLE
            rows=rplListLength(*lst);
            if(rows<1) {
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

            // WALK THE LIST
            lstptr=(*lst)+1;
            for(i=0;i<rows;++i,lstptr=rplSkipOb(lstptr)) {
                if(ISLIST(*lstptr)) {
                    tmpcols=rplListLength(lstptr);
                    if(!cols) cols=tmpcols;
                    if(cols!=tmpcols) {
                        rplError(ERR_INVALIDDIMENSION);
                        return;
                    }
                }
                else {
                    if(!cols) cols=-1;
                    if(cols!=-1) {
                        rplError(ERR_INVALIDDIMENSION);
                        return;
                    }
                }

            }

            // SANITIZE THE RESULTS WE FOUND
            if(cols==-1) {
                // IT'S A VECTOR, NOT A MATRIX
                cols=rows;
                rows=0;
            }

            // HERE WE HAVE A PROPERLY FORMED LIST
            if(rows) {
            // PUSH ALL ELEMENTS IN THE STACK
            for(i=0;i<rows;++i)
            {
                for(j=0;j<cols;++j) {
                    rplPushData(rplGetListElementFlat(*lst,i*cols+j+1));
                }
            }
            }
            else {
                for(j=1;j<=cols;++j) {
                    rplPushData(rplGetListElementFlat(*lst,j));
                }
            }

            WORDPTR mat=rplMatrixCompose(rows,cols);
            if(Exceptions || (!mat)) {
                DSTop=lst+1;
                return;
            }

            // JUST DROP EVERYTHING AND REPLACE THE LIST WITH THE NEW MATRIX
            DSTop=lst+1;
            rplOverwriteData(1,mat);

            return;

        }

        // ANY OTHER OBJECT TYPE IS NOT ALLOWED
        rplError(ERR_BADARGTYPE);
        return;

    }

    case BASIS:
        {
        //@SHORT_DESC=Find vectors forming a basis of the subspace represented by the matrix
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR *a=DSTop-1;
            WORDPTR *savestk=DSTop;

            if(!ISLIST(**a)) {
                rplError(ERR_LISTEXPECTED);
                return;
            }

            WORDPTR lelem=*a+1,endoflist=rplSkipOb(*a);
            BINT ncols=0,nvecs=0,allzeros;
            BINT i,j,k;

            while( (lelem<endoflist) && (*lelem!=CMD_ENDLIST) ) {

            if(!ISMATRIX(*lelem)) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }
            if(rplMatrixRows(lelem)>1) {
                rplError(ERR_VECTOREXPECTED);
                return;

            }
            if(ncols) {
                if(rplMatrixCols(lelem)!=ncols) {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }
            } else ncols=rplMatrixCols(lelem);

            //  EXPAND THE VALID VECTOR
            BINT off=lelem-*a;  // PROTECT FROM GC
            for(k=1;k<=ncols;++k) {
                rplPushData(rplMatrixGet(lelem,1,k));
                if(Exceptions) { DSTop=savestk; return; }
            }
            lelem=*a+off;
            ++nvecs;
            lelem=rplSkipOb(lelem);
            }

            // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
            rplMatrixBareissEx(a,0,nvecs,ncols,1);
            if(Exceptions) {
                DSTop=savestk;
                return;
            }


            rplMatrixBackSubstEx(a,nvecs,ncols);
            if(Exceptions) {
                DSTop=savestk;
                return;
            }

            // EXPAND THE MATRIX INTO ITS ROWS, BUT ELIMINATE ALL NULL ONES

            WORDPTR *base=DSTop;

            for(i=1;i<=nvecs;++i) {

            allzeros=1;
            for(j=1;j<=ncols;++j) {
                if(!rplSymbIsZero(base[-j])) { allzeros=0; break; }
            }
            if(allzeros) {
                // DROP THE ROWS THAT ARE ALL ZERO
                --base;
                rplRemoveAtData(DSTop-base,ncols);
                --nvecs;
                --i;
                base-=ncols-1;
            }
            else {
                // MAKE A ROW VECTOR AND LEAVE IT IN THE STACK
                --base;
                lelem=rplMatrixComposeN(DSTop-base,0,ncols);
                if(!lelem) { DSTop=savestk; return; }
                rplRemoveAtData(DSTop-base,ncols-1);
                base-=ncols-1;
                base[0]=lelem;
            }

            }


            // HERE WE HAVE nvecs IN THE STACK, READY TO PACK

            WORDPTR newlist=rplCreateListN(nvecs,1,1);
            if(Exceptions) { DSTop=savestk; return; }

            rplOverwriteData(1,newlist);
            return;
        }


    case CHOLESKY:
        {
        //@SHORT_DESC=Perform Cholesky decomposition on a matrix
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR *a=DSTop-1,*savestk=DSTop;

            if(!ISMATRIX(**a)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            BINT rows,cols;
            rows=rplMatrixRows(*a);
            cols=rplMatrixCols(*a);

            // ONLY ACCEPT SQUARE MATRICES
            if(rows!=cols) {
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

            // TODO: TEST FOR HERMITIAN SYMMETRY
            // ORIGINAL COMMAND GIVES BAD RESULTS IF MATRIX IS NOT SPD
            // AND DOESN'T EVEN ALLOW COMPLEX



            WORDPTR *first=rplMatrixExplode();
            if(Exceptions) return;

            // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
            BINT canreduce=rplMatrixBareissEx(a,0,rows,cols,1);
            if(!canreduce) rplError(ERR_SINGULARMATRIX);
            if(Exceptions) {
                DSTop=savestk;
                return;
            }

            // CREATE THE DIAGONAL VECTOR
            // Dii=u(i,i)*u(i-1,i-1)
            BINT k,j;

            for(k=1;k<=rows;++k) {
                rplPushData(*rplMatrixFastGetEx(first,cols,k,k));
                if(k>1) {
                    rplPushData(*rplMatrixFastGetEx(first,cols,k-1,k-1));
                    rplCallOvrOperator(CMD_OVR_MUL);
                }
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }
            }

            // MULTIPLY THE DIAGONAL BY SQRT(D^-1)

            for(k=rows;k>=1;--k) {
                rplCallOperator(CMD_SQRT);
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }
                for(j=k;j<=cols;++j) {
                rplPushData(*rplMatrixFastGetEx(first,cols,k,j));
                rplPushData(rplPeekData(2));
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }
                rplCallOvrOperator(CMD_OVR_DIV);
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }
                *rplMatrixFastGetEx(first,cols,k,j)=rplPopData();
                }
                rplPopData();

            }



            // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
            WORDPTR newmat=rplMatrixCompose(rows,cols);
            if(Exceptions) {
                DSTop=savestk;
                return;
            }
            DSTop=savestk;
            rplOverwriteData(1,newmat);

            return;
        }

    case CNRM:
    {
        //@SHORT_DESC=Column norm (one norm) of a matrix
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR *a=DSTop-1,*savestk=DSTop;

        if(!ISMATRIX(**a)) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        BINT rows,cols;
        rows=rplMatrixRows(*a);
        cols=rplMatrixCols(*a);

        if(!rows) rows=1;

        BINT i,j;

        for(j=1;j<=cols;++j) {

        for(i=1;i<=rows;++i) {
                rplPushData(rplMatrixFastGet(*a,i,j));
                rplCallOvrOperator(CMD_OVR_ABS);
                if(Exceptions) { DSTop=savestk; return; }
                if(i>1) {
                    rplCallOvrOperator(CMD_OVR_ADD);
                    if(Exceptions) { DSTop=savestk; return; }
                }
        }
        if(j>1) {
            rplCallOperator(CMD_MAX);
            if(Exceptions) { DSTop=savestk; return; }
        }
        }

        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions) { DSTop=savestk; return; }
        }

        rplOverwriteData(2,rplPeekData(1));
        rplDropData(1);
        return;
    }

    case CON:
    {
        //@SHORT_DESC=Assemble an array with given constant value
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            BINT64 rows,cols;
            WORDPTR *var=0;

            if(!rplMatrixIsAllowed(rplPeekData(1))) {
                rplError(ERR_NOTALLOWEDINMATRIX);
                return;
            }

            if(ISIDENT(*rplPeekData(2))) {

                var=rplFindLAM(rplPeekData(2),1);
                if(!var) var=rplFindGlobal(rplPeekData(2),1);
                if(!var) {
                    rplError(ERR_UNDEFINEDVARIABLE);
                    return;
                }
                ++var;
                if(!ISMATRIX(**var)) {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }

            }
            else var=DSTop-2;


            if(ISLIST(**var)) {
                BINT ndims=rplListLength(*var);
                if((ndims<1) || (ndims>2)) {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }

                cols=rplReadNumberAsBINT(rplGetListElement(*var,1));
                if(Exceptions) {
                    return;
                }

                if(ndims==2) {
                    rows=cols;
                    cols=rplReadNumberAsBINT(rplGetListElement(*var,2));
                    if(Exceptions) {
                        return;
                    }



                } else rows=0;

            }
            else if(ISNUMBER(**var)) {
                // IT HAS TO BE A NUMBER

                cols=rplReadNumberAsBINT(*var);
                if(Exceptions) {
                    return;
                }

                rows=0;

                if((cols<1)||(cols>65535))  {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }
                }
            else if(ISMATRIX(**var)) {
                        rows=rplMatrixRows(*var);
                        cols=rplMatrixCols(*var);
                    }
            else {
                rplError(ERR_INVALIDDIMENSION);
                return;
            }


            if( (rows<0)||(rows>65535)||(cols<1)||(cols>65535))  {
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

            // HERE WE HAVE PROPER ROWS AND COLUMNS

            WORDPTR newmat=rplMatrixFill(rows,cols,rplPeekData(1));

            if(newmat) {
                *var=newmat;
                if(var!=DSTop-2) rplDropData(2);
                else rplDropData(1);
            }

            return;
    }


    case COND:
    {
        //@SHORT_DESC=Column norm condition number of a matrix
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR *savestk=DSTop;

        if(!ISMATRIX(*rplPeekData(1))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        rplPushData(rplPeekData(1));
        rplMatrixInvert();
        if(Exceptions) { DSTop=savestk; return; }

        rplCallOperator(CMD_CNRM);
        if(Exceptions) { DSTop=savestk; return; }

        WORDPTR tmp=rplPeekData(2);
        rplOverwriteData(2,rplPeekData(1));
        rplOverwriteData(1,tmp);

        rplCallOperator(CMD_CNRM);
        if(Exceptions) { DSTop=savestk; return; }

        rplCallOvrOperator(CMD_OVR_MUL);
        if(Exceptions) { DSTop=savestk; return; }

        return;
    }


    case CROSS:
    {
        //@SHORT_DESC=Cross produce of vectors

    if(rplDepthData()<2) {
        rplError(ERR_BADARGCOUNT);
        return;
    }

    if(!ISMATRIX(*rplPeekData(2)) || !ISMATRIX(*rplPeekData(1))) {
        rplError(ERR_MATRIXEXPECTED);
        return;
    }
    WORDPTR *v1=DSTop-2,*v2=DSTop-1;
    BINT rows1=rplMatrixRows(*v1),rows2=rplMatrixRows(*v2);
    BINT cols1=rplMatrixCols(*v1),cols2=rplMatrixCols(*v2);



    if((rows1>1)||(rows2>1)) {
        rplError(ERR_VECTOREXPECTED);
        return;
    }
    if((cols1>3)||(cols2>3)) {
        rplError(ERR_INVALIDDIMENSION);
        return;
    }

    // FIRST COMPONENT

    if(cols1>=2) rplPushData(rplMatrixFastGet(*v1,1,2)); else rplPushData((WORDPTR)zero_bint);
    if(cols2>=3) rplPushData(rplMatrixFastGet(*v2,1,3)); else rplPushData((WORDPTR)zero_bint);
    rplCallOvrOperator(CMD_OVR_MUL);
    if(Exceptions) { DSTop=v2+1; return; }
    if(cols2>=2) rplPushData(rplMatrixFastGet(*v2,1,2)); else rplPushData((WORDPTR)zero_bint);
    if(cols1>=3) rplPushData(rplMatrixFastGet(*v1,1,3)); else rplPushData((WORDPTR)zero_bint);
    rplCallOvrOperator(CMD_OVR_MUL);
    if(Exceptions) { DSTop=v2+1; return; }
    rplCallOvrOperator(CMD_OVR_SUB);
    if(Exceptions) { DSTop=v2+1; return; }

    if(ISSYMBOLIC(*rplPeekData(1))) rplSymbAutoSimplify();
    if(Exceptions) { DSTop=v2+1; return; }

    // SECOND COMPONENT

    rplPushData(rplMatrixFastGet(*v1,1,1));
    if(cols2>=3) rplPushData(rplMatrixFastGet(*v2,1,3)); else rplPushData((WORDPTR)zero_bint);
    rplCallOvrOperator(CMD_OVR_MUL);
    if(Exceptions) { DSTop=v2+1; return; }
    rplPushData(rplMatrixFastGet(*v2,1,1));
    if(cols1>=3) rplPushData(rplMatrixFastGet(*v1,1,3)); else rplPushData((WORDPTR)zero_bint);
    rplCallOvrOperator(CMD_OVR_MUL);
    if(Exceptions) { DSTop=v2+1; return; }
    rplCallOvrOperator(CMD_OVR_SUB);
    if(Exceptions) { DSTop=v2+1; return; }

    if(ISSYMBOLIC(*rplPeekData(1))) rplSymbAutoSimplify();
    if(Exceptions) { DSTop=v2+1; return; }


    // THIRD COMPONENT

    rplPushData(rplMatrixFastGet(*v1,1,1));
    if(cols2>=2) rplPushData(rplMatrixFastGet(*v2,1,2)); else rplPushData((WORDPTR)zero_bint);
    rplCallOvrOperator(CMD_OVR_MUL);
    if(Exceptions) { DSTop=v2+1; return; }
    rplPushData(rplMatrixFastGet(*v2,1,1));
    if(cols1>=2) rplPushData(rplMatrixFastGet(*v1,1,2)); else rplPushData((WORDPTR)zero_bint);
    rplCallOvrOperator(CMD_OVR_MUL);
    if(Exceptions) { DSTop=v2+1; return; }
    rplCallOvrOperator(CMD_OVR_SUB);
    if(Exceptions) { DSTop=v2+1; return; }

    if(ISSYMBOLIC(*rplPeekData(1))) rplSymbAutoSimplify();
    if(Exceptions) { DSTop=v2+1; return; }

    WORDPTR newmat=rplMatrixComposeN(1,0,3);
    if(!newmat) { DSTop=v2+1; return; }

    *v1=newmat;
    DSTop=v2;
    return;

    }

    case CSWP:
    {
        //@SHORT_DESC=Swap two columns in a matrix
        // EXPLODE A VECTOR INTO ITS COMPONENTS
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISMATRIX(*rplPeekData(3))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }
        WORDPTR *m=DSTop-3;
        BINT rows=rplMatrixRows(*m);
        BINT cols=rplMatrixCols(*m);

        if(!ISNUMBER(*rplPeekData(1)) || !ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }
        BINT64 cfrom,cto;
        cfrom=rplReadNumberAsBINT(rplPeekData(2));
        cto=rplReadNumberAsBINT(rplPeekData(1));

        if( (cfrom<1)||(cfrom>cols) || (cto<1) || (cto>cols)) {
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }

// SWAP THE COLUMNS INTERNALLY TO REDUCE OVERHEAD
        WORDPTR newmat=rplMakeNewCopy(*m);
        WORD offset;
        BINT i;


#define MATOFFSET(matrix,row,col) ((matrix)[2+(((row)-1)*cols+((col)-1))])


        for(i=1;i<=rows;++i) {
            offset=MATOFFSET(newmat,i,cfrom);
            MATOFFSET(newmat,i,cfrom)=MATOFFSET(newmat,i,cto);
            MATOFFSET(newmat,i,cto)=offset;
        }

#undef MATOFFSET



        rplDropData(2);
        rplOverwriteData(1,newmat);

        return;
    }

    case DET:
        {
        //@SHORT_DESC=Determinant of a matrix
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR *a=DSTop-1,*savestk=DSTop;

            if(!ISMATRIX(**a)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            BINT rows,cols;
            rows=rplMatrixRows(*a);
            cols=rplMatrixCols(*a);

            // ONLY ACCEPT SQUARE MATRICES
            if(rows!=cols) {
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

            // PREPARE THE PERMUTATIONS INDEX
            WORDPTR idx=rplMatrixInitIdx(rows);
            if(!idx) return;

            rplPushData(idx);
            rplPushData(*a);
            a+=2;

            rplMatrixExplode();
            if(Exceptions) return;

            // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
            rplMatrixBareissEx(a,a-1,rows,cols,0);
            if(Exceptions) {
                DSTop=savestk;
                return;
            }

            // THE PARTIAL DETERMINANTS ARE IN THE DIAGONAL,
            // THE DETERMINANT OF THE WHOLE MATRIX IS Ukk

            // FIRST CHECK THE SIGN CHANGE DUE TO ROW PERMUTATIONS

            BINT k,count=0;
            idx=*(a-1); // RETRIEVE THE INDEX ADDRESS IN CASE IT MOVED
            for(k=1;k<=rows;++k) if(idx[k]!=(WORD)k) ++count;
            if(count&2) {
                rplCallOvrOperator(CMD_OVR_NEG);
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }
            }


            WORDPTR det=rplPeekData(1);
            DSTop=savestk;
            rplOverwriteData(1,det);
            return;
        }

    case DIAGMAP:
    {
        // TODO: COMPUTE EIGENVECTORS, FORM A BASE CONVERSION MATRIX P SO P^-1*D*P = A
        // APPLY THE FUNCTIONAL OPERATOR TO THE MATRIX D, ELEMENT BY ELEMENT
        // MULTIPLY BY P AND P^-1 TO FIND f(A)

        return;
    }

    case DOT:
    {
        //@SHORT_DESC=Internal product (dot product) of vectors

    if(rplDepthData()<2) {
        rplError(ERR_BADARGCOUNT);
        return;
    }

    if(!ISMATRIX(*rplPeekData(2)) || !ISMATRIX(*rplPeekData(1))) {
        rplError(ERR_MATRIXEXPECTED);
        return;
    }
    WORDPTR *v1=DSTop-2,*v2=DSTop-1;
    BINT rows1=rplMatrixRows(*v1),rows2=rplMatrixRows(*v2);
    BINT cols1=rplMatrixCols(*v1),cols2=rplMatrixCols(*v2);



    if((rows1>1)||(rows2>1)) {
        rplError(ERR_VECTOREXPECTED);
        return;
    }
    if(cols1>cols2) cols1=cols2;

    // FIRST COMPONENT

    BINT k;
    for(k=1;k<=cols1;++k)
    {
    rplPushData(rplMatrixFastGet(*v1,1,k));
    rplPushData(rplMatrixFastGet(*v2,1,k));
    rplCallOvrOperator(CMD_OVR_MUL);
    if(Exceptions) { DSTop=v2+1; return; }
    if(k>1) {
        rplCallOvrOperator(CMD_OVR_ADD);
        if(Exceptions) { DSTop=v2+1; return; }
    }
    }

    if(ISSYMBOLIC(*rplPeekData(1))) rplSymbAutoSimplify();
    if(Exceptions) { DSTop=v2+1; return; }

    *v1=rplPeekData(1);
    DSTop=v2;
    return;

    }

        case EGV:
        {

            // TODO:

        return;
        }
        case EGVL:
        {

            // TODO:

            return;
        }

        case GRAMSCHMIDT:
        {

        return;
        }

        case HADAMARD:
    {
        //@SHORT_DESC=Multiply corresponding elements in a matrix
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        WORDPTR a=rplPeekData(2),b=rplPeekData(1);

        if(!ISMATRIX(*a))
        {
            // CHECK DIMENSIONS OF MATRIX B
            // OPERATION IS VALID IF B IS A SCALAR
            if(ISMATRIX(*b)){
                BINT rows=rplMatrixRows(b);
                BINT cols=rplMatrixCols(b);
                if( (rows<2)&&(cols==1)) {
                    // OPERATION IS SCALAR, ACCEPT IT
                    WORDPTR scalar=rplMatrixGet(b,1,1);
                    rplOverwriteData(1,scalar);
                    rplCallOvrOperator(CurOpcode);
                    return;
                }
            }
                rplError(ERR_INCOMPATIBLEDIMENSION);
                return;
        }
        if(!ISMATRIX(*b))
        {
            // CHECK DIMENSIONS OF MATRIX A
            // OPERATION IS VALID IF A IS A SCALAR
            if(ISMATRIX(*a)){
                BINT rows=rplMatrixRows(a);
                BINT cols=rplMatrixCols(a);
                if( (rows<2)&&(cols==1)) {
                    // OPERATION IS SCALAR, ACCEPT IT
                    WORDPTR scalar=rplMatrixGet(a,1,1);
                    rplOverwriteData(2,scalar);
                    rplCallOvrOperator(CurOpcode);
                    return;
                }
            }
                rplError(ERR_INCOMPATIBLEDIMENSION);
                return;
        }

        rplMatrixHadamard();
        return;
    }

        case HILBERT:
        {
        //@SHORT_DESC=Assemble a Hilbert symbolic array
            // COMPUTE THE HILBERT MATRIX AS SYMBOLIC
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        WORDPTR *savestk=DSTop;
        BINT64 n=rplReadNumberAsBINT(rplPeekData(1));

        if(n<1) {
            rplError(ERR_INVALIDDIMENSION);
            return;
        }


        BINT i,j;

        for(i=1;i<=n;++i) {
            for(j=1;j<=n;++j) {
                if((i>1)&&(j<n)) rplPushData(rplPeekData(n-1));
                    else {
                    // CREATE THE FRACTION IN CANONICAL FORM TO SAVE THE AUTOSIMPLIFY STEP
                    rplPushData((WORDPTR)one_bint);
                    if(i+j-1>1) {
                    rplNewBINTPush(i+j-1,DECBINT);
                    if(Exceptions) { DSTop=savestk; return; }
                    rplSymbApplyOperator(CMD_OVR_INV,1);
                    if(Exceptions) { DSTop=savestk; return; }
                    rplSymbApplyOperator(CMD_OVR_MUL,2);
                    if(Exceptions) { DSTop=savestk; return; }
                    }
                }
                }


            }


        WORDPTR newmat=rplMatrixCompose(n,n);

            if(!newmat) { DSTop=savestk; return; }

            DSTop=savestk;
            rplOverwriteData(1,newmat);
            return;
        }

        case IBASIS:
        {
        //@SHORT_DESC=Find a basis of the intersection of two vector spaces
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR *a=DSTop-1,*b=DSTop-2;
            WORDPTR *savestk=DSTop;

            if(!ISLIST(**a) || !ISLIST(**b)) {
                rplError(ERR_LISTEXPECTED);
                return;
            }

            WORDPTR lelem=*a+1,endoflist=rplSkipOb(*a);
            BINT ncols=0,nvecs=0,allzeros;
            BINT i,j,k;

            while( (lelem<endoflist) && (*lelem!=CMD_ENDLIST) ) {

            if(!ISMATRIX(*lelem)) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }
            if(rplMatrixRows(lelem)>1) {
                rplError(ERR_VECTOREXPECTED);
                return;

            }
            if(ncols) {
                if(rplMatrixCols(lelem)!=ncols) {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }
            } else ncols=rplMatrixCols(lelem);

            //  EXPAND THE VALID VECTOR
            BINT off=lelem-*a;  // PROTECT FROM GC
            for(k=1;k<=ncols;++k) {
                rplPushData(rplMatrixGet(lelem,1,k));
                if(Exceptions) { DSTop=savestk; return; }
            }
            lelem=*a+off;
            ++nvecs;
            lelem=rplSkipOb(lelem);
            }

            lelem=*b+1;
            endoflist=rplSkipOb(*b);

            while( (lelem<endoflist) && (*lelem!=CMD_ENDLIST) ) {

            if(!ISMATRIX(*lelem)) {
                rplError(ERR_VECTOREXPECTED);
                return;
            }
            if(rplMatrixRows(lelem)>1) {
                rplError(ERR_VECTOREXPECTED);
                return;

            }
            if(ncols) {
                if(rplMatrixCols(lelem)!=ncols) {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }
            } else ncols=rplMatrixCols(lelem);

            //  EXPAND THE VALID VECTOR
            BINT off=lelem-*b;  // PROTECT FROM GC
            for(k=1;k<=ncols;++k) {
                rplPushData(rplMatrixGet(lelem,1,k));
                if(Exceptions) { DSTop=savestk; return; }
            }
            lelem=*b+off;
            ++nvecs;
            lelem=rplSkipOb(lelem);
            }

            // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
            rplMatrixBareissEx(a,0,nvecs,ncols,1);
            if(Exceptions) {
                DSTop=savestk;
                return;
            }


            rplMatrixBackSubstEx(a,nvecs,ncols);
            if(Exceptions) {
                DSTop=savestk;
                return;
            }

            // EXPAND THE MATRIX INTO ITS ROWS, BUT ELIMINATE ALL NULL ONES

            WORDPTR *base=DSTop;

            for(i=1;i<=nvecs;++i) {

            allzeros=1;
            for(j=1;j<=ncols;++j) {
                if(!rplSymbIsZero(base[-j])) { allzeros=0; break; }
            }
            if(allzeros) {
                // DROP THE ROWS THAT ARE ALL ZERO
                --base;
                rplRemoveAtData(DSTop-base,ncols);
                --nvecs;
                --i;
                base-=ncols-1;
            }
            else {
                // MAKE A ROW VECTOR AND LEAVE IT IN THE STACK
                --base;
                lelem=rplMatrixComposeN(DSTop-base,0,ncols);
                if(!lelem) { DSTop=savestk; return; }
                rplRemoveAtData(DSTop-base,ncols-1);
                base-=ncols-1;
                base[0]=lelem;
            }

            }


            // HERE WE HAVE nvecs IN THE STACK, READY TO PACK

            WORDPTR newlist=rplCreateListN(nvecs,1,1);
            if(Exceptions) { DSTop=savestk; return; }

            rplOverwriteData(1,newlist);
            return;
        }

        case IDN:
        {
            //@SHORT_DESC=Assemble an identity matrix
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            BINT64 rows,cols;
            WORDPTR *var=0;

            if(ISIDENT(*rplPeekData(1))) {

                var=rplFindLAM(rplPeekData(1),1);
                if(!var) var=rplFindGlobal(rplPeekData(1),1);
                if(!var) {
                    rplError(ERR_UNDEFINEDVARIABLE);
                    return;
                }
                ++var;
                if(!ISMATRIX(**var)) {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }

            }
            else var=DSTop-1;


            if(ISLIST(**var)) {
                BINT ndims=rplListLength(*var);
                if((ndims<1) || (ndims>2)) {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }

                cols=rplReadNumberAsBINT(rplGetListElement(*var,1));
                if(Exceptions) {
                    return;
                }

                if(ndims==2) {
                    rows=cols;
                    cols=rplReadNumberAsBINT(rplGetListElement(*var,2));
                    if(Exceptions) {
                        return;
                    }



                } else rows=0;

            }
            else if(ISNUMBER(**var)) {
                // IT HAS TO BE A NUMBER

                cols=rplReadNumberAsBINT(*var);
                if(Exceptions) {
                    return;
                }

                rows=0;

                }
            else if(ISMATRIX(**var)) {
                        rows=rplMatrixRows(*var);
                        cols=rplMatrixCols(*var);
                    }
            else {
                rplError(ERR_INVALIDDIMENSION);
                return;
            }


            if(!rows) rows=cols;

            if( (rows<0)||(rows>65535)||(cols<1)||(cols>65535) || (rows!=cols))  {
                rplError(ERR_INVALIDDIMENSION);
                return;
            }


            // HERE WE HAVE PROPER ROWS AND COLUMNS

            WORDPTR newmat=rplMatrixIdent(rows);

            if(newmat) {
                *var=newmat;
                if(var!=DSTop-1) rplDropData(1);
            }

            return;
    }

        case IMAGE:
        {

            //@SHORT_DESC=Find a basis of the image of a linear application
        // THE IMAGE OF THE MATRIX, SIMILAR TO BASIS USING THE COLUMNS AS THE VECTORS OF THE BASE
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR *a=DSTop-1;
            WORDPTR *savestk=DSTop;

            if(!ISMATRIX(**a)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            BINT nrows=rplMatrixRows(*a);
            BINT ncols=rplMatrixCols(*a);

            if(!nrows) {
                rplError(ERR_INVALIDDIMENSION);
                return;
            }


            rplMatrixExplodeByCols();
            if(Exceptions) {
                DSTop=savestk;
                return;
            }


            // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
            rplMatrixBareissEx(a,0,ncols,nrows,1);
            if(Exceptions) {
                DSTop=savestk;
                return;
            }


            rplMatrixBackSubstEx(a,ncols,nrows);
            if(Exceptions) {
                DSTop=savestk;
                return;
            }

            // EXPAND THE MATRIX INTO ITS ROWS, BUT ELIMINATE ALL NULL ONES

            WORDPTR *base=DSTop;
            BINT nvecs=ncols,allzeros;
            BINT i,j;

            for(i=1;i<=nvecs;++i) {

            allzeros=1;
            for(j=1;j<=nrows;++j) {
                if(!rplSymbIsZero(base[-j])) { allzeros=0; break; }
            }
            if(allzeros) {
                // DROP THE ROWS THAT ARE ALL ZERO
                --base;
                rplRemoveAtData(DSTop-base,nrows);
                --nvecs;
                --i;
                base-=nrows-1;
            }
            else {
                // MAKE A ROW VECTOR AND LEAVE IT IN THE STACK
                --base;
                WORDPTR newvect=rplMatrixComposeN(DSTop-base,0,nrows);
                if(!newvect) { DSTop=savestk; return; }
                rplRemoveAtData(DSTop-base,nrows-1);
                base-=nrows-1;
                base[0]=newvect;
            }

            }


            // HERE WE HAVE nvecs IN THE STACK, READY TO PACK

            WORDPTR newlist=rplCreateListN(nvecs,1,1);
            if(Exceptions) { DSTop=savestk; return; }

            rplOverwriteData(1,newlist);
            return;
        }

        case ISOM:
    {
        // TODO:

        return;
    }

        case JORDAN:
    {
        // TODO:
        return;
    }
        case KER:
    {

        //@SHORT_DESC=Find a basis for the kernel of a linear application
    // THE KERNEL OF THE MATRIX
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR *a=DSTop-1;
        WORDPTR *savestk=DSTop;

        if(!ISMATRIX(**a)) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        BINT nrows=rplMatrixRows(*a);
        BINT ncols=rplMatrixCols(*a);

        if(!nrows) {
            rplError(ERR_INVALIDDIMENSION);
            return;
        }


        WORDPTR *first=rplMatrixExplode();
        if(Exceptions) {
            DSTop=savestk;
            return;
        }


        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        rplMatrixBareissEx(a,0,nrows,ncols,1);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }


        rplMatrixBackSubstEx(a,nrows,ncols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }

        BINT i,j,k,nvecs;

        for(i=1;i<=ncols;++i)
        {
            if(i<=nrows) {
                for(j=1;j<ncols;++j) {
                    if(!rplSymbIsZero(*rplMatrixFastGetEx(first,ncols,i,j))) {
                        // j=COLUMN OF THE LEFTMOST NON-ZERO ELEMENT IN THIS ROW
                        while(j>i) {
                            // INSERT A NEW ROW
                            WORDPTR *rowptr=rplMatrixFastGetEx(first,ncols,i,1);
                            // MAKE ROOM
                            rplExpandStack(ncols);
                            if(Exceptions) { DSTop=savestk; return; }
                            memmovew(rowptr+ncols,rowptr,(DSTop-rowptr)*sizeof(WORDPTR)/sizeof(WORD));
                            DSTop+=ncols;
                            ++nrows;
                            for(k=1;k<=ncols;++k,++rowptr) *rowptr=(WORDPTR)zero_bint;
                            ++i;
                        }

                        break;

                        }


                    }

                }
            else {
               // NEED TO ADD A ROW AT THE END
                for(k=1;k<=ncols;++k) {
                    rplPushData((WORDPTR)zero_bint);
                    if(Exceptions) { DSTop=savestk; return; }
                }
                ++nrows;

            }
        }

        // HERE THE MATRIX IS COMPLETE, JUST EXTRACT THE COLUMNS WITH A ZERO IN THE DIAGONAL

        nvecs=0;

         for(j=1;j<=ncols;++j) {

                    if(rplSymbIsZero(*rplMatrixFastGetEx(first,ncols,j,j))) {

                    for(k=1;k<=nrows;++k) {
                        if(k==j) rplPushData((WORDPTR)minusone_bint);
                        else rplPushData(*rplMatrixFastGetEx(first,ncols,k,j));
                        if(Exceptions) {
                            DSTop=savestk;
                            return;
                        }
                    }

                    // CREATE A VECTOR
                    WORDPTR newvec=rplMatrixCompose(0,ncols);
                    if(!newvec) {
                        DSTop=savestk;
                        return;
                    }
                    rplDropData(ncols-1);
                    rplOverwriteData(1,newvec);
                    ++nvecs;

                    }
        }

        // HERE WE HAVE nvecs IN THE STACK, READY TO PACK

        WORDPTR newlist=rplCreateListN(nvecs,1,1);
        if(Exceptions) { DSTop=savestk; return; }
        DSTop=savestk;
        rplOverwriteData(1,newlist);
        return;
    }

        case LQ:
    {
        // TODO:

        return;
    }

        case LSQ:
    {
        // TODO:

        return;
    }

        case LU:
    {
        //@SHORT_DESC=LU factorization of a matrix
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR *a=DSTop-1,*savestk=DSTop;

        if(!ISMATRIX(**a)) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        BINT rows,cols;
        rows=rplMatrixRows(*a);
        cols=rplMatrixCols(*a);

        // ONLY ACCEPT SQUARE MATRICES
        if(rows!=cols) {
            rplError(ERR_INVALIDDIMENSION);
            return;
        }

        // EXPLODE THE MATRIX IN THE STACK
        WORDPTR *first=rplMatrixExplode();
        if(Exceptions) return;

        // INITIALIZE A PERMUTATION INDEX
        WORDPTR idx=rplMatrixInitIdx(rows),*indexptr;
        if(!idx || Exceptions) { DSTop=savestk; return; }
        indexptr=DSTop;
        rplPushData(idx);

        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        BINT canreduce=rplMatrixBareissEx(a,indexptr,rows,cols,0);
        if(!canreduce) rplError(ERR_SINGULARMATRIX);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }


        // SPLIT L AND U

        WORDPTR *lfirst=DSTop;
        BINT k,j;

        rplExpandStack(rows*cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }
        DSTop+=rows*cols;


        // EXTRACT L

        for(j=1;j<=cols;++j) {

            for(k=1;k<=rows;++k) {
                if(k>=j) {

                    rplPushData(*rplMatrixFastGetEx(first,cols,k,j));
                    // SPLIT THE DIAGONAL FACTOR Dii = u(i-1,i-1) FOR L AND Dii=u(i,i) FOR U
                    if(j>1) {
                        rplPushData(*rplMatrixFastGetEx(first,cols,j-1,j-1));
                        if(Exceptions) {
                            DSTop=savestk;
                            return;
                        }

                        rplCallOvrOperator(CMD_OVR_DIV);
                    }

                }
                else rplPushData((WORDPTR)zero_bint);
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }

                lfirst[(k-1)*cols+(j-1)]=rplPopData();
            }

        }


        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        WORDPTR newmat=rplMatrixCompose(rows,cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }


        DSTop=lfirst+1;
        *lfirst=newmat;


        // NOW DO THE SAME WITH U
        rplExpandStack(rows*cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }
        DSTop+=rows*cols;

        WORDPTR *ufirst=DSTop;

        for(k=1;k<=rows;++k) {
            for(j=1;j<=cols;++j) {
                if(j<k) rplPushData((WORDPTR)zero_bint);
                else if(j==k) rplPushData((WORDPTR)one_bint);
                      else {
                    rplPushData(*rplMatrixFastGetEx(first,cols,k,j));
                    // SPLIT THE DIAGONAL FACTOR Dii = u(i-1,i-1) FOR L AND Dii=u(i,i) FOR U
                    // TO OBTAIN CROUT
                    rplPushData(*rplMatrixFastGetEx(first,cols,k,k));
                    rplCallOvrOperator(CMD_OVR_DIV);
                }
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }
            }
        }

        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        newmat=rplMatrixCompose(rows,cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }

        DSTop=ufirst+1;
        *ufirst=newmat;

        // FINALLY, CREATE A PERMUTATION MATRIX P
        for(k=1;k<=rows;++k) {
            for(j=1;j<=cols;++j) {
                if((*indexptr)[k]==(WORD)j) rplPushData((WORDPTR)one_bint);
                    else rplPushData((WORDPTR)zero_bint);
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }
            }
        }


        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        newmat=rplMatrixCompose(rows,cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }


        // NOW CLEANUP THE STACK
        savestk[-1]=*lfirst;
        savestk[0]=*ufirst;
        savestk[1]=newmat;

        DSTop=savestk+2;
        return;
    }




        case MAD:
    {
        // TODO:

        return;
    }

        case MKISOM:
    {
        // TODO:

        return;
    }

        case PMINI:
        {

            //@SHORT_DESC=Minimal polynomial of a matrix
        // THE MINIMAL POLYNOMIAL OF THE MATRIX
        // OBTAINED BY A^2
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR *a=DSTop-1;
            WORDPTR *savestk=DSTop;

            if(!ISMATRIX(**a)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            BINT nrows=rplMatrixRows(*a);
            BINT ncols=rplMatrixCols(*a);
            BINT k,j;

            if(nrows!=ncols) {
                rplError(ERR_SQUAREMATRIXONLY);
                return;
            }

            // GENERATE CANONICAL VECTOR [ 1 0 0 0 ... 0 ]

            rplPushData((WORDPTR)one_bint);
            if(Exceptions) { DSTop=savestk; return; }
            for(k=1;k<nrows;++k)
            { rplPushData((WORDPTR)zero_bint);
                if(Exceptions) { DSTop=savestk; return; }
            }
            WORDPTR newvec=rplMatrixCompose(0,ncols);
            if(Exceptions) { DSTop=savestk; return; }
            a[1]=newvec;
            rplDropData(nrows-1);

            // HERE WE HAVE THE MATRIX A AND THE CANONICAL VECTOR
            for(k=1;k<=nrows;++k) {
                rplPushData(a[0]);  // matrix A
                if(Exceptions) { DSTop=savestk; return; }
                rplPushData(rplPeekData(2));    // VECTOR A^(k-1)*e1
                if(Exceptions) { DSTop=savestk; return; }
                rplMatrixMul(); // MULTIPLY AND LEAVE NEW VECTOR IN THE STACK
                if(Exceptions) { DSTop=savestk; return; }

            }

            // HERE WE HAVE e1, A*e1, A^2*e1, ... A^n*e1 AT a[1]...a[n+1]

            // NOW USE THE ELEMENTS TO CREATE AN EXPLODED AUGMENTED MATRIX
            WORDPTR *firstelem=DSTop;
            rplExpandStack(nrows*(nrows+1));
            DSTop+=nrows*(nrows+1);
            if(Exceptions) { DSTop=savestk; return; }


            for(k=1;k<=(nrows+1);++k) {
                for(j=1;j<=nrows;++j) {
                *rplMatrixFastGetEx(firstelem,nrows+1,j,k)=rplMatrixFastGet(a[k],1,j);
                }
            }

            // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
            rplMatrixBareissEx(firstelem-1,0,nrows,nrows+1,1);
            if(Exceptions) {
                DSTop=savestk;
                return;
            }

            rplMatrixBackSubstEx(firstelem-1,nrows,nrows+1);
            if(Exceptions) {
                DSTop=savestk;
                return;
            }

            // HERE WE HAVE THE ROW-REDUCED FORM OF THE MATRIX
            // THE COLUMN NEXT TO THE LAST ROW WITH A NON-ZERO DIAGONAL
            // EXCEPT FOR THE FIRST COEFFICIENT, WHICH IS ONE

            rplPushData((WORDPTR)one_bint);
            if(Exceptions) {
                DSTop=savestk;
                return;
            }
            for(j=0,k=nrows;k>=1;--k) {
                if(rplSymbIsZero(*rplMatrixFastGetEx(firstelem,nrows+1,k,k))) continue;
                if(!j) j=k+1;
                rplPushData(*rplMatrixFastGetEx(firstelem,nrows+1,k,j));
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }
                rplCallOvrOperator(CMD_OVR_NEG);
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }
            }

            WORDPTR poly=rplMatrixCompose(0,j);
            if(Exceptions) {
                DSTop=savestk;
                return;
            }
            a[0]=poly;
            DSTop=savestk;

            return;
        }

        case QR:
    {
        // TODO:
        return;
    }


        case RANK:
    {
        //@SHORT_DESC=Rank of a matrix
        // RANK COMPUTED FROM BAREISS FACTORIZATION
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            WORDPTR *a=DSTop-1,*savestk=DSTop;

            if(!ISMATRIX(**a)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }

            BINT rows,cols;
            rows=rplMatrixRows(*a);
            cols=rplMatrixCols(*a);

            rplMatrixExplode();
            if(Exceptions) return;

            // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
            rplMatrixBareissEx(a,0,rows,cols,1);

            if(Exceptions) {
                DSTop=savestk;
                return;
            }


            // COUNT THE ROWS WITH A ZERO IN THE DIAGONAL
            BINT k;
            BINT rank=rows;
            for(k=1;k<=rows;++k) {
                if(rplSymbIsZero(*rplMatrixFastGetEx(a+1,cols,k,k))) --rank;
            }

            DSTop=savestk;
            WORDPTR rankobj=rplNewBINT(rank,DECBINT);
            if(!rankobj) return;

            rplOverwriteData(1,rankobj);

            return;
    }

        case RANM:
    {
            //@SHORT_DESC=Assemble a matrix with random numbers
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            BINT64 rows,cols;
            WORDPTR *var=0;

            if(ISIDENT(*rplPeekData(1))) {

                var=rplFindLAM(rplPeekData(1),1);
                if(!var) var=rplFindGlobal(rplPeekData(1),1);
                if(!var) {
                    rplError(ERR_UNDEFINEDVARIABLE);
                    return;
                }
                ++var;
                if(!ISMATRIX(**var)) {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }

            }
            else var=DSTop-1;


            if(ISLIST(**var)) {
                BINT ndims=rplListLength(*var);
                if((ndims<1) || (ndims>2)) {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }

                cols=rplReadNumberAsBINT(rplGetListElement(*var,1));
                if(Exceptions) {
                    return;
                }

                if(ndims==2) {
                    rows=cols;
                    cols=rplReadNumberAsBINT(rplGetListElement(*var,2));
                    if(Exceptions) {
                        return;
                    }



                } else rows=0;

            }
            else if(ISNUMBER(**var)) {
                // IT HAS TO BE A NUMBER

                cols=rplReadNumberAsBINT(*var);
                if(Exceptions) {
                    return;
                }

                rows=0;

                if((cols<1)||(cols>65535))  {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }
                }
            else if(ISMATRIX(**var)) {
                        rows=rplMatrixRows(*var);
                        cols=rplMatrixCols(*var);
                    }
            else {
                rplError(ERR_INVALIDDIMENSION);
                return;
            }


            if( (rows<0)||(rows>65535)||(cols<1)||(cols>65535))  {
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

            // HERE WE HAVE PROPER ROWS AND COLUMNS
            BINT nelem=rows? rows*cols:cols;


            rplExpandStack(nelem);
            if(Exceptions) return;

            WORDPTR *first=DSTop;
            BINT k;
            BINT random;
            for(k=0;k<nelem;++k) {
                random=(rplRandomNext()>>8)%19;
                rplNewBINTPush(random-9,DECBINT);
                if(Exceptions) { DSTop=first; return; }
            }

            WORDPTR newmat=rplMatrixCompose(rows,cols);

            DSTop=first;

            if(newmat) rplOverwriteData(1,newmat);
            return;
    }

        case RCI:
    {
        //@SHORT_DESC=Multiply a row by a constant
        // MULTIPLY A ROW BY A CONSTANT
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISMATRIX(*rplPeekData(3))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }
        WORDPTR *m=DSTop-3;
        BINT rows=rplMatrixRows(*m);
        BINT cols=rplMatrixCols(*m);

        if(!ISNUMBER(*rplPeekData(1)) ) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        if(!rows) { rows=cols; cols=1; }
        BINT64 row;
        row=rplReadNumberAsBINT(rplPeekData(1));

        if( (row<1)||(row>rows) ) {
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }

        WORDPTR *savestk=DSTop,*factor=DSTop-2;
        rplPushData(rplPeekData(3));

        WORDPTR *first=rplMatrixExplode();

        if(Exceptions) { DSTop=savestk; return; }

        BINT i;


        for(i=1;i<=cols;++i) {
            rplPushData(*factor);
            rplPushData(*rplMatrixFastGetEx(first,cols,row,i));
            if(Exceptions) { DSTop=savestk; return; }
            rplCallOvrOperator(CMD_OVR_MUL);
            if(Exceptions) { DSTop=savestk; return; }
            if(ISSYMBOLIC(*rplPeekData(1))) {
                rplSymbAutoSimplify();
                if(Exceptions) { DSTop=savestk; return; }
            }
            *rplMatrixFastGetEx(first,cols,row,i)=rplPopData();
        }

        WORDPTR newmat=rplMatrixCompose(rows,cols);
        if(!newmat) { DSTop=savestk; return; }

        DSTop=savestk;

        rplOverwriteData(3,newmat);
        rplDropData(2);

        return;
    }


        case RCIJ:
    {
        //@SHORT_DESC=Multiply a row by a constant and add to other row
        // MULTIPLY A ROW BY A CONSTANT AND ADD TO OTHER ROW
        if(rplDepthData()<4) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISMATRIX(*rplPeekData(4))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }
        WORDPTR *m=DSTop-4;
        BINT rows=rplMatrixRows(*m);
        BINT cols=rplMatrixCols(*m);

        if(!ISNUMBER(*rplPeekData(1)) || !ISNUMBER(*rplPeekData(2)) ) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        if(!rows) { rows=cols; cols=1; }
        BINT64 rowf,rowto;
        rowf=rplReadNumberAsBINT(rplPeekData(2));
        rowto=rplReadNumberAsBINT(rplPeekData(1));


        if( (rowf<1)||(rowf>rows)||(rowto<1)||(rowto>rows) ) {
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }

        WORDPTR *savestk=DSTop,*factor=DSTop-3;
        rplPushData(rplPeekData(4));

        WORDPTR *first=rplMatrixExplode();

        if(Exceptions) { DSTop=savestk; return; }

        BINT i;


        for(i=1;i<=cols;++i) {
            rplPushData(*factor);
            rplPushData(*rplMatrixFastGetEx(first,cols,rowf,i));
            if(Exceptions) { DSTop=savestk; return; }
            rplCallOvrOperator(CMD_OVR_MUL);
            if(Exceptions) { DSTop=savestk; return; }

            rplPushData(*rplMatrixFastGetEx(first,cols,rowto,i));
            if(Exceptions) { DSTop=savestk; return; }
            rplCallOvrOperator(CMD_OVR_ADD);
            if(Exceptions) { DSTop=savestk; return; }

            if(ISSYMBOLIC(*rplPeekData(1))) {
                rplSymbAutoSimplify();
                if(Exceptions) { DSTop=savestk; return; }
            }
            *rplMatrixFastGetEx(first,cols,rowto,i)=rplPopData();
        }

        WORDPTR newmat=rplMatrixCompose(rows,cols);
        if(!newmat) { DSTop=savestk; return; }

        DSTop=savestk;

        rplOverwriteData(4,newmat);
        rplDropData(3);

        return;
    }

        case RDM:
    {
            //@SHORT_DESC=Change dimensions of an array
            if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
                return;
            }
            BINT64 rows,cols;
            WORDPTR *var=0;

            if(ISIDENT(*rplPeekData(2))) {

                var=rplFindLAM(rplPeekData(2),1);
                if(!var) var=rplFindGlobal(rplPeekData(2),1);
                if(!var) {
                    rplError(ERR_UNDEFINEDVARIABLE);
                    return;
                }
                ++var;
            }
            else var=DSTop-2;


            if(!ISMATRIX(**var)) {
                rplError(ERR_MATRIXEXPECTED);
                return;
            }



            // NOW CHECK THE NEW SIZE


            if(ISLIST(*rplPeekData(1))) {
                BINT ndims=rplListLength(rplPeekData(1));
                if((ndims<1) || (ndims>2)) {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }

                cols=rplReadNumberAsBINT(rplGetListElement(rplPeekData(1),1));
                if(Exceptions) {
                    return;
                }

                if(ndims==2) {
                    rows=cols;
                    cols=rplReadNumberAsBINT(rplGetListElement(rplPeekData(1),2));
                    if(Exceptions) {
                        return;
                    }



                } else rows=0;

            }
            else if(ISNUMBER(*rplPeekData(1))) {
                // IT HAS TO BE A NUMBER

                cols=rplReadNumberAsBINT(rplPeekData(1));
                if(Exceptions) {
                    return;
                }

                rows=0;

                if((cols<1)||(cols>65535))  {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }
                }
            else if(ISMATRIX(*rplPeekData(1))) {
                        rows=rplMatrixRows(rplPeekData(1));
                        cols=rplMatrixCols(rplPeekData(1));
                    }
            else {
                rplError(ERR_INVALIDDIMENSION);
                return;
            }


            if( (rows<0)||(rows>65535)||(cols<1)||(cols>65535))  {
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

            // HERE WE HAVE PROPER ROWS AND COLUMNS

            WORDPTR *savestk=DSTop;
            rplPushData(*var);

            WORDPTR *first=rplMatrixExplode();

            if(Exceptions) { DSTop=savestk; return; }

            BINT i,totalelem,actualelem;

            totalelem=rows? rows*cols:cols;

            actualelem=DSTop-first;

            if(actualelem>totalelem) rplDropData(actualelem-totalelem); // TRIM EXCESS ELEMENTS
            else {
                // FILL WITH ZEROS
                rplExpandStack(totalelem-actualelem);
                if(Exceptions) { DSTop=savestk; return; }
                for(i=0;i<totalelem-actualelem;++i) *DSTop++=(WORDPTR)zero_bint;
            }

            WORDPTR newmat=rplMatrixCompose(rows,cols);
            if(!newmat) { DSTop=savestk; return; }

            DSTop=savestk;

            rplOverwriteData(2,newmat);
            rplDropData(1);

            return;



            return;
    }

        case REF:
    {
        //@SHORT_DESC=Reduce matrix to echelon form (upper triangular form)
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR *a=DSTop-1,*savestk=DSTop;

        if(!ISMATRIX(**a)) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        BINT rows,cols;
        rows=rplMatrixRows(*a);
        cols=rplMatrixCols(*a);

        rplMatrixExplode();
        if(Exceptions) return;

        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        rplMatrixBareissEx(a,0,rows,cols,1);

        if(Exceptions) {
            DSTop=savestk;
            return;
        }

        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        WORDPTR newmat=rplMatrixCompose(rows,cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }
        DSTop=savestk;
        rplOverwriteData(1,newmat);

        return;
    }
        case RNRM:
    {
        //@SHORT_DESC=Row norm (infinity norm) of a matrix
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR *a=DSTop-1,*savestk=DSTop;

        if(!ISMATRIX(**a)) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        BINT rows,cols;
        rows=rplMatrixRows(*a);
        cols=rplMatrixCols(*a);

        if(!rows) { rows=cols; cols=1; }

        BINT i,j;

        for(j=1;j<=cols;++j) {
        for(i=1;i<=rows;++i) {
                rplPushData(rplMatrixFastGet(*a,i,j));
                rplCallOvrOperator(CMD_OVR_ABS);
                if(Exceptions) { DSTop=savestk; return; }
                if(j>1) {
                    rplCallOvrOperator(CMD_OVR_ADD);
                    if(Exceptions) { DSTop=savestk; return; }
                }
        }
        if(i>1) {
            rplCallOperator(CMD_MAX);
            if(Exceptions) { DSTop=savestk; return; }
        }
        }

        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbAutoSimplify();
            if(Exceptions) { DSTop=savestk; return; }
        }

        rplOverwriteData(2,rplPeekData(1));
        rplDropData(1);
        return;
    }

        case RREF:
    {
        //@SHORT_DESC=Fully reduce to row-reduced echelon form
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR *a=DSTop-1,*savestk=DSTop;

        if(!ISMATRIX(**a)) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        BINT rows,cols;
        rows=rplMatrixRows(*a);
        cols=rplMatrixCols(*a);

        rplMatrixExplode();
        if(Exceptions) return;

        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        rplMatrixBareissEx(a,0,rows,cols,1);

        if(Exceptions) {
            DSTop=savestk;
            return;
        }

        rplMatrixBackSubstEx(a,rows,cols);

        if(Exceptions) {
            DSTop=savestk;
            return;
        }


        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        WORDPTR newmat=rplMatrixCompose(rows,cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }
        DSTop=savestk;
        rplOverwriteData(1,newmat);

        return;
    }

        case RREFMOD:
    {
        // TODO:

        return ;
    }
        case RSD:
    {
        //@SHORT_DESC=Residual R=B-A*X' on a system A*X=B
        // PERFORM B-A*Z GIVEN B, A AND Z

        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISMATRIX(*rplPeekData(3)) || !ISMATRIX(*rplPeekData(2)) || !ISMATRIX(*rplPeekData(1)))
        {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        WORDPTR *savestk=DSTop;

        rplPushData(rplPeekData(3));
        rplPushData(rplPeekData(3));
        rplPushData(rplPeekData(3));
        if(Exceptions) { DSTop=savestk; return; }

        rplMatrixMul();

        if(Exceptions) { DSTop=savestk; return; }

        rplMatrixSub();

        if(Exceptions) { DSTop=savestk; return; }

        rplOverwriteData(3,rplPopData());
        rplDropData(2);
        return;
    }
        case RSWP:
    {
        //@SHORT_DESC=Swap two rows in a matrix
        // EXPLODE A VECTOR INTO ITS COMPONENTS
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISMATRIX(*rplPeekData(3))) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }
        WORDPTR *m=DSTop-3;
        BINT rows=rplMatrixRows(*m);
        BINT cols=rplMatrixCols(*m);

        if(!ISNUMBER(*rplPeekData(1)) || !ISNUMBER(*rplPeekData(2))) {
            rplError(ERR_INTEGEREXPECTED);
            return;
        }

        if(!rows) { rows=cols; cols=1; }
        BINT64 rfrom,rto;
        rfrom=rplReadNumberAsBINT(rplPeekData(2));
        rto=rplReadNumberAsBINT(rplPeekData(1));

        if( (rfrom<1)||(rfrom>rows) || (rto<1) || (rto>rows)) {
            rplError(ERR_INDEXOUTOFBOUNDS);
            return;
        }

// SWAP THE ROWS INTERNALLY TO REDUCE OVERHEAD
        WORDPTR newmat=rplMakeNewCopy(*m);
        WORD offset;
        BINT i;


#define MATOFFSET(matrix,row,col) ((matrix)[2+(((row)-1)*cols+((col)-1))])


        for(i=1;i<=cols;++i) {
            offset=MATOFFSET(newmat,rfrom,i);
            MATOFFSET(newmat,rfrom,i)=MATOFFSET(newmat,rto,i);
            MATOFFSET(newmat,rto,i)=offset;
        }

#undef MATOFFSET



        rplDropData(2);
        rplOverwriteData(1,newmat);

        return;
    }

        case SCHUR:
    {
        // TODO:
        return;
    }
        case SNRM:
    {
        // TODO:
        return;
    }
        case SRAD:
    {
        // TODO:
        return;
    }
        case SVD:
    {
        // TODO:
        return;
    }
        case SVL:
    {
        // TODO:
        return;
    }
        case SYLVESTER:
    {
        // TODO:
        return;
    }
        case TRACE:
    {
        //@SHORT_DESC=Sum of the items in the diagonal of a matrix
        // RETURN THE SUM OF THE DIAGONAL ITEMS
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        BINT k,rows,cols;

        if(ISMATRIX(*rplPeekData(1))) {
                    rows=rplMatrixRows(rplPeekData(1));
                    cols=rplMatrixCols(rplPeekData(1));
                }
        else {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }
        if(rows==0) rows=1;

        if(cols<rows) rows=cols;

        WORDPTR *mat=DSTop-1;

        for(k=1;k<=rows;++k)
        {
            rplPushData(rplMatrixFastGet(*mat,k,k));
            if(Exceptions) { DSTop= mat+1; return; }
            if(k>1) {
                rplCallOvrOperator(CMD_OVR_ADD);
                if(Exceptions) { DSTop= mat+1; return; }
            }
        }

        *mat=rplPopData();
        return;
    }
        case TRAN:
    {
        //@SHORT_DESC=Transpose a matrix
        // MATRIX TRANSPOSE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        BINT64 rows;
        WORDPTR *var=0;


        if(ISIDENT(*rplPeekData(1))) {

            var=rplFindLAM(rplPeekData(1),1);
            if(!var) var=rplFindGlobal(rplPeekData(1),1);
            if(!var) {
                rplError(ERR_UNDEFINEDVARIABLE);
                return;
            }
            ++var;
            if(!ISMATRIX(**var)) {
                rplError(ERR_INVALIDDIMENSION);
                return;
            }

        }
        else var=DSTop-1;


        if(ISMATRIX(**var)) {
                    rows=rplMatrixRows(*var);
                }
        else {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }
        if(rows==0) return;     // VECTORS DON'T NEED TO BE TRANSPOSED, BUT NO NEED TO ERROR ON THAT


        rplMatrixTranspose();

        return;
    }
        case TRN:
    {
        //@SHORT_DESC=Complex conjugate transpose of a matrix
                // COMPLEX CONJUGATE
                if(rplDepthData()<1) {
                    rplError(ERR_BADARGCOUNT);
                    return;
                }
                BINT64 rows,cols;
                WORDPTR *var=0;


                if(ISIDENT(*rplPeekData(1))) {

                    var=rplFindLAM(rplPeekData(1),1);
                    if(!var) var=rplFindGlobal(rplPeekData(1),1);
                    if(!var) {
                        rplError(ERR_UNDEFINEDVARIABLE);
                        return;
                    }
                    ++var;
                    if(!ISMATRIX(**var)) {
                        rplError(ERR_INVALIDDIMENSION);
                        return;
                    }

                }
                else var=DSTop-1;


                if(ISMATRIX(**var)) {
                            rows=rplMatrixRows(*var);
                            cols=rplMatrixCols(*var);
                        }
                else {
                    rplError(ERR_MATRIXEXPECTED);
                    return;
                }

                if(rows==0) {
                    // VECTORS DON'T NEED TO BE TRANSPOSED, JUST DO THE COMPLEX CONJUGATE
                    rplMatrixConj();
                    return;
                }

                if( (rows<1)||(rows>65535)||(cols<1)||(cols>65535))  {
                    rplError(ERR_INVALIDDIMENSION);
                    return;
                }


                // DO IT MANUALLY INSTEAD OF USING rplMatrixTranspose() and rplMatrixConj()
                // TO AVOID DOUBLE rplMatrixCompose() OVERHEAD


                WORDPTR *savestk=DSTop;

                WORDPTR *a=DSTop;

                rplPushData(*var);

                // TRANSPOSE IS DONE DURING EXPLODE
                rplMatrixExplodeByCols();
                if(Exceptions) { DSTop=savestk; return; }

                BINT i,j;

                // CONVENIENCE MACRO TO ACCESS ELEMENTS DIRECTLY ON THE STACK
                // a IS POINTING TO THE MATRIX, THE FIRST ELEMENT IS a[1]
#define STACKELEM(r,c) a[((r)-1)*rows+(c)]

                // NOW DO THE COMPLEX CONJUGATE
                for(i=1;i<=cols;++i)
                {
                    for(j=1;j<=rows;++j) {
                        rplPushData(STACKELEM(i,j));
                        if(Exceptions) { DSTop=savestk; return; }
                        rplCallOperator(CMD_CONJ);
                        if(Exceptions) { DSTop=savestk; return; }
                        if(ISSYMBOLIC(*rplPeekData(1))) {
                            rplSymbAutoSimplify();
                            if(Exceptions) { DSTop=savestk; return; }
                        }
                        STACKELEM(i,j)=rplPopData();
                    }
                }

#undef STACKELEM

                WORDPTR newmat=rplMatrixCompose(cols,rows);

                DSTop=savestk;
                if(newmat) {
                    *var=newmat;
                    if(var!=DSTop-1) rplDropData(1);
                }

        return;
    }
        case VANDERMONDE:
    {
        // TODO:
        return;
    }

        case LDUP:
    {
        //@SHORT_DESC=Decompose A into LDUP such that P*A=L*D<sup>-1</sup>*U
        //@NEW
        // ROUNDOFF-ERROR FREE DECOMPOSITION
        // DECOMPOSE A MATRIX AS P*A=L*D^-1*U
        // WHERE P=PERMUTATION MATRIX
        // L = LOWER TRIANGULAR
        // U = UPPER TRIANGULAR
        // D = DIAGONAL MATRIX, WHERE d(i,i)=u(i,i)*u(i-1,i-1), ASSUMING u(0,0)=1
        // THIS IS THE BASE FOR MANY DECOMPOSITIONS:
        // CHOLESKY:
        // SPLIT D^-1 INTO SQRT(D^-1)*SQRT(D^-1) --> (L*SQRT(D^-1)) * (SQRT(D^-1)*U) = L * LT
        // CROUT/DOLITTLE:
        // SPLIT D=Du*Dl --> Du(i,i)=u(i,i) AND Dl(i,i)=u(i-1,i-1)
        // CROUT: (L*Dl^-1) * (Du^-1 * U) = L' * U' WITH U' = UNIT UPPER TRIANGULAR.
        // DOLITTLE: (L*Du^-1) * (Dl^-1 * U) = L' * U' WITH L' = UNIT LOWER TRIANGULAR.



        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR *a=DSTop-1,*savestk=DSTop;

        if(!ISMATRIX(**a)) {
            rplError(ERR_MATRIXEXPECTED);
            return;
        }

        BINT rows,cols;
        rows=rplMatrixRows(*a);
        cols=rplMatrixCols(*a);

        // ONLY ACCEPT SQUARE MATRICES
        if(rows!=cols) {
            rplError(ERR_INVALIDDIMENSION);
            return;
        }

        // EXPLODE THE MATRIX IN THE STACK
        WORDPTR *first=rplMatrixExplode();
        if(Exceptions) return;

        // INITIALIZE A PERMUTATION INDEX
        WORDPTR idx=rplMatrixInitIdx(rows),*indexptr;
        if(!idx || Exceptions) { DSTop=savestk; return; }
        indexptr=DSTop;
        rplPushData(idx);

        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        BINT canreduce=rplMatrixBareissEx(a,indexptr,rows,cols,0);
        if(!canreduce) rplError(ERR_SINGULARMATRIX);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }


        // SPLIT L AND U

        WORDPTR *lfirst=DSTop;
        BINT k,j;

        rplExpandStack(rows*cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }
        DSTop+=rows*cols;


        // EXTRACT L

        for(j=1;j<=cols;++j) {

            for(k=1;k<=rows;++k) {
                if(k>=j) {
                    rplPushData(*rplMatrixFastGetEx(first,cols,k,j));
                }
                else rplPushData((WORDPTR)zero_bint);
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }

                lfirst[(k-1)*cols+(j-1)]=rplPopData();
            }

        }


        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        WORDPTR newmat=rplMatrixCompose(rows,cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }


        DSTop=lfirst+1;
        *lfirst=newmat;


        // NOW DO THE SAME WITH U
        rplExpandStack(rows*cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }
        DSTop+=rows*cols;

        WORDPTR *dfirst=DSTop;

        for(k=1;k<=rows;++k) {
            for(j=1;j<=cols;++j) {
                if(j!=k) rplPushData((WORDPTR)zero_bint);
                 else {
                    rplPushData(*rplMatrixFastGetEx(first,cols,k,k));
                    if(k>1) {
                    rplPushData(*rplMatrixFastGetEx(first,cols,k-1,k-1));
                    rplCallOvrOperator(CMD_OVR_MUL);
                    }
                }
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }
            }
        }

        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        newmat=rplMatrixCompose(rows,cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }

        DSTop=dfirst+1;
        *dfirst=newmat;




        // NOW DO THE SAME WITH U
        rplExpandStack(rows*cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }
        DSTop+=rows*cols;

        WORDPTR *ufirst=DSTop;

        for(k=1;k<=rows;++k) {
            for(j=1;j<=cols;++j) {
                if(j<k) rplPushData((WORDPTR)zero_bint);
                 else {
                    rplPushData(*rplMatrixFastGetEx(first,cols,k,j));
                }
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }
            }
        }

        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        newmat=rplMatrixCompose(rows,cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }

        DSTop=ufirst+1;
        *ufirst=newmat;

        // FINALLY, CREATE A PERMUTATION MATRIX P
        for(k=1;k<=rows;++k) {
            for(j=1;j<=cols;++j) {
                if((*indexptr)[k]==(WORD)j) rplPushData((WORDPTR)one_bint);
                    else rplPushData((WORDPTR)zero_bint);
                if(Exceptions) {
                    DSTop=savestk;
                    return;
                }
            }
        }


        // HERE WE HAVE ALL ELEMENTS OF THE MATRIX ALREADY EXPLODED
        newmat=rplMatrixCompose(rows,cols);
        if(Exceptions) {
            DSTop=savestk;
            return;
        }


        // NOW CLEANUP THE STACK
        savestk[-1]=*lfirst;
        savestk[0]=*dfirst;
        savestk[1]=*ufirst;
        savestk[2]=newmat;

        DSTop=savestk+3;
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

        // CHECK IF THE TOKEN IS THE OPEN BRACKET

        if(*((char * )TokenStart)=='[')
        {
            if(LIBNUM(CurrentConstruct)==LIBRARY_NUMBER) {
                // WE ARE COMPILING OBJECTS INSIDE A MATRIX ALREADY
               if(CurrentConstruct==MKPROLOG(LIBRARY_NUMBER,0)) {
                // WE ARE IN THE OUTER DIMENSION
                // INCREASE DEPTH OF DIMENSION AND ACCEPT
                // WARNING, THIS USES INTERNAL COMPILER WORKINGS
                WORDPTR matrix=*(ValidateTop-1);
                ++*matrix;
                // CHECK IF THERE IS A SIZE WORD YET
                if(CompileEnd==matrix+1) {
                    // THIS IS THE FIRST OBJECT IN THE ARRAY
                    // ADD A DUMMY WORD
                    rplCompileAppend(MATMKSIZE(1,0));
                }
                else {
                    // THERE SHOULD BE A SIZE WORD ALREADY
                    // INCREASE THE ROW COUNT
                    BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
                    if(!rows) {
                        // VECTOR CAN'T OPEN A SECOND DIMENSION
                        RetNum=ERR_SYNTAX;
                        return;
                    }
                    matrix[1]=MATMKSIZE(rows+1,cols);
                }



                if(TokenLen>1) NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                RetNum=OK_CONTINUE_NOVALIDATE;
                return;
                }
                else {
                    // MORE THAN 2 DIMENSIONS ARE NOT SUPPORTED
                    RetNum=ERR_NOTMINE;
                    return;
               }
            }

            rplCompileAppend((WORD) MKPROLOG(LIBRARY_NUMBER,0));
            if(TokenLen>1) {
                NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                RetNum=OK_STARTCONSTRUCT;
            }
            else RetNum=OK_STARTCONSTRUCT;
            return;
        }
        // CHECK IF THE TOKEN IS THE CLOSING BRACKET

        if(*utf8nskip((char * )TokenStart,(char *)BlankStart,TokenLen-1)==']')
        {

            if(TokenLen>1) {
                BlankStart=NextTokenStart=(WORDPTR)utf8nskip((char * )TokenStart,(char *)BlankStart,TokenLen-1);
                RetNum=ERR_NOTMINE_SPLITTOKEN;
                return;
            }

            if(LIBNUM(CurrentConstruct)!=LIBRARY_NUMBER) {
                RetNum=ERR_SYNTAX;
                return;
            }
            WORDPTR matrix=*(ValidateTop-1);
            BINT rows,cols;
            BINT totalelements;

            if(CompileEnd>matrix+1) {
            rows=MATROWS(matrix[1]);
            cols=MATCOLS(matrix[1]);
            if(rows==0) totalelements=cols;
            else totalelements=rows*cols;
            } else rows=cols=totalelements=0;

            if(CurrentConstruct!=MKPROLOG(LIBRARY_NUMBER,0)) {
                // CLOSED AN INNER DIMENSION

                // CAN'T CLOSE AN EMPTY MATRIX
                if(!totalelements) {
                    RetNum=ERR_SYNTAX;
                    return;
                }

                // DECREASE DIMENSION COUNT
                --*matrix;


                // CHECK FULL ROW SIZE IS CORRECT
                // BY CHECKING THE NEXT EMPTY OBJECT IS THE START OF A ROW

                BINT count;
                WORDPTR index=matrix+2;

                count=0;
                while((count<totalelements) && (index<CompileEnd)) { ++count; index=rplSkipOb(index); }

                if(count%cols) {
                    // INVALID MATRIX SIZE
                    RetNum=ERR_SYNTAX;
                    return;
                }


                if(TokenLen>1) NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
                RetNum=OK_CONTINUE_NOVALIDATE;
                return;

            }

            // CLOSE THE MATRIX OBJECT
            if(!totalelements) {
                RetNum=ERR_SYNTAX;
                return;
            }

            // STRETCH THE OBJECT, ADD THE INDEX AND REMOVE DUPLICATES
            WORDPTR endofobjects=rplCompileAppendWords(totalelements);
            if(Exceptions) return;

            // MAKE HOLE IN MEMORY
            memmovew(matrix+2+totalelements,matrix+2,endofobjects-(matrix+2));
            endofobjects+=totalelements;

            // NOW WRITE THE INDICES. ALL OFFSETS ARE RELATIVE TO MATRIX PROLOG!
            WORDPTR ptr=matrix+2,objptr=ptr+totalelements,nextobj,index;
            BINT count=0;

            while( (objptr<endofobjects)&&(count<totalelements)) {
                *ptr=objptr-matrix;
                ++ptr;
                ++count;
                objptr=rplSkipOb(objptr);
            }

            if( (count!=totalelements)||(objptr!=endofobjects)) {
                // MALFORMED MATRIX IS MISSING OBJECTS
                RetNum=ERR_INVALID;
                return;
            }

            // COMPACT MATRIX BY REMOVING DUPLICATED OBJECTS
            index=matrix+2;
            objptr=matrix+2+totalelements;

            while(objptr<endofobjects) {
                // CHECK AND REMOVE DUPLICATES OF CURRENT OBJECT
                ptr=rplSkipOb(objptr);
                while(ptr<endofobjects) {
                    if(rplCompareObjects(ptr,objptr)) {
                        // OBJECTS ARE IDENTICAL, REMOVE

                        // REPLACE ALL REFERENCES TO THIS COPY WITH REFERENCES TO THE ORIGINAL
                        for(count=0;count<totalelements;++count) if(index[count]==(WORD)(ptr-matrix)) index[count]=objptr-matrix;

                        // AND REMOVE THE COPY
                        nextobj=rplSkipOb(ptr);
                        if(nextobj<endofobjects) {
                            // THERE'S MORE OBJECTS, MOVE ALL MEMORY AND FIX ALL INDICES
                            memmovew(ptr,nextobj,endofobjects-nextobj);
                            for(count=0;count<totalelements;++count) if(index[count]>(WORD)(ptr-matrix)) index[count]-=nextobj-ptr;
                        }
                        endofobjects-=nextobj-ptr;
                        rplCompileRemoveWords(nextobj-ptr);
                        // DO NOT ADVANCE ptr, SINCE OBJECTS MOVED
                    }
                    else ptr=rplSkipOb(ptr);
                    }
                objptr=rplSkipOb(objptr);
                }


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

        // DecompMode = infix mode in lower 16 bits, decompiler flags in upper 16 bits, including maximum width

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors



        if(ISPROLOG(*DecompileObject)) {
            rplDecompAppendString((BYTEPTR)"[ ");
            BINT rows=MATROWS(*(DecompileObject+1)),cols=MATCOLS(*(DecompileObject+1));
            BINT doublebracket=rows;

            if(!rows) ++rows;

            // SCAN THE INDEX AND OUTPUT ALL OBJECTS INSIDE
            BINT i,j;

            for(i=0;i<rows;++i)
            {
                if(doublebracket) {
                    rplDecompDoHintsWidth(HINT_NLAFTER | ((i==0)? HINT_ADDINDENTAFTER:0));

                    rplDecompAppendString((BYTEPTR)"[ ");

                }
                if(Exceptions) { RetNum=ERR_INVALID; return; }
                for(j=0;j<cols;++j)
                {
                    BINT offset=*(DecompileObject+2+i*cols+j);

                    rplDecompile(DecompileObject+offset,DECOMP_EMBEDDED | ((CurOpcode==OPCODE_DECOMPEDIT)? (DECOMP_EDIT|DECOMP_NOHINTS):DECOMP_NOHINTS));    // RUN EMBEDDED
                 if(Exceptions) { RetNum=ERR_INVALID; return; }

                 if(!rplDecompDoHintsWidth(0)) rplDecompAppendChar(' ');
                }
                if(doublebracket) rplDecompAppendString((BYTEPTR)"] ");
                if(Exceptions) { RetNum=ERR_INVALID; return; }
            }

            if(doublebracket) rplDecompDoHintsWidth(HINT_NLAFTER|HINT_SUBINDENTAFTER);

            rplDecompAppendChar(']');
            if(Exceptions) { RetNum=ERR_INVALID; return; }

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

        // FIRST, CHECK THAT THE OBJECT IS ALLOWED WITHIN AN ARRAY
    {

        if(! (ISNUMBERCPLX(*LastCompiledObject)
              || ISSYMBOLIC(*LastCompiledObject)
              || ISIDENT(*LastCompiledObject)
              || ISANGLE(*LastCompiledObject))) {
                rplError(ERR_NOTALLOWEDINMATRIX);
                RetNum=ERR_INVALID;
                return;
            }

        WORDPTR matrix=*(ValidateTop-1);
        if(LastCompiledObject==matrix+1) {
            // THIS IS THE FIRST OBJECT IN THE ARRAY
            // ADD A DUMMY WORD
            rplCompileAppend(0);
            // MOVE THE FIRST OBJECT UP IN MEMORY TO MAKE ROOM FOR THE SIZE WORD
            memmovew(LastCompiledObject+1,LastCompiledObject,CompileEnd-1-LastCompiledObject);

            matrix[1]=MATMKSIZE(0,1);

        }

        else {
            // IF THIS IS THE FIRST ROW, INCREASE THE COLUMN COUNT
            BINT dimlevel=OBJSIZE(CurrentConstruct);
            BINT rows=MATROWS(matrix[1]),cols=MATCOLS(matrix[1]);
            
            if(rows) {
                // THIS IS A MATRIX, ONLY ALLOW ELEMENTS INSIDE LEVEL 1
                if(!dimlevel) {
                    rplError(ERR_MISPLACEDBRACKETS);
                    RetNum=ERR_INVALID;
                    return;
                }
            } else {
                // THIS IS A VECTOR, ONLY ALLOW ELEMENTS IN THE OUTER LEVEL
                if(dimlevel) {
                    rplError(ERR_MISPLACEDBRACKETS);
                    RetNum=ERR_INVALID;
                    return;
                }

            }
            if(rows<=1) { matrix[1]=MATMKSIZE(rows,cols+1); }

        }



        RetNum=OK_CONTINUE;
        return;
    }

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
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_MATRIX,0,1);
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


        // TODO: CHECK VALIDITY OF A MATRIX, RECURSIVELY VERIFY OBJECTS INSIDE


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
        ObjectPTR=ROMPTR_TABLE[MENUNUMBER(MenuCodeArg)+6];
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
