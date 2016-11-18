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
    *HWREG(ADC_REGS,0)=0x7FC2;
    *HWREG(ADC_REGS,4)=0x58;
    *HWREG(ADC_REGS,8)=0xff;
    __battery=*HWREG(ADC_REGS,0xc)&0x3ff; // INITAL READ WILL TRIGGER FIRST CONVERSION
    bat_read();
}

void bat_read()
{
    while(!(*HWREG(ADC_REGS,0)&0x8000)) ;
    __battery=*HWREG(ADC_REGS,0xc)&0x3ff;     // READ LAST KNOWN VALUE, AND TRIGGER A NEW ONE
}


