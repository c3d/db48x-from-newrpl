/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


// BASIC GRAPHICS ROUTINES
#include <ui.h>



static unsigned int gui_chgcolorfilter(unsigned int dest,unsigned int src,int param)
{
    return ggl_opmaskcol(dest,src,0,param);
}

// DRAW TEXT WITH TRANSPARENT BACKGROUND
/*
void DrawText(int x,int y,char *Text,UNIFONT *Font,int color,DRAWSURFACE *drawsurf)
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


void DrawTextBk(int x,int y,char *Text,UNIFONT *Font,int color,int bkcolor,DRAWSURFACE *drawsurf)
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


// CALCULATE WIDTH OF NULL-TERMINATED STRING WITH PROPORTIONAL FONT (IN PIXELS)

int StringWidth(char *Text,UNIFONT *Font)
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
int StringWidthN(char *Text,int nchars,UNIFONT *Font)
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

void DrawTextN(int x,int y,char *Text,int nchars,UNIFONT *Font,int color,DRAWSURFACE *drawsurf)
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

void DrawTextBkN(int x,int y,char *Text,int nchars,UNIFONT *Font,int color,int bkcolor,DRAWSURFACE *drawsurf)
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
*/
// NEW VERSION WITH UNICODE SUPPORT


int StringWidthN(char *Text,char *End,UNIFONT *Font)
{
    int cp,startcp,rangeend,offset,cpinfo;
    unsigned short *offtable;
    unsigned int *mapptr;
    int w,width=0;

    offtable=(unsigned short *)(((unsigned int *)Font)+Font->OffsetTable);

    while(Text<End) {

        cp=utf82cp(Text,End);

        if(cp==-1) { ++Text; continue; }

        if(cp=='\n' || cp=='\r') return width;

        cpinfo=getCPInfo(cp);

        if(CCLASS(cpinfo)!=0) {
        // ADD SUPPORT FOR COMBINERS
            Text=utf8skip(Text,End);
            continue;
        }


        // GET THE INFORMATION FROM THE FONT
        rangeend=0;
        mapptr=Font->MapTable-1;
        do {
            ++mapptr;
            startcp=rangeend;
            rangeend=startcp+RANGE_LEN(*mapptr);
        } while(cp>=rangeend);

        offset=FONT_OFFSET(*mapptr);
        if(offset==0xfff) w=offtable[0];
        else {
            w=offtable[offset+cp-startcp];
        }

        width+=w>>12;
        Text=utf8skip(Text,End);

    }

    return width;

}

// GET A POINTER INTO THE CHARACTER THAT WILL BE PRINTED AT *xcoord
// ASSUMING x=0 AT Text START
// IT WILL RETURN A POINTER INTO THE STRING, AND MODIFY *xcoord WITH THE CORRECT X COORDINATE
// ALIGNED WITH THE FONT

// WARNING: DO NOT PASS xcoord=NULL, NO ARGUMENT CHECKS

char *StringCoordToPointer(char *Text,char *End,UNIFONT *Font,int *xcoord)
{
    int cp,startcp,rangeend,offset,cpinfo;
    unsigned short *offtable;
    unsigned int *mapptr;
    int w,width=0;

    if(*xcoord<0) { *xcoord=0; return Text; }

    offtable=(unsigned short *)(((unsigned int *)Font)+Font->OffsetTable);

    while(Text<End) {

        cp=utf82cp(Text,End);

        if(cp==-1) { ++Text; continue; }

        if(cp=='\n' || cp=='\r') { *xcoord=width; return Text; }

        cpinfo=getCPInfo(cp);
        if(CCLASS(cpinfo)!=0) {
        // ADD SUPPORT FOR COMBINERS
            Text=utf8skip(Text,End);
            continue;
        }


        // GET THE INFORMATION FROM THE FONT
        rangeend=0;
        mapptr=Font->MapTable-1;
        do {
            ++mapptr;
            startcp=rangeend;
            rangeend=startcp+RANGE_LEN(*mapptr);
        } while(cp>=rangeend);

        offset=FONT_OFFSET(*mapptr);
        if(offset==0xfff) w=offtable[0];
        else {
            w=offtable[offset+cp-startcp];
        }

        if( (*xcoord>=width) && (*xcoord<width+(w>>12))) { *xcoord=width; return Text; }

        width+=w>>12;

        Text=utf8skip(Text,End);

    }

    *xcoord=width;
    return End;


}

int StringWidth(char *Text,UNIFONT *Font)
{
    char *End=Text;
    while(*End) ++End;
    return StringWidthN(Text,End,Font);
}


// DRAW TEXT WITH TRANSPARENT BACKGROUND
// UTF8 STRING

void DrawTextN(int x,int y,char *Text,char *End,UNIFONT *Font,int color,DRAWSURFACE *drawsurf)
{
    int cp,startcp,rangeend,offset,cpinfo;
    unsigned short *offtable;
    unsigned int *mapptr;
    char *fontbitmap;

    if(drawsurf->clipx<0) return;

    if(y>drawsurf->clipy2) return;
    if(y+(int)Font->BitmapHeight<=drawsurf->clipy) return;


    fontbitmap=(char *)(((unsigned int *)Font)+Font->OffsetBitmap);
    offtable=(unsigned short *)(((unsigned int *)Font)+Font->OffsetTable);

    int w,clipped=0,h;
    gglsurface srf;
    srf.addr=(int *)fontbitmap;
    srf.width=Font->BitmapWidth<<3;
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

    while(Text<End) {

        cp=utf82cp(Text,End);

        if(cp==-1) { ++Text; continue; }
        if(cp=='\n' || cp=='\r') return;

        cpinfo=getCPInfo(cp);
        // ADD SUPPORT FOR A COUPLE OF COMBINING MARKS
        switch(cp) {
        case 0x0305:        // COMBINING OVERLINE
            // HERE WE HAVE w THE WIDTH FROM PREVIOUS CHARACTER (POSSIBLY CLIPPED)
            // clipped&0xff = NUMBER OF PIXELS CROPPED ON THE RIGHT OF THE CHARACTER
            // clipped>>8 = NUMBER OF PIXELS CROPPED ON THE LEFT OF THE CHARACTER

            // w+(clipped&0xff)+(clipped>>8) = ORIGINAL WIDTH OF THE CHARACTER
            // drawsurf->x+(clipped&0xff) = POINTS TO THE RIGHT OF THE CHARACTER (POSSIBLY CLIPPED)

            ggl_cliphline(drawsurf,drawsurf->y,drawsurf->x-w,drawsurf->x-2+(clipped&0xff),ggl_mkcolor(color));
            break;
        }

        if(CCLASS(cpinfo)!=0) {
        // ADD SUPPORT FOR COMBINERS
            Text=utf8skip(Text,End);
            continue;
        }

        // GET THE INFORMATION FROM THE FONT
        rangeend=0;
        mapptr=Font->MapTable-1;
        do {
            ++mapptr;
            startcp=rangeend;
            rangeend=startcp+RANGE_LEN(*mapptr);
        } while(cp>=rangeend);

        offset=FONT_OFFSET(*mapptr);
        if(offset==0xfff) w=offtable[0];
        else {
            w=offtable[offset+cp-startcp];
        }

    srf.x=w&0xfff;
    w>>=12;
    clipped=0;
    if(w) {
    if(drawsurf->x>drawsurf->clipx2) return;
    if(drawsurf->x+w-1<drawsurf->clipx) { drawsurf->x+=w; Text=utf8skip(Text,End); continue; }
    if(drawsurf->x<drawsurf->clipx) {
            srf.x+=drawsurf->clipx-drawsurf->x;
            w-=drawsurf->clipx-drawsurf->x;
            clipped|=(drawsurf->clipx-drawsurf->x)<<8;     // CLIPPED ON THE LEFT
            drawsurf->x=drawsurf->clipx;
    }
    if(drawsurf->x+w-1>drawsurf->clipx2) { clipped|=w-(drawsurf->clipx2-drawsurf->x+1); w=drawsurf->clipx2-drawsurf->x+1;  }  // CLIPPED ON THE RIGHT

    // MONOCHROME TO 16-GRAYS BLIT W/CONVERSION
    if((color&0xf)==0xf) ggl_monobitbltmask(drawsurf,&srf,w,h,0);
    else ggl_monobitbltoper(drawsurf,&srf,w,h,color&0xf,&gui_chgcolorfilter);
    drawsurf->x+=w;
    }
    Text=utf8skip(Text,End);
    }
    return;


}

// DRAW TEXT WITH SOLID BACKGROUND
// UTF8 STRING
void DrawTextBkN(int x,int y,char *Text,char *End,UNIFONT *Font,int color,int bkcolor,DRAWSURFACE *drawsurf)
{
    int cp,startcp,rangeend,offset,cpinfo;
    unsigned short *offtable;
    unsigned int *mapptr;
    char *fontbitmap;

    if(drawsurf->clipx<0) return;

    if(y>drawsurf->clipy2) return;
    if(y+(int)Font->BitmapHeight<=drawsurf->clipy) return;


    fontbitmap=(char *)(((unsigned int *)Font)+Font->OffsetBitmap);
    offtable=(unsigned short *)(((unsigned int *)Font)+Font->OffsetTable);

    int w,clipped=0,h;
    gglsurface srf;
    srf.addr=(int *)fontbitmap;
    srf.width=Font->BitmapWidth<<3;
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


    while(Text<End) {

        cp=utf82cp(Text,End);

        if(cp==-1) { ++Text; continue; }

        if(cp=='\n' || cp=='\r') return;

        cpinfo=getCPInfo(cp);

        // ADD SUPPORT FOR A COUPLE OF COMBINING MARKS
        switch(cp) {
        case 0x0305:        // COMBINING OVERLINE
            // HERE WE HAVE w THE WIDTH FROM PREVIOUS CHARACTER (POSSIBLY CLIPPED)
            // clipped&0xff = NUMBER OF PIXELS CROPPED ON THE RIGHT OF THE CHARACTER
            // clipped>>8 = NUMBER OF PIXELS CROPPED ON THE LEFT OF THE CHARACTER

            // w+(clipped&0xff)+(clipped>>8) = ORIGINAL WIDTH OF THE CHARACTER
            // drawsurf->x+(clipped&0xff) = POINTS TO THE RIGHT OF THE CHARACTER (POSSIBLY CLIPPED)

            ggl_cliphline(drawsurf,drawsurf->y,drawsurf->x-w,drawsurf->x-2+(clipped&0xff),ggl_mkcolor(color));
            break;
        }

        if(CCLASS(cpinfo)!=0) {
        // ADD SUPPORT FOR COMBINERS
            Text=utf8skip(Text,End);
            continue;
        }

        // GET THE INFORMATION FROM THE FONT
        rangeend=0;
        mapptr=Font->MapTable-1;
        do {
            ++mapptr;
            startcp=rangeend;
            rangeend=startcp+RANGE_LEN(*mapptr);
        } while(cp>=rangeend);

        offset=FONT_OFFSET(*mapptr);
        if(offset==0xfff) w=offtable[0];
        else {
            w=offtable[offset+cp-startcp];
        }

    srf.x=w&0xfff;
    w>>=12;
    clipped=0;
    if(w) {

    if(drawsurf->x>drawsurf->clipx2) return;
    if(drawsurf->x+w-1<drawsurf->clipx) { drawsurf->x+=w; Text=utf8skip(Text,End); continue; }
    if(drawsurf->x<drawsurf->clipx) {
            srf.x+=drawsurf->clipx-drawsurf->x;
            w-=drawsurf->clipx-drawsurf->x;
            clipped|=(drawsurf->clipx-drawsurf->x)<<8;     // CLIPPED ON THE LEFT
            drawsurf->x=drawsurf->clipx;
    }
    if(drawsurf->x+w-1>drawsurf->clipx2) { clipped|=w-(drawsurf->clipx2-drawsurf->x+1); w=drawsurf->clipx2-drawsurf->x+1;  }  // CLIPPED ON THE RIGHT

    ggl_rect(drawsurf,drawsurf->x,drawsurf->y,drawsurf->x+w-1,drawsurf->y+h-1,ggl_mkcolor(bkcolor&0xf));
    // MONOCHROME TO 16-GRAYS BLIT W/CONVERSION
    if((color&0xf)==0xf) ggl_monobitbltmask(drawsurf,&srf,w,h,0);
    else ggl_monobitbltoper(drawsurf,&srf,w,h,color&0xf,&gui_chgcolorfilter);
    drawsurf->x+=w;
    }
    Text=utf8skip(Text,End);
    }
    return;


}

void DrawTextBk(int x,int y,char *Text,UNIFONT *Font,int color,int bkcolor,DRAWSURFACE *drawsurf)
{
    char *End=Text;

    while(*End) ++End;

    DrawTextBkN(x,y,Text,End,Font,color,bkcolor,drawsurf);
}
void DrawText(int x,int y,char *Text,UNIFONT *Font,int color,DRAWSURFACE *drawsurf)
{
    char *End=Text;

    while(*End) ++End;

    DrawTextN(x,y,Text,End,Font,color,drawsurf);
}


// DRAWS TEXT TO A 1-BIT MONOCHROME SURFACE
// TRANSPARENT BACKGROUND
void DrawTextMono(int x,int y,char *Text,UNIFONT *Font,int color,DRAWSURFACE *drawsurf)
{
    int cp,startcp,rangeend,offset,cpinfo;
    unsigned short *offtable;
    unsigned int *mapptr;
    char *fontbitmap;

    // FIND END OF STRING
    char *End=Text;
    while(*End) ++End;


    if(drawsurf->clipx<0) return;

    if(y>drawsurf->clipy2) return;
    if(y+(int)Font->BitmapHeight<=drawsurf->clipy) return;


    fontbitmap=(char *)(((unsigned int *)Font)+Font->OffsetBitmap);
    offtable=(unsigned short *)(((unsigned int *)Font)+Font->OffsetTable);


    int w,h;
    gglsurface srf;
    srf.addr=(int *)fontbitmap;
    srf.width=Font->BitmapWidth<<3;
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

    while(Text<End) {

        cp=utf82cp(Text,End);

        if(cp==-1) { ++Text; continue; }
        if(cp=='\n' || cp=='\r') return;


        cpinfo=getCPInfo(cp);

        if(CCLASS(cpinfo)!=0) {
        // ADD SUPPORT FOR COMBINERS
            Text=utf8skip(Text,End);
            continue;
        }

        // GET THE INFORMATION FROM THE FONT
        rangeend=0;
        mapptr=Font->MapTable-1;
        do {
            ++mapptr;
            startcp=rangeend;
            rangeend=startcp+RANGE_LEN(*mapptr);
        } while(cp>=rangeend);

        offset=FONT_OFFSET(*mapptr);
        if(offset==0xfff) w=offtable[0];
        else {
            w=offtable[offset+cp-startcp];
        }

    srf.x=w&0xfff;
    w>>=12;

    if(w) {

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
        if(ggl_getmonopix((char *)srf.addr,address)) {
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

    }

    Text=utf8skip(Text,End);

    }
    return;


}
