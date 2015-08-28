/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


#include <newrpl.h>
#include <ui.h>


extern int __pc_terminate;


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
    int wascleared=0;
    bat_setup();

    // MONITOR BATTERY VOLTAGE TWICE PER SECOND
    HEVENT event=tmr_eventcreate(battery_handler,500,1);

    ggl_initscr(&scr);

    //   CLEAR SCREEN
    ggl_rect(&scr,0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1,ggl_mkcolor(4));

    // CAREFUL: THESE TWO ERASE THE WHOLE RAM, SHOULD ONLY BE CALLED AFTER TTRM
    if(!halCheckMemoryMap()) {
        // WIPEOUT MEMORY
    halInitMemoryMap();
    rplInit();
    wascleared=1;
    }
    else rplWarmInit();

    halInitScreen();
    halInitKeyboard();
    halInitBusyHandler();
    halRedrawAll(&scr);

    if(wascleared) halShowMsg("Memory Cleared");
    else halShowMsg("Memory Recovered");
    halStatusAreaPopup();

    halOuterLoop();

    tmr_eventkill(event);
    //   CLEAR SCREEN
    ggl_rect(&scr,0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1,0x12345678);

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
    __pc_terminate=0;


    setup_hardware();       // SETUP ACCESS TO OUT-OF-CHIP RAM MEMORY AMONG OTHER THINGS, THIS IS DONE BY THE BOOTLOADER BUT JUST TO BE SURE

    clear_globals();    // CLEAR TO ZERO ALL NON-PERSISTENT GLOBALS

    __exception_install();  // INITIALIZE IRQ AND EXCEPTION HANDLING

    tmr_setup();
    __keyb_init();

    __lcd_contrast=7;

    // ADD MORE HARDWARE INITIALIZATION HERE

    // ...

    // DONE WITH SYSTEM INITIALIZATION, SWITCH BACK TO USER MODE

    lcd_setmode(2,(unsigned int *)MEM_PHYS_SCREEN);


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
    exit(0);

}

void halEnterPowerOff()
{
    // TODO: SAVE STATE BEFORE CLOSING

    exit(0);
}

int halExitOuterLoop()
{
    return __pc_terminate;
}
