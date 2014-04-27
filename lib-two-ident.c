#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// THIS LIBRARY PROVIDES ONLY COMPILATION OF IDENTS AFTER ALL OTHER LIBRARIES
// HAD A CHANCE TO IDENTIFY THEIR COMMANDS
// ANY LAM COMMANDS HAVE TO BE IN A SEPARATE LIBRARY



// THERE'S ONLY ONE EXTERNAL FUNCTION: THE LIBRARY HANDLER
// ALL OTHER FUNCTIONS ARE LOCAL

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  2
#define LIB_ENUM lib2enum
#define LIB_NAMES lib2_names
#define LIB_HANDLER lib2_handler
#define LIB_NUMBEROFCMDS LIB2_NUMBEROFCMDS

// THESE ARE SPECIAL OPCODES FOR THE COMPILER ONLY
// THE LOWER 16 BITS ARE THE NUMBER OF LAMS TO CREATE, OR THE INDEX OF LAM NUMBER TO STO/RCL
#define NEWNLOCALS 0x40000   // SPECIAL OPCODE TO CREATE NEW LOCAL VARIABLES
#define GETLAMN    0x20000   // SPECIAL OPCODE TO RCL THE CONTENT OF A LAM
#define PUTLAMN    0x10000   // SPECIAL OPCODE TO STO THE CONTENT OF A LAM

#define NEWLOCALENV 0

void rplCompileIDENT(BINT libnum,BYTEPTR tok,BINT len)
{
    // WE HAVE A VALID QUOTED IDENT, CREATE THE OBJECT
    BINT lenwords=(len+3)>>2;

    ScratchPointer1=(WORDPTR)tok;
    rplCompileAppend(MKPROLOG(libnum,lenwords));
    WORD nextword;
    tok=(BYTEPTR )ScratchPointer1;
    while(len>3) {
        // WARNING: THIS IS LITTLE ENDIAN ONLY!
        nextword=tok[0]+(tok[1]<<8)+(tok[2]<<16)+(tok[3]<<24);
        ScratchPointer1=(WORDPTR)tok;
        rplCompileAppend(nextword);
        tok=(BYTEPTR )ScratchPointer1;
        tok+=4;
        len-=4;
    }
    if(len) {
    nextword=0;
    BINT rot=0;
    while(len) {
        // WARNING: THIS IS LITTLE ENDIAN ONLY!
        nextword|=(*tok)<<rot;
        --len;
        ++tok;
        rot+=8;
    }
    rplCompileAppend(nextword);
    }
    // DONE

}

// THESE ARE THE ONLY CHARACTERS THAT ARE FORBIDDEN IN AN IDENTIFIER
const char const forbiddenChars[]="+-*/\\{}[]()#!^;:<>=, \"\'";


BINT rplIsValidIdent(BYTEPTR tok,BINT len)
{
    BYTEPTR ptr;
    // IDENT CANNOT START WITH A NUMBER
    if( (((char)*tok)>='0') && (((char)*tok)<='9')) return 0;

    // OR CONTAIN ANY OF THE FORBIDDEN CHARACTERS
    while(len)
    {
        ptr=(BYTEPTR )forbiddenChars;
        do {
        if(*tok==*ptr) return 0;
        ++ptr;
        } while(*ptr);
        ++tok;
        --len;
    }
    return 1;
}

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

       // TODO: COMPILE IDENTS QUOTED AND UNQUOTED, AND IF CurrentConstruct== NEWLOCALENV THEN ADD QUOTES
        {
        BYTEPTR tok=(BYTEPTR )TokenStart;
        BINT len=TokenLen;

        if(*tok=='\'') {
                // QUOTED IDENT OR ALGEBRAIC OBJECT
                if(tok[TokenLen-1]!='\'') {
                // NO CLOSING QUOTE!
                RetNum=ERR_NOTMINE;
                return;
                }

                ++tok;
                len-=2;

                if(!rplIsValidIdent(tok,len)) {
                 RetNum=ERR_NOTMINE;
                 return;
                }


                rplCompileIDENT(DOIDENT,tok,len);

                RetNum=OK_CONTINUE;
                return;
        }



        // UNQUOTED IDENTS


        if(!rplIsValidIdent(tok,len)) {
         RetNum=ERR_SYNTAX;
         return;
        }

        if(CurrentConstruct==MKOPCODE(4080,NEWLOCALENV)) {
            // INSIDE THIS CONSTRUCT WE NEED TO QUOTE ALL
            // IDENTS

            rplCompileIDENT(DOIDENT,tok,len);

            RetNum=OK_CONTINUE;
            return;
        }


        // CHECK IF IT'S A LAM, COMPILE TO A GETLAM OPCODE IF IT IS

        WORDPTR *LAMptr=rplFindLAMbyName(tok,len);


        if(LAMptr<LAMTopSaved) {
            // THIS IS NOT A VALID LAM, COMPILE AS AN UNQUOTED IDENT

            rplCompileIDENT(DOIDENTEVAL,tok,len);

            RetNum=OK_CONTINUE;
            return;
        }

        if(LAMptr<nLAMBase) {
            // THIS IS A LAM FROM AN UPPER CONSTRUCT
            // WE CAN USE GETLAM ONLY INSIDE LOOPS, NEVER ACROSS SECONDARIES

            WORDPTR *env=nLAMBase;
            WORD prolog;
            do {
                if(LAMptr>env) break;
                prolog=**(env+1);   // GET THE PROLOG OF THE SECONDARY
                if(ISPROLOG(prolog) && LIBNUM(prolog)==SECO) {
                // LAMS ACROSS << >> SECONDARIES HAVE TO BE COMPILED AS IDENTS
                rplCompileIDENT(DOIDENTEVAL,tok,len);
                RetNum=OK_CONTINUE;
                return;
                }
                env=rplGetNextLAMEnv(env);
            } while(env);



        }


        // SPECIAL CASE: WHEN A SECO DOESN'T HAVE ANY LOCALS YET
        // BUT LAMS FROM THE PREVIOUS SECO SHOULDN'T BE COMPILED TO GETLAMS

        // SCAN ALL CURRENT CONSTRUCTS TO FIND THE INNERMOST SECONDARY
        // THEN VERIFY IF THAT SECONDARY IS THE CURRENT LAM ENVIRONMENT

        // THIS IS TO FORCE ALL LAMS IN A SECO TO BE COMPILED AS IDENTS
        // INSTEAD OF GETLAMS

        // DOCOL'S ARE OK AND ALWAYS COMPILED AS GETLAMS
        WORDPTR *scanenv=ValidateTop-1;

        while(scanenv>=RSTop) {
            if( (LIBNUM(**scanenv)==SECO)&& (ISPROLOG(**scanenv))) {
                    // FOUND INNERMOST SECONDARY
                    if(*scanenv>*(nLAMBase+1)) {
                        // THE CURRENT LAM BASE IS OUTSIDE THE INNER SECONDARY
                    rplCompileIDENT(DOIDENTEVAL,tok,len);

                    RetNum=OK_CONTINUE;
                    return;
                    }
                    break;

            }
            --scanenv;
        }

        // IT'S A KNOWN LOCAL VARIABLE, COMPILE AS GETLAM

        BINT Offset=((BINT)(LAMptr-nLAMBase))>>1;
        rplCompileAppend(MKOPCODE(DOIDENT,GETLAMN+(Offset&0xffff)));

        RetNum=OK_CONTINUE;
        return;





        }


     return;

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors


        // NOTHING TO DECOMPILE HERE. THIS LIBRARY DOES NOT DEFINE ANY COMMANDS OR OBJECTS.

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





