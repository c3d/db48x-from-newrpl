/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <ui.h>

INTERRUPT_TYPE cpu_intoff_nosave();
void cpu_inton_nosave(INTERRUPT_TYPE state);
void tmr_eventreschedule();

// KEYBOARD, LOW LEVEL GLOBAL VARIABLES
unsigned int keyb_irq_buffer[KEYB_BUFFER] SYSTEM_GLOBAL;
volatile int keyb_irq_lock SYSTEM_GLOBAL;
int keyflags SYSTEM_GLOBAL;
int kused SYSTEM_GLOBAL, kcurrent SYSTEM_GLOBAL;
keymatrix kmat SYSTEM_GLOBAL;
unsigned int keyplane SYSTEM_GLOBAL;
int keynumber SYSTEM_GLOBAL, keycount SYSTEM_GLOBAL;
int keyb_irq_repeattime SYSTEM_GLOBAL, keyb_irq_longpresstime SYSTEM_GLOBAL,
        keyb_irq_debounce SYSTEM_GLOBAL;

// LOW-LEVEL ROUTINE TO BE USED BY THE IRQ HANDLERS AND EXCEPTION
// HANDLERS ONLY

keymatrix keyb_irq_getmatrix();

// WRAPPER TO DISABLE INTERRUPTS WHILE READING THE KEYBOARD
// NEEDED ONLY WHEN CALLED FROM WITHIN AN EXCEPTION HANDLER

keymatrix keyb_irq_getmatrixEX();

void keyb_irq_waitrelease();

// RETURNS THE CURRENT WORKING MATRIX INSTEAD OF
// MESSING WITH THE HARDWARE, BUT ONLY IF KEYBOARD HANDLERS WERE STARTED
keymatrix keyb_getmatrix();

// LOW-LEVEL FUNCTION TO BE USED BY THE
// EXCEPTION SUBSYSTEM ONLY

const unsigned short const keyb_irq_shiftconvert[8] = {
    0,
    SHIFT_ALPHA | SHIFT_ALPHAHOLD,
    SHIFT_LS | SHIFT_LSHOLD,
    SHIFT_ALPHA | SHIFT_ALPHAHOLD | SHIFT_LS | SHIFT_LSHOLD,
    SHIFT_RS | SHIFT_RSHOLD,
    SHIFT_ALPHA | SHIFT_ALPHAHOLD | SHIFT_RS | SHIFT_RSHOLD,
    SHIFT_LS | SHIFT_LSHOLD | SHIFT_RS | SHIFT_RSHOLD,
    SHIFT_LS | SHIFT_LSHOLD | SHIFT_ALPHA | SHIFT_ALPHAHOLD | SHIFT_RS |
            SHIFT_RSHOLD
};

int keyb_irq_getkey(int wait)
{

    keymatrix m,m_noshift;
    m = keyb_irq_getmatrixEX();

    if(wait) {
        // wait for a non-shift key to be pressed
        while((m & (~KEYMATRIX_ALL_SHIFTS)) == 0LL)
            m = keyb_irq_getmatrixEX();
    }

    int kcode,kcodebit, shft = KEYMATRIX_ALPHABIT(m) | (KEYMATRIX_LSHIFTBIT(m)<<1) | (KEYMATRIX_RSHIFTBIT(m)<<2);

    m_noshift=m & (~KEYMATRIX_ALL_SHIFTS);
    unsigned char *mbytes = (unsigned char *)&m_noshift;
    int k;
    for(k = 0, kcodebit = 0; k < 8; ++mbytes, ++k, kcodebit += 8) {
        if(*mbytes != 0) {
            k = *mbytes;
            while(!(k & 1)) {
                k >>= 1;
                ++kcodebit;
            }
            break;
        }
    }
    kcode=KEYMAP_CODEFROMBIT(kcodebit);

    if(wait) {
        while((m & (~KEYMATRIX_ALL_SHIFTS)) != 0)
            m = keyb_irq_getmatrixEX();
    }

    if(kcodebit < 64)
              return kcode | keyb_irq_shiftconvert[shft];
    return 0;

}

#define LONG_KEYPRESSTIME (keyb_irq_longpresstime)
#define REPEAT_KEYTIME (keyb_irq_repeattime)
#define BOUNCE_KEYTIME (keyb_irq_debounce)

#define KF_RUNNING   1
#define KF_ALPHALOCK 2
#define KF_NOREPEAT  4
#define KF_UPDATED   8

void keyb_irq_postmsg(unsigned int msg)
{

    keyb_irq_buffer[kcurrent] = msg;
    kcurrent = (kcurrent + 1) & (KEYB_BUFFER - 1);
// CHECK FOR BUFFER OVERRUN
    if(kcurrent == kused) {
        // BUFFER OVERRUN, DROP LAST KEY
        kcurrent = (kcurrent - 1) & (KEYB_BUFFER - 1);
    }

}

void keyb_postmsg(unsigned int msg)
{
    // WARNING: PROBLEMS MAY ARISE IF THE INTERRUPT SERVICE WANTS
    // TO POST A MESSAGE WHILE THE USER IS POSTING ONE.
    while(cpu_getlock(1, &keyb_irq_lock));

    keyb_irq_postmsg(msg);
    keyb_irq_lock = 0;

}

unsigned int keyb_getmsg()
{
    if(kused == kcurrent)
        return 0;
    unsigned int msg = keyb_irq_buffer[kused];
    kused = (kused + 1) & (KEYB_BUFFER - 1);
    return msg;
}

// CHECK IF ANY AVAILABLE KEYSTROKES
int keyb_anymsg()
{
    if(kused == kcurrent)
        return 0;
    return 1;
}

// FLUSH KEYBOARD BUFFER
void keyb_flush()
{
    while(keyb_getmatrix() != 0LL);
    kused = kcurrent;
}

// FLUSH KEYBOARD BUFFER WITHOUT WAITING
void keyb_flushnowait()
{
    kused = kcurrent;
}

// RETURN TRUE IF AN UPDATE HAPPENED
// USED TO DETECT IF AN INTERRUPT WAS DUE TO THE KEYBOARD
int keyb_wasupdated()
{
    int k = keyflags & KF_UPDATED;

    keyflags ^= k;
    return k;
}

// ANALYZE CHANGES IN THE KEYBOARD STATUS AND POST MESSAGES ACCORDINGLY
#define ALPHALOCK   (SHIFT_ALPHA<<17)
#define OTHER_KEY   (SHIFT_ALPHA<<18)
#define ONE_PRESS   (SHIFT_ALPHA<<19)
#define ALPHASWAP   (SHIFT_ALPHA<<20)

void keyb_irq_update()
{

    if(cpu_getlock(1, &keyb_irq_lock))
        return;

    keymatrix a, b;

    keyflags |= KF_UPDATED;
  doupdate:

    a = keyb_irq_getmatrix();
    b = a ^ kmat;
    kmat = a;
    //****** DEBUG
    /*
    // PRINT KEYBOARD MATRIX
    gglsurface scr;
    ggl_init_screen(&scr);
    int k;
    // DRAW THE KEYMATRIX
    for(k=0;k<64;++k)
        ggl_rect(&scr, LCD_W-4*(k+2), 0, LCD_W-4*(k+1),4 ,
                   (a&(1LL<<k))? 0xffffffff:0x44444444);

    */
    //************ END DEBUG
    // ANALYZE CHANGES
    if(b != 0) {
        // POST MESSAGE
        int key = 0;        // KEY HAS THE BIT POSITION OF A KEY, NOT THE KEYCODE
        while(b != 0) {
            if(b & 1) {
                if(a & 1) {
                    // POST KEYDN MESSAGE
                    if(keynumber == -key) {
                        // DISREGARD SPURIOUS KEYPRESS
                        kmat &= ~(1LL << key);        // CLEAR THE KEY
                        keynumber = 0;
                        keycount = 0;
                    }
                    else {
                        keyb_irq_postmsg(KM_KEYDN + KEYMAP_CODEFROMBIT(key));
                        if( (KEYMAP_CODEFROMBIT(key) < 60) || ((KEYMAP_CODEFROMBIT(key) == KB_ALPHA) && (keyplane & (SHIFT_RS | SHIFT_LS)))     // TREAT SHIFT-ALPHA LIKE REGULAR KEYPRESS
                                 || ((KEYMAP_CODEFROMBIT(key) == KB_LSHIFT) && (keyplane & (SHIFT_ONHOLD)))                                     // TRAT ON-HOLD + SHIFT AS A REGULAR KEYPRESS
                                || ((KEYMAP_CODEFROMBIT(key) == KB_RSHIFT) && (keyplane & (SHIFT_ONHOLD)))                                     // TRAT ON-HOLD + SHIFT AS A REGULAR KEYPRESS
                                )
                        {
                            keyb_irq_postmsg(KM_PRESS + KEYMAP_CODEFROMBIT(key) +
                                    (keyplane & SHIFT_ANY));
                            keynumber = key;
                            keycount = 0;

                            keyplane &= ~ONE_PRESS;

                        }
                        else {
                            unsigned int oldplane = keyplane;
                            if(KEYMAP_CODEFROMBIT(key) == KB_LSHIFT) {
                                keyplane &=
                                        ~(SHIFT_RSHOLD | SHIFT_RS | (SHIFT_RS <<
                                            16));
                                keyplane |= SHIFT_LSHOLD | SHIFT_LS;
                                keyplane ^= SHIFT_LS << 16;
                            }
                            if(KEYMAP_CODEFROMBIT(key) == KB_RSHIFT) {
                                keyplane &=
                                        ~(SHIFT_LSHOLD | SHIFT_LS | (SHIFT_LS <<
                                            16));
                                keyplane |= SHIFT_RSHOLD | SHIFT_RS;
                                keyplane ^= SHIFT_RS << 16;
                            }
                            if(KEYMAP_CODEFROMBIT(key) == KB_ALPHA) {
                                keyplane &= ~OTHER_KEY;
                                if(keyplane & SHIFT_ALPHA) {
                                    // ALREADY IN ALPHA MODE
                                    keyplane |= ALPHASWAP;
                                }
                                else {
                                    keyplane &= ~ONE_PRESS;
                                }
                                keyplane |= SHIFT_ALPHAHOLD | SHIFT_ALPHA;

                            }
                            if(KEYMAP_CODEFROMBIT(key) == KB_ON) {
                                keyplane &= ~OTHER_KEY;
                                keyplane |= SHIFT_ONHOLD;
                            }
                            // THE KM_SHIFT MESSAGE CARRIES THE OLD PLANE IN THE KEY CODE
                            // AND THE NEW PLANE IN THE SHIFT CODE.
                            keyb_irq_postmsg(KM_SHIFT | (keyplane & SHIFT_ANY) |
                                    MKOLDSHIFT(oldplane | ((oldplane &
                                                ALPHALOCK) >> 16)));

                        }
                    }
                }
                else {

                    if(KEYMAP_CODEFROMBIT(key) == KB_ON) {      // SPECIAL CASE OF THE ON KEY PRESSED AND RELEASED
                         if(!(keyplane & OTHER_KEY)) {
                             // NO OTHER KEY WAS PRESSED , ONLY ON WAS PRESSED AND RELEASED
                             // POST A LATE PRESS MESSAGE FOR THE ON KEY
                             keyb_irq_postmsg(KM_PRESS + KEYMAP_CODEFROMBIT(key) +
                                     (keyplane & (SHIFT_ANY) & ~(SHIFT_ONHOLD)));

                         }
                    }


                    keyb_irq_postmsg(KM_KEYUP + KEYMAP_CODEFROMBIT(key));



                    if((KEYMAP_CODEFROMBIT(key) < 60) || ((KEYMAP_CODEFROMBIT(key) == KB_ALPHA) && (keyplane & (SHIFT_RS | SHIFT_LS)))) {
                        if(keynumber > 0)
                            keynumber = -keynumber;
                        keycount = -BOUNCE_KEYTIME;
                        keyplane &= ~((SHIFT_LS | SHIFT_RS) << 16);

                        if(!(keyplane & (SHIFT_HOLD | SHIFT_ALHOLD |
                                        SHIFT_ONHOLD))) {
                            unsigned int oldkeyplane = keyplane;
                            keyplane &= ~(SHIFT_LS | SHIFT_RS | SHIFT_ALPHA); // KILL ALL SHIFT PLANES
                            if(keyplane & ALPHALOCK)
                                keyplane |= SHIFT_ALPHA;      // KEEP ALPHA IF LOCKED
                            keyplane &= ~((SHIFT_ALPHA) << 16);

                            if(oldkeyplane != keyplane)
                                keyb_irq_postmsg(KM_SHIFT | (keyplane &
                                            SHIFT_ANY) | MKOLDSHIFT(oldkeyplane
                                            | ((oldkeyplane & ALPHALOCK) >>
                                                16)));
                        }
                        else {

                            if(keyplane & (SHIFT_ALPHA|SHIFT_ONHOLD)) {
                                // THIS IS A PRESS AND HOLD KEY BEING RAISED
                                keyplane |= OTHER_KEY;
                            }
                            if(!(keyplane & (SHIFT_HOLD))) {
                                unsigned int oldkeyplane = keyplane;
                                // IT WAS ALPHA-HOLD OR ON-HOLD, KILL SHIFTS
                                keyplane &= ~(SHIFT_LS | SHIFT_RS);   // KILL ALL SHIFT PLANES

                                if(oldkeyplane != keyplane)
                                    keyb_irq_postmsg(KM_SHIFT | (keyplane &
                                                SHIFT_ANY) |
                                            MKOLDSHIFT(oldkeyplane |
                                                ((oldkeyplane & ALPHALOCK) >>
                                                    16)));
                            }
                        }

                    }
                    else {
                        unsigned int oldkeyplane = keyplane;
                        if(KEYMAP_CODEFROMBIT(key) == KB_LSHIFT) {
                            keyplane &=
                                    ~((SHIFT_LSHOLD | SHIFT_LS) ^ ((keyplane
                                            >> 16) & SHIFT_LS));
                            if(!(oldkeyplane & SHIFT_ALHOLD))
                                keyplane &=
                                        ~((SHIFT_ALPHA) ^ (((keyplane >> 16) |
                                                (keyplane >> 17)) &
                                            SHIFT_ALPHA));

                        }
                        if(KEYMAP_CODEFROMBIT(key) == KB_RSHIFT) {
                            keyplane &=
                                    ~((SHIFT_RSHOLD | SHIFT_RS) ^ ((keyplane
                                            >> 16) & SHIFT_RS));
                            if(!(oldkeyplane & SHIFT_ALHOLD))
                                keyplane &=
                                        ~((SHIFT_ALPHA) ^ (((keyplane >> 16) |
                                                (keyplane >> 17)) &
                                            SHIFT_ALPHA));

                        }
                        if(KEYMAP_CODEFROMBIT(key) == KB_ALPHA) {
                            if(keyplane & ALPHASWAP) {
                                // ALPHA WAS PRESSED WHILE ALREADY IN ALPHA MODE
                                if(keyplane & OTHER_KEY) {
                                    // ANOTHER KEY WAS PRESSED BEFORE RELEASING ALPHA
                                    keyplane &= ~ONE_PRESS;

                                }
                                else {
                                    // ALPHA WAS PRESSED AND RELEASED, NO OTHER KEYS
                                    if(keyplane & ONE_PRESS) {
                                        // THIS IS THE SECOND PRESS, KILL ALPHA MODE
                                        keyplane &= ~ALPHALOCK;
                                        keyplane &= ~SHIFT_ALPHA;
                                    }
                                    else
                                        keyplane |= ONE_PRESS;

                                    // SEND MESSAGE THAT ALPHA MODE CYCLING WAS REQUESTED
                                    keyb_irq_postmsg(KM_PRESS + KEYMAP_CODEFROMBIT(key) +
                                            (keyplane & SHIFT_ANY));
                                    keynumber = key;
                                    keycount = 0;

                                }
                                keyplane &= ~ALPHASWAP;

                            }
                            else {
                                // ALPHA WAS PRESSED FOR THE FIRST TIME FROM OTHER MODE
                                if(keyplane & OTHER_KEY) {

                                    keyplane &= ~SHIFT_ALPHAHOLD;
                                }
                                else {
                                    // ALPHA WAS PRESSED AND RELEASED
                                    keyplane |= ALPHALOCK;

                                }
                                keyplane &= ~ONE_PRESS;

                            }

                            keyplane &= ~SHIFT_ALHOLD;

                        }
                        if(KEYMAP_CODEFROMBIT(key) == KB_ON) {
                            keyplane &= ~SHIFT_ONHOLD;
                            keyplane &= ~OTHER_KEY;
                        }
                        keyb_irq_postmsg(KM_SHIFT | (keyplane & SHIFT_ANY) |
                                MKOLDSHIFT(oldkeyplane | ((oldkeyplane &
                                            ALPHALOCK) >> 16)));

                        keynumber = -key;
                        keycount = -BOUNCE_KEYTIME;
                    }

                }
            }
            b >>= 1;
            a >>= 1;
            ++key;
        }
    }
    // ANALYZE STATUS OF CURRENT KEYPRESS
    if(keynumber >= 0) {
        if(kmat & (1LL << keynumber)) {
            // KEY STILL PRESSED, INCREASE COUNTER
            ++keycount;
            if((keycount > LONG_KEYPRESSTIME)) {
                //if(!(keyflags&KF_NOREPEAT)) {
                // ONLY CERTAIN KEYS WILL AUTOREPEAT
                switch (KEYMAP_CODEFROMBIT(keynumber)) {
                case KB_SPC:
                case KB_BKS:
                    if(keyplane & (SHIFT_LS | SHIFT_RS | SHIFT_HOLD |
                                SHIFT_ALHOLD)) {
                        keyb_irq_postmsg(KM_LPRESS | KEYMAP_CODEFROMBIT(keynumber) | (keyplane &
                                    SHIFT_ANY));
                        keycount = -LONG_KEYPRESSTIME;
                        break;
                    }
                    // OTHERWISE DO REPEAT
                case KB_UP:
                case KB_DN:
                case KB_LF:
                case KB_RT:
                    // THESE ALWAYS REPEAT, EVEN SHIFTED
                    keyb_irq_postmsg(KM_REPEAT | KEYMAP_CODEFROMBIT(keynumber) | (keyplane &
                                SHIFT_ANY));
                    keycount = -REPEAT_KEYTIME;
                    break;
                default:
                    // DO NOT AUTOREPEAT, DO LONG PRESS
                    keyb_irq_postmsg(KM_LPRESS | KEYMAP_CODEFROMBIT(keynumber) | (keyplane &
                                SHIFT_ANY));
                    keycount = -LONG_KEYPRESSTIME;
                }

            }

            if(!keycount) {

                switch (KEYMAP_CODEFROMBIT(keynumber)) {
                case KB_SPC:
                case KB_BKS:
                    if(keyplane & (SHIFT_LS | SHIFT_RS | SHIFT_HOLD |
                                SHIFT_ALHOLD)) {
                        keyb_irq_postmsg(KM_LREPEAT | KEYMAP_CODEFROMBIT(keynumber) | (keyplane &
                                    SHIFT_ANY));
                        keycount -= LONG_KEYPRESSTIME;
                        break;
                    }
                    // OTHERWISE DO REPEAT
                case KB_UP:
                case KB_DN:
                case KB_LF:
                case KB_RT:
                    // THESE ALWAYS REPEAT, EVEN SHIFTED
                    keyb_irq_postmsg(KM_REPEAT | KEYMAP_CODEFROMBIT(keynumber) | (keyplane &
                                SHIFT_ANY));
                    keycount -= REPEAT_KEYTIME;
                    break;
                default:
                    // DO NOT AUTOREPEAT, DO LONG PRESS
                    keyb_irq_postmsg(KM_LREPEAT | KEYMAP_CODEFROMBIT(keynumber) | (keyplane &
                                SHIFT_ANY));
                    keycount -= LONG_KEYPRESSTIME;
                }

            }

        }
    }

    // REPEATER
    if(kmat == 0) {
        if(keycount >= 0) {
            tmr_events[0].status = 0;
            keynumber = 0;
        }
        else {
            ++keycount;
        }
    }
    else {

        if(!(tmr_events[0].status & 1)) {
            // ACTIVATE THE TIMER EVENT IF NOT ALREADY RUNNING
            tmr_events[0].ticks = tmr_ticks() + tmr_events[0].delay;
            tmr_events[0].status = 3;
            tmr_eventreschedule();
        }
    }

    // On-C and On-A-F handling

    if(kmat == ((1ULL << KEYMAP_BITFROMCODE(KB_ON)) | (1ULL << KEYMAP_BITFROMCODE(KB_A)) | (1ULL << KEYMAP_BITFROMCODE(KB_F)))) {
        // ON-A-F pressed, offer the option to stop the program

        keyb_irq_lock = 0;

        throw_exception("User BREAK requested",
                EX_CONT | EX_WARM | EX_WIPEOUT | EX_RESET |
                EX_RPLREGS);

        //  AFTER RETURNING FROM THE EXCEPTION HANDLER, ALL KEYS ARE GUARANTEED TO BE RELEASED
        //  DO AN UPDATE TO SEND KEY_UP MESSAGES TO THE APPLICATION AND CORRECT SHIFT PLANES
        goto doupdate;

    }

    if(kmat == ((1ULL << KEYMAP_BITFROMCODE(KB_ON)) | (1ULL << KEYMAP_BITFROMCODE(KB_A)) | (1ULL << KEYMAP_BITFROMCODE(KB_C)))) {
        // ON-A-C pressed, offer the option to stop the program

        keyb_irq_lock = 0;

        throw_exception("RPL Break requested",
                EX_CONT | EX_RPLEXIT | EX_WARM | EX_RESET);

        //  AFTER RETURNING FROM THE EXCEPTION HANDLER, ALL KEYS ARE GUARANTEED TO BE RELEASED
        //  DO AN UPDATE TO SEND KEY_UP MESSAGES TO THE APPLICATION AND CORRECT SHIFT PLANES
        goto doupdate;

    }

    keyb_irq_lock = 0;

}

void keyb_settiming(int repeat, int longpress, int debounce)
{
    keyb_irq_repeattime = (repeat + KEYB_SCANSPEED - 1) / KEYB_SCANSPEED;
    keyb_irq_longpresstime = (longpress + KEYB_SCANSPEED - 1) / KEYB_SCANSPEED;
    keyb_irq_debounce = (debounce + KEYB_SCANSPEED - 1) / KEYB_SCANSPEED;
}

void keyb_setrepeat(int repeat)
{
    if(!repeat)
        keyflags |= KF_NOREPEAT;
    else
        keyflags &= ~KF_NOREPEAT;
}

void keyb_setalphalock(int single_alpha_lock)
{
    if(single_alpha_lock)
        keyflags |= KF_ALPHALOCK;
    else
        keyflags &= ~KF_ALPHALOCK;

}

void keyb_setshiftplane(int leftshift, int rightshift, int alpha, int alphalock)
{
//    while(keyb_getmatrix()!=0LL) ;            // WAIT UNTIL NO MORE KEYS ARE PRESSED TO UPDATE SHIFT STATE

    int oldplane = keyplane;

    if(leftshift)
        keyplane |= SHIFT_LS | (SHIFT_LS << 16);
    else
        keyplane &= ~(SHIFT_LS | (SHIFT_LS << 16));
    if(rightshift)
        keyplane |= SHIFT_RS | (SHIFT_RS << 16);
    else
        keyplane &= ~(SHIFT_RS | (SHIFT_RS << 16));
    if(alpha || alphalock)
        keyplane |= SHIFT_ALPHA | (SHIFT_ALPHA << 16);
    else
        keyplane &= ~(SHIFT_ALPHA | (SHIFT_ALPHA << 16));
    if(alphalock) {
        keyplane |= SHIFT_ALPHA << 17;        // LOCK ALPHA
        keyplane &= ~(SHIFT_ALPHA << 16);
    }
    else {
        keyplane &= ~(SHIFT_ALPHA << 17);
    }
    keyb_postmsg(KM_SHIFT | (keyplane & SHIFT_ANY) | MKOLDSHIFT(oldplane |
                ((oldplane & ALPHALOCK) >> 16)));

}

unsigned int keyb_getshiftplane()
{
    return keyplane &SHIFT_ANY;
}
