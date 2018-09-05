/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <newrpl.h>
#include <ui.h>
#include <libraries.h>



// COMMAND LINE API
// BASIC PRINCIPLES OF THE COMMAND LINE:
// * ONLY ONE LINE IS EDITED AT ONCE
// * LINE IS EDITED AS THE LAST OBJECT IN TEMPOB, TO BE STRETCHED AT WILL
// * IF LINE IS NOT AT THE END OF TEMPOB, A NEW COPY IS MADE


// RETURN LINE NUMBER FOR A GIVEN OFFSET, AND THE OFFSET OF THE START OF LINE
BINT uiGetLinebyOffset(BINT offset,BINT *linestart)
{

    BYTEPTR ptr=(BYTEPTR)(CmdLineText+1);
    BINT len=rplStrSize(CmdLineText);
    BINT f,found,count;

    if(offset>len) offset=len;
    if(offset<0) offset=0;
    found=0;
    for(f=0,count=0; (f<len) && (f<offset);++f,++ptr)
    {
    if(*ptr=='\n') {
        found=f+1;
        ++count;
    }
    }

    if(linestart) *linestart=found;
    return count+1;

}


BYTEPTR uiGetStartOfLine(BYTEPTR ptr,BYTEPTR startofstring)
{
    while(ptr>startofstring) {
        --ptr;
        if(*ptr=='\n') return ptr+1;
    }
    return ptr;
}

BYTEPTR uiGetEndOfLine(BYTEPTR ptr,BYTEPTR endofstring)
{
    while(ptr<endofstring) {
        if(*ptr=='\n') return ptr;
        ++ptr;
    }
    return ptr;
}



void uiSetCmdLineState(BINT state)
{
    halScreen.CmdLineState=state;
}
BINT uiGetCmdLineState()
{
    return halScreen.CmdLineState;
}

// SET THE COMMAND LINE TO A GIVEN STRING OBJECT
// RETURN THE TOTAL NUMBER OF LINES OF TEXT
BINT uiSetCmdLineText(WORDPTR text)
{
    CmdLineText=text;
    CmdLineCurrentLine=(WORDPTR)empty_string;
    CmdLineUndoList=(WORDPTR)empty_list;
    halScreen.LineIsModified=-1;

    BYTEPTR newstart=((BYTEPTR)CmdLineText)+4;
    BYTEPTR newend=newstart+rplStrSize(CmdLineText);
    BINT nl=1;
    while(newstart!=newend) {
        if(*newstart=='\n') ++nl;
        ++newstart;
    }

    // SET CURSOR AT END OF TEXT
    BINT end=rplStrSize(CmdLineText);
    BINT linestoff;
    BYTEPTR linestart;
    halScreen.LineCurrent=uiGetLinebyOffset(end,&linestoff);
    linestart=((BYTEPTR)(CmdLineText+1))+linestoff;
    halScreen.CursorPosition=((BYTEPTR)(CmdLineText+1))+end-linestart;

    if(halScreen.CursorPosition<0) halScreen.CursorPosition=0;
    halScreen.CursorX=StringWidthN((char *)linestart,(char *)linestart+halScreen.CursorPosition,*halScreen.FontArray[FONT_CMDLINE]);



    uiExtractLine(halScreen.LineCurrent);

    if(Exceptions) {
        throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        uiOpenCmdLine(0);
        //CmdLineText=(WORDPTR)empty_string;
        //CmdLineCurrentLine=(WORDPTR)empty_string;
        //CmdLineUndoList=(WORDPTR)empty_list;
        return 0;
    }


    uiEnsureCursorVisible();

    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;


    return nl;

}

WORDPTR uiGetCmdLineText()
{
    if(halScreen.LineIsModified>0) uiModifyLine(0);
    if(Exceptions) return NULL;
    return CmdLineText;
}

// SCROLL UP/DOWN AND LEFT/RIGHT TO KEEP CURSOR ON SCREEN
void uiEnsureCursorVisible()
{
    int scrolled=0;
    // CHECK IF SCROLL UP IS NEEDED
    if(halScreen.LineCurrent<halScreen.LineVisible) {
        halScreen.LineVisible=halScreen.LineCurrent;
        scrolled=1;
    }

    // SCROLL DOWN AS NEEDED
    if(halScreen.LineCurrent>=halScreen.LineVisible+halScreen.NumLinesVisible) {
        halScreen.LineVisible=halScreen.LineCurrent-(halScreen.NumLinesVisible-1);
        scrolled=1;
    }

    // SCROLL LEFT AS NEEDED
    if(halScreen.CursorX<halScreen.XVisible+8) {
        if(halScreen.XVisible>0) {
        if(halScreen.XVisible<8) halScreen.XVisible=0;
        else {
            halScreen.XVisible=(halScreen.CursorX>8)? halScreen.CursorX-8:0;
        scrolled=1;
        }
        }
    }

    // SCROLL RIGHT AS NEEDED
    if(halScreen.CursorX>(halScreen.XVisible+SCREEN_WIDTH-8)) {
        halScreen.XVisible=halScreen.CursorX-(SCREEN_WIDTH-8);
        scrolled=1;
    }

    if(scrolled) halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;

}

void __uicursorupdate()
{
    if(halScreen.CursorState&0x4000) return;    // DON'T UPDATE IF LOCKED
    halScreen.CursorState^=0x8000;              // FLIP STATE BIT
    halScreen.DirtyFlag|=CMDLINE_CURSORDIRTY;
    DRAWSURFACE scr;
    ggl_initscr(&scr);
    halRedrawCmdLine(&scr);
}


// OPEN AN EMPTY COMMAND LINE
// MODE CAN BE: 'A','P' OR 'D'
// OR IN ALPHA: 'L','C' OR 'X', WHERE 'X'= L OR C, WHATEVER WAS USED LAST

// ANY OTHER VALUE WILL DEFAULT TO MODE 'D'

void uiOpenCmdLine(BINT mode)
{
    BINT AlphaMode;
    CmdLineText=(WORDPTR)empty_string;
    CmdLineCurrentLine=(WORDPTR)empty_string;
    CmdLineUndoList=(WORDPTR)empty_list;
    halScreen.LineCurrent=1;
    halScreen.LineIsModified=-1;
    halScreen.LineVisible=1;
    halScreen.NumLinesVisible=1;
    halScreen.CursorX=0;
    halScreen.CursorPosition=0;
    halScreen.CmdLineState=CMDSTATE_OPEN;
    halScreen.CmdLineIndent=0;
    halScreen.SelEndLine=halScreen.SelStartLine=-1;
    halScreen.SelStart=halScreen.SelEnd=0;

    if(((halScreen.CursorState&0xff)=='L')||((halScreen.CursorState&0xff)=='C')) AlphaMode=halScreen.CursorState&0xff;
    else if(((halScreen.CursorState>>24)=='L')||((halScreen.CursorState>>24)=='C')) AlphaMode=halScreen.CursorState>>24;
         else AlphaMode='L';

    if((mode=='A')||(mode=='D')||(mode=='P')) halScreen.CursorState=mode | (AlphaMode<<24);
    else {
        halScreen.CursorState='D';
        if((mode=='L')||(mode=='C')||(mode=='X')) {
            halScreen.CursorState<<=24;
            if(mode!='X') halScreen.CursorState|=mode;
            else halScreen.CursorState|=AlphaMode;
        }
        else halScreen.CursorState|=AlphaMode<<24;
    }

    halScreen.XVisible=0;
    halScreen.CursorTimer=tmr_eventcreate(&__uicursorupdate,700,1);
    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;

}

// CLOSE THE COMMAND LINE
void uiCloseCmdLine()
{
    if(halScreen.CursorTimer>=0) {
        tmr_eventkill(halScreen.CursorTimer);
        halScreen.CursorTimer=-1;
    }

    CmdLineText=(WORDPTR)empty_string;
    CmdLineCurrentLine=(WORDPTR)empty_string;
    CmdLineUndoList=(WORDPTR)empty_list;
    halScreen.LineCurrent=1;
    halScreen.LineIsModified=-1;
    halScreen.LineVisible=1;
    halScreen.NumLinesVisible=1;
    halScreen.CursorX=0;
    halScreen.CursorPosition=0;
    halScreen.CursorState&=0xff0000ff;
    halScreen.XVisible=0;
    if(halScreen.CmdLineState&CMDSTATE_ACACTIVE) halScreen.DirtyFlag|=STAREA_DIRTY;
    halScreen.CmdLineState=0;
    halScreen.ACSuggestion=0;
    SuggestedObject=0;
    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;

}


void uiSetCurrentLine(BINT line)
{

    if(line==halScreen.LineCurrent) return;
    // LOCK CURSOR
    halScreen.CursorState|=0x4000;

    if(halScreen.LineIsModified>0) {
        // INSERT THE MODIFIED TEXT BACK INTO ORIGINAL TEXT

        uiModifyLine(0);

        if(Exceptions)  return;

    }

    halScreen.LineCurrent=line;
        uiExtractLine(halScreen.LineCurrent);

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            uiOpenCmdLine(0);
            //CmdLineText=(WORDPTR)empty_string;
            //CmdLineCurrentLine=(WORDPTR)empty_string;
            //CmdLineUndoList=(WORDPTR)empty_list;
            return;
        }

    // POSITION THE CURSOR IN THE NEW LINE, TRYING TO PRESERVE THE X COORDINATE

    BINT len=rplStrSize(CmdLineCurrentLine);
    BINT targetx=halScreen.CursorX;
    BYTEPTR ptr=(BYTEPTR)(CmdLineCurrentLine+1);
    BYTEPTR ptr2=(BYTEPTR)StringCoordToPointer((char *)ptr,(char *)ptr+len,*halScreen.FontArray[FONT_CMDLINE],&targetx);

    halScreen.CursorX=targetx;
    halScreen.CursorPosition=ptr2-ptr;

    uiEnsureCursorVisible();

    // UNLOCK CURSOR
    halScreen.CursorState&=~0xc000;

    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;


}


// MAIN FUNCTION TO INSERT TEXT AT THE CURRENT CURSOR OFFSET
// RETURNS THE NUMBER OF LINES ADDED TO THE TEXT (IF ANY)
BINT uiInsertCharacters(BYTEPTR string)
{
BYTEPTR end=string+stringlen((char *)string);

return uiInsertCharactersN(string,end);
}

BINT uiInsertCharactersN(BYTEPTR string,BYTEPTR endstring)
{
if(endstring<=string) return 0;

// LOCK CURSOR
halScreen.CursorState|=0x4000;

BINT length=endstring-string;
BINT lenwords=(length+3)>>2;
// PROTECT string FROM POSSIBLE GC
ScratchPointer1=(WORDPTR)string;


if(halScreen.LineIsModified<0) {

uiExtractLine(halScreen.LineCurrent);

if(Exceptions) {
    throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
    // CLEAN UP AND RETURN
    uiOpenCmdLine(0);
    //CmdLineText=(WORDPTR)empty_string;
    //CmdLineCurrentLine=(WORDPTR)empty_string;
    //CmdLineUndoList=(WORDPTR)empty_list;

    return 0;
}

}


if(CmdLineCurrentLine==(WORDPTR)empty_string) {


    WORDPTR newobj=rplAllocTempObLowMem(lenwords);
    if(!newobj) {
        throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        uiOpenCmdLine(0);
        //CmdLineText=(WORDPTR)empty_string;
        //CmdLineCurrentLine=(WORDPTR)empty_string;
        //CmdLineUndoList=(WORDPTR)empty_list;
        return 0;
    }

    CmdLineCurrentLine=newobj;
    *CmdLineCurrentLine=MKPROLOG(DOSTRING,0);   // MAKE AN EMPTY STRING
}
else {
    BINT totallen=rplStrSize(CmdLineCurrentLine)+length;
    lenwords=(totallen+3)>>2;

    if(rplSkipOb(CmdLineCurrentLine)==TempObEnd)  rplResizeLastObject(lenwords-OBJSIZE(*CmdLineCurrentLine));
    else {
        // NOT AT THE END OF TEMPOB
        // MAKE A COPY OF THE OBJECT AT THE END

        WORDPTR newobj=rplAllocTempObLowMem(lenwords);
        if(!newobj) {
            throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            uiOpenCmdLine(0);
            //CmdLineText=(WORDPTR)empty_string;
            //CmdLineCurrentLine=(WORDPTR)empty_string;
            //CmdLineUndoList=(WORDPTR)empty_list;
            return 0;
        }
        rplCopyObject(newobj,CmdLineCurrentLine);
        CmdLineCurrentLine=newobj;

    }
}
if(Exceptions) {
    throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
    // CLEAN UP AND RETURN
    uiOpenCmdLine(0);
    //CmdLineText=(WORDPTR)empty_string;
    //CmdLineCurrentLine=(WORDPTR)empty_string;
    //CmdLineUndoList=(WORDPTR)empty_list;
    return 0;
}

// FINALLY, WE HAVE THE ORIGINAL LINE AT THE END OF TEMPOB, AND ENOUGH MEMORY ALLOCATED TO MAKE THE MOVE

// MOVE THE TAIL TO THE END
memmoveb( ((BYTEPTR)CmdLineCurrentLine)+4+halScreen.CursorPosition+length,((BYTEPTR)CmdLineCurrentLine)+4+halScreen.CursorPosition,rplStrSize(CmdLineCurrentLine)-halScreen.CursorPosition);
// ADD THE NEW DATA IN
memmoveb(((BYTEPTR)CmdLineCurrentLine)+4+halScreen.CursorPosition,ScratchPointer1,length);

// PATCH THE LENGTH OF THE STRING
BINT newlen=rplStrSize(CmdLineCurrentLine);
newlen+=length;
rplSetStringLength(CmdLineCurrentLine,newlen);

halScreen.LineIsModified=1;

// ADVANCE THE CURSOR
// IF THE INSERTED TEXT HAD ANY NEWLINES, THE CURRENT COMMAND LINE HAS MULTIPLE LINES IN ONE
BYTEPTR newstart=((BYTEPTR)CmdLineCurrentLine)+4+halScreen.CursorPosition;
BYTEPTR newend=newstart+length;
BINT nl=0;
while(newstart!=newend) {
    if(*newstart=='\n') ++nl;
    newstart=(BYTEPTR)utf8skip((char *)newstart,(char *)newend);
}

if(nl)  {
// MUST SPLIT THE LINES AND GET THE CURSOR ON THE LAST ONE
    // FIRST, UPDATE THE CURRENT TEXT WITH THE NEW LINE
        uiModifyLine(0);

        if(Exceptions)  return 0;

    BINT newoff=rplStringGetLinePtr(CmdLineText,halScreen.LineCurrent);
    BINT oldoff=halScreen.CursorPosition;
    BINT newline=rplStringGetLinePtr(CmdLineText,halScreen.LineCurrent+nl);

    if(newline<0) newline=rplStrSize(CmdLineText);

    newoff+=halScreen.CursorPosition+length;
    newoff-=newline;

    // MOVE THE CURRENT SELECTION
    if(halScreen.SelStartLine==halScreen.LineCurrent) {
        if(halScreen.SelStart>=halScreen.CursorPosition) {
            halScreen.SelStartLine+=nl;
            halScreen.SelStart+=newoff-oldoff;
        }
    }
    else {
        if(halScreen.SelStartLine>halScreen.LineCurrent) halScreen.SelStartLine+=nl;
    }

    if(halScreen.SelEndLine==halScreen.LineCurrent) {
        if(halScreen.SelEnd>=halScreen.CursorPosition) {
            halScreen.SelEndLine+=nl;
            halScreen.SelEnd+=newoff-oldoff;
        }
    }
    else {
        if(halScreen.SelEndLine>halScreen.LineCurrent) halScreen.SelEndLine+=nl;
    }


    uiSetCurrentLine(halScreen.LineCurrent+nl);
    uiMoveCursor(newoff);

}
else {
halScreen.CursorX+=StringWidthN(((char *)CmdLineCurrentLine)+4+halScreen.CursorPosition,((char *)CmdLineCurrentLine)+4+halScreen.CursorPosition+length,*halScreen.FontArray[FONT_CMDLINE]);

// MOVE CURRENT SELECTION
if((halScreen.SelStartLine==halScreen.LineCurrent)&&(halScreen.SelStart>=halScreen.CursorPosition))    halScreen.SelStart+=length;
if((halScreen.SelEndLine==halScreen.LineCurrent)&&(halScreen.SelEnd>=halScreen.CursorPosition)) halScreen.SelEnd+=length;

halScreen.CursorPosition+=length;

}

halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY;

uiEnsureCursorVisible();
// UNLOCK CURSOR
halScreen.CursorState&=~0xc000;
return nl;
}


// MAIN FUNCTION TO REMOVE TEXT AT THE CURRENT CURSOR OFFSET

void uiRemoveCharacters(BINT length)
{
if(length<=0) return;
int numlines=0;
// LOCK CURSOR
halScreen.CursorState|=0x4000;

do {

if(halScreen.LineIsModified<0) {

uiExtractLine(halScreen.LineCurrent);

if(Exceptions)    return;

}

if(rplSkipOb(CmdLineCurrentLine)!=TempObEnd) {
        // NOT AT THE END OF TEMPOB
        // MAKE A COPY OF THE OBJECT AT THE END
        WORDPTR newobj=rplAllocTempObLowMem(OBJSIZE(*CmdLineCurrentLine));
        if(!newobj) {
            throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            uiOpenCmdLine(0);
            //CmdLineText=(WORDPTR)empty_string;
            //CmdLineCurrentLine=(WORDPTR)empty_string;
            //CmdLineUndoList=(WORDPTR)empty_list;
            return;
        }
        rplCopyObject(newobj,CmdLineCurrentLine);
        CmdLineCurrentLine=newobj;

    }

// FINALLY, WE HAVE THE ORIGINAL LINE AT THE END OF TEMPOB, AND ENOUGH MEMORY ALLOCATED TO MAKE THE MOVE
BINT tailchars=rplStrSize(CmdLineCurrentLine)-halScreen.CursorPosition;
BYTEPTR delete_start=((BYTEPTR)CmdLineCurrentLine)+4+halScreen.CursorPosition;
BYTEPTR delete_end=(BYTEPTR) utf8nskipst((char *)delete_start,(char *)delete_start+tailchars,length);
BINT actualcount=utf8nlen((char *)delete_start,(char *)delete_end);

// MOVE THE TAIL TO THE END
memmoveb( delete_start,delete_end,delete_start+tailchars-delete_end);

// PATCH THE LENGTH OF THE STRING
BINT newlen=rplStrSize(CmdLineCurrentLine);
newlen-=delete_end-delete_start;
rplSetStringLength(CmdLineCurrentLine,newlen);

// TRUNCATE THE OBJECT TO RELEASE MEMORY
rplTruncateLastObject(rplSkipOb(CmdLineCurrentLine));

length-=actualcount;

halScreen.LineIsModified=1;

if(halScreen.SelStartLine==halScreen.LineCurrent) {
    if(halScreen.SelStart>halScreen.CursorPosition) {
        halScreen.SelStart-=delete_end-delete_start;
        if(halScreen.SelStart<halScreen.CursorPosition) halScreen.SelStart=halScreen.CursorPosition;
    }
}
if(halScreen.SelEndLine==halScreen.LineCurrent) {
    if(halScreen.SelEnd>halScreen.CursorPosition) {
        halScreen.SelEnd-=delete_end-delete_start;
        if(halScreen.SelEnd<halScreen.CursorPosition) halScreen.SelEnd=halScreen.CursorPosition;
    }
}



if(length>0) {
    // MORE CHARACTERS TO BE REMOVED!

    uiModifyLine(1);    // MODIFY THE LINE, REMOVE THE NEWLINE AT THE END

    if(Exceptions) return;

    --length;

    if(halScreen.SelStartLine>halScreen.LineCurrent) {
        --halScreen.SelStartLine;
        if(halScreen.SelStartLine==halScreen.LineCurrent) halScreen.SelStart+=halScreen.CursorPosition;
    }
    if(halScreen.SelEndLine>halScreen.LineCurrent) {
        --halScreen.SelEndLine;
        if(halScreen.SelEndLine==halScreen.LineCurrent) halScreen.SelEnd+=halScreen.CursorPosition;
    }

    uiExtractLine(halScreen.LineCurrent);

    if(Exceptions) return;

    ++numlines;

}

} while(length>0);




if(!numlines) halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY;
else halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;
// UNLOCK CURSOR
halScreen.CursorState&=~0xc000;

}

// IF THE LAST CHARACTER IS NOT A BLANK, THEN INSERT A BLANK SPACE
void uiSeparateToken()
{

    if(halScreen.LineIsModified<0) {

    uiExtractLine(halScreen.LineCurrent);

    if(Exceptions) {
        throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        uiOpenCmdLine(0);
        //CmdLineText=(WORDPTR)empty_string;
        //CmdLineCurrentLine=(WORDPTR)empty_string;
        //CmdLineUndoList=(WORDPTR)empty_list;
        return;
    }

    }

    BYTEPTR start=(BYTEPTR) (CmdLineCurrentLine+1);
    BYTEPTR lastchar=start+halScreen.CursorPosition-1;
    if(lastchar>=start) {
        if(*lastchar!=' ')  uiInsertCharacters((BYTEPTR)" ");
    }
}


// COPY THE EDITED LINE BACK INTO THE ORIGINAL TEXT
// CREATES A COPY OF THE TEXT
void uiModifyLine(int dontaddnewline)
{
    WORDPTR newobj;
    BINT newsize;

    // GET A NEW OBJECT WITH ROOM FOR THE ENTIRE TEXT
    newobj=rplAllocTempObLowMem( (rplStrSize(CmdLineText)+rplStrSize(CmdLineCurrentLine)+1+ 3)>>2);

    if(Exceptions) {
        throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        uiOpenCmdLine(0);
        //CmdLineText=(WORDPTR)empty_string;
        //CmdLineCurrentLine=(WORDPTR)empty_string;
        //CmdLineUndoList=(WORDPTR)empty_list;
        return;
    }
    BYTEPTR src=(BYTEPTR)(CmdLineText+1),dest=(BYTEPTR)(newobj+1);
    BINT totallen=rplStrSize(CmdLineText);
    BINT lineoff=rplStringGetLinePtr(CmdLineText,halScreen.LineCurrent);
    if(lineoff<0) lineoff=totallen;
    BYTEPTR startline=src+lineoff;
    lineoff=rplStringGetLinePtr(CmdLineText,halScreen.LineCurrent+1);
    if(lineoff<0) lineoff=totallen;
    BYTEPTR endline=src+lineoff;
    if(endline>startline) {
        if((!dontaddnewline)&&(*(endline-1)=='\n')) --endline;
    }

    // COPY ALL PREVIOUS LINES TO NEW OBJECT
    newsize=startline-src+rplStrSize(CmdLineCurrentLine);

    memmoveb(dest,src,startline-src);
    // COPY THE NEW LINE TO THE OBJECT
    memmoveb(dest+(startline-src),(WORDPTR)(CmdLineCurrentLine+1),rplStrSize(CmdLineCurrentLine));
    // COPY THE REST BACK
    if(endline<src+totallen) {
        // APPEND A NEWLINE AND KEEP GOING
        dest+=startline-src+rplStrSize(CmdLineCurrentLine);
        newsize+=src+totallen-endline;
        if((!dontaddnewline)&&(*endline!='\n')) { *dest++='\n'; ++newsize; }
        memmoveb(dest,endline,src+totallen-endline);
    }

    rplSetStringLength(newobj,newsize);

    CmdLineText=newobj;
    halScreen.LineIsModified=0;

}

// COPY A LINE FROM THE TEXT INTO THE EDITING BUFFER
void uiExtractLine(BINT line)
{
    WORDPTR newobj;

    // GET A NEW OBJECT WITH ROOM FOR THE ENTIRE LINE
    BYTEPTR text=(BYTEPTR)(CmdLineText+1);
    BYTEPTR startline=text+rplStringGetLinePtr(CmdLineText,line);
    BYTEPTR endline=text+rplStringGetLinePtr(CmdLineText,line+1);

    if(startline<text) {
        // CREATE AN EMPTY LINE
        startline=endline=text;
    }

    if(endline<text) endline=text+rplStrSize(CmdLineText);
    if(endline>startline) {
        // DO NOT EXTRACT THE FINAL NEWLINE CHARACTER
        if(*(endline-1)=='\n') --endline;
    }


    newobj=rplAllocTempObLowMem( ((endline-startline)+ 3)>>2);

    if(Exceptions) {
        throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        uiOpenCmdLine(0);
        //CmdLineText=(WORDPTR)empty_string;
        //CmdLineCurrentLine=(WORDPTR)empty_string;
        //CmdLineUndoList=(WORDPTR)empty_list;
        return;
    }

    // ZERO PADDING THE LAST WORD
    newobj[(endline-startline)>>2]=0;

    // COPY LINE TO NEW OBJECT
    memmoveb(newobj+1,(BYTEPTR)(CmdLineText+1)+(startline-text),endline-startline);

    rplSetStringLength(newobj,endline-startline);

    CmdLineCurrentLine=newobj;


}

BINT uiGetIndentLevel(BINT *isemptyline)
{
    // GET THE CURRENT INDENT LEVEL OF THE CURRENT LINE
    if(halScreen.LineIsModified<0) {
        uiExtractLine(halScreen.LineCurrent);

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            uiOpenCmdLine(0);
            //CmdLineText=(WORDPTR)empty_string;
            //CmdLineCurrentLine=(WORDPTR)empty_string;
            //CmdLineUndoList=(WORDPTR)empty_list;
            return 0;
        }

        }

    BYTEPTR ptr=(BYTEPTR)(CmdLineCurrentLine+1),end=ptr+rplStrSize(CmdLineCurrentLine);
    if(end>ptr+halScreen.CursorPosition) end=ptr+halScreen.CursorPosition;
    BINT level=0;
    while((ptr<end)&&(*ptr==' ')) { ++level; ++ptr; }

    if(isemptyline) {
        if(ptr==end) *isemptyline=1;
        else *isemptyline=0;

    }
    return level;

}


// MOVE THE CURSOR TO THE GIVEN OFFSET WITHIN THE STRING
void uiMoveCursor(BINT offset)
{
if(halScreen.LineIsModified<0) {
    uiExtractLine(halScreen.LineCurrent);

    if(Exceptions) {
        throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        uiOpenCmdLine(0);
        //CmdLineText=(WORDPTR)empty_string;
        //CmdLineCurrentLine=(WORDPTR)empty_string;
        //CmdLineUndoList=(WORDPTR)empty_list;
        return;
    }

    }

BYTEPTR ptr=(BYTEPTR )(CmdLineCurrentLine+1),ptr2;
BINT len=rplStrSize(CmdLineCurrentLine);

if(offset>len) offset=len;
if(offset<0) offset=0;

halScreen.CursorState|=0x4000;

// AVOID USING OFFSET THAT FALLS BETWEEN BYTES OF THE SAME CODEPOINT
ptr2=(BYTEPTR)utf8findst((char *)ptr+offset,(char *)ptr+len);

//while((ptr2-ptr<offset)&&(ptr2<ptr+len)) ptr2=(BYTEPTR)utf8skip((char *)ptr2,(char *)ptr+len);

offset=ptr2-ptr;

halScreen.CursorPosition=offset;

halScreen.CursorX=StringWidthN((char *)ptr,(char *)ptr2,*halScreen.FontArray[FONT_CMDLINE]);

halScreen.CursorState&=~0xc000;

halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY;
}


// MOVE THE CURSOR LEFT, NCHARS IS GIVEN IN UNICODE CODEPOINTS
void uiCursorLeft(BINT nchars)
{
    BYTEPTR ptr,ptr2;
    BINT offset;



    if(halScreen.LineIsModified<0) {
        uiExtractLine(halScreen.LineCurrent);

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            uiOpenCmdLine(0);
            //CmdLineText=(WORDPTR)empty_string;
            //CmdLineCurrentLine=(WORDPTR)empty_string;
            //CmdLineUndoList=(WORDPTR)empty_list;
            return;
        }

        }

    ptr=(BYTEPTR )(CmdLineCurrentLine+1);
    // AVOID USING OFFSET THAT FALLS BETWEEN BYTES OF THE SAME CODEPOINT
    ptr2=ptr+halScreen.CursorPosition;

    while(nchars &&(ptr2>ptr)) {
        ptr2=(BYTEPTR)utf8rskipst((char *)ptr2,(char *)ptr);
        --nchars;
    }

    if(nchars) {
        // THERE'S MORE CHARACTERS LEFT!
        if(halScreen.LineCurrent==1) {
            // BEGINNING OF FIRST LINE, NOWHERE ELSE TO GO
            nchars=0;
        }
        else {
            if(halScreen.LineIsModified>0) {
                // INSERT THE MODIFIED TEXT BACK INTO ORIGINAL TEXT

                uiModifyLine(0);

                if(Exceptions)  return;

            }

            // RELOAD VALUES FOR THE WHOLE TEXT
            offset=rplStringGetLinePtr(CmdLineText,halScreen.LineCurrent);
            if(offset<0) offset=rplStrSize(CmdLineText);
            ptr=(BYTEPTR )(CmdLineText+1);
            ptr2=ptr+offset;    // THIS IS THE BEGINNING OF THE CURRENT LINE
            while(nchars &&(ptr2>ptr)) {
                ptr2=(BYTEPTR)utf8rskipst((char *)ptr2,(char *)ptr);
                if(*ptr2=='\n') --halScreen.LineCurrent;
                --nchars;
            }

            if(nchars) nchars=0;    // NO MORE TEXT!
            if(halScreen.LineCurrent<1) halScreen.LineCurrent=1;

            offset=ptr2-uiGetStartOfLine(ptr2,ptr);

            uiExtractLine(halScreen.LineCurrent);
            if(Exceptions) {
                throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
                // CLEAN UP AND RETURN
                uiOpenCmdLine(0);
                //CmdLineText=(WORDPTR)empty_string;
                //CmdLineCurrentLine=(WORDPTR)empty_string;
                //CmdLineUndoList=(WORDPTR)empty_list;
                return;
            }

            uiMoveCursor(offset);

            halScreen.CursorState&=~0xc000;

            halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;

            uiEnsureCursorVisible();

            return;
        }
    }

    offset=ptr2-ptr;

    halScreen.CursorPosition=offset;

    halScreen.CursorX=StringWidthN((char *)ptr,(char *)ptr2,*halScreen.FontArray[FONT_CMDLINE]);

    halScreen.CursorState&=~0xc000;

    halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY;

    uiEnsureCursorVisible();
}

void uiCursorRight(BINT nchars)
{
    if(halScreen.LineIsModified<0) {
        uiExtractLine(halScreen.LineCurrent);

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            uiOpenCmdLine(0);
            //CmdLineText=(WORDPTR)empty_string;
            //CmdLineCurrentLine=(WORDPTR)empty_string;
            //CmdLineUndoList=(WORDPTR)empty_list;
            return;
        }

        }

    BYTEPTR ptr=(BYTEPTR )(CmdLineCurrentLine+1),ptr2;
    BINT len=rplStrSize(CmdLineCurrentLine);
    BINT offset;

    // AVOID USING OFFSET THAT FALLS BETWEEN BYTES OF THE SAME CODEPOINT
    ptr2=ptr+halScreen.CursorPosition;
    while(nchars &&(ptr2<ptr+len)) { ptr2=(BYTEPTR)utf8skipst((char *)ptr2,(char *)ptr+len); --nchars; }

    if(nchars) {
        // THERE'S MORE CHARACTERS LEFT!
        BINT totallines=rplStringCountLines(CmdLineText);

        if(halScreen.LineCurrent==totallines) {
            // LAST LINE, NOWHERE ELSE TO GO
            nchars=0;
        }
        else {
            if(halScreen.LineIsModified>0) {
                // INSERT THE MODIFIED TEXT BACK INTO ORIGINAL TEXT

                uiModifyLine(0);

                if(Exceptions)  return;

            }

            // RELOAD VALUES FOR THE WHOLE TEXT
            offset=rplStringGetLinePtr(CmdLineText,halScreen.LineCurrent);
            if(offset<0) offset=rplStrSize(CmdLineText);
            ptr=(BYTEPTR )(CmdLineText+1);
            len=rplStrSize(CmdLineText);
            ptr2=uiGetEndOfLine(ptr+offset,ptr+len);    // THIS IS THE END OF THE CURRENT LINE
            while(nchars &&(ptr2<ptr+len)) {
                if(*ptr2=='\n') ++halScreen.LineCurrent;
                ptr2=(BYTEPTR)utf8skipst((char *)ptr2,(char *)ptr+len);
                --nchars;
            }

            if(nchars) nchars=0;    // NO MORE TEXT!
            if(halScreen.LineCurrent>totallines) halScreen.LineCurrent=totallines;

            offset=ptr2-uiGetStartOfLine(ptr2,ptr);

            uiExtractLine(halScreen.LineCurrent);
            if(Exceptions) {
                throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
                // CLEAN UP AND RETURN
                uiOpenCmdLine(0);
                //CmdLineText=(WORDPTR)empty_string;
                //CmdLineCurrentLine=(WORDPTR)empty_string;
                //CmdLineUndoList=(WORDPTR)empty_list;
                return;
            }

            uiMoveCursor(offset);

            halScreen.CursorState&=~0xc000;

            halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;

            uiEnsureCursorVisible();

            return;
        }
    }





    offset=ptr2-ptr;

    halScreen.CursorPosition=offset;

    halScreen.CursorX=StringWidthN((char *)ptr,(char *)ptr2,*halScreen.FontArray[FONT_CMDLINE]);

    halScreen.CursorState&=~0xc000;

    halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY;

    uiEnsureCursorVisible();

}



// MOVE THE CURSOR DOWN BY NLINES
void uiCursorDown(BINT nlines)
{
BINT totallines=rplStringCountLines(CmdLineText);
BINT newline=halScreen.LineCurrent+nlines;
if(newline>totallines) newline=totallines;
if(newline<1) newline=1;

uiSetCurrentLine(newline);


}

// MOVE THE CURSOR UP BY NLINES
void uiCursorUp(BINT nlines)
{
uiCursorDown(-nlines);
}


void uiCursorEndOfLine()
{
uiMoveCursor(2147483647);
uiEnsureCursorVisible();
}

void uiCursorStartOfLine()
{
uiMoveCursor(0);
uiEnsureCursorVisible();

}

void uiCursorStartOfText()
{
uiSetCurrentLine(1);
uiCursorStartOfLine();
}

void uiCursorEndOfText()
{
    uiSetCurrentLine(rplStringCountLines(CmdLineText));
    uiCursorEndOfLine();
}

void uiCursorPageRight()
{
    if(halScreen.LineIsModified<0) {

    uiExtractLine(halScreen.LineCurrent);

    if(Exceptions) {
        throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        uiOpenCmdLine(0);
        //CmdLineText=(WORDPTR)empty_string;
        //CmdLineCurrentLine=(WORDPTR)empty_string;
        //CmdLineUndoList=(WORDPTR)empty_list;
        return;
    }
    }

// POSITION THE CURSOR TRYING TO PRESERVE THE X COORDINATE

BINT len=rplStrSize(CmdLineCurrentLine);
BINT targetx=halScreen.CursorX+SCREEN_WIDTH;
BYTEPTR ptr=(BYTEPTR)(CmdLineCurrentLine+1);
BYTEPTR ptr2=(BYTEPTR)StringCoordToPointer((char *)ptr,(char *)ptr+len,*halScreen.FontArray[FONT_CMDLINE],&targetx);

halScreen.CursorX=targetx;
halScreen.CursorPosition=ptr2-ptr;

uiEnsureCursorVisible();

// UNLOCK CURSOR
halScreen.CursorState&=~0xc000;

halScreen.DirtyFlag|=CMDLINE_CURSORDIRTY|CMDLINE_LINEDIRTY;

}


void uiCursorPageLeft()
{
    if(halScreen.LineIsModified<0) {

    uiExtractLine(halScreen.LineCurrent);

    if(Exceptions) {
        throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        uiOpenCmdLine(0);

        //CmdLineText=(WORDPTR)empty_string;
        //CmdLineCurrentLine=(WORDPTR)empty_string;
        //CmdLineUndoList=(WORDPTR)empty_list;
        return;
    }
    }

// POSITION THE CURSOR TRYING TO PRESERVE THE X COORDINATE

BINT len=rplStrSize(CmdLineCurrentLine);
BINT targetx=halScreen.CursorX-SCREEN_WIDTH;
BYTEPTR ptr=(BYTEPTR)(CmdLineCurrentLine+1);
BYTEPTR ptr2=(BYTEPTR)StringCoordToPointer((char *)ptr,(char *)ptr+len,*halScreen.FontArray[FONT_CMDLINE],&targetx);

halScreen.CursorX=targetx;
halScreen.CursorPosition=ptr2-ptr;

uiEnsureCursorVisible();

// UNLOCK CURSOR
halScreen.CursorState&=~0xc000;

halScreen.DirtyFlag|=CMDLINE_CURSORDIRTY|CMDLINE_LINEDIRTY;

}


void uiCursorPageUp()
{
    BINT linesperpage=halScreen.NumLinesVisible;

    if(linesperpage<6) linesperpage=6;

    uiCursorDown(-linesperpage);

}

void uiCursorPageDown()
{
    BINT linesperpage=halScreen.NumLinesVisible;

    if(linesperpage<6) linesperpage=6;

    uiCursorDown(linesperpage);

}



// FIND THE START OF A NUMBER IN THE COMMAND LINE, ONLY USED BY +/- ROUTINE
BYTEPTR uiFindNumberStart()
{
    if(halScreen.LineIsModified<0) {
        uiExtractLine(halScreen.LineCurrent);

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            uiOpenCmdLine(0);

            //CmdLineText=(WORDPTR)empty_string;
            //CmdLineCurrentLine=(WORDPTR)empty_string;
            //CmdLineUndoList=(WORDPTR)empty_list;
            return NULL;
        }

        }

    BYTEPTR line=(BYTEPTR )(CmdLineCurrentLine+1);
    BYTEPTR end,start,ptr;
    BINT len=rplStrSize(CmdLineCurrentLine);
    BINT flags;

    // FIND NUMBER BEFORE
    ptr=line+halScreen.CursorPosition;
    end=ptr;
    start=NULL;
    flags=0;
    while(end<line+len) {
        if((*end>='0')&&(*end<='9')) { ++end; continue; }
        if(*end=='.') { ++end; continue; }

        if((*end>='A')&&(*end<='F')) { if(!start) start=end; ++end; continue; }
        if((*end>='a')&&(*end<='f')) { if(!start) start=end; ++end; continue; }
        if(*end=='h') { flags=3; break; }
        if(*end=='o') { if(start) end=start; else flags=1; break; }
        // ANY OTHER CHARACTER ENDS THE NUMBER
        if(start) {
            end=start;
            if(*end=='b') flags=1;
            if(*end=='d') flags=1;
        }
        break;
    }
    // HERE WE HAVE THE END OF THE NUMBER, AND flags=1 IF THE NUMBER STARTS WITH #
    // NOW FIND THE START OF THE NUMBER
    start=ptr;
    if(start>=end) start=end-1;

    while(start>=line) {
        if((*start>='0')&&(*start<='9')) { --start; continue; }
        if(*start=='.') { --start; continue; }
        if((flags&2)&&(*start>='A')&&(*start<='F')) { --start; continue; }
        if((flags&2)&&(*start>='a')&&(*start<='f')) {--start; continue; }

        // ANY OTHER CHARACTER SHOULD END THE NUMBER
        if((flags&1)&&(*start=='#')) break;
        ++start;
        break;
    }

    if(start<line) start=line;

    // HERE START POINTS TO THE FIRST CHARACTER IN THE NUMBER

    if(start>=end) return NULL;  // THERE WAS NO NUMBER
    return start;
}

// ADD OR SUBTRACT VISIBLE LINES TO THE OPEN COMMAND LINE
void uiStretchCmdLine(BINT addition)
{

    if(!(halScreen.CmdLineState&CMDSTATE_OPEN)) return;

    if(halScreen.CmdLineState&CMDSTATE_FULLSCREEN) return;

    BINT oldlvis=halScreen.NumLinesVisible;

    halScreen.NumLinesVisible+=addition;
    if(halScreen.NumLinesVisible<1) halScreen.NumLinesVisible=1;

    halSetCmdLineHeight((*halScreen.FontArray[FONT_CMDLINE])->BitmapHeight*halScreen.NumLinesVisible+2);
    if(halScreen.CmdLine!=(*halScreen.FontArray[FONT_CMDLINE])->BitmapHeight*halScreen.NumLinesVisible+2) {
        // NO ROOM, ADJUST NUMBER OF VISIBLE LINES
        BINT actuallines=(halScreen.CmdLine-2)/(*halScreen.FontArray[FONT_CMDLINE])->BitmapHeight;
        halScreen.NumLinesVisible=actuallines;
        if(halScreen.NumLinesVisible<1) halScreen.NumLinesVisible=1;
    }

    // TRY TO KEEP THE CURSOR WHERE IT IS
    if(halScreen.NumLinesVisible>oldlvis) {
        halScreen.LineVisible-=halScreen.NumLinesVisible-oldlvis;
        if(halScreen.LineVisible<1) halScreen.LineVisible=1;

    }

}

// CALL THIS FUNCTION WHENEVER THE COMMAND LINE CHANGES, TO INDICATE THE
// AUTOCOMPLETE FUNCTION NEEDS TO BE UPDATED
void uiAutocompleteUpdate()
{
    // LOOK AT THE TEXT TO THE LEFT OF THE CURSOR FOR A TOKEN
    // THEN START A NEW SEARCH
    if(!(halScreen.CmdLineState&CMDSTATE_OPEN)) return;

    if(halScreen.LineIsModified<0) {
        uiExtractLine(halScreen.LineCurrent);

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);

            // CLEAN UP AND RETURN
            uiOpenCmdLine(0);
            //CmdLineText=(WORDPTR)empty_string;
            //CmdLineCurrentLine=(WORDPTR)empty_string;
            //CmdLineUndoList=(WORDPTR)empty_list;
            return;
        }

        }


    BYTEPTR start=(BYTEPTR)(CmdLineCurrentLine+1);
    BYTEPTR end=start+halScreen.CursorPosition;
    BYTEPTR ptr,tokptr=(BYTEPTR)utf8rskipst((char *)end,(char *)start);
    BINT char1,char2;
    // THESE ARE CHARACTERS THAT WOULD STOP A TOKEN SEARCH
    static const char const forbiddenChars[]="{}[]()#;:, \"\'_`@|«»";  // OUTSIDE OF ALG. MODE
    static const char const algforbiddenChars[]="+-*/\\{}[]()#!^;:<>=, \"\'_`@|√«»≤≥≠→∡"; // IN ALG. MODE

    const char *forbstring=( (halScreen.CursorState&0xff)=='A')? algforbiddenChars:forbiddenChars;


    while(tokptr>=start) {
        ptr=(BYTEPTR )forbstring;
        char1=utf82cp((char *)tokptr,(char *)end);
        do {
        char2=utf82cp((char *)ptr,(char *)ptr+4);
        if(char1==char2) {
            tokptr=(BYTEPTR)utf8skip((char *)tokptr,(char *)end);
            break;
        }
        ptr=(BYTEPTR)utf8skip((char *)ptr,(char *)ptr+4);
        } while(*ptr);
        if(*ptr) {
            // WE FOUND THE START OF THE TOKEN
            break;
        }
        if(tokptr>start) tokptr=(BYTEPTR)utf8rskipst((char *)tokptr,(char *)start);
        else --tokptr;

    }
    if(tokptr<start) tokptr=start;

    // HERE WE HAVE THE ISOLATED TOKEN WE WANT TO AUTOCOMPLETE
    halScreen.ACTokenStart=tokptr-start;
    BINT oldstate=halScreen.CmdLineState&CMDSTATE_ACACTIVE;
    if(tokptr==end) halScreen.CmdLineState&=~(CMDSTATE_ACACTIVE|CMDSTATE_ACUPDATE);
    else halScreen.CmdLineState|=CMDSTATE_ACUPDATE|CMDSTATE_ACACTIVE;

    halScreen.ACSuggestion=rplGetNextSuggestion(-1,0,tokptr,end);

    if(oldstate || (oldstate!=(halScreen.CmdLineState&CMDSTATE_ACACTIVE))) halScreen.DirtyFlag|=STAREA_DIRTY;


}


void uiAutocompNext()
{
    if(halScreen.CmdLineState&CMDSTATE_ACACTIVE) {

    if(halScreen.LineIsModified<0) {
        uiExtractLine(halScreen.LineCurrent);

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            uiOpenCmdLine(0);

            //CmdLineText=(WORDPTR)empty_string;
            //CmdLineCurrentLine=(WORDPTR)empty_string;
            //CmdLineUndoList=(WORDPTR)empty_list;
            return;
        }

        }


    BYTEPTR start=(BYTEPTR)(CmdLineCurrentLine+1);
    BYTEPTR end=start+halScreen.CursorPosition;

    halScreen.ACSuggestion=rplGetNextSuggestion(halScreen.ACSuggestion,SuggestedObject,start+halScreen.ACTokenStart,end);

    halScreen.CmdLineState|=CMDSTATE_ACUPDATE;
    halScreen.DirtyFlag|=STAREA_DIRTY;
    }

}

void uiAutocompPrev()
{
    if(halScreen.CmdLineState&CMDSTATE_ACACTIVE) {

    if(halScreen.LineIsModified<0) {
        uiExtractLine(halScreen.LineCurrent);

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            uiOpenCmdLine(0);

            //CmdLineText=(WORDPTR)empty_string;
            //CmdLineCurrentLine=(WORDPTR)empty_string;
            //CmdLineUndoList=(WORDPTR)empty_list;
            return;
        }

        }


    BYTEPTR start=(BYTEPTR)(CmdLineCurrentLine+1);
    BYTEPTR end=start+halScreen.CursorPosition;

    halScreen.ACSuggestion=rplGetPrevSuggestion(halScreen.ACSuggestion,SuggestedObject,start+halScreen.ACTokenStart,end);

    halScreen.CmdLineState|=CMDSTATE_ACUPDATE;
    halScreen.DirtyFlag|=STAREA_DIRTY;
    }

}

// INSERT THE CURRENT SUGGESTION IN THE COMMAND LINE

void uiAutocompInsert()
{
    if(halScreen.ACSuggestion!=0) {
    BYTEPTR tokstart=uiAutocompStringStart();
    BYTEPTR tokend=uiAutocompStringEnd();

    WORDPTR cmdname=rplDecompile(((ISPROLOG(halScreen.ACSuggestion))? SuggestedObject : (&halScreen.ACSuggestion)),DECOMP_NOHINTS);
    BYTEPTR namest=(BYTEPTR)(cmdname+1);
    BYTEPTR nameend=namest+rplStrSize(cmdname);

    if( (!cmdname) || Exceptions) {
        // JUST IGNORE, CLEAR EXCEPTIONS AND RETURN;
        Exceptions=0;
        return;
    }


    // MOVE THE CURSOR TO THE START OF THE TOKEN;
    BINT nchars=0;
    while(tokstart!=tokend) { ++nchars; tokstart=(BYTEPTR)utf8skipst((char *)tokstart,(char *)tokend); }

    uiCursorLeft(nchars);
    uiInsertCharactersN(namest,nameend);
    uiRemoveCharacters(nchars);


    }

}



BYTEPTR uiAutocompStringStart()
{
    if(halScreen.LineIsModified<0) {
        uiExtractLine(halScreen.LineCurrent);

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            uiOpenCmdLine(0);
            //CmdLineText=(WORDPTR)empty_string;
            //CmdLineCurrentLine=(WORDPTR)empty_string;
            //CmdLineUndoList=(WORDPTR)empty_list;
            return NULL;
        }

        }

    BYTEPTR start=(BYTEPTR)(CmdLineCurrentLine+1);

    return start+halScreen.ACTokenStart;
}


BYTEPTR uiAutocompStringEnd()
{
    if(halScreen.LineIsModified<0) {
        uiExtractLine(halScreen.LineCurrent);

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            uiOpenCmdLine(0);
            //CmdLineText=(WORDPTR)empty_string;
            //CmdLineCurrentLine=(WORDPTR)empty_string;
            //CmdLineUndoList=(WORDPTR)empty_list;
            return NULL;
        }

        }

    BYTEPTR start=(BYTEPTR)(CmdLineCurrentLine+1);
    BYTEPTR end=start+halScreen.CursorPosition;

    return end;
}

BYTEPTR uiAutocompStringTokEnd()
{
    if(halScreen.LineIsModified<0) {
        uiExtractLine(halScreen.LineCurrent);

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            uiOpenCmdLine(0);
            //CmdLineText=(WORDPTR)empty_string;
            //CmdLineCurrentLine=(WORDPTR)empty_string;
            //CmdLineUndoList=(WORDPTR)empty_list;
            return NULL;
        }

        }

    BYTEPTR start=(BYTEPTR)(CmdLineCurrentLine+1);
    BINT len=rplStrSize(CmdLineCurrentLine);
    BYTEPTR end=start+len,ptr=start+halScreen.CursorPosition;

    while((ptr<end) && (*((char *)ptr)!=' ') && (*((char *)ptr)!='\t') && (*((char *)ptr)!='\n') && (*((char *)ptr)!='\r')) ++ptr;
    return ptr;
}

// SET SELECTION START AT THE CURRENT CURSOR POSITION
void uiSetSelectionStart()
{
    halScreen.SelStart=halScreen.CursorPosition;
    halScreen.SelStartLine=halScreen.LineCurrent;

    if(halScreen.SelEndLine<halScreen.SelStartLine) {
        halScreen.SelEndLine=-1;
        halScreen.SelEnd=0;
    }
    else {
        if(halScreen.SelEndLine==halScreen.SelStartLine) {
            if(halScreen.SelEnd<halScreen.SelStart) { halScreen.SelEndLine=-1; halScreen.SelEnd=0; }
        }
    }


    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;


}

// SET SELECTION START AT THE CURRENT CURSOR POSITION
void uiSetSelectionEnd()
{
    halScreen.SelEnd=halScreen.CursorPosition;
    halScreen.SelEndLine=halScreen.LineCurrent;

    if(halScreen.SelStartLine>halScreen.SelEndLine) {
        halScreen.SelStartLine=-1;
        halScreen.SelStart=0;
    }
    else {
        if(halScreen.SelStartLine<0) {
            halScreen.SelStartLine=1;
            halScreen.SelStart=0;
        }
        if(halScreen.SelEndLine==halScreen.SelStartLine) {
            if(halScreen.SelEnd<halScreen.SelStart) { halScreen.SelStartLine=-1; halScreen.SelStart=0; }
        }
    }

    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;
}


// CREATE A NEW RPL STRING WITH THE TEXT SELECTED IN THE EDITOR

WORDPTR uiExtractSelection()
{
    if(!(halScreen.CmdLineState&CMDSTATE_OPEN)) return 0;

    if(halScreen.SelStartLine<0) return 0;
    if(halScreen.SelEndLine<0) return 0;

    // COMMIT ANY CHANGES TO THE CURRENT LINE
    if(halScreen.LineIsModified>0) uiModifyLine(0);
    if(Exceptions) return 0;


    BINT selst=rplStringGetLinePtr(CmdLineText,halScreen.SelStartLine)+halScreen.SelStart;
    BINT selend=rplStringGetLinePtr(CmdLineText,halScreen.SelEndLine)+halScreen.SelEnd;

    if(selend<=selst) return 0;

    // HERE WE NEED TO CREATE A NEW OBJECT
    WORDPTR newstr=rplCreateString((BYTEPTR)(CmdLineText+1)+selst,(BYTEPTR)(CmdLineText+1)+selend);

    return newstr;
}


BINT uiDeleteSelection()
{
    if(!(halScreen.CmdLineState&CMDSTATE_OPEN)) return 0;

    if(halScreen.SelStartLine<0) return 0;
    if(halScreen.SelEndLine<0) return 0;

    // COMMIT ANY CHANGES TO THE CURRENT LINE
    if(halScreen.LineIsModified>0) uiModifyLine(0);
    if(Exceptions) return 0;


    BINT selst=rplStringGetLinePtr(CmdLineText,halScreen.SelStartLine)+halScreen.SelStart;
    BINT selend=rplStringGetLinePtr(CmdLineText,halScreen.SelEndLine)+halScreen.SelEnd;
    BINT cursorpos=rplStringGetLinePtr(CmdLineText,halScreen.LineCurrent)+halScreen.CursorPosition;

    if(selend<=selst) return 0;

    uiSetCurrentLine(halScreen.SelStartLine);
    uiMoveCursor(halScreen.SelStart);
    uiRemoveCharacters(utf8nlen((char *)(CmdLineText+1)+selst,(char *)(CmdLineText+1)+selend));

    // NOW MOVE BACK TO THE PREVIOUS CURSOR POSITION
    if(cursorpos>selend) cursorpos-=selend-selst;
    else if(cursorpos>selst) cursorpos=selst;

    uiSetCurrentLine(uiGetLinebyOffset(cursorpos,0));

    uiMoveCursor(cursorpos);

    halScreen.SelEndLine=halScreen.SelStartLine=-1;
    halScreen.SelEnd=halScreen.SelStart=0;

    return 1;

}


// EXPORTED API FUNCTION FROM THE HAL TO LIBRARIES
// STORE ALL COMMAND LINE SETTINGS INTO AN RPL OBJECT
// MUST BE ABLE TO RESTORE THE COMMAND LINE COMPLETELY
// FROM THIS INFORMATION ALONE, AS IF NOTHING HAPPENED

WORDPTR halSaveCmdLine()
{

    WORDPTR *savestk=DSTop;

    // COMMIT ANY CHANGES TO THE CURRENT LINE
    if(halScreen.LineIsModified>0) uiModifyLine(0);
    if(Exceptions) { DSTop=savestk; return 0; }


    // START PUSHING DATA ON THE STACK FOR THE LIST

    rplPushData(CmdLineText);
    rplPushData(CmdLineUndoList);

    // SCREEN PRESENTATION
    rplNewBINTPush(halScreen.CmdLine,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }

    rplNewBINTPush(halScreen.CmdLineState,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }

    rplNewBINTPush(halScreen.LineCurrent,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }

    rplNewBINTPush(halScreen.LineVisible,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }

    rplNewBINTPush(halScreen.NumLinesVisible,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }

    rplNewBINTPush(halScreen.XVisible,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }

    rplNewBINTPush(halScreen.CursorPosition,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }

    // LOCK CURSOR
    halScreen.CursorState|=0x4000;


    rplNewBINTPush(halScreen.CursorState,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }


    // UNLOCK CURSOR
    halScreen.CursorState&=~0xc000;


    // SELECTION
    rplNewBINTPush(halScreen.SelStart,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }

    rplNewBINTPush(halScreen.SelStartLine,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }

    rplNewBINTPush(halScreen.SelEnd,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }

    rplNewBINTPush(halScreen.SelEndLine,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }


    // AUTOCOMPLETE
    if(SuggestedObject) rplPushData(SuggestedObject);
    else rplNewBINTPush(halScreen.ACSuggestion,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }

    rplNewBINTPush(halScreen.ACTokenStart,DECBINT);
    if(Exceptions) { DSTop=savestk; return 0; }


    WORDPTR list=rplCreateListN(16,1,1);
    if(!list || Exceptions) { DSTop=savestk; return 0; }


    return list;

}

// EXPORTED HAL API FOR LIBRARIES, CAN FULLY RESTORE A COMMAND LINE STATE
// FROM A LIST OF DATA GENERATED BY halSaveCmdLine()

BINT halRestoreCmdLine(WORDPTR data)
{
    if(!data) return 0;
    if(!ISLIST(*data)) return 0;


    if(halScreen.CmdLineState&CMDSTATE_OPEN) {
        // THERE'S AN EXISTING COMMAND LINE, CLOSE IT FIRST
        uiCloseCmdLine();
    }

//  NOW RESTORE FROM SCRATCH

    WORDPTR ptr=rplGetListElement(data,1);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    CmdLineText=ptr;

    ptr=rplGetListElement(data,2);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    CmdLineUndoList=ptr;


    // SCREEN PRESENTATION
    ptr=rplGetListElement(data,3);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    halSetCmdLineHeight(rplReadNumberAsBINT(ptr));

    ptr=rplGetListElement(data,4);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    halScreen.CmdLineState=rplReadNumberAsBINT(ptr);

    ptr=rplGetListElement(data,5);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    halScreen.LineCurrent=rplReadNumberAsBINT(ptr);

    ptr=rplGetListElement(data,6);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    halScreen.LineVisible=rplReadNumberAsBINT(ptr);

    ptr=rplGetListElement(data,7);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    halScreen.NumLinesVisible=rplReadNumberAsBINT(ptr);

    ptr=rplGetListElement(data,8);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    halScreen.XVisible=rplReadNumberAsBINT(ptr);

    ptr=rplGetListElement(data,9);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    halScreen.CursorPosition=rplReadNumberAsBINT(ptr);

    ptr=rplGetListElement(data,10);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    halScreen.CursorState=rplReadNumberAsBINT(ptr);

    // SELECTION
    ptr=rplGetListElement(data,11);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    halScreen.SelStart=rplReadNumberAsBINT(ptr);

    ptr=rplGetListElement(data,12);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    halScreen.SelStartLine=rplReadNumberAsBINT(ptr);

    ptr=rplGetListElement(data,13);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    halScreen.SelEnd=rplReadNumberAsBINT(ptr);

    ptr=rplGetListElement(data,14);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    halScreen.SelEndLine=rplReadNumberAsBINT(ptr);

    // AUTOCOMPLETE
    ptr=rplGetListElement(data,15);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    if(ISNUMBER(*ptr)) { halScreen.ACSuggestion=rplReadNumberAsBINT(ptr); SuggestedObject=0; }
    else { SuggestedObject=ptr; halScreen.ACSuggestion=0; }


    ptr=rplGetListElement(data,16);
    if(!ptr) { uiCloseCmdLine(); return 0;  }
    halScreen.ACTokenStart=rplReadNumberAsBINT(ptr);


    halScreen.LineIsModified=-1;    // LINE IS EXISTING BUT NEEDS TO BE EXTRACTED



    // START THE TIMER
    halScreen.CursorTimer=tmr_eventcreate(&__uicursorupdate,700,1);

    // UNLOCK CURSOR
    halScreen.CursorState&=~0xc000;

    uiMoveCursor(halScreen.CursorPosition);

    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;

    return 1;

}


void uiOpenAndInsertTextN(BYTEPTR start,BYTEPTR end)
{
if(!(halGetContext()&CONTEXT_INEDITOR)) {

    ScratchPointer1=(WORDPTR)start;
    halSetCmdLineHeight((*halScreen.FontArray[FONT_CMDLINE])->BitmapHeight+2);
    halSetContext(halGetContext()|CONTEXT_INEDITOR);
    uiOpenCmdLine('D');
    end=((BYTEPTR)ScratchPointer1)+(end-start);
    start=(BYTEPTR)ScratchPointer1;
    }

    uiInsertCharactersN(start,end);
    uiAutocompleteUpdate();
    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;
}
