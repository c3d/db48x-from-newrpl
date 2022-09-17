/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

// POWER AND BATTERY MANAGEMENT
#include <libraries.h>
#include <ui.h>
#include <recorder.h>

#define ADC_100_LIMIT   0x370
#define ADC_0_LIMIT     0x300
#define ADC_PLUGGED     0x400

#ifndef TARGET_PRIME1
// REVISIT: The values for the Prime case look better
#define ADC_LOWBAT     0x320
#define ADC_CRITICAL   0x300
#else
#define ADC_LOWBAT          (ADC_0_LIMIT+(ADC_100_LIMIT-ADC_0_LIMIT)/10)
#define ADC_CRITICAL        (ADC_0_LIMIT+(ADC_100_LIMIT-ADC_0_LIMIT)/20)
#endif

extern int bat_readcnt;

RECORDER_TWEAK_DEFINE(battery_debug, 0, "Activate battery debug code");

void battery_handler()
{

    bat_read();
    //halSetNotification(N_CONNECTION,0xf^halGetNotification(N_CONNECTION));

    if (RECORDER_TWEAK(battery_debug))
    {
#ifndef TARGET_PRIME1
        const UNIFONT * font = (const UNIFONT *) Font_5A;
        enum { W = 4, H = 7 };
#else //
        const UNIFONT * font = (const UNIFONT *) Font_8A;
        enum { W = 6, H = 9 };
#endif // TARGET_PRIME1

        gglsurface scr;
        ggl_initscr(&scr);

        // THIS IS FOR DEBUG ONLY
        char text[4] = { 0 };
        int k = 395*battery+7355;  // EMPIRICAL RELATIONSHIP OF VOLTAGE TO ADC VALUE
        int l = k & 0xffff;

        text[0] = '0' + (k>>16);
        text[1] = '.';
        text[2] =
              l <  3277 ? '0'
            : l <  9830 ? '1'
            : l < 16384 ? '2'
            : l < 22938 ? '3'
            : l < 29491 ? '4'
            : l < 36045 ? '5'
            : l < 42598 ? '6'
            : l < 49152 ? '7'
            : l < 55706 ? '8'
            : l < 62259 ? '9'
            : 'X';

        DrawTextBk(STATUS_AREA_X,LCD_H-2*H,text,font,ggl_mkcolor(PAL_STA_TEXT),ggl_mkcolor(PAL_STA_BG),&scr);

        for (unsigned s = 0; s < 3; s++)
        {
            k = (battery>>(8-4*s)) & 0xf;
            text[s] = k < 10 ? k + '0' : k + 'A' - 10;
        }
        DrawTextBk(STATUS_AREA_X,LCD_H-H,text,font,ggl_mkcolor(PAL_STA_TEXT),ggl_mkcolor(PAL_STA_BG),&scr);
    }

    // THIS IS THE REAL HANDLER
    if(battery < ADC_CRITICAL) {
        // SHOW CRITICAL BATTERY SIGNAL
        if(halFlags & HAL_FASTMODE) {
            // LOW VOLTAGE WHEN RUNNING FAST
            halSetNotification(N_LOWBATTERY,
                    0xf ^ halGetNotification(N_LOWBATTERY));
            halFlags |= HAL_SLOWLOCK;
            halScreenUpdated();
        }
        else {
            // KEEP BLINKING INDICATOR
            halSetNotification(N_LOWBATTERY,
                    0xf ^ halGetNotification(N_LOWBATTERY));
            // AND DISALLOW FAST MODE
            halFlags |= HAL_SLOWLOCK;
            halScreenUpdated();
        }
        return;
    }

    if(battery < ADC_LOWBAT) {
        // SHOW STATIC LOW BATTERY SIGNAL
        if(halFlags & HAL_FASTMODE) {
            // LOW VOLTAGE WHEN RUNNING FAST IS OK
            return;
        }
        else {
            // SET PERMANENT BATTERY ICON
            // AND DISALLOW FAST MODE
            if(!halGetNotification(N_LOWBATTERY))
                halScreenUpdated();

            halSetNotification(N_LOWBATTERY, 0xf);
            halFlags |= HAL_SLOWLOCK;
        }
        return;
    }

    if(battery == ADC_PLUGGED) {
        // WE ARE ON USB POWER
        if(!halGetNotification(N_LOWBATTERY))
            halScreenUpdated();

        halSetNotification(N_LOWBATTERY, 0x8);
        halFlags &= ~HAL_SLOWLOCK;
        return;
    }

    if(battery >= ADC_LOWBAT) {
        // REMOVE BATTERY INDICATOR AND ALLOW FAST MODE
        if(halGetNotification(N_LOWBATTERY))
            halScreenUpdated();
        halSetNotification(N_LOWBATTERY, 0);
        halFlags &= ~HAL_SLOWLOCK;
    }

#ifdef TARGET_PRIME1
    // Update notification icon
    // only once every 4 seconds
    // (4 interrupts)

    ++bat_readcnt;
    bat_readcnt&=7;

    if(!bat_readcnt) {


        if(halScreen.Menu2==0) return;  // Don't display battery in single menu mode

        gglsurface scr;
        ggl_initscr(&scr);

        int text,rot;
        int k;

        // EMPIRICAL PERCENTAGE SCALE:

        // ADC VALUE = 0x370 --> 100% = 65536
        // ADC_VALUE = 0X300 --> 0%   = 0
        // 65536 = A*0X370-B
        // 0 = A*0X300-B ---> A=B/0x300
        // B = 65536/(0x370-0x300/0x300)
        // B = 0x300 * 65536 / (0x370-0x300)

        k=(65536/(ADC_100_LIMIT-ADC_0_LIMIT))*battery- ((ADC_0_LIMIT<<16)/(ADC_100_LIMIT-ADC_0_LIMIT));  // EMPIRICAL RELATIONSHIP OF VOLTAGE TO ADC VALUE

        // STRICT BOUNDARIES SINCE VALUES ARE APPROXIMATED
        if(k>65535) k=65535;
        if(k<0) k=0;

        if(battery==0x400) {
            // Battery is charging - display charging icon
            DrawTextBk(LCD_W-StringWidth((char *)"C", Font_Notifications)-1, LCD_H-1-FONT_Notifications.BitmapHeight, (char *)"C",
                       Font_Notifications, ggl_mkcolor(PAL_STA_BAT), ggl_mkcolor(PAL_STA_BG), &scr);
        }
        else {
            // Display Battery percentage below battery icon
            // REVISIT: Assume little-endian, really hard to read
            text=0;
            rot=0;
            ++k;
            if(k>>16) { text='1'; k=0; rot+=8; }

            k=(k&0xffff)*10;

            text|=(((k>>16)+'0')&( (rot||(k>>16))? 0xff:0))<<rot;
            if(text) rot+=8;

            k=(k&0xffff)*10+32768;

            text|=((k>>16)+'0')<<rot;

            rot+=8;
            text|='%'<<rot;

            // Display battery percentage
            int percentwidth=StringWidthN((char *)&text,((char *)&text)+(rot>>3)+1,Font_10A);
            int batwidth=StringWidth((char *)"D",Font_Notifications);
            if(percentwidth>batwidth) {
                batwidth=(percentwidth+batwidth)/2;
            } else percentwidth=(percentwidth+batwidth)/2;

            DrawTextBk(LCD_W-percentwidth,LCD_H-FONT_10A.BitmapHeight-1,(char *)&text,Font_10A,ggl_mkcolor(PAL_STA_BAT), ggl_mkcolor(PAL_STA_BG),&scr);

            DrawTextBk(LCD_W-batwidth, LCD_H-2-FONT_10A.BitmapHeight-Font_Notifications->BitmapHeight, (char *)"D",
                       Font_Notifications, ggl_mkcolor(PAL_STA_BAT), ggl_mkcolor(PAL_STA_BG), &scr);
            halScreenUpdated();
        }
    }
#endif // TARGET_PRIME1
}

void busy_handler()
{
    // THE CPU IS BUSY, SWITCH TO FAST SPEED!!
    // PREVENT HIGH SPEED UNDER LOW BATTERY CONDITION
    halSetNotification(N_HOURGLASS, 0xf);

#ifdef TARGET_PRIME1
    // Force Display the Hourglass
    {
        gglsurface scr;
        ggl_initscr(&scr);
        DrawTextBk(LCD_W-StringWidth((char *)"W", Font_Notifications)-1, LCD_H-3-FONT_10A.BitmapHeight-2*FONT_Notifications.BitmapHeight, (char *)"W",
                   Font_Notifications, ggl_mkcolor(PAL_STA_BAT), ggl_mkcolor(PAL_STA_BG), &scr);

    }
#endif // TARGET_PRIME1

    halScreenUpdated();

    halFlags |= HAL_HOURGLASS;
    if(halFlags & HAL_NOCLOCKCHANGE) {
        tmr_events[halBusyEvent].status |= 2;   // SET AUTORELOAD FUNCTION
        // AND DO NOTHING
        return;
    }
    halBusyEvent = -1;
    if(halFlags & HAL_SLOWLOCK)
        return;
    halCPUFastMode();
    halFlags |= HAL_FASTMODE;
}

void halInitBusyHandler()
{
    halCPUSlowMode();
    halFlags = (halFlags & ~HAL_AUTOOFFTIME) | SET_AUTOOFFTIME(DEFAULT_AUTOOFFTIME);    // DEFAULT TO 2 MINUTES
    halBusyEvent =
            tmr_eventcreate(&busy_handler,
            (halFlags & HAL_QUICKRESPONSE) ? 30 : 500, 0);
}

void halSetBusyHandler()
{
    if(!(halFlags & HAL_FASTMODE)) {
        // START THE EVENT AGAIN
        if(halBusyEvent <= 0)
            halBusyEvent =
                    tmr_eventcreate(&busy_handler,
                    (halFlags & HAL_QUICKRESPONSE) ? 30 : 500, 0);
    }
}

// RETURN THE SYSTEM CLOCK TICKS
int64_t halTicks()
{
    return (int64_t) tmr_ticks2us(0, tmr_ticks());
}

// DO ANY PREPARATIONS BEFORE ENTERING POWER OFF STATE
void halPreparePowerOff()
{

    // SAVE THE COMMAND LINE STATE

    word_p saved;
    if(halScreen.CmdLineState & CMDSTATE_OPEN) {
        saved = halSaveCmdLine();
        if(!saved)
            saved = (word_p) empty_list;
    }
    else
        saved = (word_p) empty_list;

    rplStoreSettings((word_p) savedcmdline_ident, saved);

    // TODO: ADD OTHER POWEROF PROCEDURES

    saved = rplNewint32_t(halFlags, DECint32_t);
    if(!saved)
        saved = (word_p) zero_bint;
    rplStoreSettings((word_p) savedflags_ident, saved);

}

// DO ANY PREPARATIONS BEFORE WAKEUP FROM POWEROFF
void halWakeUp()
{
    word_p saved;

#ifdef TARGET_PRIME1

// RESTORE UI THEME

    saved = rplGetSettings((word_p) theme_ident);
    if(saved) {
        int error = 0;
        if(!ISLIST(*saved)) {
            error=1;
        }

        // Take a list of 64 integers and use them as palette entries

        if(!error && (rplListLength(saved)<PALETTE_SIZE)) error=1;

        int k;
        word_p obj=saved+1;
        WORD palette[PALETTE_SIZE];
        uint64_t color;

        if(!error) {
            for(k=0;k<PALETTE_SIZE;++k)
            {
                color=rplReadNumberAsInt64(obj);
                if(Exceptions) { rplClearErrors(); error=1; break; }
                palette[k]=(WORD)color;
                obj=rplSkipOb(obj);
            }
        }
        // Here we were able to read all numbers without any errors, so it's a valid palette

        if(!error) halSetupTheme(palette);
        else rplPurgeSettings((word_p)theme_ident);
    }

#endif /* TARGET_PRIME1 */

// RESTORE THE FLAGS

    saved = rplGetSettings((word_p) savedflags_ident);
    if(saved) {
        int32_t tmpflags = rplReadint32_t(saved);
        int32_t flagmask = (HAL_FASTMODE | HAL_HOURGLASS | HAL_SLOWLOCK | HAL_SKIPNEXTALARM);      // SOME FLAGS SHOULD NOT BE PRESERVED
        halFlags = (tmpflags & (~flagmask)) | (halFlags & flagmask);
    }
    rplPurgeSettings((word_p) savedflags_ident);

// AFTER PURGE SETTINGS WE MUST UPDATE THE FONT ARRAYS
    halUpdateFonts();

// RESTORE THE MENU2 HIDDEN STATUS

    if(rplTestSystemFlag(FL_HIDEMENU2))
        halSetMenu2Height(0);
    else
        halSetMenu2Height(MENU2_HEIGHT);

    if(rplTestSystemFlag(FL_QUICKRESPONSE))
        halFlags |= HAL_QUICKRESPONSE;
    else
        halFlags &= ~HAL_QUICKRESPONSE;
// RESTORE STACK
    saved = rplGetSettings((word_p) stksave_ident);

    if(saved) {
        if(ISAUTOEXPLIST(*saved)) {
            int32_t nitems = rplListLength(saved);
            rplExpandStack(nitems);
            if(!Exceptions) {
                word_p ptr = saved + 1;
                while(nitems--) {
                    rplPushDataNoGrow(ptr);
                    ptr = rplSkipOb(ptr);
                }
            }
        }
        else
            rplPushData(saved);

    }
    rplPurgeSettings((word_p) stksave_ident);

// AFTER PURGE SETTINGS WE MUST UPDATE THE FONT ARRAYS
    halUpdateFonts();

// RESTORE THE COMMAND LINE
    saved = rplGetSettings((word_p) savedcmdline_ident);
    if(saved) {
        if(halRestoreCmdLine(saved))
            halSetContext(halGetContext() | CONTEXT_INEDITOR);
        rplPurgeSettings((word_p) savedcmdline_ident);
    }

// AFTER PURGE SETTINGS WE MUST UPDATE THE FONT ARRAYS
    halUpdateFonts();

// FLUSH THE ON-KEY KEYPRESS FROM THE KEYBOARD BUFFER BEFORE ENTERING THE OUTER LOOP
// THIS CAN CANCEL AN EXISTING COMMAND LINE
    keyb_flushnowait();
    keyb_setshiftplane(0, 0, 0, 0);

    if(rplCheckAlarms())
        halSetNotification(N_ALARM, 0xf);
    else
        halSetNotification(N_ALARM, 0x0);

// TODO: ADD OTHER WAKEUP PROCEDURES

    halScreen.DirtyFlag |= STACK_DIRTY | FORM_DIRTY | CMDLINE_ALLDIRTY | MENU2_DIRTY | STAREA_DIRTY;    // UPDATE EVERYTHING

}
