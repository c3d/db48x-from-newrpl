TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    elf2rom.c

QMAKE_CFLAGS += $$system(pkg-config --cflags libelf)
LIBS += $$system(pkg-config --libs libelf)

install_bin.path = $$PWD/../../tools-bin
install_bin.files = $$OUT_PWD/elf2rom
INSTALLS += install_bin
