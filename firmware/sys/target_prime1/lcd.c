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

int lcd_setmode(int mode, unsigned int *physbuf)
{
    // Disable video signals immediately
    *VIDCON0 = *VIDCON0 & 0xFFFFFFFC;

    // Disable LCD controller palette access
    *WPALCON = *WPALCON | 0x00000200;

    // palette data format for window 0 8:8:8
    *WPALCON = (*WPALCON & 0xFFFFFFF8) | 1;

    uint32_t color = 0x00FFFFFF;
    for (int i = 0; i < 16; ++i) {
        // bitswap rotates the bits of a nibble
        int offset = ((i & 0x1) << 3) | ((i & 0x2) << 1) | ((i & 0x4) >> 1) | ((i & 0x8) >> 3);
        *(WIN0Palette + offset) = color;
        color -= 0x00111111;
    }

    // Allow LCD controller access to palette
    *WPALCON = *WPALCON & 0xFFFFFDFF;

    *VIDW00ADD0B0 = (unsigned int)physbuf;

    // fixed buffer0 with bit swap enabled
    *WINCON0 = (*WINCON0 & 0xFE38FFC3) | 0x00040000 | (mode << 2);
}
