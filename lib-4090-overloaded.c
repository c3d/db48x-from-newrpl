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
#define LIB_NUMCMDS 22


// GET LIST OF OVELOADABLE OPERATORS AS DEFINED IN <libraries.h>
#define OP_LIST \
    OP(INV), \
    OP(NEG), \
    OP(EVAL), \
    OP(XEQ), \
    OP(ABS), \
    OP(ISTRUE), \
    OP(NOT), \
    OP(EQ), \
    OP(NOTEQ), \
    OP(LT), \
    OP(GT), \
    OP(LTE), \
    OP(GTE), \
    OP(SAME), \
    OP(AND), \
    OP(OR), \
    OP(XOR), \
    OP(ADD), \
    OP(SUB), \
    OP(MUL), \
    OP(DIV), \
    OP(POW)
    // ADD MORE OPERATORS HERE







#define OP(a) OVRT_ ## a
char *LIB_NAMES[]=
{
        OP_LIST
};
#undef OP

#define OP(a) OVR_ ## a
int LIB_OPCODES[]=
{
        OP_LIST
};
#undef OP


// PERFORM AN INTERNAL CALL TO AN OVERLOADED OPERATOR
// op IS ONE OF THE OVR_XXX OPERATOR CONSTANTS

void rplCallOvrOperator(WORD op)
{
    int nargs=OVR_GETNARGS(op);
    int libnum=0;
    if(nargs>rplDepthData()) {
        Exceptions=EX_BADARGCOUNT;
        ExceptionPointer=IPtr;
        return;
    }
    while(nargs) {
        // CHECK EACH ARGUMENT, BUT STORE IN A GC-SAFE POINTER
        ScratchPointer1=rplPeekData(nargs);
        if(LIBNUM(*ScratchPointer1)>(WORD)libnum) libnum=LIBNUM(*ScratchPointer1);
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

        libCompileCmds(LIBRARY_NUMBER,LIB_NAMES,LIB_OPCODES,LIB_NUMCMDS);
     return;

    case OPCODE_DECOMPILE:
        libDecompileCmds(LIB_NAMES,LIB_OPCODES,LIB_NUMCMDS);
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
    }
    }

    if(OPCODE(CurOpcode)<MIN_OVERLOAD_OPCODE) {
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
        return;
    }


    rplCallOvrOperator(OPCODE(CurOpcode));

}




