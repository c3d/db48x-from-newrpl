/*
 * Copyright (c) 2022, Christophe de Dinechin and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 *
 * This is the platform-specific key handling code for the DM42
 */

#include <keyboard.h>
#include <recorder.h>
#include <stdio.h>

FLIGHT_RECORDER(shift, "Shift logic");

void keyb_irq_shift_logic(keyb_msg_t key, keymatrix hwkeys, keymatrix changes)
// ----------------------------------------------------------------------------
//    Process the shift keys on the DM42
// ----------------------------------------------------------------------------
//   On the DM42, there is a single shift key. Make it toggle Shift/Alpha
{
    keymatrix mask = KM_MASK(key);
    int       hold = (hwkeys & mask) != 0;
    int       longpress = hold && (keyb_flags & KFLAG_LONG_PRESS) != 0;

    record(shift, "Shift %x hold %d flags=%08X", key, hold, keyb_flags);
    if ((changes & mask) || longpress)
    {
        // Three-state toggle between None, Left-Shift and Alpha
        if (hold)
        {
            if (keyb_plane & KSHIFT_LEFT)
            {
                keyb_plane &= ~(KSHIFT_LEFT | KHOLD_LEFT);
                keyb_plane |= (KSHIFT_ALPHA | KHOLD_ALPHA);
            }
            else if (keyb_plane & KSHIFT_ALPHA)
            {
                if (keyb_prev_code == KB_SHIFT || keyb_prev_code == 0)
                    keyb_plane &= ~(KSHIFT_ANY | KHOLD_ANY);
                else
                    keyb_plane ^= KFLAG_ALPHA_LOWER;
            }
            else if (longpress)
            {
                keyb_plane &= ~(KSHIFT_LEFT | KHOLD_LEFT);
                keyb_plane |= KSHIFT_RIGHT | KHOLD_RIGHT;
            }
            else if (keyb_plane & KSHIFT_RIGHT)
            {
                keyb_plane &= ~(KSHIFT_ANY | KHOLD_ANY);
            }
            else
            {
                keyb_plane &= ~(KSHIFT_RIGHT | KHOLD_RIGHT);
                keyb_plane |= (KSHIFT_LEFT | KHOLD_LEFT);
            }
        }
        else
        {
            keyb_plane &= ~KHOLD_ANY;
            if (keyb_last_code == KB_SHIFT)
            {
                keyb_last_code = 0;
                keyb_flags &= ~KFLAG_LONG_PRESS;
            }
        }
    }
    record(shift, "Shift %x hold %d post flags=%08X", key, hold, keyb_flags);
}
