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


// INVALIDATE ALL CACHE ENTRIES
void uiClearRenderCache()
{
    halCacheEntry=0;
}

// ADD AN ENTRY TO THE CACHE
void uiAddCacheEntry(WORDPTR object,WORDPTR bitmap)
{
    if(GCFlags&GC_COMPLETED) {
        uiClearRenderCache();
        GCFlags=0;
    }

    halCacheContents[NEXT_ENTRY*2]=object;
    halCacheContents[NEXT_ENTRY*2+1]=bitmap;
    halCacheEntry=INC_NEXT_ENTRY;
    if(NEXT_ENTRY==0) halCacheEntry|=CACHE_FULL;
}



// USE AN ENTRY IN THE CACHE
WORDPTR uiFindCacheEntry(WORDPTR object)
{
    if(GCFlags&GC_COMPLETED) {
        uiClearRenderCache();
        GCFlags=0;
        return 0;
    }


    int k;
    int limit=(halCacheEntry&CACHE_FULL)? MAX_RENDERCACHE_ENTRIES:NEXT_ENTRY;

    limit*=2;

    for(k=0;k<limit;k+=2)
    {
        if(halCacheContents[k]==object) return halCacheContents[k+1];
    }
    return 0;
}


// ALLOCATE NEW BITMAP OBJECT, THIS IS HARDWARE DEPENDENT AS IT USES THE DEFAULT SCREEN COLOR MODE

WORDPTR uiAllocNewBitmap(BINT width, BINT height)
{

    BINT bits=DEFAULTBITSPERPIXEL*width*height;

    bits+=31;
    bits>>=5;

    WORDPTR newobj=rplAllocTempOb(bits+2);
    if(!newobj) return 0;
    newobj[0]=MKPROLOG(DOBITMAP+DEFAULTBITMAPMODE,bits+2);
    newobj[1]=width;
    newobj[2]=height;

    return newobj;
}





// RENDER AN OBJECT TO THE GIVEN DRAWSURFACE, USE CACHE IF POSSIBLE

void uiDrawObject(WORDPTR object,DRAWSURFACE *scr,UNIFONT *font)
{

    // FIRST, CHECK IF THE OBJECT IS IN THE CACHE

    WORDPTR bmp=uiRenderObject(object,font);

    if(bmp) {
        // COPY IT TO DESTINATION
           DRAWSURFACE tsurf;

           tsurf.addr=(int *)(bmp+3);
           tsurf.width=bmp[1];
           tsurf.clipx=0;
           tsurf.clipx2=bmp[1]-1;
           tsurf.clipy=0;
           tsurf.clipy2=bmp[2]-1;
           tsurf.x=0;
           tsurf.y=0;

           ggl_bitbltclip(scr,&tsurf,bmp[1],bmp[2]);

           return;
    }

        // DRAW DIRECTLY, DON'T CACHE SOMETHING WE COULDN'T RENDER

    WORDPTR string=(WORDPTR)invalid_string;

    // NOW PRINT THE STRING OBJECT
        BINT nchars=rplStrSize(string);
        BYTEPTR charptr=(BYTEPTR) (string+1);

        DrawTextN(scr->x,scr->y,(char *)charptr,(char *)charptr+nchars,font,15,scr);

}


// RENDER AN OBJECT TO A BITMAP, USE CACHE IF POSSIBLE

WORDPTR uiRenderObject(WORDPTR object,UNIFONT *font)
{

    // FIRST, CHECK IF THE OBJECT IS IN THE CACHE

    WORDPTR bmp=uiFindCacheEntry(object);

    if(bmp) return bmp;


    // OBJECT WAS NOT IN CACHE, RENDER IT AND ADD IT TO CACHE

        // TODO: CHANGE DECOMPILE INTO PROPER DISPLAY FUNCTION
        WORDPTR string;
        string=rplDecompile(object,0);

    if(!string) string=(WORDPTR)invalid_string;

    // NOW PRINT THE STRING OBJECT

        BINT nchars=rplStrSize(string);
        BYTEPTR charptr=(BYTEPTR) (string+1);
        BINT numwidth=StringWidthN((char *)charptr,(char *)charptr+nchars,font);

        ScratchPointer1=string;

        WORDPTR newbmp=uiAllocNewBitmap(numwidth,font->BitmapHeight);
        if(newbmp) {

                // RELOAD ALL POINTERS IN CASE THERE WAS A GC
                string=ScratchPointer1;
                charptr=(BYTEPTR) (string+1);


                // CLEAR THE BITMAP FIRST
                memsetw(newbmp+3,0,OBJSIZE(*newbmp)-2);

                // DRAW TO CACHE FIRST, THEN BITBLT TO SCREEN
                DRAWSURFACE tsurf;

                tsurf.addr=(int *) (newbmp+3);
                tsurf.width=numwidth;
                tsurf.clipx=0;
                tsurf.clipx2=numwidth-1;
                tsurf.clipy=0;
                tsurf.clipy2=font->BitmapHeight-1;
                tsurf.x=0;
                tsurf.y=0;

                DrawTextN(0,0,(char *)charptr,(char *)charptr+nchars,font,15,&tsurf);

                // AND ADD TO CACHE

                uiAddCacheEntry(object,newbmp);

                return newbmp;

        }
        else {
            // CAN'T CACHE, DRAW DIRECTLY
            return 0;
        }

}


// DRAW A BITMAP INTO THE SURFACE. MUST BE SYSTEM-DEFAULT BITMAP
void uiDrawBitmap(WORDPTR bmp,DRAWSURFACE *scr)
{
    if(bmp) {
    // COPY IT TO DESTINATION
       DRAWSURFACE tsurf;

       tsurf.addr=(int *)(bmp+3);
       tsurf.width=bmp[1];
       tsurf.clipx=0;
       tsurf.clipx2=bmp[1]-1;
       tsurf.clipy=0;
       tsurf.clipy2=bmp[2]-1;
       tsurf.x=0;
       tsurf.y=0;

       ggl_bitbltclip(scr,&tsurf,bmp[1],bmp[2]);
    }
    else {
        // DRAW DIRECTLY, SOMETHING WE COULDN'T RENDER

    WORDPTR string=(WORDPTR)invalid_string;

    // NOW PRINT THE STRING OBJECT
        BINT nchars=rplStrSize(string);
        BYTEPTR charptr=(BYTEPTR) (string+1);

        DrawTextN(scr->x,scr->y,(char *)charptr,(char *)charptr+nchars,halScreen.StackFont,15,scr);
    }
}
