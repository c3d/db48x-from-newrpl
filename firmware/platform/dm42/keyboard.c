/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>
#include <keyboard.h>

extern INTERRUPT_TYPE cpu_intoff_nosave();
extern void           cpu_inton_nosave(INTERRUPT_TYPE state);
extern void           tmr_event_reschedule();

extern void           keyb_irq_update();

#define DEBOUNCE 16 // 10 SEEMS TO BE ADEQUATE EVEN AT 75 MHz


keymatrix keyb_irq_get_matrix()
// ----------------------------------------------------------------------------
//   Low-level routine to fetch keyboard data from the hardware
// ----------------------------------------------------------------------------
{
    // REVISIT: Add code to fetch information from DMCP
    return 0;
}


void keyb_irq_int_handler()
// ----------------------------------------------------------------------------
// Analyze changes in the keyboard status and post messages accordingly
// ----------------------------------------------------------------------------
{
    keyb_irq_update();
}


void keyb_irq_init()
// ----------------------------------------------------------------------------
//   Initialize the keyboard interrupt subsystem
// ----------------------------------------------------------------------------
{
    // Initialize variables for interrupt subsystem
    keyb_flags                 = 0;
    keyb_plane                 = 0;
    keyb_used                  = 0;
    keyb_current               = 0;
    keyb_last_code             = 0;
    keyb_prev_code             = 0;
    keyb_matrix                = 0LL;
    keyb_irq_repeat_time       = 100 / KEYB_SCAN_SPEED;
    keyb_irq_long_press_time   = 1000 / KEYB_SCAN_SPEED;
    keyb_irq_debounce          = 20 / KEYB_SCAN_SPEED;
    keyb_irq_lock              = 0;

    // Initialize timer event 0
    tmr_events[0].eventhandler = keyb_irq_update;
    tmr_events[0].delay = (KEYB_SCAN_SPEED * tmr_getsysfreq()) / 1000;
    tmr_events[0].status = 0;

    // Add hooks for keyboard interrupts
    irq_add_hook(5, &keyb_irq_int_handler);

    // Remove any pending interrupt request
    irq_clrpending(5);

    // Unmask external interrupts
    irq_unmask(5);

    // We are now running
    keyb_flags = KFLAG_RUNNING;
}


void keyb_irq_stop()
// ----------------------------------------------------------------------------
//   Stop the keyboard interrupt handling code (dead code, never called)
// ----------------------------------------------------------------------------
{
    // No longer running
    keyb_flags &= ~KFLAG_RUNNING;

    // Disable timer event 0
    tmr_events[0].status = 0;

    // Disable interrupts, status will be fully restored on exit
    irq_mask(5);

    // Mask GPIO interrupt register
    irq_releasehook(5);
}
