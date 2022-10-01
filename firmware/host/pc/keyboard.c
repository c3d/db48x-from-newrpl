/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <ui.h>

void thread_processevents();    // ONLY NEEDED TO AVOID LOCKING IN MULTITHREADED ENVIRONMENTS

INTERRUPT_TYPE cpu_intoff_nosave();
void cpu_inton_nosave(INTERRUPT_TYPE state);
void tmr_eventreschedule();

void keyb_irq_update();

// KEYBOARD, LOW LEVEL GLOBAL VARIABLES
extern unsigned short int keyb_irq_buffer[KEYB_BUFFER] SYSTEM_GLOBAL;
extern volatile int keyb_irq_lock SYSTEM_GLOBAL;
extern int keyflags SYSTEM_GLOBAL;
extern int kused SYSTEM_GLOBAL, kcurrent SYSTEM_GLOBAL;
extern keymatrix kmat SYSTEM_GLOBAL;
extern int keyplane SYSTEM_GLOBAL;
extern int keynumber SYSTEM_GLOBAL, keycount SYSTEM_GLOBAL;
extern int keyb_irq_repeattime;
extern int keyb_irq_longpresstime SYSTEM_GLOBAL;
extern int keyb_irq_debounce SYSTEM_GLOBAL;

// QT-BASED KEYBOARD MESSAGES MUST UPDATE THIS MATRIX;
volatile keymatrix pckeymatrix;
// QT-BASED TERMINATE MESSAGE COMES IN THIS VARIABLE
extern volatile int pc_terminate;




// LOW-LEVEL ROUTINE TO BE USED BY THE IRQ HANDLERS AND EXCEPTION
// HANDLERS ONLY

keymatrix keyb_irq_getmatrix()
{
    return pckeymatrix;
}

// WRAPPER TO DISABLE INTERRUPTS WHILE READING THE KEYBOARD
// NEEDED ONLY WHEN CALLED FROM WITHIN AN EXCEPTION HANDLER

keymatrix keyb_irq_getmatrixEX()
{
    INTERRUPT_TYPE saved = cpu_intoff_nosave();
    keymatrix m = keyb_irq_getmatrix();
    thread_processevents();
    cpu_inton_nosave(saved);
    return m;
}

void keyb_irq_waitrelease()
{
    keymatrix m = 1;
    // DO NOT LOCK THE THREAD
    while(m != 0LL) {
        m = keyb_irq_getmatrixEX();
    }
    pckeymatrix = 0;
}

#define LONG_KEYPRESSTIME (keyb_irq_longpresstime)
#define REPEAT_KEYTIME (keyb_irq_repeattime)
#define BOUNCE_KEYTIME (keyb_irq_debounce)

#define KF_RUNNING   1
#define KF_ALPHALOCK 2
#define KF_NOREPEAT  4

// RETURNS THE CURRENT WORKING MATRIX INSTEAD OF
// MESSING WITH THE HARDWARE, BUT ONLY IF KEYBOARD HANDLERS WERE STARTED
keymatrix keyb_getmatrix()
{
    return pckeymatrix;
}

void keyb_irq_init()
{
    keyflags = KF_RUNNING;
    keyplane = 0;
    kused = kcurrent = 0;
    keynumber = 0;
    kmat = 0LL;
    keyb_irq_repeattime = 50 / KEYB_SCANSPEED;
    keyb_irq_longpresstime = 800 / KEYB_SCANSPEED;
    keyb_irq_debounce = 0;        //20/KEYB_SCANSPEED;
    keyb_irq_lock = 0;
    pckeymatrix = 0;
// INITIALIZE TIMER EVENT 0

    tmr_events[0].eventhandler = keyb_irq_update;
    tmr_events[0].delay = (KEYB_SCANSPEED * tmr_getsysfreq()) / 1000;

    tmr_events[0].status = 0;

    keyflags |= KF_RUNNING;

}

void keyb_irq_stop(unsigned int *keysave)
{
    UNUSED(keysave);

    tmr_events[0].status = 0;

    keyflags &= ~1;
}
