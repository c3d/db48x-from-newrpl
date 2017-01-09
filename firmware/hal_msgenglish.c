/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "ui.h"

#ifdef LANG_ENGLISH

// THIS MODULE CONTAINS ALL MESSAGES IN THE GUI
// TRANSLATED TO A SPECIFIC LANGUAGE

// ONLY ONE LANGUAGE MODULE SHOULD BE COMPILED INTO THE ROM


// THIS IS A NULL-TERMINATED TABLE OF MESSAGES
// MESSAGES CAN BE IN ANY ORDER

const MSGLIST const all_messages[]={
    // BASIC UNKNOWN ERROR MESSAGE
    { 1, "Unknown error" },


    // PROGRAM EXECUTION ERRORS
    //{ ERR_PROGRAMEXPECTED, "Expected a program" },

    // IDENTIFIERS AND VARIABLES
    //{ ERR_IDENTEXPECTED, "Expected an identifier" },
    //{ ERR_UNDEFINEDVARIABLE, "Undefined variable" },
    //{ ERR_CIRCULARREFERENCE, "Circular reference" },

    // DIRECTORIES AND VARIABLES

    // INTEGER NUMBERS

    // REAL NUMBERS

    // COMPLEX NUMBERS


    // STRINGS
//    { ERR_STRINGEXPECTED, "Expected a string" },
//    { ERR_INVALIDCODEPOINT, "Invalid Unicode code point" },

    // UNITS
//    { ERR_UNITEXPECTED, "Expected an object with units" },
//    { ERR_INCONSISTENTUNITS, "Inconsistent units" },
//    { ERR_INVALIDUNITDEFINITION, "Invalid unit expression" },
//    { ERR_EXPECTEDREALEXPONENT, "Expected real/fraction exponent" },
//    { ERR_INVALIDUNITNAME, "Invalid unit name" },
//    { ERR_UNDEFINEDUNIT, "Unit not defined" },


    // LISTS
//    { ERR_LISTEXPECTED, "Expected a list" },
//    { ERR_INDEXOUTOFBOUNDS, "Index out of bounds" },
//    { ERR_EMPTYLIST, "List is empty" },
//    { ERR_INVALIDLISTSIZE, "Invalid list size" },

    // SYMBOLICS
    { ERR_SYMBOLICEXPECTED, "Expected a symbolic" },
    { ERR_NOTAVALIDRULE, "Not a valid rule" },
    { ERR_INVALIDUSERDEFINEDFUNCTION, "Invalid user-defined function" },


    // MATRIX
    { ERR_MATRIXEXPECTED, "Expected a matrix" },
    { ERR_INVALIDDIMENSION, "Invalid dimensions" },
    { ERR_NOTALLOWEDINMATRIX, "Not allowed in matrix" },
    { ERR_INCOMPATIBLEDIMENSION, "Incompatible dimensions" },
    { ERR_MATRIXORREALEXPECTED, "Matrix or number expected" },
    { ERR_SQUAREMATRIXONLY, "Matrix must be square" },
    { ERR_VECTOREXPECTED, "Expected a vector" },
    { ERR_MISPLACEDBRACKETS, "Misplaced brackets" },

    // ADD MORE MESSAGES HERE...

    // END OF LIST MARKER
    { 0 , ""}

};


// THIS IS THE HELP TEXT FOR ALL COMMANDS PROVIDED IN ROM
// UP TO 3 LINES OF TEXT, LAYOUT IS HARDWARE DEPENDENT
// MAKE SURE IT LOOKS GOOD

const MSGLIST const all_cmdhelp[]={
    // BASIC UNKNOWN ERROR MESSAGE

    // ADD MORE MESSAGES HERE...

    // END OF LIST MARKER
    { 0 , ""}
};
#endif
