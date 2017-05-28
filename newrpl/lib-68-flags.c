/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************



// REPLACE THE NUMBER
#define LIBRARY_NUMBER  68

#define ERROR_LIST \
    ERR(SYSTEMFLAGSINVALID,0), \
    ERR(INVALIDFLAGNUMBER,1), \
    ERR(INVALIDFLAGNAME,2), \
    ERR(IDENTORINTEGEREXPECTED,3), \
    ERR(INVALIDLOCALESTRING,4), \
    ERR(INVALIDMENUDEFINITION,5), \
    ERR(INVALIDKEYNAME,6), \
    ERR(INVALIDKEYDEFINITION,7)

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(SETLOCALE,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SETNUMFORMAT,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SF,MKTOKENINFO(2,TITYPE_NOTALLOWED,1,2)), \
    CMD(CF,MKTOKENINFO(2,TITYPE_NOTALLOWED,1,2)), \
    ECMD(FCTEST,"FC?",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD(FSTEST,"FS?",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
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
    CMD(TYPE,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(TYPEE,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2))




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



ROMOBJECT dotsettings_ident[]= {
        MKPROLOG(DOIDENTHIDDEN,3),
        TEXT2WORD('.','S','e','t'),
        TEXT2WORD('t','i','n','g'),
        TEXT2WORD('s',0,0,0)
};
ROMOBJECT flags_ident[]= {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('F','l','a','g'),
        TEXT2WORD('s',0,0,0)
};
ROMOBJECT userflags_ident[]= {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('U','F','l','a'),
        TEXT2WORD('g','s',0,0)
};

ROMOBJECT locale_ident[]= {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('L','o','c','a'),
        TEXT2WORD('l','e',0,0)
};

ROMOBJECT numfmt_ident[]= {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('N','u','m','F'),
        TEXT2WORD('m','t',0,0)

};

ROMOBJECT menu1_ident[]= {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('M','e','n','u'),
        TEXT2WORD('1',0,0,0)
};

ROMOBJECT menu1hist_ident[]= {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('M','e','n','u'),
        TEXT2WORD('1','H','s','t')
};

ROMOBJECT menu2_ident[]= {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('M','e','n','u'),
        TEXT2WORD('2',0,0,0)
};

ROMOBJECT menu2hist_ident[]= {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('M','e','n','u'),
        TEXT2WORD('2','H','s','t')
};

ROMOBJECT menuhistory_ident[]= {
        MKPROLOG(DOIDENT,3),
        TEXT2WORD('M','e','n','u'),
        TEXT2WORD('H','L','e','v'),
        TEXT2WORD('e','l','s',0),

};

ROMOBJECT savedcmdline_ident[]= {
        MKPROLOG(DOIDENT,3),
        TEXT2WORD('S','a','v','e'),
        TEXT2WORD('d','C','m','d'),
        TEXT2WORD('L','i','n','e'),

};

ROMOBJECT customkey_ident[]= {
    MKPROLOG(DOIDENT,2),
    TEXT2WORD('.','C','s','t'),
    TEXT2WORD('K','e','y','s')
};


ROMOBJECT savedflags_ident[]= {
    MKPROLOG(DOIDENT,2),
    TEXT2WORD('.','H','W','F'),
    TEXT2WORD('l','a','g','s')
};

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)LIB_MSGTABLE,
    (WORDPTR)LIB_HELPTABLE,

    (WORDPTR)sysmenu_2_main,
    (WORDPTR)sysmenu_3_prog,
    (WORDPTR)sysmenu_4_math,
    (WORDPTR)sysmenu_5_symb,
    (WORDPTR)sysmenu_6_system,
    (WORDPTR)sysmenu_7_flags,
    (WORDPTR)sysmenu_8_menu,
    (WORDPTR)sysmenu_9_clipboard,
    (WORDPTR)sysmenu_10_settings,
    (WORDPTR)sysmenu_11_namedflags,
    (WORDPTR)sysmenu_12_keys,

    (WORDPTR)dotsettings_ident,
    (WORDPTR)flags_ident,
    (WORDPTR)locale_ident,
    (WORDPTR)numfmt_ident,
    (WORDPTR)menu1_ident,
    (WORDPTR)menu2_ident,
    (WORDPTR)menu1hist_ident,
    (WORDPTR)menu2hist_ident,
    (WORDPTR)menuhistory_ident,
    (WORDPTR)savedcmdline_ident,
    (WORDPTR)customkey_ident,
    (WORDPTR)savedflags_ident,
    (WORDPTR)userflags_ident,
    0
};



typedef struct {
    const char *flagname;
    unsigned char flags[8];
} systemflag;


const systemflag const flags_names[]= {
    // EACH OF THE 8 CHARS CONTAINS: BITS 0-6:= FLAG NUMBER (1-127),
    // BIT 7= VALUE (ON/OFF) TO USE WITH SET FLAG, ASSUMED ALWAYS 0 FOR CLEAR FLAG.

    { "DEG", { (-FL_ANGLEMODE1),(-FL_ANGLEMODE2),0,0,0,0,0,0}  },
    { "RAD", {0x80|(-FL_ANGLEMODE1),(-FL_ANGLEMODE2),0,0,0,0,0,0} },
    { "GRAD", {(-FL_ANGLEMODE1), 0X80|(-FL_ANGLEMODE2),0,0,0,0,0,0} },
    { "DMS", {0x80|(-FL_ANGLEMODE1),0x80|(-FL_ANGLEMODE2),0,0,0,0,0,0} },
    { "COMMENTS", {0x80|(-FL_STRIPCOMMENTS),0,0,0,0,0,0,0} },
    { "ACTIVEMENU1", { (-FL_ACTIVEMENU),0,0,0,0,0,0,0} },
    { "ACTIVEMENU2", { 0X80|(-FL_ACTIVEMENU), 0,0,0,0,0,0,0} },
    { "DATEDMY" , { 0X80|(-FL_DATEFORMAT),0,0,0,0,0,0,0} },
    { "DATEMDY" , { (-FL_DATEFORMAT),0,0,0,0,0,0,0} },
    { "TIME12" , { (-FL_TIMEFORMAT),0,0,0,0,0,0,0} },
    { "TIME24" , { 0x80|(-FL_TIMEFORMAT),0,0,0,0,0,0,0} },
    { "BEEPON" , { (-FL_ERRORBEEP),0,0,0,0,0,0,0} },
    { "BEEPOFF" , { 0x80|(-FL_ERRORBEEP),0,0,0,0,0,0,0} },
    { "ALMBEEPON" , { (-FL_ALARMBEEP),0,0,0,0,0,0,0} },
    { "ALMBEEPOFF" , { 0x80|(-FL_ALARMBEEP),0,0,0,0,0,0,0} },
    { "SAVEALM" , { 0x80|(-FL_SAVACKALRM),0,0,0,0,0,0,0} },
    { "RESCALM" , { (-FL_RESRPTALRM),0,0,0,0,0,0,0} },
    { "AUTOINDENT" , { (-FL_AUTOINDENT),0,0,0,0,0,0,0} },
    { "NOINDENT" , { 0x80|(-FL_AUTOINDENT),0,0,0,0,0,0,0} },

// TODO: ADD MORE FLAG NAMES HERE
    { NULL , {0,0,0,0,0,0,0,0} }
};



BINT rplSetUserFlag(BINT flag)
{
    if(flag<1 || flag>128) return -1;

    WORDPTR UserFlags=rplGetSettings((WORDPTR)userflags_ident);
    UBINT64 low64,hi64;

    if(!UserFlags) low64=hi64=0;
    else {
    if(!ISLIST(*UserFlags)) low64=hi64=0;
    else {
    low64=*((UBINT64 *)(UserFlags+2));
    hi64=*((UBINT64 *)(UserFlags+5));
    }
    }

    if(flag<65) low64|=(1ULL << (flag-1));
    else hi64|=(1ULL << (flag-65));

    // UNLIKE SYSTEM FLAGS, THESE ARE NOT SELF-MODIFYING OBJECTS


    UserFlags=rplAllocTempOb(7);
    if(!UserFlags) return -2;
    UserFlags[0]=MKPROLOG(DOLIST,7);
    UserFlags[1]=MKPROLOG(HEXBINT,2);
    UserFlags[2]=(WORD)low64;
    UserFlags[3]=(WORD)(low64>>32);
    UserFlags[4]=MKPROLOG(HEXBINT,2);
    UserFlags[5]=(WORD)hi64;
    UserFlags[6]=(WORD)(hi64>>32);
    UserFlags[7]=CMD_ENDLIST;

    rplStoreSettings((WORDPTR)userflags_ident,UserFlags);

    return 0;
}


BINT rplClrUserFlag(BINT flag)
{
    if(flag<1 || flag>128) return -1;

    WORDPTR UserFlags=rplGetSettings((WORDPTR)userflags_ident);
    UBINT64 low64,hi64;

    if(!UserFlags) low64=hi64=0;
    else {
    if(!ISLIST(*UserFlags)) low64=hi64=0;
    else {
    low64=*((UBINT64 *)(UserFlags+2));
    hi64=*((UBINT64 *)(UserFlags+5));
    }
    }

    if(flag<65) low64&=~(1ULL << (flag-1));
    else hi64&=~(1ULL << (flag-65));

    // UNLIKE SYSTEM FLAGS, THESE ARE NOT SELF-MODIFYING OBJECTS


    UserFlags=rplAllocTempOb(7);
    if(!UserFlags) return -2;
    UserFlags[0]=MKPROLOG(DOLIST,7);
    UserFlags[1]=MKPROLOG(HEXBINT,2);
    UserFlags[2]=(WORD)low64;
    UserFlags[3]=(WORD)(low64>>32);
    UserFlags[4]=MKPROLOG(HEXBINT,2);
    UserFlags[5]=(WORD)hi64;
    UserFlags[6]=(WORD)(hi64>>32);
    UserFlags[7]=CMD_ENDLIST;

    rplStoreSettings((WORDPTR)userflags_ident,UserFlags);

    return 0;
}

// RETURNS 1 IF FLAG IS SET, 0 OTHERWISE
// RETURN -1 IF THE NUMBER IS NOT VALID
// RETURN  0 IF SYSTEM FLAGS ARE CORRUPTED, INVALID OR NONEXISTENT

BINT rplTestUserFlag(BINT flag)
{
    if(flag<1 || flag>128) return -1;

    WORDPTR UserFlags=rplGetSettings((WORDPTR)userflags_ident);
    UBINT64 low64,hi64;

    if(!UserFlags) low64=hi64=0;
    else {
    if(!ISLIST(*UserFlags)) low64=hi64=0;
    else {
    low64=*((UBINT64 *)(UserFlags+2));
    hi64=*((UBINT64 *)(UserFlags+5));
    }
    }

    if(flag<65) return (low64&(1ULL << (flag-1)))? 1:0;
    else return (hi64&(1ULL << (flag-65)))? 1:0;
}


UBINT64 *rplGetUserFlagsLow()
{
    WORDPTR UserFlags=rplGetSettings((WORDPTR)userflags_ident);
    if(!UserFlags) return NULL;
    if(!ISLIST(*UserFlags)) return NULL;
    return (UBINT64 *)(UserFlags+2);
}


BINT rplSetSystemFlag(BINT flag)
{
    if(flag>-1 || flag<-128) return -1;
    if(!ISLIST(*SystemFlags)) return -2;

    WORDPTR low64=SystemFlags+2;
    WORDPTR hi64=SystemFlags+5;
    if(flag>=-32) low64[0]|=(1 << -(flag+1));
    else if(flag>=-64) low64[1]|=(1 << -(flag+33));
    else if(flag>=96) hi64[0]|=(1 << -(flag+65));
    else hi64[1]|=(1 << -(flag+97));

    return 0;
}




BINT rplClrSystemFlag(BINT flag)
{
    if(flag>-1 || flag<-128) return -1;
    if(!ISLIST(*SystemFlags)) return -2;

    WORDPTR low64=SystemFlags+2;
    WORDPTR hi64=SystemFlags+5;
    if(flag>=-32) low64[0]&=~(1 << -(flag+1));
    else if(flag>=-64) low64[1]&=~(1 << -(flag+33));
    else if(flag>=96) hi64[0]&=~(1 << -(flag+65));
    else hi64[1]&=~(1 << -(flag+97));

    return 0;
}

BINT rplSetSystemFlagByName(BYTEPTR name, BYTEPTR nameend)
{
    if(!ISLIST(*SystemFlags)) return -2;

    BINT idx=0;
    BINT len=utf8nlen((char *)name,(char *)nameend);
    BINT flaglen;

    while(flags_names[idx].flagname) {
        flaglen=utf8len((char *)flags_names[idx].flagname);
        if((flaglen==len) && !utf8ncmp((char *)name,flags_names[idx].flagname,len))
        {
            BINT count;
            for(count=0;count<8;++count)
            {
                if(flags_names[idx].flags[count]) {
                    BINT flag=flags_names[idx].flags[count]&0x7f;
                    BINT value=flags_names[idx].flags[count]>>7;
                    //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
                    WORDPTR low64=SystemFlags+2;
                    WORDPTR hi64=SystemFlags+5;
                    if(value) {
                        if(flag<=32) low64[0]|=(1 << (flag-1));
                        else if(flag<=64) low64[1]|=(1 << (flag-33));
                        else if(flag<=96) hi64[0]|=(1 << (flag-65));
                        else hi64[1]|=(1 << (flag-97));
                    } else {
                        if(flag<=32) low64[0]&=~(1 << (flag-1));
                        else if(flag<=64) low64[1]&=~(1 << (flag-33));
                        else if(flag<=96) hi64[0]&=~(1 << (flag-65));
                        else hi64[1]&=~(1 << (flag-97));
                    }

                }
            }

            return 0;

        }
        ++idx;
    }
 return -1;
}

BINT rplClrSystemFlagByName(BYTEPTR name,BYTEPTR nameend)
{
    if(!ISLIST(*SystemFlags)) return -2;

    BINT idx=0;
    BINT len=utf8nlen((char *)name,(char *)nameend);
    BINT flaglen;

    while(flags_names[idx].flagname) {
        flaglen=utf8len((char *)flags_names[idx].flagname);
        if((flaglen==len) && !utf8ncmp((char *)name,flags_names[idx].flagname,len))
        {
            BINT count;
            for(count=0;count<8;++count)
            {
                if(flags_names[idx].flags[count]) {
                    BINT flag=flags_names[idx].flags[count]&0x7f;
                    //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
                    WORDPTR low64=SystemFlags+2;
                    WORDPTR hi64=SystemFlags+5;
                    if(flag<=32) low64[0]&=~(1 << (flag-1));
                    else if(flag<=64) low64[1]&=~(1 << (flag-33));
                    else if(flag<=96) hi64[0]&=~(1 << (flag-65));
                    else hi64[1]&=~(1 << (flag-97));

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

BINT rplTestSystemFlag(BINT flag)
{
    if(flag>-1 || flag<-128) return -1;
    if(!ISLIST(*SystemFlags)) return -2;

        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
        WORDPTR low64=SystemFlags+2;
        WORDPTR hi64=SystemFlags+5;
        BINT result;
        if(flag>=-32) result=low64[0]&(1 << -(flag+1));
        else if(flag>=-64) result=low64[1]&(1 << -(flag+33));
        else if(flag>=96) result=hi64[0]&(1 << -(flag+65));
        else result=hi64[1]&(1 << -(flag+97));
        if(result) return 1;
        return 0;

}


// RETURN 0/1 IF THE MULTIPLE FLAGS MATCH THE SETTINGS
// RETURN -1 IF THE NAME IS NOT VALID
// RETURN -2 IF SYSTEM FLAGS ARE CORRUPTED OR INVALID

BINT rplTestSystemFlagByName(BYTEPTR name,BYTEPTR nameend)
{
    if(!ISLIST(*SystemFlags)) return -2;
    BINT idx=0;
    BINT len=utf8nlen((char *)name,(char *)nameend);
    BINT flaglen;
    while(flags_names[idx].flagname) {
        flaglen=utf8len((char *)flags_names[idx].flagname);
        if((flaglen==len) && !utf8ncmp((char *)name,flags_names[idx].flagname,len))
        {
            BINT count;
            BINT match=0;
            for(count=0;count<8;++count)
            {
                if(flags_names[idx].flags[count]) {
                    BINT flag=flags_names[idx].flags[count]&0x7f;
                    BINT value=flags_names[idx].flags[count]>>7;

                    WORDPTR low64=SystemFlags+2;
                    WORDPTR hi64=SystemFlags+5;
                    BINT res;
                    if(flag<=32) res=(low64[0]>> (flag-1))&1;
                    else if(flag<=64) res=(low64[1]>>(flag-33))&1;
                    else if(flag<=96) res=(hi64[0]>>(flag-65))&1;
                    else res=(hi64[1]>> (flag-97))&1;
                    match|=(value^res);
                }
            }
            if(!match) return 1; // MATCH LOGIC IS REVERSED
            else return 0;

        }
        ++idx;
    }
    return -1;

}

BINT rplSetSystemFlagByIdent(WORDPTR ident)
{
    BYTEPTR text=(BYTEPTR)(ident+1);
    return rplSetSystemFlagByName(text,text+rplGetIdentLength(ident));
}

BINT rplClrSystemFlagByIdent(WORDPTR ident)
{
    BYTEPTR text=(BYTEPTR)(ident+1);
    return rplClrSystemFlagByName(text,text+rplGetIdentLength(ident));
}

BINT rplTestSystemFlagByIdent(WORDPTR ident)
{
    BYTEPTR text=(BYTEPTR)(ident+1);
    return rplTestSystemFlagByName(text,text+rplGetIdentLength(ident));
}


// RETURN THE SYSTEM LOCALE WORD, CONTAINING THE CHARACTERS TO BE USED FOR NUMBERS
UBINT64 rplGetSystemLocale()
{
    WORDPTR systemlist=rplGetSettings((WORDPTR)numfmt_ident);
    if(systemlist) {
        if(ISLIST(*systemlist)) {
        WORDPTR localestring=rplGetListElement(systemlist,1);
        // EXPAND THE STRING INTO FOUR UNICODE CODEPOINTS
        if(localestring && (ISSTRING(*localestring))) {
            UBINT64 result;
            BYTEPTR locptr=(BYTEPTR)(localestring+1),locend=(BYTEPTR)rplSkipOb(localestring);
            result=utf82cp((char *)locptr,(char *)locend);
            locptr=(BYTEPTR)utf8skip((char *)locptr,(char *)locend);
            result|=((UBINT64)(utf82cp((char *)locptr,(char *)locend)&0xffff))<<16;
            locptr=(BYTEPTR)utf8skip((char *)locptr,(char *)locend);
            result|=((UBINT64)(utf82cp((char *)locptr,(char *)locend)&0xffff))<<32;
            locptr=(BYTEPTR)utf8skip((char *)locptr,(char *)locend);
            result|=((UBINT64)(utf82cp((char *)locptr,(char *)locend)&0xffff))<<48;
            locptr=(BYTEPTR)utf8skip((char *)locptr,(char *)locend);

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

void rplGetSystemNumberFormat(NUMFORMAT *fmt)
{
    WORDPTR systemlist=rplGetSettings((WORDPTR)numfmt_ident);
    if(systemlist) {
        if(ISLIST(*systemlist)) {
        WORDPTR localestring=rplGetListElement(systemlist,1);
        if(localestring && (ISSTRING(*localestring))) {
            UBINT64 result;
            BYTEPTR locptr=(BYTEPTR)(localestring+1),locend=(BYTEPTR)rplSkipOb(localestring);
            result=utf82cp((char *)locptr,(char *)locend);
            locptr=(BYTEPTR)utf8skip((char *)locptr,(char *)locend);
            result|=((UBINT64)(utf82cp((char *)locptr,(char *)locend)&0xffff))<<16;
            locptr=(BYTEPTR)utf8skip((char *)locptr,(char *)locend);
            result|=((UBINT64)(utf82cp((char *)locptr,(char *)locend)&0xffff))<<32;
            locptr=(BYTEPTR)utf8skip((char *)locptr,(char *)locend);
            result|=((UBINT64)(utf82cp((char *)locptr,(char *)locend)&0xffff))<<48;
            locptr=(BYTEPTR)utf8skip((char *)locptr,(char *)locend);

            fmt->Locale=result;
        }
        else fmt->Locale=SYSTEM_DEFAULT_LOCALE;
        WORDPTR nfmt=rplGetListElement(systemlist,2);
        if(nfmt && (ISBINT(*nfmt))) fmt->SmallFmt=(BINT)rplReadBINT(nfmt);
        else fmt->SmallFmt=12|FMT_SCI|FMT_SUPRESSEXP|FMT_USECAPITALS;
        nfmt=rplGetListElement(systemlist,3);
        if(nfmt && (ISBINT(*nfmt))) fmt->MiddleFmt=(BINT)rplReadBINT(nfmt);
        else fmt->MiddleFmt=12|FMT_USECAPITALS;
        nfmt=rplGetListElement(systemlist,4);
        if(nfmt && (ISBINT(*nfmt))) fmt->BigFmt=(BINT)rplReadBINT(nfmt);
        else fmt->BigFmt=12|FMT_SCI|FMT_SUPRESSEXP|FMT_USECAPITALS;
        nfmt=rplGetListElement(systemlist,5);
        if(nfmt && (ISNUMBER(*nfmt))) rplReadNumberAsReal(nfmt,&(fmt->SmallLimit));
        else {
            rplReadNumberAsReal((WORDPTR)one_bint,&(fmt->SmallLimit));
            fmt->SmallLimit.exp=-12;
        }
        nfmt=rplGetListElement(systemlist,6);
        if(nfmt && (ISNUMBER(*nfmt))) rplReadNumberAsReal(nfmt,&(fmt->BigLimit));
        else {
            rplReadNumberAsReal((WORDPTR)one_bint,&(fmt->BigLimit));
            fmt->BigLimit.exp=12;
        }

        return;

    }
    }

    fmt->Locale=SYSTEM_DEFAULT_LOCALE;
    fmt->SmallFmt=12|FMT_SCI|FMT_SUPRESSEXP|FMT_USECAPITALS;
    fmt->MiddleFmt=12|FMT_USECAPITALS;
    fmt->BigFmt=12|FMT_SCI|FMT_SUPRESSEXP|FMT_USECAPITALS;
    rplReadNumberAsReal((WORDPTR)one_bint,&(fmt->SmallLimit));
    fmt->SmallLimit.exp=-12;
    rplReadNumberAsReal((WORDPTR)one_bint,&(fmt->BigLimit));
    fmt->BigLimit.exp=12;
}


// SETS THE SYSTEM SETTING NUMFORMAT TO THE GIVEN STRUCTURE
// CAN TRIGGER GC, USES RREG[0], RREG[1] AND SCRATCHPOINTERS
void rplSetSystemNumberFormat(NUMFORMAT *fmt)
{
    // CREATE THE LIST WITH THE NUMFORMAT
    WORDPTR *savestk=DSTop;

    // MAKE THE LOCALE STRING

    BYTEPTR locstr=(BYTEPTR)RReg[0].data;

    WORD uchar;

    uchar=cp2utf8((int)DECIMAL_DOT(fmt->Locale));
    while(uchar) { *locstr++=uchar&0xff; uchar>>=8; }

    uchar=cp2utf8((int)THOUSAND_SEP(fmt->Locale));
    while(uchar) { *locstr++=uchar&0xff; uchar>>=8; }

    uchar=cp2utf8((int)FRAC_SEP(fmt->Locale));
    while(uchar) { *locstr++=uchar&0xff; uchar>>=8; }

    uchar=cp2utf8((int)ARG_SEP(fmt->Locale));
    while(uchar) { *locstr++=uchar&0xff; uchar>>=8; }


    WORDPTR item=rplCreateString((BYTEPTR)RReg[0].data,locstr);
    if(!item) return;

    // COPY TO RReg TO PROTECT FROM GARBAGE COLLECTION
    copyReal(&RReg[0],&(fmt->SmallLimit));
    copyReal(&RReg[1],&(fmt->BigLimit));


    rplPushData(item);
    rplNewBINTPush(fmt->SmallFmt,DECBINT);
    if(Exceptions) { DSTop=savestk; return; }
    rplNewBINTPush(fmt->MiddleFmt,DECBINT);
    if(Exceptions) { DSTop=savestk; return; }
    rplNewBINTPush(fmt->BigFmt,DECBINT);
    if(Exceptions) { DSTop=savestk; return; }
    rplNewRealFromRRegPush(0);
    if(Exceptions) { DSTop=savestk; return; }
    rplNewRealFromRRegPush(1);
    if(Exceptions) { DSTop=savestk; return; }
    rplNewSINTPush(6,DECBINT);
    rplCreateList();
    if(Exceptions) { DSTop=savestk; return; }

    rplStoreSettings((WORDPTR)numfmt_ident,rplPeekData(1));

    DSTop=savestk;
    return;

}



// ONLY VALID menunumbers ARE 1 AND 2
// THIS MAY CHANGE IN OTHER IMPLEMENTATIONS

void rplSetMenuCode(BINT menunumber,WORD menucode)
{
    if(!ISLIST(*SystemFlags)) return;

    if((menunumber<1)||(menunumber>2)) return;

    SystemFlags[7+menunumber]=menucode;

    return;
}

WORD rplGetMenuCode(BINT menunumber)
{
    if(!ISLIST(*SystemFlags)) return 0;

    if((menunumber<1)||(menunumber>2)) return 0;

    return  SystemFlags[7+menunumber];

}

void rplSetLastMenu(BINT menunumber)
{
    if((menunumber<1)||(menunumber>2)) return;

    if(menunumber==1) { rplClrSystemFlag(FL_LASTMENU); return; }
    rplSetSystemFlag(FL_LASTMENU);

}



void rplSetActiveMenu(BINT menunumber)
{
    if((menunumber<1)||(menunumber>2)) return;

    if(menunumber==1) { rplClrSystemFlag(FL_ACTIVEMENU); return; }
    rplSetSystemFlag(FL_ACTIVEMENU);

}

BINT rplGetLastMenu()
{
BINT a=rplTestSystemFlag(FL_LASTMENU);

if(a==1) return 2;
return 1;
}



BINT rplGetActiveMenu()
{
BINT a=rplTestSystemFlag(FL_ACTIVEMENU);

if(a==1) return 2;
return 1;
}

// PUSH THE CURRENT MENU INTO THE MENU HISTORY
// CAN TRIGGER GC AND USES ScratchPointers 1 thru 3
void rplSaveMenuHistory(BINT menu)
{
    WORD oldmcode=rplGetMenuCode(menu);

    // STORE THE OLD MENU IN THE HISTORY
    WORDPTR msetting;

    if((MENULIBRARY(oldmcode)==LIBRARY_NUMBER)&&(MENUNUMBER(oldmcode)<2)) {
        // SPECIAL CUSTOM MENUS, RCL FROM THE SETTINGS DIRECTORY
        if(menu==1)  msetting=rplGetSettings((WORDPTR)menu1_ident);
        else if(menu==2) msetting=rplGetSettings((WORDPTR)menu2_ident);
        else msetting=0;

        if(!msetting) msetting=(WORDPTR)empty_list; // IF MENU CONTENT CAN'T BE DETERMINED, RETURN AN EMPTY CUSTOM MENU
    }
    else {
    // NOTHING CUSTOM, JUST RETURN THE MENU CODE
    // THIS CAN TRIGGER A GC!
    msetting=rplNewBINT((BINT64)oldmcode,HEXBINT);
    }

    if(!msetting) return;

    // HERE msetting HAS EITHER THE OBJECT OR CODE TO STORE IN THE HISTORY

    WORDPTR oldlist=rplGetSettings((menu==1)? (WORDPTR)menu1hist_ident:(WORDPTR)menu2hist_ident);
    if(!oldlist) oldlist=(WORDPTR)empty_list;
    WORDPTR levels=rplGetSettings((WORDPTR)menuhistory_ident);
    if(!levels) levels=(WORDPTR)ten_bint;

    BINT64 nlevels=rplReadNumberAsBINT(levels);

    // THIS CAN TRIGGER A GC!
    WORDPTR newlist=rplListAddRot(oldlist,msetting,nlevels);

    if(!newlist) return;

    rplStoreSettings((menu==1)? (WORDPTR)menu1hist_ident:(WORDPTR)menu2hist_ident,newlist);

    return;
}

// RCL AND REMOVE FROM HISTORY THE LAST ITEM IN THE LIST

WORDPTR rplPopMenuHistory(BINT menu)
{
    WORDPTR list=rplGetSettings((menu==1)? (WORDPTR)menu1hist_ident:(WORDPTR)menu2hist_ident);
    if(!list) return 0;

    BINT nelem=rplExplodeList2(list);
    if(nelem<1) return 0;

    ScratchPointer1=rplPopData();

    list=rplCreateListN(nelem-1,1,1);
    if(!list) list=(WORDPTR)empty_list;

    rplStoreSettings((menu==1)? (WORDPTR)menu1hist_ident:(WORDPTR)menu2hist_ident,list);

    return ScratchPointer1;

}











// REPLACE THE ACTIVE MENU WITH THE GIVEN OBJECT
// THIS DOES THE SAME JOB AS TMENU, BUT CAN BE CALLED
// FROM OTHER LIBRARIES. MAY TRIGGER GC WHEN STORING
// IN SETTINGS DIRECTORY
void rplChangeMenu(BINT menu,WORDPTR newmenu)
{

       if(ISLIST(*newmenu)||ISIDENT(*newmenu)) {
           // CUSTOM MENU

          WORD mcode=MKMENUCODE(0,LIBRARY_NUMBER,menu-1,0);

          rplSetMenuCode(menu,mcode);

          // STORE THE LIST IN .Settings AS CURRENT MENU
          if(menu==2) rplStoreSettings((WORDPTR)menu2_ident,newmenu);
          else rplStoreSettings((WORDPTR)menu1_ident,newmenu);

         return;
       }



       if(ISBINT(*newmenu)) {
           // IT'S A PREDEFINED MENU CODE
           BINT64 num=rplReadBINT(newmenu);

           if((num<0)||(num>0xffffffff)) {
               // JUST SET IT TO ZERO
               rplSetMenuCode(menu,0);
               // STORE THE LIST IN .Settings AS CURRENT MENU
               if(menu==2) rplStoreSettings((WORDPTR)menu2_ident,(WORDPTR)zero_bint);
               else rplStoreSettings((WORDPTR)menu1_ident,(WORDPTR)zero_bint);

           }
           else {
           // WE HAVE A VALID MENU NUMBER

           rplSetMenuCode(menu,num);
           // STORE THE LIST IN .Settings AS CURRENT MENU
           if(menu==2) rplStoreSettings((WORDPTR)menu2_ident,newmenu);
           else rplStoreSettings((WORDPTR)menu1_ident,newmenu);

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

const BYTE const keytable[]={
    'O','N',KB_ON,
    'A','L',KB_ALPHA,
    'L','S',KB_LSHIFT,
    'R','S',KB_RSHIFT,
    'B','K',KB_BKS,
    'E','N',KB_ENT,
    'U','P',KB_UP,
    'D','N',KB_DN,
    'L','F',KB_LF,
    'R','T',KB_RT,
    'S','P',KB_SPC,
    'D','T',KB_DOT,
    '0',0,KB_0,
    '1',0,KB_1,
    '2',0,KB_2,
    '3',0,KB_3,
    '4',0,KB_4,
    '5',0,KB_5,
    '6',0,KB_6,
    '7',0,KB_7,
    '8',0,KB_8,
    '9',0,KB_9,
    'A',0,KB_A,
    'B',0,KB_B,
    'C',0,KB_C,
    'D',0,KB_D,
    'E',0,KB_E,
    'F',0,KB_F,
    'G',0,KB_G,
    'H',0,KB_H,
    'I',0,KB_I,
    'J',0,KB_J,
    'K',0,KB_K,
    'L',0,KB_L,
    'M',0,KB_M,
    'N',0,KB_N,
    'O',0,KB_O,
    'P',0,KB_P,
    'Q',0,KB_Q,
    'R',0,KB_R,
    'S',0,KB_S,
    'T',0,KB_T,
    'U',0,KB_U,
    'V',0,KB_V,
    'W',0,KB_W,
    'X',0,KB_X,
    'Y',0,KB_Y,
    'Z',0,KB_Z,
    '.',0,KB_DOT,
    '/',0,KB_DIV,
    '*',0,KB_MUL,
    '+',0,KB_ADD,
    '-',0,KB_SUB,
    0,0,0
};

const BYTE const modiftable[]={
    'A','L','H',(SHIFT_ALPHA|SHIFT_LSHOLD)>>4,
    'A','R','H',(SHIFT_ALPHA|SHIFT_RSHOLD)>>4,
    'L','H',0,SHIFT_LSHOLD>>4,
    'R','H',0,SHIFT_RSHOLD>>4,
    'A','L',0,(SHIFT_LS|SHIFT_ALPHA)>>4,
    'A','R',0,(SHIFT_RS|SHIFT_ALPHA)>>4,
    'A','H',0,SHIFT_ALPHAHOLD>>4,
    'O','H',0,SHIFT_ONHOLD>>4,
    'L',0,0,SHIFT_LS>>4,
    'R',0,0,SHIFT_RS>>4,
    'A',0,0,SHIFT_ALPHA>>4,
    0,0,0,0
};

BINT rplKeyName2Msg(WORDPTR keyname)
{
    BYTEPTR ptr,tblptr;
    BINT key,shifts,msg,len;

    if(!ISSTRING(*keyname)) return 0;

    ptr=(BYTEPTR) (keyname+1);
    len=rplStrSize(keyname);

    // IDENTIFY KEY NAME FIRST
    key=0;

    if((len<2)||((len>=2)&&(ptr[1]=='.'))) {
        // SINGLE-LETTER DEFINITION
        key=ptr[0];
        ++ptr;
        if(len>=2) { ++ptr; len-=2; }
        else --len;

    } else {
        // TRY SPECIAL 2-LETTER KEY NAME
        if((len<3)||((len>=3)&&(ptr[2]=='.'))) {
            // TWO-LETTER DEFINITION
            key=ptr[0]+256*ptr[1];
            ptr+=2;
            if(len>=3) { ++ptr; len-=3; }
            else len-=2;
    }
    }

    tblptr=(BYTEPTR)keytable;

    while(*tblptr) {
        if((tblptr[0]==(key&0xff))&&(tblptr[1]==((key>>8)&0xff))) {
            key=tblptr[2];
            break;
        }
        tblptr+=3;
    }

    if(!(*tblptr)) return 0;

    // HERE WE HAVE THE KEY CODE

    // LOOK FOR MODIFIERS

    shifts=0;
    if(len>0) {

    if(ptr[0]=='.') ++ptr;
    else if((len<2)||((len>=2)&&(ptr[1]=='.'))) {
        // SINGLE-LETTER DEFINITION
        shifts=ptr[0];
        ++ptr;
        if(len>=2) { ++ptr; len-=2; }
        else --len;

    } else {
        // 2-LETTER MODIFIER
        if((len<3)||((len>=3)&&(ptr[2]=='.'))) {
            // TWO-LETTER DEFINITION
            shifts=ptr[0]+256*ptr[1];
            ptr+=2;
            if(len>=3) { ++ptr; len-=3; }
            else len-=2;
    }
        else {
            // 3-LETTER MODIFIER
            if((len<4)||((len>=4)&&(ptr[3]=='.'))) {
                // THREE-LETTER DEFINITION
                shifts=ptr[0]+256*ptr[1]+65536*ptr[2];
                ptr+=3;
                if(len>=4) { ++ptr; len-=4; }
                else len-=3;

            }

        }
    }

    if(shifts) {

    tblptr=(BYTEPTR)modiftable;

    while(*tblptr) {
        if((tblptr[0]==(shifts&0xff))&&(tblptr[1]==((shifts>>8)&0xff))&&(tblptr[2]==((shifts>>16)&0xff))) {
            shifts=tblptr[3];
            break;
        }
        tblptr+=4;
    }

    if(!(*tblptr)) return 0;

    shifts<<=4;
    }
    }
    // FINALLY, LOOK FOR MESSAGE


    msg=0;
    if(len>0) {
    switch(ptr[0])
    {
    case 'D':
        msg=KM_KEYDN;
        break;
    case 'U':
        msg=KM_KEYUP;
        break;
    case 'L':
        msg=KM_LPRESS;
        break;
    case 'P':
        msg=KM_PRESS;
        break;
    case 'R':
        msg=KM_REPEAT;
        break;
    case 'T':
        msg=KM_LREPEAT;
        break;
    default:
       return 0;    // INVALID KEY MESSAGE

    }

    }

    // ASSEMBLE THE RESPONSE AND RETURN

    return msg|shifts|key;


}


// CREATE A STRING OBJECT FROM A KEYBOARD MESSAGE

WORDPTR rplMsg2KeyName(BINT keymsg)
{

    BYTEPTR tblptr=(BYTEPTR)keytable,keyptr;
    BYTE keytext[8];

    keyptr=keytext;

    while(*tblptr) {
        if(tblptr[2]==(KEYVALUE(keymsg))) {
            *keyptr++=tblptr[0];
            if(tblptr[1]) *keyptr++=tblptr[1];
            break;
        }
        tblptr+=3;
    }
    if(!*tblptr) {
        rplError(ERR_INVALIDKEYNAME);
        return 0;
    }


    if(KEYSHIFT(keymsg)) {
        *keyptr++='.';

        tblptr=(BYTEPTR)modiftable;

        while(*tblptr) {
        if(tblptr[3]==(KEYSHIFT(keymsg)>>4)) {
            *keyptr++=tblptr[0];
            if(tblptr[1]) *keyptr++=tblptr[1];
            if(tblptr[2]) *keyptr++=tblptr[2];
            break;
        }
        tblptr+=4;
    }
    if(!*tblptr) {
        rplError(ERR_INVALIDKEYNAME);
        return 0;
    }

    }


    if(KM_MESSAGE(keymsg)!=KM_PRESS) {

        if(!KEYSHIFT(keymsg)) *keyptr++='.';

        *keyptr++='.';

        switch(KM_MESSAGE(keymsg))
        {
        case KM_LPRESS:
            *keyptr++='L';
            break;
        case KM_KEYUP:
            *keyptr++='U';
            break;
        case KM_KEYDN:
            *keyptr++='D';
            break;
        case KM_REPEAT:
            *keyptr++='R';
            break;
        case KM_LREPEAT:
            *keyptr++='T';
            break;
        default:
            // ANY OTHER VALUES ARE ILLEGAL
            rplError(ERR_INVALIDKEYNAME);
            return 0;
        }

    }

    // NOW BUILD A STRING OBJECT AND RETURN

    return rplCreateString(keytext,keyptr);

}




void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
    }

    // LIBRARIES THAT DEFINE ONLY COMMANDS STILL HAVE TO RESPOND TO A FEW OVERLOADABLE OPERATORS
    if(LIBNUM(CurOpcode)==LIB_OVERLOADABLE) {
        // ONLY RESPOND TO EVAL, EVAL1 AND XEQ FOR THE COMMANDS DEFINED HERE
        // IN CASE OF COMMANDS TREATED AS OBJECTS (WHEN EMBEDDED IN LISTS)
        if( (OPCODE(CurOpcode)==OVR_EVAL)||
                (OPCODE(CurOpcode)==OVR_EVAL1)||
                (OPCODE(CurOpcode)==OVR_XEQ) )
        {
            // EXECUTE THE COMMAND BY CHANGING THE CURRENT OPCODE
            if(rplDepthData()<1) {
                rplError(ERR_BADARGCOUNT);
                return;
            }

            WORD saveOpcode=CurOpcode;
            CurOpcode=*rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode=saveOpcode;
            return;
        }
    }



    switch(OPCODE(CurOpcode))
    {
    case SETLOCALE:
    {
     NUMFORMAT fmt;
     if(rplDepthData()<1) {
         rplError(ERR_BADARGCOUNT);
         return;
     }
     if(!ISSTRING(*rplPeekData(1))) {
     rplError(ERR_STRINGEXPECTED);
     return;
     }

     BINT slen=rplStrLen(rplPeekData(1));

     if(slen!=4) {
         rplError(ERR_INVALIDLOCALESTRING);
         return;
    }

     rplGetSystemNumberFormat(&fmt);

     // EXTRACT ALL 4 CODE POINTS
     BYTEPTR locstring,strend;
     strend= (BYTEPTR)(rplPeekData(1)+1)+rplStrSize(rplPeekData(1));
     locstring=(BYTEPTR)(rplPeekData(1)+1);
     UBINT64 newlocale;

     BINT cp=utf82cp((char *)locstring,(char *)strend);

     if(cp<0) {
         rplError(ERR_INVALIDLOCALESTRING);
         return;
     }

     newlocale=cp;

     locstring=(BYTEPTR)utf8skipst((char *)locstring,(char *)strend);

     cp=utf82cp((char *)locstring,(char *)strend);

          if(cp<0) {
              rplError(ERR_INVALIDLOCALESTRING);
              return;
          }

     newlocale|=(((UBINT64)cp)<<16);

     locstring=(BYTEPTR)utf8skipst((char *)locstring,(char *)strend);

     cp=utf82cp((char *)locstring,(char *)strend);

          if(cp<0) {
              rplError(ERR_INVALIDLOCALESTRING);
              return;
          }

     newlocale|=(((UBINT64)cp)<<32);

     locstring=(BYTEPTR)utf8skipst((char *)locstring,(char *)strend);

     cp=utf82cp((char *)locstring,(char *)strend);

          if(cp<0) {
              rplError(ERR_INVALIDLOCALESTRING);
              return;
          }

     newlocale|=(((UBINT64)cp)<<48);

     fmt.Locale=newlocale;

     rplSetSystemNumberFormat(&fmt);
     rplDropData(1);
     return;

    }
    case CF:
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            BINT64 flag=rplReadNumberAsBINT(rplPeekData(1));
            if(flag<0) {
            switch(rplClrSystemFlag(flag))
            {
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
                switch(rplClrUserFlag(flag))
                {
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
            WORDPTR id=rplPeekData(1);
            switch(rplClrSystemFlagByIdent(id))
            {
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

        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            BINT64 flag=rplReadNumberAsBINT(rplPeekData(1));
            if(flag<0) {
            switch(rplSetSystemFlag(flag))
            {
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
                switch(rplSetUserFlag(flag))
                {
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
            WORDPTR id=rplPeekData(1);
            switch(rplSetSystemFlagByIdent(id))
            {
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
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        BINT test;
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            BINT64 flag=rplReadNumberAsBINT(rplPeekData(1));
            if(flag<0) {
            switch(test=rplTestSystemFlag(flag))
            {
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
                switch(test=rplTestUserFlag(flag))
                {
                       case -1:
                       rplError(ERR_INVALIDFLAGNUMBER);
                       return;
                       default:
                       rplDropData(1);
                }
            }
            if(test) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        }

        if(ISIDENT(*rplPeekData(1))) {
            // FLAG GIVEN BY NAME
            WORDPTR id=rplPeekData(1);
            switch(test=rplTestSystemFlagByIdent(id))
            {
                   case -1:
                   rplError(ERR_INVALIDFLAGNAME);
                   return;
                   case -2:
                   rplError(ERR_SYSTEMFLAGSINVALID);
                   return;
                   default:
                   rplDropData(1);
            }
            if(test) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        }

        rplError(ERR_IDENTORINTEGEREXPECTED);
        return;


     }
    case FSTEST:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        BINT test;
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            BINT64 flag=rplReadNumberAsBINT(rplPeekData(1));
            if(flag<0) {
            switch(test=rplTestSystemFlag(flag))
            {
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
                switch(test=rplTestUserFlag(flag))
                {
                       case -1:
                       rplError(ERR_INVALIDFLAGNUMBER);
                       return;
                       default:
                       rplDropData(1);
                }
            }

            if(test) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        }

        if(ISIDENT(*rplPeekData(1))) {
            // FLAG GIVEN BY NAME
            WORDPTR id=rplPeekData(1);
            switch(test=rplTestSystemFlagByIdent(id))
            {
                   case -1:
                   rplError(ERR_INVALIDFLAGNAME);
                   return;
                   case -2:
                   rplError(ERR_SYSTEMFLAGSINVALID);
                   return;
                   default:
                   rplDropData(1);
            }
            if(test) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        }

        rplError(ERR_IDENTORINTEGEREXPECTED);
        return;


     }

    case FCTESTCLEAR:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        BINT test;
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            BINT64 flag=rplReadNumberAsBINT(rplPeekData(1));
            if(flag<0) {
            switch(test=rplTestSystemFlag(flag))
            {
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
                    switch(test=rplTestUserFlag(flag))
                    {
                           case -1:
                           rplError(ERR_INVALIDFLAGNUMBER);
                           return;
                           default:
                           rplDropData(1);
                    }
                    rplClrUserFlag(flag);
                }


            if(test) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        }

        if(ISIDENT(*rplPeekData(1))) {
            // FLAG GIVEN BY NAME
            WORDPTR id=rplPeekData(1);
            switch(test=rplTestSystemFlagByIdent(id))
            {
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
            if(test) rplPushData((WORDPTR)zero_bint);
            else rplPushData((WORDPTR)one_bint);
            return;
        }

        rplError(ERR_IDENTORINTEGEREXPECTED);
        return;


     }

    case FSTESTCLEAR:

    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        BINT test;
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            BINT64 flag=rplReadNumberAsBINT(rplPeekData(1));
            if(flag<0) {
            switch(test=rplTestSystemFlag(flag))
            {
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
                switch(test=rplTestUserFlag(flag))
                {
                       case -1:
                       rplError(ERR_INVALIDFLAGNUMBER);
                       return;
                       default:
                       rplDropData(1);
                }
                rplClrUserFlag(flag);
            }

            if(test) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        }

        if(ISIDENT(*rplPeekData(1))) {
            // FLAG GIVEN BY NAME
            WORDPTR id=rplPeekData(1);
            switch(test=rplTestSystemFlagByIdent(id))
            {
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
            if(test) rplPushData((WORDPTR)one_bint);
            else rplPushData((WORDPTR)zero_bint);
            return;
        }

        rplError(ERR_IDENTORINTEGEREXPECTED);
        return;


     }

    case TMENU:
     {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR arg=rplPeekData(1);
        BINT menu=rplGetActiveMenu();

        rplSaveMenuHistory(menu);
        rplChangeMenu(menu,arg);

        if(!Exceptions) {
            rplDropData(1);
        }

      return;
    }


    case TMENULST:
    case TMENUOTHR:
     {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        WORDPTR arg=rplPeekData(1);
        BINT menu=rplGetLastMenu();
        if(CurOpcode==CMD_TMENUOTHR) {
            // USE THE OTHER MENU
            if(menu==1) menu=2;
            else menu=1;
        }

        rplSaveMenuHistory(menu);
        rplChangeMenu(menu,arg);

        if(!Exceptions) {
            rplDropData(1);
        }

      return;
    }

    case RCLMENU:
    case RCLMENULST:
    case RCLMENUOTHR:
    {
        BINT menu;

        if(CurOpcode!=CMD_RCLMENU) {
        menu=rplGetLastMenu();
        if(CurOpcode==CMD_RCLMENUOTHR) {
            // USE THE OTHER MENU
            if(menu==1) menu=2;
            else menu=1;
        }
        }
        else menu=rplGetActiveMenu();


        WORD mcode=rplGetMenuCode(menu);

        if((MENULIBRARY(mcode)==LIBRARY_NUMBER)&&(MENUNUMBER(mcode)<2)) {
            // SPECIAL CUSTOM MENUS, RCL FROM THE SETTINGS DIRECTORY
            WORDPTR msetting;
            if(menu==1)  msetting=rplGetSettings((WORDPTR)menu1_ident);
            else if(menu==2) msetting=rplGetSettings((WORDPTR)menu2_ident);
            else msetting=0;

            if(!msetting) msetting=(WORDPTR)empty_list; // IF MENU CONTENT CAN'T BE DETERMINED, RETURN AN EMPTY CUSTOM MENU

            rplPushData(msetting);
            return;
        }

        // NOTHING CUSTOM, JUST RETURN THE MENU CODE

        rplNewBINTPush((BINT64)mcode,HEXBINT);
        return;

    }



    case MENUSWAP:
    {
        // JUST SWAP MENUS 1 AND 2
        WORD m1code=rplGetMenuCode(1);
        WORD m2code=rplGetMenuCode(2);

        if((MENULIBRARY(m2code)==LIBRARY_NUMBER)&&(MENUNUMBER(m2code)<2)) m2code=MKMENUCODE(0,LIBRARY_NUMBER,MENUNUMBER(m2code)^1,MENUPAGE(m2code));  // ALTERNATE MENU'S 1 AND 2
        if((MENULIBRARY(m1code)==LIBRARY_NUMBER)&&(MENUNUMBER(m1code)<2)) m1code=MKMENUCODE(0,LIBRARY_NUMBER,MENUNUMBER(m1code)^1,MENUPAGE(m1code));  // ALTERNATE MENU'S 1 AND 2


        rplSetMenuCode(1,m2code);
        rplSetMenuCode(2,m1code);

        WORDPTR m1setting,m2setting;

        m1setting=rplGetSettings((WORDPTR)menu1_ident);
        m2setting=rplGetSettings((WORDPTR)menu2_ident);

        if(m1setting) rplStoreSettings((WORDPTR)menu2_ident,m1setting);
        if(m2setting) rplStoreSettings((WORDPTR)menu1_ident,m2setting);

        m1setting=rplGetSettings((WORDPTR)menu1hist_ident);
        m2setting=rplGetSettings((WORDPTR)menu2hist_ident);

        if(m1setting) rplStoreSettings((WORDPTR)menu2hist_ident,m1setting);
        if(m2setting) rplStoreSettings((WORDPTR)menu1hist_ident,m2setting);

        return;
    }

    case MENUBK:
    case MENUBKLST:
    case MENUBKOTHR:
    {
        BINT menu;

        if(CurOpcode!=CMD_MENUBK) {
        menu=rplGetLastMenu();
        if(CurOpcode==CMD_MENUBKOTHR) {
            // USE THE OTHER MENU
            if(menu==1) menu=2;
            else menu=1;
        }
        }
        else menu=rplGetActiveMenu();

        WORDPTR menuobj=rplPopMenuHistory(menu);

        if(menuobj) rplChangeMenu(menu,menuobj);

        return;
    }


    case DEG:
        rplClrSystemFlag(-17);
        rplClrSystemFlag(-18);
        return;
    case RAD:
        rplSetSystemFlag(-17);
        rplClrSystemFlag(-18);
        return;
    case GRAD:
        rplClrSystemFlag(-17);
        rplSetSystemFlag(-18);
        return;
    case DMS:
        rplSetSystemFlag(-17);
        rplSetSystemFlag(-18);
        return;


    case ASNKEY:
    {
        // ASSIGN A CUSTOM KEY DEFINITION
        if(rplDepthData()<3) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISNUMBER(*rplPeekData(1))) {
            rplError(ERR_POSITIVEINTEGEREXPECTED);
            return;
        }
        if(!ISSTRING(*rplPeekData(2))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        BINT keycode=rplKeyName2Msg(rplPeekData(2));

        if(!keycode) {
            rplError(ERR_INVALIDKEYNAME);
            return;
        }

        BINT context=rplReadNumberAsBINT(rplPeekData(1));
        if(context<0) {
            rplError(ERR_POSITIVEINTEGEREXPECTED);
            return;
        }

        rplDropData(2);
        rplNewBINTPush(keycode,HEXBINT);
        rplNewBINTPush(context,HEXBINT);
        if(Exceptions) return;

        rplPushData(rplPeekData(3));        // STACK HAS NOW PROPER ORDER: { KEYCODE CONTEXT ACTION }

        WORDPTR keylist=rplGetSettings((WORDPTR)customkey_ident);

        if(!keylist) {
            // NEED TO CREATE A LIST FROM SCRATCH
            keylist=rplCreateListN(3,1,1);
            if(!keylist) return;
        }
        else {
            // ADD 3 MORE ITEMS TO THE EXISTING LIST
            BINT n=rplExplodeList2(keylist);
            keylist=rplCreateListN(n+3,1,1);
            if(Exceptions) return;
        }

        rplDropData(1);


        rplStoreSettings((WORDPTR)customkey_ident,keylist);
        return;

    }

    case DELKEY:
    {
        // REMOVE A CUSTOM KEY DEFINITION
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISSTRING(*rplPeekData(1))) {
            rplError(ERR_STRINGEXPECTED);
            return;
        }

        BINT keycode=rplKeyName2Msg(rplPeekData(1));

        if(!keycode) {
            rplError(ERR_INVALIDKEYNAME);
            return;
        }

        rplDropData(1);

        WORDPTR keylist=rplGetSettings((WORDPTR)customkey_ident);

        if(!keylist) return;

        // SCAN THE LIST

        WORDPTR ptr=keylist+1;
        WORDPTR endlist=rplSkipOb(keylist);

        while(ptr<endlist) {
         if(ISNUMBER(*ptr)) {
         BINT code=rplReadNumberAsBINT(ptr);
         if(keycode==code) break;
         }
         ptr=rplSkipOb(ptr);    // SKIP KEYCODE
         if(ptr>=endlist) break;
         ptr=rplSkipOb(ptr);    // SKIP CONTEXT
         if(ptr>=endlist) break;
         ptr=rplSkipOb(ptr);    // SKIP ACTION
        }

        // HERE WE HAVE THE PTR INTO THE LIST
        if(ptr<endlist) {
            // COMPUTE SIZE OF NEW LIST
            WORDPTR endptr=rplSkipOb(ptr);
            if(endptr<endlist) endptr=rplSkipOb(endptr);
            if(endptr<endlist) endptr=rplSkipOb(endptr);

            // NOW MOVE THE MEMORY BETWEEN ptr AND endptr

            BINT offstart=ptr-keylist,offend=endptr-keylist;

            rplPushData(keylist);   // PROTECT FROM GC
            WORDPTR newlist=rplAllocTempOb(OBJSIZE(*keylist)-(offend-offstart));
            if(!newlist) return;
            keylist=rplPopData();
            memmovew(newlist+1,keylist+1,offstart-1);
            memmovew(newlist+offstart,keylist+offend,OBJSIZE(*keylist)-offend+1);
            // NOW WRITE A NEW PROLOG
            *newlist=MKPROLOG(DOLIST,OBJSIZE(*keylist)-(offend-offstart));
            // AND STORE THE NEW LIST
            rplStoreSettings((WORDPTR)customkey_ident,newlist);

           }


        return;

    }


    case RCLKEYS:
    {
        // GET THE ENTIRE LIST OF KEY ASSIGNMENTS
        WORDPTR keylist=rplGetSettings((WORDPTR)customkey_ident);

        if(!keylist) keylist=(WORDPTR)empty_list;

        rplPushData(keylist);
        return;
    }


    case STOKEYS:
    {
        // STORE THE ENTIRE LIST OF KEY ASSIGNMENTS
        // REMOVE A CUSTOM KEY DEFINITION
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        if(!ISLIST(*rplPeekData(1))) {
            rplError(ERR_LISTEXPECTED);
            return;
        }

        // VERIFY THAT THE LIST IS VALID
        // OTHERWISE IT CAN HAVE CATASTROPHIC CONSEQUENCES
        WORDPTR ptr=rplPeekData(1)+1;
        WORDPTR endlist=rplSkipOb(ptr-1);
        while(ptr<endlist)
        {
            if(*ptr==CMD_ENDLIST) break;
            if(!ISNUMBER(*ptr)) {
                rplError(ERR_INVALIDKEYDEFINITION);
                return;
            }
            ptr=rplSkipOb(ptr);
            if(*ptr==CMD_ENDLIST) {
                rplError(ERR_INVALIDKEYDEFINITION);
                return;
            }
            if(!ISNUMBER(*ptr)) {
                rplError(ERR_INVALIDKEYDEFINITION);
                return;
            }
            ptr=rplSkipOb(ptr);
            if(*ptr==CMD_ENDLIST) {
                rplError(ERR_INVALIDKEYDEFINITION);
                return;
            }
            ptr=rplSkipOb(ptr);
        }

        // HERE THE LIST IS VALID
        // AND STORE THE NEW LIST
        rplStoreSettings((WORDPTR)customkey_ident,rplPeekData(1));
        rplDropData(1);
        return;

    }

 case TYPE:
    {
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        LIBHANDLER han=rplGetLibHandler(LIBNUM(*rplPeekData(1)));


        // GET THE SYMBOLIC TOKEN INFORMATION
        if(han) {
            WORD savecurOpcode=CurOpcode;
            ObjectPTR=rplPeekData(1);
            CurOpcode=MKOPCODE(LIBNUM(*ObjectPTR),OPCODE_GETINFO);
            (*han)();

            CurOpcode=savecurOpcode;

            if(RetNum>OK_TOKENINFO) {
               rplDropData(1);
               rplNewBINTPush(TypeInfo/100,DECBINT);
               return;
            }
        }
        rplOverwriteData(1,(WORDPTR)zero_bint);

        return;
    }

    case TYPEE:
       {
           if(rplDepthData()<1) {
               rplError(ERR_BADARGCOUNT);
               return;
           }

           LIBHANDLER han=rplGetLibHandler(LIBNUM(*rplPeekData(1)));


           // GET THE SYMBOLIC TOKEN INFORMATION
           if(han) {
               WORD savecurOpcode=CurOpcode;
               ObjectPTR=rplPeekData(1);
               CurOpcode=MKOPCODE(LIBNUM(*ObjectPTR),OPCODE_GETINFO);
               (*han)();

               CurOpcode=savecurOpcode;

               if(RetNum>OK_TOKENINFO) {
                  rplDropData(1);
                  if(TypeInfo%100) {
                      newRealFromBINT64(&RReg[0],TypeInfo,-2);

                      rplNewRealFromRRegPush(0);
                  } else rplNewBINTPush(TypeInfo/100,DECBINT);
                  return;
               }
           }
           rplOverwriteData(1,(WORDPTR)zero_bint);

           return;
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
        libCompileCmds(LIBRARY_NUMBER,(char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);

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
        libDecompileCmds((char **)LIB_NAMES,NULL,LIB_NUMBEROFCMDS);
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


        RetNum=OK_CONTINUE;
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
        libProbeCmds((char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);

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
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL BINT, .42 = HEX INTEGER

        if(ISPROLOG(*ObjectPTR)) {
        TypeInfo=LIBRARY_NUMBER*100;
        DecompHints=0;
        RetNum=OK_TOKENINFO | MKTOKENINFO(0,TITYPE_NOTALLOWED,0,1);
        }
        else {
            TypeInfo=0;     // ALL COMMANDS ARE TYPE 0
            DecompHints=0;
            libGetInfo2(*ObjectPTR,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
        }
        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER,(WORDPTR *)ROMPTR_TABLE,ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *)ROMPTR_TABLE,ObjectID);
        return;
    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) { RetNum=ERR_INVALID; return; }

        RetNum=OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER,(char **)LIB_NAMES,LIB_NUMBEROFCMDS);
        return;

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        WORDPTR menuobj;
    switch(MENUNUMBER(MenuCodeArg))
    {
    case 0:
        menuobj=rplGetSettings((WORDPTR)menu1_ident);
        break;
    case 1:
        menuobj=rplGetSettings((WORDPTR)menu2_ident);
        break;

    default:
        if((MENUNUMBER(MenuCodeArg)<=12)&&(MENUNUMBER(MenuCodeArg)>1)) menuobj=ROMPTR_TABLE[MENUNUMBER(MenuCodeArg)];
        else menuobj=0;
    }
    if(!menuobj) ObjectPTR=(WORDPTR)empty_list;
    else ObjectPTR=menuobj;

    if(ISIDENT(*ObjectPTR)) {
        // RCL THE VARIABLE

        WORDPTR *var=rplFindGlobal(ObjectPTR,1);
        if(!var) ObjectPTR=(WORDPTR)empty_list;
        else ObjectPTR=var[1];
    }



    RetNum=OK_CONTINUE;
    return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp,(WORDPTR)LIB_HELPTABLE);
       return;
    }

    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {

        libFindMsg(LibError,(WORDPTR)LIB_MSGTABLE);
        return;
    }


    case OPCODE_LIBINSTALL:
        LibraryList=(WORDPTR)libnumberlist;
        RetNum=OK_CONTINUE;
        return;
    case OPCODE_LIBREMOVE:
        return;


    }
    // UNHANDLED OPCODE...

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode)>=MIN_RESERVED_OPCODE) {
        RetNum=ERR_NOTMINE;
        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    rplError(ERR_INVALIDOPCODE);

    return;


}



#endif
