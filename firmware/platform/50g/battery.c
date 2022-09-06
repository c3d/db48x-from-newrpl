/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

WORD battery SYSTEM_GLOBAL;

// SETUP ADC CONVERTERS TO READ BATTERY VOLTAGE
void bat_setup()
{
    //  ENABLE PRESCALER AT MAXIMUM DIVISION (255)
    //  SELECT CHANNEL 0, NORMAL OPERATION, START BY READ

    *HWREG(IO_REGS, 0x58) |= 0x2;       // DISABLE PULLUPS
    *HWREG(ADC_REGS, 0) = 0x7FC2;
    *HWREG(ADC_REGS, 4) = 0x58;
    *HWREG(ADC_REGS, 8) = 0xff;
    battery = *HWREG(ADC_REGS, 0xc) & 0x3ff;  // INITAL READ WILL TRIGGER FIRST CONVERSION
    bat_read();
}

void bat_read()
{
    if(*HWREG(IO_REGS, 0x54) & 2) {
        // GPF1 BIT SET INDICATES WE ARE ON USB POWER!!
        battery = 0x400;
        return;
    }
    while(!(*HWREG(ADC_REGS, 0) & 0x8000));
    battery = *HWREG(ADC_REGS, 0xc) & 0x3ff;  // READ LAST KNOWN VALUE, AND TRIGGER A NEW ONE
}
