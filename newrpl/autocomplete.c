/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
#include <string.h>



WORD rplGetNextSuggestion(WORD suggestion,BYTEPTR *start,BYTEPTR *end)
{

    BINT libcnt;
    LIBHANDLER han;
    WORD saveop=CurOpcode;
    if(!suggestion) libcnt=rplGetNextLib(MAXLIBNUMBER+1);  // START FROM THE HIGHEST NUMBER
    else libcnt=LIBNUM(suggestion);

    TokenStart=(WORDPTR *)start;
    TokenLen=(BINT) utf8nlen(start,end);
    SuggestedObject=zero_bint;
    SuggestedOpcode=suggestion;

    if(!suggestion) suggestion=1;
    else suggestion=0;

    do {

    while(libcnt>=0) {
        RetNum=-1;
        CurOpcode=MKOPCODE(libcnt,OPCODE_AUTOCOMPNEXT);
        han=rplGetLibHandler(libnum);
        if(han) (*han)();

        if(RetNum==OK_CONTINUE) {
            CurOpcode=saveop; return SuggestedOpcode;
        }
        // NO MORE SUGGESTIONS FROM THIS LIBRARY
        libcnt=rplGetNextLib(libcnt);
    }

    libcnt=rplGetNextLib(MAXLIBNUMBER+1);

    // RESTART FROM THE TOP IF THERE WAS A PREVIOUS SUGGESTION
    // COULD RETURN THE SAME SUGGESTION IF THERE WAS ONLY ONE
    } while(suggestion--);

    return 0;
}

WORD rplGetPrevSuggestion(WORD suggestion,BYTEPTR *start,BYTEPTR *end)
{
return 0;
}


// UPDATE THE SUGGESTION
WORD rplUpdateSuggestion(WORD suggestion,BYTEPTR *start,BYTEPTR *end)
{
    return rplGetNextSuggestion(suggestion+1,start,end);
}
