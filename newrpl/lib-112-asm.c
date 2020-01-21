/*
 * Copyright (c) 2017, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************

// REPLACE THE NUMBER
#define LIBRARY_NUMBER  112

//@TITLE=RPL Assembly

#define ERROR_LIST \
    ERR(INVALIDASMCODE,0)

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

//#define COMMAND_LIST

// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER,LIBRARY_NUMBER+1,LIBRARY_NUMBER+2,LIBRARY_NUMBER+3,LIBRARY_NUMBER+4,LIBRARY_NUMBER+5,LIBRARY_NUMBER+6,LIBRARY_NUMBER+7

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"

#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

//INCLUDE_ROMOBJECT(LIB_MSGTABLE);
//INCLUDE_ROMOBJECT(LIB_HELPTABLE);

//INCLUDE_ROMOBJECT(lib100_menu);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[] = {
//    (WORDPTR)LIB_MSGTABLE,
//    (WORDPTR)LIB_HELPTABLE,
//    (WORDPTR)lib100_menu,
    0
};

const WORDPTR const numbers_table[] = {
    (WORDPTR) zero_bint,
    (WORDPTR) one_bint,
    (WORDPTR) two_bint,
    (WORDPTR) three_bint,
    (WORDPTR) four_bint,
    (WORDPTR) five_bint,
    (WORDPTR) six_bint,
    (WORDPTR) seven_bint,
    (WORDPTR) eight_bint,
    (WORDPTR) nine_bint,
    (WORDPTR) ten_bint,
    (WORDPTR) eleven_bint,
    (WORDPTR) twelve_bint,
    (WORDPTR) thirteen_bint,
    (WORDPTR) fourteen_bint,
    (WORDPTR) fifteen_bint
};

#define ASM_NOOP 0
#define ASM_LDY  1
#define ASM_ADD  2
#define ASM_SUB  3
#define ASM_MUL  4
#define ASM_DIV  5
#define ASM_POW  6
#define ASM_CMP  7
#define ASM_POP  8
#define ASM_RPOP  9
#define ASM_PUSH  10
#define ASM_RPUSH 11
#define ASM_GET   12
#define ASM_PUT   13
#define ASM_MATH  14
#define ASM_MATH2 15
#define ASM_LOOP  16
#define ASM_SKIP  17
#define ASM_CHK   18

// ADDITIONAL COMMANDS PROPOSED:
#define ASM_MIN   19
#define ASM_MAX   20
#define ASM_RND   21
#define ASM_AND   22
#define ASM_OR    23
#define ASM_XOR   24
#define ASM_CLR   25
#define ASM_SGET  26
#define ASM_SPUT  27

#define ASM_MATH_IP   0
#define ASM_MATH_LN   1
#define ASM_MATH_EXP  2
#define ASM_MATH_SQRT 3
#define ASM_MATH_SIN  4
#define ASM_MATH_COS  5
#define ASM_MATH_TAN  6
#define ASM_MATH_ASIN 7
#define ASM_MATH_ACOS 8
#define ASM_MATH_ATAN 9
#define ASM_MATH_SINH 10
#define ASM_MATH_COSH 11
#define ASM_MATH_TANH 12
#define ASM_MATH_ASINH 13
#define ASM_MATH_ACOSH 14
#define ASM_MATH_ATANH 15

#define ASM_MATH2_FP  0
#define ASM_MATH2_ABS 1
#define ASM_MATH2_ARG 2
#define ASM_MATH2_RE  3
#define ASM_MATH2_IM  4

#define ISLITERALY(opcode) (opcode&0x2000)
#define ISLITERALZ(opcode) (opcode&0x1000)
#define GETD(opcode) (((opcode)&0xf00)>>8)
#define GETY(opcode) (((opcode)&0xf0)>>4)
#define GETZ(opcode) ((opcode)&0xf)
#define ASMOPCODE(opcode) (((opcode)>>14)&0x1f)
#define ASMSTO(opcode) (LIBNUM(opcode)-LIBRARY_NUMBER)

#define ASMSTO_NOOP    0
#define ASMSTO_STO     1
#define ASMSTO_STOADD  2
#define ASMSTO_STOSUB  3
#define ASMSTO_STOMUL  4
#define ASMSTO_STODIV  5

// FLAG CHECK LITERALS

#define ASMF_AL  0      // (always)
#define ASMF_LT  1      // (NEG flag)
#define ASMF_EQ  2      // (ZERO flag)
#define ASMF_LTE 3      // (NEG || ZERO)
#define ASMF_NA  4      // (NOT ALWAYS = NEVER)
#define ASMF_GTE 5      // (NOT LT)
#define ASMF_NE  6      // (NOT EQ)
#define ASMF_GT  7      // (NOT LTE)

const WORD const mathcmd_table[] = {
    CMD_IP,
    CMD_LN,
    CMD_EXP,
    CMD_SQRT,
    CMD_SIN,
    CMD_COS,
    CMD_TAN,
    CMD_ASIN,
    CMD_ACOS,
    CMD_ATAN,
    CMD_SINH,
    CMD_COSH,
    CMD_TANH,
    CMD_ASINH,
    CMD_ACOSH,
    CMD_ATANH
};

const WORD const math2cmd_table[] = {
    CMD_FP,
    CMD_OVR_ABS,
    CMD_ARG,
    CMD_RE,
    CMD_IM
};

// FIND THE END OF THE CURRENT TOKEN

BYTEPTR rplAsmEndOfToken(BYTEPTR start, BYTEPTR end)
{
    if(*start == '.')
        ++start;

    if((start < end) && ((*start == '+') || (*start == '-') || (*start == '*')
                || (*start == '/') || (*start == '^') || (*start == '='))) {
        // THE TOKEN IS AN OPERATOR
        while(start < end) {
            if((*start != '+') && (*start != '-') && (*start != '*')
                    && (*start != '/') && (*start != '^') && (*start != '='))
                break;
            start = (BYTEPTR) utf8skipst((char *)start, (char *)end);
        }

    }
    else {
        // THE TOKEN HAS LETTERS OR NUMBERS
        while(start < end) {
            if((*start == '.') || (*start == '+') || (*start == '-')
                    || (*start == '*') || (*start == '/') || (*start == '^')
                    || (*start == '='))
                break;
            start = (BYTEPTR) utf8skipst((char *)start, (char *)end);
        }
    }
    return start;
}

// DECODE A TOKEN
// opcode_arg RETURNS:
//                      LOWER 8 BITS = ARGUMENT ( 0-7=A-H, 8 = P|R, 9-15=S1-S7, 16-31=LITERAL (N-16), 32 = NOT SPECIFIED, 64=INVALID)
//                      NEXT 8 BITS = OPCODE (0-31 = MAIN OPCODE), 32=NOT SPECIFIED, 64=INVALID. IF OPCODE HAS SUB-OPCODES, IT'S RETURNED AS A LITERAL IN LOWER 8 BITS
//                      NEXT 8 BITS = STORAGE MODE

struct optables
{
    BINT opcode;
    WORD opname;
};

#define ASMF_LT  1      // (NEG flag)
#define ASMF_EQ  2      // (ZERO flag)
#define ASMF_LTE 3      // (NEG || ZERO)
#define ASMF_NA  4      // (NOT ALWAYS = NEVER)
#define ASMF_GTE 5      // (NOT LT)
#define ASMF_NE  6      // (NOT EQ)
#define ASMF_GT  7      // (NOT LTE)

// ALL OPCODES UP TO 4 LETTERS
const struct optables const shortopcodes[] = {
// THESE ARE LITERALS
    {ASMF_LT + 16, TEXT2WORD('L', 'T', 0, 0)},
    {ASMF_EQ + 16, TEXT2WORD('E', 'Q', 0, 0)},
    {ASMF_EQ + 16, TEXT2WORD('Z', 0, 0, 0)},
    {ASMF_LTE + 16, TEXT2WORD('L', 'E', 0, 0)},
    {ASMF_GTE + 16, TEXT2WORD('G', 'E', 0, 0)},
    {ASMF_NE + 16, TEXT2WORD('N', 'E', 0, 0)},
    {ASMF_NE + 16, TEXT2WORD('N', 'Z', 0, 0)},
    {ASMF_GT + 16, TEXT2WORD('G', 'T', 0, 0)},

    {16 + 0, TEXT2WORD('#', '0', 0, 0)},
    {16 + 1, TEXT2WORD('#', '1', 0, 0)},
    {16 + 2, TEXT2WORD('#', '2', 0, 0)},
    {16 + 3, TEXT2WORD('#', '3', 0, 0)},
    {16 + 4, TEXT2WORD('#', '4', 0, 0)},
    {16 + 5, TEXT2WORD('#', '5', 0, 0)},
    {16 + 6, TEXT2WORD('#', '6', 0, 0)},
    {16 + 7, TEXT2WORD('#', '7', 0, 0)},
    {16 + 8, TEXT2WORD('#', '8', 0, 0)},
    {16 + 9, TEXT2WORD('#', '9', 0, 0)},
    {16 + 10, TEXT2WORD('#', '1', '0', 0)},
    {16 + 11, TEXT2WORD('#', '1', '1', 0)},
    {16 + 12, TEXT2WORD('#', '1', '2', 0)},
    {16 + 13, TEXT2WORD('#', '1', '3', 0)},
    {16 + 14, TEXT2WORD('#', '1', '4', 0)},
    {16 + 15, TEXT2WORD('#', '1', '5', 0)},

    {16 + 0, TEXT2WORD('0', 0, 0, 0)},
    {16 + 1, TEXT2WORD('1', 0, 0, 0)},
    {16 + 2, TEXT2WORD('2', 0, 0, 0)},
    {16 + 3, TEXT2WORD('3', 0, 0, 0)},
    {16 + 4, TEXT2WORD('4', 0, 0, 0)},
    {16 + 5, TEXT2WORD('5', 0, 0, 0)},
    {16 + 6, TEXT2WORD('6', 0, 0, 0)},
    {16 + 7, TEXT2WORD('7', 0, 0, 0)},
    {16 + 8, TEXT2WORD('8', 0, 0, 0)},
    {16 + 9, TEXT2WORD('9', 0, 0, 0)},
    {16 + 10, TEXT2WORD('1', '0', 0, 0)},
    {16 + 11, TEXT2WORD('1', '1', 0, 0)},
    {16 + 12, TEXT2WORD('1', '2', 0, 0)},
    {16 + 13, TEXT2WORD('1', '3', 0, 0)},
    {16 + 14, TEXT2WORD('1', '4', 0, 0)},
    {16 + 15, TEXT2WORD('1', '5', 0, 0)},

    {16 + 0, TEXT2WORD('#', '0', 'H', 0)},
    {16 + 1, TEXT2WORD('#', '1', 'H', 0)},
    {16 + 2, TEXT2WORD('#', '2', 'H', 0)},
    {16 + 3, TEXT2WORD('#', '3', 'H', 0)},
    {16 + 4, TEXT2WORD('#', '4', 'H', 0)},
    {16 + 5, TEXT2WORD('#', '5', 'H', 0)},
    {16 + 6, TEXT2WORD('#', '6', 'H', 0)},
    {16 + 7, TEXT2WORD('#', '7', 'H', 0)},
    {16 + 8, TEXT2WORD('#', '8', 'H', 0)},
    {16 + 9, TEXT2WORD('#', '9', 'H', 0)},
    {16 + 10, TEXT2WORD('#', 'A', 'H', 0)},
    {16 + 11, TEXT2WORD('#', 'B', 'H', 0)},
    {16 + 12, TEXT2WORD('#', 'C', 'H', 0)},
    {16 + 13, TEXT2WORD('#', 'D', 'H', 0)},
    {16 + 14, TEXT2WORD('#', 'E', 'H', 0)},
    {16 + 15, TEXT2WORD('#', 'F', 'H', 0)},

// THESE ARE REGISTERS
    {0, TEXT2WORD('A', 0, 0, 0)},
    {1, TEXT2WORD('B', 0, 0, 0)},
    {2, TEXT2WORD('C', 0, 0, 0)},
    {3, TEXT2WORD('D', 0, 0, 0)},
    {4, TEXT2WORD('E', 0, 0, 0)},
    {5, TEXT2WORD('F', 0, 0, 0)},
    {6, TEXT2WORD('G', 0, 0, 0)},
    {7, TEXT2WORD('H', 0, 0, 0)},

// THESE ARE PSEUDO-REGISTERS
    {8, TEXT2WORD('R', 0, 0, 0)},
    {8, TEXT2WORD('P', 0, 0, 0)},

// THESE ARE STACK LEVELS
    {8 + 1, TEXT2WORD('S', '1', 0, 0)},
    {8 + 2, TEXT2WORD('S', '2', 0, 0)},
    {8 + 3, TEXT2WORD('S', '3', 0, 0)},
    {8 + 4, TEXT2WORD('S', '4', 0, 0)},
    {8 + 5, TEXT2WORD('S', '5', 0, 0)},
    {8 + 6, TEXT2WORD('S', '6', 0, 0)},
    {8 + 7, TEXT2WORD('S', '7', 0, 0)},

// THESE ARE OPERATORS
    {(ASM_ADD << 8) | 32, TEXT2WORD('+', 0, 0, 0)},
    {(ASM_SUB << 8) | 32, TEXT2WORD('-', 0, 0, 0)},
    {(ASM_MUL << 8) | 32, TEXT2WORD('*', 0, 0, 0)},
    {(ASM_DIV << 8) | 32, TEXT2WORD('/', 0, 0, 0)},
    {(ASM_POW << 8) | 32, TEXT2WORD('^', 0, 0, 0)},

// ASSIGNMENT (STORAGE)
    {(ASMSTO_STO << 16) | 0x2020, TEXT2WORD('=', 0, 0, 0)},
    {(ASMSTO_STOADD << 16) | 0x2020, TEXT2WORD('+', '=', 0, 0)},
    {(ASMSTO_STOSUB << 16) | 0x2020, TEXT2WORD('-', '=', 0, 0)},
    {(ASMSTO_STOMUL << 16) | 0x2020, TEXT2WORD('*', '=', 0, 0)},
    {(ASMSTO_STODIV << 16) | 0x2020, TEXT2WORD('/', '=', 0, 0)},

// THESE ARE OPCODES
    {(ASM_CMP << 8) | 32, TEXT2WORD('C', 'M', 'P', 0)},
    {(ASM_POP << 8) | 32, TEXT2WORD('P', 'O', 'P', 0)},
    {(ASM_RPOP << 8) | 32, TEXT2WORD('R', 'P', 'O', 'P')},
    {(ASM_PUSH << 8) | 32, TEXT2WORD('P', 'U', 'S', 'H')},
    {(ASM_GET << 8) | 32, TEXT2WORD('G', 'E', 'T', 0)},
    {(ASM_PUT << 8) | 32, TEXT2WORD('P', 'U', 'T', 0)},
    {(ASM_RPUSH << 8) | 32, TEXT2WORD('R', 'P', 'U', 'S')},
    {(ASM_CHK << 8) | 32, TEXT2WORD('C', 'H', 'K', 0)},
    {(ASM_SKIP << 8) | 32, TEXT2WORD('S', 'K', 'I', 'P')},
    {(ASM_LOOP << 8) | 32, TEXT2WORD('L', 'O', 'O', 'P')},
    {(ASM_MIN << 8) | 32, TEXT2WORD('M', 'I', 'N', 0)},
    {(ASM_MAX << 8) | 32, TEXT2WORD('M', 'A', 'X', 0)},
    {(ASM_RND << 8) | 32, TEXT2WORD('R', 'N', 'D', 0)},
    {(ASM_AND << 8) | 32, TEXT2WORD('A', 'N', 'D', 0)},
    {(ASM_OR << 8) | 32, TEXT2WORD('O', 'R', 0, 0)},
    {(ASM_XOR << 8) | 32, TEXT2WORD('X', 'O', 'R', 0)},
    {(ASM_CLR << 8) | 32, TEXT2WORD('C', 'L', 'R', 0)},
    {(ASM_SGET << 8) | 32, TEXT2WORD('S', 'G', 'E', 'T')},
    {(ASM_SPUT << 8) | 32, TEXT2WORD('S', 'P', 'U', 'T')},

// THESE ARE MATH FUNCTIONS
    {(ASM_MATH << 8) | (16 + ASM_MATH_IP), TEXT2WORD('I', 'P', 0, 0)},
    {(ASM_MATH << 8) | (16 + ASM_MATH_LN), TEXT2WORD('L', 'N', 0, 0)},
    {(ASM_MATH << 8) | (16 + ASM_MATH_EXP), TEXT2WORD('E', 'X', 'P', 0)},
    {(ASM_MATH << 8) | (16 + ASM_MATH_SQRT), TEXT2WORD('S', 'Q', 'R', 'T')},
    {(ASM_MATH << 8) | (16 + ASM_MATH_SIN), TEXT2WORD('S', 'I', 'N', 0)},
    {(ASM_MATH << 8) | (16 + ASM_MATH_COS), TEXT2WORD('C', 'O', 'S', 0)},
    {(ASM_MATH << 8) | (16 + ASM_MATH_TAN), TEXT2WORD('T', 'A', 'N', 0)},
    {(ASM_MATH << 8) | (16 + ASM_MATH_ASIN), TEXT2WORD('A', 'S', 'I', 'N')},
    {(ASM_MATH << 8) | (16 + ASM_MATH_ACOS), TEXT2WORD('A', 'C', 'O', 'S')},
    {(ASM_MATH << 8) | (16 + ASM_MATH_ATAN), TEXT2WORD('A', 'T', 'A', 'N')},
    {(ASM_MATH << 8) | (16 + ASM_MATH_SINH), TEXT2WORD('S', 'I', 'N', 'H')},
    {(ASM_MATH << 8) | (16 + ASM_MATH_COSH), TEXT2WORD('C', 'O', 'S', 'H')},
    {(ASM_MATH << 8) | (16 + ASM_MATH_TANH), TEXT2WORD('T', 'A', 'N', 'H')},

// ADDITIONAL MATH FUNCTIONS
    {(ASM_MATH2 << 8) | (16 + ASM_MATH2_FP), TEXT2WORD('F', 'P', 0, 0)},
    {(ASM_MATH2 << 8) | (16 + ASM_MATH2_ABS), TEXT2WORD('A', 'B', 'S', 0)},
    {(ASM_MATH2 << 8) | (16 + ASM_MATH2_ARG), TEXT2WORD('A', 'R', 'G', 0)},
    {(ASM_MATH2 << 8) | (16 + ASM_MATH2_RE), TEXT2WORD('R', 'E', 0, 0)},
    {(ASM_MATH2 << 8) | (16 + ASM_MATH2_IM), TEXT2WORD('I', 'M', 0, 0)},

    {0, 0}
};

// THE ONLY OPCODES WITH MORE THAN 4 LETTERS, THE FIFTH LETTER IS ALWAYS AN H
const struct optables const longopcodes[] = {
    {(ASM_RPUSH << 8) | 32, TEXT2WORD('R', 'P', 'U', 'S')},

    {(ASM_MATH << 8) | (16 + ASM_MATH_ASINH), TEXT2WORD('A', 'S', 'I', 'N')},
    {(ASM_MATH << 8) | (16 + ASM_MATH_ACOSH), TEXT2WORD('A', 'C', 'O', 'S')},
    {(ASM_MATH << 8) | (16 + ASM_MATH_ATANH), TEXT2WORD('A', 'T', 'A', 'N')},
    {0, 0}
};

BINT rplAsmDecodeToken(BYTEPTR start, BYTEPTR end, BINT * opcode_arg)
{

    if(*start == '.')
        ++start;        // SKIP THE SEPARATOR

    // OTHER LONG-NAMED OPCODES

    if((end - start == 5) && (start[4] == 'H')) {
        WORD name;
        struct optables *tbl = (struct optables *)longopcodes;
        while(tbl->opname) {
            name = TEXT2WORD(start[0], start[1], start[2], start[3]);
            if(tbl->opname == name) {
                *opcode_arg = tbl->opcode;
                return 1;
            }
            ++tbl;
        }
    }
    else {
        // SHORT-NAMED OPCODES
        WORD name;
        struct optables *tbl = (struct optables *)shortopcodes;
        switch (end - start) {
        case 1:
            while(tbl->opname) {
                name = TEXT2WORD(start[0], 0, 0, 0);
                if(tbl->opname == name) {
                    *opcode_arg = tbl->opcode;
                    return 1;
                }
                ++tbl;
            }
            break;
        case 2:
            while(tbl->opname) {
                name = TEXT2WORD(start[0], start[1], 0, 0);
                if(tbl->opname == name) {
                    *opcode_arg = tbl->opcode;
                    return 1;
                }
                ++tbl;
            }
            break;
        case 3:
            while(tbl->opname) {
                name = TEXT2WORD(start[0], start[1], start[2], 0);
                if(tbl->opname == name) {
                    *opcode_arg = tbl->opcode;
                    return 1;
                }
                ++tbl;
            }
            break;
        case 4:
            while(tbl->opname) {
                name = TEXT2WORD(start[0], start[1], start[2], start[3]);
                if(tbl->opname == name) {
                    *opcode_arg = tbl->opcode;
                    return 1;
                }
                ++tbl;
            }
            break;
        default:
            break;
        }

    }
    return 0;
}

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    // LIBRARIES THAT DEFINE ONLY COMMANDS STILL HAVE TO RESPOND TO A FEW OVERLOADABLE OPERATORS
    if(LIBNUM(CurOpcode) == LIB_OVERLOADABLE) {
        // ONLY RESPOND TO EVAL, EVAL1 AND XEQ FOR THE COMMANDS DEFINED HERE
        // IN CASE OF COMMANDS TREATED AS OBJECTS (WHEN EMBEDDED IN LISTS)
        if((OPCODE(CurOpcode) == OVR_EVAL) ||
                (OPCODE(CurOpcode) == OVR_EVAL1) ||
                (OPCODE(CurOpcode) == OVR_XEQ)) {
            // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
            if(rplDepthData() < 1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORD saveOpcode = CurOpcode;
            CurOpcode = *rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode = saveOpcode;
            return;
        }
        // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
        if(OPCODE(CurOpcode) == OVR_SAME) {
            if(*rplPeekData(2) == *rplPeekData(1)) {
                rplDropData(2);
                rplPushTrue();
            }
            else {
                rplDropData(2);
                rplPushFalse();
            }
            return;
        }
        else {
            rplError(ERR_INVALIDOPCODE);
            return;
        }

    }

    BINT opcode = OPCODE(CurOpcode) >> 14, oper;

    switch (opcode) {

    case ASM_NOOP:
        return;
    case ASM_LDY:
    {
        // LOAD THE VALUE FROM THE y ARGUMENT AND STORE
        // IMPLEMENTS :R ?= Y

        // GET THE ARGUMENT Y
        BINT skipnext = 0;
        WORDPTR argY;
        if(ISLITERALY(CurOpcode))
            argY = numbers_table[GETY(CurOpcode)];
        else {
            BINT y = GETY(CurOpcode);
            if(y & 0x8) {
                y &= 7;
                if(y == 0) {
                    argY = IPtr + 1;
                    skipnext = 1;
                }
                else {
                    if(DSTop - y < DStkProtect) {
                        rplError(ERR_BADSTACKINDEX);
                        return;
                    }
                    else
                        argY = DSTop[-y];
                }
            }
            else
                argY = GC_UserRegisters[y];
        }

        // GET ARGUMENT D
        BINT d = GETD(CurOpcode);
        WORDPTR *destD;
        if(d & 0x8) {
            d &= 7;
            if(d == 0) {
                rplPushDataNoGrow((WORDPTR) zero_bint);
                destD = DSTop - 1;
            }
            else {
                if(DSTop - d < DStkProtect) {
                    rplError(ERR_BADSTACKINDEX);
                    return;
                }
                else
                    destD = DSTop - d;
            }
        }
        else
            destD = GC_UserRegisters + d;

        BINT opcode;
        switch (ASMSTO(CurOpcode)) {
        default:
        case ASMSTO_NOOP:
            opcode = -1;
            break;
        case ASMSTO_STO:
            opcode = 0;
            break;
        case ASMSTO_STOADD:
            opcode = CMD_OVR_ADD;
            break;
        case ASMSTO_STOSUB:
            opcode = CMD_OVR_SUB;
            break;
        case ASMSTO_STOMUL:
            opcode = CMD_OVR_MUL;
            break;
        case ASMSTO_STODIV:
            opcode = CMD_OVR_DIV;
            break;
        }

        rplPushDataNoGrow(argY);
        if(opcode == -1) {
            // ONLY UPDATE THE FLAGS WITH THE RESULT, IF THE RESULT IS NUMERIC
            if(rplIsFalse(DSTop[-1]))
                rplSetSystemFlag(FL_ASMZERO);
            else
                rplClrSystemFlag(FL_ASMZERO);
            if(rplIsNegative(DSTop[-1]))
                rplSetSystemFlag(FL_ASMNEG);
            else
                rplClrSystemFlag(FL_ASMNEG);
            rplDropData(1);
            if(skipnext) {
                ++IPtr;
                CurOpcode = *IPtr;
            }
            return;
        }
        // CALL THE OPERATION
        if(opcode) {
            rplPushDataNoGrow(argY);
            DSTop[-2] = *destD;
            if(ISNUMBERCPLX(*argY) && ISNUMBERCPLX(**destD))
                rplCallOvrOperator(opcode);
            else
                rplRunAtomic(opcode);
            if(Exceptions)
                return;
        }
        *destD = rplPopData();
        if(skipnext) {
            ++IPtr;
            CurOpcode = *IPtr;
        }
        return;
    }
    case ASM_ADD:
        oper = CMD_OVR_ADD;
      common_case:
        {
            // IMPLEMENTS :R ?= Y+Z

            // GET THE ARGUMENT Y
            BINT skipnext = 0;
            WORDPTR argY;
            if(ISLITERALY(CurOpcode))
                argY = numbers_table[GETY(CurOpcode)];
            else {
                BINT y = GETY(CurOpcode);
                if(y & 0x8) {
                    y &= 7;
                    if(y == 0) {
                        argY = IPtr + 1;
                        skipnext = 1;
                    }
                    else {
                        if(DSTop - y < DStkProtect) {
                            rplError(ERR_BADSTACKINDEX);
                            return;
                        }
                        else
                            argY = DSTop[-y];
                    }
                }
                else
                    argY = GC_UserRegisters[y];
            }

            // GET ARGUMENT Z
            WORDPTR argZ;
            if(ISLITERALZ(CurOpcode))
                argZ = numbers_table[GETZ(CurOpcode)];
            else {
                BINT z = GETZ(CurOpcode);
                if(z & 0x8) {
                    z &= 7;
                    if(z == 0) {
                        argZ = IPtr + 1;
                        skipnext = 1;
                    }
                    else {
                        if(DSTop - z < DStkProtect) {
                            rplError(ERR_BADSTACKINDEX);
                            return;
                        }
                        else
                            argZ = DSTop[-z];
                    }
                }
                else
                    argZ = GC_UserRegisters[z];
            }

            // GET ARGUMENT D
            BINT d = GETD(CurOpcode);
            WORDPTR *destD;
            if(d & 0x8) {
                d &= 7;
                if(d == 0) {
                    rplPushDataNoGrow((WORDPTR) zero_bint);
                    destD = DSTop - 1;
                }
                else {
                    if(DSTop - d < DStkProtect) {
                        rplError(ERR_BADSTACKINDEX);
                        return;
                    }
                    else
                        destD = DSTop - d;
                }
            }
            else
                destD = GC_UserRegisters + d;

            BINT opcode;
            switch (ASMSTO(CurOpcode)) {
            default:
            case ASMSTO_NOOP:
                opcode = -1;
                break;
            case ASMSTO_STO:
                opcode = 0;
                break;
            case ASMSTO_STOADD:
                opcode = CMD_OVR_ADD;
                break;
            case ASMSTO_STOSUB:
                opcode = CMD_OVR_SUB;
                break;
            case ASMSTO_STOMUL:
                opcode = CMD_OVR_MUL;
                break;
            case ASMSTO_STODIV:
                opcode = CMD_OVR_DIV;
                break;
            }
            // CALL THE OPERATION
            rplPushDataNoGrow(argY);
            rplPushDataNoGrow(argZ);
            if(ISNUMBERCPLX(*argY) && ISNUMBERCPLX(*argZ))
                rplCallOperator(oper);
            else
                rplRunAtomic(oper);
            if(Exceptions)
                return;
            if(opcode == -1) {
                // ONLY UPDATE THE FLAGS WITH THE RESULT, IF THE RESULT IS NUMERIC
                if(rplIsFalse(DSTop[-1]))
                    rplSetSystemFlag(FL_ASMZERO);
                else
                    rplClrSystemFlag(FL_ASMZERO);
                if(rplIsNegative(DSTop[-1]))
                    rplSetSystemFlag(FL_ASMNEG);
                else
                    rplClrSystemFlag(FL_ASMNEG);
                rplDropData(1);
                if(skipnext) {
                    ++IPtr;
                    CurOpcode = *IPtr;
                }
                return;
            }
            if(opcode) {
                rplPushDataNoGrow(DSTop[-1]);
                DSTop[-2] = *destD;
                if(ISNUMBERCPLX(*DSTop[-1]) && ISNUMBERCPLX(**destD))
                    rplCallOperator(opcode);
                else
                    rplRunAtomic(opcode);
                if(Exceptions)
                    return;
            }
            *destD = rplPopData();
            if(skipnext) {
                ++IPtr;
                CurOpcode = *IPtr;
            }
            return;
        }
    case ASM_SUB:
        oper = CMD_OVR_SUB;
        goto common_case;
    case ASM_MUL:
        oper = CMD_OVR_MUL;
        goto common_case;
    case ASM_DIV:
        oper = CMD_OVR_DIV;
        goto common_case;
    case ASM_POW:
        oper = CMD_OVR_POW;
        goto common_case;
    case ASM_CMP:
        oper = CMD_OVR_CMP;
        goto common_case;
    case ASM_POP:
    {
        // IMPLEMENTS :R=POP.Y.Z  where:
        // R=first register to receive a stack value must be (A-H)
        // Y=stack level to start the copy (must be a literal with level # or Sn reference)
        // Z=literal or reference with number of items to move

        // EXAMPLE: :A=POP.#1.#3 --> A=Lvl1, B=Lvl2, C=Lvl3, and data from levels 1 to 3 are removed from the stack

        // GET THE ARGUMENT Y
        BINT skipnext = 0;
        BINT ylevel, nitems, nitemsdrop;

        if(ISLITERALY(CurOpcode))
            ylevel = GETY(CurOpcode);
        else {
            BINT y = GETY(CurOpcode);
            if(y & 0x8) {
                y &= 7;
                if(y == 0) {
                    ylevel = rplReadNumberAsBINT(IPtr + 1);
                    if(Exceptions)
                        return;
                    skipnext = 1;
                }
                else {
                    if(DSTop - y < DStkProtect) {
                        rplError(ERR_BADSTACKINDEX);
                        return;
                    }
                    else
                        ylevel = y;
                }
            }
            else {
                ylevel = rplReadNumberAsBINT(GC_UserRegisters[y]);
                if(Exceptions)
                    return;
            }
        }

        // HERE ylevel IS THE STACK LEVEL TO COPY FROM

        // GET ARGUMENT Z

        if(ISLITERALZ(CurOpcode))
            nitems = GETZ(CurOpcode);
        else {
            BINT z = GETZ(CurOpcode);
            if(z & 0x8) {
                z &= 7;
                if(z == 0) {
                    nitems = rplReadNumberAsBINT(IPtr + 1);
                    if(Exceptions)
                        return;
                    skipnext = 1;
                }
                else {
                    nitems = z - ylevel + 1;
                }
            }
            else {
                nitems = rplReadNumberAsBINT(GC_UserRegisters[z]);
                if(Exceptions)
                    return;
            }
        }

        if((nitems < 0) || (nitems > ylevel + rplDepthData() - 1)) {
            rplError(ERR_BADSTACKINDEX);
            return;
        }

        nitemsdrop = nitems;
        // GET ARGUMENT D
        BINT d = GETD(CurOpcode);
        WORDPTR *destD;
        if(d & 0x8) {
            rplError(ERR_INVALIDASMCODE);
            return;
        }
        else
            destD = GC_UserRegisters + d;

        if(d + nitems > 8) {
            nitems = 8 - d;
        }

        if((ASMSTO(CurOpcode) != ASMSTO_STO)
                && (ASMSTO(CurOpcode) != ASMSTO_NOOP)) {
            rplError(ERR_INVALIDASMCODE);
            return;
        }

        // POP THE VALUES FROM THE STACK

        if(ASMSTO(CurOpcode) != ASMSTO_NOOP) {
            BINT k;
            for(k = 0; k < nitems; ++k) {
                *destD = DSTop[-ylevel - k];
                ++destD;
            }
        }
        rplRemoveAtData(ylevel, nitemsdrop);
        if(skipnext) {
            ++IPtr;
            CurOpcode = *IPtr;
        }
        return;
    }
    case ASM_RPOP:
    {
        // IMPLEMENTS :R=RPOP.Y.Z (SAME AS POP BUT IN REVERSE ORDER) where:
        // R=first register to receive a stack value must be (A-H)
        // Y=stack level to start the copy (must be a literal with level # or Sn reference)
        // Z=literal or reference with number of items to move

        // EXAMPLE: :A=RPOP.#1.#3 --> A=Lvl3, B=Lvl2, C=Lvl1, and data from levels 1 to 3 are removed from the stack

        // GET THE ARGUMENT Y
        BINT skipnext = 0;
        BINT ylevel, nitems, nitemsdrop;

        if(ISLITERALY(CurOpcode))
            ylevel = GETY(CurOpcode);
        else {
            BINT y = GETY(CurOpcode);
            if(y & 0x8) {
                y &= 7;
                if(y == 0) {
                    ylevel = rplReadNumberAsBINT(IPtr + 1);
                    if(Exceptions)
                        return;
                    skipnext = 1;
                }
                else {
                    if(DSTop - y < DStkProtect) {
                        rplError(ERR_BADSTACKINDEX);
                        return;
                    }
                    else
                        ylevel = y;
                }
            }
            else {
                ylevel = rplReadNumberAsBINT(GC_UserRegisters[y]);
                if(Exceptions)
                    return;
            }
        }

        // HERE ylevel IS THE STACK LEVEL TO COPY FROM

        // GET ARGUMENT Z

        if(ISLITERALZ(CurOpcode))
            nitems = GETZ(CurOpcode);
        else {
            BINT z = GETZ(CurOpcode);
            if(z & 0x8) {
                z &= 7;
                if(z == 0) {
                    nitems = rplReadNumberAsBINT(IPtr + 1);
                    if(Exceptions)
                        return;
                    skipnext = 1;
                }
                else {
                    nitems = z - ylevel + 1;
                }
            }
            else {
                nitems = rplReadNumberAsBINT(GC_UserRegisters[z]);
                if(Exceptions)
                    return;
            }
        }

        if((nitems < 0) || (nitems > ylevel + rplDepthData() - 1)) {
            rplError(ERR_BADSTACKINDEX);
            return;
        }

        nitemsdrop = nitems;
        // GET ARGUMENT D
        BINT d = GETD(CurOpcode);
        WORDPTR *destD;
        if(d & 0x8) {
            rplError(ERR_INVALIDASMCODE);
            return;
        }
        else
            destD = GC_UserRegisters + d;

        if(d + nitems > 8) {
            nitems = 8 - d;
        }

        if((ASMSTO(CurOpcode) != ASMSTO_STO)
                && (ASMSTO(CurOpcode) != ASMSTO_NOOP)) {
            rplError(ERR_INVALIDASMCODE);
            return;
        }

        // POP THE VALUES FROM THE STACK
        if(ASMSTO(CurOpcode) != ASMSTO_NOOP) {
            BINT k;
            for(k = nitems - 1; k >= 0; --k) {
                *destD = DSTop[-ylevel - k];
                ++destD;
            }
        }
        rplRemoveAtData(ylevel, nitemsdrop);
        if(skipnext) {
            ++IPtr;
            CurOpcode = *IPtr;
        }
        return;
    }
    case ASM_PUSH:
    {
        // IMPLEMENTS PUSH.Y.Z where:
        // Y=first register to push to the stack, must be a register reference (A-H)
        // Z=literal or reference with number of registers to push

        // EXAMPLE: :PUSH.A.#3 --> Lvl1=A, Lvl2=B, Lvl1=3

        // GET THE ARGUMENT Y
        BINT skipnext = 0;
        BINT nitems;
        WORDPTR *reg = 0;
        if(ISLITERALY(CurOpcode)) {
            rplError(ERR_INVALIDASMCODE);
            return;
        }
        else {
            BINT y = GETY(CurOpcode);
            if(y & 0x8) {
                rplError(ERR_INVALIDASMCODE);
                return;
            }
            else
                reg = GC_UserRegisters + y;

        }

        if(ISLITERALZ(CurOpcode))
            nitems = GETZ(CurOpcode);
        else {
            BINT z = GETZ(CurOpcode);
            if(z & 0x8) {
                z &= 7;
                if(z == 0) {
                    nitems = rplReadNumberAsBINT(IPtr + 1);
                    if(Exceptions)
                        return;
                    skipnext = 1;
                }
                else {
                    nitems = rplReadNumberAsBINT(DSTop[-z]);
                    if(Exceptions)
                        return;
                }
            }
            else {
                nitems = rplReadNumberAsBINT(GC_UserRegisters[z]);
                if(Exceptions)
                    return;
            }
        }

        if((nitems < 0) || (nitems + (reg - GC_UserRegisters) > 8)) {
            rplError(ERR_INVALIDASMCODE);
            return;
        }

        // PUSH THE VALUES TO THE STACK
        BINT k;
        for(k = nitems - 1; k >= 0; --k) {
            rplPushData(reg[k]);
            if(Exceptions)
                return;
        }
        if(skipnext) {
            ++IPtr;
            CurOpcode = *IPtr;
        }
        return;
    }
    case ASM_RPUSH:
    {
        // IMPLEMENTS PUSH.Y.Z where:
        // Y=first register to push to the stack, must be a register reference (A-H)
        // Z=literal or reference with number of registers to push

        // EXAMPLE: :PUSH.A.#3 --> Lvl1=A, Lvl2=B, Lvl1=3

        // GET THE ARGUMENT Y
        BINT skipnext = 0;
        BINT nitems;
        WORDPTR *reg = 0;
        if(ISLITERALY(CurOpcode)) {
            rplError(ERR_INVALIDASMCODE);
            return;
        }
        else {
            BINT y = GETY(CurOpcode);
            if(y & 0x8) {
                rplError(ERR_INVALIDASMCODE);
                return;
            }
            else
                reg = GC_UserRegisters + y;

        }

        if(ISLITERALZ(CurOpcode))
            nitems = GETZ(CurOpcode);
        else {
            BINT z = GETZ(CurOpcode);
            if(z & 0x8) {
                z &= 7;
                if(z == 0) {
                    nitems = rplReadNumberAsBINT(IPtr + 1);
                    if(Exceptions)
                        return;
                    skipnext = 1;
                }
                else {
                    nitems = rplReadNumberAsBINT(DSTop[-z]);
                    if(Exceptions)
                        return;
                }
            }
            else {
                nitems = rplReadNumberAsBINT(GC_UserRegisters[z]);
                if(Exceptions)
                    return;
            }
        }

        if((nitems < 0) || (nitems + (reg - GC_UserRegisters) > 8)) {
            rplError(ERR_INVALIDASMCODE);
            return;
        }

        // PUSH THE VALUES TO THE STACK
        BINT k;
        for(k = 0; k < nitems; ++k) {
            rplPushData(reg[k]);
            if(Exceptions)
                return;
        }
        if(skipnext) {
            ++IPtr;
            CurOpcode = *IPtr;
        }
        return;
    }
    case ASM_GET:
    {
        // IMPLEMENTS :R=GET.Y.Z  where:
        // R=Destination reference (register or stack)
        // Y=Reference to a composite object (list, vector or matrix)
        // Z=literal or reference with index of the item to retrieve

        // EXAMPLE: :A=GET.S1.#3 --> Gets the third element of list on stack level 1 (S1)

        // GET THE ARGUMENT Y
        BINT skipnext = 0;
        WORDPTR argY;
        if(ISLITERALY(CurOpcode))
            argY = numbers_table[GETY(CurOpcode)];
        else {
            BINT y = GETY(CurOpcode);
            if(y & 0x8) {
                y &= 7;
                if(y == 0) {
                    argY = IPtr + 1;
                    skipnext = 1;
                }
                else {
                    if(DSTop - y < DStkProtect) {
                        rplError(ERR_BADSTACKINDEX);
                        return;
                    }
                    else
                        argY = DSTop[-y];
                }
            }
            else
                argY = GC_UserRegisters[y];
        }

        // GET ARGUMENT Z
        WORDPTR argZ;
        if(ISLITERALZ(CurOpcode))
            argZ = numbers_table[GETZ(CurOpcode)];
        else {
            BINT z = GETZ(CurOpcode);
            if(z & 0x8) {
                z &= 7;
                if(z == 0) {
                    argZ = IPtr + 1;
                    skipnext = 1;
                }
                else {
                    if(DSTop - z < DStkProtect) {
                        rplError(ERR_BADSTACKINDEX);
                        return;
                    }
                    else
                        argZ = DSTop[-z];
                }
            }
            else
                argZ = GC_UserRegisters[z];
        }

        // GET ARGUMENT D
        BINT d = GETD(CurOpcode);
        WORDPTR *destD;
        if(d & 0x8) {
            d &= 7;
            if(d == 0) {
                rplPushDataNoGrow((WORDPTR) zero_bint);
                destD = DSTop - 1;
            }
            else {
                if(DSTop - d < DStkProtect) {
                    rplError(ERR_BADSTACKINDEX);
                    return;
                }
                else
                    destD = DSTop - d;
            }
        }
        else
            destD = GC_UserRegisters + d;

        BINT opcode;
        switch (ASMSTO(CurOpcode)) {
        default:
        case ASMSTO_NOOP:
            opcode = -1;
            break;
        case ASMSTO_STO:
            opcode = 0;
            break;
        case ASMSTO_STOADD:
            opcode = CMD_OVR_ADD;
            break;
        case ASMSTO_STOSUB:
            opcode = CMD_OVR_SUB;
            break;
        case ASMSTO_STOMUL:
            opcode = CMD_OVR_MUL;
            break;
        case ASMSTO_STODIV:
            opcode = CMD_OVR_DIV;
            break;
        }
        // CALL THE OPERATION
        rplPushDataNoGrow(argY);
        rplPushDataNoGrow(argZ);

        rplCallOperator(CMD_GET);       // COMMAND GET IS KNOWN TO BE ATOMIC
        if(Exceptions)
            return;

        if(opcode == -1) {
            // ONLY UPDATE THE FLAGS WITH THE RESULT, IF THE RESULT IS NUMERIC
            if(rplIsFalse(DSTop[-1]))
                rplSetSystemFlag(FL_ASMZERO);
            else
                rplClrSystemFlag(FL_ASMZERO);
            if(rplIsNegative(DSTop[-1]))
                rplSetSystemFlag(FL_ASMNEG);
            else
                rplClrSystemFlag(FL_ASMNEG);
            rplDropData(1);
            if(skipnext) {
                ++IPtr;
                CurOpcode = *IPtr;
            }
            return;
        }
        if(opcode) {
            rplPushDataNoGrow(DSTop[-1]);
            DSTop[-2] = *destD;
            if(ISNUMBERCPLX(*DSTop[-1]) && ISNUMBERCPLX(**destD))
                rplCallOvrOperator(opcode);
            else
                rplRunAtomic(opcode);
            if(Exceptions)
                return;
        }
        *destD = rplPopData();
        if(skipnext) {
            ++IPtr;
            CurOpcode = *IPtr;
        }
        return;
    }
    case ASM_PUT:
    {
        // IMPLEMENTS :R=PUT.Y.Z  where:
        // R=Source composite AND destination reference (register or stack) - CANNOT be P.
        // Y=Reference or literal to position
        // Z=Reference to object to put in the composite

        // EXAMPLE: :A=PUT.#3.S1 --> A(3)=S1 --->Replace the third element of list on A with stack level 1 (S1)

        // GET THE ARGUMENT Y
        BINT skipnext = 0;
        WORDPTR argY;
        if(ISLITERALY(CurOpcode))
            argY = numbers_table[GETY(CurOpcode)];
        else {
            BINT y = GETY(CurOpcode);
            if(y & 0x8) {
                y &= 7;
                if(y == 0) {
                    argY = IPtr + 1;
                    skipnext = 1;
                }
                else {
                    if(DSTop - y < DStkProtect) {
                        rplError(ERR_BADSTACKINDEX);
                        return;
                    }
                    else
                        argY = DSTop[-y];
                }
            }
            else
                argY = GC_UserRegisters[y];
        }

        // GET ARGUMENT Z
        WORDPTR argZ;
        if(ISLITERALZ(CurOpcode))
            argZ = numbers_table[GETZ(CurOpcode)];
        else {
            BINT z = GETZ(CurOpcode);
            if(z & 0x8) {
                z &= 7;
                if(z == 0) {
                    argZ = IPtr + 1;
                    skipnext = 1;
                }
                else {
                    if(DSTop - z < DStkProtect) {
                        rplError(ERR_BADSTACKINDEX);
                        return;
                    }
                    else
                        argZ = DSTop[-z];
                }
            }
            else
                argZ = GC_UserRegisters[z];
        }

        // GET ARGUMENT D
        BINT d = GETD(CurOpcode);
        WORDPTR *destD;
        if(d & 0x8) {
            d &= 7;
            if(d == 0) {
                rplError(ERR_INVALIDASMCODE);
                return;
            }
            else {
                if(DSTop - d < DStkProtect) {
                    rplError(ERR_BADSTACKINDEX);
                    return;
                }
                else
                    destD = DSTop - d;
            }
        }
        else
            destD = GC_UserRegisters + d;

        if(ASMSTO(CurOpcode) != ASMSTO_STO) {
            rplError(ERR_INVALIDASMCODE);
            return;
        }

        // CALL THE OPERATION
        rplPushDataNoGrow(*destD);
        rplPushDataNoGrow(argY);
        rplPushDataNoGrow(argZ);

        rplCallOperator(CMD_PUT);       // COMMAND PUT IS KNOWN TO BE ATOMIC
        if(Exceptions)
            return;

        *destD = rplPopData();
        if(skipnext) {
            ++IPtr;
            CurOpcode = *IPtr;
        }
        return;
    }

    case ASM_MATH:
    {
        // IMPLEMENTS :R ?= CMD.Z
        // CMD=Y= LITERAL REPRESENTING A MATH FUNCTION (SEE ASM_MATH_XXX CONSTANTS)

        // GET THE ARGUMENT Y
        BINT skipnext = 0;
        BINT ycmd;
        if(ISLITERALY(CurOpcode))
            ycmd = GETY(CurOpcode);
        else {
            rplError(ERR_INVALIDASMCODE);
            return;
        }

        // GET ARGUMENT Z
        WORDPTR argZ;
        if(ISLITERALZ(CurOpcode))
            argZ = numbers_table[GETZ(CurOpcode)];
        else {
            BINT z = GETZ(CurOpcode);
            if(z & 0x8) {
                z &= 7;
                if(z == 0) {
                    argZ = IPtr + 1;
                    skipnext = 1;
                }
                else {
                    if(DSTop - z < DStkProtect) {
                        rplError(ERR_BADSTACKINDEX);
                        return;
                    }
                    else
                        argZ = DSTop[-z];
                }
            }
            else
                argZ = GC_UserRegisters[z];
        }

        // GET ARGUMENT D
        BINT d = GETD(CurOpcode);
        WORDPTR *destD;
        if(d & 0x8) {
            d &= 7;
            if(d == 0) {
                rplPushDataNoGrow((WORDPTR) zero_bint);
                destD = DSTop - 1;
            }
            else {
                if(DSTop - d < DStkProtect) {
                    rplError(ERR_BADSTACKINDEX);
                    return;
                }
                else
                    destD = DSTop - d;
            }
        }
        else
            destD = GC_UserRegisters + d;

        BINT opcode;
        switch (ASMSTO(CurOpcode)) {
        default:
        case ASMSTO_NOOP:
            opcode = -1;
            break;
        case ASMSTO_STO:
            opcode = 0;
            break;
        case ASMSTO_STOADD:
            opcode = CMD_OVR_ADD;
            break;
        case ASMSTO_STOSUB:
            opcode = CMD_OVR_SUB;
            break;
        case ASMSTO_STOMUL:
            opcode = CMD_OVR_MUL;
            break;
        case ASMSTO_STODIV:
            opcode = CMD_OVR_DIV;
            break;
        }
        // CALL THE OPERATION
        rplPushDataNoGrow(argZ);

        if(ISNUMBERCPLX(*argZ))
            rplCallOperator(mathcmd_table[ycmd]);
        else
            rplRunAtomic(mathcmd_table[ycmd]);
        if(Exceptions)
            return;
        if(opcode == -1) {
            // ONLY UPDATE THE FLAGS WITH THE RESULT, IF THE RESULT IS NUMERIC
            if(rplIsFalse(DSTop[-1]))
                rplSetSystemFlag(FL_ASMZERO);
            else
                rplClrSystemFlag(FL_ASMZERO);
            if(rplIsNegative(DSTop[-1]))
                rplSetSystemFlag(FL_ASMNEG);
            else
                rplClrSystemFlag(FL_ASMNEG);
            rplDropData(1);
            if(skipnext) {
                ++IPtr;
                CurOpcode = *IPtr;
            }
            return;
        }
        if(opcode) {
            rplPushDataNoGrow(DSTop[-1]);
            DSTop[-2] = *destD;
            if(ISNUMBERCPLX(*DSTop[-1]) && ISNUMBERCPLX(**destD))
                rplCallOvrOperator(opcode);
            else
                rplRunAtomic(opcode);
            if(Exceptions)
                return;
        }
        *destD = rplPopData();
        if(skipnext) {
            ++IPtr;
            CurOpcode = *IPtr;
        }
        return;
    }

    case ASM_MATH2:
    {
        // IMPLEMENTS :R ?= CMD.Z
        // CMD=Y= LITERAL REPRESENTING A MATH FUNCTION (SEE ASM_MATH_XXX CONSTANTS)

        // GET THE ARGUMENT Y
        BINT skipnext = 0;
        BINT ycmd;
        if(ISLITERALY(CurOpcode))
            ycmd = GETY(CurOpcode);
        else {
            rplError(ERR_INVALIDASMCODE);
            return;
        }

        // GET ARGUMENT Z
        WORDPTR argZ;
        if(ISLITERALZ(CurOpcode))
            argZ = numbers_table[GETZ(CurOpcode)];
        else {
            BINT z = GETZ(CurOpcode);
            if(z & 0x8) {
                z &= 7;
                if(z == 0) {
                    argZ = IPtr + 1;
                    skipnext = 1;
                }
                else {
                    if(DSTop - z < DStkProtect) {
                        rplError(ERR_BADSTACKINDEX);
                        return;
                    }
                    else
                        argZ = DSTop[-z];
                }
            }
            else
                argZ = GC_UserRegisters[z];
        }

        // GET ARGUMENT D
        BINT d = GETD(CurOpcode);
        WORDPTR *destD;
        if(d & 0x8) {
            d &= 7;
            if(d == 0) {
                rplPushDataNoGrow((WORDPTR) zero_bint);
                destD = DSTop - 1;
            }
            else {
                if(DSTop - d < DStkProtect) {
                    rplError(ERR_BADSTACKINDEX);
                    return;
                }
                else
                    destD = DSTop - d;
            }
        }
        else
            destD = GC_UserRegisters + d;

        BINT opcode;
        switch (ASMSTO(CurOpcode)) {
        default:
        case ASMSTO_NOOP:
            opcode = -1;
            break;
        case ASMSTO_STO:
            opcode = 0;
            break;
        case ASMSTO_STOADD:
            opcode = CMD_OVR_ADD;
            break;
        case ASMSTO_STOSUB:
            opcode = CMD_OVR_SUB;
            break;
        case ASMSTO_STOMUL:
            opcode = CMD_OVR_MUL;
            break;
        case ASMSTO_STODIV:
            opcode = CMD_OVR_DIV;
            break;
        }
        // CALL THE OPERATION
        rplPushDataNoGrow(argZ);

        if(ISNUMBERCPLX(*argZ))
            rplCallOperator(math2cmd_table[ycmd]);
        else
            rplRunAtomic(math2cmd_table[ycmd]);
        if(Exceptions)
            return;
        if(opcode == -1) {
            // ONLY UPDATE THE FLAGS WITH THE RESULT, IF THE RESULT IS NUMERIC
            if(rplIsFalse(DSTop[-1]))
                rplSetSystemFlag(FL_ASMZERO);
            else
                rplClrSystemFlag(FL_ASMZERO);
            if(rplIsNegative(DSTop[-1]))
                rplSetSystemFlag(FL_ASMNEG);
            else
                rplClrSystemFlag(FL_ASMNEG);
            rplDropData(1);
            if(skipnext) {
                ++IPtr;
                CurOpcode = *IPtr;
            }
            return;
        }
        if(opcode) {
            rplPushDataNoGrow(DSTop[-1]);
            DSTop[-2] = *destD;
            if(ISNUMBERCPLX(*DSTop[-1]) && ISNUMBERCPLX(**destD))
                rplCallOvrOperator(opcode);
            else
                rplRunAtomic(opcode);
            if(Exceptions)
                return;
        }
        *destD = rplPopData();
        if(skipnext) {
            ++IPtr;
            CurOpcode = *IPtr;
        }
        return;
    }

    case ASM_LOOP:
    {
        BINT doloop = 0;
        BINT yflags;
        if(ISLITERALY(CurOpcode))
            yflags = GETY(CurOpcode);
        else {
            rplError(ERR_INVALIDASMCODE);
            return;
        }
        BINT isneg = rplTestSystemFlag(FL_ASMNEG);
        BINT iszero = rplTestSystemFlag(FL_ASMZERO);

        switch (yflags) {
        default:
        case ASMF_AL:  // (always)
            doloop = 1;
            break;
        case ASMF_LT:  // (NEG flag)
            if(isneg)
                doloop = 1;
            break;
        case ASMF_EQ:  // (ZERO flag)
            if(iszero)
                doloop = 1;
            break;
        case ASMF_LTE: // (NEG || ZERO)
            if(isneg || iszero)
                doloop = 1;
            break;
        case ASMF_NA:  // (NOT ALWAYS = NEVER)
            break;
        case ASMF_GTE: // (NOT LT)
            if(!isneg)
                doloop = 1;
            break;
        case ASMF_NE:  // (NOT EQ)
            if(!iszero)
                doloop = 1;
            break;
        case ASMF_GT:  // (NOT LTE)
            if(!(isneg || iszero))
                doloop = 1;
            break;
        }

        if(doloop) {
            rplPushRet(IPtr - 1);       // THERE MUST BE A SINGLE-WORD OPCODE BEFORE THE LOOP INSTRUCTION, COMPILER SHOULD ADD A NOOP IF THIS ISN'T TRUE
            if(ISPROGRAM(IPtr[1]))
                ++IPtr; // GO INSIDE THE PROGRAM IN THE RUNSTREAM, SKIP THE PROLOG SO IT DOESN'T OVERWRITE THE RETURN ADDRESS
        }
        else {
            // SKIP THE NEXT INSTRUCTION OR OBJECT
            ++IPtr;
            CurOpcode = *IPtr;
        }

        return;
    }

        return;
    case ASM_SKIP:
    {
        BINT skipnext = 0;
        BINT yflags;
        if(ISLITERALY(CurOpcode))
            yflags = GETY(CurOpcode);
        else {
            rplError(ERR_INVALIDASMCODE);
            return;
        }
        BINT isneg = rplTestSystemFlag(FL_ASMNEG);
        BINT iszero = rplTestSystemFlag(FL_ASMZERO);

        switch (yflags) {
        default:
        case ASMF_AL:  // (always)
            skipnext = 1;
            break;
        case ASMF_LT:  // (NEG flag)
            if(isneg)
                skipnext = 1;
            break;
        case ASMF_EQ:  // (ZERO flag)
            if(iszero)
                skipnext = 1;
            break;
        case ASMF_LTE: // (NEG || ZERO)
            if(isneg || iszero)
                skipnext = 1;
            break;
        case ASMF_NA:  // (NOT ALWAYS = NEVER)
            break;
        case ASMF_GTE: // (NOT LT)
            if(!isneg)
                skipnext = 1;
            break;
        case ASMF_NE:  // (NOT EQ)
            if(!iszero)
                skipnext = 1;
            break;
        case ASMF_GT:  // (NOT LTE)
            if(!(isneg || iszero))
                skipnext = 1;
            break;
        }

        if(skipnext) {
            ++IPtr;
            CurOpcode = *IPtr;
        }

        return;
    }
    case ASM_CHK:
    {
        BINT yflags;
        if(ISLITERALY(CurOpcode))
            yflags = GETY(CurOpcode);
        else {
            rplError(ERR_INVALIDASMCODE);
            return;
        }
        BINT isneg = rplTestSystemFlag(FL_ASMNEG);
        BINT iszero = rplTestSystemFlag(FL_ASMZERO);
        WORDPTR result;
        switch (yflags) {
        default:
        case ASMF_AL:  // (always)
            result = (WORDPTR) one_bint;
            break;
        case ASMF_LT:  // (NEG flag)
            if(isneg)
                result = (WORDPTR) one_bint;
            else
                result = (WORDPTR) zero_bint;
            break;
        case ASMF_EQ:  // (ZERO flag)
            if(iszero)
                result = (WORDPTR) one_bint;
            else
                result = (WORDPTR) zero_bint;
            break;
        case ASMF_LTE: // (NEG || ZERO)
            if(isneg || iszero)
                result = (WORDPTR) one_bint;
            else
                result = (WORDPTR) zero_bint;
            break;
        case ASMF_NA:  // (NOT ALWAYS = NEVER)
            result = (WORDPTR) zero_bint;
            break;
        case ASMF_GTE: // (NOT LT)
            if(!isneg)
                result = (WORDPTR) one_bint;
            else
                result = (WORDPTR) zero_bint;
            break;
        case ASMF_NE:  // (NOT EQ)
            if(!iszero)
                result = (WORDPTR) one_bint;
            else
                result = (WORDPTR) zero_bint;
            break;
        case ASMF_GT:  // (NOT LTE)
            if(!(isneg || iszero))
                result = (WORDPTR) one_bint;
            else
                result = (WORDPTR) zero_bint;
            break;
        }

        // GET ARGUMENT D
        BINT d = GETD(CurOpcode);
        WORDPTR *destD;
        if(d & 0x8) {
            d &= 7;
            if(d == 0) {
                rplPushDataNoGrow((WORDPTR) zero_bint);
                destD = DSTop - 1;
            }
            else {
                if(DSTop - d < DStkProtect) {
                    rplError(ERR_BADSTACKINDEX);
                    return;
                }
                else
                    destD = DSTop - d;
            }
        }
        else
            destD = GC_UserRegisters + d;

        BINT opcode;
        switch (ASMSTO(CurOpcode)) {
        default:
        case ASMSTO_NOOP:
            opcode = -1;
            break;
        case ASMSTO_STO:
            opcode = 0;
            break;
        case ASMSTO_STOADD:
            opcode = CMD_OVR_ADD;
            break;
        case ASMSTO_STOSUB:
            opcode = CMD_OVR_SUB;
            break;
        case ASMSTO_STOMUL:
            opcode = CMD_OVR_MUL;
            break;
        case ASMSTO_STODIV:
            opcode = CMD_OVR_DIV;
            break;
        }
        // CALL THE OPERATION
        rplPushDataNoGrow(result);

        if(opcode == -1) {
            // ONLY UPDATE THE FLAGS WITH THE RESULT, IF THE RESULT IS NUMERIC
            if(rplIsFalse(DSTop[-1]))
                rplSetSystemFlag(FL_ASMZERO);
            else
                rplClrSystemFlag(FL_ASMZERO);
            if(rplIsNegative(DSTop[-1]))
                rplSetSystemFlag(FL_ASMNEG);
            else
                rplClrSystemFlag(FL_ASMNEG);
            rplDropData(1);
            return;
        }
        if(opcode) {
            rplPushDataNoGrow(DSTop[-1]);
            DSTop[-2] = *destD;
            if(ISNUMBERCPLX(*DSTop[-1]) && ISNUMBERCPLX(**destD))
                rplCallOperator(opcode);
            else
                rplRunAtomic(opcode);
            if(Exceptions)
                return;
        }
        *destD = rplPopData();
        return;
    }

    case ASM_MIN:
        oper = CMD_MIN;
        goto common_case;
    case ASM_MAX:
        oper = CMD_MAX;
        goto common_case;
    case ASM_RND:
        oper = CMD_RND;
        goto common_case;
    case ASM_AND:
        oper = CMD_OVR_AND;
        goto common_case;
    case ASM_OR:
        oper = CMD_OVR_OR;
        goto common_case;
    case ASM_XOR:
        oper = CMD_OVR_XOR;
        goto common_case;

    case ASM_CLR:
    {
        // IMPLEMENTS :CLR.Y.Z  where:
        // Y=register or stack level to zero-fill (must be A-H or Sn reference)
        // Z=literal or reference with number of items to zero-fill

        // EXAMPLE: :CLR.A.#3 --> A=0, B=0, C=0. also :CLR.S1.#3 --> S1=0, S2=0, S3=0 (error if S1..S3 levels are empty)

        // GET THE ARGUMENT Y
        BINT skipnext = 0;
        BINT nitems, instack = 0;
        WORDPTR *reg = 0;
        if(ISLITERALY(CurOpcode)) {
            rplError(ERR_INVALIDASMCODE);
            return;
        }
        else {
            BINT y = GETY(CurOpcode);
            if(y & 0x8) {
                reg = DSTop - (y & 7);
                instack = 1;
            }
            else
                reg = GC_UserRegisters + y;

        }

        if(ISLITERALZ(CurOpcode))
            nitems = GETZ(CurOpcode);
        else {
            BINT z = GETZ(CurOpcode);
            if(z & 0x8) {
                z &= 7;
                if(z == 0) {
                    nitems = rplReadNumberAsBINT(IPtr + 1);
                    if(Exceptions)
                        return;
                    skipnext = 1;
                }
                else {
                    nitems = rplReadNumberAsBINT(DSTop[-z]);
                    if(Exceptions)
                        return;
                }
            }
            else {
                nitems = rplReadNumberAsBINT(GC_UserRegisters[z]);
                if(Exceptions)
                    return;
            }
        }

        if(!instack) {
            if((nitems < 0) || (nitems + (reg - GC_UserRegisters) > 8)) {
                rplError(ERR_INVALIDASMCODE);
                return;
            }
            // CLEAR THE REGISTERS
            BINT k;
            for(k = 0; k < nitems; ++k)
                reg[k] = (WORDPTR) zero_bint;
        }
        else {
            if((nitems < 0) || ((reg - DStkProtect + 1) < nitems)) {
                rplError(ERR_BADSTACKINDEX);
                return;
            }
            // CLEAR THE STACK
            BINT k;
            for(k = 0; k < nitems; ++k)
                reg[-k] = (WORDPTR) zero_bint;
        }

        if(skipnext) {
            ++IPtr;
            CurOpcode = *IPtr;
        }
        return;
    }

    case ASM_SGET:
    {
        // IMPLEMENTS :R?=SGET.Y  where:
        // R=register to receive a stack value
        // Y=stack level (literal or register with valid stack index #)

        // EXAMPLE: :A+=SGET.B --> A+=Lvl(B)

        // GET THE ARGUMENT Y
        BINT skipnext = 0;
        BINT ylevel;

        if(ISLITERALY(CurOpcode))
            ylevel = GETY(CurOpcode);
        else {
            BINT y = GETY(CurOpcode);
            if(y & 0x8) {
                y &= 7;
                if(y == 0) {
                    ylevel = rplReadNumberAsBINT(IPtr + 1);
                    if(Exceptions)
                        return;
                    skipnext = 1;
                }
                else {
                    if(DSTop - y < DStkProtect) {
                        rplError(ERR_BADSTACKINDEX);
                        return;
                    }
                    else {
                        ylevel = rplReadNumberAsBINT(DSTop[-y]);
                        if(Exceptions)
                            return;
                    }
                }
            }
            else {
                ylevel = rplReadNumberAsBINT(GC_UserRegisters[y]);
                if(Exceptions)
                    return;
            }
        }

        // HERE ylevel IS THE STACK LEVEL TO COPY FROM
        if((ylevel < 1) || (DSTop - ylevel < DStkProtect)) {
            rplError(ERR_BADSTACKINDEX);
            return;
        }

        // GET ARGUMENT D
        BINT d = GETD(CurOpcode);
        WORDPTR *destD;
        if(d & 0x8) {
            d &= 7;
            if(d == 0) {
                rplPushDataNoGrow((WORDPTR) zero_bint);
                destD = DSTop - 1;
                ++ylevel;
            }
            else {
                if(DSTop - d < DStkProtect) {
                    rplError(ERR_BADSTACKINDEX);
                    return;
                }
                else
                    destD = DSTop - d;
            }
        }
        else
            destD = GC_UserRegisters + d;

        BINT opcode;
        switch (ASMSTO(CurOpcode)) {
        default:
        case ASMSTO_NOOP:
            opcode = -1;
            break;
        case ASMSTO_STO:
            opcode = 0;
            break;
        case ASMSTO_STOADD:
            opcode = CMD_OVR_ADD;
            break;
        case ASMSTO_STOSUB:
            opcode = CMD_OVR_SUB;
            break;
        case ASMSTO_STOMUL:
            opcode = CMD_OVR_MUL;
            break;
        case ASMSTO_STODIV:
            opcode = CMD_OVR_DIV;
            break;
        }

        if(opcode == -1) {
            // ONLY UPDATE THE FLAGS WITH THE RESULT, IF THE RESULT IS NUMERIC
            if(rplIsFalse(DSTop[-ylevel]))
                rplSetSystemFlag(FL_ASMZERO);
            else
                rplClrSystemFlag(FL_ASMZERO);
            if(rplIsNegative(DSTop[-ylevel]))
                rplSetSystemFlag(FL_ASMNEG);
            else
                rplClrSystemFlag(FL_ASMNEG);
            if(skipnext) {
                ++IPtr;
                CurOpcode = *IPtr;
            }
            return;
        }
        if(opcode) {
            rplPushDataNoGrow(*destD);
            rplPushDataNoGrow(DSTop[-ylevel - 1]);
            if(ISNUMBERCPLX(*DSTop[-1]) && ISNUMBERCPLX(**destD))
                rplCallOperator(opcode);
            else
                rplRunAtomic(opcode);
            if(Exceptions)
                return;
            *destD = rplPopData();
        }
        else
            *destD = DSTop[-ylevel];
        if(skipnext) {
            ++IPtr;
            CurOpcode = *IPtr;
        }
        return;

    }

    case ASM_SPUT:
    {
        // IMPLEMENTS :R=SPUT.Y.Z  where:
        // R=register to receive the old value from the stack
        // Y=stack level (literal or register with valid stack index #)
        // Z=register to store as new value for that stack position

        // EXAMPLE: :A=SPUT.B.C --> A=Lvl(B), Lvl(B)=C

        // GET THE ARGUMENT Y
        BINT skipnext = 0;
        BINT ylevel;

        if(ISLITERALY(CurOpcode))
            ylevel = GETY(CurOpcode);
        else {
            BINT y = GETY(CurOpcode);
            if(y & 0x8) {
                y &= 7;
                if(y == 0) {
                    ylevel = rplReadNumberAsBINT(IPtr + 1);
                    if(Exceptions)
                        return;
                    skipnext = 1;
                }
                else {
                    if(DSTop - y < DStkProtect) {
                        rplError(ERR_BADSTACKINDEX);
                        return;
                    }
                    else {
                        ylevel = rplReadNumberAsBINT(DSTop[-y]);
                        if(Exceptions)
                            return;
                    }
                }
            }
            else {
                ylevel = rplReadNumberAsBINT(GC_UserRegisters[y]);
                if(Exceptions)
                    return;
            }
        }

        // HERE ylevel IS THE OBJECT TO STORE IN THE STACK
        if((ylevel < 1) || (DSTop - ylevel < DStkProtect)) {
            rplError(ERR_BADSTACKINDEX);
            return;
        }

        // GET ARGUMENT Z
        WORDPTR argZ;
        if(ISLITERALZ(CurOpcode))
            argZ = numbers_table[GETZ(CurOpcode)];
        else {
            BINT z = GETZ(CurOpcode);
            if(z & 0x8) {
                z &= 7;
                if(z == 0) {
                    argZ = IPtr + 1;
                    skipnext = 1;
                }
                else {
                    if(DSTop - z < DStkProtect) {
                        rplError(ERR_BADSTACKINDEX);
                        return;
                    }
                    else
                        argZ = DSTop[-z];
                }
            }
            else
                argZ = GC_UserRegisters[z];
        }

        // GET ARGUMENT D
        BINT d = GETD(CurOpcode);
        WORDPTR *destD;
        if(d & 0x8) {
            d &= 7;
            if(d == 0) {
                rplPushDataNoGrow((WORDPTR) zero_bint);
                destD = DSTop - 1;
                ++ylevel;
            }
            else {
                if(DSTop - d < DStkProtect) {
                    rplError(ERR_BADSTACKINDEX);
                    return;
                }
                else
                    destD = DSTop - d;
            }
        }
        else
            destD = GC_UserRegisters + d;

        BINT opcode;
        switch (ASMSTO(CurOpcode)) {
        default:
        case ASMSTO_NOOP:
            opcode = -1;
            break;
        case ASMSTO_STO:
            opcode = 0;
            break;
        case ASMSTO_STOADD:
            opcode = CMD_OVR_ADD;
            break;
        case ASMSTO_STOSUB:
            opcode = CMD_OVR_SUB;
            break;
        case ASMSTO_STOMUL:
            opcode = CMD_OVR_MUL;
            break;
        case ASMSTO_STODIV:
            opcode = CMD_OVR_DIV;
            break;
        }

        if(opcode == -1) {
            // ONLY UPDATE THE FLAGS WITH THE RESULT, IF THE RESULT IS NUMERIC
            if(rplIsFalse(DSTop[-ylevel]))
                rplSetSystemFlag(FL_ASMZERO);
            else
                rplClrSystemFlag(FL_ASMZERO);
            if(rplIsNegative(DSTop[-ylevel]))
                rplSetSystemFlag(FL_ASMNEG);
            else
                rplClrSystemFlag(FL_ASMNEG);
            DSTop[-ylevel] = argZ;
            if(skipnext) {
                ++IPtr;
                CurOpcode = *IPtr;
            }
            return;
        }
        if(opcode) {
            rplPushDataNoGrow(*destD);
            rplPushDataNoGrow(DSTop[-ylevel - 1]);
            if(ISNUMBERCPLX(*DSTop[-1]) && ISNUMBERCPLX(**destD))
                rplCallOperator(opcode);
            else
                rplRunAtomic(opcode);
            if(Exceptions)
                return;
            *destD = rplPopData();
        }
        else
            *destD = DSTop[-ylevel];

        DSTop[-ylevel] = argZ;

        if(skipnext) {
            ++IPtr;
            CurOpcode = *IPtr;
        }
        return;
    }
    default:

        switch (OPCODE(CurOpcode)) {

            // STANDARIZED OPCODES:
            // --------------------
            // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES

        case OPCODE_COMPILE:
            // COMPILE RECEIVES:
            // TokenStart = token string
            // TokenLen = token length
            // BlankStart = token blanks afterwards
            // BlanksLen = blanks length
            // CurrentConstruct = Opcode of current construct/WORD of current composite

            // COMPILE RETURNS:
            // RetNum =  enum CompileErrors
        {
            if(LIBNUM(CurOpcode) != LIBRARY_NUMBER)
                return; // IGNORE ALL OTHER ASSOCIATED LIBRARY NUMBERS
            BYTEPTR str = (BYTEPTR) TokenStart, endtoken;

            if(*str != ':') {
                RetNum = ERR_NOTMINE;   // MUST START WITH A COLON
                return;
            }

            ++str;

            while((*str != ':') && (str < (BYTEPTR) BlankStart))
                ++str;  // MUST NOT CONTAIN ANY OTHER COLON

            if(str != (BYTEPTR) BlankStart) {
                RetNum = ERR_NOTMINE;
                return;
            }

            str = (BYTEPTR) TokenStart;

            ++str;

            BINT storemode = 0, d = 0x20, y = 0x20, z = 0x20, operator= 0;
            // FIND END OF FIRST TOKEN
            BINT opcode_arg;
            endtoken = rplAsmEndOfToken(str, (BYTEPTR) BlankStart);
            if(!rplAsmDecodeToken(str, endtoken, &opcode_arg)) {
                RetNum = ERR_SYNTAX;
                return;
            }

            // POSSIBLE SYNTAX VARIANTS:
            // [D]?=[Y]
            // [D]?=[Y][OP][Z]
            // [D]?=[CMD].[Y].[Z]
            // [D]?=[CMD].[Y]
            // [D]?=[CMD]
            // [CMD].[Y].[Z]
            // [CMD].[Y]
            // [CMD]

            // THIS IS EITHER AN OPCODE OR A DESTINATION REGISTER
            d = opcode_arg & 0xff;
            operator=(opcode_arg >> 8) & 0xff;

            if(opcode_arg >> 16) {
                RetNum = ERR_SYNTAX;    // CANNOT START WITH ASSIGNMENT
                return;
            }
            if(operator &&(operator<= ASM_POW)) {
                RetNum = ERR_SYNTAX;    // BINARY OPERATORS CANNOT BE AT THE BEGINNING
                return;
            }

            // MOVE ON TO THE SECOND TOKEN
            str = endtoken;
            endtoken = rplAsmEndOfToken(str, (BYTEPTR) BlankStart);

            if(!rplAsmDecodeToken(str, endtoken, &opcode_arg)) {
                if(str != (BYTEPTR) BlankStart) {
                    RetNum = ERR_SYNTAX;
                    return;
                }
                // THE OPCODE ENDED HERE
            }
            else {
                // WE HAVE A SECOND TOKEN
                storemode = (opcode_arg >> 16) & 0xff;

                if(!storemode) {
                    if(!(d & 0x20))     // DESTINATION WAS PROVIDED, BUT NO STORAGE OPERATOR, SO DESTINATION WAS ACTUALLY THE FIRST ARGUMENT OF AN EXPRESSION
                    {

                        y = d;  // NO DESTINATION, THAT WAS THE Y ARGUMENT

                        d = 0x20;

                        // THIS MAY BE AN OPERATOR
                        if(operator &&(opcode_arg & 0xff00)) {
                            RetNum = ERR_SYNTAX;        // CAN'T HAVE A SECOND OPERATOR!
                            return;
                        }

                        operator=(opcode_arg >> 8) & 0xff;

                    }
                    else {
                        if(opcode_arg & 0xff00) {
                            RetNum = ERR_SYNTAX;        // CAN'T HAVE A SECOND OPERATOR!
                            return;
                        }
                        y = opcode_arg & 0xff;
                    }
                    // MOVE ON TO THE THIRD TOKEN
                    str = endtoken;
                    endtoken = rplAsmEndOfToken(str, (BYTEPTR) BlankStart);

                    if(!rplAsmDecodeToken(str, endtoken, &opcode_arg)) {
                        if(str != (BYTEPTR) BlankStart) {
                            RetNum = ERR_SYNTAX;
                            return;
                        }
                        // THE OPCODE ENDED HERE
                    }
                    else {
                        // WE HAVE A THIRD TOKEN, IT CAN ONLY BE THE ARGUMENT Z
                        if(opcode_arg & ~0xff) {
                            RetNum = ERR_SYNTAX;
                            return;
                        }
                        z = opcode_arg & 0xff;

                        if(endtoken != (BYTEPTR) BlankStart) {
                            RetNum = ERR_SYNTAX;        // AND IT NEEDS TO BE THE LAST TOKEN
                            return;
                        }

                    }

                    // DONE, WE HAVE DECODED A COMPLETE COMAND IN THE FORM:
                    // [CMD].[Y].[Z]
                    // [CMD].[Y]
                    // [CMD]

                }
                else {
                    // SYNTAX WITH STORAGE, SO FAR WE HAVE A DESTINATION AND THE STORAGE OPERATOR
                    if(d & 0x20) {
                        RetNum = ERR_SYNTAX;    // STORAGE OPERATOR WITHOUT DESTINATION
                        return;
                    }

                    // MOVE ON TO THE NEXT OPCODE
                    str = endtoken;
                    endtoken = rplAsmEndOfToken(str, (BYTEPTR) BlankStart);

                    if(!rplAsmDecodeToken(str, endtoken, &opcode_arg)) {
                        RetNum = ERR_SYNTAX;
                        return;
                    }

                    if(opcode_arg & ~0xffff) {
                        RetNum = ERR_SYNTAX;    // CANNOT BE A STORAGE OPERATOR AGAIN
                        return;
                    }

                    // IT CAN BE AN OPERATOR OR THE ARGUMENT Y
                    y = opcode_arg & 0xff;
                    operator=(opcode_arg >> 8) & 0xff;

                    if(operator &&(operator<= ASM_POW)) {
                        RetNum = ERR_SYNTAX;    // BINARY OPERATOR BEFORE THE ARGUMENT
                        return;
                    }

                    // MOVE ON TO THE NEXT TOKEN
                    str = endtoken;
                    endtoken = rplAsmEndOfToken(str, (BYTEPTR) BlankStart);

                    if(!rplAsmDecodeToken(str, endtoken, &opcode_arg)) {
                        if(str != (BYTEPTR) BlankStart) {
                            RetNum = ERR_SYNTAX;
                            return;
                        }
                        // THE OPCODE ENDED HERE

                    }
                    else {
                        // WE HAVE ANOTHER TOKEN!
                        // [D]?=[Y]   --> [OP][Z]
                        // [D]?=[CMD].  --> [Y].[Z]
                        if(!operator) {
                            operator=(opcode_arg >> 8) & 0xff;
                            if(!operator ||(operator> ASM_POW)) {
                                RetNum = ERR_SYNTAX;    // NOT A BINARY OPERATOR!
                                return;
                            }
                        }
                        // GET Y OR Z SINCE [CMD] MAY HAVE USED Y ALREADY
                        if(y & 0x20) {
                            y = opcode_arg & 0xff;
                            // MOVE ON TO THE NEXT ARGUMENT
                            str = endtoken;
                            endtoken =
                                    rplAsmEndOfToken(str, (BYTEPTR) BlankStart);

                            if(!rplAsmDecodeToken(str, endtoken, &opcode_arg)) {
                                if(str != (BYTEPTR) BlankStart) {
                                    RetNum = ERR_SYNTAX;
                                    return;
                                }
                                // THE OPCODE ENDED HERE

                            }
                            else {
                                // FINAL ARGUMENT, ONLY FOR THESE CASES:
                                // [D]?=[Y][OP][Z]
                                // [D]?=[CMD].[Y].[Z]
                                z = opcode_arg & 0xff;

                                if(endtoken != (BYTEPTR) BlankStart) {
                                    RetNum = ERR_SYNTAX;        // SHOULD BE THE LAST ARGUMENT
                                    return;
                                }
                            }

                        }
                        else {
                            if(!(opcode_arg & 0x20)) {
                                z = opcode_arg & 0xff;
                                if(opcode_arg & ~0xff) {
                                    RetNum = ERR_SYNTAX;        // CANNOT BE AN OPERATOR OR STORAGE, MUST BE AN ARGUMENT
                                    return;
                                }
                                if(endtoken != (BYTEPTR) BlankStart) {
                                    RetNum = ERR_SYNTAX;        // SHOULD BE THE LAST ARGUMENT
                                    return;
                                }
                            }
                            else {
                                // MOVE ON TO THE NEXT ARGUMENT
                                str = endtoken;
                                endtoken =
                                        rplAsmEndOfToken(str,
                                        (BYTEPTR) BlankStart);

                                if(!rplAsmDecodeToken(str, endtoken,
                                            &opcode_arg)) {
                                    if(str != (BYTEPTR) BlankStart) {
                                        RetNum = ERR_SYNTAX;
                                        return;
                                    }
                                    // THE OPCODE ENDED HERE

                                }
                                else {
                                    // FINAL ARGUMENT, ONLY FOR THESE CASES:
                                    // [D]?=[Y][OP][Z]
                                    // [D]?=[CMD].[Y].[Z]
                                    z = opcode_arg & 0xff;

                                    if(endtoken != (BYTEPTR) BlankStart) {
                                        RetNum = ERR_SYNTAX;    // SHOULD BE THE LAST ARGUMENT
                                        return;
                                    }
                                }

                            }
                        }

                    }
                }
            }

            // COMMAND WAS FULLY DECODED, CONSTRUCT THE OPCODE

            // POSSIBLE SYNTAX VARIANTS:
            // [Y]
            if(!(d & 0x20) && !storemode && (y & 0x20) && (z & 0x20)
                    && !operator) {
                y = d;
                d = 0x20;
                operator= ASM_LDY;
            }

            // [D]?=[Y]
            if(!(y & 0x20) && (z & 0x20) && !operator)
                operator= ASM_LDY;
            // [D]?=[Y][OP][Z]
            if((operator>= ASM_ADD) && (operator<= ASM_POW)) {
                if((y & 0x20) || (z & 0x20)) {
                    RetNum = ERR_SYNTAX;
                    return;
                }
            }
            // [D]?=[CMD].[Y].[Z]
            // [D]?=[CMD].[Y]
            // [D]?=[CMD]
            // [CMD].[Y].[Z]
            // [CMD].[Y]
            // [CMD]

            // LOOP INSTRUCTION NEEDS A NOOP BEFORE
            if(operator== ASM_LOOP)
                rplCompileAppend(MKOPCODE(LIBRARY_NUMBER + ASMSTO_NOOP,
                            0x3000 | (ASM_NOOP << 14)));

            WORD final_opcode =
                    (z & 0xf) | ((z & 0x10) << 8) | ((y & 0xf) << 4) | ((y &
                        0x10) << 9) | ((d & 0xf) << 8) | ((operator& 0x1f) <<
                    14);

            rplCompileAppend(MKOPCODE(LIBRARY_NUMBER + storemode,
                        final_opcode));

            RetNum = OK_CONTINUE;
            return;
        }
        case OPCODE_DECOMPEDIT:

        case OPCODE_DECOMPILE:
        {
            // DECOMPILE RECEIVES:
            // DecompileObject = Ptr to prolog of object to decompile
            // DecompStringEnd = Ptr to the end of decompile string

            //DECOMPILE RETURNS
            // RetNum =  enum DecompileErrors

            // POSSIBLE SYNTAX VARIANTS:
            // [D]?=[Y]
            // [D]?=[Y][OP][Z]
            // [D]?=[CMD].[Y].[Z]
            // [D]?=[CMD].[Y]
            // [D]?=[CMD]
            // [CMD].[Y].[Z]
            // [CMD].[Y]
            // [CMD]

            WORD op = *DecompileObject;

            BINT storemode = LIBNUM(op) - LIBRARY_NUMBER;
            BINT operator=(op >> 14) & 0x1f;
            BINT noy = 0, noz = 0, yflags = 0;

            if(operator== ASM_NOOP) {
                RetNum = OK_CONTINUE;   // NOOP IS A SILENT OPCODE USED FOR LOOP INTERNALLY
                return;
            }

            rplDecompAppendChar(':');

            if(storemode) {
                BINT d = (op >> 8) & 0xf;
                if(d == 8)
                    rplDecompAppendChar('P');
                else if(d > 8) {
                    rplDecompAppendChar('S');
                    rplDecompAppendChar('0' + d - 8);
                }
                else
                    rplDecompAppendChar('A' + d);

                switch (storemode) {
                case ASMSTO_STO:
                    rplDecompAppendChar('=');
                    break;
                case ASMSTO_STOADD:
                    rplDecompAppendChar('+');
                    rplDecompAppendChar('=');
                    break;
                case ASMSTO_STOSUB:
                    rplDecompAppendChar('-');
                    rplDecompAppendChar('=');
                    break;
                case ASMSTO_STOMUL:
                    rplDecompAppendChar('*');
                    rplDecompAppendChar('=');
                    break;
                case ASMSTO_STODIV:
                    rplDecompAppendChar('/');
                    rplDecompAppendChar('=');
                    break;
                default:
                    rplDecompAppendChar('.');
                }
            }

            if(operator== ASM_LDY)
                noz = 1;
            if(operator> ASM_POW) {
                switch (operator) {
                case ASM_CMP:
                    rplDecompAppendString((BYTEPTR) "CMP.");
                    break;
                case ASM_POP:
                    rplDecompAppendString((BYTEPTR) "POP.");
                    break;

                case ASM_RPOP:
                    rplDecompAppendString((BYTEPTR) "RPOP.");
                    break;

                case ASM_PUSH:
                    rplDecompAppendString((BYTEPTR) "PUSH.");
                    break;
                case ASM_RPUSH:
                    rplDecompAppendString((BYTEPTR) "RPUSH.");
                    break;

                case ASM_GET:
                    rplDecompAppendString((BYTEPTR) "GET.");
                    break;
                case ASM_PUT:
                    rplDecompAppendString((BYTEPTR) "PUT.");
                    break;
                case ASM_MATH:
                {
                    noy = 1;

                    BINT y = (op >> 4) & 0xf;
                    switch (y) {
                    case ASM_MATH_IP:
                        rplDecompAppendString((BYTEPTR) "IP");
                        break;
                    case ASM_MATH_LN:
                        rplDecompAppendString((BYTEPTR) "LN");
                        break;
                    case ASM_MATH_EXP:
                        rplDecompAppendString((BYTEPTR) "EXP");
                        break;

                    case ASM_MATH_SQRT:
                        rplDecompAppendString((BYTEPTR) "SQRT");
                        break;

                    case ASM_MATH_SIN:
                        rplDecompAppendString((BYTEPTR) "SIN");
                        break;

                    case ASM_MATH_COS:
                        rplDecompAppendString((BYTEPTR) "COS");
                        break;

                    case ASM_MATH_TAN:
                        rplDecompAppendString((BYTEPTR) "TAN");
                        break;

                    case ASM_MATH_ASIN:
                        rplDecompAppendString((BYTEPTR) "ASIN");
                        break;

                    case ASM_MATH_ACOS:
                        rplDecompAppendString((BYTEPTR) "ACOS");
                        break;

                    case ASM_MATH_ATAN:
                        rplDecompAppendString((BYTEPTR) "ATAN");
                        break;

                    case ASM_MATH_SINH:
                        rplDecompAppendString((BYTEPTR) "SINH");
                        break;

                    case ASM_MATH_COSH:
                        rplDecompAppendString((BYTEPTR) "COSH");
                        break;

                    case ASM_MATH_TANH:
                        rplDecompAppendString((BYTEPTR) "TANH");
                        break;

                    case ASM_MATH_ASINH:
                        rplDecompAppendString((BYTEPTR) "ASINH");
                        break;

                    case ASM_MATH_ACOSH:
                        rplDecompAppendString((BYTEPTR) "ACOSH");
                        break;

                    case ASM_MATH_ATANH:
                        rplDecompAppendString((BYTEPTR) "ATANH");
                        break;

                    }
                    break;
                }

                case ASM_MATH2:
                {
                    noy = 1;

                    BINT y = (op >> 4) & 0xf;
                    switch (y) {
                    case ASM_MATH2_FP:
                        rplDecompAppendString((BYTEPTR) "FP");
                        break;
                    case ASM_MATH2_ABS:
                        rplDecompAppendString((BYTEPTR) "ABS");
                        break;
                    case ASM_MATH2_ARG:
                        rplDecompAppendString((BYTEPTR) "ARG");
                        break;
                    case ASM_MATH2_RE:
                        rplDecompAppendString((BYTEPTR) "RE");
                        break;
                    case ASM_MATH2_IM:
                        rplDecompAppendString((BYTEPTR) "IM");
                        break;

                    }
                    break;
                }

                case ASM_LOOP:
                    noz = 1;
                    yflags = 1;
                    rplDecompAppendString((BYTEPTR) "LOOP.");
                    break;

                case ASM_SKIP:
                    noz = 1;
                    yflags = 1;
                    rplDecompAppendString((BYTEPTR) "SKIP.");
                    break;

                case ASM_CHK:
                    noz = 1;
                    yflags = 1;
                    rplDecompAppendString((BYTEPTR) "CHK.");
                    break;
                case ASM_MIN:
                    rplDecompAppendString((BYTEPTR) "MIN.");
                    break;
                case ASM_MAX:
                    rplDecompAppendString((BYTEPTR) "MAX.");
                    break;
                case ASM_RND:
                    rplDecompAppendString((BYTEPTR) "RND.");
                    break;
                case ASM_AND:
                    rplDecompAppendString((BYTEPTR) "AND.");
                    break;
                case ASM_OR:
                    rplDecompAppendString((BYTEPTR) "OR.");
                    break;
                case ASM_XOR:
                    rplDecompAppendString((BYTEPTR) "XOR.");
                    break;
                case ASM_CLR:
                    rplDecompAppendString((BYTEPTR) "CLR.");
                    break;
                case ASM_SGET:
                    noz = 1;
                    rplDecompAppendString((BYTEPTR) "SGET.");
                    break;
                case ASM_SPUT:
                    rplDecompAppendString((BYTEPTR) "SPUT.");
                    break;

                }
            }

            if(!noy) {
                BINT y = (op >> 4) & 0xf;
                if(op & 0x2000) {
                    // Y IS A LITERAL VALUE
                    if(yflags) {
                        switch (y & 7) {
                        case ASMF_AL:  // (always)
                            rplDecompAppendString((BYTEPTR) "AL");
                            break;
                        case ASMF_LT:  // (NEG flag)
                            rplDecompAppendString((BYTEPTR) "LT");
                            break;
                        case ASMF_EQ:  // (ZERO flag)
                            rplDecompAppendString((BYTEPTR) "EQ");
                            break;
                        case ASMF_LTE: // (NEG || ZERO)
                            rplDecompAppendString((BYTEPTR) "LTE");
                            break;
                        case ASMF_NA:  // (NOT ALWAYS = NEVER)
                            rplDecompAppendString((BYTEPTR) "NV");
                            break;
                        case ASMF_GTE: // (NOT LT)
                            rplDecompAppendString((BYTEPTR) "GTE");
                            break;
                        case ASMF_NE:  // (NOT EQ)
                            rplDecompAppendString((BYTEPTR) "NE");
                            break;
                        case ASMF_GT:  // (NOT LTE)
                            rplDecompAppendString((BYTEPTR) "GT");
                            break;

                        }

                    }
                    else {
                        // LITERAL VALUE IN NUMERIC FORM
                        rplDecompAppendChar('#');
                        if(y < 10)
                            rplDecompAppendChar('0' + y);
                        else {
                            rplDecompAppendChar('1');
                            rplDecompAppendChar('0' + y - 10);
                        }
                    }
                }
                else {
                    // IT'S A REFERENCE
                    if(y == 8)
                        rplDecompAppendChar('R');
                    else if(y > 8) {
                        rplDecompAppendChar('S');
                        rplDecompAppendChar('0' + y - 8);
                    }
                    else
                        rplDecompAppendChar('A' + y);
                }

            }

            // HERE WE OUTPUT [D]?=([OP]).[Y] --> [op] [Z]

            if((operator>= ASM_ADD) && (operator<= ASM_POW)) {
                switch (operator) {
                case ASM_ADD:
                    rplDecompAppendChar('+');
                    break;
                case ASM_SUB:
                    rplDecompAppendChar('-');
                    break;
                case ASM_MUL:
                    rplDecompAppendChar('*');
                    break;
                case ASM_DIV:
                    rplDecompAppendChar('/');
                    break;
                case ASM_POW:
                    rplDecompAppendChar('^');
                    break;
                }
            }
            else if(!noz)
                rplDecompAppendChar('.');

            if(!noz) {
                BINT z = (op) & 0xf;
                if(op & 0x1000) {
                    // Z IS A LITERAL VALUE

                    // LITERAL VALUE IN NUMERIC FORM
                    rplDecompAppendChar('#');
                    if(z < 10)
                        rplDecompAppendChar('0' + z);
                    else {
                        rplDecompAppendChar('1');
                        rplDecompAppendChar('0' + z - 10);
                    }

                }
                else {
                    // IT'S A REFERENCE
                    if(z == 8)
                        rplDecompAppendChar('R');
                    else if(z > 8) {
                        rplDecompAppendChar('S');
                        rplDecompAppendChar('0' + z - 8);
                    }
                    else
                        rplDecompAppendChar('A' + z);
                }

            }

            RetNum = OK_CONTINUE;

            return;
        }
        case OPCODE_VALIDATE:
            // VALIDATE RECEIVES OPCODES COMPILED BY OTHER LIBRARIES, TO BE INCLUDED WITHIN A COMPOSITE OWNED BY
            // THIS LIBRARY. EVERY COMPOSITE HAS TO EVALUATE IF THE OBJECT BEING COMPILED IS ALLOWED INSIDE THIS
            // COMPOSITE OR NOT. FOR EXAMPLE, A REAL MATRIX SHOULD ONLY ALLOW REAL NUMBERS INSIDE, ANY OTHER
            // OPCODES SHOULD BE REJECTED AND AN ERROR THROWN.
            // Library receives:
            // CurrentConstruct = SET TO THE CURRENT ACTIVE CONSTRUCT TYPE
            // LastCompiledObject = POINTER TO THE LAST OBJECT THAT WAS COMPILED, THAT NEEDS TO BE VERIFIED

            // VALIDATE RETURNS:
            // RetNum =  OK_CONTINUE IF THE OBJECT IS ACCEPTED, ERR_INVALID IF NOT.

            RetNum = OK_CONTINUE;
            return;

        case OPCODE_PROBETOKEN:
            // PROBETOKEN FINDS A VALID WORD AT THE BEGINNING OF THE GIVEN TOKEN AND RETURNS
            // INFORMATION ABOUT IT. THIS OPCODE IS MANDATORY

            // COMPILE RECEIVES:
            // TokenStart = token string
            // TokenLen = token length
            // BlankStart = token blanks afterwards
            // BlanksLen = blanks length
            // CurrentConstruct = Opcode of current construct/WORD of current composite

            // COMPILE RETURNS:
            // RetNum =  OK_TOKENINFO | MKTOKENINFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
            // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND
        {
            RetNum = ERR_NOTMINE;

            return;
        }

        case OPCODE_GETINFO:
            // THIS OPCODE RECEIVES A POINTER TO AN RPL COMMAND OR OBJECT IN ObjectPTR
            // NEEDS TO RETURN INFORMATION ABOUT THE TYPE:
            // IN RetNum: RETURN THE MKTOKENINFO() DATA FOR THE SYMBOLIC COMPILER AND CAS
            // IN DecompHints: RETURN SOME HINTS FOR THE DECOMPILER TO DO CODE BEAUTIFICATION (TO BE DETERMINED)
            // IN TypeInfo: RETURN TYPE INFORMATION FOR THE TYPE COMMAND
            //             TypeInfo: TTTTFF WHERE TTTT = MAIN TYPE * 100 (NORMALLY THE MAIN LIBRARY NUMBER)
            //                                FF = 2 DECIMAL DIGITS FOR THE SUBTYPE OR FLAGS (VARIES DEPENDING ON LIBRARY)
            //             THE TYPE COMMAND WILL RETURN A REAL NUMBER TypeInfo/100
            // FOR NUMBERS: TYPE=10 (REALS), SUBTYPES = .01 = APPROX., .02 = INTEGER, .03 = APPROX. INTEGER
            // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL BINT, .42 = HEX INTEGER

            TypeInfo = 0;       // ALL COMMANDS ARE TYPE 0
            DecompHints = 0;
            RetNum = OK_TOKENINFO | MKTOKENINFO(0, TITYPE_NOTALLOWED, 0, 1);

            return;

        case OPCODE_GETROMID:
            // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
            // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
            // ObjectPTR = POINTER TO ROM OBJECT
            // LIBBRARY RETURNS: ObjectID=new ID, ObjectIDHash=hash, RetNum=OK_CONTINUE
            // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED
            RetNum = ERR_NOTMINE;
            return;
        case OPCODE_ROMID2PTR:
            // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
            // ObjectID = ID, ObjectIDHash=hash
            // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
            // OR RetNum= ERR_NOTMINE;

            RetNum = ERR_NOTMINE;
            return;

        case OPCODE_CHECKOBJ:
            // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
            // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
            // ObjectPTR = POINTER TO THE OBJECT TO CHECK
            // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
            if(ISPROLOG(*ObjectPTR)) {
                RetNum = ERR_INVALID;
                return;
            }

            RetNum = OK_CONTINUE;
            return;

        case OPCODE_AUTOCOMPNEXT:
            RetNum = ERR_NOTMINE;
            return;

        case OPCODE_LIBMENU:
            // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
            // MUST RETURN A MENU LIST IN ObjectPTR
            // AND RetNum=OK_CONTINUE;
        {
            RetNum = ERR_NOTMINE;
            return;
        }

        case OPCODE_LIBHELP:
            // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
            // MUST RETURN A STRING OBJECT IN ObjectPTR
            // AND RetNum=OK_CONTINUE;
        {
            RetNum = ERR_NOTMINE;
            return;
        }

        case OPCODE_LIBMSG:
            // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
            // MUST RETURN A STRING OBJECT IN ObjectPTR
            // AND RetNum=OK_CONTINUE;
        {

            RetNum = ERR_NOTMINE;
            return;
        }

        case OPCODE_LIBINSTALL:
            LibraryList = (WORDPTR) libnumberlist;
            RetNum = OK_CONTINUE;
            return;
        case OPCODE_LIBREMOVE:
            return;

        }
        // UNHANDLED OPCODE...

    }

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode) >= MIN_RESERVED_OPCODE) {
        RetNum = ERR_NOTMINE;
        return;
    }

    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    rplError(ERR_INVALIDOPCODE);

    return;

}

#endif
