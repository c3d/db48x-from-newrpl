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

        ggl_initscr(&scr);

        //   CLEAR SCREEN
        ggl_rect(&scr, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1,
                ggl_mksolid(PAL_STK_BG));

        // CAREFUL: THESE TWO ERASE THE WHOLE RAM, SHOULD ONLY BE CALLED AFTER TTRM
        mode = (halCheckMemoryMap() == 2) ? 1 : 0;      // mode!=0 ONLY WHEN WAKING UP FROM POWEROFF

        if(!mode) {
            // CHECK FOR MAGIC KEY COMBINATION
            if(keyb_isAnyKeyPressed()) {
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
        ggl_rect(&scr, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 1, ggl_mkcolor(PAL_GRAY4));

        keyb_flushnowait();

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

    // CLEAR THE REQUEST TO TERMINATE THE THREAD
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

    lcd_setmode( (DEFAULTBITMAPMODE==BITMAP_RAW16G)? 2:3, (unsigned int *)MEM_PHYS_SCREEN);

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
    // TODO: NOT IDEAL, BUT INDICATE WE WANT TO EXIT THE APPLICATION

    pc_terminate = 2;
}

int halExitOuterLoop()
{
    return pc_terminate;
}
