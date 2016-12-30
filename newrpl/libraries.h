/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef LIBRARIES_H
#define LIBRARIES_H

#include "newrpl.h"
#include "common-macros.h"




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
#define OPCODE_AUTOCOMPNEXT 0x7FFF5
#define OPCODE_LIBMENU      0x7FFF4
#define OPCODE_LIBHELP      0x7FFF3
#define OPCODE_LIBMSG       0x7FFF2
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

// MENU CODE OS 0XLLLMmPPP
// WHERE LLL = LIBRARY NUMBER IN HEX
//       Mm = SSmm mmmm BITS. 2 UPPER BITS (S) ARE THE SPECIAL FLAG (0-3).
//              mm mmmm = MENU NUMBER WITHIN THE LIBRARY (0-63).
//       PPP = NUMBER OF THE FIRST ITEM TO DISPLAY IN THE CURRENT PAGE

#define MENULIBRARY(menucode) (((menucode)>>20)&0xfff)
#define MENUNUMBER(menucode) (((menucode)>>12)&0x3f)
#define MENUPAGE(menucode) ((menucode)&0xfff)
#define MENUSPECIAL(menucode) (((menucode)>>18)&0x3)
#define MKMENUCODE(special,lib,num,page) ( (((special)&0x3)<<18) | (((lib)&0xfff)<<20) | (((num)&0x3f)<<12) | ((page)&0xfff) )
#define SETMENUPAGE(menucode,newpage) (((menucode)&~0xfff) | ((newpage)&0xfff))







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
    TITYPE_CUSTOMFUNC,
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

void libCompileCmds(BINT libnum, char *libnames[], WORD libopcodes[], int numcmds);
void libDecompileCmds(           char *libnames[], WORD libopcodes[], int numcmds);
void libProbeCmds(char *libnames[], BINT tokeninfo[], int numcmds);
void libGetInfo(WORD opcode,char *libnames[],WORD libopcodes[],BINT tokeninfo[],int numcmds);
void libGetInfo2(WORD opcode, char *libnames[], BINT tokeninfo[], int numcmds);
void libGetRomptrID(BINT libnum,WORDPTR *table,WORDPTR ptr);
void libGetPTRFromID(WORDPTR *table,WORD id);
void libAutoCompleteNext(BINT libnum,char *libnames[],int numcmds);
void libAutoCompletePrev(BINT libnum,char *libnames[],int numcmds);
void libFindMsg(BINT message,WORDPTR table);




#define APPROX_BIT    1
#define UNQUOTED_BIT  2
#define HIDDEN_BIT    4
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



#define DOCOMMENT   20   // LIBRARIES 20,21,22, 23 ARE COMMENTS

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

#define DOUNIT      54       // UNIT OBJECT

#define DOLIST      62

#define DOSYMB      56      // SYMBOLIC OBJECT

#define DOANGLE     48      // ANGLE TAGGED REALS




#define DOMATRIX    52       // ARRAY OBJECT



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
#define ISSECO(prolog) ( ISPROLOG(prolog) && (LIBNUM(prolog)==SECO))
#define ISNUMBER(prolog) (ISBINT(prolog)||ISREAL(prolog))
#define ISNUMBERCPLX(prolog) (ISBINT(prolog)||ISREAL(prolog)||ISCOMPLEX(prolog))
#define ISSTRING(prolog) (ISPROLOG(prolog) && ((LIBNUM(prolog)&~3)==DOSTRING))

#define ISUNIT(prolog) ( ISPROLOG(prolog) && (LIBNUM(prolog)==DOUNIT))
#define ISNUMBERORUNIT(prolog) (ISBINT(prolog)||ISREAL(prolog)||ISUNIT(prolog))

#define ISANGLE(prolog)   (ISPROLOG(prolog) && ((LIBNUM(prolog)&~3)==DOANGLE))

#define ISSYMBOLIC(prolog) ( ISPROLOG(prolog) && (LIBNUM(prolog)==DOSYMB))
#define ISMATRIX(prolog) ( ISPROLOG(prolog) && (LIBNUM(prolog)==DOMATRIX))
#define ISDIR(prolog) ( ISPROLOG(prolog) && (LIBNUM(prolog)==DODIR))

#define ISAPPROX(prolog) ((LIBNUM(prolog)&APPROX_BIT))


#define ISCOMMENT(prolog) ( ISPROLOG(prolog) && ((LIBNUM(prolog)&~3)==DOCOMMENT))

#define ANGLEMODE(prolog) ( (ISANGLE(prolog)? (BINT)(LIBNUM(prolog)&3):(BINT)ANGLENONE) )

// THIS IS TO CHECK IF AN OBJECT IS "FALSE", WHICH CAN BE THE BINT0 OR THE REAL0
#define IS_FALSE(p)   ( (OPCODE(p)==0) && (((LIBNUM(p)+4)&~7)==OCTBINT))

// CONVENIENCE MACRO TO CREATE SMALL INTEGERS
#define MAKESINT(a) MKOPCODE(DECBINT,(a)&0x3ffff)
#define MAKEBINT64(a) MKPROLOG(DECBINT,2),(WORD)(((BINT64)a)&0xffffffff),((WORD)(((BINT64)a)>>32))

#define MAKEREALFLAGS(exp,len,flags)  ((WORD)(((exp)&0xffff)|(((len)&0xfff)<<16)|(((flags)&0xf)<<28) ))

// CONVENIENCE MACRO TO ENCODE ERROR MESSAGES
#define MAKEMSG(lib,num) MKOPCODE(DECBINT, (((lib)&0xfff)<<7) | ((num)&0x7f))
#define LIBFROMMSG(msg) (((msg)>>7)&0xfff)
// CONVENIENCE MACRO TO ENCODE THE PROLOG OF STRINGS
#define MAKESTRING(length) MKPROLOG(DOSTRING+((4-((length)&3))&3),((length)+3)>>2)


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

#define MKROMPTRID(lib,idx,off) MKOPCODE(LIB_ROMPTR+(((lib)>>8)&0xf), ((((lib)&0xFF)<<11)|(((idx)&0x3f)<<5)|(((off)&0x1f))) )
#define ROMPTRID_IDX(id) ((BINT)((id)>>5)&0x3f)
#define ROMPTRID_OFF(id) ((id)&0x1f)
#define ROMPTRID_LIB(id) ((((id)>>12)&0xf00)|(((id)>>11)&0xff))

#define ISROMPTRID(id) ( (LIBNUM(id)&0xff0) == LIB_ROMPTR)

// MACRO TO INCLUDE PRECOMPILED BINARIES
// IT REPLACES ALL OBJECTS WITH BINT ZERO IF NO_RPL_OBJECTS IS DEFINED
#ifndef  NO_RPL_OBJECTS
#define INCLUDE_ROMOBJECT(id_name) extern ROMOBJECT id_name[]
#else
#define INCLUDE_ROMOBJECT(id_name) ROMOBJECT id_name[]={ MAKESINT(0) }
#endif






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
#define OVRT_NOTEQ        "≠"
#define OVR_NOTEQ   OVR_OPERATORS+OVR_BINARY+2
#define OVRT_LT        "<"
#define OVR_LT      OVR_OPERATORS+OVR_BINARY+3
#define OVRT_GT        ">"
#define OVR_GT      OVR_OPERATORS+OVR_BINARY+4
#define OVRT_LTE        "≤"
#define OVR_LTE      OVR_OPERATORS+OVR_BINARY+5
#define OVRT_GTE        "≥"
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
#define OVRT_XROOT         "XROOT"
#define OVR_XROOT      OVR_OPERATORS+OVR_BINARY+24

// ADD MORE OPERATORS HERE


// SOME EXTERNAL DATA DEFINED BY LIBRARIES, THAT IS REUSED IN OTHER LIBRARIES

extern const WORD abnd_prog[];
extern const WORD lam_baseseco_bint[];
extern const WORD lam_errhandler_bint[];
extern const WORD nulllam_ident[];
extern const WORD minusone_bint[];
extern const WORD zero_bint[];
extern const WORD one_bint[];
extern const WORD two_bint[];
extern const WORD three_bint[];
extern const WORD ten_bint[];
extern const WORD one_real[];
extern const WORD empty_string[];
extern const WORD empty_list[];
extern const WORD angle_180[];
extern const WORD savedcmdline_ident[];
extern const WORD customkey_ident[];
extern const WORD error_reenter_seco[];



// DATE AND TIME MACRO
#define ISLEAPYEAR(y) ((!((y) & 3) && ((y) % 100)) || !((y) % 400))


// DEFINE ALL ERROR CODES
#include "errorcodes.h"



// THESE ARE SPECIAL OPCODES
// THE LOWER 16 BITS ARE THE NUMBER OF LAMS TO CREATE, OR THE INDEX OF LAM NUMBER TO STO/RCL
#define NEWNLOCALS     0x40000   // SPECIAL OPCODE TO CREATE NEW LOCAL VARIABLES
#define GETLAMNEVAL    0x30000   // SPECIAL OPCODE TO RCL THE CONTENT OF A LAM AND EVAL (XEQ ITS CONTENT)
#define GETLAMN        0x20000   // SPECIAL OPCODE TO RCL THE CONTENT OF A LAM
#define PUTLAMN        0x10000   // SPECIAL OPCODE TO STO THE CONTENT OF A LAM


// DEFINE ALL COMMAND OPCODES (CMD_nnn), EXTRACTED DIRECTLY FROM EACH LIBRARY
#include "cmdcodes.h"



#endif // LIBRARIES_H
