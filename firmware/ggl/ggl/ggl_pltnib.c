
#include <ggl.h>


void ggl_pltnib(int *buf,int addr,int color)
{
register char *ptr=((char *)buf)+(addr>>1);
if(addr&1) *ptr= (*ptr&0xf) | ((color<<4)&0xf0);
else *ptr= (*ptr&0xf0) | (color&0xf);
}
