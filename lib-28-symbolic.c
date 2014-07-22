/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// THIS LIBRARY PROVIDES COMPILATION OF OUTER-LEVEL SYMBOLIC OBJECTS


// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  28
#define LIB_ENUM lib28enum
#define LIB_NAMES lib28_names
#define LIB_HANDLER lib28_handler
#define LIB_NUMBEROFCMDS LIB28_NUMBEROFCMDS

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(SYMBOLIC)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
#define CMD_EXTRANAME \
    "(", \
    ")", \
    ",", \
    "", \
    "", \
    ""

#define CMD_EXTRAENUM \
    OPENBRACKET, \
    CLOSEBRACKET, \
    COMMA, \
    EVALPRE, \
    EVALPOST, \
    EVALERR

// INTERNAL DECLARATIONS

// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST , CMD_EXTRAENUM , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
char *LIB_NAMES[]= { CMD_LIST , CMD_EXTRANAME  };
#undef CMD

extern WORD abnd_prog[];
extern WORD lam_baseseco_bint[];
extern WORD nulllam_ident[];


const WORD const symbeval_seco[]={
    MKPROLOG(DOCOL,5),
    MKOPCODE(LIBRARY_NUMBER,EVALPRE),     // PREPARE FOR CUSTOM PROGRAM EVAL
    MKOPCODE(LIB_OVERLOADABLE,OVR_EVAL),    // DO THE EVAL
    MKOPCODE(LIBRARY_NUMBER,EVALPOST),    // POST-PROCESS RESULTS AND CLOSE THE LOOP
    MKOPCODE(LIBRARY_NUMBER,EVALERR),     // ERROR HANDLER
    CMD_SEMI
};














void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        rplPushData(IPtr);
        return;
    }

    if((OPCODE(CurOpcode)>=MIN_OVERLOAD_OPCODE)&&(OPCODE(CurOpcode)<MIN_RESERVED_OPCODE)) {
        // OVERLOADED OPERATORS
    switch(OPCODE(CurOpcode))
    {
        case OVR_INV:
        case OVR_NEG:
        case OVR_NOT:
    {
     // UNARY OPERATION ON A SYMBOLIC

     WORDPTR object=rplPeekData(1);
         if(rplSymbMainOperator(object)==CurOpcode) {
             // THIS SYMBOLIC ALREADY HAS THE OPERATOR, REMOVE IT!
             rplOverwriteData(1,rplSymbUnwrap(object)+2);
             return;
         }

     object=rplSymbUnwrap(object);
     if(!object) {
         Exceptions|=EX_BADARGVALUE;
         ExceptionPointer=IPtr;
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
        case OVR_EVAL:
        // EVAL NEEDS TO SCAN THE SYMBOLIC, EVAL EACH ARGUMENT SEPARATELY AND APPLY THE OPCODE.
    {
        WORDPTR object=rplPeekData(1);
        if(!ISSYMBOLIC(*object)) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        // HERE WE HAVE program = PROGRAM TO EXECUTE

        // CREATE A NEW LAM ENVIRONMENT FOR TEMPORARY STORAGE OF INDEX
        nLAMBase=LAMTop;    // POINT THE GETLAM BASE TO THE NEW ENVIRONMENT
        rplCreateLAM(lam_baseseco_bint,IPtr);  // PUT MARKER IN LAM STACK, SET EVAL AS THE OWNER

        object=rplSymbUnwrap(object);
        WORDPTR endobject=rplSkipOb(object);
        WORD Opcode=rplSymbMainOperator(object);

        rplCreateLAM(nulllam_ident,Opcode);     // LAM 1 = OPCODE
        if(Exceptions) { rplCleanupLAMs(0); return; }

        object++;
        if(Opcode) object++;

        rplCreateLAM(nulllam_ident,endobject);     // LAM 2 = END OF CURRENT OBJECT
        if(Exceptions) { rplCleanupLAMs(0); return; }

        rplCreateLAM(nulllam_ident,object);     // LAM 3 = NEXT OBJECT TO PROCESS
        if(Exceptions) { rplCleanupLAMs(0); return; }

        // HERE GETLAM1 = OPCODE, GETLAM 2 = END OF SYMBOLIC, GETLAM3 = OBJECT

        // THIS NEEDS TO BE DONE IN 3 STEPS:
        // EVAL WILL PREPARE THE LAMS FOR OPEN EXECUTION
        // EVALPRE WILL PUSH THE NEXT OBJECT IN THE STACK AND EVAL IT
        // EVALPOST WILL CHECK IF THE ARGUMENT WAS PROCESSED WITHOUT ERRORS,
        // AND CLOSE THE LOOP TO PROCESS MORE ARGUMENTS

        // THE INITIAL CODE FOR EVAL MUST TRANSFER FLOW CONTROL TO A
        // SECONDARY THAT CONTAINS :: EVALPRE EVAL EVALPOST ;
        // MAPPOST WILL CHANGE IP AGAIN TO BEGINNING OF THE SECO
        // IN ORDER TO KEEP THE LOOP RUNNING

        rplPushRet(IPtr);
        IPtr=(WORDPTR) symbeval_seco;
        CurOpcode=MKOPCODE(LIB_OVERLOADABLE,OVR_EVAL);   // SET TO AN ARBITRARY COMMAND, SO IT WILL SKIP THE PROLOG OF THE SECO

        rplProtectData();  // PROTECT THE PREVIOUS ELEMENTS IN THE STACK FROM BEING REMOVED BY A BAD EVALUATION

        return;
    }





        case OVR_XEQ:
        // JUST LEAVE IT ON THE STACK
        return;
        case OVR_ABS:
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
         Exceptions|=EX_BADARGVALUE;
         ExceptionPointer=IPtr;
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
        case OVR_EQ:
        case OVR_NOTEQ:
        case OVR_LT:
        case OVR_GT:
        case OVR_LTE:
        case OVR_GTE:
        case OVR_SAME:
        case OVR_AND:
        case OVR_OR:
        case OVR_XOR:
        case OVR_CMP:
        case OVR_ADD:
        case OVR_SUB:
        case OVR_MUL:
        case OVR_DIV:
        case OVR_POW:
        {
        // BINARY OPERATORS

        // FIRST, CHECK THAT ARGUMENTS ARE ACCEPTABLE FOR SYMBOLIC OPERATION
        WORDPTR arg1=rplPeekData(2);
        WORDPTR arg2=rplPeekData(1);
        if( (!rplIsAllowedInSymb(arg1)) || (!rplIsAllowedInSymb(arg2)))
        {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
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


    case EVALPRE:
    {
        // HERE GETLAM1 = OPCODE, GETLAM 2 = END OF SYMBOLIC, GETLAM3 = OBJECT


        WORDPTR nextobj=*rplGetLAMn(3);
        WORDPTR endoflist=*rplGetLAMn(2);

        if(nextobj==endoflist) {
            // THE LAST ARGUMENT WAS ALREADY PROCESSED, IF THERE IS AN OPERATOR WE NEED TO APPLY IT

            WORD Opcode=*rplGetLAMn(1);

            WORDPTR *prevDStk = rplUnprotectData();
            BINT newdepth=(BINT)(DSTop-prevDStk);


            if(Opcode) {
                //rplSymbApplyOperator(Opcode,newdepth);
                // APPLY THE OPERATOR BY CALLING THE OPERATOR ITSELF

                CurOpcode=Opcode;
                LIBHANDLER han=rplGetLibHandler(LIBNUM(Opcode));
                do {
                if(han) (*han)();
                if(Exceptions) {
                    DSTop=prevDStk; // REMOVE ALL JUNK FROM THE STACK
                    rplCleanupLAMs(0);
                    IPtr=rplPopRet();
                    CurOpcode=MKOPCODE(LIB_OVERLOADABLE,OVR_EVAL);
                    return;
                }
                } while((DSTop-prevDStk>1)&&((Opcode==MKOPCODE(LIB_OVERLOADABLE,OVR_ADD))||(Opcode==MKOPCODE(LIB_OVERLOADABLE,OVR_MUL))));  // CALL IN A LOOP TO PROCESS MULTIPLE ARGUMENTS FOR PLUS AND MUL
            }
            // HERE WE ARE SUPPOSED TO HAVE ONLY ONE ARGUMENT ON THE STACK AND THE ORIGINAL OBJECT
            rplOverwriteData(2,rplPeekData(1));
            rplDropData(1);
            // CLEANUP AND RETURN
            rplCleanupLAMs(0);
            IPtr=rplPopRet();
            CurOpcode=MKOPCODE(LIB_OVERLOADABLE,OVR_EVAL);
            return;

        }

        rplSetExceptionHandler(IPtr+3); // SET THE EXCEPTION HANDLER TO THE EVALERR WORD


        // PUSH THE NEXT OBJECT IN THE STACK
        rplPushData(nextobj);

        // AND EXECUTION WILL CONTINUE AT EVAL

        return;
    }

    case EVALPOST:
    {
        // HERE GETLAM1 = OPCODE, GETLAM 2 = END OF SYMBOLIC, GETLAM3 = OBJECT

        rplRemoveExceptionHandler();    // THERE WAS NO ERROR DURING EVALUATION

        rplPutLAMn(3,rplSkipOb(*rplGetLAMn(3)));    // MOVE TO THE NEXT OBJECT IN THE LIST


        IPtr=(WORDPTR) symbeval_seco;   // CONTINUE THE LOOP
        // CurOpcode IS RIGHT NOW A COMMAND, SO WE DON'T NEED TO CHANGE IT
        return;
    }
    case EVALERR:
        // JUST CLEANUP AND EXIT
        DSTop=rplUnprotectData();
        rplCleanupLAMs(0);
        IPtr=rplPopRet();
        Exceptions=TrappedExceptions;
        ExceptionPointer=IPtr;
        CurOpcode=MKOPCODE(LIB_OVERLOADABLE,OVR_EVAL);
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
        BYTEPTR tok=(BYTEPTR )TokenStart;



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
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,CLOSEBRACKET));
                RetNum=OK_CONTINUE;
            }
            else RetNum=ERR_NOTMINE;
        return;
        }
        if(*tok==',') {
            if((TokenLen==1) && (CurrentConstruct==MKPROLOG(DOSYMB,0))) {
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,COMMA));
                RetNum=OK_CONTINUE;
            }
            else RetNum=ERR_NOTMINE;
        return;
        }




    }

            // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
            // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER,LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;

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

        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds(LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
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

        if(*((char *)TokenStart)=='\'') {
            // FOUND END OF SYMBOLIC OBJECT

            if(TokenLen>1) RetNum=ERR_SYNTAX;
            else RetNum= OK_ENDCONSTRUCT_INFIX;
            return;
        }

        if(*((char *)TokenStart)=='(') {
            RetNum= OK_TOKENINFO | MKTOKENINFO(1,TITYPE_OPENBRACKET,0,31);
            return;
        }

        if(*((char *)TokenStart)==')') {
            RetNum= OK_TOKENINFO | MKTOKENINFO(1,TITYPE_CLOSEBRACKET,0,31);
            return;
        }

        if(*((char *)TokenStart)==',') {
            RetNum= OK_TOKENINFO | MKTOKENINFO(1,TITYPE_COMMA,0,31);
            return;
        }

        RetNum = ERR_NOTMINE;
        return;


        }

    case OPCODE_GETINFO:
        if(ISPROLOG(*DecompileObject)) {
            RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_EXPRESSION,0,0);
        }
        else RetNum=ERR_NOTMINE;
        return;



    }
    // UNHANDLED OPCODE...

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode)>=MIN_RESERVED_OPCODE) {
        RetNum=ERR_NOTMINE;
        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;


}








