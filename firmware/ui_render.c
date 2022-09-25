/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <ui.h>
#include <libraries.h>

#define NEXT_ENTRY (halCacheEntry&0xffff)
#define CACHE_FULL 0x10000

#define INC_NEXT_ENTRY ((halCacheEntry+1)&(0xffff0000|(MAX_RENDERCACHE_ENTRIES-1)))

#define MAX_BMP_WIDTH (4*LCD_W)       // MAXIMUM WIDTH OF A BITMAP TO RENDER AN OBJECT = 4 SCREENS
#define MAX_BMP_HEIGHT (4*LCD_H)      // MAXIMUM HEIGHT OF A BITMAP TO RENDER AN OBJECT = 4 SCREENS

// INVALIDATE ALL CACHE ENTRIES
void uiClearRenderCache()
{
    halCacheEntry = 0;
}

// ADD AN ENTRY TO THE CACHE
void uiAddCacheEntry(word_p object, word_p bitmap, UNIFONT const *font)
{
    if(GCFlags & GC_COMPLETED) {
        uiClearRenderCache();
        GCFlags = 0;
    }

    halCacheContents[NEXT_ENTRY * 3] = object;
    halCacheContents[NEXT_ENTRY * 3 + 1] = bitmap;
    halCacheContents[NEXT_ENTRY * 3 + 2] = (word_p) font;
    halCacheEntry = INC_NEXT_ENTRY;
    if(NEXT_ENTRY == 0)
        halCacheEntry |= CACHE_FULL;
}

void uiUpdateOrAddCacheEntry(word_p object, word_p bitmap, UNIFONT const *font)
{
    if(GCFlags & GC_COMPLETED) {
        uiClearRenderCache();
        GCFlags = 0;
    }

    int k;
    int limit =
            (halCacheEntry & CACHE_FULL) ? MAX_RENDERCACHE_ENTRIES : NEXT_ENTRY;

    limit *= 3;

    for(k = 0; k < limit; k += 3) {
        if(halCacheContents[k] == object) {
            halCacheContents[k + 1] = bitmap;
            halCacheContents[k + 2] = (word_p) font;
            return;
        }

    }

    halCacheContents[NEXT_ENTRY * 3] = object;
    halCacheContents[NEXT_ENTRY * 3 + 1] = bitmap;
    halCacheContents[NEXT_ENTRY * 3 + 2] = (word_p) font;
    halCacheEntry = INC_NEXT_ENTRY;
    if(NEXT_ENTRY == 0)
        halCacheEntry |= CACHE_FULL;
}

// USE AN ENTRY IN THE CACHE
word_p uiFindCacheEntry(word_p object, const UNIFONT *font)
{
    if(GCFlags & GC_COMPLETED) {
        uiClearRenderCache();
        GCFlags = 0;
        return 0;
    }

    int k;
    int limit =
            (halCacheEntry & CACHE_FULL) ? MAX_RENDERCACHE_ENTRIES : NEXT_ENTRY;

    limit *= 3;

    for(k = 0; k < limit; k += 3) {
        if(halCacheContents[k] == object) {
            if(halCacheContents[k + 2] == (word_p) font)
                return halCacheContents[k + 1];
        }

    }
    return 0;
}

// ALLOCATE NEW BITMAP OBJECT, THIS IS HARDWARE DEPENDENT AS IT USES THE DEFAULT SCREEN COLOR MODE

word_p uiAllocNewBitmap(int32_t width, int32_t height)
{

    int32_t bits = BITS_PER_PIXEL * width * height;

    bits += 31;
    bits >>= 5;

    word_p newobj = rplAllocTempOb(bits + 2);
    if(!newobj)
        return 0;
    newobj[0] = MKPROLOG(DOBITMAP + DEFAULT_BITMAP_MODE, bits + 2);
    newobj[1] = width;
    newobj[2] = height;

    return newobj;
}

// RENDER AN OBJECT TO THE GIVEN gglsurface, USE CACHE IF POSSIBLE

void uiDrawObject(word_p object, gglsurface * scr, UNIFONT const *font)
{

    // FIRST, CHECK IF THE OBJECT IS IN THE CACHE
    word_p bmp = uiRenderObject(object, font);
    if(bmp) {
        // COPY IT TO DESTINATION
        gglsurface tsurf = ggl_grob(bmp);
        ggl_copy(scr, &tsurf, tsurf.width, tsurf.height);
        return;
    }

    // DRAW DIRECTLY, DON'T CACHE SOMETHING WE COULDN'T RENDER

    word_p string = (word_p) invalid_string;

    // NOW PRINT THE STRING OBJECT
    int32_t nchars = rplStrSize(string);
    cstring charptr = (cstring) (string + 1);

    DrawTextN(scr,
              scr->x,
              scr->y,
              charptr,
              charptr + nchars,
              font,
              ggl_solid(PAL_STK_ITEMS));
}

// RENDER AN OBJECT TO A BITMAP, USE CACHE IF POSSIBLE

word_p uiRenderObject(word_p object, UNIFONT const *font)
{

    // FIRST, CHECK IF THE OBJECT IS IN THE CACHE

    word_p bmp = uiFindCacheEntry(object, font);

    if(bmp)
        return bmp;

    // OBJECT WAS NOT IN CACHE, RENDER IT AND ADD IT TO CACHE

    // TODO: CHANGE DECOMPILE INTO PROPER DISPLAY FUNCTION
    word_p string;
    string = rplDecompile(object, DECOMP_NOHINTS);

    if(!string)
        string = (word_p) invalid_string;

    // NOW PRINT THE STRING OBJECT

    int32_t nchars = rplStrSize(string);
    cstring charptr = (cstring) (string + 1);
    int32_t numwidth = StringWidthN(charptr, charptr + nchars, font);

    if(numwidth > MAX_BMP_WIDTH)
        numwidth = MAX_BMP_WIDTH;

    ScratchPointer1 = string;

    word_p newbmp = uiAllocNewBitmap(numwidth, font->BitmapHeight);
    if(newbmp) {

        // RELOAD ALL POINTERS IN CASE THERE WAS A GC
        string = ScratchPointer1;
        charptr = (cstring) (string + 1);


        // DRAW TO CACHE FIRST, THEN BITBLT TO SCREEN
        gglsurface tsurf = ggl_grob(newbmp);

        // CLEAR THE BITMAP FIRST
        ggl_rect(&tsurf,
                 0,
                 0,
                 numwidth - 1,
                 font->BitmapHeight - 1,
                 ggl_solid(PAL_STK_BG));
        DrawTextN(&tsurf,
                  0,
                  0,
                  charptr,
                  charptr + nchars,
                  font,
                  ggl_solid(PAL_STK_ITEMS));

        // AND ADD TO CACHE

        uiAddCacheEntry(object, newbmp, font);

        return newbmp;

    }
    else {
        // CAN'T CACHE, DRAW DIRECTLY
        return 0;
    }

}

// DRAW A BITMAP INTO THE SURFACE. MUST BE SYSTEM-DEFAULT BITMAP
void uiDrawBitmap(gglsurface * scr, word_p bmp, coord x, coord y)
{
    if(bmp && ISBITMAP(*bmp)) {
        // COPY IT TO DESTINATION
        gglsurface tsurf = ggl_grob(bmp);
        ggl_copy_at(scr, &tsurf, x, y, bmp[1], bmp[2]);
    }
    else {
        // DRAW DIRECTLY, SOMETHING WE COULDN'T RENDER

        word_p string = (word_p) invalid_string;

        // NOW PRINT THE STRING OBJECT
        int32_t nchars = rplStrSize(string);
        cstring charptr = (cstring) (string + 1);
        DrawTextN(scr,
                  x, y,
                  charptr,
                  charptr + nchars,
                  FONT_STACK,
                  ggl_solid(PAL_STK_ITEMS));
    }
}
