
// THIS FILE ISOLATES THE COMPONENTS THAT INTERACT WITH NEWRPL
// THIS IS ONLY NEEDED BECAUSE OF THE REDEFINITION OF THE TYPE WORD
// IN NEWRPL CONFLICTING WITH QT

#include <newrpl.h>
#include <libraries.h>
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

// COMPILE OBJECT AT LEVEL 1 OF THE STACK
int compileobject()
{
    int strsize;
    BYTEPTR strdata;
    if(rplDepthData()<1) return 0;
    if(!ISSTRING(*rplPeekData(1))) return 0;

    strdata=(BYTEPTR)rplPeekData(1);
    strdata+=4;
    strsize=rplStrSize(rplPeekData(1));

    WORDPTR newobj=rplCompile(strdata,strsize,0);
    if(!newobj) { rplBlameError(CMD_FROMSTR); return 0; }
    rplOverwriteData(1,newobj);
}



// THESE ARE INTERNALS FROM THE USB DRIVER - COPIED HERE FOR PROPER INTERACTION
extern BINT __usb_longoffset;
extern BINT __usb_longactbuffer;                 // WHICH BUFFER IS BEING WRITTEN
extern BINT __usb_longlastsize;                  // LAST BLOCK SIZE IN A LONG TRANSMISSION
extern BYTEPTR __usb_rcvbuffer;
extern WORD __usb_rcvtotal __SYSTEM_GLOBAL__;
extern WORD __usb_rcvpartial __SYSTEM_GLOBAL__;
extern WORD __usb_rcvcrc __SYSTEM_GLOBAL__;
extern BINT __usb_rcvblkmark __SYSTEM_GLOBAL__;    // TYPE OF RECEIVED BLOCK (ONE OF USB_BLOCKMARK_XXX CONSTANTS)

extern void usb_irqservice();
extern int usb_remoteready();



int usbsendtoremote(uint32_t *data,int nwords)
{
    return usb_transmitdata((BYTEPTR) data,nwords*sizeof(WORD));
}

int usbremotearchivestart()
{
    WORD program[]={
        CMD_USBARCHIVE
    };

    return usb_transmitdata((BYTEPTR)&program,1*sizeof(WORD));
}

int usbremoterestorestart()
{
    WORD program[]={
        MKPROLOG(SECO,3),
        MAKESINT(3),
        CMD_USBRESTORE,
        CMD_QSEMI,
    };

    return usb_transmitdata((BYTEPTR)&program,4*sizeof(WORD));
}

int usbremotefwupdatestart()
{

    WORD program[]={
        MKPROLOG(SECO,2),
        CMD_USBFWUPDATE,
        CMD_QSEMI,
    };

    int attempts=0;
    while(!usb_isconfigured()) {
        usb_irqservice();
        if(attempts>300000) return 0;
        ++attempts;
    }

    return usb_transmitdata((BYTEPTR)&program,3*sizeof(WORD));
}





// RECEIVE AN ENTIRE ARCHIVE, RETURN WORD COUNT, OR -1 IF ERROR
int usbreceivearchive(uint32_t *buffer,int bufsize)
{
    int flag=rplTestSystemFlag(FL_NOAUTORECV);
    rplSetSystemFlag(FL_NOAUTORECV);

    if(!usb_receivelong_start()) {
        if(flag) rplSetSystemFlag(FL_NOAUTORECV);
        else     rplClrSystemFlag(FL_NOAUTORECV);


        return 0;
    }


     BINT count=0;

     do {
         while(!usb_hasdata()) usb_irqservice();

         // CHECK IF THE RECEIVED BLOCK IS OURS
         if((__usb_rcvblkmark==USB_BLOCKMARK_MULTISTART)||(__usb_rcvblkmark==USB_BLOCKMARK_SINGLE)) {
             if(count) {
                 // BAD BLOCK, WE CAN ONLY RECEIVE THE FIRST BLOCK ONCE, ABORT
                 if(flag) rplSetSystemFlag(FL_NOAUTORECV);
                 else     rplClrSystemFlag(FL_NOAUTORECV);
                 return -1;
             }
             if(__usb_rcvblkmark==USB_BLOCKMARK_SINGLE) __usb_longlastsize=__usb_rcvtotal;

         }
         if((__usb_rcvblkmark==USB_BLOCKMARK_MULTI)||(__usb_rcvblkmark==USB_BLOCKMARK_MULTIEND)) {
             if(!count) {
                 // BAD BLOCK, WE CAN ONLY RECEIVE THE FIRST BLOCK ONCE, ABORT
                 if(flag) rplSetSystemFlag(FL_NOAUTORECV);
                 else     rplClrSystemFlag(FL_NOAUTORECV);
                 return -1;
             }

         }


         // HAVE A GOOD BLOCK!

         if(count+__usb_rcvtotal>=bufsize*sizeof(WORD)) {
             if(flag) rplSetSystemFlag(FL_NOAUTORECV);
             else     rplClrSystemFlag(FL_NOAUTORECV);

             return -1; // BUFFER TOO SMALL
            }

        memmoveb(((unsigned char *)buffer)+count,__usb_rcvbuffer,__usb_rcvtotal);
        count+=__usb_rcvtotal;
        if(__usb_rcvblkmark==USB_BLOCKMARK_MULTIEND) {
            // LAST BLOCK RECEIVED AND PROCESSED
            usb_releasedata(0);
            break;
        }
        usb_releasedata(0);
     } while(1);

    usb_receivelong_finish();
    if(flag) rplSetSystemFlag(FL_NOAUTORECV);
    else     rplClrSystemFlag(FL_NOAUTORECV);

    return (count+3)>>2;

}

// RECEIVE AN ENTIRE ARCHIVE, RETURN WORD COUNT, OR -1 IF ERROR
int usbsendarchive(uint32_t *buffer,int bufsize)
{
    return usb_transmitdata((BYTEPTR)buffer,bufsize*sizeof(WORD));
}

void usbflush()
{
    usb_irqservice();
}

void setExceptionPoweroff()
{
    HWExceptions|=EX_POWEROFF;
}
