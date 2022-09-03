#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------

CONFIG += newrpl_color
NEWRPL_HAL = primeg1

include(simulator.pri)

TARGET = prime-simulator

DEFINES += TARGET_PC_PRIMEG1 TARGET_PRIME1


HEADERS  += \
        firmware/include/cgl.h \

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-Prime

ANDROID_EXTRA_LIBS = $$PWD/external/libusb-1.0.22/android/libs/x86/libusb1.0.so $$PWD/external/libusb-1.0.22/android/libs/armeabi-v7a/libusb1.0.so
