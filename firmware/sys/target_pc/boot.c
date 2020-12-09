/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <ui.h>
#include "../fsystem/fsyspriv.h"

int __pc_terminate;
extern int __memmap_intact;

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
                ggl_mkcolor(4));

        // CAREFUL: THESE TWO ERASE THE WHOLE RAM, SHOULD ONLY BE CALLED AFTER TTRM
        mode = (halCheckMemoryMap() == 2) ? 1 : 0;      // mode!=0 ONLY WHEN WAKING UP FROM POWEROFF

        if(!mode) {
            // CHECK FOR MAGIC KEY COMBINATION
            if(keyb_isAnyKeyPressed()) {
                throw_exception("Wipeout requested",
                        __EX_WARM | __EX_WIPEOUT | __EX_EXIT);
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
        ggl_rect(&scr, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 1, 0x11111111);

        keyb_flushnowait();

        if(halFlags & HAL_RESET) {
            rplWarmInit();
            __memmap_intact = 2;
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
    __pc_terminate = 0;

    setup_hardware();   // SETUP ACCESS TO OUT-OF-CHIP RAM MEMORY AMONG OTHER THINGS, THIS IS DONE BY THE BOOTLOADER BUT JUST TO BE SURE

    clear_globals();    // CLEAR TO ZERO ALL NON-PERSISTENT GLOBALS

    __exception_install();      // INITIALIZE IRQ AND EXCEPTION HANDLING

    tmr_setup();
    __keyb_init();

    __lcd_contrast = 7;

    // ADD MORE HARDWARE INITIALIZATION HERE

    // ...

    // DONE WITH SYSTEM INITIALIZATION, SWITCH BACK TO USER MODE

    lcd_setmode(2, (unsigned int *)MEM_PHYS_SCREEN);

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
    __memmap_intact = 2;

}

void halEnterPowerOff()
{
    // TODO: NOT IDEAL, BUT INDICATE WE WANT TO EXIT THE APPLICATION

    __pc_terminate = 2;
}

int halExitOuterLoop()
{
    return __pc_terminate;
}
