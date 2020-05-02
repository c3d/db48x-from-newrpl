/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <hal_api.h>
#include <ggl.h>

static const int lineheight = 12;
static UNIFONT const * const font = (UNIFONT const *)Font_10A;
static const int black = 0xffffffff;
static const int left = 0;
static const int right = 64;

static void printhex(gglsurface *surface, int line, char *text, uint32_t value) {
    char buffer[8];

    for (int i = 7; i >= 0; --i) {
        buffer[i] = "0123456789ABCDEF"[value % 16];
        value = value / 16;
    }

    DrawText(left, line * lineheight, text, font, black, surface);
    DrawTextN(right, line * lineheight, buffer, buffer + 8, font, black, surface);
}

static void printbin(gglsurface *surface, int line, char *text, uint32_t value) {
    char buffer[32];

    for (int i = 31; i >= 0; --i) {
        buffer[i] = "01"[value % 2];
        value = value / 2;
    }

    DrawText(left, line * lineheight, text, font, black, surface);
    DrawTextN(right, line * lineheight, buffer, buffer + 32, font, black, surface);
}

__ARM_MODE__ unsigned int get_c1()
{
    register unsigned int c1;

    asm volatile ("mrc p15, 0, %0, c1, c0, 0":"=r" (c1));

    return c1;
}

__ARM_MODE__ void startup(int) __attribute__((noreturn));
void startup(int prevstate)
{
    lcd_setmode(BPPMODE_4BPP, (unsigned int *)MEM_PHYS_SCREEN);
    lcd_on();

    gglsurface scr;
    ggl_initscr(&scr);
    ggl_rect(&scr, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, 0);

    int line = 0;
    printhex(&scr, line++, "NFCONF", *NFCONF);
    printhex(&scr, line++, "NFCONT", *NFCONT);
    printbin(&scr, line++, "c1", get_c1());

    while(1);
}
