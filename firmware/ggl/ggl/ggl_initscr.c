
#include <ui.h>

void ggl_initscr(gglsurface *srf)
{
srf->addr=(int *)MEM_PHYS_SCREEN;
srf->width=LCD_W;
srf->x=srf->y=0;
srf->clipx=srf->clipy=0;
srf->clipx2=SCREEN_WIDTH-1;
srf->clipy2=SCREEN_HEIGHT-1;
}
