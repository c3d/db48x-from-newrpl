/*
 * Copyright (c) 2017, Claudio Lapilli and the newRPL Team
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
#define LIBRARY_NUMBER  100

//@TITLE=USB Communications

#define ERROR_LIST \
    ERR(USBNOTCONNECTED,0), \
    ERR(USBINVALIDDATA,1), \
    ERR(USBCOMMERROR,2), \
    ERR(USBTIMEOUT,3)


// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(USBSTATUS,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(USBRECV,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(USBSEND,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(USBOFF,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(USBON,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(USBAUTORCV,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(USBARCHIVE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)), \
    CMD(USBRESTORE,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2))



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

INCLUDE_ROMOBJECT(lib100_menu);


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,
    (WORDPTR)lib100_menu,
    0
};

int exitOnUBSData(int useless)
{
UNUSED_ARGUMENT(useless);
if(usb_hasdata()) return 1; // END THE LOOP IF THEREŚ DATA IN THE USB
return 0;
}

typedef struct {
    BINT fileid;
    WORD progress;
} BACKUP_INFO;

// FOR rplBackup, RETURNS 1 ON SUCCESS, 0 ON ERROR
int rplUSBArchiveWriteWord(unsigned int data,void *opaque)
{
    BACKUP_INFO *info=(BACKUP_INFO *)opaque;

    if(Exceptions) return 0;

    // PROVIDE VISUAL FEEDBACK
    if(!((info->progress)&0xff))  { halSetNotification(N_CONNECTION,((info->progress)>>8)&0xf); halScreenUpdated(); }
    ++info->progress;
    WORD buffer=data;
    return usb_filewrite(info->fileid,(BYTEPTR)&buffer,4);
}

// FOR rplRestoreBackup, RETURNS THE WORD, SET AND rplError ON ERROR
WORD rplUSBArchiveReadWord(void *opaque)
{
    BACKUP_INFO *info=(BACKUP_INFO *)opaque;

    WORD data;
    if(Exceptions) return 0;

    // PROVIDE VISUAL FEEDBACK
    if(!((info->progress)&0xff)) { halSetNotification(N_CONNECTION,((info->progress)>>8)&0xf); halScreenUpdated(); }
    ++info->progress;

    switch(usb_fileread(info->fileid,(BYTEPTR)&data,4))
    {
    case 0:
        // OPERATION TIMED OUT
        if(usb_eof(info->fileid)) rplError(ERR_INVALIDDATA);
        else rplError(ERR_USBTIMEOUT);
        return 0;
    case 4:
        return data;
    case 1:
    case 2:
    case 3:
    default:
        rplError(ERR_USBCOMMERROR);
        return 0;
    }
}




extern int waitProcess(int);

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    // LIBRARIES THAT DEFINE ONLY COMMANDS STILL HAVE TO RESPOND TO A FEW OVERLOADABLE OPERATORS
    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE) {
        // ONLY RESPOND TO EVAL, EVAL1 AND XEQ FOR THE COMMANDS DEFINED HERE
        // IN CASE OF COMMANDS TREATED AS OBJECTS (WHEN EMBEDDED IN LISTS)
        if( (OPCODE(CurOpcode)==OVR_EVAL)||
                (OPCODE(CurOpcode)==OVR_EVAL1)||
                (OPCODE(CurOpcode)==OVR_XEQ) )
        {
            // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORD saveOpcode=CurOpcode;
            CurOpcode=*rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode=saveOpcode;
            return;
        }
        // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
            if(OPCODE(CurOpcode)==OVR_SAME) {
                if(*rplPeekData(2)==*rplPeekData(1)) {
                    rplDropData(2);
                    rplPushTrue();
                } else {
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

    switch(OPCODE(CurOpcode))
    {

    case USBSTATUS:
    {
        //@SHORT_DESC=Get status of the USB driver
        //@NEW
        BINT status=usb_isconnected()+2*usb_isconfigured()+4*usb_hasdata();

        rplNewBINTPush(status,HEXBINT);
        return;
    }

    case USBRECV:
    {
        //@SHORT_DESC=Receive an object through USB link
        //@NEW

        // RECEIVE A BLOCK OF DATA WITH TIMEOUT
        // TAKES ONE ARGUMENT, 0 = WAIT INDEFINITELY, OTHERWISE TIME IN SECONDS TO WAIT (SAME AS WAIT)
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!usb_isconfigured()) {
            rplError(ERR_USBNOTCONNECTED);
            return;
        }
        rplStripTagStack(1);

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        REAL timeout;
        rplReadNumberAsReal(rplPeekData(1),&timeout);
        timeout.exp+=3;    // CONVERT FROM SECONDS TO MILLISECONDS
        BINT64 mstimeout=getBINT64Real(&timeout);


        RetNum=0;

        if(mstimeout<0) mstimeout=0;

        // JUST WAIT IN THE MAIN LOOP, SO KEYS CAN CANCEL THE WAIT AND INDICATORS CAN BE UPDATED
        halOuterLoop(mstimeout,&waitProcess,&exitOnUBSData,OL_NOCOMMS|OL_NOEXIT|OL_NOAUTOOFF|OL_NOCUSTOMKEYS|OL_NODEFAULTKEYS|OL_EXITONERROR);

        if(!usb_hasdata()) {
            rplDropData(1);
            rplPushFalse();
            return;
        }


        // READ THE DATA AND PUT IT ON THE STACK
        WORDPTR newobj=0,newobjptr;
        WORD fileid;
        BINT bytesread, needwords,expectedsize,allocated,offset;


        fileid=usb_rxfileopen();

        if(!fileid) {
            rplError(ERR_USBCOMMERROR);
            return;
        }

        // GET MINIMAL STORAGE FIRST, 64 bytes=16 words
        newobj=rplAllocTempOb(16);
        if(!newobj) {
            // INSUFFICIENT MEMORY
            return;
        }
        allocated=16;

        switch(usb_filetype(fileid))
        {
        case 'O':   // THIS IS AN RPL OBJECT
            newobjptr=newobj;
            break;
        case 'D':   // THIS IS ARBITRARY BINARY DATA
            newobjptr=newobj+4;
            newobj[0]=MKPROLOG(DOBINDATA,0);    // WE DON'T KNOW THE SIZE YET
            break;
        case 'B':   // THIS IS A BACKUP
        case 'W':   // THIS IS A FIRMWARE UPDATE
            rplError(ERR_USBINVALIDDATA);
            break;
        default:    // UNKNOWN DATA TYPE IS INVALID
            rplError(ERR_USBINVALIDDATA);
            break;
        }

        while(!Exceptions) {
            offset=newobjptr-newobj;
            bytesread=usb_fileread(fileid,(BYTEPTR)newobjptr,(allocated-offset)*sizeof(WORD));
            newobjptr+=(bytesread+3)>>2;
            if(bytesread<(allocated-offset)*sizeof(WORD)) {

                if(bytesread==0) {
                    if(!usb_eof(fileid)) rplError(ERR_USBTIMEOUT);
                    break;
                }
            }
            // MORE DATA IS EXPECTED, ALLOCATE MORE MEMORY

            ScratchPointer1=newobj;
            offset=newobjptr-newobj;
            rplResizeLastObject(16);
            allocated+=16;
            newobj=ScratchPointer1;
            newobjptr=newobj+offset;
        }

        //  WE ARE DONE WITH THE TRANSMISSION
        usb_rxfileclose(fileid);


        if(Exceptions) return;

        // THERE WERE NO ERRORS, WE RECEIVED SOMETHING
        switch(usb_filetype(fileid))
        {
        case 'O':   // THIS IS AN RPL OBJECT
            if(offset<rplObjSize(newobj)) {     // CHECK IF WE RECEIVED A COMPLETE OBJECT
                rplError(ERR_USBINVALIDDATA);
                return;
            }
            if(!rplVerifyObject(newobj)) {      // AND ALSO THAT THE OBJECT IS VALID
                rplError(ERR_USBINVALIDDATA);
                return;
            }
            break;
        case 'D':   // THIS IS ARBITRARY BINARY DATA
            newobj[0]=MKPROLOG(DOBINDATA,newobjptr-newobj-1);    // FIX THE SIZE IN THE PROLOG
            break;
        default:    // UNKNOWN DATA TYPE IS INVALID - THIS IS UNREACHABLE-CAN'T HAPPEN
            break;
        }

        // WE HAVE A VALID OBJECT!

        rplOverwriteData(1,newobj);
        rplPushTrue();
        return;
    }

    case USBSEND:
    {
        //@SHORT_DESC=Send an object through the USB link
        //@NEW

        // SEND ONE OBJECT THROUGH THE USB LINK
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!usb_isconfigured()) {
            rplError(ERR_USBNOTCONNECTED);
            return;
        }

        BINT result,fileid=usb_txfileopen('O');    // OPEN AN RPL OBJECT TYPE TRANSMISSION
        result=fileid? 1:0;
        if(result) result=usb_filewrite(fileid,(BYTEPTR)rplPeekData(1),rplObjSize(rplPeekData(1))*sizeof(WORD));
        if(result) result=usb_txfileclose(fileid);
        else usb_txfileclose(fileid);

        if(!result) rplOverwriteData(1,(WORDPTR)zero_bint);
        else rplOverwriteData(1,(WORDPTR)one_bint);

        return;
    }

    case USBON:
    {
        //@SHORT_DESC=Enable USB port
        //@NEW

        usb_init(0);
        return;
    }
    case USBOFF:
    {
        //@SHORT_DESC=Disable USB port
        //@NEW

        usb_shutdown();
        return;
    }


    case USBAUTORCV:
    {
        //@SHORT_DESC=Receive an object and execute it
        //@NEW

    // SAME AS USBRCV BUT DOES NOT WAIT FOR DATA (DOES NOTHING IF DATA IS NOT AVAILABLE)
    // AND DOES XEQ IMMEDIATELY ON THE DATA RECEIVED.
        if(!usb_hasdata()) {
            return;
        }

        // READ THE DATA AND PUT IT ON THE STACK
        WORDPTR newobj=0,newobjptr;
        WORD fileid;
        BINT bytesread, needwords,expectedsize,allocated,offset;


        fileid=usb_rxfileopen();

        if(!fileid) {
            rplError(ERR_USBCOMMERROR);
            return;
        }

        // GET MINIMAL STORAGE FIRST, 64 bytes=16 words
        newobj=rplAllocTempOb(16);
        if(!newobj) {
            // INSUFFICIENT MEMORY
            return;
        }
        allocated=16;

        switch(usb_filetype(fileid))
        {
        case 'O':   // THIS IS AN RPL OBJECT
            newobjptr=newobj;
            break;
        case 'D':   // THIS IS ARBITRARY BINARY DATA
            newobjptr=newobj+4;
            newobj[0]=MKPROLOG(DOBINDATA,0);    // WE DON'T KNOW THE SIZE YET
            break;
        case 'B':   // THIS IS A BACKUP
        case 'W':   // THIS IS A FIRMWARE UPDATE
            rplError(ERR_USBINVALIDDATA);
            break;
        default:    // UNKNOWN DATA TYPE IS INVALID
            rplError(ERR_USBINVALIDDATA);
            break;
        }

        while(!Exceptions) {
            offset=newobjptr-newobj;
            bytesread=usb_fileread(fileid,(BYTEPTR)newobjptr,(allocated-offset)*sizeof(WORD));
            newobjptr+=(bytesread+3)>>2;
            if(bytesread<(allocated-offset)*sizeof(WORD)) {

                if(bytesread==0) {
                    if(!usb_eof(fileid)) rplError(ERR_USBTIMEOUT);
                    break;
                }
            }
            // MORE DATA IS EXPECTED, ALLOCATE MORE MEMORY

            ScratchPointer1=newobj;
            offset=newobjptr-newobj;
            rplResizeLastObject(16);
            allocated+=16;
            newobj=ScratchPointer1;
            newobjptr=newobj+offset;
        }

        //  WE ARE DONE WITH THE TRANSMISSION
        usb_rxfileclose(fileid);


        if(Exceptions) return;

        // THERE WERE NO ERRORS, WE RECEIVED SOMETHING
        switch(usb_filetype(fileid))
        {
        case 'O':   // THIS IS AN RPL OBJECT
            if(offset<rplObjSize(newobj)) {     // CHECK IF WE RECEIVED A COMPLETE OBJECT
                rplError(ERR_USBINVALIDDATA);
                return;
            }
            if(!rplVerifyObject(newobj)) {      // AND ALSO THAT THE OBJECT IS VALID
                rplError(ERR_USBINVALIDDATA);
                return;
            }
            break;
        case 'D':   // THIS IS ARBITRARY BINARY DATA
            newobj[0]=MKPROLOG(DOBINDATA,newobjptr-newobj-1);    // FIX THE SIZE IN THE PROLOG
            break;
        default:    // UNKNOWN DATA TYPE IS INVALID - THIS IS UNREACHABLE-CAN'T HAPPEN
            break;
        }

        // WE HAVE A VALID OBJECT!

        rplPushData(newobj);
        if(Exceptions) return;

        rplCallOvrOperator(CMD_OVR_XEQ);

        return;

    }





    case USBARCHIVE:
    {
        //@SHORT_DESC=Create a backup on a remote machine
        //@NEW

        // STORE A BACKUP OBJECT DIRECTLY INTO A FILE ON THE USB HOST MACHINE

        if(!usb_isconfigured()) {
            rplError(ERR_USBNOTCONNECTED);
            return;
        }

        BACKUP_INFO info;
        info.fileid=usb_txfileopen('B');
        info.progress=0;

        if(!info.fileid) {
            rplError(ERR_USBCOMMERROR);     // IT'S ACTUALLY OUT OF BUFFER MEMORY
            return;
        }
        // PACK THE STACK
        WORDPTR stklist=0;

        if(rplDepthData()>=1) stklist=rplCreateListN(rplDepthData(),1,0);
        // JUST DON'T SAVE THE STACK IF THERE'S NOT ENOUGH MEMORY FOR IT
        if(stklist) {
            rplListAutoExpand(stklist);
            rplStoreSettings((WORDPTR)stksave_ident,stklist);
        }
        else rplPurgeSettings((WORDPTR)stksave_ident);



        BINT err = rplBackup(&rplUSBArchiveWriteWord,(void *)&info);

        usb_txfileclose(info.fileid);

        if(err!=1) {
            if(!Exceptions) rplError(ERR_USBCOMMERROR);
            return;
        }
        rplPurgeSettings((WORDPTR)stksave_ident);
        return;

    }


    case USBRESTORE:
    {
        //@SHORT_DESC=Restore a backup from a remote machine
        //@NEW

        // RECEIVE A BACKUP FILE OVER THE USB WIRE AND RESTORE IT DIRECTLY TO MEMORY
        // EXPECTS A TIMEOUT VALUE IN SECONDS

        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }


        if(!usb_isconnected()) {
            rplError(ERR_USBNOTCONNECTED);
            return;
        }
        rplStripTagStack(1);
        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_REALEXPECTED);
            return;
        }

        REAL timeout;
        rplReadNumberAsReal(rplPeekData(1),&timeout);
        timeout.exp+=3;    // CONVERT FROM SECONDS TO MILLISECONDS
        BINT64 mstimeout=getBINT64Real(&timeout);


        rplDropData(1);

        RetNum=0;

        if(mstimeout<0) mstimeout=0;

        // JUST WAIT IN THE MAIN LOOP, SO KEYS CAN CANCEL THE WAIT AND INDICATORS CAN BE UPDATED
        halOuterLoop(mstimeout,&waitProcess,&exitOnUBSData,OL_NOCOMMS|OL_NOEXIT|OL_NOAUTOOFF|OL_NOCUSTOMKEYS|OL_NODEFAULTKEYS|OL_EXITONERROR);


        if(!usb_hasdata()) {
            return;
        }

        BACKUP_INFO info;

        info.fileid=usb_rxfileopen();
        info.progress=0;

        if(usb_filetype(info.fileid)!='B') {
            rplError(ERR_USBCOMMERROR);
            if(info.fileid) usb_rxfileclose(info.fileid);
            return;
        }





        GCFlags=GC_IN_PROGRESS; // MARK THAT A GC IS IN PROGRESS TO BLOCK ANY HARDWARE INTERRUPTS


        BINT err = rplRestoreBackup(0,&rplUSBArchiveReadWord,(void *)&info);

        // FINISH THE DATA TRANSMISSION SO THE REMOTE CAN CONFIRM IT WAS SUCCESSFUL
        if((err==1)||(err==2)) {
            WORD data;
            // KEEP READING UNLESS THERE'S SOME KIND OF ERROR
            while(!usb_eof(info.fileid)) {
              if(usb_fileread(info.fileid,(BYTEPTR)&data,4)<4) break;
            }
            }



        usb_rxfileclose(info.fileid);

        switch(err)
        {
        case -1:
            // FILE WAS CORRUPTED, AND MEMORY WAS DESTROYED
            GCFlags=GC_COMPLETED;   // MARK THAT GC WAS COMPLETED
            usb_shutdown();
            throw_dbgexception("Memory lost during restore",__EX_WIPEOUT|__EX_RESET|__EX_NOREG);
            // THIS WON'T RETURN, ONLY A RESET IS ACCEPTABLE AT THIS POINT
            return;
        case 0:
            // FILE WAS CORRUPTED, BUT MEMORY IS STILL INTACT
            rplError(ERR_USBINVALIDDATA);
            GCFlags=0;  // NOTHING MOVED IN MEMORY, SO DON'T SIGNAL THAT A GC TOOK PLACE
            return;
        default:
        case 1:
            // SUCCESS! STILL NEED TO DO A WARMSTART
            // FALL THROUGH
        case 2:
            // SOME ERRORS, BUT rplWarmInit WILL FIX AUTOMATICALLY
            usb_shutdown();
            GCFlags=GC_COMPLETED;   // MARK THAT GC WAS COMPLETED SO HARDWARE INTERRUPTS ARE ACCEPTED AGAIN
            rplException(EX_POWEROFF|EX_HWRESET);  // REQUEST A COMPLETE HARDWARE RESET
            return;
        }

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
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_NOTALLOWED,0,1);
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
        if(ISPROLOG(*ObjectPTR)) { RetNum=ERR_INVALID; return; }

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
        if(MENUNUMBER(MenuCodeArg)>0) { RetNum=ERR_NOTMINE; return; }
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR=ROMPTR_TABLE[MENUNUMBER(MenuCodeArg)+2];
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

