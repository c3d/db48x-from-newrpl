/*
 * Copyright (c) 2014-2022 Christophe de Dinechin and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 *
 * This is the screen layout for the HP Prime
 *
 * This a sorted list of items to draw, in order
 */

#define M       ((MENU_TAB_SPACE + 1))

static const layout_t layouts[] =
// ----------------------------------------------------------------------------
//    Default screen layout for the HP Prime
// ----------------------------------------------------------------------------
// On the HP Prime, we put the status area, so that the menu keys line up with
// the physical menu keys. Also, layout the menu keys vertically to match
// their physical layout
// clang-format off
//      Item                    Position                Reference       DX / DY
{
    {   screen_layout,          CENTER_IN,              NULL,           0, 0, },

    // The second menu is at the bottom, matches layout of G/H/I/J/K/L
    {  menu2_3_layout,          BOTTOM_LEFT_IN,         screen_layout,  0, 0, },
    {  menu2_2_layout,          ABOVE_LEFT,             menu2_3_layout, 0, M, },
    {  menu2_1_layout,          ABOVE_LEFT,             menu2_2_layout, 0, M, },

    // Second row is above the previous one
    {  menu2_6_layout,          BOTTOM_RIGHT_IN,         screen_layout, 0, 0, },
    {  menu2_5_layout,          ABOVE_RIGHT,            menu2_6_layout, 0, M, },
    {  menu2_4_layout,          ABOVE_RIGHT,            menu2_5_layout, 0, M, },

    // Primary menu
    {  menu1_1_layout,          BOTTOM_RIGHT_OF,        menu2_1_layout, M, 0, },
    {  menu1_2_layout,          BOTTOM_RIGHT_OF,        menu2_2_layout, M, 0, },
    {  menu1_3_layout,          BOTTOM_RIGHT_OF,        menu2_3_layout, M, 0, },
    {  menu1_4_layout,          BOTTOM_LEFT_OF,         menu2_4_layout, M, 0, },
    {  menu1_5_layout,          BOTTOM_LEFT_OF,         menu2_5_layout, M, 0, },
    {  menu1_6_layout,          BOTTOM_LEFT_OF,         menu2_6_layout, M, 0, },

    // Status area
    {  spacer_layout,           CLIPPING|TOP_RIGHT_OF,  menu1_1_layout, 1, 3, },
    {  filler_layout,           BOTTOM_LEFT_OF,         menu1_6_layout, 0, 0, },
    {  status_layout,           TOP_CENTER_IN,           filler_layout, 0, 1, },
    {  annunciators_layout,     BOTTOM_CENTER_IN,        filler_layout, 0, 1, },

    // Command line
    {  screen_layout,           CLIPPING|CENTER_IN,      screen_layout, 0, 0, },
    {  cmdline_layout,          ABOVE_LEFT,             menu2_1_layout, 0, 0, },
    {  stack_layout,            ABOVE_LEFT,             cmdline_layout, 0, 0, },

    // Status indicators (top of status area)
    { angle_mode_layout,        TOP_LEFT_IN,             status_layout, 3, 0, },
    { complex_flag_layout,      TOP_RIGHT_OF,        angle_mode_layout, 0, 0, },
    { busy_flag_layout,         TOP_RIGHT_OF,      complex_flag_layout, 0, 0, },
    { halted_flag_layout,       TOP_RIGHT_OF,         busy_flag_layout, 0, 0, },
    { alarm_flag_layout,        TOP_RIGHT_OF,       halted_flag_layout, 0, 0, },
    { receive_flag_layout,      TOP_RIGHT_OF,        alarm_flag_layout, 0, 0, },
    { sdcard_layout,            TOP_RIGHT_OF,      receive_flag_layout, 0, 0, },

    // Annunciators (bottom of status area)
    { lshift_layout,            BOTTOM_LEFT_IN,    annunciators_layout, 1, 0, },
    { rshift_layout,            BOTTOM_LEFT_IN,    annunciators_layout, 1, 0, },
    { alpha_layout,             TOP_RIGHT_OF,            lshift_layout, 1, 0, },
    { battery_layout,           BOTTOM_RIGHT_IN,   annunciators_layout, 0, 0, },
    { user_flags_layout,        BOTTOM_LEFT_OF,         battery_layout, 0, 0, },

    // Messages and autocompletion (center of status area)
    { autocomplete_layout,      ABOVE_LEFT,             cmdline_layout, 0, 1, },
    { message_layout,           BELOW_CENTER_OF,         status_layout, 0, 0, },
    { path_layout,              ABOVE_CENTER_OF,   annunciators_layout, 0, 0, },

    // Display form
    {  form_layout,             ABOVE_CENTER_OF,        cmdline_layout, 2, 2, },

    // Help covers the whole screen_layout_layout, erases the stack
    { help_layout,              CENTER_IN,              screen_layout,  0, 0, },

    // Errors displayed on top of everything else
    { errors_layout,            CENTER_IN,              screen_layout,  5, 0, },
};

#undef M

// clang-format on
