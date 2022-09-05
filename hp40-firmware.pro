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

include(hp50-firmware.pro)

TARGET = hp40-firmware.elf

DEFINES -= TARGET_50G
DEFINES += TARGET_40GS

HEADERS += \
    firmware/include/target_40gs.h

OBJECTS_DIR = build/hp40-firmware
