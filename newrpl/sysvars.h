/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef SYSVARS_H
#define SYSVARS_H

// EXTERNAL RPL MACHINE REGISTERS


// GLOBAL ARRAY OF POINTERS THAT ARE TO BE UPDATED BY THE GC
WORDPTR GC_PTRUpdate[MAX_GC_PTRUPDATE];

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
#define SuggestedObject     GC_PTRUpdate[6]     // NAME AS USED BY THE AUTOCOMPLETE FEATURE

#define EndOfObject         GC_PTRUpdate[7]     // NAME AS USED BY THE DECOMPILER
#define NextTokenStart      GC_PTRUpdate[7]     // NAME AS USED BY THE COMPILER

#define CompileStringEnd    GC_PTRUpdate[8]
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



// THE POINTERS [12] TO [15] ARE AVAILABLE FOR LIBRARIES TO USE


// MEMORY REGIONS. EACH MEMORY REGION CAN GROW INDEPENDENTLY
// GROWTH MAY TRIGGER A GC IF INSUFFICIENT MEMORY.

WORDPTR *RStk;   // BASE OF RETURN STACK
WORDPTR *DStk;   // BASE OF DATA STACK
WORDPTR *DStkBottom;   // BASE OF CURRENT DATA STACK
WORDPTR *DStkProtect;   // BASE OF PROTECTED DATA STACK
WORDPTR TempOb; // TEMPORARY OBJECT STORAGE
WORDPTR *TempBlocks;    // TEMPOB BLOCK POINTERS STORAGE
WORDPTR *Directories;   // BASE OF DIRECTORY STORAGE
WORDPTR *LAMs;          // BASE OF LOCAL VARIABLES STORAGE

// OTHER VARIABLES THAT ARE NOT AFFECTED BY GC
WORD CurOpcode; // CURRENT OPCODE (WORD)
WORD HWExceptions, Exceptions, TrappedExceptions;  // FLAGS FOR CURRENT EXCEPTIONS
WORD BreakPtFlags;                                 // FLAGS FOR HARDWARE BREAK POINTS
WORD ErrorCode, TrappedErrorCode;
WORD GCFlags;       // INTERNAL REGISTER TO INDICATE SPECIAL CONDITIONS, LIKE A GARBAGE COLLECTION HAPPENED
WORDPTR *RSTop; // TOP OF THE RETURN STACK
WORDPTR *HaltedRSTop; // TOP OF THE RETURN STACK OF HALTED PROGRAM
WORDPTR *DSTop; // TOP OF THE DATA STACK
WORDPTR *ValidateBottom; // TEMPORARY DATA AFTER OR IN THE RETURN STACK USED DURING COMPILATION
WORDPTR *ValidateTop; // TEMPORARY DATA AFTER OR IN THE RETURN STACK USED DURING COMPILATION
WORDPTR *TempBlocksEnd;  // POINTER TO END OF TEMPBLOCKS
WORDPTR *LAMTop; // TOP OF THE LAM STACK
WORDPTR *nLAMBase;  // START OF THE LAST LAM ENVIRONMENT
WORDPTR *HaltedLAMTop; // TOP OF THE LAM STACK
WORDPTR *HaltednLAMBase;  // START OF THE LAST LAM ENVIRONMENT
WORDPTR *LAMTopSaved;   // SAVED VALUE OF LAMTOP USED DURING COMPILATION
WORDPTR *ErrornLAMBase;  // SAVED BASE OF LAM ENVIRONMENT AT ERROR HANDLER
WORDPTR *ErrorLAMTop;   // SAVED VALUE OF LAMTOP AT ERROR HANDLER
WORDPTR *ErrorRSTop;     // SAVED TOP OF RETURN STACK AT ERROR HANDLER
WORDPTR *CurrentDir;     // POINTER TO CURRENT DIRECTORY
WORDPTR *DirsTop;    // POINTER TO END OF USED DIRECTORIES
BINT DStkSize;    // TOTAL SIZE OF DATA STACK
BINT RStkSize;    // TOTAL SIZE OF RETURN STACK
BINT TempBlocksSize; // TOTAL SIZE OF TEMPBLOCKS
BINT LAMSize;    // TOTAL SIZE OF LAM ENVIRONMENTS
BINT DirSize;


// ARGUMENTS TO PASS TO LIBRARY HANDLERS
// DURING COMPILATION
UBINT ArgNum1,ArgNum2,ArgNum3,RetNum;

// SOME CONVENIENCE NAMES FOR USE IN LIBRARIES
#define TokenLen ArgNum1
#define DecompMode ArgNum1

#define BlankLen ArgNum2
#define DecompHints ArgNum2

#define CurrentConstruct ArgNum3
#define SuggestedOpcode  ArgNum3
#define ObjectID   ArgNum3
#define MenuCodeArg ArgNum3
#define CmdHelp     ArgNum3
#define LibError    ArgNum3
#define TypeInfo    ArgNum3


LIBHANDLER LowLibRegistry[MAXLOWLIBS];
LIBHANDLER SysHiLibRegistry[MAXSYSHILIBS];
LIBHANDLER HiLibRegistry[MAXHILIBS];
BINT HiLibNumbers[MAXHILIBS];
BINT NumHiLibs;



// MATH LIBRARY CONTEXT
CONTEXT Context;

// PREALLOCATED STATIC REAL NUMBER REGISTERS FOR TEMPORARY STORAGE
REAL RReg[REAL_REGISTERS];
// TEMPORARY SCRATCH MEMORY FOR DIGITS
BINT RDigits[EXTRA_STORAGE];

BINT BINT2RealIdx;

#endif // SYSVARS_H
