// ****************************************************************************
//  keyboard.c<compiler>                                          DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Stubs for keyboard functions in the compiler
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms in LICENSE.txt
// ****************************************************************************

#include <host/pc/keyboard.c>

typedef unsigned long long keymatrix;
void keyb_irq_shift_logic(unsigned key, keymatrix hwkeys, keymatrix changes)
{
    UNUSED(key);
    UNUSED(hwkeys);
    UNUSED(changes);
}
