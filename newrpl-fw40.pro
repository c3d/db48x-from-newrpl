#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------
include(newrpl-fw.pro)

TARGET = newrpl40.elf

DEFINES -= TARGET_50G
DEFINES += TARGET_40GS

HEADERS += \
    firmware/include/target_40gs.h
