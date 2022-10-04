/*
 * Copyright (c) 2022, Christophe de Dinechin and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 *
 * Default keymap for the HP 50g
 */

static const  key_handler_fn keymap_unshifted[64] =
// ----------------------------------------------------------------------------
//   Key handlers for unshifted keys
// ----------------------------------------------------------------------------
{
    // Numbers
    [KB_1] = KH(number),
    [KB_2] = KH(number),
    [KB_3] = KH(number),
    [KB_4] = KH(number),
    [KB_5] = KH(number),
    [KB_6] = KH(number),
    [KB_7] = KH(number),
    [KB_8] = KH(number),
    [KB_9] = KH(number),
    [KB_0] = KH(number),
    [KB_DOT] = KH(decimalDot),

    // ON, BSP, ENTER, SPC
    [KB_ON] = KH(cancel),
    [KB_ENT] = KH(enter),
    [KB_BKS] = KH(backsp),
    [KB_SPC] = KH(spc),

    // Cursor keys
    [KB_LF] = KH(left),
    [KB_RT] = KH(right),
    [KB_DN] = KH(down),
    [KB_UP] = KH(up),

    // Operators
    [KB_ADD] = KH(add),
    [KB_SUB] = KH(sub),
    [KB_DIV] = KH(div),
    [KB_MUL] = KH(mul),

     // Soft menu keys
    [KB_A] = KH(softmenu1_1),
    [KB_B] = KH(softmenu2_1),
    [KB_C] = KH(softmenu3_1),
    [KB_D] = KH(softmenu4_1),
    [KB_E] = KH(softmenu5_1),
    [KB_F] = KH(softmenu6_1),

    [KB_G] = KH(softmenu1_2),
    [KB_H] = KH(softmenu2_2),
    [KB_I] = KH(softmenu3_2),
    [KB_J] = KH(softmenu4_2),
    [KB_K] = KH(softmenu5_2),
    [KB_L] = KH(softmenu6_2),

    // Normal commands/functions
    [KB_M] = KH(sto),
    [KB_N] = KH(eval),
    [KB_O] = KH(ticks),
    [KB_P] = KH(MainMenu),
    [KB_Q] = KH(pow),
    [KB_R] = KH(sqrt),
    [KB_S] = KH(sin),
    [KB_T] = KH(cos),
    [KB_U] = KH(tan),
    [KB_V] = KH(eex),
    [KB_W] = KH(chs),
    [KB_X] = KH(eqnVar),
    [KB_Y] = KH(inv),
};


static const  key_handler_fn keymap_left_shift[64] =
// ----------------------------------------------------------------------------
//   Key handlers with left shift
// ----------------------------------------------------------------------------
{
    // Numeric keypad
    [KB_0] = KH(infinity),
    [KB_1] = KH(ArithmeticMenu),
    [KB_2] = KH(fact),
    [KB_3] = KH(hash),
    [KB_6] = KH(convert),
    [KB_9] = KH(FinanceMenu),
    [KB_DOT] = KH(tag),
    [KB_ADD] = KH(curlyBracket),
    [KB_SUB] = KH(parenBracket),
    [KB_MUL] = KH(squareBracket),

    // ON, BSP, ENTER, SPC
    [KB_ON] = KH(powerOff),
    [KB_ENT] = KH(enter),
    [KB_BKS] = KH(backsp),
    [KB_SPC] = KH(pi),

    // Cursor keys
    [KB_LF] = KH(startSelection),
    [KB_RT] = KH(endSelection),
    [KB_UP] = KH(UPDIR),
    [KB_DN] = KH(endSelection),

    // Soft menu keys
    [KB_A] = KH(softmenu1_1),
    [KB_B] = KH(softmenu2_1),
    [KB_C] = KH(softmenu3_1),
    [KB_D] = KH(softmenu4_1),
    [KB_E] = KH(softmenu5_1),
    [KB_F] = KH(softmenu6_1),
    [KB_G] = KH(softmenu1_2),
    [KB_H] = KH(softmenu2_2),
    [KB_I] = KH(softmenu3_2),
    [KB_J] = KH(softmenu4_2),
    [KB_K] = KH(softmenu5_2),
    [KB_L] = KH(softmenu6_2),

    // Regular keys
    [KB_M] = KH(rcl),
    [KB_N] = KH(ProgramMenu),
    [KB_Q] = KH(exp),
    [KB_R] = KH(sq),
    [KB_S] = asinKeyHandler,
    [KB_T] = KH(acos),
    [KB_U] = KH(atan),
    [KB_V] = KH(alog),
    [KB_W] = KH(notequal),
    [KB_X] = KH(le),
    [KB_Y] = KH(ge),
    [KB_Z] = KH(abs),
};


static const  key_handler_fn keymap_left_hold[64] =
// ----------------------------------------------------------------------------
//   Overrides for left hold
// ----------------------------------------------------------------------------
{
    // Cursor keys
    [KB_LF] = KH(copy),
    [KB_RT] = KH(paste),
    [KB_UP] = KH(cut),
};


static const  key_handler_fn keymap_right_shift[64] =
// ----------------------------------------------------------------------------
//   Key handlers with right shift
// ----------------------------------------------------------------------------
{
    // Numeric keypad
    [KB_0] = KH(arrow),
    [KB_1] = KH(ComplexMenu),
    [KB_2] = KH(LibrariesMenu),
    [KB_3] = KH(BasesMenu),
    [KB_6] = KH(UnitsMenu),
    [KB_7] = KH(SolverMenu),
    [KB_9] = KH(TimeMenu),
    [KB_DOT] = KH(newline),
    [KB_ADD] = KH(secoBracket),
    [KB_SUB] = KH(underscore),
    [KB_MUL] = KH(textBracket),

    // ON, BSP, ENTER, SPC
    [KB_ON] = KH(powerOff),
    [KB_BKS] = KH(clear),
    [KB_ENT] = KH(tonum),
    [KB_SPC] = KH(comma),

    // Cursor keys
    [KB_LF] = KH(startOfLine),
    [KB_RT] = KH(endOfLine),
    [KB_UP] = KH(startOfText),
    [KB_DN] = KH(endOfText),

    // Soft menu keys
    [KB_A] = KH(softmenu1_1),
    [KB_B] = KH(softmenu2_1),
    [KB_C] = KH(softmenu3_1),
    [KB_D] = KH(softmenu4_1),
    [KB_E] = KH(softmenu5_1),
    [KB_F] = KH(softmenu6_1),
    [KB_G] = KH(softmenu1_2),
    [KB_H] = KH(softmenu2_2),
    [KB_I] = KH(softmenu3_2),
    [KB_J] = KH(softmenu4_2),
    [KB_K] = KH(softmenu5_2),
    [KB_L] = KH(softmenu6_2),

    // Regular keys
    [KB_M] = KH(VariablesMenu),
    [KB_N] = KH(ArithmeticMenu),
    [KB_O] = KH(ProgramMenu),
    [KB_Q] = KH(ln),
    [KB_R] = KH(xroot),
    [KB_V] = KH(log),
    [KB_W] = KH(equal),
    [KB_X] = KH(ls),
    [KB_Y] = KH(gt),
    [KB_Z] = KH(arg),
};


static const  key_handler_fn keymap_right_hold[64] =
// ----------------------------------------------------------------------------
//   Overrides for right hold
// ----------------------------------------------------------------------------
{
    // Cursor keys
    [KB_LF] = KH(pageUp),
    [KB_RT] = KH(pageDown),
    [KB_UP] = KH(pageRight),
    [KB_DN] = KH(pageLeft),
};


static const  key_handler_fn keymap_alpha[64] =
// ----------------------------------------------------------------------------
//   Alpha mode key handlers
// ----------------------------------------------------------------------------
{
    // Numbers
    [KB_1] = KH(number),
    [KB_2] = KH(number),
    [KB_3] = KH(number),
    [KB_4] = KH(number),
    [KB_5] = KH(number),
    [KB_6] = KH(number),
    [KB_7] = KH(number),
    [KB_8] = KH(number),
    [KB_9] = KH(number),
    [KB_0] = KH(number),
    [KB_DOT] = KH(decimalDot),

    // ON, BSP, ENTER, SPC
    [KB_ON] = KH(cancel),
    [KB_ENT] = KH(enter),
    [KB_BKS] = KH(backsp),
    [KB_SPC] = KH(spc),

    // Cursor keys
    [KB_LF] = KH(left),
    [KB_RT] = KH(right),
    [KB_DN] = KH(down),
    [KB_UP] = KH(up),

    // Operators
    [KB_ADD] = KH(add),
    [KB_SUB] = KH(sub),
    [KB_MUL] = KH(mul),

     // Soft menu keys
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

    // Normal commands/functions
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
    [KB_SPC] = KH(cycleFormat)
};
