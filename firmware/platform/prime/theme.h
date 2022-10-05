#ifndef THEME_H
#define THEME_H
/*
* Copyright (c) 2022, Christophe de Dinechin and the newRPL Team
* All rights reserved.
* This file is released under the 3-clause BSD license.
* See the file LICENSE.txt that shipped with this distribution.
*
* Theme colors for the HP Prime
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
#define THEME_MENU_DIR_MARK       60, 120, 250
#define THEME_MENU_DIR            THEME_GRAY1
#define THEME_MENU_DIR_BG         THEME_GRAY14
#define THEME_MENU_HLINE          THEME_GRAY8
#define THEME_MENU_FOCUS_HLINE    THEME_GRAY0
#define THEME_MENU_PRESS_BG       255, 0, 0
#define THEME_MENU_FLAG_ON        0, 200, 0
#define THEME_MENU_FLAG_OFF       200, 0, 0

// Theme colors for status area
#define THEME_STA_BG              THEME_GRAY0
#define THEME_STA_TEXT            THEME_GRAY15
#define THEME_STA_ANNPRESS        THEME_GRAY8
#define THEME_STA_ANN             THEME_GRAY15
#define THEME_STA_BAT             THEME_GRAY15
#define THEME_STA_UFLAG0          THEME_GRAY8
#define THEME_STA_UFLAG1          THEME_GRAY15

// Theme colors for help and popup messages
#define THEME_HLP_BG              THEME_GRAY0
#define THEME_HLP_TEXT            THEME_GRAY15
#define THEME_HLP_LINES           60, 120, 250
#define THEME_HLP_TITLE           60, 20, 150
#define THEME_HLP_BOLD            20, 150, 20
#define THEME_HLP_ITALIC          150, 20, 0

// Theme colors for Forms
#define THEME_FORM_BG             THEME_GRAY0
#define THEME_FORM_TEXT           THEME_GRAY15
#define THEME_FORM_SELTEXT        THEME_GRAY15
#define THEME_FORM_SEL_BG         THEME_GRAY5
#define THEME_FORM_CURSOR         THEME_GRAY15

#endif // THEME_H
