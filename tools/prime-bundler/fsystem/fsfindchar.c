/*
* Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*/

#include "fsyspriv.h"

// FILENAME PARSING FUNCTIONS

char *fsfindchar(char *strstart, char *strend, char *chars)
{
    char a, *ptr;
    while(strstart != strend) {
        a = *strstart;
        if(!a)
            return NULL;
        ptr = (char *)chars;
        while(*ptr != 0) {
            if(a == *ptr)
                return strstart;
            ++ptr;
        }
        ++strstart;
    }
    return NULL;
}

char *fsfindcharrev(char *strstart, char *strend, char *chars)
{
    char a, *ptr;
    if(strend == NULL) {
        strend = strstart;
        while(*strend != 0)
            ++strend;
    }
    while(strstart != strend) {
        --strend;
        a = *strend;
        ptr = (char *)chars;
        while(*ptr != 0) {
            if(a == *ptr)
                return strend;
            ++ptr;
        }
    }
    return NULL;
}
