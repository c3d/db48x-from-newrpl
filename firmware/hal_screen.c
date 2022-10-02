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

RECORDER(annunciators, 16, "Annunciators");

// THIS IS THE MAIN STABLE API TO ACCESS THE SCREEN

// SET TO SHOW/HIDE THE NOTIFICATION ICON

void halSetNotification(enum halNotification type, unsigned color)
{
    int old = halFlags & (1 << (16 + type));
    if (color)
        halFlags |= 1 << (16 + type);
    else
        halFlags &= ~(1 << (16 + type));

#if defined(ANN_X_COORD) && defined(ANN_Y_COORD)
    if (type < N_DATARECVD)
    {
        byte_p scrptr = (byte_p) MEM_PHYS_SCREEN;
        scrptr += ANN_X_COORD / (PIXELS_PER_WORD / 4);
        scrptr += type * (LCD_SCANLINE / (PIXELS_PER_WORD / 4));
        *scrptr =
            (*scrptr &
             ~(((1 << BITSPERPIXEL) - 1)
               << (BITSPERPIXEL * (ANN_X_COORD % (PIXELS_PER_WORD / 4))))) |
            (color << (BITSPERPIXEL * (ANN_X_COORD % (PIXELS_PER_WORD / 4))));
        return;
    }
#endif /* has physical annunciators */

    // DRAW CUSTOM ICONS INTO THE STATUS AREA FOR ALL OTHER ANNUNCIATORS
    if ((halFlags ^ old) & (1 << (16 + type)))
        halScreen.DirtyFlag |= STAREA_DIRTY; // REDRAW STATUS AS SOON AS POSSIBLE
}

unsigned halGetNotification(enum halNotification type)
{
    if (halFlags & (1 << (16 + type)))
        return 1;
    return 0;
}

void halSetStackHeight(int h)
{
    int total;
    halScreen.Stack = h;
    total           = halScreen.Form + halScreen.Stack + halScreen.CmdLine + halScreen.Menu1 + halScreen.Menu2;
    if (total != LCD_H)
    {
        if (halScreen.Form)
        {
            halScreen.Form += LCD_H - total;
            halScreen.DirtyFlag |= FORM_DIRTY;
        }

        else
            halScreen.Stack = LCD_H - halScreen.CmdLine - halScreen.Menu1 - halScreen.Menu2;
        if (halScreen.Form < 0)
        {
            halScreen.Stack += halScreen.Form;
            halScreen.Form = 0;
        }
    }
    halScreen.DirtyFlag |= STACK_DIRTY;
}

void halSetFormHeight(int h)
{
    int total;
    if (h < 0)
        h = 0;
    halScreen.Form = h;
    total          = halScreen.Form + halScreen.Stack + halScreen.CmdLine + halScreen.Menu1 + halScreen.Menu2;
    if (total != LCD_H)
    {
        if (halScreen.Stack)
        {
            halScreen.Stack += LCD_H - total;
            halScreen.DirtyFlag |= STACK_DIRTY;
        }

        else
            halScreen.Form = LCD_H - halScreen.CmdLine - halScreen.Menu1 - halScreen.Menu2;
        if (halScreen.Stack < 0)
        {
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
    if (h < 0)
        h = 0;
    halScreen.Menu1 = h;
    total           = halScreen.Form + halScreen.Stack + halScreen.CmdLine + halScreen.Menu1 + halScreen.Menu2;
    while (total != LCD_H)
    {
        // STRETCH THE STACK FIRST (IF ACTIVE), THEN FORM

        if (halScreen.Stack)
        {
            halScreen.Stack += LCD_H - total;
            halScreen.DirtyFlag |= STACK_DIRTY;

            if (halScreen.Stack < 0)
                halScreen.Stack = 0;
        }
        else
        {
            halScreen.Form += LCD_H - total;
            halScreen.DirtyFlag |= FORM_DIRTY;

            if (halScreen.Form < 0)
            {
                halScreen.Menu1 += halScreen.Form;
                halScreen.Form = 0;
            }
        }
        total = halScreen.Form + halScreen.Stack + halScreen.CmdLine + halScreen.Menu1 + halScreen.Menu2;
    }
    halScreen.DirtyFlag |= MENU1_DIRTY | CMDLINE_ALLDIRTY;
}

void halSetMenu2Height(int h)
{
    int total;
    if (h < 0)
        h = 0;
    halScreen.Menu2 = h;
    total           = halScreen.Form + halScreen.Stack + halScreen.CmdLine + halScreen.Menu1 + halScreen.Menu2;
    while (total != LCD_H)
    {
        // STRETCH THE STACK FIRST (IF ACTIVE), THEN FORM

        if (halScreen.Stack > 1)
        {
            halScreen.Stack += LCD_H - total;
            halScreen.DirtyFlag |= STACK_DIRTY;
            if (halScreen.Stack < 1)
                halScreen.Stack = 1;
        }
        else
        {
            if (halScreen.Form > 1)
            {
                halScreen.Form += LCD_H - total;
                halScreen.DirtyFlag |= FORM_DIRTY;
                if (halScreen.Form < 1)
                    halScreen.Form = 1;
            }
            else
            {
                if (halScreen.CmdLine > 1)
                {
                    int newcmdht  = halScreen.CmdLine + LCD_H - total;
                    int newnlines = (newcmdht - 2) / FONT_HEIGHT(FONT_CMDLINE);
                    if (newnlines < 1)
                    {
                        // THERE'S NO ROOM AT ALL, VANISH THE MENU REGARDLESS
                        halScreen.Menu2 = 0;
                    }
                    else
                    {
                        if (newnlines != halScreen.NumLinesVisible)
                        {
                            // WE ARE CHANGING THE COMMAND LINE HEIGHT
                            uiStretchCmdLine(newnlines - halScreen.NumLinesVisible);
                            uiEnsureCursorVisible();
                        }
                        halScreen.DirtyFlag |= CMDLINE_ALLDIRTY;
                    }
                }
            }
        }
        total = halScreen.Form + halScreen.Stack + halScreen.CmdLine + halScreen.Menu1 + halScreen.Menu2;
    }
    halScreen.DirtyFlag |= MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY | CMDLINE_ALLDIRTY;
}

void halSetCmdLineHeight(int h)
{
    int total;
    if (h < 0)
        h = 0;
    halScreen.CmdLine = h;
    total             = halScreen.Form + halScreen.Stack + halScreen.CmdLine + halScreen.Menu1 + halScreen.Menu2;
    while (total != LCD_H)
    {
        // STRETCH THE STACK FIRST (IF ACTIVE), THEN FORM

        if (halScreen.Stack > 1)
        {
            halScreen.Stack += LCD_H - total;
            halScreen.DirtyFlag |= STACK_DIRTY;

            if (halScreen.Stack < 1)
                halScreen.Stack = 1;
        }
        else
        {
            if (halScreen.Form > 1)
            {
                halScreen.Form += LCD_H - total;
                halScreen.DirtyFlag |= FORM_DIRTY;

                if (halScreen.Form < 1)
                    halScreen.Form = 1;
            }
            else
            {
                // STACK AND FORMS ARE AT MINIMUM
                if (total > LCD_H)
                {
                    unsigned height = FONT_HEIGHT(FONT_CMDLINE);
                    halScreen.CmdLine =
                        LCD_H - 2 - (halScreen.Form + halScreen.Stack + halScreen.Menu1 + halScreen.Menu2);
                    halScreen.CmdLine /= height;
                    if (halScreen.CmdLine < 1)
                        halScreen.CmdLine = 1;
                    halScreen.CmdLine *= height;
                    halScreen.CmdLine += 2;
                }
                else
                {
                    // ENLARGE STACK
                    if (halScreen.Stack > 0)
                    {
                        halScreen.Stack += LCD_H - total;
                        halScreen.DirtyFlag |= STACK_DIRTY;
                    }
                    else
                    {
                        // IF THE STACK IS CLOSED, THEN IT HAS TO BE A FORM!

                        halScreen.Form += LCD_H - total;
                        halScreen.DirtyFlag |= FORM_DIRTY;
                    }
                }
            }
        }
        total = halScreen.Form + halScreen.Stack + halScreen.CmdLine + halScreen.Menu1 + halScreen.Menu2;
    }
    halScreen.DirtyFlag |= CMDLINE_ALLDIRTY;
}

// COMPUTE HEIGHT AND WIDTH OF OBJECT TO DISPLAY ON STACK

int32_t halGetDispObjectHeight(word_p object, UNIFONT *font)
{
    UNUSED(object);
    // TODO: ADD MULTILINE OBJECTS, ETC.

    return font->BitmapHeight;
}

extern const const uint64_t powersof10[20];

// CONVERT INTEGER NUMBER INTO STRING FOR STACK LEVEL
// str MUST CONTAIN AT LEAST 15: BYTES "-1,345,789,123[NULL]"
void                       halInt2String(int num, char *str)
{
    int   pow10idx = 9; // START WITH 10^10
    int   digit, firstdigit;
    char *ptr = str;
    if (num < 0)
    {
        *ptr++ = '-';
        num    = -num;
    }

    firstdigit = 1;
    do
    {
        digit = 0;
        while ((uint64_t) num >= powersof10[pow10idx])
        {
            ++digit;
            num -= powersof10[pow10idx];
        }
        if (!((digit == 0) && firstdigit))
        {
            *ptr++     = digit + '0';
            firstdigit = 0;
        }
        ++pow10idx;
    } while (num != 0);

    if (firstdigit)
        *ptr++ = '0';
    else
    {
        while (pow10idx < 19)
        {
            *ptr++ = '0';
            ++pow10idx;
        }
    }
    *ptr = 0;
}

void halRedrawForm(gglsurface *scr)
{
    if (halScreen.Form == 0)
    {
        halScreen.DirtyFlag &= ~FORM_DIRTY;
        return;
    }

    halScreenUpdated();

    // REDRAW THE CONTENTS OF THE CURRENT FORM
    coord ystart = 0;
    coord yend = ystart + halScreen.Form;

    word_p form = rplGetSettings((word_p) currentform_ident);
    if (!form)
    {
        ggl_cliprect(scr, scr->left, ystart, scr->right, yend - 1, PAL_FORM_BG); // CLEAR RECTANGLE
        halScreen.DirtyFlag &= ~FORM_DIRTY;
        return;
    }

    word_p bmp = uiFindCacheEntry(form, FONT_FORMS);
    if (!bmp)
    {
        ggl_cliprect(scr, scr->left, ystart, scr->right, yend - 1, PAL_FORM_BG); // CLEAR RECTANGLE
        halScreen.DirtyFlag &= ~FORM_DIRTY;
        return;
    }

    gglsurface viewport = ggl_grob(bmp);
    if (yend > viewport.bottom + 1)
    {
        // CLEAR THE BACKGROUND
        ggl_cliprect(scr,
                     scr->left,
                     viewport.bottom + 1,
                     scr->right,
                     yend - 1,
                     PAL_FORM_BG); // CLEAR RECTANGLE
    }

    // DRAW THE VIEWPORT
    ggl_copy(scr, &viewport, LCD_W, yend);
    halScreen.DirtyFlag &= ~FORM_DIRTY;
}

void halRedrawStack(gglsurface *screen)
{
    if (halScreen.Stack == 0)
    {
        halScreen.DirtyFlag &= ~STACK_DIRTY;
        return;
    }

    halScreenUpdated();

    gglsurface     clipped = *screen;
    coord          ystart  = halScreen.Form;
    coord          yend    = ystart + halScreen.Stack;
    int            depth   = rplDepthData();
    int            level   = 1;
    int            objheight, ytop, y, numwidth, xright;
    size           width, height;
    char           num[16];
    const UNIFONT *levelfnt;
    word_p         object;

    // Estimate number width at 75% of the font height
    size stknum_w  = (FONT_HEIGHT(FONT_STACK) * 192) / 256;
    if (halScreen.KeyContext & CONTEXT_INTSTACK)
    {
        // Ensure the stack pointer is completely inside the screen
        if (halScreen.StkVisibleLvl < 0)
        {
            // Need to recompute this
            int k = halScreen.StkPointer;
            if (k < 1)
                k = 1;
            if (k > depth)
                k = depth;

            levelfnt = k == 1 ? FONT_STACKLVL1 : FONT_STACK;
            object   = uiRenderObject(rplPeekData(k), levelfnt);

            // Get the size of the object
            size objh = object ? object[2] : levelfnt->BitmapHeight;
            coord ypref = ystart + (yend - ystart) / 4 + objh / 2;
            if (ypref > yend)
                ypref = yend - objh;
            if (ypref < ystart)
                ypref = ystart;

            for (; k > 0; --k)
            {
                levelfnt = k == 1 ? FONT_STACKLVL1 : FONT_STACK;
                object   = uiRenderObject(rplPeekData(k), levelfnt);

                // Get the size of the object
                int stkheight = object ? object[2] : levelfnt->BitmapHeight;
                if (ypref + stkheight > yend)
                {
                    y                          = ypref + stkheight;
                    halScreen.StkVisibleLvl    = k;
                    halScreen.StkVisibleOffset = yend - y;
                    break;
                }
            }
            if (!k)
            {
                halScreen.StkVisibleLvl    = 1;
                halScreen.StkVisibleOffset = 0;
            }
        }
        xright = 2 * stknum_w;
    }
    else
    {
        xright = stknum_w;
    }

    level = halScreen.StkVisibleLvl;
    y     = yend - halScreen.StkVisibleOffset;

    for (int mul = 10; depth >= mul; mul *= 10)
        xright += stknum_w;

    // Clear the stack index area, the stack display area, and draw
    ggl_cliprect(&clipped, 0, ystart, xright - 1, yend - 1, PAL_STK_IDX_BG);
    ggl_cliprect(&clipped, xright + 1, ystart, LCD_W - 1, yend - 1, PAL_STK_BG);
    ggl_clipvline(&clipped, xright, ystart, yend - 1, PAL_STK_VLINE);

    while (y > ystart)
    {
        levelfnt = level == 1 ? FONT_STACKLVL1 : FONT_STACK;

        // Get Object size
        if (level <= depth)
        {
            // Draw the object
            object = uiRenderObject(rplPeekData(level), levelfnt);

            // Get the size of the object
            if (!object)
            {
                // Draw directly, don't cache something we couldn't render
                word_p  string = (word_p) invalid_string;

                // Now size the string object
                int32_t len    = rplStrSize(string);
                utf8_p  str    = (utf8_p) (string + 1);
                width          = StringWidthN(str, str + len, levelfnt);
                height         = levelfnt->BitmapHeight;
            }
            else
            {
                width  = (size) object[1];
                height = (size) object[2];
            }

            // Maximum height for a stack item is 4 lines, after that clip it
            objheight = min(height, 4 * levelfnt->BitmapHeight);
        }
        else
        {
            object    = 0;
            objheight = levelfnt->BitmapHeight;
            width     = 0;
        }

        ytop           = y - objheight;

        // Set clipping region
        clipped.left   = 0;
        clipped.right  = LCD_W - 1;
        clipped.top    = (ytop < 0) ? 0 : ytop;
        clipped.bottom = (y > yend) ? yend - 1 : y - 1;

        if (halScreen.KeyContext & CONTEXT_INTSTACK)
        {
            // Highlight selected items
            switch (halScreen.StkSelStatus)
            {
            default:
            case 0:
                // Nothing selected yet
                break;
            case 1:
                // Start was selected, paint all levels between start and current position
                if (halScreen.StkSelStart > halScreen.StkPointer)
                {
                    if ((level >= halScreen.StkPointer) && (level <= halScreen.StkSelStart))
                        ggl_cliprect(&clipped, 0, ytop, xright - 1, y - 1, PAL_STK_SEL_BG);
                    if (level == halScreen.StkSelStart)
                        DrawText(&clipped, 2, ytop, "▶", FONT_STACK, PAL_STK_CURSOR);
                }
                else
                {
                    if ((level >= halScreen.StkSelStart) && (level <= halScreen.StkPointer))
                        ggl_cliprect(&clipped, 0, ytop, xright - 1, y - 1, PAL_STK_SEL_BG);
                    if (level == halScreen.StkSelStart)
                        DrawText(&clipped, 2, ytop, "▶", FONT_STACK, PAL_STK_CURSOR);
                }
                break;
            case 2:
                // Both start and end selected
                if ((level >= halScreen.StkSelStart) && (level <= halScreen.StkSelEnd))
                    ggl_cliprect(&clipped, 0, ytop, xright - 1, y - 1, PAL_STK_SEL_BG);
                if (level == halScreen.StkSelStart)
                    DrawText(&clipped, 2, ytop, "▶", FONT_STACK, PAL_STK_CURSOR);
                if (level == halScreen.StkSelEnd)
                    DrawText(&clipped, 2, ytop, "▶", FONT_STACK, PAL_STK_CURSOR);
                break;
            }

            // Draw the pointer
            if ((level <= depth) && (level == halScreen.StkPointer))
                DrawText(&clipped, 0, ytop, "▶", FONT_STACK, PAL_STK_CURSOR);
            else if ((level == 1) && (halScreen.StkPointer == 0))
                DrawText(&clipped, 0, ytop + levelfnt->BitmapHeight / 2, "▶", FONT_STACK, PAL_STK_CURSOR);
            else if ((level == depth) && (halScreen.StkPointer > depth))
                DrawText(&clipped, 0, ytop - levelfnt->BitmapHeight / 2 + 1, "▶", FONT_STACK, PAL_STK_CURSOR);
        }

        if (level <= depth)
        {
            // Draw the stack level number
            halInt2String(level, num);
            numwidth = StringWidth(num, FONT_STACK);
            DrawText(&clipped, xright - numwidth, ytop, num, FONT_STACK, PAL_STK_INDEX);

            // Do proper layout: right justify unless it does not fit
            coord x = LCD_W - width;
            if (x < xright + 1)
                x = xright + 1;

            // Display the item
            clipped.left = xright + 1;
            uiDrawBitmap(&clipped, x, ytop, object);
        }

        y = ytop;
        ++level;
    }

    halScreen.DirtyFlag &= ~STACK_DIRTY;
}

#define MABS(a) (((a) < 0) ? -(a) : (a))

// FIND THE CLOSEST MATCHING SYSTEM FONT FOR A GIVEN HEIGHT
// DOES NOT SEARCH USER INSTALLED FONTS
UNIFONT const *halGetSystemFontbyHeight(int height)
{
    int                  k;
    int                  howclose = height;
    const UNIFONT       *ptr;
    const UNIFONT       *selection = NULL;
    const const word_p  *romtable  = rplGetFontRomPtrTableAddress();

    for (k = START_ROMPTR_INDEX; k < ROMLIB_MAX_SIZE; k += 2)
    {
        ptr = (const UNIFONT *) romtable[k + 1];
        if (ptr == NULL)
            break;
        if (MABS(ptr->BitmapHeight - height) < howclose)
        {
            howclose  = MABS(ptr->BitmapHeight - height);
            selection = ptr;
        }
        if (!howclose)
            break;
    }

    return selection;
}

// UPDATE AN ARRAY WITH ALL FONTS_NUM FONT POINTERS
// NEEDS TO BE FAST
void halUpdateFontArray(const UNIFONT *fontarray[])
{
    // SET ALL POINTERS TO 0

    for (int i = 0; i < FONTS_NUM; ++i)
        fontarray[i] = NULL;

    // SET CONFIGURED FONTS
    if (!ISDIR(*SettingsDir))
        return;

    word_p       *var;
    const word_p *romtable  = rplGetFontRomPtrTableAddress();
    var                     = rplFindFirstByHandle(SettingsDir);

    while (var)
    {
        unsigned index;
        for (index = 0; index < FONTS_NUM; index++)
        {
            if (rplCompareIDENT(var[0], romtable[FONT_IDENTS_ROMPTR_INDEX + index]))
            {
                const UNIFONT *font = (const UNIFONT *) var[1];
                if (ISFONT(font->Prolog))
                    fontarray[index] = font;
            }
        }
        var = rplFindNext(var);
    }

    // UNCONFIGURED FONTS GET DEFAULTS
    static const unsigned defaultHeight[FONTS_NUM] = {
        DEF_FNT_STK_HEIGHT,  DEF_FNT_1STK_HEIGHT, DEF_FNT_CMDL_HEIGHT,
        DEF_FNT_CURS_HEIGHT, DEF_FNT_MENU_HEIGHT, DEF_FNT_STAT_HEIGHT,
        DEF_FNT_PLOT_HEIGHT, DEF_FNT_FORM_HEIGHT, DEF_FNT_HLPT_HEIGHT,
        DEF_FNT_HELP_HEIGHT,
    };

    unsigned              index;
    for (index = 0; index < FONTS_NUM; index++)
        if (fontarray[index] == 0)
            fontarray[index] = halGetSystemFontbyHeight(defaultHeight[index]);
    return;
}


// ============================================================================
//
//    Default Palette values
//
// ============================================================================

// Stick to 16 grays mode for the first 15 colors for default UI elements
#define THEME_GRAY0               255, 255, 255
#define THEME_GRAY1               238, 238, 238
#define THEME_GRAY2               221, 221, 221
#define THEME_GRAY3               204, 204, 204
#define THEME_GRAY4               187, 187, 187
#define THEME_GRAY5               170, 170, 170
#define THEME_GRAY6               153, 153, 153
#define THEME_GRAY7               136, 136, 136
#define THEME_GRAY8               119, 119, 119
#define THEME_GRAY9               102, 102, 102
#define THEME_GRAY10              85, 85, 85
#define THEME_GRAY11              68, 68, 68
#define THEME_GRAY12              51, 51, 51
#define THEME_GRAY13              34, 34, 34
#define THEME_GRAY14              17, 17, 17
#define THEME_GRAY15              0, 0, 0

// Include the target-specific palette
#include <theme.h>


static struct rgb { uint8_t red, green, blue; } defaultTheme[PALETTE_SIZE] =
{
    { THEME_GRAY0 },
    { THEME_GRAY1 },
    { THEME_GRAY2 },
    { THEME_GRAY3 },
    { THEME_GRAY4 },
    { THEME_GRAY5 },
    { THEME_GRAY6 },
    { THEME_GRAY7 },
    { THEME_GRAY8 },
    { THEME_GRAY9 },
    { THEME_GRAY10 },
    { THEME_GRAY11 },
    { THEME_GRAY12 },
    { THEME_GRAY13 },
    { THEME_GRAY14 },
    { THEME_GRAY15 },

    // Theme colors for stack
    { THEME_STK_BG },
    { THEME_STK_INDEX },
    { THEME_STK_VLINE },
    { THEME_STK_IDX_BG },
    { THEME_STK_ITEMS },
    { THEME_STK_SEL_BG },
    { THEME_STK_SEL_ITEM },
    { THEME_STK_CURSOR },

    // Theme colors for command line
    { THEME_CMD_BG },
    { THEME_CMD_TEXT },
    { THEME_CMD_SEL_BG },
    { THEME_CMD_SELTEXT },
    { THEME_CMD_CURSOR_BG },
    { THEME_CMD_CURSOR },
    { THEME_DIV_LINE },

    // Theme colors for menu
    { THEME_MENU_BG },
    { THEME_MENU_UNUSED_1 },
    { THEME_MENU_TEXT },
    { THEME_MENU_DIR_MARK },
    { THEME_MENU_DIR },
    { THEME_MENU_DIR_BG },
    { THEME_MENU_HLINE },
    { THEME_MENU_FOCUS_HLINE },
    { THEME_MENU_PRESS_BG },
    { THEME_MENU_UNUSED_2 },

    // Theme colors for status area
    { THEME_STA_BG },
    { THEME_STA_TEXT },
    { THEME_STA_ANNPRESS },
    { THEME_STA_ANN },
    { THEME_STA_BAT },
    { THEME_STA_UFLAG0 },
    { THEME_STA_UFLAG1 },

    // Theme colors for help and popup messages
    { THEME_HLP_BG },
    { THEME_HLP_TEXT },
    { THEME_HLP_LINES },
};


void halSetupTheme(color16_t *palette)
// ----------------------------------------------------------------------------
//   Change the system palette to a new theme, or default theme if NULL
// ----------------------------------------------------------------------------
//   If 'palette' is NULL, use the default palette
{
    if (!palette)
        for (int k = 0; k < PALETTE_SIZE; k++)
            ggl_palette_set(k,
                            defaultTheme[k].red,
                            defaultTheme[k].green,
                            defaultTheme[k].blue);
    else
        for (int k = 0; k < PALETTE_SIZE; k++)
            ggl_palette_set(k,
                            ggl_red(palette[k]),
                            ggl_green(palette[k]),
                            ggl_blue(palette[k]));

    // Make sure all items are rendered again using the new colors
    uiClearRenderCache();
}


void halInitScreen()
// ----------------------------------------------------------------------------
// Initialize default screen parameters
// ----------------------------------------------------------------------------
{
    // Restore the screen
    word_p saved = rplGetSettings((word_p) screenconfig_ident);
    if (saved)
    {
        // JUST THE CONTRAST SETTINGS FOR NOW
        if (ISint32_t(*saved))
            lcd_setcontrast(rplReadint32_t(saved));
    }

    halSetupTheme(NULL); // SETUP DEFAULT THEME

    halUpdateFontArray(halScreen.FontArray);
    int k;
    for (k = 0; k < FONTS_NUM; ++k)
        halScreen.FontHash[k] = 0;

    halFlags        = 0;
    halProcesses[0] = halProcesses[1] = halProcesses[2] = 0;
    halScreen.HelpMode                                  = 0;
    halScreen.CmdLine                                   = 0;
    halScreen.Menu1                                     = MENU1_HEIGHT;
    halScreen.Menu2                                     = MENU2_HEIGHT;
    halScreen.Stack                                     = 1;
    halSetFormHeight(0);
    halScreen.DirtyFlag   = STACK_DIRTY | MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY;
    halScreen.SAreaTimer  = 0;
    halScreen.CursorTimer = -1;
    halScreen.KeyContext  = CONTEXT_STACK;
    halSetNotification(N_LEFT_SHIFT, 0);
    halSetNotification(N_RIGHT_SHIFT, 0);
    halSetNotification(N_ALPHA, 0);
    halSetNotification(N_LOWBATTERY, 0);
    halSetNotification(N_HOURGLASS, 0);
    halSetNotification(N_DATARECVD, 0);

    // NOT NECESSARILY PART OF HALSCREEN, BUT INITIALIZE THE COMMAND LINE
    uiCloseCmdLine();
    halScreen.StkUndolevels    = 8;
    halScreen.StkCurrentLevel  = 0;

    halScreen.StkVisibleLvl    = 1;
    halScreen.StkVisibleOffset = 0;
}

void halRedrawHelp(gglsurface *scr)
{
    if (!halScreen.Menu2)
    {
        // SHOW THE SECOND MENU TO DISPLAY THE MESSAGE
        halSetMenu2Height(MENU2_HEIGHT);
        halRedrawAll(scr); // THIS CALL WILL CALL HERE RECURSIVELY
        return;            // SO IT'S BEST TO RETURN DIRECTLY
    }

    halScreenUpdated();

    word_p helptext;
    int64_t  m1code  = rplGetMenuCode(halScreen.HelpMode >> 16);
    word_p MenuObj = uiGetLibMenu(m1code);
    int32_t    nitems  = uiCountMenuItems(m1code, MenuObj);
    int32_t    k;
    word_p item;

    // BASIC CHECK OF VALIDITY - COMMANDS MAY HAVE RENDERED THE PAGE NUMBER INVALID
    // FOR EXAMPLE BY PURGING VARIABLES
    if (MENUPAGE(m1code) >= (WORD) nitems)
    {
        m1code = SETMENUPAGE(m1code, 0);
        rplSetMenuCode(halScreen.HelpMode >> 16, m1code);
    }

    if (((halScreen.HelpMode & 0xffff) == 5) && (nitems > 6))
    {
        halScreen.HelpMode = 0; // CLOSE HELP MODE IMMEDIATELY
        halRedrawAll(scr);      // AND ISSUE A REDRAW
        return;
    }

    // GET THE ITEM
    item     = uiGetMenuItem(m1code, MenuObj, (halScreen.HelpMode & 0xffff) + MENUPAGE(m1code));
    helptext = uiGetMenuItemHelp(item);

    if (!helptext)
    {
        halScreen.HelpMode = 0; // CLOSE HELP MODE IMMEDIATELY
        halRedrawAll(scr);      // AND ISSUE A REDRAW
        return;
    }

    if (ISIDENT(*helptext))
    {
        // THE HELP TEXT SHOULD BE THE OBJECT DECOMPILED

        // SPECIAL CASE: FOR IDENTS LOOK FOR VARIABLES AND DRAW DIFFERENTLY IF IT'S A DIRECTORY
        word_p *var = rplFindGlobal(helptext, 1);

        if (!var)
        {
            halScreen.HelpMode = 0; // CLOSE HELP MODE IMMEDIATELY
            halRedrawAll(scr);      // AND ISSUE A REDRAW
            return;
        }

        int32_t SavedException = Exceptions;
        int32_t SavedErrorCode = ErrorCode;

        Exceptions          = 0; // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
        // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
        word_p objdecomp   = rplDecompile(var[1], DECOMP_NOHINTS);
        Exceptions          = SavedException;
        ErrorCode           = SavedErrorCode;

        if (!objdecomp)
            helptext = (word_p) empty_string;
        else
            helptext = objdecomp;

        int32_t ytop = halScreen.Form + halScreen.Stack + halScreen.CmdLine;
        int32_t ybot = ytop + halScreen.Menu1 + halScreen.Menu2 - 1;

        // CLEAR MENU2 AND STATUS AREA
        ggl_cliprect(scr, 0, ytop, LCD_W - 1, ybot, PAL_HLP_BG);
        // DO SOME DECORATIVE ELEMENTS
        ggl_cliphline(scr, ytop, 0, LCD_W - 1, PAL_HLP_LINES);

        // SHOW 3 LINES ONLY

        int32_t namew =
            StringWidthN((char *) (var[0] + 1), ((char *) (var[0] + 1)) + rplGetIdentLength(var[0]), FONT_HLPTITLE);

        // SHOW THE NAME OF THE VARIABLE
        DrawTextN(scr, 3,
                  ytop + 2,
                  (char *) (var[0] + 1),
                  ((char *) (var[0] + 1)) + rplGetIdentLength(var[0]),
                  FONT_HLPTITLE,
                  PAL_HLP_TEXT);
        DrawText(scr, 3 + namew, ytop + 2, ": ", FONT_HLPTITLE, PAL_HLP_TEXT);
        namew += 3 + StringWidth(": ", FONT_HLPTITLE);

        int     xend;
        utf8_p basetext  = (utf8_p) (helptext + 1);
        utf8_p endoftext = basetext + rplStrSize(helptext);
        utf8_p nextline, endofline;

        for (k = 0; k < 3; ++k)
        {
            xend      = LCD_W - 1 - namew;
            endofline = StringCoordToPointer((char *) basetext, (char *) endoftext, FONT_HLPTEXT, &xend);
            if (endofline < endoftext)
            {
                // BACK UP TO THE NEXT WHITE CHARACTER
                utf8_p whitesp = endofline;
                while ((whitesp > basetext) && (*whitesp != ' '))
                    --whitesp;
                if (whitesp >= basetext)
                    endofline = whitesp; // ONLY IF THERE'S WHITESPACES
            }
            nextline = endofline;
            // SKIP ANY NEWLINE OR WHITE CHARACTERS
            while ((nextline < endoftext) &&
                   ((*nextline == ' ') || (*nextline == '\n') || (*nextline == '\t') || (*nextline == '\r')))
                ++nextline;

            // DRAW THE TEXT
            DrawTextN(scr,
                      namew,
                      ytop + 2 + FONT_HEIGHT(FONT_HLPTITLE) +
                          (k - 1) * FONT_HEIGHT(FONT_HLPTEXT),
                      basetext,
                      endofline,
                      FONT_HLPTEXT,
                      PAL_HLP_TEXT);
            basetext = nextline;
            namew    = 3;
        }

        return;
    }

    if (ISSTRING(*helptext))
    {
        int32_t ytop = halScreen.Form + halScreen.Stack + halScreen.CmdLine;
        int32_t ybot = ytop + halScreen.Menu1 + halScreen.Menu2 - 1;

        // CLEAR MENU2 AND STATUS AREA
        ggl_cliprect(scr, 0, ytop, LCD_W - 1, ybot, PAL_HLP_BG);
        // DO SOME DECORATIVE ELEMENTS
        ggl_cliphline(scr, ytop, 0, LCD_W - 1, PAL_HLP_LINES);

        // SHOW MESSAGE'S FIRST 3 LINES ONLY
        int32_t    currentline = 0, nextline;
        byte_p basetext    = (byte_p) (helptext + 1);
        for (k = 0; k < 3; ++k)
        {
            nextline = rplStringGetLinePtr(helptext, 2 + k);
            if (nextline < 0)
                nextline = rplStrSize(helptext);
            DrawTextN(scr, 3,
                      ytop + 2 + FONT_HEIGHT(FONT_HLPTITLE) + k * FONT_HEIGHT(FONT_HLPTEXT),
                      (char *) basetext + currentline,
                      (char *) basetext + nextline,
                      FONT_HLPTEXT,
                      PAL_HLP_TEXT);

            currentline = nextline;
        }

        // FINALLY, SHOW THE NAME OF THE ITEM
        int32_t oldtop = scr->top, oldbottom = scr->bottom;

        scr->top  = ytop + 1;
        scr->bottom = ytop + 1 + FONT_HLPTITLE->BitmapHeight;

        uiDrawHelpMenuItem(scr, item);

        scr->top  = oldtop;
        scr->bottom = oldbottom;

        return;
    }
}

// REDRAW THE VARS MENU
void halRedrawMenu1(gglsurface *scr)
{
    if (halScreen.HelpMode)
    {
        // IN HELP MODE, JUST REDRAW THE HELP MESSAGE
        halRedrawHelp(scr);
        // NO NEED TO REDRAW OTHER MENUS OR THE STATUS AREA
        halScreen.DirtyFlag &= ~(MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY);
        return;
    }
    if (halScreen.Menu1 == 0)
    {
        halScreen.DirtyFlag &= ~MENU1_DIRTY;
        return;
    }

    halScreenUpdated();

    int invMenu = rplTestSystemFlag(FL_MENU1WHITE);
    pattern_t mcolor  = invMenu ? PAL_MENU_BG   : PAL_MENU_TEXT;
    pattern_t bcolor  = invMenu ? PAL_MENU_TEXT : PAL_MENU_BG;

    int ytop, ybottom;
    int oldleft, oldright, oldtop, oldbottom;

#ifndef TARGET_PRIME
    ytop    = halScreen.Form + halScreen.Stack + halScreen.CmdLine;
    ybottom = ytop + halScreen.Menu1 - 1;
    // DRAW BACKGROUND
    ggl_cliprect(scr, 0, ytop + 1, LCD_W - 1, ybottom - 1, bcolor);
    ggl_cliphline(scr, ytop, 0, LCD_W - 1, PAL_MENU_HLINE);
    ggl_cliphline(scr, ybottom, 0, LCD_W - 1, PAL_MENU_HLINE);

    // DRAW VARS OF THE CURRENT DIRECTORY IN THIS MENU
#endif /* ! TARGET_PRIME */

    oldleft  = scr->left;
    oldright = scr->right;
    oldtop  = scr->top;
    oldbottom = scr->bottom;

#ifndef TARGET_PRIME
    int64_t m1code  = rplGetMenuCode(1);
    word_p  MenuObj = uiGetLibMenu(m1code);
    int32_t nitems  = uiCountMenuItems(m1code, MenuObj);
    int32_t k;
    word_p  item;

    // BASIC CHECK OF VALIDITY - COMMANDS MAY HAVE RENDERED THE PAGE NUMBER INVALID
    // FOR EXAMPLE BY PURGING VARIABLES
    if ((MENUPAGE(m1code) >= (WORD) nitems) || (nitems <= 6))
    {
        m1code = SETMENUPAGE(m1code, 0);
        rplSetMenuCode(1, m1code);
    }

    // FIRST ROW
    scr->top  = ytop + 1;
    scr->bottom = ytop + MENU1_HEIGHT - 2;
    for (k = 0; k < 5; ++k)
    {
        scr->left  = MENU_TAB_WIDTH * k;
        scr->right = MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH - 2);
        item        = uiGetMenuItem(m1code, MenuObj, k + MENUPAGE(m1code));
        uiDrawMenuItem(scr, item);
    }

    // NOW DO THE NXT KEY
    scr->left  = MENU_TAB_WIDTH * k;
    scr->right = MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH - 2);

    if (nitems == 6)
    {
        item = uiGetMenuItem(m1code, MenuObj, 5);
        uiDrawMenuItem(scr, item);
        if (nitems > 6)
            DrawText(scr,
                     scr->left + 1,
                     scr->top + 1,
                     "NXT...",
                     FONT_MENU,
                     mcolor);
    }

#else  /* TARGET_PRIME */
    if (halScreen.Menu2 == 0)
    {
        // Draw a one-line horizontal menu with no status area when Menu2 is disabled
        ytop    = halScreen.Form + halScreen.Stack + halScreen.CmdLine;
        ybottom = ytop + halScreen.Menu1 - 1;
        // DRAW BACKGROUND
        ggl_cliprect(scr, 0, ytop + 1, LCD_W - 1, ybottom - 1, bcolor);
        ggl_cliphline(scr, ytop, 0, LCD_W - 1, PAL_MENU_HLINE);
        ggl_cliphline(scr, ybottom, 0, LCD_W - 1, PAL_MENU_HLINE);

        int64_t  m1code  = rplGetMenuCode(1);
        word_p MenuObj = uiGetLibMenu(m1code);
        int32_t    nitems  = uiCountMenuItems(m1code, MenuObj);
        int32_t    k;
        word_p item;

        // BASIC CHECK OF VALIDITY - COMMANDS MAY HAVE RENDERED THE PAGE NUMBER INVALID
        // FOR EXAMPLE BY PURGING VARIABLES
        if ((MENUPAGE(m1code) >= (WORD) nitems) || (nitems <= 6))
        {
            m1code = SETMENUPAGE(m1code, 0);
            rplSetMenuCode(1, m1code);
        }

        // FIRST ROW

        scr->top  = ytop + 1;
        scr->bottom = ytop + MENU1_HEIGHT - 2;

        for (k = 0; k < 5; ++k)
        {
            scr->left  = MENU_TAB_WIDTH * k;
            scr->right = MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH - 2);
            item        = uiGetMenuItem(m1code, MenuObj, k + MENUPAGE(m1code));
            uiDrawMenuItem(scr, item);
        }

        // NOW DO THE NXT KEY
        scr->left  = MENU_TAB_WIDTH * k;
        scr->right = MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH - 2);
        if (nitems == 6)
        {
            item = uiGetMenuItem(m1code, MenuObj, 5);
            uiDrawMenuItem(scr, item);
        }
        else
        {
            if (nitems > 6)
                DrawText(scr, scr->left + 1, scr->top + 1, "NXT...", FONT_MENU, mcolor);
        }
    }
    else
    {
        // Draw a three-line menu with centered status area when Menu2 is enabled

        ytop         = halScreen.Form + halScreen.Stack + halScreen.CmdLine;
        ybottom      = ytop + halScreen.Menu1 + halScreen.Menu2 - 1;
        pattern_t selcolor = halKeyMenuSwitch ? PAL_MENU_HLINE : PAL_MENU_FOCUS_HLINE;
        // DRAW BACKGROUND
        ggl_cliprect(scr, 0, ytop, MENU1_ENDX - 1, ybottom, bcolor);
        ggl_cliphline(scr, ytop, 0, MENU1_ENDX - 1, PAL_MENU_HLINE);
        ggl_cliphline(scr, ytop + halScreen.Menu1 - 1, 0, MENU1_ENDX - 1, selcolor);
        ggl_cliphline(scr, ytop + halScreen.Menu1 + MENU2_HEIGHT / 2 - 1, 0, MENU1_ENDX - 1, selcolor);
        ggl_cliphline(scr, ybottom, 0, MENU1_ENDX - 1, selcolor);

        // DRAW VARS OF THE CURRENT DIRECTORY IN THIS MENU

        int64_t  m1code  = rplGetMenuCode(1);
        word_p MenuObj = uiGetLibMenu(m1code);
        int32_t    nitems  = uiCountMenuItems(m1code, MenuObj);
        int32_t    k;
        word_p item;

        // BASIC CHECK OF VALIDITY - COMMANDS MAY HAVE RENDERED THE PAGE NUMBER INVALID
        // FOR EXAMPLE BY PURGING VARIABLES
        if ((MENUPAGE(m1code) >= (WORD) nitems) || (nitems <= 6))
        {
            m1code = SETMENUPAGE(m1code, 0);
            rplSetMenuCode(1, m1code);
        }
        // FIRST ROW

        scr->top  = ytop + 1;
        scr->bottom = ytop + MENU1_HEIGHT - 2;


        for (k = 0; k < 2; ++k)
        {
            scr->left  = MENU_TAB_WIDTH * k;
            scr->right = MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH - 2);
            item        = uiGetMenuItem(m1code, MenuObj, k + MENUPAGE(m1code));
            uiDrawMenuItem(scr, item);
        }

        // SECOND ROW


        scr->top  = ytop + MENU1_HEIGHT;
        scr->bottom = ytop + MENU1_HEIGHT + MENU2_HEIGHT / 2 - 2;

        for (k = 0; k < 2; ++k)
        {
            scr->left  = MENU_TAB_WIDTH * k;
            scr->right = MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH - 2);
            item        = uiGetMenuItem(m1code, MenuObj, k + 2 + MENUPAGE(m1code));
            uiDrawMenuItem(scr, item);
        }

        // THIRD ROW

        scr->top  = ytop + MENU1_HEIGHT + MENU2_HEIGHT / 2;
        scr->bottom = ybottom - 1;

        k           = 0;
        scr->left  = MENU_TAB_WIDTH * k;
        scr->right = MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH - 2);
        item        = uiGetMenuItem(m1code, MenuObj, k + 4 + MENUPAGE(m1code));
        uiDrawMenuItem(scr, item);

        // NOW DO THE NXT KEY
        scr->left  = MENU_TAB_WIDTH;
        scr->right = MENU_TAB_WIDTH + (MENU_TAB_WIDTH - 2);

        if (nitems == 6)
        {
            item = uiGetMenuItem(m1code, MenuObj, 5);
            uiDrawMenuItem(scr, item);
        }
        else
        {
            if (nitems > 6)
                DrawText(scr, scr->left + 1, scr->top + 1, "NXT...", FONT_MENU, mcolor);
        }
    }
#endif /* TARGET_PRIME */

    scr->left  = oldleft;
    scr->right = oldright;
    scr->top  = oldtop;
    scr->bottom = oldbottom;

    halScreen.DirtyFlag &= ~MENU1_DIRTY;
}

// REDRAW THE OTHER MENU
void halRedrawMenu2(gglsurface *scr)
{
    if (halScreen.HelpMode)
    {
        // IN HELP MODE, JUST REDRAW THE HELP MESSAGE
        halRedrawHelp(scr);
        // NO NEED TO REDRAW OTHER MENUS OR THE STATUS AREA
        halScreen.DirtyFlag &= ~(MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY);
        return;
    }

    if (halScreen.Menu2 == 0)
    {
        halScreen.DirtyFlag &= ~MENU2_DIRTY;
        return;
    }

    halScreenUpdated();

    int invMenu = rplTestSystemFlag(FL_MENU1WHITE);
    pattern_t mcolor = invMenu ? PAL_MENU_BG   : PAL_MENU_TEXT;
    pattern_t bcolor = invMenu ? PAL_MENU_TEXT : PAL_MENU_BG;

    int ytop, ybottom;
    int oldleft, oldright, oldtop, oldbottom;

#ifndef TARGET_PRIME
    ytop    = halScreen.Form + halScreen.Stack + halScreen.CmdLine + halScreen.Menu1;
    ybottom = ytop + halScreen.Menu2 - 1;
    // DRAW BACKGROUND
    ggl_cliprect(scr, 0, ytop, STATUS_AREA_X - 1, ybottom, bcolor);
    // ggl_clipvline(scr,21,ytop+1,ybottom,ggl_color(0x8));
    // ggl_clipvline(scr,43,ytop+1,ybottom,ggl_color(0x8));
    // ggl_clipvline(scr,STATUS_AREA_X-1,ytop+1,ybottom,ggl_color(0x8));
    //    ggl_clipvline(scr,87,ytop,ybottom,0);
    //    ggl_clipvline(scr,109,ytop,ybottom,0);
    // ggl_cliphline(scr,ytop,0,LCD_W-1,ggl_color(0x8));
    ggl_cliphline(scr, ytop + MENU2_HEIGHT / 2 - 1, 0, STATUS_AREA_X - 2, PAL_MENU_HLINE);
    ggl_cliphline(scr, ybottom, 0, STATUS_AREA_X - 2, PAL_MENU_HLINE);

    // DRAW VARS OF THE CURRENT DIRECTORY IN THIS MENU
#endif /* ! TARGET_PRIME */

    oldleft  = scr->left;
    oldright = scr->right;
    oldtop  = scr->top;
    oldbottom = scr->bottom;

#ifdef TARGET_PRIME
    // Draw a three-line menu with centered status area when Menu2 is enabled
    ytop         = halScreen.Form + halScreen.Stack + halScreen.CmdLine;
    ybottom      = ytop + halScreen.Menu1 + halScreen.Menu2 - 1;
    pattern_t selcolor = halKeyMenuSwitch ? PAL_MENU_FOCUS_HLINE : PAL_MENU_HLINE;

    // DRAW BACKGROUND
    ggl_cliprect(scr, MENU2_STARTX, ytop, MENU2_ENDX - 1, ybottom, bcolor);
    ggl_cliphline(scr, ytop, MENU2_STARTX, MENU2_ENDX - 1, PAL_MENU_HLINE);
    ggl_cliphline(scr, ytop + halScreen.Menu1 - 1, MENU2_STARTX, MENU2_ENDX - 1, selcolor);
    ggl_cliphline(scr, ytop + halScreen.Menu1 + MENU2_HEIGHT / 2 - 1, MENU2_STARTX, MENU2_ENDX - 1, selcolor);
    ggl_cliphline(scr, ybottom, MENU2_STARTX, MENU2_ENDX - 1, selcolor);

    ggl_clipvline(scr, MENU2_ENDX, ytop, ybottom, PAL_MENU_HLINE);
    ggl_clipvline(scr, MENU2_STARTX - 1, ytop, ybottom, PAL_MENU_HLINE);

    // DRAW VARS OF THE CURRENT DIRECTORY IN THIS MENU
#endif /* TARGET_PRIME */

    int64_t  m2code  = rplGetMenuCode(2);
    word_p MenuObj = uiGetLibMenu(m2code);
    int32_t    nitems  = uiCountMenuItems(m2code, MenuObj);
    int32_t    k;
    word_p item;

    // BASIC CHECK OF VALIDITY - COMMANDS MAY HAVE RENDERED THE PAGE NUMBER INVALID
    // FOR EXAMPLE BY PURGING VARIABLES
    if ((MENUPAGE(m2code) >= (WORD) nitems) || (nitems <= 6))
    {
        m2code = SETMENUPAGE(m2code, 0);
        rplSetMenuCode(2, m2code);
    }

    // FIRST ROW
#ifndef TARGET_PRIME
    scr->top  = ytop;
    scr->bottom = ytop + MENU2_HEIGHT / 2 - 2;
#else  /* TARGET_PRIME */
    scr->top  = ytop + 1;
    scr->bottom = ytop + MENU1_HEIGHT - 2;
#endif /* TARGET_PRIME */

    for (k = 0; k < MENU2_COUNT; ++k)
    {
        scr->left  = MENU2_STARTX + MENU_TAB_WIDTH * k;
        scr->right = MENU2_STARTX + MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH - 2);
        item        = uiGetMenuItem(m2code, MenuObj, k + MENUPAGE(m2code));
        uiDrawMenuItem(scr, item);
    }

    // SECOND ROW
#ifndef TARGET_PRIME
    scr->top  = ytop + MENU2_HEIGHT / 2;
    scr->bottom = ybottom - 1;
#else  /* TARGET_PRIME */
    scr->top  = ytop + MENU1_HEIGHT;
    scr->bottom = ytop + MENU1_HEIGHT + MENU2_HEIGHT / 2 - 3;
#endif /* TARGET_PRIME */

    for (k = 0; k < 2; ++k)
    {
        scr->left  = MENU2_STARTX + MENU_TAB_WIDTH * k;
        scr->right = MENU2_STARTX + MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH - 2);
        item        = uiGetMenuItem(m2code, MenuObj, k + 2 + MENUPAGE(m2code));
        uiDrawMenuItem(scr, item);
    }

#ifdef TARGET_PRIME

    // THIRD ROW
    scr->top  = ytop + MENU1_HEIGHT + MENU2_HEIGHT / 2;
    scr->bottom = ybottom - 1;

    k           = 0;
    scr->left  = MENU2_STARTX + MENU_TAB_WIDTH * k;
    scr->right = MENU2_STARTX + MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH - 2);
    item        = uiGetMenuItem(m2code, MenuObj, k + 4 + MENUPAGE(m2code));
    uiDrawMenuItem(scr, item);
#endif /* TARGET_PRIME */

    // NOW DO THE NXT KEY
#ifndef TARGET_PRIME
    scr->left  = MENU_TAB_WIDTH * k;
    scr->right = MENU_TAB_WIDTH * k + (MENU_TAB_WIDTH - 2);
#else  /* TARGET_PRIME */
    scr->left  = MENU2_STARTX + MENU_TAB_WIDTH;
    scr->right = MENU2_STARTX + MENU_TAB_WIDTH + (MENU_TAB_WIDTH - 2);
#endif /* TARGET_PRIME */

    if (nitems == 6)
    {
        item = uiGetMenuItem(m2code, MenuObj, 5);
        uiDrawMenuItem(scr, item);
    }
    else
    {
        if (nitems > 6)
            DrawText(scr, scr->left + 1, scr->top + 1, "NXT...", FONT_MENU, mcolor);
    }

    scr->left  = oldleft;
    scr->right = oldright;
    scr->top  = oldtop;
    scr->bottom = oldbottom;

    halScreen.DirtyFlag &= ~MENU2_DIRTY;
}

void halRedrawStatus(gglsurface *scr)
{
    if (halScreen.HelpMode)
    {
        // IN HELP MODE, JUST REDRAW THE HELP MESSAGE
        halRedrawHelp(scr);
        // NO NEED TO REDRAW OTHER MENUS OR THE STATUS AREA
        halScreen.DirtyFlag &= ~(MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY);
        return;
    }

    halScreenUpdated();

    if (halScreen.Menu2)
    {
#ifndef TARGET_PRIME
        int ytop = halScreen.Form + halScreen.Stack + halScreen.CmdLine + halScreen.Menu1;
        ggl_cliprect(scr, STATUS_AREA_X, ytop, LCD_W - 1, ytop + halScreen.Menu2 - 1, PAL_STA_BG);
#else  /* TARGET_PRIME */
        int ytop = halScreen.Form + halScreen.Stack + halScreen.CmdLine;

        ggl_hline(scr, ytop, STATUS_AREA_X, LCD_W - 1, PAL_MENU_HLINE);
        ggl_cliprect(scr,
                     STATUS_AREA_X,
                     ytop + 1,
                     LCD_W - 1,
                     ytop + halScreen.Menu1 + halScreen.Menu2 - 1,
                     PAL_STA_BG);
#endif /* TARGET_PRIME */
        int32_t xc, yc;
        xc         = scr->left;
        yc         = scr->top;
        scr->left = STATUS_AREA_X;
        scr->top = ytop;

#ifdef TARGET_PRIME
        ytop++;
#endif // TARGET_PRIME

        // AUTOCOMPLETE
        if (halScreen.CmdLineState & CMDSTATE_ACACTIVE)
        {
            byte_p namest;
            byte_p nameend;
            if (halScreen.ACSuggestion != 0)
            {
                // DISPLAY THE CURRENTLY SELECTED AUTOCOMPLETE COMMAND IN THE
                // SECOND LINE
                if (!Exceptions)
                {
                    // BUT ONLY IF THERE WERE NO ERRORS
                    int32_t    y       = ytop + 1 + FONT_STATUS->BitmapHeight;

                    // FOR NOW JUST DISPLAY THE SELECTED TOKEN
                    word_p cmdname = rplDecompile(((ISPROLOG(halScreen.ACSuggestion) && SuggestedObject)
                                                        ? SuggestedObject
                                                        : (&halScreen.ACSuggestion)),
                                                   DECOMP_NOHINTS);
                    if ((!cmdname) || Exceptions)
                    {
                        // JUST IGNORE, CLEAR EXCEPTIONS AND RETURN;
                        Exceptions = 0;
                        halScreen.DirtyFlag &= ~STAREA_DIRTY;
                        return;
                    }

                    namest  = (byte_p) (cmdname + 1);
                    nameend = namest + rplStrSize(cmdname);
                    DrawTextBkN(scr, STATUS_AREA_X + 2,
                                y,
                                (char *) namest,
                                (char *) nameend,
                                FONT_STATUS,
                                PAL_STA_TEXT,
                                PAL_STA_BG);
                }
            }
        }
        else
        {
            // SHOW CURRENT PATH ON SECOND LINE
            int32_t    nnames, j, width, xst;
            word_p pathnames[8], lastword;
            byte_p start, end;
            int32_t    y = ytop + 1 + FONT_HEIGHT(FONT_STATUS);

            nnames    = rplGetFullPath(CurrentDir, pathnames, 8);

            // COMPUTE THE WIDTH OF ALL NAMES
            width     = 0;
            for (j = nnames - 1; j >= 0; --j)
            {
                if (ISIDENT(*pathnames[j]))
                {
                    start    = (byte_p) (pathnames[j] + 1);
                    lastword = rplSkipOb(pathnames[j]) - 1;
                    if (*lastword & 0xff000000)
                    {
                        end = (byte_p) (lastword + 1);
                        width += StringWidthN((char *) start, (char *) end, FONT_STATUS);
                    }
                    else
                        width += StringWidth((char *) start, FONT_STATUS);
                }
            }
            // ADD WIDTH OF SYMBOLS
            width += 4 * nnames;
            if (width > LCD_W - STATUS_AREA_X)
                xst = LCD_W - width;
            else
                xst = STATUS_AREA_X;

            // NOW DRAW THE PATH
            for (j = nnames - 1; j >= 0; --j)
            {
                if (ISIDENT(*pathnames[j]))
                {
                    start    = (byte_p) (pathnames[j] + 1);
                    lastword = rplSkipOb(pathnames[j]) - 1;
                    xst = DrawTextBk(scr, xst, y, "/", FONT_STATUS, PAL_STA_TEXT, PAL_STA_BG);
                    if (*lastword & 0xff000000)
                    {
                        end = (byte_p) (lastword + 1);
                        xst = DrawTextBkN(scr, xst,
                                          y,
                                          (char *) start,
                                          (char *) end,
                                          FONT_STATUS,
                                          PAL_STA_TEXT,
                                          PAL_STA_BG);
                    }
                    else
                        xst = DrawTextBk(scr, xst,
                                         y,
                                         (char *) start,
                                         FONT_STATUS,
                                         PAL_STA_TEXT,
                                         PAL_STA_BG);

                }
            }
#if 0
            if(width > LCD_W - STATUS_AREA_X) {
                // FADE THE TEXT OUT

                scr->x = STATUS_AREA_X;
                ggl_filter(scr, 2, FONT_HEIGHT(FONT_STATUS), 0xA, &ggl_fltlighten);
                scr->x += 2;
                ggl_filter(scr, 2, FONT_HEIGHT(FONT_STATUS), 0x6, &ggl_fltlighten);
                scr->x += 2;
                ggl_filter(scr, 2, FONT_HEIGHT(FONT_STATUS), 0x4, &ggl_fltlighten);

            }

            if (width > LCD_W - STATUS_AREA_X)
            {
                int rf, rb, gf, gb, bf, bb;

                rf              = RGBRED(ggl_color(PAL_STA_TEXT));
                rb              = RGBRED(ggl_color(PAL_STA_BG));
                gf              = RGBGREEN(ggl_color(PAL_STA_TEXT));
                gb              = RGBGREEN(ggl_color(PAL_STA_BG));
                bf              = RGBBLUE(ggl_color(PAL_STA_TEXT));
                bb              = RGBBLUE(ggl_color(PAL_STA_BG));

                // CREATE 3 INTERPOLATED COLORS: (3F+B)/4, (F+B)/2 AND (F+3B)/4
                int vanishwidth = MENU_TAB_WIDTH / 16;

                scr->x          = STATUS_AREA_X;

                ggl_filter(scr,
                           vanishwidth,
                           FONT_HEIGHT(FONT_STATUS),
                           RGB_TO_RGB16((rf + 3 * rb) >> 2, (gf + 3 * gb) >> 2, (bf + 3 * bb) >> 2).value |
                               (ggl_color(PAL_STA_TEXT).value << 16),
                           &ggl_fltreplace);
                scr->x += vanishwidth;
                ggl_filter(scr,
                           vanishwidth,
                           FONT_HEIGHT(FONT_STATUS),
                           RGB_TO_RGB16((rf + rb) >> 1, (gf + gb) >> 1, (bf + bb) >> 1).value |
                               (ggl_color(PAL_STA_TEXT).value << 16),
                           &ggl_fltreplace);
                scr->x += vanishwidth;
                ggl_filter(scr,
                           vanishwidth,
                           FONT_HEIGHT(FONT_STATUS),
                           RGB_TO_RGB16((3 * rf + rb) >> 2, (3 * gf + gb) >> 2, (3 * bf + bb) >> 2).value |
                               (ggl_color(PAL_STA_TEXT).value << 16),
                           &ggl_fltreplace);
            }
#endif /* Disabled code */
        }

        int xctracker = 0; // TRACK THE WIDTH OF THE INDICATORS TO MAKE SURE THEY ALL FIT

        // ANGLE MODE INDICATOR
        {
            int32_t              anglemode = rplTestSystemFlag(FL_ANGLEMODE1) | (rplTestSystemFlag(FL_ANGLEMODE2) << 1);
            const char *const name[4]   = { "∡°", "∡r", "∡g", "∡d" };

            DrawTextBk(scr,
                       STATUS_AREA_X + 1,
                       ytop + 1,
                       name[anglemode],
                       FONT_STATUS,
                       PAL_STA_TEXT,
                       PAL_STA_BG);
            xctracker += 4 + StringWidth((char *) name[anglemode], FONT_STATUS);
        }

        // COMPLEX MODE INDICATOR

        if (rplTestSystemFlag(FL_COMPLEXMODE))
        {
            DrawTextBk(scr, STATUS_AREA_X + 1 + xctracker,
                       ytop + 1,
                       (char *) "C",
                       FONT_STATUS,
                       PAL_STA_TEXT,
                       PAL_STA_BG);
            xctracker += 4 + StringWidth((char *) "C", FONT_STATUS);
        }

        // HALTED PROGRAM INDICATOR
        if (halFlags & HAL_HALTED)
        {
            DrawTextBk(scr, STATUS_AREA_X + 1 + xctracker,
                       ytop + 1,
                       (char *) "H",
                       FONT_STATUS,
                       PAL_STA_TEXT,
                       PAL_STA_BG);
            xctracker += 4 + StringWidth((char *) "H", FONT_STATUS);
        }

        // FIRST 6 USER FLAGS
        {
            uint64_t *flags = rplGetUserFlagsLow();
            if (flags)
            {
                ggl_rect(scr,
                         STATUS_AREA_X + 1 + xctracker,
                         ytop + 1,
                         STATUS_AREA_X + 6 + xctracker,
                         ytop + 6,
                         PAL_STA_UFLAG0);

                if (*flags & 4)
                    ggl_rect(scr,
                             STATUS_AREA_X + 1 + xctracker,
                             ytop + 1,
                             STATUS_AREA_X + 2 + xctracker,
                             ytop + 3,
                             PAL_STA_UFLAG1);
                if (*flags & 2)
                    ggl_rect(scr,
                             STATUS_AREA_X + 3 + xctracker,
                             ytop + 1,
                             STATUS_AREA_X + 4 + xctracker,
                             ytop + 3,
                             PAL_STA_UFLAG1);
                if (*flags & 1)
                    ggl_rect(scr,
                             STATUS_AREA_X + 5 + xctracker,
                             ytop + 1,
                             STATUS_AREA_X + 6 + xctracker,
                             ytop + 3,
                             PAL_STA_UFLAG1);

                if (*flags & 32)
                    ggl_rect(scr,
                             STATUS_AREA_X + 1 + xctracker,
                             ytop + 4,
                             STATUS_AREA_X + 2 + xctracker,
                             ytop + 6,
                             PAL_STA_UFLAG1);
                if (*flags & 16)
                    ggl_rect(scr,
                             STATUS_AREA_X + 3 + xctracker,
                             ytop + 4,
                             STATUS_AREA_X + 4 + xctracker,
                             ytop + 6,
                             PAL_STA_UFLAG1);
                if (*flags & 8)
                    ggl_rect(scr,
                             STATUS_AREA_X + 5 + xctracker,
                             ytop + 4,
                             STATUS_AREA_X + 6 + xctracker,
                             ytop + 6,
                             PAL_STA_UFLAG1);

                xctracker += 7;
            }
        }

#ifndef TARGET_PRIME
        // NOTIFICATION ICONS! ONLY ONE WILL BE DISPLAYED AT A TIME
        if (halGetNotification(N_ALARM))
        {
            char txt[4];
            txt[0] = 'A';
            txt[1] = 'L';
            txt[2] = 'M';
            txt[3] = 0;
            DrawTextBk(scr, STATUS_AREA_X + xctracker,
                       ytop + 1,
                       txt,
                       FONT_STATUS,
                       PAL_STA_TEXT,
                       PAL_STA_BG);
            xctracker += 2 + StringWidth(txt, FONT_STATUS);
        }
        else if (halGetNotification(N_DATARECVD))
        {
            char txt[4];
            txt[0] = 'R';
            txt[1] = 'X';
            txt[2] = ' ';
            txt[3] = 0;
            DrawTextBk(scr, STATUS_AREA_X + xctracker,
                       ytop + 1,
                       txt,
                       FONT_STATUS,
                       PAL_STA_TEXT,
                       PAL_STA_BG);
            xctracker += 2 + StringWidth(txt, FONT_STATUS);
        }
#endif // TARGET_PRIME

#ifndef CONFIG_NO_FSYSTEM
        // SD CARD INSERTED INDICATOR
        {
            char txt[4];
            int color;
            txt[0] = 'S';
            txt[1] = 'D';
            txt[2] = ' ';
            txt[3] = 0;
            if (FSCardInserted())
                color = 6;
            else
                color = 0;
            if (FSIsInit())
            {
                if (FSVolumeMounted(FSGetCurrentVolume()))
                    color = 0xf;
                if (!FSCardInserted())
                {
                    txt[2] = '?';
                    color  = 6;
                }
                else if (FSCardIsSDHC())
                {
                    txt[0] = 'H';
                    txt[1] = 'C';
                }
                int k = FSIsDirty();
                if (k == 1)
                    color = -1; // 1 =  DIRTY FS NEEDS FLUSH
                if (k == 2)
                    color = -2; // 2 =  FS IS FLUSHED BUT THERE'S OPEN FILES
            }

            if (color)
            {
                if (color == -1)
                {
                    DrawTextBk(scr, STATUS_AREA_X + 1 + xctracker,
                               ytop + 1,
                               txt,
                               FONT_STATUS,
                               PAL_STA_BG,
                               PAL_STA_TEXT);
                }
                else
                {
                    if (color == -2)
                        DrawTextBk(scr, STATUS_AREA_X + 1 + xctracker,
                                   ytop + 1,
                                   txt,
                                   FONT_STATUS,
                                   PAL_STA_BG,
                                   PAL_STA_TEXT);
                    else
                        DrawTextBk(scr, STATUS_AREA_X + 1 + xctracker,
                                   ytop + 1,
                                   txt,
                                   FONT_STATUS,
                                   PAL_STA_TEXT,
                                   PAL_STA_BG);
                }
                xctracker += 4 + StringWidth(txt, FONT_STATUS);
            }
        }
#endif // CONFIG_NO_FSYSTEM

#if !defined(ANN_X_COORD) || !defined(ANN_Y_COORD)
        // NOTIFICATION ICONS! ONLY ONE WILL BE DISPLAYED AT A TIME
        xctracker = 4;
        ytop      = LCD_H - 1 - Font_Notifications->BitmapHeight;

        unsigned highlightFlags =
            ((halGetNotification(N_DATARECVD) != 0)     << 1)   |
            (((keyb_flags & KHOLD_LEFT)       != 0)     << 2)   |
            (((keyb_flags & KHOLD_RIGHT)      != 0)     << 3)   |
            (((keyb_flags & KHOLD_ALPHA)      != 0)     << 4);

        static struct annunciator
        {
            int id;
            int highlight;
            utf8_p label;
        } annunciators[] = {
            {      N_ALARM, 0, "X"},
            { N_CONNECTION, 1, "U"},
            { N_LEFT_SHIFT, 2, "L"},
            {N_RIGHT_SHIFT, 3, "R"},
            {      N_ALPHA, 4, "A"},
            {  N_HOURGLASS, 0, "W"},
        };

        record(annunciators, "Annunciators highlight=%X\n", highlightFlags);
        const unsigned count = sizeof(annunciators)/sizeof(annunciators[0]);
        for (unsigned k = 0; k < count; k++)
        {
            if (halGetNotification(annunciators[k].id))
            {
                pattern_t fg = PAL_STA_ANN;
                pattern_t bg = PAL_STA_BG;

                record(annunciators, "  %d: highlight=%d",
                       k, (highlightFlags >> annunciators[k].highlight) & 1);
                if ((highlightFlags >> annunciators[k].highlight) & 1)
                    fg = PAL_STA_ANNPRESS;
                DrawTextBk(scr,
                           STATUS_AREA_X + 1 + xctracker,
                           ytop + 1,
                           annunciators[k].label,
                           Font_Notifications,
                           fg,
                           bg);
                xctracker += 4 + StringWidth((char *) "X", Font_Notifications);
            }
        }
#endif // No physical annunciators

        // ADD OTHER INDICATORS HERE
        scr->left = xc;
        scr->top = yc;
    }

    halScreen.DirtyFlag &= ~STAREA_DIRTY;
}

void halRedrawCmdLine(gglsurface *scr)
{
    halScreenUpdated();

    if (halScreen.CmdLine)
    {
        int  ytop = halScreen.Form + halScreen.Stack;
        size rowh = FONT_HEIGHT(FONT_CMDLINE);

        if ((halScreen.DirtyFlag & CMDLINE_ALLDIRTY) == CMDLINE_ALLDIRTY)
        {
            ggl_cliphline(scr, ytop, 0, LCD_W - 1, PAL_DIV_LINE);
            ggl_cliphline(scr, ytop + 1, 0, LCD_W - 1, PAL_CMD_BG);
        }

        size   rows    = halScreen.LineCurrent - halScreen.LineVisible;
        coord  y       = rows * rowh;
        utf8_p cmdline = (utf8_p) (CmdLineCurrentLine + 1);
        size_t nchars  = rplStrSize(CmdLineCurrentLine);

        if (halScreen.DirtyFlag & CMDLINE_DIRTY)
        {
            // Show other lines here except the current edited line
            coord startoff = -1;
            coord endoff = -1;
            for (int k = 0; k < halScreen.NumLinesVisible; ++k)
            {
                int row = halScreen.LineVisible + k;

                // Update the line
                if (row < 1)
                    continue;

                if (row == halScreen.LineCurrent)
                {
                    if (startoff < 0)
                        continue;
                    startoff = endoff;
                    endoff = startoff < 0
                        ? -1
                        : rplStringGetNextLine(CmdLineText, startoff);
                    continue;
                }
                startoff = startoff < 0
                    ? rplStringGetLinePtr(CmdLineText, row)
                    : endoff;

                endoff = startoff < 0
                    ? -1
                    : rplStringGetNextLine(CmdLineText, startoff);

                coord xcoord  = -halScreen.XVisible;
                coord ycoord  = ytop + 2 + k * rowh;
                coord ybottom = ycoord + rowh - 1;
                if (startoff >= 0 || endoff >= 0)
                {
                    utf8_p string = (utf8_p) (CmdLineText + 1) + startoff;
                    utf8_p strend = (startoff >= 0 && endoff < 0)
                        ? (utf8_p) (CmdLineText + 1) + rplStrSize(CmdLineText)
                        : (utf8_p) (CmdLineText + 1) + endoff;
                    utf8_p selst  = strend;
                    utf8_p selend = strend;
                    coord  tail   = 0;

                    if (halScreen.SelStartLine < row)
                    {
                        selst = string;
                        tail  = 1;
                    }
                    if (halScreen.SelStartLine == row)
                    {
                        selst = string + halScreen.SelStart;
                        tail  = 1;
                    }
                    if (halScreen.SelEndLine < row)
                    {
                        selend = string;
                        tail   = 0;
                    }
                    if (halScreen.SelEndLine == row)
                    {
                        selend = string + halScreen.SelEnd;
                        tail   = 0;
                    }

                    if (selend <= selst)
                        selend = selst = string;

                    // Draw the line split in 3 sections:
                    // - string to selst
                    // - selst to selend
                    // - selend to strend
                    if (selst > string)
                    {
                        xcoord = DrawTextBkN(scr,
                                             xcoord,
                                             ycoord,
                                             string,
                                             selst,
                                             FONT_CMDLINE,
                                             PAL_CMD_TEXT,
                                             PAL_CMD_BG);
                    }
                    if (selend > selst)
                    {
                        xcoord = DrawTextBkN(scr,
                                             xcoord,
                                             ycoord,
                                             selst,
                                             selend,
                                             FONT_CMDLINE,
                                             PAL_CMD_SELTEXT,
                                             PAL_CMD_SEL_BG);
                    }
                    if (strend > selend)
                    {
                        xcoord = DrawTextBkN(scr,
                                             xcoord,
                                             ycoord,
                                             selend,
                                             strend,
                                             FONT_CMDLINE,
                                             PAL_CMD_TEXT,
                                             PAL_CMD_BG);
                    }
                    if (tail)
                    {
                        ggl_cliprect(scr,
                                     xcoord,
                                     ycoord,
                                     xcoord + 3,
                                     ybottom,
                                     PAL_CMD_SEL_BG);
                        xcoord += 3;
                    }
                }

                // Clear up to end of line
                ggl_cliprect(scr,
                             xcoord,
                             ycoord,
                             LCD_SCANLINE - 1,
                             ybottom,
                             PAL_CMD_BG);
            }
        }

        if (halScreen.DirtyFlag & CMDLINE_LINEDIRTY)
        {
            // Update the current line
            utf8_p string = cmdline;
            utf8_p strend = cmdline + nchars;
            utf8_p selst  = strend;
            utf8_p selend = strend;
            coord  tail   = 0;
            if (halScreen.SelStartLine < halScreen.LineCurrent)
            {
                selst = string;
                tail  = 1;
            }
            if (halScreen.SelStartLine == halScreen.LineCurrent)
            {
                selst = string + halScreen.SelStart;
                tail  = 1;
            }
            if (halScreen.SelEndLine < halScreen.LineCurrent)
            {
                selend = string;
                tail   = 0;
            }
            if (halScreen.SelEndLine == halScreen.LineCurrent)
            {
                selend = string + halScreen.SelEnd;
                tail   = 0;
            }

            if (selend <= selst)
                selend = selst = string;

            // Draw the line split in 3 sections:
            // - string to selst,
            // - selst to selend,
            // - selend to strend
            coord xcoord  = -halScreen.XVisible;
            coord ycoord  = ytop + 2 + y;
            coord ybottom = ycoord + rowh - 1;
            if (selst > string)
            {
                xcoord = DrawTextBkN(scr,
                                     xcoord,
                                     ycoord,
                                     string,
                                     selst,
                                     FONT_CMDLINE,
                                     PAL_CMD_TEXT,
                                     PAL_CMD_BG);
            }
            if (selend > selst)
            {
                xcoord = DrawTextBkN(scr,
                                     xcoord,
                                     ycoord,
                                     selst,
                                     selend,
                                     FONT_CMDLINE,
                                     PAL_CMD_SELTEXT,
                                     PAL_CMD_SEL_BG);
            }
            if (strend > selend)
            {
                xcoord = DrawTextBkN(scr,
                                     xcoord,
                                     ycoord,
                                     selend,
                                     strend,
                                     FONT_CMDLINE,
                                     PAL_CMD_TEXT,
                                     PAL_CMD_BG);
            }
            if (tail)
            {
                ggl_cliprect(scr,
                             xcoord,
                             ycoord,
                             xcoord + 3,
                             ybottom,
                             PAL_CMD_SEL_BG);
                xcoord += 3;
            }

            // Clear up to end of line
            ggl_cliprect(scr,
                         xcoord,
                         ycoord,
                         LCD_SCANLINE - 1,
                         ybottom,
                         PAL_CMD_BG);
        }

        if (halScreen.DirtyFlag & CMDLINE_CURSORDIRTY)
        {
            // Draw the cursor
            coord xcoord  = halScreen.CursorX - halScreen.XVisible;
            coord ycoord  = ytop + 2 + y;
            coord ybottom = ycoord + rowh - 1;
            if (!(halScreen.CursorState & 0x8000))
            {
                DrawTextBkN(scr,
                            xcoord,
                            ycoord,
                            (utf8_p) &halScreen.CursorState,
                            (utf8_p) (&halScreen.CursorState) + 1,
                            FONT_CURSOR,
                            PAL_CMD_CURSOR,
                            PAL_CMD_CURSOR_BG);
            }
            else
            {
                scr->left  = xcoord;

                 // Hard code maximum width of the cursor
                scr->right = min(scr->left + rowh + 4, LCD_W - 1);

                // Redraw the portion of command line under the cursor
                if (!(halScreen.DirtyFlag & CMDLINE_LINEDIRTY))
                {
                    // Update the current line
                    utf8_p string = cmdline;
                    utf8_p strend = cmdline + nchars;
                    coord  tail   = 0;
                    utf8_p selst  = strend;
                    utf8_p selend = selend;
                    if (halScreen.SelStartLine < halScreen.LineCurrent)
                    {
                        selst = string;
                        tail  = 1;
                    }
                    if (halScreen.SelStartLine == halScreen.LineCurrent)
                    {
                        selst = string + halScreen.SelStart;
                        tail  = 1;
                    }
                    if (halScreen.SelEndLine < halScreen.LineCurrent)
                    {
                        selend = string;
                        tail   = 0;
                    }
                    if (halScreen.SelEndLine == halScreen.LineCurrent)
                    {
                        selend = string + halScreen.SelEnd;
                        tail   = 0;
                    }

                    if (selend <= selst)
                        selend = selst = string;

                    // Draw the line split in 3 sections:
                    // - string to selst,
                    // - selst to selend,
                    // - selend to strend
                    xcoord = -halScreen.XVisible;
                    if (selst > string)
                    {
                        xcoord = DrawTextBkN(scr,
                                             xcoord,
                                             ycoord,
                                             string,
                                             selst,
                                             FONT_CMDLINE,
                                             PAL_CMD_TEXT,
                                             PAL_CMD_BG);
                    }
                    if (selend > selst)
                    {
                        xcoord = DrawTextBkN(scr,
                                             xcoord,
                                             ycoord,
                                             selst,
                                             selend,
                                             FONT_CMDLINE,
                                             PAL_CMD_SELTEXT,
                                             PAL_CMD_SEL_BG);
                    }
                    if (strend > selend)
                    {
                        xcoord = DrawTextBkN(scr,
                                             xcoord,
                                             ycoord,
                                             selend,
                                             strend,
                                             FONT_CMDLINE,
                                             PAL_CMD_TEXT,
                                             PAL_CMD_BG);
                    }

                    if (tail)
                    {
                        ggl_cliprect(scr,
                                     xcoord,
                                     ycoord,
                                     xcoord + 3,
                                     ybottom,
                                     PAL_CMD_SEL_BG);
                        xcoord += 3;
                    }

                    // Clear up to end of line
                    ggl_cliprect(scr,
                                 xcoord,
                                 ycoord,
                                 LCD_SCANLINE - 1,
                                 ybottom,
                                 PAL_CMD_BG);
                }

                // Reset the clipping rectangle back to whole screen
                scr->left  = 0;
                scr->right = scr->width - 1;
                scr->top  = 0;
                scr->bottom = scr->height - 1;
            }
        }
    }

    halScreen.DirtyFlag &= ~CMDLINE_ALLDIRTY;
}

// GET NEW FONT DATA FROM THE RPL ENVIRONMENT
void halUpdateFonts()
{
    UNIFONT const *tmparray[FONTS_NUM];
    WORD           hash;
    halUpdateFontArray(tmparray);

    int f;
    for (f = 0; f < FONTS_NUM; ++f)
    {
        // COMPUTE THE HASH OF THE FONT
        hash = OPCODE(tmparray[f]->Prolog) | (tmparray[f]->BitmapHeight << 20);
        if (halScreen.FontHash[f] != hash)
        {
            halScreen.FontArray[f] = tmparray[f];
            halScreen.FontHash[f]  = hash;
            switch (f)
            {
            case FONT_INDEX_STACK:
            case FONT_INDEX_STACKLVL1:
                uiClearRenderCache();
                halScreen.DirtyFlag |= STACK_DIRTY;
                break;
            case FONT_INDEX_CMDLINE:
            case FONT_INDEX_CURSOR:
                if (halScreen.CmdLine)
                {
                    halScreen.DirtyFlag |= CMDLINE_ALLDIRTY;
                    uiStretchCmdLine(0);
                }
                break;
            case FONT_INDEX_MENU:
                if (halScreen.Menu1)
                    halSetMenu1Height(MENU1_HEIGHT);
                if (halScreen.Menu2)
                    halSetMenu2Height(MENU2_HEIGHT);
                halScreen.DirtyFlag |= MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY;
                break;
            case FONT_INDEX_STATUS: halScreen.DirtyFlag |= MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY; break;
            case FONT_INDEX_PLOT: halScreen.DirtyFlag |= FORM_DIRTY; break;
            case FONT_INDEX_FORMS: halScreen.DirtyFlag |= FORM_DIRTY; break;
            case FONT_INDEX_HLPTITLE:
            case FONT_INDEX_HLPTEXT: break;
            }
        }
        else
            halScreen.FontArray[f] = tmparray[f];
    }
}

// PREPARE TO DRAW ON THE ALTERNATIVE BUFFER
void halPrepareBuffer(gglsurface *scr)
{
#ifndef TARGET_PRIME
    UNUSED(scr);
#else  // TARGET_PRIME
    size_t offset = LCD_W * LCD_H / PIXELS_PER_WORD;
    pixword *base = (pixword *) MEM_PHYS_SCREEN;
    if (scr->pixels == base)
        base += offset;

    // Avoid background processes from writing to the buffer while we copy it
    halScreen.DirtyFlag |= BUFFER_LOCK;

    // Copy data between buffers (no overlap)
    memcpy(base, scr->pixels, offset * sizeof(pixword));

    // Let background processes know to use the alternative buffer
    halScreen.DirtyFlag |= BUFFER_ALT;

    // Remove the lock
    halScreen.DirtyFlag &= ~BUFFER_LOCK;

    scr->pixels = base;
#endif /* TARGET_PRIME */
}

void halSwapBuffer(gglsurface *scr)
{
#ifndef TARGET_PRIME
    UNUSED(scr);
#else  // TARGET_PRIME
    // Show new buffer on the screen
    // Avoid background processes from writing to the buffer while we copy it
    halScreen.DirtyFlag |= BUFFER_LOCK;

    pixword *base = (pixword *) MEM_PHYS_SCREEN;
    lcd_setactivebuffer(scr->pixels != base);
    halScreen.DirtyFlag &= ~BUFFER_ALT;

    // Remove the lock
    halScreen.DirtyFlag &= ~BUFFER_LOCK;

    halScreenUpdated();
#endif /* TARGET_PRIME */
}

void halForceRedrawAll(gglsurface *scr)
{
    halPrepareBuffer(scr);
    halUpdateFonts();
    halRedrawForm(scr);
    halRedrawStack(scr);
    halRedrawCmdLine(scr);
    halRedrawMenu1(scr);
    halRedrawMenu2(scr);
    halRedrawStatus(scr);
    halSwapBuffer(scr);
}

void halRedrawAll(gglsurface *scr)
{
    if (halScreen.DirtyFlag)
    {
        halUpdateFonts();
        halPrepareBuffer(scr);

        if (halScreen.DirtyFlag & FORM_DIRTY)
            halRedrawForm(scr);
        if (halScreen.DirtyFlag & STACK_DIRTY)
            halRedrawStack(scr);
        if (halScreen.DirtyFlag & CMDLINE_ALLDIRTY)
            halRedrawCmdLine(scr);
        if (halScreen.DirtyFlag & MENU1_DIRTY)
            halRedrawMenu1(scr);
        if (!halScreen.SAreaTimer)
        {
            // ONLY REDRAW IF THERE'S NO POPUP MESSAGES
#ifdef TARGET_PRIME
            if (halScreen.DirtyFlag & MENU1_DIRTY)
                halRedrawMenu1(scr);
#endif /* TARGET_PRIME */
            if (halScreen.DirtyFlag & MENU2_DIRTY)
                halRedrawMenu2(scr);
            if (halScreen.DirtyFlag & STAREA_DIRTY)
                halRedrawStatus(scr);
        }
        halSwapBuffer(scr);
    }
}

// MARK STATUS AREA FOR IMMEDIATE UPDATE
void halUpdateStatus()
{
    halScreen.DirtyFlag |= STAREA_DIRTY;
}

void status_popup_handler()
{
    if (rplTestSystemFlag(FL_HIDEMENU2))
    {
        halSetMenu2Height(0);
    }
    else
    {
#ifdef TARGET_PRIME
        halScreen.DirtyFlag |= STAREA_DIRTY | MENU1_DIRTY | MENU2_DIRTY;
#else  // !TARGET_PRIME
        gglsurface scr;
        ggl_init_screen(&scr);
        halRedrawMenu1(&scr);
        halRedrawMenu2(&scr);
        halRedrawStatus(&scr);
#endif // TARGET_PRIME
    }
    halScreen.SAreaTimer = 0;
}

// WILL KEEP THE STATUS AREA AS-IS FOR 5 SECONDS, THEN REDRAW IT
// TO CLEAN UP POP-UP MESSAGES
void halStatusAreaPopup()
{
    if (halScreen.SAreaTimer)
    {
        tmr_eventkill(halScreen.SAreaTimer);
        status_popup_handler();
        // tmr_eventpause(halScreen.SAreaTimer);
        // tmr_eventresume(halScreen.SAreaTimer);      // PAUSE/RESUME WILL RESTART THE 5 SECOND COUNT
        // return;
    }
    halScreen.SAreaTimer = tmr_eventcreate(&status_popup_handler, 3000, 0);
}

void halCancelPopup()
{
    if (halScreen.SAreaTimer)
    {
        tmr_eventkill(halScreen.SAreaTimer);
        // MARK DIRTY BUT DON'T REDRAW YET
        halScreen.DirtyFlag |= STAREA_DIRTY | MENU2_DIRTY;
        halScreen.SAreaTimer = 0;
    }
    if (rplTestSystemFlag(FL_HIDEMENU2))
        halSetMenu2Height(0);
}

// WILL KEEP THE STATUS AREA AS-IS FOR 5 SECONDS, THEN REDRAW IT
// TO CLEAN UP POP-UP MESSAGES
void halErrorPopup()
{
    if (halScreen.SAreaTimer)
    {
        tmr_eventkill(halScreen.SAreaTimer);
        status_popup_handler();
        // tmr_eventpause(halScreen.SAreaTimer);
        // tmr_eventresume(halScreen.SAreaTimer);      // PAUSE/RESUME WILL RESTART THE 3 SECOND COUNT
        // return;
    }
    halScreen.SAreaTimer = tmr_eventcreate(&status_popup_handler, 3000, 0);
}

// DECOMPILE THE OPCODE NAME IF POSSIBLE
const WORD const text_editor_string[] = { MAKESTRING(12),
                                          TEXT2WORD('C', 'o', 'm', 'm'),
                                          TEXT2WORD('a', 'n', 'd', ' '),
                                          TEXT2WORD('L', 'i', 'n', 'e') };

word_p          halGetCommandName(word_p NameObject)
{
    WORD Opcode = NameObject ? *NameObject : 0;

    if (Opcode == 0)
        return (word_p) text_editor_string;
    if (ISSYMBOLIC(Opcode))
    {
        word_p OpObject = rplSymbMainOperatorPTR(NameObject);
        if (OpObject)
        {
            Opcode     = *OpObject;
            NameObject = OpObject;
        }
    }

    if (ISPROLOG(Opcode))
    {
        // ONLY ACCEPT IDENTS AND STRINGS AS COMMAND NAMES
        if (ISSTRING(Opcode))
            return NameObject;
        if (!ISIDENT(Opcode))
            return 0;
    }

    int32_t SavedException = Exceptions;
    int32_t SavedErrorCode = ErrorCode;

    Exceptions          = 0; // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
    word_p opname      = rplDecompile(NameObject, DECOMP_NOHINTS);
    Exceptions          = SavedException;
    ErrorCode           = SavedErrorCode;

    return opname;
}

// DISPLAY AN ERROR BOX FOR 5 SECONDS WITH AN ERROR MESSAGE
// USES ERROR CODE FROM SYSTEM Exceptions
void halShowErrorMsg()
{
    int errbit;
    if (!Exceptions)
        return;

    halErrorPopup();

    gglsurface scr;
    ggl_init_screen(&scr);

    if (!halScreen.Menu2)
    {
        // SHOW THE SECOND MENU TO DISPLAY THE MESSAGE
        halSetMenu2Height(MENU2_HEIGHT);
        halRedrawAll(&scr);
    }

    int32_t ytop = halScreen.Form + halScreen.Stack + halScreen.CmdLine + 1;
    int32_t ybot = ytop + halScreen.Menu1 + halScreen.Menu2 - 1;

    // CLEAR MENU2 AND STATUS AREA
    ggl_cliprect(&scr, 0, ytop, LCD_W - 1, ybot, PAL_HLP_BG);
    // DO SOME DECORATIVE ELEMENTS
    ggl_cliphline(&scr, ytop + FONT_HEIGHT(FONT_HLPTITLE) + 1, 0, LCD_W - 1, PAL_HLP_LINES);
    // ggl_cliphline(&scr,ytop+halScreen.Menu2-1,0,LCD_W-1,8);
    ggl_cliprect(&scr, 0, ytop, 4, ybot, PAL_HLP_LINES);

    scr.left  = 1;
    scr.right = LCD_W - 2;
    scr.top  = ytop;
    scr.bottom = ybot - 1;
    // SHOW ERROR MESSAGE

    if (Exceptions != EX_ERRORCODE)
    {
        int32_t xstart = scr.left + 6;
        if (ExceptionPointer != 0) // ONLY IF THERE'S A VALID COMMAND TO BLAME
        {
            word_p cmdname = halGetCommandName(ExceptionPointer);
            if (cmdname)
            {
                utf8_p start = (utf8_p) (cmdname + 1);
                utf8_p end   = start + rplStrSize(cmdname);

                xstart += StringWidthN(start, end, FONT_HLPTITLE);
                DrawTextN(&scr, scr.left + 6,
                          scr.top + 1,
                          start,
                          end,
                          FONT_HLPTITLE,
                          PAL_HLP_TEXT);
                xstart += 4;
            }
        }
        DrawText(&scr, xstart, scr.top + 1, "Exception:", FONT_HLPTITLE, PAL_HLP_TEXT);

        int32_t ecode;
        for (errbit = 0; errbit < 8; ++errbit) // THERE'S ONLY A FEW EXCEPTIONS IN THE NEW ERROR MODEL
        {
            if (Exceptions & (1 << errbit))
            {
                ecode           = MAKEMSG(0, errbit);
                word_p message = uiGetLibMsg(ecode);
                if (!message)
                    message = uiGetLibMsg(ERR_UNKNOWNEXCEPTION);
                if (message)
                {
                    utf8_p msgstart = (utf8_p) (message + 1);
                    utf8_p msgend   = msgstart + rplStrSize(message);

                    DrawTextN(&scr, scr.left + 6,
                              scr.top + 3 + FONT_HEIGHT(FONT_HLPTEXT),
                              msgstart,
                              msgend,
                              FONT_HLPTEXT,
                              PAL_HLP_TEXT);
                }
                break;
            }
        }
    }
    else
    {
        // TRY TO DECOMPILE THE OPCODE THAT CAUSED THE ERROR
        int32_t xstart = scr.left + 6;
        if (ExceptionPointer != 0) // ONLY IF THERE'S A VALID COMMAND TO BLAME
        {
            word_p cmdname = halGetCommandName(ExceptionPointer);
            if (cmdname)
            {
                utf8_p start = (utf8_p) (cmdname + 1);
                utf8_p end   = start + rplStrSize(cmdname);

                xstart += StringWidthN(start, end, FONT_HLPTITLE);
                DrawTextN(&scr, scr.left + 6,
                          scr.top + 1,
                          start,
                          end,
                          FONT_HLPTITLE,
                          PAL_HLP_TEXT);
                xstart += 4;
            }
        }
        DrawText(&scr, xstart, scr.top + 1, "Error:", FONT_HLPTITLE, PAL_HLP_TEXT);
        // GET NEW TRANSLATABLE MESSAGES

        word_p message = uiGetLibMsg(ErrorCode);
        if (!message)
            message = uiGetLibMsg(ERR_UNKNOWNEXCEPTION);
        if (message)
        {
            utf8_p msgstart = (utf8_p) (message + 1);
            utf8_p msgend   = msgstart + rplStrSize(message);

            DrawTextN(&scr, scr.left + 6,
                      scr.top + 3 + FONT_HEIGHT(FONT_HLPTITLE),
                      msgstart,
                      msgend,
                      FONT_HLPTEXT,
                      PAL_HLP_TEXT);
        }
    }
}

void halShowMsgN(utf8_p Text, utf8_p End)
{
    halErrorPopup();

    gglsurface scr;
    ggl_init_screen(&scr);

    if (!halScreen.Menu2)
    {
        // SHOW THE SECOND MENU TO DISPLAY THE MESSAGE
        halSetMenu2Height(MENU2_HEIGHT);
        halRedrawAll(&scr);
    }

    int32_t ytop = halScreen.Form + halScreen.Stack + halScreen.CmdLine + 1;
    int32_t ybot = ytop + halScreen.Menu1 + halScreen.Menu2 - 1;

    // CLEAR MENU2 AND STATUS AREA
    ggl_cliprect(&scr, 0, ytop, LCD_W - 1, ybot, PAL_HLP_BG);
    // DO SOME DECORATIVE ELEMENTS
    ggl_cliphline(&scr, ytop + 1, 1, LCD_W - 2, PAL_HLP_LINES);
    ggl_cliphline(&scr, ybot, 1, LCD_W - 2, PAL_HLP_LINES);
    ggl_clipvline(&scr, 1, ytop + 2, ybot - 1, PAL_HLP_LINES);
    ggl_clipvline(&scr, LCD_W - 2, ytop + 2, ybot - 1, PAL_HLP_LINES);

    // SHOW MESSAGE

    DrawTextN(&scr, 3, ytop + 3, Text, End, FONT_HLPTEXT, PAL_HLP_TEXT);
}

void halShowMsg(utf8_p Text)
{
    utf8_p End = StringEnd(Text);
    halShowMsgN(Text, End);
}

// CHANGE THE CONTEXT AND DISPLAY THE CURRENT FORM
void halSwitch2Form()
{
    if (halGetContext() & CONTEXT_INEDITOR)
    {
        // CLOSE THE EDITOR FIRST
        uiCloseCmdLine();
        halSetCmdLineHeight(0);
    }

    halSetContext(CONTEXT_FORM);

    // ENLARGE THE FORM
    halSetFormHeight(halScreen.Stack);
    // AND ELIMINATE THE STACK
    halSetStackHeight(0);

    // uiFormEnterEvent();

    halScreen.DirtyFlag |= STACK_DIRTY | FORM_DIRTY | STAREA_DIRTY | MENU1_DIRTY | MENU2_DIRTY;
}

// CHANGE THE CONTEXT AND DISPLAY THE STACK
void halSwitch2Stack()
{
    if (halGetContext() & CONTEXT_INEDITOR)
    {
        // CLOSE THE EDITOR FIRST
        uiCloseCmdLine();
        halSetCmdLineHeight(0);
    }

    halSetContext(CONTEXT_STACK);

    // ENLARGE THE STACK
    halSetStackHeight(halScreen.Form);
    // AND ELIMINATE THE FORM
    halSetFormHeight(0);

    // uiFormExitEvent();

    uiClearRenderCache();

    halScreen.DirtyFlag |= STACK_DIRTY | FORM_DIRTY | STAREA_DIRTY | MENU1_DIRTY | MENU2_DIRTY;
}
