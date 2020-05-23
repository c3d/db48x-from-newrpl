/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <hal_api.h>
#include <ggl.h>
#include <stdio.h>
#include "nand.h"
#include "hal_api.h"

#define lineheight 12
#define font (UNIFONT const *)Font_10A
#define black 0xffffffff
#define left 0
#define right 160

static int line;
static gglsurface surface;

void tohex(uint32_t value, char *buffer) {
    buffer[8] = 0;

    for (int i = 7; i >= 0; --i) {
        buffer[i] = "0123456789ABCDEF"[value % 16];
        value = value / 16;
    }
}

static void tobin(uint32_t value, char *buffer) {
    buffer[32] = 0;

    for (int i = 31; i >= 0; --i) {
        buffer[i] = "01"[value % 2];
        value = value / 2;
    }
}

void printline(char *left_text, char *right_text) {
    if (left_text) {
        DrawText(left, line * lineheight, left_text, font, black, &surface);
    }
    if (right_text) {
        DrawText(right, line * lineheight, right_text, font, black, &surface);
    }
    if (left_text || right_text) {
        ++line;
    }
}

__ARM_MODE__ void startup(int) __attribute__((noreturn));
void startup(int prevstate)
{
    initContext(32);
    Context.alloc_bmp = EMPTY_STORAGEBMP;
    init_simpalloc();
    line = 0;

    lcd_setmode(BPPMODE_4BPP, (unsigned int *)MEM_PHYS_SCREEN);
    lcd_on();

    ggl_initscr(&surface);
    ggl_rect(&surface, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, 0);

    // Playing it save for testing
    NANDWriteProtect();

    static char buffer[9];

    for (uint32_t i = 0; i < 0x10000000; i += 0x20000) {
        if (!NANDIsBlockValid(i)) {
            tohex(i, buffer);
            printline(buffer, 0);
        }
    }

    while(1);
}

//************************************************************************************
//****** THESE ARE STUBS FROM NEWRPL, REMOVE AS SOON AS THEY ARE IMPLEMENTED *********

int halGetFreePages()
{
    return 1;
}
int halGetTotalPages()
{
    return 1;
}

void halReset()
{
    while(1);
}

void halWipeoutWarmStart()
{
    while(1);
}

void halWarmStart()
{
    while(1);
}

uint32_t RPLLastOpcode;

//********* END OF STUBS *************************************************************
//************************************************************************************
//************************************************************************************
