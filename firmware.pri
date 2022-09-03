#******************************************************************************
# firmwware.pri                                                   DB48X project
#******************************************************************************
#
#  File Description:
#
#   Configuration for firmware builds
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

# Strip the configuratoin to the strictest minimim
CONFIG(debug, debug|release) {
    CONFIG = debug static ordered
} else {
    CONFIG = static ordered
}

CONFIG += newrpl_firmware

TEMPLATE = app

DEFINES += TARGET_CALC

# Uncomment below to compile in thumb mode
#THUMB_MODE=-mthumb

# Uncomment below to generate detailed assembly output of each file
#DEVEL_OPTIONS=-Wa,-adhln=$@.s


# Need to put preamble and boot files first, do not change order
SOURCES +=\
    firmware/sys/target_50g/preamble.c \
    firmware/sys/target_50g/boot.c \
    firmware/sys/target_50g/battery.c \
    firmware/sys/target_50g/cpu.c \
    firmware/sys/target_50g/exception.c \
    firmware/sys/target_50g/irq.c \
    firmware/sys/target_50g/keyboard.c \
    firmware/sys/keybcommon.c \
    firmware/sys/target_50g/lcd.c \
    firmware/sys/target_50g/stdlib.c \
    firmware/sys/target_50g/timer.c \
    firmware/sys/target_50g/mem.c \
    firmware/sys/target_50g/flash.c \
    firmware/sys/target_50g/rtc.c \
    firmware/sys/usbcommon.c \
    firmware/sys/target_50g/usbdriver.c \
    firmware/sys/target_50g/fwupdate.c \
    firmware/sys/target_50g/sddriver.c \

include(newrpl.pri)


# Cross compiler
GCC_LIBDIR = $$system(arm-none-eabi-gcc -print-file-name=)

INCLUDEPATH += $$GCC_LIBDIR/include
QMAKE_LIBDIR += $$GCC_LIBDIR

INCLUDEPATH += firmware/include newrpl

## FIXME - We need this for a #include_next <stdint.h>, but we should not
INCLUDEPATH += /usr/include

LIBS += -lgcc

DISTFILES += firmware/sys/target_50g/ld.script

QMAKE_CC = arm-none-eabi-gcc
QMAKE_CXX = arm-none-eabi-g++
QMAKE_LINK = arm-none-eabi-gcc


QMAKE_CFLAGS_DEBUG = -g $${DEVEL_OPTIONS} -mtune=arm920t -mcpu=arm920t -march=armv4t -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -Og -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc -fno-tree-loop-distribute-patterns
QMAKE_CFLAGS_RELEASE = $${DEVEL_OPTIONS} -mtune=arm920t -mcpu=arm920t -march=armv4t -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -O2 -fno-partial-inlining -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc -fno-tree-loop-distribute-patterns

QMAKE_LFLAGS = -g -T$$PWD/firmware/sys/target_50g/ld.script -nodefaultlibs -nostdlib -L$$GCC_LIBDIR

QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom $(TARGET)
