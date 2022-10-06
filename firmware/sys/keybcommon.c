/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <ui.h>
#include <keyboard.h>
#include <strings.h>


// ============================================================================
//
// Keyboard, low level global variables
//
// ============================================================================

keyb_msg_t      keyb_irq_buffer[KEYB_BUFFER]            SYSTEM_GLOBAL;
volatile int    keyb_irq_lock                           SYSTEM_GLOBAL;
unsigned        keyb_flags                              SYSTEM_GLOBAL;
keyb_index_t    keyb_used                               SYSTEM_GLOBAL;
keyb_index_t    keyb_current                            SYSTEM_GLOBAL;
keymatrix       keyb_matrix                             SYSTEM_GLOBAL;
unsigned        keyb_plane                              SYSTEM_GLOBAL;
int             keyb_last_code                          SYSTEM_GLOBAL;
int             keyb_duration                           SYSTEM_GLOBAL;
int             keyb_irq_repeat_time                    SYSTEM_GLOBAL;
int             keyb_irq_long_press_time                SYSTEM_GLOBAL;
int             keyb_irq_debounce                       SYSTEM_GLOBAL;


// ============================================================================
//
//    Low-level function to be used by the exception subsystem only
//
// ============================================================================

keyb_msg_t keyb_irq_get_key()
// ----------------------------------------------------------------------------
//   Get a non-shift key
// ----------------------------------------------------------------------------
{
    keymatrix m = keyb_irq_get_matrix_no_interrupts();

    // Wait for a non-shift key to be pressed
    while (KM_UNSHIFTED(m) == 0)
        m = keyb_irq_get_matrix_no_interrupts();

    // Check which shift keys are pressed
    int shifts = 0;
    if (KM_HAS_LEFT_SHIFT(m))
        shifts |= KSHIFT_LEFT | KHOLD_LEFT;

#if KB_RSHIFT != KB_LSHIFT      // Has a genuine right shift (HP50G, etc)
    if (KM_HAS_RIGHT_SHIFT(m))
        shifts |= KSHIFT_RIGHT | KHOLD_RIGHT;
#endif

#if KB_ALPHA != KB_LSHIFT       // Has a genuine alpha key (all but DM42)
    if (KM_HAS_ALPHA(m))
        shifts |= KSHIFT_ALPHA | KHOLD_ALPHA;
#endif

    keymatrix noshift  = KM_UNSHIFTED(m);
    int       kcodebit = ffsll(noshift) - 1;
    int       kcode    = keyb_code(kcodebit);

    // Wait for all non-shifted key to be released
    while (KM_UNSHIFTED(m) != 0)
        m = keyb_irq_get_matrix_no_interrupts();

    return kcode | shifts;
}


void keyb_irq_post_message(keyb_msg_t msg)
// ----------------------------------------------------------------------------
//  Post a message in the keyboard buffer
// ----------------------------------------------------------------------------
{
    keyb_index_t current = keyb_current;
    keyb_index_t next = (current + 1) % KEYB_BUFFER;

    // Buffer overrun: exit without recording anything
    if(next == keyb_used)
        return;

    // Tag the relevant flags to the message
    msg |= keyb_flags & KM_FLAGS_MASK;

    // Write buffer before index. In case of race, only one message is recorded
    keyb_irq_buffer[current] = msg;
    keyb_current = next;
}


void keyb_post_message(keyb_msg_t msg)
// ----------------------------------------------------------------------------
//    Post a message under CPU lock
// ----------------------------------------------------------------------------
// WARNING: Problems may arise if the interrupt service wants
// to post a message while the user is posting one.
// REVISIT: What kind of problem exactly?
{
    while(cpu_get_lock(1, &keyb_irq_lock));
    keyb_irq_post_message(msg);
    keyb_irq_lock = 0;

}


keyb_msg_t keyb_get_message()
// ----------------------------------------------------------------------------
//    Get the next keyboard message posted
// ----------------------------------------------------------------------------
{
    if(keyb_used == keyb_current)
        return 0;
    keyb_msg_t msg = keyb_irq_buffer[keyb_used];
    keyb_used = (keyb_used + 1) & (KEYB_BUFFER - 1);
    return msg;
}


unsigned keyb_any_message()
// ----------------------------------------------------------------------------
//   Return number of pending keystrokes
// ----------------------------------------------------------------------------
{
    return keyb_current - keyb_used;
}


void keyb_flush()
// ----------------------------------------------------------------------------
//   Wait until no key is pressed, then flush keyboard buffer
// ----------------------------------------------------------------------------
{
    while(keyb_get_matrix() != 0LL);
    keyb_used = keyb_current;
}


void keyb_flush_no_wait()
// ----------------------------------------------------------------------------
//   Flush keyboard buffer without waiting
// ----------------------------------------------------------------------------
{
    keyb_used = keyb_current;
}


int keyb_was_updated()
// ----------------------------------------------------------------------------
//    Check if a keyboard update happened
// ----------------------------------------------------------------------------
{
    unsigned k = keyb_flags & KFLAG_UPDATED;
    keyb_flags ^= k;            // Clear if it was set
    return k;
}


int keyb_irq_on_keys(keymatrix hw, keymatrix changes)
// ----------------------------------------------------------------------------
//   Process ON key changes
// ----------------------------------------------------------------------------
//   ON alone: Acts as EXIT (interrupts the program) or OFF if shifted
//             This is handled as a normal key in the RPL main loop
//   ON-hold : Keys for systems setup, handled in the main RPL loop
//    ON +   : Increase contrast
//    ON -   : Decrease contrast
//    ON UP  : Increase font size
//    ON DN  : Decrease font size
//    ON .   : Cycle locale
//    ON SPC : Cycle display mode
//    ON 0-9 : Adjust precision (mapping to be decided)
//   ON-A    : Attention - Operations under interrupt, work even when frozen
//    ON-A-C : Take control - Offer option to force interrupt RPL program
//    ON-A-D : Dump - Dump the recorder on screen, possibly to flash
//    ON-A-F : Forced attention - Throw exception
{
    // Check if ON is still held, and if other keys changed
    if ((hw & KM(ON)) && (changes & KM(ON)) == 0)
    {
        // ON A: Attention
        if (hw & KM(A))
        {
            // Check ON-A-C pressed, offer the option to stop the program
            if (hw & KM(C))
            {
                throw_exception("RPL Break requested",
                                EX_CONT | EX_RPLEXIT | EX_WARM | EX_RESET);
                // Loop in caller to send key up for released keys
                return 1;
            }

             // Check ON-A-D: Recorder dump
            if (hw & KM(D))
            {
                throw_exception("Recorder dump",
                                EX_CONT | EX_WARM | EX_WIPEOUT | EX_RESET |
                                EX_RPLREGS);
                // Loop in caller to send key up for released keys
                return 1;
            }

            // Check ON-A-F: User break
            if (hw & KM(F))
            {
                throw_exception("User BREAK requested",
                                EX_CONT | EX_WARM | EX_WIPEOUT | EX_RESET |
                                EX_RPLREGS);
                // Loop in caller to send key up for released keys
                return 1;
            }
        }
    }

    // All other cases: return to the main handler
    return 0;
}


static inline void keyb_irq_shifts(keymatrix hwkeys, keymatrix changes)
// ----------------------------------------------------------------------------
//    Run the logic for how shift keys change
// ----------------------------------------------------------------------------
{
    // Need to observe if either key changes, or currently held
    keymatrix relevant = (hwkeys | changes) & KM_ALL_SHIFTS;
    while (relevant)
    {
        int        key    = ffsll(relevant) - 1;
        keymatrix  change = KM_MASK(key);
        keyb_msg_t msg    = key;
        keyb_flags |= KFLAG_SHIFTS_CHANGED;
        keyb_irq_shift_logic(key, hwkeys, changes);
        keyb_irq_post_message(msg); // Will pass shifts flags from keyb_flags
        keyb_last_code = key;
        relevant ^= change;
    }
}


static inline void keyb_irq_normal_keys(keymatrix hwkeys, keymatrix changes)
// ----------------------------------------------------------------------------
//    Run the logic for how normal keys change
// ----------------------------------------------------------------------------
{
    keymatrix relevant = changes & ~KM_ALL_SHIFTS;
    int       unshift  = 0;
    while (relevant)
    {
        int        key  = ffsll(relevant) - 1;
        keymatrix  mask = KM_MASK(key);
        int        down = (hwkeys & mask) != 0;
        keyb_msg_t msg  = key | (down ? KM_KEYDN : KM_KEYUP);
        if (!down)
        {
            if ((keyb_flags & KFLAG_LONG_PRESS) == 0 || key != keyb_last_code)
                keyb_irq_post_key(key);
            else
                keyb_flags &= ~(KFLAG_LONG_PRESS | KFLAG_REPEAT);
            unshift = 1;
        }

        // Will pass required flags from keyb_flags
        keyb_irq_post_message(msg);
        keyb_last_code = key;
        relevant ^= mask;
    }

    // Clear shifts after regular keys
    if (unshift)
    {
        unsigned old = keyb_flags;
        if (!(keyb_flags & KHOLD_LEFT))
            keyb_flags &= ~KSHIFT_LEFT;
        if (!(keyb_flags & KHOLD_RIGHT))
            keyb_flags &= ~KSHIFT_RIGHT;
        if (old != keyb_flags)
            keyb_flags |= KFLAG_SHIFTS_CHANGED;
    }
}


static inline void keyb_irq_setup_repeat_timer(keymatrix hwkeys)
// ----------------------------------------------------------------------------
//   Enable or disable key scanning timer depending on need
// ----------------------------------------------------------------------------
{
    if (hwkeys == 0)
    {
        // No key pressed: we can disable the timer, use keyboard interrupts
        if (keyb_duration)
        {
            tmr_events[0].status = 0;
            keyb_duration        = 0;
        }
    }
    else if ((tmr_events[0].status & 1) == 0)
    {
        // Activate the timer event if not already running
        tmr_events[0].ticks  = tmr_ticks() + tmr_events[0].delay;
        tmr_events[0].status = 3;
        tmr_event_reschedule();
    }
}


static inline void keyb_irq_check_long_presses(keymatrix hwkeys,
                                               keymatrix changes)
// ----------------------------------------------------------------------------
//   Check if we got long presses and need to post long press or repeat
// ----------------------------------------------------------------------------
{
    keymatrix longPressKeyMask = KM_MASK(keyb_last_code);
    if (changes & ~longPressKeyMask)
    {
        // Any change in keyboard matrix state disables repeat / long-press
        keyb_duration = 0;
        keyb_flags &= ~(KFLAG_LONG_PRESS | KFLAG_REPEAT);
    }
    else if (hwkeys & longPressKeyMask)
    {
        ++keyb_duration;

        // Check if key handler asked for key repeat, if so send it
        if (keyb_flags & KFLAG_REPEAT)
        {
            if (keyb_duration >= KEYB_REPEAT_TIME)
            {
                // Post a repeat event
                keyb_irq_post_repeat(keyb_last_code);
                keyb_duration -= KEYB_REPEAT_TIME;
            }
        }
        else if ((keyb_flags & KFLAG_LONG_PRESS) == 0)
        {
            // Post long-press only the first time we see it
            if (keyb_duration >= KEYB_LONG_PRESS_TIME)
            {
                keyb_flags |= KFLAG_LONG_PRESS;
                keyb_irq_post_message(keyb_last_code);
                keyb_duration -= KEYB_LONG_PRESS_TIME;
            }
        }
    }
}


void keyb_irq_update()
// ----------------------------------------------------------------------------
//   Heavy lifting state machine for key status updates
// ----------------------------------------------------------------------------
//   REVISIT: This badly needs to be broken up and simplified
{
    if (cpu_get_lock(1, &keyb_irq_lock))
        return;

retry:
    // Indicate we are updating
    keyb_flags |= KFLAG_UPDATED;

    keymatrix hwkeys  = keyb_irq_get_matrix();
    keymatrix changes = hwkeys ^ keyb_matrix;
    keyb_matrix       = hwkeys;

#if 0 // DEBUG code
    // Print keyboard matrix for debugging
    gglsurface scr;
    ggl_init_screen(&scr);
    int k;
    for (k = 0; k < 64; ++k)
        ggl_rect(&scr, LCD_W - 4 * (k + 2), 0, LCD_W - 4 * (k + 1), 4,
                 (hw & (1LL << k)) ? 0xffffffff : 0x44444444);
#endif // Debug code

    // Check if this is a configuration or interrupt function (ON + keys)
    if (changes & KM(ON))
        if (keyb_irq_on_keys(hwkeys, changes))
            goto retry;

    // Enable / disable the repeater timer
    keyb_irq_setup_repeat_timer(hwkeys);

    // Check long press / repeat
    keyb_irq_check_long_presses(hwkeys, changes);

    // Before any other key, process shifts
    if ((hwkeys | changes) & KM_ALL_SHIFTS)
        keyb_irq_shifts(hwkeys, changes);

    // Analyze other keys
    keyb_irq_normal_keys(hwkeys, changes);

    // Signal the key udpate and release the interrupt lock
    keyb_flags |= KFLAG_UPDATED;
    keyb_irq_lock = 0;
}


void keyb_set_timing(int repeat, int lgprs, int debounce)
// ----------------------------------------------------------------------------
//   Set the timing for repeat, long press and debounce
// ----------------------------------------------------------------------------
{
    keyb_irq_repeat_time = (repeat + KEYB_SCAN_SPEED - 1) / KEYB_SCAN_SPEED;
    keyb_irq_long_press_time = (lgprs + KEYB_SCAN_SPEED - 1) / KEYB_SCAN_SPEED;
    keyb_irq_debounce = (debounce + KEYB_SCAN_SPEED - 1) / KEYB_SCAN_SPEED;
}


void keyb_set_repeat(int repeat)
// ----------------------------------------------------------------------------
//   Used by key handlers to indicate the key can repeat
// ----------------------------------------------------------------------------
{
    if(repeat)
        keyb_flags |= KFLAG_REPEAT;
    else
        keyb_flags &= ~KFLAG_REPEAT;
}


void keyb_set_alpha_once(int single_alpha)
// ----------------------------------------------------------------------------
//   Indicate that we want a single alpha
// ----------------------------------------------------------------------------
{
    if(single_alpha)
        keyb_flags |= KFLAG_ALPHA_ONCE;
    else
        keyb_flags &= ~KFLAG_ALPHA_ONCE;

}

void keyb_set_shift_plane(unsigned newplane)
// ----------------------------------------------------------------------------
//    Change the shift plane (use one of the SHIFT constants)
// ----------------------------------------------------------------------------
{
    unsigned oldplane = keyb_plane;

    if(newplane & KSHIFT_LEFT)
        keyb_plane |= KSHIFT_LEFT;
    else
        keyb_plane &= ~KSHIFT_LEFT;
    if(newplane & KSHIFT_RIGHT)
        keyb_plane |= KSHIFT_RIGHT;
    else
        keyb_plane &= ~KSHIFT_RIGHT;
    if(newplane & KSHIFT_ALPHA)
        keyb_plane |= KSHIFT_ALPHA;
    else
        keyb_plane &= ~KSHIFT_ALPHA;

    if (keyb_plane != oldplane)
        keyb_flags |= KFLAG_SHIFTS_CHANGED;
}


unsigned int keyb_get_shift_plane()
// ----------------------------------------------------------------------------
//    Get the current shift plane
// ----------------------------------------------------------------------------
{
    return keyb_plane & KSHIFT_ANY;
}



// ============================================================================
//
//   Shared interrupt routines
//
// ============================================================================

keymatrix keyb_irq_get_matrix_no_interrupts()
// ----------------------------------------------------------------------------
//   Read the keyboard from exception handler (Disable interrupts)
// ----------------------------------------------------------------------------
// This is needed only when called from within an exception handler
{
    INTERRUPT_TYPE saved = cpu_intoff_nosave();
    keymatrix m = keyb_irq_get_matrix();
    cpu_inton_nosave(saved);
    return m;
}


void keyb_irq_wait_release()
// ----------------------------------------------------------------------------
//   Wait for all keys to be released
// ----------------------------------------------------------------------------
{
    while(keyb_irq_get_matrix_no_interrupts())
        /* spin */;
}


keymatrix keyb_get_matrix()
// ----------------------------------------------------------------------------
//   Get the cached matrix value if running, otherwise fetch it from hardware
// ----------------------------------------------------------------------------
{
    return (keyb_flags & KFLAG_RUNNING) ? keyb_matrix : keyb_irq_get_matrix();
}
