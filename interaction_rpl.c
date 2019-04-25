
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
extern BYTEPTR __usb_longbuffer[2];              // DOUBLE BUFFERING FOR LONG TRANSMISSIONS OF DATA
extern BYTE __usb_rxtmpbuffer[RAWHID_TX_SIZE+1] __SYSTEM_GLOBAL__;  // TEMPORARY BUFFER FOR NON-BLOCKING CONTROL TRANSFERS

extern BINT __usb_localbigoffset __SYSTEM_GLOBAL__;

extern volatile int __usb_paused;
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
         BINT datasize,byteoffset=__usb_localbigoffset,totalsize=0;

         // WAIT FOR NEXT BLOCK IN A MULTIPART TRANSACTION
         if(!usb_waitfordata()) {
             usb_ignoreuntilend();
             if(flag) rplSetSystemFlag(FL_NOAUTORECV);
             else     rplClrSystemFlag(FL_NOAUTORECV);

             return -1;
         }

         BYTEPTR data2=usb_accessdata(&datasize);

         if(!totalsize) totalsize=usb_remotetotalsize();
         if(byteoffset!=usb_remoteoffset()) {
             // WE NEED TO REQUEST THE CORRECT OFFSET, BUT IT'S IMPOSSIBLE IN LONG TRANSMISSIONS
             usb_setoffset(-1);  // ABORT, SOMETHING WENT WRONG
             usb_releasedata();
             usb_sendfeedback();
             if(flag) rplSetSystemFlag(FL_NOAUTORECV);
             else     rplClrSystemFlag(FL_NOAUTORECV);

             return -1;
         }


         if(!usb_checkcrc()) {
             // WE NEED TO REQUEST THE CORRECT OFFSET, BUT IT'S IMPOSSIBLE IN LONG TRANSMISSIONS
             usb_setoffset(-1);  // ABORT, SOMETHING WENT WRONG
             usb_releasedata();
             usb_sendfeedback();
             if(flag) rplSetSystemFlag(FL_NOAUTORECV);
             else     rplClrSystemFlag(FL_NOAUTORECV);

             return -1;
         }

         // WE GOT A BLOCK AND THE CRC MATCHES, IT'S ALL WE NEED

         byteoffset+=datasize;
         usb_addremoteoffset(datasize);  // ADVANCE THE COUNTER OF REMOTE DATA RECEIVED FROM THE REMOTE
         usb_setoffset(byteoffset);      // AND OUR LOCAL COUNTER

         if(datasize<USB_BLOCKSIZE) {
         usb_sendfeedback();

         usb_waitdatareceived(); // WAIT FOR THE DATA RECEIVED ACKNOWLEDGEMENT TO BE SENT TO THE REMOTE, INDICATING WE FINISHED RECEIVING DATA

         // THIS IS REQUIRED ONLY AT END OF TRANSMISSION:
         usb_setoffset(0);   // NEXT TRANSMISSION WE NEED TO REQUEST FROM OFFSET ZERO, RATHER THAN RESUME FROM WHERE WE LEFT OFF

         }

         // DONE RECEIVING BLOCKS OF DATA


         // CHECK IF THE RECEIVED BLOCK IS OURS
         if((__usb_rcvblkmark==USB_BLOCKMARK_MULTISTART)||(__usb_rcvblkmark==USB_BLOCKMARK_SINGLE)) {
             if(__usb_longoffset) {
                 // BAD BLOCK, WE CAN ONLY RECEIVE THE FIRST BLOCK ONCE, ABORT
                 usb_releasedata();
                 if(flag) rplSetSystemFlag(FL_NOAUTORECV);
                 else     rplClrSystemFlag(FL_NOAUTORECV);

                 return -1;
             }
             if(__usb_rcvblkmark==USB_BLOCKMARK_SINGLE) __usb_longlastsize=datasize;

         }
         if((__usb_rcvblkmark==USB_BLOCKMARK_MULTI)||(__usb_rcvblkmark==USB_BLOCKMARK_MULTIEND)) {
             if(!__usb_longoffset) {
                 // BAD BLOCK, WE CAN ONLY RECEIVE THE FIRST BLOCK ONCE, ABORT
                 usb_releasedata();
                 if(flag) rplSetSystemFlag(FL_NOAUTORECV);
                 else     rplClrSystemFlag(FL_NOAUTORECV);

                 return -1;
             }
             if(__usb_rcvblkmark==USB_BLOCKMARK_MULTIEND) __usb_longlastsize=datasize;

         }

         // TAKE OWNERSHIP OF THE BUFFER

         // HAVE A GOOD BLOCK!

         if(count+datasize>=bufsize*sizeof(WORD)) {
             if(flag) rplSetSystemFlag(FL_NOAUTORECV);
             else     rplClrSystemFlag(FL_NOAUTORECV);

             return -1; // BUFFER TOO SMALL
            }

        memmoveb(((unsigned char *)buffer)+count,__usb_rcvbuffer,datasize);
        count+=datasize;
        __usb_longoffset+=datasize;
        if(__usb_rcvblkmark==USB_BLOCKMARK_MULTIEND) {
            // LAST BLOCK RECEIVED AND PROCESSED
            usb_releasedata();
            break;
        }
        usb_releasedata();
     } while(1);

    usb_receivelong_finish();
    if(flag) rplSetSystemFlag(FL_NOAUTORECV);
    else     rplClrSystemFlag(FL_NOAUTORECV);

    return (count+3)>>2;

}

// SEND AN ENTIRE ARCHIVE, RETURN WORD COUNT, OR -1 IF ERROR
int usbsendarchive(uint32_t *buffer,int bufsize)
{
    if(!usb_isconfigured()) {
        rplError(ERR_USBNOTCONNECTED);
        return -1;
    }

    if(!usb_transmitlong_start()) {
        rplError(ERR_USBCOMMERROR);     // IT'S ACTUALLY OUT OF BUFFER MEMORY
        return -1;
    }

    int k;
    for(k=0;k<bufsize;++k)
    {
        if(!usb_transmitlong_word(buffer[k])) {
            rplError(ERR_USBCOMMERROR);
            break;
        }
    }

    usb_transmitlong_finish();

    if(k!=bufsize) return -1;

    return bufsize;

}

void usbflush()
{

}

void setExceptionPoweroff()
{
    HWExceptions|=EX_POWEROFF;
}
