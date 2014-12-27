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
// LIB4090 PROVIDES OVERLOADABLE OPERATORS, IT COMPILES GENERIC VERSIONS OF OPERATORS AND DISPATCHES
// TO OTHER LIBRARIES FOR PROCESSING

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  LIB_OVERLOADABLE
#define LIB_HANDLER lib4090_handler
#define LIB_NAMES lib4090_names
#define LIB_OPCODES lib4090_opcodes
#define LIB_TOKENINFO lib4090_tokeninfo
#define LIB_NUMCMDS 23

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };

// GET LIST OF OVERLOADABLE OPERATORS AS DEFINED IN <libraries.h>
#define OP_LIST \
    OP(INV,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    OP(NEG,MKTOKENINFO(1,TITYPE_FUNCTION,1,4)), \
    OP(EVAL,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    OP(XEQ,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    OP(ABS,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    OP(ISTRUE,MKTOKENINFO(6,TITYPE_FUNCTION,1,2)), \
    OP(NOT,MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    OP(EQ,MKTOKENINFO(2,TITYPE_BINARYOP_LEFT,2,10)), \
    OP(NOTEQ,MKTOKENINFO(2,TITYPE_BINARYOP_LEFT,2,10)), \
    OP(LT,MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,9)), \
    OP(GT,MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,9)), \
    OP(LTE,MKTOKENINFO(2,TITYPE_BINARYOP_LEFT,2,9)), \
    OP(GTE,MKTOKENINFO(2,TITYPE_BINARYOP_LEFT,2,9)), \
    OP(SAME,MKTOKENINFO(1,TITYPE_FUNCTION,2,2)), \
    OP(AND,MKTOKENINFO(3,TITYPE_BINARYOP_LEFT,2,11)), \
    OP(OR,MKTOKENINFO(2,TITYPE_BINARYOP_LEFT,2,13)), \
    OP(XOR,MKTOKENINFO(3,TITYPE_BINARYOP_LEFT,2,12)), \
    OP(CMP,MKTOKENINFO(3,TITYPE_FUNCTION,2,9)), \
    OP(ADD,MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,8)), \
    OP(SUB,MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,7)), \
    OP(MUL,MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,6)), \
    OP(DIV,MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,2,5)), \
    OP(POW,MKTOKENINFO(1,TITYPE_BINARYOP_RIGHT,2,3))
    // ADD MORE OPERATORS HERE







#define OP(a,b) OVRT_ ## a
const char * const LIB_NAMES[]=
{
        OP_LIST
};
#undef OP

#define OP(a,b) OVR_ ## a
const WORD const LIB_OPCODES[]=
{
        OP_LIST
};
#undef OP
#define OP(a,b) b
const BINT const LIB_TOKENINFO[]=
{
        OP_LIST
};
#undef OP

// EXECUTE AN OPERATOR, NOT NECESSARILY AN OVERLOADED ONE
// NO ARGUMENT CHECK! (TO BE DONE BY OPERATOR BEING CALLED)

void rplCallOperator(WORD op)
{
    int libnum=LIBNUM(op);
    if(libnum!=LIBRARY_NUMBER) {
    LIBHANDLER han=rplGetLibHandler(libnum);
    if(han) {
        // EXECUTE THE OTHER LIBRARY DIRECTLY
        BINT SavedOpcode=CurOpcode;
        CurOpcode=op;
        (*han)();
        if(CurOpcode==op) CurOpcode=SavedOpcode;
    }
    else {
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
    }
    return;
    }
    rplCallOvrOperator(op);
}



// PERFORM AN INTERNAL CALL TO AN OVERLOADED OPERATOR
// op IS ONE OF THE OVR_XXX OPERATOR CONSTANTS

void rplCallOvrOperator(WORD op)
{
    int nargs=OVR_GETNARGS(op);
    int libnum=0;
    WORDPTR ptr;
    if(nargs>rplDepthData()) {
        Exceptions=EX_BADARGCOUNT;
        ExceptionPointer=IPtr;
        return;
    }
    while(nargs) {
        // CHECK EACH ARGUMENT
        ptr=rplPeekData(nargs);
        if(LIBNUM(*ptr)>(WORD)libnum) libnum=LIBNUM(*ptr);
        --nargs;
    }
    LIBHANDLER han=rplGetLibHandler(libnum);
    if(han) {
        // EXECUTE THE OTHER LIBRARY DIRECTLY
        BINT SavedOpcode=CurOpcode;
        CurOpcode=MKOPCODE(LIBRARY_NUMBER,op);
        (*han)();
        if(CurOpcode==MKOPCODE(LIBRARY_NUMBER,op)) CurOpcode=SavedOpcode;
    }
    else {
        // THE LIBRARY DOESN'T EXIST BUT THE OBJECT DOES?
        // THIS CAN ONLY HAPPEN IF TRYING TO EXECUTE WITH A CUSTOM OBJECT
        // WHOSE LIBRARY WAS UNINSTALLED AFTER BEING COMPILED (IT'S AN INVALID OBJECT)
        Exceptions=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
    }

}





void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE A TYPE, SO THIS IS AN INVALID OPCODE
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
        return;
    }

    if(OPCODE(CurOpcode)>=MIN_RESERVED_OPCODE) {


    switch(OPCODE(CurOpcode))
    {

    // STANDARIZED OPCODES:
    // --------------------
    // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES


    case OPCODE_COMPILE:

        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,(WORDPTR)LIB_OPCODES,LIB_NUMCMDS);
     return;

    case OPCODE_DECOMPILE:

        // MANUALLY DECOMPILE UNARY PLUS AND UNARY MINUS IN SYMBOLICS
        if(OPCODE(*DecompileObject)==OVR_UMINUS) { rplDecompAppendChar('-'); RetNum=OK_CONTINUE; return; }
        if(OPCODE(*DecompileObject)==OVR_UPLUS) { rplDecompAppendChar('+'); RetNum=OK_CONTINUE; return; }


        libDecompileCmds((char **)LIB_NAMES,(WORDPTR)LIB_OPCODES,LIB_NUMCMDS);
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
        libProbeCmds((char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMCMDS);
        return;
    case OPCODE_GETINFO:
        // MANUALLY RETURN INFO FOR UNARY PLUS AND MINUS
        if(OPCODE(*DecompileObject)==OVR_UMINUS) { RetNum=OK_TOKENINFO | MKTOKENINFO(1,TITYPE_PREFIXOP,1,4); return; }
        if(OPCODE(*DecompileObject)==OVR_UPLUS) { RetNum=OK_TOKENINFO | MKTOKENINFO(1,TITYPE_PREFIXOP,1,4); return; }


        libGetInfo(*DecompileObject,(char **)LIB_NAMES,(WORDPTR)LIB_OPCODES,(BINT *)LIB_TOKENINFO,LIB_NUMCMDS);
        return;

    case OPCODE_LIBINSTALL:
        RetNum=(UBINT)libnumberlist;
        return;
    case OPCODE_LIBREMOVE:
        return;


    }

    RetNum=ERR_NOTMINE;
    return;
    }

    if(OPCODE(CurOpcode)<MIN_OVERLOAD_OPCODE) {
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
        return;
    }


    rplCallOvrOperator(OPCODE(CurOpcode));

}




