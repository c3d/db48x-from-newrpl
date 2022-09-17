/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "libraries.h"

#ifndef COMMANDS_ONLY_PASS
#include "cmdcodes.h"
#include "hal_api.h"
#include "newrpl.h"
#include "sysvars.h"
#endif

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************

// REPLACE THE NUMBER
#define LIBRARY_NUMBER  68

//@TITLE=Menus, Flags and System Settings

#define ERROR_LIST \
    ERR(SYSTEMFLAGSINVALID,0), \
    ERR(INVALIDFLAGNUMBER,1), \
    ERR(INVALIDFLAGNAME,2), \
    ERR(IDENTORINTEGEREXPECTED,3), \
    ERR(INVALIDLOCALESTRING,4), \
    ERR(INVALIDMENUDEFINITION,5), \
    ERR(INVALIDKEYNAME,6), \
    ERR(INVALIDKEYDEFINITION,7), \
    ERR(INVALIDNUMFORMAT,8)

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(SETLOCALE,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SETNFMT,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(SF,MKTOKENINFO(2,TITYPE_NOTALLOWED,1,2)), \
    CMD(CF,MKTOKENINFO(2,TITYPE_NOTALLOWED,1,2)), \
    ECMD(FCTEST,"FC?",MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    ECMD(FSTEST,"FS?",MKTOKENINFO(3,TITYPE_FUNCTION,1,2)), \
    ECMD(FCTESTCLEAR,"FC?C",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD(FSTESTCLEAR,"FS?C",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(TMENU,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)),\
    CMD(TMENULST,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)),\
    CMD(TMENUOTHR,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)),\
    CMD(MENUSWAP,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(MENUBK,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(MENUBKLST,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(MENUBKOTHR,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(RCLMENU,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)),\
    CMD(RCLMENULST,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)),\
    CMD(RCLMENUOTHR,MKTOKENINFO(11,TITYPE_NOTALLOWED,1,2)),\
    CMD(DEG,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(GRAD,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(RAD,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(DMS,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(ASNKEY,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(DELKEY,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(STOKEYS,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(RCLKEYS,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(TYPE,MKTOKENINFO(4,TITYPE_FUNCTION,1,2)), \
    CMD(TYPEE,MKTOKENINFO(5,TITYPE_FUNCTION,1,2)), \
    CMD(GETLOCALE,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(GETNFMT,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(RCLF,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(STOF,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(VTYPE,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(VTYPEE,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    ECMD(FMTSTR,"FMT→STR",MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2))


// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"

#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);

INCLUDE_ROMOBJECT(sysmenu_2_main);
INCLUDE_ROMOBJECT(sysmenu_3_prog);
INCLUDE_ROMOBJECT(sysmenu_4_math);
INCLUDE_ROMOBJECT(sysmenu_5_symb);
INCLUDE_ROMOBJECT(sysmenu_6_system);
INCLUDE_ROMOBJECT(sysmenu_7_flags);
INCLUDE_ROMOBJECT(sysmenu_8_menu);
INCLUDE_ROMOBJECT(sysmenu_9_clipboard);
INCLUDE_ROMOBJECT(sysmenu_10_settings);
INCLUDE_ROMOBJECT(sysmenu_11_namedflags);
INCLUDE_ROMOBJECT(sysmenu_12_keys);

ROMOBJECT dotsettings_ident[] = {
    MKPROLOG(DOIDENTHIDDEN, 3),
    TEXT2WORD('.', 'S', 'e', 't'),
    TEXT2WORD('t', 'i', 'n', 'g'),
    TEXT2WORD('s', 0, 0, 0)
};

ROMOBJECT flags_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'l', 'a', 'g'),
    TEXT2WORD('s', 0, 0, 0)
};

ROMOBJECT userflags_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('U', 'F', 'l', 'a'),
    TEXT2WORD('g', 's', 0, 0)
};

ROMOBJECT locale_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('L', 'o', 'c', 'a'),
    TEXT2WORD('l', 'e', 0, 0)
};

ROMOBJECT numfmt_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('N', 'u', 'm', 'F'),
    TEXT2WORD('m', 't', 0, 0)
};

ROMOBJECT menu1_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('M', 'e', 'n', 'u'),
    TEXT2WORD('1', 0, 0, 0)
};

ROMOBJECT menu1hist_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('M', 'e', 'n', 'u'),
    TEXT2WORD('1', 'H', 's', 't')
};

ROMOBJECT menu2_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('M', 'e', 'n', 'u'),
    TEXT2WORD('2', 0, 0, 0)
};

ROMOBJECT menu2hist_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('M', 'e', 'n', 'u'),
    TEXT2WORD('2', 'H', 's', 't')
};

ROMOBJECT menuhistory_ident[] = {
    MKPROLOG(DOIDENT, 3),
    TEXT2WORD('M', 'e', 'n', 'u'),
    TEXT2WORD('H', 'L', 'e', 'v'),
    TEXT2WORD('e', 'l', 's', 0),

};

ROMOBJECT savedcmdline_ident[] = {
    MKPROLOG(DOIDENT, 3),
    TEXT2WORD('S', 'a', 'v', 'e'),
    TEXT2WORD('d', 'C', 'm', 'd'),
    TEXT2WORD('L', 'i', 'n', 'e'),

};

ROMOBJECT customkey_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('.', 'C', 's', 't'),
    TEXT2WORD('K', 'e', 'y', 's')
};

ROMOBJECT stksave_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('.', 'S', 't', 'k'),
    TEXT2WORD('S', 'a', 'v', 'e')
};

ROMOBJECT savedflags_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('.', 'H', 'W', 'F'),
    TEXT2WORD('l', 'a', 'g', 's')
};

ROMOBJECT editwidth_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('E', 'd', 'W', 'i'),
    TEXT2WORD('d', 't', 'h', 0)
};

ROMOBJECT currentform_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('.', 'C', 'u', 'r'),
    TEXT2WORD('F', 'o', 'r', 'm')
};

ROMOBJECT screenconfig_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('S', 'c', 'r', 'C'),
    TEXT2WORD('o', 'n', 'f', 0)
};

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const word_p const ROMPTR_TABLE[] = {
    (word_p) LIB_MSGTABLE,
    (word_p) LIB_HELPTABLE,

    (word_p) sysmenu_2_main,
    (word_p) sysmenu_3_prog,
    (word_p) sysmenu_4_math,
    (word_p) sysmenu_5_symb,
    (word_p) sysmenu_6_system,
    (word_p) sysmenu_7_flags,
    (word_p) sysmenu_8_menu,
    (word_p) sysmenu_9_clipboard,
    (word_p) sysmenu_10_settings,
    (word_p) sysmenu_11_namedflags,
    (word_p) sysmenu_12_keys,

    (word_p) dotsettings_ident,
    (word_p) flags_ident,
    (word_p) locale_ident,
    (word_p) numfmt_ident,
    (word_p) menu1_ident,
    (word_p) menu2_ident,
    (word_p) menu1hist_ident,
    (word_p) menu2hist_ident,
    (word_p) menuhistory_ident,
    (word_p) savedcmdline_ident,
    (word_p) customkey_ident,
    (word_p) savedflags_ident,
    (word_p) userflags_ident,
    (word_p) stksave_ident,
    (word_p) editwidth_ident,
    (word_p) currentform_ident,
    (word_p) screenconfig_ident,
    0
};

typedef struct
{
    const char *flagname;
    unsigned char flags[8];
} systemflag;

const systemflag const flags_names[] = {
    // EACH OF THE 8 CHARS CONTAINS: BITS 0-6:= FLAG NUMBER (1-127),
    // BIT 7= VALUE (ON/OFF) TO USE WITH SET FLAG, ASSUMED ALWAYS 0 FOR CLEAR FLAG.

    {"DEG", {(-FL_ANGLEMODE1), (-FL_ANGLEMODE2), 0, 0, 0, 0, 0, 0}},
    {"RAD", {0x80 | (-FL_ANGLEMODE1), (-FL_ANGLEMODE2), 0, 0, 0, 0, 0, 0}},
    {"GRAD", {(-FL_ANGLEMODE1), 0X80 | (-FL_ANGLEMODE2), 0, 0, 0, 0, 0, 0}},
    {"DMS", {0x80 | (-FL_ANGLEMODE1), 0x80 | (-FL_ANGLEMODE2), 0, 0, 0, 0, 0,
                        0}},
    {"COMMENTS", {0x80 | (-FL_STRIPCOMMENTS), 0, 0, 0, 0, 0, 0, 0}},
    {"ACTIVEMENU1", {(-FL_ACTIVEMENU), 0, 0, 0, 0, 0, 0, 0}},
    {"ACTIVEMENU2", {0X80 | (-FL_ACTIVEMENU), 0, 0, 0, 0, 0, 0, 0}},
    {"DATEDMY", {0X80 | (-FL_DATEFORMAT), 0, 0, 0, 0, 0, 0, 0}},
    {"DATEMDY", {(-FL_DATEFORMAT), 0, 0, 0, 0, 0, 0, 0}},
    {"TIME12", {(-FL_TIMEFORMAT), 0, 0, 0, 0, 0, 0, 0}},
    {"TIME24", {0x80 | (-FL_TIMEFORMAT), 0, 0, 0, 0, 0, 0, 0}},
    {"BEEPON", {(-FL_ERRORBEEP), 0, 0, 0, 0, 0, 0, 0}},
    {"BEEPOFF", {0x80 | (-FL_ERRORBEEP), 0, 0, 0, 0, 0, 0, 0}},
    {"ALMBEEPON", {(-FL_ALARMBEEP), 0, 0, 0, 0, 0, 0, 0}},
    {"ALMBEEPOFF", {0x80 | (-FL_ALARMBEEP), 0, 0, 0, 0, 0, 0, 0}},
    {"SAVEALM", {0x80 | (-FL_SAVACKALRM), 0, 0, 0, 0, 0, 0, 0}},
    {"RESCALM", {(-FL_RESRPTALRM), 0, 0, 0, 0, 0, 0, 0}},
    {"AUTOINDENT", {(-FL_AUTOINDENT), 0, 0, 0, 0, 0, 0, 0}},
    {"NOINDENT", {0x80 | (-FL_AUTOINDENT), 0, 0, 0, 0, 0, 0, 0}},
    {"DECOMPDISP", {(-FL_DECOMPEDIT), 0, 0, 0, 0, 0, 0, 0}},
    {"DECOMPEDIT", {0x80 | (-FL_DECOMPEDIT), 0, 0, 0, 0, 0, 0, 0}},
    {"CPLX", {0x80 | (-FL_COMPLEXMODE), 0, 0, 0, 0, 0, 0, 0}},
    {"REAL", {(-FL_COMPLEXMODE), 0, 0, 0, 0, 0, 0, 0}},
    {"APPROX.", {(-FL_APPROXSIGN) }},
    {"APPROX~", {0x80 | (-FL_APPROXSIGN) }},

// TODO: ADD MORE FLAG NAMES HERE
    {NULL, {0, 0, 0, 0, 0, 0, 0, 0}}
};

int32_t rplSetUserFlag(int32_t flag)
{
    if(flag < 1 || flag > 128)
        return -1;

    word_p UserFlags = rplGetSettings((word_p) userflags_ident);
    uint64_t low64, hi64;

    if(!UserFlags || (!ISBINDATA(*UserFlags)))
        low64 = hi64 = 0;
    else {
        low64 = *((uint64_t *) (UserFlags + 1));
        hi64 = *((uint64_t *) (UserFlags + 3));
    }

    if(flag < 65)
        low64 |= (1ULL << (flag - 1));
    else
        hi64 |= (1ULL << (flag - 65));

    // UNLIKE SYSTEM FLAGS, THESE ARE NOT SELF-MODIFYING OBJECTS

    UserFlags = rplAllocTempOb(4);
    if(!UserFlags)
        return -2;
    UserFlags[0] = MKPROLOG(DOBINDATA, 4);
    UserFlags[1] = (WORD) low64;
    UserFlags[2] = (WORD) (low64 >> 32);
    UserFlags[3] = (WORD) hi64;
    UserFlags[4] = (WORD) (hi64 >> 32);

    rplStoreSettings((word_p) userflags_ident, UserFlags);

    return 0;
}

int32_t rplClrUserFlag(int32_t flag)
{
    if(flag < 1 || flag > 128)
        return -1;

    word_p UserFlags = rplGetSettings((word_p) userflags_ident);
    uint64_t low64, hi64;

    if(!UserFlags || (!ISBINDATA(*UserFlags)))
        low64 = hi64 = 0;
    else {
        low64 = *((uint64_t *) (UserFlags + 1));
        hi64 = *((uint64_t *) (UserFlags + 3));
    }

    if(flag < 65)
        low64 &= ~(1ULL << (flag - 1));
    else
        hi64 &= ~(1ULL << (flag - 65));

    // UNLIKE SYSTEM FLAGS, THESE ARE NOT SELF-MODIFYING OBJECTS

    UserFlags = rplAllocTempOb(4);
    if(!UserFlags)
        return -2;
    UserFlags[0] = MKPROLOG(DOBINDATA, 4);
    UserFlags[1] = (WORD) low64;
    UserFlags[2] = (WORD) (low64 >> 32);
    UserFlags[3] = (WORD) hi64;
    UserFlags[4] = (WORD) (hi64 >> 32);

    rplStoreSettings((word_p) userflags_ident, UserFlags);
    return 0;
}

// RETURNS 1 IF FLAG IS SET, 0 OTHERWISE
// RETURN -1 IF THE NUMBER IS NOT VALID
// RETURN  0 IF SYSTEM FLAGS ARE CORRUPTED, INVALID OR NONEXISTENT

int32_t rplTestUserFlag(int32_t flag)
{
    if(flag < 1 || flag > 128)
        return -1;

    word_p UserFlags = rplGetSettings((word_p) userflags_ident);
    uint64_t low64, hi64;

    if(!UserFlags || (!ISBINDATA(*UserFlags)))
        low64 = hi64 = 0;
    else {
        low64 = *((uint64_t *) (UserFlags + 1));
        hi64 = *((uint64_t *) (UserFlags + 3));
    }

    if(flag < 65)
        return (low64 & (1ULL << (flag - 1))) ? 1 : 0;
    else
        return (hi64 & (1ULL << (flag - 65))) ? 1 : 0;
}

uint64_t *rplGetUserFlagsLow()
{
    word_p UserFlags = rplGetSettings((word_p) userflags_ident);
    if(!UserFlags)
        return NULL;
    if(!ISBINDATA(*UserFlags))
        return NULL;
    return (uint64_t *) (UserFlags + 1);
}

int32_t rplSetSystemFlag(int32_t flag)
{
    if(flag > -1 || flag < -128)
        return -1;
    if(!ISBINDATA(*SystemFlags))
        return -2;
    word_p low64 = SystemFlags + 1;
    flag = -flag - 1;
    low64[flag >> 5] |= 1 << (flag & 31);


    return 0;
}

int32_t rplClrSystemFlag(int32_t flag)
{
    if(flag > -1 || flag < -128)
        return -1;
    if(!ISBINDATA(*SystemFlags))
        return -2;

    word_p low64 = SystemFlags + 1;

    flag = -flag - 1;
    low64[flag >> 5] &= ~(1 << (flag & 31));

    return 0;
}

int32_t rplSetSystemFlagByName(byte_p name, byte_p nameend)
{
    if(!ISBINDATA(*SystemFlags))
        return -2;

    int32_t idx = 0;
    int32_t len = utf8nlen((char *)name, (char *)nameend);
    int32_t flaglen;

    while(flags_names[idx].flagname) {
        flaglen = utf8len((char *)flags_names[idx].flagname);
        if((flaglen == len)
                && !utf8ncmp2((char *)name, (char *)nameend,
                    flags_names[idx].flagname, len)) {
            int32_t count;
            for(count = 0; count < 8; ++count) {
                if(flags_names[idx].flags[count]) {
                    int32_t flag = flags_names[idx].flags[count] & 0x7f;
                    int32_t value = flags_names[idx].flags[count] >> 7;
                    //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
                    word_p low64 = SystemFlags + 1;
                    flag = flag - 1;
                    if(value) {
                        low64[flag >> 5] |= 1 << (flag & 31);
                    }
                    else {
                        low64[flag >> 5] &= ~(1 << (flag & 31));
                    }

                }
            }

            return 0;

        }
        ++idx;
    }
    return -1;
}

int32_t rplClrSystemFlagByName(byte_p name, byte_p nameend)
{
    if(!ISBINDATA(*SystemFlags))
        return -2;

    int32_t idx = 0;
    int32_t len = utf8nlen((char *)name, (char *)nameend);
    int32_t flaglen;

    while(flags_names[idx].flagname) {
        flaglen = utf8len((char *)flags_names[idx].flagname);
        if((flaglen == len)
                && !utf8ncmp2((char *)name, (char *)nameend,
                    flags_names[idx].flagname, len)) {
            int32_t count;
            for(count = 0; count < 8; ++count) {
                if(flags_names[idx].flags[count]) {
                    int32_t flag = flags_names[idx].flags[count] & 0x7f;
                    //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
                    word_p low64 = SystemFlags + 1;
                    flag = flag - 1;
                    low64[flag >> 5] &= ~(1 << (flag & 31));

                }
            }

            return 0;

        }
        ++idx;
    }

    return -1;

}

// RETURNS 1 IF FLAG IS SET, 0 OTHERWISE
// RETURN -1 IF THE NUMBER IS NOT VALID
// RETURN -2 IF SYSTEM FLAGS ARE CORRUPTED OR INVALID

int32_t rplTestSystemFlag(int32_t flag)
{
    if(flag > -1 || flag < -128)
        return -1;
    if(!ISBINDATA(*SystemFlags))
        return -2;

    //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
    word_p low64 = SystemFlags + 1;
    int32_t result;
    flag = -flag - 1;
    result = low64[flag >> 5] & (1 << (flag & 31));
    if(result)
        return 1;
    return 0;

}

// RETURN 0/1 IF THE MULTIPLE FLAGS MATCH THE SETTINGS
// RETURN -1 IF THE NAME IS NOT VALID
// RETURN -2 IF SYSTEM FLAGS ARE CORRUPTED OR INVALID

int32_t rplTestSystemFlagByName(byte_p name, byte_p nameend)
{
    if(!ISBINDATA(*SystemFlags))
        return -2;
    int32_t idx = 0;
    int32_t len = utf8nlen((char *)name, (char *)nameend);
    int32_t flaglen;
    while(flags_names[idx].flagname) {
        flaglen = utf8len((char *)flags_names[idx].flagname);
        if((flaglen == len)
                && !utf8ncmp2((char *)name, (char *)nameend,
                    flags_names[idx].flagname, len)) {
            int32_t count;
            int32_t match = 0;
            for(count = 0; count < 8; ++count) {
                if(flags_names[idx].flags[count]) {
                    int32_t flag = flags_names[idx].flags[count] & 0x7f;
                    int32_t value = flags_names[idx].flags[count] >> 7;

                    word_p low64 = SystemFlags + 1;
                    int32_t res;
                    flag--;

                    res = (low64[flag >> 5] >> (flag & 31)) & 1;
                    match |= (value ^ res);
                }
            }
            if(!match)
                return 1;       // MATCH LOGIC IS REVERSED
            else
                return 0;

        }
        ++idx;
    }
    return -1;

}

int32_t rplSetSystemFlagByIdent(word_p ident)
{
    byte_p text = (byte_p) (ident + 1);
    return rplSetSystemFlagByName(text, text + rplGetIdentLength(ident));
}

int32_t rplClrSystemFlagByIdent(word_p ident)
{
    byte_p text = (byte_p) (ident + 1);
    return rplClrSystemFlagByName(text, text + rplGetIdentLength(ident));
}

int32_t rplTestSystemFlagByIdent(word_p ident)
{
    byte_p text = (byte_p) (ident + 1);
    return rplTestSystemFlagByName(text, text + rplGetIdentLength(ident));
}

// RETURN THE SYSTEM LOCALE WORD, CONTAINING THE CHARACTERS TO BE USED FOR NUMBERS
uint64_t rplGetSystemLocale()
{
    word_p systemlist = rplGetSettings((word_p) numfmt_ident);
    if(systemlist) {
        if(ISLIST(*systemlist)) {
            word_p localestring = rplGetListElement(systemlist, 1);
            // EXPAND THE STRING INTO FOUR UNICODE CODEPOINTS
            if(localestring && (ISSTRING(*localestring))) {
                uint64_t result;
                byte_p locptr = (byte_p) (localestring + 1), locend =
                        (byte_p) rplSkipOb(localestring);
                result = utf82cp((char *)locptr, (char *)locend);
                locptr = (byte_p) utf8skip((char *)locptr, (char *)locend);
                result |=
                        ((uint64_t) (utf82cp((char *)locptr,
                                (char *)locend) & 0xffff)) << 16;
                locptr = (byte_p) utf8skip((char *)locptr, (char *)locend);
                result |=
                        ((uint64_t) (utf82cp((char *)locptr,
                                (char *)locend) & 0xffff)) << 32;
                locptr = (byte_p) utf8skip((char *)locptr, (char *)locend);
                result |=
                        ((uint64_t) (utf82cp((char *)locptr,
                                (char *)locend) & 0xffff)) << 48;
                locptr = (byte_p) utf8skip((char *)locptr, (char *)locend);

                return result;
            }
        }
    }
    // INVALID FLAGS, JUST RETURN A DEFAULT SETTING
    return SYSTEM_DEFAULT_LOCALE;

}

// FILLS OUT THE NUMFORMAT STRUCTURE WITH INFORMATION FROM THE SYSTEM FLAGS
// IT PROVIDES DEFAULTS IF SYSTEM FLAGS ARE INVALID, NEVER FAILS
// fmt MUST POINT TO A PREVIOUSLY ALLOCATED NUMFORMAT STRUCTURE THAT WILL BE
// OVERWRITTEN, NO NULL CHECKS

void rplGetSystemNumberFormat(NUMFORMAT * fmt)
{
    word_p systemlist = rplGetSettings((word_p) numfmt_ident);
    if(systemlist) {
        if(ISLIST(*systemlist)) {
            word_p localestring = rplGetListElement(systemlist, 1);
            if(localestring && (ISSTRING(*localestring))) {
                uint64_t result;
                byte_p locptr = (byte_p) (localestring + 1), locend =
                        (byte_p) rplSkipOb(localestring);
                result = utf82cp((char *)locptr, (char *)locend);
                locptr = (byte_p) utf8skip((char *)locptr, (char *)locend);
                result |=
                        ((uint64_t) (utf82cp((char *)locptr,
                                (char *)locend) & 0xffff)) << 16;
                locptr = (byte_p) utf8skip((char *)locptr, (char *)locend);
                result |=
                        ((uint64_t) (utf82cp((char *)locptr,
                                (char *)locend) & 0xffff)) << 32;
                locptr = (byte_p) utf8skip((char *)locptr, (char *)locend);
                result |=
                        ((uint64_t) (utf82cp((char *)locptr,
                                (char *)locend) & 0xffff)) << 48;
                locptr = (byte_p) utf8skip((char *)locptr, (char *)locend);

                fmt->Locale = result;
            }
            else
                fmt->Locale = SYSTEM_DEFAULT_LOCALE;
            word_p nfmt = rplGetListElement(systemlist, 2);
            if(nfmt && (ISint32_t(*nfmt)))
                fmt->SmallFmt = (int32_t) rplReadint32_t(nfmt);
            else
                fmt->SmallFmt = 12 | FMT_SCI | FMT_SUPRESSEXP | FMT_USECAPITALS;
            nfmt = rplGetListElement(systemlist, 3);
            if(nfmt && (ISint32_t(*nfmt)))
                fmt->MiddleFmt = (int32_t) rplReadint32_t(nfmt);
            else
                fmt->MiddleFmt = 12 | FMT_USECAPITALS;
            nfmt = rplGetListElement(systemlist, 4);
            if(nfmt && (ISint32_t(*nfmt)))
                fmt->BigFmt = (int32_t) rplReadint32_t(nfmt);
            else
                fmt->BigFmt = 12 | FMT_SCI | FMT_SUPRESSEXP | FMT_USECAPITALS;
            nfmt = rplGetListElement(systemlist, 5);
            if(nfmt && (ISNUMBER(*nfmt)))
                rplReadNumberAsReal(nfmt, &(fmt->SmallLimit));
            else {
                rplReadNumberAsReal((word_p) one_bint, &(fmt->SmallLimit));
                fmt->SmallLimit.exp = -12;
            }
            nfmt = rplGetListElement(systemlist, 6);
            if(nfmt && (ISNUMBER(*nfmt)))
                rplReadNumberAsReal(nfmt, &(fmt->BigLimit));
            else {
                rplReadNumberAsReal((word_p) one_bint, &(fmt->BigLimit));
                fmt->BigLimit.exp = 12;
            }

            // Move SmallLimit and BigLimit numbers data into the structure, truncate to 32 digits maximum (these limits are typically 1Exxx, no need for many digits)

            if(fmt->SmallLimit.len>4) {
            memmovew(fmt->SmallLimitData,fmt->SmallLimit.data+fmt->SmallLimit.len-4,4);
            fmt->SmallLimit.data=fmt->SmallLimitData;
            fmt->SmallLimit.exp+=8*(fmt->SmallLimit.len-4);
            fmt->SmallLimit.len=4;
            }
            else {
                memmovew(fmt->SmallLimitData,fmt->SmallLimit.data,fmt->SmallLimit.len);
                fmt->SmallLimit.data=fmt->SmallLimitData;
            }

            if(fmt->BigLimit.len>4) {
            memmovew(fmt->BigLimitData,fmt->BigLimit.data+fmt->BigLimit.len-4,4);
            fmt->BigLimit.data=fmt->BigLimitData;
            fmt->BigLimit.exp+=8*(fmt->BigLimit.len-4);
            fmt->BigLimit.len=4;
            }
            else {
                memmovew(fmt->BigLimitData,fmt->BigLimit.data,fmt->BigLimit.len);
                fmt->BigLimit.data=fmt->BigLimitData;
            }

            return;

        }
    }

    fmt->Locale = SYSTEM_DEFAULT_LOCALE;
    fmt->SmallFmt = 12 | FMT_SCI | FMT_SUPRESSEXP | FMT_USECAPITALS;
    fmt->MiddleFmt = 12 | FMT_USECAPITALS;
    fmt->BigFmt = 12 | FMT_SCI | FMT_SUPRESSEXP | FMT_USECAPITALS;

    fmt->SmallLimit.data = fmt->SmallLimitData;
    newRealFromint32_t(&fmt->SmallLimit,1,-12);

    fmt->BigLimit.data=fmt->BigLimitData;
    newRealFromint32_t(&fmt->BigLimit,1,12);
}

// SETS THE SYSTEM SETTING NUMFORMAT TO THE GIVEN STRUCTURE
// CAN TRIGGER GC, USES AND SCRATCHPOINTERS
void rplSetSystemNumberFormat(NUMFORMAT * fmt)
{
    // CREATE THE LIST WITH THE NUMFORMAT
    word_p *savestk = DSTop;

    // MAKE THE LOCALE STRING

    BYTE locbase[16];   // 4 BYTES PER UNICODE CHARACTER MAXIMUM
    byte_p locstr = locbase;

    WORD uchar;

    uchar = cp2utf8((int)DECIMAL_DOT(fmt->Locale));
    while(uchar) {
        *locstr++ = uchar & 0xff;
        uchar >>= 8;
    }

    uchar = cp2utf8((int)THOUSAND_SEP(fmt->Locale));
    while(uchar) {
        *locstr++ = uchar & 0xff;
        uchar >>= 8;
    }

    uchar = cp2utf8((int)FRAC_SEP(fmt->Locale));
    while(uchar) {
        *locstr++ = uchar & 0xff;
        uchar >>= 8;
    }

    uchar = cp2utf8((int)ARG_SEP(fmt->Locale));
    while(uchar) {
        *locstr++ = uchar & 0xff;
        uchar >>= 8;
    }

    word_p item = rplCreateString(locbase, locstr);
    if(!item)
        return;

    rplPushData(item);
    rplNewint32_tPush(fmt->SmallFmt, DECint32_t);
    if(Exceptions) {
        DSTop = savestk;
        return;
    }
    rplNewint32_tPush(fmt->MiddleFmt, DECint32_t);
    if(Exceptions) {
        DSTop = savestk;
        return;
    }
    rplNewint32_tPush(fmt->BigFmt, DECint32_t);
    if(Exceptions) {
        DSTop = savestk;
        return;
    }
    rplNewRealPush(&fmt->SmallLimit);
    if(Exceptions) {
        DSTop = savestk;
        return;
    }
    rplNewRealPush(&fmt->BigLimit);
    if(Exceptions) {
        DSTop = savestk;
        return;
    }
    word_p newlist = rplCreateListN(6, 1, 1);
    if(Exceptions) {
        DSTop = savestk;
        return;
    }

    rplStoreSettings((word_p) numfmt_ident, newlist);

    DSTop = savestk;
    return;

}

#define MAXFMTLEN (1+3+2+6+1+2+2+1+1)

// CREATE A STRING OBJECT REPRESENTING ALL POSSIBLE OPTIONS IN fmtbits
// RETURNS NULL ON NOT ENOUGH MEMORY ONLY
word_p rplNumFormat2String(int32_t fmtbits)
{
    // STRING FORMAT:

    // "#" = NO DECIMAL DIGITS AFTER THE DOT, NO TRAILING DOT IF APPROXIMATED
    // "#.###" = STANDARD FORMAT WITH UP TO 3 DIGITS AFTER DECIMAL DOT
    // "#.250#" = SAME WITH 250 DIGITS
    // "#.A#" = ALL AVAILABLE DIGITS AFTER THE DOT
    // "#.000" OR "#.##0" = 3 DIGITS, ADD TRAILING ZEROS
    // "#.250#0" = 250 DIGITS, ADD TRAILING ZEROS
    // "#.###E" = 4 SIGNIFICANT FIGURES TOTAL, SCI NOTATION WITH CAPITAL 'E'
    // "#.249#0E" = 250 TOTAL SIGNIFICANT FIGURES, SCI NOTATION, ADD TRAILING ZEROS
    // "+#" = FORCE + SIGN ON NUMBER
    // "#.###E+" = FORCE + SIGN ON EXPONENT
    // "#.###E+#" = ENGINEERING MODE WITH FORCED SIGN AND NO PREFERRED EXPONENT
    // "#.###E-12" = ENGINEERING MODE WITH PREFERRED EXPONENT 10^-12
    // "+S#.250#0S.E" = FORCE PLUS, SEPARATOR EVERY 3 DIGITS (BY DEFAULT) ON INTEGER PART, 250 DECIMAL FIGURES, TRAILING ZEROS, SEPARATOR FRACTIONAL PART, USE TRAILING DOT
    // "+#.####S4." = FORCE PLUS, 4 DECIMAL DIGITS, SEPARATOR EVERY 4 DIGITS ON FRACTIONAL PART, APPEND TRAILING DOT WHEN APPROXIMATED NUMBER.
    // NOTE THAT USING Sn, n DIGITS MUST BE THE SAME ON INTEGER AND FRACTIONAL PART, DIFFERENT GROUPING IS NOT SUPPORTED.
    // "#.###E*3" = ENGINEERING MODE, 4 TOTAL DIGITS, PREFERRED EXPONENT OF 3, * = SUPPRESS EXPONENT (FOR USER TO APPEND UNIT, ETC.)
    // "#.###E*" = SCI MODE, 4 TOTAL DIGITS, * = SUPPRESS EXPONENT ONLY WHEN EXPONENT IS 0 (1.234 INSTEAD OF 1.234E0)
    // "#.###E*#" = ENGINEERING MODE, 4 TOTAL DIGITS, NO PREFERRED EXPONENT, SUPRESS EXPONENT ONLY WHEN 0

    BYTE str[MAXFMTLEN];
    int32_t ndigits = FMT_DIGITS(fmtbits);
    int32_t offset = 0;

    // FORCE SIGN BIT
    if(fmtbits & FMT_FORCESIGN)
        str[offset++] = '+';
    // SEPARATOR ON INTEGER PART
    if(fmtbits & FMT_NUMSEPARATOR) {
        int32_t sepspacing = SEP_SPACING(fmtbits);
        str[offset++] = 'S';
        if((sepspacing != 0) && (sepspacing != 3)) {
            if(sepspacing >= 10) {
                str[offset++] = '1';
                sepspacing -= 10;
            }
            str[offset++] = sepspacing + '0';
        }
    }
    str[offset++] = '#';

    // NUMBER OF DIGITS
    //if( (ndigits>0)&&(ndigits<0xfff)&&(fmtbits&(FMT_SCI|FMT_ENG))) --ndigits;    // INCLUDE THE DIGIT BEFORE THE DECIMAL DOT IN THE TOTAL COUNT
    if(ndigits != 0) {
        str[offset++] = '.';
        if(ndigits <= 6) {
            int32_t k;
            for(k = 1; k < ndigits; ++k)
                str[offset++] = '#';
            str[offset++] = (fmtbits & FMT_TRAILINGZEROS) ? '0' : '#';
        }
        else {
            // SPELL OUT THE NUMBER OF DIGITS
            if(ndigits < 0xfff) {
                if(ndigits >= 1000) {
                    str[offset++] = '0' + ndigits / 1000;
                    ndigits = ndigits % 1000;
                }
                if(ndigits >= 100) {
                    str[offset++] = '0' + ndigits / 100;
                    ndigits = ndigits % 100;
                }
                if(ndigits >= 10) {
                    str[offset++] = '0' + ndigits / 10;
                    ndigits = ndigits % 10;
                }
                str[offset++] = '0' + ndigits;
                str[offset++] = '#';
                if(fmtbits & FMT_TRAILINGZEROS)
                    str[offset++] = '0';
            }
            else {
                // SPECIFY ALL DIGITS
                str[offset++] = 'A';
                str[offset++] = '#';
            }

        }

        // FRACTIONAL SEPARATOR
        if(fmtbits & FMT_FRACSEPARATOR) {
            str[offset++] = 'S';
            int32_t sepspacing = SEP_SPACING(fmtbits);
            if(!(fmtbits & FMT_NUMSEPARATOR) && (sepspacing != 0)
                    && (sepspacing != 3)) {
                // NEED TO INCLUDE THE SEPARATOR SPACING HERE
                if(sepspacing >= 10) {
                    str[offset++] = '1';
                    sepspacing -= 10;
                }
                str[offset++] = sepspacing + '0';
            }

        }

    }

    // USE TRAILING DOT?
    if(!(fmtbits & FMT_NOTRAILDOT))
        str[offset++] = '.';

    // SCI OR ENG MODE?

    if(fmtbits & (FMT_SCI | FMT_ENG)) {
        str[offset++] = (fmtbits & FMT_USECAPITALS) ? 'E' : 'e';
        if(fmtbits & FMT_SUPRESSEXP)
            str[offset++] = '*';
        if(fmtbits & FMT_EXPSIGN)
            str[offset++] = '+';
        if(fmtbits & FMT_ENG) {
            int32_t pref = PREFERRED_EXP(fmtbits);
            if(pref == -24)
                str[offset++] = '#';    // NO PREFERRED EXPONENT
            else {
                if(pref < 0) {
                    str[offset++] = '-';
                    pref = -pref;
                }
                if(pref >= 20) {
                    str[offset++] = '2';
                    pref -= 20;
                }
                if(pref >= 10) {
                    str[offset++] = '1';
                    pref -= 10;
                }
                str[offset++] = '0' + pref;
            }

        }
    }

    // STRING IS READY

    return rplCreateString(str, str + offset);

}

// CONVERT A STRING BACK INTO NUMBER FORMAT
// RETURN -1 IF INVALID STRING, DOESN'T THROW ANY ERRORS

int32_t rplNumFormatFromString(word_p string)
{
    byte_p str = (byte_p) (string + 1);
    byte_p end = str + rplStrSize(string);

    int32_t fmt = FMT_NOTRAILDOT | FMT_USECAPITALS;

    // FORCE SIGN?
    if(*str == '+') {
        fmt |= FMT_FORCESIGN;
        ++str;
    }

    if(str >= end)
        return -1;      // INCOMPLETE DEFINITION

    if(*str == 'S') {
        int32_t sepspacing = 0;
        ++str;
        if(str >= end)
            return -1;  // INCOMPLETE DEFINITION
        if((*str >= '0') && (*str <= '9')) {
            sepspacing = *str - '0';
            ++str;
        }
        if(str >= end)
            return -1;  // INCOMPLETE DEFINITION
        if((*str >= '0') && (*str <= '9')) {
            sepspacing = sepspacing * 10 + (*str - '0');
            ++str;
        }
        if(str >= end)
            return -1;  // INCOMPLETE DEFINITION

        if((sepspacing < 1) || (sepspacing > 15))
            sepspacing = 3;

        fmt |= FMT_GROUPDIGITS(sepspacing);
        fmt |= FMT_NUMSEPARATOR;
    }

    if(*str != '#')
        return -1;

    ++str;

    if(str >= end)
        return fmt;     // COMPLETE DEFINITION

    if(*str == '.') {
        ++str;
        if(str == end) {
            fmt ^= FMT_NOTRAILDOT;
            return fmt;
        }
    }
    else return -1;

    // NUMBER OF FIGURES

    int32_t ndig = 0, isnumeric = 0;
    while(str < end) {
        if(isnumeric) {
            if((*str >= '0') && (*str <= '9')) {
                ndig *= 10;
                ndig += *str - '0';
                ++str;
                continue;
            }
            if(*str == '#') {
                ++str;
                break;
            }
            return -1;
        }
        else {
            if(*str == 'A') {
                if(ndig != 0)
                    return -1;  // INVALID STRING
                ndig = 0xfff;
                ++str;
                continue;
            }
            if(*str == '#') {
                if(ndig == 0xfff) {
                    ++str;
                    break;
                }
                ++ndig;
                ++str;
                continue;
            }
            if(*str == '0') {
                if(ndig == 0xfff)
                    return -1;
                ++ndig;
                ++str;
                continue;
            }
            if((*str > '0') && (*str <= '9')) {
                if(ndig)
                    return -1;
                isnumeric = 1;
                continue;       // DO NOT INCREASE str, SO THE DIGIT IS CAUGHT AGAIN
            }
            if(ndig == 0xfff)
                return -1;
            if(*str == 'S')
                break;
            if(*str == '.')
                break;
            if(*str == 'E')
                break;
            if(*str == 'e')
                break;
            return -1;
        }
    }

    // HERE WE HAVE A NUMBER OF DIGITS, THERE COULD BE AN ADDITIONAL ZERO

    if((str < end) && (*str == '0')) {
        if((!isnumeric) && (ndig != 0xfff))
            ++ndig;

        fmt |= FMT_TRAILINGZEROS;
        ++str;
    }
    else {
        // NO ADDITIONAL ZERO, CHECK IF THE PREVIOUS ONE WAS A ZERO
        if(str[-1] == '0') {
            fmt |= FMT_TRAILINGZEROS;

        }
    }

    if(ndig == 0xfff)
        fmt &= ~FMT_TRAILINGZEROS;      // TRAILING ZEROS NOT ALLOWED WHEN USING ALL AVAILABLE DIGITS!
    fmt |= FMT_DIGITS(ndig);

    if(str == end)
        return fmt;     // DONE, NO EXPONENT

    // FRACTIONAL SEPARATOR?
    if(*str == 'S') {
        int32_t sepspacing = 0;
        ++str;
        if(str >= end)
            return fmt;
        if((*str >= '0') && (*str <= '9')) {
            sepspacing = *str - '0';
            ++str;
        }
        if(str < end) {
            if((*str >= '0') && (*str <= '9')) {
                sepspacing = sepspacing * 10 + (*str - '0');
                ++str;
            }
        }
        if((sepspacing < 1) || (sepspacing > 15))
            sepspacing = 3;

        if(fmt & FMT_NUMSEPARATOR) {
            if(sepspacing != SEP_SPACING(fmt)) {
                // INVALID, SPACING MUST BE THE SAME ON BOTH INTEGER AND FRACTIONAL PARTS
                return -1;
            }
        }
        else
            fmt |= FMT_GROUPDIGITS(sepspacing);
        fmt |= FMT_FRACSEPARATOR;
    }

    // ONLY ACCEPTABLE CHARACTERS HERE ARE THE EXPONENT LETTER OR THE TRAILING DOT

    if(str == end)
        return fmt;     // DONE, NO EXPONENT

    if(*str == '.') {
        ++str;
        fmt ^= FMT_NOTRAILDOT;
    }

    if(str == end)
        return fmt;     // DONE, NO EXPONENT

    // ONLY ACCEPTABLE CHARACTERS HERE ARE THE EXPONENT LETTER
    if(*str == 'E') {
        ++str;
        fmt |= FMT_SCI;
    }
    else if(*str == 'e') {
        ++str;
        fmt &= ~FMT_USECAPITALS;
        fmt |= FMT_SCI;
    }
    else
        return -1;      // INVALID FORMAT

    if(str == end)
        return fmt;     // DONE, NO EXPONENT

    if(*str == '*') {
        ++str;
        fmt |= FMT_SUPRESSEXP;
    }

    if(str == end)
        return fmt;     // DONE, NO EXPONENT

    if(*str == '+') {
        ++str;
        fmt |= FMT_EXPSIGN;
    }

    if(str == end)
        return fmt;     // DONE, NO EXPONENT

    if(*str == '#') {
        fmt |= FMT_ENG; // ENGINEERING MODE, NO PREFERRED EXPONENT
        ++str;
        if(str != end)
            return -1;  // THIS IS SUPPOSED TO BE THE END!
    }
    else {
        // GET A PREFERRED EXPONENT
        int32_t isneg, prefexp;
        if(*str == '-') {
            isneg = 1;
            ++str;
            if(str == end)
                return -1;
        }
        else
            isneg = 0;

        if((*str >= '0') && (*str <= '9')) {
            prefexp = *str - '0';
            ++str;
        }
        else
            return -1;
        if(str < end) {
            if((*str >= '0') && (*str <= '9')) {
                prefexp = prefexp * 10 + (*str - '0');
                ++str;
            }
        }

        if(str != end)
            return -1;  // THIS IS THE LAST ITEM

        if(prefexp % 3)
            return -1;  // MUST BE A MULTIPLE OF 3
        if(prefexp > 21)
            return -1;  // AND IN THE RANGE +21/-21

        if(isneg)
            prefexp = -prefexp;

        fmt |= FMT_ENG | FMT_PREFEREXPONENT(prefexp);

    }

    return fmt;

}

// ONLY VALID menunumbers ARE 1 AND 2
// THIS MAY CHANGE IN OTHER IMPLEMENTATIONS

void rplSetMenuCode(int32_t menunumber, int64_t menucode)
{
    if(!ISBINDATA(*SystemFlags))
        return;

    if((menunumber < 1) || (menunumber > 2))
        return;

    SystemFlags[4 + menunumber] = menucode;
    SystemFlags[6 + menunumber] = (menucode >> 32);

    return;
}

int64_t rplGetMenuCode(int32_t menunumber)
{
    if(!ISBINDATA(*SystemFlags))
        return 0;

    if((menunumber < 1) || (menunumber > 2))
        return 0;

    return ((int64_t) SystemFlags[4 + menunumber]) | (((int64_t) SystemFlags[6 +
                    menunumber]) << 32);

}

void rplSetLastMenu(int32_t menunumber)
{
    if((menunumber < 1) || (menunumber > 2))
        return;

    if(menunumber == 1) {
        rplClrSystemFlag(FL_LASTMENU);
        return;
    }
    rplSetSystemFlag(FL_LASTMENU);

}

void rplSetActiveMenu(int32_t menunumber)
{
    if((menunumber < 1) || (menunumber > 2))
        return;

    if(menunumber == 1) {
        rplClrSystemFlag(FL_ACTIVEMENU);
        return;
    }
    rplSetSystemFlag(FL_ACTIVEMENU);

}

int32_t rplGetLastMenu()
{
    int32_t a = rplTestSystemFlag(FL_LASTMENU);

    if(a == 1)
        return 2;
    return 1;
}

int32_t rplGetActiveMenu()
{
    int32_t a = rplTestSystemFlag(FL_ACTIVEMENU);

    if(a == 1)
        return 2;
    return 1;
}

// PUSH THE CURRENT MENU INTO THE MENU HISTORY
// CAN TRIGGER GC AND USES ScratchPointers 1 thru 3
void rplSaveMenuHistory(int32_t menu)
{
    int64_t oldmcode = rplGetMenuCode(menu);

    // STORE THE OLD MENU IN THE HISTORY
    word_p msetting;

    if((MENULIBRARY(oldmcode) == LIBRARY_NUMBER) && (MENUNUMBER(oldmcode) < 2)) {
        // SPECIAL CUSTOM MENUS, RCL FROM THE SETTINGS DIRECTORY
        if(menu == 1)
            msetting = rplGetSettings((word_p) menu1_ident);
        else if(menu == 2)
            msetting = rplGetSettings((word_p) menu2_ident);
        else
            msetting = 0;

        if(!msetting)
            msetting = (word_p) empty_list;    // IF MENU CONTENT CAN'T BE DETERMINED, RETURN AN EMPTY CUSTOM MENU
    }
    else {
        // NOTHING CUSTOM, JUST RETURN THE MENU CODE
        // THIS CAN TRIGGER A GC!
        msetting = rplNewint32_t(oldmcode, HEXint32_t);
    }

    if(!msetting)
        return;

    // HERE msetting HAS EITHER THE OBJECT OR CODE TO STORE IN THE HISTORY

    word_p oldlist =
            rplGetSettings((menu ==
                1) ? (word_p) menu1hist_ident : (word_p) menu2hist_ident);
    if(!oldlist)
        oldlist = (word_p) empty_list;
    word_p levels = rplGetSettings((word_p) menuhistory_ident);
    if(!levels)
        levels = (word_p) ten_bint;

    int64_t nlevels = rplReadNumberAsInt64(levels);

    // THIS CAN TRIGGER A GC!
    word_p newlist = rplListAddRot(oldlist, msetting, nlevels);

    if(!newlist)
        return;

    rplStoreSettings((menu ==
                1) ? (word_p) menu1hist_ident : (word_p) menu2hist_ident,
            newlist);

    return;
}

// RCL AND REMOVE FROM HISTORY THE LAST ITEM IN THE LIST

word_p rplPopMenuHistory(int32_t menu)
{
    word_p list =
            rplGetSettings((menu ==
                1) ? (word_p) menu1hist_ident : (word_p) menu2hist_ident);
    if(!list)
        return 0;

    int32_t nelem = rplExplodeList2(list);
    if(nelem < 1)
        return 0;

    ScratchPointer1 = rplPopData();

    list = rplCreateListN(nelem - 1, 1, 1);
    if(!list)
        list = (word_p) empty_list;

    rplStoreSettings((menu ==
                1) ? (word_p) menu1hist_ident : (word_p) menu2hist_ident,
            list);

    return ScratchPointer1;

}

// REPLACE THE ACTIVE MENU WITH THE GIVEN OBJECT
// THIS DOES THE SAME JOB AS TMENU, BUT CAN BE CALLED
// FROM OTHER LIBRARIES. MAY TRIGGER GC WHEN STORING
// IN SETTINGS DIRECTORY
void rplChangeMenu(int32_t menu, word_p newmenu)
{

    if(ISLIST(*newmenu) || ISIDENT(*newmenu)) {
        // CUSTOM MENU

        int64_t mcode = MKMENUCODE(0, LIBRARY_NUMBER, menu - 1, 0);

        rplSetMenuCode(menu, mcode);

        // STORE THE LIST IN .Settings AS CURRENT MENU
        if(menu == 2)
            rplStoreSettings((word_p) menu2_ident, newmenu);
        else
            rplStoreSettings((word_p) menu1_ident, newmenu);

        return;
    }

    if(ISint32_t(*newmenu)) {
        // IT'S A PREDEFINED MENU CODE
        int64_t num = rplReadint32_t(newmenu);

        if(num < 0) {
            // JUST SET IT TO ZERO
            rplSetMenuCode(menu, 0);
            // STORE THE LIST IN .Settings AS CURRENT MENU
            if(menu == 2)
                rplStoreSettings((word_p) menu2_ident, (word_p) zero_bint);
            else
                rplStoreSettings((word_p) menu1_ident, (word_p) zero_bint);

        }
        else {
            // WE HAVE A VALID MENU NUMBER

            rplSetMenuCode(menu, num);
            // STORE THE LIST IN .Settings AS CURRENT MENU
            if(menu == 2)
                rplStoreSettings((word_p) menu2_ident, newmenu);
            else
                rplStoreSettings((word_p) menu1_ident, newmenu);

        }

        return;
    }

    rplError(ERR_INVALIDMENUDEFINITION);

    return;
}

// CONVERT A KEY NAME GIVEN BY A STRING INTO A KEY MESSAGE
// RETURN 0 ON INVALID KEY
/*
"keyname" is a string with the key name:
Format:
"KEY.SHIFT.MODIFIER"
KEY = "A thru Z, 0 thru 9, * - + . or UP/DN/LF/RT, BK for backspace, EN for enter
SHIFT = L, LH,R, RH,A, AH,OH (L=left shift, LH=left shift hold, A=Alpha, OH=On-hold). Notice that On can only be defined with a hold plane.
MODIFIER = P,R,L (P=key down, R=key released, L=long press, no modifier means the standard KM_PRESS event)

Examples:
"7.L" = Left-shift 7
"7.LH" = Left-shift-hold 7
"7..P" = Action executed on key-down event for unshifted key number 7
"7..R" = Action executed on key-up event
*/

// WARNING!! THESE TABLES COULD BE HARDWARE-DEPENDANT
// IT WORKS FOR MOST CALCULATORS AS LONG AS THE CONSTANTS IN
// hal_api.h ARE CORRECT.

const BYTE const keytable[] = {
    'O', 'N', KB_ON,
    'A', 'L', KB_ALPHA,
    'L', 'S', KB_LSHIFT,
    'R', 'S', KB_RSHIFT,
    'B', 'K', KB_BKS,
    'E', 'N', KB_ENT,
    'U', 'P', KB_UP,
    'D', 'N', KB_DN,
    'L', 'F', KB_LF,
    'R', 'T', KB_RT,
    'S', 'P', KB_SPC,
    'D', 'T', KB_DOT,
    '0', 0, KB_0,
    '1', 0, KB_1,
    '2', 0, KB_2,
    '3', 0, KB_3,
    '4', 0, KB_4,
    '5', 0, KB_5,
    '6', 0, KB_6,
    '7', 0, KB_7,
    '8', 0, KB_8,
    '9', 0, KB_9,
    'A', 0, KB_A,
    'B', 0, KB_B,
    'C', 0, KB_C,
    'D', 0, KB_D,
    'E', 0, KB_E,
    'F', 0, KB_F,
    'G', 0, KB_G,
    'H', 0, KB_H,
    'I', 0, KB_I,
    'J', 0, KB_J,
    'K', 0, KB_K,
    'L', 0, KB_L,
    'M', 0, KB_M,
    'N', 0, KB_N,
    'O', 0, KB_O,
    'P', 0, KB_P,
    'Q', 0, KB_Q,
    'R', 0, KB_R,
    'S', 0, KB_S,
    'T', 0, KB_T,
    'U', 0, KB_U,
    'V', 0, KB_V,
    'W', 0, KB_W,
    'X', 0, KB_X,
    'Y', 0, KB_Y,
    'Z', 0, KB_Z,
    '.', 0, KB_DOT,
    '/', 0, KB_DIV,
    '*', 0, KB_MUL,
    '+', 0, KB_ADD,
    '-', 0, KB_SUB,
// HP Prime additional keys
    'A', 'P', KB_APP,
    'H', 'O', KB_HOM,
    'S', 'Y', KB_SYM,
    'P', 'L', KB_PLT,
    'N', 'U', KB_NUM,
    'H', 'L', KB_HLP,
    'V', 'W', KB_VIE,
    'M', 'E', KB_MEN,
    'C', 'A', KB_CAS,
    0, 0, 0
};

const BYTE const modiftable[] = {
    'A', 'L', 'H', (SHIFT_ALPHA | SHIFT_LSHOLD) >> 4,
    'A', 'R', 'H', (SHIFT_ALPHA | SHIFT_RSHOLD) >> 4,
    'L', 'H', 0, SHIFT_LSHOLD >> 4,
    'R', 'H', 0, SHIFT_RSHOLD >> 4,
    'A', 'L', 0, (SHIFT_LS | SHIFT_ALPHA) >> 4,
    'A', 'R', 0, (SHIFT_RS | SHIFT_ALPHA) >> 4,
    'A', 'H', 0, SHIFT_ALPHAHOLD >> 4,
    'O', 'H', 0, SHIFT_ONHOLD >> 4,
    'L', 0, 0, SHIFT_LS >> 4,
    'R', 0, 0, SHIFT_RS >> 4,
    'A', 0, 0, SHIFT_ALPHA >> 4,
    0, 0, 0, 0
};

WORD rplKeyName2Msg(word_p keyname)
{
    byte_p ptr, tblptr;
    int32_t key, shifts, msg, len;

    if(!ISSTRING(*keyname))
        return 0;

    ptr = (byte_p) (keyname + 1);
    len = rplStrSize(keyname);

    // IDENTIFY KEY NAME FIRST
    key = 0;

    if((len < 2) || ((len >= 2) && (ptr[1] == '.'))) {
        // SINGLE-LETTER DEFINITION
        key = ptr[0];
        ++ptr;
        if(len >= 2) {
            ++ptr;
            len -= 2;
        }
        else
            --len;

    }
    else {
        // TRY SPECIAL 2-LETTER KEY NAME
        if((len < 3) || ((len >= 3) && (ptr[2] == '.'))) {
            // TWO-LETTER DEFINITION
            key = ptr[0] + 256 * ptr[1];
            ptr += 2;
            if(len >= 3) {
                ++ptr;
                len -= 3;
            }
            else
                len -= 2;
        }
    }

    tblptr = (byte_p) keytable;

    while(*tblptr) {
        if((tblptr[0] == (key & 0xff)) && (tblptr[1] == ((key >> 8) & 0xff))) {
            key = tblptr[2];
            break;
        }
        tblptr += 3;
    }

    if(!(*tblptr))
        return 0;

    // HERE WE HAVE THE KEY CODE

    // LOOK FOR MODIFIERS

    shifts = 0;
    if(len > 0) {

        if(ptr[0] == '.')
            ++ptr;
        else if((len < 2) || ((len >= 2) && (ptr[1] == '.'))) {
            // SINGLE-LETTER DEFINITION
            shifts = ptr[0];
            ++ptr;
            if(len >= 2) {
                ++ptr;
                len -= 2;
            }
            else
                --len;

        }
        else {
            // 2-LETTER MODIFIER
            if((len < 3) || ((len >= 3) && (ptr[2] == '.'))) {
                // TWO-LETTER DEFINITION
                shifts = ptr[0] + 256 * ptr[1];
                ptr += 2;
                if(len >= 3) {
                    ++ptr;
                    len -= 3;
                }
                else
                    len -= 2;
            }
            else {
                // 3-LETTER MODIFIER
                if((len < 4) || ((len >= 4) && (ptr[3] == '.'))) {
                    // THREE-LETTER DEFINITION
                    shifts = ptr[0] + 256 * ptr[1] + 65536 * ptr[2];
                    ptr += 3;
                    if(len >= 4) {
                        ++ptr;
                        len -= 4;
                    }
                    else
                        len -= 3;

                }

            }
        }

        if(shifts) {

            tblptr = (byte_p) modiftable;

            while(*tblptr) {
                if((tblptr[0] == (shifts & 0xff))
                        && (tblptr[1] == ((shifts >> 8) & 0xff))
                        && (tblptr[2] == ((shifts >> 16) & 0xff))) {
                    shifts = tblptr[3];
                    break;
                }
                tblptr += 4;
            }

            if(!(*tblptr))
                return 0;

            shifts <<= 4;
        }
    }
    // FINALLY, LOOK FOR MESSAGE

    msg = 0;
    if(len > 0) {
        switch (ptr[0]) {
        case 'D':
            msg = KM_KEYDN;
            break;
        case 'U':
            msg = KM_KEYUP;
            break;
        case 'L':
            msg = KM_LPRESS;
            break;
        case 'P':
            msg = KM_PRESS;
            break;
        case 'R':
            msg = KM_REPEAT;
            break;
        case 'T':
            msg = KM_LREPEAT;
            break;
        default:
            return 0;   // INVALID KEY MESSAGE

        }

    }

    // ASSEMBLE THE RESPONSE AND RETURN

    return msg | shifts | key;

}

// CREATE A STRING OBJECT FROM A KEYBOARD MESSAGE

word_p rplMsg2KeyName(WORD keymsg)
{

    byte_p tblptr = (byte_p) keytable, keyptr;
    BYTE keytext[16];

    keyptr = keytext;

    if(KM_MESSAGE(keymsg)==KM_TOUCH) {
        *keyptr++='T';
        *keyptr++=KM_TOUCHFINGER(keymsg)+'0';
        switch(KM_TOUCHEVENT(keymsg))
        {
        case 0:
            *keyptr++='?';
            break;
        case KM_FINGERDOWN:
            *keyptr++='D';
            break;
        case KM_FINGERMOVE:
            *keyptr++='M';
            break;
        case KM_FINGERUP:
            *keyptr++='U';
        }
        *keyptr++='.';

        int x=KM_TOUCHX(keymsg),startnum=0;

        if(x>=1000) { *keyptr++='0'+(x/1000); x%=1000; startnum=1; }
        if((x>=100)||startnum) { *keyptr++='0'+(x/100); x%=100; startnum=1; }
        if((x>=10)||startnum) { *keyptr++='0'+(x/10); x%=10; startnum=1; }
        *keyptr++='0'+x;

        *keyptr++='.';
        startnum=0;
        x=KM_TOUCHY(keymsg);
        if(x>=1000) { *keyptr++='0'+(x/1000); x%=1000; startnum=1; }
        if((x>=100)||startnum) { *keyptr++='0'+(x/100); x%=100; startnum=1; }
        if((x>=10)||startnum) { *keyptr++='0'+(x/10); x%=10; startnum=1; }
        *keyptr++='0'+x;


    }


    else {


    while(*tblptr) {
        if(tblptr[2] == (KEYVALUE(keymsg))) {
            *keyptr++ = tblptr[0];
            if(tblptr[1])
                *keyptr++ = tblptr[1];
            break;
        }
        tblptr += 3;
    }
    // EMPTY KEY VALUE IS OK FOR PURE KM_SHIFT CHANGE MESSAGES
    /*
    if(!*tblptr) {
        rplError(ERR_INVALIDKEYNAME);
        return 0;
    }
    */

    if(KEYSHIFT(keymsg)) {
        *keyptr++ = '.';

        tblptr = (byte_p) modiftable;

        while(*tblptr) {
            if(tblptr[3] == (KEYSHIFT(keymsg) >> 4)) {
                *keyptr++ = tblptr[0];
                if(tblptr[1])
                    *keyptr++ = tblptr[1];
                if(tblptr[2])
                    *keyptr++ = tblptr[2];
                break;
            }
            tblptr += 4;
        }
        if(!*tblptr) {
            rplError(ERR_INVALIDKEYNAME);
            return 0;
        }

    }

    if(KM_MESSAGE(keymsg) != KM_PRESS) {

        if(!KEYSHIFT(keymsg))
            *keyptr++ = '.';

        *keyptr++ = '.';

        switch (KM_MESSAGE(keymsg)) {
        case KM_LPRESS:
            *keyptr++ = 'L';
            break;
        case KM_KEYUP:
            *keyptr++ = 'U';
            break;
        case KM_KEYDN:
            *keyptr++ = 'D';
            break;
        case KM_REPEAT:
            *keyptr++ = 'R';
            break;
        case KM_LREPEAT:
            *keyptr++ = 'T';
            break;
        }

    }
    }

    // NOW BUILD A STRING OBJECT AND RETURN

    return rplCreateString(keytext, keyptr);

}

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    // LIBRARIES THAT DEFINE ONLY COMMANDS STILL HAVE TO RESPOND TO A FEW OVERLOADABLE OPERATORS
    if(LIBNUM(CurOpcode) == LIB_OVERLOADABLE) {
        // ONLY RESPOND TO EVAL, EVAL1 AND XEQ FOR THE COMMANDS DEFINED HERE
        // IN CASE OF COMMANDS TREATED AS OBJECTS (WHEN EMBEDDED IN LISTS)
        if((OPCODE(CurOpcode) == OVR_EVAL) ||
                (OPCODE(CurOpcode) == OVR_EVAL1) ||
                (OPCODE(CurOpcode) == OVR_XEQ)) {
            // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
            if(rplDepthData() < 1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORD saveOpcode = CurOpcode;
            CurOpcode = *rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode = saveOpcode;
            return;
        }
        // COMPARE COMMANDS WITH "SAME" TO AVOID CHOKING SEARCH/REPLACE COMMANDS IN LISTS
        if(OPCODE(CurOpcode) == OVR_SAME) {
            if(*rplPeekData(2) == *rplPeekData(1)) {
                rplDropData(2);
                rplPushTrue();
            }
            else {
                rplDropData(2);
                rplPushFalse();
            }
            return;
        }
        else {
            rplError(ERR_INVALIDOPCODE);
            return;
        }

    }

    switch (OPCODE(CurOpcode)) {
    case SETLOCALE:
    {
        //@SHORT_DESC=Change the separator symbols
        //@NEW
        NUMFORMAT fmt;
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISSTRING(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        int32_t slen = rplStrLenCp(rplPeekData(1));

        if(slen != 4) {
            rplError(ERR_INVALIDLOCALESTRING);
            return;
        }

        rplGetSystemNumberFormat(&fmt);

        // EXTRACT ALL 4 CODE POINTS
        byte_p locstring, strend;
        strend = (byte_p) (rplPeekData(1) + 1) + rplStrSize(rplPeekData(1));
        locstring = (byte_p) (rplPeekData(1) + 1);
        uint64_t newlocale;

        int32_t cp = utf82cp((char *)locstring, (char *)strend);

        if(cp < 0) {
            rplError(ERR_INVALIDLOCALESTRING);
            return;
        }

        newlocale = cp;

        locstring = (byte_p) utf8skipst((char *)locstring, (char *)strend);

        cp = utf82cp((char *)locstring, (char *)strend);

        if(cp < 0) {
            rplError(ERR_INVALIDLOCALESTRING);
            return;
        }

        newlocale |= (((uint64_t) cp) << 16);

        locstring = (byte_p) utf8skipst((char *)locstring, (char *)strend);

        cp = utf82cp((char *)locstring, (char *)strend);

        if(cp < 0) {
            rplError(ERR_INVALIDLOCALESTRING);
            return;
        }

        newlocale |= (((uint64_t) cp) << 32);

        locstring = (byte_p) utf8skipst((char *)locstring, (char *)strend);

        cp = utf82cp((char *)locstring, (char *)strend);

        if(cp < 0) {
            rplError(ERR_INVALIDLOCALESTRING);
            return;
        }

        newlocale |= (((uint64_t) cp) << 48);

        fmt.Locale = newlocale;

        rplSetSystemNumberFormat(&fmt);
        rplDropData(1);
        return;

    }

    case SETNFMT:
    {
        //@SHORT_DESC=Change the display format for numbers
        //@NEW
        // SET THE SYSTEM NUMBER FORMAT
        // ACCEPT EITHER A SINGLE STRING OR A LIST AS FOLLOWS:
        // FORMAT_STRING --> REPLACE NUMBER FORMAT FOR ALL NUMBERS (BIG, SMALL AND NORMAL)
        // { FORMAT_STRING } --> REPLACE NUMBER FORMAT FOR "NORMAL" NUMBERS ONLY
        // { CUTOFF_LIMIT FORMAT_STRING } --> IF CUTOFF LIMIT<1 THEN IT WILL REPLACE ONLY SMALL NUMBER FORMAT, IF >1 IT REPLACES BIG NUMBER FORMAT
        // { CUTOFF_LIMIT FORMAT_STRING CUTOFF_LIMIT FORMAT_STRING } --> REPLACE BIG OR SMALL FORMATS OR BOTH, BUT NOT THE DEFAULT ONE
        // { DEFAULT_FORMAT_STRING SMALL_CUTOFF SMALL_FORMAT_STRING BIGNUM_CUTOFF BIGNUM_FORMAT_STRING } --> REPLACE ALL 3 FORMATS

        // IF REAL NUMBERS ARE OMITTED, THE DEFAULT CUTOFF VALUES OF 1E12 AND 1E-12 WILL BE USED
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        NUMFORMAT f;
        rplGetSystemNumberFormat(&f);   // GET CURRENT NUMBER FORMAT, TO OVERWRITE

        word_p item;
        int32_t nitems;
        int32_t format;
        REAL num;
        int32_t hasnumber;

        item = rplPeekData(1);
        if(ISLIST(*item)) {
            nitems = rplListLength(item);
            item++;
            hasnumber = 0;
        }
        else {
            nitems = 1;
            hasnumber = -1;
        }

        while(nitems > 0) {
            if(ISNUMBER(*item)) {
                rplReadNumberAsReal(item, &num);
                rplOneToRReg(0);
                if(gteReal(&num, &RReg[0])) {
                    cloneReal(&f.BigLimit, &num);
                    hasnumber = 2;
                }
                else {
                    cloneReal(&f.SmallLimit, &num);
                    hasnumber = 1;
                }
            }
            else {
                if(ISSTRING(*item)) {
                    format = rplNumFormatFromString(item);
                    if(format < 0) {
                        rplError(ERR_INVALIDNUMFORMAT);
                        return;
                    }

                    switch (hasnumber) {

                    case -1:   // SINGLE STRING FOR ALL FORMATS
                        f.BigFmt = f.MiddleFmt = f.SmallFmt = format;
                        break;
                    case 0:    // FIRST STRING, NO NUMBERS BEFORE
                    case 5:    // STRING HAS NO NUMBER PRECEDING, BIG FORMAT WAS ALREADY SET, SO LOOP BACK TO NORMAL FORMAT
                    default:
                        f.MiddleFmt = format;
                        hasnumber = 3;
                        break;
                    case 1:    // STRING HAS A SMALL NUMBER PRECEDING
                    case 3:    // STRING HAS NO NUMBER PRECEDING, MIDDLE FORMAT WAS ALREADY SET
                        f.SmallFmt = format;
                        hasnumber = 4;
                        break;
                    case 2:    // STRING HAS A BIG NUMBER PRECEDING
                    case 4:    // STRING HAS NO NUMBER PRECEDING, SMALL FORMAT WAS ALREADY SET
                        f.BigFmt = format;
                        hasnumber = 5;
                        break;
                    }

                }
                else {
                    rplError(ERR_INVALIDNUMFORMAT);
                    return;
                }

            }
            --nitems;
            item = rplSkipOb(item);
        }

        rplSetSystemNumberFormat(&f);
        rplDropData(1);

        return;
    }

    case CF:
        //@SHORT_DESC=Clear a flag
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            int64_t flag = rplReadNumberAsInt64(rplPeekData(1));
            if(flag < 0) {
                switch (rplClrSystemFlag(flag)) {
                case -1:
                    rplError(ERR_INVALIDFLAGNUMBER);
                    return;
                case -2:
                    rplError(ERR_SYSTEMFLAGSINVALID);
                    return;
                default:
                    rplDropData(1);
                }
            }
            else {
                switch (rplClrUserFlag(flag)) {
                case -1:
                    rplError(ERR_INVALIDFLAGNUMBER);
                    return;
                case -2:
                    return;
                default:
                    rplDropData(1);
                }

            }
            return;
        }

        if(ISIDENT(*rplPeekData(1))) {
            // FLAG GIVEN BY NAME
            word_p id = rplPeekData(1);
            switch (rplClrSystemFlagByIdent(id)) {
            case -1:
                rplError(ERR_INVALIDFLAGNAME);
                return;
            case -2:
                rplError(ERR_SYSTEMFLAGSINVALID);
                return;
            default:
                rplDropData(1);
            }
            return;
        }

        rplError(ERR_IDENTORINTEGEREXPECTED);
        return;

    case SF:
        //@SHORT_DESC=Set a flag
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            int64_t flag = rplReadNumberAsInt64(rplPeekData(1));
            if(flag < 0) {
                switch (rplSetSystemFlag(flag)) {
                case -1:
                    rplError(ERR_INVALIDFLAGNUMBER);
                    return;
                case -2:
                    rplError(ERR_SYSTEMFLAGSINVALID);
                    return;
                default:
                    rplDropData(1);
                }
            }
            else {
                switch (rplSetUserFlag(flag)) {
                case -1:
                    rplError(ERR_INVALIDFLAGNUMBER);
                    return;
                case -2:
                    return;
                default:
                    rplDropData(1);
                }
            }
            return;
        }

        if(ISIDENT(*rplPeekData(1))) {
            // FLAG GIVEN BY NAME
            word_p id = rplPeekData(1);
            switch (rplSetSystemFlagByIdent(id)) {
            case -1:
                rplError(ERR_INVALIDFLAGNAME);
                return;
            case -2:
                rplError(ERR_SYSTEMFLAGSINVALID);
                return;
            default:
                rplDropData(1);
            }
            return;
        }

        rplError(ERR_IDENTORINTEGEREXPECTED);
        return;

    case FCTEST:
    {
        //@SHORT_DESC=Test if a flag is clear
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t test;
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            int64_t flag = rplReadNumberAsInt64(rplPeekData(1));
            if(flag < 0) {
                switch (test = rplTestSystemFlag(flag)) {
                case -1:
                    rplError(ERR_INVALIDFLAGNUMBER);
                    return;
                case -2:
                    rplError(ERR_SYSTEMFLAGSINVALID);
                    return;
                default:
                    rplDropData(1);
                }
            }
            else {
                switch (test = rplTestUserFlag(flag)) {
                case -1:
                    rplError(ERR_INVALIDFLAGNUMBER);
                    return;
                default:
                    rplDropData(1);
                }
            }
            if(test)
                rplPushData((word_p) zero_bint);
            else
                rplPushData((word_p) one_bint);
            return;
        }

        if(ISIDENT(*rplPeekData(1))) {
            // FLAG GIVEN BY NAME
            word_p id = rplPeekData(1);
            switch (test = rplTestSystemFlagByIdent(id)) {
            case -1:
                rplError(ERR_INVALIDFLAGNAME);
                return;
            case -2:
                rplError(ERR_SYSTEMFLAGSINVALID);
                return;
            default:
                rplDropData(1);
            }
            if(test)
                rplPushData((word_p) zero_bint);
            else
                rplPushData((word_p) one_bint);
            return;
        }

        rplError(ERR_IDENTORINTEGEREXPECTED);
        return;

    }
    case FSTEST:
    {
        //@SHORT_DESC=Test if a flag is set
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t test;
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            int64_t flag = rplReadNumberAsInt64(rplPeekData(1));
            if(flag < 0) {
                switch (test = rplTestSystemFlag(flag)) {
                case -1:
                    rplError(ERR_INVALIDFLAGNUMBER);
                    return;
                case -2:
                    rplError(ERR_SYSTEMFLAGSINVALID);
                    return;
                default:
                    rplDropData(1);
                }
            }
            else {
                switch (test = rplTestUserFlag(flag)) {
                case -1:
                    rplError(ERR_INVALIDFLAGNUMBER);
                    return;
                default:
                    rplDropData(1);
                }
            }

            if(test)
                rplPushData((word_p) one_bint);
            else
                rplPushData((word_p) zero_bint);
            return;
        }

        if(ISIDENT(*rplPeekData(1))) {
            // FLAG GIVEN BY NAME
            word_p id = rplPeekData(1);
            switch (test = rplTestSystemFlagByIdent(id)) {
            case -1:
                rplError(ERR_INVALIDFLAGNAME);
                return;
            case -2:
                rplError(ERR_SYSTEMFLAGSINVALID);
                return;
            default:
                rplDropData(1);
            }
            if(test)
                rplPushData((word_p) one_bint);
            else
                rplPushData((word_p) zero_bint);
            return;
        }

        rplError(ERR_IDENTORINTEGEREXPECTED);
        return;

    }

    case FCTESTCLEAR:
    {
        //@SHORT_DESC=Test if a flag is clear, then clear it
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t test;
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            int64_t flag = rplReadNumberAsInt64(rplPeekData(1));
            if(flag < 0) {
                switch (test = rplTestSystemFlag(flag)) {
                case -1:
                    rplError(ERR_INVALIDFLAGNUMBER);
                    return;
                case -2:
                    rplError(ERR_SYSTEMFLAGSINVALID);
                    return;
                default:
                    rplDropData(1);
                }
                rplClrSystemFlag(flag);
            }
            else {
                switch (test = rplTestUserFlag(flag)) {
                case -1:
                    rplError(ERR_INVALIDFLAGNUMBER);
                    return;
                default:
                    rplDropData(1);
                }
                rplClrUserFlag(flag);
            }

            if(test)
                rplPushData((word_p) zero_bint);
            else
                rplPushData((word_p) one_bint);
            return;
        }

        if(ISIDENT(*rplPeekData(1))) {
            // FLAG GIVEN BY NAME
            word_p id = rplPeekData(1);
            switch (test = rplTestSystemFlagByIdent(id)) {
            case -1:
                rplError(ERR_INVALIDFLAGNAME);
                return;
            case -2:
                rplError(ERR_SYSTEMFLAGSINVALID);
                return;
            default:
                rplDropData(1);
            }
            rplClrSystemFlagByIdent(id);
            if(test)
                rplPushData((word_p) zero_bint);
            else
                rplPushData((word_p) one_bint);
            return;
        }

        rplError(ERR_IDENTORINTEGEREXPECTED);
        return;

    }

    case FSTESTCLEAR:

    {
        //@SHORT_DESC=Test if a flag is set, then clear it
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t test;
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            int64_t flag = rplReadNumberAsInt64(rplPeekData(1));
            if(flag < 0) {
                switch (test = rplTestSystemFlag(flag)) {
                case -1:
                    rplError(ERR_INVALIDFLAGNUMBER);
                    return;
                case -2:
                    rplError(ERR_SYSTEMFLAGSINVALID);
                    return;
                default:
                    rplDropData(1);
                }
                rplClrSystemFlag(flag);
            }
            else {
                switch (test = rplTestUserFlag(flag)) {
                case -1:
                    rplError(ERR_INVALIDFLAGNUMBER);
                    return;
                default:
                    rplDropData(1);
                }
                rplClrUserFlag(flag);
            }

            if(test)
                rplPushData((word_p) one_bint);
            else
                rplPushData((word_p) zero_bint);
            return;
        }

        if(ISIDENT(*rplPeekData(1))) {
            // FLAG GIVEN BY NAME
            word_p id = rplPeekData(1);
            switch (test = rplTestSystemFlagByIdent(id)) {
            case -1:
                rplError(ERR_INVALIDFLAGNAME);
                return;
            case -2:
                rplError(ERR_SYSTEMFLAGSINVALID);
                return;
            default:
                rplDropData(1);
            }
            rplClrSystemFlagByIdent(id);
            if(test)
                rplPushData((word_p) one_bint);
            else
                rplPushData((word_p) zero_bint);
            return;
        }

        rplError(ERR_IDENTORINTEGEREXPECTED);
        return;

    }

    case TMENU:
    {
        //@SHORT_DESC=Display the given menu on the active menu area
        //@INCOMPAT
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t menu = rplGetActiveMenu();

        rplSaveMenuHistory(menu);
        rplChangeMenu(menu, rplPeekData(1));

        if(!Exceptions) {
            rplDropData(1);
        }

        return;
    }

    case TMENULST:
        //@SHORT_DESC=Display the given menu on the menu area the user used last
        //@NEW
    case TMENUOTHR:
    {
        //@SHORT_DESC=Display the given menu on the menu are the user did not use last
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        int32_t menu = rplGetLastMenu();
        if(CurOpcode == CMD_TMENUOTHR) {
            // USE THE OTHER MENU
            if(menu == 1)
                menu = 2;
            else
                menu = 1;
        }

        rplSaveMenuHistory(menu);
        rplChangeMenu(menu, rplPeekData(1));

        if(!Exceptions) {
            rplDropData(1);
        }

        return;
    }

    case RCLMENU:
        //@SHORT_DESC=Recall the active menu
        //@INCOMPAT
    case RCLMENULST:
        //@SHORT_DESC=Recall the menu the user used last
        //@NEW
    case RCLMENUOTHR:
    {
        //@SHORT_DESC=Recall the menu the user did not use last
        //@NEW
        int32_t menu;

        if(CurOpcode != CMD_RCLMENU) {
            menu = rplGetLastMenu();
            if(CurOpcode == CMD_RCLMENUOTHR) {
                // USE THE OTHER MENU
                if(menu == 1)
                    menu = 2;
                else
                    menu = 1;
            }
        }
        else
            menu = rplGetActiveMenu();

        int64_t mcode = rplGetMenuCode(menu);

        if((MENULIBRARY(mcode) == LIBRARY_NUMBER) && (MENUNUMBER(mcode) < 2)) {
            // SPECIAL CUSTOM MENUS, RCL FROM THE SETTINGS DIRECTORY
            word_p msetting;
            if(menu == 1)
                msetting = rplGetSettings((word_p) menu1_ident);
            else if(menu == 2)
                msetting = rplGetSettings((word_p) menu2_ident);
            else
                msetting = 0;

            if(!msetting)
                msetting = (word_p) empty_list;        // IF MENU CONTENT CAN'T BE DETERMINED, RETURN AN EMPTY CUSTOM MENU

            rplPushData(msetting);
            return;
        }

        // NOTHING CUSTOM, JUST RETURN THE MENU CODE

        rplNewint32_tPush((int64_t) mcode, HEXint32_t);
        return;

    }

    case MENUSWAP:
    {
        //@SHORT_DESC=Swap the contents of menu areas 1 and 2
        //@NEW
        // JUST SWAP MENUS 1 AND 2
        int64_t m1code = rplGetMenuCode(1);
        int64_t m2code = rplGetMenuCode(2);

        if((MENULIBRARY(m2code) == LIBRARY_NUMBER) && (MENUNUMBER(m2code) < 2))
            m2code = MKMENUCODE(0, LIBRARY_NUMBER, MENUNUMBER(m2code) ^ 1, MENUPAGE(m2code));   // ALTERNATE MENU'S 1 AND 2
        if((MENULIBRARY(m1code) == LIBRARY_NUMBER) && (MENUNUMBER(m1code) < 2))
            m1code = MKMENUCODE(0, LIBRARY_NUMBER, MENUNUMBER(m1code) ^ 1, MENUPAGE(m1code));   // ALTERNATE MENU'S 1 AND 2

        rplSetMenuCode(1, m2code);
        rplSetMenuCode(2, m1code);

        word_p m1setting, m2setting;

        m1setting = rplGetSettings((word_p) menu1_ident);
        m2setting = rplGetSettings((word_p) menu2_ident);

        if(m1setting)
            rplStoreSettings((word_p) menu2_ident, m1setting);
        if(m2setting)
            rplStoreSettings((word_p) menu1_ident, m2setting);

        m1setting = rplGetSettings((word_p) menu1hist_ident);
        m2setting = rplGetSettings((word_p) menu2hist_ident);

        if(m1setting)
            rplStoreSettings((word_p) menu2hist_ident, m1setting);
        if(m2setting)
            rplStoreSettings((word_p) menu1hist_ident, m2setting);

        return;
    }

    case MENUBK:
        //@SHORT_DESC=Display the previous menu on the active menu area
        //@NEW
    case MENUBKLST:
        //@SHORT_DESC=Display the previous menu on the area the user used last
        //@NEW
    case MENUBKOTHR:
    {
        //@SHORT_DESC=Display the previous menu on the area the user did not use last
        //@NEW
        int32_t menu;

        if(CurOpcode != CMD_MENUBK) {
            menu = rplGetLastMenu();
            if(CurOpcode == CMD_MENUBKOTHR) {
                // USE THE OTHER MENU
                if(menu == 1)
                    menu = 2;
                else
                    menu = 1;
            }
        }
        else
            menu = rplGetActiveMenu();

        word_p menuobj = rplPopMenuHistory(menu);

        if(menuobj)
            rplChangeMenu(menu, menuobj);

        return;
    }

    case DEG:
        //@SHORT_DESC=Set the angle mode flags to degrees
        rplClrSystemFlag(-17);
        rplClrSystemFlag(-18);
        return;
    case RAD:
        //@SHORT_DESC=Set the angle mode flags to radians
        rplSetSystemFlag(-17);
        rplClrSystemFlag(-18);
        return;
    case GRAD:
        //@SHORT_DESC=Set the angle mode flags to grads (gons)
        rplClrSystemFlag(-17);
        rplSetSystemFlag(-18);
        return;
    case DMS:
        //@SHORT_DESC=Set the angle mode to DMS (as DD.MMSS)
        rplSetSystemFlag(-17);
        rplSetSystemFlag(-18);
        return;

    case ASNKEY:
    {
        //@SHORT_DESC=Assign a custom definition to a key
        //@NEW
        // ASSIGN A CUSTOM KEY DEFINITION
        if(rplDepthData() < 3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(3);

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_POSITIVEINTEGEREXPECTED);
            return;
        }
        if(!ISSTRING(*rplPeekData(2))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        int32_t keycode = rplKeyName2Msg(rplPeekData(2));

        if(!keycode) {
            rplError(ERR_INVALIDKEYNAME);
            return;
        }

        int32_t context = rplReadNumberAsInt64(rplPeekData(1));
        if(context < 0) {
            rplError(ERR_POSITIVEINTEGEREXPECTED);
            return;
        }

        rplDropData(2);
        rplNewint32_tPush(keycode, HEXint32_t);
        rplNewint32_tPush(context, HEXint32_t);
        if(Exceptions)
            return;

        rplPushData(rplPeekData(3));    // STACK HAS NOW PROPER ORDER: { KEYCODE CONTEXT ACTION }

        word_p keylist = rplGetSettings((word_p) customkey_ident);

        if(!keylist) {
            // NEED TO CREATE A LIST FROM SCRATCH
            keylist = rplCreateListN(3, 1, 1);
            if(!keylist)
                return;
        }
        else {
            // ADD 3 MORE ITEMS TO THE EXISTING LIST
            int32_t n = rplExplodeList2(keylist);
            keylist = rplCreateListN(n + 3, 1, 1);
            if(Exceptions)
                return;
        }

        rplDropData(1);

        rplStoreSettings((word_p) customkey_ident, keylist);
        return;

    }

    case DELKEY:
    {
        //@SHORT_DESC=Remove a custom key definition
        //@NEW
        // REMOVE A CUSTOM KEY DEFINITION
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISSTRING(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        int32_t keycode = rplKeyName2Msg(rplPeekData(1));

        if(!keycode) {
            rplError(ERR_INVALIDKEYNAME);
            return;
        }

        rplDropData(1);

        word_p keylist = rplGetSettings((word_p) customkey_ident);

        if(!keylist)
            return;

        // SCAN THE LIST

        word_p ptr = keylist + 1;
        word_p endlist = rplSkipOb(keylist);

        while(ptr < endlist) {
            if(ISNUMBER(*ptr)) {
                int32_t code = rplReadNumberAsInt64(ptr);
                if(keycode == code)
                    break;
            }
            ptr = rplSkipOb(ptr);       // SKIP KEYCODE
            if(ptr >= endlist)
                break;
            ptr = rplSkipOb(ptr);       // SKIP CONTEXT
            if(ptr >= endlist)
                break;
            ptr = rplSkipOb(ptr);       // SKIP ACTION
        }

        // HERE WE HAVE THE PTR INTO THE LIST
        if(ptr < endlist) {
            // COMPUTE SIZE OF NEW LIST
            word_p endptr = rplSkipOb(ptr);
            if(endptr < endlist)
                endptr = rplSkipOb(endptr);
            if(endptr < endlist)
                endptr = rplSkipOb(endptr);

            // NOW MOVE THE MEMORY BETWEEN ptr AND endptr

            int32_t offstart = ptr - keylist, offend = endptr - keylist;

            rplPushData(keylist);       // PROTECT FROM GC
            word_p newlist =
                    rplAllocTempOb(OBJSIZE(*keylist) - (offend - offstart));
            if(!newlist)
                return;
            keylist = rplPopData();
            memmovew(newlist + 1, keylist + 1, offstart - 1);
            memmovew(newlist + offstart, keylist + offend,
                    OBJSIZE(*keylist) - offend + 1);
            // NOW WRITE A NEW PROLOG
            *newlist =
                    MKPROLOG(DOLIST, OBJSIZE(*keylist) - (offend - offstart));
            // AND STORE THE NEW LIST
            rplStoreSettings((word_p) customkey_ident, newlist);

        }

        return;

    }

    case RCLKEYS:
    {
        //@SHORT_DESC=Recall the list of all custom key definitions
        //@INCOMPAT
        // GET THE ENTIRE LIST OF KEY ASSIGNMENTS
        word_p keylist = rplGetSettings((word_p) customkey_ident);

        if(!keylist)
            keylist = (word_p) empty_list;

        rplPushData(keylist);
        return;
    }

    case STOKEYS:
    {
        //@SHORT_DESC=Store and replace all custom key definitions
        //@INCOMPAT
        // STORE THE ENTIRE LIST OF KEY ASSIGNMENTS
        // REMOVE A CUSTOM KEY DEFINITION
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(!ISLIST(*rplPeekData(1))) {
            rplError(ERR_LISTEXPECTED);
            return;
        }

        // VERIFY THAT THE LIST IS VALID
        // OTHERWISE IT CAN HAVE CATASTROPHIC CONSEQUENCES
        word_p ptr = rplPeekData(1) + 1;
        word_p endlist = rplSkipOb(ptr - 1);
        while(ptr < endlist) {
            if(*ptr == CMD_ENDLIST)
                break;
            if(!ISNUMBER(*ptr)) {
                rplError(ERR_INVALIDKEYDEFINITION);
                return;
            }
            ptr = rplSkipOb(ptr);
            if(*ptr == CMD_ENDLIST) {
                rplError(ERR_INVALIDKEYDEFINITION);
                return;
            }
            if(!ISNUMBER(*ptr)) {
                rplError(ERR_INVALIDKEYDEFINITION);
                return;
            }
            ptr = rplSkipOb(ptr);
            if(*ptr == CMD_ENDLIST) {
                rplError(ERR_INVALIDKEYDEFINITION);
                return;
            }
            ptr = rplSkipOb(ptr);
        }

        // HERE THE LIST IS VALID
        // AND STORE THE NEW LIST
        rplStoreSettings((word_p) customkey_ident, rplPeekData(1));
        rplDropData(1);
        return;

    }

    case TYPE:
    {
        //@SHORT_DESC=Get type information from an object
        //@INCOMPAT
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        int32_t istag = rplStripTagStack(1);

        LIBHANDLER han = rplGetLibHandler(LIBNUM(*rplPeekData(1)));

        // GET THE SYMBOLIC TOKEN INFORMATION
        if(han) {
            WORD savecurOpcode = CurOpcode;
            ObjectPTR = rplPeekData(1);
            CurOpcode = MKOPCODE(LIBNUM(*ObjectPTR), OPCODE_GETINFO);
            (*han) ();

            CurOpcode = savecurOpcode;

            if(RetNum > OK_TOKENINFO) {
                rplDropData(1);
                rplNewint32_tPush((TypeInfo / 100) + (istag ? 10000 : 0), DECint32_t);
                return;
            }
        }
        rplOverwriteData(1, (word_p) zero_bint);

        return;
    }

    case TYPEE:
    {
        //@SHORT_DESC=Get extended type information from an object
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        int32_t istag = rplStripTagStack(1);

        LIBHANDLER han = rplGetLibHandler(LIBNUM(*rplPeekData(1)));

        // GET THE SYMBOLIC TOKEN INFORMATION
        if(han) {
            WORD savecurOpcode = CurOpcode;
            ObjectPTR = rplPeekData(1);
            CurOpcode = MKOPCODE(LIBNUM(*ObjectPTR), OPCODE_GETINFO);
            (*han) ();

            CurOpcode = savecurOpcode;

            if(RetNum > OK_TOKENINFO) {
                rplDropData(1);
                if(TypeInfo % 100) {
                    newRealFromint64_t(&RReg[0],
                            TypeInfo + (istag ? 1000000 : 0), -2);

                    rplNewRealFromRRegPush(0);
                }
                else
                    rplNewint32_tPush(TypeInfo / 100 + (istag ? 10000 : 0),
                            DECint32_t);
                return;
            }
        }
        rplOverwriteData(1, (word_p) zero_bint);

        return;
    }

    case GETLOCALE:
    {
        //@SHORT_DESC=Get the current separator symbols
        //@NEW
        // GET LOCALE STRING

        uint64_t loc;

        loc = rplGetSystemLocale();

        // MAKE THE LOCALE STRING

        BYTE locbase[16];       // 4 BYTES PER UNICODE CHARACTER MAXIMUM
        byte_p locstr = locbase;

        WORD uchar;

        uchar = cp2utf8((int)DECIMAL_DOT(loc));
        while(uchar) {
            *locstr++ = uchar & 0xff;
            uchar >>= 8;
        }

        uchar = cp2utf8((int)THOUSAND_SEP(loc));
        while(uchar) {
            *locstr++ = uchar & 0xff;
            uchar >>= 8;
        }

        uchar = cp2utf8((int)FRAC_SEP(loc));
        while(uchar) {
            *locstr++ = uchar & 0xff;
            uchar >>= 8;
        }

        uchar = cp2utf8((int)ARG_SEP(loc));
        while(uchar) {
            *locstr++ = uchar & 0xff;
            uchar >>= 8;
        }

        word_p item = rplCreateString(locbase, locstr);
        if(!item)
            return;

        rplPushData(item);

        return;
    }
    case GETNFMT:
    {
        //@SHORT_DESC=Recall the current display format for numbers
        //@NEW
        // GET SYSTEM NUMBER FORMAT IN USER-READABLE FORMAT
        NUMFORMAT f;

        rplGetSystemNumberFormat(&f);

        // COPY REALS TO REGISTERS IN CASE OF GC
        copyReal(&RReg[0], &f.SmallLimit);
        copyReal(&RReg[1], &f.BigLimit);

        word_p *savestk = DSTop;

        word_p obj;

        obj = rplNumFormat2String(f.MiddleFmt);
        if(!obj) {
            DSTop = savestk;
            return;
        }
        rplPushData(obj);

        rplNewRealFromRRegPush(0);
        if(Exceptions) {
            DSTop = savestk;
            return;
        }
        obj = rplNumFormat2String(f.SmallFmt);
        if(!obj) {
            DSTop = savestk;
            return;
        }
        rplPushData(obj);

        rplNewRealFromRRegPush(1);
        if(Exceptions) {
            DSTop = savestk;
            return;
        }
        obj = rplNumFormat2String(f.BigFmt);
        if(!obj) {
            DSTop = savestk;
            return;
        }
        rplPushData(obj);

        obj = rplCreateListN(5, 1, 1);
        if(!obj) {
            DSTop = savestk;
            return;
        }

        rplPushData(obj);
        return;

    }

    case RCLF:
    {
        //@SHORT_DESC=Recall all system flags
        //@INCOMPAT
        if(!ISBINDATA(*SystemFlags)) {
            rplError(ERR_SYSTEMFLAGSINVALID);
            return;
        }
        // MAKE A NON-SELF-MODIFYING COPY OF THE SYSTEM FLAGS
        word_p newflags = rplMakeNewCopy(SystemFlags);
        if(!newflags)
            return;
        rplPushData(newflags);
        return;
    }

    case STOF:
    {
        //@SHORT_DESC=Store and replace all system flags
        //@INCOMPAT
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(ISLIST(*rplPeekData(1))) {
            // CONVERT FLAGS STORED AS THE OLD LIST FORMAT TO THE NEW BINDATA FORMAT

            if(!ISBINDATA(*SystemFlags)) {
                // TRY TO RECOVER INSTEAD OF ERROR
                rplResetSystemFlags();
                if(!SystemFlags) {
                    rplError(ERR_SYSTEMFLAGSINVALID);
                    return;
                }
                rplStoreSettings((word_p) flags_ident, SystemFlags);
            }

            int32_t nitems = rplListLength(rplPeekData(1));
            if(nitems < 4) {
                rplError(ERR_SYSTEMFLAGSINVALID);
                return;
            }

            // ALL int32_tS
            int32_t k;
            for(k = 1; k <= 4; ++k) {
                if(!ISint32_t(*rplGetListElement(rplPeekData(1), k))) {
                    rplError(ERR_SYSTEMFLAGSINVALID);
                    return;
                }
            }

            // IT ALL CHECKS OUT, DO THE MAGIC:

            uint64_t value;
            word_p nptr = SystemFlags + 1;     // DATA OF THE FIRST 64-BIT INTEGER
            uint64_t *uptr;
            for(k = 1; k <= 4; ++k) {
                value = rplReadint32_t(rplGetListElement(rplPeekData(1), k));
                uptr = (uint64_t *) nptr;
                *uptr = value;
                nptr += 2;
            }
            rplDropData(1);
            return;
        }

        if(ISBINDATA(*rplPeekData(1)) && (OBJSIZE(*rplPeekData(1)) >= 8)) {
            if(!ISBINDATA(*SystemFlags)) {
                // TRY TO RECOVER INSTEAD OF ERROR
                rplResetSystemFlags();
                if(!SystemFlags) {
                    rplError(ERR_SYSTEMFLAGSINVALID);
                    return;
                }
                rplStoreSettings((word_p) flags_ident, SystemFlags);
            }

            // WE RECEIVED A PROPER BINDATA OBJECT, JUST COPY IT OVER
            memcpyw(SystemFlags + 1, rplPeekData(1) + 1, 8);
            rplDropData(1);
            return;

        }

        rplError(ERR_SYSTEMFLAGSINVALID);
        return;

    }

    case VTYPE:
    {
        //@SHORT_DESC=Get type information on the contents of a variable
        //@INCOMPAT
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryDoCmd();
            return;
        }

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        // FIND THE VARIABLE
        word_p *var = rplFindLAM(rplPeekData(1), 1);
        if(!var) {
            // NO LAM, TRY A GLOBAL
            var = rplFindGlobal(rplPeekData(1), 1);
            if(!var) {
                rplError(ERR_UNDEFINEDVARIABLE);
                return;
            }
        }

        word_p obj = rplStripTag(var[1]);
        LIBHANDLER han = rplGetLibHandler(LIBNUM(*obj));

        // GET THE SYMBOLIC TOKEN INFORMATION
        if(han) {
            WORD savecurOpcode = CurOpcode;
            int32_t istag = ISTAG(*var[1]);
            ObjectPTR = obj;
            CurOpcode = MKOPCODE(LIBNUM(*ObjectPTR), OPCODE_GETINFO);
            (*han) ();

            CurOpcode = savecurOpcode;

            if(RetNum > OK_TOKENINFO) {
                rplDropData(1);
                rplNewint32_tPush(TypeInfo / 100 + (istag ? 10000 : 0), DECint32_t);
                return;
            }
        }
        rplOverwriteData(1, (word_p) zero_bint);

        return;
    }

    case VTYPEE:
    {
        //@SHORT_DESC=Get extended type information on the contents of a variable
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);

        if(ISLIST(*rplPeekData(1))) {
            rplListUnaryDoCmd();
            return;
        }

        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }

        // FIND THE VARIABLE
        word_p *var = rplFindLAM(rplPeekData(1), 1);
        if(!var) {
            // NO LAM, TRY A GLOBAL
            var = rplFindGlobal(rplPeekData(1), 1);
            if(!var) {
                rplError(ERR_UNDEFINEDVARIABLE);
                return;
            }
        }

        word_p obj = rplStripTag(var[1]);
        LIBHANDLER han = rplGetLibHandler(LIBNUM(*obj));

        // GET THE SYMBOLIC TOKEN INFORMATION
        if(han) {
            WORD savecurOpcode = CurOpcode;
            int32_t istag = ISTAG(*var[1]);
            ObjectPTR = obj;
            CurOpcode = MKOPCODE(LIBNUM(*ObjectPTR), OPCODE_GETINFO);
            (*han) ();

            CurOpcode = savecurOpcode;

            if(RetNum > OK_TOKENINFO) {
                rplDropData(1);
                if(TypeInfo % 100) {
                    newRealFromint64_t(&RReg[0],
                            TypeInfo + (istag ? 1000000 : 0), -2);

                    rplNewRealFromRRegPush(0);
                }
                else
                    rplNewint32_tPush(TypeInfo / 100 + (istag ? 10000 : 0),
                            DECint32_t);
                return;
            }
        }
        rplOverwriteData(1, (word_p) zero_bint);

        return;
    }

    case FMTSTR:
    {
        //@SHORT_DESC=Do →STR using a specific numeric format
        //@NEW




    }
        // ADD MORE OPCODES HERE

        // STANDARIZED OPCODES:
        // --------------------
        // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES

    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors

        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES
        libCompileCmds(LIBRARY_NUMBER, (char **)LIB_NAMES, NULL,
                LIB_NUMBEROFCMDS);

        return;
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to WORD of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors

        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds((char **)LIB_NAMES, NULL, LIB_NUMBEROFCMDS);
        return;
    case OPCODE_VALIDATE:
        // VALIDATE RECEIVES OPCODES COMPILED BY OTHER LIBRARIES, TO BE INCLUDED WITHIN A COMPOSITE OWNED BY
        // THIS LIBRARY. EVERY COMPOSITE HAS TO EVALUATE IF THE OBJECT BEING COMPILED IS ALLOWED INSIDE THIS
        // COMPOSITE OR NOT. FOR EXAMPLE, A REAL MATRIX SHOULD ONLY ALLOW REAL NUMBERS INSIDE, ANY OTHER
        // OPCODES SHOULD BE REJECTED AND AN ERROR THROWN.
        // Library receives:
        // CurrentConstruct = SET TO THE CURRENT ACTIVE CONSTRUCT TYPE
        // LastCompiledObject = POINTER TO THE LAST OBJECT THAT WAS COMPILED, THAT NEEDS TO BE VERIFIED

        // VALIDATE RETURNS:
        // RetNum =  OK_CONTINUE IF THE OBJECT IS ACCEPTED, ERR_INVALID IF NOT.

        RetNum = OK_CONTINUE;
        return;

    case OPCODE_PROBETOKEN:
        // PROBETOKEN FINDS A VALID WORD AT THE BEGINNING OF THE GIVEN TOKEN AND RETURNS
        // INFORMATION ABOUT IT. THIS OPCODE IS MANDATORY

        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND
    {
        libProbeCmds((char **)LIB_NAMES, (int32_t *) LIB_TOKENINFO,
                LIB_NUMBEROFCMDS);

        return;
    }

    case OPCODE_GETINFO:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL COMMAND OR OBJECT IN ObjectPTR
        // NEEDS TO RETURN INFORMATION ABOUT THE TYPE:
        // IN RetNum: RETURN THE MKTOKENINFO() DATA FOR THE SYMBOLIC COMPILER AND CAS
        // IN DecompHints: RETURN SOME HINTS FOR THE DECOMPILER TO DO CODE BEAUTIFICATION (TO BE DETERMINED)
        // IN TypeInfo: RETURN TYPE INFORMATION FOR THE TYPE COMMAND
        //             TypeInfo: TTTTFF WHERE TTTT = MAIN TYPE * 100 (NORMALLY THE MAIN LIBRARY NUMBER)
        //                                FF = 2 DECIMAL DIGITS FOR THE SUBTYPE OR FLAGS (VARIES DEPENDING ON LIBRARY)
        //             THE TYPE COMMAND WILL RETURN A REAL NUMBER TypeInfo/100
        // FOR NUMBERS: TYPE=10 (REALS), SUBTYPES = .01 = APPROX., .02 = INTEGER, .03 = APPROX. INTEGER
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL int32_t, .42 = HEX INTEGER

        if(ISPROLOG(*ObjectPTR)) {
            TypeInfo = LIBRARY_NUMBER * 100;
            DecompHints = 0;
            RetNum = OK_TOKENINFO | MKTOKENINFO(0, TITYPE_NOTALLOWED, 0, 1);
        }
        else {
            TypeInfo = 0;       // ALL COMMANDS ARE TYPE 0
            DecompHints = 0;
            libGetInfo2(*ObjectPTR, (char **)LIB_NAMES, (int32_t *) LIB_TOKENINFO,
                    LIB_NUMBEROFCMDS);
        }
        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, ObjectIDHash=hash, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER, (word_p *) ROMPTR_TABLE, ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID, ObjectIDHash=hash
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((word_p *) ROMPTR_TABLE, ObjectID, ObjectIDHash);
        return;
    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) {
            RetNum = ERR_INVALID;
            return;
        }

        RetNum = OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER, (char **)LIB_NAMES,
                LIB_NUMBEROFCMDS);
        return;

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        word_p menuobj;
        switch (MENUNUMBER(MenuCodeArg)) {
        case 0:
            menuobj = rplGetSettings((word_p) menu1_ident);
            break;
        case 1:
            menuobj = rplGetSettings((word_p) menu2_ident);
            break;

        default:
            if((MENUNUMBER(MenuCodeArg) <= 12) && (MENUNUMBER(MenuCodeArg) > 1))
                menuobj = ROMPTR_TABLE[MENUNUMBER(MenuCodeArg)];
            else
                menuobj = 0;
        }
        if(!menuobj)
            ObjectPTR = (word_p) empty_list;
        else
            ObjectPTR = menuobj;

        if(ISIDENT(*ObjectPTR)) {
            // RCL THE VARIABLE

            word_p *var = rplFindGlobal(ObjectPTR, 1);
            if(!var)
                ObjectPTR = (word_p) empty_list;
            else
                ObjectPTR = var[1];
        }

        RetNum = OK_CONTINUE;
        return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp, (word_p) LIB_HELPTABLE);
        return;
    }

    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {

        libFindMsg(LibError, (word_p) LIB_MSGTABLE);
        return;
    }

    case OPCODE_LIBINSTALL:
        LibraryList = (word_p) libnumberlist;
        RetNum = OK_CONTINUE;
        return;
    case OPCODE_LIBREMOVE:
        return;

    }
    // UNHANDLED OPCODE...

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode) >= MIN_RESERVED_OPCODE) {
        RetNum = ERR_NOTMINE;
        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    rplError(ERR_INVALIDOPCODE);

    return;

}

#endif
