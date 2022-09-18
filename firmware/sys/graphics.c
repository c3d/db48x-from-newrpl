/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// BASIC GRAPHICS ROUTINES
#include <ui.h>


typedef const unsigned *offset_p;


static inline coord DrawTextInternal(gglsurface    *surface,
                                     coord          x,
                                     coord          y,
                                     utf8_p         Text,
                                     utf8_p         End,
                                     UNIFONT const *Font,
                                     gglop          fgop,
                                     pattern_t      foreground,
                                     gglop          bgop,
                                     pattern_t      background)
// ----------------------------------------------------------------------------
//   Draw a UTF-8 text applying the given graphic operations
// ----------------------------------------------------------------------------
{
    // If clipping conditions are such that we won't draw anything, quick exit
    if (surface->right < 0)
        return x;
    if (y > surface->bottom)
        return x;
    if (y + (int) Font->BitmapHeight <= surface->top)
        return x;

    unsigned  *fbase    = (unsigned *) Font;
    offset_p   offtable = (offset_p) (fbase + Font->OffsetTable);
    pixword   *fontbits = (pixword *) (fbase + Font->OffsetBitmap);

    size       fwidth   = Font->BitmapWidth << 3;
    size       fheight  = Font->BitmapHeight;
    gglsurface fontsrf  = ggl_monochrome_bitmap(fontbits, fwidth, fheight);

    coord      x0       = x;
    size       width    = 0;
    size       height   = fheight;

    while (Text < End)
    {
        int cp = utf82cp(Text, End);
        if (cp == -1)
        {
            // Skip invalid code points
            ++Text;
            continue;
        }
        if (cp == '\n' || cp == '\r')
        {
            // CR or LF:
            Text++;
            if (Text != End)
            {
                x = x0;
                y += height;
            }
            continue;
        }

        if (cp > 0x7f)
        {
            int cpinfo = getCPInfo(cp);

            // Check combining marks
            switch (cp)
            {
            case 0x0305:
                // Combining overline
                // We use the width from previous character to draw overline
                // REVISIT: How does this work with background?
                ggl_cliphline(surface, y, x - width, x, foreground);
                break;
            }

            if (CCLASS(cpinfo) != 0)
            {
                // TODO: Add support for combiners
                Text = utf8skip(Text, End);
                continue;
            }
        }

        // Get the information from the font
        int      rangeend = 0;
        int      startcp  = 0;
        offset_p mapptr   = Font->MapTable - 1;
        do
        {
            ++mapptr;
            startcp  = rangeend;
            rangeend = startcp + RANGE_LEN(*mapptr);
        } while (cp >= rangeend);

        int offset = FONT_OFFSET(*mapptr);
        if (offset == 0xfff)
            width = offtable[0];
        else
            width = offtable[offset + cp - startcp];

        int xf = width & 0xffff;
        width >>= 16;

        if (width)
        {
            ggl_mixblit(surface,        // Target surface
                        &fontsrf,       // Font we get the bits from
                        x,              // Left position on screen
                        x + width - 1,  // Right position on screen
                        y,              // Top position on screen
                        y + height - 1, // Bottom position on screen
                        xf,             // Horizontal position in the font
                        0,              // Vertical position in the font
                        fgop,           // Set the pixels
                        foreground,     // Colors to use
                        surface->bpp,   // Bits per pixel of target surface
                        1,              // Bits per pixel of font
                        surface->bpp,   // Bits per pixel for colors
                        CLIP_ALL);      // Perform clipping
            if (bgop)
                ggl_mixblit(surface,
                            &fontsrf,
                            x,
                            x + width - 1,
                            y,
                            y + height - 1,
                            xf,
                            0,
                            bgop,
                            background,
                            surface->bpp,
                            1,
                            surface->bpp,
                            CLIP_ALL);

            x += width;
        }
        Text = utf8skip(Text, End);
    }
    return x;
}


size StringWidthN(utf8_p Text, utf8_p End, UNIFONT const *Font)
// ----------------------------------------------------------------------------
//   Compute the width of a text given as an extent
// ----------------------------------------------------------------------------
{
    int cp, startcp, rangeend, offset, cpinfo;
    unsigned int *offtable;
    const unsigned int *mapptr;
    unsigned int w;
    int width = 0;

    offtable = (unsigned int *)(((unsigned int *)Font) + Font->OffsetTable);

    while(Text < End) {

        cp = utf82cp(Text, End);

        if(cp == -1) {
            ++Text;
            continue;
        }

        if(cp == '\n' || cp == '\r')
            return width;

        if(cp > 0x7f) {

            cpinfo = getCPInfo(cp);

            if(CCLASS(cpinfo) != 0) {
                // ADD SUPPORT FOR COMBINERS
                Text = utf8skip(Text, End);
                continue;
            }
        }

        // GET THE INFORMATION FROM THE FONT
        rangeend = 0;
        mapptr = Font->MapTable - 1;
        do {
            ++mapptr;
            startcp = rangeend;
            rangeend = startcp + RANGE_LEN(*mapptr);
        }
        while(cp >= rangeend);

        offset = FONT_OFFSET(*mapptr);
        if(offset == 0xfff)
            w = offtable[0];
        else {
            w = offtable[offset + cp - startcp];
        }

        width += (int)(w >> 16);
        Text = utf8skip(Text, End);

    }

    return width;

}

// GET A POINTER INTO THE CHARACTER THAT WILL BE PRINTED AT *xcoord
// ASSUMING x=0 AT Text START
// IT WILL RETURN A POINTER INTO THE STRING, AND MODIFY *xcoord WITH THE CORRECT X COORDINATE
// ALIGNED WITH THE FONT

// WARNING: DO NOT PASS xcoord=NULL, NO ARGUMENT CHECKS

utf8_p StringCoordToPointer(utf8_p        Text,
                             utf8_p        End,
                             UNIFONT const *Font,
                             int           *xcoord)
{
    int cp, startcp, rangeend, offset, cpinfo;
    unsigned int *offtable;
    const unsigned int *mapptr;
    unsigned int w;
    int  width = 0;

    if(*xcoord < 0) {
        *xcoord = 0;
        return Text;
    }

    offtable = (unsigned int *)(((unsigned int *)Font) + Font->OffsetTable);

    while(Text < End) {

        cp = utf82cp(Text, End);

        if(cp == -1) {
            ++Text;
            continue;
        }

        if(cp == '\n' || cp == '\r') {
            *xcoord = width;
            return Text;
        }

        if(cp > 0x7f) {

            cpinfo = getCPInfo(cp);
            if(CCLASS(cpinfo) != 0) {
                // ADD SUPPORT FOR COMBINERS
                Text = utf8skip(Text, End);
                continue;
            }

        }

        // GET THE INFORMATION FROM THE FONT
        rangeend = 0;
        mapptr = Font->MapTable - 1;
        do {
            ++mapptr;
            startcp = rangeend;
            rangeend = startcp + RANGE_LEN(*mapptr);
        }
        while(cp >= rangeend);

        offset = FONT_OFFSET(*mapptr);
        if(offset == 0xfff)
            w = offtable[0];
        else {
            w = offtable[offset + cp - startcp];
        }

        if((*xcoord >= width) && (*xcoord < width + (int)(w >> 16))) {
            *xcoord = width;
            return Text;
        }

        width += w >> 16;

        Text = utf8skip(Text, End);

    }

    *xcoord = width;
    return End;

}

size StringWidth(utf8_p Text, UNIFONT const * Font)
// ----------------------------------------------------------------------------
//   Compute the width of a zero-terminated UTF-8 text
// ----------------------------------------------------------------------------
{
    utf8_p End = StringEnd(Text);
    return StringWidthN(Text, End, Font);
}


void DrawTextN(gglsurface    *surface,
               coord          x,
               coord          y,
               utf8_p         Text,
               utf8_p         End,
               UNIFONT const *Font,
               pattern_t      foreground)
// ----------------------------------------------------------------------------
//   Draw a range of text
// ----------------------------------------------------------------------------
{
    DrawTextInternal(surface,
                     x,
                     y,
                     Text,
                     End,
                     Font,
                     ggl_mono_fg,
                     foreground,
                     NULL,
                     foreground);
}


void DrawText(gglsurface    *surface,
              coord          x,
              coord          y,
              utf8_p        Text,
              UNIFONT const *Font,
              pattern_t      color)
// ----------------------------------------------------------------------------
//   Draw a null-terminated transparent text
// ----------------------------------------------------------------------------
{
    utf8_p End = StringEnd(Text);
    DrawTextN(surface, x, y, Text, End, Font, color);
}



void DrawTextBkN(gglsurface    *surface,
                 coord          x,
                 coord          y,
                 utf8_p         Text,
                 utf8_p         End,
                 UNIFONT const *Font,
                 pattern_t      foreground,
                 pattern_t      background)
// ----------------------------------------------------------------------------
//   Draw a text range with a background color
// ----------------------------------------------------------------------------
{
    DrawTextInternal(surface,
                     x,
                     y,
                     Text,
                     End,
                     Font,
                     ggl_mono_fg,
                     foreground,
                     ggl_mono_bg,
                     background);
}

void DrawTextBk(gglsurface    *surface,
                coord          x,
                coord          y,
                utf8_p        Text,
                UNIFONT const *Font,
                pattern_t      foreground,
                pattern_t      background)
// ----------------------------------------------------------------------------
//   Draw a nul-terminated text with a foreground and background color
// ----------------------------------------------------------------------------
{
    utf8_p End = StringEnd(Text);
    DrawTextBkN(surface, x, y, Text, End, Font, foreground, background);
}


void DrawTextMono(gglsurface    *surface,
                  coord          x,
                  coord          y,
                  utf8_p        Text,
                  UNIFONT const *Font,
                  pattern_t      color)
// ----------------------------------------------------------------------------
//   Draw a text to a monochrome target surface
// ----------------------------------------------------------------------------
{
    utf8_p End = StringEnd(Text);
    DrawTextInternal(surface,
                     x,
                     y,
                     Text,
                     End,
                     Font,
                     ggl_mono_fg_1bpp,
                     color,
                     NULL,
                     color);
}
