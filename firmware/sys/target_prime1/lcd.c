/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

void halScreenUpdated()
{
}

void __lcd_fix()
{
}

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
    *VIDCON0 = (*VIDCON0 & 0xFFFFFFFC) | 0x3;
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

int lcd_setmode(int mode, unsigned int *physbuf)
{
    // Disable video signals immediately
    *VIDCON0 = *VIDCON0 & 0xFFFFFFFC;

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
        break;
    case BPPMODE_2BPP:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (SCREEN_WIDTH*SCREEN_HEIGHT)>>2;
        break;
    case BPPMODE_4BPP:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (SCREEN_WIDTH*SCREEN_HEIGHT)>>1;
        break;
    case BPPMODE_8BPP:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (SCREEN_WIDTH*SCREEN_HEIGHT);
        break;
    case BPPMODE_16BPP565:
    case BPPMODE_16BPP555:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (SCREEN_WIDTH*SCREEN_HEIGHT)*2;
        break;
    case BPPMODE_18BPP:
    case BPPMODE_24BPP:
        *VIDW00ADD1B0 = (((unsigned int)physbuf)&0x00ffffff) + (SCREEN_WIDTH*SCREEN_HEIGHT)*4;
        break;
    }

    // set buffer1 immediately after the screen, with 4kpage alignment for exception handlers
    *VIDW00ADD0B1 = ((unsigned int)physbuf& 0xff000000) + ((*VIDW00ADD1B0 + 0xfff) & 0x00fff000);
    *VIDW00ADD1B1 = (*VIDW00ADD0B1 + (*VIDW00ADD1B0 - (*VIDW00ADD0B0&0x00ffffff)))&0x00ffffff;

    // fixed buffer0 with bit swap enabled
    *WINCON0 = (*WINCON0 & 0xFF38FFC3) | 0x00040001 | ((mode&0xf) << 2);
}
