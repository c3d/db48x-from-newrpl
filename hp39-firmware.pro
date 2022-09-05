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

# UNCOMMENT BELOW TO COMPILE IN THUMB MODE
THUMB_MODE=-mthumb

include(hp50-firmware.pro)

TARGET = hp39-firmware.elf

DEFINES -= TARGET_50G
DEFINES += TARGET_39GS

HEADERS += \
    firmware/include/target_39gs.h

OBJECTS_DIR = build/hp39-firmware
