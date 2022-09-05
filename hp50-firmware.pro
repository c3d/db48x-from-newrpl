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

NEWRPL_HAL = hp50

TARGET = hp50-firmware.elf

CONFIG(release, debug|release) {
    CONFIG = static ordered
}
CONFIG(debug, debug|release) {
    CONFIG = debug static ordered
}

DEFINES += TARGET_50G

# Need to put preamble and boot files first, do not change order
SOURCES +=\
        firmware/sys/target_50g/preamble.c \
        firmware/sys/target_50g/boot.c \
        firmware/sys/target_50g/battery.c \
        firmware/sys/target_50g/cpu.c \
        firmware/sys/target_50g/exception.c \
        firmware/sys/target_50g/irq.c \
        firmware/sys/target_50g/keyboard.c \
        firmware/sys/target_50g/lcd.c \
        firmware/sys/target_50g/stdlib.c \
        firmware/sys/target_50g/timer.c \
        firmware/sys/target_50g/mem.c \
        firmware/sys/target_50g/flash.c \
        firmware/sys/target_50g/rtc.c \
        firmware/sys/target_50g/usbdriver.c \
        firmware/sys/target_50g/fwupdate.c \
        firmware/sys/target_50g/sddriver.c \

include(firmware.pri)

DISTFILES += firmware/sys/target_50g/ld.script
