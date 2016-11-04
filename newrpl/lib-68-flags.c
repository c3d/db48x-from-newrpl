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
    CMD(RCLMENU,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)),\
    CMD(RCLMENULST,MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2)),\
    CMD(RCLMENUOTHR,MKTOKENINFO(11,TITYPE_NOTALLOWED,1,2)),\
    CMD(COPYCLIP,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(CUTCLIP,MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(PASTECLIP,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(DEG,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(GRAD,MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    CMD(RAD,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    CMD(DMS,MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2))


// ADD MORE OPCODES HERE


// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS LIBRARY_NUMBER


// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"


#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************



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

ROMOBJECT menu2_ident[]= {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('M','e','n','u'),
        TEXT2WORD('2',0,0,0)
};

ROMOBJECT clipbd_ident[] = {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('C','l','i','p'),
        TEXT2WORD('B','d',0,0)

};


// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
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

    (WORDPTR)dotsettings_ident,
    (WORDPTR)flags_ident,
    (WORDPTR)locale_ident,
    (WORDPTR)numfmt_ident,
    (WORDPTR)menu1_ident,
    (WORDPTR)menu2_ident,
    (WORDPTR)clipbd_ident,

    0
};



typedef struct {
    const char *flagname;
    unsigned char flags[8];
} systemflag;


const systemflag const flags_names[]= {
    // EACH OF THE 8 CHARS CONTAINS: BITS 0-6:= FLAG NUMBER (1-127),
    // BIT 7= VALUE (ON/OFF) TO USE WITH SET FLAG, ASSUMED ALWAYS 0 FOR CLEAR FLAG.

    { "DEG", { 17,18,0,0,0,0,0,0}  },
    { "RAD", {0x80|17,18,0,0,0,0,0,0} },
    { "GRAD", {17, 0X80|18,0,0,0,0,0,0} },
    { "DMS", {0x80|17,0x80|18,0,0,0,0,0,0} },
    { "COMMENTS", {0x80|30,0,0,0,0,0,0,0} },
    { "ACTIVEMENU1", { 0x80|4,0,0,0,0,0,0,0} },
    { "ACTIVEMENU2", { 4, 0,0,0,0,0,0,0} },
    { "DATEDMY" , { 0X80|42,0,0,0,0,0,0,0} },
    { "DATEMDY" , { 42,0,0,0,0,0,0,0} },
    { "TIME12" , { 41,0,0,0,0,0,0,0} },
    { "TIME24" , { 0x80|41,0,0,0,0,0,0,0} },



// TODO: ADD MORE FLAG NAMES HERE
    { NULL , {0,0,0,0,0,0,0,0} }
};













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
    return rplSetSystemFlagByName(text,text+rplGetIdentLength(ident));
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

// REPLACE THE ACTIVE MENU WITH THE GIVEN OBJECT
// THIS DOES THE SAME JOB AS TMENU, BUT CAN BE CALLED
// FROM OTHER LIBRARIES. MAY TRIGGER GC WHEN STORING
// IN SETTINGS DIRECTORY
void rplChangeMenu(WORDPTR newmenu)
{
       BINT menu=rplGetActiveMenu();

       if(ISIDENT(*newmenu)) {

           // RCL THE VARIABLE AND LEAVE CONTENTS ON THE STACK

           WORDPTR *var=rplFindLAM(newmenu,1);
           if(!var) var=rplFindGlobal(newmenu,1);

           if(!var) {
              rplError(ERR_UNDEFINEDVARIABLE);
              return;
           }

           // REPLACE THE IDENT WITH ITS CONTENTS
           newmenu=var[1];

           // AND CONTINUE EXCECUTION
       }

       if(ISLIST(*newmenu)) {
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

     BINT slen=rplStrSize(rplPeekData(1));

     if(slen!=4) {
         rplError(ERR_INVALIDLOCALESTRING);
         return;
    }

     rplGetSystemNumberFormat(&fmt);

     fmt.Locale=*(rplPopData()+1);

     rplSetSystemNumberFormat(&fmt);
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

        if(ISIDENT(*arg)) {

            // CUSTOM MENU

           WORD mcode=MKMENUCODE(0,LIBRARY_NUMBER,menu-1,0);

           rplSetMenuCode(menu,mcode);

           // STORE THE IDENT IN .Settings AS CURRENT MENU
           if(menu==2) rplStoreSettings((WORDPTR)menu2_ident,arg);
           else rplStoreSettings((WORDPTR)menu1_ident,arg);

           rplDropData(1);
          return;

            // AND CONTINUE EXCECUTION
        }

        if(ISLIST(*arg)) {
            // CUSTOM MENU

           WORD mcode=MKMENUCODE(0,LIBRARY_NUMBER,menu-1,0);

           rplSetMenuCode(menu,mcode);

           // STORE THE LIST IN .Settings AS CURRENT MENU
           if(menu==2) rplStoreSettings((WORDPTR)menu2_ident,arg);
           else rplStoreSettings((WORDPTR)menu1_ident,arg);

           rplDropData(1);
          return;
        }



        if(ISBINT(*arg)) {
            // IT'S A PREDEFINED MENU CODE
            BINT64 num=rplReadBINT(arg);

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
            if(menu==2) rplStoreSettings((WORDPTR)menu2_ident,arg);
            else rplStoreSettings((WORDPTR)menu1_ident,arg);

            }

            rplDropData(1);
            return;
        }

        rplError(ERR_BADARGTYPE);

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

        if(ISIDENT(*arg)) {

            // CUSTOM MENU

           WORD mcode=MKMENUCODE(0,LIBRARY_NUMBER,menu-1,0);

           rplSetMenuCode(menu,mcode);

           // STORE THE IDENT IN .Settings AS CURRENT MENU
           if(menu==2) rplStoreSettings((WORDPTR)menu2_ident,arg);
           else rplStoreSettings((WORDPTR)menu1_ident,arg);

           rplDropData(1);
          return;
        }

        if(ISLIST(*arg)) {
            // CUSTOM MENU

           WORD mcode=MKMENUCODE(0,LIBRARY_NUMBER,menu-1,0);

           rplSetMenuCode(menu,mcode);

           // STORE THE LIST IN .Settings AS CURRENT MENU
           if(menu==2) rplStoreSettings((WORDPTR)menu2_ident,arg);
           else rplStoreSettings((WORDPTR)menu1_ident,arg);

           rplDropData(1);
          return;
        }



        if(ISBINT(*arg)) {
            // IT'S A PREDEFINED MENU CODE
            BINT64 num=rplReadBINT(arg);

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
            if(menu==2) rplStoreSettings((WORDPTR)menu2_ident,arg);
            else rplStoreSettings((WORDPTR)menu1_ident,arg);

            }

            rplDropData(1);
            return;
        }

        rplError(ERR_BADARGTYPE);

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


        return;
    }

    case COPYCLIP:
    {
        // STORE LEVEL 1 INTO .Settings/Clipbd
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStoreSettings((WORDPTR)clipbd_ident,rplPeekData(1));

        return;

    }
    case CUTCLIP:
    {
        // STORE LEVEL 1 INTO .Settings/Clipbd
        if(rplDepthData()<1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }

        rplStoreSettings((WORDPTR)clipbd_ident,rplPopData());

        return;

    }

    case PASTECLIP:
    {
        WORDPTR object=rplGetSettings((WORDPTR)clipbd_ident);

        if(!object) rplError(ERR_EMPTYCLIPBOARD);
        else rplPushData(object);
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
        libGetInfo2(*DecompileObject,(char **)LIB_NAMES,(BINT *)LIB_TOKENINFO,LIB_NUMBEROFCMDS);
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
        if((MENUNUMBER(MenuCodeArg)<=10)&&(MENUNUMBER(MenuCodeArg)>1)) menuobj=ROMPTR_TABLE[MENUNUMBER(MenuCodeArg)-1];
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
