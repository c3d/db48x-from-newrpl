/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <cmdcodes.h>
#include <libraries.h>
#include <newrpl.h>
#include <sysvars.h>
#include <ui.h>

// SOFT MENU API

// RETURN THE NUMBER OF ITEMS IN A MENU

int32_t uiCountMenuItems(WORD MenuCode, word_p menu)
{
    if(MENUSPECIAL(MenuCode) == MENU_VARS) {
        // MENU IS VARS
        return rplGetVisibleVarCount();
    }
    if(MENUSPECIAL(MenuCode) == MENU_USERLIB) {
        // MENU IS LIBS
        word_p libdir = rplGetSettings((word_p) library_dirname);

        if(!libdir)
            return 0;

        word_p *direntry = rplFindDirbyHandle(libdir);

        if(!direntry)
            return 0;

        return rplGetVisibleVarCountInDir(direntry);
    }

    if(!menu)
        return 0;
    if(ISLIST(*menu))
        return rplListLength(menu);
    return 1;
}

word_p uiGetLibObject(int32_t libnum, WORD arg2, WORD arg3, WORD Opcode)
{
    LIBHANDLER han = rplGetLibHandler(libnum);
    if(!han)
        return 0;
    WORD SavedOpcode = CurOpcode;
    int32_t SavedException = Exceptions;
    int32_t SavedErrorCode = ErrorCode;

    Exceptions = 0;     // ERASE ANY PREVIOUS ERROR TO ALLOW THE LIBRARY TO RUN
    CurOpcode = MKOPCODE(libnum, Opcode);
    ArgNum2 = arg2;
    ArgNum3 = arg3;
    RetNum = -1;
    (*han) ();

    Exceptions = SavedException;
    ErrorCode = SavedErrorCode;
    CurOpcode = SavedOpcode;

    if(RetNum != OK_CONTINUE)
        return 0;

    return ObjectPTR;

}

// GET A MENU OBJECT FROM A MENU CODE

word_p uiGetLibMenu(int64_t MenuCode)
{
    if(MENUSPECIAL(MenuCode) == MENU_VARS) {
        // MENU IS VARS, NO NEED FOR MENU OBJECT
        return 0;
    }
    if(MENUSPECIAL(MenuCode) == MENU_USERLIB) {
        // MENU IS LIBS, NO NEED FOR MENU OBJECT
        return 0;
    }
    return uiGetLibObject(MENULIBRARY(MenuCode), (MenuCode >> 32), MenuCode,
            OPCODE_LIBMENU);

}

word_p uiGetLibHelp(word_p Object)
{
    WORD hash = (ISPROLOG(*Object)) ? libComputeHash(Object) : *Object;
    return uiGetLibObject(LIBNUM(*Object), 0, hash, OPCODE_LIBHELP);
}

word_p uiGetLibPtrHelp(word_p LibCommand)
{

    return uiGetLibObject(LIBNUM(*LibCommand), LibCommand[1], MKOPCODE(DOLIBPTR,
                LibCommand[2]), OPCODE_LIBHELP);
}

word_p uiGetLibMsg(WORD MsgCode)
{
    return uiGetLibObject(LIBFROMMSG(MsgCode), 0, MsgCode, OPCODE_LIBMSG);
}

// RETURN A POINTER TO A MENU ITEM OBJECT
// FIRST ITEM = NUMBER 0

word_p uiGetMenuItem(int64_t MenuCode, word_p menu, int32_t item)
{
    if(MENUSPECIAL(MenuCode) == MENU_VARS) {
// MENU IS VARS
// RETURN A POINTER TO THE VARIABLE NAME

        word_p *var = rplFindVisibleGlobalByIndex(item);
        if(!var)
            return 0;
        return var[0];
    }
    if(MENUSPECIAL(MenuCode) == MENU_USERLIB) {
// MENU IS LIBS
// RETURN A POINTER TO THE VARIABLE NAME
        word_p libdir = rplGetSettings((word_p) library_dirname);

        if(!libdir)
            return 0;

        word_p *direntry = rplFindDirbyHandle(libdir);

        if(!direntry)
            return 0;

        word_p *var = rplFindVisibleGlobalByIndexInDir(item, direntry);
        if(!var)
            return 0;
        return var[1];
    }

    if(!menu)
        return 0;

    if(!ISLIST(*menu)) {
        if(item != 0)
            return 0;
        return menu;
    }

    word_p ptr = menu + 1, end = rplSkipOb(menu);

    while(ptr < end - 1) {
        if(!item)
            return ptr;
        ptr = rplSkipOb(ptr);
        --item;
    }
    return 0;
}

// GET THE ACTION OBJECT OF A MENU ITEM

word_p uiGetMenuItemAction(word_p item, int32_t shift)
{

    if(!item)
        return 0;

    if(!ISLIST(*item))
        return item;

    // GET ACTION ITEM WITHIN THE ITEM

    item = rplGetListElement(item, 2);

    if(!item)
        return 0;

    if(!ISLIST(*item))
        return item;

    int nactions = rplListLength(item);

    int index;
    switch (KM_SHIFT(shift)) {
    case 0:
        index = 0;
        break;
    case KSHIFT_LEFT:
        if(nactions < 2) {
            index = 0;
            break;
        }
        index = 1;
        break;
    case KSHIFT_RIGHT:
        if(nactions < 2) {
            index = 0;
            break;
        }
        if(nactions < 3) {
            index = 1;  // SINGLE-SHIFT SPEC
            break;
        }
        index = 2;
        break;
    case KSHIFT_LEFT | KHOLD_LEFT:
        if(nactions < 2) {
            index = 0;
            break;
        }
        if(nactions < 3) {
            index = 1;  // SINGLE-SHIFT SPEC
            break;
        }
        if(nactions < 4) {
            index = 1;  // LS-HOLD = LS IF NOT GIVEN
            break;
        }
        index = 3;
        break;
    case KSHIFT_RIGHT | KHOLD_RIGHT:
        if(nactions < 2) {
            index = 0;
            break;
        }
        if(nactions < 3) {
            index = 1;  // SINGLE-SHIFT SPEC
            break;
        }
        if(nactions < 5) {
            index = 2;  // LS-HOLD = LS IF NOT GIVEN
            break;
        }
        index = 4;
        break;
    default:
        index = 5;
    }

    word_p ptr = item + 1, end = rplSkipOb(item);

    while(ptr < end - 1) {
        if(!index)
            return ptr;
        ptr = rplSkipOb(ptr);
        --index;
    }

    return 0;

}

// GET THE HELP OBJECT OF A MENU ITEM
// RETURN 0 IF THE HELP IS NOT A STRING OR THERE WAS NO HELP IN THE MENU DEFINITION

word_p uiGetMenuItemHelp(word_p item)
{

    if(!item)
        return 0;

    if(ISIDENT(*item))
        return item;
    if(ISLIBPTR(*item)) {
        // USER LIBRARY COMMAND
        return uiGetLibPtrHelp(item);
    }
    if(ISLIBRARY(*item)) {
        // SHOW THE LIBRARY TITLE AS HELP STRING
        return rplGetLibPtr2(item[2], USERLIB_TITLE);
    }
    if(!ISLIST(*item)) {
        // SEARCH FOR HELP
        return uiGetLibHelp(item);
    }

    // GET HELP ITEM WITHIN THE ITEM

    item = rplGetListElement(item, 3);

    if(!item)
        return 0;

    if(!ISSTRING(*item))
        return 0;

    return item;

}


int uiMenuItemName(word_p        item,
                   menu_flags_t *flags,
                   utf8_p       *textP,
                   utf8_p       *endP)
// ----------------------------------------------------------------------------
//   Check if a menu item has a known name, if so return it
// ----------------------------------------------------------------------------
{
    if (!item)
        return 0;

    word_p  ptr;
    if (ISLIST(*item))
    {
        ptr = item + 1;
        if (ptr >= rplSkipOb(item) - 1)
            return 0;

        if (*ptr == CMD_ENDLIST)
            ptr = item;
        else
        {
            // If it's a program, run it and take the result from the stack
            if (ISPROGRAM(*ptr))
            {
                rplPushData(item);
                rplPushData(ptr);

                int32_t nresults = uiCmdRunTransparent(CMD_OVR_XEQ, 1, 1);

                if (nresults == 1)
                    ptr = rplPopData();
                else
                    // Failed to run, use empty string
                    ptr = (word_p) empty_string;

                // Restore the item pointer in case of GC
                item = rplPopData();
                halUpdateFonts();
            }
            if (ISLIBRARY(*ptr))
            {
                // Make the item be the library identifier
                ptr++;
            }

            //  If it's a list, then take the flags from the second element in
            //  the list, and use the first as the display object
            if (ISLIST(*ptr))
            {
                ptr = ptr + 1;
                if (ptr >= rplSkipOb(item) - 1)
                    return 0;
                if (*ptr == CMD_ENDLIST)
                    ptr = item;
                else
                {
                    word_p next = rplSkipOb(ptr);
                    if (ISint32_t(*next))
                        *flags = rplReadint32_t(next);
                }
            }
        }
    }
    else
    {
        ptr = item;
    }

    // Here ptr has an object to display and flags has the flags

    // Select what to display
    utf8_p    text      = "";
    utf8_p    end       = text;

    if (ISLIBRARY(*ptr))
    {
        // Draw it like a directory, displaying the library ID
        ptr++;
        text = (utf8_p) (ptr+1);
        end =  text + rplGetIdentLength(ptr);
        *flags |= MENU_IS_DIRECTORY;             // Show as directory
    }
    else if (ISIDENT(*ptr))
    {
        // For idents look for variables and check for directories
        word_p *var = rplFindGlobal(ptr, 1);
        if (var)
        {
            if (ISDIR(*var[1]))
                *flags |= MENU_IS_DIRECTORY;
            if ((rplGetIdentAttr(var[0]) & IDATTR_DEFN) == 0)
                *flags |= MENU_INVERT; // Invert undefined entries
        }
        text = (utf8_p) (ptr + 1);
        end =  text + rplGetIdentLength(ptr);
    }
    else if (ISSTRING(*ptr))
    {
        // Strings can be displayed as is
        unsigned totaln = rplStrLenCp(ptr);
        text = (utf8_p) (ptr + 1);
        end  = utf8nskip(text, (utf8_p) rplSkipOb(ptr), totaln);
    }
    else
    {
        // Other objects must be decompiled for display
        WORD    ptrprolog      = *ptr;

        int32_t SavedException = Exceptions;
        int32_t SavedErrorCode = ErrorCode;
        int32_t removevalue    = 0;

        if (ISUNIT(ptrprolog))
        {
            REAL r;
            if (ISNUMBER(ptr[1]))
            {
                rplReadNumberAsReal(ptr + 1, &r);
                rplOneToRReg(0);
                removevalue = eqReal(&r, &RReg[0]);
            }
        }

        // Erase any previous error to allow the decompiler to run
        Exceptions = 0;

        // Do not save IPtr because it can move
        word_p opname = rplDecompile(ptr, DECOMP_NOHINTS);
        Exceptions    = SavedException;
        ErrorCode     = SavedErrorCode;

        if (!opname)
        {
            // Error, show a sign of it
            text = "ERR?";
            end  = text + 4;
        }
        else
        {
            // Here we have a string
            // Do some more post-processing depending on object
            unsigned totaln = rplStrLenCp(opname);
            text = (utf8_p) (opname + 1);
            end  = utf8nskip(text, (utf8_p) rplSkipOb(opname), totaln);

            if (removevalue)
            {
                // Skip the numeric portion, leave just the unit
                int32_t offset = 0;
                for (unsigned k = 0; k < totaln; k++)
                {
                    offset = utf8skip(text + offset, end) - text;
                    if (utf82cp(text + offset, end) == '_')
                    {
                        totaln -= k + 1;
                        text += offset + 1;
                        break;
                    }
                }
                // TODO: Add more specialized handling here
            }
        }
    }

    *textP = text;
    *endP = end;
    return 1;
}


static void uiDrawMenuItemInternal(gglsurface  *scr,
                                   word_p       item,
                                   menu_flags_t flags)
// ----------------------------------------------------------------------------
//   Draw a single item in the current clipping box, does not clear background
// ----------------------------------------------------------------------------
// flags & 1  == is directory
// flags & 2  == inverted
// flags & 4  == use help colors
// flags & 8  == flag menu
// flags & 16 == flag value for flag menu
{
    utf8_p    text = "";
    utf8_p    end  = "";
    if (!uiMenuItemName(item, &flags, &text, &end))
        return;

    // Select display colors
    int       directory = (flags & 1) != 0;
    int       inverted  = (flags & 2) != 0;
    int       help_menu = (flags & 4) != 0;
    int       flag_menu = (flags & 8) != 0;
    pattern_t color_1   = help_menu ? PAL_HLP_TEXT
                        : directory ? PAL_MENU_DIR
                                    : PAL_MENU_TEXT;
    pattern_t color_2   = help_menu ? PAL_HLP_BG
                        : directory ? PAL_MENU_DIR_BG
                                    : PAL_MENU_BG;
    pattern_t fg        = inverted ? color_2 : color_1;
    pattern_t bg        = inverted ? color_1 : color_2;

    size      height    = scr->bottom - scr->top;
    size      width     = StringWidthN(text, end, FONT_MENU);
    size      swidth    = scr->right - scr->left - height * flag_menu;
    coord     pos       = (width >= swidth)
        ? 1 + scr->left
        : (1 + scr->right + scr->left - width) >> 1;

    // Draw the background
    ggl_cliprect(scr, scr->left, scr->top, scr->right, scr->bottom, bg);

    // Draw a marker to indicate directories
    if (directory)
    {
        size marker = height / 2;
        coord x = scr->left;
        coord top = scr->top;
        pattern_t earmark = PAL_MENU_DIR_MARK;
        for (unsigned row = 0; row < marker; row++)
        {
            coord y = top + row;
            size  w = marker - row;
            ggl_cliprect(scr, x, y, x+w-1, y, earmark);
        }
    }

    // Draw a marker to indicate flag selections
    if (flag_menu)
    {
        int flag_value = (flags & 16) != 0;
        pattern_t color = flag_value ? PAL_MENU_FLAG_ON : PAL_MENU_FLAG_OFF;
        coord x = scr->right - height / 2;
        coord y = scr->top + height / 2;
        if (height >= 8)
        {
            ggl_cliprect(scr, x - 1, y - 2, x + 1, y - 2, color);
            ggl_cliprect(scr, x - 2, y - 1, x + 2, y + 1, color);
            ggl_cliprect(scr, x - 1, y + 2, x + 1, y + 2, color);
        }
        else
        {
            ggl_cliprect(scr, x - 1, y - 1, x + 1, y + 1, color);
        }
    }

    // Draw the text for the menu
    scr->right -= height * flag_menu;
    DrawTextN(scr, pos, scr->top + 1, text, end, FONT_MENU, fg);
    scr->right += height * flag_menu;

}


void uiDrawMenuItem(gglsurface   *scr, word_p item)
// ----------------------------------------------------------------------------
//   Draw a regular menu item
// ----------------------------------------------------------------------------
{
    uiDrawMenuItemInternal(scr, item, MENU_NORMAL);
}


void uiDrawHelpMenuItem(gglsurface   *scr, word_p item)
// ----------------------------------------------------------------------------
//   Draw a regular menu item
// ----------------------------------------------------------------------------
{
    uiDrawMenuItemInternal(scr, item, MENU_HELP);
}
