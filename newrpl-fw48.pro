#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------
include(newrpl-fw.pro)

TARGET = newrpl48.elf

DEFINES -= TARGET_50G
DEFINES += TARGET_48GII

HEADERS += \
    firmware/include/target_48gii.h
