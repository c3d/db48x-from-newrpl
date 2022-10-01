/*
 * Copyright (c) 2022, Christophe de Dinechin and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 *
 * This is the platform-specific key handling code for the HP-50G
 */

#include <keyboard.h>
#include <recorder.h>

RECORDER(shift, 8, "Shift logic");

void keyb_irq_shift_logic(keyb_msg_t key, keymatrix hwkeys, keymatrix changes)
// ----------------------------------------------------------------------------
//    Process the shift keys on the 50G
// ----------------------------------------------------------------------------
//   On the 50G, we have all three keys, so the shift logic is pretty simple
{
    keymatrix mask = KM_MASK(key);
    int       hold = (hwkeys & mask) != 0;

    record(shift, "Shift %x hold %d flags=%08X", key, hold, keyb_flags);
    if (changes & mask)
    {
        switch(key)
        {
        case KB_LSHIFT:
            // Left shift disables right shift but not alpha
            keyb_plane &= ~(KSHIFT_RIGHT | KHOLD_RIGHT);
            if (hold)
            {
                keyb_plane ^= KSHIFT_LEFT;
                keyb_plane |= KHOLD_LEFT;
            }
            else
            {
                keyb_plane &= ~KHOLD_LEFT;
            }
            break;
        case KB_RSHIFT:
            // Right shift disables left shift but not alpha
            keyb_plane &= ~(KSHIFT_LEFT | KHOLD_LEFT);
            if (hold)
            {
                keyb_plane ^= KSHIFT_RIGHT;
                keyb_plane |= KHOLD_RIGHT;
            }
            else
            {
                keyb_plane &= ~KHOLD_RIGHT;
            }
            break;
        case KB_ALPHA:
            // Toggle alpha, or lowercase if either shift
            // REVISIT: Other role for right shift?
            if (hold)
            {
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
            }
            else
            {
                keyb_flags &= ~KHOLD_ALPHA;
            }

            // Disable left shift unless held
            if ((hwkeys & KM_MASK(KB_LSHIFT)) == 0)
                keyb_plane &= ~(KSHIFT_LEFT|KHOLD_LEFT);

            // Disable right shift unless held
            if ((hwkeys & KM_MASK(KB_RSHIFT)) == 0)
                keyb_plane &= ~(KSHIFT_RIGHT|KHOLD_RIGHT);
            break;
        }
    }
    record(shift, "Shift %x hold %d post flags=%08X", key, hold, keyb_flags);
}
