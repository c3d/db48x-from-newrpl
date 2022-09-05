#******************************************************************************
# primeg1-firmware.pro                                            BD48X project
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

NEWRPL_HAL = primeg1
CONFIG += newrpl_color

TARGET = primeg1-newrpl.elf

DEFINES += TARGET_PRIME1

SOURCES += \
        firmware/sys/target_prime1/preamble_prime1.c \
        firmware/sys/target_prime1/boot_prime1.c \
        firmware/sys/target_prime1/nand.c \
        firmware/sys/target_prime1/battery.c \
        firmware/sys/target_prime1/cpu.c \
        firmware/sys/target_prime1/exception.c \
        firmware/sys/target_prime1/irq.c \
        firmware/sys/target_prime1/keyboard.c \
        firmware/sys/target_prime1/lcd.c \
        firmware/sys/target_prime1/stdlib.c \
        firmware/sys/target_prime1/timer.c \
        firmware/sys/target_prime1/mem.c \
        firmware/sys/target_prime1/rtc.c \
        firmware/sys/target_prime1/touch.c \
        firmware/sys/target_prime1/usbdriver.c \
        firmware/sys/target_prime1/uart.c \
        firmware/sys/target_prime1/fwupdate.c \
        firmware/sys/target_prime1/sddriver.c \

include(firmware.pri)


DISTFILES += firmware/sys/target_prime1/ld_newrpl.script

QMAKE_CC = arm-none-eabi-gcc
QMAKE_CXX = arm-none-eabi-g++
QMAKE_LINK = arm-none-eabi-gcc

QMAKE_CFLAGS_DEBUG = -g $${DEVEL_OPTIONS} -mtune=arm926ej-s -mcpu=arm926ej-s -march=armv5tej -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -Og -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc -fno-tree-loop-distribute-patterns -fno-isolate-erroneous-paths-dereference
QMAKE_CFLAGS_RELEASE = $${DEVEL_OPTIONS} -mtune=arm926ej-s -mcpu=arm926ej-s -march=armv5tej -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -O2 -fno-partial-inlining -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc -fno-tree-loop-distribute-patterns -fno-isolate-erroneous-paths-dereference
QMAKE_LFLAGS = -g -T$$PWD/firmware/sys/target_prime1/ld_newrpl.script -nodefaultlibs -nostdlib -L$$GCC_LIBDIR

QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom -outNEWRPL.ROM $(TARGET)
