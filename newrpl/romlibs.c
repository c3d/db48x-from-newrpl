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
extern void lib9_handler();
extern void lib10_handler();
extern void lib12_handler();
extern void lib20_handler();
extern void lib24_handler();
extern void lib26_handler();
extern void lib28_handler();
extern void lib30_handler();

extern void lib32_handler();



extern void lib52_handler();

extern void lib58_handler();

extern void lib60_handler();
extern void lib62_handler();

extern void lib64_handler();
extern void lib65_handler();
extern void lib66_handler();
extern void lib68_handler();

extern void lib70_handler();
extern void lib72_handler();



extern void lib4080_handler();
extern void lib4090_handler();


void dummy_libhandler()
{
if(ISPROLOG(CurOpcode)) {
    rplError(ERR_UNRECOGNIZEDOBJECT);
    return;
}

switch(OPCODE(CurOpcode))
        {

// STANDARIZED OPCODES:
        // --------------------
        // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES

        case OPCODE_LIBINSTALL:
        RetNum=ERR_INVALID;
        return;

        }

        RetNum=ERR_INVALID;
        rplError(ERR_INVALIDOPCODE);

        return;


}




// THIS IS THE LIST OF ALL PRE-INSTALLED LIBRARIES IN THIS ROM

const LIBHANDLER const ROMLibs[]={
    lib2_handler,
    lib8_handler,
    lib9_handler,
    // LIB 10 IS REAL NUMBERS
    lib10_handler,
    // LIBS 12 THRU 19 ARE BINTS
    lib12_handler,

    // LIB 24 THRU 27 ARE STRINGS
    lib24_handler,

    // DIRECTORIES
    lib28_handler,

    // COMPLEX NUMBERS
    lib30_handler,

    // LIB 32 THRU 47 ARE IDENTS
    lib32_handler,

    // SYMBOLICS
    lib52_handler,

    // MATRIX
    lib58_handler,

    // UNITS
    lib60_handler,

    // LISTS
    lib62_handler,

    lib64_handler,
    lib65_handler,
    lib66_handler,
    lib68_handler,
    lib70_handler,
    lib72_handler,

    lib4080_handler,
    lib4090_handler,

    // ADD MORE LIBRARIES HERE
    0
};
