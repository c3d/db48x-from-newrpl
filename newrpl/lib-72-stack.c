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
#define LIBRARY_NUMBER  72

//@TITLE=Stack manipulation

#define ERROR_LIST \
    ERR(BADSTACKINDEX,0)

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    ECMD(UNPROTECTSTACK,"",MKTOKENINFO(0,TITYPE_NOTALLOWED,1,2)), \
    CMD(CLEAR,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(DEPTH,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(DROP,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(DROP2,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(DROPN,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(DUP,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(DUP2,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(DUPDUP,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(DUPN,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(NDUPN,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(NIP,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(OVER,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(PICK,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(PICK3,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(ROLL,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(ROLLD,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(ROT,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(SWAP,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(UNPICK,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(UNROT,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(IFT,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(IFTE,MKTOKENINFO(4,TITYPE_FUNCTION,3,2)), \
    CMD(STKPUSH,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(STKPOP,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(STKDROP,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(STKPICK,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(STKDEPTH,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(STKNEW,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"

#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);

INCLUDE_ROMOBJECT(lib72menu_main);

ROMOBJECT unprotect_seco[] = {
    MKPROLOG(DOCOL, 2),
    MKOPCODE(LIBRARY_NUMBER, UNPROTECTSTACK),
    CMD_SEMI
};

ROMOBJECT ift_seco[] = {
    MKPROLOG(DOCOL, 7),
    (CMD_IF),
    (CMD_THEN),
    (CMD_SWAP),
    (CMD_DROP),
    (CMD_OVR_XEQ),      // DO XEQ, IT WILL RUN IF CODE, DO NOTHING OTHERWISE
    (CMD_ENDIF),
    CMD_SEMI
};

ROMOBJECT ifte_seco[] = {
    MKPROLOG(DOCOL, 12),
    (CMD_IF),
    (CMD_THEN),
    (CMD_DROP),
    (CMD_SWAP),
    (CMD_DROP),
    (CMD_OVR_XEQ),      // DO XEQ, IT WILL RUN IF CODE, DO NOTHING OTHERWISE
    (CMD_ELSE),
    (CMD_UNROT),
    (CMD_DROP2),
    (CMD_OVR_XEQ),      // DO XEQ, IT WILL RUN IF CODE, DO NOTHING OTHERWISE
    (CMD_ENDIF),
    CMD_SEMI
};

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const word_p const ROMPTR_TABLE[] = {
    (word_p) LIB_MSGTABLE,
    (word_p) LIB_HELPTABLE,
    (word_p) lib72menu_main,
    (word_p) unprotect_seco,
    (word_p) ift_seco,
    (word_p) ifte_seco,
    0
};

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    // LIBRARIES THAT DEFINE ONLY COMMANDS STILL HAVE TO RESPOND TO A FEW OVERLOADABLE OPERATORS
    if(LIBNUM(CurOpcode) == LIB_OVERLOADABLE) {
        // ONLY RESPOND TO EVAL, EVAL1 AND XEQ FOR THE COMMANDS DEFINED HERE
        // IN CASE OF COMMANDS TREATED AS OBJECTS (WHEN EMBEDDED IN LISTS)
        if((OPCODE(CurOpcode) == OVR_EVAL) ||
                (OPCODE(CurOpcode) == OVR_EVAL1) ||
                (OPCODE(CurOpcode) == OVR_XEQ)) {
            // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
            if(rplDepthData() < 1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORD saveOpcode = CurOpcode;
            CurOpcode = *rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode = saveOpcode;
            return;
        }
        // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
        if(OPCODE(CurOpcode) == OVR_SAME) {
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
        else {
            rplError(ERR_INVALIDOPCODE);
            return;
        }

    }

    switch (OPCODE(CurOpcode)) {
    case CLEAR:
        //@SHORT_DESC=Remove all objects from the stack
        // ONLY CLEAR UP TO THE STACK PROTECTED AREA
        // DON'T THROW AN ERROR
        DSTop = DStkProtect;
        return;
    case DEPTH:
        //@SHORT_DESC=Get the current stack depth
        rplNewint32_tPush(rplDepthData(), DECint32_t);
        return;
    case DROP:
        //@SHORT_DESC=Remove an object from the stack
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplDropData(1);
        return;
    case DROP2:
        //@SHORT_DESC=Remove two objects form the stack
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplDropData(2);
        return;
    case DROPN:
    {
        //@SHORT_DESC=Remove N objects from the stack
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        int64_t count = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;
        if(count < 0 || rplDepthData() < count + 1) {
            rplError(ERR_BADSTACKINDEX);
            return;
        }
        rplDropData(count + 1);
        return;
    }
    case DUP:
        //@SHORT_DESC=Duplicate an object on the stack
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplPushData(rplPeekData(1));
        return;
    case DUP2:
        //@SHORT_DESC=Duplicate two objects on the stack
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplPushData(rplPeekData(2));
        rplPushData(rplPeekData(2));
        return;
    case DUPDUP:
        //@SHORT_DESC=Duplicate the same object twice on the stack
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplPushData(rplPeekData(1));
        rplPushData(rplPeekData(1));
        return;
    case DUPN:
    {
        //@SHORT_DESC=Duplicate a group of N objects
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        int64_t count = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;
        if(count < 0 || rplDepthData() < count + 1) {
            rplError(ERR_BADSTACKINDEX);

            return;
        }
        rplDropData(1);
        int64_t f;
        for(f = 0; f < count; ++f) {
            rplPushData(rplPeekData(count));
            if(Exceptions)
                return;
        }
        return;
    }
    case NDUPN:
    {
        //@SHORT_DESC=Replicate one object N times and return N
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        int64_t count = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;
        if(count < 0) {
            rplError(ERR_BADSTACKINDEX);

            return;
        }
        rplDropData(1);
        int64_t f;
        for(f = 1; f < count; ++f) {
            rplPushData(rplPeekData(1));
            if(Exceptions)
                return;
        }
        rplNewint32_tPush(count, DECint32_t);
        return;
    }
    case NIP:
        //@SHORT_DESC=Remove object at level 2 on the stack
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplOverwriteData(2, rplPeekData(1));
        rplDropData(1);
        return;

    case OVER:
        //@SHORT_DESC=Duplicate object at level 2 on the stack
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplPushData(rplPeekData(2));
        return;
    case PICK:
    {
        //@SHORT_DESC=Duplicate object at position N on the stack
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        int64_t level = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;
        if((level < 1) || (rplDepthData() < 1 + level)) {
            rplError(ERR_BADSTACKINDEX);

            return;
        }

        rplOverwriteData(1, rplPeekData(1 + level));

        return;
    }
    case PICK3:
        //@SHORT_DESC=Duplicate object at level 3 on the stack
        if(rplDepthData() < 3) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        rplPushData(rplPeekData(3));
        return;
    case ROLL:
    {
        //@SHORT_DESC=Move object at level N to level 1
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        int64_t level = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;

        if((level < 1) || (rplDepthData() < 1 + level)) {
            rplError(ERR_BADSTACKINDEX);

            return;
        }

        rplDropData(1);

        word_p objn = rplPeekData(level);
        word_p *stkptr = DSTop - level;

        int64_t count;
        for(count = 1; count < level; ++count, ++stkptr)
            *stkptr = *(stkptr + 1);

        rplOverwriteData(1, objn);

        return;
    }
    case ROLLD:
    {
        //@SHORT_DESC=Move object from level 1 to level N
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        int64_t level = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;

        if((level < 1) || (rplDepthData() < 1 + level)) {
            rplError(ERR_BADSTACKINDEX);

            return;
        }

        rplDropData(1);

        word_p objn = rplPeekData(1);
        word_p *stkptr = DSTop - 1;

        int64_t count;
        for(count = 1; count < level; ++count, --stkptr)
            *stkptr = *(stkptr - 1);

        rplOverwriteData(level, objn);

        return;
    }
    case ROT:
    {
        //@SHORT_DESC=Move object from level 3 to level 1
        if(rplDepthData() < 3) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        word_p obj1 = rplPeekData(1);
        rplOverwriteData(1, rplPeekData(3));
        rplOverwriteData(3, rplPeekData(2));
        rplOverwriteData(2, obj1);
        return;
    }

    case SWAP:
    {
        //@SHORT_DESC=Exchange objects in levels 1 and 2
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        word_p obj = rplPeekData(1);
        rplOverwriteData(1, rplPeekData(2));
        rplOverwriteData(2, obj);
        return;
    }

    case UNPICK:
    {
        //@SHORT_DESC=Move object from level 1 to level N.
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);

            return;
        }

        int64_t level = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;

        if((level < 1) || (rplDepthData() < 2 + level)) {
            rplError(ERR_BADSTACKINDEX);

            return;
        }

        word_p obj = rplPeekData(2);
        rplDropData(2);

        rplOverwriteData(level, obj);

        return;
    }

    case UNROT:
    {
        //@SHORT_DESC=Move object from level 1 to level 3
        if(rplDepthData() < 3) {
            rplError(ERR_BADARGCOUNT);

            return;
        }
        word_p obj1 = rplPeekData(1);
        rplOverwriteData(1, rplPeekData(2));
        rplOverwriteData(2, rplPeekData(3));
        rplOverwriteData(3, obj1);
        return;
    }

    case UNPROTECTSTACK:
    {
        // THIS INTERNAL OPCODE PROVIDES SAFETY GUARD AGAINST DATA STACK PROTECTION
        // IF A PROGRAM FORGETS TO UNPROTECT THE STACK, IT WILL BE UNPROTECTED
        // AUTOMATICALLY ON EXIT
        int32_t protlevel = (int32_t) ((word_p *) rplPopRet() - DStk);
        if((DStkBottom + protlevel >= DStkBottom)
                && (DStkBottom + protlevel < DSTop))
            DStkProtect = DStkBottom + protlevel;
        else
            DStkProtect = DStkBottom;
        return;
    }

    case IFT:
    {
        //@SHORT_DESC=Evaluate objects on the stack conditionally
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        word_p *savestk = DSTop;

        rplPushData(rplPeekData(2));
        if(Exceptions) {
            DSTop = savestk;
            return;
        }

        word_p *rstopsave = RSTop;
        rplPushRet(IPtr);
        rplCallOvrOperator(CMD_OVR_ISTRUE);
        if(Exceptions) {
            DSTop = savestk;
            RSTop = rstopsave;
            return;
        }
        if(IPtr != *rstopsave) {
            // THIS OPERATION WAS NOT ATOMIC, LET THE RPL ENGINE RUN UNTIL IT COMES BACK HERE
            rstopsave[1] = (word_p) ift_seco + 1;      // REPLACE THE RETURN ADDRESS WITH OUR SECONDARY
            return;
        }
        RSTop = rstopsave;
        // IT WAS AN ATOMIC OPERATION
        // DIRECT EXECUTION

        if(rplIsFalse(rplPopData())) {
            rplDropData(2);
            return;
        }

        rplOverwriteData(2, rplPeekData(1));
        rplDropData(1);
        rplCallOvrOperator(CMD_OVR_XEQ);
        return;

    }

    case IFTE:
    {
        //@SHORT_DESC=Evaluate objects on the stack conditionally
        if(rplDepthData() < 3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        word_p *savestk = DSTop;

        rplPushData(rplPeekData(3));
        if(Exceptions) {
            DSTop = savestk;
            return;
        }

        word_p *rstopsave = RSTop;
        rplPushRet(IPtr);
        rplCallOvrOperator(CMD_OVR_ISTRUE);
        if(Exceptions) {
            DSTop = savestk;
            RSTop = rstopsave;
            return;
        }
        if(IPtr != *rstopsave) {
            // THIS OPERATION WAS NOT ATOMIC, LET THE RPL ENGINE RUN UNTIL IT COMES BACK HERE
            rstopsave[1] = (word_p) ifte_seco + 1;     // REPLACE THE RETURN ADDRESS WITH OUR SECONDARY
            return;
        }
        RSTop = rstopsave;
        // IT WAS AN ATOMIC OPERATION

        // DIRECT EXECUTION

        if(rplIsFalse(rplPopData()))
            rplOverwriteData(3, rplPeekData(1));
        else
            rplOverwriteData(3, rplPeekData(2));
        rplDropData(2);
        rplCallOvrOperator(CMD_OVR_XEQ);
        return;

    }

    case STKPUSH:
    {
        //@SHORT_DESC=Push a snapshot of the current stack on the undo stack
        //@NEW
        // PUSH CURRENT STACK TO UNDO STACK
        rplTakeSnapshot();
        return;
    }
    case STKPOP:
    {
        //@SHORT_DESC=Pop a stack snapshot from the undo stack
        //@NEW
        rplRevertToSnapshot(1);
        return;
    }
    case STKDROP:
    {
        //@SHORT_DESC=Drop a snapshot from the undo stack
        //@NEW
        rplRemoveSnapshot(1);
        return;
    }

    case STKPICK:
        //  PICK A VALUE FROM ANY SNAPSHOT
    {
        //@SHORT_DESC=Copy snapshot in level N to the current stack
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        int64_t snap, level;

        snap = rplReadNumberAsInt64(rplPeekData(2));
        if(Exceptions)
            return;
        level = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;

        if((snap < 1) || (snap > rplCountSnapshots())) {
            rplError(ERR_BADSTACKINDEX);
            return;
        }

        if((level < 1) || (level > rplDepthSnapshot(snap))) {
            rplError(ERR_BADSTACKINDEX);
            return;
        }

        rplDropData(1);
        rplOverwriteData(1, rplPeekSnapshot(snap, level));

        return;

    }

    case STKDEPTH:
        //  PICK A VALUE FROM ANY SNAPSHOT
    {
        //@SHORT_DESC=Get the depth of the undo stack
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        int64_t snap;

        snap = rplReadNumberAsInt64(rplPeekData(1));
        if(Exceptions)
            return;

        if((snap < 1) || (snap > rplCountSnapshots())) {
            rplError(ERR_BADSTACKINDEX);
            return;
        }

        int32_t depth = rplDepthSnapshot(snap);

        word_p newbint = rplNewint32_t(depth, DECint32_t);
        if(!newbint)
            return;
        rplOverwriteData(1, newbint);

        return;

    }
    case STKNEW:
    {
        //@SHORT_DESC=Push a snapshot of the current stack on the undo stack and clears the current stack
        //@NEW
        // PUSH CURRENT STACK AND CLEAR
        rplTakeSnapshotAndClear();
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
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND
    {
        libProbeCmds((char **)LIB_NAMES, (int32_t *) LIB_TOKENINFO,
                LIB_NUMBEROFCMDS);

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
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL int32_t, .42 = HEX INTEGER

        if(ISPROLOG(*ObjectPTR)) {
            TypeInfo = LIBRARY_NUMBER * 100;
            DecompHints = 0;
            RetNum = OK_TOKENINFO | MKTOKENINFO(0, TITYPE_NOTALLOWED, 0, 1);
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
        // LIBRARY RETURNS: ObjectID=new ID, ObjectIDHash=hash RetNum=OK_CONTINUE
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
        if(ISPROLOG(*ObjectPTR)) {
            RetNum = ERR_INVALID;
            return;
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
        if(MENUNUMBER(MenuCodeArg) > 0) {
            RetNum = ERR_NOTMINE;
            return;
        }
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR = ROMPTR_TABLE[MENUNUMBER(MenuCodeArg) + 2];
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
