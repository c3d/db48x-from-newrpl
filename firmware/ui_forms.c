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
    UNUSED(scr);

}

// COMPUTE THE TOTAL DIMENSIONS OF A FORM
// USED TO ALLOCATE THE FORM BITMAP

void uiFormGetDimensions(word_p form, int32_t * width, int32_t * height)
{
    int32_t w = 0, h = 0;
    int32_t roww, rowh;
    word_p item, end;

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

        if(ISint32_t(*item)) {
            rowh = rplReadint32_t(item);
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
            word_p col = item + 1, endcol = rplSkipOb(item);

            while(col < endcol) {
                // SCAN THE COLUMN

                if(ISint32_t(*col)) {
                    roww += rplReadint32_t(col);
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

void uiUpdateForm(word_p form)
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

    int32_t formw, formh;
    uiFormGetDimensions(form, &formw, &formh);

    // ALLOCATE NEW BITMAP
    ScratchPointer1 = form;

    word_p newbmp = rplBmpCreate(DEFAULT_BITMAP_MODE, formw, formh, 1);  // ALLOCATE A NEW BITMAP AND CLEAR IT
    if(!newbmp)
        return;
    form = ScratchPointer1;

    // Pass 2: Redraw each element onto the new bitmap
    gglsurface backgnd = ggl_grob(newbmp);

    int32_t ycoord = 0;

    int32_t roww, rowh, rowid = 0;
    word_p item, end;
    end = rplSkipOb(form);
    item = form + 1;
    while(item < end) {
        roww = LCD_W;

        if(ISint32_t(*item)) {
            rowh = rplReadint32_t(item);
            item = rplSkipOb(item);
        }
        else
            rowh = FONT_HEIGHT(FONT_FORMS);

        if(ISLIST(*item)) {
            // A LIST OF COLUMNS
            int ncols = 0, forced = 0;
            roww = 0;
            word_p col = item + 1, endcol = rplSkipOb(item);
            int32_t def_itemw, itemw;

            while(col < endcol) {
                // Scan the column
                if(ISint32_t(*col)) {
                    roww += rplReadint32_t(col);
                    col = rplSkipOb(col);
                    ++forced;
                }

                if(*col != CMD_ENDLIST)
                    ++ncols;
                col = rplSkipOb(col);
            }
            def_itemw = LCD_W / ncols;
            roww += (ncols - forced) * (LCD_W / (ncols));

            // Now render it
            col = item + 1;
            backgnd.left = 0;
            backgnd.top = ycoord;
            backgnd.bottom = ycoord + rowh - 1;
            while(col < endcol) {
                if(ISint32_t(*col)) {
                    itemw = rplReadint32_t(col);
                    col = rplSkipOb(col);
                }
                else
                    itemw = def_itemw;
                if(ISSTRING(*col)) {
                    // Draw the string
                    // TODO: Support multiline text here, with proper text wrap
                    backgnd.right = backgnd.left + itemw - 1;
                    DrawTextBkN(&backgnd, backgnd.left, backgnd.top, (char *)(col + 1),
                                (char *)(col + 1) + rplStrSize(col),
                                FONT_FORMS, PAL_GRAY15, PAL_GRAY0);
                    backgnd.left = backgnd.right + 1;
                }

                //TODO: Render fields here
                col = rplSkipOb(col);
            }

        }

        if(ISSTRING(*item)) {
            // Draw the string
            // TODO: Support multiline text here, with proper text wrap
            utf8_p str = (utf8_p) (item+1);
            size_t len = rplStrSize(item);
            coord left = 0;
            coord top = ycoord;
            coord right = backgnd.left + roww - 1;
            coord bottom = ycoord + rowh - 1;
            backgnd.right = right;
            backgnd.bottom = bottom;
            if(rowid)
            {
                // Regular row: just draw it as is
                DrawTextBkN(&backgnd,
                            left,
                            top,
                            str,
                            str + len,
                            FONT_FORMS,
                            PAL_GRAY15,
                            PAL_GRAY0);
            }
            else
            {
                size width = StringWidthN(str, str + len, FONT_FORMS);
                ggl_cliprect(&backgnd,
                             left, top, right, bottom,
                             PAL_GRAY12);
                DrawTextBkN(&backgnd,
                            left + width/2,
                            top,
                            str,
                            str + len,
                            FONT_FORMS,
                            PAL_GRAY2,
                            PAL_GRAY12);
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
