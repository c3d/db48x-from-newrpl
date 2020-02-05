/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

#define LCD_TARGET_FREQ 500000
#define LCD_W 160
#define HOZVAL ((LCD_W>>2)-1)

int __lcd_contrast __SYSTEM_GLOBAL__;

void __lcd_fix()
{
    volatile unsigned int *CLKPTR = (unsigned int *)CLK_REGS;
    volatile unsigned int *LCDPTR = (unsigned int *)LCD_REGS;
    int mpllcon = CLKPTR[1], clkslow = CLKPTR[4], clkdivn = CLKPTR[5];
// INITIAL CLOCK FREQUENCY
    int freq = 12000000, clkval;

// CALCULATE ACTUAL WORKING CALCULATOR FREQUENCY
    if(clkslow & 0x10) {
        int slow_val = clkslow & 7;
        if(slow_val)
            freq /= slow_val << 1;
    }
    else {
        int mdiv = (mpllcon >> 12) & 0xff, pdiv = (mpllcon >> 4) & 0x3f, sdiv =
                mpllcon & 0x3;
        freq *= mdiv + 8;
        freq /= pdiv + 2;
        freq >>= sdiv;
    }
// HERE FREQ=CPU CLOCK FREQUENCY FCLK

    if(clkdivn & 2)
        freq /= 2;

// HERE FREQ=HCLK

    clkval = freq / LCD_TARGET_FREQ;
    clkval += 1;
    clkval >>= 1;       // ROUND TOWARDS SLOWER SPEED

    int linecnt, lineblank;

// SET A REASONABLE LINEBLANK FOR SLOW HCLK
    lineblank = freq / 2000000;

// LCD OFF
    LCDPTR[0] &= 0xFFFFFFFE;
    linecnt = LCDPTR[0] & 0xff;
    linecnt |= clkval << 8;

// FIX CLKVAL
    LCDPTR[0] = linecnt;

// FIX LINEBLANK AND FORCE WDLY=0 FOR SLOW MODE, BUT ;
    LCDPTR[2] &= 0x7ff00;
    if(freq > 37500000)
        LCDPTR[2] |= 0x80000;   // USE WDLY=32 HCLK
    if(freq > 60000000)
        LCDPTR[2] |= 0x100000;  // USE WDLY=64 HCLK

    LCDPTR[2] |= lineblank;

// FORCE WLH=0;
    LCDPTR[3] &= 0xFFFFFF00;
    if(freq > 37500000)
        LCDPTR[3] |= 1;
    if(freq > 60000000)
        LCDPTR[3] |= 2;

// LCD ON
    LCDPTR[0] |= 1;

}

void lcd_sync()
{
    unsigned int volatile *LCDCON1 = (unsigned int *)LCD_REGS;

    // USE THE LOWER 5 LINES TO AVOID
    // NEXT FRAME FETCHED BEFORE WE ACT
    // WAIT UNTIL SCANNER IS OFF THE SCREEN (PROGRAMMED FOR 85 LINES)
    // BUT AT LEAST 2 SCANS BEFORE THE END OF FRAME, TO ALLOW
    // PROCESSING TIME BEFORE NEXT FRAME IS FETCHED
    while(((*LCDCON1) >> 18) > 2);
}

void lcd_off()
{
    lcd_sync();
    volatile unsigned int *LCDCON1 = (unsigned int *)LCD_REGS;
    *LCDCON1 = (*LCDCON1) & (0xFFFFFFFE);

    *IO_GPDCON = (*IO_GPDCON & ~0xC000) | 0x4000;       // SET GPD7 AS OUTPUT
    *IO_GPDUP &= ~0x80; // ENABLE PULLUPS
    *IO_GPDDAT &= ~0x80;        // DISCONNECT POWER TO THE LCD WITH GPD7

}

void lcd_on()
{

    volatile unsigned int *LCDCON1 = (unsigned int *)LCD_REGS;
    *LCDCON1 = (*LCDCON1) | 1;

    *IO_GPDCON = (*IO_GPDCON & ~0xC000) | 0x4000;       // SET GPD7 AS OUTPUT
    *IO_GPDUP &= ~0x80; // ENABLE PULLUPS
    *IO_GPDDAT |= 0x80; // ENABLE POWER TO THE LCD WITH GPD7
}

void lcd_save(unsigned int *buf)
{
// SAVE FIRST 11 REGISTERS
    memcpyw(buf, (int *)LCD_REGS, 11);

// SAVE LAST 6 REGISTERS
    memcpyw(buf + 11, (int *)(LCD_REGS + 0x4c), 6);
}

void lcd_restore(unsigned int *buf)
{
    register int *lcdr = (int *)LCD_REGS;
// DISABLE LCD WHILE PROGRAMMING IT
    *buf &= 0x3fffe;
// RESTORE FIRST 11 REGISTERS
    memcpyw(lcdr, buf, 11);

// RESTORE LAST 6 REGISTERS
    memcpyw(lcdr + 0x13, buf + 11, 6);

// TURN LCD ON
    *lcdr |= 1;

}

#define CONT_TXSTOP  0x200
#define CONT_BITREADY 0x2000
#define CONT_TX 0x1000

const unsigned short int const __lcdcontrast_table[16] = {
    0x43fc,
    0x43ff,
    0x44f8,
    0x44fc,
    0x44ff,
    0x46e9,
    0x46ea,
    0x46eb,
    0x45ff,
    0x46f6,
    0x46fa,
    0x46fd,
    0x47ef,
    0x47f4,
    0x47f8,
    0x47fc
};

void __lcd_txbyte(int byte)
{
    int f;
    *IO_GPDDAT &= ~CONT_TXSTOP; // CLEAR STOP BIT

    byte <<= 4;
    for(f = 0; f < 8; ++f) {
        byte <<= 1;
        *IO_GPDDAT &= ~CONT_BITREADY;   // CLEAR BIT READY
        *IO_GPDDAT = ((*IO_GPDDAT) & (~CONT_TX)) | (byte & CONT_TX);    // SET TX BIT
        *IO_GPDDAT |= CONT_BITREADY;    // SIGNAL ONE BIT IS READY;
    }
    // NOW CLEANUP TRANSMISSION
    *IO_GPDDAT &= ~(CONT_BITREADY | CONT_TX);

    *IO_GPDDAT |= CONT_TXSTOP;  // SET STOP BIT

    return;
}

void lcd_setcontrast(int level)
{
    int value;
    if(level > 15 || level < 0)
        level = 7;

    value = __lcdcontrast_table[level];
    __lcd_txbyte((value >> 8) & 0xff);
    __lcd_txbyte(value & 0xff);
    return;

}

int __lcd_setmode(int mode, unsigned int *physbuf)
{
    // mode=0 -> Mono
    //     =1 -> 4-gray
    //     =2 -> 16-gray
    // physbuf MUST be the physical address

    volatile unsigned int *lcdreg = (unsigned int *)LCD_REGS;
    int height = 80 /*(lcdreg[3])>>8 */ , pagewidth = LCD_W >> (4 - mode);

    // TURN OFF
    lcdreg[0] &= 0xfffffffe;

    // set LINEVAL to height+5-1 (like ROM does)
    // when LINECNT<5 is safe for updating display
    lcdreg[1] = (height + 4) << 14;
    // set HOZVAL, but leave LINEBLANK for sys_lcdfix()
    lcdreg[2] = HOZVAL << 8;

    // leave MVAL alone (used for screen height determination)

    // set proper byte swapping, ensure the rest at 0
    lcdreg[4] = (2 - mode);

    // set LCDBANK and LCDBASEU
    lcdreg[5] = ((unsigned int)physbuf) >> 1;

    // set LCDBASEL
    lcdreg[6] = (((unsigned int)physbuf) >> 1) + (height + 5) * pagewidth;

    // set PAGEWIDTH
    lcdreg[7] = pagewidth;

    // set palette lookup for 4-GRAY mode. ignored for other modes
    lcdreg[10] = 0xfa70;

    // set dither mode only when in grayscale
    if(mode)
        lcdreg[19] = 0x12210;
    else
        lcdreg[19] = 0;

    // set video mode, overwrite CLKVAL
    lcdreg[0] = 32 + (mode << 1);

    return pagewidth << 1;

}

// SETS VIDEO MODE AND RETURNS WIDTH OF THE SCREEN IN BYTES

int lcd_setmode(int mode, unsigned int *physbuf)
{
    int pagewidth = __lcd_setmode(mode, physbuf);

    // fix CLKVAL and other clock dependent constants
    // and turn on
    __lcd_fix();

    lcd_setcontrast(__lcd_contrast);

    return pagewidth;
}

void lcd_poweron()
{
    __lcd_setmode(2, (int *)MEM_PHYS_SCREEN);   // set default values

    *IO_GPDDAT = 0x300;

// send unknown init commands to lcd
    __lcd_txbyte(0);
    __lcd_txbyte(0x27);
    __lcd_txbyte(0x65);

    lcd_setcontrast(__lcd_contrast);

    __lcd_fix();        // fix frequency and enable video

    lcd_on();

}
