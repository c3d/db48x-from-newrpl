#******************************************************************************
# prime-simulator.pro                                             DB48X project
#******************************************************************************
#
#  File Description:
#
#    HP Prime G1 simulator using Qt for the user interface
#    running newRPL
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

CONFIG += newrpl_color
NEWRPL_HAL = primeg1

include(simulator.pri)

TARGET = prime-simulator

DEFINES += TARGET_PC_PRIMEG1 TARGET_PRIME1


HEADERS  += \
        firmware/include/cgl.h \

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-Prime

ANDROID_EXTRA_LIBS = $$PWD/external/libusb-1.0.22/android/libs/x86/libusb1.0.so $$PWD/external/libusb-1.0.22/android/libs/armeabi-v7a/libusb1.0.so
