/*
 * Copyright (c) 2014-2020, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#include <newrpl.h>
#include <hal_api.h>
#include <ggl.h>

__ARM_MODE__ void startup(int) __attribute__((noreturn));
void startup(int prevstate)
{
    lcd_setmode(BPPMODE_4BPP, (unsigned int *)MEM_PHYS_SCREEN);
    lcd_on();

    gglsurface scr;
    
    ggl_initscr(&scr);
    ggl_rect(&scr, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, 0);

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            ggl_rect(&scr,
                    x * 10 + 10, y * 10 + 10,
                    x * 10 + 20, y * 10 + 20,
                    ggl_mkcolor(x + y)
                    );
        }
    }

    while(1);
}
