// ****************************************************************************
//  ttf2font.c                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     This converts a TTF file to an HP48 bitmap
//
//     This is freely inspired by ttf2RasterFont in the wp43s project,
//     but simplified to generate the simpler data structures in newRPL
//
//
//
//
//
// ****************************************************************************
//   (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the GNU General Public License v3
// ****************************************************************************
//   This file is part of DB48X.
//
//   DB48X is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with DB48X.  If not, see <https://www.gnu.org/licenses/>.
// ****************************************************************************

#include <ft2build.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include FT_FREETYPE_H

typedef const char *cstring;
int verbose = 0;


const char *getErrorMessage(FT_Error err)
// ----------------------------------------------------------------------------
//    Error messages from freetype2
// ----------------------------------------------------------------------------

{
#undef __FTERRORS_H__
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERRORDEF(e, v, s)       case e: return s;
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
    return "(Unknown error)";
}


int sortCharCodes(const void *a, const void *b)
// ----------------------------------------------------------------------------
//    Sort according to the char codes
// ----------------------------------------------------------------------------
{
    return *(FT_ULong *) a - *(FT_ULong *) b;
}


void processFont(cstring fontName,
                 cstring ttfName,
                 cstring cSourceName,
                 int     fontSize,
                 int     threshold)
// ----------------------------------------------------------------------------
//   Process a font and generate the C source file from it
// ----------------------------------------------------------------------------
/* Font structure
 *
 * 4-Bytes Prolog with size (Compatible with newRPL object format)
 * 4-Bytes:
 *          0xHHHHWWWW --> H=Font height, W=Total bitmap row width in bytes
 * 4-Bytes:
 *          2-Bytes: Offset in words from prolog to font bitmap
 *          2-Bytes: Offset in words from prolog to width & offset table
 * ------ Table of Unicode->glyph mapping -----
 * 4-Byte ranges: 0xNNNNNOOO
 *                NNNNN=number of codes in this range,
 *                OOO=Index into width & offset table
 *                    FFF being reserved for unmapped ranges
 * ------ Table of width & offset
 * 4-Bytes values: 0xWWWWOOOO
 *                 WWWW: width in pixels of the glyph
 *                 OOOO: Coordinate of the glyph within the bitmap
 * ------ BITMAP
 * Total number of bytes=height*row width in bytes
 * ----- PADDING FOR WORD-ALIGNMENT
 */
{
    FILE *output = fopen(cSourceName, "w");
    if (!output)
    {
        fprintf(stderr, "Cannot open source file %s", cSourceName);
        fprintf(stderr, "Error %d: %s", errno, strerror(errno));
        exit(1);
    }

    // Initialize freetype2
    FT_Library library;
    FT_Error error = FT_Init_FreeType(&library);
    if (error != FT_Err_Ok)
    {
        fprintf(stderr, "Error during freetype2 library initialisation.\n");
        fprintf(stderr, "Error %d : %s\n", error, getErrorMessage(error));
        exit(1);
    }

    // Open the font face
    FT_Face   face;
    error = FT_New_Face(library, ttfName, 0, &face);
    if (error != FT_Err_Ok)
    {
        fprintf(stderr, "Error during face creation from file %s\n", ttfName);
        fprintf(stderr, "Error %d : %s\n", error, getErrorMessage(error));
        FT_Done_FreeType(library);
        exit(1);
    }

    // Set font size - Formula lifted from wp43s
    int unitsPerEM       = face->units_per_EM;
    int pixelSize        = unitsPerEM == 1024 ? 32 : 50;
    int baseSize         = unitsPerEM / pixelSize;
    int fontHeightPixels = fontSize ? fontSize : baseSize;
#define SCALED(x) ((x) * fontHeightPixels / baseSize)

    error = FT_Set_Pixel_Sizes(face, 0, fontHeightPixels);
    if (error != FT_Err_Ok)
    {
        fprintf(stderr, "Error setting pixel size from file %s\n", ttfName);
        fprintf(stderr, "Error %d : %s\n", error, getErrorMessage(error));
        FT_Done_FreeType(library);
        exit(1);
    }

    // Face scaling
    unsigned  bitmapWidth    = 0;
    int       ascend         = face->ascender;
    int       descend        = face->descender;
    int       faceHeight     = ascend - descend;
    int       bitmapHeight   = SCALED(faceHeight) / pixelSize;
    int       renderFlag     = FT_LOAD_RENDER;
    if (!threshold)
        renderFlag |= FT_LOAD_TARGET_MONO;


    // Count number of glyphs
    unsigned  numberOfGlyphs = face->num_glyphs;
    FT_ULong *charCodes      = calloc(numberOfGlyphs, sizeof(FT_ULong));
    FT_ULong *curCharCode    = charCodes;
    FT_UInt   glyphIndex     = 0;
    unsigned  glyphCount     = 0;
    int       minRowsBelow   = 0;
    for (FT_ULong charCode = FT_Get_First_Char(face, &glyphIndex);
         glyphIndex;
         charCode          = FT_Get_Next_Char(face, charCode, &glyphIndex))
    {
        *curCharCode++      = charCode;
        error               = FT_Load_Glyph(face, glyphIndex, renderFlag);
        if (error != FT_Err_Ok)
        {
            fprintf(stderr, "warning: failed to load glyph 0x%04lX\n", charCode);
            fprintf(stderr, "Error %d : %s\n", error, getErrorMessage(error));
        }
        FT_Glyph_Metrics *m = &face->glyph->metrics;
        FT_Bitmap        *b = &face->glyph->bitmap;
        int rowsGlyph       = b->rows;
        int rowsDescend     = SCALED(descend) / pixelSize;
        int rowsBelowGlyph  = m->horiBearingY / 64 - rowsDescend - rowsGlyph;
        if (rowsBelowGlyph < minRowsBelow)
            minRowsBelow = rowsBelowGlyph;

        bitmapWidth += face->glyph->metrics.horiAdvance / 64;
        glyphCount++;
    }

    if (minRowsBelow < 0)
        bitmapHeight -= minRowsBelow;

    if (verbose || glyphCount > numberOfGlyphs)
    {
        fprintf(stderr, "Number of glyphs %u, glyph count %u, width %u\n",
                numberOfGlyphs, glyphCount, bitmapWidth);
        if (glyphCount > numberOfGlyphs)
            exit(1);
    }

    // Sort them per unicode point
    qsort(charCodes, glyphCount, sizeof(charCodes[0]), sortCharCodes);

    // Round up the bitmap Width to full bytes
    bitmapWidth = (bitmapWidth + 7) / 8 * 8;

    // Allocate the global bitmap where we will draw glyphs and other structures
    size_t    bitmapSize      = (bitmapWidth * bitmapHeight + 31) / 32;
    uint32_t *bitmap          = calloc(bitmapSize, sizeof(uint32_t));
    uint32_t *offsets         = calloc(numberOfGlyphs, sizeof(uint32_t));
    uint32_t *ranges          = calloc(numberOfGlyphs, sizeof(uint32_t));

    if (verbose)
        printf("Font bitmap width %u height %u size %u\n",
               bitmapWidth, bitmapHeight, (unsigned) bitmapSize);

    // Set the transform for the font
    FT_Vector pen;
    pen.x = 0;
    pen.y = (fontHeightPixels - SCALED(face->ascender) / pixelSize) * 64;
    FT_Set_Transform(face, NULL, &pen);

    // Start on the left of the bitmap
    uint32_t  bitmapX          = 0;
    uint32_t *offset           = offsets;
    uint32_t *range            = ranges;
    uint32_t  firstCode        = 0;
    uint32_t  currentCode      = 0;
    uint32_t  widthOffsetIndex = 0;

    // Loop on all glyphs
    for (unsigned g = 0; g < glyphCount; g++)
    {
        FT_ULong charCode   = charCodes[g];
        FT_UInt  glyphIndex = FT_Get_Char_Index(face, charCode);
        if (glyphIndex == 0)
        {
            fprintf(stderr, "Glyph 0x%04lX undefined\n", charCode);
            continue;
        }

        error = FT_Load_Glyph(face, glyphIndex, renderFlag);
        if (error != FT_Err_Ok)
        {
            fprintf(stderr, "warning: failed to load glyph 0x%04lX\n", charCode);
            fprintf(stderr, "Error %d : %s\n", error, getErrorMessage(error));
            continue;
        }

        // Get glyph metrics and bitmap
        FT_Glyph_Metrics *m = &face->glyph->metrics;
        FT_Bitmap        *b = &face->glyph->bitmap;

        // Columns in the glyph
        int colsBeforeGlyph = m->horiBearingX / 64;
        int colsGlyph       = b->width;
        int colsAfterGlyph  = m->horiAdvance / 64 - colsBeforeGlyph - colsGlyph;
        if (colsBeforeGlyph < 0)
        {
            colsGlyph += colsBeforeGlyph;
            colsBeforeGlyph = 0;
        }

        // Rows in the glyph
        int rowsAboveGlyph = SCALED(ascend) / pixelSize - m->horiBearingY / 64;
        int rowsAboveSave  = rowsAboveGlyph;
        int rowsGlyph      = b->rows;
        int rowsDescend    = SCALED(descend) / pixelSize;
        int rowsBelowGlyph = m->horiBearingY / 64 - rowsDescend - rowsGlyph;
        int rowsBelowSave  = rowsBelowGlyph;
        if (rowsAboveGlyph < 0)
        {
            rowsGlyph += rowsAboveGlyph;
            rowsAboveGlyph = 0;
        }

        uint32_t glyphWidth = face->glyph->metrics.horiAdvance / 64;
        if (verbose)
        {
            char utf8[4] = { 0 };
            if (charCode < 0x80)
            {
                utf8[0] = charCode;
            }
            else if (charCode < 0x800)
            {
                utf8[0] = 0xC0 | (charCode >> 6);
                utf8[1] = 0x80 | (charCode & 63);
            }
            else if (charCode < 0x10000)
            {
                utf8[0] = 0xE0 | (charCode >> 12);
                utf8[1] = 0x80 | ((charCode >> 6) & 63);
                utf8[2] = 0x80 | ((charCode >> 0) & 63);
            }
            else
            {
                utf8[0] = utf8[1] = utf8[2] = '-';
            }
            printf("Glyph %4lu '%s' width %u"
                   "  Columns: %d %d %d"
                   "  Rows: %d %d %d\n",
                   charCode,
                   utf8,
                   glyphWidth,
                   colsBeforeGlyph,
                   colsGlyph,
                   colsAfterGlyph,
                   rowsAboveSave,
                   rowsGlyph,
                   rowsBelowSave);
        }

        // Copy the bits from the bitmap
        unsigned char *buffer = face->glyph->bitmap.buffer;
        unsigned       pitch  = face->glyph->bitmap.pitch;
        unsigned       bwidth = face->glyph->bitmap.width;
        for (int y = 0; y < rowsGlyph; y++)
        {
            int by = y + rowsAboveGlyph;
            for (int x = 0; x < colsGlyph; x++)
            {
                int bit = 0;
                if (threshold)
                {
                    int bo = y * bwidth + x;
                    bit = buffer[bo] >= threshold;
                }
                else
                {
                    int bo = y * pitch + x/8;
                    bit = (buffer[bo] >> (7 - x % 8)) & 1;
                }
                if (verbose)
                    putchar(bit ? '#' : '.');

                if (bit)
                {
                    int bx = bitmapX + x + colsBeforeGlyph;
                    uint32_t bitOffset = by * bitmapWidth + bx;
                    uint32_t wordOffset = bitOffset / 32;
                    if (wordOffset > bitmapSize)
                    {
                        fprintf(stderr, "Ooops, wordOffset=%u, size=%u\n"
                                "  bx=%u by=%u bitOffset=%u\n",
                                wordOffset, (unsigned) bitmapSize,
                                bx, by, bitOffset);
                        exit(127);
                    }
                    bitmap[wordOffset] |= 1UL << (bitOffset % 32);
                }
            }
            if (verbose)
                putchar('\n');
        }

        // Fill the range table
        if (charCode != currentCode)
        {
            // We are changing ranges, write the previous one
            uint32_t numCodes = currentCode - firstCode;
            if (verbose)
                printf("New glyph range at %u, had %u codes in %u..%u\n",
                       (int) charCode, numCodes, firstCode, currentCode);
            *range++ = (numCodes << 12) | widthOffsetIndex;
            *range++ = ((charCode - currentCode) << 12) | 0xFFF;
            currentCode = charCode;
            firstCode = charCode;
            widthOffsetIndex = offset - offsets;
        }

        // Fill the width/offset table
        if (verbose)
            printf("glyphWidth %u bitmapX %u offset table %08X\n",
                   glyphWidth, bitmapX, (glyphWidth << 16) | bitmapX);
        *offset++ = (glyphWidth << 16) | bitmapX;

        // Advance to next glyph in the bitmap
        bitmapX += glyphWidth;
        currentCode++;
    }

    // Fill the last range
    uint32_t numCodes = currentCode - firstCode;
    if (verbose)
        printf("Last glyph range had %u codes in %u..%u\n",
               numCodes, firstCode, currentCode);
    *range++ = (numCodes << 12) | widthOffsetIndex;
    *range++ = ((0xFFFFF - currentCode) << 12) | 0xFFF;

    // Now time to emit the actual data
    fprintf(output,
            "/** Font %s, generated from %s - Do not edit manually **/\n",
            fontName, ttfName);
    fprintf(output,
            "\n"
            "#include <unifont.h>\n"
            "\n"
            "const UNIFONT %s=\n"
            "{\n",
            fontName);

#define MK_PROLOG(lib, size)                                                \
    ((((lib) &0xFFF) << 20) | 0x80000 |                                    \
     ((size > 0x3ffff) ? (0x40000 | ((((size) -0x40000) >> 10) & 0x3ffff)) \
                       : ((size) &0x3FFFF)))
#define LIB_FONTS       78

    unsigned usedRanges = range - ranges;
    unsigned usedData   = offset - offsets;
    unsigned totalSize  = 2 + usedRanges + usedData + bitmapSize / 4;
    uint32_t prolog     = MK_PROLOG(LIB_FONTS, totalSize);

    fprintf(output,
            "    .Prolog       = 0x%X,\n"
            "    .BitmapWidth  = %u,\n"
            "    .BitmapHeight = %u,\n"
            "    .OffsetBitmap = %u,\n"
            "    .OffsetTable  = %u,\n"
            "\n"
            "    .MapTable = {\n",
            prolog,
            bitmapWidth / 8,
            bitmapHeight,
            3 + usedRanges + usedData,
            3 + usedRanges);

    // Emit the ranges
    fprintf(output, "        // Ranges");
    for (uint32_t *r = ranges; r < range; r++)
        fprintf(output, "%s0x%08X,",
                (r - ranges) % 8 == 0 ? "\n       " : " ",
                *r);

    // Emit the width / offsets
    fprintf(output, "\n\n        // Width/offset");
    for (uint32_t *o = offsets; o < offset; o++)
        fprintf(output, "%s0x%08X,",
                (o - offsets) % 8 == 0 ? "\n       " : " ",
                *o);

    // Emit the bitmap
    uint32_t *bitmapEnd = bitmap + bitmapSize;
    fprintf(output, "\n\n        // Bitmap");
    for (uint32_t *b = bitmap; b < bitmapEnd; b++)
        fprintf(output, "%s0x%08X,",
                (b - bitmap) % 8 == 0 ? "\n       " : " ",
                *b);

    // Close the data table
    fprintf(output,
            "\n"
            "        // End of bitmap data\n"
            "    }\n"
            "};\n"
            "\n");
    fclose(output);

    // Free memory
    free(ranges);
    free(offsets);
    free(bitmap);
    free(charCodes);

    // Free the face and library ressources
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}


void usage(cstring prog)
// ----------------------------------------------------------------------------
//   Display the usage message for the program
// ----------------------------------------------------------------------------
{
    printf("Usage: %s [-h] [-v] [-s <size>] <name> <ttf> <output>\n"
           "  name: Name of the structure in C\n"
           "  ttf: TrueType input font\n"
           "  output: C source file to be generated\n"
           "  -h: Display this usage message\n"
           "  -v: Verbose output\n"
           "  -s <size>: Force font size to s pixels\n", prog);
}


int main(int argc, char *argv[])
// ----------------------------------------------------------------------------
//   Run the tool
// ----------------------------------------------------------------------------
{
    // Process options
    int opt;
    int fontSize = 0;
    int threshold = 0;
    while ((opt = getopt(argc, argv, "hs:t:v")) != -1)
    {
        switch (opt)
        {
        case 'v':
            verbose = 1;
            break;
        case 's':
            fontSize = atoi(optarg);
            break;
        case 't':
            threshold = atoi(optarg);
            break;
        case 'h':
            usage(argv[0]);
            exit(0);
        default:
            usage(argv[0]);
            exit(1);
        }
    }
    argc -= optind;
    if (argc < 3)
    {
        usage(argv[0]);
        return 1;
    }
    argv += optind;

    // Generate the C source code
    processFont(argv[0], argv[1], argv[2], fontSize, threshold);

    return 0;
}
