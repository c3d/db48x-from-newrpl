#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  4080
#define LIB_ENUM lib4080enum
#define LIB_NAMES lib4080_names
#define LIB_HANDLER lib4080_handler
#define LIB_NUMBEROFCMDS LIB4080_NUMBEROFCMDS

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
#define CMD_EXTRANAME \
    "->"
#define CMD_EXTRAENUM \
    NEWLOCALENV


// INTERNAL DECLARATIONS

// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM {  CMD_EXTRAENUM , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
char *LIB_NAMES[]= {  CMD_EXTRANAME  };
#undef CMD


#define NEWNLOCALS 0x40000   // SPECIAL OPCODE TO CREATE NEW LOCAL VARIABLES

// INTERNAL RPL PROGRAM THAT CALLS ABND
extern WORD abnd_prog[];
// INTERNAL SINT OBJECTS
extern WORD lam_baseseco_bint[];
// INTERNAL SINT OBJECTS
extern WORD lam_errhandler_bint[];





void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
        return;
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

        // CHECK IF THE TOKEN IS THE OBJECT DOCOL
        // BUT ONLY IF WE ARE WITHIN A NEWLOCALENV CONSTRUCT

       if((TokenLen==2) && (!strncmp((char *)TokenStart,"<<",2)))
       {
           if(CurrentConstruct!=MKOPCODE(LIBRARY_NUMBER,NEWLOCALENV)) {
               RetNum=ERR_NOTMINE;
               return;
           }


           // COUNT HOW MANY LAMS ARE IN THE CONSTRUCT
           ScratchPointer1=*(ValidateTop-1);
           BINT lamcount=0;

           // INITIALIZE AN ENVIRONMENT FOR COMPILE TIME
           nLAMBase=LAMTop;
           rplCreateLAM(lam_baseseco_bint,ScratchPointer1);
           ++ScratchPointer1;  // SKIP THE START OF CONSTRUCT WORD
           while(ScratchPointer1<CompileEnd) {
               rplCreateLAM(ScratchPointer1,ScratchPointer1);   // CREATE ALL THE LAMS FOR FUTURE COMPILATION
               ScratchPointer1=rplSkipOb(ScratchPointer1);
               ++lamcount;
           }
           // NOW REPLACE THE -> WORD FOR A STANDARD <<

           ScratchPointer1=*(ValidateTop-1);
           *ScratchPointer1=MKPROLOG(SECO,0);  // STANDARD SECONDARY PROLOG SO ALL LAMS ARE CREATED INSIDE OF IT
           CurrentConstruct=MKPROLOG(SECO,0);
           rplCompileAppend((WORD) MKOPCODE(DOIDENT,NEWNLOCALS+lamcount));   // OPCODE TO CREATE ALL THESE LAMS
           RetNum=OK_CONTINUE;
           return;
       }

       // CHECK IF THE TOKEN IS THE NEW LOCAL

       if((TokenLen==2) && (!strncmp((char *)TokenStart,"->",2)))
       {
           rplCompileAppend(CMD_EVALSECO);   // EVAL THE NEXT SECO IN THE RUNSTREAM
           rplCompileAppend(MKOPCODE(LIBRARY_NUMBER,NEWLOCALENV));  // PUT A MARKER
           RetNum=OK_STARTCONSTRUCT;
           return;
       }


       RetNum=ERR_NOTMINE;
        return;

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        // THIS LIBRARY DOES NOT GENERATE ANY OPCODES!
        RetNum=ERR_INVALID;
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






