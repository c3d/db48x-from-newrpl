/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

int lcd_contrast SYSTEM_GLOBAL;


// Function supplied by CPU module, returns the hardware clock frequency in Hertz
extern int cpu_getHCLK();


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
    buf[0]=*WINCON0;        // SAVE STATE OF WINDOW 0
    buf[1]=*VIDW00ADD0B0;
    buf[2]=*VIDW00ADD1B0;
    buf[3]=*VIDW00ADD2B0;
    buf[4]=*VIDW00ADD0B1;
    buf[5]=*VIDW00ADD1B1;
    buf[6]=*VIDW00ADD2B1;
    buf[7]=*VIDOSD0A;
    buf[8]=*VIDOSD0B;

}

void lcd_restore(unsigned int *buf)
{
    *WINCON0&=~1;            // WINDOW 0 OFF
    *VIDW00ADD0B0=buf[1];
    *VIDW00ADD1B0=buf[2];
    *VIDW00ADD2B0=buf[3];
    *VIDW00ADD0B1=buf[4];
    *VIDW00ADD1B1=buf[5];
    *VIDW00ADD2B1=buf[6];
    *VIDOSD0A=buf[7];
    *VIDOSD0B=buf[8];
    *WINCON0=buf[0];        // RESTORE WINDOW 0
}

void lcd_set_contrast(int level)
{
}
#define BKGND_RED 172
#define BKGND_GREEN 222
#define BKGND_BLUE 157
#define FOGND_RED  0
#define FOGND_GREEN  0
#define FOGND_BLUE  0







extern unsigned int cpu_getPCLK();







#define SET_CLK_HIGH *GPHDAT|=0x40
#define SET_CLK_LOW  *GPHDAT&=~0x40

#define SET_CS_HIGH  *GPHDAT|=0x20
#define SET_CS_LOW   *GPHDAT&=~0x20

#define SET_DATA(a)  *GPHDAT=(*GPHDAT&~0x10)|((a)? 0x10:0)

#define DELAY_ONETICK   tmr_delay100us(); tmr_delay100us()
#define DELAY_10MS      tmr_delay10ms()
#define DELAY_20MS      tmr_delay20ms()



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

void lcd_initspidisplay()
{
    lcd_sendi2c(0x1,0x14);      // Set VCOM amplitude to x1.10
    lcd_sendi2c(0x2,0x34);      // Set VCOM High voltage to x0.89

    // Set various gamma correction curves
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

// As per ILI9322 datasheet: Back porch is 241 ticks. Needs correction to 240 using register 9 to make it multiple of 3.
// Total then is 240/3 = 80 (includes the sync pulse).
// Using a 1 pulse for sync, the value to use for VIDTCON is HBPD=79 (+sync)
// HFPD should then be: 1560 - 3*320 - 240 = 360; HFPD=360/3 = 119 (+1)

// Vertical back porch = 18 lines (incl. 1 line for VSYNC), hence VBPD = 17 (+1)
// Front porch is 4 typical and sync should be 1 line only (VFPD = 3 (+1) , VSPW = 0 (+1))

// Total lines: 240 + 1 (sync) + 17 (front porch) + 4 (back porch) = 262 lines

// Total ticks per frame = (320+80+120) * 262 = 136240 ticks * 3 (serial RGB interface takes 3 ticks per pixel) = 408720 ticks total

// Vertical refresh rate of 60 Hz would require a RGB_VCLK = 60 * 408720 = 24,523,200 Hz

#define LCD_TARGET_REFRESH      55

// UPDATE THE LCD CLOCK WHEN THE CPU CLOCK CHANGES
void lcd_fix()
{
    int clkdiv=(cpu_getHCLK()<<3)/(408720*LCD_TARGET_REFRESH);

    clkdiv+=7;
    clkdiv>>=3; // Round to closest integer to stay as close to target as possible

    *VIDCON0 = 0x5030 | ((clkdiv&0x3f)<<6); // Serial R->G->B, CLKVALUP=Start of frame, VCLKEN=Enabled, CLKDIR=Divided using CLKVAL_F, CLKSEL_F = Use HCLK source

}







int lcd_setmode(int mode, unsigned int *physbuf)
{
    // Disable video signals immediately
    *VIDCON0 = *VIDCON0 & 0xFFFFFFFC;


    int clkdiv=(cpu_getHCLK()<<3)/(408720*LCD_TARGET_REFRESH);

    clkdiv+=7;
    clkdiv>>=3; // Round to closest integer to stay as close to target as possible

    *VIDCON0 = 0x5030 | ((clkdiv&0x3f)<<6); // Serial R->G->B, CLKVALUP=Start of frame, VCLKEN=Enabled, CLKDIR=Divided using CLKVAL_F, CLKSEL_F = Use HCLK source
    *VIDCON1 = 0x80; // All pulses normal, use VCLK rising edge


    *VIDTCON0 = 0x110300;    // Set Vertical Front/Back porch and sync (17 + 4 + 1)
    *VIDTCON1 = 0x4f7700;   // Set Horizontal Front/Back porch and sync (79 + 120 + 1)
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
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (LCD_W*LCD_H)>>3;
        *VIDW00ADD2B0 = (((LCD_W)>>3)+15)&~0xf; // Set PAGEWIDTH aligned to 4-words bursts, no OFFSET
        *VIDW00ADD0B1 = *VIDW00ADD0B0 + (LCD_W*LCD_H)>>3;
        *VIDW00ADD1B1 = *VIDW00ADD1B0 + (LCD_W*LCD_H)>>3;
        *VIDW00ADD2B1 = *VIDW00ADD2B0 ;
        *WINCON0 = 0x40000;     // Enable bit swap
        break;
    case BPPMODE_2BPP:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (LCD_W*LCD_H)>>2;
        *VIDW00ADD2B0 = (((LCD_W)>>2)+15)&~0xf; // Set PAGEWIDTH aligned to 4-words bursts, no OFFSET
        *VIDW00ADD0B1 = *VIDW00ADD0B0 + (LCD_W*LCD_H)>>2;
        *VIDW00ADD1B1 = *VIDW00ADD1B0 + (LCD_W*LCD_H)>>2;
        *VIDW00ADD2B1 = *VIDW00ADD2B0 ;
        *WINCON0 = 0x40000;     // Enable bit swap
        break;
    case BPPMODE_4BPP:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (LCD_W*LCD_H)>>1;
        *VIDW00ADD2B0 = (((LCD_W)>>1)+15)&~0xf; // Set PAGEWIDTH aligned to 4-words bursts, no OFFSET
        *VIDW00ADD0B1 = *VIDW00ADD0B0 + (LCD_W*LCD_H)>>1;
        *VIDW00ADD1B1 = *VIDW00ADD1B0 + (LCD_W*LCD_H)>>1;
        *VIDW00ADD2B1 = *VIDW00ADD2B0 ;

        *WINCON0 = 0x40000;     // Enable bit swap
        break;
    case BPPMODE_8BPP:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (LCD_W*LCD_H);
        *VIDW00ADD2B0 = (((LCD_W))+15)&~0xf; // Set PAGEWIDTH aligned to 4-words bursts, no OFFSET
        *VIDW00ADD0B1 = *VIDW00ADD0B0 + (LCD_W*LCD_H);
        *VIDW00ADD1B1 = *VIDW00ADD1B0 + (LCD_W*LCD_H);
        *VIDW00ADD2B1 = *VIDW00ADD2B0 ;

        *WINCON0 = 0x20000;     // Enable byte swap
        break;
    case BPPMODE_16BPP565:
    case BPPMODE_16BPP555:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (LCD_W*LCD_H)*2;
        *VIDW00ADD2B0 = (((LCD_W)<<1)+15)&~0xf; // Set PAGEWIDTH aligned to 4-words bursts, no OFFSET
        *VIDW00ADD0B1 = *VIDW00ADD0B0 + (LCD_W*LCD_H)*2;
        *VIDW00ADD1B1 = *VIDW00ADD1B0 + (LCD_W*LCD_H)*2;
        *VIDW00ADD2B1 = *VIDW00ADD2B0 ;

        *WINCON0 = 0x10000;     // Enable half-word swap
        break;
    case BPPMODE_18BPP:
    case BPPMODE_24BPP:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (LCD_W*LCD_H)*4;
        *VIDW00ADD2B0 = (((LCD_W)<<2)+15)&~0xf; // Set PAGEWIDTH aligned to 4-words bursts, no OFFSET
        *VIDW00ADD0B1 = *VIDW00ADD0B0 + (LCD_W*LCD_H)*4;
        *VIDW00ADD1B1 = *VIDW00ADD1B0 + (LCD_W*LCD_H)*4;
        *VIDW00ADD2B1 = *VIDW00ADD2B0 ;

        *WINCON0 = 0;     // Disable bit swap
        break;
    }

    *WINCON1=0; // Disable Window 1
    // Set final mode, set 4-word burst length, and enable window 0
    // Preserve bit swap as set for each mode
    *WINCON0 = (*WINCON0&0x70000) | 0x401  | ((mode&0xf) << 2);


}

// ILI9322 PowerOn Sequence to brign up the display
void lcd_poweron()
{

    // Setup GPIO

    *GPCCON = (*GPCCON&0xfc00) | 0xaaaa02aa;    // ALL GPC PINS SET FOR LCD FUNCTION
    *GPCUDP = (*GPCUDP&0xfc00);                 // PULL DOWN DISABLED ON ALL LCD PINS
    *GPDCON = (*GPDCON&0xffff0000) | 0xaaaa0000; // GPD 8 THRU 15 SET TO LCD FUNCTION
    *GPDUDP = (*GPDUDP&0xffff0000);              // PULL DOWN DISABLED
    *GPHCON=(*GPHCON&~0x3f00)|0x1500;   // SET GPH4,5,6 AS OUTPUT

    *GPHUDP&=0x3f00;                    // DISABLE PULLUP/DOWN

    *GPBCON = (*GPBCON & (~0xc030c)) | 0x40004;  // GPB9 SET TO OUTPUT (POWER TO LCD DRIVER CHIP), GPB4 = INPUT (LCD SPI COMMS DATA IN), GPB1 = OUTPUT (BACKLIGHT)
    *GPBUDP = (*GPBUDP & (~0xc030c)) | 0x400;    // GPB9 DISABLE PULLUP/DOWN, GPB4 = ENABLE PULL UP, GPB1 = DISABLE UP/DOWN

    *GPFCON = (*GPFCON & (~0x300)) | 0x100;      // GPF4 SET TO OUTPUT (POWER TO LCD DRIVER CHIP)
    *GPFUDP = (*GPFUDP & (~0x300));              // GPF4 DISABLE PULLUP/DOWN

    // TURN BACKLIGHT ON
    *GPBDAT = (*GPBDAT | 0x2);                 // GPB1 TURN BACKLIGHT ON

    // SET BOTH TO ZERO TO RESET THE CHIP
    *GPBDAT = (*GPBDAT & ~0x200);                 // GPB9 POWER UP THE LCD DRIVER CHIP
    *GPFDAT = (*GPFDAT & ~0x10);                  // GPF4 POWER UP THE LCD DRIVER CHIP


    // INITIALIZE THE LCD DRIVER CHIP THROUGH SPI CONNECTED TO GPH 4=DATA,5=CS,6=CLK
    // SET THE LINES TO VALID INITIAL STATES (IDLE)

    SET_DATA(1);
    SET_CLK_HIGH;
    SET_CS_HIGH;

    *GPBDAT = (*GPBDAT | 0x200);                 // GPB9 POWER UP THE LCD DRIVER CHIP (VCC/IOVCC)




    DELAY_ONETICK;

    *GPFDAT = (*GPFDAT | 0x10);                  // GPF4 POWER UP THE LCD DRIVER CHIP (nRESET)

    DELAY_20MS;

    lcd_sendi2c(0x4,0x0);                       // ISSUE ILI9322 GLOBAL RESET


    DELAY_10MS;

    // And image should appear after 10 to 80 frames
    lcd_sendi2c(0x30,0x4);                       // ISSUE ILI9322 power on control: Display image immediately

    lcd_sendi2c(0x7,0xEF);                       // ISSUE ILI9322 power on and out of Standby (typically done automatically, but done here again just in case)
    lcd_sendi2c(0x9,0x7F);                       // ISSUE ILI9322 back porch correction: CPU has a 3*VCLK resolution on pulses, therefore total horizontal period must be a multiple of 3, 1 clock correction is needed


    lcd_initspidisplay();                        // Setup various ILI9322 parameters



    // SETUP BASIC CLOCKS TO ENABLE A VALID VCLK - NO VIDEO MODE SETTINGS YET

    // Disable ALL video signals immediately
    *VIDCON0 = *VIDCON0 & 0xFFFFFFDC;


    int clkdiv=(cpu_getHCLK()<<3)/(332955*LCD_TARGET_REFRESH);

    clkdiv+=7;
    clkdiv>>=3; // Round to closest integer to stay as close to target as possible


    *VIDCON1 = 0x80; // All pulses normal, use VCLK rising edge

    *VIDTCON0 = 0x030000;    // Set Vertical Front/Back porch and sync
    *VIDTCON1 = 0x4e3400;   // Set Horizontal Front/Back porch and sync
    *VIDTCON2 = 0x7793f;    // Set screen size 320x240

    *VIDOSD1A = *VIDOSD0A = 0;          // Window 0/1 top left corner = (0,0)
    *VIDOSD1B = *VIDOSD0B = 0xa00f0;    // Window 0/1 lower corner = (320,240)

    *VIDOSD1C = 0;

    *WINCON0 = *WINCON1 = 0;                        // Disable both windows (just display black screen until video mode is set)

    *VIDCON0 = 0x5030 | ((clkdiv&0x3f)<<6); // Serial R->G->B, CLKVALUP=Start of frame, VCLKEN=Enabled, CLKDIR=Divided using CLKVAL_F, CLKSEL_F = Use HCLK source


    *VIDCON0|= 3;                                 // Enable all video signals and clocks



}

volatile int lcd_scanline()
{
int state=*VIDCON1;
if(state&0x6000 == 0x4000) return (state>>16)&0x3ff;
else return LCD_H+1;
}

void lcd_setactivebuffer(int buffer)
{
if(buffer) *WINCON0|=0x00800000;
else *WINCON0&=~0x00800000;
}

int lcd_getactivebuffer()
{
if(*WINCON0&0x00800000) return 1;
return 0;
}
