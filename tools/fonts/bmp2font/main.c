#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    unsigned int   FileSize;
    unsigned int   Reserved;
    unsigned int   Offset;
    unsigned int   HeaderSize;
    unsigned int   Width;
    int            Height;
    unsigned short Planes;
    unsigned short BitsPixel;
    unsigned int   Compression;
    unsigned int   ImageSize;
    unsigned int   XPels_Meter;
    unsigned int   YPels_Meter;
    unsigned int   NumClr;
    unsigned int   ClrImp;
} BMP_Header;

#define LIB_FONTS                        78
#define MAX_GLYPHS                       65536
#define MAX_NCHARS                       0xfffff
#define PACKDATA(w, o)                   (((w) << 16) | ((o & 0xffff)))
#define SING_OFFSET(val)                 (((val) &0xFFF))
#define SING_LEN(val)                    (((val) >> 12) & 0xfffff)
#define MK_SINGRANGE(start, end, offset) ((((end) - (start) + 1) << 12) | ((offset) &0xfff))
#define MK_SINGGAP(start, end)           MK_SINGRANGE(start, end, 0xfff)
#define MKPROLOG(lib, size)              ((((lib) &0xFFF) << 20) | ((size) &0x3FFFF) | 0x80000)

// GET FONT INFORMATION
int          width[MAX_GLYPHS];
int          offset[MAX_GLYPHS];
int          codeidx[0x110000];
char         txtbuff[1024 * 1024]; // MAX. 1MB FOR THE TEXT FILE
unsigned int packedata[0x110000];
unsigned int ranges[2000];
int          used_ranges;
unsigned int offdata[20000 + MAX_NCHARS];
int          used_data;

// FIND DUPLICATED ITEMS IN THE STREAM
int searchDupData(unsigned int *start, int nitems)
{
    int j, k;

    for (j = 0; j < used_data - nitems; ++j)
    {
        for (k = 0; k < nitems; ++k)
            if (start[k] != offdata[j + k])
                break;
        if (k == nitems)
            return j;
    }
    return -1;
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("bmp2font 1.0\n------------\n\n");
        printf(
            "Syntax: bmp2font <font-bitmap-file.bmp> <font-text-file.txt> <output[.c or .nrpl]> [name of C "
            "variable]\n\n");
        return 0;
    }
    char          *bmpfile = argv[1], *txtfile = argv[2], *outfile = argv[3], *fontname;
    char           fname[4096];
    unsigned char *bmpdata;
    int            binary = 1;

    if (!strncmp(outfile + strlen(outfile) - 2, ".c", 2))
    {
        // THE FILE REQUESTED IS IN .c FORMAT
        binary = 0;
        if (argc >= 5)
            fontname = argv[4];
        else
        {
            // IF NOT PROVIDED BY THE USER, USE A NAME EXTRACTED FROM THE OUTPUT FILE
            int end = strlen(outfile) - 2;
            strncpy(fname, outfile, end);
            fname[end] = 0;
            fontname   = fname;
        }
    }
    else
        fontname = NULL;

    FILE *han = fopen(bmpfile, "rb");

    if (!han)
    {
        printf("Unable to open file '%s'\n", bmpfile);
        return 1;
    }

    int          invert;
    BMP_Header   hdr;
    unsigned int Palette[256];

    fseek(han, 2, SEEK_SET);
    fread(&hdr, sizeof(BMP_Header), 1, han);

    if (hdr.BitsPixel != 8 || hdr.Planes != 1)
    {
        fclose(han);
        printf("Only grayscale or 256-color bitmaps supported\n");
        return 0;
    }

    if (hdr.HeaderSize > 40)
        fseek(han, hdr.HeaderSize - 40, SEEK_CUR);

    fread(Palette, 4, 256, han);

    if (hdr.Height < 0)
    {
        hdr.Height = -hdr.Height;
        invert     = 0;
    }
    else
        invert = 1;

    int rowwords = ((hdr.Width * hdr.BitsPixel) / 8 + 3) >> 2;

    // printf("rowwords=%d\n",rowwords);

    bmpdata      = malloc(rowwords * 4 * hdr.Height);

    if (!bmpdata)
    {
        fclose(han);
        printf("Memory allocation error\n");
        return 1;
    }

    fseek(han, hdr.Offset, SEEK_SET);
    fread(bmpdata, 4, rowwords * hdr.Height, han);
    fclose(han);

    // printf("Bitmap read=%d bytes (requested %d bytes)\n",bytesread*4,rowwords*hdr.Height*4);

    printf("Bitmap read correctly\n");

    // *********************************************************
    // BEGIN FONT PROCESSING
    // *********************************************************

    // SCAN THE BITMAP FOR WIDTH AND OFFSET INFO

    int            x;
    unsigned char *addr;
    unsigned char  change = 0xaa;
    int            idx    = 0;
    addr                  = bmpdata + (invert ? 0 : (rowwords * 4 * (hdr.Height - 1)));

    for (x = 0; x < (int) hdr.Width; ++x)
    {
        if (addr[x] != change)
        {
            if (idx > 0)
                width[idx - 1] = x - offset[idx - 1];
            offset[idx] = x;
            ++idx;
            change = addr[x];
        }
    }
    if (idx > 0)
        width[idx - 1] = x - offset[idx - 1];
    // offset[idx]=x;
    //++idx;

    printf("Detected %d symbols in bitmap\n", idx);

    // CONVERT THE BITMAP TO MONOCHROME

    unsigned char *monobitmap = calloc((hdr.Height - 1), ((hdr.Width + 7) >> 3));
    if (!monobitmap)
    {
        printf("Memory allocation error.\n");
        free(bmpdata);
        return 1;
    }

    int color;
    unsigned char *ptr = monobitmap;
    int            y, ystart, yend, yinc, rowlen;

    rowlen = (hdr.Width + 7) >> 3;

    if (invert)
    {
        ystart = hdr.Height - 1;
        yend   = 0;
        yinc   = -1;
    }
    else
    {
        ystart = 0;
        yend   = hdr.Height - 1;
        yinc   = 1;
    }

    for (y = ystart; y != yend; y += yinc)
    {
        addr = bmpdata + y * rowwords * 4;
        for (x = 0; x < (int) hdr.Width; ++x)
        {
            color = Palette[(int) addr[x]];
            if (!color)
                ptr[x >> 3] |= 1 << (x & 7);
        }
        ptr += rowlen;
    }

    // FINISHED BITMAP CONVERSION

    // READ THE GLYPHS CODES
    han = fopen(txtfile, "rb");

    if (!han)
    {
        printf("Unable to open file '%s'\n", txtfile);
        free(monobitmap);
        free(bmpdata);
        return 1;
    }
    int txtsize;
    txtsize = fread(txtbuff, 1, 256 * 1024, han);
    fclose(han);

    // PROCESS THE TEXT FILE
    char *txtend = txtbuff + txtsize;
    char *lineend, *taddr;
    taddr      = txtbuff;
    int txtidx = 0, usedline = 0, usedflag;
    int base, ndigits;

    // RESET ALL UNICODE CHARACTERS TO POINT TO SYMBOL 0
    for (y = 0; y < 0x110000; ++y)
        codeidx[y] = 0;

    // SCAN THE FILE FOR CODES
    while (taddr < txtend)
    {
        lineend  = taddr;
        usedflag = 0;
        while ((*lineend != '\n') && (lineend < txtend))
            ++lineend;

        do
        {
            // SKIP ANY BLANKS
            while (((*taddr == ' ') || (*taddr == '\t')) && (taddr < lineend))
                ++taddr;

            if (taddr >= txtend)
                break;

            if ((taddr[0] == '/') || (taddr[0] == '\n'))
            {
                // DISCARD REST OF THE LINE
                taddr = lineend;
                ++txtidx;
                continue;
            }

            base = 10;
            if (taddr[0] == '0')
            {
                if ((taddr[1] == 'x') || (taddr[1] == 'X'))
                {
                    base = 16;
                    taddr += 2;
                }
                // TODO: ADD OTHER FORMATS HERE
            }
            // CONVERT THE NUMBER
            unsigned int result = 0;
            int          digit  = -1;
            ndigits             = 0;

            do
            {
                if ((taddr[0] >= '0') && (taddr[0] <= '9'))
                    digit = taddr[0] - '0';
                else
                {
                    if (base == 16)
                    {
                        if ((taddr[0] >= 'A') && (taddr[0] <= 'F'))
                            digit = taddr[0] - 'A' + 10;
                        else if ((taddr[0] >= 'a') && (taddr[0] <= 'f'))
                            digit = taddr[0] - 'a' + 10;
                    }
                }

                if (digit < 0)
                {
                    if ((taddr[0] != ',') && (taddr[0] != '/') && (taddr[0] != '\n') && (taddr[0] != '\r') &&
                        (taddr[0] != ' ') && (taddr[0] != '\t'))
                    {
                        printf("Syntax error in text file, line=%d\n", txtidx + 1);
                        free(bmpdata);
                        free(monobitmap);
                        return 1;
                    }
                    if (ndigits)
                    {
                        if (result > 0x10ffff)
                        {
                            printf("Number out of range in text file, line=%d\n", txtidx + 1);
                            free(bmpdata);
                            free(monobitmap);
                            return 1;
                        }
                        codeidx[result] = usedline;
                        // printf("Matched %04X = %04X , Offset=%d\n",result,usedline,offset[usedline]);

                        usedflag        = 1;
                    }
                    break;
                }
                else
                {
                    result = result * base + digit;
                    ++ndigits;
                    digit = -1;
                    ++taddr;
                }

            } while (taddr < lineend);

            if (taddr == lineend)
            {
                if (ndigits)
                {
                    if (result > 0x10ffff)
                    {
                        printf("Number out of range in text file, line=%d\n", txtidx + 1);
                        free(bmpdata);
                        free(monobitmap);
                        return 1;
                    }
                    codeidx[result] = usedline;
                    // printf("Matched %04X = %04X , Offset=%d\n",result,usedline,offset[usedline]);
                    usedflag        = 1;
                }
            }
            // POSSIBLY SKIP BLANKS BEFORE A COMMA
            while (((*taddr == ' ') || (*taddr == '\t')) && (taddr < lineend))
                ++taddr;

        } while (*taddr++ == ',');

        // END OF LINE REACHED
        if (usedflag)
            ++usedline;
        taddr = lineend + 1;
    }

    printf("Number of codes in text file: %d\n", usedline);

    if (usedline != idx)
    {
        int references[idx];

        for (y = 0; y < idx; ++y)
            references[y] = 0;

        for (y = 0; y < 0x110000; ++y)
            references[codeidx[y]]++;

        for (y = 0; y < idx; ++y)
            if (references[y] == 0)
                printf("**Warning**: Symbol %d at X=%d is not referenced from text file (unused).\n", y, offset[y]);
    }

    // PACK AND SAVE THE DATA

    // FONT STRUCTURE
    /*
     * 4-BYTES PROLOG W/SIZE (COMPATIBLE WITH NEWRPL OBJECT FORMAT)
     * 4-BYTES:
     *          0xHHHHWWWW --> H=FONT HEIGHT, W=TOTAL BITMAP ROW WIDTH IN BYTES
     * 4-BYTES:
     *          2-BYTES: OFFSET IN WORDS FROM PROLOG TO FONT BITMAP
     *          2-BYTES: OFFSET IN WORDS FROM PROLOG TO WIDTH&OFFSET TABLE
     * ------ TABLE OF UNICODE->GLYPH MAPPING -----
     * 4-BYTE RANGES: 0xNNNNNOOO, WITH N=NUMBER OF CODES IN THIS RANGE, OOO=INDEX INTO WIDTH&OFFSET TABLE (0-4094, 4095
     * IS RESERVED FOR UNMAPPED) 0xNNNNNFFF, WHEN OOO=0XFFF, THE ENTIRE RANGE OF CODES IS MAPPED TO INDEX 0 OF THE
     * WIDTH&OFFSET TABLE. REPEAT UNTIL IT COVERS THE ENTIRE UNICODE RANGE 0x110000
     *
     * ------ TABLE OF WIDTH & OFFSET
     * 4-BYTES VALUES 0xWWWWOOOO, WITH W=WIDTH IN PIXELS, OOO=x COORDINATE WITHIN THE BITMAP
     * REPEAT AS NEEDED
     *
     * ------ BITMAP
     * TOTAL NUMBER OF BYTES=HEIGHT*ROW WIDTH IN BYTES
     *
     * ----- PADDING FOR WORD-ALIGNMENT
     */

    // ANALYZE RANGES AND PACK
    // BEGIN ANALYSIS OF RANGES

    // PACK THE OFFSET AND WIDTH DATA

#define PACK_THRESHOLD 40

    int j, r;

    for (j = 0; j < 0x110000; ++j)
        packedata[j] = PACKDATA(width[codeidx[j]], offset[codeidx[j]]);

    used_data     = 0;
    used_ranges   = 0;

    int prevrange = 0;
    j             = 0;
    do
    {
        r = j + 1;
        while ((packedata[r] == packedata[j]) && (r < 0x110000))
            ++r;
        if (r - j > PACK_THRESHOLD)
        {
            if (j != prevrange)
            {
                while (j - prevrange > MAX_NCHARS)
                {
                    int location = searchDupData(packedata + prevrange, MAX_NCHARS);
                    printf("Range: %04X..%04X, LEN=%d --> OFFSET=%d\n",
                           prevrange,
                           prevrange + MAX_NCHARS - 1,
                           MAX_NCHARS,
                           location < 0 ? used_data : location);
                    unsigned int data;

                    if (location < 0)
                    {
                        // APPEND NEW DATA
                        data = MK_SINGRANGE(1, MAX_NCHARS, used_data);
                        int f;
                        for (f = prevrange; f < prevrange + MAX_NCHARS; ++f, ++used_data)
                            offdata[used_data] = packedata[f];
                    }
                    else
                    {
                        // DATA IS REPEATED, REUSE
                        data = MK_SINGRANGE(1, MAX_NCHARS, location);
                    }

                    ranges[used_ranges] = data;
                    ++used_ranges;
                    prevrange += MAX_NCHARS;
                }

                // THERE'S A GAP OF NON-REPEATED BYTES

                int          location = searchDupData(packedata + prevrange, j - prevrange);
                unsigned int data;
                printf("Range: %04X..%04X, LEN=%d --> OFFSET=%d\n",
                       prevrange,
                       j - 1,
                       j - prevrange,
                       location < 0 ? used_data : location);
                if (location < 0)
                {
                    data = MK_SINGRANGE(prevrange, j - 1, used_data);
                    int f;
                    for (f = prevrange; f < j; ++f, ++used_data)
                        offdata[used_data] = packedata[f];
                }
                else
                {
                    data = MK_SINGRANGE(prevrange, j - 1, location);
                }
                ranges[used_ranges] = data;
                ++used_ranges;
            }
            // ADD THE RANGE WITH REPETITIVE DATA

            printf("Range: %04X..%04X = %02X, LEN=%d\n", j, r - 1, packedata[j], r - j);
            ranges[used_ranges] = MK_SINGGAP(j, r - 1);
            ++used_ranges;
            prevrange = r;
        }
        j = r;
    } while (j < 0x110000);

    printf("Total ranges=%d\n", used_ranges);
    printf("Total table bytes=%d\n", (int) sizeof(unsigned int) * used_data);

    // DONE PROCESSING, CREATE FILE AND SAVE IT

    // WRITE BINARY FILE
    if (binary)
    {
        han = fopen(outfile, "wb");

        if (!han)
        {
            printf("Cannot open output file '%s'.\n", outfile);
            free(monobitmap);
            free(bmpdata);
            return 1;
        }

        printf("Starting binary output to file '%s'\n", outfile);

        int          totalsize = 2 + used_ranges + used_data + ((rowlen * (hdr.Height - 1) + 3) >> 2);

        unsigned int prolog    = MKPROLOG(LIB_FONTS, totalsize);

        fwrite(&prolog, 4, 1, han);

        prolog = ((hdr.Height - 1) << 16) | (rowlen & 0xffff);

        fwrite(&prolog, 4, 1, han);

        prolog = ((3 + used_ranges) << 16 | (3 + used_ranges + used_data));

        fwrite(&prolog, 4, 1, han);

        // WRITE RANGES
        fwrite(&ranges, 4, used_ranges, han);

        // WRITE DATA TABLE
        fwrite(&offdata, 4, used_data, han);

        // WRITE BITMAP
        fwrite(&monobitmap, rowlen, hdr.Height - 1, han);

        j = 4 - ((rowlen * (hdr.Height - 1)) & 3);
        if (j < 4)
        {
            prolog = 0;
            fwrite(&prolog, 1, j, han);
        }
    }
    else
    {
        // TEXT OUTPUT
        FILE *fontlist = fopen("fontlist.h", "a");
        if (!fontlist)
        {
            fprintf(stderr, "Cannot open header file '%s'.\n", "fontlist.h");
            free(monobitmap);
            free(bmpdata);
            return 1;
        }
        fprintf(fontlist,
                "\n"
                "extern const UNIFONT FONT_%s;\n"
                "#define Font_%s (&FONT_%s)\n",
                fontname,
                fontname,
                fontname);
        fclose(fontlist);

        han = fopen(outfile, "wt");

        if (!han)
        {
            fprintf(stderr, "Cannot open output file '%s'.\n", outfile);
            free(monobitmap);
            free(bmpdata);
            return 1;
        }

        printf("Starting C format output to file '%s'\n", outfile);

        int          totalsize = 2 + used_ranges + used_data + ((rowlen * (hdr.Height - 1) + 3) >> 2);

        unsigned int prolog    = MKPROLOG(LIB_FONTS, totalsize);

        // fwrite(&prolog,4,1,han);
        fprintf(han,
                "\n"
                "/*************** FONT FILE CONVERTED FROM %s AND %s ************** */\n",
                bmpfile,
                txtfile);

        fprintf(han,
                "\n"
                "#include <unifont.h>\n"
                "\n"
                "const UNIFONT FONT_%s=\n"
                "{\n",
                fontname);

        fprintf(han,
                "    .Prolog       = 0x%X,\n"
                "    .BitmapWidth  = %u,\n"
                "    .BitmapHeight = %u,\n"
                "    .OffsetBitmap = %u,\n"
                "    .OffsetTable  = %u,\n"
                "\n"
                "    .MapTable = {\n",
                prolog,
                rowlen,
                hdr.Height - 1,
                3 + used_ranges + used_data,
                3 + used_ranges);

        // WRITE RANGES
        // fwrite(&ranges,4,used_ranges,han);
        fprintf(han, "    // Ranges");
        for (j = 0; j < used_ranges; ++j)
        {
            if (j % 8 == 0)
                fprintf(han, "\n        ");
            fprintf(han, "0x%08X, ", ranges[j]);
        }
        fprintf(han,
                "\n"
                "        // End of ranges\n"
                "\n");

        // WRITE DATA TABLE
        // fwrite(&offdata,2,used_data,han);
        fprintf(han, "        // Data tables");
        for (j = 0; j < used_data; ++j)
        {
            if (j % 8 == 0)
                fprintf(han, "\n        ");
            fprintf(han, "0x%08X, ", offdata[j]);
        }
        fprintf(han,
                "\n"
                "        // End of data tables\n"
                "\n");

        // WRITE BITMAP
        // fwrite(&monobitmap,rowlen,hdr.Height-1,han);
        // REVISIT: This code assumes little-endian
        fprintf(han, "        // Bitmap data");
        r = rowlen * (hdr.Height - 1);
        for (j = 0; j < r; ++j)
        {
            if (j % 32 == 0)
                fprintf(han, "\n        ");
            switch (j & 3)
            {
            case 0: prolog = monobitmap[j]; break;
            case 1: prolog |= monobitmap[j] << 8; break;
            case 2: prolog |= monobitmap[j] << 16; break;
            case 3:
                prolog |= monobitmap[j] << 24;

                fprintf(han, "0x%08X", prolog);
                if (j != r - 1)
                    fprintf(han, ", ");
            }
        }
        if (j & 3)
        {
            fprintf(han, "0x%04X", prolog);
            fprintf(han, "\n");
        }

        // FINISHED OUTPUT
        fprintf(han,
                "\n"
                "        // End of bitmap data\n"
                "    }\n"
                "};\n"
                "\n"
                "/*********** END OF CONVERTED FONT ******************/\n");

        /*
           fprintf(han,"unsigned int ggl_op_mono2gray[256]={\n");

           for(j=0;j<256;j++)
           {
           prolog=0;
           for(r=7;r>=0;--r)
           {
           prolog<<=4;
           if(j&(1<<r)) prolog|=0xf;
           }
           fprintf(han,"0x%X",prolog);
           if(j!=255) fprintf(han,", ");
           if(j%16==0) fprintf(han,"\n");
           }
           fprintf(han,"\n\n};");
         */
    }

    fclose(han);

    printf("Done.\n\n");
    // FINISHED WRITING FONT FILE

    return 0;
}
