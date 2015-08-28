/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <newrpl.h>
#include <ui.h>



extern unsigned int __cpu_intoff();
extern void __cpu_inton(unsigned int);
extern void __tmr_eventreschedule();

extern void __keyb_update();

#define DEBOUNCE  16  // 10 SEEMS TO BE ADEQUATE EVEN AT 75 MHz


// KEYBOARD, LOW LEVEL GLOBAL VARIABLES
extern unsigned short int __keyb_buffer[KEYB_BUFFER] __SYSTEM_GLOBAL__;
extern volatile int __keyb_lock __SYSTEM_GLOBAL__;
extern int __keyflags __SYSTEM_GLOBAL__;
extern int __kused __SYSTEM_GLOBAL__,__kcurrent __SYSTEM_GLOBAL__;
extern keymatrix __kmat __SYSTEM_GLOBAL__;
extern int __keyplane __SYSTEM_GLOBAL__;
extern int __keynumber __SYSTEM_GLOBAL__,__keycount __SYSTEM_GLOBAL__;
extern int __keyb_repeattime,__keyb_longpresstime __SYSTEM_GLOBAL__,__keyb_debounce __SYSTEM_GLOBAL__;



// QT-BASED KEYBOARD MESSAGES MUST UPDATE THIS MATRIX;
volatile keymatrix __pckeymatrix;
// QT-BASED TERMINATE MESSAGE COMES IN THIS VARIABLE
volatile int __pc_terminate;






// LOW-LEVEL ROUTINE TO BE USED BY THE IRQ HANDLERS AND EXCEPTION
// HANDLERS ONLY

keymatrix __keyb_getmatrix()
{
return __pckeymatrix;
}


// WRAPPER TO DISABLE INTERRUPTS WHILE READING THE KEYBOARD
// NEEDED ONLY WHEN CALLED FROM WITHIN AN EXCEPTION HANDLER



keymatrix __keyb_getmatrixEX()
{
    unsigned int saved=__cpu_intoff();
    keymatrix m=__keyb_getmatrix();
    thread_processevents();
    __cpu_inton(saved);
    return m;
}



void __keyb_waitrelease()
{
    keymatrix m=1;
    // DO NOT LOCK THE THREAD
    while(m!=0LL) { m=__keyb_getmatrixEX(); }
    __pckeymatrix=0;
}



#define LONG_KEYPRESSTIME (__keyb_longpresstime)
#define REPEAT_KEYTIME (__keyb_repeattime)
#define BOUNCE_KEYTIME (__keyb_debounce)

#define KF_RUNNING   1
#define KF_ALPHALOCK 2
#define KF_NOREPEAT  4



// RETURNS THE CURRENT WORKING MATRIX INSTEAD OF
// MESSING WITH THE HARDWARE, BUT ONLY IF KEYBOARD HANDLERS WERE STARTED
keymatrix keyb_getmatrix()
{
    return __pckeymatrix;
}



void __keyb_init()
{
__keyflags=KF_RUNNING;
__keyplane=0;
__kused=__kcurrent=0;
__keynumber=0;
__kmat=0LL;
__keyb_repeattime=50/KEYB_SCANSPEED;
__keyb_longpresstime=800/KEYB_SCANSPEED;
__keyb_debounce=0; //20/KEYB_SCANSPEED;
__keyb_lock=0;
__pckeymatrix=0;
// INITIALIZE TIMER EVENT 0

tmr_events[0].eventhandler=__keyb_update;
tmr_events[0].delay=(KEYB_SCANSPEED*tmr_getsysfreq())/1000;

tmr_events[0].status=0;

__keyflags|=KF_RUNNING;

}


void __keyb_stop(unsigned int *keysave)
{
UNUSED_ARGUMENT(keysave);

tmr_events[0].status=0;

__keyflags&=~1;
}


