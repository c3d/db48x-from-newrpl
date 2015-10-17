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
void uiSetCmdLineText(WORDPTR text)
{
    CmdLineText=text;
    CmdLineCurrentLine=(WORDPTR)empty_string;
    CmdLineUndoList=(WORDPTR)empty_list;
    halScreen.LineIsModified=-1;


    // SET CURSOR AT END OF TEXT
    BINT end=rplStrSize(CmdLineText);
    BINT linestoff;
    BYTEPTR linestart;
    halScreen.LineCurrent=uiGetLinebyOffset(end,&linestoff);
    linestart=((BYTEPTR)(CmdLineText+1))+linestoff;
    halScreen.CursorPosition=((BYTEPTR)(CmdLineText+1))+end-linestart;

    if(halScreen.CursorPosition<0) halScreen.CursorPosition=0;
    halScreen.CursorX=StringWidthN((char *)linestart,(char *)linestart+halScreen.CursorPosition,(UNIFONT *)halScreen.CmdLineFont);



    uiExtractLine(halScreen.LineCurrent);

    if(Exceptions) {
        throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        CmdLineText=(WORDPTR)empty_string;
        CmdLineCurrentLine=(WORDPTR)empty_string;
        CmdLineUndoList=(WORDPTR)empty_list;
        return;
    }


    uiEnsureCursorVisible();

    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;
}

WORDPTR uiGetCmdLineText()
{
    if(halScreen.LineIsModified>0) uiModifyLine();
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
    if(halScreen.CursorX>halScreen.XVisible+SCREEN_WIDTH-8) {
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
    halScreen.CmdLineState=0;
    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;

}


void uiSetCurrentLine(BINT line)
{

    if(line==halScreen.LineCurrent) return;
    // LOCK CURSOR
    halScreen.CursorState|=0x4000;

    if(halScreen.LineIsModified>0) {
        // INSERT THE MODIFIED TEXT BACK INTO ORIGINAL TEXT

        uiModifyLine();

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            CmdLineText=(WORDPTR)empty_string;
            CmdLineCurrentLine=(WORDPTR)empty_string;
            CmdLineUndoList=(WORDPTR)empty_list;
            return;
        }

    }

    halScreen.LineCurrent=line;
        uiExtractLine(halScreen.LineCurrent);

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            CmdLineText=(WORDPTR)empty_string;
            CmdLineCurrentLine=(WORDPTR)empty_string;
            CmdLineUndoList=(WORDPTR)empty_list;
            return;
        }

    // POSITION THE CURSOR IN THE NEW LINE, TRYING TO PRESERVE THE X COORDINATE

    BINT len=rplStrSize(CmdLineCurrentLine);
    BINT targetx=halScreen.CursorX;
    BYTEPTR ptr=(BYTEPTR)(CmdLineCurrentLine+1);
    BYTEPTR ptr2=(BYTEPTR)StringCoordToPointer((char *)ptr,(char *)ptr+len,halScreen.CmdLineFont,&targetx);

    halScreen.CursorX=targetx;
    halScreen.CursorPosition=ptr2-ptr;

    uiEnsureCursorVisible();

    // UNLOCK CURSOR
    halScreen.CursorState&=~0xc000;

    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY;


}


// MAIN FUNCTION TO INSERT TEXT AT THE CURRENT CURSOR OFFSET
void uiInsertCharacters(BYTEPTR string)
{
BYTEPTR end=string+stringlen((char *)string);

uiInsertCharactersN(string,end);
}

void uiInsertCharactersN(BYTEPTR string,BYTEPTR endstring)
{
if(endstring<=string) return;

// LOCK CURSOR
halScreen.CursorState|=0x4000;

if(halScreen.LineIsModified<0) {

uiExtractLine(halScreen.LineCurrent);

if(Exceptions) {
    throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
    // CLEAN UP AND RETURN
    CmdLineText=(WORDPTR)empty_string;
    CmdLineCurrentLine=(WORDPTR)empty_string;
    CmdLineUndoList=(WORDPTR)empty_list;
    return;
}

}

BINT length=endstring-string;
BINT lenwords=(length+3)>>2;

if(CmdLineCurrentLine==(WORDPTR)empty_string) {
    WORDPTR newobj=rplAllocTempOb(lenwords);
    if(!newobj) {
        throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        CmdLineText=(WORDPTR)empty_string;
        CmdLineCurrentLine=(WORDPTR)empty_string;
        CmdLineUndoList=(WORDPTR)empty_list;
        return;
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

        WORDPTR newobj=rplAllocTempOb(lenwords);
        if(!newobj) {
            throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            CmdLineText=(WORDPTR)empty_string;
            CmdLineCurrentLine=(WORDPTR)empty_string;
            CmdLineUndoList=(WORDPTR)empty_list;
            return;
        }
        rplCopyObject(newobj,CmdLineCurrentLine);
        CmdLineCurrentLine=newobj;

    }
}
if(Exceptions) {
    throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
    // CLEAN UP AND RETURN
    CmdLineText=(WORDPTR)empty_string;
    CmdLineCurrentLine=(WORDPTR)empty_string;
    CmdLineUndoList=(WORDPTR)empty_list;
    return;
}

// FINALLY, WE HAVE THE ORIGINAL LINE AT THE END OF TEMPOB, AND ENOUGH MEMORY ALLOCATED TO MAKE THE MOVE

// MOVE THE TAIL TO THE END
memmove( ((BYTEPTR)CmdLineCurrentLine)+4+halScreen.CursorPosition+length,((BYTEPTR)CmdLineCurrentLine)+4+halScreen.CursorPosition,rplStrSize(CmdLineCurrentLine)-halScreen.CursorPosition);
// ADD THE NEW DATA IN
memmove(((BYTEPTR)CmdLineCurrentLine)+4+halScreen.CursorPosition,string,length);

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
    BINT newoff=rplStringGetLinePtr(CmdLineText,halScreen.LineCurrent)+halScreen.CursorPosition+length;

    uiSetCurrentLine(halScreen.LineCurrent+nl);
    uiMoveCursor(newoff);

}
else {
halScreen.CursorX+=StringWidthN(((char *)CmdLineCurrentLine)+4+halScreen.CursorPosition,((char *)CmdLineCurrentLine)+4+halScreen.CursorPosition+length,halScreen.CmdLineFont);
halScreen.CursorPosition+=length;
}

halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY;

uiEnsureCursorVisible();
// UNLOCK CURSOR
halScreen.CursorState&=~0xc000;

}


// MAIN FUNCTION TO REMOVE TEXT AT THE CURRENT CURSOR OFFSET

void uiRemoveCharacters(BINT length)
{
if(length<=0) return;

// LOCK CURSOR
halScreen.CursorState|=0x4000;


if(halScreen.LineIsModified<0) {

uiExtractLine(halScreen.LineCurrent);

if(Exceptions) {
    throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
    // CLEAN UP AND RETURN
    CmdLineText=(WORDPTR)empty_string;
    CmdLineCurrentLine=(WORDPTR)empty_string;
    CmdLineUndoList=(WORDPTR)empty_list;
    return;
}

}

if(rplSkipOb(CmdLineCurrentLine)!=TempObEnd) {
        // NOT AT THE END OF TEMPOB
        // MAKE A COPY OF THE OBJECT AT THE END
        WORDPTR newobj=rplAllocTempOb(OBJSIZE(*CmdLineCurrentLine));
        if(!newobj) {
            throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            CmdLineText=(WORDPTR)empty_string;
            CmdLineCurrentLine=(WORDPTR)empty_string;
            CmdLineUndoList=(WORDPTR)empty_list;
            return;
        }
        rplCopyObject(newobj,CmdLineCurrentLine);
        CmdLineCurrentLine=newobj;

    }

// FINALLY, WE HAVE THE ORIGINAL LINE AT THE END OF TEMPOB, AND ENOUGH MEMORY ALLOCATED TO MAKE THE MOVE
BINT tailchars=rplStrSize(CmdLineCurrentLine)-halScreen.CursorPosition;
BYTEPTR delete_start=((BYTEPTR)CmdLineCurrentLine)+4+halScreen.CursorPosition;
BYTEPTR delete_end=(BYTEPTR) utf8nskip((char *)delete_start,(char *)delete_start+tailchars,length);


// MOVE THE TAIL TO THE END
memmove( delete_start,delete_end,delete_start+tailchars-delete_end);

// PATCH THE LENGTH OF THE STRING
BINT newlen=rplStrSize(CmdLineCurrentLine);
newlen-=delete_end-delete_start;
rplSetStringLength(CmdLineCurrentLine,newlen);

// TRUNCATE THE OBJECT TO RELEASE MEMORY
rplTruncateLastObject(rplSkipOb(CmdLineCurrentLine));

halScreen.LineIsModified=1;

halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY;

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
        CmdLineText=(WORDPTR)empty_string;
        CmdLineCurrentLine=(WORDPTR)empty_string;
        CmdLineUndoList=(WORDPTR)empty_list;
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
void uiModifyLine()
{
    WORDPTR newobj;
    BINT newsize;

    // GET A NEW OBJECT WITH ROOM FOR THE ENTIRE TEXT
    newobj=rplAllocTempOb( (rplStrSize(CmdLineText)+rplStrSize(CmdLineCurrentLine)+1+ 3)>>2);

    if(Exceptions) {
        throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        CmdLineText=(WORDPTR)empty_string;
        CmdLineCurrentLine=(WORDPTR)empty_string;
        CmdLineUndoList=(WORDPTR)empty_list;
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
        if(*(endline-1)=='\n') --endline;
    }

    // COPY ALL PREVIOUS LINES TO NEW OBJECT
    newsize=startline-src+rplStrSize(CmdLineCurrentLine);

    memmove(dest,src,startline-src);
    // COPY THE NEW LINE TO THE OBJECT
    memmove(dest+(startline-src),(WORDPTR)(CmdLineCurrentLine+1),rplStrSize(CmdLineCurrentLine));
    // COPY THE REST BACK
    if(endline<src+totallen) {
        // APPEND A NEWLINE AND KEEP GOING
        dest+=startline-src+rplStrSize(CmdLineCurrentLine);
        newsize+=src+totallen-endline;
        if(*endline!='\n') { *dest++='\n'; ++newsize; }
        memmove(dest,endline,src+totallen-endline);
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


    newobj=rplAllocTempOb( ((endline-startline)+ 3)>>2);

    if(Exceptions) {
        throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        CmdLineText=(WORDPTR)empty_string;
        CmdLineCurrentLine=(WORDPTR)empty_string;
        CmdLineUndoList=(WORDPTR)empty_list;
        return;
    }

    // ZERO PADDING THE LAST WORD
    newobj[(endline-startline)>>2]=0;

    // COPY LINE TO NEW OBJECT
    memmove(newobj+1,startline,endline-startline);

    rplSetStringLength(newobj,endline-startline);

    CmdLineCurrentLine=newobj;

    halScreen.LineIsModified=0;

}




// MOVE THE CURSOR TO THE GIVEN OFFSET WITHIN THE STRING
void uiMoveCursor(BINT offset)
{
if(halScreen.LineIsModified<0) {
    uiExtractLine(halScreen.LineCurrent);

    if(Exceptions) {
        throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        CmdLineText=(WORDPTR)empty_string;
        CmdLineCurrentLine=(WORDPTR)empty_string;
        CmdLineUndoList=(WORDPTR)empty_list;
        return;
    }

    }

BYTEPTR ptr=(BYTEPTR )(CmdLineCurrentLine+1),ptr2;
BINT len=rplStrSize(CmdLineCurrentLine);

if(offset>len) offset=len;
if(offset<0) offset=0;

if(halScreen.CursorPosition==offset) return;

halScreen.CursorState|=0x4000;

// AVOID USING OFFSET THAT FALLS BETWEEN BYTES OF THE SAME CODEPOINT
ptr2=ptr;
while((ptr2-ptr<offset)&&(ptr2<ptr+len)) ptr2=(BYTEPTR)utf8skip((char *)ptr2,(char *)ptr+len);

offset=ptr2-ptr;

halScreen.CursorPosition=offset;

halScreen.CursorX=StringWidthN((char *)ptr,(char *)ptr2,halScreen.CmdLineFont);

halScreen.CursorState&=~0xc000;

halScreen.DirtyFlag|=CMDLINE_LINEDIRTY|CMDLINE_CURSORDIRTY;
}


// MOVE THE CURSOR LEFT, NCHARS IS GIVEN IN UNICODE CODEPOINTS
void uiCursorLeft(BINT nchars)
{
    BYTEPTR ptr,ptr2,newptr;
    BINT offset;



    if(halScreen.LineIsModified<0) {
        uiExtractLine(halScreen.LineCurrent);

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            CmdLineText=(WORDPTR)empty_string;
            CmdLineCurrentLine=(WORDPTR)empty_string;
            CmdLineUndoList=(WORDPTR)empty_list;
            return;
        }

        }

    ptr=(BYTEPTR )(CmdLineCurrentLine+1);
    // AVOID USING OFFSET THAT FALLS BETWEEN BYTES OF THE SAME CODEPOINT
    ptr2=ptr+halScreen.CursorPosition;

    while(nchars &&(ptr2>ptr)) {
        ptr2=(BYTEPTR)utf8rskip((char *)ptr2,(char *)ptr);
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

                uiModifyLine();

                if(Exceptions) {
                    throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
                    // CLEAN UP AND RETURN
                    CmdLineText=(WORDPTR)empty_string;
                    CmdLineCurrentLine=(WORDPTR)empty_string;
                    CmdLineUndoList=(WORDPTR)empty_list;
                    return;
                }

            }

            // RELOAD VALUES FOR THE WHOLE TEXT
            offset=rplStringGetLinePtr(CmdLineText,halScreen.LineCurrent);
            if(offset<0) offset=rplStrSize(CmdLineText);
            ptr=(BYTEPTR )(CmdLineText+1);
            ptr2=ptr+offset;    // THIS IS THE BEGINNING OF THE CURRENT LINE
            while(nchars &&(ptr2>ptr)) {
                ptr2=(BYTEPTR)utf8rskip((char *)ptr2,(char *)ptr);
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
                CmdLineText=(WORDPTR)empty_string;
                CmdLineCurrentLine=(WORDPTR)empty_string;
                CmdLineUndoList=(WORDPTR)empty_list;
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

    halScreen.CursorX=StringWidthN((char *)ptr,(char *)ptr2,halScreen.CmdLineFont);

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
            CmdLineText=(WORDPTR)empty_string;
            CmdLineCurrentLine=(WORDPTR)empty_string;
            CmdLineUndoList=(WORDPTR)empty_list;
            return;
        }

        }

    BYTEPTR ptr=(BYTEPTR )(CmdLineCurrentLine+1),ptr2;
    BINT len=rplStrSize(CmdLineCurrentLine);
    BINT offset;

    // AVOID USING OFFSET THAT FALLS BETWEEN BYTES OF THE SAME CODEPOINT
    ptr2=ptr+halScreen.CursorPosition;
    while(nchars &&(ptr2<ptr+len)) { ptr2=(BYTEPTR)utf8skip((char *)ptr2,(char *)ptr+len); --nchars; }

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

                uiModifyLine();

                if(Exceptions) {
                    throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
                    // CLEAN UP AND RETURN
                    CmdLineText=(WORDPTR)empty_string;
                    CmdLineCurrentLine=(WORDPTR)empty_string;
                    CmdLineUndoList=(WORDPTR)empty_list;
                    return;
                }

            }

            // RELOAD VALUES FOR THE WHOLE TEXT
            offset=rplStringGetLinePtr(CmdLineText,halScreen.LineCurrent);
            if(offset<0) offset=rplStrSize(CmdLineText);
            ptr=(BYTEPTR )(CmdLineText+1);
            len=rplStrSize(CmdLineText);
            ptr2=uiGetEndOfLine(ptr+offset,ptr+len);    // THIS IS THE END OF THE CURRENT LINE
            while(nchars &&(ptr2<ptr+len)) {
                if(*ptr2=='\n') ++halScreen.LineCurrent;
                ptr2=(BYTEPTR)utf8skip((char *)ptr2,(char *)ptr+len);
                --nchars;
            }

            if(nchars) nchars=0;    // NO MORE TEXT!
            if(halScreen.LineCurrent>totallines) halScreen.LineCurrent=totallines;

            offset=ptr2-uiGetStartOfLine(ptr2,ptr);

            uiExtractLine(halScreen.LineCurrent);
            if(Exceptions) {
                throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
                // CLEAN UP AND RETURN
                CmdLineText=(WORDPTR)empty_string;
                CmdLineCurrentLine=(WORDPTR)empty_string;
                CmdLineUndoList=(WORDPTR)empty_list;
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

    halScreen.CursorX=StringWidthN((char *)ptr,(char *)ptr2,halScreen.CmdLineFont);

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
        CmdLineText=(WORDPTR)empty_string;
        CmdLineCurrentLine=(WORDPTR)empty_string;
        CmdLineUndoList=(WORDPTR)empty_list;
        return;
    }
    }

// POSITION THE CURSOR TRYING TO PRESERVE THE X COORDINATE

BINT len=rplStrSize(CmdLineCurrentLine);
BINT targetx=halScreen.CursorX+SCREEN_WIDTH;
BYTEPTR ptr=(BYTEPTR)(CmdLineCurrentLine+1);
BYTEPTR ptr2=StringCoordToPointer((char *)ptr,(char *)ptr+len,halScreen.CmdLineFont,&targetx);

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
        CmdLineText=(WORDPTR)empty_string;
        CmdLineCurrentLine=(WORDPTR)empty_string;
        CmdLineUndoList=(WORDPTR)empty_list;
        return;
    }
    }

// POSITION THE CURSOR TRYING TO PRESERVE THE X COORDINATE

BINT len=rplStrSize(CmdLineCurrentLine);
BINT targetx=halScreen.CursorX-SCREEN_WIDTH;
BYTEPTR ptr=(BYTEPTR)(CmdLineCurrentLine+1);
BYTEPTR ptr2=StringCoordToPointer((char *)ptr,(char *)ptr+len,halScreen.CmdLineFont,&targetx);

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
            CmdLineText=(WORDPTR)empty_string;
            CmdLineCurrentLine=(WORDPTR)empty_string;
            CmdLineUndoList=(WORDPTR)empty_list;
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

    halScreen.NumLinesVisible+=addition;
    if(halScreen.NumLinesVisible<1) halScreen.NumLinesVisible=1;

    halSetCmdLineHeight(halScreen.CmdLineFont->BitmapHeight*halScreen.NumLinesVisible+2);
    if(halScreen.CmdLine!=halScreen.CmdLineFont->BitmapHeight*halScreen.NumLinesVisible+2) {
        // NO ROOM, ADJUST NUMBER OF VISIBLE LINES
        BINT actuallines=(halScreen.CmdLine-2)/halScreen.CmdLineFont->BitmapHeight;
        halScreen.NumLinesVisible=actuallines;
        if(halScreen.NumLinesVisible<1) halScreen.NumLinesVisible=1;
    }



}
