/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"



WORD rplGetNextSuggestion(WORD suggestion,WORDPTR suggobject,BYTEPTR start,BYTEPTR end)
{

    BINT libcnt;
    LIBHANDLER han;
    WORD saveop=CurOpcode;
    if(!suggestion && !suggobject) libcnt=rplGetNextLib(MAXLIBNUMBER+1);  // START FROM THE HIGHEST NUMBER
    else {
        if(suggestion) libcnt=LIBNUM(suggestion);
        else libcnt=LIBNUM(*suggobject);
    }

    TokenStart=(WORDPTR)start;
    BlankStart=(WORDPTR)end;
    TokenLen=(BINT) utf8nlen((char *)start,(char *)end);
    SuggestedObject=suggobject;
    SuggestedOpcode=suggestion;

    //if(!suggestion) suggestion=1;
    //else suggestion=0;

    suggestion=1;

    do {

    while(libcnt>=0) {
        RetNum=-1;
        CurOpcode=MKOPCODE(libcnt,OPCODE_AUTOCOMPNEXT);
        han=rplGetLibHandler(libcnt);
        if(han) (*han)();

        if(RetNum==OK_CONTINUE) {
            CurOpcode=saveop;
            // IF SuggestedOpcode IS ZERO, THEN THE POINTER IS RETURNED IN SuggestedObject
            return SuggestedOpcode;
        }
        // NO MORE SUGGESTIONS FROM THIS LIBRARY
        libcnt=rplGetNextLib(libcnt);
    }

    libcnt=rplGetNextLib(MAXLIBNUMBER+1);

    SuggestedOpcode=0xffffffff;

    // RESTART FROM THE TOP IF THERE WAS A PREVIOUS SUGGESTION
    // COULD RETURN THE SAME SUGGESTION IF THERE WAS ONLY ONE
    } while(suggestion--);

    SuggestedObject=0;
    return 0;
}

WORD rplGetPrevSuggestion(WORD suggestion,WORDPTR suggobject,BYTEPTR start,BYTEPTR end)
{

    BINT libcnt;
    LIBHANDLER han;
    WORD saveop=CurOpcode,prevsugg;
    WORDPTR prevsuggobj;

    if(!suggestion && !suggobject) return 0;

    libcnt=rplGetNextLib(MAXLIBNUMBER+1);  // START FROM THE HIGHEST NUMBER

    TokenStart=(WORDPTR)start;
    BlankStart=(WORDPTR)end;
    TokenLen=(BINT) utf8nlen((char *)start,(char *)end);
    SuggestedObject=(WORDPTR)zero_bint;
    SuggestedOpcode=-1;

    //if(!suggestion) suggestion=1;
    //else suggestion=0;
    prevsugg=0;
    prevsuggobj=0;

    while(libcnt>=0) {
        RetNum=-1;
        CurOpcode=MKOPCODE(libcnt,OPCODE_AUTOCOMPNEXT);
        han=rplGetLibHandler(libcnt);
        if(han) (*han)();

        if(RetNum==OK_CONTINUE) {
            if((SuggestedOpcode &&(SuggestedOpcode==suggestion))||(!SuggestedOpcode && (SuggestedObject==suggobject))) {
                if(prevsugg || prevsuggobj) {
                    CurOpcode=saveop;
                    SuggestedOpcode=prevsuggobj;
                    return prevsugg;
                }
                else {
                    // THIS THE FIRST SUGGESTION, SO THE RESULT SHOULD BE THE LAST
                    // ONE WE GET, KEEP WORKING ON IT
                    prevsugg=SuggestedOpcode;
                    prevsuggobj=SuggestedObject;
                }

            } else {
                prevsugg=SuggestedOpcode;
                prevsuggobj=SuggestedObject;
            }
        }
        // NO MORE SUGGESTIONS FROM THIS LIBRARY
        else libcnt=rplGetNextLib(libcnt);
    }

    SuggestedObject=prevsuggobj;
    return prevsugg;
}


// UPDATE THE SUGGESTION
WORD rplUpdateSuggestion(WORD suggestion,WORDPTR suggobject,BYTEPTR start,BYTEPTR end)
{
    return rplGetNextSuggestion(suggestion,suggobject,start,end);
}
