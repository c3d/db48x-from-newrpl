// POWER AND BATTERY MANAGEMENT
#include <ui.h>

void battery_handler()
{

    bat_read();
    halSetNotification(N_CONNECTION,0xf^halGetNotification(N_CONNECTION));

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
    halBusyEvent=-1;
    if(halFlags&HAL_SLOWLOCK) return;
    cpu_setspeed(192000000);
    halFlags|=HAL_FASTMODE;
}


void halSetBusyHandler()
{
    if(!(halFlags&HAL_FASTMODE)) {
    // START THE EVENT AGAIN
    if(halBusyEvent<=0) halBusyEvent=tmr_eventcreate(&busy_handler,500,0);
    }
}


// RETURN THE SYSTEM CLOCK TICKS
BINT64 halTicks()
{
    return (BINT64)tmr_ticks2us(0,tmr_ticks());
}
