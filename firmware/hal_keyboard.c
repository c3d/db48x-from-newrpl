/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


// THIS IS THE MAIN STABLE API FOR KEYBOARD ACCESS
#include <newrpl.h>
#include <ui.h>
#include <libraries.h>


// WAITS FOR A KEY TO BE PRESSED IN SLOW MODE

BINT halWaitForKey()
{
    int keymsg,wokeup;

    if(!(halFlags&HAL_FASTMODE) && (halBusyEvent>=0)) {
    tmr_eventkill(halBusyEvent);
    halBusyEvent=-1;
    }

    wokeup=0;
    do {


    keymsg=keyb_getmsg();

    if(!keymsg) {
    // FIRST: ENTER LOW SPEED MODE
    if(halFlags&HAL_FASTMODE) {
    cpu_setspeed(6000000);
    halFlags&=~HAL_FASTMODE;
    }
    if(halFlags&HAL_HOURGLASS) {
    halSetNotification(N_HOURGLASS,0);
    halFlags&=~HAL_HOURGLASS;
    }

    if(wokeup) return 0;   // ALLOW SCREEN REFRESH REQUESTED BY OTHER IRQ'S


    // LAST: GO INTO "WAIT FOR INTERRUPT"
    cpu_waitforinterrupt();
    wokeup=1;
    }
    } while(!keymsg);



    return keymsg;

}

// DO-NOTHING HANDLER
void timeouthandler()
{
    halFlags|=HAL_TIMEOUT;
}

BINT halWaitForKeyTimeout(BINT timeoutms)
{
    int keymsg,wokeup;

    if(!(halFlags&HAL_FASTMODE) && (halBusyEvent>=0)) {
    tmr_eventkill(halBusyEvent);
    halBusyEvent=-1;
    }

    wokeup=0;

    // START A TIMER TO PROVIDE PROPER TIMEOUT
    // USE timeoutms <=0 TO CONTINUE WAITING FOR A PREVIOUSLY SCHEDULED TIMEOUT
    // IN CASE OTHER EVENT WAKES UP THE CPU
    if(timeoutms>0) {
        halFlags&=~HAL_TIMEOUT;
        tmr_eventcreate(&timeouthandler,timeoutms,0);
    }

    do {


    keymsg=keyb_getmsg();

    if(!keymsg) {
    // FIRST: ENTER LOW SPEED MODE
    if(halFlags&HAL_FASTMODE) {
    cpu_setspeed(6000000);
    halFlags&=~HAL_FASTMODE;
    }
    if(halFlags&HAL_HOURGLASS) {
    halSetNotification(N_HOURGLASS,0);
    halFlags&=~HAL_HOURGLASS;
    }

    if(wokeup)
    {
        if(halFlags&HAL_TIMEOUT) {
            halFlags&=~HAL_TIMEOUT;
            return -1;
        }
        return 0;   // ALLOW SCREEN REFRESH REQUESTED BY OTHER IRQ'S
    }

    // LAST: GO INTO "WAIT FOR INTERRUPT"
    cpu_waitforinterrupt();
    wokeup=1;
    }
    } while(!keymsg);



    return keymsg;

}


// SYSTEM CONTEXT VARIABLE
// STORES THE CONTEXT ID
// ID=0 MEANS ANY CONTEXT
// ID BIT 0 --> SET TO 1 WHEN THE COMMAND LINE IS ACTIVE OR TEXT IS BEING EDITED
// ID BITS 1 AND BIT 2 ARE RESERVED FOR FUTURE USE AND SHOULD BE ZERO
// ID=8 IS THE STACK
// ID=16 IS PICT
// ID N*8 WITH N<100 ARE RESERVED FOR THE SYSTEM APPLICATIONS (SOLVER, ETC)
// ID= N*8 --> USER CONTEXTS FROM N=100 AND UP TO 16250 ARE FREE TO USE

// SET THE KEYBOARD CONTEXT
void halSetContext(BINT KeyContext)
{
halScreen.KeyContext=KeyContext;
}

// AND RETRIEVE
BINT halGetContext()
{
    return halScreen.KeyContext;
}

// TOGGLES BETWEEN ALPHA AND ANOTHER MODE
// isalpha TELLS IF ALPHA MODE IS ACTIVE, TO
// KEEP THE CURSOR IN SYNC
void halSwapCmdLineMode(BINT isalpha)
{
    int tmp=halScreen.CursorState;

    if(((tmp&0xff)=='L')||((tmp&0xff)=='C')) {
        // DO NOTHING IF WE ALREADY ARE IN ALPHA MODE
        if(isalpha) return;
    }
    else {
        if(!isalpha) return;
        // LOCK CAPS MODE WHEN ENTERING ALPHA MODE
        tmp&=0x00ffffff;
        tmp|='C'<<24;
    }
    halScreen.CursorState&=0x00ffff00;
    halScreen.CursorState|=tmp<<24;
    halScreen.CursorState|=(tmp>>24)&0xff;
}

void halSetCmdLineMode(BYTE mode)
{
    halScreen.CursorState=(halScreen.CursorState&~0xff)|mode;
}

BYTE halGetCmdLineMode()
{
    return halScreen.CursorState&0xff;
}

// DEBUG: DO-NOTHING KEYBOARD HANDLER
void dummyKeyhandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);
    return;
}

// END THE CURRENTLY OPEN COMMAND LINE, RETURN 1 IF COMPILED SUCCESSFULLY
// 0 IF ERROR.
// WHEN 1, THE STACK CONTAINS THE OBJECT/S COMPILED
// WHEN 0, THE COMMAND LINE IS STILL OPEN, WITH THE ERROR HIGHLIGHTED
BINT endCmdLineAndCompile()
{
    WORDPTR text=uiGetCmdLineText();
    if(!text) {
        throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
        return 0;
    }
    BINT len=rplStrSize(text);
    WORDPTR newobject;
    if(len) {
        newobject=rplCompile((BYTEPTR)(text+1),len,1);
        if(Exceptions || (!newobject)) {
            // TODO: SELECT THE WORD THAT CAUSED THE ERROR
            WORD fakeprogram = 0;
            ExceptionPointer=&fakeprogram;
            halShowErrorMsg();
            Exceptions=0;

            return 0;
        }
        else {
            // END ALPHA MODE
            halSwapCmdLineMode(0);
            keyb_setshiftplane(0,0,0,0);
            if(uiGetCmdLineState()&CMDSTATE_OVERWRITE) {
                if(rplDepthData()>=1) rplDropData(1);
            }
            uiCloseCmdLine();
            halSetCmdLineHeight(0);
            halSetContext(halGetContext()& (~CONTEXT_INEDITOR));
            // RUN THE OBJECT

            rplSetEntryPoint(newobject);

            // RUN AND CLEANUP PROPERLY
            BINT rstksave=RSTop-RStk,lamsave=LAMTop-LAMs,nlambase=nLAMBase-LAMs;
            BINT result=rplRun();


            switch(result)
            {
            case CLEAN_RUN:
            {
                // SOMEBODY CALLED EXITRPL EXPLICITLY
                // EVERYTHING WAS COMPLETELY CLEANED UP AND RESET
                halFlags&=~(HAL_HALTED|HAL_AUTORESUME);
                break;
            }
            case NEEDS_CLEANUP:
            {
                // UNTRAPPED ERROR
                // CLEANUP ANY GARBAGE AFTER OUR SAVED POINTER
                if(RSTop>=RStk+rstksave) {
                    RSTop=RStk+rstksave;
                }
                else { rplCleanup(); halFlags&=~(HAL_HALTED|HAL_AUTORESUME); }
                if(LAMTop>LAMs+lamsave) LAMTop=LAMs+lamsave;
                if(nLAMBase>LAMs+nlambase) nLAMBase=LAMs+nlambase;
                break;
            }


            case CODE_HALTED:
            {
                // UNTRAPPED ERROR
                // CLEANUP ANY GARBAGE AFTER OUR SAVED POINTER
                if(RSTop>RStk+rstksave)
                {
                    // THE CODE HALTED SOMEWHERE INSIDE!
                    halFlags|=HAL_HALTED;
                    if(Exceptions&EX_AUTORESUME) {
                        halFlags|=HAL_AUTORESUME;
                        Exceptions=0;
                    }
                }
                else {
                    if(RSTop<RStk+rstksave) {

                        // THIS CAN ONLY HAPPEN IF SOMEHOW
                        // THE CODE ESCAPED FROM THE SECONDARY
                        // WE CREATED, THIS CAN HAPPEN WHEN USING 'CONT'
                        // INSIDE A SECONDARY AND IT'S NOT NECESSARILY BAD
                        if(CurOpcode==CMD_ENDOFCODE) { rplClearErrors(); rplCleanup(); }
                        if(HaltedIPtr) {
                            halFlags|=HAL_HALTED;
                            if(Exceptions&EX_AUTORESUME) {
                                halFlags|=HAL_AUTORESUME;
                                Exceptions=0;
                            }
                        }
                        else halFlags&=~(HAL_HALTED|HAL_AUTORESUME);

                    }
                    else {
                        // RETURN STACK WAS INTACT, RESTORE THE REST
                        if(LAMTop>LAMs+lamsave) LAMTop=LAMs+lamsave;
                        if(nLAMBase>LAMs+nlambase) nLAMBase=LAMs+nlambase;

                        // DON'T ALTER THE INSTRUCTION POINTER OF THE HALTED PROGRAM
                        if(CurOpcode==CMD_ENDOFCODE) rplClearErrors();
                        if(HaltedIPtr) {
                            halFlags|=HAL_HALTED;
                            if(Exceptions&EX_AUTORESUME) {
                                halFlags|=HAL_AUTORESUME;
                                Exceptions=0;
                            }
                        }
                        else halFlags&=~(HAL_HALTED|HAL_AUTORESUME);

                    }
                }


                break;
            }

            }


            if(Exceptions) {
                // TODO: SHOW ERROR MESSAGE
                halShowErrorMsg();
                Exceptions=0;
                return 1;
            }
            // EVERYTHING WENT FINE, CLOSE THE COMMAND LINE
            return 1;

        }
    } else {
        // EMPTY COMMAND LINE!

        // END ALPHA MODE
        halSwapCmdLineMode(0);
        keyb_setshiftplane(0,0,0,0);

        // AND COMMAND LINE
        uiCloseCmdLine();
        halSetCmdLineHeight(0);
        halSetContext(halGetContext()& (~CONTEXT_INEDITOR));

        return 1;
    }

    return 0;
}

void endCmdLine()
{
            // END ALPHA MODE
            halSwapCmdLineMode(0);
            keyb_setshiftplane(0,0,0,0);

            // CLOSE COMMAND LINE DISCARDING CONTENTS
            uiCloseCmdLine();
            halSetCmdLineHeight(0);
            halSetContext(halGetContext()& (~CONTEXT_INEDITOR));
}




// **************************************************************************
// *******************    DEFAULT KEY HANDLERS     **************************
// **************************************************************************



void numberKeyHandler(BINT keymsg)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight+2);
        halSetContext(halGetContext()|CONTEXT_INEDITOR);
        if(KM_SHIFTPLANE(keymsg)&SHIFT_ALPHA) uiOpenCmdLine('X');
        else uiOpenCmdLine('D');
        }
    BINT number;
    switch(KM_KEY(keymsg))
    {
    case KB_1:
        number='1';
        break;
    case KB_2:
        number='2';
        break;
    case KB_3:
        number='3';
        break;
    case KB_4:
        number='4';
        break;
    case KB_5:
        number='5';
        break;
    case KB_6:
        number='6';
        break;
    case KB_7:
        number='7';
        break;
    case KB_8:
        number='8';
        break;
    case KB_9:
        number='9';
        break;
    case KB_0:
        number='0';
        break;
    }
        uiInsertCharactersN((BYTEPTR) &number,((BYTEPTR) &number)+1);
        uiAutocompleteUpdate();

}

void uiCmdRun(WORD Opcode)
{
    WORDPTR obj=rplAllocTempObLowMem(2);
if(obj) {

    // ENABLE UNDO
    rplRemoveSnapshot(halScreen.StkUndolevels+1);
    rplRemoveSnapshot(halScreen.StkUndolevels);
    if(halScreen.StkCurrentLevel!=1) rplTakeSnapshot();
    halScreen.StkCurrentLevel=0;


obj[0]=Opcode;
obj[1]=CMD_ENDOFCODE;
obj[2]=CMD_QSEMI;   // THIS IS FOR SAFETY REASONS
rplSetEntryPoint(obj);
BINT iseval=(Opcode==(CMD_OVR_XEQ)) || (Opcode==(CMD_OVR_EVAL)) || (Opcode==(CMD_OVR_EVAL1));

if(iseval) {
    // STORE THE OBJECT/OPCODE THAT MAY CAUSE AN EXCEPTION
    if(rplDepthData()>0) BlameCmd=rplPeekData(1);
    else BlameCmd=0;
} else BlameCmd=0;
    BINT rstksave=RSTop-RStk,lamsave=LAMTop-LAMs,nlambase=nLAMBase-LAMs;
    BINT result=rplRun();
    switch(result)
    {

    case CLEAN_RUN:
    {
        // SOMEBODY CALLED EXITRPL EXPLICITLY
        // EVERYTHING WAS COMPLETELY CLEANED UP AND RESET
        halFlags&=~(HAL_HALTED|HAL_AUTORESUME);
        break;
    }
    case NEEDS_CLEANUP:
    {
        // UNTRAPPED ERROR
        // CLEANUP ANY GARBAGE AFTER OUR SAVED POINTER
        if(RSTop>=RStk+rstksave) {
            RSTop=RStk+rstksave;
            // BLAME THE ERROR ON THE COMMAND WE CALLED
            if(BlameCmd!=0) rplBlameError(BlameCmd);
        }
        else { rplCleanup(); halFlags&=~(HAL_HALTED|HAL_AUTORESUME); }
        if(LAMTop>LAMs+lamsave) LAMTop=LAMs+lamsave;
        if(nLAMBase>LAMs+nlambase) nLAMBase=LAMs+nlambase;
        break;
    }


    case CODE_HALTED:
    {
        // UNTRAPPED ERROR
        // CLEANUP ANY GARBAGE AFTER OUR SAVED POINTER
        if(RSTop>RStk+rstksave)
        {
            // THE CODE HALTED SOMEWHERE INSIDE!
            halFlags|=HAL_HALTED;
            if(Exceptions&EX_AUTORESUME) {
                halFlags|=HAL_AUTORESUME;
                Exceptions=0;
            }
        }
        else {
            if(RSTop<RStk+rstksave) {
                // THIS CAN ONLY HAPPEN IF SOMEHOW
                // THE CODE ESCAPED FROM THE SECONDARY
                // WE CREATED, THIS CAN HAPPEN WHEN USING 'CONT'
                // INSIDE A SECONDARY AND IT'S NOT NECESSARILY BAD
                if(CurOpcode==CMD_ENDOFCODE) { rplClearErrors(); rplCleanup(); }
                if(HaltedIPtr) {
                    halFlags|=HAL_HALTED;
                    if(Exceptions&EX_AUTORESUME) {
                        halFlags|=HAL_AUTORESUME;
                        Exceptions=0;
                    }
                }
                else halFlags&=~(HAL_HALTED|HAL_AUTORESUME);

            }
            else {
                // RETURN STACK WAS INTACT, RESTORE THE REST
                if(LAMTop>LAMs+lamsave) LAMTop=LAMs+lamsave;
                if(nLAMBase>LAMs+nlambase) nLAMBase=LAMs+nlambase;

                    // DON'T ALTER THE INSTRUCTION POINTER OF THE HALTED PROGRAM
                    rplClearErrors();
                    if(HaltedIPtr) {
                        halFlags|=HAL_HALTED;
                        if(Exceptions&EX_AUTORESUME) {
                            halFlags|=HAL_AUTORESUME;
                            Exceptions=0;
                        }
                    }
                    else halFlags&=~(HAL_HALTED|HAL_AUTORESUME);

            }
        }


        break;
    }
    }

}

}

void uiCmdRunHide(WORD Opcode,BINT narguments)
{
    WORDPTR obj=rplAllocTempObLowMem(2);
if(obj) {

    // ENABLE UNDO
    rplRemoveSnapshot(halScreen.StkUndolevels+1);
    rplRemoveSnapshot(halScreen.StkUndolevels);
    if(halScreen.StkCurrentLevel!=1) rplTakeSnapshotHide(narguments);
    halScreen.StkCurrentLevel=0;


obj[0]=Opcode;
obj[1]=CMD_ENDOFCODE;
obj[2]=CMD_QSEMI;   // THIS IS FOR SAFETY REASONS
rplSetEntryPoint(obj);
BINT iseval=(Opcode==(CMD_OVR_XEQ)) || (Opcode==(CMD_OVR_EVAL)) || (Opcode==(CMD_OVR_EVAL1));

if(iseval) {
    // STORE THE OBJECT/OPCODE THAT MAY CAUSE AN EXCEPTION
    if(rplDepthData()>0) BlameCmd=rplPeekData(1);
    else BlameCmd=0;
} else BlameCmd=0;
    BINT rstksave=RSTop-RStk,lamsave=LAMTop-LAMs,nlambase=nLAMBase-LAMs;
    BINT result=rplRun();
    switch(result)
    {

    case CLEAN_RUN:
    {
        // SOMEBODY CALLED EXITRPL EXPLICITLY
        // EVERYTHING WAS COMPLETELY CLEANED UP AND RESET
        halFlags&=~(HAL_HALTED|HAL_AUTORESUME);
        break;
    }
    case NEEDS_CLEANUP:
    {
        // UNTRAPPED ERROR
        // CLEANUP ANY GARBAGE AFTER OUR SAVED POINTER
        if(RSTop>=RStk+rstksave) {
            RSTop=RStk+rstksave;
            // BLAME THE ERROR ON THE COMMAND WE CALLED
            if(BlameCmd!=0) rplBlameError(BlameCmd);
        }
        else { rplCleanup(); halFlags&=~(HAL_HALTED|HAL_AUTORESUME); }
        if(LAMTop>LAMs+lamsave) LAMTop=LAMs+lamsave;
        if(nLAMBase>LAMs+nlambase) nLAMBase=LAMs+nlambase;
        break;
    }


    case CODE_HALTED:
    {
        // UNTRAPPED ERROR
        // CLEANUP ANY GARBAGE AFTER OUR SAVED POINTER
        if(RSTop>RStk+rstksave)
        {
            // THE CODE HALTED SOMEWHERE INSIDE!
            halFlags|=HAL_HALTED;
            if(Exceptions&EX_AUTORESUME) {
                halFlags|=HAL_AUTORESUME;
                Exceptions=0;
            }

        }
        else {
            if(RSTop<RStk+rstksave) {
                // THIS CAN ONLY HAPPEN IF SOMEHOW
                // THE CODE ESCAPED FROM THE SECONDARY
                // WE CREATED, THIS CAN HAPPEN WHEN USING 'CONT'
                // INSIDE A SECONDARY AND IT'S NOT NECESSARILY BAD
                if(CurOpcode==CMD_ENDOFCODE) { rplClearErrors(); rplCleanup(); }
                if(HaltedIPtr) {
                    halFlags|=HAL_HALTED;
                    if(Exceptions&EX_AUTORESUME) {
                        halFlags|=HAL_AUTORESUME;
                        Exceptions=0;
                    }
                }
                else halFlags&=~(HAL_HALTED|HAL_AUTORESUME);

            }
            else {
                // RETURN STACK WAS INTACT, RESTORE THE REST
                if(LAMTop>LAMs+lamsave) LAMTop=LAMs+lamsave;
                if(nLAMBase>LAMs+nlambase) nLAMBase=LAMs+nlambase;

                    // DON'T ALTER THE INSTRUCTION POINTER OF THE HALTED PROGRAM
                    rplClearErrors();
                    if(HaltedIPtr) {
                        halFlags|=HAL_HALTED;
                        if(Exceptions&EX_AUTORESUME) {
                            halFlags|=HAL_AUTORESUME;
                            Exceptions=0;
                        }
                    }
                    else halFlags&=~(HAL_HALTED|HAL_AUTORESUME);

            }
        }


        break;
    }
    }

}

}


// EXECUTE THE OPCODE IN A PROTECTED "TRANSPARENT" ENVIRONMENT
// THE USER STACK. RETURN STACK AND LAM ENVIRONMENTS ARE
// ALL PRESERVED AND PROTECTED
// THE COMMAND RECEIVES nargs IN THE STACK AND RETURNS AT MOST nresults
// IT RETURNS THE NUMBER OF RESULTS LEFT IN THE STACK

BINT uiCmdRunTransparent(WORD Opcode,BINT nargs,BINT nresults)
{
WORDPTR obj=rplAllocTempObLowMem(2);
if(obj) {
obj[0]=Opcode;
obj[1]=CMD_ENDOFCODE;
obj[2]=CMD_QSEMI;   // THIS IS FOR SAFETY REASONS

BINT rsave,lamsave,nlambase,retvalue;
WORD exceptsave,errcodesave;
// PRESERVE VARIOUS STACK POINTERS

exceptsave=Exceptions;
errcodesave=ErrorCode;

rplSetExceptionHandler(0);  // SAVE CURRENT EXCEPTION HANDLERS
rplPushRet(IPtr);   // SAVE THE CURRENT INSTRUCTION POINTER

rplTakeSnapshotN(nargs);  // RUN THE COMMAND WITH A PROTECTED STACK WITH nargs ARGUMENTS ONLY

rsave=RSTop-RStk;        // PROTECT THE RETURN STACK
lamsave=LAMTop-LAMs;     // PROTECT LAM ENVIRONMENTS
nlambase=nLAMBase-LAMs;

Exceptions=0;           // REMOVE ALL EXCEPTIONS

rplSetEntryPoint(obj);

rplRun();

// DISCARD ANY ERRORS DURING EXECUTION,  IDEALLY IT HIT THE BREAKPOINT
if(Exceptions!=EX_HALT) {
    // THERE WAS SOME OTHER ERROR DURING EXECUTION, DISCARD ALL OUTPUT FROM THE FAILED PROGRAM
    rplClearData();
}

Exceptions=0;

// MANUAL RESTORE

if(RSTop>=RStk+rsave) RSTop=RStk+rsave;  // IF RSTop<RStk+rsave THE RETURN STACK WAS COMPLETELY CORRUPTED, SHOULD NEVER HAPPEN BUT...
else rplCleanup();
if(LAMTop>=LAMs+lamsave) LAMTop=LAMs+lamsave;  // OTHERWISE THE LAM ENVIRONMENTS WERE DESTROYED, SHOULD NEVER HAPPEN BUT...
else rplCleanup();
if(nLAMBase>=LAMs+nlambase) nLAMBase=LAMs+nlambase;  // OTHERWISE THE LAM ENVIRONMENTS WERE DESTROYED, SHOULD NEVER HAPPEN BUT...
else rplCleanup();

// CLEAN THE STACK
if(rplDepthData()>nresults) {
    BINT f;
    BINT depth=rplDepthData(),offset=depth-nresults;
    for(f=depth;f>depth-nresults;--f) {
        rplOverwriteData(f,rplPeekData(f-offset));
    }
    rplDropData(offset);
}
// HERE THE STACK CONTAINS UP TO nresults ONLY

rplTakeSnapshotAndClear();  // HERE SNAPSHOT1 = RESULTS, SNAPSHOT2 = PREVIOUS STACK
rplRevertToSnapshot(2);     // RECOVER THE PREVIOUS STACK
rplDropData(nargs);         // REMOVE THE ORIGINAL ARGUMENTS
nresults=retvalue=rplDepthSnapshot(1);   // GET THE NUMBER OF RESULTS
while(nresults) { rplPushData(rplPeekSnapshot(1,nresults)); --nresults; }   // EXTRACT THE RESULTS INTO THE CURRENT STACK
rplRemoveSnapshot(1);       // AND CLEANUP

// RESTORE THE ERROR CODES FIRST, TO CAPTURE ANY ERRORS DURING POPPING THE RETURN STACK
Exceptions=exceptsave;
ErrorCode=errcodesave;

// RESTORE THE IP POINTER
IPtr=rplPopRet();

// AND THE ERROR HANDLERS
rplRemoveExceptionHandler();

// IF EVERYTHING WENT WELL, HERE WE HAVE THE SAME ENVIRONMENT AS BEFORE
// IF SOMETHING GOT CORRUPTED, WE SHOULD HAVE AN INTERNAL EMPTY RSTACK ERROR
return retvalue;

}
return 0;
}

// RESTURE THE STACK TO WHAT IT WAS AT THE GIVEN LEVEL
// LEVEL 1 = MOST IMMEDIATE ... LEVEL StkUndoLevel = OLDEST
// SPECIAL CASE: LEVEL 0 = USER'S CURRENT STACK
BINT uiRestoreUndoLevel(BINT level)
{
BINT nlevels=rplCountSnapshots();

if(level<1) return halScreen.StkCurrentLevel;
if(level>nlevels) level=nlevels;

if(!halScreen.StkCurrentLevel) {
    // WHEN CURRENT LEVEL IS ZERO, MEANS THE PREVIOUS ACTION WAS NOT A RESTORE
    // WE NEED TO PRESERVE THE CURRENT STACK AS LEVEL 0
    rplTakeSnapshot();
    ++level;
}


// HERE LEVEL 1 = USER STACK, 2..(N+1) = N UNDO LEVELS PRESERVED

rplRestoreSnapshot(level);
return level;

}

void uiStackUndo()
{
    halScreen.StkCurrentLevel=uiRestoreUndoLevel(halScreen.StkCurrentLevel+1);
}

void uiStackRedo()
{
    halScreen.StkCurrentLevel=uiRestoreUndoLevel(halScreen.StkCurrentLevel-1);
}


// TYPICAL COMMAND KEY HANDLER.
// EXECUTES Opcode IN DIRECT MODE
// INSERTS Progmode AS TEXT IN THE COMMAND LINE WHEN IN PROGRAMMING MODE
// IF IsFunc == 0 --> IN ALG MODE INSERT THE SAME TEXT AS IN PROG. MODE
//    IsFunc == 1 --> IN ALG MODE INSERT THE SAME TEXT AS IN PROG, WITH FUNCTION PARENTHESIS
//    IsFunc == 2 --> IN ALG MODE, RUN THE OPCODE DIRECTLY, AS IN 'D' MODE
//    IsFunc < 0  --> NOT ALLOWED IN SYMBOLIC (ALG) MODE, DO NOTHING



void cmdKeyHandler(WORD Opcode,BYTEPTR Progmode,BINT IsFunc)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // ACTION WHEN IN THE STACK
                uiCmdRun(Opcode);
                if(Exceptions) {
                    // TODO: SHOW ERROR MESSAGE
                    halShowErrorMsg();
                    Exceptions=0;
                } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY|STAREA_DIRTY;
            halScreen.DirtyFlag|=STACK_DIRTY;


        }

    }
    else{
        // ACTION INSIDE THE EDITOR
        switch(halScreen.CursorState&0xff)
        {

        case 'D':   // DIRECT EXECUTION
        {

                if(endCmdLineAndCompile()) {
                uiCmdRun(Opcode);
                if(Exceptions) {
                    // TODO: SHOW ERROR MESSAGE
                    halShowErrorMsg();
                    Exceptions=0;
                } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY|STAREA_DIRTY;
            halScreen.DirtyFlag|=STACK_DIRTY;
                }
            break;
        }
        case 'P':   // PROGRAMMING MODE
            // TODO: SEPARATE TOKENS
            uiSeparateToken();
            uiInsertCharacters(Progmode);
            uiSeparateToken();
            uiAutocompleteUpdate();
            break;

        case 'L':
        case 'C':
        case 'A':   // ALPHANUMERIC MODE
            if(IsFunc>=0) {

                if(IsFunc==2) {
                    if(endCmdLineAndCompile()) {
                        uiCmdRun(Opcode);
                        if(Exceptions) {
                            // TODO: SHOW ERROR MESSAGE
                            halShowErrorMsg();
                            Exceptions=0;
                        } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY|STAREA_DIRTY;
                        halScreen.DirtyFlag|=STACK_DIRTY;
                    }
                    break;
                }
            if(IsFunc<2) {
            uiInsertCharacters(Progmode);
            if(IsFunc==1) {
                uiInsertCharacters((BYTEPTR)"()");
                uiCursorLeft(1);
            }
            uiAutocompleteUpdate();
            }
            }
            break;
        default:
         break;
        }
    }
}


void transpcmdKeyHandler(WORD Opcode)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // ACTION WHEN IN THE STACK
                uiCmdRun(Opcode);
                if(Exceptions) {
                    // TODO: SHOW ERROR MESSAGE
                    halShowErrorMsg();
                    Exceptions=0;
                } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY|STAREA_DIRTY;
            halScreen.DirtyFlag|=STACK_DIRTY;
        }

    }
    else{
        // ACTION INSIDE THE EDITOR
            uiCmdRun(Opcode);
            if(Exceptions) {
                // TODO: SHOW ERROR MESSAGE
                halShowErrorMsg();
                Exceptions=0;
            } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY|STAREA_DIRTY;
        halScreen.DirtyFlag|=STACK_DIRTY;
    }

}


void varsKeyHandler(BINT keymsg,BINT menunum,BINT varnum)
{
    if(KM_MESSAGE(keymsg)==KM_LPRESS) {
        // ENTER MENU HELP MODE
        // KILL ANY PENDING POPUPS
        halScreen.HelpMode=(menunum<<16)|varnum;
        halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY;
        return;
    }

    if(KM_MESSAGE(keymsg)==KM_KEYUP) {
       if(halScreen.HelpMode) {
            halCancelPopup();
            halScreen.HelpMode=0;
            halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY|STAREA_DIRTY;
        }

        return;
    }

    // DEFAULT PRESS MESSAGE
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // ACTION WHEN IN THE STACK
                WORD mcode=rplGetMenuCode(menunum);
                WORDPTR menu=uiGetLibMenu(mcode);
                BINT nitems=uiCountMenuItems(mcode,menu);
                BINT idx=MENUPAGE(mcode)+varnum,page=MENUPAGE(mcode);

                rplSetLastMenu(menunum);

                if((nitems>6)&&(varnum==5)) {
                    // THIS IS THE NXT KEY
                    if( (KM_SHIFTPLANE(keymsg)==SHIFT_LS)||(KM_SHIFTPLANE(keymsg)==SHIFT_LSHOLD)) page-=5;
                    else page+=5;
                    if(page>=nitems) page=0;
                    if(page<=-5) {
                        page=nitems/5;
                        page*=5;
                        if(page==nitems) page-=5;
                    }
                    if(page<0) page=0;
                    rplSetMenuCode(menunum,SETMENUPAGE(mcode,page));
                    halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY;
                    return;
                }
                // THIS IS A REGULAR VAR KEY

                WORDPTR item=uiGetMenuItem(mcode,menu,idx);

                WORDPTR action=uiGetMenuItemAction(item,KM_SHIFTPLANE(keymsg));
                WORD Opcode=0;
                BINT hideargument=1;

                if(!action) return;

                switch(KM_SHIFTPLANE(keymsg))
                {
                case SHIFT_LS:
                case SHIFT_LSHOLD:
                {
                    // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

                    if(ISIDENT(*action)) {
                        // USER IS TRYING TO 'STO' INTO THE VARIABLE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=CMD_STO;
                        break;
                    }
                    if(ISUNIT(*action)) {
                        // FOR UNITS, TRY TO CONVERT
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=CMD_CONVERT;
                        break;
                    }

                    // ALL OTHER OBJECTS AND COMMANDS, DO XEQ
                    rplPushData(action);
                    Opcode=(CMD_OVR_XEQ);
                    break;

                }
                case SHIFT_RS:
                case SHIFT_RSHOLD:
                {
                    // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

                    if(ISIDENT(*action)) {
                        // USER IS TRYING TO 'RCL' THE VARIABLE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=CMD_RCL;
                        break;
                    }
                    if(ISUNIT(*action)) {
                        // FOR UNITS, APPLY THE INVERSE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=(CMD_OVR_DIV);
                        break;
                    }

                    // ALL OTHER OBJECTS AND COMMANDS, DO XEQ
                    rplPushData(action);
                    Opcode=(CMD_OVR_XEQ);
                    break;

                }
                default:
                {
                    // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

                    if(ISIDENT(*action)) {
                        // JUST EVAL THE VARIABLE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=(CMD_OVR_EVAL1);
                        break;
                    }
                    if(ISUNIT(*action)) {
                        // FOR UNITS, APPLY THE INVERSE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=(CMD_OVR_MUL);
                        break;
                    }

                    // ALL OTHER OBJECTS AND COMMANDS, DO XEQ
                    rplPushData(action);
                    Opcode=(CMD_OVR_XEQ);
                    break;


                }
                }

                if(Opcode) uiCmdRunHide(Opcode,hideargument);
                if(Exceptions) {
                    // TODO: SHOW ERROR MESSAGE
                    halShowErrorMsg();
                    Exceptions=0;
                } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY;
            halScreen.DirtyFlag|=STACK_DIRTY|STAREA_DIRTY;
        }

    }
    else {
        // ACTION INSIDE THE EDITOR
        WORD mcode=rplGetMenuCode(menunum);
        WORDPTR menu=uiGetLibMenu(mcode);
        BINT nitems=uiCountMenuItems(mcode,menu);
        BINT idx=MENUPAGE(mcode)+varnum,page=MENUPAGE(mcode);

        rplSetLastMenu(menunum);


        if((nitems>6)&&(varnum==5)) {
            // THIS IS THE NXT KEY
            if( (KM_SHIFTPLANE(keymsg)==SHIFT_LS)||(KM_SHIFTPLANE(keymsg)==SHIFT_LSHOLD)) page-=5;
            else page+=5;
            if(page>=nitems) page=0;
            if(page<=-5) {
                page=nitems/5;
                page*=5;
                if(page==nitems) page-=5;
            }
            if(page<0) page=0;
            rplSetMenuCode(menunum,SETMENUPAGE(mcode,page));
            halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY;
            return;
        }
        // THIS IS A REGULAR VAR KEY

        WORDPTR item=uiGetMenuItem(mcode,menu,idx);

        WORDPTR action=uiGetMenuItemAction(item,KM_SHIFTPLANE(keymsg));
        WORD Opcode=0;
        BINT hideargument=1;

        if(!action) return;

        switch(KM_SHIFTPLANE(keymsg))
        {
        case SHIFT_LS:
        case SHIFT_LSHOLD:
        {
            // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

            if(ISIDENT(*action)) {
                switch(halScreen.CursorState&0xff)
                {
                case 'D':
                case 'A':
                    if(endCmdLineAndCompile()) {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu=uiGetLibMenu(mcode);
                        item=uiGetMenuItem(mcode,menu,MENUPAGE(mcode)+varnum);
                        action=uiGetMenuItemAction(item,KM_SHIFTPLANE(keymsg));

                        // USER IS TRYING TO 'STO' INTO THE VARIABLE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=CMD_STO;
                    }
                    break;
                case 'P':
                    // USER IS TRYING TO 'STO' INTO THE VARIABLE
                    uiSeparateToken();
                    uiInsertCharacters((BYTEPTR)"'");
                    uiInsertCharactersN((BYTEPTR)(action+1),(BYTEPTR)(action+1)+rplGetIdentLength(action));
                    uiInsertCharacters((BYTEPTR)"' STO");
                    uiSeparateToken();
                    uiAutocompleteUpdate();
                    break;
                }
                break;
            }
            if(ISUNIT(*action)) {

                switch(halScreen.CursorState&0xff)
                {
                case 'D':
                    if(endCmdLineAndCompile()) {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu=uiGetLibMenu(mcode);
                        item=uiGetMenuItem(mcode,menu,MENUPAGE(mcode)+varnum);
                        action=uiGetMenuItemAction(item,KM_SHIFTPLANE(keymsg));

                        // USER IS TRYING TO CONVERT
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=CMD_CONVERT;
                    }
                    break;

                case 'A':
                case 'P':
                {

                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    uiSeparateToken();
                    uiInsertCharactersN(string,endstring);
                    uiSeparateToken();
                    uiInsertCharacters((BYTEPTR)"CONVERT");
                    uiSeparateToken();
                    uiAutocompleteUpdate();

                    break;
                }

                }
                break;
            }


            if(ISPROGRAM(*action)) {
                if(!ISSECO(*action)) {
                    // IT'S A DOCOL PROGRAM, EXECUTE TRANSPARENTLY
                    rplPushData(action);    // PUSH THE NAME ON THE STACK
                    Opcode=CMD_OVR_XEQ;
                    break;
                }
            }

            // ALL OTHER OBJECTS AND COMMANDS, DO XEQ AFTER ENDING THE COMMAND LINE
            if(endCmdLineAndCompile()) {
                // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                menu=uiGetLibMenu(mcode);
                item=uiGetMenuItem(mcode,menu,MENUPAGE(mcode)+varnum);
                action=uiGetMenuItemAction(item,KM_SHIFTPLANE(keymsg));

                // USER IS TRYING TO 'STO' INTO THE VARIABLE
                rplPushData(action);    // PUSH THE NAME ON THE STACK
                Opcode=CMD_OVR_XEQ;
                break;
            }
            break;

        }
        case SHIFT_RS:
        case SHIFT_RSHOLD:
        {
            // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

            if(ISIDENT(*action)) {
                switch(halScreen.CursorState&0xff)
                {
                case 'D':

                    if(KM_SHIFTPLANE(keymsg)&SHIFT_HOLD) {
                    //  DECOMPILE THE CONTENTS AND INSERT DIRECTLY INTO THE COMMAND LINE

                        WORDPTR *var=rplFindGlobal(action,1);
                        BYTEPTR string=0,endstring;

                        if(var) {

                        if(ISDIR(*var[1])) {
                            // VARIABLE IS A DIRECTORY, DON'T RCL
                            // BUT PUT THE NAME
                            string=(BYTEPTR)(action+1);
                            endstring=string+rplGetIdentLength(action);
                        }
                        else {

                        // VARIABLE EXISTS, GET THE CONTENTS
                        BINT SavedException=Exceptions;
                        BINT SavedErrorCode=ErrorCode;

                        Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                        // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                        WORDPTR opname=rplDecompile(var[1],DECOMP_EDIT);
                        Exceptions=SavedException;
                        ErrorCode=SavedErrorCode;

                        if(opname) {
                        BINT totaln=rplStrLen(opname);
                        string=(BYTEPTR) (opname+1);
                        endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);
                        }
                        }

                        if(string) {

                        uiSeparateToken();
                        uiInsertCharactersN(string,endstring);
                        uiSeparateToken();
                        uiAutocompleteUpdate();
                        }
                        }
                        break;

                    }


                    // NOT HOLD, JUST END THE COMMAND LINE AND RCL THE VARIABLE


                    if(endCmdLineAndCompile()) {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu=uiGetLibMenu(mcode);
                        item=uiGetMenuItem(mcode,menu,MENUPAGE(mcode)+varnum);
                        action=uiGetMenuItemAction(item,KM_SHIFTPLANE(keymsg));

                        // USER IS TRYING TO 'RCL' THE VARIABLE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=CMD_RCL;
                    }
                    break;
                case 'A':
                    // USER IS TRYING TO 'RCL' THE VARIABLE

                    if(KM_SHIFTPLANE(keymsg)&SHIFT_HOLD) {
                    //  DECOMPILE THE CONTENTS AND INSERT DIRECTLY INTO THE COMMAND LINE

                        WORDPTR *var=rplFindGlobal(action,1);
                        BYTEPTR string=0,endstring;

                        if(var) {

                        if(ISDIR(*var[1])) {
                            // VARIABLE IS A DIRECTORY, DON'T RCL
                            // BUT PUT THE NAME
                            string=(BYTEPTR)(action+1);
                            endstring=string+rplGetIdentLength(action);
                        }
                        else {


                        // VARIABLE EXISTS, GET THE CONTENTS
                        BINT SavedException=Exceptions;
                        BINT SavedErrorCode=ErrorCode;

                        Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                        // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                        WORDPTR opname=rplDecompile(var[1],DECOMP_EDIT);
                        Exceptions=SavedException;
                        ErrorCode=SavedErrorCode;

                        if(opname) {
                        string=(BYTEPTR) (opname+1);
                        BINT totaln=rplStrLen(opname);
                        endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                        // IN ALGEBRAIC MODE, REMOVE THE TICK MARKS AND INSERT WITHOUT SEPARATION
                        // TO ALLOW PASTING EQUATIONS INTO OTHER EXPRESSIONS
                        if( (totaln>2) && (string[0]=='\'')) {
                            string++;
                            endstring--;
                        }
                        }
                        }

                        if(string) {
                        uiInsertCharactersN(string,endstring);
                        uiAutocompleteUpdate();
                        }
                        }
                        break;


                    }
                    uiSeparateToken();
                    uiInsertCharacters((BYTEPTR)"'");
                    uiInsertCharactersN((BYTEPTR)(action+1),(BYTEPTR)(action+1)+rplGetIdentLength(action));
                    uiInsertCharacters((BYTEPTR)"' RCL");
                    uiSeparateToken();
                    uiAutocompleteUpdate();
                    break;

                case 'P':
                    // USER IS TRYING TO 'RCL' THE VARIABLE

                    if(KM_SHIFTPLANE(keymsg)&SHIFT_HOLD) {
                    //  DECOMPILE THE CONTENTS AND INSERT DIRECTLY INTO THE COMMAND LINE


                        WORDPTR *var=rplFindGlobal(action,1);

                        BYTEPTR string=0,endstring;

                        if(var) {

                        if(ISDIR(*var[1])) {
                            // VARIABLE IS A DIRECTORY, DON'T RCL
                            // BUT PUT THE NAME
                            string=(BYTEPTR)(action+1);
                            endstring=string+rplGetIdentLength(action);
                        }
                        else {
                        // VARIABLE EXISTS, GET THE CONTENTS
                        BINT SavedException=Exceptions;
                        BINT SavedErrorCode=ErrorCode;

                        Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                        // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                        WORDPTR opname=rplDecompile(var[1],DECOMP_EDIT);
                        Exceptions=SavedException;
                        ErrorCode=SavedErrorCode;

                        if(opname) {
                        string=(BYTEPTR) (opname+1);
                        BINT totaln=rplStrLen(opname);
                        endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                        }

                        }
                        if(string) {
                        uiSeparateToken();
                        uiInsertCharactersN(string,endstring);
                        uiSeparateToken();
                        uiAutocompleteUpdate();
                        }
                        }
                        break;


                    }
                    uiSeparateToken();
                    uiInsertCharacters((BYTEPTR)"'");
                    uiInsertCharactersN((BYTEPTR)(action+1),(BYTEPTR)(action+1)+rplGetIdentLength(action));
                    uiInsertCharacters((BYTEPTR)"' RCL");
                    uiSeparateToken();
                    uiAutocompleteUpdate();
                    break;
                }
                break;
            }
            if(ISUNIT(*action)) {

                switch(halScreen.CursorState&0xff)
                {
                case 'D':
                    if(endCmdLineAndCompile()) {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu=uiGetLibMenu(mcode);
                        item=uiGetMenuItem(mcode,menu,MENUPAGE(mcode)+varnum);
                        action=uiGetMenuItemAction(item,KM_SHIFTPLANE(keymsg));

                        // USER IS TRYING TO APPLY DIVIDING
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=(CMD_OVR_DIV);
                    }
                    break;

                case 'A':
                case 'P':
                {

                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    uiSeparateToken();
                    uiInsertCharactersN(string,endstring);
                    uiSeparateToken();
                    uiInsertCharacters((BYTEPTR)"/");
                    uiSeparateToken();
                    uiAutocompleteUpdate();

                    break;
                }

                }
                break;
            }

            if(ISPROGRAM(*action)) {
                if(!ISSECO(*action)) {
                    // IT'S A DOCOL PROGRAM, EXECUTE TRANSPARENTLY
                    rplPushData(action);    // PUSH THE NAME ON THE STACK
                    Opcode=CMD_OVR_XEQ;
                    break;
                }
            }

            // ALL OTHER OBJECTS AND COMMANDS, DO XEQ AFTER ENDING THE COMMAND LINE
            if(endCmdLineAndCompile()) {
                // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                menu=uiGetLibMenu(mcode);
                item=uiGetMenuItem(mcode,menu,MENUPAGE(mcode)+varnum);
                action=uiGetMenuItemAction(item,KM_SHIFTPLANE(keymsg));
                rplPushData(action);    // PUSH THE NAME ON THE STACK
                Opcode=CMD_OVR_XEQ;
            }

            break;
        }
        default:
        {
            // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

            if(ISIDENT(*action)) {
                switch(halScreen.CursorState&0xff)
                {
                case 'D':
                {
                    // HANDLE DIRECTORIES IN A SPECIAL WAY: DON'T CLOSE THE COMMAND LINE
                    WORDPTR *var=rplFindGlobal(action,1);
                    if(var) {
                        if(ISDIR(*(var[1]))) {
                            // CHANGE THE DIR WITHOUT CLOSING THE COMMAND LINE
                            rplPushData(action);    // PUSH THE NAME ON THE STACK
                            Opcode=(CMD_OVR_EVAL);
                            break;
                        }
                    }

                    if(endCmdLineAndCompile()) {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu=uiGetLibMenu(mcode);
                        item=uiGetMenuItem(mcode,menu,MENUPAGE(mcode)+varnum);
                        action=uiGetMenuItemAction(item,KM_SHIFTPLANE(keymsg));

                        // USER IS TRYING TO EVAL THE VARIABLE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=(CMD_OVR_EVAL);
                    }
                    break;
                }
                case 'A':
                {
                    WORDPTR *var=rplFindGlobal(action,1);
                    if(var) {
                        if(ISDIR(*(var[1]))) {
                            // CHANGE THE DIR WITHOUT CLOSING THE COMMAND LINE
                            rplPushData(action);    // PUSH THE NAME ON THE STACK
                            Opcode=(CMD_OVR_EVAL);
                            break;
                        }
                    }

                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    // REMOVE THE TICK MARKS IN ALG MODE
                    if((totaln>2)&&(string[0]=='\'')) {
                        ++string;
                        --endstring;
                    }

                    uiInsertCharactersN(string,endstring);
                    uiAutocompleteUpdate();

                    break;
                }

                case 'P':
                {
                    WORDPTR *var=rplFindGlobal(action,1);
                    if(var) {
                        if(ISDIR(*(var[1]))) {
                            // CHANGE THE DIR WITHOUT CLOSING THE COMMAND LINE
                            rplPushData(action);    // PUSH THE NAME ON THE STACK
                            Opcode=(CMD_OVR_EVAL);
                            break;
                        }
                    }

                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    // REMOVE THE TICK MARKS IN ALG MODE
                    if((totaln>2)&&(string[0]=='\'')) {
                        ++string;
                        --endstring;
                    }

                    uiSeparateToken();
                    uiInsertCharactersN(string,endstring);
                    uiSeparateToken();
                    uiAutocompleteUpdate();

                    break;
                }
                }
                break;

            }
            if(ISUNIT(*action)) {
                switch(halScreen.CursorState&0xff)
                {
                case 'D':
                    if(endCmdLineAndCompile()) {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu=uiGetLibMenu(mcode);
                        item=uiGetMenuItem(mcode,menu,MENUPAGE(mcode)+varnum);
                        action=uiGetMenuItemAction(item,KM_SHIFTPLANE(keymsg));

                        // USER IS TRYING TO APPLY THE UNIT
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=(CMD_OVR_MUL);
                    }
                    break;

                case 'A':
                {

                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    if( (totaln>2)&&(string[0]=='1')&&(string[1]=='_')) string+=2;

                    uiInsertCharactersN(string,endstring);
                    uiAutocompleteUpdate();

                    break;
                }

                case 'P':
                {

                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    uiSeparateToken();
                    uiInsertCharactersN(string,endstring);
                    uiSeparateToken();
                    uiInsertCharacters((BYTEPTR)"*");
                    uiSeparateToken();
                    uiAutocompleteUpdate();

                    break;
                }

                }
                break;
            }
            if(!ISPROLOG(*action)) {
                // THIS IS A COMMAND, DECOMPILE AND INSERT NAME
                switch(halScreen.CursorState&0xff)
                {
                case 'D':
                    if(endCmdLineAndCompile()) {
                        // FIND THE COMMAND AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu=uiGetLibMenu(mcode);
                        item=uiGetMenuItem(mcode,menu,MENUPAGE(mcode)+varnum);
                        action=uiGetMenuItemAction(item,KM_SHIFTPLANE(keymsg));

                        Opcode=*action;
                        hideargument=0;
                    }
                    break;

                case 'A':
                {

                    WORD tokeninfo=0;
                    LIBHANDLER han=rplGetLibHandler(LIBNUM(*action));


                    // GET THE SYMBOLIC TOKEN INFORMATION
                    if(han) {
                        WORD savecurOpcode=CurOpcode;
                        DecompileObject=action;
                        CurOpcode=MKOPCODE(LIBNUM(*action),OPCODE_GETINFO);
                        (*han)();

                        if(RetNum>OK_TOKENINFO) tokeninfo=RetNum;

                        CurOpcode=savecurOpcode;
                    }


                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    uiInsertCharactersN(string,endstring);
                    if(TI_TYPE(tokeninfo)==TITYPE_FUNCTION) {
                        uiInsertCharacters((BYTEPTR)"()");
                        uiCursorLeft(1);
                    }
                    uiAutocompleteUpdate();

                    break;
                }

                case 'P':
                {

                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    uiSeparateToken();
                    uiInsertCharactersN(string,endstring);
                    uiSeparateToken();
                    uiAutocompleteUpdate();

                    break;
                }

                }
                break;




            }

            if(ISPROGRAM(*action)) {
                if(!ISSECO(*action)) {
                    // IT'S A DOCOL PROGRAM, EXECUTE TRANSPARENTLY
                    rplPushData(action);    // PUSH THE NAME ON THE STACK
                    Opcode=CMD_OVR_XEQ;
                }
                else {
                    if(endCmdLineAndCompile()) {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu=uiGetLibMenu(mcode);
                        item=uiGetMenuItem(mcode,menu,MENUPAGE(mcode)+varnum);
                        action=uiGetMenuItemAction(item,KM_SHIFTPLANE(keymsg));
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=CMD_OVR_XEQ;
                    }
                }
                break;
            }


            // ALL OTHER OBJECTS AND COMMANDS
            switch(halScreen.CursorState&0xff)
            {
            case 'D':
                if(endCmdLineAndCompile()) {
                    // FIND THE COMMAND AGAIN, IT MIGHT'VE MOVED DUE TO GC
                    menu=uiGetLibMenu(mcode);
                    item=uiGetMenuItem(mcode,menu,MENUPAGE(mcode)+varnum);
                    action=uiGetMenuItemAction(item,KM_SHIFTPLANE(keymsg));
                    if(!ISPROLOG(*action)) { Opcode=*action; hideargument=0; }// RUN COMMANDS DIRECTLY
                    else {
                        Opcode=(CMD_OVR_XEQ);
                        rplPushData(action);
                    }
                }
                break;

            case 'A':
            {

                WORD tokeninfo=0;
                LIBHANDLER han=rplGetLibHandler(LIBNUM(*action));


                // GET THE SYMBOLIC TOKEN INFORMATION
                if(han) {
                    WORD savecurOpcode=CurOpcode;
                    DecompileObject=action;
                    CurOpcode=MKOPCODE(LIBNUM(*action),OPCODE_GETINFO);
                    (*han)();

                    if(RetNum>OK_TOKENINFO) tokeninfo=RetNum;

                    CurOpcode=savecurOpcode;
                }


                // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                BINT SavedException=Exceptions;
                BINT SavedErrorCode=ErrorCode;

                Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                Exceptions=SavedException;
                ErrorCode=SavedErrorCode;

                if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                BYTEPTR string=(BYTEPTR) (opname+1);
                BINT totaln=rplStrLen(opname);
                BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                uiInsertCharactersN(string,endstring);
                if(TI_TYPE(tokeninfo)==TITYPE_FUNCTION) {
                    uiInsertCharacters((BYTEPTR)"()");
                    uiCursorLeft(1);
                }
                uiAutocompleteUpdate();

                break;
            }

            case 'P':
            {

                // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                BINT SavedException=Exceptions;
                BINT SavedErrorCode=ErrorCode;

                Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                Exceptions=SavedException;
                ErrorCode=SavedErrorCode;

                if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                BYTEPTR string=(BYTEPTR) (opname+1);
                BINT totaln=rplStrLen(opname);
                BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                uiSeparateToken();
                uiInsertCharactersN(string,endstring);
                uiSeparateToken();
                uiAutocompleteUpdate();

                break;
            }

            }


            break;


        }
        }

        if(Opcode) uiCmdRunHide(Opcode,hideargument);
        if(Exceptions) {
            // TODO: SHOW ERROR MESSAGE
            halShowErrorMsg();
            Exceptions=0;
        } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY;
        halScreen.DirtyFlag|=STACK_DIRTY|STAREA_DIRTY;
}

}







void symbolKeyHandler(BINT keymsg,BYTEPTR symbol,BINT separate)
{
if(!(halGetContext()&CONTEXT_INEDITOR)) {
    halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight+2);
    halSetContext(halGetContext()|CONTEXT_INEDITOR);
    if(KM_SHIFTPLANE(keymsg)&SHIFT_ALPHA) uiOpenCmdLine('X');
    else uiOpenCmdLine('D');
    }

    if(separate && ((halScreen.CursorState&0xff)=='P')) uiSeparateToken();

    uiInsertCharacters(symbol);

    if(separate && ((halScreen.CursorState&0xff)=='P')) uiSeparateToken();
    uiAutocompleteUpdate();

}

void alphasymbolKeyHandler(BINT keymsg,BYTEPTR Lsymbol,BYTEPTR Csymbol)
{
if(!(halGetContext()&CONTEXT_INEDITOR)) {
    halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight+2);
    halSetContext(halGetContext()|CONTEXT_INEDITOR);
    if(KM_SHIFTPLANE(keymsg)&SHIFT_ALPHA) uiOpenCmdLine('X');
    else uiOpenCmdLine('D');
    }

    if(halGetCmdLineMode()=='L') uiInsertCharacters(Lsymbol);
    if(halGetCmdLineMode()=='C') uiInsertCharacters(Csymbol);
    uiAutocompleteUpdate();

}



void VarMenuKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    // SIMPLY TOGGLE THE MENU UPON PRESS
    if(halScreen.Menu2) halSetMenu2Height(0);
    else halSetMenu2Height(MENU2_HEIGHT);

}


void newlineKeyHandler(BINT keymsg)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight+2);
        halSetContext(halGetContext()|CONTEXT_INEDITOR);
        if(KM_SHIFTPLANE(keymsg)&SHIFT_ALPHA) uiOpenCmdLine('X');
        else uiOpenCmdLine('D');

        }

    // INCREASE THE HEIGHT ON-SCREEN UP TO THE MAXIMUM
    uiStretchCmdLine(1);

    // ADD A NEW LINE
    uiInsertCharacters((BYTEPTR)"\n");

    uiAutocompleteUpdate();

}


void decimaldotKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight+2);
        halSetContext(halGetContext()|CONTEXT_INEDITOR);
        if(KM_SHIFTPLANE(keymsg)&SHIFT_ALPHA) uiOpenCmdLine('X');
        else uiOpenCmdLine('D');
        }

        UBINT64 Locale=rplGetSystemLocale();

        WORD ucode=cp2utf8(DECIMAL_DOT(Locale));
        if(ucode&0xff000000) uiInsertCharactersN((BYTEPTR)&ucode,((BYTEPTR)&ucode)+4);
        else uiInsertCharacters((BYTEPTR)&ucode);

        uiAutocompleteUpdate();


}

void  enterKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        // PERFORM DUP
        if(halGetContext()&CONTEXT_STACK) {
            // PERFORM DUP ONLY IF THERE'S DATA ON THE STACK
            // DON'T ERROR IF STACK IS EMPTY
            if(rplDepthData()>0) uiCmdRun(CMD_DUP);
            halScreen.DirtyFlag|=STACK_DIRTY;
        }

        }
    else{
        // ENABLE UNDO
        if(halScreen.StkCurrentLevel!=1) rplTakeSnapshot();
        halScreen.StkCurrentLevel=0;


     if(endCmdLineAndCompile()) {
         halScreen.DirtyFlag|=STACK_DIRTY|MENU1_DIRTY|MENU2_DIRTY|STAREA_DIRTY;
         rplRemoveSnapshot(halScreen.StkUndolevels+2);
         rplRemoveSnapshot(halScreen.StkUndolevels+1);

     }
     else  {
         // SOMETHING WENT WRONG DURING COMPILE, STACK DIDN'T CHANGE
         rplRemoveSnapshot(1);

     }

   }
}


void cancelKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(halGetNotification(N_RIGHTSHIFT)) {
      // SHIFT-ON MEANS POWER OFF!
       halPreparePowerOff();
       halEnterPowerOff();
       return;

    }

    if((halGetContext()&CONTEXT_INEDITOR)) {
        // END THE COMMAND LINE
     endCmdLine();
   }
}




void cutclipKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // ACTION WHEN IN THE STACK
                uiCmdRunTransparent(CMD_CUTCLIP,1,1);
                if(Exceptions) {
                    // TODO: SHOW ERROR MESSAGE
                    halShowErrorMsg();
                    Exceptions=0;
                } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY;
            halScreen.DirtyFlag|=STACK_DIRTY;
        }

    }
    else {
        // ACTION INSIDE THE EDITOR
            WORDPTR string=uiExtractSelection();

            if(string) {
                rplPushData(string);
                uiCmdRunTransparent(CMD_CUTCLIP,1,0);
            if(Exceptions) {
                // TODO: SHOW ERROR MESSAGE
                halShowErrorMsg();
                Exceptions=0;
            } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY;

            uiDeleteSelection();
        halScreen.DirtyFlag|=STACK_DIRTY;
    }


    }
}

void copyclipKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // ACTION WHEN IN THE STACK
                uiCmdRunTransparent(CMD_COPYCLIP,1,1);
                if(Exceptions) {
                    // TODO: SHOW ERROR MESSAGE
                    halShowErrorMsg();
                    Exceptions=0;
                } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY;
            halScreen.DirtyFlag|=STACK_DIRTY;
        }

    }
    else {
        // ACTION INSIDE THE EDITOR
            WORDPTR string=uiExtractSelection();

            if(string) {
                rplPushData(string);
                uiCmdRunTransparent(CMD_CUTCLIP,1,0);
            if(Exceptions) {
                // TODO: SHOW ERROR MESSAGE
                halShowErrorMsg();
                Exceptions=0;
            } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY;
        halScreen.DirtyFlag|=STACK_DIRTY;
    }


    }
}

void pasteclipKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // ACTION WHEN IN THE STACK
                uiCmdRunTransparent(CMD_PASTECLIP,0,1);
                if(Exceptions) {
                    halShowErrorMsg();
                    Exceptions=0;
                } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY;
            halScreen.DirtyFlag|=STACK_DIRTY;
        }

    }
    else {
        // ACTION INSIDE THE EDITOR
                if(uiCmdRunTransparent(CMD_PASTECLIP,0,1)==1) {
                    WORDPTR object=rplPopData(1);
                    if(!ISSTRING(*object)) {
                        object=rplDecompile(object,DECOMP_EDIT);
                        if(!object || Exceptions) {
                            halShowErrorMsg();
                            Exceptions=0;
                            return;
                        }
                        if(((halScreen.CursorState&0xff)=='P')||((halScreen.CursorState&0xff)=='D')) uiSeparateToken();
                    }


                    uiInsertCharactersN((BYTEPTR)(object+1),(BYTEPTR)(object+1)+rplStrSize(object));

                }


    }
}





void backspKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        // DO DROP
        if(halGetContext()&CONTEXT_STACK) {
            // PERFORM DROP ONLY IF THERE'S DATA ON THE STACK
            // DON'T ERROR IF STACK IS EMPTY
            if(rplDepthData()>0) uiCmdRun(CMD_DROP);
            halScreen.DirtyFlag|=STACK_DIRTY;
        }

    }
    else{
        // REMOVE CHARACTERS FROM THE COMMAND LINE
        // TODO: IMPLEMENT THIS!
        uiCursorLeft(1);
        uiRemoveCharacters(1);
        uiAutocompleteUpdate();
    }
}

void deleteKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if((halGetContext()&CONTEXT_INEDITOR)) {
        // REMOVE CHARACTERS FROM THE COMMAND LINE
        uiRemoveCharacters(1);
    }
}



void leftKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // PERFORM UNDO IN THE STACK
            uiStackUndo();
            halScreen.DirtyFlag|=STACK_DIRTY|STAREA_DIRTY;
            return;

        }

    }
    else{
        uiCursorLeft(1);
        uiAutocompleteUpdate();
    }
}


void rsleftKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // REDO ACTION
            uiStackRedo();
            halScreen.DirtyFlag|=STACK_DIRTY|STAREA_DIRTY;
            return;
        }

    }
    else{
        uiCursorStartOfLine();
        uiAutocompleteUpdate();
    }
}

void rsholdleftKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // REDO ACTION
            uiStackRedo();
            halScreen.DirtyFlag|=STACK_DIRTY|STAREA_DIRTY;
            return;

        }

    }
    else{
        uiCursorPageLeft();
        uiAutocompleteUpdate();
    }
}


void lsleftKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // TODO: WHAT TO DO WITH RS-LEFT CURSOR??
            // THIS SHOULD SCROLL A LARGE OBJECT IN LEVEL 1

        }

    }
    else {
        uiSetSelectionStart();
    }
}






void rightKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {

            if(rplDepthData()>1) {
                uiCmdRun(CMD_SWAP);
            halScreen.DirtyFlag|=STACK_DIRTY;
            }

        }

    }
    else{
        uiCursorRight(1);
        uiAutocompleteUpdate();
    }
}


void rsrightKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // TODO: WHAT TO DO WITH RS-RIGHT CURSOR??
            // THIS SHOULD SCROLL A LARGE OBJECT IN LEVEL 1

        }

    }
    else{
        uiCursorEndOfLine();
        uiAutocompleteUpdate();
    }
}

void lsrightKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // NOT SURE WHAT TO DO WITH THIS KEY

        }

    }
    else {
        // IN THE EDITOR, DO SELECTION
        uiSetSelectionEnd();
    }
}




void rsholdrightKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // TODO: WHAT TO DO WITH RS-RIGHT CURSOR??
            // THIS SHOULD SCROLL A LARGE OBJECT IN LEVEL 1

        }

    }
    else{
        uiCursorPageRight();
        uiAutocompleteUpdate();
    }
}

void alphaholdrightKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // TODO: ??
            }
        // TODO: ADD OTHER CONTEXTS HERE
    }

    else {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiAutocompInsert();
        uiAutocompleteUpdate();
    }
}

void downKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {

            if(rplDepthData()>=1) {
                WORDPTR ptr=rplPeekData(1);
                WORDPTR text=rplDecompile(ptr,DECOMP_EDIT);
                if(Exceptions) {
                    halShowErrorMsg();
                    Exceptions=0;
                    return;
                }
                BYTE cursorstart='D';

                if(ISPROGRAM(*ptr)) cursorstart='P';
                if(ISSYMBOLIC(*ptr)) cursorstart='A';
                if(ISUNIT(*ptr)) cursorstart='A';
                if(ISLIST(*ptr)) cursorstart='P';

                // OPEN THE COMMAND LINE
                halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight+2);
                halSetContext(halGetContext()|CONTEXT_INEDITOR);
                if(KM_SHIFTPLANE(keymsg)&SHIFT_ALPHA) uiOpenCmdLine('X');
                else uiOpenCmdLine(cursorstart);
                uiSetCmdLineText(text);
                uiSetCmdLineState(uiGetCmdLineState()|CMDSTATE_OVERWRITE);
                return;
                }

            }
        // TODO: ADD OTHER CONTEXTS HERE
    }

    else {
        // GO DOWN ONE LINE IN MULTILINE TEXT EDITOR
        uiCursorDown(1);
        uiAutocompleteUpdate();
    }
}


void rsholddownKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // TODO: ??
            }
        // TODO: ADD OTHER CONTEXTS HERE
    }

    else {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiCursorPageDown();
        uiAutocompleteUpdate();
    }
}

void rsdownKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // TODO: ??
            }
        // TODO: ADD OTHER CONTEXTS HERE
    }

    else {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiCursorEndOfText();
        uiAutocompleteUpdate();
    }
}

void alphaholddownKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // TODO: ??
            }
        // TODO: ADD OTHER CONTEXTS HERE
    }

    else {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiAutocompNext();
    }
}


void upKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // TODO: START INTERACTIVE STACK MANIPULATION HERE
            }
        // TODO: ADD OTHER CONTEXTS HERE
    }

    else {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiCursorUp(1);
        uiAutocompleteUpdate();
    }
}


void rsholdupKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // TODO: START INTERACTIVE STACK MANIPULATION HERE
            }
        // TODO: ADD OTHER CONTEXTS HERE
    }

    else {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiCursorPageUp();
        uiAutocompleteUpdate();
    }
}

void rsupKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // TODO: START INTERACTIVE STACK MANIPULATION HERE
            }
        // TODO: ADD OTHER CONTEXTS HERE
    }

    else {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiCursorStartOfText();
        uiAutocompleteUpdate();
    }
}


void alphaholdupKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // TODO: ??
            }
        // TODO: ADD OTHER CONTEXTS HERE
    }

    else {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiAutocompPrev();
    }
}



void chsKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            // ACTION WHEN IN THE STACK
                uiCmdRun((CMD_OVR_NEG));
                if(Exceptions) {
                    // TODO: SHOW ERROR MESSAGE
                    halShowErrorMsg();
                    Exceptions=0;
                }
            halScreen.DirtyFlag|=STACK_DIRTY;
        }

    }
    else{
        // ACTION INSIDE THE EDITOR

        BYTEPTR startnum;
        BYTEPTR line=(BYTEPTR)(CmdLineCurrentLine+1);

        // FIRST CASE: IF TOKEN UNDER THE CURSOR IS OR CONTAINS A VALID NUMBER, CHANGE THE SIGN OF THE NUMBER IN THE TEXT
        startnum=uiFindNumberStart();
        if(!startnum) {
            startnum=line+halScreen.CursorPosition;
            if(startnum>line) {
            if(startnum[-1]=='+') { uiRemoveCharacters(1); uiInsertCharacters((BYTEPTR)"-"); halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY; return; }
            if(startnum[-1]=='-') { uiRemoveCharacters(1); uiInsertCharacters((BYTEPTR)"+"); halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY; return; }
            if((startnum[-1]=='E')||(startnum[-1]=='e') ) { uiInsertCharacters((BYTEPTR)"-"); uiAutocompleteUpdate(); return; }


            }

            // SECOND CASE: IF TOKEN UNDER CURSOR IS EMPTY, IN 'D' MODE COMPILE OBJECT AND THEN EXECUTE NEG

            if((halScreen.CursorState&0xff)=='D') {
            // COMPILE AND EXECUTE NEG
            if(endCmdLineAndCompile()) {
            uiCmdRun((CMD_OVR_NEG));
            if(Exceptions) {
                // TODO: SHOW ERROR MESSAGE
                halShowErrorMsg();
                Exceptions=0;
            }
        halScreen.DirtyFlag|=STACK_DIRTY;
            }

            return;
            }

            if((halScreen.CursorState&0xff)=='P') {
                uiSeparateToken();
                uiInsertCharacters((BYTEPTR)"NEG");
                uiSeparateToken();
                uiAutocompleteUpdate();
            return;
            }

            if((halScreen.CursorState&0xff)=='A') {
                uiInsertCharacters((BYTEPTR)"-");
                uiAutocompleteUpdate();
            return;
            }



        }
        else {
            // WE FOUND A NUMBER
            BINT oldposition=halScreen.CursorPosition;
            uiMoveCursor(startnum-line);
            BYTEPTR plusminus=(BYTEPTR)"-";

            if(startnum>line) {
            if(startnum[-1]=='+') { uiMoveCursor(startnum-line-1); uiRemoveCharacters(1); --oldposition; }
            if(startnum[-1]=='-') { uiMoveCursor(startnum-line-1); uiRemoveCharacters(1); plusminus=(BYTEPTR)"+"; -- oldposition; }
            }

            // NEED TO INSERT A CHARACTER HERE
            uiInsertCharacters(plusminus);
            uiMoveCursor(oldposition+1);
            uiEnsureCursorVisible();
            uiAutocompleteUpdate();
            return;
      }

        // THIRD CASE: IF TOKEN UNDER CURSOR IS SOMETHING OTHER THAN A NUMBER, JUST INSERT A MINUS SIGN

    }
}

void eexKeyHandler(BINT keymsg)
{

    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()&CONTEXT_STACK) {
            halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight+2);
            halSetContext(halGetContext()|CONTEXT_INEDITOR);
            if(KM_SHIFTPLANE(keymsg)&SHIFT_ALPHA) uiOpenCmdLine('X');
            else uiOpenCmdLine('D');
            NUMFORMAT config;

            rplGetSystemNumberFormat(&config);
            if((config.MiddleFmt|config.BigFmt|config.SmallFmt)&FMT_USECAPITALS) uiInsertCharacters((BYTEPTR)"1E");
            else uiInsertCharacters((BYTEPTR)"1e");
            uiAutocompleteUpdate();
            return;
        }

    }
    else{
        // ACTION INSIDE THE EDITOR

        // FIRST CASE: IF TOKEN UNDER THE CURSOR IS OR CONTAINS A VALID NUMBER
        BYTEPTR startnum;
        NUMFORMAT config;

        rplGetSystemNumberFormat(&config);


        startnum=uiFindNumberStart();

        BYTEPTR line=(BYTEPTR)(CmdLineCurrentLine+1);

        if(!startnum) {
            startnum=line+halScreen.CursorPosition;
            // DO NOTHING IF THERE'S ALREADY AN 'E' BEFORE THE CURSOR
            if((startnum>line) && ((startnum[-1]=='E')||(startnum[-1]=='e') )) return;

            // SECOND CASE: IF TOKEN UNDER CURSOR IS EMPTY, IN 'D' MODE COMPILE OBJECT AND THEN APPEND 1E
            if((config.MiddleFmt|config.BigFmt|config.SmallFmt)&FMT_USECAPITALS)    uiInsertCharacters((BYTEPTR)"1E");
            else uiInsertCharacters((BYTEPTR)"1e");
            uiAutocompleteUpdate();
            return;
        }
        else {
            // WE FOUND A NUMBER
            if((startnum>line)&&((startnum[-1]=='-')||(startnum[-1]=='+'))) --startnum;

            if((startnum>line) && ((startnum[-1]=='E')||(startnum[-1]=='e') )) {
                uiMoveCursor(startnum-line);
                // TODO: SELECT THE EXISTING NUMBER FOR DELETION ON NEXT KEYPRESS
                uiEnsureCursorVisible();
                uiAutocompleteUpdate();
                return;
            }

            // NEED TO INSERT A CHARACTER HERE
            BINT oldposition=halScreen.CursorPosition;
            if((config.MiddleFmt|config.BigFmt|config.SmallFmt)&FMT_USECAPITALS) uiInsertCharacters((BYTEPTR)"E");
            else uiInsertCharacters((BYTEPTR)"e");
            uiMoveCursor(oldposition+1);
            uiEnsureCursorVisible();
            uiAutocompleteUpdate();
            return;
      }

    }
}

// COMMON FUNCTION FOR AL "BRACKET TYPES"
void bracketKeyHandler(BINT keymsg,BYTEPTR string)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight+2);
        halSetContext(halGetContext()|CONTEXT_INEDITOR);
        if(KM_SHIFTPLANE(keymsg)&SHIFT_ALPHA) uiOpenCmdLine('X');
        else uiOpenCmdLine('D');
        }
    if(((halScreen.CursorState&0xff)=='D')||((halScreen.CursorState&0xff)=='P')) uiSeparateToken();

    BYTEPTR end=string+stringlen((char *)string);
    uiInsertCharactersN(string,end);
    uiCursorLeft(utf8nlen((char *)string,(char *)end)>>1);
    uiAutocompleteUpdate();

}

void curlyBracketKeyHandler(BINT keymsg)
{
    bracketKeyHandler(keymsg,(BYTEPTR)"{  }");

    if( (halGetCmdLineMode()!='L')&&(halGetCmdLineMode()!='C'))
        halSetCmdLineMode('P');

}
void squareBracketKeyHandler(BINT keymsg)
{
    bracketKeyHandler(keymsg,(BYTEPTR)"[  ]");

}
void secoBracketKeyHandler(BINT keymsg)
{
    bracketKeyHandler(keymsg,(BYTEPTR)"«  »");

    if( (halGetCmdLineMode()!='L')&&(halGetCmdLineMode()!='C'))
        halSetCmdLineMode('P');

}
void parenBracketKeyHandler(BINT keymsg)
{
    bracketKeyHandler(keymsg,(BYTEPTR)"()");

}
void textBracketKeyHandler(BINT keymsg)
{
    bracketKeyHandler(keymsg,(BYTEPTR)"\"\"");

    //  LOCK ALPHA MODE
    if( (halGetCmdLineMode()!='L')&&(halGetCmdLineMode()!='C'))
        keyb_setshiftplane(0,0,1,1);


}

void ticksKeyHandler(BINT keymsg)
{
    bracketKeyHandler(keymsg,(BYTEPTR)"''");
    // GO INTO ALGEBRAIC MODE
    if( (halGetCmdLineMode()!='L')&&(halGetCmdLineMode()!='C'))
    halSetCmdLineMode('A');

}

void tagKeyHandler(BINT keymsg)
{
    bracketKeyHandler(keymsg,(BYTEPTR)"::");
    //  LOCK ALPHA MODE
    if( (halGetCmdLineMode()!='L')&&(halGetCmdLineMode()!='C'))
        keyb_setshiftplane(0,0,1,1);

}


void onPlusKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    halStatusAreaPopup();


// INCREASE CONTRAST
DRAWSURFACE scr;
ggl_initscr(&scr);
int ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
// CLEAR STATUS AREA
ggl_rect(&scr,STATUSAREA_X,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);

int j;
for(j=0;j<15;++j) {
    ggl_rect(&scr,STATUSAREA_X+1+3*j,ytop+7,STATUSAREA_X+1+3*j+2,ytop+12,ggl_mkcolor(j));
    ggl_rect(&scr,STATUSAREA_X+1+3*j,ytop,STATUSAREA_X+1+3*j+2,ytop+5,ggl_mkcolor(15-j));
}


__lcd_contrast++;
if(__lcd_contrast>0xf) __lcd_contrast=0xf;

lcd_setcontrast(__lcd_contrast);

}

void onMinusKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    halStatusAreaPopup();

// DECREASE CONTRAST
DRAWSURFACE scr;
ggl_initscr(&scr);
int ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
// CLEAR STATUS AREA
ggl_rect(&scr,STATUSAREA_X,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);

int j;
for(j=0;j<15;++j) {
    ggl_rect(&scr,STATUSAREA_X+1+3*j,ytop+7,STATUSAREA_X+1+3*j+2,ytop+12,ggl_mkcolor(j));
    ggl_rect(&scr,STATUSAREA_X+1+3*j,ytop,STATUSAREA_X+1+3*j+2,ytop+5,ggl_mkcolor(15-j));
}


__lcd_contrast--;
if(__lcd_contrast<0) __lcd_contrast=0;
lcd_setcontrast(__lcd_contrast);
}


void onDotKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

// CYCLE BETWEEN VARIOUS OPTIONS
    const char * const options[]={
        "1000.000000","1,000.000000","1 000.000000","1000.000 000","1,000.000 000","1 000.000 000",
        "1000,000000","1.000,000000","1 000,000000","1000,000 000","1.000,000 000","1 000,000 000"
    };

    NUMFORMAT fmt;
    BINT option=0;
    rplGetSystemNumberFormat(&fmt);
    if(DECIMAL_DOT(fmt.Locale)==',') option+=6;
    if(fmt.MiddleFmt&FMT_NUMSEPARATOR) {
        if(THOUSAND_SEP(fmt.Locale)==',') option+=1;
        if(THOUSAND_SEP(fmt.Locale)=='.') option+=1;
        if(THOUSAND_SEP(fmt.Locale)==THIN_SPACE) option+=2;
    }
    if(fmt.MiddleFmt&FMT_FRACSEPARATOR) option+=3;


    // CYCLE THROUGH ITEMS:
    ++option;
    if(option>11) option=0;

    halStatusAreaPopup();

DRAWSURFACE scr;
ggl_initscr(&scr);
int ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
// CLEAR STATUS AREA
ggl_rect(&scr,STATUSAREA_X,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);

DrawTextBk(STATUSAREA_X+1,ytop+1,"Format:",halScreen.StAreaFont,0xf,0,&scr);
DrawTextBk(STATUSAREA_X+1,ytop+1+halScreen.StAreaFont->BitmapHeight,(char *)options[option],halScreen.StAreaFont,0xf,0,&scr);


// CHANGE THE FORMAT TO THE SELECTED OPTION
switch(option)
{
default:
case 0:
    fmt.BigFmt&=~(FMT_NUMSEPARATOR|FMT_FRACSEPARATOR);
    fmt.SmallFmt&=~(FMT_NUMSEPARATOR|FMT_FRACSEPARATOR);
    fmt.MiddleFmt&=~(FMT_NUMSEPARATOR|FMT_FRACSEPARATOR);
    fmt.Locale=MAKELOCALE('.',THIN_SPACE,THIN_SPACE,',');
    break;
case 1:
    fmt.BigFmt&=~(FMT_FRACSEPARATOR);
    fmt.BigFmt|=FMT_NUMSEPARATOR;
    fmt.SmallFmt&=~(FMT_FRACSEPARATOR);
    fmt.SmallFmt|=FMT_NUMSEPARATOR;
    fmt.MiddleFmt&=~(FMT_FRACSEPARATOR);
    fmt.MiddleFmt|=FMT_NUMSEPARATOR;
    fmt.Locale=MAKELOCALE('.',',',THIN_SPACE,';');
    break;
case 2:
    fmt.BigFmt&=~(FMT_FRACSEPARATOR);
    fmt.BigFmt|=FMT_NUMSEPARATOR;
    fmt.SmallFmt&=~(FMT_FRACSEPARATOR);
    fmt.SmallFmt|=FMT_NUMSEPARATOR;

    fmt.MiddleFmt&=~(FMT_FRACSEPARATOR);
    fmt.MiddleFmt|=FMT_NUMSEPARATOR;
    fmt.Locale=MAKELOCALE('.',THIN_SPACE,THIN_SPACE,',');
    break;
case 3:
    fmt.BigFmt&=~FMT_NUMSEPARATOR;
    fmt.BigFmt|=FMT_FRACSEPARATOR;
    fmt.SmallFmt&=~FMT_NUMSEPARATOR;
    fmt.SmallFmt|=FMT_FRACSEPARATOR;
    fmt.MiddleFmt&=~FMT_NUMSEPARATOR;
    fmt.MiddleFmt|=FMT_FRACSEPARATOR;
    fmt.Locale=MAKELOCALE('.',THIN_SPACE,THIN_SPACE,',');
    break;
case 4:
    fmt.BigFmt|=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR;
    fmt.SmallFmt|=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR;
    fmt.MiddleFmt|=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR;
    fmt.Locale=MAKELOCALE('.',',',THIN_SPACE,';');
    break;
case 5:
    fmt.BigFmt|=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR;
    fmt.SmallFmt|=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR;
    fmt.MiddleFmt|=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR;
    fmt.Locale=MAKELOCALE('.',THIN_SPACE,THIN_SPACE,',');
    break;
case 6:
    fmt.BigFmt&=~(FMT_NUMSEPARATOR|FMT_FRACSEPARATOR);
    fmt.SmallFmt&=~(FMT_NUMSEPARATOR|FMT_FRACSEPARATOR);
    fmt.MiddleFmt&=~(FMT_NUMSEPARATOR|FMT_FRACSEPARATOR);
    fmt.Locale=MAKELOCALE(',',THIN_SPACE,THIN_SPACE,';');
    break;
case 7:
    fmt.BigFmt&=~(FMT_FRACSEPARATOR);
    fmt.BigFmt|=FMT_NUMSEPARATOR;
    fmt.SmallFmt&=~(FMT_FRACSEPARATOR);
    fmt.SmallFmt|=FMT_NUMSEPARATOR;
    fmt.MiddleFmt&=~(FMT_FRACSEPARATOR);
    fmt.MiddleFmt|=FMT_NUMSEPARATOR;
    fmt.Locale=MAKELOCALE(',','.',THIN_SPACE,';');
    break;
case 8:
    fmt.BigFmt&=~(FMT_FRACSEPARATOR);
    fmt.BigFmt|=FMT_NUMSEPARATOR;
    fmt.SmallFmt&=~(FMT_FRACSEPARATOR);
    fmt.SmallFmt|=FMT_NUMSEPARATOR;
    fmt.MiddleFmt&=~(FMT_FRACSEPARATOR);
    fmt.MiddleFmt|=FMT_NUMSEPARATOR;
    fmt.Locale=MAKELOCALE(',',THIN_SPACE,THIN_SPACE,';');
    break;
case 9:
    fmt.BigFmt&=~FMT_NUMSEPARATOR;
    fmt.BigFmt|=FMT_FRACSEPARATOR;
    fmt.SmallFmt&=~FMT_NUMSEPARATOR;
    fmt.SmallFmt|=FMT_FRACSEPARATOR;
    fmt.MiddleFmt&=~FMT_NUMSEPARATOR;
    fmt.MiddleFmt|=FMT_FRACSEPARATOR;
    fmt.Locale=MAKELOCALE(',',THIN_SPACE,THIN_SPACE,';');
    break;
case 10:
    fmt.BigFmt|=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR;
    fmt.SmallFmt|=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR;
    fmt.MiddleFmt|=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR;
    fmt.Locale=MAKELOCALE(',','.',THIN_SPACE,';');
    break;
case 11:
    fmt.BigFmt|=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR;
    fmt.SmallFmt|=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR;
    fmt.MiddleFmt|=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR;
    fmt.Locale=MAKELOCALE(',',THIN_SPACE,THIN_SPACE,';');
    break;


}

rplSetSystemNumberFormat(&fmt);
halScreen.DirtyFlag|=STACK_DIRTY;

}


void onSpcKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

// CYCLE BETWEEN VARIOUS OPTIONS
    const char * const options[]={
        "STD","FIX","SCI","ENG"
    };

    NUMFORMAT fmt;
    BINT option=0;
    rplGetSystemNumberFormat(&fmt);

    if(fmt.MiddleFmt&FMT_TRAILINGZEROS) option=1;
    if(fmt.MiddleFmt&FMT_SCI) option=2;
    if(fmt.MiddleFmt&FMT_ENG) option=3;


    // CYCLE THROUGH ITEMS:
    ++option;
    if(option>3) option=0;

    halStatusAreaPopup();

DRAWSURFACE scr;
ggl_initscr(&scr);
int ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
// CLEAR STATUS AREA
ggl_rect(&scr,STATUSAREA_X,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);

DrawTextBk(STATUSAREA_X+1,ytop+1,"Display Mode:",halScreen.StAreaFont,0xf,0,&scr);
DrawTextBk(STATUSAREA_X+1,ytop+1+halScreen.StAreaFont->BitmapHeight,(char *)options[option],halScreen.StAreaFont,0xf,0,&scr);


// CHANGE THE FORMAT TO THE SELECTED OPTION
switch(option)
{
default:
case 0:
    fmt.MiddleFmt&=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR|FMT_GROUPDIGITSMSK|FMT_USECAPITALS|FMT_NUMDIGITS|FMT_PREFEXPMSK;   // PRESERVE ALL THESE
    break;
case 1:
    fmt.MiddleFmt&=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR|FMT_GROUPDIGITSMSK|FMT_USECAPITALS|FMT_NUMDIGITS|FMT_PREFEXPMSK;   // PRESERVE ALL THESE
    fmt.MiddleFmt|=FMT_TRAILINGZEROS;
    break;
case 2:
    fmt.MiddleFmt&=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR|FMT_GROUPDIGITSMSK|FMT_USECAPITALS|FMT_NUMDIGITS|FMT_PREFEXPMSK;   // PRESERVE ALL THESE
    fmt.MiddleFmt|=FMT_SCI;
    break;
case 3:
    fmt.MiddleFmt&=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR|FMT_GROUPDIGITSMSK|FMT_USECAPITALS|FMT_NUMDIGITS|FMT_PREFEXPMSK;   // PRESERVE ALL THESE
    fmt.MiddleFmt|=FMT_SCI|FMT_ENG;
    if((PREFERRED_EXPRAW(fmt.MiddleFmt)==0) || (PREFERRED_EXPRAW(fmt.MiddleFmt)==8)) fmt.MiddleFmt|=FMT_SUPRESSEXP;    // SUPRESS ZERO EXPONENT ONLY WHEN FIXED EXPONENT IS ZERO

    break;
}

rplSetSystemNumberFormat(&fmt);
halScreen.DirtyFlag|=STACK_DIRTY;

}


void onMulDivKeyHandler(BINT keymsg)
{

// CYCLE BETWEEN VARIOUS OPTIONS
    const char * const options[]={
        "Auto",
        "  =  0",
        "k = +3",
        "M = +6",
        "G = +9",
        "T = +12",
        "P = +15",
        "E = +18",
        "Z = +21",
        "z = -21",
        "a = -18",
        "f = -15",
        "p = -12",
        "n = -9",
        "µ = -6",
        "m = -3"
    };

    NUMFORMAT fmt;
    BINT option=0;
    rplGetSystemNumberFormat(&fmt);

    option=PREFERRED_EXPRAW(fmt.MiddleFmt);
    if(option) {
        option-=7;
        if(option<=0) option+=15;
    }



    // CYCLE THROUGH ITEMS:
    if(KM_KEY(keymsg)==KB_MUL) {
        if(option!=9) --option;
    }
    else {
        if(option!=8) ++option;
    }
    if(option<0) option=15;
    if(option>15) option=0;


    halStatusAreaPopup();

DRAWSURFACE scr;
ggl_initscr(&scr);
int ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
// CLEAR STATUS AREA
ggl_rect(&scr,STATUSAREA_X,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);

DrawTextBk(STATUSAREA_X+1,ytop+1,"ENG exponent:",halScreen.StAreaFont,0xf,0,&scr);
DrawTextBk(STATUSAREA_X+1,ytop+1+halScreen.StAreaFont->BitmapHeight,(char *)options[option],halScreen.StAreaFont,0xf,0,&scr);


if(option) option+=7;
if(option>15) option-=15;

    fmt.MiddleFmt&=FMT_NUMSEPARATOR|FMT_FRACSEPARATOR|FMT_GROUPDIGITSMSK|FMT_USECAPITALS|FMT_NUMDIGITS;   // PRESERVE ALL THESE
    fmt.MiddleFmt|=FMT_SCI|FMT_ENG|FMT_PREFEREXPRAW(option);
    if((option==0) || (option==8)) fmt.MiddleFmt|=FMT_SUPRESSEXP;    // SUPRESS ZERO EXPONENT ONLY WHEN FIXED EXPONENT IS ZERO


rplSetSystemNumberFormat(&fmt);
halScreen.DirtyFlag|=STACK_DIRTY;

}


void onDigitKeyHandler(BINT keymsg)
{
    NUMFORMAT fmt;
    BINT digits=0;
    rplGetSystemNumberFormat(&fmt);

    switch(KM_KEY(keymsg))
    {
    case KB_0:
        digits=0;
        break;
    case KB_1:
        digits=1;
        break;
    case KB_2:
        digits=2;
        break;
    case KB_3:
        digits=3;
        break;
    case KB_4:
        digits=4;
        break;
    case KB_5:
        digits=5;
        break;
    case KB_6:
        digits=6;
        break;
    case KB_7:
        digits=7;
        break;
    case KB_8:
        digits=8;
        break;
    case KB_9:
        digits=9;
        break;
    }


    fmt.MiddleFmt&=~FMT_NUMDIGITS;
    fmt.MiddleFmt|=digits;
    fmt.BigFmt&=~FMT_NUMDIGITS;
    fmt.BigFmt|=digits;
    fmt.SmallFmt&=~FMT_NUMDIGITS;
    fmt.SmallFmt|=digits;

    digits+='0';


    halStatusAreaPopup();

DRAWSURFACE scr;
ggl_initscr(&scr);
int ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
// CLEAR STATUS AREA
ggl_rect(&scr,STATUSAREA_X,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);

DrawTextBk(STATUSAREA_X+1,ytop+1,"Display Digits:",halScreen.StAreaFont,0xf,0,&scr);
DrawTextBk(STATUSAREA_X+1,ytop+1+halScreen.StAreaFont->BitmapHeight,(char *)&digits,halScreen.StAreaFont,0xf,0,&scr);


rplSetSystemNumberFormat(&fmt);
halScreen.DirtyFlag|=STACK_DIRTY;

}



void onUpDownKeyHandler(BINT keymsg)
{
BINT precision=Context.precdigits;

    if(KM_KEY(keymsg)==KB_UP) precision+=8;
    else precision-=8;

    if(precision<8) precision=8;
    if(precision>MAX_USERPRECISION) precision=MAX_USERPRECISION;


    Context.precdigits=precision;

    char digits_string[12],empty=' ';

    stringcpy(digits_string,"0000 digits");

    if(precision>=1000) { digits_string[0]=precision/1000+'0'; precision=precision%1000; empty='0'; }
    else digits_string[0]=empty;
    if(precision>=100) { digits_string[1]=precision/100+'0'; precision=precision%100; empty='0'; }
    else digits_string[1]=empty;
    if(precision>=10) { digits_string[2]=precision/10+'0'; precision=precision%10; empty='0'; }
    else digits_string[2]=empty;
    digits_string[3]=precision+'0';

    halStatusAreaPopup();


DRAWSURFACE scr;
ggl_initscr(&scr);
int ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
// CLEAR STATUS AREA
ggl_rect(&scr,STATUSAREA_X,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);

DrawTextBk(STATUSAREA_X+1,ytop+1,"System precision:",halScreen.StAreaFont,0xf,0,&scr);
DrawTextBk(STATUSAREA_X+1,ytop+1+halScreen.StAreaFont->BitmapHeight,digits_string,halScreen.StAreaFont,0xf,0,&scr);


halScreen.DirtyFlag|=STACK_DIRTY;

}

// SHOW/HIDE THE SECOND MENU WHEN PRESSED
void onVarKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);
    if(halScreen.Menu2) {
        halSetMenu2Height(0);  // HIDE THE MENU
        rplSetSystemFlag(FL_HIDEMENU2);
    }
    else {
        halSetMenu2Height(MENU2_HEIGHT);
        rplClrSystemFlag(FL_HIDEMENU2);
    }

}

void onBKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    halFlags |= HAL_SKIPNEXTALARM;

}

void alphaKeyHandler(BINT keymsg)
{
    UNUSED_ARGUMENT(keymsg);

    if((halScreen.CursorState&0xff)=='L') {
        halSetCmdLineMode('C');
        halScreen.DirtyFlag|=CMDLINE_CURSORDIRTY;

    }
    else
    {
        if((halScreen.CursorState&0xff)=='C') {
            halSetCmdLineMode('L');
            halScreen.DirtyFlag|=CMDLINE_CURSORDIRTY;

        }

    }
}

void shiftedalphaKeyHandler(BINT keymsg)
{
// CYCLE BETWEEN D, P AND A MODES WHEN ALPHA IS DISABLED
    UNUSED_ARGUMENT(keymsg);

    switch(halScreen.CursorState&0xff)
    {
     case 'D':
        halSetCmdLineMode('P');
        halScreen.DirtyFlag|=CMDLINE_CURSORDIRTY;
        break;
    case 'P':
        halSetCmdLineMode('A');
        halScreen.DirtyFlag|=CMDLINE_CURSORDIRTY;
        break;
    case 'A':
        halSetCmdLineMode('P');
        halScreen.DirtyFlag|=CMDLINE_CURSORDIRTY;
        break;
    }

}


void changemenuKeyHandler(BINT keymsg,WORD menucode)
{
    UNUSED_ARGUMENT(keymsg);

    WORDPTR numobject=rplNewBINT(menucode,HEXBINT);

    if(!numobject || Exceptions) return;
    BINT menu=rplGetActiveMenu();

    rplSaveMenuHistory(menu);
    rplChangeMenu(menu,numobject);

    if(menu==1) halScreen.DirtyFlag|=MENU1_DIRTY;
    else halScreen.DirtyFlag|=MENU2_DIRTY;

}

void backmenuKeyHandler(BINT keymsg,WORD menu)
{
    UNUSED_ARGUMENT(keymsg);

    if(menu<1 || menu>2) return;

    WORDPTR oldmenu=rplPopMenuHistory(menu);
    if(oldmenu) {
        rplChangeMenu(menu,oldmenu);

    if(menu==1) halScreen.DirtyFlag|=MENU1_DIRTY;
    else halScreen.DirtyFlag|=MENU2_DIRTY;
    }

}


void backmenu1KeyHandler(BINT keymsg)
{
backmenuKeyHandler(keymsg,1);
}

void backmenu2KeyHandler(BINT keymsg)
{
backmenuKeyHandler(keymsg,2);
}


// CUSTOM KEY DEFINITIONS - LOWER LEVEL HANDLER
void customKeyHandler(BINT keymsg,WORDPTR action)
{

    if(!action) return;
    BINT inlist=0;
    // COMMANDS CAN BE PUT INSIDE LISTS
    if(ISLIST(*action)) {
        action=rplGetListElement(action,1);
        if(!action) return;
        if(*action==CMD_ENDLIST) return;    // EMPTY LIST!
        inlist=1;
    }


    // DEFAULT MESSAGE
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
            // ACTION WHEN IN THE STACK OR SUBCONTEXTS OTHER THAN THE EDITOR
                WORD Opcode=0;
                BINT hideargument=1;


                    // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

                    if(ISIDENT(*action)) {
                        // JUST EVAL THE VARIABLE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=(CMD_OVR_EVAL1);
                    }
                    else if( (!ISPROLOG(*action)) && (!ISBINT(*action))) {
                         // THIS IS AN OPCODE, EXECUTE DIRECTLY
                         Opcode=*action;
                         hideargument=0;
                        }
                   else if(ISSTRING(*action) && inlist) {
                            // A STRING TO INSERT INTO THE EDITOR
                            // OPEN AN EDITOR AND INSERT THE STRING
                            halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight+2);
                            halSetContext(halGetContext()|CONTEXT_INEDITOR);
                            if(KM_SHIFTPLANE(keymsg)&SHIFT_ALPHA) uiOpenCmdLine('X');
                            else uiOpenCmdLine('D');

                            uiInsertCharactersN((BYTEPTR) (action+1),(BYTEPTR) (action+1)+rplStrSize(action));
                            uiAutocompleteUpdate();

                        }
                    else {
                    // ALL OTHER OBJECTS AND COMMANDS, DO XEQ
                    rplPushData(action);
                    Opcode=(CMD_OVR_XEQ);
                    }


                if(Opcode) uiCmdRunHide(Opcode,hideargument);
                if(Exceptions) {
                    // TODO: SHOW ERROR MESSAGE
                    halShowErrorMsg();
                    Exceptions=0;
                } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY;
            halScreen.DirtyFlag|=STACK_DIRTY|STAREA_DIRTY;

    }
    else {
        // ACTION INSIDE THE EDITOR
        WORD Opcode=0;
        BINT hideargument=1;

        if(!action) return;

            // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

            if(ISIDENT(*action)&& inlist) {
                switch(halScreen.CursorState&0xff)
                {
                case 'D':
                {
                    // HANDLE DIRECTORIES IN A SPECIAL WAY: DON'T CLOSE THE COMMAND LINE
                    WORDPTR *var=rplFindGlobal(action,1);
                    if(var) {
                        if(ISDIR(*(var[1]))) {
                            // CHANGE THE DIR WITHOUT CLOSING THE COMMAND LINE
                            rplPushData(action);    // PUSH THE NAME ON THE STACK
                            Opcode=(CMD_OVR_EVAL);
                            break;
                        }
                    }
                    rplPushRet(action);
                    BINT result=endCmdLineAndCompile();
                    action=rplPopRet();
                    if(result) {
                        // USER IS TRYING TO EVAL THE VARIABLE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=(CMD_OVR_EVAL);
                    }
                    break;
                }
                case 'A':
                {
                    WORDPTR *var=rplFindGlobal(action,1);
                    if(var) {
                        if(ISDIR(*(var[1]))) {
                            // CHANGE THE DIR WITHOUT CLOSING THE COMMAND LINE
                            rplPushData(action);    // PUSH THE NAME ON THE STACK
                            Opcode=(CMD_OVR_EVAL);
                            break;
                        }
                    }

                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    // REMOVE THE TICK MARKS IN ALG MODE
                    if((totaln>2)&&(string[0]=='\'')) {
                        ++string;
                        --endstring;
                    }

                    uiInsertCharactersN(string,endstring);
                    uiAutocompleteUpdate();

                    break;
                }

                case 'P':
                {
                    WORDPTR *var=rplFindGlobal(action,1);
                    if(var) {
                        if(ISDIR(*(var[1]))) {
                            // CHANGE THE DIR WITHOUT CLOSING THE COMMAND LINE
                            rplPushData(action);    // PUSH THE NAME ON THE STACK
                            Opcode=(CMD_OVR_EVAL);
                            break;
                        }
                    }

                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    // REMOVE THE TICK MARKS IN ALG MODE
                    if((totaln>2)&&(string[0]=='\'')) {
                        ++string;
                        --endstring;
                    }

                    uiSeparateToken();
                    uiInsertCharactersN(string,endstring);
                    uiSeparateToken();
                    uiAutocompleteUpdate();

                    break;
                }
                }
            }
            else if(ISUNIT(*action)) {
                switch(halScreen.CursorState&0xff)
                {
                case 'D':
                {
                    rplPushRet(action);
                    BINT result=endCmdLineAndCompile();
                    action=rplPopRet();
                    if(result) {
                        // USER IS TRYING TO APPLY THE UNIT
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=(CMD_OVR_MUL);
                    }
                    break;
                }
                case 'A':
                {

                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    if( (totaln>2)&&(string[0]=='1')&&(string[1]=='_')) string+=2;

                    uiInsertCharactersN(string,endstring);
                    uiAutocompleteUpdate();

                    break;
                }

                case 'P':
                {

                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    uiSeparateToken();
                    uiInsertCharactersN(string,endstring);
                    uiSeparateToken();
                    uiInsertCharacters((BYTEPTR)"*");
                    uiSeparateToken();
                    uiAutocompleteUpdate();

                    break;
                }

                }
            }
            else if(!ISPROLOG(*action)) {
                // THIS IS A COMMAND, DECOMPILE AND INSERT NAME
                switch(halScreen.CursorState&0xff)
                {
                case 'D':
                {
                    rplPushRet(action);
                    BINT result=endCmdLineAndCompile();
                    action=rplPopRet();
                    if(result) {
                        Opcode=*action;
                        hideargument=0;
                    }
                    break;
                }
                case 'A':
                {

                    WORD tokeninfo=0;
                    LIBHANDLER han=rplGetLibHandler(LIBNUM(*action));


                    // GET THE SYMBOLIC TOKEN INFORMATION
                    if(han) {
                        WORD savecurOpcode=CurOpcode;
                        DecompileObject=action;
                        CurOpcode=MKOPCODE(LIBNUM(*action),OPCODE_GETINFO);
                        (*han)();

                        if(RetNum>OK_TOKENINFO) tokeninfo=RetNum;

                        CurOpcode=savecurOpcode;
                    }


                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    uiInsertCharactersN(string,endstring);
                    if(TI_TYPE(tokeninfo)==TITYPE_FUNCTION) {
                        uiInsertCharacters((BYTEPTR)"()");
                        uiCursorLeft(1);
                    }
                    uiAutocompleteUpdate();

                    break;
                }

                case 'P':
                {

                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    BINT SavedException=Exceptions;
                    BINT SavedErrorCode=ErrorCode;

                    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                    Exceptions=SavedException;
                    ErrorCode=SavedErrorCode;

                    if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    BYTEPTR string=(BYTEPTR) (opname+1);
                    BINT totaln=rplStrLen(opname);
                    BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                    uiSeparateToken();
                    uiInsertCharactersN(string,endstring);
                    uiSeparateToken();
                    uiAutocompleteUpdate();

                    break;
                }

                }




            }
            else if(ISPROGRAM(*action)) {
                if(!ISSECO(*action)) {
                    // IT'S A DOCOL PROGRAM, EXECUTE TRANSPARENTLY
                    rplPushData(action);    // PUSH THE NAME ON THE STACK
                    Opcode=CMD_OVR_XEQ;
                }
                else {
                    rplPushRet(action);
                    BINT result=endCmdLineAndCompile();
                    action=rplPopRet();
                    if(result) {
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode=CMD_OVR_XEQ;
                    }
                }
            }
            else if(ISSTRING(*action)) {
                BYTEPTR string=(BYTEPTR) (action+1);
                BINT totaln=rplStrLen(action);
                BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(action),totaln);

                if(!inlist) {
                    // ADD THE QUOTES IN D OR P MODE
                    if(((halScreen.CursorState&0xff)=='P')||((halScreen.CursorState&0xff)=='D')) {
                        uiSeparateToken();
                        uiInsertCharacters((BYTEPTR)"\"");
                    }
                }
                uiInsertCharactersN(string,endstring);
                if(!inlist && (((halScreen.CursorState&0xff)=='P')||((halScreen.CursorState&0xff)=='D')))  {
                    uiInsertCharacters((BYTEPTR)"\"");
                    uiSeparateToken();
                }
                uiAutocompleteUpdate();
                }
            else {
            // ALL OTHER OBJECTS AND COMMANDS
            switch(halScreen.CursorState&0xff)
            {
            case 'D':
            {
                rplPushRet(action);
                BINT result=endCmdLineAndCompile();
                action=rplPopRet();
                if(result) {
                    if(!ISPROLOG(*action)) { Opcode=*action; hideargument=0; }// RUN COMMANDS DIRECTLY
                    else {
                        Opcode=(CMD_OVR_XEQ);
                        rplPushData(action);
                    }
                }
                break;
            }
            case 'A':
            {

                WORD tokeninfo=0;
                LIBHANDLER han=rplGetLibHandler(LIBNUM(*action));


                // GET THE SYMBOLIC TOKEN INFORMATION
                if(han) {
                    WORD savecurOpcode=CurOpcode;
                    DecompileObject=action;
                    CurOpcode=MKOPCODE(LIBNUM(*action),OPCODE_GETINFO);
                    (*han)();

                    if(RetNum>OK_TOKENINFO) tokeninfo=RetNum;

                    CurOpcode=savecurOpcode;
                }


                // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                BINT SavedException=Exceptions;
                BINT SavedErrorCode=ErrorCode;

                Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                Exceptions=SavedException;
                ErrorCode=SavedErrorCode;

                if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                BYTEPTR string=(BYTEPTR) (opname+1);
                BINT totaln=rplStrLen(opname);
                BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                uiInsertCharactersN(string,endstring);
                if(TI_TYPE(tokeninfo)==TITYPE_FUNCTION) {
                    uiInsertCharacters((BYTEPTR)"()");
                    uiCursorLeft(1);
                }
                uiAutocompleteUpdate();

                break;
            }

            case 'P':
            {

                // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                BINT SavedException=Exceptions;
                BINT SavedErrorCode=ErrorCode;

                Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                WORDPTR opname=rplDecompile(action,DECOMP_EDIT);
                Exceptions=SavedException;
                ErrorCode=SavedErrorCode;

                if(!opname) break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                BYTEPTR string=(BYTEPTR) (opname+1);
                BINT totaln=rplStrLen(opname);
                BYTEPTR endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

                uiSeparateToken();
                uiInsertCharactersN(string,endstring);
                uiSeparateToken();
                uiAutocompleteUpdate();

                break;
            }

            }



        }

        if(Opcode) uiCmdRunHide(Opcode,hideargument);
        if(Exceptions) {
            // TODO: SHOW ERROR MESSAGE
            halShowErrorMsg();
            Exceptions=0;
        } else halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY;
        halScreen.DirtyFlag|=STACK_DIRTY|STAREA_DIRTY;
}

}




























#define DECLARE_TRANSPCMDKEYHANDLER(name,opcode) void name##KeyHandler(BINT keymsg) \
                                                             { \
                                                             UNUSED_ARGUMENT(keymsg); \
                                                             transpcmdKeyHandler(opcode); \
                                                             }

#define DECLARE_CMDKEYHANDLER(name,opcode,string,issymbfunc) void name##KeyHandler(BINT keymsg) \
                                                             { \
                                                             UNUSED_ARGUMENT(keymsg); \
                                                             cmdKeyHandler(opcode,(BYTEPTR)string,issymbfunc); \
                                                             }



#define DECLARE_VARKEYHANDLER(name,menu,idx) void name##KeyHandler(BINT keymsg) \
                                                    { \
                                                    varsKeyHandler(keymsg,(menu),(BINT)(idx)); \
                                                    }

#define DECLARE_MENUKEYHANDLER(name,menucode) void name##KeyHandler(BINT keymsg) \
                                                    { \
                                                    changemenuKeyHandler(keymsg,(WORD)(menucode)); \
                                                    }


#define DECLARE_KEYHANDLER(name,lsymbol,csymbol) void name##KeyHandler(BINT keymsg) \
                                                    { \
                                                    alphasymbolKeyHandler(keymsg,(BYTEPTR)(lsymbol),(BYTEPTR)(csymbol)); \
                                                    }

#define DECLARE_SYMBKEYHANDLER(name,symbol,sep) void name##KeyHandler(BINT keymsg) \
                                                    { \
                                                    symbolKeyHandler(keymsg,(BYTEPTR)(symbol),(sep)); \
                                                    }

#define KEYHANDLER_NAME(name)  &(name##KeyHandler)

DECLARE_KEYHANDLER(a,"a","A")
DECLARE_KEYHANDLER(b,"b","B")
DECLARE_KEYHANDLER(c,"c","C")
DECLARE_KEYHANDLER(d,"d","D")
DECLARE_KEYHANDLER(e,"e","E")
DECLARE_KEYHANDLER(f,"f","F")
DECLARE_KEYHANDLER(g,"g","G")
DECLARE_KEYHANDLER(h,"h","H")
DECLARE_KEYHANDLER(i,"i","I")
DECLARE_KEYHANDLER(j,"j","J")
DECLARE_KEYHANDLER(k,"k","K")
DECLARE_KEYHANDLER(l,"l","L")
DECLARE_KEYHANDLER(m,"m","M")
DECLARE_KEYHANDLER(n,"n","N")
DECLARE_KEYHANDLER(o,"o","O")
DECLARE_KEYHANDLER(p,"p","P")
DECLARE_KEYHANDLER(q,"q","Q")
DECLARE_KEYHANDLER(r,"r","R")
DECLARE_KEYHANDLER(s,"s","S")
DECLARE_KEYHANDLER(t,"t","T")
DECLARE_KEYHANDLER(u,"u","U")
DECLARE_KEYHANDLER(v,"v","V")
DECLARE_KEYHANDLER(w,"w","W")
DECLARE_KEYHANDLER(x,"x","X")
DECLARE_KEYHANDLER(y,"y","Y")
DECLARE_KEYHANDLER(z,"z","Z")

DECLARE_SYMBKEYHANDLER(arrow,"→",1)
DECLARE_SYMBKEYHANDLER(comma,",",0)
DECLARE_SYMBKEYHANDLER(semi,";",0)
DECLARE_SYMBKEYHANDLER(colon,":",0)
DECLARE_SYMBKEYHANDLER(infinity,"∞",1)
DECLARE_SYMBKEYHANDLER(undinfinity,"∞̅",1)
DECLARE_SYMBKEYHANDLER(dot,".",0)



DECLARE_SYMBKEYHANDLER(question,"?",0)
DECLARE_SYMBKEYHANDLER(openquestion,"¿",0)
DECLARE_SYMBKEYHANDLER(exclamation,"!",0)
DECLARE_SYMBKEYHANDLER(openexclamation,"¡",0)
DECLARE_SYMBKEYHANDLER(approx,"~",0)
DECLARE_SYMBKEYHANDLER(percent,"%",0)
DECLARE_SYMBKEYHANDLER(dollar,"$",0)
DECLARE_SYMBKEYHANDLER(euro,"€",0)
DECLARE_SYMBKEYHANDLER(backslash,"\\",0)
DECLARE_SYMBKEYHANDLER(pound,"£",0)
DECLARE_SYMBKEYHANDLER(angle,"∡",0)
DECLARE_SYMBKEYHANDLER(degree,"°",0)
DECLARE_SYMBKEYHANDLER(pi,"π",1)
DECLARE_SYMBKEYHANDLER(delta,"Δ",0)
DECLARE_SYMBKEYHANDLER(at,"@",0)
DECLARE_SYMBKEYHANDLER(and,"&",0)

DECLARE_VARKEYHANDLER(var1_1,1,0)
DECLARE_VARKEYHANDLER(var2_1,1,1)
DECLARE_VARKEYHANDLER(var3_1,1,2)
DECLARE_VARKEYHANDLER(var4_1,1,3)
DECLARE_VARKEYHANDLER(var5_1,1,4)
DECLARE_VARKEYHANDLER(var6_1,1,5)

DECLARE_VARKEYHANDLER(var1,2,0)
DECLARE_VARKEYHANDLER(var2,2,1)
DECLARE_VARKEYHANDLER(var3,2,2)
DECLARE_VARKEYHANDLER(var4,2,3)
DECLARE_VARKEYHANDLER(var5,2,4)
DECLARE_VARKEYHANDLER(var6,2,5)






void underscoreKeyHandler(BINT keymsg)
{
    symbolKeyHandler(keymsg,(BYTEPTR)"_",0);

    if((halGetCmdLineMode()!='L')&&(halGetCmdLineMode()!='C')) {
     uiInsertCharacters((BYTEPTR)"[]");
     uiCursorLeft(1);
     halSetCmdLineMode('A');
    }
}

DECLARE_SYMBKEYHANDLER(spc," ",0)
DECLARE_SYMBKEYHANDLER(thinspc," ",0)

DECLARE_SYMBKEYHANDLER(hash,"#",0)
DECLARE_SYMBKEYHANDLER(equal,"=",1)
DECLARE_SYMBKEYHANDLER(notequal,"≠",1)
DECLARE_SYMBKEYHANDLER(ls,"<",1)
DECLARE_SYMBKEYHANDLER(gt,">",1)
DECLARE_SYMBKEYHANDLER(le,"≤",1)
DECLARE_SYMBKEYHANDLER(ge,"≥",1)

DECLARE_SYMBKEYHANDLER(sadd,"+",0)
DECLARE_SYMBKEYHANDLER(ssub,"-",0)
DECLARE_SYMBKEYHANDLER(smul,"*",0)
DECLARE_SYMBKEYHANDLER(sdiv,"/",0)
DECLARE_SYMBKEYHANDLER(spow,"^",0)



DECLARE_KEYHANDLER(sub0,"₀","⁰")
DECLARE_KEYHANDLER(sub1,"₁","¹")
DECLARE_KEYHANDLER(sub2,"₂","²")
DECLARE_KEYHANDLER(sub3,"₃","³")
DECLARE_KEYHANDLER(sub4,"₄","⁴")
DECLARE_KEYHANDLER(sub5,"₅","⁵")
DECLARE_KEYHANDLER(sub6,"₆","⁶")
DECLARE_KEYHANDLER(sub7,"₇","⁷")
DECLARE_KEYHANDLER(sub8,"₈","⁸")
DECLARE_KEYHANDLER(sub9,"₉","⁹")

DECLARE_SYMBKEYHANDLER(keyx,"X",0)




DECLARE_CMDKEYHANDLER(clear,CMD_CLEAR,"CLEAR",-1)
DECLARE_CMDKEYHANDLER(add,(CMD_OVR_ADD),"+",0)
DECLARE_CMDKEYHANDLER(sub,(CMD_OVR_SUB),"-",0)
DECLARE_CMDKEYHANDLER(div,(CMD_OVR_DIV),"/",0)
DECLARE_CMDKEYHANDLER(mul,(CMD_OVR_MUL),"*",0)
DECLARE_CMDKEYHANDLER(inv,(CMD_OVR_INV),"INV",1)
DECLARE_CMDKEYHANDLER(sin,CMD_SIN,"SIN",1)
DECLARE_CMDKEYHANDLER(asin,CMD_ASIN,"ASIN",1)
DECLARE_CMDKEYHANDLER(sinh,CMD_SINH,"SINH",1)
DECLARE_CMDKEYHANDLER(asinh,CMD_ASINH,"ASINH",1)

DECLARE_CMDKEYHANDLER(cos,CMD_COS,"COS",1)
DECLARE_CMDKEYHANDLER(acos,CMD_ACOS,"ACOS",1)
DECLARE_CMDKEYHANDLER(cosh,CMD_COSH,"COSH",1)
DECLARE_CMDKEYHANDLER(acosh,CMD_ACOSH,"ACOSH",1)

DECLARE_CMDKEYHANDLER(tan,CMD_TAN,"TAN",1)
DECLARE_CMDKEYHANDLER(atan,CMD_ATAN,"ATAN",1)
DECLARE_CMDKEYHANDLER(tanh,CMD_TANH,"TANH",1)
DECLARE_CMDKEYHANDLER(atanh,CMD_ATANH,"ATANH",1)

DECLARE_CMDKEYHANDLER(eval,(CMD_OVR_EVAL),"EVAL",-1)
DECLARE_CMDKEYHANDLER(eval1,(CMD_OVR_EVAL1),"EVAL1",-1)
DECLARE_CMDKEYHANDLER(tonum,(CMD_OVR_NUM),"→NUM",-1)


DECLARE_CMDKEYHANDLER(sqrt,CMD_SQRT,"√",0)
DECLARE_CMDKEYHANDLER(pow,(CMD_OVR_POW),"^",0)
DECLARE_CMDKEYHANDLER(ln,CMD_LN,"LN",1)
DECLARE_CMDKEYHANDLER(exp,CMD_EXP,"EXP",1)
DECLARE_CMDKEYHANDLER(log,CMD_LOG,"LOG",1)
DECLARE_CMDKEYHANDLER(alog,CMD_ALOG,"ALOG",1)
DECLARE_CMDKEYHANDLER(sq,CMD_SQ,"SQ",1)
DECLARE_CMDKEYHANDLER(xroot,(CMD_OVR_XROOT),"XROOT",1)

DECLARE_CMDKEYHANDLER(sto,CMD_STO,"STO",2)
DECLARE_CMDKEYHANDLER(rcl,CMD_RCL,"RCL",2)

DECLARE_CMDKEYHANDLER(purge,CMD_PURGE,"PURGE",-1)

DECLARE_CMDKEYHANDLER(abs,CMD_OVR_ABS,"ABS",1)
DECLARE_CMDKEYHANDLER(arg,CMD_ARG,"ARG",1)

DECLARE_TRANSPCMDKEYHANDLER(updir,CMD_UPDIR)
DECLARE_TRANSPCMDKEYHANDLER(home,CMD_HOME)

DECLARE_TRANSPCMDKEYHANDLER(menuswap,CMD_MENUSWAP)


DECLARE_MENUKEYHANDLER(unitmenu,MKMENUCODE(0,DOUNIT,0,0))
DECLARE_MENUKEYHANDLER(prgmenu,MKMENUCODE(0,68,3,0))
DECLARE_MENUKEYHANDLER(varsmenu,MKMENUCODE(1,0,0,0))
DECLARE_MENUKEYHANDLER(mainmenu,MKMENUCODE(0,68,2,0))
DECLARE_MENUKEYHANDLER(arithmenu,MKMENUCODE(0,64,0,0))
DECLARE_MENUKEYHANDLER(cplxmenu,MKMENUCODE(0,30,0,0))
DECLARE_MENUKEYHANDLER(timemenu,MKMENUCODE(0,65,0,0))
DECLARE_MENUKEYHANDLER(basemenu,MKMENUCODE(0,70,0,0))


// **************************************************************************
// ******************* END OF DEFAULT KEY HANDLERS **************************
// **************************************************************************


typedef void (*handlerfunc_t)(BINT keymsg);

// STRUCTURE FOR DEFAULT KEYBOARD HANDLERS
struct keyhandler_t {
    BINT message;
    BINT context;
    handlerfunc_t action;
} ;


// LIST OF HANDLERS, END WITH action=NULL
const struct keyhandler_t const __keydefaulthandlers[]= {

    // BASIC NUMBERS
    { KM_PRESS|KB_1, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_2, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_3, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_4, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_5, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_6, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_7, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_8, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_9, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_0, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_DOT, CONTEXT_ANY,&decimaldotKeyHandler },
    { KM_PRESS|KB_1|SHIFT_ALPHAHOLD, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_2|SHIFT_ALPHAHOLD, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_3|SHIFT_ALPHAHOLD, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_4|SHIFT_ALPHAHOLD, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_5|SHIFT_ALPHAHOLD, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_6|SHIFT_ALPHAHOLD, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_7|SHIFT_ALPHAHOLD, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_8|SHIFT_ALPHAHOLD, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_9|SHIFT_ALPHAHOLD, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_0|SHIFT_ALPHAHOLD, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_DOT|SHIFT_ALPHAHOLD, CONTEXT_ANY,&dotKeyHandler },
    { KM_PRESS|KB_1|SHIFT_ALPHA, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_2|SHIFT_ALPHA, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_3|SHIFT_ALPHA, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_4|SHIFT_ALPHA, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_5|SHIFT_ALPHA, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_6|SHIFT_ALPHA, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_7|SHIFT_ALPHA, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_8|SHIFT_ALPHA, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_9|SHIFT_ALPHA, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_0|SHIFT_ALPHA, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_DOT|SHIFT_ALPHA, CONTEXT_ANY,&decimaldotKeyHandler },

    // BASIC ON AND SHIFTS
    { KM_KEYDN|KB_ON, CONTEXT_ANY,&cancelKeyHandler },

    { KM_PRESS|KB_ALPHA|SHIFT_RS, CONTEXT_ANY,&shiftedalphaKeyHandler },
    { KM_PRESS|KB_ALPHA|SHIFT_RSHOLD, CONTEXT_ANY,&shiftedalphaKeyHandler },

    // TEXT EDITING KEYS
    { KM_PRESS|KB_ENT, CONTEXT_ANY,&enterKeyHandler },
    { KM_PRESS|KB_ENT|SHIFT_ALPHA, CONTEXT_ANY,&enterKeyHandler },
    { KM_PRESS|KB_ENT|SHIFT_ALPHAHOLD, CONTEXT_ANY,&enterKeyHandler },
    { KM_PRESS|KB_BKS, CONTEXT_ANY,&backspKeyHandler },
    { KM_REPEAT|KB_BKS, CONTEXT_ANY,&backspKeyHandler },
    { KM_PRESS|KB_BKS|SHIFT_ALPHA, CONTEXT_ANY,&backspKeyHandler },
    { KM_REPEAT|KB_BKS|SHIFT_ALPHA, CONTEXT_ANY,&backspKeyHandler },
    { KM_PRESS|KB_BKS|SHIFT_RS, CONTEXT_ANY,&clearKeyHandler },
    { KM_PRESS|KB_BKS|SHIFT_RSHOLD, CONTEXT_ANY,&clearKeyHandler },
    { KM_PRESS|KB_BKS|SHIFT_LS, CONTEXT_ANY,&deleteKeyHandler },
    { KM_PRESS|KB_BKS|SHIFT_LSHOLD, CONTEXT_ANY,&deleteKeyHandler },
    { KM_PRESS|KB_BKS|SHIFT_LS|SHIFT_ALPHA, CONTEXT_ANY,&deleteKeyHandler },
    { KM_PRESS|KB_BKS|SHIFT_LSHOLD|SHIFT_ALPHA, CONTEXT_ANY,&deleteKeyHandler },
    { KM_PRESS|KB_LF|SHIFT_LS, CONTEXT_ANY,&lsleftKeyHandler },
    { KM_PRESS|KB_RT|SHIFT_LS, CONTEXT_ANY,&lsrightKeyHandler },
    { KM_PRESS|KB_LF|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&copyclipKeyHandler },
    { KM_PRESS|KB_RT|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&pasteclipKeyHandler },
    { KM_PRESS|KB_DN|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&cutclipKeyHandler },
    { KM_PRESS|KB_LF|SHIFT_LS|SHIFT_ALPHA, CONTEXT_ANY,&lsleftKeyHandler },
    { KM_PRESS|KB_RT|SHIFT_LS|SHIFT_ALPHA, CONTEXT_ANY,&lsrightKeyHandler },
    { KM_PRESS|KB_LF|SHIFT_LS|SHIFT_LSHOLD|SHIFT_ALPHA, CONTEXT_ANY,&copyclipKeyHandler },
    { KM_PRESS|KB_RT|SHIFT_LS|SHIFT_LSHOLD|SHIFT_ALPHA, CONTEXT_ANY,&pasteclipKeyHandler },


    // CURSOR MOVEMENT KEYS
    { KM_PRESS|KB_LF, CONTEXT_ANY,&leftKeyHandler },
    { KM_REPEAT|KB_LF, CONTEXT_ANY,&leftKeyHandler },
    { KM_PRESS|KB_RT, CONTEXT_ANY,&rightKeyHandler },
    { KM_REPEAT|KB_RT, CONTEXT_ANY,&rightKeyHandler },
    { KM_PRESS|KB_LF|SHIFT_ALPHA, CONTEXT_ANY,&leftKeyHandler },
    { KM_REPEAT|KB_LF|SHIFT_ALPHA, CONTEXT_ANY,&leftKeyHandler },
    { KM_PRESS|KB_RT|SHIFT_ALPHA, CONTEXT_ANY,&rightKeyHandler },
    { KM_REPEAT|KB_RT|SHIFT_ALPHA, CONTEXT_ANY,&rightKeyHandler },
    { KM_PRESS|KB_DN, CONTEXT_ANY,&downKeyHandler },
    { KM_REPEAT|KB_DN, CONTEXT_ANY,&downKeyHandler },
    { KM_PRESS|KB_DN|SHIFT_ALPHA, CONTEXT_ANY,&downKeyHandler },
    { KM_REPEAT|KB_DN|SHIFT_ALPHA, CONTEXT_ANY,&downKeyHandler },
    { KM_PRESS|KB_UP, CONTEXT_ANY,&upKeyHandler },
    { KM_REPEAT|KB_UP, CONTEXT_ANY,&upKeyHandler },
    { KM_PRESS|KB_UP|SHIFT_ALPHA, CONTEXT_ANY,&upKeyHandler },
    { KM_REPEAT|KB_UP|SHIFT_ALPHA, CONTEXT_ANY,&upKeyHandler },

    { KM_PRESS|KB_LF|SHIFT_RS, CONTEXT_ANY,&rsleftKeyHandler },
    { KM_PRESS|KB_LF|SHIFT_RSHOLD, CONTEXT_ANY,&rsholdleftKeyHandler },
    { KM_PRESS|KB_LF|SHIFT_RS|SHIFT_ALPHA, CONTEXT_ANY,&rsleftKeyHandler },
    { KM_PRESS|KB_LF|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,&rsholdleftKeyHandler },
    { KM_PRESS|KB_RT|SHIFT_RS, CONTEXT_ANY,&rsrightKeyHandler },
    { KM_PRESS|KB_RT|SHIFT_RSHOLD, CONTEXT_ANY,&rsholdrightKeyHandler },
    { KM_PRESS|KB_RT|SHIFT_RS|SHIFT_ALPHA, CONTEXT_ANY,&rsrightKeyHandler },
    { KM_PRESS|KB_RT|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,&rsholdrightKeyHandler },
    { KM_PRESS|KB_RT|SHIFT_ALPHAHOLD, CONTEXT_ANY,&alphaholdrightKeyHandler },

    { KM_PRESS|KB_UP|SHIFT_RS, CONTEXT_ANY,&rsupKeyHandler },
    { KM_PRESS|KB_UP|SHIFT_RSHOLD, CONTEXT_ANY,&rsholdupKeyHandler },
    { KM_PRESS|KB_UP|SHIFT_RS|SHIFT_ALPHA, CONTEXT_ANY,&rsupKeyHandler },
    { KM_PRESS|KB_UP|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,&rsholdupKeyHandler },
    { KM_PRESS|KB_UP|SHIFT_ALPHAHOLD|SHIFT_ALPHA, CONTEXT_ANY,&alphaholdupKeyHandler },

    { KM_PRESS|KB_DN|SHIFT_RS, CONTEXT_ANY,&rsdownKeyHandler },
    { KM_PRESS|KB_DN|SHIFT_RSHOLD, CONTEXT_ANY,&rsholddownKeyHandler },
    { KM_PRESS|KB_DN|SHIFT_RS|SHIFT_ALPHA, CONTEXT_ANY,&rsdownKeyHandler },
    { KM_PRESS|KB_DN|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,&rsholddownKeyHandler },
    { KM_PRESS|KB_DN|SHIFT_ALPHAHOLD|SHIFT_ALPHA, CONTEXT_ANY,&alphaholddownKeyHandler },



    { KM_PRESS|KB_DOT|SHIFT_RS,CONTEXT_ANY,&newlineKeyHandler },
    { KM_PRESS|KB_DOT|SHIFT_RSHOLD,CONTEXT_ANY,&newlineKeyHandler },
    { KM_PRESS|KB_DOT|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY,&newlineKeyHandler },
    { KM_PRESS|KB_DOT|SHIFT_RSHOLD|SHIFT_ALPHA,CONTEXT_ANY,&newlineKeyHandler },



    // BASIC OPERATORS
    { KM_PRESS|KB_ADD, CONTEXT_ANY,&addKeyHandler },
    { KM_PRESS|KB_SUB, CONTEXT_ANY,&subKeyHandler },
    { KM_PRESS|KB_DIV, CONTEXT_ANY,&divKeyHandler },
    { KM_PRESS|KB_MUL, CONTEXT_ANY,&mulKeyHandler },
    { KM_PRESS|KB_ADD|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(sadd) },
    { KM_PRESS|KB_SUB|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(ssub) },
    { KM_PRESS|KB_DIV|SHIFT_ALPHA|SHIFT_RS, CONTEXT_ANY,KEYHANDLER_NAME(sdiv) },
    { KM_PRESS|KB_DIV|SHIFT_ALPHA|SHIFT_RSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(sdiv) },
    { KM_PRESS|KB_MUL|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(smul) },

    // VARS MENU KEYS
    { KM_PRESS|KB_G, CONTEXT_ANY,KEYHANDLER_NAME(var1)},
    { KM_PRESS|KB_G|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(var1)},
    { KM_PRESS|KB_G|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var1)},
    { KM_PRESS|KB_G|SHIFT_RS, CONTEXT_ANY,KEYHANDLER_NAME(var1)},
    { KM_PRESS|KB_G|SHIFT_RSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var1)},
    { KM_LPRESS|KB_G, CONTEXT_ANY,KEYHANDLER_NAME(var1)},
    { KM_KEYUP|KB_G, CONTEXT_ANY,KEYHANDLER_NAME(var1)},

    { KM_PRESS|KB_H, CONTEXT_ANY,KEYHANDLER_NAME(var2)},
    { KM_PRESS|KB_H|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(var2)},
    { KM_PRESS|KB_H|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var2)},
    { KM_PRESS|KB_H|SHIFT_RS, CONTEXT_ANY,KEYHANDLER_NAME(var2)},
    { KM_PRESS|KB_H|SHIFT_RSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var2)},
    { KM_LPRESS|KB_H, CONTEXT_ANY,KEYHANDLER_NAME(var2)},
    { KM_KEYUP|KB_H, CONTEXT_ANY,KEYHANDLER_NAME(var2)},

    { KM_PRESS|KB_I, CONTEXT_ANY,KEYHANDLER_NAME(var3)},
    { KM_PRESS|KB_I|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(var3)},
    { KM_PRESS|KB_I|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var3)},
    { KM_PRESS|KB_I|SHIFT_RS, CONTEXT_ANY,KEYHANDLER_NAME(var3)},
    { KM_PRESS|KB_I|SHIFT_RSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var3)},
    { KM_LPRESS|KB_I, CONTEXT_ANY,KEYHANDLER_NAME(var3)},
    { KM_KEYUP|KB_I, CONTEXT_ANY,KEYHANDLER_NAME(var3)},

    { KM_PRESS|KB_J, CONTEXT_ANY,KEYHANDLER_NAME(var4)},
    { KM_PRESS|KB_J|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(var4)},
    { KM_PRESS|KB_J|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var4)},
    { KM_PRESS|KB_J|SHIFT_RS, CONTEXT_ANY,KEYHANDLER_NAME(var4)},
    { KM_PRESS|KB_J|SHIFT_RSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var4)},
    { KM_LPRESS|KB_J, CONTEXT_ANY,KEYHANDLER_NAME(var4)},
    { KM_KEYUP|KB_J, CONTEXT_ANY,KEYHANDLER_NAME(var4)},

    { KM_PRESS|KB_K, CONTEXT_ANY,KEYHANDLER_NAME(var5)},
    { KM_PRESS|KB_K|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(var5)},
    { KM_PRESS|KB_K|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var5)},
    { KM_PRESS|KB_K|SHIFT_RS, CONTEXT_ANY,KEYHANDLER_NAME(var5)},
    { KM_PRESS|KB_K|SHIFT_RSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var5)},
    { KM_LPRESS|KB_K, CONTEXT_ANY,KEYHANDLER_NAME(var5)},
    { KM_KEYUP|KB_K, CONTEXT_ANY,KEYHANDLER_NAME(var5)},

    { KM_PRESS|KB_L, CONTEXT_ANY,KEYHANDLER_NAME(var6)},
    { KM_PRESS|KB_L|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(var6)},
    { KM_PRESS|KB_L|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var6)},
    { KM_PRESS|KB_L|SHIFT_RS, CONTEXT_ANY,KEYHANDLER_NAME(var6)},
    { KM_PRESS|KB_L|SHIFT_RSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var6)},
    { KM_LPRESS|KB_L, CONTEXT_ANY,KEYHANDLER_NAME(var6)},
    { KM_KEYUP|KB_L, CONTEXT_ANY,KEYHANDLER_NAME(var6)},

    { KM_PRESS|KB_A, CONTEXT_ANY,KEYHANDLER_NAME(var1_1)},
    { KM_PRESS|KB_A|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(var1_1)},
    { KM_PRESS|KB_A|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var1_1)},
    { KM_PRESS|KB_A|SHIFT_RS, CONTEXT_ANY,KEYHANDLER_NAME(var1_1)},
    { KM_PRESS|KB_A|SHIFT_RSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var1_1)},
    { KM_LPRESS|KB_A, CONTEXT_ANY,KEYHANDLER_NAME(var1_1)},
    { KM_KEYUP|KB_A, CONTEXT_ANY,KEYHANDLER_NAME(var1_1)},

    { KM_PRESS|KB_B, CONTEXT_ANY,KEYHANDLER_NAME(var2_1)},
    { KM_PRESS|KB_B|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(var2_1)},
    { KM_PRESS|KB_B|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var2_1)},
    { KM_PRESS|KB_B|SHIFT_RS, CONTEXT_ANY,KEYHANDLER_NAME(var2_1)},
    { KM_PRESS|KB_B|SHIFT_RSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var2_1)},
    { KM_LPRESS|KB_B, CONTEXT_ANY,KEYHANDLER_NAME(var2_1)},
    { KM_KEYUP|KB_B, CONTEXT_ANY,KEYHANDLER_NAME(var2_1)},



{ KM_PRESS|KB_C, CONTEXT_ANY,KEYHANDLER_NAME(var3_1)},
{ KM_PRESS|KB_C|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(var3_1)},
{ KM_PRESS|KB_C|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var3_1)},
{ KM_PRESS|KB_C|SHIFT_RS, CONTEXT_ANY,KEYHANDLER_NAME(var3_1)},
{ KM_PRESS|KB_C|SHIFT_RSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var3_1)},
{ KM_LPRESS|KB_C, CONTEXT_ANY,KEYHANDLER_NAME(var3_1)},
{ KM_KEYUP|KB_C, CONTEXT_ANY,KEYHANDLER_NAME(var3_1)},

{ KM_PRESS|KB_D, CONTEXT_ANY,KEYHANDLER_NAME(var4_1)},
{ KM_PRESS|KB_D|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(var4_1)},
{ KM_PRESS|KB_D|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var4_1)},
{ KM_PRESS|KB_D|SHIFT_RS, CONTEXT_ANY,KEYHANDLER_NAME(var4_1)},
{ KM_PRESS|KB_D|SHIFT_RSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var4_1)},
{ KM_LPRESS|KB_D, CONTEXT_ANY,KEYHANDLER_NAME(var4_1)},
{ KM_KEYUP|KB_D, CONTEXT_ANY,KEYHANDLER_NAME(var4_1)},

{ KM_PRESS|KB_E, CONTEXT_ANY,KEYHANDLER_NAME(var5_1)},
{ KM_PRESS|KB_E|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(var5_1)},
{ KM_PRESS|KB_E|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var5_1)},
{ KM_PRESS|KB_E|SHIFT_RS, CONTEXT_ANY,KEYHANDLER_NAME(var5_1)},
{ KM_PRESS|KB_E|SHIFT_RSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var5_1)},
{ KM_LPRESS|KB_E, CONTEXT_ANY,KEYHANDLER_NAME(var5_1)},
{ KM_KEYUP|KB_E, CONTEXT_ANY,KEYHANDLER_NAME(var5_1)},

{ KM_PRESS|KB_F, CONTEXT_ANY,KEYHANDLER_NAME(var6_1)},
{ KM_PRESS|KB_F|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(var6_1)},
{ KM_PRESS|KB_F|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var6_1)},
{ KM_PRESS|KB_F|SHIFT_RS, CONTEXT_ANY,KEYHANDLER_NAME(var6_1)},
{ KM_PRESS|KB_F|SHIFT_RSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(var6_1)},
{ KM_LPRESS|KB_F, CONTEXT_ANY,KEYHANDLER_NAME(var6_1)},
{ KM_KEYUP|KB_F, CONTEXT_ANY,KEYHANDLER_NAME(var6_1)},


    // NORMAL COMMANDS/FUNCTIONS

    { KM_PRESS|KB_Y, CONTEXT_ANY,&invKeyHandler },
    { KM_PRESS|KB_SPC, CONTEXT_ANY,&spcKeyHandler },
    { KM_REPEAT|KB_SPC, CONTEXT_ANY,&spcKeyHandler },
    { KM_PRESS|KB_SPC|SHIFT_ALPHA, CONTEXT_ANY,&spcKeyHandler },
    { KM_REPEAT|KB_SPC|SHIFT_ALPHA, CONTEXT_ANY,&spcKeyHandler },
    { KM_PRESS|KB_SPC|SHIFT_ALPHAHOLD, CONTEXT_ANY,&thinspcKeyHandler },
    { KM_REPEAT|KB_SPC|SHIFT_ALPHAHOLD, CONTEXT_ANY,&thinspcKeyHandler },
    { KM_PRESS|KB_W, CONTEXT_ANY,&chsKeyHandler },
    { KM_PRESS|KB_V, CONTEXT_ANY,&eexKeyHandler },
    { KM_PRESS|KB_ADD|SHIFT_LS, CONTEXT_ANY,&curlyBracketKeyHandler },
    { KM_PRESS|KB_ADD|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&curlyBracketKeyHandler },
    { KM_PRESS|KB_ADD|SHIFT_RS, CONTEXT_ANY,&secoBracketKeyHandler },
    { KM_PRESS|KB_ADD|SHIFT_RS|SHIFT_RSHOLD, CONTEXT_ANY,&secoBracketKeyHandler },
    { KM_PRESS|KB_SUB|SHIFT_LS, CONTEXT_ANY,&parenBracketKeyHandler },
    { KM_PRESS|KB_SUB|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&parenBracketKeyHandler },
    { KM_PRESS|KB_MUL|SHIFT_LS, CONTEXT_ANY,&squareBracketKeyHandler },
    { KM_PRESS|KB_MUL|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&squareBracketKeyHandler },
    { KM_PRESS|KB_MUL|SHIFT_RS, CONTEXT_ANY,&textBracketKeyHandler },
    { KM_PRESS|KB_MUL|SHIFT_RS|SHIFT_RSHOLD, CONTEXT_ANY,&textBracketKeyHandler },
    { KM_PRESS|KB_O, CONTEXT_ANY,&ticksKeyHandler },
    { KM_PRESS|KB_ADD|SHIFT_ALPHA|SHIFT_LS, CONTEXT_ANY,&curlyBracketKeyHandler },
    { KM_PRESS|KB_ADD|SHIFT_ALPHA|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&curlyBracketKeyHandler },
    { KM_PRESS|KB_ADD|SHIFT_ALPHA|SHIFT_RS, CONTEXT_ANY,&secoBracketKeyHandler },
    { KM_PRESS|KB_ADD|SHIFT_ALPHA|SHIFT_RS|SHIFT_RSHOLD, CONTEXT_ANY,&secoBracketKeyHandler },
    { KM_PRESS|KB_SUB|SHIFT_ALPHA|SHIFT_LS, CONTEXT_ANY,&parenBracketKeyHandler },
    { KM_PRESS|KB_SUB|SHIFT_ALPHA|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&parenBracketKeyHandler },
    { KM_PRESS|KB_MUL|SHIFT_ALPHA|SHIFT_LS, CONTEXT_ANY,&squareBracketKeyHandler },
    { KM_PRESS|KB_MUL|SHIFT_ALPHA|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&squareBracketKeyHandler },
    { KM_PRESS|KB_MUL|SHIFT_ALPHA|SHIFT_RS, CONTEXT_ANY,&textBracketKeyHandler },
    { KM_PRESS|KB_MUL|SHIFT_ALPHA|SHIFT_RS|SHIFT_RSHOLD, CONTEXT_ANY,&textBracketKeyHandler },
    { KM_PRESS|KB_O|SHIFT_ALPHA|SHIFT_RS, CONTEXT_ANY,&ticksKeyHandler },
    { KM_PRESS|KB_O|SHIFT_ALPHA|SHIFT_RS|SHIFT_RSHOLD, CONTEXT_ANY,&ticksKeyHandler },
    { KM_PRESS|KB_DOT|SHIFT_LS, CONTEXT_ANY,&tagKeyHandler },
    { KM_PRESS|KB_DOT|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&tagKeyHandler },
    { KM_PRESS|KB_DOT|SHIFT_ALPHA|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(colon) },
    { KM_PRESS|KB_DOT|SHIFT_ALPHA|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(colon) },


    { KM_PRESS|KB_ADD|SHIFT_ONHOLD, CONTEXT_ANY,&onPlusKeyHandler },
    { KM_PRESS|KB_SUB|SHIFT_ONHOLD, CONTEXT_ANY,&onMinusKeyHandler },

    { KM_PRESS|KB_DOT|SHIFT_ONHOLD, CONTEXT_ANY,&onDotKeyHandler },
    { KM_PRESS|KB_SPC|SHIFT_ONHOLD, CONTEXT_ANY,&onSpcKeyHandler },
    { KM_PRESS|KB_MUL|SHIFT_ONHOLD, CONTEXT_ANY,&onMulDivKeyHandler },
    { KM_PRESS|KB_Z|SHIFT_ONHOLD, CONTEXT_ANY,&onMulDivKeyHandler },

    { KM_PRESS|KB_UP|SHIFT_ONHOLD, CONTEXT_ANY,&onUpDownKeyHandler },
    { KM_REPEAT|KB_UP|SHIFT_ONHOLD, CONTEXT_ANY,&onUpDownKeyHandler },
    { KM_PRESS|KB_DN|SHIFT_ONHOLD, CONTEXT_ANY,&onUpDownKeyHandler },
    { KM_REPEAT|KB_DN|SHIFT_ONHOLD, CONTEXT_ANY,&onUpDownKeyHandler },


    { KM_PRESS|KB_0|SHIFT_ONHOLD, CONTEXT_ANY,&onDigitKeyHandler },
    { KM_PRESS|KB_1|SHIFT_ONHOLD, CONTEXT_ANY,&onDigitKeyHandler },
    { KM_PRESS|KB_2|SHIFT_ONHOLD, CONTEXT_ANY,&onDigitKeyHandler },
    { KM_PRESS|KB_3|SHIFT_ONHOLD, CONTEXT_ANY,&onDigitKeyHandler },
    { KM_PRESS|KB_4|SHIFT_ONHOLD, CONTEXT_ANY,&onDigitKeyHandler },
    { KM_PRESS|KB_5|SHIFT_ONHOLD, CONTEXT_ANY,&onDigitKeyHandler },
    { KM_PRESS|KB_6|SHIFT_ONHOLD, CONTEXT_ANY,&onDigitKeyHandler },
    { KM_PRESS|KB_7|SHIFT_ONHOLD, CONTEXT_ANY,&onDigitKeyHandler },
    { KM_PRESS|KB_8|SHIFT_ONHOLD, CONTEXT_ANY,&onDigitKeyHandler },
    { KM_PRESS|KB_9|SHIFT_ONHOLD, CONTEXT_ANY,&onDigitKeyHandler },

    { KM_LPRESS|KB_J|SHIFT_ONHOLD, CONTEXT_ANY,&onVarKeyHandler },
    { KM_PRESS|KB_J|SHIFT_ONHOLD, CONTEXT_ANY,KEYHANDLER_NAME(menuswap) },
    { KM_PRESS|KB_B|SHIFT_ONHOLD, CONTEXT_ANY,&onBKeyHandler },



    { KM_PRESS|KB_0|SHIFT_LS, CONTEXT_ANY,&infinityKeyHandler },
    { KM_PRESS|KB_0|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&undinfinityKeyHandler },
    { KM_PRESS|KB_0|SHIFT_LS|SHIFT_ALPHA, CONTEXT_ANY,&infinityKeyHandler },
    { KM_PRESS|KB_0|SHIFT_LS|SHIFT_LSHOLD|SHIFT_ALPHA, CONTEXT_ANY,&undinfinityKeyHandler },
    { KM_PRESS|KB_0|SHIFT_RS, CONTEXT_ANY,&arrowKeyHandler },
    { KM_PRESS|KB_0|SHIFT_RS|SHIFT_ALPHA, CONTEXT_ANY,&arrowKeyHandler },
    { KM_PRESS|KB_SPC|SHIFT_RS, CONTEXT_ANY,&commaKeyHandler },
    { KM_PRESS|KB_SPC|SHIFT_RS|SHIFT_ALPHA, CONTEXT_ANY,&commaKeyHandler },
    { KM_PRESS|KB_SPC|SHIFT_RS|SHIFT_RSHOLD, CONTEXT_ANY,&semiKeyHandler },
    { KM_PRESS|KB_SPC|SHIFT_RS|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,&semiKeyHandler },
    { KM_PRESS|KB_SUB|SHIFT_RS, CONTEXT_ANY,&underscoreKeyHandler },
    { KM_PRESS|KB_SUB|SHIFT_RS|SHIFT_RSHOLD, CONTEXT_ANY,&underscoreKeyHandler },
    { KM_PRESS|KB_SUB|SHIFT_RS|SHIFT_ALPHA, CONTEXT_ANY,&underscoreKeyHandler },
    { KM_PRESS|KB_S, CONTEXT_ANY,&sinKeyHandler },
    { KM_PRESS|KB_T, CONTEXT_ANY,&cosKeyHandler },
    { KM_PRESS|KB_U, CONTEXT_ANY,&tanKeyHandler },
    { KM_PRESS|KB_S|SHIFT_LS, CONTEXT_ANY,&asinKeyHandler },
    { KM_PRESS|KB_T|SHIFT_LS, CONTEXT_ANY,&acosKeyHandler },
    { KM_PRESS|KB_U|SHIFT_LS, CONTEXT_ANY,&atanKeyHandler },
    { KM_PRESS|KB_S|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&asinKeyHandler },
    { KM_PRESS|KB_T|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&acosKeyHandler },
    { KM_PRESS|KB_U|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&atanKeyHandler },
    { KM_LPRESS|KB_S, CONTEXT_ANY,&sinhKeyHandler },
    { KM_LPRESS|KB_T, CONTEXT_ANY,&coshKeyHandler },
    { KM_LPRESS|KB_U, CONTEXT_ANY,&tanhKeyHandler },
    { KM_LPRESS|KB_S|SHIFT_LS, CONTEXT_ANY,&asinhKeyHandler },
    { KM_LPRESS|KB_T|SHIFT_LS, CONTEXT_ANY,&acoshKeyHandler },
    { KM_LPRESS|KB_U|SHIFT_LS, CONTEXT_ANY,&atanhKeyHandler },
    { KM_LPRESS|KB_S|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&asinhKeyHandler },
    { KM_LPRESS|KB_T|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&acoshKeyHandler },
    { KM_LPRESS|KB_U|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,&atanhKeyHandler },


    { KM_PRESS|KB_N, CONTEXT_ANY,&evalKeyHandler },
    { KM_LPRESS|KB_N, CONTEXT_ANY,&eval1KeyHandler },
    { KM_PRESS|KB_ENT|SHIFT_RS, CONTEXT_ANY,&tonumKeyHandler },
    { KM_PRESS|KB_ENT|SHIFT_RS|SHIFT_RSHOLD, CONTEXT_ANY,&tonumKeyHandler },

    { KM_PRESS|KB_R, CONTEXT_ANY,&sqrtKeyHandler },
    { KM_PRESS|KB_Q, CONTEXT_ANY,&powKeyHandler },
    { KM_PRESS|KB_Q|SHIFT_ALPHA|SHIFT_RS, CONTEXT_ANY,&powKeyHandler },
    { KM_PRESS|KB_M, CONTEXT_ANY,KEYHANDLER_NAME(sto) },
    { KM_LPRESS|KB_M, CONTEXT_ANY,KEYHANDLER_NAME(purge) },
    { KM_PRESS|KB_M|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(rcl) },
    { KM_PRESS|KB_M|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(rcl) },

    { KM_PRESS|KB_Q|SHIFT_LS,CONTEXT_ANY,KEYHANDLER_NAME(exp) },
    { KM_PRESS|KB_Q|SHIFT_LS|SHIFT_LSHOLD,CONTEXT_ANY,KEYHANDLER_NAME(exp) },
    { KM_PRESS|KB_Q|SHIFT_RS,CONTEXT_ANY,KEYHANDLER_NAME(ln) },
    { KM_PRESS|KB_Q|SHIFT_RS|SHIFT_RSHOLD,CONTEXT_ANY,KEYHANDLER_NAME(ln) },

    { KM_PRESS|KB_V|SHIFT_LS,CONTEXT_ANY,KEYHANDLER_NAME(alog) },
    { KM_PRESS|KB_V|SHIFT_LS|SHIFT_LSHOLD,CONTEXT_ANY,KEYHANDLER_NAME(alog) },
    { KM_PRESS|KB_V|SHIFT_RS,CONTEXT_ANY,KEYHANDLER_NAME(log) },
    { KM_PRESS|KB_V|SHIFT_RS|SHIFT_RSHOLD,CONTEXT_ANY,KEYHANDLER_NAME(log) },

    { KM_PRESS|KB_R|SHIFT_LS,CONTEXT_ANY,KEYHANDLER_NAME(sq) },
    { KM_PRESS|KB_R|SHIFT_LS|SHIFT_LSHOLD,CONTEXT_ANY,KEYHANDLER_NAME(sq) },
    { KM_PRESS|KB_R|SHIFT_RS,CONTEXT_ANY,KEYHANDLER_NAME(xroot) },
    { KM_PRESS|KB_R|SHIFT_RS|SHIFT_RSHOLD,CONTEXT_ANY,KEYHANDLER_NAME(xroot) },

    { KM_PRESS|KB_Z|SHIFT_LS,CONTEXT_ANY,KEYHANDLER_NAME(abs) },
    { KM_PRESS|KB_Z|SHIFT_LS|SHIFT_LSHOLD,CONTEXT_ANY,KEYHANDLER_NAME(abs) },
    { KM_PRESS|KB_Z|SHIFT_RS,CONTEXT_ANY,KEYHANDLER_NAME(arg) },
    { KM_PRESS|KB_Z|SHIFT_RS|SHIFT_RSHOLD,CONTEXT_ANY,KEYHANDLER_NAME(arg) },


    { KM_PRESS|KB_UP|SHIFT_LS, CONTEXT_ANY,KEYHANDLER_NAME(updir) },
    { KM_PRESS|KB_UP|SHIFT_LS|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(updir) },
    { KM_PRESS|KB_UP|SHIFT_LS|SHIFT_LSHOLD, CONTEXT_ANY,KEYHANDLER_NAME(home) },
    { KM_PRESS|KB_UP|SHIFT_LS|SHIFT_LSHOLD|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(home) },


    { KM_PRESS|KB_X, CONTEXT_ANY,KEYHANDLER_NAME(keyx) },


    // LETTERS


    { KM_PRESS|KB_A|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(a) },
    { KM_PRESS|KB_B|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(b) },
    { KM_PRESS|KB_C|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(c) },
    { KM_PRESS|KB_D|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(d) },
    { KM_PRESS|KB_E|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(e) },
    { KM_PRESS|KB_F|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(f) },
    { KM_PRESS|KB_G|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(g) },
    { KM_PRESS|KB_H|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(h) },
    { KM_PRESS|KB_I|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(i) },
    { KM_PRESS|KB_J|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(j) },
    { KM_PRESS|KB_K|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(k) },
    { KM_PRESS|KB_L|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(l) },
    { KM_PRESS|KB_M|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(m) },
    { KM_PRESS|KB_N|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(n) },
    { KM_PRESS|KB_O|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(o) },
    { KM_PRESS|KB_P|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(p) },
    { KM_PRESS|KB_Q|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(q) },
    { KM_PRESS|KB_R|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(r) },
    { KM_PRESS|KB_S|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(s) },
    { KM_PRESS|KB_T|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(t) },
    { KM_PRESS|KB_U|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(u) },
    { KM_PRESS|KB_V|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(v) },
    { KM_PRESS|KB_W|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(w) },
    { KM_PRESS|KB_X|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(x) },
    { KM_PRESS|KB_Y|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(y) },
    { KM_PRESS|KB_DIV|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(z) },
    { KM_PRESS|KB_A|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(a) },
    { KM_PRESS|KB_B|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(b) },
    { KM_PRESS|KB_C|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(c) },
    { KM_PRESS|KB_D|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(d) },
    { KM_PRESS|KB_E|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(e) },
    { KM_PRESS|KB_F|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(f) },
    { KM_PRESS|KB_G|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(g) },
    { KM_PRESS|KB_H|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(h) },
    { KM_PRESS|KB_I|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(i) },
    { KM_PRESS|KB_J|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(j) },
    { KM_PRESS|KB_K|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(k) },
    { KM_PRESS|KB_L|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(l) },
    { KM_PRESS|KB_M|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(m) },
    { KM_PRESS|KB_N|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(n) },
    { KM_PRESS|KB_O|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(o) },
    { KM_PRESS|KB_P|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(p) },
    { KM_PRESS|KB_Q|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(q) },
    { KM_PRESS|KB_R|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(r) },
    { KM_PRESS|KB_S|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(s) },
    { KM_PRESS|KB_T|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(t) },
    { KM_PRESS|KB_U|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(u) },
    { KM_PRESS|KB_V|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(v) },
    { KM_PRESS|KB_W|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(w) },
    { KM_PRESS|KB_X|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(x) },
    { KM_PRESS|KB_Y|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(y) },
    { KM_PRESS|KB_DIV|SHIFT_ALPHAHOLD,CONTEXT_ANY, KEYHANDLER_NAME(z) },

    { KM_PRESS|KB_ALPHA|SHIFT_ALPHAHOLD,CONTEXT_ANY, &alphaKeyHandler },

    // SYMBOLS
    { KM_PRESS|KB_9|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(openquestion) },
    { KM_PRESS|KB_9|SHIFT_LS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(delta) },
    { KM_PRESS|KB_9|SHIFT_LS|SHIFT_LSHOLD|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(delta) },

    { KM_PRESS|KB_8|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(openexclamation) },
    { KM_PRESS|KB_1|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(approx) },
    { KM_PRESS|KB_1|SHIFT_LS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(percent) },
    { KM_PRESS|KB_2|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(exclamation) },
    { KM_PRESS|KB_3|SHIFT_LS,CONTEXT_ANY, KEYHANDLER_NAME(hash) },
    { KM_PRESS|KB_3|SHIFT_LS|SHIFT_LSHOLD,CONTEXT_ANY, KEYHANDLER_NAME(hash) },
    { KM_PRESS|KB_3|SHIFT_LS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(hash) },
    { KM_PRESS|KB_3|SHIFT_LS|SHIFT_LSHOLD|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(hash) },

    { KM_PRESS|KB_3|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(question) },
    { KM_PRESS|KB_4|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(euro) },
    { KM_PRESS|KB_4|SHIFT_LS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(dollar) },
    { KM_PRESS|KB_5|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(backslash) },
    { KM_PRESS|KB_5|SHIFT_LS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(pound) },
    { KM_PRESS|KB_6|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(angle) },
    { KM_PRESS|KB_6|SHIFT_LS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(degree) },
    { KM_PRESS|KB_6|SHIFT_LS|SHIFT_LSHOLD|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(degree) },

    { KM_PRESS|KB_SPC|SHIFT_LS,CONTEXT_ANY, KEYHANDLER_NAME(pi) },
    { KM_PRESS|KB_SPC|SHIFT_LS|SHIFT_LSHOLD,CONTEXT_ANY, KEYHANDLER_NAME(pi) },
    { KM_PRESS|KB_SPC|SHIFT_LS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(pi) },
    { KM_PRESS|KB_SPC|SHIFT_LS|SHIFT_LSHOLD|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(pi) },

    { KM_PRESS|KB_ENT|SHIFT_LS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(and) },
    { KM_PRESS|KB_ENT|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(at) },
    { KM_PRESS|KB_ENT|SHIFT_LS|SHIFT_LSHOLD|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(and) },
    { KM_PRESS|KB_ENT|SHIFT_RS|SHIFT_RSHOLD|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(at) },



    { KM_PRESS|KB_W|SHIFT_RS,CONTEXT_ANY, KEYHANDLER_NAME(equal) },
    { KM_PRESS|KB_W|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(equal) },
    { KM_PRESS|KB_W|SHIFT_LS,CONTEXT_ANY, KEYHANDLER_NAME(notequal) },
    { KM_PRESS|KB_W|SHIFT_LS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(notequal) },
    { KM_PRESS|KB_X|SHIFT_RS,CONTEXT_ANY, KEYHANDLER_NAME(ls) },
    { KM_PRESS|KB_X|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(ls) },
    { KM_PRESS|KB_Y|SHIFT_RS,CONTEXT_ANY, KEYHANDLER_NAME(gt) },
    { KM_PRESS|KB_Y|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(gt) },
    { KM_PRESS|KB_X|SHIFT_LS,CONTEXT_ANY, KEYHANDLER_NAME(le) },
    { KM_PRESS|KB_X|SHIFT_LS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(le) },
    { KM_PRESS|KB_Y|SHIFT_LS,CONTEXT_ANY, KEYHANDLER_NAME(ge) },
    { KM_PRESS|KB_Y|SHIFT_LS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(ge) },

    { KM_PRESS|KB_DIV|SHIFT_ALPHA|SHIFT_RS,CONTEXT_ANY, KEYHANDLER_NAME(sdiv) },
    { KM_PRESS|KB_DIV|SHIFT_ALPHA|SHIFT_RSHOLD,CONTEXT_ANY, KEYHANDLER_NAME(sdiv) },




    // NUMBERS

    { KM_PRESS|KB_0|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(sub0) },
    { KM_PRESS|KB_1|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(sub1) },
    { KM_PRESS|KB_2|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(sub2) },
    { KM_PRESS|KB_3|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(sub3) },
    { KM_PRESS|KB_4|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(sub4) },
    { KM_PRESS|KB_5|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(sub5) },
    { KM_PRESS|KB_6|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(sub6) },
    { KM_PRESS|KB_7|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(sub7) },
    { KM_PRESS|KB_8|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(sub8) },
    { KM_PRESS|KB_9|SHIFT_RSHOLD|SHIFT_ALPHA, CONTEXT_ANY,KEYHANDLER_NAME(sub9) },


    // MENUS
    { KM_PRESS|KB_6|SHIFT_RS,CONTEXT_ANY, KEYHANDLER_NAME(unitmenu) },
    { KM_PRESS|KB_N|SHIFT_LS,CONTEXT_ANY, KEYHANDLER_NAME(prgmenu) },
    { KM_PRESS|KB_P,CONTEXT_ANY, KEYHANDLER_NAME(mainmenu) },
    { KM_PRESS|KB_1|SHIFT_LS,CONTEXT_ANY, KEYHANDLER_NAME(arithmenu) },
    { KM_PRESS|KB_1|SHIFT_RS,CONTEXT_ANY, KEYHANDLER_NAME(cplxmenu) },
    { KM_PRESS|KB_9|SHIFT_RS,CONTEXT_ANY, KEYHANDLER_NAME(timemenu) },
    { KM_PRESS|KB_3|SHIFT_RS,CONTEXT_ANY, KEYHANDLER_NAME(basemenu) },
    { KM_PRESS|KB_M|SHIFT_RS,CONTEXT_ANY, KEYHANDLER_NAME(backmenu1) },
    { KM_PRESS|KB_M|SHIFT_RS|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(backmenu1) },
    { KM_PRESS|KB_M|SHIFT_RS|SHIFT_RSHOLD,CONTEXT_ANY, KEYHANDLER_NAME(backmenu2) },
    { KM_PRESS|KB_M|SHIFT_RS|SHIFT_RSHOLD|SHIFT_ALPHA,CONTEXT_ANY, KEYHANDLER_NAME(backmenu2) },

    { 0 , 0 , 0 }
};

// DO CUSTOM KEYBOARD ACTIONS. RETURN 0 IF NO ACTION WAS DEFINED, NONZERO IF SOMETHING WAS EXECUTED
// KEY MESSAGES ARE PROCESSED THROUGH A LIST OF USER DEFINED KEYCODES
// { [KEYMESSAGE] [KEYCONTEXT] [ACTION] ... [KEYMESSAGE2] [KEYCONTEXT2] [ACTION2] ...}
// KEYS ARE IN NO PARTICULAR ORDER
// KEY TABLE IS SCANNED FROM START TO FINISH, NEW KEYS SHOULD BE ADDED TO THE HEAD
// OF THE LIST IN ORDER TO OVERRIDE PREVIOUS DEFINITIONS
// [KEYMESSAGE] AND [KEYCONTEXT] ARE BOTH SINT OBJECTS.
// [ACTION] IS AN ARBITRARY OBJECT THAT WILL BE XEQ'TED.
// HANDLER SCANS THE LIST, LOOKS FOR A MATCH IN KEYMESSAGE AND KEYCONTEXT.
// IF [KEYCONTEXT] IN THE TABLE IS 0, THEN ANY CONTEXT IS CONSIDERED A MATCH.
// ONCE A MATCH IS FOUND, THE [ACTION] OBJECT IS XEQ'TED.
// ONLY THE FIRST MATCH IS EXECUTED, THE SEARCH STOPS THERE.
// IF THE TABLE HAS NO MATCH, THE DEFAULT ACTION HANDLER IS CALLED.
// CUSTOM KEY LIST IS STORED IN Settings

// [ACTION] CAN BE:
// A PROGRAM: A NORMAL SECONDARY WILL BE EXECUTED USING uiCmdRun()
//            A :: ; SECONDARY WILL BE EXECUTED USING uiCmdRunTransparent()
// A LIST IN THE FORM { [COMMAND] }: THE COMMAND WILL BE EXECUTED USING cmdKeyHandler()
// A LIST IN THE FORM { [STRING] }:



int halDoCustomKey(BINT keymsg)
{
    if(rplTestSystemFlag(FL_NOCUSTOMKEYS)) return 0;    // DON'T USE CUSTOM KEYS IF DISABLED PER FLAG


    // TODO: READ THE KEYBOARD TABLE FROM THE Settings DIRECTORY AND DO IT
    WORDPTR keytable;

    keytable=rplGetSettings((WORDPTR)customkey_ident);

    if(!keytable) return 0; // NO CUSTOM KEY DEFINED

    if(!ISLIST(*keytable)) return 0;    // INVALID KEY DEFINITION

    WORDPTR ptr=keytable+1,endoftable=rplSkipOb(keytable),action=0;
    BINT msg,ctx;

    while(ptr<endoftable)
    {
    msg=rplReadNumberAsBINT(ptr);
    if(Exceptions) {
       // CLEAR ALL ERRORS AND KEEP GOING
       rplClearErrors();
       return 0;
    }
    ptr=rplSkipOb(ptr);
    if(ptr>=endoftable) return 0;
    ctx=rplReadNumberAsBINT(ptr);
    if(Exceptions) {
       // CLEAR ALL ERRORS AND KEEP GOING
       rplClearErrors();
       return 0;
    }
    ptr=rplSkipOb(ptr);
    if(ptr>=endoftable) return 0;

    if(msg==keymsg) {
        if(ctx==0) { action=ptr; break; }
        if(!(ctx&0x1f)) {
            if( ctx==(halScreen.KeyContext&~0x1f)) { action=ptr; break; }
        }
        else {
            if(ctx==halScreen.KeyContext) { action=ptr; break; }
        }
    }
    ptr=rplSkipOb(ptr);
    }

    if(action) {
        // EXECUTE THE REQUESTED ACTION
        customKeyHandler(keymsg,action);
        return 1;
    }

    return 0;
}

// RETURN TRUE/FALSE IF A CUSTOM HANDLER EXISTS
int halCustomKeyExists(BINT keymsg)
{
    WORDPTR keytable;

    keytable=rplGetSettings((WORDPTR)customkey_ident);

    if(!keytable) return 0; // NO CUSTOM KEY DEFINED

    if(!ISLIST(*keytable)) return 0;    // INVALID KEY DEFINITION

    WORDPTR ptr=keytable+1,endoftable=rplSkipOb(keytable);
    BINT msg,ctx;

    while(ptr<endoftable)
    {
    msg=rplReadNumberAsBINT(ptr);
    if(Exceptions) {
       // CLEAR ALL ERRORS AND KEEP GOING
       rplClearErrors();
       return 0;
    }
    ptr=rplSkipOb(ptr);
    if(ptr>=endoftable) return 0;
    ctx=rplReadNumberAsBINT(ptr);
    if(Exceptions) {
       // CLEAR ALL ERRORS AND KEEP GOING
       rplClearErrors();
       return 0;
    }
    if(msg==keymsg) {
        if(ctx==0) return 1;
        if(!(ctx&0x1f)) {
            if( ctx==(halScreen.KeyContext&~0x1f)) return 1;
        }
        else {
            if(ctx==halScreen.KeyContext) return 1;
        }
    }
    ptr=rplSkipOb(ptr);
    if(ptr>=endoftable) return 0;
    ptr=rplSkipOb(ptr);
    }

    //   SCANNED ENTIRE TABLE, NO LUCK
    return 0;
}


// CONTEXT MATCH FOR KEYS:
// IF CONTEXT == 0 (CONTEXT_ANY) IT'S CONSIDERED A MATCH TO ALL CONTEXTS/SUBCONTEXTS
// IF CONTEXT HAS A SUBCONTEXT, THEN IT ONLY MATCHES THE CONTEXT AND SUBCONTEXT EXACTLY
// IF CONTEXT DOESN'T HAVE A SUBCONTEXT (SUBCONTEXT_ANY), THEN IT MATCHES ALL SUBCONTEXTS WITHIN
// THE GIVEN MAIN CONTEXT.



int halDoDefaultKey(BINT keymsg)
{
struct keyhandler_t *ptr=(struct keyhandler_t *)__keydefaulthandlers;

while(ptr->action) {
    if(ptr->message==keymsg) {
        // CHECK IF CONTEXT MATCHES
        if((!ptr->context) || (ptr->context==halScreen.KeyContext) ||
                ( !(ptr->context&0x1f)&&(ptr->context==(halScreen.KeyContext&~0x1f)))) {
            //  IT'S A MATCH, EXECUTE THE ACTION;
            (ptr->action)(keymsg);
            return 1;
        }
        }
    ++ptr;
}
return 0;
}

// RETURN TRUE/FALSE IF A DEFAULT HANDLER EXISTS
int halDefaultKeyExists(BINT keymsg)
{
    struct keyhandler_t *ptr=(struct keyhandler_t *)__keydefaulthandlers;

    while(ptr->action) {
        if(ptr->message==keymsg) {
            // CHECK IF CONTEXT MATCHES
            if((!ptr->context) || (ptr->context==halScreen.KeyContext) ||
                ( !(ptr->context&0x1f)&&(ptr->context==(halScreen.KeyContext&~0x1f))))
            {
                //  IT'S A MATCH;
                return 1;
            }
            }
        ++ptr;
    }
    return 0;

}



// PROCESSES KEY MESSAGES AND CALL APPROPRIATE HANDLERS BY KEYCODE

// RETURNS 0 IF THE LOOP HAS TO CONTINUE, 1 TO TERMINATE OUTER LOOP




int halProcessKey(BINT keymsg, int (*dokey)(BINT),BINT flags)
{
    int wasProcessed;

    if(!keymsg) return 0;

    if(KM_MESSAGE(keymsg)==KM_SHIFT) {
        // THERE WAS A CHANGE IN SHIFT PLANE, UPDATE ANNUNCIATORS
        if(KM_SHIFTPLANE(keymsg)&SHIFT_LS) {
            if((KM_SHIFTPLANE(keymsg)&SHIFT_HOLD)) halSetNotification(N_LEFTSHIFT,0xf);
            else halSetNotification(N_LEFTSHIFT,0x8);
        } else halSetNotification(N_LEFTSHIFT,0);
        if(KM_SHIFTPLANE(keymsg)&SHIFT_RS) {
            if((KM_SHIFTPLANE(keymsg)&SHIFT_HOLD)) halSetNotification(N_RIGHTSHIFT,0xf);
            else halSetNotification(N_RIGHTSHIFT,0x8);
        } else halSetNotification(N_RIGHTSHIFT,0);
        if(KM_SHIFTPLANE(keymsg)&SHIFT_ALPHA) {
            if((KM_SHIFTPLANE(keymsg)&SHIFT_ALHOLD)) halSetNotification(N_ALPHA,0xf);
            else halSetNotification(N_ALPHA,0x8);
        } else halSetNotification(N_ALPHA,0);

        // UPDATE EDITOR MODE ACCORDINGLY
        int oldplane=OLDKEYSHIFT(keymsg);
        if( KM_SHIFTPLANE(keymsg^oldplane)&SHIFT_ALPHA) {
            // THERE WAS A CHANGE IN ALPHA MODE
            halSwapCmdLineMode(KM_SHIFTPLANE(keymsg)&SHIFT_ALPHA);
        }
        else {
            // NO CHANGE IN ALPHA STATE
            if(KM_SHIFTPLANE(oldplane)&SHIFT_ALPHALOCK) {
            if((KM_SHIFTPLANE(keymsg^oldplane)&SHIFT_ALPHAHOLD)==SHIFT_ALHOLD) {
                // CHECK GOING FROM ALPHA TO ALPHA-HOLD OR VICEVERSA
                // TEMPORARILY CHANGE SHIFT STATE
                alphaKeyHandler(0);
            }
            }
        }

        return 0;

    }

    // THIS ALLOWS KEYS WITH LONG PRESS DEFINITION TO POSTPONE
    // EXECUTION UNTIL THE KEY IS RELEASED
    if(halLongKeyPending) {
        // THERE WAS A KEY PENDING EXECUTION
        if( (KM_MESSAGE(keymsg)==KM_LPRESS) && (KM_KEY(keymsg)==KM_KEY(halLongKeyPending))) {
            // WE RECEIVED A LONG PRESS ON THAT KEY, DISCARD THE OLD EVENT AND DO A LONG PRESS ONLY
           halLongKeyPending=0;
        }
        else {
            // ANY OTHER MESSAGE SHOULD CAUSE THE EXECUTION OF THE OLD KEY FIRST, THEN THE NEW ONE

            BINT tmp=halLongKeyPending;
            halLongKeyPending=0;     // THIS CLEANUP IS ONLY NEEDED IN CASE THE KEY HANDLER CALLS A KEYBOARD LOOP

            if(dokey) wasProcessed=(*dokey)(tmp);
            else wasProcessed=0;

            if(!(flags&OL_NOCUSTOMKEYS)) if(!wasProcessed) wasProcessed=halDoCustomKey(tmp);
            if(!(flags&OL_NODEFAULTKEYS)) if(!wasProcessed) wasProcessed=halDoDefaultKey(tmp);


            if(wasProcessed<0) return 1;        // DON'T EXECUTE THE NEW KEY IF THIS ONE WANTS TO END THE LOOP
        }

    }

    // BEFORE EXECUTING, CHECK IF THIS KEY HAS A LONG PRESS ASSIGNMENT
    // AND IF SO, DELAY EXECUTION

    if(KM_MESSAGE(keymsg)==KM_PRESS) {
        if(flags&OL_LONGPRESS) {
            // ALL KEYS WAIT FOR A LONG PRESS EVENT
            halLongKeyPending=keymsg;
            return 0;
        } else {
            // ONLY KEYS THAT HAVE LONG PRESS DEFINITION WILL WAIT, OTHERWISE EXECUTE IMMEDIATELY
        BINT longmsg=KM_LPRESS | KM_SHIFTEDKEY(keymsg);

        if(halCustomKeyExists(longmsg)) { halLongKeyPending=keymsg; return 0; }
        if(halDefaultKeyExists(longmsg)) { halLongKeyPending=keymsg; return 0; }
        }
    }


    if(dokey) wasProcessed=(*dokey)(keymsg);
    else wasProcessed=0;
    if(!(flags&OL_NOCUSTOMKEYS)) if(!wasProcessed) wasProcessed=halDoCustomKey(keymsg);
    if(!(flags&OL_NODEFAULTKEYS)) if(!wasProcessed) wasProcessed=halDoDefaultKey(keymsg);

    // *************** DEBUG ONLY ************
/*
    if(!wasProcessed && ((KM_MESSAGE(keymsg)==KM_PRESS)||(KM_MESSAGE(keymsg)==KM_LPRESS)||(KM_MESSAGE(keymsg)==KM_REPEAT))) {

    // ALL OTHER KEYS, JUST DISPLAY THE KEY NAME ON SCREEN
        DRAWSURFACE scr;
        ggl_initscr(&scr);
        UNIFONT *fnt=halScreen.StAreaFont;

    // FOR DEBUG ONLY
    int width=StringWidth((char *)keyNames[KM_KEY(keymsg)],fnt);
    int ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
    // CLEAR STATUS AREA AND SHOW KEY THERE
    ggl_rect(&scr,STATUSAREA_X,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);
    DrawTextBk(SCREEN_WIDTH-width,ytop+halScreen.Menu2/2,(char *)keyNames[KM_KEY(keymsg)],fnt,15,0,&scr);
    char *shiftstr;
    switch(KM_SHIFTPLANE(keymsg))
    {
    case SHIFT_LS:
        shiftstr="(LS)";
        break;
    case SHIFT_LS|SHIFT_LSHOLD:
        shiftstr="(LSH)";
        break;
    case SHIFT_RS:
        shiftstr="(RS)";
        break;
    case SHIFT_RS|SHIFT_RSHOLD:
        shiftstr="(RSH)";
        break;
    case SHIFT_ALPHA:
        shiftstr="(AL)";
        break;
    case SHIFT_ALPHA|SHIFT_ALPHAHOLD:
        shiftstr="(ALH)";
        break;
    case SHIFT_ONHOLD:
        shiftstr="(ONH)";
        break;
    case SHIFT_ALPHA|SHIFT_LS:
        shiftstr="(AL-LS)";
        break;
    case SHIFT_ALPHA|SHIFT_RS:
        shiftstr="(AL-RS)";
        break;
    case SHIFT_ALPHA|SHIFT_LSHOLD:
        shiftstr="(AL-LSH)";
        break;
    case SHIFT_ALPHA|SHIFT_RSHOLD:
        shiftstr="(AL-RSH)";
        break;

    default:
        shiftstr="";
    }
    DrawTextBk(SCREEN_WIDTH-width-32,ytop+halScreen.Menu2/2,shiftstr,fnt,15,0,&scr);

    if(KM_MESSAGE(keymsg)==KM_LPRESS) DrawTextBk(SCREEN_WIDTH-width-42,ytop+halScreen.Menu2/2,"L=",fnt,15,0,&scr);

    }

*/
    // ONLY RETURN 1 WHEN THE OUTER LOOP IS SUPPOSED TO END
    if(wasProcessed<0) return 1;
    else return 0;
}

// THIS FUNCTION RETURNS WHEN THE FORM CLOSES, OR THE USER EXITS WITH THE ON KEY

void halOuterLoop(BINT timeoutms, int (*dokey)(BINT), BINT flags)
{
    int keymsg,isidle,jobdone;
    BINT64 offcounter;

    DRAWSURFACE scr;
    ggl_initscr(&scr);
    jobdone=isidle=0;
    do {
        halRedrawAll(&scr);
        if(!(flags&OL_NOEXIT) && halExitOuterLoop()) break;
        if(Exceptions) {
            halShowErrorMsg();
            Exceptions=0;
        }

        keymsg=halWaitForKeyTimeout(timeoutms);
        timeoutms=0;

        if(keymsg<0) break; // TIMED OUT


        if(!keymsg) {
            // SOMETHING OTHER THAN A KEY WOKE UP THE CPU


            if(!isidle) offcounter=halTicks();

            // FLUSH FILE SYSTEM CACHES WHEN IDLING FOR MORE THAN 3 SECONDS
            if(!(flags&OL_NOSDFLUSH) && !(jobdone&1) && FSIsInit()) {

            if(halTicks()-offcounter >=3000000) {
                if(FSIsDirty()) { FSFlushAll(); halUpdateStatus(); }
                jobdone|=1;
                isidle=0;
            }

            }


            // AUTO-OFF WHEN IDLING
            if(!(flags&OL_NOAUTOOFF) && (halFlags&HAL_AUTOOFFTIME)) {
            BINT64 autoofftime=15000000 << (GET_AUTOOFFTIME(halFlags));
            if(halTicks()-offcounter >=autoofftime) {
                halPreparePowerOff();
                halEnterPowerOff();
            }
            }


            if(!(flags&OL_NOALARM)) {
            if(halCheckSystemAlarm()) {
                jobdone=isidle=0;
                halTriggerAlarm();
            }
            }

            // DO OTHER IDLE PROCESSING HERE


            isidle++;
            if(isidle>10) isidle=1;     //  COUNT UP TO TEN IDLE CYCLES

        } else { jobdone=isidle=0; }


        halSetBusyHandler();

        if((isidle==10) && (halFlags&HAL_AUTORESUME)) {
            uiCmdRun(CMD_CONT);   // AUTOMATICALLY CONTINUE EXECUTION AFTER 10 IDLE CYCLES
            halScreen.DirtyFlag|=CMDLINE_ALLDIRTY|STACK_DIRTY|STAREA_DIRTY|MENU1_DIRTY|MENU2_DIRTY|FORM_DIRTY;
        }

    } while(!halProcessKey(keymsg,dokey,flags));

}


void halInitKeyboard()
{
    keyb_setalphalock(1);
}


// API USED BY RPL PROGRAMS TO INSERT KEY SEQUENCES TO THE KEYBOARD

void halPostKeyboardMessage(BINT keymsg)
{
    //  POST A COMPLETE KEY SEQUENCE TO PREVENT PROBLEMS.

    switch(KM_MESSAGE(keymsg))
    {
    case KM_PRESS:
    {
        keyb_postmsg(KM_KEYDN | (keymsg^(KM_MESSAGE(keymsg))));
        keyb_postmsg(keymsg);
        keyb_postmsg(KM_KEYUP | (keymsg^(KM_MESSAGE(keymsg))));
        break;

    }
    case KM_LPRESS:
    {
        keyb_postmsg(KM_KEYDN | (keymsg^(KM_MESSAGE(keymsg))));
        keyb_postmsg(KM_PRESS | (keymsg^(KM_MESSAGE(keymsg))));
        keyb_postmsg(keymsg);
        keyb_postmsg(KM_KEYUP | (keymsg^(KM_MESSAGE(keymsg))));
        break;
    }
    default:
        keyb_postmsg(keymsg);
    }

}
