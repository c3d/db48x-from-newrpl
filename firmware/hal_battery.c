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


RECORDER_TWEAK_DEFINE(battery_debug, 0, "Activate battery debug code");

void battery_handler()
{
    battery_read();

    // This is the real handler
    int low = battery_low();
    if (low)
    {
        if (low > 1)
        {
            // Low voltage, keep blinking indicator
            halSetNotification(N_LOWBATTERY,
                               0xf ^ halGetNotification(N_LOWBATTERY));
            halFlags |= HAL_SLOWLOCK;
            halScreenUpdated();
        }
        else
        {
            // Low voltage when running fast is OK
            if(halFlags & HAL_FASTMODE)
                return;

            // Set permanent battery icon and disallow fast mode
            halSetNotification(N_LOWBATTERY, 0xf);
            halFlags |= HAL_SLOWLOCK;
            return;
        }
    }

    if (battery_charging())
    {
        // We are on USB power
        halSetNotification(N_LOWBATTERY, 8);
        halFlags &= ~HAL_SLOWLOCK;
        return;
    }

    // Remove battery indicator and allow fast mode
    halSetNotification(N_LOWBATTERY, 0);
    halFlags &= ~HAL_SLOWLOCK;
}


void busy_handler()
{
    // THE CPU IS BUSY, SWITCH TO FAST SPEED!!
    // PREVENT HIGH SPEED UNDER LOW BATTERY CONDITION
    halSetNotification(N_HOURGLASS, 0xf);
    halRefresh(STATUS_DIRTY);
    halFlags |= HAL_HOURGLASS;
    if(halFlags & HAL_NOCLOCKCHANGE) {
        tmr_events[halBusyEvent].status |= 2;   // Set autoreload function
        // And do nothing
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
    // Default to 2 minutes
    halCPUSlowMode();
    halFlags = (halFlags & ~HAL_AUTOOFFTIME) |
               SET_AUTOOFFTIME(DEFAULT_AUTOOFFTIME);
    halBusyEvent =
            tmr_eventcreate(&busy_handler,
            (halFlags & HAL_QUICKRESPONSE) ? 30 : 500, 0);
}

void halSetBusyHandler()
{
    if (!(halFlags & HAL_FASTMODE))
    {
        // Start the event again
        if (halBusyEvent <= 0)
            halBusyEvent =
                tmr_eventcreate(&busy_handler,
                                (halFlags & HAL_QUICKRESPONSE) ? 30 : 500,
                                0);
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

    saved = rplNewBINT(halFlags, DECBINT);
    if(!saved)
        saved = (word_p) zero_bint;
    rplStoreSettings((word_p) savedflags_ident, saved);

}

// DO ANY PREPARATIONS BEFORE WAKEUP FROM POWEROFF
void halWakeUp()
{
    word_p saved;

#ifdef TARGET_PRIME
    // Restore UI theme
    saved = rplGetSettings((word_p) theme_ident);
    if (saved)
    {
        int error = 0;
        if (!ISLIST(*saved))
            error = 1;

        // Take a list of 64 integers and use them as palette entries
        if (!error && (rplListLength(saved) < PALETTE_SIZE))
            error = 1;

        int       k;
        word_p    obj = saved + 1;
        color16_t palette[PALETTE_SIZE];
        color16_t color;

        if (!error)
        {
            for (k = 0; k < PALETTE_SIZE; ++k)
            {
                color.value = rplReadNumberAsInt64(obj);
                if (Exceptions)
                {
                    rplClearErrors();
                    error = 1;
                    break;
                }
                palette[k] = color;
                obj        = rplSkipOb(obj);
            }
        }
        // Here we were able to read all numbers without any errors,
        // so it's a valid palette
        if (!error)
            halSetupTheme(palette);
        else
            rplPurgeSettings((word_p) theme_ident);
    }

#endif /* TARGET_PRIME */

    // Restore the flags
    saved = rplGetSettings((word_p) savedflags_ident);
    if (saved)
    {
        int32_t tmpflags = rplReadint32_t(saved);
        int32_t flagmask =
            (HAL_FASTMODE | HAL_HOURGLASS | HAL_SLOWLOCK |
             HAL_SKIPNEXTALARM); // SOME FLAGS SHOULD NOT BE PRESERVED
        halFlags = (tmpflags & (~flagmask)) | (halFlags & flagmask);
    }
    rplPurgeSettings((word_p) savedflags_ident);

    // After purge settings we must update the font arrays
    halUpdateFonts();

    // Restore the menu2 hidden status
    if(rplTestSystemFlag(FL_HIDEMENU2))
        halSetMenu2Height(0);
    else
        halSetMenu2Height(MENU2_HEIGHT);

    if(rplTestSystemFlag(FL_QUICKRESPONSE))
        halFlags |= HAL_QUICKRESPONSE;
    else
        halFlags &= ~HAL_QUICKRESPONSE;

    // Restore stack
    saved = rplGetSettings((word_p) stksave_ident);

    if (saved)
    {
        if (ISAUTOEXPLIST(*saved))
        {
            int32_t nitems = rplListLength(saved);
            rplExpandStack(nitems);
            if (!Exceptions)
            {
                word_p ptr = saved + 1;
                while (nitems--)
                {
                    rplPushDataNoGrow(ptr);
                    ptr = rplSkipOb(ptr);
                }
            }
        }
        else
        {
            rplPushData(saved);
        }
    }
    rplPurgeSettings((word_p) stksave_ident);

    // After purge settings we must update the font arrays
    halUpdateFonts();

    // Restore the command line
    saved = rplGetSettings((word_p) savedcmdline_ident);
    if (saved)
    {
        if (halRestoreCmdLine(saved))
            halSetContext(CONTEXT_EDITOR);
        rplPurgeSettings((word_p) savedcmdline_ident);
    }

    // After purge settings we must update the font arrays
    halUpdateFonts();

    // Flush the ON-key keypress from the keyboard buffer before entering the
    // outer loop. This can cancel an existing command line
    keyb_flush_no_wait();
    keyb_set_shift_plane(KSHIFT_NONE);

    halSetNotification(N_ALARM, rplCheckAlarms() ? 0xf : 0);
}
