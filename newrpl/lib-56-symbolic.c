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
    ECMD(RULESEPARATOR,":→",MKTOKENINFO(2,TITYPE_CASBINARYOP_LEFT,2,16)), \
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
    CMD(RULEMATCH,MKTOKENINFO(9,TITYPE_NOTALLOWED,2,2)), \
    CMD(RULEAPPLY,MKTOKENINFO(9,TITYPE_NOTALLOWED,2,2)), \
    ECMD(TOFRACTION,"→Q",MKTOKENINFO(2,TITYPE_FUNCTION,1,2)), \
    ECMD(SYMBEVAL1CHK,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    ECMD(EQUATIONOPERATOR,"=",MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,14)), \
    ECMD(LISTOPENBRACKET,"{",MKTOKENINFO(1,TITYPE_OPENBRACKET,0,31)), \
    ECMD(LISTCLOSEBRACKET,"}",MKTOKENINFO(1,TITYPE_CLOSEBRACKET,0,31)), \
    CMD(RULEAPPLY1,MKTOKENINFO(10,TITYPE_NOTALLOWED,2,2)), \
    ECMD(GIVENTHAT,"|",MKTOKENINFO(1,TITYPE_CASBINARYOP_LEFT,2,17)), \
    CMD(TRIGSIN,MKTOKENINFO(7,TITYPE_CASFUNCTION,1,2)), \
    CMD(ALLROOTS,MKTOKENINFO(8,TITYPE_CASFUNCTION,1,2)), \
    ECMD(CLISTOPENBRACKET,"c{",MKTOKENINFO(2,TITYPE_OPENBRACKET,0,31)), \
    ECMD(CLISTCLOSEBRACKET,"}",MKTOKENINFO(1,TITYPE_CLOSEBRACKET,0,31)), \
    CMD(RANGE,MKTOKENINFO(5,TITYPE_FUNCTION,3,2)), \
    CMD(ASSUME,MKTOKENINFO(6,TITYPE_CASFUNCTION,2,2))



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
INCLUDE_ROMOBJECT(lib56_autosimplify_pre);
INCLUDE_ROMOBJECT(lib56_autosimplify_group1);
INCLUDE_ROMOBJECT(lib56_autosimplify_group2);
INCLUDE_ROMOBJECT(lib56_autosimplify_group3);
INCLUDE_ROMOBJECT(lib56_autosimplify_group4);
INCLUDE_ROMOBJECT(lib56_autosimplify_group5);
INCLUDE_ROMOBJECT(lib56_autosimplify_group6);
INCLUDE_ROMOBJECT(lib56_autosimplify_group7);
INCLUDE_ROMOBJECT(lib56_autosimplify_group8);
INCLUDE_ROMOBJECT(lib56_autosimplify_post);

INCLUDE_ROMOBJECT(trigsin_rules);
INCLUDE_ROMOBJECT(allroots_rules);


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)symbeval_seco,
    (WORDPTR)symbeval1_seco,
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib56_menu,
    (WORDPTR)lib56_autosimplify_pre,
    (WORDPTR)lib56_autosimplify_group1,
    (WORDPTR)lib56_autosimplify_group2,
    (WORDPTR)lib56_autosimplify_group3,
    (WORDPTR)lib56_autosimplify_group4,
    (WORDPTR)lib56_autosimplify_group5,
    (WORDPTR)lib56_autosimplify_group6,
    (WORDPTR)lib56_autosimplify_group7,
    (WORDPTR)lib56_autosimplify_group8,
    (WORDPTR)lib56_autosimplify_post,
    (WORDPTR)trigsin_rules,

    0
};


BINT rplIsTopLevelEnv(WORDPTR obj)
{

}

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
        if((lamnum>=0) && (lamnum<=nlams)) {
        lamobj=rplGetLAMnEnv(lamenv,lamnum);
        if(*lamobj==object) return 1;
        }
        }
        lamenv=rplGetNextLAMEnv(lamenv);
    }
    return 0;
}

// EXPAND A LIST OF RULES IN THE STACK
BINT rplExpandRuleList()
{
    BINT nrules=1;
    WORDPTR *savestk=DSTop;
    if(ISLIST(*rplPeekData(1))) {
        // THIS IS A RULE SET, APPLY ALL RULES IN THE LIST TO THE EXPRESSION, IN SEQUENCE
        // RETURN TOTAL NUMBER OF POSITIVE MATCHES IN THE ENTIRE SET (ZERO IF NO CHANGES WERE MADE)
        // FIRST MAKE A FULL LIST OF RULES IN THE STACK, IN ORDER OF APPLICATION

        WORDPTR *rulelist=DSTop-1;
        WORDPTR first=(*rulelist)+1;
        while(first!=rplSkipOb(*rulelist)) {

            if(rplSymbIsRule(first)) {
               rplPushData(first);
               first=rplPeekData(1);   // READ AGAIN IN CASE THERE WAS A GC
            }
            else {

            if(ISLIST(*first)) {
                rplPushDataNoGrow(first);   // OBJECT TO SKIP
                rplPushData(first);         // THE LIST ITSELF
                first=rplPeekData(1)+1;
                continue;
            } // ENTER INTO THE LIST, PUT THE LIST ON THE STACK

            if(*first==CMD_ENDLIST) {
             WORDPTR *stkptr=DSTop-1;
             while((stkptr>=rulelist) && !ISLIST(**stkptr)) { --stkptr; }   // FIND THE LIST
             if(stkptr==rulelist) break;    // END OF MAIN LIST
             first=rplSkipOb(*(stkptr-1));  // NEXT ARGUMENT IN THE PARENT LIST
             // REMOVE THE LIST AND THE OBJECT
             if(DSTop>stkptr+1) memmovew(stkptr-1,stkptr+1,(DSTop-stkptr-1)*sizeof(WORDPTR)/sizeof(WORD));
             DSTop-=2;
             continue;
            }

            // IDENTIFIERS ARE VARIABLES CONTAINING SETS OF RULES
            if(ISIDENT(*first)) {
                WORDPTR *var=rplFindLAM(first,1);
                if(!var) var=rplFindGlobal(first,1);
                if(var) {
                    if(ISLIST(*var[1])) {
                        rplPushDataNoGrow(first);   // OBJECT TO SKIP
                        rplPushData(var[1]);         // THE LIST ITSELF
                        first=rplPeekData(1)+1;
                        continue;
                    }
                    if(rplSymbIsRule(var[1])) {
                       ScratchPointer1=first;
                       rplPushData(var[1]);
                       first=ScratchPointer1;   // READ AGAIN IN CASE THERE WAS A GC
                    }
                } // TAKE THE VALUE OF THE VARIABLE AS THE NEXT RULE
            }
            }

             // JUST SKIP ANY OTHER OBJECT
             first=rplSkipOb(first);
            }


        nrules=DSTop-savestk+1;
        }
    return nrules;
}


// IMPLEMENTS THE COMMAND RULEAPPLY, CAN BE CALLED FROM OTHER CAS COMMANDS
// NEEDS 2 ARGUMENTS ON THE STACK: EXPRESSION AND RULE SET

void rplSymbRuleApply()
{
        if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        // THE ARGUMENT TYPES WILL BE CHECKED AT rplSymbRuleMatch
        BINT nrules;
        WORDPTR *savestk=DSTop;

        nrules=rplExpandRuleList();

        WORDPTR *firstrule=savestk-1;
        BINT k;
        BINT nsolutions,totalreplacements=0,prevreplacements=-1;
        WORD objhash,prevhash;
        rplPushDataNoGrow(*(savestk-2)); // COPY THE EXPRESSION
        prevhash=objhash=0;
        while(totalreplacements!=prevreplacements) {
        prevreplacements=totalreplacements;
        WORD tmphash=(WORD)rplObjChecksum(rplPeekData(1));
        if(prevhash==tmphash)
            break;    // SAME OBJECT AFTER 2 ROUNDS OF RULES BEING APPLIED
        prevhash=tmphash;

        for(k=0;k<nrules;++k) {

        if(rplSymbIsRule(firstrule[k])) {
            rplPushDataNoGrow(firstrule[k]);
            nsolutions=rplSymbRuleMatch();
            if(Exceptions) { DSTop=savestk; return; }
            totalreplacements+=nsolutions;
            rplOverwriteData(3,rplPeekData(1));  // REPLACE THE ORIGINAL EXPRESSION WITH THE NEW ONE
            rplDropData(2);
            // CLEANUP ALL LAM ENVIRONMENTS
            while(nsolutions>0) { rplCleanupLAMs(firstrule[k]); --nsolutions; }
        }
        }

        // WE APPLIED ALL RULES

        }

        firstrule[-1]=rplPeekData(1); // REPLACE ORIGINAL EXPRESSION WITH RESULT
        firstrule[0]=rplNewBINT(totalreplacements,DECBINT);
        DSTop=savestk;
        return;

}


// IMPLEMENTS THE COMMAND RULEAPPLY1, CAN BE CALLED FROM OTHER CAS COMMANDS
// NEEDS 2 ARGUMENTS ON THE STACK: EXPRESSION AND RULE SET

void rplSymbRuleApply1()
{
        if(rplDepthData()<2) {
                rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        // THE ARGUMENT TYPES WILL BE CHECKED AT rplSymbRuleMatch
        BINT nrules;
        WORDPTR *savestk=DSTop;

        nrules=rplExpandRuleList();

        WORDPTR *firstrule=savestk-1;
        BINT k;
        BINT nsolutions,totalreplacements=0;
        rplPushDataNoGrow(*(savestk-2)); // COPY THE EXPRESSION



        for(k=0;k<nrules;++k) {

        if(rplSymbIsRule(firstrule[k])) {
            rplPushDataNoGrow(firstrule[k]);
            nsolutions=rplSymbRuleMatch();
            if(Exceptions) { DSTop=savestk; return; }
            totalreplacements+=nsolutions;
            rplOverwriteData(3,rplPeekData(1));  // REPLACE THE ORIGINAL EXPRESSION WITH THE NEW ONE
            rplDropData(2);
            // CLEANUP ALL LAM ENVIRONMENTS
            while(nsolutions>0) { rplCleanupLAMs(firstrule[k]); --nsolutions; }
        }
        }

        // WE APPLIED ALL RULES


        firstrule[-1]=rplPeekData(1); // REPLACE ORIGINAL EXPRESSION WITH RESULT
        firstrule[0]=rplNewBINT(totalreplacements,DECBINT);
        DSTop=savestk;

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
        // DELIBERATE FALL-THROUGH
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

            // TRACK FORCED_RAD FLAG THROUGH THE OPCODE'S ARGUMENTS
            rplClrSystemFlag(FL_FORCED_RAD);

            if(OPCODE(Opcode)==OVR_FUNCEVAL) {
                // DON'T MARK THE LAST OBJECT AS THE END OF OBJECT
                WORDPTR lastobj=object;
                while(rplSkipOb(lastobj)!=endobject) lastobj=rplSkipOb(lastobj);
                endobject=lastobj;
            }
            else {
                BINT tokeninfo=rplSymbGetTokenInfo(Opcodeptr);

                if((TI_TYPE(tokeninfo)==TITYPE_CASFUNCTION)||(TI_TYPE(tokeninfo)==TITYPE_CASBINARYOP_LEFT)||(TI_TYPE(tokeninfo)==TITYPE_CASBINARYOP_RIGHT)) {

                    // HANDLE SPECIAL CASE OF CAS COMMANDS, NO NEED TO EVALUATE ARGUMENTS ON THOSE
                    WORDPTR *argstart=DSTop;
                    ScratchPointer1=object;
                    ScratchPointer2=endobject;
                    // PUSH ALL ARGUMENTS TO THE STACK UNEVALUATED
                    while(ScratchPointer1<ScratchPointer2) {
                        rplPushData(ScratchPointer1);
                        if(Exceptions) { rplCleanupLAMs(0); return; }
                        ScratchPointer1=rplSkipOb(ScratchPointer1);
                    }
                    // SIGNAL THAT WE ARE DONE PROCESSING THE LAST ARGUMENT
                    object=endobject=ScratchPointer2;


                    rplCreateLAM((WORDPTR)nulllam_ident,endobject);     // LAM 2 = END OF CURRENT OBJECT
                    if(Exceptions) { rplCleanupLAMs(0); return; }

                    rplCreateLAM((WORDPTR)nulllam_ident,object);     // LAM 3 = NEXT OBJECT TO PROCESS
                    if(Exceptions) { rplCleanupLAMs(0); return; }

                    rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);      // LAM 4 = ANY ARGUMENT CHANGED?
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
                    if((rplPeekRet(1)<symbeval1_seco)||(rplPeekRet(1)>symbeval1_seco+6))
                    {
                        // THIS EVAL IS NOT INSIDE A RECURSIVE LOOP
                        // PUSH AUTOSIMPLIFY TO BE EXECUTED AFTER EVAL
                        rplPushRet((WORDPTR)symbeval1_seco+6);
                    }
                    IPtr=(WORDPTR) symbeval1_seco;
                    CurOpcode=(CMD_OVR_EVAL);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

                    // DO NOT PROTECT THE DATA, WAS ALREADY PROTECTED BEFORE THE ARGUMENTS WERE PUSHED
                    rplProtectData();   // PROTECT STACK ABOVE
                    DStkProtect=argstart;   // MOVE STACK PROTECTION TO ALLOW ACCESS TO THE ARGUMENTS
                    return;
                }
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
    WORD Opcode;
    WORDPTR Opcodeptr=rplSymbMainOperatorPTR(object);
    if(!Opcodeptr) { Opcodeptr=(WORDPTR)zero_bint; Opcode=0; }
    else Opcode=*Opcodeptr;

    rplCreateLAM((WORDPTR)nulllam_ident,Opcodeptr);     // LAM 1 = OPCODE
    if(Exceptions) { rplCleanupLAMs(0); return; }

    object++;
    if(Opcode) {
        object++;
        // TRACK FORCED_RAD FLAG THROUGH THE OPCODE'S ARGUMENTS
        rplClrSystemFlag(FL_FORCED_RAD);

        if(OPCODE(Opcode)==OVR_FUNCEVAL) {
            // DON'T MARK THE LAST OBJECT AS THE END OF OBJECT
            WORDPTR lastobj=object;
            while(rplSkipOb(lastobj)!=endobject) lastobj=rplSkipOb(lastobj);
            endobject=lastobj;
        }
        else {
            BINT tokeninfo=rplSymbGetTokenInfo(Opcodeptr);

            if((TI_TYPE(tokeninfo)==TITYPE_CASFUNCTION)||(TI_TYPE(tokeninfo)==TITYPE_CASBINARYOP_LEFT)||(TI_TYPE(tokeninfo)==TITYPE_CASBINARYOP_RIGHT)) {

                // HANDLE SPECIAL CASE OF CAS COMMANDS, NO NEED TO EVALUATE ARGUMENTS ON THOSE
                WORDPTR *argstart=DSTop;
                ScratchPointer1=object;
                ScratchPointer2=endobject;
                ScratchPointer3=mainobj;
                // PUSH ALL ARGUMENTS TO THE STACK UNEVALUATED
                while(ScratchPointer1<ScratchPointer2) {
                    rplPushData(ScratchPointer1);
                    if(Exceptions) { rplCleanupLAMs(0); return; }
                    ScratchPointer1=rplSkipOb(ScratchPointer1);
                }
                // SIGNAL THAT WE ARE DONE PROCESSING THE LAST ARGUMENT
                object=endobject=ScratchPointer2;
                mainobj=ScratchPointer3;

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

                // DO NOT PROTECT THE DATA, WAS ALREADY PROTECTED BEFORE THE ARGUMENTS WERE PUSHED
                rplProtectData();   // PROTECT STACK ABOVE
                DStkProtect=argstart;   // MOVE STACK PROTECTION TO ALLOW ACCESS TO THE ARGUMENTS
                return;
            }
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

        // TRACK FORCED_RAD FLAG THROUGH THE OPCODE'S ARGUMENTS
        rplClrSystemFlag(FL_FORCED_RAD);


        if(OPCODE(Opcode)==OVR_FUNCEVAL) {
            // DON'T MARK THE LAST OBJECT AS THE END OF OBJECT
            WORDPTR lastobj=object;
            while(rplSkipOb(lastobj)!=endobject) lastobj=rplSkipOb(lastobj);
            endobject=lastobj;
        }
        else {
            BINT tokeninfo=rplSymbGetTokenInfo(Opcodeptr);

            if((TI_TYPE(tokeninfo)==TITYPE_CASFUNCTION)||(TI_TYPE(tokeninfo)==TITYPE_CASBINARYOP_LEFT)||(TI_TYPE(tokeninfo)==TITYPE_CASBINARYOP_RIGHT)) {

                // HANDLE SPECIAL CASE OF CAS COMMANDS, NO NEED TO EVALUATE ARGUMENTS ON THOSE
                WORDPTR *argstart=DSTop;
                ScratchPointer1=object;
                ScratchPointer2=endobject;
                ScratchPointer3=mainobj;
                // PUSH ALL ARGUMENTS TO THE STACK UNEVALUATED
                while(ScratchPointer1<ScratchPointer2) {
                    rplPushData(ScratchPointer1);
                    if(Exceptions) { rplCleanupLAMs(0); return; }
                    ScratchPointer1=rplSkipOb(ScratchPointer1);
                }
                // SIGNAL THAT WE ARE DONE PROCESSING THE LAST ARGUMENT
                object=endobject=ScratchPointer2;
                mainobj=ScratchPointer3;

                rplCreateLAM((WORDPTR)nulllam_ident,endobject);     // LAM 2 = END OF CURRENT OBJECT
                if(Exceptions) { rplCleanupLAMs(0); return; }

                rplCreateLAM((WORDPTR)nulllam_ident,object);     // LAM 3 = NEXT OBJECT TO PROCESS
                if(Exceptions) { rplCleanupLAMs(0); return; }

                rplCreateLAM((WORDPTR)nulllam_ident,mainobj);     // LAM 4 = MAIN SYMBOLIC EXPRESSION, FOR CIRCULAR REFERENCE CHECK
                if(Exceptions) { rplCleanupLAMs(0); return; }

                rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);   // LAM 5 = TRACK FORCED_RAD FLAGS ACROSS ARGUMENTS
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
                /*
                if((rplPeekRet(1)<symbnum_seco)||(rplPeekRet(1)>symbnum_seco+4))
                {
                    // THIS ->NUM IS NOT INSIDE A RECURSIVE LOOP
                    // PUSH AUTOSIMPLIFY TO BE EXECUTED AFTER ->NUM
                    rplPushRet((WORDPTR)symbnum_seco+6);
                }
                */
                IPtr=(WORDPTR) symbnum_seco;
                CurOpcode=(CMD_OVR_NUM);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

                // DO NOT PROTECT THE DATA, WAS ALREADY PROTECTED BEFORE THE ARGUMENTS WERE PUSHED
                rplProtectData();   // PROTECT STACK ABOVE
                DStkProtect=argstart;   // MOVE STACK PROTECTION TO ALLOW ACCESS TO THE ARGUMENTS
                return;
            }
        }



    }

    rplCreateLAM((WORDPTR)nulllam_ident,endobject);     // LAM 2 = END OF CURRENT OBJECT
    if(Exceptions) { rplCleanupLAMs(0); return; }

    rplCreateLAM((WORDPTR)nulllam_ident,object);     // LAM 3 = NEXT OBJECT TO PROCESS
    if(Exceptions) { rplCleanupLAMs(0); return; }

    rplCreateLAM((WORDPTR)nulllam_ident,mainobj);     // LAM 4 = MAIN SYMBOLIC EXPRESSION, FOR CIRCULAR REFERENCE CHECK
    if(Exceptions) { rplCleanupLAMs(0); return; }

    rplCreateLAM((WORDPTR)nulllam_ident,(WORDPTR)zero_bint);   // LAM 5 = TRACK FORCED_RAD FLAGS ACROSS ARGUMENTS
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
    /*
    if((rplPeekRet(1)<symbnum_seco)||(rplPeekRet(1)>symbnum_seco+4))
    {
        // THIS ->NUM IS NOT INSIDE A RECURSIVE LOOP
        // PUSH AUTOSIMPLIFY TO BE EXECUTED AFTER ->NUM
        rplPushRet((WORDPTR)symbnum_seco+6);
    }
    */
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

        if(ISCOMPLEX(*rplPeekData(2))) {
            WORDPTR newsymb=rplComplexToSymb(rplPeekData(2));
            if(!newsymb) return;
            rplOverwriteData(2,newsymb);
            arg1=rplPeekData(2);
            arg2=rplPeekData(1);
        }

        if(ISCOMPLEX(*rplPeekData(1))) {
            WORDPTR newsymb=rplComplexToSymb(rplPeekData(1));
            if(!newsymb) return;
            rplOverwriteData(1,newsymb);
            arg1=rplPeekData(2);
            arg2=rplPeekData(1);
        }



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

        if(ISCOMPLEX(*rplPeekData(2))) {
            WORDPTR newsymb=rplComplexToSymb(rplPeekData(2));
            if(Exceptions) return;
            rplOverwriteData(2,newsymb);
            arg1=rplPeekData(2);
            arg2=rplPeekData(1);
        }

        if(ISCOMPLEX(*rplPeekData(1))) {
            WORDPTR newsymb=rplComplexToSymb(rplPeekData(1));
            if(Exceptions) return;
            rplOverwriteData(1,newsymb);
            arg1=rplPeekData(2);
            arg2=rplPeekData(1);
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

            if(ISPROLOG(*ScratchPointer1) || ISBINT(*ScratchPointer1)  || ISCONSTANT(*ScratchPointer1)) {
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

        if(ISCOMPLEX(*rplPeekData(2))) {
            WORDPTR newsymb=rplComplexToSymb(rplPeekData(2));
            if(Exceptions) return;
            rplOverwriteData(2,newsymb);

        }

        if(ISCOMPLEX(*rplPeekData(1))) {
            WORDPTR newsymb=rplComplexToSymb(rplPeekData(1));
            if(Exceptions) return;
            rplOverwriteData(1,newsymb);
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

                    if((Opcode==CMD_OPENBRACKET) || (Opcode==CMD_LISTOPENBRACKET) || (Opcode==CMD_CLISTOPENBRACKET)) {
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

            // SET/CLEAR THE FORCED_RAD ANGLE
            if(*rplGetLAMn(5)==(WORDPTR)zero_bint) rplClrSystemFlag(FL_FORCED_RAD);
            else rplSetSystemFlag(FL_FORCED_RAD);


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


        // TRACK THE FORCED_RAD FLAG ACROSS ARGUMENTS
        BINT forced_rad;
        if(nextobj>endoflist) {
            // WE ALREADY RAN THE OPCODE, THE RESULTING FLAG IS THE ONLY ONE THAT MATTERS
            forced_rad=0;
        }
        else {
            // BEFORE RUNNING THE OPCODE, NEED TO COMBINE THE FLAGS OF THE ARGUMENTS
        if(*rplGetLAMn(5)==(WORDPTR)zero_bint) forced_rad=0;
        else forced_rad=1;
        }
        if(rplTestSystemFlag(FL_FORCED_RAD)) forced_rad|=1;
        if(forced_rad) rplPutLAMn(5,(WORDPTR)one_bint);
        else rplPutLAMn(5,(WORDPTR)zero_bint);

        if(nextobj<endoflist) rplClrSystemFlag(FL_FORCED_RAD);    // CLEAR THE FLAG BEFORE ANALYZING NEXT ARGUMENT
        else {
         if(forced_rad) rplSetSystemFlag(FL_FORCED_RAD);
         else rplClrSystemFlag(FL_FORCED_RAD);
        }


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
        rplStripTagStack(1);

        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryDoCmd();
            return;
        }
        if(ISMATRIX(*rplPeekData(1))) {
           // TODO: AUTOSIMPLIFY WITHIN VECTORS AND MATRICES ELEMENT BY ELEMENT
           rplMatrixUnary(CMD_AUTOSIMPLIFY);
            return;
        }
        if(!ISSYMBOLIC(*rplPeekData(1))) return;    // LEAVE IT ON THE STACK, NOT A SYMBOLIC

        WORD hash=0,prevhash;

        rplSymbAutoSimplify();      // A STAGE OF NUMERIC REDUCTION
        if(Exceptions) return;


        if(!rplTestSystemFlag(FL_AUTOSIMPRULES)) {

        WORDPTR *stksave=DSTop;

        rplPushDataNoGrow((WORDPTR)lib56_autosimplify_pre);
        rplCallOperator(CMD_RULEAPPLY1);
        //DEBUG ONLY:
        if(!rplIsFalse(rplPeekData(1))) rplOverwriteData(1,rplPeekData(2));
        else DSTop=stksave;
        // NON-DEBUG:
        // DSTop=stksave;

        if(Exceptions)  return;

        do {

        prevhash=hash;



        if(!rplTestSystemFlag(FL_AUTOSIMPGROUP1)) {
        rplPushDataNoGrow((WORDPTR)lib56_autosimplify_group1);
        rplCallOperator(CMD_RULEAPPLY1);
        //DEBUG ONLY:
        if(!rplIsFalse(rplPeekData(1))) rplOverwriteData(1,rplPeekData(2));
        else DSTop=stksave;
        // NON-DEBUG:
        // DSTop=stksave;
        if(Exceptions)  return;
        }

        if(!rplTestSystemFlag(FL_AUTOSIMPGROUP2)) {
        rplPushDataNoGrow((WORDPTR)lib56_autosimplify_group2);
        rplCallOperator(CMD_RULEAPPLY1);
        //DEBUG ONLY:
        if(!rplIsFalse(rplPeekData(1))) rplOverwriteData(1,rplPeekData(2));
        else DSTop=stksave;
        // NON-DEBUG:
        // DSTop=stksave;
        if(Exceptions)  return;
        }

        if(!rplTestSystemFlag(FL_AUTOSIMPGROUP3)) {
        rplPushDataNoGrow((WORDPTR)lib56_autosimplify_group3);
        rplCallOperator(CMD_RULEAPPLY1);
        //DEBUG ONLY:
        if(!rplIsFalse(rplPeekData(1))) rplOverwriteData(1,rplPeekData(2));
        else DSTop=stksave;
        // NON-DEBUG:
        // DSTop=stksave;
        if(Exceptions)  return;
        }

        if(!rplTestSystemFlag(FL_AUTOSIMPGROUP4)) {
        rplPushDataNoGrow((WORDPTR)lib56_autosimplify_group4);
        rplCallOperator(CMD_RULEAPPLY1);
        //DEBUG ONLY:
        if(!rplIsFalse(rplPeekData(1))) rplOverwriteData(1,rplPeekData(2));
        else DSTop=stksave;
        // NON-DEBUG:
        // DSTop=stksave;
        if(Exceptions)  return;
        }

        if(!rplTestSystemFlag(FL_AUTOSIMPGROUP5)) {
        rplPushDataNoGrow((WORDPTR)lib56_autosimplify_group5);
        rplCallOperator(CMD_RULEAPPLY1);
        //DEBUG ONLY:
        if(!rplIsFalse(rplPeekData(1))) rplOverwriteData(1,rplPeekData(2));
        else DSTop=stksave;
        // NON-DEBUG:
        // DSTop=stksave;
        if(Exceptions)  return;
        }

        if(!rplTestSystemFlag(FL_AUTOSIMPGROUP6)) {
        rplPushDataNoGrow((WORDPTR)lib56_autosimplify_group6);
        rplCallOperator(CMD_RULEAPPLY1);
        //DEBUG ONLY:
        if(!rplIsFalse(rplPeekData(1))) rplOverwriteData(1,rplPeekData(2));
        else DSTop=stksave;
        // NON-DEBUG:
        // DSTop=stksave;
        if(Exceptions)  return;
        }

        if(!rplTestSystemFlag(FL_AUTOSIMPGROUP7)) {
        rplPushDataNoGrow((WORDPTR)lib56_autosimplify_group7);
        rplCallOperator(CMD_RULEAPPLY1);
        //DEBUG ONLY:
        if(!rplIsFalse(rplPeekData(1))) rplOverwriteData(1,rplPeekData(2));
        else DSTop=stksave;
        // NON-DEBUG:
        // DSTop=stksave;
        if(Exceptions)  return;
        }

        if(!rplTestSystemFlag(FL_AUTOSIMPGROUP8)) {
        rplPushDataNoGrow((WORDPTR)lib56_autosimplify_group8);
        rplCallOperator(CMD_RULEAPPLY1);
        //DEBUG ONLY:
        if(!rplIsFalse(rplPeekData(1))) rplOverwriteData(1,rplPeekData(2));
        else DSTop=stksave;
        // NON-DEBUG:
        // DSTop=stksave;
        if(Exceptions)  return;
        }


        rplSymbAutoSimplify();      // A STAGE OF NUMERIC REDUCTION
        if(Exceptions) return;

        hash=rplObjChecksum(rplPeekData(1));

        } while(prevhash!=hash);

        rplPushDataNoGrow((WORDPTR)lib56_autosimplify_post);
        rplCallOperator(CMD_RULEAPPLY1);
        //DEBUG ONLY:
        if(!rplIsFalse(rplPeekData(1))) rplOverwriteData(1,rplPeekData(2));
        DSTop=stksave;
        // NON-DEBUG:
        // DSTop=stksave;
        if(Exceptions)  return;


        }

        // REORGANIZE THE EXPRESSION IN A WAY THAT'S OPTIMIZED FOR DISPLAY
        WORDPTR newobj=rplSymbCanonicalForm(rplPeekData(1),1);
        if(newobj) rplOverwriteData(1,newobj);

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

                    if((Opcode==CMD_OPENBRACKET) || (Opcode==CMD_LISTOPENBRACKET)  || (Opcode==CMD_CLISTOPENBRACKET)) {
                        // SPECIAL CASE, THESE COMMANDS NEED THE NUMBER OF ARGUMENTS PUSHED ON THE STACK
                        rplNewBINTPush(newdepth,DECBINT);
                    }

                    if((Opcode==CMD_OVR_MUL)||(Opcode==CMD_OVR_ADD)) {
                        // CHECK FOR FLATTENED LIST, APPLY MORE THAN ONCE IF MORE THAN 2 ARGUMENTS
                        if(newdepth<=2) rplPutLAMn(1,(WORDPTR)zero_bint);  // SIGNAL OPCODE IS DONE
                    }
                    else  {
                        BINT tinfo=rplSymbGetTokenInfo(Opcodeptr);
                        if((TI_TYPE(tinfo)==TITYPE_CASFUNCTION)||(TI_TYPE(tinfo)==TITYPE_CASBINARYOP_LEFT)||(TI_TYPE(tinfo)==TITYPE_CASBINARYOP_RIGHT)) {
                            // CAS OPERATORS NEED TO EVAL THE RESULT AFTERWARDS
                            rplPutLAMn(1,symbeval_seco+2);  // SIGNAL OPCODE EVAL IS NEXT

                        }
                        else rplPutLAMn(1,(WORDPTR)zero_bint);  // SIGNAL OPCODE IS DONE
                    }

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

            // SET/CLEAR THE FORCED_RAD ANGLE
            if(*rplGetLAMn(5)==(WORDPTR)zero_bint) rplClrSystemFlag(FL_FORCED_RAD);
            else rplSetSystemFlag(FL_FORCED_RAD);

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


        // TRACK THE FORCED_RAD FLAG ACROSS ARGUMENTS
        BINT forced_rad;
        if(nextobj>endoflist) {
            // WE ALREADY RAN THE OPCODE, THE RESULTING FLAG IS THE ONLY ONE THAT MATTERS
            forced_rad=0;
        }
        else {
            // BEFORE RUNNING THE OPCODE, NEED TO COMBINE THE FLAGS OF THE ARGUMENTS
        if(*rplGetLAMn(5)==(WORDPTR)zero_bint) forced_rad=0;
        else forced_rad=1;
        }
        if(rplTestSystemFlag(FL_FORCED_RAD)) forced_rad|=1;
        if(forced_rad) rplPutLAMn(5,(WORDPTR)one_bint);
        else rplPutLAMn(5,(WORDPTR)zero_bint);

        if(nextobj<endoflist) rplClrSystemFlag(FL_FORCED_RAD);    // CLEAR THE FLAG BEFORE ANALYZING NEXT ARGUMENT
        else {
         if(forced_rad) rplSetSystemFlag(FL_FORCED_RAD);
         else rplClrSystemFlag(FL_FORCED_RAD);
        }

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

                    if((Opcode==CMD_OPENBRACKET) || (Opcode==CMD_LISTOPENBRACKET)  || (Opcode==CMD_CLISTOPENBRACKET)) {
                        // SPECIAL CASE, THESE COMMANDS NEED THE NUMBER OF ARGUMENTS PUSHED ON THE STACK
                        rplNewBINTPush(newdepth,DECBINT);
                    }


                    if((Opcode==CMD_OVR_MUL)||(Opcode==CMD_OVR_ADD)) {
                        // CHECK FOR FLATTENED LIST, APPLY MORE THAN ONCE IF MORE THAN 2 ARGUMENTS
                        if(newdepth<=2) rplPutLAMn(1,(WORDPTR)zero_bint);  // SIGNAL OPCODE IS DONE
                    }
                    else  {
                        BINT tinfo=rplSymbGetTokenInfo(Opcodeptr);
                        if((TI_TYPE(tinfo)==TITYPE_CASFUNCTION)||(TI_TYPE(tinfo)==TITYPE_CASBINARYOP_LEFT)||(TI_TYPE(tinfo)==TITYPE_CASBINARYOP_RIGHT)) {
                            // CAS OPERATORS NEED TO ->NUM THE RESULT AFTERWARDS
                            rplPutLAMn(1,symbnum_seco+2);  // SIGNAL OPCODE ->NUM IS NEXT

                        }
                        else rplPutLAMn(1,(WORDPTR)zero_bint);  // SIGNAL OPCODE IS DONE
                    }
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

            // SET/CLEAR THE FORCED_RAD ANGLE
            if(*rplGetLAMn(5)==(WORDPTR)zero_bint) rplClrSystemFlag(FL_FORCED_RAD);
            else rplSetSystemFlag(FL_FORCED_RAD);


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

        // TRACK THE FORCED_RAD FLAG ACROSS ARGUMENTS
        BINT forced_rad;
        if(nextobj>endoflist) {
            // WE ALREADY RAN THE OPCODE, THE RESULTING FLAG IS THE ONLY ONE THAT MATTERS
            forced_rad=0;
        }
        else {
            // BEFORE RUNNING THE OPCODE, NEED TO COMBINE THE FLAGS OF THE ARGUMENTS
        if(*rplGetLAMn(5)==(WORDPTR)zero_bint) forced_rad=0;
        else forced_rad=1;
        }
        if(rplTestSystemFlag(FL_FORCED_RAD)) forced_rad|=1;
        if(forced_rad) rplPutLAMn(5,(WORDPTR)one_bint);
        else rplPutLAMn(5,(WORDPTR)zero_bint);

        if(nextobj<endoflist) rplClrSystemFlag(FL_FORCED_RAD);    // CLEAR THE FLAG BEFORE ANALYZING NEXT ARGUMENT
        else {
         if(forced_rad) rplSetSystemFlag(FL_FORCED_RAD);
         else rplClrSystemFlag(FL_FORCED_RAD);
        }


        if(nextobj<=endoflist) {
            rplPutLAMn(3,nextobj);    // MOVE TO THE NEXT OBJECT IN THE LIST
        }


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
        rplStripTagStack(2);

        // THE ARGUMENT TYPES WILL BE CHECKED AT rplSymbRuleMatch

        BINT nsolutions=rplSymbRuleMatch(),nsol2=nsolutions;
        if(Exceptions) return;

        while(nsolutions) {
        // HERE WE HAVE A NEW LOCAL ENVIRONMENT
        // PUSH THE RESULT OF THE MATCH IN THE STACK AS A LIST OF RULES

        BINT numlams=rplLAMCount(0);
        BINT f;

        if(numlams>=1) {

        for(f=1;f<=numlams;++f) {
            // CREATE A RULE FOR ALL LAMS
            if(f>1) {
            rplPushData(*rplGetLAMnName(f));    // PUT THE NAME ON THE STACK
            rplPushData(*rplGetLAMn(f));        // PUT THE MATCHED EXPRESSION
            rplSymbApplyOperator(CMD_RULESEPARATOR,2);  // CREATE A RULE
            if(Exceptions) {
                rplCleanupLAMs(0);
                return;
            }
            }
            else {
                rplPushData(*rplGetLAMn(f));        // PUT THE MATCHED SUBEXPRESSION
            }

        }
        rplNewBINTPush(numlams,DECBINT);
        rplCreateList();
        if(Exceptions) {
            rplCleanupLAMs(0);
            return;
        }

        } else { rplPushData((WORDPTR)empty_list); }

        rplCleanupLAMs(0);
        --nsolutions;
        }
        rplNewBINTPush(nsol2,DECBINT);
        return;

     }

    case RULEAPPLY:
    {
        //@SHORT_DESC=Match and apply a rule to an expression repeatedly
        //@NEW
        rplSymbRuleApply();

        if(!Exceptions) {
        // REORGANIZE THE EXPRESSION IN A WAY THAT'S OPTIMIZED FOR DISPLAY
        WORDPTR newobj=rplSymbCanonicalForm(rplPeekData(2),1);
        if(newobj) rplOverwriteData(2,newobj); // REPLACE ORIGINAL EXPRESSION WITH RESULT
        }

        return;
     }

    case RULEAPPLY1:
    {
        //@SHORT_DESC=Match and apply a rule to an expression only once
        //@NEW
        rplSymbRuleApply1();

        if(!Exceptions) {
        // REORGANIZE THE EXPRESSION IN A WAY THAT'S OPTIMIZED FOR DISPLAY
        WORDPTR newobj=rplSymbCanonicalForm(rplPeekData(2),1);
        if(newobj) rplOverwriteData(2,newobj); // REPLACE ORIGINAL EXPRESSION WITH RESULT
        }


        return;
     }

    case TOFRACTION:
    {
        //@SHORT_DESC=Convert number to fraction
        if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryDoCmd();
            return;
        }

        if(ISSYMBOLIC(*rplPeekData(1))) {
            rplSymbApplyOperator(CurOpcode,1);
            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        REAL num;

#define t1 RReg[0]
#define t2 RReg[1]
#define t3 RReg[2]
#define n  RReg[4]
#define d  RReg[5]
#define a  RReg[6]
#define b  RReg[7]
#define c  RReg[8]
#define p  RReg[9]

#define NUM2REG(reg,nn) { reg.len=1; reg.data[0]=nn; reg.exp=0; reg.flags=0; }
#define ZERO_REG(reg) { reg.len=1; reg.data[0]=0; reg.exp=0; reg.flags=0; }
#define ONE_REG(reg) { reg.len=1; reg.data[0]=1; reg.exp=0; reg.flags=0; }


        BINT s,d0exp;
        rplReadNumberAsReal(rplPeekData(1),&num);
        if(Exceptions) return;


        /*
         **********************************************************************
         *************** ORIGINAL PDQ ALGORITHM SOURCE CODE *******************
         *************** PDQ Algorithm by Joe Horn ****************************
         *************** Adapted and ported to newRPL *************************
         **********************************************************************
        pdq(j,t):=BEGIN
        LOCAL n,n0,d,d0,c,p,s,t1,t2,t3,a,b;
        IF t==0 THEN
            err:=0;
            ic:=0;
            RETURN(d2f(j))  END ;
        IF FP(t) AND (ABS(t)>1) THEN RETURN("Illegal Tolerance");  END ;
        IF FP(t)==0 THEN t:=1/10^(exact(ABS(t)));  END ;
        j:=d2f(j);
        */

        BINT savesign=num.flags&F_NEGATIVE;
        num.flags&=~(F_APPROX|F_NEGATIVE);   // FROM NOW ON ALL NUMBERS ARE EXACT AND POSITIVE
        BINT saveprec=Context.precdigits;
        Context.precdigits=MAX_USERPRECISION;   // ALLOW INTEGERS TO GROW UP TO MAXIMUM SYSTEM PRECISION


        copyReal(&n,&num); //  n:=numer(j);
        d0exp=-n.exp;
        n.exp=0;           // d0:=denom(j);
        ZERO_REG(c);        // c=0;
                            // t:=d2f(t);
        ONE_REG(a);         // a:=numer(t);  - TOLERANCE WILL BE EXPRESSED AS 1E-prec
        NUM2REG(b,2)
        b.exp=saveprec-intdigitsReal(&num);  // b:=denom(t);

                            //   n:=n0;
        ONE_REG(d);
        d.exp=d0exp;        //   d:=d0;
        ZERO_REG(c);        //   c:=0;
        ONE_REG(p);         //   p:=1;
        s=1;                //   s:=1;

        do {                //   REPEAT

        swapReal(&t1,&c);   //   t1:=c;

        divmodReal(&c,&t3,&n,&d);  // t3:=irem(n,d);
        mulReal(&c,&c,&t1);
        addReal(&c,&c,&p);  // c:=c*iquo(n,d)+p;

        swapReal(&p,&t1);   // p:=t1;
        swapReal(&n,&d);    //  t2:=d; n:=t2;
        swapReal(&d,&t3);   //  d:=t3;

        s=-s;               // s:=-s

        // AT THIS POINT, t1 AND t2 AND t3 ARE FREE TO USE
        mulReal(&t1,&b,&d);
        mulReal(&t3,&a,&c);
        t3.exp+=d0exp;          // t3=a*c*d0

        } while(gtReal(&t1,&t3));  //  UNTIL (b*d)<=(c*a*d0);

        // WHEN THE LOOP ENDS, t3=a*c*d0, t1=b*d, t2 IS FREE TO USE

        subReal(&t2,&t3,&t1);   // t2=c*a*d0-b*d
        mulReal(&t1,&a,&p);
        t1.exp+=d0exp;          // t1=p*a*d0
        mulReal(&t3,&b,&n);
        addReal(&t1,&t1,&t3);   // t1=p*a*d0+b*n

        divmodReal(&t1,&t3,&t2,&t1);    // t1:=iquo(c*a*d0-b*d,p*a*d0+b*n);

        mulReal(&t2,&n,&t1);
        addReal(&t2,&t2,&d);
        if(s<0) t2.flags^=F_NEGATIVE;   //   t2:=(n*t1+d)*s;

        mulReal(&t3,&p,&t1);
        subReal(&t3,&c,&t3);            //    t3:=c-p*t1;

        num.exp=0;                      // num=n0
        mulReal(&n,&t3,&num);
        addReal(&t1,&t2,&n);            // t1=(t2+t3*n0)
        t3.exp+=d0exp;                  // t3=t3*d0

        // IGNORE ERROR FOR THIS COMMAND
        // err:=t2/(d0*t3);
        // ic:=t1;

        // ELIMINATE SOME ZEROS (UNNECESSARY)

        BINT t1zn=trailzerosReal(&t1);
        BINT t3zn=trailzerosReal(&t3);
        if(t1.exp+t1zn>t3.exp+t3zn) { t1.exp-=t3.exp+t3zn; t3.exp=-t3zn; }
        else { t3.exp-=t1.exp+t1zn; t1.exp=-t1zn; }

        Context.precdigits=saveprec;

        WORDPTR *stksave=DSTop;

        rplNewRealPush(&t1);
        if(Exceptions) { DSTop=stksave; return; }
        rplNewRealPush(&t3);
        if(Exceptions) { DSTop=stksave; return; }
        rplSymbApplyOperator(CMD_OVR_DIV,2);  // RETURN((t2+t3*n0)/(d0*t3));
        if(Exceptions) { DSTop=stksave; return; }
        if(savesign) {
            rplSymbApplyOperator(CMD_OVR_UMINUS,1);  // RETURN(-(t2+t3*n0)/(d0*t3));
            if(Exceptions) { DSTop=stksave; return; }
        }

        rplOverwriteData(2,rplPeekData(1));
        rplDropData(1);


        return;
    }

/*

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
*/

    case RULESEPARATOR:
                //@SHORT_DESC=@HIDE
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplSymbApplyOperator(CurOpcode,2);
        if(Exceptions) return;
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
        //if(!Exceptions) rplListAutoExpand(rplPeekData(1));
        return;

    case CLISTOPENBRACKET:
                //@SHORT_DESC=@HIDE
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplCreateList();
        if(!Exceptions) rplListAutoExpand(rplPeekData(1));

        return;


    case LISTCLOSEBRACKET:
                //@SHORT_DESC=@HIDE

        return;
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

    case GIVENTHAT:
                //@SHORT_DESC=@HIDE
        // OPERATOR "GIVEN THAT" WILL DO 3 THINGS:

        // A) WHEN GIVEN AN IDENT WITH ATTRIBUTES, IT WILL SUBSTITUTE ALL OCCURRENCES OF A VARIABLE WITH THE NEW ATTRIBUTES

        // B) WHEN GIVEN A RULE, IT WILL APPLY THE RULE

    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR firstexp=rplPeekData(1);
        WORDPTR endexp=rplSkipOb(firstexp);
        WORDPTR *stksave=DSTop;

        if(ISLIST(*firstexp)) {
            firstexp++;
            endexp--;   // END BEFORE THE CMD_ENDLIST WORD
        }

        if(!ISSYMBOLIC(*rplPeekData(2)) && !ISIDENT(*rplPeekData(2))) {
               rplError(ERR_SYMBOLICEXPECTED);
               return;
        }
        rplPushDataNoGrow(rplPeekData(2));

        // START APPLYING ALL ASSUMPTIONS
        while(firstexp<endexp) {

            if(ISSYMBOLIC(*firstexp)) {
                // COULD BE A SYMBOLIC LIST OR A LIST OF ARGUMENTS IN BRACKETS
                WORD opcode=rplSymbMainOperator(firstexp);
                if(opcode==CMD_OPENBRACKET) {
                    // USE ALL THE INDIVIDUAL ITEMS IN THE LIST
                    firstexp=rplSymbMainOperatorPTR(firstexp)+1;
                    continue;
                }

                firstexp=rplSymbUnwrap(firstexp);   // REMOVE ALL LAYERS OF SYMBOLICS
                if(!opcode) ++firstexp;

            }

            if(ISIDENT(*firstexp)) {

            // REPLACE ALL OCCURRENCES OF THE IDENT IN THE SYMBOLIC
            ScratchPointer4=firstexp;
            ScratchPointer5=endexp;
            WORDPTR newobj=rplSymbReplaceVar(rplPeekData(1),firstexp,firstexp);
            if(Exceptions || !newobj) {
                DSTop=stksave;
                return;
            }
            firstexp=ScratchPointer4;
            endexp=ScratchPointer5;

            rplOverwriteData(1,newobj);

            }
            else {
            // EITHER AN IDENT OR A SYMBOLIC WITH A RULE
            if(rplSymbIsRule(firstexp)) {
                // SAVE POINTERS ON THE STACK
                rplPushDataNoGrow(firstexp);
                rplPushDataNoGrow(endexp);


                // APPLY THE RULE
                rplPushDataNoGrow(rplPeekData(3));
                rplPushDataNoGrow(firstexp);

                rplSymbRuleApply();

                if(!Exceptions) {
                // REORGANIZE THE EXPRESSION IN A WAY THAT'S OPTIMIZED FOR DISPLAY
                WORDPTR newobj=rplSymbCanonicalForm(rplPeekData(2),1);
                if(newobj) rplOverwriteData(2,newobj); // REPLACE ORIGINAL EXPRESSION WITH RESULT
                }

                if(Exceptions) {
                    DSTop=stksave;
                    return;
                }

                // RELOAD ALL POINTERS
                rplOverwriteData(5,rplPeekData(2));

                endexp=rplPeekData(3);
                firstexp=rplPeekData(4);
                rplDropData(4);

            }
            else {
                rplError(ERR_NOTAVALIDRULE);
                DSTop=stksave;
                return;
            }

            firstexp=rplSkipOb(firstexp);

            }


        }   // END WHILE

        // HERE WE HAVE ALL VARIABLES SUBSTITUTED

        rplOverwriteData(3,rplPeekData(1));
        rplDropData(2);
        return;

    }



    case TRIGSIN:
        //@SHORT_DESC=Simplify replacing cos(x)^2+sin(x)^2=1
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR *stksave=DSTop;
        rplPushDataNoGrow((WORDPTR)trigsin_rules);
        rplSymbRuleApply();
        DSTop=stksave;
        if(!Exceptions) rplSymbAutoSimplify();
        return;
    }

    case ALLROOTS:
        //@SHORT_DESC=Expand powers with rational exponents to consider all roots
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR *stksave=DSTop;
        rplPushDataNoGrow((WORDPTR)allroots_rules);
        rplSymbRuleApply();
        DSTop=stksave;
        if(!Exceptions) rplSymbAutoSimplify();
        return;
    }

    case RANGE:
        //@SHORT_DESC=Create a case-list of integers in the given range.
        //@NEW

    {
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(3);


        WORDPTR argstart,argend,argstep;

        argstart=rplPeekData(3);
        argend=rplPeekData(2);
        argstep=rplPeekData(1);

        if( ISSYMBOLIC(*argstart)||ISIDENT(*argstart)||ISCONSTANT(*argstart) ||
                ISSYMBOLIC(*argend)||ISIDENT(*argend)||ISCONSTANT(*argend) ||
                ISSYMBOLIC(*argstep)||ISIDENT(*argstep)||ISCONSTANT(*argstep)
                ) {
            rplSymbApplyOperator(CurOpcode,3);
            return;
        }


        if(ISBINT(*argstart)&&ISBINT(*argend)&&(ISBINT(*argstep)))
        {
            // ALL INTEGERS!, DO THIS FASTER
            BINT64 start,end,step,k;
            BINT size=1;
            start=rplReadBINT(argstart);
            end=rplReadBINT(argend);
            step=rplReadBINT(argstep);
            if(end<start) {
                if(step>=0) step=(end-start)*2;
            }
            else if(step<=0) step=(end-start)*2;

            if(step>0) {
                for(k=start;k<=end;k+=step) {
                    if( (k<=MAX_SINT)&&(k>=MIN_SINT)) size+=1;
                    else size+=3;
                }
            } else {
                for(k=start;k>=end;k+=step) {
                    if( (k<=MAX_SINT)&&(k>=MIN_SINT)) size+=1;
                    else size+=3;
                }

            }
            // HERE WE HAVE THE SIZE OF THE LIST
            WORDPTR newlist=rplAllocTempOb(size),ptr;
            if(!newlist) return;
            newlist[0]=MKPROLOG(DOCASELIST,size);
            newlist[size]=CMD_ENDLIST;
            ptr=newlist+1;
            if(step>0) {
                for(k=start;k<=end;k+=step) {
                    ptr=rplWriteBINT(k,DECBINT,ptr);
                }
            } else {
                for(k=start;k>=end;k+=step) {
                    ptr=rplWriteBINT(k,DECBINT,ptr);
                }
            }

            rplOverwriteData(3,newlist);
            rplDropData(2);
            return;
        }

        // USE REAL NUMBERS, WE ARE DEALING WITH LARGE INTEGERS
        REAL start,end,step;
        BINT direction;
        WORDPTR newlist=rplAllocTempOb(2),ptr;
        if(!newlist) return;
        ptr=newlist+1;

        rplReadNumberAsReal(argstart,&start);
        if(Exceptions) return;
        rplReadNumberAsReal(argend,&end);
        if(Exceptions) return;
        rplReadNumberAsReal(argstep,&step);
        if(Exceptions) return;

        subReal(&RReg[0],&end,&start);

        if(RReg[0].flags&F_NEGATIVE) {
            if(iszeroReal(&step) || !(step.flags&F_NEGATIVE)) addReal(&RReg[0],&RReg[0],&RReg[0]); // STEP=(END-REAL)*2 SO ONLY ONE POINT WILL BE RETURNED
            else copyReal(&RReg[0],&step);
        }
        else {
            if(iszeroReal(&step) || (step.flags&F_NEGATIVE)) addReal(&RReg[0],&RReg[0],&RReg[0]); // STEP=(END-REAL)*2 SO ONLY ONE POINT WILL BE RETURNED
            else copyReal(&RReg[0],&step);
        }


        copyReal(&RReg[1],&start);
        copyReal(&RReg[2],&end);
        if(RReg[0].flags&F_NEGATIVE) direction=-1;
        else direction=1;
        do {
            // ADD THE CURRENT NUMBER TO THE LIST
            ScratchPointer1=newlist;
            ScratchPointer2=ptr;
            rplResizeLastObject(RReg[1].len+2);
            if(Exceptions) return;
            ptr=ScratchPointer2;
            newlist=ScratchPointer1;
            ptr=rplNewRealInPlace(&RReg[1],ptr);
            addReal(&RReg[1],&RReg[1],&RReg[0]);

        } while(cmpReal(&RReg[1],&RReg[2])!=direction);

        newlist[0]=MKPROLOG(DOCASELIST,ptr-newlist);
        newlist[ptr-newlist]=CMD_ENDLIST;


       rplOverwriteData(3,newlist);
       rplDropData(2);
       return;


    }

    case ASSUME:
        //@SHORT_DESC=Apply certain assumptions about a variable to an expression.
        //@INCOMPAT
    {
        if(rplDepthData()<2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        WORDPTR firstexp=rplPeekData(1);
        WORDPTR endexp=rplSkipOb(firstexp);
        WORDPTR *stksave=DSTop;


        if(ISLIST(*rplPeekData(2))) {
           rplListBinaryDoCmd();
            return;
        }

        if(ISMATRIX(rplPeekData(2))) {
            // TODO:
            //rplMatrixDoMat(2);
            return;
        }

        if(ISLIST(*firstexp)) {
            firstexp++;
            endexp--;   // END BEFORE THE CMD_ENDLIST WORD
        }

        if(!ISSYMBOLIC(*rplPeekData(2)) && !ISIDENT(*rplPeekData(2))) {
               rplError(ERR_SYMBOLICEXPECTED);
               return;
        }
        rplPushDataNoGrow(rplPeekData(2));

        // START APPLYING ALL ASSUMPTIONS
        while(firstexp<endexp) {

            if(ISSYMBOLIC(*firstexp)) {
                // COULD BE A SYMBOLIC LIST OR A LIST OF ARGUMENTS IN BRACKETS
                WORD opcode=rplSymbMainOperator(firstexp);
                if(opcode==CMD_OPENBRACKET) {
                    // USE ALL THE INDIVIDUAL ITEMS IN THE LIST
                    firstexp=rplSymbMainOperatorPTR(firstexp)+1;
                    continue;
                }

                firstexp=rplSymbUnwrap(firstexp);   // REMOVE ALL LAYERS OF SYMBOLICS
                if(!opcode) ++firstexp;

            }

            if(!ISIDENT(*firstexp)) {
                rplError(ERR_IDENTEXPECTED);
                DSTop=stksave;
                return;
            }

            // REPLACE ALL OCCURRENCES OF THE IDENT IN THE SYMBOLIC
            ScratchPointer4=firstexp;
            ScratchPointer5=endexp;
            WORDPTR newobj=rplSymbReplaceVar(rplPeekData(1),firstexp,firstexp);
            if(Exceptions || !newobj) {
                DSTop=stksave;
                return;
            }
            firstexp=ScratchPointer4;
            endexp=ScratchPointer5;

            rplOverwriteData(1,newobj);
            firstexp=rplSkipOb(firstexp);


        }   // END WHILE

        // HERE WE HAVE ALL VARIABLES SUBSTITUTED

        rplOverwriteData(3,rplPeekData(1));
        rplDropData(2);
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
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,CLOSEBRACKET)); // INDICATE THE OPENING BRACKET TO MATCH
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

        if( (TokenLen==2) && (*tok=='c') && (tok[1]=='{')) {
            if(CurrentConstruct==MKPROLOG(DOSYMB,0)) {
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,CLISTOPENBRACKET));
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
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,LISTCLOSEBRACKET));
                RetNum=OK_CONTINUE;
            }
            else RetNum=ERR_NOTMINE;
            return;
        }



/*
        if( (TokenLen==2) && !utf8ncmp2((char *)tok,(char *)BlankStart,":→",2)) {
            if(CurrentConstruct==MKPROLOG(DOSYMB,0)) {
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,RULESEPARATOR));
                RetNum=OK_CONTINUE;
            }
            else RetNum=ERR_NOTMINE;
        return;
        }
*/
        /*
        if(*tok=='|') {
            if((TokenLen==1)&&(CurrentConstruct==MKPROLOG(DOSYMB,0))) {
                // ISSUE A BUILDLIST OPERATOR
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,GIVENTHAT));
                RetNum=OK_CONTINUE;
            }
            else RetNum=ERR_NOTMINE;
            return;
        }
        */


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
