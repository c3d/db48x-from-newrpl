#include <ggl.h>


void ggl_scrollup(gglsurface *dest,int width, int height, int npixelsup)
{

// SCROLLS A RECTANGULAR REGION UP npixelsup
// gglsurface CONTAINS THE BUFFER AND OFFSETS
// width AND height ARE THE SIZE OF THE BLOCK TO MOVE

// RESTRICTIONS: NO SAFETY CHECKS REGARDING MEMORY MOVEMENTS

gglsurface srf;

srf.addr=dest->addr;
srf.width=dest->width;
srf.x=dest->x;
srf.y=dest->y+npixelsup;

ggl_bitblt(dest,&srf,width,height-npixelsup);

}


