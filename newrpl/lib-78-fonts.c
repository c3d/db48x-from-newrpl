/*
 * Copyright (c) 2016, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "libraries.h"
#include "fontlist.h"

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
#define LIBRARY_NUMBER  78

//@TITLE=Fonts

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    CMD(FNTSTO,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(FNTRCL,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(FNTPG,MKTOKENINFO(5,TITYPE_NOTALLOWED,1,2)), \
    CMD(FNTSTK,MKTOKENINFO(6,TITYPE_NOTALLOWED,1,2)), \
    CMD(FNT1STK,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(FNTMENU,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(FNTCMDL,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(FNTSTAT,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(FNTPLOT,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(FNTFORM,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STOFNTSTK,"→FNTSTK",MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STOFNT1STK,"→FNT1STK",MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STOFNTMENU,"→FNTMENU",MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STOFNTCMDL,"→FNTCMDL",MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STOFNTSTAT,"→FNTSTAT",MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STOFNTPLOT,"→FNTPLOT",MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STOFNTFORM,"→FNTFORM",MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    CMD(FNTHELP,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    CMD(FNTHLPT,MKTOKENINFO(7,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STOFNTHELP,"→FNTHELP",MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2)), \
    ECMD(STOFNTHLPT,"→FNTHLPT",MKTOKENINFO(8,TITYPE_NOTALLOWED,1,2))


// ADD MORE OPCODES HERE

#define ERROR_LIST \
    ERR(FONTEXPECTED,0), \
    ERR(RESERVEDNAME,1), \
    ERR(FONTNOTINSTALLED,2)

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
INCLUDE_ROMOBJECT(lib78_menu);

ROMOBJECT fnt5a_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'n', 't'),
    TEXT2WORD('5', 'A', 0, 0)
};

ROMOBJECT fnt5b_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'n', 't'),
    TEXT2WORD('5', 'B', 0, 0)
};

ROMOBJECT fnt5c_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'n', 't'),
    TEXT2WORD('5', 'C', 0, 0)
};

ROMOBJECT fnt6a_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'n', 't'),
    TEXT2WORD('6', 'A', 0, 0)
};

ROMOBJECT fnt6b_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'n', 't'),
    TEXT2WORD('6', 'B', 0, 0)
};

ROMOBJECT fnt7a_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'n', 't'),
    TEXT2WORD('7', 'A', 0, 0)
};

ROMOBJECT fnt8a_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'n', 't'),
    TEXT2WORD('8', 'A', 0, 0)
};

ROMOBJECT fnt8b_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'n', 't'),
    TEXT2WORD('8', 'B', 0, 0)
};

ROMOBJECT fnt8c_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'n', 't'),
    TEXT2WORD('8', 'C', 0, 0)
};

ROMOBJECT fnt8d_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'n', 't'),
    TEXT2WORD('8', 'D', 0, 0)
};

ROMOBJECT fnt10a_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'n', 't'),
    TEXT2WORD('1', '0', 'A', 0)
};

ROMOBJECT sysfonts_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('S', 'y', 's', 'F'),
    TEXT2WORD('o', 'n', 't', 's')
};

ROMOBJECT fontstack1_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('S', 't', 'k', '1'),
    TEXT2WORD('F', 'o', 'n', 't')
};

ROMOBJECT fontstack_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('S', 't', 'k', '2'),
    TEXT2WORD('F', 'o', 'n', 't')
};

ROMOBJECT fontmenu_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('M', 'e', 'n', 'u'),
    TEXT2WORD('F', 'o', 'n', 't')
};

ROMOBJECT fontform_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'r', 'm'),
    TEXT2WORD('F', 'o', 'n', 't')
};

ROMOBJECT fonthelptitle_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('H', 'l', 'p', 'T'),
    TEXT2WORD('F', 'o', 'n', 't')
};

ROMOBJECT fonthelp_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('H', 'e', 'l', 'p'),
    TEXT2WORD('F', 'o', 'n', 't')
};

ROMOBJECT fontcmdline_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('C', 'm', 'd', 'l'),
    TEXT2WORD('F', 'o', 'n', 't')
};

ROMOBJECT fontstarea_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('S', 't', 'a', 't'),
    TEXT2WORD('F', 'o', 'n', 't')
};

ROMOBJECT fontplot_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('P', 'l', 'o', 't'),
    TEXT2WORD('F', 'o', 'n', 't')
};

ROMOBJECT fnt18_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'n', 't'),
    TEXT2WORD('1', '8', 0, 0)
};

ROMOBJECT fnt24_ident[] = {
    MKPROLOG(DOIDENT, 2),
    TEXT2WORD('F', 'o', 'n', 't'),
    TEXT2WORD('2', '4', 0, 0)
};


// THIS LIBRARY DEPENDS ON THE FONTS INSTALLED IN THE FIRMWARE
// SO IT'S NOT HARDWARE-INDEPENDENT, BUT THE NAMES OF THE FONTS SHOULD BE CONSISTENT

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const WORDPTR const ROMPTR_TABLE[] = {
    (WORDPTR) LIB_MSGTABLE,
    (WORDPTR) LIB_HELPTABLE,

    (WORDPTR) lib78_menu,
    (WORDPTR) sysfonts_ident,

    // OTHER ROM OBJECTS INDEX 4-15

    (WORDPTR) fontstack_ident,
    (WORDPTR) fontstack1_ident,
    (WORDPTR) fontcmdline_ident,
    (WORDPTR) fontmenu_ident,
    (WORDPTR) fontstarea_ident,
    (WORDPTR) fontplot_ident,
    (WORDPTR) fontform_ident,
    (WORDPTR) fonthelp_ident,
    (WORDPTR) fonthelptitle_ident,
    (WORDPTR) zero_bint,
    (WORDPTR) zero_bint,
    (WORDPTR) zero_bint,

    // START OF ROM FONT NAME/OBJECT PAIRS INDEX 16-63

    (WORDPTR) fnt5a_ident,
    (WORDPTR) Font_5A,
    (WORDPTR) fnt5b_ident,
    (WORDPTR) Font_5B,
    (WORDPTR) fnt5c_ident,
    (WORDPTR) Font_5C,
    (WORDPTR) fnt6a_ident,
    (WORDPTR) Font_6A,
    (WORDPTR) fnt6b_ident,
    (WORDPTR) Font_6m,
    (WORDPTR) fnt7a_ident,
    (WORDPTR) Font_7A,
    (WORDPTR) fnt8a_ident,
    (WORDPTR) Font_8A,
    (WORDPTR) fnt8b_ident,
    (WORDPTR) Font_8B,
    (WORDPTR) fnt8c_ident,
    (WORDPTR) Font_8C,
    (WORDPTR) fnt8d_ident,
    (WORDPTR) Font_8D,
    (WORDPTR) fnt10a_ident,
    (WORDPTR) Font_10A,
    (WORDPTR) fnt18_ident,
    (WORDPTR) Font_18,
    (WORDPTR) fnt24_ident,
    (WORDPTR) Font_24,

    0, // ZERO PAIR AS FINALIZER
    0
};

WORDPTR const *rplGetFontRomPtrTableAddress(void)
{
    return ROMPTR_TABLE;
}

// FIND A SYSTEM FONT BY NAME

WORDPTR rplGetSystemFont(WORDPTR ident)
{
    // CHECK FOR RESERVED ROM NAMES FIRST
    int k;

    for(k = START_ROMPTR_INDEX; k < ROMLIB_MAX_SIZE; k += 2) {
        if (ROMPTR_TABLE[k] == NULL)
            break;
        if(rplCompareIDENT(ident, ROMPTR_TABLE[k]))
            return ROMPTR_TABLE[k + 1];
    }

    // CHECK FOR USER INSTALLED FONTS

    WORDPTR fontlist = rplGetSettings((WORDPTR) sysfonts_ident);

    if(!fontlist)
        return 0;
    if(!ISLIST(*fontlist))
        return 0;

    WORDPTR name = fontlist + 1, endofobj = rplSkipOb(fontlist);

    while((*name != CMD_ENDLIST) && (name < endofobj)) {
        if(rplCompareIDENT(ident, name))
            return rplSkipOb(name);
        name = rplSkipOb(name);
        name = rplSkipOb(name);
    }

    return 0;   // NOT FOUND
}

// FIND A FONT NAME BY OBJECT
WORDPTR rplGetSystemFontName(WORDPTR font)
{
    // CHECK FOR RESERVED ROM NAMES FIRST
    int k;

    for(k = START_ROMPTR_INDEX; k < ROMLIB_MAX_SIZE; k += 2) {
        if (ROMPTR_TABLE[k] == NULL)
            break;
        if(font == ROMPTR_TABLE[k + 1])
            return ROMPTR_TABLE[k];
    }

    // CHECK FOR USER INSTALLED FONTS

    WORDPTR fontlist = rplGetSettings((WORDPTR) sysfonts_ident);

    if(!fontlist)
        return 0;
    if(!ISLIST(*fontlist))
        return 0;

    WORDPTR name = fontlist + 1, endofobj = rplSkipOb(fontlist);

    while((*name != CMD_ENDLIST) && (name < endofobj)) {
        if(rplSkipOb(name) == font)
            return name;
        name = rplSkipOb(name);
        name = rplSkipOb(name);
    }

    return 0;   // NOT FOUND
}

void rplFontsNewList(WORDPTR oldlist, WORDPTR newlist)
{

    if(oldlist) {
        WORDPTR endoldlst = rplSkipOb(oldlist);
        WORDPTR font, fontname;
        WORDPTR fntid[FONTS_NUM] = {
            (WORDPTR) fontstack_ident,
            (WORDPTR) fontstack1_ident,
            (WORDPTR) fontcmdline_ident,
            (WORDPTR) fontmenu_ident,
            (WORDPTR) fontstarea_ident,
            (WORDPTR) fontplot_ident,
            (WORDPTR) fontform_ident,
            (WORDPTR) fonthelp_ident,
            (WORDPTR) fonthelptitle_ident
        };

        int k;
        for(k = 0; k < FONTS_NUM; ++k) {
            font = rplGetSettings(fntid[k]);
            if((font >= oldlist) && (font < endoldlst)) {
                // MOVE THIS FONT TO THE NEW LIST
                fontname = rplGetSystemFontName(font);
                if(!fontname)
                    rplPurgeSettings(fntid[k]);
                else {
                    // FIND THE NAME IN THE NEW LIST
                    WORDPTR ptr = newlist + 1, endnewlst = rplSkipOb(newlist);
                    while((*ptr != CMD_ENDLIST) && (ptr < endnewlst)) {
                        if(rplCompareIDENT(ptr, fontname)) {
                            // FOUND IT
                            font = rplSkipOb(ptr);
                            if(!ISFONT(*font))
                                rplPurgeSettings(fntid[k]);
                            else
                                rplStoreSettings(fntid[k], font);
                            break;
                        }
                        ptr = rplSkipOb(ptr);
                        ptr = rplSkipOb(ptr);
                    }

                    if((*ptr == CMD_ENDLIST) || (ptr >= endnewlst))
                        rplPurgeSettings(fntid[k]);

                }

            }
        }

    }

    // ALL FONTS POINTERS MOVED TO THE NEW LIST, NOW REPLACE THE LIST WITH THE NEW ONE
    rplStoreSettings((WORDPTR) sysfonts_ident, newlist);

}

// ADD/REPLACE A SYSTEM FONT, RETURN TRUE
void rplAddSystemFont(WORDPTR ident, WORDPTR font)
{
    if(!ISFONT(*font))
        return;
    if(!ISIDENT(*ident))
        return;

    // MAKE SURE THE IDENT IS NOT A RESERVED NAME
    // CHECK FOR RESERVED ROM NAMES FIRST
    int k;

    for(k = START_ROMPTR_INDEX; k < ROMLIB_MAX_SIZE; k += 2) {
        if (ROMPTR_TABLE[k] == NULL)
            break;
        if(rplCompareIDENT(ident, ROMPTR_TABLE[k])) {
            rplError(ERR_RESERVEDNAME);
            return;

        }

    }

    WORDPTR fontlist =
            rplGetSettings((WORDPTR) sysfonts_ident), oldfont, endofobj;
    BINT newsize;

    if(fontlist) {
        endofobj = rplSkipOb(fontlist);
        oldfont = fontlist + 1;

        while((*oldfont != CMD_ENDLIST) && (oldfont < endofobj)) {
            if(rplCompareIDENT(ident, oldfont))
                break;
            oldfont = rplSkipOb(oldfont);
            oldfont = rplSkipOb(oldfont);
        }

        if((*oldfont == CMD_ENDLIST) || (oldfont >= endofobj))
            oldfont = 0;
        else {
            oldfont = rplSkipOb(oldfont);
            if(!ISFONT(*oldfont))
                oldfont = 0;
        }
    }
    else
        oldfont = 0;

    if(fontlist)
        newsize = rplObjSize(fontlist) - 1;
    else
        newsize = 1;

    if(!oldfont)
        newsize += rplObjSize(ident);
    else
        newsize -= rplObjSize(oldfont);
    newsize += rplObjSize(font);
    // PROTECT ALL POINTERS

    ScratchPointer1 = ident;
    ScratchPointer2 = font;
    ScratchPointer3 = oldfont;
    ScratchPointer4 = fontlist;

    WORDPTR newlist = rplAllocTempOb(newsize);
    if(!newlist)
        return;

    ident = ScratchPointer1;
    font = ScratchPointer2;
    oldfont = ScratchPointer3;
    fontlist = ScratchPointer4;

    if(fontlist) {
        endofobj = rplSkipOb(fontlist);
        if(oldfont) {
            memmovew(newlist + 1, fontlist + 1, oldfont - fontlist - 1);
            memmovew(newlist + (oldfont - fontlist), font, rplObjSize(font));
            memmovew(newlist + (oldfont - fontlist) + rplObjSize(font),
                    rplSkipOb(oldfont),
                    rplSkipOb(fontlist) - rplSkipOb(oldfont));
        }
        else {
            memmovew(newlist + 1, fontlist + 1, rplObjSize(fontlist) - 2);
            memmovew(newlist + rplObjSize(fontlist) - 1, ident,
                    rplObjSize(ident));
            memmovew(newlist + rplObjSize(fontlist) - 1 + rplObjSize(ident),
                    font, rplObjSize(font));
        }
    }
    else {
        memmovew(newlist + 1, ident, rplObjSize(ident));
        memmovew(newlist + 1 + rplObjSize(ident), font, rplObjSize(font));
    }

    // CLOSE THE LIST
    newlist[0] = MKPROLOG(DOLIST, newsize);
    newlist[newsize] = CMD_ENDLIST;

    rplFontsNewList(fontlist, newlist);

}

// PURGE A SYSTEM FONT
void rplPurgeSystemFont(WORDPTR ident)
{
    if(!ISIDENT(*ident))
        return;

    // MAKE SURE THE IDENT IS NOT A RESERVED NAME
    // CHECK FOR RESERVED ROM NAMES FIRST
    int k;

    for(k = START_ROMPTR_INDEX; k < ROMLIB_MAX_SIZE; k += 2) {
        if (ROMPTR_TABLE[k] == NULL)
            break;
        if(rplCompareIDENT(ident, ROMPTR_TABLE[k])) {
            rplError(ERR_RESERVEDNAME);
            return;

        }

    }

    WORDPTR fontlist =
            rplGetSettings((WORDPTR) sysfonts_ident), oldfont, endofobj =
            rplSkipOb(fontlist);
    BINT newsize, oldsize;

    if(fontlist) {
        oldfont = fontlist + 1;

        while((*oldfont != CMD_ENDLIST) && (oldfont < endofobj)) {
            if(rplCompareIDENT(ident, oldfont))
                break;
            oldfont = rplSkipOb(oldfont);
            oldfont = rplSkipOb(oldfont);
        }

        if((*oldfont == CMD_ENDLIST) || (oldfont >= endofobj))
            oldfont = 0;
    }
    else
        return;

    if(!oldfont)
        return;

    newsize = rplObjSize(fontlist) - 1;

    oldsize = rplSkipOb(rplSkipOb(oldfont)) - oldfont;

    newsize -= oldsize;
    // PROTECT ALL POINTERS

    ScratchPointer1 = ident;
    ScratchPointer2 = oldfont;
    ScratchPointer3 = fontlist;

    WORDPTR newlist = rplAllocTempOb(newsize);
    if(!newlist)
        return;

    ident = ScratchPointer1;
    oldfont = ScratchPointer2;
    fontlist = ScratchPointer3;
    endofobj = rplSkipOb(fontlist);
    WORDPTR endold = rplSkipOb(rplSkipOb(oldfont));
    if(endold > endofobj - 1)
        endold = endofobj - 1;

    memmovew(newlist + 1, fontlist + 1, oldfont - fontlist - 1);
    memmovew(newlist + (oldfont - fontlist), endold,
            rplSkipOb(fontlist) - endold);

    // CLOSE THE LIST
    newlist[0] = MKPROLOG(DOLIST, newsize);
    newlist[newsize] = CMD_ENDLIST;

    rplStoreSettings((WORDPTR) sysfonts_ident, newlist);

}

// CHANGE THE CURRENT FONT
void rplSetCurrentFont(BINT area, WORDPTR ident)
{
    WORDPTR font = rplGetSystemFont(ident);
    if(!font) {
        rplError(ERR_FONTNOTINSTALLED);
        return;
    }
    WORDPTR fntid;
    switch (area) {
    case FONT_STACK:
        fntid = (WORDPTR) fontstack_ident;
        break;
    case FONT_STACKLVL1:
        fntid = (WORDPTR) fontstack1_ident;
        break;
    case FONT_CMDLINE:
        fntid = (WORDPTR) fontcmdline_ident;
        break;
    case FONT_MENU:
        fntid = (WORDPTR) fontmenu_ident;
        break;
    case FONT_STATUS:
        fntid = (WORDPTR) fontstarea_ident;
        break;
    case FONT_PLOT:
        fntid = (WORDPTR) fontplot_ident;
        break;
    case FONT_FORMS:
        fntid = (WORDPTR) fontform_ident;
        break;
    case FONT_HLPTEXT:
        fntid = (WORDPTR) fonthelp_ident;
        break;
    case FONT_HLPTITLE:
        fntid = (WORDPTR) fonthelptitle_ident;
        break;

    default:
        return;
    }

    rplStoreSettings((WORDPTR) fntid, font);

}

WORDPTR rplGetCurrentFont(BINT area)
{
    WORDPTR fntid;
    switch (area) {
    case FONT_STACK:
        fntid = (WORDPTR) fontstack_ident;
        break;
    case FONT_STACKLVL1:
        fntid = (WORDPTR) fontstack1_ident;
        break;
    case FONT_CMDLINE:
        fntid = (WORDPTR) fontcmdline_ident;
        break;
    case FONT_MENU:
        fntid = (WORDPTR) fontmenu_ident;
        break;
    case FONT_STATUS:
        fntid = (WORDPTR) fontstarea_ident;
        break;
    case FONT_PLOT:
        fntid = (WORDPTR) fontplot_ident;
        break;
    case FONT_FORMS:
        fntid = (WORDPTR) fontform_ident;
        break;
    case FONT_HLPTEXT:
        fntid = (WORDPTR) fonthelp_ident;
        break;
    case FONT_HLPTITLE:
        fntid = (WORDPTR) fonthelptitle_ident;
        break;
    default:
        return 0;
    }

    fntid = rplGetSettings(fntid);

    if(!fntid) {
        // RETURN A DEFAULT SYSTEM FONT
        switch (area) {
        case FONT_STACK:
            return (WORDPTR) fnt8c_ident;
        case FONT_STACKLVL1:
            return (WORDPTR) fnt8c_ident;
        case FONT_CMDLINE:
            return (WORDPTR) fnt8c_ident;
        case FONT_MENU:
            return (WORDPTR) fnt6a_ident;
        case FONT_STATUS:
            return (WORDPTR) fnt6a_ident;
        case FONT_PLOT:
            return (WORDPTR) fnt6a_ident;
        case FONT_FORMS:
            return (WORDPTR) fnt8c_ident;
        default:
            return 0;
        }

    }

    return rplGetSystemFontName(fntid);
}

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // JUST PUSH THE OBJECT ON THE STACK
        rplPushData(IPtr);
        return;
    }

    // PROCESS OVERLOADED OPERATORS FIRST
    if(LIBNUM(CurOpcode) == LIB_OVERLOADABLE) {

        if(ISUNARYOP(CurOpcode)) {
            // APPLY UNARY OPERATOR DIRECTLY TO THE CONTENTS OF THE VARIABLE
            switch (OPCODE(CurOpcode)) {
            case OVR_FUNCEVAL:
            case OVR_EVAL1:
            case OVR_EVAL:
            case OVR_XEQ:
                // JUST KEEP THE OBJECT ON THE STACK, UNEVALUATED
                if(!ISPROLOG(*rplPeekData(1))) {
                    WORD saveOpcode = CurOpcode;
                    CurOpcode = *rplPopData();
                    // RECURSIVE CALL
                    LIB_HANDLER();
                    CurOpcode = saveOpcode;
                    return;
                }

                return;
            case OVR_ISTRUE:
                rplOverwriteData(1, (WORDPTR) one_bint);
                return;

            default:
                // BY DEFAULT, ISSUE A BAD OPCODE ERROR
                rplError(ERR_INVALIDOPCODE);
                return;

            }

        }       // END OF UNARY OPERATORS

        if(ISBINARYOP(CurOpcode)) {

            switch (OPCODE(CurOpcode)) {
            case OVR_SAME:
            {
                BINT same = rplCompareObjects(rplPeekData(1), rplPeekData(2));
                rplDropData(2);
                if(same)
                    rplPushTrue();
                else
                    rplPushFalse();
                return;
            }
            default:
                // BY DEFAULT, ISSUE A BAD OPCODE ERROR
                rplError(ERR_INVALIDOPCODE);

                return;

            }
        }

    }

    switch (OPCODE(CurOpcode)) {

    case FNTSTO:
        // INSTALL A NEW USER FONT
    {
        //@SHORT_DESC=Install a user font for system use
        //@NEW
        if(rplDepthData() < 2) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(2);
        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }
        if(!ISFONT(*rplPeekData(2))) {
            rplError(ERR_FONTEXPECTED);
            return;
        }

        rplAddSystemFont(rplPeekData(1), rplPeekData(2));
        rplDropData(2);
        return;
    }

    case FNTRCL:
        // RECALL A SYSTEM FONT
    {
        //@SHORT_DESC=Recall a system font
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }
        WORDPTR fnt = rplGetSystemFont(rplPeekData(1));
        if(!fnt) {
            rplError(ERR_FONTNOTINSTALLED);
            return;
        }
        rplOverwriteData(1, fnt);
        return;
    }

    case FNTPG:
        // PURGE A USER INSTALLED FONT
    {
        //@SHORT_DESC=Purge a user-installed system font
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }
        rplPurgeSystemFont(rplPeekData(1));
        rplDropData(1);
        return;
    }

    case FNTSTK:
    {
        //@SHORT_DESC=Recall name of current font for stack area
        //@NEW
        WORDPTR fntid = rplGetCurrentFont(FONT_STACK);
        if(fntid)
            rplPushData(fntid);
        return;
    }
    case FNT1STK:
    {
        //@SHORT_DESC=Recall name of current font for stack level 1
        //@NEW
        WORDPTR fntid = rplGetCurrentFont(FONT_STACKLVL1);
        if(fntid)
            rplPushData(fntid);
        return;
    }
    case FNTMENU:
    {
        //@SHORT_DESC=Recall name of current font for menu area
        //@NEW
        WORDPTR fntid = rplGetCurrentFont(FONT_MENU);
        if(fntid)
            rplPushData(fntid);
        return;
    }
    case FNTCMDL:
    {
        //@SHORT_DESC=Recall name of current font for command line area
        //@NEW
        WORDPTR fntid = rplGetCurrentFont(FONT_CMDLINE);
        if(fntid)
            rplPushData(fntid);
        return;
    }
    case FNTSTAT:
    {
        //@SHORT_DESC=Recall name of current font for status area
        //@NEW
        WORDPTR fntid = rplGetCurrentFont(FONT_STATUS);
        if(fntid)
            rplPushData(fntid);
        return;
    }
    case FNTPLOT:
    {
        //@SHORT_DESC=Recall name of current font for plot objects
        //@NEW

        WORDPTR fntid = rplGetCurrentFont(FONT_PLOT);
        if(fntid)
            rplPushData(fntid);
        return;
    }
    case FNTFORM:
    {
        //@SHORT_DESC=Recall name of current font for forms
        //@NEW
        WORDPTR fntid = rplGetCurrentFont(FONT_FORMS);
        if(fntid)
            rplPushData(fntid);
        return;
    }
    case STOFNTSTK:
        // CHANGE CURRENT FONT
    {
        //@SHORT_DESC=Change current font for stack area
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }
        rplSetCurrentFont(FONT_STACK, rplPeekData(1));
        rplDropData(1);
        return;
    }

    case STOFNT1STK:
        // CHANGE CURRENT FONT
    {
        //@SHORT_DESC=Change current font for stack level 1
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }
        rplSetCurrentFont(FONT_STACKLVL1, rplPeekData(1));
        rplDropData(1);
        return;
    }

    case STOFNTMENU:
        // CHANGE CURRENT FONT
    {
        //@SHORT_DESC=Change current font for menu area
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }
        rplSetCurrentFont(FONT_MENU, rplPeekData(1));
        rplDropData(1);
        return;
    }

    case STOFNTCMDL:
        // CHANGE CURRENT FONT
    {
        //@SHORT_DESC=Change current font for command line area
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }
        rplSetCurrentFont(FONT_CMDLINE, rplPeekData(1));
        rplDropData(1);
        return;
    }

    case STOFNTSTAT:
        // CHANGE CURRENT FONT
    {
        //@SHORT_DESC=Change current font for status area
        //@NEW
        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }
        rplSetCurrentFont(FONT_STATUS, rplPeekData(1));
        rplDropData(1);
        return;
    }

    case STOFNTPLOT:
        // CHANGE CURRENT FONT
    {
        //@SHORT_DESC=Change current font for plot objects
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }
        rplSetCurrentFont(FONT_PLOT, rplPeekData(1));
        rplDropData(1);
        return;
    }

    case STOFNTFORM:
        // CHANGE CURRENT FONT
    {
        //@SHORT_DESC=Change current font for forms
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }
        rplSetCurrentFont(FONT_FORMS, rplPeekData(1));
        rplDropData(1);
        return;
    }


    case FNTHELP:
    {
        //@SHORT_DESC=Recall name of current font for help
        //@NEW
        WORDPTR fntid = rplGetCurrentFont(FONT_HLPTEXT);
        if(fntid)
            rplPushData(fntid);
        return;
    }
    case FNTHLPT:
    {
        //@SHORT_DESC=Recall name of current font for help title
        //@NEW
        WORDPTR fntid = rplGetCurrentFont(FONT_HLPTITLE);
        if(fntid)
            rplPushData(fntid);
        return;
    }

    case STOFNTHELP:
        // CHANGE CURRENT FONT
    {
        //@SHORT_DESC=Change current font for help text
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }
        rplSetCurrentFont(FONT_HLPTEXT, rplPeekData(1));
        rplDropData(1);
        return;
    }
    case STOFNTHLPT:
        // CHANGE CURRENT FONT
    {
        //@SHORT_DESC=Change current font for help title
        //@NEW

        if(rplDepthData() < 1) {
            rplError(ERR_BADARGCOUNT);
            return;
        }
        rplStripTagStack(1);
        if(!ISIDENT(*rplPeekData(1))) {
            rplError(ERR_IDENTEXPECTED);
            return;
        }
        rplSetCurrentFont(FONT_HLPTITLE, rplPeekData(1));
        rplDropData(1);
        return;
    }



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

        if((TokenLen == 8)
                && (!utf8ncmp2((char *)TokenStart, (char *)BlankStart,
                        "FONTDATA", 8))) {

            ScratchPointer4 = CompileEnd;
            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER, 0));
            RetNum = OK_NEEDMORE;
            return;
        }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER, (char **)LIB_NAMES, NULL,
                LIB_NUMBEROFCMDS);
        return;
    case OPCODE_COMPILECONT:
    {
        if(OBJSIZE(*ScratchPointer4) == 0) {
            // NEED TO OBTAIN THE SIZE IN WORDS FIRST
            // GIVEN AS A HEX NUMBER

            if((BINT) TokenLen != (BYTEPTR) BlankStart - (BYTEPTR) TokenStart) {
                // THERE'S UNICODE CHARACTERS IN BETWEEN, THAT MAKES IT AN INVALID STRING
                RetNum = ERR_SYNTAX;
                return;
            }

            BYTEPTR ptr = (BYTEPTR) TokenStart;
            WORD value = 0;
            BINT digit;
            while(ptr < (BYTEPTR) BlankStart) {
                if((*ptr >= '0') && (*ptr <= '9'))
                    digit = *ptr - '0';
                else if((*ptr >= 'A') && (*ptr <= 'F'))
                    digit = *ptr - 'A' + 10;
                else if((*ptr >= 'a') && (*ptr <= 'f'))
                    digit = *ptr - 'a' + 10;
                else {
                    RetNum = ERR_SYNTAX;
                    return;
                }
                value <<= 4;
                value |= digit;
                ++ptr;
            }

            // WE GOT THE PAYLOAD SIZE IN WORDS
            if(value > 0x3ffff) {
                RetNum = ERR_INVALID;
                return;
            }

            *ScratchPointer4 = MKPROLOG(LIBRARY_NUMBER, value);
            RetNum = OK_NEEDMORE;
            return;

        }

        // WE HAVE A SIZE
        // DO WE NEED ANY MORE DATA?

        BYTEPTR ptr = (BYTEPTR) TokenStart;

        WORD value = 0;
        WORD checksum = 0;
        BINT ndigits = 0;
        BINT dig;

        if(LIBNUM(*ScratchPointer4) & 1) {
            // CONTINUE WHERE WE LEFT OFF
            --CompileEnd;
            ndigits = (*CompileEnd) & 0xffff;
            checksum = (*CompileEnd) >> 16;
            --CompileEnd;
            value = *CompileEnd;
            *ScratchPointer4 &= ~0x00100000;
        }

        while((CompileEnd - ScratchPointer4 - 1) <
                (BINT) OBJSIZE(*ScratchPointer4)) {
            do {
                if((*ptr >= '0') && (*ptr <= '9'))
                    dig = (*ptr + 4);
                else if((*ptr >= 'A') && (*ptr <= 'Z'))
                    dig = (*ptr - 65);
                else if((*ptr >= 'a') && (*ptr <= 'z'))
                    dig = (*ptr - 71);
                else if(*ptr == '#')
                    dig = 62;
                else if(*ptr == '$')
                    dig = 63;
                else {
                    // INVALID CHARACTER!
                    RetNum = ERR_SYNTAX;
                    return;
                }

                // STILL NEED MORE WORDS, KEEP COMPILING
                if(ndigits == 5) {
                    value <<= 2;

                    value |= dig & 3;
                    checksum += dig & 3;
                    if((checksum & 0xf) != ((dig >> 2) & 0xf)) {
                        RetNum = ERR_INVALID;
                        return;
                    }
                    // CHECKSUM PASSED, IT'S A VALID WORD
                    rplCompileAppend(value);
                    value = 0;
                    ndigits = 0;
                    checksum = 0;
                }
                else {
                    value <<= 6;
                    value |= dig;
                    checksum += (dig & 3) + ((dig >> 2) & 3) + ((dig >> 4) & 3);
                    ++ndigits;
                }
                ++ptr;
            }
            while(ptr != (BYTEPTR) BlankStart);
            if(ndigits
                    || (((CompileEnd - ScratchPointer4 - 1) <
                            (BINT) OBJSIZE(*ScratchPointer4)))) {
                // INCOMPLETE WORD, PREPARE FOR RESUME ON NEXT TOKEN
                rplCompileAppend(value);
                rplCompileAppend(ndigits | (checksum << 16));
                *ScratchPointer4 |= 0x00100000;
                RetNum = OK_NEEDMORE;
                return;
            }
            else
                *ScratchPointer4 = MKPROLOG(DOFONT, OBJSIZE(*ScratchPointer4));

        }

        RetNum = OK_CONTINUE;
        return;
    }
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors
        if(ISPROLOG(*DecompileObject)) {
            // DECOMPILE FONT

            rplDecompAppendString((BYTEPTR) "FONTDATA ");
            BINT size = OBJSIZE(*DecompileObject);
            BINT k, zero = 1, nibble;
            for(k = 4; k >= 0; --k) {
                nibble = (size >> (k * 4)) & 0xf;
                if(!zero || nibble) {
                    nibble += 48;
                    if(nibble >= 58)
                        nibble += 7;
                    rplDecompAppendChar(nibble);
                    zero = 0;
                }
            }

            rplDecompAppendChar(' ');

            // OUTPUT THE DATA BY WORDS, WITH FOLLOWING ENCODING:
            // 32-BIT WORDS GO ENCODED IN 6 TEXT CHARACTERS
            // EACH CHARACTER CARRIES 6-BITS IN BASE64 ENCONDING
            // MOST SIGNIFICANT 6-BIT PACKET GOES FIRST
            // LAST PACKET HAS 2 LSB BITS TO COMPLETE THE 32-BIT WORDS
            // AND 4-BIT CHECKSUM. THE CHECKSUM IS THE SUM OF THE (16) 2-BIT PACKS IN THE WORD, MODULO 15

            BYTE encoder[7];

            encoder[6] = 0;

            WORDPTR ptr = DecompileObject + 1;
            BINT nwords = 0;

            while(size) {
                // ENCODE THE 6 CHARACTERS
                int k;
                BINT chksum = 0;
                for(k = 0; k < 5; ++k) {
                    encoder[k] = ((*ptr) >> (26 - 6 * k)) & 0x3f;
                    chksum +=
                            (encoder[k] & 3) + ((encoder[k] >> 2) & 3) +
                            ((encoder[k] >> 4) & 3);
                }
                encoder[5] = (*ptr) & 3;
                chksum += *ptr & 3;
                encoder[5] |= (chksum & 0xf) << 2;

                // NOW CONVERT TO BASE64
                for(k = 0; k < 6; ++k) {
                    if(encoder[k] < 26)
                        encoder[k] += 65;
                    else if(encoder[k] < 52)
                        encoder[k] += 71;
                    else if(encoder[k] < 62)
                        encoder[k] -= 4;
                    else if(encoder[k] == 62)
                        encoder[k] = '#';
                    else
                        encoder[k] = '$';
                }

                ScratchPointer1 = ptr;
                rplDecompAppendString(encoder);
                if(Exceptions) {
                    RetNum = ERR_INVALID;
                    return;
                }
                ptr = ScratchPointer1;

                ++ptr;
                ++nwords;
                if(nwords == 8) {
                    rplDecompAppendChar(' ');
                    nwords = 0;
                }

                --size;

            }

            RetNum = OK_CONTINUE;
            return;

        }

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
        libProbeCmds((char **)LIB_NAMES, (BINT *) LIB_TOKENINFO,
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
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL BINT, .42 = HEX INTEGER

        if(ISPROLOG(*ObjectPTR)) {
            TypeInfo = LIBRARY_NUMBER * 100;
            DecompHints = 0;
            RetNum = OK_TOKENINFO | MKTOKENINFO(0, TITYPE_NOTALLOWED, 0, 1);
        }
        else {
            TypeInfo = 0;       // ALL COMMANDS ARE TYPE 0
            DecompHints = 0;
            libGetInfo2(*ObjectPTR, (char **)LIB_NAMES, (BINT *) LIB_TOKENINFO,
                    LIB_NUMBEROFCMDS);
        }
        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, ObjectIDHash=hash, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER, (WORDPTR *) ROMPTR_TABLE, ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID, ObjectIDHash=hash
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((WORDPTR *) ROMPTR_TABLE, ObjectID, ObjectIDHash);
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        //if(ISPROLOG(*ObjectPTR)) { RetNum=ERR_INVALID; return; }

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
        if(MENUNUMBER(MenuCodeArg) > 0) {
            RetNum = ERR_NOTMINE;
            return;
        }
        ObjectPTR = (WORDPTR) lib78_menu;
        RetNum = OK_CONTINUE;
        return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp, (WORDPTR) LIB_HELPTABLE);
        return;
    }
    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {

        libFindMsg(LibError, (WORDPTR) LIB_MSGTABLE);
        return;
    }

    case OPCODE_LIBINSTALL:
        LibraryList = (WORDPTR) libnumberlist;
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
