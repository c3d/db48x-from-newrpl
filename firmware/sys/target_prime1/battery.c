/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

WORD __battery __SYSTEM_GLOBAL__;
WORD __bat_avg[8] __SCRATCH_MEMORY__;
WORD __bat_avgidx __SCRATCH_MEMORY__;
int __bat_readcnt __SCRATCH_MEMORY__;
// SETUP ADC CONVERTERS TO READ BATTERY VOLTAGE
void bat_setup()
{
    //  ENABLE PRESCALER AT MAXIMUM DIVISION (255)
    //  SELECT CHANNEL 0, NORMAL OPERATION, START BY READ

    // USB DRIVER USES GPF3 AS WELL, LET THEM SET IT UP EITHER AS INPUT OR AS EINT
    //*GPFCON &= ~0xc0;    // SET GPF3 AS INPUT


    *ADCDLY = 0x5dc;
    *ADCMUX=0;
    *ADCCON=0x7fc2;     // Enable prescaler, maximum prescaler=0xff, start by Read
    *ADCTSC = 0xd8;     // YM 2 ground switch enable, YP to VDD disable, XM to GND disable, XP to VDD disable, PULL_UP disable
    __battery = *ADCDAT0 & 0x3ff;  // INITAL READ WILL TRIGGER FIRST CONVERSION
    while(!(*ADCCON & 0x8000));
    __battery = *ADCDAT0 & 0x3ff;  // SECOND READ IS A GOOD VALUE

    for(int k=0;k<8;++k) __bat_avg[k]=__battery;
    __bat_avgidx=0;
    bat_read();
}

void bat_read()
{

    if(CABLE_IS_CONNECTED) {
        // GPF3 BIT SET INDICATES WE ARE ON USB POWER!!
        __battery = 0x400;
        return;
    }

    while(!(*ADCCON & 0x8000));

    __bat_avg[__bat_avgidx]= *ADCDAT0 & 0x3ff;  // READ LAST KNOWN VALUE, AND TRIGGER A NEW ONE

    if(__bat_avg[__bat_avgidx]<0x100) __bat_avg[__bat_avgidx]=0x3ff;    // WHEN BATTERY IS FULLY CHARGED AD CONVERSION RETURNS 0x0nn INSTEAD OF 0x4nn, JUST MAKE IT MAXIMUM CHARGE
    __bat_avgidx++;
    __bat_avgidx&=7;


    int count=0;
    for(int k=0;k<8;++k) count += __bat_avg[k]; // AVERAGE OUT TO MAKE IT MORE STABLE

    __battery = count >> 3;
}
