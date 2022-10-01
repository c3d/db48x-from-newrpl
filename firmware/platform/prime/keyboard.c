/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

extern INTERRUPT_TYPE cpu_intoff_nosave();
extern void cpu_inton_nosave(INTERRUPT_TYPE state);
extern void tmr_eventreschedule();

extern void keyb_irq_update();

#define DEBOUNCE  16    // 10 SEEMS TO BE ADEQUATE EVEN AT 75 MHz

// KEYBOARD, LOW LEVEL GLOBAL VARIABLES
extern unsigned int keyb_irq_buffer[KEYB_BUFFER];
extern volatile int keyb_irq_lock;
extern int          keyflags;
extern int          kused, kcurrent;
extern keymatrix    kmat;
extern int          keyplane;
extern int          keynumber, keycount;
extern int          keyb_irq_repeattime;
extern int          keyb_irq_longpresstime;
extern int          keyb_irq_debounce;

// LOW-LEVEL ROUTINE TO BE USED BY THE IRQ HANDLERS AND EXCEPTION
// HANDLERS ONLY

keymatrix keyb_irq_getmatrix()
{
    unsigned int c;
    uint32_t v;

    unsigned int lo = 0, hi = 0;

    int col;
    unsigned int control;

    *GPGCON=0;    // ALL INPUTS



    for(col = 7; col >= 4; --col) {

        *GPDDAT= (*GPDDAT & ~0xff) | (1<<col);  // ONLY 1 COLUMN SET TO HIGH, ALL OTHERS TO LOW (OTHERWISE THERE'S GHOSTING ON THE FLOATING LINES SET TO INPUT)
        *GPDUDP= (*GPDUDP &0xffff0000) | (0x5555 ^ (1<< (2*col)));    // PULLDOWN ALL LINES EXCEPT THE OUTPUT ONE
        *GPDCON= (*GPDCON &0xffff0000) | (1<< (2*col));    // ONLY 1 COLUMN TO OUTPUT

        // DEBOUNCE TECHNIQUE
        c = 0;
        while (c < DEBOUNCE) {
            uint32_t nv = *GPGDAT & 0xfe;
            if (nv == v) {
                ++c;
            } else {
                c = 0;
                v = nv;
            }
        }

        hi = (hi << 8) | (v & 0xff);

    }

    for(; col >= 0; --col) {

        *GPDDAT= (*GPDDAT & ~0xff) | (1<<col);  // ONLY 1 COLUMN SET TO HIGH, ALL OTHERS TO LOW (OTHERWISE THERE'S GHOSTING ON THE FLOATING LINES SET TO INPUT)
        *GPDUDP= (*GPDUDP &0xffff0000) | (0x5555 ^ (1<< (2*col)));    // PULLDOWN ALL LINES EXCEPT THE OUTPUT ONE
        *GPDCON= (*GPDCON &0xffff0000) | (1<< (2*col));    // ONLY 1 COLUMN TO OUTPUT

        // DEBOUNCE TECHNIQUE
        c = 0;
        while (c < DEBOUNCE) {
            uint32_t nv = *GPGDAT & 0xfe;
            if (nv == v) {
                ++c;
            } else {
                c = 0;
                v = nv;
            }
        }

        lo = (lo << 8) | ( v & 0xff);

    }


    c = 0;
    while (c < DEBOUNCE) {
        uint32_t nv = *GPGDAT & 0x1;
        if (nv == v) {
            ++c;
        } else {
            c = 0;
            v = nv;
        }
    }

    *GPDCON= (*GPDCON &0xffff0000) | 0x5555;    // ALL COLUMNS TO OUTPUT
    *GPDDAT|= 0xff;      // ALL LINES OUTPUT HIGH
    *GPDUDP&= ~0xFFFF;   // DISABLE ALL PULL UP/DOWN


    *GPGCON = 0xaaaa;        // ALL KEYS FUNCTION BACK TO EINT

    hi |= v << 31;          // read the on key at GPG0

    return ((keymatrix) lo) | (((keymatrix) hi) << 32);
}

// WRAPPER TO DISABLE INTERRUPTS WHILE READING THE KEYBOARD
// NEEDED ONLY WHEN CALLED FROM WITHIN AN EXCEPTION HANDLER

keymatrix keyb_irq_getmatrixEX()
{
    INTERRUPT_TYPE saved = cpu_intoff_nosave();
    keymatrix m = keyb_irq_getmatrix();
    cpu_inton_nosave(saved);
    return m;
}

void keyb_irq_waitrelease()
{
    keymatrix m = 1;
    while(m != 0LL) {
        m = keyb_irq_getmatrixEX();
    }
}

// RETURNS THE CURRENT WORKING MATRIX INSTEAD OF
// MESSING WITH THE HARDWARE, BUT ONLY IF KEYBOARD HANDLERS WERE STARTED
keymatrix keyb_getmatrix()
{
    if(keyflags & KF_RUNNING)
        return kmat;
    else
        return keyb_irq_getmatrix();
}

// ANALYZE CHANGES IN THE KEYBOARD STATUS AND POST MESSAGES ACCORDINGLY

void keyb_irq_int_handler()
{

    *EINTMASK |= 0xff00;

    keyb_irq_update();

    *EINTPEND |= 0xff00;
    *EINTMASK &= ~0xff00;

}

void keyb_irq_init()
{
    keyflags = KF_RUNNING;
    keyplane = 0;
    kused = kcurrent = 0;
    keynumber = 0;
    kmat = 0LL;
    keyb_irq_repeattime = 100 / KEYB_SCANSPEED;
    keyb_irq_longpresstime = 1000 / KEYB_SCANSPEED;
    keyb_irq_debounce = 20 / KEYB_SCANSPEED;
    keyb_irq_lock = 0;
// INITIALIZE TIMER EVENT 0

    tmr_events[0].eventhandler = keyb_irq_update;
    tmr_events[0].delay = (KEYB_SCANSPEED * tmr_getsysfreq()) / 1000;

    tmr_events[0].status = 0;

// MASK ALL EXTINT UNTIL THEY ARE PROPERLY PROGRAMMED
    *INTMSK1 |= 0x20;

    *GPDCON= (*GPDCON &0xffff0000) | 0x5555;    // ALL COLUMNS TO OUTPUT
    *GPDDAT |= 0xFF;        // DRIVE OUTPUTS HIGH
    *GPDUDP = (*GPDUDP &0xffff0000);   // PULL UP/DOWN DISABLE
    //*GPGUDP = 0;       // DISABLE PULLDOWN ON ALL INPUT LINES
    //*GPDUDP = (*GPDUDP &0xffff0000) | 0x5555;   // PULL DOWN ENABLE ON ALL OUTPUTS (TEMPORARILY SET TO INPUTS DURING SCAN)
    *GPGUDP = 0x5555;       // ENABLE PULLDOWN ON ALL INPUT LINES
    *GPGCON = 0xaaaa;       // ALL ROWS TO EINT



    *EXTINT1 = 0x66666666;      // ALL OTHER KEYS TRIGGER ON BOTH EDGES
    *EINTMASK = (*EINTMASK & ~0x00ff00);       // UNMASK 8-15
    *EINTPEND = 0xfff0;     // CLEAR ALL PENDING INTERRUPTS

    irq_addhook(5, &keyb_irq_int_handler);

    irq_clrpending(5);    // REMOVE ANY PENDING REQUESTS
    irq_unmask(5);     // UNMASK EXTERNAL INTERRUPTS

}

void keyb_irq_stop()
{

    tmr_events[0].status = 0;

// DISABLE INTERRUPTS, STATUS WILL BE FULLY RESTORED ON EXIT
    irq_mask(5);     // MASK EXTERNAL INTERRUPTS
    *EINTMASK |= 0xFF00;    // MASK GPIO INTERRUPT REGISTERS
    irq_releasehook(5);

    keyflags &= ~KF_RUNNING;
}
