/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// LIBRARIES TO BE INCLUDED IN ROM
#define ROM_LIST \
    INCLUDELIB(2,lib-two-ident.c) _COMMA \
    INCLUDELIB(8,lib-eight-docol.c) _COMMA \
    INCLUDELIB(9,lib-nine-docol.c) _COMMA \
    INCLUDELIB(10,a) _COMMA \
    INCLUDELIB(12,a) _COMMA \
    INCLUDELIB(20,lib-20-comments.c) _COMMA \
    INCLUDELIB(24,a) _COMMA \
    INCLUDELIB(26,a) _COMMA \
    INCLUDELIB(28,a) _COMMA \
    INCLUDELIB(30,a) _COMMA \
    INCLUDELIB(32,a) _COMMA \
    INCLUDELIB(52,a) _COMMA \
    INCLUDELIB(54,a) _COMMA \
    INCLUDELIB(56,a) _COMMA \
    INCLUDELIB(62,a) _COMMA \
    INCLUDELIB(64,a) _COMMA \
    INCLUDELIB(65,a) _COMMA \
    INCLUDELIB(66,a) _COMMA \
    INCLUDELIB(68,a) _COMMA \
    INCLUDELIB(70,a) _COMMA \
    INCLUDELIB(72,a) _COMMA \
    INCLUDELIB(4080,a) _COMMA \
    INCLUDELIB(4090,a)


#ifndef COMMANDS_ONLY_PASS


#include "libraries.h"
#include "newrpl.h"


// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE switch() DISPATCHER
#define _COMMA ,
#define INCLUDELIB(a,b) lib##a##_handler
// THIS IS THE LIST OF ALL PRE-INSTALLED LIBRARIES IN THIS ROM
const LIBHANDLER const ROMLibs[]={ ROM_LIST , 0 };
#undef L

#else

#define _COMMA
#define INCLUDELIB(a,b) #b

#define HEAD(first, ... ) first
#define TAIL(first, ... ) __VA_ARGS__
#define ARGLIST( ... ) __VA_ARGS__ , ENDOFLIST

// ITERATE OVER ALL LIBRARIES
#include "romlib-loop.h"

#undef __COMMA
#undef _COMMA
#undef INCLUDELIB

#ifndef CMD_STRIPCOMMENTS
#error Not working!
#endif

#endif


