

#include <ggl.h>


void ggl_scrolldn(gglsurface *dest,int width, int height, int npixels)
{

// SCROLLS A RECTANGULAR REGION DOWN npixels
// gglsurface CONTAINS THE BUFFER AND OFFSETS
// width AND height ARE THE SIZE OF THE BLOCK TO MOVE

// RESTRICTIONS: NO SAFETY CHECKS REGARDING MEMORY MOVEMENTS

gglsurface srf;

srf.addr=dest->addr;
srf.width=dest->width;
srf.x=dest->x;
srf.y=dest->y+npixels;

ggl_revblt(&srf,dest,width,height-npixels);

}


