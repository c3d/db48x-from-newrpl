#******************************************************************************
# dm42-simulator.pro                                              DB48X project
#******************************************************************************
#
#  File Description:
#
#    DM42  simulator using Qt for the user interface running newRPL
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

isEmpty(PLATFORM):PLATFORM = dm42

include(simulator.pri)

TARGET = dm42-simulator

DEFINES += TARGET_PC_DM42 TARGET_DM42
