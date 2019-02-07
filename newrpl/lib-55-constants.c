/*
 * Copyright (c) 2019, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"


// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************



// REPLACE THE NUMBER
#define LIBRARY_NUMBER  55

//@TITLE=Constants

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY


// SPECIAL CASE: LOWEST BIT OF CONSTANTS INDICATE IF CONSTANT IS COMPLEX OR REAL
// MAKE SURE ALL COMPLEX CONSTANTS ARE IN ODD NUMBERS IN THE LIST


#define COMMAND_LIST \
    ECMD(PICONST,"π",MKTOKENINFO(1,TITYPE_REAL,0,1)), \
    ECMD(ICONST,"і",MKTOKENINFO(1,TITYPE_COMPLEX,0,1)), \
    ECMD(ECONST,"е",MKTOKENINFO(1,TITYPE_REAL,0,1)), \
    ECMD(JCONST,"ј",MKTOKENINFO(1,TITYPE_COMPLEX,0,1))


// ADD MORE OPCODES HERE

#define ERROR_LIST \
ERR(CONSTANTEXPECTED,0), \
ERR(UNDEFINEDCONSTANT,1)

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS    LIBRARY_NUMBER

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************



INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib55_menu);


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)lib55_menu,
    0
};


// CONVERT A CONSTANT TO A NUMBER
// RETURNS EITHER A NEW OBJECT WITH THE NUMERIC REPRESENTATION OF THE CONSTANT
// OR THE SAME OBJECT AS BEFORE. MAY TRIGGER A GC BUT RETURNED POINTER IS SAFE FROM GC.

WORDPTR rplConstant2Number(WORDPTR object)
{
    if(!ISCONSTANT(*object)) return object;
    WORD saveopcode=CurOpcode;
    rplPushDataNoGrow(object);
    CurOpcode=object[1];   // GET THE OPCODE FOR THE SYMBOL
    LIB_HANDLER();
    CurOpcode=saveopcode;
    return rplPopData();
}

// PUT THE GIVEN CONSTANT INTO RReg[0] OR RReg[0] AND RReg[1] IF COMPLEX
// RETURNS 1 IF REAL, 1000+ANGLE_MODE IF COMPLEX


BINT rplConstant2NumberDirect(WORDPTR object)
{
    if(!ISCONSTANT(*object)) return 0;
    WORD saveopcode=CurOpcode;
    CurOpcode=object[1]|CONSTANT_DIRECT2NUMBER;   // GET THE OPCODE FOR THE SYMBOL
    LIB_HANDLER();
    CurOpcode=saveopcode;
    return (BINT)RetNum;
}

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        // NORMAL BEHAVIOR  ON A IDENT IS TO PUSH THE OBJECT ON THE STACK:
        rplPushData(IPtr);

        return;
    }


    // PROCESS OVERLOADED OPERATORS FIRST
    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE) {

        if(ISUNARYOP(CurOpcode))
        {
        // APPLY UNARY OPERATOR DIRECTLY TO THE CONTENTS OF THE VARIABLE
            switch(OPCODE(CurOpcode))
            {
            case OVR_EVAL1:
                // LEAVE IT ON THE STACK
                return;

            case OVR_FUNCEVAL:
            {
                    rplError(ERR_INVALIDUSERDEFINEDFUNCTION);
                    return;
             }
            case OVR_EVAL:
                // LEAVE IT ON THE STACK
                return;
            case OVR_NUM:
            {
                WORDPTR obj=rplPeekData(1);
                CurOpcode=obj[1];   // GET THE OPCODE FOR THE SYMBOL
                break;              // AND CONTINUE TO THE EXECUTION PART
            }

            case OVR_ISTRUE:
                    // CONSTANTS ARE NEVER FALSE
                    rplOverwriteData(1,(WORDPTR)one_bint);
                    return;

            case OVR_XEQ:
                // JUST KEEP THE CONSTANT ON THE STACK, UNEVALUATED
               return;

            default:
                // PASS AL OTHER OPERATORS DIRECTLY AS A SYMBOLIC OBJECT
            {
                    LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
                    (*symblib)();
                    return;
            }
        }


    }   // END OF UNARY OPERATORS

    else if(ISBINARYOP(CurOpcode)) {


        switch(OPCODE(CurOpcode))
        {
        case OVR_SAME:
        {
         BINT same=rplCompareObjects(rplPeekData(1),rplPeekData(2));
         rplDropData(2);
         if(same) rplPushTrue(); else rplPushFalse();
         return;
        }
        default:
        {
        // PASS AL OTHER OPERATORS DIRECTLY AS A SYMBOLIC OBJECT
            LIBHANDLER symblib=rplGetLibHandler(DOSYMB);
            (*symblib)();
            return;
        }
        }
    }


    }



    switch(OPCODE(CurOpcode))
    {

    case PICONST:
    {
        //@SHORT_DESC=Numeric constant π with twice the current system precision
        //@NEW

        if(rplDepthData()<1) { rplError(ERR_BADARGCOUNT); return; }
        REAL pi;

        decconst_PI(&pi);

        WORDPTR result=rplNewReal(&pi);
        if(result) rplOverwriteData(1,result);
        return;

    }
    case PICONST | CONSTANT_DIRECT2NUMBER:
    {
        // PUT THE CONSTANT DIRECTLY INTO RReg[0] (REAL) OR RReg[0] AND [1] FOR COMPLEX
        // RETURNS RetNum=1 IF REAL, 1000+ANGLE_MODE IF COMPLEX
        REAL pi;

        decconst_PI(&pi);
        copyReal(&RReg[0],&pi);
        normalize(&RReg[0]);
        return;
    }
    case ECONST:
    {
        //@SHORT_DESC=Numeric constant e at current system precision
        //@NEW

        if(rplDepthData()<1) { rplError(ERR_BADARGCOUNT); return; }

        rplOneToRReg(0);

        hyp_exp(&RReg[0]);

        normalize(&RReg[0]);

        WORDPTR result=rplNewReal(&RReg[0]);
        if(result) rplOverwriteData(1,result);
        return;

    }
    case ECONST | CONSTANT_DIRECT2NUMBER:
    {
        // PUT THE CONSTANT DIRECTLY INTO RReg[0] (REAL) OR RReg[0] AND [1] FOR COMPLEX
        // RETURNS RetNum=1 IF REAL, 1000+ANGLE_MODE IF COMPLEX
        rplOneToRReg(0);

        hyp_exp(&RReg[0]);

        normalize(&RReg[0]);
        return;
    }

    case ICONST:
    case JCONST:
    {
        //@SHORT_DESC=Imaginary constant i = j = (0,1)
        //@NEW

        if(rplDepthData()<1) { rplError(ERR_BADARGCOUNT); return; }

        rplOneToRReg(0);
        rplZeroToRReg(1);

        WORDPTR result=rplNewComplex(&RReg[1],&RReg[0],ANGLENONE);
        if(result) rplOverwriteData(1,result);
        return;

    }

    case ICONST | CONSTANT_DIRECT2NUMBER:
    case JCONST | CONSTANT_DIRECT2NUMBER:
    {
        // PUT THE CONSTANT DIRECTLY INTO RReg[0] (REAL) OR RReg[0] AND [1] FOR COMPLEX
        // RETURNS RetNum=1 IF REAL, 1000+ANGLE_MODE IF COMPLEX
        rplOneToRReg(0);
        rplZeroToRReg(1);
        return;
    }


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
        if(RetNum==OK_CONTINUE) {
            // ENCAPSULATE THE OPCODE INSIDE AN OBJECT
            rplCompileAppend(MKOPCODE(DECBINT,OPCODE(*(CompileEnd-1))));
            if(CompileEnd[-2]&1) {
                // THIS IS A COMPLEX CONSTANT, JUST REPEAT THE OPCODE
                rplCompileAppend(*(CompileEnd-1));
                CompileEnd[-3]=MKPROLOG(LIBRARY_NUMBER,2);
            } else  CompileEnd[-2]=MKPROLOG(LIBRARY_NUMBER,1);
        }
     return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors



        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        if(ISPROLOG(*DecompileObject)) {
            ++DecompileObject;
            libDecompileCmds((char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
            --DecompileObject;
        } else libDecompileCmds((char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

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
        {
        libProbeCmds((char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);

        return;
        }


    case OPCODE_GETINFO:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL COMMAND OR OBJECT IN ObjectPTR
        // NEEDS TO RETURN INFORMATION ABOUT THE TYPE:
        // IN RetNum: RETURN THE MKTOKENINFO() DATA FOR THE SYMBOLIC COMPILER AND CAS
        // IN DecompHints: RETURN SOME HINTS FOR THE DECOMPILER TO DO CODE BEAUTIFICATION (TO BE DETERMINED)
        // IN TypeInfo: RETURN TYPE INFORMATION FOR THE TYPE COMMAND
        //             TypeInfo: TTTTFF WHERE TTTT = MAIN TYPE * 100 (NORMALLY THE MAIN LIBRARY NUMBER)
        //                                FF = 2 DECIMAL DIGITS FOR THE SUBTYPE OR FLAGS (VARIES DEPENDING ON LIBRARY)
        //             THE TYPE COMMAND WILL RETURN A REAL NUMBER TypeInfo/100
        // FOR NUMBERS: TYPE=10 (REALS), SUBTYPES = .01 = APPROX., .02 = INTEGER, .03 = APPROX. INTEGER
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL BINT, .42 = HEX INTEGER
        if(ISPROLOG(*ObjectPTR)) {
        TypeInfo=LIBRARY_NUMBER*100;
        DecompHints=0;
        libGetInfo2(ObjectPTR[1],(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        if(TI_TYPE(RetNum)==TITYPE_REAL) TypeInfo+=DOREAL;
        else if(TI_TYPE(RetNum)==TITYPE_COMPLEX) TypeInfo+=DOCMPLX;
        else if(TI_TYPE(RetNum)==TITYPE_MATRIX) TypeInfo+=DOMATRIX;
        }
        else {
            TypeInfo=0;     // ALL COMMANDS ARE TYPE 0
            DecompHints=0;
            libGetInfo2(*ObjectPTR,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        }

        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, ObjectIDHash=hash, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER,(WORDPTR *)ROMPTR_TABLE,ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID, ObjectIDHash=hash
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID,ObjectIDHash);
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) {

            if(OBJSIZE(*ObjectPTR)!=1) { RetNum=ERR_INVALID; return; }  // WRONG SIZE

        }
        RetNum=OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        return;

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(MENUNUMBER(MenuCodeArg)>0) {
            RetNum=ERR_NOTMINE;
            return;
        }
        ObjectPTR=(WORDPTR)lib55_menu;
        RetNum=OK_CONTINUE;
       return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp,(WORDPTR)LIB_HELPTABLE);
       return;
    }
    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {

        libFindMsg(LibError,(WORDPTR)LIB_MSGTABLE);
       return;
    }

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

#endif




