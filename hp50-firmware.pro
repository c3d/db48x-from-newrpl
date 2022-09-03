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
#  This software is licensed under the GNU General Public License v3
#******************************************************************************
#  This file is part of BD48X.
#
#  BD48X is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  BD48X is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with BD48X.  If not, see <https://www.gnu.org/licenses/>.
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
