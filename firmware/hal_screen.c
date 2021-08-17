/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <cmdcodes.h>
#include <fsystem.h>
#include <libraries.h>
#include <newrpl.h>
#include <ui.h>

// THIS IS THE MAIN STABLE API TO ACCESS THE SCREEN

// SET TO SHOW/HIDE THE NOTIFICATION ICON

void halSetNotification(enum halNotification type, int color)
{

    int old = halFlags & (1 << (16 + type));
    if(color)
        halFlags |= 1 << (16 + type);
    else
        halFlags &= ~(1 << (16 + type));

    if(type < N_DATARECVD) {
        unsigned char *scrptr = (unsigned char *)MEM_PHYS_SCREEN;
        scrptr += ANN_X_COORD / (PIXELS_PER_WORD/4);
        scrptr += type * (SCREEN_W / (PIXELS_PER_WORD/4));
        *scrptr = (*scrptr & ~(((1<<BITSPERPIXEL)-1) << (BITSPERPIXEL*(ANN_X_COORD % (PIXELS_PER_WORD/4))))) | (color << (BITSPERPIXEL*(ANN_X_COORD % (PIXELS_PER_WORD/4))));
        return;
    }
    else {
        // TODO: DRAW CUSTOM ICONS INTO THE STATUS AREA FOR ALL OTHER ANNUNCIATORS
        if((halFlags ^ old) & (1 << (16 + type))) {
            halScreen.DirtyFlag |= STAREA_DIRTY;        // REDRAW STATUS AS SOON AS POSSIBLE
        }
    }
}

int halGetNotification(enum halNotification type)
{
    if(halFlags & (1 << (16 + type)))
        return 1;
    return 0;
}

void halSetStackHeight(int h)
{
    int total;
    halScreen.Stack = h;
    total = halScreen.Form + halScreen.Stack + halScreen.CmdLine +
            halScreen.Menu1 + halScreen.Menu2;
    if(total != SCREEN_HEIGHT) {
        if(halScreen.Form) {
            halScreen.Form += SCREEN_HEIGHT - total;
            halScreen.DirtyFlag |= FORM_DIRTY;
        }

        else
            halScreen.Stack =
                    SCREEN_HEIGHT - halScreen.CmdLine - halScreen.Menu1 -
                    halScreen.Menu2;
        if(halScreen.Form < 0) {
            halScreen.Stack += halScreen.Form;
            halScreen.Form = 0;
        }
    }
    halScreen.DirtyFlag |= STACK_DIRTY;
}

void halSetFormHeight(int h)
{
    int total;
    if(h < 0)
        h = 0;
    halScreen.Form = h;
    total = halScreen.Form + halScreen.Stack + halScreen.CmdLine +
            halScreen.Menu1 + halScreen.Menu2;
    if(total != SCREEN_HEIGHT) {
        if(halScreen.Stack) {
            halScreen.Stack += SCREEN_HEIGHT - total;
            halScreen.DirtyFlag |= STACK_DIRTY;
        }

        else
            halScreen.Form =
                    SCREEN_HEIGHT - halScreen.CmdLine - halScreen.Menu1 -
                    halScreen.Menu2;
        if(halScreen.Stack < 0) {
            halScreen.Form += halScreen.Stack;
            halScreen.Stack = 0;
        }
    }
    halScreen.DirtyFlag |= FORM_DIRTY;

}

// MENU1 AREA IS USUALLY FIXED TO 1 LINE, BUT THIS IS GENERIC CODE
void halSetMenu1Height(int h)
{
    int total;
    if(h < 0)
        h = 0;
    halScreen.Menu1 = h;
    total = halScreen.Form + halScreen.Stack + halScreen.CmdLine +
            halScreen.Menu1 + halScreen.Menu2;
    while(total != SCREEN_HEIGHT) {
        // STRETCH THE STACK FIRST (IF ACTIVE), THEN FORM

        if(halScreen.Stack) {
            halScreen.Stack += SCREEN_HEIGHT - total;
            halScreen.DirtyFlag |= STACK_DIRTY;

            if(halScreen.Stack < 0)
                halScreen.Stack = 0;
        }
        else {
            halScreen.Form += SCREEN_HEIGHT - total;
            halScreen.DirtyFlag |= FORM_DIRTY;

            if(halScreen.Form < 0) {
                halScreen.Menu1 += halScreen.Form;
                halScreen.Form = 0;
            }
        }
        total = halScreen.Form + halScreen.Stack + halScreen.CmdLine +
                halScreen.Menu1 + halScreen.Menu2;
    }
    halScreen.DirtyFlag |= MENU1_DIRTY | CMDLINE_ALLDIRTY;

}

void halSetMenu2Height(int h)
{
    int total;
    if(h < 0)
        h = 0;
    halScreen.Menu2 = h;
    total = halScreen.Form + halScreen.Stack + halScreen.CmdLine +
            halScreen.Menu1 + halScreen.Menu2;
    while(total != SCREEN_HEIGHT) {
        // STRETCH THE STACK FIRST (IF ACTIVE), THEN FORM

        if(halScreen.Stack > 1) {
            halScreen.Stack += SCREEN_HEIGHT - total;
            halScreen.DirtyFlag |= STACK_DIRTY;
            if(halScreen.Stack < 1)
                halScreen.Stack = 1;
        }
        else {
            if(halScreen.Form > 1) {
                halScreen.Form += SCREEN_HEIGHT - total;
                halScreen.DirtyFlag |= FORM_DIRTY;
                if(halScreen.Form < 1)
                    halScreen.Form = 1;
            }
            else {
                if(halScreen.CmdLine > 1) {
                    int newcmdht = halScreen.CmdLine + SCREEN_HEIGHT - total;
                    int newnlines =
                            (newcmdht -
                            2) /
                            (*halScreen.FontArray[FONT_CMDLINE])->BitmapHeight;
                    if(newnlines < 1) {
                        // THERE'S NO ROOM AT ALL, VANISH THE MENU REGARDLESS
                        halScreen.Menu2 = 0;
                    }
                    else {
                        if(newnlines != halScreen.NumLinesVisible) {
                            // WE ARE CHANGING THE COMMAND LINE HEIGHT
                            uiStretchCmdLine(newnlines -
                                    halScreen.NumLinesVisible);
                            uiEnsureCursorVisible();
                        }
                        halScreen.DirtyFlag |= CMDLINE_ALLDIRTY;
                    }
                }
            }
        }
        total = halScreen.Form + halScreen.Stack + halScreen.CmdLine +
                halScreen.Menu1 + halScreen.Menu2;
    }
    halScreen.DirtyFlag |=
            MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY | CMDLINE_ALLDIRTY;

}

void halSetCmdLineHeight(int h)
{
    int total;
    if(h < 0)
        h = 0;
    halScreen.CmdLine = h;
    total = halScreen.Form + halScreen.Stack + halScreen.CmdLine +
            halScreen.Menu1 + halScreen.Menu2;
    while(total != SCREEN_HEIGHT) {
        // STRETCH THE STACK FIRST (IF ACTIVE), THEN FORM

        if(halScreen.Stack > 1) {
            halScreen.Stack += SCREEN_HEIGHT - total;
            halScreen.DirtyFlag |= STACK_DIRTY;

            if(halScreen.Stack < 1)
                halScreen.Stack = 1;
        }
        else {
            if(halScreen.Form > 1) {
                halScreen.Form += SCREEN_HEIGHT - total;
                halScreen.DirtyFlag |= FORM_DIRTY;

                if(halScreen.Form < 1)
                    halScreen.Form = 1;
            }
            else {
                // STACK AND FORMS ARE AT MINIMUM
                if(total > SCREEN_HEIGHT) {
                    halScreen.CmdLine =
                            SCREEN_HEIGHT - 2 - (halScreen.Form +
                            halScreen.Stack + halScreen.Menu1 +
                            halScreen.Menu2);
                    halScreen.CmdLine /= (*halScreen.FontArray[FONT_CMDLINE])->
                            BitmapHeight;
                    if(halScreen.CmdLine < 1)
                        halScreen.CmdLine = 1;
                    halScreen.CmdLine *=
                            (*halScreen.FontArray[FONT_CMDLINE])->BitmapHeight;
                    halScreen.CmdLine += 2;
                }
                else {
                    // ENLARGE STACK
                    if(halScreen.Stack > 0) {
                        halScreen.Stack += SCREEN_HEIGHT - total;
                        halScreen.DirtyFlag |= STACK_DIRTY;
                    }
                    else {
                        // IF THE STACK IS CLOSED, THEN IT HAS TO BE A FORM!

                        halScreen.Form += SCREEN_HEIGHT - total;
                        halScreen.DirtyFlag |= FORM_DIRTY;
                    }

                }
            }
        }
        total = halScreen.Form + halScreen.Stack + halScreen.CmdLine +
                halScreen.Menu1 + halScreen.Menu2;
    }
    halScreen.DirtyFlag |= CMDLINE_ALLDIRTY;

}

// COMPUTE HEIGHT AND WIDTH OF OBJECT TO DISPLAY ON STACK

BINT halGetDispObjectHeight(WORDPTR object, UNIFONT * font)
{
    UNUSED_ARGUMENT(object);
    // TODO: ADD MULTILINE OBJECTS, ETC.

    return font->BitmapHeight;
}

extern const UBINT64 const powersof10[20];

// CONVERT INTEGER NUMBER INTO STRING FOR STACK LEVEL
// str MUST CONTAIN AT LEAST 15: BYTES "-1,345,789,123[NULL]"
void halInt2String(int num, char *str)
{
    int pow10idx = 9;   // START WITH 10^10
    int digit, firstdigit;
    char *ptr = str;
    if(num < 0) {
        *ptr++ = '-';
        num = -num;
    }

    firstdigit = 1;
    do {
        digit = 0;
        while((UBINT64) num >= powersof10[pow10idx]) {
            ++digit;
            num -= powersof10[pow10idx];
        }
        if(!((digit == 0) && firstdigit)) {
            *ptr++ = digit + '0';
            firstdigit = 0;
        }
        ++pow10idx;
    }
    while(num != 0);

    if(firstdigit)
        *ptr++ = '0';
    else {
        while(pow10idx < 19) {
            *ptr++ = '0';
            ++pow10idx;
        }
    }
    *ptr = 0;
}

void halRedrawForm(DRAWSURFACE * scr)
{

    if(halScreen.Form == 0) {
        halScreen.DirtyFlag &= ~FORM_DIRTY;
        return;
    }

    halScreenUpdated();

    // REDRAW THE CONTENTS OF THE CURRENT FORM
    int oldclipx, oldclipx2, oldclipy, oldclipy2;
    int ystart = 0, yend = ystart + halScreen.Form;

    oldclipx = scr->clipx;
    oldclipy = scr->clipy;
    oldclipx2 = scr->clipx2;
    oldclipy2 = scr->clipy2;

    WORDPTR form, bmp;

    form = rplGetSettings((WORDPTR) currentform_ident);

    if(!form) {
        cgl_cliprect(scr, scr->clipx, ystart, scr->clipx2, yend - 1, cgl_mkcolor(PAL_FORMBACKGND));       // CLEAR RECTANGLE
        halScreen.DirtyFlag &= ~FORM_DIRTY;
        return;
    }

    bmp = uiFindCacheEntry(form, halScreen.FontArray[FONT_FORMS]);

    if(!bmp) {
        cgl_cliprect(scr, scr->clipx, ystart, scr->clipx2, yend - 1, cgl_mkcolor(PAL_FORMBACKGND));       // CLEAR RECTANGLE
        halScreen.DirtyFlag &= ~FORM_DIRTY;
        return;
    }

    DRAWSURFACE viewport;

    viewport.addr = (int *)(bmp + 3);
    viewport.width = bmp[1];
    viewport.x = 0;
    viewport.y = 0;     // TODO: CHANGE THIS TO ENABLE SCROLLING
    viewport.clipx = 0;
    viewport.clipx2 = bmp[1] - 1;
    viewport.clipy = 0;
    viewport.clipy2 = bmp[2] - 1;

    // POSITION THE VIEWPORT ON THE SCREEN

    scr->x = 0;
    scr->y = 0;

    scr->clipx = 0;
    scr->clipx2 = SCREEN_WIDTH - 1;
    scr->clipy = 0;
    scr->clipy2 = yend;

    if(yend > viewport.clipy2 + 1) {
        // CLEAR THE BACKGROUND
        cgl_cliprect(scr, scr->clipx, viewport.clipy2 + 1, scr->clipx2, yend - 1, cgl_mkcolor(PAL_FORMBACKGND));   // CLEAR RECTANGLE
    }

    // DRAW THE VIEWPORT
    cgl_bitbltclip(scr, &viewport, viewport.width, viewport.clipy2 + 1);
    halScreen.DirtyFlag &= ~FORM_DIRTY;

    scr->clipx = oldclipx;
    scr->clipx2 = oldclipx2;
    scr->clipy = oldclipy;
    scr->clipy2 = oldclipy2;
}

void halRedrawStack(DRAWSURFACE * scr)
{
    if(halScreen.Stack == 0) {
        halScreen.DirtyFlag &= ~STACK_DIRTY;
        return;
    }

    halScreenUpdated();

    int oldclipx, oldclipx2, oldclipy, oldclipy2;
    int ystart = halScreen.Form, yend = ystart + halScreen.Stack;
    int depth = rplDepthData(), level = 1;
    int objheight, ytop, y, numwidth, xright,stknum_w;
    BINT width, height;
    char num[16];
    UNIFONT const **levelfnt;
    WORDPTR object;

    oldclipx = scr->clipx;
    oldclipy = scr->clipy;
    oldclipx2 = scr->clipx2;
    oldclipy2 = scr->clipy2;

    stknum_w=((*halScreen.FontArray[FONT_STACK])->BitmapHeight*192)/256;    // ESTIMATE NUMBER WIDTH AT 75% OF THE FONT HEIGHT
    if(halScreen.KeyContext & CONTEXT_INTSTACK) {

        // ENSURE THE STACK POINTER IS COMPLETELY INSIDE THE SCREEN

        if(halScreen.StkVisibleLvl < 0) {
            // NEED TO RECOMPUTE THIS
            int k = halScreen.StkPointer;
            int objh, stkheight = 0;
            if(k < 1)
                k = 1;
            if(k > depth)
                k = depth;

            if(k == 1)
                levelfnt = halScreen.FontArray[FONT_STACKLVL1];
            else
                levelfnt = halScreen.FontArray[FONT_STACK];
            object = uiRenderObject(rplPeekData(k), levelfnt);
            // GET THE SIZE OF THE OBJECT

            if(!object)
                objh = (*levelfnt)->BitmapHeight;
            else
                objh = object[2];

            int ypref = ystart + (yend - ystart) / 4 + objh / 2;
            if(ypref > yend)
                ypref = yend - objh;
            if(ypref < ystart)
                ypref = ystart;

            for(; k > 0; --k) {
                if(k == 1)
                    levelfnt = halScreen.FontArray[FONT_STACKLVL1];
                else
                    levelfnt = halScreen.FontArray[FONT_STACK];
                object = uiRenderObject(rplPeekData(k), levelfnt);
                // GET THE SIZE OF THE OBJECT

                if(!object)
                    stkheight += (*levelfnt)->BitmapHeight;
                else
                    stkheight += object[2];

                if(ypref + stkheight > yend) {
                    y = ypref + stkheight;
                    halScreen.StkVisibleLvl = k;
                    halScreen.StkVisibleOffset = yend - y;
                    break;
                }

            }
            if(!k) {
                halScreen.StkVisibleLvl = 1;
                halScreen.StkVisibleOffset = 0;
            }

        }

        xright = 2*stknum_w;
    }
    else
        xright = stknum_w;

    level = halScreen.StkVisibleLvl;
    y = yend - halScreen.StkVisibleOffset;

    if(depth >= 10)
        xright += stknum_w;
    if(depth >= 100)
        xright += stknum_w;
    if(depth >= 1000)
        xright += stknum_w;
    if(depth >= 10000)
        xright += stknum_w;

    cgl_cliprect(scr, 0, ystart, xright-1, yend - 1, cgl_mkcolor(PAL_STKIDXBACKGND));    // CLEAR RECTANGLE
    cgl_cliprect(scr, xright+1, ystart, SCREEN_WIDTH-1, yend - 1, cgl_mkcolor(PAL_STKBACKGND));    // CLEAR RECTANGLE
    cgl_clipvline(scr, xright, ystart, yend - 1, cgl_mkcolor(PAL_STKVLINE));

    while(y > ystart) {
        if(level == 1)
            levelfnt = halScreen.FontArray[FONT_STACKLVL1];
        else
            levelfnt = halScreen.FontArray[FONT_STACK];

        // GET OBJECT SIZE

        if(level <= depth) {
            // DRAW THE OBJECT
            object = uiRenderObject(rplPeekData(level), levelfnt);
            // GET THE SIZE OF THE OBJECT

            if(!object) {
                // DRAW DIRECTLY, DON'T CACHE SOMETHING WE COULDN'T RENDER

                WORDPTR string = (WORDPTR) invalid_string;

                // NOW SIZE THE STRING OBJECT
                BINT nchars = rplStrSize(string);
                BYTEPTR charptr = (BYTEPTR) (string + 1);

                width = StringWidthN((char *)charptr, (char *)charptr + nchars,
                        *levelfnt);
                height = (*levelfnt)->BitmapHeight;
            }
            else {
                width = (BINT) object[1];
                height = (BINT) object[2];
            }

            objheight = height;
            if(objheight > 4 * (*levelfnt)->BitmapHeight)
                objheight = 4 * (*levelfnt)->BitmapHeight;      // MAXIMUM HEIGHT FOR A STACK ITEM IS 4 LINES, AFTER THAT CLIP IT

        }
        else {
            object = 0;
            objheight = (*levelfnt)->BitmapHeight;
            width = 0;
        }

        ytop = y - objheight;
        scr->x = xright + 1;
        scr->y = ytop;

        // SET CLIPPING REGION

        scr->clipx = 0;
        scr->clipx2 = SCREEN_WIDTH - 1;
        scr->clipy = (ytop < 0) ? 0 : ytop;
        scr->clipy2 = (y > yend) ? yend - 1 : y - 1;

        if(halScreen.KeyContext & CONTEXT_INTSTACK) {
            // HIGHLIGHT SELECTED ITEMS
            switch (halScreen.StkSelStatus) {
            default:
            case 0:
                // NOTHING SELECTED YET
                break;
            case 1:
                // START WAS SELECTED, PAINT ALL LEVELS BETWEEN START AND CURRENT POSITION
                if(halScreen.StkSelStart > halScreen.StkPointer) {
                    if((level >= halScreen.StkPointer)
                            && (level <= halScreen.StkSelStart))
                        cgl_cliprect(scr, 0, ytop, xright - 1, y - 1,
                                cgl_mkcolor(PAL_STKSELBKGND));
                    if(level == halScreen.StkSelStart)
                        DrawText(2, ytop, "▶", *halScreen.FontArray[FONT_STACK], cgl_mkcolor(PAL_STKCURSOR), scr);
                }
                else {
                    if((level >= halScreen.StkSelStart)
                            && (level <= halScreen.StkPointer))
                        cgl_cliprect(scr, 0, ytop, xright - 1, y - 1,
                                cgl_mkcolor(PAL_STKSELBKGND));
                    if(level == halScreen.StkSelStart)
                        DrawText(2, ytop, "▶", *halScreen.FontArray[FONT_STACK], cgl_mkcolor(PAL_STKCURSOR), scr);
                }
                break;
            case 2:
                // BOTH START AND END SELECTED
                if((level >= halScreen.StkSelStart)
                        && (level <= halScreen.StkSelEnd))
                    cgl_cliprect(scr, 0, ytop, xright - 1, y - 1, cgl_mkcolor(PAL_STKSELBKGND));
                if(level == halScreen.StkSelStart)
                    DrawText(2, ytop, "▶", *halScreen.FontArray[FONT_STACK], cgl_mkcolor(PAL_STKCURSOR), scr);
                if(level == halScreen.StkSelEnd)
                    DrawText(2, ytop, "▶", *halScreen.FontArray[FONT_STACK], cgl_mkcolor(PAL_STKCURSOR), scr);
                break;
            }

            // DRAW THE POINTER
            if((level <= depth) && (level == halScreen.StkPointer))
                DrawText(0, ytop, "▶", *halScreen.FontArray[FONT_STACK], cgl_mkcolor(PAL_STKCURSOR), scr);
            else if((level == 1) && (halScreen.StkPointer == 0))
                DrawText(0, ytop + (*levelfnt)->BitmapHeight / 2, "▶",
                        *halScreen.FontArray[FONT_STACK], cgl_mkcolor(PAL_STKCURSOR), scr);
            else if((level == depth) && (halScreen.StkPointer > depth))
                DrawText(0, ytop - (*levelfnt)->BitmapHeight / 2 + 1, "▶",
                        *halScreen.FontArray[FONT_STACK], cgl_mkcolor(PAL_STKCURSOR), scr);
        }

        if(level <= depth) {
            // DRAW THE NUMBER
            halInt2String(level, num);
            numwidth = StringWidth(num, *halScreen.FontArray[FONT_STACK]);

            DrawText(xright - numwidth, ytop, num,  *halScreen.FontArray[FONT_STACK], cgl_mkcolor(PAL_STKINDEX), scr);
        }

        if(level <= depth) {

            // DO PROPER LAYOUT

            BINT x = SCREEN_WIDTH - width;      // RIGHT-JUSTIFY ITEMS
            if(x < xright + 1)
                x = xright + 1; // UNLESS IT DOESN'T FIT, THEN LEFT JUSTIFY

            // DISPLAY THE ITEM

            scr->clipx = xright + 1;

            scr->x = x;
            scr->y = ytop;

            uiDrawBitmap(object, scr);

        }

        y = ytop;
        ++level;
    }

    scr->clipx = oldclipx;
    scr->clipx2 = oldclipx2;
    scr->clipy = oldclipy;
    scr->clipy2 = oldclipy2;

    halScreen.DirtyFlag &= ~STACK_DIRTY;
}

#define MABS(a) (((a)<0)? -(a):(a))

// FIND THE CLOSEST MATCHING SYSTEM FONT FOR A GIVEN HEIGHT
// DOES NOT SEARCH USER INSTALLED FONTS
WORDPTR const *halGetSystemFontbyHeight(int height)
{
    int k;
    int howclose=height;
    UNIFONT *ptr;
    WORDPTR const *selection=0;

    for(k = START_ROMPTR_INDEX; k < ROMLIB_MAX_SIZE; k += 2) {
        ptr = (UNIFONT *)rplGetFontRomPtrTableAddress()[k + 1];
        if (ptr == NULL) break;
        if(MABS(ptr->BitmapHeight-height)<howclose) {
            selection = rplGetFontRomPtrTableAddress() + k + 1;
            howclose = MABS(ptr->BitmapHeight-height);
        }
        if(!howclose) break;
    }

    return selection;
}

// UPDATE AN ARRAY WITH ALL FONTS_NUM FONT POINTERS
// NEEDS TO BE FAST
void halUpdateFontArray(WORDPTR const ** fontarray)
{
    // SET ALL POINTERS TO 0

    for (int i = 0; i < FONTS_NUM; ++i) {
        fontarray[i] = 0;
    }

    // SET CONFIGURED FONTS

    WORDPTR *var;
    if(!ISDIR(*SettingsDir))
        return;

    var = rplFindFirstByHandle(SettingsDir);

    while(var) {
        if(rplCompareIDENT(var[0], rplGetFontRomPtrTableAddress()[FONT_IDENTS_ROMPTR_INDEX + FONT_STACK])) {
            if(ISFONT(*var[1]))
                fontarray[FONT_STACK] = var + 1;
        }
        else if(rplCompareIDENT(var[0], rplGetFontRomPtrTableAddress()[FONT_IDENTS_ROMPTR_INDEX + FONT_STACKLVL1])) {
            if(ISFONT(*var[1]))
                fontarray[FONT_STACKLVL1] = var + 1;
        }
        else if(rplCompareIDENT(var[0], rplGetFontRomPtrTableAddress()[FONT_IDENTS_ROMPTR_INDEX + FONT_CMDLINE])) {
            if(ISFONT(*var[1]))
                fontarray[FONT_CMDLINE] = var + 1;
        }
        else if(rplCompareIDENT(var[0], rplGetFontRomPtrTableAddress()[FONT_IDENTS_ROMPTR_INDEX + FONT_MENU])) {
            if(ISFONT(*var[1]))
                fontarray[FONT_MENU] = var + 1;
        }
        else if(rplCompareIDENT(var[0], rplGetFontRomPtrTableAddress()[FONT_IDENTS_ROMPTR_INDEX + FONT_STATUS])) {
            if(ISFONT(*var[1]))
                fontarray[FONT_STATUS] = var + 1;
        }
        else if(rplCompareIDENT(var[0], rplGetFontRomPtrTableAddress()[FONT_IDENTS_ROMPTR_INDEX + FONT_FORMS])) {
            if(ISFONT(*var[1]))
                fontarray[FONT_FORMS] = var + 1;
        }
        else if(rplCompareIDENT(var[0], rplGetFontRomPtrTableAddress()[FONT_IDENTS_ROMPTR_INDEX + FONT_PLOT])) {
            if(ISFONT(*var[1]))
                fontarray[FONT_PLOT] = var + 1;
        }
        else if(rplCompareIDENT(var[0], rplGetFontRomPtrTableAddress()[FONT_IDENTS_ROMPTR_INDEX + FONT_HLPTEXT])) {
            if(ISFONT(*var[1]))
                fontarray[FONT_HLPTEXT] = var + 1;
        }
        else if(rplCompareIDENT(var[0], rplGetFontRomPtrTableAddress()[FONT_IDENTS_ROMPTR_INDEX + FONT_HLPTITLE])) {
            if(ISFONT(*var[1]))
                fontarray[FONT_HLPTITLE] = var + 1;
        }


        var = rplFindNext(var);
    }

    // UNCONFIGURED FONTS GET DEFAULTS

    if (fontarray[FONT_STACK] == 0) {
        fontarray[FONT_STACK] =
                halGetSystemFontbyHeight(DEF_FNTSTK_HEIGHT);
    }
    if (fontarray[FONT_STACKLVL1] == 0) {
        fontarray[FONT_STACKLVL1] =
                halGetSystemFontbyHeight(DEF_FNT1STK_HEIGHT);
    }
    if (fontarray[FONT_CMDLINE] == 0) {
        fontarray[FONT_CMDLINE] =
                halGetSystemFontbyHeight(DEF_FNTCMDL_HEIGHT);
    }
    if (fontarray[FONT_MENU] == 0) {
        fontarray[FONT_MENU] = halGetSystemFontbyHeight(DEF_FNTMENU_HEIGHT);
    }
    if (fontarray[FONT_STATUS] == 0) {
        fontarray[FONT_STATUS] =
               halGetSystemFontbyHeight(DEF_FNTSTAT_HEIGHT);
    }
    if (fontarray[FONT_PLOT] == 0) {
        fontarray[FONT_PLOT] = halGetSystemFontbyHeight(DEF_FNTPLOT_HEIGHT);
    }
    if (fontarray[FONT_FORMS] == 0) {
        fontarray[FONT_FORMS] =
                halGetSystemFontbyHeight(DEF_FNTFORM_HEIGHT);
    }
    if (fontarray[FONT_HLPTEXT] == 0) {
        fontarray[FONT_HLPTEXT] =
                halGetSystemFontbyHeight(DEF_FNTHELP_HEIGHT);
    }    if (fontarray[FONT_HLPTITLE] == 0) {
        fontarray[FONT_HLPTITLE] =
                halGetSystemFontbyHeight(DEF_FNTHLPT_HEIGHT);
    }
    return;
}

// CHANGE SYSTEM PALETTE TO A NEW THEME
// IF palette IS NULL, USE DEFAULT THEME
void halSetupTheme(WORDPTR palette)
{
    if(!palette) {
        // Setup Default color palette
        cgl_setpalette(PAL_GRAY0,THEME_GRAY0);
        cgl_setpalette(PAL_GRAY1,THEME_GRAY1);
        cgl_setpalette(PAL_GRAY2,THEME_GRAY2);
        cgl_setpalette(PAL_GRAY3,THEME_GRAY3);
        cgl_setpalette(PAL_GRAY4,THEME_GRAY4);
        cgl_setpalette(PAL_GRAY5,THEME_GRAY5);
        cgl_setpalette(PAL_GRAY6,THEME_GRAY6);
        cgl_setpalette(PAL_GRAY7,THEME_GRAY7);
        cgl_setpalette(PAL_GRAY8,THEME_GRAY8);
        cgl_setpalette(PAL_GRAY9,THEME_GRAY9);
        cgl_setpalette(PAL_GRAY10,THEME_GRAY10);
        cgl_setpalette(PAL_GRAY11,THEME_GRAY11);
        cgl_setpalette(PAL_GRAY12,THEME_GRAY12);
        cgl_setpalette(PAL_GRAY13,THEME_GRAY13);
        cgl_setpalette(PAL_GRAY14,THEME_GRAY14);
        cgl_setpalette(PAL_GRAY15,THEME_GRAY15);

        // Theme colors for the stack
        cgl_setpalette( PAL_STKBACKGND, GTHEME_STKBACKGND);
        cgl_setpalette( PAL_STKINDEX, GTHEME_STKINDEX);
        cgl_setpalette( PAL_STKVLINE, GTHEME_STKVLINE);
        cgl_setpalette( PAL_STKIDXBACKGND, GTHEME_STKIDXBACKGND);
        cgl_setpalette( PAL_STKITEMS, GTHEME_STKITEMS);
        cgl_setpalette( PAL_STKSELBKGND, GTHEME_STKSELBKGND);
        cgl_setpalette( PAL_STKSELITEM, GTHEME_STKSELITEM);
        cgl_setpalette( PAL_STKCURSOR, GTHEME_STKCURSOR);

        // Theme colors for the command line
        cgl_setpalette( PAL_CMDBACKGND, GTHEME_CMDBACKGND);
        cgl_setpalette( PAL_CMDTEXT, GTHEME_CMDTEXT);
        cgl_setpalette( PAL_CMDSELBACKGND, GTHEME_CMDSELBACKGND);
        cgl_setpalette( PAL_CMDSELTEXT, GTHEME_CMDSELTEXT);
        cgl_setpalette( PAL_CMDCURSORBACKGND, GTHEME_CMDCURSORBACKGND);
        cgl_setpalette( PAL_CMDCURSOR, GTHEME_CMDCURSOR);
        cgl_setpalette( PAL_DIVLINE, GTHEME_DIVLINE);

        // Theme colors for menu
        cgl_setpalette( PAL_MENUBACKGND, GTHEME_MENUBACKGND);
        cgl_setpalette( PAL_MENUINVBACKGND, GTHEME_MENUINVBACKGND);
        cgl_setpalette( PAL_MENUTEXT, GTHEME_MENUTEXT);
        cgl_setpalette( PAL_MENUINVTEXT, GTHEME_MENUINVTEXT);
        cgl_setpalette( PAL_MENUDIRMARK, GTHEME_MENUDIRMARK);
        cgl_setpalette( PAL_MENUINVDIRMARK, GTHEME_MENUINVDIRMARK);
        cgl_setpalette( PAL_MENUHLINE, GTHEME_MENUHLINE);
        cgl_setpalette( PAL_MENUFOCUSHLINE, GTHEME_MENUFOCUSHLINE);
        cgl_setpalette( PAL_MENUPRESSBACKGND, GTHEME_MENUPRESSBACKGND);
        cgl_setpalette( PAL_MENUPRESSINVBACKGND, GTHEME_MENUPRESSINVBACKGND);

        // Theme colors for status area
        cgl_setpalette( PAL_STABACKGND, GTHEME_STABACKGND);
        cgl_setpalette( PAL_STATEXT, GTHEME_STATEXT);
        cgl_setpalette( PAL_STAANNPRESS, GTHEME_STAANNPRESS);
        cgl_setpalette( PAL_STAANN, GTHEME_STAANN);
        cgl_setpalette( PAL_STABAT, GTHEME_STABAT);
        cgl_setpalette( PAL_STAUFLAG0, GTHEME_STAUFLAG0);
        cgl_setpalette( PAL_STAUFLAG1, GTHEME_STAUFLAG1);

        // Theme colors for help and popup messages
        cgl_setpalette( PAL_HLPBACKGND, GTHEME_HLPBACKGND);
        cgl_setpalette( PAL_HLPTEXT, GTHEME_HLPTEXT);
        cgl_setpalette( PAL_HLPLINES,GTHEME_HLPLINES);

        // Theme colors for Forms
        cgl_setpalette( PAL_FORMBACKGND, GTHEME_FORMBACKGND);
        cgl_setpalette( PAL_FORMTEXT, GTHEME_FORMTEXT);
        cgl_setpalette( PAL_FORMSELTEXT, GTHEME_FORMSELTEXT);
        cgl_setpalette( PAL_FORMSELBACKGND, GTHEME_FORMSELBACKGND);
        cgl_setpalette( PAL_FORMCURSOR, GTHEME_FORMCURSOR);

        // More default colors here


        return;
    }


    // Replace the palette completely

    for(int k=0;k<PALETTESIZE;++k)
    {
        cgl_setpalette(k,palette[k]);
    }

    // Make sure all items are rendered again using the new colors

    uiClearRenderCache();

}

// INITIALIZE DEFAULT SCREEN PARAMETERS

void halInitScreen()
{

    // RESTORE THE SCREEN
    WORDPTR saved = rplGetSettings((WORDPTR) screenconfig_ident);
    if(saved) {
        // JUST THE CONTRAST SETTINGS FOR NOW
        if(ISBINT(*saved))
            lcd_setcontrast(rplReadBINT(saved));
    }

    halSetupTheme(NULL);        // SETUP DEFAULT THEME

    halUpdateFontArray((WORDPTR const **) halScreen.FontArray);
    int k;
    for(k = 0; k < FONTS_NUM; ++k)
        halScreen.FontHash[k] = 0;

    halFlags = 0;
    halProcesses[0] = halProcesses[1] = halProcesses[2] = 0;
    halScreen.HelpMode = 0;
    halScreen.CmdLine = 0;
    halScreen.Menu1 = MENU1_HEIGHT;
    halScreen.Menu2 = MENU2_HEIGHT;
    halScreen.Stack = 1;
    halSetFormHeight(0);
    halScreen.DirtyFlag =
            STACK_DIRTY | MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY;
    halScreen.SAreaTimer = 0;
    halScreen.CursorTimer = -1;
    halScreen.KeyContext = CONTEXT_STACK;
    halSetNotification(N_LEFTSHIFT, 0);
    halSetNotification(N_RIGHTSHIFT, 0);
    halSetNotification(N_ALPHA, 0);
    halSetNotification(N_LOWBATTERY, 0);
    halSetNotification(N_HOURGLASS, 0);
    halSetNotification(N_DATARECVD, 0);

// NOT NECESSARILY PART OF HALSCREEN, BUT INITIALIZE THE COMMAND LINE
    uiCloseCmdLine();
    halScreen.StkUndolevels = 8;
    halScreen.StkCurrentLevel = 0;

    halScreen.StkVisibleLvl = 1;
    halScreen.StkVisibleOffset = 0;

}

void halRedrawHelp(DRAWSURFACE * scr)
{

    if(!halScreen.Menu2) {
        // SHOW THE SECOND MENU TO DISPLAY THE MESSAGE
        halSetMenu2Height(MENU2_HEIGHT);
        halRedrawAll(scr);      // THIS CALL WILL CALL HERE RECURSIVELY
        return; // SO IT'S BEST TO RETURN DIRECTLY
    }

    halScreenUpdated();

    WORDPTR helptext;
    BINT64 m1code = rplGetMenuCode(halScreen.HelpMode >> 16);
    WORDPTR MenuObj = uiGetLibMenu(m1code);
    BINT nitems = uiCountMenuItems(m1code, MenuObj);
    BINT k;
    WORDPTR item;

    // BASIC CHECK OF VALIDITY - COMMANDS MAY HAVE RENDERED THE PAGE NUMBER INVALID
    // FOR EXAMPLE BY PURGING VARIABLES
    if(MENUPAGE(m1code) >= (WORD) nitems) {
        m1code = SETMENUPAGE(m1code, 0);
        rplSetMenuCode(halScreen.HelpMode >> 16, m1code);
    }

    if(((halScreen.HelpMode & 0xffff) == 5) && (nitems > 6)) {
        halScreen.HelpMode = 0; // CLOSE HELP MODE IMMEDIATELY
        halRedrawAll(scr);      // AND ISSUE A REDRAW
        return;
    }

    // GET THE ITEM
    item = uiGetMenuItem(m1code, MenuObj,
            (halScreen.HelpMode & 0xffff) + MENUPAGE(m1code));
    helptext = uiGetMenuItemHelp(item);

    if(!helptext) {
        halScreen.HelpMode = 0; // CLOSE HELP MODE IMMEDIATELY
        halRedrawAll(scr);      // AND ISSUE A REDRAW
        return;
    }

    if(ISIDENT(*helptext)) {
        // THE HELP TEXT SHOULD BE THE OBJECT DECOMPILED

        // SPECIAL CASE: FOR IDENTS LOOK FOR VARIABLES AND DRAW DIFFERENTLY IF IT'S A DIRECTORY
        WORDPTR *var = rplFindGlobal(helptext, 1);

        if(!var) {
            halScreen.HelpMode = 0;     // CLOSE HELP MODE IMMEDIATELY
            halRedrawAll(scr);  // AND ISSUE A REDRAW
            return;
        }

        BINT SavedException = Exceptions;
        BINT SavedErrorCode = ErrorCode;

        Exceptions = 0; // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
        // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
        WORDPTR objdecomp = rplDecompile(var[1], DECOMP_NOHINTS);
        Exceptions = SavedException;
        ErrorCode = SavedErrorCode;

        if(!objdecomp)
            helptext = (WORDPTR) empty_string;
        else
            helptext = objdecomp;

        BINT ytop = halScreen.Form + halScreen.Stack + halScreen.CmdLine;
        BINT ybot = ytop + halScreen.Menu1 + halScreen.Menu2 - 1;

        // CLEAR MENU2 AND STATUS AREA
        cgl_cliprect(scr, 0, ytop, SCREEN_WIDTH - 1, ybot, cgl_mkcolor(PAL_HLPBACKGND));
        // DO SOME DECORATIVE ELEMENTS
        cgl_cliphline(scr, ytop, 0, SCREEN_WIDTH - 1, cgl_mkcolor(PAL_HLPLINES));

        // SHOW 3 LINES ONLY

        BINT namew =
                StringWidthN((char *)(var[0] + 1),
                ((char *)(var[0] + 1)) + rplGetIdentLength(var[0]),
                *halScreen.FontArray[FONT_HLPTITLE]);

        // SHOW THE NAME OF THE VARIABLE
        DrawTextN(3, ytop + 2, (char *)(var[0] + 1),
                ((char *)(var[0] + 1)) + rplGetIdentLength(var[0]),
                *halScreen.FontArray[FONT_HLPTITLE], cgl_mkcolor(PAL_HLPTEXT), scr);
        DrawText(3 + namew, ytop + 2, ": ", *halScreen.FontArray[FONT_HLPTITLE],
                cgl_mkcolor(PAL_HLPTEXT), scr);
        namew += 3 + StringWidth(": ", *halScreen.FontArray[FONT_HLPTITLE]);

        int xend;
        BYTEPTR basetext = (BYTEPTR) (helptext + 1);
        BYTEPTR endoftext = basetext + rplStrSize(helptext);
        BYTEPTR nextline, endofline;

        for(k = 0; k < 3; ++k) {
            xend = SCREEN_WIDTH - 1 - namew;
            endofline =
                    (BYTEPTR) StringCoordToPointer((char *)basetext,
                    (char *)endoftext, *halScreen.FontArray[FONT_HLPTEXT],
                    &xend);
            if(endofline < endoftext) {
                // BACK UP TO THE NEXT WHITE CHARACTER
                BYTEPTR whitesp = endofline;
                while((whitesp > basetext) && (*whitesp != ' '))
                    --whitesp;
                if(whitesp >= basetext)
                    endofline = whitesp;        // ONLY IF THERE'S WHITESPACES
            }
            nextline = endofline;
            // SKIP ANY NEWLINE OR WHITE CHARACTERS
            while((nextline < endoftext) && ((*nextline == ' ')
                        || (*nextline == '\n') || (*nextline == '\t')
                        || (*nextline == '\r')))
                ++nextline;

            // DRAW THE TEXT
            DrawTextN(namew,
                    ytop + 2 + (*halScreen.FontArray[FONT_HLPTITLE])->BitmapHeight +
                    (k-1) * (*halScreen.FontArray[FONT_HLPTEXT])->BitmapHeight,
                    (char *)basetext, (char *)endofline,
                    *halScreen.FontArray[FONT_HLPTEXT], cgl_mkcolor(PAL_HLPTEXT), scr);
            basetext = nextline;
            namew = 3;
        }

        return;

    }

    if(ISSTRING(*helptext)) {

        BINT ytop = halScreen.Form + halScreen.Stack + halScreen.CmdLine;
        BINT ybot = ytop + halScreen.Menu1 + halScreen.Menu2 - 1;

        // CLEAR MENU2 AND STATUS AREA
        cgl_cliprect(scr, 0, ytop, SCREEN_WIDTH - 1, ybot,  cgl_mkcolor(PAL_HLPBACKGND));
        // DO SOME DECORATIVE ELEMENTS
        cgl_cliphline(scr, ytop, 0, SCREEN_WIDTH - 1,cgl_mkcolor(PAL_HLPLINES));

        // SHOW MESSAGE'S FIRST 3 LINES ONLY
        BINT currentline = 0, nextline;
        BYTEPTR basetext = (BYTEPTR) (helptext + 1);
        for(k = 0; k < 3; ++k) {
            nextline = rplStringGetLinePtr(helptext, 2 + k);
            if(nextline < 0) {
                nextline = rplStrSize(helptext);
            }
            DrawTextN(3,
                    ytop + 2 + (*halScreen.FontArray[FONT_HLPTITLE])->BitmapHeight + k * (*halScreen.FontArray[FONT_HLPTEXT])->BitmapHeight,
                    (char *)basetext + currentline, (char *)basetext + nextline,
                    *halScreen.FontArray[FONT_HLPTEXT], cgl_mkcolor(PAL_HLPTEXT), scr);

            currentline = nextline;
        }

        // FINALLY, SHOW THE NAME OF THE ITEM
        BINT oldclipy = scr->clipy, oldclipy2 = scr->clipy2;

        scr->clipy = ytop + 1;
        scr->clipy2 =
                ytop + 1 + (*halScreen.FontArray[FONT_HLPTITLE])->BitmapHeight;

        uiDrawMenuItem(item, PAL_HLPTEXT, PAL_HLPBACKGND, scr);

        scr->clipy = oldclipy;
        scr->clipy2 = oldclipy2;

        return;
    }

}

// REDRAW THE VARS MENU
void halRedrawMenu1(DRAWSURFACE * scr)
{
    if(halScreen.HelpMode) {
        // IN HELP MODE, JUST REDRAW THE HELP MESSAGE
        halRedrawHelp(scr);
        // NO NEED TO REDRAW OTHER MENUS OR THE STATUS AREA
        halScreen.DirtyFlag &= ~(MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY);
        return;
    }
    if(halScreen.Menu1 == 0) {
        halScreen.DirtyFlag &= ~MENU1_DIRTY;
        return;
    }

    halScreenUpdated();

    int mcolor,bcolor,mpalette,bpalette;

    if(rplTestSystemFlag(FL_MENU1WHITE)) { mpalette=PAL_MENUINVTEXT; bpalette=PAL_MENUINVBACKGND; mcolor=cgl_mkcolor(PAL_MENUINVTEXT); bcolor=cgl_mkcolor(PAL_MENUINVBACKGND); }
    else { mpalette=PAL_MENUTEXT; bpalette=PAL_MENUBACKGND; mcolor=cgl_mkcolor(PAL_MENUTEXT); bcolor=cgl_mkcolor(PAL_MENUBACKGND); }

    int ytop, ybottom;
    int oldclipx, oldclipx2, oldclipy, oldclipy2;

    ytop = halScreen.Form + halScreen.Stack + halScreen.CmdLine;
    ybottom = ytop + halScreen.Menu1 - 1;
    // DRAW BACKGROUND
    cgl_cliprect(scr, 0, ytop + 1, SCREEN_WIDTH - 1, ybottom - 1,
            bcolor);
    cgl_cliphline(scr, ytop, 0, SCREEN_WIDTH - 1, cgl_mkcolor(PAL_MENUHLINE));
    cgl_cliphline(scr, ybottom, 0, SCREEN_WIDTH - 1, cgl_mkcolor(PAL_MENUHLINE));

    // DRAW VARS OF THE CURRENT DIRECTORY IN THIS MENU

    oldclipx = scr->clipx;
    oldclipx2 = scr->clipx2;
    oldclipy = scr->clipy;
    oldclipy2 = scr->clipy2;

    BINT64 m1code = rplGetMenuCode(1);
    WORDPTR MenuObj = uiGetLibMenu(m1code);
    BINT nitems = uiCountMenuItems(m1code, MenuObj);
    BINT k;
    WORDPTR item;

    // BASIC CHECK OF VALIDITY - COMMANDS MAY HAVE RENDERED THE PAGE NUMBER INVALID
    // FOR EXAMPLE BY PURGING VARIABLES
    if((MENUPAGE(m1code) >= (WORD) nitems) || (nitems <= 6)) {
        m1code = SETMENUPAGE(m1code, 0);
        rplSetMenuCode(1, m1code);
    }

    // FIRST ROW

    scr->clipy = ytop + 1;
    scr->clipy2 = ytop + MENU1_HEIGHT - 2;

    for(k = 0; k < 5; ++k) {
        scr->clipx = MENU_TAB_WIDTH * k;
        scr->clipx2 = MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH-2);
        item = uiGetMenuItem(m1code, MenuObj, k + MENUPAGE(m1code));
        uiDrawMenuItem(item, mpalette, bpalette, scr);
    }

    // NOW DO THE NXT KEY
    scr->clipx = MENU_TAB_WIDTH * k;
    scr->clipx2 = MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH-2);

    if(nitems == 6) {
        item = uiGetMenuItem(m1code, MenuObj, 5);
        uiDrawMenuItem(item, mpalette, bpalette, scr);
    }
    else {
        if(nitems > 6) {
            DrawText(scr->clipx + 1, scr->clipy + 1, "NXT...",
                    *halScreen.FontArray[FONT_MENU], mcolor, scr);
        }
    }

    scr->clipx = oldclipx;
    scr->clipx2 = oldclipx2;
    scr->clipy = oldclipy;
    scr->clipy2 = oldclipy2;

    halScreen.DirtyFlag &= ~MENU1_DIRTY;
}

// REDRAW THE OTHER MENU
void halRedrawMenu2(DRAWSURFACE * scr)
{
    if(halScreen.HelpMode) {
        // IN HELP MODE, JUST REDRAW THE HELP MESSAGE
        halRedrawHelp(scr);
        // NO NEED TO REDRAW OTHER MENUS OR THE STATUS AREA
        halScreen.DirtyFlag &= ~(MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY);
        return;
    }

    if(halScreen.Menu2 == 0) {
        halScreen.DirtyFlag &= ~MENU2_DIRTY;
        return;
    }

    halScreenUpdated();

    int mcolor,bcolor,mpalette,bpalette;

    if(rplTestSystemFlag(FL_MENU2WHITE)) { mpalette=PAL_MENUINVTEXT; bpalette=PAL_MENUINVBACKGND; mcolor=cgl_mkcolor(PAL_MENUINVTEXT); bcolor=cgl_mkcolor(PAL_MENUINVBACKGND); }
    else { mpalette=PAL_MENUTEXT; bpalette=PAL_MENUBACKGND; mcolor=cgl_mkcolor(PAL_MENUTEXT); bcolor=cgl_mkcolor(PAL_MENUBACKGND); }


    int ytop, ybottom;
    int oldclipx, oldclipx2, oldclipy, oldclipy2;

    ytop = halScreen.Form + halScreen.Stack + halScreen.CmdLine +
            halScreen.Menu1;
    ybottom = ytop + halScreen.Menu2 - 1;
    // DRAW BACKGROUND
    cgl_cliprect(scr, 0, ytop, STATUSAREA_X - 1, ybottom,
            bcolor);
    //ggl_clipvline(scr,21,ytop+1,ybottom,ggl_mkcolor(0x8));
    //ggl_clipvline(scr,43,ytop+1,ybottom,ggl_mkcolor(0x8));
    //ggl_clipvline(scr,STATUSAREA_X-1,ytop+1,ybottom,ggl_mkcolor(0x8));
//    ggl_clipvline(scr,87,ytop,ybottom,0);
//    ggl_clipvline(scr,109,ytop,ybottom,0);
    //ggl_cliphline(scr,ytop,0,SCREEN_WIDTH-1,ggl_mkcolor(0x8));
    cgl_cliphline(scr, ytop + MENU2_HEIGHT / 2 - 1, 0, STATUSAREA_X - 2,
            cgl_mkcolor(PAL_MENUHLINE));
    cgl_cliphline(scr, ybottom, 0, STATUSAREA_X - 2, cgl_mkcolor(PAL_MENUHLINE));

    // DRAW VARS OF THE CURRENT DIRECTORY IN THIS MENU

    oldclipx = scr->clipx;
    oldclipx2 = scr->clipx2;
    oldclipy = scr->clipy;
    oldclipy2 = scr->clipy2;

    BINT64 m2code = rplGetMenuCode(2);
    WORDPTR MenuObj = uiGetLibMenu(m2code);
    BINT nitems = uiCountMenuItems(m2code, MenuObj);
    BINT k;
    WORDPTR item;

    // BASIC CHECK OF VALIDITY - COMMANDS MAY HAVE RENDERED THE PAGE NUMBER INVALID
    // FOR EXAMPLE BY PURGING VARIABLES
    if((MENUPAGE(m2code) >= (WORD) nitems) || (nitems <= 6)) {
        m2code = SETMENUPAGE(m2code, 0);
        rplSetMenuCode(2, m2code);
    }

    // FIRST ROW

    scr->clipy = ytop;
    scr->clipy2 = ytop + MENU2_HEIGHT / 2 - 2;

    for(k = 0; k < 3; ++k) {
        scr->clipx = MENU_TAB_WIDTH * k;
        scr->clipx2 = MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH-2);
        item = uiGetMenuItem(m2code, MenuObj, k + MENUPAGE(m2code));
        uiDrawMenuItem(item, mpalette, bpalette, scr);
    }

    // SECOND ROW

    scr->clipy = ytop + MENU2_HEIGHT / 2;
    scr->clipy2 = ybottom - 1;

    for(k = 0; k < 2; ++k) {
        scr->clipx = MENU_TAB_WIDTH * k;
        scr->clipx2 = MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH-2);
        item = uiGetMenuItem(m2code, MenuObj, k + 3 + MENUPAGE(m2code));
        uiDrawMenuItem(item, mpalette, bpalette, scr);
    }

    // NOW DO THE NXT KEY
    scr->clipx = MENU_TAB_WIDTH * k;
    scr->clipx2 = MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH-2);

    if(nitems == 6) {
        item = uiGetMenuItem(m2code, MenuObj, 5);
        uiDrawMenuItem(item, mpalette, bpalette, scr);
    }
    else {
        if(nitems > 6) {
            DrawText(scr->clipx + 1, scr->clipy + 1, "NXT...",
                    *halScreen.FontArray[FONT_MENU], mcolor, scr);
        }
    }

    scr->clipx = oldclipx;
    scr->clipx2 = oldclipx2;
    scr->clipy = oldclipy;
    scr->clipy2 = oldclipy2;

    halScreen.DirtyFlag &= ~MENU2_DIRTY;
}

void halRedrawStatus(DRAWSURFACE * scr)
{
    if(halScreen.HelpMode) {
        // IN HELP MODE, JUST REDRAW THE HELP MESSAGE
        halRedrawHelp(scr);
        // NO NEED TO REDRAW OTHER MENUS OR THE STATUS AREA
        halScreen.DirtyFlag &= ~(MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY);
        return;
    }

    halScreenUpdated();

    if(halScreen.Menu2) {
        int ytop =
                halScreen.Form + halScreen.Stack + halScreen.CmdLine +
                halScreen.Menu1;
        cgl_cliprect(scr, STATUSAREA_X, ytop, SCREEN_WIDTH - 1,
                ytop + halScreen.Menu2 - 1, cgl_mkcolor(PAL_STABACKGND));
        BINT xc, yc;
        xc = scr->clipx;
        yc = scr->clipy;
        scr->clipx = STATUSAREA_X;
        scr->clipy = ytop;

        // AUTOCOMPLETE

        if(halScreen.CmdLineState & CMDSTATE_ACACTIVE) {
            BYTEPTR namest;
            BYTEPTR nameend;
            if(halScreen.ACSuggestion != 0) {
                // DISPLAY THE CURRENTLY SELECTED AUTOCOMPLETE COMMAND IN THE
                // SECOND LINE
                if(!Exceptions) {
                    // BUT ONLY IF THERE WERE NO ERRORS
                    BINT y = ytop + 1 +
                            (*halScreen.FontArray[FONT_STATUS])->BitmapHeight;
                    // FOR NOW JUST DISPLAY THE SELECTED TOKEN
                    WORDPTR cmdname =
                            rplDecompile(((ISPROLOG(halScreen.ACSuggestion)
                                    && SuggestedObject) ? SuggestedObject
                                : (&halScreen.ACSuggestion)), DECOMP_NOHINTS);
                    if((!cmdname) || Exceptions) {
                        // JUST IGNORE, CLEAR EXCEPTIONS AND RETURN;
                        Exceptions = 0;
                        halScreen.DirtyFlag &= ~STAREA_DIRTY;
                        return;
                    }

                    namest = (BYTEPTR) (cmdname + 1);
                    nameend = namest + rplStrSize(cmdname);
                    DrawTextBkN(STATUSAREA_X + 2, y, (char *)namest,
                            (char *)nameend, *halScreen.FontArray[FONT_STATUS],
                            cgl_mkcolor(PAL_STATEXT), cgl_mkcolor(PAL_STABACKGND), scr);

                }
            }

        }
        else {
            // SHOW CURRENT PATH ON SECOND LINE
            BINT nnames, j, width, xst;
            WORDPTR pathnames[8], lastword;
            BYTEPTR start, end;
            BINT y = ytop + 1 +
                    (*halScreen.FontArray[FONT_STATUS])->BitmapHeight;

            nnames = rplGetFullPath(CurrentDir, pathnames, 8);

            // COMPUTE THE WIDTH OF ALL NAMES
            width = 0;
            for(j = nnames - 1; j >= 0; --j) {
                if(ISIDENT(*pathnames[j])) {
                    start = (BYTEPTR) (pathnames[j] + 1);
                    lastword = rplSkipOb(pathnames[j]) - 1;
                    if(*lastword & 0xff000000) {
                        end = (BYTEPTR) (lastword + 1);
                        width += StringWidthN((char *)start, (char *)end,
                                *halScreen.FontArray[FONT_STATUS]);
                    }
                    else
                        width += StringWidth((char *)start,
                                *halScreen.FontArray[FONT_STATUS]);

                }
            }
            // ADD WIDTH OF SYMBOLS
            width += 4 * nnames;
            if(width > SCREEN_WIDTH - STATUSAREA_X)
                xst = SCREEN_WIDTH - width;
            else
                xst = STATUSAREA_X;

            // NOW DRAW THE PATH
            for(j = nnames - 1; j >= 0; --j) {
                if(ISIDENT(*pathnames[j])) {
                    start = (BYTEPTR) (pathnames[j] + 1);
                    lastword = rplSkipOb(pathnames[j]) - 1;
                    DrawTextBk(xst, y, "/", *halScreen.FontArray[FONT_STATUS],
                            cgl_mkcolor(PAL_STATEXT), cgl_mkcolor(PAL_STABACKGND), scr);
                    xst = scr->x;
                    if(*lastword & 0xff000000) {
                        end = (BYTEPTR) (lastword + 1);
                        DrawTextBkN(xst, y, (char *)start, (char *)end,
                                *halScreen.FontArray[FONT_STATUS], cgl_mkcolor(PAL_STATEXT), cgl_mkcolor(PAL_STABACKGND), scr);
                    }
                    else
                        DrawTextBk(xst, y, (char *)start,
                                *halScreen.FontArray[FONT_STATUS], cgl_mkcolor(PAL_STATEXT), cgl_mkcolor(PAL_STABACKGND), scr);

                    xst = scr->x;
                }

            }
            /*
            if(width > SCREEN_WIDTH - STATUSAREA_X) {
                // FADE THE TEXT OUT

                scr->x = STATUSAREA_X;
                ggl_filter(scr, 2,
                        (*halScreen.FontArray[FONT_STATUS])->BitmapHeight, 0xA,
                        &ggl_fltlighten);
                scr->x += 2;
                ggl_filter(scr, 2,
                        (*halScreen.FontArray[FONT_STATUS])->BitmapHeight, 0x6,
                        &ggl_fltlighten);
                scr->x += 2;
                ggl_filter(scr, 2,
                        (*halScreen.FontArray[FONT_STATUS])->BitmapHeight, 0x4,
                        &ggl_fltlighten);

            }
            */
            if(width > SCREEN_WIDTH - STATUSAREA_X) {

                int rf,rb,gf,gb,bf,bb;

                rf=G2RGBRED(cgl_mkcolor(PAL_STATEXT));
                rb=G2RGBRED(cgl_mkcolor(PAL_STABACKGND));
                gf=G2RGBGREEN(cgl_mkcolor(PAL_STATEXT));
                gb=G2RGBGREEN(cgl_mkcolor(PAL_STABACKGND));
                bf=G2RGBBLUE(cgl_mkcolor(PAL_STATEXT));
                bb=G2RGBBLUE(cgl_mkcolor(PAL_STABACKGND));

                // CREATE 3 INTERPOLATED COLORS: (3F+B)/4, (F+B)/2 AND (F+3B)/4
                int vanishwidth=MENU_TAB_WIDTH/16;

                scr->x = STATUSAREA_X;

                cgl_filter(scr, vanishwidth, (*halScreen.FontArray[FONT_STATUS])->BitmapHeight,
                       RGB_TO_RGB16( (rf+3*rb)>>2 , (gf+3*gb)>>2 , (bf+3*bb)>>2) | (cgl_mkcolor(PAL_STATEXT)<<16) , &cgl_fltreplace);
                scr->x+=vanishwidth;
                cgl_filter(scr, vanishwidth, (*halScreen.FontArray[FONT_STATUS])->BitmapHeight,
                        RGB_TO_RGB16( (rf+rb)>>1 , (gf+gb)>>1 , (bf+bb)>>1) | (cgl_mkcolor(PAL_STATEXT)<<16), &cgl_fltreplace);
                scr->x+=vanishwidth;
                cgl_filter(scr, vanishwidth, (*halScreen.FontArray[FONT_STATUS])->BitmapHeight,
                        RGB_TO_RGB16( (3*rf+rb)>>2 , (3*gf+gb)>>2 , (3*bf+bb)>>2) | (cgl_mkcolor(PAL_STATEXT)<<16), &cgl_fltreplace);

            }


        }

        // ANGLE MODE INDICATOR

        {
            BINT anglemode =
                    rplTestSystemFlag(FL_ANGLEMODE1) |
                    (rplTestSystemFlag(FL_ANGLEMODE2) << 1);
            const char *const name[4] = {
                "∡°",
                "∡r",
                "∡g",
                "∡d"
            };

            DrawTextBk(STATUSAREA_X + 1, ytop + 1, (char *)name[anglemode],
                    *halScreen.FontArray[FONT_STATUS], cgl_mkcolor(PAL_STATEXT), cgl_mkcolor(PAL_STABACKGND), scr);
        }

        // COMPLEX MODE INDICATOR

        if(rplTestSystemFlag(FL_COMPLEXMODE))
            DrawTextBk(STATUSAREA_X + 14, ytop + 1, (char *)"C",
                    *halScreen.FontArray[FONT_STATUS], cgl_mkcolor(PAL_STATEXT), cgl_mkcolor(PAL_STABACKGND), scr);

        // HALTED PROGRAM INDICATOR

        if(halFlags & HAL_HALTED)
            DrawTextBk(STATUSAREA_X + 20, ytop + 1, (char *)"H",
                    *halScreen.FontArray[FONT_STATUS], cgl_mkcolor(PAL_STATEXT), cgl_mkcolor(PAL_STABACKGND), scr);

        // FIRST 6 USER FLAGS
        {
            UBINT64 *flags = rplGetUserFlagsLow();
            if(flags) {
                    cgl_rect(scr, STATUSAREA_X + 30 , ytop + 2,
                        STATUSAREA_X + 36 , ytop + 6, cgl_mkcolor(PAL_STAUFLAG0));

                if(*flags & 4)
                    cgl_rect(scr, STATUSAREA_X + 30   , ytop + 2,
                            STATUSAREA_X + 31 , ytop + 3, cgl_mkcolor(PAL_STAUFLAG1));
                if(*flags & 2)
                    cgl_rect(scr, STATUSAREA_X + 32 , ytop + 2,
                            STATUSAREA_X + 33 , ytop + 3, cgl_mkcolor(PAL_STAUFLAG1));
                if(*flags & 1)
                    cgl_rect(scr, STATUSAREA_X + 34 , ytop + 2,
                            STATUSAREA_X + 35 , ytop + 3, cgl_mkcolor(PAL_STAUFLAG1));

                if(*flags & 32)
                    cgl_rect(scr, STATUSAREA_X + 30 , ytop + 5,
                            STATUSAREA_X + 31 , ytop + 6, cgl_mkcolor(PAL_STAUFLAG1));
                if(*flags & 16)
                    cgl_rect(scr, STATUSAREA_X + 32 , ytop + 5,
                            STATUSAREA_X + 33 , ytop + 6, cgl_mkcolor(PAL_STAUFLAG1));
                if(*flags & 8)
                    cgl_rect(scr, STATUSAREA_X + 34 , ytop + 5,
                            STATUSAREA_X + 35 , ytop + 6, cgl_mkcolor(PAL_STAUFLAG1));
            }



        }

        // NOTIFICATION ICONS! ONLY ONE WILL BE DISPLAYED AT A TIME

        if(halGetNotification(N_ALARM)) {
            char txt[4];
            txt[0] = 'A';
            txt[1] = 'L';
            txt[2] = 'M';
            txt[3] = 0;
            DrawTextBk(STATUSAREA_X + 38, ytop + 1, txt,
                    *halScreen.FontArray[FONT_STATUS], cgl_mkcolor(PAL_STATEXT), cgl_mkcolor(PAL_STABACKGND), scr);
        }
        else if(halGetNotification(N_DATARECVD)) {
            char txt[4];
            txt[0] = 'R';
            txt[1] = 'X';
            txt[2] = ' ';
            txt[3] = 0;
            DrawTextBk(STATUSAREA_X + 38, ytop + 1, txt,
                    *halScreen.FontArray[FONT_STATUS], cgl_mkcolor(PAL_STATEXT), cgl_mkcolor(PAL_STABACKGND), scr);
        }

#ifndef CONFIG_NO_FSYSTEM

        // SD CARD INSERTED INDICATOR
        {
            char txt[4];
            int color;
            txt[0] = 'S';
            txt[1] = 'D';
            txt[2] = ' ';
            txt[3] = 0;
            if(FSCardInserted())
                color = 6;
            else
                color = 0;
            if(FSIsInit()) {
                if(FSVolumeMounted(FSGetCurrentVolume()))
                    color = 0xf;
                if(!FSCardInserted()) {
                    txt[2] = '?';
                    color = 6;
                }
                else if(FSCardIsSDHC()) {
                    txt[0] = 'H';
                    txt[1] = 'C';
                }
                int k = FSIsDirty();
                if(k == 1)
                    color = -1; // 1 =  DIRTY FS NEEDS FLUSH
                if(k == 2)
                    color = -2; // 2 =  FS IS FLUSHED BUT THERE'S OPEN FILES
            }

            if(color) {
                if(color == -1)
                    DrawTextBk(STATUSAREA_X + 53, ytop + 1, txt,
                            *halScreen.FontArray[FONT_STATUS], cgl_mkcolor(PAL_STABACKGND), cgl_mkcolor(PAL_STATEXT), scr);
                else {
                    if(color == -2)
                        DrawTextBk(STATUSAREA_X + 53, ytop + 1, txt,
                                *halScreen.FontArray[FONT_STATUS], cgl_mkcolor(PAL_STABACKGND), cgl_mkcolor(PAL_STATEXT), scr);
                    else
                        DrawTextBk(STATUSAREA_X + 53, ytop + 1, txt,
                                *halScreen.FontArray[FONT_STATUS], cgl_mkcolor(PAL_STATEXT), cgl_mkcolor(PAL_STABACKGND),
                                scr);
                }
            }

        }

#endif

        // ADD OTHER INDICATORS HERE

        scr->clipx = xc;
        scr->clipy = yc;
    }

    halScreen.DirtyFlag &= ~STAREA_DIRTY;

}

void halRedrawCmdLine(DRAWSURFACE * scr)
{
    halScreenUpdated();

    if(halScreen.CmdLine) {
        int ytop = halScreen.Form + halScreen.Stack;
        if((halScreen.DirtyFlag & CMDLINE_ALLDIRTY) == CMDLINE_ALLDIRTY) {
            //ggl_cliprect(scr,0,ytop,SCREEN_WIDTH-1,ytop+halScreen.CmdLine-1,0);
            cgl_cliphline(scr, ytop, 0, SCREEN_WIDTH - 1, cgl_mkcolor(PAL_DIVLINE));
            cgl_cliphline(scr, ytop + 1, 0, SCREEN_WIDTH - 1, cgl_mkcolor(PAL_CMDBACKGND));
        }

        BINT y = (halScreen.LineCurrent -
                halScreen.LineVisible) *
                (*halScreen.FontArray[FONT_CMDLINE])->BitmapHeight;
        BYTEPTR cmdline = (BYTEPTR) (CmdLineCurrentLine + 1);
        BINT nchars = rplStrSize(CmdLineCurrentLine);

        if(halScreen.DirtyFlag & CMDLINE_DIRTY) {
            // SHOW OTHER LINES HERE EXCEPT THE CURRENT EDITED LINE
            BINT k;
            BINT startoff = -1;
            BINT endoff;

            for(k = 0; k < halScreen.NumLinesVisible; ++k) {
                // UPDATE THE LINE
                if(halScreen.LineVisible + k < 1)
                    continue;
                //if(halScreen.LineVisible+k>totallines) break;

                if(halScreen.LineVisible + k == halScreen.LineCurrent) {
                    if(startoff < 0)
                        continue;
                    startoff = endoff;
                    if(startoff < 0)
                        endoff = -1;
                    else
                        endoff = rplStringGetNextLine(CmdLineText, startoff);
                    continue;
                }
                if(startoff < 0)
                    startoff =
                            rplStringGetLinePtr(CmdLineText,
                            halScreen.LineVisible + k);
                else
                    startoff = endoff;

                if(startoff < 0)
                    endoff = -1;
                else
                    endoff = rplStringGetNextLine(CmdLineText, startoff);

                BINT xcoord, tail;
                xcoord = -halScreen.XVisible;

                if((startoff >= 0) || (endoff >= 0)) {
                    BYTEPTR string = (BYTEPTR) (CmdLineText + 1) + startoff;
                    BYTEPTR selst, selend;
                    BYTEPTR strend;

                    if((startoff >= 0) && (endoff < 0))
                        strend = (BYTEPTR) (CmdLineText + 1) +
                                rplStrSize(CmdLineText);
                    else
                        strend = (BYTEPTR) (CmdLineText + 1) + endoff;

                    selst = selend = strend;
                    tail = 0;
                    if(halScreen.SelStartLine < halScreen.LineVisible + k) {
                        selst = string;
                        tail = 1;
                    }
                    if(halScreen.SelStartLine == halScreen.LineVisible + k) {
                        selst = string + halScreen.SelStart;
                        tail = 1;
                    }
                    if(halScreen.SelEndLine < halScreen.LineVisible + k) {
                        selend = string;
                        tail = 0;
                    }
                    if(halScreen.SelEndLine == halScreen.LineVisible + k) {
                        selend = string + halScreen.SelEnd;
                        tail = 0;
                    }

                    if(selend <= selst)
                        selend = selst = string;

                    // DRAW THE LINE SPLIT IN 3 SECTIONS: string TO selst, selst TO selend, selend TO strend

                    if(selst > string) {
                        DrawTextBkN(xcoord,
                                ytop + 2 +
                                k *
                                (*halScreen.FontArray[FONT_CMDLINE])->
                                BitmapHeight, (char *)string, (char *)selst,
                                *halScreen.FontArray[FONT_CMDLINE], cgl_mkcolor(PAL_CMDTEXT), cgl_mkcolor(PAL_CMDBACKGND),
                                scr);
                        //xcoord+=StringWidthN((char *)string,(char *)selst,(UNIFONT *)halScreen.FontArray[FONT_CMDLINE]);
                        xcoord = scr->x;
                    }
                    if(selend > selst) {
                        DrawTextBkN(xcoord,
                                ytop + 2 +
                                k *
                                (*halScreen.FontArray[FONT_CMDLINE])->
                                BitmapHeight, (char *)selst, (char *)selend,
                                *halScreen.FontArray[FONT_CMDLINE], cgl_mkcolor(PAL_CMDSELTEXT), cgl_mkcolor(PAL_CMDSELBACKGND),
                                scr);
                        //xcoord+=StringWidthN((char *)selst,(char *)selend,(UNIFONT *)halScreen.FontArray[FONT_CMDLINE]);
                        xcoord = scr->x;
                    }
                    if(strend > selend) {
                        DrawTextBkN(xcoord,
                                ytop + 2 +
                                k *
                                (*halScreen.FontArray[FONT_CMDLINE])->
                                BitmapHeight, (char *)selend, (char *)strend,
                                *halScreen.FontArray[FONT_CMDLINE], cgl_mkcolor(PAL_CMDTEXT), cgl_mkcolor(PAL_CMDBACKGND),
                                scr);
                        //xcoord+=StringWidthN((char *)selend,(char *)strend,(UNIFONT *)halScreen.FontArray[FONT_CMDLINE]);
                        xcoord = scr->x;
                    }
                    if(tail) {
                        cgl_cliprect(scr, xcoord,
                                ytop + 2 +
                                k *
                                (*halScreen.FontArray[FONT_CMDLINE])->
                                BitmapHeight, xcoord + 3,
                                ytop + 2 + (k +
                                    1) *
                                (*halScreen.FontArray[FONT_CMDLINE])->
                                BitmapHeight - 1, cgl_mkcolor(PAL_CMDSELBACKGND));
                        xcoord += 3;
                    }
                }

                // CLEAR UP TO END OF LINE
                cgl_cliprect(scr, xcoord,
                        ytop + 2 +
                        k * (*halScreen.FontArray[FONT_CMDLINE])->BitmapHeight,
                        SCREEN_W - 1,
                        ytop + 2 + (k +
                            1) *
                        (*halScreen.FontArray[FONT_CMDLINE])->BitmapHeight - 1,
                        cgl_mkcolor(PAL_CMDBACKGND));
            }
        }

        if(halScreen.DirtyFlag & CMDLINE_LINEDIRTY) {
            // UPDATE THE CURRENT LINE
            BYTEPTR string = cmdline;
            BYTEPTR selst, selend;
            BYTEPTR strend = cmdline + nchars;
            BINT xcoord, tail;

            selst = selend = strend;
            tail = 0;
            if(halScreen.SelStartLine < halScreen.LineCurrent) {
                selst = string;
                tail = 1;
            }
            if(halScreen.SelStartLine == halScreen.LineCurrent) {
                selst = string + halScreen.SelStart;
                tail = 1;
            }
            if(halScreen.SelEndLine < halScreen.LineCurrent) {
                selend = string;
                tail = 0;
            }
            if(halScreen.SelEndLine == halScreen.LineCurrent) {
                selend = string + halScreen.SelEnd;
                tail = 0;
            }

            if(selend <= selst)
                selend = selst = string;

            // DRAW THE LINE SPLIT IN 3 SECTIONS: string TO selst, selst TO selend, selend TO strend
            xcoord = -halScreen.XVisible;
            if(selst > string) {
                DrawTextBkN(xcoord, ytop + 2 + y, (char *)string, (char *)selst,
                        *halScreen.FontArray[FONT_CMDLINE], cgl_mkcolor(PAL_CMDTEXT), cgl_mkcolor(PAL_CMDBACKGND), scr);
                //xcoord+=StringWidthN((char *)string,(char *)selst,(UNIFONT *)halScreen.FontArray[FONT_CMDLINE]);
                xcoord = scr->x;
            }
            if(selend > selst) {
                DrawTextBkN(xcoord, ytop + 2 + y, (char *)selst, (char *)selend,
                        *halScreen.FontArray[FONT_CMDLINE], cgl_mkcolor(PAL_CMDSELTEXT), cgl_mkcolor(PAL_CMDSELBACKGND), scr);
                //xcoord+=StringWidthN((char *)selst,(char *)selend,(UNIFONT *)halScreen.FontArray[FONT_CMDLINE]);
                xcoord = scr->x;
            }
            if(strend > selend) {
                DrawTextBkN(xcoord, ytop + 2 + y, (char *)selend,
                        (char *)strend, *halScreen.FontArray[FONT_CMDLINE], cgl_mkcolor(PAL_CMDTEXT),
                            cgl_mkcolor(PAL_CMDBACKGND), scr);
                //xcoord+=StringWidthN((char *)selend,(char *)strend,(UNIFONT *)halScreen.FontArray[FONT_CMDLINE]);
                xcoord = scr->x;
            }
            if(tail) {
                cgl_cliprect(scr, xcoord, ytop + 2 + y, xcoord + 3,
                        ytop + 2 + y +
                        (*halScreen.FontArray[FONT_CMDLINE])->BitmapHeight - 1,
                        cgl_mkcolor(PAL_CMDSELBACKGND));
                xcoord += 3;
            }

            // CLEAR UP TO END OF LINE
            cgl_cliprect(scr, xcoord, ytop + 2 + y, SCREEN_W - 1,
                    ytop + 2 + y +
                    (*halScreen.FontArray[FONT_CMDLINE])->BitmapHeight - 1, cgl_mkcolor(PAL_CMDBACKGND));

        }

        if(halScreen.DirtyFlag & CMDLINE_CURSORDIRTY) {
            // DRAW THE CURSOR
            if(!(halScreen.CursorState & 0x8000))
                DrawTextBkN(halScreen.CursorX - halScreen.XVisible,
                        ytop + 2 + y, (char *)&halScreen.CursorState,
                        ((char *)&halScreen.CursorState) + 1,
                        *halScreen.FontArray[FONT_CMDLINE], cgl_mkcolor(PAL_CMDCURSOR), cgl_mkcolor(PAL_CMDCURSORBACKGND), scr);

            else {
                scr->clipx = halScreen.CursorX - halScreen.XVisible;
                scr->clipx2 = scr->clipx + (*halScreen.FontArray[FONT_CMDLINE])->BitmapHeight+4;   // HARD CODED MAXIMUM WIDTH OF THE CURSOR
                if(scr->clipx2 >= SCREEN_WIDTH)
                    scr->clipx2 = SCREEN_WIDTH - 1;

                // REDRAW THE PORTION OF COMMAND LINE UNDER THE CURSOR
                if(!(halScreen.DirtyFlag & CMDLINE_LINEDIRTY)) {
                    // UPDATE THE CURRENT LINE
                    // UPDATE THE CURRENT LINE
                    BYTEPTR string = cmdline;
                    BYTEPTR selst, selend;
                    BYTEPTR strend = cmdline + nchars;
                    BINT xcoord, tail;

                    selst = selend = strend;
                    tail = 0;
                    if(halScreen.SelStartLine < halScreen.LineCurrent) {
                        selst = string;
                        tail = 1;
                    }
                    if(halScreen.SelStartLine == halScreen.LineCurrent) {
                        selst = string + halScreen.SelStart;
                        tail = 1;
                    }
                    if(halScreen.SelEndLine < halScreen.LineCurrent) {
                        selend = string;
                        tail = 0;
                    }
                    if(halScreen.SelEndLine == halScreen.LineCurrent) {
                        selend = string + halScreen.SelEnd;
                        tail = 0;
                    }

                    if(selend <= selst)
                        selend = selst = string;

                    // DRAW THE LINE SPLIT IN 3 SECTIONS: string TO selst, selst TO selend, selend TO strend
                    xcoord = -halScreen.XVisible;
                    if(selst > string) {
                        DrawTextBkN(xcoord, ytop + 2 + y, (char *)string,
                                (char *)selst,
                                *halScreen.FontArray[FONT_CMDLINE], cgl_mkcolor(PAL_CMDTEXT), cgl_mkcolor(PAL_CMDBACKGND),
                                scr);
                        //xcoord+=StringWidthN((char *)string,(char *)selst,(UNIFONT *)halScreen.FontArray[FONT_CMDLINE]);
                        xcoord = scr->x;
                    }
                    if(selend > selst) {
                        DrawTextBkN(xcoord, ytop + 2 + y, (char *)selst,
                                (char *)selend,
                                *halScreen.FontArray[FONT_CMDLINE], cgl_mkcolor(PAL_CMDSELTEXT), cgl_mkcolor(PAL_CMDSELBACKGND),
                                scr);
                        //xcoord+=StringWidthN((char *)selst,(char *)selend,(UNIFONT *)halScreen.FontArray[FONT_CMDLINE]);
                        xcoord = scr->x;
                    }
                    if(strend > selend) {
                        DrawTextBkN(xcoord, ytop + 2 + y, (char *)selend,
                                (char *)strend,
                                *halScreen.FontArray[FONT_CMDLINE], cgl_mkcolor(PAL_CMDTEXT), cgl_mkcolor(PAL_CMDBACKGND),
                                scr);
                        //xcoord+=StringWidthN((char *)selend,(char *)strend,(UNIFONT *)halScreen.FontArray[FONT_CMDLINE]);
                        xcoord = scr->x;
                    }

                    if(tail) {
                        cgl_cliprect(scr, xcoord, ytop + 2 + y, xcoord + 3,
                                ytop + 2 + y +
                                (*halScreen.FontArray[FONT_CMDLINE])->
                                BitmapHeight - 1, cgl_mkcolor(PAL_CMDSELBACKGND));
                        xcoord += 3;
                    }
                    // CLEAR UP TO END OF LINE
                    cgl_cliprect(scr, xcoord, ytop + 2 + y, SCREEN_W - 1,
                            ytop + 2 + y +
                            (*halScreen.FontArray[FONT_CMDLINE])->BitmapHeight -
                            1, cgl_mkcolor(PAL_CMDBACKGND));
                }

                // RESET THE CLIPPING RECTANGLE BACK TO WHOLE SCREEN
                scr->clipx = 0;
                scr->clipx2 = SCREEN_WIDTH - 1;
                scr->clipy = 0;
                scr->clipy2 = SCREEN_HEIGHT - 1;

            }
        }

    }

    halScreen.DirtyFlag &= ~CMDLINE_ALLDIRTY;

}

// GET NEW FONT DATA FROM THE RPL ENVIRONMENT
void halUpdateFonts()
{
    UNIFONT const **tmparray[FONTS_NUM];
    WORD hash;
    halUpdateFontArray((WORDPTR const **) tmparray);

    int f;
    for(f = 0; f < FONTS_NUM; ++f) {
        // COMPUTE THE HASH OF THE FONT
        hash = OPCODE((*tmparray[f])->Prolog) | (((*tmparray[f])->
                    BitmapHeight) << 20);
        if(halScreen.FontHash[f] != hash) {

            halScreen.FontArray[f] = tmparray[f];
            halScreen.FontHash[f] = hash;
            switch (f) {
            case FONT_STACK:
            case FONT_STACKLVL1:
                uiClearRenderCache();
                halScreen.DirtyFlag |= STACK_DIRTY;
                break;
            case FONT_CMDLINE:
                if(halScreen.CmdLine) {
                    halScreen.DirtyFlag |= CMDLINE_ALLDIRTY;
                    uiStretchCmdLine(0);
                }
                break;
            case FONT_MENU:
                if(halScreen.Menu1)
                    halSetMenu1Height(MENU1_HEIGHT);
                if(halScreen.Menu2)
                    halSetMenu2Height(MENU2_HEIGHT);
                halScreen.DirtyFlag |= MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY;
                break;
            case FONT_STATUS:
                halScreen.DirtyFlag |= MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY;
                break;
            case FONT_PLOT:
                halScreen.DirtyFlag |= FORM_DIRTY;
                break;
            case FONT_FORMS:
                halScreen.DirtyFlag |= FORM_DIRTY;
                break;
            }

        }
        else
            halScreen.FontArray[f] = tmparray[f];

    }

}

void halForceRedrawAll(DRAWSURFACE * scr)
{
    halUpdateFonts();
    halRedrawForm(scr);
    halRedrawStack(scr);
    halRedrawCmdLine(scr);
    halRedrawMenu1(scr);
    halRedrawMenu2(scr);
    halRedrawStatus(scr);
}

void halRedrawAll(DRAWSURFACE * scr)
{
    if(halScreen.DirtyFlag)
        halUpdateFonts();
    if(halScreen.DirtyFlag & FORM_DIRTY)
        halRedrawForm(scr);
    if(halScreen.DirtyFlag & STACK_DIRTY)
        halRedrawStack(scr);
    if(halScreen.DirtyFlag & CMDLINE_ALLDIRTY)
        halRedrawCmdLine(scr);
    if(halScreen.DirtyFlag & MENU1_DIRTY)
        halRedrawMenu1(scr);
    if(!halScreen.SAreaTimer) {
        // ONLY REDRAW IF THERE'S NO POPUP MESSAGES
        if(halScreen.DirtyFlag & MENU2_DIRTY)
            halRedrawMenu2(scr);
        if(halScreen.DirtyFlag & STAREA_DIRTY)
            halRedrawStatus(scr);
    }
}

// MARK STATUS AREA FOR IMMEDIATE UPDATE
void halUpdateStatus()
{
    halScreen.DirtyFlag |= STAREA_DIRTY;
}

void status_popup_handler()
{
    if(rplTestSystemFlag(FL_HIDEMENU2)) {
        halSetMenu2Height(0);
    }
    else {
        DRAWSURFACE scr;
        cgl_initscr(&scr);
        halRedrawMenu2(&scr);
        halRedrawStatus(&scr);
    }
    halScreen.SAreaTimer = 0;
}

// WILL KEEP THE STATUS AREA AS-IS FOR 5 SECONDS, THEN REDRAW IT
// TO CLEAN UP POP-UP MESSAGES
void halStatusAreaPopup()
{
    if(halScreen.SAreaTimer) {
        tmr_eventkill(halScreen.SAreaTimer);
        status_popup_handler();
        //tmr_eventpause(halScreen.SAreaTimer);
        //tmr_eventresume(halScreen.SAreaTimer);      // PAUSE/RESUME WILL RESTART THE 5 SECOND COUNT
        //return;
    }
    halScreen.SAreaTimer = tmr_eventcreate(&status_popup_handler, 3000, 0);
}

void halCancelPopup()
{
    if(halScreen.SAreaTimer) {
        tmr_eventkill(halScreen.SAreaTimer);
        // MARK DIRTY BUT DON'T REDRAW YET
        halScreen.DirtyFlag |= STAREA_DIRTY | MENU2_DIRTY;
        halScreen.SAreaTimer = 0;
    }
    if(rplTestSystemFlag(FL_HIDEMENU2))
        halSetMenu2Height(0);

}

// WILL KEEP THE STATUS AREA AS-IS FOR 5 SECONDS, THEN REDRAW IT
// TO CLEAN UP POP-UP MESSAGES
void halErrorPopup()
{
    if(halScreen.SAreaTimer) {
        tmr_eventkill(halScreen.SAreaTimer);
        status_popup_handler();
        //tmr_eventpause(halScreen.SAreaTimer);
        //tmr_eventresume(halScreen.SAreaTimer);      // PAUSE/RESUME WILL RESTART THE 3 SECOND COUNT
        //return;
    }
    halScreen.SAreaTimer = tmr_eventcreate(&status_popup_handler, 3000, 0);
}

// DECOMPILE THE OPCODE NAME IF POSSIBLE
const WORD const text_editor_string[] = {
    MAKESTRING(12),
    TEXT2WORD('C', 'o', 'm', 'm'),
    TEXT2WORD('a', 'n', 'd', ' '),
    TEXT2WORD('L', 'i', 'n', 'e')
};

WORDPTR halGetCommandName(WORDPTR NameObject)
{
    WORD Opcode = NameObject ? *NameObject : 0;

    if(Opcode == 0)
        return (WORDPTR) text_editor_string;
    if(ISSYMBOLIC(Opcode)) {
        WORDPTR OpObject = rplSymbMainOperatorPTR(NameObject);
        if(OpObject) {
            Opcode = *OpObject;
            NameObject = OpObject;
        }
    }

    if(ISPROLOG(Opcode)) {
        // ONLY ACCEPT IDENTS AND STRINGS AS COMMAND NAMES
        if(ISSTRING(Opcode))
            return NameObject;
        if(!ISIDENT(Opcode))
            return 0;
    }

    BINT SavedException = Exceptions;
    BINT SavedErrorCode = ErrorCode;

    Exceptions = 0;     // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
    WORDPTR opname = rplDecompile(NameObject, DECOMP_NOHINTS);
    Exceptions = SavedException;
    ErrorCode = SavedErrorCode;

    return opname;
}

// DISPLAY AN ERROR BOX FOR 5 SECONDS WITH AN ERROR MESSAGE
// USES ERROR CODE FROM SYSTEM Exceptions
void halShowErrorMsg()
{
    int errbit;
    if(!Exceptions)
        return;

    halErrorPopup();

    DRAWSURFACE scr;
    cgl_initscr(&scr);

    if(!halScreen.Menu2) {
        // SHOW THE SECOND MENU TO DISPLAY THE MESSAGE
        halSetMenu2Height(MENU2_HEIGHT);
        halRedrawAll(&scr);
    }

    BINT ytop =
            halScreen.Form + halScreen.Stack + halScreen.CmdLine +1;
    BINT ybot = ytop + halScreen.Menu1 + halScreen.Menu2 - 1;

    // CLEAR MENU2 AND STATUS AREA
    cgl_cliprect(&scr, 0, ytop, SCREEN_WIDTH - 1, ybot,
            cgl_mkcolor(PAL_HLPBACKGND));
    // DO SOME DECORATIVE ELEMENTS
    cgl_cliphline(&scr,
            ytop + (*halScreen.FontArray[FONT_HLPTITLE])->BitmapHeight + 1, 0,
            SCREEN_WIDTH - 1, cgl_mkcolor(PAL_HLPLINES));
    //ggl_cliphline(&scr,ytop+halScreen.Menu2-1,0,SCREEN_WIDTH-1,ggl_mkcolor(8));
    cgl_cliprect(&scr, 0, ytop, 4, ybot , cgl_mkcolor(PAL_HLPLINES));

    scr.clipx = 1;
    scr.clipx2 = SCREEN_WIDTH - 2;
    scr.clipy = ytop;
    scr.clipy2 = ybot - 1;
    // SHOW ERROR MESSAGE

    if(Exceptions != EX_ERRORCODE) {
        BINT xstart = scr.clipx + 6;
        if(ExceptionPointer != 0)       // ONLY IF THERE'S A VALID COMMAND TO BLAME
        {
            WORDPTR cmdname = halGetCommandName(ExceptionPointer);
            if(cmdname) {
                BYTEPTR start = (BYTEPTR) (cmdname + 1);
                BYTEPTR end = start + rplStrSize(cmdname);

                xstart +=
                        StringWidthN((char *)start, (char *)end,
                        *halScreen.FontArray[FONT_HLPTITLE]);
                DrawTextN(scr.clipx + 6, scr.clipy + 1, (char *)start,
                        (char *)end, *halScreen.FontArray[FONT_HLPTITLE], cgl_mkcolor(PAL_HLPTEXT),
                        &scr);
                xstart += 4;
            }
        }
        DrawText(xstart, scr.clipy + 1, "Exception:",
                *halScreen.FontArray[FONT_HLPTITLE], cgl_mkcolor(PAL_HLPTEXT), &scr);

        BINT ecode;
        for(errbit = 0; errbit < 8; ++errbit)   // THERE'S ONLY A FEW EXCEPTIONS IN THE NEW ERROR MODEL
        {
            if(Exceptions & (1 << errbit)) {
                ecode = MAKEMSG(0, errbit);
                WORDPTR message = uiGetLibMsg(ecode);
                if(!message)
                    message = uiGetLibMsg(ERR_UNKNOWNEXCEPTION);
                if(message) {
                    BYTEPTR msgstart = (BYTEPTR) (message + 1);
                    BYTEPTR msgend = msgstart + rplStrSize(message);

                    DrawTextN(scr.clipx + 6,
                            scr.clipy + 3 +
                            (*halScreen.FontArray[FONT_HLPTEXT])->BitmapHeight,
                            (char *)msgstart, (char *)msgend,
                            *halScreen.FontArray[FONT_HLPTEXT], cgl_mkcolor(PAL_HLPTEXT), &scr);
                }
                break;
            }
        }
    }
    else {
        // TRY TO DECOMPILE THE OPCODE THAT CAUSED THE ERROR
        BINT xstart = scr.clipx + 6;
        if(ExceptionPointer != 0)       // ONLY IF THERE'S A VALID COMMAND TO BLAME
        {
            WORDPTR cmdname = halGetCommandName(ExceptionPointer);
            if(cmdname) {
                BYTEPTR start = (BYTEPTR) (cmdname + 1);
                BYTEPTR end = start + rplStrSize(cmdname);

                xstart +=
                        StringWidthN((char *)start, (char *)end,
                        *halScreen.FontArray[FONT_HLPTITLE]);
                DrawTextN(scr.clipx + 6, scr.clipy + 1, (char *)start,
                        (char *)end, *halScreen.FontArray[FONT_HLPTITLE], cgl_mkcolor(PAL_HLPTEXT),
                        &scr);
                xstart += 4;
            }
        }
        DrawText(xstart, scr.clipy + 1, "Error:",
                *halScreen.FontArray[FONT_HLPTITLE], cgl_mkcolor(PAL_HLPTEXT), &scr);
        // GET NEW TRANSLATABLE MESSAGES

        WORDPTR message = uiGetLibMsg(ErrorCode);
        if(!message)
            message = uiGetLibMsg(ERR_UNKNOWNEXCEPTION);
        if(message) {
            BYTEPTR msgstart = (BYTEPTR) (message + 1);
            BYTEPTR msgend = msgstart + rplStrSize(message);

            DrawTextN(scr.clipx + 6,
                    scr.clipy + 3 +
                    (*halScreen.FontArray[FONT_HLPTITLE])->BitmapHeight,
                    (char *)msgstart, (char *)msgend,
                    *halScreen.FontArray[FONT_HLPTEXT], cgl_mkcolor(PAL_HLPTEXT), &scr);
        }

    }

}

void halShowMsgN(char *Text, char *End)
{

    halErrorPopup();

    DRAWSURFACE scr;
    cgl_initscr(&scr);

    if(!halScreen.Menu2) {
        // SHOW THE SECOND MENU TO DISPLAY THE MESSAGE
        halSetMenu2Height(MENU2_HEIGHT);
        halRedrawAll(&scr);
    }

    BINT ytop =
            halScreen.Form + halScreen.Stack + halScreen.CmdLine +
            1;
    BINT ybot = ytop + halScreen.Menu1 + halScreen.Menu2 - 1;

    // CLEAR MENU2 AND STATUS AREA
    cgl_cliprect(&scr, 0, ytop, SCREEN_WIDTH - 1, ybot,
            cgl_mkcolor(PAL_HLPBACKGND));
    // DO SOME DECORATIVE ELEMENTS
    cgl_cliphline(&scr, ytop + 1, 1, SCREEN_WIDTH - 2, cgl_mkcolor(PAL_HLPLINES));
    cgl_cliphline(&scr, ybot, 1, SCREEN_WIDTH - 2,
            cgl_mkcolor(PAL_HLPLINES));
    cgl_clipvline(&scr, 1, ytop + 2, ybot - 1,
            cgl_mkcolor(PAL_HLPLINES));
    cgl_clipvline(&scr, SCREEN_WIDTH - 2, ytop + 2, ybot - 1,
            cgl_mkcolor(PAL_HLPLINES));

    // SHOW MESSAGE

    DrawTextN(3, ytop + 3, Text, End, *halScreen.FontArray[FONT_HLPTEXT], cgl_mkcolor(PAL_HLPTEXT),
            &scr);

}

void halShowMsg(char *Text)
{
    char *End = Text;
    while(*End)
        ++End;

    halShowMsgN(Text, End);
}

// CHANGE THE CONTEXT AND DISPLAY THE CURRENT FORM
void halSwitch2Form()
{

    if(halGetContext() & CONTEXT_INEDITOR) {
        // CLOSE THE EDITOR FIRST
        uiCloseCmdLine();
        halSetCmdLineHeight(0);
    }

    halSetContext(CONTEXT_FORM);

    // ENLARGE THE FORM
    halSetFormHeight(halScreen.Stack);
    // AND ELIMINATE THE STACK
    halSetStackHeight(0);

    //uiFormEnterEvent();

    halScreen.DirtyFlag |=
            STACK_DIRTY | FORM_DIRTY | STAREA_DIRTY | MENU1_DIRTY | MENU2_DIRTY;
}

// CHANGE THE CONTEXT AND DISPLAY THE STACK
void halSwitch2Stack()
{
    if(halGetContext() & CONTEXT_INEDITOR) {
        // CLOSE THE EDITOR FIRST
        uiCloseCmdLine();
        halSetCmdLineHeight(0);
    }

    halSetContext(CONTEXT_STACK);

    // ENLARGE THE STACK
    halSetStackHeight(halScreen.Form);
    // AND ELIMINATE THE FORM
    halSetFormHeight(0);

    //uiFormExitEvent();

    uiClearRenderCache();

    halScreen.DirtyFlag |=
            STACK_DIRTY | FORM_DIRTY | STAREA_DIRTY | MENU1_DIRTY | MENU2_DIRTY;

}
