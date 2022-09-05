#******************************************************************************
# hp50-firmware.pro                                               DB48X project
#******************************************************************************
#
#  File Description:
#
#     NewRPL firmware for the HP50 calculator
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

# Set HAL if not set by including file
isEmpty(PLATFORM):PLATFORM = 50g
isEmpty(TARGET):TARGET = hp50-firmware.elf

CONFIG(release, debug|release) {
    CONFIG = static ordered
}
CONFIG(debug, debug|release) {
    CONFIG = debug static ordered
}

DEFINES += TARGET_50G

include(firmware.pri)
