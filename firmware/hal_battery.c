/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */


// POWER AND BATTERY MANAGEMENT
#include <newrpl.h>
#include "libraries.h"
#include <ui.h>

void battery_handler()
{

    bat_read();
    //halSetNotification(N_CONNECTION,0xf^halGetNotification(N_CONNECTION));

    /*
    gglsurface scr;
    ggl_initscr(&scr);

    // THIS IS FOR DEBUG ONLY
    int k;
    k=395*__battery+7355;  // EMPIRICAL RELATIONSHIP OF VOLTAGE TO ADC VALUE

    int text;

    text=k>>16;
    text+='0';
    text|='.'<<8;

    if((k&0xffff)<3277) text|='0'<<16;
            else         if((k&0xffff)<9830) text|='1'<<16;
            else         if((k&0xffff)<16384) text|='2'<<16;
            else         if((k&0xffff)<22938) text|='3'<<16;
            else         if((k&0xffff)<29491) text|='4'<<16;
            else         if((k&0xffff)<36045) text|='5'<<16;
            else         if((k&0xffff)<42598) text|='6'<<16;
            else         if((k&0xffff)<49152) text|='7'<<16;
            else         if((k&0xffff)<55706) text|='8'<<16;
            else         if((k&0xffff)<62259) text|='9'<<16;
            else         { text|='0'<<16; ++text; }

    DrawTextBk(STATUSAREA_X,SCREEN_HEIGHT-14,(char *)&text,(UNIFONT *)MiniFont,0xf,0,&scr);

    k=(__battery>>8)&0xf;
    if(k>9) k+='A'-10;
    else k+='0';

    DrawTextBk(STATUSAREA_X,SCREEN_HEIGHT-7,(char *)&k,(UNIFONT *)MiniFont,0xf,0,&scr);

    k=(__battery>>4)&0xf;
    if(k>9) k+='A'-10;
    else k+='0';
    DrawTextBk(STATUSAREA_X+4,SCREEN_HEIGHT-7,(char *)&k,(UNIFONT *)MiniFont,0xf,0,&scr);

    k=(__battery)&0xf;
    if(k>9) k+='A'-10;
    else k+='0';
    // CAREFUL, INTEGER USED AS STRING IS ONLY VALID IN LITTLE ENDIAN!
    DrawTextBk(STATUSAREA_X+8,SCREEN_HEIGHT-7,(char *)&k,(UNIFONT *)MiniFont,0xf,0,&scr);
    */


    // THIS IS THE REAL HANDLER
    if(__battery<0x300) {
        // SHOW CRITICAL BATTERY SIGNAL
        if(halFlags&HAL_FASTMODE) {
            // LOW VOLTAGE WHEN RUNNING FAST
            halSetNotification(N_LOWBATTERY,0xf^halGetNotification(N_LOWBATTERY));
            halFlags|=HAL_SLOWLOCK;
        }
        else {
            // KEEP BLINKING INDICATOR
        halSetNotification(N_LOWBATTERY,0xf^halGetNotification(N_LOWBATTERY));
        // AND DISALLOW FAST MODE
        halFlags|=HAL_SLOWLOCK;
        }
        return;
    }

    if(__battery<0x320) {
        // SHOW STATIC LOW BATTERY SIGNAL
        if(halFlags&HAL_FASTMODE) {
            // LOW VOLTAGE WHEN RUNNING FAST IS OK
            return;
        }
        else {
        // SET PERMANENT BATTERY ICON
        // AND DISALLOW FAST MODE
        halSetNotification(N_LOWBATTERY,0xf);
        halFlags|=HAL_SLOWLOCK;
        }
        return;
    }

    if(__battery==0x400) {
        // WE ARE ON USB POWER
        halSetNotification(N_LOWBATTERY,0x8);
        halFlags&=~HAL_SLOWLOCK;
        return;
    }


    if(__battery>=0x320) {
            // REMOVE BATTERY INDICATOR AND ALLOW FAST MODE
            halSetNotification(N_LOWBATTERY,0);
            halFlags&=~HAL_SLOWLOCK;
    }


}




void busy_handler()
{
    // THE CPU IS BUSY, SWITCH TO FAST SPEED!!
    // PREVENT HIGH SPEED UNDER LOW BATTERY CONDITION
    halSetNotification(N_HOURGLASS,0xf);
    halFlags|=HAL_HOURGLASS;
    if(halFlags&HAL_NOCLOCKCHANGE) {
        tmr_events[halBusyEvent].status|=2; // SET AUTORELOAD FUNCTION
        // AND DO NOTHING
        return;
    }
    halBusyEvent=-1;
    if(halFlags&HAL_SLOWLOCK) return;
    halCPUFastMode();
    halFlags|=HAL_FASTMODE;
}

void halInitBusyHandler()
{
    halCPUSlowMode();
    halFlags=(halFlags&~HAL_AUTOOFFTIME)|SET_AUTOOFFTIME(DEFAULT_AUTOOFFTIME);        // DEFAULT TO 2 MINUTES
    halBusyEvent=tmr_eventcreate(&busy_handler,(halFlags&HAL_QUICKRESPONSE)? 30:500,0);
}

void halSetBusyHandler()
{
    if(!(halFlags&HAL_FASTMODE)) {
    // START THE EVENT AGAIN
    if(halBusyEvent<=0) halBusyEvent=tmr_eventcreate(&busy_handler,(halFlags&HAL_QUICKRESPONSE)? 30:500,0);
    }
}

// RETURN THE SYSTEM CLOCK TICKS
BINT64 halTicks()
{
    return (BINT64)tmr_ticks2us(0,tmr_ticks());
}



// DO ANY PREPARATIONS BEFORE ENTERING POWER OFF STATE
void halPreparePowerOff()
{

    // SAVE THE COMMAND LINE STATE

    WORDPTR saved;
    if(halScreen.CmdLineState&CMDSTATE_OPEN) {
        saved=halSaveCmdLine();
        if(!saved) saved=(WORDPTR)empty_list;
    } else saved=(WORDPTR)empty_list;

    rplStoreSettings((WORDPTR)savedcmdline_ident,saved);

    // TODO: ADD OTHER POWEROF PROCEDURES

    saved=rplNewBINT(halFlags,DECBINT);
    if(!saved) saved=(WORDPTR)zero_bint;
    rplStoreSettings((WORDPTR)savedflags_ident,saved);

}

// DO ANY PREPARATIONS BEFORE WAKEUP FROM POWEROFF
void halWakeUp()
{
WORDPTR saved;

// RESTORE THE FLAGS

saved=rplGetSettings((WORDPTR)savedflags_ident);
if(saved) {
    BINT tmpflags=rplReadBINT(saved);
    BINT flagmask=(HAL_FASTMODE|HAL_HOURGLASS|HAL_SLOWLOCK|HAL_SKIPNEXTALARM); // SOME FLAGS SHOULD NOT BE PRESERVED
    halFlags=(tmpflags&(~flagmask)) | (halFlags&flagmask);
}
rplPurgeSettings((WORDPTR)savedflags_ident);

// AFTER PURGE SETTINGS WE MUST UPDATE THE FONT ARRAYS
halUpdateFonts();

// RESTORE THE MENU2 HIDDEN STATUS

if(rplTestSystemFlag(FL_HIDEMENU2)) halSetMenu2Height(0);
else halSetMenu2Height(MENU2_HEIGHT);

if(rplTestSystemFlag(FL_QUICKRESPONSE)) halFlags|=HAL_QUICKRESPONSE;
else halFlags&=~HAL_QUICKRESPONSE;
// RESTORE STACK
saved=rplGetSettings((WORDPTR)stksave_ident);

if(saved) {
    if(ISAUTOEXPLIST(*saved)) {
        BINT nitems=rplListLength(saved);
        rplExpandStack(nitems);
        if(!Exceptions) {
        WORDPTR ptr=saved+1;
        while(nitems--) { rplPushDataNoGrow(ptr); ptr=rplSkipOb(ptr); }
        }
    }
    else rplPushData(saved);

}
rplPurgeSettings((WORDPTR)stksave_ident);

// AFTER PURGE SETTINGS WE MUST UPDATE THE FONT ARRAYS
halUpdateFonts();

// RESTORE THE COMMAND LINE
saved=rplGetSettings((WORDPTR)savedcmdline_ident);
if(saved) {
    if(halRestoreCmdLine(saved))  halSetContext(halGetContext()|CONTEXT_INEDITOR);
    rplPurgeSettings((WORDPTR)savedcmdline_ident);
}

// AFTER PURGE SETTINGS WE MUST UPDATE THE FONT ARRAYS
halUpdateFonts();


// FLUSH THE ON-KEY KEYPRESS FROM THE KEYBOARD BUFFER BEFORE ENTERING THE OUTER LOOP
// THIS CAN CANCEL AN EXISTING COMMAND LINE
keyb_flushnowait();
keyb_setshiftplane(0,0,0,0);

if (rplCheckAlarms())
    halSetNotification(N_ALARM, 0xf);
else
    halSetNotification(N_ALARM, 0x0);

// TODO: ADD OTHER WAKEUP PROCEDURES

halScreen.DirtyFlag|=STACK_DIRTY|FORM_DIRTY|CMDLINE_ALLDIRTY|MENU2_DIRTY|STAREA_DIRTY;  // UPDATE EVERYTHING

}
