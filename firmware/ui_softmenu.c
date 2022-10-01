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

// DRAW A SINGLE ITEM IN THE CURRENT CLIPPING BOX
// DOES NOT CLEAR BACKGROUND

void uiDrawMenuItem(word_p item, palette_index palette_color, palette_index palette_bkcolor, gglsurface * scr)
{
    word_p ptr;
    int32_t flags = 0;
    if(!item)
        return;



    if(ISLIST(*item)) {
        ptr = item + 1;
        if(ptr >= rplSkipOb(item) - 1)
            return;

        if(*ptr == CMD_ENDLIST)
            ptr = item;
        else {
            // IF IT'S A PROGRAM, RUN IT AND TAKE THE RESULT FROM THE STACK
            if(ISPROGRAM(*ptr)) {
                rplPushData(item);
                rplPushData(ptr);

                int32_t nresults = uiCmdRunTransparent(CMD_OVR_XEQ, 1, 1);

                if(nresults == 1)
                    ptr = rplPopData();
                else
                    ptr = (word_p) empty_string;       // IF THE PROGRAM FAILED TO RETURN AN OBJECT, JUST USE THE EMPTY STRING
                item = rplPopData();    // RESTORE THE item POINTER IN CASE OF GC
                halUpdateFonts();
                // CONTINUE HERE WITH THE NEW ptr

            }
            if(ISLIBRARY(*ptr)) {
                // MAKE THE ITEM BE THE LIBRARY IDENTIFIER
                ptr++;
            }

            //  IF IT'S A LIST, THEN TAKE THE FLAGS FROM THE SECOND ELEMENT IN THE LIST, AND USE THE FIRST AS THE DISPLAY OBJECT
            if(ISLIST(*ptr)) {
                ptr = ptr + 1;
                if(ptr >= rplSkipOb(item) - 1)
                    return;
                if(*ptr == CMD_ENDLIST)
                    ptr = item;
                else {
                    word_p next = rplSkipOb(ptr);
                    if(ISint32_t(*next))
                        flags = rplReadint32_t(next);
                }
            }

        }
    }
    else
        ptr = item;

    // HERE ptr HAS AN OBJECT TO DISPLAY

    // AND flags HAS THE FLAGS


    pattern_t color, bcolor, dircolor;
    if (palette_color == PAL_MENU_INV_TEXT)
    {
        // IF INVERTED BY FLAGS, USE THE ALTERNATIVE SET OF COLORS
        if (flags & 2)
        {
            color    = ggl_solid(PAL_MENU_TEXT);
            bcolor   = ggl_solid(PAL_MENU_BG);
            dircolor = ggl_solid(PAL_MENU_DIR_MARK);
        }
        else
        {
            color    = ggl_solid(PAL_MENU_INV_TEXT);
            bcolor   = ggl_solid(PAL_MENU_INV_BG);
            dircolor = ggl_solid(PAL_MENU_INV_DIR_MARK);
        }
    }
    else
    {
        if (palette_color == PAL_MENU_TEXT)
        {
            // IF INVERTED BY FLAGS, USE THE ALTERNATIVE SET OF COLORS
            if (flags & 2)
            {
                color    = ggl_solid(PAL_MENU_INV_TEXT);
                bcolor   = ggl_solid(PAL_MENU_INV_BG);
                dircolor = ggl_solid(PAL_MENU_INV_DIR_MARK);
            }
            else
            {
                color    = ggl_solid(PAL_MENU_TEXT);
                bcolor   = ggl_solid(PAL_MENU_BG);
                dircolor = ggl_solid(PAL_MENU_DIR_MARK);
            }
        }
        else
        {
            // JUST USE THE SPECIFIC COLORS REQUESTED BY CALLER, SWAP COLORS IF
            // INVERTED BY FLAGS
            if (flags & 2)
            {
                dircolor = bcolor = ggl_solid(palette_color);
                color             = ggl_solid(palette_bkcolor);
            }
            else
            {
                color    = ggl_solid(palette_color);
                dircolor = bcolor = ggl_solid(palette_bkcolor);
            }
        }
    }

    if (ISLIBRARY(*ptr))
    {
        // SPECIAL CASE: DRAW IT LIKE A DIRECTORY, DISPLAYING THE LIBRARY ID
        ptr++;


        // REDRAW THE BACKGROUND WITH THE NEW COLOR
        if (flags & 2)
            ggl_rect(scr, scr->left, scr->top, scr->right, scr->bottom, bcolor);


        int32_t w = StringWidthN((char *) (ptr + 1),
                                 (char *) (ptr + 1) + rplGetIdentLength(ptr),
                                 FONT_MENU),
                pos;

        if (w >= scr->right - scr->left)
            pos = scr->left + 1;
        else
            pos = (scr->right + 1 + scr->left - w) >> 1;

        // FIRST LETTER GRAY BACKGROUND
        ggl_cliprect(scr,
                     pos,
                     scr->top,
                     pos + MENU1_HEIGHT / 2,
                     scr->bottom,
                     dircolor);
        /*
        ggl_clipvline(scr, pos + 1, scr->top, scr->bottom,
                ggl_solid((color) ? 0X4 : 0x8));
        ggl_clipvline(scr, pos + 2, scr->top, scr->bottom,
                ggl_solid((color) ? 0X4 : 0x8));
        ggl_clipvline(scr, pos + 3, scr->top, scr->bottom,
                ggl_solid((color) ? 0X4 : 0x8));
        */
        DrawTextN(scr,
                  pos,
                  scr->top + 1,
                  (char *) (ptr + 1),
                  (char *) (ptr + 1) + rplGetIdentLength(ptr),
                  FONT_MENU,
                  color);

#if 1
#warning Need to revisit that lighten/darken code
#else
        // DARKEN/LIGHTEN EFFECT ON LAST FEW PIXELS
        if (w >= scr->right - scr->left)
        {
            int rf = RGBRED(color);
            int rb = RGBRED(bcolor);
            int gf = RGBGREEN(color);
            int gb = RGBGREEN(bcolor);
            int bf = RGBBLUE(color);
            int bb = RGBBLUE(bcolor);


            // Vanishing width
            // Create 3 interpolated colors: (3f+b)/4, (f+b)/2 and (f+3b)/4
            int vw = MENU_TAB_WIDTH / 16;

            scr->x = scr->right - vw + 1;
            scr->y = scr->top;


            ggl_filter(scr, vw, scr->bottom - scr->top + 1,
                       RGB_TO_RGB16( (rf+3*rb)>>2 , (gf+3*gb)>>2 , (bf+3*bb)>>2).value | (color.value<<16) , &ggl_fltreplace);
            scr->x-=vw;
            ggl_filter(scr, vw, scr->bottom - scr->top + 1,
                       RGB_TO_RGB16( (rf+rb)>>1 , (gf+gb)>>1 , (bf+bb)>>1).value | (color.value<<16), &ggl_fltreplace);
            scr->x-=vw;
            ggl_filter(scr, vw, scr->bottom - scr->top + 1,
                       RGB_TO_RGB16( (3*rf+rb)>>2 , (3*gf+gb)>>2 , (3*bf+bb)>>2).value | (color.value<<16), &ggl_fltreplace);
        }
#endif // Darken / Lighten


        return;
    }

    if(ISIDENT(*ptr)) {

        // SPECIAL CASE: FOR IDENTS LOOK FOR VARIABLES AND DRAW DIFFERENTLY IF IT'S A DIRECTORY
        word_p *var = rplFindGlobal(ptr, 1);

        int32_t w = StringWidthN((char *)(ptr + 1),
                (char *)(ptr + 1) + rplGetIdentLength(ptr),
                FONT_MENU), pos;

        if(w >= scr->right - scr->left)
            pos = scr->left + 1;
        else
            pos = (scr->right + 1 + scr->left - w) >> 1;
        if(palette_color==PAL_MENU_INV_TEXT) {
            if((flags & 2) || (var && (rplGetIdentAttr(var[0]) & IDATTR_DEFN))) {
                color=ggl_solid(PAL_MENU_TEXT);
                bcolor=ggl_solid(PAL_MENU_BG);
                dircolor=ggl_solid(PAL_MENU_DIR_MARK);

                // REDRAW THE BACKGROUND WITH THE NEW COLOR
                ggl_rect(scr,scr->left,scr->top,scr->right,scr->bottom,bcolor);
            }

        }
        else {
        if((flags & 2) || (var && (rplGetIdentAttr(var[0]) & IDATTR_DEFN))) {
            color=ggl_solid(PAL_MENU_INV_TEXT);
            bcolor=ggl_solid(PAL_MENU_INV_BG);
            dircolor=ggl_solid(PAL_MENU_INV_DIR_MARK);

            // REDRAW THE BACKGROUND WITH THE NEW COLOR
            ggl_rect(scr,scr->left,scr->top,scr->right,scr->bottom,bcolor);

        }
        }




        // flags & 1 == IS_DIRECTORY
        // flags & 2 == INVERTED
        if((flags & 1) || (var && ISDIR(*var[1]))) {
            //ggl_clipvline(scr,scr->right,scr->top,scr->bottom,ggl_solid(color));
            //ggl_cliphline(scr,scr->top,scr->left,scr->left+3,ggl_solid(color));
            //DrawTextN(pos+1,scr->top+1,(char *)(ptr+1),(char *)(ptr+1)+rplGetIdentLength(ptr),FONT_MENU,(color)? 0x4:0xa,scr);



            // FIRST LETTER GRAY BACKGROUND
            ggl_cliprect(scr, pos, scr->top, pos+MENU1_HEIGHT/2,scr->bottom,
                    dircolor);

            /*
            ggl_clipvline(scr, pos, scr->top, scr->bottom,
                    ggl_solid((color) ? 0X4 : 0x8));
            ggl_clipvline(scr, pos + 1, scr->top, scr->bottom,
                    ggl_solid((color) ? 0X4 : 0x8));
            ggl_clipvline(scr, pos + 2, scr->top, scr->bottom,
                    ggl_solid((color) ? 0X4 : 0x8));
            ggl_clipvline(scr, pos + 3, scr->top, scr->bottom,
                    ggl_solid((color) ? 0X4 : 0x8));
            */
            // LOWER 2 LINES GRAY
            //ggl_cliphline(scr,scr->bottom,scr->left,scr->right,ggl_solid( (color)? 0X4:0x6));
            //ggl_cliphline(scr,scr->bottom-1,scr->left,scr->right,ggl_solid( (color)? 0X4:0x6));
            //ggl_cliphline(scr,scr->bottom-2,scr->left,scr->right,ggl_solid( (color)? 0X4:0x6));

            // UNDERLINE FIRST LETTER
            //ggl_cliphline(scr,scr->bottom,pos,pos+6,ggl_solid( (color)? 0X4:0x6));

        }

        DrawTextN(scr, pos, scr->top + 1, (char *)(ptr + 1),
                (char *)(ptr + 1) + rplGetIdentLength(ptr),
                FONT_MENU, color);

#if 1
#warning Revisit
#else
        // DARKEN/LIGHTEN EFFECT ON LAST FEW PIXELS
        if(w >= scr->right - scr->left) {

            int rf,rb,gf,gb,bf,bb;

            rf=RGBRED(color);
            rb=RGBRED(bcolor);
            gf=RGBGREEN(color);
            gb=RGBGREEN(bcolor);
            bf=RGBBLUE(color);
            bb=RGBBLUE(bcolor);

            // CREATE 3 INTERPOLATED COLORS: (3F+B)/4, (F+B)/2 AND (F+3B)/4
            int vw=MENU_TAB_WIDTH/16;

            scr->x = scr->right-vw+1;
            scr->y = scr->top;


            ggl_filter(scr,
                       vw,
                       scr->bottom - scr->top + 1,
                       RGB_TO_RGB16((rf + 3 * rb) >> 2, (gf + 3 * gb) >> 2, (bf + 3 * bb) >> 2).value |
                           (color.value << 16),
                       &ggl_fltreplace);
            scr->x-=vw;
            ggl_filter(scr,
                       vw,
                       scr->bottom - scr->top + 1,
                       RGB_TO_RGB16((rf + rb) >> 1, (gf + gb) >> 1, (bf + bb) >> 1).value | (color.value << 16),
                       &ggl_fltreplace);
            scr->x-=vw;
            ggl_filter(scr,
                       vw,
                       scr->bottom - scr->top + 1,
                       RGB_TO_RGB16((3 * rf + rb) >> 2, (3 * gf + gb) >> 2, (3 * bf + bb) >> 2).value |
                       (color.value << 16),
                       &ggl_fltreplace);
        }
#endif // 0

        return;
    }

    // ALL OTHER OBJECTS NEED TO BE DECOMPILED, EXCEPT THE STRING AND GROBS

    int32_t totaln;
    byte_p string, endstring;

    // TODO: ADD GROBS HERE

    if(!ISSTRING(*ptr)) {

        WORD ptrprolog = *ptr;

        int32_t SavedException = Exceptions;
        int32_t SavedErrorCode = ErrorCode;
        int32_t removevalue = 0;

        if(ISUNIT(ptrprolog)) {
            REAL r;
            if(ISNUMBER(ptr[1])) {
                rplReadNumberAsReal(ptr + 1, &r);
                rplOneToRReg(0);
                removevalue = eqReal(&r, &RReg[0]);
            }
        }

        Exceptions = 0; // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
        // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
        word_p opname = rplDecompile(ptr, DECOMP_NOHINTS);
        Exceptions = SavedException;
        ErrorCode = SavedErrorCode;

        if(!opname)
            return;

        // HERE WE HAVE A STRING, DO SOME MORE POST-PROCESSING DEPENDING ON OBJECT

        string = (byte_p) (opname + 1);
        totaln = rplStrLenCp(opname);
        endstring =
                (byte_p) utf8nskip((char *)string, (char *)rplSkipOb(opname),
                totaln);

        if(removevalue) {
            // SKIP THE NUMERIC PORTION, LEAVE JUST THE UNIT
            int32_t k, offset;
            for(k = 0, offset = 0; k < totaln;
                    ++k, offset =
                    (byte_p) utf8skip((char *)string + offset,
                        (char *)endstring) - string)
                if(utf82cp((char *)string + offset, (char *)endstring) == '_') {
                    totaln -= k + 1;
                    string += offset + 1;
                    break;
                }
        }

        // TODO: ADD MORE SPECIALIZED HANDLING HERE

    }
    else {
        string = (byte_p) (ptr + 1);
        totaln = rplStrLenCp(ptr);
        endstring =
                (byte_p) utf8nskip((char *)string, (char *)rplSkipOb(ptr),
                totaln);
    }

    // JUST DISPLAY THE STRING

    // REDRAW THE BACKGROUND WITH THE NEW COLOR
    if(flags&2) ggl_rect(scr,scr->left,scr->top,scr->right,scr->bottom,bcolor);


    int32_t w = StringWidthN((char *)string, (char *)endstring,
            FONT_MENU), pos;
    if(w >= scr->right - scr->left)
        pos = scr->left + 1;
    else
        pos = (scr->right + 1 + scr->left - w) >> 1;

    if(flags & 1)       // FOR NOW, flags & 1 INDICATES THE MENU IS TO BE DISPLAYED AS A DIRECTORY
    {
        //ggl_clipvline(scr,scr->right,scr->top,scr->bottom,ggl_solid(color));
        //ggl_cliphline(scr,scr->top,scr->left,scr->left+3,ggl_solid(color));
        //DrawTextN(pos+1,scr->top+1,(char *)string,(char *)endstring,FONT_MENU,(color)? 0x4:0xa,scr);

        ggl_cliprect(scr, pos, scr->top, pos+MENU1_HEIGHT/2,scr->bottom,
                dircolor);
        /*
        // FIRST LETTER GRAY BACKGROUND
        ggl_clipvline(scr, pos, scr->top, scr->bottom,
                ggl_solid((color) ? 0X4 : 0x8));
        ggl_clipvline(scr, pos + 1, scr->top, scr->bottom,
                ggl_solid((color) ? 0X4 : 0x8));
        ggl_clipvline(scr, pos + 2, scr->top, scr->bottom,
                ggl_solid((color) ? 0X4 : 0x8));
        ggl_clipvline(scr, pos + 3, scr->top, scr->bottom,
                ggl_solid((color) ? 0X4 : 0x8));
        */
        // LOWER 2 LINES GRAY
        //ggl_cliphline(scr,scr->bottom,scr->left,scr->right,ggl_solid( (color)? 0X4:0x6));
        //ggl_cliphline(scr,scr->bottom-1,scr->left,scr->right,ggl_solid( (color)? 0X4:0x6));
        //ggl_cliphline(scr,scr->bottom-2,scr->left,scr->right,ggl_solid( (color)? 0X4:0x6));

        // UNDERLINE FIRST LETTER
        //ggl_cliphline(scr,scr->bottom,pos,pos+6,ggl_solid( (color)? 0X4:0x6));

    }

    DrawTextN(scr, pos, scr->top + 1, (char *)string, (char *)endstring,
            FONT_MENU, color);


#if 1
#warning Revisit
#else
    // DARKEN/LIGHTEN EFFECT ON LAST FEW PIXELS
    if(w >= scr->right - scr->left) {

        int rf,rb,gf,gb,bf,bb;

        rf=RGBRED(color);
        rb=RGBRED(bcolor);
        gf=RGBGREEN(color);
        gb=RGBGREEN(bcolor);
        bf=RGBBLUE(color);
        bb=RGBBLUE(bcolor);

        // CREATE 3 INTERPOLATED COLORS: (3F+B)/4, (F+B)/2 AND (F+3B)/4
        int vw=MENU_TAB_WIDTH/16;

        scr->x = scr->right-vw+1;
        scr->y = scr->top;


        ggl_filter(scr,
                   vw,
                   scr->bottom - scr->top + 1,
                   RGB_TO_RGB16((rf + 3 * rb) >> 2, (gf + 3 * gb) >> 2, (bf + 3 * bb) >> 2).value | (color.value << 16),
                   &ggl_fltreplace);
        scr->x -= vw;
        ggl_filter(scr,
                   vw,
                   scr->bottom - scr->top + 1,
                   RGB_TO_RGB16((rf + rb) >> 1, (gf + gb) >> 1, (bf + bb) >> 1).value | (color.value << 16),
                   &ggl_fltreplace);
        scr->x -= vw;
        ggl_filter(scr,
                   vw,
                   scr->bottom - scr->top + 1,
                   RGB_TO_RGB16((3 * rf + rb) >> 2, (3 * gf + gb) >> 2, (3 * bf + bb) >> 2).value | (color.value << 16),
                   &ggl_fltreplace);
    }
#endif
    return;

}
