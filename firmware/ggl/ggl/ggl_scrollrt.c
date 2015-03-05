
#include <ggl.h>


void ggl_scrollrt(gglsurface *dest,int width, int height, int npixels)
{

// SCROLLS A RECTANGULAR REGION RIGHT npixelsup
// gglsurface CONTAINS THE BUFFER AND OFFSETS
// width AND height ARE THE SIZE OF THE BLOCK TO MOVE

// RESTRICTIONS: NO SAFETY CHECKS REGARDING MEMORY MOVEMENTS

gglsurface srf;

srf.addr=dest->addr;
srf.width=dest->width;
srf.x=dest->x+npixels;
srf.y=dest->y;

ggl_bitblt(&srf,dest,width-npixels,height);

}


