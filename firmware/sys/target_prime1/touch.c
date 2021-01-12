/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>


// HP Prime touchscreen driver for Novatek NT11002

// From datasheet:
// Minimum time for clock low or clock high is 1250 ns

// Minimum time for Start = 600 ns

// Lines used by the touchscreen:

// GPE14 is configured as IICSCL
// GPE15 is configured as IICSDA
// GPF2 is configured as EINT[2] with which the slave signals to the master that data is available (INT line of NT11002)
// GPF0 probably VCC

// Default address for touchscreen panel per datasheet
#define TS_I2CADDR      0x46


// Setup TIMER 4 for delays in LCD chip communications
#define LCDTIMER_FREQ 100000        // 100 kHz tick

extern unsigned int __cpu_getPCLK();

typedef struct {
    uint8_t GID1,GID2;

    struct {
        uint8_t D1,D2,D3,D4;
    } P[10];

    uint8_t FWVER,Pwr_Ctl1,Pwr_Ctl2,Read_Pnt;

    uint8_t Reserved1,Reserved2;

} tsregisters_t;



volatile tsregisters_t ts_data __SYSTEM_GLOBAL__;
volatile int ts_status __SYSTEM_GLOBAL__;

#define TS_STAT_DATAREADY   1
#define TS_STAT_DATAMISSED  2






// Send bytes to device address 'addr'

int i2c_txdata(int addr,uint8_t *data, int nbytes)
{
    //  Set Master Transmit mode, STOP signal, enable Rx/Tx
    *IICSTAT0 = 0xd0 ;

    // Write slave address
    *IICDS0 = addr&0xff;

    // Clear interrupt pending
    *IICCON0 = (*IICCON0 & ~0x10);

    // START transmit
    *IICSTAT0 = 0xf0 ;

    while(nbytes>0) {

        // Wait for an interrupt
        while(! (*IICCON0 & 0x10));

        // If there was any error, return the number of bytes still pending
        if(*IICSTAT0&0x9) {
            // STOP transmission
            *IICSTAT0=0xd0;
            // Clear interrupt pending
            *IICCON0 = (*IICCON0 & ~0x10);

            return nbytes;
        }

        // Set next byte
        *IICDS0 = *data++;

        // Clear interrupt pending to resume transmission
        *IICCON0 = (*IICCON0 & ~0x10);

        --nbytes;
    }

    // Wait for an interrupt
    while(! (*IICCON0 & 0x10));

    // If there was any error, return the number of bytes still pending
    if(*IICSTAT0&0x9) nbytes=1;
    // STOP transmission
    *IICSTAT0=0xd0;
    // Clear interrupt pending
    *IICCON0 = (*IICCON0 & ~0x10);

    return nbytes;

}

// Read bytes from device address 'addr'

int i2c_rxdata(int addr,uint8_t *data, int nbytes)
{
    //  Set Master Receive mode, STOP signal, enable Rx/Tx
    *IICSTAT0 = 0x90 ;

    // Write slave address
    *IICDS0 = addr&0xff;

    // Clear interrupt pending
    *IICCON0 = (*IICCON0 & ~0x10);

    // START transmit
    *IICSTAT0 = 0xb0 ;

    while(nbytes>0) {

        // Wait for an interrupt
        while(! (*IICCON0 & 0x10));

        // If there was any error, return the number of bytes still pending
        if(*IICSTAT0&0x9) {
            // STOP transmission
            *IICSTAT0=0x90;
            // Clear interrupt pending
            *IICCON0 = (*IICCON0 & ~0x10);

            return nbytes;
        }

        // Read next byte
        *data++ = *IICDS0;

        // Clear interrupt pending to resume transmission
        *IICCON0 = (*IICCON0 & ~0x10);

        --nbytes;
    }

    // Wait for an interrupt
    while(! (*IICCON0 & 0x10));

    // If there was any error, return the number of bytes still pending
    if(*IICSTAT0&0x9) nbytes=1;
    // STOP transmission
    *IICSTAT0=0x90;
    // Clear interrupt pending
    *IICCON0 = (*IICCON0 & ~0x10);

    return nbytes;

}


// IRQ handler
// Read all registers from the touchscreen
void ts_update()
{
    if(ts_status&TS_STAT_DATAREADY) ts_status|=TS_STAT_DATAMISSED;

    if(i2c_rxdata(TS_I2CADDR,(uint8_t *)&ts_data,sizeof(TS_I2CADDR))==0) ts_status|=TS_STAT_DATAREADY;
    else ts_status=0;
}

void ts_init()
{
    // Set GPE14 and 15 for IIC function
    *GPECON = (*GPECON & ~0xf0000000) | 0xA0000000 ;
    // Disable Pull ups/down on both lines
    *GPEUDP = (*GPECON & ~0xf0000000) ;

    // Set GPF0 as output, GPF2 as EINT2
    *GPFCON = (*GPFCON & ~0x33) | 0x12 ;
    // Disable pull up/down
    *GPFUDP = (*GPFCON & ~0x33) ;

    // Power up the TS Chip
    *GPFDAT|=1;


    // With PCLK = 50 MHz, use IICCLK = PCLK/16 = 3.125 MHz
    // and Tx Clock = IICCLK / 8 = 390 kHz (NT11002 datasheet says Max = 400 kHz)
    // Enable ACK bit
    *IICCON0 = 0xA7 ;

    //  Set Master Receive mode, STOP signal, disable Rx/Tx
    *IICSTAT0 = 0x80 ;

    // Single-Master does not need an address
    *IICADD0 = 0 ;

    // Enable filter with 15 PCLKs delay
    *IICLC0 = 0x7 ;

    ts_status=0;

    __irq_addhook(2,&ts_update);

    __irq_unmask(2);

}




