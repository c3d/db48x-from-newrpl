/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <ui.h>
#include "recorder.h"


RECORDER(exceptions, 16, "System exceptions");

extern unsigned int RPLLastOpcode;

ARM_MODE void ex_print(int x, int y, char *str)
{
    gglsurface dr;
    dr.pixels = (int *)MEM_PHYS_EXSCREEN;
    dr.width = LCD_SCANLINE;
    dr.left = dr.top = 0;
    dr.right = LCD_W;
    dr.bottom = LCD_H;

    DrawTextMono(&dr, x, y, str, Font_6A, ggl_solid(PAL_GRAY1));
}

ARM_MODE void ex_clrscreen()
{
    int *ptr = (int *)MEM_PHYS_EXSCREEN, *end = ptr + 400;
    while(ptr != end)
        *ptr++ = 0;
}

ARM_MODE void ex_hline(int y)
{
    int *yptr = ((int *)MEM_PHYS_EXSCREEN) + 5 * y;
    yptr[0] = yptr[1] = yptr[2] = yptr[3] = yptr[4] = 0xaaaaaaaa;
}

#define ex_width(string) StringWidth((string),Font_6A)

// GET HIGH REGISTERS R8 TO R14 + CPSR (8 WORDS)

ARM_MODE void ex_gethireg(unsigned int *hi_reg)
{
    register unsigned int tmp asm("r3");
    register unsigned int tmp2 asm("r2");

    asm volatile ("mrs %0,cpsr":"=r" (tmp));
    asm volatile ("mrs %0,spsr":"=r" (tmp2));
    asm volatile ("bic %0,%1,#0x1f":"=r" (tmp):"r"(tmp));       // CLEAN STATE BITS
    asm volatile ("and %0,%1,#0x1f":"=r" (tmp2):"r"(tmp2));     // ISOLATE ORIGINAL STATUS
    asm volatile ("orr %0,%1,%2":"=r" (tmp):"r"(tmp), "r"(tmp2));       // APPLY ORIGINAL STATUS
    asm volatile ("cmp %0,#0x10":"=r" (tmp2));  // CHECK FOR USER MODE
    asm volatile ("orreq %0,%1,#0x1f":"=r" (tmp):"r"(tmp));     // IF USER MODE, SWITCH TO SYSTEM MODE
    asm volatile ("mrs %0,cpsr":"=r" (tmp2));
    asm volatile ("msr cpsr,%0":"=r" (tmp));
    asm volatile ("stmia r0,{r8,r9,r10,r11,r12,r13,r14}");
    asm volatile ("msr cpsr,%0":"=r" (tmp2));

    asm volatile ("mrs %0,spsr":"=r" (tmp));

    hi_reg[7] = tmp;

    return;
}

ARM_MODE int exception_handler(char *exstr, unsigned int *registers,
        int options)
{
    unsigned int lcd_buffer[17];
    unsigned int hi_reg[8];
    char a[10];

    int f, j;

    lcd_save(lcd_buffer);
    lcd_setmode(0, (int *)MEM_PHYS_EXSCREEN);

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

        if(options & EX_RPLREGS) {
            ex_hline(8);
            // SHOW RPL CORE INFORMATION INSTEAD
            ex_print(0, 12, "IP: ");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) IPtr) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(16, 12, a);

            ex_print(0, 18, "OPC:");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) RPLLastOpcode) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(16, 18, a);

            ex_print(0, 24, "TOe:");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) TempObEnd) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(16, 24, a);

            ex_print(0, 30, "TOs:");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) TempObSize) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(16, 30, a);

            ex_print(0, 36, "TBe:");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) TempBlocksEnd) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(16, 36, a);

            ex_print(0, 42, "TBs:");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) TempBlocksSize) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(16, 42, a);

            ex_print(0, 48, "RSe:");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) RSTop) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(16, 48, a);

            ex_print(0, 54, "RSs:");

            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) RStkSize) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(16, 54, a);

            ex_print(0, 60, "DSe:");

            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) DSTop) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(16, 60, a);

            // RIGHT COLUMN

            ex_print(64, 12, "DSs: ");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) DStkSize) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(80, 12, a);

            ex_print(64, 18, "DIe:");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) DirsTop) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(80, 18, a);

            ex_print(64, 24, "DIs:");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) DirSize) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(80, 24, a);

            ex_print(64, 30, "LAe:");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) LAMTop) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(80, 30, a);

            ex_print(64, 36, "LAs:");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) LAMSize) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(80, 36, a);

            ex_print(64, 42, "Exc:");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) Exceptions) >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(80, 42, a);

            ex_print(64, 48, "Err:");
            a[8] = 0;
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((((WORD) ErrorCode) >> (j << 2)) & 0xf) + 48;
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

        }
        else {
            // PRINT CPU REGISTERS
            a[2] = 0;
            for(f = 0; f < 8; ++f) {
                ex_print(0, f * 6 + 12, "R :");
                a[1] = 0;
                a[0] = f + 48;
                ex_print(4, f * 6 + 12, a);
                ex_print(48, f * 6 + 12, "R  :");
                if(f < 2)
                    a[0] = f + 8 + 48;
                else {
                    a[0] = '1';
                    a[1] = f - 2 + 48;
                }
                ex_print(52, f * 6 + 12, a);
            }

            ex_hline(8);

            a[8] = 0;

            // DISPLAY REGISTERS

            for(f = 0; f < 8; ++f) {
                for(j = 7; j >= 0; j--) {
                    a[7 - j] = ((registers[f + 1] >> (j << 2)) & 0xf) + 48;
                    if(a[7 - j] > '9')
                        a[7 - j] += 7;
                }
                ex_print(12, f * 6 + 12, a);

            }

            // DISPLAY BANKED REGISTERS
            // GET BANKED REGISTERS
            ex_gethireg(hi_reg);

            for(f = 0; f < 7; ++f) {

                for(j = 7; j >= 0; j--) {
                    a[7 - j] = ((hi_reg[f] >> (j << 2)) & 0xf) + 48;
                    if(a[7 - j] > '9')
                        a[7 - j] += 7;
                }
                ex_print(64, f * 6 + 12, a);

            }

            // PC
            for(j = 7; j >= 0; j--) {
                a[7 - j] = ((registers[0] >> (j << 2)) & 0xf) + 48;
                if(a[7 - j] > '9')
                    a[7 - j] += 7;
            }
            ex_print(64, 54, a);

            // FLAGS
            if(hi_reg[7] & 0x80000000) {
                ex_print(31 * 4, 2 * 6, "N");
                ex_print(28 * 4, 2 * 6, "MI");
            }
            else
                ex_print(28 * 4, 2 * 6, "PL");
            if(hi_reg[7] & 0x40000000) {
                ex_print(31 * 4, 3 * 6, "Z");
                ex_print(28 * 4, 3 * 6, "EQ");
            }
            else
                ex_print(28 * 4, 3 * 6, "NE");
            if(hi_reg[7] & 0x20000000) {
                ex_print(31 * 4, 4 * 6, "C");
                ex_print(28 * 4, 4 * 6, "CS");
            }
            else
                ex_print(28 * 4, 4 * 6, "CC");
            if(hi_reg[7] & 0x10000000) {
                ex_print(31 * 4, 5 * 6, "V");
                ex_print(28 * 4, 5 * 6, "VS");
            }
            else
                ex_print(28 * 4, 5 * 6, "VC");
            if((hi_reg[7] & 0x60000000) == 0x20000000) {
                ex_print(28 * 4, 6 * 6, "HI");
            }
            else
                ex_print(28 * 4, 6 * 6, "LS");
            if((hi_reg[7] ^ (hi_reg[7] >> 3)) & 0x10000000) {
                ex_print(28 * 4, 7 * 6, "LT");
            }
            else {
                if(!(hi_reg[7] & 0x40000000))
                    ex_print(28 * 4, 7 * 6, "GT");
            }

            if((hi_reg[7] & 0x40)) {
                ex_print(27 * 4, 8 * 6, "F");
            }
            if((hi_reg[7] & 0x80)) {
                ex_print(31 * 4, 8 * 6, "I");
            }

            if((hi_reg[7] & 0x1f) == 0x10) {
                ex_print(27 * 4, 9 * 6, "USER");
            }
            if((hi_reg[7] & 0x1f) == 0x11) {
                ex_print(27 * 4, 9 * 6, "FIQ");
                for(f = 0; f < 7; ++f) {
                    ex_print(24 * 4, f * 6 + 12, "/B");
                }
            }
            if((hi_reg[7] & 0x1f) == 0x12) {
                ex_print(27 * 4, 9 * 6, "IRQ");
                for(f = 5; f < 7; ++f) {
                    ex_print(24 * 4, f * 6 + 12, "/B");
                }
            }
            if((hi_reg[7] & 0x1f) == 0x13) {
                ex_print(27 * 4, 9 * 6, "SUP");
                for(f = 5; f < 7; ++f) {
                    ex_print(24 * 4, f * 6 + 12, "/B");
                }
            }
            if((hi_reg[7] & 0x1f) == 0x17) {
                ex_print(27 * 4, 9 * 6, "ABT");
                for(f = 5; f < 7; ++f) {
                    ex_print(24 * 4, f * 6 + 12, "/B");
                }
            }
            if((hi_reg[7] & 0x1f) == 0x1B) {
                ex_print(27 * 4, 9 * 6, "UND");
                for(f = 5; f < 7; ++f) {
                    ex_print(24 * 4, f * 6 + 12, "/B");
                }
            }
            if((hi_reg[7] & 0x1f) == 0x1f) {
                ex_print(27 * 4, 9 * 6, "SYS");
            }

            if(hi_reg[7] & 0x20)
                ex_print(27 * 4, 10 * 6, "Thumb");
            else
                ex_print(28 * 4, 10 * 6, "ARM");
        }
        ex_hline(70);

    }

    if(options & EX_CONT) {
        int *pnewb = (int *)MEM_PHYS_EXSCREEN;
        // DRAW BUTTON 1
        ex_print(0, (LCD_H - 8), "Cont");
        //pnewb[70*5]|=0x10000;
        for(f = 0; f < 8; ++f)
            pnewb[(LCD_H - 9) * 5 + 5 * f] |= 0x20000;
        pnewb[(LCD_H - 2) * 5] |= 0x3ffff;
        pnewb[(LCD_H - 1) * 5] |= 0x3fffc;
    }

    if(options & (EX_EXIT | EX_RPLEXIT)) {
        int *pnewb = (int *)MEM_PHYS_EXSCREEN;

        // DRAW BUTTON 2
        ex_print(5 * 4, (LCD_H - 8), "Exit");
        for(f = 0; f < 8; ++f)
            pnewb[(LCD_H - 9) * 5 + 1 + 5 * f] |= 0x20;
        pnewb[(LCD_H - 2) * 5] |= 0xfff80000;
        pnewb[(LCD_H - 2) * 5 + 1] |= 0x3f;
        pnewb[(LCD_H - 1) * 5] |= 0xffe00000;
        pnewb[(LCD_H - 1) * 5 + 1] |= 0x3f;
    }

    if(options & EX_WARM) {
        int *pnewb = (int *)MEM_PHYS_EXSCREEN;

        // DRAW BUTTON 3
        if(options & EX_WIPEOUT)
            ex_print(11 * 4, (LCD_H - 8), "*Clear Mem*");
        else
            ex_print(11 * 4, (LCD_H - 8), "*Warmstart*");
        for(f = 0; f < 8; ++f)
            pnewb[(LCD_H - 9) * 5 + 2 + 5 * f] |= 0x2000000;
        pnewb[(LCD_H - 2) * 5 + 2] |= 0x3ffffff;
        pnewb[(LCD_H - 2) * 5 + 1] |= 0xfffff000;
        pnewb[(LCD_H - 1) * 5 + 2] |= 0x3ffffff;
        pnewb[(LCD_H - 1) * 5 + 1] |= 0xffffc000;
    }

    if(options & EX_RESET) {
        int *pnewb = (int *)MEM_PHYS_EXSCREEN;
        // DRAW BUTTON 4
        ex_print(23 * 4, (LCD_H - 8), "**Reset**");
        for(f = 0; f < 9; ++f)
            pnewb[(LCD_H - 9) * 5 + 4 + 5 * f] |= 0x1;
        pnewb[(LCD_H - 2) * 5 + 3] |= 0xffffffff;
        pnewb[(LCD_H - 2) * 5 + 2] |= 0xf0000000;
        pnewb[(LCD_H - 1) * 5 + 3] |= 0xffffffff;
        pnewb[(LCD_H - 1) * 5 + 2] |= 0xc0000000;
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

void handler_dabort(void) __attribute__((naked));
ARM_MODE void handler_dabort(void)
{
    // STORE ALL REGISTERS INTO THE SAFE STACK
    asm volatile ("stmfd sp!, {r0-r12, r14}");
    asm volatile ("mrs r0,cpsr");
    asm volatile ("orr r0,r0,#0xc0");   // DISABLE ALL FIQ AND IRQ INTERRUPTS
    asm volatile ("msr cpsr,r0");

    asm volatile ("sub r14,r14,#8");    // CHANGE FOR ALL DIFFERENT EXCEPTIONS TO POINT TO THE RIGHT PC
    asm volatile ("str r14,[sp,#-4]!");

    register unsigned int *stackptr asm("sp");
    // CALL CUSTOM HANDLER
    register int f =
            exception_handler("Data abort", stackptr,
            EX_CONT | EX_EXIT | EX_WARM | EX_RESET);

    if(f == EX_CONT) {
        // RESTORE ALL REGISTERS
        asm volatile ("add sp,sp,#4");
        asm volatile ("ldmfd sp!, {r0-r12, r14}");

        // RETURN TO THE NEXT INSTRUCTION AFTER DATA FAILED (DON'T RETRY)
        asm volatile ("subs pc, r14,#4");

    }

    asm volatile (".Ldohandlerexit:");

    /*
       if(f==EX_EXIT) {

       asm volatile ("ldr lr, .Lexit");
       asm volatile ("b .Lretexit");

       }
     */

    if(f == EX_WARM) {
        asm volatile ("ldr lr, .Lexitwarm");
        asm volatile ("b .Lretexit");
    }

    if(f == EX_WIPEOUT) {
        asm volatile ("ldr lr, .Lexitwipeout");
        asm volatile ("b .Lretexit");
    }

    // DEFAULT = EXIT W/RESET

    asm volatile ("ldr lr, .Lexitreset");

    asm volatile (".Lretexit:");

    // MASK ALL INTERRUPTS AND CLEAR ALL PENDING REQUESTS
    *HWREG(INT_REGS, 0x8) = 0xffffffff;
    *HWREG(INT_REGS, 0) = 0xffffffff;
    *HWREG(INT_REGS, 0x10) = 0xffffffff;

    register unsigned int tmp asm("r0");

    asm volatile ("mrs %0,spsr":"=r" (tmp));
    asm volatile ("bic %0,%1,#0xff":"=r" (tmp):"r"(tmp));       // CLEAN STATE BITS
    asm volatile ("orr %0,%1, #0x1f":"=r" (tmp):"r"(tmp));      // SET SYSTEM CPU MODE W/IRQ AND FIQ ENABLED, ARM MODE
    asm volatile ("tst lr,#1");
    asm volatile ("orrne %0,%1, #0x20":"=r" (tmp):"r"(tmp));    // SET THUMB MODE IF THE RETURN ADDRESS IS THUMB
    asm volatile ("msr spsr,%0":"=r" (tmp));    // SAVE USER MODE STATUS

    // RESTORE ALL REGISTERS
    asm volatile ("add sp,sp,#4");
    asm volatile ("ldmfd sp!, {r0-r12}");
    asm volatile ("add sp,sp,#4");

    asm volatile ("mov r0,#-1");        // EXIT WITH CODE -1

    // RETURN DIRECTLY INTO THE exit FUNCTION
    asm volatile ("movs pc, r14");

    asm volatile (".Lexitwarm: .word halWarmStart");
    asm volatile (".Lexitreset: .word halReset");
    asm volatile (".Lexitwipeout: .word halWipeoutWarmStart");

}

void handler_iabort(void) __attribute__((naked));
ARM_MODE void handler_iabort(void)
{
    register unsigned int *stackptr asm("sp");

    // STORE ALL REGISTERS INTO THE SAFE STACK
    asm volatile ("stmfd sp!, {r0-r12, r14}");
    asm volatile ("mrs r0,cpsr");
    asm volatile ("orr r0,r0,#0xc0");   // DISABLE ALL FIQ AND IRQ INTERRUPTS
    asm volatile ("msr cpsr,r0");
    asm volatile ("sub r14,r14,#4");    // CHANGE FOR ALL DIFFERENT EXCEPTIONS TO POINT TO THE RIGHT PC
    asm volatile ("str r14,[sp,#-4]!");

    // CALL CUSTOM HANDLER
    register int f =
            exception_handler("Prefetch abort", stackptr,
            EX_CONT | EX_WARM | EX_RESET);

    if(f == EX_CONT) {
        // RESTORE ALL REGISTERS
        asm volatile ("add sp,sp,#4");
        asm volatile ("ldmfd sp!, {r0-r12, r14}");

        // RETURN TO THE NEXT INSTRUCTION AFTER DATA FAILED (DON'T RETRY)
        asm volatile ("movs pc, r14");

    }

    asm volatile ("b .Ldohandlerexit");

}

void handler_und(void) __attribute__((naked));
ARM_MODE void handler_und(void)
{
    register unsigned int *stackptr asm("sp");
    register unsigned int value asm("r0");

    // STORE ALL REGISTERS INTO THE SAFE STACK
    asm volatile ("stmfd sp!, {r0-r12, r14}");
    asm volatile ("sub r14,r14,#4");    // CHANGE FOR ALL DIFFERENT EXCEPTIONS TO POINT TO THE RIGHT PC
    asm volatile ("mrs r0,cpsr");
    asm volatile ("orr r0,r0,#0xc0");   // DISABLE ALL FIQ AND IRQ INTERRUPTS
    asm volatile ("msr cpsr,r0");
    asm volatile ("mrs r0,spsr");
    asm volatile ("ands r0,r0,#0x20");
    asm volatile ("add r14,r14,r0,LSR #4");     // ADD 2 IF IN THUMB MODE (PC+2)

    asm volatile ("str r14,[sp,#-4]!");

    // DO NOT LOAD ANY VALUE IF IN THUMB MODE
    asm volatile ("ldreq %0,[r14]":"=r" (value):"r"(value));    // GET OFFENDING INSTRUCTION

    if(value == 0xe6cccc10) {

        value = exception_handler((char *)(stackptr[1]), stackptr,
                stackptr[2] | EX_NOREG);

    }
    else {
        // CALL CUSTOM HANDLER
        if(value == 0xe6dddd10)
            value = exception_handler((char *)(stackptr[1]), stackptr,
                    stackptr[2]);
        else
            value = exception_handler("Undefined instruction", stackptr,
                    EX_CONT | EX_WARM | EX_RESET);
    }

    if(value == EX_RPLEXIT) {
        // RAISE AN RPL EXCEPTION AND ISSUE A CONTINUE
        Exceptions |= EX_EXITRPL;
        ExceptionPointer = IPtr;
        value = EX_CONT;
    }

    if(value == EX_CONT) {
        // RESTORE ALL REGISTERS
        asm volatile ("add sp,sp,#4");
        asm volatile ("ldmfd sp!, {r0-r12, r14}");

        // RETURN TO THE NEXT INSTRUCTION AFTER DATA FAILED (DON'T RETRY)
        asm volatile ("movs pc, r14");

    }

    asm volatile ("b .Ldohandlerexit");

}

ARM_MODE void exception_install()
{
    unsigned *handler_addr = (unsigned int *)0x08000000L;
    handler_addr[1] = (unsigned int)(&handler_und);
    handler_addr[3] = (unsigned int)(&handler_iabort);
    handler_addr[4] = (unsigned int)(&handler_dabort);

    irq_install();
}

ARM_MODE void throw_exception(cstring message, unsigned options)
{
    asm volatile (".word 0xE6CCCC10");
}

ARM_MODE void throw_dbgexception(cstring message, unsigned options)
{
    asm volatile (".word 0xE6DDDD10");
}


#ifndef NDEBUG
void exit(int rc)
{
    record(exceptions, "exit(%d), reseting system", rc);
    halReset();
    while(1);
}
#endif // NDEBUG
