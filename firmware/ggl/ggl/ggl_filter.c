#include <ggl.h>


void ggl_filter(gglsurface *dest,int width, int height, int param, gglfilter filterfunc)
{

// FILTERS A RECTANGULAR REGION FROM src TO dest
// gglsurface CONTAINS THE BUFFER AND OFFSETS
// width AND height ARE THE SIZE OF THE BLOCK TO MOVE

// RESTRICTIONS: NO SAFETY CHECKS REGARDING MEMORY MOVEMENTS

int doff,line;

    doff=dest->y*dest->width+dest->x;

    for(line=0;line<height;++line)
    {
        ggl_hbltfilter(dest->addr,doff,width,param,filterfunc);
        doff+=dest->width;
    }
}

