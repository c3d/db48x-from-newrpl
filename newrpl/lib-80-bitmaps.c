/*
 * Copyright (c) 2016, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include "libraries.h"

#ifndef COMMANDS_ONLY_PASS
#include "cmdcodes.h"
#include "hal_api.h"
#include "newrpl.h"
#include "render.h"
#include "sysvars.h"
#include "ui.h"
#endif

// *****************************
// *** COMMON LIBRARY HEADER ***
// *****************************

// REPLACE THE NUMBER
#define LIBRARY_NUMBER  80

//@TITLE=Bitmaps

// LIST OF COMMANDS EXPORTED,
// INCLUDING INFORMATION FOR SYMBOLIC COMPILER
// IN THE CMD() FORM, THE COMMAND NAME AND ITS
// ENUM SYMBOL ARE IDENTICAL
// IN THE ECMD() FORM, THE ENUM SYMBOL AND THE
// COMMAND NAME TEXT ARE GIVEN SEPARATEDLY

#define COMMAND_LIST \
    ECMD(TOSYSBITMAP,"→SYSBITMAP",MKTOKENINFO(10,TITYPE_NOTALLOWED,1,2))

#define ERROR_LIST \
    ERR(BITMAPEXPECTED,0), \
    ERR(UNSUPPORTEDBITMAP,1), \
    ERR(INVALIDCHECKSUM,2), \
    ERR(UNEXPECTEDENDOFDATA,3)

// ADD MORE OPCODES HERE

// LIST ALL LIBRARY NUMBERS THIS LIBRARY WILL ATTACH TO
#define LIBRARY_ASSIGNED_NUMBERS \
    LIBRARY_NUMBER, \
    LIBRARY_NUMBER+1, \
    LIBRARY_NUMBER+2, \
    LIBRARY_NUMBER+3, \
    LIBRARY_NUMBER+4, \
    LIBRARY_NUMBER+5, \
    LIBRARY_NUMBER+6, \
    LIBRARY_NUMBER+7

// THIS HEADER DEFINES MANY COMMON MACROS FOR ALL LIBRARIES
#include "lib-header.h"

#ifndef COMMANDS_ONLY_PASS

// ************************************
// *** END OF COMMON LIBRARY HEADER ***
// ************************************

INCLUDE_ROMOBJECT(LIB_MSGTABLE);
INCLUDE_ROMOBJECT(LIB_HELPTABLE);
INCLUDE_ROMOBJECT(lib80_menu);

// EXTERNAL EXPORTED OBJECT TABLE
// UP TO 64 OBJECTS ALLOWED, NO MORE
const const word_p ROMPTR_TABLE[] = { (word_p) LIB_MSGTABLE,
                                      (word_p) LIB_HELPTABLE,
                                      (word_p) lib80_menu,

                                      0 };

const char *const bitmap_modes[] = {
    "MONO",
    "16GR",
    "256G",
    "64KC",
    "ARGB",
    "OTHR",
    "INVA",
    "INVA"
};

// CONVERT RGB 0-255 TO GRAY 0-255 PER BT.709 HDTV FORMULA FOR LUMINANCE

#define LUMINANCE(r,g,b) ((((r)*55+(g)*183+(b)*18)+128)>>8)
#define RGB5TO8(comp) (((int32_t)(comp)*2106)>>8)
#define RGB6TO8(comp) (((int32_t)(comp)*1036)>>8)

typedef struct
{
    // ADD OTHER INFO HERE
    gglsurface srf;
    int32_t npoints, ptalloc;
    CURVEPT points[1];

} BMP_RENDERSTATE;

#define RENDERSTATE_SIZE(npoints) ((sizeof(BMP_RENDERSTATE)+(npoints-1)*sizeof(((BMP_RENDERSTATE *)0)->points))>>2)

// CONVERT A BITMAP FROM ANY FORMAT INTO THE DEFAULT DISPLAY FORMAT

word_p rplBmpToDisplay(word_p bitmap)
{
    if(!ISBITMAP(*bitmap)) {
        rplError(ERR_BITMAPEXPECTED);
        return 0;
    }

    int32_t type = LIBNUM(*bitmap) & 7;

    if(type == DEFAULT_BITMAP_MODE)
        return bitmap;  // NO CONVERSION NEEEDED

    int32_t width = (int32_t) bitmap[1];
    int32_t height = (int32_t) bitmap[2];

    int32_t totalsize = (width * height * BITS_PER_PIXEL) + 31;

    totalsize >>= 5;    // BITMAP SIZE IN WORDS

    word_p newbmp = rplAllocTempOb(totalsize + 2);
    if(!newbmp)
        return 0;

    int32_t npixels = width * height;

    // THIS IS FOR THE 50G HARDWARE, BUT FUTURE-PROOF FOR 16-BIT COLOR DISPLAYS AS WELL

#if DEFAULT_BITMAP_MODE == BITMAP_RAW16G
    switch (type) {
    case BITMAP_RAWMONO:

    {
        byte_p destptr, srcptr;

        int32_t mask = 1, destmask = 0;
        int32_t pixel;

        srcptr = (byte_p) (bitmap + 3);
        destptr = (byte_p) (newbmp + 3);

        while(npixels) {

            // READ A PIXEL FROM SOURCE
            pixel = *srcptr & mask;
            // CONVERT TO PROPER FORMAT
            if(pixel)
                pixel = 0xf;
            else
                pixel = 0;

            // WRITE TO DESTINATION
            if(!destmask)
                *destptr = (BYTE) pixel;
            else
                *destptr |= (BYTE) (pixel << 4);

            //INCREASE SOURCE POINTER
            mask <<= 1;
            if(mask > 128) {
                ++srcptr;
                mask >>= 8;
            }

            // INCREASE DEST PTR
            destmask ^= 1;
            if(!destmask)
                ++destptr;

            --npixels;
        }
        break;

    }

    case BITMAP_RAW16G:
        break;
    case BITMAP_RAW256G:

    {

        byte_p destptr, srcptr;

        int32_t destmask = 0;
        int32_t pixel;

        srcptr = (byte_p) (bitmap + 3);
        destptr = (byte_p) (newbmp + 3);

        while(npixels) {

            // READ A PIXEL FROM SOURCE
            pixel = *srcptr;
            // CONVERT TO PROPER FORMAT
            pixel = ((255 - pixel) + 128) >> 4;

            // WRITE TO DESTINATION
            if(!destmask)
                *destptr = (BYTE) pixel;
            else
                *destptr |= (BYTE) (pixel << 4);

            //INCREASE SOURCE POINTER
            ++srcptr;

            // INCREASE DEST PTR
            destmask ^= 1;
            if(!destmask)
                ++destptr;

            --npixels;
        }
        break;
    }
    case BITMAP_RAW64KC:

    {

        byte_p destptr, srcptr;

        int32_t destmask = 0;
        int32_t pixel;

        srcptr = (byte_p) (bitmap + 3);
        destptr = (byte_p) (newbmp + 3);

        while(npixels) {

            // READ A PIXEL FROM SOURCE
            pixel = srcptr[0] + 256 * srcptr[1];

            // CONVERT TO PROPER FORMAT
            pixel = LUMINANCE(RGB5TO8(pixel >> 11),
                    RGB6TO8((pixel >> 5) & 0x3f), RGB5TO8(pixel & 0x1f));
            pixel = ((255 - pixel) + 128) >> 4;

            // WRITE TO DESTINATION
            if(!destmask)
                *destptr = (BYTE) pixel;
            else
                *destptr |= (BYTE) (pixel << 4);

            //INCREASE SOURCE POINTER
            srcptr += 2;

            // INCREASE DEST PTR
            destmask ^= 1;
            if(!destmask)
                ++destptr;

            --npixels;
        }
        break;
    }

    case BITMAP_RAWARGB:

    {

        byte_p destptr, srcptr;

        int32_t destmask = 0;
        int32_t pixel;

        srcptr = (byte_p) (bitmap + 3);
        destptr = (byte_p) (newbmp + 3);

        while(npixels) {

            // READ A PIXEL FROM SOURCE
            //pixel=srcptr[0]+256*srcptr[1];

            // CONVERT TO PROPER FORMAT
            pixel = LUMINANCE(srcptr[2], srcptr[1], srcptr[0]);
            pixel = ((255 - pixel) + 128) >> 4;

            // WRITE TO DESTINATION
            if(!destmask)
                *destptr = (BYTE) pixel;
            else
                *destptr |= (BYTE) (pixel << 4);

            //INCREASE SOURCE POINTER
            srcptr += 4;

            // INCREASE DEST PTR
            destmask ^= 1;
            if(!destmask)
                ++destptr;

            --npixels;
        }
        break;
    }

    }
#endif

#if DEFAULT_BITMAP_MODE == BITMAP_RAW64KC
    switch (type) {
    case BITMAP_RAWMONO:

    {
        uint16_p destptr;
        byte_p srcptr;

        int32_t mask = 1;
        int32_t pixel;

        srcptr = (byte_p) (bitmap + 3);
        destptr = (uint16_p) (newbmp + 3);

        while(npixels) {

            // READ A PIXEL FROM SOURCE
            pixel = *srcptr & mask;

            // Convert to proper format
            // Assume monochrome graphics are black on white
            // (Same as 50g or a fax)
            if(pixel)
                pixel = ggl_rgb16(0,0,0).value;
            else
                pixel = ggl_rgb16(255,255,255).value;

            // Write to destination
            *destptr = (uint16_t) pixel;

            //INCREASE SOURCE POINTER
            mask <<= 1;
            if(mask > 128) {
                ++srcptr;
                mask >>= 8;
            }

            // INCREASE DEST PTR
                ++destptr;
            --npixels;
        }
        break;

    }

    case BITMAP_RAW16G:
    {
        uint16_p destptr;
        byte_p srcptr;

        int32_t mask = 0xf,rot=4;
        int32_t pixel;

        srcptr = (byte_p) (bitmap + 3);
        destptr = (uint16_p) (newbmp + 3);

        while(npixels) {

            // READ A PIXEL FROM SOURCE
            pixel = 0xf0 - ((*srcptr & mask) << rot);
            if(pixel&0x80) pixel|=0xf;

            // Convert to proper format
            // Assume gray16 graphics are black on white (same as 50g bitmaps)
            pixel = ggl_rgb16(pixel,pixel,pixel).value;

            // WRITE TO DESTINATION
            *destptr = (uint16_t) pixel;

            //INCREASE SOURCE POINTER
            mask <<= 4;
            rot^=4;
            if(mask > 0xf0) {
                ++srcptr;
                mask >>= 8;
            }

            // INCREASE DEST PTR
                ++destptr;
            --npixels;
        }
        break;

    }
    case BITMAP_RAW256G:

    {

        uint16_p destptr;
        byte_p srcptr;

        int32_t pixel;

        srcptr = (byte_p) (bitmap + 3);
        destptr = (uint16_p) (newbmp + 3);

        while(npixels) {

            // READ A PIXEL FROM SOURCE
            pixel = *srcptr;
            // CONVERT TO PROPER FORMAT
            color16_t pixel16 = ggl_rgb16(pixel,pixel,pixel);

            // WRITE TO DESTINATION
            *destptr = pixel16.value;

            //INCREASE SOURCE POINTER
            ++srcptr;

            ++destptr;

            --npixels;
        }
        break;
    }
    case BITMAP_RAW64KC:

    {
        break;
    }

    case BITMAP_RAWARGB:

    {

        uint16_p destptr;
        word_p srcptr;

        WORD pixel;

        srcptr = (word_p) (bitmap + 3);
        destptr = (uint16_p) (newbmp + 3);

        while(npixels) {

            // READ A PIXEL FROM SOURCE
            pixel=*srcptr;

            // CONVERT TO PROPER FORMAT
            pixel = ggl_rgb16((pixel >> 16) & 0xff,
                              (pixel >> 8) & 0xff,
                              (pixel) &0xff)
                        .value;

            *destptr = (uint16_t) pixel;

            //INCREASE SOURCE POINTER
            srcptr++;

            // INCREASE DEST PTR
            ++destptr;

            --npixels;
        }
        break;
    }

    }
#endif

    // ALL PIXELS CONVERTED

    return newbmp;

}

// CREATE A BITMAP OF THE REQUESTED SIZE AND TYPE

word_p rplBmpCreate(int32_t type, int32_t width, int32_t height, int32_t clear)
{
    int32_t bitspixel;

    switch (type) {
    case BITMAP_RAWMONO:
        bitspixel = 1;
        break;
    case BITMAP_RAW16G:
        bitspixel = 4;
        break;
    case BITMAP_RAW256G:
        bitspixel = 8;
        break;
    case BITMAP_RAW64KC:
        bitspixel = 16;
        break;
    case BITMAP_RAWARGB:
        bitspixel = 32;
        break;
    default:
        rplError(ERR_UNSUPPORTEDBITMAP);
        return 0;
    }

    int32_t totalsize = (width * height * bitspixel) + 31;

    totalsize >>= 5;    // BITMAP SIZE IN WORDS

    word_p newbmp = rplAllocTempOb(totalsize + 2);
    if(!newbmp)
        return 0;

    newbmp[0] = MKPROLOG(DOBITMAP + type, totalsize + 2);
    newbmp[1] = width;
    newbmp[2] = height;
    if(clear)
        memsetw(newbmp + 3, 0, totalsize);      // CLEAR THE BITMAP

    return newbmp;

}

// QUICKLY RETRIEVE THE RENDERER STATUS BEFORE EACH COMMAND IS PROCESSED
// RECEIVES rstatus LIST IN STACK LEVEL 2
void rplBMPRenderUdateState(word_p * rstatusptr,
        BMP_RENDERSTATE ** renderstptr)
{
    word_p rstatus = rplPeekData(2);
    BMP_RENDERSTATE *renderst;

    *rstatusptr = rstatus;      // UPDATE rstatus LIST
    renderst = (BMP_RENDERSTATE *) (PERSISTPTR(rstatus) + 1);
    *renderstptr = renderst;    // UPDATE render STATUS STRUCTURE

    renderst->srf.pixels = (pixword *) (ROBJPTR(rstatus) + 3);       // UPDATE BITMAP ADDRESS IN CASE IT MOVED!

}

// ALLOCATE MORE SPACE FOR PATH POINTS IN THE BMP_RENDERSTATE STRUCTURE
// UPDATE STACK LEVELS 1 AND 2 AS NEEDED IF THINGS MOVE
void rplBMPRenderAllocPoint(word_p * rstatusptr,
        BMP_RENDERSTATE ** renderstptr, int32_t npoints)
{
    word_p rstatus = *rstatusptr;
    BMP_RENDERSTATE *renderst = *renderstptr;

    if(renderst->ptalloc >= renderst->npoints + npoints)
        return; // NO NEED TO ALLOCATE MORE MEMORY

    int32_t need = renderst->npoints + npoints;

    need = (need + 7) / 8;      // ALLOCATE IN BLOCKS OF 8 POINTS FOR SPEED

    int32_t wordsneed = need * sizeof(renderst->points) * 2;       // TOTAL WORDS NEEDED AT THE STRUCTURE

    wordsneed +=
            rplObjSize(rstatus) - 1 -
            sizeof(renderst->points) / 4 * renderst->ptalloc;

    word_p newobj = rplAllocTempOb(wordsneed);
    if(!newobj)
        return;

    rstatus = rplPeekData(2);   // RELOAD IN CASE IT MOVED

    memmovew(newobj, rstatus, OBJSIZE(*rstatus));

    // STRETCH THE BINDATA OBJECT
    word_p libdata = PERSISTPTR(newobj);
    *libdata = MKPROLOG(DOBINDATA, RENDERSTATE_SIZE(need * 8));
    renderst = (BMP_RENDERSTATE *) (libdata + 1);
    libdata = rplSkipOb(libdata);
    *libdata = CMD_ENDLIST;

    // STRETCH THE CONTAINER LIST OBJECT
    *newobj = MKPROLOG(DOLIST, libdata - newobj);

    // UPDATE ALLOCATION COUNT
    renderst->ptalloc = need * 8;

    // UPDATE POINTERS
    *rstatusptr = newobj;       // UPDATE rstatus LIST
    *renderstptr = renderst;    // UPDATE render STATUS STRUCTURE

    renderst->srf.pixels = (pixword *) (ROBJPTR(rstatus) + 3);       // UPDATE BITMAP ADDRESS IN CASE IT MOVED!

    rplOverwriteData(2, newobj);

    return;
}

void LIB_HANDLER()
{
    if(ISPROLOG(CurOpcode)) {
        // JUST PUSH THE OBJECT ON THE STACK
        rplPushData(IPtr);
        return;
    }

    if((OPCODE(CurOpcode) > CMD_PLTBASE)
            && (OPCODE(CurOpcode) < MIN_OVERLOAD_OPCODE)) {
        // SAME LIBRARY CAN WORK AS A RENDERER FOR PLOTS

        // PLOT RENDERER RECIEVES ON STACK LEVEL 1 = CURRENT PLOT OBJECT BEING PROCESSED
        // STACK LEVEL 2 = RENDERING STATUS LIST OBJECT
        // PLOT RENDERER MUST NOT LEAVE ANYTHING ON THE STACK OR REMOVE ANY PARAMETERS

        switch (OPCODE(CurOpcode)) {

        case CMD_PLTRESET:
        {
            // RESET ENGINE, NOTHING TO DO IN THIS CASE
            // THE ABSENCE OF INVALID OPCODE ERROR IS USED TO DETECT A VALID RENDERER
            return;
        }
        case CMD_PLTRENDERSIZE:
        {
            // USE THE INFORMATION IN THE RENDERER STATUS, NOT ARGUMENTS
            word_p rstatus = rplPeekData(2);

            // NO CHECKS, RENDERER HAS TO BE CALLED WITH PROPER ARGUMENTS
            int64_t w = (*WIDTHPTR(rstatus)) >> 24;
            int64_t h = (*HEIGHTPTR(rstatus)) >> 24;

            int32_t bitmaptype = LIBNUM(CurOpcode) - LIBRARY_NUMBER;

            // CREATE A GROB THE RIGHT SIZE INSIDE rstatus AND APPEND A RENDER STATUS STRUCTURE

            int32_t wordsneeded = ROBJPTR(rstatus) - rstatus;      // INCLUDES AN EXTRA WORD FOR CMD_ENDLIST
            wordsneeded += RENDERSTATE_SIZE(1) + 1;

            int32_t bitspixel;

            switch (bitmaptype) {
            case BITMAP_RAWMONO:
                bitspixel = 1;
                break;
            case BITMAP_RAW16G:
                bitspixel = 4;
                break;
            case BITMAP_RAW256G:
                bitspixel = 8;
                break;
            case BITMAP_RAW64KC:
                bitspixel = 16;
                break;
            case BITMAP_RAWARGB:
                bitspixel = 32;
                break;
            default:
                rplError(ERR_UNSUPPORTEDBITMAP);
                return;
            }

            int32_t totalsize = (w * h * bitspixel) + 31;

            totalsize >>= 5;    // BITMAP SIZE IN WORDS

            wordsneeded += totalsize + 3;

            word_p newrst = rplAllocTempOb(wordsneeded), ptr;

            if(!newrst)
                return;

            memmovew(newrst, rstatus, ROBJPTR(rstatus) - rstatus);
            ptr = ROBJPTR(newrst);
            ptr[0] = MKPROLOG(DOBITMAP + bitmaptype, totalsize + 2);
            ptr[1] = w;
            ptr[2] = h;
            memsetw(ptr + 3, 0, totalsize);     // CLEAR NEW BITMAP BACKGROUND
            ptr = PERSISTPTR(newrst);
            ptr[0] = MKPROLOG(DOBINDATA, RENDERSTATE_SIZE(1));

            // INITIALIZE THE RENDERING STRUCTURE
            BMP_RENDERSTATE *renderst = (BMP_RENDERSTATE *) (ptr + 1);

            // INITIALIZE THE RENDERING INTERNAL STATUS

            renderst->ptalloc = 1;
            renderst->npoints = 0;
            renderst->srf.left = 0;
            renderst->srf.right = w - 1;
            renderst->srf.top = 0;
            renderst->srf.bottom = h - 1;
            renderst->srf.pixels = (pixword *) (ROBJPTR(rstatus) + 3);
            renderst->srf.width = w;
            renderst->points[0].type = 0;
            renderst->points[0].x = 0;
            renderst->points[0].y = 0;
            ptr = rplSkipOb(ptr);
            ptr[0] = CMD_ENDLIST;       // CLOSE THE LIST

            newrst[0] = MKPROLOG(DOLIST, wordsneeded);

            rplOverwriteData(1, newrst);

            return;

        }

        case CMD_PLTBASE + PLT_MOVETO:
        {
            // UPDATE RENDER STATUS
            word_p rstatus;
            BMP_RENDERSTATE *renderst;
            rplBMPRenderUdateState(&rstatus, &renderst);

            renderst->npoints = 0;      // END ANY PREVIOUS PATH

            *CXPTR(rstatus) = *ARG1PTR(rstatus);
            *CYPTR(rstatus) = *ARG2PTR(rstatus);
            return;

        }
        case CMD_PLTBASE + PLT_LINETO:
        {
            // UPDATE RENDER STATUS
            word_p rstatus;
            BMP_RENDERSTATE *renderst;
            rplBMPRenderUdateState(&rstatus, &renderst);

            if(renderst->npoints < 1) {
                // ADD THE STARTING POINT, STORAGE IS GUARANTEED TO EXIST!

                renderst->points[0].type = TYPE_STARTPOINT;     // STARTING POINT
                renderst->points[0].x = *CXPTR(rstatus);
                renderst->points[0].y = *CYPTR(rstatus);
                renderst->npoints = 1;
            }

            rplBMPRenderAllocPoint(&rstatus, &renderst, 1);
            if(Exceptions)
                return; // RETURN IF OUT OF MEMORY

            renderst->points[renderst->npoints].type = TYPE_LINE;       // STARTING POINT
            renderst->points[renderst->npoints].x = *ARG1PTR(rstatus);
            renderst->points[renderst->npoints].y = *ARG2PTR(rstatus);
            renderst->npoints++;

            return;

        }

        case CMD_PLTBASE + PLT_STROKE:
        {
            // UPDATE RENDER STATUS
            word_p rstatus;
            BMP_RENDERSTATE *renderst;
            rplBMPRenderUdateState(&rstatus, &renderst);
            // DRAW THE PERIMETER OF THE PATH

            // TODO: ACTUALLY DRAW THE LINE TO THE BITMAP
            // USING A REAL SCAN-LINE RENDERER
            int32_t k;
            for(k = 0; k < renderst->npoints; ++k) {
                ggl_cliphline(&(renderst->srf), renderst->points[k].y >> 24,
                              renderst->points[k].x, renderst->points[k].x, PAL_GRAY15);
            }

            return;

        }

        }
        // RETURN QUIETLY ON UNKNOWN OPERATIONS, JUST DO NOTHING.
        return;
    }

    switch (OPCODE(CurOpcode)) {

    case OVR_SAME:
        // COMPARE AS PLAIN OBJECTS, THIS INCLUDES SIMPLE COMMANDS IN THIS LIBRARY
    {
        int32_t same = rplCompareObjects(rplPeekData(1), rplPeekData(2));
        rplDropData(2);
        if(same)
            rplPushTrue();
        else
            rplPushFalse();
        return;
    }

    case OVR_ISTRUE:
    {
        if(ISPROLOG(*rplPeekData(1))) {
            word_p dataptr=rplPeekData(1);
            int32_t size=OBJSIZE(*dataptr)-2;
            dataptr+=2;
            int32_t iszero=1;
            while(size--) if(*dataptr++) { iszero=0; break; }
            rplOverwriteData(1, (iszero)? (word_p)zero_bint:(word_p) one_bint);
        }
        else
            rplOverwriteData(1, (word_p) one_bint);
        return;
    }

    case OVR_FUNCEVAL:
    case OVR_EVAL:
    case OVR_EVAL1:
    case OVR_XEQ:
        // ALSO EXECUTE THE OBJECT
        if(!ISPROLOG(*rplPeekData(1))) {
            // EXECUTE THE COMMAND BY CALLING THE HANDLER DIRECTLY
            WORD saveOpcode = CurOpcode;
            CurOpcode = *rplPopData();
            // RECURSIVE CALL
            LIB_HANDLER();
            CurOpcode = saveOpcode;
            return;
        }

        return;

        // STANDARIZED OPCODES:
        // --------------------
        // LIBRARIES ARE FORCED TO ALWAYS HANDLE THE STANDARD OPCODES

    case OPCODE_COMPILE:
        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  enum CompileErrors
    {
        if((TokenLen == 10)
                && (!utf8ncmp2((char *)TokenStart, (char *)BlankStart,
                        "BITMAPDATA", 10))) {

            ScratchPointer4 = CompileEnd;
            rplCompileAppend(MKPROLOG(LIBRARY_NUMBER, 0));
            RetNum = OK_NEEDMORE;
            return;
        }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF COMPILATION OF STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS CUSTOM OPCODES

        libCompileCmds(LIBRARY_NUMBER, (char **)LIB_NAMES, NULL,
                LIB_NUMBEROFCMDS);
        return;
    }
    case OPCODE_COMPILECONT:
    {
        if((LIBNUM(*ScratchPointer4) & ~7) != LIBRARY_NUMBER) {
            // SOMETHING BAD HAPPENED, THERE'S NO BMPDATA HEADER
            RetNum = ERR_SYNTAX;
            return;
        }

        if(!(*ScratchPointer4 & 0x10000)) {
            // NEED TO INPUT THE BITMAP TYPE IN 4-LETTERS: MONO,16GR,256G,64KC,ARGB,OTHR
            if(((int32_t) TokenLen != (byte_p) BlankStart - (byte_p) TokenStart)
                    || (TokenLen != 4)) {
                // THERE'S UNICODE CHARACTERS IN BETWEEN, THAT MAKES IT AN INVALID STRING
                // OR THERE'S NOT 4 CHARACTERS
                rplError(ERR_UNSUPPORTEDBITMAP);
                RetNum = ERR_SYNTAX;
                return;
            }
            if(!utf8ncmp2((char *)TokenStart, (char *)BlankStart, "MONO", 4))
                *ScratchPointer4 |= BITMAP_RAWMONO | 0x10000;
            else if(!utf8ncmp2((char *)TokenStart, (char *)BlankStart, "16GR",
                        4))
                *ScratchPointer4 |= BITMAP_RAW16G | 0x10000;
            else if(!utf8ncmp2((char *)TokenStart, (char *)BlankStart, "256G",
                        4))
                *ScratchPointer4 |= BITMAP_RAW256G | 0x10000;
            else if(!utf8ncmp2((char *)TokenStart, (char *)BlankStart, "64KC",
                        4))
                *ScratchPointer4 |= BITMAP_RAW64KC | 0x10000;
            else if(!utf8ncmp2((char *)TokenStart, (char *)BlankStart, "ARGB",
                        4))
                *ScratchPointer4 |= BITMAP_RAWARGB | 0x10000;
            else if(!utf8ncmp2((char *)TokenStart, (char *)BlankStart, "OTHR",
                        4))
                *ScratchPointer4 |= BITMAP_EXTERNAL | 0x10000;
            else {
                rplError(ERR_UNSUPPORTEDBITMAP);
                RetNum = ERR_SYNTAX;
                return;
            }
            RetNum = OK_NEEDMORE;
            return;
        }

        // HERE WE ALREADY HAVE THE TYPE OF BITMAP

        if(!(*ScratchPointer4 & 0x20000)) {
            // NEED TO CAPTURE THE WIDTH AND HEIGHT AS INTEGERS
            WORD value = 0;
            int32_t digit;
            byte_p ptr = (byte_p) TokenStart;

            while(ptr < (byte_p) BlankStart) {
                if((*ptr >= '0') && (*ptr <= '9'))
                    digit = *ptr - '0';
                else {
                    RetNum = ERR_SYNTAX;
                    return;
                }
                value *= 10;
                value += digit;
                ++ptr;
            }

            rplCompileAppend(value);

            // IF THERE WERE 2 NUMBERS ALREADY, THEN MARK THE SIZE AS DONE
            if(CompileEnd - ScratchPointer4 >= 3)
                *ScratchPointer4 |= 0x20000;
            RetNum = OK_NEEDMORE;
            return;
        }

        // HERE WE ALREADY HAVE THE BITMAP TYPE, WIDTH AND HEIGHT

        // WE HAVE A SIZE
        WORD totalsize;

        if((*ScratchPointer4 & 0xff) == BITMAP_EXTERNAL) {
            // THE SIZE IS IN THE FIRST WORD, GET A MINIMAL OF 1 WORDS
            totalsize = CompileEnd - ScratchPointer4 - 3;
            if(LIBNUM(*ScratchPointer4) & 1)
                totalsize -= 2;
            if(totalsize >= 1) {
                // WE ALREADY RECOVERED THE SIZE OF THE OBJECT
                totalsize = ScratchPointer4[3];
            }
            else
                totalsize = 0xffffffff;
        }
        else {
            totalsize = ScratchPointer4[1] * ScratchPointer4[2];
            switch (*ScratchPointer4 & 0xff) {
            case BITMAP_RAWMONO:
                break;
            case BITMAP_RAW16G:
                totalsize *= 4;
                break;
            case BITMAP_RAW256G:
                totalsize *= 8;
                break;
            case BITMAP_RAW64KC:
                totalsize *= 16;
                break;
            case BITMAP_RAWARGB:
            default:
                totalsize *= 32;
                break;
            }
            totalsize += 31;
            totalsize >>= 5;
        }

        // DO WE NEED ANY MORE DATA?

        byte_p ptr = (byte_p) TokenStart;

        WORD value = 0;
        WORD checksum = 0;
        int32_t ndigits = 0;
        int32_t dig;

        if(LIBNUM(*ScratchPointer4) & 1) {
            // CONTINUE WHERE WE LEFT OFF
            --CompileEnd;
            ndigits = (*CompileEnd) & 0xffff;
            checksum = (*CompileEnd) >> 16;
            --CompileEnd;
            value = *CompileEnd;
            *ScratchPointer4 &= ~0x00100000;
        }

        do {
            if((*ptr >= '0') && (*ptr <= '9'))
                dig = (*ptr + 4);
            else if((*ptr >= 'A') && (*ptr <= 'Z'))
                dig = (*ptr - 65);
            else if((*ptr >= 'a') && (*ptr <= 'z'))
                dig = (*ptr - 71);
            else if(*ptr == '#')
                dig = 62;
            else if(*ptr == '$')
                dig = 63;
            else {
                // INVALID CHARACTER!
                RetNum = ERR_SYNTAX;
                return;
            }

            // STILL NEED MORE WORDS, KEEP COMPILING
            if(ndigits == 5) {
                value <<= 2;
                value |= dig & 3;
                checksum += dig & 3;
                if((checksum & 0xf) != ((dig >> 2) & 0xf)) {
                    rplError(ERR_INVALIDCHECKSUM);
                    RetNum = ERR_INVALID;
                    return;
                }
                // CHECKSUM PASSED, IT'S A VALID WORD
                rplCompileAppend(value);
                value = 0;
                ndigits = 0;
                checksum = 0;
            }
            else {
                value <<= 6;
                value |= dig;
                checksum += (dig & 3) + ((dig >> 2) & 3) + ((dig >> 4) & 3);
                ++ndigits;
            }
            ++ptr;
        }
        while(ptr != (byte_p) BlankStart);

        if(ndigits) {
            // INCOMPLETE WORD, PREPARE FOR RESUME ON NEXT TOKEN
            rplCompileAppend(value);
            rplCompileAppend(ndigits | (checksum << 16));
            *ScratchPointer4 |= 0x00100000;
        }
        else {
            if((CompileEnd - ScratchPointer4 - 3) == totalsize) {
                //   DONE!  FIX THE PROLOG WITH THE RIGHT LIBRARY NUMBER AND SIZE
                *ScratchPointer4 =
                        MKPROLOG(DOBITMAP + (*ScratchPointer4 & 0xff),
                        totalsize + 2);
                RetNum = OK_CONTINUE;
                return;
            }
            RetNum = OK_NEEDMORE;
            return;
        }

        // END OF TOKEN, NEED MORE!
        RetNum = OK_NEEDMORE;
        return;

    }
    case OPCODE_DECOMPEDIT:

    case OPCODE_DECOMPILE:
        // DECOMPILE RECEIVES:
        // DecompileObject = Ptr to prolog of object to decompile
        // DecompStringEnd = Ptr to the end of decompile string

        //DECOMPILE RETURNS
        // RetNum =  enum DecompileErrors
        if(ISPROLOG(*DecompileObject)) {
            // DECOMPILE BITMAP

            rplDecompAppendString((byte_p) "BITMAPDATA ");

            // TYPE
            rplDecompAppendString((byte_p)
                    bitmap_modes[LIBNUM(*DecompileObject) & 7]);

            rplDecompAppendChar(' ');

            // SIZE
            BYTE buffer[50];
            int32_t len =
                    rplIntToString((int64_t) DecompileObject[1], DECint32_t, buffer,
                    buffer + 50);

            rplDecompAppendString2(buffer, len);

            rplDecompAppendChar(' ');

            len = rplIntToString((int64_t) DecompileObject[2], DECint32_t, buffer,
                    buffer + 50);

            rplDecompAppendString2(buffer, len);

            rplDecompAppendChar(' ');

            int32_t size = OBJSIZE(*DecompileObject) - 2;

            // OUTPUT THE DATA BY WORDS, WITH FOLLOWING ENCODING:
            // 32-BIT WORDS GO ENCODED IN 6 TEXT CHARACTERS
            // EACH CHARACTER CARRIES 6-BITS IN BASE64 ENCONDING
            // MOST SIGNIFICANT 6-BIT PACKET GOES FIRST
            // LAST PACKET HAS 2 LSB BITS TO COMPLETE THE 32-BIT WORDS
            // AND 4-BIT CHECKSUM. THE CHECKSUM IS THE SUM OF THE (16) 2-BIT PACKS IN THE WORD, MODULO 15

            BYTE encoder[7];

            encoder[6] = 0;

            word_p ptr = DecompileObject + 3;
            int32_t nwords = 0;

            while(size) {
                // ENCODE THE 6 CHARACTERS
                int k;
                int32_t chksum = 0;
                for(k = 0; k < 5; ++k) {
                    encoder[k] = ((*ptr) >> (26 - 6 * k)) & 0x3f;
                    chksum +=
                            (encoder[k] & 3) + ((encoder[k] >> 2) & 3) +
                            ((encoder[k] >> 4) & 3);
                }
                encoder[5] = (*ptr) & 3;
                chksum += *ptr & 3;
                encoder[5] |= (chksum & 0xf) << 2;

                // NOW CONVERT TO BASE64
                for(k = 0; k < 6; ++k) {
                    if(encoder[k] < 26)
                        encoder[k] += 65;
                    else if(encoder[k] < 52)
                        encoder[k] += 71;
                    else if(encoder[k] < 62)
                        encoder[k] -= 4;
                    else if(encoder[k] == 62)
                        encoder[k] = '#';
                    else
                        encoder[k] = '$';
                }

                ScratchPointer1 = ptr;
                rplDecompAppendString(encoder);
                if(Exceptions) {
                    RetNum = ERR_INVALID;
                    return;
                }
                ptr = ScratchPointer1;

                ++ptr;

                ++nwords;
                if(nwords == 8) {
                    rplDecompAppendChar(' ');
                    nwords = 0;
                }

                --size;

            }

            RetNum = OK_CONTINUE;
            return;

        }

        // THIS STANDARD FUNCTION WILL TAKE CARE OF DECOMPILING STANDARD COMMANDS GIVEN IN THE LIST
        // NO NEED TO CHANGE THIS UNLESS THERE ARE CUSTOM OPCODES
        libDecompileCmds((char **)LIB_NAMES, NULL, LIB_NUMBEROFCMDS);
        return;
    case OPCODE_VALIDATE:
        // VALIDATE RECEIVES OPCODES COMPILED BY OTHER LIBRARIES, TO BE INCLUDED WITHIN A COMPOSITE OWNED BY
        // THIS LIBRARY. EVERY COMPOSITE HAS TO EVALUATE IF THE OBJECT BEING COMPILED IS ALLOWED INSIDE THIS
        // COMPOSITE OR NOT. FOR EXAMPLE, A REAL MATRIX SHOULD ONLY ALLOW REAL NUMBERS INSIDE, ANY OTHER
        // OPCODES SHOULD BE REJECTED AND AN ERROR THROWN.
        // Library receives:
        // CurrentConstruct = SET TO THE CURRENT ACTIVE CONSTRUCT TYPE
        // LastCompiledObject = POINTER TO THE LAST OBJECT THAT WAS COMPILED, THAT NEEDS TO BE VERIFIED

        // VALIDATE RETURNS:
        // RetNum =  OK_CONTINUE IF THE OBJECT IS ACCEPTED, ERR_INVALID IF NOT.

        RetNum = OK_CONTINUE;
        return;

    case OPCODE_PROBETOKEN:
        // PROBETOKEN FINDS A VALID WORD AT THE BEGINNING OF THE GIVEN TOKEN AND RETURNS
        // INFORMATION ABOUT IT. THIS OPCODE IS MANDATORY

        // COMPILE RECEIVES:
        // TokenStart = token string
        // TokenLen = token length
        // BlankStart = token blanks afterwards
        // BlanksLen = blanks length
        // CurrentConstruct = Opcode of current construct/WORD of current composite

        // COMPILE RETURNS:
        // RetNum =  OK_TOKENINFO | MKTOKENINFO(...) WITH THE INFORMATION ABOUT THE CURRENT TOKEN
        // OR RetNum = ERR_NOTMINE IF NO TOKEN WAS FOUND
    {
        libProbeCmds((char **)LIB_NAMES, (int32_t *) LIB_TOKENINFO,
                LIB_NUMBEROFCMDS);

        return;
    }

    case OPCODE_GETINFO:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL COMMAND OR OBJECT IN ObjectPTR
        // NEEDS TO RETURN INFORMATION ABOUT THE TYPE:
        // IN RetNum: RETURN THE MKTOKENINFO() DATA FOR THE SYMBOLIC COMPILER AND CAS
        // IN DecompHints: RETURN SOME HINTS FOR THE DECOMPILER TO DO CODE BEAUTIFICATION (TO BE DETERMINED)
        // IN TypeInfo: RETURN TYPE INFORMATION FOR THE TYPE COMMAND
        //             TypeInfo: TTTTFF WHERE TTTT = MAIN TYPE * 100 (NORMALLY THE MAIN LIBRARY NUMBER)
        //                                FF = 2 DECIMAL DIGITS FOR THE SUBTYPE OR FLAGS (VARIES DEPENDING ON LIBRARY)
        //             THE TYPE COMMAND WILL RETURN A REAL NUMBER TypeInfo/100
        // FOR NUMBERS: TYPE=10 (REALS), SUBTYPES = .01 = APPROX., .02 = INTEGER, .03 = APPROX. INTEGER
        // .12 =  BINARY INTEGER, .22 = DECIMAL INT., .32 = OCTAL int32_t, .42 = HEX INTEGER

        if(ISPROLOG(*ObjectPTR)) {
            TypeInfo = LIBRARY_NUMBER * 100;
            DecompHints = 0;
            RetNum = OK_TOKENINFO | MKTOKENINFO(0, TITYPE_NOTALLOWED, 0, 1);
        }
        else {
            TypeInfo = 0;       // ALL COMMANDS ARE TYPE 0
            DecompHints = 0;
            libGetInfo2(*ObjectPTR, (char **)LIB_NAMES, (int32_t *) LIB_TOKENINFO,
                    LIB_NUMBEROFCMDS);
        }
        return;

    case OPCODE_GETROMID:
        // THIS OPCODE RECEIVES A POINTER TO AN RPL OBJECT IN ROM, EXPORTED BY THIS LIBRARY
        // AND CONVERTS IT TO A UNIQUE ID FOR BACKUP PURPOSES
        // ObjectPTR = POINTER TO ROM OBJECT
        // LIBBRARY RETURNS: ObjectID=new ID, ObjectIDHash=hash, RetNum=OK_CONTINUE
        // OR RetNum=ERR_NOTMINE IF THE OBJECT IS NOT RECOGNIZED

        libGetRomptrID(LIBRARY_NUMBER, (word_p *) ROMPTR_TABLE, ObjectPTR);
        return;
    case OPCODE_ROMID2PTR:
        // THIS OPCODE GETS A UNIQUE ID AND MUST RETURN A POINTER TO THE OBJECT IN ROM
        // ObjectID = ID, ObjectIDHash=hash
        // LIBRARY RETURNS: ObjectPTR = POINTER TO THE OBJECT, AND RetNum=OK_CONTINUE
        // OR RetNum= ERR_NOTMINE;

        libGetPTRFromID((word_p *) ROMPTR_TABLE, ObjectID, ObjectIDHash);
        return;

    case OPCODE_CHECKOBJ:
        // THIS OPCODE RECEIVES A POINTER TO AN OBJECT FROM THIS LIBRARY AND MUST
        // VERIFY IF THE OBJECT IS PROPERLY FORMED AND VALID
        // ObjectPTR = POINTER TO THE OBJECT TO CHECK
        // LIBRARY MUST RETURN: RetNum=OK_CONTINUE IF OBJECT IS VALID OR RetNum=ERR_INVALID IF IT'S INVALID
        if(ISPROLOG(*ObjectPTR)) {
            RetNum = ERR_INVALID;
            return;
        }

        RetNum = OK_CONTINUE;
        return;

    case OPCODE_AUTOCOMPNEXT:
        libAutoCompleteNext(LIBRARY_NUMBER, (char **)LIB_NAMES,
                LIB_NUMBEROFCMDS);
        return;

    case OPCODE_LIBMENU:
        // LIBRARY RECEIVES A MENU CODE IN MenuCodeArg
        // MUST RETURN A MENU LIST IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        if(MENUNUMBER(MenuCodeArg) > 0) {
            RetNum = ERR_NOTMINE;
            return;
        }
        // WARNING: MAKE SURE THE ORDER IS CORRECT IN ROMPTR_TABLE
        ObjectPTR = ROMPTR_TABLE[MENUNUMBER(MenuCodeArg) + 2];
        RetNum = OK_CONTINUE;
        return;
    }

    case OPCODE_LIBHELP:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN CmdHelp
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {
        libFindMsg(CmdHelp, (word_p) LIB_HELPTABLE);
        return;
    }
    case OPCODE_LIBMSG:
        // LIBRARY RECEIVES AN OBJECT OR OPCODE IN LibError
        // MUST RETURN A STRING OBJECT IN ObjectPTR
        // AND RetNum=OK_CONTINUE;
    {

        libFindMsg(LibError, (word_p) LIB_MSGTABLE);
        return;
    }

    case OPCODE_LIBINSTALL:
        LibraryList = (word_p) libnumberlist;
        RetNum = OK_CONTINUE;
        return;
    case OPCODE_LIBREMOVE:
        return;

    }
    // UNHANDLED OPCODE...

    // IF IT'S A COMPILER OPCODE, RETURN ERR_NOTMINE
    if(OPCODE(CurOpcode) >= MIN_RESERVED_OPCODE) {
        RetNum = ERR_NOTMINE;
        return;
    }
    // BY DEFAULT, ISSUE A BAD OPCODE ERROR
    rplError(ERR_INVALIDOPCODE);

    return;

}

#endif
