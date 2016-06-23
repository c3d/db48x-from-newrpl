TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c

install_bin.path = $$PWD/../../../tools-bin
install_bin.files = $$OUT_PWD/bmp2font
INSTALLS += install_bin
