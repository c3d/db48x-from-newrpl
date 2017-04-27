/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

extern unsigned int __cpu_intoff();
extern void __cpu_inton(unsigned int);
extern void __tmr_eventreschedule();

extern void __keyb_update();




#define DEBOUNCE  16  // 10 SEEMS TO BE ADEQUATE EVEN AT 75 MHz


// KEYBOARD, LOW LEVEL GLOBAL VARIABLES
extern unsigned short int __keyb_buffer[KEYB_BUFFER];
extern volatile int __keyb_lock;
extern int __keyflags;
extern int __kused,__kcurrent ;
extern keymatrix __kmat ;
extern int __keyplane ;
extern int __keynumber ,__keycount ;
extern int __keyb_repeattime,__keyb_longpresstime ,__keyb_debounce ;












// LOW-LEVEL ROUTINE TO BE USED BY THE IRQ HANDLERS AND EXCEPTION
// HANDLERS ONLY

keymatrix __keyb_getmatrix()
{


    unsigned int volatile * GPGDAT = (unsigned int*) (IO_REGS+0x64); //data
    unsigned int volatile * GPGCON = (unsigned int*) (IO_REGS+0x60); //control

    unsigned int tmp[DEBOUNCE],f,k;


    unsigned int lo=0,hi=0;

    int col;
    unsigned int control;

    for(col=7;col>=4;--col)
    {

    control = 1<<((col+8)*2);
    control = control | 0x1;
    *GPGDAT = 0;        // DRIVE THE OUTPUT COLUMN LOW
    *GPGCON = control;

    // DEBOUNCE TECHNIQUE

    // FILL DEBOUNCE BUFFER
    for(f=0;f<DEBOUNCE;++f) tmp[f]=f;
    // DO CIRCULAR BUFFER, CHECKING FOR NOISE ON EVERY READ
    k=0;
    do {
        tmp[k]=*GPGDAT & 0xfe;
    for(f=1;f<DEBOUNCE;++f) {
        if(tmp[f]!=tmp[f-1]) break;
    }
    ++k;
    if(k>=DEBOUNCE) k=0;
    } while(f<DEBOUNCE);


    hi=(hi<<8) | ((~(tmp[0]))&0xfe);

    }

    for(;col>=0;--col)
    {

    control = 1<<((col+8)*2);
    control = control | 0x1;
    *GPGDAT = 0;        // DRIVE THE OUTPUT COLUMN LOW
    *GPGCON = control;

    // GPGDAT WAS SET TO ZERO, SO THE SELECTED COLUMN IS DRIVEN LOW
    for(f=0;f<DEBOUNCE;++f) tmp[f]=f;
    // DO CIRCULAR BUFFER, CHECKING FOR NOISE ON EVERY READ
    k=0;
    do {
        tmp[k]=*GPGDAT & 0xfe;
    for(f=1;f<DEBOUNCE;++f) {
        if(tmp[f]!=tmp[f-1]) break;
    }
    ++k;
    if(k>=DEBOUNCE) k=0;
    } while(f<DEBOUNCE);


    lo=(lo<<8) | ((~(tmp[0]))&0xfe);

    }

    *GPGCON = 0x5555AAA9; // SET TO TRIGGER INTERRUPTS ON ANY KEY
    *GPGDAT = 0;        // DRIVE ALL OUTPUT COLUMNS LOW

    unsigned int volatile *GPFDAT = ((unsigned int*) (IO_REGS+0x54));


    for(f=0;f<DEBOUNCE;++f) tmp[f]=f;
    // DO CIRCULAR BUFFER, CHECKING FOR NOISE ON EVERY READ
    k=0;
    do {
        tmp[k]=*GPFDAT& 0x71;
    for(f=1;f<DEBOUNCE;++f) {
        if(tmp[f]!=tmp[f-1]) break;
    }
    ++k;
    if(k>=DEBOUNCE) k=0;
    } while(f<DEBOUNCE);



    hi |=  (tmp[k]&0x70)<<24;
    hi |=  tmp[k]<<31;

    return ((keymatrix)lo) | (((keymatrix)hi)<<32);
}


// WRAPPER TO DISABLE INTERRUPTS WHILE READING THE KEYBOARD
// NEEDED ONLY WHEN CALLED FROM WITHIN AN EXCEPTION HANDLER



keymatrix __keyb_getmatrixEX()
{
    unsigned int saved=__cpu_intoff();
    keymatrix m=__keyb_getmatrix();
    __cpu_inton(saved);
    return m;
}


void __keyb_waitrelease()
{
    keymatrix m=1;
    while(m!=0LL) { m=__keyb_getmatrixEX(); }
}



#define LONG_KEYPRESSTIME (__keyb_longpresstime)
#define REPEAT_KEYTIME (__keyb_repeattime)
#define BOUNCE_KEYTIME (__keyb_debounce)

#define KF_RUNNING   1
#define KF_ALPHALOCK 2
#define KF_NOREPEAT  4
#define KF_UPDATED   8



#define GPGCON ((unsigned int *)(IO_REGS+0x60))
#define GPGDAT ((unsigned int *)(IO_REGS+0x64))
#define GPGUP ((unsigned int *)(IO_REGS+0x68))
#define GPFCON ((unsigned int *)(IO_REGS+0x50))
#define EXTINT0 ((unsigned int *)(IO_REGS+0x88))
#define EXTINT1 ((unsigned int *)(IO_REGS+0x8c))
#define EINTMASK ((unsigned int *)(IO_REGS+0xa4))
#define EINTPEND ((unsigned int *)(IO_REGS+0xa8))
#define INTMSK   ((unsigned int *)(INT_REGS+0x8))
#define INTPND   ((unsigned int *)(INT_REGS+0x10))
#define SRCPND   ((unsigned int *)(INT_REGS+0x0))






// RETURNS THE CURRENT WORKING MATRIX INSTEAD OF
// MESSING WITH THE HARDWARE, BUT ONLY IF KEYBOARD HANDLERS WERE STARTED
keymatrix keyb_getmatrix()
{
    if(__keyflags&KF_RUNNING)	return __kmat;
    else return __keyb_getmatrix();
}

// ANALYZE CHANGES IN THE KEYBOARD STATUS AND POST MESSAGES ACCORDINGLY


void __keyb_int_handler()
{

*EINTMASK|=0xfe70;

    __keyb_update();

*EINTPEND|=0xfe70;
*EINTMASK&=~0xfe70;


}


void __keyb_init()
{
__keyflags=KF_RUNNING;
__keyplane=0;
__kused=__kcurrent=0;
__keynumber=0;
__kmat=0LL;
__keyb_repeattime=100/KEYB_SCANSPEED;
__keyb_longpresstime=1000/KEYB_SCANSPEED;
__keyb_debounce=20/KEYB_SCANSPEED;
__keyb_lock=0;
// INITIALIZE TIMER EVENT 0

tmr_events[0].eventhandler=__keyb_update;
tmr_events[0].delay=(KEYB_SCANSPEED*tmr_getsysfreq())/1000;

tmr_events[0].status=0;

// MASK ALL EXTINT UNTIL THEY ARE PROPERLY PROGRAMMED
*INTMSK|=0x31;


*GPGCON=0x5555AAA9; // DRIVE ALL COLUMNS TO OUTPUT, ROWS TO EINT
*GPGDAT=0;      // DRIVE OUTPUTS LOW
*GPGUP=0x1;    // ENABLE PULLUPS ON ALL INPUT LINES, DISABLE ON ALL OUTPUTS
//keysave[1]=*GPFCON;
*GPFCON=(*GPFCON&0xffffc0fc)|0x2a02;    // SET ALL SHIFTS TO GENERATE INTERRUPTS

__irq_addhook(5,&__keyb_int_handler);
__irq_addhook(4,&__keyb_int_handler);	// SHIFTS
__irq_addhook(0,&__keyb_int_handler); // ON

*EXTINT0=(*EXTINT0&0xf000fff0)|0x06660006;      // ALL SHIFTS TRIGGER ON BOTH EDGES
*EXTINT1=0x66666666;                            // ALL OTHER KEYS TRIGGER ON BOTH EDGES
*EINTMASK=(*EINTMASK&0x00ff018f);               // UNMASK 4,5,6 AND 9-15
*EINTPEND=0xffffffff;                           // CLEAR ALL PENDING INTERRUPTS
*INTMSK=*INTMSK&0xffffffce;                     // UNMASK EXTERNAL INTERRUPTS
*SRCPND|=0x31;
*INTPND|=0x31;                     // UNMASK EXTERNAL INTERRUPTS




}


void __keyb_stop(unsigned int *keysave)
{

tmr_events[0].status=0;

// DISABLE INTERRUPTS, STATUS WILL BE FULLY RESTORED ON EXIT
*INTMSK|=0x31;
*EINTMASK|=0xFE70;
__irq_releasehook(5);
__irq_releasehook(4);
__irq_releasehook(0);

// RESTORE IO PORT CONFIGURATION
*GPGCON=keysave[0];
*GPFCON=keysave[1];
__keyflags&=~KF_RUNNING;
}


