#include <ui.h>
#include <newrpl.h>
#include <libraries.h>


// COMMAND LINE API
// BASIC PRINCIPLES OF THE COMMAND LINE:
// * ONLY ONE LINE IS EDITED AT ONCE
// * LINE IS EDITED AS THE LAST OBJECT IN TEMPOB, TO BE STRETCHED AT WILL
// * IF LINE IS NOT AT THE END OF TEMPOB, A NEW COPY IS MADE

BINT ui_visibleline,ui_nlines;
BINT ui_currentline;
BINT ui_islinemodified;
BINT ui_cursorx,ui_cursoroffset;
BINT ui_visiblex;

// SET THE COMMAND LINE TO A GIVEN STRING OBJECT
void uiSetCmdLineText(WORDPTR text)
{
    CmdLineText=text;
    CmdLineCurrentLine=empty_string;
    CmdLineUndoList=empty_list;
    ui_islinemodified=-1;

    // SET CURSOR AT END OF TEXT
    BINT end=rplStrLen(CmdLineText);
    BYTEPTR linestart;
    ui_currentline=uiGetLinebyOffset(end,&linestart);
    ui_cursoroffset=((BYTEPTR)rplSkipOb(CmdLineText))-linestart;

    if(ui_cursoroffset<0) ui_cursoroffset=0;
    ui_cursorx=StringWidthN(linestart,ui_cursoroffset,(FONTDATA *)halScreen.CmdLineFont);


    uiEnsureCursorVisible();
}

// SCROLL UP/DOWN AND LEFT/RIGHT TO KEEP CURSOR ON SCREEN
void uiEnsureCursorVisible()
{
    // TODO: DO THE SCROLL

}


// OPEN AN EMPTY COMMAND LINE
void uiOpenCmdLine()
{
    CmdLineText=empty_string;
    CmdLineCurrentLine=empty_string;
    CmdLineUndoList=empty_list;
    ui_currentline=1;
    ui_islinemodified=-1;
    ui_visibleline=1;
    ui_cursorx=0;
    ui_cursoroffset=0;
    ui_visiblex=0;
}

void uiSetCurrentLine(BINT line)
{
    if(line==ui_currentline) return;

    if(ui_islinemodified>0) {
        // INSERT THE MODIFIED TEXT BACK INTO ORIGINAL TEXT

        uiModifyLine();

        if(Exceptions) {
            throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            CmdLineText=empty_string;
            CmdLineCurrentLine=empty_string;
            CmdLineUndoList=empty_list;
            return;
        }

    }

    ui_currentline=line;
    ui_islinemodified=-1;

    uiUpdateCursor();

}


// MAIN FUNCTION TO INSERT TEXT AT THE CURRENT CURSOR OFFSET

void uiInsertCharacters(BYTEPTR string,BINT length)
{
if(length<=0) return;

if(ui_islinemodified<0) {

uiExtractLine(ui_currentline);

if(Exceptions) {
    throw_dbgexception("No memory for command line",__EX_CONT|__EX_WARM|__EX_RESET);
    // CLEAN UP AND RETURN
    CmdLineText=empty_string;
    CmdLineCurrentLine=empty_string;
    CmdLineUndoList=empty_list;
    return;
}

}

BINT lenwords=(length+3)>>2;

if(CmdLineCurrentLine==empty_string) {
    WORDPTR newobj=rplAllocTempOb(lenwords);
    if(!newobj) {
        throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        CmdLineText=empty_string;
        CmdLineCurrentLine=empty_string;
        CmdLineUndoList=empty_list;
        return;
    }
    CmdLineCurrentLine=newobj;
    *CmdLineCurrentLine=MKPROLOG(DOSTRING,0);   // MAKE AN EMPTY STRING
}
else {
    if(rplSkipOb(CmdLineCurrentLine)==TempObEnd)  rplResizeLastObject(lenwords);
    else {
        // NOT AT THE END OF TEMPOB
        // MAKE A COPY OF THE OBJECT AT THE END
        WORDPTR newobj=rplAllocTempOb(OBJSIZE(*CmdLineCurrentLine));
        if(!newobj) {
            throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
            // CLEAN UP AND RETURN
            CmdLineText=empty_string;
            CmdLineCurrentLine=empty_string;
            CmdLineUndoList=empty_list;
            return;
        }
        rplCopyObject(newobj,CmdLineCurrentLine);
        CmdLineCurrentLine=newobj;

    }
}
if(Exceptions) {
    throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
    // CLEAN UP AND RETURN
    CmdLineText=empty_string;
    CmdLineCurrentLine=empty_string;
    CmdLineUndoList=empty_list;
    return;
}

// FINALLY, WE HAVE THE ORIGINAL LINE AT THE END OF TEMPOB, AND ENOUGH MEMORY ALLOCATED TO MAKE THE MOVE

// MOVE THE TAIL TO THE END
memmove( ((BYTEPTR)CmdLineCurrentLine)+4+ui_cursoroffset+length,((BYTEPTR)CmdLineCurrentLine)+4+ui_cursoroffset,rplStrLen(CmdLineCurrentLine)-ui_cursoroffset);
// ADD THE NEW DATA IN
memmove(((BYTEPTR)CmdLineCurrentLine)+4+ui_cursoroffset,string,length);

// PATCH THE LENGTH OF THE STRING
BINT newlen=rplStrLen(CmdLineCurrentLine);
newlen+=length;
rplSetStringLength(CmdLineCurrentLine,newlen);

ui_islinemodified=1;

// ADVANCE THE CURSOR
ui_cursoroffset+=length;

// IF THE INSERTED TEXT HAD ANY NEWLINES, THE CURRENT COMMAND LINE HAS MULTIPLE LINES IN ONE
// MUST SPLIT THE LINES AND GET THE CURSOR ON THE LAST ONE

// TODO: GET THIS DONE


}


// COPY THE EDITED LINE BACK INTO THE ORIGINAL TEXT
// CREATES A COPY OF THE TEXT
void uiModifyLine()
{
    WORDPTR newobj;

    // GET A NEW OBJECT WITH ROOM FOR THE ENTIRE TEXT
    newobj=rplAllocTempOb( (rplStrLen(CmdLineText)+rplStrLen(CmdLineCurrentLine)+1+ 3)>>2);

    if(Exceptions) {
        throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        CmdLineText=empty_string;
        CmdLineCurrentLine=empty_string;
        CmdLineUndoList=empty_list;
        return;
    }
    BYTEPTR src=(BYTEPTR)(CmdLineText+1),dest=(BYTEPTR)(newobj+1);
    BYTEPTR startline=src+rplStringGetLinePtr(CmdLineText,ui_currentline);
    BYTEPTR endline=src+rplStringGetLinePtr(CmdLineText,ui_currentline+1);

    if(startline<src) {
        /*
        throw_dbgexception("Bad starting line",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        CmdLineText=empty_string;
        CmdLineCurrentLine=empty_string;
        CmdLineUndoList=empty_list;
        return;
        */
        // LINE DOESN'T EXIST, ADD AT THE END OF TEXT AS A NEW LINE
        startline=src;
    }

    // COPY ALL PREVIOUS LINES TO NEW OBJECT
    memmove(dest,src,startline-src);
    // COPY THE NEW LINE TO THE OBJECT
    memmove(dest+(startline-src),(WORDPTR)(CmdLineCurrentLine+1),rplStrLen(CmdLineCurrentLine));
    // COPY THE REST BACK
    if(endline>=src) {
        // APPEND A NEWLINE AND KEEP GOING
        dest+=startline-src+rplStrLen(CmdLineCurrentLine);
        *dest++='\n';
        memmove(dest,endline,((BYTEPTR)rplSkipOb(CmdLineText))-endline);
    }

    CmdLineText=newobj;
    ui_islinemodified=0;

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

    if(endline<text) endline=text+rplStrLen(CmdLineText);


    newobj=rplAllocTempOb( ((endline-startline)+ 3)>>2);

    if(Exceptions) {
        throw_dbgexception("No memory to insert text",__EX_CONT|__EX_WARM|__EX_RESET);
        // CLEAN UP AND RETURN
        CmdLineText=empty_string;
        CmdLineCurrentLine=empty_string;
        CmdLineUndoList=empty_list;
        return;
    }

    // ZERO PADDING THE LAST WORD
    newobj[(endline-startline)>>2]=0;

    // COPY LINE TO NEW OBJECT
    memmove(newobj+1,startline,endline-startline);

    rplSetStringLength(newobj,endline-startline);

    CmdLineCurrentLine=newobj;

    ui_islinemodified=0;

}


// RETURN LINE NUMBER FOR A GIVEN OFFSET, AND THE OFFSET OF THE START OF LINE
BINT uiGetLinebyOffset(BINT offset,BINT *linestart)
{

    BYTEPTR ptr=(BYTEPTR)(CmdLineText+1);
    BYTEPTR end=rplSkipOb(CmdLineText);
    BINT len=rplStrLen(CmdLineText);
    BINT f,found,count;
    BYTEPTR *prevfind,*currfind;

    if(offset>len) offset=len;
    if(offset<0) offset=0;
    prevfind=currfind=ptr;
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

// MOVE THE CURSOR TO THE GIVEN OFFSET WITHIN THE STRING
void uiMoveCursor(BINT offset)
{
if(ui_islinemodified>0) uiModifyLine();

BYTEPTR ptr=(BYTEPTR )(CmdLineText+1);
BINT len=rplStrLen(CmdLineText);
BINT lineoff;

if(offset>len) offset=len;
if(offset<0) offset=0;

BINT line=uiGetLinebyOffset(offset,&lineoff);

uiSetCurrentLine(line);

ui_cursoroffset=offset;

ui_cursorx=StringWidthN(ptr,offset-lineoff,halScreen.CmdLineFont);
}



void uiUpdateCursor()
{
    // TODO: NOT YET IMPLEMENTED


}
