/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <ui.h>
#include <keyboard.h>


// Qt event processing
extern void thread_processevents();

// Qt-based terminate message comes in this variable
extern volatile int pc_terminate;

// Keys set by the PC to simulate hardware reading of the keyboard matrix
keymatrix pc_key_matrix = 0;



keymatrix keyb_irq_get_matrix()
// ----------------------------------------------------------------------------
//   Stub for the low-level keyboard routine
// ----------------------------------------------------------------------------
{
    thread_processevents();
    return pc_key_matrix;
}


void keyb_irq_init()
// ----------------------------------------------------------------------------
//   Initialize the PC keyboard subsystem
// ----------------------------------------------------------------------------
{
    keyb_flags                 = 0;
    keyb_plane                 = 0;
    keyb_used                  = 0;
    keyb_current               = 0;
    keyb_last_code             = 0;
    keyb_prev_code             = 0;
    keyb_matrix                = 0LL;
    keyb_irq_repeat_time       = 50 / KEYB_SCAN_SPEED;
    keyb_irq_long_press_time   = 800 / KEYB_SCAN_SPEED;
    keyb_irq_debounce          = 0; // 20/KEYB_SCAN_SPEED;
    keyb_irq_lock              = 0;

    pc_key_matrix              = 0;

    // Initialize timer event 0
    tmr_events[0].eventhandler = keyb_irq_update;
    tmr_events[0].delay        = (KEYB_SCAN_SPEED * tmr_getsysfreq()) / 1000;

    tmr_events[0].status       = 0;

    keyb_flags                 = KFLAG_RUNNING;
}


void keyb_irq_stop(unsigned int *keysave)
// ----------------------------------------------------------------------------
//    Shutdown keyboard subsystem - Dead code (not called)
// ----------------------------------------------------------------------------
{
    UNUSED(keysave);

    tmr_events[0].status = 0;

    keyb_flags &= ~KFLAG_RUNNING;
}
