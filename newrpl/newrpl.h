/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef NEWRPL_H
#define NEWRPL_H

#include <stdint.h>

#ifndef FIRMWARE_H
#include <firmware.h>
#endif

#ifndef UTF8LIB_H
#include "utf8lib.h"
#endif


// BUILD SYSTEM PROVIDES NEWRPL_BUILDNUM MACRO WITH THE COMMIT NUMBER



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
#if (defined(__LP64__) || defined(_WIN64)) && !defined(TARGET_50G)
typedef uint64_t PTR2NUMBER;
#define NUMBER2PTR(a) ((WORDPTR)((UBINT64)(a)))
#else
typedef uint32_t PTR2NUMBER;
#define NUMBER2PTR(a) ((WORDPTR)((WORD)(a)))
#endif




#ifdef __cplusplus
extern "C" {
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
// THIS IS THE NUMBER OF WORDS, EACH GOOD FOR 8 DIGITS.
// THIS AREA WILL ALSO BE SHARED WITH THE FILE SYSTEM, SO MAKE SURE
// REAL_REGISTER_STORAGE HAS AT LEAST 512 BYTES TO STORE ONE SECTOR

#define REAL_REGISTER_STORAGE ((REAL_PRECISION_MAX*3)/8+3)
#define BINT_REGISTER_STORAGE  3
#define EXTRA_STORAGE BINT2REAL*BINT_REGISTER_STORAGE

// DEFINE THE LIMITS FOR THE EXPONENT RANGE FOR ALL REALS
// NOTE: THIS HAS TO FIT WITHIN THE FIELDS OF REAL_HEADER
#define REAL_EXPONENT_MAX   30000
#define REAL_EXPONENT_MIN   -30000


#ifndef DECIMAL_H
#include "decimal.h"
#endif

#ifndef FASTMATH_H
#include "fastmath.h"
#endif




// HIGH LIBRARIES ARE USER LIBRARIES, SLOWER TO EXECUTE
// LOW LIBRARIES ARE SYSTEM LIBRARIES, WITH FASTER EXECUTION
#define MAXHILIBS 256
#define MAXLOWLIBS 256
#define MAXSYSHILIBS 16
#define MAXLIBNUMBER 4095
// NUMBER OF SCRATCH POINTERS
#define MAX_GC_PTRUPDATE 29


// MINIMUM GUARANTEED STACK LEVELS FOR MEMORY ALLOCATION
#define DSTKSLACK   16
// MINIMUM GUARANTEED STACK LEVELS IN RETURN STACK
#define RSTKSLACK   16
// MINIMUM GUARANTEED STACK LEVELS IN LAM STACK
#define LAMSLACK   16
// MINIMUM GUARANTEED SPACE IN TEMPOB FOR LOWMEM CONDITIONS (IN 32-BIT WORDS)
#define TEMPOBSLACK 32
// MINIMUM GUARANTEED SPACE IN TEMPOB FOR NORMAL OPERATION (IN 32-BIT WORDS)
#define TEMPOBLARGESLACK 128
// MINIMUM GUARANTEED SPACE IN TEMPBLOCKS (IN 32-BIT WORDS)
#define TEMPBLOCKSLACK 16

// MINIMUM GUARANTEED STACK LEVELS IN DIRECTORY STACK
#define DIRSLACK   16


#include "sysvars.h"

// FORMATTING FOR NUMBERS
typedef struct {
    UBINT64 Locale;
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


// COMPACT TIME STRUCTURE - 32 BITS.
struct time {
    unsigned sec    : 6,    // seconds after the minute	0-59
             min    : 6,    // minutes after the hour	0-59
             hour   : 5,    // hours since midnight     0-23
             isdst  : 1,    // daylight saving time flag
                    : 14;   // to pad up to 32 bits
};

// COMPACT DATE STRUCTURE - 32 BITS.
struct date {
    unsigned mday   : 5,    // day of the month     1-31
             mon    : 4,    // months since January	1-12
             wday   : 3,    // days since Monday	1-7
             year   : 14,   // years                1582-9999
                    : 6;    // to pad up to 32 bits
};

#define TIME_MAXSEC  ((1 <<  6) - 1)
#define TIME_MAXMIN  ((1 <<  6) - 1)
#define TIME_MAXHOUR ((1 <<  5) - 1)
#define DATE_MAXDAY  ((1 <<  5) - 1)
#define DATE_MAXMON  ((1 <<  4) - 1)
#define DATE_MAXYEAR ((1 << 14) - 1)

// ALARM STRUCTURE
// AN ALARM CAN BE IN 5 DIFFERENT STATES:
// 1-PENDING ALARM (ALARM DUE):
//   0x0 - ann=0 & ack=0 & dis=0
// 2-PAST DUE ALARM:
//   0x1 - ann=1 & ack=0 & dis=0
// 3-PAST ALARM ACKNOWLEDGED:
//   0x2 - ann=0 & ack=1 & dis=0
// 4-PAST CONTROL ALARM:
//   0X3 - ann=1 & ack=1 & dis=0
// 5-DISABLED ALARM:
//   0x? - ann=? & ack=? & dis=1
struct alarm {
    BINT64   time;          // alarm time (elapsed seconds since 10/15/1582)
    UBINT    rpt;           // repeat interval (seconds)
    WORDPTR  obj;
    union  {
    BYTE     flags;         // Represents all alarm flags.
    struct {
    unsigned ann     : 1,   // announced alarm?       Y:1 N:0
             ack     : 1,   // acknowledged alarm?    Y:1 N:0
             dis     : 1,   // disabled alarm?        Y:1 N:0
                     : 5;   // to pad up to 8 bits
    };
    };
};

#define DUE_ALM      0x0
#define PASTDUE_ALM  0x1
#define PAST_ALM     0x2
#define PAST_CTLALM  0x3
#define DISABLED_ALM 0x4

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

#define CLEAN_RUN       0
#define NEEDS_CLEANUP   1
#define CODE_HALTED     2

// ENVIRONMENT FUNCTIONS IN RUNSTREAM.C
void rplInit();
void rplWarmInit();
void rplHotInit();
void rplSetEntryPoint(WORDPTR ip);
BINT rplRun();
void rplCleanup();
void rplDisableSingleStep();
void rplEnableSingleStep();
void rplResetSystemFlags();

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

BINT rplGetFreeMemory();
WORDPTR rplAllocTempOb(WORD size);
WORDPTR rplAllocTempObLowMem(WORD size);
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
#define DECOMP_NOHINTS      4
#define DECOMP_MAXWIDTH(n) (((n)&0xfff)<<4)
#define DECOMP_GETMAXWIDTH(f) (((f)>>4)&0xfff)

#define DEFAULT_DECOMP_WIDTH 20


// DECOMPILER FUNCTIONS
WORDPTR rplDecompile(WORDPTR object, BINT flags);
void rplDecompAppendChar(BYTE c);
void rplDecompAppendUTF8(WORD utf8bytes);
void rplDecompAppendString(BYTEPTR str);
void rplDecompAppendString2(BYTEPTR str,BINT len);
BINT rplDecompDoHintsWidth(BINT dhints);




// DATA STACK FUNCTIONS IN DATASTACK.C

void rplPushData(WORDPTR p);
void rplPushDataNoGrow(WORDPTR p);
WORDPTR rplPopData();
WORDPTR rplPeekData(int level);
void rplOverwriteData(int level,WORDPTR ptr);
BINT rplDepthData();
void rplClearData();
void rplDropData(int n);
void rplRemoveAtData(BINT level,BINT num);
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
void rplPushRetNoGrow(WORDPTR p);
WORDPTR rplPopRet();
void rplDropRet(int nlevels);
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

// USER FLAGS
BINT rplSetUserFlag(BINT flag);
BINT rplClrUserFlag(BINT flag);
BINT rplTestUserFlag(BINT flag);
UBINT64 *rplGetUserFlagsLow();



UBINT64 rplGetSystemLocale();
void rplGetSystemNumberFormat(NUMFORMAT *fmt);
void rplSetSystemNumberFormat(NUMFORMAT *fmt);

// SYSTEM SOFT MENUS
void rplSetMenuCode(BINT menunumber,WORD menucode);
WORD rplGetMenuCode(BINT menunumber);
void rplSetActiveMenu(BINT menunumber);
BINT rplGetActiveMenu();
void rplChangeMenu(BINT menu, WORDPTR newmenu);
void rplSetLastMenu(BINT menunumber);
BINT rplGetLastMenu();
WORDPTR rplPopMenuHistory(BINT menu);
void rplSaveMenuHistory(BINT menu);




// SYSTEM AUTOCOMPLETE
WORD rplGetNextSuggestion(WORD suggestion, BYTEPTR start, BYTEPTR end);
WORD rplGetPrevSuggestion(WORD suggestion,BYTEPTR start,BYTEPTR end);
WORD rplUpdateSuggestion(WORD suggestion,BYTEPTR start,BYTEPTR end);



// GARBAGE COLLECTION
void rplGCollect();

// BACKUP/RESTORE
BINT rplBackup(void (*writefunc)(unsigned int,void *),void *OpaqueArg);
BINT rplRestoreBackup(WORD (*readfunc)(void *),void *Opaque);


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
WORDPTR rplMakeIdentUnquoted(WORDPTR ident);
WORDPTR rplMakeIdentHidden(WORDPTR ident);
WORDPTR rplMakeIdentVisible(WORDPTR ident);
WORDPTR rplMakeIdentReadOnly(WORDPTR ident);
WORDPTR rplMakeIdentWriteable(WORDPTR ident);


void rplCreateGlobalInDir(WORDPTR nameobj,WORDPTR value,WORDPTR *parentdir);
void rplCreateGlobal(WORDPTR nameobj,WORDPTR value);
void rplPurgeGlobal(WORDPTR nameobj);
WORDPTR *rplFindDirbyHandle(WORDPTR handle);
WORDPTR *rplFindDirFromPath(WORDPTR pathlist,BINT uselastname);
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
BINT rplIsVarVisible(WORDPTR *var);
BINT rplIsVarReadOnly(WORDPTR *var);
BINT rplIsVarDirectory(WORDPTR *var);
BINT rplIsVarEmptyDir(WORDPTR *var);


WORDPTR rplGetGlobal(WORDPTR nameobj);
WORDPTR *rplMakeNewDir();
WORDPTR rplGetDirName(WORDPTR *dir);
BINT rplGetFullPath(WORDPTR *dir,WORDPTR *buffer,BINT maxdepth);
WORDPTR *rplGetDirfromGlobal(WORDPTR *var);
WORDPTR *rplDeepCopyDir(WORDPTR *sourcedir);
void rplWipeDir(WORDPTR *directory);
void rplPurgeForced(WORDPTR *var);

// FUNCTIONS SPECIFIC FOR THE .Settings DIRECTORY
void rplPurgeSettings(WORDPTR nameobj);
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
void rplUndInfinityToRReg(int num);
void rplNANToRReg(int num);
void rplBINTToRReg(int num,BINT64 value);
void rplReadReal(WORDPTR real,REAL *dec);
BINT rplReadRealFlags(WORDPTR object);
BINT rplIsNumberZero(WORDPTR obj);
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
BINT rplComplexClass(WORDPTR complex);
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
BINT rplIntToString(BINT64 number,BINT base,BYTEPTR buffer,BYTEPTR endbuffer);



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
WORDPTR rplCreateListN(BINT num, BINT level, BINT remove);
void rplListAutoExpand(WORDPTR list);
BINT rplExplodeList(WORDPTR composite);
BINT rplExplodeList2(WORDPTR composite);
WORDPTR rplGetListElement(WORDPTR composite, BINT pos);
WORDPTR rplGetListElementFlat(WORDPTR composite, BINT pos);
WORDPTR rplGetNextListElementFlat(WORDPTR composite, WORDPTR elem);
BINT rplIsLastElementFlat(WORDPTR composite, BINT pos);
void rplListUnaryDoCmd();
void rplListUnaryNoResultDoCmd();
void rplListBinaryDoCmd();
void rplListMultiArgDoCmd(BINT nargs);
WORDPTR rplListAddRot(WORDPTR list,WORDPTR object,BINT nmax);
WORDPTR rplListReplace(WORDPTR list,BINT position,WORDPTR object);
WORDPTR rplListReplaceMulti(WORDPTR list,BINT position,WORDPTR object);



// SYMBOLIC FUNCTIONS
WORDPTR rplSymbUnwrap(WORDPTR symbolic);
WORDPTR rplSymbWrap(WORDPTR obj);
void rplSymbWrapN(BINT level, BINT nargs);
WORD rplSymbMainOperator(WORDPTR symbolic);
WORDPTR rplSymbMainOperatorPTR(WORDPTR symbolic);
BINT rplIsAllowedInSymb(WORDPTR object);
void rplSymbApplyOperator(WORD Opcode,BINT nargs);
void rplSymbRuleMatch();
void rplSymbRuleApply();
BINT rplSymbIsRule(WORDPTR ptr);
void rplSymbAutoSimplify();
WORDPTR rplSymbNumericReduce(WORDPTR object);
BINT rplSymbIsNumeric(WORDPTR ptr);
BINT rplSymbIsZero(WORDPTR ptr);


// INTERNAL SYMBOLIC API, FOR USE BY OTHER LIBRARIES
BINT rplCheckCircularReference(WORDPTR env_owner,WORDPTR object,BINT lamnum);
BINT rplFractionSimplify();
BINT rplFractionAdd();
WORDPTR rplSymbCanonicalForm(WORDPTR object);
BINT rplSymbExplodeOneLevel(WORDPTR object);


// STRINGS
// RPL STRING OBJECT
void rplSetStringLength(WORDPTR string,BINT length);
BINT rplStrLen(WORDPTR string);
BINT rplStrLenCp(WORDPTR string);
BINT rplStrSize(WORDPTR string);
BINT rplStringGetLinePtr(WORDPTR str,BINT line);
BINT rplStringGetNextLine(WORDPTR str,BINT prevlineoff);

BINT rplStringCountLines(WORDPTR str);
BINT rplStringCompare(WORDPTR str1, WORDPTR str2);
WORDPTR rplCreateString(BYTEPTR text,BYTEPTR textend);
WORDPTR rplCreateStringBySize(BINT lenbytes);


// MATRIX
WORDPTR rplMatrixCompose(BINT rows,BINT cols);
WORDPTR rplMatrixComposeN(BINT level,BINT rows,BINT cols);
WORDPTR rplMatrixFlexComposeN(BINT level,BINT totalelements);
WORDPTR rplMatrixFill(BINT rows,BINT cols,WORDPTR obj);
WORDPTR rplMatrixIdent(BINT rows);



BINT rplMatrixIsAllowed(WORDPTR object);


WORDPTR *rplMatrixExplode();
WORDPTR *rplMatrixExplodeByCols();
WORDPTR rplMatrixGetFirstObj(WORDPTR matrix);
BINT rplMatrixCols(WORDPTR matrix);
BINT rplMatrixRows(WORDPTR matrix);
WORDPTR rplMatrixGet(WORDPTR matrix,BINT row,BINT col);
WORDPTR rplMatrixFastGet(WORDPTR matrix,BINT row,BINT col);
WORDPTR *rplMatrixFastGetEx(WORDPTR *first,BINT cols,BINT i,BINT j);
WORDPTR *rplMatrixNewEx(BINT rows,BINT cols);
void rplMatrixNorm();
void rplMatrixNeg();
void rplMatrixConj();
void rplMatrixEval1();
void rplMatrixEval();
void rplMatrixToNum();
void rplMatrixSame();
void rplMatrixAdd();
void rplMatrixSub();
void rplMatrixMul();
void rplMatrixMulScalar();
void rplMatrixDivScalar();
void rplMatrixTranspose();
void rplMatrixHadamard();
void rplMatrixReduce();
WORDPTR rplMatrixInitIdx(BINT nrows);
BINT rplMatrixBareissEx(WORDPTR *a, WORDPTR *index, BINT rowsa, BINT colsa, BINT upperonly);
void rplMatrixInvert();
void rplMatrixBackSubstEx(WORDPTR *a,BINT rowsa,BINT colsa);
BINT rplMatrixIsPolar(WORDPTR matobj);
BINT rplIsZeroMatrix(WORDPTR object);
void rplMatrixPolarToRectEx(WORDPTR *a,BINT rowsa,BINT colsa);
void rplMatrixRectToPolarEx(WORDPTR *a,BINT rowsa,BINT colsa,WORD angtemplate,BINT angmode);



// NUMERIC SOLVERS

WORDPTR rplPolyEvalEx(WORDPTR *first,BINT degree,WORDPTR *value);
WORDPTR rplPolyEvalDerivEx(BINT deriv,WORDPTR *first,BINT degree,WORDPTR *value);
WORDPTR rplPolyRootEx(WORDPTR *first,BINT degree);
WORDPTR rplPolyDeflateEx(WORDPTR *first,BINT degree,WORDPTR *value);

// RANDOM NUMBER GENERATOR
void rplRandomSeed(UBINT64 seed);
void rplRandomJump(void);
UBINT64 rplRandomNext(void);
BINT rplRandom8Digits();




// DATE AND TIME FUNCTIONS
BINT rplReadRealAsDate(REAL *date, struct date *dt);
BINT rplReadRealAsDateNoCk(REAL *date, struct date *dt);
BINT rplReadDateAsReal(struct date dt, REAL *date);
BINT rplGetMonthDays(BINT month, BINT year);
BINT rplIsValidDate(struct date dt);
BINT rplReadRealAsTime(REAL *time, struct time *tm);
BINT rplReadTimeAsReal(struct time tm, REAL *time);
BINT rplDateToDays(struct date dt);
struct date rplDaysToDate(BINT days);
BINT64 rplDateToSeconds(struct date dt, struct time tm);
void rplSecondsToDate(BINT64 sec, struct date *dt, struct time *tm);
void rplDecimalToHMS(REAL *dec, REAL *hms);
void rplHMSToDecimal(REAL *hms, REAL *dec);


// ALARM FUNCTIONS
BINT rplReadAlarm(WORDPTR obj, struct alarm *alrm);
void rplPushAlarm(struct alarm *alrm);
BINT rplAddAlarm(struct alarm *alrm);
BINT rplGetAlarm(BINT id, struct alarm *alrm);
BINT rplDelAlarm(BINT id);
BINT rplCheckAlarms();
BINT rplTriggerAlarm();
void rplUpdateAlarms();
void rplSkipNextAlarm();

// KEYBOARD FUNCTIONS
WORDPTR rplMsg2KeyName(BINT keymsg);
BINT rplKeyName2Msg(WORDPTR keyname);

// FONT FUNCTIONS
WORDPTR rplGetCurrentFont(BINT area);
void rplSetCurrentFont(BINT area,WORDPTR ident);
void rplUpdateFontArray(WORDPTR **fontarray);
void rplAddSystemFont(WORDPTR ident,WORDPTR font);
void rplPurgeSystemFont(WORDPTR ident);
WORDPTR rplGetSystemFont(WORDPTR ident);
WORDPTR rplGetSystemFontName(WORDPTR font);



// ANGULAR MODES
#define ANGLENONE    -1
#define ANGLEDEG     0
#define ANGLERAD     1
#define ANGLEGRAD    2
#define ANGLEDMS     3

// COMPLEX NUMBER CLASSES
#define CPLX_NORMAL     0
#define CPLX_ZERO       1
#define CPLX_NAN        2
#define CPLX_INF        4
#define CPLX_UNDINF     8
#define CPLX_POLAR     16
#define CPLX_MALFORMED 32



// SYSTEM FLAGS
#define FL_NOCUSTOMKEYS   -4
// WORDSIZE = FLAGS -5 TO -10 INCLUSIVE
#define FL_ACTIVEMENU     -11
#define FL_LASTMENU       -12
#define FL_HIDEMENU2      -13

#define FL_MENU1WHITE     -15
#define FL_MENU2WHITE     -16
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
#define FL_TIMEFORMAT     -41
#define FL_DATEFORMAT     -42
#define FL_RESRPTALRM     -43
#define FL_SAVACKALRM     -44
#define FL_ERRORBEEP      -56
#define FL_ALARMBEEP      -57
#define FL_COMPLEXMODE    -103
#define FL_AUTOINDENT     -59
#define FL_INDEXWRAP      -64










// DEFINED EXCEPTIONS

#define EX_EXITRPL          1
#define EX_HALT             2
#define EX_HWHALT           4
#define EX_OUTOFMEM         8
#define EX_ERRORCODE       16
#define EX_POWEROFF        32
#define EX_DIRTYFS         64
#define EX_AUTORESUME     128
#define EX_HWBKPOINT      256
#define EX_HWBKPTSKIP     512
#define EX_TIMER          1024
#define EX_ALARM          2048
#define EX_HALRESET       4096

// ADD OTHER EXCEPTIONS HERE

// BREAKPOINT FLAGS

#define BKPT_ENABLED    1
#define BKPT_LOCATION   2 // TRIGGER ONLY WHEN IPtr==BreakPtArg
#define BKPT_COND       4 // XEQ RPL OBJECT AT BreakPtArg, IF IT RETURNS TRUE THEN TRIGGER
#define BKPT_PAUSED     8 // DO NOT TRIGGER BREAKPOINT WHILE PAUSED. INTERNAL USE DURING CONDITION EXECUTION
#define BKPT_ALLPAUSED 0x08080808 // INTERNAL USE MASK
#define SET_BKPOINTFLAG(n,flags) BreakPtFlags=(BreakPtFlags & ~(0xff<<(8*((n)))))|( ((flags)&0xff)<<(8*((n))))
#define GET_BKPOINTFLAG(n) ((BreakPtFlags>>(8*((n))))&0xff)




// GARBAGE COLLECTOR STATUS FLAGS

#define GC_IN_PROGRESS 1
#define GC_COMPLETED   2
#define GC_ERROR       4

// START OF PLOTTING COMMANDS ON A RENDER LIBRARY
#define CMD_PLTBASE     0x4000

// RESET THE ENGINE PRIOR TO START A NEW JOB
#define CMD_PLTRESET    0x4001
// DEFINE THE TARGET CANVAS SIZE
#define CMD_PLTRENDERSIZE 0x4002

// PLOTTING ENGINE COMMANDS
//'w' SETCANVAS: X Y ->
#define PLT_SETSIZE 'w'
//'k' STROKECOLOR: C ->
#define PLT_STROKECOL 'k'
//'e' STROKETYPE: T ->
#define PLT_STROKETYPE 'e'
//'i' FILLCOLOR: C ->
#define PLT_FILLCOL 'i'
//'j' FILLTYPE: T ->
#define PLT_FILLTYPE 'j'
//'m' MOVETO: X Y ->
#define PLT_MOVETO  'm'
//'l' LINETO: X Y ->
#define PLT_LINETO  'l'
//'z' LINECLOSE: ->
#define PLT_LCLOSE  'z'
//'c' CIRCLE: R ->
#define PLT_CIRCLE  'c'
//'r' RECTANGLE: X Y ->
#define PLT_RECTANG 'r'
//'n' CONTROLNODE: X Y ->
#define PLT_CTLNODE 'n'
//'p' CURVE: X Y ->
#define PLT_CURVE   'p'
//'f' FILL
#define PLT_FILL    'f'
//'g' STROKE
#define PLT_STROKE  'g'
//'h' FILL&STROKE
#define PLT_FILLSTROKE 'h'
//'{' BEGINGROUP
#define PLT_BGROUP  '{'
//'}' ENDGROUP
#define PLT_EGROUP  '}'
// 'b' BASE POINT: X Y ->
#define PLT_BASEPT  'b'
//'t' TRANSLATE: X Y ->
#define PLT_TRANS   't'
//'q' ROTATE: ANG ->
#define PLT_ROTATE  'q'
//'s' SCALE: SX SY ->
#define PLT_SCALE   's'
//'u' CLRTRANSFORM
#define PLT_CLRTRANSFORM    'u'
//'a' REPEATGROUP
#define PLT_DOGROUP 'a'
//'v' SETFONT: F -> (F IS SYSTEM FONT INDEX NUMBER OR FONT NAME)
#define PLT_TXTFONT 'v'
//'x' TEXTHEIGHT: H ->
#define PLT_TXTHEIGHT   'x'
//'o' TEXTOUT: S ->
#define PLT_TXTOUT  'o'
//'~' ENDOFPLOT
#define PLT_ENDOFPLOT '~'

#ifdef __cplusplus
}
#endif


#include "arithmetic.h"

#endif // NEWRPL_H
