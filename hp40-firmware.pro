#******************************************************************************
# hp40-firmware.pro                                               DB48X project
#******************************************************************************
#
#  File Description:
#
#    Build the firmware for the HP40
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

isEmpty(PLATFORM):PLATFORM=40gs
isEmpty(TARGET):TARGET = hp40-firmware.elf

include(hp50-firmware.pro)

DEFINES -= TARGET_50G
DEFINES += TARGET_40GS

# Let's find the various platform .c files from the hp50g
INCLUDEPATH += firmware/platform/50g
