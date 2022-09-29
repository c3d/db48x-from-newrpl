TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += ttf2font.c

QMAKE_CFLAGS += $$system($$pkgConfigExecutable() --cflags freetype2)
LIBS += $$system($$pkgConfigExecutable() --libs freetype2)

install_bin.path = $$PWD/../../../tools-bin
install_bin.files = $$OUT_PWD/ttf2font
INSTALLS += install_bin
