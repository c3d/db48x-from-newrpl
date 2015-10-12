/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

#include <stdio.h>



// LIB 65 PROVIDES COMMANDS THAT ARE ONLY TEMPORARY AND USED FOR DEBUG/DEVELOPMENT

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  65
#define LIB_ENUM lib65_enum
#define LIB_NAMES lib65_names
#define LIB_HANDLER lib65_handler
#define LIB_NUMBEROFCMDS LIB65_NUMBEROFCMDS

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(TICKS), \
    CMD(MEMCHECK), \
    CMD(MEMFIX), \
    CMD(CONSTANTS)


// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
/*#define CMD_EXTRANAME \
    "VOID",

#define CMD_EXTRAENUM \
    VOID
*/

// INTERNAL DECLARATIONS


// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a) a
enum LIB_ENUM { CMD_LIST /*, CMD_EXTRAENUM */, LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a) #a
const char * const LIB_NAMES[]= { CMD_LIST /*, CMD_EXTRANAME*/ };
#undef CMD


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    switch(OPCODE(CurOpcode))
    {
    case TICKS:
        // RETURN SYSTEM CLOCK
    {
        BINT64 ticks=halTicks();
        rplNewBINTPush(ticks,DECBINT);
        return;
    }
    case MEMCHECK:
    {
        // SYSTEM SANITY CHECK

        if(rplVerifyDStack(0)) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        if(rplVerifyRStack()) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        if(rplVerifyTempOb(0)) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        if(rplVerifyDirectories(0)) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);

        return;

    }
    case MEMFIX:
    {
        if(rplVerifyDStack(0)) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        if(rplVerifyRStack()) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        if(rplVerifyTempOb(1)) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        if(rplVerifyDirectories(0)) rplPushDataNoGrow((WORDPTR)one_bint);
        else rplPushDataNoGrow((WORDPTR)zero_bint);
        return;
    }
    case CONSTANTS:
    {
        REAL num;
     decconst_PI_200(&num);

     rplNewRealPush(&num);

     decconst_PI_180(&num);

     rplNewRealPush(&num);

     decconst_180_PI(&num);

     rplNewRealPush(&num);


     return;
    }
        /*
    case DONUM:
    {
        BINT f;
        REAL r;

        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplReadNumberAsReal(rplPeekData(1),&r);
        copyReal(&RReg[0],&r);
        left_justify(&RReg);

        printf("// LEN=%d\n",RReg[0].len);

        for(f=0;f<RReg[0].len;++f) {
            printf("%d, ",RReg[0].data[f]);
        }

        return;


    }*/

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


        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

     return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
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

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        RetNum=ERR_NOTMINE;
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        RetNum=ERR_NOTMINE;
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) { RetNum=ERR_INVALID; return; }

        RetNum=OK_CONTINUE;
        return;
    case OPCODE_AUTOCOMPNEXT:
        // THIS OPCODE RECEIVES
        // TokenStart = token string
        // TokenLen = token length
        // SuggestedOpcode = OPCODE OF THE CURRENT SUGGESTION, OR 0 IF SUGGESTION IS AN OBJECT
        // SuggestedObject = POINTER TO AN OBJECT (ONLY VALID IF SuggestedOpcode==0)
        // IF SuggestedOpcode LIBRARY NUMBER<THIS LIBRARY
        // IT MUST RETURN ERR_NOTMINE
        // IF SuggestedOpcode OR SuggestedObject BELONG TO THIS LIBRARY,
        // SEARCH MUST CONTINUE AFTER THAT SUGGESTION
        // IF A NEW SUGGESTION IS FOUND, RETURN OK_CONTINUE, AND SET
        // SuggestedOpcode TO THE NEXT SUGGESTION, or 0 IF SUGGESTION IS AN OBJECT
        // IN SUCH CASE, SuggestedObject MUST BE SET TO POINT TO THE OBJECT IN QUESTION
        // IF THERE ARE NO MORE SUGGESTIONS, RETURN ERR_NOTMINE

        // AUTOCOMP RETURNS:
        // RetNum =  OK_CONTINUE
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND

        // NEXT OPCODE SCANS IN DECREASING LIBRARY AND DECREASING OPCODE NUMBER

    case OPCODE_AUTOCOMPPREV:
        //  SAME AS AUTOCOMPNEXT, BUT SCANS IN INCREASING OPCODE NUMBER


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





