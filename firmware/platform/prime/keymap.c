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
    [KB_ESC] = KH(cancel),
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
    [KB_SYMB] = KH(softmenu1_1),
    [KB_PLOT] = KH(softmenu2_1),
    [KB_NUM]  = KH(softmenu3_1),
    [KB_HELP] = KH(softmenu4_1),
    [KB_VIEW] = KH(softmenu5_1),
    [KB_MENU] = KH(softmenu6_1),

    // Special Prime keys
    [KB_APPS] = KH(MainMenu),
    [KB_HOME] = KH(HOME),
    [KB_CAS]  = KH(EquationsMenu),

    // White keys
    [KB_A] = KH(VariablesMenu),
    [KB_B] = KH(ProgramMenu),
    [KB_C] = KH(MathMenu),
    [KB_D] = KH(eqnVar),
    [KB_E] = KH(tofrac),
    [KB_F] = KH(pow),
    [KB_G] = KH(sin),
    [KB_H] = KH(cos),
    [KB_I] = KH(tan),
    [KB_J] = KH(ln),
    [KB_K] = KH(log),
    [KB_L] = KH(sq),
    [KB_M] = KH(chs),
    [KB_N] = KH(parenBracket),
    [KB_O] = KH(comma),
    [KB_P] = KH(eex),
};


static const  key_handler_fn keymap_left_shift[64] =
// ----------------------------------------------------------------------------
//   Key handlers with left shift
// ----------------------------------------------------------------------------
{
    // Numbers
    [KB_0] = KH(NotesMenu),
    [KB_1] = KH(secoBracket),
    [KB_2] = KH(iconst),
    [KB_3] = KH(pi),
    [KB_4] = KH(MatrixMenu),
    [KB_5] = KH(squareBracket),
    [KB_6] = KH(TestsMenu),
    [KB_7] = KH(ListsMenu),
    [KB_8] = KH(curlyBracket),
    [KB_9] = KH(SymbolsMenu),
    [KB_DOT] = KH(equal),

    // ON, BSP, ENTER, SPC
    [KB_ON] = KH(powerOff),
    [KB_ESC] = KH(clear),
    [KB_ENT] = KH(tonum),
    [KB_BKS] = KH(del),
    [KB_SPC] = KH(underscore),

    // Cursor keys
    [KB_LF] = KH(startSelection),
    [KB_RT] = KH(endSelection),
    [KB_UP] = KH(UPDIR),
    [KB_DN] = KH(endSelection),

    // Operators
    [KB_ADD] = KH(StackMenu),
    [KB_SUB] = KH(BasesMenu),
    [KB_MUL] = KH(angle),
    [KB_DIV] = KH(inv),

     // Soft menu keys
    [KB_SYMB] = KH(softmenu1_2),
    [KB_PLOT] = KH(softmenu2_2),
    [KB_NUM]  = KH(softmenu3_2),
    [KB_HELP] = KH(softmenu4_2),
    [KB_VIEW] = KH(softmenu5_2),
    [KB_MENU] = KH(softmenu6_2),

    // Special Prime keys
    [KB_APPS] = KH(InfoMenu),
    [KB_HOME] = KH(SettingsMenu),
    [KB_CAS]  = KH(FlagsMenu),

    // White keys
    [KB_A] = KH(CharsMenu),
    [KB_B] = KH(MemoryMenu),
    [KB_C] = KH(UnitsMenu),
    [KB_D] = KH(MENUSWAP),
    [KB_E] = KH(textBracket),
    [KB_F] = KH(xroot),
    [KB_G] = KH(asin),
    [KB_H] = KH(acos),
    [KB_I] = KH(atan),
    [KB_J] = KH(exp),
    [KB_K] = KH(alog),
    [KB_L] = KH(sqrt),
    [KB_M] = KH(abs),
    [KB_N] = KH(ticks),
    [KB_O] = KH(eval),
    [KB_P] = KH(sto),
};


static const  key_handler_fn keymap_left_hold[64] =
// ----------------------------------------------------------------------------
//   Overrides for left hold
// ----------------------------------------------------------------------------
{
    [KB_VIEW] = KH(copy),
    [KB_MENU] = KH(paste),
    [KB_HELP] = KH(cut),

    // Cursor keys
    [KB_LF] = KH(pageUp),
    [KB_RT] = KH(pageDown),
    [KB_UP] = KH(pageRight),
    [KB_DN] = KH(pageLeft),

    [KB_HOME] = KH(startOfText),
    [KB_CAS]  = KH(startOfText),
    [KB_SUB]  = KH(basecycle),
};


static const  key_handler_fn keymap_right_shift[64] =
// ----------------------------------------------------------------------------
//   Key handlers with right shift
// ----------------------------------------------------------------------------
{
    [KB_DOT] = KH(notequal),
    [KB_2] = KH(jconst),
    [KB_N] = KH(textBracket),
    [KB_SUB] = KH(basecycle),
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
    [KB_0] = KH(textBracket),
    [KB_3] = KH(hash),
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
    [KB_ADD] = KH(semi),
    [KB_SUB] = KH(colon),

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
    // REVISIT: Greek letters here?
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
