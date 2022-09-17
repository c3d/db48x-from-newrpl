/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef NEWRPL_H
#define NEWRPL_H

#include <stdlib.h>
#include <string.h>
#include "newrpl_types.h"
#include <firmware.h>
#include "utf8lib.h"
#include "arithmetic.h"
#include "decimal.h"
#include "fastmath.h"
#include "sysvars.h"
#include "recorder.h"

/* The following is used to silence warnings for unused variables */
#define UNUSED(var)		do { (void)(var); } while(0)

// BUILD SYSTEM PROVIDES NEWRPL_BUILDNUM MACRO WITH THE COMMIT NUMBER

// EXTERNAL API FOR THE NEWRPL MACHINE - TO BE USED ONLY BY USER LIBRARIES
// BASIC CONSTANTS AND TYPE DEFINITIONS FOR THE RUN ENVIRONMENT

#ifdef __cplusplus
extern "C"
{
#endif

// stdlib wrappers
static inline void memcpyw(void *dst, const void *src, size_t nwords)
{
    WORD *dstw = (WORD *) dst;
    const WORD *srcw = (const WORD *) src;
    while (nwords--)
        *dstw++ = *srcw++;
}

static inline void memmovew(void *dst, const void *src, size_t nwords)
{
    memmove(dst, src, nwords * sizeof(WORD));
}

static inline size_t stringlen(const char *s)
{
    return strlen(s);
}

static inline char *stringcpy(char *dst, const char *src)
{
    return strcpy(dst, src);
}

static inline void *memcpyb(void *dst, const void *src, size_t n)
{
    return memcpy(dst, src, n);
}

static inline void *memmoveb(void *dst, const void *src, size_t n)
{
    return memmove(dst, src, n);
}

static inline void memsetw(void *dst, WORD value, size_t nwords)
{
    WORD *dstw = (WORD *) dst;
    while(nwords--)
        *dstw++ = value;
}

static inline void *memsetb(void *dst, int value, size_t nbytes)
{
    return memset(dst, value, nbytes);
}


// Bit manipulation macros
#define BIT(n)          (1U << (n))
#define BITS(n,m)       (((1U << ((m) + 1 - (n))) - 1) << (n))


// FORMATTING FOR NUMBERS
    typedef struct
    {
        uint64_t Locale;
        int32_t SmallFmt;
        int32_t MiddleFmt;
        int32_t BigFmt;
        REAL SmallLimit;
        REAL BigLimit;
        int32_t SmallLimitData[4];
        int32_t BigLimitData[4];
    } NUMFORMAT;

    typedef union
    {
        WORD word;
        struct
        {
            signed exp:16;
            unsigned len:12, flags:4;
        };
    } REAL_HEADER;

// COMPACT TIME STRUCTURE - 32 BITS.
struct time
{
    unsigned sec:6, // seconds after the minute 0-59
        min:6,      // minutes after the hour   0-59
        hour:5,     // hours since midnight     0-23
        isdst:1,    // daylight saving time flag
    :   14; // to pad up to 32 bits
};

// COMPACT DATE STRUCTURE - 32 BITS.
struct date
{
    unsigned mday:5,        // day of the month     1-31
        mon:4,      // months since January     1-12
        wday:3,     // days since Monday        1-7
        year:14,    // years                1582-9999
    :   6;  // to pad up to 32 bits
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
    struct alarm
    {
        int64_t time;    // alarm time (elapsed seconds since 10/15/1582)
        uint32_t rpt;      // repeat interval (seconds)
        WORDPTR obj;
        union
        {
            BYTE flags; // Represents all alarm flags.
            struct
            {
                unsigned ann:1, // announced alarm?       Y:1 N:0
                    ack:1,      // acknowledged alarm?    Y:1 N:0
                    dis:1,      // disabled alarm?        Y:1 N:0
                :   5;  // to pad up to 8 bits
            };
        };
    };

#define DUE_ALM      0x0
#define PASTDUE_ALM  0x1
#define PAST_ALM     0x2
#define PAST_CTLALM  0x3
#define DISABLED_ALM 0x4

// ERROR MANAGEMENT FUNCTIONS
    void decTrapHandler(int32_t error);
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
    void rplInitMemoryAllocator();
    void rplWarmInit();
    void rplHotInit();
    void rplSetEntryPoint(WORDPTR ip);
    int32_t rplRun();
    int32_t rplRunAtomic(WORD opcode);
    void rplCleanup();
    void rplDisableSingleStep();
    void rplEnableSingleStep();
    void rplResetSystemFlags();

// LIBRARY MANAGEMENT
    int32_t rplInstallLibrary(LIBHANDLER handler);
    void rplRemoveLibrary(int32_t number);

// LIBRARY LOW-LEVEL ACCESS FUNCTIONS
    LIBHANDLER rplGetLibHandler(int32_t libnum);
    int32_t rplGetNextLib(int32_t libnum);

// BASIC GENERIC OBJECT FUNCTIONS
    WORDPTR rplSkipOb(WORDPTR ip);
    void rplSkipNext();
    WORD rplObjSize(WORDPTR ip);

// LOW-LEVEL MEMORY MANAGEMENT

    int32_t rplGetFreeMemory();
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

    WORDPTR rplCompile(BYTEPTR string, int32_t len, int32_t addwrapper);
    void rplCompileAppend(WORD word);
    void rplCompileInsert(WORDPTR position, WORD word);
    WORDPTR rplCompileAppendWords(int32_t nwords);
    void rplCompileRemoveWords(int32_t nwords);

#define DECOMP_EMBEDDED     1
#define DECOMP_EDIT         2
#define DECOMP_NOHINTS      4
#define DECOMP_MAXWIDTH(n) (((n)&0xfff)<<4)
#define DECOMP_GETMAXWIDTH(f) (((f)>>4)&0xfff)

#define DEFAULT_DECOMP_WIDTH 20

// DECOMPILER FUNCTIONS
    WORDPTR rplDecompile(WORDPTR object, int32_t flags);
    void rplDecompAppendChar(BYTE c);
    void rplDecompAppendUTF8(WORD utf8bytes);
    void rplDecompAppendString(BYTEPTR str);
    void rplDecompAppendString2(BYTEPTR str, int32_t len);
    int32_t rplDecompDoHintsWidth(int32_t dhints);

// DATA STACK FUNCTIONS IN DATASTACK.C

    void rplPushData(WORDPTR p);
    void rplPushDataNoGrow(WORDPTR p);
    WORDPTR rplPopData();
    WORDPTR rplPeekData(int level);
    void rplOverwriteData(int level, WORDPTR ptr);
    int32_t rplDepthData();
    void rplClearData();
    void rplDropData(int n);
    void rplRemoveAtData(int32_t level, int32_t num);
    void rplExpandStack(int32_t numobjects);
    void growDStk(WORD newsize);
    WORDPTR *rplProtectData();
    WORDPTR *rplUnprotectData();

// SNAPSHOT FUNCTIONS THAT SAVE/RESTORE THE STACK
    int32_t rplCountSnapshots();
    void rplTakeSnapshot();
    void rplRemoveSnapshot(int32_t numsnap);
    void rplRestoreSnapshot(int32_t numsnap);
    void rplRevertToSnapshot(int32_t numsnap);
    void rplTakeSnapshotN(int32_t nargs);
    void rplTakeSnapshotHide(int32_t nargs);
    void rplTakeSnapshotAndClear();
    int32_t rplDepthSnapshot(int32_t numsnap);
    WORDPTR rplPeekSnapshot(int32_t numsnap, int32_t level);
    void rplDropCurrentStack();
    void rplCleanupSnapshots(WORDPTR * newstkbottom);

// RETURN STACK FUNCTIONS IN RETURNSTACK.C

    void rplPushRet(WORDPTR p);
    void rplPushRetNoGrow(WORDPTR p);
    WORDPTR rplPopRet();
    void rplDropRet(int nlevels);
    void growRStk(WORD newsize);
    WORDPTR rplPeekRet(int level);
    void rplClearRStk();
    int32_t rplDepthRet();

// SYSTEM FLAGS
    int32_t rplSetSystemFlag(int32_t flag);
    int32_t rplSetSystemFlagByName(BYTEPTR name, BYTEPTR nameend);
    int32_t rplSetSystemFlagByIdent(WORDPTR ident);
    int32_t rplClrSystemFlag(int32_t flag);
    int32_t rplClrSystemFlagByName(BYTEPTR name, BYTEPTR nameend);
    int32_t rplClrSystemFlagByIdent(WORDPTR ident);
    int32_t rplTestSystemFlag(int32_t flag);
    int32_t rplTestSystemFlagByName(BYTEPTR name, BYTEPTR nameend);
    int32_t rplTestSystemFlagByIdent(WORDPTR ident);

// USER FLAGS
    int32_t rplSetUserFlag(int32_t flag);
    int32_t rplClrUserFlag(int32_t flag);
    int32_t rplTestUserFlag(int32_t flag);
    uint64_t *rplGetUserFlagsLow();

    uint64_t rplGetSystemLocale();
    void rplGetSystemNumberFormat(NUMFORMAT * fmt);
    void rplSetSystemNumberFormat(NUMFORMAT * fmt);

// SYSTEM SOFT MENUS
    void rplSetMenuCode(int32_t menunumber, int64_t menucode);
    int64_t rplGetMenuCode(int32_t menunumber);
    void rplSetActiveMenu(int32_t menunumber);
    int32_t rplGetActiveMenu();
    void rplChangeMenu(int32_t menu, WORDPTR newmenu);
    void rplSetLastMenu(int32_t menunumber);
    int32_t rplGetLastMenu();
    WORDPTR rplPopMenuHistory(int32_t menu);
    void rplSaveMenuHistory(int32_t menu);

// SYSTEM AUTOCOMPLETE
    WORD rplGetNextSuggestion(WORD suggestion, WORDPTR suggobject,
            BYTEPTR start, BYTEPTR end);
    WORD rplGetPrevSuggestion(WORD suggestion, WORDPTR suggobject,
            BYTEPTR start, BYTEPTR end);
    WORD rplUpdateSuggestion(WORD suggestion, WORDPTR suggobject, BYTEPTR start,
            BYTEPTR end);

// GARBAGE COLLECTION
    void rplGCollect();

// BACKUP/RESTORE
    int32_t rplBackup(int (*writefunc)(unsigned int, void *), void *OpaqueArg);
    int32_t rplRestoreBackup(int32_t includestack, WORD(*readfunc) (void *),
            void *Opaque);

// SYSTEM SANITY CHECKS
    int32_t rplVerifyObject(WORDPTR obj);
    int32_t rplIsTempObPointer(WORDPTR ptr);
    int32_t rplVerifyObjPointer(WORDPTR ptr);
    int32_t rplVerifyDStack(int32_t fix);
    int32_t rplVerifyRStack();
    int32_t rplVerifyTempOb(int32_t fix);
    int32_t rplVerifyDirectories(int32_t fix);

//IDENTIFIER FUNCTIONS
    int32_t rplGetIdentLength(WORDPTR ident);
    void rplCompileIDENT(int32_t libnum, BYTEPTR tok, BYTEPTR tokend);
    WORDPTR rplCreateIDENT(int32_t libnum, BYTEPTR tok, BYTEPTR tokend);
    int32_t rplIsValidIdent(BYTEPTR tok, BYTEPTR tokend);
    WORD rplGetIdentAttr(WORDPTR name);
    WORDPTR rplSetIdentAttr(WORDPTR name, WORD attr, WORD attrmask);
    WORD rplGetIdentProp(WORDPTR ident);
    int32_t rplDecodeAttrib(BYTEPTR st, BYTEPTR end);

// LAM FUNCTIONS
    void growLAMs(WORD newtotalsize);
    void rplCreateLAMEnvironment(WORDPTR owner);
    void rplDupLAMEnv();
    int32_t rplCreateLAM(WORDPTR nameobj, WORDPTR value);
    int32_t rplCompareIDENT(WORDPTR id1, WORDPTR id2);
    int32_t rplCompareIDENTByName(WORDPTR id1, BYTEPTR name, BYTEPTR nameend);
    int32_t rplCompareObjects(WORDPTR id1, WORDPTR id2);
    WORDPTR rplGetLAM(WORDPTR nameobj);
    WORDPTR *rplGetLAMn(int32_t idx);
    WORDPTR *rplGetLAMnName(int32_t idx);
    WORDPTR *rplGetLAMnEnv(WORDPTR * LAMEnv, int32_t idx);
    WORDPTR *rplGetLAMnNameEnv(WORDPTR * LAMEnv, int32_t idx);
    void rplPutLAMn(int32_t idx, WORDPTR object);
    void rplCleanupLAMs(WORDPTR currentseco);
    void rplClearLAMs();
    WORDPTR *rplFindLAM(WORDPTR nameobj, int32_t scanparents);
    WORDPTR *rplFindLAMbyName(BYTEPTR name, int32_t len, int32_t scanparents);
    WORDPTR *rplGetNextLAMEnv(WORDPTR * startpoint);
    int32_t rplNeedNewLAMEnv();
    int32_t rplNeedNewLAMEnvCompiler();
    int32_t rplLAMCount(WORDPTR * LAMEnvironment);

// GLOBAL VARIABLES AND DIRECTORY FUNCTIONS
    void growDirs(WORD newtotalsize);
    WORDPTR rplMakeIdentQuoted(WORDPTR ident);
    WORDPTR rplMakeIdentUnquoted(WORDPTR ident);
    WORDPTR rplMakeIdentHidden(WORDPTR ident);
    WORDPTR rplMakeIdentVisible(WORDPTR ident);
    WORDPTR rplMakeIdentReadOnly(WORDPTR ident);
    WORDPTR rplMakeIdentWriteable(WORDPTR ident);
    WORDPTR rplMakeIdentNoProps(WORDPTR ident);

    void rplCreateGlobalInDir(WORDPTR nameobj, WORDPTR value,
            WORDPTR * parentdir);
    WORDPTR *rplCreateNGlobalsInDir(int32_t n, WORDPTR * parentdir);
    void rplCreateGlobal(WORDPTR nameobj, WORDPTR value);
    void rplPurgeGlobal(WORDPTR nameobj);
    WORDPTR *rplFindDirbyHandle(WORDPTR handle);
    WORDPTR *rplFindDirFromPath(WORDPTR pathlist, int32_t uselastname);
    WORDPTR rplCreateNewDir(WORDPTR nameobj, WORDPTR * parentdir);
    void rplPurgeDir(WORDPTR nameobj);
    void rplPurgeDirByHandle(WORDPTR handle);
    WORDPTR *rplGetParentDir(WORDPTR * directory);
    int32_t rplGetDirSize(WORDPTR * directory);
    void rplPackDirinPlace(WORDPTR * directory, WORDPTR where);
// VARIOUS WAYS TO RCL GLOBAL VARIABLES
    WORDPTR *rplFindGlobalbyNameInDir(BYTEPTR name, BYTEPTR nameend,
            WORDPTR * parent, int32_t scanparents);
    WORDPTR *rplFindGlobalbyName(BYTEPTR name, BYTEPTR nameend,
            int32_t scanparents);
    WORDPTR *rplFindGlobalByIndexInDir(int32_t idx, WORDPTR * directory);
    WORDPTR *rplFindGlobalByIndex(int32_t idx);
    WORDPTR *rplFindGlobalInDir(WORDPTR nameobj, WORDPTR * parentdir,
            int32_t scanparents);
    WORDPTR *rplFindGlobal(WORDPTR nameobj, int32_t scanparents);
    WORDPTR *rplFindVisibleGlobalByIndexInDir(int32_t idx, WORDPTR * directory);
    WORDPTR *rplFindVisibleGlobalByIndex(int32_t idx);
    WORDPTR *rplFindGlobalPropInDir(WORDPTR nameobj, WORD propname,
            WORDPTR * parent, int32_t scanparents);
// DIRECTORY SCANNING AND LOWER-LEVEL ACCESS
    WORDPTR *rplFindFirstInDir(WORDPTR * directory);
    WORDPTR *rplFindFirstByHandle(WORDPTR dirhandle);
    WORDPTR *rplFindNext(WORDPTR * direntry);
    int32_t rplGetVarCountInDir(WORDPTR * directory);
    int32_t rplGetVarCount();
    int32_t rplGetVisibleVarCountInDir(WORDPTR * directory);
    int32_t rplGetVisibleVarCount();
    int32_t rplIsVarVisible(WORDPTR * var);
    int32_t rplIsVarReadOnly(WORDPTR * var);
    int32_t rplIsVarDirectory(WORDPTR * var);
    int32_t rplIsVarEmptyDir(WORDPTR * var);

    WORDPTR rplGetGlobal(WORDPTR nameobj);
    WORDPTR *rplMakeNewDir();
    WORDPTR rplGetDirName(WORDPTR * dir);
    int32_t rplGetFullPath(WORDPTR * dir, WORDPTR * buffer, int32_t maxdepth);
    WORDPTR *rplGetDirfromGlobal(WORDPTR * var);
    WORDPTR *rplDeepCopyDir(WORDPTR * sourcedir);
    void rplWipeDir(WORDPTR * directory);
    void rplPurgeForced(WORDPTR * var);

// FUNCTIONS SPECIFIC FOR THE .Settings DIRECTORY
    void rplPurgeSettings(WORDPTR nameobj);
    void rplStoreSettings(WORDPTR nameobject, WORDPTR object);
    void rplStoreSettingsbyName(BYTEPTR name, BYTEPTR nameend, WORDPTR object);
    WORDPTR rplGetSettings(WORDPTR nameobject);
    WORDPTR rplGetSettingsbyName(BYTEPTR name, BYTEPTR nameend);

// AUTOMATIC EVALUATION
    void rplDoAutoEval(WORDPTR varname, WORDPTR * indir);
    void rplUpdateDependencyTree(WORDPTR varname, WORDPTR * dir,
            WORDPTR olddefn, WORDPTR newdefn);

// SOLVERS AUXILIARY FUNCTIONS
    void rplEvalUserFunc(WORDPTR arg_userfunc, WORD Opcode);
    void rplEvalMultiUserFunc(WORDPTR * listofeq, WORDPTR * listofvars,
            int32_t nvars, int32_t minimizer);

// GENERIC OBJECT FUNCTIONS
    void rplCallOvrOperator(WORD op);
    void rplCallOperator(WORD op);
    void rplCopyObject(WORDPTR dest, WORDPTR src);
    WORDPTR rplMakeNewCopy(WORDPTR object);
    int64_t rplObjChecksum(WORDPTR object);

// int32_t FUNCTIONS
    WORDPTR rplNewSINT(int num, int base);
    WORDPTR rplNewint32_t(int64_t num, int base);
    void rplNewSINTPush(int num, int base);
    void rplNewint32_tPush(int64_t num, int base);
    int64_t rplReadint32_t(WORDPTR ptr);
    WORDPTR rplWriteint32_t(int64_t num, int base, WORDPTR dest);
    void rplCompileint32_t(int64_t num, int base);

// TRUE/FALSE FUNCTIONS
    void rplPushFalse();
    void rplPushTrue();
    int32_t rplIsFalse(WORDPTR objptr);
    int32_t rplIsTrue(WORDPTR objptr);

// REAL FUNCTIONS
    void rplOneToRReg(int num);
    void rplZeroToRReg(int num);
    void rplInfinityToRReg(int num);
    void rplUndInfinityToRReg(int num);
    void rplNANToRReg(int num);
    void rplint32_tToRReg(int num, int64_t value);
    void rplReadReal(WORDPTR real, REAL * dec);
    int32_t rplReadRealFlags(WORDPTR object);
    int32_t rplIsNumberZero(WORDPTR obj);
    void rplCopyRealToRReg(int num, WORDPTR real);
    WORDPTR rplNewReal(REAL * num);
    WORDPTR rplNewRealInPlace(REAL * num, WORDPTR addr);
    WORDPTR rplNewRealFromRReg(int num);
    void rplNewRealPush(REAL * num);
    void rplNewRealFromRRegPush(int num);
    void rplNewApproxRealFromRRegPush(int num);
    WORDPTR rplRRegToRealInPlace(int num, WORDPTR dest);
    void rplCheckResultAndError(REAL * real);
    void rplCompileReal(REAL * num);

// COMPLEX FUNCTIONS
    void rplRealPart(WORDPTR complex, REAL * real);
    void rplImaginaryPart(WORDPTR complex, REAL * imag);
    int32_t rplPolarComplexMode(WORDPTR complex);
    int32_t rplComplexClass(WORDPTR complex);
    void rplReadCNumber(WORDPTR complex, REAL * real, REAL * imag,
            int32_t * angmode);
    void rplReadCNumberAsReal(WORDPTR complex, REAL * real);
    void rplReadCNumberAsImag(WORDPTR complex, REAL * imag);
    WORDPTR rplNewComplex(REAL * real, REAL * imag, int32_t angmode);
    void rplNewComplexPush(REAL * real, REAL * imag, int32_t angmode);
    void rplRRegToComplexPush(int32_t real, int32_t imag, int32_t angmode);
    WORDPTR rplRRegToComplexInPlace(int32_t real, int32_t imag, WORDPTR dest,
            int32_t angmode);
    void rplRect2Polar(REAL * re, REAL * im, int32_t angmode);
    void rplPolar2Rect(REAL * r, REAL * theta, int32_t angmode);
    int32_t rplIsZeroComplex(REAL * re, REAL * im, int32_t angmode);
    int rplNormalizeComplex(REAL * real, REAL * imag, int32_t angmode);

// GENERIC FUNCTIONS FOR int32_tS AND REALS
    void rplNumberToRReg(int num, WORDPTR number);
    int64_t rplReadNumberAsInt64(WORDPTR number);
    void rplReadNumberAsReal(WORDPTR number, REAL * dec);
    void rplLoadInt64AsReal(int64_t number, REAL * dec);
    int32_t rplIsNegative(WORDPTR objptr);
    int32_t rplIntToString(int64_t number, int32_t base, BYTEPTR buffer,
            BYTEPTR endbuffer);

// CONSTANTS
    WORDPTR rplConstant2Number(WORDPTR object);
    int32_t rplConstant2NumberDirect(WORDPTR object);

// ANGLE FUNCTIONS
    WORDPTR rplNewAngleFromReal(REAL * number, int32_t newmode);
    WORDPTR rplNewAngleFromNumber(WORDPTR numobj, int32_t newmode);
    void rplConvertAngleObj(WORDPTR angleobj, int32_t newmode);

// UNIT FUNCTIONS
    int32_t rplUnitExplode(WORDPTR unitobj);
    WORDPTR rplUnitAssemble(int32_t nlevels);
    int32_t rplUnitPopItem(int32_t level);
    void rplUnitPickItem(int32_t level);
    int32_t rplUnitMulItem(int32_t level1, int32_t level2);
    void rplUnitPowItem(int32_t level1, int32_t level2);
    int32_t rplUnitSkipItem(int32_t level);
    int32_t rplUnitSimplify(int32_t nlevels);
    int32_t rplUnitDivide(int32_t numlvl, int32_t divlvl);
    void rplUnitInvert(int32_t level);
    int32_t rplUnitExpand(int32_t level);
    int32_t rplUnitToBase(int32_t nlevels);
    int32_t rplUnitSort(int32_t nlevels, int32_t reflevel);
    int32_t rplUnitIsConsistent(int32_t nlevels, int32_t reflevel);
    int32_t rplUnitPow(int32_t lvlexp, int32_t nlevels);

    int32_t rplUnitIsSpecial(WORDPTR unitobj);
    void rplUnitReplaceSpecial(int32_t nlevels);
    void rplUnitReverseReplaceSpecial(int32_t nlevels);
    void rplUnitReverseReplaceSpecial2(int32_t isspec_idx);
    void rplUnitSpecialToDelta(int32_t nlevels);
    WORDPTR *rplUnitFindCustom(WORDPTR ident, int32_t * siindex);
    void rplUnitUnaryDoCmd();
    int32_t rplUnitIsNonDimensional(WORDPTR uobject);
    void rplUnitUnaryDoCmdNonDimensional();
    WORDPTR rplUnitApply(WORDPTR value, WORDPTR unitobj);

// LIST FUNCTIONS
    int32_t rplListLength(WORDPTR composite);
    int32_t rplListLengthFlat(WORDPTR composite);
    void rplCreateList();
    WORDPTR rplCreateListN(int32_t num, int32_t level, int32_t remove);
    void rplListAutoExpand(WORDPTR list);
    int32_t rplExplodeList(WORDPTR composite);
    int32_t rplExplodeList2(WORDPTR composite);
    WORDPTR rplGetListElement(WORDPTR composite, int32_t pos);
    WORDPTR rplGetListElementFlat(WORDPTR composite, int32_t pos);
    WORDPTR rplGetNextListElementFlat(WORDPTR composite, WORDPTR elem);
    int32_t rplIsLastElementFlat(WORDPTR composite, int32_t pos);
    int32_t rplListSame();
    void rplListUnaryDoCmd();
    void rplListUnaryNoResultDoCmd();
    void rplListUnaryNonRecursiveDoCmd();
    void rplListBinaryDoCmd();
    void rplListBinaryNoResultDoCmd();
    void rplListMultiArgDoCmd(int32_t nargs);
    WORDPTR rplListAddRot(WORDPTR list, WORDPTR object, int32_t nmax);
    WORDPTR rplListReplace(WORDPTR list, int32_t position, WORDPTR object);
    WORDPTR rplListReplaceMulti(WORDPTR list, int32_t position, WORDPTR object);
    void rplListExpandCases();
    int32_t rplListHasLists(WORDPTR list);

// SYMBOLIC FUNCTIONS
    WORDPTR rplSymbUnwrap(WORDPTR symbolic);
    WORDPTR rplSymbWrap(WORDPTR obj);
    void rplSymbWrapN(int32_t level, int32_t nargs);
    WORD rplSymbMainOperator(WORDPTR symbolic);
    WORDPTR rplSymbMainOperatorPTR(WORDPTR symbolic);
    int32_t rplIsAllowedInSymb(WORDPTR object);
    int32_t rplSymbGetTokenInfo(WORDPTR object);
    void rplSymbApplyOperator(WORD Opcode, int32_t nargs);
    int32_t rplSymbRuleMatch();
    int32_t rplSymbGetAttr(WORDPTR object);
    WORDPTR rplComplexToSymb(WORDPTR complex);

    void rplSymbRuleApply();
    int32_t rplSymbIsRule(WORDPTR ptr);
    void rplSymbAutoSimplify();
    WORDPTR rplSymbNumericReduce(WORDPTR object);
    int32_t rplSymbIsNumeric(WORDPTR ptr);
    int32_t rplSymbIsZero(WORDPTR ptr);
    void rplSymbNumericCompute();

// INTERNAL SYMBOLIC API, FOR USE BY OTHER LIBRARIES
    int32_t rplCheckCircularReference(WORDPTR env_owner, WORDPTR object,
            int32_t lamnum);
    int32_t rplFractionSimplify();
    int32_t rplFractionAdd();
    int32_t rplSymbExplode(WORDPTR object);
    WORDPTR rplSymbImplode(WORDPTR * exprstart);
    WORDPTR rplSymbCanonicalForm(WORDPTR object, int32_t fordisplay);
    int32_t rplSymbExplodeOneLevel(WORDPTR object);
    WORDPTR rplSymbReplaceVar(WORDPTR symb, WORDPTR findvar, WORDPTR newvar);

// STRINGS
// RPL STRING OBJECT
    void rplSetStringLength(WORDPTR string, int32_t length);
    int32_t rplStrLen(WORDPTR string);
    int32_t rplStrLenCp(WORDPTR string);
    int32_t rplStrSize(WORDPTR string);
    int32_t rplStringGetLinePtr(WORDPTR str, int32_t line);
    int32_t rplStringGetNextLine(WORDPTR str, int32_t prevlineoff);

    int32_t rplStringCountLines(WORDPTR str);
    int32_t rplStringCompare(WORDPTR str1, WORDPTR str2);
    WORDPTR rplCreateString(BYTEPTR text, BYTEPTR textend);
    WORDPTR rplCreateStringBySize(int32_t lenbytes);

// MATRIX
    WORDPTR rplMatrixCompose(int32_t rows, int32_t cols);
    WORDPTR rplMatrixComposeN(int32_t level, int32_t rows, int32_t cols);
    WORDPTR rplMatrixFlexComposeN(int32_t level, int32_t totalelements);
    WORDPTR rplMatrixFill(int32_t rows, int32_t cols, WORDPTR obj);
    WORDPTR rplMatrixIdent(int32_t rows);

    int32_t rplMatrixIsAllowed(WORDPTR object);

    WORDPTR *rplMatrixExplode();
    WORDPTR *rplMatrixExplodeByCols();
    WORDPTR rplMatrixGetFirstObj(WORDPTR matrix);
    int32_t rplMatrixCols(WORDPTR matrix);
    int32_t rplMatrixRows(WORDPTR matrix);
    WORDPTR rplMatrixGet(WORDPTR matrix, int32_t row, int32_t col);
    WORDPTR rplMatrixFastGet(WORDPTR matrix, int32_t row, int32_t col);
    WORDPTR *rplMatrixFastGetEx(WORDPTR * first, int32_t cols, int32_t i, int32_t j);
    WORDPTR rplMatrixFastGetFlat(WORDPTR matrix, int32_t index);
    WORDPTR *rplMatrixNewEx(int32_t rows, int32_t cols);
    void rplMatrixNorm();
    void rplMatrixNeg();
    void rplMatrixNegPolar();
    void rplMatrixConj();
    void rplMatrixEval1();
    void rplMatrixEval();
    void rplMatrixToNum();
    void rplMatrixSame();
    void rplMatrixEqual();
    void rplMatrixAdd();
    void rplMatrixAddPolar(int32_t negv2);
    void rplMatrixSub();
    void rplMatrixMul();
    void rplMatrixMulScalar();
    void rplMatrixDivScalar();
    void rplMatrixTranspose();
    void rplMatrixHadamard();
    void rplMatrixReduce();
    WORDPTR rplMatrixInitIdx(int32_t nrows);
    int32_t rplMatrixBareissEx(WORDPTR * a, WORDPTR * index, int32_t rowsa,
            int32_t colsa, int32_t upperonly);
    void rplMatrixInvert();
    void rplMatrixBackSubstEx(WORDPTR * a, int32_t rowsa, int32_t colsa);
    int32_t rplMatrixIsPolar(WORDPTR matobj);
    WORD rplMatrixPolarGetTemplate(WORDPTR matrix);
    int32_t rplIsZeroMatrix(WORDPTR object);
    void rplMatrixPolarToRectEx(WORDPTR * a, int32_t rowsa, int32_t colsa);
    void rplMatrixRectToPolarEx(WORDPTR * a, int32_t rowsa, int32_t colsa,
            WORD angtemplate, int32_t angmode);
    void rplMatrixQREx(WORDPTR * a, int32_t rowsa, int32_t colsa);
    WORDPTR rplMatrixQRGetQ(WORDPTR * a, int32_t rowsa, int32_t colsa,
            WORDPTR * diagv);
    WORDPTR rplMatrixQRGetR(WORDPTR * a, int32_t rowsa, int32_t colsa,
            WORDPTR * diagv);
    WORDPTR rplMatrixQRDoRQ(WORDPTR * a, int32_t n, WORDPTR * diagv);

    void rplMatrixUnary(WORD Opcode);

// USER LIBRARIES
    WORDPTR rplGetLibPtr(WORDPTR libptr);
    WORDPTR rplGetLibPtr2(WORD libid, WORD libcmd);
    WORDPTR rplGetLibPtrName(WORDPTR libptr);
    WORDPTR rplGetLibPtrInfo(WORDPTR libptr);

// NUMERIC SOLVERS

    WORDPTR rplPolyEvalEx(WORDPTR * first, int32_t degree, WORDPTR * value);
    WORDPTR rplPolyEvalDerivEx(int32_t deriv, WORDPTR * first, int32_t degree,
            WORDPTR * value);
    WORDPTR rplPolyRootEx(WORDPTR * first, int32_t degree);
    WORDPTR rplPolyDeflateEx(WORDPTR * first, int32_t degree, WORDPTR * value);

// RANDOM NUMBER GENERATOR
    void rplRandomSeed(uint64_t seed);
    void rplRandomJump(void);
    uint64_t rplRandomNext(void);
    int32_t rplRandom8Digits();

// DATE AND TIME FUNCTIONS
    int32_t rplReadRealAsDate(REAL * date, struct date *dt);
    int32_t rplReadRealAsDateNoCk(REAL * date, struct date *dt);
    int32_t rplReadDateAsReal(struct date dt, REAL * date);
    int32_t rplGetMonthDays(int32_t month, int32_t year);
    int32_t rplIsValidDate(struct date dt);
    int32_t rplReadRealAsTime(REAL * time, struct time *tm);
    int32_t rplReadTimeAsReal(struct time tm, REAL * time);
    int32_t rplDateToDays(struct date dt);
    struct date rplDaysToDate(int32_t days);
    int64_t rplDateToSeconds(struct date dt, struct time tm);
    void rplSecondsToDate(int64_t sec, struct date *dt, struct time *tm);
    void rplDecimalToHMS(REAL * dec, REAL * hms);
    void rplHMSToDecimal(REAL * hms, REAL * dec);

// ALARM FUNCTIONS
    int32_t rplReadAlarm(WORDPTR obj, struct alarm *alrm);
    void rplPushAlarm(struct alarm *alrm);
    int32_t rplAddAlarm(struct alarm *alrm);
    int32_t rplGetAlarm(int32_t id, struct alarm *alrm);
    int32_t rplDelAlarm(int32_t id);
    int32_t rplCheckAlarms();
    int32_t rplTriggerAlarm();
    void rplUpdateAlarms();
    void rplSkipNextAlarm();

// KEYBOARD FUNCTIONS
    WORDPTR rplMsg2KeyName(WORD keymsg);
    WORD rplKeyName2Msg(WORDPTR keyname);

// FONT FUNCTIONS
    #define FONT_IDENTS_ROMPTR_INDEX 4
    #define START_ROMPTR_INDEX 16    // START OF THE ROM FONTS TABLE
    WORDPTR const *rplGetFontRomPtrTableAddress(void);
    WORDPTR rplGetCurrentFont(int32_t area);
    void rplSetCurrentFont(int32_t area, WORDPTR ident);
    void rplAddSystemFont(WORDPTR ident, WORDPTR font);
    void rplPurgeSystemFont(WORDPTR ident);
    WORDPTR rplGetSystemFont(WORDPTR ident);
    WORDPTR rplGetSystemFontName(WORDPTR font);

// BITMAP FUNCTIONS
    WORDPTR rplBmpCreate(int32_t type, int32_t width, int32_t height, int32_t clear);
    WORDPTR rplBmpToDisplay(WORDPTR bitmap);

// TAG FUNCTIONS
    int32_t rplStripTagStack(int32_t nlevels);
    WORDPTR rplStripTag(WORDPTR object);

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

#define FL_TVMEND         -14

#define FL_MENU1WHITE     -15
#define FL_MENU2WHITE     -16
#define FL_ANGLEMODE1     -17
#define FL_ANGLEMODE2     -18
#define FL_APPROXSIGN     -19   // Use '~' instead of '.' as approximation sign
#define FL_UNDERFLOWERROR -20
#define FL_OVERFLOWERROR  -21
#define FL_INFINITEERROR  -22
#define FL_NEGUNDERFLOW   -23
#define FL_POSUNDERFLOW   -24
#define FL_OVERFLOW       -25
#define FL_INFINITE       -26

#define FL_PREFERJ        -27

#define FL_MODERPN       -28    // ENABLE RPN MODE WITH 4-LEVEL STACK
#define FL_EXTENDEDRPN   -29    // ENABLE RPN MODE WITH 8-LEVEL STACK
#define FL_STRIPCOMMENTS  -30

#define FL_TIMEFORMAT     -41
#define FL_DATEFORMAT     -42
#define FL_RESRPTALRM     -43
#define FL_SAVACKALRM     -44

// FLAGS -45 THROUGH -51 INCLUDED WERE MOVED TO SETTINGS AND CAN BE REUSED
#define FL_DONEXTCUSTKEY  -45   // CHAIN EXECUTION OF CUSTOM KEYBOARD HANDLERS
#define FL_DODEFAULTKEY   -46   // CHAIN EXECUTION OF DEFAULT KEYBOARD HANDLER
#define FL_NOAUTORECV     -47   // DISABLE AUTOMATIC EXECUTION OF RECEIVED DATA

#define FL_LISTCMDCLEANUP -48   // INTERNAL USE: LIST COMMANDS DO ADDITIONAL CLEANUP ON ERRORS
#define FL_FORCED_RAD     -49   // INTERNAL USE: COMMANDS THAT TAKE ANGLES AS ARGUMENTS NEED TO INTERPRET REALS AS RADIANS

#define FL_ERRORBEEP      -56
#define FL_ALARMBEEP      -57

#define FL_ASMZERO        -58   // ASM INSTRUCTIONS SET THIS FLAG WHEN RESULT IS ZERO
#define FL_ASMNEG         -59   // ASM INSTRUCTIONS SET THIS FLAG WHEN RESULT IS NEGATIVE

#define FL_DECOMPEDIT     -60   // SET TO FORCE ->STR AND + OPERATOR TO DECOMPILE TO PRESERVE OBJECTS
// CLEAR THE FLAG (DEFAULT) TO DECOMPILE FOR DISPLAY ONLY

#define FL_INDEXWRAP      -64

#define FL_AUTOINDENT     -68   // SET TO DISABLE AUTO INDENTING OF SOURCE CODE

#define FL_AUTOSIMPRULES  -70   // SET TO DISABLE ALL RULES DURING AUTOSIMPLIFICATION
#define FL_AUTOSIMPGROUP1 -71   // SET TO DISABLE APPLICATION OF GROUP 1 SIMPLIFICATION RULES
#define FL_AUTOSIMPGROUP2 -72   // SET TO DISABLE APPLICATION OF GROUP 2 SIMPLIFICATION RULES
#define FL_AUTOSIMPGROUP3 -73   // SET TO DISABLE APPLICATION OF GROUP 3 SIMPLIFICATION RULES
#define FL_AUTOSIMPGROUP4 -74   // SET TO DISABLE APPLICATION OF GROUP 4 SIMPLIFICATION RULES
#define FL_AUTOSIMPGROUP5 -75   // SET TO DISABLE APPLICATION OF GROUP 5 SIMPLIFICATION RULES
#define FL_AUTOSIMPGROUP6 -76   // SET TO DISABLE APPLICATION OF GROUP 6 SIMPLIFICATION RULES
#define FL_AUTOSIMPGROUP7 -77   // SET TO DISABLE APPLICATION OF GROUP 7 SIMPLIFICATION RULES
#define FL_AUTOSIMPGROUP8 -78   // SET TO DISABLE APPLICATION OF GROUP 8 SIMPLIFICATION RULES

#define FL_QUICKRESPONSE  -88   // LOWER RESPONSE TIME TO 30 ms BEFORE GOING TO FULL SPEED (WHEN SET)
#define FL_COMPLEXMODE    -103

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
#define EX_HWRESET        8192

// ADD OTHER EXCEPTIONS HERE

// BREAKPOINT FLAGS

#define BKPT_ENABLED    1
#define BKPT_LOCATION   2       // TRIGGER ONLY WHEN IPtr==BreakPtArg
#define BKPT_COND       4       // XEQ RPL OBJECT AT BreakPtArg, IF IT RETURNS TRUE THEN TRIGGER
#define BKPT_PAUSED     8       // DO NOT TRIGGER BREAKPOINT WHILE PAUSED. INTERNAL USE DURING CONDITION EXECUTION
#define BKPT_ALLPAUSED 0x08080808       // INTERNAL USE MASK
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

#endif // NEWRPL_H
