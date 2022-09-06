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
#  (C) 2022 Claudio Lapilli and the newRPL team
#  This software is licensed under the terms described in LICENSE.txt
#******************************************************************************

isEmpty(HOST):HOST = hardware

newrpl_color:NEWRPL_CONFIG += newrpl_color

# Strip the configuratoin to the strictest minimim
CONFIG(debug, debug|release) {
    CONFIG = debug static ordered
} else {
    CONFIG = static ordered
}

CONFIG += newrpl_firmware $$NEWRPL_CONFIG

TEMPLATE = app

DEFINES += TARGET_FIRMWARE
DEFINES += RECORDER_STANDALONE RECORDER_STANDALONE_PRINTF RECORDER_NO_ATOMICS

# Uncomment below to compile in thumb mode
#THUMB_MODE=-mthumb

# Uncomment below to generate detailed assembly output of each file
#DEVEL_OPTIONS=-Wa,-adhln=$@.s

include(newrpl.pri)

# ARM-specific optimizations
SOURCES += \
    newrpl/mul_real_arm.c \

# Cross compiler
GCC_LIBDIR = $$system(arm-none-eabi-gcc -print-file-name=)

INCLUDEPATH += $$GCC_LIBDIR/include
QMAKE_LIBDIR += $$GCC_LIBDIR

INCLUDEPATH += firmware/include newrpl

## FIXME - We need this for a #include_next <stdint.h>, but we should not
INCLUDEPATH += /usr/include

LIBS += -lc -lgcc

DISTFILES += firmware/platform/$$PLATFORM/ld.script

QMAKE_CC = arm-none-eabi-gcc
QMAKE_CXX = arm-none-eabi-g++
QMAKE_LINK = arm-none-eabi-gcc


QMAKE_CFLAGS_DEBUG = -g $${DEVEL_OPTIONS} -mtune=arm920t -mcpu=arm920t -march=armv4t -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -Og -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc -fno-tree-loop-distribute-patterns
QMAKE_CFLAGS_RELEASE = $${DEVEL_OPTIONS} -mtune=arm920t -mcpu=arm920t -march=armv4t -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -O2 -fno-partial-inlining -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc -fno-tree-loop-distribute-patterns

QMAKE_LFLAGS = -g -T$$PWD/firmware/platform/$$PLATFORM/ld.script -nodefaultlibs -nostdlib -L$$GCC_LIBDIR

QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom $(TARGET)
