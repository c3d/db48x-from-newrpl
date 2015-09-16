/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
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
#define LIBRARY_NUMBER  48
#define LIB_ENUM lib480enum
#define LIB_NAMES lib48_names
#define LIB_HANDLER lib48_handler
#define LIB_TOKENINFO lib48_tokeninfo
#define LIB_NUMBEROFCMDS LIB48_NUMBEROFCMDS
#define ROMPTR_TABLE    romptr_table48

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };


// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(UNITDEF,MKTOKENINFO(6,TITYPE_NOTALLOWED,2,2)), \
    CMD(UNITPURGE,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(UVAL,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(UBASE,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(CONVERT,MKTOKENINFO(7,TITYPE_NOTALLOWED,2,2))
// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY

#define CMD_EXTRANAME \
    "", \
    "→UNIT"
#define CMD_EXTRAENUM \
    UNITOP, \
    TOUNIT

#define CMD_EXTRAINFO \
    MKTOKENINFO(1,TITYPE_BINARYOP_LEFT,1,2), \
    MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)


// INTERNAL DECLARATIONS

// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a,b) a
enum LIB_ENUM { CMD_LIST , CMD_EXTRAENUM , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a,b) #a
const char * const LIB_NAMES[]= { CMD_LIST , CMD_EXTRANAME };
#undef CMD

// AND INFORMATION FOR THE SYMBOLIC COMPILER
#define CMD(a,b) b
const BINT const LIB_TOKENINFO[]=
{
        CMD_LIST ,
        CMD_EXTRAINFO
};

// LIST OF ALL KNOWN SYSTEM UNIT OBJECTS
ROMOBJECT system_units[]={

    // 'm'
    MKPROLOG(DOIDENT,1),
    TEXT2WORD('m',0,0,0),
    0
};

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
     0
};


// RETURN A POINTER TO THE END OF THE NEXT UNIT IDENTIFIER
// SEPARATOR SYMBOLS ALLOWED ARE ( ) * / ^

BYTEPTR rplNextUnitToken(BYTEPTR start,BYTEPTR end)
{
    while(start<end) {
        if((*start=='*')||(*start=='/')||(*start=='^')||(*start=='(')||(*start==')')) break;
        start=(BYTEPTR)utf8skip((char *)start,(char *)end);
    }
    return start;
}











void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplPushData(IPtr);
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case UNITDEF:
    case UNITPURGE:
    case UVAL:
    case UBASE:
    case CONVERT:
    case TOUNIT:
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
    {

        BYTEPTR ptr=(BYTEPTR)TokenStart;

        if(*ptr=='_') {
        // STARTS WITH THE UNIT, CHECK IF WE ARE IN A UNIT CONSTRUCT

        if(CurrentConstruct==MKPROLOG(LIBRARY_NUMBER,0)) {
            // THE NUMBER WAS COMPILED PROPERLY, NOW ADD THE UNIT ITSELF

            // START COMPILING THE UNIT EXPRESSION

            // ONLY ALLOWS IDENTIFIERS, * AND / OPERATIONS BETWEEN THEM
            // ALSO ALLOWS ^ BUT ONLY WITH REAL EXPONENTS
            // PARENTHESIS ARE SUPPORTED BUT REMOVED AT COMPILE TIME s^2/(Kg*m) --> s^2*Kg^-1*m^-1
            // MAXIMUM 8 LEVELS SUPPORTED

            BYTEPTR nextptr;
            BINT expisreal=0;
            BINT count=0;
            BINT exponent=1,negexp=0,needident=0,needexp=0;
            BINT groupoff[8];
            BINT groupidx=0;

            nextptr=ptr+1;

            needident=1;

            while(count<(BINT)TokenLen-1) {
                if(needident) {

                // HANDLE THE SPECIAL CASE OF A PARENTHESIS

                if(*nextptr=='(') {
                    // SET THE EXPONENT FOR ALL IDENTS
                    if(negexp) {
                        exponent=-exponent;
                        negexp=0;
                    }
                    // OPEN A NEW GROUP
                    groupoff[groupidx]=CompileEnd-*(ValidateTop-1); // STORE THE OFFSET OF THE CURRENT OBJECT
                    ++groupidx;
                    if(groupidx>=8) {
                        // NO MORE THAN 8 NESTED LEVELS ALLOWED
                        RetNum=ERR_SYNTAX;
                        return;
                    }
                    ++nextptr;
                    ++count;
                    continue;
                }


                // GET THE NEXT IDENT
                BYTEPTR nameend=rplNextUnitToken(nextptr,(BYTEPTR)BlankStart);

                if(nameend<=nextptr) {
                    RetNum=ERR_SYNTAX;
                    return;
                }

                if(!rplIsValidIdent(nextptr,nameend)) {
                    RetNum=ERR_SYNTAX;
                    return;
                }

                BINT nletters=utf8nlen((char *)nextptr,(char *)nameend);
                // COMPILE THE IDENT

                rplCompileIDENT(DOIDENT,nextptr,nameend);
                if(Exceptions) {
                RetNum=ERR_INVALID;
                return;
                }


                // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+1,(char *)BlankStart,count)+(nameend-nextptr);

                count+=nletters;
                needexp=1;
                needident=0;

                }
                else {
                    // NOT LOOKING FOR AN IDENTIFIER
                if(*nextptr==')') {


                    if(*(nextptr+1)=='^') {
                        // TODO: HANDLE SPECIAL CASE OF A GROUP TO AN EXPONENT




                    }

                    // END OF A GROUP
                    if(!groupidx) {
                        RetNum=ERR_SYNTAX;
                        return;
                    }

                    --groupidx;
                    ++nextptr;
                    ++count;
                    continue;
                }

                if(*nextptr=='*') {
                    if(needexp) {
                        BINT finalexp=(negexp)? -exponent:exponent;
                        rplCompileAppend(MAKESINT(finalexp));
                        // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                        nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+1,(char *)BlankStart,count);
                        needexp=0;
                    }


                    // NOTHING TO DO ON MULTIPLICATION
                   needident=1;
                   ++nextptr;
                    ++count;
                    continue;
                }
                if(*nextptr=='/') {
                    if(needexp) {
                        BINT finalexp=(negexp)? -exponent:exponent;
                        rplCompileAppend(MAKESINT(finalexp));
                        // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                        nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+1,(char *)BlankStart,count);
                        needexp=0;
                    }

                    // NEGATE THE EXPONENT FOR THE NEXT IDENT
                    negexp^=1;
                    needident=1;
                    ++nextptr;
                    ++count;
                    continue;
                }

                if(*nextptr=='^') {
                    // ONLY A REAL NUMBER SUPPORTED AS EXPONENT
                    // GET THE NEXT TOKEN
                    nextptr++;
                    BYTEPTR numend=rplNextUnitToken(nextptr,(BYTEPTR)BlankStart);

                    if(numend<=nextptr) {
                        RetNum=ERR_SYNTAX;
                        return;
                    }

                    newRealFromText(&RReg[0],nextptr,numend,rplGetSystemLocale());

                    if(RReg[0].flags&(F_ERROR|F_INFINITY|F_NOTANUMBER|F_NEGUNDERFLOW|F_POSUNDERFLOW|F_OVERFLOW)) {
                        // BAD EXPONENT!
                        RetNum=ERR_SYNTAX;
                        return;
                    }


                    BINT nletters=utf8nlen((char *)nextptr,(char *)numend);


                    if(isintegerReal(&RReg[0]) && inBINT64Range(&RReg[0])) {
                        // EXPONENT IS AN INTEGER
                        BINT64 finalexp=getBINT64Real(&RReg[0]);
                        finalexp*=exponent;
                        if(negexp) finalexp=-finalexp;

                        // COMPILE AS A BINT OR A SINT
                        rplCompileBINT(finalexp,DECBINT);
                        if(Exceptions) {
                        RetNum=ERR_INVALID;
                        return;
                        }

                    }
                    else {
                        // EXPONENT WILL HAVE TO BE A REAL
                        BINT sign=(negexp)? -exponent:exponent;

                        if(sign<0) RReg[0].flags^=F_NEGATIVE;

                        rplCompileReal(&RReg[0]);
                        if(Exceptions) {
                        RetNum=ERR_INVALID;
                        return;
                        }


                    }


                    count+=1+nletters;
                    // RESTORE THE NEXT POINTER, WHICH MAY HAVE BEEN MOVED DUE TO GC
                    nextptr=(BYTEPTR)utf8nskip(((char *)TokenStart)+1,(char *)BlankStart,count);
                    needexp=0;
                    continue;

                }

                // AT THIS POINT ANYTHING ELSE IS A SYNTAX ERROR
                RetNum=ERR_SYNTAX;
                return;

                }
            }   // END WHILE

            if(needexp) {
                BINT finalexp=(negexp)? -exponent:exponent;
                rplCompileAppend(MAKESINT(finalexp));
            }

            // HERE WE SHOULD HAVE A UNIT OBJECT PROPERLY COMPILED!

            RetNum=OK_ENDCONSTRUCT;
            return;
        }
        // IF  WE ARE NOT IN A UNIT CONSTRUCT, IT'S SYNTAX ERROR
            RetNum=ERR_SYNTAX;
            return;
        }

        // DOESN'T START WITH '_'
        // FIRST LOOK FOR THE PRESENCE OF THE '_' SEPARATOR INSIDE THE TOKEN

        int f;

        for(f=0;f<(int)TokenLen;++f)
        {
            if(*ptr=='_') break;
            ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)BlankStart);
        }

        if(f==(int)TokenLen) {
            // NOT FOUND, THIS IS NOT A UNIT
            libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
            return;
        }

        // THERE IS A '_', NOW SPLIT THE TOKEN AND START A PROLOG OF A UNIT
        rplCompileAppend(MKPROLOG(LIBRARY_NUMBER,0));

        BlankStart=NextTokenStart=(WORDPTR)utf8nskip((char * )TokenStart,(char *)BlankStart,f);
        RetNum=OK_STARTCONSTRUCT_SPLITTOKEN;
        return;
    }


    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

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
        libProbeCmds((char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        return;
    case OPCODE_GETINFO:
        // MANUALLY RETURN INFO FOR THE UNIT OPERATOR
//        if(OPCODE(*DecompileObject)==OVR_UMINUS) { RetNum=OK_TOKENINFO | MKTOKENINFO(1,TITYPE_PREFIXOP,1,4); return; }
        libGetInfo2(*DecompileObject,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        return;


    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER,(WORDPTR *)ROMPTR_TABLE,ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID);
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) { RetNum=ERR_INVALID; return; }

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
    rplError(ERR_INVALIDOPCODE);

    return;


}









