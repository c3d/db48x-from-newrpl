/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// THIS LIBRARY PROVIDES COMPILATION OF DIRECTORY PSEUDO-OBJECTS AND RELATED COMMANDS


// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  28
#define LIB_ENUM lib28enum
#define LIB_NAMES lib28_names
#define LIB_HANDLER lib28_handler
#define LIB_NUMBEROFCMDS LIB28_NUMBEROFCMDS

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(STO), \
    CMD(RCL), \
    CMD(INCR), \
    CMD(DECR), \
    CMD(PURGE), \
    CMD(CRDIR), \
    CMD(PGDIR), \
    CMD(UPDIR), \
    CMD(HOME), \
    CMD(PATH)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
/*
#define CMD_EXTRANAME \
    "→"
#define CMD_EXTRAENUM \
    NEWLOCALENV
*/

// THESE ARE SPECIAL OPCODES FOR THE COMPILER ONLY
// THE LOWER 16 BITS ARE THE NUMBER OF LAMS TO CREATE, OR THE INDEX OF LAM NUMBER TO STO/RCL
#define NEWNLOCALS 0x40000   // SPECIAL OPCODE TO CREATE NEW LOCAL VARIABLES
#define GETLAMN    0x20000   // SPECIAL OPCODE TO RCL THE CONTENT OF A LAM
#define PUTLAMN    0x10000   // SPECIAL OPCODE TO STO THE CONTENT OF A LAM

// INTERNAL DECLARATIONS

// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST /*, CMD_EXTRAENUM*/ , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
const char * const LIB_NAMES[]= { CMD_LIST /*, CMD_EXTRANAME*/  };
#undef CMD


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        // THIS SHOULD NEVER HAPPEN, AS DIRECTORY OBJECTS ARE SPECIAL HANDLES
        // THEY ARE NEVER USED IN THE MIDDLE OF THE CODE
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case STO:
    {
        // STORE CONTENT INSIDE A LAM OR GLOBAL VARIABLE, CREATE A NEW "GLOBAL" VARIABLE IF NEEDED
        if(rplDepthData()<2) {
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS

        if(!ISIDENT(*rplPeekData(1))) {
            Exceptions=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        WORDPTR *val=rplFindLAM(rplPeekData(1),1);



        if(val) {
            val[1]=rplPeekData(2);
            rplDropData(2);
        }
        else {
            // LAM WAS NOT FOUND, TRY A GLOBAL
            val=rplFindGlobal(rplPeekData(1),0);

            // HANDLE SPECIAL CASE OF STORING DIRECTORY OBJECTS
            WORDPTR obj=rplPeekData(2);
            if(LIBNUM(*obj)==DODIR) {
                WORDPTR *sourcedir=rplFindDirbyHandle(obj);
                if(sourcedir) {
                    WORDPTR *newdir=rplDeepCopyDir(sourcedir);
                    if(newdir) {
                        if(val) {
                            *(newdir+3)=*(rplGetDirfromGlobal(val)+1);   // SET PARENT DIR
                            *(val+1)=*(newdir+1);                   // AND NEW HANDLE
                        }
                        else {
                            *(newdir+3)=*(CurrentDir+1);
                            rplCreateGlobal(rplPeekData(1),*(newdir+1));
                        }
                        rplDropData(2);
                        return;
                    }
                }
            }



            if(val) {
                val[1]=rplPeekData(2);
            }
            else {
                // CREATE A NEW GLOBAL VARIABLE
                rplCreateGlobal(rplPeekData(1),rplPeekData(2));
            }
            rplDropData(2);
        }
    }
    return;


    case RCL:
    {
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE
        if(rplDepthData()<1) {
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            Exceptions=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;

        }

        WORDPTR val=rplGetLAM(rplPeekData(1));
        if(val) {
            rplOverwriteData(1,val);
        }
        else {
            // NO LAM, TRY A GLOBAL
            val=rplGetGlobal(rplPeekData(1));
            if(val) {
                rplOverwriteData(1,val);
            }
            else {
            Exceptions=EX_VARUNDEF;
            ExceptionPointer=IPtr;
            return;
            }
        }
    }
        return;

    case INCR:
        {
        if(rplDepthData()<1) {
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            Exceptions=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;

        }

        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        WORDPTR *var=rplFindLAM(rplPeekData(1),1);
        if(!var) var=rplFindGlobal(rplPeekData(1),1);
        if(var) {
            rplOverwriteData(1,*(var+1));
            rplPushData((WORDPTR)one_bint);       // PUSH THE NUMBER ONE

            // CALL THE OVERLOADED OPERATOR '+'

            rplCallOvrOperator(OVR_ADD);

            if(Exceptions) return;


            *(var+1)=rplPeekData(1);      // STORE THE INCREMENTED COUNTER
        }
        else {
            Exceptions=EX_VARUNDEF;
            ExceptionPointer=IPtr;
            return;
            }

        }
        return;


    case DECR:
    {
    if(rplDepthData()<1) {
        Exceptions=EX_BADARGCOUNT;
        ExceptionPointer=IPtr;
        return;
    }
    // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

    if(!ISIDENT(*rplPeekData(1))) {
        Exceptions=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
        return;

    }

    // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

    WORDPTR *var=rplFindLAM(rplPeekData(1),1);
    if(!var) var=rplFindGlobal(rplPeekData(1),1);
    if(var) {
        rplOverwriteData(1,*(var+1));
        rplPushData((WORDPTR)one_bint);       // PUSH THE NUMBER ONE

        // CALL THE OVERLOADED OPERATOR '+'

        rplCallOvrOperator(OVR_SUB);

        if(Exceptions) return;


        *(var+1)=rplPeekData(1);      // STORE THE INCREMENTED COUNTER
    }
    else {
        Exceptions=EX_VARUNDEF;
        ExceptionPointer=IPtr;
        return;
        }

    }
    return;



    case PURGE:
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE
        if(rplDepthData()<1) {
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            Exceptions=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;

        }

        // TODO: ALSO ACCEPT A LIST OF VARS, WHEN WE HAVE LISTS!

        rplPurgeGlobal(rplPeekData(1));
        if(!Exceptions) rplDropData(1);
        return;

    case CRDIR:
    {
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE
        if(rplDepthData()<1) {
            Exceptions=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            Exceptions=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;

        }

        rplCreateNewDir(rplPeekData(1),CurrentDir);

        rplDropData(1);

     }
    return;
    case PGDIR:
    {
        // SAME AS PURGE BUT RECURSIVELY DELETE EVERYTHING IN NON-EMPTY DIRECTORY
    }
    return;
    case UPDIR:
    {
        WORDPTR *dir=rplGetParentDir(CurrentDir);
        if(dir) CurrentDir=dir;
        return;
    }
    return;
    case HOME:
        CurrentDir=Directories;
        return;
    case PATH:

        // TODO: DO THIS LATER

        return;



    // ADD MORE OPCODES HERE

    case OVR_EVAL:
    case OVR_EVAL1:
    case OVR_XEQ:
    // EVALUATING THE OBJECT HAS TO CHANGE THE CURRENT DIRECTORY INTO THIS ONE
    {
        WORDPTR *dir=rplFindDirbyHandle(rplPeekData(1));

        if(!dir) {
            Exceptions|=EX_UNDEFINED;
            ExceptionPointer=IPtr;
            return; //  LEAVE THE OBJECT UNEVALUATED. IT'S AN ORPHAN DIRECTORY OBJECT???
        }
        CurrentDir=dir;
        rplDropData(1);
        return;
    }
        return;
    case OVR_NUM:
        // DO NOTHING
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
            rplDecompAppendString2((BYTEPTR)"DIRObject",9);

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
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;


}







