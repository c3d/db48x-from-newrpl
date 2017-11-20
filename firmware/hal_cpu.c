/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <newrpl.h>
#include <ui.h>
#include <libraries.h>


void halCPUSlowMode()
{
if(usb_isconnected()) cpu_setspeed(HAL_USBCLOCK);
else cpu_setspeed(HAL_SLOWCLOCK);
}

void halCPUFastMode()
{
    cpu_setspeed(HAL_FASTCLOCK);
}

