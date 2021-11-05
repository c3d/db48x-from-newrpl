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
extern unsigned int __keyb_buffer[KEYB_BUFFER];
extern volatile int __keyb_lock;
extern int __keyflags;
extern int __kused, __kcurrent;
extern keymatrix __kmat;
extern int __keyplane;
extern int __keynumber, __keycount;
extern int __keyb_repeattime, __keyb_longpresstime, __keyb_debounce;


/*

KEYBOARD BIT MAP
----------------
This is the bit number in the 64-bit keymatrix.
Bit set means key is pressed.

    AP]+  SY]+                   HL]+  ES]+
    |36|  |20|                   |61|  |52|
    +--+  +--+                   +--+  +--+

    HM]+  PL]+        UP]+       VW]+  CA]+
    |28|  |12|        |37|       |29|  |13|
    +--+  +--+  LF]+  +--+  RT]+ +--+  +--+
                |57|  DN]+  |15|
          NM]+  +--+  |44|  +--+ ME]+
          |04|        +--+       |21|
          +--+                   +--+

    A]--+  B]--+  C]--+  D]--+  E]--+  BKS]+
    | 42|  | 58|  | 18|  | 10|  | 34|  | 02|
    +---+  +---+  +---+  +---+  +---+  +---+

    F]--+  G]--+  H]--+  I]--+  J]--+  K]--+
    | 59|  | 50|  | 43|  | 35|  | 27|  | 19|
    +---+  +---+  +---+  +---+  +---+  +---+

    L]--+  M]--+  N]--+  O]--+  ENTER]-----+
    | 11|  | 03|  | 60|  | 06|  |    07    |
    +---+  +---+  +---+  +---+  +----------+

    P]--+  7]---+  8]---+  9]---+  /]--+
    | 01|  | 22 |  | 14 |  | 05 |  | 17|
    +---+  +----+  +----+  +----+  +---+

    AL]-+  4]---+  5]---+  6]---+  *]--+
    | 26|  | 46 |  | 38 |  | 30 |  | 25|
    +---+  +----+  +----+  +----+  +---+

    RS]-+  1]---+  2]---+  3]---+  -]--+
    | 51|  | 45 |  | 62 |  | 54 |  | 33|
    +---+  +----+  +----+  +----+  +---+

    ON]-+  0]---+  .]---+  SP]--+  +]--+
    | 63|  | 09 |  | 53 |  | 49 |  | 41|
    +---+  +----+  +----+  +----+  +---+

*/


// Matrix to KeyCode mapping - Defined in keyboard.c for this target
const unsigned char const __keyb_codefrombit[64] = {
     0, KB_P, KB_BKS, KB_M,KB_NUM, KB_9, KB_O, KB_ENT, 8, KB_0,
    KB_D,KB_L,KB_PLT,KB_CAS,KB_8,KB_RT,16,KB_DIV,KB_C,KB_K,
    KB_SYM,KB_MEN,KB_7,23,24,KB_MUL,KB_ALPHA,KB_J,KB_HOM,KB_VIE,
    KB_6,31,32,KB_SUB,KB_E,KB_I,KB_APP,KB_UP,KB_5,39,
    40,KB_ADD,KB_A,KB_H,KB_DN,KB_1,KB_4,47,48,KB_SPC,
    KB_G,KB_LSHIFT,KB_ON,KB_DOT,KB_3,55,56,KB_LF,KB_B,KB_F,
    KB_N,KB_HLP,KB_2, KB_RSHIFT
};
const unsigned char const __keyb_bitfromcode[64] = {


//    BKS U   /   *   -  +  ENT APP P
    0, 2, 0, 17, 25, 33, 41, 7, 36, 1,

//  T  Y  9  6    3  SPC HOM  O  S  X
    0, 0, 5, 60, 54, 49, 28,  6, 0, 0,

//  8   5   2   .  SYM  N   R   W  7   4
   14, 38, 62, 53, 20, 60,  0,  0, 22, 46,

//  1  0  PLT  M   Q   V  HLP VIE MEN ESC
   45, 9, 12,  3,  0,  0, 61, 29, 21, 52,

// CAS  A   B   C   D   E   F   G  NUM UP
   13, 42, 58, 18, 10, 34, 59, 50, 4, 37,

// LF  DN  RT  H    I   J      K   L
   57, 44, 15, 43, 35, 27, 0, 19, 11, 0,

// AL  LS  RS  ON
   26, 51, 63, 52
};












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
    *GPDUDP = (*GPDUDP &0xffff0000);   // PULL UP/DOWN DISABLE
    //*GPGUDP = 0;       // DISABLE PULLDOWN ON ALL INPUT LINES
    //*GPDUDP = (*GPDUDP &0xffff0000) | 0x5555;   // PULL DOWN ENABLE ON ALL OUTPUTS (TEMPORARILY SET TO INPUTS DURING SCAN)
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
