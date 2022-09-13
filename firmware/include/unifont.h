#ifndef UNIFONT_H
#define UNIFONT_H

typedef struct
{
    unsigned int   Prolog;
    unsigned short BitmapWidth;
    unsigned short BitmapHeight;
    unsigned short OffsetBitmap;
    unsigned short OffsetTable;
    unsigned int   MapTable[];
} UNIFONT;
#endif // UNIFONT_H
