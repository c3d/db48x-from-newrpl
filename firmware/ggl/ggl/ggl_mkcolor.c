
#include <ggl.h>


int ggl_mkcolor(int color)
{
    // RETURNS A SOLID PATTERN WITH ALL 8 PIXELS SET TO color
    color&=0xf;
    color|=color<<4;
    color|=color<<8;
    color|=color<<16;

    return color;
}

