#******************************************************************************
# primeg1-multiload-firmware.pro                                  BD48X project
#******************************************************************************
#
#  File Description:
#
#
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

SOURCES += \
    firmware/sys/target_prime1/preamble_multiload.c \
    firmware/sys/target_prime1/boot_multiload.c \

include(primeg1-firmware.pro)

SOURCES -= \
        firmware/sys/target_prime1/preamble_prime1.c \
        firmware/sys/target_prime1/boot_prime1.c \

TARGET = primeg1_multiload.elf

DISTFILES += firmware/sys/target_prime1/ld_multiload.script

QMAKE_LFLAGS = -g -T$$PWD/firmware/sys/target_prime1/ld_multiload.script -nodefaultlibs -nostdlib -L$$GCC_LIBDIR

QMAKE_POST_LINK = $$PWD/tools-bin/elf2rom -pad1024k -outPRIME_OS.ROM $(TARGET)
