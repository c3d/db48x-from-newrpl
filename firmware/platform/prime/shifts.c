/*
 * Copyright (c) 2022, Christophe de Dinechin and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 *
 * This is the platform-specific key handling code for the HP Prime
 */

#include <keyboard.h>
#include <recorder.h>

RECORDER(shift, 8, "Shift logic");

void keyb_irq_shift_logic(keyb_msg_t key, keymatrix hwkeys, keymatrix changes)
// ----------------------------------------------------------------------------
//    Process the shift keys on the Prime
// ----------------------------------------------------------------------------
//   On the Prime, there is only one shift, which toggles left / right
{
    keymatrix mask = KM_MASK(key);
    int       hold = (hwkeys & mask) != 0;
    int       longpress = hold && (keyb_flags & KFLAG_LONG_PRESS) != 0;

    record(shift, "Shift %x hold %d flags=%08X", key, hold, keyb_flags);
    if ((changes & mask) || longpress)
    {
        switch(key)
        {
        case KB_SHIFT:
            // Left shift disables right shift but not alpha
            if (hold)
            {
                if (longpress || (keyb_plane & KSHIFT_LEFT))
                {
                    keyb_plane &= ~(KSHIFT_LEFT | KHOLD_LEFT);
                    keyb_plane |= (KSHIFT_RIGHT | KHOLD_RIGHT);
                }
                else if (keyb_plane & KSHIFT_RIGHT)
                {
                    keyb_plane &= ~(KSHIFT_RIGHT | KHOLD_RIGHT);
                }
                else
                {
                    keyb_plane |= (KSHIFT_LEFT | KHOLD_LEFT);
                }
            }
            else
            {
                keyb_plane &= ~(KHOLD_LEFT | KHOLD_RIGHT);
            }
            break;
        case KB_ALPHA:
            if (hold)
            {
                // Toggle alpha, or lowercase if either shift
                // REVISIT: Other role for right shift?
                if (keyb_plane & (KSHIFT_LEFT | KSHIFT_RIGHT))
                {
                    keyb_flags ^= KFLAG_ALPHA_LOWER;
                    keyb_flags |= KSHIFT_ALPHA;
                }
                else
                {
                    keyb_flags ^= KSHIFT_ALPHA;
                    if (!(keyb_flags & KSHIFT_ALPHA))
                        keyb_flags &= ~KFLAG_ALPHA_LOWER;
                }
                keyb_flags |= KHOLD_ALPHA;

                // Disable shift unless held
                if ((hwkeys & KM_MASK(KB_LSHIFT)) == 0)
                    keyb_plane &= ~(KSHIFT_LEFT | KHOLD_LEFT
                                    | KSHIFT_RIGHT | KHOLD_RIGHT);
            }
            else
            {
                keyb_flags &= ~KHOLD_ALPHA;
            }
            break;
        }
    }
    record(shift, "Shift %x hold %d post flags=%08X", key, hold, keyb_flags);
}
