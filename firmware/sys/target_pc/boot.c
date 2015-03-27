#include <newrpl.h>
#include <ui.h>

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
    halRedrawAll(&scr);

    if(wascleared) DrawText(STATUSAREA_X,halScreen.Stack+halScreen.CmdLine+halScreen.Form+halScreen.Menu1,"Memory Cleared",(FONTDATA *)System7Font,0xf,&scr);
    else DrawText(STATUSAREA_X,halScreen.Stack+halScreen.CmdLine+halScreen.Form+halScreen.Menu1,"Memory Recovered",(FONTDATA *)System7Font,0xf,&scr);
    halStatusAreaPopup();

    halOuterLoop();

    tmr_eventkill(event);
    //   CLEAR SCREEN
    ggl_rect(&scr,0,0,131,80,0x12345678);

}


void clear_globals()
{
    // TODO: DO THIS MANUALLY ON THE PC



}


void startup()
{

    // BOOTLOADER LEAVES STACK ON MAIN RAM, MOVE TO SRAM
    // ALSO WE ENTER IN SUPERVISOR MODE

    setup_hardware();       // SETUP ACCESS TO OUT-OF-CHIP RAM MEMORY AMONG OTHER THINGS, THIS IS DONE BY THE BOOTLOADER BUT JUST TO BE SURE

    clear_globals();    // CLEAR TO ZERO ALL NON-PERSISTENT GLOBALS

    __exception_install();  // INITIALIZE IRQ AND EXCEPTION HANDLING

    tmr_setup();
    __keyb_init();

    __lcd_contrast=7;

    // ADD MORE HARDWARE INITIALIZATION HERE

    // ...

    // DONE WITH SYSTEM INITIALIZATION, SWITCH BACK TO USER MODE

    lcd_setmode(2,(int *)MEM_PHYS_SCREEN);


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

