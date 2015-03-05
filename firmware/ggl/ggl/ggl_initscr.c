
#include <ui.h>

void ggl_initscr(gglsurface *srf)
{
srf->addr=(int *)MEM_PHYS_SCREEN;
srf->width=LCD_W;
srf->x=srf->y=0;
srf->clipx=srf->clipy=0;
srf->clipx2=USERSCREEN_W-1;
srf->clipy2=USERSCREEN_H-1;
}
