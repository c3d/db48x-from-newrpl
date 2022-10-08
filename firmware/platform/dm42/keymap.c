/*
 * Copyright (c) 2022, Christophe de Dinechin and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 *
 * Default keymap for the HP Prime
 */

static const  key_handler_fn keymap_unshifted[64] =
// ----------------------------------------------------------------------------
//   Key handlers for unshifted keys
// ----------------------------------------------------------------------------
{
    // Numbers
    [KB_0] = KH(number),
    [KB_1] = KH(number),
    [KB_2] = KH(number),
    [KB_3] = KH(number),
    [KB_4] = KH(number),
    [KB_5] = KH(number),
    [KB_6] = KH(number),
    [KB_7] = KH(number),
    [KB_8] = KH(number),
    [KB_9] = KH(number),
    [KB_DOT] = KH(decimalDot),

    // ON, BSP, ENTER, SPC
    [KB_ON] = KH(cancel),
    [KB_ENT] = KH(enter),
    [KB_BKS] = KH(backsp),
    [KB_RUNSTOP] = KH(eval),

    // Cursor keys (only two of them, darn!)
    [KB_UP] = KH(upOrLeft),
    [KB_DN] = KH(downOrRight),

    // Operators
    [KB_ADD] = KH(add),
    [KB_SUB] = KH(sub),
    [KB_DIV] = KH(div),
    [KB_MUL] = KH(mul),

     // Soft menu keys
    [KB_F1] = KH(softmenu1_1),
    [KB_F2] = KH(softmenu2_1),
    [KB_F3] = KH(softmenu3_1),
    [KB_F4] = KH(softmenu4_1),
    [KB_F5] = KH(softmenu5_1),
    [KB_F6] = KH(softmenu6_1),

    // Normal keys
    [KB_A] = KH(softmenu1_2),
    [KB_B] = KH(softmenu2_2),
    [KB_C] = KH(softmenu3_2),
    [KB_D] = KH(softmenu4_2),
    [KB_E] = KH(softmenu5_2),
    [KB_F] = KH(softmenu6_2),
    [KB_G] = KH(sto),
    [KB_H] = KH(VariablesMenu),
    [KB_I] = KH(StackMenu),
    [KB_J] = KH(sin),
    [KB_K] = KH(cos),
    [KB_L] = KH(tan),
    [KB_M] = KH(ticks),
    [KB_N] = KH(chs),
    [KB_O] = KH(eex),
};


static const  key_handler_fn keymap_left_shift[64] =
// ----------------------------------------------------------------------------
//   Key handlers with left shift
// ----------------------------------------------------------------------------
{
    // Numbers
    [KB_0] = KH(SystemMenu),
    [KB_1] = KH(HOME),
    [KB_2] = KH(LibrariesMenu),
    [KB_3] = KH(ProgramMenu),
    [KB_4] = KH(BasesMenu),
    [KB_5] = KH(UnitsMenu),
    [KB_6] = KH(SettingsMenu),
    [KB_7] = KH(SolverMenu),
    [KB_8] = KH(EquationsMenu),
    [KB_9] = KH(MatrixMenu),
    [KB_DOT] = KH(InfoMenu),

    // ON, BSP, ENTER, SPC
    [KB_ON] = KH(powerOff),
    [KB_ENT] = KH(CharsMenu),
    [KB_BKS] = KH(del),
    [KB_SPC] = KH(secoBracket),

    // Cursor keys (only two of them, darn!)
    [KB_UP] = KH(upOrLeft),
    [KB_DN] = KH(downOrRight),

    // Operators
    [KB_ADD] = KH(MainMenu),
    [KB_SUB] = KH(DevicesMenu),
    [KB_MUL] = KH(ProbaMenu),
    [KB_DIV] = KH(StatisticsMenu),

     // Soft menu keys
    [KB_F1] = KH(softmenu1_2),
    [KB_F2] = KH(softmenu2_2),
    [KB_F3] = KH(softmenu3_2),
    [KB_F4] = KH(softmenu4_2),
    [KB_F5] = KH(softmenu5_2),
    [KB_F6] = KH(softmenu6_2),

    // Normal keys
    [KB_A] = KH(softmenu1_2),
    [KB_B] = KH(softmenu2_2),
    [KB_C] = KH(softmenu3_2),
    [KB_D] = KH(softmenu4_2),
    [KB_E] = KH(softmenu5_2),
    [KB_F] = KH(softmenu6_2),
    [KB_G] = KH(ComplexMenu),
    [KB_H] = KH(tofrac),
    [KB_I] = KH(ConstantsMenu),
    [KB_J] = KH(asin),
    [KB_K] = KH(acos),
    [KB_L] = KH(atan),
    [KB_M] = KH(curlyBracket),
    [KB_N] = KH(squareBracket),
    [KB_O] = KH(textBracket),
};


static const  key_handler_fn keymap_left_hold[64] =
// ----------------------------------------------------------------------------
//   Overrides for left hold
// ----------------------------------------------------------------------------
{
    [KB_UP] = KH(copy),
    [KB_DN] = KH(cut),
    [KB_ENT] = KH(paste),
    [KB_BKS] = KH(clear),

    // Simulate cursor keys with the keypad
    [KB_8] = KH(up),
    [KB_4] = KH(left),
    [KB_6] = KH(right),
    [KB_2] = KH(down),

    [KB_7] = KH(pageUp),
    [KB_1] = KH(pageDown),
    [KB_9] = KH(startSelection),
    [KB_3] = KH(endSelection),
};


static const  key_handler_fn keymap_right_shift[64] =
// ----------------------------------------------------------------------------
//   Key handlers with right shift
// ----------------------------------------------------------------------------
{
    // Numbers
    [KB_0] = KH(SystemMenu),
    [KB_1] = KH(HOME),
    [KB_2] = KH(LibrariesMenu),
    [KB_3] = KH(ProgramMenu),
    [KB_4] = KH(BasesMenu),
    [KB_5] = KH(UnitsMenu),
    [KB_6] = KH(SettingsMenu),
    [KB_7] = KH(SolverMenu),
    [KB_8] = KH(EquationsMenu),
    [KB_9] = KH(MatrixMenu),
    [KB_DOT] = KH(InfoMenu),

    // ON, BSP, ENTER, SPC
    [KB_ON]  = KH(SystemMenu),
    [KB_ENT] = KH(CharsMenu),
    [KB_BKS] = KH(del),
    [KB_SPC] = KH(secoBracket),

    // Cursor keys (only two of them, darn!)
    [KB_UP] = KH(startSelection),
    [KB_DN] = KH(endSelection),

    // Operators
    [KB_ADD] = KH(MainMenu),
    [KB_SUB] = KH(DevicesMenu),
    [KB_MUL] = KH(ProbaMenu),
    [KB_DIV] = KH(StatisticsMenu),

     // Soft menu keys
    [KB_F1] = KH(softmenu1_2),
    [KB_F2] = KH(softmenu2_2),
    [KB_F3] = KH(softmenu3_2),
    [KB_F4] = KH(softmenu4_2),
    [KB_F5] = KH(softmenu5_2),
    [KB_F6] = KH(softmenu6_2),

    // Normal keys
    [KB_A] = KH(softmenu1_2),
    [KB_B] = KH(softmenu2_2),
    [KB_C] = KH(softmenu3_2),
    [KB_D] = KH(softmenu4_2),
    [KB_E] = KH(softmenu5_2),
    [KB_F] = KH(softmenu6_2),
    [KB_G] = KH(ComplexMenu),
    [KB_H] = KH(tofrac),
    [KB_I] = KH(ConstantsMenu),
    [KB_J] = KH(asin),
    [KB_K] = KH(acos),
    [KB_L] = KH(atan),
    [KB_M] = KH(curlyBracket),
    [KB_N] = KH(squareBracket),
    [KB_O] = KH(textBracket),
};


static const  key_handler_fn keymap_right_hold[64] =
// ----------------------------------------------------------------------------
//   Overrides for right hold
// ----------------------------------------------------------------------------
{
    [KB_UP] = KH(copy),
    [KB_DN] = KH(cut),
    [KB_ENT] = KH(paste),
    [KB_BKS] = KH(clear),

    // Simulate cursor keys with the keypad
    [KB_8] = KH(up),
    [KB_4] = KH(left),
    [KB_6] = KH(right),
    [KB_2] = KH(down),

    [KB_7] = KH(pageUp),
    [KB_1] = KH(pageDown),
    [KB_9] = KH(startSelection),
    [KB_3] = KH(endSelection),
};


static const  key_handler_fn keymap_alpha[64] =
// ----------------------------------------------------------------------------
//   Alpha mode key handlers
// ----------------------------------------------------------------------------
{
    [KB_0] = KH(colon),
    [KB_DOT] = KH(comma),
    [KB_SPC] = KH(spc),

    // Cursor keys
    [KB_UP] = KH(upOrLeft),
    [KB_DN] = KH(downOrRight),

    // Operators
    [KB_ADD] = KH(CharsMenu),
    [KB_SUB] = KH(underscore),

     // Alphabetic keys
    [KB_A] = KH(a),
    [KB_B] = KH(b),
    [KB_C] = KH(c),
    [KB_D] = KH(d),
    [KB_E] = KH(e),
    [KB_F] = KH(f),
    [KB_G] = KH(g),
    [KB_H] = KH(h),
    [KB_I] = KH(i),
    [KB_J] = KH(j),
    [KB_K] = KH(k),
    [KB_L] = KH(l),
    [KB_M] = KH(m),
    [KB_N] = KH(n),
    [KB_O] = KH(o),
    [KB_P] = KH(p),
    [KB_Q] = KH(q),
    [KB_R] = KH(r),
    [KB_S] = KH(s),
    [KB_T] = KH(t),
    [KB_U] = KH(u),
    [KB_V] = KH(v),
    [KB_W] = KH(w),
    [KB_X] = KH(x),
    [KB_Y] = KH(y),
    [KB_Z] = KH(z),
};


static const  key_handler_fn keymap_alpha_hold[64] =
// ----------------------------------------------------------------------------
//   Overrides for alpha hold
// ----------------------------------------------------------------------------
{
    NULL
};


static const key_handler_fn keymap_on_hold[64] =
// ----------------------------------------------------------------------------
//   Functions when holding the ON key
// ----------------------------------------------------------------------------
{
    // Numeric keypad
    [KB_0] = KH(onDigit),
    [KB_1] = KH(onDigit),
    [KB_2] = KH(onDigit),
    [KB_3] = KH(onDigit),
    [KB_4] = KH(onDigit),
    [KB_5] = KH(onDigit),
    [KB_6] = KH(onDigit),
    [KB_7] = KH(onDigit),
    [KB_8] = KH(onDigit),
    [KB_9] = KH(onDigit),

    [KB_ADD] = KH(increaseContrast),
    [KB_SUB] = KH(decreaseContrast),
    [KB_MUL] = KH(previousScale),
    [KB_DIV] = KH(nextScale),
    [KB_DOT] = KH(cycleLocale),
    [KB_SPC] = KH(cycleFormat),

    [KB_B] = KH(SkipNextAlarm),
    [KB_M] = KH(ToggleMenu),
};
