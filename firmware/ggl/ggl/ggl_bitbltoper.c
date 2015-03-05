
#include <ggl.h>


void ggl_bitbltoper(gglsurface *dest,gglsurface *src,int width, int height,int param,ggloperator fop)
{

// COPIES A RECTANGULAR REGION FROM src TO dest AND APPLIES THE GIVEN OPERATOR
// gglsurface CONTAINS THE BUFFER AND OFFSETS
// width AND height ARE THE SIZE OF THE BLOCK TO MOVE

// RESTRICTIONS: NO SAFETY CHECKS REGARDING MEMORY MOVEMENTS

int doff,soff,line;

    doff=dest->y*dest->width+dest->x;
    soff=src->y*src->width+src->x;

    for(line=0;line<height;++line)
    {
        ggl_hbltoper(dest->addr,doff,src->addr,soff,width,param,fop);
        doff+=dest->width;
        soff+=src->width;
    }
}
