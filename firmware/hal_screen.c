#include <ui.h>
#include <newrpl.h>

// THIS IS THE MAIN STABLE API TO ACCESS THE SCREEN


// SET TO SHOW/HIDE THE NOTIFICATION ICON

void halSetNotification(enum halNotification type,int show)
{
    if(type<N_DISKACCESS) {
        unsigned char *scrptr=(unsigned char *)MEM_PHYS_SCREEN;
        scrptr+=65;
        scrptr+=type*80;
        if(show) *scrptr|=0xf0;
        else *scrptr&=0x0f;
        return;
    }
    else {
        // TODO: DRAW CUSTOM ICONS INTO THE STATUS AREA FOR ALL OTHER ANNUNCIATORS

    }
}

int halGetNotification(enum halNotification type)
{
    if(type<N_DISKACCESS) {
        unsigned char *scrptr=(unsigned char *)MEM_PHYS_SCREEN;
        scrptr+=65;
        scrptr+=type*80;
        if(*scrptr&0xf0) return 1;
        else return 0;
    }
    else {
        // TODO: IMPLEMENT CUSTOM ANNUNCIATORS
    }
    return 0;
}

void halSetStackHeight(int h)
{
    int total;
    halScreen.Stack=h;
    total=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1+halScreen.Menu2;
    if(total!=SCREEN_HEIGHT) {
        if(halScreen.Form) { halScreen.Form+=SCREEN_HEIGHT-total;     halScreen.DirtyFlag|=FORM_DIRTY; }

        else halScreen.Stack=SCREEN_HEIGHT-halScreen.CmdLine-halScreen.Menu1-halScreen.Menu2;
        if(halScreen.Form<0) {
            halScreen.Stack+=halScreen.Form;
            halScreen.Form=0;
        }
    }
    halScreen.DirtyFlag|=STACK_DIRTY;
}


void halSetFormHeight(int h)
{
    int total;
    if(h<0) h=0;
    halScreen.Form=h;
    total=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1+halScreen.Menu2;
    if(total!=SCREEN_HEIGHT) {
        if(halScreen.Stack) { halScreen.Stack+=SCREEN_HEIGHT-total;  halScreen.DirtyFlag|=STACK_DIRTY; }

        else halScreen.Form=SCREEN_HEIGHT-halScreen.CmdLine-halScreen.Menu1-halScreen.Menu2;
        if(halScreen.Stack<0) {
            halScreen.Form+=halScreen.Stack;
            halScreen.Stack=0;
        }
    }
    halScreen.DirtyFlag|=FORM_DIRTY;

}

// MENU1 AREA IS USUALLY FIXED TO 1 LINE, BUT THIS IS GENERIC CODE
void halSetMenu1Height(int h)
{
    int total;
    if(h<0) h=0;
    halScreen.Menu1=h;
    total=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1+halScreen.Menu2;
    while(total!=SCREEN_HEIGHT) {
        // STRETCH THE STACK FIRST (IF ACTIVE), THEN FORM

        if(halScreen.Stack) {
            halScreen.Stack+=SCREEN_HEIGHT-total;
            halScreen.DirtyFlag|=STACK_DIRTY;

            if(halScreen.Stack<0) halScreen.Stack=0;
        }
        else {
            halScreen.Form+=SCREEN_HEIGHT-total;
            halScreen.DirtyFlag|=FORM_DIRTY;

            if(halScreen.Form<0) {
                halScreen.Menu1+=halScreen.Form;
                halScreen.Form=0;
            }
        }
        total=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1+halScreen.Menu2;
    }
    halScreen.DirtyFlag|=MENU1_DIRTY;

}

void halSetMenu2Height(int h)
{
    int total;
    if(h<0) h=0;
    halScreen.Menu2=h;
    total=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1+halScreen.Menu2;
    while(total!=SCREEN_HEIGHT) {
        // STRETCH THE STACK FIRST (IF ACTIVE), THEN FORM

        if(halScreen.Stack) {
            halScreen.Stack+=SCREEN_HEIGHT-total;
            halScreen.DirtyFlag|=STACK_DIRTY;
            if(halScreen.Stack<0) halScreen.Stack=0;
        }
        else {
            halScreen.Form+=SCREEN_HEIGHT-total;
            halScreen.DirtyFlag|=FORM_DIRTY;

            if(halScreen.Form<0) {
                halScreen.Menu2+=halScreen.Form;
                halScreen.Form=0;
            }
        }
        total=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1+halScreen.Menu2;
    }
    halScreen.DirtyFlag|=MENU2_DIRTY|STAREA_DIRTY;

}

void halSetCmdLineHeight(int h)
{
    int total;
    if(h<0) h=0;
    halScreen.CmdLine=h;
    total=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1+halScreen.Menu2;
    while(total!=SCREEN_HEIGHT) {
        // STRETCH THE STACK FIRST (IF ACTIVE), THEN FORM

        if(halScreen.Stack) {
            halScreen.Stack+=SCREEN_HEIGHT-total;
            halScreen.DirtyFlag|=STACK_DIRTY;

            if(halScreen.Stack<0) halScreen.Stack=0;
        }
        else {
            halScreen.Form+=SCREEN_HEIGHT-total;
            halScreen.DirtyFlag|=FORM_DIRTY;

            if(halScreen.Form<0) {
                halScreen.Menu2+=halScreen.Form;
                halScreen.Form=0;
            }
        }
        total=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1+halScreen.Menu2;
    }
    halScreen.DirtyFlag|=CMDLINE_DIRTY;

}



// COMPUTE HEIGHT AND WIDTH OF OBJECT TO DISPLAY ON STACK

BINT halGetDispObjectHeight(WORDPTR object)
{
    // TODO: ADD PROPER FONT SELECTION, MULTILINE OBJECTS, ETC.

    return 7;
}


extern const UBINT64 const powersof10[20];

// CONVERT INTEGER NUMBER INTO STRING FOR STACK LEVEL
// str MUST CONTAIN AT LEAST 15: BYTES "-1,345,789,123[NULL]"
void halInt2String(int num,char *str)
{
    int pow10idx=9;  // START WITH 10^10
    int digit,firstdigit;
    char *ptr=str;
    if(num<0) { *ptr++='-'; num=-num; }

    firstdigit=1;
    do {
        digit=0;
        while(num>=powersof10[pow10idx]) { ++digit; num-=powersof10[pow10idx]; }
        if(!( (digit==0) && firstdigit)) {
        *ptr++=digit+'0';
        firstdigit=0;
        }
        ++pow10idx;
    } while(num!=0);

    if(firstdigit) *ptr++='0';
    *ptr=0;
}

void halRedrawStack(DRAWSURFACE *scr)
{
    if(halScreen.Stack==0) {
        halScreen.DirtyFlag&=~STACK_DIRTY;
        return;
    }
  int ystart=halScreen.Form,yend=ystart+halScreen.Stack;
  int depth=rplDepthData(),level=1;
  int objheight,ytop,y,numwidth,xright,xobj;
  char num[16];
  WORDPTR string;
  BINT nchars;
  BYTEPTR charptr;



  y=yend;

  while(y>ystart) {
      // DRAW THE NUMBER
      if(level<=depth) objheight=halGetDispObjectHeight(rplPeekData(level));
      else objheight=((FONTDATA *)System7Font)->BitmapHeight;
      ytop=y-objheight;
      halInt2String(level,num);
      numwidth=StringWidth(num,(FONTDATA *)System7Font);
      xright=numwidth>12? numwidth:12;
      ggl_rect(scr,scr->clipx,ytop,scr->clipx2,y-1,0);  // CLEAR RECTANGLE
      DrawText(xright-numwidth,ytop,num,(FONTDATA *)System7Font,0xf,scr);
      ggl_vline(scr,xright,ytop,y-1,ggl_mkcolor(0x8));

      if(level<=depth) {
      // DRAW THE OBJECT


      // TODO: CHANGE DECOMPILE INTO PROPER DISPLAY FUNCTION
      string=rplDecompile(rplPeekData(level));

      if(string) {
      // NOW PRINT THE STRING OBJECT
          nchars=rplStrLen(string);
          charptr=(BYTEPTR) (string+1);
          numwidth=StringWidthN(charptr,nchars,(FONTDATA *)System7Font);
          xobj=SCREEN_WIDTH-numwidth;
          if(xobj<=xright) {
              xobj=xright+1;
              // TODO: OBJECT WILL BE TRUNCATED ON THE RIGHT
          }
          DrawTextN(xobj,ytop,charptr,nchars,(FONTDATA *)System7Font,15,scr);
      }
      else {
          DrawText(SCREEN_WIDTH-44,ytop,"~~Unknown~~",(FONTDATA *)System7Font,15,scr);
      }
       }
        y=ytop;
        ++level;
  }

    halScreen.DirtyFlag&=~STACK_DIRTY;
}


// INITIALIZE DEFAULT SCREEN PARAMETERS

void halInitScreen()
{
halScreen.CmdLine=0;
halScreen.Menu1=7;
halScreen.Menu2=13;
halScreen.Stack=1;
halSetFormHeight(0);
halScreen.DirtyFlag=STACK_DIRTY|MENU1_DIRTY|MENU2_DIRTY;
halScreen.SAreaTimer=0;
halScreen.CursorTimer=0;
halScreen.FormFont=halScreen.StackFont=halScreen.Stack1Font=(FONTDATA *)System7Font;
halScreen.MenuFont=(FONTDATA *)System5Font;
halScreen.StAreaFont=(FONTDATA *)MiniFont;
halScreen.CmdLineFont=(FONTDATA *)System7Font;
}


// REDRAW THE VARS MENU
void halRedrawMenu1(DRAWSURFACE *scr)
{
    if(halScreen.Menu1==0) {
        halScreen.DirtyFlag&=~MENU1_DIRTY;
        return;
    }

// TODO: EVERYTHING, SHOW EMPTY MENU FOR NOW
    int ytop,ybottom;
    ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine;
    ybottom=ytop+halScreen.Menu1-1;
    // DRAW BACKGROUND
    ggl_rect(scr,0,ytop,SCREEN_WIDTH-1,ybottom-1,ggl_mkcolor(0xf));
    ggl_hline(scr,ybottom,0,SCREEN_WIDTH-1,0);
    ggl_vline(scr,21,ytop,ybottom,0);
    ggl_vline(scr,43,ytop,ybottom,0);
    ggl_vline(scr,65,ytop,ybottom,0);
    ggl_vline(scr,87,ytop,ybottom,0);
    ggl_vline(scr,109,ytop,ybottom,0);

    halScreen.DirtyFlag&=~MENU1_DIRTY;
}

// REDRAW THE OTHER MENU
void halRedrawMenu2(DRAWSURFACE *scr)
{
    if(halScreen.Menu2==0) {
        halScreen.DirtyFlag&=~MENU2_DIRTY;
        return;
    }
// TODO: EVERYTHING, SHOW EMPTY MENU FOR NOW
    int ytop,ybottom,y;
    ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
    ybottom=ytop+halScreen.Menu2-1;
    // DRAW BACKGROUND
    ggl_rect(scr,0,ytop,64,ybottom,ggl_mkcolor(0x8));
    ggl_vline(scr,21,ytop,ybottom,0);
    ggl_vline(scr,43,ytop,ybottom,0);
    ggl_vline(scr,65,ytop,ybottom,0);
//    ggl_vline(scr,87,ytop,ybottom,0);
//    ggl_vline(scr,109,ytop,ybottom,0);
    ggl_hline(scr,ytop+6,0,64,0);

    halScreen.DirtyFlag&=~MENU2_DIRTY;
}

void halRedrawStatus(DRAWSURFACE *scr)
{
    if(halScreen.Menu2) {
    int ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
    ggl_rect(scr,STATUSAREA_X,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);

    }
    // TODO: SHOW THE CURRENT DIR, ETC. HERE

    halScreen.DirtyFlag&=~STAREA_DIRTY;

}

void halRedrawCmdLine(DRAWSURFACE *scr)
{
    if(halScreen.CmdLine) {
    int ytop=halScreen.Form+halScreen.Stack;
    if((halScreen.DirtyFlag&CMDLINE_ALLDIRTY)==CMDLINE_ALLDIRTY) {
        ggl_rect(scr,0,ytop,SCREEN_WIDTH-1,ytop+halScreen.CmdLine-1,0);
        ggl_hline(scr,ytop,0,SCREEN_WIDTH-1,0xf0f0f0f0);
    }

    BINT y=(halScreen.LineCurrent-halScreen.LineVisible)*halScreen.CmdLineFont->BitmapHeight;
    BYTEPTR cmdline=(BYTEPTR) (CmdLineCurrentLine+1);
    BINT nchars=rplStrLen(CmdLineCurrentLine);

    // TODO: SHOW OTHER LINES HERE

    if(halScreen.DirtyFlag&CMDLINE_LINEDIRTY) {
    // UPDATE THE CURRENT LINE
    DrawTextBkN(-halScreen.XVisible,ytop+2+y,cmdline,nchars,(FONTDATA *)halScreen.CmdLineFont,0xf,0x0,scr);
    }

    if(halScreen.DirtyFlag&CMDLINE_CURSORDIRTY) {
    // DRAW THE CURSOR
    if(!(halScreen.CursorState&0x8000)) DrawTextBkN(halScreen.CursorX-halScreen.XVisible,ytop+2+y,&halScreen.CursorState,1,(FONTDATA *)halScreen.CmdLineFont,0x0,0xf,scr);

    else {
        scr->clipx=halScreen.CursorX-halScreen.XVisible;
        scr->clipx2=scr->clipx+8;   // HARD CODED MAXIMUM WIDTH OF THE CURSOR
        if(scr->clipx2>=SCREEN_WIDTH) scr->clipx2=SCREEN_WIDTH-1;

        ggl_cliprect(scr,halScreen.CursorX-halScreen.XVisible,ytop+2+y,halScreen.CursorX-halScreen.XVisible+StringWidthN(&halScreen.CursorState,1,halScreen.CmdLineFont)-1,ytop+2+y+halScreen.CmdLineFont->BitmapHeight-1,0);

        // EITHER DON'T DRAW IT OR REDRAW THE PORTION OF COMMAND LINE UNDER THE CURSOR
        if(!(halScreen.DirtyFlag&CMDLINE_LINEDIRTY))
        {
            // THE LINE WAS NOT UPDATED, MEANS WE ARE UPDATING ONLY THE CURSOR
            // UPDATE THE CURRENT LINE
            DrawTextBkN(-halScreen.XVisible,ytop+2+y,cmdline,nchars,(FONTDATA *)halScreen.CmdLineFont,0xf,0x0,scr);
        }

        // RESET THE CLIPPING RECTANGLE BACK TO WHOLE SCREEN
        scr->clipx=0;
        scr->clipx2=SCREEN_WIDTH-1;
        scr->clipy=0;
        scr->clipy2=SCREEN_HEIGHT-1;


    }
    }

    }

    halScreen.DirtyFlag&=~CMDLINE_ALLDIRTY;

}



void halForceRedrawAll(DRAWSURFACE *scr)
{
    halRedrawStack(scr);
    halRedrawCmdLine(scr);
    halRedrawMenu1(scr);
    halRedrawMenu2(scr);
    halRedrawStatus(scr);
}

void halRedrawAll(DRAWSURFACE *scr)
{
    if(halScreen.DirtyFlag&STACK_DIRTY) halRedrawStack(scr);
    if(halScreen.DirtyFlag&CMDLINE_ALLDIRTY) halRedrawCmdLine(scr);
    if(halScreen.DirtyFlag&MENU1_DIRTY) halRedrawMenu1(scr);
    if(halScreen.DirtyFlag&MENU2_DIRTY) halRedrawMenu2(scr);
    if(halScreen.DirtyFlag&STAREA_DIRTY) halRedrawStatus(scr);
}

void status_popup_handler()
{
    if(halScreen.DirtyFlag&STAREA_DIRTY) {
        DRAWSURFACE scr;
        ggl_initscr(&scr);
        halRedrawStatus(&scr);
    }
    halScreen.SAreaTimer=0;
}


// WILL KEEP THE STATUS AREA AS-IS FOR 5 SECONDS, THEN REDRAW IT
// TO CLEAN UP POP-UP MESSAGES
void halStatusAreaPopup()
{
    halScreen.DirtyFlag|=STAREA_DIRTY;
    if(halScreen.SAreaTimer) {
        tmr_eventkill(halScreen.SAreaTimer);
        //tmr_eventpause(halScreen.SAreaTimer);
        //tmr_eventresume(halScreen.SAreaTimer);      // PAUSE/RESUME WILL RESTART THE 5 SECOND COUNT
        //return;
    }
    halScreen.SAreaTimer=tmr_eventcreate(&status_popup_handler,5000,0);
}
