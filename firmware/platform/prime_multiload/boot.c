/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <stdio.h>
#include <newrpl.h>
#include <hal_api.h>
#include "fsystem.h"
#include "nand.h"
#include "hal_api.h"
#include "sys/fsystem/fsyspriv.h"

#define lineheight 12
#define font (UNIFONT const *)Font_10A
#define black RGB_TO_RGB16(0,0,0)
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

    if(line==-1) {
        lcd_poweron();
        lcd_setmode(BPPMODE_16BPP565, (unsigned int *)MEM_PHYS_SCREEN);
        lcd_on();

        cgl_initscr(&surface);
        cgl_rect(&surface, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, RGB_TO_RGB16(255,0,0));
        line =0;
    }

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

int esc_pressed() {
    *GPGCON = 0;    // SET ALL KEYBOARD COLUMNS AS INPUTS
    *GPGUDP = 0x5555;       // ENABLE PULLDOWN ON ALL INPUT LINES
    *GPDCON = (*GPDCON & 0xffff0000) | 0X5555;   // ALL ROWS TO OUTPUT
    *GPDUDP = (*GPDUDP &0xffff0000) | 0x5555;   // PULL DOWN ENABLE ON ALL OUTPUTS (TEMPORARILY SET TO INPUTS DURING SCAN)
    *GPDDAT &= 0xffff0000;    // ALL ROWS LOW
    *GPDDAT |= (1 << 6);      // JUST 1 ROW ENABLED

    // READ MANY TIMES TO ALLOW SIGNALS TO STABILIZE
    int k;
    for(k=0;k<500;++k) {
        if(*GPGDAT & (1 << 4)) return 1;
    }
    return 0;
}

// Initialize global variables region to zero
// Any globals that need to have a value must be initialized at run time or be declared read-only.
extern const int data_start;
extern const int data_size;
void clear_globals()
{
    int size=(unsigned int) (&data_size);
    unsigned int *data= (unsigned int *) (&data_start);

    while(size>0) { *data++=0; size-=4; }
}

ARM_MODE  __attribute__((naked)) void start_os(int entrypoint)
{
    // No prerequisites yet
    // r0 is entrypoint according to calling convention
    asm volatile ("blx r0");
}

// Returns entrypoint of os
int load_os(char *name)
{
    BINT err;

    err = FSSetCurrentVolume(0);
    if (err != FS_OK) {
        printline("Could not set volume", (char *)FSGetErrorMsg(err));
        while(1);
    }

    FSMarkVolumeReadOnly();

    FS_FILE *fileptr;

    err = FSOpen(name, FSMODE_READ | FSMODE_NOCREATE, &fileptr);
    if (err != FS_OK) {
        printline("Could not open file", (char *)FSGetErrorMsg(err));
        while(1);
    }

    struct Preamble preamble;

    err = FSRead((unsigned char *)&preamble, 32, fileptr);
    if (err != 32) {
        printline("Could not read preamble", 0);
        while(1);
    }

    err = FSSeek(fileptr, 0, SEEK_SET);
    if (err != FS_OK) {
        printline("Could not rewind", (char *)FSGetErrorMsg(err));
        while(1);
    }

    err = FSRead((unsigned char *)preamble.load_addr, preamble.load_size, fileptr);
    if (err != preamble.copy_size) {
        printline("Could not read data", 0);
        while(1);
    }

    err = FSClose(fileptr);
    if (err != FS_OK) {
        printline("Could not close file", (char *)FSGetErrorMsg(err));
        while(1);
    }

    err = FSShutdown();
    if (err != FS_OK) {
        printline("Could not unmount filesystem", (char *)FSGetErrorMsg(err));
        while(1);
    }

    return preamble.entrypoint;
}

ARM_MODE __attribute__((noreturn)) void main()
{
    initContext(32);
    Context.alloc_bmp = EMPTY_STORAGEBMP;
    init_simpalloc();

    // Initialize screen earlier so we can print error messages
    line = -1;

    int result = NANDInit();
    if (result != NAND_STATUS_OK) {
        char buffer[9];
        tohex(result, buffer);
        printline("NANDInit status", buffer);
        while(1);
    }

    FSHardReset();

    int isnewrpl=((*INFORM3)==NEWRPL_MAGIC)? 1:0;
    int entrypoint;

    // Switch operating systems if ESC is pressed
    if (esc_pressed()) {
        if(isnewrpl) {
            *INFORM3=0;
            isnewrpl=0;
        }
        else {
            *INFORM3=NEWRPL_MAGIC;
            isnewrpl=1;
        }
    }

    entrypoint = load_os( isnewrpl? "NEWRPL.ROM" : "PRIME_OS.ROM");
    cpu_flushwritebuffers();   // Ensure exception handlers are written to actual RAM
    cpu_flushicache();         // Ensure any old code is removed from caches, force the new exception handlers to be re-read from RAM
    start_os(entrypoint);

    while(1);
}

ARM_MODE __attribute__((noreturn)) void startup(int prevstate)
{
    clear_globals();

    cpu_flushTLB();            // We did not change the MMU, but doesn't hurt to flush it
    cpu_flushwritebuffers();   // Ensure exception handlers are written to actual RAM
    cpu_flushicache();         // Ensure any old code is removed from caches, force the new exception handlers to be re-read from RAM

    main(); // never returns
}

//************************************************************************************
//****** THESE ARE STUBS FROM NEWRPL, REMOVE AS SOON AS THEY ARE IMPLEMENTED *********

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

// ENTER POWER OFF MODE
void halEnterPowerOff()
{
    while(1);
}

// NEVER EXIT, THIS IS NOT AN APP, IT'S A FIRMWARE
int halExitOuterLoop()
{
    return 0;
}

char SCRATCH_MEMORY SERIAL_NUMBER_ADDRESS[11];

//********* END OF STUBS *************************************************************
//************************************************************************************
//************************************************************************************
