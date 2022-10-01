/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>
#include <keyboard.h>


// ============================================================================
//
// Low-level routine to be used by the irq and exception handlers only
//
// ============================================================================

#define DEBOUNCE  48    // 10 SEEMS TO BE ADEQUATE EVEN AT 75 MHz

keymatrix keyb_irq_get_matrix()
// ----------------------------------------------------------------------------
//   Get the keyboard matrix
// ----------------------------------------------------------------------------
{
    unsigned lo = 0;
    unsigned hi = 0;

    for (int col = 7; col >= 0; --col)
    {
        unsigned control = (1 << ((col + 8) * 2)) | 0x1;

         // Drive the output column low
        *GPGDAT = 0;
        *GPGCON = control;

        // Read debounced column
        unsigned colKeys = debounce (GPGDAT, 0xfe, DEBOUNCE);
        lo = (lo << 8) | ((~colKeys) & 0xfe);
        if (col == 4)
            hi = lo;
    }

    *GPGCON = 0x5555AAA9; // Set to trigger interrupts on any key
    *GPGDAT = 0;          // Drive all output columns low
    unsigned intKeys = debounce(GPFDAT, 0x71, DEBOUNCE);

    hi |= (intKeys & 0x70) << 24;
    hi |= intKeys << 31;

    return ((keymatrix) lo) | (((keymatrix) hi) << 32);
}


void keyb_irq_int_handler()
// ----------------------------------------------------------------------------
// Analyze changes in the keyboard status and post messages accordingly
// ----------------------------------------------------------------------------
{
    *EINTMASK |= 0xfe70;

    keyb_irq_update();

    *EINTPEND |= 0xfe70;
    *EINTMASK &= ~0xfe70;
}


void keyb_irq_init()
// ----------------------------------------------------------------------------
//   Initialize the keyboard interrupt subsystem for HP50G and similar
// ----------------------------------------------------------------------------
{
    // Mask all external interrupts until they are properly programmed
    *INTMSK |= 0x31;

    // Initialize all global variables used by the keyboard subsystem
    keyb_flags                 = 0;
    keyb_plane                 = 0;
    keyb_used                  = 0;
    keyb_current               = 0;
    keyb_last_code             = 0;
    keyb_matrix                = 0LL;
    keyb_irq_repeat_time       = 100 / KEYB_SCAN_SPEED;
    keyb_irq_long_press_time   = 1000 / KEYB_SCAN_SPEED;
    keyb_irq_debounce          = 20 / KEYB_SCAN_SPEED;
    keyb_irq_lock              = 0;

    // Initialize timer event 0
    tmr_events[0].eventhandler = keyb_irq_update;
    tmr_events[0].delay = (KEYB_SCAN_SPEED * tmr_getsysfreq()) / 1000;
    tmr_events[0].status = 0;

    // Initialize the hardware
    *GPGCON = 0x5555AAA9;       // Drive all columns to output, rows to EINT
    *GPGDAT = 0;                // Drive outputs low
    *GPGUP = 0x1;               // Enable pullups on input lines only

    // Set all shifts to generate interrupts
    *GPFCON = (*GPFCON & 0xffffc0fc) | 0x2a02;

    // Add interrupt hooks for the relevant keys
    irq_add_hook(5, &keyb_irq_int_handler);
    irq_add_hook(4, &keyb_irq_int_handler);      // Shifts
    irq_add_hook(0, &keyb_irq_int_handler);      // ON

    // Shifts trigger on both edges
    *EXTINT0 = (*EXTINT0 & 0xf000fff0) | 0x06660006;

    // Other keys trigger on both edges
    *EXTINT1 = 0x66666666;

    // Unmask interrupts 4, 5, 6 and 9-15
    *EINTMASK = (*EINTMASK & 0x00ff018f);

    // Clear all pending interrupts
    *EINTPEND = 0xffffffff;

    // Unmask external interrupts
    *INTMSK = *INTMSK & 0xffffffce;
    *SRCPND |= 0x31;
    *INTPND |= 0x31;

    // We are done - Mark keyboard interrupt handling  as running
    keyb_flags = KFLAG_RUNNING;
}


void keyb_irq_stop(unsigned int *keysave)
// ----------------------------------------------------------------------------
//   Shutdown the keyboard interrupt subsystem (dead code, not ever called)
// ----------------------------------------------------------------------------
{
    // We can no longer run at this point
    keyb_flags &= ~KFLAG_RUNNING;

    tmr_events[0].status = 0;

    // Disable interrupts, status will be fully restored on exit
    *INTMSK |= 0x31;
    *EINTMASK |= 0xFE70;
    irq_releasehook(5);
    irq_releasehook(4);
    irq_releasehook(0);

    // Restore io port configuration
    *GPGCON = keysave[0];
    *GPFCON = keysave[1];
}
