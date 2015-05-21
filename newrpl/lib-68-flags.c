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

// LIST OF LIBRARY NUMBERS WHERE THIS LIBRARY REGISTERS TO
// HAS TO BE A HALFWORD LIST TERMINATED IN ZERO
static const HALFWORD const libnumberlist[]={ LIBRARY_NUMBER,0 };

// LIST OF COMMANDS EXPORTED, CHANGE FOR EACH LIBRARY
#define CMD_LIST \
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



const WORD dotsettings_ident[]= {
        MKPROLOG(DOIDENT,3),
        TEXT2WORD('.','S','e','t'),
        TEXT2WORD('t','i','n','g'),
        TEXT2WORD('s',0,0,0)
};
const WORD flags_ident[]= {
        MKPROLOG(DOIDENT,2),
        TEXT2WORD('F','L','A','G'),
        TEXT2WORD('S',0,0,0)
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


BINT rplGetSystemFlag(BINT flag)
{
    if(flag>-1 || flag<-128) return;

    WORDPTR low64=SystemFlags+2;
    WORDPTR hi64=SystemFlags+5;
    if(flag>=-32) return low64[0]&(1 << -(flag+1));
    else if(flag>=-64) return low64[1]&(1 << -(flag+33));
    else if(flag>=96) return hi64[0]&(1 << -(flag+65));
    else return hi64[1]&(1 << -(flag+97));
}


void rplSetSystemFlag(BINT flag)
{
    if(flag>-1 || flag<-128) return;

    WORDPTR low64=SystemFlags+2;
    WORDPTR hi64=SystemFlags+5;
    if(flag>=-32) low64[0]|=(1 << -(flag+1));
    else if(flag>=-64) low64[1]|=(1 << -(flag+33));
    else if(flag>=96) hi64[0]|=(1 << -(flag+65));
    else hi64[1]|=(1 << -(flag+97));
}

void rplClrSystemFlag(BINT flag)
{
    if(flag>-1 || flag<-128) return;

    WORDPTR low64=SystemFlags+2;
    WORDPTR hi64=SystemFlags+5;
    if(flag>=-32) low64[0]&=~(1 << -(flag+1));
    else if(flag>=-64) low64[1]&=~(1 << -(flag+33));
    else if(flag>=96) hi64[0]&=~(1 << -(flag+65));
    else hi64[1]&=~(1 << -(flag+97));
}

void rplSetSystemFlagByName(BYTEPTR name,BINT len)
{
        BINT idx=0;
        while(flags_names[idx].flagname) {
        if(!utf8ncmp((char *)name,flags_names[idx].flagname,len))
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

            return;

        }
        ++idx;
        }

}

void rplClrSystemFlagByName(BYTEPTR name,BINT len)
{
        BINT idx=0;
        while(flags_names[idx].flagname) {
        if(!utf8ncmp((char *)name,flags_names[idx].flagname,len))
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

            return;

        }
        ++idx;
        }

}


void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // THIS LIBRARY DOES NOT DEFINE ANY OBJECTS
        Exceptions=EX_BADOPCODE;
        ExceptionPointer=IPtr;
        return;
    }

    switch(OPCODE(CurOpcode))
    {

    case CF:
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            BINT64 flag=rplReadNumberAsBINT(rplPeekData(1));

            if(flag<0 && flag>=-128) {
                if(!ISLIST(*SystemFlags)) {
                    // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
                    Exceptions|=EX_VARUNDEF;
                    ExceptionPointer=IPtr;
                    return;
                }
                //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
                WORDPTR low64=SystemFlags+2;
                WORDPTR hi64=SystemFlags+5;
                if(flag>=-32) low64[0]&=~(1 << -(flag+1));
                else if(flag>=-64) low64[1]&=~(1 << -(flag+33));
                else if(flag>=96) hi64[0]&=~(1 << -(flag+65));
                else hi64[1]&=~(1 << -(flag+97));
                rplDropData(1);
                return;
            }

            // USER FLAGS NOT SUPPORTED FOR NOW
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;

        }

        if(ISIDENT(*rplPeekData(1))) {

            WORDPTR id=rplPeekData(1);
            BINT idx=0;
            while(flags_names[idx].flagname) {
            if(rplCompareIDENTByName(id,(BYTEPTR)flags_names[idx].flagname,utf8len(flags_names[idx].flagname)))
            {
                BINT count;
                for(count=0;count<8;++count)
                {
                    if(flags_names[idx].flags[count]) {
                        BINT flag=flags_names[idx].flags[count]&0x7f;
                        //BINT value=flags_names[idx].flags[count]>>7;
                        if(!ISLIST(*SystemFlags)) {
                            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
                            Exceptions|=EX_VARUNDEF;
                            ExceptionPointer=IPtr;
                            return;
                        }
                        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
                        WORDPTR low64=SystemFlags+2;
                        WORDPTR hi64=SystemFlags+5;
                        // ALWAYS CLEAR THE FLAGS, REGARDLESS OF VALUE
                            if(flag<=32) low64[0]&=~(1 << (flag-1));
                            else if(flag<=64) low64[1]&=~(1 << (flag-33));
                            else if(flag<=96) hi64[0]&=~(1 << (flag-65));
                            else hi64[1]&=~(1 << (flag-97));

                    }
                }

                rplDropData(1);
                return;
            }
            ++idx;
            }

            // UNKNOWN IDENTIFIER FOR A FLAG
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;


        }

        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
        return;

    case SF:
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            BINT64 flag=rplReadNumberAsBINT(rplPeekData(1));

            if(flag<0 && flag>=-128) {
                if(!ISLIST(*SystemFlags)) {
                    // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
                    Exceptions|=EX_VARUNDEF;
                    ExceptionPointer=IPtr;
                    return;
                }
                //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
                WORDPTR low64=SystemFlags+2;
                WORDPTR hi64=SystemFlags+5;
                if(flag>=-32) low64[0]|=(1 << -(flag+1));
                else if(flag>=-64) low64[1]|=(1 << -(flag+33));
                else if(flag>=96) hi64[0]|=(1 << -(flag+65));
                else hi64[1]|=(1 << -(flag+97));
                rplDropData(1);
                return;
            }

            // USER FLAGS NOT SUPPORTED FOR NOW
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;

        }

        if(ISIDENT(*rplPeekData(1))) {

            WORDPTR id=rplPeekData(1);
            BINT idx=0;
            while(flags_names[idx].flagname) {
            if(rplCompareIDENTByName(id,(BYTEPTR)flags_names[idx].flagname,utf8len(flags_names[idx].flagname)))
            {
                BINT count;
                for(count=0;count<8;++count)
                {
                    if(flags_names[idx].flags[count]) {
                        BINT flag=flags_names[idx].flags[count]&0x7f;
                        BINT value=flags_names[idx].flags[count]>>7;
                        if(!ISLIST(*SystemFlags)) {
                            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
                            Exceptions|=EX_VARUNDEF;
                            ExceptionPointer=IPtr;
                            return;
                        }
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

                rplDropData(1);
                return;

            }
            ++idx;
            }

            // UNKNOWN IDENTIFIER FOR A FLAG
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;

        }
        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
        return;

    case FCTEST:
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            BINT64 flag=rplReadNumberAsBINT(rplPeekData(1));

            if(flag<0 && flag>=-128) {
                if(!ISLIST(*SystemFlags)) {
                    // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
                    Exceptions|=EX_VARUNDEF;
                    ExceptionPointer=IPtr;
                    return;
                }
                //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
                WORDPTR low64=SystemFlags+2;
                WORDPTR hi64=SystemFlags+5;
                BINT result;
                if(flag>=-32) result=low64[0]&(1 << -(flag+1));
                else if(flag>=-64) result=low64[1]&(1 << -(flag+33));
                else if(flag>=96) result=hi64[0]&(1 << -(flag+65));
                else result=hi64[1]&(1 << -(flag+97));
                if(result) rplOverwriteData(1,(WORDPTR)zero_bint);
                else rplOverwriteData(1,(WORDPTR)one_bint);
                return;
            }

            // USER FLAGS NOT SUPPORTED FOR NOW
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;

        }

        if(ISIDENT(*rplPeekData(1))) {

            WORDPTR id=rplPeekData(1);
            BINT idx=0;
            while(flags_names[idx].flagname) {
            if(rplCompareIDENTByName(id,(BYTEPTR)flags_names[idx].flagname,utf8len(flags_names[idx].flagname)))
            {
                BINT count;
                BINT match=1;
                for(count=0;count<8;++count)
                {
                    if(flags_names[idx].flags[count]) {
                        BINT flag=flags_names[idx].flags[count]&0x7f;
                        BINT value=flags_names[idx].flags[count]>>7;
                        if(!ISLIST(*SystemFlags)) {
                            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
                            Exceptions|=EX_VARUNDEF;
                            ExceptionPointer=IPtr;
                            return;
                        }
                        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
                        WORDPTR low64=SystemFlags+2;
                        WORDPTR hi64=SystemFlags+5;
                        BINT res;
                            if(flag<=32) res=(low64[0]>> (flag-1))&1;
                            else if(flag<=64) res=(low64[1]>>(flag-33))&1;
                            else if(flag<=96) res=(hi64[0]>>(flag-65))&1;
                            else res=(hi64[1]>> (flag-97))&1;
                            match&=!(value^res);
                    }
                }

                if(match) rplOverwriteData(1,(WORDPTR)zero_bint);
                else rplOverwriteData(1,(WORDPTR)one_bint);
                return;

            }
            ++idx;
            }

            // UNKNOWN IDENTIFIER FOR A FLAG
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;

        }

        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
        return;

    case FSTEST:
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            BINT64 flag=rplReadNumberAsBINT(rplPeekData(1));

            if(flag<0 && flag>=-128) {
                if(!ISLIST(*SystemFlags)) {
                    // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
                    Exceptions|=EX_VARUNDEF;
                    ExceptionPointer=IPtr;
                    return;
                }
                //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
                WORDPTR low64=SystemFlags+2;
                WORDPTR hi64=SystemFlags+5;
                BINT result;
                if(flag>=-32) result=low64[0]&(1 << -(flag+1));
                else if(flag>=-64) result=low64[1]&(1 << -(flag+33));
                else if(flag>=96) result=hi64[0]&(1 << -(flag+65));
                else result=hi64[1]&(1 << -(flag+97));
                if(result) rplOverwriteData(1,(WORDPTR)one_bint);
                else rplOverwriteData(1,(WORDPTR)zero_bint);
                return;
            }

            // USER FLAGS NOT SUPPORTED FOR NOW
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;

        }

        if(ISIDENT(*rplPeekData(1))) {

            WORDPTR id=rplPeekData(1);
            BINT idx=0;
            while(flags_names[idx].flagname) {
            if(rplCompareIDENTByName(id,(BYTEPTR)flags_names[idx].flagname,utf8len(flags_names[idx].flagname)))
            {
                BINT count;
                BINT match=1;
                for(count=0;count<8;++count)
                {
                    if(flags_names[idx].flags[count]) {
                        BINT flag=flags_names[idx].flags[count]&0x7f;
                        BINT value=flags_names[idx].flags[count]>>7;
                        if(!ISLIST(*SystemFlags)) {
                            // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
                            Exceptions|=EX_VARUNDEF;
                            ExceptionPointer=IPtr;
                            return;
                        }
                        //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
                        WORDPTR low64=SystemFlags+2;
                        WORDPTR hi64=SystemFlags+5;
                        BINT res;
                            if(flag<=32) res=(low64[0]>> (flag-1))&1;
                            else if(flag<=64) res=(low64[1]>>(flag-33))&1;
                            else if(flag<=96) res=(hi64[0]>>(flag-65))&1;
                            else res=(hi64[1]>> (flag-97))&1;
                            match&=!(value^res);
                    }
                }

                if(match) rplOverwriteData(1,(WORDPTR)one_bint);
                else rplOverwriteData(1,(WORDPTR)zero_bint);
                return;

            }
            ++idx;
            }

            // UNKNOWN IDENTIFIER FOR A FLAG
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;

        }


        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
        return;

    case FCTESTCLEAR:
        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            BINT64 flag=rplReadNumberAsBINT(rplPeekData(1));

            if(flag<0 && flag>=-128) {
                if(!ISLIST(*SystemFlags)) {
                    // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
                    Exceptions|=EX_VARUNDEF;
                    ExceptionPointer=IPtr;
                    return;
                }
                //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
                WORDPTR low64=SystemFlags+2;
                WORDPTR hi64=SystemFlags+5;
                BINT result;
                if(flag>=-32) { result=low64[0]&(1 << -(flag+1)); low64[0]&=~(1 << -(flag+1)); }
                else if(flag>=-64) { result=low64[1]&(1 << -(flag+33)); low64[1]&=~(1 << -(flag+33)); }
                else if(flag>=96) { result=hi64[0]&(1 << -(flag+65)); hi64[0]&=~(1 << -(flag+65)); }
                else { result=hi64[1]&(1 << -(flag+97)); hi64[1]&=~(1 << -(flag+97)); }
                if(result) rplOverwriteData(1,(WORDPTR)zero_bint);
                else rplOverwriteData(1,(WORDPTR)one_bint);
                return;
            }

            // USER FLAGS NOT SUPPORTED FOR NOW
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;

        }

        // TODO: ADD SUPPORT FOR NAMED FLAGS

        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
        return;

    case FSTESTCLEAR:

        if(rplDepthData()<1) {
            Exceptions|=EX_BADARGCOUNT;
            ExceptionPointer=IPtr;
            return;
        }
        if(ISNUMBER(*rplPeekData(1))) {
            // THIS IS A FLAG NUMBER
            BINT64 flag=rplReadNumberAsBINT(rplPeekData(1));

            if(flag<0 && flag>=-128) {
                if(!ISLIST(*SystemFlags)) {
                    // THIS IS FOR DEBUGGING ONLY, SYSTEM FLAGS SHOULD ALWAYS EXIST
                    Exceptions|=EX_VARUNDEF;
                    ExceptionPointer=IPtr;
                    return;
                }
                //SYSTEM FLAGS IS THE ONLY OBJECT THAT IS MODIFIED IN PLACE
                WORDPTR low64=SystemFlags+2;
                WORDPTR hi64=SystemFlags+5;
                BINT result;
                if(flag>=-32) { result=low64[0]&(1 << -(flag+1)); low64[0]&=~(1 << -(flag+1)); }
                else if(flag>=-64) { result=low64[1]&(1 << -(flag+33)); low64[1]&=~(1 << -(flag+33)); }
                else if(flag>=96) { result=hi64[0]&(1 << -(flag+65)); hi64[0]&=~(1 << -(flag+65)); }
                else { result=hi64[1]&(1 << -(flag+97)); hi64[1]&=~(1 << -(flag+97)); }
                if(result) rplOverwriteData(1,(WORDPTR)one_bint);
                else rplOverwriteData(1,(WORDPTR)zero_bint);
                return;
            }

            // USER FLAGS NOT SUPPORTED FOR NOW
            Exceptions|=EX_BADARGVALUE;
            ExceptionPointer=IPtr;
            return;

        }

        // TODO: ADD SUPPORT FOR NAMED FLAGS

        Exceptions|=EX_BADARGTYPE;
        ExceptionPointer=IPtr;
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
    Exceptions|=EX_BADOPCODE;
    ExceptionPointer=IPtr;
    return;


}





