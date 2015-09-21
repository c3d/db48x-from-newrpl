/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef LIBRARIES_H
#define LIBRARIES_H

#include "newrpl.h"




// GENERIC DEFINITIONS

// SPECIAL RESERVED OPCODES
// ALL LIBRARIES MUST HANDLE THESE

#define OPCODE_COMPILE      0x7FFFF
#define OPCODE_COMPILECONT  0x7FFFE
#define OPCODE_DECOMPILE    0x7FFFD
#define OPCODE_DECOMPEDIT   0x7FFFC
#define OPCODE_VALIDATE     0x7FFFB
#define OPCODE_PROBETOKEN   0x7FFFA
#define OPCODE_GETINFO      0x7FFF9
#define OPCODE_GETROMID     0x7FFF8
#define OPCODE_ROMID2PTR    0x7FFF7
#define OPCODE_CHECKOBJ     0x7FFF6
#define OPCODE_LIBREMOVE    0x7FFF1
#define OPCODE_LIBINSTALL   0x7FFF0



// ADD MORE HERE...


#define MAXLOWLIBS  256
#define MAXHILIBS   256
extern const LIBHANDLER ROMLibs[];


// USEFUL MACROS

#define MKOPCODE(lib,op) (WORD)((((lib)&0xFFF)<<20)|((op)&0x7FFFF))
#define MKPROLOG(lib,size) ((((lib)&0xFFF)<<20)|((size)&0x3FFFF)|0x80000)


#define OPCODE(p) ( (p)&0x7FFFF)
#define OBJSIZE(p) ((p)&0x3FFFF)
#define LIBNUM(p) ((((WORD)(p))>>20)&0xFFF)
#define SETLIBNUMBIT(p,bitmask) (((WORD)(p))|(((WORD)(bitmask))<<20))
#define CLRLIBNUMBIT(p,bitmask) (((WORD)(p))&(~(((WORD)(bitmask))<<20)))

#define ISPROLOG(p) ((((WORD)(p))>>19)&1)

#define TEXT2WORD(a,b,c,d) (WORD)( (WORD)(a) | ((WORD)(b)<<8) | ((WORD)(c)<<16) | ((WORD)(d)<<24))


#define TI_LENGTH(tokeninfo) ((tokeninfo)&0x3fff)
#define TI_TYPE(tokeninfo) (((tokeninfo)&0xfc000)>>14)
#define TI_NARGS(tokeninfo) (((tokeninfo)&0xf00000)>>20)
#define TI_PRECEDENCE(tokeninfo) (((tokeninfo)&0x3f000000)>>24)

#define MKTOKENINFO(length,type,nargs,precedence) (((length)&0x3fff)|(((type)&0x3f)<<14)|(((nargs)&0xf)<<20)|(((precedence)&0x3f)<<24))

enum TokenInfo_Type {
    TITYPE_UNKNOWN=0,
    TITYPE_INTEGER,
    TITYPE_REAL,
    TITYPE_NUMBER=3,
    TITYPE_COMPLEX,
    TITYPE_CNUMBER=7,
    TITYPE_IDENT,
    TITYPE_REALIDENT,
    TITYPE_COMPLEXIDENT,
    TITYPE_CONSTANTIDENT,
    TITYPE_EXPRESSION,
    TITYPE_OPERATORS=16,
    TITYPE_PREFIXOP,
    TITYPE_POSTFIXOP,
    TITYPE_BINARYOP_LEFT,
    TITYPE_BINARYOP_RIGHT,
    TITYPE_FUNCTION,
    TITYPE_OPENBRACKET,
    TITYPE_CLOSEBRACKET,
    TITYPE_COMMA,
    TITYPE_NOTALLOWED
};


enum CompileErrors {
    OK_CONTINUE=1,
    OK_CONTINUE_NOVALIDATE,
    OK_STARTCONSTRUCT,
    OK_CHANGECONSTRUCT,
    OK_INCARGCOUNT,
    OK_ENDCONSTRUCT,
    OK_NEEDMORE,
    OK_NEEDMORESTARTCONST,
    OK_STARTVALIDATE,
    OK_SPLITTOKEN,
    OK_STARTCONSTRUCT_SPLITTOKEN,
    OK_STARTCONSTRUCT_INFIX,
    OK_ENDCONSTRUCT_INFIX,
    OK_ENDCONSTRUCT_INFIX_SPLITTOKEN,
    ERR_NOTMINE,
    ERR_NOTMINE_SPLITTOKEN,
    ERR_SYNTAX,
    ERR_INVALID,
    // ADD MORE HERE...

    OK_TOKENINFO=0x40000000
};

extern void libCompileCmds(BINT libnum, char *libnames[], WORD libopcodes[], int numcmds);
extern void libDecompileCmds(           char *libnames[], WORD libopcodes[], int numcmds);
extern void libProbeCmds(char *libnames[], BINT tokeninfo[], int numcmds);
extern void libGetInfo(WORD opcode,char *libnames[],WORD libopcodes[],BINT tokeninfo[],int numcmds);
extern void libGetInfo2(WORD opcode, char *libnames[], BINT tokeninfo[], int numcmds);
extern void libGetRomptrID(BINT libnum,WORDPTR *table,WORDPTR ptr);
extern void libGetPTRFromID(WORDPTR *table,WORD id);




#define APPROX_BIT    1
#define UNQUOTED_BIT  2
#define HIDDEN_BIT    2
#define REALASSUME_BIT 4
#define ACCEPTPREFIX_BIT 4
// BIT 8 FOR FUTURE USE

// SOME BASIC OBJECT TYPES HERE NEEDED FOR COMPILER
#define SECO          9
#define DOCOL         8
#define DOREAL       10
#define DOREALAPP    (DOREAL|APPROX_BIT)
#define DOBINT       12                     // JUST A GENERIC ALIAS
#define BINBINT      12
#define DECBINT      14
#define OCTBINT      16
#define HEXBINT      18
#define BINBINTAPP   (BINBINT|APPROX_BIT)
#define DECBINTAPP   (DECBINT|APPROX_BIT)
#define OCTBINTAPP   (OCTBINT|APPROX_BIT)
#define HEXBINTAPP   (HEXBINT|APPROX_BIT)


// LIBRARIES 20,21,22, 23 ARE FREE FOR FUTURE USE

#define DOSTRING    24   // ACTUALLY USES 24, 25, 26 AND 27, WHERE LIBNUM&3 = NUMBER OF BYTES OF PADDING IN LAST WORD

#define DODIR       28      // DIRECTORY OBJECTS

#define DOCMPLX     30

// IDENTS TAKE ALL LIBRARIES BETWEEN 32 AND 47 INCLUDED
// 4 BITS ARE USED FOR FLAGS IN THE LIBRARY NUMBER
#define DOIDENT     32      // THIS IS FOR QUOTED IDENTS, BEING PUSHED IN THE STACK
#define DOIDENTAPP  (DOIDENT|APPROX_BIT)      // THIS IS FOR QUOTED IDENTS, BEING PUSHED IN THE STACK
#define DOIDENTEVAL  (DOIDENT|UNQUOTED_BIT)     // LIBRARY THAT EVALUATES THE IDENT IMMEDIATELY (UNQUOTED IDENTS, LAMS ARE PUSHED IN TEH STACK)
#define DOIDENTHIDDEN  (DOIDENT|HIDDEN_BIT)     // VARIABLE IS HIDDEN FROM THE VARS MENU
#define DOIDENTEVALAPP  (DOIDENTEVAL|APPROX_BIT)     // LIBRARY THAT EVALUATES THE IDENT IMMEDIATELY (UNQUOTED IDENTS, LAMS ARE PUSHED IN TEH STACK)
#define DOIDENTREAL (DOIDENT|REALASSUME_BIT)                 // IDENTIFIER ASSUMED TO BE REAL IN SYMBOLICS
#define DOIDENTSIPREFIX (DOIDENT|ACCEPTPREFIX_BIT)  // USED ONLY FOR UNIT DEFINITIONS: THIS UNIT WILL ACCEPT SI PREFIXES

// RESERVE (DOIDENT|8) FOR FUTURE FLAG USE IN SYMBOLICS
#define DOMAXIDENT  47                          // JUST A CONVENIENCE MACRO - MAXIMUM LIB NUMBER TAKEN BY IDENTS

#define DOUNIT      48       // UNIT OBJECT

#define DOLIST      50

#define DOSYMB      52      // SYMBOLIC OBJECT

#define DOMATRIX    58       // ARRAY OBJECT



// USEFUL MACROS FOR TYPE IDENTIFICATION

#define ISIDENT(prolog) ( ISPROLOG(prolog) && ((LIBNUM(prolog)>=DOIDENT)&&(LIBNUM(prolog)<=DOMAXIDENT)) )
#define ISQUOTEDIDENT(prolog) ( ISIDENT(prolog) && !(LIBNUM(prolog)&UNQUOTED_BIT) )
#define ISUNQUOTEDIDENT(prolog) ( ISIDENT(prolog) && (LIBNUM(prolog)&UNQUOTED_BIT) )
#define ISHIDDENIDENT(prolog) ( ISIDENT(prolog) && (LIBNUM(prolog)&HIDDEN_BIT) )
#define ISREALIDENT(prolog) ( ISIDENT(prolog) && (LIBNUM(prolog)&REALASSUME_BIT))
#define ISBINT(prolog) ( ((OPCODE(prolog)<0x400000) || ISPROLOG(prolog)) && (((LIBNUM(prolog)&~APPROX_BIT)>=BINBINT) && ((LIBNUM(prolog)&~APPROX_BIT)<=HEXBINT)))
#define ISLIST(prolog) ( ISPROLOG(prolog) && (LIBNUM(prolog)==DOLIST))
#define ISREAL(prolog) ( ISPROLOG(prolog) && (((LIBNUM(prolog)&~APPROX_BIT)==DOREAL)))
#define ISCOMPLEX(prolog) ( ISPROLOG(prolog) && ((LIBNUM(prolog)==DOCMPLX)))
#define ISPROGRAM(prolog) ( ISPROLOG(prolog) && ((LIBNUM(prolog)==DOCOL) || (LIBNUM(prolog)==SECO)))

#define ISNUMBER(prolog) (ISBINT(prolog)||ISREAL(prolog))
#define ISNUMBERCPLX(prolog) (ISBINT(prolog)||ISREAL(prolog)||ISCOMPLEX(prolog))
#define ISSTRING(prolog) ((LIBNUM(prolog)&~3)==DOSTRING)

#define ISUNIT(prolog) ( ISPROLOG(prolog) && (LIBNUM(prolog)==DOUNIT))

#define ISSYMBOLIC(prolog) ( ISPROLOG(prolog) && (LIBNUM(prolog)==DOSYMB))
#define ISMATRIX(prolog) ( ISPROLOG(prolog) && (LIBNUM(prolog)==DOMATRIX))
#define ISDIR(prolog) ( ISPROLOG(prolog) && (LIBNUM(prolog)==DODIR))

#define ISAPPROX(prolog) ((LIBNUM(prolog)&APPROX_BIT))

// THIS IS TO CHECK IF AN OBJECT IS "FALSE", WHICH CAN BE THE BINT0 OR THE REAL0
#define IS_FALSE(p)   ( (OPCODE(p)==0) && (((LIBNUM(p)+4)&~7)==OCTBINT))

// CONVENIENCE MACRO TO CREATE SMALL INTEGERS
#define MAKESINT(a) MKOPCODE(DECBINT,(a)&0x3ffff)

#define MAKEREALFLAGS(exp,len,flags)  ((WORD)(((exp)&0xffff)|(((len)&0xfff)<<16)|(((flags)&0xf)<<28) ))

// CONVENIENCE MACRO TO ENCODE ERROR MESSAGES
#define MAKEMSG(lib,num) MKOPCODE(DECBINT, (((lib)&0xfff)<<7) | ((num)&0x7f))


// CONVENIENCE MACRO TO GET SIZE OF A MATRIX
#define MATMKSIZE(rows,cols) ( (((rows)&0xffff)<<16)|((cols)&0xffff) )
#define MATROWS(size) ( ((size)>>16)&0xffff )
#define MATCOLS(size) ( (size)&0xffff )



// CONSTANTS FOR LAM ENVIRONMENTS
#define LAM_BASESECO     MAKESINT(0x10000)
#define LAM_ERRHANDLER   MAKESINT(0x10001)

#define DIR_START_MARKER  MAKESINT(0x20000)
#define DIR_END_MARKER    MAKESINT(0x20001)
#define DIR_PARENT_MARKER MAKESINT(0x20002)


// LOCAL ENVIRONMENTS LIBRARY
#define LIB_LOCALENV 4080
// DEFINE OVERLOADABLE OPERATORS
#define LIB_OVERLOADABLE    4090
// ROMPTR ID'S
#define LIB_ROMPTR          0xfe0

// MACRO TO CREATE/EXTRACT ROMPTR ID'S
// ROMPTR IDS HAVE THE HIGH BYTE = 0XFE
// NO LIBRARIES CAN USE THE NUMBERS ABOVE 0XFE0 (4064 TO 4079 ARE ROMPTR ID'S, 4080 TO 4095 ARE SYSTEM LIBS)
// ROMPTR ID ENCODES UP TO 63 ROM OBJECTS, WITH MAXIMUM SIZE OF 31 WORDS EACH

// SPECIAL TYPE DEFINITION FOR RPL OBJECTS STORED IN ROM
#define ROMOBJECT const WORD const __ROMOBJECT__

#define MKROMPTRID(lib,idx,off) MKOPCODE(LIB_ROMPTR+(((lib)>>8)&0xf), ((((lib)&0xFF)<<11)|(((idx)<<5)&0x3f)|(((off)&0x1f))) )
#define ROMPTRID_IDX(id) (((id)>>5)&0x3f)
#define ROMPTRID_OFF(id) ((id)&0x1f)
#define ROMPTRID_LIB(id) ((((id)>>12)&0xf00)|(((id)>>11)&0xff))

#define ISROMPTRID(id) ( (LIBNUM(id)&0xff0) == LIB_ROMPTR)


// COMMANDS THAT NEED TO BE ACCESSED FROM MULTIPLE LIBRARIES
// WARNING: IF COMMANDS ARE REORGANIZED WITHIN LIBRARIES, THIS WILL BREAK
// TODO: FIND A BETTER WAY TO EXPORT COMMANDS (??)

#define CMD_EXITRPL         MKOPCODE(DOCOL,0)
#define CMD_XEQSECO         MKOPCODE(DOCOL,2)
#define CMD_SEMI            MKOPCODE(DOCOL,3)
#define CMD_ENDLIST         MKOPCODE(DOLIST,0)

#define CMD_RULESEPARATOR   MKOPCODE(DOSYMB,0)

#define CMD_NEWLOCALENV     MKOPCODE(LIB_LOCALENV,0)







#define OVR_GETNARGS(p) ( ((p)>>12)&0xf )


#define MIN_RESERVED_OPCODE 0x7FFF0
#define MIN_OVERLOAD_OPCODE 0x70000 // OPCODES BETWEEN 0X70000 AND 0X7FFF0 ARE OVERLOADABLE

#define OVR_OPERATORS 0x70000
#define OVR_UNARY      0x1000
#define OVR_BINARY     0x2000

#define ISUNARYOP(prolog) ( (OPCODE(prolog)>=OVR_OPERATORS) && (((prolog)&0xf000)==OVR_UNARY))
#define ISBINARYOP(prolog) ( (OPCODE(prolog)>=OVR_OPERATORS) && (((prolog)&0xf000)==OVR_BINARY))
#define ISTESTOP(prolog)   ( (OPCODE(prolog)>=OVR_OPERATORS) && (((prolog)&0xfff0)==OVR_BINARY))

// UNARY OPERATORS
#define OVRT_INV              "INV"
#define OVR_INV      OVR_OPERATORS+OVR_UNARY+1
#define OVRT_NEG              "NEG"
#define OVR_NEG      OVR_OPERATORS+OVR_UNARY+2
#define OVRT_EVAL1             "EVAL1"
#define OVR_EVAL1     OVR_OPERATORS+OVR_UNARY+3
#define OVRT_EVAL             "EVAL"
#define OVR_EVAL      OVR_OPERATORS+OVR_UNARY+4
#define OVRT_FUNCEVAL       "FUNCEVAL"
#define OVR_FUNCEVAL      OVR_OPERATORS+OVR_UNARY+5
#define OVRT_XEQ             "XEQ"
#define OVR_XEQ      OVR_OPERATORS+OVR_UNARY+6
#define OVRT_NUM             "→NUM"
#define OVR_NUM      OVR_OPERATORS+OVR_UNARY+7
#define OVRT_ABS              "ABS"
#define OVR_ABS       OVR_OPERATORS+OVR_UNARY+8
#define OVRT_ISTRUE           "ISTRUE"
#define OVR_ISTRUE     OVR_OPERATORS+OVR_UNARY+9
#define OVRT_NOT              "NOT"
#define OVR_NOT        OVR_OPERATORS+OVR_UNARY+10

//UNARY PLUS IN SYMBOLICS
#define OVRT_UPLUS              "+"
#define OVR_UPLUS        OVR_OPERATORS+OVR_UNARY+11
// OPCODE FOR UNARY MINUS IN SYMBOLICS ONLY
#define OVRT_UMINUS           "-"
#define OVR_UMINUS     OVR_OPERATORS+OVR_UNARY+12




// TESTS
#define OVRT_EQ           "=="
#define OVR_EQ      OVR_OPERATORS+OVR_BINARY+1
#define OVRT_NOTEQ        "!="
#define OVR_NOTEQ   OVR_OPERATORS+OVR_BINARY+2
#define OVRT_LT        "<"
#define OVR_LT      OVR_OPERATORS+OVR_BINARY+3
#define OVRT_GT        ">"
#define OVR_GT      OVR_OPERATORS+OVR_BINARY+4
#define OVRT_LTE        "<="
#define OVR_LTE      OVR_OPERATORS+OVR_BINARY+5
#define OVRT_GTE        ">="
#define OVR_GTE      OVR_OPERATORS+OVR_BINARY+6
#define OVRT_SAME        "SAME"
#define OVR_SAME    OVR_OPERATORS+OVR_BINARY+7
#define OVRT_AND        "AND"
#define OVR_AND    OVR_OPERATORS+OVR_BINARY+8
#define OVRT_OR        "OR"
#define OVR_OR    OVR_OPERATORS+OVR_BINARY+9
#define OVRT_XOR        "XOR"
#define OVR_XOR    OVR_OPERATORS+OVR_BINARY+10
#define OVRT_CMP        "CMP"
#define OVR_CMP    OVR_OPERATORS+OVR_BINARY+11


// BASIC OPERATORS
#define OVRT_ADD           "+"
#define OVR_ADD      OVR_OPERATORS+OVR_BINARY+16
#define OVRT_SUB           "-"
#define OVR_SUB      OVR_OPERATORS+OVR_BINARY+17
#define OVRT_MUL           "*"
#define OVR_MUL      OVR_OPERATORS+OVR_BINARY+18
#define OVRT_DIV           "/"
#define OVR_DIV      OVR_OPERATORS+OVR_BINARY+19
#define OVRT_POW           "^"
#define OVR_POW      OVR_OPERATORS+OVR_BINARY+22

// ADD MORE OPERATORS HERE


// SOME EXTERNAL DATA DEFINED BY LIBRARIES, THAT IS REUSED IN OTHER LIBRARIES

extern const WORD abnd_prog[];
extern const WORD lam_baseseco_bint[];
extern const WORD lam_errhandler_bint[];
extern const WORD nulllam_ident[];
extern const WORD zero_bint[];
extern const WORD one_bint[];
extern const WORD two_bint[];
extern const WORD three_bint[];
extern const WORD empty_string[];
extern const WORD empty_list[];

// CONVENIENCE COMMAND NUMBERS
// THESE ARE HARDCODED, THERE SHOULD BE A BETTER WAY
// TO REFERENCE COMMANDS FROM THE KEYBOARD DRIVERS

#define CMD_SIN MKOPCODE(66,0)
#define CMD_COS MKOPCODE(66,1)
#define CMD_TAN MKOPCODE(66,2)
#define CMD_ASIN MKOPCODE(66,3)
#define CMD_ACOS MKOPCODE(66,4)
#define CMD_ATAN MKOPCODE(66,5)
#define CMD_ATAN2 MKOPCODE(66,6)
#define CMD_LN MKOPCODE(66,7)
#define CMD_EXP MKOPCODE(66,8)
#define CMD_SINH MKOPCODE(66,9)
#define CMD_COSH MKOPCODE(66,10)
#define CMD_TANH MKOPCODE(66,11)
#define CMD_ASINH MKOPCODE(66,12)
#define CMD_ACOSH MKOPCODE(66,13)
#define CMD_ATANH MKOPCODE(66,14)
#define CMD_DEG  MKOPCODE(66,15)
#define CMD_GRAD MKOPCODE(66,16)
#define CMD_RAD  MKOPCODE(66,17)
#define CMD_SQRT MKOPCODE(66,18)
#define CMD_STO MKOPCODE(28,0)
#define CMD_XEQ MKOPCODE(LIB_OVERLOADABLE,OVR_XEQ)
#define CMD_RCL MKOPCODE(28,1)
#define CMD_PURGE MKOPCODE(28,4)
#define CMD_CLEAR MKOPCODE(72,1)

#include "errorcodes.h"

#endif // LIBRARIES_H
