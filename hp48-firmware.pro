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

include(hp50-firmware.pro)

TARGET = hp48-firmware.elf

DEFINES -= TARGET_50G
DEFINES += TARGET_48GII

HEADERS += \
    firmware/include/target_48gii.h

OBJECTS_DIR = build/hp48-firmware
