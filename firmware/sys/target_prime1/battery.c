/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

WORD __battery __SYSTEM_GLOBAL__;

// SETUP ADC CONVERTERS TO READ BATTERY VOLTAGE
void bat_setup()
{
    //  ENABLE PRESCALER AT MAXIMUM DIVISION (255)
    //  SELECT CHANNEL 0, NORMAL OPERATION, START BY READ

    *GPFCON &= ~0xc0;    // SET GPF3 AS INPUT


    *ADCDLY = 0x5dc;
    *ADCMUX=0;
    *ADCCON=0x7fc2;     // Enable prescaler, maximum prescaler=0xff, start by Read
    *ADCTSC = 0xd8;     // YM 2 ground switch enable, YP to VDD disable, XM to GND disable, XP to VDD disable, PULL_UP disable
    __battery = *ADCDAT0 & 0x3ff;  // INITAL READ WILL TRIGGER FIRST CONVERSION

    bat_read();
}

void bat_read()
{

    if(*GPFDAT & 8) {
        // GPF3 BIT SET INDICATES WE ARE ON USB POWER!!
        __battery = 0x400;
        return;
    }

    while(!(*ADCCON & 0x8000));

    __battery = *ADCDAT0 & 0x3ff;  // READ LAST KNOWN VALUE, AND TRIGGER A NEW ONE
}
