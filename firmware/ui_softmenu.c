/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <newrpl.h>
#include <ui.h>
#include <libraries.h>

// SOFT MENU API



// RETURN THE NUMBER OF ITEMS IN A MENU

BINT uiCountMenuItems(WORD MenuCode,WORDPTR menu)
{
if(MENUSPECIAL(MenuCode)==1) {
    // MENU IS VARS
    return rplGetVisibleVarCount();
}
if(!menu) return 0;
if(ISLIST(*menu)) return rplListLength(menu);
return 1;
}


// GET A MENU OBJECT FROM A MENU CODE

WORDPTR uiGetLibMenu(WORD MenuCode)
{
    if(MENUSPECIAL(MenuCode)==1) {
        // MENU IS VARS, NO NEED FOR MENU OBJECT
        return 0;
    }

    BINT libnum=MENULIBRARY(MenuCode);
    LIBHANDLER han=rplGetLibHandler(libnum);
    if(!han) return 0;
    WORD SavedOpcode=CurOpcode;
    BINT SavedException=Exceptions;
    BINT SavedErrorCode=ErrorCode;

    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE LIBRARY TO RUN
    CurOpcode=MKOPCODE(libnum,OPCODE_LIBMENU);
    MenuCodeArg=MenuCode;
    RetNum=-1;
    (*han)();

    Exceptions=SavedException;
    ErrorCode=SavedErrorCode;
    CurOpcode=SavedOpcode;

    if(RetNum!=OK_CONTINUE) return 0;

    return ObjectPTR;

}



// RETURN A POINTER TO A MENU ITEM OBJECT
// FIRST ITEM = NUMBER 0

WORDPTR uiGetMenuItem(WORD MenuCode,WORDPTR menu,BINT item)
{
if(MENUSPECIAL(MenuCode)==1) {
 // MENU IS VARS
 // RETURN A POINTER TO THE VARIABLE NAME

    WORDPTR *var=rplFindVisibleGlobalByIndex(item);
    if(!var) return 0;
    return var[0];
}

    if(!menu) return 0;

    if(!ISLIST(*menu)) {
        if(item!=0) return 0;
        return menu;
    }

    WORDPTR ptr=menu+1,end=rplSkipOb(menu);


    while(ptr<end-1) {
        if(!item) return ptr;
        ptr=rplSkipOb(ptr);
        --item;
    }
    return 0;
}


// GET THE ACTION OBJECT OF A MENU ITEM

WORDPTR uiGetMenuItemAction(WORDPTR item,BINT shift)
{

    if(!item) return 0;

    if(!ISLIST(*item)) return item;

    // GET ACTION ITEM WITHIN THE ITEM

    item=rplGetListElement(item,2);

    if(!item) return 0;

    if(!ISLIST(*item)) return item;


    int index;
    switch(KM_SHIFTPLANE(shift))
    {
    case 0:
        index=0;
        break;
    case SHIFT_LS:
        index=1;
        break;
    case SHIFT_RS:
        index=2;
        break;
    case SHIFT_LS|SHIFT_LSHOLD:
        index=3;
        break;
    case SHIFT_RS|SHIFT_RSHOLD:
        index=4;
        break;
    default:
        index=5;
    }

    WORDPTR ptr=item+1,end=rplSkipOb(item);


    while(ptr<end-1) {
        if(!index) return ptr;
        ptr=rplSkipOb(ptr);
        --index;
    }


    return 0;

}


// DRAW A SINGLE ITEM IN THE CURRENT CLIPPING BOX
// DOES NOT CLEAR BACKGROUND


void uiDrawMenuItem(WORDPTR item,BINT color,DRAWSURFACE *scr)
{
    WORDPTR ptr;
    BINT flags=0;
    if(!item) return;
    if(ISLIST(*item)) {
        ptr=item+1;
        if(ptr>=rplSkipOb(item)-1) return;

        if(*ptr==CMD_ENDLIST) ptr=item;
        else {
        // IF IT'S A PROGRAM, RUN IT AND TAKE THE RESULT FROM THE STACK
        if(ISPROGRAM(*ptr)) {
            rplPushData(item);
            rplPushData(ptr);

            BINT nresults=cmdRunTransparent(CMD_XEQ,1,1);

            if(nresults==1) ptr=rplPopData();
            else ptr=empty_string;      // IF THE PROGRAM FAILED TO RETURN AN OBJECT, JUST USE THE EMPTY STRING
            item=rplPopData();  // RESTORE THE item POINTER IN CASE OF GC
            // CONTINUE HERE WITH THE NEW ptr

        }

        //  IF IT'S A LIST, THEN TAKE THE FLAGS FROM THE SECOND ELEMENT IN THE LIST, AND USE THE FIRST AS THE DISPLAY OBJECT
        if(ISLIST(*ptr)) {
            ptr=ptr+1;
            if(ptr>=rplSkipOb(item)-1) return;
            if(*ptr==CMD_ENDLIST) ptr=item;
            else {
                WORDPTR next=rplSkipOb(ptr);
                if(ISBINT(*next)) flags=rplReadBINT(next);
            }
        }

        }
    } else ptr=item;


    // HERE ptr HAS AN OBJECT TO DISPLAY

    // AND flags HAS THE FLAGS



    if(ISIDENT(*ptr)) {

        // SPECIAL CASE: FOR IDENTS LOOK FOR VARIABLES AND DRAW DIFFERENTLY IF IT'S A DIRECTORY
        WORDPTR *var=rplFindGlobal(ptr,1);

        BINT w=StringWidthN((char *)(ptr+1),(char *)(ptr+1)+rplGetIdentLength(ptr),halScreen.MenuFont),pos;

        if(w>=scr->clipx2-scr->clipx) pos=scr->clipx+1;
        else pos=(scr->clipx2+scr->clipx-w)>>1;

        if((flags&1) || (var && ISDIR(*var[1]))) {
            //ggl_clipvline(scr,scr->clipx2,scr->clipy,scr->clipy2,ggl_mkcolor(color));
            ggl_cliphline(scr,scr->clipy2,scr->clipx,scr->clipx2,ggl_mkcolor(color));
        }

        DrawTextN(pos,scr->clipy,(char *)(ptr+1),(char *)(ptr+1)+rplGetIdentLength(ptr),halScreen.MenuFont,color,scr);

        // DARKEN/LIGHTEN EFFECT ON LAST FEW PIXELS
        if(w>=scr->clipx2-scr->clipx) {
            scr->x=scr->clipx2;
            scr->y=scr->clipy;
            ggl_filter(scr,1,scr->clipy2-scr->clipy+1,0xA,(color)? &ggl_fltlighten:&ggl_fltdarken);
            scr->x--;
            ggl_filter(scr,1,scr->clipy2-scr->clipy+1,0x6,(color)? &ggl_fltlighten:&ggl_fltdarken);
            scr->x--;
            ggl_filter(scr,1,scr->clipy2-scr->clipy+1,0x4,(color)? &ggl_fltlighten:&ggl_fltdarken);

        }

                if(flags&2) {
                   // SECOND BIT IN FLAGS MEANS INVERTED
                            scr->x=scr->clipx;
                            scr->y=scr->clipy;
                            ggl_filter(scr,scr->clipx2-scr->clipx+1,scr->clipy2-scr->clipy+1,0,&ggl_fltinvert);
                }

        return;
    }

    // ALL OTHER OBJECTS NEED TO BE DECOMPILED, EXCEPT THE STRING AND GROBS

    BINT skipn,totaln;
    BYTEPTR string,endstring;


    // TODO: ADD GROBS HERE

    if(!ISSTRING(*ptr)) {

    WORD ptrprolog=*ptr;

    BINT SavedException=Exceptions;
    BINT SavedErrorCode=ErrorCode;

    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
    WORDPTR opname=rplDecompile(ptr,0);
    Exceptions=SavedException;
    ErrorCode=SavedErrorCode;

    if(!opname) return;

    // HERE WE HAVE A STRING, DO SOME MORE POST-PROCESSING DEPENDING ON OBJECT

    string=(BYTEPTR) (opname+1);
    skipn=0;
    totaln=rplStrLen(opname);
    endstring=utf8nskip(string,rplSkipOb(opname),totaln);

    if(ISUNIT(ptrprolog)) {
        // TODO: SKIP THE NUMERIC PORTION, LEAVE JUST THE UNIT
        if((totaln>2)&&(string[0]=='1')&&(string[1]=='_')) {
            totaln-=2;
            string+=2;
        }

    }

    // TODO: ADD MORE SPECIALIZED HANDLING HERE

    }
    else {
        string=(BYTEPTR) (ptr+1);
        skipn=0;
        totaln=rplStrLen(ptr);
        endstring=utf8nskip(string,rplSkipOb(ptr),totaln);
    }

    // JUST DISPLAY THE STRING

    BINT w=StringWidthN((char *)string,(char *)endstring,halScreen.MenuFont),pos;
    if(w>=scr->clipx2-scr->clipx) pos=scr->clipx+1;
    else pos=(scr->clipx2+scr->clipx-w)>>1;

    if(flags&1) {   // FOR NOW, flags & 1 INDICATES THE MENU IS TO BE DISPLAYED AS A DIRECTORY
        //ggl_clipvline(scr,scr->clipx2,scr->clipy,scr->clipy2,ggl_mkcolor(color));
        ggl_cliphline(scr,scr->clipy2,scr->clipx,scr->clipx2,ggl_mkcolor(color));
    }


    DrawTextN(pos,scr->clipy,(char *)string,(char *)endstring,halScreen.MenuFont,color,scr);

    // DARKEN/LIGHTEN EFFECT ON LAST FEW PIXELS
    if(w>=scr->clipx2-scr->clipx) {
        scr->x=scr->clipx2;
        scr->y=scr->clipy;
        ggl_filter(scr,1,scr->clipy2-scr->clipy+1,0xA,(color)? &ggl_fltlighten:&ggl_fltdarken);
        scr->x--;
        ggl_filter(scr,1,scr->clipy2-scr->clipy+1,0x6,(color)? &ggl_fltlighten:&ggl_fltdarken);
        scr->x--;
        ggl_filter(scr,1,scr->clipy2-scr->clipy+1,0x4,(color)? &ggl_fltlighten:&ggl_fltdarken);

    }

    if(flags&2) {
       // SECOND BIT IN FLAGS MEANS INVERTED
                scr->x=scr->clipx;
                scr->y=scr->clipy;
                ggl_filter(scr,scr->clipx2-scr->clipx+1,scr->clipy2-scr->clipy+1,0,&ggl_fltinvert);
    }

    return;

}
