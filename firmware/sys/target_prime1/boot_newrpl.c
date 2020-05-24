/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <hal_api.h>
#include <ggl.h>


#define lineheight 12
#define font (UNIFONT const *)Font_10A
#define black 0xffffffff
#define left 0
#define right 160

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

void printline(gglsurface *surface,int line,char *left_text, char *right_text) {
    if (left_text) {
        DrawText(left, line * lineheight, left_text, font, black, surface);
    }
    if (right_text) {
        DrawText(right, line * lineheight, right_text, font, black, surface);
    }

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


// Initialize global variables region to zero
// Any globals that need to have a value must be initialized at run time or be declared read-only.
extern const int __data_start;
extern const int __data_size;
void clear_globals()
{
int size=(unsigned int) (&__data_size);
unsigned int *data= (unsigned int *) (&__data_start);

while(size>0) { *data++=0; --size; }

}

__ARM_MODE__ void startup(int) __attribute__((noreturn));
void startup(int prevstate)
{
    clear_globals();

    lcd_setmode(BPPMODE_4BPP, (unsigned int *)MEM_PHYS_SCREEN);
    lcd_on();

    gglsurface scr;
    
    ggl_initscr(&scr);
    ggl_rect(&scr, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, 0);

    char buffer[9];
    buffer[8]=0;
    tohex((unsigned int) (&__data_size),buffer);
    printline(&scr, 1, "Data size=", buffer);

    tohex((unsigned int) (&__data_start),buffer);
    printline(&scr, 2, "Data start=", buffer);

    printline(&scr, 10, "Attempt to install handlers", 0);

    __exception_install();      // INITIALIZE IRQ AND EXCEPTION HANDLING

    printline(&scr, 11, "Attempt to enable interrupts", 0);

    enable_interrupts();

    printline(&scr, 12, "Attempt to enable timers", 0);

    tmr_setup();

    printline(&scr, 13, "Attempt to enable keyboard", 0);

    __keyb_init();

    printline(&scr, 14, "Press any key", 0);

    unsigned int msg;
    do {
    msg=keyb_getmsg();
    tohex(msg,buffer);
    buffer[8]=0;
    if(msg) printline(&scr, 15, "MSG=", buffer);
    } while (1);




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
