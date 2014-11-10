/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "libraries.h"
#include "newrpl.h"



extern void lib2_handler();
extern void lib8_handler();
extern void lib10_handler();
extern void lib11_handler();
extern void lib12_handler();
extern void lib16_handler();
extern void lib20_handler();
extern void lib22_handler();
extern void lib24_handler();
extern void lib26_handler();
extern void lib28_handler();


extern void lib50_handler();


extern void lib64_handler();
extern void lib65_handler();
extern void lib66_handler();
extern void lib68_handler();




extern void lib4080_handler();
extern void lib4090_handler();

void dummy_libhandler()
{
if(ISPROLOG(CurOpcode)) return;


switch(OPCODE(CurOpcode))
        {

// STANDARIZED OPCODES:
        // --------------------
        // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES


        case OPCODE_COMPILE:
               RetNum=ERR_NOTMINE;
               return;

        case OPCODE_DECOMPILE:
                RetNum=ERR_INVALID;
                return;
        }
}




// THIS IS THE LIST OF ALL PRE-INSTALLED LIBRARIES IN THIS ROM

const LIBHANDLER const ROMLibs[]={
    lib2_handler,
    lib8_handler,
    lib10_handler,
    // LIB 11 IS REAL NUMBERS
    lib11_handler,
    // LIBS 12 THRU 15 ARE BINTS
    lib12_handler,

    // LIB 16 THRU 19 ARE STRING
    lib16_handler,

    // LIB 20 AND 21 ARE IDENTS
    lib20_handler,

    // DIRECTORIES
    lib22_handler,

    // STACK COMMANDS
    lib24_handler,

    // COMPLEX NUMBERS
    lib26_handler,


    // SYMBOLICS
    lib28_handler,


    // LISTS
    lib50_handler,     // THIS IS LIB 50

    lib64_handler,
    lib65_handler,
    lib66_handler,
    lib68_handler,

    lib4080_handler,
    lib4090_handler,

    // ADD MORE LIBRARIES HERE
    0
};
