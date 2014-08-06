TEMPLATE = app

TARGET = CQToolStripTest

DEPENDPATH += .

QT += widgets

#CONFIG += debug

# Input
SOURCES += \
CQToolStripTest.cpp \

HEADERS += \
CQToolStripTest.h \

DESTDIR     = .
OBJECTS_DIR = .

INCLUDEPATH += \
../include \
../../CQToolStrip/include \
.

unix:LIBS += \
-L../lib \
-lCQToolStrip
