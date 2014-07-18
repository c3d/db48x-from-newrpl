/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef LIBRARIES_H
#define LIBRARIES_H

#include "newrpl.h"




// GENERIC DEFINITIONS ABOUT A LIBRARY

// SPECIAL RESERVED OPCODES
// ALL LIBRARIES MUST HANDLE THESE

#define CMD_EXITRPL         0x00800000
#define CMD_XEQSECO         0x00800002
#define CMD_SEMI            0x00800003
#define CMD_ENDLIST         0x03200000

#define OPCODE_COMPILE      0x7FFFF
#define OPCODE_COMPILECONT  0x7FFFE
#define OPCODE_DECOMPILE    0x7FFFD
#define OPCODE_VALIDATE     0x7FFFC
#define OPCODE_PROBETOKEN   0x7FFFB
#define OPCODE_GETINFO      0x7FFFA



// ADD MORE HERE...


#define MAXLOWLIBS  256
#define MAXHILIBS   256
extern LIBHANDLER ROMLibs[];
extern LIBHANDLER ROMLibs2[];
extern BINT ROMLibs2Num[];




#define MKOPCODE(lib,op) (WORD)((((lib)&0xFFF)<<20)|((op)&0x7FFFF))
#define MKPROLOG(lib,size) ((((lib)&0xFFF)<<20)|((size)&0x3FFFF)|0x80000)
#define OPCODE(p) ( (p)&0x7FFFF)
#define OBJSIZE(p) ((p)&0x3FFFF)
#define LIBNUM(p) ((((WORD)(p))>>20)&0xFFF)
#define ISPROLOG(p) ((((WORD)(p))>>19)&1)


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
    OK_CONTINUE=0,
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
    ERR_NOTMINE,
    ERR_NOTMINE_SPLITTOKEN,
    ERR_SYNTAX,
    ERR_INVALID,
    // ADD MORE HERE...

    OK_TOKENINFO=0x40000000
};

extern void libCompileCmds(BINT libnum, char *libnames[], WORD libopcodes[], int numcmds);
extern void libDecompileCmds(           char *libnames[], WORD libopcodes[], int numcmds);
extern void libProbeCmds(BINT libnum, char *libnames[], BINT tokeninfo[], int numcmds);
extern void libGetInfo(WORD opcode,char *libnames[],WORD libopcodes[],BINT tokeninfo[],int numcmds);


// SOME BASIC OBJECT TYPES HERE NEEDED FOR COMPILER
#define SECO         10
#define DOCOL         8
#define DOREAL       11
#define BINBINT      12
#define DECBINT      13   // ACTUALLY IT'S 12 (BIN), 13, 14 (OCT) AND 15 (HEX), BUT 13 = DECIMAL NUMBER
#define OCTBINT      14
#define HEXBINT      15
#define DOSTRING    16   // ACTUALLY USES 16, 17, 18 AND 19, WHERE LIBNUM&3 = NUMBER OF BYTES OF PADDING IN LAST WORD
#define DOIDENT     20      // THIS IS FOR QUOTED IDENTS, BEING PUSHED IN THE STACK
#define DOIDENTEVAL  21     // LIBRARY THAT EVALUATES THE IDENT IMMEDIATELY (UNQUOTED IDENTS, LAMS ARE PUSHED IN TEH STACK)
#define DODIR       22      // DIRECTORY OBJECTS
#define DOCMPLX     26
#define DOSYMB      28      // SYMBOLIC OBJECT
#define DOLIST      50

#define ISIDENT(prolog) ( ISPROLOG(prolog) && ((LIBNUM(prolog)==DOIDENT)||(LIBNUM(prolog)==DOIDENTEVAL)) )
#define ISBINT(prolog) ( ((OPCODE(prolog)<0x400000) || ISPROLOG(prolog)) && ((LIBNUM(prolog)>=BINBINT) && (LIBNUM(prolog)<=HEXBINT)))
#define ISLIST(prolog) ( ISPROLOG(prolog) && (LIBNUM(prolog)==DOLIST))
#define ISREAL(prolog) ( ISPROLOG(prolog) && ((LIBNUM(prolog)==DOREAL)))
#define ISCOMPLEX(prolog) ( ISPROLOG(prolog) && ((LIBNUM(prolog)==DOCMPLX)))
#define ISPROGRAM(prolog) ( ISPROLOG(prolog) && ((LIBNUM(prolog)==DOCOL) || (LIBNUM(prolog)==SECO)))

#define ISNUMBER(prolog) (ISBINT(prolog)||ISREAL(prolog))
#define ISNUMBERCPLX(prolog) (ISBINT(prolog)||ISREAL(prolog)||ISCOMPLEX(prolog))
#define ISSTRING(prolog) ((LIBNUM(prolog)&~3)==DOSTRING)

#define ISSYMBOLIC(prolog) ( ISPROLOG(prolog) && (LIBNUM(prolog)==DOSYMB))


// THIS IS TO CHECK IF AN OBJECT IS "FALSE", WHICH CAN BE THE BINT0 OR THE REAL0
#define IS_FALSE(p)  ( (OPCODE(p)==0) && (((LIBNUM(p)+2)&~3)==BINBINT))

// CONVENIENCE MACRO TO CREATE SMALL INTEGERS
#define MAKESINT(a) MKOPCODE(DECBINT,(a)&0x3ffff)

// CONSTANTS FOR LAM ENVIRONMENTS
#define LAM_BASESECO     MAKESINT(0x10000)
#define LAM_ERRHANDLER   MAKESINT(0x10001)

#define DIR_START_MARKER  MAKESINT(0x20000)
#define DIR_END_MARKER    MAKESINT(0x20001)
#define DIR_PARENT_MARKER MAKESINT(0x20002)



// DEFINE OVERLOADABLE OPERATORS
#define LIB_OVERLOADABLE    4090

#define OVR_GETNARGS(p) ( ((p)>>12)&0xf )


#define MIN_RESERVED_OPCODE 0x7FFF0
#define MIN_OVERLOAD_OPCODE 0x70000 // OPCODES BETWEEN 0X70000 AND 0X7FFF0 ARE OVERLOADABLE

#define OVR_OPERATORS 0x70000
#define OVR_UNARY      0x1000
#define OVR_BINARY     0x2000

#define ISUNARYOP(prolog) ( (OPCODE(prolog)>=OVR_OPERATORS) && (((prolog)&0xf000)==OVR_UNARY))
#define ISBINARYOP(prolog) ( (OPCODE(prolog)>=OVR_OPERATORS) && (((prolog)&0xf000)==OVR_BINARY))

// UNARY OPERATORS
#define OVRT_INV              "INV"
#define OVR_INV      OVR_OPERATORS+OVR_UNARY+1
#define OVRT_NEG              "NEG"
#define OVR_NEG      OVR_OPERATORS+OVR_UNARY+2
#define OVRT_EVAL             "EVAL"
#define OVR_EVAL      OVR_OPERATORS+OVR_UNARY+3
#define OVRT_XEQ             "XEQ"
#define OVR_XEQ      OVR_OPERATORS+OVR_UNARY+4
#define OVRT_ABS              "ABS"
#define OVR_ABS       OVR_OPERATORS+OVR_UNARY+5
#define OVRT_ISTRUE           "ISTRUE"
#define OVR_ISTRUE     OVR_OPERATORS+OVR_UNARY+6
#define OVRT_NOT              "NOT"
#define OVR_NOT        OVR_OPERATORS+OVR_UNARY+7




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





#endif // LIBRARIES_H
