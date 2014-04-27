// LIBRARY 16 DEFINES THE STRING OBJECT
// AND ASSOCIATED FUNCTIONS


#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  16
#define LIB_ENUM lib16enum
#define LIB_NAMES lib16_names
#define LIB_HANDLER lib16_handler
#define LIB_NUMBEROFCMDS LIB16_NUMBEROFCMDS

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
char *LIB_NAMES[]= { CMD_LIST , CMD_EXTRANAME  };
#undef CMD


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

        return;

    case FROMSTR:
        // COMPILER FUNCTION, FOR STR-> AND ->OBJ COMMANDS


    return;


    // ADD MORE OPCODES HERE

    case OVR_ADD:
        // APPEND TWO STRINGS

    return;

    case OVR_EVAL:
        return;

    case OVR_SAME:
    case OVR_EQ:
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
                if(count==0) temp.word=0;
                temp.bytes[count]=*ptr;
                ++count;
                ++ptr;
                if(*ptr=='\"') {
                    // END OF STRING!
                    ++ptr;
                    if(ptr!=(BYTEPTR)BlankStart) {
                        // QUOTE IN THE MIDDLE OF THE TOKEN IS A SYNTAX ERROR
                        RetNum=ERR_INVALID;
                        return;
                    }
                    // WE HAVE REACHED THE END OF THE STRING

                    rplCompileAppend(temp.word);
                    *ScratchPointer4=MKPROLOG(DOSTRING+4-count,(WORD)(CompileEnd-ScratchPointer4)-1);
                    RetNum=OK_CONTINUE;
                    return;
                }
                if(ptr==(BYTEPTR)NextTokenStart) {
                 // WE ARE AT THE END OF THE GIVEN STRING, STILL NO CLOSING QUOTE, SO WE NEED MORE

                    // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                    rplCompileAppend(temp.word);
                    *ScratchPointer4=MKPROLOG(DOSTRING+4-count,(WORD)(CompileEnd-ScratchPointer4));
                    RetNum=OK_NEEDMORE;
                    return;
                }

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
        libCompileCmds(LIBRARY_NUMBER,LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
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
            if(count==0) temp.word=0;
            temp.bytes[count]=*ptr;
            ++count;
            ++ptr;
            if(*ptr=='\"') {
                // END OF STRING!
                ++ptr;
                if(ptr!=(BYTEPTR)BlankStart) {
                    // QUOTE IN THE MIDDLE OF THE TOKEN IS A SYNTAX ERROR
                    RetNum=ERR_INVALID;
                    return;
                }
                // WE HAVE REACHED THE END OF THE STRING

                rplCompileAppend(temp.word);
                *ScratchPointer4=MKPROLOG(DOSTRING+4-count,(WORD)(CompileEnd-ScratchPointer4)-1);
                RetNum=OK_CONTINUE;
                return;
            }
            if(ptr==(BYTEPTR)NextTokenStart) {
             // WE ARE AT THE END OF THE GIVEN STRING, STILL NO CLOSING QUOTE, SO WE NEED MORE

                // CLOSE THE OBJECT, BUT WE'LL REOPEN IT LATER
                rplCompileAppend(temp.word);
                *ScratchPointer4=MKPROLOG(DOSTRING+4-count,(WORD)(CompileEnd-ScratchPointer4));
                RetNum=OK_NEEDMORE;
                return;
            }

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





