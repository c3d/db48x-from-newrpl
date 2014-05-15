#ifndef NEWRPL_H
#define NEWRPL_H


#ifndef MPDECIMAL_H
#include "mpdecimal.h"
#endif



// EXTERNAL API FOR THE NEWRPL MACHINE - TO BE USED ONLY BY USER LIBRARIES
// BASIC CONSTANTS AND TYPE DEFINITIONS FOR THE RUN ENVIRONMENT

typedef void (*LIBHANDLER)(void);

typedef unsigned int WORD;
typedef unsigned char BYTE;
typedef WORD *WORDPTR;
typedef BYTE   *BYTEPTR;
typedef int BINT;
typedef long long BINT64;
typedef int OFFSET;


#ifdef __cplusplus
extern "C" {
#endif

// CONSTANTS THAT CONTROL THE MAIN RPL ENGINE

// NUMBER OF STATIC REGISTERS FOR FAST HANDLING OF REAL NUMBERS
#define REAL_REGISTERS 8

// MAXIMUM PRECISION ALLOWED IN THE SYSTEM
// MAKE SURE REAL_SCRATCHMEM CAN HAVE AT LEAST "REAL_REGISTERS*PRECISION_MAX*2/9" WORDS
// WARNING: THIS CONSTANT CANNOT BE CHANGED UNLESS ALL PRECOMPUTED CONSTANT TABLES ARE CHANGED ACCORDINGLY
#define REAL_PRECISION_MAX 2016



// SCRATCHPAD MEMORY TO ALLOCATE DIGITS FOR ARBITRARY PRECISION TEMP RESULTS
// THIS IS THE NUMBER OF WORDS, EACH GOOD FOR 9 DIGITS.
#define REAL_REGISTER_STORAGE ((REAL_PRECISION_MAX*2)/9+3)
#define REAL_SCRATCHMEM (REAL_REGISTERS*REAL_REGISTER_STORAGE)

// DEFINE THE LIMITS FOR THE EXPONENT RANGE FOR ALL REALS
// NOTE: THIS HAS TO FIT WITHIN THE FIELDS OF REAL_HEADER
#define REAL_EXPONENT_MAX   30000
#define REAL_EXPONENT_MIN   -30000



// HIGH LIBRARIES ARE USER LIBRARIES, SLOWER TO EXECUTE
// LOW LIBRARIES ARE SYSTEM LIBRARIES, WITH FASTER EXECUTION
#define MAXHILIBS 256
#define MAXLOWLIBS 256
// NUMBER OF SCRATCH POINTERS
#define MAX_GC_PTRUPDATE 16


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


// ERROR MANAGEMENT FUNCTIONS
extern void MPDTrapHandler(mpd_context_t *ctx);
extern void rplSetExceptionHandler(WORDPTR Handler);
extern void rplRemoveExceptionHandler();
extern void rplCatchException();


// ENVIRONMENT FUNCTIONS IN RUNSTREAM.C
extern void rplInit(void);
extern void rplSetEntryPoint(WORDPTR ip);
extern void rplRun(void);
extern LIBHANDLER rplGetLibHandler(BINT libnum);
extern inline WORDPTR rplSkipOb(WORDPTR ip);
extern inline void rplSkipNext();
extern inline WORD rplObjSize(WORDPTR ip);



// TEMPOB MEMORY MANAGEMENT IN TEMPOB.C

extern WORDPTR rplAllocTempOb(WORD size);
extern void rplTruncateLastObject(WORDPTR newend);
extern void growTempOb(WORD newtotalsize);
extern void rplAddTempBlock(WORDPTR block);
extern void growTempBlocks(WORD newtotalsize);


// COMPILER FUNCTIONS IN COMPILER.C

extern WORDPTR rplCompile(BYTEPTR string, BINT addwrapper);
extern void rplCompileAppend(WORD word);

// DECOMPILER FUNCTIONS
extern WORDPTR rplDecompile(WORDPTR object);
extern void rplDecompAppendChar(BYTE c);
extern void rplDecompAppendString(BYTEPTR str);
extern void rplDecompAppendString2(BYTEPTR str,BINT len);



// DATA STACK FUNCTIONS IN DATASTACK.C

extern void rplPushData(WORDPTR p);
extern WORDPTR rplPopData();
extern inline WORDPTR rplPeekData(int level);
extern inline void rplOverwriteData(int level,WORDPTR ptr);
extern inline BINT rplDepthData();
extern void rplDropData(int n);
extern void growDStk(WORD newsize);

// RETURN STACK FUNCTIONS IN RETURNSTACK.C

extern void rplPushRet(WORDPTR p);
extern WORDPTR rplPopRet();
extern void growRStk(WORD newsize);
extern inline WORDPTR rplPeekRet(int level);

// GARBAGE COLLECTION
extern void rplGCollect();




// LAM FUNCTIONS
extern void growLAMs(WORD newtotalsize);
extern void rplCreateLAM(WORDPTR nameobj,WORDPTR value);
extern BINT rplCompareIDENT(WORDPTR id1,WORDPTR id2);
extern BINT rplCompareIDENTByName(WORDPTR id1,BYTEPTR name,BINT len);
extern WORDPTR rplGetLAM(WORDPTR nameobj);
extern inline WORDPTR *rplGetLAMn(BINT idx);
extern void rplCleanupLAMs(WORDPTR currentseco);
extern WORDPTR *rplFindLAM(WORDPTR nameobj);
extern WORDPTR *rplFindLAMbyName(BYTEPTR name,BINT len);
extern WORDPTR *rplGetNextLAMEnv(WORDPTR *startpoint);
extern BINT rplNeedNewLAMEnv();
extern BINT rplNeedNewLAMEnvCompiler();
extern void rplCompileIDENT(BINT libnum,BYTEPTR tok,BINT len);
extern BINT rplIsValidIdent(BYTEPTR tok,BINT len);


// GLOBAL VARIABLES AND DIRECTORY FUNCTIONS
extern void growDirs(WORD newtotalsize);
extern void rplCreateGlobalInDir(WORDPTR nameobj,WORDPTR value,WORDPTR *parentdir);
extern void rplCreateGlobal(WORDPTR nameobj,WORDPTR value);
extern WORDPTR *rplFindDirbyHandle(WORDPTR handle);
extern void rplCreateNewDir(WORDPTR name,WORDPTR *parentdir);
extern WORDPTR *rplGetParentDir(WORDPTR *directory);
extern WORDPTR *rplFindGlobalbyName(BYTEPTR name,BINT len,BINT scanparents);
extern WORDPTR *rplFindGlobal(WORDPTR nameobj,BINT scanparents);
extern void rplPurgeGlobal(WORDPTR nameobj);
extern WORDPTR rplGetGlobal(WORDPTR nameobj);
extern WORDPTR *rplMakeNewDir();
extern WORDPTR rplGetDirName(WORDPTR *dir);
extern WORDPTR *rplGetDirfromGlobal(WORDPTR *var);
extern WORDPTR *rplDeepCopyDir(WORDPTR *sourcedir);

// GENERIC OBJECT FUNCTIONS
extern void rplCallOvrOperator(WORD op);
extern void rplCopyObject(WORDPTR dest, WORDPTR src);

// BINT FUNCTIONS
extern void rplNewSINTPush(int num,int base);
extern void rplNewBINTPush(BINT64 num,int base);
extern BINT64 rplReadBINT(WORDPTR ptr);


// REAL FUNCTIONS
extern void rplOneToRReg(int num);
extern void rplZeroToRReg(int num);
extern void rplBINTToRReg(int num,BINT64 value);
extern void rplReadReal(WORDPTR real,mpd_t *dec);
extern void rplCopyRealToRReg(int num,WORDPTR real);
extern void rplRRegToRealPush(int num);

// GENERIC FUNCTIONS FOR BINTS AND REALS
extern void rplNumberToRReg(int num,WORDPTR number);
extern BINT64 rplReadNumberAsBINT(WORDPTR number);
extern void rplReadNumberAsReal(WORDPTR number,mpd_t*dec);


// DEFINED EXCEPTIONS

#define EX_BADOPCODE        1
#define EX_BKPOINT          2
#define EX_OUTOFMEM         4
#define EX_PTROUTOFRANGE    8
#define EX_MATHDIVZERO     16
#define EX_MATHOVERFLOW    32
#define EX_EMPTYSTACK      64
#define EX_EMPTYRSTK      128
#define EX_SYNTAXERROR    256
#define EX_UNDEFINED      512
#define EX_BADARGCOUNT   1024
#define EX_BADARGTYPE    2048
#define EX_BADARGVALUE   4096
#define EX_VARUNDEF      8192
#define EX_NONEMPTYDIR  16384
// ADD MORE HERE...











#ifdef __cplusplus
}
#endif


#endif // NEWRPL_H
