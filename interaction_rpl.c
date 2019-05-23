
// THIS FILE ISOLATES THE COMPONENTS THAT INTERACT WITH NEWRPL
// THIS IS ONLY NEEDED BECAUSE OF THE REDEFINITION OF THE TYPE WORD
// IN NEWRPL CONFLICTING WITH QT


// ONLY REQUIRED UNDER MINGW
#ifdef DrawText
#undef DrawText
#endif
#define WORD _WORD


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
    if(!newobj) { rplBlameError(0); return 0; }
    rplOverwriteData(1,newobj);
    return 1;
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
extern BYTEPTR __usb_longbuffer[2];              // DOUBLE BUFFERING FOR LONG TRANSMISSIONS OF DATA
extern BYTE __usb_rxtmpbuffer[RAWHID_TX_SIZE+1] __SYSTEM_GLOBAL__;  // TEMPORARY BUFFER FOR NON-BLOCKING CONTROL TRANSFERS

extern BINT __usb_localbigoffset __SYSTEM_GLOBAL__;

extern volatile int __usb_paused;
extern void usb_irqservice();
extern int usb_remoteready();



int usb_transmitdata(BYTEPTR data,int nbytes)
{
    int fileid=usb_txfileopen('O');
    if(!fileid) return 0;

    if(!usb_filewrite(fileid,data,nbytes)) return 0;

    if(!usb_txfileclose(fileid)) return 0;

    return 1;
}


int usbsendtoremote(uint32_t *data,int nwords)
{
    return usb_transmitdata((BYTEPTR) data,nwords*sizeof(WORD));
}

int usbremotearchivestart()
{
    WORD program[]={
        MKPROLOG(SECO,2),
        CMD_USBARCHIVE,
        CMD_QSEMI,
    };

    return usb_transmitdata((BYTEPTR)&program,3*sizeof(WORD));
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

    return usb_transmitdata((BYTEPTR)&program,3*sizeof(WORD));
}





// RECEIVE AN ENTIRE ARCHIVE, RETURN WORD COUNT, OR -1 IF ERROR
int usbreceivearchive(uint32_t *buffer,int bufsize)
{
  int count,bytesread;
  int fileid;
  BYTEPTR bufptr;

  if(!usb_waitfordata(4)) return 0;

  fileid=usb_rxfileopen();

  if(usb_filetype(fileid)!='B') { usb_rxfileclose(fileid); return 0; }

  count=0;

  bufptr=(BYTEPTR) buffer;
  do {

      bytesread=usb_fileread(fileid,bufptr,bufsize-count);

      if(bytesread>0) {
          bufptr+=bytesread;
      }


      if(bytesread<bufsize-count) {
          if(usb_eof(fileid)) {
              // WE FINISHED THE FILE!
              break;
          }
      }
      // MORE DATA IS EXPECTED, ALLOCATE MORE MEMORY

  } while(bytesread>0);

  if((!bytesread) && (!usb_eof(fileid))) return -1;
  //  WE ARE DONE WITH THE TRANSMISSION
  if(!usb_rxfileclose(fileid)) return -1;

    return (bytesread+3)>>2;

}

// SEND AN ENTIRE ARCHIVE, RETURN WORD COUNT, OR -1 IF ERROR
int usbsendarchive(uint32_t *buffer,int bufsize)
{
    if(!usb_isconfigured()) {
        rplError(ERR_USBNOTCONNECTED);
        return -1;
    }

    int fileid=usb_txfileopen('B');
    if(!fileid) {
        rplError(ERR_USBCOMMERROR);     // IT'S ACTUALLY OUT OF BUFFER MEMORY
        return -1;
    }


    if(!usb_filewrite(fileid,(BYTEPTR)buffer,bufsize*sizeof(WORD))) {
            rplError(ERR_USBCOMMERROR);
            bufsize=-1;
        }


    if(!usb_txfileclose(fileid)) {
        rplError(ERR_USBCOMMERROR);
        bufsize=-1;
    }

    return bufsize;

}

void usbflush()
{

}

void setExceptionPoweroff()
{
    HWExceptions|=EX_POWEROFF;
}

int change_autorcv(int newfl)
{
    int fl=rplTestSystemFlag(FL_NOAUTORECV);
    if(newfl) rplSetSystemFlag(FL_NOAUTORECV);
    else rplClrSystemFlag(FL_NOAUTORECV);
    return fl;
}

