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
    unsigned    hi = 0;
    unsigned    lo = 0;

    // All inputs
    *GPGCON = 0;

    // Loop on high
    for (int col = 7; col >= 0; --col)
    {
        // Only one column set to high, all others to low
        // Otherwise, there is ghosting no teh floating lines set to input
        *GPDDAT = (*GPDDAT & ~0xff) | (1 << col);

        // Pulldown all lines except the output one
        *GPDUDP = (*GPDUDP & 0xffff0000) | (0x5555 ^ (1 << (2 * col)));

        // Only one column to output
        *GPDCON = (*GPDCON & 0xffff0000) | (1 << (2 * col));

        unsigned colKeys = debounce(GPGDAT, 0xFE, DEBOUNCE);

        lo = (lo << 8) | (colKeys & 0xff);
        if (col == 4)
            hi = lo;
    }

    // Read the ON key at GPG0
    unsigned onKey = debounce(GPGDAT, 1, DEBOUNCE);
    hi |= onKey << 31; // read the on key at GPG0

    // All columns to output
    *GPDCON = (*GPDCON & 0xffff0000) | 0x5555;

    // All lines output high
    *GPDDAT |= 0xff;

    // Disable all pull up/down
    *GPDUDP &= ~0xFFFF;

    // All keys function back to EINT
    *GPGCON = 0xaaaa;

    return ((keymatrix) lo) | (((keymatrix) hi) << 32);
}


void keyb_irq_int_handler()
// ----------------------------------------------------------------------------
// Analyze changes in the keyboard status and post messages accordingly
// ----------------------------------------------------------------------------
{
    *EINTMASK |= 0xff00;

    keyb_irq_update();

    *EINTPEND |= 0xff00;
    *EINTMASK &= ~0xff00;

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

    // Mask all external interrupts until they are properly programmed
    *INTMSK1 |= 0x20;

    // All columns to output
    *GPDCON= (*GPDCON &0xffff0000) | 0x5555;

    // Drive outputs high
    *GPDDAT |= 0xFF;

    // Disable pull up/down
    *GPDUDP = (*GPDUDP &0xffff0000);

    // Enable pulldown on all input lines
    *GPGUDP = 0x5555;

    // All rows on EINT
    *GPGCON = 0xaaaa;

    // All other keys trigger on both edges
    *EXTINT1 = 0x66666666;

    // Unmask 8-15
    *EINTMASK = (*EINTMASK & ~0x00ff00);

    // Clear all pending interrupts
    *EINTPEND = 0xfff0;

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
    *EINTMASK |= 0xFF00;
    irq_releasehook(5);
}
