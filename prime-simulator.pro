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
#  (C) 2022 Claudio Lapilli and the newRPL team
#  This software is licensed under the terms described in LICENSE.txt
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
