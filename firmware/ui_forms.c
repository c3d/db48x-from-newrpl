/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <cmdcodes.h>
#include <libraries.h>
#include <ui.h>

void uiRepaintForm(gglsurface * scr)
{
    // REPAINT VISIBLE PART OF A FORM
    UNUSED_ARGUMENT(scr);

}

// COMPUTE THE TOTAL DIMENSIONS OF A FORM
// USED TO ALLOCATE THE FORM BITMAP

void uiFormGetDimensions(WORDPTR form, BINT * width, BINT * height)
{
    BINT w = 0, h = 0;
    BINT roww, rowh;
    WORDPTR item, end;

    if(!ISLIST(*form)) {
        if(width)
            *width = 0;
        if(height)
            *height = 0;
        return;
    }
    end = rplSkipOb(form);
    item = form + 1;
    while(item < end) {
        roww = LCD_W;

        if(ISBINT(*item)) {
            rowh = rplReadBINT(item);
            item = rplSkipOb(item);
        }
        else {
            // TODO: DEFINE THE HEIGHT FOR VARIOUS OTHER TYPES

            // BY DEFAULT, THIS IS THE HEIGHT OF ONE LINE OF TEXT
            rowh = FONT_HEIGHT(FONT_FORMS);

        }

        if(ISLIST(*item)) {
            // A LIST OF COLUMNS
            int ncols = 0, forced = 0;
            roww = 0;
            WORDPTR col = item + 1, endcol = rplSkipOb(item);

            while(col < endcol) {
                // SCAN THE COLUMN

                if(ISBINT(*col)) {
                    roww += rplReadBINT(col);
                    col = rplSkipOb(col);
                    ++forced;
                }

                if(*col != CMD_ENDLIST)
                    ++ncols;
                col = rplSkipOb(col);

            }

            roww += (ncols - forced) * (LCD_W / (ncols));

        }

        // HERE WE HAVE ROW WIDTH AND HEIGHT
        h += rowh;
        if(roww > w)
            w = roww;

        item = rplSkipOb(item);
    }

    if(width)
        *width = w;
    if(height)
        *height = h;

}

void uiUpdateForm(WORDPTR form)
{
    // THE FORM IS A LIST OF ELEMENTS IN ROW ORDER
    // AN ELEMENT CAN BE ONLY 3 THINGS: TEXT, AN IDENT OR A LIST. ANY OF THESE ELEMENTS MIGHT BE PRECEDED BY AN INTEGER WITH THE ROW HEIGHT
    // ELEMENTS:
    //          INTEGERS: AN INTEGER PROVIDES THE HEIGHT OF THE CURRENT ROW, THEN IT'S IGNORED
    //                    A ROW WITHOUT AN INTEGER NUMBER DEFAULTS TO THE FONT HEIGHT OF THE FORM (1 LINE)
    //          TEXT:     WILL BE OUTPUT LITERALLY ON THE FORM
    //          IDENT:    THE IDENTIFIER IS TREATED AS A FIELD NAME. SEVERAL RELATED IDENTIFIERS DEFINE A FIELD.
    //          LIST:     IF THE ROW IS A LIST, THEN ELEMENTS ARE LAID OUT IN COLUMNS FROM LEFT TO RIGHT
    //                    COLUMN ELEMENTS ARE THE SAME WAY, WHERE INTEGERS OPTIONALLY DEFINE THE WIDTH OF THE ITEM (DEFAULT TO EQUALLY SPACED)

    // PASS 1: COMPUTE TOTAL WIDTH AND HEIGHT

    if(!ISLIST(*form))
        return;

    BINT formw, formh;
    uiFormGetDimensions(form, &formw, &formh);

    // ALLOCATE NEW BITMAP
    ScratchPointer1 = form;

    WORDPTR newbmp = rplBmpCreate(DEFAULT_BITMAP_MODE, formw, formh, 1);  // ALLOCATE A NEW BITMAP AND CLEAR IT
    if(!newbmp)
        return;
    form = ScratchPointer1;

    // PASS 2: REDRAW EACH ELEMENT ONTO THE NEW BITMAP

    gglsurface backgnd;

    backgnd.addr = (int *)(newbmp + 3);
    backgnd.width = formw;
    backgnd.x = 0;
    backgnd.y = 0;

    BINT ycoord = 0;

    BINT roww, rowh, rowid = 0;
    WORDPTR item, end;
    end = rplSkipOb(form);
    item = form + 1;
    while(item < end) {
        roww = LCD_W;

        if(ISBINT(*item)) {
            rowh = rplReadBINT(item);
            item = rplSkipOb(item);
        }
        else
            rowh = FONT_HEIGHT(FONT_FORMS);

        if(ISLIST(*item)) {
            // A LIST OF COLUMNS
            int ncols = 0, forced = 0;
            roww = 0;
            WORDPTR col = item + 1, endcol = rplSkipOb(item);
            BINT def_itemw, itemw;

            while(col < endcol) {
                // SCAN THE COLUMN

                if(ISBINT(*col)) {
                    roww += rplReadBINT(col);
                    col = rplSkipOb(col);
                    ++forced;
                }

                if(*col != CMD_ENDLIST)
                    ++ncols;
                col = rplSkipOb(col);

            }
            def_itemw = LCD_W / ncols;
            roww += (ncols - forced) * (LCD_W / (ncols));

            // NOW RENDER IT

            col = item + 1;
            backgnd.clipx = 0;
            backgnd.clipy = ycoord;
            backgnd.clipy2 = ycoord + rowh - 1;
            while(col < endcol) {
                if(ISBINT(*col)) {
                    itemw = rplReadBINT(col);
                    col = rplSkipOb(col);
                }
                else
                    itemw = def_itemw;
                if(ISSTRING(*col)) {
                    // DRAW THE STRING
                    // TODO: SUPPORT MULTILINE TEXT HERE, WITH PROPER TEXT WRAP
                    backgnd.clipx2 = backgnd.clipx + itemw - 1;
                    DrawTextBkN(backgnd.clipx, backgnd.clipy, (char *)(col + 1),
                            (char *)(col + 1) + rplStrSize(col),
                            FONT_FORMS, 0xf, 0, &backgnd);
                    backgnd.clipx = backgnd.clipx2 + 1;
                }

                //TODO: RENDER FIELDS HERE

                col = rplSkipOb(col);

            }

        }

        if(ISSTRING(*item)) {
            // DRAW THE STRING
            // TODO: SUPPORT MULTILINE TEXT HERE, WITH PROPER TEXT WRAP
            backgnd.clipx = 0;
            backgnd.clipy = ycoord;
            backgnd.clipy2 = ycoord + rowh - 1;
            backgnd.clipx2 = backgnd.clipx + roww - 1;
            DrawTextBkN(backgnd.clipx, backgnd.clipy, (char *)(item + 1),
                    (char *)(item + 1) + rplStrSize(item),
                    FONT_FORMS, 0xf, 0, &backgnd);
            if(!rowid) {
                // THIS IS THE FORM TITLE, HIGHLIGHT IT
                gglsurface copy;
                copy.addr = backgnd.addr;
                copy.width = backgnd.width;

                copy.x = 0;
                copy.y = 0;
                copy.clipx = backgnd.x;
                backgnd.x = (backgnd.width - backgnd.x) / 2;
                // CENTER THE TEXT
                ggl_bitblt(&backgnd, &copy, copy.clipx,
                        backgnd.clipy2 - backgnd.clipy + 1);
                ggl_cliprect(&backgnd, backgnd.x + copy.clipx, backgnd.clipy,
                        backgnd.clipx2, backgnd.clipy2, 0x44444444);
                ggl_cliprect(&backgnd, 0, backgnd.clipy, backgnd.x - 1,
                        backgnd.clipy2, 0x44444444);
            }

        }

        // TODO: RENDER FIELDS HERE

        // NEXT ROW
        ycoord += rowh;
        rowid++;
        item = rplSkipOb(item);
    }

    // DONE, EVERYTHING WAS DRAWN

    uiUpdateOrAddCacheEntry(form, newbmp, FONT_FORMS);

}
