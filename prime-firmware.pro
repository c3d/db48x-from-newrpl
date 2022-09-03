#******************************************************************************
# prime-firmware.pro                                              DB48X project
#******************************************************************************
#
#  File Description:
#
#    Build a firmware image for the HP Prime G1
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

isEmpty(PLATFORM):PLATFORM = prime
CONFIG += newrpl_color

MACHINE_CPU  = arm920t
MACHINE_TUNE = $$MACHINE_CPU
MACHINE_ARCH = armv4t

TARGET = primeg1-newrpl.elf

DEFINES += TARGET_PRIME1

include(firmware.pri)

SOURCES += \
        firmware/platform/prime/touch.c \
        firmware/platform/prime/uart.c \

DISTFILES += firmware/platform/$$PLATFORM/ld.script

QMAKE_CC = arm-none-eabi-gcc
QMAKE_CXX = arm-none-eabi-g++
QMAKE_LINK = arm-none-eabi-gcc

QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom -outNEWRPL.ROM $(TARGET)
