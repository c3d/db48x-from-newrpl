
// THIS FILE ISOLATES THE COMPONENTS THAT INTERACT WITH NEWRPL
// THIS IS ONLY NEEDED BECAUSE OF THE REDEFINITION OF THE TYPE WORD
// IN NEWRPL CONFLICTING WITH QT

#include <newrpl.h>
#include <ui.h>

// GET A POINTER TO AN OBJECT ON THE STACK, AS WELL AS ITS SIZE
uint32_t *getrplstackobject(int level,int *size)
{
    if(level>rplDepthData()) return 0;
    if(level<1) return 0;
    if(size) *size=rplObjSize(rplPeekData(level));
    return rplPeekData(level);
}

void removestackobject(int level,int nitems)
{
    if(rplDepthData()>=level+nitems-1) rplRemoveAtData(level,nitems);
    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY|FORM_DIRTY|STACK_DIRTY|MENU1_DIRTY|MENU2_DIRTY|STAREA_DIRTY;

}



// DECOMPILES THE OBJECT AT THE STACK, THEN STORES THE SIZE OF THE STRING AND
// RETURNS A POINTER TO THE TEXT
char *getdecompiledrplobject(int level,int *strsize)
{
    if(level>rplDepthData()) return 0;
    if(level<1) return 0;

    WORDPTR newobj=rplDecompile(rplPeekData(level),DECOMP_MAXWIDTH(60));
    if(!newobj) return 0;
    if(strsize) *strsize=rplStrSize(newobj);
    return (char *)(newobj+1);
}

// PUSH A BINARY OBJECT IN THE STACK
// OBJECT IS GIVEN BY A STREAM OF BYTES
void pushobject(char *data,int sizebytes)
{
    if(sizebytes&3) return; // SIZE MUST BE MULTIPLE OF 4, OTHERWISE IT'S AN INVALID OBJECT

    WORDPTR newobj=rplAllocTempOb((sizebytes-1)/4);
    if(!newobj) return;
    memmoveb((char *)newobj,data,sizebytes);
    rplPushData(newobj);
    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY|FORM_DIRTY|STACK_DIRTY|MENU1_DIRTY|MENU2_DIRTY|STAREA_DIRTY;
}

// PUSH A TEXT OBJECT IN THE STACK
void pushtext(char *data,int sizebytes)
{
    WORDPTR newobj=rplCreateStringBySize(sizebytes);
    if(!newobj) return;

    memmoveb((char *)(newobj+1),data,sizebytes);

    rplPushData(newobj);
    halScreen.DirtyFlag|=CMDLINE_ALLDIRTY|FORM_DIRTY|STACK_DIRTY|MENU1_DIRTY|MENU2_DIRTY|STAREA_DIRTY;

}
