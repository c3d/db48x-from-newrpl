/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// THIS IS THE MAIN STABLE API FOR KEYBOARD ACCESS
// KEYBOARD RESPONSE IS CUSTOMIZED FOR HP PRIME G1 TARGET

#include <cmdcodes.h>
#include <fsystem.h>
#include <libraries.h>
#include <newrpl.h>
#include <sysvars.h>
#include <ui.h>
#include <stdio.h>


RECORDER(keys, 16, "Keyboard handling");
RECORDER(keys_debug, 16, "Detailed debugging");

// For use in the debugger, which does not know about macros
const keyb_msg_t     km_press          = KM_PRESS;
const keyb_msg_t     km_repeat         = KM_REPEAT;
const keyb_msg_t     km_long_press     = KM_LONG_PRESS;
const keyb_msg_t     km_long_or_repeat = KM_LONG_OR_REPEAT;
const keyb_msg_t     km_keydn          = KM_KEYDN;
const keyb_msg_t     km_keyup          = KM_KEYUP;

// Convenience macro to name key handlers
#define KH(name)        name##KeyHandler


static inline word_p rplDecompileAnyway(word_p object, int32_t flags)
// ----------------------------------------------------------------------------
//   Decomppile an RPL object
// ----------------------------------------------------------------------------
{
    int32_t SavedException = Exceptions;
    int32_t SavedErrorCode = ErrorCode;

     // Erase any previous error to allow the decompiler to run
    Exceptions = 0;

    // Do not save iptr because it can move
    word_p opname = rplDecompile(object, flags);

    Exceptions = SavedException;
    ErrorCode = SavedErrorCode;

    return opname;
}


static int32_t rplGetStringPointers(word_p object, utf8_p *start, utf8_p *end)
// ----------------------------------------------------------------------------
// Sets pointers to string. Returns string length in code points
// ----------------------------------------------------------------------------
{
    *start = (utf8_p) (object + 1);
    int32_t totaln = rplStrLenCp(object);
    *end = utf8nskip((char *)*start, (char *)rplSkipOb(object), totaln);
    return totaln;
}


static int32_t rplGetDecompiledString(word_p  object,
                                      int32_t flags,
                                      utf8_p *start,
                                      utf8_p *end)
// ----------------------------------------------------------------------------
// Decompiles object and sets pointers to resulting string
// ----------------------------------------------------------------------------
// Returns 0 on error with target pointers set to null
// Returns string length in code points if ok with target pointers set
{
    word_p opname = rplDecompileAnyway(object, flags);
    if (!opname)
    {
        *start = NULL;
        *end = NULL;
        return 0;
    }

    return rplGetStringPointers(opname, start, end);
}


static int rplGetDecompiledStringWithoutTickmarks(word_p  object,
                                                  int32_t flags,
                                                  utf8_p *start,
                                                  utf8_p *end)
// ----------------------------------------------------------------------------
// Decompiles object to string with tickmarks removed
// ----------------------------------------------------------------------------
// returns 0 on error with target pointers set to null
// returns 1 if ok with target pointers set
{
    int32_t totaln = rplGetDecompiledString(object, flags, start, end);
    if (!totaln)
        return 0;

    // In algebraic mode, remove the tick marks and insert without separation
    // to allow pasting equations into other expressions
    if ((totaln > 2) && ((*start)[0] == '\''))
    {
        ++(*start);
        --(*end);
    }

    return 1;
}


static inline utf8_p halHelpMessage(utf8_p topic)
// ----------------------------------------------------------------------------
//   Find the help message associated with the topic
// ----------------------------------------------------------------------------
{
    static const char helpfile[] = {
#include "helpfile.inc"
    };
    size_t len = strlen(topic);
    for (utf8_p ptr = helpfile; *ptr; ptr++)
    {
        if (ptr[0] == '\n' && ptr[1] == '#')
        {
            utf8_p help = ptr + 2;
            while (*help && (*help == '#' || *help == ' '))
                help++;
            if (strncmp(help, topic, len) == 0 && help[len] == '\n')
                return help;
        }
    }

    // Help topic not found - Say so
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "No help for %s", topic);
    return buffer;
}


#define halRepeatingKey(keymsg)                                         \
/* ----------------------------------------------------------------- */ \
/* Indicate that a key can repeat                                    */ \
/* ----------------------------------------------------------------- */ \
    do                                                                  \
    {                                                                   \
        keyb_msg_t msg = KM_MESSAGE(keymsg);                            \
        if (msg & KFLAG_LONG_PRESS)                                     \
            keyb_flags |= KFLAG_REPEAT;                                 \
        /* Repeating keys act on keydown, not on key press */           \
        if (msg == KM_PRESS ||                                          \
            (msg & KFLAG_CHANGED) && (msg & KFLAG_PRESSED) == 0)        \
            return;                                                     \
    } while (0)


#define halKeyHelp(keymsg, command)                                         \
    /* ----------------------------------------------------------------- */ \
    /*  Specify the help message for a key                               */ \
    /* ----------------------------------------------------------------- */ \
    /*  Key down   : Show the short help (name of the command)           */ \
    /*  Key up     : Clear the help and cancel execution                 */ \
    /*  Long Press : Show the extended help message (and cancel)         */ \
    do                                                                      \
    {                                                                       \
        keyb_msg_t msg = KM_MESSAGE(keymsg);                                \
                                                                            \
        if (msg & KFLAG_CHANGED)                                            \
        {                                                                   \
            if (msg & KFLAG_PRESSED)                                        \
                halScreen.ShortHelpMessage = (command);                     \
            else                                                            \
                halScreen.ShortHelpMessage = NULL;                          \
            halRefresh(HELP_DIRTY);                                         \
            return;                                                         \
        }                                                                   \
        if (msg & KFLAG_LONG_PRESS)                                         \
        {                                                                   \
            halScreen.HelpMessage = halHelpMessage(command);                \
            halScreen.HelpLine    = 0;                                      \
            halRefresh(HELP_DIRTY);                                         \
            return;                                                         \
        }                                                                   \
    } while (0)


#define halOpenCmdLine(keymsg, cursorstart)                             \
/* ----------------------------------------------------------------- */ \
/*  Shared code for opening a command line                           */ \
/* ----------------------------------------------------------------- */ \
/*  Key down   : Show the short help (name of the command)           */ \
/*  Key up     : Clear the help and cancel execution                 */ \
/*  Long Press : Show the extended help message (and cancel)         */ \
    do                                                                  \
    {                                                                   \
        halSetCmdLineHeight(CMDLINE_HEIGHT);                            \
        halSetContext(CONTEXT_EDITOR);                                  \
        int alpha = KM_SHIFT(keymsg) & KSHIFT_ALPHA;                    \
        uiOpenCmdLine(alpha ? 'X' : (cursorstart));                     \
    } while(0);


#define halKeyOpensEditor(keymsg)                                       \
/* ----------------------------------------------------------------- */ \
/*  Shared code for keys that open the editor                        */ \
/* ----------------------------------------------------------------- */ \
/*  Key down   : Show the short help (name of the command)           */ \
/*  Key up     : Clear the help and cancel execution                 */ \
/*  Long Press : Show the extended help message (and cancel)         */ \
    do                                                                  \
    {                                                                   \
        if (!halContext(CONTEXT_EDITOR))                                \
        {                                                               \
            if (halContext(CONTEXT_FORM | CONTEXT_INTERACTIVE_STACK))   \
                return;                                                 \
            halOpenCmdLine(keymsg, 'D');                                \
        }                                                               \
    } while (0)


static void keybTimeoutHandler()
// ----------------------------------------------------------------------------
//   Handler that simply records the timeout and does nothing else
// ----------------------------------------------------------------------------
{
    halFlags |= HAL_TIMEOUT;
}


keyb_msg_t halWaitForKeyTimeout(int32_t timeoutms)
// ----------------------------------------------------------------------------
//   Wait for a key for the given amount of time
// ----------------------------------------------------------------------------
// Use timeoutms <=0 to continue waiting for a previously scheduled timeout
// in case other event wakes up the cpu
{
    keyb_msg_t keymsg = 0;
    int        wokeup = 0;

    if (!(halFlags & HAL_FASTMODE) && (halBusyEvent >= 0))
    {
        tmr_event_kill(halBusyEvent);
        halBusyEvent = -1;
    }

    // Start a timer to provide proper timeout
    if (timeoutms > 0)
    {
        halFlags &= ~HAL_TIMEOUT;
        halTimeoutEvent = tmr_event_create(&keybTimeoutHandler, timeoutms, 0);
    }

    for(;;)
    {
        // Fetch current key if any, and return if we have it
        keymsg = keyb_get_message();
        if (keymsg)
            return keymsg;

        // Enter low speed mode
        if (rplTestSystemFlag(FL_QUICKRESPONSE))
            halFlags |= HAL_QUICKRESPONSE;
        else
            halFlags &= ~HAL_QUICKRESPONSE;

        if (halFlags & HAL_FASTMODE)
        {
            halCPUSlowMode();
            halFlags &= ~HAL_FASTMODE;
        }
        if (halFlags & HAL_HOURGLASS)
        {
            halSetNotification(N_HOURGLASS, 0);
            halFlags &= ~HAL_HOURGLASS;
            halScreenUpdated();
        }

        if (wokeup)
        {
            // Check if we woke up because of a timeout
            if (timeoutms && (halFlags & HAL_TIMEOUT))
            {
                halFlags &= ~HAL_TIMEOUT;
                return HAL_KEY_TIMEOUT;
            }

            // Otherwise allow screen refresh requested by other irqs
            return HAL_KEY_WAKEUP;
        }

        // Wait for an interrupt to wake us up
        cpu_waitforinterrupt();
        wokeup = 1;
    }

    return HAL_KEY_TIMEOUT;
}


keyb_msg_t halWaitForKey()
// ----------------------------------------------------------------------------
// Wait for a key to be pressed in slow mode
// ----------------------------------------------------------------------------
{
    return halWaitForKeyTimeout(0);
}


inline void halSetContext(keyboard_context_t KeyContext)
// ----------------------------------------------------------------------------
//  Set a given context bit
// ----------------------------------------------------------------------------
{
    halScreen.KeyContext |= KeyContext;
}


inline void halExitContext(keyboard_context_t KeyContext)
// ----------------------------------------------------------------------------
//  Clear a given context bit
// ----------------------------------------------------------------------------
{
    halScreen.KeyContext &= ~KeyContext;
}


inline keyboard_context_t halContext(keyboard_context_t mask)
// ----------------------------------------------------------------------------
//   Check which context we are in
// ----------------------------------------------------------------------------
{
    return halScreen.KeyContext & mask;
}


void halSwapCmdLineMode(int32_t isalpha)
// ----------------------------------------------------------------------------
//   Toggle between Alpha and another mode, e.g. D or A
// ----------------------------------------------------------------------------
{
    unsigned state = halScreen.CursorState;
    BYTE mode = (BYTE) halScreen.CursorState;
    BYTE old = (BYTE) (state >> 24);
    if (mode == 'L' || mode == 'C')
    {
        // If we are already in alpha mode, don't change
        if (isalpha)
            return;
    }
    else
    {
        // If we are already not in alpha mode, stay there
        if (!isalpha)
            return;

        // Lock uppercase mode when entering alpha
        old = 'C';
    }

    state = (mode << 24) | (state & 0x00FFFF00) | old;
    halScreen.CursorState = state;
}


void halSetCmdLineMode(BYTE mode)
// ----------------------------------------------------------------------------
//   Change the command line mode
// ----------------------------------------------------------------------------
// 'A': Algebraic
// 'C': Alphabetic capitals
// 'D': Direct mode (i.e. key press on a function key executes it)
// 'L': Alphabetic lowercase
// 'P': Program mode
// 'X': Alphabetic, use last mode
{
    BYTE old = (BYTE) halScreen.CursorState;
    if (mode != old)
    {
        halScreen.CursorState = (halScreen.CursorState & ~0xff) | mode;
        halRefresh(CMDLINE_CURSOR_DIRTY);
    }
}

BYTE halGetCmdLineMode()
{
    return halScreen.CursorState & 0xff;
}

void halForceAlphaModeOn()
{
    halSwapCmdLineMode(1);
    keyb_set_shift_plane(KSHIFT_ALPHA);
}

void halForceAlphaModeOff()
{
    halSwapCmdLineMode(0);
    keyb_set_shift_plane(KSHIFT_NONE);
}

// DEBUG: DO-NOTHING KEYBOARD HANDLER
void KH(dummy)(keyb_msg_t keymsg)
{
    halKeyHelp(keymsg, "dummy");
    return;
}

// END THE CURRENTLY OPEN COMMAND LINE, RETURN 1 IF COMPILED SUCCESSFULLY
// 0 IF ERROR.
// WHEN 1, THE STACK CONTAINS THE OBJECT/S COMPILED
// WHEN 0, THE COMMAND LINE IS STILL OPEN, WITH THE ERROR HIGHLIGHTED
int32_t endCmdLineAndCompile()
{
    word_p text = uiGetCmdLineText();
    if(!text) {
        throw_dbgexception("No memory for command line",
                EX_CONT | EX_WARM | EX_RESET);
        return 0;
    }
    int32_t len = rplStrSize(text);
    word_p newobject;
    if(len) {
        newobject = rplCompile((utf8_p) (text + 1), len, 1);
        if(Exceptions || (!newobject)) {
            // HIGHLIGHT THE WORD THAT CAUSED THE ERROR

            char *mainbuffer = (char *)(CmdLineText + 1);

            // COUNT LINES UNTIL THE TOKEN ERROR

            char *position = (char *)TokenStart;
            char *linestart = NULL;

            // COMPUTE LINE NUMBER
            int linenum = 1;

            while(position > mainbuffer) {
                --position;
                if(*position == '\n') {
                    ++linenum;
                    if(!linestart)
                        linestart = position + 1;
                }
            }

            // COUNT CHARACTERS FROM START OF LINE
            position = (char *)TokenStart;
            if(!linestart)
                linestart = mainbuffer;

            while(*linestart == '\r')
                ++linestart;

            int posnum = utf8nlen(linestart, position) + 1;

            // HERE linenum HAS THE LINE NUMBER OF THE TOKEN CAUSING THE ERROR
            // AND posnum IS THE POSITION OF THE FIRST CHARACTER OF THE TOKEN

            WORD SavedExceptions = Exceptions;

            Exceptions = 0;

            uiSetCurrentLine(linenum);
            uiCursorStartOfLine();
            uiCursorRight(posnum - 1);

            if(!Exceptions)
                Exceptions = SavedExceptions;

            WORD fakeprogram = 0;
            ExceptionPointer = &fakeprogram;
            return 0;
        }
        else {
            // END ALPHA MODE
            halSwapCmdLineMode(0);
            keyb_set_shift_plane(KSHIFT_NONE);
            if(uiGetCmdLineState() & CMDSTATE_OVERWRITE) {
                if(rplDepthData() >= 1)
                    rplDropData(1);
            }
            uiCloseCmdLine();
            halSetCmdLineHeight(0);
            halExitContext(CONTEXT_EDITOR);
            // RUN THE OBJECT

            rplSetEntryPoint(newobject);

            // RUN AND CLEANUP PROPERLY
            int32_t rstksave = RSTop - RStk, lamsave = LAMTop - LAMs, nlambase =
                    nLAMBase - LAMs;
            int32_t result = rplRun();

            switch (result) {
            case CLEAN_RUN:
            {
                // SOMEBODY CALLED EXITRPL EXPLICITLY
                // EVERYTHING WAS COMPLETELY CLEANED UP AND RESET
                halFlags &= ~(HAL_HALTED | HAL_AUTORESUME | HAL_FASTAUTORESUME);
                break;
            }
            case NEEDS_CLEANUP:
            {
                // UNTRAPPED ERROR
                // CLEANUP ANY GARBAGE AFTER OUR SAVED POINTER
                if(RSTop >= RStk + rstksave) {
                    RSTop = RStk + rstksave;
                }
                else {
                    rplCleanup();
                    halFlags &=
                            ~(HAL_HALTED | HAL_AUTORESUME | HAL_FASTAUTORESUME);
                }
                if(LAMTop > LAMs + lamsave)
                    LAMTop = LAMs + lamsave;
                if(nLAMBase > LAMs + nlambase)
                    nLAMBase = LAMs + nlambase;
                break;
            }

            case CODE_HALTED:
            {
                // UNTRAPPED ERROR
                // CLEANUP ANY GARBAGE AFTER OUR SAVED POINTER
                if(RSTop > RStk + rstksave) {
                    // THE CODE HALTED SOMEWHERE INSIDE!
                    halFlags |= HAL_HALTED;
                    if(Exceptions & EX_POWEROFF)
                        halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                    if(Exceptions & EX_HALRESET)
                        halFlags |= HAL_RESET;
                    if(Exceptions & EX_HWRESET)
                        halFlags |= HAL_HWRESET;
                    if(Exceptions & EX_AUTORESUME) {
                        halFlags |= HAL_AUTORESUME;
                        Exceptions = 0;
                    }
                }
                else {
                    if(RSTop < RStk + rstksave) {

                        // THIS CAN ONLY HAPPEN IF SOMEHOW
                        // THE CODE ESCAPED FROM THE SECONDARY
                        // WE CREATED, THIS CAN HAPPEN WHEN USING 'CONT'
                        // INSIDE A SECONDARY AND IT'S NOT NECESSARILY BAD
                        if(CurOpcode == CMD_ENDOFCODE) {
                            rplClearErrors();
                            rplCleanup();
                        }
                        if(HaltedIPtr) {
                            halFlags |= HAL_HALTED;
                            if(Exceptions & EX_POWEROFF)
                                halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                            if(Exceptions & EX_HALRESET)
                                halFlags |= HAL_RESET;
                            if(Exceptions & EX_HWRESET)
                                halFlags |= HAL_HWRESET;
                            if(Exceptions & EX_AUTORESUME) {
                                halFlags |= HAL_AUTORESUME;
                                Exceptions = 0;
                            }
                        }
                        else {
                            halFlags &=
                                    ~(HAL_HALTED | HAL_AUTORESUME |
                                    HAL_FASTAUTORESUME);
                            if(Exceptions & EX_POWEROFF)
                                halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                            if(Exceptions & EX_HALRESET)
                                halFlags |= HAL_RESET;
                            if(Exceptions & EX_HWRESET)
                                halFlags |= HAL_HWRESET;
                        }

                    }
                    else {
                        // RETURN STACK WAS INTACT, RESTORE THE REST
                        if(LAMTop > LAMs + lamsave)
                            LAMTop = LAMs + lamsave;
                        if(nLAMBase > LAMs + nlambase)
                            nLAMBase = LAMs + nlambase;

                        // DON'T ALTER THE INSTRUCTION POINTER OF THE HALTED PROGRAM
                        if(CurOpcode == CMD_ENDOFCODE)
                            rplClearErrors();
                        if(HaltedIPtr) {
                            halFlags |= HAL_HALTED;
                            if(Exceptions & EX_POWEROFF)
                                halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                            if(Exceptions & EX_HALRESET)
                                halFlags |= HAL_RESET;
                            if(Exceptions & EX_HWRESET)
                                halFlags |= HAL_HWRESET;

                            if(Exceptions & EX_AUTORESUME) {
                                halFlags |= HAL_AUTORESUME;
                                Exceptions = 0;
                            }
                        }
                        else {
                            halFlags &=
                                    ~(HAL_HALTED | HAL_AUTORESUME |
                                    HAL_FASTAUTORESUME);
                            if(Exceptions & EX_POWEROFF)
                                halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                            if(Exceptions & EX_HALRESET)
                                halFlags |= HAL_RESET;
                            if(Exceptions & EX_HWRESET)
                                halFlags |= HAL_HWRESET;
                        }

                    }
                }

                break;
            }

            }

            // Everything went fine, close the command line
            return 1;

        }
    }
    else {
        // EMPTY COMMAND LINE!

        // END ALPHA MODE
        halSwapCmdLineMode(0);
        keyb_set_shift_plane(KSHIFT_NONE);

        // AND COMMAND LINE
        uiCloseCmdLine();
        halSetCmdLineHeight(0);
        halExitContext(CONTEXT_EDITOR);

        return 1;
    }

    return 0;
}

void endCmdLine()
{
    // END ALPHA MODE
    halSwapCmdLineMode(0);
    keyb_set_shift_plane(KSHIFT_NONE);

    // CLOSE COMMAND LINE DISCARDING CONTENTS
    uiCloseCmdLine();
    halSetCmdLineHeight(0);
    halExitContext(CONTEXT_EDITOR);
}



// ============================================================================
//
//    Default key handlers
//
// ============================================================================

void KH(number)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Number keys
// ----------------------------------------------------------------------------
{
    record(keys, "number key handler %08X key %d msg %x shift %x ",
           keymsg, KM_KEY(keymsg), KM_MESSAGE(keymsg), KM_SHIFT(keymsg));

    halKeyHelp(keymsg, "Numbers");
    halKeyOpensEditor(keymsg);

    char number[2] = { 0 };
    switch (KM_KEY(keymsg))
    {
    case KB_1: number[0] = '1'; break;
    case KB_2: number[0] = '2'; break;
    case KB_3: number[0] = '3'; break;
    case KB_4: number[0] = '4'; break;
    case KB_5: number[0] = '5'; break;
    case KB_6: number[0] = '6'; break;
    case KB_7: number[0] = '7'; break;
    case KB_8: number[0] = '8'; break;
    case KB_9: number[0] = '9'; break;
    case KB_0: number[0] = '0'; break;
    }
    uiInsertCharactersN(number, number + 1);
    uiAutocompleteUpdate();
}

void uiCmdRun(WORD Opcode)
{
    word_p obj = rplAllocTempObLowMem(2);
    if(obj) {

        // ENABLE UNDO
        // PRESERVE obj IN CASE OF GC
        ScratchPointer1 = obj;

        rplRemoveSnapshot(halScreen.StkUndolevels + 1);
        rplRemoveSnapshot(halScreen.StkUndolevels);
        if(halScreen.StkCurrentLevel != 1)
            rplTakeSnapshot();
        halScreen.StkCurrentLevel = 0;
        obj = ScratchPointer1;

        obj[0] = Opcode;
        obj[1] = CMD_ENDOFCODE;
        obj[2] = CMD_QSEMI;     // THIS IS FOR SAFETY REASONS
        rplSetEntryPoint(obj);
        int32_t iseval = (Opcode == (CMD_OVR_XEQ)) || (Opcode == (CMD_OVR_EVAL))
                || (Opcode == (CMD_OVR_EVAL1));

        if(iseval) {
            // STORE THE OBJECT/OPCODE THAT MAY CAUSE AN EXCEPTION
            if(rplDepthData() > 0)
                BlameCmd = rplPeekData(1);
            else
                BlameCmd = 0;
        }
        else
            BlameCmd = 0;
        int32_t rstksave = RSTop - RStk, lamsave = LAMTop - LAMs, nlambase =
                nLAMBase - LAMs;
        int32_t result = rplRun();
        switch (result) {

        case CLEAN_RUN:
        {
            // SOMEBODY CALLED EXITRPL EXPLICITLY
            // EVERYTHING WAS COMPLETELY CLEANED UP AND RESET
            halFlags &= ~(HAL_HALTED | HAL_AUTORESUME | HAL_FASTAUTORESUME);
            break;
        }
        case NEEDS_CLEANUP:
        {
            // UNTRAPPED ERROR
            // CLEANUP ANY GARBAGE AFTER OUR SAVED POINTER
            if(RSTop >= RStk + rstksave) {
                RSTop = RStk + rstksave;
                // BLAME THE ERROR ON THE COMMAND WE CALLED
                if(!rplIsTempObPointer(ExceptionPointer)) {
                    if(BlameCmd != 0)
                        rplBlameError(BlameCmd);
                }
            }
            else {
                rplCleanup();
                halFlags &= ~(HAL_HALTED | HAL_AUTORESUME | HAL_FASTAUTORESUME);
            }
            if(LAMTop > LAMs + lamsave)
                LAMTop = LAMs + lamsave;
            if(nLAMBase > LAMs + nlambase)
                nLAMBase = LAMs + nlambase;
            break;
        }

        case CODE_HALTED:
        {
            // UNTRAPPED ERROR
            // CLEANUP ANY GARBAGE AFTER OUR SAVED POINTER
            if(RSTop > RStk + rstksave) {
                // THE CODE HALTED SOMEWHERE INSIDE!
                halFlags |= HAL_HALTED;
                if(Exceptions & EX_POWEROFF)
                    halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                if(Exceptions & EX_HALRESET)
                    halFlags |= HAL_RESET;
                if(Exceptions & EX_HWRESET)
                    halFlags |= HAL_HWRESET;

                if(Exceptions & EX_AUTORESUME) {
                    halFlags |= HAL_AUTORESUME;
                    Exceptions = 0;
                }
            }
            else {
                if(RSTop < RStk + rstksave) {
                    // THIS CAN ONLY HAPPEN IF SOMEHOW
                    // THE CODE ESCAPED FROM THE SECONDARY
                    // WE CREATED, THIS CAN HAPPEN WHEN USING 'CONT'
                    // INSIDE A SECONDARY AND IT'S NOT NECESSARILY BAD
                    if(CurOpcode == CMD_ENDOFCODE) {
                        rplClearErrors();
                        rplCleanup();
                    }
                    if(HaltedIPtr) {
                        halFlags |= HAL_HALTED;
                        if(Exceptions & EX_POWEROFF)
                            halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                        if(Exceptions & EX_HALRESET)
                            halFlags |= HAL_RESET;
                        if(Exceptions & EX_HWRESET)
                            halFlags |= HAL_HWRESET;

                        if(Exceptions & EX_AUTORESUME) {
                            halFlags |= HAL_AUTORESUME;
                            Exceptions = 0;
                        }
                    }
                    else {
                        halFlags &=
                                ~(HAL_HALTED | HAL_AUTORESUME |
                                HAL_FASTAUTORESUME);
                        if(Exceptions & EX_POWEROFF)
                            halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                        if(Exceptions & EX_HALRESET)
                            halFlags |= HAL_RESET;
                        if(Exceptions & EX_HWRESET)
                            halFlags |= HAL_HWRESET;
                    }

                }
                else {
                    // RETURN STACK WAS INTACT, IT HALTED AT OUR OWN SECONDARY
                    //if(LAMTop>LAMs+lamsave) LAMTop=LAMs+lamsave;
                    //if(nLAMBase>LAMs+nlambase) nLAMBase=LAMs+nlambase;

                    // DON'T ALTER THE INSTRUCTION POINTER OF THE HALTED PROGRAM
                    if(HaltedIPtr) {
                        halFlags |= HAL_HALTED;
                        if(Exceptions & EX_POWEROFF)
                            halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                        if(Exceptions & EX_HALRESET)
                            halFlags |= HAL_RESET;
                        if(Exceptions & EX_HWRESET)
                            halFlags |= HAL_HWRESET;

                        if(Exceptions & EX_AUTORESUME) {
                            halFlags |= HAL_AUTORESUME;
                            Exceptions = 0;
                        }
                    }
                    else {
                        halFlags &=
                                ~(HAL_HALTED | HAL_AUTORESUME |
                                HAL_FASTAUTORESUME);
                        if(Exceptions & EX_POWEROFF)
                            halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                        if(Exceptions & EX_HALRESET)
                            halFlags |= HAL_RESET;
                        if(Exceptions & EX_HWRESET)
                            halFlags |= HAL_HWRESET;
                    }
                    rplClearErrors();

                }
            }

            break;
        }
        }

    }

}

void uiCmdRunHide(WORD Opcode, int32_t narguments)
{
    word_p obj = rplAllocTempObLowMem(2);
    if(obj) {

        ScratchPointer1 = obj;
        // ENABLE UNDO
        rplRemoveSnapshot(halScreen.StkUndolevels + 1);
        rplRemoveSnapshot(halScreen.StkUndolevels);
        if(halScreen.StkCurrentLevel != 1)
            rplTakeSnapshotHide(narguments);
        halScreen.StkCurrentLevel = 0;
        obj = ScratchPointer1;

        obj[0] = Opcode;
        obj[1] = CMD_ENDOFCODE;
        obj[2] = CMD_QSEMI;     // THIS IS FOR SAFETY REASONS
        rplSetEntryPoint(obj);
        int32_t iseval = (Opcode == (CMD_OVR_XEQ)) || (Opcode == (CMD_OVR_EVAL))
                || (Opcode == (CMD_OVR_EVAL1));

        if(iseval) {
            // STORE THE OBJECT/OPCODE THAT MAY CAUSE AN EXCEPTION
            if(rplDepthData() > 0)
                BlameCmd = rplPeekData(1);
            else
                BlameCmd = 0;
        }
        else
            BlameCmd = 0;
        int32_t rstksave = RSTop - RStk, lamsave = LAMTop - LAMs, nlambase =
                nLAMBase - LAMs;
        int32_t result = rplRun();
        switch (result) {

        case CLEAN_RUN:
        {
            // SOMEBODY CALLED EXITRPL EXPLICITLY
            // EVERYTHING WAS COMPLETELY CLEANED UP AND RESET
            halFlags &= ~(HAL_HALTED | HAL_AUTORESUME | HAL_FASTAUTORESUME);
            break;
        }
        case NEEDS_CLEANUP:
        {
            // UNTRAPPED ERROR
            // CLEANUP ANY GARBAGE AFTER OUR SAVED POINTER
            if(RSTop >= RStk + rstksave) {
                RSTop = RStk + rstksave;
                // BLAME THE ERROR ON THE COMMAND WE CALLED
                if(BlameCmd != 0)
                    rplBlameError(BlameCmd);
            }
            else {
                rplCleanup();
                halFlags &= ~(HAL_HALTED | HAL_AUTORESUME | HAL_FASTAUTORESUME);
            }
            if(LAMTop > LAMs + lamsave)
                LAMTop = LAMs + lamsave;
            if(nLAMBase > LAMs + nlambase)
                nLAMBase = LAMs + nlambase;
            break;
        }

        case CODE_HALTED:
        {
            // UNTRAPPED ERROR
            // CLEANUP ANY GARBAGE AFTER OUR SAVED POINTER
            if(RSTop > RStk + rstksave) {
                // THE CODE HALTED SOMEWHERE INSIDE!
                halFlags |= HAL_HALTED;
                if(Exceptions & EX_POWEROFF)
                    halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                if(Exceptions & EX_HALRESET)
                    halFlags |= HAL_RESET;
                if(Exceptions & EX_HWRESET)
                    halFlags |= HAL_HWRESET;

                if(Exceptions & EX_AUTORESUME) {
                    halFlags |= HAL_AUTORESUME;
                    Exceptions = 0;
                }

            }
            else {
                if(RSTop < RStk + rstksave) {
                    // THIS CAN ONLY HAPPEN IF SOMEHOW
                    // THE CODE ESCAPED FROM THE SECONDARY
                    // WE CREATED, THIS CAN HAPPEN WHEN USING 'CONT'
                    // INSIDE A SECONDARY AND IT'S NOT NECESSARILY BAD
                    if(CurOpcode == CMD_ENDOFCODE) {
                        rplClearErrors();
                        rplCleanup();
                    }
                    if(HaltedIPtr) {
                        halFlags |= HAL_HALTED;
                        if(Exceptions & EX_POWEROFF)
                            halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                        if(Exceptions & EX_HALRESET)
                            halFlags |= HAL_RESET;
                        if(Exceptions & EX_HWRESET)
                            halFlags |= HAL_HWRESET;

                        if(Exceptions & EX_AUTORESUME) {
                            halFlags |= HAL_AUTORESUME;
                            Exceptions = 0;
                        }
                    }
                    else {
                        halFlags &=
                                ~(HAL_HALTED | HAL_AUTORESUME |
                                HAL_FASTAUTORESUME);
                        if(Exceptions & EX_POWEROFF)
                            halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                        if(Exceptions & EX_HALRESET)
                            halFlags |= HAL_RESET;
                        if(Exceptions & EX_HWRESET)
                            halFlags |= HAL_HWRESET;
                    }

                }
                else {
                    // RETURN STACK WAS INTACT, RESTORE THE REST
                    //if(LAMTop>LAMs+lamsave) LAMTop=LAMs+lamsave;
                    //if(nLAMBase>LAMs+nlambase) nLAMBase=LAMs+nlambase;

                    // DON'T ALTER THE INSTRUCTION POINTER OF THE HALTED PROGRAM
                    if(HaltedIPtr) {
                        halFlags |= HAL_HALTED;
                        if(Exceptions & EX_POWEROFF)
                            halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                        if(Exceptions & EX_HALRESET)
                            halFlags |= HAL_RESET;
                        if(Exceptions & EX_HWRESET)
                            halFlags |= HAL_HWRESET;

                        if(Exceptions & EX_AUTORESUME) {
                            halFlags |= HAL_AUTORESUME;
                            Exceptions = 0;
                        }
                    }
                    else {
                        halFlags &=
                                ~(HAL_HALTED | HAL_AUTORESUME |
                                HAL_FASTAUTORESUME);
                        if(Exceptions & EX_POWEROFF)
                            halFlags |= HAL_POWEROFF | HAL_FASTAUTORESUME;
                        if(Exceptions & EX_HALRESET)
                            halFlags |= HAL_RESET;
                        if(Exceptions & EX_HWRESET)
                            halFlags |= HAL_HWRESET;
                    }
                    rplClearErrors();

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

int32_t uiCmdRunTransparent(WORD Opcode, int32_t nargs, int32_t nresults)
{
    word_p obj = rplAllocTempObLowMem(2);
    if(obj) {
        obj[0] = Opcode;
        obj[1] = CMD_ENDOFCODE;
        obj[2] = CMD_QSEMI;     // THIS IS FOR SAFETY REASONS

        int32_t rsave, lamsave, nlambase, retvalue;
        WORD exceptsave, errcodesave;
        // PRESERVE VARIOUS STACK POINTERS

        exceptsave = Exceptions;
        errcodesave = ErrorCode;

        rplSetExceptionHandler(0);      // SAVE CURRENT EXCEPTION HANDLERS
        rplPushRet(IPtr);       // SAVE THE CURRENT INSTRUCTION POINTER

        ScratchPointer1 = obj;
        rplTakeSnapshotN(nargs);        // RUN THE COMMAND WITH A PROTECTED STACK WITH nargs ARGUMENTS ONLY
        obj = ScratchPointer1;
        rsave = RSTop - RStk;   // PROTECT THE RETURN STACK
        lamsave = LAMTop - LAMs;        // PROTECT LAM ENVIRONMENTS
        nlambase = nLAMBase - LAMs;

        Exceptions = 0; // REMOVE ALL EXCEPTIONS

        rplSetEntryPoint(obj);

        rplRun();

        // DISCARD ANY ERRORS DURING EXECUTION,  IDEALLY IT HIT THE BREAKPOINT
        if(Exceptions != EX_HALT) {
            // THERE WAS SOME OTHER ERROR DURING EXECUTION, DISCARD ALL OUTPUT FROM THE FAILED PROGRAM
            rplClearData();
        }

        Exceptions = 0;

        // MANUAL RESTORE

        if(RSTop >= RStk + rsave)
            RSTop = RStk + rsave;       // IF RSTop<RStk+rsave THE RETURN STACK WAS COMPLETELY CORRUPTED, SHOULD NEVER HAPPEN BUT...
        else
            rplCleanup();
        if(LAMTop >= LAMs + lamsave)
            LAMTop = LAMs + lamsave;    // OTHERWISE THE LAM ENVIRONMENTS WERE DESTROYED, SHOULD NEVER HAPPEN BUT...
        else
            rplCleanup();
        if(nLAMBase >= LAMs + nlambase)
            nLAMBase = LAMs + nlambase; // OTHERWISE THE LAM ENVIRONMENTS WERE DESTROYED, SHOULD NEVER HAPPEN BUT...
        else
            rplCleanup();

        // CLEAN THE STACK
        if(rplDepthData() > nresults) {
            int32_t f;
            int32_t depth = rplDepthData(), offset = depth - nresults;
            for(f = depth; f > depth - nresults; --f) {
                rplOverwriteData(f, rplPeekData(f - offset));
            }
            rplDropData(offset);
        }
        // HERE THE STACK CONTAINS UP TO nresults ONLY

        rplTakeSnapshotAndClear();      // HERE SNAPSHOT1 = RESULTS, SNAPSHOT2 = PREVIOUS STACK
        rplRevertToSnapshot(2); // RECOVER THE PREVIOUS STACK
        rplDropData(nargs);     // REMOVE THE ORIGINAL ARGUMENTS
        nresults = retvalue = rplDepthSnapshot(1);      // GET THE NUMBER OF RESULTS
        while(nresults) {
            rplPushData(rplPeekSnapshot(1, nresults));  // EXTRACT THE RESULTS INTO THE CURRENT STACK
            --nresults;
        }
        rplRemoveSnapshot(1);   // AND CLEANUP

        // RESTORE THE ERROR CODES FIRST, TO CAPTURE ANY ERRORS DURING POPPING THE RETURN STACK
        Exceptions = exceptsave;
        ErrorCode = errcodesave;

        // RESTORE THE IP POINTER
        IPtr = rplPopRet();

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
int32_t uiRestoreUndoLevel(int32_t level)
{
    int32_t nlevels = rplCountSnapshots();

    if(level < 1)
        return halScreen.StkCurrentLevel;
    if(level > nlevels)
        level = nlevels;

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
    halScreen.StkCurrentLevel =
            uiRestoreUndoLevel(halScreen.StkCurrentLevel + 1);
}

void uiStackRedo()
{
    halScreen.StkCurrentLevel =
            uiRestoreUndoLevel(halScreen.StkCurrentLevel - 1);
}

// Typical command key handler.
// executes Opcode in direct mode
// Inserts Progmode as text in the command line when in programming mode
// if IsFunc == 0 --> In alg mode insert the same text as in prog. mode
//    IsFunc == 1 --> In alg mode insert it with parentheses
//    IsFunc == 2 --> In alg mode, run the opcode directly, as in direct mode
//    IsFunc < 0  --> Not allowed in symbolic (alg) mode, do nothing

void cmdKeyHandler(WORD Opcode, utf8_p Progmode, int32_t IsFunc)
{
    if (!halContext(CONTEXT_EDITOR))
    {
        if (halContext(CONTEXT_STACK))
        {
            // Action when in the stack
            uiCmdRun(Opcode);
            halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
        }
    }
    else
    {
        // ACTION INSIDE THE EDITOR
        switch (halScreen.CursorState & 0xff) {

        case 'D':      // DIRECT EXECUTION
        {

            if(endCmdLineAndCompile())
            {
                uiCmdRun(Opcode);
                halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
            }
            break;
        }
        case 'P':      // PROGRAMMING MODE
            // TODO: SEPARATE TOKENS
            uiSeparateToken();
            uiInsertCharacters((utf8_p) Progmode);
            uiSeparateToken();
            uiAutocompleteUpdate();
            break;

        case 'L':
        case 'C':
        case 'A':      // ALPHANUMERIC MODE
            if(IsFunc >= 0) {

                if(IsFunc == 2) {
                    if(endCmdLineAndCompile())
                    {
                        uiCmdRun(Opcode);
                        halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
                    }
                    break;
                }
                if(IsFunc < 2) {
                    uiInsertCharacters((utf8_p) Progmode);
                    if(IsFunc == 1) {
                        uiInsertCharacters("()");
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
// ----------------------------------------------------------------------------
//   Transpose command
// ----------------------------------------------------------------------------
{
    if (halContext(CONTEXT_STACK | CONTEXT_EDITOR))
    {
        uiCmdRun(Opcode);
        halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
    }
}


utf8_p halCommandName(word_p item)
// ----------------------------------------------------------------------------
//   Return the command name associated with a given item
// ----------------------------------------------------------------------------
{
    if (!item)
        return NULL;

    utf8_p       text, end;
    menu_flags_t flags = MENU_NORMAL;
    if (!uiMenuItemName(item, &flags, &text, &end))
        return NULL;

    // Copy in a local buffer, since the original may be subject to GC
    static char name[32] = { 0 };
    int         size     = end - text;
    int         isdir    = (flags & MENU_IS_DIRECTORY) != 0;
    int         isflg    = (flags & MENU_IS_FLAG) != 0;
    utf8_p      menu     = isdir ? " menu" : isflg ? " flag" : "";
    snprintf(name, sizeof(name), "%.*s%s", size, text, menu);
    return name;

#if 0
    word_p helptext = uiGetMenuItemHelp(item);
    if (!helptext)
        return NULL;
#endif
}


utf8_p halMenuCommandName(int menu, int index)
// ----------------------------------------------------------------------------
//   Find the command name associated with a given menu entry
// ----------------------------------------------------------------------------
{
    uint64_t m1code  = rplGetMenuCode(menu);
    word_p   MenuObj = uiGetLibMenu(m1code);
    unsigned nitems  = uiCountMenuItems(m1code, MenuObj);

    // Check if the command still exists (may have been purged)
    if (MENU_PAGE(m1code) >= nitems)
    {
        m1code = SET_MENU_PAGE(m1code, 0);
        rplSetMenuCode(menu, m1code);
    }

    // Get the item's name
    word_p item = uiGetMenuItem(m1code, MenuObj, index + MENU_PAGE(m1code));
    return halCommandName(item);
}


void functionKeyHandler(keyb_msg_t keymsg, int32_t menunum, int32_t varnum)
// ----------------------------------------------------------------------------
//   Handler for function keys (soft menus)
// ----------------------------------------------------------------------------
// REVISIT: This function is 1097 lines long!
{
    // Switch menus
    if (halKeyMenuSwitch)
        menunum = (menunum == 2) ? 1 : 2;

    halKeyHelp(keymsg, halMenuCommandName(menunum, varnum));

    // Default press message
    if (!halContext(CONTEXT_EDITOR))
    {
        if (!(halContext(CONTEXT_INTERACTIVE_STACK |
                         CONTEXT_PICT |
                         CONTEXT_PLOT)))
        {
            // ACTION WHEN IN THE STACK
            int64_t mcode = rplGetMenuCode(menunum);
            word_p menu = uiGetLibMenu(mcode);
            int32_t nitems = uiCountMenuItems(mcode, menu);
            int32_t idx = MENU_PAGE(mcode) + varnum, page = MENU_PAGE(mcode);

            rplSetLastMenu(menunum);

            if((nitems > 6) && (varnum == 5)) {
                // THIS IS THE NXT KEY
                if((KM_SHIFT(keymsg) == KSHIFT_LEFT)
                        || (KM_SHIFT(keymsg) == KHOLD_LEFT))
                    page -= 5;
                else
                    page += 5;
                if(page >= nitems)
                    page = 0;
                if(page <= -5) {
                    page = nitems / 5;
                    page *= 5;
                    if(page == nitems)
                        page -= 5;
                }
                if(page < 0)
                    page = 0;
                rplSetMenuCode(menunum, SET_MENU_PAGE(mcode, page));
                halRefresh(MENU_DIRTY);
                return;
            }
            // THIS IS A REGULAR VAR KEY

            word_p item = uiGetMenuItem(mcode, menu, idx);

            word_p action = uiGetMenuItemAction(item, KM_SHIFT(keymsg));
            WORD Opcode = 0;
            int32_t hideargument = 1;

            if(!action)
                return;

            switch (KM_SHIFT(keymsg)) {
            case KSHIFT_LEFT:
            case KHOLD_LEFT:
            {
                // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

                if(ISIDENT(*action)) {
                    // USER IS TRYING TO 'STO' INTO THE VARIABLE
                    rplPushData(action);        // PUSH THE NAME ON THE STACK
                    Opcode = CMD_STO;
                    break;
                }
                if(ISUNIT(*action)) {
                    // FOR UNITS, TRY TO CONVERT
                    rplPushData(action);        // PUSH THE NAME ON THE STACK
                    Opcode = CMD_CONVERT;
                    break;
                }

                if(ISLIBRARY(*action)) {
                    // SHOW THE LIBRARY MENU
                    int64_t libmcode =
                            (((int64_t) action[2]) << 32) | MK_MENU_CODE(0,
                            DOLIBPTR, 0, 0);
                    word_p numobject = rplNewBINT(libmcode, HEXBINT);

                    if(!numobject || Exceptions)
                        return;

                    rplPushDataNoGrow(numobject);
                    rplSaveMenuHistory(menunum);
                    rplChangeMenu(menunum, rplPopData());
                    halRefresh(MENU_DIRTY);
                    break;
                }
                // ALL OTHER OBJECTS AND COMMANDS, DO XEQ
                rplPushData(action);
                Opcode = (CMD_OVR_XEQ);
                break;

            }
            case KSHIFT_RIGHT:
            case KHOLD_RIGHT:
            {
                // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

                if(ISIDENT(*action)) {
                    // USER IS TRYING TO 'RCL' THE VARIABLE
                    rplPushData(action);        // PUSH THE NAME ON THE STACK
                    Opcode = CMD_RCL;
                    break;
                }
                if(ISUNIT(*action)) {
                    // FOR UNITS, APPLY THE INVERSE
                    rplPushData(action);        // PUSH THE NAME ON THE STACK
                    Opcode = (CMD_OVR_DIV);
                    break;
                }

                if(ISLIBRARY(*action)) {
                    // SHOW THE LIBRARY MENU
                    int64_t libmcode =
                            (((int64_t) action[2]) << 32) | MK_MENU_CODE(0,
                            DOLIBPTR, 0, 0);
                    word_p numobject = rplNewBINT(libmcode, HEXBINT);

                    if(!numobject || Exceptions)
                        return;

                    rplPushDataNoGrow(numobject);
                    rplSaveMenuHistory(menunum);
                    rplChangeMenu(menunum, rplPopData());
                    halRefresh(MENU_DIRTY);
                    break;
                }
                // ALL OTHER OBJECTS AND COMMANDS, DO XEQ
                rplPushData(action);
                Opcode = (CMD_OVR_XEQ);
                break;

            }
            default:
            {
                // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

                if(ISIDENT(*action)) {
                    // JUST EVAL THE VARIABLE
                    rplPushData(action);        // PUSH THE NAME ON THE STACK
                    Opcode = (CMD_OVR_EVAL1);
                    break;
                }
                if(ISUNIT(*action)) {
                    // FOR UNITS, APPLY THE INVERSE
                    rplPushData(action);        // PUSH THE NAME ON THE STACK
                    Opcode = (CMD_OVR_MUL);
                    break;
                }
                if(ISLIBRARY(*action)) {
                    // SHOW THE LIBRARY MENU
                    int64_t libmcode =
                            (((int64_t) action[2]) << 32) | MK_MENU_CODE(0,
                            DOLIBPTR, 0, 0);
                    word_p numobject = rplNewBINT(libmcode, HEXBINT);

                    if(!numobject || Exceptions)
                        return;

                    rplPushDataNoGrow(numobject);
                    rplSaveMenuHistory(menunum);
                    rplChangeMenu(menunum, rplPopData());
                    halRefresh(MENU_DIRTY);
                    break;
                }

                // ALL OTHER OBJECTS AND COMMANDS, DO XEQ
                rplPushData(action);
                Opcode = (CMD_OVR_XEQ);
                break;

            }
            }

            if(Opcode)
                uiCmdRunHide(Opcode, hideargument);
            halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
        }
    }
    else {
        // ACTION INSIDE THE EDITOR
        int64_t mcode = rplGetMenuCode(menunum);
        word_p menu = uiGetLibMenu(mcode);
        int32_t nitems = uiCountMenuItems(mcode, menu);
        int32_t idx = MENU_PAGE(mcode) + varnum, page = MENU_PAGE(mcode);

        rplSetLastMenu(menunum);

        if((nitems > 6) && (varnum == 5)) {
            // THIS IS THE NXT KEY
            if((KM_SHIFT(keymsg) == KSHIFT_LEFT)
                    || (KM_SHIFT(keymsg) == KHOLD_LEFT))
                page -= 5;
            else
                page += 5;
            if(page >= nitems)
                page = 0;
            if(page <= -5) {
                page = nitems / 5;
                page *= 5;
                if(page == nitems)
                    page -= 5;
            }
            if(page < 0)
                page = 0;
            rplSetMenuCode(menunum, SET_MENU_PAGE(mcode, page));
            halRefresh(MENU_DIRTY);
            return;
        }
        // THIS IS A REGULAR VAR KEY

        word_p item = uiGetMenuItem(mcode, menu, idx);

        word_p action = uiGetMenuItemAction(item, KM_SHIFT(keymsg));
        WORD Opcode = 0;
        int32_t hideargument = 1;

        if(!action)
            return;

        switch (KM_SHIFT(keymsg)) {
        case KSHIFT_LEFT:
        case KHOLD_LEFT:
        {
            // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

            if(ISIDENT(*action)) {
                switch (halScreen.CursorState & 0xff) {
                case 'D':
                case 'A':
                    if(endCmdLineAndCompile()) {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu = uiGetLibMenu(mcode);
                        item = uiGetMenuItem(mcode, menu,
                                MENU_PAGE(mcode) + varnum);
                        action = uiGetMenuItemAction(item,
                                KM_SHIFT(keymsg));

                        // USER IS TRYING TO 'STO' INTO THE VARIABLE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode = CMD_STO;
                    }
                    break;
                case 'P':
                    // USER IS TRYING TO 'STO' INTO THE VARIABLE
                    uiSeparateToken();
                    uiInsertCharacters("'");
                    uiInsertCharactersN((utf8_p) (action + 1),
                                        (utf8_p) (action + 1) +
                                            rplGetIdentLength(action));
                    uiInsertCharacters("' STO");
                    uiSeparateToken();
                    uiAutocompleteUpdate();
                    break;
                }
                break;
            }
            if(ISUNIT(*action)) {

                switch (halScreen.CursorState & 0xff) {
                case 'D':
                    if(endCmdLineAndCompile()) {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu = uiGetLibMenu(mcode);
                        item = uiGetMenuItem(mcode, menu,
                                MENU_PAGE(mcode) + varnum);
                        action = uiGetMenuItemAction(item,
                                KM_SHIFT(keymsg));

                        // USER IS TRYING TO CONVERT
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode = CMD_CONVERT;
                    }
                    break;

                case 'A':
                case 'P':
                {
                    utf8_p string, endstring;
                    if (!rplGetDecompiledString(action,
                                                DECOMP_EDIT | DECOMP_NOHINTS,
                                                &string,
                                                &endstring))
                        break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    uiSeparateToken();
                    uiInsertCharactersN(string, endstring);
                    uiSeparateToken();
                    uiInsertCharacters("CONVERT");
                    uiSeparateToken();
                    uiAutocompleteUpdate();

                    break;
                }

                }
                break;
            }

            if(ISLIBRARY(*action)) {

                // SHOW THE LIBRARY MENU
                int64_t libmcode =
                        (((int64_t) action[2]) << 32) | MK_MENU_CODE(0, DOLIBPTR,
                        0, 0);
                word_p numobject = rplNewBINT(libmcode, HEXBINT);

                if(!numobject || Exceptions)
                    return;

                rplPushDataNoGrow(numobject);
                rplSaveMenuHistory(menunum);
                rplChangeMenu(menunum, rplPopData());
                halRefresh(MENU_DIRTY);
                break;
            }

            if(ISPROGRAM(*action)) {
                if(!ISSECO(*action)) {
                    // IT'S A DOCOL PROGRAM, EXECUTE TRANSPARENTLY
                    rplPushData(action);        // PUSH THE NAME ON THE STACK
                    Opcode = CMD_OVR_XEQ;
                    break;
                }
            }

            // ALL OTHER OBJECTS AND COMMANDS, DO XEQ AFTER ENDING THE COMMAND LINE
            if(endCmdLineAndCompile()) {
                // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                menu = uiGetLibMenu(mcode);
                item = uiGetMenuItem(mcode, menu, MENU_PAGE(mcode) + varnum);
                action = uiGetMenuItemAction(item, KM_SHIFT(keymsg));

                // USER IS TRYING TO 'STO' INTO THE VARIABLE
                rplPushData(action);    // PUSH THE NAME ON THE STACK
                Opcode = CMD_OVR_XEQ;
                break;
            }
            break;

        }
        case KSHIFT_RIGHT:
        case KHOLD_RIGHT:
        {
            // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

            if(ISIDENT(*action)) {
                switch (halScreen.CursorState & 0xff) {
                case 'D':

                    if(KM_SHIFT(keymsg) & KHOLD_LEFT) {
                        //  DECOMPILE THE CONTENTS AND INSERT DIRECTLY INTO THE COMMAND LINE

                        word_p *var = rplFindGlobal(action, 1);
                        utf8_p string = 0, endstring;

                        if(var) {

                            if(ISDIR(*var[1])) {
                                // VARIABLE IS A DIRECTORY, DON'T RCL
                                // BUT PUT THE NAME
                                string = (utf8_p) (action + 1);
                                endstring = string + rplGetIdentLength(action);
                            }
                            else {
                                rplGetDecompiledString(var[1], DECOMP_EDIT, &string, &endstring);
                            }

                            if(string) {

                                uiSeparateToken();
                                int32_t nlines =
                                    uiInsertCharactersN(string,
                                                        endstring);
                                if(nlines)
                                    uiStretchCmdLine(nlines);
                                uiSeparateToken();
                                uiAutocompleteUpdate();
                            }
                        }
                        break;

                    }

                    // NOT HOLD, JUST END THE COMMAND LINE AND RCL THE VARIABLE

                    if(endCmdLineAndCompile()) {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu = uiGetLibMenu(mcode);
                        item = uiGetMenuItem(mcode, menu,
                                MENU_PAGE(mcode) + varnum);
                        action = uiGetMenuItemAction(item,
                                KM_SHIFT(keymsg));

                        // USER IS TRYING TO 'RCL' THE VARIABLE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode = CMD_RCL;
                    }
                    break;
                case 'A':
                    // USER IS TRYING TO 'RCL' THE VARIABLE

                    if(KM_SHIFT(keymsg) & KHOLD_LEFT) {
                        //  DECOMPILE THE CONTENTS AND INSERT DIRECTLY INTO THE COMMAND LINE

                        word_p *var = rplFindGlobal(action, 1);
                        utf8_p string = 0, endstring;

                        if(var) {

                            if(ISDIR(*var[1])) {
                                // VARIABLE IS A DIRECTORY, DON'T RCL
                                // BUT PUT THE NAME
                                string = (utf8_p) (action + 1);
                                endstring = string + rplGetIdentLength(action);
                            }
                            else {
                                rplGetDecompiledStringWithoutTickmarks(
                                    var[1],
                                    DECOMP_EDIT | DECOMP_NOHINTS,
                                    &string,
                                    &endstring);
                            }

                            if(string) {
                                int32_t nlines =
                                    uiInsertCharactersN(string, endstring);
                                if(nlines)
                                    uiStretchCmdLine(nlines);

                                uiAutocompleteUpdate();
                            }
                        }
                        break;

                    }

                    // JUST INSERT THE NAME IN ALGEBRAIC MODE

                    uiInsertCharactersN((utf8_p) (action + 1),
                                        (utf8_p) (action + 1) +
                                        rplGetIdentLength(action));
                    break;

                case 'P':
                    // USER IS TRYING TO 'RCL' THE VARIABLE

                    if(KM_SHIFT(keymsg) & KHOLD_LEFT) {
                        //  DECOMPILE THE CONTENTS AND INSERT DIRECTLY INTO THE COMMAND LINE

                        word_p *var = rplFindGlobal(action, 1);

                        utf8_p string = 0, endstring;

                        if(var) {

                            if(ISDIR(*var[1])) {
                                // VARIABLE IS A DIRECTORY, DON'T RCL
                                // BUT PUT THE NAME
                                string = (utf8_p) (action + 1);
                                endstring = string + rplGetIdentLength(action);
                            }
                            else {
                                rplGetDecompiledString(var[1],
                                                       DECOMP_EDIT,
                                                       &string,
                                                       &endstring);
                            }
                            if(string) {
                                uiSeparateToken();
                                int32_t nlines =
                                        uiInsertCharactersN(string, endstring);
                                if(nlines)
                                    uiStretchCmdLine(nlines);

                                uiSeparateToken();
                                uiAutocompleteUpdate();
                            }
                        }
                        break;

                    }
                    uiSeparateToken();
                    uiInsertCharacters("'");
                    uiInsertCharactersN((utf8_p) (action + 1),
                                        (utf8_p) (action + 1) +
                                            rplGetIdentLength(action));
                    uiInsertCharacters("' RCL");
                    uiSeparateToken();
                    uiAutocompleteUpdate();
                    break;
                }
                break;
            }
            if(ISUNIT(*action)) {

                switch (halScreen.CursorState & 0xff) {
                case 'D':
                    if(endCmdLineAndCompile()) {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu = uiGetLibMenu(mcode);
                        item = uiGetMenuItem(mcode, menu,
                                MENU_PAGE(mcode) + varnum);
                        action = uiGetMenuItemAction(item,
                                KM_SHIFT(keymsg));

                        // USER IS TRYING TO APPLY DIVIDING
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode = (CMD_OVR_DIV);
                    }
                    break;

                case 'A':
                case 'P':
                {
                    utf8_p string, endstring;
                    if (!rplGetDecompiledString(action, DECOMP_EDIT, &string, &endstring))
                        break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    uiSeparateToken();
                    uiInsertCharactersN(string, endstring);
                    uiSeparateToken();
                    uiInsertCharacters("/");
                    uiSeparateToken();
                    uiAutocompleteUpdate();

                    break;
                }

                }
                break;
            }

            if(ISLIBRARY(*action)) {
                switch (halScreen.CursorState & 0xff) {
                case 'D':
                case 'P':
                {
                    // SHOW THE LIBRARY MENU
                    int64_t libmcode =
                            (((int64_t) action[2]) << 32) | MK_MENU_CODE(0,
                            DOLIBPTR, 0, 0);
                    word_p numobject = rplNewBINT(libmcode, HEXBINT);

                    if(!numobject || Exceptions)
                        return;

                    rplPushDataNoGrow(numobject);
                    rplSaveMenuHistory(menunum);
                    rplChangeMenu(menunum, rplPopData());
                    halRefresh(MENU_DIRTY);
                    break;
                }
                case 'A':
                {
                    // INSERT THE LIBRARY IDENTIFIER
                    utf8_p string, endstring;
                    string = (utf8_p) (action + 2);
                    endstring = string + rplGetIdentLength(action + 1);
                    int32_t nlines = uiInsertCharactersN(string, endstring);
                    if(nlines)
                        uiStretchCmdLine(nlines);
                    uiAutocompleteUpdate();

                }
                    break;
                }
                break;
            }

            if(ISPROGRAM(*action)) {
                if(!ISSECO(*action)) {
                    // IT'S A DOCOL PROGRAM, EXECUTE TRANSPARENTLY
                    rplPushData(action);        // PUSH THE NAME ON THE STACK
                    Opcode = CMD_OVR_XEQ;
                    break;
                }
            }

            // ALL OTHER OBJECTS AND COMMANDS, DO XEQ AFTER ENDING THE COMMAND LINE
            if(endCmdLineAndCompile()) {
                // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                menu = uiGetLibMenu(mcode);
                item = uiGetMenuItem(mcode, menu, MENU_PAGE(mcode) + varnum);
                action = uiGetMenuItemAction(item, KM_SHIFT(keymsg));
                rplPushData(action);    // PUSH THE NAME ON THE STACK
                Opcode = CMD_OVR_XEQ;
            }

            break;
        }
        default:
        {
            // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

            if(ISIDENT(*action)) {
                switch (halScreen.CursorState & 0xff) {
                case 'D':
                {
                    // HANDLE DIRECTORIES IN A SPECIAL WAY: DON'T CLOSE THE COMMAND LINE
                    word_p *var = rplFindGlobal(action, 1);
                    if(var) {
                        if(ISDIR(*(var[1]))) {
                            // CHANGE THE DIR WITHOUT CLOSING THE COMMAND LINE
                            rplPushData(action);        // PUSH THE NAME ON THE STACK
                            Opcode = (CMD_OVR_EVAL);
                            break;
                        }
                    }

                    if(endCmdLineAndCompile()) {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu = uiGetLibMenu(mcode);
                        item = uiGetMenuItem(mcode, menu,
                                MENU_PAGE(mcode) + varnum);
                        action = uiGetMenuItemAction(item,
                                KM_SHIFT(keymsg));

                        // USER IS TRYING TO EVAL THE VARIABLE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode = (CMD_OVR_EVAL);
                    }
                    break;
                }
                case 'A':
                {
                    word_p *var = rplFindGlobal(action, 1);
                    if(var) {
                        if(ISDIR(*(var[1]))) {
                            // CHANGE THE DIR WITHOUT CLOSING THE COMMAND LINE
                            rplPushData(action);        // PUSH THE NAME ON THE STACK
                            Opcode = (CMD_OVR_EVAL);
                            break;
                        }
                    }
                    utf8_p string, endstring;
                    if (!rplGetDecompiledStringWithoutTickmarks(action, DECOMP_EDIT, &string, &endstring))
                        break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    uiInsertCharactersN(string, endstring);
                    uiAutocompleteUpdate();

                    break;
                }

                case 'P':
                {
                    word_p *var = rplFindGlobal(action, 1);
                    if(var) {
                        if(ISDIR(*(var[1]))) {
                            // CHANGE THE DIR WITHOUT CLOSING THE COMMAND LINE
                            rplPushData(action);        // PUSH THE NAME ON THE STACK
                            Opcode = (CMD_OVR_EVAL);
                            break;
                        }
                    }

                    utf8_p string, endstring;
                    if (!rplGetDecompiledStringWithoutTickmarks(action, DECOMP_EDIT, &string, &endstring))
                        break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    uiSeparateToken();
                    uiInsertCharactersN(string, endstring);
                    uiSeparateToken();
                    uiAutocompleteUpdate();

                    break;
                }
                }
                break;

            }
            if(ISUNIT(*action)) {
                switch (halScreen.CursorState & 0xff) {
                case 'D':
                    if(endCmdLineAndCompile()) {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu = uiGetLibMenu(mcode);
                        item = uiGetMenuItem(mcode, menu,
                                MENU_PAGE(mcode) + varnum);
                        action = uiGetMenuItemAction(item,
                                KM_SHIFT(keymsg));

                        // USER IS TRYING TO APPLY THE UNIT
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode = (CMD_OVR_MUL);
                    }
                    break;

                case 'A':
                {

                    // DECOMPILE THE OBJECT AND INCLUDE IN COMMAND LINE
                    int32_t SavedException = Exceptions;
                    int32_t SavedErrorCode = ErrorCode;
                    int32_t removevalue;

                    if(ISNUMBER(action[1])) {
                        REAL r;
                        rplReadNumberAsReal(action + 1, &r);
                        rplOneToRReg(0);
                        removevalue = eqReal(&r, &RReg[0]);
                    }
                    else
                        removevalue = 0;

                    Exceptions = 0;     // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
                    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
                    word_p opname = rplDecompile(action, DECOMP_EDIT);
                    Exceptions = SavedException;
                    ErrorCode = SavedErrorCode;

                    if(!opname)
                        break;  // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    utf8_p string, endstring;
                    int32_t totaln =
                        rplGetStringPointers(opname, &string, &endstring);

                    if (removevalue)
                    {
                        // SKIP THE NUMERIC PORTION, LEAVE JUST THE UNIT
                        int32_t k, offset = 0;
                        for (k = 0; k < totaln; ++k)
                        {
                            utf8_p next = utf8skip(string + offset, endstring);
                            offset = next - string;
                            if (utf82cp(next, endstring) == '_')
                            {
                                totaln -= k + 1;
                                string = next + 1;
                                break;
                            }
                        }
                    }

                    uiInsertCharactersN(string, endstring);
                    uiAutocompleteUpdate();

                    break;
                }

                case 'P':
                {
                    utf8_p string, endstring;
                    if (!rplGetDecompiledString(action, DECOMP_EDIT, &string, &endstring))
                        break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    uiSeparateToken();
                    uiInsertCharactersN(string, endstring);
                    uiSeparateToken();
                    uiInsertCharacters("*");
                    uiSeparateToken();
                    uiAutocompleteUpdate();

                    break;
                }

                }
                break;
            }
            if(!IS_PROLOG(*action)) {
                // THIS IS A COMMAND, DECOMPILE AND INSERT NAME
                switch (halScreen.CursorState & 0xff) {
                case 'D':
                    if(endCmdLineAndCompile()) {
                        // FIND THE COMMAND AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu = uiGetLibMenu(mcode);
                        item = uiGetMenuItem(mcode, menu,
                                MENU_PAGE(mcode) + varnum);
                        action = uiGetMenuItemAction(item,
                                KM_SHIFT(keymsg));

                        Opcode = *action;
                        hideargument = 0;
                    }
                    break;

                case 'A':
                {

                    WORD tokeninfo = 0;
                    LIBHANDLER han = rplGetLibHandler(LIBNUM(*action));

                    // GET THE SYMBOLIC TOKEN INFORMATION
                    if(han) {
                        WORD savecurOpcode = CurOpcode;
                        DecompileObject = action;
                        CurOpcode = MK_OPCODE(LIBNUM(*action), OPCODE_GETINFO);
                        (*han) ();

                        if(RetNum > OK_TOKENINFO) {
                            tokeninfo = RetNum;
                        }

                        CurOpcode = savecurOpcode;
                    }

                    utf8_p string, endstring;
                    if (!rplGetDecompiledString(action, DECOMP_EDIT | DECOMP_NOHINTS, &string, &endstring))
                        break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    int32_t nlines = uiInsertCharactersN(string, endstring);
                    if(nlines)
                        uiStretchCmdLine(nlines);

                    if(TI_TYPE(tokeninfo) == TITYPE_FUNCTION) {
                        uiInsertCharacters("()");
                        uiCursorLeft(1);
                    }
                    uiAutocompleteUpdate();

                    break;
                }

                case 'P':
                {

                    int32_t dhints = 0;

                    if(!rplTestSystemFlag(FL_AUTOINDENT)) {

                        LIBHANDLER han = rplGetLibHandler(LIBNUM(*action));

                        // GET THE SYMBOLIC TOKEN INFORMATION
                        if(han) {
                            WORD savecurOpcode = CurOpcode;
                            DecompileObject = action;
                            CurOpcode =
                                    MK_OPCODE(LIBNUM(*action), OPCODE_GETINFO);
                            (*han) ();

                            if(RetNum > OK_TOKENINFO) {
                                dhints = DecompHints;
                            }

                            CurOpcode = savecurOpcode;
                        }

                    }

                    utf8_p string, endstring;
                    if (!rplGetDecompiledString(action, DECOMP_EDIT | DECOMP_NOHINTS, &string, &endstring))
                        break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                    int32_t nlines = 0;

                    if (dhints & HINT_ALLBEFORE)
                    {
                        if (dhints & HINT_ADDINDENTBEFORE)
                            halScreen.CmdLineIndent += 2;
                        if (dhints & HINT_SUBINDENTBEFORE)
                            halScreen.CmdLineIndent -= 2;

                        if (dhints & HINT_NLBEFORE)
                        {
                            int32_t isempty;
                            int32_t nlvl = uiGetIndentLevel(&isempty);
                            // IF THE LINE IS EMPTY DON'T ADD A NEW LINE
                            if (isempty)
                            {
                                if (dhints & HINT_ADDINDENTBEFORE)
                                    uiInsertCharacters("  ");
                                if (dhints & HINT_SUBINDENTBEFORE)
                                {
                                    if (nlvl > 2)
                                        nlvl = 2;
                                    uiCursorLeft(nlvl);
                                    uiRemoveCharacters(nlvl);
                                }
                            }
                            else
                            {
                                uiInsertCharacters("\n");

                                ++nlines;
                                int k;
                                for (k = 0; k < nlvl + halScreen.CmdLineIndent;
                                     ++k)
                                    uiInsertCharacters(
                                        " "); // APPLY INDENT
                                halScreen.CmdLineIndent = 0;
                            }
                        }
                    }

                    uiSeparateToken();
                    nlines += uiInsertCharactersN(string, endstring);

                    if (dhints & HINT_ALLAFTER)
                    {
                        if (dhints & HINT_ADDINDENTAFTER)
                            halScreen.CmdLineIndent += 2;
                        if (dhints & HINT_SUBINDENTAFTER)
                            halScreen.CmdLineIndent -= 2;
                        if (dhints & HINT_NLAFTER)
                        {
                            int32_t nlvl = uiGetIndentLevel(0);

                            uiInsertCharacters("\n");

                            ++nlines;
                            int k;
                            for (k = 0; k < nlvl + halScreen.CmdLineIndent; ++k)
                                uiInsertCharacters(
                                    " "); // APPLY INDENT
                            halScreen.CmdLineIndent = 0;
                        }
                    }

                    if (nlines)
                        uiStretchCmdLine(nlines);

                    uiSeparateToken();
                    uiAutocompleteUpdate();

                    break;
                }
                }
                break;
            }

            if (ISLIBRARY(*action))
            {
                // SHOW THE LIBRARY MENU
                int64_t libmcode = (((int64_t) action[2]) << 32) |
                                   MK_MENU_CODE(0, DOLIBPTR, 0, 0);
                word_p numobject = rplNewBINT(libmcode, HEXBINT);

                if (!numobject || Exceptions)
                    return;

                rplPushDataNoGrow(numobject);
                rplSaveMenuHistory(menunum);
                rplChangeMenu(menunum, rplPopData());
                halRefresh(MENU_DIRTY);
                break;
            }

            if (ISPROGRAM(*action))
            {
                if (!ISSECO(*action))
                {
                    // IT'S A DOCOL PROGRAM, EXECUTE TRANSPARENTLY
                    rplPushData(action); // PUSH THE NAME ON THE STACK
                    Opcode = CMD_OVR_XEQ;
                }
                else
                {
                    if (endCmdLineAndCompile())
                    {
                        // FIND THE VARIABLE AGAIN, IT MIGHT'VE MOVED DUE TO GC
                        menu   = uiGetLibMenu(mcode);
                        item   = uiGetMenuItem(mcode,
                                             menu,
                                             MENU_PAGE(mcode) + varnum);
                        action = uiGetMenuItemAction(item, KM_SHIFT(keymsg));
                        rplPushData(action); // PUSH THE NAME ON THE STACK
                        Opcode = CMD_OVR_XEQ;
                    }
                }
                break;
            }

            // ALL OTHER OBJECTS AND COMMANDS
            switch (halScreen.CursorState & 0xff)
            {
            case 'D':
                if (endCmdLineAndCompile())
                {
                    // FIND THE COMMAND AGAIN, IT MIGHT'VE MOVED DUE TO GC
                    menu = uiGetLibMenu(mcode);
                    item = uiGetMenuItem(mcode, menu, MENU_PAGE(mcode) + varnum);
                    action = uiGetMenuItemAction(item, KM_SHIFT(keymsg));
                    if (!IS_PROLOG(*action))
                    {
                        Opcode       = *action; // RUN COMMANDS DIRECTLY
                        hideargument = 0;
                    }
                    else
                    {
                        Opcode = (CMD_OVR_XEQ);
                        rplPushData(action);
                    }
                }
                break;

            case 'A':
            {
                WORD       tokeninfo = 0;
                LIBHANDLER han       = rplGetLibHandler(LIBNUM(*action));

                // GET THE SYMBOLIC TOKEN INFORMATION
                if (han)
                {
                    WORD savecurOpcode = CurOpcode;
                    DecompileObject    = action;
                    CurOpcode = MK_OPCODE(LIBNUM(*action), OPCODE_GETINFO);
                    (*han)();

                    if (RetNum > OK_TOKENINFO)
                        tokeninfo = RetNum;

                    CurOpcode = savecurOpcode;
                }

                utf8_p string, endstring;
                if (!rplGetDecompiledString(action,
                                            DECOMP_EDIT | DECOMP_NOHINTS,
                                            &string,
                                            &endstring))
                    break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                uiInsertCharactersN(string, endstring);
                if (TI_TYPE(tokeninfo) == TITYPE_FUNCTION)
                {
                    uiInsertCharacters("()");
                    uiCursorLeft(1);
                }
                uiAutocompleteUpdate();

                break;
            }

            case 'P':
            {
                utf8_p string, endstring;
                if (!rplGetDecompiledString(action,
                                            DECOMP_EDIT,
                                            &string,
                                            &endstring))
                    break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                uiSeparateToken();
                int32_t nlines = uiInsertCharactersN(string, endstring);
                if (nlines)
                    uiStretchCmdLine(nlines);

                uiSeparateToken();
                uiAutocompleteUpdate();

                break;
            }
            }

            break;
        }
        }

        if (Opcode)
            uiCmdRunHide(Opcode, hideargument);
        halRefresh(MENU_DIRTY);
    }
}


void symbolKeyHandler(keyb_msg_t keymsg, utf8_p symbol, int32_t separate)
// ----------------------------------------------------------------------------
//   Insert a symbol
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, symbol);
    halKeyOpensEditor(keymsg);

    if(separate && ((halScreen.CursorState & 0xff) == 'P'))
        uiSeparateToken();

    uiInsertCharacters(symbol);

    if(separate && ((halScreen.CursorState & 0xff) == 'P'))
        uiSeparateToken();
    uiAutocompleteUpdate();

}

void alphasymbolKeyHandler(keyb_msg_t keymsg, utf8_p Lsymbol, utf8_p Csymbol)
// ----------------------------------------------------------------------------
//   Shared handler for alpha keys
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, Csymbol);
    halKeyOpensEditor(keymsg);

    if (keymsg & KSHIFT_ALPHA)
    {
        if(keymsg & KFLAG_ALPHA_LOWER)
            uiInsertCharacters(Lsymbol);
        else
            uiInsertCharacters(Csymbol);
    }
    uiAutocompleteUpdate();

}

void KH(ToggleMenu)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Toggle second menu
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, halScreen.Menu2 ? "Hide menu" : "Show menu");

    // Simply toggle the menu upon press
    if(halScreen.Menu2)
    {
        halSetMenu2Height(0);
        rplSetSystemFlag(FL_HIDEMENU2);
    }
    else
    {
        halSetMenu2Height(MENU2_HEIGHT);
        rplClrSystemFlag(FL_HIDEMENU2);
    }
}


void KH(newline)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Insert newline character
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "New line");
    halKeyOpensEditor(keymsg);

    // INCREASE THE HEIGHT ON-SCREEN UP TO THE MAXIMUM
    uiStretchCmdLine(1);
    int32_t ilvl = uiGetIndentLevel(0);

    // ADD A NEW LINE
    uiInsertCharacters("\n");
    int k;
    for(k = 0; k < ilvl + halScreen.CmdLineIndent; ++k)
        uiInsertCharacters(" ");
    halScreen.CmdLineIndent = 0;

    uiAutocompleteUpdate();

}

void KH(decimalDot)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Insert a decimal separator
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Decimal separator");

    uint64_t Locale = rplGetSystemLocale();

    WORD ucode = cp2utf8(DECIMAL_DOT(Locale));
    utf8_p text = (utf8_p) &ucode;
    if(ucode & 0xff000000)
        uiInsertCharactersN(text, text+4);
    else
        uiInsertCharacters(text);

    uiAutocompleteUpdate();

}

void KH(enter)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Enter key
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "ENTER");

    if(!halContext(CONTEXT_EDITOR)) {
        // PERFORM DUP
        if(halContext(CONTEXT_STACK)) {
            // PERFORM DUP ONLY IF THERE'S DATA ON THE STACK
            // DON'T ERROR IF STACK IS EMPTY
            if(rplDepthData() > 0)
                uiCmdRun(CMD_DUP);
            halRefresh(STACK_DIRTY);
        }

        if(halContext(CONTEXT_INTERACTIVE_STACK)) {
            // COPY THE ELEMENT TO LEVEL ONE (PICK)
            if((halScreen.StkPointer > 0)
                    && (halScreen.StkPointer <= rplDepthData())) {
                rplPushData(rplPeekData(halScreen.StkPointer));
                ++halScreen.StkPointer;
                halScreen.StkVisibleLvl = -1;
                halRefresh(STACK_DIRTY);
            }
        }

    }
    else {
        // ENABLE UNDO
        if(halScreen.StkCurrentLevel != 1)
            rplTakeSnapshot();
        halScreen.StkCurrentLevel = 0;

        if(endCmdLineAndCompile()) {
            halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
            if(!(halFlags & (HAL_HWRESET | HAL_RESET))) {
                rplRemoveSnapshot(halScreen.StkUndolevels + 2);
                rplRemoveSnapshot(halScreen.StkUndolevels + 1);
            }

        }
        else {
            // SOMETHING WENT WRONG DURING COMPILE, STACK DIDN'T CHANGE
            rplRemoveSnapshot(1);

        }

    }
}

void KH(cut)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Cut to clipboard
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Cut");

    if(!halContext(CONTEXT_EDITOR)) {
        if(halContext(CONTEXT_STACK)) {
            // ACTION WHEN IN THE STACK
            uiCmdRunTransparent(CMD_CUTCLIP, 1, 1);
            halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
            return;
        }
        if(halContext(CONTEXT_INTERACTIVE_STACK)) {
            int32_t selst, selend;
            switch (halScreen.StkSelStatus) {
            case 0:    // NO ITEMS SELECTED
                if((halScreen.StkPointer < 1)
                        || (halScreen.StkPointer > rplDepthData()))
                    return;
                selst = selend = halScreen.StkPointer;
                break;
            case 1:    // 1 ITEM SELECTED, SELECT ALL ITEMS BETWEEN POINTER AND SELSTART
                if(halScreen.StkPointer > halScreen.StkSelStart) {
                    selst = halScreen.StkSelStart;
                    selend = (halScreen.StkPointer <
                            rplDepthData())? halScreen.
                            StkPointer : rplDepthData();
                }
                else {
                    selend = halScreen.StkSelStart;
                    selst = (halScreen.StkPointer <
                            1) ? 1 : halScreen.StkPointer;
                }
                break;
            case 2:    // BOTH START AND END SELECTED
                selst = halScreen.StkSelStart;
                selend = halScreen.StkSelEnd;
                break;
            default:
                return;
            }

            // PUT ALL OBJECTS IN A LIST

            if(selend - selst == 0) {
                // SINGLE OBJECT, JUST PUT IN THE CLIPBOARD
                rplPushData(rplPeekData(selst));
                uiCmdRunTransparent(CMD_CUTCLIP, 1, 1);
                halRefresh(STACK_DIRTY);
                rplRemoveAtData(selst, 1);

                if(rplDepthData() < 1) {
                    // END INTERACTIVE STACK
                    halExitContext(CONTEXT_INTERACTIVE_STACK);
                    halSetContext(CONTEXT_STACK);
                    halScreen.StkVisibleLvl = 1;
                    halScreen.StkVisibleOffset = 0;
                    halScreen.StkSelStart = halScreen.StkSelEnd =
                            halScreen.StkSelStatus = 0;

                }
                else {

                    halScreen.StkVisibleLvl = -1;
                    halScreen.StkSelStatus = 0;
                    if(halScreen.StkPointer > selend)
                        halScreen.StkPointer--;
                    else if(halScreen.StkPointer >= selst)
                        halScreen.StkPointer = (selst > 1) ? (selst - 1) : 1;

                }

                return;

            }

            word_p newlist = rplCreateListN(selend - selst + 1, selst, 0);
            if((!newlist) || Exceptions)
                return;
            rplListAutoExpand(newlist);

            rplPushData(newlist);
            uiCmdRunTransparent(CMD_CUTCLIP, 1, 1);
            rplRemoveAtData(selst, selend - selst + 1);
            halRefresh(STACK_DIRTY);

            if(rplDepthData() < 1) {
                // END INTERACTIVE STACK
                halExitContext(CONTEXT_INTERACTIVE_STACK);
                halSetContext(CONTEXT_STACK);
                halScreen.StkVisibleLvl = 1;
                halScreen.StkVisibleOffset = 0;
                halScreen.StkSelStart = halScreen.StkSelEnd =
                        halScreen.StkSelStatus = 0;

            }
            else {

                // DISABLE SELECTION STATUS
                halScreen.StkSelStatus = 0;
                if(halScreen.StkPointer > selend)
                    halScreen.StkPointer -= selend - selst + 1;
                else if(halScreen.StkPointer >= selst)
                    halScreen.StkPointer = (selst > 1) ? (selst - 1) : 1;

            }
            return;

        }

    }
    else {
        // ACTION INSIDE THE EDITOR
        word_p string = uiExtractSelection();

        if(string) {
            rplPushData(string);
            uiCmdRunTransparent(CMD_CUTCLIP, 1, 0);
            uiDeleteSelection();
            halRefresh(STACK_DIRTY);
        }

    }
}

void KH(copy)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Copy to clipboard
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Copy");

    if(!halContext(CONTEXT_EDITOR)) {
        if(halContext(CONTEXT_STACK))
        {
            // Action when in the stack
            uiCmdRunTransparent(CMD_COPYCLIP, 1, 1);
            halRefresh(STACK_DIRTY | MENU_DIRTY);
        }
        if(halContext(CONTEXT_INTERACTIVE_STACK)) {
            int32_t selst, selend;
            switch (halScreen.StkSelStatus) {
            case 0:    // NO ITEMS SELECTED
                if((halScreen.StkPointer < 1)
                        || (halScreen.StkPointer > rplDepthData()))
                    return;
                selst = selend = halScreen.StkPointer;
                break;
            case 1:    // 1 ITEM SELECTED, SELECT ALL ITEMS BETWEEN POINTER AND SELSTART
                if(halScreen.StkPointer > halScreen.StkSelStart) {
                    selst = halScreen.StkSelStart;
                    selend = (halScreen.StkPointer <
                            rplDepthData())? halScreen.
                            StkPointer : rplDepthData();
                }
                else {
                    selend = halScreen.StkSelStart;
                    selst = (halScreen.StkPointer <
                            1) ? 1 : halScreen.StkPointer;
                }
                break;
            case 2:    // BOTH START AND END SELECTED
                selst = halScreen.StkSelStart;
                selend = halScreen.StkSelEnd;
                break;
            default:
                return;
            }

            // PUT ALL OBJECTS IN A LIST

            if(selend - selst == 0) {
                // SINGLE OBJECT, JUST PUT IN THE CLIPBOARD
                rplPushData(rplPeekData(selst));
                uiCmdRunTransparent(CMD_COPYCLIP, 1, 1);
                rplDropData(1);
                halRefresh(STACK_DIRTY);
                return;

            }

            word_p newlist = rplCreateListN(selend - selst + 1, selst, 0);
            if((!newlist) || Exceptions)
                return;

            // MAKE NEW LIST AUTOEXPAND ON PASTE
            rplListAutoExpand(newlist);

            rplPushData(newlist);
            uiCmdRunTransparent(CMD_COPYCLIP, 1, 1);
            rplDropData(1);
            halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
            return;

        }

    }
    else {
        // ACTION INSIDE THE EDITOR
        word_p string = uiExtractSelection();

        if(string) {
            rplPushData(string);
            uiCmdRunTransparent(CMD_CUTCLIP, 1, 0);
            halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
        }

    }
}

void KH(paste)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Paste from clipboard
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Paste");

    if(!halContext(CONTEXT_EDITOR)) {
        if(halContext(CONTEXT_STACK))
        {
            // Action when in the stack
            uiCmdRun(CMD_PASTECLIP);
            halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
        }
        if(halContext(CONTEXT_INTERACTIVE_STACK)) {
            int32_t depth = rplDepthData();
            int32_t clevel =
                    (halScreen.StkPointer >
                    depth) ? depth : halScreen.StkPointer;

            uiCmdRun(CMD_PASTECLIP);
            halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
            if(Exceptions)
                return;

            // Now move the new object to the current level
            int32_t nitems = rplDepthData() - depth;
            rplExpandStack(nitems);

            if(Exceptions)
                return;

            // MAKE ROOM
            memmovew(DSTop - clevel, DSTop - clevel - nitems,
                    (clevel + nitems) * sizeof(word_p) / sizeof(WORD));
            // MOVE THE OBJECTS
            memmovew(DSTop - clevel - nitems, DSTop,
                    nitems * sizeof(word_p) / sizeof(WORD));

            if(halScreen.StkSelStatus) {
                if(halScreen.StkSelStart > clevel)
                    halScreen.StkSelStart += nitems;
                if(halScreen.StkSelEnd > clevel)
                    halScreen.StkSelEnd += nitems;
            }
            halScreen.StkPointer++;

            halScreen.StkVisibleLvl = -1;

        }

    }
    else {
        // ACTION INSIDE THE EDITOR
        int32_t depth = rplDepthData();
        uiCmdRun(CMD_PASTECLIP);
        int32_t nitems = rplDepthData() - depth;
        while(nitems >= 1) {
            word_p object = rplPeekData(nitems);
            if(!ISSTRING(*object)) {
                object = rplDecompile(object, DECOMP_EDIT);
                if(!object || Exceptions)
                    return;

                if(((halScreen.CursorState & 0xff) == 'P')
                   || ((halScreen.CursorState & 0xff) == 'D'))
                    uiSeparateToken();
            }

            rplRemoveAtData(nitems, 1);
            uiInsertCharactersN((utf8_p) (object + 1),
                    (utf8_p) (object + 1) + rplStrSize(object));
            --nitems;
        }

    }
}

void KH(backsp)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Backspace key, delete to the left of cursor
// ----------------------------------------------------------------------------
{
    halRepeatingKey(keymsg);

    if(!halContext(CONTEXT_EDITOR)) {
        // DO DROP
        if(halContext(CONTEXT_STACK)) {
            // PERFORM DROP ONLY IF THERE'S DATA ON THE STACK
            // DON'T ERROR IF STACK IS EMPTY
            if(rplDepthData() > 0)
                uiCmdRun(CMD_DROP);
            halRefresh(STACK_DIRTY);
        }

        if(halContext(CONTEXT_INTERACTIVE_STACK)) {
            // SELECTION MODE
            switch (halScreen.StkSelStatus) {
            case 0:
                // NOTHING SELECTED YET, DROP CURRENT ELEMENT
                if(halScreen.StkPointer > rplDepthData())
                    break;
                if(rplDepthData() == 1) {
                    // DROP THE OBJECT AND END THE INTERACTIVE STACK
                    rplDropData(1);

                    // END INTERACTIVE STACK
                    halExitContext(CONTEXT_INTERACTIVE_STACK);
                    halSetContext(CONTEXT_STACK);
                    halScreen.StkVisibleLvl = 1;
                    halScreen.StkVisibleOffset = 0;
                    halScreen.StkSelStart = halScreen.StkSelEnd =
                            halScreen.StkSelStatus = 0;
                    halRefresh(STACK_DIRTY);
                    return;
                }
                if(halScreen.StkPointer <= 0)
                    return;
                if(halScreen.StkPointer == 1)
                    rplDropData(1);
                else {
                    rplRemoveAtData(halScreen.StkPointer, 1);
                    if(halScreen.StkPointer > rplDepthData())
                        halScreen.StkPointer = rplDepthData();
                }
                halScreen.StkVisibleLvl = -1;
                halRefresh(STACK_DIRTY);
                break;
            case 1:
                // START WAS SELECTED, DELETE EVERYTHING HIGHLIGHTED

                if(halScreen.StkPointer > halScreen.StkSelStart) {
                    int32_t count =
                            ((halScreen.StkPointer >
                                rplDepthData())? rplDepthData() : halScreen.
                            StkPointer) - halScreen.StkSelStart + 1;
                    if(rplDepthData() <= count) {
                        // COMPLETELY CLEAR THE STACK AND END INTERACTIVE MODE
                        rplClearData();

                        // END INTERACTIVE STACK
                        halExitContext(CONTEXT_INTERACTIVE_STACK);
                        halSetContext(CONTEXT_STACK);
                        halScreen.StkVisibleLvl = 1;
                        halScreen.StkVisibleOffset = 0;
                        halScreen.StkSelStart = halScreen.StkSelEnd =
                                halScreen.StkSelStatus = 0;
                        halRefresh(STACK_DIRTY);
                        return;

                    }
                    rplRemoveAtData(halScreen.StkSelStart, count);
                    halScreen.StkPointer -= count;
                    if(halScreen.StkPointer < 1)
                        halScreen.StkPointer = 1;
                    halScreen.StkSelStatus = 0;
                    halScreen.StkVisibleLvl = -1;

                }
                else {
                    if(rplDepthData() ==
                            (halScreen.StkPointer ? halScreen.StkPointer : 1) -
                            halScreen.StkSelStart + 1) {
                        // COMPLETELY CLEAR THE STACK AND END INTERACTIVE MODE
                        rplClearData();

                        // END INTERACTIVE STACK
                        halExitContext(CONTEXT_INTERACTIVE_STACK);
                        halSetContext(CONTEXT_STACK);
                        halScreen.StkVisibleLvl    = 1;
                        halScreen.StkVisibleOffset = 0;
                        halScreen.StkSelStart      = 0;
                        halScreen.StkSelEnd        = 0;
                        halScreen.StkSelStatus     = 0;
                        halRefresh(STACK_DIRTY);
                        return;
                    }

                    if(halScreen.StkPointer <= 1) {
                        // JUST DROP ALL ITEMS
                        rplDropData(halScreen.StkSelStart);
                    }
                    else {
                        rplRemoveAtData(halScreen.StkPointer,
                                halScreen.StkSelStart - halScreen.StkPointer +
                                1);
                    }
                    if(halScreen.StkPointer > rplDepthData())
                        halScreen.StkPointer = rplDepthData();
                    halScreen.StkSelStatus = 0;
                    halScreen.StkVisibleLvl = -1;

                }
                halRefresh(STACK_DIRTY);
                break;
            case 2:
            {
                // BOTH START AND END SELECTED, DELETE SELECTED ITEMS
                if(rplDepthData() ==
                        halScreen.StkSelEnd - halScreen.StkSelStart + 1) {
                    // COMPLETELY CLEAR THE STACK AND END INTERACTIVE MODE
                    rplClearData();

                    // END INTERACTIVE STACK
                    halExitContext(CONTEXT_INTERACTIVE_STACK);
                    halSetContext(CONTEXT_STACK);
                    halScreen.StkVisibleLvl    = 1;
                    halScreen.StkVisibleOffset = 0;
                    halScreen.StkSelStart      = 0;
                    halScreen.StkSelEnd        = 0;
                    halScreen.StkSelStatus     = 0;
                    halRefresh(STACK_DIRTY);
                    return;
                }

                int32_t count = halScreen.StkSelEnd - halScreen.StkSelStart + 1;
                rplRemoveAtData(halScreen.StkSelStart, count);
                if(halScreen.StkPointer > halScreen.StkSelEnd)
                    halScreen.StkPointer -= count;
                else if(halScreen.StkPointer >= halScreen.StkSelStart)
                    halScreen.StkPointer = halScreen.StkSelStart;
                if(halScreen.StkPointer > rplDepthData())
                    halScreen.StkPointer = rplDepthData();
                halScreen.StkSelStatus = 0;
                halScreen.StkVisibleLvl = -1;
                halRefresh(STACK_DIRTY);
                break;
            }
            }

        }

    }
    else {
        // REMOVE CHARACTERS FROM THE COMMAND LINE

        uiCursorLeft(1);
        uiRemoveCharacters(1);
        uiAutocompleteUpdate();
    }
}


void KH(del)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Delete to right of cursor
// ----------------------------------------------------------------------------
{
    halRepeatingKey(keymsg);
    if((halContext(CONTEXT_EDITOR))) {
        // REMOVE CHARACTERS FROM THE COMMAND LINE
        uiRemoveCharacters(1);
    }
}



// ============================================================================
//
//    Cursor keys
//
// ============================================================================

void KH(left)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Left cursor
// ----------------------------------------------------------------------------
{
    halRepeatingKey(keymsg);
    if(!halContext(CONTEXT_EDITOR)) {
        if(halContext(CONTEXT_STACK)) {
            // Perform undo in the stack
            uiStackUndo();
            halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
            return;

        }
        if(halContext(CONTEXT_INTERACTIVE_STACK)) {
            switch (halScreen.StkSelStatus) {
            case 0:
                // NO ITEM SELECTED, ROT WITH LEVEL 1
                if(rplDepthData() >= halScreen.StkPointer) {
                    word_p *stptr, *endptr, *cptr;
                    word_p item;
                    stptr = DSTop - halScreen.StkPointer;
                    endptr = DSTop - 1;

                    cptr = stptr;

                    item = *cptr;

                    while(cptr < endptr) {
                        cptr[0] = cptr[1];
                        ++cptr;
                    }
                    *cptr = item;
                }
                break;

            case 1:
            {
                // ONE ITEM SELECTED, ROT THE WHOLE BLOCK BETWEEN POINTER AND SELSTART
                word_p *stptr, *endptr;

                if(halScreen.StkPointer > halScreen.StkSelStart) {
                    endptr = DSTop - halScreen.StkSelStart;
                    stptr = DSTop - ((halScreen.StkPointer >=
                                rplDepthData())? rplDepthData() : halScreen.
                            StkPointer);
                }
                else {
                    stptr = DSTop - halScreen.StkSelStart;
                    endptr = DSTop - halScreen.StkPointer;
                }

                // NOW ROT BETWEEN THEM
                word_p *cptr = stptr;
                word_p item = *stptr;
                while(cptr < endptr) {
                    cptr[0] = cptr[1];
                    ++cptr;
                }
                *cptr = item;
                break;
            }

            case 2:
                // START AND END SELECTED, MOVE THE BLOCK TO THE CURSOR
            {
                if(halScreen.StkPointer > halScreen.StkSelEnd) {
                    word_p *stptr, *endptr, *cptr;
                    word_p item;
                    stptr = DSTop - halScreen.StkSelStart;
                    endptr = DSTop - ((halScreen.StkPointer >
                                rplDepthData())? rplDepthData() : halScreen.
                            StkPointer);

                    // DO UNROT UNTIL THE ENTIRE BLOCK MOVED
                    int32_t count =
                            halScreen.StkSelEnd - halScreen.StkSelStart + 1;

                    while(count--) {
                        cptr = stptr;

                        item = *cptr;

                        while(cptr > endptr) {
                            cptr[0] = cptr[-1];
                            --cptr;
                        }
                        *cptr = item;
                    }

                    count = halScreen.StkSelEnd - halScreen.StkSelStart;
                    halScreen.StkSelEnd =
                            ((halScreen.StkPointer >
                                rplDepthData())? rplDepthData() : halScreen.
                            StkPointer);
                    halScreen.StkSelStart = halScreen.StkSelEnd - count;

                    break;

                }
                if(halScreen.StkPointer < halScreen.StkSelStart) {

                    word_p *stptr, *endptr, *cptr;
                    word_p item;
                    stptr = DSTop - halScreen.StkSelEnd;
                    endptr = DSTop - halScreen.StkPointer - 1;

                    // DO ROT UNTIL THE ENTIRE BLOCK MOVED
                    int32_t count =
                            halScreen.StkSelEnd - halScreen.StkSelStart + 1;
                    while(count--) {
                        cptr = stptr;

                        item = *cptr;

                        while(cptr < endptr) {
                            cptr[0] = cptr[1];
                            ++cptr;
                        }
                        *cptr = item;
                    }

                    count = halScreen.StkSelEnd - halScreen.StkSelStart;
                    halScreen.StkSelStart = halScreen.StkPointer + 1;
                    halScreen.StkSelEnd = halScreen.StkPointer + 1 + count;
                    halScreen.StkPointer += count + 1;
                    halScreen.StkVisibleLvl = -1;

                    break;
                }

                // WHEN THE POINTER IS WITHIN THE BLOCK JUST UNROT THE BLOCK
                word_p *stptr, *endptr, *cptr;
                word_p item;
                stptr = DSTop - halScreen.StkSelStart;
                endptr = DSTop - halScreen.StkSelEnd;

                cptr = stptr;

                item = *cptr;

                while(cptr > endptr) {
                    cptr[0] = cptr[-1];
                    --cptr;
                }
                *cptr = item;

                break;

            }

            }
            halRefresh(STACK_DIRTY);
            return;

        }

    }
    else {
        int32_t line = halScreen.LineCurrent;
        uiCursorLeft(1);
        if(line != halScreen.LineCurrent)
            halScreen.CmdLineIndent = 0;

        halDeferProcess(&uiAutocompleteUpdate);
    }
}

void KH(right)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Right cursor
// ----------------------------------------------------------------------------
{
    halRepeatingKey(keymsg);
    if(!halContext(CONTEXT_EDITOR)) {
        if(halContext(CONTEXT_STACK)) {

            if(rplDepthData() > 1)
            {
                uiCmdRun(CMD_SWAP);
                halRefresh(STACK_DIRTY);
            }
        }
        if(halContext(CONTEXT_INTERACTIVE_STACK)) {
            switch (halScreen.StkSelStatus) {
            case 0:
            {
                if(rplDepthData() >= halScreen.StkPointer) {

                    // NO ITEM SELECTED, UNROT THE WHOLE BLOCK BETWEEN POINTER AND LEVEL 1
                    word_p *stptr, *endptr;

                    stptr = DSTop - 1;
                    endptr = DSTop -
                            (halScreen.StkPointer ? halScreen.StkPointer : 1);

                    // NOW UNROT BETWEEN THEM
                    word_p *cptr = stptr;
                    word_p item = *stptr;
                    while(cptr > endptr) {
                        cptr[0] = cptr[-1];
                        --cptr;
                    }
                    *cptr = item;
                }
                break;
            }
            case 1:
            {
                // ONE ITEM SELECTED, UNROT THE WHOLE BLOCK BETWEEN POINTER AND SELSTART
                word_p *stptr, *endptr;

                if(halScreen.StkPointer > halScreen.StkSelStart) {
                    stptr = DSTop - halScreen.StkSelStart;
                    endptr = DSTop - ((halScreen.StkPointer >
                                rplDepthData())? rplDepthData() : halScreen.
                            StkPointer);
                }
                else {
                    endptr = DSTop - halScreen.StkSelStart;
                    stptr = DSTop -
                            (halScreen.StkPointer ? halScreen.StkPointer : 1);
                }

                // NOW UNROT BETWEEN THEM
                word_p *cptr = stptr;
                word_p item = *stptr;
                while(cptr > endptr) {
                    cptr[0] = cptr[-1];
                    --cptr;
                }
                *cptr = item;
                break;
            }

            case 2:
                // START AND END SELECTED, COPY THE BLOCK TO THE CURSOR
            {
                if(halScreen.StkPointer > halScreen.StkSelEnd) {
                    // MAKE HOLE
                    int32_t count =
                            halScreen.StkSelEnd - halScreen.StkSelStart + 1;
                    int32_t stkptr = halScreen.StkPointer;
                    if(halScreen.StkPointer > rplDepthData())
                        stkptr = rplDepthData();

                    rplExpandStack(count);
                    if(Exceptions)
                        return;

                    memmovew(DSTop - stkptr + count, DSTop - stkptr,
                            stkptr * sizeof(word_p) / sizeof(WORD));

                    // AND COPY THE SELECTION
                    memmovew(DSTop - stkptr,
                            DSTop - halScreen.StkSelEnd + count,
                            count * sizeof(word_p) / sizeof(WORD));

                    DSTop += count;
                    halScreen.StkPointer += count;
                    halScreen.StkVisibleLvl = -1;

                    break;

                }
                if(halScreen.StkPointer < halScreen.StkSelStart) {

                    // MAKE HOLE
                    int32_t count =
                            halScreen.StkSelEnd - halScreen.StkSelStart + 1;

                    rplExpandStack(count);
                    if(Exceptions)
                        return;

                    memmovew(DSTop - halScreen.StkPointer + count,
                            DSTop - halScreen.StkPointer,
                            halScreen.StkPointer * sizeof(word_p) /
                            sizeof(WORD));

                    // AND COPY THE SELECTION
                    memmovew(DSTop - halScreen.StkPointer,
                            DSTop - halScreen.StkSelEnd,
                            count * sizeof(word_p) / sizeof(WORD));

                    DSTop += count;
                    halScreen.StkPointer += count;
                    halScreen.StkSelStart += count;
                    halScreen.StkSelEnd += count;
                    halScreen.StkVisibleLvl = -1;

                    break;
                }

                // WHEN THE POINTER IS WITHIN THE BLOCK JUST ROT THE BLOCK
                word_p *stptr, *endptr, *cptr;
                word_p item;
                endptr = DSTop - halScreen.StkSelStart;
                stptr = DSTop - halScreen.StkSelEnd;

                cptr = stptr;

                item = *cptr;

                while(cptr < endptr) {
                    cptr[0] = cptr[1];
                    ++cptr;
                }
                *cptr = item;

                break;

            }

            }
            halRefresh(STACK_DIRTY);
            return;

        }
    }
    else {
        int32_t line = halScreen.LineCurrent;
        uiCursorRight(1);
        if(line != halScreen.LineCurrent)
            halScreen.CmdLineIndent = 0;

        halDeferProcess(&uiAutocompleteUpdate);
    }
}

void KH(up)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Up cursor
// ----------------------------------------------------------------------------
{
    halRepeatingKey(keymsg);
    if(!halContext(CONTEXT_EDITOR)) {
        if(halContext(CONTEXT_STACK)) {
            // START INTERACTIVE STACK MANIPULATION HERE

            if(rplDepthData() > 0) {

                // ENABLE UNDO
                rplRemoveSnapshot(halScreen.StkUndolevels + 1);
                rplRemoveSnapshot(halScreen.StkUndolevels);
                if(halScreen.StkCurrentLevel != 1)
                    rplTakeSnapshot();
                halScreen.StkCurrentLevel = 0;

                halExitContext(CONTEXT_STACK);
                halSetContext(CONTEXT_INTERACTIVE_STACK);

                halScreen.StkPointer       = 1;
                halScreen.StkSelStart      = -1;
                halScreen.StkSelEnd        = -1;
                halScreen.StkVisibleLvl    = 1;
                halScreen.StkVisibleOffset = 0;
                halScreen.StkSelStatus     = 0;
                halRefresh(STACK_DIRTY);
            }
            return;
        }
        if (halContext(CONTEXT_INTERACTIVE_STACK))
        {
            if (halScreen.StkPointer <= rplDepthData())
            {
                ++halScreen.StkPointer;
                halScreen.StkVisibleLvl = -1;
                halRefresh(STACK_DIRTY);
            }
            return;
        }
        // TODO: ADD OTHER CONTEXTS HERE
    }

    else {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiCursorUp(1);
        halScreen.CmdLineIndent = 0;
        halDeferProcess(&uiAutocompleteUpdate);

    }
}

void KH(down)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//    Down cursor
// ----------------------------------------------------------------------------
{
    halRepeatingKey(keymsg);
    if(!halContext(CONTEXT_EDITOR)) {
        if(halContext(CONTEXT_STACK)) {

            if(rplDepthData() >= 1) {
                word_p prefwidth = rplGetSettings((word_p) editwidth_ident);
                int32_t width;
                if(!prefwidth)
                    width = 0;
                else
                    width = rplReadNumberAsInt64(prefwidth);
                if(Exceptions) {
                    width = 0;
                    Exceptions = 0;
                }
                word_p ptr = rplPeekData(1);
                word_p text =
                        rplDecompile(ptr, DECOMP_EDIT | DECOMP_MAXWIDTH(width));
                if(Exceptions)
                    return;

                BYTE cursorstart = 'D';

                if(ISPROGRAM(*ptr))
                    cursorstart = 'P';
                if(ISSYMBOLIC(*ptr))
                    cursorstart = 'A';
                if(ISUNIT(*ptr))
                    cursorstart = 'A';
                if(ISLIST(*ptr))
                    cursorstart = 'P';

                // Open the command line
                halOpenCmdLine(keymsg, cursorstart);
                int32_t lines = uiSetCmdLineText(text);
                if(lines > 1) {
                    uiStretchCmdLine(lines - 1);
                    halScreen.LineVisible = 1;
                    uiEnsureCursorVisible();
                }

                uiSetCmdLineState(uiGetCmdLineState() | CMDSTATE_OVERWRITE);
                return;
            }

        }

        if(halContext(CONTEXT_INTERACTIVE_STACK)) {
            if(halScreen.StkPointer > 0) {
                --halScreen.StkPointer;
                halScreen.StkVisibleLvl = -1;
                halRefresh(STACK_DIRTY);
            }
            return;
        }

        // TODO: ADD OTHER CONTEXTS HERE
    }

    else {
        // GO DOWN ONE LINE IN MULTILINE TEXT EDITOR
        uiCursorDown(1);
        halScreen.CmdLineIndent = 0;
        halDeferProcess(&uiAutocompleteUpdate);
    }
}


void KH(upOrLeft)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Up or left for DM42, which has no left/right keys
// ----------------------------------------------------------------------------
{
    keyb_msg_t key     = KM_KEY(keymsg);
    unsigned   shifted = (key & KSHIFT_LEFT) != 0;
    unsigned   editing = (halContext(CONTEXT_EDITOR)) != 0;
    if (shifted ^ editing)
        leftKeyHandler(keymsg);
    else
        upKeyHandler(keymsg);
}


void KH(downOrRight)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Down or right for DM42, which has no left/right keys
// ----------------------------------------------------------------------------
{
    keyb_msg_t key     = KM_KEY(keymsg);
    unsigned   shifted = (key & KSHIFT_LEFT) != 0;
    unsigned   editing = (halContext(CONTEXT_EDITOR)) != 0;
    if (shifted ^ editing)
        rightKeyHandler(keymsg);
    else
        upKeyHandler(keymsg);
}


void KH(startOfLine)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Move cursor to start of line
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Start of line");
    if(!halContext(CONTEXT_EDITOR)) {
        if(halContext(CONTEXT_STACK)) {
            // REDO ACTION
            uiStackRedo();
            halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
            return;
        }

    }
    else {
        uiCursorStartOfLine();
        uiAutocompleteUpdate();
    }
}

void KH(endOfLine)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//    Move cursor to end of line
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "End of line");
    if(!halContext(CONTEXT_EDITOR)) {
        if(halContext(CONTEXT_STACK)) {
            // TODO: WHAT TO DO WITH RS-RIGHT CURSOR??
            // THIS SHOULD SCROLL A LARGE OBJECT IN LEVEL 1

        }

    }
    else {
        uiCursorEndOfLine();
        uiAutocompleteUpdate();
    }
}

void KH(startOfText)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Move cursor to start of text
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Start of text");
    if (!halContext(CONTEXT_EDITOR))
    {
        if (halContext(CONTEXT_STACK))
        {
            // TODO: START INTERACTIVE STACK MANIPULATION HERE
        }
        if (halContext(CONTEXT_INTERACTIVE_STACK))
        {
            if (halScreen.StkPointer != rplDepthData())
            {
                halScreen.StkPointer    = rplDepthData();
                halScreen.StkVisibleLvl = -1;
                halRefresh(STACK_DIRTY);
            }
            return;
        }

        // TODO: ADD OTHER CONTEXTS HERE
    }
    else
    {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiCursorStartOfText();
        halScreen.CmdLineIndent = 0;

        uiAutocompleteUpdate();
    }
}

void KH(endOfText)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Move cursor to end of text
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "End of text");
    if (!halContext(CONTEXT_EDITOR))
    {
        if (halContext(CONTEXT_STACK))
        {
            // TODO: ??
        }
        if (halContext(CONTEXT_INTERACTIVE_STACK))
        {
            if (halScreen.StkPointer > 1)
            {
                halScreen.StkPointer       = 1;
                halScreen.StkVisibleLvl    = 1;
                halScreen.StkVisibleOffset = 0;
                halRefresh(STACK_DIRTY);
            }
            return;
        }

        // TODO: ADD OTHER CONTEXTS HERE
    }
    else
    {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiCursorEndOfText();
        halScreen.CmdLineIndent = 0;

        uiAutocompleteUpdate();
    }
}

void KH(pageLeft)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Move one page left
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Page left");
    if (!halContext(CONTEXT_EDITOR))
    {
        if (halContext(CONTEXT_STACK))
        {
            // REVISIT: Knowledge of key mapping within key handler
            // Redo action
            uiStackRedo();
            halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
            return;
        }
    }
    else
    {
        uiCursorPageLeft();
        uiAutocompleteUpdate();
    }
}

void KH(pageRight)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Move one page right
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Page right");
    if (!halContext(CONTEXT_EDITOR))
    {
        if (halContext(CONTEXT_STACK))
        {
            // TODO: WHAT TO DO WITH RS-RIGHT CURSOR??
            // THIS SHOULD SCROLL A LARGE OBJECT IN LEVEL 1
        }
    }
    else
    {
        uiCursorPageRight();
        uiAutocompleteUpdate();
    }
}

void KH(pageUp)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Move one page up
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Page up"); // REVISIT: Is it more useful to repeat?
    if (!halContext(CONTEXT_EDITOR))
    {
        if (halContext(CONTEXT_STACK))
        {
            // TODO: START INTERACTIVE STACK MANIPULATION HERE
        }
        if (halContext(CONTEXT_INTERACTIVE_STACK))
        {
            if (halScreen.StkPointer < rplDepthData())
            {
                halScreen.StkPointer += 5;
                if (halScreen.StkPointer >= rplDepthData())
                    halScreen.StkPointer = rplDepthData();
                halScreen.StkVisibleLvl = -1;
                halRefresh(STACK_DIRTY);
            }
            return;
        }

        // TODO: ADD OTHER CONTEXTS HERE
    }
    else
    {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiCursorPageUp();
        halScreen.CmdLineIndent = 0;
        halDeferProcess(&uiAutocompleteUpdate);
    }
}

void KH(pageDown)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Move one page down
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Page down");
    if (!halContext(CONTEXT_EDITOR))
    {
        if (halContext(CONTEXT_STACK))
        {
            // TODO: ??
        }
        if (halContext(CONTEXT_INTERACTIVE_STACK))
        {
            if (halScreen.StkPointer > 1)
            {
                halScreen.StkPointer = halScreen.StkVisibleLvl - 1;
                if (halScreen.StkPointer < 1)
                    halScreen.StkPointer = 1;
                halScreen.StkVisibleLvl = -1;
                halRefresh(STACK_DIRTY);
            }
            return;
        }
        // TODO: ADD OTHER CONTEXTS HERE
    }
    else
    {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiCursorPageDown();
        halScreen.CmdLineIndent = 0;

        halDeferProcess(&uiAutocompleteUpdate);
    }
}

void KH(startSelection)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Start the selection at cursor
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Start selection");
    if (!halContext(CONTEXT_EDITOR))
    {
        if (halContext(CONTEXT_STACK))
        {
            // TODO: WHAT TO DO WITH RS-LEFT CURSOR??
            // THIS SHOULD SCROLL A LARGE OBJECT IN LEVEL 1
        }
    }
    else
    {
        uiSetSelectionStart();
    }
}

void KH(endSelection)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   End selection at cursor
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "End selection");
    if (!halContext(CONTEXT_EDITOR))
    {
        if (halContext(CONTEXT_STACK))
        {
            // NOT SURE WHAT TO DO WITH THIS KEY
        }
    }
    else
    {
        // IN THE EDITOR, DO SELECTION
        uiSetSelectionEnd();
    }
}

void KH(autoComplete)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//    Auto-complete
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Auto-complete");
    if (!halContext(CONTEXT_EDITOR))
    {
        if (halContext(CONTEXT_STACK))
        {
            // TODO: ??
        }
        // TODO: ADD OTHER CONTEXTS HERE
    }

    else
    {
        // GO UP ONE LINE IN MULTILINE TEXT EDITOR
        uiAutocompInsert();
        uiAutocompleteUpdate();
    }
}

void KH(autoCompleteNext)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Select next auto-completion
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Next completion");
    if (!halContext(CONTEXT_EDITOR))
    {
        if (halContext(CONTEXT_STACK))
        {
            // TODO: ??
        }
        // TODO: ADD OTHER CONTEXTS HERE
    }

    else
    {
        uiAutocompNext();
    }
}

void KH(autoCompletePrevious)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Select previous auto-completion
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Previous completion");
    if (!halContext(CONTEXT_EDITOR))
    {
        if (halContext(CONTEXT_STACK))
        {
            // TODO: ??
        }
        // TODO: ADD OTHER CONTEXTS HERE
    }

    else
    {
        uiAutocompPrev();
    }
}


// ============================================================================
//
//    Operators
//
// ============================================================================

void KH(chs)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Change sign
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "NEG");
    if(!halContext(CONTEXT_EDITOR)) {
        if(halContext(CONTEXT_STACK)) {
            // ACTION WHEN IN THE STACK
            uiCmdRun((CMD_OVR_NEG));
            halRefresh(STACK_DIRTY);
        }
        if(halContext(CONTEXT_INTERACTIVE_STACK)) {
            // SELECTION MODE
            switch (halScreen.StkSelStatus) {
            case 0:
                // NOTHING SELECTED YET
                halScreen.StkSelStart =
                        (halScreen.StkPointer ? halScreen.StkPointer : 1);
                if(halScreen.StkSelStart > rplDepthData())
                    halScreen.StkSelStart = rplDepthData();
                halScreen.StkSelEnd = halScreen.StkSelStart;
                halScreen.StkSelStatus += 2;
                halRefresh(STACK_DIRTY);
                break;
            case 1:
                // START WAS SELECTED
                if(halScreen.StkSelStart > halScreen.StkPointer) {
                    halScreen.StkSelEnd = halScreen.StkSelStart;
                    halScreen.StkSelStart =
                            (halScreen.StkPointer ? halScreen.StkPointer : 1);
                }
                else {
                    halScreen.StkSelEnd =
                            (halScreen.StkPointer ? halScreen.StkPointer : 1);
                    if(halScreen.StkSelEnd > rplDepthData())
                        halScreen.StkSelEnd = rplDepthData();
                }
                ++halScreen.StkSelStatus;
                halRefresh(STACK_DIRTY);
                break;
            case 2:
                // BOTH START AND END SELECTED, REPLACE SELECTION WITH NEW ITEM
                halScreen.StkSelStart =
                        (halScreen.StkPointer ? halScreen.StkPointer : 1);
                if(halScreen.StkSelStart > rplDepthData())
                    halScreen.StkSelStart = rplDepthData();
                halScreen.StkSelEnd = halScreen.StkSelStart;
                halRefresh(STACK_DIRTY);
                break;
            }

        }

    }
    else {
        // ACTION INSIDE THE EDITOR
        utf8_p endnum;
        int32_t flags;

        // FIRST CASE: IF TOKEN UNDER THE CURSOR IS OR CONTAINS A VALID NUMBER, CHANGE THE SIGN OF THE NUMBER IN THE TEXT
        utf8_p startnum = (utf8_p) uiFindNumberStart(&endnum, &flags);
        utf8_p line = (utf8_p) (CmdLineCurrentLine + 1);
        if(!startnum) {
            startnum = line + halScreen.CursorPosition;
            if(startnum > line) {
                if(startnum[-1] == '+') {
                    uiCursorLeft(1);
                    uiRemoveCharacters(1);
                    uiInsertCharacters("-");
                    halRefresh(CMDLINE_LINE_DIRTY | CMDLINE_CURSOR_DIRTY);
                    return;
                }
                if(startnum[-1] == '-') {
                    uiCursorLeft(1);
                    uiRemoveCharacters(1);
                    uiInsertCharacters("+");
                    halRefresh(CMDLINE_LINE_DIRTY | CMDLINE_CURSOR_DIRTY);
                    return;
                }
                if((startnum[-1] == 'E') || (startnum[-1] == 'e')) {
                    if(startnum[0] == '+') {
                        uiRemoveCharacters(1);
                        uiInsertCharacters("-");
                        uiAutocompleteUpdate();
                        return;
                    }
                    else if(startnum[0] == '-') {
                        uiRemoveCharacters(1);
                        uiInsertCharacters("+");
                        uiAutocompleteUpdate();
                        return;
                    }
                    else {
                        uiInsertCharacters("+");
                        uiAutocompleteUpdate();
                        return;
                    }
                }

            }

            // SECOND CASE: IF TOKEN UNDER CURSOR IS EMPTY, IN 'D' MODE COMPILE OBJECT AND THEN EXECUTE NEG

            if((halScreen.CursorState & 0xff) == 'D') {
                // COMPILE AND EXECUTE NEG
                if(endCmdLineAndCompile())
                {
                    uiCmdRun((CMD_OVR_NEG));
                    halRefresh(STACK_DIRTY);
                }
                return;
            }

            if((halScreen.CursorState & 0xff) == 'P') {
                uiSeparateToken();
                uiInsertCharacters("NEG");
                uiSeparateToken();
                uiAutocompleteUpdate();
                return;
            }

            if((halScreen.CursorState & 0xff) == 'A') {
                // IN ALGEBRAIC MODE, TRY TO CHANGE THE SIGN OF THE LAST + OR - SIGN TO THE LEFT OF THE CURSOR

                startnum = line + halScreen.CursorPosition;
                int moveleft = 0;
                utf8_p prevstnum = startnum;
                startnum = utf8rskipst(startnum, line);
                if(startnum != prevstnum)
                    ++moveleft;
                int32_t char1, char2;
                extern const const char forbiddenChars[];
                while(startnum >= line) {
                    utf8_p ptr = (utf8_p) forbiddenChars;
                    char1 = utf82cp((char *)startnum, (char *)prevstnum);
                    do {
                        char2 = utf82cp((char *)ptr, (char *)ptr + 4);
                        if(char1 == char2)
                            break;
                        ptr = (utf8_p) utf8skip((char *)ptr, (char *)ptr + 4);
                    }
                    while(*ptr);
                    if(*ptr)
                        break;
                    if(*startnum == '\'')
                        break;
                    utf8_p newptr = utf8rskipst(startnum, line);
                    if(newptr == startnum)
                        break;  // COULDN'T SKIP ANYMORE
                    ++moveleft;
                    prevstnum = startnum;
                    startnum = newptr;
                }
                if(*startnum == '+') {
                    if(moveleft > 0)
                        uiCursorLeft(moveleft);
                    uiRemoveCharacters(1);
                    uiInsertCharacters("-");
                    if(moveleft > 0)
                        uiCursorRight(moveleft - 1);
                }
                else {
                    if(*startnum == '-') {
                        if(moveleft > 0)
                            uiCursorLeft(moveleft);
                        uiRemoveCharacters(1);
                        uiInsertCharacters("+");
                        if(moveleft > 0)
                            uiCursorRight(moveleft - 1);
                    }
                    else {
                        // FOUND NOTHING!
                        if(moveleft > 0)
                            uiCursorLeft(moveleft - 1);
                        else
                            uiCursorRight(1);
                        startnum = (utf8_p) utf8skipst(startnum, startnum + 4);
                        if(*startnum == '+') {
                            uiRemoveCharacters(1);
                            uiInsertCharacters("-");
                        }
                        else if(*startnum == '-') {
                            uiRemoveCharacters(1);
                            uiInsertCharacters("+");
                        }
                        else
                            uiInsertCharacters("-");

                        if(moveleft > 0)
                            uiCursorRight(moveleft - 1);

                    }
                }

                uiAutocompleteUpdate();
                return;
            }

        }
        else {
            // WE FOUND A NUMBER
            int32_t oldposition = halScreen.CursorPosition;
            // IF THIS IS A NUMBER WITH AN EXPONENT, SEE IF WE NEED TO CHANGE THE SIGN OF THE NUMBER OR THE EXPONENT
            if((flags >> 16) & 4) {
                // LOOK FOR THE 'e' OR 'E'
                int32_t epos = 0;
                while((epos < endnum - startnum) && ((startnum[epos] != 'E')
                            && (startnum[epos] != 'e')))
                    ++epos;

                if(oldposition > (startnum - line) + epos)
                    startnum += epos + 1;       // MOVE START OF NUMBER TO AFTER THE EXPONENT LETTER
                if((startnum[0] == '-') || (startnum[0] == '+'))
                    ++startnum;
            }
            uiMoveCursor(startnum - line);
            utf8_p plusminus = "-";

            if(startnum > line) {
                if(startnum[-1] == '+') {
                    uiMoveCursor(startnum - line - 1);
                    uiRemoveCharacters(1);
                    --oldposition;
                }
                if(startnum[-1] == '-') {
                    uiMoveCursor(startnum - line - 1);
                    uiRemoveCharacters(1);
                    plusminus = "+";
                    --oldposition;
                }
            }

            // NEED TO INSERT A CHARACTER HERE
            uiInsertCharacters(plusminus);
            uiMoveCursor(oldposition + 1);
            uiEnsureCursorVisible();
            uiAutocompleteUpdate();
            return;
        }

        // THIRD CASE: IF TOKEN UNDER CURSOR IS SOMETHING OTHER THAN A NUMBER, JUST INSERT A MINUS SIGN

    }
}

void KH(eex)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Select an exponent
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Exponent");
    if(!halContext(CONTEXT_EDITOR)) {
        if(halContext(CONTEXT_STACK)) {
            halOpenCmdLine(keymsg, 'D');
            NUMFORMAT config;

            rplGetSystemNumberFormat(&config);
            if((config.MiddleFmt | config.BigFmt | config.
                        SmallFmt) & FMT_USECAPITALS)
                uiInsertCharacters("1E");
            else
                uiInsertCharacters("1e");
            uiAutocompleteUpdate();
            return;
        }

    }
    else {
        // ACTION INSIDE THE EDITOR

        // FIRST CASE: IF TOKEN UNDER THE CURSOR IS OR CONTAINS A VALID NUMBER
        utf8_p  startnum, endnum;
        int32_t flags;
        NUMFORMAT config;

        rplGetSystemNumberFormat(&config);

        startnum = uiFindNumberStart(&endnum, &flags);

        utf8_p  line = (utf8_p) (CmdLineCurrentLine + 1);

        if(!startnum) {
            startnum = line + halScreen.CursorPosition;
            // DO NOTHING IF THERE'S ALREADY AN 'E' BEFORE THE CURSOR
            if((startnum > line) && ((startnum[-1] == 'E')
                        || (startnum[-1] == 'e')))
                return;

            // SECOND CASE: IF TOKEN UNDER CURSOR IS EMPTY, IN 'D' MODE COMPILE OBJECT AND THEN APPEND 1E
            if((config.MiddleFmt | config.BigFmt | config.
                        SmallFmt) & FMT_USECAPITALS)
                uiInsertCharacters("1E");
            else
                uiInsertCharacters("1e");
            uiAutocompleteUpdate();
            return;
        }
        else {
            // WE FOUND A NUMBER
            if((startnum > line) && ((startnum[-1] == '-')
                        || (startnum[-1] == '+')))
                --startnum;

            if(halScreen.CursorPosition <= endnum + 1 - line) {
                // THE CURSOR IS WITHIN THE NUMBER
                if((flags >> 16) & 4) {
                    // THE NUMBER ALREADY HAS AN EXPONENT, LOOK FOR THE 'e' OR 'E'
                    int32_t epos = 0;
                    while((epos < endnum - startnum) && ((startnum[epos] != 'E')
                                && (startnum[epos] != 'e')))
                        ++epos;

                    startnum += epos + 1;       // MOVE START OF NUMBER TO AFTER THE EXPONENT LETTER
                    uiMoveCursor(startnum - line);
                    uiRemoveCharacters(endnum - startnum + 1);
                    uiEnsureCursorVisible();
                    uiAutocompleteUpdate();
                    return;
                }

                // NEED TO INSERT A CHARACTER HERE
                int32_t oldposition = halScreen.CursorPosition;
                if((*endnum == 'e') || (*endnum == 'E'))
                    uiMoveCursor(endnum - line + 1);
                else {
                    if((config.MiddleFmt | config.BigFmt | config.
                                SmallFmt) & FMT_USECAPITALS)
                        uiInsertCharacters("E");
                    else
                        uiInsertCharacters("e");
                    uiMoveCursor(oldposition + 1);
                }
                uiEnsureCursorVisible();
                uiAutocompleteUpdate();
                return;

            }
            // THE CURSOR WAS PAST THE END OF THE NUMBER
            if((config.MiddleFmt | config.BigFmt | config.
                        SmallFmt) & FMT_USECAPITALS)
                uiInsertCharacters("1E");
            else
                uiInsertCharacters("1e");
            uiAutocompleteUpdate();
            return;
        }

    }
}

void bracketKeyHandler(keyb_msg_t keymsg, utf8_p string)
// ----------------------------------------------------------------------------
// Common function for all brackets (parenthese, curly braces, etc)
// ----------------------------------------------------------------------------
{
    halKeyOpensEditor(keymsg);
    if(((halScreen.CursorState & 0xff) == 'D')
       || ((halScreen.CursorState & 0xff) == 'P'))
        uiSeparateToken();

    utf8_p end = string + strlen(string);
    uiInsertCharactersN(string, end);
    uiCursorLeft(utf8nlenst(string, end) >> 1);
    uiAutocompleteUpdate();

}

void KH(curlyBracket)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   List separators { } are list delimiters in RPL
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "List delimiters");
    if((halGetCmdLineMode() == 'A') || (halGetCmdLineMode() == 'C')
            || (halGetCmdLineMode() == 'L'))
        bracketKeyHandler(keymsg, "{}");
    else {
        bracketKeyHandler(keymsg, "{  }");
        halSetCmdLineMode('P');
    }

}

void KH(squareBracket)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Square brackets [ ] are vector / matrix delimiters in RPL
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Matrix delimiters");
    if((halGetCmdLineMode() == 'A') || (halGetCmdLineMode() == 'C')
            || (halGetCmdLineMode() == 'L'))
        bracketKeyHandler(keymsg, "[]");
    else
        bracketKeyHandler(keymsg, "[  ]");

}

void KH(secoBracket)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Secondary brackets are program delimiters in RPL
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Program delimiters");
    bracketKeyHandler(keymsg, "«  »");

    if((halGetCmdLineMode() != 'L') && (halGetCmdLineMode() != 'C'))
        halSetCmdLineMode('P');

}

void KH(parenBracket)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Parentheses are used for priorities in arithmetic mode
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Parentheses");
    bracketKeyHandler(keymsg, "()");

}

void KH(textBracket)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Quotes are used as text delimiters in RPL
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Text delimiters");
    bracketKeyHandler(keymsg, "\"\"");

    //  LOCK ALPHA MODE
    if((halGetCmdLineMode() != 'L') && (halGetCmdLineMode() != 'C'))
        keyb_set_shift_plane(KSHIFT_ALPHA);

}

void KH(ticks)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Ticks are used as algebraic expression delimiters in RPL
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Expression delimiters");
    if((halGetCmdLineMode() != 'L') && (halGetCmdLineMode() != 'C')) {
        bracketKeyHandler(keymsg, "''");
        halSetCmdLineMode('A');
    }
    else
        symbolKeyHandler(keymsg, "'", 0);
}

void KH(tag)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Colons are used as tag delimiter in RPL
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Tag delimiter");
    if((halGetCmdLineMode() != 'L') && (halGetCmdLineMode() != 'C')) {
        bracketKeyHandler(keymsg, "::");
        //  Lock alpha mode
        keyb_set_shift_plane(KSHIFT_ALPHA);
    }
    else
        symbolKeyHandler(keymsg, ":", 0);

}


static void contrastPattern()
// ----------------------------------------------------------------------------
//   Draw a contrast pattern for the ON+ and ON- key handlers
// ----------------------------------------------------------------------------
{
    gglsurface scr;
    ggl_init_screen(&scr);
    int ytop =
        halScreen.Form + halScreen.Stack + halScreen.CmdLine +
        halScreen.Menu1;

    // Clear status area
    ggl_rect(&scr, STATUS_AREA_X, ytop, LCD_W - 1,
             ytop + halScreen.Menu2 - 1, PAL_STA_BG);

    for (int j = 0; j < 16; ++j)
    {
        ggl_rect(&scr, STATUS_AREA_X + 1 + 3 * j, ytop + 7,
                 STATUS_AREA_X + 1 + 3 * j + 2, ytop + 12, ggl_color(j));
        ggl_rect(&scr, STATUS_AREA_X + 1 + 3 * j, ytop,
                 STATUS_AREA_X + 1 + 3 * j + 2, ytop + 5, ggl_color(15 - j));
    }
    halRefresh(ALL_DIRTY);
}


void KH(increaseContrast)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Increase contrast on the display
// ----------------------------------------------------------------------------
{
    halRepeatingKey(keymsg);
    contrastPattern();

    // Increase contrast
    lcd_contrast++;
    if(lcd_contrast > 0xf)
        lcd_contrast = 0xf;

    lcd_setcontrast(lcd_contrast);
    WORD savedex = Exceptions;
    Exceptions = 0;
    word_p contrast = rplNewSINT(lcd_contrast, DECBINT);
    if(contrast)
        rplStoreSettings((word_p) screenconfig_ident, contrast);
    Exceptions = savedex;
}


void KH(decreaseContrast)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Decrease contrast
// ----------------------------------------------------------------------------
{
    halRepeatingKey(keymsg);
    contrastPattern();

    // Decrease contrast
    lcd_contrast--;
    if(lcd_contrast < 0)
        lcd_contrast = 0;
    lcd_setcontrast(lcd_contrast);
    WORD savedex = Exceptions;
    Exceptions = 0;
    word_p contrast = rplNewSINT(lcd_contrast, DECBINT);
    if(contrast)
        rplStoreSettings((word_p) screenconfig_ident, contrast);
    Exceptions = savedex;
}


void KH(cycleLocale)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Cycle through major locales
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Cycle locale");
    halRefresh(ALL_DIRTY);

    // CYCLE BETWEEN VARIOUS OPTIONS
    const char *const options[] = {
        "1000.000000", "1,000.000000", "1 000.000000", "1000.000 000",
                "1,000.000 000", "1 000.000 000",
        "1000,000000", "1.000,000000", "1 000,000000", "1000,000 000",
                "1.000,000 000", "1 000,000 000"
    };

    NUMFORMAT fmt;
    int32_t option = 0;
    rplGetSystemNumberFormat(&fmt);
    if(DECIMAL_DOT(fmt.Locale) == ',')
        option += 6;
    if(fmt.MiddleFmt & FMT_NUMSEPARATOR) {
        if(THOUSAND_SEP(fmt.Locale) == ',')
            option += 1;
        if(THOUSAND_SEP(fmt.Locale) == '.')
            option += 1;
        if(THOUSAND_SEP(fmt.Locale) == THIN_SPACE)
            option += 2;
    }
    if(fmt.MiddleFmt & FMT_FRACSEPARATOR)
        option += 3;

    // CYCLE THROUGH ITEMS:
    ++option;
    if(option > 11)
        option = 0;

    gglsurface scr;
    ggl_init_screen(&scr);
    int ytop =
        halScreen.Form + halScreen.Stack + halScreen.CmdLine +
        halScreen.Menu1;
    // CLEAR STATUS AREA
    ggl_rect(&scr, STATUS_AREA_X, ytop, LCD_W - 1,
             ytop + halScreen.Menu2 - 1, PAL_STA_BG);

    DrawTextBk(&scr,STATUS_AREA_X + 1, ytop + 1, "Format:",
               FONT_STATUS, PAL_STA_TEXT, PAL_STA_BG);
    DrawTextBk(&scr, STATUS_AREA_X + 1,
               ytop + 1 + FONT_HEIGHT(FONT_STATUS),
               options[option], FONT_STATUS,PAL_STA_TEXT, PAL_STA_BG);

    // CHANGE THE FORMAT TO THE SELECTED OPTION
    switch (option) {
    default:
    case 0:
        fmt.BigFmt &= ~(FMT_NUMSEPARATOR | FMT_FRACSEPARATOR);
        fmt.SmallFmt &= ~(FMT_NUMSEPARATOR | FMT_FRACSEPARATOR);
        fmt.MiddleFmt &= ~(FMT_NUMSEPARATOR | FMT_FRACSEPARATOR);
        fmt.Locale = MAKELOCALE('.', THIN_SPACE, THIN_SPACE, ',');
        break;
    case 1:
        fmt.BigFmt &= ~(FMT_FRACSEPARATOR);
        fmt.BigFmt |= FMT_NUMSEPARATOR;
        fmt.SmallFmt &= ~(FMT_FRACSEPARATOR);
        fmt.SmallFmt |= FMT_NUMSEPARATOR;
        fmt.MiddleFmt &= ~(FMT_FRACSEPARATOR);
        fmt.MiddleFmt |= FMT_NUMSEPARATOR;
        fmt.Locale = MAKELOCALE('.', ',', THIN_SPACE, ';');
        break;
    case 2:
        fmt.BigFmt &= ~(FMT_FRACSEPARATOR);
        fmt.BigFmt |= FMT_NUMSEPARATOR;
        fmt.SmallFmt &= ~(FMT_FRACSEPARATOR);
        fmt.SmallFmt |= FMT_NUMSEPARATOR;

        fmt.MiddleFmt &= ~(FMT_FRACSEPARATOR);
        fmt.MiddleFmt |= FMT_NUMSEPARATOR;
        fmt.Locale = MAKELOCALE('.', THIN_SPACE, THIN_SPACE, ',');
        break;
    case 3:
        fmt.BigFmt &= ~FMT_NUMSEPARATOR;
        fmt.BigFmt |= FMT_FRACSEPARATOR;
        fmt.SmallFmt &= ~FMT_NUMSEPARATOR;
        fmt.SmallFmt |= FMT_FRACSEPARATOR;
        fmt.MiddleFmt &= ~FMT_NUMSEPARATOR;
        fmt.MiddleFmt |= FMT_FRACSEPARATOR;
        fmt.Locale = MAKELOCALE('.', THIN_SPACE, THIN_SPACE, ',');
        break;
    case 4:
        fmt.BigFmt |= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR;
        fmt.SmallFmt |= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR;
        fmt.MiddleFmt |= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR;
        fmt.Locale = MAKELOCALE('.', ',', THIN_SPACE, ';');
        break;
    case 5:
        fmt.BigFmt |= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR;
        fmt.SmallFmt |= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR;
        fmt.MiddleFmt |= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR;
        fmt.Locale = MAKELOCALE('.', THIN_SPACE, THIN_SPACE, ',');
        break;
    case 6:
        fmt.BigFmt &= ~(FMT_NUMSEPARATOR | FMT_FRACSEPARATOR);
        fmt.SmallFmt &= ~(FMT_NUMSEPARATOR | FMT_FRACSEPARATOR);
        fmt.MiddleFmt &= ~(FMT_NUMSEPARATOR | FMT_FRACSEPARATOR);
        fmt.Locale = MAKELOCALE(',', THIN_SPACE, THIN_SPACE, ';');
        break;
    case 7:
        fmt.BigFmt &= ~(FMT_FRACSEPARATOR);
        fmt.BigFmt |= FMT_NUMSEPARATOR;
        fmt.SmallFmt &= ~(FMT_FRACSEPARATOR);
        fmt.SmallFmt |= FMT_NUMSEPARATOR;
        fmt.MiddleFmt &= ~(FMT_FRACSEPARATOR);
        fmt.MiddleFmt |= FMT_NUMSEPARATOR;
        fmt.Locale = MAKELOCALE(',', '.', THIN_SPACE, ';');
        break;
    case 8:
        fmt.BigFmt &= ~(FMT_FRACSEPARATOR);
        fmt.BigFmt |= FMT_NUMSEPARATOR;
        fmt.SmallFmt &= ~(FMT_FRACSEPARATOR);
        fmt.SmallFmt |= FMT_NUMSEPARATOR;
        fmt.MiddleFmt &= ~(FMT_FRACSEPARATOR);
        fmt.MiddleFmt |= FMT_NUMSEPARATOR;
        fmt.Locale = MAKELOCALE(',', THIN_SPACE, THIN_SPACE, ';');
        break;
    case 9:
        fmt.BigFmt &= ~FMT_NUMSEPARATOR;
        fmt.BigFmt |= FMT_FRACSEPARATOR;
        fmt.SmallFmt &= ~FMT_NUMSEPARATOR;
        fmt.SmallFmt |= FMT_FRACSEPARATOR;
        fmt.MiddleFmt &= ~FMT_NUMSEPARATOR;
        fmt.MiddleFmt |= FMT_FRACSEPARATOR;
        fmt.Locale = MAKELOCALE(',', THIN_SPACE, THIN_SPACE, ';');
        break;
    case 10:
        fmt.BigFmt |= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR;
        fmt.SmallFmt |= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR;
        fmt.MiddleFmt |= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR;
        fmt.Locale = MAKELOCALE(',', '.', THIN_SPACE, ';');
        break;
    case 11:
        fmt.BigFmt |= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR;
        fmt.SmallFmt |= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR;
        fmt.MiddleFmt |= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR;
        fmt.Locale = MAKELOCALE(',', THIN_SPACE, THIN_SPACE, ';');
        break;

    }

    rplSetSystemNumberFormat(&fmt);
    uiClearRenderCache();
    halRefresh(STACK_DIRTY);
}


void KH(cycleFormat)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Cycle through major number formats
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Cycle format");

    // CYCLE BETWEEN VARIOUS OPTIONS
    const char *const options[] = {
        "STD", "FIX", "SCI", "ENG"
    };

    NUMFORMAT fmt;
    int32_t option = 0;
    rplGetSystemNumberFormat(&fmt);

    if(fmt.MiddleFmt & FMT_TRAILINGZEROS)
        option = 1;
    if(fmt.MiddleFmt & FMT_SCI)
        option = 2;
    if(fmt.MiddleFmt & FMT_ENG)
        option = 3;

    // CYCLE THROUGH ITEMS:
    ++option;
    if(option > 3)
        option = 0;

    gglsurface scr;
    ggl_init_screen(&scr);
    int ytop =
            halScreen.Form + halScreen.Stack + halScreen.CmdLine +
            halScreen.Menu1;
    // CLEAR STATUS AREA
    ggl_rect(&scr, STATUS_AREA_X, ytop, LCD_W - 1,
            ytop + halScreen.Menu2 - 1, PAL_STA_BG);

    DrawTextBk(&scr, STATUS_AREA_X + 1, ytop + 1, "Display Mode:",
               FONT_STATUS, PAL_STA_TEXT, PAL_STA_BG);
    DrawTextBk(&scr, STATUS_AREA_X + 1,
               ytop + 1 + FONT_HEIGHT(FONT_STATUS),
               options[option], FONT_STATUS, PAL_STA_TEXT,PAL_STA_BG);

    // CHANGE THE FORMAT TO THE SELECTED OPTION
    switch (option) {
    default:
    case 0:
        fmt.MiddleFmt &= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS | FMT_PREFEXPMSK;  // PRESERVE ALL THESE
        fmt.BigFmt &= FMT_SCI | FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS | FMT_PREFEXPMSK;  // PRESERVE ALL THESE
        fmt.SmallFmt &= FMT_SCI | FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS | FMT_PREFEXPMSK;  // PRESERVE ALL THESE
        break;
    case 1:
        fmt.MiddleFmt &= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS | FMT_PREFEXPMSK;  // PRESERVE ALL THESE
        fmt.BigFmt &= FMT_SCI | FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS | FMT_PREFEXPMSK;  // PRESERVE ALL THESE
        fmt.SmallFmt &= FMT_SCI |FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS | FMT_PREFEXPMSK;  // PRESERVE ALL THESE

        fmt.MiddleFmt |= FMT_TRAILINGZEROS;
        fmt.BigFmt |=  FMT_TRAILINGZEROS;
        fmt.SmallFmt |= FMT_TRAILINGZEROS;

        break;
    case 2:
        fmt.MiddleFmt &= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS | FMT_PREFEXPMSK;  // PRESERVE ALL THESE
        fmt.BigFmt &= FMT_SCI | FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS | FMT_PREFEXPMSK;  // PRESERVE ALL THESE
        fmt.SmallFmt &= FMT_SCI |FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS | FMT_PREFEXPMSK;  // PRESERVE ALL THESE

        fmt.MiddleFmt |= FMT_SCI;
        fmt.BigFmt |= FMT_SCI;
        fmt.SmallFmt |= FMT_SCI;
        break;
    case 3:
        fmt.MiddleFmt &= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS | FMT_PREFEXPMSK;  // PRESERVE ALL THESE
        fmt.BigFmt &= FMT_SCI | FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS | FMT_PREFEXPMSK;  // PRESERVE ALL THESE
        fmt.SmallFmt &= FMT_SCI |FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS | FMT_PREFEXPMSK;  // PRESERVE ALL THESE

        fmt.MiddleFmt |= FMT_SCI | FMT_ENG;
        fmt.BigFmt |= FMT_SCI | FMT_ENG;
        fmt.SmallFmt |= FMT_SCI | FMT_ENG;
        if((PREFERRED_EXPRAW(fmt.MiddleFmt) == 0)
                || (PREFERRED_EXPRAW(fmt.MiddleFmt) == 8))
        {
            fmt.MiddleFmt |= FMT_SUPRESSEXP;    // SUPRESS ZERO EXPONENT ONLY WHEN FIXED EXPONENT IS ZERO
            fmt.BigFmt |= FMT_SUPRESSEXP;    // SUPRESS ZERO EXPONENT ONLY WHEN FIXED EXPONENT IS ZERO
            fmt.SmallFmt |= FMT_SUPRESSEXP;    // SUPRESS ZERO EXPONENT ONLY WHEN FIXED EXPONENT IS ZERO
        }

        break;
    }

    rplSetSystemNumberFormat(&fmt);
    uiClearRenderCache();
    halRefresh(ALL_DIRTY);
}

const const char *const onMulDivKeyHandler_options[] = {
    "Auto",    "  =  0",  "k = +3",  "M = +6",  "G = +9",  "T = +12",
    "P = +15", "E = +18", "Z = +21", "z = -21", "a = -18", "f = -15",
    "p = -12", "n = -9",  "µ = -6",  "m = -3"
};

static void formatChange(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Helper function for format changes, cycle through options
// ----------------------------------------------------------------------------
{
    NUMFORMAT fmt;
    int32_t option = 0;
    rplGetSystemNumberFormat(&fmt);

    option = PREFERRED_EXPRAW(fmt.MiddleFmt);
    if(option) {
        option -= 7;
        if(option <= 0)
            option += 15;
    }

    // CYCLE THROUGH ITEMS:
    switch(keymsg)
    {
    case KB_UP:
        if(option != 9)
            --option;
        break;
    case KB_DN:
        if(option != 8)
            ++option;
    }
    if(option < 0)
        option = 15;
    if(option > 15)
        option = 0;

    gglsurface scr;
    ggl_init_screen(&scr);
    int ytop =
            halScreen.Form + halScreen.Stack + halScreen.CmdLine +
            halScreen.Menu1;
    // CLEAR STATUS AREA
    ggl_rect(&scr, STATUS_AREA_X, ytop, LCD_W - 1,
            ytop + halScreen.Menu2 - 1, PAL_STA_BG);

    DrawTextBk(&scr, STATUS_AREA_X + 1, ytop + 1, "ENG exponent:",
               FONT_STATUS, PAL_STA_TEXT, PAL_STA_BG);
    DrawTextBk(&scr, STATUS_AREA_X + 1,
               ytop + 1 + FONT_HEIGHT(FONT_STATUS),
               onMulDivKeyHandler_options[option],
               FONT_STATUS, PAL_STA_TEXT, PAL_STA_BG);

    if(option)
        option += 7;
    if(option > 15)
        option -= 15;

    fmt.MiddleFmt &= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS;       // PRESERVE ALL THESE
    fmt.MiddleFmt |= FMT_SCI | FMT_ENG | FMT_PREFEREXPRAW(option);
    fmt.BigFmt &= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS;       // PRESERVE ALL THESE
    fmt.BigFmt |= FMT_SCI | FMT_ENG | FMT_PREFEREXPRAW(option);
    fmt.SmallFmt &= FMT_NUMSEPARATOR | FMT_FRACSEPARATOR | FMT_GROUPDIGITSMSK | FMT_USECAPITALS | FMT_NUMDIGITS;       // PRESERVE ALL THESE
    fmt.SmallFmt |= FMT_SCI | FMT_ENG | FMT_PREFEREXPRAW(option);
    if((option == 0) || (option == 8))
    {
        fmt.MiddleFmt |= FMT_SUPRESSEXP;        // SUPRESS ZERO EXPONENT ONLY WHEN FIXED EXPONENT IS ZERO
        fmt.BigFmt |= FMT_SUPRESSEXP;        // SUPRESS ZERO EXPONENT ONLY WHEN FIXED EXPONENT IS ZERO
        fmt.SmallFmt |= FMT_SUPRESSEXP;        // SUPRESS ZERO EXPONENT ONLY WHEN FIXED EXPONENT IS ZERO
    }

    rplSetSystemNumberFormat(&fmt);
    uiClearRenderCache();
    halRefresh(ALL_DIRTY);
}

void KH(nextScale)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Select next scale for display format
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Next scale");
    formatChange(KB_UP);
}

void KH(previousScale)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Select previous scale for display format
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Previous scale");
    formatChange(KB_UP);
}


void KH(onDigit)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Select the display precision
// ----------------------------------------------------------------------------
{
    utf8_p    help = "?";
    NUMFORMAT fmt;
    int32_t   digits = 0;
    switch (KM_KEY(keymsg))
    {
    case KB_0: digits = 0xfff;  help = "Show all digits";    break;
    case KB_1: digits = 1;      help = "Show 1 digit";       break;
    case KB_2: digits = 2;      help = "Show 2 digits";      break;
    case KB_3: digits = 3;      help = "Show 3 digits";      break;
    case KB_4: digits = 4;      help = "Show 4 digits";      break;
    case KB_5: digits = 5;      help = "Show 5 digits";      break;
    case KB_6: digits = 6;      help = "Show 6 digits";      break;
    case KB_7: digits = 7;      help = "Show 7 digits";      break;
    case KB_8: digits = 8;      help = "Show 8 digits";      break;
    case KB_9: digits = 9;      help = "Show 9 digits";      break;
    }
    halKeyHelp(keymsg, help);

    rplGetSystemNumberFormat(&fmt);
    fmt.MiddleFmt &= ~FMT_NUMDIGITS;
    fmt.MiddleFmt |= FMT_DIGITS(digits);


    fmt.BigFmt &= ~FMT_NUMDIGITS;
    fmt.BigFmt |= FMT_DIGITS(digits);
    fmt.SmallFmt &= ~FMT_NUMDIGITS;
    fmt.SmallFmt |= FMT_DIGITS(digits);

    fmt.SmallLimit.data=fmt.SmallLimitData;
    newRealFromint32_t(&fmt.SmallLimit,1,(digits==0xfff)? -12:-digits);

    if(digits==0xfff) digits= TEXT2WORD('A','l','l',0);
      else digits += '0';

    gglsurface scr;
    ggl_init_screen(&scr);
    int ytop =
            halScreen.Form + halScreen.Stack + halScreen.CmdLine +
            halScreen.Menu1;
    // CLEAR STATUS AREA
    ggl_rect(&scr, STATUS_AREA_X, ytop, LCD_W - 1,
            ytop + halScreen.Menu2 - 1, PAL_STA_BG);

    DrawTextBk(&scr, STATUS_AREA_X + 1, ytop + 1, "Display Digits:",
               FONT_STATUS, PAL_STA_TEXT, PAL_STA_BG);
    DrawTextBk(&scr, STATUS_AREA_X + 1,
               ytop + 1 + FONT_HEIGHT(FONT_STATUS),
               (cstring) &digits, FONT_STATUS, PAL_STA_TEXT, PAL_STA_BG);

    rplSetSystemNumberFormat(&fmt);
    uiClearRenderCache();
    halRefresh(ALL_DIRTY);
}


void KH(SetPrecision)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//    Change precision for computations
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg,
               KM_KEY(keymsg) == KB_UP ? "Increase precision"
                                       : "Decrease precision");
    int32_t precision = Context.precdigits;

    if(KM_KEY(keymsg) == KB_UP)
        precision += 8;
    else
        precision -= 8;

    if(precision < 8)
        precision = 8;
    if(precision > MAX_USERPRECISION)
        precision = MAX_USERPRECISION;

    Context.precdigits = precision;

    char digits_string[12], empty = ' ';

    stringcpy(digits_string, "0000 digits");

    if(precision >= 1000) {
        digits_string[0] = precision / 1000 + '0';
        precision = precision % 1000;
        empty = '0';
    }
    else
        digits_string[0] = empty;
    if(precision >= 100) {
        digits_string[1] = precision / 100 + '0';
        precision = precision % 100;
        empty = '0';
    }
    else
        digits_string[1] = empty;
    if(precision >= 10) {
        digits_string[2] = precision / 10 + '0';
        precision = precision % 10;
        empty = '0';
    }
    else
        digits_string[2] = empty;
    digits_string[3] = precision + '0';

    gglsurface scr;
    ggl_init_screen(&scr);
    int ytop =
            halScreen.Form + halScreen.Stack + halScreen.CmdLine +
            halScreen.Menu1;
    // CLEAR STATUS AREA
    ggl_rect(&scr, STATUS_AREA_X, ytop, LCD_W - 1,
            ytop + halScreen.Menu2 - 1, PAL_STA_BG);

    DrawTextBk(&scr, STATUS_AREA_X + 1, ytop + 1, "System precision:",
               FONT_STATUS, PAL_STA_TEXT,PAL_STA_BG);
    DrawTextBk(&scr, STATUS_AREA_X + 1,
               ytop + 1 + FONT_HEIGHT(FONT_STATUS),
               digits_string, FONT_STATUS, PAL_STA_TEXT, PAL_STA_BG);
    halRefresh(STACK_DIRTY);
}


void KH(SkipNextAlarm)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Skip next alarm
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Skip next alarm");
    halFlags |= HAL_SKIPNEXTALARM;

}

void SelectMenuKeyHandler(keyb_msg_t keymsg, int64_t menucode, utf8_p help)
// ----------------------------------------------------------------------------
//   Select a given menu
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, help);
    word_p numobject = rplNewBINT(menucode, HEXBINT);

    if(!numobject || Exceptions)
        return;
    int32_t menu = rplGetActiveMenu();

    rplPushDataNoGrow(numobject);
    rplSaveMenuHistory(menu);
    rplChangeMenu(menu, rplPopData());
    halRefresh(MENU_DIRTY);
}


static int backMenuKeyHandler(int menu)
// ----------------------------------------------------------------------------
//   Helper function to move back a given menu
// ----------------------------------------------------------------------------
{
    if(menu < 1 || menu > 2)
        return 0;

    word_p oldmenu = rplPopMenuHistory(menu);
    if(oldmenu)
        rplChangeMenu(menu, oldmenu);
    return oldmenu != 0;
}


void KH(backmenu1)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Go back on menu 1
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Previous menu");
    backMenuKeyHandler(1);
    halRefresh(MENU1_DIRTY);
}


void KH(backmenu2)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Go back on menu 2
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Previous secondary menu");
    backMenuKeyHandler(2);
    halRefresh(MENU1_DIRTY);
}


void customKeyHandler(keyb_msg_t keymsg, word_p action)
// ----------------------------------------------------------------------------
//   Process a custom action
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, halCommandName(action));

    int32_t inlist = 0;
    // COMMANDS CAN BE PUT INSIDE LISTS
    if(ISLIST(*action)) {
        action = rplGetListElement(action, 1);
        if(!action)
            return;
        if(*action == CMD_ENDLIST)
            return;     // EMPTY LIST!
        inlist = 1;
    }

    // DEFAULT MESSAGE
    if(!halContext(CONTEXT_EDITOR)) {
        // ACTION WHEN IN THE STACK OR SUBCONTEXTS OTHER THAN THE EDITOR
        WORD Opcode = 0;
        int32_t hideargument = 1;

        // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

        if(ISIDENT(*action)) {
            // JUST EVAL THE VARIABLE
            rplPushData(action);        // PUSH THE NAME ON THE STACK
            Opcode = (CMD_OVR_EVAL1);
        }
        else if((!IS_PROLOG(*action)) && (!ISint32_t(*action))) {
            // THIS IS AN OPCODE, EXECUTE DIRECTLY
            Opcode = *action;
            hideargument = 0;
        }
        else if(ISSTRING(*action) && inlist) {
            // A STRING TO INSERT INTO THE EDITOR
            // OPEN AN EDITOR AND INSERT THE STRING
            halOpenCmdLine(keymsg, 'D');

            utf8_p  start = (utf8_p) (action + 1);
            utf8_p  end   = start + rplStrSize(action);
            int32_t nlines = uiInsertCharactersN(start, end);
            if(nlines)
                uiStretchCmdLine(nlines);
            uiAutocompleteUpdate();

        }
        else {
            // ALL OTHER OBJECTS AND COMMANDS, DO XEQ
            rplPushData(action);
            Opcode = (CMD_OVR_XEQ);
        }

        if(Opcode)
            uiCmdRunHide(Opcode, hideargument);
        halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
    }
    else {
        // ACTION INSIDE THE EDITOR
        WORD Opcode = 0;
        int32_t hideargument = 1;

        if(!action)
            return;

        // DO DIFFERENT ACTIONS BASED ON OBJECT TYPE

        if(ISIDENT(*action) && inlist) {
            switch (halScreen.CursorState & 0xff) {
            case 'D':
            {
                // HANDLE DIRECTORIES IN A SPECIAL WAY: DON'T CLOSE THE COMMAND LINE
                word_p *var = rplFindGlobal(action, 1);
                if(var) {
                    if(ISDIR(*(var[1]))) {
                        // CHANGE THE DIR WITHOUT CLOSING THE COMMAND LINE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode = (CMD_OVR_EVAL);
                        break;
                    }
                }
                rplPushRet(action);
                int32_t result = endCmdLineAndCompile();
                action = rplPopRet();
                if(result) {
                    // USER IS TRYING TO EVAL THE VARIABLE
                    rplPushData(action);        // PUSH THE NAME ON THE STACK
                    Opcode = (CMD_OVR_EVAL);
                }
                break;
            }
            case 'A':
            {
                word_p *var = rplFindGlobal(action, 1);
                if(var) {
                    if(ISDIR(*(var[1]))) {
                        // CHANGE THE DIR WITHOUT CLOSING THE COMMAND LINE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode = (CMD_OVR_EVAL);
                        break;
                    }
                }

                utf8_p string, endstring;
                if (!rplGetDecompiledStringWithoutTickmarks(action, DECOMP_EDIT, &string, &endstring))
                    break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                uiInsertCharactersN(string, endstring);
                uiAutocompleteUpdate();

                break;
            }

            case 'P':
            {
                word_p *var = rplFindGlobal(action, 1);
                if(var) {
                    if(ISDIR(*(var[1]))) {
                        // CHANGE THE DIR WITHOUT CLOSING THE COMMAND LINE
                        rplPushData(action);    // PUSH THE NAME ON THE STACK
                        Opcode = (CMD_OVR_EVAL);
                        break;
                    }
                }

                utf8_p string, endstring;
                if (!rplGetDecompiledStringWithoutTickmarks(action, DECOMP_EDIT, &string, &endstring))
                    break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                uiSeparateToken();
                uiInsertCharactersN(string, endstring);

                uiSeparateToken();
                uiAutocompleteUpdate();

                break;
            }
            }
        }
        else if(ISUNIT(*action)) {
            switch (halScreen.CursorState & 0xff) {
            case 'D':
            {
                rplPushRet(action);
                int32_t result = endCmdLineAndCompile();
                action = rplPopRet();
                if(result) {
                    // USER IS TRYING TO APPLY THE UNIT
                    rplPushData(action);        // PUSH THE NAME ON THE STACK
                    Opcode = (CMD_OVR_MUL);
                }
                break;
            }
            case 'A':
            {
                utf8_p string, endstring;
                int32_t totaln = rplGetDecompiledString(action, DECOMP_EDIT, &string, &endstring);
                if (!totaln)
                    break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                if((totaln > 2) && (string[0] == '1') && (string[1] == '_'))
                    string += 2;

                uiInsertCharactersN(string, endstring);

                uiAutocompleteUpdate();

                break;
            }

            case 'P':
            {
                utf8_p string, endstring;
                if (!rplGetDecompiledString(action, DECOMP_EDIT, &string, &endstring))
                    break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                uiSeparateToken();
                uiInsertCharactersN(string, endstring);
                uiSeparateToken();
                uiInsertCharacters("*");
                uiSeparateToken();
                uiAutocompleteUpdate();

                break;
            }

            }
        }
        else if(!IS_PROLOG(*action)) {
            // THIS IS A COMMAND, DECOMPILE AND INSERT NAME
            switch (halScreen.CursorState & 0xff) {
            case 'D':
            {
                rplPushRet(action);
                int32_t result = endCmdLineAndCompile();
                action = rplPopRet();
                if(result) {
                    Opcode = *action;
                    hideargument = 0;
                }
                break;
            }
            case 'A':
            {

                WORD tokeninfo = 0;
                LIBHANDLER han = rplGetLibHandler(LIBNUM(*action));

                // GET THE SYMBOLIC TOKEN INFORMATION
                if(han) {
                    WORD savecurOpcode = CurOpcode;
                    DecompileObject = action;
                    CurOpcode = MK_OPCODE(LIBNUM(*action), OPCODE_GETINFO);
                    (*han) ();

                    if(RetNum > OK_TOKENINFO)
                        tokeninfo = RetNum;

                    CurOpcode = savecurOpcode;
                }
                utf8_p string, endstring;
                if (!rplGetDecompiledString(action, DECOMP_EDIT | DECOMP_NOHINTS, &string, &endstring))
                    break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                uiInsertCharactersN(string, endstring);
                if(TI_TYPE(tokeninfo) == TITYPE_FUNCTION) {
                    uiInsertCharacters("()");
                    uiCursorLeft(1);
                }
                uiAutocompleteUpdate();

                break;
            }

            case 'P':
            {
                utf8_p string, endstring;
                if (!rplGetDecompiledString(action, DECOMP_EDIT, &string, &endstring))
                    break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                uiSeparateToken();
                int32_t nlines = uiInsertCharactersN(string, endstring);
                if(nlines)
                    uiStretchCmdLine(nlines);

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
                Opcode = CMD_OVR_XEQ;
            }
            else {
                rplPushRet(action);
                int32_t result = endCmdLineAndCompile();
                action = rplPopRet();
                if(result) {
                    rplPushData(action);        // PUSH THE NAME ON THE STACK
                    Opcode = CMD_OVR_XEQ;
                }
            }
        }
        else if(ISSTRING(*action)) {
            utf8_p string, endstring;
            rplGetStringPointers(action, &string, &endstring);

            if(!inlist) {
                // ADD THE QUOTES IN D OR P MODE
                if(((halScreen.CursorState & 0xff) == 'P')
                        || ((halScreen.CursorState & 0xff) == 'D')) {
                    uiSeparateToken();
                    uiInsertCharacters("\"");
                }
            }
            uiInsertCharactersN(string, endstring);
            if(!inlist && (((halScreen.CursorState & 0xff) == 'P')
                        || ((halScreen.CursorState & 0xff) == 'D'))) {
                uiInsertCharacters("\"");
                uiSeparateToken();
            }
            uiAutocompleteUpdate();
        }
        else {
            // ALL OTHER OBJECTS AND COMMANDS
            switch (halScreen.CursorState & 0xff) {
            case 'D':
            {
                rplPushRet(action);
                int32_t result = endCmdLineAndCompile();
                action = rplPopRet();
                if(result) {
                    if(!IS_PROLOG(*action)) {
                        Opcode = *action;       // RUN COMMANDS DIRECTLY
                        hideargument = 0;
                    }
                    else {
                        Opcode = (CMD_OVR_XEQ);
                        rplPushData(action);
                    }
                }
                break;
            }
            case 'A':
            {

                WORD tokeninfo = 0;
                LIBHANDLER han = rplGetLibHandler(LIBNUM(*action));

                // GET THE SYMBOLIC TOKEN INFORMATION
                if(han) {
                    WORD savecurOpcode = CurOpcode;
                    DecompileObject = action;
                    CurOpcode = MK_OPCODE(LIBNUM(*action), OPCODE_GETINFO);
                    (*han) ();

                    if(RetNum > OK_TOKENINFO)
                        tokeninfo = RetNum;

                    CurOpcode = savecurOpcode;
                }
                utf8_p string, endstring;
                if (!rplGetDecompiledString(action, DECOMP_EDIT | DECOMP_NOHINTS, &string, &endstring))
                    break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                uiInsertCharactersN(string, endstring);
                if(TI_TYPE(tokeninfo) == TITYPE_FUNCTION) {
                    uiInsertCharacters("()");
                    uiCursorLeft(1);
                }
                uiAutocompleteUpdate();

                break;
            }

            case 'P':
            {
                utf8_p string, endstring;
                if (!rplGetDecompiledString(action, DECOMP_EDIT, &string, &endstring))
                    break; // ERROR WITHIN A MENU PROGRAM! JUST IGNORE FOR NOW

                uiSeparateToken();
                int32_t nlines = uiInsertCharactersN(string, endstring);
                if(nlines)
                    uiStretchCmdLine(nlines);

                uiSeparateToken();
                uiAutocompleteUpdate();

                break;
            }

            }

        }

        if(Opcode)
            uiCmdRunHide(Opcode, hideargument);
        halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
    }

}

void KH(formswitcher)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Switch to/from form
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Switch form");

    if (halContext(CONTEXT_FORM))
    {
        // The user is running a form or other context, just close it
        // uiCloseFormEvent();
        halSwitch2Stack();
        return;
    }
    halSwitch2Form();
    return;
}

void KH(switchmenukeys)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Switch menus
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Switch menus");

    // TODO: Just toggle a flag for the key handlers
    halKeyMenuSwitch ^= 1;
    halRefresh(MENU_DIRTY);
    return;
}

void KH(basecycle)(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Cycle numeric base
// ----------------------------------------------------------------------------
{
    halKeyHelp(keymsg, "Cycle base");

    if(!halContext(CONTEXT_EDITOR)) {
        if(halContext(CONTEXT_STACK)) {
            // ACTION WHEN IN THE STACK, CYCLE THROUGH DIFFERENT BASES

            rplPushDataNoGrow((word_p) lib70_basecycle);
            uiCmdRunHide(CMD_OVR_XEQ, 1);
            halRefresh(STACK_DIRTY);
        }

    }
    else {

        // ACTION INSIDE THE EDITOR

        utf8_p  startnum, endnum;
        utf8_p  line;
        int32_t numflags;

        // FIRST CASE: IF TOKEN UNDER THE CURSOR IS OR CONTAINS A VALID NUMBER,
        // CHANGE THE BASE OF THE NUMBER IN THE TEXT
        startnum = uiFindNumberStart(&endnum, &numflags);
        line = (utf8_p) (CmdLineCurrentLine + 1);
        if(!startnum) {

            return;

        }
        else {
            // WE FOUND A NUMBER
            int32_t oldposition = halScreen.CursorPosition;
            uiMoveCursor(startnum - line);
            BYTE str[2];
            int32_t endchar, minbase = numflags & 0xffff;

            numflags >>= 16;
            str[1] = 0;
            str[0] = '#';

            if(endnum > startnum)
                endchar = *endnum;
            else
                endchar = 0;

            // TODO: FIND THE END OF THE TOKEN, AND CYCLE THE LETTER
            switch (endchar) {
            case 'b':
            case 'B':
                endchar = 'o';
                break;
            case 'o':
            case 'O':
                endchar = 'h';
                break;
            case 'h':
            case 'H':
                endchar = -1;
                break;
            default:
                endchar = 0;
            }

            if(endchar < 0) {
                if(minbase <= 10) {
                    // REMOVE NUMERAL SIGN
                    if(startnum[0] == '#') {
                        uiRemoveCharacters(1);
                        if(oldposition > startnum - line)
                            --oldposition;
                        --endnum;
                    }
                }
                else if(minbase == 2)
                    endchar = 'b';
                else if(minbase == 8)
                    endchar = 'o';
                else
                    endchar = 'h';
            }
            else {
                // ADD NUMERAL SIGN IF NOT THERE YET AND IF THERE'S NO EXPONENT WITH A SIGN
                if(!(numflags & 8) && (startnum[0] != '#')) {
                    uiInsertCharacters((utf8_p) str);
                    if(oldposition > startnum - line)
                        ++oldposition;
                    ++endnum;
                }
            }

            if(endchar != 0) {
                uiMoveCursor(endnum - line);
                uiRemoveCharacters(1);
                if(oldposition > endnum - line)
                    --oldposition;
            }
            else {
                uiMoveCursor(endnum - line + 1);
                if(minbase == 2)
                    endchar = 'b';
                else if(minbase == 8)
                    endchar = 'o';
                else
                    endchar = 'h';
            }
            if((endchar > 0) && !(numflags & 8))        // IF WE NEED TO SET A BASE AND THERE'S NO EXPONENT SIGN IN BETWEEN
            {

                str[0] = endchar;
                uiInsertCharacters((utf8_p) str);
                if(oldposition >= endnum - line)
                    ++oldposition;
            }

            uiMoveCursor(oldposition);
            uiEnsureCursorVisible();
            uiAutocompleteUpdate();
            return;
        }

        // THIRD CASE: IF TOKEN UNDER CURSOR IS SOMETHING OTHER THAN A NUMBER, JUST DO NOTHING

    }
}

#define TRANSP_CMD_KEY_HANDLER(name, opcode) \
    void name##KeyHandler(keyb_msg_t keymsg) \
    {                                        \
        halKeyHelp(keymsg, #name);           \
        transpcmdKeyHandler(opcode);         \
    }                                        \
    extern int dummy

#define CMD_KEY_HANDLER(name, opcode, string, issymbfunc)   \
    void name##KeyHandler(keyb_msg_t keymsg)                \
    {                                                       \
        halKeyHelp(keymsg, string);                         \
        cmdKeyHandler(opcode, (utf8_p) string, issymbfunc); \
    }                                                       \
    extern int dummy

#define FUNCTION_KEY_HANDLER(name, menu, idx)                \
    void name##KeyHandler(keyb_msg_t keymsg)                 \
    {                                                        \
        functionKeyHandler(keymsg, (menu), (int32_t) (idx)); \
    }                                                        \
    extern int dummy

#define MENU_KEY_HANDLER(name, menucode)                          \
    void name##MenuKeyHandler(keyb_msg_t keymsg)                  \
    {                                                             \
        SelectMenuKeyHandler(keymsg, (menucode), #name " menu"); \
    }                                                             \
    extern int dummy

#define KEY_HANDLER(name, lsymbol, csymbol)                  \
    void name##KeyHandler(keyb_msg_t keymsg)                 \
    {                                                        \
        alphasymbolKeyHandler(keymsg, (lsymbol), (csymbol)); \
    }                                                        \
    extern int dummy

#define SYMB_KEY_HANDLER(name, symbol, sep)        \
    void name##KeyHandler(keyb_msg_t keymsg)       \
    {                                              \
        symbolKeyHandler(keymsg, (symbol), (sep)); \
    }                                              \
    extern int dummy

KEY_HANDLER(a, "a", "A");
KEY_HANDLER(b, "b", "B");
KEY_HANDLER(c, "c", "C");
KEY_HANDLER(d, "d", "D");
KEY_HANDLER(e, "e", "E");
KEY_HANDLER(f, "f", "F");
KEY_HANDLER(g, "g", "G");
KEY_HANDLER(h, "h", "H");
KEY_HANDLER(i, "i", "I");
KEY_HANDLER(j, "j", "J");
KEY_HANDLER(k, "k", "K");
KEY_HANDLER(l, "l", "L");
KEY_HANDLER(m, "m", "M");
KEY_HANDLER(n, "n", "N");
KEY_HANDLER(o, "o", "O");
KEY_HANDLER(p, "p", "P");
KEY_HANDLER(q, "q", "Q");
KEY_HANDLER(r, "r", "R");
KEY_HANDLER(s, "s", "S");
KEY_HANDLER(t, "t", "T");
KEY_HANDLER(u, "u", "U");
KEY_HANDLER(v, "v", "V");
KEY_HANDLER(w, "w", "W");
KEY_HANDLER(x, "x", "X");
KEY_HANDLER(y, "y", "Y");
KEY_HANDLER(z, "z", "Z");
SYMB_KEY_HANDLER(arrow, "→", 1);
SYMB_KEY_HANDLER(rulesep, ":→", 1);
SYMB_KEY_HANDLER(comma, ",", 0);
SYMB_KEY_HANDLER(semi, ";", 0);
SYMB_KEY_HANDLER(colon, ":", 0);
SYMB_KEY_HANDLER(infinity, "∞", 1);
SYMB_KEY_HANDLER(undinfinity, "∞̅", 1);
SYMB_KEY_HANDLER(dot, ".", 0);
SYMB_KEY_HANDLER(giventhat, "|", 0);
SYMB_KEY_HANDLER(question, "?", 0);
SYMB_KEY_HANDLER(openquestion, "¿", 0);
SYMB_KEY_HANDLER(exclamation, "!", 0);
SYMB_KEY_HANDLER(openexclamation, "¡", 0);
SYMB_KEY_HANDLER(approx, "~", 0);
SYMB_KEY_HANDLER(percent, "%", 0);
SYMB_KEY_HANDLER(dollar, "$", 0);
SYMB_KEY_HANDLER(euro, "€", 0);
SYMB_KEY_HANDLER(backslash, "\\", 0);
SYMB_KEY_HANDLER(pound, "£", 0);
SYMB_KEY_HANDLER(angle, "∡", 0);
SYMB_KEY_HANDLER(degree, "°", 0);
SYMB_KEY_HANDLER(pi, "π", 1);
SYMB_KEY_HANDLER(delta, "Δ", 0);
SYMB_KEY_HANDLER(at, "@", 0);
SYMB_KEY_HANDLER(and, "&", 0);
SYMB_KEY_HANDLER(econst, "е", 0);
SYMB_KEY_HANDLER(iconst, "і", 0);
SYMB_KEY_HANDLER(jconst, "ј", 0);
SYMB_KEY_HANDLER(greekalpha, "α", 0);
SYMB_KEY_HANDLER(greekbeta, "β", 0);
SYMB_KEY_HANDLER(greekgamma, "γ", 0);
SYMB_KEY_HANDLER(greekdelta, "δ", 0);
SYMB_KEY_HANDLER(greekepsilon, "ε", 0);
SYMB_KEY_HANDLER(greeketa, "η", 0);
SYMB_KEY_HANDLER(greekrho, "ρ", 0);
SYMB_KEY_HANDLER(greeksigma, "σ", 0);
SYMB_KEY_HANDLER(greektau, "τ", 0);
SYMB_KEY_HANDLER(greektheta, "θ", 0);
SYMB_KEY_HANDLER(greeklambda, "λ", 0);
SYMB_KEY_HANDLER(greekkappa, "κ", 0);
SYMB_KEY_HANDLER(greekmu, "μ", 0);
SYMB_KEY_HANDLER(greeknu, "ν", 0);
SYMB_KEY_HANDLER(greekphi, "φ", 0);
SYMB_KEY_HANDLER(greekomega, "ω", 0);
SYMB_KEY_HANDLER(greekgammacap, "Γ", 0);
SYMB_KEY_HANDLER(greeklambdacap, "Λ", 0);
SYMB_KEY_HANDLER(greekomegacap, "Ω", 0);
SYMB_KEY_HANDLER(greekpicap, "Π", 0);
SYMB_KEY_HANDLER(greeksigmacap, "Σ", 0);
SYMB_KEY_HANDLER(greekthetacap, "Θ", 0);
SYMB_KEY_HANDLER(greekphicap, "Φ", 0);
SYMB_KEY_HANDLER(micro, "µ", 0);
FUNCTION_KEY_HANDLER(softmenu1_1, 1, 0);
FUNCTION_KEY_HANDLER(softmenu2_1, 1, 1);
FUNCTION_KEY_HANDLER(softmenu3_1, 1, 2);
FUNCTION_KEY_HANDLER(softmenu4_1, 1, 3);
FUNCTION_KEY_HANDLER(softmenu5_1, 1, 4);
FUNCTION_KEY_HANDLER(softmenu6_1, 1, 5);
FUNCTION_KEY_HANDLER(softmenu1_2, 2, 0);
FUNCTION_KEY_HANDLER(softmenu2_2, 2, 1);
FUNCTION_KEY_HANDLER(softmenu3_2, 2, 2);
FUNCTION_KEY_HANDLER(softmenu4_2, 2, 3);
FUNCTION_KEY_HANDLER(softmenu5_2, 2, 4);
FUNCTION_KEY_HANDLER(softmenu6_2, 2, 5);
SYMB_KEY_HANDLER(thinspc, " ", 0);
SYMB_KEY_HANDLER(hash, "#", 0);
SYMB_KEY_HANDLER(equal, "=", 1);
SYMB_KEY_HANDLER(notequal, "≠", 1);
SYMB_KEY_HANDLER(ls, "<", 1);
SYMB_KEY_HANDLER(gt, ">", 1);
SYMB_KEY_HANDLER(le, "≤", 1);
SYMB_KEY_HANDLER(ge, "≥", 1);
SYMB_KEY_HANDLER(sadd, "+", 0);
SYMB_KEY_HANDLER(ssub, "-", 0);
SYMB_KEY_HANDLER(smul, "*", 0);
SYMB_KEY_HANDLER(sdiv, "/", 0);
SYMB_KEY_HANDLER(spow, "^", 0);
KEY_HANDLER(sub0, "₀", "⁰");
KEY_HANDLER(sub1, "₁", "¹");
KEY_HANDLER(sub2, "₂", "²");
KEY_HANDLER(sub3, "₃", "³");
KEY_HANDLER(sub4, "₄", "⁴");
KEY_HANDLER(sub5, "₅", "⁵");
KEY_HANDLER(sub6, "₆", "⁶");
KEY_HANDLER(sub7, "₇", "⁷");
KEY_HANDLER(sub8, "₈", "⁸");
KEY_HANDLER(sub9, "₉", "⁹");
SYMB_KEY_HANDLER(eqnVar, "X", 0); // REVISIT: Depend on state, e.g. rho/theta
CMD_KEY_HANDLER(clear, CMD_CLEAR, "CLEAR", -1);
CMD_KEY_HANDLER(add, CMD_OVR_ADD, "+", 0);
CMD_KEY_HANDLER(sub, CMD_OVR_SUB, "-", 0);
CMD_KEY_HANDLER(div, CMD_OVR_DIV, "/", 0);
CMD_KEY_HANDLER(mul, CMD_OVR_MUL, "*", 0);
CMD_KEY_HANDLER(fact, CMD_FACTORIAL, "!", 0);
CMD_KEY_HANDLER(inv, CMD_OVR_INV, "INV", 1);
CMD_KEY_HANDLER(sin, CMD_SIN, "SIN", 1);
CMD_KEY_HANDLER(asin, CMD_ASIN, "ASIN", 1);
CMD_KEY_HANDLER(sinh, CMD_SINH, "SINH", 1);
CMD_KEY_HANDLER(asinh, CMD_ASINH, "ASINH", 1);
CMD_KEY_HANDLER(cos, CMD_COS, "COS", 1);
CMD_KEY_HANDLER(acos, CMD_ACOS, "ACOS", 1);
CMD_KEY_HANDLER(cosh, CMD_COSH, "COSH", 1);
CMD_KEY_HANDLER(acosh, CMD_ACOSH, "ACOSH", 1);
CMD_KEY_HANDLER(tan, CMD_TAN, "TAN", 1);
CMD_KEY_HANDLER(atan, CMD_ATAN, "ATAN", 1);
CMD_KEY_HANDLER(tanh, CMD_TANH, "TANH", 1);
CMD_KEY_HANDLER(atanh, CMD_ATANH, "ATANH", 1);
CMD_KEY_HANDLER(eval, CMD_OVR_EVAL, "EVAL", -1);
CMD_KEY_HANDLER(eval1, CMD_OVR_EVAL1, "EVAL1", -1);
CMD_KEY_HANDLER(tonum, CMD_OVR_NUM, "→NUM", -1);
CMD_KEY_HANDLER(tofrac, CMD_TOFRACTION, "→Q", -1);
CMD_KEY_HANDLER(sqrt, CMD_SQRT, "√", 0);
CMD_KEY_HANDLER(swap, CMD_SWAP, "SWAP", 0);
CMD_KEY_HANDLER(pow, CMD_OVR_POW, "^", 0);
CMD_KEY_HANDLER(ln, CMD_LN, "LN", 1);
CMD_KEY_HANDLER(exp, CMD_EXP, "EXP", 1);
CMD_KEY_HANDLER(log, CMD_LOG, "LOG", 1);
CMD_KEY_HANDLER(alog, CMD_ALOG, "ALOG", 1);
CMD_KEY_HANDLER(sq, CMD_SQ, "SQ", 1);
CMD_KEY_HANDLER(xroot, CMD_OVR_XROOT, "XROOT", 1);
CMD_KEY_HANDLER(sto, CMD_STO, "STO", 2);
CMD_KEY_HANDLER(rcl, CMD_RCL, "RCL", 2);
CMD_KEY_HANDLER(purge, CMD_PURGE, "PURGE", -1);
CMD_KEY_HANDLER(abs, CMD_OVR_ABS, "ABS", 1);
CMD_KEY_HANDLER(arg, CMD_ARG, "ARG", 1);
CMD_KEY_HANDLER(convert, CMD_CONVERT, "CONVERT", -1);
CMD_KEY_HANDLER(cont, CMD_CONT, "CONT", -1);
TRANSP_CMD_KEY_HANDLER(UPDIR, CMD_UPDIR);
TRANSP_CMD_KEY_HANDLER(HOME, CMD_HOME);
TRANSP_CMD_KEY_HANDLER(MENUSWAP, CMD_MENUSWAP);

MENU_KEY_HANDLER(Main,       MK_MENU_CODE(0, 68, 2, 0));
MENU_KEY_HANDLER(Units,      MK_MENU_CODE(0, DOUNIT, 0, 0));
MENU_KEY_HANDLER(System,     MK_MENU_CODE(0, 68, 3, 0));
MENU_KEY_HANDLER(Program,    MK_MENU_CODE(0, 68, 3, 0));
MENU_KEY_HANDLER(Stack,      MK_MENU_CODE(0, 68, 3, 0));
MENU_KEY_HANDLER(Tests,      MK_MENU_CODE(0, 68, 3, 0));        // REVISIT
MENU_KEY_HANDLER(Lists,      MK_MENU_CODE(0, 68, 3, 0));         // REVISIT
MENU_KEY_HANDLER(Chars,      MK_MENU_CODE(0, 68, 3, 0));        // REVISIT
MENU_KEY_HANDLER(Symbols,    MK_MENU_CODE(0, 68, 3, 0));      // REVISIT
MENU_KEY_HANDLER(Memory,     MK_MENU_CODE(0, 68, 3, 0));          // REVISIT
MENU_KEY_HANDLER(Settings,   MK_MENU_CODE(0, 68, 3, 0));     // REVISIT
MENU_KEY_HANDLER(Flags,      MK_MENU_CODE(0, 68, 3, 0)); // REVISIT
MENU_KEY_HANDLER(Variables,  MK_MENU_CODE(1, 0, 0, 0));
MENU_KEY_HANDLER(Info,       MK_MENU_CODE(0, 68, 2, 0));   // REVISIT
MENU_KEY_HANDLER(Matrix,     MK_MENU_CODE(0, 68, 2, 0)); // REVISIT
MENU_KEY_HANDLER(Constants,  MK_MENU_CODE(0, 68, 2, 0)); // REVISIT
MENU_KEY_HANDLER(Notes,      MK_MENU_CODE(0, 68, 2, 0));  // REVISIT
MENU_KEY_HANDLER(Math,       MK_MENU_CODE(0, 64, 0, 0)); // REVISIT
MENU_KEY_HANDLER(Arithmetic, MK_MENU_CODE(0, 64, 0, 0));
MENU_KEY_HANDLER(Statistics, MK_MENU_CODE(0, 64, 0, 0)); // REVISIT
MENU_KEY_HANDLER(Proba,      MK_MENU_CODE(0, 64, 0, 0)); // REVISIT
MENU_KEY_HANDLER(Equations,  MK_MENU_CODE(0, 64, 0, 0)); // REVISIT
MENU_KEY_HANDLER(Complex,    MK_MENU_CODE(0, 30, 0, 0));
MENU_KEY_HANDLER(Time,       MK_MENU_CODE(0, 65, 0, 0));
MENU_KEY_HANDLER(Devices,    MK_MENU_CODE(0, 65, 0, 0));
MENU_KEY_HANDLER(Storage,    MK_MENU_CODE(0, 65, 0, 0));
MENU_KEY_HANDLER(Print,      MK_MENU_CODE(0, 65, 0, 0));
MENU_KEY_HANDLER(Bases,      MK_MENU_CODE(0, 70, 0, 0));
MENU_KEY_HANDLER(Libraries,  MK_MENU_CODE(2, 0, 0, 0));
MENU_KEY_HANDLER(Solver,     MK_MENU_CODE(0, 104, 0, 0));
MENU_KEY_HANDLER(Finance,    MK_MENU_CODE(0, 104, 1, 0));

void KH(underscore)(keyb_msg_t keymsg)
{
    halKeyHelp(keymsg, "Underscore");
    symbolKeyHandler(keymsg, "_", 0);

    if((halGetCmdLineMode() != 'L') && (halGetCmdLineMode() != 'C')) {
        uiInsertCharacters("[]");
        uiCursorLeft(1);
        halSetCmdLineMode('A');
    }
}

void KH(spc)(keyb_msg_t keymsg)
{
    halRepeatingKey(keymsg);
    if(halContext(CONTEXT_INTERACTIVE_STACK))
    {
        // SELECTION MODE
        switch (halScreen.StkSelStatus) {
        case 0:
            // NOTHING SELECTED YET
            halScreen.StkSelStart =
                    (halScreen.StkPointer ? halScreen.StkPointer : 1);
            if(halScreen.StkSelStart > rplDepthData())
                halScreen.StkSelStart = rplDepthData();
            ++halScreen.StkSelStatus;
            halRefresh(STACK_DIRTY);
            break;
        case 1:
            // START WAS SELECTED
            if(halScreen.StkSelStart > halScreen.StkPointer) {
                halScreen.StkSelEnd = halScreen.StkSelStart;
                halScreen.StkSelStart =
                        (halScreen.StkPointer ? halScreen.StkPointer : 1);
            }
            else {
                halScreen.StkSelEnd =
                        (halScreen.StkPointer ? halScreen.StkPointer : 1);
                if(halScreen.StkSelEnd > rplDepthData())
                    halScreen.StkSelEnd = rplDepthData();

            }
            ++halScreen.StkSelStatus;
            halRefresh(STACK_DIRTY);
            break;
        case 2:
            // BOTH START AND END SELECTED, JUST CLEAR THE SELECTION
            halScreen.StkSelStatus = 0;
            halRefresh(STACK_DIRTY);
            break;
        }

        return;

    }

    // We marked the key as repeating above
    // This means space acts on key down, not key press
    keymsg &= ~(KFLAG_CHANGED | KFLAG_LONG_PRESS | KFLAG_REPEAT);
    symbolKeyHandler(keymsg, " ", 0);

}

// INTERACTIVE STACK ONLY
void KH(tolist)(keyb_msg_t keymsg)
{
    halKeyHelp(keymsg, "TOLIST");
    switch (halScreen.StkSelStatus) {
    case 0:
        // NO ITEM SELECTED, MAKE A ONE-ELEMENT LIST
        if((rplDepthData() >= halScreen.StkPointer)
                && (halScreen.StkPointer > 0)) {

            word_p newlist = rplCreateListN(1, halScreen.StkPointer, 0);
            if(!newlist || Exceptions) {
                rplBlameError(0);
                return;
            }
            rplOverwriteData(halScreen.StkPointer, newlist);

        }
        break;

    case 1:
    {
        // MAKE A LIST BETWEEN SELSTART AND STKPOINTER
        int32_t endlvl, stlvl;

        if(halScreen.StkPointer > halScreen.StkSelStart) {
            stlvl = halScreen.StkSelStart;
            endlvl = (halScreen.StkPointer >
                    rplDepthData())? rplDepthData() : halScreen.StkPointer;
        }
        else {
            endlvl = halScreen.StkSelStart;
            stlvl = (halScreen.StkPointer > 0) ? halScreen.StkPointer : 1;
        }

        // MAKE A LIST
        word_p newlist = rplCreateListN(endlvl - stlvl + 1, stlvl, 0);
        if(!newlist || Exceptions) {
            rplBlameError(0);
            return;
        }
        rplOverwriteData(stlvl, newlist);
        if(endlvl - stlvl > 0)
            rplRemoveAtData(stlvl + 1, endlvl - stlvl);
        // AND END THE SELECTION
        halScreen.StkPointer = stlvl;
        halScreen.StkVisibleLvl = -1;
        halScreen.StkSelStatus = 0;

        break;
    }

    case 2:
        // START AND END SELECTED, MOVE THE BLOCK INTO A LIST AT CURSOR
    {
        // MAKE A LIST BETWEEN SELSTART AND SELEND
        int32_t endlvl, stlvl;
        endlvl = halScreen.StkSelEnd;
        stlvl = halScreen.StkSelStart;

        // MAKE A LIST
        word_p newlist = rplCreateListN(endlvl - stlvl + 1, stlvl, 0);
        if(!newlist || Exceptions) {
            rplBlameError(0);
            return;
        }

        if(halScreen.StkPointer > endlvl) {
            int32_t lstlvl =
                    (halScreen.StkPointer >
                    rplDepthData())? rplDepthData() : halScreen.StkPointer;
            // MAKE ROOM
            memmovew(DSTop - lstlvl + 1, DSTop - lstlvl,
                    (lstlvl - endlvl) * sizeof(word_p) / sizeof(WORD));
            // INSERT THE LIST
            rplOverwriteData(lstlvl, newlist);
            // REMOVE THE ORIGINAL ITEMS
            if(endlvl > stlvl)
                rplRemoveAtData(stlvl, endlvl - stlvl);

            halScreen.StkPointer -= (endlvl - stlvl);
        }
        else if(halScreen.StkPointer < stlvl) {
            int32_t lstlvl;
            if(halScreen.StkPointer > 0) {
                lstlvl = halScreen.StkPointer;
                // MAKE ROOM, USE STACK SLACK TEMPORARILY
                memmovew(DSTop, DSTop - 1,
                        lstlvl * sizeof(word_p) / sizeof(WORD));
                // INSERT THE LIST
                rplOverwriteData(lstlvl, newlist);

                ++DSTop;
            }
            else
                rplPushData(newlist);
            // REMOVE THE ORIGINAL ITEMS
            if(endlvl >= stlvl)
                rplRemoveAtData(stlvl + 1, endlvl - stlvl + 1);

        }
        else {
            // POINTER IS WITHIN THE BLOCK
            rplOverwriteData(endlvl, newlist);
            // REMOVE THE ORIGINAL ITEMS
            if(endlvl > stlvl)
                rplRemoveAtData(stlvl, endlvl - stlvl);

            halScreen.StkPointer = stlvl;

        }

        // AND END THE SELECTION
        halScreen.StkVisibleLvl = -1;
        halScreen.StkSelStatus = 0;

        break;

    }

    }
    halRefresh(STACK_DIRTY);
    return;
}

void KH(tomat)(keyb_msg_t keymsg)
{
    halKeyHelp(keymsg, "TOMAT");
    switch (halScreen.StkSelStatus) {
    case 0:
        // NO ITEM SELECTED, MAKE A ONE-ELEMENT MATRIX
        if((rplDepthData() >= halScreen.StkPointer)
                && (halScreen.StkPointer > 0)) {

            word_p newmat = rplMatrixFlexComposeN(halScreen.StkPointer, 1);    // MAKE A SINGLE ELEMENT VECTOR
            if(!newmat || Exceptions) {
                rplBlameError(0);
                return;
            }
            rplOverwriteData(halScreen.StkPointer, newmat);

        }
        break;

    case 1:
    {
        // MAKE A LIST BETWEEN SELSTART AND STKPOINTER
        int32_t endlvl, stlvl;

        if(halScreen.StkPointer > halScreen.StkSelStart) {
            stlvl = halScreen.StkSelStart;
            endlvl = (halScreen.StkPointer >
                    rplDepthData())? rplDepthData() : halScreen.StkPointer;
        }
        else {
            endlvl = halScreen.StkSelStart;
            stlvl = (halScreen.StkPointer > 0) ? halScreen.StkPointer : 1;
        }

        // MAKE A LIST
        word_p newmat = rplMatrixFlexComposeN(stlvl, endlvl - stlvl + 1);
        if(!newmat || Exceptions) {
            rplBlameError(0);
            return;
        }
        rplOverwriteData(stlvl, newmat);
        if(endlvl - stlvl > 0)
            rplRemoveAtData(stlvl + 1, endlvl - stlvl);
        // AND END THE SELECTION
        halScreen.StkPointer = stlvl;
        halScreen.StkVisibleLvl = -1;
        halScreen.StkSelStatus = 0;

        break;
    }

    case 2:
        // START AND END SELECTED, MOVE THE BLOCK INTO A LIST AT CURSOR
    {
        // MAKE A LIST BETWEEN SELSTART AND SELEND
        int32_t endlvl, stlvl;
        endlvl = halScreen.StkSelEnd;
        stlvl = halScreen.StkSelStart;

        // MAKE A LIST
        word_p newmat = rplMatrixFlexComposeN(stlvl, endlvl - stlvl + 1);
        if(!newmat || Exceptions) {
            rplBlameError(0);
            return;
        }

        if(halScreen.StkPointer > endlvl) {
            int32_t lstlvl =
                    (halScreen.StkPointer >
                    rplDepthData())? rplDepthData() : halScreen.StkPointer;
            // MAKE ROOM
            memmovew(DSTop - lstlvl + 1, DSTop - lstlvl,
                    (lstlvl - endlvl) * sizeof(word_p) / sizeof(WORD));
            // INSERT THE LIST
            rplOverwriteData(lstlvl, newmat);
            // REMOVE THE ORIGINAL ITEMS
            if(endlvl > stlvl)
                rplRemoveAtData(stlvl, endlvl - stlvl);

            halScreen.StkPointer -= (endlvl - stlvl);
        }
        else if(halScreen.StkPointer < stlvl) {
            int32_t lstlvl;
            if(halScreen.StkPointer > 0) {
                lstlvl = halScreen.StkPointer;
                // MAKE ROOM, USE STACK SLACK TEMPORARILY
                memmovew(DSTop, DSTop - 1,
                        lstlvl * sizeof(word_p) / sizeof(WORD));
                // INSERT THE LIST
                rplOverwriteData(lstlvl, newmat);

                ++DSTop;
            }
            else
                rplPushData(newmat);
            // REMOVE THE ORIGINAL ITEMS
            if(endlvl >= stlvl)
                rplRemoveAtData(stlvl + 1, endlvl - stlvl + 1);

        }
        else {
            // POINTER IS WITHIN THE BLOCK
            rplOverwriteData(endlvl, newmat);
            // REMOVE THE ORIGINAL ITEMS
            if(endlvl > stlvl)
                rplRemoveAtData(stlvl, endlvl - stlvl);

            halScreen.StkPointer = stlvl;

        }

        // AND END THE SELECTION
        halScreen.StkVisibleLvl = -1;
        halScreen.StkSelStatus = 0;

        break;

    }

    }
    halRefresh(STACK_DIRTY);
    return;
}

// INTERACTIVE STACK ONLY
void KH(tocplx)(keyb_msg_t keymsg)
{
    halKeyHelp(keymsg, "TOCPLX");

    switch (halScreen.StkSelStatus) {
    case 0:
        // NO ITEM SELECTED, DO NOTHING
        break;

    case 1:
    {
        // MAKE A LIST BETWEEN SELSTART AND STKPOINTER
        int32_t endlvl, stlvl;

        if(halScreen.StkPointer > halScreen.StkSelStart) {
            stlvl = halScreen.StkSelStart;
            endlvl = (halScreen.StkPointer >
                    rplDepthData())? rplDepthData() : halScreen.StkPointer;
        }
        else {
            endlvl = halScreen.StkSelStart;
            stlvl = (halScreen.StkPointer > 0) ? halScreen.StkPointer : 1;
        }

        if(endlvl - stlvl != 1)
            break;      // DO-NOTHING IF MORE THAN 2 ITEMS ARE SELECTED

        word_p real, imag;
        int32_t angmode;
        real = rplPeekData(endlvl);
        imag = rplPeekData(stlvl);
        if(!ISNUMBER(*real)) {
            rplError(ERR_NOTALLOWEDINCOMPLEX);
            rplBlameError(0);
            break;
        }
        angmode = ANGLEMODE(*imag);
        if(!(ISNUMBER(*imag) || ISANGLE(*imag))) {
            rplError(ERR_NOTALLOWEDINCOMPLEX);
            rplBlameError(0);
            break;
        }

        REAL re, im;

        rplReadNumberAsReal(real, &re);
        rplReadNumberAsReal(imag, &im);

        word_p newcplx = rplNewComplex(&re, &im, angmode);

        // MAKE THE COMPLEX NUMBER
        if(!newcplx || Exceptions) {
            rplBlameError(0);
            return;
        }

        rplOverwriteData(stlvl, newcplx);
        if(endlvl - stlvl > 0)
            rplRemoveAtData(stlvl + 1, endlvl - stlvl);
        // AND END THE SELECTION
        halScreen.StkPointer = stlvl;
        halScreen.StkVisibleLvl = -1;
        halScreen.StkSelStatus = 0;

        break;
    }

    case 2:
        // START AND END SELECTED, MOVE THE BLOCK INTO A LIST AT CURSOR
    {
        // MAKE A LIST BETWEEN SELSTART AND SELEND
        int32_t endlvl, stlvl;
        endlvl = halScreen.StkSelEnd;
        stlvl = halScreen.StkSelStart;

        if(endlvl - stlvl != 1)
            break;      // DO-NOTHING IF MORE THAN 2 ITEMS ARE SELECTED

        word_p real, imag;
        int32_t angmode;
        real = rplPeekData(endlvl);
        imag = rplPeekData(stlvl);
        if(!ISNUMBER(*real)) {
            rplError(ERR_NOTALLOWEDINCOMPLEX);
            rplBlameError(0);
            break;
        }
        angmode = ANGLEMODE(*imag);
        if(!(ISNUMBER(*imag) || ISANGLE(*imag))) {
            rplError(ERR_NOTALLOWEDINCOMPLEX);
            rplBlameError(0);
            break;
        }

        REAL re, im;

        rplReadNumberAsReal(real, &re);
        rplReadNumberAsReal(imag, &im);

        word_p newcplx = rplNewComplex(&re, &im, angmode);

        // MAKE THE COMPLEX NUMBER
        if(!newcplx || Exceptions) {
            rplBlameError(0);
            return;
        }

        if(halScreen.StkPointer > endlvl) {
            int32_t lstlvl =
                    (halScreen.StkPointer >
                    rplDepthData())? rplDepthData() : halScreen.StkPointer;
            // MAKE ROOM
            memmovew(DSTop - lstlvl + 1, DSTop - lstlvl,
                    (lstlvl - endlvl) * sizeof(word_p) / sizeof(WORD));
            // INSERT THE LIST
            rplOverwriteData(lstlvl, newcplx);
            // REMOVE THE ORIGINAL ITEMS
            if(endlvl > stlvl)
                rplRemoveAtData(stlvl, endlvl - stlvl);

            halScreen.StkPointer -= (endlvl - stlvl);
        }
        else if(halScreen.StkPointer < stlvl) {
            int32_t lstlvl;
            if(halScreen.StkPointer > 0) {
                lstlvl = halScreen.StkPointer;
                // MAKE ROOM, USE STACK SLACK TEMPORARILY
                memmovew(DSTop, DSTop - 1,
                        lstlvl * sizeof(word_p) / sizeof(WORD));
                // INSERT THE LIST
                rplOverwriteData(lstlvl, newcplx);

                ++DSTop;
            }
            else
                rplPushData(newcplx);
            // REMOVE THE ORIGINAL ITEMS
            if(endlvl >= stlvl)
                rplRemoveAtData(stlvl + 1, endlvl - stlvl + 1);

        }
        else {
            // POINTER IS WITHIN THE BLOCK
            rplOverwriteData(endlvl, newcplx);
            // REMOVE THE ORIGINAL ITEMS
            if(endlvl > stlvl)
                rplRemoveAtData(stlvl, endlvl - stlvl);

            halScreen.StkPointer = stlvl;

        }

        // AND END THE SELECTION
        halScreen.StkVisibleLvl = -1;
        halScreen.StkSelStatus = 0;

        break;

    }

    }
    return;
}

void KH(explode)(keyb_msg_t keymsg)
{
    halKeyHelp(keymsg, "EXPLODE");
    int32_t endlvl, stlvl;

    endlvl = stlvl = -1;

    switch (halScreen.StkSelStatus) {
    case 0:
        // NO ITEM SELECTED, EXPLODE ITEM AT CURSOR
        if((rplDepthData() >= halScreen.StkPointer)
                && (halScreen.StkPointer > 0)) {

            word_p obj = rplPeekData(halScreen.StkPointer);

            if(ISMATRIX(*obj) || ISLIST(*obj) || ISCOMPLEX(*obj))
                stlvl = endlvl = halScreen.StkPointer;
        }
        break;
    case 1:
        if(halScreen.StkPointer > halScreen.StkSelStart) {
            stlvl = halScreen.StkSelStart;
            endlvl = (halScreen.StkPointer >
                    rplDepthData())? rplDepthData() : halScreen.StkPointer;
        }
        else {
            endlvl = halScreen.StkSelStart;
            stlvl = (halScreen.StkPointer > 0) ? halScreen.StkPointer : 1;
        }

        break;
    case 2:
        endlvl = halScreen.StkSelEnd;
        stlvl = halScreen.StkSelStart;
        break;
    }

    if(endlvl < 0)
        return; // NOTHING TO DO

    int32_t c, totalelem = 0;

    for(c = endlvl; c >= stlvl; --c) {

        // EXPLODE ALL SELECTED ITEMS
        word_p obj = rplPeekData(c);
        int32_t nelem;

        if(ISMATRIX(*obj)) {
            nelem = rplMatrixRows(obj);
            if(!nelem)
                nelem = rplMatrixCols(obj);
        }
        else if(ISLIST(*obj)) {
            nelem = rplListLength(obj);
        }
        else if(ISCOMPLEX(*obj))
            nelem = 2;
        else {
            // NOTHING TO EXPLODE
            ++totalelem;
            continue;
        }

        totalelem += nelem;

        rplExpandStack(nelem);
        if(Exceptions) {
            rplBlameError(0);
            return;
        }

        // MAKE ROOM IN THE STACK TO START EXPLODING
        memmovew(DSTop - c + nelem, DSTop - c + 1,
                (c - 1) * sizeof(word_p) / sizeof(WORD));

        // NOW EXPLODE THE MAIN OBJECT IN-PLACE
        word_p *ptr = DSTop - c;

        obj = *ptr;     // READ AGAIN AS THERE MIGHT'VE BEEN A GC DURING EXPANDSTACK

        if(ISMATRIX(*obj)) {
            int32_t rows = rplMatrixRows(obj);
            if(!rows) {
                // EXPAND BY ELEMENTS
                int32_t k;
                for(k = 1; k <= nelem; ++k)
                    *ptr++ = rplMatrixFastGet(obj, 1, k);
            }
            else {
                // TODO: EXPAND BY ROWS - MUCH MORE DIFFICULT THAN LISTS AS IT REQUIRES CREATING NEW MATRIX OBJECTS FOR THE ROWS

                // COMPUTE SIZE OF INDIVIDUAL ROWS
                int32_t cols = rplMatrixCols(obj);
                int32_t totalsize = (2 + cols) * rows;     // ACCOUNT FOR PROLOG+SIZE+OFFSET TABLE OF ALL ROWS

                int32_t i, j, k;

                for(i = 1; i <= rows; ++i) {
                    for(j = 1; j <= cols; ++j) {
                        for(k = 1; k < j; ++k)
                            if(rplMatrixFastGet(obj, i,
                                        j) == rplMatrixFastGet(obj, i, k))
                                break;
                        if(k == j)
                            totalsize += rplObjSize(rplMatrixFastGet(obj, i, j));       // ONLY COUNT NON-REPEATED OBJECTS
                    }
                }

                // HERE WE HAVE TOTAL SIZE OF ALL ROWS TO BE EXPANDED
                // ALLOCATE ONE BIG BLOCK OF MEMORY FOR ALL ROWS

                word_p newrows = rplAllocTempOb(totalsize - 1);
                if(!newrows) {
                    // CLOSE THE STACK BACK BEFORE RETURNING
                    memmovew(DSTop - c + 1, DSTop - c + nelem,
                            (c - 1) * sizeof(word_p) / sizeof(WORD));
                    rplBlameError(0);
                    return;
                }

                obj = *ptr;     // READ AGAIN AS THERE MIGHT'VE BEEN A GC DURING ALLOCATION

                word_p rptr = newrows, objptr;

                for(i = 1; i <= rows; ++i) {
                    rptr[1] = MATMKSIZE(0, cols);
                    objptr = rptr + 2 + cols;   // POINT TO THE LOCATION OF THE NEXT OBJECT TO BE STORED
                    for(j = 1; j <= cols; ++j) {
                        for(k = 1; k < j; ++k)
                            if(rplMatrixFastGet(obj, i,
                                        j) == rplMatrixFastGet(obj, i, k))
                                break;
                        if(k == j) {
                            rplCopyObject(objptr, rplMatrixFastGet(obj, i, j)); // ADD A NEW OBJECT
                            rptr[1 + j] = objptr - rptr;
                            objptr = rplSkipOb(objptr);
                        }
                        else
                            rptr[1 + j] = rptr[1 + k];  // REUSE THE OBJECT
                    }
                    // DONE WITH THE ROW, FIX THE SIZE
                    rptr[0] = MK_PROLOG(DOMATRIX, objptr - rptr - 1);
                    rptr = objptr;
                }

                // NOW EXPAND IT ON THE STACK LIKE A LIST

                rptr = newrows;
                for(i = 0; i < rows; ++i) {
                    *ptr++ = rptr;
                    rptr = rplSkipOb(rptr);
                }

            }

        }
        else if(ISLIST(*obj)) {
            int32_t k;
            word_p item = obj + 1;
            for(k = 0; k < nelem; ++k) {
                *ptr++ = item;
                item = rplSkipOb(item);
            }
        }
        else if(ISCOMPLEX(*obj)) {
            *ptr++ = obj + 1;
            *ptr++ = rplSkipOb(obj + 1);
        }

        DSTop += nelem - 1;
        if(halScreen.StkPointer > c)
            halScreen.StkPointer += nelem - 1;
        endlvl += nelem - 1;
    }

    // DONE EXPLODING, ADJUST POINTERS TO SELECT EVERYTHING
    halScreen.StkSelStart = stlvl;
    halScreen.StkSelEnd = endlvl;

    // SPECIAL CASE: WHEN BLOCK IS SELECTED AND POINTER IS OUTSIDE THE BLOCK, MOVE EXPLODED ITEMS TO THE NEW LOCATION
    if(halScreen.StkSelStatus == 2) {
        if(halScreen.StkPointer > halScreen.StkSelEnd) {
            word_p *stptr, *endptr, *cptr;
            word_p item;
            stptr = DSTop - halScreen.StkSelStart;
            endptr = DSTop - ((halScreen.StkPointer >
                        rplDepthData())? rplDepthData() : halScreen.StkPointer);

            // DO UNROT UNTIL THE ENTIRE BLOCK MOVED
            int32_t count = halScreen.StkSelEnd - halScreen.StkSelStart + 1;

            while(count--) {
                cptr = stptr;

                item = *cptr;

                while(cptr > endptr) {
                    cptr[0] = cptr[-1];
                    --cptr;
                }
                *cptr = item;
            }

            count = halScreen.StkSelEnd - halScreen.StkSelStart;
            halScreen.StkSelEnd =
                    ((halScreen.StkPointer >
                        rplDepthData())? rplDepthData() : halScreen.StkPointer);
            halScreen.StkSelStart = halScreen.StkSelEnd - count;

        }
        else if(halScreen.StkPointer < halScreen.StkSelStart) {

            word_p *stptr, *endptr, *cptr;
            word_p item;
            stptr = DSTop - halScreen.StkSelEnd;
            endptr = DSTop - halScreen.StkPointer - 1;

            // DO ROT UNTIL THE ENTIRE BLOCK MOVED
            int32_t count = halScreen.StkSelEnd - halScreen.StkSelStart + 1;
            while(count--) {
                cptr = stptr;

                item = *cptr;

                while(cptr < endptr) {
                    cptr[0] = cptr[1];
                    ++cptr;
                }
                *cptr = item;
            }

            count = halScreen.StkSelEnd - halScreen.StkSelStart;
            halScreen.StkSelStart = halScreen.StkPointer + 1;
            halScreen.StkSelEnd = halScreen.StkPointer + 1 + count;
            halScreen.StkPointer += count + 1;
            halScreen.StkVisibleLvl = -1;

        }
    }

    if(stlvl == endlvl)
        halScreen.StkSelStatus = 0;
    else
        halScreen.StkSelStatus = 2;
    halScreen.StkPointer = halScreen.StkSelEnd;
    halScreen.StkVisibleLvl = -1;
    halRefresh(STACK_DIRTY);
    return;
}

void KH(powerOff)(keyb_msg_t keymsg)
{
    halKeyHelp(keymsg, "Power off");

    // SHIFT-ON MEANS POWER OFF!
    halPreparePowerOff();
    halEnterPowerOff();
    return;
}

void KH(cancel)(keyb_msg_t keymsg)
{
    halKeyHelp(keymsg, "Cancel");

    if((halContext(CONTEXT_EDITOR))) {
        // END THE COMMAND LINE
        endCmdLine();
    }

    if ((halContext(CONTEXT_INTERACTIVE_STACK)))
    {
        // End interactive stack
        halExitContext(CONTEXT_INTERACTIVE_STACK);
        halSetContext(CONTEXT_STACK);
        halScreen.StkVisibleLvl    = 1;
        halScreen.StkVisibleOffset = 0;
        halScreen.StkSelStart      = 0;
        halScreen.StkSelEnd        = 0;
        halScreen.StkSelStatus     = 0;
        halRefresh(STACK_DIRTY);
    }
}


// **************************************************************************
// ******************* END OF DEFAULT KEY HANDLERS **************************
// **************************************************************************


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

int halDoCustomKey(keyb_msg_t keymsg)
{
    if(rplTestSystemFlag(FL_NOCUSTOMKEYS))
        return 0;       // DON'T USE CUSTOM KEYS IF DISABLED PER FLAG

    // TODO: READ THE KEYBOARD TABLE FROM THE Settings DIRECTORY AND DO IT
    word_p keytable;

    keytable = rplGetSettings((word_p) customkey_ident);

    if(!keytable)
        return 0;       // NO CUSTOM KEY DEFINED

    if(!ISLIST(*keytable))
        return 0;       // INVALID KEY DEFINITION

    word_p             ptr        = keytable + 1;
    word_p             endoftable = rplSkipOb(keytable);
    word_p             action     = 0;
    keyboard_context_t ctx;
    int                keepgoing, hanoffset;
    WORD               msg;

    // CLEAR THE DEFAULT KEY FLAG, ANY OF THE CUSTOM HANDLERS CAN SET THIS FLAG TO HAVE THE DEFAULT KEY HANDLER EXECUTED
    rplClrSystemFlag(FL_DODEFAULTKEY);

    do {
        keepgoing = 0;
        while(ptr < endoftable) {
            msg = rplReadNumberAsInt64(ptr);
            if(Exceptions) {
                // CLEAR ALL ERRORS AND KEEP GOING
                rplClearErrors();
                return 0;
            }
            ptr = rplSkipOb(ptr);
            if(ptr >= endoftable)
                return 0;
            ctx = rplReadNumberAsInt64(ptr);
            if(Exceptions) {
                // CLEAR ALL ERRORS AND KEEP GOING
                rplClearErrors();
                return 0;
            }
            ptr = rplSkipOb(ptr);
            if(ptr >= endoftable)
                return 0;

            if (msg == keymsg)
            {
                if (ctx == CONTEXT_ANY)
                {
                    action = ptr;
                    break;
                }
                if (!(ctx & CONTEXT_SYSTEM_MASK))
                {
                    if (ctx == halContext(CONTEXT_SYSTEM_MASK))
                    {
                        action = ptr;
                        break;
                    }
                }
                else if (ctx == halScreen.KeyContext)
                {
                    action = ptr;
                    break;
                }
            }
            ptr = rplSkipOb(ptr);
        }

        if(action) {
            // EXECUTE THE REQUESTED ACTION
            // CLEAR THE NEXT HANDLER FLAGS, THE KEY HANDLER CAN SET THE FLAG TO CHAIN THE PREVIOUS HANDLER

            // REMEMBER WHICH HANDLER WAS EXECUTED

            hanoffset = action - keytable;

            rplClrSystemFlag(FL_DONEXTCUSTKEY);
            customKeyHandler(keymsg, action);

            if(rplTestSystemFlag(FL_DONEXTCUSTKEY) > 0) {

                // RESTORE ALL POINTERS, SINCE EXECUTION COULD'VE CHANGED EVERYTHING

                keytable = rplGetSettings((word_p) customkey_ident);

                if(!keytable)
                    return 1;   // NO MORE KEYS, KEYTABLE VANISHED?

                if(!ISLIST(*keytable))
                    return 1;   // INVALID KEY DEFINITION

                endoftable = rplSkipOb(keytable);
                action = 0;
                ptr = keytable + 1;
                while(ptr - keytable <= hanoffset) {
                    ptr = rplSkipOb(ptr);
                    if(ptr >= endoftable)
                        break;
                    ptr = rplSkipOb(ptr);
                    if(ptr >= endoftable)
                        break;
                    ptr = rplSkipOb(ptr);
                    if(ptr >= endoftable)
                        break;
                }

                // FOUND THE NEXT HANDLER TO SCAN OR THE END OF LIST

                keepgoing = 1;

            }
            else {
                if(rplTestSystemFlag(FL_DODEFAULTKEY) > 0)
                    halDoDefaultKey(keymsg);
                return 1;
            }

        }

    }
    while(keepgoing);

    return 0;
}

// RETURN TRUE/FALSE IF A CUSTOM HANDLER EXISTS
int halCustomKeyExists(keyb_msg_t keymsg)
{
    word_p keytable;

    keytable = rplGetSettings((word_p) customkey_ident);

    if(!keytable)
        return 0;       // NO CUSTOM KEY DEFINED

    if(!ISLIST(*keytable))
        return 0;       // INVALID KEY DEFINITION

    word_p             ptr = keytable + 1, endoftable = rplSkipOb(keytable);
    keyboard_context_t ctx;
    WORD               msg;

    while(ptr < endoftable) {
        msg = rplReadNumberAsInt64(ptr);
        if(Exceptions) {
            // CLEAR ALL ERRORS AND KEEP GOING
            rplClearErrors();
            return 0;
        }
        ptr = rplSkipOb(ptr);
        if(ptr >= endoftable)
            return 0;
        ctx = rplReadNumberAsInt64(ptr);
        if(Exceptions) {
            // CLEAR ALL ERRORS AND KEEP GOING
            rplClearErrors();
            return 0;
        }
        if (msg == keymsg)
        {
            if (ctx == 0)
                return 1;
            if (!(ctx & CONTEXT_SYSTEM_MASK))
            {
                if (ctx == halContext(CONTEXT_SYSTEM_MASK))
                    return 1;
            }
            else
            {
                if (ctx == halScreen.KeyContext)
                    return 1;
            }
        }
        ptr = rplSkipOb(ptr);
        if (ptr >= endoftable)
            return 0;
        ptr = rplSkipOb(ptr);
    }

    //   SCANNED ENTIRE TABLE, NO LUCK
    return 0;
}


typedef void (*key_handler_fn)(keyb_msg_t keymsg);

static const key_handler_fn keymap_unshifted[64];
static const key_handler_fn keymap_left_shift[64];
static const key_handler_fn keymap_right_shift[64];
static const key_handler_fn keymap_alpha[64];
static const key_handler_fn keymap_left_hold[64];
static const key_handler_fn keymap_right_hold[64];
static const key_handler_fn keymap_alpha_hold[64];
static const key_handler_fn keymap_on_hold[64];

#include "keymap.c"         // Platform-dependent

static inline key_handler_fn halKeymap(keyb_msg_t           keymsg,
                                       unsigned             flag,
                                       const key_handler_fn keymap[])
// ----------------------------------------------------------------------------
//   Check if we find an entry in the given keymap
// ----------------------------------------------------------------------------
{
    if (keymsg & flag)
    {
        unsigned key = KM_KEY(keymsg);
        key_handler_fn function = keymap[key];
        if (function)
            return function;
    }
    return NULL;
}


static inline key_handler_fn halDefaultKeyHandler(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
// Return the key handler if it exists
// ----------------------------------------------------------------------------
{
    key_handler_fn fn = NULL;
    if (!fn)
        fn = halKeymap(keymsg, KHOLD_ON,        keymap_on_hold);
    if (!fn)
        fn = halKeymap(keymsg, KHOLD_ALPHA,     keymap_alpha_hold);
    if (!fn)
        fn = halKeymap(keymsg, KSHIFT_ALPHA,    keymap_alpha);
    if (!fn)
        fn = halKeymap(keymsg, KHOLD_LEFT,      keymap_left_hold);
    if (!fn)
        fn = halKeymap(keymsg, KHOLD_RIGHT,     keymap_right_hold);
    if (!fn)
        fn = halKeymap(keymsg, KSHIFT_LEFT,     keymap_left_shift);
    if (!fn)
        fn = halKeymap(keymsg, KSHIFT_RIGHT,    keymap_right_shift);
    if (!fn)
        fn = halKeymap(keymsg, ~0U,             keymap_unshifted);
    return fn;
}


int halDoDefaultKey(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   Find the operation to execute depending on current keyboard mode
// ----------------------------------------------------------------------------
{
    record(keys, "halDoDefaultKey %08X key %d msg %x shift %x ",
           keymsg, KM_KEY(keymsg), KM_MESSAGE(keymsg), KM_SHIFT(keymsg));

    key_handler_fn handler = halDefaultKeyHandler(keymsg);
    if (handler)
        handler(keymsg);
    return handler != NULL;
}


void halNavigateHelp(keyb_msg_t keymsg)
// ----------------------------------------------------------------------------
//   When help is active, navigate with arrows, exit with ON/EXIT/ENTER/SPC
// ----------------------------------------------------------------------------
{
    // Displaying the on-line help clears the last error message
    halScreen.ErrorMessage = NULL;

    if (KM_MESSAGE(keymsg) == KM_PRESS)
    {
        keyb_msg_t key = KM_KEY(keymsg);
        if (key == KB_ON || key == KB_ESC || key == KB_ENT || key == KB_SPC)
            halScreen.HelpMessage = NULL;
        else if (key == KB_UP && halScreen.HelpLine > 0)
            halScreen.HelpLine--;
        else if (key == KB_DN && halScreen.HelpLine < halScreen.HelpMaxLine)
            halScreen.HelpLine++;
        halRefresh(ERROR_DIRTY);
    }
}


int halProcessKey(keyb_msg_t keymsg, int (*dokey)(WORD), int32_t flags)
// ----------------------------------------------------------------------------
//   Process key messages and call appropriate handlers for keycode
// ----------------------------------------------------------------------------
//  Returns 0 if the loop has to continue, 1 to terminate outer loop
{
    if (!keymsg)
    {
        // Check if there are pending shift changes - If so, update display
        if ((keyb_flags & KFLAG_SHIFTS_CHANGED) == 0)
            return 0;
        keymsg = (keyb_flags & (KSHIFT_ANY|KHOLD_ANY)) | KFLAG_SHIFTS_CHANGED;
    }

    record(keys,
           "halProcessKey %08x (%d) flags=%08X, handler=%p",
           keymsg,
           KM_KEY(keymsg),
           flags,
           dokey);

#ifdef TARGET_PRIME
    if (KM_MESSAGE(keymsg) == KM_TOUCH)
    {
        // TODO: Convert touch events into keys or gestures
        word_p keyname = rplMsg2KeyName(keymsg);
        if (keyname)
            rplPushData(keyname);
        else
            rplPushFalse();
    }
#endif /* TARGET_PRIME */

    int processed = 0;
    if (keymsg & KFLAG_SHIFTS_CHANGED)
    {
        halScreenUpdated();
        halRefresh(STACK_DIRTY);

        // There was a change in shift plane, update annunciators
        halSetNotification(N_LEFT_SHIFT,
                           (keymsg & KHOLD_LEFT)  ? 0x8 :
                           (keymsg & KSHIFT_LEFT) ? 0xF :
                           0);
        halSetNotification(N_RIGHT_SHIFT,
                           (keymsg & KHOLD_RIGHT)  ? 0x8 :
                           (keymsg & KSHIFT_RIGHT) ? 0xF :
                           0);
        halSetNotification(N_ALPHA,
                           (keymsg & KHOLD_ALPHA)  ? 0x8 :
                           (keymsg & KSHIFT_ALPHA) ? 0xF :
                           0);

        // Update editor mode accordingly
        if (keymsg & KSHIFT_ALPHA)
        {
            // There was a change in alpha mode
            halSwapCmdLineMode(keymsg & KSHIFT_ALPHA);
        }

        // Mark that we got the shift change
        keyb_flags &= ~KFLAG_SHIFTS_CHANGED;

        return 0;
    }

    // If we are displaying help, wait until we exit
    if (halScreen.HelpMessage)
    {
        halNavigateHelp(keymsg);
        return 0;
    }

    // Check if a handler takes care of the event
    processed =
        (dokey                           && dokey(keymsg))               ||
        ((flags & OL_NOCUSTOMKEYS ) == 0 && halDoCustomKey(keymsg))      ||
        ((flags & OL_NODEFAULTKEYS) == 0 && halDoDefaultKey(keymsg));

    if (processed)
    {
        if (Exceptions)
        {
            halRefresh(ERROR_DIRTY);
        }
        else if (halScreen.ErrorMessage && KM_MESSAGE(keymsg) == KM_PRESS)
        {
            halScreen.ErrorMessage = NULL;
            halRefresh(ERROR_DIRTY);
        }
    }

    if (RECORDER_TWEAK(keys_debug))
    {
        // *************** DEBUG ONLY ************
        if (!processed && ((KM_MESSAGE(keymsg) == KM_PRESS) ||
                           (KM_MESSAGE(keymsg) == KM_LONG_PRESS) ||
                           (KM_MESSAGE(keymsg) == KM_REPEAT)))
        {
            // All other keys, just display the key name on screen
            gglsurface scr;
            ggl_init_screen(&scr);
            const UNIFONT *fnt         = FONT_STATUS;
            char           keyName[16] = { 0 };
            snprintf(keyName, sizeof(keyName), "%X %u", keymsg, KM_KEY(keymsg));

            int width = StringWidth(keyName, fnt);
            int ytop  = halScreen.Form + halScreen.Stack + halScreen.CmdLine +
                       halScreen.Menu1;

            ggl_rect(&scr,
                     STATUS_AREA_X,
                     ytop,
                     LCD_W - 1,
                     ytop + halScreen.Menu2 - 1,
                     PAL_STA_TEXT);
            DrawTextBk(&scr,
                       LCD_W - width,
                       ytop + halScreen.Menu2 / 2,
                       keyName,
                       fnt,
                       PAL_STA_TEXT,
                       PAL_STA_BG);
            char *shiftstr;
            switch (KM_SHIFT(keymsg))
            {
            case KSHIFT_LEFT: shiftstr = "(LS)"; break;
            case KSHIFT_LEFT | KHOLD_LEFT: shiftstr = "(LSH)"; break;
            case KSHIFT_RIGHT: shiftstr = "(RS)"; break;
            case KSHIFT_RIGHT | KHOLD_RIGHT: shiftstr = "(RSH)"; break;
            case KSHIFT_ALPHA: shiftstr = "(AL)"; break;
            case KSHIFT_ALPHA | KHOLD_ALPHA: shiftstr = "(ALH)"; break;
            case KHOLD_ON: shiftstr = "(ONH)"; break;
            case KSHIFT_ALPHA | KSHIFT_LEFT: shiftstr = "(AL-LS)"; break;
            case KSHIFT_ALPHA | KSHIFT_RIGHT: shiftstr = "(AL-RS)"; break;
            case KSHIFT_ALPHA | KHOLD_LEFT: shiftstr = "(AL-LSH)"; break;
            case KSHIFT_ALPHA | KHOLD_RIGHT: shiftstr = "(AL-RSH)"; break;

            default: shiftstr = "";
            }
            DrawTextBk(&scr,
                       LCD_W - width - 32,
                       ytop + halScreen.Menu2 / 2,
                       shiftstr,
                       fnt,
                       PAL_STA_TEXT,
                       PAL_STA_BG);

            if (KM_MESSAGE(keymsg) == KM_LONG_PRESS)
                DrawTextBk(&scr,
                           LCD_W - width - 42,
                           ytop + halScreen.Menu2 / 2,
                           "L=",
                           fnt,
                           PAL_STA_TEXT,
                           PAL_STA_BG);
        }
    }

    if (processed < 0)
        return 1;
    else
        return 0;
}


// SET A PROCESS TO BE EXECUTED AS SOON AS THERE'S NO MORE KEY PRESSES
void halDeferProcess(void (*function)(void))
{
    int k;
    for(k = 0; k < 3; ++k) {
        if(halProcesses[k] == 0) {
            halProcesses[k] = function;
            break;
        }
    }
}

// PERFORM ALL DEFERRED PROCESSES
void halDoDeferredProcess()
{
    int k;
    for(k = 0; k < 3; ++k) {
        if(halProcesses[k] != 0) {
            void (*func)() = halProcesses[k];
            halProcesses[k] = 0;
            (*func) ();
        }
    }

}

void halOuterLoop(int32_t timeoutms,
                  int (*dokey)(WORD),
                  int (*doidle)(WORD),
                  int32_t flags)
// ----------------------------------------------------------------------------
//    Main handler for an interactive context, e.g. form or command line
// ----------------------------------------------------------------------------
// This function returns when the form closes, or the user exits with ON/EXIT
{
    keyb_msg_t keymsg     = 0;
    int        isidle     = 0;
    int        jobdone    = 0;
    int64_t    offcounter = 0;

    gglsurface scr;
    ggl_init_screen(&scr);
    halTimeoutEvent  = -1;

    do
    {
        if (!(halFlags & (HAL_RESET | HAL_HWRESET)))
            halRedrawAll(&scr);
        if (!(flags & OL_NOEXIT) && halExitOuterLoop())
            break;
        if (halFlags & HAL_POWEROFF)
        {
            halFlags &= ~HAL_POWEROFF;
#ifndef CONFIG_NO_FSYSTEM
            if (FSIsInit())
            {
                if (FSCardInserted())
                    FSShutdown();
                else
                    FSShutdownNoCard();
            }
#endif
            if (!(halFlags & (HAL_RESET | HAL_HWRESET)))
            {
                halPreparePowerOff();
                halEnterPowerOff();
            }
            else
            {
                if (!(halFlags & HAL_HWRESET))
                {
                    // halFlags=0;   // DURING HAL RESET, DON'T PRESERVE THE
                    // FLAGS halPreparePowerOff(); // DON'T DO A POWEROFF
                    // PROCEDURE
                    halFlags = HAL_RESET;
                }
                else
                    halReset(); // THIS FUNCTION DOES NOT RETURN
            }
            return;
        }

        if (halFlags & HAL_FASTAUTORESUME)
        {
            halSetBusyHandler();
            jobdone = isidle = 0;
            halFlags &= ~HAL_FASTAUTORESUME;
            uiCmdRun(CMD_CONT); // AUTOMATICALLY CONTINUE EXECUTION BEFORE
                                // PROCESSING ANY KEYS
            halRefresh(ALL_DIRTY);
            continue;
        }

        if (Exceptions)
            if (flags & OL_EXITONERROR)
                break;

        keymsg    = halWaitForKeyTimeout(timeoutms);
        timeoutms = 0;

        if (keymsg == HAL_KEY_TIMEOUT)
        {
            // TIMED OUT!
            if (halTimeoutEvent >= 0)
                tmr_event_kill(halTimeoutEvent);
            halTimeoutEvent = -1;
            halFlags &= ~HAL_TIMEOUT;
            break; // JUST EXIT THE POL
        }

        if (keymsg == HAL_KEY_WAKEUP)
        {
            // Something other than a key woke up the cpu
            halDoDeferredProcess();

            if (usb_isconfigured())
            {
                halSetNotification(N_CONNECTION, 15);
                if (usb_hasdata())
                    halSetNotification(N_DATARECVD, 15);
                else
                    halSetNotification(N_DATARECVD, 0);
            }
            else
                halSetNotification(N_CONNECTION, 0);

            if (!(flags & OL_NOCOMMS))
            {
                if (usb_hasdata())
                {
                    if (!rplTestSystemFlag(FL_NOAUTORECV))
                    {
                        uiCmdRun(CMD_USBAUTORCV);
                        halRefresh(ALL_DIRTY);
                        continue;
                    }
                }
            }

            if (!isidle)
                offcounter = halTicks();

#ifndef CONFIG_NO_FSYSTEM
            // FLUSH FILE SYSTEM CACHES WHEN IDLING FOR MORE THAN 3 SECONDS
            if (!(flags & OL_NOSDFLUSH) && !(jobdone & 1) && FSIsInit())
            {
                if (halTicks() - offcounter >= 3000000)
                {
                    if (FSIsDirty())
                    {
                        FSFlushAll();
                        halUpdateStatus();
                    }
                    jobdone |= 1;
                    isidle = 0;
                }
            }
#endif

            // AUTO-OFF WHEN IDLING
            if (!(flags & OL_NOAUTOOFF) && (halFlags & HAL_AUTOOFFTIME) &&
                (!usb_isconnected()))
            {
                int64_t autoofftime = 15000000 << (GET_AUTOOFFTIME(halFlags));
                if (halTicks() - offcounter >= autoofftime)
                {
                    halPreparePowerOff();
                    halEnterPowerOff();
                }
            }

            if (!(flags & OL_NOALARM))
            {
                if (halCheckSystemAlarm())
                {
                    jobdone = isidle = 0;
                    halTriggerAlarm();
                }
            }

            // DO OTHER IDLE PROCESSING HERE

            if (halFlags & HAL_AUTORESUME)
            {
                halSetBusyHandler();
                jobdone = isidle = 0;
                uiCmdRun(CMD_CONT); // AUTOMATICALLY CONTINUE EXECUTION AFTER 10
                                    // IDLE CYCLES
                halRefresh(ALL_DIRTY);
                continue;
            }

            if (doidle)
            {
                if ((*doidle)(0))
                    break;
            }

            isidle = 1;
        }
        else
        {
            jobdone = isidle = 0;
        }

        halSetBusyHandler();

    } while (!halProcessKey(keymsg, dokey, flags));

    // MAKE SURE WE CLEANUP THE EVENT TIMER BEFORE WE EXIT SO IT DOESN'T TRIGGER
    // INSIDE A PARENT POL
    if (halTimeoutEvent >= 0)
        tmr_event_kill(halTimeoutEvent);
    halTimeoutEvent = -1;
    halFlags &= ~HAL_TIMEOUT;
}

void halInitKeyboard()
{
    keyb_set_alpha_once(1);
    keyb_set_shift_plane(KSHIFT_NONE);
#ifdef TARGET_PRIME
    halKeyMenuSwitch=0;
#endif /* TARGET_PRIME */
}

// API USED BY RPL PROGRAMS TO INSERT KEY SEQUENCES TO THE KEYBOARD

void halPostKeyboardMessage(keyb_msg_t keymsg)
{
    //  POST A COMPLETE KEY SEQUENCE TO PREVENT PROBLEMS.

    switch (KM_MESSAGE(keymsg)) {
    case KM_PRESS:
    {
        keyb_post_message(KM_KEYDN | (keymsg ^ (KM_MESSAGE(keymsg))));
        keyb_post_message(keymsg);
        keyb_post_message(KM_KEYUP | (keymsg ^ (KM_MESSAGE(keymsg))));
        break;

    }
    case KM_LONG_PRESS:
    {
        keyb_post_message(KM_KEYDN | (keymsg ^ (KM_MESSAGE(keymsg))));
        keyb_post_message(KM_PRESS | (keymsg ^ (KM_MESSAGE(keymsg))));
        keyb_post_message(keymsg);
        keyb_post_message(KM_KEYUP | (keymsg ^ (KM_MESSAGE(keymsg))));
        break;
    }
    default:
        keyb_post_message(keymsg);
    }

}

#ifdef TARGET_PRIME
void halTouch2KeyMsg()
{
    // TODO: Check for touch events and simulate key presses
}
#endif /* TARGET_PRIME */
