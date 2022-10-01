/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <ui.h>
#include <keyboard.h>

extern unsigned int RPLLastOpcode;

void ex_print(int x, int y, char *str)
{
    gglsurface dr;
    dr.pixels = (pixword *)MEM_PHYS_EXSCREEN;
    dr.width = LCD_SCANLINE;
    dr.left = dr.top = 0;
    dr.right = LCD_W;
    dr.bottom = LCD_H;

    DrawTextMono(&dr, x, y, str, Font_6A, ggl_solid(PAL_GRAY1));
}

void ex_clrscreen()
{
    int *ptr = (int *)MEM_PHYS_EXSCREEN, *end = ptr + 400;
    while(ptr != end)
        *ptr++ = 0;
}

void ex_hline(int y)
{
    int *yptr = ((int *)MEM_PHYS_EXSCREEN) + 5 * y;
    yptr[0] = yptr[1] = yptr[2] = yptr[3] = yptr[4] = 0xaaaaaaaa;
}

int ex_width(char *string)
{
    return StringWidth(string, Font_6A);
}

int exception_handler(char *exstr, unsigned int *registers, int options)
{
    UNUSED(registers);

    unsigned int lcd_buffer[17];
    char a[10];

    int f, j;

    lcd_save(lcd_buffer);
    lcd_setmode(0, (unsigned int *)MEM_PHYS_EXSCREEN);

  doitagain:

    ex_clrscreen();

    if(options & EX_NOREG) {

        ex_print(36, 12, "-- EXCEPTION --");
        ex_hline(8);
        ex_hline(20);

        ex_print(65 - (ex_width(exstr) >> 1), 30, exstr);

    }
    else {
        ex_print(0, 0, "Exception: ");
        ex_print(44, 0, exstr);

        // ALWAYS SHOW RPL CORE INSTEAD OF REGISTERS IN PC TARGET
        ex_hline(8);
        // SHOW RPL CORE INFORMATION INSTEAD
        ex_print(0, 12, "IP: ");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) IPtr) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(16, 12, a);

        ex_print(0, 18, "OPC:");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) RPLLastOpcode) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(16, 18, a);

        ex_print(0, 24, "TOe:");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) TempObEnd) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(16, 24, a);

        ex_print(0, 30, "TOs:");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) TempObSize) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(16, 30, a);

        ex_print(0, 36, "TBe:");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) TempBlocksEnd) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(16, 36, a);

        ex_print(0, 42, "TBs:");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) TempBlocksSize) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(16, 42, a);

        ex_print(0, 48, "RSe:");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) RSTop) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(16, 48, a);

        ex_print(0, 54, "RSs:");

        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) RStkSize) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(16, 54, a);

        ex_print(0, 60, "DSe:");

        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) DSTop) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(16, 60, a);

        // RIGHT COLUMN

        ex_print(64, 12, "DSs: ");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) DStkSize) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(80, 12, a);

        ex_print(64, 18, "DIe:");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) DirsTop) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(80, 18, a);

        ex_print(64, 24, "DIs:");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) DirSize) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(80, 24, a);

        ex_print(64, 30, "LAe:");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) LAMTop) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(80, 30, a);

        ex_print(64, 36, "LAs:");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) LAMSize) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(80, 36, a);

        ex_print(64, 42, "Exc:");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) Exceptions) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(80, 42, a);

        ex_print(64, 48, "Err:");
        a[8] = 0;
        for(j = 7; j >= 0; j--) {
            a[7 - j] = ((((intptr_t) ErrorCode) >> (j << 2)) & 0xf) + 48;
            if(a[7 - j] > '9')
                a[7 - j] += 7;
        }
        ex_print(80, 48, a);

        ex_print(64, 54, "Um:");

        {
            WORD total = halGetTotalPages();
            WORD freemem = halGetFreePages();
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = (((freemem) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(80, 54, a);

            ex_print(64, 60, "Tm:");

            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = (((total) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(80, 60, a);
        }

        ex_hline(70);

    }

    if(options & EX_CONT) {
        int *pnewb = (int *)MEM_PHYS_EXSCREEN;
        // DRAW BUTTON 1
        ex_print(0, 12 * 6, "Cont");
        //pnewb[70*5]|=0x10000;
        for(f = 0; f < 8; ++f)
            pnewb[71 * 5 + 5 * f] |= 0x20000;
        pnewb[78 * 5] |= 0x3ffff;
        pnewb[79 * 5] |= 0x3fffc;
    }

    if(options & (EX_EXIT | EX_RPLEXIT)) {
        int *pnewb = (int *)MEM_PHYS_EXSCREEN;

        // DRAW BUTTON 2
        ex_print(5 * 4, 12 * 6, "Exit");
        for(f = 0; f < 8; ++f)
            pnewb[71 * 5 + 1 + 5 * f] |= 0x20;
        pnewb[78 * 5] |= 0xfff80000;
        pnewb[78 * 5 + 1] |= 0x3f;
        pnewb[79 * 5] |= 0xffe00000;
        pnewb[79 * 5 + 1] |= 0x3f;
    }

    if(options & EX_WARM) {
        int *pnewb = (int *)MEM_PHYS_EXSCREEN;

        // DRAW BUTTON 3
        if(options & EX_WIPEOUT)
            ex_print(11 * 4, 12 * 6, "*Clear Mem*");
        else
            ex_print(11 * 4, 12 * 6, "*Warmstart*");
        for(f = 0; f < 8; ++f)
            pnewb[71 * 5 + 2 + 5 * f] |= 0x2000000;
        pnewb[78 * 5 + 2] |= 0x3ffffff;
        pnewb[78 * 5 + 1] |= 0xfffff000;
        pnewb[79 * 5 + 2] |= 0x3ffffff;
        pnewb[79 * 5 + 1] |= 0xffffc000;
    }

    if(options & EX_RESET) {
        int *pnewb = (int *)MEM_PHYS_EXSCREEN;
        // DRAW BUTTON 4
        ex_print(23 * 4, 12 * 6, "**Reset**");
        for(f = 0; f < 9; ++f)
            pnewb[71 * 5 + 4 + 5 * f] |= 0x1;
        pnewb[78 * 5 + 3] |= 0xffffffff;
        pnewb[78 * 5 + 2] |= 0xf0000000;
        pnewb[79 * 5 + 3] |= 0xffffffff;
        pnewb[79 * 5 + 2] |= 0xc0000000;
    }

// WARMSTART AND RESET REQUIRE SIMULTANEOUS SHIFT OR ALPHA KEYPRESS

// WAIT FOR ALL KEYS TO BE RELEASED TO AVOID ACCIDENTAL KEYPRESSES

    keyb_irq_wait_release();

    do {
        f = keyb_irq_get_key();

        if(options & EX_CONT) {
            j = EX_CONT;
            if(KM_KEY(f) == KB_A)
                break;
        }
        if(options & (EX_EXIT | EX_RPLEXIT)) {
            j = options & (EX_EXIT | EX_RPLEXIT);
            if(KM_KEY(f) == KB_B)
                break;
        }
        if(KM_KEY(f) == KB_L) {
            options ^= EX_RPLREGS;
            options &= ~EX_NOREG;
            goto doitagain;
        }
// FORCE A SHIFTED KEY PRESS
        if(!KM_SHIFT(f))
            continue;

        if(options & (EX_WARM | EX_WIPEOUT)) {
            if((options & EX_WIPEOUT)
                    && (KM_SHIFT(f) ==
                        (KSHIFT_ALPHA | KHOLD_ALPHA | KSHIFT_RIGHT | KHOLD_RIGHT
                            | KSHIFT_LEFT | KHOLD_LEFT)))
                j = EX_WIPEOUT;
            else
                j = EX_WARM;
            if(KM_KEY(f) == KB_C)
                break;
            if(KM_KEY(f) == KB_D)
                break;
        }

        if(options & EX_RESET) {
            j = EX_RESET;
            if(KM_KEY(f) == KB_E)
                break;
            if(KM_KEY(f) == KB_F)
                break;
        }
    }
    while(1);

    keyb_irq_wait_release();
    lcd_restore(lcd_buffer);

    return j;

}

void exception_install()
{

}

void __attribute__((noinline)) throw_exception(cstring message,
                                               unsigned options)
{
    int value;

    value = exception_handler((char *)message, NULL, options | EX_NOREG);

    if(value == EX_RPLEXIT) {
        Exceptions |= EX_EXITRPL;
        ExceptionPointer = IPtr;
        value = EX_CONT;
    }

    if(value == EX_RESET) {
        // TODO: RESET ON THE PC TARGET

    }
    if(value == EX_WARM) {
        // TODO: WARMSTART ON THE PC TARGET
    }
    if(value == EX_WIPEOUT) {
        // TODO: WIPEOUT ON THE PC TARGET

    }
}

void __attribute__((noinline)) throw_dbgexception(cstring message,
                                                  unsigned options)
{
    int value;

    value = exception_handler((char *)message, NULL, options);

    if(value == EX_RPLEXIT) {
        Exceptions |= EX_EXITRPL;
        ExceptionPointer = IPtr;
        value = EX_CONT;
    }

    if(value == EX_RESET) {
        // TODO: RESET ON THE PC TARGET

    }
    if(value == EX_WARM) {
        // TODO: WARMSTART ON THE PC TARGET
    }
    if(value == EX_WIPEOUT) {
        // TODO: WIPEOUT ON THE PC TARGET

    }

}
