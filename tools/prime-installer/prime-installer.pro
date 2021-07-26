QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    fsystem/fatconvert.c \
    fsystem/fsallocator.c \
    fsystem/fsattr.c \
    fsystem/fscalcfreespace.c \
    fsystem/fschattr.c \
    fsystem/fschdir.c \
    fsystem/fschmode.c \
    fsystem/fsclose.c \
    fsystem/fscloseanddelete.c \
    fsystem/fsconvert2shortentry.c \
    fsystem/fscreate.c \
    fsystem/fsdelete.c \
    fsystem/fsdeletedirentry.c \
    fsystem/fseof.c \
    fsystem/fsexpandchain.c \
    fsystem/fsfileisopen.c \
    fsystem/fsfileisreferenced.c \
    fsystem/fsfilelength.c \
    fsystem/fsfindchar.c \
    fsystem/fsfindentry.c \
    fsystem/fsfindfile.c \
    fsystem/fsfindforcreation.c \
    fsystem/fsflushbuffers.c \
    fsystem/fsflushfatcache.c \
    fsystem/fsfreechain.c \
    fsystem/fsfreefile.c \
    fsystem/fsgetaccessdate.c \
    fsystem/fsgetchain.c \
    fsystem/fsgetchainsize.c \
    fsystem/fsgetcreattime.c \
    fsystem/fsgetcurrentvolume.c \
    fsystem/fsgetcwd.c \
    fsystem/fsgetdatetime.c \
    fsystem/fsgeterrormsg.c \
    fsystem/fsgetfilename.c \
    fsystem/fsgethandle.c \
    fsystem/fsgetnametype.c \
    fsystem/fsgetnextentry.c \
    fsystem/fsgetvolumefree.c \
    fsystem/fsgetvolumesize.c \
    fsystem/fsgetwritetime.c \
    fsystem/fsinit.c \
    fsystem/fsmkdir.c \
    fsystem/fsmountvolume.c \
    fsystem/fsmovedopenfiles.c \
    fsystem/fsnamecompare.c \
    fsystem/fsopen.c \
    fsystem/fsopendir.c \
    fsystem/fspackdir.c \
    fsystem/fspackname.c \
    fsystem/fspatchfatblock.c \
    fsystem/fsread.c \
    fsystem/fsreadll.c \
    fsystem/fsreleaseentry.c \
    fsystem/fsrename.c \
    fsystem/fsrestart.c \
    fsystem/fsrmdir.c \
    fsystem/fsseek.c \
    fsystem/fssetcasemode.c \
    fsystem/fssetcurrentvolume.c \
    fsystem/fsshutdown.c \
    fsystem/fssleep.c \
    fsystem/fsstripsemi.c \
    fsystem/fstell.c \
    fsystem/fstruncatechain.c \
    fsystem/fsupdatedirentry.c \
    fsystem/fsvolumeinserted.c \
    fsystem/fsvolumemounted.c \
    fsystem/fsvolumepresent.c \
    fsystem/fswrite.c \
    fsystem/fswritefatentry.c \
    fsystem/fswritell.c \
    fsystem/fsystem.c \
    fsystem/misalign.c \
    main.cpp \
    maindialog.cpp \
    sddriver.c \
    stdlib.c \
    thirdparty/md5c.c \
    utf8data.c \
    utf8lib.c

HEADERS += \
    fsystem/fsyspriv.h \
    fsystem/fsystem.h \
    maindialog.h \
    sddriver.h \
    thirdparty/global.h \
    thirdparty/md5.h \
    utf8lib.h

FORMS += \
    maindialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    installer-binaries.qrc

DISTFILES += \
    thirdparty/LICENSE.md5

# Set application icon for the windows applications - this is target-specific
win32: RC_ICONS = ../../../bitmap/newRPL.ico
