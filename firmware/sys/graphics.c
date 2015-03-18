// BASIC GRAPHICS ROUTINES
#include <ui.h>



static unsigned int gui_chgcolorfilter(unsigned int dest,unsigned int src,int param)
{
    return ggl_opmaskcol(dest,src,0,param);
}

// DRAW TEXT WITH TRANSPARENT BACKGROUND

void DrawText(int x,int y,char *Text,FONTDATA *Font,int color,DRAWSURFACE *drawsurf)
{
    //if(Font->Signature!=0x544E4647) halSetNotification(0,1);

    if(drawsurf->clipx<0) return;

    if(y>drawsurf->clipy2) return;
    if(y+(int)Font->BitmapHeight<=drawsurf->clipy) return;

    int w,h;
    gglsurface srf;
    srf.addr=(int *)Font->Bitmap;
    srf.width=Font->BitmapWidth;
    srf.y=0;

    h=Font->BitmapHeight;

    if(y<drawsurf->clipy) {
        h-=drawsurf->clipy-y;
        srf.y=drawsurf->clipy-y;
        y=drawsurf->clipy;
    }
    if(y+h-1>drawsurf->clipy2) h=drawsurf->clipy2-y+1;

    drawsurf->y=y;
    drawsurf->x=x;

    while(*Text) {
        if(*Text=='\n' || *Text=='\r') return;
    w=Font->WidthMap[(int)(*Text)];
    srf.x=w&0xffff;
    w>>=16;
    if(drawsurf->x>drawsurf->clipx2) return;
    if(drawsurf->x+w-1<drawsurf->clipx) return;
    if(drawsurf->x<drawsurf->clipx) {
            srf.x+=drawsurf->clipx-drawsurf->x;
            w-=drawsurf->clipx-drawsurf->x;
            drawsurf->x=drawsurf->clipx;
    }
    if(drawsurf->x+w-1>drawsurf->clipx2) w=drawsurf->clipx2-drawsurf->x+1;

    if((color&0xf)==0xf) ggl_bitbltmask(drawsurf,&srf,w,h,0);
    else ggl_bitbltoper(drawsurf,&srf,w,h,color&0xf,&gui_chgcolorfilter);
    drawsurf->x+=w;
    ++Text;

    }
    return;


}

// DRAW TEXT WITH SOLID BACKGROUND


void DrawTextBk(int x,int y,char *Text,FONTDATA *Font,int color,int bkcolor,DRAWSURFACE *drawsurf)
{

    if(drawsurf->clipx<0) return;

    if(y>drawsurf->clipy2) return;
    if(y+(int)Font->BitmapHeight<=drawsurf->clipy) return;

    int w,h;
    gglsurface srf;
    srf.addr=(int *)Font->Bitmap;
    srf.width=Font->BitmapWidth;
    srf.y=0;

    h=Font->BitmapHeight;

    if(y<drawsurf->clipy) {
        h-=drawsurf->clipy-y;
        srf.y=drawsurf->clipy-y;
        y=drawsurf->clipy;
    }
    if(y+h-1>drawsurf->clipy2) h=drawsurf->clipy2-y+1;

    drawsurf->y=y;
    drawsurf->x=x;

    while(*Text) {
        if(*Text=='\n' || *Text=='\r') return;
    w=Font->WidthMap[(int)(*Text)];
    srf.x=w&0xffff;
    w>>=16;
    if(drawsurf->x>drawsurf->clipx2) return;
    if(drawsurf->x+w-1<drawsurf->clipx) return;
    if(drawsurf->x<drawsurf->clipx) {
            srf.x+=drawsurf->clipx-drawsurf->x;
            w-=drawsurf->clipx-drawsurf->x;
            drawsurf->x=drawsurf->clipx;
    }
    if(drawsurf->x+w-1>drawsurf->clipx2) w=drawsurf->clipx2-drawsurf->x+1;

    ggl_rect(drawsurf,drawsurf->x,drawsurf->y,drawsurf->x+w-1,drawsurf->y+h-1,ggl_mkcolor(bkcolor&0xf));
    if((color&0xf)==0xf) ggl_bitbltmask(drawsurf,&srf,w,h,0);
    else ggl_bitbltoper(drawsurf,&srf,w,h,color&0xf,&gui_chgcolorfilter);
    drawsurf->x+=w;
    ++Text;

    }
    return;


}

// DRAWS TEXT TO A 1-BIT MONOCHROME SURFACE
// TRANSPARENT BACKGROUND
void DrawTextMono(int x,int y,char *Text,FONTDATA *Font,int color,DRAWSURFACE *drawsurf)
{
    if(drawsurf->clipx<0) return;

    if(y>drawsurf->clipy2) return;
    if(y+(int)Font->BitmapHeight<=drawsurf->clipy) return;

    int w,h;
    gglsurface srf;
    srf.addr=(int *)Font->Bitmap;
    srf.width=Font->BitmapWidth;
    srf.y=0;

    h=Font->BitmapHeight;

    if(y<drawsurf->clipy) {
        h-=drawsurf->clipy-y;
        srf.y=drawsurf->clipy-y;
        y=drawsurf->clipy;
    }
    if(y+h-1>drawsurf->clipy2) h=drawsurf->clipy2-y+1;

    drawsurf->y=y;
    drawsurf->x=x;

    while(*Text) {
        if(*Text=='\n' || *Text=='\r') return;
    w=Font->WidthMap[(int)(*Text)];
    srf.x=w&0xffff;
    w>>=16;
    if(drawsurf->x>drawsurf->clipx2) return;
    if(drawsurf->x+w-1<drawsurf->clipx) return;
    if(drawsurf->x<drawsurf->clipx) {
            srf.x+=drawsurf->clipx-drawsurf->x;
            w-=drawsurf->clipx-drawsurf->x;
            drawsurf->x=drawsurf->clipx;
    }
    if(drawsurf->x+w-1>drawsurf->clipx2) w=drawsurf->clipx2-drawsurf->x+1;

    int address,f,k;
    unsigned int destword;
    // OFFSET TO THE FIRST SCAN IN PIXELS
    for(k=0;k<h;++k) {
        address=srf.x+(srf.y+k)*srf.width;
        destword=0;
    for(f=0;f<w;++f) {
        if(ggl_getnib(srf.addr,address)) {
            // PLOT A PIXEL ON DESTINATION
            destword|=1<<f;
        }
        ++address;
    }

    unsigned char *cptr=(unsigned char *)drawsurf->addr;
    int offset=drawsurf->x+(drawsurf->y+k)*drawsurf->width;
    cptr+=offset>>3;
    offset&=7;

    // NOW ROTATE DESTINATION
    destword<<=offset;
    // THIS ONLY WORKS FOR FONTS WITH UP TO 8 PIXELS WIDE CHARACTERS
    if(color) {
        // BLACK LETTERS ON TRANSPARENT BACKGROUND
    *cptr|=destword;
    if(destword>>8) {
        cptr[1]|=(destword>>8);
    }
    } else {
        // WHITE LETTERS ON TRANSPARENT BACKGROUND
        destword=~destword;
        *cptr&=destword;
        if(destword>>8) {
            cptr[1]&=destword>>8;
        }

    }
    }

    drawsurf->x+=w;
    ++Text;

    }
    return;


}

// CALCULATE WIDTH OF NULL-TERMINATED STRING WITH PROPORTIONAL FONT (IN PIXELS)

int StringWidth(char *Text,FONTDATA *Font)
{
    int w=0;

    while(*Text) {
        if(*Text=='\n' || *Text=='\r') return w;
        w+=(Font->WidthMap[(int)(*Text)])>>16;
        ++Text;
    }
    return w;
}

// CALCULATE WIDTH OF STRING WITH nchars WITH PROPORTIONAL FONT (IN PIXELS)
// DOES NOT STOP FOR NULL CHARACTER
// IT DOES STOP AT NEWLINES
int StringWidthN(char *Text,int nchars,FONTDATA *Font)
{
    int w=0;

    while(nchars>0) {
        if(*Text=='\n' || *Text=='\r') return w;
        w+=(Font->WidthMap[(int)(*Text)])>>16;
        ++Text;
        --nchars;
    }
    return w;
}

// DRAW TEXT WITH TRANSPARENT BACKGROUND
// STRING LENGTH IS nchars
// DOES NOT STOP DUE TO NULL CHARACTER

void DrawTextN(int x,int y,char *Text,int nchars,FONTDATA *Font,int color,DRAWSURFACE *drawsurf)
{
    if(drawsurf->clipx<0) return;

    if(y>drawsurf->clipy2) return;
    if(y+(int)Font->BitmapHeight<=drawsurf->clipy) return;

    int w,h;
    gglsurface srf;
    srf.addr=(int *)Font->Bitmap;
    srf.width=Font->BitmapWidth;
    srf.y=0;

    h=Font->BitmapHeight;

    if(y<drawsurf->clipy) {
        h-=drawsurf->clipy-y;
        srf.y=drawsurf->clipy-y;
        y=drawsurf->clipy;
    }
    if(y+h-1>drawsurf->clipy2) h=drawsurf->clipy2-y+1;

    drawsurf->y=y;
    drawsurf->x=x;

    while(nchars>0) {
        if(*Text=='\n' || *Text=='\r') return;
    w=Font->WidthMap[(int)(*Text)];
    srf.x=w&0xffff;
    w>>=16;
    if(drawsurf->x>drawsurf->clipx2) return;
    if(drawsurf->x+w-1<drawsurf->clipx) { drawsurf->x+=w; ++Text; --nchars; continue; }
    if(drawsurf->x<drawsurf->clipx) {
            srf.x+=drawsurf->clipx-drawsurf->x;
            w-=drawsurf->clipx-drawsurf->x;
            drawsurf->x=drawsurf->clipx;
    }
    if(drawsurf->x+w-1>drawsurf->clipx2) w=drawsurf->clipx2-drawsurf->x+1;

    if((color&0xf)==0xf) ggl_bitbltmask(drawsurf,&srf,w,h,0);
    else ggl_bitbltoper(drawsurf,&srf,w,h,color&0xf,&gui_chgcolorfilter);
    drawsurf->x+=w;
    ++Text;
    --nchars;
    }
    return;


}

// DRAW TEXT WITH SOLID BACKGROUND
// DRAWS nchars, DOES NOT STOP FOR NULL CHARACTERS

void DrawTextBkN(int x,int y,char *Text,int nchars,FONTDATA *Font,int color,int bkcolor,DRAWSURFACE *drawsurf)
{

    if(drawsurf->clipx<0) return;

    if(y>drawsurf->clipy2) return;
    if(y+(int)Font->BitmapHeight<=drawsurf->clipy) return;

    int w,h;
    gglsurface srf;
    srf.addr=(int *)Font->Bitmap;
    srf.width=Font->BitmapWidth;
    srf.y=0;

    h=Font->BitmapHeight;

    if(y<drawsurf->clipy) {
        h-=drawsurf->clipy-y;
        srf.y=drawsurf->clipy-y;
        y=drawsurf->clipy;
    }
    if(y+h-1>drawsurf->clipy2) h=drawsurf->clipy2-y+1;

    drawsurf->y=y;
    drawsurf->x=x;

    while(nchars>0) {
        if(*Text=='\n' || *Text=='\r') return;
    w=Font->WidthMap[(int)(*Text)];
    srf.x=w&0xffff;
    w>>=16;
    if(drawsurf->x>drawsurf->clipx2) return;
    if(drawsurf->x+w-1<drawsurf->clipx) { drawsurf->x+=w; ++Text; --nchars; continue; }
    if(drawsurf->x<drawsurf->clipx) {
            srf.x+=drawsurf->clipx-drawsurf->x;
            w-=drawsurf->clipx-drawsurf->x;
            drawsurf->x=drawsurf->clipx;
    }
    if(drawsurf->x+w-1>drawsurf->clipx2) w=drawsurf->clipx2-drawsurf->x+1;

    ggl_rect(drawsurf,drawsurf->x,drawsurf->y,drawsurf->x+w-1,drawsurf->y+h-1,ggl_mkcolor(bkcolor&0xf));
    if((color&0xf)==0xf) ggl_bitbltmask(drawsurf,&srf,w,h,0);
    else ggl_bitbltoper(drawsurf,&srf,w,h,color&0xf,&gui_chgcolorfilter);
    drawsurf->x+=w;
    ++Text;
    --nchars;
    }
    return;


}
