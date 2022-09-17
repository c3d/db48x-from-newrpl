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
extern int keyb_irq_repeattime, keyb_irq_longpresstime SYSTEM_GLOBAL,
        keyb_irq_debounce SYSTEM_GLOBAL;

// QT-BASED KEYBOARD MESSAGES MUST UPDATE THIS MATRIX;
volatile keymatrix pckeymatrix;
// QT-BASED TERMINATE MESSAGE COMES IN THIS VARIABLE
extern volatile int pc_terminate;




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
const unsigned char const keyb_irq_codefrombit[64] = {
     0, KB_P, KB_BKS, KB_M,KB_NUM, KB_9, KB_O, KB_ENT, 8, KB_0,
    KB_D,KB_L,KB_PLT,KB_CAS,KB_8,KB_RT,16,KB_DIV,KB_C,KB_K,
    KB_SYM,KB_MEN,KB_7,23,24,KB_MUL,KB_ALPHA,KB_J,KB_HOM,KB_VIE,
    KB_6,31,32,KB_SUB,KB_E,KB_I,KB_APP,KB_UP,KB_5,39,
    40,KB_ADD,KB_A,KB_H,KB_DN,KB_1,KB_4,47,48,KB_SPC,
    KB_G,KB_LSHIFT,KB_ON,KB_DOT,KB_3,55,56,KB_LF,KB_B,KB_F,
    KB_N,KB_HLP,KB_2, KB_RSHIFT
};
const unsigned char const keyb_irq_bitfromcode[64] = {


//    BKS U   /   *   -  +  ENT    P
    0, 2, 0, 17, 25, 33, 41, 7, 0, 1,

//  T  Y  9  6    3  SPC     O  S  X
    0, 0, 5, 60, 54, 49, 0,  6, 0, 0,

//  8   5   2   .      N   R   W  7   4
   14, 38, 62, 53, 0, 60,  0,  0, 22, 46,

//  1  0      M   Q   V
   45, 9, 0,  3,  0,  0, 0, 0, 0, 0,

//    A    B   C   D   E   F   G     UP
   0, 42, 58, 18, 10, 34, 59, 50, 0, 37,

// LF  DN  RT  H    I   J      K   L
   57, 44, 15, 43, 35, 27, 0, 19, 11, 0,

// AL  LS  RS  ON
   26, 51, 63, 52
};




















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


