/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

#define ADC_100_LIMIT 0x370
#define ADC_0_LIMIT   0x300
#define ADC_PLUGGED   0x400
#define ADC_LOWBAT    0x320     // REVISIT: the Prime values look better
#define ADC_CRITICAL  0x300

static WORD raw_battery SYSTEM_GLOBAL;


void battery_setup()
// ----------------------------------------------------------------------------
// Setup ADC converters to read battery voltage
// ----------------------------------------------------------------------------
{
    //  Enable prescaler at maximum division (255)
    //  Select channel 0, normal operation, start by read
    *HWREG(IO_REGS, 0x58) |= 0x2; // Disable pullups
    *HWREG(ADC_REGS, 0) = 0x7FC2;
    *HWREG(ADC_REGS, 4) = 0x58;
    *HWREG(ADC_REGS, 8) = 0xff;
    raw_battery = *HWREG(ADC_REGS, 0xc) & 0x3ff; // Inital read triggers conversion
    battery_read();
}


void battery_read()
// ----------------------------------------------------------------------------
//   Scan battery level
// ----------------------------------------------------------------------------
{
    if (*HWREG(IO_REGS, 0x54) & 2)
    {
        // GPF1 bit set indicates we are on USB power
        raw_battery = ADC_PLUGGED;
        return;
    }
    while(!(*HWREG(ADC_REGS, 0) & 0x8000));

    // Read last known value, and trigger a new one
    raw_battery = *HWREG(ADC_REGS, 0xc) & 0x3ff;
}


int battery_level()
// ----------------------------------------------------------------------------
//    Return a normalized battery level
// ----------------------------------------------------------------------------
{
    return 100 * (raw_battery - ADC_0_LIMIT) / (ADC_100_LIMIT - ADC_0_LIMIT);
}


int battery_charging()
// ----------------------------------------------------------------------------
//   Checks if we are on USB
// ----------------------------------------------------------------------------
{
    return raw_battery == ADC_PLUGGED;
}


int battery_low()
// ----------------------------------------------------------------------------
//   Return 1 if low, 2 if critical
// ----------------------------------------------------------------------------
{
    return raw_battery <= ADC_CRITICAL ? 2 : raw_battery <= ADC_LOWBAT ? 1 : 0;
}
