
#include <ggl.h>

void ggl_rect(gglsurface *srf,int x1,int y1,int x2,int y2,int color)
{
// DRAWS A RECTANGLE BETWEEN x1,y1 and x2,y2 ALL INCLUSIVE
// color CAN BE AN 8-BIT PATTERN THAT REPEATS VERTICALLY
// RESTRICTIONS:
//        NO BOUNDARY CHECKS
//        y2>=y1 && x2>=x1

while(y1<=y2) {
ggl_hline(srf,y1,x1,x2,color);
++y1;
}

}
