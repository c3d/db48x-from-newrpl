/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// SOFTWARE ALARM MANAGEMENT - HIGHER LEVEL API

#include <newrpl.h>
#include <ui.h>

/*
void halInitAlarm(uint32_t hot_start, int32_t reset)
{
    if (hot_start) {
        // BOOT FROM POWEROFF

        if (rplCheckAlarms())
            halSetNotification(N_CONNECTION, 0xf);
        else
            halSetNotification(N_CONNECTION, 0x0);

            // TODO : USE ALARM ICON NOTIFICATION

        return;
    }

    if (!reset) {
        // BOOT FROM WARMSTART

        rplUpdateAlarms();
        if (rplCheckAlarms())
            halSetNotification(N_CONNECTION, 0xf);
        else
            halSetNotification(N_CONNECTION, 0x0);

    } else {
        // BOOT FROM RESET

    }

    return;
}
*/
void halTriggerAlarm()
{
    if(halFlags & HAL_SKIPNEXTALARM) {
        halFlags &= ~HAL_SKIPNEXTALARM;
        rplSkipNextAlarm();
        return;
    }

    if(rplTriggerAlarm())
        halSetNotification(N_ALARM, 0xf);
    else
        halSetNotification(N_ALARM, 0x0);

    halScreen.DirtyFlag |=
            STACK_DIRTY | MENU1_DIRTY | MENU2_DIRTY | STAREA_DIRTY;

    return;
}
