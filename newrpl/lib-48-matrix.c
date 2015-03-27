/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// LIBRARY 48 IMPLEMENTS THE MATRIX (AND VECTOR) OBJECTS

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  48
#define LIB_ENUM lib48_enum
#define LIB_NAMES lib48_names
#define LIB_HANDLER lib48_handler
#define LIB_NUMBEROFCMDS LIB48_NUMBEROFCMDS

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(IDN)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
#define CMD_EXTRANAME \
    "->ARRY", \
    "ARRY->"


#define CMD_EXTRAENUM \
    TOARRAY, \
    ARRAYDECOMP



// INTERNAL DECLARATIONS


// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_EXTRAENUM , CMD_LIST ,  LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
const char * const LIB_NAMES[]= { CMD_EXTRANAME , CMD_LIST  };
#undef CMD



#define MKSIZE(rows,cols) ( (((rows)&0xffff)<<16)|((cols)&0xffff) )
#define ROWS(size) ( ((size)>>16)&0xffff )
#define COLS(size) ( (size)&0xffff )




// COMPARE TWO ITEMS WITHIN AN ARRAY, BY CALLING THE OPERATOR CMP
// OPERATOR CMP MUST RETURN -1, 0 OR 1 IF B>A, B==A, OR A>B RESPECTIVELY

BINT rplArraytemCompare(WORDPTR a,WORDPTR b)
{

    rplPushData(a);
    rplPushData(b);
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_CMP));
    if(Exceptions) return 0;
    BINT r=rplReadBINT(rplPopData());
    if(r==0) return (BINT)(a-b);
    return r;

}

// PERFORM AN OPERATION BETWEEN 2 ITEMS, POP THE RESULT FROM THE STACK
// AND RETURN IT AS A POINTER TO THE OBJECT.
// KEEPS THE STACK CLEAN EVEN IF THERE ARE EXCEPTIONS
// USES STACK PROTECTION AND PERFORMS PROPER STACK CLEANUP

WORDPTR rplArrayItemBinaryOp(WORDPTR a,WORDPTR b, WORD Opcode)
{
    rplProtectData();
    rplPushData(a);
    rplPushData(b);
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OPCODE(Opcode)));
    if(Exceptions) {
        rplClearData();
        rplUnprotectData();
        return 0;
    }
    WORDPTR result=0;
    if(rplDepthData()>0)   result=rplPopData();
    rplClearData();
    rplUnprotectData();
    return result;
}

WORDPTR rplArrayItemUnaryOp(WORDPTR a, WORD Opcode)
{
    rplProtectData();
    rplPushData(a);
    rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OPCODE(Opcode)));
    if(Exceptions) {
        rplClearData();
        rplUnprotectData();
        return 0;
    }
    WORDPTR result=0;
    if(rplDepthData()>0)   result=rplPopData();
    rplClearData();
    rplUnprotectData();
    return result;
}


// OTHER ARRAY FUNCTIONS HERE





void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        rplPushData(IPtr);
        return;
    }


    if(ISUNARYOP(CurOpcode)) {
        // TODO: IMPLEMENT UNARY OPERATORS

    }

    if(ISBINARYOP(CurOpcode)) {

        // TODO: IMPLEMENT BINARY OPERATORS

    }

    switch(OPCODE(CurOpcode))
    {
    case TOARRAY:
        // TODO: BUILD AN ARRAY FROM ITS PIECES
    return;

    case ARRAYDECOMP:
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(!ISARRAY(*rplPeekData(1))) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }

        // TODO: IMPLEMENT BASIC MATRIX FUNCTIONS
        //rplExplodeMatrix(rplPopData());

        return;

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
                    rplCompileAppend(MKSIZE(1,0));
                }
                else {
                    // THERE SHOULD BE A SIZE WORD ALREADY
                    // INCREASE THE ROW COUNT
                    BINT rows=ROWS(matrix[1]),cols=COLS(matrix[1]);
                    matrix[1]=MKSIZE(rows+1,cols);
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

        if(((char * )TokenStart)[TokenLen-1]==']')
        {

            if(TokenLen>1) {
                BlankStart=NextTokenStart=(WORDPTR)(((char * )TokenStart)+TokenLen-1);
                RetNum=ERR_NOTMINE_SPLITTOKEN;
                return;
            }

            if(LIBNUM(CurrentConstruct)!=LIBRARY_NUMBER) {
                RetNum=ERR_SYNTAX;
                return;
            }
            WORDPTR matrix=*(ValidateTop-1);
            BINT rows=ROWS(matrix[1]),cols=COLS(matrix[1]);
            BINT totalelements=rows*cols;

            if(CurrentConstruct!=MKPROLOG(LIBRARY_NUMBER,0)) {
                // CLOSED AN INNER DIMENSION

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


            // TODO: STRETCH THE OBJECT, ADD THE INDEX AND REMOVE DUPLICATES
            WORDPTR endofobjects=rplCompileAppendWords(totalelements);
            if(Exceptions) return;

            // MAKE HOLE IN MEMORY
            memmovew(matrix+2+totalelements,matrix+2,endofobjects-(matrix+2));
            endofobjects+=totalelements;

            // NOW WRITE THE INDICES. ALL OFFSETS ARE RELATIVE TO MATRIX PROLOG!
            WORDPTR ptr=matrix+2,objptr=ptr+totalelements;
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

            // TODO: COMPACT MATRIX BY REMOVING DUPLICATED OBJECTS

            RetNum=OK_ENDCONSTRUCT;
            return;
        }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

        return;

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors


        if(ISPROLOG(*DecompileObject)) {
            rplDecompAppendString((BYTEPTR)"[ ");
            BINT rows=ROWS(*(DecompileObject+1)),cols=COLS(*(DecompileObject+1));

            // SCAN THE INDEX AND OUTPUT ALL OBJECTS INSIDE
            BINT i,j;

            for(i=0;i<rows;++i)
            {
                if(rows!=1) rplDecompAppendString("[ ");
                if(Exceptions) { RetNum=ERR_INVALID; return; }
                for(j=0;j<cols;++j)
                {
                    BINT offset=*(DecompileObject+2+i*cols+j);

                    rplDecompile(DecompileObject+offset,1);    // RUN EMBEDDED
                 if(Exceptions) { RetNum=ERR_INVALID; return; }
                 rplDecompAppendChar(' ');
                }
                if(rows!=1) rplDecompAppendString("] ");
                if(Exceptions) { RetNum=ERR_INVALID; return; }
            }

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
              || ISIDENT(*LastCompiledObject))) {
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

            matrix[1]=MKSIZE(1,1);

        }

        else {
            // IF THIS IS THE FIRST ROW, INCREASE THE COLUMN COUNT
            BINT rows=ROWS(matrix[1]),cols=COLS(matrix[1]);
            if(rows==1) { matrix[1]=MKSIZE(rows,cols+1); }

        }



        RetNum=OK_CONTINUE;
        return;
    }
    case OPCODE_LIBINSTALL:
        RetNum=(UBINT)libnumberlist;
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





