/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

int __lcd_contrast __SYSTEM_GLOBAL__;



void lcd_sync()
{
}

void lcd_off()
{
    // Disable backlight
    *GPBDAT = *GPBDAT & ~0x00000002;

    // Disable video signals immediately
    *VIDCON0 = *VIDCON0 & 0xFFFFFFFC;
}


void lcd_on()
{
    // Enable backlight
    *GPBDAT = *GPBDAT | 0x00000002;

    // Enable video output and logics
    *VIDCON0 |= 0x3;
}

void lcd_save(unsigned int *buf)
{
}

void lcd_restore(unsigned int *buf)
{
}

void lcd_setcontrast(int level)
{
}
#define BKGND_RED 172
#define BKGND_GREEN 222
#define BKGND_BLUE 157
#define FOGND_RED  0
#define FOGND_GREEN  0
#define FOGND_BLUE  0



/*
 * GPH4,5,6 = OUTPUT, DISABLE PULLUP/DOWN, LEAVE IN LOW STATE
 * LCD SPI DRIVER IS CONNECTED TO GPH4,5,6
 * GPH6 = CLK
 * GPH5 = CS
 * GPH4 = DATA
 *
 * LCD SPI INITIALIZATION SEQUENCE:
 * 0XCCDD WHERE CC=CMD, DD=DATA
 *
 * 0X0114
 * 0X0234
 * 0X10A7
 * 0X1155
 * 0X1271
 * 0X1371
 * 0X1473
 * 0X1555
 * 0X1618
 * 0X1762
 *
*/

#define SET_CLK_HIGH *GPHDAT|=0x40
#define SET_CLK_LOW  *GPHDAT&=~0x40

#define SET_CS_HIGH  *GPHDAT|=0x20
#define SET_CS_LOW   *GPHDAT&=~0x20

#define SET_DATA(a)  *GPHDAT=(*GPHDAT&~0x10)|((a)? 0x10:0)

#define DELAY_ONETICK   tmr_delayus(1000)

void lcd_sendi2c(int cmd,int data)
{
    SET_CLK_HIGH;

    DELAY_ONETICK;

    SET_CS_LOW;

    SET_CLK_LOW;

    DELAY_ONETICK;

    SET_DATA(0);    // Start bit

    DELAY_ONETICK;

    SET_CLK_HIGH;

    DELAY_ONETICK;

    for( int i=0;i<7;++i) {
        SET_CLK_LOW;

        SET_DATA(cmd&0x40);

        cmd<<=1;

        DELAY_ONETICK;

        SET_CLK_HIGH;

        DELAY_ONETICK;

    }

    for( int i=0;i<8;++i) {
        SET_CLK_LOW;

        SET_DATA(data&0x80);

        data<<=1;

        DELAY_ONETICK;

        SET_CLK_HIGH;

        DELAY_ONETICK;

    }

    SET_CS_HIGH;

}


void lcd_initspidisplay()
{
    *GPHCON=(*GPHCON&~0x3f00)|0x1500;   // SET GPH4,5,6 AS OUTPUT

    *GPHUDP&=0x3f00;                    // DISABLE PULLUP/DOWN


    SET_DATA(0);
    SET_CLK_LOW;
    SET_CS_HIGH;

    DELAY_ONETICK;

    lcd_sendi2c(0x1,0x14);
    lcd_sendi2c(0x2,0x34);
    lcd_sendi2c(0x10,0xa7);
    lcd_sendi2c(0x11,0x55);
    lcd_sendi2c(0x12,0x71);
    lcd_sendi2c(0x13,0x71);
    lcd_sendi2c(0x14,0x73);
    lcd_sendi2c(0x15,0x55);
    lcd_sendi2c(0x16,0x18);
    lcd_sendi2c(0x17,0x62);


}


// BOOTLOADER VALUES FOR ORIGINAL MODE:
// VIDCON0 = 0X5270
// WINCON0 = 0X2D
// VIDOSD0A = 0
// VIDOSD0B = 0XEFA7F
// VIDINTCON =  0X3F0102D
// VIDW00ADD0B0 = 0x31a00000
// VIDW00ADD1B0 = 0XA0000
// VIDW00ADD0B1 = 0x31a00000
// VIDW00ADD1B1 = 0XA0000
// VIDTCON0 = 0x110300
// VIDTCON1 = 0X401100
// VIDTCON2 = 0X7793F
// VIDCON1 = 0X80

// ORIGINAL LCD PROGRAMMING IS WITH HCLK = 66 MHZ

// RGB_VCLK = 66,666,666 / (9+1) = 6,666,666 Hz
// Display configuration:
// Sync pulse width: Vertical VSPW= 0 (+1), Horizontal HSPW= 0 (+1)
// Horizontal Front Porch HFPD=17 (+1), Back Porch = 64 (+1)
// Vertical Front Porch VFPD=3 (+1), Back Porch = 17 (+1)

// Total clock pulses per line: 320 + 1 (sync) + 18 (front porch) + 65 (back porch) = 404 ticks per line
// Total lines: 240 + 1 (sync) + 4 (front porch) + 18 (back porch) = 263 lines

// Total ticks per frame = 404*263 = 106,252 ticks * 3 (serial RGB interface takes 3 ticks per pixel) = 318,756 ticks total

// Vertical refresh rate of 60 Hz would require a RGB_VCLK = 60 * 318,756 = 19,125,360 Hz

#define LCD_TARGET_REFRESH      55

// UPDATE THE LCD CLOCK WHEN THE CPU CLOCK CHANGES
void __lcd_fix()
{
    int clkdiv=(__cpu_getHCLK()<<3)/(318756*LCD_TARGET_REFRESH);

    clkdiv+=7;
    clkdiv>>=3; // Round to closest integer to stay as close to target as possible

    *VIDCON0 = 0x5030 | ((clkdiv&0x3f)<<6); // Serial R->G->B, CLKVALUP=Start of frame, VCLKEN=Enabled, CLKDIR=Divided using CLKVAL_F, CLKSEL_F = Use HCLK source

}



int lcd_setmode(int mode, unsigned int *physbuf)
{
    // Disable video signals immediately
    *VIDCON0 = *VIDCON0 & 0xFFFFFFFC;

    lcd_initspidisplay();

    int clkdiv=(__cpu_getHCLK()<<3)/(318756*LCD_TARGET_REFRESH);

    clkdiv+=7;
    clkdiv>>=3; // Round to closest integer to stay as close to target as possible

    *VIDCON0 = 0x5030 | ((clkdiv&0x3f)<<6); // Serial R->G->B, CLKVALUP=Start of frame, VCLKEN=Enabled, CLKDIR=Divided using CLKVAL_F, CLKSEL_F = Use HCLK source
    *VIDCON1 = 0x80; // All pulses normal, use VCLK rising edge


    *VIDTCON0 = 0x110300;    // Set Vertical Front/Back porch and sync
    *VIDTCON1 = 0x3f1100;   // Set Horizontal Front/Back porch and sync
    *VIDTCON2 = 0x7793f;    // Set screen size 320x240

    *VIDOSD1A = *VIDOSD0A = 0;          // Window 0/1 top left corner = (0,0)
    *VIDOSD1B = *VIDOSD0B = 0xa00f0;    // Window 0/1 lower corner = (320,240)

    *VIDOSD1C = 0;

    *WIN1MAP = *WIN0MAP = 0;                       // Disable colormap

    // Disable LCD controller palette access
    *WPALCON = *WPALCON | 0x00000200;

    // palette data format for window 0 8:8:8
    *WPALCON = (*WPALCON & 0xFFFFFFF8) | 1;

    int red,green,blue;
    uint32_t color;
    for (int i = 0; i < 16; ++i) {
        // bitswap rotates the bits of a nibble
        int offset = ((i & 0x1) << 3) | ((i & 0x2) << 1) | ((i & 0x4) >> 1) | ((i & 0x8) >> 3);

        // linear color interpolation between foreground and background
        red= 128 + BKGND_RED * 256 + ((FOGND_RED-BKGND_RED) * (i+1) * 256) / 16 ;
        green= 128 + BKGND_GREEN * 256 + ((FOGND_GREEN-BKGND_GREEN) * (i+1) * 256) / 16 ;
        blue= 128 + BKGND_BLUE * 256 + ((FOGND_BLUE-BKGND_BLUE) * (i+1) * 256) / 16 ;
        color = ((red&0xff00)<<8)|((green&0xff00))|((blue&0xff00)>>8);

        *(WIN0Palette + offset) = color;
    }

    // Allow LCD controller access to palette
    *WPALCON = *WPALCON & 0xFFFFFDFF;

    *VIDW00ADD0B0 = (unsigned int)physbuf;

    switch(mode)
    {
    case BPPMODE_1BPP:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (SCREEN_WIDTH*SCREEN_HEIGHT)>>3;
        *VIDW00ADD2B0 = (((SCREEN_WIDTH)>>3)+15)&~0xf; // Set PAGEWIDTH aligned to 4-words bursts, no OFFSET
        *WINCON0 = 0x40000;     // Enable bit swap
        break;
    case BPPMODE_2BPP:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (SCREEN_WIDTH*SCREEN_HEIGHT)>>2;
        *VIDW00ADD2B0 = (((SCREEN_WIDTH)>>2)+15)&~0xf; // Set PAGEWIDTH aligned to 4-words bursts, no OFFSET
        *WINCON0 = 0x40000;     // Enable bit swap
        break;
    case BPPMODE_4BPP:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (SCREEN_WIDTH*SCREEN_HEIGHT)>>1;
        *VIDW00ADD2B0 = (((SCREEN_WIDTH)>>1)+15)&~0xf; // Set PAGEWIDTH aligned to 4-words bursts, no OFFSET
        *WINCON0 = 0x40000;     // Enable bit swap
        break;
    case BPPMODE_8BPP:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (SCREEN_WIDTH*SCREEN_HEIGHT);
        *VIDW00ADD2B0 = (((SCREEN_WIDTH))+15)&~0xf; // Set PAGEWIDTH aligned to 4-words bursts, no OFFSET
        *WINCON0 = 0;     // Disable bit swap
        break;
    case BPPMODE_16BPP565:
    case BPPMODE_16BPP555:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (SCREEN_WIDTH*SCREEN_HEIGHT)*2;
        *VIDW00ADD2B0 = (((SCREEN_WIDTH)<<1)+15)&~0xf; // Set PAGEWIDTH aligned to 4-words bursts, no OFFSET
        *WINCON0 = 0;     // Disable bit swap
        break;
    case BPPMODE_18BPP:
    case BPPMODE_24BPP:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (SCREEN_WIDTH*SCREEN_HEIGHT)*4;
        *VIDW00ADD2B0 = (((SCREEN_WIDTH)<<2)+15)&~0xf; // Set PAGEWIDTH aligned to 4-words bursts, no OFFSET
        *WINCON0 = 0;     // Disable bit swap
        break;
    }

    *WINCON1=0; // Disable Window 1
    // Set final mode, set 4-word burst length, and enable window 0
    // Preserve bit swap as set for each mode
    *WINCON0 = (*WINCON0&0x40000) | 0x401  | ((mode&0xf) << 2);


}
