/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <ui.h>

#define LCD_TARGET_FREQ 500000
#define LCD_W SCREEN_W
#define HOZVAL ((LCD_W>>2)-1)

// SIMULATED SYSTEM REGISTERS
int __lcd_mode = -1;
int __lcd_needsupdate = 0;
unsigned int *__lcd_buffer;
// SIMULATED SCREEN MEMORY
char PhysicalScreen[(SCREEN_W*SCREEN_H)*4/PIXELS_PER_WORD];
char ExceptionScreen[(SCREEN_W*SCREEN_H)*4/PIXELS_PER_WORD];

int __lcd_contrast __SYSTEM_GLOBAL__;

void halScreenUpdated()
{
    // SEND SIGNAL THAT EMULATED SCREEN NEEDS TO BE UPDATED
    __lcd_needsupdate = 1;
}

void __lcd_fix()
{
}

void lcd_sync()
{
}

void lcd_off()
{
// TODO: TURN LCD OFF IN QT EMULATED SCREEN
}

void lcd_on()
{
    //TODO: TURN LCD ON IN QT EMULATED SCREEN

}

void lcd_save(unsigned int *buf)
{
    *buf = __lcd_mode;
    unsigned int **ptr = (unsigned int **)&(buf[1]);
    *ptr = __lcd_buffer;
}

void lcd_restore(unsigned int *buf)
{
    unsigned int **ptr = (unsigned int **)&(buf[1]);
    __lcd_buffer = *ptr;
    __lcd_mode = *buf;
}

void lcd_setcontrast(int level)
{
//    int value;
    if(level > 15 || level < 0)
        level = 7;

// TODO: ADJUST CONTRAST IN EMULATED SCREEN
}

// SETS VIDEO MODE AND RETURNS WIDTH OF THE SCREEN IN BYTES

int lcd_setmode(int mode, unsigned int *physbuf)
{

// mode=0 -> Mono
//     =1 -> 4-gray
//     =2 -> 16-gray
//     =3 -> 64k COLOR RGB 5-6-5
// physbuf MUST be the physical address

    int pagewidth;

    if(mode<3) pagewidth = LCD_W >> (3 - mode);
    else pagewidth = LCD_W * 2;

    __lcd_buffer = physbuf;
    lcd_setcontrast(__lcd_contrast);
    __lcd_mode = mode;

    return pagewidth;
}
