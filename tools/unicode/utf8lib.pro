TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    utf8lib.c

include(deployment.pri)
qtcAddDeployment()

DISTFILES += \
    UnicodeData.txt \
    DerivedNormalizationProps.txt \
    CompositionExclusions.txt \
    NormalizationTest.txt

HEADERS += \
    utf8lib.h

