#******************************************************************************
# hp39-firmware.pro                                               DB48X project
#******************************************************************************
#
#  File Description:
#
#    Build the firmware for the HP39
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

isEmpty(TARGET):TARGET = hp39-firmware.elf

isEmpty(PLATFORM):PLATFORM=39gs
include(hp50-firmware.pro)

DEFINES -= TARGET_50G
DEFINES += TARGET_39GS

# Let's find the various platform .c files from the hp50g
INCLUDEPATH += firmware/platform/50g
