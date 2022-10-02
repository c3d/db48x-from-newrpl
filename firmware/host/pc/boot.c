/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <ui.h>
#include <stdlib.h>
#include "sys/fsystem/fsyspriv.h"

int pc_terminate;
extern int memmap_intact;

void setup_hardware()
{
    // SETUP HARDWARE

    // DUMMY FUNCTION ON PC-TARGET, NO HARDWARE TO SETUP
}

// REAL PROGRAM STARTUP (main)
// WITH VIRTUAL MEMORY ALREADY INITIALIZED
void main_virtual()
{

    // INITIALIZE SOME SYSTEM VARIABLES

    gglsurface scr;
    int wascleared = 0, mode;

    do {

        bat_setup();

        // MONITOR BATTERY VOLTAGE TWICE PER SECOND
        HEVENT event = tmr_eventcreate(battery_handler, 500, 1);

        ggl_init_screen(&scr);

        //   CLEAR SCREEN
        ggl_rect(&scr, 0, 0, LCD_W - 1, LCD_H - 1, PAL_STK_BG);

        // CAREFUL: THESE TWO ERASE THE WHOLE RAM, SHOULD ONLY BE CALLED AFTER TTRM
        mode = (halCheckMemoryMap() == 2) ? 1 : 0;      // mode!=0 ONLY WHEN WAKING UP FROM POWEROFF

        if(!mode) {
            // CHECK FOR MAGIC KEY COMBINATION
            if(keyb_is_any_key_pressed()) {
                throw_exception("Wipeout requested",
                        EX_WARM | EX_WIPEOUT | EX_EXIT);
            }

            // CAREFUL: THESE TWO ERASE THE WHOLE RAM, SHOULD ONLY BE CALLED AFTER TTRM
            if(!halCheckMemoryMap()) {
                // WIPEOUT MEMORY
                halInitMemoryMap();
                rplInitMemoryAllocator();
                rplInit();
                wascleared = 1;
            }
            else {
                if(!halCheckRplMemory()) {
                    // WIPEOUT MEMORY
                    halInitMemoryMap();
                    rplInitMemoryAllocator();
                    rplInit();
                    wascleared = 1;
                }
                else {
                    rplInitMemoryAllocator();
                    rplWarmInit();
                }
            }

        }
        else {
            rplInitMemoryAllocator();
            rplHotInit();
        }

        // INITIALIZE SD CARD SYSTEM MEMORY ALLOCATOR
        FSHardReset();

        halInitScreen();
        halInitKeyboard();
        halInitBusyHandler();
        halRedrawAll(&scr);

        if(!mode) {
            if(wascleared)
                halShowMsg("Memory Cleared");
            else {
                halShowMsg("Memory Recovered");
            }
        }
        else {
            // RESTORE OTHER SYSTEM STATUS FROM POWER OFF
            halWakeUp();
        }

        halOuterLoop(0, 0, 0, 0);

        // RETURNED MEANS WE MUST SHUT DOWN

        tmr_eventkill(event);
        // KILL ALL OTHER PENDING EVENTS TOO
        int k;
        for(k = 0; k < 5; ++k)
            tmr_eventkill(k);

        //   CLEAR SCREEN
        ggl_rect(&scr, 0, 0, LCD_W, LCD_H - 1, PAL_GRAY4);

        keyb_flush_no_wait();

        if(halFlags & HAL_RESET) {
            rplWarmInit();
            memmap_intact = 2;
        }

    }
    while(halFlags & HAL_RESET);

}

void clear_globals()
{
    // TODO: DO THIS MANUALLY ON THE PC

}

void startup()
{

    // BOOTLOADER LEAVES STACK ON MAIN RAM, MOVE TO SRAM
    // ALSO WE ENTER IN SUPERVISOR MODE

    // Clear the request to terminate the thread
    pc_terminate = 0;

    setup_hardware();   // SETUP ACCESS TO OUT-OF-CHIP RAM MEMORY AMONG OTHER THINGS, THIS IS DONE BY THE BOOTLOADER BUT JUST TO BE SURE

    clear_globals();    // CLEAR TO ZERO ALL NON-PERSISTENT GLOBALS

    exception_install();      // INITIALIZE IRQ AND EXCEPTION HANDLING

    tmr_setup();
    keyb_irq_init();

    lcd_contrast = 7;

    // ADD MORE HARDWARE INITIALIZATION HERE

    // ...

    // DONE WITH SYSTEM INITIALIZATION, SWITCH BACK TO USER MODE

    // START IN FULL COLOR MODE BY DEFAULT

    lcd_setmode( (DEFAULT_BITMAP_MODE==BITMAP_RAW16G)? 2:DEFAULT_BITMAP_MODE, (unsigned int *)MEM_PHYS_SCREEN);

    main_virtual();

    // END OF THE THREAD
}

// THIS FUNCTION REBOOTS THE RPL CORE COMPLETELY
// ALL ELEMENTS IN THE STACK WILL BE LOST
// MUST BE ENTERED IN SUPERVISOR MODE
void halWarmStart()
{
    // TODO: ADD RPL ENGINE CLEANUP HERE BEFORE RESET

    // DUMMY FUNCTION ON PC-TARGET: WARMSTART NOT ALLOWED
    // OR IT CREATES RECURSIVITY, FILLING UP THE STACK

    exit(0);

}

void halWipeoutWarmStart()
{

    // DUMMY ON PC-TARGET: WIPEOUT NOT ALLOWED FROM WITHIN THREAD
    halWarmStart();
}

void halReset()
{
    // TODO: ADD RPL ENGINE CLEANUP HERE BEFORE RESET

    // DUMMY FUNCTION ON PC-TARGET, RESET NOT ALLOWED
    halFlags |= HAL_RESET;
    memmap_intact = 2;

}

void halEnterPowerOff()
{
    // Indicate we want to exit the application
    pc_terminate = 2;
}

int halExitOuterLoop()
{
    return pc_terminate;
}
