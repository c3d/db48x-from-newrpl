/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <ui.h>
#include <libraries.h>

extern INTERRUPT_TYPE cpu_intoff_nosave();
extern void cpu_inton_nosave(INTERRUPT_TYPE);

void halCPUSlowMode()
{
    INTERRUPT_TYPE saved = cpu_intoff_nosave();
    if(usb_isconnected())
        cpu_setspeed(HAL_USBCLOCK);
    else
        cpu_setspeed(HAL_SLOWCLOCK);
    cpu_inton_nosave(saved);
}

void halCPUFastMode()
{
    INTERRUPT_TYPE saved = cpu_intoff_nosave();
    cpu_setspeed(HAL_FASTCLOCK);
    cpu_inton_nosave(saved);
}
