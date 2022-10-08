#ifndef THEME_H
#define THEME_H
/*
* Copyright (c) 2022, Christophe de Dinechin and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*
* Theme colors for the HP 50g and other gray-scale hardware
*/

// Theme colors for stack
#define THEME_STK_BG              THEME_GRAY0
#define THEME_STK_INDEX           THEME_GRAY15
#define THEME_STK_VLINE           THEME_GRAY8
#define THEME_STK_IDX_BG          THEME_GRAY0
#define THEME_STK_ITEMS           THEME_GRAY15
#define THEME_STK_SEL_BG          THEME_GRAY6
#define THEME_STK_SEL_ITEM        THEME_GRAY15
#define THEME_STK_CURSOR          THEME_GRAY15

// Theme colors for command line
#define THEME_CMD_BG              THEME_GRAY2
#define THEME_CMD_TEXT            THEME_GRAY15
#define THEME_CMD_SEL_BG          THEME_GRAY6
#define THEME_CMD_SELTEXT         THEME_GRAY15
#define THEME_CMD_CURSOR_BG       THEME_GRAY15
#define THEME_CMD_CURSOR          THEME_GRAY0
#define THEME_DIV_LINE            THEME_GRAY8

// Theme colors for menu
#define THEME_MENU_BG             THEME_GRAY15
#define THEME_MENU_TEXT           THEME_GRAY0
#define THEME_MENU_DIR_MARK       THEME_GRAY6
#define THEME_MENU_DIR            THEME_GRAY1
#define THEME_MENU_DIR_BG         THEME_GRAY14
#define THEME_MENU_HLINE          THEME_GRAY8
#define THEME_MENU_FOCUS_HLINE    THEME_GRAY0
#define THEME_MENU_PRESS_BG       255, 0,   0
#define THEME_MENU_FLAG_ON        THEME_GRAY0
#define THEME_MENU_FLAG_OFF       THEME_GRAY8

// Theme colors for status area
#define THEME_STA_BG              THEME_GRAY0
#define THEME_STA_TEXT            THEME_GRAY15
#define THEME_STA_ANNPRESS        THEME_GRAY8
#define THEME_STA_ANN             THEME_GRAY15
#define THEME_STA_BAT             THEME_GRAY15
#define THEME_STA_UFLAG0          THEME_GRAY8
#define THEME_STA_UFLAG1          THEME_GRAY15

// Theme colors for help and popup messages
#define THEME_HLP_BG              THEME_GRAY1
#define THEME_HLP_TEXT            THEME_GRAY10
#define THEME_HLP_LINES           THEME_GRAY8
#define THEME_HLP_TITLE           THEME_GRAY12
#define THEME_HLP_BOLD            THEME_GRAY15
#define THEME_HLP_ITALIC          THEME_GRAY6
#define THEME_HLP_CODE            THEME_GRAY15
#define THEME_HLP_CODE_BG         THEME_GRAY4


// Theme colors for Forms
#define THEME_FORM_BG             THEME_GRAY0
#define THEME_FORM_TEXT           THEME_GRAY15
#define THEME_FORM_SELTEXT        THEME_GRAY15
#define THEME_FORM_SEL_BG         THEME_GRAY5
#define THEME_FORM_CURSOR         THEME_GRAY15

// Theme color for errors
#define THEME_ERROR               THEME_GRAY15
#define THEME_ERROR_BG            THEME_GRAY2
#define THEME_ERROR_LINE          THEME_GRAY8


static const UNIFONT *defaultFont[FONTS_NUM] =
// ----------------------------------------------------------------------------
//   Table of default fonts for the HP 50G
// ----------------------------------------------------------------------------
{
    [FONT_INDEX_STACK]          = Font_7A,
    [FONT_INDEX_STACK_LEVEL1]   = Font_8A,
    [FONT_INDEX_STACK_INDEX]    = Font_8A,
    [FONT_INDEX_CMDLINE]        = Font_10A,
    [FONT_INDEX_CURSOR]         = Font_7A,
    [FONT_INDEX_MENU]           = Font_6A,
    [FONT_INDEX_STATUS]         = Font_6A,
    [FONT_INDEX_PLOT]           = Font_6A,
    [FONT_INDEX_FORMS]          = Font_6A,
    [FONT_INDEX_ERRORS]         = Font_8B,
    [FONT_INDEX_HELP_TEXT]      = Font_6m,
    [FONT_INDEX_HELP_TITLE]     = Font_8B,
    [FONT_INDEX_HELP_BOLD]      = Font_6A,
    [FONT_INDEX_HELP_ITALIC]    = Font_6A,
    [FONT_INDEX_HELP_CODE]      = Font_6m,
    [FONT_INDEX_BATTERY]        = Font_5A,
};

#endif // THEME_H
