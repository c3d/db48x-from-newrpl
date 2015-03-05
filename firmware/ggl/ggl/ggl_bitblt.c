#include <ggl.h>


void ggl_bitblt(gglsurface *dest,gglsurface *src,int width, int height)
{

// COPIES A RECTANGULAR REGION FROM src TO dest
// gglsurface CONTAINS THE BUFFER AND OFFSETS
// width AND height ARE THE SIZE OF THE BLOCK TO MOVE

// RESTRICTIONS: NO SAFETY CHECKS REGARDING MEMORY MOVEMENTS

int doff,soff,line;

    doff=dest->y*dest->width+dest->x;
    soff=src->y*src->width+src->x;

    for(line=0;line<height;++line)
    {
        ggl_hblt(dest->addr,doff,src->addr,soff,width);
        doff+=dest->width;
        soff+=src->width;
    }
}

