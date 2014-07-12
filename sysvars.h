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
extern WORDPTR GC_PTRUpdate[MAX_GC_PTRUPDATE];

#define IPtr                GC_PTRUpdate[0]
#define TempObEnd           GC_PTRUpdate[1]     // END OF KNOWN AND STABLE TEMPOB BLOCKS
#define TempObSize          GC_PTRUpdate[2]
#define CompileEnd          GC_PTRUpdate[3]     // ADDITIONAL DATA AT THE END OF TEMPOB DURING COMPILE
#define ExceptionPointer    GC_PTRUpdate[4]
#define ArgPtr1             GC_PTRUpdate[5]     // NAME FOR USE BY THE LIBRARIES
#define DecompileObject     GC_PTRUpdate[5]     // NAME FOR USE BY THE DECOMPILER
#define TokenStart          GC_PTRUpdate[5]     // NAME AS USED BY THE COMPILER
#define ArgPtr2             GC_PTRUpdate[6]     // NAME FOR USE BY LIBRARIES
#define BlankStart          GC_PTRUpdate[6]     // NAME AS USED BY THE COMPILER
#define DecompStringEnd     GC_PTRUpdate[6]     // NAME AS USED BY THE DECOMPILER
#define EndOfObject         GC_PTRUpdate[7]     // NAME AS USED BY THE DECOMPILER
#define NextTokenStart      GC_PTRUpdate[7]     // NAME AS USED BY THE COMPILER
#define CompileStringEnd    GC_PTRUpdate[8]
#define ErrorHandler        GC_PTRUpdate[9]
#define SettingsDir         GC_PTRUpdate[10]
#define SystemFlags         GC_PTRUpdate[11]
#define LastCompiledObject  GC_PTRUpdate[12]
#define SavedDecompObject   GC_PTRUpdate[12]
#define ScratchPointer1     GC_PTRUpdate[13]
#define ScratchPointer2     GC_PTRUpdate[14]
#define ScratchPointer3     GC_PTRUpdate[15]
#define ScratchPointer4     GC_PTRUpdate[16]
#define ScratchPointer5     GC_PTRUpdate[17]


// THE POINTERS [12] TO [15] ARE AVAILABLE FOR LIBRARIES TO USE


// MEMORY REGIONS. EACH MEMORY REGION CAN GROW INDEPENDENTLY
// GROWTH MAY TRIGGER A GC IF INSUFFICIENT MEMORY.

extern WORDPTR *RStk;   // BASE OF RETURN STACK
extern WORDPTR *DStk;   // BASE OF DATA STACK
extern WORDPTR *DStkProtect;   // BASE OF PROTECTED DATA STACK
extern WORDPTR TempOb; // TEMPORARY OBJECT STORAGE
extern WORDPTR *TempBlocks;    // TEMPOB BLOCK POINTERS STORAGE
extern WORDPTR *Directories;   // BASE OF DIRECTORY STORAGE
extern WORDPTR *LAMs;          // BASE OF LOCAL VARIABLES STORAGE

// OTHER VARIABLES THAT ARE NOT AFFECTED BY GC
extern WORD CurOpcode; // CURRENT OPCODE (WORD)
extern WORD Exceptions, TrappedExceptions;  // FLAGS FOR CURRENT EXCEPTIONS
extern WORDPTR *RSTop; // TOP OF THE RETURN STACK
extern WORDPTR *DSTop; // TOP OF THE DATA STACK
extern WORDPTR *ValidateTop; // TEMPORARY DATA AFTER THE RETURN STACK USED DURING COMPILATION
extern WORDPTR *TempBlocksEnd;  // POINTER TO END OF TEMPBLOCKS
extern WORDPTR *LAMTop; // TOP OF THE LAM STACK
extern WORDPTR *nLAMBase;  // START OF THE LAST LAM ENVIRONMENT
extern WORDPTR *LAMTopSaved;   // SAVED VALUE OF LAMTOP USED DURING COMPILATION
extern WORDPTR *ErrornLAMBase;  // SAVED BASE OF LAM ENVIRONMENT AT ERROR HANDLER
extern WORDPTR *ErrorLAMTop;   // SAVED VALUE OF LAMTOP AT ERROR HANDLER
extern WORDPTR *ErrorRSTop;     // SAVED TOP OF RETURN STACK AT ERROR HANDLER
extern WORDPTR *CurrentDir;     // POINTER TO CURRENT DIRECTORY
extern WORDPTR *DirsTop;    // POINTER TO END OF USED DIRECTORIES
extern BINT DStkSize;    // TOTAL SIZE OF DATA STACK
extern BINT RStkSize;    // TOTAL SIZE OF RETURN STACK
extern BINT TempBlocksSize; // TOTAL SIZE OF TEMPBLOCKS
extern BINT LAMSize;    // TOTAL SIZE OF LAM ENVIRONMENTS
extern BINT DirSize;


// ARGUMENTS TO PASS TO LIBRARY HANDLERS
// DURING COMPILATION
extern UBINT ArgNum1,ArgNum2,ArgNum3,RetNum;

// SOME CONVENIENCE NAMES FOR USE IN LIBRARIES
#define TokenLen ArgNum1
#define BlankLen ArgNum2
#define CurrentConstruct ArgNum3



extern LIBHANDLER LowLibRegistry[MAXLOWLIBS];
extern LIBHANDLER HiLibRegistry[MAXHILIBS];
extern BINT HiLibNumbers[MAXHILIBS];
extern BINT NumHiLibs;



// MATH LIBRARY CONTEXT
extern mpd_context_t Context;

// PREALLOCATED STATIC REAL NUMBER REGISTERS FOR TEMPORARY STORAGE
extern mpd_t RReg[REAL_REGISTERS];
// TEMPORARY SCRATCH MEMORY FOR DIGITS
extern mpd_uint_t RDigits[REAL_SCRATCHMEM];

extern BINT BINT2RealIdx;

#endif // SYSVARS_H
