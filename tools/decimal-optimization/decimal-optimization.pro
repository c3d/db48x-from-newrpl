TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += NDEBUG

SOURCES += main.c \
    decimal.c \
    sysvars.c \
    utf8data.c \
    utf8lib.c \
    atan_ltables.c \
    ln_ltables.c \
    lighttranscend.c

DISTFILES +=

HEADERS += \
    decimal.h \
    newrpl.h \
    sysvars.h \
    ui.h \
    utf8lib.h \
    bindecimal.h

