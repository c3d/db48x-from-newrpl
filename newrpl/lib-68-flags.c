/*
 * Copyright (c) 2014, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "newrpl.h"
#include "libraries.h"
#include "hal.h"
// LIB 68 PROVIDES COMMANDS THAT DEAL WITH SYSTEM SETTINGS AND FLAGS

// MAIN LIBRARY NUMBER, CHANGE THIS FOR EACH LIBRARY
#define LIBRARY_NUMBER  68
#define LIB_ENUM lib68_enum
#define LIB_NAMES lib68_names
#define LIB_HANDLER lib68_handler
#define LIB_TOKENINFO lib68_tokeninfo
#define LIB_NUMBEROFCMDS LIB68_NUMBEROFCMDS
#define ROMPTR_TABLE    romptr_table68

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
    CMD(SETLOCALE,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SETNUMFORMAT,MKTOKENINFO(9,TITYPE_NOTALLOWED,1,2)), \
    CMD(SF,MKTOKENINFO(2,TITYPE_NOTALLOWED,1,2)), \
    CMD(CF,MKTOKENINFO(2,TITYPE_NOTALLOWED,1,2))

// ADD MORE OPCODES HERE


// EXTRA LIST FOR COMMANDS WITH SYMBOLS THAT ARE DISALLOWED IN AN ENUM
// THE NAMES AND ENUM SYMBOLS ARE GIVEN SEPARATELY
#define CMD_EXTRANAME \
    ECMD("FC?",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD("FS?",MKTOKENINFO(3,TITYPE_NOTALLOWED,1,2)), \
    ECMD("FC?C",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2)), \
    ECMD("FS?C",MKTOKENINFO(4,TITYPE_NOTALLOWED,1,2))

#define CMD_EXTRAENUM \
    FCTEST, \
    FSTEST, \
    FCTESTCLEAR, \
    FSTESTCLEAR

// INTERNAL DECLARATIONS


// CREATE AN ENUM WITH THE OPCODE NAMES FOR THE DISPATCHER
#define CMD(a,b) a
enum LIB_ENUM { CMD_LIST , CMD_EXTRAENUM , LIB_NUMBEROFCMDS };
#undef CMD

// AND A LIST OF STRINGS WITH THE NAMES FOR THE COMPILER
#define CMD(a,b) #a
#define ECMD(a,b) a
const char * const LIB_NAMES[]= { CMD_LIST , CMD_EXTRANAME };
#undef CMD
#undef ECMD

// AND A LIST WITH THE TOKENINFO DATA
#define CMD(a,b) b
#define ECMD(a,b) b
const BINT const LIB_TOKENINFO[]=
{
        CMD_LIST,
        CMD_EXTRANAME
};
#undef ECMD
#undef CMD



const WORD const dotsettings_ident[]= {
        MKPROLOG(DOIDENTHIDDEN,3),
        TEXT2WORD('.','S','e','t'),
        TEXT2WORD('t','i','n','g'),
        TEXT2WORD('s',0,0,0)
};
const WORD const flags_ident[]= {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('F','l','a','g'),
        TEXT2WORD('s',0,0,0)
};

const WORD const locale_ident[]= {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('L','o','c','a'),
        TEXT2WORD('l','e',0,0)
};

const WORD const numfmt_ident[]= {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('N','u','m','F'),
        TEXT2WORD('m','t',0,0)

};



// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[]={
    (WORDPTR)dotsettings_ident,
    (WORDPTR)flags_ident,
    (WORDPTR)locale_ident,
    (WORDPTR)numfmt_ident,
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
    { "COMMENTS", {0x80|30,0,0,0,0,0,0,0} },

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
WORD rplGetSystemLocale()
{
    WORDPTR systemlist=rplGetSettings((WORDPTR)numfmt_ident);
    if(systemlist) {
        if(ISLIST(*systemlist)) {
        WORDPTR localestring=rplGetListElement(systemlist,1);
        if(localestring && (ISSTRING(*localestring))) return *(localestring+1);
        }
    }
    // INVALID FLAGS, JUST RETURN A DEFAULT SETTING
    return ((WORD)'.') | (((WORD)' ')<<8) | (((WORD)' ')<<16) | (((WORD)',')<<24);

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
        if(localestring && (ISSTRING(*localestring))) fmt->Locale=*(localestring+1);
        else fmt->Locale=((WORD)'.') | (((WORD)' ')<<8) | (((WORD)' ')<<16) | (((WORD)',')<<24);
        WORDPTR nfmt=rplGetListElement(systemlist,2);
        if(nfmt && (ISBINT(*nfmt))) fmt->SmallFmt=(BINT)rplReadBINT(nfmt);
        else fmt->SmallFmt=12|FMT_SCI|FMT_NOZEROEXP;
        nfmt=rplGetListElement(systemlist,3);
        if(nfmt && (ISBINT(*nfmt))) fmt->MiddleFmt=(BINT)rplReadBINT(nfmt);
        else fmt->MiddleFmt=12;
        nfmt=rplGetListElement(systemlist,4);
        if(nfmt && (ISBINT(*nfmt))) fmt->BigFmt=(BINT)rplReadBINT(nfmt);
        else fmt->BigFmt=12|FMT_SCI|FMT_NOZEROEXP;
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

    fmt->Locale=((WORD)'.') | (((WORD)' ')<<8) | (((WORD)' ')<<16) | (((WORD)',')<<24);
    fmt->SmallFmt=12|FMT_SCI|FMT_NOZEROEXP;
    fmt->MiddleFmt=12;
    fmt->BigFmt=12|FMT_SCI|FMT_NOZEROEXP;
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

    // COPY TO RReg TO PROTECT FROM GARBAGE COLLECTION
    copyReal(&RReg[0],&(fmt->SmallLimit));
    copyReal(&RReg[1],&(fmt->BigLimit));

    WORDPTR item=rplCreateString((BYTEPTR)&(fmt->Locale),((BYTEPTR)&(fmt->Locale))+4);
    if(!item) return;
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











void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        rplError(ERR_UNRECOGNIZEDOBJECT);
        return;
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





