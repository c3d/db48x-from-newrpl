
#include <ggl.h>

unsigned int ggl_opmask(unsigned int dest,unsigned int src, int tcol)
{
    // APPLY 100% TRANSPARENCY MASK
    // tcol = TRANSPARENT COLOR IN src
    register int f;
    register unsigned int res  =0;
    for(f=0;f<8;++f,src>>=4,dest>>=4)
    {
        if((src&0xf)==(unsigned int)tcol) res|=dest&0xf;
        else res|=src&0xf;
        res= (res>>4) | (res<<28);


    }
    return res;
}


// COPY src INTO dest, WITH tcol=TRANSPARENT COLOR IN src, AND THEN REPLACE BLACK COLOR IN src
// WITH THE COLOR GIVEN IN newcolor

unsigned int ggl_opmaskcol(unsigned int dest,unsigned int src, int tcol, int newcolor)
{
    // APPLY 100% TRANSPARENCY MASK
    // tcol = TRANSPARENT COLOR IN src
    register int f;
    register unsigned int res  =0;
    for(f=0;f<8;++f,src>>=4,dest>>=4)
    {
        if((src&0xf)==(unsigned int)tcol) res|=dest&0xf;
        else res|=src&newcolor;
        res= (res>>4) | (res<<28);
    }
    return res;
}


