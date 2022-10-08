/*
 * Copyright (c) 2014-2022 Christophe de Dinechin and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 *
 * This is the screen layout for the HP 50G
 *
 * This a sorted list of items to draw, in order
 */

#define M       ((MENU_TAB_SPACE + 1))

static const layout_t layouts[] =
// ----------------------------------------------------------------------------
//    Default screen layout for the HP50G
// ----------------------------------------------------------------------------
// On the HP50G, we put the extra 6 physical menu keys on the left, to mach
// the physical keyboard layout. The status area is on the right, below
// the first menu, to occupy the "empty space" left by the arrow pad on the
// physical keyboard. It extends to the left of the screen when the second
// menu is not enabled.
// clang-format off
//      Item                    Position                Reference       DX / DY
{
    {   screen_layout,          CENTER_IN,              NULL,           0, 0, },

    // The second menu is at the bottom, matches layout of G/H/I/J/K/L
    {   menu2_4_layout,         BOTTOM_LEFT_IN,         screen_layout,  0, 0, },
    {   menu2_5_layout,         BOTTOM_RIGHT_OF,        menu2_4_layout, M, 0, },
    {   menu2_6_layout,         BOTTOM_RIGHT_OF,        menu2_5_layout, M, 0, },

    // Second row is above the previous one
    {  menu2_1_layout,          ABOVE_LEFT,             menu2_4_layout, 0, M, },
    {  menu2_2_layout,          ABOVE_LEFT,             menu2_5_layout, 0, M, },
    {  menu2_3_layout,          ABOVE_LEFT,             menu2_6_layout, 0, M, },

    // Primary menu
    {  menu1_1_layout,          ABOVE_LEFT,             menu2_1_layout, 0, M, },
    {  menu1_2_layout,          BOTTOM_RIGHT_OF,        menu1_1_layout, M, 0, },
    {  menu1_3_layout,          BOTTOM_RIGHT_OF,        menu1_2_layout, M, 0, },
    {  menu1_4_layout,          BOTTOM_RIGHT_OF,        menu1_3_layout, M, 0, },
    {  menu1_5_layout,          BOTTOM_RIGHT_OF,        menu1_4_layout, M, 0, },
    {  menu1_6_layout,          BOTTOM_RIGHT_OF,        menu1_5_layout, M, 0, },

    // Status area
    {  status_layout,           TOP_RIGHT_OF,           menu2_3_layout, 1, 0, },
    {  annunciators_layout,     BOTTOM_RIGHT_OF,        menu2_6_layout, 1, 0, },

    // Command line
    {  cmdline_layout,          ABOVE_LEFT,             menu1_1_layout, 0, 1, },
    {  stack_layout,            ABOVE_LEFT,             cmdline_layout, 0, 1, },

    // Flags (top of status area)
    { angle_mode_layout,        TOP_LEFT_IN,             status_layout, 2, 0, },
    { complex_flag_layout,      TOP_RIGHT_OF,        angle_mode_layout, 0, 0, },
    { busy_flag_layout,         TOP_RIGHT_OF,      complex_flag_layout, 0, 0, },
    { halted_flag_layout,       TOP_RIGHT_OF,         busy_flag_layout, 0, 0, },
    { alarm_flag_layout,        TOP_RIGHT_OF,       halted_flag_layout, 0, 0, },
    { receive_flag_layout,      TOP_RIGHT_OF,        alarm_flag_layout, 0, 0, },
    { sdcard_layout,            TOP_RIGHT_OF,      receive_flag_layout, 0, 0, },

    // Annunciators (bottom of status area)
    { lshift_layout,            BOTTOM_LEFT_IN,    annunciators_layout, 2, 0, },
    { rshift_layout,            BOTTOM_LEFT_IN,    annunciators_layout, 2, 0, },
    { alpha_layout,             TOP_RIGHT_OF,            lshift_layout, 2, 0, },
    { battery_layout,           BOTTOM_RIGHT_IN,   annunciators_layout, 0, 0, },
    { user_flags_layout,        BOTTOM_LEFT_OF,         battery_layout, 0, 0, },

    // Messages (center of status area)
    { message_layout,           BELOW_CENTER_OF,         status_layout, 0, 0, },
    { autocomplete_layout,      BELOW_CENTER_OF,         status_layout, 0, 0, },
    { path_layout,              BOTTOM_CENTER_IN,  annunciators_layout, 0, 0, },

    // Display form
    {  form_layout,             ABOVE_CENTER_OF,        cmdline_layout, 0, 0, },

    // Help covers the whole screen_layout_layout, erasses the stack
    { help_layout,              CENTER_IN,              screen_layout,  0, 0, },

    // Errors displayed on top of everything else
    { errors_layout,            CENTER_IN,             screen_layout,  15, 0, },
};

#undef M

// clang-format on
