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
#include "fsystem.h"
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

int n_pressed() {
    *GPGCON = 0;    // SET ALL KEYBOARD COLUMNS AS INPUTS
    *GPDCON = (*GPDCON & 0xffff0000) | 0X5555;   // ALL ROWS TO OUTPUT

    *GPDDAT &= 0xffff0000;    // ALL ROWS LOW
    *GPDDAT |= (1 << 7);

    return *GPGDAT & (1 << 4);
}

__ARM_MODE__ void startup(int) __attribute__((noreturn));
void startup(int prevstate)
{
    // line = 0; bss

    lcd_setmode(BPPMODE_4BPP, (unsigned int *)MEM_PHYS_SCREEN);
    lcd_on();

    ggl_initscr(&surface);
    ggl_rect(&surface, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, 0);

    // Playing it save for testing
    NANDWriteProtect();

    initContext(32);
    Context.alloc_bmp = EMPTY_STORAGEBMP;
    init_simpalloc();

    BINT err;

    err = FSSetCurrentVolume(0);
    if (err != FS_OK) {
        throw_exception("Could not set volume", err);
    }

    FS_FILE *fileptr;

    err = FSOpen(
            (n_pressed()) ? "NEWRPL.ROM" : "PRIME_OS.ROM",
            FSMODE_READ | FSMODE_NOCREATE, &fileptr);
    if (err != FS_OK) {
        throw_exception("Could not open file", err);
    }

    struct Preamble preamble;
    err = FSRead((unsigned char *)&preamble, 32, fileptr);
    if (err != 32) {
        throw_exception("Could not read preamble", 0);
    }

    err = FSSeek(fileptr, 0, SEEK_SET);
    if (err != FS_OK) {
        throw_exception("Could not rewind", err);
    }

    err = FSRead((unsigned char *)preamble.load_addr, preamble.copy_size, fileptr);
    if (err != preamble.copy_size) {
        throw_exception("Could not read data", 0);
    }

    err = FSClose(fileptr);
    if (err != FS_OK) {
        // Can't be closed due to missing write support
    }

    // call payload
    ((void (*)(void))preamble.entrypoint)();
    
    while(1);
}
