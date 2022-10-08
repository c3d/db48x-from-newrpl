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
RECORDER(layout, 16, "Layout and redrawing");



// ============================================================================
//
//   Layout management
//
// ============================================================================

typedef struct rect
// ----------------------------------------------------------------------------
//   Rectangle with the coordinates of a given surface
// ----------------------------------------------------------------------------
{
    coord       left, right;
    coord       top, bottom;
} rect_t;


typedef struct layout layout_t;
typedef const layout_t *layout_p;
typedef void (*redraw_fn)(gglsurface *s, layout_p layout, rect_t *rect);


typedef enum anchor
// ----------------------------------------------------------------------------
//   Position relative to the reference item
// ----------------------------------------------------------------------------
//   The order in this enum matters, see the layout() function
//   Bits are used as follows:                         m i      s      n j
//    0: anchor right vs. left                          +---------------+
//    1: anchor bottom vs. top                         e|a      q      f|b
//    2: to-left vs. to-right horizontally              |               |
//    3: above vs. below vertically                    w|u      z      x|v
//    4: Center horizontally                            |               |
//    5: Center vertically                             o|k      t      p|l
//                                                      +---------------+
//                                                     g c      r      h d
//
{
    TOP_LEFT_IN,                                        // a
    TOP_RIGHT_OF,                                       // b
    BELOW_BOTTOM_LEFT,                                  // c
    BELOW_RIGHT_OF,                                     // d
    TOP_LEFT_OF,                                        // e
    TOP_RIGHT_IN,                                       // f
    BELOW_LEFT_OF,                                      // g
    BELOW_RIGHT,                                        // h

    ABOVE_LEFT,                                         // i
    ABOVE_RIGHT_OF,                                     // j
    BOTTOM_LEFT_IN,                                     // k
    BOTTOM_RIGHT_OF,                                    // l
    ABOVE_LEFT_OF,                                      // m
    ABOVE_RIGHT,                                        // n
    BOTTOM_LEFT_OF,                                     // o
    BOTTOM_RIGHT_IN,                                    // p

    TOP_CENTER_IN       = (1<<4),                       // q
    BELOW_CENTER_OF     = (1<<4) | (1<<1),              // r
    ABOVE_CENTER_OF     = (1<<4) |          (1<<3),     // s
    BOTTOM_CENTER_IN    = (1<<4) | (1<<1) | (1<<3),     // t

    CENTER_LEFT_IN      = (1<<5),                       // u
    CENTER_RIGHT_OF     = (1<<5) | (1<<0),              // v
    CENTER_LEFT_OF      = (1<<5) |          (1<<2),     // w
    CENTER_RIGHT_IN     = (1<<5) | (1<<0) | (1<<2),     // x

    CENTER_IN           = (1<<4) | (1<<5),              // z

    CLIPPING            = (1<<6),                       // Keep clipping rect

} anchor_t;


typedef struct layout
// ----------------------------------------------------------------------------
//   An enumeration for the items we can put on the screen
// ----------------------------------------------------------------------------
{
    redraw_fn   draw;           // Draw the given layout
    anchor_t    position;       // Relative location to other layout
    redraw_fn   base;           // Reference layout
    coord       dx;             // Horizontal offset for the layout
    coord       dy;             // Vertical offset for the layout
} layout_t;


static void layout_clip(gglsurface *s,
                        layout_p    layout,
                        rect_t     *rect,
                        coord       width,
                        coord       height)
// ----------------------------------------------------------------------------
//   Apply a layout, return old clipping rectangle
// ----------------------------------------------------------------------------
//   On input, the rectangle is set to the reference
//   On output, we resize that rectangle to at most width and height
{
    anchor_t pos    = layout->position;
    coord    dx     = layout->dx;
    coord    dy     = layout->dy;
    coord    left   = rect->left;
    coord    top    = rect->top;
    coord    right  = rect->right;
    coord    bottom = rect->bottom;

    // Case where we request a 0,0 size with an offset of 1
    if (width < dx)
        dx = 0;
    if (height < dy)
        dy = 0;

    if (pos & (1 << 4))
    {
        coord x = (left + right) / 2;
        if (width >= right - left)
            width = right - left - 1;
        width -= 2 * dx;
        left  = x - width / 2;
        right = left + width - 1;
    }
    else
    {
        coord x = pos & (1 << 0) ? right : left;
        if (pos & (1 << 2))
        {
            x -= dx;
            left = x - width + 1;
            right = x;
        }
        else
        {
            x += dx;
            left = x;
            right = x + width - 1;
        }
    }
    if (pos & (1 << 5))
    {
        coord y = (top + bottom) / 2;
        if (height >= bottom - top)
            height = bottom - top - 1;
        height -= 2 * dy;
        top = y - height / 2;
        bottom = top + height - 1;
    }
    else
    {
        coord y = pos & (1 << 1) ? bottom : top;
        if (pos & (1 << 3))
        {
            y -= dy;
            top = y - height + 1;
            bottom = y;
        }
        else
        {
            y += dy;
            top = y;
            bottom = y + height - 1;
        }
    }

    // Clip resulting rectangle
    if (left < s->left)
        left = s->left;
    if (top < s->top)
        top = s->top;
    if (right > s->right)
        right = s->right;
    if (bottom > s->bottom)
        bottom = s->bottom;

    // Store the resulting coordinates
    rect->left   = left;
    rect->top    = top;
    rect->right  = right;
    rect->bottom = bottom;
    s->left      = left;
    s->top       = top;
    s->right     = right;
    s->bottom    = bottom;
}


// Actual rendering of the various panels
static void screen_layout       (gglsurface *, layout_p , rect_t *);
static void spacer_layout       (gglsurface *, layout_p , rect_t *);
static void filler_layout       (gglsurface *, layout_p , rect_t *);
static void stack_layout        (gglsurface *, layout_p , rect_t *);
static void cmdline_layout      (gglsurface *, layout_p , rect_t *);
static void status_area_layout  (gglsurface *, layout_p , rect_t *);
static void angle_mode_layout   (gglsurface *, layout_p , rect_t *);
static void complex_flag_layout (gglsurface *, layout_p , rect_t *);
static void halted_flag_layout  (gglsurface *, layout_p , rect_t *);
static void alarm_flag_layout   (gglsurface *, layout_p , rect_t *);
static void receive_flag_layout (gglsurface *, layout_p , rect_t *);
static void sdcard_layout       (gglsurface *, layout_p , rect_t *);
static void lshift_layout       (gglsurface *, layout_p , rect_t *);
static void rshift_layout       (gglsurface *, layout_p , rect_t *);
static void alpha_layout        (gglsurface *, layout_p , rect_t *);
static void busy_flag_layout    (gglsurface *, layout_p, rect_t *);
static void user_flags_layout   (gglsurface *, layout_p , rect_t *);
static void message_layout      (gglsurface *, layout_p , rect_t *);
static void autocomplete_layout (gglsurface *, layout_p , rect_t *);
static void path_layout         (gglsurface *, layout_p , rect_t *);
static void battery_layout      (gglsurface *, layout_p , rect_t *);
static void form_layout         (gglsurface *, layout_p , rect_t *);
static void help_layout         (gglsurface *, layout_p , rect_t *);
static void errors_layout       (gglsurface *, layout_p , rect_t *);
static void menu1_1_layout      (gglsurface *, layout_p , rect_t *);
static void menu1_2_layout      (gglsurface *, layout_p , rect_t *);
static void menu1_3_layout      (gglsurface *, layout_p , rect_t *);
static void menu1_4_layout      (gglsurface *, layout_p , rect_t *);
static void menu1_5_layout      (gglsurface *, layout_p , rect_t *);
static void menu1_6_layout      (gglsurface *, layout_p , rect_t *);
static void menu2_1_layout      (gglsurface *, layout_p , rect_t *);
static void menu2_2_layout      (gglsurface *, layout_p , rect_t *);
static void menu2_3_layout      (gglsurface *, layout_p , rect_t *);
static void menu2_4_layout      (gglsurface *, layout_p , rect_t *);
static void menu2_5_layout      (gglsurface *, layout_p , rect_t *);
static void menu2_6_layout      (gglsurface *, layout_p , rect_t *);


static void render_layouts(gglsurface *scr)
// ----------------------------------------------------------------------------
//   Render all the layouts in order
// ----------------------------------------------------------------------------
{
    // Per-target layout definition
#include <layout.h>

    int    NUM_LAYOUTS = sizeof(layouts) / sizeof(layouts[0]);
    rect_t rects[NUM_LAYOUTS];

    coord     left   = 0;
    coord     right  = LCD_W-1;
    coord     top    = 0;
    coord     bottom = LCD_H-1;

    for (int l = 0; l < NUM_LAYOUTS; l++)
    {
        const layout_p layout = layouts + l;
        rect_t *r = rects + l;

        // Initialize rectangle, notably useful for inii
        r->left   = left;
        r->right  = right;
        r->top    = top;
        r->bottom = bottom;

        // Search the layout reference, and if so, update rectangle
        redraw_fn base = layout->base;
        if (base)
        {
            // Find the reference layout referenced by this layout
            for (int ref = l - 1; ref >= 0; ref--)
            {
                if (layouts[ref].draw == base)
                {
                    *r = rects[ref];
                    break;
                }
            }
        }

        // Call the renderer with the rectangle that we found
        redraw_fn draw   = layout->draw;
        draw(scr, layout, r);

        // Restore clipping after draw
        if (layout->position & CLIPPING)
        {
            left   = scr->left;
            right  = scr->right;
            top    = scr->top;
            bottom = scr->bottom;
        }
        scr->left   = left;
        scr->top    = top;
        scr->right  = right;
        scr->bottom = bottom;
    }
}



// ============================================================================
//
//   Redraw functions
//
// ============================================================================

static void screen_layout(gglsurface *s, layout_p l, rect_t *r)
// ----------------------------------------------------------------------------
//   Size the screen for other components to layout inside
// ----------------------------------------------------------------------------
{
    record(layout, "Redrawing screen %p, layout %p, rectangle %p", s, l, r);
    if (halDirty(BACKGROUND_DIRTY))
    {
        ggl_rect(s, 0, 0, LCD_W - 1, LCD_H - 1, PAL_GRAY8);
        halRepainted(BACKGROUND_DIRTY);
    }
    s->left   = 0;
    s->right  = LCD_W - 1;
    s->top    = 0;
    s->bottom = LCD_H - 1;
}


static void spacer_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   A filler makes it possible to adjust between other objects
// ----------------------------------------------------------------------------
{
    layout_clip(scr, layout, rect, LCD_W, LCD_H);
}


static void filler_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   A filler makes it possible to adjust between other objects
// ----------------------------------------------------------------------------
{
    layout_clip(scr, layout, rect, LCD_W, LCD_H);
    coord     left       = rect->left;
    coord     top        = rect->top;
    coord     right      = rect->right;
    coord     bottom     = rect->bottom;
    pattern_t background = PAL_STA_BG;
    ggl_cliprect(scr, left, top, right, bottom, background);
}


static void menu_item_layout(gglsurface *scr,
                             layout_p    layout,
                             rect_t     *rect,
                             int         menu,
                             int         index)
// ----------------------------------------------------------------------------
//   Redraw a menu at the given index
// ----------------------------------------------------------------------------
{
    UNUSED(filler_layout);
    UNUSED(spacer_layout);

    // Check active menu
    int active = menu == 2 ? halScreen.Menu2 : halScreen.Menu1;

    // Compute the size for the layout
    size width  = active ? MENU_TAB_WIDTH  : 0;
    size height = active ? MENU_TAB_HEIGHT : 0;
    layout_clip(scr, layout, rect, width, height);
    if (!active)
        return;

    int       invertMenu = rplTestSystemFlag(FL_MENU1WHITE);
    pattern_t bcolor     = invertMenu ? PAL_MENU_TEXT : PAL_MENU_BG;
    pattern_t background = PAL_STK_BG;

    // Draw the button
    coord     left       = rect->left;
    coord     top        = rect->top;
    coord     right      = rect->right;
    coord     bottom     = rect->bottom;

    ggl_cliprect(scr, left, top, right, bottom, background);
    ggl_cliprect(scr, left + 1, top + 0, right - 1, bottom - 0, bcolor);
    ggl_cliprect(scr, left + 0, top + 1, right - 0, bottom - 1, bcolor);

    int64_t  menuCode = rplGetMenuCode(menu);
    word_p   menuObj  = uiGetLibMenu(menuCode);
    unsigned nitems   = uiCountMenuItems(menuCode, menuObj);

    // Validity check: Commands may have rendered the page number invalid,
    // for example by purging variables
    if (MENU_PAGE(menuCode) >= nitems || nitems <= 6)
    {
        menuCode = SET_MENU_PAGE(menuCode, 0);
        rplSetMenuCode(menu, menuCode);
    }

    int64_t menuId = MENU_PAGE(menuCode) + index - 1;
    word_p item = uiGetMenuItem(menuCode, menuObj, menuId);

    // Inset before drawing text
    scr->top       = top + MENU_TAB_INSET;
    scr->bottom    = bottom - MENU_TAB_INSET;
    uiDrawMenuItem(scr, item);
}


#define DRAW_MENU_BUTTON(m, id)                                         \
static void menu##m##_##id##_layout(gglsurface *scr,                    \
                                    layout_p    layout,                 \
                                    rect_t     *rect)                   \
/* ---------------------------------------------------------------- */  \
/*   Generate a menu button                                         */  \
/* ---------------------------------------------------------------- */  \
{                                                                       \
    menu_item_layout(scr, layout, rect, m, id);                         \
}

DRAW_MENU_BUTTON(1, 1);
DRAW_MENU_BUTTON(1, 2);
DRAW_MENU_BUTTON(1, 3);
DRAW_MENU_BUTTON(1, 4);
DRAW_MENU_BUTTON(1, 5);
DRAW_MENU_BUTTON(1, 6);
DRAW_MENU_BUTTON(2, 1);
DRAW_MENU_BUTTON(2, 2);
DRAW_MENU_BUTTON(2, 3);
DRAW_MENU_BUTTON(2, 4);
DRAW_MENU_BUTTON(2, 5);
DRAW_MENU_BUTTON(2, 6);


void halSetNotification(enum halNotification type, unsigned color)
// ----------------------------------------------------------------------------
//  Show/hide the notification icon
// ----------------------------------------------------------------------------
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

    // Draw custom icons into the status area for all other annunciators
    if ((halFlags ^ old) & (1 << (16 + type)))
        halRefresh(MENU_DIRTY | STATUS_DIRTY | STACK_DIRTY);
}


unsigned halGetNotification(enum halNotification type)
// ----------------------------------------------------------------------------
//   Return a given notification
// ----------------------------------------------------------------------------
{
    if (halFlags & (1 << (16 + type)))
        return 1;
    return 0;
}


void halSetStackHeight(int h)
// ----------------------------------------------------------------------------
//   Adjust the height of the stack
// ----------------------------------------------------------------------------
{
    halScreen.Stack = h > 0 ? h : 0;
    halRefresh(STACK_DIRTY);
}


void halSetFormHeight(int h)
// ----------------------------------------------------------------------------
//   Adjust the height of a form
// ----------------------------------------------------------------------------
{
    halScreen.Form = h > 0 ? h : 0;
    halRefresh(FORM_DIRTY);
}


void halSetMenu1Height(int h)
// ----------------------------------------------------------------------------
//   Set the height of menu 1, which we use to identify if menu is active
// ----------------------------------------------------------------------------
{
    halScreen.Menu1 = h > 0 ? h : 0;
    halRefresh(MENU1_DIRTY);
}


void halSetMenu2Height(int h)
// ----------------------------------------------------------------------------
//  Set the height of the second menu
// ----------------------------------------------------------------------------
{
    halScreen.Menu2 = h > 0 ? h : 0;
    halRefresh(MENU2_DIRTY);
}


void halSetCmdLineHeight(int h)
// ----------------------------------------------------------------------------
//   Set the height of the command line
// ----------------------------------------------------------------------------
{
    halScreen.CmdLine = h > 0 ? h : 0;
    halRefresh(CMDLINE_ALL_DIRTY);
}


int32_t halGetDispObjectHeight(word_p object, UNIFONT *font)
// ----------------------------------------------------------------------------
// Compute height and width of object to display on stack
// ----------------------------------------------------------------------------
{
    UNUSED(object);

    // TODO: Add multiline objects, etc.
    return font->BitmapHeight;
}


static void form_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Redraw a form
// ----------------------------------------------------------------------------
{
    int active = halScreen.Form != 0;

    // Compute the size for the layout
    size width  = active ? LCD_W : 0;
    size height = active ? LCD_H : 0;
    layout_clip(scr, layout, rect, width, height);
    if (!active)
        return;

    // Redraw the contents of the current form
    coord     left       = rect->left;
    coord     top        = rect->top;
    coord     right      = rect->right;
    coord     bottom     = rect->bottom;
    ggl_cliprect(scr, left, top, right, bottom, PAL_FORM_BG);

    // Check the current form
    word_p form = rplGetSettings((word_p) currentform_ident);
    if (form)
    {
        word_p bmp = uiFindCacheEntry(form, FONT_FORMS);
        if (bmp)
        {
            gglsurface viewport = ggl_grob(bmp);
            ggl_copy(scr, &viewport, LCD_W, LCD_H);
        }
    }
    halRepainted(FORM_DIRTY);
}


static void stack_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Redraw the stack
// ----------------------------------------------------------------------------
{
    int active = halScreen.Stack != 0;

    // Compute the size for the layout
    size twidth  = active ? LCD_W : 0;
    size theight = active ? LCD_H : 0;
    layout_clip(scr, layout, rect, twidth, theight);
    if (!active)
        return;

    // Redraw the contents of the stack
    coord left   = rect->left;
    coord top    = rect->top;
    coord right  = rect->right;
    coord bottom = rect->bottom;
    coord height = bottom - top;
    coord width  = right - left;
    int   depth  = rplDepthData();
    int   level  = 1;

    ggl_cliprect(scr, left, top, right, bottom, PAL_STK_BG);

    const UNIFONT *hdrfnt = FONT(STACK_INDEX);
    size  stknum_w = StringWidth("0", hdrfnt);
    coord xright   = stknum_w;
    if (halContext(CONTEXT_INTERACTIVE_STACK))
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

            const UNIFONT *levelfnt = k == 1 ? FONT_STACK_LEVEL1 : FONT_STACK;
            word_p         object   = uiRenderObject(rplPeekData(k), levelfnt);

            // Get the size of the object
            size objh = object ? object[2] : levelfnt->BitmapHeight;
            coord ypref = top + height / 4 + objh / 2;
            if (ypref > bottom)
                ypref = bottom - objh;
            if (ypref < top)
                ypref = top;

            for (; k > 0; --k)
            {
                levelfnt = k == 1 ? FONT_STACK_LEVEL1 : FONT_STACK;
                object   = uiRenderObject(rplPeekData(k), levelfnt);

                // Get the size of the object
                int stkheight = object ? object[2] : levelfnt->BitmapHeight;
                if (ypref + stkheight > bottom)
                {
                    coord y                    = ypref + stkheight;
                    halScreen.StkVisibleLvl    = k;
                    halScreen.StkVisibleOffset = bottom - y;
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

    level   = halScreen.StkVisibleLvl;
    coord y = bottom - halScreen.StkVisibleOffset;

    for (int mul = 10; depth >= mul; mul *= 10)
        xright += stknum_w;

    // Clear the stack index area, the stack display area, and draw a sep line
    ggl_cliprect(scr, 0, top, xright - 1, bottom - 1, PAL_STK_IDX_BG);
    ggl_cliprect(scr, xright + 1, top, LCD_W - 1, bottom - 1, PAL_STK_BG);
    ggl_clipvline(scr, xright, top, bottom - 1, PAL_STK_VLINE);

    pattern_t      selbg  = PAL_STK_SEL_BG;
    pattern_t      cursor = PAL_STK_CURSOR;

    while (y > top)
    {
        const UNIFONT *levelfnt  = level == 1 ? FONT_STACK_LEVEL1 : FONT_STACK;
        size           objheight = 0;
        word_p         object    = NULL;

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

        size  hdrh  = hdrfnt->BitmapHeight;
        size  stkh  = levelfnt->BitmapHeight;
        coord ytop  = y - objheight;
        coord yhtop = ytop + (hdrh < stkh ? (stkh - hdrh) / 2 : 0);

        if (halContext(CONTEXT_INTERACTIVE_STACK))
        {
            int selStart   = halScreen.StkSelStart;
            int selEnd     = halScreen.StkSelEnd;
            int stkPointer = halScreen.StkPointer;

            // Highlight selected items
            switch (halScreen.StkSelStatus)
            {
            default:
            case 0:
                // Nothing selected yet
                break;
            case 1:
                // Start was selected:
                // Paint all levels between start and current position
                if (selStart > stkPointer)
                {
                    if (level >= stkPointer && level <= selStart)
                        ggl_cliprect(scr, left, ytop, xright - 1, y - 1, selbg);
                    if (level == selStart)
                        DrawText(scr, left + 2, yhtop, "▶", hdrfnt, cursor);
                }
                else
                {
                    if (level >= selStart && level <= stkPointer)
                        ggl_cliprect(scr, left, ytop, xright - 1, y - 1, selbg);
                    if (level == selStart)
                        DrawText(scr, left + 2, yhtop, "▶", hdrfnt, cursor);
                }
                break;
            case 2:
                // Both start and end selected
                if (level >= selStart && level <= selEnd)
                    ggl_cliprect(scr, left, ytop, xright - 1, y - 1, selbg);
                if (level == selStart)
                    DrawText(scr, left + 2, yhtop, "▶", hdrfnt, cursor);
                if (level == selEnd)
                    DrawText(scr, left + 2, yhtop, "▶", hdrfnt, cursor);
                break;
            }

            // Draw the pointer
            if (level <= depth && level == stkPointer)
                DrawText(scr, left, yhtop, "▶", hdrfnt, cursor);
            else if (level == 1 && stkPointer == 0)
                DrawText(scr, left, yhtop + stkh/2, "▶", hdrfnt, cursor);
            else if (level == depth && stkPointer > depth)
                DrawText(scr, left, yhtop - stkh/2 + 1, "▶", hdrfnt, cursor);
        }

        if (level <= depth)
        {
            // Draw the stack level number
            char  num[16];
            snprintf(num, sizeof(num), "%d", level);
            size numwidth = StringWidth(num, hdrfnt);
            DrawText(scr, xright - numwidth, yhtop, num, hdrfnt, PAL_STK_INDEX);

            // Do proper layout: right justify unless it does not fit
            coord x = right - width;
            if (x < xright + 1)
                x = xright + 1;

            // Display the item
            coord oldLeft = scr->left;
            scr->left = xright + 1;
            uiDrawBitmap(scr, x, ytop, object);
            scr->left = oldLeft;
        }

        y = ytop;
        ++level;
    }
    halRepainted(STACK_DIRTY);
}


#define MABS(a) (((a) < 0) ? -(a) : (a))


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
#define THEME_GRAY7               140, 140, 140
#define THEME_GRAY8               128, 128, 128
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
    { THEME_MENU_TEXT },
    { THEME_MENU_DIR_MARK },
    { THEME_MENU_DIR },
    { THEME_MENU_DIR_BG },
    { THEME_MENU_HLINE },
    { THEME_MENU_FOCUS_HLINE },
    { THEME_MENU_PRESS_BG },
    { THEME_MENU_FLAG_ON },
    { THEME_MENU_FLAG_OFF },

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
    { THEME_HLP_TITLE },
    { THEME_HLP_BOLD },
    { THEME_HLP_ITALIC },
    { THEME_HLP_CODE },
    { THEME_HLP_CODE_BG },

    // Forms
    { THEME_FORM_BG },
    { THEME_FORM_TEXT },
    { THEME_FORM_SELTEXT },
    { THEME_FORM_SEL_BG },
    { THEME_FORM_CURSOR },

    // Errors
    { THEME_ERROR },
    { THEME_ERROR_BG },
    { THEME_ERROR_LINE },
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


void halUpdateFontArray(const UNIFONT *fontarray[])
// ----------------------------------------------------------------------------
//   Update an array with all the fonts pointers
// ----------------------------------------------------------------------------
{
    // Set all pointers to 0
    for (int i = 0; i < FONTS_NUM; ++i)
        fontarray[i] = NULL;

    // Set configured fonts
    if (!ISDIR(*SettingsDir))
        return;

    const word_p *romtable = rplGetFontRomPtrTableAddress();
    word_p       *var      = rplFindFirstByHandle(SettingsDir);
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

    for (int index = 0; index < FONTS_NUM; index++)
        if (fontarray[index] == 0)
            fontarray[index] = defaultFont[index];

    return;
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

    halFlags                     = 0;
    halProcesses[0]              = 0;
    halProcesses[1]              = 0;
    halProcesses[2]              = 0;
    halScreen.HelpMessage        = NULL;
    halScreen.ShortHelpMessage   = NULL;
    halScreen.ErrorMessage       = NULL;
    halScreen.ErrorMessageLength = 0;
    halScreen.CmdLine            = 0;
    halScreen.Menu1              = MENU1_HEIGHT;
    halScreen.Menu2              = MENU2_HEIGHT;
    halScreen.Stack              = 1;
    halScreen.DirtyFlag          = ALL_DIRTY;
    halScreen.CursorTimer        = -1;
    halScreen.KeyContext         = CONTEXT_STACK;
    halSetNotification(N_LEFT_SHIFT, 0);
    halSetNotification(N_RIGHT_SHIFT, 0);
    halSetNotification(N_ALPHA, 0);
    halSetNotification(N_LOWBATTERY, 0);
    halSetNotification(N_HOURGLASS, 0);
    halSetNotification(N_DATARECVD, 0);
    halSetFormHeight(0);

    // NOT NECESSARILY PART OF HALSCREEN, BUT INITIALIZE THE COMMAND LINE
    uiCloseCmdLine();
    halScreen.StkUndolevels    = 8;
    halScreen.StkCurrentLevel  = 0;

    halScreen.StkVisibleLvl    = 1;
    halScreen.StkVisibleOffset = 0;
}


static void help_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Redraw contextual help on the whole screen
// ----------------------------------------------------------------------------
{
    // Compute the size for the layout
    utf8_p text   = halScreen.HelpMessage;
    size   width  = text ? LCD_W : 0;
    size   height = text ? LCD_H : 0;
    layout_clip(scr, layout, rect, width, height);
    if (!text)
        return;

    coord ytop   = rect->top;
    coord ybot   = rect->bottom;
    coord xleft  = rect->left;
    coord xright = rect->right;

    enum style {
        TITLE,
        NORMAL,
        BULLETS,
        BOLD,
        ITALIC,
        CODE,
        NUM_STYLES
    } style = TITLE;

    struct
    {
        const UNIFONT *font;
        pattern_t      color;
        pattern_t      background;
    } styles[NUM_STYLES] = {
        { FONT_HELP_TITLE,  PAL_HELP_TITLE,      PAL_HELP_BG},
        {  FONT_HELP_TEXT,   PAL_HELP_TEXT,      PAL_HELP_BG},
        {  FONT_HELP_BOLD,   PAL_HELP_BOLD, PAL_HELP_CODE_BG},
        {  FONT_HELP_BOLD,   PAL_HELP_BOLD,      PAL_HELP_BG},
        {FONT_HELP_ITALIC, PAL_HELP_ITALIC,      PAL_HELP_BG},
        {  FONT_HELP_CODE,   PAL_HELP_CODE, PAL_HELP_CODE_BG},
    };

    // Clear help area and add some decorative elements
    ggl_cliprect(scr, xleft, ytop, xright, ybot, PAL_HELP_LINES);
    xleft += 1;
    xright -= 1;
    ytop += 1;
    ybot -= 1;
    ggl_cliprect(scr, xleft, ytop, xright, ybot, PAL_HELP_BG);

    const UNIFONT *font  = styles[style].font;
    // Center the header
    utf8_p hend = text;
    while (*hend && *hend != '\n')
        hend++;
    coord x     = (LCD_W - StringWidthN(text, hend, font)) / 2;
    coord y     = ytop + 2;
    int   hadcr = 1;

    // Display until end of help or next markdown section title
    while(*text && (text[0] != '\n' || text[1] != '#') && y < ybot)
    {
        utf8_p         end   = text;
        int            reset = 0;

        // Find end of word
        while (*end && *end != ' ' && *end != '\n')
            end++;

        // Check bold and italic
        utf8_p first = text;
        utf8_p last  = end - 1;

        // Check if we need to append a space in what we draw
        int needspace = *end == ' ';

        if (hadcr && text[0] == '*' && text[1] == ' ')
        {
            const char bullet[] = "•";
            x += 2;
            first = bullet;
            last = bullet + sizeof(bullet) - 2;
            reset = -1;
            text += 2;
            end = text - 1; // Compensate for the +1 from needspace
            needspace = 1;
        }
        else if (last - text > 2)
        {
            if (*text == '*')
            {
                // Bold text, as in: *Hello World*
                first++;
                style = BOLD;
            }
            else if (*text == '_')
            {
                // Italic text, as in: _Hello World_
                first++;
                style = ITALIC;
            }
            else if (*text == '`')
            {
                // Code text, as in: `SIN`
                first++;
                style = CODE;
            }
            if (*last == '*' || *last == '_' || *last == ':' ||
                *last == '`')
            {
                // End marker: switch back to normal text after render
                reset = 1;
                if (*last == ':' )
                    end--;
                last--;
                needspace = 0;
            }
        }

        // Select font and color based on style
        pattern_t      color = styles[style].color;
        pattern_t      bg    = styles[style].background;
        const UNIFONT *font  = styles[style].font;

        // Check if word fits. If not, skip to new line
        coord right = x + StringWidthN(first, last+1, font);
        size  height = font->BitmapHeight;
        if (right >= xright - 1)
        {
            x = xleft + 2;
            y += height;
        }

#if BITS_PER_PIXEL == 1
        // Draw a decoration
        if (style == BULLETS || style == CODE)
        {
            size width = StringWidthN(first, last+1, font);
            ggl_hline(scr, y + height - 1, x, x + width - 1, bg);
            if (style == CODE)
            {
                ggl_hline(scr, y, x, x + width - 1, bg);
                ggl_vline(scr, x, y, y + height - 1, bg);
                ggl_vline(scr, x + width - 1, y, y + height - 1, bg);
            }
        }

        // Draw next word
        x = DrawTextN(scr, x, y, first, last+1, font, color);
#else
        // Draw next word
        x = DrawTextBkN(scr, x, y, first, last+1, font, color, bg);
#endif
        if (needspace)
        {
            // Force a thin space in help text, it looks better
            unsigned sw = style == TITLE ? 4 : 2;
            ggl_cliprect(scr, x, y, x + sw -1, y + height - 1, bg);
            x += sw;
            end++;
        }

        // Check if we need to skip to end of line
        text = end;
        hadcr = *text == '\n';
        if (hadcr)
        {
            x = xleft + 2;
            y += height;
            text++;
            style = NORMAL;
        }
        if (reset)
            style = reset < 0 ? BULLETS : NORMAL;
    }
    halRepainted(HELP_DIRTY);
}


static inline void text_layout(gglsurface    *scr,
                               layout_p       layout,
                               rect_t        *rect,
                               utf8_p         msg,
                               unsigned       len,
                               const UNIFONT *font,
                               pattern_t      color)
// ----------------------------------------------------------------------------
//    Display a text
// ----------------------------------------------------------------------------
{
    // Compute the size for the layout
    size height = msg ? font->BitmapHeight                     : 0;
    size width  = msg ? StringWidthN(msg, msg + len, font) + 3 : 0;
    layout_clip(scr, layout, rect, width, height);
    if (msg)
    {
        // Redraw the given message
        coord x = (rect->left + rect->right - width + 1) / 2;
        coord y = rect->top;
        DrawTextN(scr, x, y, msg, msg + len, font, color);
    }
}


static void status_area_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//  Clear status area
// ----------------------------------------------------------------------------
{
    // Compute the size for the layout
    const UNIFONT *font   = FONT_STATUS;
    coord          width  = LCD_W;
    coord          height = 3 * font->BitmapHeight;
    layout_clip(scr, layout, rect, width, height);

    // Draw the background
    pattern_t background = PAL_STA_BG;
    coord     left       = rect->left;
    coord     top        = rect->top;
    coord     right      = rect->right;
    coord     bottom     = rect->bottom;
    ggl_cliprect(scr, left, top, right, bottom, background);
}


static void message_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Draw the message
// ----------------------------------------------------------------------------
{
    utf8_p         msg   = halScreen.ShortHelpMessage;
    unsigned       len   = msg ? strlen(msg) : 0;
    const UNIFONT *font  = FONT_HELP_CODE;

    // Compute the size for the layout
    size height = msg ? font->BitmapHeight                 : 0;
    size width  = msg ? StringWidthN(msg, msg + len, font) : 0;
    layout_clip(scr, layout, rect, width, height);
    if (msg)
    {
        pattern_t      color = PAL_HELP_BG;
        pattern_t      bg    = PAL_HELP_TEXT;

        // Redraw the given message
        coord x = (rect->left + rect->right - width) / 2;
        coord y = rect->top;
        DrawTextBkN(scr, x, y, msg, msg + len, font, color, bg);
    }
}


static void autocomplete_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Draw the auto-complete
// ----------------------------------------------------------------------------
{
    utf8_p   info = NULL;
    unsigned len  = 0;

    // Autocomplete
    if (halScreen.CmdLineState & CMDSTATE_ACACTIVE)
    {
        if (halScreen.ACSuggestion != 0)
        {
            // Display the currently selected autocomplete command
            if (!Exceptions)
            {
                // Only if there were no errors
                word_p cmdname = rplDecompile(
                    ((IS_PROLOG(halScreen.ACSuggestion) && SuggestedObject)
                         ? SuggestedObject
                         : (&halScreen.ACSuggestion)),
                    DECOMP_NOHINTS);
                if (!cmdname || Exceptions)
                {
                    // Just ignore, clear exceptions and return;
                    Exceptions = 0;
                }
                else
                {
                    info  = (utf8_p) (cmdname + 1);
                    len = rplStrSize(cmdname);
                }
            }
        }
    }

    const UNIFONT *font  = FONT_HELP_CODE;
    pattern_t      color = PAL_STA_TEXT;
    text_layout(scr, layout, rect, info, len, font, color);
    halRepainted(STATUS_DIRTY);
}


static void path_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Show the current path
// ----------------------------------------------------------------------------
{
    layout_clip(scr, layout, rect, LCD_W, LCD_H);

    word_p         pathnames[8];
    unsigned       count  = sizeof(pathnames) / sizeof(pathnames[0]);
    int32_t        nnames = rplGetFullPath(CurrentDir, pathnames, count);
    coord          x      = rect->left;
    coord          y      = rect->top;
    pattern_t      color  = PAL_STA_TEXT;
    pattern_t      bg     = PAL_STA_BG;
    const UNIFONT *font   = FONT_STATUS;

    // Redraw the given paths
    for (int j = nnames - 1; j >= 0; --j)
    {
        if (ISIDENT(*pathnames[j]))
        {
            utf8_p start = (utf8_p) (pathnames[j] + 1);
            word_p last  = rplSkipOb(pathnames[j]) - 1;
            int    hasZ  = (*last & 0xff000000) != 0;
            utf8_p end   = hasZ ? start + strlen(start) : (utf8_p) (last + 1);
            x            = DrawTextBk(scr, x, y, "/", font, color, bg);
            x            = DrawTextBkN(scr, x, y, start, end, font, color, bg);
        }
    }
}


static void angle_mode_layout(gglsurface *s, layout_p l, rect_t *r)
// ----------------------------------------------------------------------------
//   Draw the angle mode indicator
// ----------------------------------------------------------------------------
{
    // Draw angle mode characters
    int               a1      = rplTestSystemFlag(FL_ANGLEMODE1);
    int               a2      = rplTestSystemFlag(FL_ANGLEMODE2);
    int               mode    = a1 | (a2 << 1);
    const char *const name[4] = { "∡°", "∡r", "∡g", "∡d" };
    utf8_p            msg     = name[mode];
    unsigned          len     = strlen(msg);
    text_layout(s, l, r, msg, len, FONT_STATUS, PAL_STA_TEXT);
}


static void flag_layout(gglsurface *scr,
                        layout_p    layout,
                        rect_t     *rect,
                        utf8_p      name,
                        int         flag)
// ----------------------------------------------------------------------------
//   Shared code to display all flags
// ----------------------------------------------------------------------------
{
    pattern_t color = flag ? PAL_STA_UFLAG1 : PAL_STA_UFLAG0;
    text_layout(scr, layout, rect, name, strlen(name), FONT_STATUS, color);
}


static void complex_flag_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Draw the complex flag display
// ----------------------------------------------------------------------------
{
    flag_layout(scr, layout, rect, "C", rplTestSystemFlag(FL_COMPLEXMODE));
}


static void halted_flag_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Draw the halted flag display
// ----------------------------------------------------------------------------
{
    flag_layout(scr, layout, rect, "H", halFlags & HAL_HALTED);
}


static void sdcard_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Display the alarm flag
// ----------------------------------------------------------------------------
{
#ifndef CONFIG_NO_FSYSTEM
    char type[] = { 'S', 'D', '-', '-', 0};

    // SD card inserted indicator
    int inserted = FSCardInserted();
    if (FSIsInit())
    {
        if (FSCardIsSDHC())
        {
            type[0] = 'H';
            type[1] = 'C';
        }
        if (FSVolumeMounted(FSGetCurrentVolume()))
            type[2] = '+';
        int dirty = FSIsDirty();
        if (dirty == 1)
            type[3] = '!';
        else if (dirty == 2)
            type[3] = '>';
    }
    flag_layout(scr, layout, rect, type, inserted);
#endif // CONFIG_NO_FSYSTEM}
}


static void shift_mode_layout(gglsurface *scr,
                              layout_p    layout,
                              rect_t     *rect,
                              utf8_p      name,
                              int         flag)
// ----------------------------------------------------------------------------
//   Shared code to display all shift-mode annunciators
// ----------------------------------------------------------------------------
{
#if !defined(ANN_X_COORD) || !defined(ANN_Y_COORD)
    // This test is for physical annunciators
    pattern_t color = flag == 2 ? PAL_STA_ANNPRESS
                    : flag == 1 ? PAL_STA_ANN
                                : PAL_STA_BG;

    // Compute the size for the layout
    const UNIFONT *font   = Font_Notifications;
    unsigned       length = strlen(name);
    size           height = font->BitmapHeight;
    size           width  = StringWidthN(name, name + length, font);
    layout_clip(scr, layout, rect, width, height);
    if (flag)
    {
        // Redraw the given message
        coord x = (rect->left + rect->right - width) / 2;
        coord y = rect->top;
        DrawTextN(scr, x, y, name, name + length, font, color);
    }
#else
    // Physical annunciators are rendered directly in halSetNotification
    UNUSED(scr);
    UNUSED(layout);
    UNUSED(rect);
    UNUSED(name);
    UNUSED(flag);
#endif // Physical annunciators
}


static void lshift_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Draw the left-shift annunciator
// ----------------------------------------------------------------------------
{
    int status = keyb_flags & KHOLD_LEFT  ? 2
               : keyb_flags & KSHIFT_LEFT ? 1
                                          : 0;
    shift_mode_layout(scr, layout, rect, "L", status);\
}


static void rshift_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Draw the right-shift annunciator
// ----------------------------------------------------------------------------
{
    int status = keyb_flags & KHOLD_RIGHT  ? 2
               : keyb_flags & KSHIFT_RIGHT ? 1
                                           : 0;
    shift_mode_layout(scr, layout, rect, "R", status);\
}


static void alpha_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Draw the alpha mode annunciator
// ----------------------------------------------------------------------------
{
    if (keyb_flags & KFLAG_ALPHA_LOWER)
        text_layout(scr, layout, rect, "a", 1, FONT_STATUS, PAL_STA_ANN);
    int status = keyb_flags & KHOLD_ALPHA  ? 2
               : keyb_flags & KSHIFT_ALPHA ? 1
                                           : 0;
    shift_mode_layout(scr, layout, rect, "A", status);
}


static void busy_flag_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Draw the left-shift annunciator
// ----------------------------------------------------------------------------
{
    int status = halGetNotification(N_HOURGLASS);
    shift_mode_layout(scr, layout, rect, "W", status);\
}


static void alarm_flag_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Display the alarm flag
// ----------------------------------------------------------------------------
{
    int status = halGetNotification(N_ALARM);
    shift_mode_layout(scr, layout, rect, "X", status);
}


static void receive_flag_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Display the alarm flag
// ----------------------------------------------------------------------------
{
    int status = halGetNotification(N_DATARECVD)  ? 2
               : halGetNotification(N_CONNECTION) ? 1
                                                  : 0;
    flag_layout(scr, layout, rect, "U", status);
}


static void user_flags_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//  Show first six user flags
// ----------------------------------------------------------------------------
{
    const UNIFONT *font   = FONT_STATUS;
    coord          width  = StringWidth("012345", font);
    coord          height = font->BitmapHeight;
    layout_clip(scr, layout, rect, width, height);

    pattern_t      off     = PAL_STA_UFLAG0;
    pattern_t      on      = PAL_STA_UFLAG1;
    coord          x       = rect->left;
    coord          y       = rect->top;

    // First 6 user flags
    uint64_t      *flags_p = rplGetUserFlagsLow();
    uint64_t       flags   = flags_p ? *flags_p : 0;
    for (int i = 0; i < 6; i++)
    {
        pattern_t color = ((flags >> i) & 1) ? on : off;
        char buffer[2] = { 0 };
        buffer[0] = '0' + i;
        x = DrawTextN(scr, x, y, buffer, buffer + 1, font, color);
    }
}


static void cmdline_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Layout of the command line
// ----------------------------------------------------------------------------
{
    if (!halScreen.CmdLine)
    {
        layout_clip(scr, layout, rect, 0, 0);
        return;
    }

    // Draw the command line
    size  rowh   = FONT_HEIGHT(FONT_CMDLINE);
    coord height = halScreen.CmdLine;
    coord width  = LCD_W;
    layout_clip(scr, layout, rect, width, height);

    const UNIFONT *font   = FONT_CMDLINE;
    pattern_t      color  = PAL_CMD_TEXT;
    pattern_t      bg     = PAL_CMD_BG;
    pattern_t      selcol = PAL_CMD_SELTEXT;
    pattern_t      selbg  = PAL_CMD_SEL_BG;

    // Decoration
    coord          top    = rect->top;
    coord          left   = rect->left;
    coord          right  = rect->right;
    coord          bottom = rect->bottom;
    ggl_cliphline(scr, top, left, right, PAL_DIV_LINE);
    ggl_cliphline(scr, top + 1, left, right, bg);
    ggl_cliphline(scr, bottom, left, right, PAL_DIV_LINE);

    size   rows    = halScreen.LineCurrent - halScreen.LineVisible;
    coord  stkh    = rows * rowh;
    utf8_p cmdline = (utf8_p) (CmdLineCurrentLine + 1);
    size_t nchars = rplStrSize(CmdLineCurrentLine);

    if (halScreen.DirtyFlag & CMDLINE_DIRTY)
    {
        // Show other lines here except the current edited line
        coord startoff = -1;
        coord endoff   = -1;
        for (int k     = 0; k < halScreen.NumLinesVisible; ++k)
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
                endoff   = startoff < 0
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

            coord x       = -halScreen.XVisible;
            coord y       = top + 2 + k * rowh;
            coord bottom = y + rowh - 1;
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
                    tail = 1;
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
                    selend  = selst = string;

                // Draw the line split in 3 sections:
                // - string to selst
                // - selst to selend
                // - selend to strend
                if (selst > string)
                    x = DrawTextBkN(scr, x, y, string, selst, font, color, bg);
                if (selend > selst)
                    x = DrawTextBkN(scr, x, y, selst, selend, font, selcol, selbg);
                if (strend > selend)
                    x = DrawTextBkN(scr, x, y, selend, strend, font, color, bg);
                if (tail)
                {
                    ggl_cliprect(scr, x, y, x + 3, bottom, bg);
                    x += 3;
                }
            }

            // Clear up to end of line
            ggl_cliprect(scr, x, y, LCD_SCANLINE - 1, bottom, bg);
        }
    }

    if (halScreen.DirtyFlag & CMDLINE_LINE_DIRTY)
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
            selend  = selst = string;

        // Draw the line split in 3 sections:
        // - string to selst,
        // - selst to selend,
        // - selend to strend
        coord x      = -halScreen.XVisible;
        coord y      = top + 2 + stkh;
        coord bottom = y + rowh - 1;
        if (selst > string)
            x = DrawTextBkN(scr, x, y, string, selst, font, color, bg);
        if (selend > selst)
            x = DrawTextBkN(scr, x, y, selst, selend, font, selcol, selbg);
        if (strend > selend)
            x = DrawTextBkN(scr, x, y, selend, strend, font, color, bg);
        if (tail)
        {
            ggl_cliprect(scr, x, y, x + 3, bottom, selbg);
            x += 3;
        }

        // Clear up to end of line
        ggl_cliprect(scr, x, y, LCD_SCANLINE - 1, bottom, bg);
    }

    if (halScreen.DirtyFlag & CMDLINE_CURSOR_DIRTY)
    {
        // Draw the cursor
        coord x      = halScreen.CursorX - halScreen.XVisible;
        coord y      = top + 2 + stkh;
        coord bottom = y + rowh - 1;
        if (!(halScreen.CursorState & 0x8000))
        {
            // REVISIT - EndianBug in 'cursor'
            const UNIFONT *cfont   = FONT_CURSOR;
            utf8_p         cursor = (utf8_p) &halScreen.CursorState;
            pattern_t      color  = PAL_CMD_CURSOR;
            pattern_t      bg     = PAL_CMD_CURSOR_BG;
            coord          cx     = x;
            coord          cy     = y;
            coord          ch     = cfont->BitmapHeight;
            coord          th     = font->BitmapHeight;
            if (ch < th)
            {
                cy += (th - ch) / 2;
                cx += 1;
                ggl_cliprect(scr, x, y-1, x+(LCD_W>131), y + th - 1, bg);
            }
            DrawTextBkN(scr, cx, cy, cursor, cursor + 1, cfont, color, bg);
        }
        else
        {
            coord saveLeft = scr->left;
            coord saveRight = scr->left;
            scr->left      = x;

            // Hard code maximum width of the cursor
            scr->right = min(scr->left + rowh + 4, LCD_W - 1);

            // Redraw the portion of command line under the cursor
            if (!(halScreen.DirtyFlag & CMDLINE_LINE_DIRTY))
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
                    tail = 1;
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
                    selend  = selst = string;

                // Draw the line split in 3 sections:
                // - string to selst,
                // - selst to selend,
                // - selend to strend
                x = -halScreen.XVisible;
                if (selst > string)
                    x = DrawTextBkN(scr, x, y, string, selst, font, color, bg);
                if (selend > selst)
                    x = DrawTextBkN(scr, x, y, selst, selend, font, selcol, selbg);
                if (strend > selend)
                    x = DrawTextBkN(scr, x, y, selend, strend, font, color, bg);
                if (tail)
                {
                    ggl_cliprect(scr, x, y, x + 3, bottom, bg);
                    x += 3;
                }

                // Clear up to end of line
                ggl_cliprect(scr, x, y, LCD_SCANLINE - 1, bottom, bg);
            }

            // Reset the clipping rectangle back to whole screen
            scr->left  = saveLeft;
            scr->right = saveRight;
        }
    }
    halRepainted(CMDLINE_ALL_DIRTY);
}


static void battery_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Draw the battery indicator
// ----------------------------------------------------------------------------
{
    const UNIFONT *iconFont   = Font_Notifications;
    const UNIFONT *labelFont  = FONT(BATTERY);
    coord          width  = StringWidth("100%", labelFont) + 3;
    coord          height = labelFont->BitmapHeight + iconFont->BitmapHeight;
    layout_clip(scr, layout, rect, width, height);

    // Extract draw coordinates
    coord x = (rect->left + rect->right) / 2;
    coord y = rect->top;

    if (battery_charging())
    {
        // Battery is charging - display charging icon
        DrawTextBk(scr, x, y, "C", iconFont, PAL_STA_BAT, PAL_STA_BG);
    }
    else
    {
        int battery = battery_level();

        // Draw the battery icon
        pattern_t color = PAL_STA_BAT;
        if (battery_low())
            color = PAL_ERROR;

        coord batH = 10;
        coord batW =  2;
        coord batC = battery * batH / 100;
        ggl_cliprect(scr, x-batW, y+batH-batC, x+batW, y+batH, PAL_STA_BAT);
        ggl_cliprect(scr, x, y, x, y, PAL_STA_BAT);
        ggl_cliprect(scr, x-batW, y+1, x-batW, y+batH, PAL_STA_BAT);
        ggl_cliprect(scr, x+batW, y+1, x+batW, y+batH, PAL_STA_BAT);
        ggl_cliprect(scr, x-batW, y+1, x+batW, y+1,    PAL_STA_BAT);
        ggl_cliprect(scr, x-batW, y+batH, x+batW, y+batH, PAL_STA_BAT);

        // Display Battery percentage below battery icon
        char buf[8];
        snprintf(buf, sizeof(buf), "%3d%%", battery);
        coord labelWidth = StringWidth(buf, labelFont);
        DrawText(scr, x - labelWidth / 2, y + batH + 2, buf, labelFont, color);
    }
}


void halUpdateFonts()
// ----------------------------------------------------------------------------
// Get new font data from the rpl environment
// ----------------------------------------------------------------------------
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
            case FONT_INDEX_STACK_LEVEL1:
                uiClearRenderCache();
                halRefresh(STACK_DIRTY);
                break;
            case FONT_INDEX_CMDLINE:
            case FONT_INDEX_CURSOR:
                if (halScreen.CmdLine)
                    uiStretchCmdLine(0);
                halRefresh(CMDLINE_ALL_DIRTY);
                break;
            case FONT_INDEX_MENU:
                if (halScreen.Menu1)
                    halSetMenu1Height(MENU1_HEIGHT);
                if (halScreen.Menu2)
                    halSetMenu2Height(MENU2_HEIGHT);
                halRefresh(MENU_DIRTY);
                break;
            case FONT_INDEX_STATUS:
                halRefresh(STATUS_DIRTY);
                break;
            case FONT_INDEX_PLOT:
                halRefresh(ALL_DIRTY);
                break;
            case FONT_INDEX_FORMS:
                halRefresh(FORM_DIRTY);
                break;
            case FONT_INDEX_HELP_TITLE:
            case FONT_INDEX_HELP_TEXT:
                halRefresh(HELP_DIRTY);
                break;
            }
        }
        else
        {
            halScreen.FontArray[f] = tmparray[f];
        }
    }
}


void halPrepareBuffer(gglsurface *scr)
// ----------------------------------------------------------------------------
// Prepare to draw on the alternative buffer
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
//  Draw on alternative buffer
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
//   Force an entire screen redraw
// ----------------------------------------------------------------------------
{
    halScreen.DirtyFlag = ALL_DIRTY;
    halRedrawAll(scr);
}


void halRedrawAll(gglsurface *scr)
// ----------------------------------------------------------------------------
//   Redraw the whole screen using the layout engine
// ----------------------------------------------------------------------------
{
    unsigned dirty = halScreen.DirtyFlag;
    if (dirty)
    {
        halUpdateFonts();
        halPrepareBuffer(scr);
        render_layouts(scr);
        halSwapBuffer(scr);
        halScreen.DirtyFlag = 0;
        halScreenUpdated();
    }
}

void halUpdateStatus()
// ----------------------------------------------------------------------------
// Mark status area for immediate update
// ----------------------------------------------------------------------------
{
    halRefresh(STATUS_DIRTY);
}


const WORD const text_editor_string[] = {
    MAKESTRING(12),
    TEXT2WORD('C', 'o', 'm', 'm'),
    TEXT2WORD('a', 'n', 'd', ' '),
    TEXT2WORD('L', 'i', 'n', 'e')
};

word_p halGetCommandName(word_p NameObject)
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

    if (IS_PROLOG(Opcode))
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


static void errors_layout(gglsurface *scr, layout_p layout, rect_t *rect)
// ----------------------------------------------------------------------------
//   Draw the last error message
// ----------------------------------------------------------------------------
{
    // Check if there is an error
    WORD     error   = Exceptions;
    utf8_p   message = halScreen.ErrorMessage;
    unsigned length  = halScreen.ErrorMessageLength;
    size     width   = error ? LCD_W : 0;
    size     height  = error ? LCD_H : 0;
    layout_clip(scr, layout, rect, width, height);
    if (!error)
        return;

    // Color schemes
    pattern_t color      = PAL_ERROR;
    pattern_t line       = PAL_ERROR_LINE;
    pattern_t bg         = PAL_ERROR_BG;

    // Draw the error outline and erase background
    coord     left       = rect->left;
    coord     top        = rect->top;
    coord     right      = rect->right;
    coord     bottom     = rect->bottom;
    ggl_cliprect(scr, left, top, right, bottom, line);

    coord inset = 3;
    left   += inset;
    right  -= inset;
    top    += inset;
    bottom -= inset;
    ggl_cliprect(scr, left, top, right, bottom, bg);

    // Buffers to keep copy of error in case of GC
    static char error_cmd[64];
    static char error_message[256];

    // Update message from exceptions if needed
    if (error)
    {
        // Clear Exceptions, we generate / display the message
        Exceptions = 0;

        // Clear short help message
        halScreen.ShortHelpMessage = NULL;

        // Check if there is a command to blame
        if (ExceptionPointer != 0)
        {
            word_p cmdname = halGetCommandName(ExceptionPointer);
            if (cmdname)
            {
                // Save a copy of the command name in case of GC
                utf8_p start = (utf8_p) (cmdname + 1);
                int    size  = rplStrSize(cmdname);

                snprintf(error_cmd, sizeof(error_cmd),
                         "%.*s", size, start);
                halScreen.ShortHelpMessage = error_cmd;
            }
        }

        // Check if we have an exception
        if (error != EX_ERRORCODE)
        {
            // Check exception
            int32_t ecode = 0;
            for (int errbit = 0; errbit < 8; ++errbit)
            {
                if (error & (1 << errbit))
                {
                    ecode           = MAKEMSG(0, errbit);
                    word_p emsg = uiGetLibMsg(ecode);
                    if (!emsg)
                        emsg = uiGetLibMsg(ERR_UNKNOWNEXCEPTION);
                    if (emsg)
                    {
                        utf8_p etext = (utf8_p) (emsg + 1);
                        int msglen = rplStrSize(emsg);

                        snprintf(error_message, sizeof(error_message),
                                 "Exception: %.*s", msglen, etext);

                        // Update for later display
                        message = error_message;
                        length = msglen;
                    }
                }
                break;
            }
        }
        else
        {
            // Regular error code
            word_p emsg = uiGetLibMsg(ErrorCode);
            if (!emsg)
                emsg = uiGetLibMsg(ERR_UNKNOWNEXCEPTION);
            if (emsg)
            {
                utf8_p etext = (utf8_p) (emsg + 1);
                int msglen   = rplStrSize(emsg);

                snprintf(error_message, sizeof(error_message),
                         "Error: %.*s", msglen, etext);

                // Update for later display
                message = error_message;
                length = msglen;
            }
        }

        // Update the persistent message
        halScreen.ErrorMessage = message;
        halScreen.ErrorMessageLength = length;
    }

    // Draw the command if we have it
    utf8_p cmd = halScreen.ShortHelpMessage;
    if (cmd)
    {
        const UNIFONT *font = FONT_HELP_CODE;
        DrawText(scr, left, top, cmd, font, color);
        top += FONT_HEIGHT(font);
    }

    // Draw the command if we have it
    if (message)
    {
        const UNIFONT *font = FONT_ERRORS;
        DrawTextN(scr, left, top, message, message + length, font, color);
    }
}


void halShowMsgN(utf8_p Text, utf8_p End)
// ----------------------------------------------------------------------------
//   Show an error message
// ----------------------------------------------------------------------------
{
    halScreen.ErrorMessage = Text;
    halScreen.ErrorMessageLength = End - Text;
}


void halShowMsg(utf8_p Text)
// ----------------------------------------------------------------------------
//   Show a static message
// ----------------------------------------------------------------------------
{
    utf8_p End = StringEnd(Text);
    halShowMsgN(Text, End);
}


void halCancelPopup()
// ----------------------------------------------------------------------------
//   Cancel error messages
// ----------------------------------------------------------------------------
{
    halScreen.ErrorMessage = NULL;
    halScreen.ErrorMessageLength = 0;
}


void halSwitch2Form()
// ----------------------------------------------------------------------------
// Change the context and display the current form
// ----------------------------------------------------------------------------
{
    if (halContext(CONTEXT_EDITOR))
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
}


void halSwitch2Stack()
// ----------------------------------------------------------------------------
// Change the context and display the stack
// ----------------------------------------------------------------------------
{
    if (halContext(CONTEXT_EDITOR))
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
}
