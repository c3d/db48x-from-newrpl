/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef SYSVARS_H
#define SYSVARS_H

#include "decimal.h"
#include "libraries.h"
#include "newrpl_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

// EXTERNAL RPL MACHINE REGISTERS

// GLOBAL ARRAY OF POINTERS THAT ARE TO BE UPDATED BY THE GC
extern word_p GC_PTRUpdate[MAX_GC_PTRUPDATE];

#define IPtr                GC_PTRUpdate[0]
#define TempObEnd           GC_PTRUpdate[1]     // END OF KNOWN AND STABLE TEMPOB BLOCKS
#define TempObSize          GC_PTRUpdate[2]
#define CompileEnd          GC_PTRUpdate[3]     // ADDITIONAL DATA AT THE END OF TEMPOB DURING COMPILE
#define ExceptionPointer    GC_PTRUpdate[4]

#define ArgPtr1             GC_PTRUpdate[5]     // NAME FOR USE BY THE LIBRARIES
#define DecompileObject     GC_PTRUpdate[5]     // NAME FOR USE BY THE DECOMPILER
#define TokenStart          GC_PTRUpdate[5]     // NAME AS USED BY THE COMPILER
#define LibraryList         GC_PTRUpdate[5]     // NAME AS USED BY THE LIBRARY INSTALLATION ROUTINES
#define ObjectPTR           GC_PTRUpdate[5]     // NAME AS USED BY THE ROM ID CONVERSION OPCODES

#define ArgPtr2             GC_PTRUpdate[6]     // NAME FOR USE BY LIBRARIES
#define BlankStart          GC_PTRUpdate[6]     // NAME AS USED BY THE COMPILER
#define DecompStringEnd     GC_PTRUpdate[6]     // NAME AS USED BY THE DECOMPILER

#define EndOfObject         GC_PTRUpdate[7]     // NAME AS USED BY THE DECOMPILER
#define NextTokenStart      GC_PTRUpdate[7]     // NAME AS USED BY THE COMPILER

#define CompileStringEnd    GC_PTRUpdate[8]
#define SuggestedObject     GC_PTRUpdate[8]     // NAME AS USED BY THE AUTOCOMPLETE FEATURE

#define ErrorHandler        GC_PTRUpdate[9]
#define SettingsDir         GC_PTRUpdate[10]
#define SystemFlags         GC_PTRUpdate[11]
#define LastCompiledObject  GC_PTRUpdate[12]
#define SavedDecompObject   GC_PTRUpdate[12]
#define CmdLineText         GC_PTRUpdate[13]
#define CmdLineCurrentLine  GC_PTRUpdate[14]
#define CmdLineUndoList     GC_PTRUpdate[15]
#define HaltedIPtr          GC_PTRUpdate[16]
#define BlameCmd            GC_PTRUpdate[17]
#define ScratchPointer1     GC_PTRUpdate[18]
#define ScratchPointer2     GC_PTRUpdate[19]
#define ScratchPointer3     GC_PTRUpdate[20]
#define ScratchPointer4     GC_PTRUpdate[21]
#define ScratchPointer5     GC_PTRUpdate[22]
#define BreakPt1Pointer     GC_PTRUpdate[23]    // CODE BREAKPOINT LOCATION
#define BreakPt1Arg         GC_PTRUpdate[24]    // CODE BREAKPOINT ARGUMENT
#define BreakPt2Pointer     GC_PTRUpdate[25]    // CODE BREAKPOINT LOCATION
#define BreakPt2Arg         GC_PTRUpdate[26]    // CODE BREAKPOINT ARGUMENT
#define BreakPt3Pointer     GC_PTRUpdate[27]    // CODE BREAKPOINT LOCATION
#define BreakPt3Arg         GC_PTRUpdate[28]    // CODE BREAKPOINT ARGUMENT

#define LastRegisterT       GC_PTRUpdate[29]    // SAVED T REGISTER FOR RPN MODE
#define GC_UserRegisters     (GC_PTRUpdate+30)  // 30 TO 37 ARE 8 REGISTERS EXPOSED TO FINAL USER THROUGH ASSEMBLY

// THE POINTERS [12] TO [15] ARE AVAILABLE FOR LIBRARIES TO USE

// MEMORY REGIONS. EACH MEMORY REGION CAN GROW INDEPENDENTLY
// GROWTH MAY TRIGGER A GC IF INSUFFICIENT MEMORY.

extern word_p *RStk;  // BASE OF RETURN STACK
extern word_p *DStk;  // BASE OF DATA STACK
extern word_p *DStkBottom;    // BASE OF CURRENT DATA STACK
extern word_p *DStkProtect;   // BASE OF PROTECTED DATA STACK
extern word_p TempOb; // TEMPORARY OBJECT STORAGE
extern word_p *TempBlocks;    // TEMPOB BLOCK POINTERS STORAGE
extern word_p *Directories;   // BASE OF DIRECTORY STORAGE
extern word_p *LAMs;  // BASE OF LOCAL VARIABLES STORAGE

// OTHER VARIABLES THAT ARE NOT AFFECTED BY GC
extern WORD CurOpcode; // CURRENT OPCODE (WORD)
extern WORD HWExceptions, Exceptions, TrappedExceptions;       // FLAGS FOR CURRENT EXCEPTIONS
extern WORD BreakPtFlags;      // FLAGS FOR HARDWARE BREAK POINTS
extern WORD ErrorCode, TrappedErrorCode;
extern WORD GCFlags;   // INTERNAL REGISTER TO INDICATE SPECIAL CONDITIONS, LIKE A GARBAGE COLLECTION HAPPENED
extern word_p *RSTop; // TOP OF THE RETURN STACK
extern word_p *HaltedRSTop;   // TOP OF THE RETURN STACK OF HALTED PROGRAM
extern word_p *DSTop; // TOP OF THE DATA STACK
extern word_p *ValidateBottom;        // TEMPORARY DATA AFTER OR IN THE RETURN STACK USED DURING COMPILATION
extern word_p *ValidateTop;   // TEMPORARY DATA AFTER OR IN THE RETURN STACK USED DURING COMPILATION
extern word_p *TempBlocksEnd; // POINTER TO END OF TEMPBLOCKS
extern word_p *LAMTop;        // TOP OF THE LAM STACK
extern word_p *nLAMBase;      // START OF THE LAST LAM ENVIRONMENT
extern word_p *HaltedLAMTop;  // TOP OF THE LAM STACK
extern word_p *HaltednLAMBase;        // START OF THE LAST LAM ENVIRONMENT
extern word_p *LAMTopSaved;   // SAVED VALUE OF LAMTOP USED DURING COMPILATION
extern word_p *ErrornLAMBase; // SAVED BASE OF LAM ENVIRONMENT AT ERROR HANDLER
extern word_p *ErrorLAMTop;   // SAVED VALUE OF LAMTOP AT ERROR HANDLER
extern word_p *ErrorRSTop;    // SAVED TOP OF RETURN STACK AT ERROR HANDLER
extern word_p *CurrentDir;    // POINTER TO CURRENT DIRECTORY
extern word_p *DirsTop;       // POINTER TO END OF USED DIRECTORIES
extern int32_t DStkSize;  // TOTAL SIZE OF DATA STACK
extern int32_t RStkSize;  // TOTAL SIZE OF RETURN STACK
extern int32_t TempBlocksSize;    // TOTAL SIZE OF TEMPBLOCKS
extern int32_t LAMSize;   // TOTAL SIZE OF LAM ENVIRONMENTS
extern int32_t DirSize;

// ARGUMENTS TO PASS TO LIBRARY HANDLERS
// DURING COMPILATION
extern uint32_t ArgNum1, ArgNum2, ArgNum3, RetNum;

// SOME CONVENIENCE NAMES FOR USE IN LIBRARIES
#define TokenLen ArgNum1
#define DecompMode ArgNum1

#define BlankLen ArgNum2
#define DecompHints ArgNum2
#define ObjectIDHash ArgNum2

#define CurrentConstruct ArgNum3
#define SuggestedOpcode  ArgNum3
#define ObjectID   ArgNum3
#define MenuCodeArg ArgNum3
#define CmdHelp     ArgNum3
#define LibError    ArgNum3
#define TypeInfo    ArgNum3

extern LIBHANDLER LowLibRegistry[MAXLOWLIBS];
extern LIBHANDLER SysHiLibRegistry[MAXSYSHILIBS];
extern LIBHANDLER HiLibRegistry[MAXHILIBS];
extern int32_t HiLibNumbers[MAXHILIBS];
extern int32_t NumHiLibs;

// MATH LIBRARY CONTEXT
extern DECIMAL_CONTEXT Context;

// PREALLOCATED STATIC REAL NUMBER REGISTERS FOR TEMPORARY STORAGE
extern REAL RReg[REAL_REGISTERS];
// TEMPORARY SCRATCH MEMORY FOR DIGITS
extern int32_t RDigits[EXTRA_STORAGE];

extern int32_t int32_t2RealIdx;

#ifdef __cplusplus
}
#endif

#endif // SYSVARS_H
