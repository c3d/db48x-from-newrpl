#******************************************************************************
# compiler.pro                                                    DB48X project
#******************************************************************************
#
#  File Description:
#
#    Qt project file for the newRPL command-line compiler
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

TARGET = newrpl-comp
isEmpty(HOST):HOST = compiler
isEmpty(PLATFORM):PLATFORM = dummy

include(newrpl.pri)

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += TARGET_PC NO_RPL_OBJECTS

SOURCES += \
    firmware/host/compiler/non-gui-stubs.c \
    newrpl/lib-4079-rpl2c.c \
    newrpl-comp.c

install_bin.path = $$PWD/tools-bin
!win32: install_bin.files = $$OUT_PWD/newrpl-comp
win32: install_bin.files = $$OUT_PWD/release/newrpl-comp.exe
INSTALLS += install_bin


# Additional external library HIDAPI linked statically into the code
INCLUDEPATH += external/hidapi/hidapi

HEADERS += external/hidapi/hidapi/hidapi.h

win32: SOURCES += external/hidapi/windows/hid.c
win32: LIBS += -lsetupapi

freebsd: SOURCES += external/hidapi/libusb/hid.c
freebsd: LIBS += -lusb -lthr -liconv

unix:!macx:!freebsd: SOURCES += external/hidapi/linux/hid.c
unix:!macx:!freebsd: LIBS += -ludev

macx: SOURCES += external/hidapi/mac/hid.c
macx: LIBS += -framework CoreFoundation -framework IOKit

# End of HIDAPI
