/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// LIBRARIES TO BE INCLUDED IN ROM
#define ROM_LIST \
    INCLUDELIB(2,lib-two-ident.c) , \
    INCLUDELIB(8,lib-eight-docol.c) , \
    INCLUDELIB(9,lib-nine-docol.c) , \
    INCLUDELIB(10,a) , \
    INCLUDELIB(12,a) , \
    INCLUDELIB(20,lib-20-comments.c) , \
    INCLUDELIB(24,a) , \
    INCLUDELIB(26,a) , \
    INCLUDELIB(28,a) , \
    INCLUDELIB(30,a) , \
    INCLUDELIB(32,a) , \
    INCLUDELIB(52,a) , \
    INCLUDELIB(54,a) , \
    INCLUDELIB(56,a) , \
    INCLUDELIB(62,a) , \
    INCLUDELIB(64,a) , \
    INCLUDELIB(65,a) , \
    INCLUDELIB(66,a) , \
    INCLUDELIB(68,a) , \
    INCLUDELIB(70,a) , \
    INCLUDELIB(72,a) , \
    INCLUDELIB(4080,a) , \
    INCLUDELIB(4090,a)


#ifndef COMMANDS_ONLY_PASS


#include "libraries.h"
#include "newrpl.h"


// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE switch() DISPATCHER
#define INCLUDELIB(a,b) lib##a##_handler
// THIS IS THE LIST OF ALL PRE-INSTALLED LIBRARIES IN THIS ROM
const LIBHANDLER const ROMLibs[]={ ROM_LIST , 0 };
#undef INCLUDELIB

#else


#define INCLUDELIB(a,b) b
#define LIBRARY_LIST ROM_LIST

// ITERATE OVER ALL LIBRARIES
#include "romlib-loop.h"

#undef LIBRARY_LIST
#undef INCLUDELIB


#endif

