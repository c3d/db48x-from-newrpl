/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <hal_api.h>
#include <ggl.h>
#include "fsystem.h"
#include "nand.h"

static const int lineheight = 12;
static UNIFONT const * const font = (UNIFONT const *)Font_10A;
static const int black = 0xffffffff;
static const int left = 0;
static const int right = 160;
static int line = 0;
static char left_buffer[33];
static char right_buffer[33];
static gglsurface scr;

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
    DrawText(left, line * lineheight, left_text, font, black, &scr);
    DrawText(right, line * lineheight, right_text, font, black, &scr);
    ++line;
}

__ARM_MODE__ void startup(int) __attribute__((noreturn));
void startup(int prevstate)
{
    lcd_setmode(BPPMODE_4BPP, (unsigned int *)MEM_PHYS_SCREEN);
    lcd_on();

    ggl_initscr(&scr);
    ggl_rect(&scr, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, 0);

    // Playing it save for testing
    NANDWriteProtect();

    initContext(32);
    Context.alloc_bmp = EMPTY_STORAGEBMP;
    init_simpalloc();

    BINT err;

    err = FSSetCurrentVolume(0);
    if(err != FS_OK) {
        tohex(err, right_buffer);
        printline("Could not set volume", right_buffer);
    }

    FS_FILE *directory;
    err = FSOpenDir(".", &directory);

    if((err != FS_OK) && (err != FS_OPENDIR)) {
        tohex(err, right_buffer);
        printline("Could not open dir", right_buffer);
    }

    FS_FILE entry;
    while(1) {
        err = FSGetNextEntry(&entry, directory);

        if (err == FS_EOF) {
            break;
        }
        
        if(err != FS_OK) {
            tohex(err, right_buffer);
            printline("Could not get next entry", right_buffer);
            break;
        }

        printline("", entry.Name);
        FSReleaseEntry(&entry);
    }
    
    FSClose(directory);

    while(1);
}
