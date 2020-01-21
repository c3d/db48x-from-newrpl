/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <ui.h>
#include <libraries.h>

extern unsigned int __cpu_intoff();
extern void __cpu_inton(unsigned int);

void halCPUSlowMode()
{
    unsigned int saved = __cpu_intoff();
    if(usb_isconnected())
        cpu_setspeed(HAL_USBCLOCK);
    else
        cpu_setspeed(HAL_SLOWCLOCK);
    __cpu_inton(saved);
}

void halCPUFastMode()
{
    unsigned int saved = __cpu_intoff();
    cpu_setspeed(HAL_FASTCLOCK);
    __cpu_inton(saved);
}
