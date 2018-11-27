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
#define LIBRARY_NUMBER  56

//@TITLE=Operations with Symbolic Expressions

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    ECMD(RULESEPARATOR,":→",MKTOKENINFO(2,TITYPE_BINARYOP_LEFT,2,14)), \
    ECMD(OPENBRACKET,"(",MKTOKENINFO(1,TITYPE_OPENBRACKET,0,31)), \
    ECMD(CLOSEBRACKET,")",MKTOKENINFO(1,TITYPE_CLOSEBRACKET,0,31)), \
    ECMD(COMMA,"",MKTOKENINFO(1,TITYPE_COMMA,0,31)), \
    ECMD(SYMBEVALPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(SYMBEVALPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(SYMBEVALERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(SYMBEVAL1PRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(SYMBEVAL1POST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(SYMBEVAL1ERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(SYMBNUMPRE,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(SYMBNUMPOST,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(SYMBNUMERR,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    CMD(AUTOSIMPLIFY,MKTOKENINFO(12,TITYPE_NOTALLOWED,1,2)), \
    CMD(RULEMATCH,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(RULEAPPLY,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    ECMD(TOFRACTION,"→Q",MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    ECMD(SYMBEVAL1CHK,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(EQUATIONOPERATOR,"=",MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,15)), \
    ECMD(LISTOPENBRACKET,"{",MKTOKENINFO(1,TITYPE_OPENBRACKET,0,31)), \
    ECMD(LISTCLOSEBRACKET,"}",MKTOKENINFO(1,TITYPE_CLOSEBRACKET,0,31))

//    CMD(TEST,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2))

//    ECMD(CMDNAME,"CMDNAME",MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE

#define ERROR_LIST \
    ERR(SYMBOLICEXPECTED,0), \
    ERR(NOTAVALIDRULE,1), \
    ERR(INVALIDUSERDEFINEDFUNCTION,2)

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

ROMOBJECT symbeval_seco[]={
    MKPROLOG(DOCOL,6),
    MKOPCODE(LIBRARY_NUMBER,SYMBEVALPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    (CMD_OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,SYMBEVALPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,SYMBEVALERR),     // ERROR HANDLER
    MKOPCODE(LIBRARY_NUMBER,AUTOSIMPLIFY),   // SIMPLIFY BEFORE RETURN
    CMD_SEMI
};

ROMOBJECT symbeval1_seco[]={
    MKPROLOG(DOCOL,8),
    MKOPCODE(LIBRARY_NUMBER,SYMBEVAL1PRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    (CMD_OVR_EVAL1),    // DO THE EVAL1
    MKOPCODE(LIBRARY_NUMBER,SYMBEVAL1CHK),    // PREPARE TO CHECK IF ANY CHANGE IN THE ARGUMENTS
    (CMD_OVR_SAME),     // CHECK FOR ANY CHANGES IN THE OBJECT
    MKOPCODE(LIBRARY_NUMBER,SYMBEVAL1POST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,SYMBEVAL1ERR),     // ERROR HANDLER
    MKOPCODE(LIBRARY_NUMBER,AUTOSIMPLIFY),   // SIMPLIFY BEFORE RETURN
    CMD_SEMI
};

ROMOBJECT symbnum_seco[]={
    MKPROLOG(DOCOL,7),
    MKOPCODE(LIBRARY_NUMBER,SYMBNUMPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    (CMD_OVR_NUM),    // DO NUM
    MKOPCODE(LIBRARY_NUMBER,SYMBNUMPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,SYMBNUMERR),     // ERROR HANDLER
    (CMD_OVR_EVAL),    // DO EVAL ON OPERATORS
    MKOPCODE(LIBRARY_NUMBER,SYMBNUMPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    CMD_SEMI
};




INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib56_menu);



// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)symbeval_seco,
    (WORDPTR)symbeval1_seco,
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib56_menu,
    0
};


// LOOKS INTO UPPER ENVIRONMENTS THAT MATCH env_owner,
// SEARCHING IN lamnum INDEX FOR object. IF FOUND, MEANS
// THAT A PARENT EVALUATION ALREADY USED THIS OBJECT
// SO IT'S A CIRCULAR REFERENCE


BINT rplCheckCircularReference(WORDPTR env_owner,WORDPTR object,BINT lamnum)
{
    WORDPTR *lamenv=rplGetNextLAMEnv(LAMTop);
    WORDPTR *lamobj;
    BINT nlams;
    while(lamenv) {
        if(*rplGetLAMnEnv(lamenv,0)==env_owner) {
        nlams=rplLAMCount(lamenv);
        if(lamnum>=nlams) {
        lamobj=rplGetLAMnEnv(lamenv,lamnum);
        if(*lamobj==object) return 1;
        }
        }
        lamenv=rplGetNextLAMEnv(lamenv);
    }
    return 0;
}




void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        rplPushData(IPtr);
        return;
    }

    if((OPCODE(CurOpcode)>=MIN_OVERLOAD_OPCODE)&&(OPCODE(CurOpcode)<MIN_RESERVED_OPCODE)) {

        if(ISUNARYOP(CurOpcode) && !ISPROLOG(*rplPeekData(1))) {
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



        // OVERLOADED OPERATORS
    switch(OPCODE(CurOpcode))
    {
        case OVR_NEG:
        CurOpcode=(CMD_OVR_UMINUS);
        case OVR_INV:
        case OVR_NOT:
        case OVR_UMINUS:
        case OVR_UPLUS:
    {
     // UNARY OPERATION ON A SYMBOLIC



     WORDPTR object=rplPeekData(1);



         if(rplSymbMainOperator(object)==CurOpcode) {
             // THIS SYMBOLIC ALREADY HAS THE OPERATOR, REMOVE IT!
             WORDPTR arg=rplSymbUnwrap(object)+2;

             if(!ISSYMBOLIC(*arg)) arg=rplSymbWrap(arg);    // WRAP IT AS SYMBOLIC
             rplOverwriteData(1,arg);
             return;
         }

     object=rplSymbUnwrap(object);
     if(!object) {
         rplError(ERR_SYMBOLICEXPECTED);

         return;
     }
     if(ISSYMBOLIC(*object)) {
         if(ISPROLOG(object[1]) || ISBINT(object[1])) ++object;   // POINT TO THE SINGLE OBJECT WITHIN THE SYMBOLIC WRAPPER
     }

     BINT size=rplObjSize(object);
     // NEED TO WRAP AND ADD THE OPERATOR
     size+=2;

     ScratchPointer1=object;
    WORDPTR newobject=rplAllocTempOb(size-1);
    if(!newobject) return;

    newobject[0]=MKPROLOG(DOSYMB,size-1);
    newobject[1]=MKOPCODE(LIB_OVERLOADABLE,OPCODE(CurOpcode));
    object=ScratchPointer1; // RESTORE AS IT MIGHT'VE MOVED DURING GC

    WORDPTR endptr=rplSkipOb(object);
    WORDPTR ptr=newobject+2;
    while(object!=endptr) *ptr++=*object++;

    // REMOVE QUOTES ON ANY IDENTS
    ptr=newobject+2;
    if(ISIDENT(*ptr)) *ptr=SETLIBNUMBIT(*ptr,UNQUOTED_BIT);

    rplOverwriteData(1,newobject);
    return;

    }
        case OVR_EVAL1:
        // EVAL NEEDS TO SCAN THE SYMBOLIC, EVAL EACH ARGUMENT SEPARATELY AND APPLY THE OPCODE.
    {
        WORDPTR object=rplPeekData(1);
        if(!ISSYMBOLIC(*object)) {
            rplError(ERR_SYMBOLICEXPECTED);

            return;
        }

        // HERE WE HAVE program = PROGRAM TO EXECUTE

        // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
        rplCreateLAMEnvironment((WORDPTR)symbeval1_seco+2);

        object=rplSymbUnwrap(object);
        WORDPTR endobject=rplSkipOb(object);
        WORD Opcode=rplSymbMainOperator(object);
        WORDPTR Opcodeptr=rplSymbMainOperatorPTR(object);
        if(!Opcodeptr) Opcodeptr=(WORDPTR)zero_bint;

        rplCreateLAM((WORDPTR)nulllam_ident,Opcodeptr);     // LAM 1 = OPCODE
        if(Exceptions) { rplCleanupLAMs(0); return; }

        object++;
        if(Opcode) {
            object++;
            if(OPCODE(Opcode)==OVR_FUNCEVAL) {
                // DON'T MARK THE LAST OBJECT AS THE END OF OBJECT
                WORDPTR lastobj=object;
                while(rplSkipOb(lastobj)!=endobject) lastobj=rplSkipOb(lastobj);
                endobject=lastobj;
            }

        }

        rplCreateLAM((WORDPTR)nulllam_ident,endobject);     // LAM 2 = END OF CURRENT OBJECT
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,object);     // LAM 3 = NEXT OBJECT TO PROCESS
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);     // LAM 4 = ANY ARGUMENT CHANGED?
        if(Exceptions) { rplCleanupLAMs(0); return; }
        // HERE GETLAM1 = OPCODE, GETLAM 2 = END OF SYMBOLIC, GETLAM3 = OBJECT, GETLAM4=FLAG TO TEST IF ANY ARGUMENT CHANGED

        // THIS NEEDS TO BE DONE IN 3 STEPS:
        // EVAL WILL PREPARE THE LAMS FOR OPEN EXECUTION
        // SYMBEVAL1PRE WILL PUSH THE NEXT OBJECT IN THE STACK AND EVAL IT
        // SYMBEVAL1POST WILL CHECK IF THE ARGUMENT WAS PROCESSED WITHOUT ERRORS,
        // AND CLOSE THE LOOP TO PROCESS MORE ARGUMENTS

        // THE INITIAL CODE FOR EVAL MUST TRANSFER FLOW CONTROL TO A
        // SECONDARY THAT CONTAINS :: SYMBEVAL1PRE EVAL SYMBEVAL1POST ;
        // SYMBEVAL1POST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
        // IN ORDER TO KEEP THE LOOP RUNNING

        rplPushRet(IPtr);
        if((rplPeekRet(1)<symbeval1_seco)||(rplPeekRet(1)>symbeval1_seco+6))
        {
            // THIS EVAL IS NOT INSIDE A RECURSIVE LOOP
            // PUSH AUTOSIMPLIFY TO BE EXECUTED AFTER EVAL
            rplPushRet((WORDPTR)symbeval1_seco+6);
        }
        IPtr=(WORDPTR) symbeval1_seco;
        CurOpcode=(CMD_OVR_EVAL1);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

        rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD EVALUATION

        return;
    }

    case OVR_FUNCEVAL:

    {
        // A SYMBOLIC OBJECT CANNOT BE FUNCEVALED
        rplError(ERR_INVALIDUSERDEFINEDFUNCTION);
        return;

    }

    case OVR_EVAL:
    // EVAL NEEDS TO SCAN THE SYMBOLIC, EVAL EACH ARGUMENT SEPARATELY AND APPLY THE OPCODE.
{
    WORDPTR object=rplPeekData(1),mainobj;
    if(!ISSYMBOLIC(*object)) {
        rplError(ERR_SYMBOLICEXPECTED);

        return;
    }

    if(rplCheckCircularReference((WORDPTR)symbeval_seco+2,object,4)) {
        rplError(ERR_CIRCULARREFERENCE);

        return;
    }

    mainobj=object;

    // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
    rplCreateLAMEnvironment((WORDPTR)symbeval_seco+2);

    object=rplSymbUnwrap(object);
    WORDPTR endobject=rplSkipOb(object);
    WORD Opcode=rplSymbMainOperator(object);
    WORDPTR Opcodeptr=rplSymbMainOperatorPTR(object);
    if(!Opcodeptr) Opcodeptr=(WORDPTR)zero_bint;

    rplCreateLAM((WORDPTR)nulllam_ident,Opcodeptr);     // LAM 1 = OPCODE
    if(Exceptions) { rplCleanupLAMs(0); return; }

    object++;
    if(Opcode) {
        object++;
        if(OPCODE(Opcode)==OVR_FUNCEVAL) {
            // DON'T MARK THE LAST OBJECT AS THE END OF OBJECT
            WORDPTR lastobj=object;
            while(rplSkipOb(lastobj)!=endobject) lastobj=rplSkipOb(lastobj);
            endobject=lastobj;
        }

    }

    rplCreateLAM((WORDPTR)nulllam_ident,endobject);     // LAM 2 = END OF CURRENT OBJECT
    if(Exceptions) { rplCleanupLAMs(0); return; }

    rplCreateLAM((WORDPTR)nulllam_ident,object);     // LAM 3 = NEXT OBJECT TO PROCESS
    if(Exceptions) { rplCleanupLAMs(0); return; }

    rplCreateLAM((WORDPTR)nulllam_ident,mainobj);     // LAM 4 = MAIN SYMBOLIC EXPRESSION, FOR CIRCULAR REFERENCE CHECK
    if(Exceptions) { rplCleanupLAMs(0); return; }

    // HERE GETLAM1 = OPCODE, GETLAM 2 = END OF SYMBOLIC, GETLAM3 = OBJECT

    // THIS NEEDS TO BE DONE IN 3 STEPS:
    // EVAL WILL PREPARE THE LAMS FOR OPEN EXECUTION
    // SYMBEVAL1PRE WILL PUSH THE NEXT OBJECT IN THE STACK AND EVAL IT
    // SYMBEVAL1POST WILL CHECK IF THE ARGUMENT WAS PROCESSED WITHOUT ERRORS,
    // AND CLOSE THE LOOP TO PROCESS MORE ARGUMENTS

    // THE INITIAL CODE FOR EVAL MUST TRANSFER FLOW CONTROL TO A
    // SECONDARY THAT CONTAINS :: SYMBEVALPRE EVAL SYMBEVALPOST ;
    // SYMBEVAL1POST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
    // IN ORDER TO KEEP THE LOOP RUNNING

    rplPushRet(IPtr);
    if((rplPeekRet(1)<symbeval_seco)||(rplPeekRet(1)>symbeval_seco+4))
    {
        // THIS EVAL IS NOT INSIDE A RECURSIVE LOOP
        // PUSH AUTOSIMPLIFY TO BE EXECUTED AFTER EVAL
        rplPushRet((WORDPTR)symbeval_seco+4);
    }
    IPtr=(WORDPTR) symbeval_seco;
    CurOpcode=(CMD_OVR_EVAL);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

    rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD EVALUATION

    return;
}

    case OVR_ISTRUE:
    {
     // UNARY OPERATION ON A SYMBOLIC

     WORDPTR object=rplPeekData(1);

         if(rplSymbMainOperator(object)==CurOpcode) {
             // THIS SYMBOLIC ALREADY HAS THE OPERATOR, NO NEED TO ADD IT
             return;
         }

     object=rplSymbUnwrap(object);
     if(!object) {
         rplError(ERR_SYMBOLICEXPECTED);

         return;
     }
     BINT size=rplObjSize(object);
     // NEED TO WRAP AND ADD THE OPERATOR
     size+=2;

    WORDPTR newobject=rplAllocTempOb(size-1);
    if(!newobject) return;

    newobject[0]=MKPROLOG(DOSYMB,size-1);
    newobject[1]=MKOPCODE(LIB_OVERLOADABLE,OPCODE(CurOpcode));
    object=rplSymbUnwrap(rplPeekData(1));  // READ AGAIN, GC MIGHT'VE MOVED THE OBJECT

    WORDPTR endptr=rplSkipOb(object);
    WORDPTR ptr=newobject+2;
    while(object!=endptr) *ptr++=*object++;

    rplOverwriteData(1,newobject);
    //return; //DELIBERATE FALL-THROUGH ->NUM - DO NOT REORDER!!

    }

    case OVR_NUM:
    // NUM NEEDS TO SCAN THE SYMBOLIC, EVAL EACH ARGUMENT SEPARATELY AND APPLY THE OPCODE.
{
    WORDPTR object=rplPeekData(1),mainobj;
    if(!ISSYMBOLIC(*object)) {
        rplError(ERR_SYMBOLICEXPECTED);

        return;
    }

    if(rplCheckCircularReference((WORDPTR)symbnum_seco+2,object,4)) {
        rplError(ERR_CIRCULARREFERENCE);

        return;
    }

    mainobj=object;

    // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
    rplCreateLAMEnvironment((WORDPTR)symbnum_seco+2);

    object=rplSymbUnwrap(object);
    WORDPTR endobject=rplSkipOb(object);
    WORD Opcode=rplSymbMainOperator(object);
    WORDPTR Opcodeptr=rplSymbMainOperatorPTR(object);
    if(!Opcodeptr) Opcodeptr=(WORDPTR)zero_bint;

    rplCreateLAM((WORDPTR)nulllam_ident,Opcodeptr);     // LAM 1 = OPCODE
    if(Exceptions) { rplCleanupLAMs(0); return; }

    object++;
    if(Opcode) {
        object++;
        if(OPCODE(Opcode)==OVR_FUNCEVAL) {
            // DON'T MARK THE LAST OBJECT AS THE END OF OBJECT
            WORDPTR lastobj=object;
            while(rplSkipOb(lastobj)!=endobject) lastobj=rplSkipOb(lastobj);
            endobject=lastobj;
        }

    }

    rplCreateLAM((WORDPTR)nulllam_ident,endobject);     // LAM 2 = END OF CURRENT OBJECT
    if(Exceptions) { rplCleanupLAMs(0); return; }

    rplCreateLAM((WORDPTR)nulllam_ident,object);     // LAM 3 = NEXT OBJECT TO PROCESS
    if(Exceptions) { rplCleanupLAMs(0); return; }

    rplCreateLAM((WORDPTR)nulllam_ident,mainobj);     // LAM 4 = MAIN SYMBOLIC EXPRESSION, FOR CIRCULAR REFERENCE CHECK
    if(Exceptions) { rplCleanupLAMs(0); return; }

    // HERE GETLAM1 = OPCODE, GETLAM 2 = END OF SYMBOLIC, GETLAM3 = OBJECT

    // THIS NEEDS TO BE DONE IN 3 STEPS:
    // EVAL WILL PREPARE THE LAMS FOR OPEN EXECUTION
    // SYMBEVAL1PRE WILL PUSH THE NEXT OBJECT IN THE STACK AND EVAL IT
    // SYMBEVAL1POST WILL CHECK IF THE ARGUMENT WAS PROCESSED WITHOUT ERRORS,
    // AND CLOSE THE LOOP TO PROCESS MORE ARGUMENTS

    // THE INITIAL CODE FOR EVAL MUST TRANSFER FLOW CONTROL TO A
    // SECONDARY THAT CONTAINS :: SYMBEVALPRE EVAL SYMBEVALPOST ;
    // SYMBEVAL1POST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
    // IN ORDER TO KEEP THE LOOP RUNNING

    rplPushRet(IPtr);
    if((rplPeekRet(1)<symbnum_seco)||(rplPeekRet(1)>symbnum_seco+4))
    {
        // THIS ->NUM IS NOT INSIDE A RECURSIVE LOOP
        // PUSH AUTOSIMPLIFY TO BE EXECUTED AFTER ->NUM
        rplPushRet((WORDPTR)symbnum_seco+6);
    }
    IPtr=(WORDPTR) symbnum_seco;
    CurOpcode=(CMD_OVR_NUM);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

    rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD EVALUATION

    return;
}



        case OVR_XEQ:
        // JUST LEAVE IT ON THE STACK
        return;
        case OVR_ABS:

    {
     // UNARY OPERATION ON A SYMBOLIC

     WORDPTR object=rplPeekData(1);

         if(rplSymbMainOperator(object)==CurOpcode) {
             // THIS SYMBOLIC ALREADY HAS THE OPERATOR, NO NEED TO ADD IT
             return;
         }

     object=rplSymbUnwrap(object);
     if(!object) {
         rplError(ERR_SYMBOLICEXPECTED);

         return;
     }
     BINT size=rplObjSize(object);
     // NEED TO WRAP AND ADD THE OPERATOR
     size+=2;

    WORDPTR newobject=rplAllocTempOb(size-1);
    if(!newobject) return;

    newobject[0]=MKPROLOG(DOSYMB,size-1);
    newobject[1]=MKOPCODE(LIB_OVERLOADABLE,OPCODE(CurOpcode));
    object=rplSymbUnwrap(rplPeekData(1));  // READ AGAIN, GC MIGHT'VE MOVED THE OBJECT

    WORDPTR endptr=rplSkipOb(object);
    WORDPTR ptr=newobject+2;
    while(object!=endptr) *ptr++=*object++;

    rplOverwriteData(1,newobject);
    return;

    }

    case OVR_ADD:
        // ADDITION IS A SPECIAL CASE, NEEDS TO KEEP ARGUMENTS FLAT
    {
        WORDPTR arg1=rplPeekData(2);
        WORDPTR arg2=rplPeekData(1);


        // ALLOW LIST PROCESSING AND MATRIX PROCESSING FIRST
        if(ISLIST(*arg1) || ISLIST(*arg2)){
            rplListBinaryDoCmd();
            return;
        }

        if(ISMATRIX(*arg1)||ISMATRIX(*arg2)) {
           rplMatrixAdd();
           return;
        }


        // FIRST, CHECK THAT ARGUMENTS ARE ACCEPTABLE FOR SYMBOLIC OPERATION
        if( (!rplIsAllowedInSymb(rplPeekData(2))) || (!rplIsAllowedInSymb(rplPeekData(1))))
        {
            rplError(ERR_NOTALLOWEDINSYMBOLICS);

            return;
        }

        BINT initdepth=rplDepthData();
        BINT argtype=0;

        if(ISSYMBOLIC(*arg1) && rplSymbMainOperator(arg1)==(CMD_OVR_ADD)) {
            // EXPLODE ALL ARGUMENTS ON THE STACK
            ScratchPointer1=rplSymbUnwrap(arg1);
            ScratchPointer2=rplSkipOb(ScratchPointer1);
            while(ScratchPointer1!=ScratchPointer2) {
            if(ISSYMBOLIC(*ScratchPointer1) && rplSymbMainOperator(ScratchPointer1)==(CMD_OVR_ADD)) {
                ScratchPointer1=rplSymbUnwrap(ScratchPointer1);
                ScratchPointer1++;  // POINT TO THE OPCODE, SO THE NEXT OBJECT WILL BE THE FIRST ARGUMENT
            }
            else {
                // ALSO CHECK FOR NEGATIVE NUMBERS, CONVERT TO POSITIVE WITH UMINUS OPERATOR
                rplPushData(ScratchPointer1);
            }
            ScratchPointer1=rplSkipOb(ScratchPointer1);
            }
            argtype|=1;
        } else { if(ISSYMBOLIC(*arg1)||ISIDENT(*arg1)) argtype|=4; rplPushData(arg1); }

        arg2=rplPeekData(rplDepthData()-initdepth+1);

        // EXPLODE THE SECOND ARGUMENT EXACTLY THE SAME
        if(ISSYMBOLIC(*arg2) && rplSymbMainOperator(arg2)==(CMD_OVR_ADD)) {
            // EXPLODE ALL ARGUMENTS ON THE STACK
            ScratchPointer1=rplSymbUnwrap(arg2);
            ScratchPointer2=rplSkipOb(ScratchPointer1);
            while(ScratchPointer1!=ScratchPointer2) {
            if(ISSYMBOLIC(*ScratchPointer1) && rplSymbMainOperator(ScratchPointer1)==(CMD_OVR_ADD)) {
                ScratchPointer1=rplSymbUnwrap(ScratchPointer1);
                ScratchPointer1++;  // POINT TO THE OPCODE, SO THE NEXT OBJECT WILL BE THE FIRST ARGUMENT
            }
            else {
                // ALSO CHECK FOR NEGATIVE NUMBERS, CONVERT TO POSITIVE WITH UMINUS OPERATOR
                rplPushData(ScratchPointer1);
            }
            ScratchPointer1=rplSkipOb(ScratchPointer1);
            }
            argtype|=2;
        } else { if(ISSYMBOLIC(*arg2)||ISIDENT(*arg2)) argtype|=8; rplPushData(arg2); }

/*
        if( (argtype==0) || (argtype&3)) {
            // ONE OR MORE ARGUMENTS WERE EXPLODED IN THE STACK
            // OR BOTH ARGUMENTS ARE NON-SYMBOLIC

        // SORT ARGUMENTS BY LIBRARY NUMBER
            WORDPTR *ptr,*ptr2,*endlimit,*startlimit,save;
            WORDPTR *left,*right;

            startlimit=DSTop-(rplDepthData()-initdepth)+1;    // POINT TO SECOND ELEMENT IN THE LIST
            endlimit=DSTop;           // POINT AFTER THE LAST ELEMENT

            for(ptr=startlimit;ptr<endlimit;++ptr)
            {
                save=*ptr;

                left=startlimit-1;
                right=ptr-1;
                if(SYMBITEMCOMPARE(*right,save)>0) {
                   if(SYMBITEMCOMPARE(save,*left)>0) {
                while(right-left>1) {
                    if(SYMBITEMCOMPARE(*(left+(right-left)/2),save)>0) {
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






        // TODO: REPLACE THIS WITH A NON-RECURSIVE SOLUTION
        while(rplDepthData()-initdepth>1) {
            rplCallOvrOperator(CurOpcode);
            if(ISSYMBOLIC(*rplPeekData(1))) {
                if(rplDepthData()-initdepth>1) rplSymbApplyOperator(CurOpcode,rplDepthData()-initdepth);
                rplOverwriteData(3,rplPeekData(1));
                rplDropData(2);
                return;

            }
        }

        rplOverwriteData(3,rplPeekData(1));
        rplDropData(2);
        return;
        }
*/
        // OTHERWISE DO THE SYMBOLIC OPERATION

        //rplSymbApplyOperator(CurOpcode,2);
        rplSymbApplyOperator(CurOpcode,rplDepthData()-initdepth);

        rplOverwriteData(3,rplPeekData(1));
        rplDropData(2);
        return;

    }

        break;

        case OVR_SAME:
        {
        // IMMEDIATELY RETURN TRUE/FALSE BY COMPARISON OF OBJECTS

        WORDPTR arg1=rplPeekData(2);
        WORDPTR arg2=rplPeekData(1);


        // ALLOW LIST PROCESSING AND MATRIX PROCESSING FIRST
        if(ISLIST(*arg1) || ISLIST(*arg2)) {
            rplListBinaryDoCmd();
            return;
        }

        if(ISMATRIX(*arg1)||ISMATRIX(*arg2)) {
           // PASS IT DIRECTLY TO HANDLER OF MATRIX OBJECT
           LIBHANDLER han=rplGetLibHandler(DOMATRIX);
           if(han) (*han)();
           else rplError(ERR_MISSINGLIBRARY);
           return;
        }

        ScratchPointer1=arg1;
        ScratchPointer2=arg2;
        ScratchPointer3=rplSkipOb(arg1);
        ScratchPointer4=rplSkipOb(arg2);

        WORDPTR *stksave=DSTop;
        BINT same=1;
        while(same && (ScratchPointer1<ScratchPointer3) && (ScratchPointer2<ScratchPointer4)) {
            while(ISSYMBOLIC(*ScratchPointer1)) ++ScratchPointer1;
            while(ISSYMBOLIC(*ScratchPointer2)) ++ScratchPointer2;

            if(ISPROLOG(*ScratchPointer1) || ISBINT(*ScratchPointer1)) {
                // COMPARE OBJECTS BY USING THE SAME OPERATOR
            rplPushDataNoGrow(ScratchPointer1);
            rplPushDataNoGrow(ScratchPointer2);

            rplCallOvrOperator(CMD_OVR_SAME);

            if(Exceptions) {
                DSTop=stksave;
                return;
            }

            if(rplIsFalse(rplPopData())) same=0;

            } else {
                // COMPARE COMMANDS BY USING DIRECT COMPARISON
                if(*ScratchPointer1!=*ScratchPointer2) same=0;
            }

            ScratchPointer1=rplSkipOb(ScratchPointer1);
            ScratchPointer2=rplSkipOb(ScratchPointer2);
        }

        rplDropData(2);
        if(same && (ScratchPointer1==ScratchPointer3) && (ScratchPointer2==ScratchPointer4)) rplPushTrue();
        else rplPushFalse();

        return;
        }

        case OVR_EQ:
        case OVR_NOTEQ:
        case OVR_LT:
        case OVR_GT:
        case OVR_LTE:
        case OVR_GTE:
        case OVR_AND:
        case OVR_OR:
        case OVR_XOR:
        case OVR_CMP:
        case OVR_SUB:
        case OVR_MUL:
        case OVR_DIV:
        case OVR_POW:
        case OVR_XROOT:
        {
        // BINARY OPERATORS



        // FIRST, CHECK THAT ARGUMENTS ARE ACCEPTABLE FOR SYMBOLIC OPERATION
        WORDPTR arg1=rplPeekData(2);
        WORDPTR arg2=rplPeekData(1);

        // ALLOW LIST PROCESSING AND MATRIX PROCESSING FIRST
        if(ISLIST(*arg1) || ISLIST(*arg2)){
            rplListBinaryDoCmd();
            return;
        }

        if(ISMATRIX(*arg1)||ISMATRIX(*arg2)) {
           // PASS IT DIRECTLY TO HANDLER OF MATRIX OBJECT
           LIBHANDLER han=rplGetLibHandler(DOMATRIX);
           if(han) (*han)();
           else rplError(ERR_MISSINGLIBRARY);
           return;
        }




        if( (!rplIsAllowedInSymb(arg1)) || (!rplIsAllowedInSymb(arg2)))
        {
            rplError(ERR_NOTALLOWEDINSYMBOLICS);

            return;
        }

        rplSymbApplyOperator(CurOpcode,2);
        return;
        }
        break;
        }


    }

    switch(OPCODE(CurOpcode))
    {


    case SYMBEVAL1PRE:
    {
        // HERE GETLAM1 = OPCODE, GETLAM 2 = END OF SYMBOLIC, GETLAM3 = OBJECT, GETLAM4=FLAG TO SEE IF ANY OBJECT CHANGED


        WORDPTR nextobj=*rplGetLAMn(3);
        WORDPTR endoflist=*rplGetLAMn(2);

        if(nextobj==endoflist) {
            // THE LAST ARGUMENT WAS ALREADY PROCESSED, IF THERE IS AN OPERATOR WE NEED TO APPLY IT

            WORDPTR Opcodeptr=*rplGetLAMn(1),anyargschanged=*rplGetLAMn(4);
            WORD Opcode=(Opcodeptr==zero_bint)? 0:*Opcodeptr;

            WORDPTR *prevDStk;
            if(Opcodeptr==zero_bint) prevDStk = rplUnprotectData();
            else prevDStk=DStkProtect;
            BINT newdepth=(BINT)(DSTop-prevDStk);

            if(Opcode) {
                if(Opcode==CMD_OVR_FUNCEVAL) {
                    // SPECIAL CASE, ALL ARGUMENTS WERE EVALUATED BUT ACTUAL FUNCTION NEEDS TO BE CALLED HERE
                    rplPushData(endoflist);
                    ++newdepth;
                    // DO MINIMAL TYPE CHECKING, LAST ARGUMENT HAS TO BE
                    // AN IDENT, OTHERWISE THE RESULT IS INVALID
                    if(!ISIDENT(*rplPeekData(1)) && !ISLIBPTR(*rplPeekData(1))) {
                        // IT SHOULD ACTUALLY RETURN SOMETHING LIKE "INVALID USER FUNCTION"
                        DSTop=rplUnprotectData();
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        rplError(ERR_INVALIDUSERDEFINEDFUNCTION);
                        CurOpcode=(CMD_OVR_EVAL);
                        return;
                    }

                    rplSetExceptionHandler(IPtr+5); // SET THE EXCEPTION HANDLER TO THE SYMBEVAL1ERR WORD

                    // PUSH THE NEXT OBJECT IN THE STACK
                    rplPushData(Opcodeptr);

                    rplPutLAMn(1,(WORDPTR)zero_bint);  // SIGNAL OPCODE IS DONE
                    // AND EXECUTION WILL CONTINUE AT EVAL

                    return;


                }

                    // DO SYMBOLIC WRAP ON ALL OBJECTS THAT ARE NOT MATRICES OR LISTS
                    rplSymbWrapN(1,newdepth);

                if(anyargschanged!=zero_bint) {
                    // SOME ARGUMENTS CHANGED IN THIS STEP, DON'T EVALUATE THE FUNCTION
                    rplSymbApplyOperator(Opcode,newdepth);
                    newdepth=(BINT)(DSTop-prevDStk);
                }

                else {
                // PUSH THE OPERATOR IN THE STACK AND EVAL IT. THIS SHOULD APPLY THE OPERATOR IF THE RESULT IS SYMBOLIC
                    // OTHERWISE IT WILL CALCULATE IT

                    rplSetExceptionHandler(IPtr+5); // SET THE EXCEPTION HANDLER TO THE SYMBEVAL1ERR WORD

                    if((Opcode==CMD_OPENBRACKET) || (Opcode==CMD_LISTOPENBRACKET)) {
                        // SPECIAL CASE, THESE COMMANDS NEED THE NUMBER OF ARGUMENTS PUSHED ON THE STACK
                        rplNewBINTPush(newdepth,DECBINT);


                    }

                    if((Opcode==CMD_OVR_MUL)||(Opcode==CMD_OVR_ADD)) {
                        // CHECK FOR FLATTENED LIST, APPLY MORE THAN ONCE IF MORE THAN 2 ARGUMENTS
                        if(newdepth<=2) rplPutLAMn(1,(WORDPTR)zero_bint);  // SIGNAL OPCODE IS DONE
                    }
                    else  rplPutLAMn(1,(WORDPTR)zero_bint);  // SIGNAL OPCODE IS DONE


                    // PUSH THE NEXT OBJECT IN THE STACK
                    rplPushData(Opcodeptr);

                    // AND EXECUTION WILL CONTINUE AT EVAL

                    return;
                }

            }




            if(newdepth!=1) {
                rplCleanupLAMs(0);
                IPtr=rplPopRet();
                rplError(ERR_BADARGCOUNT);
                CurOpcode=(CMD_OVR_EVAL1);
                return;
            }
            // HERE WE ARE SUPPOSED TO HAVE ONLY ONE ARGUMENT ON THE STACK AND THE ORIGINAL OBJECT
            rplOverwriteData(2,rplPeekData(1));
            rplDropData(1);
            // CLEANUP AND RETURN
            rplCleanupLAMs(0);
            IPtr=rplPopRet();
            CurOpcode=(CMD_OVR_EVAL1);
            return;

        }

        rplSetExceptionHandler(IPtr+5); // SET THE EXCEPTION HANDLER TO THE SYMBEVAL1ERR WORD


        // PUSH THE NEXT OBJECT IN THE STACK
        rplPushData(nextobj);
        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case SYMBEVAL1CHK:
    {
        // INTERMEDIATE OPCODE:
        // HERE THE STACK HAS AN ELEMENT THAT WAS JUST EVAL1(X)
        // IF IT WASN'T THE OPCODE, KEEP GOING TO THE SAME OPERATOR
        // ELSE SKIP TO THE POST-PROCESSING

        WORDPTR nextobj=*rplGetLAMn(3);
        WORDPTR endoflist=*rplGetLAMn(2);

        if(nextobj<endoflist) {
            rplPushData(nextobj);           // GET THE ORIGINAL OBJECT
            rplPushData(rplPeekData(2));    // AND THE ONE WE JUST EVAL'd

            // AND CONTINUE TO 'SAME'
            return;
        }

        // WE ALREADY PROCESSED THE ENTIRE SYMBOLIC
        // JUST PUT A 1 ON THE STACK, AND SKIP SAME
        rplPushData((WORDPTR)one_bint);
        IPtr++; // SKIP NEXT COMMAND

        return;
    }
    case SYMBEVAL1POST:
    {
        // HERE GETLAM1 = OPCODE, GETLAM 2 = END OF SYMBOLIC, GETLAM3 = OBJECT

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR DURING EVALUATION

        // STACK IS SUPPOSED TO HAVE EVAL1(OBJECT) THEN THE RESULT OF SAME(OBJECT,EVAL1(OBJECT))
        if(rplIsFalse(rplPopData())) rplPutLAMn(4,(WORDPTR)one_bint);

        WORDPTR nextobj=*rplGetLAMn(3);
        WORDPTR endoflist=*rplGetLAMn(2);

        if(nextobj<endoflist) rplPutLAMn(3,rplSkipOb(nextobj));    // MOVE TO THE NEXT OBJECT IN THE LIST


        IPtr=(WORDPTR) symbeval1_seco;   // CONTINUE THE LOOP
        // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
        return;
    }
    case SYMBEVAL1ERR:
        // BLAME THE ERROR ON THE LAST OBJECT EVALUATED UNLESS A MORE SPECIFIC SUSPECT WAS ALREADY BLAMED
        if((ExceptionPointer<*rplGetLAMn(4))||(ExceptionPointer>=rplSkipOb(*rplGetLAMn(4))))
                rplBlameError(*rplGetLAMn(4));
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

        CurOpcode=(CMD_OVR_EVAL1);
        return;

    case AUTOSIMPLIFY:
    {
        //@SHORT_DESC=Reduce numeric subexpressions
        //@NEW
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            rplBlameUserCommand();
            return;
        }

        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryDoCmd();
            return;
        }
        if(!ISSYMBOLIC(*rplPeekData(1))) return;    // LEAVE IT ON THE STACK, NOT A SYMBOLIC

        rplSymbAutoSimplify();

        return;

    }


    case SYMBEVALPRE:
    {
        // HERE GETLAM1 = OPCODE, GETLAM 2 = END OF SYMBOLIC, GETLAM3 = OBJECT


        WORDPTR nextobj=*rplGetLAMn(3);
        WORDPTR endoflist=*rplGetLAMn(2);

        if(nextobj>=endoflist) {
            // THE LAST ARGUMENT WAS ALREADY PROCESSED, IF THERE IS AN OPERATOR WE NEED TO APPLY IT

            WORDPTR Opcodeptr=*rplGetLAMn(1);
            WORD Opcode=(Opcodeptr==zero_bint)? 0:*Opcodeptr;


            WORDPTR *prevDStk;
            if(Opcodeptr==zero_bint) prevDStk = rplUnprotectData();
            else prevDStk=DStkProtect;
            BINT newdepth=(BINT)(DSTop-prevDStk);


            if(Opcode) {
                if(Opcode==CMD_OVR_FUNCEVAL) {
                    // SPECIAL CASE, ALL ARGUMENTS WERE EVALUATED BUT ACTUAL FUNCTION NEEDS TO BE CALLED HERE
                    rplPushData(endoflist);
                    ++newdepth;
                    // DO MINIMAL TYPE CHECKING, LAST ARGUMENT HAS TO BE
                    // AN IDENT, OTHERWISE THE RESULT IS INVALID
                    if(!ISIDENT(*rplPeekData(1)) && !ISLIBPTR(*rplPeekData(1))) {
                        // IT SHOULD ACTUALLY RETURN SOMETHING LIKE "INVALID USER FUNCTION"
                        DSTop=rplUnprotectData();
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        rplError(ERR_INVALIDUSERDEFINEDFUNCTION);
                        CurOpcode=(CMD_OVR_EVAL);
                        return;
                    }

                    rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE SYMBEVAL1ERR WORD

                    // PUSH THE NEXT OBJECT IN THE STACK
                    rplPushData(Opcodeptr);

                    rplPutLAMn(1,(WORDPTR)zero_bint);  // SIGNAL OPCODE IS DONE
                    // AND EXECUTION WILL CONTINUE AT EVAL

                    return;


                }

                    // DO SYMBOLIC WRAP ON ALL OBJECTS THAT ARE NOT MATRICES OR LISTS
                    rplSymbWrapN(1,newdepth);

                    // PUSH THE OPERATOR IN THE STACK AND EVAL IT. THIS SHOULD APPLY THE OPERATOR IF THE RESULT IS SYMBOLIC
                    // OTHERWISE IT WILL CALCULATE IT

                    rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE SYMBEVAL1ERR WORD

                    if((Opcode==CMD_OPENBRACKET) || (Opcode==CMD_LISTOPENBRACKET)) {
                        // SPECIAL CASE, THESE COMMANDS NEED THE NUMBER OF ARGUMENTS PUSHED ON THE STACK
                        rplNewBINTPush(newdepth,DECBINT);
                    }

                    if((Opcode==CMD_OVR_MUL)||(Opcode==CMD_OVR_ADD)) {
                        // CHECK FOR FLATTENED LIST, APPLY MORE THAN ONCE IF MORE THAN 2 ARGUMENTS
                        if(newdepth<=2) rplPutLAMn(1,(WORDPTR)zero_bint);  // SIGNAL OPCODE IS DONE
                    }
                    else  rplPutLAMn(1,(WORDPTR)zero_bint);  // SIGNAL OPCODE IS DONE

                    // PUSH THE NEXT OBJECT IN THE STACK
                    rplPushData(Opcodeptr);

                    // AND EXECUTION WILL CONTINUE AT EVAL

                    return;


            }
            if(newdepth!=1) {
                rplCleanupLAMs(0);
                IPtr=rplPopRet();
                rplError(ERR_BADARGCOUNT);
                CurOpcode=(CMD_OVR_EVAL);
                return;
            }
            // HERE WE ARE SUPPOSED TO HAVE ONLY ONE ARGUMENT ON THE STACK AND THE ORIGINAL OBJECT
            rplOverwriteData(2,rplPeekData(1));
            rplDropData(1);
            // CLEANUP AND RETURN
            rplCleanupLAMs(0);
            IPtr=rplPopRet();
            CurOpcode=(CMD_OVR_EVAL);
            return;

        }

        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE SYMBEVAL1ERR WORD


        // PUSH THE NEXT OBJECT IN THE STACK
        rplPushData(nextobj);

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case SYMBEVALPOST:
    {
        // HERE GETLAM1 = OPCODE, GETLAM 2 = END OF SYMBOLIC, GETLAM3 = OBJECT

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR DURING EVALUATION

        WORDPTR nextobj=*rplGetLAMn(3);
        WORDPTR endoflist=*rplGetLAMn(2);

        if(nextobj<endoflist) rplPutLAMn(3,rplSkipOb(nextobj));    // MOVE TO THE NEXT OBJECT IN THE LIST


        IPtr=(WORDPTR) symbeval_seco;   // CONTINUE THE LOOP
        // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
        return;
    }
    case SYMBEVALERR:
        // BLAME THE ERROR ON THE LAST OBJECT EVALUATED UNLESS A MORE SPECIFIC SUSPECT WAS ALREADY BLAMED
        if((ExceptionPointer<*rplGetLAMn(4))||(ExceptionPointer>=rplSkipOb(*rplGetLAMn(4))))
                rplBlameError(*rplGetLAMn(4));

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

        CurOpcode=(CMD_OVR_EVAL);
        return;


    case SYMBNUMPRE:
    {
        // HERE GETLAM1 = OPCODE, GETLAM 2 = END OF SYMBOLIC, GETLAM3 = OBJECT


        WORDPTR nextobj=*rplGetLAMn(3);
        WORDPTR endoflist=*rplGetLAMn(2);

        if(nextobj>=endoflist) {
            // THE LAST ARGUMENT WAS ALREADY PROCESSED, IF THERE IS AN OPERATOR WE NEED TO APPLY IT

            WORDPTR Opcodeptr=*rplGetLAMn(1);
            WORD Opcode=(Opcodeptr==zero_bint)? 0:*Opcodeptr;


            WORDPTR *prevDStk;
            if(Opcodeptr==zero_bint) prevDStk = rplUnprotectData();
            else prevDStk=DStkProtect;
            BINT newdepth=(BINT)(DSTop-prevDStk);


            if(Opcode) {
                if(Opcode==CMD_OVR_FUNCEVAL) {
                    // SPECIAL CASE, ALL ARGUMENTS WERE EVALUATED BUT ACTUAL FUNCTION NEEDS TO BE CALLED HERE
                    rplPushData(endoflist);
                    ++newdepth;
                    // DO MINIMAL TYPE CHECKING, LAST ARGUMENT HAS TO BE
                    // AN IDENT, OTHERWISE THE RESULT IS INVALID
                    if(!ISIDENT(*rplPeekData(1)) && !ISLIBPTR(*rplPeekData(1))) {
                        // IT SHOULD ACTUALLY RETURN SOMETHING LIKE "INVALID USER FUNCTION"
                        DSTop=rplUnprotectData();
                        rplCleanupLAMs(0);
                        IPtr=rplPopRet();
                        rplError(ERR_INVALIDUSERDEFINEDFUNCTION);
                        CurOpcode=(CMD_OVR_EVAL);
                        return;
                    }

                    rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE SYMBEVAL1ERR WORD


                    // PUSH THE NEXT OBJECT IN THE STACK
                    rplPushData(Opcodeptr);

                    rplPutLAMn(1,(WORDPTR)zero_bint);  // SIGNAL OPCODE IS DONE
                    // AND EXECUTION WILL CONTINUE AT EVAL
                    IPtr+=3;
                    return;


                }

                //if(newdepth!=1)
                    // DO SYMBOLIC WRAP ON ALL OBJECTS THAT ARE NOT MATRICES OR LISTS
                    //rplSymbWrapN(1,newdepth);

                    // PUSH THE OPERATOR IN THE STACK AND EVAL IT. THIS SHOULD APPLY THE OPERATOR IF THE RESULT IS SYMBOLIC
                    // OTHERWISE IT WILL CALCULATE IT

                    rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE SYMBEVAL1ERR WORD

                    if((Opcode==CMD_OPENBRACKET) || (Opcode==CMD_LISTOPENBRACKET)) {
                        // SPECIAL CASE, THESE COMMANDS NEED THE NUMBER OF ARGUMENTS PUSHED ON THE STACK
                        rplNewBINTPush(newdepth,DECBINT);
                    }


                    if((Opcode==CMD_OVR_MUL)||(Opcode==CMD_OVR_ADD)) {
                        // CHECK FOR FLATTENED LIST, APPLY MORE THAN ONCE IF MORE THAN 2 ARGUMENTS
                        if(newdepth<=2) rplPutLAMn(1,(WORDPTR)zero_bint);  // SIGNAL OPCODE IS DONE
                    }
                    else  rplPutLAMn(1,(WORDPTR)zero_bint);  // SIGNAL OPCODE IS DONE

                    // PUSH THE NEXT OBJECT IN THE STACK
                    rplPushData(Opcodeptr);

                    // AND EXECUTION WILL CONTINUE AT EVAL
                    IPtr+=3;

                    return;


            }

            if(newdepth!=1) {
                rplCleanupLAMs(0);
                IPtr=rplPopRet();
                rplError(ERR_BADARGCOUNT);
                CurOpcode=(CMD_OVR_NUM);
                return;
            }
            // HERE WE ARE SUPPOSED TO HAVE ONLY ONE ARGUMENT ON THE STACK AND THE ORIGINAL OBJECT
            rplOverwriteData(2,rplPeekData(1));
            rplDropData(1);
            // CLEANUP AND RETURN
            rplCleanupLAMs(0);
            IPtr=rplPopRet();
            CurOpcode=(CMD_OVR_NUM);
            return;

        }

        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE SYMBEVAL1ERR WORD


        // PUSH THE NEXT OBJECT IN THE STACK
        rplPushData(nextobj);

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case SYMBNUMPOST:
    {
        // HERE GETLAM1 = OPCODE, GETLAM 2 = END OF SYMBOLIC, GETLAM3 = OBJECT

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR DURING EVALUATION

        WORDPTR endoflist=*rplGetLAMn(2);
        WORDPTR nextobj=rplSkipOb(*rplGetLAMn(3));



        if(nextobj<=endoflist) rplPutLAMn(3,nextobj);    // MOVE TO THE NEXT OBJECT IN THE LIST


        IPtr=(WORDPTR) symbnum_seco;   // CONTINUE THE LOOP
        // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
        return;
    }
    case SYMBNUMERR:

        // BLAME THE ERROR ON THE LAST OBJECT EVALUATED UNLESS A MORE SPECIFIC SUSPECT WAS ALREADY BLAMED
        if((ExceptionPointer<*rplGetLAMn(4))||(ExceptionPointer>=rplSkipOb(*rplGetLAMn(4))))
                rplBlameError(*rplGetLAMn(4));

        // SAME PROCEDURE AS ENDERR
        rplRemoveExceptionHandler();
        rplPopRet();
        rplUnprotectData();
        rplRemoveExceptionHandler();

        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        // REMOVE ALL RECURSIVE ROUTINES FROM THE STACK
        while((IPtr>=symbnum_seco)&&(IPtr<symbnum_seco+5)) {
            if(RSTop<=RStk) break;  // NOTHING ELSE ON THE STACK!?
            IPtr=rplPopRet();
        }

        Exceptions=TrappedExceptions;
        ErrorCode=TrappedErrorCode;
        CurOpcode=(CMD_OVR_EVAL);
        return;


    case RULEMATCH:
    {
        //@SHORT_DESC=Find if an expression matches a rule pattern
        //@NEW
        if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
            return;
        }
        // THE ARGUMENT TYPES WILL BE CHECKED AT rplSymbRuleMatch

        BINT nsolutions=rplSymbRuleMatch2(0);
        if(Exceptions) return;

        while(nsolutions) {
        // HERE WE HAVE A NEW LOCAL ENVIRONMENT
        // PUSH THE RESULT OF THE MATCH IN THE STACK AS A LIST OF RULES

        BINT numlams=rplLAMCount(0);
        BINT f;

        if(numlams>=3) {

        for(f=3;f<=numlams;++f) {
            // CREATE A RULE FOR ALL LAMS
            rplPushData(*rplGetLAMnName(f));    // PUT THE NAME ON THE STACK
            rplPushData(*rplGetLAMn(f));        // PUT THE MATCHED EXPRESSION
            rplSymbApplyOperator(CMD_RULESEPARATOR,2);  // CREATE A RULE
            if(Exceptions) {
                rplCleanupLAMs(0);
                return;
            }
        }
        rplNewBINTPush(numlams-2,DECBINT);
        rplCreateList();
        if(Exceptions) {
            rplCleanupLAMs(0);
            return;
        }

        } else { rplPushData((WORDPTR)empty_list); }

        rplPushData(*rplGetLAMn(1));    // NULLAM1 HAS THE RESULT OF THE MATCH (0=NO MATCH, 1 = MATCH FOUND)

        rplCleanupLAMs(0);
        --nsolutions;
        }
        return;

     }

    case RULEAPPLY:
    {
        //@SHORT_DESC=Match and apply a rule to an expression
        //@NEW
        if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISSYMBOLIC(*rplPeekData(2))) {
            rplError(ERR_SYMBOLICEXPECTED);

            return;
        }
        if(!ISSYMBOLIC(*rplPeekData(1))) {
            rplError(ERR_SYMBOLICEXPECTED);

            return;
        }

        if(!rplSymbIsRule(rplPeekData(1))) {
            rplError(ERR_NOTAVALIDRULE);
            return;
        }


        rplSymbRuleApply();
        if(Exceptions) return;

        // HERE WE HAVE A NEW LOCAL ENVIRONMENT WITH THE
        // PUSH THE RESULT OF THE MATCH IN THE STACK AS A LIST OF RULES

        rplCleanupLAMs(0);
        return;

     }

    case TOFRACTION:
    {
        //@SHORT_DESC=Convert number to fraction
        if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        BINT64 Ai1,Ai2,Bi1,Bi2,A,B,k,oflowA,oflowB;
        REAL num;
        BINT isneg;
        rplReadNumberAsReal(rplPeekData(1),&num);

        isneg=num.flags&F_NEGATIVE;
        num.flags&=~F_NEGATIVE;

        Ai1=0;
        Ai2=1;
        Bi1=1;
        Bi2=0;
        A=0;
        B=1;
        // WORK WITH FRACTIONAL PART ONLY
        fracReal(&RReg[0],&num);
        // SAVE INTEGER PART FOR LATER
        ipReal(&RReg[1],&num,0);

        rplOneToRReg(2);    // CONSTANT ONE FOR INVERSE

        do {
            if(iszeroReal(&RReg[0])) break; // NO FRACTIONAL PART, THE NUMBER WAS EXACT
            divReal(&RReg[3],&RReg[2],&RReg[0]);
            ipReal(&RReg[4],&RReg[3],0);
            fracReal(&RReg[0],&RReg[3]);
            if(!inBINT64Range(&RReg[4])) {
                // DENOMINATOR TOO BIG = ERROR TOO SMALL
                break;
            }
            k=getBINT64Real(&RReg[4]);
            oflowA=(Ai1>>32)*k+(Ai2>>32)+((((Ai1&0xffffffff)*k+(Ai2&0xffffffff))+0x80000000)>>32);
            oflowA>>=31;
            oflowB=(Bi1>>32)*k+(Bi2>>32)+((((Bi1&0xffffffff)*k+(Bi2&0xffffffff))+0x80000000)>>32);
            oflowB>>=31;
            if(oflowA || oflowB) {
                // OVERFLOW! USE THE PREVIOUS RESULT
                break;
            }

            A=Ai2+k*Ai1;
            B=Bi2+k*Bi1;
            Ai2=Ai1;
            Bi2=Bi1;
            Ai1=A;
            Bi1=B;
        } while(1);


        newRealFromBINT64(&RReg[0],B,0);
        newRealFromBINT64(&RReg[2],A,0);
        RReg[1].flags&=~F_APPROX;
        mulReal(&RReg[3],&RReg[1],&RReg[0]);
        addReal(&RReg[0],&RReg[2],&RReg[3]);

        RReg[0].flags|=isneg;

        rplNewRealFromRRegPush(0);
        if(Exceptions) return;

        if(B!=1) {
        rplNewBINTPush(B,DECBINT);
        if(Exceptions) { rplDropData(1); return; }
        rplSymbApplyOperator(CMD_OVR_DIV,2);
        if(Exceptions) return;
        }
        rplOverwriteData(2,rplPeekData(1));
        rplDropData(1);

        return;
    }


    case RULESEPARATOR:
                //@SHORT_DESC=@HIDE
        return;
    case OPENBRACKET:
                //@SHORT_DESC=@HIDE
    {
        // OPERATOR USED IN SYMBOLIC NUMBERS, SHOULD CREATE SOME KIND OF SET OF NUMBERS, A LIST FOR NOW
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplCreateList();
        return;
    }
    case CLOSEBRACKET:
                //@SHORT_DESC=@HIDE
        return;
    case LISTOPENBRACKET:
                //@SHORT_DESC=@HIDE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplCreateList();
        return;
    case LISTCLOSEBRACKET:
                //@SHORT_DESC=@HIDE

    case EQUATIONOPERATOR:
                //@SHORT_DESC=@HIDE
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR arg1=rplPeekData(2);
        WORDPTR arg2=rplPeekData(1);


        // ALLOW LIST PROCESSING AND MATRIX PROCESSING FIRST
        if(ISLIST(*arg1) || ISLIST(*arg2)){
            rplListBinaryDoCmd();
            return;
        }

        if(ISSYMBOLIC(*arg1)||ISSYMBOLIC(*arg2)) {
                // MAKE SURE WE KEEP THE EQUALITY WHEN EVAL'd
            rplSymbApplyOperator(CurOpcode,2);
        }
        else rplCallOvrOperator(CMD_OVR_SUB);
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
        BYTEPTR tok=(BYTEPTR )TokenStart;
        UBINT64 Locale=rplGetSystemLocale();



        if(*tok=='(') {
            if((TokenLen==1) && (CurrentConstruct==MKPROLOG(DOSYMB,0))) {
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,OPENBRACKET));
                RetNum=OK_CONTINUE;
            }
            else RetNum=ERR_NOTMINE;
        return;
        }


        if(*tok==')') {
            if((TokenLen==1) && (CurrentConstruct==MKPROLOG(DOSYMB,0))) {
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,OPENBRACKET)); // INDICATE THE OPENING BRACKET TO MATCH
                RetNum=OK_CONTINUE;
            }
            else RetNum=ERR_NOTMINE;
        return;
        }
        if((WORD)utf82cp((char *)tok,(char *)BlankStart)==ARG_SEP(Locale)) {
            if((TokenLen==1) && (CurrentConstruct==MKPROLOG(DOSYMB,0))) {
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,COMMA));
                RetNum=OK_CONTINUE;
            }
            else RetNum=ERR_NOTMINE;
        return;
        }

        if(*tok=='{') {
            if((TokenLen==1)&&(CurrentConstruct==MKPROLOG(DOSYMB,0))) {
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LISTOPENBRACKET));
                RetNum=OK_CONTINUE;
            }
            else RetNum=ERR_NOTMINE;
            return;
        }
        if(*tok=='}') {
            if((TokenLen==1)&&(CurrentConstruct==MKPROLOG(DOSYMB,0))) {
                // ISSUE A BUILDLIST OPERATOR
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LISTOPENBRACKET));
                RetNum=OK_CONTINUE;
            }
            else RetNum=ERR_NOTMINE;
            return;
        }




        if( (TokenLen==2) && !utf8ncmp2((char *)tok,(char *)BlankStart,":→",2)) {
            if(CurrentConstruct==MKPROLOG(DOSYMB,0)) {
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,RULESEPARATOR));
                RetNum=OK_CONTINUE;
            }
            else RetNum=ERR_NOTMINE;
        return;
        }



    }

            // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
            // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        if(ISPROLOG(*DecompileObject)) {
            RetNum=OK_STARTCONSTRUCT_INFIX;
            return;
        }

        if(*DecompileObject==MKOPCODE(LIBRARY_NUMBER,COMMA)) {
            // SPECIAL TREATMENT FOR ARGUMENT SEPARATOR DUE TO LOCALE
            UBINT64 Locale=rplGetSystemLocale();

            rplDecompAppendUTF8(cp2utf8(ARG_SEP(Locale)));
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

        UBINT64 Locale=rplGetSystemLocale();
        if(*((char *)TokenStart)=='\'') {
            // FOUND END OF SYMBOLIC OBJECT

            if(TokenLen>1) NextTokenStart=(WORDPTR)(((char *)TokenStart)+1);
            RetNum= OK_ENDCONSTRUCT_INFIX;
            return;
        }

        if((WORD)utf82cp((char *)TokenStart,(char *)BlankStart)==ARG_SEP(Locale)) {
            RetNum= OK_TOKENINFO | MKTOKENINFO(1,TITYPE_COMMA,0,31);
            return;
        }
/*
        if( !utf8ncmp2((char *)TokenStart,(char *)BlankStart,":→",2) ) {
            RetNum= OK_TOKENINFO | MKTOKENINFO(2,TITYPE_BINARYOP_LEFT,2,14);
            return;
        }
*/

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
            RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_EXPRESSION,0,0);
            return;
        }
        TypeInfo=0;     // ALL COMMANDS ARE TYPE 0
        DecompHints=0;
        libGetInfo2(*ObjectPTR,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
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

        // TODO: PROPER VERIFICATION OF SYMBOLICS IS VERY COMPLEX

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
        ObjectPTR=ROMPTR_TABLE[MENUNUMBER(MenuCodeArg)+4];
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
