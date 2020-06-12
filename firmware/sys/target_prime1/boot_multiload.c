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


__ARM_MODE__ void enable_interrupts()
{
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");
}

__ARM_MODE__ void disable_interrupts()
{
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("orr r1,r1,#0xc0");
    asm volatile ("msr cpsr_all,r1");
}

__ARM_MODE__ void set_stack(unsigned int *) __attribute__((naked));
void set_stack(unsigned int *newstackptr)
{

    asm volatile ("mov sp,r0");
    asm volatile ("bx lr");

}
__ARM_MODE__ void switch_mode(int mode) __attribute__((naked));
void switch_mode(int mode)
{
    asm volatile ("and r0,r0,#0x1f");
    asm volatile ("mrs r1,cpsr_all");
    asm volatile ("bic r1,r1,#0x1f");
    asm volatile ("orr r1,r1,r0");
    asm volatile ("mov r0,lr"); // GET THE RETURN ADDRESS **BEFORE** MODE CHANGE
    asm volatile ("msr cpsr_all,r1");
    asm volatile ("bx r0");
}

// Move Stack for all modes to better locations
// Stage 2 bootloader leaves stack at:
// Above 0x31ffff00 it's the relocated exception handlers
// FIQ: 0x31ffff00
// IRQ: 0x31fffe00
// ABT: 0x31fffd00
// UND: 0x31fffc00
// SUP: 0x31fffb00
// and stays in supervisor mode

// New stack locations for all modes:
// FIQ: 0x31fffefc (4-byte buffer in case of stack underrun)
// IRQ: 0x31fff400  // Provide 1kbyte stack for IRQ handlers
// ABT: 0x31fff800  // Data Abort and Prefetch Undefined share the same handler, so stack is shared
// UND: 0x31fff800  // Data Abort and Prefetch Undefined share the same handler, so stack is shared
// SUP: 0x31fff800  // Superfisor mode is never used, share the same stack with other abort handlers
// SYS: 0x31ff7c00  // Stack below the MMU table with 1kbytes buffer in case of underrun
// Stay in SYS mode, with supervisor privileges but using no banked registers

__ARM_MODE__ void set_stackall() __attribute__((naked));
void set_stackall()
{
    register unsigned int lr_copy;
    // THE USER STACK IS ALREADY SETUP PROPERLY
    asm volatile ("mov %[res],lr" : [res] "=r" (lr_copy) : );

    switch_mode(SVC_MODE);

    set_stack((unsigned int *)0x31fff800);

    switch_mode(ABT_MODE);

    set_stack((unsigned int *)0x31fff800);

    switch_mode(UND_MODE);

    set_stack((unsigned int *)0x31fff800);

    switch_mode(FIQ_MODE);

    set_stack((unsigned int *)0x31fffefc);

    switch_mode(IRQ_MODE);

    set_stack((unsigned int *)0x31fff400);

    switch_mode(SYS_MODE);

    set_stack((unsigned int *)0x31ff7c00);  // Leave 1 kbytes buffer to make sure a bad stack does not overwrite the MMU tables

    asm volatile ("bx %[lr]" : : [lr] "r" (lr_copy));       // DO SOMETHING IN USER MODE TO PREVENT COMPILER FROM MAKING A TAIL CALL OPTIMIZATION
}

__ARM_MODE__ void finaljump(int address) __attribute__((noreturn));
void finaljump(int address)
{
    // This needs to be a register, not in the stack since the stack will change during the mode change
    register void (*funcptr)(void);

    funcptr=(void (*)(void))address;
    switch_mode(SVC_MODE);     // Firmware expects to be in supervisor mode. PRIME_OS will crash if not.

    cpu_flushwritebuffers();   // Ensure the file we just read is completely written to actual DRAM
    cpu_flushicache();         // Ensure any old code is removed from caches, force the new code we loaded to be read from DRAM

    funcptr();

    while(1);
}



__ARM_MODE__ void main() __attribute__((noreturn));
void main()
{
    // Playing it save for testing
    NANDWriteProtect();

    initContext(32);
    Context.alloc_bmp = EMPTY_STORAGEBMP;
    init_simpalloc();

    // Initialize screen earlier so we can print error messages
    line = 0;

    lcd_setmode(BPPMODE_4BPP, (unsigned int *)MEM_PHYS_SCREEN);
    lcd_on();

    ggl_initscr(&surface);
    ggl_rect(&surface, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, 0);


    char buffer[9];
    char buffer2[9];
    BINT err;



    if (NANDInit() == 0)
        printline("No BFX", NULL);
    buffer[8]=0;
    tohex((unsigned int)&line,buffer);
    printline("Line=",buffer);

    buffer[8]=0;
    tohex(__cpu_getFCLK(),buffer);
    printline("FCLK=",buffer);
    buffer[8]=0;
    tohex(__cpu_getHCLK(),buffer);
    printline("HCLK=",buffer);
    buffer[8]=0;
    tohex(__cpu_getPCLK(),buffer);
    printline("PCLK=",buffer);



    FSHardReset();



    err = FSSetCurrentVolume(0);
    if (err != FS_OK) {
        printline("Could not set volume", (char *)FSGetErrorMsg(err));
    }

    FS_FILE *fileptr;

    err = FSOpen(
            (n_pressed()) ?  "PRIME_OS.ROM" : "NEWRPL.ROM",
            FSMODE_READ | FSMODE_NOCREATE, &fileptr);
    if (err != FS_OK) {
        printline("Could not open file", (char *)FSGetErrorMsg(err));
    } else {
        printline("File open OK",fileptr->Name);
        tohex(fileptr->FileSize,buffer);
        printline("File Size=",buffer);
        tohex(fileptr->FirstCluster,buffer);
        printline("First Cluster=",buffer);
        tohex(fileptr->Chain.StartAddr<<9,buffer);
        printline("Flash Offset=",buffer);
    }

    struct Preamble preamble;
    err = FSRead((unsigned char *)&preamble, 32, fileptr);
    if (err != 32) {
        printline("Could not read preamble", 0);
    }  else {
        tohex(preamble.load_addr,buffer);
        printline("Load addr=",buffer);
        tohex(preamble.load_size,buffer);
        printline("Load size=",buffer);
        tohex(preamble.entrypoint,buffer);
        printline("Entry addr=",buffer);

        uint8_t otherbuffer[512];

    }

    err = FSSeek(fileptr, 0, SEEK_SET);
    if (err != FS_OK) {
        printline("Could not rewind", (char *)FSGetErrorMsg(err));
    }

    err = FSRead((unsigned char *)preamble.load_addr, preamble.load_size, fileptr);
    if (err != preamble.copy_size) {
        printline("Could not read data", 0);
    } else printline("Finished reading file",NULL);

    err = FSClose(fileptr);
    if (err != FS_OK) {
        // Can't be closed due to missing write support
    }

    // show debug information until we press and release N again
    while(n_pressed()) ;
    while(!n_pressed()) ;
    while(n_pressed()) ;

tohex(preamble.entrypoint,buffer);
printline("Jumping to Entry addr=",buffer);

// show debug information until we press and release N again
while(n_pressed()) ;
while(!n_pressed()) ;
while(n_pressed()) ;


    finaljump(preamble.entrypoint);

}

__ARM_MODE__ void startup(int) __attribute__((noreturn));
void startup(int prevstate)
{
    disable_interrupts();

    set_stackall();

    clear_globals();

// Do not install exception handlers in the boot loader to avoid conflict with firmware
//    __exception_install();

    cpu_flushTLB();            // We did not change the MMU, but doesn't hurt to flush it
    cpu_flushwritebuffers();   // Ensure exception handlers are written to actual RAM
    cpu_flushicache();         // Ensure any old code is removed from caches, force the new exception handlers to be re-read from RAM

    main(); // never returns
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
