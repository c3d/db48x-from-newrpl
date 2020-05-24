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
#include "../fsystem/fsyspriv.h"

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




// Initialize global variables region to zero
// Any globals that need to have a value must be initialized at run time or be declared read-only.
extern const int __data_start;
extern const int __data_size;
void clear_globals()
{
int size=(unsigned int) (&__data_size);
unsigned int *data= (unsigned int *) (&__data_start);

while(size>0) { *data++=0; size-=4; }

}



__ARM_MODE__ void startup(int) __attribute__((noreturn));
void startup(int prevstate)
{

    clear_globals();
    lcd_setmode(BPPMODE_4BPP, (unsigned int *)MEM_PHYS_SCREEN);
    lcd_on();

    ggl_initscr(&surface);
    ggl_rect(&surface, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, 0);



    line=0;
    printline("Multiboot stated",NULL);

    initContext(32);
    Context.alloc_bmp = EMPTY_STORAGEBMP;
    init_simpalloc();
    printline("Memory allocation started",NULL);

    FSHardReset();

    printline("File system started",NULL);


    // Playing it save for testing
    NANDWriteProtect();

    printline("Write protect",NULL);

//    __exception_install();

//    printline("Exceptions & IRQ vectors installed",NULL);

//    tmr_setup();
/*
    {
        char buffer[9];
        buffer[8]=0;
        tohex(tmr_getsysfreq(),buffer);
        printline("Timer init=",buffer);
    }
*/
//    __keyb_init();

//    printline("Keyboard driver initialized",NULL);


    BINT err;

    err = FSSetCurrentVolume(0);
    if (err != FS_OK) {
        printline("Could not set volume", (char *)FSGetErrorMsg(err));
    }

    struct __file *fileptr;


    printline("Loading ",(n_pressed()) ? "NEWRPL.ROM" : "PRIME_OS.ROM");

    err = FSOpen(
            (n_pressed()) ? "NEWRPL.ROM" : "PRIME_OS.ROM",
            FSMODE_READ | FSMODE_NOCREATE, &fileptr);
    if (err != FS_OK) {
        printline("Could not open file", (char *)FSGetErrorMsg(err));
    } else {
        char buffer[9];
        buffer[8]=0;
        printline("File open OK",fileptr->Name);
        tohex(fileptr->FileSize,buffer);
        printline("File Size=",buffer);
    }



    struct Preamble preamble;
    err = FSRead((unsigned char *)&preamble, 32, fileptr);
    if (err != 32) {
        printline("Could not read preamble", 0);
    }  else {
        struct Preamble *pr=(struct Preamble *) (fileptr->RdBuffer.Data);
        char buffer[9];
        buffer[8]=0;
        tohex(preamble.load_addr,buffer);
        printline("Load addr=",buffer);
        tohex(pr->load_addr,buffer);
        printline("Load addr=",buffer);
        tohex(preamble.load_size,buffer);
        printline("Load size=",buffer);
        tohex(pr->load_size,buffer);
        printline("Load size=",buffer);
        tohex(preamble.entrypoint,buffer);
        printline("Entry addr=",buffer);
        tohex(pr->entrypoint,buffer);
        printline("Entry addr=",buffer);
    }

    err = FSSeek(fileptr, 0, SEEK_SET);
    if (err != FS_OK) {
        printline("Could not rewind", (char *)FSGetErrorMsg(err));
    }

    err = FSRead((unsigned char *)preamble.load_addr, preamble.load_size, fileptr);
    if (err != preamble.load_size) {
        printline("Could not read data", 0);
    } else printline("Finished reading file",NULL);

    err = FSClose(fileptr);
    if (err != FS_OK) {
        // Can't be closed due to missing write support
    }


    printline("Preparing to jump...",NULL);

    // call payload
    ((void (*)(void))preamble.entrypoint)();
    
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
