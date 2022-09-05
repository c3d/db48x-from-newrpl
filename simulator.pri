#******************************************************************************
# simulator.pri                                                   DB48X project
#******************************************************************************
#
#  File Description:
#
#    Configurations specific to a simulator build
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

# WARNING: Cannot use 'simulator' here, used by Qt itself
CONFIG += newrpl_simulator

QT += core gui quick widgets quickcontrols2 quickwidgets
TEMPLATE = app

include(newrpl.pri)

DEFINES += TARGET_PC

# Support for the PC target
SOURCES += \
        firmware/sys/target_pc/battery.c \
        firmware/sys/target_pc/boot.c \
        firmware/sys/target_pc/cpu.c \
        firmware/sys/target_pc/exception.c \
        firmware/sys/target_pc/flash.c \
        firmware/sys/target_pc/fwupdate.c \
        firmware/sys/target_pc/irq.c \
        firmware/sys/target_pc/keyboard.c \
        firmware/sys/target_pc/lcd.c \
        firmware/sys/target_pc/mem.c \
        firmware/sys/target_pc/rtc.c \
        firmware/sys/target_pc/sddriver.c \
        firmware/sys/target_pc/stdlib.c \
        firmware/sys/target_pc/timer.c \
        firmware/sys/target_pc/usbdriver.c \

# Qt support code
SOURCES += \
        interaction.cpp \
        interaction_rpl.c \
        main.cpp \
        mainwindow.cpp \
        qemuscreen.cpp \
        qpaletteeditor.cpp \
        rplthread.cpp \
        usbselector.cpp \

# Headers for the PC support in the firmware
HEADERS += \
        firmware/include/target_pc.h \

# Headers for PC GUI support
HEADERS  += \
        mainwindow.h \
        menuwidget.h \
        qemuscreen.h \
        qpaletteeditor.h \
        rplthread.h \
        usbselector.h \

# User interface forms
FORMS    += \
        mainwindow.ui \
        qpaletteeditor.ui \
        usbselector.ui

# Resources for the target platform
RESOURCES += annunciators-$${NEWRPL_HAL}.qrc

# Set application icon for the windows applications - this is target-specific
win32: RC_ICONS = bitmap/newRPL.ico


# Additional external library HIDAPI linked statically into the code
INCLUDEPATH += external/hidapi/hidapi

HEADERS += external/hidapi/hidapi/hidapi.h

win32: SOURCES += external/hidapi/windows/hid.c
win32: LIBS += -lsetupapi

android: SOURCES += external/hidapi/libusb/hid.c
android: INCLUDEPATH += external/libusb-1.0.22/libusb/
android: LIBS += -L$$PWD/external/libusb-1.0.22/android/libs/armeabi-v7a -L$$PWD/external/libusb-1.0.22/android/libs/x86 -lusb1.0

freebsd: SOURCES += external/hidapi/libusb/hid.c
freebsd: LIBS += -lusb -lthr -liconv

unix:!macx:!freebsd:!android: SOURCES += external/hidapi/linux/hid.c
unix:!macx:!freebsd:!android: LIBS += -ludev

macx: SOURCES += external/hidapi/mac/hid.c
macx: LIBS += -framework CoreFoundation -framework IOKit
