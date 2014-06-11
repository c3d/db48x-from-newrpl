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


extern void lib50_handler();


extern void lib64_handler();
extern void lib65_handler();
extern void lib66_handler();




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

LIBHANDLER ROMLibs[]={
    dummy_libhandler,
    dummy_libhandler,
    lib2_handler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    lib8_handler,
    dummy_libhandler,
    lib10_handler,
    // LIB 11 IS REAL NUMBERS
    lib11_handler,
    // LIBS 12 THRU 15 ARE BINTS
    lib12_handler,
    lib12_handler,
    lib12_handler,
    lib12_handler,

    // LIB 16 THRU 19 ARE STRING
    lib16_handler,
    lib16_handler,
    lib16_handler,
    lib16_handler,

    // LIB 20 AND 21 ARE IDENTS
    lib20_handler,
    lib20_handler,

    // DIRECTORIES
    lib22_handler,

    dummy_libhandler,

    // STACK COMMANDS
    lib24_handler,

    dummy_libhandler,

    dummy_libhandler,


    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,

    dummy_libhandler,   // THIS IS LIB30
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,

    dummy_libhandler,   // THIS IS LIB40
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,

    // LISTS
    lib50_handler,     // THIS IS LIB 50
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,

    dummy_libhandler,   // THIS IS LIB60
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,
    lib64_handler,
    lib65_handler,
    lib66_handler,
    dummy_libhandler,
    dummy_libhandler,
    dummy_libhandler,

    // ADD MORE LIBRARIES HERE
    0
};


// HIGH LIBRARIES

LIBHANDLER ROMLibs2[]={
    lib4080_handler,
    lib4090_handler,
    0
};

BINT ROMLibs2Num[]={
    4080,
    4090,
    0
};
