/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

extern INTERRUPT_TYPE __cpu_intoff();
extern void __cpu_inton(INTERRUPT_TYPE state);
extern void __tmr_eventreschedule();

extern void __keyb_update();

#define DEBOUNCE  16    // 10 SEEMS TO BE ADEQUATE EVEN AT 75 MHz

// KEYBOARD, LOW LEVEL GLOBAL VARIABLES
extern unsigned short int __keyb_buffer[KEYB_BUFFER];
extern volatile int __keyb_lock;
extern int __keyflags;
extern int __kused, __kcurrent;
extern keymatrix __kmat;
extern int __keyplane;
extern int __keynumber, __keycount;
extern int __keyb_repeattime, __keyb_longpresstime, __keyb_debounce;

// LOW-LEVEL ROUTINE TO BE USED BY THE IRQ HANDLERS AND EXCEPTION
// HANDLERS ONLY

keymatrix __keyb_getmatrix()
{
    unsigned int c;
    uint32_t v;

    unsigned int lo = 0, hi = 0;

    int col;
    unsigned int control;

    *GPGCON=0;    // ALL INPUTS


    for(col = 7; col >= 4; --col) {

        *GPDDAT = (*GPDDAT & (~0xff)) | (1 << col);

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

      *GPDDAT = (*GPDDAT & (~0xff)) | (1 << col);

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


    *GPDDAT |= 0xff;       // SET ALL COLUMNS TO OUTPUT
    *GPGCON = 0xaaaa;        // ALL KEYS FUNCTION BACK TO EINT

    hi |= v << 31;          // read the on key at GPG0

    return ((keymatrix) lo) | (((keymatrix) hi) << 32);
}

// WRAPPER TO DISABLE INTERRUPTS WHILE READING THE KEYBOARD
// NEEDED ONLY WHEN CALLED FROM WITHIN AN EXCEPTION HANDLER

keymatrix __keyb_getmatrixEX()
{
    INTERRUPT_TYPE saved = __cpu_intoff();
    keymatrix m = __keyb_getmatrix();
    __cpu_inton(saved);
    return m;
}

void __keyb_waitrelease()
{
    keymatrix m = 1;
    while(m != 0LL) {
        m = __keyb_getmatrixEX();
    }
}

#define LONG_KEYPRESSTIME (__keyb_longpresstime)
#define REPEAT_KEYTIME (__keyb_repeattime)
#define BOUNCE_KEYTIME (__keyb_debounce)

#define KF_RUNNING   1
#define KF_ALPHALOCK 2
#define KF_NOREPEAT  4
#define KF_UPDATED   8

// RETURNS THE CURRENT WORKING MATRIX INSTEAD OF
// MESSING WITH THE HARDWARE, BUT ONLY IF KEYBOARD HANDLERS WERE STARTED
keymatrix keyb_getmatrix()
{
    if(__keyflags & KF_RUNNING)
        return __kmat;
    else
        return __keyb_getmatrix();
}

// ANALYZE CHANGES IN THE KEYBOARD STATUS AND POST MESSAGES ACCORDINGLY

void __keyb_int_handler()
{

    *EINTMASK |= 0xff00;

    __keyb_update();

    *EINTPEND |= 0xff00;
    *EINTMASK &= ~0xff00;

}

void __keyb_init()
{
    __keyflags = KF_RUNNING;
    __keyplane = 0;
    __kused = __kcurrent = 0;
    __keynumber = 0;
    __kmat = 0LL;
    __keyb_repeattime = 100 / KEYB_SCANSPEED;
    __keyb_longpresstime = 1000 / KEYB_SCANSPEED;
    __keyb_debounce = 20 / KEYB_SCANSPEED;
    __keyb_lock = 0;
// INITIALIZE TIMER EVENT 0

    tmr_events[0].eventhandler = __keyb_update;
    tmr_events[0].delay = (KEYB_SCANSPEED * tmr_getsysfreq()) / 1000;

    tmr_events[0].status = 0;

// MASK ALL EXTINT UNTIL THEY ARE PROPERLY PROGRAMMED
    *INTMSK1 |= 0x20;

    *GPDCON= (*GPDCON &0xffff0000) | 0x5555;    // ALL COLUMNS TO OUTPUT
    *GPDDAT |= 0xFF;        // DRIVE OUTPUTS HIGH
    *GPDUDP = (*GPDUDP &0xffff0000) | 0x5555;   // PULL DOWN ENABLE ON ALL OUTPUTS (TEMPORARILY SET TO INPUTS DURING SCAN)
    *GPGUDP = 0x5555;       // ENABLE PULLDOWN ON ALL INPUT LINES
    *GPGCON = 0xaaaa;       // ALL ROWS TO EINT



    *EXTINT1 = 0x66666666;      // ALL OTHER KEYS TRIGGER ON BOTH EDGES
    *EINTMASK = (*EINTMASK & ~0x00ff00);       // UNMASK 8-15
    *EINTPEND = 0xfff0;     // CLEAR ALL PENDING INTERRUPTS

    __irq_addhook(5, &__keyb_int_handler);

    __irq_clrpending(5);    // REMOVE ANY PENDING REQUESTS
    __irq_unmask(5);     // UNMASK EXTERNAL INTERRUPTS

}

void __keyb_stop()
{

    tmr_events[0].status = 0;

// DISABLE INTERRUPTS, STATUS WILL BE FULLY RESTORED ON EXIT
    __irq_mask(5);     // MASK EXTERNAL INTERRUPTS
    *EINTMASK |= 0xFF00;    // MASK GPIO INTERRUPT REGISTERS
    __irq_releasehook(5);

    __keyflags &= ~KF_RUNNING;
}
