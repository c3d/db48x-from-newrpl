
#include <ggl.h>


int ggl_getnib(int *buf,int addr)
{
register char *ptr=((char *)buf)+(addr>>1);
register int a=(int)(*ptr);
if(addr&1) a>>=4;
return a&0xf;
}
