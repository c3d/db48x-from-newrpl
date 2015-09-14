/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <newrpl.h>
#include <ui.h>
#include <libraries.h>
// THIS IS THE MAIN STABLE API TO ACCESS THE SCREEN


// SET TO SHOW/HIDE THE NOTIFICATION ICON

void halSetNotification(enum halNotification type,int color)
{
    if(type<N_DISKACCESS) {
        unsigned char *scrptr=(unsigned char *)MEM_PHYS_SCREEN;
        scrptr+=65;
        scrptr+=type*80;
        *scrptr=(*scrptr&0xf)|(color<<4);
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
        return *scrptr>>4;
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
    halScreen.DirtyFlag|=MENU1_DIRTY|MENU2_DIRTY|STAREA_DIRTY;

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

BINT halGetDispObjectHeight(WORDPTR object,UNIFONT *font)
{
    UNUSED_ARGUMENT(object);
    // TODO: ADD MULTILINE OBJECTS, ETC.

    return font->BitmapHeight;
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
        while((UBINT64)num>=powersof10[pow10idx]) { ++digit; num-=powersof10[pow10idx]; }
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
  UNIFONT *levelfnt;


  y=yend;

  while(y>ystart) {
      if(level==1) levelfnt=halScreen.Stack1Font;
      else levelfnt=halScreen.StackFont;
      // DRAW THE NUMBER
      if(level<=depth) objheight=halGetDispObjectHeight(rplPeekData(level),levelfnt);
      else {
          objheight=levelfnt->BitmapHeight;
      }
      ytop=y-objheight;
      halInt2String(level,num);
      numwidth=StringWidth(num,levelfnt);
      xright=numwidth>12? numwidth:12;
      ggl_cliprect(scr,scr->clipx,ytop,scr->clipx2,y-1,0);  // CLEAR RECTANGLE
      DrawText(xright-numwidth,ytop,num,levelfnt,0xf,scr);
      ggl_clipvline(scr,xright,ytop,y-1,ggl_mkcolor(0x8));

      if(level<=depth) {
      // DRAW THE OBJECT


      // TODO: CHANGE DECOMPILE INTO PROPER DISPLAY FUNCTION
      string=rplDecompile(rplPeekData(level),0);

      if(string) {
      // NOW PRINT THE STRING OBJECT
          nchars=rplStrSize(string);
          charptr=(BYTEPTR) (string+1);
          numwidth=StringWidthN((char *)charptr,(char *)charptr+nchars,levelfnt);
          xobj=SCREEN_WIDTH-numwidth;
          if(xobj<=xright) {
              xobj=xright+1;
              // TODO: OBJECT WILL BE TRUNCATED ON THE RIGHT
          }
          DrawTextN(xobj,ytop,(char *)charptr,(char *)charptr+nchars,levelfnt,15,scr);
      }
      else {
          DrawText(SCREEN_WIDTH-44,ytop,"~~Unknown~~",levelfnt,15,scr);
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
halScreen.Menu1=MENU1_HEIGHT;
halScreen.Menu2=MENU2_HEIGHT;
halScreen.Stack=1;
halSetFormHeight(0);
halScreen.DirtyFlag=STACK_DIRTY|MENU1_DIRTY|MENU2_DIRTY;
halScreen.SAreaTimer=0;
halScreen.CursorTimer=-1;
halScreen.KeyContext=CONTEXT_STACK;
halScreen.FormFont=halScreen.StackFont=halScreen.Stack1Font=(UNIFONT *)Font_8C;
halScreen.MenuFont=(UNIFONT *)Font_6A;
halScreen.StAreaFont=(UNIFONT *)Font_6A;
halScreen.CmdLineFont=(UNIFONT *)Font_8C;
halScreen.Menu1List=0;
halScreen.Menu2Dir=0;
halScreen.Menu1Page=halScreen.Menu2Page=0;
halSetNotification(N_LEFTSHIFT,0);
halSetNotification(N_RIGHTSHIFT,0);
halSetNotification(N_ALPHA,0);
halSetNotification(N_LOWBATTERY,0);
halSetNotification(N_HOURGLASS,0);
halSetNotification(N_DISKACCESS,0);

// NOT NECESSARILY PART OF HALSCREEN, BUT INITIALIZE THE COMMAND LINE
uiCloseCmdLine();

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
    ggl_cliprect(scr,0,ytop,SCREEN_WIDTH-1,ybottom-1,ggl_mkcolor(0xf));
    ggl_cliphline(scr,ybottom,0,SCREEN_WIDTH-1,0);
    ggl_clipvline(scr,21,ytop,ybottom,0);
    ggl_clipvline(scr,43,ytop,ybottom,0);
    ggl_clipvline(scr,65,ytop,ybottom,0);
    ggl_clipvline(scr,87,ytop,ybottom,0);
    ggl_clipvline(scr,109,ytop,ybottom,0);

    halScreen.DirtyFlag&=~MENU1_DIRTY;
}

// REDRAW THE OTHER MENU
void halRedrawMenu2(DRAWSURFACE *scr)
{
    if(halScreen.Menu2==0) {
        halScreen.DirtyFlag&=~MENU2_DIRTY;
        return;
    }

    int ytop,ybottom;
    int oldclipx,oldclipx2,oldclipy,oldclipy2;

    ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
    ybottom=ytop+halScreen.Menu2-1;
    // DRAW BACKGROUND
    ggl_cliprect(scr,0,ytop-1,64,ybottom,0);
    ggl_clipvline(scr,21,ytop-1,ybottom,ggl_mkcolor(0x8));
    ggl_clipvline(scr,43,ytop-1,ybottom,ggl_mkcolor(0x8));
    ggl_clipvline(scr,65,ytop-1,ybottom,ggl_mkcolor(0x8));
//    ggl_clipvline(scr,87,ytop,ybottom,0);
//    ggl_clipvline(scr,109,ytop,ybottom,0);
    ggl_cliphline(scr,ytop+6,0,64,ggl_mkcolor(0x8));

    // DRAW VARS OF THE CURRENT DIRECTORY IN THIS MENU

    oldclipx=scr->clipx;
    oldclipx2=scr->clipx2;
    oldclipy=scr->clipy;
    oldclipy2=scr->clipy2;

    // BASIC CHECK FOR CHANGE OF DIRECTORY
    if(halScreen.Menu2Dir!=CurrentDir) {
        halScreen.Menu2Dir=CurrentDir;
        halScreen.Menu2Page=0;
    }

    BINT nvars=rplGetVisibleVarCount();
    BINT k;
    WORDPTR *var;

    // BASIC CHECK OF VALIDITY - COMMANDS MAY HAVE RENDERED THE PAGE NUMBER INVALID
    // FOR EXAMPLE BY PURGING VARIABLES
    if((halScreen.Menu2Page>=nvars)||(halScreen.Menu2Page<0)) halScreen.Menu2Page=0;


    // FIRST ROW

    scr->clipy=ytop;
    scr->clipy2=ytop+5;

    for(k=0;k<3;++k) {
    scr->clipx=22*k;
    scr->clipx2=22*k+20;
    var=rplFindVisibleGlobalByIndex(halScreen.Menu2Page+k);
    if(var) {
            BINT w=StringWidthN((char *)(*var+1),(char *)(*var+1)+rplGetIdentLength(*var),halScreen.MenuFont);
            if(w>=scr->clipx2-scr->clipx+1) w=scr->clipx;
            else w=(scr->clipx2+scr->clipx-w)>>1;
            if(ISDIR(*var[1])) {
                //DrawTextN(w+1,scr->clipy,(char *)(*var+1),(char *)(*var+1)+rplGetIdentLength(*var),halScreen.MenuFont,0x0,scr);
                ggl_clipvline(scr,scr->clipx2,scr->clipy,scr->clipy2,ggl_mkcolor(0xf));
                ggl_cliphline(scr,scr->clipy2,scr->clipx,scr->clipx2,ggl_mkcolor(0xf));
            }

        DrawTextN(w,scr->clipy,(char *)(*var+1),(char *)(*var+1)+rplGetIdentLength(*var),halScreen.MenuFont,0xf,scr);
    }
    }

    // SECOND ROW

    scr->clipy=ytop+7;
    scr->clipy2=ybottom;

    for(k=0;k<2;++k) {
    scr->clipx=22*k;
    scr->clipx2=22*k+20;
    var=rplFindVisibleGlobalByIndex(halScreen.Menu2Page+3+k);
    if(var) {
            BINT w=StringWidthN((char *)(*var+1),(char *)(*var+1)+rplGetIdentLength(*var),halScreen.MenuFont);
            if(w>=scr->clipx2-scr->clipx+1) w=scr->clipx;
            else w=(scr->clipx2+scr->clipx-w)>>1;
            if(ISDIR(*var[1])) {
                //  DrawTextN(w+1,scr->clipy,(char *)(*var+1),(char *)(*var+1)+rplGetIdentLength(*var),halScreen.MenuFont,0x0,scr);
                ggl_clipvline(scr,scr->clipx2,scr->clipy,scr->clipy2,ggl_mkcolor(0xf));
                ggl_cliphline(scr,scr->clipy2,scr->clipx,scr->clipx2,ggl_mkcolor(0xf));

            }

            DrawTextN(w,scr->clipy,(char *)(*var+1),(char *)(*var+1)+rplGetIdentLength(*var),halScreen.MenuFont,0xf,scr);
    }
    }

    // NOW DO THE NXT KEY
    scr->clipx=22*k;
    scr->clipx2=22*k+20;

    if(nvars==6) {
        var=rplFindVisibleGlobalByIndex(halScreen.Menu2Page+3+k);
        if(var) {
                BINT w=StringWidthN((char *)(*var+1),(char *)(*var+1)+rplGetIdentLength(*var),halScreen.MenuFont);
                if(w>=scr->clipx2-scr->clipx+1) w=scr->clipx;
                else w=(scr->clipx2+scr->clipx-w)>>1;
                if(ISDIR(*var[1])) {
                    // DrawTextN(w+1,scr->clipy,(char *)(*var+1),(char *)(*var+1)+rplGetIdentLength(*var),halScreen.MenuFont,0x0,scr);
                    ggl_clipvline(scr,scr->clipx2,scr->clipy,scr->clipy2,ggl_mkcolor(0xf));
                    ggl_cliphline(scr,scr->clipy2,scr->clipx,scr->clipx2,ggl_mkcolor(0xf));

                }
                DrawTextN(w,scr->clipy,(char *)(*var+1),(char *)(*var+1)+rplGetIdentLength(*var),halScreen.MenuFont,0xf,scr);
        }
    } else {
     if(nvars>6) {
         DrawText(scr->clipx,scr->clipy,"NXT...",halScreen.MenuFont,0xf,scr);
     }
    }



    scr->clipx=oldclipx;
    scr->clipx2=oldclipx2;
    scr->clipy=oldclipy;
    scr->clipy2=oldclipy2;



    halScreen.DirtyFlag&=~MENU2_DIRTY;
}

void halRedrawStatus(DRAWSURFACE *scr)
{
    if(halScreen.Menu2) {
    int ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
    ggl_cliprect(scr,STATUSAREA_X,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);

    }
    // TODO: SHOW THE CURRENT DIR, ETC. HERE

    halScreen.DirtyFlag&=~STAREA_DIRTY;

}

void halRedrawCmdLine(DRAWSURFACE *scr)
{
    if(halScreen.CmdLine) {
    int ytop=halScreen.Form+halScreen.Stack;
    if((halScreen.DirtyFlag&CMDLINE_ALLDIRTY)==CMDLINE_ALLDIRTY) {
        ggl_cliprect(scr,0,ytop,SCREEN_WIDTH-1,ytop+halScreen.CmdLine-1,0);
        ggl_cliphline(scr,ytop,0,SCREEN_WIDTH-1,0xf0f0f0f0);
    }

    BINT y=(halScreen.LineCurrent-halScreen.LineVisible)*halScreen.CmdLineFont->BitmapHeight;
    BYTEPTR cmdline=(BYTEPTR) (CmdLineCurrentLine+1);
    BINT nchars=rplStrSize(CmdLineCurrentLine);

    // TODO: SHOW OTHER LINES HERE

    if(halScreen.DirtyFlag&CMDLINE_LINEDIRTY) {
    // UPDATE THE CURRENT LINE
        BINT linelen=StringWidthN((char *)cmdline,(char *)cmdline+nchars,(UNIFONT *)halScreen.CmdLineFont);
        DrawTextBkN(-halScreen.XVisible,ytop+2+y,(char *)cmdline,(char *)cmdline+nchars,(UNIFONT *)halScreen.CmdLineFont,0xf,0x0,scr);
        // CLEAR UP TO END OF LINE
        ggl_cliprect(scr,-halScreen.XVisible+linelen,ytop+2+y,SCREEN_W-1,ytop+2+y+halScreen.CmdLineFont->BitmapHeight-1,0);
    }

    if(halScreen.DirtyFlag&CMDLINE_CURSORDIRTY) {
    // DRAW THE CURSOR
    if(!(halScreen.CursorState&0x8000)) DrawTextBkN(halScreen.CursorX-halScreen.XVisible,ytop+2+y,(char *)&halScreen.CursorState,((char *)&halScreen.CursorState)+1,(UNIFONT *)halScreen.CmdLineFont,0x0,0xf,scr);

    else {
        scr->clipx=halScreen.CursorX-halScreen.XVisible;
        scr->clipx2=scr->clipx+8;   // HARD CODED MAXIMUM WIDTH OF THE CURSOR
        if(scr->clipx2>=SCREEN_WIDTH) scr->clipx2=SCREEN_WIDTH-1;

        ggl_cliprect(scr,halScreen.CursorX-halScreen.XVisible,ytop+2+y,halScreen.CursorX-halScreen.XVisible+StringWidthN((char *)&halScreen.CursorState,((char *)&halScreen.CursorState)+1,halScreen.CmdLineFont)-1,ytop+2+y+halScreen.CmdLineFont->BitmapHeight-1,0);

        // EITHER DON'T DRAW IT OR REDRAW THE PORTION OF COMMAND LINE UNDER THE CURSOR
        if(!(halScreen.DirtyFlag&CMDLINE_LINEDIRTY))
        {
            // UPDATE THE CURRENT LINE
                BINT linelen=StringWidthN((char *)cmdline,(char *)cmdline+nchars,(UNIFONT *)halScreen.CmdLineFont);
            // THE LINE WAS NOT UPDATED, MEANS WE ARE UPDATING ONLY THE CURSOR
            // UPDATE THE CURRENT LINE
            DrawTextBkN(-halScreen.XVisible,ytop+2+y,(char *)cmdline,(char *)cmdline+nchars,(UNIFONT *)halScreen.CmdLineFont,0xf,0x0,scr);
            // CLEAR UP TO END OF LINE
            ggl_cliprect(scr,-halScreen.XVisible+linelen,ytop+2+y,SCREEN_WIDTH-1,ytop+2+y+halScreen.CmdLineFont->BitmapHeight-1,0);

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
    if(!halScreen.SAreaTimer) {
    // ONLY REDRAW IF THERE'S NO POPUP MESSAGES
    if(halScreen.DirtyFlag&MENU2_DIRTY) halRedrawMenu2(scr);
    if(halScreen.DirtyFlag&STAREA_DIRTY) halRedrawStatus(scr);
    }
}

void status_popup_handler()
{
        DRAWSURFACE scr;
        ggl_initscr(&scr);
        halRedrawMenu2(&scr);
        halRedrawStatus(&scr);
    halScreen.SAreaTimer=0;
}


// WILL KEEP THE STATUS AREA AS-IS FOR 5 SECONDS, THEN REDRAW IT
// TO CLEAN UP POP-UP MESSAGES
void halStatusAreaPopup()
{
    if(halScreen.SAreaTimer) {
        tmr_eventkill(halScreen.SAreaTimer);
        //tmr_eventpause(halScreen.SAreaTimer);
        //tmr_eventresume(halScreen.SAreaTimer);      // PAUSE/RESUME WILL RESTART THE 5 SECOND COUNT
        //return;
    }
    halScreen.SAreaTimer=tmr_eventcreate(&status_popup_handler,3000,0);
}

// WILL KEEP THE STATUS AREA AS-IS FOR 5 SECONDS, THEN REDRAW IT
// TO CLEAN UP POP-UP MESSAGES
void halErrorPopup()
{
    if(halScreen.SAreaTimer) {
        tmr_eventkill(halScreen.SAreaTimer);
        //tmr_eventpause(halScreen.SAreaTimer);
        //tmr_eventresume(halScreen.SAreaTimer);      // PAUSE/RESUME WILL RESTART THE 3 SECOND COUNT
        //return;
    }
    halScreen.SAreaTimer=tmr_eventcreate(&status_popup_handler,3000,0);
}



// DECOMPILE THE OPCODE NAME IF POSSIBLE

WORDPTR halGetCommandName(WORD Opcode)
{

    if(ISPROLOG(Opcode)) return 0;  // ONLY DECOMPILE COMMANDS, NOT OBJECTS

    BINT SavedException=Exceptions;
    BINT SavedErrorCode=ErrorCode;
    WORD Opcodes[1];


    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
    Opcodes[0]=Opcode;  // STORE IT IN MEMORY INSTEAD OF REGISTER
    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
    WORDPTR opname=rplDecompile(Opcodes,0);
    Exceptions=SavedException;
    ErrorCode=SavedErrorCode;

    return opname;
}


// RETRIEVES A NULL-TERMINATED MESSAGE BASED ON MESSAGE CODE
BYTEPTR halGetMessage(WORD errorcode)
{
    MSGLIST *ptr=all_messages;
    while(ptr->code) {
        if(ptr->code==errorcode) return (BYTEPTR)ptr->text;
        ptr++;
    }
    // MESSAGE 0 IS THE UNKNOWN ERROR MESSAGE
    return all_messages[0].text;
}

// DISPLAY AN ERROR BOX FOR 5 SECONDS WITH AN ERROR MESSAGE
// USES ERROR CODE FROM SYSTEM Exceptions
void halShowErrorMsg()
{
        int errbit;
        if(!Exceptions) return;
        WORD error=Exceptions;
        DRAWSURFACE scr;
        ggl_initscr(&scr);


        if(!halScreen.Menu2) {
            // SHOW THE SECOND MENU TO DISPLAY THE MESSAGE
            halSetMenu2Height(MENU2_HEIGHT);
            halRedrawAll(&scr);
        }



        BINT ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
        // CLEAR MENU2 AND STATUS AREA
        ggl_cliprect(&scr,0,ytop,SCREEN_WIDTH-1,ytop+halScreen.StAreaFont->BitmapHeight-1,ggl_mkcolor(6));
        ggl_cliprect(&scr,0,ytop+halScreen.StAreaFont->BitmapHeight,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);
        // DO SOME DECORATIVE ELEMENTS
        //ggl_cliphline(&scr,ytop,0,SCREEN_WIDTH-1,ggl_mkcolor(8));
        ggl_cliphline(&scr,ytop+halScreen.Menu2-1,0,SCREEN_WIDTH-1,ggl_mkcolor(8));
        ggl_clipvline(&scr,0,ytop+halScreen.StAreaFont->BitmapHeight,ytop+halScreen.Menu2-2,ggl_mkcolor(6));
        ggl_clipvline(&scr,SCREEN_WIDTH-1,ytop+1,ytop+halScreen.Menu2-2,ggl_mkcolor(6));

        scr.clipx=1;
        scr.clipx2=SCREEN_WIDTH-2;
        scr.clipy=ytop;
        scr.clipy2=ytop+halScreen.Menu2-2;
        // SHOW ERROR MESSAGE

        if(Exceptions!=EX_ERRORCODE) {
            BINT xstart=scr.clipx;
            if(*ExceptionPointer!=0) {  // ONLY IF THERE'S A VALID COMMAND TO BLAME
            WORDPTR cmdname=halGetCommandName(*ExceptionPointer);
            if(cmdname) {
            BYTEPTR start=(BYTEPTR)(cmdname+1);
            BYTEPTR end=start+rplStrSize(cmdname);

            xstart+=StringWidthN((char *)start,(char *)end,halScreen.StAreaFont);
            DrawTextN(scr.clipx,scr.clipy,(char *)start,(char *)end,halScreen.StAreaFont,0xf,&scr);
            xstart+=4;
            }
            }
            DrawText(xstart,scr.clipy,"Exception:",halScreen.StAreaFont,0xf,&scr);

            BINT ecode;
            for(errbit=0;errbit<8;++errbit)     // THERE'S ONLY A FEW EXCEPTIONS IN THE NEW ERROR MODEL
            {
            if(Exceptions&(1<<errbit)) {
                ecode=MAKEMSG(0,errbit);
                BYTEPTR message=halGetMessage(ecode);
                DrawText(scr.clipx,scr.clipy+halScreen.StAreaFont->BitmapHeight,message,halScreen.StAreaFont,0xf,&scr);
                break;
            }
            }
        }
        else {
            // TRY TO DECOMPILE THE OPCODE THAT CAUSED THE ERROR
            BINT xstart=scr.clipx;
            if(*ExceptionPointer!=0) {  // ONLY IF THERE'S A VALID COMMAND TO BLAME
            WORDPTR cmdname=halGetCommandName(*ExceptionPointer);
            if(cmdname) {
            BYTEPTR start=(BYTEPTR)(cmdname+1);
            BYTEPTR end=start+rplStrSize(cmdname);

            xstart+=StringWidthN((char *)start,(char *)end,halScreen.StAreaFont);
            DrawTextN(scr.clipx,scr.clipy,(char *)start,(char *)end,halScreen.StAreaFont,0xf,&scr);
            xstart+=4;
            }
            }
            DrawText(xstart,scr.clipy,"Error:",halScreen.StAreaFont,0xf,&scr);
            // TODO: GET NEW TRANSLATABLE MESSAGES
            BYTEPTR message=halGetMessage(ErrorCode);
            DrawText(scr.clipx,scr.clipy+halScreen.StAreaFont->BitmapHeight,message,halScreen.StAreaFont,0xf,&scr);

        }




        halErrorPopup();

}

void halShowMsgN(char *Text,char *End)
{
        DRAWSURFACE scr;
        ggl_initscr(&scr);

        if(!halScreen.Menu2) {
            // SHOW THE SECOND MENU TO DISPLAY THE MESSAGE
            halSetMenu2Height(MENU2_HEIGHT);
            halRedrawAll(&scr);
        }


        BINT ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
        // CLEAR MENU2 AND STATUS AREA
        ggl_cliprect(&scr,0,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);
        // DO SOME DECORATIVE ELEMENTS
        ggl_cliphline(&scr,ytop+1,1,SCREEN_WIDTH-2,ggl_mkcolor(8));
        ggl_cliphline(&scr,ytop+halScreen.Menu2-1,1,SCREEN_WIDTH-2,ggl_mkcolor(8));
        ggl_clipvline(&scr,1,ytop+2,ytop+halScreen.Menu2-2,ggl_mkcolor(8));
        ggl_clipvline(&scr,SCREEN_WIDTH-2,ytop+2,ytop+halScreen.Menu2-2,ggl_mkcolor(8));

        // SHOW MESSAGE

        DrawTextN(3,ytop+3,Text,End,halScreen.StAreaFont,0xf,&scr);

        halErrorPopup();

}

void halShowMsg(char *Text)
{
    char *End=Text;
    while(*End) ++End;

    halShowMsgN(Text,End);
}
