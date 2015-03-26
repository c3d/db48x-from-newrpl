// THIS IS THE MAIN STABLE API FOR KEYBOARD ACCESS
#include <ui.h>
#include <newrpl.h>
#include <libraries.h>



// DEBUG ONLY
const BYTEPTR testprogram=(const BYTEPTR) "<< 8 0 0 0 { } -> R S X Y A "
                "  << "
               " 1 R START 0 NEXT R ->LIST 'A' STO "
               " DO "
               "  'A' 'X' INCR R PUT "
               "  DO "
               "    'S' INCR DROP "
               "   X 'Y' STO "
               "   WHILE Y 1 > REPEAT "
               "     A X GET A 'Y' DECR  GET - "
               "     IF DUP 0 == SWAP ABS X Y - == OR THEN "
               "       0 'Y' STO "
               "      'A' X A X GET 1 - PUT "
               "       WHILE A X GET 0 == REPEAT "
               "         'A' 'X' DECR A X GET 1 - PUT "
               "       END "
               "     END "
               "   END "
               "  UNTIL Y 1 == END "
               " UNTIL X R == END "
               " "
               " S A "
            " >> "
          " >> "
          " 'PRO' STO "
          "     1 10 START PRO NEXT "
           ;

const BYTEPTR nq_stk=(const BYTEPTR) "<< 8 0 0 0 -> R S X Y "
  "<< 1 R "
    " START 0 "
    " NEXT DO R 'X' INCR UNPICK "
        " DO 'S' INCR DROP X 'Y' STO "
        " WHILE Y 1 > REPEAT X PICK 'Y' DECR 1 + PICK - "
          " IF DUP 0 == SWAP ABS X Y - == OR "
          " THEN 0 'Y' STO X PICK 1 - X UNPICK "
            " WHILE X PICK 0 == "
            " REPEAT 'X' DECR PICK 1 - X UNPICK "
            " END "
          " END "
        " END "
      " UNTIL Y 1 == "
      " END "
    " UNTIL X R == "
    " END 8 ->LIST S "
  " >> "
" >> "
        " 'NQ.STK' STO 1 10 START NQ.STK NEXT "
;

const BYTEPTR nq_idx=(const BYTEPTR) "<< 8 0 0 0 -> R S X Y "
        "<< 1 R START 0 NEXT "
    " DO R 'X' INCR UNPICK "
      " DO 'S' INCR DROP X 'Y' STO "
        " WHILE Y 1 > "
        " REPEAT X PICK 'Y' DECR 1 + PICK - "
          " IF DUP 0 == SWAP ABS X Y - == OR "
          " THEN 0 'Y' STO X PICK 1 - X UNPICK "
            " WHILE X PICK 0 == "
            " REPEAT 'X' DECR PICK 1 - X UNPICK "
            " END "
          " END "
        " END "
        " UNTIL Y 1 == "
      " END "
    " UNTIL X R == "
    " END 8 ->LIST S "
        " >> "
" >> "
        " 'NQ.IDX' STO 1 10 START NQ.IDX NEXT "
;

const BYTEPTR nq_istk=(const BYTEPTR) "<< 8 0 0 0 0 -> R S X Y Ax "
  " << 1 R START 0 NEXT "
    " DO 'X' INCR DROP R 'Ax' STO "
      " DO 'S' INCR DROP X 'Y' STO "
        " WHILE Y 1 > "
        " REPEAT Ax Y PICK - 'Y' DECR DROP "
          " IF DUP 0 == SWAP ABS X Y - == OR "
          " THEN 0 'Y' STO 'Ax' DECR DROP "
            " WHILE Ax 0 == "
            " REPEAT 'X' DECR PICK 1 - 'Ax' STO "
            " END "
          " END "
        " END "
      " UNTIL Y 1 == "
      " END Ax X UNPICK "
    " UNTIL X R == "
    " END 8 ->LIST S "
  " >> "
" >> "
        " 'NQ.ISTK' STO 1 10 START NQ.ISTK NEXT "
;

const BYTEPTR nq_new=(const BYTEPTR) "<< 1 -> X RES << "
" IF X 1 > THEN "
" X PICK 1 X 1 - FOR I "
        " DUP I 2 + PICK - ABS X I - ABS "
   " IF == THEN "
     " 0 'RES' STO X 'I' STO "
   " END "
   " NEXT "
   " DROP RES "
 " ELSE "
   " 1 "
 " END "
" >> "
" >> "
        " 'CHECKQUEEN' STO "
        " <<  9 OVER - -> X LIMIT "
        " << "

         " 1 8 START LIMIT 8 + ROLLD NEXT "
         " LIMIT DUPN 1 8 START LIMIT LIMIT 8 + + ROLL NEXT "

          " DO 9 ROLL X UNPICK "
                " IF X CHECKQUEEN "
                " THEN X 1 + DUP  "
                 " IF 8 <= THEN IF DOLEVEL THEN 0 9 ROLLD 0 'LIMIT' STO ELSE X PICK 17 X - ROLLD END "
                  " ELSE 9 ROLLD 0 'LIMIT' STO END "

                " ELSE X PICK 17 X - ROLLD "
                " END 'LIMIT' DECR "
              " UNTIL 0 <= "
              " END "
              " 1 9 X - START 9 ROLL DROP NEXT "
              " IF LIMIT 0 == "
              " THEN 0 ELSE 1 END "
          " >> "
        " >> "

        " 'DOLEVEL' STO "

        " << "
          " 1 2 3 4 5 6 7 8 "
          " 0 0 0 0 0 0 0 0 "
          " 1 DOLEVEL DROP "
          " 1 8 START 9 ROLL DROP NEXT "
        " 8 ->LIST "
        " >> "
        " 'NEW.RUN' STO 1 10 START NEW.RUN NEXT "
;

const BYTEPTR nq_werner=(const BYTEPTR) "<< "
  " 0 "
  " DO "
        " 8 SWAP 1 + "
    " WHILE "
        "  DUP2 "
      " DO 1 - "
      " UNTIL "
        " DUP2 5 + PICK - ABS "
        " DUP2 - * NOT "
      " END "
    " REPEAT "
        "  DROP "
        "  WHILE SWAP DUP 1 SAME "
      " REPEAT - "
      " END "
      " 1 - SWAP "
    " END "
    " DROP "
  " UNTIL DUP 8 SAME "
  " END "
  " ->LIST "
" >> "
        " 'NQ.W' STO 1 10 START NQ.W NEXT "
;

const BYTEPTR sincostest=(const BYTEPTR) "108 SETPREC -1 ACOS DUP 6 / SIN ";

const BYTEPTR realtest1008=(const BYTEPTR)  "1008 SETPREC << 0. 1000. 0.1 FOR n "
                                        "0.02 n DUP 0.1 + * / + "
                                        "-0.1 STEP "
                                        ">> EVAL " ;



const BYTEPTR realtest108=(const BYTEPTR)  "108 SETPREC << 0. 1000. 0.1 FOR n "
                                        "0.02 n DUP 0.1 + * / + "
                                        "-0.1 STEP "
                                        ">> EVAL " ;
const BYTEPTR realtest36=(const BYTEPTR)  "36 SETPREC << 0. 1000. 0.1 FOR n "
                                        "0.02 n DUP 0.1 + * / + "
                                        "-0.1 STEP "
                                        ">> EVAL " ;



void PrintObj(int x,int y,WORDPTR obj,DRAWSURFACE *scr)
{
    WORDPTR string;
    BINT nchars;
    BYTEPTR charptr;
    string=rplDecompile(obj);

    if(string) {
    // NOW PRINT THE STRING OBJECT
        nchars=rplStrLen(string);
        charptr=(BYTEPTR) (string+1);
        DrawTextN(x,y,charptr,nchars,(FONTDATA *)&System7Font,15,scr);
    }


}




// WAITS FOR A KEY TO BE PRESSED IN SLOW MODE

BINT halWaitForKey()
{
    int keymsg;

    if(!(halFlags&HAL_FASTMODE)) {
    tmr_eventkill(halBusyEvent);
    }

    do {

    keymsg=keyb_getmsg();

    if(!keymsg) {
    // FIRST: ENTER LOW SPEED MODE
    if(halFlags&HAL_FASTMODE) {
    cpu_setspeed(6000000);
    halFlags&=~HAL_FASTMODE;
    }
    if(halFlags&HAL_HOURGLASS) {
    halSetNotification(N_HOURGLASS,0);
    halFlags&=~HAL_HOURGLASS;
    }

    // LAST: GO INTO "WAIT FOR INTERRUPT"
    cpu_waitforinterrupt();
    }
    } while(!keymsg);



    return keymsg;

}



// FOR TESTING ONLY
volatile int waitforspeed;
const char * const keyNames[64]={
    "[NONE]",
    "<-",
    "TAN",
    "DIV",
    "MUL",
    "SUB",
    "ADD",
    "ENTER",
    "[NONE]",
    "SYMB",
    "COS",  // 10
    "1/X",
    "9",
    "6",
    "3",
    "SPC",
    "[NONE]",
    "''",
    "SIN",
    "X",
    "8",    // 20
    "5",
    "2",
    ".",
    "[NONE]",
    "EVAL",
    "SQRT",
    "+/-",
    "7",
    "4",
    "1",    // 30
    "0",
    "[NONE]",
    "HIST",
    "Y^X",
    "EEX",
    "[NONE]",
    "[NONE]",
    "[NONE]",
    "[NONE]",
    "[NONE]",   // 40
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "APP",
    "[NONE]",
    "UP",
    "LEFT",     // 50
    "DOWN",
    "RIGHT",
    "MODE",
    "TOOL",
    "VAR",
    "[NONE]",
    "STO",
    "NXT",
    "[NONE]",
    "ALPHA",    // 60
    "LSHIFT",
    "RSHIFT",
    "ON"
};



void waitforspeed_handler()
{
    waitforspeed=0;
}

// SYSTEM CONTEXT VARIABLE
// STORES THE CONTEXT ID
// ID=0 MEANS ANY CONTEXT
// ID BIT 0 --> SET TO 1 WHEN THE COMMAND LINE IS ACTIVE OR TEXT IS BEING EDITED
// ID BITS 1 AND BIT 2 ARE RESERVED FOR FUTURE USE AND SHOULD BE ZERO
// ID=8 IS THE STACK
// ID=16 IS PICT
// ID N*8 WITH N<100 ARE RESERVED FOR THE SYSTEM APPLICATIONS (SOLVER, ETC)
// ID= N*8 --> USER CONTEXTS FROM N=100 AND UP TO 16250 ARE FREE TO USE

// SET THE KEYBOARD CONTEXT
void halSetContext(BINT KeyContext)
{
halScreen.KeyContext=KeyContext;
}

// AND RETRIEVE
BINT halGetContext()
{
    return halScreen.KeyContext;
}



// DEBUG: DO-NOTHING KEYBOARD HANDLER
void dummyKeyhandler(BINT keymsg)
{
    return;
}

// END THE CURRENTLY OPEN COMMAND LINE, RETURN 1 IF COMPILED SUCCESSFULLY
// 0 IF ERROR.
// WHEN 1, THE STACK CONTAINS THE OBJECT/S COMPILED
// WHEN 0, THE COMMAND LINE IS STILL OPEN, WITH THE ERROR HIGHLIGHTED
BINT endCmdLineAndCompile()
{
    WORDPTR text=uiGetCmdLineText();
    if(!text) {
        throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
        return 0;
    }
    BINT len=rplStrLen(text);
    WORDPTR newobject;
    if(len) {
        newobject=rplCompile((BYTEPTR)(text+1),len,1);
        if(Exceptions || (!newobject)) {
            // TODO: SHOW ERROR MESSAGE AND SELECT THE WORD THAT CAUSED THE ERROR
            halShowErrorMsg();
            Exceptions=0;

            return 0;
        }
        else {
            // RUN THE OBJECT
            rplSetEntryPoint(newobject);
            rplRun();
            if(Exceptions) {
                // TODO: SHOW ERROR MESSAGE
                halShowErrorMsg();
                Exceptions=0;
                uiCloseCmdLine();
                halSetCmdLineHeight(0);
                halSetContext(halGetContext()& (~CONTEXT_INEDITOR));
                return 1;
            }
            // EVERYTHING WENT FINE, CLOSE THE COMMAND LINE
            uiCloseCmdLine();
            halSetCmdLineHeight(0);
            halSetContext(halGetContext()& (~CONTEXT_INEDITOR));

        }
    }

}

// **************************************************************************
// *******************    DEFAULT KEY HANDLERS     **************************
// **************************************************************************



void numberKeyHandler(BINT keymsg)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight+2);
        halSetContext(halGetContext()|CONTEXT_INEDITOR);
        uiOpenCmdLine();
        }
    BINT number;
    switch(KM_KEY(keymsg))
    {
    case KB_1:
        number='1';
        break;
    case KB_2:
        number='2';
        break;
    case KB_3:
        number='3';
        break;
    case KB_4:
        number='4';
        break;
    case KB_5:
        number='5';
        break;
    case KB_6:
        number='6';
        break;
    case KB_7:
        number='7';
        break;
    case KB_8:
        number='8';
        break;
    case KB_9:
        number='9';
        break;
    case KB_0:
        number='0';
        break;
    }
        uiInsertCharacters((BYTEPTR) &number,1);

}



void cmdKeyHandler(WORD Opcode,BYTEPTR Progmode,BINT IsFunc)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()==CONTEXT_STACK) {
            // ACTION WHEN IN THE STACK
                rplCallOvrOperator(Opcode);
                if(Exceptions) {
                    // TODO: SHOW ERROR MESSAGE
                    halShowErrorMsg();
                    Exceptions=0;
                }
            halScreen.DirtyFlag|=STACK_DIRTY;


        }

    }
    else{
        // ACTION INSIDE THE EDITOR
        switch(halScreen.CursorState&0xff)
        {

        case 'D':   // DIRECT EXECUTION
        {

                if(endCmdLineAndCompile()) {
                rplCallOvrOperator(Opcode);
                if(Exceptions) {
                    // TODO: SHOW ERROR MESSAGE
                    halShowErrorMsg();
                    Exceptions=0;
                }
            halScreen.DirtyFlag|=STACK_DIRTY;
                }
            break;
        }
        case 'P':   // PROGRAMMING MODE
            // TODO: SEPARATE TOKENS
            uiSeparateToken();
            uiInsertCharacters(Progmode,1);
            break;

        case 'A':   // ALPHANUMERIC MODE
            uiInsertCharacters(Progmode,1);
            if(IsFunc) {
                uiInsertCharacters("()",2);
                uiCursorLeft();
            }
            break;
        default:
         break;
        }
    }
}


void dotKeyHandler(BINT keymsg)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight+2);
        halSetContext(halGetContext()|CONTEXT_INEDITOR);
        uiOpenCmdLine();
        }
        uiInsertCharacters(".",1);
}

void enterKeyHandler(BINT keymsg)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        // PERFORM DUP
        if(halGetContext()==CONTEXT_STACK) {
            // PERFORM DUP ONLY IF THERE'S DATA ON THE STACK
            // DON'T ERROR IF STACK IS EMPTY
            if(rplDepthData()>0) rplPushData(rplPeekData(1));
            halScreen.DirtyFlag|=STACK_DIRTY;
        }

        }
    else{
     endCmdLineAndCompile();
   }
}

void backspKeyHandler(BINT keymsg)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        // DO DROP
        if(halGetContext()==CONTEXT_STACK) {
            // PERFORM DROP ONLY IF THERE'S DATA ON THE STACK
            // DON'T ERROR IF STACK IS EMPTY
            if(rplDepthData()>0) rplDropData(1);
            halScreen.DirtyFlag|=STACK_DIRTY;
        }

    }
    else{
        // REMOVE CHARACTERS FROM THE COMMAND LINE
        // TODO: IMPLEMENT THIS!
        uiCursorLeft(1);
        uiRemoveCharacters(1);
    }
}

void leftKeyHandler(BINT keymsg)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()==CONTEXT_STACK) {
            // TODO: WHAT TO DO WITH LEFT CURSOR??

        }

    }
    else{
        uiCursorLeft(1);
    }
}

void rightKeyHandler(BINT keymsg)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()==CONTEXT_STACK) {

            if(rplDepthData()>1) {
                WORDPTR ptr=rplPeekData(2);
                rplOverwriteData(2,rplPeekData(1));
                rplOverwriteData(1,ptr);
            halScreen.DirtyFlag|=STACK_DIRTY;
            }

        }

    }
    else{
        uiCursorRight(1);
    }
}




void addKeyHandler(BINT keymsg)
{
    cmdKeyHandler(MKOPCODE(LIB_OVERLOADABLE,OVR_ADD),"+",0);
}
void subKeyHandler(BINT keymsg)
{
    cmdKeyHandler(MKOPCODE(LIB_OVERLOADABLE,OVR_SUB),"-",0);
}

void divKeyHandler(BINT keymsg)
{
    cmdKeyHandler(MKOPCODE(LIB_OVERLOADABLE,OVR_DIV),"/",0);
}
void mulKeyHandler(BINT keymsg)
{
    cmdKeyHandler(MKOPCODE(LIB_OVERLOADABLE,OVR_MUL),"*",0);
}
void invKeyHandler(BINT keymsg)
{
    cmdKeyHandler(MKOPCODE(LIB_OVERLOADABLE,OVR_INV),"INV",1);
}


void spcKeyHandler(BINT keymsg)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight+2);
        halSetContext(halGetContext()|CONTEXT_INEDITOR);
        uiOpenCmdLine();
        }
        uiInsertCharacters(" ",1);

}

void chsKeyHandler(BINT keymsg)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()==CONTEXT_STACK) {
            // ACTION WHEN IN THE STACK
                rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NEG));
                if(Exceptions) {
                    // TODO: SHOW ERROR MESSAGE
                    halShowErrorMsg();
                    Exceptions=0;
                }
            halScreen.DirtyFlag|=STACK_DIRTY;
        }

    }
    else{
        // ACTION INSIDE THE EDITOR

        // FIRST CASE: IF TOKEN UNDER THE CURSOR IS OR CONTAINS A VALID NUMBER, CHANGE THE SIGN OF THE NUMBER IN THE TEXT
        BYTEPTR startnum;
        BYTEPTR line=(BYTEPTR)(CmdLineCurrentLine+1);

        startnum=uiFindNumberStart();
        if(!startnum) {
            // SECOND CASE: IF TOKEN UNDER CURSOR IS EMPTY, IN 'D' MODE COMPILE OBJECT AND THEN EXECUTE NEG
            startnum=line+halScreen.CursorPosition;
            if(startnum>line) {
            if(startnum[-1]=='+') { startnum[-1]='-'; halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY; return; }
            if(startnum[-1]=='-') { startnum[-1]='+'; halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY; return; }
            if((startnum[-1]=='E')||(startnum[-1]=='e') ) { uiInsertCharacters("-",1); return; }


            }


            if((halScreen.CursorState&0xff)=='D') {
            // COMPILE AND EXECUTE NEG
            if(endCmdLineAndCompile()) {
            rplCallOvrOperator(MKOPCODE(LIB_OVERLOADABLE,OVR_NEG));
            if(Exceptions) {
                // TODO: SHOW ERROR MESSAGE
                halShowErrorMsg();
                Exceptions=0;
            }
        halScreen.DirtyFlag|=STACK_DIRTY;
            }

            return;
            }

        }
        else {
            // WE FOUND A NUMBER
            if(startnum>line) {
            if(startnum[-1]=='+') { startnum[-1]='-'; halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY; return; }
            if(startnum[-1]=='-') { startnum[-1]='+'; halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY; return; }
            }
            // NEED TO INSERT A CHARACTER HERE
            BINT oldposition=halScreen.CursorPosition;
            uiMoveCursor(startnum-line);
            uiInsertCharacters("-",1);
            uiMoveCursor(oldposition+1);
            uiEnsureCursorVisible();
            return;
      }

        // THIRD CASE: IF TOKEN UNDER CURSOR IS SOMETHING OTHER THAN A NUMBER, JUST INSERT A MINUS SIGN

    }
}

void eexKeyHandler(BINT keymsg)
{
    if(!(halGetContext()&CONTEXT_INEDITOR)) {
        if(halGetContext()==CONTEXT_STACK) {
            halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight+2);
            halSetContext(halGetContext()|CONTEXT_INEDITOR);
            uiOpenCmdLine();
            uiInsertCharacters("1E",2);
            return;
        }

    }
    else{
        // ACTION INSIDE THE EDITOR

        // FIRST CASE: IF TOKEN UNDER THE CURSOR IS OR CONTAINS A VALID NUMBER
        BYTEPTR startnum;
        BYTEPTR line=(BYTEPTR)(CmdLineCurrentLine+1);

        startnum=uiFindNumberStart();
        if(!startnum) {
            startnum=line+halScreen.CursorPosition;
            // DO NOTHING IF THERE'S ALREADY AN 'E' BEFORE THE CURSOR
            if((startnum>line) && ((startnum[-1]=='E')||(startnum[-1]=='e') )) return;

            // SECOND CASE: IF TOKEN UNDER CURSOR IS EMPTY, IN 'D' MODE COMPILE OBJECT AND THEN APPEND 1E
            uiInsertCharacters("1E",2);
            return;
        }
        else {
            // WE FOUND A NUMBER
            if((startnum>line)&&((startnum[-1]=='-')||(startnum[-1]=='+'))) --startnum;

            if((startnum>line) && ((startnum[-1]=='E')||(startnum[-1]=='e') )) {
                uiMoveCursor(startnum-line);
                // TODO: SELECT THE EXISTING NUMBER FOR DELETION ON NEXT KEYPRESS
                uiEnsureCursorVisible();
                return;
            }

            // NEED TO INSERT A CHARACTER HERE
            BINT oldposition=halScreen.CursorPosition;
            uiInsertCharacters("E",1);
            uiMoveCursor(oldposition+1);
            uiEnsureCursorVisible();
            return;
      }

    }
}





void onPlusKeyHandler(BINT keymsg)
{
// INCREASE CONTRAST
DRAWSURFACE scr;
ggl_initscr(&scr);
int ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
// CLEAR STATUS AREA
ggl_rect(&scr,STATUSAREA_X,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);

int j;
for(j=0;j<15;++j) {
    ggl_rect(&scr,STATUSAREA_X+1+3*j,ytop+7,STATUSAREA_X+1+3*j+2,ytop+12,ggl_mkcolor(j));
    ggl_rect(&scr,STATUSAREA_X+1+3*j,ytop,STATUSAREA_X+1+3*j+2,ytop+5,ggl_mkcolor(15-j));
}

halStatusAreaPopup();

__lcd_contrast++;
if(__lcd_contrast>0xf) __lcd_contrast=0xf;

lcd_setcontrast(__lcd_contrast);

}

void onMinusKeyHandler(BINT keymsg)
{
// DECREASE CONTRAST
DRAWSURFACE scr;
ggl_initscr(&scr);
int ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
// CLEAR STATUS AREA
ggl_rect(&scr,STATUSAREA_X,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);

int j;
for(j=0;j<15;++j) {
    ggl_rect(&scr,STATUSAREA_X+1+3*j,ytop+7,STATUSAREA_X+1+3*j+2,ytop+12,ggl_mkcolor(j));
    ggl_rect(&scr,STATUSAREA_X+1+3*j,ytop,STATUSAREA_X+1+3*j+2,ytop+5,ggl_mkcolor(15-j));
}

halStatusAreaPopup();

__lcd_contrast--;
if(__lcd_contrast<0) __lcd_contrast=0;
lcd_setcontrast(__lcd_contrast);
}


// **************************************************************************
// ******************* END OF DEFAULT KEY HANDLERS **************************
// **************************************************************************


typedef void (*handlerfunc_t)(BINT keymsg);

// STRUCTURE FOR DEFAULT KEYBOARD HANDLERS
struct keyhandler_t {
    BINT message;
    BINT context;
    handlerfunc_t action;
} ;


// LIST OF HANDLERS, END WITH action=NULL
struct keyhandler_t __keydefaulthandlers[]= {
    { KM_PRESS|KB_1, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_2, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_3, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_4, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_5, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_6, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_7, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_8, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_9, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_0, CONTEXT_ANY,&numberKeyHandler },
    { KM_PRESS|KB_DOT, CONTEXT_ANY,&dotKeyHandler },
    { KM_PRESS|KB_ENT, CONTEXT_ANY,&enterKeyHandler },
    { KM_PRESS|KB_BKS, CONTEXT_ANY,&backspKeyHandler },
    { KM_PRESS|KB_LF, CONTEXT_ANY,&leftKeyHandler },
    { KM_PRESS|KB_RT, CONTEXT_ANY,&rightKeyHandler },
    { KM_PRESS|KB_ADD, CONTEXT_ANY,&addKeyHandler },
    { KM_PRESS|KB_SUB, CONTEXT_ANY,&subKeyHandler },
    { KM_PRESS|KB_DIV, CONTEXT_ANY,&divKeyHandler },
    { KM_PRESS|KB_MUL, CONTEXT_ANY,&mulKeyHandler },
    { KM_PRESS|KB_Y, CONTEXT_ANY,&invKeyHandler },
    { KM_PRESS|KB_SPC, CONTEXT_ANY,&spcKeyHandler },
    { KM_PRESS|KB_W, CONTEXT_ANY,&chsKeyHandler },
    { KM_PRESS|KB_V, CONTEXT_ANY,&eexKeyHandler },

    { KM_PRESS|KB_ADD|SHIFT_ONHOLD, CONTEXT_ANY,&onPlusKeyHandler },
    { KM_PRESS|KB_SUB|SHIFT_ONHOLD, CONTEXT_ANY,&onMinusKeyHandler },

    { 0 , 0 , 0 }
};

// DO CUSTOM KEYBOARD ACTIONS. RETURN 0 IF NO ACTION WAS DEFINED, NONZERO IF SOMETHING WAS EXECUTED
// KEY MESSAGES ARE PROCESSED THROUGH A LIST OF USER DEFINED KEYCODES
// { [KEYMESSAGE] [KEYCONTEXT] [ACTION] ... [KEYMESSAGE2] [KEYCONTEXT2] [ACTION2] ...}
// KEYS ARE IN NO PARTICULAR ORDER
// KEY TABLE IS SCANNED FROM START TO FINISH, NEW KEYS SHOULD BE ADDED TO THE HEAD
// OF THE LIST IN ORDER TO OVERRIDE PREVIOUS DEFINITIONS
// [KEYMESSAGE] AND [KEYCONTEXT] ARE BOTH SINT OBJECTS.
// [ACTION] IS AN ARBITRARY OBJECT THAT WILL BE XEQ'TED.
// HANDLER SCANS THE LIST, LOOKS FOR A MATCH IN KEYMESSAGE AND KEYCONTEXT.
// IF [KEYCONTEXT] IN THE TABLE IS 0, THEN ANY CONTEXT IS CONSIDERED A MATCH.
// ONCE A MATCH IS FOUND, THE [ACTION] OBJECT IS XEQ'TED.
// ONLY THE FIRST MATCH IS EXECUTED, THE SEARCH STOPS THERE.
// IF THE TABLE HAS NO MATCH, THE DEFAULT ACTION HANDLER IS CALLED.
// CUSTOM KEY LIST IS STORED IN Settings

int halDoCustomKey(BINT keymsg)
{
    // TODO: READ THE KEYBOARD TABLE FROM THE Settings DIRECTORY AND DO IT

    return 0;

}

int halDoDefaultKey(BINT keymsg)
{
struct keyhandler_t *ptr=__keydefaulthandlers;

while(ptr->action) {
    if(ptr->message==keymsg) {
        // CHECK IF CONTEXT MATCHES
        if((!ptr->context) || (ptr->context==halScreen.KeyContext)) {
            //  IT'S A MATCH, EXECUTE THE ACTION;
            (ptr->action)(keymsg);
            return 1;
        }
        }
    ++ptr;
}
return 0;
}

// PROCESSES KEY MESSAGES AND CALL APPROPRIATE HANDLERS BY KEYCODE

// RETURNS 0 IF THE LOOP HAS TO CONTINUE, 1 TO TERMINATE OUTER LOOP




int halProcessKey(BINT keymsg)
{
    int wasProcessed;

    waitforspeed=0;

    if(KM_MESSAGE(keymsg)==KM_SHIFT) {
        // THERE WAS A CHANGE IN SHIFT PLANE, UPDATE ANNUNCIATORS
        halSetNotification(N_LEFTSHIFT,((KM_SHIFTPLANE(keymsg)&SHIFT_LS)? 1:0));
        halSetNotification(N_RIGHTSHIFT,((KM_SHIFTPLANE(keymsg)&SHIFT_RS)? 1:0));
        halSetNotification(N_ALPHA,((KM_SHIFTPLANE(keymsg)&SHIFT_ALPHA)? 1:0));
    }

    wasProcessed=halDoCustomKey(keymsg);

    if(!wasProcessed) wasProcessed=halDoDefaultKey(keymsg);

    // *************** DEBUG ONLY ************
    /*
    if(!wasProcessed) {

    // ALL OTHER KEYS, JUST DISPLAY THE KEY NAME ON SCREEN
        DRAWSURFACE scr;
        ggl_initscr(&scr);

    // FOR DEBUG ONLY
    int width=StringWidth((char *)keyNames[KM_KEY(keymsg)],(FONTDATA *)&System7Font);
    int ytop=halScreen.Form+halScreen.Stack+halScreen.CmdLine+halScreen.Menu1;
    // CLEAR STATUS AREA AND SHOW KEY THERE
    //ggl_rect(&scr,STATUSAREA_X,ytop,SCREEN_WIDTH-1,ytop+halScreen.Menu2-1,0);
    DrawText(SCREEN_WIDTH-width,ytop+halScreen.Menu2/2,(char *)keyNames[KM_KEY(keymsg)],(FONTDATA *)System7Font,15,&scr);
    char *shiftstr;
    switch(KM_SHIFTPLANE(keymsg))
    {
    case SHIFT_LS:
        shiftstr="(LS)";
        break;
    case SHIFT_LS|SHIFT_LSHOLD:
        shiftstr="(LSH)";
        break;
    case SHIFT_RS:
        shiftstr="(RS)";
        break;
    case SHIFT_RS|SHIFT_RSHOLD:
        shiftstr="(RSH)";
        break;
    case SHIFT_ALPHA:
        shiftstr="(AL)";
        break;
    case SHIFT_ALPHA|SHIFT_ALPHAHOLD:
        shiftstr="(ALH)";
        break;
    case SHIFT_ONHOLD:
        shiftstr="(ONH)";
        break;
    default:
        shiftstr="";
    }
    DrawText(SCREEN_WIDTH-width-32,ytop+halScreen.Menu2/2,shiftstr,(FONTDATA *)&System7Font,15,&scr);

    if(KM_MESSAGE(keymsg)==KM_LPRESS) DrawText(SCREEN_WIDTH-width-42,ytop+halScreen.Menu2/2,"L=",(FONTDATA *)&System7Font,15,&scr);

    }
    */

    // ONLY RETURN 1 WHEN THE OUTER LOOP IS SUPPOSED TO END
    return 0;
}

// THIS FUNCTION RETURNS WHEN THE FORM CLOSES, OR THE USER EXITS WITH THE ON KEY

void halOuterLoop()
{
    int keymsg;
    DRAWSURFACE scr;
    ggl_initscr(&scr);
    do {
        halRedrawAll(&scr);
        keymsg=halWaitForKey();
    } while(!halProcessKey(keymsg));

}
