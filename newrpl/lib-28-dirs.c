/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "libraries.h"

#ifndef COMMANDS_ONLY_PASS
#include "cmdcodes.h"
#include "hal_api.h"
#include "newrpl.h"
#include "sysvars.h"
#endif

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************

// REPLACE THE NUMBER
#define LIBRARY_NUMBER  28

//@TITLE=Variables and directories

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(STO,MK_TOKEN_INFO(3,TITYPE_NOTALLOWED,2,2)), \
    CMD(RCL,MK_TOKEN_INFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STOADD,"STO+",MK_TOKEN_INFO(4,TITYPE_NOTALLOWED,2,2)), \
    ECMD(STOSUB,"STO-",MK_TOKEN_INFO(4,TITYPE_NOTALLOWED,2,2)), \
    ECMD(STOMUL,"STO*",MK_TOKEN_INFO(4,TITYPE_NOTALLOWED,2,2)), \
    ECMD(STODIV,"STO/",MK_TOKEN_INFO(4,TITYPE_NOTALLOWED,2,2)), \
    CMD(SINV,MK_TOKEN_INFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(SNEG,MK_TOKEN_INFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(SCONJ,MK_TOKEN_INFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(INCR,MK_TOKEN_INFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(DECR,MK_TOKEN_INFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(PURGE,MK_TOKEN_INFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(CRDIR,MK_TOKEN_INFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(PGDIR,MK_TOKEN_INFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(UPDIR,MK_TOKEN_INFO(5,TITYPE_NOTALLOWED,0,2)), \
    CMD(HOME,MK_TOKEN_INFO(4,TITYPE_NOTALLOWED,0,2)), \
    CMD(PATH,MK_TOKEN_INFO(4,TITYPE_NOTALLOWED,0,2)), \
    CMD(VARS,MK_TOKEN_INFO(4,TITYPE_NOTALLOWED,0,2)), \
    CMD(ALLVARS,MK_TOKEN_INFO(7,TITYPE_NOTALLOWED,0,2)), \
    CMD(ORDER,MK_TOKEN_INFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(QUOTEID,MK_TOKEN_INFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(UNQUOTEID,MK_TOKEN_INFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(HIDEVAR,MK_TOKEN_INFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(UNHIDEVAR,MK_TOKEN_INFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(CLVAR,MK_TOKEN_INFO(5,TITYPE_NOTALLOWED,0,2)), \
    CMD(LOCKVAR,MK_TOKEN_INFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(UNLOCKVAR,MK_TOKEN_INFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(RENAME,MK_TOKEN_INFO(6,TITYPE_NOTALLOWED,2,2)), \
    CMD(TVARS,MK_TOKEN_INFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(TVARSE,MK_TOKEN_INFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(SADD,MK_TOKEN_INFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(SPROP,MK_TOKEN_INFO(5,TITYPE_NOTALLOWED,2,2)), \
    CMD(RPROP,MK_TOKEN_INFO(5,TITYPE_NOTALLOWED,2,2)), \
    CMD(PACKDIR,MK_TOKEN_INFO(7,TITYPE_NOTALLOWED,2,2))

//    ECMD(CMDNAME,"CMDNAME",MK_TOKEN_INFO(7,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE
#define ERROR_LIST \
    ERR(NONEMPTYDIRECTORY,0), \
    ERR(DIRECTORYNOTFOUND,1), \
    ERR(CANTOVERWRITEDIR,2), \
    ERR(READONLYVARIABLE,3), \
    ERR(PROPERTYEXPECTED,4), \
    ERR(INVALIDPROPERTY,5), \
    ERR(UNDEFINEDPROPERTY,6), \
    ERR(LOCALSNOTALLOWED,7), \
    ERR(DIRECTORYEXPECTED,8)

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER, LIBRARY_NUMBER+1

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"

#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

// THESE ARE SPECIAL OPCODES FOR THE COMPILER ONLY
// THE LOWER 16 BITS ARE THE NUMBER OF LAMS TO CREATE, OR THE INDEX OF LAM NUMBER TO STO/RCL
#define NEWNLOCALS 0x40000      // SPECIAL OPCODE TO CREATE NEW LOCAL VARIABLES
#define GETLAMN    0x20000      // SPECIAL OPCODE TO RCL THE CONTENT OF A LAM
#define PUTLAMN    0x10000      // SPECIAL OPCODE TO STO THE CONTENT OF A LAM

// THIS IS USED AS A MARKER FOR VARIABLE TRACKING, BUT IT'S DEFINED IN THE LAM LIBRARY
extern ROMOBJECT lameval_seco[];
extern ROMOBJECT retrysemi_seco[];

ROMOBJECT dir_start_bint[] = {
    (WORD) DIR_START_MARKER
};

ROMOBJECT dir_end_bint[] = {
    (WORD) DIR_END_MARKER
};

ROMOBJECT dir_parent_bint[] = {
    (WORD) DIR_PARENT_MARKER
};

ROMOBJECT root_dir_handle[] = {
    (WORD) MK_PROLOG(DODIR, 1),
    (WORD) 0
};

ROMOBJECT home_opcode[] = {
    (WORD) MK_OPCODE(LIBRARY_NUMBER, HOME)
};

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib28_menu);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const word_p const ROMPTR_TABLE[] = {
    (word_p) LIB_MSGTABLE,
    (word_p) LIB_HELPTABLE,
    (word_p) lib28_menu,
    (word_p) dir_start_bint,
    (word_p) dir_parent_bint,
    (word_p) dir_end_bint,
    (word_p) root_dir_handle,
    (word_p) home_opcode,
    0
};

void LIB_HANDLER()
{
    if(IS_PROLOG(CurOpcode)) {
        // PROVIDE BEHAVIOR OF EXECUTING THE OBJECT HERE
        // THIS SHOULD NEVER HAPPEN, AS DIRECTORY OBJECTS ARE SPECIAL HANDLES
        // THEY ARE NEVER USED IN THE MIDDLE OF THE CODE
        if(!ISPACKEDDIR(CurOpcode))
            rplError(ERR_UNRECOGNIZEDOBJECT);
        else
            rplPushData(IPtr);
        return;
    }

    switch (OPCODE(CurOpcode)) {
    case STO:
    {
        //@SHORT_DESC=Store an object into a variable
        //@INCOMPAT
        // STORE CONTENT INSIDE A LAM OR GLOBAL VARIABLE, CREATE A NEW "GLOBAL" VARIABLE IF NEEDED
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        word_p *stksave = DSTop;

        if(rplDepthData() >= 5) {
            // CHECK IF THIS IS A RETRY DUE TO SYMBOLIC EVALUATION
            if(rplPeekData(3) == IPtr) {

                // REMOVE THE ERROR HANDLER IF THERE WERE NO ERRORS
                if(ErrorHandler == (word_p) retrysemi_seco) {
                    rplRemoveExceptionHandler();
                    rplDropRet(1);
                }
                // RETURN STACK HAS THE SAVED TOP OF STACK
                word_p *stkptr = (word_p *) rplPopRet();
                // ONLY RESTORE THE DATA STACK IF WITHIN LIMITS
                if((stkptr >= DStkBottom) && (stkptr < stksave))
                    stksave = stkptr;

                // THE EVALUATION IS COMPLETE, NOW STORE THE VARIABLE
                // THE STACK HAS: OBJECT , EXPRESSION, MARKER, IDENT, IDX_EXPRESSION_LIST
                rplPushData(rplPeekData(5));
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }

                rplCallOperator(CMD_PUT);
                if(Exceptions) {
                    rplBlameError(IPtr);
                    DSTop = stksave;
                    return;
                }
                // EVERYTHING WORKED OK
                DSTop = stksave;
                rplDropData(2);
                return;
            }
        }

        word_p *indir = 0;
        // LIST IS A PATH, ONLY ENABLE PARALLEL PROCESSING FOR LISTS OF LISTS
        if(ISLIST(*rplPeekData(1))) {
            int32_t elemcount = rplListLength(rplPeekData(1));
            word_p firstelem = rplPeekData(1) + 1;
            if((elemcount > 1) || !ISLIST(*firstelem)) {
                rplListBinaryNoResultDoCmd();
                return;
            }
            // LIST OF LIST, TREAT LIKE A PATH
            indir = rplFindDirFromPath(rplPeekData(1) + 1, 0);
            if(!indir) {
                rplError(ERR_DIRECTORYNOTFOUND);
                return;
            }

        }

        if(!indir) {
            if(ISSYMBOLIC(*rplPeekData(1))) {
                // ONLY ACCEPT A FUNCEVAL EXPRESSION TO DO 'PUT'
                WORD oper = rplSymbMainOperator(rplPeekData(1));

                if(oper != CMD_OVR_FUNCEVAL) {
                    rplError(ERR_IDENTEXPECTED);
                    return;
                }

                // GET THE NAME OF THE IDENT IN THE LAST ARGUMENT OF THE EXPRESSION
                rplPushDataNoGrow(IPtr);        // PUSH A MARKER FOR RETRY

                int32_t nargs = rplSymbExplodeOneLevel(rplPeekData(2));

                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }

                rplDropData(2);
                nargs -= 2;     // REMOVE THE OPERATOR AND COUNT FROM THE STACK
                if(nargs > 2) {
                    // MAKE A LIST OF ARGUMENTS
                    word_p newlist = rplCreateListN(nargs - 1, 2, 1);
                    if(!newlist) {
                        DSTop = stksave;
                        return;
                    }

                    rplPushData(newlist);
                }
                else {
                    // SWAP THE IDENT NAME AND ARGUMENT
                    word_p tmp = rplPeekData(1);
                    rplOverwriteData(1, rplPeekData(2));
                    rplOverwriteData(2, tmp);
                }

                // NOW PREPARE FRO A NON-ATOMIC EXECUTION OF ->NUM

                rplPushRet((word_p) stksave);
                rplPushRet(IPtr);
                rplSetExceptionHandler((word_p) retrysemi_seco);
                word_p *rstopsave = RSTop;
                rplPushRet(IPtr);
                rplCallOvrOperator(CMD_OVR_NUM);
                if(IPtr != rstopsave[0]) {
                    // THIS OPERATION WAS NOT ATOMIC, LET THE RPL ENGINE RUN UNTIL IT COMES BACK HERE
                    rstopsave[1] = (word_p) retrysemi_seco;    // REPLACE THE RETURN ADDRESS WITH A RETRY
                    return;
                }
                // OPERATION WAS ATOMIC, RESTORE AND CONTINUE
                RSTop = rstopsave;
                rplRemoveExceptionHandler();
                rplPopRet();
                // STORE THE VARIABLE

                rplPushData(rplPeekData(5));
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }

                rplCallOperator(CMD_PUT);
                if(Exceptions) {
                    rplBlameError(IPtr);
                    DSTop = stksave;
                    return;
                }
                // EVERYTHING WORKED OK
                DSTop = stksave;
                rplDropData(2);
                return;
            }

            if(!ISIDENT(*rplPeekData(1))) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }
        }
        else {
            if(!ISIDENT(*rplGetListElement(rplPeekData(1) + 1,
                            rplListLength(rplPeekData(1) + 1)))) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }

        }

        word_p *val;
        WORD valattr = 0;

        if(!indir)
            val = rplFindLAM(rplPeekData(1), 1);
        else
            val = 0;

        if(val) {
            if(ISDIR(*val[1])) {
                rplError(ERR_CANTOVERWRITEDIR);
                return;
            }
            if(ISLOCKEDIDENT(*val[0])) {
                rplError(ERR_READONLYVARIABLE);
                return;
            }
            if(ISPACKEDDIR(*rplPeekData(2))) {
                rplError(ERR_LOCALSNOTALLOWED);
                return;

            }
            val[1] = rplPeekData(2);
            rplDropData(2);
        }
        else {
            // LAM WAS NOT FOUND, TRY A GLOBAL
            if(indir) {
                val = rplFindGlobalInDir(rplGetListElement(rplPeekData(1) + 1,
                            rplListLength(rplPeekData(1) + 1)), indir, 0);
            }
            else
                val = rplFindGlobal(rplPeekData(1), 0);
            if(val) {
                if(ISDIR(*val[1])) {
                    rplError(ERR_CANTOVERWRITEDIR);
                    return;
                }
                if(ISLOCKEDIDENT(*val[0])) {
                    rplError(ERR_READONLYVARIABLE);
                    return;
                }
                valattr = rplGetIdentAttr(val[0]);

            }
            // HANDLE SPECIAL CASE OF STORING DIRECTORY OBJECTS
            word_p obj = rplPeekData(2);
            if(LIBNUM(*obj) == DODIR) {
                word_p *sourcedir = rplFindDirbyHandle(obj);
                if(sourcedir) {
                    word_p *newdir = rplDeepCopyDir(sourcedir);
                    if(newdir) {
                        if(val) {
                            *(newdir + 3) = *(rplGetDirfromGlobal(val) + 1);    // SET PARENT DIR
                            *(val + 1) = *(newdir + 1); // AND NEW HANDLE
                        }
                        else {
                            // NOT FOUND, CREATE A NEW VARIABLE
                            if(!indir) {
                                *(newdir + 3) = *(CurrentDir + 1);
                                word_p name =
                                        rplMakeIdentQuoted(rplPeekData(1));
                                if(!name)
                                    return;
                                rplCreateGlobal(name, *(newdir + 1));
                            }
                            else {
                                // SET PARENT DIR
                                *(newdir + 3) = *(indir + 1);
                                word_p name =
                                        rplMakeIdentQuoted(rplGetListElement
                                        (rplPeekData(1) + 1,
                                            rplListLength(rplPeekData(1) + 1)));
                                if(!name)
                                    return;
                                rplCreateGlobalInDir(name, *(newdir + 1),
                                        indir);
                            }
                        }
                        rplDropData(2);
                        return;
                    }
                    else
                        return;
                }
                else {
                    rplError(ERR_DIRECTORYNOTFOUND);
                    return;
                }
            }
            else if(LIBNUM(*obj) == DOPACKDIR) {
                word_p *recurseptr = 0;
                int32_t recurseoffset = 0;
                do {
                    if(recurseptr) {
                        while(recurseptr < DirsTop) {
                            if(ISPACKEDDIR(*recurseptr[1]))
                                break;
                            recurseptr += 2;
                        }
                        if(recurseptr >= DirsTop)
                            break;

                        // HERE WE HAVE TO EXTRACT ANOTHER EMBEDDED PACKED DIRECTORY
                        val = recurseptr;
                        recurseoffset = recurseptr[1] - rplPeekData(2);
                        recurseptr += 2;
                    }

                    word_p *newdir = rplMakeNewDir();
                    if(newdir) {
                        if(val) {
                            *(newdir + 3) = *(rplGetDirfromGlobal(val) + 1);    // SET PARENT DIR
                            *(val + 1) = *(newdir + 1); // AND NEW HANDLE
                        }
                        else {
                            // NOT FOUND, CREATE A NEW VARIABLE
                            if(!indir) {
                                *(newdir + 3) = *(CurrentDir + 1);
                                word_p name =
                                        rplMakeIdentQuoted(rplPeekData(1));
                                if(!name)
                                    return;
                                rplCreateGlobal(name, *(newdir + 1));
                                // newdir WAS JUST CREATED AT THE END OF DIRECTORIES, IT MUST'VE MOVED
                                newdir += 2;
                            }
                            else {
                                // SET PARENT DIR
                                *(newdir + 3) = *(indir + 1);
                                word_p name =
                                        rplMakeIdentQuoted(rplGetListElement
                                        (rplPeekData(1) + 1,
                                            rplListLength(rplPeekData(1) + 1)));
                                if(!name)
                                    return;
                                rplCreateGlobalInDir(name, *(newdir + 1),
                                        indir);
                                // newdir WAS JUST CREATED AT THE END OF DIRECTORIES, IT MUST'VE MOVED
                                newdir += 2;
                            }
                        }

                        // NOW STORE EVERY SINGLE VARIABLE
                        int32_t count;
                        obj = rplPeekData(2);   // READ AGAIN JUST IN CASE IT MOVED DUE TO GC
                        if(recurseptr)
                            obj += recurseoffset;

                        word_p name, value, endobj;

                        endobj = rplSkipOb(obj);

                        name = obj + 1;
                        count = 0;
                        while(name < endobj) {
                            value = rplSkipOb(name);
                            if(value >= endobj)
                                break;
                            ++count;
                            name = rplSkipOb(value);
                        }

                        // NOW WE HAVE A COUNT OF VARIABLES

                        word_p *direntries =
                                rplCreateNGlobalsInDir(count, newdir);

                        if(!direntries || Exceptions)
                            return;

                        // POPULATE THE NEW ENTRIES
                        obj = rplPeekData(2);   // READ AGAIN, MIGHT'VE MOVED
                        if(recurseptr)
                            obj += recurseoffset;
                        name = obj + 1;
                        int32_t offset;
                        while(count--) {
                            offset = name - obj;
                            ScratchPointer2 = obj;
                            name = rplMakeIdentQuoted(name);
                            if(!name)
                                return;
                            *direntries++ = name;
                            obj = ScratchPointer2;
                            name = ScratchPointer2 + offset;
                            value = rplSkipOb(name);
                            *direntries++ = value;
                            name = rplSkipOb(value);
                            if((recurseptr == 0) && ISPACKEDDIR(*value))
                                recurseptr = direntries - 2;
                        }

                    }
                    else
                        return;
                }
                while(recurseptr);
                rplDropData(2);
                return;

            }

            if(val) {
                val[1] = rplPeekData(2);
            }
            else {
                // CREATE A NEW GLOBAL VARIABLE
                if(!indir) {
                    word_p name = rplMakeIdentQuoted(rplPeekData(1));
                    if(!name)
                        return;
                    rplCreateGlobal(name, rplPeekData(2));
                }
                else {
                    word_p name =
                            rplMakeIdentQuoted(rplGetListElement(rplPeekData(1)
                                + 1, rplListLength(rplPeekData(1) + 1)));
                    if(!name)
                        return;
                    rplCreateGlobalInDir(name, rplPeekData(2), indir);
                }
            }
            rplDropData(2);
        }
        if(valattr & IDATTR_DEPEND)
            rplDoAutoEval(val[0], indir);
    }
        return;

    case RCL:
    {
        //@SHORT_DESC=Recall the contents of a variable
        //@INCOMPAT
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        word_p *indir = 0;
        // LIST IS A PATH, ONLY ENABLE PARALLEL PROCESSING FOR LISTS OF LISTS
        if(ISLIST(*rplPeekData(1))) {
            int32_t nelem = rplListLength(rplPeekData(1));
            word_p firstelem = rplPeekData(1) + 1;
            if((nelem > 1) || !ISLIST(*firstelem)) {
                rplListUnaryNonRecursiveDoCmd();
                return;
            }
            // NOT A LIST, SO IT MUST BE A PATH
            indir = rplFindDirFromPath(rplPeekData(1) + 1, 0);
            if(!indir) {
                rplError(ERR_DIRECTORYNOTFOUND);
                return;
            }

        }

        if(!indir) {
            if(!ISIDENT(*rplPeekData(1))) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }
        }
        else {
            if(!ISIDENT(*rplGetListElement(rplPeekData(1) + 1,
                            rplListLength(rplPeekData(1) + 1)))) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }

        }

        word_p val;

        if(!indir)
            val = rplGetLAM(rplPeekData(1));
        else
            val = 0;

        if(val) {
            rplOverwriteData(1, val);
        }
        else {
            if(!indir) {
                // NO LAM, TRY A GLOBAL
                val = rplGetGlobal(rplPeekData(1));
            }
            else {
                word_p *var =
                        rplFindGlobalInDir(rplGetListElement(rplPeekData(1) + 1,
                            rplListLength(rplPeekData(1) + 1)), indir, 0);
                if(var)
                    val = var[1];
                else
                    val = 0;
            }

            if(val) {
                rplOverwriteData(1, val);
            }
            else {
                rplError(ERR_UNDEFINEDVARIABLE);
                return;
            }
        }
    }
        return;

    case STOADD:
    {
        //@SHORT_DESC=Add to the content of a variable
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t varidx = 0, done_add = 0;

        if(rplDepthData() >= 5) {
            // CHECK IF THIS IS A RETRY
            if(rplPeekData(3) == IPtr) {
                // THE ADDITION IS COMPLETE, NOW STORE THE VARIABLE
                done_add = 1;
                varidx = 2;
            }
        }

        if(!varidx) {
            if(ISIDENT(*rplPeekData(1)))
                varidx = 1;
            else if(ISIDENT(*rplPeekData(2)))
                varidx = 2;
            if(!varidx) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }
        }
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *stksave = DSTop;

        word_p *var = rplFindLAM(rplPeekData(varidx), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(varidx), 1);
        if(var) {
            if(ISDIR(*var[1])) {
                rplError(ERR_CANTOVERWRITEDIR);
                return;
            }
            if(ISLOCKEDIDENT(*var[0])) {
                rplError(ERR_READONLYVARIABLE);
                return;
            }

            if(!done_add) {

                rplPushData(IPtr);
                rplPushData(rplPeekData(varidx + 1));
                if(Exceptions)
                    return;
                if(varidx == 2) {
                    rplPushData(*(var + 1));
                    rplPushData(rplPeekData((varidx ^ 3) + 3)); // PUSH THE OBJECT TO ADD
                }
                else {
                    rplPushData(rplPeekData((varidx ^ 3) + 2)); // PUSH THE OBJECT TO ADD
                    rplPushData(*(var + 1));
                }
                if(Exceptions)
                    return;
                word_p *rstopsave = RSTop;
                rplPushRet(IPtr);
                rplCallOvrOperator(CMD_OVR_ADD);
                if(IPtr != *rstopsave) {
                    // THIS OPERATION WAS NOT ATOMIC, LET THE RPL ENGINE RUN UNTIL IT COMES BACK HERE
                    rstopsave[1] = (word_p) retrysemi_seco;    // REPLACE THE RETURN ADDRESS WITH A RETRY
                    return;
                }
                RSTop = rstopsave;
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }

            }

            *(var + 1) = rplPeekData(1);        // STORE THE INCREMENTED COUNTER
            rplDropData(5);
            WORD valattr = rplGetIdentAttr(var[0]);
            if(valattr & IDATTR_DEPEND)
                rplDoAutoEval(var[0], rplGetDirfromGlobal(var));

        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }
        return;

    case STOSUB:

    {
        //@SHORT_DESC=Subtract from the contents of a variable
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t varidx = 0, done_add = 0;

        if(rplDepthData() >= 5) {
            // CHECK IF THIS IS A RETRY
            if(rplPeekData(3) == IPtr) {
                // THE ADDITION IS COMPLETE, NOW STORE THE VARIABLE
                done_add = 1;
                varidx = 2;
            }
        }

        if(!varidx) {
            if(ISIDENT(*rplPeekData(1)))
                varidx = 1;
            else if(ISIDENT(*rplPeekData(2)))
                varidx = 2;
            if(!varidx) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }
        }
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *stksave = DSTop;

        word_p *var = rplFindLAM(rplPeekData(varidx), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(varidx), 1);
        if(var) {
            if(ISDIR(*var[1])) {
                rplError(ERR_CANTOVERWRITEDIR);
                return;
            }
            if(ISLOCKEDIDENT(*var[0])) {
                rplError(ERR_READONLYVARIABLE);
                return;
            }

            if(!done_add) {

                rplPushData(IPtr);
                rplPushData(rplPeekData(varidx + 1));
                if(Exceptions)
                    return;
                if(varidx == 2) {
                    rplPushData(*(var + 1));
                    rplPushData(rplPeekData((varidx ^ 3) + 3)); // PUSH THE OBJECT TO ADD
                }
                else {
                    rplPushData(rplPeekData((varidx ^ 3) + 2)); // PUSH THE OBJECT TO ADD
                    rplPushData(*(var + 1));
                }
                if(Exceptions)
                    return;
                word_p *rstopsave = RSTop;
                rplPushRet(IPtr);
                rplCallOvrOperator(CMD_OVR_SUB);
                if(IPtr != *rstopsave) {
                    // THIS OPERATION WAS NOT ATOMIC, LET THE RPL ENGINE RUN UNTIL IT COMES BACK HERE
                    rstopsave[1] = (word_p) retrysemi_seco;    // REPLACE THE RETURN ADDRESS WITH A RETRY
                    return;
                }
                RSTop = rstopsave;
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }

            }

            *(var + 1) = rplPeekData(1);        // STORE THE INCREMENTED COUNTER
            rplDropData(5);
            WORD valattr = rplGetIdentAttr(var[0]);
            if(valattr & IDATTR_DEPEND)
                rplDoAutoEval(var[0], rplGetDirfromGlobal(var));


        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }
        return;

    case STOMUL:

    {
        //@SHORT_DESC=Multiply contents of a variable
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t varidx = 0, done_add = 0;

        if(rplDepthData() >= 5) {
            // CHECK IF THIS IS A RETRY
            if(rplPeekData(3) == IPtr) {
                // THE ADDITION IS COMPLETE, NOW STORE THE VARIABLE
                done_add = 1;
                varidx = 2;
            }
        }

        if(!varidx) {
            if(ISIDENT(*rplPeekData(1)))
                varidx = 1;
            else if(ISIDENT(*rplPeekData(2)))
                varidx = 2;
            if(!varidx) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }
        }
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *stksave = DSTop;

        word_p *var = rplFindLAM(rplPeekData(varidx), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(varidx), 1);
        if(var) {
            if(ISDIR(*var[1])) {
                rplError(ERR_CANTOVERWRITEDIR);
                return;
            }
            if(ISLOCKEDIDENT(*var[0])) {
                rplError(ERR_READONLYVARIABLE);
                return;
            }

            if(!done_add) {

                rplPushData(IPtr);
                rplPushData(rplPeekData(varidx + 1));
                if(Exceptions)
                    return;
                if(varidx == 2) {
                    rplPushData(*(var + 1));
                    rplPushData(rplPeekData((varidx ^ 3) + 3)); // PUSH THE OBJECT TO ADD
                }
                else {
                    rplPushData(rplPeekData((varidx ^ 3) + 2)); // PUSH THE OBJECT TO ADD
                    rplPushData(*(var + 1));
                }
                if(Exceptions)
                    return;
                word_p *rstopsave = RSTop;
                rplPushRet(IPtr);
                rplCallOvrOperator(CMD_OVR_MUL);
                if(IPtr != *rstopsave) {
                    // THIS OPERATION WAS NOT ATOMIC, LET THE RPL ENGINE RUN UNTIL IT COMES BACK HERE
                    rstopsave[1] = (word_p) retrysemi_seco;    // REPLACE THE RETURN ADDRESS WITH A RETRY
                    return;
                }
                RSTop = rstopsave;
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }

            }

            *(var + 1) = rplPeekData(1);        // STORE THE INCREMENTED COUNTER
            rplDropData(5);
            WORD valattr = rplGetIdentAttr(var[0]);
            if(valattr & IDATTR_DEPEND)
                rplDoAutoEval(var[0], rplGetDirfromGlobal(var));


        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }
        return;

    case STODIV:

    {
        //@SHORT_DESC=Divide the content of a variable
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t varidx = 0, done_add = 0;

        if(rplDepthData() >= 5) {
            // CHECK IF THIS IS A RETRY
            if(rplPeekData(3) == IPtr) {
                // THE ADDITION IS COMPLETE, NOW STORE THE VARIABLE
                done_add = 1;
                varidx = 2;
            }
        }

        if(!varidx) {
            if(ISIDENT(*rplPeekData(1)))
                varidx = 1;
            else if(ISIDENT(*rplPeekData(2)))
                varidx = 2;
            if(!varidx) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }
        }
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *stksave = DSTop;

        word_p *var = rplFindLAM(rplPeekData(varidx), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(varidx), 1);
        if(var) {
            if(ISDIR(*var[1])) {
                rplError(ERR_CANTOVERWRITEDIR);
                return;
            }
            if(ISLOCKEDIDENT(*var[0])) {
                rplError(ERR_READONLYVARIABLE);
                return;
            }

            if(!done_add) {

                rplPushData(IPtr);
                rplPushData(rplPeekData(varidx + 1));
                if(Exceptions)
                    return;
                if(varidx == 2) {
                    rplPushData(*(var + 1));
                    rplPushData(rplPeekData((varidx ^ 3) + 3)); // PUSH THE OBJECT TO ADD
                }
                else {
                    rplPushData(rplPeekData((varidx ^ 3) + 2)); // PUSH THE OBJECT TO ADD
                    rplPushData(*(var + 1));
                }
                if(Exceptions)
                    return;
                word_p *rstopsave = RSTop;
                rplPushRet(IPtr);
                rplCallOvrOperator(CMD_OVR_DIV);
                if(IPtr != *rstopsave) {
                    // THIS OPERATION WAS NOT ATOMIC, LET THE RPL ENGINE RUN UNTIL IT COMES BACK HERE
                    rstopsave[1] = (word_p) retrysemi_seco;    // REPLACE THE RETURN ADDRESS WITH A RETRY
                    return;
                }
                RSTop = rstopsave;
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }

            }

            *(var + 1) = rplPeekData(1);        // STORE THE INCREMENTED COUNTER
            rplDropData(5);
            WORD valattr = rplGetIdentAttr(var[0]);
            if(valattr & IDATTR_DEPEND)
                rplDoAutoEval(var[0], rplGetDirfromGlobal(var));


        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }
        return;

    case SINV:

    {
        //@SHORT_DESC=Invert the content of a variable
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t varidx = 0, done_add = 0;

        if(rplDepthData() >= 3) {
            // CHECK IF THIS IS A RETRY
            if(rplPeekData(2) == IPtr) {
                // THE ADDITION IS COMPLETE, NOW STORE THE VARIABLE
                done_add = 1;
                varidx = 3;
            }
        }

        if(!varidx) {
            if(ISIDENT(*rplPeekData(1)))
                varidx = 1;
            if(!varidx) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }
        }
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *stksave = DSTop;

        word_p *var = rplFindLAM(rplPeekData(varidx), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(varidx), 1);
        if(var) {
            if(ISDIR(*var[1])) {
                rplError(ERR_CANTOVERWRITEDIR);
                return;
            }
            if(ISLOCKEDIDENT(*var[0])) {
                rplError(ERR_READONLYVARIABLE);
                return;
            }

            if(!done_add) {

                rplPushData(IPtr);
                if(Exceptions)
                    return;
                rplPushData(*(var + 1));
                if(Exceptions)
                    return;
                word_p *rstopsave = RSTop;
                rplPushRet(IPtr);
                rplCallOvrOperator(CMD_OVR_INV);
                if(IPtr != *rstopsave) {
                    // THIS OPERATION WAS NOT ATOMIC, LET THE RPL ENGINE RUN UNTIL IT COMES BACK HERE
                    rstopsave[1] = (word_p) retrysemi_seco;    // REPLACE THE RETURN ADDRESS WITH A RETRY
                    return;
                }
                RSTop = rstopsave;
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }

            }

            *(var + 1) = rplPeekData(1);        // STORE THE RESULT
            rplDropData(3);
            WORD valattr = rplGetIdentAttr(var[0]);
            if(valattr & IDATTR_DEPEND)
                rplDoAutoEval(var[0], rplGetDirfromGlobal(var));


        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }
        return;

    case SNEG:
    {
        //@SHORT_DESC=Change sign (negate) the content of a variable
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t varidx = 0, done_add = 0;

        if(rplDepthData() >= 3) {
            // CHECK IF THIS IS A RETRY
            if(rplPeekData(2) == IPtr) {
                // THE ADDITION IS COMPLETE, NOW STORE THE VARIABLE
                done_add = 1;
                varidx = 3;
            }
        }

        if(!varidx) {
            if(ISIDENT(*rplPeekData(1)))
                varidx = 1;
            if(!varidx) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }
        }
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *stksave = DSTop;

        word_p *var = rplFindLAM(rplPeekData(varidx), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(varidx), 1);
        if(var) {
            if(ISDIR(*var[1])) {
                rplError(ERR_CANTOVERWRITEDIR);
                return;
            }
            if(ISLOCKEDIDENT(*var[0])) {
                rplError(ERR_READONLYVARIABLE);
                return;
            }

            if(!done_add) {

                rplPushData(IPtr);
                if(Exceptions)
                    return;
                rplPushData(*(var + 1));
                if(Exceptions)
                    return;
                word_p *rstopsave = RSTop;
                rplPushRet(IPtr);
                rplCallOvrOperator(CMD_OVR_NEG);
                if(IPtr != *rstopsave) {
                    // THIS OPERATION WAS NOT ATOMIC, LET THE RPL ENGINE RUN UNTIL IT COMES BACK HERE
                    rstopsave[1] = (word_p) retrysemi_seco;    // REPLACE THE RETURN ADDRESS WITH A RETRY
                    return;
                }
                RSTop = rstopsave;
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }

            }

            *(var + 1) = rplPeekData(1);        // STORE THE RESULT
            rplDropData(3);
            WORD valattr = rplGetIdentAttr(var[0]);
            if(valattr & IDATTR_DEPEND)
                rplDoAutoEval(var[0], rplGetDirfromGlobal(var));


        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }
        return;

    case SCONJ:
    {
        //@SHORT_DESC=Complex conjugate the contents of a variable
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t varidx = 0, done_add = 0;

        if(rplDepthData() >= 3) {
            // CHECK IF THIS IS A RETRY
            if(rplPeekData(2) == IPtr) {
                // THE ADDITION IS COMPLETE, NOW STORE THE VARIABLE
                done_add = 1;
                varidx = 3;
            }
        }

        if(!varidx) {
            if(ISIDENT(*rplPeekData(1)))
                varidx = 1;
            if(!varidx) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }
        }
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *stksave = DSTop;

        word_p *var = rplFindLAM(rplPeekData(varidx), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(varidx), 1);
        if(var) {
            if(ISDIR(*var[1])) {
                rplError(ERR_CANTOVERWRITEDIR);
                return;
            }
            if(ISLOCKEDIDENT(*var[0])) {
                rplError(ERR_READONLYVARIABLE);
                return;
            }

            if(!done_add) {

                rplPushData(IPtr);
                if(Exceptions)
                    return;
                rplPushData(*(var + 1));
                if(Exceptions)
                    return;
                word_p *rstopsave = RSTop;
                rplPushRet(IPtr);
                rplCallOperator(CMD_CONJ);
                if(IPtr != *rstopsave) {
                    // THIS OPERATION WAS NOT ATOMIC, LET THE RPL ENGINE RUN UNTIL IT COMES BACK HERE
                    rstopsave[1] = (word_p) retrysemi_seco;    // REPLACE THE RETURN ADDRESS WITH A RETRY
                    return;
                }
                RSTop = rstopsave;
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }

            }

            *(var + 1) = rplPeekData(1);        // STORE THE RESULT
            rplDropData(3);
            WORD valattr = rplGetIdentAttr(var[0]);
            if(valattr & IDATTR_DEPEND)
                rplDoAutoEval(var[0], rplGetDirfromGlobal(var));


        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }
        return;

    case INCR:

    {
        //@SHORT_DESC=Add one to the content of a variable
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t varidx = 0, done_add = 0;

        if(rplDepthData() >= 4) {
            // CHECK IF THIS IS A RETRY
            if(rplPeekData(3) == IPtr) {
                // THE ADDITION IS COMPLETE, NOW STORE THE VARIABLE
                done_add = 1;
                varidx = 2;
            }
        }

        if(!varidx) {
            // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
            // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
            if(ISLIST(*rplPeekData(1))) {
                rplListUnaryDoCmd();
                return;
            }

            if(ISIDENT(*rplPeekData(1)))
                varidx = 1;
            if(!varidx) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }
        }
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *stksave = DSTop;

        word_p *var = rplFindLAM(rplPeekData(varidx), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(varidx), 1);
        if(var) {
            if(ISDIR(*var[1])) {
                rplError(ERR_CANTOVERWRITEDIR);
                return;
            }
            if(ISLOCKEDIDENT(*var[0])) {
                rplError(ERR_READONLYVARIABLE);
                return;
            }

            if(!done_add) {

                rplPushData(IPtr);
                rplPushData(rplPeekData(varidx + 1));
                if(Exceptions)
                    return;
                rplPushData(*(var + 1));
                rplPushData((word_p) one_bint);        // PUSH THE OBJECT TO ADD
                if(Exceptions)
                    return;
                word_p *rstopsave = RSTop;
                rplPushRet(IPtr);
                rplCallOvrOperator(CMD_OVR_ADD);
                if(IPtr != *rstopsave) {
                    // THIS OPERATION WAS NOT ATOMIC, LET THE RPL ENGINE RUN UNTIL IT COMES BACK HERE
                    rstopsave[1] = (word_p) retrysemi_seco;    // REPLACE THE RETURN ADDRESS WITH A RETRY
                    return;
                }
                RSTop = rstopsave;
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }

            }

            *(var + 1) = rplPeekData(1);        // STORE THE INCREMENTED COUNTER
            rplOverwriteData(4, rplPeekData(1));
            rplDropData(3);
            WORD valattr = rplGetIdentAttr(var[0]);
            if(valattr & IDATTR_DEPEND)
                rplDoAutoEval(var[0], rplGetDirfromGlobal(var));


        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }
        return;

    case DECR:

    {
        //@SHORT_DESC=Subtract one from content of a variable
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t varidx = 0, done_add = 0;

        if(rplDepthData() >= 4) {
            // CHECK IF THIS IS A RETRY
            if(rplPeekData(3) == IPtr) {
                // THE ADDITION IS COMPLETE, NOW STORE THE VARIABLE
                done_add = 1;
                varidx = 2;
            }
        }

        if(!varidx) {
            // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
            // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
            if(ISLIST(*rplPeekData(1))) {
                rplListUnaryDoCmd();
                return;
            }

            if(ISIDENT(*rplPeekData(1)))
                varidx = 1;
            if(!varidx) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }
        }
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *stksave = DSTop;

        word_p *var = rplFindLAM(rplPeekData(varidx), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(varidx), 1);
        if(var) {
            if(ISDIR(*var[1])) {
                rplError(ERR_CANTOVERWRITEDIR);
                return;
            }
            if(ISLOCKEDIDENT(*var[0])) {
                rplError(ERR_READONLYVARIABLE);
                return;
            }

            if(!done_add) {

                rplPushData(IPtr);
                rplPushData(rplPeekData(varidx + 1));
                if(Exceptions)
                    return;
                rplPushData(*(var + 1));
                rplPushData((word_p) one_bint);        // PUSH THE OBJECT TO ADD
                if(Exceptions)
                    return;
                word_p *rstopsave = RSTop;
                rplPushRet(IPtr);
                rplCallOvrOperator(CMD_OVR_SUB);
                if(IPtr != *rstopsave) {
                    // THIS OPERATION WAS NOT ATOMIC, LET THE RPL ENGINE RUN UNTIL IT COMES BACK HERE
                    rstopsave[1] = (word_p) retrysemi_seco;    // REPLACE THE RETURN ADDRESS WITH A RETRY
                    return;
                }
                RSTop = rstopsave;
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }

            }

            *(var + 1) = rplPeekData(1);        // STORE THE INCREMENTED COUNTER
            rplOverwriteData(4, rplPeekData(1));
            rplDropData(3);
            WORD valattr = rplGetIdentAttr(var[0]);
            if(valattr & IDATTR_DEPEND)
                rplDoAutoEval(var[0], rplGetDirfromGlobal(var));


        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }
        return;

    case PURGE:
        //@SHORT_DESC=Delete a variable
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {

            rplListUnaryNoResultDoCmd();
            return;
        }

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        rplPurgeGlobal(rplPeekData(1));
        if(!Exceptions)
            rplDropData(1);
        return;

    case CRDIR:
    {
        //@SHORT_DESC=Create new directory
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryNoResultDoCmd();
            return;
        }

        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;

        }

        // SAFEGUARD AGAINST OVERWRITING
        word_p *val = rplFindGlobal(rplPeekData(1), 0);
        if(val) {
            if(ISDIR(*val[1])) {
                rplError(ERR_CANTOVERWRITEDIR);
                return;
            }
            if(ISLOCKEDIDENT(*val[0])) {
                rplError(ERR_READONLYVARIABLE);
                return;
            }

        }

        word_p name = rplMakeIdentQuoted(rplPeekData(1));
        rplCreateNewDir(name, CurrentDir);

        rplDropData(1);

    }
        return;
    case PGDIR:
        //@SHORT_DESC=Purge entire directory tree
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

        // APPLY THE OPCODE TO LISTS ELEMENT BY ELEMENT
        // THIS IS GENERIC, USE THE SAME CONCEPT FOR OTHER OPCODES
        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryNoResultDoCmd();
            return;
        }

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        // TODO: ALSO ACCEPT A LIST OF VARS, WHEN WE HAVE LISTS!

        rplPurgeDir(rplPeekData(1));
        if(!Exceptions)
            rplDropData(1);
        return;

    case UPDIR:
    {
        //@SHORT_DESC=Change current directory to its parent
        word_p *dir = rplGetParentDir(CurrentDir);
        if(dir)
            CurrentDir = dir;
        return;
    }
        return;
    case HOME:
        //@SHORT_DESC=Change current directory to HOME
        CurrentDir = Directories;
        return;
    case PATH:

    {
        //@SHORT_DESC=Get a path to the current directory
        word_p *scandir = CurrentDir;
        int32_t nitems = 0;
        word_p *stksave = DSTop;

        while(scandir != Directories) {
            word_p name = rplGetDirName(scandir);
            if(name) {
                rplPushData(name);
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }
                ++nitems;
            }
            scandir = rplGetParentDir(scandir);
        }

        if(scandir == Directories) {
            rplPushData((word_p) home_opcode);
            ++nitems;
            if(Exceptions) {
                DSTop = stksave;
                return;
            }
        }

        if(nitems == 0)
            rplPushData((word_p) empty_list);
        else {
            // REVERSE THE ORDER IN THE STACK, AS HOME IS THE LAST ONE

            int32_t f;
            word_p obj;
            for(f = 1; f <= nitems / 2; ++f) {
                obj = rplPeekData(f);
                rplOverwriteData(f, rplPeekData(nitems + 1 - f));
                rplOverwriteData(nitems + 1 - f, obj);
            }
            rplNewBINTPush(nitems, DECBINT);
            rplCreateList();
        }
        if(Exceptions)
            DSTop = stksave;
        return;
    }

    case VARS:
    {
        //@SHORT_DESC=List all visible variables in a directory
        word_p *varptr = rplFindFirstInDir(CurrentDir);
        word_p *stksave = DSTop;
        int32_t nitems = 0;

        while(varptr) {
            if(ISIDENT(*varptr[0]) && !ISHIDDENIDENT(*varptr[0])) {
                rplPushData(varptr[0]);
                ++nitems;
            }
            varptr = rplFindNext(varptr);
        }

        if(nitems == 0)
            rplPushData((word_p) empty_list);
        else {
            rplNewBINTPush(nitems, DECBINT);
            if(Exceptions) {
                DSTop = stksave;
                return;
            }
            rplCreateList();
            if(Exceptions) {
                DSTop = stksave;
                return;
            }
        }

        return;
    }

    case ALLVARS:
    {
        //@SHORT_DESC=List all variables in a directory
        //@NEW
        word_p *varptr = rplFindFirstInDir(CurrentDir);
        word_p *stksave = DSTop;
        int32_t nitems = 0;

        while(varptr) {
            rplPushData(varptr[0]);
            ++nitems;
            varptr = rplFindNext(varptr);
        }

        if(nitems == 0)
            rplPushData((word_p) empty_list);
        else {
            rplNewBINTPush(nitems, DECBINT);
            if(Exceptions) {
                DSTop = stksave;
                return;
            }
            rplCreateList();
            if(Exceptions) {
                DSTop = stksave;
                return;
            }
        }

        return;
    }

    case ORDER:
    {
        //@SHORT_DESC=Sort variables in a directory
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISLIST(*rplPeekData(1))) {
            rplError(ERR_LISTEXPECTED);
            return;
        }

        word_p nextname = rplPeekData(1) + 1;
        word_p endoflist = rplSkipOb(rplPeekData(1));

        word_p *firstentry = rplFindFirstInDir(CurrentDir);
        word_p *foundentry;

        while((*nextname != CMD_ENDLIST) && (nextname < endoflist)) {
            foundentry = rplFindGlobal(nextname, 0);
            if(foundentry) {
                word_p name, val;
                // BRING THIS ENTRY TO THE PROPER LOCATION
                name = foundentry[0];
                val = foundentry[1];
                memmovew(firstentry + 2, firstentry,
                        (foundentry - firstentry) * (sizeof(void *) >> 2));
                firstentry[0] = name;
                firstentry[1] = val;
                firstentry += 2;
            }

            nextname = rplSkipOb(nextname);
        }

        // ALL VARIABLES SORTED
        rplDropData(1);
        return;
    }

    case QUOTEID:
    {
        //@SHORT_DESC=Add single quotes to a variable name
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        word_p ident = rplMakeIdentQuoted(rplPeekData(1));

        if(!ident)
            return;     // SOME ERROR OCCURRED

        rplOverwriteData(1, ident);

        return;

    }
    case UNQUOTEID:
    {
        //@SHORT_DESC=Remove single quotes from a variable name
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        word_p ident = rplMakeIdentUnquoted(rplPeekData(1));

        if(!ident)
            return;     // SOME ERROR OCCURRED

        rplOverwriteData(1, ident);

        return;

    }

    case HIDEVAR:
    {
        //@SHORT_DESC=Hide a variable (make invisible)
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryNoResultDoCmd();
            return;
        }

        // ONLY ACCEPT IDENTS AS KEYS

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;

        }

        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *var = rplFindLAM(rplPeekData(1), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(1), 1);
        if(var) {
            word_p ident = rplMakeIdentHidden(var[0]);
            if(!ident)
                return;
            if(ident != var[0])
                var[0] = ident;
            rplDropData(1);
            return;
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }

    case UNHIDEVAR:
    {
        //@SHORT_DESC=Make a hidden variable visible
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryNoResultDoCmd();
            return;
        }

        // ONLY ACCEPT IDENTS AS KEYS

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;

        }

        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *var = rplFindLAM(rplPeekData(1), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(1), 1);
        if(var) {
            word_p ident = rplMakeIdentVisible(var[0]);
            if(!ident)
                return;
            if(ident != var[0])
                var[0] = ident;
            rplDropData(1);
            return;
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }

        // ADD MORE OPCODES HERE

    case CLVAR:
    {
        //@SHORT_DESC=Purge all variables and empty subdirectories in current directory
        // PURGE ALL VARIABLES AND EMPTY SUBDIRECTORIES IN CURRENT DIR

        word_p *var;

        int32_t idx = 0;

        while((var = rplFindGlobalByIndexInDir(idx, CurrentDir))) {
            // PURGE THE VARIABLE UNLESS IT'S EITHER LOCKED OR
            // IT'S A NON-EMPTY DIRECTORY
            if(rplIsVarReadOnly(var) || (rplIsVarDirectory(var)
                        && !rplIsVarEmptyDir(var)))
                ++idx;
            else
                rplPurgeForced(var);
            if(Exceptions)
                return;
        }

        return;

    }

    case LOCKVAR:
    {
        //@SHORT_DESC=Make variable read-only
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryNoResultDoCmd();
            return;
        }

        // ONLY ACCEPT IDENTS AS KEYS

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;

        }

        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *var = rplFindLAM(rplPeekData(1), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(1), 1);
        if(var) {
            word_p ident = rplMakeIdentReadOnly(var[0]);
            if(!ident)
                return;
            if(ident != var[0])
                var[0] = ident;
            rplDropData(1);
            return;
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }

    case UNLOCKVAR:
    {
        //@SHORT_DESC=Make variable read/write
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryNoResultDoCmd();
            return;
        }

        // ONLY ACCEPT IDENTS AS KEYS

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;

        }

        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *var = rplFindLAM(rplPeekData(1), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(1), 1);
        if(var) {
            word_p ident = rplMakeIdentWriteable(var[0]);
            if(!ident)
                return;
            if(ident != var[0])
                var[0] = ident;
            rplDropData(1);
            return;
        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }

    case RENAME:
    {
        //@SHORT_DESC=Change the name of a variable
        //@NEW
        // RENAME A LAM OR GLOBAL VARIABLE
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);

        // ONLY ACCEPT IDENTS AS KEYS

        if(!ISIDENT(*rplPeekData(1)) || !ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        word_p *val = rplFindLAM(rplPeekData(2), 0);   // DON'T ALLOW TO RENAME IN UPPER ENVIRONMENTS

        if(val) {
            if(ISLOCKEDIDENT(*val[0])) {
                rplError(ERR_READONLYVARIABLE);
                return;
            }
            val[0] = rplPeekData(1);
            rplDropData(2);
        }
        else {
            // LAM WAS NOT FOUND, TRY A GLOBAL
            val = rplFindGlobal(rplPeekData(2), 0);
            if(val) {
                if(ISLOCKEDIDENT(*val[0])) {
                    rplError(ERR_READONLYVARIABLE);
                    return;
                }
                val[0] = rplPeekData(1);
                rplDropData(2);
            }
            else {
                rplError(ERR_UNDEFINEDVARIABLE);
                return;
            }
        }
    }
        return;

    case TVARS:
    {
        //@SHORT_DESC=List variables of a specific type
        //@INCOMPAT
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t nitems;
        word_p first, itemptr;
        if(!ISLIST(*rplPeekData(1))) {
            nitems = 1;
            first = rplPeekData(1);
        }
        else {
            nitems = rplListLength(rplPeekData(1));
            first = rplPeekData(1) + 1;
        }
        // SCAN CURRENT DIRECTORY FOR VARIABLES

        int32_t nvars = rplGetVisibleVarCount();
        word_p *savestk = DSTop;
        int32_t k, j, totalcount = 0;
        word_p *var;

        for(k = 0; k < nvars; ++k) {
            var = rplFindVisibleGlobalByIndex(k);

            LIBHANDLER han = rplGetLibHandler(LIBNUM(*var[1]));

            // GET THE SYMBOLIC TOKEN INFORMATION
            if(han) {
                WORD savecurOpcode = CurOpcode;
                ObjectPTR = var[1];
                CurOpcode = MK_OPCODE(LIBNUM(*ObjectPTR), OPCODE_GETINFO);
                (*han) ();

                CurOpcode = savecurOpcode;

                if(RetNum > OK_TOKENINFO) {

                    int32_t type = TypeInfo / 100;

                    // SCAN ENTIRE LIST

                    itemptr = first;
                    for(j = 0; j < nitems; ++j, itemptr = rplSkipOb(itemptr)) {
                        int64_t ltype = rplReadNumberAsInt64(itemptr);
                        if(Exceptions) {
                            DSTop = savestk;
                            return;
                        }
                        if(type == ltype) {
                            // SAVE REGISTERS
                            ScratchPointer1 = first;
                            ScratchPointer2 = itemptr;
                            rplPushData(var[0]);
                            ++totalcount;
                            first = ScratchPointer1;
                            itemptr = ScratchPointer2;
                            break;
                        }

                    }

                }
            }

        }

        word_p newlist = rplCreateListN(totalcount, 1, 1);
        if(!newlist) {
            DSTop = savestk;
            return;
        }
        rplOverwriteData(1, newlist);

        return;
    }

    case TVARSE:
    {
        //@SHORT_DESC=List all variables with extended type information
        //@NEW
        // SAME AS TVARS BUT USE EXTENDED TYPE INFORMATION
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t nitems;
        word_p first, itemptr;
        if(!ISLIST(*rplPeekData(1))) {
            nitems = 1;
            first = rplPeekData(1);
        }
        else {
            nitems = rplListLength(rplPeekData(1));
            first = rplPeekData(1) + 1;
        }
        // SCAN CURRENT DIRECTORY FOR VARIABLES

        int32_t nvars = rplGetVisibleVarCount();
        word_p *savestk = DSTop;
        int32_t k, j, totalcount = 0;
        word_p *var;

        for(k = 0; k < nvars; ++k) {
            var = rplFindVisibleGlobalByIndex(k);

            LIBHANDLER han = rplGetLibHandler(LIBNUM(*var[1]));

            // GET THE SYMBOLIC TOKEN INFORMATION
            if(han) {
                WORD savecurOpcode = CurOpcode;
                ObjectPTR = var[1];
                CurOpcode = MK_OPCODE(LIBNUM(*ObjectPTR), OPCODE_GETINFO);
                (*han) ();

                CurOpcode = savecurOpcode;

                if(RetNum > OK_TOKENINFO) {

                    int32_t type = TypeInfo;

                    // SCAN ENTIRE LIST

                    itemptr = first;
                    for(j = 0; j < nitems; ++j, itemptr = rplSkipOb(itemptr)) {
                        REAL rtype;
                        rplReadNumberAsReal(itemptr, &rtype);
                        if(Exceptions) {
                            DSTop = savestk;
                            return;
                        }
                        rtype.exp += 2;
                        int32_t ltype = getint32_tReal(&rtype);

                        if(type == ltype) {
                            // SAVE REGISTERS
                            ScratchPointer1 = first;
                            ScratchPointer2 = itemptr;
                            rplPushData(var[0]);
                            ++totalcount;
                            first = ScratchPointer1;
                            itemptr = ScratchPointer2;
                            break;
                        }

                    }

                }
            }

        }

        word_p newlist = rplCreateListN(totalcount, 1, 1);
        if(!newlist) {
            DSTop = savestk;
            return;
        }
        rplOverwriteData(1, newlist);

        return;
    }

    case SADD:
    {
        //@SHORT_DESC=Apply command ADD to the stored contents of the variable
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t varidx = 0, done_add = 0;

        if(rplDepthData() >= 5) {
            // CHECK IF THIS IS A RETRY
            if(rplPeekData(3) == IPtr) {
                // THE ADDITION IS COMPLETE, NOW STORE THE VARIABLE
                done_add = 1;
                varidx = 2;
            }
        }

        if(!varidx) {
            if(ISIDENT(*rplPeekData(1)))
                varidx = 1;
            else if(ISIDENT(*rplPeekData(2)))
                varidx = 2;
            if(!varidx) {
                rplError(ERR_IDENTEXPECTED);
                return;
            }
        }
        // GET CONTENT FROM LOCAL OR GLOBAL VARIABLE

        word_p *stksave = DSTop;

        word_p *var = rplFindLAM(rplPeekData(varidx), 1);
        if(!var)
            var = rplFindGlobal(rplPeekData(varidx), 1);
        if(var) {
            if(ISDIR(*var[1])) {
                rplError(ERR_CANTOVERWRITEDIR);
                return;
            }
            if(ISLOCKEDIDENT(*var[0])) {
                rplError(ERR_READONLYVARIABLE);
                return;
            }

            if(!done_add) {

                rplPushData(IPtr);
                rplPushData(rplPeekData(varidx + 1));
                if(Exceptions)
                    return;
                if(varidx == 2) {
                    rplPushData(*(var + 1));
                    rplPushData(rplPeekData((varidx ^ 3) + 3)); // PUSH THE OBJECT TO ADD
                }
                else {
                    rplPushData(rplPeekData((varidx ^ 3) + 2)); // PUSH THE OBJECT TO ADD
                    rplPushData(*(var + 1));
                }
                if(Exceptions)
                    return;
                word_p *rstopsave = RSTop;
                rplPushRet(IPtr);
                rplCallOperator(CMD_ADD);
                if(IPtr != *rstopsave) {
                    // THIS OPERATION WAS NOT ATOMIC, LET THE RPL ENGINE RUN UNTIL IT COMES BACK HERE
                    rstopsave[1] = (word_p) retrysemi_seco;    // REPLACE THE RETURN ADDRESS WITH A RETRY
                    return;
                }
                RSTop = rstopsave;
                if(Exceptions) {
                    DSTop = stksave;
                    return;
                }

            }

            *(var + 1) = rplPeekData(1);        // STORE THE INCREMENTED COUNTER
            rplDropData(5);
            WORD valattr = rplGetIdentAttr(var[0]);
            if(valattr & IDATTR_DEPEND)
                rplDoAutoEval(var[0], rplGetDirfromGlobal(var));


        }
        else {
            rplError(ERR_UNDEFINEDVARIABLE);
            return;
        }

    }
        return;

    case SPROP:
    {
        //@SHORT_DESC=Store a property to a variable
        //@NEW

        // PROPERTIES MUST BE 4 LETTERS (NO LESS, NO MORE) SEPARATED BY 3 DOTS:
        // 'X...Defn' MEANS PROPERTY 'Defn' OF VARIABLE X
        // PROPERTY NAME IS ARBITRARY BUT A COUPLE OF NAMES ARE USED BY THE SYSTEM (MORE TO COME):
        // 'Defn' = DEFINITION, WORKS AS A FORMULA IN A SPREADSHEET CELL
        // 'Depn' = DEPENDENTS, CONTAINS A LIST OF ALL VARIABLES THAT NEED UPDATE IF THIS VARIABLE CHANGES
        // 'Unit' = PREFERRED UNIT: VARIABLES WITH THIS PROPERTY WILL BE DISPLAYED IN THEIR PREFERRED UNIT IF CONVERSION IS POSSIBLE

        // OTHER NAMES ARE FREE FOR THE USER. SYSTEM/CAS PROPERTIES WILL ALWAYS BE CAMEL CASE, USER SHOULD USE ALL UPPERCASE TO AVOID CONFLICTS

        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_PROPERTYEXPECTED);
            return;
        }

        WORD prop = rplGetIdentProp(rplPeekData(1));
        if(!prop || (prop & 0x80808080))        // IF THERE'S ANY UNICODE CHARACTERS IS NOT A VALID PROPERTY, ASCII ONLY
        {
            rplError(ERR_INVALIDPROPERTY);
            return;
        }

        // WE HAVE A VALID PROPERTY, JUST STORE IT IN THE CURRENT DIRECTORY
        word_p varname = rplMakeIdentNoProps(rplPeekData(1));
        if(!varname)
            return;

        word_p *var = rplFindGlobalInDir(varname, CurrentDir, 0);
        if(!var) {
            // CAN'T SET A PROPERTY OF A VARIABLE THAT DOESN'T EXIST
            // TODO: UNLESS IT'S THE 'Defn' PROPERTY, THEN IT SHOULD BE EVALUATED
            if(prop == IDPROP_DEFN) {
                ScratchPointer1 = varname;
                rplCreateGlobalInDir(varname, (word_p) zero_bint, CurrentDir);
                if(Exceptions)
                    return;

                varname = ScratchPointer1;
                var = rplFindFirstInDir(CurrentDir);
            }
            else {
                rplError(ERR_UNDEFINEDVARIABLE);
                return;
            }
        }
        if(ISLOCKEDIDENT(*var[0])) {
            rplError(ERR_READONLYVARIABLE);
            return;
        }

        word_p *varprop = rplFindGlobalInDir(rplPeekData(1), CurrentDir, 0);
        word_p oldvarprop = 0;
        if(varprop) {
            oldvarprop = varprop[1];
            varprop[1] = rplPeekData(2);
        }
        else {
            // CREATE A NEW PROPERTY
            ScratchPointer2 = varname;
            word_p newname = rplMakeIdentHidden(rplPeekData(1));
            if(!newname)
                return;
            rplCreateGlobal(newname, rplPeekData(2));
            if(Exceptions)
                return;
            varname = ScratchPointer2;
            var += 2;   // CORRECT THE POINTER TO THE ORIGINAL VARIABLE, MOVED IN MEMORY WHEN CREATING THE GLOBAL
        }

        // IF THE PROPERTY IS A SYSTEM PROPERTY, DO SOME EXTRA HOUSEKEEPING

        if(prop == IDPROP_DEFN) {
            // CHANGING Defn NEEDS TO UPDATE THE DEPENDENCY CACHE

            // EVALUATE THE DEFINITION FOR THE FIRST TIME
            // FOUND A FORMULA OR PROGRAM, IF A PROGRAM, RUN XEQ THEN ->NUM
            // IF A FORMULA OR ANYTHING ELSE, ->NUM
            word_p *stksave = DSTop;
            rplPushData(oldvarprop);
            rplPushData(rplPeekData(2));
            word_p *stkcheck = DSTop;
            if(ISPROGRAM(*rplPeekData(1)))
                rplRunAtomic(CMD_OVR_XEQ);
            if(Exceptions) {
                rplBlameError(stksave[-1]);     // AT LEAST SHOW WHERE THE ERROR CAME FROM
                if(DSTop > stksave)
                    DSTop = stksave;
                return;
            }
            rplRunAtomic(CMD_OVR_NUM);
            if(Exceptions) {
                rplBlameError(stksave[-1]);     // AT LEAST SHOW WHERE THE ERROR CAME FROM
                if(DSTop > stksave)
                    DSTop = stksave;
                return;
            }

            // WE GOT THE RESULT ON THE STACK
            if(DSTop != stkcheck) {
                rplError(ERR_BADARGCOUNT);
                rplBlameError(stksave[-1]);     // AT LEAST SHOW WHERE THE ERROR CAME FROM
                DSTop = stksave;
                return;
            }
            // STORE THE NEW RESULT AND CONTINUE
            var[1] = rplPopData();
            varname = rplSetIdentAttr(var[0], IDATTR_DEFN, IDATTR_DEFN);
            if(!varname) {
                DSTop = stksave;
                return;
            }
            var[0] = varname;
            oldvarprop = rplPopData();

            rplUpdateDependencyTree(varname, CurrentDir, oldvarprop,
                    rplPeekData(2));

            if(Exceptions)
                return;
            // AND TRIGGER AUTO EVALUATION

            rplDoAutoEval(var[0], CurrentDir);
            if(Exceptions)
                return;

        }

        if(prop == IDPROP_UNIT) {
            // SET THE ATTRIBUTE MARKER
            varname = rplSetIdentAttr(var[0], IDATTR_PREFUNIT, IDATTR_PREFUNIT);
            if(!varname) {
                return;
            }
            var[0] = varname;

            if(Exceptions)
                return;
            // AND TRIGGER AUTO EVALUATION

        }

        // ADD OTHER SYSTEM SIDE EFFECTS HERE

        rplDropData(2);
        return;
    }
    case RPROP:
    {
        //@SHORT_DESC=Recall a property of a variable
        //@NEW
        // PROPERTIES MUST BE 4 LETTERS (NO LESS, NO MORE) SEPARATED BY 3 DOTS:
        // 'X...Defn' MEANS PROPERTY 'Defn' OF VARIABLE X

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_PROPERTYEXPECTED);
            return;
        }

        word_p *varprop = rplFindGlobalInDir(rplPeekData(1), CurrentDir, 0);
        if(varprop)
            rplOverwriteData(1, varprop[1]);
        else {
            rplError(ERR_UNDEFINEDPROPERTY);
            return;
        }

        return;
    }

    case PACKDIR:
    {
        //@SHORT_DESC=Pack a directory in an editable object
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        word_p *indir = 0;
        // LIST IS A PATH WHEN USING DOUBLE LISTS {{ }}, OTHERWISE ENABLE LIST PROCESSING
        if(ISLIST(*rplPeekData(1))) {
            word_p firstelem = rplPeekData(1) + 1;
            if(!ISLIST(*firstelem)) {
                rplListUnaryDoCmd();
                return;
            }
            // NOT A LIST, SO IT MUST BE A PATH
            indir = rplFindDirFromPath(rplPeekData(1) + 1, 0);

        }
        else if(ISIDENT(*rplPeekData(1))) {
            word_p *var = rplFindGlobal(rplPeekData(1), 1);

            if(!var) {
                rplError(ERR_DIRECTORYNOTFOUND);
                return;
            }
            if(!ISDIR(*var[1])) {
                rplError(ERR_DIRECTORYEXPECTED);
                return;
            }
            indir = rplFindDirbyHandle(var[1]);
        }
        else if(ISDIR(*rplPeekData(1)))
            indir = rplFindDirbyHandle(rplPeekData(1));
        else {
            rplError(ERR_DIRECTORYEXPECTED);
            return;
        }

        if(!indir) {
            rplError(ERR_DIRECTORYNOTFOUND);
            return;
        }

        // COMPUTE THE SIZE
        int32_t size = rplGetDirSize(indir);

        word_p newobj = rplAllocTempOb(size);
        if(!newobj)
            return;     // NOT ENOUGH MEMORY!!
        rplPackDirinPlace(indir, newobj);

        rplOverwriteData(1, newobj);

        return;
    }

    case OVR_FUNCEVAL:
    case OVR_EVAL:
    case OVR_EVAL1:
    case OVR_XEQ:
        // EVALUATING THE OBJECT HAS TO CHANGE THE CURRENT DIRECTORY INTO THIS ONE
    {
        if(!IS_PROLOG(*rplPeekData(1))) {
            WORD saveOpcode = CurOpcode;
            CurOpcode = *rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode = saveOpcode;
            return;
        }

        if(LIBNUM(*rplPeekData(1)) == DOPACKDIR) {
            // DO ABSOLUTELY NOTHING WITH PACKED DIRS
            return;

        }

        word_p *dir = rplFindDirbyHandle(rplPeekData(1));

        if(!dir) {
            rplError(ERR_DIRECTORYNOTFOUND);
            return;     //  LEAVE THE OBJECT UNEVALUATED. IT'S AN ORPHAN DIRECTORY OBJECT???
        }
        CurrentDir = dir;
        rplDropData(1);
        return;
    }
        return;
    case OVR_SAME:

        // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
        if(!IS_PROLOG(*rplPeekData(2)) || !IS_PROLOG(*rplPeekData(1))) {
            if(*rplPeekData(2) == *rplPeekData(1)) {
                rplDropData(2);
                rplPushTrue();
            }
            else {
                rplDropData(2);
                rplPushFalse();
            }
            return;

        }

        if(ISPACKEDDIR(*rplPeekData(1)) && ISPACKEDDIR(*rplPeekData(2))) {
            if(rplCompareObjects(rplPeekData(1), rplPeekData(2))) {
                rplDropData(2);
                rplPushTrue();
            }
            else {
                rplDropData(2);
                rplPushFalse();
            }
            return;
        }

        // DIRECTORY OBJECTS ARE THE SAME ONLY IF THEY POINT TO THE SAME DIRECTORY, HENCE THEY ARE THE SAME HANDLE
        if(rplPeekData(2) == rplPeekData(1)) {
            rplDropData(2);
            rplPushTrue();
        }
        else {
            rplDropData(2);
            rplPushFalse();
        }
        return;
    case OVR_ISTRUE:
        if(ISPACKEDDIR(*rplPeekData(1))) {
           if(OBJSIZE(*rplPeekData(1))==0) {
               rplOverwriteData(1, (word_p) zero_bint);
               return;
           }
        }
        rplOverwriteData(1, (word_p) one_bint);
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

        // COMPILE PACKED DIRECTORIES

        if((TokenLen == 9)
                && (!utf8ncmp2((char *)TokenStart, (char *)BlankStart,
                        "DIRECTORY", 9))) {
            // START A CONSTRUCT AND PUT ALL OBJECTS INSIDE
            rplCompileAppend((WORD) MK_PROLOG(LIBRARY_NUMBER + 1, 0));
            // FROM NOW ON IT'S ALL LOOSE OBJECTS NAME/OBJECT UNTIL FINAL OFFSET TABLES
            RetNum = OK_STARTCONSTRUCT;
            return;
        }

        if((TokenLen == 6)
                && (!utf8ncmp2((char *)TokenStart, (char *)BlankStart, "ENDDIR",
                        6))) {
            if((CurrentConstruct != MK_PROLOG(LIBRARY_NUMBER + 1, 0))) {
                RetNum = ERR_SYNTAX;
                return;
            }

            // JUST CLOSE THE OBJECT AND BE DONE
            RetNum = OK_ENDCONSTRUCT;
            return;
        }

        // LSTO NEEDS SPECIAL CONSIDERATION TO CREATE LAMS AT COMPILE TIME

        if((TokenLen == 3)
                && (!utf8ncmp2((char *)TokenStart, (char *)BlankStart, "STO",
                        3))) {

            // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

            // CHECK IF THE PREVIOUS OBJECT IS A QUOTED IDENT?
            word_p object, prevobject;
            int32_t notrack = 0;
            if(ValidateTop <= ValidateBottom) {
                // THERE'S NO ENVIRONMENT
                object = TempObEnd;     // START OF COMPILATION
            }
            else {
                object = *(ValidateTop - 1);    // GET LATEST CONSTRUCT
                ++object;       // AND SKIP THE PROLOG / ENTRY WORD

                // CHECK FOR CONDITIONAL VARIABLE CREATION!
                word_p *construct = ValidateTop - 1;
                while(construct >= ValidateBottom) {
                    if((**construct == CMD_THEN) || (**construct == CMD_ELSE)
                            || (**construct == CMD_THENERR)
                            || (**construct == CMD_ELSEERR)
                            || (**construct == CMD_THENCASE)) {
                        // DON'T TRACK LAMS THAT COULD BE CONDITIONALLY CREATED
                        notrack = 1;
                        break;
                    }
                    --construct;
                }

                // CHECK FOR PREVIOUS LAM TRACKING DISABLE MARKERS
                if(!notrack) {

                    word_p *env = nLAMBase;
                    do {
                        if(env < LAMTopSaved)
                            break;
                        if(env[1] == (word_p) lameval_seco) {
                            // FOUND THE MARKER, STOP TRACKING VARIABLES THIS COMPILE SESSION
                            notrack = 1;
                            break;
                        }
                        env = rplGetNextLAMEnv(env);
                    }
                    while(env);

                }

            }

            if(object < CompileEnd) {
                do {
                    prevobject = object;
                    object = rplSkipOb(object);
                }
                while(object < CompileEnd);

                // HERE PREVOBJECT CONTAINS THE LAST OBJECT THAT WAS COMPILED

                if(ISIDENT(*prevobject)) {
                    // WE HAVE A HARD-CODED IDENT, CHECK IF IT EXISTS ALREADY

                    // CHECK IF IT'S AN EXISTING LAM, COMPILE TO A PUTLAM OPCODE IF POSSIBLE

                    word_p *LAMptr = rplFindLAM(prevobject, 1);

                    if(LAMptr < LAMTopSaved) {
                        // THIS IS NOT A VALID LAM, LEAVE AS IDENT

                        rplCompileAppend(MK_OPCODE(LIBRARY_NUMBER, STO));

                        RetNum = OK_CONTINUE;
                        return;
                    }

                    if(LAMptr < nLAMBase) {
                        // THIS IS A LAM FROM AN UPPER CONSTRUCT
                        // WE CAN USE PUTLAM ONLY INSIDE LOOPS, NEVER ACROSS SECONDARIES

                        word_p *env = nLAMBase;
                        WORD prolog;
                        do {
                            if(LAMptr > env)
                                break;
                            prolog = **(env + 1);       // GET THE PROLOG OF THE SECONDARY
                            if(IS_PROLOG(prolog) && LIBNUM(prolog) == SECO) {
                                // LAMS ACROSS << >> SECONDARIES HAVE TO BE COMPILED AS IDENTS
                                rplCompileAppend(MK_OPCODE(LIBRARY_NUMBER, STO));

                                RetNum = OK_CONTINUE;
                                return;
                            }
                            env = rplGetNextLAMEnv(env);
                        }
                        while(env);

                    }

                    // SPECIAL CASE: WHEN A SECO DOESN'T HAVE ANY LOCALS YET
                    // BUT LAMS FROM THE PREVIOUS SECO SHOULDN'T BE COMPILED TO GETLAMS

                    // SCAN ALL CURRENT CONSTRUCTS TO FIND THE INNERMOST SECONDARY
                    // THEN VERIFY IF THAT SECONDARY IS THE CURRENT LAM ENVIRONMENT

                    // THIS IS TO FORCE ALL LAMS IN A SECO TO BE COMPILED AS IDENTS
                    // INSTEAD OF PUTLAMS

                    // LAMS ACROSS DOCOL'S ARE OK AND ALWAYS COMPILED AS PUTLAMS WHEN USING STO, CREATE NEW ONE WHEN USING LSTO
                    word_p *scanenv = ValidateTop - 1;

                    while(scanenv >= ValidateBottom) {
                        if(((LIBNUM(**scanenv) == DOCOL)
                                    || (LIBNUM(**scanenv) == SECO))
                                && (IS_PROLOG(**scanenv))) {
                            // FOUND INNERMOST SECONDARY
                            if(*scanenv > *(nLAMBase + 1)) {
                                // THE CURRENT LAM BASE IS OUTSIDE THE INNER SECONDARY
                                rplCompileAppend(MK_OPCODE(LIBRARY_NUMBER, STO));

                                RetNum = OK_CONTINUE;
                                return;
                            }
                            break;

                        }
                        --scanenv;
                    }

                    // IT'S A KNOWN LOCAL VARIABLE, COMPILE AS PUTLAM
                    int32_t Offset = ((int32_t) (LAMptr - nLAMBase)) >> 1;

                    // ONLY USE PUTLAM IF OFFSET IS WITHIN RANGE
                    if(Offset <= 32767 && Offset >= -32768) {
                        CompileEnd = prevobject;
                        rplCompileAppend(MK_OPCODE(DOIDENT,
                                    PUTLAMN + (Offset & 0xffff)));
                    }
                    else {
                        rplCompileAppend(MK_OPCODE(LIBRARY_NUMBER, STO));
                    }

                    RetNum = OK_CONTINUE;
                    return;
                }

            }

            rplCompileAppend(MK_OPCODE(LIBRARY_NUMBER, STO));
            RetNum = OK_CONTINUE;
            return;
        }

        if((TokenLen == 3)
                && (!utf8ncmp2((char *)TokenStart, (char *)BlankStart, "RCL",
                        3))) {

            // ONLY ACCEPT IDENTS AS KEYS (ONLY LOW-LEVEL VERSION CAN USE ARBITRARY OBJECTS)

            // CHECK IF THE PREVIOUS OBJECT IS A QUOTED IDENT?
            word_p object, prevobject;
            if(ValidateTop <= ValidateBottom) {
                // THERE'S NO ENVIRONMENT
                object = TempObEnd;     // START OF COMPILATION
            }
            else {
                object = *(ValidateTop - 1);    // GET LATEST CONSTRUCT
                ++object;       // AND SKIP THE PROLOG / ENTRY WORD
            }

            if(object < CompileEnd) {
                do {
                    prevobject = object;
                    object = rplSkipOb(object);
                }
                while(object < CompileEnd);

                // HERE PREVOBJECT CONTAINS THE LAST OBJECT THAT WAS COMPILED

                if(ISIDENT(*prevobject)) {
                    // WE HAVE A HARD-CODED IDENT, CHECK IF IT EXISTS ALREADY

                    // CHECK IF IT'S AN EXISTING LAM, COMPILE TO A GETLAM OPCODE IF POSSIBLE

                    word_p *LAMptr = rplFindLAM(prevobject, 1);

                    if(LAMptr < LAMTopSaved) {
                        // THIS IS NOT A VALID LAM, LEAVE AS IDENT

                        rplCompileAppend(MK_OPCODE(LIBRARY_NUMBER, RCL));
                        RetNum = OK_CONTINUE;
                        return;
                    }

                    if(LAMptr < nLAMBase) {
                        // THIS IS A LAM FROM AN UPPER CONSTRUCT
                        // WE CAN USE GETLAM ONLY INSIDE LOOPS, NEVER ACROSS SECONDARIES

                        word_p *env = nLAMBase;
                        WORD prolog;
                        do {
                            if(LAMptr > env)
                                break;
                            prolog = **(env + 1);       // GET THE PROLOG OF THE SECONDARY
                            if(IS_PROLOG(prolog) && LIBNUM(prolog) == SECO) {
                                // LAMS ACROSS << >> SECONDARIES HAVE TO BE COMPILED AS IDENTS
                                rplCompileAppend(MK_OPCODE(LIBRARY_NUMBER, RCL));

                                RetNum = OK_CONTINUE;
                                return;
                            }
                            env = rplGetNextLAMEnv(env);
                        }
                        while(env);

                    }

                    // SPECIAL CASE: WHEN A SECO DOESN'T HAVE ANY LOCALS YET
                    // BUT LAMS FROM THE PREVIOUS SECO SHOULDN'T BE COMPILED TO GETLAMS

                    // SCAN ALL CURRENT CONSTRUCTS TO FIND THE INNERMOST SECONDARY
                    // THEN VERIFY IF THAT SECONDARY IS THE CURRENT LAM ENVIRONMENT

                    // THIS IS TO FORCE ALL LAMS IN A SECO TO BE COMPILED AS IDENTS
                    // INSTEAD OF GETLAMS

                    // LAMS ACROSS DOCOL'S ARE OK AND ALWAYS COMPILED AS GETLAMS
                    word_p *scanenv = ValidateTop - 1;

                    while(scanenv >= ValidateBottom) {
                        if((LIBNUM(**scanenv) == SECO) && (IS_PROLOG(**scanenv))) {
                            // FOUND INNERMOST SECONDARY
                            if(*scanenv > *(nLAMBase + 1)) {
                                // THE CURRENT LAM BASE IS OUTSIDE THE INNER SECONDARY
                                rplCompileAppend(MK_OPCODE(LIBRARY_NUMBER, RCL));
                                RetNum = OK_CONTINUE;
                                return;
                            }
                            break;

                        }
                        --scanenv;
                    }

                    // IT'S A KNOWN LOCAL VARIABLE, COMPILE AS GETLAM
                    int32_t Offset = ((int32_t) (LAMptr - nLAMBase)) >> 1;

                    if(Offset <= 32767 && Offset >= -32768) {
                        CompileEnd = prevobject;
                        rplCompileAppend(MK_OPCODE(DOIDENT,
                                    GETLAMN + (Offset & 0xffff)));
                    }
                    else {
                        rplCompileAppend(MK_OPCODE(LIBRARY_NUMBER, RCL));
                    }
                    RetNum = OK_CONTINUE;
                    return;
                }

            }

            rplCompileAppend(MK_OPCODE(LIBRARY_NUMBER, RCL));
            RetNum = OK_CONTINUE;
            return;
        }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER, (char **)LIB_NAMES, NULL,
                LIB_NUMBEROFCMDS);
        return;

    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        if(IS_PROLOG(*DecompileObject)) {

            if(LIBNUM(*DecompileObject) == LIBRARY_NUMBER) {
                // IT'S A DIRECTORY OBJECT

                rplDecompAppendString("DIR<");

                word_p *dir = rplFindDirbyHandle(DecompileObject);
                if(dir) {
                    word_p path[16];
                    int32_t lvlcount = rplGetFullPath(dir, path, 16);

                    if(lvlcount == 16)
                        rplDecompAppendString("...");

                    while(lvlcount > 0) {
                        rplDecompAppendChar('/');
                        rplDecompile(path[lvlcount - 1], DECOMP_EMBEDDED);
                        lvlcount--;
                    }
                }
                else
                    rplDecompAppendString("*unlinked*");

                rplDecompAppendString(">");
                RetNum = OK_CONTINUE;
                return;
            }

            // OTHERWISE IT'S A PACKED DIRECTORY OBJECT

            rplDecompAppendString("DIRECTORY");
            int32_t depth = 0, needseparator;

            needseparator =
                    !rplDecompDoHintsWidth(HINT_NLAFTER | HINT_ADDINDENTAFTER);
            if(needseparator)
                rplDecompAppendChar(' ');

            int32_t offset = 1, endoffset =
                    rplObjSize(DecompileObject), innerendoffset = endoffset;
            int32_t isodd = 1;

            while(offset <= endoffset) {
                if(offset == innerendoffset) {
                    rplDecompDoHintsWidth(HINT_SUBINDENTBEFORE);
                    rplDecompAppendString("ENDDIR");
                    if(Exceptions) {
                        RetNum = ERR_INVALID;
                        return;
                    }

                    if(depth)
                        needseparator = !rplDecompDoHintsWidth(HINT_NLAFTER);
                    else {
                        if(innerendoffset == endoffset)
                            break;      // END AFTER THE OUTER ENDDIR
                        needseparator = !rplDecompDoHintsWidth(0);
                    }
                    if(needseparator && offset < endoffset - 1)
                        rplDecompAppendChar(' ');

                    --depth;

                    // FIND THE INNER_END_OFFSET OF THE PARENT
                    innerendoffset = endoffset;

                    int32_t tmpoff = 1, endtmpoff;
                    while(tmpoff < offset) {
                        endtmpoff =
                                tmpoff + rplObjSize(DecompileObject + tmpoff);
                        if(ISPACKEDDIR(DecompileObject[tmpoff])) {
                            if((endtmpoff > offset)
                                    && (endtmpoff < innerendoffset))
                                innerendoffset = endtmpoff;
                        }
                        tmpoff = endtmpoff;
                    }
                    // HERE WE HAVE A PROPER INNERENDOFFSET

                    continue;
                }

                if(ISPACKEDDIR(DecompileObject[offset])) {

                    //rplDecompDoHintsWidth(HINT_NLAFTER);
                    rplDecompAppendString("DIRECTORY");
                    needseparator =
                            !rplDecompDoHintsWidth(HINT_NLAFTER |
                            HINT_ADDINDENTAFTER);
                    if(needseparator)
                        rplDecompAppendChar(' ');

                    if(Exceptions) {
                        RetNum = ERR_INVALID;
                        return;
                    }

                    innerendoffset =
                            offset + rplObjSize(DecompileObject + offset);

                    ++depth;

                    ++offset;
                    isodd ^= 1;

                    continue;
                }

                rplDecompile(DecompileObject + offset, DECOMP_EMBEDDED | ((CurOpcode == OPCODE_DECOMPEDIT) ? (DECOMP_EDIT | DECOMP_NOHINTS) : DECOMP_NOHINTS)); // RUN EMBEDDED
                if(Exceptions) {
                    RetNum = ERR_INVALID;
                    return;
                }

                offset += rplObjSize(DecompileObject + offset);
                if(!isodd)
                    needseparator = !rplDecompDoHintsWidth(HINT_NLAFTER);
                else
                    needseparator = !rplDecompDoHintsWidth(0);
                if(needseparator)
                    rplDecompAppendChar(' ');

                isodd ^= 1;

            }

            RetNum = OK_CONTINUE;
            return;

        }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds((char **)LIB_NAMES, NULL, LIB_NUMBEROFCMDS);
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

        RetNum = OK_CONTINUE;
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
        // RetNum =  OK_TOKENINFO | MK_TOKEN_INFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND
    {
        libProbeCmds((char **)LIB_NAMES, (int32_t *) LIB_TOKENINFO,
                LIB_NUMBEROFCMDS);

        return;
    }

    case OPCODE_GETINFO:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL COMMAND OR OBJECT IN ObjectPTR
        // NEEDS TO RETURN INFORMATION ABOUT THE TYPE:
        // IN RetNum: RETURN THE MK_TOKEN_INFO() DATA FOR THE SYMBOLIC COMPILER AND CAS
        // IN DecompHints: RETURN SOME HINTS FOR THE DECOMPILER TO DO CODE BEAUTIFICATION (TO BE DETERMINED)
        // IN TypeInfo: RETURN TYPE INFORMATION FOR THE TYPE COMMAND
        //             TypeInfo: TTTTFF WHERE TTTT = MAIN TYPE * 100 (NORMALLY THE MAIN LIBRARY NUMBER)
        //                                FF = 2 DECIMAL DIGITS FOR THE SUBTYPE OR FLAGS (VARIES DEPENDING ON LIBRARY)
        //             THE TYPE COMMAND WILL RETURN A REAL NUMBER TypeInfo/100
        // FOR NUMBERS: TYPE=10 (REALS), SUBTYPES = .01 = APPROX., .02 = INTEGER, .03 = APPROX. INTEGER
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL int32_t, .42 = HEX INTEGER
        if(IS_PROLOG(*ObjectPTR)) {
            TypeInfo = LIBRARY_NUMBER * 100;
            DecompHints = 0;
            RetNum = OK_TOKENINFO | MK_TOKEN_INFO(0, TITYPE_NOTALLOWED, 0, 1);
        }
        else {
            TypeInfo = 0;       // ALL COMMANDS ARE TYPE 0
            DecompHints = 0;
            libGetInfo2(*ObjectPTR, (char **)LIB_NAMES, (int32_t *) LIB_TOKENINFO,
                    LIB_NUMBEROFCMDS);
        }
        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, ObjectIDHash=hash, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER, (word_p *) ROMPTR_TABLE, ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID, ObjectIDHash=hash
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((word_p *) ROMPTR_TABLE, ObjectID, ObjectIDHash);
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID

        if(IS_PROLOG(*ObjectPTR)) {
            if(!ISPACKEDDIR(*ObjectPTR)) {
                // DIRECTORY OBJECTS CAN ONLY HAVE 1 WORD
                if(OBJSIZE(*ObjectPTR) != 1) {
                    RetNum = ERR_INVALID;
                    return;
                }

                // ANY OTHER CHECKS TO DIRECTORY STRUCTURE ARE DANGEROUS TO BE DONE HERE
            }
        }
        RetNum = OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER, (char **)LIB_NAMES,
                LIB_NUMBEROFCMDS);
        return;

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(MENU_NUMBER(MenuCodeArg) > 0) {
            RetNum = ERR_NOTMINE;
            return;
        }
        ObjectPTR = (word_p) lib28_menu;
        RetNum = OK_CONTINUE;
        return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp, (word_p) LIB_HELPTABLE);
        return;
    }
    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {

        libFindMsg(LibError, (word_p) LIB_MSGTABLE);
        return;
    }

    case OPCODE_LIBINSTALL:
        LibraryList = (word_p) libnumberlist;
        RetNum = OK_CONTINUE;
        return;
    case OPCODE_LIBREMOVE:
        return;

    }
    // UNHANDLED OPCODE...

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode) >= MIN_RESERVED_OPCODE) {
        RetNum = ERR_NOTMINE;
        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    rplError(ERR_INVALIDOPCODE);

    return;

}

#endif
