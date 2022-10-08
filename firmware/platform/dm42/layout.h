/*
 * Copyright (c) 2014-2022 Christophe de Dinechin and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 *
 * This is the screen layout for the DM42
 *
 * This a sorted list of items to draw, in order
 */

#define M       ((MENU_TAB_SPACE + 1))

static const layout_t layouts[] =
// ----------------------------------------------------------------------------
//    Default screen layout for the DM42
// ----------------------------------------------------------------------------
//    On the DM42, the softmenu keys are at the bottom, and the status area is
//    at the top, matching the origina HP48 layout and aligning naturally with
//    the DM42 physical keyboard. When a second menu is displayed, it shows
//    below the first one, and is accessed with the keys in the Sigma+ row.
//      Item                    Position                Reference       DX / DY
{
    {   screen_layout,          CENTER_IN,              NULL,           0, 0, },

    // The second menu is at the bottom
    {   menu2_1_layout,         BOTTOM_LEFT_IN,         screen_layout,  0, 0, },
    {   menu2_2_layout,         BOTTOM_RIGHT_OF,        menu2_1_layout, M, 0, },
    {   menu2_3_layout,         BOTTOM_RIGHT_OF,        menu2_2_layout, M, 0, },
    {   menu2_4_layout,         BOTTOM_RIGHT_OF,        menu2_3_layout, M, 0, },
    {   menu2_5_layout,         BOTTOM_RIGHT_OF,        menu2_4_layout, M, 0, },
    {   menu2_6_layout,         BOTTOM_RIGHT_OF,        menu2_5_layout, M, 0, },

    // Primary menu
    {  menu1_1_layout,          ABOVE_LEFT,             menu2_1_layout, 0, M, },
    {  menu1_2_layout,          BOTTOM_RIGHT_OF,        menu1_1_layout, M, 0, },
    {  menu1_3_layout,          BOTTOM_RIGHT_OF,        menu1_2_layout, M, 0, },
    {  menu1_4_layout,          BOTTOM_RIGHT_OF,        menu1_3_layout, M, 0, },
    {  menu1_5_layout,          BOTTOM_RIGHT_OF,        menu1_4_layout, M, 0, },
    {  menu1_6_layout,          BOTTOM_RIGHT_OF,        menu1_5_layout, M, 0, },

    // Command line
    {  cmdline_layout,          ABOVE_LEFT,             menu1_1_layout, 0, 0, },
    {  stack_layout,            ABOVE_LEFT,             cmdline_layout, 0, 0, },

    // Status area
    {  status_layout,           TOP_RIGHT_IN,            screen_layout, 0, 0, },
    {  annunciators_layout,     TOP_LEFT_IN,             screen_layout, 0, 0, },

    // Flags on the left at top
    { angle_mode_layout,        TOP_LEFT_IN,             status_layout, 4, 0, },
    { complex_flag_layout,      TOP_RIGHT_OF,        angle_mode_layout, 3, 0, },
    { busy_flag_layout,         TOP_RIGHT_OF,      complex_flag_layout, 3, 0, },
    { halted_flag_layout,       TOP_RIGHT_OF,         busy_flag_layout, 3, 0, },
    { alarm_flag_layout,        TOP_RIGHT_OF,       halted_flag_layout, 3, 0, },
    { receive_flag_layout,      TOP_RIGHT_OF,        alarm_flag_layout, 3, 0, },
    { sdcard_layout,            TOP_RIGHT_OF,      receive_flag_layout, 3, 0, },
    { user_flags_layout,        TOP_RIGHT_OF,            sdcard_layout, 8, 0, },

    // Battery and annunciators on the right at top
    { battery_layout,           TOP_RIGHT_IN,      annunciators_layout, 0, 0, },
    { lshift_layout,            TOP_LEFT_OF,            battery_layout, 4, 0, },
    { rshift_layout,            TOP_LEFT_OF,            battery_layout, 4, 0, },
    { alpha_layout,             TOP_LEFT_OF,             lshift_layout, 3, 0, },

    // Center messages
    { autocomplete_layout,      TOP_CENTER_IN,           status_layout, 0, 0, },
    { message_layout,           BELOW_CENTER_OF,   autocomplete_layout, 0, 1, },
    { path_layout,              BELOW_BOTTOM_LEFT,       status_layout, 0, 0, },

    // Display form
    {  form_layout,             ABOVE_LEFT,             cmdline_layout, 0, 0, },

    // Help covers the whole screen_layout_layout, erasses the stack
    { help_layout,              CENTER_IN,              screen_layout,  0, 0, },

    // Errors displayed on top of everything else
    { errors_layout,            CENTER_IN,              screen_layout,  5, 0, },
};
