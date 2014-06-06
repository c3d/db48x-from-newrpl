/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// THIS LIBRARY PROVIDES COMPILATION OF STACK RELATED COMMANDS AND OTHER BASIC COMMANDS


// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  24
#define LIB_ENUM lib24enum
#define LIB_NAMES lib24_names
#define LIB_HANDLER lib24_handler
#define LIB_NUMBEROFCMDS LIB24_NUMBEROFCMDS

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(GARBAGE), \
    CMD(CLEAR), \
    CMD(DROP), \
    CMD(DUP), \
    CMD(SWAP), \
    CMD(ROT), \
    CMD(UNROT), \
    CMD(PICK)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
/*
#define CMD_EXTRANAME \
    "->"
#define CMD_EXTRAENUM \
    NEWLOCALENV
*/


// INTERNAL DECLARATIONS

// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST /*, CMD_EXTRAENUM*/ , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
char *LIB_NAMES[]= { CMD_LIST /*, CMD_EXTRANAME*/  };
#undef CMD


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case GARBAGE:
        rplGCollect();
        return;
    case CLEAR:
        // ONLY CLEAR UP TO THE STACK PROTECTED AREA
        // DON'T THROW AN ERROR
        DSTop=DStkProtect;
        return;
    case DROP:
        rplDropData(1);
        return;
    case DUP:
        if(rplDepthData()<1) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        rplPushData(rplPeekData(1));
        return;
    case SWAP:
    {
        if(rplDepthData()<2) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        WORDPTR obj=rplPeekData(1);
        rplOverwriteData(1,rplPeekData(2));
        rplOverwriteData(2,obj);
        return;
    }
    case ROT:
    {
        if(rplDepthData()<3) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        WORDPTR obj1=rplPeekData(1);
        rplOverwriteData(1,rplPeekData(3));
        rplOverwriteData(3,rplPeekData(2));
        rplOverwriteData(2,obj1);
        return;
    }
    case UNROT:
    {

        if(rplDepthData()<3) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }
        WORDPTR obj1=rplPeekData(1);
        rplOverwriteData(1,rplPeekData(2));
        rplOverwriteData(2,rplPeekData(3));
        rplOverwriteData(3,obj1);
        return;
    }
    case PICK:

        if(rplDepthData()<1) {
         Exceptions|=EX_BADARGCOUNT;
         ExceptionPointer=IPtr;
         return;
        }

        BINT64 level=rplReadNumberAsBINT(rplPeekData(1));

        if( (level<1) || (rplDepthData()<1+level)) {
         Exceptions|=EX_BADARGVALUE;
         ExceptionPointer=IPtr;
         return;
        }

        rplOverwriteData(1,rplPeekData(1+level));

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
        // BlankStart =
        // BlanksLen = Opcode/WORD of object

        // VALIDATE RETURNS:
        // RetNum =  enum CompileErrors



        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;

}








