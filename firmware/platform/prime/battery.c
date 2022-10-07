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
#define ADC_LOWBAT    (ADC_0_LIMIT + (ADC_100_LIMIT - ADC_0_LIMIT) / 10)
#define ADC_CRITICAL  (ADC_0_LIMIT + (ADC_100_LIMIT - ADC_0_LIMIT) / 20)


static WORD raw_battery SYSTEM_GLOBAL;
static WORD bat_avg[8] SCRATCH_MEMORY;
static WORD bat_avgidx SCRATCH_MEMORY;
static int  battery_readcnt SCRATCH_MEMORY;


void battery_setup()
// ----------------------------------------------------------------------------
// Setup adc converters to read battery voltage
// ----------------------------------------------------------------------------
{
    // Enable prescaler at maximum division (255)
    // Select channel 0, normal operation, start by read
    // USB driver uses GPF3 as well, let them set it up
    // either as input or as eint
    //   *GPFCON &= ~0xc0;    // SET GPF3 AS INPUT

    *ADCDLY = 0x5dc;
    *ADCMUX = 0;
    *ADCCON = 0x7fc2; // Enable prescaler, maximum prescaler=0xff, start by Read
    *ADCTSC = 0xd8;   // YM 2 ground switch enable, YP to VDD disable, XM to GND
                      // disable, XP to VDD disable, PULL_UP disable
    battery = *ADCDAT0 & 0x3ff; // Inital read will trigger first conversion
    while (!(*ADCCON & 0x8000))
        ;
    battery = *ADCDAT0 & 0x3ff; // Second read is a good value

    for (int k = 0; k < 8; ++k)
        bat_avg[k] = battery;
    bat_avgidx = 0;

    battery_read();
}


void battery_read()
// ----------------------------------------------------------------------------
//   Read raw battery value
// ----------------------------------------------------------------------------
{
    if (CABLE_IS_CONNECTED)
    {
        // GPF3 bit set indicates we are on USB power
        battery = ADC_PLUGGED;
        return;
    }

    while (!(*ADCCON & 0x8000))
        ;

    bat_avg[bat_avgidx] =
        *ADCDAT0 & 0x3ff; // Read last known value, and trigger a new one

    // When battery is fully charged ad conversion returns 0x0nn
    // Instead of 0x4nn, just make it maximum charge
    if (bat_avg[bat_avgidx] < 0x100)
        bat_avg[bat_avgidx] =
            0x3ff;
    bat_avgidx++;
    bat_avgidx&=7;


    // Average out to make it more stable
    int count = 0;
    for (int k = 0; k < 8; ++k)
        count += bat_avg[k];

    battery = count >> 3;
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
