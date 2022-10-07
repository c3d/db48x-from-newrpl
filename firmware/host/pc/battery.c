/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */
#include <ui.h>
#include <recorder.h>


// This is a recorder tweak (see https://github.com/c3d/recorder)
// This makes it possible to control the battery level externally, using the
// recorder_scope application. Instructions:
// - Start the simulator with an environment variable RECORDER_SHARE pointing to
//   some temporary data file, e.g.:
//     RECORDER_SHARE=/tmp/sim.rec /path/to/simulator -tkeys=a,b,c
// - Start the recorder_scope with a slider to control the battery:
//     RECORDER_SHARE=/tmp/sim.rec /path/to/recorder_scope -sbattery=0:0:110
// - Tweak the battery level at runtime using the slider
RECORDER_TWEAK_DEFINE(battery, 25, "Battery level");


void battery_setup()
// ----------------------------------------------------------------------------
//   Setup the virtual battery
// ----------------------------------------------------------------------------
{
}


void battery_read()
// ----------------------------------------------------------------------------
//   Read the virtual battery level
// ----------------------------------------------------------------------------
{
}


int battery_level()
// ----------------------------------------------------------------------------
//    Return a normalized battery level
// ----------------------------------------------------------------------------
{
    int battery = RECORDER_TWEAK(battery);
    if (battery < 0)
        battery = 0;
    else if (battery > 100)
        battery = 100;
    return battery;
}


int battery_charging()
// ----------------------------------------------------------------------------
//   Checks if we are on USB
// ----------------------------------------------------------------------------
{
    return RECORDER_TWEAK(battery) > 100;
}


int battery_low()
// ----------------------------------------------------------------------------
//   Return 1 if low, 2 if critical
// ----------------------------------------------------------------------------
{
    int battery = RECORDER_TWEAK(battery);
    return battery < 10 ? 2 : battery < 20 ? 1 : 0;
}
