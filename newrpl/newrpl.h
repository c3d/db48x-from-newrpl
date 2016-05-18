/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef NEWRPL_H
#define NEWRPL_H

#include <stdint.h>
#ifndef UTF8LIB_H
#include "utf8lib.h"
#endif

// EXTERNAL API FOR THE NEWRPL MACHINE - TO BE USED ONLY BY USER LIBRARIES
// BASIC CONSTANTS AND TYPE DEFINITIONS FOR THE RUN ENVIRONMENT

typedef void (*LIBHANDLER)(void);

typedef uint16_t HALFWORD;
typedef uint32_t WORD;
typedef uint8_t BYTE;
typedef WORD *WORDPTR;
typedef BYTE   *BYTEPTR;
typedef int32_t BINT;
typedef uint32_t UBINT;
typedef int64_t BINT64;
typedef uint64_t UBINT64;
#if defined(__LP64__) || defined(_WIN64)
typedef uint64_t PTR2NUMBER;
#define NUMBER2PTR(a) ((WORDPTR)((UBINT64)(a)))
#else
typedef uint32_t PTR2NUMBER;
#define NUMBER2PTR(a) ((WORDPTR)(a))
#endif



#ifdef __cplusplus
"C" {
#endif

// CONSTANTS THAT CONTROL THE MAIN RPL ENGINE

// NUMBER OF STATIC REGISTERS FOR FAST HANDLING OF REAL NUMBERS
#define REAL_REGISTERS 10
// NUMBER OF SIMULTANEOUS CONVERSIONS OF BINT TO REALS THAT CAN BE DONE
#define BINT2REAL      4

// MAXIMUM PRECISION ALLOWED IN THE SYSTEM
// MAKE SURE REAL_SCRATCHMEM CAN HAVE AT LEAST "REAL_REGISTERS*PRECISION_MAX*2/9" WORDS
// WARNING: THIS CONSTANT CANNOT BE CHANGED UNLESS ALL PRECOMPUTED CONSTANT TABLES ARE CHANGED ACCORDINGLY
#define REAL_PRECISION_MAX 2016
#define MAX_USERPRECISION  2000



// SCRATCHPAD MEMORY TO ALLOCATE DIGITS FOR ARBITRARY PRECISION TEMP RESULTS
// THIS IS THE NUMBER OF WORDS, EACH GOOD FOR 9 DIGITS.
#define REAL_REGISTER_STORAGE ((REAL_PRECISION_MAX*2)/8+3)
#define BINT_REGISTER_STORAGE  3
#define EXTRA_STORAGE BINT2REAL*BINT_REGISTER_STORAGE

// DEFINE THE LIMITS FOR THE EXPONENT RANGE FOR ALL REALS
// NOTE: THIS HAS TO FIT WITHIN THE FIELDS OF REAL_HEADER
#define REAL_EXPONENT_MAX   30000
#define REAL_EXPONENT_MIN   -30000


#ifndef DECIMAL_H
#include "decimal.h"
#endif





// HIGH LIBRARIES ARE USER LIBRARIES, SLOWER TO EXECUTE
// LOW LIBRARIES ARE SYSTEM LIBRARIES, WITH FASTER EXECUTION
#define MAXHILIBS 256
#define MAXLOWLIBS 256
#define MAXSYSHILIBS 16
#define MAXLIBNUMBER 4095
// NUMBER OF SCRATCH POINTERS
#define MAX_GC_PTRUPDATE 21


// MINIMUM GUARANTEED STACK LEVELS FOR MEMORY ALLOCATION
#define DSTKSLACK   16
// MINIMUM GUARANTEED STACK LEVELS IN RETURN STACK
#define RSTKSLACK   16
// MINIMUM GUARANTEED STACK LEVELS IN LAM STACK
#define LAMSLACK   16
// MINIMUM GUARANTEED SPACE IN TEMPOB (IN 32-BIT WORDS)
#define TEMPOBSLACK 32
// MINIMUM GUARANTEED SPACE IN TEMPBLOCKS (IN 32-BIT WORDS)
#define TEMPBLOCKSLACK 16

// MINIMUM GUARANTEED STACK LEVELS IN DIRECTORY STACK
#define DIRSLACK   16


#include "sysvars.h"

// FORMATTING FOR NUMBERS
typedef struct {
    WORD Locale;
    BINT SmallFmt;
    BINT MiddleFmt;
    BINT BigFmt;
    REAL SmallLimit;
    REAL BigLimit;
} NUMFORMAT;


typedef union {
    WORD word;
    struct {
    signed exp:16;
    unsigned len:12,flags:4;
    };
} REAL_HEADER;



// ERROR MANAGEMENT FUNCTIONS
void decTrapHandler(BINT error);
void rplSetExceptionHandler(WORDPTR Handler);
void rplRemoveExceptionHandler();
void rplCatchException();

// ERROR TRIGGER FUNCTIONS
void rplError(WORD errorcode);
void rplException(WORD exception);
void rplClearErrors();
void rplBlameUserCommand();
void rplBlameError(WORDPTR command);


#define NEEDS_CLEANUP   1

// ENVIRONMENT FUNCTIONS IN RUNSTREAM.C
void rplInit();
void rplWarmInit();
void rplHotInit();
void rplSetEntryPoint(WORDPTR ip);
BINT rplRun();
void rplCleanup();

// LIBRARY MANAGEMENT
BINT rplInstallLibrary(LIBHANDLER handler);
void rplRemoveLibrary(BINT number);


// LIBRARY LOW-LEVEL ACCESS FUNCTIONS
LIBHANDLER rplGetLibHandler(BINT libnum);
BINT rplGetNextLib(BINT libnum);

// BASIC GENERIC OBJECT FUNCTIONS
WORDPTR rplSkipOb(WORDPTR ip);
void rplSkipNext();
WORD rplObjSize(WORDPTR ip);



// LOW-LEVEL MEMORY MANAGEMENT

WORDPTR rplAllocTempOb(WORD size);
void rplTruncateLastObject(WORDPTR newend);
void rplResizeLastObject(WORD additionalsize);
void growTempOb(WORD newtotalsize);
void shrinkTempOb(WORD newtotalsize);
void rplAddTempBlock(WORDPTR block);
void growTempBlocks(WORD newtotalsize);
void shrinkTempBlocks(WORD newtotalsize);
void growDirs(WORD newtotalsize);
void shrinkDirs(WORD newtotalsize);
void growLAMs(WORD newtotalsize);
void shrinkLAMs(WORD newtotalsize);
void growDStk(WORD newtotalsize);
void shrinkDStk(WORD newtotalsize);
void growRStk(WORD newtotalsize);
void shrinkRStk(WORD newtotalsize);


// COMPILER FUNCTIONS IN COMPILER.C


WORDPTR rplCompile(BYTEPTR string, BINT len, BINT addwrapper);
void rplCompileAppend(WORD word);
void rplCompileInsert(WORDPTR position,WORD word);
WORDPTR rplCompileAppendWords(BINT nwords);
void rplCompileRemoveWords(BINT nwords);

#define DECOMP_EMBEDDED     1
#define DECOMP_EDIT         2


// DECOMPILER FUNCTIONS
WORDPTR rplDecompile(WORDPTR object, BINT flags);
void rplDecompAppendChar(BYTE c);
void rplDecompAppendString(BYTEPTR str);
void rplDecompAppendString2(BYTEPTR str,BINT len);



// DATA STACK FUNCTIONS IN DATASTACK.C

void rplPushData(WORDPTR p);
void rplPushDataNoGrow(WORDPTR p);
WORDPTR rplPopData();
WORDPTR rplPeekData(int level);
void rplOverwriteData(int level,WORDPTR ptr);
BINT rplDepthData();
void rplClearData();
void rplDropData(int n);
void rplExpandStack(BINT numobjects);
void growDStk(WORD newsize);
WORDPTR *rplProtectData();
WORDPTR *rplUnprotectData();


// SNAPSHOT FUNCTIONS THAT SAVE/RESTORE THE STACK
BINT rplCountSnapshots();
void rplTakeSnapshot();
void rplRemoveSnapshot(BINT numsnap);
void rplRestoreSnapshot(BINT numsnap);
void rplRevertToSnapshot(BINT numsnap);
void rplTakeSnapshotN(BINT nargs);
void rplTakeSnapshotHide(BINT nargs);
void rplTakeSnapshotAndClear();
BINT rplDepthSnapshot(BINT numsnap);
WORDPTR rplPeekSnapshot(BINT numsnap,BINT level);










// RETURN STACK FUNCTIONS IN RETURNSTACK.C

void rplPushRet(WORDPTR p);
WORDPTR rplPopRet();
void growRStk(WORD newsize);
WORDPTR rplPeekRet(int level);
void rplClearRStk();
BINT rplDepthRet();


// SYSTEM FLAGS
BINT rplSetSystemFlag(BINT flag);
BINT rplSetSystemFlagByName(BYTEPTR name,BYTEPTR nameend);
BINT rplSetSystemFlagByIdent(WORDPTR ident);
BINT rplClrSystemFlag(BINT flag);
BINT rplClrSystemFlagByName(BYTEPTR name, BYTEPTR nameend);
BINT rplClrSystemFlagByIdent(WORDPTR ident);
BINT rplTestSystemFlag(BINT flag);
BINT rplTestSystemFlagByName(BYTEPTR name, BYTEPTR nameend);
BINT rplTestSystemFlagByIdent(WORDPTR ident);

WORD rplGetSystemLocale();
void rplGetSystemNumberFormat(NUMFORMAT *fmt);
void rplSetSystemNumberFormat(NUMFORMAT *fmt);

// SYSTEM SOFT MENUS
void rplSetMenuCode(BINT menunumber,WORD menucode);
WORD rplGetMenuCode(BINT menunumber);
void rplSetActiveMenu(BINT menunumber);
BINT rplGetActiveMenu();
void rplChangeMenu(WORDPTR newmenu);
void rplSetLastMenu(BINT menunumber);
BINT rplGetLastMenu();


// SYSTEM AUTOCOMPLETE
WORD rplGetNextSuggestion(WORD suggestion, BYTEPTR start, BYTEPTR end);
WORD rplGetPrevSuggestion(WORD suggestion,BYTEPTR start,BYTEPTR end);
WORD rplUpdateSuggestion(WORD suggestion,BYTEPTR start,BYTEPTR end);



// GARBAGE COLLECTION
void rplGCollect();

// BACKUP/RESTORE
BINT rplBackup(void (*writefunc)(unsigned int));
BINT rplRestoreBackup(WORD (*readfunc)());


// SYSTEM SANITY CHECKS
BINT rplVerifyObject(WORDPTR obj);
BINT rplIsTempObPointer(WORDPTR ptr);
BINT rplVerifyObjPointer(WORDPTR ptr);
BINT rplVerifyDStack(BINT fix);
BINT rplVerifyRStack();
BINT rplVerifyTempOb(BINT fix);
BINT rplVerifyDirectories(BINT fix);



//IDENTIFIER FUNCTIONS
BINT rplGetIdentLength(WORDPTR ident);
void rplCompileIDENT(BINT libnum, BYTEPTR tok, BYTEPTR tokend);
WORDPTR rplCreateIDENT(BINT libnum,BYTEPTR tok,BYTEPTR tokend);
BINT rplIsValidIdent(BYTEPTR tok,BYTEPTR tokend);




// LAM FUNCTIONS
void growLAMs(WORD newtotalsize);
void rplCreateLAMEnvironment(WORDPTR owner);
BINT rplCreateLAM(WORDPTR nameobj,WORDPTR value);
BINT rplCompareIDENT(WORDPTR id1,WORDPTR id2);
BINT rplCompareIDENTByName(WORDPTR id1, BYTEPTR name, BYTEPTR nameend);
BINT rplCompareObjects(WORDPTR id1,WORDPTR id2);
WORDPTR rplGetLAM(WORDPTR nameobj);
WORDPTR *rplGetLAMn(BINT idx);
WORDPTR *rplGetLAMnName(BINT idx);
WORDPTR *rplGetLAMnEnv(WORDPTR *LAMEnv, BINT idx);
WORDPTR *rplGetLAMnNameEnv(WORDPTR *LAMEnv, BINT idx);
void rplPutLAMn(BINT idx,WORDPTR object);
void rplCleanupLAMs(WORDPTR currentseco);
void rplClearLAMs();
WORDPTR *rplFindLAM(WORDPTR nameobj, BINT scanparents);
WORDPTR *rplFindLAMbyName(BYTEPTR name,BINT len,BINT scanparents);
WORDPTR *rplGetNextLAMEnv(WORDPTR *startpoint);
BINT rplNeedNewLAMEnv();
BINT rplNeedNewLAMEnvCompiler();
BINT rplLAMCount(WORDPTR *LAMEnvironment);


// GLOBAL VARIABLES AND DIRECTORY FUNCTIONS
void growDirs(WORD newtotalsize);
WORDPTR rplMakeIdentQuoted(WORDPTR ident);
void rplCreateGlobalInDir(WORDPTR nameobj,WORDPTR value,WORDPTR *parentdir);
void rplCreateGlobal(WORDPTR nameobj,WORDPTR value);
void rplPurgeGlobal(WORDPTR nameobj);
WORDPTR *rplFindDirbyHandle(WORDPTR handle);
WORDPTR rplCreateNewDir(WORDPTR nameobj, WORDPTR *parentdir);
void rplPurgeDir(WORDPTR nameobj);
WORDPTR *rplGetParentDir(WORDPTR *directory);
// VARIOUS WAYS TO RCL GLOBAL VARIABLES
WORDPTR *rplFindGlobalbyNameInDir(BYTEPTR name, BYTEPTR nameend, WORDPTR *parent, BINT scanparents);
WORDPTR *rplFindGlobalbyName(BYTEPTR name, BYTEPTR nameend, BINT scanparents);
WORDPTR *rplFindGlobalByIndexInDir(BINT idx,WORDPTR *directory);
WORDPTR *rplFindGlobalByIndex(BINT idx);
WORDPTR *rplFindGlobalInDir(WORDPTR nameobj,WORDPTR *parentdir,BINT scanparents);
WORDPTR *rplFindGlobal(WORDPTR nameobj,BINT scanparents);
WORDPTR *rplFindVisibleGlobalByIndexInDir(BINT idx,WORDPTR *directory);
WORDPTR *rplFindVisibleGlobalByIndex(BINT idx);
// DIRECTORY SCANNING AND LOWER-LEVEL ACCESS
WORDPTR *rplFindFirstInDir(WORDPTR *directory);
WORDPTR *rplFindFirstByHandle(WORDPTR dirhandle);
WORDPTR *rplFindNext(WORDPTR *direntry);
BINT rplGetVarCountInDir(WORDPTR *directory);
BINT rplGetVarCount();
BINT rplGetVisibleVarCountInDir(WORDPTR *directory);
BINT rplGetVisibleVarCount();

WORDPTR rplGetGlobal(WORDPTR nameobj);
WORDPTR *rplMakeNewDir();
WORDPTR rplGetDirName(WORDPTR *dir);
BINT rplGetFullPath(WORDPTR *dir,WORDPTR *buffer,BINT maxdepth);
WORDPTR *rplGetDirfromGlobal(WORDPTR *var);
WORDPTR *rplDeepCopyDir(WORDPTR *sourcedir);
void rplWipeDir(WORDPTR *directory);
void rplPurgeForced(WORDPTR *var);

// FUNCTIONS SPECIFIC FOR THE .Settings DIRECTORY
void rplStoreSettings(WORDPTR nameobject,WORDPTR object);
void rplStoreSettingsbyName(BYTEPTR name, BYTEPTR nameend, WORDPTR object);
WORDPTR rplGetSettings(WORDPTR nameobject);
WORDPTR rplGetSettingsbyName(BYTEPTR name, BYTEPTR nameend);







// GENERIC OBJECT FUNCTIONS
void rplCallOvrOperator(WORD op);
void rplCallOperator(WORD op);
void rplCopyObject(WORDPTR dest, WORDPTR src);
WORDPTR rplMakeNewCopy(WORDPTR object);


// BINT FUNCTIONS
WORDPTR rplNewSINT(int num,int base);
WORDPTR rplNewBINT(BINT64 num,int base);
void rplNewSINTPush(int num,int base);
void rplNewBINTPush(BINT64 num,int base);
BINT64 rplReadBINT(WORDPTR ptr);
WORDPTR rplWriteBINT(BINT64 num,int base,WORDPTR dest);
void rplCompileBINT(BINT64 num,int base);


// TRUE/FALSE FUNCTIONS
void rplPushFalse();
void rplPushTrue();
BINT rplIsFalse(WORDPTR objptr);
BINT rplIsTrue(WORDPTR objptr);


// REAL FUNCTIONS
void rplOneToRReg(int num);
void rplZeroToRReg(int num);
void rplInfinityToRReg(int num);
void rplNANToRReg(int num);
void rplBINTToRReg(int num,BINT64 value);
void rplReadReal(WORDPTR real,REAL *dec);
void rplCopyRealToRReg(int num,WORDPTR real);
WORDPTR rplNewReal(REAL *num);
WORDPTR rplNewRealInPlace(REAL *num,WORDPTR addr);
WORDPTR rplNewRealFromRReg(int num);
void rplNewRealPush(REAL *num);
void rplNewRealFromRRegPush(int num);
void rplNewApproxRealFromRRegPush(int num);
WORDPTR rplRRegToRealInPlace(int num, WORDPTR dest);
void rplCheckResultAndError(REAL *real);
void rplCompileReal(REAL *num);


// COMPLEX FUNCTIONS
void rplRealPart(WORDPTR complex,REAL *real);
void rplImaginaryPart(WORDPTR complex,REAL *imag);
BINT rplPolarComplexMode(WORDPTR complex);
void rplReadCNumber(WORDPTR complex,REAL *real,REAL *imag, BINT *angmode);
void rplReadCNumberAsReal(WORDPTR complex,REAL *real);
void rplReadCNumberAsImag(WORDPTR complex,REAL *imag);
WORDPTR rplNewComplex(REAL *real, REAL *imag, BINT angmode);
void rplNewComplexPush(REAL *real, REAL *imag, BINT angmode);
void rplRRegToComplexPush(BINT real, BINT imag, BINT angmode);
WORDPTR rplRRegToComplexInPlace(BINT real, BINT imag, WORDPTR dest, BINT angmode);
void rplRect2Polar(REAL *re,REAL *im,BINT angmode);
void rplPolar2Rect(REAL *r,REAL *theta,BINT angmode);
BINT rplIsZeroComplex(REAL *re,REAL *im,BINT angmode);




// GENERIC FUNCTIONS FOR BINTS AND REALS
void rplNumberToRReg(int num,WORDPTR number);
BINT64 rplReadNumberAsBINT(WORDPTR number);
void rplReadNumberAsReal(WORDPTR number,REAL*dec);
void rplLoadBINTAsReal(BINT64 number,REAL*dec);
BINT rplIsNegative(WORDPTR objptr);


// ANGLE FUNCTIONS
WORDPTR rplNewAngleFromReal(REAL *number,BINT newmode);
WORDPTR rplNewAngleFromNumber(WORDPTR numobj,BINT newmode);
void rplConvertAngleObj(WORDPTR angleobj,BINT newmode);





// UNIT FUNCTIONS
BINT rplUnitExplode(WORDPTR unitobj);
WORDPTR rplUnitAssemble(BINT nlevels);
BINT rplUnitPopItem(BINT level);
void rplUnitPickItem(BINT level);
BINT rplUnitMulItem(BINT level1,BINT level2);
void rplUnitPowItem(BINT level1,BINT level2);
BINT rplUnitSkipItem(BINT level);
BINT rplUnitSimplify(BINT nlevels);
BINT rplUnitDivide(BINT numlvl,BINT divlvl);
void rplUnitInvert(BINT level);
BINT rplUnitExpand(BINT level);
BINT rplUnitToBase(BINT nlevels);
BINT rplUnitSort(BINT nlevels, BINT reflevel);
BINT rplUnitIsConsistent(BINT nlevels,BINT reflevel);
BINT rplUnitPow(BINT lvlexp,BINT nlevels);


BINT rplUnitIsSpecial(WORDPTR unitobj);
void rplUnitReplaceSpecial(BINT nlevels);
void rplUnitReverseReplaceSpecial(BINT nlevels);
void rplUnitReverseReplaceSpecial2(BINT isspec_idx);
void rplUnitSpecialToDelta(BINT nlevels);
WORDPTR *rplUnitFindCustom(WORDPTR ident,BINT *siindex);



// LIST FUNCTIONS
BINT rplListLength(WORDPTR composite);
BINT rplListLengthFlat(WORDPTR composite);
void rplCreateList();
BINT rplExplodeList(WORDPTR composite);
WORDPTR rplGetListElement(WORDPTR composite, BINT pos);
WORDPTR rplGetListElementFlat(WORDPTR composite, BINT pos);
BINT rplIsLastElementFlat(WORDPTR composite, BINT pos);
void rplListUnaryDoCmd();
void rplListBinaryDoCmd(WORDPTR arg1, WORDPTR arg2);

// SYMBOLIC FUNCTIONS
WORDPTR rplSymbUnwrap(WORDPTR symbolic);
WORD rplSymbMainOperator(WORDPTR symbolic);
WORDPTR rplSymbMainOperatorPTR(WORDPTR symbolic);
BINT rplIsAllowedInSymb(WORDPTR object);
void rplSymbApplyOperator(WORD Opcode,BINT nargs);
void rplSymbRuleMatch();
void rplSymbRuleApply();
BINT rplSymbIsRule(WORDPTR ptr);
void rplSymbAutoSimplify();
WORDPTR rplSymbNumericReduce(WORDPTR object);


// INTERNAL SYMBOLIC API, FOR USE BY OTHER LIBRARIES
BINT rplCheckCircularReference(WORDPTR env_owner,WORDPTR object,BINT lamnum);
BINT rplFractionSimplify();
BINT rplFractionAdd();
WORDPTR rplSymbCanonicalForm(WORDPTR object);


// STRINGS
// RPL STRING OBJECT
void rplSetStringLength(WORDPTR string,BINT length);
BINT rplStrLen(WORDPTR string);
BINT rplStrSize(WORDPTR string);
BINT rplStringGetLinePtr(WORDPTR str,BINT line);
BINT rplStringCountLines(WORDPTR str);

WORDPTR rplCreateString(BYTEPTR text,BYTEPTR textend);


// MATRIX
WORDPTR rplMatrixCompose(BINT rows,BINT cols);
WORDPTR *rplMatrixExplode();
WORDPTR rplMatrixGetFirstObj(WORDPTR matrix);
WORDPTR rplMatrixGet(WORDPTR matrix,BINT row,BINT col);
WORDPTR rplMatrixFastGet(WORDPTR matrix,BINT row,BINT col);
WORDPTR *rplMatrixFastGetEx(WORDPTR *first,BINT cols,BINT i,BINT j);
WORDPTR *rplMatrixNewEx(BINT rows,BINT cols);
void rplMatrixNorm();
void rplMatrixNeg();
void rplMatrixEval1();
void rplMatrixEval();
void rplMatrixToNum();
void rplMatrixAdd();
void rplMatrixSub();
void rplMatrixMul();
void rplMatrixMulScalar();
void rplMatrixBareiss();
void rplMatrixBareissEx(WORDPTR *a,BINT rowsa,BINT colsa);
void rplMatrixInvert();
void rplMatrixBackSubstEx(WORDPTR *a,BINT rowsa,BINT colsa);
BINT rplMatrixIsPolar(WORDPTR matobj);
void rplMatrixPolarToRectEx(WORDPTR *a,BINT rowsa,BINT colsa);
void rplMatrixRectToPolarEx(WORDPTR *a,BINT rowsa,BINT colsa,WORD angtemplate,BINT angmode);


// ANGULAR MODES
#define ANGLENONE    -1
#define ANGLEDEG     0
#define ANGLERAD     1
#define ANGLEGRAD    2
#define ANGLEDMS     3


// SYSTEM FLAGS
#define FL_ACTIVEMENU     -11
#define FL_LASTMENU       -12
#define FL_ANGLEMODE1     -17
#define FL_ANGLEMODE2     -18
#define FL_UNDERFLOWERROR -20
#define FL_OVERFLOWERROR  -21
#define FL_INIFINITEERROR -22
#define FL_NEGUNDERFLOW   -23
#define FL_POSUNDERFLOW   -24
#define FL_OVERFLOW       -25
#define FL_INFINITE       -26
#define FL_STRIPCOMMENTS  -30
#define FL_COMPLEXMODE    -103













// DEFINED EXCEPTIONS

#define EX_EXITRPL          1
#define EX_BKPOINT          2
#define EX_OUTOFMEM         4
#define EX_ERRORCODE        8
#define EX_POWEROFF        16

// ADD OTHER EXCEPTIONS HERE


#ifdef __cplusplus
}
#endif


#include "arithmetic.h"

#endif // NEWRPL_H
