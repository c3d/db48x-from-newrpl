/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// LIBRARY 16 DEFINES THE STRING OBJECT
// AND ASSOCIATED FUNCTIONS


#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  24
#define LIB_ENUM lib24enum
#define LIB_NAMES lib24_names
#define LIB_HANDLER lib24_handler
#define LIB_NUMBEROFCMDS LIB24_NUMBEROFCMDS

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,LIBRARY_NUMBER+1,LIBRARY_NUMBER+2,LIBRARY_NUMBER+3,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(TOUPPER), \
    CMD(TOLOWER), \
    CMD(LEN), \
    CMD(LEFT),   \
    CMD(MID), \
    CMD(RIGHT), \
    CMD(SUBSTR)

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
#define CMD_EXTRANAME \
    "->STR", \
    "STR->"
#define CMD_EXTRAENUM \
    TOSTR, \
    FROMSTR



// INTERNAL DECLARATIONS



// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST , CMD_EXTRAENUM , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
const char * const LIB_NAMES[]= { CMD_LIST , CMD_EXTRANAME  };
#undef CMD




// GET THE LENGTH OF A STRING FROM ITS PROLOG
#define STRLEN(prolog) ((OBJSIZE(prolog)<<2)-(LIBNUM(prolog)&3))


const WORD const empty_string[]={
    MKPROLOG(DOSTRING,0)
};



BINT rplStrLen(WORDPTR string)
{
    if(ISSTRING(*string))  return STRLEN(*string);
    return 0;
}


// FIX THE PROLOG OF A STRING TO MATCH THE DESIRED LENGTH IN CHARACTERS
// LOW-LEVEL FUNCTION, DOES NOT ACTUALLY RESIZE THE OBJECT
void rplSetStringLength(WORDPTR string,BINT length)
{
    BINT padding=(4-((length)&3))&3;

    *string=MKPROLOG(DOSTRING+padding,(length+3)>>2);
}


// ADDITIONAL API TO WORK WITH STRINGS FROM OTHER LIBRARIES

// RETURN AN OFFSET TO THE START OF THE REQUESTED LINE
// IF LINE<1 OR LINE>NUMBER OF LINES IN THE STRING, RETURNS -1
//
BINT rplStringGetLinePtr(WORDPTR str,BINT line)
{
    if(!ISSTRING(*str)) return -1;
    BYTEPTR start=(BYTEPTR) (str+1),ptr;
    BINT len=STRLEN(*str);
    BINT count=1;

    ptr=start;
    while(count<line) {
        while((ptr-start<len) && (*ptr!='\n')) ++ptr;
        if(ptr-start>=len) return -1;
        ++count;
        ++ptr;

    }

    if(ptr-start>=len) return -1;   // THIS CAN ONLY HAPPEN ON STRINGS TERMINATED IN A NEWLINE WHEN THE NEXT LINE IS REQUESTED
    return ptr-start;
}


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        // NORMAL BEHAVIOR IS TO PUSH THE OBJECT ON THE STACK:

        rplPushData(IPtr);
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case TOUPPER:
    case TOLOWER:
    case LEN:
    case LEFT:
    case MID:
    case RIGHT:
    case SUBSTR:
        // TODO: IMPLEMENT ALL THESE!
        return;

    case TOSTR:
        // VERY IMPORTANT: DECOMPILE FUNCTION
    {
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }

     WORDPTR string=rplDecompile(rplPeekData(1),0);
     if(!string) { ExceptionPointer=IPtr; return; }   // THERE WAS AN ERROR, TAKE OWNERSHIP OF IT
     rplOverwriteData(1,string);
    }
        return;

    case FROMSTR:
        // COMPILER FUNCTION, FOR STR-> AND ->OBJ COMMANDS
    {
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        WORDPTR string=rplPeekData(1);
        if(!ISSTRING(*string)) {
            Exceptions|=EX_BADARGTYPE;
            ExceptionPointer=IPtr;
            return;
        }
        BINT length=STRLEN(*string);
        WORDPTR newobj=rplCompile((BYTEPTR)(string+1),length,1);

        if(!newobj) { ExceptionPointer=IPtr; return; }   // THERE WAS AN ERROR, TAKE OWNERSHIP OF IT

        rplDropData(1);
        rplPushRet(IPtr);   // PUSH RETURN ADDRESS

        IPtr=newobj;
        CurOpcode=0;        // TRANSFER CONTROL TO THE NEW SECONDARY
        return;
    }


    // ADD MORE OPCODES HERE

    case OVR_ADD:
        // APPEND TWO STRINGS
    {
        if(rplDepthData()<2) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        ScratchPointer1=rplPeekData(2);
        ScratchPointer2=rplPeekData(1);

        if(!ISSTRING(*ScratchPointer1)) {
        WORDPTR string=rplDecompile(ScratchPointer1,0);
        if(!string) { ExceptionPointer=IPtr; return; }   // THERE WAS AN ERROR, TAKE OWNERSHIP OF IT
        ScratchPointer1=string;
        }

        if(!ISSTRING(*ScratchPointer2)) {
        WORDPTR string=rplDecompile(ScratchPointer2,0);
        if(!string) { ExceptionPointer=IPtr; return; }   // THERE WAS AN ERROR, TAKE OWNERSHIP OF IT
        ScratchPointer2=string;
        }

        BINT len1=STRLEN(*ScratchPointer1);
        BINT len2=STRLEN(*ScratchPointer2);

        WORDPTR newobject=rplAllocTempOb((len1+len2+3)>>2);
        if(!newobject) {
           Exceptions|=EX_OUTOFMEM;
           ExceptionPointer=IPtr;
           return;
        }
        // COPY BOTH STRINGS
        memmove(newobject+1,ScratchPointer1+1,len1);
        memmove( ((BYTEPTR)newobject)+len1+4,ScratchPointer2+1,len2);

        BINT padding=(4-((len1+len2)&3))&3;

        *newobject=MKPROLOG(DOSTRING+padding,(len1+len2+3)>>2);

        rplOverwriteData(2,newobject);
        rplDropData(1);

    }
    return;

    case OVR_EVAL:
    case OVR_EVAL1:
    case OVR_XEQ:
        // JUST LEAVE THE OBJECT ON THE STACK WHERE IT IS.
        return;

    case OVR_SAME:
    case OVR_EQ:

        if(rplDepthData()<2) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }

        if( (!ISSTRING(*rplPeekData(2))) || (!ISSTRING(*rplPeekData(1)))) {
            rplOverwriteData(2,(WORDPTR)zero_bint);
            rplDropData(1);
            return;
        }

        if(rplCompareObjects(rplPeekData(1),rplPeekData(2))) rplOverwriteData(2,(WORDPTR)one_bint);
        else rplOverwriteData(2,(WORDPTR)zero_bint);
        rplDropData(1);
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

        if(*((BYTEPTR)TokenStart)=='\"') {
            // START A STRING


            ScratchPointer4=CompileEnd;     // SAVE CURRENT COMPILER POINTER TO FIX THE OBJECT AT THE END

            rplCompileAppend(MKPROLOG(DOSTRING,0));

            union {
                WORD word;
                BYTE bytes[4];
            } temp;

            BINT count=0;
            BYTEPTR ptr=(BYTEPTR) TokenStart;
            ++ptr;  // SKIP THE QUOTE
            do {
            while(count<4) {
                if(ptr==(BYTEPTR)NextTokenStart) {
                 // WE ARE AT THE END OF THE GIVEN STRING, STILL NO CLOSING QUOTE, SO WE NEED MORE

                    // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                    if(count) rplCompileAppend(temp.word);
                    *ScratchPointer4=MKPROLOG(DOSTRING+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                    RetNum=OK_NEEDMORE;
                    return;
                }
                if(*ptr=='\"') {
                    // END OF STRING!
                    ++ptr;
                    if(ptr!=(BYTEPTR)BlankStart) {
                        // QUOTE IN THE MIDDLE OF THE TOKEN IS A SYNTAX ERROR
                        RetNum=ERR_INVALID;
                        return;
                    }
                    // WE HAVE REACHED THE END OF THE STRING
                    if(count) {
                    rplCompileAppend(temp.word);
                    }
                    *ScratchPointer4=MKPROLOG(DOSTRING+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                    RetNum=OK_CONTINUE;
                    return;
                }
                if(count==0) temp.word=0;
                temp.bytes[count]=*ptr;
                ++count;
                ++ptr;
                }
                //  WE HAVE A COMPLETE WORD HERE
                ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
                rplCompileAppend(temp.word);
                ptr=(BYTEPTR)ScratchPointer1;

                count=0;


            } while(1);     // DANGEROUS! BUT WE WILL RETURN FROM THE CHECK WITHIN THE INNER LOOP;
            //  THIS IS UNREACHABLE CODE HERE

        }








        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
     return;


    case OPCODE_COMPILECONT:
        // CONTINUE COMPILING STRING
    {
        union {
            WORD word;
            BYTE bytes[4];
        } temp;

        BINT count=(4-(LIBNUM(*ScratchPointer4)&3))&3; // GET NUMBER OF BYTES ALREADY WRITTEN IN LAST WORD
        if(count) {
            --CompileEnd;
            temp.word=*CompileEnd;  // GET LAST WORD
        }
        BYTEPTR ptr=(BYTEPTR) TokenStart;
        do {
        while(count<4) {
            if(ptr==(BYTEPTR)NextTokenStart) {
             // WE ARE AT THE END OF THE GIVEN STRING, STILL NO CLOSING QUOTE, SO WE NEED MORE

                // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                if(count) rplCompileAppend(temp.word);
                *ScratchPointer4=MKPROLOG(DOSTRING+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                RetNum=OK_NEEDMORE;
                return;
            }

            if(*ptr=='\"') {
                // END OF STRING!
                ++ptr;
                if(ptr!=(BYTEPTR)BlankStart) {
                    // QUOTE IN THE MIDDLE OF THE TOKEN IS A SYNTAX ERROR
                    RetNum=ERR_INVALID;
                    return;
                }
                // WE HAVE REACHED THE END OF THE STRING
                if(count) rplCompileAppend(temp.word);
                *ScratchPointer4=MKPROLOG(DOSTRING+((4-count)&3),(WORD)(CompileEnd-ScratchPointer4)-1);
                RetNum=OK_CONTINUE;
                return;
            }
            if(count==0) temp.word=0;
            temp.bytes[count]=*ptr;
            ++count;
            ++ptr;


            }
            //  WE HAVE A COMPLETE WORD HERE
            ScratchPointer1=(WORDPTR)ptr;           // SAVE AND RESTORE THE POINTER TO A GC-SAFE LOCATION
            rplCompileAppend(temp.word);
            ptr=(BYTEPTR)ScratchPointer1;

            count=0;


        } while(1);     // DANGEROUS! BUT WE WILL RETURN FROM THE CHECK WITHIN THE INNER LOOP;
        //  THIS IS UNREACHABLE CODE HERE

    }

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors
        if(ISPROLOG(*DecompileObject)) {
            rplDecompAppendChar('\"');
            rplDecompAppendString2((BYTEPTR)(DecompileObject+1),(OBJSIZE(*DecompileObject)<<2)-(LIBNUM(*DecompileObject)&3));
            rplDecompAppendChar('\"');

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





