#******************************************************************************
# hp48-firmware.pro                                               DB48X project
#******************************************************************************
#
#  File Description:
#
#    Build the firmware for the HP 48GX II
#
#
#
#
#
#
#
#
#******************************************************************************
#  (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
#  (C) 2022 Claudio Lapilli and the newRPL team
#  This software is licensed under the terms described in LICENSE.txt
#******************************************************************************

isEmpty(PLATFORM):PLATFORM=48gii
isEmpty(TARGET):TARGET = hp48-firmware.elf

include(hp50-firmware.pro)

DEFINES -= TARGET_50G
DEFINES += TARGET_48GII

# Let's find the various platform .c files from the hp50g
INCLUDEPATH += firmware/platform/50g
