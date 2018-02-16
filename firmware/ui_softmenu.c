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
if(MENUSPECIAL(MenuCode)==MENU_VARS) {
    // MENU IS VARS
    return rplGetVisibleVarCount();
}
if(MENUSPECIAL(MenuCode)==MENU_USERLIB) {
    // MENU IS LIBS
    WORDPTR libdir=rplGetSettings((WORDPTR)library_dirname);

    if(!libdir) return 0;

    WORDPTR *direntry=rplFindDirbyHandle(libdir);

    if(!direntry) return 0;

    return rplGetVisibleVarCountInDir(direntry);
}

if(!menu) return 0;
if(ISLIST(*menu)) return rplListLength(menu);
return 1;
}

WORDPTR uiGetLibObject(BINT libnum,WORD arg2,WORD arg3,WORD Opcode)
{
    LIBHANDLER han=rplGetLibHandler(libnum);
    if(!han) return 0;
    WORD SavedOpcode=CurOpcode;
    BINT SavedException=Exceptions;
    BINT SavedErrorCode=ErrorCode;

    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE LIBRARY TO RUN
    CurOpcode=MKOPCODE(libnum,Opcode);
    ArgNum2=arg2;
    ArgNum3=arg3;
    RetNum=-1;
    (*han)();

    Exceptions=SavedException;
    ErrorCode=SavedErrorCode;
    CurOpcode=SavedOpcode;

    if(RetNum!=OK_CONTINUE) return 0;

    return ObjectPTR;

}




// GET A MENU OBJECT FROM A MENU CODE

WORDPTR uiGetLibMenu(BINT64 MenuCode)
{
    if(MENUSPECIAL(MenuCode)==MENU_VARS) {
        // MENU IS VARS, NO NEED FOR MENU OBJECT
        return 0;
    }
    if(MENUSPECIAL(MenuCode)==MENU_USERLIB) {
        // MENU IS LIBS, NO NEED FOR MENU OBJECT
        return 0;
    }
    return uiGetLibObject(MENULIBRARY(MenuCode),(MenuCode>>32),MenuCode,OPCODE_LIBMENU);

}


WORDPTR uiGetLibCmdHelp(WORD Command)
{
   return uiGetLibObject(LIBNUM(Command),0,Command,OPCODE_LIBHELP);
}

WORDPTR uiGetLibPtrHelp(WORDPTR LibCommand)
{

   return uiGetLibObject(LIBNUM(*LibCommand),LibCommand[1],MKOPCODE(DOLIBPTR,LibCommand[2]),OPCODE_LIBHELP);
}



WORDPTR uiGetLibMsg(WORD MsgCode)
{
   return uiGetLibObject(LIBFROMMSG(MsgCode),0,MsgCode,OPCODE_LIBMSG);
}

// RETURN A POINTER TO A MENU ITEM OBJECT
// FIRST ITEM = NUMBER 0

WORDPTR uiGetMenuItem(BINT64 MenuCode, WORDPTR menu, BINT item)
{
if(MENUSPECIAL(MenuCode)==MENU_VARS) {
 // MENU IS VARS
 // RETURN A POINTER TO THE VARIABLE NAME

    WORDPTR *var=rplFindVisibleGlobalByIndex(item);
    if(!var) return 0;
    return var[0];
}
if(MENUSPECIAL(MenuCode)==MENU_USERLIB) {
 // MENU IS LIBS
 // RETURN A POINTER TO THE VARIABLE NAME
    WORDPTR libdir=rplGetSettings((WORDPTR)library_dirname);

    if(!libdir) return 0;

    WORDPTR *direntry=rplFindDirbyHandle(libdir);

    if(!direntry) return 0;

    WORDPTR *var=rplFindVisibleGlobalByIndexInDir(item,direntry);
    if(!var) return 0;
    return var[1];
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

    int nactions=rplListLength(item);

    int index;
    switch(KM_SHIFTPLANE(shift))
    {
    case 0:
        index=0;
        break;
    case SHIFT_LS:
        if(nactions<2) { index=0; break; }
        index=1;
        break;
    case SHIFT_RS:
        if(nactions<2) { index=0; break; }
        if(nactions<3) { index=1; break; }  // SINGLE-SHIFT SPEC
        index=2;
        break;
    case SHIFT_LS|SHIFT_LSHOLD:
        if(nactions<2) { index=0; break; }
        if(nactions<3) { index=1; break; }  // SINGLE-SHIFT SPEC
        if(nactions<4) { index=1; break; }  // LS-HOLD = LS IF NOT GIVEN
        index=3;
        break;
    case SHIFT_RS|SHIFT_RSHOLD:
        if(nactions<2) { index=0; break; }
        if(nactions<3) { index=1; break; }  // SINGLE-SHIFT SPEC
        if(nactions<5) { index=2; break; }  // LS-HOLD = LS IF NOT GIVEN
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


// GET THE HELP OBJECT OF A MENU ITEM
// RETURN 0 IF THE HELP IS NOT A STRING OR THERE WAS NO HELP IN THE MENU DEFINITION

WORDPTR uiGetMenuItemHelp(WORDPTR item)
{

    if(!item) return 0;

    if(ISIDENT(*item)) return item;
    if(ISLIBPTR(*item)) {
        // USER LIBRARY COMMAND
        return uiGetLibPtrHelp(item);
    }
    if(!ISLIST(*item)) {
        if(!ISPROLOG(*item)) {
           // THIS IS A COMMAND, SEARCH FOR HELP
            return uiGetLibCmdHelp(*item);

        }



        return 0;
    }

    // GET HELP ITEM WITHIN THE ITEM

    item=rplGetListElement(item,3);

    if(!item) return 0;

    if(!ISSTRING(*item)) return 0;


    return item;

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

            BINT nresults=uiCmdRunTransparent(CMD_OVR_XEQ,1,1);

            if(nresults==1) ptr=rplPopData();
            else ptr=(WORDPTR)empty_string;      // IF THE PROGRAM FAILED TO RETURN AN OBJECT, JUST USE THE EMPTY STRING
            item=rplPopData();  // RESTORE THE item POINTER IN CASE OF GC
            halUpdateFonts();
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

        BINT w=StringWidthN((char *)(ptr+1),(char *)(ptr+1)+rplGetIdentLength(ptr),*halScreen.FontArray[FONT_MENU]),pos;

        if(w>=scr->clipx2-scr->clipx) pos=scr->clipx+1;
        else pos=(scr->clipx2+1+scr->clipx-w)>>1;

        if((flags&1) || (var && ISDIR(*var[1]))) {
            //ggl_clipvline(scr,scr->clipx2,scr->clipy,scr->clipy2,ggl_mkcolor(color));
            //ggl_cliphline(scr,scr->clipy,scr->clipx,scr->clipx+3,ggl_mkcolor(color));
            //DrawTextN(pos+1,scr->clipy+1,(char *)(ptr+1),(char *)(ptr+1)+rplGetIdentLength(ptr),halScreen.FontArray[FONT_MENU],(color)? 0x4:0xa,scr);

            // FIRST LETTER GRAY BACKGROUND
            ggl_clipvline(scr,pos,scr->clipy,scr->clipy2,ggl_mkcolor( (color)? 0X4:0x8));
            ggl_clipvline(scr,pos+1,scr->clipy,scr->clipy2,ggl_mkcolor( (color)? 0X4:0x8));
            ggl_clipvline(scr,pos+2,scr->clipy,scr->clipy2,ggl_mkcolor( (color)? 0X4:0x8));
            ggl_clipvline(scr,pos+3,scr->clipy,scr->clipy2,ggl_mkcolor( (color)? 0X4:0x8));

            // LOWER 2 LINES GRAY
            //ggl_cliphline(scr,scr->clipy2,scr->clipx,scr->clipx2,ggl_mkcolor( (color)? 0X4:0x6));
            //ggl_cliphline(scr,scr->clipy2-1,scr->clipx,scr->clipx2,ggl_mkcolor( (color)? 0X4:0x6));
            //ggl_cliphline(scr,scr->clipy2-2,scr->clipx,scr->clipx2,ggl_mkcolor( (color)? 0X4:0x6));

            // UNDERLINE FIRST LETTER
            //ggl_cliphline(scr,scr->clipy2,pos,pos+6,ggl_mkcolor( (color)? 0X4:0x6));


        }

        DrawTextN(pos,scr->clipy+1,(char *)(ptr+1),(char *)(ptr+1)+rplGetIdentLength(ptr),*halScreen.FontArray[FONT_MENU],color,scr);

        // DARKEN/LIGHTEN EFFECT ON LAST FEW PIXELS
        if(w>=scr->clipx2-scr->clipx) {
            scr->x=scr->clipx2;
            scr->y=scr->clipy;
            ggl_filter(scr,1,scr->clipy2-scr->clipy+1,(color)? 0xf4:0x0c,&ggl_fltreplace);
            scr->x--;
            ggl_filter(scr,1,scr->clipy2-scr->clipy+1,(color)? 0xf6:0x0a,&ggl_fltreplace);
            scr->x--;
            ggl_filter(scr,1,scr->clipy2-scr->clipy+1,(color)? 0xfa:0x06,&ggl_fltreplace);

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

    BINT totaln;
    BYTEPTR string,endstring;


    // TODO: ADD GROBS HERE

    if(!ISSTRING(*ptr)) {

    WORD ptrprolog=*ptr;

    BINT SavedException=Exceptions;
    BINT SavedErrorCode=ErrorCode;

    Exceptions=0;       // ERASE ANY PREVIOUS ERROR TO ALLOW THE DECOMPILER TO RUN
    // DO NOT SAVE IPtr BECAUSE IT CAN MOVE
    WORDPTR opname=rplDecompile(ptr,DECOMP_NOHINTS);
    Exceptions=SavedException;
    ErrorCode=SavedErrorCode;

    if(!opname) return;

    // HERE WE HAVE A STRING, DO SOME MORE POST-PROCESSING DEPENDING ON OBJECT

    string=(BYTEPTR) (opname+1);
    totaln=rplStrLenCp(opname);
    endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(opname),totaln);

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
        totaln=rplStrLenCp(ptr);
        endstring=(BYTEPTR)utf8nskip((char *)string,(char *)rplSkipOb(ptr),totaln);
    }

    // JUST DISPLAY THE STRING

    BINT w=StringWidthN((char *)string,(char *)endstring,*halScreen.FontArray[FONT_MENU]),pos;
    if(w>=scr->clipx2-scr->clipx) pos=scr->clipx+1;
    else pos=(scr->clipx2+1+scr->clipx-w)>>1;

    if(flags&1) {   // FOR NOW, flags & 1 INDICATES THE MENU IS TO BE DISPLAYED AS A DIRECTORY
        //ggl_clipvline(scr,scr->clipx2,scr->clipy,scr->clipy2,ggl_mkcolor(color));
        //ggl_cliphline(scr,scr->clipy,scr->clipx,scr->clipx+3,ggl_mkcolor(color));
        //DrawTextN(pos+1,scr->clipy+1,(char *)string,(char *)endstring,halScreen.FontArray[FONT_MENU],(color)? 0x4:0xa,scr);

        // FIRST LETTER GRAY BACKGROUND
        ggl_clipvline(scr,pos,scr->clipy,scr->clipy2,ggl_mkcolor( (color)? 0X4:0x8));
        ggl_clipvline(scr,pos+1,scr->clipy,scr->clipy2,ggl_mkcolor( (color)? 0X4:0x8));
        ggl_clipvline(scr,pos+2,scr->clipy,scr->clipy2,ggl_mkcolor( (color)? 0X4:0x8));
        ggl_clipvline(scr,pos+3,scr->clipy,scr->clipy2,ggl_mkcolor( (color)? 0X4:0x8));

        // LOWER 2 LINES GRAY
        //ggl_cliphline(scr,scr->clipy2,scr->clipx,scr->clipx2,ggl_mkcolor( (color)? 0X4:0x6));
        //ggl_cliphline(scr,scr->clipy2-1,scr->clipx,scr->clipx2,ggl_mkcolor( (color)? 0X4:0x6));
        //ggl_cliphline(scr,scr->clipy2-2,scr->clipx,scr->clipx2,ggl_mkcolor( (color)? 0X4:0x6));

        // UNDERLINE FIRST LETTER
        //ggl_cliphline(scr,scr->clipy2,pos,pos+6,ggl_mkcolor( (color)? 0X4:0x6));

    }


    DrawTextN(pos,scr->clipy+1,(char *)string,(char *)endstring,*halScreen.FontArray[FONT_MENU],color,scr);

    // DARKEN/LIGHTEN EFFECT ON LAST FEW PIXELS
    if(w>=scr->clipx2-scr->clipx) {
        scr->x=scr->clipx2;
        scr->y=scr->clipy;
        ggl_filter(scr,1,scr->clipy2-scr->clipy+1,(color)? 0xf4:0x0c,&ggl_fltreplace);
        scr->x--;
        ggl_filter(scr,1,scr->clipy2-scr->clipy+1,(color)? 0xf6:0x0a,&ggl_fltreplace);
        scr->x--;
        ggl_filter(scr,1,scr->clipy2-scr->clipy+1,(color)? 0xfa:0x06,&ggl_fltreplace);

    }

    if(flags&2) {
       // SECOND BIT IN FLAGS MEANS INVERTED
                scr->x=scr->clipx;
                scr->y=scr->clipy;
                ggl_filter(scr,scr->clipx2-scr->clipx+1,scr->clipy2-scr->clipy+1,0,&ggl_fltinvert);
    }

    return;

}
