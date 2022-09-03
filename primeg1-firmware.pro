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

NEWRPL_HAL = primeg1

TARGET = primeg1-newrpl.elf

DEFINES += TARGET_PRIME1

include(firmware.pri)


DISTFILES += firmware/sys/target_prime1/ld_newrpl.script

QMAKE_CC = arm-none-eabi-gcc
QMAKE_CXX = arm-none-eabi-g++
QMAKE_LINK = arm-none-eabi-gcc

QMAKE_CFLAGS_DEBUG = -g $${DEVEL_OPTIONS} -mtune=arm926ej-s -mcpu=arm926ej-s -march=armv5tej -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -Og -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc -fno-tree-loop-distribute-patterns -fno-isolate-erroneous-paths-dereference
QMAKE_CFLAGS_RELEASE = $${DEVEL_OPTIONS} -mtune=arm926ej-s -mcpu=arm926ej-s -march=armv5tej -mlittle-endian -fno-jump-tables -fomit-frame-pointer -fno-toplevel-reorder -msoft-float -O2 -fno-partial-inlining -pipe $${THUMB_MODE} -mthumb-interwork -nostdinc -fno-tree-loop-distribute-patterns -fno-isolate-erroneous-paths-dereference
QMAKE_LFLAGS = -g -T$$PWD/firmware/sys/target_prime1/ld_newrpl.script -nodefaultlibs -nostdlib -L$$GCC_LIBDIR

QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom -outNEWRPL.ROM $(TARGET)
