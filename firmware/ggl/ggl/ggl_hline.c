
#include <ggl.h>


void ggl_hline(gglsurface *srf,int y,int xl,int xr, int color)
{
    // PAINTS A HORIZONTAL LINE FROM xl TO xr BOTH INCLUSIVE
    // color=COLORED PATTERN TO USE, 8 PIXELS - 1 NIBBLE PER PIXEL
    //         PATTERN IS ALWAYS WORD ALIGNED

    // RESTRICTIONS: xr>=xl
    //                 y MUST BE VALID





	int loff=(y*srf->width+xl);
	int roff=(y*srf->width+xr);
    register int *left=(int *)srf->addr+ (loff>>3);
    register int *right=(int *)srf->addr+ (roff>>3);
    int ml=ggl_leftmask(loff),mr=ggl_rightmask(roff);


#ifndef NDEBUG
if( (left<srf->addr)||(right>srf->addr+2048)) {
    // BAD EXCEPTION
    return;
}


#endif




    if(left==right) {
        // single word operation
        ml|=mr;
        *left=  (*left & ml) | (color & (~ml));
        return;
    }

    *left=  (*left & ml) | (color & (~ml));
    ++left;
    while(left!=right)
    {
        *left=color;
        ++left;
    }

    *right=  (*right & mr) | (color & (~mr));

}

