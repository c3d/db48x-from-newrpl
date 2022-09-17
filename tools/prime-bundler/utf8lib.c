/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "utf8lib.h"

// UNICODE DATA FOR NFC NORMALIZATION

extern const int used_CCData;
extern const unsigned int const packed_CCData[];

extern const int used_CCBytes;
extern const unsigned char const packed_CCBytes[];

extern const int used_singletonRanges;
extern const unsigned int const packed_singletonRanges[];

extern const int used_singletonData;
extern const unsigned int const packed_singletonData[];

extern const int used_doubleRanges;
extern const unsigned int const packed_doubleRanges[];

extern const int used_doubleData;
extern const unsigned int const packed_doubleData[];

extern const int used_combiners;
extern const unsigned int const packed_combiners[];

extern const int used_starteroff;
extern const unsigned int const packed_starters[];

extern const int used_starterdata;
extern const unsigned int const packed_starterData[];

unsigned int unicodeBuffer[MAX_UNICODE_CHARACTER_LEN];

// DECODE A UTF8 CODE POINT AND RETURN ITS VALUE
int utf82cp(cstring ptr, cstring end)
{
    if(*ptr & 0x80) {
        if((*ptr & 0xe0) == 0xc0) {
            if(end - ptr < 2)
                return -1;
            if((ptr[1] & 0xc0) != 0x80)
                return -1;
            return ((((unsigned int)ptr[0]) & 0x1f) << 6) | (((unsigned int)
                        ptr[1]) & 0x3f);
        }
        if((*ptr & 0xf0) == 0xe0) {
            if(end - ptr < 3)
                return -1;
            if((ptr[1] & 0xc0) != 0x80)
                return -1;
            if((ptr[2] & 0xc0) != 0x80)
                return -1;

            return ((((unsigned int)ptr[0]) & 0xf) << 12) | ((((unsigned int)
                            ptr[1]) & 0x3f) << 6) | (((unsigned int)ptr[2]) &
                    0x3f);
        }
        if((*ptr & 0xf8) == 0xf0) {
            if(end - ptr < 4)
                return -1;
            if((ptr[1] & 0xc0) != 0x80)
                return -1;
            if((ptr[2] & 0xc0) != 0x80)
                return -1;
            if((ptr[3] & 0xc0) != 0x80)
                return -1;
            return ((((unsigned int)ptr[0]) & 0x7) << 18) | ((((unsigned int)
                            ptr[1]) & 0x3f) << 12) | ((((unsigned int)ptr[2]) &
                        0x3f) << 6) | (((unsigned int)ptr[3]) & 0x3f);
        }
        // THIS IS AN INVALID SEQUENCE
        return -1;
    }

    return (int)*ptr;
}

// SKIP A CODE POINT
char *utf8skip(char *ptr, char *end)
{
    if(end <= ptr)
        return ptr;
    if(*ptr & 0x80) {
        ++ptr;
        while(((*ptr & 0xc0) == 0x80) && (ptr < end))
            ++ptr;
        return ptr;
    }
    return ++ptr;
}

// SKIP BYTES UNTIL A STARTER CODEPOINT IS FOUND
// USED TO ALIGN ARBITRARY POINTERS INTO THE UTF8 SEQUENCE
char *utf8findst(char *ptr, char *end)
{
    if(end <= ptr)
        return ptr;
    while(((*ptr & 0xc0) == 0x80) && (ptr < end))
        ++ptr;
    // GOT THE BEGINNING OF A UTF8 SEQUENCE
    if(end <= ptr)
        return ptr;

    int cp, cpinfo;

    do {

        cp = utf82cp(ptr, end);
        if(cp < 0)
            return end; // INVALID UTF8 SEQUENCE!
        cpinfo = getCPInfo(cp);
        if(CCLASS(cpinfo) == 0)
            return ptr; // FOUND A STARTER
        // NOT A STARTER, SKIP IT
        if(*ptr & 0x80) {
            ++ptr;
            while(((*ptr & 0xc0) == 0x80) && (ptr < end))
                ++ptr;
        }
        else
            ++ptr;
    }
    while(ptr < end);

    return end;

}

// SAME AS SKIP, BUT ALSO SKIP OVER COMBINING MARKS
// SKIP CODEPOINTS UNTIL A STARTER CODEPOINT IS FOUND
char *utf8skipst(char *ptr, char *end)
{
    if(end <= ptr)
        return ptr;
    int cp, cpinfo;

    do {
        // SKIP CODEPOINT
        if(*ptr & 0x80) {
            ++ptr;
            while(((*ptr & 0xc0) == 0x80) && (ptr < end))
                ++ptr;
        }
        else
            ++ptr;

        if(end <= ptr)
            return ptr;

        cp = utf82cp(ptr, end);
        if(cp < 0)
            return end; // INVALID UTF8 SEQUENCE!
        cpinfo = getCPInfo(cp);
        if(CCLASS(cpinfo) == 0)
            return ptr; // FOUND A STARTER
        // NOT A STARTER, SKIP IT
    }
    while(ptr < end);

    return ptr;

}

// SKIP n CODE POINTS
char *utf8nskip(char *ptr, char *end, int n)
{
    while(n > 0) {
        if(ptr >= end)
            break;
        if(*ptr & 0x80) {
            ++ptr;
            while(((*ptr & 0xc0) == 0x80) && (ptr < end))
                ++ptr;

        }
        else {
            ++ptr;
        }
        --n;
    }
    return ptr;
}

// SKIP n STARTERS AND THEIR COMBINING MARKS
char *utf8nskipst(char *ptr, char *end, int n)
{
    if(end <= ptr)
        return ptr;
    int cp, cpinfo;

    while(n > 0) {
        do {
            // SKIP CODEPOINT
            if(*ptr & 0x80) {
                ++ptr;
                while(((*ptr & 0xc0) == 0x80) && (ptr < end))
                    ++ptr;
            }
            else
                ++ptr;

            if(end <= ptr)
                return ptr;

            cp = utf82cp(ptr, end);
            if(cp < 0)
                return end;     // INVALID UTF8 SEQUENCE!
            cpinfo = getCPInfo(cp);
            if(CCLASS(cpinfo) == 0)
                break;  // FOUND A STARTER
            // NOT A STARTER, SKIP IT
        }
        while(ptr < end);
        --n;
    }

    return ptr;

}

// SKIP A CODE POINT IN REVERSE
char *utf8rskip(char *ptr, char *start)
{
    if(start >= ptr)
        return ptr;
    --ptr;
    while(((*ptr & 0xc0) == 0x80) && (ptr > start))
        --ptr;
    return ptr;
}

// REVERSE SKIP CODEPOINTS UNTIL NEXT STARTER
char *utf8rskipst(char *ptr, char *start)
{

    if(start >= ptr)
        return ptr;

    char *prevptr;
    int cp, cpinfo;

    do {
        // SKIP THE CODEPOINT
        prevptr = ptr;
        --ptr;
        while(((*ptr & 0xc0) == 0x80) && (ptr > start))
            --ptr;

        cp = utf82cp(ptr, prevptr);
        if(cp < 0)
            return start;       // INVALID UTF8 SEQUENCE!
        cpinfo = getCPInfo(cp);
        if(CCLASS(cpinfo) == 0)
            return ptr; // FOUND A STARTER
        // NOT A STARTER, SKIP IT
    }
    while(ptr > start);
    // REACHED THE START OF STRING WITHOUT FINDING A STARTER!
    return ptr;

}

// ENCODE A CHARACTER AND RETURN A NULL TERMINATED STRING,
// OR A NON-TERMINATED 4-BYTE STRING
unsigned int cp2utf8(int codepoint)
{
    if(codepoint <= 0x7f)
        return codepoint;
    if(codepoint <= 0x7ff)
        return (((codepoint & 0x3f) | 0x80) << 8) | ((codepoint >> 6) & 0x1f) |
                0xc0;
    if(codepoint <= 0xffff)
        return (((codepoint & 0x3f) | 0x80) << 16) | ((((codepoint >> 6) & 0x3f)
                    | 0x80) << 8) | ((codepoint >> 12) & 0xf) | 0xe0;
    if(codepoint <= 0x10ffff)
        return (((codepoint & 0x3f) | 0x80) << 24) | ((((codepoint >> 6) & 0x3f)
                    | 0x80) << 16) | ((((codepoint >> 12) & 0x3f) | 0x80) << 8)
                | ((codepoint >> 18) & 0x7) | 0xf0;
    // INVALID CHARACTER
    return -1;
}

// OPTIMIZED VERSION RETURNS NFC QUICK CHECK, CC AND COMPOSITION EXCLUSION PROPERTIES
// GIVEN A SINGLE CODE POINT
unsigned char getCPInfo(unsigned int cp)
{
    int k;
    unsigned int codept = 0;
    int nchars;
    for(k = 0; k < used_CCData; ++k) {
        if(CCBYTE(packed_CCData[k]) != 0xff) {
            nchars = LONG_NCHARS(packed_CCData[k]);
            if(cp < codept + nchars)
                return CCBYTE(packed_CCData[k]);
        }
        else {
            nchars = NCHARS(packed_CCData[k]);
            if(cp < codept + nchars)
                return packed_CCBytes[TOFFSET(packed_CCData[k]) + cp - codept];
        }
        codept += nchars;
    }

    return 0;

}

// *********************************************************************************
// HANGUL DECOMPOSITION - PORTED FROM UNICODE STANDARD EXAMPLE CHAPTER 3.12
// *********************************************************************************

#define SBase 0xAC00
#define LBase 0x1100
#define VBase 0x1161
#define TBase 0x11A7
#define LCount 19
#define VCount 21
#define TCount 28
#define NCount (VCount * TCount)
#define SCount (LCount * NCount)

void quickDecomp(unsigned int cp, unsigned int *dec1, unsigned int *dec2,
        unsigned int *dec3)
{
    // SLOW VERSION - REPLACE WITH OPTIMIZED VERSION LATER

    //  ALGORITHMIC HANGUL DECOMPOSITIONS
    int SIndex = cp - SBase;
    if(SIndex >= 0 && SIndex < SCount) {
        *dec1 = LBase + SIndex / NCount;
        *dec2 = VBase + (SIndex % NCount) / TCount;
        int T = TBase + SIndex % TCount;
        if(T != TBase)
            *dec3 = T;
        else
            *dec3 = -1;
        return;
    }

    *dec3 = -1;

    // TRY SINGLETONS FIRST
    int k;
    unsigned int codept = 0;
    int nchars, off;
    for(k = 0; k < used_singletonRanges; ++k) {
        nchars = SING_LEN(packed_singletonRanges[k]);
        off = SING_OFFSET(packed_singletonRanges[k]);
        if(cp < codept + nchars) {
            if(off == 0xfff)
                break;  // NOT A SINGLETON
            if(packed_singletonData[off + cp - codept] != 0xffffffff) {
                *dec1 = packed_singletonData[off + cp - codept];
                *dec2 = -1;
                return;
            }
            else
                break;
        }
        codept += nchars;
    }

    // TRY DOUBLES
    codept = 0;
    for(k = 0; k < used_doubleRanges; ++k) {
        nchars = SING_LEN(packed_doubleRanges[k]);
        off = SING_OFFSET(packed_doubleRanges[k]);
        if(cp < codept + nchars) {
            if(off == 0xfff)
                break;  // NOT A DOUBLE
            *dec1 = packed_doubleData[(off + cp - codept) << 1];
            *dec2 = packed_doubleData[((off + cp - codept) << 1) + 1];
            return;
        }
        codept += nchars;
    }

    *dec1 = -1;
    *dec2 = -1;

}

// READ ONE CHARACTER OF A UTF8 STRING

int appendDecomp(unsigned int cp, int lastchar)
{
    // DECOMPOSE IN THE BUFFER
    unsigned int dec1, dec2, dec3;
    //dec1=dec2=-1;
    quickDecomp(cp, &dec1, &dec2, &dec3);

    if(dec1 != 0xffffffff) {
        lastchar = appendDecomp(dec1, lastchar);
        if(dec2 != 0xffffffff) {
            lastchar = appendDecomp(dec2, lastchar);
            if(dec3 != 0xffffffff) {
                lastchar = appendDecomp(dec3, lastchar);
            }
        }
    }
    else {
        unicodeBuffer[lastchar++] = cp;
        bubbleSort(lastchar - 1);
    }

    return lastchar;
}

void bubbleSort(int lastch)
{
    int cc = CCLASS(getCPInfo(unicodeBuffer[lastch]));
    int cc2;
    if(!cc)
        return;
    while(lastch > 0) {
        --lastch;
        cc2 = CCLASS(getCPInfo(unicodeBuffer[lastch]));
        if(cc2 > cc) {
            // SWAP POSITIONS
            unsigned int tmp = unicodeBuffer[lastch];
            unicodeBuffer[lastch] = unicodeBuffer[lastch + 1];
            unicodeBuffer[lastch + 1] = tmp;
        }
        else
            return;
    }
}

int getComposition(unsigned int char1, unsigned int char2)
{
    // TRY DOUBLES
    unsigned int codept = 0;
    int k, nchars, off, j;
    for(k = 0; k < used_combiners; ++k) {
        nchars = SING_LEN(packed_combiners[k]);
        off = SING_OFFSET(packed_combiners[k]);
        if(char2 < codept + nchars) {
            if(off == 0xfff)
                return -1;      // NOT A COMBINER
            int tableoff = packed_starters[(off + char2 - codept)];
            if(tableoff < 0)
                return -1;      // NOT A COMBINER
            for(j = 0; j < (int)packed_starterData[tableoff]; ++j) {
                if(char1 == packed_starterData[tableoff + 1 + (j << 1)])
                    return packed_starterData[tableoff + 2 + (j << 1)];
                if(char1 < packed_starterData[tableoff + 1 + (j << 1)])
                    return -1;
            }
            return -1;
        }
        codept += nchars;
    }
    return -1;

}

int quickCompose(int lastch)
{
    // SLOW VERSION - REPLACE WITH OPTIMIZED VERSION LATER
    int hanguldone = 0;
    // DO HANGUL COMPOSITION FIRST
    int LIndex = unicodeBuffer[0] - LBase;
    if(0 <= LIndex && LIndex < LCount) {
        int VIndex = unicodeBuffer[1] - VBase;
        if(0 <= VIndex && VIndex < VCount) {
            // make syllable of form LV
            unicodeBuffer[0] = (SBase + (LIndex * VCount + VIndex) * TCount);
            int idx = 1;
            --lastch;
            while(idx <= lastch) {
                unicodeBuffer[idx] = unicodeBuffer[idx + 1];
                ++idx;
            }
            hanguldone = 1;
        }
    }

    // HANGUL CASE 2
    int SIndex = unicodeBuffer[0] - SBase;
    if(0 <= SIndex && SIndex < SCount && (SIndex % TCount) == 0) {
        int TIndex = unicodeBuffer[1] - TBase;
        if(0 < TIndex && TIndex < TCount) {
            // make syllable of form LVT
            unicodeBuffer[0] += TIndex;
            int idx = 1;
            --lastch;
            while(idx <= lastch) {
                unicodeBuffer[idx] = unicodeBuffer[idx + 1];
                ++idx;
            }
            hanguldone = 1;
        }
    }

    if(hanguldone)
        return lastch;

    // NORMAL CHARACTER COMPOSITION

    unsigned int starter = unicodeBuffer[0];

    int idx = 1;
    while(idx < lastch) {
        if(idx > 1) {
            // CHECK IF THE CHARACTER IS BLOCKED
            int ccidx = CCLASS(getCPInfo(unicodeBuffer[idx]));
            if(ccidx == 0) {
                // FOUND NEXT STARTER, CANNOT COMBINE
                return lastch;

            }
            if(ccidx == CCLASS(getCPInfo(unicodeBuffer[idx - 1]))) {
                // BLOCKED CHARACTER CANNOT COMBINE, SKIP
                ++idx;
                continue;
            }
        }

        // DO THE ACTUAL COMPOSITION

        int combined = getComposition(starter, unicodeBuffer[idx]);

        if(combined != -1) {
            // COMPOSE THE CHARACTER
            starter = unicodeBuffer[0] = combined;
            --lastch;
            while(idx <= lastch) {
                unicodeBuffer[idx] = unicodeBuffer[idx + 1];
                ++idx;
            }

            idx = 0;
        }

        // NO MATCH FOUND FOR COMPOSITION, CONTINUE TO NEXT CHARACTER
        ++idx;
    }
    return lastch;

}

enum
{
    NEED_DECOMP = 1,
    NEED_COMP = 2
};
// READ A UTF8 CHARACTER, CONVERT TO NFC AND LEAVE AT THE BUFFER.
// RETURNS THE NUMBER OF BYTES USED FROM STRING

// NFC NORMALIZATION IS:
// NFC_QC=0 -> CC=0 --> NO ACTION NEEDED
// NFC_QC=0 -> CC!=0 --> STORE BUT MIGHT NEED BUBBLE SORT
// NFC_QC!=0 -> CC=0 --> FULL DECOMPOSITION/COMPOSITION, NO BUBBLE SORT
// NFC_QC!=0 -> CC!=0 --> FULL DEC+BUBBLE SORT+COMPOSITION

int utf82NFC(char *string, char *end)
{
    unsigned int cp, cc, qc, cpinfo;
    int lastchar = 0, flags;
    int len = end - string;
    flags = 0;
    while(string < end) {
        cp = utf82cp(string, end);
        if(cp == 0xffffffff) {
            unicodeBuffer[0] = 0;
            return len;
        }
        cpinfo = getCPInfo(cp);
        qc = NFC_QC(cpinfo);
        cc = CCLASS(cpinfo);

        if((cc == 0) && (qc == 0)) {
            if(lastchar != 0) {

                if(flags & NEED_DECOMP) {
                    lastchar =
                            appendDecomp(unicodeBuffer[lastchar - 1],
                            lastchar - 1);
                }

                lastchar = quickCompose(lastchar);

                unicodeBuffer[lastchar] = 0;
                return len - (end - string);
            }
        }
        else {
            if(lastchar == 1) {
                // THE FIRST CHARACTER NEEDS TO BE DECOMPOSED
                lastchar =
                        appendDecomp(unicodeBuffer[lastchar - 1], lastchar - 1);
                flags |= NEED_COMP;

            }

        }

        string = utf8skip(string, end);

        if(qc) {
            // FAILED QUICK CHECK TEST
            // DECOMPOSE IN THE BUFFER
            lastchar = appendDecomp(cp, lastchar);
            flags |= NEED_COMP;

        }
        else {
            // QUICK CHECK PASSED
            unicodeBuffer[lastchar++] = cp;
            if(lastchar > 1)
                bubbleSort(lastchar - 1);
        }
    }

    if(flags & NEED_COMP)
        lastchar = quickCompose(lastchar);

    unicodeBuffer[lastchar] = 0;
    return len - (end - string);

}

// COMPARE len UNICODE CODE POINTS IN UTF8 ENCODED STRINGS
// SIMILAR TO strncmp BUT WITH UTF8 SUPPORT

int utf8ncmp(const char *s1, const char *s1end, const char *s2,
        const char *s2end, int len)
{
    if(len > 0) {
        while((len > 0) && (s1 < s1end) && (s2 < s2end)) {
            if(utf82cp((char *)s1, (char *)s1end) != utf82cp((char *)s2,
                        (char *)s2end))
                break;

            s1 = utf8skip((char *)s1, (char *)s1end);
            s2 = utf8skip((char *)s2, (char *)s2end);
            --len;
        }

        if(len > 0) {
            if((s1 < s1end) && (s2 < s2end))
                return (utf82cp((char *)s1, (char *)s1end) - utf82cp((char *)s2,
                            (char *)s2end));
            else {
                if(s1 == s1end)
                    return -1;
                return 1;
            }
        }

        // THE CHARACTERS OF THE SHORTEST STRING WERE ALL IDENTICAL

        if(s1 < s1end) {
            // CHECK IF LEFTOVER IS A NON-STARTER
            if(CCLASS(getCPInfo(utf82cp((char *)s1, (char *)s1end))) != 0) {
                // NOT A STARTER, STRINGS ARE NOT EQUAL
                return 1;
            }
        }
        if(s2 < s2end) {
            if(CCLASS(getCPInfo(utf82cp((char *)s2, (char *)s2 + 4))) != 0) {
                // NOT A STARTER, STRINGS ARE NOT EQUAL
                return -1;
            }
        }

    }

    return 0;
}

// SIMILAR TO utf8ncmp2 BUT SECOND STRING IS EXACTLY len CODE POINTS
// (TO AVOID COMPUTING THE END OF A NULL TERMINATED STRING)
int utf8ncmp2(const char *s1, const char *s1end, const char *s2, int len)
{
    if(len > 0) {
        while((len > 0) && (s1 < s1end)) {
            if(utf82cp((char *)s1, (char *)s1end) != utf82cp((char *)s2,
                        (char *)s2 + 4))
                break;

            s1 = utf8skip((char *)s1, (char *)s1end);
            s2 = utf8skip((char *)s2, (char *)s2 + 4);
            --len;
        }

        if(len > 0) {
            if(s1 < s1end)
                return (utf82cp((char *)s1, (char *)s1end) - utf82cp((char *)s2,
                            (char *)s2 + 4));
            else
                return -1;
        }

        // THE N CHARACTERS WERE ALL IDENTICAL

        if(s1 < s1end) {
            // CHECK IF LEFTOVER IS A NON-STARTER
            if(CCLASS(getCPInfo(utf82cp((char *)s1, (char *)s1end))) != 0) {
                // NOT A STARTER, STRINGS ARE NOT EQUAL
                return 1;
            }
        }

    }

    return 0;
}

// SIMILAR TO strcmp BUT WITH UTF8 SUPPORT
int utf8cmp(const char *s1, const char *s2)
{
    while(*s1 && *s2) {
        if(utf82cp((char *)s1, (char *)s1 + 4) != utf82cp((char *)s2,
                    (char *)s2 + 4))
            break;
        s1 = utf8skip((char *)s1, (char *)s1 + 4);
        s2 = utf8skip((char *)s2, (char *)s2 + 4);
    }

    if((*s1 == 0) && (*s2 == 0))
        return 0;
    if(*s1 == '\0')
        return -1;
    if(*s2 == '\0')
        return 1;
    return (utf82cp((char *)s1, (char *)s1 + 4) - utf82cp((char *)s2,
                (char *)s2 + 4));
}

// SAME AS STRLEN BUT RETURNS THE LENGTH IN UNICODE CODEPOINTS OF
// A NULL-TERMINATED STRING

int utf8len(char *string)
{
    int count = 0;
    while(*string) {
        ++count;
        if(*string & 0x80) {
            ++string;
            while(((*string & 0xc0) == 0x80))
                ++string;
        }
        else
            ++string;
    }
    return count;
}

// RETURNS THE LENGTH IN UNICODE CODEPOINTS, SKIPPING NON-STARTER CHARACTERS

int utf8nlenst(char *string, char *end)
{
    int count = 0;
    while(string < end) {
        ++count;
        string = utf8skipst(string, end);
    }
    return count;
}

int utf8nlen(char *string, char *end)
{
    int count = 0;
    while(string < end) {
        ++count;
        string = utf8skip(string, end);
    }
    return count;
}
