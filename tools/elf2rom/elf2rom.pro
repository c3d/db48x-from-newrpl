TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    elf2rom.c

include(deployment.pri)
qtcAddDeployment()

LIBS += -lelf
