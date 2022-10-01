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

#define DEBOUNCE  48    // 10 SEEMS TO BE ADEQUATE EVEN AT 75 MHz

// KEYBOARD, LOW LEVEL GLOBAL VARIABLES
extern unsigned short int keyb_irq_buffer[KEYB_BUFFER];
extern volatile int       keyb_irq_lock;
extern int                keyflags;
extern int                kused, kcurrent;
extern keymatrix          kmat;
extern int                keyplane;
extern int                keynumber, keycount;
extern int                keyb_irq_repeattime;
extern int                keyb_irq_longpresstime;
extern int                keyb_irq_debounce;

// LOW-LEVEL ROUTINE TO BE USED BY THE IRQ HANDLERS AND EXCEPTION
// HANDLERS ONLY

keymatrix keyb_irq_getmatrix()
{
    unsigned int c;
    uint32_t v;

    unsigned int lo = 0, hi = 0;

    int col;
    unsigned int control;

    for(col = 7; col >= 4; --col) {

        control = 1 << ((col + 8) * 2);
        control = control | 0x1;
        *GPGDAT = 0;    // DRIVE THE OUTPUT COLUMN LOW
        *GPGCON = control;

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

        hi = (hi << 8) | ((~(v)) & 0xfe);

    }

    for(; col >= 0; --col) {

        control = 1 << ((col + 8) * 2);
        control = control | 0x1;
        *GPGDAT = 0;    // DRIVE THE OUTPUT COLUMN LOW
        *GPGCON = control;

        // GPGDAT WAS SET TO ZERO, SO THE SELECTED COLUMN IS DRIVEN LOW

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

        lo = (lo << 8) | ((~(v)) & 0xfe);

    }

    *GPGCON = 0x5555AAA9;       // SET TO TRIGGER INTERRUPTS ON ANY KEY
    *GPGDAT = 0;        // DRIVE ALL OUTPUT COLUMNS LOW

    volatile unsigned int *GPFDAT = ((unsigned int *) (IO_REGS + 0x54));

    c = 0;
    while (c < DEBOUNCE) {
        uint32_t nv = *GPFDAT & 0x71;
        if (nv == v) {
            ++c;
        } else {
            c = 0;
            v = nv;
        }
    }

    hi |= (v & 0x70) << 24;
    hi |= v << 31;

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

    *EINTMASK |= 0xfe70;

    keyb_irq_update();

    *EINTPEND |= 0xfe70;
    *EINTMASK &= ~0xfe70;

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
    *INTMSK |= 0x31;

    *GPGCON = 0x5555AAA9;       // DRIVE ALL COLUMNS TO OUTPUT, ROWS TO EINT
    *GPGDAT = 0;        // DRIVE OUTPUTS LOW
    *GPGUP = 0x1;       // ENABLE PULLUPS ON ALL INPUT LINES, DISABLE ON ALL OUTPUTS
//keysave[1]=*GPFCON;
    *GPFCON = (*GPFCON & 0xffffc0fc) | 0x2a02;  // SET ALL SHIFTS TO GENERATE INTERRUPTS

    irq_addhook(5, &keyb_irq_int_handler);
    irq_addhook(4, &keyb_irq_int_handler);      // SHIFTS
    irq_addhook(0, &keyb_irq_int_handler);      // ON

    *EXTINT0 = (*EXTINT0 & 0xf000fff0) | 0x06660006;    // ALL SHIFTS TRIGGER ON BOTH EDGES
    *EXTINT1 = 0x66666666;      // ALL OTHER KEYS TRIGGER ON BOTH EDGES
    *EINTMASK = (*EINTMASK & 0x00ff018f);       // UNMASK 4,5,6 AND 9-15
    *EINTPEND = 0xffffffff;     // CLEAR ALL PENDING INTERRUPTS
    *INTMSK = *INTMSK & 0xffffffce;     // UNMASK EXTERNAL INTERRUPTS
    *SRCPND |= 0x31;
    *INTPND |= 0x31;    // UNMASK EXTERNAL INTERRUPTS

}

void keyb_irq_stop(unsigned int *keysave)
{

    tmr_events[0].status = 0;

// DISABLE INTERRUPTS, STATUS WILL BE FULLY RESTORED ON EXIT
    *INTMSK |= 0x31;
    *EINTMASK |= 0xFE70;
    irq_releasehook(5);
    irq_releasehook(4);
    irq_releasehook(0);

// RESTORE IO PORT CONFIGURATION
    *GPGCON = keysave[0];
    *GPFCON = keysave[1];
    keyflags &= ~KF_RUNNING;
}
