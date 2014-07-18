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
    ","
#define CMD_EXTRAENUM \
    OPENBRACKET, \
    CLOSEBRACKET, \
    COMMA

// INTERNAL DECLARATIONS

// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST , CMD_EXTRAENUM , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
char *LIB_NAMES[]= { CMD_LIST , CMD_EXTRANAME  };
#undef CMD


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
        case OVR_XEQ:
        break;
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

        BINT size1=rplObjSize(arg1),size2=rplObjSize(arg2);

        WORDPTR newobject=rplAllocTempOb(size1+size2+1);
        if(!newobject) return;

        newobject[0]=MKPROLOG(LIBRARY_NUMBER,size1+size2+1);
        newobject[1]=CurOpcode;
        rplCopyObject(newobject+2,rplPeekData(2));
        rplCopyObject(newobject+2+size1,rplPeekData(1));

        rplDropData(1);
        rplOverwriteData(1,newobject);

        return;
        }
        break;
        }


    }

    switch(OPCODE(CurOpcode))
    {

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








