#include "newrpl.h"
#include "libraries.h"



// GLOBAL SUPPORT FUNCTIONS FOR SYMBOLICS


/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */



/* COMPILING A SYMBOLIC:
 *
 * Compiler will switch to infix mode with the return value: OK_STARTCONSTRUCT_INFIX
 * And will return to RPN on ENDCONSTRUCT.
 *
 * In infix mode, the compiler sends OPCODE_MAXTOKEN to all libraries.
 * Libraries must determine if the token string starts with a token provided by the library.
 * Libraries reply with OK_TOKENINFO + MKTOKENINFO(precedence,nargs,length), with length=maximum
 * number of characters that the compiled token will absorb (length<=TokenLen)
 * At the same time, libraries must return the precedence of the compiled token they detected and
 * the number of arguments that this operator/function needs from the stack, and whether it is left
 * or right associative.
 * The compiler will choose the library that absorbs the most characters, will split the token
 * and pass the new token to the library to compile using OPCODE_COMPILE.
 *
 *
 */

/* Operators precedence should be:

31 = COMPLEX IDENT
30 = REAL IDENTS
29 = CONSTANT IDENT
28 = COMPOSITE OBJECT
27 = NUMERIC TYPES

20 = ALL OTHER FUNCTIONS AND COMMANDS

16 = OVR_EVAL, OVR_XEQ

12= OVR_OR
11= OVR_XOR
10= OVR_AND
9= OVR_EQ, OVR_NOTEQ, OVR_SAME
8= OVR_LT OVR_GT OVR_LTE OVR_GTE

6= OVR_ADD, OVR_SUB

5= OVR_MUL,OVR_DIV,OVR_INV

4= OVR_ABS,OVR_POW

3= OVR_NEG, OVR_ISTRUE, OVR_NOT

2= BRACKETS/PARENTHESIS

*/
