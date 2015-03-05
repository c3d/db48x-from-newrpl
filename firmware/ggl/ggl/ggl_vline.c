
#include <ggl.h>


void ggl_vline(gglsurface *srf,int x,int yt,int yb, int color)
{
    // PAINTS A VERTICAL LINE FROM yt TO yb BOTH INCLUSIVE
    // color=number from 0 to 15
    // RESTRICTIONS: yb>=yt

    int offset=srf->width*yt+x;

    while(yt<=yb) { ggl_pltnib(srf->addr,offset,color>>((yt&7)<<2)); offset+=srf->width; ++yt; }

}

