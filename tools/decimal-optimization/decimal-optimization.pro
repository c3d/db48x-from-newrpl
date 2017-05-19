TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += NDEBUG

SOURCES += main.c \
    atan_1_8_comp.c \
    atan_1_8_light.c \
    atan_1_comp.c \
    atan_2_8_comp.c \
    atan_2_8_light.c \
    atan_2_comp.c \
    atan_5_8_comp.c \
    atan_5_8_light.c \
    atan_5_comp.c \
    atanh_1_8_comp.c \
    atanh_1_8_light.c \
    atanh_1_comp.c \
    atanh_2_8_comp.c \
    atanh_2_8_light.c \
    atanh_2_comp.c \
    atanh_5_8_comp.c \
    atanh_5_8_light.c \
    atanh_5_comp.c \
    cordic_K_8_comp.c \
    cordic_K_8_light.c \
    cordic_K_comp.c \
    cordic_kh_8_light.c \
    cordic_Kh_8_comp.c \
    cordic_Kh_comp.c \
    decimal.c \
    dectranscen.c \
    sysvars.c \
    utf8data.c \
    utf8lib.c \
    bin-integers.c \
    atan_bintable.c

DISTFILES +=

HEADERS += \
    decimal.h \
    newrpl.h \
    sysvars.h \
    ui.h \
    utf8lib.h

